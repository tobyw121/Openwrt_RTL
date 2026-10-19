#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Host regression tests; these do not replace a MIPS build or hardware tests.

Run: python3 -m unittest discover -s scripts/tests -v
The C tests compile the actual poll/delivery functions with mocked kernel/HAL
boundaries. Shell tests execute the actual policy functions with fake UCI/proc
interfaces, never the host firewall or network services.
"""
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "target/linux/realtek/base-files"
WLAN = ROOT / "package/kernel/rtl8192cd-rtl8197f/src"


def shell_function(path, name):
    match = re.search(r"^" + re.escape(name) + r"\(\)\s*\{\n.*?^\}",
                      path.read_text(), re.M | re.S)
    if not match:
        raise AssertionError(f"missing shell function {name} in {path}")
    return match[0]


def c_function(path, signature):
    source = path.read_text()
    start = source.index(signature)
    return source[start:source.index("\n}", start) + 2]


class SoftwareOffloadTests(unittest.TestCase):
    def run_shell(self, source, command, *, enabled="1", hardware="1", board="mw5"):
        with tempfile.TemporaryDirectory() as tmp:
            log = Path(tmp) / "calls"
            # Log every mutation. Return an old/offloading-enabled UCI state
            # so recovery must actively turn it off, rather than pass a no-op.
            mocks = r'''
uci() {
    [ "$1" != -q ] || shift
    case "$1" in
        get)
            case "$2" in
                mw5_accel.core.enabled) echo "$TEST_ENABLED" ;;
                mw5_accel.core.hardware) echo "$TEST_HARDWARE" ;;
                mw5_accel.core.software|firewall.@defaults\[0\].flow_offloading*) echo 1 ;;
                firewall.@defaults\[0\]) echo defaults ;;
                network.wan.device) echo wan ;;
                *) return 1 ;;
            esac ;;
        show) return 0 ;;
        add) echo testsection ;;
        *) echo "uci $*" >> "$TEST_LOG" ;;
    esac
}
is_mw5() { [ "$TEST_BOARD" = mw5 ]; }
proc_cmd() { echo "proc $1" >> "$TEST_LOG"; }
firewall_reload() { echo reload >> "$TEST_LOG"; }
firewall_zone_section() { return 1; }
wan_route_ready() { return 0; }
ip() { :; }
log() { :; }
'''
            # Stub absolute service calls as well as the proc interface.
            source = source.replace(
                "[ -x /etc/init.d/firewall ] && /etc/init.d/firewall reload",
                "firewall_reload")
            source = source.replace("/etc/init.d/firewall reload", "firewall_reload")
            env = dict(os.environ, TEST_LOG=str(log), TEST_ENABLED=enabled,
                       TEST_HARDWARE=hardware, TEST_BOARD=board)
            result = subprocess.run(["sh", "-c", source + "\n" + mocks + "\n" + command],
                                    env=env, text=True, capture_output=True)
            return result, log.read_text() if log.exists() else ""

    def accel_source(self):
        return (BASE / "usr/sbin/mw5-accel").read_text().split(
            'case "${1:-status}" in', 1)[0]

    def assert_off(self, calls):
        self.assertIn("set firewall.@defaults[0].flow_offloading=0", calls)
        self.assertIn("set firewall.@defaults[0].flow_offloading_hw=0", calls)
        self.assertNotIn("flow_offloading=1", calls)
        self.assertNotIn("flow_offloading_hw=1", calls)

    def test_accel_boot_migrates_old_config_without_transient_software_enable(self):
        result, calls = self.run_shell(self.accel_source(), "apply")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assert_off(calls)
        self.assertIn("set mw5_accel.core.software=0", calls)
        self.assertLess(calls.index("proc accel-sw-off"), calls.index("proc accel-on"))

    def test_master_off_and_hardware_off_keep_software_disabled(self):
        for enabled, hardware, expected in (("0", "1", "accel-off"),
                                            ("1", "0", "accel-hw-off")):
            with self.subTest(enabled=enabled, hardware=hardware):
                result, calls = self.run_shell(self.accel_source(), "apply",
                                               enabled=enabled, hardware=hardware)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assert_off(calls)
                self.assertIn("proc " + expected, calls)

    def test_master_and_hardware_on_dispatch_do_not_enable_software(self):
        source = (BASE / "usr/sbin/mw5-accel").read_text()
        prefix, dispatch = source.split('case "${1:-status}" in', 1)
        for args in ("on", "hardware on", "software off"):
            with self.subTest(command=args):
                result, calls = self.run_shell(prefix, "set -- " + args +
                    '\ncase "${1:-status}" in' + dispatch)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assert_off(calls)

    def test_software_on_is_rejected_without_mutation(self):
        result = subprocess.run(["sh", str(BASE / "usr/sbin/mw5-accel"),
                                 "software", "on"], capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn("disabled pending hardware stress validation", result.stderr)

    def test_other_boards_are_untouched(self):
        result, calls = self.run_shell(self.accel_source(), "apply", board="rd05")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(calls, "")

    def test_missing_proc_interface_still_disables_firewall_and_reports_failure(self):
        result, calls = self.run_shell(self.accel_source(),
                                       "proc_cmd() { return 1; }; apply")
        self.assertEqual(result.returncode, 1)
        self.assert_off(calls)

    def test_kernel_software_cache_is_off_before_userspace_runs(self):
        driver = (ROOT / "target/linux/realtek/files-6.6/drivers/net/ethernet/"
                  "rtl8197f_rtknet.c").read_text()
        self.assertIn("static bool mw5_accel_sw_enable;", driver)
        master = driver.split('if (!strcmp(buf, "accel-on")) {', 1)[1].split(
            'if (!strcmp(buf, "accel-off")) {', 1)[0]
        self.assertNotIn("WRITE_ONCE(mw5_accel_sw_enable, true)", master)
        self.assertIn("priv->accel_sw = READ_ONCE(mw5_accel_sw_enable);", master)

    def test_portmode_firewall_repair_keeps_offloading_off(self):
        source = shell_function(BASE / "usr/sbin/mw5-portmode", "ensure_base_firewall")
        result, calls = self.run_shell(source, "ensure_base_firewall")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assert_off(calls)

    def test_delayed_network_repair_disables_and_commits_old_offloading(self):
        source = shell_function(BASE / "etc/init.d/mw5-network-core", "ensure_wan")
        result, calls = self.run_shell(source, "LOG=/dev/null; ensure_wan safe delayed")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assert_off(calls)
        self.assertIn("uci commit firewall", calls)
        self.assertIn("reload", calls)

    def test_fresh_config_and_all_automatic_writers_are_safe(self):
        config = (BASE / "etc/config/mw5_accel").read_text()
        self.assertIn("option software '0'", config)
        self.assertFalse((BASE / "etc/config/mw5-accel").exists())
        for path in BASE.rglob("*"):
            if path.is_file():
                text = path.read_text(errors="replace")
                self.assertNotRegex(text, r"flow_offloading(?:_hw)?=['\"]?1",
                                    str(path))
        base = (ROOT / "package/network/config/mw5-base/Makefile").read_text()
        self.assertIn("../init.d/mw5-accel $(1)/etc/rc.d/S94mw5-accel", base)


class NapiTests(unittest.TestCase):
    def test_actual_poll_and_delivery_functions(self):
        poll = c_function(WLAN / "8192cd_rx.c", "int rtl8192cd_linux6_napi_poll(")
        receive = c_function(WLAN / "core/8192cd_core_rx.c",
                             "static void rtl8192cd_mw5_receive(")
        harness = r'''
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#define CONFIG_MW5_FORCE_WLAN_DEVICE_TABLE
#define CONFIG_MW5_ACCEL
#define VERSION_8822B 1
#define container_of(p, t, m) ((t *)((char *)(p) - offsetof(t, m)))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define IS_DRV_OPEN(p) ((p)->open)
#define GET_CHIP_VER(p) ((p)->chip)
#define GET_HAL_INTERFACE(p) (&hal)
struct napi_struct { int unused; };
struct sk_buff { struct sk_buff *next; };
struct sk_buff_head { struct sk_buff *head, *tail; };
struct net_device { int unused; };
struct rtl8192cd_priv;
struct priv_shared_info {
    struct napi_struct linux6_rx_napi;
    struct sk_buff_head linux6_rx_delivery;
    struct rtl8192cd_priv *linux6_root_priv;
    int linux6_rx_budget, linux6_rx_work_done, linux6_radio_index;
    bool linux6_napi_polling;
    unsigned long linux6_napi_polls, linux6_napi_work_done, linux6_tx_reclaim_kicks;
    unsigned long linux6_gro_packets, linux6_gro_merged, linux6_backlog_packets;
    struct { int tasklet; } tx_tasklet;
};
struct rtl8192cd_priv {
    struct priv_shared_info *pshare;
    struct net_device *dev;
    bool open;
    int chip;
};
static int rx_calls, drains, completed, irq_enabled, gro, backlog, kicks;
static int supplied_work, complete_ok;
static bool vendor_locked;
typedef enum { GRO_NORMAL, GRO_MERGED, GRO_MERGED_FREE } gro_result_t;
static gro_result_t gro_result;
static bool napi_complete_done(struct napi_struct *n, int work)
{ (void)n; (void)work; completed++; return complete_ok; }
static void enable_rx(struct rtl8192cd_priv *p)
{ assert(!p->pshare->linux6_napi_polling); irq_enabled++; }
static struct { void (*EnableRxRelatedInterruptHandler)(struct rtl8192cd_priv *); }
    hal = { enable_rx };
static gro_result_t napi_gro_receive(struct napi_struct *n, struct sk_buff *s)
{ (void)n; (void)s; assert(!vendor_locked); gro++; return gro_result; }
static void netif_rx(struct sk_buff *s) { (void)s; backlog++; }
static void skb_queue_tail(struct sk_buff_head *q, struct sk_buff *s)
{
    s->next = NULL;
    if (q->tail) q->tail->next = s; else q->head = s;
    q->tail = s;
}
static struct sk_buff *skb_dequeue(struct sk_buff_head *q)
{
    struct sk_buff *s = q->head;
    if (s) {
        q->head = s->next;
        if (!q->head) q->tail = NULL;
    }
    return s;
}
static int rtk_tasklet_set_triggered(void *t) { (void)t; return 1; }
static void tasklet_schedule(void *t) { (void)t; kicks++; }
RECEIVE_FUNCTION
static void rtl8192cd_rx_isr(struct rtl8192cd_priv *p)
{
    static struct sk_buff skb;
    assert(p->pshare->linux6_napi_polling);
    assert(p->pshare->linux6_rx_budget > 0);
    rx_calls++;
    p->pshare->linux6_rx_work_done = supplied_work;
    vendor_locked = true;
    if (supplied_work) rtl8192cd_mw5_receive(p, &skb);
    vendor_locked = false;
}
static void schedule_rxq_tasklet(struct rtl8192cd_priv *p, struct net_device *d)
{
    static struct sk_buff skb;
    (void)d;
    drains++;
    /* Delivery must still belong to NAPI while software queues drain. */
    assert(p->pshare->linux6_napi_polling);
    vendor_locked = true;
    if (supplied_work) rtl8192cd_mw5_receive(p, &skb);
    vendor_locked = false;
}
POLL_FUNCTION
int main(void)
{
    struct priv_shared_info s = {0};
    struct rtl8192cd_priv p = { .pshare = &s, .open = true, .chip = 1 };
    struct sk_buff skb;
    s.linux6_root_priv = &p;
    /* Zero budget must neither consume descriptors nor complete NAPI. */
    assert(rtl8192cd_linux6_napi_poll(&s.linux6_rx_napi, 0) == 0);
    assert(!rx_calls && !drains && !completed && !irq_enabled);
    complete_ok = 1;
    supplied_work = 3;
    assert(rtl8192cd_linux6_napi_poll(&s.linux6_rx_napi, 64) == 3);
    assert(rx_calls == 1 && drains == 1 && completed == 1 && irq_enabled == 1);
    assert(gro == 2 && !backlog);
    assert(s.linux6_gro_packets == 2 && !s.linux6_gro_merged);
    assert(!s.linux6_rx_delivery.head && !s.linux6_rx_delivery.tail);
    assert(!s.linux6_napi_polling && !s.linux6_rx_budget);
    rtl8192cd_mw5_receive(&p, &skb); /* e.g. reorder timer, outside NAPI */
    assert(backlog == 1 && gro == 2);
    assert(s.linux6_backlog_packets == 1);
    /* At budget, leave NAPI scheduled and RX IRQs masked. */
    supplied_work = 64;
    gro_result = GRO_MERGED;
    assert(rtl8192cd_linux6_napi_poll(&s.linux6_rx_napi, 64) == 64);
    assert(s.linux6_gro_merged == 2);
    assert(completed == 1 && irq_enabled == 1);
    supplied_work = 65;
    gro_result = GRO_MERGED_FREE;
    assert(rtl8192cd_linux6_napi_poll(&s.linux6_rx_napi, 64) == 64);
    assert(s.linux6_gro_packets == 6 && s.linux6_gro_merged == 4);
    /* Busy-poll ownership may refuse completion; don't unmask interrupts. */
    supplied_work = 0;
    complete_ok = 0;
    assert(rtl8192cd_linux6_napi_poll(&s.linux6_rx_napi, 64) == 0);
    assert(completed == 2 && irq_enabled == 1);
    complete_ok = 1;
    assert(rtl8192cd_linux6_napi_poll(&s.linux6_rx_napi, 64) == 0);
    assert(irq_enabled == 2);
    /* Preserve the existing periodic TX reclaim for the 5 GHz HAL. */
    s.linux6_napi_polls = 15;
    supplied_work = 1;
    assert(rtl8192cd_linux6_napi_poll(&s.linux6_rx_napi, 64) == 1);
    assert(kicks == 1 && s.linux6_tx_reclaim_kicks == 1);
    p.open = false;
    rx_calls = 0;
    irq_enabled = 0;
    assert(rtl8192cd_linux6_napi_poll(&s.linux6_rx_napi, 64) == 0);
    assert(!rx_calls && !irq_enabled);
    s.linux6_root_priv = NULL;
    assert(rtl8192cd_linux6_napi_poll(&s.linux6_rx_napi, 64) == 0);
    return 0;
}
'''.replace("RECEIVE_FUNCTION", receive).replace("POLL_FUNCTION", poll)
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "test.c"
            binary = Path(tmp) / "test"
            source.write_text(harness)
            subprocess.run(["cc", "-std=gnu11", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=undefined", str(source), "-o", str(binary)],
                           check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    unittest.main()
