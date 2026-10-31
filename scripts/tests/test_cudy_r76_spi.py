#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Validate the R76 dump, generated DT, image selection and read-only mapping."""
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

from test_mw5_stability import ROOT, shell_function

spec = importlib.util.spec_from_file_location('cudy_r76', ROOT / 'scripts/rtl8197f-cudy-r76-inspect.py')
inspector = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inspector)


class CudyR76Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        p = ROOT / 'fullflash-templates/Cudy_AX1500.bin'
        cls.dump = p.read_bytes() if p.is_file() else None

    def test_real_dump_partitions_and_corruption_rejection(self):
        if self.dump is None:
            self.skipTest('private SPI dump not installed')
        report, kernel = inspector.inspect(self.dump)
        self.assertEqual(report['source_sha256'], 'f50f3147274a82fcbf4fef7a0c7500aeb7868b60e4e17975faea588237abd188')
        self.assertEqual(report['kernel']['sha256'], '7d7e41a50d6659b5c84c524205bcca25306e2e98a5949c61032810ff84fd003e')
        self.assertEqual([(x['name'], x['offset'], x['size']) for x in report['partitions']], inspector.PARTITIONS)
        for offset in (0x40004, 0x42860, 0x2b0000):
            broken = bytearray(self.dump)
            broken[offset] ^= 1
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                inspector.inspect(broken)
        broken = bytearray(kernel)
        broken[report['partitions'][4]['kernel_record_offset'] + 16] ^= 4
        with self.assertRaises(ValueError):
            inspector.partition_table(broken)
        with self.assertRaises(ValueError):
            inspector.inspect(self.dump[0x10000:])

    @unittest.skipUnless(shutil.which('gcc'), 'host C compiler missing')
    def test_actual_shift_selector(self):
        header = ROOT / 'target/linux/realtek/files-6.6/drivers/mtd/maps/rtl8197f-spirom-shift.h'
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp)
            (p/'linux').mkdir()
            (p/'linux/types.h').write_text('#include <stdbool.h>\ntypedef unsigned char u8;\n')
            (p/'linux/errno.h').write_text('#define EINVAL 22\n#define ENODEV 19\n')
            (p/'check.c').write_text('''
#include <assert.h>
#include <string.h>
#include "rtl8197f-spirom-shift.h"
int main(void) {
    const u8 magic[2][4] = {{0x27,5,0x19,0x56},{0x68,0x73,0x71,0x73}};
    u8 samples[2][7];
    for (int shift=0; shift<4; shift++) {
        memset(samples,0xff,sizeof(samples));
        for (int i=0;i<2;i++) memcpy(samples[i]+3-shift,magic[i],4);
        assert(rtl8197f_spirom_select_shift(samples,magic)==shift);
        samples[1][3-shift]^=1;
        assert(rtl8197f_spirom_select_shift(samples,magic)==-ENODEV);
    }
    memset(samples,0,sizeof(samples));
    const u8 zeros[2][4]={{0}};
    assert(rtl8197f_spirom_select_shift(samples,zeros)==-EINVAL);
    assert(rtl8197f_spirom_select_shift(samples,magic)==-ENODEV);
    return 0;
}
''')
            subprocess.run(['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-I'+tmp, '-I'+str(header.parent), str(p/'check.c'),
                            '-o', str(p/'check')], check=True, capture_output=True)
            subprocess.run([str(p/'check')], check=True)

    def test_single_profile_builds_flash_and_ram(self):
        image = (ROOT/'target/linux/realtek/image/rtl8197f.mk').read_text().replace('include ./common.mk', '')
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp)/'Makefile'
            p.write_text(image + '''
RTL8197F_FULLFLASH_ENABLED := 1
IMAGES := sysupgrade.bin
$(eval $(call Device/cudy_ax1500_r76))
$(info IMAGES=$(IMAGES))
$(info DTS=$(DEVICE_DTS))
$(info CMDLINE=$(RTL8197F_KERNEL_CMDLINE))
$(info SUFFIX=$(KERNEL_INITRAMFS_SUFFIX))
$(info RAM=$(KERNEL_INITRAMFS))
all: ; @:
''')
            out = subprocess.check_output(['make', '-sf', str(p)], text=True)
            self.assertIn('IMAGES=squashfs-spi-full.bin\n', out)
            self.assertIn('DTS=rtl8197f_cudy_ax1500-r76-fullflash rtl8197f_cudy_ax1500-r76\n', out)
            self.assertIn('console=ttyS0,38400n8 root=/dev/mtdblock6', out)
            self.assertIn('SUFFIX=-ram-loader.bin', out)
            self.assertIn('append-cudy-r76-ram-dtb', out)
            self.assertIn('rtl8197f-loader-kernel console=ttyS0,38400n8 rdinit=/sbin/init', out)

    def test_only_one_wr1500_menu_profile(self):
        image = (ROOT/'target/linux/realtek/image/rtl8197f.mk').read_text()
        profiles = [line.split('+=', 1)[1].strip() for line in image.splitlines()
                    if line.startswith('TARGET_DEVICES += cudy_')]
        self.assertEqual([p for p in profiles if p not in ('cudy_re1500', 'cudy_m1500')],
                         ['cudy_ax1500_r76'])

    def test_upgrade_handlers_and_vendor_autostart_are_blocked(self):
        base = ROOT/'target/linux/realtek/base-files'
        upgrade = '\n'.join(shell_function(base/'lib/upgrade/platform.sh', name)
                            for name in ('platform_check_image', 'platform_do_upgrade'))
        for name in ('platform_check_image', 'platform_do_upgrade'):
            out = subprocess.run(['sh', '-c', upgrade + '''
board_name() { echo cudy,ax1500-r76; }
default_do_upgrade() { echo UNEXPECTED_FLASH; }
''' + name + ' image.bin'], capture_output=True, text=True)
            self.assertEqual(out.returncode, 1)
            self.assertNotIn('UNEXPECTED_FLASH', out.stdout)
        guard = shell_function(base/'lib/functions/rtl8197f-board.sh', 'rtl8197f_is_cudy_ram')
        out = subprocess.run(['sh', '-c', guard + '\ncat() { echo cudy,ax1500-r76; }\nrtl8197f_is_cudy_ram'])
        self.assertEqual(out.returncode, 0)

    def test_compiled_dts_preserves_map_and_hardware_isolation(self):
        dtc = os.environ.get('DTC') or shutil.which('dtc')
        headers = os.environ.get('RTL8197F_KERNEL_INCLUDE')
        if not dtc or not headers:
            self.skipTest('set DTC and RTL8197F_KERNEL_INCLUDE')
        dts = ROOT/'target/linux/realtek/dts/rtl8197f_cudy_ax1500-r76.dts'
        with tempfile.TemporaryDirectory() as tmp:
            pp = subprocess.check_output(['cpp', '-nostdinc', '-undef', '-D__DTS__',
                 '-x', 'assembler-with-cpp', '-I', str(dts.parent), '-I', headers, str(dts)])
            dtb = Path(tmp)/'board.dtb'
            subprocess.run([dtc, '-I', 'dts', '-O', 'dtb', '-o', str(dtb), '-'],
                           input=pp, check=True, capture_output=True)
            decoded = subprocess.check_output([dtc, '-I', 'dtb', '-O', 'dts', str(dtb)], stderr=subprocess.DEVNULL, text=True)
            self.assertIn('cudy,ax1500-r76', decoded)
            self.assertIn('serial0:38400n8', decoded)
            self.assertIn('0x40000 0x27051956 0x2b0000 0x68737173', decoded)
            self.assertNotIn('realtek,writable-offset', decoded)
            self.assertNotIn('realtek,writable-size', decoded)
            self.assertEqual(decoded.count('read-only;'), 7)
            self.assertNotIn('label = "rootfs";', decoded)
            self.assertNotIn('label = "firmware";', decoded)
            # Disabled inherited blocks do not become active through the overlay.
            for node in ('ethernet@10000', 'gpio-controller@3500', 'wifi@640000'):
                block = decoded.split(node+' {', 1)[1].split('};', 1)[0]
                self.assertIn('status = "disabled";', block)


if __name__ == '__main__':
    unittest.main()
