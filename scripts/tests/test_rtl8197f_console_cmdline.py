"""The board DT, not a forced shared 115200 command line, owns the console."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT


class ConsoleTests(unittest.TestCase):
    def test_no_builtin_override_or_append(self):
        config = (ROOT/'target/linux/realtek/rtl8197f/config-6.6').read_text().splitlines()
        self.assertNotIn('CONFIG_CMDLINE_OVERRIDE=y', config)
        self.assertNotIn('CONFIG_CMDLINE_BOOL=y', config)
        self.assertIn('CONFIG_CMDLINE=""', config)
        self.assertIn('CONFIG_MIPS_CMDLINE_FROM_DTB=y', config)
        self.assertNotIn('CONFIG_MIPS_CMDLINE_BUILTIN_EXTEND=y', config)

    def test_effective_board_console_and_root_arguments(self):
        dtc = os.environ.get('DTC') or shutil.which('dtc')
        headers = os.environ.get('RTL8197F_KERNEL_INCLUDE')
        if not dtc or not headers:
            self.skipTest('DTC and RTL8197F_KERNEL_INCLUDE required')
        dts_dir = ROOT/'target/linux/realtek/dts'
        for board, baud in [('cudy_wr1500', 38400), ('cudy_ax1500-r76', 38400),
                            ('cudy_ax1500-r76-fullflash', 38400),
                            ('tenda_nova-mw5', 115200), ('xiaomi_r4-rd05', 115200)]:
            with self.subTest(board=board), tempfile.TemporaryDirectory() as tmp:
                data = subprocess.check_output(['cpp', '-nostdinc', '-undef', '-D__DTS__',
                    '-x', 'assembler-with-cpp', '-I', str(dts_dir), '-I', headers,
                    str(dts_dir/f'rtl8197f_{board}.dts')])
                out = Path(tmp)/'board.dtb'
                subprocess.run([dtc, '-I', 'dts', '-O', 'dtb', '-o', str(out), '-'],
                               input=data, check=True, capture_output=True)
                text = subprocess.check_output([dtc, '-I', 'dtb', '-O', 'dts', str(out)],
                                                text=True, stderr=subprocess.DEVNULL)
                self.assertIn(f'console=ttyS0,{baud}n8', text)
                self.assertIn(f'serial0:{baud}n8', text)
                if board.startswith('cudy'):
                    self.assertIn('current-speed = <0x9600>;', text)
                if board.endswith('fullflash'):
                    self.assertIn('root=/dev/mtdblock6', text)


if __name__ == '__main__':
    unittest.main()
