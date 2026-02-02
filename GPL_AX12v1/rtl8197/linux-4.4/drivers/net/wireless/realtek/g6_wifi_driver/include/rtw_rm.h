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

#ifndef __RTW_RM_H_
#define __RTW_RM_H_

u8 rm_post_event_hdl(_adapter *padapter, u8 *pbuf);

#define RM_TIMER_NUM 		32
#define RM_ALL_MEAS		BIT(1)
#define RM_ID_FOR_ALL(aid)	((aid<<16)|RM_ALL_MEAS)

#define RM_CAP_ARG(x) ((u8 *)(x))[4], ((u8 *)(x))[3], ((u8 *)(x))[2], ((u8 *)(x))[1], ((u8 *)(x))[0]
#define RM_CAP_FMT "%02x %02x%02x %02x%02x"

#define DOT11_PHY_TYPE_HT 7
#define DOT11_PHY_TYPE_VHT 9
#define DOT11_PHY_TYPE_HE 14

#ifdef CONFIG_RTW_80211K
enum MEASUREMENT_RESULT{
	MEASUREMENT_UNKNOWN = 0,
	MEASUREMENT_PROCESSING = 1,
	MEASUREMENT_SUCCEED = 2,
	MEASUREMENT_INCAPABLE = 3,
	MEASUREMENT_REFUSED = 4,
	MEASUREMENT_RECEIVED = 5,
};
#endif

#define RTW_MAX_RADIO_RPT_NUM		16
#define _FRAME_BODY_SUBIE_			1

#define SET_RM_NB_RPT_BSSINFO_AP_REACH(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart), 0, 2, _val)
#define SET_RM_NB_RPT_BSSINFO_SECURITY(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart), 2, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_KEY_SCOPE(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart), 3, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_SPEC_MGMT(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart), 4, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_QOS(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart), 5, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_APSD(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart), 6, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_RM(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart), 7, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_DELAY_BA(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart) + 1, 0, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_IMMED_BA(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart) + 1, 1, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_MOBI_DOMAIN(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart) + 1, 2, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_HTC(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart) + 1, 3, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_VHT(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart) + 1, 4, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_FTM(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart) + 1, 5, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_HE(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart) + 1, 6, 1, _val)
#define SET_RM_NB_RPT_BSSINFO_RXT_RANGE_BSS(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE((_pEleStart) + 1, 7, 1, _val)

/* remember to modify rm_event_name() when adding new event */
enum RM_EV_ID {
	RM_EV_state_in,
	RM_EV_busy_timer_expire,
	RM_EV_delay_timer_expire,
	RM_EV_meas_timer_expire,
	RM_EV_retry_timer_expire,
	RM_EV_repeat_delay_expire,
	RM_EV_request_timer_expire,
	RM_EV_wait_report,
	RM_EV_start_meas,
	RM_EV_survey_done,
	RM_EV_recv_rep,
	RM_EV_cancel,
	RM_EV_state_out,
	RM_EV_max
};

struct rm_event {
	u32 rmid;
	enum RM_EV_ID evid;
	_list list;
};

#ifdef CONFIG_RTW_80211K

struct rm_clock {
	struct rm_obj *prm;
	ATOMIC_T counter;
	enum RM_EV_ID evid;
};

struct rm_priv {
	u8 enable;
	_queue ev_queue;
	_queue rm_queue;
	_timer rm_timer;

	struct rm_clock clock[RM_TIMER_NUM];
	u8 rm_en_cap_def[5];
	u8 rm_en_cap_assoc[5];

	/* rm debug */
	void *prm_sel;
};

int rtw_init_rm(_adapter *padapter);
int rtw_free_rm_priv(_adapter *padapter);

unsigned int rm_on_action(_adapter *padapter, union recv_frame *precv_frame);
void RM_IE_handler(_adapter *padapter, PNDIS_802_11_VARIABLE_IEs pIE);
void rtw_ap_parse_sta_rm_en_cap(_adapter *padapter,
	struct sta_info *psta, struct rtw_ieee802_11_elems *elems);

int rm_post_event(_adapter *padapter, u32 rmid, enum RM_EV_ID evid);
void rm_handler(_adapter *padapter, struct rm_event *pev);

u8 rm_add_nb_req(_adapter *padapter, struct sta_info *psta);

void rtw_rm_construct_self_nb_report(_adapter *padapter);
void rtw_rm_check_mgmt_tx(_adapter *padapter, const u8 *buf, u32 len);
int rtw_rm_get_sta_beacon_report(_adapter *padapter, u8 *macaddr, u8 *result_buf);
void rtw_rm_nb_rpt_expire(_adapter *padapter);

#endif /*CONFIG_RTW_80211K */
#endif /* __RTW_RM_H_ */
