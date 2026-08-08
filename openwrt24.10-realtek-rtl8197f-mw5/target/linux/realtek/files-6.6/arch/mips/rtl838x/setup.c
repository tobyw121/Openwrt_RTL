// SPDX-License-Identifier: GPL-2.0-only
/*
 * Setup for the Realtek RTL838X SoC:
 *	Memory, Timer and Serial
 *
 * Copyright (C) 2020 B. Koblitz
 * based on the original BSP by
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 *
 */

#include <linux/console.h>
#include <linux/init.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/of_fdt.h>
#include <linux/irqchip.h>
#include <linux/bits.h>

#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/bootinfo.h>
#include <asm/time.h>
#include <asm/prom.h>
#include <asm/irq.h>
#include <asm/mipsregs.h>
#include <asm/smp-ops.h>

#include "mach-rtl83xx.h"

extern struct rtl83xx_soc_info soc_info;


/*
 * RTL8197F vendor BSP interrupt routing.
 *
 * The RD05 boots far enough with the generic Realtek IRQ controller, but the
 * late userspace log stays at [0.000000] and OpenWrt can stall after the first
 * kmodloader phase when the CPU interrupt mask/routing is not restored exactly
 * like the SDK.  Program the 8197F ICTL routing table before Linux drivers
 * start requesting peripheral IRQs and unmask the CPU interrupt lines used by
 * the external controller and by the CP0 Count/Compare timer.
 */
#define RTL8197F_ICTL_BASE       0x18003000
#define RTL8197F_ICTL_GIMR       0x00
#define RTL8197F_ICTL_GISR       0x04
#define RTL8197F_ICTL_IRR0       0x08
#define RTL8197F_ICTL_IRR1       0x0c
#define RTL8197F_ICTL_IRR2       0x10
#define RTL8197F_ICTL_IRR3       0x14
#define RTL8197F_ICTL_GIMR2      0x20
#define RTL8197F_ICTL_GIMR2_CPU_SI_TIMER_IE BIT(15)
#define RTL8197F_ICTL_GISR2      0x24
#define RTL8197F_ICTL_IRR4       0x28
#define RTL8197F_ICTL_IRR5       0x2c
#define RTL8197F_ICTL_IRR6       0x30
#define RTL8197F_ICTL_IRR7       0x34

#define RTL8197F_IRR_IPTOCPU(a7, a6, a5, a4, a3, a2, a1, a0) \
	(((a7) << 28) | ((a6) << 24) | ((a5) << 20) | ((a4) << 16) | \
	 ((a3) << 12) | ((a2) << 8) | ((a1) << 4) | ((a0) << 0))

#define RTL8197F_IRR0_SETTING	RTL8197F_IRR_IPTOCPU(0, 0, 0, 0, 0, 0, 0, 0)
#define RTL8197F_IRR1_SETTING	RTL8197F_IRR_IPTOCPU(4, 2, 2, 2, 0, 0, 2, 0)
#define RTL8197F_IRR2_SETTING	RTL8197F_IRR_IPTOCPU(2, 0, 5, 0, 0, 0, 2, 2)
#define RTL8197F_IRR3_SETTING	RTL8197F_IRR_IPTOCPU(0, 2, 6, 0, 0, 2, 0, 0)
#define RTL8197F_IRR4_SETTING	RTL8197F_IRR_IPTOCPU(0, 0, 2, 0, 2, 0, 0, 0)
#define RTL8197F_IRR5_SETTING	RTL8197F_IRR_IPTOCPU(7, 0, 0, 0, 0, 0, 0, 0)
#define RTL8197F_IRR6_SETTING	RTL8197F_IRR_IPTOCPU(0, 0, 0, 0, 0, 0, 0, 0)
#define RTL8197F_IRR7_SETTING	RTL8197F_IRR_IPTOCPU(0, 0, 0, 0, 0, 0, 0, 0)

static void __init rtl8197f_init_irq_routing(void)
{
	void __iomem *ictl = (void __iomem *)CKSEG1ADDR(RTL8197F_ICTL_BASE);

	writel(RTL8197F_IRR0_SETTING, ictl + RTL8197F_ICTL_IRR0);
	writel(RTL8197F_IRR1_SETTING, ictl + RTL8197F_ICTL_IRR1);
	writel(RTL8197F_IRR2_SETTING, ictl + RTL8197F_ICTL_IRR2);
	writel(RTL8197F_IRR3_SETTING, ictl + RTL8197F_ICTL_IRR3);
	writel(RTL8197F_IRR4_SETTING, ictl + RTL8197F_ICTL_IRR4);
	writel(RTL8197F_IRR5_SETTING, ictl + RTL8197F_ICTL_IRR5);
	writel(RTL8197F_IRR6_SETTING, ictl + RTL8197F_ICTL_IRR6);
	writel(RTL8197F_IRR7_SETTING, ictl + RTL8197F_ICTL_IRR7);

	/*
	 * Clear stale pending ICTL bits.  Linux irqchip code owns most
	 * peripheral GIMR masks, but the RTL8197F vendor BSP treats the CP0
	 * Count/Compare timer as CPU_SI_TIMER_IP in GISR2 bit 15 and enables
	 * that line explicitly via GIMR2.  Without this bit, the kernel can
	 * print and execute PID1, but timer sleeps never wake after the first
	 * kthread heartbeat.
	 */
	writel(0xffffffff, ictl + RTL8197F_ICTL_GISR);
	writel(0xffffffff, ictl + RTL8197F_ICTL_GISR2);
	writel(readl(ictl + RTL8197F_ICTL_GIMR2) | RTL8197F_ICTL_GIMR2_CPU_SI_TIMER_IE,
	       ictl + RTL8197F_ICTL_GIMR2);

	/* SDK enables CPU interrupt inputs for both ICTL and CP0 Compare. */
	set_c0_status(ST0_IM);

	pr_info("RTL8197F: OEM SDK IRQ routing restored (UART0 IP9->IRR=2/output-index=1, CPU_SI_TIMER GIMR2 bit15 on), c0_status=0x%08x gimr2=0x%08x\n",
		read_c0_status(), readl(ictl + RTL8197F_ICTL_GIMR2));
}

/*
 * RTL8197F routes the CP0 Count/Compare timer to CPU interrupt line 7 in
 * the vendor BSP.  Keep this as a strong platform override so the R4K
 * clockevent requests the intended Linux CPU IRQ even if cp0_compare_irq was
 * left at a SoC-default value by generic MIPS probing.
 */
unsigned int get_c0_compare_int(void)
{
	if (IS_ENABLED(CONFIG_RTL8197F))
		return MIPS_CPU_IRQ_BASE + 7;

	return MIPS_CPU_IRQ_BASE + cp0_compare_irq;
}


void __init plat_mem_setup(void)
{
	void *dtb;

	set_io_port_base(KSEG1);

	dtb = get_fdt();
	if (!dtb)
		panic("no dtb found");

	/*
	 * Load the devicetree. This causes the chosen node to be
	 * parsed resulting in our memory appearing
	 */
	__dt_setup_arch(dtb);
}

void plat_time_init_fallback(void)
{
	struct device_node *np;
	u32 freq = 500000000;

	np = of_find_node_by_name(NULL, "cpus");
	if (!np) {
		pr_err("Missing 'cpus' DT node, using default frequency.");
	} else {
		if (of_property_read_u32(np, "frequency", &freq) < 0)
			pr_err("No 'frequency' property in DT, using default.");
		else
			pr_info("CPU frequency from device tree: %dMHz", freq / 1000000);
		of_node_put(np);
	}
	mips_hpt_frequency = freq / 2;
}

void __init plat_time_init(void)
{
/*
 * Initialization routine resembles generic MIPS plat_time_init() with
 * lazy error handling. The final fallback is only needed until we have
 * converted all device trees to new clock syntax.
 */
	struct device_node *np;
	struct clk *clk;

	of_clk_init(NULL);

	mips_hpt_frequency = 0;
	np = of_get_cpu_node(0, NULL);
	if (!np) {
		pr_err("Failed to get CPU node\n");
	} else {
		clk = of_clk_get(np, 0);
		if (IS_ERR(clk)) {
			pr_err("Failed to get CPU clock: %ld\n", PTR_ERR(clk));
		} else {
			mips_hpt_frequency = clk_get_rate(clk) / 2;
			clk_put(clk);
		}
	}

	if (!mips_hpt_frequency)
		plat_time_init_fallback();

	/*
	 * RTL8197F/RD05 uses the MIPS R4K CP0 Count/Compare timer in the
	 * vendor BSP.  The inherited RTL83xx platform calls timer_probe() for
	 * the Realtek Otto/DW timer block; that block is not compatible with
	 * RTL8197F's TC register layout and the RD05 DT deliberately keeps it
	 * disabled for first bring-up.  Calling timer_probe() therefore produces
	 * "timer_probe: no matching timers found" and may leave early boot
	 * without the expected CP0 compare IRQ selection.
	 *
	 * Vendor RTL8197F BSP definitions:
	 *   BSP_COMPARE_IRQ   = BSP_IRQ_CPU_BASE + 7
	 *   BSP_PERFCOUNT_IRQ = BSP_IRQ_CPU_BASE + 7
	 * Since both share the same CPU interrupt line, disable perfcount IRQ and
	 * let CONFIG_CEVT_R4K/CONFIG_CSRC_R4K provide the clockevent/clocksource.
	 */
	if (IS_ENABLED(CONFIG_RTL8197F)) {
		/* fixed 1GHz CPU / 500MHz Count clock on RD05/RTL8197F-VG */
		mips_hpt_frequency = 500000000;
		cp0_compare_irq = 7;
		cp0_perfcount_irq = -1;
		pr_info("RTL8197F: using R4K CP0 timer, hpt_frequency=%u Hz, irq=%d\n",
			mips_hpt_frequency, cp0_compare_irq);
		return;
	}

	timer_probe();
}

void __init arch_init_irq(void)
{
	if (IS_ENABLED(CONFIG_RTL8197F))
		rtl8197f_init_irq_routing();

	irqchip_init();

	if (IS_ENABLED(CONFIG_RTL8197F)) {
		void __iomem *ictl = (void __iomem *)CKSEG1ADDR(RTL8197F_ICTL_BASE);

		/* Re-assert the SDK's CPU_SI_TIMER mask after irqchip setup. */
		writel(readl(ictl + RTL8197F_ICTL_GIMR2) | RTL8197F_ICTL_GIMR2_CPU_SI_TIMER_IE,
		       ictl + RTL8197F_ICTL_GIMR2);
		set_c0_status(ST0_IM);
	}
}

static int __init rtl8197f_platform_late_init(void)
{
	if (!IS_ENABLED(CONFIG_RTL8197F))
		return 0;

	pr_info("RTL8197F: late platform init reached, hpt_frequency=%u Hz, c0_status=0x%08x, compare_irq=%u\n",
		mips_hpt_frequency, read_c0_status(), get_c0_compare_int());
	return 0;
}
late_initcall(rtl8197f_platform_late_init);

/* RD05 bring-up heartbeat removed for standard OpenWrt boot. */


