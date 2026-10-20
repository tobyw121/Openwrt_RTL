#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Run the UART helper against fake sysfs/procfs; never touch host serial ports."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import BASE, shell_function

HELPER = BASE / 'usr/sbin/mw5-uart'


class UARTTests(unittest.TestCase):
    def run_helper(self, compatible, irq, command, *, direct_irq=False):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for path in ('proc/device-tree', 'proc/tty/driver', 'sys/class/tty/ttyS0/device'):
                (root / path).mkdir(parents=True)
            (root / 'proc/device-tree/compatible').write_bytes(compatible)
            (root / 'proc/tty/driver/serial').write_text(
                f'serinfo:1.0 driver revision:\n0: uart:16550A mmio:0x18147000 irq:{irq} tx:20 rx:10 oe:1\n')
            trigger = root / 'sys/class/tty/ttyS0/rx_trig_bytes'
            trigger.write_text('8\n')
            if direct_irq:
                (root / 'sys/class/tty/ttyS0/device/irq').write_text(f'{irq}\n')
            functions = '\n'.join(shell_function(HELPER, name) for name in
                                  ('is_rtl8197f', 'rx_trigger_paths', 'uart_irq', 'tune', 'status'))
            functions = functions.replace('/sys/', str(root / 'sys') + '/')
            functions = functions.replace('/proc/', str(root / 'proc') + '/')
            result = subprocess.run(['sh', '-eu', '-c', functions + '\n' + command],
                                    text=True, capture_output=True)
            return result, trigger.read_text()

    def test_all_rtl8197f_profiles_tune_rx_trigger(self):
        for board in ('xiaomi,r4-rd05', 'tenda,ac23', 'tenda,nova-mw5', 'tenda,lynx-rtl8197f'):
            with self.subTest(board=board):
                compat = board.encode() + b'\0realtek,rtl8197f-soc\0'
                result, trigger = self.run_helper(compat, 9, 'tune')
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(trigger, '1\n')

    def test_polling_is_detected_and_not_reported_as_successful_tuning(self):
        compat = b'xiaomi,r4-rd05\0realtek,rtl8197f-soc\0'
        result, trigger = self.run_helper(compat, 0, 'tune')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('UART is polling', result.stderr)
        self.assertEqual(trigger, '8\n')
        result, _ = self.run_helper(compat, 0, 'status')
        self.assertIn('irq=0\nreceive_mode=polling', result.stdout)

    def test_status_reads_sysfs_or_proc_irq(self):
        for direct in (False, True):
            with self.subTest(direct=direct):
                result, _ = self.run_helper(b'realtek,rtl8197f-soc\0', 9,
                                            'status', direct_irq=direct)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertIn('irq=9\nreceive_mode=interrupt', result.stdout)

    def test_unrelated_board_is_untouched(self):
        result, trigger = self.run_helper(b'unrelated,board\0', 9, 'tune')
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(trigger, '8\n')


if __name__ == '__main__':
    unittest.main()
