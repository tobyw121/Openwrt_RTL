#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise name/address/marker validation with synthetic OEM kernel tables."""
import importlib.util
from pathlib import Path
import struct
import unittest


SCRIPT = Path(__file__).resolve().parents[1] / 'rtl8197f-oem-kallsyms.py'
SPEC = importlib.util.spec_from_file_location('oem_kallsyms', SCRIPT)
DECODER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(DECODER)


def fixture(padding=0, exported=False):
    count = 1024
    blob = bytearray(0x100)
    addresses = [0x80000000 + 4 * i for i in range(count)]
    blob.extend(struct.pack('<%dI' % count, *addresses))
    blob.extend(b'\0' * padding)
    blob.extend(struct.pack('<I', count))
    blob.extend(b'\0' * 12)
    names_start = len(blob)
    alphabet = list(dict.fromkeys('T_sym0123456789textkrabpo'))
    tokens = [v.encode() for v in alphabet]
    tokens += [('unused%d' % i).encode() for i in range(256 - len(tokens))]
    encode = {v: i for i, v in enumerate(tokens)}
    markers = []
    for i in range(count):
        if not i % 256:
            markers.append(len(blob) - names_start)
        name = 'T' + ('_text' if i == 0 else 'sym%04d' % i)
        if exported and i in (10, 20, 30):
            name = 'T' + {10: 'test_export', 20: '__ksymtab_test_export',
                          30: '__kstrtab_test_export'}[i]
        encoded = bytes(encode[c.encode()] for c in name)
        blob.append(len(encoded))
        blob.extend(encoded)
    blob.extend(b'\0' * (-len(blob) % 16))
    marker_start = len(blob)
    blob.extend(struct.pack('<%dI' % len(markers), *markers))
    blob.extend(b'\0' * (-len(blob) % 16))
    offsets = []
    table_start = len(blob)
    for token in tokens:
        offsets.append(len(blob) - table_start)
        blob.extend(token + b'\0')
    blob.extend(b'\0' * (-len(blob) % 16))
    blob.extend(struct.pack('<256H', *offsets))
    if exported:
        struct.pack_into('<II', blob, 20 * 4, addresses[10], addresses[30])
        blob[30 * 4:30 * 4 + 12] = b'test_export\0'
    return blob, names_start, marker_start


class KallsymsTests(unittest.TestCase):
    def test_reconstructs_symbols_with_both_linker_padding_variants(self):
        for padding in (0, 12):
            with self.subTest(padding=padding):
                blob, names, markers = fixture(padding)
                result = DECODER.recover(blob)
                self.assertEqual(result['symbol_count'], 1024)
                self.assertEqual(result['offsets']['names'], names)
                self.assertEqual(result['offsets']['markers'], markers)
                self.assertEqual(result['symbols'][0],
                                 {'name': '_text', 'type': 'T', 'address': 0x80000000})
                self.assertEqual(result['symbols'][-1]['name'], 'sym1023')
                self.assertEqual(result['symbols'][-1]['address'], 0x80000ffc)

    def test_corrupt_names_markers_and_address_order_fail_closed(self):
        for damage in ('name', 'marker', 'address'):
            with self.subTest(damage=damage):
                blob, names, markers = fixture()
                if damage == 'name':
                    blob[names] = 0
                elif damage == 'marker':
                    blob[markers + 4] ^= 1
                else:
                    struct.pack_into('<I', blob, 0x100 + 500 * 4, 0x80000001)
                with self.assertRaisesRegex(ValueError, 'validated kallsyms'):
                    DECODER.recover(blob)

    def test_export_records_independently_validate_function_and_name_pointers(self):
        blob, _, _ = fixture(exported=True)
        self.assertEqual(DECODER.recover(blob)['export_cross_checks'], 1)
        blob[20 * 4] ^= 1
        with self.assertRaisesRegex(ValueError, 'cross-check failed'):
            DECODER.recover(blob)

    def test_random_or_truncated_input_is_rejected(self):
        for blob in (b'', b'\xff' * 16384, fixture()[0][:-300]):
            with self.assertRaises(ValueError):
                DECODER.recover(blob)


if __name__ == '__main__':
    unittest.main()
