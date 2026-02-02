/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
#ifndef _RTW_MBO_H_
#define _RTW_MBO_H_

#define MBO_CH_LIST_MAX_NUM 247
#define MBO_OCE_ELEMENT_MAX_LEN 255

#define MBO_CH_PREFER_NON_OP 0
#define MBO_CH_PREFER_NOT 1
#define MBO_CH_PREFER_OK 255

struct nb_rpt_hdr;

enum rtw_mbo_attri_type {
	MBO_AP_CAPABILITY = 1,
	NON_PREFER_CHANNEL_RPT = 2,
	CELLULAR_DATA_CAPABILITY = 3,
	ASSOCIATION_DISALLOW = 4,
	CELLULAR_DATA_CONNECT_PREFER = 5,
	TRANS_REASON_CODE = 6,
	TRANS_REJECT_REASON_CODE = 7,
	ASSOCIATION_RETRY_DELAY = 8
};

enum rtw_bss_termination_phase {
	BSS_TERMINATION_NONE = 0, 
	BSS_TERMINATION_WAIT = 1,
	BSS_TERMINATION_DUR = 2,
};

struct rtw_mbo_ch_list {
	u8 op_class;
	u8 channel;
	u8 preference;
};

struct mbo_priv {
	u8 enable;
	u8 assoc_disallow;
	u8 cellular_aware;
	struct rtw_mbo_ch_list ch_list[MBO_CH_LIST_MAX_NUM];
	u8 ch_list_num;
	u8 mbo_oce_element[MBO_OCE_ELEMENT_MAX_LEN];
	u8 mbo_oce_element_len;
	_timer bss_termination_timer;
	_timer bss_termination_dur_timer;
	u16 bss_termination_dur; // in minutes
	u8  bss_termination_phase;
	u32 bss_termination_tsf_l;
	u32 bss_termination_tsf_h;
};

void rtw_mbo_ie_handler(_adapter *padapter, struct mbo_priv *mbopriv,
						const u8 *pbuf, uint limit_len);
void rtw_ap_parse_sta_mbo_element(_adapter *padapter,
						struct sta_info *psta, u8 *ies_buf, u16 ies_len);
int rtw_mbo_check_channel_valid(_adapter *padapter, struct nb_rpt_hdr report,
								struct sta_info *sta);
void rtw_mbo_fill_non_prefer_channel_list(_adapter *padapter, struct mbo_priv *mbopriv,
											const u8 *pbuf, u8 len);
void rtw_bss_termination_timeout_handler(void *ctx);
void rtw_bss_termination_dur_timeout_handler(void *ctx);

#endif /* _RTW_MBO_H_ */

