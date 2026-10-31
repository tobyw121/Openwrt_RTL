#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Execute real rtl8367b detection/allocation paths with mocked SMI hardware."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
DRIVERS = ROOT/'target/linux/generic/files/drivers/net/phy'


def function(text, name):
    start = re.search(r'^static (?:int|bool) '+name+r'\(', text, re.M).start()
    brace = text.index('{', start)
    depth, end = 1, brace+1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


class RTL8367BBackportTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which('gcc') and shutil.which('patch'), 'compiler/patch required')
    def test_dsa_chip_id_window_error_cleanup(self):
        patch = ROOT/'target/linux/realtek/patches-6.6/341-net-dsa-realtek-relock-chip-id-after-read-errors.patch'
        # Recover the exact old hunk and apply the real patch with no fuzz.
        old = '\n'.join(line[1:] for line in patch.read_text().splitlines()
                        if line.startswith((' ', '-')) and not line.startswith('---'))+'\n'
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)
            target = path/'drivers/net/dsa/realtek/rtl8365mb.c'
            target.parent.mkdir(parents=True)
            target.write_text(old)
            subprocess.run(['patch', '--batch', '--fuzz=0', '-p1', '-i', str(patch)],
                           cwd=path, check=True, capture_output=True)
            actual = function(target.read_text(), 'rtl8365mb_get_chip_id_and_ver')
            c = r'''
#include <assert.h>
#include <stdint.h>
#include <errno.h>
typedef uint32_t u32;
#define RTL8365MB_MAGIC_REG 0x13c2
#define RTL8365MB_MAGIC_VALUE 0x0249
#define RTL8365MB_CHIP_ID_REG 0x1300
#define RTL8365MB_CHIP_VER_REG 0x1301
struct regmap { int unlock_err, id_err, ver_err, reset_err, unlocks, resets; };
static int regmap_write(struct regmap *m, int reg, int value)
{
    assert(reg == RTL8365MB_MAGIC_REG);
    if (value) { m->unlocks++; return m->unlock_err; }
    m->resets++; return m->reset_err;
}
static int regmap_read(struct regmap *m, int reg, u32 *value)
{
    if (reg == RTL8365MB_CHIP_ID_REG) { *value = 0x6642; return m->id_err; }
    assert(reg == RTL8365MB_CHIP_VER_REG);
    *value = 0x30; return m->ver_err;
}
'''+actual+r'''
int main(void)
{
    struct regmap m = {0}; u32 id, ver;
    assert(rtl8365mb_get_chip_id_and_ver(&m, &id, &ver) == 0);
    assert(id == 0x6642 && ver == 0x30 && m.resets == 1);
    m = (struct regmap){.id_err = -EIO};
    assert(rtl8365mb_get_chip_id_and_ver(&m, &id, &ver) == -EIO);
    assert(m.resets == 1);
    m = (struct regmap){.ver_err = -EIO, .reset_err = -ETIMEDOUT};
    assert(rtl8365mb_get_chip_id_and_ver(&m, &id, &ver) == -EIO);
    assert(m.resets == 1);
    m = (struct regmap){.reset_err = -ETIMEDOUT};
    assert(rtl8365mb_get_chip_id_and_ver(&m, &id, &ver) == -ETIMEDOUT);
    m = (struct regmap){.unlock_err = -EBUSY};
    assert(rtl8365mb_get_chip_id_and_ver(&m, &id, &ver) == -EBUSY);
    assert(m.resets == 0);
    return 0;
}
'''
            (path/'test.c').write_text(c)
            subprocess.run(['gcc', '-std=gnu11', '-Wall', '-Wextra', '-Werror',
                            str(path/'test.c'), '-o', str(path/'test')],
                           check=True, capture_output=True)
            subprocess.run([str(path/'test')], check=True)

    @unittest.skipUnless(shutil.which('gcc'), 'host compiler required')
    def test_real_chip_detection_and_allocation_failure(self):
        source = (DRIVERS/'rtl8367b.c').read_text()
        header = (DRIVERS/'rtl8366_smi.h').read_text()
        enum = re.search(r'typedef enum rtl8367b_chip_e \{.*?rtl8367b_chip_t;',
                         header, re.S).group()
        names = ['RTL8367B_RTL_MAGIC_ID_REG', 'RTL8367B_RTL_MAGIC_ID_VAL',
                 'RTL8367B_CHIP_NUMBER_REG', 'RTL8367B_CHIP_VER_REG']
        defines = '\n'.join(re.search(r'^#define\s+'+name+r'\s+[^\n]+', source, re.M).group()
                            for name in names)
        stubs = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <stddef.h>
typedef uint32_t u32;
struct rtl8366_vlan_mc { int dummy; };
struct rtl8366_smi {
    int rtl8367b_chip, num_vlan_mc;
    struct rtl8366_vlan_mc *emu_vlanmc;
    void *parent;
};
struct rtl8367b_initval { int reg, value; };
static const struct rtl8367b_initval rtl8367b_initvals[] = {{0,0}};
static const struct rtl8367b_initval rtl8367c_initvals[] = {{0,0}};
static struct rtl8366_vlan_mc pool[32];
static int allocation_fails, writes, read_error, reset_error, unlock_error;
static int relocks, unlocks;
static u32 chip_id, chip_version;
#define dev_err(...) ((void)0)
#define dev_info(...) ((void)0)
#define GFP_KERNEL 0
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
static void *kzalloc(size_t size, int flags)
{
    assert(size == sizeof(pool));
    return allocation_fails ? NULL : pool;
}
static int rtl8366_smi_write_reg(struct rtl8366_smi *smi, int reg, int value)
{
    assert(reg == 0x13c2);
    if (value) { unlocks++; return unlock_error; }
    relocks++; return reset_error;
}
static int rtl8366_smi_read_reg(struct rtl8366_smi *smi, int reg, u32 *value)
{
    if (read_error) return read_error;
    *value = reg == 0x1300 ? chip_id : chip_version;
    return 0;
}
static int rtl8367b_write_initvals(struct rtl8366_smi *smi,
                const struct rtl8367b_initval *values, int count)
{ writes++; return 0; }
'''
        main = r'''
int main(void)
{
    struct rtl8366_smi smi = {.num_vlan_mc = 32};
    struct { u32 id, ver; int chip; } cases[] = {
        {0x6642, 0x10, RTL8367B_CHIP_RTL8367S_VB},
        {0x6642, 0x30, RTL8367B_CHIP_RTL8367D_RB_VC},
        {0x6367, 0x20, RTL8367B_CHIP_RTL8367RB_VB},
        {0x6367, 0xa0, RTL8367B_CHIP_RTL8367S},
        {0x6367, 0x1000, RTL8367B_CHIP_RTL8367RB},
        {0x6367, 0x1010, RTL8367B_CHIP_RTL8367R_VB},
    };
    for (unsigned i = 0; i < ARRAY_SIZE(cases); i++) {
        chip_id = cases[i].id; chip_version = cases[i].ver;
        assert(rtl8367b_detect(&smi) == 0);
        assert(smi.rtl8367b_chip == cases[i].chip);
        assert(rtl8367b_is_d(&smi) == (i < 2));
        allocation_fails = 1; writes = 0;
        int ret = rtl8367b_init_regs(&smi);
        assert(ret == (i < 2 ? -ENOMEM : 0));
        assert(writes == (i < 2 ? 0 : 1));
        allocation_fails = 0; writes = 0;
        assert(rtl8367b_init_regs(&smi) == 0);
        assert(writes == (i == 1 ? 0 : 1));
        assert((smi.emu_vlanmc != NULL) == (i < 2));
    }
    chip_id = 0x1234; chip_version = 0x30;
    assert(rtl8367b_detect(&smi) == -ENODEV);
    assert(smi.rtl8367b_chip == RTL8367B_CHIP_UNKNOWN);
    chip_id = 0x6642; chip_version = 0x40;
    assert(rtl8367b_detect(&smi) == -ENODEV);
    read_error = -EIO;
    relocks = 0;
    assert(rtl8367b_detect(&smi) == -EIO);
    assert(relocks == 1);
    reset_error = -ETIMEDOUT;
    assert(rtl8367b_detect(&smi) == -EIO); /* original read error wins */
    read_error = 0;
    assert(rtl8367b_detect(&smi) == -ETIMEDOUT);
    unlock_error = -EBUSY; relocks = 0;
    assert(rtl8367b_detect(&smi) == -EBUSY);
    assert(relocks == 0); /* no unlock succeeded */
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)
            (path/'test.c').write_text(stubs+'\n'+enum+'\n'+defines+'\n'+
                '\n'.join(function(source, name) for name in
                    ('rtl8367b_is_d', 'rtl8367b_init_regs', 'rtl8367b_detect'))+main)
            subprocess.run(['gcc', '-std=gnu11', '-Wall', '-Wextra', '-Werror',
                            '-Wno-unused-parameter', str(path/'test.c'), '-o', str(path/'test')],
                           check=True, capture_output=True)
            subprocess.run([str(path/'test')], check=True)


if __name__ == '__main__':
    unittest.main()
