#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Verify RD05 length-profile evidence and execute the patched DSA callbacks."""
import hashlib
import json
import lzma
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

from test_mw5_stability import ROOT, c_function

PATCH = ROOT / 'target/linux/realtek/patches-6.6/334-net-dsa-realtek-rd05-select-mtu-profile0.patch'


class MtuProfileTests(unittest.TestCase):
    def test_oem_profile_words_match_complete_kernel_archive(self):
        data = json.loads((ROOT / 'target/linux/realtek/rtl8197f/oem-interface-evidence.json').read_text())
        item = data['boards']['rd05']
        kernel = lzma.decompress((ROOT / 'reverse-engineering/oem-kernels/rd05/kernel.raw.xz').read_bytes())
        self.assertEqual(hashlib.sha256(kernel).hexdigest(), item['kernel_sha256'])
        for name in ('rtk_switch_maxPktLen_set', 'dal_rtl8367d_switch_portMaxPktLen_set',
                     'dal_rtl8367d_switch_maxPktLenCfg_set'):
            function = item['functions'][name]
            words = [int(w, 16) for w in function['instruction_words']]
            code = struct.pack('<%dI' % len(words), *words)
            start = function['address'] - 0x80000000
            self.assertEqual(code, kernel[start:start + function['span']])
            self.assertEqual(hashlib.sha256(code).hexdigest(), function['span_sha256'])
        selector = item['functions']['dal_rtl8367d_switch_portMaxPktLen_set']['instruction_words']
        self.assertIn('0x001028c0', selector)  # sll a1,s0,3 (speed * 8)
        self.assertIn('0x24040883', selector)  # selector register in call delay slot
        length = item['functions']['dal_rtl8367d_switch_maxPktLenCfg_set']['instruction_words']
        self.assertIn('0x2604088c', length)  # CFG0 + cfgId
        self.assertIn('0x24053fff', length)  # 14-bit maximum length mask

    def test_actual_callbacks_preserve_unused_ports_speeds_and_errors(self):
        added = '\n'.join(line[1:] for line in PATCH.read_text().splitlines()
                          if line.startswith('+') and not line.startswith('+++'))
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
typedef uint32_t u32;
#define BIT(p) (1U << (p))
#define VLAN_ETH_HLEN 18
#define ETH_FCS_LEN 4
#define RTL8365MB_CFG0_MAX_LEN_REG 0x088c
#define RTL8365MB_CFG0_MAX_LEN_MASK 0x3fff
#define FIELD_PREP(mask,v) ((v)&(mask))
#define dev_dbg(...) do {} while(0)
struct info { bool rtl8367d; };
struct rtl8365mb { struct info *chip_info; };
struct dsa_switch { void *priv; unsigned num_ports, users, cpu, dsa; };
struct realtek_priv { struct rtl8365mb *chip_data; struct dsa_switch *ds; void *map; };
static int dsa_is_user_port(struct dsa_switch *ds,int p) { return !!(ds->users & BIT(p)); }
static int dsa_is_cpu_port(struct dsa_switch *ds,int p) { return !!(ds->cpu & BIT(p)); }
static int dsa_is_dsa_port(struct dsa_switch *ds,int p) { return !!(ds->dsa & BIT(p)); }
static unsigned cfg0, selector, calls, addresses[2];
static int fail_at=-1;
static int regmap_update_bits(void *map,unsigned reg,unsigned mask,unsigned value)
{
    (void)map;assert(calls<2);addresses[calls]=reg;
    if ((int)calls++==fail_at) return -5;
    assert(reg==0x088c || reg==0x0883);
    unsigned *target=reg==0x088c ? &cfg0 : &selector;
    *target=(*target & ~mask) | (value & mask);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'mtu.c'
            exe = Path(tmp) / 'mtu'
            src.write_text(added)
            code += c_function(src, 'static int rtl8367d_mtu_select_cfg0(') + '\n'
            code += c_function(src, 'static int rtl8365mb_port_change_mtu(') + '\n'
            code += r'''
int main(void)
{
    struct info info={true};struct rtl8365mb chip={&info};
    struct dsa_switch ds={.num_ports=8,.users=7,.cpu=128};
    struct realtek_priv priv={&chip,&ds,0};ds.priv=&priv;
    cfg0=0xc000;selector=0xffff;
    assert(!rtl8365mb_port_change_mtu(&ds,7,1500));
    assert(calls==2 && addresses[0]==0x088c && addresses[1]==0x0883);
    assert(cfg0==(0xc000|1522) && selector==0x7878);
    calls=0;
    assert(!rtl8365mb_port_change_mtu(&ds,1,1500) && !calls);
    info.rtl8367d=false;selector=0xffff;
    assert(!rtl8365mb_port_change_mtu(&ds,7,1400));
    assert(calls==1 && selector==0xffff && cfg0==(0xc000|1422));
    info.rtl8367d=true;
    for (fail_at=0;fail_at<2;fail_at++) {
        calls=0;selector=0xffff;
        assert(rtl8365mb_port_change_mtu(&ds,7,1500)==-5);
        assert(calls==(unsigned)fail_at+1 && selector==0xffff);
    }
    fail_at=-1;calls=0;ds.users=ds.cpu=0;
    assert(!rtl8367d_mtu_select_cfg0(&priv) && !calls);
    ds.dsa=64;selector=0xffff;
    assert(!rtl8367d_mtu_select_cfg0(&priv) && calls==1 && selector==0xbfbf);
    return 0;
}
'''
            src.write_text(code)
            subprocess.run(['cc', '-std=gnu11', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
