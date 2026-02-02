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
#ifndef _PHL_RU_H_
#define _PHL_RU_H_

#include "phl_types.h"
#include "phl_struct.h"

#ifdef CONFIG_WFA_OFDMA_Logo_Test
/*
#define PHL_MAX_DLRU_TBL_HW_NUM 16
#define PHL_MAX_DLRU_TBL_SW_NUM 32
#define PHL_MAX_DLRU_TBL_NUM PHL_MAX_DLRU_TBL_HW_NUM + PHL_MAX_DLRU_TBL_SW_NUM

#define PHL_MAX_ULRU_TBL_HW_NUM 15
#define PHL_MAX_ULRU_TBL_SW_NUM 32
#define PHL_MAX_ULRU_TBL_NUM PHL_MAX_ULRU_TBL_HW_NUM + PHL_MAX_ULRU_TBL_SW_NUM

#define PHL_MAX_RU_STA_NUM 128
*/
#define PHL_MAX_DLRU_TBL_BMP_SZ 2 /* unit : 32 bit*/
#define PHL_MAX_ULRU_TBL_BMP_SZ 2 /* unit : 32 bit*/
#define PHL_MAX_RU_STA_BMP_SZ 4 /* unit : 32 bit*/
#define PHL_MAX_FIXMODE_BMP_SZ 2 /* unit : 32 bit*/
#define PHL_MAX_FIXMODE_NUM 64

struct phl_dlru_para {
	u8 tbl_num_sw;
	u8 tbl_num_hw;
	struct rtw_phl_dlru_tbl *tbl;
	struct rtw_phl_dlru_fix_tbl *tbl_fix;
	u32 tbl_used_bmp[PHL_MAX_DLRU_TBL_BMP_SZ]; /* PHL_MAX_DLRU_TBL_NUM */
};

struct phl_ulru_para {
	u8 tbl_num_sw;
	u8 tbl_num_hw;
	struct rtw_phl_ulru_tbl *tbl;
	struct rtw_phl_ulru_fix_tbl *tbl_fix;
	u32 tbl_used_bmp[PHL_MAX_ULRU_TBL_BMP_SZ]; /* PHL_MAX_ULRU_TBL_NUM */
};

struct phl_fixmode_para {
	u8 tbl_num;
	struct rtw_phl_mac_ax_fixmode_para *tbl;
	u32 tbl_used_bmp[PHL_MAX_FIXMODE_BMP_SZ]; /* PHL_MAX_ULRU_TBL_NUM */
};

struct phl_ru_obj {
	_os_lock ru_lock;
	u8 ru_sta_num;
	struct rtw_phl_ru_sta_info *ru_sta;
	u32 ru_sta_used_bmp[PHL_MAX_RU_STA_BMP_SZ]; /* PHL_MAX_RU_STA_NUM */
	struct phl_dlru_para dlru;
	struct phl_ulru_para ulru;
	struct phl_fixmode_para fixmode;
};


enum rtw_phl_status
phl_ru_obj_deinit(struct phl_info_t *phl_info);

enum rtw_phl_status
phl_ru_obj_init(struct phl_info_t *phl_info);

/* export to other phl grouping module */
struct rtw_phl_dlru_fix_tbl *
rtw_phl_ru_get_dlru_fix_tbl_by_idx(void *phl, u8 tbl_idx);
struct rtw_phl_dlru_tbl *
rtw_phl_ru_get_dlru_tbl_by_idx(void *phl,
			       enum rtw_phl_ru_tbl_type type,
			       u8 tbl_idx);
struct rtw_phl_ru_sta_info *
rtw_phl_ru_get_ru_sta_by_idx(void *phl, u8 idx);

struct rtw_phl_ulru_fix_tbl *
rtw_phl_ru_get_ulru_fix_tbl_by_idx(void *phl,  u8 tbl_idx);

struct rtw_phl_ulru_tbl *
rtw_phl_ru_get_ulru_tbl_by_idx(void *phl,
			       enum rtw_phl_ru_tbl_type type,
			       u8 tbl_idx);

enum rtw_phl_status
rtw_phl_ru_query_dlru_fix_tbl_res(void *phl,
			 	  bool init_tbl,
				  struct rtw_phl_stainfo_t *psta,
				  struct rtw_phl_dlru_tbl *tbl,
				  struct rtw_phl_dlru_fix_tbl **fix_tbl);
enum rtw_phl_status
rtw_phl_ru_query_dlru_tbl_res(void *phl,
			      struct rtw_phl_stainfo_t *psta,
			      enum rtw_phl_ru_tbl_type type,
			      bool init_tbl,
			      struct rtw_phl_dlru_tbl **tbl);

enum rtw_phl_status
rtw_phl_ru_query_ru_sta_res(void *phl,
			    bool init_sta,
			    struct rtw_phl_stainfo_t *psta,
			    struct rtw_phl_ru_sta_info **ru_sta);

enum rtw_phl_status
rtw_phl_ru_query_ulru_fix_tbl_res(void *phl,
			 	  bool init_tbl,
				  struct rtw_phl_stainfo_t *psta,
				  struct rtw_phl_ulru_tbl *tbl,
				  struct rtw_phl_ulru_fix_tbl **fix_tbl);

enum rtw_phl_status
rtw_phl_ru_query_ulru_tbl_res(void *phl,
			      struct rtw_phl_stainfo_t *psta,
			      enum rtw_phl_ru_tbl_type type,
			      bool init_tbl,
			      struct rtw_phl_ulru_tbl **tbl);

void
rtw_phl_ru_release_dlru_tbl_res(void *phl, struct rtw_phl_dlru_tbl *tbl);
void
rtw_phl_ru_release_all_dlru_tbl_res(void *phl);
void
rtw_phl_ru_release_all_ulru_tbl_res(void *phl);
void
rtw_phl_ru_release_ulru_tbl_res(void *phl, struct rtw_phl_ulru_tbl *tbl);
void
rtw_phl_ru_release_ru_sta_res(void *phl, struct rtw_phl_ru_sta_info *ru_sta);
void
rtw_phl_ru_release_all_ru_sta_res(void *phl);

enum rtw_phl_status
rtw_phl_ru_set_dlru_fix_tbl_fw(void *phl,  struct rtw_phl_dlru_fix_tbl *tbl);
enum rtw_phl_status
rtw_phl_ru_set_dlru_tbl_fw(void *phl, struct rtw_phl_dlru_tbl *tbl);
enum rtw_phl_status
rtw_phl_ru_set_ru_sta_fw(void *phl, struct rtw_phl_ru_sta_info *ru_sta);
enum rtw_phl_status
rtw_phl_ru_set_ulru_fix_tbl_fw(void *phl, struct rtw_phl_ulru_fix_tbl *tbl);
enum rtw_phl_status
rtw_phl_ru_set_ulru_tbl_fw(void *phl, struct rtw_phl_ulru_tbl *tbl);

void
rtw_phl_ru_release_all_fixmode_tbl_res(void *phl);
enum rtw_phl_status
rtw_phl_ru_query_mac_fix_mode_para(void *phl, bool init_tbl,
				  struct rtw_phl_mac_ax_fixmode_para **fix_mode_para);
enum rtw_phl_status
rtw_phl_mac_set_fixmode_mib(void *phl,
			       struct rtw_phl_mac_ax_fixmode_para *tbl);

enum rtw_phl_status
rtw_phl_mac_set_ru_fwc2h_en(void *phl, struct rtw_phl_mac_ax_ru_fwc2h_en *info);

enum rtw_phl_status
rtw_phl_mac_set_dl_grp_info(void *phl,
			       struct rtw_phl_mac_ss_dl_grp_upd *info);

enum rtw_phl_status
rtw_phl_mac_set_ul_grp_info(void *phl);

enum rtw_phl_status
rtw_phl_ru_set_dlmacid_cfg(void *phl,
			   struct rtw_phl_dlmacid_cfg *cfg);

enum rtw_phl_status
rtw_phl_ru_set_ulmacid_cfg(void *phl,
			   struct rtw_phl_ulmacid_set *cfg);

enum rtw_phl_status
rtw_phl_ru_set_ch_bw(void *phl,
			       struct rtw_phl_ch_bw_notif *info);

enum rtw_phl_status
rtw_phl_set_pwr_tbl_notify(void *phl,
			       struct rtw_phl_pwrtbl_notif *info);

int rtw_phl_cal_ru_quota(struct phl_info_t *phl_info, int macid);

void rtw_phl_ru_cal_he_su_tx_info(struct rtw_phl_stainfo_t *phl_sta, u16 ra_rate);
void rtw_phl_ru_cal_he_su_rx_info(struct rtw_stats *sta_stats);
void rtw_phl_ru_cal_he_tb_info(struct phl_info_t *phl_info, struct rtw_stats *sta_stats);
void rtw_phl_grp_cal_rtw_stats_info(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole);

void rtw_phl_reset_wd_balance_status(void *phl, struct rtw_wifi_role_t *wrole);
void rtw_phl_dump_wd_balance_status(void *phl, void *m);
#endif	// End of CONFIG_WFA_OFDMA_Logo_Test

#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
void rtw_phl_reset_tx_status(void *phl, struct rtw_wifi_role_t *wrole);
void rtw_phl_dump_wd_ring_ist(void *phl, u8 ring_type, u8 type);
#endif	// End of CONFIG_WFA_OFDMA_Logo_Test_Statistic

#endif