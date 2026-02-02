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
#define _PHL_RU_C_
#include "phl_headers.h"

#ifdef CONFIG_WFA_OFDMA_Logo_Test
static u8
_phl_tbl_is_used(u32 *map, const u16 id)
{
	int map_idx = (int)id / 32;

	if (map[map_idx] & BIT(id % 32))
		return true;
	else
		return false;
}

static void
_phl_tbl_map_set(u32 *map, const u16 id)
{
	int map_idx = (int)id / 32;
	map[map_idx] |=  BIT(id % 32);
}

static void
_phl_tbl_map_clr(u32 *map, const u16 id)
{
	int map_idx = (int)id / 32;
	map[map_idx] &= ~BIT(id % 32);
}


/* Start of COMMON */
void _phl_ru_deinit_dlru(struct phl_info_t *phl_info)
{
	struct phl_ru_obj *ru_obj = (struct phl_ru_obj *)phl_info->ru_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_dlru_tbl *tbl = ru_obj->dlru.tbl;
	struct rtw_phl_dlru_fix_tbl *tbl_fix = ru_obj->dlru.tbl_fix;
	u8 num = ru_obj->dlru.tbl_num_sw+ru_obj->dlru.tbl_num_hw;

	if (NULL != ru_obj->dlru.tbl) {
		_os_kmem_free(drv_priv, tbl,
			     num * sizeof(struct rtw_phl_dlru_tbl));
	}

	if (NULL != ru_obj->dlru.tbl_fix) {
		_os_kmem_free(drv_priv, tbl_fix,
			     num * sizeof(struct rtw_phl_dlru_fix_tbl));
	}
	ru_obj->dlru.tbl_num_sw = 0;
	ru_obj->dlru.tbl_num_hw = 0;
	ru_obj->dlru.tbl_fix = NULL;
	ru_obj->dlru.tbl = NULL;

}

enum rtw_phl_status _phl_ru_init_dlru(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_ru_obj *ru_obj = (struct phl_ru_obj *)phl_info->ru_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct hal_spec_t *hal_spec = &phl_info->phl_com->hal_spec;
	u8 num = hal_spec->fw_dlru_grp_num + hal_spec->hw_dlru_grp_num;	/* 32 + 16 */
	u8 num_fix;
	struct rtw_phl_dlru_tbl *tbl;
	struct rtw_phl_dlru_fix_tbl *tbl_fix;
	u8 i = 0;

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
		  "%s allocate %d DURU Tables\n",
		  __func__, num);

	do {
		if(0 == num)
			break;
		ru_obj->dlru.tbl = _os_kmem_alloc(drv_priv,
				num * sizeof(struct rtw_phl_dlru_tbl));
		if (NULL == ru_obj->dlru.tbl)
			break;
		ru_obj->dlru.tbl_num_sw = hal_spec->fw_dlru_grp_num;
		ru_obj->dlru.tbl_num_hw = hal_spec->hw_dlru_grp_num;
		num_fix = (ru_obj->dlru.tbl_num_sw > ru_obj->dlru.tbl_num_hw) ?
			   ru_obj->dlru.tbl_num_sw : ru_obj->dlru.tbl_num_hw;
		ru_obj->dlru.tbl_fix = _os_kmem_alloc(drv_priv,
				num_fix * sizeof(struct rtw_phl_dlru_fix_tbl));
		if (NULL == ru_obj->dlru.tbl_fix)
			break;

		for (i = 0; i < PHL_MAX_DLRU_TBL_BMP_SZ; i++)
			ru_obj->dlru.tbl_used_bmp[i] = 0;

		for (i = 0; i < num; i++) {
			tbl = &ru_obj->dlru.tbl[i];
			tbl->tbl_hdr.idx = i;
			/* idx : 0 ~ (PHL_MAX_DLRU_TBL_SW_NUM-1) : SW Table */
			/* idx : PHL_MAX_DLRU_TBL_SW_NUM ~ (PHL_MAX_DLRU_TBL_NUM-1) : HW Table */
			tbl->tbl_hdr.type = (i >= ru_obj->dlru.tbl_num_sw) ?
					RTW_PHL_RU_TBL_HW : RTW_PHL_RU_TBL_SW;
			tbl->tbl_hdr.is_upd_to_fw = 0;
		}
		for (i = 0; i < num_fix; i++) {
			tbl_fix = &ru_obj->dlru.tbl_fix[i];
			tbl_fix->tbl_hdr.idx = i;
			tbl_fix->tbl_hdr.is_upd_to_fw = 0;
			tbl_fix->tbl_hdr.type = RTW_PHL_RU_TBL_MAX;
		}

		pstatus = RTW_PHL_STATUS_SUCCESS;
	} while (0);

	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "init dl ru table fail \n");
		 _phl_ru_deinit_dlru(phl_info);
	}
	return pstatus;
}


void _phl_ru_deinit_ulru(struct phl_info_t *phl_info)
{
	struct phl_ru_obj *ru_obj = (struct phl_ru_obj *)phl_info->ru_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_ulru_tbl *tbl = ru_obj->ulru.tbl;
	struct rtw_phl_ulru_fix_tbl *tbl_fix = ru_obj->ulru.tbl_fix;
	u8 num = ru_obj->ulru.tbl_num_sw +  ru_obj->ulru.tbl_num_hw;

	if (NULL != ru_obj->ulru.tbl) {
		_os_kmem_free(drv_priv, tbl,
			     num * sizeof(struct rtw_phl_ulru_tbl));
	}

	if (NULL != ru_obj->ulru.tbl_fix) {
		_os_kmem_free(drv_priv, tbl_fix,
			     num * sizeof(struct rtw_phl_ulru_fix_tbl));
	}
	ru_obj->ulru.tbl_num_sw = 0;
	ru_obj->ulru.tbl_num_hw = 0;
	ru_obj->ulru.tbl_fix = NULL;
	ru_obj->ulru.tbl = NULL;

}

enum rtw_phl_status _phl_ru_init_ulru(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_ru_obj *ru_obj = (struct phl_ru_obj *)phl_info->ru_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct hal_spec_t *hal_spec = &phl_info->phl_com->hal_spec;
	u8 num = hal_spec->fw_ulru_grp_num + hal_spec->hw_ulru_grp_num;
	u8 num_fix = 0;
	struct rtw_phl_ulru_tbl *tbl;
	struct rtw_phl_ulru_fix_tbl *tbl_fix;
	u8 i = 0;

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
		  "%s allocate %d DURU Tables\n",
		  __func__, num);

	do {
		if(0 == num)
			break;
		ru_obj->ulru.tbl = _os_kmem_alloc(drv_priv,
				num * sizeof(struct rtw_phl_ulru_tbl));
		if (NULL == ru_obj->ulru.tbl)
			break;
		ru_obj->ulru.tbl_num_sw = hal_spec->fw_ulru_grp_num;
		ru_obj->ulru.tbl_num_hw = hal_spec->hw_ulru_grp_num;
		num_fix = (ru_obj->ulru.tbl_num_sw > ru_obj->ulru.tbl_num_hw) ?
			   ru_obj->ulru.tbl_num_sw : ru_obj->ulru.tbl_num_hw;

		ru_obj->ulru.tbl_fix = _os_kmem_alloc(drv_priv,
				num_fix * sizeof(struct rtw_phl_ulru_fix_tbl));
		if (NULL == ru_obj->ulru.tbl_fix)
			break;

		for (i = 0; i < PHL_MAX_ULRU_TBL_BMP_SZ; i++)
			ru_obj->ulru.tbl_used_bmp[i] = 0;

		for (i = 0; i < num; i++) {
			tbl = &ru_obj->ulru.tbl[i];
			tbl->tbl_hdr.idx = i;
			/* idx : 0 ~ (PHL_MAX_ULRU_TBL_SW_NUM-1) : SW Table */
			/* idx : PHL_MAX_ULRU_TBL_SW_NUM ~ (PHL_MAX_ULRU_TBL_NUM-1) : HW Table */
			tbl->tbl_hdr.type = (i >= ru_obj->ulru.tbl_num_sw) ?
					RTW_PHL_RU_TBL_HW : RTW_PHL_RU_TBL_SW;
			tbl->tbl_hdr.is_upd_to_fw = 0;
		}
		for (i = 0; i < num_fix; i++) {
			tbl_fix = &ru_obj->ulru.tbl_fix[i];
			tbl_fix->tbl_hdr.idx = i;
			tbl_fix->tbl_hdr.is_upd_to_fw = 0;
			tbl_fix->tbl_hdr.type = RTW_PHL_RU_TBL_MAX;
		}

		pstatus = RTW_PHL_STATUS_SUCCESS;
	} while (0);

	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "init ul ru table fail \n");
		 _phl_ru_deinit_ulru(phl_info);
	}
	return pstatus;
}


void _phl_ru_deinit_rusta(struct phl_info_t *phl_info)
{
	struct phl_ru_obj *ru_obj = (struct phl_ru_obj *)phl_info->ru_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_ru_sta_info *ru_sta = ru_obj->ru_sta;
	u8 num = ru_obj->ru_sta_num;

	if (NULL != ru_obj->ru_sta) {
		_os_mem_free(drv_priv, ru_sta,
			     num * sizeof(struct rtw_phl_ru_sta_info));
	}

	ru_obj->ru_sta_num = 0;
	ru_obj->ru_sta = NULL;
}

enum rtw_phl_status _phl_ru_init_rusta(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_ru_obj *ru_obj = (struct phl_ru_obj *)phl_info->ru_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct hal_spec_t *hal_spec = &phl_info->phl_com->hal_spec;
	u8 num = hal_spec->fw_ru_sta_num;	/*128*/
	u8 i = 0;

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
		  "%s allocate %d RU STAs\n",
		  __func__, num);

	do {
		if(0 == num)
			break;
		ru_obj->ru_sta = _os_mem_alloc(drv_priv,
				num * sizeof(struct rtw_phl_ru_sta_info));
		if (NULL == ru_obj->ru_sta)
			break;
		ru_obj->ru_sta_num = num;

		for (i = 0; i < PHL_MAX_RU_STA_BMP_SZ; i++)
			ru_obj->ru_sta_used_bmp[i] = 0;

		pstatus = RTW_PHL_STATUS_SUCCESS;
	} while (0);

	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "init ru sta fail \n");
		 _phl_ru_deinit_rusta(phl_info);
	}
	return pstatus;
}

void _phl_ru_deinit_mac_fixmode(struct phl_info_t *phl_info)
{
	struct phl_ru_obj *ru_obj = (struct phl_ru_obj *)phl_info->ru_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_mac_ax_fixmode_para *fixmode_tbl = ru_obj->fixmode.tbl;
	u8 num = PHL_MAX_FIXMODE_NUM;

	if (NULL != fixmode_tbl) {
		_os_mem_free(drv_priv, fixmode_tbl,
			     num * sizeof(struct rtw_phl_mac_ax_fixmode_para));
	}
	ru_obj->fixmode.tbl = NULL;
}

enum rtw_phl_status _phl_ru_init_mac_fixmode(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_ru_obj *ru_obj = (struct phl_ru_obj *)phl_info->ru_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	u8 i = 0;
	u8 num = PHL_MAX_FIXMODE_NUM;
	struct rtw_phl_mac_ax_fixmode_para *tbl;

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
		  "%s allocate mac_ax_fixmode_para\n",
		  __func__);

	do {
		ru_obj->fixmode.tbl = _os_mem_alloc(drv_priv,
					num * sizeof(struct rtw_phl_mac_ax_fixmode_para));
		if (NULL == ru_obj->fixmode.tbl)
			break;

		for (i = 0; i < PHL_MAX_FIXMODE_BMP_SZ; i++)
			ru_obj->fixmode.tbl_used_bmp[i] = 0;

		for (i = 0; i < num; i++) {
			tbl = &ru_obj->fixmode.tbl[i];
			tbl->rugrpid = i;
		}

		pstatus = RTW_PHL_STATUS_SUCCESS;
	} while (0);

	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "init mac_ax_fixmode_para fail \n");
		 _phl_ru_deinit_mac_fixmode(phl_info);
	}
	return pstatus;
}

enum rtw_phl_status
phl_ru_obj_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_ru_obj *ru_obj = NULL;
	void *drv_priv = phl_to_drvpriv(phl_info);
	FUNCIN();
	do {
		phl_info->ru_obj = _os_mem_alloc(drv_priv,
						 sizeof(struct phl_ru_obj));
		if (NULL == phl_info->ru_obj) {
			break;
		}
		ru_obj = phl_info->ru_obj;

		_os_spinlock_init(drv_priv, &ru_obj->ru_lock);
		ru_obj->dlru.tbl_fix = NULL;
		ru_obj->dlru.tbl = NULL;
		ru_obj->ulru.tbl_fix = NULL;
		ru_obj->ulru.tbl = NULL;
		ru_obj->ru_sta = NULL;
		if (RTW_PHL_STATUS_FAILURE == _phl_ru_init_dlru(phl_info)) {
			break;
		}
		if (RTW_PHL_STATUS_FAILURE == _phl_ru_init_ulru(phl_info)) {
			break;
		}
		if (RTW_PHL_STATUS_FAILURE == _phl_ru_init_rusta(phl_info)) {
			break;
		}
		if (RTW_PHL_STATUS_FAILURE == _phl_ru_init_mac_fixmode(phl_info)) {
			break;
		}
		pstatus = RTW_PHL_STATUS_SUCCESS;
	} while (0);

	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		phl_ru_obj_deinit(phl_info);
	}
	FUNCOUT();
	return pstatus;
}


enum rtw_phl_status
phl_ru_obj_deinit(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	do {
		if (phl_info->ru_obj == NULL)
			break;
		_phl_ru_deinit_dlru(phl_info);
		_phl_ru_deinit_ulru(phl_info);
		_phl_ru_deinit_rusta(phl_info);

		_os_spinlock_free(drv_priv, &ru_obj->ru_lock);

		_os_mem_free(phl_to_drvpriv(phl_info), phl_info->ru_obj,
			     sizeof(struct phl_ru_obj));
		phl_info->ru_obj = NULL;
	} while (0);

	return pstatus;
}


/* External APIs */
/* External APIs  : DL RU Related */
void
_phl_ru_set_dlru_tbl_default(struct rtw_phl_dlru_tbl *tbl,
			     struct rtw_phl_stainfo_t *psta)
{
	tbl->tbl_hdr.is_upd_to_fw = false;
	tbl->ppdu_bw = psta->chandef.bw;
	tbl->tx_pwr = 0x30; /*TODO:get from bb api*/
	tbl->pwr_boost_fac = 0;/*TODO:get from bb api*/
	tbl->fix_mode_flag = false;

	/* Trigger BA settings */
	tbl->tf.tf_rate = RTW_DATA_RATE_OFDM24;
	tbl->tf.tb_ppdu_bw = psta->chandef.bw;;
	tbl->tf.rate.dcm = 0;
	tbl->tf.rate.mcs = 3;
	tbl->tf.rate.ss = 0;
	tbl->tf.fix_ba = 0;
	tbl->tf.ru_psd = 20;/*TODO:get from bb api*/
	tbl->tf.rf_gain_fix = 0;/*TODO:get from bb api*/
	tbl->tf.rf_gain_idx = 0;/*TODO:get from bb api*/
	tbl->tf.gi_ltf = RTW_GILTF_2XHE16;
	tbl->tf.doppler = 0;
	tbl->tf.stbc = 0;
	tbl->tf.sta_coding = 0;
	tbl->tf.tb_t_pe_nom = 2;
	tbl->tf.pr20_bw_en = 0;
	tbl->tf.ma_type = 0;
}

void
_phl_ru_set_dlru_tbl_fix_default(struct rtw_phl_dlru_fix_tbl *tbl,
				 enum channel_width tb_ppdu_bw,
			         struct rtw_phl_stainfo_t *psta)
{
	u8 i = 0;
	tbl->tbl_hdr.is_upd_to_fw = 0;
	tbl->ppdu_bw = tb_ppdu_bw;
	tbl->min_sta_num = 2;
	tbl->max_sta_num = 4;
	tbl->fixru_flag = true;
	tbl->rupos_csht_flag = 1;
	tbl->ru_swp_flg = 1;
	tbl->gi_ltf = RTW_GILTF_2XHE16; //RTW_GILTF_LGI_4XHE32
	tbl->ma_type = 0;
	tbl->stbc = 0;
	tbl->doppler = 0;
	for (i = 0; i < RTW_PHL_MAX_RU_STA_NUM; i++) {
		tbl->sta[i].mac_id = 0xFF;
		tbl->sta[i].fix_pwr_fac = false;
		tbl->sta[i].fix_txbf = true;
		tbl->sta[i].fix_coding = true;;
		tbl->sta[i].fix_rate = true;
		tbl->sta[i].rate.ss = 0;
		tbl->sta[i].rate.dcm = 0;
		tbl->sta[i].rate.mcs = 9;
		tbl->sta[i].pwr_boost_fac = 0;/*TODO:get from bb api*/
		tbl->sta[i].coding = 0;
		tbl->sta[i].txbf = 0;
	}
	if (CHANNEL_WIDTH_80 == tb_ppdu_bw) {
		tbl->sta[0].ru_pos[0] = RTW_HE_RU484_1; /*2STA*/
		tbl->sta[0].ru_pos[1] = RTW_HE_RU484_1; /*3STA*/
		tbl->sta[0].ru_pos[2] = RTW_HE_RU242_1; /*4STA*/

		tbl->sta[1].ru_pos[0] = RTW_HE_RU484_2; /*2STA*/
		tbl->sta[1].ru_pos[1] = RTW_HE_RU242_3; /*3STA*/
		tbl->sta[1].ru_pos[2] = RTW_HE_RU242_2; /*4STA*/

		tbl->sta[2].ru_pos[0] = RTW_HE_RU484_2; /*2STA*/
		tbl->sta[2].ru_pos[1] = RTW_HE_RU242_4; /*3STA*/
		tbl->sta[2].ru_pos[2] = RTW_HE_RU242_3; /*4STA*/

		tbl->sta[3].ru_pos[0] = RTW_HE_RU484_2; /*2STA*/
		tbl->sta[3].ru_pos[1] = RTW_HE_RU242_4; /*3STA*/
		tbl->sta[3].ru_pos[2] = RTW_HE_RU242_4; /*4STA*/
	} else if (CHANNEL_WIDTH_40 == tb_ppdu_bw) {
		tbl->sta[0].ru_pos[0] = RTW_HE_RU242_1; /*2STA*/
		tbl->sta[0].ru_pos[1] = RTW_HE_RU242_1; /*3STA*/
		tbl->sta[0].ru_pos[2] = RTW_HE_RU106_1; /*4STA*/

		tbl->sta[1].ru_pos[0] = RTW_HE_RU242_2; /*2STA*/
		tbl->sta[1].ru_pos[1] = RTW_HE_RU106_3; /*3STA*/
		tbl->sta[1].ru_pos[2] = RTW_HE_RU106_2; /*4STA*/

		tbl->sta[2].ru_pos[0] = RTW_HE_RU242_2; /*2STA*/
		tbl->sta[2].ru_pos[1] = RTW_HE_RU106_4; /*3STA*/
		tbl->sta[2].ru_pos[2] = RTW_HE_RU106_3; /*4STA*/

		tbl->sta[3].ru_pos[0] = RTW_HE_RU242_2; /*2STA*/
		tbl->sta[3].ru_pos[1] = RTW_HE_RU106_4; /*3STA*/
		tbl->sta[3].ru_pos[2] = RTW_HE_RU106_4; /*4STA*/
	} else {
		tbl->sta[0].ru_pos[0] = RTW_HE_RU106_1; /*2STA*/
		tbl->sta[0].ru_pos[1] = RTW_HE_RU106_1; /*3STA*/
		tbl->sta[0].ru_pos[2] = RTW_HE_RU52_1; /*4STA*/

		tbl->sta[1].ru_pos[0] = RTW_HE_RU106_2; /*2STA*/
		tbl->sta[1].ru_pos[1] = RTW_HE_RU52_3; /*3STA*/
		tbl->sta[1].ru_pos[2] = RTW_HE_RU52_2; /*4STA*/

		tbl->sta[3].ru_pos[0] = RTW_HE_RU106_2; /*2STA*/
		tbl->sta[3].ru_pos[1] = RTW_HE_RU52_4; /*3STA*/
		tbl->sta[3].ru_pos[2] = RTW_HE_RU52_3; /*4STA*/

		tbl->sta[2].ru_pos[0] = RTW_HE_RU106_2; /*2STA*/
		tbl->sta[2].ru_pos[1] = RTW_HE_RU52_4; /*3STA*/
		tbl->sta[2].ru_pos[2] = RTW_HE_RU52_4; /*4STA*/
	}
}

void
_phl_ru_dl_sw_upd_ru_sta(void *phl,
			 struct rtw_phl_dlru_tbl *tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct rtw_phl_ru_sta_info *ru_sta = NULL;
	u8 i = 0;

	/* check ru sta bitmap and update to fw */
	for (i = 0; i < ru_obj->ru_sta_num; i++) {
		if (_phl_tbl_is_used(ru_obj->ru_sta_used_bmp, i)) {
			ru_sta = &ru_obj->ru_sta[i];
			if (ru_sta->dl_swgrp_bitmap & BIT(tbl->tbl_hdr.idx)) {
				ru_sta->dl_swgrp_bitmap &=
						(~BIT(tbl->tbl_hdr.idx));
				pstatus = rtw_phl_ru_set_ru_sta_fw(phl_info,
								   ru_sta);
				if (RTW_PHL_STATUS_SUCCESS != pstatus) {
					PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
					"%s update fw ru station info fail\n",
					__func__);
				}
			}
		}
	}
}

void
rtw_phl_ru_release_dlru_tbl_res(void *phl,
				struct rtw_phl_dlru_tbl *tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_dlru_para *dlru = &ru_obj->dlru;
	u8 idx = 0;

	if (RTW_PHL_RU_TBL_HW == tbl->tbl_hdr.type)
		idx = tbl->tbl_hdr.idx + dlru->tbl_num_sw;
	else
		idx = tbl->tbl_hdr.idx;

	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	if(_phl_tbl_is_used(dlru->tbl_used_bmp, idx))
		_phl_tbl_map_clr(dlru->tbl_used_bmp, idx);
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);

	/* check ru sta bitmap and update to fw */
	if (RTW_PHL_RU_TBL_SW == tbl->tbl_hdr.type) {
		_phl_ru_dl_sw_upd_ru_sta(phl_info, tbl);
	}
	/* TODO: RU-STA HW GRP update */
}

void
rtw_phl_ru_release_all_dlru_tbl_res(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_dlru_para *dlru = &ru_obj->dlru;
	u8 idx = 0;
	u8 i = 0, num = 0, num_fix = 0;
	struct rtw_phl_dlru_tbl *tbl;
	struct rtw_phl_dlru_fix_tbl *tbl_fix;

	num = ru_obj->dlru.tbl_num_sw + ru_obj->dlru.tbl_num_hw;
	num_fix = (ru_obj->dlru.tbl_num_sw > ru_obj->dlru.tbl_num_hw) ?
				   ru_obj->dlru.tbl_num_sw : ru_obj->dlru.tbl_num_hw;

	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	for (i = 0; i < PHL_MAX_DLRU_TBL_BMP_SZ; i++)
		ru_obj->dlru.tbl_used_bmp[i] = 0;

	for (i = 0; i < num; i++) {
		tbl = &ru_obj->dlru.tbl[i];
		tbl->tbl_hdr.idx = i;
		/* idx : 0 ~ (PHL_MAX_DLRU_TBL_SW_NUM-1) : SW Table */
		/* idx : PHL_MAX_DLRU_TBL_SW_NUM ~ (PHL_MAX_DLRU_TBL_NUM-1) : HW Table */
		tbl->tbl_hdr.type = (i >= ru_obj->dlru.tbl_num_sw) ?
				RTW_PHL_RU_TBL_HW : RTW_PHL_RU_TBL_SW;
		tbl->tbl_hdr.is_upd_to_fw = 0;
	}
	for (i = 0; i < num_fix; i++) {
		tbl_fix = &ru_obj->dlru.tbl_fix[i];
		tbl_fix->tbl_hdr.idx = i;
		tbl_fix->tbl_hdr.is_upd_to_fw = 0;
		tbl_fix->tbl_hdr.type = RTW_PHL_RU_TBL_MAX;
	}
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
}

void
rtw_phl_ru_release_all_ulru_tbl_res(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_ulru_para *ulru = &ru_obj->ulru;
	u8 idx = 0;
	u8 i = 0, num = 0, num_fix = 0;
	struct rtw_phl_ulru_tbl *tbl;
	struct rtw_phl_ulru_fix_tbl *tbl_fix;

	num = ru_obj->ulru.tbl_num_sw + ru_obj->ulru.tbl_num_hw;
	num_fix = (ru_obj->ulru.tbl_num_sw > ru_obj->ulru.tbl_num_hw) ?
				   ru_obj->ulru.tbl_num_sw : ru_obj->ulru.tbl_num_hw;

	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	for (i = 0; i < PHL_MAX_ULRU_TBL_BMP_SZ; i++)
		ru_obj->ulru.tbl_used_bmp[i] = 0;

	for (i = 0; i < num; i++) {
		tbl = &ru_obj->ulru.tbl[i];
		tbl->tbl_hdr.idx = i;
		/* idx : 0 ~ (PHL_MAX_DLRU_TBL_SW_NUM-1) : SW Table */
		/* idx : PHL_MAX_DLRU_TBL_SW_NUM ~ (PHL_MAX_DLRU_TBL_NUM-1) : HW Table */
		tbl->tbl_hdr.type = (i >= ru_obj->ulru.tbl_num_sw) ?
				RTW_PHL_RU_TBL_HW : RTW_PHL_RU_TBL_SW;
		tbl->tbl_hdr.is_upd_to_fw = 0;
	}
	for (i = 0; i < num_fix; i++) {
		tbl_fix = &ru_obj->ulru.tbl_fix[i];
		tbl_fix->tbl_hdr.idx = i;
		tbl_fix->tbl_hdr.is_upd_to_fw = 0;
		tbl_fix->tbl_hdr.type = RTW_PHL_RU_TBL_MAX;
	}
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
}

enum rtw_phl_status
rtw_phl_ru_query_dlru_tbl_res(void *phl,
			      struct rtw_phl_stainfo_t *psta,
			      enum rtw_phl_ru_tbl_type type,
			      bool init_tbl,
			      struct rtw_phl_dlru_tbl **tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_dlru_para *dlru = &ru_obj->dlru;
	u8 tbl_idx = 0, start = 0, end = 0;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s\n", __func__);

	if (NULL == ru_obj) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	if (NULL == dlru->tbl) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	if (NULL == psta) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "%s : primary STA is NULL\n", __func__);
		(*tbl) = NULL;
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}


	if (type == RTW_PHL_RU_TBL_SW) {
		start = 0;
		end = ru_obj->dlru.tbl_num_sw;/*32*/
	} else {
		start = ru_obj->dlru.tbl_num_sw;/*32*/
		end = ru_obj->dlru.tbl_num_sw + ru_obj->dlru.tbl_num_hw;/*32+16*/
	}
	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "dlru->tbl_used_bmp:%d, %d\n", dlru->tbl_used_bmp[0], dlru->tbl_used_bmp[1]);
	for (tbl_idx = start; tbl_idx < end ; tbl_idx++) {
		if (!_phl_tbl_is_used(dlru->tbl_used_bmp, tbl_idx)) {
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "tbl_idx:%d\n", tbl_idx);
			_phl_tbl_map_set(dlru->tbl_used_bmp, tbl_idx);
			break;
		}
	}
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);


	if (tbl_idx == end) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "Get DL RU Table Fail, return Fail and NULL\n");
		(*tbl) = NULL;
		pstatus = RTW_PHL_STATUS_FAILURE;
	} else {
		(*tbl) = &dlru->tbl[tbl_idx];
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "Get DL RU Table (Idx 0x%x) Success\n",
			  (*tbl)->tbl_hdr.idx);
		if (true == init_tbl) {
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
				  "Set Default Value for DL RU Table\n");
			_phl_ru_set_dlru_tbl_default((*tbl), psta);
		}
	}
	return pstatus;
}

enum rtw_phl_status
rtw_phl_ru_query_dlru_fix_tbl_res(void *phl,
			 	  bool init_tbl,
				  struct rtw_phl_stainfo_t *psta,
				  struct rtw_phl_dlru_tbl *tbl,
				  struct rtw_phl_dlru_fix_tbl **fix_tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_dlru_para *dlru = &ru_obj->dlru;
	u8 num_fix;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s\n", __func__);

	if (NULL == ru_obj) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}
	if (NULL == dlru->tbl_fix) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}
	if (NULL == tbl) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}


	if (NULL == psta) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "%s : primary STA is NULL\n", __func__);
		(*fix_tbl) = NULL;
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	num_fix = (ru_obj->dlru.tbl_num_sw > ru_obj->dlru.tbl_num_hw) ?
		   ru_obj->dlru.tbl_num_sw : ru_obj->dlru.tbl_num_hw;

	if (tbl->tbl_hdr.idx > num_fix) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "%s : tbl_idx is invalid for fixed mode table \n",
			   __func__);
		(*fix_tbl) = NULL;
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	(*fix_tbl) = &dlru->tbl_fix[tbl->tbl_hdr.idx];

	if ((*fix_tbl)->tbl_hdr.type != RTW_PHL_RU_TBL_MAX) {
		if((*fix_tbl)->tbl_hdr.type != tbl->tbl_hdr.type) {
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
				"Fixed Table [%d] might used by another SW/HW group\n",
				(*fix_tbl)->tbl_hdr.idx);
			/*TODO: set anther table's fixed mode flage = 0 */
		}
	} else {
		(*fix_tbl)->tbl_hdr.type = tbl->tbl_hdr.type;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
				"set fixed table [%d] type = 0x%x \n",
				(*fix_tbl)->tbl_hdr.idx, (*fix_tbl)->tbl_hdr.type);
	}

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			"Get DL RU Table (Idx 0x%x) Success\n",
			(*fix_tbl)->tbl_hdr.idx);
	if (true == init_tbl) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
				"Set Default Value for DL RU Fixed Table\n");
		_phl_ru_set_dlru_tbl_fix_default((*fix_tbl), tbl->ppdu_bw, psta);
	}

	return pstatus;
}

struct rtw_phl_dlru_tbl *
rtw_phl_ru_get_dlru_tbl_by_idx(void *phl,
			       enum rtw_phl_ru_tbl_type type,
			       u8 tbl_idx)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_dlru_para *dlru = &ru_obj->dlru;
	struct rtw_phl_dlru_tbl *ret = NULL;

	do {
		if (NULL == ru_obj) {
			break;
		}

		if (NULL == dlru->tbl) {
			break;
		}
		if ((type == RTW_PHL_RU_TBL_HW) && (tbl_idx < dlru->tbl_num_hw)) {
			tbl_idx = tbl_idx + dlru->tbl_num_sw;
		}

		if (_phl_tbl_is_used(dlru->tbl_used_bmp, tbl_idx)) {
			ret = &dlru->tbl[tbl_idx];
		}
	} while(0);

	return ret;
}


struct rtw_phl_dlru_fix_tbl *
rtw_phl_ru_get_dlru_fix_tbl_by_idx(void *phl,
				   u8 tbl_idx)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_dlru_para *dlru = &ru_obj->dlru;
	struct rtw_phl_dlru_fix_tbl *ret = NULL;
	u8 num_fix = 0;

	do {
		if (NULL == ru_obj) {
			break;
		}

		if (NULL == dlru->tbl) {
			break;
		}
		num_fix = (ru_obj->dlru.tbl_num_sw > ru_obj->dlru.tbl_num_hw) ?
		  	   ru_obj->dlru.tbl_num_sw : ru_obj->dlru.tbl_num_hw;

		if (tbl_idx >= num_fix) {
			break;
		}

		ret = &dlru->tbl_fix[tbl_idx];

	} while(0);

	return ret;
}


enum rtw_phl_status
rtw_phl_ru_set_dlru_tbl_fw(void *phl,
			   struct rtw_phl_dlru_tbl *tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s <=============\n", __func__);

	if(NULL == tbl)
		return pstatus;
	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_ru_set_tbl_fw(phl_info->hal,
					RTW_PHL_RU_TBL_DLRU, tbl)) {
	pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;
}
enum rtw_phl_status
rtw_phl_ru_set_dlru_fix_tbl_fw(void *phl,
			       struct rtw_phl_dlru_fix_tbl *tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s <=============\n", __func__);

	if(NULL == tbl)
		return pstatus;

	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_ru_set_tbl_fw(phl_info->hal,
					RTW_PHL_RU_TBL_DLRU_FIX, tbl)) {
	pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}

	return pstatus;
}


/* External APIs  : UL RU Related */
void
_phl_ru_set_ulru_tbl_default(struct rtw_phl_ulru_tbl *tbl,
			     struct rtw_phl_stainfo_t *psta)
{
	tbl->tbl_hdr.is_upd_to_fw = false;
	tbl->ppdu_bw = psta->chandef.bw;
	tbl->grp_psd_max = 0;/*TODO:get from bb api*/
	tbl->grp_psd_min = 0;/*TODO:get from bb api*/
	tbl->fix_tf_rate = true;
	tbl->rf_gain_fix = false;
	tbl->fix_mode_flags = false;
	tbl->tf_rate = RTW_DATA_RATE_OFDM24;
	tbl->rf_gain_idx = 0;/*TODO:get from bb api*/
}

void
_phl_ru_set_ulru_tbl_fix_default(struct rtw_phl_ulru_fix_tbl *tbl,
				 enum channel_width tb_ppdu_bw,
			         struct rtw_phl_stainfo_t *psta)
{
	u8 i = 0;
	tbl->tbl_hdr.is_upd_to_fw = false;
	tbl->ppdu_bw = tb_ppdu_bw;
	tbl->min_sta_num = 2;
	tbl->max_sta_num = 4;
	tbl->gi_ltf = RTW_GILTF_2XHE16;
	tbl->fixru_flag = true;
	tbl->fix_tb_t_pe_nom = true;
	tbl->tb_t_pe_nom = 2;
	tbl->stbc = 0;
	tbl->ma_type = 0;
	tbl->doppler = 0;
	for (i = 0; i < RTW_PHL_MAX_RU_STA_NUM; i++) {
		tbl->sta[i].mac_id = 0xFF;
		tbl->sta[i].fix_coding = true;
		tbl->sta[i].fix_rate = true;
		tbl->sta[i].fix_tgt_rssi = true;
		tbl->sta[i].coding = 0;
		tbl->sta[i].rate.mcs = 7;
		tbl->sta[i].rate.ss = 0;
		tbl->sta[i].rate.dcm = 0;
		tbl->sta[i].tgt_rssi[0] = psta->hal_sta->rssi_stat.rssi;
		tbl->sta[i].tgt_rssi[1] = psta->hal_sta->rssi_stat.rssi;
		tbl->sta[i].tgt_rssi[2] = psta->hal_sta->rssi_stat.rssi;
	}
	if (CHANNEL_WIDTH_80 == tb_ppdu_bw) {
		tbl->sta[0].ru_pos[0] = RTW_HE_RU484_1; /*2STA*/
		tbl->sta[0].ru_pos[1] = RTW_HE_RU484_1; /*3STA*/
		tbl->sta[0].ru_pos[2] = RTW_HE_RU242_1; /*4STA*/

		tbl->sta[1].ru_pos[0] = RTW_HE_RU484_2; /*2STA*/
		tbl->sta[1].ru_pos[1] = RTW_HE_RU242_3; /*3STA*/
		tbl->sta[1].ru_pos[2] = RTW_HE_RU242_2; /*4STA*/

		tbl->sta[2].ru_pos[0] = RTW_HE_RU484_2; /*2STA*/
		tbl->sta[2].ru_pos[1] = RTW_HE_RU242_4; /*3STA*/
		tbl->sta[2].ru_pos[2] = RTW_HE_RU242_3; /*4STA*/

		tbl->sta[3].ru_pos[0] = RTW_HE_RU484_2; /*2STA*/
		tbl->sta[3].ru_pos[1] = RTW_HE_RU242_4; /*3STA*/
		tbl->sta[3].ru_pos[2] = RTW_HE_RU242_4; /*4STA*/
	} else if (CHANNEL_WIDTH_40 == tb_ppdu_bw) {
		tbl->sta[0].ru_pos[0] = RTW_HE_RU242_1; /*2STA*/
		tbl->sta[0].ru_pos[1] = RTW_HE_RU242_1; /*3STA*/
		tbl->sta[0].ru_pos[2] = RTW_HE_RU106_1; /*4STA*/

		tbl->sta[1].ru_pos[0] = RTW_HE_RU242_2; /*2STA*/
		tbl->sta[1].ru_pos[1] = RTW_HE_RU106_3; /*3STA*/
		tbl->sta[1].ru_pos[2] = RTW_HE_RU106_2; /*4STA*/

		tbl->sta[2].ru_pos[0] = RTW_HE_RU242_2; /*2STA*/
		tbl->sta[2].ru_pos[1] = RTW_HE_RU106_4; /*3STA*/
		tbl->sta[2].ru_pos[2] = RTW_HE_RU106_3; /*4STA*/

		tbl->sta[3].ru_pos[0] = RTW_HE_RU242_2; /*2STA*/
		tbl->sta[3].ru_pos[1] = RTW_HE_RU106_4; /*3STA*/
		tbl->sta[3].ru_pos[2] = RTW_HE_RU106_4; /*4STA*/
	} else {
		tbl->sta[0].ru_pos[0] = RTW_HE_RU106_1; /*2STA*/
		tbl->sta[0].ru_pos[1] = RTW_HE_RU106_1; /*3STA*/
		tbl->sta[0].ru_pos[2] = RTW_HE_RU52_1; /*4STA*/

		tbl->sta[1].ru_pos[0] = RTW_HE_RU106_2; /*2STA*/
		tbl->sta[1].ru_pos[1] = RTW_HE_RU52_3; /*3STA*/
		tbl->sta[1].ru_pos[2] = RTW_HE_RU52_2; /*4STA*/

		tbl->sta[3].ru_pos[0] = RTW_HE_RU106_2; /*2STA*/
		tbl->sta[3].ru_pos[1] = RTW_HE_RU52_4; /*3STA*/
		tbl->sta[3].ru_pos[2] = RTW_HE_RU52_3; /*4STA*/

		tbl->sta[2].ru_pos[0] = RTW_HE_RU106_2; /*2STA*/
		tbl->sta[2].ru_pos[1] = RTW_HE_RU52_4; /*3STA*/
		tbl->sta[2].ru_pos[2] = RTW_HE_RU52_4; /*4STA*/
	}
}

void
_phl_ru_ul_sw_upd_ru_sta(void *phl,
			 struct rtw_phl_ulru_tbl *tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct rtw_phl_ru_sta_info *ru_sta = NULL;
	u8 i = 0;

	/* check ru sta bitmap and update to fw */
	for (i = 0; i < ru_obj->ru_sta_num; i++) {
		if (_phl_tbl_is_used(ru_obj->ru_sta_used_bmp, i)) {
			ru_sta = &ru_obj->ru_sta[i];
			if (ru_sta->ul_swgrp_bitmap & BIT(tbl->tbl_hdr.idx)) {
				ru_sta->ul_swgrp_bitmap &=
						(~BIT(tbl->tbl_hdr.idx));
				pstatus = rtw_phl_ru_set_ru_sta_fw(phl_info,
								   ru_sta);
				if (RTW_PHL_STATUS_SUCCESS != pstatus) {
					PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
					"%s update fw ru station info fail\n",
					__func__);
				}
			}
		}
	}
}

void
rtw_phl_ru_release_ulru_tbl_res(void *phl,
				struct rtw_phl_ulru_tbl *tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_dlru_para *ulru = &ru_obj->dlru;
	u8 idx = 0;

	if (RTW_PHL_RU_TBL_HW == tbl->tbl_hdr.type)
		idx = tbl->tbl_hdr.idx + ulru->tbl_num_sw;
	else
		idx = tbl->tbl_hdr.idx;

	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	if(_phl_tbl_is_used(ulru->tbl_used_bmp, idx))
		_phl_tbl_map_clr(ulru->tbl_used_bmp, idx);
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);

	/* check ru sta bitmap and update to fw */
	if (RTW_PHL_RU_TBL_SW == tbl->tbl_hdr.type) {
		_phl_ru_ul_sw_upd_ru_sta(phl_info, tbl);
	}
	/* TODO: RU-STA HW GRP update */

}

enum rtw_phl_status
rtw_phl_ru_query_ulru_tbl_res(void *phl,
			      struct rtw_phl_stainfo_t *psta,
			      enum rtw_phl_ru_tbl_type type,
			      bool init_tbl,
			      struct rtw_phl_ulru_tbl **tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_ulru_para *ulru = &ru_obj->ulru;
	u8 tbl_idx = 0, start = 0, end = 0;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s\n", __func__);

	if (NULL == ru_obj) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	if (NULL == ulru->tbl) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	if (NULL == psta) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "%s : primary STA is NULL\n", __func__);
		(*tbl) = NULL;
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}


	if (type == RTW_PHL_RU_TBL_SW) {
		start = 0;
		end = ru_obj->ulru.tbl_num_sw;/*32*/
	} else {
		start = ru_obj->ulru.tbl_num_sw;/*32*/
		end = ru_obj->ulru.tbl_num_sw + ru_obj->ulru.tbl_num_hw;/*32+16*/
	}
	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "ulru->tbl_used_bmp:%d, %d\n", ulru->tbl_used_bmp[0], ulru->tbl_used_bmp[1]);
	for (tbl_idx = start; tbl_idx < end ; tbl_idx++) {
		if (!_phl_tbl_is_used(ulru->tbl_used_bmp, tbl_idx)) {
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "tbl_idx:%d\n", tbl_idx);
			_phl_tbl_map_set(ulru->tbl_used_bmp, tbl_idx);
			break;
		}
	}
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);


	if (tbl_idx == end) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "Get UL RU Table Fail, return Fail and NULL\n");
		(*tbl) = NULL;
		pstatus = RTW_PHL_STATUS_FAILURE;
	} else {
		(*tbl) = &ulru->tbl[tbl_idx];
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "Get UL RU Table (Idx 0x%x) Success\n",
			  (*tbl)->tbl_hdr.idx);
		if (true == init_tbl) {
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
				  "Set Default Value for UL RU Table\n");
			_phl_ru_set_ulru_tbl_default((*tbl), psta);
		}
	}
	return pstatus;
}

enum rtw_phl_status
rtw_phl_ru_query_ulru_fix_tbl_res(void *phl,
			 	  bool init_tbl,
				  struct rtw_phl_stainfo_t *psta,
				  struct rtw_phl_ulru_tbl *tbl,
				  struct rtw_phl_ulru_fix_tbl **fix_tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_ulru_para *ulru = &ru_obj->ulru;
	u8 num_fix;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s\n", __func__);

	if (NULL == ru_obj) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}
	if (NULL == ulru->tbl_fix) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}
	if (NULL == tbl) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}


	if (NULL == psta) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "%s : primary STA is NULL\n", __func__);
		(*fix_tbl) = NULL;
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	num_fix = (ru_obj->ulru.tbl_num_sw > ru_obj->ulru.tbl_num_hw) ?
		   ru_obj->ulru.tbl_num_sw : ru_obj->ulru.tbl_num_hw;

	if (tbl->tbl_hdr.idx > num_fix) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "%s : tbl_idx is invalid for fixed mode table \n",
			   __func__);
		(*fix_tbl) = NULL;
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	(*fix_tbl) = &ulru->tbl_fix[tbl->tbl_hdr.idx];

	if ((*fix_tbl)->tbl_hdr.type != RTW_PHL_RU_TBL_MAX) {
		if((*fix_tbl)->tbl_hdr.type != tbl->tbl_hdr.type) {
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
				"Fixed Table [%d] might used by another SW/HW group\n",
				(*fix_tbl)->tbl_hdr.idx);
			/*TODO: set anther table's fixed mode flage = 0 */
		}
	} else {
		(*fix_tbl)->tbl_hdr.type = tbl->tbl_hdr.type;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
				"set fixed table [%d] type = 0x%x \n",
				(*fix_tbl)->tbl_hdr.idx, (*fix_tbl)->tbl_hdr.type);
	}

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			"Get UL RU Table (Idx 0x%x) Success\n",
			(*fix_tbl)->tbl_hdr.idx);
	if (true == init_tbl) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
				"Set Default Value for UL RU Fixed Table\n");
		_phl_ru_set_ulru_tbl_fix_default((*fix_tbl), tbl->ppdu_bw, psta);
	}

	return pstatus;
}


struct rtw_phl_ulru_tbl *
rtw_phl_ru_get_ulru_tbl_by_idx(void *phl,
			       enum rtw_phl_ru_tbl_type type,
			       u8 tbl_idx)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_ulru_para *ulru = &ru_obj->ulru;
	struct rtw_phl_ulru_tbl *ret = NULL;

	do {
		if (NULL == ru_obj) {
			break;
		}

		if (NULL == ulru->tbl) {
			break;break;
		}
		if ((type == RTW_PHL_RU_TBL_HW) && (tbl_idx < ulru->tbl_num_hw)) {
			tbl_idx = tbl_idx + ulru->tbl_num_sw;
		}

		if (_phl_tbl_is_used(ulru->tbl_used_bmp, tbl_idx)) {
			ret = &ulru->tbl[tbl_idx];
		}
	} while(0);

	return ret;
}


struct rtw_phl_ulru_fix_tbl *
rtw_phl_ru_get_ulru_fix_tbl_by_idx(void *phl,
				   u8 tbl_idx)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_ulru_para *ulru = &ru_obj->ulru;
	struct rtw_phl_ulru_fix_tbl *ret = NULL;
	u8 num_fix = 0;

	do {
		if (NULL == ru_obj) {
			break;
		}

		if (NULL == ulru->tbl) {
			break;
		}
		num_fix = (ru_obj->ulru.tbl_num_sw > ru_obj->ulru.tbl_num_hw) ?
		  	   ru_obj->ulru.tbl_num_sw : ru_obj->ulru.tbl_num_hw;

		if (tbl_idx >= num_fix) {
			break;
		}

		ret = &ulru->tbl_fix[tbl_idx];

	} while(0);

	return ret;
}

enum rtw_phl_status
rtw_phl_ru_set_ulru_tbl_fw(void *phl,
			   struct rtw_phl_ulru_tbl *tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s\n", __func__);

	if(NULL == tbl)
		return pstatus;
	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_ru_set_tbl_fw(phl_info->hal,
					RTW_PHL_RU_TBL_ULRU, tbl)) {
	pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;
}
enum rtw_phl_status
rtw_phl_ru_set_ulru_fix_tbl_fw(void *phl,
			       struct rtw_phl_ulru_fix_tbl *tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s\n", __func__);

	if(NULL == tbl)
		return pstatus;
	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_ru_set_tbl_fw(phl_info->hal,
					RTW_PHL_RU_TBL_ULRU_FIX, tbl)) {
	pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;
}



/* RU STA related */
void
_phl_ru_set_ru_sta_default(struct rtw_phl_ru_sta_info *ru_sta,
			   struct rtw_phl_stainfo_t *psta)
{
	ru_sta->tbl_hdr.is_upd_to_fw = false;
	ru_sta->tbl_hdr.idx = psta->macid;
	ru_sta->gi_ltf_18spt = 0;
	ru_sta->gi_ltf_48spt = 0;
	/* dl su */
	ru_sta->csi_info_bitmap = 0;
	ru_sta->dl_swgrp_bitmap = 0;
	ru_sta->dlru_ratetbl_ridx = psta->macid;
	ru_sta->dl_fwcqi_flag = false;
	ru_sta->dlsu_info_en = true;
	ru_sta->dlsu_gi_ltf = RTW_GILTF_2XHE08;
	ru_sta->dlsu_bw = psta->chandef.bw;
	if (1 == psta->asoc_cap.nss_tx)
		ru_sta->dlsu_rate = RTW_DATA_RATE_HE_NSS2_MCS11;
	else
		ru_sta->dlsu_rate = RTW_DATA_RATE_HE_NSS1_MCS11;
	ru_sta->dlsu_dcm = 0;
	ru_sta->dlsu_pwr = 30; /*TODO: get from bb api*/
	ru_sta->dlsu_doppler_ctrl = 0;
	ru_sta->dlsu_stbc = 0;
	ru_sta->dlsu_txbf = 0;
	ru_sta->dlsu_coding = 0;

	/* ul su */
	ru_sta->ul_swgrp_bitmap = 0;
	ru_sta->ulru_ratetbl_ridx = psta->macid;
	ru_sta->ul_fwcqi_flag = 0;
	ru_sta->ulsu_info_en = false;
	ru_sta->ulsu_gi_ltf = RTW_GILTF_2XHE08;
	ru_sta->ulsu_bw = psta->chandef.bw;
	ru_sta->ulsu_rate.ss = psta->asoc_cap.nss_tx;
	ru_sta->ulsu_rate.mcs = 11;
	ru_sta->ulsu_rate.dcm = 0;

	ru_sta->ulsu_rssi = psta->hal_sta->rssi_stat.rssi;
	ru_sta->ulsu_doppler_ctrl = 0;
	ru_sta->ulsu_coding = 0;
	ru_sta->ulsu_stbc = 0;
}


void
rtw_phl_ru_release_ru_sta_res(void *phl,
			      struct rtw_phl_ru_sta_info *ru_sta)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;


	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	if(_phl_tbl_is_used(ru_obj->ru_sta_used_bmp, ru_sta->tbl_hdr.idx))
		_phl_tbl_map_clr(ru_obj->ru_sta_used_bmp, ru_sta->tbl_hdr.idx);
	ru_sta->phl_sta_info = NULL;
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
}

void
rtw_phl_ru_release_all_ru_sta_res(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	u8 i = 0;

	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	for (i = 0; i < PHL_MAX_DLRU_TBL_BMP_SZ; i++)
		ru_obj->ru_sta_used_bmp[i] = 0;
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
}

enum rtw_phl_status
rtw_phl_ru_query_ru_sta_res(void *phl,
			    bool init_sta,
			    struct rtw_phl_stainfo_t *psta,
			    struct rtw_phl_ru_sta_info **ru_sta)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	u8 idx = 0;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s\n", __func__);

	if (NULL == ru_obj) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	if (NULL == ru_obj->ru_sta) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	if (NULL == psta) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "%s : PHL STA is NULL\n", __func__);
		(*ru_sta) = NULL;
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}
	(*ru_sta) = NULL;
	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "%s : PHL STA  macid 0x%x \n", __func__, psta->macid);
	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	for (idx = 0; idx < ru_obj->ru_sta_num ; idx++) {
		if (psta == ru_obj->ru_sta[idx].phl_sta_info) {
			(*ru_sta) = &ru_obj->ru_sta[idx];
			_phl_tbl_map_set(ru_obj->ru_sta_used_bmp, idx);
			break;
		}
	}
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	/* return the found ru sta */
	if (NULL != (*ru_sta)) {
		return pstatus;
	}

	/* Get a new one */
	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	for (idx = 0; idx < ru_obj->ru_sta_num ; idx++) {
		if (!_phl_tbl_is_used(ru_obj->ru_sta_used_bmp, idx)) {
			_phl_tbl_map_set(ru_obj->ru_sta_used_bmp, idx);
			break;
		}
	}
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);

	if (idx == ru_obj->ru_sta_num) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "Get RU STA resource Fail, return Fail and NULL\n");
		(*ru_sta) = NULL;
		pstatus = RTW_PHL_STATUS_FAILURE;
	} else {
		(*ru_sta) = &ru_obj->ru_sta[idx];
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "Get RU STA INFO (Idx 0x%x) Success\n",
			  (*ru_sta)->tbl_hdr.idx);
		(*ru_sta)->phl_sta_info = psta;
		if (true == init_sta) {
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
				  "Set Default Value for RU STA\n");
			_phl_ru_set_ru_sta_default((*ru_sta), psta);
		}
	}
	return pstatus;
}

struct rtw_phl_ru_sta_info *
rtw_phl_ru_get_ru_sta_by_idx(void *phl, u8 idx)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct rtw_phl_ru_sta_info *ret = NULL;

	do {
		if (NULL == ru_obj) {
			break;
		}

		if (NULL == ru_obj->ru_sta) {
			break;
		}
		if (idx > ru_obj->ru_sta_num) {
			break;
		}

		if (_phl_tbl_is_used(ru_obj->ru_sta_used_bmp, idx)) {
			ret = &ru_obj->ru_sta[idx];
		}
	} while(0);

	return ret;
}


enum rtw_phl_status
rtw_phl_ru_set_ru_sta_fw(void *phl,
			 struct rtw_phl_ru_sta_info *ru_sta)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s <=============\n", __func__);

	if(NULL == ru_sta)
		return pstatus;

	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_ru_set_tbl_fw(phl_info->hal,
					RTW_PHL_RU_TBL_RU_STA, ru_sta)) {
	pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}

	return pstatus;
}

enum rtw_phl_status
rtw_phl_ru_set_dlmacid_cfg(void *phl,
			   struct rtw_phl_dlmacid_cfg *cfg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s <=============\n", __func__);

	if(NULL == cfg)
		return pstatus;
	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_ru_set_tbl_fw(phl_info->hal,
					RTW_PHL_RU_DLMACID_CFG, cfg)) {
	pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;
}

enum rtw_phl_status
rtw_phl_ru_set_ulmacid_cfg(void *phl,
			   struct rtw_phl_ulmacid_set *cfg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s <=============\n", __func__);

	if(NULL == cfg)
		return pstatus;
	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_ru_set_tbl_fw(phl_info->hal,
					RTW_PHL_RU_ULMACID_CFG, cfg)) {
	pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;
}

enum rtw_phl_status
rtw_phl_set_swgrp_set(void *phl,
			       struct rtw_phl_sw_grp_set *info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s <=============\n", __func__);

	if(NULL == info)
		return pstatus;

	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_ru_set_tbl_fw(phl_info->hal,
					RTW_PHL_RU_SW_GRP_SET, info)) {
	pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;
}

enum rtw_phl_status
rtw_phl_ru_set_ch_bw(void *phl,
			       struct rtw_phl_ch_bw_notif *info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	printk("%s: \n", __func__);

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s <=============\n", __func__);

	if(NULL == info)
		return pstatus;

	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_ru_set_tbl_fw(phl_info->hal,
					RTW_PHL_CH_BW_UPD, info)) {
		pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;
}

enum rtw_phl_status
rtw_phl_set_pwr_tbl_notify(void *phl,
			       struct rtw_phl_pwrtbl_notif *info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s <=============\n", __func__);

	if(NULL == info)
		return pstatus;

	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_ru_set_tbl_fw(phl_info->hal,
					RTW_PHL_PWRTABL_NOTIFY, info)) {
		pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;
}

enum rtw_phl_status
rtw_phl_mac_set_upd_ul_fixinfo(void *phl,
			       struct rtw_phl_ax_ul_fixinfo *tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;


	if(NULL == tbl)
		return pstatus;

	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_mac_set_upd_ul_fixinfo(phl_info->hal, tbl)) {
		pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;
}

void
_phl_ru_set_fixmode_tbl_default(struct rtw_phl_mac_ax_fixmode_para *tbl){

	tbl->force_sumuru_en = 0;
	tbl->forcesu = 0;
	tbl->forcemu = 0;
	tbl->forceru = 0;
	tbl->fix_fe_su_en = 0;
	tbl->fix_fe_vhtmu_en = 0;
	tbl->fix_fe_hemu_en = 0;
	tbl->fix_fe_heru_en = 0;

	tbl->fix_fe_ul_en = 0;
	tbl->fix_frame_seq_su = 0;
	tbl->fix_frame_seq_vhtmu = 0;
	tbl->fix_frame_seq_hemu = 0;
	tbl->fix_frame_seq_heru = 0;
	tbl->fix_frame_seq_ul = 0;
	tbl->is_dlruhwgrp = 0;
	tbl->is_ulruhwgrp = 0;

	tbl->prot_type_su = 0;

	tbl->prot_type_vhtmu = 0;
	tbl->resp_type_vhtmu = 0;

	tbl->prot_type_hemu = 0;
	tbl->resp_type_hemu = 0;

	tbl->prot_type_heru = 0;
	tbl->resp_type_heru = 0;

	tbl->ul_prot_type = 0;

	tbl->mugrpid = 0;
	tbl->ulgrpid = 0;

	tbl->fix_txcmdnum_en = 0;
	tbl->force_to_one = 0;
#if 1 // Mark.CS_update
	tbl->set_ulmode = 0;
	tbl->ulmode = 0;
#endif
}

void
rtw_phl_ru_release_all_fixmode_tbl_res(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct rtw_phl_mac_ax_fixmode_para *tbl;
	u8 i = 0;
	u8 num = PHL_MAX_FIXMODE_NUM;

	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	for (i = 0; i < PHL_MAX_FIXMODE_BMP_SZ; i++)
		ru_obj->fixmode.tbl_used_bmp[i] = 0;

	for (i = 0; i < num; i++) {
		tbl = &ru_obj->fixmode.tbl[i];
		tbl->rugrpid = i;
	}
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
}

enum rtw_phl_status
rtw_phl_ru_query_mac_fix_mode_para(void *phl,
					bool init_tbl,
					struct rtw_phl_mac_ax_fixmode_para **tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_ru_obj *ru_obj = phl_info->ru_obj;
	struct phl_fixmode_para *fixmode = &ru_obj->fixmode;
	u8 tbl_idx = 0;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s\n", __func__);

	if (NULL == ru_obj) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	if (NULL == fixmode->tbl) {
		pstatus = RTW_PHL_STATUS_FAILURE;
		return pstatus;
	}

	_os_spinlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);
	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "fixmode->tbl_used_bmp:%d, %d\n", fixmode->tbl_used_bmp[0], fixmode->tbl_used_bmp[1]);
	for (tbl_idx = 0; tbl_idx < PHL_MAX_FIXMODE_NUM ; tbl_idx++) {
		if (!_phl_tbl_is_used(fixmode->tbl_used_bmp, tbl_idx)) {
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "tbl_idx:%d\n", tbl_idx);
			_phl_tbl_map_set(fixmode->tbl_used_bmp, tbl_idx);
			break;
		}
	}
	_os_spinunlock(drv_priv, &ru_obj->ru_lock, _bh, NULL);

	if (tbl_idx == PHL_MAX_FIXMODE_NUM) {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "Get Fixmode Table Fail, return Fail and NULL\n");
		(*tbl) = NULL;
		pstatus = RTW_PHL_STATUS_FAILURE;
	} else {
		(*tbl) = &fixmode->tbl[tbl_idx];
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
			  "Get Fixmode Table (rugrpid 0x%x) Success\n",
			  (*tbl)->rugrpid);
		if (true == init_tbl) {
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_,
				  "Set Default Value for DL RU Table\n");
			_phl_ru_set_fixmode_tbl_default((*tbl));
		}
	}
	return pstatus;
}

enum rtw_phl_status
rtw_phl_mac_set_fixmode_mib(void *phl,
			       struct rtw_phl_mac_ax_fixmode_para *tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s <=============\n", __func__);

	if(NULL == tbl)
		return pstatus;
	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_mac_set_fixmode_mib(phl_info->hal, tbl)) {
	pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;
}

enum rtw_phl_status
rtw_phl_mac_set_dl_grp_info(void *phl,
			       struct rtw_phl_mac_ss_dl_grp_upd *info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s <=============\n", __func__);

	if(NULL == info)
		return pstatus;

	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_mac_set_dl_grp_info(phl_info->hal, info)) {
		pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;

}

enum rtw_phl_status
rtw_phl_mac_set_ul_grp_info(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s <=============\n", __func__);

	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_mac_set_ul_grp_info(phl_info->hal)) {
		pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;

}

enum rtw_phl_status
rtw_phl_mac_set_ru_fwc2h_en(void *phl, struct rtw_phl_mac_ax_ru_fwc2h_en *info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	printk("%s: en:%d (intvl:%d ms) \n", __func__, info->en, info->intvl_ms);
	PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s: en:%d (intvl:%d ms) \n", __func__, info->en, info->intvl_ms);

	if (RTW_HAL_STATUS_SUCCESS == rtw_hal_mac_fwc2h_en(phl_info->hal, info)) {
		pstatus = RTW_PHL_STATUS_SUCCESS;
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Success\n", __func__);
	} else {
		PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "%s Fail\n", __func__);
	}
	return pstatus;

}

int rtw_phl_cal_ru_quota(struct phl_info_t *phl_info, int macid)
{
	struct ru_grp_table *rugrptable = &phl_info->phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_stainfo_t *phl_sta;

	int phl_wd_quota;
	s32 wd_ptr, pending_ptr;
	int quota = 0;

	if(!ru_ctrl->tx_phase){
		return 0;
	}

	if(phl_info->phl_com->dev_cap.dlul_group_mode & BIT0){		// Dynamic quota
		phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, macid);
		if(phl_sta){
			wd_ptr = _os_atomic_read(drv_priv, &phl_sta->cnt_current_wd);
			pending_ptr = _os_atomic_read(drv_priv, &phl_sta->cnt_pending_wd);

			phl_wd_quota = (phl_sta->grp_info.is_dlru_sta || phl_sta->grp_info.is_mu_sta) ?
								phl_sta->phl_muru_wd_quota : phl_info->su_wd_quota;

			// TODO: need calculate cnt_current_wd / cnt_pending_wd of su sta

			if(phl_sta->cnt_hold_times > ru_ctrl->phl_wd_hold_cnt_thd){
				quota = ru_ctrl->phl_wd_hi_thd;
				phl_sta->cnt_hold_times = 0;
				phl_sta->cnt_wd_force_out++;
			}else if ((wd_ptr + pending_ptr) >= phl_wd_quota){
				phl_sta->cnt_wd_over_quota++;
				phl_sta->cnt_hold_times++;
				quota = 0;
			}else{
				phl_sta->cnt_wd_normal_out++;
				quota = phl_wd_quota - (wd_ptr + pending_ptr);
				phl_sta->cnt_hold_times = 0;
			}
		}
	}
	else {	// fix quota
		// TODO: Need to consider macid == 0 case?
		phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, macid);
		if(phl_sta){
			wd_ptr = _os_atomic_read(drv_priv, &phl_sta->cnt_current_wd);
			pending_ptr = _os_atomic_read(drv_priv, &phl_sta->cnt_pending_wd);

			if(phl_sta->cnt_hold_times > ru_ctrl->phl_wd_hold_cnt_thd){
				quota = ru_ctrl->phl_wd_hi_thd;
				phl_sta->cnt_hold_times = 0;
				phl_sta->cnt_wd_force_out++;
			}else if ((wd_ptr + pending_ptr) >= ru_ctrl->phl_wd_quota[macid]){
				phl_sta->cnt_wd_over_quota++;
				phl_sta->cnt_hold_times++;
				quota = 0;
			}else{
				phl_sta->cnt_wd_normal_out++;
				quota = ru_ctrl->phl_wd_quota[macid] - (wd_ptr + pending_ptr);
				phl_sta->cnt_hold_times = 0;
			}
		}
	}

	return quota;
}

void rtw_phl_ru_cal_he_su_tx_info(struct rtw_phl_stainfo_t *phl_sta, u16 ra_rate)
{

	int i=0, tmp =0;
	u8 idx=0;
	struct rtw_stats *sta_stats = NULL;
	u32 rate;

	sta_stats = &phl_sta->stats;

	sta_stats->check_intvl++;

	if(phl_is_he_rate(ra_rate)){
		rate = phl_cal_he_rate_level(ra_rate, &idx);
		if(idx >= 4)
			printk("%s: wrong tx rate idx", __func__);
		else{
			sta_stats->HE_tx_cnt[idx]++;
			sta_stats->HE_tx_rate[idx] += rate;
		}
	}

	phl_sta->cur_tx_data_rate = ra_rate;

	if(sta_stats->check_intvl >=8){
		for (i=0; i<4; i++) {
			if(tmp < sta_stats->HE_tx_cnt[i]){
				tmp = sta_stats->HE_tx_cnt[i];
				idx = i;
			}
		}

		if (sta_stats->HE_tx_cnt[idx] > 0) {
			rate =
				(sta_stats->HE_tx_rate[idx] / sta_stats->HE_tx_cnt[idx]);

			rate += (RTW_DATA_RATE_HE_NSS1_MCS0 + idx*0x10);

			sta_stats->average_HE_tx_rate_new = rate;
		}

		for (i=0; i<4; i++) {
			sta_stats->HE_tx_rate[i] = 0;
			sta_stats->HE_tx_cnt[i] = 0;
		}

		sta_stats->check_intvl = 0;
	}

}

void rtw_phl_ru_cal_he_su_rx_info(struct rtw_stats *sta_stats)
{

	int i=0, tmp =0, idx=0;
	u32 rate;

	/* cal avg HE SU rssi */
	if(sta_stats->HE_rx_rssi_cnt > 0){
		sta_stats->average_HE_rx_rssi =
			   sta_stats->HE_rx_rssi / sta_stats->HE_rx_rssi_cnt;
		sta_stats->HE_rx_rssi = 0;
		sta_stats->HE_rx_rssi_cnt = 0;
	}

	for(i=0; i<4; i++){
		if(tmp < sta_stats->HE_rx_cnt[i]){
			tmp = sta_stats->HE_rx_cnt[i];
			idx = i;
		}
	}

	/* cal avg HE SU rx rate & ss */
	if(sta_stats->HE_rx_cnt[idx] > 0){
		rate =
			(sta_stats->HE_rx_rate[idx] / sta_stats->HE_rx_cnt[idx]);

		rate += (RTW_DATA_RATE_HE_NSS1_MCS0 + idx*0x10);

		sta_stats->average_HE_rx_rate = rate;
		sta_stats->HE_rx_avg_mcs = rate & 0xf;
		sta_stats->HE_rx_avg_ss = RTW_GET_RATE_NSS(rate);
	}

	for(i=0; i<4; i++){
		sta_stats->HE_rx_rate[i] = 0;
		sta_stats->HE_rx_cnt[i] = 0;
	}

	sta_stats->ampdu_cnt_per_sec = sta_stats->ampdu_cnt;

	if(sta_stats->ampdu_cnt_per_sec > 0){
		/* cal avg HE SU ampdu agg cnt */
		sta_stats->avg_ampdu_agg =
			(sta_stats->ampdu_agg_total / sta_stats->ampdu_cnt_per_sec);

		/* cal avg HE SU ampdu size */
		sta_stats->avg_ampdu_size =
			(sta_stats->ampdu_size_total / sta_stats->ampdu_cnt_per_sec);
	}
	else {
		sta_stats->avg_ampdu_agg = 0;
		sta_stats->avg_ampdu_size = 0;
	}

	sta_stats->ampdu_cnt = 0;
	sta_stats->ampdu_agg_total = 0;
	sta_stats->ampdu_size_total = 0;
}

void rtw_phl_ru_cal_he_tb_info(struct phl_info_t *phl_info, struct rtw_stats *sta_stats)
{
	int i=0, tmp =0, idx=0;
	u32 rate;

	for(i=0; i<4; i++){
		if(tmp < sta_stats->HE_TB_stat.HE_TB_rate_cnt_per_sec[i]){
			tmp = sta_stats->HE_TB_stat.HE_TB_rate_cnt_per_sec[i];
			idx = i;
		}
	}

	/* cal avg HE TB rate */
	if(sta_stats->HE_TB_stat.HE_TB_rate_cnt_per_sec[idx] > 0){
		rate =
			(sta_stats->HE_TB_stat.HE_TB_rx_rate[idx] / sta_stats->HE_TB_stat.HE_TB_rate_cnt_per_sec[idx]);

		rate += (RTW_DATA_RATE_HE_NSS1_MCS0 + idx*0x10);
		sta_stats->HE_TB_stat.HE_TB_avg_rx_rate = rate;
	}

	for(i=0; i<4; i++){
		sta_stats->HE_TB_stat.HE_TB_rate_cnt_per_sec[i] = 0;
		sta_stats->HE_TB_stat.HE_TB_rx_rate[i] = 0;
	}

	sta_stats->HE_TB_stat.ampdu_cnt_per_sec = sta_stats->HE_TB_stat.ampdu_cnt_2;

	if (sta_stats->HE_TB_stat.ampdu_cnt_per_sec > 0) {

		/* cal avg HE TB ampdu agg cnt */
		sta_stats->HE_TB_stat.avg_ampdu_agg_2 =
			(sta_stats->HE_TB_stat.ampdu_agg_total_2 / sta_stats->HE_TB_stat.ampdu_cnt_per_sec);

		/*  DBG
		if(sta_stats->macid == 1 || sta_stats->macid == 2)
			PHL_TRACE(COMP_PHL_GROUP, _PHL_ALWAYS_, "macid:%d, avg_ampdu_agg_2:%d, ampdu_agg_total_2:%d, ampdu_cnt_per_sec:%d\n", sta_stats->macid,
				sta_stats->HE_TB_stat.avg_ampdu_agg_2, sta_stats->HE_TB_stat.ampdu_agg_total_2,
				sta_stats->HE_TB_stat.ampdu_cnt_per_sec);
		*/

		/* cal avg HE TB ampdu size */
		sta_stats->HE_TB_stat.avg_ampdu_size_2 =
			(sta_stats->HE_TB_stat.ampdu_size_total_2 / sta_stats->HE_TB_stat.ampdu_cnt_per_sec);

		/* cal avg HE TB mpdu size (unit: 64Bytes) */
		if (sta_stats->HE_TB_stat.avg_ampdu_agg_2 > 0)
			sta_stats->HE_TB_stat.avg_mpdu_size_2 = (u8)(sta_stats->HE_TB_stat.avg_ampdu_size_2 / sta_stats->HE_TB_stat.avg_ampdu_agg_2)/64;

		/* cal avg zero padding size */
		sta_stats->HE_TB_stat.avg_zero_padding_2 =
			//(stats->HE_TB_stat.zero_padding_total_2 / stats->HE_TB_stat.ampdu_cnt_per_sec);
			do_div(sta_stats->HE_TB_stat.zero_padding_total_2, sta_stats->HE_TB_stat.ampdu_cnt_per_sec);
	}
	else {
		sta_stats->HE_TB_stat.avg_ampdu_agg_2 = 0;
		sta_stats->HE_TB_stat.avg_ampdu_size_2 = 0;
		sta_stats->HE_TB_stat.avg_mpdu_size_2 = 0;
		sta_stats->HE_TB_stat.avg_zero_padding_2 = 0;
	}

	sta_stats->HE_TB_stat.ampdu_cnt_2 = 0;
	sta_stats->HE_TB_stat.ampdu_agg_total_2 = 0;
	sta_stats->HE_TB_stat.ampdu_size_total_2 = 0;
	sta_stats->HE_TB_stat.zero_padding_total_2 = 0;


	sta_stats->HE_TB_stat.crc32_ratio = sta_stats->HE_TB_stat.HE_TB_rx_cnt ? ((sta_stats->HE_TB_stat.crc32_cnt * 100)/sta_stats->HE_TB_stat.HE_TB_rx_cnt): 0;
	sta_stats->HE_TB_stat.HE_TB_rx_cnt = 0;
	sta_stats->HE_TB_stat.crc32_cnt = 0;

	#if 0  /* TBD */
	switch (phl_sta->macid) {
		case 1:
			rtw_phl_write8(phl_info, 0xc080, sta_stats->HE_TB_stat.avg_mpdu_size_2);
			break;
		case 2:
			rtw_phl_write8(phl_info, 0xc081, sta_stats->HE_TB_stat.avg_mpdu_size_2);
			break;
		case 3:
			rtw_phl_write8(phl_info, 0xc082, sta_stats->HE_TB_stat.avg_mpdu_size_2);
			break;
		case 4:
			rtw_phl_write8(phl_info, 0xc083, sta_stats->HE_TB_stat.avg_mpdu_size_2);
			break;
		case 5:
			rtw_phl_write8(phl_info, 0xc084, sta_stats->HE_TB_stat.avg_mpdu_size_2);
			break;
		case 6:
			rtw_phl_write8(phl_info, 0xc085, sta_stats->HE_TB_stat.avg_mpdu_size_2);
			break;
		case 7:
			rtw_phl_write8(phl_info, 0xc086, sta_stats->HE_TB_stat.avg_mpdu_size_2);
			break;
		case 8:
			rtw_phl_write8(phl_info, 0xc087, sta_stats->HE_TB_stat.avg_mpdu_size_2);
			break;
		default:
			break;
	}
	#endif

}

void rtw_phl_grp_cal_rtw_stats_info(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *gsta, *gn;
	struct rtw_phl_stainfo_t *self, *psta, *n;
	_os_spinlockfg sp_flags;
	struct rtw_stats *sta_stats = NULL;
	u16 rate;
	u16 size = 0;
	u32 val, cnt, grp_idx=0;
	int i, sta_cnt = 0;

	if (wrole == NULL)
		return;

	self = rtw_phl_get_stainfo_self(phl_info, wrole);
	_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
			       &wrole->assoc_sta_queue.queue, list) {
		if(psta == NULL)
			continue;
		if (self == psta)
			continue;
		if (psta->active == false)
			continue;

		sta_stats = &psta->stats;

		rtw_phl_ru_cal_he_su_rx_info(sta_stats);

		rtw_phl_ru_cal_he_tb_info(phl_info, sta_stats);
	}

	_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
}

void rtw_phl_reset_wd_balance_status(void *phl, struct rtw_wifi_role_t *wrole){

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	struct rtw_phl_stainfo_t *self, *psta, *n;
	self = rtw_phl_get_stainfo_self(phl_info, wrole);

	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,&wrole->assoc_sta_queue.queue, list)
	{
		//if (self == psta)
		//   continue;
		psta->cnt_wd_normal_out = 0;
		psta->cnt_wd_force_out = 0;
		psta->cnt_wd_over_quota = 0;
	}
}

void rtw_phl_dump_wd_balance_status(void *phl, void *m){

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct dvobj_priv *pobj = (struct dvobj_priv *)drv_priv;
	_adapter *adapter = dvobj_get_primary_adapter(pobj);
	struct rtw_wifi_role_t *wrole = adapter->phl_role;
	struct rtw_phl_stainfo_t *psta, *n, *phl_sta;
	_os_spinlockfg sp_flags;

	u32 wd_ptr, pending_ptr;

	RTW_PRINT_SEL(m, "======= Dump wd balance status =======\n");
	RTW_PRINT_SEL(m, "	mid[idx] cnt_cur_wd: (enq bz, enq bz fail, deq bz), pendind: (enq / deq pending)\n");

	_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
		       &wrole->assoc_sta_queue.queue, list) {
		wd_ptr = _os_atomic_read(drv_priv, &psta->cnt_current_wd);
		pending_ptr = _os_atomic_read(drv_priv, &psta->cnt_pending_wd);

		#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
		RTW_PRINT_SEL(m, "	macid[%d] cnt_cur_wd: %d (%d, %d, %d), pendind: %d (%d, %d)\n", psta->macid,
			wd_ptr, psta->wd_enq_busy, psta->wd_enq_busy_fail, psta->wd_deq_busy,
			pending_ptr, psta->wd_enq_pending, psta->wd_deq_pending);
		#else
		RTW_PRINT_SEL(m, "	macid[%d] cnt_cur_wd: %d, pendind: %d\n", psta->macid, wd_ptr, pending_ptr);
		#endif
	}
	_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);

	//RTW_PRINT_SEL(m, "\n");

	RTW_PRINT_SEL(m, "[wd flow control quota status]\n");
	_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
		       &wrole->assoc_sta_queue.queue, list) {
		if (psta->cnt_wd_normal_out != 0 || psta->cnt_wd_over_quota != 0 || psta->cnt_wd_force_out != 0)
		RTW_PRINT_SEL(m, "	mid[%d] nor_out:%d, over_quota:%d, force_out:%d\n",
			psta->macid, psta->cnt_wd_normal_out, psta->cnt_wd_over_quota, psta->cnt_wd_force_out);
	}
	_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);

}
#endif	// End of CONFIG_WFA_OFDMA_Logo_Test

#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
void rtw_phl_dump_wd_ring_ist(void *phl, u8 ring_type, u8 type){

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;

	struct rtw_wd_page_ring *wd_ring = NULL;
	u8 ch = 0;
	wd_ring = (struct rtw_wd_page_ring *)hci_info->wd_ring;

	switch (ring_type) {
		case idle_wd_page:
			//for (ch = 0; ch < hci_info->total_txch_num; ch++) {
			//	phl_dump_link_list(phl, &wd_ring[ch].idle_wd_page_list, type);
			//}
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "=== wd idle page[0] ring: cnt:%d ===\n", wd_ring[0].idle_wd_page_cnt);
			phl_dump_link_list(phl, &wd_ring[0].idle_wd_page_list, type);
			break;
		case pending_wd_page:
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "=== wd pending page[0] ring: cnt:%d ===\n", wd_ring[0].pending_wd_page_cnt);
			phl_dump_link_list(phl, &wd_ring[0].pending_wd_page_list, type);
			break;
		case busy_wd_page:
			PHL_TRACE(COMP_PHL_OFDMA, _PHL_INFO_, "=== wd busy page[0] ring: cnt:%d ===\n", wd_ring[0].busy_wd_page_cnt);
			phl_dump_link_list(phl, &wd_ring[0].busy_wd_page_list, type);
			break;
		default :
			break;
	}

}

void rtw_phl_reset_tx_status(void *phl, struct rtw_wifi_role_t *wrole){

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	struct rtw_phl_stainfo_t *self, *psta, *n;
	self = rtw_phl_get_stainfo_self(phl_info, wrole);

	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,&wrole->assoc_sta_queue.queue, list)
	{
		//if (self == psta)
		//   continue;
		psta->add_tx_ring_ok = 0;
		psta->add_tx_ring_fail = 0;

		psta->enq_wd_pending_ok = 0;
		psta->enq_wd_pending_fail = 0;

		psta->wd_enq_pending = 0;
		psta->wd_deq_pending = 0;

		psta->wd_enq_busy = 0;
		psta->wd_enq_busy_fail = 0;
		psta->wd_deq_busy = 0;
	}
}
#endif	// End of CONFIG_WFA_OFDMA_Logo_Test_Statistic

