/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#ifndef __PLATFORM_OPS_H__
#define __PLATFORM_OPS_H__

/*
 * Return:
 *0:	power on successfully
 *others: power on failed
 */

#ifdef CONFIG_PLATFORM_OPS
int platform_wifi_power_on(void);
void platform_wifi_power_off(void);
void pci_cache_wback(struct pci_dev *hwdev,
	dma_addr_t *bus_addr, size_t size, int direction);
void pci_cache_inv(struct pci_dev *hwdev,
	dma_addr_t *bus_addr, size_t size, int direction);
void pci_get_bus_addr(struct pci_dev *hwdev,
	void *vir_addr, dma_addr_t *bus_addr, size_t size, int direction);
void pci_unmap_bus_addr(struct pci_dev *hwdev,
	dma_addr_t *bus_addr, size_t size, int direction);
void *pci_alloc_cache_mem(struct pci_dev *pdev,
	dma_addr_t *bus_addr, size_t size, int direction);
void *pci_alloc_noncache_mem(struct pci_dev *pdev,
	dma_addr_t *bus_addr, size_t size);
void pci_free_cache_mem(struct pci_dev *pdev, void *vir_addr,
	dma_addr_t *bus_addr, size_t size, int direction);
void pci_free_noncache_mem(struct pci_dev *pdev, void *vir_addr,
	dma_addr_t *bus_addr, size_t size);

#ifdef CONFIG_SHARE_XSTAL
/* Check if this PCI device shares other device's XO */
int pci_use_share_xo(struct pci_dev *pdev);
#else
static inline int pci_use_share_xo(struct pci_dev *pdev) { return 0; }
#endif /* CONFIG_SHARE_XSTAL */

#if defined(CONFIG_RTW_DEV_IS_SINGLE_BAND) || defined(CPTCFG_RTW_DEV_IS_SINGLE_BAND)
int dev_get_band_cap(void *dev);
#else
extern int rtw_band_type;
static inline int dev_get_band_cap(void *dev) { return rtw_band_type; }
#endif /* CONFIG_RTW_DEV_IS_SINGLE_BAND */

#else
#define platform_wifi_power_on(void) 0
#define platform_wifi_power_off(void)
#endif

/* mirror dump */
#ifdef CONFIG_RTW_MIRROR_DUMP
u32 platform_mirror_dump(u8 *hdr, u32 hdr_len, u8 *buf, u32 sz);
#else
#define platform_mirror_dump(hdr, hdr_len, buf, sz) 0
#endif

#ifdef CONFIG_RTW_WATCHDOG_KICK
void rtw_watchdog_kick(void);
#endif /* CONFIG_RTW_WATCHDOG_KICK */

#endif /* __PLATFORM_OPS_H__ */
