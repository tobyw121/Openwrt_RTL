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
#include "init_8852a.h"
#include "cmac_tx_8852a.h"
#include "fwcmd_8852a.h"
#include "rrsr_8852a.h"
#include "coex_8852a.h"
#include "phy_rpt_8852a.h"
#include "tblupd_8852a.h"
#include "dbgpkg_8852a.h"
#include "efuse_8852a.h"
#include "hci_fc_8852a.h"
#include "dle_8852a.h"
#if MAC_AX_PCIE_SUPPORT
#include "_pcie_8852a.h"
#endif
#if MAC_AX_USB_SUPPORT
#include "_usb_8852a.h"
#endif
#if MAC_AX_SDIO_SUPPORT
#include "_sdio_8852a.h"
#endif

#if MAC_AX_8852A_SUPPORT

static struct mac_ax_priv_ops mac8852a_priv_ops = {
	NULL, /* pwr_off, 8852a DO NOT USE this pwr_off function */
	NULL, /* pwr_on, 8852a DO NOT USE this pwr_off function */
	NULL, /* intf_pwr_switch */
	dmac_func_en_8852a, /* dmac_func_en */
	dmac_func_pre_en_8852a, /* dmac_func_pre_en */
	mac_init_cctl_info_8852a, /* init cmac table */
	cmac_init, /*cmac module init*/
	cmac_func_en,/* cmac_func_en */
	macid_idle_ck_8852a, /* macid_idle_ck */
	stop_sch_tx_8852a, /* stop_sch_tx */
	switch_efuse_bank_8852a, /* switch_efuse_bank */
	enable_efuse_sw_pwr_cut_8852a, /* enable_efuse_sw_pwr_cut */
	disable_efuse_sw_pwr_cut_8852a, /* disable_efuse_sw_pwr_cut */
	get_h2creg_offset_8852a, /* get_h2creg_offset */
	get_c2hreg_offset_8852a, /* get_c2hreg_offset */
	ser_imr_config, /*ser_imr_config*/
	disconnect_flush_key, /* disconnect_flush_key */
	sec_info_tbl_init, /* sec_info_tbl_init */
	free_sec_info_tbl, /* free_sec_info_tbl */
	mac_wowlan_secinfo, /* mac_wowlan_secinfo */
	mac_get_rrsr_cfg_8852a, /*get RRSR related config*/
	mac_set_rrsr_cfg_8852a, /*set RRSR related config*/
	mac_get_cts_rrsr_cfg_8852a, /*get CTS RRSR related config*/
	mac_set_cts_rrsr_cfg_8852a, /*set CTS RRSR related config*/
	mac_cfg_gnt_8852a, /* cfg_ctrl_path*/
	mac_cfg_ctrl_path_8852a, /* cfg_ctrl_path */
	mac_get_gnt_8852a, /* get_gnt */
	mac_get_ctrl_path_8852a, /* get_ctrl_path */
	get_bbrpt_dle_cfg_8852a, /*get_bbrpt_dle_cfg*/
	dbg_port_sel_8852a, /*for mac debug port*/
	tx_flow_ptcl_dbg_port_8852a, /*for mac tx flow ptcl dbg*/
	tx_flow_sch_dbg_port_8852a, /*for mac tx schdueler ptcl dbg*/
	ss_stat_chk_8852a, /*for mac station scheduler check*/
	dmac_dbg_dump_8852a, /*for dmac debug dump*/
	cmac_dbg_dump_8852a, /*for cmac debug dump*/
	crit_dbg_dump_8852a, /*for system critical debug dump*/
	tx_dbg_dump_8852a, /*for tx flow debug dump*/
	coex_mac_init_8852a, /* coex_mac_init */
	set_fc_page_ctrl_reg_8852a, /* set_fc_page_ctrl_reg */
	get_fc_page_info_8852a, /* get_fc_page_info */
	set_fc_pubpg_8852a, /* set_fc_pubpg */
	get_fc_mix_info_8852a, /* get_fc_mix_info */
	set_fc_h2c_8852a, /* set_fc_h2c */
	set_fc_mix_cfg_8852a, /* set_fc_mix_cfg */
	set_fc_func_en_8852a, /* set_fc_func_en */
	dle_dfi_ctrl_8852a, /* dle_dfi_ctrl */
	dle_is_txq_empty_8852a, /* dle_is_txq_empty */
	dle_is_rxq_empty_8852a, /* dle_is_rxq_empty */
	preload_cfg_set_8852a, /* preload_cfg_set */
	preload_init_set_8852a, /* preload_init_set */
	dle_func_en_8852a, /* dle_func_en */
	dle_clk_en_8852a, /* dle_clk_en */
	dle_mix_cfg_8852a, /* dle_mix_cfg */
	wde_quota_cfg_8852a, /* wde_quota_cfg */
	ple_quota_cfg_8852a, /* ple_quota_cfg */
	chk_dle_rdy_8852a, /* chk_dle_rdy */
	is_dbg_port_not_valid_8852a, /* is_dbg_port_not_valid */
	dbg_port_sel_rst_8852a, /* dbg_port_sel_rst */
	dle_dfi_sel_8852a, /* dle_dfi_sel */
#if MAC_AX_PCIE_SUPPORT
	get_pcie_info_def_8852a, /* get_pcie_info_def */
	get_bdram_tbl_pcie_8852a, /* get_bdram_tbl_pcie */
	mio_w32_pcie_8852a, /* mio_w32_pcie */
	mio_r32_pcie_8852a, /* mio_r32_pcie */
	get_txbd_reg_pcie_8852a, /* get_txbd_reg_pcie */
	set_txbd_reg_pcie_8852a, /* set_txbd_reg_pcie */
	get_rxbd_reg_pcie_8852a, /* get_rxbd_reg_pcie */
	set_rxbd_reg_pcie_8852a, /* set_rxbd_reg_pcie */
	ltr_sw_trigger_8852a, /* ltr_sw_trigger */
	pcie_cfgspc_write_8852a, /* pcie_cfgspc_write */
	pcie_cfgspc_read_8852a, /* pcie_cfgspc_read */
	pcie_ltr_write_8852a, /* pcie_ltr_write */
	pcie_ltr_read_8852a, /* pcie_ltr_read */
	ctrl_hci_dma_en_pcie_8852a, /* ctrl_hci_dma_en_pcie */
	ctrl_trxdma_pcie_8852a, /* ctrl_trxdma_pcie */
	ctrl_wpdma_pcie_8852a, /* ctrl_wpdma_pcie */
	poll_io_idle_pcie_8852a, /* poll_io_idle_pcie */
	poll_dma_all_idle_pcie_8852a, /* poll_dma_all_idle_pcie */
	clr_idx_ch_pcie_8852a, /* clr_idx_ch_pcie */
	rst_bdram_pcie_8852a, /* rst_bdram_pcie */
	trx_mit_pcie_8852a, /* trx_mit_pcie */
	mode_op_pcie_8852a, /* mode_op_pcie */
	get_err_flag_pcie_8852a, /* get_err_flag_pcie */
	mac_auto_refclk_cal_pcie_8852a, /* mac_auto_refclk_cal_pcie */
#ifdef RTW_WKARD_GET_PROCESSOR_ID
	chk_proc_long_ldy, /* chk_proc_long_ldy_pcie */
#endif
	sync_trx_bd_idx_pcie, /* sync_trx_bd_idx */
#endif
#if MAC_AX_SDIO_SUPPORT
	r_indir_cmd52_sdio_8852a, /* r_indir_cmd52_sdio */
	_r_indir_cmd52_sdio_8852a, /* _r_indir_cmd52_sdio */
	_r_indir_cmd53_sdio_8852a, /* _r_indir_cmd53_sdio */
	r16_indir_sdio_8852a, /* r16_indir_sdio */
	r32_indir_sdio_8852a, /* r32_indir_sdio */
	w_indir_cmd52_sdio_8852a, /* w_indir_cmd52_sdio */
	w_indir_cmd53_sdio_8852a, /* w_indir_cmd53_sdio */
	ud_fs_8852a, /* ud_fs */
	sdio_pre_init_8852a, /* sdio_pre_init */
	tx_mode_cfg_sdio_8852a, /* tx_mode_cfg_sdio */
	leave_suspend_sdio_8852a, /* leave_suspend_sdio */
	get_int_latency_sdio_8852a, /* get_int_latency_sdio */
	get_clk_cnt_sdio_8852a, /* get_clk_cnt_sdio */
	set_wt_cfg_sdio_8852a, /* set_wt_cfg_sdio */
	set_clk_mon_sdio_8852a, /* set_clk_mon_sdio */
	sdio_pwr_switch_8852a, /* sdio_pwr_switch */
	rx_agg_cfg_sdio_8852a, /* rx_agg_cfg_sdio */
	aval_page_cfg_sdio_8852a, /* aval_page_cfg_sdio */
	get_sdio_rx_req_len_8852a, /* get_sdio_rx_req_len */
#endif
#if MAC_AX_USB_SUPPORT
	usb_ep_cfg_8852a, /* USB endpoint pause release */
#endif
};

struct mac_ax_priv_ops *get_mac_8852a_priv_ops(enum mac_ax_intf intf)
{
	switch (intf) {
#if MAC_AX_SDIO_SUPPORT
	case MAC_AX_INTF_SDIO:
		mac8852a_priv_ops.intf_pwr_switch = sdio_pwr_switch_8852a;
		break;
#endif
#if MAC_AX_USB_SUPPORT
	case MAC_AX_INTF_USB:
		mac8852a_priv_ops.intf_pwr_switch = usb_pwr_switch_8852a;
		break;
#endif
#if MAC_AX_PCIE_SUPPORT
	case MAC_AX_INTF_PCIE:
		mac8852a_priv_ops.intf_pwr_switch = pcie_pwr_switch;
		break;
#endif
	default:
		return NULL;
	}

	return &mac8852a_priv_ops;
}
#endif /* #if MAC_AX_8852A_SUPPORT */
