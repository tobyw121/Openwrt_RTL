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

#include "init_hv_8852d.h"
#include "trx_desc_hv_8852d.h"
#include "../phy_rpt_hv.h"
#include "../phy_hv.h"
#include "../sta_sch_hv.h"
#include "../dbgpkg_hv.h"
#include "../btc_hv.h"
#include "../cmac_misc_hv.h"
#include "../fwcmd_hv.h"
#include "../gpio_hv.h"
#include "../../mac_ax/mac_8852d/gpio_8852d.h"
#include "../../mac_ax/mac_8852d/coex_8852d.h"
#include "../hw_hv.h"
#include "pwr_seq_hv_8852d.h"
#include "btc_hv_8852d.h"
#if MAC_AX_8852D_SUPPORT

static struct hv_ax_ops hv8852d_ops = {
	hv_tx_post_agg_8852d /* tx_post_desc */,
	hv_get_ppdu /* get_ppdu */,
	hv_chk_ps_dfs /* chk_ps_dfs */,
	hv_chk_ps_ppdu /*chk_ps_ppdu */,
	hv_chk_ps_ch_info /* chk_ps_ch_info */,
	hv_phy_cfg /* phy_cfg */,
	hv_sta_bmp_cfg /* sta_bmp_cfg */,
	hv_sta_len_cfg /* sta_len_cfg */,
	hv_sta_dl_rugrp_cfg /* sta_dl_rugrp_cfg */,
	hv_sta_muru_cfg /* sta_muru_cfg */,
	hv_sta_quota_cfg /* sta_quota_cfg */,
	hv_sta_link_cfg /* sta_link_cfg */,
	hv_ss_dl_rpt_cfg /* ss_dl_rpt_cfg */,
	hv_ss_ul_rpt_cfg /* ss_ul_rpt_cfg */,
	hv_ss_query_search /* ss_query_search */,
	hv_ss_rpt_path_cfg /* ss_rpt_path_cfg */,
	hv_ss_set_bsr_thold /* ss_set_bsr_thold */,
	hv_ss_dlru_search_mode /* ss_dlru_search_mode */,
	hv_ss_set_delay_tx /* ss_set_delay_tx */,
	hv_sta_dl_mutbl_cfg /* sta_dl_mutbl_cfg */,
	hv_ss_dlmu_search_mode /* ss_dlmu_search_mode */,
	hv_ss_quota_mode /* ss_quota_mode */,
	hv_get_dbg_port_info /* get_dbg_port_info */,
	hv_get_dle_dfi_info /* get_dle_dfi_info */,
	hv_ss_wmm_tbl_cfg /* ss_wmm_tbl_cfg */,
	hv_ss_wmm_sta_move /* ss_wmm_sta_move */,
	hv_ss_set_wmm_bmp /* ss_set_wmm_bmp */,
	hv_cfg_btc_dbg_port_8852d /* cfg_btc_dbg_port */,
	hv_en_rtk_mode_8852d /*en_btc_rtk_mode */,
	hv_set_ctrl_frame_cnt /*set_ctrl_frame_cnt */,
	hv_set_rx_cnt /*set_rx_cnt */,
	hv_set_freerun_cfg /* set_freerun_cfg */,
	hv_get_freerun_info /* get_freerun_info */,
	hv_set_lifetime_mg2 /* set_lifetime_mg2 */,
	hv_get_lifetime_mg2 /* get_lifetime_mg2 */,
	hv_ptn_h2c_common /* pattern common h2c */,
	hv_get_mac_err_isr /* get_mac_err_isr */,
	mac_get_gpio_status_8852d /* get_gpio_status */,
	hv_get_gpio_val /* get_gpio_val */,
	hv_get_rxd_drv_info_unit /* get_rxd_drv_info_unit */,
	hv_get_ampdu_cfg /* get_ampdu_cfg */,
	hv_get_edca_param /* get_edca_param */,
	hv_get_muedca_param /* get_muedca_param */,
	hv_get_muedca_timer /* get_muedca_timer */,
	hv_get_muedca_ctrl /* get_muedca_ctrl */,
	hv_get_ch_stat_cnt /* get_ch_stat_cnt */,
	hv_get_lifetime_cfg /* get_lifetime_cfg */,
	hv_get_hw_edcca_param /*get_edcca parm*/,
	hv_set_ofld_cfg /*set_ofld_cfg parm*/,
	hv_get_macid_pause /*get_macid_pause*/,
	hv_get_hw_sch_tx_en /*get_hw_sch_tx_en*/,
	hv_set_hw_muedca_timer /*set_hw_muedca_timer*/,
	hv_set_hw_ch_busy_cnt /*set_hw_ch_busy_cnt*/,
	hv_run_pwr_seq_8852d /* run_pwr_seq */,
	mac_read_lte_8852d /* read_lte */,
	mac_write_lte_8852d /* write_lte */,
	hv_c2h_log_test /* c2h_log_test */,
};

struct hv_ax_ops *get_hv_8852d_ops(void)
{
	return &hv8852d_ops;
}
#endif /* #if MAC_AX_8852D_SUPPORT */
