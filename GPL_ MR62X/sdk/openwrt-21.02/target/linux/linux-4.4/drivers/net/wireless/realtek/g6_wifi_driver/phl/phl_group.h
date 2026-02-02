/******************************************************************************
 *
 * Copyright(c) 2019 - 2020 Realtek Corporation.
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
#ifndef __PHL_GROUP_H__
#define __PHL_GROUP_H__

#ifdef CONFIG_WFA_OFDMA_Logo_Test

#define MAX_SUPPORT_SU_STA_NUM 64
#define MAX_SUPPORT_MU_STA_NUM 64
#define MAX_SUPPORT_RU_STA_NUM 64

#define MIN_MU_TP_CRITERIA 0 /* TODO: */
#define MIN_SU_TP_CRITERIA 0 /* TODO: */
#define MIN_RU_TP_CRITERIA 6  /* 6 Mbps */

#define MAX_MU_GROUP_NUM 32
enum phl_grp_mu_bitmap {
	GPR_MU_GRP_HE_0 = 0,
	GPR_MU_GRP_HE_1,
	GPR_MU_GRP_HE_2,
	GPR_MU_GRP_HE_3,
	GRP_MU_GRP_HE = 15,
	GPR_MU_GRP_VHT_0 = 16,
	GPR_MU_GRP_VHT_1,
	GPR_MU_GRP_VHT_2,
	GPR_MU_GRP_VHT_3,
	GRP_MU_GRP_VHT = 31
};

#define MAX_RU_GROUP_NUM 16		// Max RU group num
#define MAX_RU_GRP_STA_NUM 64	// Max RU STA num ( MAX_RU_GROUP_NUM x MAX_DLRU_GRP_STA_NUM )

#if 0
enum phl_grp_ru_bitmap {
	GPR_RU_GRP_0 = 0,
	GPR_RU_GRP_1 = 1,
	GPR_RU_GRP_2 = 2,
	GPR_RU_GRP_3 = 3,
	//GPR_RU_GRP_MAX = 32
	GPR_RU_GRP_MAX = 16
};
#endif

struct grp_sta {
	_os_list list;
	struct rtw_phl_stainfo_t *sta_info;
	u32 grp_bitmap;
};

struct su_para {
	u32 txbf_tp_crit_min;
	u8 vht_he_min_txbf_rate;
	u8 allow_su_mu;
};
struct mu_grp_para {
	u32 mu_tp_crit_min;
	u8 allow_he_to_vht;
};

//#define MAX_DLRU_GRP_STA_NUM 8
#define MAX_DLRU_GRP_STA_NUM 4

struct dlru_grp_para {
	u8 grp_status;
	u8 dev_class;
	enum channel_width bw;
	u8 stbc;
	u8 txpwr_lv;
	u16 tx_pwr_group;
	struct rtw_phl_stainfo_t * sta[MAX_DLRU_GRP_STA_NUM];
	u8 sta_cnt;
	u32 tbl_idx;
	u32 tbl_idx_bitmap;
};

//#define MAX_ULRU_GRP_STA_NUM 8
#define MAX_ULRU_GRP_STA_NUM 4

struct trigger_group{
	u16 user1;
	u16 user2;
};

struct ulru_grp_para {
	u8 grp_status;
	u8 dev_class;
	enum channel_width bw;
	u8 stbc;
	u8 txpwr_lv;
	struct rtw_phl_stainfo_t * sta[MAX_ULRU_GRP_STA_NUM];
	u8 sta_cnt;
	u32 tbl_idx;
	u32 tbl_idx_bitmap;
};

struct phl_dlsu {
	_os_list idle_list;
	_os_list busy_list;
	_os_lock list_lock;
	u32 idle_cnt;
	struct grp_sta sta[MAX_SUPPORT_SU_STA_NUM];
	struct su_para para;
};

struct phl_dlmu {
	_os_list idle_list;
	_os_list busy_list;
	_os_lock list_lock;
	u32 idle_cnt;
	struct grp_sta sta[MAX_SUPPORT_MU_STA_NUM];
	struct mu_grp_para para;
	u8 grp_sta_num[MAX_MU_GROUP_NUM];
};

#define MAX_CAP_GRP_NUM 32

struct phl_dlru {
	_os_list idle_list;		// init 64 grp_sta sta
	_os_list busy_list;
	_os_lock list_lock;
	u32 idle_cnt;
	u32 busy_cnt;
	u8 tx_cap_grp_num[MAX_CAP_GRP_NUM];
	struct grp_sta sta[MAX_SUPPORT_RU_STA_NUM];
	struct dlru_grp_para grp[MAX_RU_GROUP_NUM];
	u32 grp_used_bmp;
	u8 grp_num;
	u32 fill_tbl_cnt;
};

struct phl_ulru {
	_os_list idle_list;
	_os_list busy_list;
	_os_lock list_lock;
	u32 idle_cnt;
	u32 busy_cnt;
	struct grp_sta sta[MAX_SUPPORT_RU_STA_NUM];
	struct ulru_grp_para grp[MAX_RU_GROUP_NUM];
	u8 rx_cap_grp_num[MAX_CAP_GRP_NUM];
	u8 grp_num;
	u32 fill_tbl_cnt;
};

struct phl_grp_obj {
	bool is_grp_in_progress;
	struct phl_queue tx_tp_queue;
	struct phl_queue rx_tp_queue;

	_os_lock grp_lock;
	/* SU TXBF */
	struct phl_dlsu su;
	/* MU - MIMO */
	struct phl_dlmu mu;
	/* DL - OFDMA */
	struct phl_dlru dlru;
	/* UL - OFDMA */
	struct phl_ulru ulru;

	/* group wrole */
	struct rtw_wifi_role_t *wrole;
};

void phl_grp_dump_info(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole);

int
rtw_phl_grp_sort_tx_performance(struct phl_info_t *phl_info,
     			   struct rtw_wifi_role_t *wrole);
int
rtw_phl_grp_sort_rx_performance(struct phl_info_t *phl_info,
     			   struct rtw_wifi_role_t *wrole);
enum rtw_phl_status
rtw_phl_group_mu(struct phl_info_t *phl_info,
	      struct rtw_wifi_role_t *wrole);

enum rtw_phl_status
rtw_phl_ru_group(struct phl_info_t *phl_info,
	      struct rtw_wifi_role_t *wrole, bool reset);

//enum rtw_phl_status
//rtw_phl_grp_watchdog_callback(struct phl_info_t *phl_info,
//			      struct rtw_wifi_role_t *wrole);

void rtw_phl_grp_bw_setting_apply(void *phl, unsigned char bw);		// Mark.CS_update

bool
phl_grp_is_in_progress(struct phl_info_t *phl_info);

enum rtw_phl_status
phl_grp_obj_init(struct phl_info_t *phl_info);

enum rtw_phl_status
phl_ru_grp_init(struct phl_info_t *phl_info);


enum rtw_phl_status
phl_grp_obj_deinit(struct phl_info_t *phl_info);

u8
phl_grp_mu_get_sta_num_by_gidx(struct phl_info_t *phl_info, u8 gidx);

enum rtw_phl_status
phl_grp_mu_get_macid_list_by_gidx(struct phl_info_t *phl_info,
			    	  struct rtw_wifi_role_t *wrole,
		   		  u8 gidx, u16 *macid_list, u8 *num);

//struct rtw_phl_stainfo_t *
//rtw_phl_grp_get_highest_txtp_sta(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole);

#endif

bool phl_is_he_rate(u16 rate);
u16 phl_cal_he_rate_level(u16 rate, u8 *idx);

#endif