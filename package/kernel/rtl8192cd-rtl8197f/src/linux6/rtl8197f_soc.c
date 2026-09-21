// SPDX-License-Identifier: GPL-2.0-only
/* RTL8197FS integrated WLAN platform backend for Linux 6.x. */
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "rtl8192cd_linux6.h"

static int rtl8197f_soc_probe(struct platform_device *pdev)
{
	struct rtl8192cd_linux6_hw *hw;
	struct resource *res;
	int ret;

	hw = devm_kzalloc(&pdev->dev, sizeof(*hw), GFP_KERNEL);
	if (!hw)
		return -ENOMEM;

	hw->dev = &pdev->dev;
	hw->bus_type = RTL8192CD_BUS_SOC;
	hw->mmio = devm_platform_get_and_ioremap_resource(pdev, 0, &res);
	if (IS_ERR(hw->mmio))
		return PTR_ERR(hw->mmio);
	hw->mmio_len = resource_size(res);

	hw->irq = platform_get_irq(pdev, 0);
	if (hw->irq < 0)
		return hw->irq;
	dev_info(&pdev->dev,
		 "MW5 RTL8197FS IRQ=%d via RTL8197F ICTL input29 -> CPU6\n",
		 hw->irq);

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
	if (ret)
		return dev_err_probe(&pdev->dev, ret, "32-bit DMA mask unavailable\n");

	/* MW5: integrated RTL8197FS is wlan1/2.4 GHz, RFE 5, factory MAC +1. */
	ret = rtl8192cd_linux6_parse_board_data(&pdev->dev, &hw->board, 1, 5, 1);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, hw);
	ret = rtl8192cd_linux6_attach(hw);
	if (ret)
		return dev_err_probe(&pdev->dev, ret, "vendor core attach failed\n");

	return 0;
}

static void rtl8197f_soc_remove(struct platform_device *pdev)
{
	struct rtl8192cd_linux6_hw *hw = platform_get_drvdata(pdev);

	rtl8192cd_linux6_detach(hw);
}

static const struct of_device_id rtl8197f_soc_of_match[] = {
	{ .compatible = "realtek,rtl8197fs-wlan" },
	{ .compatible = "realtek,rtl8197f-wmac" },
	{ .compatible = "realtek,rtl8197f-wlan" },
	{ }
};
MODULE_DEVICE_TABLE(of, rtl8197f_soc_of_match);

static struct platform_driver rtl8197f_soc_driver = {
	.probe = rtl8197f_soc_probe,
	.remove_new = rtl8197f_soc_remove,
	.driver = {
		.name = "rtl8197f_soc",
		.of_match_table = rtl8197f_soc_of_match,
	},
};

int rtl8197f_soc_register(void)
{
	return platform_driver_register(&rtl8197f_soc_driver);
}

void rtl8197f_soc_unregister(void)
{
	platform_driver_unregister(&rtl8197f_soc_driver);
}
