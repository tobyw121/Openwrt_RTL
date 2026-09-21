// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Realtek RTL8197F/RTL8197FH USB2 host glue.
 *
 * The RTL8197F BSP exposes standard EHCI/OHCI register blocks, but clocks,
 * host/OTG selection and the two USB PHYs must be enabled through SoC and
 * host-control registers first.  This driver performs only that glue and
 * then instantiates generic-ehci/generic-ohci child nodes.
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

#define RTL8197F_SYS_CHIP_ID		0x0008
#define RTL8197F_SYS_CLK_MANAGE		0x0014
#define RTL8197F_SYS_USB_HOST_EN	0x0160
#define RTL8197F_SYS_USB_PHY		0x016c
#define RTL8197F_SYS_USB_MODE		0x0180

#define RTL8197F_USB_CLK_EN		BIT(3)
#define RTL8197F_USB_HOST_EN		BIT(0)
#define RTL8197F_USB_TWO_PORTS		BIT(17)
#define RTL8197F_USB_PORT1_HOST	BIT(18)
#define RTL8197F_XTAL_40MHZ		BIT(24)

#define RTL8197F_USB_DEBUG		0x08
#define RTL8197F_USB_PHY_DATA		0x0c
#define RTL8197F_USB_HOST_CTRL		0x10
#define RTL8197F_USB_DEBUG_SELECT	0x02000000
#define RTL8197F_USB_SUSPEND_FIX	0x02000200

#define RTL8197F_USB_PHY0_EN		BIT(8)
#define RTL8197F_USB_PHY0_RESET	BIT(9)
#define RTL8197F_USB_PHY0_ACTIVE	BIT(10)
#define RTL8197F_USB_PHY1_EN		BIT(19)
#define RTL8197F_USB_PHY1_RESET	BIT(20)
#define RTL8197F_USB_PHY1_ACTIVE	BIT(21)

struct rtl8197f_usb {
	struct device *dev;
	struct regmap *syscon;
	void __iomem *host;
	void __iomem *phy_cmd;
};

static int rtl8197f_usb_sys_update(struct rtl8197f_usb *usb, unsigned int reg,
				   unsigned int mask, unsigned int val)
{
	int ret = regmap_update_bits(usb->syscon, reg, mask, val);

	if (ret)
		dev_err(usb->dev, "syscon update 0x%x failed: %d\n", reg, ret);
	return ret;
}

static void rtl8197f_usb_phy_write(struct rtl8197f_usb *usb, unsigned int port,
				   u8 reg, u8 val)
{
	u32 select = port ? 0x5000 : 0x3000;
	u32 clear = port ? 0x4000 : 0x2000;
	u32 reg_lo = reg & 0x0f;
	u32 reg_hi = (reg >> 4) & 0x0f;
	u32 data = 0x00340000 | (port ? ((u32)val << 8) : val);

	writel(data, usb->host + RTL8197F_USB_PHY_DATA);
	msleep(10);
	writel(select | (reg_lo << 8), usb->phy_cmd);
	msleep(10);
	writel(clear | (reg_lo << 8), usb->phy_cmd);
	msleep(10);
	writel(select | (reg_lo << 8), usb->phy_cmd);
	msleep(10);
	writel(select | (reg_hi << 8), usb->phy_cmd);
	msleep(10);
	writel(clear | (reg_hi << 8), usb->phy_cmd);
	msleep(10);
	writel(select | (reg_hi << 8), usb->phy_cmd);
	msleep(10);
}

static void rtl8197f_usb_phy_tune_40mhz(struct rtl8197f_usb *usb,
					unsigned int port)
{
	/* Exact RTL8197F-B-cut 40 MHz sequence from the GPL BSP. */
	rtl8197f_usb_phy_write(usb, port, 0xf4, 0x9b);
	rtl8197f_usb_phy_write(usb, port, 0xe2, 0x33);
	rtl8197f_usb_phy_write(usb, port, 0xe4, 0xc9);
	rtl8197f_usb_phy_write(usb, port, 0xe6, 0xc1);
	rtl8197f_usb_phy_write(usb, port, 0xf4, 0xbb);
	rtl8197f_usb_phy_write(usb, port, 0xe6, 0x00);
	rtl8197f_usb_phy_write(usb, port, 0xe7, 0x00);
	rtl8197f_usb_phy_write(usb, port, 0xf4, 0x9b);
}

static void rtl8197f_usb_phy_tune_25mhz(struct rtl8197f_usb *usb,
					unsigned int port)
{
	/* Exact 25 MHz sequence from the same RTL8197F BSP. */
	rtl8197f_usb_phy_write(usb, port, 0xf4, 0x9b);
	rtl8197f_usb_phy_write(usb, port, 0xe0, 0xe3);
	rtl8197f_usb_phy_write(usb, port, 0xe1, 0x30);
	rtl8197f_usb_phy_write(usb, port, 0xe2, 0xd5);
	rtl8197f_usb_phy_write(usb, port, 0xe4, 0xc9);
	rtl8197f_usb_phy_write(usb, port, 0xe6, 0xc1);
	rtl8197f_usb_phy_write(usb, port, 0xf4, 0xbb);
	rtl8197f_usb_phy_write(usb, port, 0xe5, 0x11);
	rtl8197f_usb_phy_write(usb, port, 0xe6, 0x06);
	rtl8197f_usb_phy_write(usb, port, 0xe7, 0x66);
	rtl8197f_usb_phy_write(usb, port, 0xf4, 0x9b);
}

static int rtl8197f_usb_hw_init(struct rtl8197f_usb *usb)
{
	u32 chip = 0;
	u32 phy;
	bool refclk_40mhz;
	int ret;

	ret = rtl8197f_usb_sys_update(usb, RTL8197F_SYS_CLK_MANAGE,
				      RTL8197F_USB_CLK_EN, RTL8197F_USB_CLK_EN);
	if (ret)
		return ret;
	msleep(50);

	writel(readl(usb->host + RTL8197F_USB_HOST_CTRL) &
	       ~RTL8197F_USB_SUSPEND_FIX,
	       usb->host + RTL8197F_USB_HOST_CTRL);

	ret = rtl8197f_usb_sys_update(usb, RTL8197F_SYS_USB_MODE,
				      RTL8197F_USB_TWO_PORTS | RTL8197F_USB_PORT1_HOST,
				      RTL8197F_USB_TWO_PORTS | RTL8197F_USB_PORT1_HOST);
	if (ret)
		return ret;

	writel(RTL8197F_USB_DEBUG_SELECT, usb->host + RTL8197F_USB_DEBUG);
	ret = rtl8197f_usb_sys_update(usb, RTL8197F_SYS_USB_HOST_EN,
				      RTL8197F_USB_HOST_EN, RTL8197F_USB_HOST_EN);
	if (ret)
		return ret;

	ret = regmap_read(usb->syscon, RTL8197F_SYS_USB_PHY, &phy);
	if (ret)
		return ret;
	phy |= RTL8197F_USB_PHY0_EN;
	ret = regmap_write(usb->syscon, RTL8197F_SYS_USB_PHY,
			   phy | RTL8197F_USB_PHY0_RESET);
	if (ret)
		return ret;
	udelay(10);
	phy = (phy & ~RTL8197F_USB_PHY0_RESET) | RTL8197F_USB_PHY0_ACTIVE;
	ret = regmap_write(usb->syscon, RTL8197F_SYS_USB_PHY, phy);
	if (ret)
		return ret;
	writel((readl(usb->host + RTL8197F_USB_HOST_CTRL) | BIT(7)) & ~BIT(5),
	       usb->host + RTL8197F_USB_HOST_CTRL);

	phy |= RTL8197F_USB_PHY1_EN;
	ret = regmap_write(usb->syscon, RTL8197F_SYS_USB_PHY,
			   phy | RTL8197F_USB_PHY1_RESET);
	if (ret)
		return ret;
	udelay(10);
	phy = (phy & ~RTL8197F_USB_PHY1_RESET) | RTL8197F_USB_PHY1_ACTIVE;
	ret = regmap_write(usb->syscon, RTL8197F_SYS_USB_PHY, phy);
	if (ret)
		return ret;
	writel((readl(usb->host + RTL8197F_USB_HOST_CTRL) | BIT(23)) & ~BIT(21),
	       usb->host + RTL8197F_USB_HOST_CTRL);

	regmap_read(usb->syscon, RTL8197F_SYS_CHIP_ID, &chip);
	refclk_40mhz = device_property_read_bool(usb->dev,
						 "realtek,refclk-40mhz") ||
			 (chip & RTL8197F_XTAL_40MHZ);

	if (!device_property_read_bool(usb->dev, "realtek,skip-phy-tuning")) {
		if (refclk_40mhz) {
			rtl8197f_usb_phy_tune_40mhz(usb, 0);
			rtl8197f_usb_phy_tune_40mhz(usb, 1);
		} else {
			rtl8197f_usb_phy_tune_25mhz(usb, 0);
			rtl8197f_usb_phy_tune_25mhz(usb, 1);
		}
	}

	msleep(100);
	dev_info(usb->dev,
		 "RTL8197F USB2 host initialized: two ports, refclk=%uMHz phy=0x%08x ctrl=0x%08x\n",
		 refclk_40mhz ? 40 : 25, phy,
		 readl(usb->host + RTL8197F_USB_HOST_CTRL));
	return 0;
}

static int rtl8197f_usb_probe(struct platform_device *pdev)
{
	struct rtl8197f_usb *usb;
	int ret;

	usb = devm_kzalloc(&pdev->dev, sizeof(*usb), GFP_KERNEL);
	if (!usb)
		return -ENOMEM;

	usb->dev = &pdev->dev;
	usb->syscon = syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
						      "realtek,syscon");
	if (IS_ERR(usb->syscon))
		return dev_err_probe(&pdev->dev, PTR_ERR(usb->syscon),
				     "failed to get syscon\n");

	usb->host = devm_platform_ioremap_resource_byname(pdev, "hostctrl");
	if (IS_ERR(usb->host))
		return PTR_ERR(usb->host);

	/* The BSP PHY command register sits at EHCI + 0xa4.  Do not describe it
	 * as a second parent resource: that would overlap the generic EHCI child
	 * resource and make platform_device_add() reject the child.  Map the four
	 * bytes without reserving the full EHCI aperture; the generic HCD owns it.
	 */
	{
		struct device_node *ehci_np;
		struct resource ehci_res;

		ehci_np = of_get_child_by_name(pdev->dev.of_node, "usb@21000");
		if (!ehci_np)
			return dev_err_probe(&pdev->dev, -ENODEV,
					     "EHCI child node is missing\n");
		ret = of_address_to_resource(ehci_np, 0, &ehci_res);
		of_node_put(ehci_np);
		if (ret)
			return dev_err_probe(&pdev->dev, ret,
					     "failed to translate EHCI resource\n");
		usb->phy_cmd = devm_ioremap(&pdev->dev, ehci_res.start + 0xa4, 4);
		if (!usb->phy_cmd)
			return -ENOMEM;
	}

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
	if (ret)
		return dev_err_probe(&pdev->dev, ret, "32-bit DMA mask unavailable\n");

	ret = rtl8197f_usb_hw_init(usb);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, usb);
	ret = of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
	if (ret)
		return dev_err_probe(&pdev->dev, ret,
				     "failed to create EHCI/OHCI children\n");
	return 0;
}

static void rtl8197f_usb_remove(struct platform_device *pdev)
{
	of_platform_depopulate(&pdev->dev);
}

static const struct of_device_id rtl8197f_usb_of_match[] = {
	{ .compatible = "realtek,rtl8197f-usb-host" },
	{ }
};
MODULE_DEVICE_TABLE(of, rtl8197f_usb_of_match);

static struct platform_driver rtl8197f_usb_driver = {
	.probe = rtl8197f_usb_probe,
	.remove_new = rtl8197f_usb_remove,
	.driver = {
		.name = "rtl8197f-usb-host",
		.of_match_table = rtl8197f_usb_of_match,
	},
};
module_platform_driver(rtl8197f_usb_driver);

MODULE_DESCRIPTION("Realtek RTL8197F USB2 host glue");
MODULE_LICENSE("GPL");
