#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Execute the patched flow-control helper against verified binary evidence."""
import json
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

from test_mw5_stability import ROOT, c_function


PATCH = ROOT / 'target/linux/realtek/patches-6.6/332-net-dsa-realtek-mw5-restore-oem-flow-control.patch'
EVIDENCE = ROOT / 'target/linux/realtek/rtl8197f/oem-switch-evidence.json'


class OemFlowControlTests(unittest.TestCase):
    def test_evidence_records_real_mips_call_arguments(self):
        data = json.loads(EVIDENCE.read_text())
        writes = data['mw5_flowctrl']['writes']
        self.assertEqual(len(writes), 8)
        for i, item in enumerate(writes):
            with self.subTest(register=item['register']):
                words = [int(v, 16) for v in item['instruction_words']]
                self.assertEqual(words[0] >> 16, 0x2404)  # li a0, register
                self.assertEqual(words[2] >> 16, 0x2405)  # delay slot: li a1, value
                self.assertEqual(words[0] & 0xffff, item['register'])
                self.assertEqual(words[2] & 0xffff, item['value'])
                self.assertEqual(words[1] >> 26, 3)  # jal
                target = ((item['call_pc'] + 4) & 0xf0000000) | ((words[1] & 0x3ffffff) << 2)
                self.assertEqual(target, data['boards']['mw5']['selected_symbols']
                                 ['rtl8367c_setAsicReg']['address'])
                self.assertEqual(item['register'], 0x121f + i)

    def test_exact_write_order_board_revision_gates_and_errors(self):
        evidence = json.loads(EVIDENCE.read_text())['mw5_flowctrl']['writes']
        added = '\n'.join(line[1:] for line in PATCH.read_text().splitlines()
                          if line.startswith('+') and not line.startswith('+++'))
        expected = ','.join('{%d,%d}' % (item['register'], item['value']) for item in evidence)
        preamble = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
struct node { bool enabled; };
struct device { struct node *of_node; };
struct rtl8365mb { unsigned detected_chip_id, detected_chip_ver; };
struct realtek_priv { struct rtl8365mb *chip_data; struct device *dev; void *map; };
struct rtl8365mb_jam_tbl_entry { unsigned reg, val; };
static unsigned writes, messages;
static int fail_at;
static const unsigned expected[][2] = {EXPECTED};
static bool of_property_read_bool(struct node *node, const char *key)
{
    assert(!strcmp(key,"realtek,rtl8197f-mw5-oem-init"));
    return node->enabled;
}
static int regmap_write(void *map, unsigned reg, unsigned value)
{
    (void)map;
    assert(writes<8 && reg==expected[writes][0] && value==expected[writes][1]);
    return (int)writes++==fail_at ? -5 : 0;
}
#define dev_info(...) do { messages++; } while (0)
'''.replace('EXPECTED', expected)
        main = r'''
int main(void)
{
    struct node node={true};
    struct device dev={&node};
    struct rtl8365mb mb={0x6367,0x0060};
    struct realtek_priv priv={&mb,&dev,0};
    fail_at=-1;
    assert(rtl8365mb_mw5_oem_flowctrl_init(&priv)==0);
    assert(writes==8 && messages==1);
    writes=messages=0;node.enabled=false;
    assert(rtl8365mb_mw5_oem_flowctrl_init(&priv)==0 && !writes && !messages);
    node.enabled=true;
    unsigned ids[]={0x0801,0x6511,0x6642,0x6367};
    unsigned revisions[]={0x0060,0x0080,0x0030,0x00a0};
    for(unsigned i=0;i<4;i++) {
        mb.detected_chip_id=ids[i];mb.detected_chip_ver=revisions[i];
        assert(rtl8365mb_mw5_oem_flowctrl_init(&priv)==0 && !writes && !messages);
    }
    mb.detected_chip_id=0x6367;mb.detected_chip_ver=0x0060;
    for(fail_at=0;fail_at<8;fail_at++) {
        writes=messages=0;
        assert(rtl8365mb_mw5_oem_flowctrl_init(&priv)==-5);
        assert(writes==(unsigned)fail_at+1 && !messages);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'flowctrl.c'
            src.write_text(added)
            helper = c_function(src, 'static int rtl8365mb_mw5_oem_flowctrl_init(')
            src.write_text(preamble + helper + main)
            exe = Path(tmp) / 'flowctrl'
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
