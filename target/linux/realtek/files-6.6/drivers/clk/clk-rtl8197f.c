// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * RTL8197F CPU clock: port of the Cudy/Realtek GPL BSP timer.c strap
 * decoding to the common clock framework. Read-only: no PLL changes.
 * Use the complete timer decoder, not clk.c's simplified 600/800/1000 MHz
 * lookup, which ignores dividers and the 200 MHz startup mode.
 */

#include <linux/clk-provider.h>
#include <linux/bits.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/of_address.h>

static unsigned long rtl8197f_cpu_rate(u32 strap)
{
	u32 selector = (strap >> 15) & 0xf;
	u32 divisor = 1U << ((strap >> 19) & 0x3);

	if (strap & BIT(14))
		return 200000000;

	/* BSP table: 450, 500, ... 1200 MHz; divide by 1, 2, 4 or 8. */
	return (450000000UL + selector * 50000000UL) / divisor;
}

static void __init rtl8197f_cpu_clk_init(struct device_node *np)
{
	void __iomem *base;
	struct clk_hw *hw;
	unsigned long rate;
	u32 strap;
	int ret;

	base = of_iomap(np, 0);
	if (!base) {
		pr_err("%pOF: cannot map RTL8197F strap register\n", np);
		return;
	}
	strap = readl(base);
	iounmap(base);
	rate = rtl8197f_cpu_rate(strap);

	hw = clk_hw_register_fixed_rate(NULL, np->name, NULL, 0, rate);
	if (IS_ERR(hw)) {
		pr_err("%pOF: cannot register CPU clock: %ld\n", np, PTR_ERR(hw));
		return;
	}
	ret = of_clk_add_hw_provider(np, of_clk_hw_simple_get, hw);
	if (ret) {
		clk_hw_unregister_fixed_rate(hw);
		pr_err("%pOF: cannot add CPU clock provider: %d\n", np, ret);
		return;
	}
	pr_info("RTL8197F CPU strap=0x%08x rate=%lu Hz (read-only)\n",
		strap, rate);
}
CLK_OF_DECLARE(rtl8197f_cpu_clk, "realtek,rtl8197f-cpu-clock", rtl8197f_cpu_clk_init);
