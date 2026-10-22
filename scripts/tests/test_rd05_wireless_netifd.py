#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise RD05 UCI migration and netifd notifications with hostapd boundaries mocked."""
import os
from pathlib import Path
import subprocess
import tempfile
import time
import unittest
from test_mw5_stability import ROOT, BASE, shell_function

FRONT = BASE / 'usr/sbin/mw5-wifi'
ADAPTER = ROOT / 'package/network/config/rtl8197f-rd05-base/files/lib/netifd/rtl8197f-wireless.sh'


class RD05WirelessTests(unittest.TestCase):
    def test_ssid_validation_preserves_normal_text_and_rejects_controls(self):
        function = shell_function(ADAPTER, 'rd05_valid_text')
        for value in ['Openwrt_5GHz', 'Openwrt_2,4GHz', '[:cntrl:]', "WiFi ' Küche"]:
            result = subprocess.run(['sh','-c',function+'\nrd05_valid_text "$TEST_TEXT"'],
                env=dict(os.environ, TEST_TEXT=value))
            self.assertEqual(result.returncode, 0, repr(value))
        for value in ['', *(f'AP{chr(c)}injected=1' for c in [*range(1,32),127])]:
            result = subprocess.run(['sh','-c',function+'\nrd05_valid_text "$TEST_TEXT"'],
                env=dict(os.environ, TEST_TEXT=value))
            self.assertNotEqual(result.returncode, 0, repr(value))

    @unittest.skipUnless(os.environ.get('RTL8197F_BUSYBOX_NO_TR_CLASSES'),
                         'set RTL8197F_BUSYBOX_NO_TR_CLASSES to a BusyBox binary built without TR_CLASSES')
    def test_busybox_without_tr_classes_reproduces_bug_and_accepts_fix(self):
        busybox = Path(os.environ['RTL8197F_BUSYBOX_NO_TR_CLASSES']).resolve()
        sample = 'Openwrt_5GHz'
        broken = subprocess.check_output([str(busybox),'tr','-d','[:cntrl:]'], input=sample, text=True)
        self.assertNotEqual(broken, sample, 'BusyBox fixture must have TR_CLASSES disabled')
        function = shell_function(ADAPTER, 'rd05_valid_text')
        with tempfile.TemporaryDirectory() as tmp:
            (Path(tmp)/'tr').symlink_to(busybox)
            for value, success in [(sample, True), ('Openwrt_2,4GHz', True),
                                   ('[:cntrl:]', True), ('AP\ninjected=1', False),
                                   ('AP\rinjected=1', False), ('AP\x7f', False), ('', False)]:
                result = subprocess.run([str(busybox),'ash','-c',function+'\nrd05_valid_text "$TEST_TEXT"'],
                    env=dict(os.environ, PATH=tmp, TEST_TEXT=value))
                self.assertEqual(result.returncode == 0, success, repr(value))

    def test_ap_lock_serializes_processes_and_releases_after_failure(self):
        function = shell_function(ADAPTER, 'rd05_ap_lock')
        with tempfile.TemporaryDirectory() as tmp:
            env = dict(os.environ, RD05_AP_LOCK=tmp+'/lock', TEST_LOG=tmp+'/events')
            body = function + r'''
rd05_ap_lock || exit 1
echo "enter $1" >> "$TEST_LOG"
sleep 0.1
echo "exit $1" >> "$TEST_LOG"
exit "$2"
'''
            first = subprocess.Popen(['sh','-c',body,'test','first','1'], env=env)
            try:
                deadline = time.monotonic()+3
                while not Path(tmp+'/events').exists() and time.monotonic() < deadline:
                    time.sleep(0.01)
                second = subprocess.run(['sh','-c',body,'test','second','0'], env=env, timeout=4)
                self.assertEqual(first.wait(timeout=3), 1)
                self.assertEqual(second.returncode, 0)
                self.assertEqual(Path(tmp+'/events').read_text().splitlines(),
                                 ['enter first','exit first','enter second','exit second'])
                self.assertFalse(Path(tmp+'/lock').exists())
            finally:
                if first.poll() is None:
                    first.kill()
                    first.wait()

    def test_ap_lock_timeout_preserves_another_owners_lock(self):
        function = shell_function(ADAPTER, 'rd05_ap_lock')
        with tempfile.TemporaryDirectory() as tmp:
            lock = Path(tmp)/'lock'
            lock.mkdir()
            result = subprocess.run(['sh','-c', function+'\nsleep() { :; }; rd05_ap_lock'],
                env=dict(os.environ, RD05_AP_LOCK=str(lock)), text=True, capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('lock unavailable', result.stderr)
            self.assertTrue(lock.is_dir())

    def test_ap_lock_releases_on_termination(self):
        function = shell_function(ADAPTER, 'rd05_ap_lock')
        with tempfile.TemporaryDirectory() as tmp:
            result = subprocess.run(['sh','-c', function+'\nrd05_ap_lock; kill -TERM $$'],
                env=dict(os.environ, RD05_AP_LOCK=tmp+'/lock'))
            self.assertEqual(result.returncode, 1)
            self.assertFalse(Path(tmp+'/lock').exists())

    def test_real_frontend_can_be_loaded_without_dispatch_or_nounset(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'frontend'
            source = FRONT.read_text().replace('/tmp/mw5-webui', tmp+'/log')
            source = source.replace('/var/run/mw5-wifi', tmp+'/run')
            path.write_text(source)
            result = subprocess.run(['sh', '-c', 'MW5_WIFI_LIBRARY=1 . "$1"; '
                'set_defaults; command -v start_radio; echo "${not_set_yet}"; echo loaded',
                'test', str(path)], text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, 'start_radio\n\nloaded\n')

    def run_adapter(self, *, command='drv_mac80211_setup radio0', **values):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)
            helper = path / 'ap-library'
            # Execute the real start/stop orchestration; replace radio hardware.
            helper.write_text(r'''
LOG_FILE="${TEST_HELPER}.log"
RUN_DIR="${TEST_HELPER}.runtime"
set_defaults() { :; }
load_radio_tuning() { :; }
mw5_rtknet_reseed() { :; }
stop_hostapd_band() { echo "stop $1" >> "$TEST_LOG"; }
probe_driver() { echo probe >> "$TEST_LOG"; }
iw() { echo "iw $*" >> "$TEST_LOG"; }
start_radio() {
    echo "start $1 $2 ssid=$4 channel=$5 width=$6 encryption=$7 bridge=$BRIDGE hidden=$MW5_NETIFD_HIDDEN isolate=$MW5_NETIFD_ISOLATE" >> "$TEST_LOG"
    [ "$TEST_FAIL_START" != 1 ]
}
ip() { echo "ip $*" >> "$TEST_LOG"; }
''')
            script = ADAPTER.read_text().replace('. /lib/netifd/netifd-wireless.sh', '')
            script = script.replace('init_wireless_driver "$@"', '')
            script = script.replace('. /usr/sbin/mw5-wifi', '. "$TEST_HELPER"')
            script = script.replace('mkdir -p /tmp/mw5-webui', ':')
            script = script.replace('/tmp/mw5-webui/netifd-wlan.log', str(path / 'ap.log'))
            script = script.replace('/var/run/rd05-wlan-apply.lock', str(path / 'ap.lock'))
            script = script.replace('cat "/var/run/mw5-wifi/hostapd-$rd05_band.pid"', 'printf 123')
            mocks = r'''
add_driver() { :; }
flock() { return 127; }
sleep() { :; }
json_select() { :; }
json_get_vars() {
    for v in "$@"; do
        eval "$v=\"\${TEST_$v-}\""
    done
}
json_get_var() { network_bridge="$TEST_bridge"; }
for_each_interface() {
    shift
    i=0
    while [ "$i" -lt "$TEST_COUNT" ]; do
        "$@" "default_radio$i"
        i=$((i+1))
    done
}
wireless_setup_failed() { echo "failed $*" >> "$TEST_LOG"; }
wireless_add_vif() { echo "vif $*" >> "$TEST_LOG"; }
wireless_add_process() { echo "process $*" >> "$TEST_LOG"; }
wireless_set_data() { echo "data $*" >> "$TEST_LOG"; }
wireless_set_up() { echo up >> "$TEST_LOG"; }
wireless_process_kill_all() { echo kill_all >> "$TEST_LOG"; }
'''
            defaults = dict(phy='phy0', band='5g', country='DE', htmode='VHT80',
                            channel='40', mode='ap', ssid="My ' AP", encryption='psk2',
                            key='a test passphrase', bridge='br-lan', hidden='1', isolate='1')
            defaults.update(values)
            busy = defaults.pop('busy_lock', False)
            if busy:
                (path / 'ap.lock.d').mkdir()
            env = dict(os.environ, TEST_LOG=str(path / 'calls'), TEST_HELPER=str(helper),
                       TEST_COUNT=str(defaults.pop('count', 1)),
                       TEST_FAIL_START=str(defaults.pop('fail_start', 0)))
            env.update({'TEST_'+k: str(v) for k, v in defaults.items()})
            shell = ['sh']
            if os.environ.get('RTL8197F_BUSYBOX_NO_TR_CLASSES'):
                # Also exercise the full setup/teardown callbacks with target-like
                # ash/tr, while retaining mocked JSON and hardware boundaries.
                busybox = Path(os.environ['RTL8197F_BUSYBOX_NO_TR_CLASSES']).resolve()
                tools = path / 'bin'
                tools.mkdir()
                (tools / 'tr').symlink_to(busybox)
                env['PATH'] = str(tools) + ':' + env['PATH']
                shell = [str(busybox), 'ash']
            result = subprocess.run([*shell, '-c', mocks + script + '\n' + command],
                                    env=env, text=True, capture_output=True)
            calls = (path / 'calls').read_text() if (path / 'calls').exists() else ''
            if busy:
                self.assertTrue((path / 'ap.lock.d').is_dir())
            return result, calls

    def test_busy_ap_lock_does_not_start_or_stop_a_radio(self):
        for command in ['drv_mac80211_setup radio0', 'drv_mac80211_teardown radio0']:
            result, calls = self.run_adapter(command=command, busy_lock=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertNotIn('start ', calls)
            self.assertNotIn('stop ', calls)
            self.assertNotIn('kill_all', calls)

    def test_both_radios_register_with_netifd_after_hostapd_start(self):
        for args, iface, width in [({}, 'wlan0', '80'),
                                  ({'phy':'phy1', 'band':'2g', 'htmode':'HT20', 'channel':'6'}, 'wlan1', '20')]:
            result, calls = self.run_adapter(**args)
            self.assertEqual(result.returncode, 0, result.stderr + calls)
            self.assertIn(' '+iface+' ssid=', calls)
            self.assertIn('width='+width, calls)
            self.assertIn('bridge=br-lan hidden=1 isolate=1', calls)
            self.assertIn('vif default_radio0 '+iface, calls)
            self.assertIn('process 123 /usr/sbin/hostapd 1', calls)
            self.assertTrue(calls.endswith('up\n'), calls)
            self.assertLess(calls.index('start '), calls.index('vif '))

    def test_invalid_settings_do_not_start_or_publish_an_ap(self):
        for args in [{'mode':'sta'}, {'key':'short'}, {'encryption':'sae'},
                     {'ssid':'bad\nconfig=1'}, {'key':'passphrase\ninjection=1'},
                     {'phy':'phy9'}, {'band':'6g'}, {'htmode':'HE80'},
                     {'ifname':'wlan9'}, {'bridge':''}, {'channel':'auto'},
                     {'country':'D\nE'}, {'count':2}, {'count':0},
                     {'wds':'1'}, {'ieee80211w':'2'}, {'txpower':'30'}]:
            with self.subTest(args=args):
                result, calls = self.run_adapter(**args)
                self.assertNotEqual(result.returncode, 0, result.stdout + calls)
                self.assertIn('failed ', calls)
                self.assertNotIn('start ', calls)
                self.assertNotIn('vif ', calls)
                self.assertNotIn('\nup\n', calls)

    def test_failed_hostapd_start_is_cleaned_up_and_never_marked_up(self):
        result, calls = self.run_adapter(fail_start=1)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('failed HOSTAPD_START_FAILED', calls)
        self.assertIn('kill_all', calls)
        self.assertNotIn('vif ', calls)
        self.assertNotIn('\nup\n', calls)

    def test_stop_affects_only_the_selected_radio(self):
        result, calls = self.run_adapter(command='drv_mac80211_teardown radio0')
        self.assertEqual(result.returncode, 0, result.stderr)
        # The teardown helper's log is independent of stdout redirection.
        self.assertIn('stop 5g', calls)
        self.assertIn('ip link set wlan0 down', calls)
        self.assertNotIn('wlan1', calls)

    def test_migration_preserves_settings_and_existing_luci_configuration(self):
        funcs = '\n'.join(shell_function(FRONT, name) for name in
                          ['rd05_migrate_wireless', 'set_defaults'])
        funcs += '\n' + next(line for line in FRONT.read_text().splitlines() if line.startswith('sanitize_bool()'))
        with tempfile.TemporaryDirectory() as tmp:
            funcs = funcs.replace('/etc/config', tmp+'/config')
            script = r'''
rd05_netifd_managed() { [ "$TEST_BOARD" = rd05 ]; }
load_config() {
    set_defaults
    R5_SSID="Retained ' SSID"; R5_KEY='keep this passphrase'
    R2_ENABLED=0
}
uci() {
    shift
    if [ "$1" = show ]; then
        [ "$TEST_EXISTING" != 1 ] || echo "wireless.radio0='wifi-device'"
        return
    fi
    echo "$*" >> "$TEST_LOG"
}
''' + funcs + '\nrd05_migrate_wireless\n'
            for board, existing in [('rd05','0'), ('rd05','1'), ('mw5','0')]:
                log = Path(tmp) / 'calls'
                log.unlink(missing_ok=True)
                result = subprocess.run(['sh', '-c', script], text=True, capture_output=True,
                    env=dict(os.environ, TEST_BOARD=board, TEST_EXISTING=existing, TEST_LOG=str(log)))
                self.assertEqual(result.returncode, 0, result.stderr)
                calls = log.read_text() if log.exists() else ''
                if board != 'rd05' or existing == '1':
                    self.assertEqual(calls, '')
                    continue
                self.assertIn("set wireless.default_radio0.ssid=Retained ' SSID", calls)
                self.assertIn('set wireless.default_radio0.key=keep this passphrase', calls)
                self.assertIn('set wireless.radio0.band=5g', calls)
                self.assertIn('set wireless.radio1.band=2g', calls)
                self.assertIn('set wireless.radio1.disabled=1', calls)
                self.assertEqual(calls.count('commit wireless'), 1)

    def test_frontend_config_save_does_not_overwrite_luci_wireless(self):
        funcs = '\n'.join(shell_function(FRONT, name) for name in
                          ['set_defaults', 'write_config_file'])
        funcs += '\n' + next(line for line in FRONT.read_text().splitlines() if line.startswith('sq()'))
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)
            wireless = path / 'wireless'
            original = "config wifi-device 'radio0'\n\toption channel '44'\n"
            wireless.write_text(original)
            funcs = funcs.replace('/etc/config', tmp)
            script = funcs + r'''
rd05_netifd_managed() { return 0; }
write_runtime() { :; }
VERSION=test
set_defaults
write_config_file
'''
            result = subprocess.run(['sh', '-c', script], text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(wireless.read_text(), original)

    def test_driver_dispatch_is_rd05_only(self):
        dispatcher = ROOT / 'package/network/config/wifi-scripts/files/lib/netifd/wireless/mac80211.sh'
        prefix = dispatcher.read_text().split('. /lib/netifd/netifd-wireless.sh', 1)[0]
        with tempfile.TemporaryDirectory() as tmp:
            helper = Path(tmp) / 'adapter'
            helper.write_text('#!/bin/sh\necho adapter "$@"\n')
            helper.chmod(0o755)
            prefix = prefix.replace('/lib/netifd/rtl8197f-wireless.sh', str(helper))
            script = 'cat() { echo "$TEST_BOARD"; };\n' + prefix + '\necho standard\n'
            for board in ['xiaomi,r4-rd05', 'tenda,nova-mw5', 'tenda,ac23', '']:
                result = subprocess.run(['sh','-c',script,'test','mac80211','dump'],
                    env=dict(os.environ, TEST_BOARD=board), text=True, capture_output=True)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout.strip(), 'adapter mac80211 dump' if board == 'xiaomi,r4-rd05' else 'standard')


if __name__ == '__main__':
    unittest.main()
