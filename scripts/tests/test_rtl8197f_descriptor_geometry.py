#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Check real CPUIF stride encoding and padding against OEM geometry proof."""
import json
import hashlib
import lzma
from pathlib import Path
import re
import subprocess
import struct
import tempfile
import unittest

from test_mw5_stability import ROOT, c_function

DRIVER = ROOT / 'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'


class DescriptorGeometryTests(unittest.TestCase):
    def test_nic_evidence_matches_archived_original_kernel_bytes(self):
        evidence = json.loads((ROOT / 'target/linux/realtek/rtl8197f/oem-nic-evidence.json').read_text())
        for board, data in evidence['boards'].items():
            with self.subTest(board=board):
                path = ROOT / 'reverse-engineering/oem-kernels' / board / 'kernel.raw.xz'
                kernel = lzma.decompress(path.read_bytes())
                self.assertEqual(hashlib.sha256(kernel).hexdigest(), data['kernel_sha256'])
                for key in ('New_swNic_init', 'switchcore_reinit'):
                    item = data[key]
                    offset = item['address'] - 0x80000000
                    code = kernel[offset:offset + item['span']]
                    self.assertEqual(hashlib.sha256(code).hexdigest(), item['sha256'])
                for item in data['ring_stride_proofs'] + data['switchcore_reinit']['direct_calls']:
                    actual = struct.unpack_from('<I', kernel, item['pc'] - 0x80000000)[0]
                    self.assertEqual(actual, int(item['instruction_word'], 16))
                    if 'target' in item:
                        self.assertEqual(actual >> 26, 3)
                        target = ((item['pc'] + 4) & 0xf0000000) | ((actual & 0x3ffffff) << 2)
                        self.assertEqual(target, item['target'])

    def test_original_driver_cpuicr1_and_padding_for_both_oem_formats(self):
        source = DRIVER.read_text()
        constants = '\n'.join(re.findall(r'^#define RTL_RTK_CPUICR1[^\n]+', source, re.M))
        functions = c_function(DRIVER, 'static u32 rtl8197f_rtk_expected_cpuicr1(')
        functions += '\n' + c_function(DRIVER, 'static inline void\nrtl8197f_rtk_desc_clear_padding(')
        program = r'''
#include <assert.h>
#include <stdint.h>
typedef uint32_t u32;
#define BIT(n) (1U << (n))
CONSTANTS
struct rtl8197f_rtknet_priv { unsigned desc_dwords; };
struct rtl8197f_rtk_desc { u32 opts1,addr,opts2,opts3,opts4,opts5,opts6,opts7; };
static u32 rtl8197f_rtk_read(struct rtl8197f_rtknet_priv *p, u32 reg)
{ (void)p; assert(reg == RTL_RTK_CPUICR1); return 0xffffffff; }
FUNCTIONS
int main(void)
{
    struct rtl8197f_rtknet_priv p={6};
    struct rtl8197f_rtk_desc d={0,0,0,0,0,0,0x12345678,0x87654321};
    u32 masks=RTL_RTK_CPUICR1_TXDESC_MASK | RTL_RTK_CPUICR1_RXDESC_MASK;
    u32 native=rtl8197f_rtk_expected_cpuicr1(&p);
    assert((native & masks) == 0); /* Six dwords use the native zero encoding. */
    assert((native & RTL_RTK_CPUICR1_PKT_HDR_TYPE_MASK) ==
           RTL_RTK_CPUICR1_PKT_HDR_SHORTCUT_LSO);
    rtl8197f_rtk_desc_clear_padding(&p,&d);
    assert(d.opts6 == 0x12345678 && d.opts7 == 0x87654321);
    p.desc_dwords=8;
    u32 padded=rtl8197f_rtk_expected_cpuicr1(&p);
    assert((padded & RTL_RTK_CPUICR1_TXDESC_MASK) == (8U << 12));
    assert((padded & RTL_RTK_CPUICR1_RXDESC_MASK) == (8U << 18));
    assert((padded & ~masks) == native);
    rtl8197f_rtk_desc_clear_padding(&p,&d);
    assert(!d.opts6 && !d.opts7);
    return 0;
}
'''.replace('CONSTANTS', constants).replace('FUNCTIONS', functions)
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'geometry.c'
            exe = Path(tmp) / 'geometry'
            c.write_text(program)
            subprocess.run(['cc', '-std=gnu11', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            str(c), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)

    def test_machine_loop_steps_agree_with_board_word_and_byte_counts(self):
        evidence = json.loads((ROOT / 'target/linux/realtek/rtl8197f/oem-nic-evidence.json').read_text())
        for board, data in evidence['boards'].items():
            with self.subTest(board=board):
                stride = data['descriptor_stride_bytes']
                self.assertEqual(data['descriptor_dwords'] * 4, stride)
                self.assertGreaterEqual(len(data['ring_stride_proofs']), 2)
                for item in data['ring_stride_proofs']:
                    word = int(item['instruction_word'], 16)
                    self.assertEqual(word >> 26, 9)  # addiu
                    self.assertEqual((word >> 21) & 31, (word >> 16) & 31)
                    self.assertEqual(word & 0xffff, stride)


if __name__ == '__main__':
    unittest.main()
