// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Xiaomi RD05 / RTL8197FH-VG RTL8367D OEM switch pre-initialisation.
 *
 * The Realtek GPL/SDK code does not leave the external 83xx/8367 switch reset
 * pin to a generic DSA reset handler.  For RTL8197F_VG boards it first selects
 * the LED_P0/H2 pinmux function, configures H2 as GPIO, and drives the switch
 * reset line high before the 83xx/8367 SMI probe runs.  The relevant SDK
 * sequence in rtl865x_asicL2.c is:
 *
 *   PIN_MUX_SEL14 = (PIN_MUX_SEL14 & ~(0xf << 28)) | (2 << 28);
 *   PEFGH_CNR &= ~(1 << gpio_reset);
 *   PEFGH_DIR |=  (1 << gpio_reset);
 *   PEFGH_DAT |=  (1 << gpio_reset);
 *
 * On RD05, the OEM userspace exposes rtl819x_8367r_reset_pin.0 as GPIO58.
 * In the RTL8197F GPIO register block this is PEFGH bit 26 (H2).  The native
 * realtek-smi probe runs very early; if H2 remains low at that moment the
 * external RTL8367D never ACKs the first SMI command.  This tiny initcall only
 * performs the OEM deassert-high step before platform devices are probed.  It
 * intentionally does not pulse reset low.
 *
 * This file also registers an RD05-specific restart/poweroff fallback.  The
 * generic MIPS fallback only halts on this board; the vendor BSP uses the
 * legacy RTL8197F one-register watchdog block at 0x1800311c.  The upstream
 * Realtek Otto watchdog driver is for the newer 0xc-byte 83xx/93xx block and
 * is not compatible with this RTL8197F register layout, so keep this tiny
 * board-local reset hook here instead of enabling the generic watchdog node.
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/pm.h>
#include <linux/printk.h>
#include <linux/reboot.h>

#define RTL8197F_SYS_BASE	0x18000000
#define RTL8197F_SYS_SIZE	0x00010000
#define RTL8197F_PINMUX14	0x0838
#define RTL8197F_SYS_CLK_MAG	0x0010
#define RTL8197F_WDTCNR		0x311c

#define RTL8197F_GPIO_BASE	0x18003500
#define RTL8197F_GPIO_SIZE	0x00000038
#define RTL8197F_PEFGH_CNR	0x001c
#define RTL8197F_PEFGH_DIR	0x0024
#define RTL8197F_PEFGH_DAT	0x0028

#define RD05_RTL8367_RESET_H2_BIT	26
#define RD05_RTL8367_MDC_H0_BIT		24
#define RD05_RTL8367_MDIO_G7_BIT		23

/* RTL8197F legacy one-register watchdog bits from the Realtek BSP. */
#define RTL8197F_WDT_ENABLE		BIT(24)
#define RTL8197F_WDT_CLEAR		BIT(23)
#define RTL8197F_WDT_OVSEL_MASK		(BIT(22) | BIT(21))
#define RTL8197F_WDT_OVSEL_15		0
#define RTL8197F_WDT_STOP_PATTERN	(0xa5u << 24)

/* Vendor SYS_CLK_MAG bit used on related 819x parts for software reset. */
#define RTL8197F_SYS_SW_RESET		BIT(11)

static void __iomem *rd05_sys_base;

static void rtl8197f_rd05_assert_reset(void)
{
	u32 v;

	if (!rd05_sys_base)
		return;

	/*
	 * First arm the legacy watchdog for its shortest overflow period.  Do not
	 * write the 0xa5 stop pattern: on the RTL819x BSP this disables the WDT.
	 */
	writel(RTL8197F_WDT_CLEAR, rd05_sys_base + RTL8197F_WDTCNR);
	v = readl(rd05_sys_base + RTL8197F_WDTCNR);
	v &= ~RTL8197F_WDT_OVSEL_MASK;
	v &= ~RTL8197F_WDT_STOP_PATTERN;
	v |= RTL8197F_WDT_ENABLE | RTL8197F_WDT_OVSEL_15;
	writel(v, rd05_sys_base + RTL8197F_WDTCNR);

	/*
	 * Some RTL819x revisions also expose a software reset latch in SYS_CLK_MAG.
	 * Toggle it as a belt-and-braces fallback while the watchdog is counting.
	 */
	v = readl(rd05_sys_base + RTL8197F_SYS_CLK_MAG);
	writel(v & ~RTL8197F_SYS_SW_RESET, rd05_sys_base + RTL8197F_SYS_CLK_MAG);
	udelay(1000);
	writel(v | RTL8197F_SYS_SW_RESET, rd05_sys_base + RTL8197F_SYS_CLK_MAG);

	mdelay(2500);
}

static int rtl8197f_rd05_restart_handler(struct notifier_block *nb,
						 unsigned long action, void *data)
{
	pr_emerg("rd05-restart: asserting RTL8197F watchdog/software reset\n");
	rtl8197f_rd05_assert_reset();
	return NOTIFY_DONE;
}

static struct notifier_block rtl8197f_rd05_restart_nb = {
	.notifier_call = rtl8197f_rd05_restart_handler,
	.priority = 192,
};

static void rtl8197f_rd05_poweroff(void)
{
	pr_emerg("rd05-poweroff: no soft power-off path; resetting via RTL8197F watchdog\n");
	rtl8197f_rd05_assert_reset();
}

static int __init rtl8197f_rd05_oem_switch_preinit(void)
{
	void __iomem *gpio;
	u32 pinmux14;
	u32 cnr, dir, dat;
	int ret;

	if (!of_machine_is_compatible("xiaomi,r4-rd05"))
		return 0;

	rd05_sys_base = ioremap(RTL8197F_SYS_BASE, RTL8197F_SYS_SIZE);
	if (!rd05_sys_base) {
		pr_warn("rd05-oem-switch-preinit: failed to map RTL8197F system registers\n");
		return 0;
	}

	gpio = ioremap(RTL8197F_GPIO_BASE, RTL8197F_GPIO_SIZE);
	if (!gpio) {
		pr_warn("rd05-oem-switch-preinit: failed to map RTL8197F GPIO registers\n");
		return 0;
	}

	/* OEM/GPL: select the H2/LED_P0 pinmux mode used for 83xx reset. */
	pinmux14 = readl(rd05_sys_base + RTL8197F_PINMUX14);
	pinmux14 = (pinmux14 & ~(0xf << 28)) | (2 << 28);
	writel(pinmux14, rd05_sys_base + RTL8197F_PINMUX14);

	cnr = readl(gpio + RTL8197F_PEFGH_CNR);
	dir = readl(gpio + RTL8197F_PEFGH_DIR);
	dat = readl(gpio + RTL8197F_PEFGH_DAT);

	/* Ensure H0/G7/H2 are GPIO-controlled before DSA touches SMI/reset. */
	cnr &= ~BIT(RD05_RTL8367_MDC_H0_BIT);
	cnr &= ~BIT(RD05_RTL8367_MDIO_G7_BIT);
	cnr &= ~BIT(RD05_RTL8367_RESET_H2_BIT);

	/* Only H2 is driven here: deassert RTL8367D reset high, no low pulse. */
	dir |= BIT(RD05_RTL8367_RESET_H2_BIT);
	dat |= BIT(RD05_RTL8367_RESET_H2_BIT);

	writel(cnr, gpio + RTL8197F_PEFGH_CNR);
	writel(dat, gpio + RTL8197F_PEFGH_DAT);
	writel(dir, gpio + RTL8197F_PEFGH_DIR);

	pr_info("rd05-oem-switch-preinit: RTL8367D GPIO58/H2 reset deasserted high; pinmux14=0x%08x PEFGH_CNR=0x%08x DIR=0x%08x DAT=0x%08x\n",
		pinmux14, cnr, dir, dat);

	iounmap(gpio);

	/* Stop any stale WDT state left by a previous crash/recovery boot. */
	writel(RTL8197F_WDT_STOP_PATTERN, rd05_sys_base + RTL8197F_WDTCNR);

	ret = register_restart_handler(&rtl8197f_rd05_restart_nb);
	if (ret)
		pr_warn("rd05-restart: failed to register restart handler: %d\n", ret);
	else
		pr_info("rd05-restart: registered RTL8197F watchdog/software reset handler\n");

	pm_power_off = rtl8197f_rd05_poweroff;

	return 0;
}
postcore_initcall(rtl8197f_rd05_oem_switch_preinit);
