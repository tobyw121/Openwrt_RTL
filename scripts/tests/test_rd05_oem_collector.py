"""Exercise the standalone collector against an offline OEM proc/sys fixture."""
from pathlib import Path
import os
import io
import gzip
import subprocess
import tarfile
import tempfile
import unittest
from test_mw5_stability import shell_function

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

    def invoke(self, root, out, env=None, extra=()):
        return subprocess.run([os.environ.get('COLLECTOR_TEST_SHELL', 'sh'), str(SCRIPT), '--root', str(root),
                               '--duration', '0', '--output', str(out), *extra],
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

    def test_deep_discovery_repeated_compressed_snapshots_and_exclusions(self):
        with tempfile.TemporaryDirectory() as tmp:
            root, out, _ = self.fixture(Path(tmp))
            for name, value in {
                'proc/wlan0/stats': 'rx=123 tx=456\n',
                'proc/wlan0/rf_finetune': 'rfe=0\n',
                'proc/rtl865x/new_vendor_state': 'state=ready\n',
                'proc/wlan0/mib_gkeytbl': 'FORBIDDEN_WIFI_KEY',
                'proc/wlan0/reset_stats': 'FORBIDDEN_RESET',
                'proc/rtl865x/memory': 'FORBIDDEN_MMIO_ENDPOINT',
            }.items():
                p = root / name
                p.parent.mkdir(parents=True, exist_ok=True)
                p.write_text(value)
            result = self.invoke(root, out, extra=['--deep'])
            self.assertEqual(result.returncode, 0, result.stderr)
            with tarfile.open(next(out.glob('*.tar.gz'))) as t:
                regular = {m.name.split('/', 1)[1]: t.extractfile(m).read()
                           for m in t if m.isfile()}
            decisions = regular['inventory/realtek-decisions.tsv'].decode()
            self.assertIn('inventory-only-unverified-callback\t/proc/wlan0/stats', decisions)
            self.assertIn('excluded-command-or-sensitive\t/proc/wlan0/mib_gkeytbl', decisions)
            samples = [data for name, data in regular.items()
                       if name.startswith('live/') and name.endswith('.tar.gz')]
            self.assertEqual(len(samples), 2)
            for data in samples:
                with tarfile.open(fileobj=io.BytesIO(data), mode='r:gz') as t:
                    snapshot = {m.name.removeprefix('./'): t.extractfile(m).read()
                                for m in t if m.isfile()}
                self.assertNotIn('files/proc/wlan0/stats', snapshot)
                self.assertNotIn('files/proc/rtl865x/new_vendor_state', snapshot)
                self.assertFalse(any(b'FORBIDDEN_' in v for v in snapshot.values()))
            self.assertFalse(any(name.startswith('live-raw-') for name in regular))

    def test_observation_spans_full_minute_even_with_slow_reads(self):
        # Execute the actual scheduler with a monotonic clock and sleep mock.
        # No wall-clock minute is needed to verify boundary/final snapshots.
        source = shell_function(SCRIPT, 'observe')
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            for cost in (0, 2, 17, 65):
                work = base / str(cost)
                work.mkdir()
                clock = work / 'clock'
                clock.write_text('0\n')
                script = r'''
BASE_WORK=$1
CLOCK=$BASE_WORK/clock
COST=$2
DURATION=60
INTERVAL=5
monotonic() { cat "$CLOCK"; }
sleep() { n=$(cat "$CLOCK"); echo "$((n + $1))" > "$CLOCK"; }
start_capture() { :; }
stop_capture() { :; }
live_snapshot() {
    n=$(cat "$CLOCK")
    printf '%s %s\n' "$1" "$n" >> "$BASE_WORK/calls"
    echo "$((n + COST))" > "$CLOCK"
}
''' + source + '\nobserve\n'
                result = subprocess.run([os.environ.get('COLLECTOR_TEST_SHELL', 'sh'),
                                         '-c', script, 'sh', str(work), str(cost)],
                                        capture_output=True, text=True, timeout=10)
                self.assertEqual(result.returncode, 0, result.stderr)
                starts = [int(line.split()[1]) for line in (work/'calls').read_text().splitlines()]
                self.assertEqual(starts[0], 0)
                self.assertGreaterEqual(starts[-1], 60)
                self.assertGreaterEqual(len(starts), 2)
                self.assertIn('requested_seconds=60', (work/'live/window.txt').read_text())


if __name__ == '__main__':
    unittest.main()
