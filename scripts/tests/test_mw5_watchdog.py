#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Compile the real Ethernet watchdog/control functions against fake DMA/MMIO.

Exercises progress with no diagnostic process or proc-file reads. This is a
logic/ordering regression test, not an emulation of the MW5 hardware.
"""
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_mw5_stability import ROOT, c_function

DRIVER = ROOT / "target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c"


class EthernetWatchdogTests(unittest.TestCase):
    def test_actual_driver_control_watchdog_and_poll_functions(self):
        signatures = [
            "static void rtl8197f_rtk_write_control(",
            "static bool rtl8197f_rtk_tx_cdp_index_value(",
            "static bool rtl8197f_rtk_mw5_tx_napi_ready(",
            "static void rtl8197f_rtk_enable_irq(",
            "static void rtl8197f_rtk_disable_irq(",
            "static void rtl8197f_rtk_kick_rx(",
            "static void rtl8197f_rtk_tx_doorbell(",
            "static int rtl8197f_rtk_poll(",
            "static bool rtl8197f_rtk_mw5_runout_guard_active(",
            "static void rtl8197f_rtk_rx_poll_timer(",
        ]
        functions = "\n".join(c_function(DRIVER, signature) for signature in signatures)
        program = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef uint32_t u32;
typedef uint64_t u64;
#define READ_ONCE(x) (x)
#define WRITE_ONCE(x,y) ((x) = (y))
#define le32_to_cpu(x) (x)
#define unlikely(x) (x)
#define max_t(t,a,b) ((t)(a) > (t)(b) ? (t)(a) : (t)(b))
#define container_of(p,t,m) ((t *)((char *)(p) - offsetof(t,m)))
#define from_timer(p,t,m) container_of(t,struct rtl8197f_rtknet_priv,m)
#define time_before(a,b) ((long)((a)-(b)) < 0)
#define msecs_to_jiffies(x) (x)
#define RTL_RTK_DESC_OWN (1U << 31)
#define RTL_RTK_CPUIIMR 0
#define RTL_RTK_CPUICR 1
#define RTL_RTK_CPUTPDCR0 2
#define RTL_RTK_INT_ENABLE_MASK 0x123U
#define RTL_RTK_CPUICR_RXCMD 1U
#define RTL_RTK_CPUICR_TXCMD 2U
#define RTL_RTK_CPUICR_TXFD 4U
#define RTL_RTK_MW5_RX_WATCHDOG_MS 1000U
#define RTL_RTK_MW5_RX_RUNOUT_RECHECK_MS 10U
#define RTL_RTK_MW5_RX_PUBLISH_RECHECK_MS 1U
struct timer_list { unsigned long expires; int arms; };
struct napi_struct { int schedules; };
struct net_device { bool running; };
struct rtl8197f_rtk_desc { u32 opts1; };
struct rtl8197f_rtknet_priv {
    bool is_mw5, rx_poll_fallback, rx_publish_wait_pending;
    bool rx_restart_pending, rx_runout_pending;
    struct rtl8197f_rtk_desc *tx_desc;
    u32 tx_ring_size, desc_stride, tx_desc_dma, tx_head, tx_tail, rx_poll_ms;
    int tx_lock;
    struct timer_list rx_poll_timer;
    struct napi_struct napi;
    struct net_device *ndev;
    unsigned long rx_runout_guard_until;
    u64 irq_rx_done, rx_poll_last_irq_rx_done, rx_poll_timer_runs;
    u64 rx_publish_timer_runs, rx_runout_fast_timer_runs;
    u64 rx_poll_irq_progress_skips, rx_poll_fallback_schedules;
    u64 rx_runout_fast_schedules, tx_watchdog_schedules;
    u64 napi_polls, napi_rx_work, napi_tx_clean, napi_zero_rx_polls;
    u64 napi_budget_polls, rx_runout_trxrdy_writes, rx_runout_recoveries;
    u64 napi_rearm_race, hw_rx_restarts, tx_kicks;
};
static unsigned long jiffies = 100;
static bool tx_locked, rx_ready, discover_publication, allow_schedule = true;
static int reads, writes, pending_reg = -1;
static u32 regs[3], pending_value;
static int ready_calls, tx_clean_calls, rx_poll_calls, completions, trx_calls;
static int supplied_rx_work;
#define spin_lock_irqsave(l,f) do { (void)(l); (f)=0; assert(!tx_locked); tx_locked=true; } while(0)
#define spin_unlock_irqrestore(l,f) do { (void)(l); (void)(f); assert(tx_locked); tx_locked=false; } while(0)
static void dma_rmb(void) { assert(tx_locked); }
static void dma_wmb(void) { }
static u32 rtl8197f_rtk_hw_dma_addr(struct rtl8197f_rtknet_priv *p, u32 dma)
{ (void)p; return dma; }
static struct rtl8197f_rtk_desc *rtl8197f_rtk_tx_desc(struct rtl8197f_rtknet_priv *p, u32 idx)
{ assert(tx_locked); assert(idx < p->tx_ring_size); return &p->tx_desc[idx]; }
static void rtl8197f_rtk_write(struct rtl8197f_rtknet_priv *p, u32 reg, u32 value)
{
    writes++;
    if (p->is_mw5) { assert(pending_reg == -1); pending_reg=reg; pending_value=value; }
    else regs[reg]=value;
}
static u32 rtl8197f_rtk_read(struct rtl8197f_rtknet_priv *p, u32 reg)
{
    (void)p;
    reads++;
    if (pending_reg != -1) { regs[pending_reg]=pending_value; pending_reg=-1; }
    return regs[reg];
}
static bool netif_running(struct net_device *d) { return d->running; }
static bool napi_schedule_prep(struct napi_struct *n) { (void)n; return allow_schedule; }
static void __napi_schedule(struct napi_struct *n)
{ assert(!tx_locked); assert(regs[RTL_RTK_CPUIIMR] == 0); assert(pending_reg == -1); n->schedules++; }
static void mod_timer(struct timer_list *t, unsigned long expires)
{ t->expires=expires; t->arms++; }
static bool rtl8197f_rtk_rx_napi_ready(struct rtl8197f_rtknet_priv *p)
{ ready_calls++; if (discover_publication) p->rx_publish_wait_pending=true; return rx_ready; }
static unsigned int rtl8197f_rtk_tx_clean(struct rtl8197f_rtknet_priv *p)
{ (void)p; tx_clean_calls++; return 2; }
static int rtl8197f_rtk_rx_poll(struct rtl8197f_rtknet_priv *p, int budget)
{ (void)p; assert(budget > 0); rx_poll_calls++; return supplied_rx_work; }
static bool napi_complete_done(struct napi_struct *n, int work)
{ (void)n; (void)work; completions++; return true; }
static void rtl8197f_rtk_assert_trxrdy(struct rtl8197f_rtknet_priv *p)
{ (void)p; trx_calls++; }
FUNCTIONS
static struct rtl8197f_rtknet_priv fresh(struct net_device *d, struct rtl8197f_rtk_desc *ring)
{
    struct rtl8197f_rtknet_priv p = {0};
    p.is_mw5=true; p.rx_poll_fallback=true; p.ndev=d; p.tx_desc=ring;
    p.tx_ring_size=8; p.desc_stride=24; p.tx_desc_dma=0x1000; p.rx_poll_ms=10;
    memset(regs,0,sizeof(regs));
    memset(ring,0,sizeof(*ring)*8);
    reads=writes=ready_calls=tx_clean_calls=rx_poll_calls=completions=trx_calls=0;
    rx_ready=discover_publication=tx_locked=false;
    allow_schedule=true; pending_reg=-1; supplied_rx_work=0;
    return p;
}
static void cdp(struct rtl8197f_rtknet_priv *p, u32 index)
{ regs[RTL_RTK_CPUTPDCR0]=p->tx_desc_dma+index*p->desc_stride; }
int main(void)
{
    struct net_device d = { .running=true };
    struct rtl8197f_rtk_desc ring[8];
    struct rtl8197f_rtknet_priv p = fresh(&d,ring);
    /* Posted masks/doorbells take effect before return, without proc reads. */
    rtl8197f_rtk_enable_irq(&p);
    assert(reads == 1 && writes == 1 && pending_reg == -1);
    assert(regs[RTL_RTK_CPUIIMR] == RTL_RTK_INT_ENABLE_MASK);
    rtl8197f_rtk_disable_irq(&p);
    assert(reads == 2 && writes == 2 && regs[RTL_RTK_CPUIIMR] == 0);
    regs[RTL_RTK_CPUICR]=0x100;
    rtl8197f_rtk_kick_rx(&p);
    assert(regs[RTL_RTK_CPUICR] == 0x101 && p.hw_rx_restarts == 1);
    rtl8197f_rtk_tx_doorbell(&p);
    assert(regs[RTL_RTK_CPUICR] == 0x107 && p.tx_kicks == 1 && pending_reg == -1);
    p.is_mw5=false; reads=0;
    rtl8197f_rtk_enable_irq(&p);
    assert(reads == 0); /* Other boards retain their existing MMIO policy. */

    /* Only valid CDP progress plus OWN release can request TX reclamation. */
    p=fresh(&d,ring); p.tx_tail=1; p.tx_head=3; cdp(&p,2);
    assert(rtl8197f_rtk_mw5_tx_napi_ready(&p));
    assert(p.tx_tail == 1 && p.tx_head == 3 && !tx_locked);
    ring[1].opts1=RTL_RTK_DESC_OWN;
    assert(!rtl8197f_rtk_mw5_tx_napi_ready(&p));
    ring[1].opts1=0;
    cdp(&p,1); assert(!rtl8197f_rtk_mw5_tx_napi_ready(&p));
    cdp(&p,4); assert(!rtl8197f_rtk_mw5_tx_napi_ready(&p));
    regs[RTL_RTK_CPUTPDCR0]=0x1001; assert(!rtl8197f_rtk_mw5_tx_napi_ready(&p));
    regs[RTL_RTK_CPUTPDCR0]=0; assert(!rtl8197f_rtk_mw5_tx_napi_ready(&p));
    p.tx_tail=6; p.tx_head=1; cdp(&p,0);
    assert(rtl8197f_rtk_mw5_tx_napi_ready(&p)); /* ring wrap */
    cdp(&p,2); assert(!rtl8197f_rtk_mw5_tx_napi_ready(&p));
    p.tx_tail=p.tx_head; assert(!rtl8197f_rtk_mw5_tx_napi_ready(&p));
    p.tx_desc=NULL; assert(!rtl8197f_rtk_mw5_tx_napi_ready(&p));

    /* Recent RX interrupts must not hide the final missed RX/TX interrupt. */
    p=fresh(&d,ring); p.irq_rx_done=1; p.tx_tail=1; p.tx_head=3; cdp(&p,2);
    rtl8197f_rtk_rx_poll_timer(&p.rx_poll_timer);
    assert(p.napi.schedules == 1 && p.tx_watchdog_schedules == 1);
    assert(ready_calls == 1 && p.rx_poll_irq_progress_skips == 0);
    assert(p.rx_poll_timer.expires == jiffies+1000);
    assert(p.tx_tail == 1 && p.tx_head == 3); /* No timer-side DMA reuse. */
    p=fresh(&d,ring); p.irq_rx_done=1; rx_ready=true;
    rtl8197f_rtk_rx_poll_timer(&p.rx_poll_timer);
    assert(p.napi.schedules == 1 && p.tx_watchdog_schedules == 0);

    /* Idle/owned rings do not cause gratuitous NAPI wakeups. */
    p=fresh(&d,ring); p.irq_rx_done=1;
    rtl8197f_rtk_rx_poll_timer(&p.rx_poll_timer);
    assert(p.napi.schedules == 0 && p.rx_poll_timer.arms == 1);
    p.tx_tail=1; p.tx_head=3; cdp(&p,2); ring[1].opts1=RTL_RTK_DESC_OWN;
    rtl8197f_rtk_rx_poll_timer(&p.rx_poll_timer);
    assert(p.napi.schedules == 0);
    ring[1].opts1=0; allow_schedule=false;
    rtl8197f_rtk_rx_poll_timer(&p.rx_poll_timer);
    assert(p.napi.schedules == 0 && p.tx_watchdog_schedules == 0);

    /* Discover publication waits during this pass; retry in 1 ms, not 1 s. */
    p=fresh(&d,ring); p.irq_rx_done=1; discover_publication=true;
    rtl8197f_rtk_rx_poll_timer(&p.rx_poll_timer);
    assert(p.napi.schedules == 0 && p.rx_publish_wait_pending);
    assert(p.rx_poll_timer.expires == jiffies+1);
    p=fresh(&d,ring); p.irq_rx_done=1; p.rx_runout_guard_until=jiffies+100;
    rtl8197f_rtk_rx_poll_timer(&p.rx_poll_timer);
    assert(p.rx_poll_timer.expires == jiffies+10);

    /* Stop/fallback-disabled must neither access rings nor rearm timers. */
    p=fresh(&d,ring); d.running=false;
    rtl8197f_rtk_rx_poll_timer(&p.rx_poll_timer);
    assert(!reads && !ready_calls && !p.rx_poll_timer.arms);
    d.running=true; p.rx_poll_fallback=false;
    rtl8197f_rtk_rx_poll_timer(&p.rx_poll_timer);
    assert(!reads && !ready_calls && !p.rx_poll_timer.arms);
    p=fresh(&d,ring); p.is_mw5=false; p.irq_rx_done=1;
    rtl8197f_rtk_rx_poll_timer(&p.rx_poll_timer);
    assert(!ready_calls && p.rx_poll_irq_progress_skips == 1);

    /* TX-only NAPI can reclaim TX but cannot consume RX recovery state. */
    p=fresh(&d,ring); p.rx_restart_pending=true; p.rx_runout_pending=true;
    assert(rtl8197f_rtk_poll(&p.napi,0) == 0);
    assert(tx_clean_calls == 1 && !rx_poll_calls && !completions && !trx_calls);
    assert(p.napi_tx_clean == 2);
    assert(p.rx_restart_pending && p.rx_runout_pending && !p.napi_budget_polls);
    assert(rtl8197f_rtk_poll(&p.napi,64) == 0);
    assert(rx_poll_calls == 1 && completions == 1 && trx_calls == 1);
    assert(p.napi_tx_clean == 4);
    assert(!p.rx_restart_pending && !p.rx_runout_pending && p.hw_rx_restarts == 1);
    assert(regs[RTL_RTK_CPUIIMR] == RTL_RTK_INT_ENABLE_MASK);
    p=fresh(&d,ring); supplied_rx_work=64;
    assert(rtl8197f_rtk_poll(&p.napi,64) == 64);
    assert(!completions && p.napi_budget_polls == 1);
    return 0;
}
'''.replace("FUNCTIONS", functions)
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "watchdog.c"
            binary = Path(tmp) / "watchdog"
            source.write_text(program)
            subprocess.run(["cc", "-std=gnu11", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=undefined", str(source), "-o", str(binary)],
                           check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    unittest.main()
