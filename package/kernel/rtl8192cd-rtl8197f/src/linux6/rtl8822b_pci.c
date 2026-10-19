// SPDX-License-Identifier: GPL-2.0-only
/* Linux-owned PCIe resources for RTL8822B/8812BRH, RTL8812F and RTL8814B. */
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/pci.h>
#include <linux/slab.h>

#include "rtl8192cd_linux6.h"

#define RTL8822B_PCI_VENDOR_ID 0x10ec
#define RTL8822B_PCI_DEVICE_ID 0xb822
#define RTL8812F_PCI_DEVICE_ID 0xf812
#define RTL8814B_PCI_DEVICE_ID 0xb814
#define RTL8822B_MW5_BAR2_START 0x19000000UL

static int rtl8822b_find_mmio_bar(struct pci_dev *pdev)
{
	int bar;

	/* The recovered vendor 8822B-compatible path used BAR2. Prefer it when firmware exposes it. */
	if ((pci_resource_flags(pdev, 2) & IORESOURCE_MEM) && pci_resource_len(pdev, 2))
		return 2;

	for (bar = 0; bar < PCI_STD_NUM_BARS; bar++)
		if ((pci_resource_flags(pdev, bar) & IORESOURCE_MEM) && pci_resource_len(pdev, bar))
			return bar;

	return -ENODEV;
}

static int rtl8822b_mw5_host_irq(struct pci_dev *pdev)
{
	struct device *bridge_dev;
	int irq = -ENXIO;

	/* v44.28: the endpoint advertises irq=255 before the WLAN driver.  Resolve
	 * the Linux IRQ from the RTL8197F host DT node.  That DT interrupt is now
	 * the real ICTL child (input21 routed to OEM CPU5), so the returned value
	 * is a Linux child virq and must not be forced to the numeric CPU hwirq 5. */
	bridge_dev = pdev->bus ? pdev->bus->bridge : NULL;
	if (bridge_dev && bridge_dev->of_node)
		irq = of_irq_get(bridge_dev->of_node, 0);
	if (irq < 0 && bridge_dev && bridge_dev->parent && bridge_dev->parent->of_node)
		irq = of_irq_get(bridge_dev->parent->of_node, 0);

	return irq;
}

static void rtl8192cd_free_irq_vectors(void *data)
{
	pci_free_irq_vectors(data);
}

static int rtl8822b_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct rtl8192cd_linux6_hw *hw;
	void __iomem * const *iomap;
	resource_size_t bar_start, bar_len;
	u32 bar_cfg = 0;
	u16 command = 0;
	int bar;
	int host_irq;
	int ret;
	int default_rfe = of_machine_is_compatible("tenda,nova-mw5") ? 6 : -1;

	ret = pcim_enable_device(pdev);
	if (ret)
		return ret;

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
	if (ret)
		return dev_err_probe(&pdev->dev, ret, "32-bit DMA mask unavailable\n");

	pci_set_master(pdev);
	bar = rtl8822b_find_mmio_bar(pdev);
	if (bar < 0)
		return dev_err_probe(&pdev->dev, bar, "no MMIO BAR found\n");

	bar_start = pci_resource_start(pdev, bar);
	bar_len = pci_resource_len(pdev, bar);
	pci_read_config_dword(pdev, PCI_BASE_ADDRESS_0 + bar * 4, &bar_cfg);
	pci_read_config_word(pdev, PCI_COMMAND, &command);
	dev_info(&pdev->dev,
		 "RTL8192CD PCIe %04x:%04x BAR%d: resource=%pa len=%pa cfg=%#010x command=%#06x\n",
		 pdev->vendor, pdev->device, bar, &bar_start, &bar_len, bar_cfg, command);

	/*
	 * The recovered MW5 OEM init for the physical RTL8812BRH programs BAR2 to 0x19000004.
	 * A zero-based PCI child range makes Linux program BAR2 near address 0,
	 * while the RTL8197F outbound aperture and vendor HAL access 0x19000000.
	 * Refuse to touch MMIO if that invariant is not satisfied; a read from
	 * the wrongly programmed BAR causes a fatal MIPS data-bus exception.
	 */
	if (of_machine_is_compatible("tenda,nova-mw5") && bar == 2 &&
	    (bar_start != RTL8822B_MW5_BAR2_START ||
	     (bar_cfg & PCI_BASE_ADDRESS_MEM_MASK) != RTL8822B_MW5_BAR2_START))
		return dev_err_probe(&pdev->dev, -ENXIO,
			"unsafe MW5 BAR2 mapping: resource=%pa cfg=%#010x expected=0x19000004\n",
			&bar_start, bar_cfg);
	if (!(command & PCI_COMMAND_MEMORY))
		return dev_err_probe(&pdev->dev, -EIO,
			"BAR%d memory decoding disabled (command=%#06x)\n",
			bar, command);

	ret = pcim_iomap_regions_request_all(pdev, BIT(bar), "rtl8822b_pci");
	if (ret)
		return dev_err_probe(&pdev->dev, ret, "cannot map BAR%d\n", bar);

	iomap = pcim_iomap_table(pdev);
	if (!iomap || !iomap[bar])
		return dev_err_probe(&pdev->dev, -ENOMEM, "BAR%d mapping missing\n", bar);

	/* Prefer the PCI core's IRQ mapping. The RTL8197F host can leave 255
	 * before endpoint probe; resolve that case through the host DT only.
	 */
	if (!pdev->irq || pdev->irq == 255) {
		host_irq = rtl8822b_mw5_host_irq(pdev);
		if (host_irq < 0)
			return dev_err_probe(&pdev->dev, host_irq,
				"cannot resolve PCIe WLAN IRQ from host DT\n");
		pdev->irq = host_irq;
	}
	ret = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_LEGACY);
	if (ret < 0)
		return dev_err_probe(&pdev->dev, ret, "cannot allocate PCI IRQ\n");
	ret = devm_add_action_or_reset(&pdev->dev, rtl8192cd_free_irq_vectors, pdev);
	if (ret)
		return ret;
	host_irq = pci_irq_vector(pdev, 0);
	if (host_irq < 0)
		return host_irq;

	hw = devm_kzalloc(&pdev->dev, sizeof(*hw), GFP_KERNEL);
	if (!hw)
		return -ENOMEM;

	hw->dev = &pdev->dev;
	hw->pdev = pdev;
	hw->pci_id = id;
	hw->bus_type = RTL8192CD_BUS_PCI;
	hw->mmio = iomap[bar];
	hw->mmio_len = pci_resource_len(pdev, bar);
	hw->irq = host_irq;

	/* Preserve eFuse RFE on other boards unless DT supplies an override. */
	ret = rtl8192cd_linux6_parse_board_data(&pdev->dev, &hw->board, 0,
					     default_rfe, 4);
	if (ret)
		return ret;

	pci_set_drvdata(pdev, hw);
	ret = rtl8192cd_linux6_attach(hw);
	if (ret)
		return ret;

	return 0;
}

static void rtl8822b_pci_remove(struct pci_dev *pdev)
{
	struct rtl8192cd_linux6_hw *hw = pci_get_drvdata(pdev);

	rtl8192cd_linux6_detach(hw);
}

static const struct pci_device_id rtl8822b_pci_ids[] = {
#ifdef CONFIG_WLAN_HAL_8822BE
	{ PCI_DEVICE(RTL8822B_PCI_VENDOR_ID, RTL8822B_PCI_DEVICE_ID) },
#endif
#ifdef CONFIG_WLAN_HAL_8812FE
	{ PCI_DEVICE(RTL8822B_PCI_VENDOR_ID, RTL8812F_PCI_DEVICE_ID) },
#endif
#ifdef CONFIG_WLAN_HAL_8814BE
	{ PCI_DEVICE(RTL8822B_PCI_VENDOR_ID, RTL8814B_PCI_DEVICE_ID) },
#endif
	{ }
};
MODULE_DEVICE_TABLE(pci, rtl8822b_pci_ids);

static struct pci_driver rtl8822b_pci_driver = {
	.name = "rtl8822b_pci",
	.id_table = rtl8822b_pci_ids,
	.probe = rtl8822b_pci_probe,
	.remove = rtl8822b_pci_remove,
};

int rtl8822b_pci_register(void)
{
	return pci_register_driver(&rtl8822b_pci_driver);
}

void rtl8822b_pci_unregister(void)
{
	pci_unregister_driver(&rtl8822b_pci_driver);
}
