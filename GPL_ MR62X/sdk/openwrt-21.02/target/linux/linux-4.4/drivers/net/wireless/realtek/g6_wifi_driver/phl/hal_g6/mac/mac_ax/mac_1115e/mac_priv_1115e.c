/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
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
 ******************************************************************************/

#include "../mac_priv.h"
#include "init_1115e.h"
#include "pwr_seq_func_1115e.h"
#include "security_cam_1115e.h"
#include "cmac_tx_1115e.h"
#include "fwcmd_1115e.h"
#include "ser_1115e.h"
#include "rrsr_1115e.h"
#include "coex_1115e.h"
#include "../tblupd.h"
#include "phy_rpt_1115e.h"
#include "efuse_1115e.h"
#include "dle_1115e.h"
#include "trxcfg_1115e.h"
#include "hci_fc_1115e.h"
#if MAC_AX_PCIE_SUPPORT
#include "_pcie_1115e.h"
#endif
#if MAC_AX_USB_SUPPORT
#include "_usb_1115e.h"
#endif
#if MAC_AX_SDIO_SUPPORT
#include "_sdio_1115e.h"
#endif

#if MAC_AX_1115E_SUPPORT

static struct mac_ax_priv_ops mac1115e_priv_ops = {
	NULL, /* pwr_off */
	NULL, /* pwr_on */
	NULL, /* intf_pwr_switch */
	dmac_func_en_1115e, /* dmac_func_en */
	dmac_func_pre_en_1115e, /* dmac_func_pre_en */
	cmac_init_1115e, /*cmac module init*/
	mac_init_cctl_info_1115e, /* init cmac table */
	cmac_func_en_1115e,/* cmac_func_en */
	macid_idle_ck_1115e, /* macid_idle_ck */
	stop_sch_tx_1115e, /* stop_sch_tx */
	switch_efuse_bank_1115e, /* switch_efuse_bank */
	enable_efuse_sw_pwr_cut_1115e, /* enable_efuse_sw_pwr_cut */
	disable_efuse_sw_pwr_cut_1115e, /* disable_efuse_sw_pwr_cut */
	get_h2creg_offset_1115e, /* get_h2creg_offset */
	get_c2hreg_offset_1115e, /* get_c2hreg_offset */
	ser_imr_config_1115e,
	disconnect_flush_key_1115e, /* disconnect_flush_key */
	NULL, /* sec_info_tbl_init */
	NULL, /* free_sec_info_tbl */
	mac_wowlan_secinfo_1115e,/* free_sec_info_tbl */
	mac_get_rrsr_cfg_1115e, /*get RRSR related config*/
	mac_set_rrsr_cfg_1115e, /*set RRSR related config*/
	mac_get_cts_rrsr_cfg_1115e, /*get CTS RRSR related config*/
	mac_set_cts_rrsr_cfg_1115e, /*set CTS RRSR related config*/
	mac_cfg_gnt_1115e, /* cfg_ctrl_path*/
	mac_cfg_ctrl_path_1115e, /* cfg_ctrl_path */
	mac_get_gnt_1115e, /* get_gnt */
	mac_get_ctrl_path_1115e, /* get_ctrl_path */
	get_bbrpt_dle_cfg_1115e, /*get_bbrpt_dle_cfg*/
	dbg_port_sel_1115e, /*for mac debug port*/
	tx_flow_ptcl_dbg_port_1115e, /*for mac tx flow ptcl dbg*/
	tx_flow_sch_dbg_port_1115e, /*for mac tx schdueler ptcl dbg*/
	ss_stat_chk_1115e, /*for mac station scheduler check*/
	dmac_dbg_dump_1115e, /*for dmac debug dump*/
	cmac_dbg_dump_1115e, /*for cmac debug dump*/
	crit_dbg_dump_1115e, /*for system critical debug dump*/
	tx_dbg_dump_1115e, /*for tx flow debug dump*/
	coex_mac_init_1115e, /* coex_mac_init */
	set_fc_page_ctrl_reg_1115e, /* set_fc_page_ctrl_reg */
	get_fc_page_info_1115e, /* get_fc_page_info */
	set_fc_pubpg_1115e, /* set_fc_pubpg */
	get_fc_mix_info_1115e, /* get_fc_mix_info */
	set_fc_h2c_1115e, /* set_fc_h2c */
	set_fc_mix_cfg_1115e, /* set_fc_mix_cfg */
	set_fc_func_en_1115e, /* set_fc_func_en */
	dle_dfi_ctrl_1115e, /* dle_dfi_ctrl */
	dle_is_txq_empty_1115e, /* dle_is_txq_empty */
	dle_is_rxq_empty_1115e, /* dle_is_rxq_empty */
	preload_cfg_set_1115e, /* preload_cfg_set */
	preload_init_set_1115e, /* preload_init_set */
	dle_func_en_1115e, /* dle_func_en */
	dle_clk_en_1115e, /* dle_clk_en */
	dle_mix_cfg_1115e, /* dle_mix_cfg */
	wde_quota_cfg_1115e, /* wde_quota_cfg */
	ple_quota_cfg_1115e, /* ple_quota_cfg */
	chk_dle_rdy_1115e, /* chk_dle_rdy */
	is_dbg_port_not_valid_1115e, /* is_dbg_port_not_valid */
	dbg_port_sel_rst_1115e, /* dbg_port_sel_rst */
	dle_dfi_sel_1115e, /* dle_dfi_sel */
	mac_bacam_init, /* bacam init */
#if MAC_AX_PCIE_SUPPORT
	get_pcie_info_def_1115e, /* get_pcie_info_def */
	get_bdram_tbl_pcie_1115e, /* get_bdram_tbl_pcie */
	mio_w32_pcie_1115e, /* mio_w32_pcie */
	mio_r32_pcie_1115e, /* mio_r32_pcie */
	get_txbd_reg_pcie_1115e, /* get_txbd_reg_pcie */
	set_txbd_reg_pcie_1115e, /* set_txbd_reg_pcie */
	get_rxbd_reg_pcie_1115e, /* get_rxbd_reg_pcie */
	set_rxbd_reg_pcie_1115e, /* set_rxbd_reg_pcie */
	ltr_sw_trigger_1115e, /* ltr_sw_trigger */
	pcie_cfgspc_write_1115e, /* pcie_cfgspc_write */
	pcie_cfgspc_read_1115e, /* pcie_cfgspc_read */
	pcie_ltr_write_1115e, /* pcie_ltr_write */
	pcie_ltr_read_1115e, /* pcie_ltr_read */
	ctrl_hci_dma_en_pcie_1115e, /* ctrl_hci_dma_en_pcie */
	ctrl_trxdma_pcie_1115e, /* ctrl_trxdma_pcie */
	ctrl_wpdma_pcie_1115e, /* ctrl_wpdma_pcie */
	poll_io_idle_pcie_1115e, /* poll_io_idle_pcie */
	poll_dma_all_idle_pcie_1115e, /* poll_dma_all_idle_pcie */
	clr_idx_ch_pcie_1115e, /* clr_idx_ch_pcie */
	rst_bdram_pcie_1115e, /* rst_bdram_pcie */
	trx_mit_pcie_1115e, /* trx_mit_pcie */
	mode_op_pcie_1115e, /* mode_op_pcie */
	get_err_flag_pcie_1115e, /* get_err_flag_pcie */
	mac_auto_refclk_cal_pcie_1115e, /* mac_auto_refclk_cal_pcie */
#ifdef RTW_WKARD_GET_PROCESSOR_ID
	chk_proc_long_ldy, /* chk_proc_long_ldy_pcie */
#endif
	sync_trx_bd_idx_pcie, /* sync_trx_bd_idx */
#endif
#if MAC_AX_SDIO_SUPPORT
	r_indir_cmd52_sdio_1115e, /* r_indir_cmd52_sdio */
	_r_indir_cmd52_sdio_1115e, /* _r_indir_cmd52_sdio */
	_r_indir_cmd53_sdio_1115e, /* _r_indir_cmd53_sdio */
	r16_indir_sdio_1115e, /* r16_indir_sdio */
	r32_indir_sdio_1115e, /* r32_indir_sdio */
	w_indir_cmd52_sdio_1115e, /* w_indir_cmd52_sdio */
	w_indir_cmd53_sdio_1115e, /* w_indir_cmd53_sdio */
	ud_fs_1115e, /* ud_fs */
	sdio_pre_init_1115e, /* sdio_pre_init */
	tx_mode_cfg_sdio_1115e, /* tx_mode_cfg_sdio */
	leave_suspend_sdio_1115e, /* leave_suspend_sdio */
	get_int_latency_sdio_1115e, /* get_int_latency_sdio */
	get_clk_cnt_sdio_1115e, /* get_clk_cnt_sdio */
	set_wt_cfg_sdio_1115e, /* set_wt_cfg_sdio */
	set_clk_mon_sdio_1115e, /* set_clk_mon_sdio */
	sdio_pwr_switch_1115e, /* sdio_pwr_switch */
	rx_agg_cfg_sdio_1115e, /* rx_agg_cfg_sdio */
	aval_page_cfg_sdio_1115e, /* aval_page_cfg_sdio */
	get_sdio_rx_req_len_1115e, /* get_sdio_rx_req_len */
#endif
#if MAC_AX_USB_SUPPORT
	usb_ep_cfg_1115e, /* USB endpoint pause release */
#endif
};

struct mac_ax_priv_ops *get_mac_1115e_priv_ops(enum mac_ax_intf intf)
{
	switch (intf) {
#if MAC_AX_SDIO_SUPPORT
	case MAC_AX_INTF_SDIO:
		mac1115e_priv_ops.pwr_on = mac_pwr_on_sdio_1115e;
		mac1115e_priv_ops.pwr_off = mac_pwr_off_sdio_1115e;
		mac1115e_priv_ops.intf_pwr_switch = sdio_pwr_switch_1115e;
		break;
#endif
#if MAC_AX_USB_SUPPORT
	case MAC_AX_INTF_USB:
		mac1115e_priv_ops.pwr_on = mac_pwr_on_usb_1115e;
		mac1115e_priv_ops.pwr_off = mac_pwr_off_usb_1115e;
		mac1115e_priv_ops.intf_pwr_switch = usb_pwr_switch_1115e;
		break;
#endif
#if MAC_AX_PCIE_SUPPORT
	case MAC_AX_INTF_PCIE:
#ifdef PHL_PLATFORM_AP
		mac1115e_priv_ops.pwr_on = mac_pwr_on_ap_pcie_1115e;
		mac1115e_priv_ops.pwr_off = mac_pwr_off_ap_pcie_1115e;
#else
		mac1115e_priv_ops.pwr_on = mac_pwr_on_nic_pcie_1115e;
		mac1115e_priv_ops.pwr_off = mac_pwr_off_nic_pcie_1115e;
#endif
		mac1115e_priv_ops.intf_pwr_switch = pcie_pwr_switch;
		break;
#endif
	default:
		return NULL;
	}

	return &mac1115e_priv_ops;
}
#endif /* #if MAC_AX_1115E_SUPPORT */
