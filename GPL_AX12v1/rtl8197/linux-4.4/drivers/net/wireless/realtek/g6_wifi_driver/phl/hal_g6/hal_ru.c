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
 ******************************************************************************/
#define _HAL_RU_C_
#include "hal_headers.h"

#ifdef CONFIG_WFA_OFDMA_Logo_Test
enum rtw_hal_status
rtw_hal_set_dlru_fix_tbl(void *hal, struct rtw_phl_dlru_fix_tbl *tbl)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;

	hstatus = trx_ops->set_dlru_fix_tbl(hal_info, tbl);

	return hstatus;
}

enum rtw_hal_status 
rtw_hal_set_ulru_fix_tbl(void *hal, struct rtw_phl_ulru_fix_tbl *tbl)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;

	hstatus = trx_ops->set_ulru_fix_tbl(hal_info, tbl);

	return hstatus;
}

enum rtw_hal_status
rtw_hal_ru_set_tbl_fw(void *hal, u8 tbl_cls, void *tbl)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;

	//PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s\n", __func__);
	
	if (NULL == tbl)
		return hstatus;

	switch (tbl_cls) {
		case RTW_PHL_RU_TBL_DLRU :
			hstatus = rtw_hal_bb_set_dlru_tbl(hal_info, tbl);
			break;
		case RTW_PHL_RU_TBL_DLRU_FIX :
			hstatus = rtw_hal_set_dlru_fix_tbl(hal_info, tbl);
			break;
		case RTW_PHL_RU_TBL_ULRU :
			hstatus = rtw_hal_bb_set_ulru_tbl(hal_info, tbl);
			break;
		case RTW_PHL_RU_TBL_ULRU_FIX :
			hstatus = rtw_hal_set_ulru_fix_tbl(hal_info, tbl);
			break;
		case RTW_PHL_RU_TBL_RU_STA :
			hstatus = rtw_hal_bb_set_ru_sta_info(hal_info, tbl);
			break;
		case RTW_PHL_RU_DLMACID_CFG :
			hstatus = rtw_hal_bb_set_dlmacid_cfg(hal_info, tbl);
			break;
		case RTW_PHL_RU_ULMACID_CFG :
			hstatus = rtw_hal_bb_set_ulmacid_cfg(hal_info, tbl);
			break;	
		case RTW_PHL_RU_SW_GRP_SET :
			hstatus = rtw_hal_bb_set_swgrp_set(hal_info, tbl);
			break;
		case RTW_PHL_CH_BW_UPD :
			hstatus = rtw_hal_bb_ch_bw_notif(hal_info, tbl);
			break;
		case RTW_PHL_PWRTABL_NOTIFY :
			hstatus = rtw_hal_bb_pwrtbl_notif(hal_info, tbl);
			break;
		default:
			break;
	}
	return hstatus;
}

enum rtw_phl_status
rtw_hal_handle_ofdma_log_c2h(
	void *phl_com, void *c2h_sts)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_com_t *phl_com_info= (struct rtw_phl_com_t *)phl_com;
	struct phl_info_t *phl_info = (struct phl_info_t*)phl_com_info->phl_priv;
	int i = 0;
	u16 macid, rate = 0x180;
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	struct mac_ax_fwc2h_sts *fw_c2h_sts = (struct mac_ax_fwc2h_sts *)c2h_sts;

	#if 0 // DL-OFDMA
	for (i = 0; i < fw_c2h_sts->dlrusts.user_num; i++) {
		macid = fw_c2h_sts->dlrusts.user_sts[i].macid;
		phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, macid);
		if(phl_sta){
			phl_sta->ru_stats.su_ru_ratio = fw_c2h_sts->dlrusts.user_sts[i].su_ru_ratio;
			phl_sta->ru_stats.su_fail_ratio = fw_c2h_sts->dlrusts.user_sts[i].su_fail_ratio;
			phl_sta->ru_stats.ru_fail_ratio = fw_c2h_sts->dlrusts.user_sts[i].ru_fail_ratio;
			phl_sta->ru_stats.ru_tx_agg_cnt = fw_c2h_sts->dlrusts.user_sts[i].ru_avg_agg;
			rate |= (fw_c2h_sts->dlrusts.user_sts[i].nss << 4) ;
			rate |= (fw_c2h_sts->dlrusts.user_sts[i].mcs) ;
			phl_sta->ru_stats.ru_tx_phy_rate = rate;
		}
	}
	#endif
	
	// UL-OFDMA
	for (i = 0; i < fw_c2h_sts->tfsts.user_num; i++) {
		macid = fw_c2h_sts->tfsts.tf_user_sts[i].macid;
		phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, macid);
		//printk("%s: i:%d, macid:%d\n", __func__, i, macid);
		if(phl_sta){
			phl_sta->ru_stats.rssi_m = fw_c2h_sts->tfsts.tf_user_sts[i].avg_tb_rssi;
			phl_sta->ru_stats.ru_rx_cca_miss = fw_c2h_sts->tfsts.tf_user_sts[i].cca_miss_per;
			phl_sta->ru_stats.ru_rx_error = fw_c2h_sts->tfsts.tf_user_sts[i].tb_fail_per;
			phl_sta->ru_stats.ru_rx_uph_headroom = fw_c2h_sts->tfsts.tf_user_sts[i].avg_uph;
			phl_sta->ru_stats.ru_rx_min_flag_ratio = fw_c2h_sts->tfsts.tf_user_sts[i].minflag_per;
			phl_sta->ru_stats.ru_rx_evm = fw_c2h_sts->tfsts.tf_user_sts[i].avg_tb_evm;
			phl_sta->ru_stats.ru_rx_tf_cnt = fw_c2h_sts->tfsts.tf_user_sts[i].tf_num;
			phl_sta->ru_stats.ru_rx_bsr_len = fw_c2h_sts->tfsts.tf_user_sts[i].bsr_len;
		}
	}	

	return pstatus;
}

#endif
