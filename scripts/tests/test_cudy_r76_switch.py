#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Run production DSA timing/isolation functions against a faulting regmap."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest
from test_rtl8367b_backport import ROOT, function

PATCHES = ROOT/'target/linux/realtek/patches-6.6'


def additions(path):
    return '\n'.join(line[1:] for line in path.read_text().splitlines()
                     if line.startswith('+') and not line.startswith('+++'))+'\n'


class R76SwitchTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which('gcc'), 'host compiler needed')
    def test_actual_timing_isolation_and_transport_failures(self):
        base = additions(PATCHES/'325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch')
        patch = PATCHES/'342-net-dsa-realtek-add-cudy-r76-measured-host-link.patch'
        # These two complete functions were introduced in patch 325 and are
        # unchanged by the intervening patches. Apply matching 342 hunks to
        # this exact source fragment, rather than duplicate its timing logic.
        fragment = function(base, 'rtl8367d_ext1_set_ssc')+'\n\n'+function(base, 'rtl8367d_ext1_config_rgmii')+'\n'
        fragment += ('\nstatic int rtl8365mb_ext_config_rgmii(struct realtek_priv *priv, int port,\n'
                     '\t\t\t\t      phy_interface_t interface)\n')
        for hunk in re.split(r'^@@[^\n]*\n', patch.read_text(), flags=re.M)[1:]:
            lines = hunk.splitlines(keepends=True)
            old = ''.join(line[1:] for line in lines if line.startswith((' ', '-'))
                          and not line.startswith('---'))
            new = ''.join(line[1:] for line in lines if line.startswith((' ', '+'))
                          and not line.startswith('+++'))
            if old and old in fragment:
                fragment = fragment.replace(old, new, 1)
        self.assertIn('bool r76 = rtl8367d_r76_board(priv);', fragment)
        self.assertIn('rtl8367d_ext1_set_ssc(priv, ssc)', fragment)
        fragment = '\n'.join(function(fragment, name) for name in
            ('rtl8367d_ext1_set_ssc', 'rtl8367d_r76_board', 'rtl8367d_ext1_config_rgmii'))
        added = additions(patch)
        defines = []
        lines = base.splitlines(keepends=True)
        i = 0
        while i < len(lines):
            line = lines[i]
            if line.startswith('#define RTL8367D_') or line.startswith('#define   RTL8367D_'):
                value = line
                while line.rstrip().endswith('\\'):
                    i += 1; line = lines[i]; value += line
                defines.append(value)
            i += 1
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
typedef uint32_t u32;
#define BIT(n) (1U << (n))
#define GENMASK(h,l) ((~0U << (l)) & (~0U >> (31-(h))))
#define FIELD_PREP(mask,v) (((u32)(v) << __builtin_ctz(mask)) & (mask))
#define READ_ONCE(v) (v)
#define min_t(t,a,b) ((t)(a) < (t)(b) ? (t)(a) : (t)(b))
#define dev_info(...) ((void)0)
#define RTL8365MB_EXT_RGMXF_REG(i) (0x1306+(i))
#define RTL8365MB_EXT_RGMXF_RXDELAY_MASK 7
#define RTL8365MB_EXT_RGMXF_TXDELAY_MASK 8
#define RTL8365MB_DIGITAL_INTERFACE_SELECT_REG0 0x1305
#define RTL8365MB_EXT_PORT_MODE_RGMII 1
#define RTL8365MB_PORT_ISOLATION_REG(p) (0x08a2+(p))
#define RTL8365MB_PORT_ISOLATION_MASK 0x07ff
#define RTL8365MB_CPU_PORT_MASK_REG 0x1219
#define RTL8365MB_CPU_PORT_MASK_MASK 0x07ff
#define RTL8365MB_SOURCE_PORT_PERMIT_REG 0x08c5
#define RTL8365MB_UNDA_FLOODING_PMSK_REG 0x0890
#define RTL8365MB_UNMCAST_FLOODING_PMSK_REG 0x0891
#define RTL8365MB_BCAST_FLOODING_PMSK_REG 0x0892
#define RTL8365MB_LOOKUP_HIT_ISO_ACT_REG 0x0a10
struct chip { bool rtl8367d; };
struct rtl8365mb { struct chip *chip_info; };
struct realtek_priv { void *chip_data, *dev, *map; };
static const char *board;
static bool of_machine_is_compatible(const char *s) { return !strcmp(board,s); }
static u32 regs[0x2000];
static int calls, fail_at, corrupt_isolation;
static int transaction(void) { return ++calls == fail_at ? -EIO : 0; }
static int regmap_write(void *m, u32 reg, u32 value)
{
    int rc=transaction(); if(rc) return rc;
    assert(reg < 0x2000); regs[reg]=value; return 0;
}
static int regmap_read(void *m, u32 reg, u32 *value)
{
    int rc=transaction(); if(rc) return rc;
    assert(reg < 0x2000); *value=regs[reg];
    if (corrupt_isolation && reg == RTL8365MB_PORT_ISOLATION_REG(2)) *value ^= 1;
    return 0;
}
static int regmap_update_bits(void *m, u32 reg, u32 mask, u32 value)
{
    int rc=transaction(); if(rc) return rc;
    assert(reg < 0x2000); regs[reg]=(regs[reg]&~mask)|(value&mask); return 0;
}
static int rtl8365mb_port_set_isolation(struct realtek_priv *p, int port, u32 mask)
{ return regmap_write(p->map, RTL8365MB_PORT_ISOLATION_REG(port), mask); }
static u32 rtl8367d_rd05_ext1_tx_delay, rtl8367d_rd05_ext1_rx_delay=7;
static u32 rtl8367d_rd05_ext1_txc_tap;
static bool rtl8367d_rd05_ext1_ssc;
'''+''.join(defines)+'\n'+fragment+'\n'+function(added, 'rtl8367d_r76_setup_cpu_path')+r'''
static void reset(void)
{
    memset(regs,0,sizeof(regs)); calls=0; fail_at=0; corrupt_isolation=0;
    regs[RTL8365MB_CPU_PORT_MASK_REG]=BIT(7);
    regs[RTL8367D_EXT_TXC_DLY_REG]=0x0104;
}
int main(void)
{
    struct chip chip={true}; struct rtl8365mb mb={&chip};
    struct realtek_priv p={.chip_data=&mb}; int total;
    board="cudy,ax1500-r76";
    reset(); assert(rtl8367d_ext1_config_rgmii(&p)==0); total=calls;
    assert(regs[0x1307]==8 && regs[0x13f9]==0x124);
    assert(regs[0x1d52]==0x2473 && regs[0x1d53]==1);
    assert(regs[0x1d54]==0x5fa && regs[0x1d55]==0xf);
    assert(regs[0x1d70]==0x2000 && regs[0x1d78]==0x1f);
    for (int n=1;n<=total;n++) {
        reset(); fail_at=n; assert(rtl8367d_ext1_config_rgmii(&p)==-EIO);
    }
    /* RD05 continues using its own existing runtime parameters. */
    board="xiaomi,r4-rd05";
    reset(); rtl8367d_rd05_ext1_rx_delay=5;
    assert(rtl8367d_ext1_config_rgmii(&p)==0);
    assert(regs[0x1307]==5 && regs[0x13f9]==0x104);
    assert(regs[0x1d52]==RTL8367D_EXT1_SSC_CTRL_DISABLE);
    reset(); assert(rtl8367d_r76_setup_cpu_path(&p)==0 && calls==0);
    board="tenda,nova-mw5";
    reset(); assert(rtl8367d_r76_setup_cpu_path(&p)==0 && calls==0);
    board="cudy,ax1500-r76"; chip.rtl8367d=false;
    reset(); assert(rtl8367d_r76_setup_cpu_path(&p)==0 && calls==0);
    chip.rtl8367d=true;
    reset(); assert(rtl8367d_r76_setup_cpu_path(&p)==0); total=calls;
    for (int i=0;i<4;i++) assert(regs[0x08a2+i]==0x80);
    for (int i=4;i<7;i++) assert(regs[0x08a2+i]==0);
    assert(regs[0x08a2+7]==0xf);
    assert(regs[0x07a8]==0 && regs[0x07a9]==0);
    assert(regs[0x08c5]==0x8f && regs[0x0a10]==0);
    for (int n=1;n<=total;n++) {
        reset(); fail_at=n; assert(rtl8367d_r76_setup_cpu_path(&p)==-EIO);
    }
    reset(); corrupt_isolation=1;
    assert(rtl8367d_r76_setup_cpu_path(&p)==-EIO);
    reset(); regs[RTL8365MB_CPU_PORT_MASK_REG]=BIT(6);
    assert(rtl8367d_r76_setup_cpu_path(&p)==-EIO);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)
            (path/'test.c').write_text(code)
            subprocess.run(['gcc', '-std=gnu11', '-Werror', '-fsanitize=undefined',
                            str(path/'test.c'), '-o', str(path/'test')], check=True)
            subprocess.run([str(path/'test')], check=True)


if __name__ == '__main__':
    unittest.main()
