#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Run the production strap decoder and CP0 timer policy natively."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT, c_function

CLOCK = ROOT / 'target/linux/realtek/files-6.6/drivers/clk/clk-rtl8197f.c'
SETUP = ROOT / 'target/linux/realtek/files-6.6/arch/mips/rtl838x/setup.c'


class RTL8197FCPUClockTests(unittest.TestCase):
    def test_strap_decode_and_timer_policy(self):
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>
typedef uint32_t u32;
#define __init
#define BIT(n) (1U << (n))
#define IS_ENABLED(x) is_rtl
#define IS_ERR(p) (!(p))
#define PTR_ERR(p) (-1L)
#define pr_err(...) ((void)0)
#define pr_info(...) ((void)0)
struct device_node {}; struct clk {};
static struct device_node node;
static struct clk cpu_clk;
static const char *board;
static bool is_rtl = true, failed_clock, failed_node;
static unsigned int mips_hpt_frequency;
static int cp0_compare_irq, cp0_perfcount_irq, probes, fallbacks;
static unsigned long rate;
static jmp_buf panic_target;
static bool of_machine_is_compatible(const char *s) { return !strcmp(s,board); }
static void of_clk_init(void *p) {}
static struct device_node *of_get_cpu_node(int n, void *p) { return failed_node ? 0 : &node; }
static struct clk *of_clk_get(struct device_node *p, int n) { return failed_clock ? 0 : &cpu_clk; }
static unsigned long clk_get_rate(struct clk *p) { return rate; }
static void clk_put(struct clk *p) {}
static void plat_time_init_fallback(void) { fallbacks++; mips_hpt_frequency=250000000; }
static void timer_probe(void) { probes++; }
static void panic(const char *s) { longjmp(panic_target,1); }
''' + c_function(CLOCK, 'static unsigned long rtl8197f_cpu_rate(') + '\n' + \
            c_function(SETUP, 'void __init plat_time_init(') + r'''
int main(void) {
    const char *boards[] = {"cudy,wr1500", "cudy,re1500", "cudy,m1500",
                           "xiaomi,r4-rd05", "tenda,nova-mw5", "tenda,ac23"};
    const unsigned int table[] = {450,500,550,600,650,700,750,800,
                                  850,900,950,1000,1050,1100,1150,1200};
    const unsigned int divisors[] = {1,2,4,8};
    for (int s=0;s<16;s++) for (int d=0;d<4;d++) for (int low=0;low<2;low++) {
        u32 strap = s<<15 | d<<19 | low<<14;
        unsigned long expected = low ? 200000000 : table[s]*1000000UL/divisors[d];
        assert(rtl8197f_cpu_rate(strap) == expected);
        assert(rtl8197f_cpu_rate(strap | ~(0x7fU<<14)) == expected);
        for (int b=0;b<6;b++) {
            board=boards[b]; rate=rtl8197f_cpu_rate(strap);
            plat_time_init();
            assert(mips_hpt_frequency == (b<3 ? expected/2 : 500000000));
            assert(cp0_compare_irq==7 && cp0_perfcount_irq==-1);
            assert(probes==0 && fallbacks==0);
        }
    }
    /* No guessed fallback timer on a Cudy clock-provider failure. */
    board=boards[0]; failed_clock=true;
    if (!setjmp(panic_target)) { plat_time_init(); assert(0); }
    assert(fallbacks==0);
    failed_clock=false; failed_node=true;
    if (!setjmp(panic_target)) { plat_time_init(); assert(0); }
    failed_node=false; rate=0;
    if (!setjmp(panic_target)) { plat_time_init(); assert(0); }
    /* Existing boards retain the previous fallback/override policy. */
    board=boards[3]; plat_time_init();
    assert(mips_hpt_frequency==500000000 && fallbacks==1);
    is_rtl=false; rate=800000000; plat_time_init();
    assert(mips_hpt_frequency==400000000 && probes==1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / 'clock.c'
            binary = Path(tmp) / 'clock'
            source.write_text(code)
            subprocess.run(['cc', '-std=gnu11', '-Werror', '-fsanitize=undefined',
                            str(source), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
