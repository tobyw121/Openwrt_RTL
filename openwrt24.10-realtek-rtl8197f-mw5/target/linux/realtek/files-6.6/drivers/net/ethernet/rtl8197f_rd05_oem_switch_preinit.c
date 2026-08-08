// SPDX-License-Identifier: GPL-2.0-only
/*
 * RTL8197F board-local external switch GPIO/pinmux pre-initialisation.
 *
 * The Realtek SMI device probes before ordinary userspace.  Xiaomi RD05 and
 * Tenda Nova MW5 need their OEM pinmux/reset state established before the
 * rtl8365mb DSA driver sends its first SMI transaction.
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
#define RTL8197F_PINMUX0	0x0800
#define RTL8197F_PINMUX1	0x0804
#define RTL8197F_PINMUX2	0x0808
#define RTL8197F_PINMUX14	0x0838
#define RTL8197F_SYS_CLK_MAG	0x0010
#define RTL8197F_WDTCNR		0x311c

#define RTL8197F_GPIO_BASE	0x18003500
#define RTL8197F_GPIO_SIZE	0x00000038
#define RTL8197F_PABCD_CNR	0x0000
#define RTL8197F_PABCD_DIR	0x0008
#define RTL8197F_PABCD_DAT	0x000c
#define RTL8197F_PEFGH_CNR	0x001c
#define RTL8197F_PEFGH_DIR	0x0024
#define RTL8197F_PEFGH_DAT	0x0028

#define RD05_RESET_H2_BIT	26
#define RD05_MDC_H0_BIT		24
#define RD05_MDIO_G7_BIT	23

#define MW5_MDC_C2_BIT		18
#define MW5_MDIO_C3_BIT		19
#define MW5_RESET_H2_BIT	26

#define RTL8197F_WDT_ENABLE		BIT(24)
#define RTL8197F_WDT_CLEAR		BIT(23)
#define RTL8197F_WDT_OVSEL_MASK	(BIT(22) | BIT(21))
#define RTL8197F_WDT_OVSEL_15		0
#define RTL8197F_WDT_STOP_PATTERN	(0xa5u << 24)
#define RTL8197F_SYS_SW_RESET		BIT(11)

static void __iomem *rtl8197f_sys_base;

/*
 * The RTL8197F vendor watchdog driver documents that a bare write of the
 * 0xa5000000 stop pattern can occasionally reset the SoC immediately.
 * Always reload the counter first, then write the exact stop pattern and
 * perform readbacks before the long MW5 switch-reset delays.
 */
static void rtl8197f_stop_watchdog(void)
{
	u32 v;

	if (!rtl8197f_sys_base)
		return;

	v = readl(rtl8197f_sys_base + RTL8197F_WDTCNR);
	writel(v | RTL8197F_WDT_CLEAR,
	       rtl8197f_sys_base + RTL8197F_WDTCNR);
	readl(rtl8197f_sys_base + RTL8197F_WDTCNR);
	writel(RTL8197F_WDT_STOP_PATTERN,
	       rtl8197f_sys_base + RTL8197F_WDTCNR);
	readl(rtl8197f_sys_base + RTL8197F_WDTCNR);
}

static void rtl8197f_assert_reset(void)
{
	u32 v;

	if (!rtl8197f_sys_base)
		return;

	writel(RTL8197F_WDT_CLEAR, rtl8197f_sys_base + RTL8197F_WDTCNR);
	v = readl(rtl8197f_sys_base + RTL8197F_WDTCNR);
	v &= ~RTL8197F_WDT_OVSEL_MASK;
	v &= ~RTL8197F_WDT_STOP_PATTERN;
	v |= RTL8197F_WDT_ENABLE | RTL8197F_WDT_OVSEL_15;
	writel(v, rtl8197f_sys_base + RTL8197F_WDTCNR);

	v = readl(rtl8197f_sys_base + RTL8197F_SYS_CLK_MAG);
	writel(v & ~RTL8197F_SYS_SW_RESET,
	       rtl8197f_sys_base + RTL8197F_SYS_CLK_MAG);
	udelay(1000);
	writel(v | RTL8197F_SYS_SW_RESET,
	       rtl8197f_sys_base + RTL8197F_SYS_CLK_MAG);

	mdelay(2500);
}

static int rtl8197f_restart_handler(struct notifier_block *nb,
				    unsigned long action, void *data)
{
	pr_emerg("rtl8197f-restart: asserting watchdog/software reset\n");
	rtl8197f_assert_reset();
	return NOTIFY_DONE;
}

static struct notifier_block rtl8197f_restart_nb = {
	.notifier_call = rtl8197f_restart_handler,
	.priority = 192,
};

static void rtl8197f_poweroff(void)
{
	pr_emerg("rtl8197f-poweroff: no soft power-off path; resetting\n");
	rtl8197f_assert_reset();
}

static void rtl8197f_rd05_switch_gpio_init(void __iomem *gpio)
{
	u32 pinmux14, cnr, dir, dat;

	pinmux14 = readl(rtl8197f_sys_base + RTL8197F_PINMUX14);
	pinmux14 = (pinmux14 & ~(0xf << 28)) | (2 << 28);
	writel(pinmux14, rtl8197f_sys_base + RTL8197F_PINMUX14);

	cnr = readl(gpio + RTL8197F_PEFGH_CNR);
	dir = readl(gpio + RTL8197F_PEFGH_DIR);
	dat = readl(gpio + RTL8197F_PEFGH_DAT);
	cnr &= ~BIT(RD05_MDC_H0_BIT);
	cnr &= ~BIT(RD05_MDIO_G7_BIT);
	cnr &= ~BIT(RD05_RESET_H2_BIT);
	dir |= BIT(RD05_RESET_H2_BIT);
	dat |= BIT(RD05_RESET_H2_BIT);
	writel(cnr, gpio + RTL8197F_PEFGH_CNR);
	writel(dat, gpio + RTL8197F_PEFGH_DAT);
	writel(dir, gpio + RTL8197F_PEFGH_DIR);

	pr_info("rtl8197f-switch-preinit: RD05 H2 reset high, SMI H0/G7 prepared\n");
}

static void rtl8197f_mw5_switch_gpio_init(void __iomem *gpio)
{
	u32 abcd_cnr, abcd_dir, abcd_dat;
	u32 efgh_cnr, efgh_dir, efgh_dat;

	/* Exact RTL8197FS vendor sequence before C2/C3 SMI setup. */
	writel(0, rtl8197f_sys_base + RTL8197F_PINMUX0);
	writel(0, rtl8197f_sys_base + RTL8197F_PINMUX1);

	abcd_cnr = readl(gpio + RTL8197F_PABCD_CNR);
	abcd_dir = readl(gpio + RTL8197F_PABCD_DIR);
	abcd_dat = readl(gpio + RTL8197F_PABCD_DAT);
	abcd_cnr &= ~(BIT(MW5_MDC_C2_BIT) | BIT(MW5_MDIO_C3_BIT));
	abcd_dir |= BIT(MW5_MDC_C2_BIT) | BIT(MW5_MDIO_C3_BIT);
	abcd_dat |= BIT(MW5_MDC_C2_BIT) | BIT(MW5_MDIO_C3_BIT);
	writel(abcd_cnr, gpio + RTL8197F_PABCD_CNR);
	writel(abcd_dat, gpio + RTL8197F_PABCD_DAT);
	writel(abcd_dir, gpio + RTL8197F_PABCD_DIR);

	efgh_cnr = readl(gpio + RTL8197F_PEFGH_CNR);
	efgh_dir = readl(gpio + RTL8197F_PEFGH_DIR);
	efgh_dat = readl(gpio + RTL8197F_PEFGH_DAT);
	efgh_cnr &= ~BIT(MW5_RESET_H2_BIT);
	efgh_dir |= BIT(MW5_RESET_H2_BIT);
	efgh_dat &= ~BIT(MW5_RESET_H2_BIT);
	writel(efgh_cnr, gpio + RTL8197F_PEFGH_CNR);
	writel(efgh_dat, gpio + RTL8197F_PEFGH_DAT);
	writel(efgh_dir, gpio + RTL8197F_PEFGH_DIR);
	msleep(1000);

	efgh_dat |= BIT(MW5_RESET_H2_BIT);
	writel(efgh_dat, gpio + RTL8197F_PEFGH_DAT);
	msleep(1000);

	/* Vendor PIN_MUX_SEL2 value selecting C2/C3 for the 83xx SMI path. */
	writel(0x0660, rtl8197f_sys_base + RTL8197F_PINMUX2);

	pr_info("rtl8197f-switch-preinit: MW5 SMI C2/C3, H2 reset low/high 1s, PINMUX2=0x0660\n");
}

static int __init rtl8197f_oem_switch_preinit(void)
{
	bool rd05 = of_machine_is_compatible("xiaomi,r4-rd05");
	bool mw5 = of_machine_is_compatible("tenda,nova-mw5");
	void __iomem *gpio;
	int ret;

	if (!rd05 && !mw5)
		return 0;

	rtl8197f_sys_base = ioremap(RTL8197F_SYS_BASE, RTL8197F_SYS_SIZE);
	if (!rtl8197f_sys_base) {
		pr_warn("rtl8197f-switch-preinit: failed to map system registers\n");
		return 0;
	}

	/* Stop the bootloader watchdog before the MW5 2 x 1 s reset delays. */
	rtl8197f_stop_watchdog();

	gpio = ioremap(RTL8197F_GPIO_BASE, RTL8197F_GPIO_SIZE);
	if (!gpio) {
		pr_warn("rtl8197f-switch-preinit: failed to map GPIO registers\n");
		return 0;
	}

	if (rd05)
		rtl8197f_rd05_switch_gpio_init(gpio);
	else
		rtl8197f_mw5_switch_gpio_init(gpio);

	iounmap(gpio);

	/* Repeat the safe kick-then-stop sequence after switch reset/pinmux. */
	rtl8197f_stop_watchdog();
	ret = register_restart_handler(&rtl8197f_restart_nb);
	if (ret)
		pr_warn("rtl8197f-restart: handler registration failed: %d\n", ret);
	else
		pr_info("rtl8197f-restart: watchdog/software reset handler registered\n");

	pm_power_off = rtl8197f_poweroff;
	return 0;
}
postcore_initcall(rtl8197f_oem_switch_preinit);
