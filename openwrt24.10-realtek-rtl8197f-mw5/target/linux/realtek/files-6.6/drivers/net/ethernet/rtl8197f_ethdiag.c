// SPDX-License-Identifier: GPL-2.0-only
/*
 * RTL8197F Ethernet/MDIO/GPIO-SMI diagnostics for RD05 bring-up.
 *
 * v37 notes:
 * - This variant is for a native DSA probe attempt.  The DTS switch node owns
 *   GPIO-SMI and GPIO58 reset.
 * - ethdiag remains enabled only as a read-only/coexistence logger.  It performs
 *   register dumps, bounded internal MDCIO reads, and a GPIO level test, but it
 *   deliberately skips GPIO-SMI register transactions and never asserts GPIO58.
 *
 * Diagnostic only: no netdev, no VLANs, no DSA switch configuration and no
 * persistent GPIO ownership.
 */
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

struct rtl8197f_ethdiag_reg {
	const char *name;
	u32 off;
};

struct rtl8197f_ethdiag_window {
	const char *name;
	phys_addr_t phys;
	u32 size;
	const struct rtl8197f_ethdiag_reg *regs;
	unsigned int nregs;
};

struct rtl8197f_smi_gpio_bus {
	const char *name;
	unsigned int mdc_gpio;
	unsigned int mdio_gpio;
};

struct rtl8197f_reset_candidate {
	const char *name;
	unsigned int gpio;
	bool active_low;
};

#define ARRAYSZ(a) (sizeof(a) / sizeof((a)[0]))

/* RTL8197F GPIO block, relative to 0x18003500. */
#define RTL8197F_GPIO_PABCD_CNR	0x00
#define RTL8197F_GPIO_PABCD_DIR	0x08
#define RTL8197F_GPIO_PABCD_DAT	0x0c
#define RTL8197F_GPIO_PEFGH_CNR	0x1c
#define RTL8197F_GPIO_PEFGH_DIR	0x24
#define RTL8197F_GPIO_PEFGH_DAT	0x28

#define RTL8366_SMI_ACK_RETRY_COUNT 5
#define RTL8367_SMI_READ_CTRL0      0x0b
#define RTL8367_SMI_READ_CTRL1      0x04
#define RTL8367_SMI_READ_OP         0x01

/* rtl865x/8196x SWCORE MDCIO, relative to SWCORE base. */
#define RTL865X_MDCIOCR             0x4004
#define RTL865X_MDCIOSR             0x4008
#define RTL865X_MDCIO_COMMAND_READ  0x00000000
#define RTL865X_MDCIO_PHYADD_SHIFT  24
#define RTL865X_MDCIO_REGADD_SHIFT  16
#define RTL865X_MDCIO_STATUS_BUSY   BIT(31)
#define RTL865X_MDCIO_STATUS_RERR   BIT(30)

static const struct rtl8197f_ethdiag_reg rtl8197f_sys_regs[] = {
	{ "SYS_ID?/REV?",       0x0000 },
	{ "SYS_STATUS?",        0x0004 },
	{ "HW_STRAP",           0x0008 },
	{ "CLK_MANAGE1",        0x0010 },
	{ "CLK_MANAGE2",        0x0014 },
	{ "SYS_PINMUX0",        0x0800 },
	{ "SYS_PINMUX1",        0x0804 },
	{ "SYS_PINMUX2",        0x0808 },
	{ "SYS_PINMUX6",        0x0818 },
	{ "SYS_PINMUX7",        0x081c },
	{ "SYS_PINMUX8",        0x0820 },
	{ "SYS_PINMUX9",        0x0824 },
	{ "SYS_PINMUX12",       0x0830 },
	{ "SYS_PINMUX13",       0x0834 },
	{ "SYS_PINMUX14",       0x0838 },
	{ "SYS_PINMUX15",       0x083c },
	{ "SYS_PINMUX16",       0x0840 },
	{ "SYS_PINMUX17",       0x0844 },
	{ "SYS_PINMUX18",       0x0848 },
};

static const struct rtl8197f_ethdiag_reg rtl8197f_gpio_regs[] = {
	{ "PABCD_CNR",          RTL8197F_GPIO_PABCD_CNR },
	{ "PABCD_DIR",          RTL8197F_GPIO_PABCD_DIR },
	{ "PABCD_DAT",          RTL8197F_GPIO_PABCD_DAT },
	{ "PEFGH_CNR",          RTL8197F_GPIO_PEFGH_CNR },
	{ "PEFGH_DIR",          RTL8197F_GPIO_PEFGH_DIR },
	{ "PEFGH_DAT",          RTL8197F_GPIO_PEFGH_DAT },
};

static const struct rtl8197f_ethdiag_reg rtl8197f_swcore_regs[] = {
	/* BSP rtl8196x/asicregs.h layout, relative to SWCORE_BASE. */
	{ "PHY0_CONTROL",       0x2000 },
	{ "PHY0_STATUS",        0x2004 },
	{ "PHY0_ID1",           0x2008 },
	{ "PHY0_ID2",           0x200c },
	{ "PHY4_CONTROL",       0x2080 },
	{ "PHY4_STATUS",        0x2084 },
	{ "PHY4_ID1",           0x2088 },
	{ "PHY4_ID2",           0x208c },
	{ "PHY6_CONTROL",       0x20c0 },
	{ "PHY6_STATUS",        0x20c4 },
	{ "PHY6_ID1",           0x20c8 },
	{ "PHY6_ID2",           0x20cc },
	{ "MACCR",              0x4000 },
	{ "MDCIOCR",            0x4004 },
	{ "MDCIOSR",            0x4008 },
	{ "PMCR",               0x400c },
	{ "SWCORECNR",          0x6000 },
	{ "MACMR",              0x6004 },
	{ "GDSR0",              0x6100 },
	{ "v28_CHIP_INFO",      0x00d8 },
	{ "v28_DMA_IF_CTRL",    0x9f58 },
	{ "v28_MAC_LINK_STS",   0xa188 },
};


static const struct rtl8197f_ethdiag_reg rtl865x_cpu_iface_regs[] = {
	/* SDK rtknet AsicDriver/rtl865xc_asicregs.h: CPU_IFACE_BASE. */
	{ "CPUICR",              0x0000 },
	{ "CPUIIMR",             0x0028 },
	{ "CPUIISR",             0x002c },
	{ "DMA_CR0",             0x003c },
	{ "DMA_CR1",             0x0040 },
	{ "DMA_CR2",             0x0044 },
	{ "DMA_CR3",             0x0068 },
	{ "CPUIMCR",             0x0080 },
	{ "CPUIMTTR0",           0x0084 },
	{ "CPUIMTTR2",           0x008c },
	{ "CPUIMPNTR0",          0x0094 },
	{ "CPUIMPNTR2",          0x009c },
	{ "DMA_CR4",             0x00a0 },
	{ "CPUICR1",             0x00a4 },
};

static const struct rtl8197f_ethdiag_reg rtl8367_smi_regs[] __maybe_unused = {
	{ "RTL8367_CHIP_NUMBER", 0x1300 },
	{ "RTL8367_CHIP_VER",    0x1301 },
	{ "RTL8367_CHIP_MODE",   0x1302 },
	{ "RTL8367_CHIP_DEBUG0", 0x1303 },
	{ "RTL8367_SWC0",        0x1200 },
	{ "RTL8367_VS_TPID",     0x1202 },
	{ "RTL8367_PORT0_STATUS",0x1352 },
	{ "RTL8367_PORT1_STATUS",0x1353 },
	{ "RTL8367_PORT2_STATUS",0x1354 },
	{ "RTL8367_PORT3_STATUS",0x1355 },
	{ "RTL8367_PORT4_STATUS",0x1356 },
	{ "RTL8367_PORT5_STATUS",0x1357 },
	{ "RTL8367_PORT6_STATUS",0x1358 },
	{ "RTL8367_PORT7_STATUS",0x1359 },
};

static u32 rtl8197f_gpio_cnr_reg(unsigned int gpio)
{
	return gpio < 32 ? RTL8197F_GPIO_PABCD_CNR : RTL8197F_GPIO_PEFGH_CNR;
}

static u32 rtl8197f_gpio_dir_reg(unsigned int gpio)
{
	return gpio < 32 ? RTL8197F_GPIO_PABCD_DIR : RTL8197F_GPIO_PEFGH_DIR;
}

static u32 rtl8197f_gpio_dat_reg(unsigned int gpio)
{
	return gpio < 32 ? RTL8197F_GPIO_PABCD_DAT : RTL8197F_GPIO_PEFGH_DAT;
}

static u32 rtl8197f_gpio_bit(unsigned int gpio)
{
	return BIT(gpio & 0x1f);
}

static void rtl8197f_gpio_mux_gpio(void __iomem *gpio_base, unsigned int gpio)
{
	u32 reg = rtl8197f_gpio_cnr_reg(gpio);
	u32 bit = rtl8197f_gpio_bit(gpio);

	/* Vendor BSP: CNR bit 0 selects GPIO, bit 1 selects peripheral. */
	writel(readl(gpio_base + reg) & ~bit, gpio_base + reg);
}

static void rtl8197f_gpio_dir_out(void __iomem *gpio_base, unsigned int gpio, int val)
{
	u32 dreg = rtl8197f_gpio_dir_reg(gpio);
	u32 vreg = rtl8197f_gpio_dat_reg(gpio);
	u32 bit = rtl8197f_gpio_bit(gpio);
	u32 v;

	rtl8197f_gpio_mux_gpio(gpio_base, gpio);

	v = readl(gpio_base + vreg);
	if (val)
		v |= bit;
	else
		v &= ~bit;
	writel(v, gpio_base + vreg);

	v = readl(gpio_base + dreg);
	v |= bit;
	writel(v, gpio_base + dreg);
}

static void rtl8197f_gpio_dir_in(void __iomem *gpio_base, unsigned int gpio)
{
	u32 dreg = rtl8197f_gpio_dir_reg(gpio);
	u32 bit = rtl8197f_gpio_bit(gpio);

	rtl8197f_gpio_mux_gpio(gpio_base, gpio);
	writel(readl(gpio_base + dreg) & ~bit, gpio_base + dreg);
}

static void rtl8197f_gpio_set(void __iomem *gpio_base, unsigned int gpio, int val)
{
	u32 reg = rtl8197f_gpio_dat_reg(gpio);
	u32 bit = rtl8197f_gpio_bit(gpio);
	u32 v = readl(gpio_base + reg);

	if (val)
		v |= bit;
	else
		v &= ~bit;
	writel(v, gpio_base + reg);
}

static int rtl8197f_gpio_get(void __iomem *gpio_base, unsigned int gpio)
{
	return !!(readl(gpio_base + rtl8197f_gpio_dat_reg(gpio)) &
		 rtl8197f_gpio_bit(gpio));
}

struct rtl8197f_gpio_snapshot {
	u32 pabcd_cnr;
	u32 pabcd_dir;
	u32 pabcd_dat;
	u32 pefgh_cnr;
	u32 pefgh_dir;
	u32 pefgh_dat;
};

static void rtl8197f_gpio_snapshot_take(void __iomem *gpio,
					       struct rtl8197f_gpio_snapshot *s)
{
	s->pabcd_cnr = readl(gpio + RTL8197F_GPIO_PABCD_CNR);
	s->pabcd_dir = readl(gpio + RTL8197F_GPIO_PABCD_DIR);
	s->pabcd_dat = readl(gpio + RTL8197F_GPIO_PABCD_DAT);
	s->pefgh_cnr = readl(gpio + RTL8197F_GPIO_PEFGH_CNR);
	s->pefgh_dir = readl(gpio + RTL8197F_GPIO_PEFGH_DIR);
	s->pefgh_dat = readl(gpio + RTL8197F_GPIO_PEFGH_DAT);
}

static void rtl8197f_gpio_snapshot_restore(void __iomem *gpio,
						  const struct rtl8197f_gpio_snapshot *s)
{
	writel(s->pabcd_dat, gpio + RTL8197F_GPIO_PABCD_DAT);
	writel(s->pefgh_dat, gpio + RTL8197F_GPIO_PEFGH_DAT);
	writel(s->pabcd_dir, gpio + RTL8197F_GPIO_PABCD_DIR);
	writel(s->pefgh_dir, gpio + RTL8197F_GPIO_PEFGH_DIR);
	writel(s->pabcd_cnr, gpio + RTL8197F_GPIO_PABCD_CNR);
	writel(s->pefgh_cnr, gpio + RTL8197F_GPIO_PEFGH_CNR);
}

static void rtl8197f_gpio_log_snapshot(struct device *dev, const char *tag,
					      const struct rtl8197f_gpio_snapshot *s)
{
	dev_info(dev,
		 "%s PABCD cnr=0x%08x dir=0x%08x dat=0x%08x; PEFGH cnr=0x%08x dir=0x%08x dat=0x%08x\n",
		 tag, s->pabcd_cnr, s->pabcd_dir, s->pabcd_dat,
		 s->pefgh_cnr, s->pefgh_dir, s->pefgh_dat);
}

static void rtl8197f_mdio_level_test(struct device *dev, void __iomem *gpio,
					    const struct rtl8197f_smi_gpio_bus *bus)
{
	struct rtl8197f_gpio_snapshot before, after;
	int mdio_initial, mdio_drive0, mdio_drive1, mdio_release;
	int mdio_clk_low, mdio_clk_high, mdc_drive0, mdc_drive1;

	rtl8197f_gpio_snapshot_take(gpio, &before);

	rtl8197f_gpio_dir_in(gpio, bus->mdio_gpio);
	rtl8197f_gpio_dir_in(gpio, bus->mdc_gpio);
	udelay(20);
	mdio_initial = rtl8197f_gpio_get(gpio, bus->mdio_gpio);

	rtl8197f_gpio_dir_out(gpio, bus->mdc_gpio, 0);
	udelay(10);
	mdc_drive0 = rtl8197f_gpio_get(gpio, bus->mdc_gpio);
	rtl8197f_gpio_dir_out(gpio, bus->mdc_gpio, 1);
	udelay(10);
	mdc_drive1 = rtl8197f_gpio_get(gpio, bus->mdc_gpio);

	rtl8197f_gpio_dir_out(gpio, bus->mdio_gpio, 0);
	udelay(20);
	mdio_drive0 = rtl8197f_gpio_get(gpio, bus->mdio_gpio);
	rtl8197f_gpio_dir_out(gpio, bus->mdio_gpio, 1);
	udelay(20);
	mdio_drive1 = rtl8197f_gpio_get(gpio, bus->mdio_gpio);
	rtl8197f_gpio_dir_in(gpio, bus->mdio_gpio);
	udelay(50);
	mdio_release = rtl8197f_gpio_get(gpio, bus->mdio_gpio);

	rtl8197f_gpio_dir_out(gpio, bus->mdc_gpio, 0);
	udelay(20);
	mdio_clk_low = rtl8197f_gpio_get(gpio, bus->mdio_gpio);
	rtl8197f_gpio_dir_out(gpio, bus->mdc_gpio, 1);
	udelay(20);
	mdio_clk_high = rtl8197f_gpio_get(gpio, bus->mdio_gpio);
	rtl8197f_gpio_dir_in(gpio, bus->mdc_gpio);

	rtl8197f_gpio_snapshot_take(gpio, &after);

	dev_info(dev,
		 "MDIO level test %s: MDC gpio%u drive0=%d drive1=%d; MDIO gpio%u initial=%d drive0=%d drive1=%d release/input=%d sample@clk0=%d sample@clk1=%d%s\n",
		 bus->name, bus->mdc_gpio, mdc_drive0, mdc_drive1,
		 bus->mdio_gpio, mdio_initial, mdio_drive0, mdio_drive1,
		 mdio_release, mdio_clk_low, mdio_clk_high,
		 mdio_release ? "" : "  <-- MDIO stays low after release");
	rtl8197f_gpio_log_snapshot(dev, "MDIO level after", &after);

	rtl8197f_gpio_snapshot_restore(gpio, &before);
}

static void rtl8197f_smi_delay(void)
{
	ndelay(1000);
}

static void rtl8197f_smi_start(void __iomem *gpio, const struct rtl8197f_smi_gpio_bus *bus)
{
	rtl8197f_gpio_dir_out(gpio, bus->mdc_gpio, 0);
	rtl8197f_gpio_dir_out(gpio, bus->mdio_gpio, 1);
	rtl8197f_smi_delay();

	rtl8197f_gpio_set(gpio, bus->mdc_gpio, 1);
	rtl8197f_smi_delay();
	rtl8197f_gpio_set(gpio, bus->mdc_gpio, 0);
	rtl8197f_smi_delay();

	rtl8197f_gpio_set(gpio, bus->mdc_gpio, 1);
	rtl8197f_smi_delay();
	rtl8197f_gpio_set(gpio, bus->mdio_gpio, 0);
	rtl8197f_smi_delay();
	rtl8197f_gpio_set(gpio, bus->mdc_gpio, 0);
	rtl8197f_smi_delay();
	rtl8197f_gpio_set(gpio, bus->mdio_gpio, 1);
}

static void rtl8197f_smi_stop(void __iomem *gpio, const struct rtl8197f_smi_gpio_bus *bus)
{
	rtl8197f_smi_delay();
	rtl8197f_gpio_set(gpio, bus->mdio_gpio, 0);
	rtl8197f_gpio_set(gpio, bus->mdc_gpio, 1);
	rtl8197f_smi_delay();
	rtl8197f_gpio_set(gpio, bus->mdio_gpio, 1);
	rtl8197f_smi_delay();
	rtl8197f_gpio_set(gpio, bus->mdc_gpio, 1);
	rtl8197f_smi_delay();
	rtl8197f_gpio_set(gpio, bus->mdc_gpio, 0);
	rtl8197f_smi_delay();
	rtl8197f_gpio_set(gpio, bus->mdc_gpio, 1);

	rtl8197f_smi_delay();
	rtl8197f_gpio_set(gpio, bus->mdc_gpio, 0);
	rtl8197f_smi_delay();
	rtl8197f_gpio_set(gpio, bus->mdc_gpio, 1);

	rtl8197f_gpio_dir_in(gpio, bus->mdio_gpio);
	rtl8197f_gpio_dir_in(gpio, bus->mdc_gpio);
}

static void rtl8197f_smi_write_bits(void __iomem *gpio,
				    const struct rtl8197f_smi_gpio_bus *bus,
				    u32 data, u32 len)
{
	for (; len > 0; len--) {
		rtl8197f_smi_delay();
		rtl8197f_gpio_set(gpio, bus->mdio_gpio, !!(data & BIT(len - 1)));
		rtl8197f_smi_delay();
		rtl8197f_gpio_set(gpio, bus->mdc_gpio, 1);
		rtl8197f_smi_delay();
		rtl8197f_gpio_set(gpio, bus->mdc_gpio, 0);
	}
}

static void rtl8197f_smi_read_bits(void __iomem *gpio,
				   const struct rtl8197f_smi_gpio_bus *bus,
				   u32 len, u32 *data)
{
	*data = 0;
	rtl8197f_gpio_dir_in(gpio, bus->mdio_gpio);

	for (; len > 0; len--) {
		rtl8197f_smi_delay();
		rtl8197f_gpio_set(gpio, bus->mdc_gpio, 1);
		rtl8197f_smi_delay();
		*data |= rtl8197f_gpio_get(gpio, bus->mdio_gpio) << (len - 1);
		rtl8197f_gpio_set(gpio, bus->mdc_gpio, 0);
	}

	rtl8197f_gpio_dir_out(gpio, bus->mdio_gpio, 0);
}

static int rtl8197f_smi_wait_ack(void __iomem *gpio,
				 const struct rtl8197f_smi_gpio_bus *bus,
				 const char *phase, u32 *last_ack)
{
	unsigned int i;
	u32 ack = 1;

	for (i = 0; i < RTL8366_SMI_ACK_RETRY_COUNT; i++) {
		rtl8197f_smi_read_bits(gpio, bus, 1, &ack);
		if (ack == 0) {
			if (last_ack)
				*last_ack = ack;
			return 0;
		}
	}

	if (last_ack)
		*last_ack = ack;
	return -ETIMEDOUT;
}

static int rtl8197f_smi_read_reg(void __iomem *gpio,
				 const struct rtl8197f_smi_gpio_bus *bus,
				 u32 reg, u32 *val, u32 *ack_phase)
{
	u32 raw = 0, lo = 0, hi = 0, ack = 0;
	int ret = 0;

	*val = 0;
	if (ack_phase)
		*ack_phase = 0;

	rtl8197f_smi_start(gpio, bus);

	/* Realtek vendor SMI read: 4'b1011, 3'b100, 1'b1, addr lo, addr hi. */
	rtl8197f_smi_write_bits(gpio, bus, RTL8367_SMI_READ_CTRL0, 4);
	rtl8197f_smi_write_bits(gpio, bus, RTL8367_SMI_READ_CTRL1, 3);
	rtl8197f_smi_write_bits(gpio, bus, RTL8367_SMI_READ_OP, 1);
	ret = rtl8197f_smi_wait_ack(gpio, bus, "cmd", &ack);
	if (ret) {
		if (ack_phase)
			*ack_phase = 1;
		goto out;
	}

	rtl8197f_smi_write_bits(gpio, bus, reg & 0xff, 8);
	ret = rtl8197f_smi_wait_ack(gpio, bus, "addr-lo", &ack);
	if (ret) {
		if (ack_phase)
			*ack_phase = 2;
		goto out;
	}

	rtl8197f_smi_write_bits(gpio, bus, reg >> 8, 8);
	ret = rtl8197f_smi_wait_ack(gpio, bus, "addr-hi", &ack);
	if (ret) {
		if (ack_phase)
			*ack_phase = 3;
		goto out;
	}

	rtl8197f_smi_read_bits(gpio, bus, 8, &raw);
	lo = raw & 0xff;
	rtl8197f_smi_write_bits(gpio, bus, 0x00, 1); /* CPU ACK */

	rtl8197f_smi_read_bits(gpio, bus, 8, &raw);
	hi = raw & 0xff;
	rtl8197f_smi_write_bits(gpio, bus, 0x01, 1); /* CPU NACK */

	*val = lo | (hi << 8);

out:
	rtl8197f_smi_stop(gpio, bus);
	return ret;
}

static void rtl8197f_dump_window(struct device *dev,
					const struct rtl8197f_ethdiag_window *win)
{
	void __iomem *base;
	unsigned int i;

	base = devm_ioremap(dev, win->phys, win->size);
	if (!base) {
		dev_info(dev, "%s phys=%pa size=0x%08x: ioremap failed\n",
			 win->name, &win->phys, win->size);
		return;
	}

	dev_info(dev, "%s phys=%pa size=0x%08x read-only\n",
		 win->name, &win->phys, win->size);

	for (i = 0; i < win->nregs; i++) {
		const struct rtl8197f_ethdiag_reg *r = &win->regs[i];

		if (r->off + 4 > win->size) {
			dev_info(dev, "  %-20s @0x%04x = <outside-window>\n",
				 r->name, r->off);
			continue;
		}

		dev_info(dev, "  %-20s @0x%04x = 0x%08x\n",
			 r->name, r->off, readl(base + r->off));
	}
}

static void __maybe_unused rtl8197f_dump_smi_bus(struct device *dev, void __iomem *gpio,
					 const struct rtl8197f_smi_gpio_bus *bus)
{
	struct rtl8197f_gpio_snapshot before, after;
	unsigned int i;

	rtl8197f_gpio_snapshot_take(gpio, &before);
	dev_info(dev,
		 "RTL8367 GPIO-SMI probe on %s: MDC gpio%u, MDIO gpio%u\n",
		 bus->name, bus->mdc_gpio, bus->mdio_gpio);
	rtl8197f_gpio_log_snapshot(dev, "GPIO before SMI", &before);

	for (i = 0; i < ARRAY_SIZE(rtl8367_smi_regs); i++) {
		const struct rtl8197f_ethdiag_reg *r = &rtl8367_smi_regs[i];
		u32 val = 0, phase = 0;
		int ret;

		ret = rtl8197f_smi_read_reg(gpio, bus, r->off, &val, &phase);
		if (ret)
			dev_info(dev,
				 "  %-20s @0x%04x = <SMI timeout ret=%d phase=%u>\n",
				 r->name, r->off, ret, phase);
		else
			dev_info(dev, "  %-20s @0x%04x = 0x%04x\n",
				 r->name, r->off, val & 0xffff);
	}

	rtl8197f_gpio_snapshot_take(gpio, &after);
	rtl8197f_gpio_log_snapshot(dev, "GPIO after SMI", &after);
	dev_info(dev, "RTL8367 GPIO-SMI probe on %s done\n", bus->name);
}

static void __maybe_unused rtl8197f_dump_smi_chipid(struct device *dev, void __iomem *gpio,
					    const char *tag,
					    const struct rtl8197f_smi_gpio_bus *bus)
{
	u32 chip = 0, ver = 0, phase = 0;
	int ret_chip, ret_ver;

	ret_chip = rtl8197f_smi_read_reg(gpio, bus, 0x1300, &chip, &phase);
	if (ret_chip) {
		dev_info(dev,
			 "Reset-sweep %s/%s: CHIP_NUMBER timeout ret=%d phase=%u\n",
			 tag, bus->name, ret_chip, phase);
		return;
	}

	ret_ver = rtl8197f_smi_read_reg(gpio, bus, 0x1301, &ver, &phase);
	if (ret_ver) {
		dev_info(dev,
			 "Reset-sweep %s/%s: CHIP_NUMBER=0x%04x CHIP_VER timeout ret=%d phase=%u\n",
			 tag, bus->name, chip & 0xffff, ret_ver, phase);
		return;
	}

	dev_info(dev,
		 "Reset-sweep %s/%s: CHIP_NUMBER=0x%04x CHIP_VER=0x%04x%s\n",
		 tag, bus->name, chip & 0xffff, ver & 0xffff,
		 (!chip && !ver) ? "  <-- all-zero read, not a valid RTL8367 ID" : "");
}

static void __maybe_unused rtl8197f_reset_sweep(struct device *dev, void __iomem *gpio,
					const struct rtl8197f_smi_gpio_bus *buses,
					unsigned int nbuses)
{
	static const struct rtl8197f_reset_candidate resets[] = {
		{ .name = "oem-reset-H2", .gpio = 58, .active_low = true },
	};
	static const unsigned int test_bus_idx[] = { 0, 1 };
	struct rtl8197f_gpio_snapshot before, after;
	unsigned int i, j;

	dev_info(dev,
		 "RTL8367D OEM reset test: GPIO58/H2 active-low only; assert 100ms, deassert 1500ms, then read GPIO56/55 and GPIO55/56\n");

	for (i = 0; i < ARRAY_SIZE(resets); i++) {
		const struct rtl8197f_reset_candidate *rst = &resets[i];
		int assert_level = rst->active_low ? 0 : 1;
		int deassert_level = rst->active_low ? 1 : 0;

		rtl8197f_gpio_snapshot_take(gpio, &before);
		rtl8197f_gpio_log_snapshot(dev, "OEM reset before", &before);
		dev_info(dev,
			 "OEM reset %s gpio%u active-%s: assert level=%d for 100ms\n",
			 rst->name, rst->gpio, rst->active_low ? "low" : "high",
			 assert_level);
		rtl8197f_gpio_dir_out(gpio, rst->gpio, assert_level);
		msleep(100);

		dev_info(dev,
			 "OEM reset %s gpio%u: deassert level=%d and wait 1500ms\n",
			 rst->name, rst->gpio, deassert_level);
		rtl8197f_gpio_dir_out(gpio, rst->gpio, deassert_level);
		msleep(1500);

		for (j = 0; j < ARRAY_SIZE(test_bus_idx); j++) {
			unsigned int bi = test_bus_idx[j];

			if (bi < nbuses)
				rtl8197f_dump_smi_chipid(dev, gpio, rst->name,
						       &buses[bi]);
		}

		rtl8197f_gpio_snapshot_take(gpio, &after);
		rtl8197f_gpio_log_snapshot(dev, "OEM reset after", &after);
		rtl8197f_gpio_snapshot_restore(gpio, &before);
	}
}

static void rtl8197f_internal_mdcio_test(struct device *dev)
{
	void __iomem *swcore;
	unsigned int phy, reg, loops;

	swcore = devm_ioremap(dev, 0x1b800000, 0x10000);
	if (!swcore) {
		dev_info(dev, "internal MDCIO test skipped: SWCORE ioremap failed\n");
		return;
	}

	dev_info(dev,
		 "internal MDCIO Clause-22 test: SWCORE=0x1b800000 MDCIOCR=0x4004 MDCIOSR=0x4008, probing PHY0..7 regs 2/3 with bounded busy wait\n");

	for (phy = 0; phy < 8; phy++) {
		for (reg = 2; reg <= 3; reg++) {
			u32 cmd, status = 0;

			cmd = RTL865X_MDCIO_COMMAND_READ |
			      (phy << RTL865X_MDCIO_PHYADD_SHIFT) |
			      (reg << RTL865X_MDCIO_REGADD_SHIFT);
			writel(cmd, swcore + RTL865X_MDCIOCR);

			for (loops = 0; loops < 10000; loops++) {
				status = readl(swcore + RTL865X_MDCIOSR);
				if (!(status & RTL865X_MDCIO_STATUS_BUSY))
					break;
				ndelay(100);
			}

			if (loops == 10000)
				dev_info(dev,
					 "internal MDCIO phy%u reg%u: busy timeout status=0x%08x\n",
					 phy, reg, status);
			else
				dev_info(dev,
					 "internal MDCIO phy%u reg%u: status=0x%08x data=0x%04x%s loops=%u\n",
					 phy, reg, status, status & 0xffff,
					 (status & RTL865X_MDCIO_STATUS_RERR) ? " read-error" : "",
					 loops);
		}
	}
}

static int rtl8197f_ethdiag_probe(struct platform_device *pdev)
{
	static const struct rtl8197f_ethdiag_window windows[] = {
		{
			.name = "RTL8197F system regs",
			.phys = 0x18000000,
			.size = 0x00010000,
			.regs = rtl8197f_sys_regs,
			.nregs = ARRAYSZ(rtl8197f_sys_regs),
		},
		{
			.name = "RTL8197F GPIO regs",
			.phys = 0x18003500,
			.size = 0x00000038,
			.regs = rtl8197f_gpio_regs,
			.nregs = ARRAYSZ(rtl8197f_gpio_regs),
		},
		{
			.name = "RTL8197F rtl865x CPU interface",
			.phys = 0x18010000,
			.size = 0x00001000,
			.regs = rtl865x_cpu_iface_regs,
			.nregs = ARRAYSZ(rtl865x_cpu_iface_regs),
		},
		{
			.name = "RTL8197F SWCORE 8196x/rtknet layout",
			.phys = 0x1b800000,
			.size = 0x00010000,
			.regs = rtl8197f_swcore_regs,
			.nregs = ARRAYSZ(rtl8197f_swcore_regs),
		},
	};
	static const struct rtl8197f_smi_gpio_bus smi_buses[] = {
		/* OEM RD05 platform devices rtl819x_8367r_i2c_pin.{2,1}. */
		{ .name = "oem-H0-G7-gpio56-55", .mdc_gpio = 56, .mdio_gpio = 55 },
		{ .name = "oem-swap-G7-H0-gpio55-56", .mdc_gpio = 55, .mdio_gpio = 56 },
	};
	void __iomem *gpio;
	unsigned int i;

	dev_info(&pdev->dev,
		 "RTL8197F Ethernet/MDIO diag v38: DSA coexistence after OEM preinit; read-only GPIO56/55/GPIO58 + rtl865x MDCIO\n");
	dev_info(&pdev->dev,
		 "OEM reference: switchChip=8367D, WAN port2 VID8, LAN ports0/1/3/4 VID9, CPU extension bit 0x20000\n");

	for (i = 0; i < ARRAY_SIZE(windows); i++)
		rtl8197f_dump_window(&pdev->dev, &windows[i]);

	gpio = devm_ioremap(&pdev->dev, 0x18003500, 0x38);
	if (!gpio) {
		dev_info(&pdev->dev, "RTL8367 GPIO-SMI probe skipped: GPIO ioremap failed\n");
		return 0;
	}

	dev_info(&pdev->dev,
		 "MDIO level diagnostics: OEM pins only; DSA coexistence after OEM preinit, no ethdiag reset\n");
	for (i = 0; i < ARRAY_SIZE(smi_buses); i++)
		rtl8197f_mdio_level_test(&pdev->dev, gpio, &smi_buses[i]);

	dev_info(&pdev->dev,
		 "GPIO-SMI read skipped in v38: native realtek-smi/DSA owns GPIO56/55\n");

	dev_info(&pdev->dev, "internal MDCIO read-only snapshot with DSA enabled\n");
	rtl8197f_internal_mdcio_test(&pdev->dev);

	dev_info(&pdev->dev,
		 "GPIO58 reset assert skipped in v38: OEM preinit deasserts high; DSA has no reset-gpios\n");

	dev_info(&pdev->dev,
		 "RTL8197F Ethernet/MDIO diag v38 complete; DSA owns GPIO-SMI; no ethdiag GPIO58 assert\n");

	return 0;
}

static const struct of_device_id rtl8197f_ethdiag_of_match[] = {
	{ .compatible = "realtek,rtl8197f-ethdiag" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl8197f_ethdiag_of_match);

static struct platform_driver rtl8197f_ethdiag_driver = {
	.probe = rtl8197f_ethdiag_probe,
	.driver = {
		.name = "rtl8197f-ethdiag",
		.of_match_table = rtl8197f_ethdiag_of_match,
	},
};
module_platform_driver(rtl8197f_ethdiag_driver);

MODULE_DESCRIPTION("RTL8197F rtl865x/rtknet Ethernet/MDIO/GPIO-SMI diagnostic driver");
MODULE_AUTHOR("OpenAI");
MODULE_LICENSE("GPL");
