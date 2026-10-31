import importlib.util
from pathlib import Path
import struct
import unittest
import zlib
from test_mw5_stability import ROOT

spec = importlib.util.spec_from_file_location('cudy_full', ROOT/'scripts/rtl8197f-cudy-fullflash.py')
builder = importlib.util.module_from_spec(spec)
spec.loader.exec_module(builder)


class FullflashTests(unittest.TestCase):
    def test_container_and_preservation(self):
        path = ROOT/'fullflash-templates/Cudy_AX1500.bin'
        if not path.exists():
            self.skipTest('private template unavailable')
        original = path.read_bytes()
        fs = bytearray(128)
        fs[:4] = b'hsqs'
        struct.pack_into('<HH', fs, 28, 4, 0)
        struct.pack_into('<Q', fs, 40, 128)
        for loader in (b'odd', b'even'):
            out, manifest = builder.build(original, loader, fs)
            self.assertEqual(len(out), 0x1000000)
            self.assertEqual(out[:0x40000], original[:0x40000])
            self.assertEqual(out[0xf00000:], original[0xf00000:])
            self.assertEqual(out[0x440000:0x440080], fs)
            h = bytearray(out[0x40000:0x40040])
            crc = struct.unpack_from('>I', h, 4)[0]
            h[4:8] = bytes(4)
            self.assertEqual(zlib.crc32(h), crc)
            length = struct.unpack_from('>I', h, 12)[0]
            payload = out[0x40040:0x40040+length]
            self.assertEqual(zlib.crc32(payload), struct.unpack_from('>I', h, 24)[0])
            self.assertEqual(payload[:4], b'cr6c')
            self.assertEqual(sum(struct.unpack('>'+str((len(payload)-16)//2)+'H', payload[16:])) & 65535, 0)
            self.assertEqual(manifest['entry'], 0x82000000)
        for loader, rootfs, template in [(b'', fs, original), (bytes(0x400000), fs, original),
                                         (b'x', b'bad', original), (b'x', fs, bytes(0x1000000)),
                                         (b'x', fs+bytes(0xac0000), original)]:
            with self.assertRaises(ValueError):
                builder.build(template, loader, rootfs)

if __name__ == '__main__':
    unittest.main()
