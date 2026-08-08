// SPDX-License-Identifier: GPL-2.0-only
/*
 * Native PCIe host bridge for the Realtek RTL8197F/RTL8197FH family.
 *
 * The reset, MDIO and configuration-window sequence is derived from the
 * GPL-released RTL8197F board support package.  The old BSP registered an
 * arch-specific pci_controller; this driver exposes the same single-device
 * root complex through the generic Linux PCI host-bridge API.
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/sizes.h>
#include <linux/spinlock.h>
#include <linux/mfd/syscon.h>

#define RTL8197F_SYSCON_CLK_MANAGE	0x010
#define RTL8197F_SYSCON_CLK_DETECT	0x008
#define RTL8197F_SYSCON_ENABLE		0x050
#define RTL8197F_SYSCON_PCIE_PHY	0x100

#define RTL8197F_PCIE_CLK_LX1		BIT(12)
#define RTL8197F_PCIE_CLK_LX2		BIT(13)
#define RTL8197F_PCIE_CLK_ACTIVE	BIT(14)
#define RTL8197F_PCIE_CLK_PHY		BIT(18)
#define RTL8197F_PCIE_PERST_N		BIT(1)
#define RTL8197F_PCIE_REFCLK_40MHZ	BIT(24)

#define RTL8197F_PCIE_EXT_MDIO		0x1000
#define RTL8197F_PCIE_EXT_PWRCR		0x1008
#define RTL8197F_PCIE_EXT_IPCFG		0x100c
#define RTL8197F_PCIE_LINK_STATE	0x0728

#define RTL8197F_PCIE_MDIO_REG_SHIFT	8
#define RTL8197F_PCIE_MDIO_DATA_SHIFT	16
#define RTL8197F_PCIE_MDIO_WRITE	BIT(0)
#define RTL8197F_PCIE_LINK_UP		0x11
#define RTL8197F_PCIE_COMMAND_ENABLE	0x00100007

#define RTL8197F_PCIE_IO_BASE		0x18c00000
#define RTL8197F_PCIE_IO_SIZE		SZ_2M
#define RTL8197F_PCIE_MEM_BASE		0x19000000
#define RTL8197F_PCIE_MEM_SIZE		SZ_16M

struct rtl8197f_pcie {
	struct device *dev;
	struct regmap *syscon;
	void __iomem *rc;
	void __iomem *ep;
	struct pci_host_bridge *bridge;
	struct resource io_res;
	struct resource mem_res;
	struct resource busn_res;
	spinlock_t cfg_lock;
	int irq;
	bool refclk_40mhz;
	bool vg_phy_tuning;
};

static void rtl8197f_pcie_mdio_write(struct rtl8197f_pcie *pcie,
				     u8 reg, u16 value)
{
	u32 command;

	command = FIELD_PREP(GENMASK(12, 8), reg & 0x1f) |
		  FIELD_PREP(GENMASK(31, 16), value) |
		  RTL8197F_PCIE_MDIO_WRITE;
	writel(command, pcie->rc + RTL8197F_PCIE_EXT_MDIO);

	/* The BSP used a short uncached busy loop after each MDIO write. */
	udelay(100);
}

static int rtl8197f_pcie_hw_init(struct rtl8197f_pcie *pcie)
{
	u32 value;
	int ret;

	ret = regmap_update_bits(pcie->syscon, RTL8197F_SYSCON_CLK_MANAGE,
				 RTL8197F_PCIE_CLK_LX1 |
				 RTL8197F_PCIE_CLK_LX2 |
				 RTL8197F_PCIE_CLK_PHY |
				 RTL8197F_PCIE_CLK_ACTIVE,
				 RTL8197F_PCIE_CLK_LX1 |
				 RTL8197F_PCIE_CLK_LX2 |
				 RTL8197F_PCIE_CLK_PHY |
				 RTL8197F_PCIE_CLK_ACTIVE);
	if (ret)
		return ret;

	mdelay(10);

	/* BSP MDIO reset: reset low, reset high, then load_done high. */
	ret = regmap_write(pcie->syscon, RTL8197F_SYSCON_PCIE_PHY, BIT(3));
	if (ret)
		return ret;
	ret = regmap_write(pcie->syscon, RTL8197F_SYSCON_PCIE_PHY,
			   BIT(3) | BIT(0));
	if (ret)
		return ret;
	ret = regmap_write(pcie->syscon, RTL8197F_SYSCON_PCIE_PHY,
			   BIT(3) | BIT(1) | BIT(0));
	if (ret)
		return ret;

	mdelay(10);

	/* Enable LTSSM and release the PHY reset. */
	writel(0x01, pcie->rc + RTL8197F_PCIE_EXT_PWRCR);
	writel(0x81, pcie->rc + RTL8197F_PCIE_EXT_PWRCR);
	mdelay(10);

	if (pcie->refclk_40mhz) {
		rtl8197f_pcie_mdio_write(pcie, 0x0f, 0x12f6);
		rtl8197f_pcie_mdio_write(pcie, 0x00, 0x0071);
		rtl8197f_pcie_mdio_write(pcie, 0x06, 0x1ac1);
		if (pcie->vg_phy_tuning)
			rtl8197f_pcie_mdio_write(pcie, 0x08, 0x3101);
	} else {
		rtl8197f_pcie_mdio_write(pcie, 0x00, 0x0071);
		rtl8197f_pcie_mdio_write(pcie, 0x06, 0x18c1);
	}

	mdelay(10);
	writel(0x01, pcie->rc + RTL8197F_PCIE_EXT_PWRCR);
	writel(0x81, pcie->rc + RTL8197F_PCIE_EXT_PWRCR);

	/* Hold the endpoint in PERST# for the 300 ms used by the BSP. */
	ret = regmap_update_bits(pcie->syscon, RTL8197F_SYSCON_ENABLE,
				 RTL8197F_PCIE_PERST_N, 0);
	if (ret)
		return ret;
	msleep(300);
	ret = regmap_update_bits(pcie->syscon, RTL8197F_SYSCON_ENABLE,
				 RTL8197F_PCIE_PERST_N,
				 RTL8197F_PCIE_PERST_N);
	if (ret)
		return ret;

	ret = readl_poll_timeout(pcie->rc + RTL8197F_PCIE_LINK_STATE, value,
				 (value & 0x1f) == RTL8197F_PCIE_LINK_UP,
				 10000, 250000);
	if (ret) {
		dev_err(pcie->dev, "PCIe link did not reach state 0x11 (last %#x)\n",
			value & 0x1f);
		regmap_update_bits(pcie->syscon, RTL8197F_SYSCON_CLK_MANAGE,
				   RTL8197F_PCIE_CLK_ACTIVE, 0);
		return ret;
	}

	/* Enable IO, MEM, bus mastering and the BSP's extended command bit. */
	writel(RTL8197F_PCIE_COMMAND_ENABLE, pcie->rc + PCI_COMMAND);
	writel(RTL8197F_PCIE_COMMAND_ENABLE, pcie->ep + PCI_COMMAND);

	value = readl(pcie->ep + PCI_VENDOR_ID);
	dev_info(pcie->dev,
		 "native host ready: endpoint=%04x:%04x, refclk=%s, IRQ=%d\n",
		 value & 0xffff, value >> 16,
		 pcie->refclk_40mhz ? "40MHz" : "25MHz", pcie->irq);

	return 0;
}

static void rtl8197f_pcie_select_bdf(struct rtl8197f_pcie *pcie,
				     struct pci_bus *bus, unsigned int devfn)
{
	u32 value;
	u32 bdf = ((bus->number << 8) | devfn) & 0xffff;

	value = readl(pcie->rc + RTL8197F_PCIE_EXT_IPCFG);
	value &= ~GENMASK(15, 0);
	value |= bdf;
	writel(value, pcie->rc + RTL8197F_PCIE_EXT_IPCFG);
}

static int rtl8197f_pcie_config_read(struct pci_bus *bus, unsigned int devfn,
				     int where, int size, u32 *value)
{
	struct rtl8197f_pcie *pcie = bus->sysdata;
	unsigned long flags;

	*value = ~0U;
	if (bus->number != 0 || PCI_SLOT(devfn) != 0 || where < 0 ||
	    where + size > SZ_4K || (where & (size - 1)))
		return PCIBIOS_DEVICE_NOT_FOUND;

	spin_lock_irqsave(&pcie->cfg_lock, flags);
	rtl8197f_pcie_select_bdf(pcie, bus, devfn);
	switch (size) {
	case 1:
		*value = readb(pcie->ep + where);
		break;
	case 2:
		*value = readw(pcie->ep + where);
		break;
	case 4:
		*value = readl(pcie->ep + where);
		break;
	default:
		spin_unlock_irqrestore(&pcie->cfg_lock, flags);
		return PCIBIOS_BAD_REGISTER_NUMBER;
	}
	spin_unlock_irqrestore(&pcie->cfg_lock, flags);

	return PCIBIOS_SUCCESSFUL;
}

static int rtl8197f_pcie_config_write(struct pci_bus *bus, unsigned int devfn,
				      int where, int size, u32 value)
{
	struct rtl8197f_pcie *pcie = bus->sysdata;
	unsigned long flags;

	if (bus->number != 0 || PCI_SLOT(devfn) != 0 || where < 0 ||
	    where + size > SZ_4K || (where & (size - 1)))
		return PCIBIOS_DEVICE_NOT_FOUND;

	spin_lock_irqsave(&pcie->cfg_lock, flags);
	rtl8197f_pcie_select_bdf(pcie, bus, devfn);
	switch (size) {
	case 1:
		writeb(value, pcie->ep + where);
		break;
	case 2:
		writew(value, pcie->ep + where);
		break;
	case 4:
		writel(value, pcie->ep + where);
		break;
	default:
		spin_unlock_irqrestore(&pcie->cfg_lock, flags);
		return PCIBIOS_BAD_REGISTER_NUMBER;
	}
	spin_unlock_irqrestore(&pcie->cfg_lock, flags);

	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops rtl8197f_pcie_ops = {
	.read = rtl8197f_pcie_config_read,
	.write = rtl8197f_pcie_config_write,
};

static int rtl8197f_pcie_map_irq(const struct pci_dev *pdev, u8 slot, u8 pin)
{
	struct rtl8197f_pcie *pcie = pdev->bus->sysdata;

	if (!pin || slot)
		return 0;

	return pcie->irq;
}

static void rtl8197f_pcie_init_resources(struct rtl8197f_pcie *pcie)
{
	struct pci_host_bridge *bridge = pcie->bridge;

	pcie->io_res = (struct resource) {
		.name = "RTL8197F PCIe IO",
		.start = RTL8197F_PCIE_IO_BASE,
		.end = RTL8197F_PCIE_IO_BASE + RTL8197F_PCIE_IO_SIZE - 1,
		.flags = IORESOURCE_IO,
	};
	pcie->mem_res = (struct resource) {
		.name = "RTL8197F PCIe MEM",
		.start = RTL8197F_PCIE_MEM_BASE,
		.end = RTL8197F_PCIE_MEM_BASE + RTL8197F_PCIE_MEM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	};
	pcie->busn_res = (struct resource) {
		.name = "RTL8197F PCIe bus",
		.start = 0,
		.end = 0,
		.flags = IORESOURCE_BUS,
	};

	pci_add_resource(&bridge->windows, &pcie->io_res);
	pci_add_resource(&bridge->windows, &pcie->mem_res);
	pci_add_resource(&bridge->windows, &pcie->busn_res);
}

static int rtl8197f_pcie_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pci_host_bridge *bridge;
	struct rtl8197f_pcie *pcie;
	u32 value;
	int ret;

	bridge = devm_pci_alloc_host_bridge(dev, sizeof(*pcie));
	if (!bridge)
		return -ENOMEM;

	pcie = pci_host_bridge_priv(bridge);
	pcie->dev = dev;
	pcie->bridge = bridge;
	spin_lock_init(&pcie->cfg_lock);

	pcie->rc = devm_platform_ioremap_resource_byname(pdev, "rc");
	if (IS_ERR(pcie->rc))
		return PTR_ERR(pcie->rc);

	pcie->ep = devm_platform_ioremap_resource_byname(pdev, "ep");
	if (IS_ERR(pcie->ep))
		return PTR_ERR(pcie->ep);

	pcie->syscon = syscon_regmap_lookup_by_phandle(dev->of_node,
						  "realtek,syscon");
	if (IS_ERR(pcie->syscon))
		return dev_err_probe(dev, PTR_ERR(pcie->syscon),
				     "missing RTL8197F syscon\n");

	pcie->irq = platform_get_irq(pdev, 0);
	if (pcie->irq < 0)
		return pcie->irq;

	pcie->vg_phy_tuning = of_property_read_bool(dev->of_node,
						    "realtek,rtl8197f-vg");
	pcie->refclk_40mhz = of_property_read_bool(dev->of_node,
						   "realtek,refclk-40mhz");
	if (!pcie->refclk_40mhz &&
	    !regmap_read(pcie->syscon, RTL8197F_SYSCON_CLK_DETECT, &value))
		pcie->refclk_40mhz = !!(value & RTL8197F_PCIE_REFCLK_40MHZ);

	ret = rtl8197f_pcie_hw_init(pcie);
	if (ret)
		return ret;

	bridge->sysdata = pcie;
	bridge->ops = &rtl8197f_pcie_ops;
	bridge->busnr = 0;
	bridge->domain_nr = 0;
	bridge->swizzle_irq = pci_common_swizzle;
	bridge->map_irq = rtl8197f_pcie_map_irq;
	bridge->size_windows = 1;

	rtl8197f_pcie_init_resources(pcie);
	ret = devm_request_pci_bus_resources(dev, &bridge->windows);
	if (ret)
		return dev_err_probe(dev, ret, "cannot reserve PCIe windows\n");

	ret = pci_host_probe(bridge);
	if (ret)
		return dev_err_probe(dev, ret, "PCI host probe failed\n");

	platform_set_drvdata(pdev, pcie);
	return 0;
}

static void rtl8197f_pcie_remove(struct platform_device *pdev)
{
	struct rtl8197f_pcie *pcie = platform_get_drvdata(pdev);

	if (pcie->bridge->bus) {
		pci_stop_root_bus(pcie->bridge->bus);
		pci_remove_root_bus(pcie->bridge->bus);
	}

	regmap_update_bits(pcie->syscon, RTL8197F_SYSCON_CLK_MANAGE,
			   RTL8197F_PCIE_CLK_ACTIVE, 0);
}

static const struct of_device_id rtl8197f_pcie_of_match[] = {
	{ .compatible = "realtek,rtl8197f-pcie" },
	{ }
};
MODULE_DEVICE_TABLE(of, rtl8197f_pcie_of_match);

static struct platform_driver rtl8197f_pcie_driver = {
	.probe = rtl8197f_pcie_probe,
	.remove_new = rtl8197f_pcie_remove,
	.driver = {
		.name = "rtl8197f-pcie",
		.of_match_table = rtl8197f_pcie_of_match,
	},
};
module_platform_driver(rtl8197f_pcie_driver);

MODULE_DESCRIPTION("Realtek RTL8197F native PCIe host bridge");
MODULE_AUTHOR("OpenWrt RTL8197F port");
MODULE_LICENSE("GPL");
