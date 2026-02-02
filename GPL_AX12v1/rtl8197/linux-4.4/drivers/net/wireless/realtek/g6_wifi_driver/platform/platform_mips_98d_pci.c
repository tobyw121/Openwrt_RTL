/******************************************************************************
 *
 * Copyright(c) 2019 -  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#include "drv_types.h"
#include <bspchip.h>

#if 1
void pci_cache_wback(struct pci_dev *hwdev,
			dma_addr_t *bus_addr, size_t size, int direction)
{
	if (NULL != hwdev && NULL != bus_addr)
	  	pci_dma_sync_single_for_device(hwdev, *bus_addr, size,
					direction);
	else
		RTW_ERR("pcie hwdev handle or bus addr is NULL!\n");
}
void pci_cache_inv(struct pci_dev *hwdev,
			dma_addr_t *bus_addr, size_t size, int direction)
{
	if (NULL != hwdev && NULL != bus_addr)
		pci_dma_sync_single_for_cpu(hwdev, *bus_addr, size, direction);
	else
		RTW_ERR("pcie hwdev handle or bus addr is NULL!\n");
}
void pci_get_bus_addr(struct pci_dev *hwdev,
			void *vir_addr, dma_addr_t *bus_addr,
			size_t size, int direction)
{
	if (NULL != hwdev) {
		*bus_addr = pci_map_single(hwdev, vir_addr, size, direction);
	} else {
		RTW_ERR("pcie hwdev handle is NULL!\n");
	*bus_addr = (dma_addr_t)virt_to_phys(vir_addr);
		/*RTW_ERR("Get bus_addr: %x by virt_to_phys()\n", bus_addr);*/
	}
}
void pci_unmap_bus_addr(struct pci_dev *hwdev,
			dma_addr_t *bus_addr, size_t size, int direction)
{
	if (NULL != hwdev && NULL != bus_addr)
		pci_unmap_single(hwdev, *bus_addr, size, direction);
	else
		RTW_ERR("pcie hwdev handle or bus addr is NULL!\n");
}
void *pci_alloc_cache_mem(struct pci_dev *pdev,
			dma_addr_t *bus_addr, size_t size, int direction)
{
	void *vir_addr = NULL;

	vir_addr = rtw_zmalloc(size);

	if (!vir_addr)
		bus_addr = NULL;
	else
		pci_get_bus_addr(pdev, vir_addr, bus_addr, size, direction);

	return vir_addr;
}
void *pci_alloc_noncache_mem(struct pci_dev *pdev,
			dma_addr_t *bus_addr, size_t size)
{
	void *vir_addr = NULL;

	if (NULL != pdev)
	vir_addr = pci_alloc_consistent(pdev, size, bus_addr);

	if (!vir_addr)
		bus_addr = NULL;
	else
		bus_addr = (dma_addr_t *)((((SIZE_PTR)bus_addr + 3) / 4) * 4);

	return vir_addr;
}
void pci_free_cache_mem(struct pci_dev *pdev,
		void *vir_addr, dma_addr_t *bus_addr,
		size_t size, int direction)
{
	pci_unmap_bus_addr(pdev, bus_addr, size, direction);
	rtw_mfree(vir_addr, size);

	vir_addr = NULL;
}
void pci_free_noncache_mem(struct pci_dev *pdev,
		void *vir_addr, dma_addr_t *bus_addr, size_t size)
{
	if (NULL != pdev)
	pci_free_consistent(pdev, size, vir_addr, *bus_addr);
	vir_addr = NULL;
}

#if defined(CONFIG_SHARE_XSTAL) || defined(CPTCFG_SHARE_XSTAL)
int pci_use_shared_xo(struct pci_dev *pdev)
{
	#ifdef CONFIG_SHARE_XSTAL_DEV
	#pragma message("Wlan device sharing other's XO is from CONFIG_SHARE_XSTAL_DEV: \"" CONFIG_SHARE_XSTAL_DEV "\"")
	static const char share_xo_dev[] = CONFIG_SHARE_XSTAL_DEV;
	#elif defined CPTCFG_SHARE_XSTAL_DEV
	#pragma message("Wlan device sharing other's XO is from CPTCFG_SHARE_XSTAL_DEV: \"" CPTCFG_SHARE_XSTAL_DEV "\"")
	static const char share_xo_dev[] = CPTCFG_SHARE_XSTAL_DEV;
	#else
	#pragma message("Wlan device sharing other's XO is from default value \"" "0000:02:01.0" "\"")
	/* Default value is for reference design of 98D */
	static const char share_xo_dev[] = "0000:02:01.0";
	#endif /* CONFIG_RTW_2_4G_DEV */

	const char *pci_dev_name = pci_name(pdev);

	if (strcmp(share_xo_dev, pci_dev_name) == 0) {
		return 1;
	}

	return 0;
}

#endif /* CONFIG_SHARE_XSTAL */

#if defined(CONFIG_RTW_DEV_IS_SINGLE_BAND) || defined(CPTCFG_RTW_DEV_IS_SINGLE_BAND)
int dev_get_band_cap(void *dev)
{
	struct pci_dev *pdev = (struct pci_dev *)dev;
	const char *pci_dev_name = pci_name(pdev);

	#ifdef CONFIG_RTW_2_4G_DEV
	#pragma message("2.4G wlan device is from CONFIG_RTW_2_4G_DEV: \"" CONFIG_RTW_2_4G_DEV "\"")
	static const char wlan_2_4g_dev[] = CONFIG_RTW_2_4G_DEV;
	#elif defined CPTCFG_RTW_2_4G_DEV
	#pragma message("2.4G wlan device is from CPTCFG_RTW_2_4G_DEV: \"" CPTCFG_RTW_2_4G_DEV "\"")
	static const char wlan_2_4g_dev[] = CPTCFG_RTW_2_4G_DEV;
	#else
	#pragma message("2.4G wlan device is set to default value \"" "0000:02:01.0" "\"")
	/* Default value is for reference design of 98D */
	static const char wlan_2_4g_dev[] = "0000:02:01.0";
	#endif /* CONFIG_RTW_2_4G_DEV */

	if (strcmp(wlan_2_4g_dev, pci_dev_name) == 0)
		return BAND_CAP_2G;

	return BAND_CAP_5G;
}
#endif /* CONFIG_RTW_DEV_IS_SINGLE_BAND */

#endif /* 1 */

#if 0
void pci_cache_wback(struct pci_dev *hwdev,
			dma_addr_t *bus_addr, size_t size, int direction)
{
  	pci_dma_sync_single_for_device(hwdev, *bus_addr, size, direction);
}
void pci_cache_inv(struct pci_dev *hwdev,
			dma_addr_t *bus_addr, size_t size, int direction)
{
	pci_dma_sync_single_for_cpu(hwdev, *bus_addr, size, direction);
}
void pci_get_bus_addr(struct pci_dev *hwdev,
			void *vir_addr, dma_addr_t *bus_addr,
			size_t size, int direction)
{
	*bus_addr = (dma_addr_t)virt_to_phys(vir_addr);

	return bus_addr;
}
void pci_unmap_bus_addr(struct pci_dev *hwdev,
			dma_addr_t *bus_addr, size_t size, int direction)
{
	/* pci_unmap_single(hwdev, *bus_addr, size, direction); */
}
void *pci_alloc_cache_mem(struct pci_dev *pdev,
			dma_addr_t *bus_addr, size_t size, int direction)
{
	void *vir_addr = NULL;

	vir_addr = rtw_zmalloc(size);

	if (!vir_addr) {
		bus_addr = NULL;
	}
	else {
		pci_get_bus_addr(pdev, vir_addr, bus_addr, size,
					direction);
	}

	return vir_addr;
}
void *pci_alloc_noncache_mem(struct pci_dev *pdev,
			dma_addr_t *bus_addr, size_t size)
{
	void *vir_addr = NULL;

	vir_addr = pci_alloc_consistent(pdev, size, bus_addr);

	if (!vir_addr)
		bus_addr = NULL;
	else
		bus_addr = (dma_addr_t *)((((SIZE_PTR)bus_addr + 3) / 4) * 4);
	return vir_addr;
}
void pci_free_cache_mem(struct pci_dev *pdev, void *vir_addr,
			dma_addr_t *bus_addr, size_t size, int direction)
{
	platform_mips_98d_unmap_bus_addr(pdev, bus_addr, size, direction);

	rtw_mfree(vir_addr, size);

	vir_addr = NULL;
}
void pci_free_noncache_mem(struct pci_dev *pdev, void *vir_addr,
			dma_addr_t *bus_addr, size_t size)
{
	pci_free_consistent(pdev, size, vir_addr, *bus_addr);
	vir_addr = NULL;
}
#endif

#ifdef CONFIG_RTW_MIRROR_DUMP
u32 platform_mirror_dump(u8 *hdr, u32 hdr_len, u8 *buf, u32 sz)
{
	return 0;
}
#endif

#ifdef CONFIG_RTW_WATCHDOG_KICK
void rtw_watchdog_kick(void)
{
	(REG32(BSP_WDTCNTRR) = REG32(BSP_WDTCNTRR)| WDT_KICK);
}
#endif
