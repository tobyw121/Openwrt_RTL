#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Check RAM-only build outputs, DT hardware isolation and upgrade refusal."""
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT, shell_function

IMAGE = ROOT / 'target/linux/realtek/image/rtl8197f.mk'
BASE = ROOT / 'target/linux/realtek/base-files'
MODELS = ('wr1500', 're1500', 'm1500')


class CudyAX1500Tests(unittest.TestCase):
    def test_make_profiles_clear_inherited_flash_images(self):
        # Evaluate the real GNU make templates, including their shared parent.
        text = IMAGE.read_text().replace('include ./common.mk', '')
        for model in MODELS:
            with self.subTest(model=model), tempfile.TemporaryDirectory() as tmp:
                makefile = Path(tmp) / 'Makefile'
                makefile.write_text(text + f'''
IMAGES := sysupgrade.bin squashfs-spi-full.bin
$(eval $(call Device/cudy_{model}))
$(info IMAGES=$(IMAGES))
$(info DTS=$(DEVICE_DTS))
$(info INITRAMFS=$(KERNEL_INITRAMFS))
$(info SUFFIX=$(KERNEL_INITRAMFS_SUFFIX))
$(info LOADER=$(RTL8197F_LZMA_TEXT_START))
all: ; @:
''')
                out = subprocess.check_output(['make', '-sf', str(makefile)], text=True)
                self.assertIn('IMAGES=\n', out)
                self.assertIn(f'DTS=rtl8197f_cudy_{model}\n', out)
                self.assertIn('SUFFIX=-ram-loader.bin', out)
                self.assertIn('LOADER=0x82000000', out)
                self.assertIn('rtl8197f-loader-kernel | check-size 31744k', out)

    def test_upgrade_is_refused_even_after_forced_image_check(self):
        path = BASE / 'lib/upgrade/platform.sh'
        funcs = '\n'.join(shell_function(path, name) for name in
                          ('platform_check_image', 'platform_do_upgrade'))
        for model in MODELS:
            for function in ('platform_check_image', 'platform_do_upgrade'):
                with self.subTest(model=model, function=function):
                    script = funcs + f'''
board_name() {{ echo cudy,{model}; }}
default_do_upgrade() {{ echo UNEXPECTED_FLASH; }}
{function} ignored.bin
'''
                    result = subprocess.run(['sh', '-c', script], text=True, capture_output=True)
                    self.assertEqual(result.returncode, 1)
                    self.assertNotIn('UNEXPECTED_FLASH', result.stdout)
                    self.assertIn('Cudy AX1500:', result.stderr)
        script = funcs + '\nboard_name() { echo tenda,nova-mw5; }\nplatform_check_image ignored.bin\n'
        self.assertEqual(subprocess.run(['sh', '-c', script]).returncode, 0)

    def test_unverified_hardware_is_disabled_and_ram_is_not_claimed_measured(self):
        db = json.loads((ROOT / 'target/linux/realtek/rtl8197f/cudy-ax1500-profiles.json').read_text())
        common = (ROOT / 'target/linux/realtek/dts/rtl8197f_cudy_ax1500.dtsi').read_text()
        for label in ('spi0', 'spirom0', 'gpio0', 'ethernet0', 'ethdiag0',
                      'switchcore', 'wlan', 'pcie0', 'usb_host', 'usb_otg'):
            self.assertRegex(common, '&'+label+r'\s*\{\s*status = "disabled";\s*\};')
        self.assertNotIn('fixed-partitions', common)
        for model in MODELS:
            board = db['boards']['cudy,'+model]
            self.assertIsNone(board['physical_ram_bytes'])
            self.assertFalse(board['diagnostic_ram_window_is_measured'])
            self.assertFalse(board['flash_upgrade_supported'])

    def test_foreign_wifi_and_webui_boot_are_skipped(self):
        helper = shell_function(BASE / 'lib/functions/rtl8197f-board.sh', 'rtl8197f_is_cudy_ram')
        for service in ('mw5-wifi', 'mw5-webui'):
            boot = shell_function(BASE / 'etc/init.d' / service, 'boot')
            boot = boot.replace('. /lib/functions/rtl8197f-board.sh', ':')
            # If the guard fails, this sentinel catches later runtime actions.
            boot = boot.replace('mkdir ', 'unexpected mkdir ')
            for model in MODELS:
                script = helper + '\n' + boot + f'''
cat() {{ echo cudy,{model}; }}
unexpected() {{ echo UNEXPECTED_ACTION; return 1; }}
boot
'''
                result = subprocess.run(['sh', '-c', script], text=True, capture_output=True)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, '')

    @unittest.skipUnless(shutil.which('dtc') and shutil.which('cpp'), 'dtc/cpp not installed')
    def test_dts_compiles(self):
        # Use the headers from a Linux source tree supplied by the test runner.
        import os
        headers = os.environ.get('RTL8197F_KERNEL_INCLUDE')
        if not headers:
            self.skipTest('set RTL8197F_KERNEL_INCLUDE to Linux include directory')
        for model in MODELS:
            with self.subTest(model=model), tempfile.TemporaryDirectory() as tmp:
                dts = ROOT / f'target/linux/realtek/dts/rtl8197f_cudy_{model}.dts'
                pp = subprocess.check_output(['cpp', '-nostdinc', '-undef', '-D__DTS__',
                    '-x', 'assembler-with-cpp', '-I', str(dts.parent), '-I', headers, str(dts)])
                result = subprocess.run(['dtc', '-I', 'dts', '-O', 'dtb', '-o', str(Path(tmp)/'board.dtb'), '-'],
                                        input=pp, capture_output=True)
                self.assertEqual(result.returncode, 0, result.stderr.decode())


if __name__ == '__main__':
    unittest.main()
