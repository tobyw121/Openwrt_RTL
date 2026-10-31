"""Exercise the standalone collector against an offline OEM proc/sys fixture."""
from pathlib import Path
import os
import subprocess
import tarfile
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / 'scripts/rd05-oem-collect.sh'


class RD05OEMCollectorTests(unittest.TestCase):
    def fixture(self, base):
        root = base / 'router'
        files = {
            'proc/cpuinfo': 'system type: RTL8197FH\n',
            'proc/net/dev': 'eth1: 100 2 0 0\n',
            'proc/interrupts': '21: 50 PCI\n',
            'proc/rtl865x/txRing': 'CPU owned descriptor\n',
            'proc/rtl865x/rxRing': 'R' * 379227,
            'sys/bus/pci/devices/0000:00:00.0/vendor': '0x10ec\n',
            'sys/bus/pci/devices/0000:00:00.0/device': '0xf812\n',
            'sys/class/gpio/gpiochip0/label': 'rtl8197f-gpio\n',
            'sys/class/gpio/gpio33/direction': 'out\n',
            'sys/class/gpio/gpio33/value': '1\n',
            'sys/class/net/eth1/address': 'a4:ba:70:1b:d4:10\n',
            # These are deliberately present but never eligible for collection.
            'dev/mtd0': 'FORBIDDEN_FLASH_CONTENT',
            'proc/mtd': 'FORBIDDEN_MTD_HEADER',
            'sys/bus/pci/devices/0000:00:00.0/config': 'FORBIDDEN_PCI_CONFIG',
            'sys/bus/pci/devices/0000:00:00.0/resource0': 'FORBIDDEN_BAR_MEMORY',
            'sys/bus/pci/devices/0000:00:00.0/eeprom': 'FORBIDDEN_EEPROM',
            'sys/kernel/debug/reset': 'FORBIDDEN_DEBUG_RESET',
            'etc/config/wireless': 'FORBIDDEN_WIFI_KEY',
        }
        for name, value in files.items():
            p = root / name
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(value)
        out = base / 'output with spaces'
        out.mkdir()
        return root, out, files

    def invoke(self, root, out, env=None):
        return subprocess.run([os.environ.get('COLLECTOR_TEST_SHELL', 'sh'), str(SCRIPT), '--root', str(root),
                               '--duration', '0', '--output', str(out)],
                              capture_output=True, text=True, timeout=35, env=env)

    def test_inventory_no_flash_or_configuration_and_originals_preserved(self):
        with tempfile.TemporaryDirectory() as tmp:
            root, out, files = self.fixture(Path(tmp))
            result = self.invoke(root, out)
            self.assertEqual(result.returncode, 0, result.stderr)
            archive = next(out.glob('*.tar.gz'))
            with tarfile.open(archive) as t:
                regular = {m.name.split('/', 1)[1]: t.extractfile(m).read()
                           for m in t if m.isfile()}
            self.assertEqual(regular['files/sys/bus/pci/devices/0000:00:00.0/device'], b'0xf812\n')
            self.assertEqual(len(regular['realtek/before/rxRing.txt']), 379227)
            self.assertEqual(regular['realtek/before/rxRing.txt'], regular['realtek/after/rxRing.txt'])
            self.assertEqual(regular['files/sys/class/gpio/gpio33/value'], b'1\n')
            for data in regular.values():
                self.assertNotIn(b'FORBIDDEN_', data)
            self.assertFalse(any(name.startswith('commands/') for name in regular))
            for name, value in files.items():
                self.assertEqual((root / name).read_text(), value)

    def test_stalled_reader_is_bounded_and_reported(self):
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            root, out, _ = self.fixture(base)
            mock = base / 'bin'
            mock.mkdir()
            dd = mock / 'dd'
            dd.write_text('#!/bin/sh\ncase "$*" in *if=*/proc/cpuinfo*) exec sleep 20 ;; esac\nexec /usr/bin/dd "$@"\n')
            dd.chmod(0o755)
            env = dict(os.environ, PATH=str(mock) + ':' + os.environ['PATH'])
            result = self.invoke(root, out, env)
            self.assertEqual(result.returncode, 0, result.stderr)
            status = next(out.glob('rd05-oem-collect.*/status.tsv')).read_text()
            self.assertIn('timeout\t', status)
            self.assertIn('files/proc/cpuinfo', status)

    def test_offline_mode_refuses_live_operations_and_invalid_options(self):
        with tempfile.TemporaryDirectory() as tmp:
            root, out, _ = self.fixture(Path(tmp))
            for extra in (['--rd05-registers'], ['--capture', 'eth1'], ['--duration', '121']):
                result = subprocess.run(['sh', str(SCRIPT), '--root', str(root),
                                         '--output', str(out), '--duration', '0'] + extra,
                                        capture_output=True, text=True)
                self.assertNotEqual(result.returncode, 0)
            self.assertEqual(list(out.iterdir()), [])


if __name__ == '__main__':
    unittest.main()
