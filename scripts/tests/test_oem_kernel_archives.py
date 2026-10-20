#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
import importlib.util
import json
import lzma
from pathlib import Path
import struct
import tempfile
import unittest


def module(name):
    path = Path(__file__).resolve().parents[1] / (name + '.py')
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


ARCHIVE = module('rtl8197f-archive-oem-kernel')
VERIFY = module('verify-oem-kernel-archives')


class KernelArchiveTests(unittest.TestCase):
    def test_lzma_extraction_stops_before_following_flash_partitions(self):
        raw = struct.pack('<4I', 0, 1, 2, 3)
        stream = lzma.compress(raw, format=lzma.FORMAT_ALONE)
        flash = b'prefix' + stream + b'private rootfs/factory not included'
        self.assertEqual(ARCHIVE.unpack_kernel(flash, 6), (raw, len(stream)))
        with self.assertRaises(ValueError):
            ARCHIVE.unpack_kernel(stream[:12], 0)
        with self.assertRaises(ValueError):
            ARCHIVE.unpack_kernel(lzma.compress(b'abc', format=lzma.FORMAT_ALONE), 0)

    def test_disassembly_rejects_missing_zero_words_overlap_and_wrong_endian(self):
        raw = struct.pack('<2I', 0, 0x03e00008)
        good = '80000000: 00000000 nop\n80000004: 03e00008 jr ra\n'
        self.assertIn('<entry>', ARCHIVE.annotate_disassembly(
            good, raw, 0x80000000, 0x80000008, 0x80000000, {0x80000000: ['entry']}))
        for bad in (good.splitlines()[1], good + good.splitlines()[1],
                    good.replace('03e00008', '0800e003'), good.splitlines()[0]):
            with self.subTest(bad=bad), self.assertRaises(ValueError):
                ARCHIVE.annotate_disassembly(bad, raw, 0x80000000,
                                             0x80000008, 0x80000000, {})

    def test_factory_identifier_and_private_key_matches_block_archival(self):
        flash = bytearray(0x30000)
        identifier = b'TENDA.lan_mac=123456789abc\0'
        flash[0x20000:0x20000 + len(identifier)] = identifier
        self.assertEqual(ARCHIVE.privacy_check(flash, b'generic kernel', 'mw5')['matches'], 0)
        for raw in (b'123456789abc', bytes.fromhex('123456789abc'),
                    b'-----BEGIN RSA PRIVATE KEY-----'):
            with self.assertRaisesRegex(ValueError, 'privacy'):
                ARCHIVE.privacy_check(flash, raw, 'mw5')

    def test_persisted_archive_verifier_checks_bytes_even_with_updated_text_hashes(self):
        raw = struct.pack('<2I', 0, 0x03e00008)
        text = b'80000000: 00000000  nop\n80000004: 03e00008  jr ra\n'
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            manifest = {'board': 'fixture', 'image_base': 0x80000000,
                        'kernel_size': 8, 'kernel_sha256': VERIFY.sha(raw),
                        'symbol_count': 1, 'artifacts': {}, 'disassembly_chunks': [
                            {'path': 'part.dis.xz', 'start': 0x80000000,
                             'stop': 0x80000008, 'words': 2}]}

            def save(name, data):
                packed = lzma.compress(data)
                (root / name).write_bytes(packed)
                manifest['artifacts'][name] = {
                    'size': len(packed), 'sha256': VERIFY.sha(packed),
                    'unpacked_size': len(data), 'unpacked_sha256': VERIFY.sha(data)}
                (root / 'manifest.json').write_text(json.dumps(manifest))

            save('kernel.raw.xz', raw)
            save('symbols.tsv.xz', b'address\ttype\tname\n80000000\tT\tentry\n')
            save('text-symbol-spans.tsv.xz', ('start\tstop\tsize\tsha256\tnames\n'
                '80000000\t80000008\t8\t%s\tentry\n' % VERIFY.sha(raw)).encode())
            save('part.dis.xz', text)
            self.assertEqual(VERIFY.verify(root)['symbol_count'], 1)
            save('part.dis.xz', text.replace(b'03e00008', b'03e00009'))
            with self.assertRaisesRegex(ValueError, 'differs'):
                VERIFY.verify(root)
            save('part.dis.xz', text)
            manifest['disassembly_chunks'][0]['start'] += 4
            (root / 'manifest.json').write_text(json.dumps(manifest))
            with self.assertRaisesRegex(ValueError, 'gap or overlap'):
                VERIFY.verify(root)


if __name__ == '__main__':
    unittest.main()
