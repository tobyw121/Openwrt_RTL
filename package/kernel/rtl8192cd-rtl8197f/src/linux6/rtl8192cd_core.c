// SPDX-License-Identifier: GPL-2.0-only
/* Shared Linux 6.x bus/core glue; HAL/PHYDM/RF remains in the vendor core. */
#include <linux/debugfs.h>
#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/pci.h>

#include "../8192cd.h"
#include "rtl8192cd_linux6.h"

extern struct _device_info_ wlan_device[];

#define RTL8192CD_MW5_RADIOS 2

static struct dentry *rtl8192cd_debugfs_root;
extern int rtl8192cd_init_one(struct pci_dev *pdev,
			      const struct pci_device_id *ent,
			      struct _device_info_ *wdev, int vap_idx);
extern void rtl8192cd_deinit_one(struct rtl8192cd_priv *priv);

static const struct pci_device_id rtl8822b_id = {
	PCI_DEVICE(0x10ec, 0xb822)
};

int rtl8192cd_linux6_attach(struct rtl8192cd_linux6_hw *hw)
{
	struct _device_info_ *wdev;
	const struct pci_device_id *id = NULL;
	int ret;

	if (!hw || !hw->dev || !hw->mmio || hw->irq < 0)
		return -EINVAL;
	if (hw->board.radio_index >= RTL8192CD_MW5_RADIOS)
		return -EINVAL;

	wdev = &wlan_device[hw->board.radio_index];
	if (wdev->priv)
		return -EBUSY;

	memset(wdev, 0, sizeof(*wdev));
	/*
	 * Linux6/cfg80211 owns virtual-interface lifetime.  Do not ask the
	 * legacy vendor core to auto-create its WDS/mesh netdevs here.  Those
	 * side interfaces are not exposed through our nl80211 model and can
	 * already be in a non-REGISTERED state when cfg80211/hostapd tears down
	 * the root interface, which makes Linux 6.6 BUG_ON() during module
	 * removal.  Standard cfg80211 virtual interfaces can be added explicitly
	 * later without resurrecting this legacy auto-allocation path.
	 */
	wdev->type = ACCESS_SWAP_MEM |
		((hw->bus_type == RTL8192CD_BUS_PCI ? TYPE_PCI_DIRECT : TYPE_EMBEDDED) << TYPE_SHIFT);
	wdev->base_addr = (unsigned long)hw->mmio;
	wdev->irq = hw->irq;
	wdev->linux_dev = hw->dev;
	wdev->managed_mmio = hw->mmio;
	wdev->managed_mmio_len = hw->mmio_len;
	wdev->linux6_managed = true;
	wdev->radio_index = hw->board.radio_index;
	wdev->rfe_type = hw->board.rfe_type;
	wdev->rfe_valid = true;
	wdev->calibration = hw->board.calibration;
	wdev->calibration_len = hw->board.calibration_len;
	wdev->factory = hw->board.factory;
	wdev->factory_len = hw->board.factory_len;
	if (hw->board.mac_valid) {
		ether_addr_copy(wdev->mac_addr, hw->board.mac_addr);
		wdev->mac_valid = true;
	}

	if (hw->bus_type == RTL8192CD_BUS_PCI)
		id = &rtl8822b_id;

	dev_info(hw->dev, "attach vendor core: radio=%u bus=%s irq=%d mmio=%p rfe=%u\n",
		 hw->board.radio_index,
		 hw->bus_type == RTL8192CD_BUS_PCI ? "pci" : "soc",
		 hw->irq, hw->mmio, hw->board.rfe_type);

	ret = rtl8192cd_init_one(hw->pdev, id, wdev, -1);
	if (ret) {
		memset(wdev, 0, sizeof(*wdev));
		return ret;
	}

	hw->priv = wdev->priv;

#if defined(CONFIG_MW5_FORCE_WLAN_DEVICE_TABLE)
	/* v44.49: no separate coherent RTL8197FS beacon payload object.
	 * Report14 proved TXBD consumption but no OTA beacon.  The HAL now uses
	 * the OEM direct priv->beaconbuf bus-address + cache-writeback contract.
	 * Keep the legacy diagnostics at zero for report compatibility. */
	if (hw->bus_type == RTL8192CD_BUS_SOC && IS_HARDWARE_TYPE_8197F(hw->priv)) {
		hw->priv->pshare->linux6_beacon_dma_buf = NULL;
		hw->priv->pshare->linux6_beacon_dma_addr = 0;
		hw->priv->pshare->linux6_beacon_dma_size = 0;
		hw->priv->pshare->linux6_beacon_payload_mode = 0;
		hw->priv->pshare->linux6_beacon_payload_bus = 0;
	}
#endif

	if (!IS_ERR_OR_NULL(rtl8192cd_debugfs_root)) {
		hw->debugfs_dir = debugfs_create_dir(dev_name(hw->dev),
					       rtl8192cd_debugfs_root);
		if (!IS_ERR_OR_NULL(hw->debugfs_dir)) {
			debugfs_create_u8("radio_index", 0444, hw->debugfs_dir,
					  &hw->board.radio_index);
			debugfs_create_u8("rfe_type", 0444, hw->debugfs_dir,
					  &hw->board.rfe_type);
			debugfs_create_u8("active_rfe_type", 0444, hw->debugfs_dir,
					  &hw->priv->pmib->dot11RFEntry.rfe_type);
			debugfs_create_u32("trswitch", 0444, hw->debugfs_dir,
					   &hw->priv->pmib->dot11RFEntry.trswitch);
			debugfs_create_u32("mimo_mode", 0444, hw->debugfs_dir,
					   &hw->priv->pmib->dot11RFEntry.MIMO_TR_mode);
			debugfs_create_u32("tx2path", 0444, hw->debugfs_dir,
					   &hw->priv->pmib->dot11RFEntry.tx2path);
			debugfs_create_u8("mimo_hw", 0444, hw->debugfs_dir,
					  &hw->priv->pshare->phw->MIMO_TR_hw_support);
			debugfs_create_u8("txpower_offset", 0444, hw->debugfs_dir,
					  &hw->priv->pshare->phw->TXPowerOffset);
#if defined(CONFIG_MW5_FORCE_WLAN_DEVICE_TABLE)
			debugfs_create_u64("rx_dma_maps", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_dma_maps);
			debugfs_create_u64("rx_dma_unmaps", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_dma_unmaps);
			debugfs_create_u64("rx_dma_map_errors", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_dma_map_errors);
			debugfs_create_u64("rx_direct_bus", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_direct_bus);
			debugfs_create_u64("rx_direct_sync_device", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_direct_sync_device);
			debugfs_create_u64("rx_direct_sync_cpu", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_direct_sync_cpu);
			debugfs_create_u64("irq_count", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_irq_count);
			debugfs_create_u64("rx_irq_count", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_irq_count);
			debugfs_create_u64("napi_schedules", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_napi_schedules);
			debugfs_create_u64("napi_polls", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_napi_polls);
			debugfs_create_u64("rx_irq_fallback_count", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_irq_fallback_count);
			debugfs_create_u64("rx_irq_fallback_suppressed", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_irq_fallback_suppressed);
			debugfs_create_u64("rx_irq_fallback_probe", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_irq_fallback_probe);
			debugfs_create_u64("rx_pending_checks", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_pending_checks);
			debugfs_create_u64("napi_work_done", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_napi_work_done);
			debugfs_create_u64("rx_icv_errors", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_icv_errors);
			debugfs_create_u64("rx_refill_alloc_failures", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rx_refill_alloc_failures);
			debugfs_create_u64("c2h_oversize_reclassified", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_c2h_oversize_reclassified);
			debugfs_create_u32("rcr_after_init", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rcr_after_init);
			debugfs_create_u32("rxfltmap0_after_init", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rxfltmap0_after_init);
			debugfs_create_u32("rxfltmap2_after_init", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rxfltmap2_after_init);
			debugfs_create_u32("rxbd_dma_base", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rxbd_dma_base);
			debugfs_create_u32("rxbd_hw_base", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rxbd_hw_base);
			debugfs_create_u32("cr_after_init", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_cr_after_init);
			debugfs_create_u32("sys_func_en_after_init", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_sys_func_en_after_init);
			debugfs_create_u32("rf_ctrl_after_init", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rf_ctrl_after_init);
			debugfs_create_u32("afe_ctrl4_after_init", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_afe_ctrl4_after_init);
			debugfs_create_u32("afe_ldo_ctrl_after_init", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_afe_ldo_ctrl_after_init);
			debugfs_create_u32("rxbd_idx_after_init", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rxbd_idx_after_init);
			debugfs_create_u32("rxbd0_dw0", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rxbd0_dw0);
			debugfs_create_u32("rxbd0_dw1", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_rxbd0_dw1);
			debugfs_create_u32("txbd_dma_base", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_txbd_dma_base);
			debugfs_create_u32("txbd_hw_base", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_txbd_hw_base);
			debugfs_create_u32("txdesc_dma_base", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_txdesc_dma_base);
			debugfs_create_u32("bcnbd_dma_base", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_bcnbd_dma_base);
			debugfs_create_u32("bcnbd_hw_base", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_bcnbd_hw_base);
			debugfs_create_u32("bcndesc_dma_base", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_bcndesc_dma_base);
			debugfs_create_u32("mgq_idx_after_init", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_mgq_idx_after_init);
			debugfs_create_u64("tx_dma_maps", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_dma_maps);
			debugfs_create_u64("tx_dma_unmaps", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_dma_unmaps);
			debugfs_create_u64("tx_dma_map_errors", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_dma_map_errors);
			debugfs_create_u64("tx_dma_mgt_maps", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_dma_mgt_maps);
			debugfs_create_u64("tx_dma_data_maps", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_dma_data_maps);
			debugfs_create_u64("mgntdok_irq_count", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_mgntdok_irq_count);
			debugfs_create_u64("highdok_irq_count", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_highdok_irq_count);
			debugfs_create_u64("critical_hi0_tx", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_critical_hi0_tx);
			debugfs_create_u64("tx_reclaim_kicks", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_reclaim_kicks);
			debugfs_create_u64("tx_reclaim_tx_kicks", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_reclaim_tx_kicks);
			debugfs_create_u64("tx_data_submits", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_data_submits);
			debugfs_create_u64("tx_reclaim_data_kicks", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_reclaim_data_kicks);
			debugfs_create_u64("tx_redoorbell_kicks", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_redoorbell_kicks);
			debugfs_create_u64("tx_mgt_redoorbell_kicks", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_mgt_redoorbell_kicks);
			debugfs_create_u64("tx_data_redoorbell_kicks", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_tx_data_redoorbell_kicks);
			debugfs_create_u32("mgq_rwptr", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_mgq_rwptr);
			debugfs_create_u32("bkq_rwptr", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_bkq_rwptr);
			debugfs_create_u32("beq_rwptr", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_beq_rwptr);
			debugfs_create_u32("viq_rwptr", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_viq_rwptr);
			debugfs_create_u32("voq_rwptr", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_voq_rwptr);
			debugfs_create_u32("hi0_rwptr", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_hi0_rwptr);
			debugfs_create_u64("txdma_errors", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_txdma_errors);
			debugfs_create_u64("txdma_qsel_diff", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_txdma_qsel_diff);
			debugfs_create_u32("txdma_last_status", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_txdma_last_status);
			debugfs_create_u64("beacon_dma_updates", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_beacon_dma_updates);
			debugfs_create_u64("beacon_dma_alloc_errors", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_beacon_dma_alloc_errors);
			debugfs_create_size_t("beacon_dma_size", 0444, hw->debugfs_dir,
					      &hw->priv->pshare->linux6_beacon_dma_size);
			debugfs_create_u32("beacon_payload_mode", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_beacon_payload_mode);
			debugfs_create_u32("beacon_payload_bus", 0444, hw->debugfs_dir,
					   &hw->priv->pshare->linux6_beacon_payload_bus);
			debugfs_create_u32("beacon_txbd0_dw0", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txbd0_dw0);
			debugfs_create_u32("beacon_txbd0_dw1", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txbd0_dw1);
			debugfs_create_u32("beacon_txbd1_dw0", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txbd1_dw0);
			debugfs_create_u32("beacon_txbd1_dw1", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txbd1_dw1);
			debugfs_create_u32("beacon_txdesc_dw0", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txdesc_dw0);
			debugfs_create_u32("beacon_txdesc_dw1", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txdesc_dw1);
			debugfs_create_u32("beacon_txdesc_dw2", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txdesc_dw2);
			debugfs_create_u32("beacon_txdesc_dw3", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txdesc_dw3);
			debugfs_create_u32("beacon_txdesc_dw4", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txdesc_dw4);
			debugfs_create_u32("beacon_txdesc_dw5", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txdesc_dw5);
			debugfs_create_u32("rf18_a", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_rf18_a);
			debugfs_create_u32("rf18_b", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_rf18_b);
			debugfs_create_u32("mw5_binary_pwrseq", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_mw5_binary_pwrseq);
			debugfs_create_u32("pwrseq_a0", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_pwrseq_a0);
			debugfs_create_u32("pwrseq_a1", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_pwrseq_a1);
			debugfs_create_u32("pwrseq_a3", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_pwrseq_a3);
			debugfs_create_u32("pwrseq_05", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_pwrseq_05);
			debugfs_create_u32("pwrseq_06", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_pwrseq_06);
			debugfs_create_u32("soc_wlan_gate", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_soc_wlan_gate);
			debugfs_create_u32("gpio_muxcfg", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_gpio_muxcfg);
			debugfs_create_u64("trsw_reasserts", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_trsw_reasserts);
			debugfs_create_u64("eapol_mgt_redirects", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_mgt_redirects);
			debugfs_create_u64("eapol_oem_tx", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_oem_tx);
			debugfs_create_u64("eapol_desc_fills", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_desc_fills);
			debugfs_create_u64("eapol_syncs", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_syncs);
			debugfs_create_u32("eapol_last_q", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_last_q);
			debugfs_create_u32("eapol_last_rate", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_last_rate);
			debugfs_create_u32("eapol_last_privacy", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_last_privacy);
			debugfs_create_u32("eapol_last_fixed", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_last_fixed);
			debugfs_create_u32("eapol_desc_q", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_desc_q);
			debugfs_create_u32("eapol_desc_idx", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_desc_idx);
			debugfs_create_u32("eapol_desc_dw0", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_desc_dw0);
			debugfs_create_u32("eapol_desc_dw1", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_desc_dw1);
			debugfs_create_u32("eapol_desc_dw2", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_desc_dw2);
			debugfs_create_u32("eapol_desc_dw3", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_desc_dw3);
			debugfs_create_u32("eapol_desc_dw4", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_desc_dw4);
			debugfs_create_u32("eapol_desc_dw5", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_desc_dw5);
			debugfs_create_u32("eapol_txbd0_dw0", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txbd0_dw0);
			debugfs_create_u32("eapol_txbd0_dw1", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txbd0_dw1);
			debugfs_create_u32("eapol_txbd1_dw0", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txbd1_dw0);
			debugfs_create_u32("eapol_txbd1_dw1", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txbd1_dw1);
			debugfs_create_u32("eapol_txbd2_dw0", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txbd2_dw0);
			debugfs_create_u32("eapol_txbd2_dw1", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txbd2_dw1);
			debugfs_create_u32("eapol_host_before_fill", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_host_before_fill);
			debugfs_create_u32("eapol_host_after_fill", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_host_after_fill);
			debugfs_create_u32("eapol_host_before_sync", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_host_before_sync);
			debugfs_create_u32("eapol_host_after_sync", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_host_after_sync);
			debugfs_create_u32("eapol_hw_before_sync", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_hw_before_sync);
			debugfs_create_u32("eapol_hw_after_sync", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_hw_after_sync);
			debugfs_create_u32("eapol_rwptr_before_sync", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_rwptr_before_sync);
			debugfs_create_u32("eapol_rwptr_after_sync", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_rwptr_after_sync);
			debugfs_create_u32("eapol_last_macid", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_last_macid);
			debugfs_create_u32("eapol_last_tid", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_last_tid);
			debugfs_create_u32("eapol_last_hdr_len", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_last_hdr_len);
			debugfs_create_u32("eapol_last_llc_len", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_last_llc_len);
			debugfs_create_u32("eapol_last_payload_len", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_last_payload_len);
			debugfs_create_u32("eapol_need_ack", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_need_ack);
			debugfs_create_u32("eapol_hdr_dw0", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_hdr_dw0);
			debugfs_create_u32("eapol_hdr_dw1", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_hdr_dw1);
			debugfs_create_u32("eapol_hdr_dw2", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_hdr_dw2);
			debugfs_create_u32("eapol_hdr_dw3", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_hdr_dw3);
			debugfs_create_u32("eapol_hdr_dw4", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_hdr_dw4);
			debugfs_create_u32("eapol_hdr_dw5", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_hdr_dw5);
			debugfs_create_u32("eapol_hdr_dw6", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_hdr_dw6);
			debugfs_create_u32("eapol_hdr_dw7", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_hdr_dw7);
			debugfs_create_u32("eapol_hdr_dw8", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_hdr_dw8);
			debugfs_create_u32("eapol_payload_dw0", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_payload_dw0);
			debugfs_create_u32("eapol_payload_dw1", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_payload_dw1);
			debugfs_create_u32("eapol_payload_dw2", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_payload_dw2);
			debugfs_create_u32("eapol_payload_dw3", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_payload_dw3);
			debugfs_create_u64("eapol_txrpt_reports", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txrpt_reports);
			debugfs_create_u64("eapol_txrpt_txok_sum", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txrpt_txok_sum);
			debugfs_create_u64("eapol_txrpt_txfail_sum", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txrpt_txfail_sum);
			debugfs_create_u32("eapol_txrpt_last_macid", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txrpt_last_macid);
			debugfs_create_u32("eapol_txrpt_last_txok", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txrpt_last_txok);
			debugfs_create_u32("eapol_txrpt_last_txfail", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txrpt_last_txfail);
			debugfs_create_u32("eapol_txrpt_last_rate", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_eapol_txrpt_last_rate);
			debugfs_create_u64("beacon_signins", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_signins);
			debugfs_create_u64("beacon_enable_calls", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_enable_calls);
			debugfs_create_u64("beacon_disable_calls", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_disable_calls);
			debugfs_create_u64("beacon_hw_consume_seen", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_hw_consume_seen);
			debugfs_create_u32("beacon_last_action", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_last_action);
			debugfs_create_u32("beacon_own_at_signin", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_own_at_signin);
			debugfs_create_u32("beacon_own_before_set", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_own_before_set);
			debugfs_create_u32("beacon_own_after_set", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_own_after_set);
			debugfs_create_u32("beacon_pslen", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_pslen);
			debugfs_create_u32("beacon_txbd_bus", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txbd_bus);
			debugfs_create_u32("beacon_txdesc_bus", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_txdesc_bus);
			debugfs_create_u32("beacon_bcnq_desa", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_beacon_bcnq_desa);
			debugfs_create_u32("txagc_e00", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_e00);
			debugfs_create_u32("txagc_e04", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_e04);
			debugfs_create_u32("txagc_e08", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_e08);
			debugfs_create_u32("txagc_e10", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_e10);
			debugfs_create_u32("txagc_e14", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_e14);
			debugfs_create_u32("txagc_e18", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_e18);
			debugfs_create_u32("txagc_e1c", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_e1c);
			debugfs_create_u32("txagc_830", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_830);
			debugfs_create_u32("txagc_834", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_834);
			debugfs_create_u32("txagc_838", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_838);
			debugfs_create_u32("txagc_83c", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_83c);
			debugfs_create_u32("txagc_848", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_848);
			debugfs_create_u32("txagc_84c", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_84c);
			debugfs_create_u32("txagc_868", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_868);
			debugfs_create_u32("txagc_86c", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txagc_86c);
			debugfs_create_u32("txpause", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_txpause);
			debugfs_create_u32("bb40", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_bb40);
			debugfs_create_u32("bb4c", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_bb4c);
			debugfs_create_u32("bb64", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_bb64);
			debugfs_create_u32("rf30_a", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_rf30_a);
			debugfs_create_u32("rf30_b", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_rf30_b);
			debugfs_create_u32("rf31_a", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_rf31_a);
			debugfs_create_u32("rf31_b", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_rf31_b);
			debugfs_create_u32("rf32_a", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_rf32_a);
			debugfs_create_u32("rf32_b", 0444, hw->debugfs_dir, &hw->priv->pshare->linux6_rf32_b);
			debugfs_create_u8("phydm_board_type", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.board_type);
			debugfs_create_u8("phydm_ext_trsw", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.ext_trsw);
			/* v44.50: passive PHYDM receive discriminator for the integrated 8197FS. */
			debugfs_create_u32("phydm_fa_all", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_all);
			debugfs_create_u32("phydm_ofdm_fail", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_ofdm_fail);
			debugfs_create_u32("phydm_cck_fail", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_cck_fail);
			debugfs_create_u32("phydm_cca_all", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_cca_all);
			debugfs_create_u32("phydm_ofdm_cca", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_ofdm_cca);
			debugfs_create_u32("phydm_cck_cca", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_cck_cca);
			debugfs_create_u32("phydm_cck_crc_ok", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_cck_crc32_ok);
			debugfs_create_u32("phydm_cck_crc_err", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_cck_crc32_error);
			debugfs_create_u32("phydm_ofdm_crc_ok", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_ofdm_crc32_ok);
			debugfs_create_u32("phydm_ofdm_crc_err", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_ofdm_crc32_error);
			debugfs_create_u32("phydm_ht_crc_ok", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_ht_crc32_ok);
			debugfs_create_u32("phydm_ht_crc_err", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_ht_crc32_error);
			debugfs_create_u32("phydm_crc_ok_all", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_crc32_ok_all);
			debugfs_create_u32("phydm_crc_err_all", 0444, hw->debugfs_dir, &hw->priv->pshare->_dmODM.false_alm_cnt.cnt_crc32_error_all);
#endif
#if defined(HIGH_POWER_EXT_PA)
			debugfs_create_u8("use_ext_pa", 0444, hw->debugfs_dir,
					  &hw->priv->pshare->rf_ft_var.use_ext_pa);
#endif
#if defined(HIGH_POWER_EXT_LNA)
			debugfs_create_u8("use_ext_lna", 0444, hw->debugfs_dir,
					  &hw->priv->pshare->rf_ft_var.use_ext_lna);
#endif
			debugfs_create_bool("mac_from_nvmem", 0444, hw->debugfs_dir,
					    &hw->board.mac_valid);
			debugfs_create_size_t("calibration_len", 0444, hw->debugfs_dir,
					      &hw->board.calibration_len);
			debugfs_create_size_t("factory_len", 0444, hw->debugfs_dir,
					      &hw->board.factory_len);
			/* v44.20: distinguish cfg80211 AP state from real hardware beacon TX. */
			debugfs_create_u16("beacon_len", 0444, hw->debugfs_dir,
					   &hw->priv->tx_beacon_len);
			debugfs_create_ulong("beacon_ok", 0444, hw->debugfs_dir,
					     &hw->priv->ext_stats.beacon_ok);
		}
	}

	return 0;
}

void rtl8192cd_linux6_detach(struct rtl8192cd_linux6_hw *hw)
{
	struct _device_info_ *wdev;

	if (!hw || hw->board.radio_index >= RTL8192CD_MW5_RADIOS)
		return;

	debugfs_remove_recursive(hw->debugfs_dir);
	hw->debugfs_dir = NULL;

	wdev = &wlan_device[hw->board.radio_index];
	if (wdev->priv)
		rtl8192cd_deinit_one(wdev->priv);

	hw->priv = NULL;
	memset(wdev, 0, sizeof(*wdev));
}

int rtl8192cd_linux6_register_buses(void)
{
	int ret;

	pr_info("rtl8197f-wlan: v44.54 MW5 dual-radio: RTL8822B 5G critical AUTH/ASSOC responses on matching HI0/QSEL_HIGH + MGT/HI0 completion reclaim; AP-SME, RTL8197FS 40MHz/RFE5/Type5/TRSW0 and Ethernet+SSH retained\n");
	rtl8192cd_debugfs_root = debugfs_create_dir("rtl8192cd", NULL);

	/* v44.4: follow the recovered MW5 OEM wlan_device[] order exactly:
	 * entry 0 = physical RTL8812BRH PCIe (10ec:b822; vendor 8822B HAL path),
	 * entry 1 = integrated RTL8197FS.
	 * pci_register_driver() probes an already-enumerated endpoint synchronously,
	 * so registering it first also preserves the original 5G-before-2G bring-up.
	 */
	ret = rtl8822b_pci_register();
	if (ret) {
		debugfs_remove_recursive(rtl8192cd_debugfs_root);
		rtl8192cd_debugfs_root = NULL;
		return ret;
	}

	ret = rtl8197f_soc_register();
	if (ret) {
		rtl8822b_pci_unregister();
		debugfs_remove_recursive(rtl8192cd_debugfs_root);
		rtl8192cd_debugfs_root = NULL;
		return ret;
	}

	return 0;
}

void rtl8192cd_linux6_unregister_buses(void)
{
	/* Reverse of OEM registration order. */
	rtl8197f_soc_unregister();
	rtl8822b_pci_unregister();
	debugfs_remove_recursive(rtl8192cd_debugfs_root);
	rtl8192cd_debugfs_root = NULL;
}
