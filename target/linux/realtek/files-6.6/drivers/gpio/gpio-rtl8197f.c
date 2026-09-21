// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * GPIO controller support for Realtek RTL8197F/RTL8197F-VG.
 *
 * Register layout and interrupt encoding reconstructed from Realtek's GPL
 * RTL8197F BSP:
 *   PABCD_CNR 0x00, PABCD_DIR 0x08, PABCD_DAT 0x0c, PABCD_ISR 0x10
 *   PAB_IMR   0x14, PCD_IMR   0x18
 *   PEFGH_CNR 0x1c, PEFGH_DIR 0x24, PEFGH_DAT 0x28, PEFGH_ISR 0x2c
 *   PEF_IMR   0x30, PGH_IMR   0x34
 *
 * Pins are numbered A0..H7 as 0..63.  GPIO interrupts are edge-only in the
 * vendor code: 1 = falling, 2 = rising, 3 = both.
 */

#include <linux/bitops.h>
#include <linux/gpio/driver.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/irqdesc.h>
#include <linux/irqdomain.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define RTL8197F_GPIO_NGPIO          64

#define RTL8197F_GPIO_PABCD_CNR      0x00
#define RTL8197F_GPIO_PABCD_DIR      0x08
#define RTL8197F_GPIO_PABCD_DAT      0x0c
#define RTL8197F_GPIO_PABCD_ISR      0x10
#define RTL8197F_GPIO_PAB_IMR        0x14
#define RTL8197F_GPIO_PCD_IMR        0x18

#define RTL8197F_GPIO_PEFGH_CNR      0x1c
#define RTL8197F_GPIO_PEFGH_DIR      0x24
#define RTL8197F_GPIO_PEFGH_DAT      0x28
#define RTL8197F_GPIO_PEFGH_ISR      0x2c
#define RTL8197F_GPIO_PEF_IMR        0x30
#define RTL8197F_GPIO_PGH_IMR        0x34

struct rtl8197f_gpio {
	struct gpio_chip gc;
	void __iomem *base;
	spinlock_t lock;
	u8 irq_type[RTL8197F_GPIO_NGPIO];
	unsigned int parent_irqs[2];
};

static inline struct rtl8197f_gpio *to_rtl8197f_gpio(struct gpio_chip *gc)
{
	return gpiochip_get_data(gc);
}

static inline unsigned int rtl8197f_gpio_bit(unsigned int offset)
{
	return offset & 0x1f;
}

static inline unsigned int rtl8197f_gpio_2bit(unsigned int offset)
{
	return (offset & 0x0f) * 2;
}

static inline u32 rtl8197f_gpio_reg_cnr(unsigned int offset)
{
	return offset < 32 ? RTL8197F_GPIO_PABCD_CNR : RTL8197F_GPIO_PEFGH_CNR;
}

static inline u32 rtl8197f_gpio_reg_dir(unsigned int offset)
{
	return offset < 32 ? RTL8197F_GPIO_PABCD_DIR : RTL8197F_GPIO_PEFGH_DIR;
}

static inline u32 rtl8197f_gpio_reg_dat(unsigned int offset)
{
	return offset < 32 ? RTL8197F_GPIO_PABCD_DAT : RTL8197F_GPIO_PEFGH_DAT;
}

static inline u32 rtl8197f_gpio_reg_isr(unsigned int offset)
{
	return offset < 32 ? RTL8197F_GPIO_PABCD_ISR : RTL8197F_GPIO_PEFGH_ISR;
}

static inline u32 rtl8197f_gpio_reg_imr(unsigned int offset)
{
	if (offset < 16)
		return RTL8197F_GPIO_PAB_IMR;
	if (offset < 32)
		return RTL8197F_GPIO_PCD_IMR;
	if (offset < 48)
		return RTL8197F_GPIO_PEF_IMR;
	return RTL8197F_GPIO_PGH_IMR;
}

static void rtl8197f_gpio_set_mux_gpio(struct rtl8197f_gpio *rg, unsigned int offset)
{
	u32 reg = rtl8197f_gpio_reg_cnr(offset);
	u32 bit = BIT(rtl8197f_gpio_bit(offset));
	u32 val;

	/* Vendor BSP: CNR bit 0 selects GPIO, bit 1 selects peripheral. */
	val = readl(rg->base + reg);
	val &= ~bit;
	writel(val, rg->base + reg);
}

static int rtl8197f_gpio_request(struct gpio_chip *gc, unsigned int offset)
{
	struct rtl8197f_gpio *rg = to_rtl8197f_gpio(gc);
	unsigned long flags;

	if (offset >= gc->ngpio)
		return -EINVAL;

	spin_lock_irqsave(&rg->lock, flags);
	rtl8197f_gpio_set_mux_gpio(rg, offset);
	spin_unlock_irqrestore(&rg->lock, flags);

	return 0;
}

static int rtl8197f_gpio_get(struct gpio_chip *gc, unsigned int offset)
{
	struct rtl8197f_gpio *rg = to_rtl8197f_gpio(gc);

	return !!(readl(rg->base + rtl8197f_gpio_reg_dat(offset)) &
		BIT(rtl8197f_gpio_bit(offset)));
}

static void rtl8197f_gpio_set(struct gpio_chip *gc, unsigned int offset, int value)
{
	struct rtl8197f_gpio *rg = to_rtl8197f_gpio(gc);
	unsigned long flags;
	u32 reg = rtl8197f_gpio_reg_dat(offset);
	u32 bit = BIT(rtl8197f_gpio_bit(offset));
	u32 val;

	spin_lock_irqsave(&rg->lock, flags);
	val = readl(rg->base + reg);
	if (value)
		val |= bit;
	else
		val &= ~bit;
	writel(val, rg->base + reg);
	spin_unlock_irqrestore(&rg->lock, flags);
}

static int rtl8197f_gpio_direction_input(struct gpio_chip *gc, unsigned int offset)
{
	struct rtl8197f_gpio *rg = to_rtl8197f_gpio(gc);
	unsigned long flags;
	u32 reg = rtl8197f_gpio_reg_dir(offset);
	u32 bit = BIT(rtl8197f_gpio_bit(offset));
	u32 val;

	spin_lock_irqsave(&rg->lock, flags);
	rtl8197f_gpio_set_mux_gpio(rg, offset);
	val = readl(rg->base + reg);
	val &= ~bit;
	writel(val, rg->base + reg);
	spin_unlock_irqrestore(&rg->lock, flags);

	return 0;
}

static int rtl8197f_gpio_direction_output(struct gpio_chip *gc,
					  unsigned int offset, int value)
{
	struct rtl8197f_gpio *rg = to_rtl8197f_gpio(gc);
	unsigned long flags;
	u32 dir_reg = rtl8197f_gpio_reg_dir(offset);
	u32 dat_reg = rtl8197f_gpio_reg_dat(offset);
	u32 bit = BIT(rtl8197f_gpio_bit(offset));
	u32 val;

	spin_lock_irqsave(&rg->lock, flags);
	rtl8197f_gpio_set_mux_gpio(rg, offset);

	val = readl(rg->base + dat_reg);
	if (value)
		val |= bit;
	else
		val &= ~bit;
	writel(val, rg->base + dat_reg);

	val = readl(rg->base + dir_reg);
	val |= bit;
	writel(val, rg->base + dir_reg);
	spin_unlock_irqrestore(&rg->lock, flags);

	return 0;
}

static int rtl8197f_gpio_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	struct rtl8197f_gpio *rg = to_rtl8197f_gpio(gc);
	u32 val = readl(rg->base + rtl8197f_gpio_reg_dir(offset));

	return (val & BIT(rtl8197f_gpio_bit(offset))) ?
		GPIO_LINE_DIRECTION_OUT : GPIO_LINE_DIRECTION_IN;
}

static void rtl8197f_gpio_irq_ack(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct rtl8197f_gpio *rg = to_rtl8197f_gpio(gc);
	unsigned int offset = irqd_to_hwirq(d);

	writel(BIT(rtl8197f_gpio_bit(offset)),
	       rg->base + rtl8197f_gpio_reg_isr(offset));
}

static void rtl8197f_gpio_irq_mask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct rtl8197f_gpio *rg = to_rtl8197f_gpio(gc);
	unsigned int offset = irqd_to_hwirq(d);
	unsigned long flags;
	u32 reg = rtl8197f_gpio_reg_imr(offset);
	u32 shift = rtl8197f_gpio_2bit(offset);
	u32 val;

	spin_lock_irqsave(&rg->lock, flags);
	val = readl(rg->base + reg);
	val &= ~(0x3 << shift);
	writel(val, rg->base + reg);
	spin_unlock_irqrestore(&rg->lock, flags);
}

static void rtl8197f_gpio_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct rtl8197f_gpio *rg = to_rtl8197f_gpio(gc);
	unsigned int offset = irqd_to_hwirq(d);
	unsigned long flags;
	u32 reg = rtl8197f_gpio_reg_imr(offset);
	u32 shift = rtl8197f_gpio_2bit(offset);
	u32 type = rg->irq_type[offset] ?: 0x1; /* default to falling edge */
	u32 val;

	spin_lock_irqsave(&rg->lock, flags);
	val = readl(rg->base + reg);
	val &= ~(0x3 << shift);
	val |= type << shift;
	writel(val, rg->base + reg);
	spin_unlock_irqrestore(&rg->lock, flags);
}

static int rtl8197f_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct rtl8197f_gpio *rg = to_rtl8197f_gpio(gc);
	unsigned int offset = irqd_to_hwirq(d);
	u8 hw_type;

	switch (type & IRQ_TYPE_SENSE_MASK) {
	case IRQ_TYPE_EDGE_FALLING:
		hw_type = 0x1;
		break;
	case IRQ_TYPE_EDGE_RISING:
		hw_type = 0x2;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		hw_type = 0x3;
		break;
	default:
		return -EINVAL;
	}

	rg->irq_type[offset] = hw_type;
	irq_set_handler_locked(d, handle_edge_irq);

	return 0;
}

static const struct irq_chip rtl8197f_gpio_irq_chip = {
	.name = "rtl8197f-gpio",
	.irq_ack = rtl8197f_gpio_irq_ack,
	.irq_mask = rtl8197f_gpio_irq_mask,
	.irq_unmask = rtl8197f_gpio_irq_unmask,
	.irq_set_type = rtl8197f_gpio_irq_set_type,
	.flags = IRQCHIP_IMMUTABLE,
	GPIOCHIP_IRQ_RESOURCE_HELPERS,
};

static void rtl8197f_gpio_irq_handler(struct irq_desc *desc)
{
	struct gpio_chip *gc = irq_desc_get_handler_data(desc);
	struct rtl8197f_gpio *rg = to_rtl8197f_gpio(gc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	unsigned int irq = irq_desc_get_irq(desc);
	unsigned int base = (irq == rg->parent_irqs[0]) ? 0 : 32;
	u32 status, mask_lo, mask_hi, enabled = 0;
	int bit;

	chained_irq_enter(chip, desc);

	if (!base) {
		status = readl(rg->base + RTL8197F_GPIO_PABCD_ISR);
		mask_lo = readl(rg->base + RTL8197F_GPIO_PAB_IMR);
		mask_hi = readl(rg->base + RTL8197F_GPIO_PCD_IMR);
	} else {
		status = readl(rg->base + RTL8197F_GPIO_PEFGH_ISR);
		mask_lo = readl(rg->base + RTL8197F_GPIO_PEF_IMR);
		mask_hi = readl(rg->base + RTL8197F_GPIO_PGH_IMR);
	}

	for (bit = 0; bit < 16; bit++) {
		if (mask_lo & (0x3 << (bit * 2)))
			enabled |= BIT(bit);
		if (mask_hi & (0x3 << (bit * 2)))
			enabled |= BIT(bit + 16);
	}

	status &= enabled;
	for (bit = 0; bit < 32; bit++) {
		if (status & BIT(bit))
			generic_handle_domain_irq(gc->irq.domain, base + bit);
	}

	chained_irq_exit(chip, desc);
}

static int rtl8197f_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rtl8197f_gpio *rg;
	struct gpio_irq_chip *girq;
	u32 ngpios;
	int irq0, irq1;

	rg = devm_kzalloc(dev, sizeof(*rg), GFP_KERNEL);
	if (!rg)
		return -ENOMEM;

	rg->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(rg->base))
		return PTR_ERR(rg->base);

	spin_lock_init(&rg->lock);

	ngpios = RTL8197F_GPIO_NGPIO;
	of_property_read_u32(dev->of_node, "ngpios", &ngpios);
	if (ngpios > RTL8197F_GPIO_NGPIO)
		ngpios = RTL8197F_GPIO_NGPIO;

	rg->gc.label = dev_name(dev);
	rg->gc.parent = dev;
	rg->gc.owner = THIS_MODULE;
	rg->gc.request = rtl8197f_gpio_request;
	rg->gc.get = rtl8197f_gpio_get;
	rg->gc.set = rtl8197f_gpio_set;
	rg->gc.direction_input = rtl8197f_gpio_direction_input;
	rg->gc.direction_output = rtl8197f_gpio_direction_output;
	rg->gc.get_direction = rtl8197f_gpio_get_direction;
	rg->gc.base = -1;
	rg->gc.ngpio = ngpios;

	/* Start with all GPIO IRQs masked and clear any stale edge latches. */
	writel(0, rg->base + RTL8197F_GPIO_PAB_IMR);
	writel(0, rg->base + RTL8197F_GPIO_PCD_IMR);
	writel(0, rg->base + RTL8197F_GPIO_PEF_IMR);
	writel(0, rg->base + RTL8197F_GPIO_PGH_IMR);
	writel(0xffffffff, rg->base + RTL8197F_GPIO_PABCD_ISR);
	writel(0xffffffff, rg->base + RTL8197F_GPIO_PEFGH_ISR);

	irq0 = platform_get_irq_optional(pdev, 0);
	irq1 = platform_get_irq_optional(pdev, 1);
	if (irq0 > 0 && irq1 > 0) {
		rg->parent_irqs[0] = irq0;
		rg->parent_irqs[1] = irq1;

		girq = &rg->gc.irq;
		gpio_irq_chip_set_chip(girq, &rtl8197f_gpio_irq_chip);
		girq->parent_handler = rtl8197f_gpio_irq_handler;
		girq->num_parents = 2;
		girq->parents = devm_kcalloc(dev, 2, sizeof(*girq->parents), GFP_KERNEL);
		if (!girq->parents)
			return -ENOMEM;
		girq->parents[0] = irq0;
		girq->parents[1] = irq1;
		girq->default_type = IRQ_TYPE_EDGE_FALLING;
		girq->handler = handle_edge_irq;
	}

	return devm_gpiochip_add_data(dev, &rg->gc, rg);
}

static const struct of_device_id rtl8197f_gpio_of_match[] = {
	{ .compatible = "realtek,rtl8197f-gpio" },
	{ .compatible = "realtek,rtl8197fh-gpio" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl8197f_gpio_of_match);

static struct platform_driver rtl8197f_gpio_driver = {
	.probe = rtl8197f_gpio_probe,
	.driver = {
		.name = "gpio-rtl8197f",
		.of_match_table = rtl8197f_gpio_of_match,
	},
};
module_platform_driver(rtl8197f_gpio_driver);

MODULE_DESCRIPTION("Realtek RTL8197F/RTL8197FH GPIO controller");
MODULE_AUTHOR("OpenWrt RTL8197F bring-up");
MODULE_LICENSE("GPL");
