#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise MIPS argument decoding with genuine instruction encodings."""
import importlib.util
from pathlib import Path
import struct
import unittest


PATH = Path(__file__).resolve().parents[1] / 'rtl8197f-oem-switch-audit.py'
SPEC = importlib.util.spec_from_file_location('switch_audit', PATH)
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)


def fixture(words, target='rtl8367c_setAsicReg'):
    blob = struct.pack('<%dI' % len(words), *words) + b'\0' * 8
    metadata = {'image_base': 0x80000000, 'symbols': [
        {'name': 'RTL83XX_init', 'address': 0x80000000, 'type': 'T'},
        {'name': target, 'address': 0x80000000 + len(words) * 4, 'type': 'T'},
    ]}
    return blob, metadata


class SwitchAuditTests(unittest.TestCase):
    def test_register_arguments_in_either_order_and_delay_slot(self):
        # jal 0x80000018; each delay-slot argument is part of its call.
        words = [0x2404121f, 0x0c000006, 0x240501d0,
                 0x34050222, 0x0c000006, 0x24041b03]
        blob, metadata = fixture(words)
        item = AUDIT.audit_functions(blob, metadata, ['RTL83XX_init'])['RTL83XX_init']
        self.assertEqual([c['arguments'] for c in item['literal_call_sites']],
                         [[0x121f, 0x1d0], [0x1b03, 0x222]])
        self.assertEqual(item['direct_calls'][1]['pc'], 0x80000010)
        self.assertEqual(item['direct_calls'][0]['target'], 0x80000018)

    def test_rgmii_three_arguments_and_unknown_values_are_not_guessed(self):
        words = [0x24040010, 0x00002821, 0x0c000008, 0x24060005,
                 0x8c440000, 0x00002825, 0x0c000008, 0x24060002]
        blob, metadata = fixture(words, 'rtk_port_rgmiiDelayExt_set')
        item = AUDIT.audit_functions(blob, metadata, ['RTL83XX_init'])['RTL83XX_init']
        self.assertEqual(len(item['direct_calls']), 2)
        self.assertEqual(len(item['literal_call_sites']), 1)
        self.assertEqual(item['literal_call_sites'][0]['arguments'], [16, 0, 5])
        # Unknown loads, unsigned immediates and sign extension differ.
        self.assertIsNone(AUDIT.literal(0x8c440000))
        self.assertEqual(AUDIT.literal(0x3405ffff), (5, 65535))
        self.assertEqual(AUDIT.literal(0x2405ffff), (5, 0xffffffff))
        self.assertIsNone(AUDIT.literal(0x00002861))  # reserved shamt in addu

    def test_success_stub_is_exact_and_missing_delay_slot_is_rejected(self):
        blob, metadata = fixture([0x03e00008, 0x00001025])
        item = AUDIT.audit_functions(blob, metadata, ['RTL83XX_init'])['RTL83XX_init']
        self.assertEqual(item['constant_return']['value'], 0)
        for words in ([0x03e00008, 0x24020001],
                      [0x03e00008, 0x00001025, 0x24040001],
                      [0x2404121f, 0x0c000002]):
            blob, metadata = fixture(words)
            item = AUDIT.audit_functions(blob, metadata, ['RTL83XX_init'])['RTL83XX_init']
            self.assertNotIn('constant_return', item)
            self.assertFalse(item['literal_call_sites'])

    def test_non_text_and_truncated_ranges_are_rejected(self):
        blob, metadata = fixture([0x03e00008, 0x00001025])
        metadata['symbols'][0]['type'] = 'D'
        with self.assertRaisesRegex(ValueError, 'text symbol'):
            AUDIT.audit_functions(blob, metadata, ['RTL83XX_init'])
        metadata['symbols'][0]['type'] = 'T'
        with self.assertRaisesRegex(ValueError, 'range'):
            AUDIT.audit_functions(blob[:4], metadata, ['RTL83XX_init'])
        self.assertEqual(AUDIT.audit_functions(blob, metadata, ['absent']), {})


if __name__ == '__main__':
    unittest.main()
