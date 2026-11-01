#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise R76 memory isolation, routed defaults and installed hardware data."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
PACKAGE = ROOT / 'package/network/config/rtl8197f-cudy-ax1500-base'


class CudyPDFProfileTests(unittest.TestCase):
    def test_installed_hardware_profile_and_network_contract(self):
        source = (PACKAGE / 'Makefile').read_text()
        source = '\n'.join(x for x in source.splitlines() if not x.startswith('include '))
        with tempfile.TemporaryDirectory() as tmp:
            dest = Path(tmp) / 'root'
            mk = Path(tmp) / 'Makefile'
            mk.write_text(source + '''
INSTALL_DIR=mkdir -p
INSTALL_DATA=install -m644
install:
	$(call Package/rtl8197f-cudy-r76-base/install,$(DEST))
''')
            subprocess.run(['make', '-sf', str(mk), 'DEST='+str(dest), 'install'],
                           cwd=PACKAGE, check=True, capture_output=True)
            profile = json.loads((dest/'usr/share/cudy-r76/hardware.json').read_text())
            self.assertEqual(profile['ram_bytes'], 128*1024*1024)
            self.assertEqual([(p['label'], p['port']) for p in profile['switch']['ports']],
                             [('lan1', 3), ('lan2', 0), ('lan3', 1), ('wan', 2)])
            # OEM wiring is measured; OpenWrt traffic still needs a hardware test.
            self.assertEqual(profile['switch']['physical_cpu_port'], 7)
            self.assertTrue(profile['switch']['enabled_in_openwrt'])
            self.assertFalse(profile['switch']['openwrt_traffic_validated'])
            self.assertFalse(profile['wifi']['5g']['linux66_driver_complete'])

        board_script = (ROOT/'target/linux/realtek/base-files/etc/board.d/02_network').read_text()
        board_script = '\n'.join(x for x in board_script.splitlines() if not x.startswith('. /lib/'))
        stubs = '''
board_name() { echo "$TEST_BOARD"; }
board_config_update() { :; }
board_config_flush() { echo FLUSH; }
ucidef_set_interfaces_lan_wan() { printf 'LAN=%s\nWAN=%s\n' "$1" "$2"; }
'''
        result = subprocess.run(['sh', '-c', stubs+board_script],
                                env={**os.environ, 'TEST_BOARD': 'cudy,ax1500-r76'},
                                text=True, capture_output=True, check=True)
        self.assertEqual(result.stdout, 'LAN=lan1 lan2 lan3\nWAN=wan\nFLUSH\n')
        for board in ('cudy,re1500', 'cudy,m1500', 'cudy,wr1500'):
            result = subprocess.run(['sh', '-c', stubs+board_script],
                                    env={**os.environ, 'TEST_BOARD': board},
                                    text=True, capture_output=True, check=True)
            self.assertEqual(result.stdout, 'FLUSH\n')

    def test_compiled_memory_is_r76_only_and_dma_geometry_is_preserved(self):
        dtc = os.environ.get('DTC') or shutil.which('dtc')
        headers = os.environ.get('RTL8197F_KERNEL_INCLUDE')
        if not dtc or not headers:
            self.skipTest('DTC and RTL8197F_KERNEL_INCLUDE required')
        with tempfile.TemporaryDirectory() as tmp:
            for model, size in [('ax1500-r76', '0x8000000'),
                                ('ax1500-r76-fullflash', '0x8000000'),
                                ('wr1500', '0x4000000'), ('re1500', '0x4000000'),
                                ('m1500', '0x4000000')]:
                with self.subTest(model=model):
                    src = ROOT/f'target/linux/realtek/dts/rtl8197f_cudy_{model}.dts'
                    pp = subprocess.check_output(['cpp', '-nostdinc', '-undef', '-D__DTS__',
                        '-x', 'assembler-with-cpp', '-I', str(src.parent), '-I', headers, str(src)])
                    blob = Path(tmp)/'board.dtb'
                    subprocess.run([dtc, '-I', 'dts', '-O', 'dtb', '-o', str(blob), '-'],
                                   input=pp, check=True, capture_output=True)
                    tree = subprocess.check_output([dtc, '-I', 'dtb', '-O', 'dts', str(blob)],
                                                   text=True, stderr=subprocess.DEVNULL)
                    memory = tree.split('memory@0 {', 1)[1].split('};', 1)[0]
                    self.assertIn('reg = <0x00 '+size+'>;', memory)
                    if model.startswith('ax1500-r76'):
                        eth = tree.split('ethernet@10000 {', 1)[1].split('};', 1)[0]
                        self.assertIn('realtek,rx-ring-size = <0x384>;', eth)
                        self.assertIn('realtek,tx-ring-size = <0x300>;', eth)
                        self.assertIn('realtek,descriptor-dwords = <0x08>;', eth)
                        self.assertIn('status = "okay";', eth)
                        switch = tree.split('\n\tswitch {', 1)[1]
                        for port, label in ((0, 'lan2'), (1, 'lan3'), (2, 'wan'), (3, 'lan1')):
                            block = switch.split(f'port@{port} {{', 1)[1].split('};', 1)[0]
                            self.assertIn(f'label = "{label}";', block)
                        cpu = switch.split('port@7 {', 1)[1].split('fixed-link', 1)[0]
                        self.assertIn('tx-internal-delay-ps = <0x7d0>;', cpu)
                        self.assertIn('rx-internal-delay-ps = <0x00>;', cpu)
                        self.assertNotIn('cpu-tag-4bytes-proto9', tree)
                        self.assertRegex(switch, r'mdc-gpios = <0x[0-9a-f]+ 0x38 0x00>;')
                        self.assertRegex(switch, r'mdio-gpios = <0x[0-9a-f]+ 0x37 0x00>;')
                        self.assertRegex(switch, r'reset-gpios = <0x[0-9a-f]+ 0x3a 0x01>;')


if __name__ == '__main__':
    unittest.main()
