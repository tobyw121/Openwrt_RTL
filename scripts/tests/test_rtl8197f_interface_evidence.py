#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Verify OEM mask proofs and preservation of adjacent extension fields."""
import importlib.util
import json
import lzma
import struct
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_mw5_stability import ROOT

SPEC = importlib.util.spec_from_file_location(
    'interface_evidence', ROOT / 'scripts/rtl8197f-oem-interface-evidence.py')
EVIDENCE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(EVIDENCE)
PATCH = ROOT / 'target/linux/realtek/patches-6.6/333-net-dsa-realtek-fix-extension-mode-mask.patch'


class InterfaceEvidenceTests(unittest.TestCase):
    def test_committed_evidence_reproduces_from_all_original_kernels(self):
        expected = json.loads((ROOT /
            'target/linux/realtek/rtl8197f/oem-interface-evidence.json').read_text())
        actual = EVIDENCE.collect(ROOT / 'reverse-engineering/oem-kernels')
        self.assertEqual(actual, expected)
        for board, data in actual['boards'].items():
            with self.subTest(board=board):
                self.assertEqual(len(data['extension_mode_proofs']), 1)
                self.assertEqual(data['extension_mode_proofs'][0]['register'], 0x1305)
        rd05 = actual['boards']['rd05']['functions']
        self.assertEqual(rd05['StopHW8812FE']['constant_return']['value'], 0)
        calls = {call['symbol'] for call in rd05['rtl_net80211_delkey']['direct_calls']}
        self.assertIn('CamResetAllEntry', calls)
        self.assertIn('CamDeleteOneEntry', calls)
        self.assertEqual(rd05['dal_rtl8367d_switch_init']['constant_return']['value'], 0)

    def test_unknown_instruction_in_mask_setup_is_not_followed(self):
        data = json.loads((ROOT /
            'target/linux/realtek/rtl8197f/oem-interface-evidence.json').read_text())['boards']['mw5']
        kernel = bytearray(lzma.decompress((ROOT /
            'reverse-engineering/oem-kernels/mw5/kernel.raw.xz').read_bytes()))
        proof = data['extension_mode_proofs'][0]
        # Replace li a0,0x1305 with a branch. A static call later in the
        # function must not be reported as proof for this setup anymore.
        struct.pack_into('<I', kernel, proof['pc'] - 0x80000000 + 4, 0x10000001)
        with self.assertRaisesRegex(ValueError, 'no exact'):
            EVIDENCE.mode_mask_proofs(kernel, 0x80000000,
                data['functions']['rtl8367c_setAsicPortExtMode'], proof['setter'])

    def test_patch_mode_mask_preserves_every_adjacent_register_bit(self):
        # Compile the actual patch's resulting macros, retaining context
        # and additions and excluding removed lines and patch metadata.
        lines = PATCH.read_text().split('@@', 2)[2].splitlines()[1:]
        source = '\n'.join(line[1:] for line in lines if line.startswith((' ', '+')))
        source = source[source.index('#define'):]
        source = source.split('#define   RTL8365MB_DIGITAL_INTERFACE_SELECT_MODE_OFFSET', 1)[0]
        # The unchanged value-offset macro is supplied as the boundary;
        # independent expected masks below catch the original one-bit shift.
        code = ('#include <assert.h>\n'
                '#define RTL8365MB_DIGITAL_INTERFACE_SELECT_MODE_OFFSET(id) (((id)%2)*4)\n'
                + source + r'''
int main(void)
{
    for (unsigned id=0;id<3;id++) {
        unsigned mask=RTL8365MB_DIGITAL_INTERFACE_SELECT_MODE_MASK(id);
        unsigned shift=RTL8365MB_DIGITAL_INTERFACE_SELECT_MODE_OFFSET(id);
        unsigned expected=id==1 ? 0xf0 : 0x0f;
        assert(mask==expected);
        for (unsigned old=0;old<=0xffff;old++) {
            for (unsigned mode=0;mode<16;mode++) {
                unsigned value=mode<<shift;
                unsigned updated=(old & ~mask) | (value & mask);
                assert(((updated>>shift)&15)==mode);
                assert((updated & ~expected)==(old & ~expected));
            }
        }
    }
    return 0;
}
''')
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'mode.c'
            exe = Path(tmp) / 'mode'
            src.write_text(code)
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
