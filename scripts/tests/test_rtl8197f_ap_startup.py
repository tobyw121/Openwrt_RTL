#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise actual shell policy and HAL interrupt dispatch on the host."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import BASE, WLAN, c_function, shell_function


class APStartupTests(unittest.TestCase):
    def test_only_mw5_waits_for_auto_wan(self):
        helper = (BASE / 'lib/functions/rtl8197f-board.sh').read_text()
        wait = shell_function(BASE / 'etc/init.d/mw5-wifi', 'wait_for_auto_wan')
        # Inject the sourced helper as shell text, keeping the actual policy.
        wait = wait.replace('. /lib/functions/rtl8197f-board.sh', ':')
        for board, expected in [('tenda,nova-mw5', 0), ('nova-mw5', 0),
                                ('xiaomi,rd05', 1), ('tenda,ac23', 1),
                                ('', 1)]:
            with self.subTest(board=board):
                script = helper + '\ncat() { printf "%s" "$BOARD"; }\n'
                script += 'tr() { :; }\n' + wait
                script += '\nsleep() { echo WAIT; }\nrtl8197f_is_mw5; rc=$?\n'
                script += 'wait_for_auto_wan\n'
                script += 'exit "$rc"\n'
                result = subprocess.run(['sh', '-c', script], env={'BOARD': board},
                                        text=True, capture_output=True)
                self.assertEqual(result.returncode, expected, result.stderr)
                self.assertEqual(result.stdout.count('WAIT'), 0 if expected else 26)

    def test_non_mw5_preserves_reseed_without_port_mac_policy(self):
        func = shell_function(BASE / 'usr/sbin/mw5-wifi', 'mw5_network_guard')
        func = func.replace('. /lib/functions/rtl8197f-board.sh', ':')
        script = func + r"""
rtl8197f_is_mw5() { return 1; }
mw5_rtknet_reseed() { echo "reseed:$1"; }
mw5_effective_port_mode() { echo unexpected-policy >&2; return 1; }
ip() { echo unexpected-ip >&2; return 1; }
mw5_network_guard post-driver
"""
        result = subprocess.run(['sh', '-c', script], text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, 'reseed:post-driver\n')
        self.assertEqual(result.stderr, '')

    def test_hostapd_requires_current_enabled_event(self):
        func = shell_function(BASE / 'usr/sbin/mw5-wifi', 'wait_hostapd_enabled')
        cases = [('', 1), ('wlan0: interface state UNINITIALIZED->HT_SCAN\n', 1),
                 ('wlan1: AP-ENABLED\n', 1), ('wlan0: AP-ENABLED\n', 0),
                 ('wlan0: AP-ENABLED\nwlan0: AP-DISABLED\n', 1)]
        with tempfile.TemporaryDirectory() as tmp:
            log = Path(tmp) / 'hostapd.log'
            for events, expected in cases:
                with self.subTest(events=events):
                    log.write_text('wlan0: AP-ENABLED\n' + events)
                    script = func + '\nsleep() { :; }\n'
                    script += f'LOG_FILE="{log}"\nwait_hostapd_enabled wlan0 $$ 2\n'
                    result = subprocess.run(['sh', '-c', script], capture_output=True)
                    self.assertEqual(result.returncode, expected, result.stderr)

    def test_quiet_phases_retain_diagnostics_and_errors(self):
        func = c_function(WLAN / 'rtl8197f_platform_glue.c',
                          'void rtl8197f_wlan_phase_set(')
        src = r"""
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
static int init_phase, init_rc, prints;
static char last_phase[80];
static bool trace_init;
#define strscpy(dst, src, size) snprintf(dst, size, "%s", src)
#define printk(...) (++prints)
""" + func + r"""
int main(void) {
    rtl8197f_wlan_phase_set(90, "ready", 0);
    assert(init_phase == 90 && init_rc == 0 && !strcmp(last_phase, "ready"));
    assert(prints == 0);
    rtl8197f_wlan_phase_set(100, "failed", -12);
    assert(init_phase == 100 && init_rc == -12 && prints == 1);
    trace_init = true;
    rtl8197f_wlan_phase_set(110, NULL, 0);
    assert(!strcmp(last_phase, "unnamed") && prints == 2);
    return 0;
}
"""
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'test.c'; binary = Path(tmp) / 'test'
            c.write_text(src)
            subprocess.run(['cc', '-std=c11', '-Werror', str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)

    def test_management_completions_schedule_recycling_on_all_radios(self):
        func = c_function(WLAN / '8192cd_osdep.c', 'InterruptTxHandle(')
        src = r'''
#include <assert.h>
#include <stdbool.h>
#define CONFIG_RTL8192CD_LINUX6_BUS 1
#define CONFIG_MW5_FORCE_WLAN_DEVICE_TABLE 1
#define __KERNEL__ 1
#define HAL_INT_TYPE_MGNTDOK 1
#define HAL_INT_TYPE_HIGHDOK 2
#define BOOLEAN bool
#define FALSE false
#define VOID void
struct rtl8192cd_hw { int unused; };
struct share { int linux6_mgntdok_irq_count, linux6_highdok_irq_count;
    struct { int tasklet; } tx_tasklet; };
struct rtl8192cd_priv { struct share *pshare; int chip, radio; };
static int mgmt, high, threshold, scheduled;
static bool get_irq(struct rtl8192cd_priv *p, int type) {
    return type == HAL_INT_TYPE_MGNTDOK ? mgmt : high;
}
static bool query(struct rtl8192cd_priv *p) { return threshold; }
static struct { bool (*GetInterruptHandler)(struct rtl8192cd_priv *, int);
    bool (*QueryTxConditionMatchHandler)(struct rtl8192cd_priv *); }
    hal = { get_irq, query };
#define GET_HAL_INTERFACE(p) (&hal)
#define GET_HW(p) ((struct rtl8192cd_hw *)0)
#define rtk_tasklet_set_triggered(p) 1
#define tasklet_schedule(p) (++scheduled)
'''
        src += 'static void ' + func
        src += r'''
int main(void) {
    for (int chip = 0; chip < 5; chip++) for (int radio = 0; radio < 2; radio++)
    for (int mask = 0; mask < 8; mask++) {
        struct share s = {0};
        struct rtl8192cd_priv p = {&s, chip, radio};
        mgmt = mask & 1; high = mask & 2; threshold = mask & 4; scheduled = 0;
        InterruptTxHandle(&p);
        assert(scheduled == !!mask);
        assert(s.linux6_mgntdok_irq_count == !!mgmt);
        assert(s.linux6_highdok_irq_count == !!high);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'test.c'; binary = Path(tmp) / 'test'
            c.write_text(src)
            subprocess.run(['cc', '-std=c11', '-Werror', str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
