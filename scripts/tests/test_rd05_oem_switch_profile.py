#!/usr/bin/env python3
"""RD05 OEM tap migration, board isolation and pre-bootstrap call ordering."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT, c_function

SCRIPT = ROOT / 'package/network/config/rtl8367d-compat/files/rd05-rgmii-calibrate'
PATCHES = ROOT / 'target/linux/realtek/patches-6.6'


class RD05OEMSwitchProfileTests(unittest.TestCase):
    def run_profile(self, command, config, board='xiaomi,r4-rd05'):
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            soc, sw, binpath = [base / name for name in ('soc', 'sw', 'bin')]
            for p in (soc, sw, binpath):
                p.mkdir()
            initial = {'rd05_p0_tx_delay': '0', 'rd05_p0_rx_delay': '6',
                       'rd05_ext1_tx_delay': '0', 'rd05_ext1_txc_tap': '0',
                       'rd05_ext1_rx_delay': '5', 'rd05_ext1_ssc': '1'}
            for name, value in initial.items():
                ((soc if name.startswith('rd05_p0') else sw) / name).write_text(value+'\n')
            conf = base / 'profile'
            conf.write_text(config)
            mocks = {
                'cat': '#!/bin/sh\nif [ "$1" = /tmp/sysinfo/board_name ]; then echo "$MOCK_BOARD"; else exec /bin/cat "$@"; fi\n',
                'rd05-ioctl': '#!/bin/sh\necho "$*" >> "$MOCK_CALLS"\n',
                'dmesg': '#!/bin/sh\nexit 0\n',
                'ethtool': '#!/bin/sh\nexit 0\n',
            }
            for name, source in mocks.items():
                p = binpath / name
                p.write_text(source)
                p.chmod(0o755)
            env = dict(os.environ, PATH=str(binpath)+':'+os.environ['PATH'],
                       MOCK_BOARD=board, MOCK_CALLS=str(base/'calls'),
                       RD05_RGMII_SOC_SYSFS=str(soc), RD05_RGMII_SW_SYSFS=str(sw),
                       RD05_RGMII_CONF=str(conf), RD05_RGMII_APPLY_SETTLE_MS='0')
            result = subprocess.run(['sh', str(SCRIPT), command], env=env,
                                    capture_output=True, text=True)
            return (result, conf.read_text(),
                    (base/'profile.pre-v39').read_text() if (base/'profile.pre-v39').exists() else None,
                    (sw/'rd05_ext1_txc_tap').read_text().strip(),
                    (base/'calls').read_text() if (base/'calls').exists() else '')

    def test_old_profile_migrates_to_measured_tap_and_keeps_backup(self):
        old = 'PROFILE_VERSION=38\nSOC_TX=0\nSOC_RX=6\nSW_TX=0\nSW_TXC=0\nSW_RX=5\nSSC=1\n'
        result, config, backup, tap, calls = self.run_profile('load', old)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(backup, old)
        self.assertIn('PROFILE_VERSION=39', config)
        self.assertIn('SW_TXC=4', config)
        self.assertEqual(tap, '4')
        self.assertEqual(calls, 'eth0 reseed\n')

    def test_current_saved_profile_is_preserved(self):
        config = 'PROFILE_VERSION=39\nSOC_TX=0\nSOC_RX=6\nSW_TX=0\nSW_TXC=2\nSW_RX=5\nSSC=1\n'
        result, after, backup, tap, _ = self.run_profile('load', config)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(after, config)
        self.assertIsNone(backup)
        self.assertEqual(tap, '2')

    def test_mw5_cannot_migrate_or_apply_rd05_settings(self):
        old = 'PROFILE_VERSION=38\nSW_TXC=0\n'
        for command in ('load', 'sdk'):
            result, config, backup, tap, calls = self.run_profile(command, old, 'tenda,nova-mw5')
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('only Xiaomi RD05', result.stderr)
            self.assertEqual(config, old)
            self.assertIsNone(backup)
            self.assertEqual(tap, '0')
            self.assertEqual(calls, '')

    def test_sdk_restore_uses_measured_tap(self):
        result, _, _, tap, calls = self.run_profile('sdk', '')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(tap, '4')
        self.assertEqual(calls, 'eth0 reseed\n')

    def test_native_link_prepare_is_board_gated_and_propagates_errors(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / 'test.c'
            helpers = []
            for name, signature in [
                ('337-net-dsa-realtek-scope-rd05-cpu-policy.patch', 'static bool rtl8367d_rd05_board('),
                ('338-net-dsa-realtek-rd05-prepare-ext1-before-bootstrap.patch', 'static int rtl8367d_rd05_prepare_link('),
            ]:
                p.write_text('\n'.join(line[1:] for line in (PATCHES/name).read_text().splitlines()
                                       if line.startswith('+') and not line.startswith('+++')))
                helpers.append(c_function(p, signature))
            p.write_text(r'''
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#define READ_ONCE(x) (x)
#define WRITE_ONCE(x,y) ((x)=(y))
#define SPEED_1000 1000
#define DUPLEX_FULL 1
struct info { bool rtl8367d; };
struct rtl8365mb { struct info *chip_info; };
struct realtek_priv { struct rtl8365mb *chip_data; };
static const char *board;
static int steps, error;
static unsigned int rtl8367d_rd05_ext1_txc_tap;
static bool rtl8367d_rd05_ext1_txc_explicit;
static bool of_machine_is_compatible(const char *s) { return !strcmp(s,board); }
static int rtl8367d_ext1_config_rgmii(struct realtek_priv *p) {
 assert(steps++==0); return error==1 ? -5 : 0;
}
static int rtl8367d_ext_config_forcemode(struct realtek_priv *p,int port,bool link,int speed,int duplex,bool tx,bool rx) {
 assert(steps++==1); assert(port==7 && link && speed==1000 && duplex==1 && tx && rx);
 return error==2 ? -6 : 0;
}
''' + '\n'.join(helpers) + r'''
int main(void) {
 struct info info={.rtl8367d=true}; struct rtl8365mb mb={.chip_info=&info};
 struct realtek_priv p={.chip_data=&mb};
 board="tenda,nova-mw5"; assert(!rtl8367d_rd05_prepare_link(&p) && !steps);
 assert(!rtl8367d_rd05_ext1_txc_tap);
 board="xiaomi,r4-rd05"; info.rtl8367d=false;
 assert(!rtl8367d_rd05_prepare_link(&p) && !steps); info.rtl8367d=true;
 for(error=0;error<3;error++) {
  steps=0; int rc=rtl8367d_rd05_prepare_link(&p);
  assert(rc==(error==1 ? -5 : error==2 ? -6 : 0)); assert(steps==(error==1 ? 1 : 2));
  assert(rtl8367d_rd05_ext1_txc_tap==4);
 }
 error=0;
 for(int tap=0;tap<8;tap++) {
  steps=0; rtl8367d_rd05_ext1_txc_explicit=true; rtl8367d_rd05_ext1_txc_tap=tap;
  assert(!rtl8367d_rd05_prepare_link(&p)); assert(rtl8367d_rd05_ext1_txc_tap==tap);
 }
 return 0;
}
''')
            binary = Path(tmp) / 'test'
            subprocess.run(['cc', '-Wall', '-Werror', '-fsanitize=undefined',
                            str(p), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
