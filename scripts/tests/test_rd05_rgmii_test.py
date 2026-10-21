#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Run timing policy and failure cleanup against fake counters/sysfs/DHCP."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT, shell_function

SCRIPT = ROOT / 'package/network/config/rtl8197f-rd05-base/files/usr/sbin/rd05-rgmii-test'


class RGMIIProbeTests(unittest.TestCase):
    def test_classification_requires_samples_and_both_directions(self):
        function = shell_function(SCRIPT, 'classify')
        cases = [
            ('100 100 3 0 10 200 200 4 0 11', 'crc-errors'),
            ('100 100 3 0 10 200 200 3 0 11', 'tx-clean-rx-missing'),
            ('100 100 3 0 10 100 200 3 0 10', 'no-tx-sample'),
            ('100 100 3 0 10 100 200 3 0 11', 'tx-missing-at-switch'),
            ('100 100 3 0 10 99 200 3 0 11', 'counter-reset'),
            ('100 100 3 0 10 200 200 2 0 11', 'counter-reset'),
            ('100 100 3 0 10 200 200 3 1 11', 'bidirectional-traffic'),
            ('100 100 3 0 10 200 200 3 1 9', 'counter-reset'),
        ]
        for args, expected in cases:
            with self.subTest(args=args):
                out = subprocess.check_output(['sh', '-c', function+'\nclassify '+args], text=True)
                self.assertEqual(out.strip(), expected)

    def run_probe(self, tmp, failure=''):
        path = Path(tmp)
        param = path/'delay'; param.write_text('5\n')
        funcs = '\n'.join(shell_function(SCRIPT, name) for name in ['classify', 'restore', 'run_test'])
        funcs = funcs.replace('$(cat /tmp/sysinfo/board_name 2>/dev/null)', 'xiaomi,r4-rd05')
        mocks = r'''
wan_up() { [ -f "$TEST_DIR/lease" ]; }
snapshot() { printf '%s %s %s %s %s\n' "${count:-100}" "${count:-100}" 0 "${rx:-0}" "${count:-100}"; }
sleep() { :; }
udhcpc() { :; }
timeout() { :; }
probe_packets() {
    d=$(cat "$PARAM")
    [ "$d" != "$FAIL_AT" ] || return 1
    if [ "$FAIL_AT" = term ]; then kill -TERM $$; fi
    count=$(( ${count:-100} + 100 ))
    if [ "$d" = 3 ]; then rx=1; : > "$TEST_DIR/lease"; fi
}
'''
        text = 'PARAM="$TEST_DIR/delay"\nREPORT="$TEST_DIR/report"\nLOCK="$TEST_DIR/lock"\nSAVED_DELAY=\n'
        text += funcs+'\n'+mocks+'\nrun_test\n'
        result = subprocess.run(['sh','-c',text], text=True, capture_output=True,
                                env=dict(os.environ,TEST_DIR=tmp,FAIL_AT=failure))
        self.assertEqual(param.read_text().strip(), '5', result.stderr)
        self.assertFalse((path/'lock').exists())
        self.assertIn('restored_rx_delay=5', (path/'report').read_text())
        return result, (path/'report').read_text()

    def test_restore_after_success(self):
        with tempfile.TemporaryDirectory() as tmp:
            result, report = self.run_probe(tmp)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn('DHCP_UP_AT_DELAY=3', report)
            self.assertIn('result=bidirectional-traffic', report)
            self.assertNotIn('delay=4 before=', report)

    def test_restore_after_dhcp_stimulus_failure(self):
        with tempfile.TemporaryDirectory() as tmp:
            result, report = self.run_probe(tmp, '2')
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('DHCP probe failed', report)

    def test_restore_after_termination(self):
        with tempfile.TemporaryDirectory() as tmp:
            result, _ = self.run_probe(tmp, 'term')
            self.assertEqual(result.returncode, 143)

    def test_unavailable_and_sentinel_counters_are_rejected(self):
        function = shell_function(SCRIPT, 'snapshot')
        for values in ['12 34 5', '12 34 18446744073709551615', '12 missing 5']:
            with self.subTest(values=values):
                a,b,c=values.split()
                mock = f'''\nethtool() {{ printf 'cpu_ifInOctets: {a}\\ncpu_ifOutOctets: {b}\\ncpu_fcs_errors: {c}\\n'; }}
cat() {{ echo 0; }}
snapshot
'''
                result = subprocess.run(['sh','-c',function+mock], capture_output=True)
                self.assertEqual(result.returncode == 0, values == '12 34 5')

    def test_private_probe_uses_separate_pid_and_no_network_configuration(self):
        function = shell_function(SCRIPT, 'probe_packets')
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)
            script = 'LOCK="$TEST_DIR"; REPORT="$TEST_DIR/report"; PROBE_JOB=; PROBE_LEASE=0\n'
            script += function + r"""
timeout() { printf '%s\n' "$*" > "$TEST_DIR/args"; return "$PROBE_RC"; }
probe_packets
rc=$?
printf '%s %s\n' "$rc" "$PROBE_LEASE"
"""
            for rc in [0, 1, 124, 143, 127]:
                with self.subTest(rc=rc):
                    result = subprocess.check_output(['sh','-c',script], text=True,
                        env=dict(os.environ,TEST_DIR=tmp,PROBE_RC=str(rc)))
                    self.assertEqual(result.strip(), f'{1 if rc == 127 else 0} {1 if rc == 0 else 0}')
                    args = (path/'args').read_text()
                    self.assertIn('4 udhcpc -f -n -q -t 1 -T 1 -C -B -i wan -s /bin/true', args)
                    self.assertIn('-p '+tmp+'/probe.pid', args)
                    self.assertNotIn('/var/run/', args)


if __name__ == '__main__':
    unittest.main()
