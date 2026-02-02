/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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
#ifndef __RTW_ACS_H_
#define __RTW_ACS_H_


#ifdef CONFIG_RTW_ACS

#define KEEP_PREVIOS_ACS_RESULT_INTERVAL 30000 /* ms */
#define MAX_CHANNEL 165

struct acs_result {
	enum band_type band;
	u8 channel;
	u8 clm_ratio;
	s8 noise;
	u32 rx_count;
	s8 score;
	bool overlap;
	u8 nhm_rpt[NHM_RPT_NUM];
	u32 nhm_score;
};

struct acs_parm {
	u8 num;						/* total num of scan channel */
	u8 acs_idx[MAX_ACS_INFO]; 	/* mapping to phl_acs_chnl_tbl */
};

struct acs_priv {
	struct acs_parm parm;
	struct acs_result result[MAX_ACS_INFO];
	struct acs_result dacs_result[MAX_ACS_INFO];
	u8 best_channel;
	u32 max_nhm_score;
	u8 use_nhm;
	systime last_acs_time;
	bool exist_clean_channel;
	bool overlap[MAX_CHANNEL + 1];
	u8 dacs_idx;
};

void acs_parm_init(_adapter *adapter, u8 ch_num, struct phl_scan_channel *ch);
void acs_reset_info(_adapter *adapter);
void rtw_select_clean_channel(_adapter *adapter);
void acs_change_bss_chbw(_adapter *adapter, u8 with_csa_ie, s8 csa_cnt);
bool is_acs_ban_channel(_adapter *adapter, u8 channel, u8 bw);
void acs_replace_hapd_channel(_adapter *adapter, const u8 *head, size_t head_len,
                          const u8 *bcn_ies, size_t bcn_ies_len,
                          const u8 *tail, size_t tail_len);
#ifdef CONFIG_RTW_PACS
void pacs_change_bss_chbw(_adapter *adapter);
#endif /* CONFIG_RTW_PACS */
#ifdef CONFIG_RTW_DACS
void rtw_dacs_mnt_result(_adapter *adapter);
void rtw_dacs_mnt_trigger(_adapter *adapter);
#endif /* CONFIG_RTW_DACS */
#endif /* CONFIG_RTW_ACS */

#endif
