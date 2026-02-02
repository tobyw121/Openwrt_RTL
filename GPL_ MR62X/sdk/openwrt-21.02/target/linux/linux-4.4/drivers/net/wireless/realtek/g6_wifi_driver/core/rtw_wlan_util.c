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
#define _RTW_WLAN_UTIL_C_

#include <drv_types.h>
#ifdef CONFIG_CROSSBAND_REPEATER_SUPPORT_WIFI5_WIFI6
#include <net/rtl/rtw_crossband_repeater.h>
#endif

unsigned char ARTHEROS_OUI1[] = {0x00, 0x03, 0x7f};
unsigned char ARTHEROS_OUI2[] = {0x00, 0x13, 0x74};

#define BROADCOM_OUI_NUM 3
unsigned char BROADCOM_OUI[][3] = {
	{0x00, 0x05, 0xb5},
	{0x00, 0x0a, 0xf7},
	{0x00, 0x10, 0x18}
};

unsigned char CISCO_OUI[] = {0x00, 0x40, 0x96};
unsigned char MARVELL_OUI[] = {0x00, 0x50, 0x43};
unsigned char RALINK_OUI[] = {0x00, 0x0c, 0x43};
unsigned char REALTEK_OUI[] = {0x00, 0xe0, 0x4c};
unsigned char AIRGOCAP_OUI[] = {0x00, 0x0a, 0xf5};

unsigned char REALTEK_96B_IE[] = {0x00, 0xe0, 0x4c, 0x02, 0x01, 0x20};

#define APPLE_OUI_NUM 32
unsigned char APPLE_OUI_G6[][3] = {
	{0x00, 0x17, 0xf2},
	{0x1C, 0x36, 0xBB},
	{0x30, 0x35, 0xAD},
	{0x3C, 0x2E, 0xFF},
	{0x40, 0xCB, 0xC0},
	{0x40, 0x98, 0xAD},
	{0x48, 0xA1, 0x95},
	{0x4c, 0x32, 0x75},
	{0x50, 0x32, 0x37},
	{0x6C, 0x4D, 0x73},
	{0x6C, 0x96, 0xCF},
	{0x6C, 0xAB, 0x31},
	{0x70, 0xA2, 0xB3},
	{0x7C, 0x50, 0x49},
	{0x84, 0x38, 0x35},
	{0x8c, 0x29, 0x37},
	{0x8c, 0x85, 0x90},
	{0x98, 0x01, 0xA7},
	{0x98, 0x9E, 0x63},
	{0xA8, 0xBE, 0x27},
	{0xAC, 0xBC, 0x32},
	{0xB0, 0x48, 0x1A},
	{0xB8, 0x63, 0x4D},
	{0xC4, 0x84, 0x66},
	{0xC4, 0xB3, 0x01},
	{0xD0, 0xA6, 0x37},
	{0xD4, 0x61, 0x9D},
	{0xDC, 0xA9, 0x04},
	{0xE4, 0x2B, 0x34},
	{0xF0, 0x76, 0x6F},
	{0x9C, 0xF3, 0x87},
	{0x8C, 0x85, 0x90},
};

#define VERIWAVE_OUI_NUM 10
unsigned char VERIWAVE_OUI_G6[][3]= {
	{0x00,0x11,0xC0},
	{0x00,0x21,0xC0},
	{0x00,0x31,0xC0},
	{0x00,0x41,0xC0},
	{0x00,0x11,0xDD},
	{0x00,0x21,0xDD},
	{0x00,0x31,0xDD},
	{0x00,0x41,0xDD},
	{0x00,0x20,0x98},
	{0x00,0x14,0x94}
};

#define SPIRENT_OUI_NUM 3
unsigned char SPIRENT_OUI_G6[][3]= {
	{0x00,0x10,0x94},
	{0x00,0x60,0xF3},
	{0x00,0x00,0x00}
};

#define OCTOSCOPE_OUI_NUM 4
unsigned char OCTOSCOPE_OUI[][3]= {
	{0x2c,0x27,0x9e},
	{0x34,0xcf,0xf6},
	{0xa0,0x36,0x9f},
	{0xac,0x67,0x5d}
};


extern unsigned char RTW_WPA_OUI[];
extern unsigned char WPA_TKIP_CIPHER[4];
extern unsigned char RSN_TKIP_CIPHER[4];

#define R2T_PHY_DELAY	(0)

/* #define WAIT_FOR_BCN_TO_MIN	(3000) */
#define WAIT_FOR_BCN_TO_MIN	(6000)
#define WAIT_FOR_BCN_TO_MAX	(20000)

static u8 rtw_basic_rate_cck[4] = {
	IEEE80211_CCK_RATE_1MB | IEEE80211_BASIC_RATE_MASK, IEEE80211_CCK_RATE_2MB | IEEE80211_BASIC_RATE_MASK,
	IEEE80211_CCK_RATE_5MB | IEEE80211_BASIC_RATE_MASK, IEEE80211_CCK_RATE_11MB | IEEE80211_BASIC_RATE_MASK
};

static u8 rtw_basic_rate_ofdm[3] = {
	IEEE80211_OFDM_RATE_6MB | IEEE80211_BASIC_RATE_MASK, IEEE80211_OFDM_RATE_12MB | IEEE80211_BASIC_RATE_MASK,
	IEEE80211_OFDM_RATE_24MB | IEEE80211_BASIC_RATE_MASK
};

static u8 rtw_basic_rate_mix[7] = {
	IEEE80211_CCK_RATE_1MB | IEEE80211_BASIC_RATE_MASK, IEEE80211_CCK_RATE_2MB | IEEE80211_BASIC_RATE_MASK,
	IEEE80211_CCK_RATE_5MB | IEEE80211_BASIC_RATE_MASK, IEEE80211_CCK_RATE_11MB | IEEE80211_BASIC_RATE_MASK,
	IEEE80211_OFDM_RATE_6MB | IEEE80211_BASIC_RATE_MASK, IEEE80211_OFDM_RATE_12MB | IEEE80211_BASIC_RATE_MASK,
	IEEE80211_OFDM_RATE_24MB | IEEE80211_BASIC_RATE_MASK
};

typedef struct _OP_CLASS_ {
	u8        op_class;
	u8        band;     /* 0: 2g, 1: 5g*/
} OP_CLASS;

/*Global Operating Classes*/
static const OP_CLASS GLOBAL_OP_CLASS[] = {
	{ 81, BAND_ON_24G },
	{ 82, BAND_ON_24G },
	{ 83, BAND_ON_24G },
	{ 84, BAND_ON_24G },
	{ 115, BAND_ON_5G },
	{ 116, BAND_ON_5G },
	{ 117, BAND_ON_5G },
	{ 118, BAND_ON_5G },
	{ 119, BAND_ON_5G },
	{ 120, BAND_ON_5G },
	{ 121, BAND_ON_5G },
	{ 122, BAND_ON_5G },
	{ 123, BAND_ON_5G },
	{ 124, BAND_ON_5G },
	{ 125, BAND_ON_5G },
	{ 126, BAND_ON_5G },
	{ 127, BAND_ON_5G },
	{ 128, BAND_ON_5G }
};

extern u8	WIFI_CCKRATES[];
bool rtw_is_cck_rate(u8 rate)
{
	int i;

	for (i = 0; i < 4; i++)
		if ((WIFI_CCKRATES[i] & 0x7F) == (rate & 0x7F))
			return 1;
	return 0;
}

extern u8	WIFI_OFDMRATES[];
bool rtw_is_ofdm_rate(u8 rate)
{
	int i;

	for (i = 0; i < 8; i++)
		if ((WIFI_OFDMRATES[i] & 0x7F) == (rate & 0x7F))
			return 1;
	return 0;
}

/* test if rate is defined in rtw_basic_rate_cck */
bool rtw_is_basic_rate_cck(u8 rate)
{
	int i;

	for (i = 0; i < 4; i++)
		if ((rtw_basic_rate_cck[i] & 0x7F) == (rate & 0x7F))
			return 1;
	return 0;
}

/* test if rate is defined in rtw_basic_rate_ofdm */
bool rtw_is_basic_rate_ofdm(u8 rate)
{
	int i;

	for (i = 0; i < 3; i++)
		if ((rtw_basic_rate_ofdm[i] & 0x7F) == (rate & 0x7F))
			return 1;
	return 0;
}

/* test if rate is defined in rtw_basic_rate_mix */
bool rtw_is_basic_rate_mix(u8 rate)
{
	int i;

	for (i = 0; i < 7; i++)
		if ((rtw_basic_rate_mix[i] & 0x7F) == (rate & 0x7F))
			return 1;
	return 0;
}
int cckrates_included(unsigned char *rate, int ratelen)
{
	int	i;

	for (i = 0; i < ratelen; i++) {
		if ((((rate[i]) & 0x7f) == 2)	|| (((rate[i]) & 0x7f) == 4) ||
		    (((rate[i]) & 0x7f) == 11)  || (((rate[i]) & 0x7f) == 22))
			return _TRUE;
	}

	return _FALSE;

}

int cckratesonly_included(unsigned char *rate, int ratelen)
{
	int	i;

	for (i = 0; i < ratelen; i++) {
		if ((((rate[i]) & 0x7f) != 2) && (((rate[i]) & 0x7f) != 4) &&
		    (((rate[i]) & 0x7f) != 11)  && (((rate[i]) & 0x7f) != 22))
			return _FALSE;
	}

	return _TRUE;
}

s8 rtw_get_sta_rx_nss(_adapter *adapter, struct sta_info *psta)
{
	s8 nss = 1;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	if (!psta)
		return nss;

	nss = GET_HAL_RX_NSS(dvobj);

#ifdef CONFIG_80211N_HT
	#ifdef CONFIG_80211AC_VHT
	#ifdef CONFIG_80211AX_HE
	if (psta->hepriv.he_option)
		nss = psta->phl_sta->asoc_cap.nss_tx;
	else
	#endif /* CONFIG_80211AX_HE */
	if (psta->vhtpriv.vht_option)
		nss = rtw_min(nss, rtw_vht_mcsmap_to_nss(psta->vhtpriv.vht_mcs_map));
	else
	#endif /* CONFIG_80211AC_VHT */
	if (psta->htpriv.ht_option)
		nss = rtw_min(nss, rtw_ht_mcsset_to_nss(psta->htpriv.ht_cap.supp_mcs_set));
#endif /*CONFIG_80211N_HT*/
	RTW_INFO("%s: %d ss\n", __func__, nss);
	return nss;
}

s8 rtw_get_sta_tx_nss(_adapter *adapter, struct sta_info *psta)
{
	s8 nss = 1;

	if (!psta)
		return nss;

	nss = GET_HAL_TX_NSS(adapter_to_dvobj(adapter));

#ifdef CONFIG_80211N_HT
	#ifdef CONFIG_80211AC_VHT
	#ifdef CONFIG_80211AX_HE
	if (psta->hepriv.he_option)
		nss = psta->phl_sta->asoc_cap.nss_rx;
	else
	#endif /* CONFIG_80211AX_HE */
	if (psta->vhtpriv.vht_option)
		nss = rtw_min(nss, rtw_vht_mcsmap_to_nss(psta->vhtpriv.vht_mcs_map));
	else
	#endif /* CONFIG_80211AC_VHT */
	if (psta->htpriv.ht_option)
		nss = rtw_min(nss, rtw_ht_mcsset_to_nss(psta->htpriv.ht_cap.supp_mcs_set));
#endif /*CONFIG_80211N_HT*/
	RTW_INFO("%s: %d SS\n", __func__, nss);
	return nss;
}

unsigned char ratetbl_val_2wifirate(unsigned char rate)
{
	unsigned char val = 0;

	switch (rate & 0x7f) {
	case 0:
		val = IEEE80211_CCK_RATE_1MB;
		break;

	case 1:
		val = IEEE80211_CCK_RATE_2MB;
		break;

	case 2:
		val = IEEE80211_CCK_RATE_5MB;
		break;

	case 3:
		val = IEEE80211_CCK_RATE_11MB;
		break;

	case 4:
		val = IEEE80211_OFDM_RATE_6MB;
		break;

	case 5:
		val = IEEE80211_OFDM_RATE_9MB;
		break;

	case 6:
		val = IEEE80211_OFDM_RATE_12MB;
		break;

	case 7:
		val = IEEE80211_OFDM_RATE_18MB;
		break;

	case 8:
		val = IEEE80211_OFDM_RATE_24MB;
		break;

	case 9:
		val = IEEE80211_OFDM_RATE_36MB;
		break;

	case 10:
		val = IEEE80211_OFDM_RATE_48MB;
		break;

	case 11:
		val = IEEE80211_OFDM_RATE_54MB;
		break;

	}

	return val;

}

int is_basicrate(_adapter *padapter, unsigned char rate)
{
	int i;
	unsigned char val;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	for (i = 0; i < NumRates; i++) {
		val = pmlmeext->basicrate[i];

		if ((val != 0xff) && (val != 0xfe)) {
			if (rate == ratetbl_val_2wifirate(val))
				return _TRUE;
		}
	}

	return _FALSE;
}

unsigned int ratetbl2rateset(_adapter *padapter, unsigned char *rateset)
{
	int i;
	unsigned char rate;
	unsigned int	len = 0;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	for (i = 0; i < NumRates; i++) {
		rate = pmlmeext->datarate[i];

		if (rtw_get_oper_ch(padapter) > 14 && rate < _6M_RATE_) /*5G no support CCK rate*/
			continue;

		switch (rate) {
		case 0xff:
			return len;

		case 0xfe:
			continue;

		default:
			rate = ratetbl_val_2wifirate(rate);

			if (is_basicrate(padapter, rate) == _TRUE)
				rate |= IEEE80211_BASIC_RATE_MASK;

			rateset[len] = rate;
			len++;
			break;
		}
	}
	return len;
}

void get_rate_set(_adapter *padapter, unsigned char *pbssrate, int *bssrate_len)
{
	unsigned char supportedrates[NumRates];

	_rtw_memset(supportedrates, 0, NumRates);
	*bssrate_len = ratetbl2rateset(padapter, supportedrates);
	_rtw_memcpy(pbssrate, supportedrates, *bssrate_len);
}

void set_mcs_rate_by_mask(u8 *mcs_set, u32 mask)
{
	u8 mcs_rate_1r = (u8)(mask & 0xff);
	u8 mcs_rate_2r = (u8)((mask >> 8) & 0xff);
	u8 mcs_rate_3r = (u8)((mask >> 16) & 0xff);
	u8 mcs_rate_4r = (u8)((mask >> 24) & 0xff);

	mcs_set[0] &= mcs_rate_1r;
	mcs_set[1] &= mcs_rate_2r;
	mcs_set[2] &= mcs_rate_3r;
	mcs_set[3] &= mcs_rate_4r;
}

void UpdateBrateTbl(
	_adapter *adapter,
	u8			*mBratesOS
)
{
	u8	i;
	u8	rate;

	/* 1M, 2M, 5.5M, 11M, 6M, 12M, 24M are mandatory. */
	for (i = 0; i < NDIS_802_11_LENGTH_RATES_EX; i++) {
		rate = mBratesOS[i] & 0x7f;
		switch (rate) {
		case IEEE80211_CCK_RATE_1MB:
		case IEEE80211_CCK_RATE_2MB:
		case IEEE80211_CCK_RATE_5MB:
		case IEEE80211_CCK_RATE_11MB:
		case IEEE80211_OFDM_RATE_6MB:
		case IEEE80211_OFDM_RATE_12MB:
		case IEEE80211_OFDM_RATE_24MB:
			mBratesOS[i] |= IEEE80211_BASIC_RATE_MASK;
			break;
		}
	}

}

void UpdateBrateTblForSoftAP(u8 *bssrateset, u32 bssratelen)
{
	u8	i;
	u8	rate;

	for (i = 0; i < bssratelen; i++) {
		rate = bssrateset[i] & 0x7f;
		switch (rate) {
		case IEEE80211_CCK_RATE_1MB:
		case IEEE80211_CCK_RATE_2MB:
		case IEEE80211_CCK_RATE_5MB:
		case IEEE80211_CCK_RATE_11MB:
			bssrateset[i] |= IEEE80211_BASIC_RATE_MASK;
			break;
		}
	}

}

inline u8 rtw_get_oper_ch(_adapter *adapter)
{
	return adapter_to_dvobj(adapter)->oper_channel;
}

inline void rtw_set_oper_ch(_adapter *adapter, u8 ch)
{
#ifdef DBG_CH_SWITCH
	const int len = 128;
	char msg[128] = {0};
	int cnt = 0;
	int i = 0;
#endif  /* DBG_CH_SWITCH */
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	if (dvobj->oper_channel != ch) {
		dvobj->on_oper_ch_time = rtw_get_current_time();

#ifdef DBG_CH_SWITCH
		cnt += snprintf(msg + cnt, len - cnt, "switch to ch %3u", ch);

		for (i = 0; i < dvobj->iface_nums; i++) {
			_adapter *iface = dvobj->padapters[i];
			cnt += snprintf(msg + cnt, len - cnt, " ["ADPT_FMT":", ADPT_ARG(iface));
			if (iface->mlmeextpriv.cur_channel == ch)
				cnt += snprintf(msg + cnt, len - cnt, "C");
			else
				cnt += snprintf(msg + cnt, len - cnt, "_");
			if (iface->wdinfo.listen_channel == ch && !rtw_p2p_chk_state(&iface->wdinfo, P2P_STATE_NONE))
				cnt += snprintf(msg + cnt, len - cnt, "L");
			else
				cnt += snprintf(msg + cnt, len - cnt, "_");
			cnt += snprintf(msg + cnt, len - cnt, "]");
		}

		RTW_INFO(FUNC_ADPT_FMT" %s\n", FUNC_ADPT_ARG(adapter), msg);
#endif /* DBG_CH_SWITCH */
	}

	dvobj->oper_channel = ch;
}

inline u8 rtw_get_oper_bw(_adapter *adapter)
{
	return adapter_to_dvobj(adapter)->oper_bwmode;
}

inline void rtw_set_oper_bw(_adapter *adapter, u8 bw)
{
	adapter_to_dvobj(adapter)->oper_bwmode = bw;
}

inline u8 rtw_get_oper_choffset(_adapter *adapter)
{
	return adapter_to_dvobj(adapter)->oper_ch_offset;
}

inline void rtw_set_oper_choffset(_adapter *adapter, u8 offset)
{
	adapter_to_dvobj(adapter)->oper_ch_offset = offset;
}


inline systime rtw_get_on_oper_ch_time(_adapter *adapter)
{
	return adapter_to_dvobj(adapter)->on_oper_ch_time;
}

inline systime rtw_get_on_cur_ch_time(_adapter *adapter)
{
	if (adapter->mlmeextpriv.cur_channel == adapter_to_dvobj(adapter)->oper_channel)
		return adapter_to_dvobj(adapter)->on_oper_ch_time;
	else
		return 0;
}

void set_channel_bwmode(_adapter *padapter,
				unsigned char channel,
				unsigned char channel_offset,
				unsigned short bwmode,
				u8 do_rfk)
{
#if (defined(CONFIG_TDLS) && defined(CONFIG_TDLS_CH_SW)) || defined(CONFIG_MCC_MODE)
	u8 iqk_info_backup = _FALSE;
#endif

	if (padapter->bNotifyChannelChange)
		RTW_INFO("[%s] ch = %d, offset = %d, bwmode = %d\n", __FUNCTION__, channel, channel_offset, bwmode);

	_rtw_mutex_lock_interruptible(&(adapter_to_dvobj(padapter)->setch_mutex));

#ifdef CONFIG_MCC_MODE
	if (MCC_EN(padapter)) {
		/* driver doesn't set channel setting reg under MCC */
		if (rtw_hal_check_mcc_status(padapter, MCC_STATUS_DOING_MCC))
			RTW_INFO("Warning: Do not set channel setting reg MCC mode\n");
	}
#endif

#ifdef CONFIG_DFS_MASTER
	{
		struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
		bool ori_overlap_radar_detect_ch = rtw_rfctl_overlap_radar_detect_ch(rfctl);
		bool new_overlap_radar_detect_ch = _rtw_rfctl_overlap_radar_detect_ch(rfctl, channel, bwmode, channel_offset);

		if (new_overlap_radar_detect_ch && IS_CH_WAITING(rfctl)) {
			rtw_phl_cmd_dfs_rd_set_cac_status(padapter->dvobj->phl,
							HW_BAND_0,
							true,
							PHL_CMD_DIRECTLY,
							0);
		}
#endif /* CONFIG_DFS_MASTER */

		/* set Channel */
		/* saved channel/bw info */
		rtw_set_oper_ch(padapter, channel);
		rtw_set_oper_bw(padapter, bwmode);
		rtw_set_oper_choffset(padapter, channel_offset);

#if (defined(CONFIG_TDLS) && defined(CONFIG_TDLS_CH_SW)) || defined(CONFIG_MCC_MODE)
		/* To check if we need to backup iqk info after switch chnl & bw */
		{
			u8 take_care_iqk;

			rtw_hal_get_hwreg(padapter, HW_VAR_CH_SW_NEED_TO_TAKE_CARE_IQK_INFO, &take_care_iqk);
			if ((take_care_iqk == _TRUE) && (do_rfk == _TRUE))
				iqk_info_backup = _TRUE;
		}
#endif

		rtw_hw_set_ch_bw(padapter, channel, (enum channel_width)bwmode,
				 channel_offset, do_rfk);

#if (defined(CONFIG_TDLS) && defined(CONFIG_TDLS_CH_SW)) || defined(CONFIG_MCC_MODE)
		if (iqk_info_backup == _TRUE)
			rtw_hal_ch_sw_iqk_info_backup(padapter);
#endif

#ifdef CONFIG_DFS_MASTER
		if (new_overlap_radar_detect_ch) {
			rtw_dfs_hal_radar_detect_enable(padapter);
		} else if (ori_overlap_radar_detect_ch) {
			rtw_dfs_hal_radar_detect_disable(padapter);
			rtw_phl_cmd_dfs_rd_set_cac_status(padapter->dvobj->phl,
						HW_BAND_0,
						false,
						PHL_CMD_DIRECTLY,
						0);
		}
	}
#endif /* CONFIG_DFS_MASTER */

	_rtw_mutex_unlock(&(adapter_to_dvobj(padapter)->setch_mutex));
}

__inline u8 *get_my_bssid(WLAN_BSSID_EX *pnetwork)
{
	return pnetwork->MacAddress;
}

u16 get_beacon_interval(WLAN_BSSID_EX *bss)
{
	unsigned short val;
	_rtw_memcpy((unsigned char *)&val, rtw_get_beacon_interval_from_ie(bss->IEs), 2);

	return le16_to_cpu(val);

}

int is_client_associated_to_ap(_adapter *padapter)
{
	struct mlme_ext_priv	*pmlmeext;
	struct mlme_ext_info	*pmlmeinfo;

	if (!padapter)
		return _FAIL;

	pmlmeext = &padapter->mlmeextpriv;
	pmlmeinfo = &(pmlmeext->mlmext_info);

	if (   MLME_HAS_STATE(pmlmeinfo, WIFI_FW_ASSOC_SUCCESS)
	    && MLME_IS_STATE(pmlmeinfo, WIFI_FW_STATION_STATE))
		return _TRUE;
	else
		return _FAIL;
}

int is_client_associated_to_ibss(_adapter *padapter)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (   MLME_HAS_STATE(pmlmeinfo, WIFI_FW_ASSOC_SUCCESS)
	    && MLME_IS_STATE(pmlmeinfo, WIFI_FW_ADHOC_STATE))
		return _TRUE;
	else
		return _FAIL;
}

/*GEORGIA_TODO_FIXIT*/
#define GET_H2CCMD_MSRRPT_PARM_OPMODE(__pH2CCmd) 	LE_BITS_TO_1BYTE(((u8 *)(__pH2CCmd)), 0, 1)
#define GET_H2CCMD_MSRRPT_PARM_ROLE(__pH2CCmd)	LE_BITS_TO_1BYTE(((u8 *)(__pH2CCmd)), 4, 4)

int is_IBSS_empty(_adapter *padapter)
{
/* ToDo */
#if 0
	int i;
	struct macid_ctl_t *macid_ctl = &padapter->dvobj->macid_ctl;

	for (i = 0; i < macid_ctl->num; i++) {
		if (!rtw_macid_is_used(macid_ctl, i))
			continue;
		if (!rtw_macid_is_iface_specific(macid_ctl, i, padapter))
			continue;
		if (!GET_H2CCMD_MSRRPT_PARM_OPMODE(&macid_ctl->h2c_msr[i]))
			continue;
		if (GET_H2CCMD_MSRRPT_PARM_ROLE(&macid_ctl->h2c_msr[i]) == H2C_MSR_ROLE_ADHOC)
			return _FAIL;
	}
#endif
	return _TRUE;
}

unsigned int decide_wait_for_beacon_timeout(unsigned int bcn_interval)
{
	if ((bcn_interval << 2) < WAIT_FOR_BCN_TO_MIN)
		return WAIT_FOR_BCN_TO_MIN;
	else if ((bcn_interval << 2) > WAIT_FOR_BCN_TO_MAX)
		return WAIT_FOR_BCN_TO_MAX;
	else
		return bcn_interval << 2;
}

#if defined(CONFIG_P2P) && defined(CONFIG_WFD)
void rtw_process_wfd_ie(_adapter *adapter, u8 *wfd_ie, u8 wfd_ielen, const char *tag)
{
	struct wifidirect_info *wdinfo = &adapter->wdinfo;

	u8 *attr_content;
	u32 attr_contentlen = 0;

	if (!rtw_hw_chk_wl_func(adapter_to_dvobj(adapter), WL_FUNC_MIRACAST))
		return;

	RTW_INFO("[%s] Found WFD IE\n", tag);
	attr_content = rtw_get_wfd_attr_content(wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, NULL, &attr_contentlen);
	if (attr_content && attr_contentlen) {
		wdinfo->wfd_info->peer_rtsp_ctrlport = RTW_GET_BE16(attr_content + 2);
		RTW_INFO("[%s] Peer PORT NUM = %d\n", tag, wdinfo->wfd_info->peer_rtsp_ctrlport);
	}
}

void rtw_process_wfd_ies(_adapter *adapter, u8 *ies, u8 ies_len, const char *tag)
{
	u8 *wfd_ie;
	u32	wfd_ielen;

	if (!rtw_hw_chk_wl_func(adapter_to_dvobj(adapter), WL_FUNC_MIRACAST))
		return;

	wfd_ie = rtw_get_wfd_ie(ies, ies_len, NULL, &wfd_ielen);
	while (wfd_ie) {
		rtw_process_wfd_ie(adapter, wfd_ie, wfd_ielen, tag);
		wfd_ie = rtw_get_wfd_ie(wfd_ie + wfd_ielen, (ies + ies_len) - (wfd_ie + wfd_ielen), NULL, &wfd_ielen);
	}
}
#endif /* defined(CONFIG_P2P) && defined(CONFIG_WFD) */

int WMM_param_handler(_adapter *padapter, PNDIS_802_11_VARIABLE_IEs	pIE)
{
	/* struct registry_priv	*pregpriv = &padapter->registrypriv; */
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (pmlmepriv->qospriv.qos_option == 0) {
		pmlmeinfo->WMM_enable = 0;
		return _FALSE;
	}

	if (_rtw_memcmp(&(pmlmeinfo->WMM_param), (pIE->data + 6), sizeof(struct WMM_para_element)))
		return _FALSE;
	else
		_rtw_memcpy(&(pmlmeinfo->WMM_param), (pIE->data + 6), sizeof(struct WMM_para_element));
	pmlmeinfo->WMM_enable = 1;
	return _TRUE;

#if 0
	if (pregpriv->wifi_spec == 1) {
		if (pmlmeinfo->WMM_enable == 1) {
			/* todo: compare the parameter set count & decide wheher to update or not */
			return _FAIL;
		} else {
			pmlmeinfo->WMM_enable = 1;
			_rtw_rtw_memcpy(&(pmlmeinfo->WMM_param), (pIE->data + 6), sizeof(struct WMM_para_element));
			return _TRUE;
		}
	} else {
		pmlmeinfo->WMM_enable = 0;
		return _FAIL;
	}
#endif

}

#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
u8 rtw_is_tbtx_capabilty(u8 *p, u8 len){
	int i;
	u8 tbtx_cap_ie[8] = {0x00, 0xe0, 0x4c, 0x01, 0x00, 0x00, 0x00, 0x00};

	for (i = 0; i < len; i++) {
		if (*(p + i) != tbtx_cap_ie[i])
			return _FALSE;
		else
			continue;
	}
	return _TRUE;
}
#endif

void WMMOnAssocRsp(_adapter *padapter)
{
	u8	ACI, ACM, AIFS, ECWMin, ECWMax, aSifsTime;
	u8	acm_mask;
	u16	TXOP;
	u32	acParm, i;
	u32	edca[4], inx[4];
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct xmit_priv		*pxmitpriv = &padapter->xmitpriv;
	struct registry_priv	*pregpriv = &padapter->registrypriv;
#ifdef CONFIG_WMMPS_STA
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct qos_priv	*pqospriv = &pmlmepriv->qospriv;
#endif /* CONFIG_WMMPS_STA */

	acm_mask = 0;

	if (WIFI_ROLE_IS_ON_5G(padapter) ||
	    (pmlmeext->cur_wireless_mode >= WLAN_MD_11N))
		aSifsTime = 16;
	else
		aSifsTime = 10;

	if (pmlmeinfo->WMM_enable == 0) {
		padapter->mlmepriv.acm_mask = 0;

		AIFS = aSifsTime + (2 * pmlmeinfo->slotTime);

		if (pmlmeext->cur_wireless_mode & (WLAN_MD_11G | WLAN_MD_11A)) {
			ECWMin = 4;
			ECWMax = 10;
		} else if (pmlmeext->cur_wireless_mode & WLAN_MD_11B) {
			ECWMin = 5;
			ECWMax = 10;
		} else {
			ECWMin = 4;
			ECWMax = 10;
		}

		TXOP = 0;
		acParm = AIFS | (ECWMin << 8) | (ECWMax << 12) | (TXOP << 16);
		rtw_hw_set_edca(padapter, 0, acParm);
		rtw_hw_set_edca(padapter, 1, acParm);
		rtw_hw_set_edca(padapter, 2, acParm);

		ECWMin = 2;
		ECWMax = 3;
		TXOP = 0x2f;
		acParm = AIFS | (ECWMin << 8) | (ECWMax << 12) | (TXOP << 16);
		rtw_hw_set_edca(padapter, 3, acParm);
	} else {
		edca[0] = edca[1] = edca[2] = edca[3] = 0;

		for (i = 0; i < 4; i++) {
			ACI = (pmlmeinfo->WMM_param.ac_param[i].ACI_AIFSN >> 5) & 0x03;
			ACM = (pmlmeinfo->WMM_param.ac_param[i].ACI_AIFSN >> 4) & 0x01;

			/* AIFS = AIFSN * slot time + SIFS - r2t phy delay */
			AIFS = (pmlmeinfo->WMM_param.ac_param[i].ACI_AIFSN & 0x0f) * pmlmeinfo->slotTime + aSifsTime;

			ECWMin = (pmlmeinfo->WMM_param.ac_param[i].CW & 0x0f);
			ECWMax = (pmlmeinfo->WMM_param.ac_param[i].CW & 0xf0) >> 4;
			TXOP = le16_to_cpu(pmlmeinfo->WMM_param.ac_param[i].TXOP_limit);

			acParm = AIFS | (ECWMin << 8) | (ECWMax << 12) | (TXOP << 16);
			rtw_hw_set_edca(padapter, ACI, acParm);

			switch (ACI) {
			case 0x0:
				acm_mask |= (ACM ? BIT(1) : 0);
				edca[XMIT_BE_QUEUE] = acParm;
				break;

			case 0x1:
				/* acm_mask |= (ACM? BIT(0):0); */
				edca[XMIT_BK_QUEUE] = acParm;
				break;

			case 0x2:
				acm_mask |= (ACM ? BIT(2) : 0);
				edca[XMIT_VI_QUEUE] = acParm;
				break;

			case 0x3:
				acm_mask |= (ACM ? BIT(3) : 0);
				edca[XMIT_VO_QUEUE] = acParm;
				break;
			}

			RTW_INFO("WMM(%x): %x, %x\n", ACI, ACM, acParm);
		}

		if (padapter->registrypriv.acm_method == 1)
			rtw_hal_set_hwreg(padapter, HW_VAR_ACM_CTRL, (u8 *)(&acm_mask));
		else
			padapter->mlmepriv.acm_mask = acm_mask;

		inx[0] = 0;
		inx[1] = 1;
		inx[2] = 2;
		inx[3] = 3;

		if (pregpriv->wifi_spec == 1) {
			u32	j, tmp, change_inx = _FALSE;

			/* entry indx: 0->vo, 1->vi, 2->be, 3->bk. */
			for (i = 0; i < 4; i++) {
				for (j = i + 1; j < 4; j++) {
					/* compare CW and AIFS */
					if ((edca[j] & 0xFFFF) < (edca[i] & 0xFFFF))
						change_inx = _TRUE;
					else if ((edca[j] & 0xFFFF) == (edca[i] & 0xFFFF)) {
						/* compare TXOP */
						if ((edca[j] >> 16) > (edca[i] >> 16))
							change_inx = _TRUE;
					}

					if (change_inx) {
						tmp = edca[i];
						edca[i] = edca[j];
						edca[j] = tmp;

						tmp = inx[i];
						inx[i] = inx[j];
						inx[j] = tmp;

						change_inx = _FALSE;
					}
				}
			}
		}

		for (i = 0; i < 4; i++) {
			pxmitpriv->wmm_para_seq[i] = inx[i];
			RTW_INFO("wmm_para_seq(%d): %d\n", i, pxmitpriv->wmm_para_seq[i]);
		}

#ifdef CONFIG_WMMPS_STA
		/* if AP supports UAPSD function, driver must set each uapsd TID to coresponding mac register 0x693 */
		if (pmlmeinfo->WMM_param.QoS_info & AP_SUPPORTED_UAPSD) {
			pqospriv->uapsd_ap_supported = 1;
			rtw_hal_set_hwreg(padapter, HW_VAR_UAPSD_TID, NULL);
		}
#endif /* CONFIG_WMMPS_STA */
	}
}

static void bwmode_update_check(_adapter *padapter, PNDIS_802_11_VARIABLE_IEs pIE)
{
#ifdef CONFIG_80211N_HT
	unsigned char	 new_bwmode;
	unsigned char  new_ch_offset;
	struct HT_info_element	*pHT_info;
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	struct ht_priv			*phtpriv = &pmlmepriv->htpriv;
	u8	cbw40_enable = 0;

	if (!pIE)
		return;

	if (phtpriv->ht_option == _FALSE)
		return;

	if (pmlmeext->cur_bwmode >= CHANNEL_WIDTH_80)
		return;

	if (pIE->Length > sizeof(struct HT_info_element))
		return;

	pHT_info = (struct HT_info_element *)pIE->data;

	if (rtw_hw_chk_bw_cap(adapter_to_dvobj(padapter), BW_CAP_40M)) {
		if (pmlmeext->cur_channel > 14) {
			if (REGSTY_IS_BW_5G_SUPPORT(pregistrypriv, CHANNEL_WIDTH_40))
				cbw40_enable = 1;
		} else {
			if (REGSTY_IS_BW_2G_SUPPORT(pregistrypriv, CHANNEL_WIDTH_40))
				cbw40_enable = 1;
		}
	}

	if ((pHT_info->infos[0] & BIT(2)) && cbw40_enable) {
		new_bwmode = CHANNEL_WIDTH_40;

		switch (pHT_info->infos[0] & 0x3) {
		case 1:
			new_ch_offset = CHAN_OFFSET_UPPER;
			break;

		case 3:
			new_ch_offset = CHAN_OFFSET_LOWER;
			break;

		default:
			new_bwmode = CHANNEL_WIDTH_20;
			new_ch_offset = CHAN_OFFSET_NO_EXT;
			break;
		}
	} else {
		new_bwmode = CHANNEL_WIDTH_20;
		new_ch_offset = CHAN_OFFSET_NO_EXT;
	}


	if ((new_bwmode != pmlmeext->cur_bwmode || new_ch_offset != pmlmeext->cur_ch_offset)
	    && new_bwmode < pmlmeext->cur_bwmode
	   ) {
		pmlmeinfo->bwmode_updated = _TRUE;

		pmlmeext->cur_bwmode = new_bwmode;
		pmlmeext->cur_ch_offset = new_ch_offset;

		/* update HT info also */
		HT_info_handler(padapter, pIE);
	} else
		pmlmeinfo->bwmode_updated = _FALSE;


	if (_TRUE == pmlmeinfo->bwmode_updated) {
		struct sta_info *psta;
		WLAN_BSSID_EX	*cur_network = &(pmlmeinfo->network);
		struct sta_priv	*pstapriv = &padapter->stapriv;

		/* set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode); */


		/* update ap's stainfo */
		psta = rtw_get_stainfo(pstapriv, cur_network->MacAddress);
		if (psta) {
			struct ht_priv	*phtpriv_sta = &psta->htpriv;

			if (phtpriv_sta->ht_option) {
				/* bwmode				 */
				psta->phl_sta->chandef.bw = pmlmeext->cur_bwmode;
				phtpriv_sta->ch_offset = pmlmeext->cur_ch_offset;
			} else {
				psta->phl_sta->chandef.bw = CHANNEL_WIDTH_20;
				phtpriv_sta->ch_offset = CHAN_OFFSET_NO_EXT;
			}

			rtw_dm_ra_mask_wk_cmd(padapter, (u8 *)psta);
		}

		/* pmlmeinfo->bwmode_updated = _FALSE; */ /* bwmode_updated done, reset it! */
	}
#endif /* CONFIG_80211N_HT */
}

#ifdef ROKU_PRIVATE
void Supported_rate_infra_ap(_adapter *padapter, PNDIS_802_11_VARIABLE_IEs pIE)
{
	unsigned int	i;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info		*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (pIE == NULL)
		return;

	for (i = 0 ; i < MIN(pIE->Length,sizeof(NDIS_802_11_RATES_EX)); i++)
		pmlmeinfo->SupportedRates_infra_ap[i] = (pIE->data[i]);

}

void Extended_Supported_rate_infra_ap(_adapter *padapter, PNDIS_802_11_VARIABLE_IEs pIE)
{
	unsigned int i, j;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info		*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (pIE == NULL)
		return;

	if (pIE->Length > 0) {
		for (i = 0; i < NDIS_802_11_LENGTH_RATES_EX; i++) {
			if (pmlmeinfo->SupportedRates_infra_ap[i] == 0)
				break;
		}
		for (j = 0; j < pIE->Length, i+j < NDIS_802_11_LENGTH_RATES_EX; j++)
			pmlmeinfo->SupportedRates_infra_ap[i+j] = (pIE->data[j]);
	}

}

void HT_get_ss_from_mcs_set(u8 *mcs_set, u8 *Rx_ss)
{
	u8 i, j;
	u8 r_ss = 0, t_ss = 0;

	for (i = 0; i < 4; i++) {
		if ((mcs_set[3-i] & 0xff) != 0x00) {
			r_ss = 4-i;
			break;
		}
	}

	*Rx_ss = r_ss;
}

void HT_caps_handler_infra_ap(_adapter *padapter, PNDIS_802_11_VARIABLE_IEs pIE)
{
	unsigned int	i;
	u8	cur_stbc_cap_infra_ap = 0;
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct ht_priv_infra_ap		*phtpriv = &pmlmepriv->htpriv_infra_ap;

	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info		*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (pIE == NULL)
		return;

	pmlmeinfo->ht_vht_received |= BIT(0);

	/*copy MCS_SET*/
	for (i = 3; i < 19; i++)
		phtpriv->MCS_set_infra_ap[i-3] = (pIE->data[i]);

	/*get number of stream from mcs set*/
	HT_get_ss_from_mcs_set(phtpriv->MCS_set_infra_ap, &phtpriv->Rx_ss_infra_ap);

	phtpriv->rx_highest_data_rate_infra_ap = le16_to_cpu(GET_HT_CAP_ELE_RX_HIGHEST_DATA_RATE(pIE->data));

	phtpriv->ldpc_cap_infra_ap = GET_HT_CAP_ELE_LDPC_CAP(pIE->data);

	if (GET_HT_CAP_ELE_RX_STBC(pIE->data))
		SET_FLAG(cur_stbc_cap_infra_ap, STBC_HT_ENABLE_RX);
	if (GET_HT_CAP_ELE_TX_STBC(pIE->data))
		SET_FLAG(cur_stbc_cap_infra_ap, STBC_HT_ENABLE_TX);
	phtpriv->stbc_cap_infra_ap = cur_stbc_cap_infra_ap;

	/*store ap info SGI 20m 40m*/
	phtpriv->sgi_20m_infra_ap = GET_HT_CAP_ELE_SHORT_GI20M(pIE->data);
	phtpriv->sgi_40m_infra_ap = GET_HT_CAP_ELE_SHORT_GI40M(pIE->data);

	/*store ap info for supported channel bandwidth*/
	phtpriv->channel_width_infra_ap = GET_HT_CAP_ELE_CHL_WIDTH(pIE->data);
}
#endif /* ROKU_PRIVATE */

void HT_caps_handler(_adapter *padapter, PNDIS_802_11_VARIABLE_IEs pIE)
{
#ifdef CONFIG_80211N_HT
	unsigned int i;
	u8 max_AMPDU_len, min_MPDU_spacing;
	u8 cur_ldpc_cap = 0, cur_stbc_cap = 0, cur_beamform_cap = 0, rx_nss = 0;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct ht_priv *phtpriv = &pmlmepriv->htpriv;

#ifdef CONFIG_DISABLE_MCS13TO15
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
#endif

	if (pIE == NULL)
		return;

	if (phtpriv->ht_option == _FALSE)
		return;

	pmlmeinfo->HT_caps_enable = 1;

	for (i = 0; i < (pIE->Length); i++) {
		if (i != 2) {
			/*	Commented by Albert 2010/07/12 */
			/*	Got the endian issue here. */
			pmlmeinfo->HT_caps.u.HT_cap[i] &= (pIE->data[i]);
		} else {
			/* AMPDU Parameters field */

			/* Get MIN of MAX AMPDU Length Exp */
			if ((pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para & 0x3) > (pIE->data[i] & 0x3))
				max_AMPDU_len = (pIE->data[i] & 0x3);
			else
				max_AMPDU_len = (pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para & 0x3);

			/* Get MAX of MIN MPDU Start Spacing */
			if ((pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para & 0x1c) > (pIE->data[i] & 0x1c))
				min_MPDU_spacing = (pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para & 0x1c);
			else
				min_MPDU_spacing = (pIE->data[i] & 0x1c);

			pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para = max_AMPDU_len | min_MPDU_spacing;
		}
	}

	/*	Commented by Albert 2010/07/12 */
	/*	Have to handle the endian issue after copying. */
	/*	HT_ext_caps didn't be used yet.	 */
	pmlmeinfo->HT_caps.u.HT_cap_element.HT_caps_info = le16_to_cpu(pmlmeinfo->HT_caps.u.HT_cap_element.HT_caps_info);
	pmlmeinfo->HT_caps.u.HT_cap_element.HT_ext_caps = le16_to_cpu(pmlmeinfo->HT_caps.u.HT_cap_element.HT_ext_caps);

	/* update the MCS set */
	for (i = 0; i < 16; i++)
		pmlmeinfo->HT_caps.u.HT_cap_element.MCS_rate[i] &= pmlmeext->default_supported_mcs_set[i];

	rx_nss = GET_HAL_RX_NSS(adapter_to_dvobj(padapter));

	switch (rx_nss) {
	case 1:
		set_mcs_rate_by_mask(pmlmeinfo->HT_caps.u.HT_cap_element.MCS_rate, MCS_RATE_1R);
		break;
	case 2:
		#ifdef CONFIG_DISABLE_MCS13TO15
		if (pmlmeext->cur_bwmode == CHANNEL_WIDTH_40 && pregistrypriv->wifi_spec != 1)
			set_mcs_rate_by_mask(pmlmeinfo->HT_caps.u.HT_cap_element.MCS_rate, MCS_RATE_2R_13TO15_OFF);
		else
		#endif
			set_mcs_rate_by_mask(pmlmeinfo->HT_caps.u.HT_cap_element.MCS_rate, MCS_RATE_2R);
		break;
	case 3:
		set_mcs_rate_by_mask(pmlmeinfo->HT_caps.u.HT_cap_element.MCS_rate, MCS_RATE_3R);
		break;
	case 4:
		set_mcs_rate_by_mask(pmlmeinfo->HT_caps.u.HT_cap_element.MCS_rate, MCS_RATE_4R);
		break;
	default:
		RTW_WARN("rf_type:%d or tx_nss:%u is not expected\n", GET_HAL_RFPATH(adapter_to_dvobj(padapter)), rx_nss);
	}

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE)) {

#ifdef CONFIG_BEAMFORMING
		/* Config Tx beamforming setting */
		if (TEST_FLAG(phtpriv->beamform_cap, BEAMFORMING_HT_BEAMFORMER_ENABLE) &&
		    GET_HT_CAP_TXBF_EXPLICIT_COMP_STEERING_CAP(pIE->data)) {
			SET_FLAG(cur_beamform_cap, BEAMFORMING_HT_BEAMFORMER_ENABLE);
			/* Shift to BEAMFORMING_HT_BEAMFORMEE_CHNL_EST_CAP*/
			SET_FLAG(cur_beamform_cap, GET_HT_CAP_TXBF_CHNL_ESTIMATION_NUM_ANTENNAS(pIE->data) << 6);
		}

		if (TEST_FLAG(phtpriv->beamform_cap, BEAMFORMING_HT_BEAMFORMEE_ENABLE) &&
		    GET_HT_CAP_TXBF_EXPLICIT_COMP_FEEDBACK_CAP(pIE->data)) {
			SET_FLAG(cur_beamform_cap, BEAMFORMING_HT_BEAMFORMEE_ENABLE);
			/* Shift to BEAMFORMING_HT_BEAMFORMER_STEER_NUM*/
			SET_FLAG(cur_beamform_cap, GET_HT_CAP_TXBF_COMP_STEERING_NUM_ANTENNAS(pIE->data) << 4);
		}
		phtpriv->beamform_cap = cur_beamform_cap;
		if (cur_beamform_cap)
			RTW_INFO("AP HT Beamforming Cap = 0x%02X\n", cur_beamform_cap);
#endif /*CONFIG_BEAMFORMING*/
		/* Config TX STBC setting */
		if (!GET_HT_CAP_ELE_TX_STBC(pIE->data))
			CLEAR_FLAG(phtpriv->stbc_cap, STBC_HT_ENABLE_TX);
	} else {
		/*WIFI_STATION_STATEorI_ADHOC_STATE or WIFI_ADHOC_MASTER_STATE*/
		/* Config LDPC Coding Capability */
		if (TEST_FLAG(phtpriv->ldpc_cap, LDPC_HT_ENABLE_TX) && GET_HT_CAP_ELE_LDPC_CAP(pIE->data)) {
			SET_FLAG(cur_ldpc_cap, (LDPC_HT_ENABLE_TX | LDPC_HT_CAP_TX));
			RTW_INFO("Enable HT Tx LDPC!\n");
		}
		phtpriv->ldpc_cap = cur_ldpc_cap;

		/* Config STBC setting */
		if (TEST_FLAG(phtpriv->stbc_cap, STBC_HT_ENABLE_TX) && GET_HT_CAP_ELE_RX_STBC(pIE->data)) {
			SET_FLAG(cur_stbc_cap, (STBC_HT_ENABLE_TX | STBC_HT_CAP_TX));
			RTW_INFO("Enable HT Tx STBC!\n");
		}
		phtpriv->stbc_cap = cur_stbc_cap;

#ifdef CONFIG_BEAMFORMING
		/* Config beamforming setting */
		if (TEST_FLAG(phtpriv->beamform_cap, BEAMFORMING_HT_BEAMFORMEE_ENABLE) &&
		    GET_HT_CAP_TXBF_EXPLICIT_COMP_STEERING_CAP(pIE->data)) {
			SET_FLAG(cur_beamform_cap, BEAMFORMING_HT_BEAMFORMEE_ENABLE);
			/* Shift to BEAMFORMING_HT_BEAMFORMEE_CHNL_EST_CAP*/
			SET_FLAG(cur_beamform_cap, GET_HT_CAP_TXBF_CHNL_ESTIMATION_NUM_ANTENNAS(pIE->data) << 6);
		}

		if (TEST_FLAG(phtpriv->beamform_cap, BEAMFORMING_HT_BEAMFORMER_ENABLE) &&
		    GET_HT_CAP_TXBF_EXPLICIT_COMP_FEEDBACK_CAP(pIE->data)) {
			SET_FLAG(cur_beamform_cap, BEAMFORMING_HT_BEAMFORMER_ENABLE);
			/* Shift to BEAMFORMING_HT_BEAMFORMER_STEER_NUM*/
			SET_FLAG(cur_beamform_cap, GET_HT_CAP_TXBF_COMP_STEERING_NUM_ANTENNAS(pIE->data) << 4);
		}
		phtpriv->beamform_cap = cur_beamform_cap;
		if (cur_beamform_cap)
			RTW_INFO("Client HT Beamforming Cap = 0x%02X\n", cur_beamform_cap);
#endif /*CONFIG_BEAMFORMING*/
	}

#endif /* CONFIG_80211N_HT */
}

void HT_info_handler(_adapter *padapter, PNDIS_802_11_VARIABLE_IEs pIE)
{
#ifdef CONFIG_80211N_HT
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct ht_priv			*phtpriv = &pmlmepriv->htpriv;

	if (pIE == NULL)
		return;

	if (phtpriv->ht_option == _FALSE)
		return;


	if (pIE->Length > sizeof(struct HT_info_element))
		return;

	pmlmeinfo->HT_info_enable = 1;
	_rtw_memcpy(&(pmlmeinfo->HT_info), pIE->data, pIE->Length);
#endif /* CONFIG_80211N_HT */
	return;
}

void HTOnAssocRsp(_adapter *padapter)
{
	unsigned char		max_AMPDU_len;
	unsigned char		min_MPDU_spacing;
	/* struct registry_priv	 *pregpriv = &padapter->registrypriv; */
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	RTW_INFO("%s\n", __FUNCTION__);

	if ((pmlmeinfo->HT_info_enable) && (pmlmeinfo->HT_caps_enable))
		pmlmeinfo->HT_enable = 1;
	else {
		pmlmeinfo->HT_enable = 0;
		/* set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode); */
		return;
	}

	/* handle A-MPDU parameter field */
	/*
		AMPDU_para [1:0]:Max AMPDU Len => 0:8k , 1:16k, 2:32k, 3:64k
		AMPDU_para [4:2]:Min MPDU Start Spacing
	*/
	max_AMPDU_len = pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para & 0x03;

	min_MPDU_spacing = (pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para & 0x1c) >> 2;

	rtw_hal_set_hwreg(padapter, HW_VAR_AMPDU_MIN_SPACE, (u8 *)(&min_MPDU_spacing));
#ifdef CONFIG_80211N_HT
	rtw_hal_set_hwreg(padapter, HW_VAR_AMPDU_FACTOR, (u8 *)(&max_AMPDU_len));
#endif /* CONFIG_80211N_HT */
#if 0 /* move to rtw_update_ht_cap() */
	if ((pregpriv->bw_mode > 0) &&
	    (pmlmeinfo->HT_caps.u.HT_cap_element.HT_caps_info & BIT(1)) &&
	    (pmlmeinfo->HT_info.infos[0] & BIT(2))) {
		/* switch to the 40M Hz mode accoring to the AP */
		pmlmeext->cur_bwmode = CHANNEL_WIDTH_40;
		switch ((pmlmeinfo->HT_info.infos[0] & 0x3)) {
		case IEEE80211_SCA:
			pmlmeext->cur_ch_offset = CHAN_OFFSET_UPPER;
			break;

		case IEEE80211_SCB:
			pmlmeext->cur_ch_offset = CHAN_OFFSET_LOWER;
			break;

		default:
			pmlmeext->cur_ch_offset = CHAN_OFFSET_NO_EXT;
			break;
		}
	}
#endif

	/* set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode); */

#if 0 /* move to rtw_update_ht_cap() */
	/*  */
	/* Config SM Power Save setting */
	/*  */
	pmlmeinfo->SM_PS = (pmlmeinfo->HT_caps.u.HT_cap_element.HT_caps_info & 0x0C) >> 2;
	if (pmlmeinfo->SM_PS == WLAN_HT_CAP_SM_PS_STATIC) {
#if 0
		u8 i;
		/* update the MCS rates */
		for (i = 0; i < 16; i++)
			pmlmeinfo->HT_caps.HT_cap_element.MCS_rate[i] &= MCS_rate_1R[i];
#endif
		RTW_INFO("%s(): WLAN_HT_CAP_SM_PS_STATIC\n", __FUNCTION__);
	}

	/*  */
	/* Config current HT Protection mode. */
	/*  */
	pmlmeinfo->HT_protection = pmlmeinfo->HT_info.infos[1] & 0x3;
#endif

}

void ERP_IE_handler(_adapter *padapter, PNDIS_802_11_VARIABLE_IEs pIE)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (pIE->Length > 1)
		return;

	pmlmeinfo->ERP_enable = 1;
	_rtw_memcpy(&(pmlmeinfo->ERP_IE), pIE->data, pIE->Length);
}

u8 VCS_update(_adapter *padapter, struct sta_info *psta)
{
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 rts_sel = 0, ret = false;

	if (psta->phl_sta == NULL) {
		return ret;
	}

	switch (pregpriv->vrtl_carrier_sense) { /* 0:off 1:on 2:auto */
	case 0: /* off */
		rts_sel = 1;
		psta->phl_sta->rts_en = 0;
		psta->phl_sta->hw_rts_en = 0;
		psta->phl_sta->cts2self = 0;
		break;

	case 1: /* on */
		if (pregpriv->vcs_type == RTS_CTS) { /* 1:RTS/CTS 2:CTS to self */
			rts_sel = 2;
			psta->phl_sta->rts_en = 1;
			psta->phl_sta->hw_rts_en = 1;
			psta->phl_sta->cts2self = 0;
		} else if (pregpriv->vcs_type == CTS_TO_SELF) {
			rts_sel = 3;
			psta->phl_sta->rts_en = 0;
			psta->phl_sta->hw_rts_en = 0;
			psta->phl_sta->cts2self = 1;
		} else {
			rts_sel = 4;
			psta->phl_sta->rts_en = 0;
			psta->phl_sta->hw_rts_en = 0;
			psta->phl_sta->cts2self = 0;
		}
		break;

	case 2: /* auto */
	default:
		if (psta->phl_sta->asoc_cap.sm_ps == SM_PS_DYNAMIC) {
			rts_sel = 5;
			psta->phl_sta->rts_en = 1;
			psta->phl_sta->hw_rts_en = 1;
			psta->phl_sta->cts2self = 0;
		} else if (((pmlmeinfo->ERP_enable) && (pmlmeinfo->ERP_IE & BIT(1)))
			/*||(pmlmepriv->ht_op_mode & HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT)
			|| (psta->traffic_mode != TRAFFIC_MODE_RX)*/
		) {
			if (pregpriv->vcs_type == RTS_CTS) {
				rts_sel = 6;
				psta->phl_sta->rts_en = 1;
				psta->phl_sta->hw_rts_en = 1;
				psta->phl_sta->cts2self = 0;
			} else if (pregpriv->vcs_type == CTS_TO_SELF) {
				rts_sel = 7;
				psta->phl_sta->rts_en = 0;
				psta->phl_sta->hw_rts_en = 0;
				psta->phl_sta->cts2self = 1;
			} else {
				rts_sel = 8;
				psta->phl_sta->rts_en = 0;
				psta->phl_sta->hw_rts_en = 0;
				psta->phl_sta->cts2self = 0;
			}
		} else {
			rts_sel = 9;
			psta->phl_sta->rts_en = 0;
			psta->phl_sta->hw_rts_en = 0;
			psta->phl_sta->cts2self = 0;
		}
		break;
	}

	/****************/
	/* special case */
	/****************/

	psta->phl_sta->rts_cca_mode = 0;

	if (padapter->registrypriv.wifi_mib.cts2self == 1) {
		rts_sel = 10;
		psta->phl_sta->rts_en = 0;
		psta->phl_sta->cts2self = 1;
		psta->phl_sta->hw_rts_en = 0;
	}
	else if (1 == padapter->no_rts
#ifdef CONFIG_VW_REFINE
		||	2 == padapter->vw_enable
#endif
	) {
		rts_sel = 11;
		psta->phl_sta->rts_en = 0;
		psta->phl_sta->cts2self = 0;
		psta->phl_sta->hw_rts_en = 0;
	}
	else if (padapter->cca_rts_mode) {		//_CCA_RTS_MODE_
		/* if RX TP do not turn on rts */
		if (psta->traffic_mode != TRAFFIC_MODE_RX) {
			rts_sel = 12;
			psta->phl_sta->rts_en = 1;
			psta->phl_sta->cts2self = 0;
		} else {
			rts_sel = 13;
			psta->phl_sta->rts_en = 0;
			psta->phl_sta->cts2self = 1;
		}
		psta->phl_sta->hw_rts_en = 0;
		psta->phl_sta->rts_cca_mode = padapter->cca_rts_mode;
	}

	if (rts_sel && rts_sel != psta->phl_sta->rts_sel) {
		psta->phl_sta->rts_sel = rts_sel;
		ret = true;
	}

	return ret;
}

void	update_ldpc_stbc_cap(struct sta_info *psta)
{
#ifdef CONFIG_80211N_HT

#ifdef CONFIG_80211AC_VHT
#ifdef CONFIG_80211AX_HE
	/* CONFIG_80211AX_HE_TODO */
#endif /* CONFIG_80211AX_HE */
	if (psta->vhtpriv.vht_option) {
		if (TEST_FLAG(psta->vhtpriv.ldpc_cap, LDPC_VHT_ENABLE_TX))
			psta->phl_sta->asoc_cap.vht_ldpc = 1;
		else
			psta->phl_sta->asoc_cap.vht_ldpc = 0;
	} else
#endif /* CONFIG_80211AC_VHT */
		if (psta->htpriv.ht_option) {
			if (TEST_FLAG(psta->htpriv.ldpc_cap, LDPC_HT_ENABLE_TX))
				psta->phl_sta->asoc_cap.ht_ldpc = 1;
			else
				psta->phl_sta->asoc_cap.ht_ldpc = 0;
		} else {
			psta->phl_sta->asoc_cap.vht_ldpc = 0;
			psta->phl_sta->asoc_cap.ht_ldpc = 0;
		}

#endif /* CONFIG_80211N_HT */
}

int check_ielen(u8 *start, uint len)
{
	int left = len;
	u8 *pos = start;
	u8 id, elen;

	while (left >= 2) {
		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left) {
			RTW_DBG("IEEE 802.11 element parse failed (id=%d elen=%d left=%lu)\n",
					id, elen, (unsigned long) left);
			return _FALSE;
		}
		if ((id == WLAN_EID_VENDOR_SPECIFIC) && (elen < 3))
				return _FALSE;

		left -= elen;
		pos += elen;
	}
	if (left)
		return _FALSE;

	return _TRUE;
}

int validate_beacon_len(u8 *pframe, u32 len)
{
	u8 ie_offset = _BEACON_IE_OFFSET_ + sizeof(struct rtw_ieee80211_hdr_3addr);

	if (len < ie_offset) {
		RTW_INFO("%s: incorrect beacon length(%d)\n", __func__, len);
		return _FALSE;
	}

	if (check_ielen(pframe + ie_offset, len - ie_offset) == _FALSE)
		return _FALSE;

	return _TRUE;
}


u8 support_rate_ranges[] = {
	IEEE80211_CCK_RATE_1MB,
	IEEE80211_CCK_RATE_2MB,
	IEEE80211_CCK_RATE_5MB,
	IEEE80211_CCK_RATE_11MB,
	IEEE80211_OFDM_RATE_6MB,
	IEEE80211_OFDM_RATE_9MB,
	IEEE80211_OFDM_RATE_12MB,
	IEEE80211_OFDM_RATE_18MB,
	IEEE80211_PBCC_RATE_22MB,
	IEEE80211_FREAK_RATE_22_5MB,
	IEEE80211_OFDM_RATE_24MB,
	IEEE80211_OFDM_RATE_36MB,
	IEEE80211_OFDM_RATE_48MB,
	IEEE80211_OFDM_RATE_54MB,
};

inline bool match_ranges(u16 EID, u32 value)
{
	int i;
	int nr_range;

	switch (EID) {
	case _EXT_SUPPORTEDRATES_IE_:
	case _SUPPORTEDRATES_IE_:
		nr_range = sizeof(support_rate_ranges)/sizeof(u8);
		for (i = 0; i < nr_range; i++) {
			/*	clear bit7 before searching.	*/
			value &= ~BIT(7);
			if (value == support_rate_ranges[i])
				return _TRUE;
		}
		break;
	default:
		break;
	};
	return _FALSE;
}

/*
 * rtw_validate_value: validate the IE contain.
 *
 *	Input :
 *		EID : Element ID
 *		p	: IE buffer (without EID & length)
 *		len	: IE length
 *	return:
 * 		_TRUE	: All Values are validated.
 *		_FALSE	: At least one value is NOT validated.
 */
bool rtw_validate_value(u16 EID, u8 *p, u16 len)
{
	u8 rate;
	u32 i, nr_val;

	switch (EID) {
	case _EXT_SUPPORTEDRATES_IE_:
	case _SUPPORTEDRATES_IE_:
		nr_val = len;
		for (i=0; i<nr_val; i++) {
			rate = *(p+i);
			if (match_ranges(EID, rate) == _FALSE)
				return _FALSE;
		}
		break;
	default:
		break;
	};
	return _TRUE;
}

bool is_hidden_ssid(char *ssid, int len)
{
	return len == 0 || is_all_null(ssid, len) == _TRUE;
}

inline bool hidden_ssid_ap(WLAN_BSSID_EX *snetwork)
{
	return ((snetwork->Ssid.SsidLength == 0) ||
		is_all_null(snetwork->Ssid.Ssid, snetwork->Ssid.SsidLength) == _TRUE);
}

/*
	Get SSID if this ilegal frame(probe resp) comes from a hidden SSID AP.
	Update the SSID to the corresponding pnetwork in scan queue.
*/
void rtw_absorb_ssid_ifneed(_adapter *padapter, WLAN_BSSID_EX *bssid, u8 *pframe)
{
	struct wlan_network *scanned = NULL;
	WLAN_BSSID_EX	*snetwork;
	u8 ie_offset, *p=NULL, *next_ie=NULL;
	u8 *mac;
	sint ssid_len_ori;
	u32 remain_len = 0;
	u8 backupIE[MAX_IE_SZ];
	u16 subtype;
#ifdef RTW_MI_SHARE_BSS_LIST
	_queue *queue = &padapter->dvobj->scanned_queue;
#else
	_queue *queue = &padapter->mlmepriv.scanned_queue;
#endif


	mac = get_addr2_ptr(pframe);
	subtype = get_frame_sub_type(pframe);

	if (subtype == WIFI_BEACON) {
		bssid->Reserved[0] = BSS_TYPE_BCN;
		ie_offset = _BEACON_IE_OFFSET_;
	} else {
		/* FIXME : more type */
		if (subtype == WIFI_PROBERSP) {
			ie_offset = _PROBERSP_IE_OFFSET_;
			bssid->Reserved[0] = BSS_TYPE_PROB_RSP;
		} else if (subtype == WIFI_PROBEREQ) {
			ie_offset = _PROBEREQ_IE_OFFSET_;
			bssid->Reserved[0] = BSS_TYPE_PROB_REQ;
		} else {
			bssid->Reserved[0] = BSS_TYPE_UNDEF;
			ie_offset = _FIXED_IE_LENGTH_;
		}
	}

	_rtw_spinlock_bh(&(queue->lock));
	scanned = _rtw_find_network(queue, mac);
	if (!scanned) {
		_rtw_spinunlock_bh(&(queue->lock));
		return;
	}

	snetwork = &(scanned->network);
	/* scan queue records as Hidden SSID && Input frame is NOT Hidden SSID	*/
	if (hidden_ssid_ap(snetwork) && !hidden_ssid_ap(bssid)) {
		p = rtw_get_ie(snetwork->IEs+ie_offset, _SSID_IE_, &ssid_len_ori, snetwork->IELength-ie_offset);
		if (!p) {
			_rtw_spinunlock_bh(&(queue->lock));
			return;
		}
		next_ie = p + 2 + ssid_len_ori;
		remain_len = snetwork->IELength - (next_ie - snetwork->IEs);
		scanned->network.Ssid.SsidLength = bssid->Ssid.SsidLength;
		_rtw_memcpy(scanned->network.Ssid.Ssid, bssid->Ssid.Ssid, bssid->Ssid.SsidLength);

		//update pnetwork->ssid, pnetwork->ssidlen
		_rtw_memcpy(backupIE, next_ie, remain_len);
		*(p+1) = bssid->Ssid.SsidLength;
		_rtw_memcpy(p+2, bssid->Ssid.Ssid, bssid->Ssid.SsidLength);
		_rtw_memcpy(p+2+bssid->Ssid.SsidLength, backupIE, remain_len);
		snetwork->IELength += bssid->Ssid.SsidLength;
	}
	_rtw_spinunlock_bh(&(queue->lock));
}

#ifdef DBG_RX_BCN
void rtw_debug_rx_bcn(_adapter *adapter, u8 *pframe, u32 packet_len)
{
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;
	struct mlme_ext_info *mlmeinfo = &(pmlmeext->mlmext_info);
	u16 sn = ((struct rtw_ieee80211_hdr_3addr *)pframe)->seq_ctl >> 4;
	u64 tsf, tsf_offset;
	u8 dtim_cnt, dtim_period, tim_bmap, tim_pvbit;

	update_TSF(pmlmeext, pframe, packet_len);
	tsf = pmlmeext->TSFValue;
	tsf_offset = rtw_modular64(pmlmeext->TSFValue, (mlmeinfo->bcn_interval * 1024));

	/*get TIM IE*/
	/*DTIM Count*/
	dtim_cnt = pmlmeext->tim[0];
	/*DTIM Period*/
	dtim_period = pmlmeext->tim[1];
	/*Bitmap*/
	tim_bmap = pmlmeext->tim[2];
	/*Partial VBitmap AID 0 ~ 7*/
	tim_pvbit = pmlmeext->tim[3];

	RTW_INFO("[BCN] SN-%d, TSF-%lld(us), offset-%lld, bcn_interval-%d DTIM-%d[%d] bitmap-0x%02x-0x%02x\n",
		sn, tsf, tsf_offset, mlmeinfo->bcn_interval, dtim_period, dtim_cnt, tim_bmap, tim_pvbit);
}
#endif

/*
 * rtw_get_bcn_keys: get beacon keys from recv frame
 *
 * TODO:
 *	WLAN_EID_COUNTRY
 *	WLAN_EID_ERP_INFO
 *	WLAN_EID_CHANNEL_SWITCH
 *	WLAN_EID_PWR_CONSTRAINT
 */
int rtw_get_bcn_keys(_adapter *adapter, u8 *pframe, u32 packet_len,
		     struct beacon_keys *recv_beacon)
{
	int left;
	u16 capability;
	unsigned char *pos;
	struct rtw_ieee802_11_elems elems;
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;

	_rtw_memset(recv_beacon, 0, sizeof(*recv_beacon));

	/* checking capabilities */
	capability = le16_to_cpu(*(unsigned short *)(pframe + WLAN_HDR_A3_LEN + 10));

	/* checking IEs */
	left = packet_len - sizeof(struct rtw_ieee80211_hdr_3addr) - _BEACON_IE_OFFSET_;
	pos = pframe + sizeof(struct rtw_ieee80211_hdr_3addr) + _BEACON_IE_OFFSET_;
	if (rtw_ieee802_11_parse_elems(pos, left, &elems, 1) == ParseFailed)
		return _FALSE;

	if (elems.ht_capabilities) {
		if (elems.ht_capabilities_len != 26)
			return _FALSE;
	}

	if (elems.ht_operation) {
		if (elems.ht_operation_len != 22)
			return _FALSE;
	}

	if (elems.vht_capabilities) {
		if (elems.vht_capabilities_len != 12)
			return _FALSE;
	}

	if (elems.vht_operation) {
		if (elems.vht_operation_len != 5)
			return _FALSE;
	}

	if (rtw_ies_get_supported_rate(pos, left, recv_beacon->rate_set, &recv_beacon->rate_num) == _FAIL)
		return _FALSE;

	if (cckratesonly_included(recv_beacon->rate_set, recv_beacon->rate_num) == _TRUE)
		recv_beacon->proto_cap |= PROTO_CAP_11B;
	else if (cckrates_included(recv_beacon->rate_set, recv_beacon->rate_num) == _TRUE)
		recv_beacon->proto_cap |= PROTO_CAP_11B | PROTO_CAP_11G;
	//else
		//recv_beacon->proto_cap |= PROTO_CAP_11G;


	if (elems.ht_capabilities && elems.ht_operation)
		recv_beacon->proto_cap |= PROTO_CAP_11N;

	if (elems.vht_capabilities && elems.vht_operation)
		recv_beacon->proto_cap |= PROTO_CAP_11AC;

	if (elems.he_capabilities && elems.he_operation)
		recv_beacon->proto_cap |= PROTO_CAP_11AX;

	/* check bw and channel offset */
	rtw_ies_get_chbw(pos, left, &recv_beacon->ch, &recv_beacon->bw, &recv_beacon->offset, 1, 1);
	if (!recv_beacon->ch) {
		/* we don't find channel IE, so don't check it */
		/* RTW_INFO("Oops: %s we don't find channel IE, so don't check it\n", __func__); */
		recv_beacon->ch = adapter->mlmeextpriv.cur_channel;
	}

	/* checking SSID */
	if (elems.ssid) {
		if (elems.ssid_len > sizeof(recv_beacon->ssid))
			return _FALSE;

		_rtw_memcpy(recv_beacon->ssid, elems.ssid, elems.ssid_len);
		recv_beacon->ssid_len = elems.ssid_len;
	}

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	/* checking WAPI first */
	if (elems.wapi_ie && elems.wapi_ie_len) {
		recv_beacon->encryp_protocol = ENCRYP_PROTOCOL_WAPI;
		rtw_parse_wapi_ie(elems.wapi_ie - 2, elems.wapi_ie_len + 2,
				  &recv_beacon->group_cipher, &recv_beacon->pairwise_cipher,
				  NULL, &recv_beacon->akm, &recv_beacon->mfp_opt);
	}
	else
#endif
	/* checking RSN first */
	if (elems.rsn_ie && elems.rsn_ie_len) {
		recv_beacon->encryp_protocol = ENCRYP_PROTOCOL_RSN;
		rtw_parse_wpa2_ie(elems.rsn_ie - 2, elems.rsn_ie_len + 2,
			&recv_beacon->group_cipher, &recv_beacon->pairwise_cipher,
			NULL, &recv_beacon->akm, &recv_beacon->mfp_opt);
	}
	/* checking WPA second */
	else if (elems.wpa_ie && elems.wpa_ie_len) {
		recv_beacon->encryp_protocol = ENCRYP_PROTOCOL_WPA;
		rtw_parse_wpa_ie(elems.wpa_ie - 2, elems.wpa_ie_len + 2,
			&recv_beacon->group_cipher, &recv_beacon->pairwise_cipher,
				 &recv_beacon->akm);
	}
	else if (capability & BIT(4))
		recv_beacon->encryp_protocol = ENCRYP_PROTOCOL_WEP;

	if (elems.tim && elems.tim_len) {

#ifdef CONFIG_AP_MODE
		if (!MLME_IS_AP(adapter))
#endif
		{
			#ifdef DBG_RX_BCN
			_rtw_memcpy(pmlmeext->tim, elems.tim, 4);
			#endif
			pmlmeext->dtim = elems.tim[1];
		}
	}

	/* checking RTW TBTX */
#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
	if (elems.tbtx_cap && elems.tbtx_cap_len) {
		struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

		if (rtw_is_tbtx_capabilty(elems.tbtx_cap, elems.tbtx_cap_len)){
			RTW_DBG("AP support TBTX\n");
		}
	}
#endif

	return _TRUE;
}

void rtw_dump_bcn_keys(void *sel, struct beacon_keys *recv_beacon)
{
	u8 ssid[IW_ESSID_MAX_SIZE + 1];

	_rtw_memcpy(ssid, recv_beacon->ssid, recv_beacon->ssid_len);
	ssid[recv_beacon->ssid_len] = '\0';

	RTW_PRINT_SEL(sel, "ssid = %s (len = %u)\n", ssid, recv_beacon->ssid_len);
	RTW_PRINT_SEL(sel, "ch = %u,%u,%u\n"
		, recv_beacon->ch, recv_beacon->bw, recv_beacon->offset);
	RTW_PRINT_SEL(sel, "proto_cap = 0x%02x\n", recv_beacon->proto_cap);
	RTW_MAP_DUMP_SEL(sel, "rate_set = "
		, recv_beacon->rate_set, recv_beacon->rate_num);
	RTW_PRINT_SEL(sel, "sec = %d, group = 0x%x, pair = 0x%x, akm = 0x%08x\n"
		, recv_beacon->encryp_protocol, recv_beacon->group_cipher
		, recv_beacon->pairwise_cipher, recv_beacon->akm);
}

int rtw_check_bcn_info(_adapter *adapter, u8 *pframe, u32 packet_len)
{
#define BCNKEY_VERIFY_PROTO_CAP 0
#define BCNKEY_VERIFY_WHOLE_RATE_SET 0

	u8 *pbssid = GetAddr3Ptr(pframe);
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct wlan_network *cur_network = &(adapter->mlmepriv.cur_network);
	struct beacon_keys *cur_beacon = &pmlmepriv->cur_beacon_keys;
	struct beacon_keys recv_beacon;
	int ret = 0;

#ifdef CONFIG_RTW_MULTI_AP /* todo: needs refine for hidden ap  */
		goto exit_success;
#endif

	if (is_client_associated_to_ap(adapter) == _FALSE)
		goto exit_success;

	if (rtw_get_bcn_keys(adapter, pframe, packet_len, &recv_beacon) == _FALSE)
		goto exit_success; /* parsing failed => broken IE */

	if(is_hidden_ssid(recv_beacon.ssid, recv_beacon.ssid_len)){
		_rtw_memcpy(recv_beacon.ssid,
			adapter->mlmeextpriv.mlmext_info.network.Ssid.Ssid, IW_ESSID_MAX_SIZE);
		recv_beacon.ssid_len = adapter->mlmeextpriv.mlmext_info.network.Ssid.SsidLength;
	}

#ifdef DBG_RX_BCN
	rtw_debug_rx_bcn(adapter, pframe, packet_len);
#endif

	/* hidden ssid, replace with current beacon ssid directly */
	if (is_hidden_ssid(recv_beacon.ssid, recv_beacon.ssid_len)) {
		_rtw_memcpy(recv_beacon.ssid, cur_beacon->ssid, cur_beacon->ssid_len);
		recv_beacon.ssid_len = cur_beacon->ssid_len;
	}

	if (_rtw_memcmp(&recv_beacon, cur_beacon, sizeof(recv_beacon)) == _FALSE) {
		struct beacon_keys tmp_beacon;

		RTW_INFO(FUNC_ADPT_FMT" new beacon occur!!\n", FUNC_ADPT_ARG(adapter));
		RTW_INFO(FUNC_ADPT_FMT" cur beacon key:\n", FUNC_ADPT_ARG(adapter));
		rtw_dump_bcn_keys(RTW_DBGDUMP, cur_beacon);
		RTW_INFO(FUNC_ADPT_FMT" new beacon key:\n", FUNC_ADPT_ARG(adapter));
		rtw_dump_bcn_keys(RTW_DBGDUMP, &recv_beacon);

		if (!rtw_is_chbw_grouped(cur_beacon->ch, cur_beacon->bw, cur_beacon->offset
				, recv_beacon.ch, recv_beacon.bw, recv_beacon.offset))
			goto exit;

		_rtw_memcpy(&tmp_beacon, cur_beacon, sizeof(tmp_beacon));

		/* check fields excluding below */
		tmp_beacon.ch = recv_beacon.ch;
		tmp_beacon.bw = recv_beacon.bw;
		tmp_beacon.offset = recv_beacon.offset;
		if (!BCNKEY_VERIFY_PROTO_CAP)
			tmp_beacon.proto_cap = recv_beacon.proto_cap;
		if (!BCNKEY_VERIFY_WHOLE_RATE_SET) {
			tmp_beacon.rate_num = recv_beacon.rate_num;
			_rtw_memcpy(tmp_beacon.rate_set, recv_beacon.rate_set, 12);
		}
		if (_rtw_memcmp(&tmp_beacon, &recv_beacon, sizeof(recv_beacon)) == _FALSE)
			goto exit;

		_rtw_memcpy(cur_beacon, &recv_beacon, sizeof(recv_beacon));
	}

exit_success:
	ret = 1;

exit:
	return ret;
}

void update_beacon_info(_adapter *padapter, u8 *pframe, uint pkt_len, struct sta_info *psta)
{
	unsigned int i;
	unsigned int len;
	PNDIS_802_11_VARIABLE_IEs	pIE;

#ifdef CONFIG_TDLS
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	u8 tdls_prohibited[] = { 0x00, 0x00, 0x00, 0x00, 0x10 }; /* bit(38): TDLS_prohibited */
#endif /* CONFIG_TDLS */

	len = pkt_len - (_BEACON_IE_OFFSET_ + WLAN_HDR_A3_LEN);

	for (i = 0; i < len;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(pframe + (_BEACON_IE_OFFSET_ + WLAN_HDR_A3_LEN) + i);

		switch (pIE->ElementID) {
		case _VENDOR_SPECIFIC_IE_:
			/* to update WMM paramter set while receiving beacon */
			if (_rtw_memcmp(pIE->data, WMM_PARA_OUI, 6) && pIE->Length == WLAN_WMM_LEN)	/* WMM */
				(WMM_param_handler(padapter, pIE)) ? report_wmm_edca_update(padapter) : 0;

			break;

		case _HT_EXTRA_INFO_IE_:	/* HT info */
			/* HT_info_handler(padapter, pIE); */
			bwmode_update_check(padapter, pIE);
			break;
#ifdef CONFIG_80211AC_VHT
		case EID_OpModeNotification:
			rtw_process_vht_op_mode_notify(padapter, pIE->data, psta);
			break;
#endif /* CONFIG_80211AC_VHT */
		case _ERPINFO_IE_:
			ERP_IE_handler(padapter, pIE);
			VCS_update(padapter, psta);
			break;

#ifdef CONFIG_TDLS
		case _EXT_CAP_IE_:
			if (check_ap_tdls_prohibited(pIE->data, pIE->Length) == _TRUE)
				ptdlsinfo->ap_prohibited = _TRUE;
			if (check_ap_tdls_ch_switching_prohibited(pIE->data, pIE->Length) == _TRUE)
				ptdlsinfo->ch_switch_prohibited = _TRUE;
			break;
#endif /* CONFIG_TDLS */
		default:
			break;
		}

		i += (pIE->Length + 2);
	}
}

#if CONFIG_DFS
void process_csa_ie(_adapter *padapter, u8 *ies, uint ies_len)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
#ifdef CONFIG_DFS_CSA_IE
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	_adapter *pri_adapter = dvobj_get_primary_adapter(dvobj);
	u16 ifbmp_m = rtw_mi_get_ap_mesh_ifbmp(pri_adapter);
#endif
	unsigned int i;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	u8 ch = 0, cnt = 0;

	/* TODO: compare with scheduling CSA */
	if (rfctl->csa_ch)
		return;

	for (i = 0; i + 1 < ies_len;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(ies + i);

		switch (pIE->ElementID) {
		case _CH_SWTICH_ANNOUNCE_:
			ch = *(pIE->data + 1);
			cnt = *(pIE->data + 2);
			break;
		default:
			break;
		}

		i += (pIE->Length + 2);
	}

	if (ch != 0) {
		rfctl->csa_ch = ch;
		rfctl->csa_cntdown = cnt;
#ifdef CONFIG_DFS_CSA_IE
		if (ifbmp_m) {
			rfctl->csa_set_ie = 1;
			RTW_INFO("[%s %d] repeater, csa_ch: %u, csa_cntdown: %u\n", __FUNCTION__, __LINE__, ch, cnt);
			rtw_mi_tx_beacon_hdl(padapter);
		} else
#endif
		{
			RTW_INFO("[%s %d] client, csa_ch: %u, csa_cntdown: %u\n", __FUNCTION__, __LINE__, ch, cnt);
			if (rtw_set_csa_cmd(padapter) != _SUCCESS) {
				rfctl->csa_ch = 0;
				rfctl->csa_cntdown = 0;
			}
		}
	}
}
#endif /* CONFIG_DFS */

#if (CONFIG_RTW_MULTI_AP_DFS_EN) && (CONFIG_DFS)
void process_map_member_csa_ie(_adapter *padapter, u8 *ies, uint ies_len)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
#ifdef CONFIG_DFS_CSA_IE
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	_adapter *pri_adapter = dvobj_get_primary_adapter(dvobj);
	u16 ifbmp_m = rtw_mi_get_ap_mesh_ifbmp(pri_adapter);
#endif
	unsigned int i;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	u8 ch = 0, cnt = 0;
	u8* mac = GetAddr3Ptr(ies - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

	/* TODO: compare with scheduling CSA */
	if (rfctl->csa_ch)
		return;

	for (i = 0; i + 1 < ies_len;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(ies + i);

		switch (pIE->ElementID) {
		case _CH_SWTICH_ANNOUNCE_:
			ch = *(pIE->data + 1);
			cnt = *(pIE->data + 2);
			break;
		default:
			break;
		}

		i += (pIE->Length + 2);
	}

	if ((ch != 0) && (rfctl->radar_detect_enabled)) {
		rfctl->csa_ch = ch;
		rfctl->csa_cntdown = cnt;
		rfctl->radar_detect_map_member = 1;
	}
}

bool is_multiap_neighbor(_adapter *padapter, u8 *mac)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	u8 i = 0;
	u8 num_bss = 0;

	if (padapter->map_neighbor_bss_tbl == NULL)
		return _FALSE;

	num_bss = padapter->map_neighbor_bss_tbl[0];

	for (i = 0; i < num_bss; i++) {
		if (_rtw_memcmp(mac, padapter->map_neighbor_bss_tbl + (1 + i*MACADDRLEN) , MACADDRLEN))
			return _TRUE;
	}

	return _FALSE;
}
#endif /* CONFIG_RTW_MULTI_AP_DFS_EN */
enum eap_type parsing_eapol_packet(_adapter *padapter, u8 *key_payload, struct sta_info *psta, u8 trx_type)
{
	struct security_priv *psecuritypriv = &(padapter->securitypriv);
	struct ieee802_1x_hdr *hdr;
	struct wpa_eapol_key *key;
	u16 key_info, key_data_length;
	char *trx_msg = trx_type ? "send" : "recv";
	enum eap_type eapol_type;

	hdr = (struct ieee802_1x_hdr *) key_payload;

	 /* WPS - eapol start packet */
	if (hdr->type == 1 && hdr->length == 0) {
		RTW_INFO("%s eapol start packet\n", trx_msg);
		return EAPOL_START;
	}

	if (hdr->type == 0) { /* WPS - eapol packet */
		RTW_INFO("%s eapol packet\n", trx_msg);
		return EAPOL_PACKET;
	}

	key = (struct wpa_eapol_key *) (hdr + 1);
#ifdef PLATFORM_ECOS
	key_info = be16_to_cpu(get_unaligned(((u16 *)(key->key_info))));
	key_data_length = be16_to_cpu(get_unaligned(((u16 *)(key->key_data_length))));
#else /* !PLATFORM_ECOS */
	key_info = be16_to_cpu(*((u16 *)(key->key_info)));
	key_data_length = be16_to_cpu(*((u16 *)(key->key_data_length)));
#endif /* PLATFORM_ECOS */

	if (!(key_info & WPA_KEY_INFO_KEY_TYPE)) { /* WPA group key handshake */
		if (key_info & WPA_KEY_INFO_ACK) {
			RTW_INFO("%s eapol packet - WPA Group Key 1/2\n", trx_msg);
			eapol_type = EAPOL_WPA_GROUP_KEY_1_2;
		} else {
			RTW_INFO("%s eapol packet - WPA Group Key 2/2\n", trx_msg);
			eapol_type = EAPOL_WPA_GROUP_KEY_2_2;

			/* WPA key-handshake has completed */
			if (psecuritypriv->ndisauthtype == Ndis802_11AuthModeWPAPSK)
				psta->state &= (~WIFI_UNDER_KEY_HANDSHAKE);
		}
	} else if (key_info & WPA_KEY_INFO_MIC) {
		if (key_data_length == 0) {
			RTW_INFO("%s eapol packet 4/4\n", trx_msg);
			eapol_type = EAPOL_4_4;
		} else if (key_info & WPA_KEY_INFO_ACK) {
			RTW_INFO("%s eapol packet 3/4\n", trx_msg);
			eapol_type = EAPOL_3_4;
		} else {
			RTW_INFO("%s eapol packet 2/4\n", trx_msg);
			eapol_type = EAPOL_2_4;
		}
	} else {
		RTW_INFO("%s eapol packet 1/4\n", trx_msg);
		eapol_type = EAPOL_1_4;
	}

	return eapol_type;
}

unsigned int is_ap_in_tkip(_adapter *padapter)
{
	u32 i;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*cur_network = &(pmlmeinfo->network);

	if (rtw_get_capability((WLAN_BSSID_EX *)cur_network) & WLAN_CAPABILITY_PRIVACY) {
		for (i = sizeof(NDIS_802_11_FIXED_IEs); i < pmlmeinfo->network.IELength;) {
			pIE = (PNDIS_802_11_VARIABLE_IEs)(pmlmeinfo->network.IEs + i);

			switch (pIE->ElementID) {
			case _VENDOR_SPECIFIC_IE_:
				if ((_rtw_memcmp(pIE->data, RTW_WPA_OUI, 4)) && (_rtw_memcmp((pIE->data + 12), WPA_TKIP_CIPHER, 4)))
					return _TRUE;
				break;

			case _RSN_IE_2_:
				if (_rtw_memcmp((pIE->data + 8), RSN_TKIP_CIPHER, 4))
					return _TRUE;

			default:
				break;
			}

			i += (pIE->Length + 2);
		}

		return _FALSE;
	} else
		return _FALSE;

}

unsigned int should_forbid_n_rate(_adapter *padapter)
{
	u32 i;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	WLAN_BSSID_EX  *cur_network = &pmlmepriv->cur_network.network;

	if (rtw_get_capability((WLAN_BSSID_EX *)cur_network) & WLAN_CAPABILITY_PRIVACY) {
		for (i = sizeof(NDIS_802_11_FIXED_IEs); i < cur_network->IELength;) {
			pIE = (PNDIS_802_11_VARIABLE_IEs)(cur_network->IEs + i);

			switch (pIE->ElementID) {
			case _VENDOR_SPECIFIC_IE_:
				if (_rtw_memcmp(pIE->data, RTW_WPA_OUI, 4) &&
				    ((_rtw_memcmp((pIE->data + 12), WPA_CIPHER_SUITE_CCMP, 4)) ||
				     (_rtw_memcmp((pIE->data + 16), WPA_CIPHER_SUITE_CCMP, 4))))
					return _FALSE;
				break;

			case _RSN_IE_2_:
				if ((_rtw_memcmp((pIE->data + 8), RSN_CIPHER_SUITE_CCMP, 4))  ||
				    (_rtw_memcmp((pIE->data + 12), RSN_CIPHER_SUITE_CCMP, 4)))
					return _FALSE;

			default:
				break;
			}

			i += (pIE->Length + 2);
		}

		return _TRUE;
	} else
		return _FALSE;

}


unsigned int is_ap_in_wep(_adapter *padapter)
{
	u32 i;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*cur_network = &(pmlmeinfo->network);

	if (rtw_get_capability((WLAN_BSSID_EX *)cur_network) & WLAN_CAPABILITY_PRIVACY) {
		for (i = sizeof(NDIS_802_11_FIXED_IEs); i < pmlmeinfo->network.IELength;) {
			pIE = (PNDIS_802_11_VARIABLE_IEs)(pmlmeinfo->network.IEs + i);

			switch (pIE->ElementID) {
			case _VENDOR_SPECIFIC_IE_:
				if (_rtw_memcmp(pIE->data, RTW_WPA_OUI, 4))
					return _FALSE;
				break;

			case _RSN_IE_2_:
				return _FALSE;

			default:
				break;
			}

			i += (pIE->Length + 2);
		}

		return _TRUE;
	} else
		return _FALSE;

}

int wifirate2_ratetbl_inx(unsigned char rate);
int wifirate2_ratetbl_inx(unsigned char rate)
{
	int	inx = 0;
	rate = rate & 0x7f;

	switch (rate) {
	case 54*2:
		inx = 11;
		break;

	case 48*2:
		inx = 10;
		break;

	case 36*2:
		inx = 9;
		break;

	case 24*2:
		inx = 8;
		break;

	case 18*2:
		inx = 7;
		break;

	case 12*2:
		inx = 6;
		break;

	case 9*2:
		inx = 5;
		break;

	case 6*2:
		inx = 4;
		break;

	case 11*2:
		inx = 3;
		break;
	case 11:
		inx = 2;
		break;

	case 2*2:
		inx = 1;
		break;

	case 1*2:
		inx = 0;
		break;

	}
	return inx;
}

unsigned int update_basic_rate(unsigned char *ptn, unsigned int ptn_sz)
{
	unsigned int i, num_of_rate;
	unsigned int mask = 0;

	num_of_rate = (ptn_sz > NumRates) ? NumRates : ptn_sz;

	for (i = 0; i < num_of_rate; i++) {
		if ((*(ptn + i)) & 0x80)
			mask |= 0x1 << wifirate2_ratetbl_inx(*(ptn + i));
	}
	return mask;
}

unsigned int update_supported_rate(unsigned char *ptn, unsigned int ptn_sz)
{
	unsigned int i, num_of_rate;
	unsigned int mask = 0;

	num_of_rate = (ptn_sz > NumRates) ? NumRates : ptn_sz;

	for (i = 0; i < num_of_rate; i++)
		mask |= 0x1 << wifirate2_ratetbl_inx(*(ptn + i));

	return mask;
}

int support_short_GI(_adapter *padapter, struct HT_caps_element *pHT_caps, u8 bwmode)
{
	unsigned char					bit_offset;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (!(pmlmeinfo->HT_enable))
		return _FAIL;

	bit_offset = (bwmode & CHANNEL_WIDTH_40) ? 6 : 5;

	if (pHT_caps->u.HT_cap_element.HT_caps_info & (0x1 << bit_offset))
		return _SUCCESS;
	else
		return _FAIL;
}

unsigned char get_highest_rate_idx(u64 mask)
{
	int i;
	unsigned char rate_idx = 0;

	for (i = 63; i >= 0; i--) {
		if ((mask >> i) & 0x01) {
			rate_idx = i;
			break;
		}
	}

	return rate_idx;
}
unsigned char get_lowest_rate_idx_ex(u64 mask, int start_bit)
{
	int i;
	unsigned char rate_idx = 0;

	for (i = start_bit; i < 64; i++) {
		if ((mask >> i) & 0x01) {
			rate_idx = i;
			break;
		}
	}

	return rate_idx;
}

u8 get_highest_bw_cap(u8 bwmode)
{
	u8 hbw = CHANNEL_WIDTH_20;

	if (bwmode & BW_CAP_80_80M)
		hbw = CHANNEL_WIDTH_80_80;
	else if (bwmode & BW_CAP_160M)
		hbw = CHANNEL_WIDTH_160;
	else if (bwmode & BW_CAP_80M)
		hbw = CHANNEL_WIDTH_80;
	else if (bwmode & BW_CAP_40M)
		hbw = CHANNEL_WIDTH_40;
	else if (bwmode & BW_CAP_20M)
		hbw = CHANNEL_WIDTH_20;
	else if (bwmode & BW_CAP_10M)
		hbw = CHANNEL_WIDTH_10;
	else if (bwmode & BW_CAP_5M)
		hbw = CHANNEL_WIDTH_5;

	return hbw;
}

/* Update RRSR and Rate for USERATE */
void update_tx_basic_rate(_adapter *padapter, u8 wirelessmode)
{
	NDIS_802_11_RATES_EX	supported_rates;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &padapter->wdinfo;

	/*	Added by Albert 2011/03/22 */
	/*	In the P2P mode, the driver should not support the b mode. */
	/*	So, the Tx packet shouldn't use the CCK rate */
	if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
		return;
#endif /* CONFIG_P2P */

	_rtw_memset(supported_rates, 0, NDIS_802_11_LENGTH_RATES_EX);

	/* clear B mod if current channel is in 5G band, avoid tx cck rate in 5G band. */
	if (pmlmeext->cur_channel > 14)
		wirelessmode &= ~(WLAN_MD_11B);

	if ((wirelessmode & WLAN_MD_11B) && (wirelessmode == WLAN_MD_11B))
		_rtw_memcpy(supported_rates, rtw_basic_rate_cck, 4);
	else if (wirelessmode & WLAN_MD_11B)
		_rtw_memcpy(supported_rates, rtw_basic_rate_mix, 7);
	else
		_rtw_memcpy(supported_rates, rtw_basic_rate_ofdm, 3);

	if (wirelessmode & WLAN_MD_11B)
		update_mgnt_tx_rate(padapter, IEEE80211_CCK_RATE_1MB);
	else
		update_mgnt_tx_rate(padapter, IEEE80211_OFDM_RATE_6MB);

	rtw_hal_set_hwreg(padapter, HW_VAR_BASIC_RATE, supported_rates);
}

unsigned char check_assoc_AP(u8 *pframe, uint len)
{
	unsigned int	i;
	PNDIS_802_11_VARIABLE_IEs	pIE;

	for (i = sizeof(NDIS_802_11_FIXED_IEs); i < len;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(pframe + i);

		switch (pIE->ElementID) {
		case _VENDOR_SPECIFIC_IE_:
			if ((_rtw_memcmp(pIE->data, ARTHEROS_OUI1, 3)) || (_rtw_memcmp(pIE->data, ARTHEROS_OUI2, 3))) {
				RTW_INFO("link to Artheros AP\n");
				return HT_IOT_PEER_ATHEROS;
			} else if ((_rtw_memcmp(pIE->data, BROADCOM_OUI[0], 3))
				   || (_rtw_memcmp(pIE->data, BROADCOM_OUI[1], 3))
				|| (_rtw_memcmp(pIE->data, BROADCOM_OUI[2], 3))) {
				RTW_INFO("link to Broadcom AP\n");
				return HT_IOT_PEER_BROADCOM;
			} else if (_rtw_memcmp(pIE->data, MARVELL_OUI, 3)) {
				RTW_INFO("link to Marvell AP\n");
				return HT_IOT_PEER_MARVELL;
			} else if (_rtw_memcmp(pIE->data, RALINK_OUI, 3)) {
				RTW_INFO("link to Ralink AP\n");
				return HT_IOT_PEER_RALINK;
			} else if (_rtw_memcmp(pIE->data, CISCO_OUI, 3)) {
				RTW_INFO("link to Cisco AP\n");
				return HT_IOT_PEER_CISCO;
			} else if (_rtw_memcmp(pIE->data, REALTEK_OUI, 3)) {
				u32	Vender = HT_IOT_PEER_REALTEK;

				if (pIE->Length >= 5) {
					if (pIE->data[4] == 1) {
						/* if(pIE->data[5] & RT_HT_CAP_USE_LONG_PREAMBLE) */
						/*	bssDesc->BssHT.RT2RT_HT_Mode |= RT_HT_CAP_USE_LONG_PREAMBLE; */

						if (pIE->data[5] & RT_HT_CAP_USE_92SE) {
							/* bssDesc->BssHT.RT2RT_HT_Mode |= RT_HT_CAP_USE_92SE; */
							Vender = HT_IOT_PEER_REALTEK_92SE;
						}
					}

					if (pIE->data[5] & RT_HT_CAP_USE_SOFTAP)
						Vender = HT_IOT_PEER_REALTEK_SOFTAP;

					if (pIE->data[4] == 2) {
						if (pIE->data[6] & RT_HT_CAP_USE_JAGUAR_BCUT) {
							Vender = HT_IOT_PEER_REALTEK_JAGUAR_BCUTAP;
							RTW_INFO("link to Realtek JAGUAR_BCUTAP\n");
						}
						if (pIE->data[6] & RT_HT_CAP_USE_JAGUAR_CCUT) {
							Vender = HT_IOT_PEER_REALTEK_JAGUAR_CCUTAP;
							RTW_INFO("link to Realtek JAGUAR_CCUTAP\n");
						}
					}
				}

				RTW_INFO("link to Realtek AP\n");
				return Vender;
			} else if (_rtw_memcmp(pIE->data, AIRGOCAP_OUI, 3)) {
				RTW_INFO("link to Airgo Cap\n");
				return HT_IOT_PEER_AIRGO;
			} else
				break;

		default:
			break;
		}

		i += (pIE->Length + 2);
	}

	RTW_INFO("link to new AP\n");
	return HT_IOT_PEER_UNKNOWN;
}

void get_assoc_AP_Vendor(char *vendor, u8 assoc_AP_vendor)
{
	switch (assoc_AP_vendor) {

	case HT_IOT_PEER_UNKNOWN:
	sprintf(vendor, "%s", "unknown");
	break;

	case HT_IOT_PEER_REALTEK:
	case HT_IOT_PEER_REALTEK_92SE:
	case HT_IOT_PEER_REALTEK_SOFTAP:
	case HT_IOT_PEER_REALTEK_JAGUAR_BCUTAP:
	case HT_IOT_PEER_REALTEK_JAGUAR_CCUTAP:

	sprintf(vendor, "%s", "Realtek");
	break;

	case HT_IOT_PEER_BROADCOM:
	sprintf(vendor, "%s", "Broadcom");
	break;

	case HT_IOT_PEER_MARVELL:
	sprintf(vendor, "%s", "Marvell");
	break;

	case HT_IOT_PEER_RALINK:
	sprintf(vendor, "%s", "Ralink");
	break;

	case HT_IOT_PEER_CISCO:
	sprintf(vendor, "%s", "Cisco");
	break;

	case HT_IOT_PEER_AIRGO:
	sprintf(vendor, "%s", "Airgo");
	break;

	case HT_IOT_PEER_ATHEROS:
	sprintf(vendor, "%s", "Atheros");
	break;

	default:
	sprintf(vendor, "%s", "unkown");
	break;
	}

}
#ifdef CONFIG_RTS_FULL_BW
void rtw_parse_sta_vendor_ie_8812(_adapter *adapter, struct sta_info *sta, u8 *tlv_ies, u16 tlv_ies_len)
{
	unsigned char REALTEK_OUI[] = {0x00,0xe0, 0x4c};
	u8 *p;

	p = rtw_get_ie_ex(tlv_ies, tlv_ies_len, WLAN_EID_VENDOR_SPECIFIC, REALTEK_OUI, 3, NULL, NULL);
	if (!p)
		goto exit;
	else {
		if(*(p+1) > 6 ) {

			if(*(p+6) != 2)
				goto exit;

			if(*(p+8) == RT_HT_CAP_USE_JAGUAR_BCUT)
				sta->vendor_8812 = TRUE;
			else if (*(p+8) == RT_HT_CAP_USE_JAGUAR_CCUT)
				sta->vendor_8812 = TRUE;
		}
	}
exit:
	return;
}
#endif/*CONFIG_RTS_FULL_BW*/

#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
void rtw_parse_vendor_ie(_adapter *padapter, struct sta_info *sta, u8 *pframe, u8 *tlv_ies, u16 tlv_ies_len)
{
	int ie_len, remain_len = tlv_ies_len;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u8 *pvsie, *oui;
	int i;

	while (remain_len > 0) {
		pvsie = rtw_get_ie(tlv_ies, WLAN_EID_VENDOR_SPECIFIC, &ie_len, remain_len);
		if (pvsie == NULL)
			break;
		for (i = 0 ; i < WLAN_MAX_VENDOR_IE_NUM ; i++) {
			if ((pmlmepriv->vendor_ielen[i] > 0) &&
					(pmlmepriv->vendor_ie_mask[i] & WIFI_PROBEREQ_RX_VENDOR_IE_BIT)){
				if (!memcmp(&pmlmepriv->vendor_ie[i][2], &pvsie[2], 3)) {
					if (sta) {
						if(pvsie[1] < WLAN_MAX_VENDOR_IE_LEN)
						{
							memcpy(sta->vendor_ie[i], &pvsie[2], pvsie[1]);
							sta->vendor_ielen[i] = pvsie[1];
						}
						else
						{
							memcpy(sta->vendor_ie[i], &pvsie[2], WLAN_MAX_VENDOR_IE_LEN);
							sta->vendor_ielen[i] = WLAN_MAX_VENDOR_IE_LEN;
							RTW_ERR("In %s(%d), overrunning sta->vendor_ie[%d]\n", __FUNCTION__, __LINE__,i);
						}
					} else {
#if defined(RTK_WLAN_EVENT_INDICATE) || defined(CONFIG_WLAN_EVENT_INDICATE_GENL)
						unsigned char tmpbuf[256+ETH_ALEN];
						memcpy(tmpbuf, get_sa(pframe), ETH_ALEN);
						memcpy(tmpbuf + ETH_ALEN, pvsie, pvsie[1]+2);
#ifdef RTK_WLAN_EVENT_INDICATE
						general_wlan_IndicateEvent(padapter, EVENT_PROBE_RX_VSIE, tmpbuf);
#endif
#ifdef CONFIG_WLAN_EVENT_INDICATE_GENL
						genl_wlan_IndicateEvent(padapter, EVENT_PROBE_RX_VSIE, tmpbuf);
#endif
#endif /* defined(RTK_WLAN_EVENT_INDICATE) || defined(CONFIG_WLAN_EVENT_INDICATE_GENL) */
					}
					break;
				}
			}
		}
		tlv_ies += (ie_len + 2);
		remain_len -= (ie_len + 2);
	}

	return;
}
#endif

#ifdef CONFIG_WLAN_MANAGER
u8 rtw_parse_opclass_ie(_adapter *padapter, struct sta_info *sta, u8 *pframe, u8 *tlv_ies, u16 tlv_ies_len)
{
	int ie_len, remain_len = tlv_ies_len, op_num, opclass;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u8 *pvsie;
	int i, j;

	pvsie = rtw_get_ie(tlv_ies, WLAN_EID_SUPPORTED_REGULATORY_CLASSES, &ie_len, remain_len);
	if (pvsie == NULL)
		return 0;

	op_num  = pvsie[1];
	i       = 0;

	RTW_INFO("===================>>>>op_num:%u\n", op_num);
	while(i < op_num)
	{
		opclass = pvsie[i+1];
		RTW_INFO("%u ", opclass);
		for (j = 0; j < (int)(sizeof(GLOBAL_OP_CLASS) / sizeof(OP_CLASS)); j++)
		{
			if(opclass == GLOBAL_OP_CLASS[j].op_class) {
				switch (GLOBAL_OP_CLASS[j].band) {
				case BAND_ON_24G:
					sta->supported_band |= BAND_CAP_2G;
					break;
				case BAND_ON_5G:
					sta->supported_band |= BAND_CAP_5G;
					break;
				default:
					break;
				}
				break;
			}
		}
		i++;
	}
	RTW_INFO("\n");
	RTW_INFO("supported_band: %u\n", sta->supported_band);
	RTW_INFO("===================>>>>\n");

	return 0;
}
#endif

#ifdef CONFIG_80211AC_VHT
unsigned char get_vht_mu_bfer_cap(u8 *pframe, uint len)
{
	unsigned int i;
	unsigned int mu_bfer=0;
	PNDIS_802_11_VARIABLE_IEs pIE;

	for (i = sizeof(NDIS_802_11_FIXED_IEs); i < len;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(pframe + i);

		switch (pIE->ElementID) {

		case EID_VHTCapability:
			mu_bfer = GET_VHT_CAPABILITY_ELE_MU_BFER(pIE->data);
			break;
		default:
			break;
		}
		i += (pIE->Length + 2);
	}
	return mu_bfer;
}
#endif

void update_capinfo(_adapter *adapter, u16 updateCap)
{
	struct mlme_ext_priv	*pmlmeext = &adapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	BOOLEAN		ShortPreamble;

	/* Check preamble mode, 2005.01.06, by rcnjko. */
	/* Mark to update preamble value forever, 2008.03.18 by lanhsin */
	/* if( pMgntInfo->RegPreambleMode == PREAMBLE_AUTO ) */
	{

		if (updateCap & cShortPreamble) {
			/* Short Preamble */
			if (pmlmeinfo->preamble_mode != PREAMBLE_SHORT) { /* PREAMBLE_LONG or PREAMBLE_AUTO */
				ShortPreamble = _TRUE;
				pmlmeinfo->preamble_mode = PREAMBLE_SHORT;
				rtw_hal_set_hwreg(adapter, HW_VAR_ACK_PREAMBLE, (u8 *)&ShortPreamble);
			}
		} else {
			/* Long Preamble */
			if (pmlmeinfo->preamble_mode != PREAMBLE_LONG) { /* PREAMBLE_SHORT or PREAMBLE_AUTO */
				ShortPreamble = _FALSE;
				pmlmeinfo->preamble_mode = PREAMBLE_LONG;
				rtw_hal_set_hwreg(adapter, HW_VAR_ACK_PREAMBLE, (u8 *)&ShortPreamble);
			}
		}
	}

	if (updateCap & cIBSS) {
		/* Filen: See 802.11-2007 p.91 */
		pmlmeinfo->slotTime = NON_SHORT_SLOT_TIME;
	} else {
		/* Filen: See 802.11-2007 p.90 */
		if (pmlmeext->cur_wireless_mode & (WLAN_MD_11N | WLAN_MD_11A | WLAN_MD_11AC))
			pmlmeinfo->slotTime = SHORT_SLOT_TIME;
		else if (pmlmeext->cur_wireless_mode & (WLAN_MD_11G)) {
			if ((updateCap & cShortSlotTime) /* && (!(pMgntInfo->pHTInfo->RT2RT_HT_Mode & RT_HT_CAP_USE_LONG_PREAMBLE)) */) {
				/* Short Slot Time */
				pmlmeinfo->slotTime = SHORT_SLOT_TIME;
			} else {
				/* Long Slot Time */
				pmlmeinfo->slotTime = NON_SHORT_SLOT_TIME;
			}
		} else {
			/* B Mode */
			pmlmeinfo->slotTime = NON_SHORT_SLOT_TIME;
		}
	}

	rtw_hal_set_hwreg(adapter, HW_VAR_SLOT_TIME, &pmlmeinfo->slotTime);

}

void update_client_wirelessmode_cap(u8 band, _adapter *padapter)
{
	u8 *p;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if(MLME_IS_STA(padapter))
	{
#ifdef CONFIG_80211AX_HE
		if((band & WLAN_MD_11AX)==0 )
		{
			pmlmeinfo->HE_enable = _FALSE;
			pmlmepriv->hepriv.he_option = _FALSE;
		}

#endif /* CONFIG_80211AX_HE */
#ifdef CONFIG_80211AC_VHT
		if((band & WLAN_MD_11AC)==0 )
		{
			pmlmeinfo->VHT_enable = _FALSE;
			pmlmepriv->vhtpriv.vht_option = _FALSE;
		}

#endif /* CONFIG_80211AC_VHT */
#ifdef CONFIG_80211N_HT
		if((band & WLAN_MD_11N)==0 )
		{
			pmlmepriv->htpriv.ht_option = _FALSE;
			pmlmeinfo->HT_info_enable = _FALSE;
			pmlmeinfo->HT_caps_enable = _FALSE;
			pmlmeinfo->HT_enable = _FALSE;
		}
	}
#endif /* CONFIG_80211N_HT */
}

/*
* According to pmlmeext->cur_wireless_mode set band in priv mib
* padapter->mlmeextpriv->cur_wireless_mode: the high mode | basic mode, used for bb
* padapter->registrypriv.wifi_mib: the current mode, used for packet & proc
*/
void update_priv_mib_band(_adapter *padapter)
{
	int network = padapter->mlmeextpriv.cur_wireless_mode;
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;

	pmibpriv->band = network;
	if (network & WLAN_MD_11AX) {
		if (padapter->mlmeextpriv.mlmext_info.VHT_enable) {
			pmibpriv->band |= (WLAN_MD_11AC | WLAN_MD_11N);
		} else {
			pmibpriv->band |= WLAN_MD_11N;
		}
	} else if (network & WLAN_MD_11AC) {
		pmibpriv->band |= WLAN_MD_11N;
	}
}
/*
* set adapter.mlmeextpriv.mlmext_info.HT_enable
* set adapter.mlmeextpriv.cur_wireless_mode
* set SIFS register
* set mgmt tx rate
*/
void update_wireless_mode(_adapter *padapter)
{
	int ratelen, network_type = 0;
	u32 SIFS_Timer;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*cur_network = &(pmlmeinfo->network);
	unsigned char			*rate = cur_network->SupportedRates;
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
#endif /* CONFIG_P2P */
	_adapter *primary_adapter = GET_PRIMARY_ADAPTER(padapter);
	u8 tmp_wireless_mode = 0;

	ratelen = rtw_get_rateset_len(cur_network->SupportedRates);

	if(MLME_IS_STA(padapter))
	{
		if(!is_primary_adapter(padapter))
			tmp_wireless_mode = primary_adapter->mlmeextpriv.cur_wireless_mode;
		else
			tmp_wireless_mode = pmibpriv->band;

		if(tmp_wireless_mode & WLAN_MD_11AX)
		{
			tmp_wireless_mode |= WLAN_MD_11N;
			if(tmp_wireless_mode & WLAN_MD_11A)
				tmp_wireless_mode |= WLAN_MD_11AC;
		}
		else if(tmp_wireless_mode & WLAN_MD_11AC)
			tmp_wireless_mode |= WLAN_MD_11N;

		update_client_wirelessmode_cap(tmp_wireless_mode, padapter);
		if (pmlmeext->cur_channel <= 14)
		{
			rtw_set_supported_rate(cur_network->SupportedRates, tmp_wireless_mode, cur_network->Configuration.DSConfig);
			ratelen = rtw_get_rateset_len(cur_network->SupportedRates);
		}
	}

	if ((pmlmeinfo->HT_info_enable) && (pmlmeinfo->HT_caps_enable))
		pmlmeinfo->HT_enable = 1;

//	pmibpriv->band = 0;
	if (pmlmeext->cur_channel > 14) {
		if (pmlmeinfo->HE_enable)
			network_type = WLAN_MD_11AX;
		else if (pmlmeinfo->VHT_enable)
			network_type = WLAN_MD_11AC;
		else if (pmlmeinfo->HT_enable)
			network_type = WLAN_MD_11N;

		network_type |= WLAN_MD_11A;
	} else {
		if (pmlmeinfo->HE_enable)
			network_type = WLAN_MD_11AX;
		else if (pmlmeinfo->VHT_enable)
			network_type = WLAN_MD_11AC;
		else if (pmlmeinfo->HT_enable)
			network_type = WLAN_MD_11N;

		if ((cckratesonly_included(rate, ratelen)) == _TRUE)
			network_type |= WLAN_MD_11B;
		else if ((cckrates_included(rate, ratelen)) == _TRUE)
			network_type |= WLAN_MD_11BG;
		else
			network_type |= WLAN_MD_11G;
	}

	pmlmeext->cur_wireless_mode = network_type & padapter->registrypriv.wireless_mode;
	RTW_PRINT(FUNC_ADPT_FMT" chan=%u, network_type=%02x, wireless_mode=%02x(%02x)\n",
	          FUNC_ADPT_ARG(padapter), pmlmeext->cur_channel,
	          network_type, pmlmeext->cur_wireless_mode,
	          padapter->registrypriv.wireless_mode);

	if(MLME_IS_AP(padapter))
		update_priv_mib_band(padapter);

	if ((pmlmeext->cur_wireless_mode & WLAN_MD_11B)
		#ifdef CONFIG_P2P
		&& (rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)
			#ifdef CONFIG_IOCTL_CFG80211
			|| !rtw_cfg80211_iface_has_p2p_group_cap(padapter)
			#endif
			)
		#endif
	)
		update_mgnt_tx_rate(padapter, IEEE80211_CCK_RATE_1MB);
	else
		update_mgnt_tx_rate(padapter, IEEE80211_OFDM_RATE_6MB);
}

void update_sta_basic_rate(struct sta_info *psta, u8 wireless_mode)
{
	if (is_supported_tx_cck(wireless_mode)) {
		/* Only B, B/G, and B/G/N AP could use CCK rate */
		_rtw_memcpy(psta->bssrateset, rtw_basic_rate_cck, 4);
		psta->bssratelen = 4;
	} else {
		_rtw_memcpy(psta->bssrateset, rtw_basic_rate_ofdm, 3);
		psta->bssratelen = 3;
	}
}

int rtw_ies_get_supported_rate(u8 *ies, uint ies_len, u8 *rate_set, u8 *rate_num)
{
	u8 *ie, *p;
	unsigned int ie_len;
	int i, j;

	struct support_rate_handler support_rate_tbl[] = {
		{IEEE80211_CCK_RATE_1MB, 		_FALSE,		_FALSE},
		{IEEE80211_CCK_RATE_2MB, 		_FALSE,		_FALSE},
		{IEEE80211_CCK_RATE_5MB, 		_FALSE,		_FALSE},
		{IEEE80211_CCK_RATE_11MB,		_FALSE,		_FALSE},
		{IEEE80211_OFDM_RATE_6MB,		_FALSE,		_FALSE},
		{IEEE80211_OFDM_RATE_9MB,		_FALSE,		_FALSE},
		{IEEE80211_OFDM_RATE_12MB,		_FALSE,		_FALSE},
		{IEEE80211_OFDM_RATE_18MB,		_FALSE,		_FALSE},
		{IEEE80211_OFDM_RATE_24MB,		_FALSE,		_FALSE},
		{IEEE80211_OFDM_RATE_36MB,		_FALSE,		_FALSE},
		{IEEE80211_OFDM_RATE_48MB,		_FALSE,		_FALSE},
		{IEEE80211_OFDM_RATE_54MB,		_FALSE,		_FALSE},
	};

	if (!rate_set || !rate_num)
		return _FALSE;

	*rate_num = 0;
	ie = rtw_get_ie(ies, _SUPPORTEDRATES_IE_, &ie_len, ies_len);
	if (ie == NULL)
		goto ext_rate;

	/* get valid supported rates */
	for (i = 0; i < 12; i++) {
		p = ie + 2;
		for (j = 0; j < ie_len; j++) {
			if ((*p & ~BIT(7)) == support_rate_tbl[i].rate){
				support_rate_tbl[i].existence = _TRUE;
				if ((*p) & BIT(7))
					support_rate_tbl[i].basic = _TRUE;
			}
			p++;
		}
	}

ext_rate:
	ie = rtw_get_ie(ies, _EXT_SUPPORTEDRATES_IE_, &ie_len, ies_len);
	if (ie) {
		/* get valid extended supported rates */
		for (i = 0; i < 12; i++) {
			p = ie + 2;
			for (j = 0; j < ie_len; j++) {
				if ((*p & ~BIT(7)) == support_rate_tbl[i].rate){
					support_rate_tbl[i].existence = _TRUE;
					if ((*p) & BIT(7))
						support_rate_tbl[i].basic = _TRUE;
				}
				p++;
			}
		}
	}

	for (i = 0; i < 12; i++){
		if (support_rate_tbl[i].existence){
			if (support_rate_tbl[i].basic)
				rate_set[*rate_num] = support_rate_tbl[i].rate | IEEE80211_BASIC_RATE_MASK;
			else
				rate_set[*rate_num] = support_rate_tbl[i].rate;
			*rate_num += 1;
		}
	}

	if (*rate_num == 0)
		return _FAIL;

	if (0) {
		int i;

		for (i = 0; i < *rate_num; i++)
			RTW_INFO("rate:0x%02x\n", *(rate_set + i));
	}

	return _SUCCESS;
}

void process_addba_req(_adapter *padapter, u8 *paddba_req, u8 *addr)
{
	struct sta_info *psta;
	u16 tid, start_seq, param;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct ADDBA_request	*preq = (struct ADDBA_request *)paddba_req;
	u8 size, accept = _FALSE;
	u16 sta_size = 0;

	psta = rtw_get_stainfo(pstapriv, addr);
	if (!psta)
		goto exit;

	start_seq = le16_to_cpu(preq->BA_starting_seqctrl) >> 4;
	param = le16_to_cpu(preq->BA_para_set);
	tid = (param >> 2) & 0x0f;
	sta_size = (param >> 6);

	RTW_INFO("[%s] from %pM tid=%d ampdu_num=%d\n", __func__, addr, tid, sta_size);

	accept = rtw_rx_ampdu_is_accept(padapter);
	if (padapter->fix_rx_ampdu_size != RX_AMPDU_SIZE_INVALID)
		size = padapter->fix_rx_ampdu_size;
	else {
		size = rtw_rx_ampdu_size(padapter);

		if ((sta_size < size) && (sta_size != 0))
			size = sta_size;

		size = rtw_min(size, rx_ampdu_size_sta_limit(padapter, psta));
	}

	RTW_INFO("[%s] send addbarsp to %pM bw=%d size=%d\n",
			__func__, addr, psta->phl_sta->chandef.bw, size);

	if (accept == _TRUE)
		rtw_addbarsp_cmd(padapter, addr, tid, preq, 0, size, start_seq);
	else
		rtw_addbarsp_cmd(padapter, addr, tid, preq, 37, size, start_seq);/* reject ADDBA Req */

exit:
	return;
}

void rtw_process_bar_frame(_adapter *padapter, union recv_frame *precv_frame)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	struct sta_info *psta = NULL;
	struct recv_reorder_ctrl *preorder_ctrl = NULL;
	u8 tid = 0;
	u16 start_seq=0;

	psta = rtw_get_stainfo(pstapriv, get_addr2_ptr(pframe));
	if (psta == NULL)
		goto exit;

	tid = ((cpu_to_le16((*(u16 *)(pframe + 16))) & 0xf000) >> 12);
	preorder_ctrl = &psta->recvreorder_ctrl[tid];
	start_seq = ((cpu_to_le16(*(u16 *)(pframe + 18))) >> 4);
	preorder_ctrl->indicate_seq = start_seq;

	rtw_phl_rx_bar(padapter->dvobj->phl, psta->phl_sta, tid, start_seq);
	/* for Debug use */
	if (0)
		RTW_INFO(FUNC_ADPT_FMT" tid=%d, start_seq=%d\n", FUNC_ADPT_ARG(padapter),  tid, start_seq);

exit:
	return;
}

void update_TSF(struct mlme_ext_priv *pmlmeext, u8 *pframe, uint len)
{
	u8 *pIE;
	u32 *pbuf;

	pIE = pframe + sizeof(struct rtw_ieee80211_hdr_3addr);
	pbuf = (u32 *)pIE;

	pmlmeext->TSFValue = le32_to_cpu(*(pbuf + 1));

	pmlmeext->TSFValue = pmlmeext->TSFValue << 32;

	pmlmeext->TSFValue |= le32_to_cpu(*pbuf);
}

#ifdef CONFIG_BCN_RECV_TIME
/*	calculate beacon receiving time
	1.RxBCNTime(CCK_1M) = [192us(preamble)] + [length of beacon(byte)*8us] + [10us]
	2.RxBCNTime(OFDM_6M) = [8us(S) + 8us(L) + 4us(L-SIG)] + [(length of beacon(byte)/3 + 1] *4us] + [10us]
*/
inline u16 _rx_bcn_time_calculate(uint bcn_len, u8 data_rate)
{
	u16 rx_bcn_time = 0;/*us*/

	if (data_rate == DESC_RATE1M)
		rx_bcn_time = 192 + bcn_len * 8 + 10;
	else if(data_rate == DESC_RATE6M)
		rx_bcn_time = 8 + 8 + 4 + (bcn_len /3 + 1) * 4 + 10;
/*
	else
		RTW_ERR("%s invalid data rate(0x%02x)\n", __func__, data_rate);
*/
	return rx_bcn_time;
}
void rtw_rx_bcn_time_update(_adapter *adapter, uint bcn_len, u8 data_rate)
{
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;

	pmlmeext->bcn_rx_time = _rx_bcn_time_calculate(bcn_len, data_rate);
}
#endif

void beacon_timing_control(_adapter *padapter)
{
	rtw_hal_bcn_param_setting(padapter);
}

inline void rtw_collect_bcn_info(_adapter *adapter)
{
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;

	if (!is_client_associated_to_ap(adapter))
		return;

	pmlmeext->cur_bcn_cnt = pmlmeext->bcn_cnt - pmlmeext->last_bcn_cnt;
	pmlmeext->last_bcn_cnt = pmlmeext->bcn_cnt;
	/*TODO get offset of bcn's timestamp*/
	/*pmlmeext->bcn_timestamp;*/
}

inline bool rtw_bmp_is_set(const u8 *bmp, u8 bmp_len, u8 id)
{
	if (id / 8 >= bmp_len)
		return 0;

	return bmp[id / 8] & BIT(id % 8);
}

inline void rtw_bmp_set(u8 *bmp, u8 bmp_len, u8 id)
{
	if (id / 8 < bmp_len)
		bmp[id / 8] |= BIT(id % 8);
}

inline void rtw_bmp_clear(u8 *bmp, u8 bmp_len, u8 id)
{
	if (id / 8 < bmp_len)
		bmp[id / 8] &= ~BIT(id % 8);
}

inline bool rtw_bmp_not_empty(const u8 *bmp, u8 bmp_len)
{
	int i;

	for (i = 0; i < bmp_len; i++) {
		if (bmp[i])
			return 1;
	}

	return 0;
}

inline bool rtw_bmp_not_empty_exclude_bit0(const u8 *bmp, u8 bmp_len)
{
	int i;

	for (i = 0; i < bmp_len; i++) {
		if (i == 0) {
			if (bmp[i] & 0xFE)
				return 1;
		} else {
			if (bmp[i])
				return 1;
		}
	}

	return 0;
}

#ifdef CONFIG_AP_MODE
/* Check the id be set or not in map , if yes , return a none zero value*/
bool rtw_tim_map_is_set(_adapter *padapter, const u8 *map, u8 id)
{
	return rtw_bmp_is_set(map, padapter->stapriv.aid_bmp_len, id);
}

/* Set the id into map array*/
void rtw_tim_map_set(_adapter *padapter, u8 *map, u8 id)
{
	rtw_bmp_set(map, padapter->stapriv.aid_bmp_len, id);
}

/* Clear the id from map array*/
void rtw_tim_map_clear(_adapter *padapter, u8 *map, u8 id)
{
	rtw_bmp_clear(map, padapter->stapriv.aid_bmp_len, id);
}

/* Check have anyone bit be set , if yes return true*/
bool rtw_tim_map_anyone_be_set(_adapter *padapter, const u8 *map)
{
	return rtw_bmp_not_empty(map, padapter->stapriv.aid_bmp_len);
}

/* Check have anyone bit be set exclude bit0 , if yes return true*/
bool rtw_tim_map_anyone_be_set_exclude_aid0(_adapter *padapter, const u8 *map)
{
	return rtw_bmp_not_empty_exclude_bit0(map, padapter->stapriv.aid_bmp_len);
}
#endif /* CONFIG_AP_MODE */

_adapter *dvobj_get_unregisterd_adapter(struct dvobj_priv *dvobj)
{
	_adapter *adapter = NULL;
	int i;

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (dvobj->padapters[i]->registered == 0)
			break;
	}

	if (i < dvobj->iface_nums)
		adapter = dvobj->padapters[i];

	return adapter;
}

_adapter *dvobj_get_adapter_by_addr(struct dvobj_priv *dvobj, u8 *addr)
{
	_adapter *adapter = NULL;
	int i;

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (_rtw_memcmp(dvobj->padapters[i]->mac_addr, addr, ETH_ALEN) == _TRUE)
			break;
	}

	if (i < dvobj->iface_nums)
		adapter = dvobj->padapters[i];

	return adapter;
}

#if defined(CONFIG_RTW_PERSIST_IF) || defined(CONFIG_RTW_PERSIST_IF)
_adapter *dvobj_get_adapter_by_name(struct dvobj_priv *dvobj, const char *ndev_name)
{
        int i;

        for (i = 0; i < dvobj->iface_nums; i++) {
                _adapter *adapter = dvobj->padapters[i];

                if (!adapter->registered || adapter->pnetdev == NULL)
                        continue;

                if (strcmp(adapter->pnetdev->name, ndev_name) == 0) {
                        return adapter;
                }
        }
        return NULL;
}
#endif /* CONFIG_RTW_PERSIST_IF */

#ifdef POWER_PERCENT_ADJUSTMENT
int txpower_percent_to_level(u8 percentage)
{
	const int percent_threshold_pos[] = {105, 118, 133, 149, 167, 188, 211};
	const int percent_threshold_neg[] = {95, 85, 75, 67, 60, 54, 48, 43, 38, 34, 30, 27, 24, 22, 19, 17, 15, 14, 12, 11, 10};
	const int pwrlevel_diff[9] = { -40, -34, -30, -28, -26, -24, -23, -22, -21}; // for < 10% case
	int i;

	if (percentage > 100) {
		for (i = 0; i < ARRAY_SIZE(percent_threshold_pos); i++) {
			if (percentage <= percent_threshold_pos[i]) {
				return (i * 2);
			}
		}
		return 12; /* +3db */
	}

	if (percentage >= 10) {
		for (i = 0; i < ARRAY_SIZE(percent_threshold_neg); i++) {
			if (percentage >= percent_threshold_neg[i]) {
				return (int) - (i * 2);
			}
		}
	}

	if (percentage < 1)
		percentage = 1;
	else if (percentage > 9)
		percentage = 9;

	return (pwrlevel_diff[percentage - 1] * 2);
}
#endif /* POWER_PERCENT_ADJUSTMENT */


#ifdef RTK_WLAN_EVENT_INDICATE
//
// B0~B5     B6        B7~
// Sta Addr  Reason  Interface Name
//
int rtk_wlan_event_indicate(char* ifname, int event, unsigned char* addr, char reason)
{
	char data[MAC_ADDR_LEN+1+IFNAMSIZ+2] = {0};
	int data_len = 0, name_len = 0;
	int rtk_eventd_pid = get_nl_eventd_pid();
	struct sock* nl_eventd_sk = get_nl_eventd_sk();

	if(nl_eventd_sk==NULL || rtk_eventd_pid==0)
	{
		printk("%s:%d, report wifi link status failed, pid=%d,sk=%p\n",__FUNCTION__,__LINE__, rtk_eventd_pid, nl_eventd_sk);
		return -1;
	}

	memcpy(data, addr, MAC_ADDR_LEN);
	data_len += MAC_ADDR_LEN;

	data[data_len] = reason;
	data_len += 1;

	if(strlen(ifname) > IFNAMSIZ){
		printk("%s:%d, ifname[%s] len longer than %d!\n",__FUNCTION__,__LINE__, ifname, IFNAMSIZ);
		name_len = IFNAMSIZ;
	}else{
		name_len = strlen(ifname);
	}
	strncpy(data+data_len, ifname, name_len);
	data_len += name_len;

	data[data_len] = '\0';
	data_len += 1;

	rtk_eventd_netlink_send(rtk_eventd_pid, nl_eventd_sk, event, NULL, data, data_len);

	return 0;
}

// B0~B5     B6-B(len+6-1)        B(len+6)~
// Sta Addr  info           Interface Name
int rtk_wlan_with_info_event_indicate(char* ifname, int event, unsigned char* addr, unsigned char *info, int len)
{
	char *data;
	int data_len = 0, name_len = 0;
	int rtk_eventd_pid = get_nl_eventd_pid();
	struct sock* nl_eventd_sk = get_nl_eventd_sk();

	data = (char *)rtw_zmalloc((MAC_ADDR_LEN+WLAN_INFO_LEN+IFNAMSIZ+2)*sizeof(char));
	if(data == NULL){
		RTW_ERR("[%s:%d]rtw_zmalloc failed!\n", __FUNCTION__,__LINE__);
		return -1;
	}

	if(nl_eventd_sk==NULL || rtk_eventd_pid==0)
	{
		printk("%s:%d, report wifi link status failed, pid=%d,sk=%p\n",__FUNCTION__,__LINE__, rtk_eventd_pid, nl_eventd_sk);
		rtw_mfree(data, (MAC_ADDR_LEN+WLAN_INFO_LEN+IFNAMSIZ+2)*sizeof(char));
		return -1;
	}

	memcpy(data, addr, MAC_ADDR_LEN);
	data_len += MAC_ADDR_LEN;

	if(len > WLAN_INFO_LEN){
		printk("%s:%d, ifname[%s] info len longer than %d!\n",__FUNCTION__,__LINE__, ifname, WLAN_INFO_LEN);
		rtw_mfree(data, (MAC_ADDR_LEN+WLAN_INFO_LEN+IFNAMSIZ+2)*sizeof(char));
		return -1;
	}
	memcpy(data+data_len, info, len);
	data_len += len;

	if(strlen(ifname) > IFNAMSIZ){
		printk("%s:%d, ifname[%s] len longer than %d!\n",__FUNCTION__,__LINE__, ifname, IFNAMSIZ);
		name_len = IFNAMSIZ;
	}else{
		name_len = strlen(ifname);
	}
	strncpy(data+data_len, ifname, name_len);
	data_len += name_len;

	data[data_len] = '\0';
	data_len += 1;

	rtk_eventd_netlink_send(rtk_eventd_pid, nl_eventd_sk, event, NULL, data, data_len);

	rtw_mfree(data, (MAC_ADDR_LEN+WLAN_INFO_LEN+IFNAMSIZ+2)*sizeof(char));
	return 0;
}

void general_wlan_IndicateEvent(_adapter *padapter, unsigned char event, void *data)
{

	unsigned char mac[ETH_ALEN]={0};
	unsigned char *info;
	int eventid=WIFI_EVENT_WITH_INFO;
	unsigned char *data_ptr;
	int len=0;
	struct net_device *pnetdev = padapter->pnetdev;

	info = (unsigned char *)rtw_zmalloc(WLAN_INFO_LEN*sizeof(unsigned char));
	if(info == NULL){
		RTW_ERR("[%s:%d]rtw_zmalloc failed!\n", __FUNCTION__,__LINE__);
		return;
	}

	switch (event)
	{
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
		case EVENT_PROBE_RX_VSIE:
		{   /*tmp = [mac + pvsie(eid + len + oui + value)]*/
			unsigned char* tmp = (unsigned char*)data;
			PROBE_VSIE_RECORD probe_vsie_record;
			memset(&probe_vsie_record, 0, sizeof(PROBE_VSIE_RECORD));
			probe_vsie_record.IsMoreEvent =1;
			probe_vsie_record.EventId = event;
			memcpy(probe_vsie_record.hwaddr, tmp, ETH_ALEN);
			memcpy(mac, tmp, ETH_ALEN);
			probe_vsie_record.data_len = tmp[ETH_ALEN+1];
			if (probe_vsie_record.data_len < WLAN_MAX_VENDOR_IE_LEN) {
				memcpy(probe_vsie_record.ie_data, tmp + ETH_ALEN + 2, probe_vsie_record.data_len);
			}
			RTW_INFO("[PROBE_VS_IE] Probe_req %02X%02X%02X%02X%02X%02X, OUI %02X-%02X-%02X, len %d\n",
					probe_vsie_record.hwaddr[0],probe_vsie_record.hwaddr[1],probe_vsie_record.hwaddr[2],
					probe_vsie_record.hwaddr[3],probe_vsie_record.hwaddr[4],probe_vsie_record.hwaddr[5],
					probe_vsie_record.ie_data[0],probe_vsie_record.ie_data[1],probe_vsie_record.ie_data[2],probe_vsie_record.data_len);

			data_ptr = info;

			memcpy(data_ptr, &probe_vsie_record.EventId, 1);
			data_ptr += 1;
			len += 1;
			memcpy(data_ptr, &probe_vsie_record.IsMoreEvent, 1);
			data_ptr += 1;
			len += 1;
			memcpy(data_ptr, probe_vsie_record.hwaddr, ETH_ALEN);
			data_ptr += ETH_ALEN;
			len += ETH_ALEN;
			memcpy(data_ptr, &probe_vsie_record.data_len, 1);
			data_ptr += 1;
			len += 1;
			memcpy(data_ptr, probe_vsie_record.ie_data, probe_vsie_record.data_len);
			len += probe_vsie_record.data_len;
			break;
		}
#endif
#ifdef RTL_LINK_ROAMING_REPORT
		case EVENT_ROAMING_INFO_RPT:
		{
			len = construct_rtl_link_roaming_info(padapter, info);
			_rtw_memset(mac, 0, MAC_ADDR_LEN); //useless
			break;
		}
#endif
#ifdef MONITOR_UNASSOC_STA
		case EVENT_UNASSOC_STA_RPT:
		{
			len = rtw_construct_unassoc_sta_info(padapter, info);
			_rtw_memset(mac, 0, MAC_ADDR_LEN); //useless
			break;
		}
#endif
		default:
			RTW_ERR("Not supported event : %d\n", event);
			break;
	}

	if(len)
		rtk_wlan_with_info_event_indicate(pnetdev->name, eventid, mac, info, len);

	rtw_mfree(info, WLAN_INFO_LEN*sizeof(unsigned char));
}
#endif

#ifdef CONFIG_WLAN_EVENT_INDICATE_GENL
#define WLAN_INFO_LEN		300
//
// B0~B5     B6        B7~
// Sta Addr  Reason  Interface Name
//
int rtk_wlan_genl_indicate(char* ifname, int event, unsigned char* addr, char reason)
{
	char data[MAC_ADDR_LEN+1+IFNAMSIZ+2] = {0};
	int data_len = 0, name_len = 0;
	int rtk_eventd_pid = get_genl_eventd_pid();

	if (rtk_eventd_pid == 0)
	{
		printk("%s:%d, report wifi link status failed, pid=%d\n",__FUNCTION__,__LINE__, rtk_eventd_pid);
		return -1;
	}

	memcpy(data, addr, MAC_ADDR_LEN);
	data_len += MAC_ADDR_LEN;

	data[data_len] = reason;
	data_len += 1;

	if(strlen(ifname) > IFNAMSIZ){
		printk("%s:%d, ifname[%s] len longer than %d!\n",__FUNCTION__,__LINE__, ifname, IFNAMSIZ);
		name_len = IFNAMSIZ;
	}else{
		name_len = strlen(ifname);
	}
	strncpy(data+data_len, ifname, name_len);
	data_len += name_len;

	data[data_len] = '\0';
	data_len += 1;

	rtk_eventd_genl_send(rtk_eventd_pid, event, NULL, data, data_len);

	return 0;
}

// B0~B5     B6-B(len+6-1)        B(len+6)~
// Sta Addr  info           Interface Name
int rtk_wlan_with_info_genl_indicate(char* ifname, int event, unsigned char* addr, unsigned char *info, int len)
{
	char data[MAC_ADDR_LEN+WLAN_INFO_LEN+IFNAMSIZ+2] = {0};
	int data_len = 0, name_len = 0;
	int rtk_eventd_pid = get_genl_eventd_pid();

	if (rtk_eventd_pid==0)
	{
		printk("%s:%d, report wifi link status failed, pid=%d\n",__FUNCTION__,__LINE__, rtk_eventd_pid);
		return -1;
	}

	memcpy(data, addr, MAC_ADDR_LEN);
	data_len += MAC_ADDR_LEN;

	if(len > WLAN_INFO_LEN){
		printk("%s:%d, info len longer than %d!\n",__FUNCTION__,__LINE__, WLAN_INFO_LEN);
		return -1;
	}
	memcpy(data+data_len, info, len);
	data_len += len;

	if(strlen(ifname) > IFNAMSIZ){
		printk("%s:%d, ifname[%s] len longer than %d!\n",__FUNCTION__,__LINE__, ifname, IFNAMSIZ);
		name_len = IFNAMSIZ;
	}else{
		name_len = strlen(ifname);
	}
	strncpy(data+data_len, ifname, name_len);
	data_len += name_len;

	data[data_len] = '\0';
	data_len += 1;

	rtk_eventd_genl_send(rtk_eventd_pid, event, NULL, data, data_len);

	return 0;
}

void genl_wlan_IndicateEvent(_adapter *padapter, unsigned char event, void *data)
{
	unsigned char wlanid=0;
	unsigned char mac[ETH_ALEN]={0};
	unsigned char info[WLAN_INFO_LEN]={0};
	int eventid=WIFI_EVENT_WITH_INFO;
	unsigned char *data_ptr;
	int len=0;
	struct net_device *pnetdev = padapter->pnetdev;

	switch (event)
	{
#ifdef CONFIG_CTC_FEATURE
		case EVENT_CTC_ROAMING_STA_RSSI_ALARM:
		{
			struct sta_info *psta = (struct sta_info *)data;
			CTC_ROAMING_STA_RSSI_ALARM sta_rssi_alarm;
			memset(&sta_rssi_alarm, 0, sizeof(CTC_ROAMING_STA_RSSI_ALARM));
			sta_rssi_alarm.event_id = event;
			sta_rssi_alarm.more_event = 1;
			_rtw_memcpy(sta_rssi_alarm.hwaddr, psta->phl_sta->mac_addr, ETH_ALEN);
			_rtw_memcpy(mac, psta->phl_sta->mac_addr, ETH_ALEN);
			sta_rssi_alarm.rssi = rtw_phl_get_sta_rssi(psta->phl_sta);
			sta_rssi_alarm.type = (psta->roaming_indicate == 1)?0:1;
			RTW_INFO("[CTC Roaming]STA "MAC_FMT" rssi alarm, type = %d, rssi = %d\n",
							MAC_ARG(sta_rssi_alarm.hwaddr),
							sta_rssi_alarm.type,
							sta_rssi_alarm.rssi);

			data_ptr = info;
			_rtw_memcpy(data_ptr, &sta_rssi_alarm.event_id, 1);
			data_ptr += 1;
			len += 1;
			_rtw_memcpy(data_ptr, &sta_rssi_alarm.more_event, 1);
			data_ptr += 1;
			len += 1;
			_rtw_memcpy(data_ptr, sta_rssi_alarm.hwaddr, ETH_ALEN);
			data_ptr += ETH_ALEN;
			len += ETH_ALEN;
			_rtw_memcpy(data_ptr, &sta_rssi_alarm.type, 1);
			data_ptr += 1;
			len += 1;
			_rtw_memcpy(data_ptr, &sta_rssi_alarm.rssi, 1);
			data_ptr += 1;
			len += 1;
			break;
		}
		case EVENT_CTC_ROAMING_BSS_TRANSMIT_RESP:
		{
			unsigned char* tmp = (unsigned char*)data;
			CTC_ROAMING_BSS_TRANSMIT_RESP btm_resp;
			memset(&btm_resp, 0, sizeof(CTC_ROAMING_BSS_TRANSMIT_RESP));

			btm_resp.event_id = event;
			btm_resp.more_event = 1;
			btm_resp.result_code = tmp[0];
			_rtw_memcpy(btm_resp.hwaddr, tmp + 1, ETH_ALEN);
			_rtw_memcpy(mac, tmp + 1, ETH_ALEN);

			data_ptr = info;
			_rtw_memcpy(data_ptr, &btm_resp.event_id, 1);
			data_ptr += 1;
			len += 1;
			_rtw_memcpy(data_ptr, &btm_resp.more_event, 1);
			data_ptr += 1;
			len += 1;
			_rtw_memcpy(data_ptr, btm_resp.hwaddr, ETH_ALEN);
			data_ptr += ETH_ALEN;
			len += ETH_ALEN;
			_rtw_memcpy(data_ptr, &btm_resp.result_code, 1);
			data_ptr += 1;
			len += 1;
			break;
		}
#endif /* CONFIGCTC_FEATURE */
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
		case EVENT_PROBE_RX_VSIE:
		{   /*tmp = [mac + pvsie(eid + len + oui + value)]*/
			unsigned char* tmp = (unsigned char*)data;
			PROBE_VSIE_RECORD probe_vsie_record;
			memset(&probe_vsie_record, 0, sizeof(PROBE_VSIE_RECORD));
			probe_vsie_record.IsMoreEvent = 1;
			probe_vsie_record.EventId = event;
			_rtw_memcpy(probe_vsie_record.hwaddr, tmp, ETH_ALEN);
			_rtw_memcpy(mac, tmp, ETH_ALEN);
			probe_vsie_record.data_len = tmp[ETH_ALEN+1];
			memcpy(probe_vsie_record.ie_data, tmp + ETH_ALEN + 2, probe_vsie_record.data_len);
			RTW_INFO("[PROBE_VS_IE] Probe_req %02X%02X%02X%02X%02X%02X, OUI %02X-%02X-%02X, len %d\n",
					probe_vsie_record.hwaddr[0],probe_vsie_record.hwaddr[1],probe_vsie_record.hwaddr[2],
					probe_vsie_record.hwaddr[3],probe_vsie_record.hwaddr[4],probe_vsie_record.hwaddr[5],
					probe_vsie_record.ie_data[0],probe_vsie_record.ie_data[1],probe_vsie_record.ie_data[2],
					probe_vsie_record.data_len);

			data_ptr = info;

			_rtw_memcpy(data_ptr, &probe_vsie_record.EventId, 1);
			data_ptr += 1;
			len += 1;
			_rtw_memcpy(data_ptr, &probe_vsie_record.IsMoreEvent, 1);
			data_ptr += 1;
			len += 1;
			_rtw_memcpy(data_ptr, probe_vsie_record.hwaddr, ETH_ALEN);
			data_ptr += ETH_ALEN;
			len += ETH_ALEN;
			_rtw_memcpy(data_ptr, &probe_vsie_record.data_len, 1);
			data_ptr += 1;
			len += 1;
			_rtw_memcpy(data_ptr, probe_vsie_record.ie_data, probe_vsie_record.data_len);
			len += probe_vsie_record.data_len;
			break;
		}
#endif /* CONFIG_APPEND_VENDOR_IE_ENABLE */
		default:
			RTW_ERR("Not supported event : %d\n", event);
			break;
	}

	rtk_wlan_with_info_genl_indicate(pnetdev->name, eventid, mac, info, len);
}
#endif

#ifdef RTL_LINK_ROAMING_REPORT
int construct_rtl_link_roaming_sta(_adapter *padapter, ROAMING_INFO_RPT *proam_info)
{
	struct sta_info *psta = NULL, *pfirsta=NULL;
    unsigned int    retry_ratio=0, fail_ratio=0, i;
    unsigned char   roaming_num = 0, rssi;
	_list	*plist, *phead;
	u32 	tx_pkts_cur=0, retry_pkts_cur=0, fail_pkts_cur=0;
	struct registry_priv *pregpriv = &padapter->registrypriv;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct net_device *ndev = padapter->pnetdev;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		if (psta && psta->phl_sta && !is_broadcast_mac_addr(psta->phl_sta->mac_addr)) {
			rssi = rtw_phl_get_sta_rssi(psta->phl_sta);

			// please see collect_sta_traffic_statistics()
			tx_pkts_cur = psta->sta_stats.tx_data_pkts_cur;
			retry_pkts_cur = psta->sta_stats.tx_data_retry_pkts_cur;
			fail_pkts_cur = psta->sta_stats.tx_data_fail_pkts_cur;
			if(tx_pkts_cur){
				retry_ratio = (retry_pkts_cur * 100)/tx_pkts_cur;
				fail_ratio = (fail_pkts_cur * 100)/tx_pkts_cur;
			}

			RTW_WARN("(%s)(%s)sta:%pm, rssi:%d, fail_ratio:%d, retry_ratio:%d\n", __FUNCTION__,
				padapter->pnetdev->name,psta->phl_sta->mac_addr, rssi, fail_ratio, retry_ratio);

			if(rssi >= pregpriv->wifi_mib.RSSIThreshold &&
				fail_ratio <= pregpriv->wifi_mib.fail_ratio &&
				retry_ratio <= pregpriv->wifi_mib.retry_ratio)
				continue;

			_rtw_memcpy(proam_info->sta_info[roaming_num].addr,&(psta->phl_sta->mac_addr), MAC_ADDR_LEN);
			proam_info->sta_info[roaming_num].rssi = rssi;
			proam_info->sta_info[roaming_num].rxRate = psta->cur_rx_data_rate;
			proam_info->sta_info[roaming_num].txRate = psta->cur_tx_data_rate;
			proam_info->sta_info[roaming_num].channel = pmlmeext->cur_channel;
			proam_info->sta_info[roaming_num].bw = pmlmeext->cur_bwmode;
			proam_info->sta_info[roaming_num].fail_ratio = fail_ratio;
			proam_info->sta_info[roaming_num].retry_ratio = retry_ratio;
			proam_info->sta_info[roaming_num].tx_pkts = tx_pkts_cur;
			proam_info->sta_info[roaming_num].low_qos = 0; //TODO
			proam_info->sta_info[roaming_num].used = 1;

			roaming_num++;
		}
	}

	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	proam_info->sta_num = roaming_num;

	return roaming_num;
}

int construct_rtl_link_roaming_info(_adapter *padapter, unsigned char *data)
{
	int offset=0, len=0, i;
	ROAMING_INFO_RPT roam_info;
	ONE_STA one_sta;
	struct registry_priv *pregpriv = &padapter->registrypriv;

	RTW_WARN("(%s)(%s)roaming_switch:%d, rssi_th:%d\n", __FUNCTION__,padapter->pnetdev->name,
		pregpriv->wifi_mib.roaming_switch, pregpriv->wifi_mib.RSSIThreshold);

	if(!pregpriv->wifi_mib.roaming_switch)
		goto exit;

	_rtw_memset(&roam_info, 0, sizeof(ROAMING_INFO_RPT));
	construct_rtl_link_roaming_sta(padapter, &roam_info);

	RTW_WARN("(%s)(%s)sta_num:%d\n", __FUNCTION__,padapter->pnetdev->name,roam_info.sta_num);

	if(roam_info.sta_num==0)
		goto exit;

	roam_info.eventID = EVENT_ROAMING_INFO_RPT;
	/* eventID must be first */
	len = sizeof(roam_info.eventID);
	_rtw_memcpy(data+offset,&(roam_info.eventID),len);
	offset+=len;

	/* sta num */
	len = sizeof(roam_info.sta_num);
	_rtw_memcpy(data+offset,&(roam_info.sta_num),len);
	offset+=len;

	len = sizeof(ONE_STA) * (roam_info.sta_num);
	_rtw_memcpy(data+offset,&(roam_info.sta_info[0]),len);
	offset+=len;

exit:
	return offset;
}
#endif

#ifdef MONITOR_UNASSOC_STA
static int search_sta_info(_adapter *padapter, u8* macaddr)
{
	int i;
	int INVALID = -2, NOT_FOUND = -1;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	if(macaddr == NULL)
		return INVALID;

	for(i=0; i<NUM_MONITOR; i++)
	{
		if(pmlmeext->monitor_sta_info.monitor_sta_ent[i].valid == 1)
		{
			if(_rtw_memcmp(pmlmeext->monitor_sta_info.monitor_sta_ent[i].mac,macaddr,ETH_ALEN)== _TRUE)
				return i; //found, return index
			else
				continue;
		}
		else
		{
			break;
		}
	}

	return NOT_FOUND;//not found
}

static int is_from_ap(_adapter *padapter, union recv_frame *precv_frame)
{
	int ret=0;
	u8 type, subtype, to_fr_ds;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	u8 *prx_data = precv_frame->u.hdr.rx_data;

	type =  GetFrameType(prx_data);
	subtype = get_frame_sub_type(prx_data);
	to_fr_ds = pattrib->to_fr_ds;
	switch (type) {
		case WIFI_DATA_TYPE:
			if(to_fr_ds & 0x01) //0x1 or 0x11
				ret = 1;
			else
				ret = 0;
			break;
		case WIFI_CTRL_TYPE:
		case WIFI_MGT_TYPE:
			 if(subtype==WIFI_BEACON || subtype==WIFI_PROBERSP || subtype==WIFI_ASSOCRSP || subtype==WIFI_REASSOCRSP)
				ret = 1;
			 else
				ret = 0;
			break;
		default:
			ret = 0;
			break;
	}

	return ret;
}

static __inline__ u8 rtk_monitor_cal_rssi_avg(u32 agv, u32 pkt_rssi)
{
	u32 rssi;

	if(agv == 0)
	    return (u8)pkt_rssi;

	rssi = ((agv * 9) + pkt_rssi) / 10;
	if (pkt_rssi > agv)
		rssi++;

	return (u8)rssi;
}

int rtk_monitor_sta_info(_adapter *padapter, union recv_frame *prframe)
{
	int search_result = 0, index = 0, i, tmp_index = 0;
	unsigned long max=0, age=0;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	u8 rssi, to_fr_ds;
	u8 *ta;
	struct net_device *ndev = padapter->pnetdev;

	/*just record tods packet*/
	if(is_from_ap(padapter, prframe))
		return 0;

	rssi = pattrib->rssi;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	ta = get_addr2_ptr(prframe->u.hdr.wlan_hdr);
	#else
	ta = get_addr2_ptr(prframe->u.hdr.rx_data);
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	search_result = search_sta_info(padapter, ta);
	if( search_result>=0 ) // found, update
	{
		index = search_result;

	}else if( search_result==-1 ) //not in the list, insert
	{
		if( NUM_MONITOR==pmlmeext->monitor_sta_info.sta_entry_num )
		{
			for(i=0; i<NUM_MONITOR; ++i)
			{
				if(jiffies/HZ >= pmlmeext->monitor_sta_info.monitor_sta_ent[i].sec)
					age = jiffies/HZ - pmlmeext->monitor_sta_info.monitor_sta_ent[i].sec;
				else
					age = jiffies/HZ + ~(unsigned long)0/HZ - pmlmeext->monitor_sta_info.monitor_sta_ent[i].sec;

				//find the oldest sta to update
				if( age>=max ){
					max = age;
					tmp_index = i;
				}
			}
			index = tmp_index;
		}else{
			index = pmlmeext->monitor_sta_info.sta_entry_num;
			pmlmeext->monitor_sta_info.sta_entry_num++;
		}

		_rtw_memcpy(pmlmeext->monitor_sta_info.monitor_sta_ent[index].mac,ta,ETH_ALEN);
		pmlmeext->monitor_sta_info.monitor_sta_ent[index].valid = 1;
		pmlmeext->monitor_sta_info.monitor_sta_ent[index].isAP = is_from_ap(padapter, prframe);
	}

	if(rssi)
		pmlmeext->monitor_sta_info.monitor_sta_ent[index].rssi = rtk_monitor_cal_rssi_avg(pmlmeext->monitor_sta_info.monitor_sta_ent[index].rssi, rssi);

	pmlmeext->monitor_sta_info.monitor_sta_ent[index].sec = jiffies/HZ;

	return 0;
}

int rtw_construct_unassoc_sta_info(_adapter *padapter, unsigned char *data)
{
	int i, len, sta_num, offset=0;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	MONITOR_STA monitor_sta;
	MONITOR_STA_ENTRY *alloc_buff=NULL;

	/* eventID must be first */
	len = sizeof(pmlmeext->monitor_sta_info.eventID);
	pmlmeext->monitor_sta_info.eventID = EVENT_UNASSOC_STA_RPT;
	_rtw_memcpy(data+offset, &(pmlmeext->monitor_sta_info.eventID), len);
	offset+=len;

	/* sta num */
	sta_num = pmlmeext->monitor_sta_info.sta_entry_num;
	len = sizeof(monitor_sta.sta_entry_num);
	_rtw_memcpy(data+offset, &(sta_num), len);
	offset+=len;

	if(!sta_num){
		offset = 0;
		goto exit;
	}

	/* sta info */
	len = sizeof(MONITOR_STA_ENTRY) * sta_num;
	alloc_buff = (MONITOR_STA_ENTRY *)rtw_zmalloc(len);
	if( !alloc_buff ){
		printk("alloc buff fail(%s)\n", __FUNCTION__);
		offset = 0;
		goto exit;
	}
	_rtw_memcpy(alloc_buff, &(pmlmeext->monitor_sta_info.monitor_sta_ent[0]), len);
	// calc relative time(after last seen)
	for(i=0; i<sta_num; ++i){
		if(jiffies/HZ >= alloc_buff[i].sec)
			alloc_buff[i].sec = jiffies/HZ - alloc_buff[i].sec;
		else
			alloc_buff[i].sec = jiffies/HZ + ~(unsigned long)0/HZ - alloc_buff[i].sec;
	}
	_rtw_memcpy(data+offset, (char *)alloc_buff, len);
	offset+=len;

	rtw_mfree(alloc_buff, len);
exit:
	return offset;
}
#endif

#ifdef RTW_BLOCK_STA_CONNECT
int block_sta_search(_adapter *padapter, u8 *macaddr)
{
	int INVALID = -2, NOT_FOUND=-1, i;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	if(macaddr == NULL)
		return INVALID;

	for(i=0; i<MACID_NUM_SW_LIMIT; i++)
	{
		if(pmlmeext->blockStaExt.block_sta[i].isvalid== 1)
		{
			if(_rtw_memcmp(pmlmeext->blockStaExt.block_sta[i].mac,macaddr,ETH_ALEN)== _TRUE)
				return i;
			else
				continue;
		}
		else
		{
			break;
		}
	}
	return NOT_FOUND;
}

int block_sta_conn_chk(_adapter *padapter, u8 *macaddr)
{
	int ret = 0, index=0;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	index = block_sta_search(padapter, macaddr);
	if((index>=0) && (pmlmeext->blockStaExt.block_sta[index].block_time>0)){
		ret = 1;
	}
	return ret;
}

int block_sta_conn_expire(_adapter *padapter)
{
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	int i, expire_gap=2;

	for(i=0; i<pmlmeext->blockStaExt.sta_entry_num; ++i){
		if(pmlmeext->blockStaExt.block_sta[i].block_time>=expire_gap)
			pmlmeext->blockStaExt.block_sta[i].block_time -= expire_gap;
		if(pmlmeext->blockStaExt.block_sta[i].block_time && pmlmeext->blockStaExt.block_sta[i].block_time<expire_gap)
			pmlmeext->blockStaExt.block_sta[i].block_time=0;
	}
	return 0;
}

int rtw_block_sta_conn_lookup(_adapter *padapter, RTK_WLAN_BLOCK_STA blockSta)
{
	int i, max=0, tmp_index;
	int INVALID = -2, index=-1;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	unsigned long age=0;
	unsigned char macaddr[ETH_ALEN]={0};
	int search_result=0;

	_rtw_memcpy(macaddr, blockSta.mac, ETH_ALEN);
	search_result = block_sta_search(padapter, macaddr);
	if( search_result==-1 ){//not found, insert
		if( MACID_NUM_SW_LIMIT == pmlmeext->blockStaExt.sta_entry_num )
		{
			for(i=0; i<MACID_NUM_SW_LIMIT; ++i)
			{
				if(jiffies/HZ >= pmlmeext->blockStaExt.block_sta[i].age)
					age = jiffies/HZ - pmlmeext->blockStaExt.block_sta[i].age;
				else
					age = jiffies/HZ + ~(unsigned long)0/HZ - pmlmeext->blockStaExt.block_sta[i].age;

				//find the oldest sta to update
				if( age>=max ){
					max = age;
					tmp_index = i;
				}
			}
			index = tmp_index;
		}else{
			index = pmlmeext->blockStaExt.sta_entry_num;
			pmlmeext->blockStaExt.sta_entry_num++;
		}

		_rtw_memcpy(pmlmeext->blockStaExt.block_sta[index].mac,macaddr,ETH_ALEN);
		pmlmeext->blockStaExt.block_sta[index].isvalid = 1;
	}else
	{
		index = search_result;
	}

	pmlmeext->blockStaExt.block_sta[index].age = jiffies/HZ;
	pmlmeext->blockStaExt.block_sta[index].block_time = blockSta.block_time;

	return 0;
}
#endif

#ifdef SBWC
int sbwc_tx(_adapter *padapter, struct sk_buff *skb)
{
	unsigned int tx_count, tx_limit;
	struct sta_info *pstat = NULL;
	u8 da[6];

	if (!MLME_IS_AP(padapter))
		return TX_CONTINUE;

	_rtw_memcpy(da, skb->data, 6);
	pstat = rtw_get_stainfo(&padapter->stapriv, da);

	if (!pstat || !(pstat->SBWC_mode & SBWC_MODE_LIMIT_STA_TX))
		return TX_CONTINUE;

	tx_count = (pstat->SBWC_tx_count + skb->len);
	tx_limit = pstat->SBWC_tx_limit_byte;

	if (tx_count > tx_limit) {
		if (pstat->SBWC_consuming_q) {
			return SBWC_FREE_SKB;
		} else {
			if (skb_queue_len(&pstat->SBWC_txq) < NUM_TXPKT_QUEUE) {
				skb_queue_tail(&pstat->SBWC_txq, skb);
				return SBWC_QUEUE_SKB;
			} else {
				return SBWC_FREE_SKB;
			}
		}
	} else {
		if (skb_queue_len(&pstat->SBWC_txq) && !pstat->SBWC_consuming_q) {
			if (skb_queue_len(&pstat->SBWC_txq) < NUM_TXPKT_QUEUE) {
				skb_queue_tail(&pstat->SBWC_txq, skb);
				return SBWC_QUEUE_SKB;
			} else {
				return SBWC_FREE_SKB;
			}
		} else {
			pstat->SBWC_tx_count = tx_count;
			return TX_CONTINUE;
		}
	}
}

int sbwc_rx(_adapter *adapter, union recv_frame *prframe)
{
		unsigned int rx_count;
		unsigned int rx_limit;
		struct sta_info *pstat = prframe->u.hdr.psta;
		struct sk_buff *skb = prframe->u.hdr.pkt;

		if (!pstat || !(pstat->SBWC_mode & SBWC_MODE_LIMIT_STA_RX))
			return CORE_RX_CONTINUE;

		rx_count = (pstat->SBWC_rx_count + skb->len);
		rx_limit = pstat->SBWC_rx_limit_byte;

		if (rx_count > rx_limit) {
			if (pstat->SBWC_consuming_q) {
				return CORE_RX_DROP;
			} else {
				if (skb_queue_len(&pstat->SBWC_rxq) < NUM_TXPKT_QUEUE) {
					put_unaligned((unsigned long)prframe, (unsigned long *)&(skb->cb[_SKB_CB_BWC_]));
					skb_queue_tail(&pstat->SBWC_rxq, skb);
					return CORE_RX_ENQUEUED;
				} else {
					return CORE_RX_DROP;
				}
			}
		} else {
			if (skb_queue_len(&pstat->SBWC_rxq) && !pstat->SBWC_consuming_q) {
				if (skb_queue_len(&pstat->SBWC_rxq) < NUM_TXPKT_QUEUE) {
					put_unaligned((unsigned long)prframe, (unsigned long *)&(skb->cb[_SKB_CB_BWC_]));
					skb_queue_tail(&pstat->SBWC_rxq, skb);
					return CORE_RX_ENQUEUED;
				} else {
					return CORE_RX_DROP;
				}
			} else {
				pstat->SBWC_rx_count = rx_count;
				return CORE_RX_CONTINUE;
			}
		}
}

void sbwc_timer_handler(void *context){

	_adapter *padapter = (_adapter *)context;
	struct sta_info *psta;
	struct sta_priv *pstapriv = &padapter->stapriv;
	int i, j;
	_list	*plist, *phead;
	struct sk_buff *pskb;
	SBWC_MODE mode;
	union recv_frame *prframe = NULL;

	_adapter *primary_adapter = dvobj_get_primary_adapter(padapter->dvobj);
	struct recv_priv	*precvpriv = &(primary_adapter->recvpriv);
	_queue *pfree_recv_queue = &(precvpriv->free_recv_queue);

	if (!netif_running(padapter->pnetdev))
		return;

	//_rtw_spinlock_bh(&pstapriv->sta_hash_lock);

	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);

			mode = SBWC_MODE_DISABLE;
			if (psta->SBWC_tx_limit)
				mode |= SBWC_MODE_LIMIT_STA_TX;
			if (psta->SBWC_rx_limit)
				mode |= SBWC_MODE_LIMIT_STA_RX;
			if (mode == SBWC_MODE_DISABLE)
				continue;

			psta->SBWC_consuming_q = 1;

			// clear bandwidth control counter
			RTW_DBG("======>%02X:%02X:%02X:%02X:%02X:%02X, SBWC_tx_count[%d], SBWC_txqlen[%d], SBWC_rx_count[%d], SBWC_rxqlen[%d]\n",
				psta->phl_sta->mac_addr[0], psta->phl_sta->mac_addr[1], psta->phl_sta->mac_addr[2],
				psta->phl_sta->mac_addr[3], psta->phl_sta->mac_addr[4], psta->phl_sta->mac_addr[5],
				psta->SBWC_tx_count,skb_queue_len(&psta->SBWC_txq), psta->SBWC_rx_count,skb_queue_len(&psta->SBWC_rxq));
			psta->SBWC_tx_count = 0;
			psta->SBWC_rx_count = 0;

			// consume Tx queue
			while(skb_queue_len(&psta->SBWC_txq)) {
				pskb = skb_dequeue(&psta->SBWC_txq);

				if (!pskb)
					break;
				//rtw_core_tx(padapter, &pskb, NULL);
				rtw_os_tx(pskb, padapter->pnetdev);
			}

			// consume Rx queue
			while(skb_queue_len(&psta->SBWC_rxq)) {
				pskb = skb_dequeue(&psta->SBWC_rxq);
				if (!pskb)
					break;
				prframe = (union recv_frame *)get_unaligned((unsigned long *)&(pskb->cb[_SKB_CB_BWC_]));
				if(rtw_core_rx_data_post_process(padapter, prframe) != CORE_RX_DONE)
					rtw_free_recvframe(prframe, pfree_recv_queue);
				//rtw_netif_rx(padapter->pnetdev, pskb);
			}

			psta->SBWC_consuming_q = 0;
			psta->SBWC_mode = mode;

		}

	}

	//_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

	if (netif_running(padapter->pnetdev))
		_set_timer(&padapter->SBWC_timer, SBWC_PERIOD);
}
#endif

#ifdef RTW_STA_BWC
int sta_bwc_tx(_adapter *padapter, struct sk_buff *skb)
{
	unsigned int tx_count, tx_limit;
	struct sta_info *pstat = NULL;
	u8 da[6];

	if (!MLME_IS_AP(padapter))
		return STA_BWC_TX_CONTINUE;

	_rtw_memcpy(da, skb->data, 6);
	pstat = rtw_get_stainfo(&padapter->stapriv, da);

	if (!pstat || (pstat->sta_bwc_tx_limit == 0))
		return STA_BWC_TX_CONTINUE;

	tx_count = (pstat->sta_bwc_tx_cnt + skb->len);
	tx_limit = pstat->sta_bwc_tx_limit_byte;

	if (tx_count > tx_limit) {
		if (pstat->sta_bwc_consuming_q) {
			return STA_BWC_FREE_SKB;
		} else {
			if (skb_queue_len(&pstat->sta_bwc_txq) < NUM_TXPKT_QUEUE) {
				skb_queue_tail(&pstat->sta_bwc_txq, skb);
				return STA_BWC_QUEUE_SKB;
			} else {
				return STA_BWC_FREE_SKB;
			}
		}
	} else {
		if (skb_queue_len(&pstat->sta_bwc_txq) && !pstat->sta_bwc_consuming_q) {
			if (skb_queue_len(&pstat->sta_bwc_txq) < NUM_TXPKT_QUEUE) {
				skb_queue_tail(&pstat->sta_bwc_txq, skb);
				return STA_BWC_QUEUE_SKB;
			} else {
				return STA_BWC_FREE_SKB;
			}
		} else {
			pstat->sta_bwc_tx_cnt = tx_count;
			return STA_BWC_TX_CONTINUE;
		}
	}
}

void sta_bwc_timer_handler(void *context){

	_adapter *padapter = (_adapter *)context;
	struct sta_info *psta;
	struct sta_priv *pstapriv = &padapter->stapriv;
	int i, j;
	_list	*plist, *phead;
	struct sk_buff *pskb;

	if (!netif_running(padapter->pnetdev))
		return;

	//_rtw_spinlock_bh(&pstapriv->sta_hash_lock);

	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);

			if (psta->sta_bwc_tx_limit == 0)
				continue;

			psta->sta_bwc_consuming_q = 1;

			// clear bandwidth control counter
			RTW_DBG("======>%02X:%02X:%02X:%02X:%02X:%02X, sta_bwc_tx_cnt[%d], sta_bwc_txqlen[%d]\n",
				psta->phl_sta->mac_addr[0], psta->phl_sta->mac_addr[1], psta->phl_sta->mac_addr[2],
				psta->phl_sta->mac_addr[3], psta->phl_sta->mac_addr[4], psta->phl_sta->mac_addr[5],
				psta->sta_bwc_tx_cnt,skb_queue_len(&psta->sta_bwc_txq));

			psta->sta_bwc_tx_cnt = 0;

			// consume Tx queue
			while(skb_queue_len(&psta->sta_bwc_txq)) {
				pskb = skb_dequeue(&psta->sta_bwc_txq);

				if (!pskb)
					break;
				//rtw_core_tx(padapter, &pskb, NULL);
				rtw_os_tx(pskb, padapter->pnetdev);
			}

			psta->sta_bwc_consuming_q = 0;
		}

	}

	//_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

	if((padapter->last_asoc_cnt != padapter->stapriv.asoc_list_cnt) && (padapter->txduty))
		g6_decide_limit_tp(padapter, padapter->txduty_level);

	if (netif_running(padapter->pnetdev))
		_set_timer(&padapter->sta_bwc_timer, STA_BWC_PERIOD);
}
#endif

#ifdef GBWC
int gbwc_tx(_adapter *padapter, struct sk_buff *skb, struct sta_info *pstat)
{
	int ret = 0;

	if (rtw_get_intf_type(padapter) != RTW_HCI_PCIE)
		return CORE_RX_CONTINUE;

	if(padapter->registrypriv.wifi_mib.gbwcmode && padapter->GBWC_timer_alive)
	{
		if(((padapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_MAC_INNER) && pstat->GBWC_in_group) ||
			((padapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_MAC_OUTTER) && !pstat->GBWC_in_group) ||
			(padapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_IF_TX) ||
			(padapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_IF_TRX))
		{
			if((padapter->GBWC_tx_count + skb->len) > ((padapter->registrypriv.wifi_mib.gbwcthrd_tx * 1024 / 8) / (1000 / GBWC_PERIOD)))
			{
				if(padapter->GBWC_consuming_Q)
				{
					return GBWC_DROP_SKB;
				}
				else
				{
					if (skb_queue_len(&padapter->GBWC_tx_queue) < NUM_TXPKT_QUEUE)
					{
						if(pstat)
							put_unaligned((unsigned long)pstat, (unsigned long *)&(skb->cb[_SKB_CB_BWC_]));
						else
							skb->cb[_SKB_CB_BWC_] = 0;
						skb_queue_tail(&padapter->GBWC_tx_queue, skb);
						return GBWC_QUEUE_SKB;
					}
					else
					{
						return GBWC_DROP_SKB;
					}
				}
			}
			else
			{
				if (skb_queue_len(&padapter->GBWC_tx_queue) && !padapter->GBWC_consuming_Q)
				{
					if (skb_queue_len(&padapter->GBWC_tx_queue) < NUM_TXPKT_QUEUE)
					{
						if(pstat)
							put_unaligned((unsigned long)pstat, (unsigned long *)&(skb->cb[_SKB_CB_BWC_]));
						else
							skb->cb[_SKB_CB_BWC_] = 0;
						skb_queue_tail(&padapter->GBWC_tx_queue, skb);
						return GBWC_QUEUE_SKB;
					}
					else
					{
						return GBWC_DROP_SKB;
					}
				}
				else
				{
					padapter->GBWC_tx_count += skb->len;
					return GBWC_TX_CONTINUE;
				}
			}
		}
	}
	return GBWC_TX_CONTINUE;
}

int gbwc_rx(_adapter *padapter, union recv_frame *prframe)
{
	struct sta_info *pstat = prframe->u.hdr.psta;
	struct sk_buff *skb = prframe->u.hdr.pkt;
	int ret = 0;

	if (rtw_get_intf_type(padapter) != RTW_HCI_PCIE)
		return CORE_RX_CONTINUE;

	if(padapter->registrypriv.wifi_mib.gbwcmode && padapter->GBWC_timer_alive)
	{
		if(((padapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_MAC_INNER) && pstat->GBWC_in_group) ||
			((padapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_MAC_OUTTER) && !pstat->GBWC_in_group) ||
			(padapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_IF_RX) ||
			(padapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_IF_TRX))
		{
			if((padapter->GBWC_rx_count + skb->len) > ((padapter->registrypriv.wifi_mib.gbwcthrd_rx * 1024 / 8) / (1000 / GBWC_PERIOD)))
			{
				if(padapter->GBWC_consuming_Q)
				{
					return CORE_RX_DROP;
				}
				else
				{
					if (skb_queue_len(&padapter->GBWC_rx_queue) < NUM_TXPKT_QUEUE)
					{
						put_unaligned((unsigned long)prframe, (unsigned long *)&(skb->cb[_SKB_CB_BWC_]));
						skb_queue_tail(&padapter->GBWC_rx_queue, skb);
						return CORE_RX_ENQUEUED;
					}
					else
					{
						return CORE_RX_DROP;
					}
				}
			}
			else
			{
				if (skb_queue_len(&padapter->GBWC_rx_queue) && !padapter->GBWC_consuming_Q)
				{
					if (skb_queue_len(&padapter->GBWC_rx_queue) < NUM_TXPKT_QUEUE)
					{
						put_unaligned((unsigned long)prframe, (unsigned long *)&(skb->cb[_SKB_CB_BWC_]));
						skb_queue_tail(&padapter->GBWC_rx_queue, skb);
						return CORE_RX_ENQUEUED;
					}
					else
					{
						return CORE_RX_DROP;
					}
				}
				else
				{
					padapter->GBWC_rx_count += skb->len;
					return CORE_RX_CONTINUE;
				}
			}
		}
	}
	return GBWC_TX_CONTINUE;
}

void gbwc_timer_handler(void *context){

	_adapter *padapter = (_adapter *)context;
	struct sk_buff *pskb;
	_adapter *primary_adapter = dvobj_get_primary_adapter(padapter->dvobj);
	struct recv_priv	*precvpriv = &(primary_adapter->recvpriv);
	_queue *pfree_recv_queue = &(precvpriv->free_recv_queue);
	union recv_frame *prframe = NULL;
	struct sta_info *psta = NULL;

	if (!netif_running(padapter->pnetdev))
		return;

	if(padapter->registrypriv.wifi_mib.gbwcmode && padapter->GBWC_timer_alive)
	{
		padapter->GBWC_consuming_Q = 1;

		padapter->GBWC_rx_count = 0;
		padapter->GBWC_tx_count = 0;

		while(skb_queue_len(&padapter->GBWC_tx_queue)) {
			pskb = skb_dequeue(&padapter->GBWC_tx_queue);

			if (!pskb)
				break;
			psta = (struct sta_info *)get_unaligned((unsigned long *)&(pskb->cb[_SKB_CB_BWC_]));
			rtw_core_tx(padapter, &pskb, psta);
			//rtw_os_tx(pskb, padapter->pnetdev);
		}
		while(skb_queue_len(&padapter->GBWC_rx_queue)) {
			pskb = skb_dequeue(&padapter->GBWC_rx_queue);
			if (!pskb)
				break;
			prframe = (union recv_frame *)get_unaligned((unsigned long *)&(pskb->cb[_SKB_CB_BWC_]));
			if(rtw_core_rx_data_post_process(padapter, prframe) != CORE_RX_DONE)
				rtw_free_recvframe(prframe, pfree_recv_queue);
			//rtw_netif_rx(padapter->pnetdev, pskb);
		}

		padapter->GBWC_consuming_Q = 0;
	}

	if (netif_running(padapter->pnetdev))
	{
		_set_timer(&padapter->GBWC_timer, GBWC_PERIOD);
	}

	if(padapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_DISABLE)
	{
		padapter->GBWC_timer_alive = 0;
	}
	else
	{
		padapter->GBWC_timer_alive = 1;
	}

	return;
}
#endif

#ifdef CONFIG_RTW_MULTI_DEV_MULTI_BAND
u8 rtw_get_phyband_on_dev(_adapter *padapter)
{
	u8 band = BAND_ON_24G;

#ifdef CONFIG_2G_ON_PCIE_SLOT0
	if (padapter->dvobj->dev_id == IFACE_ID0)
		band = BAND_ON_24G;
	else if (padapter->dvobj->dev_id == IFACE_ID1)
		band = BAND_ON_5G;
#else
	if (padapter->dvobj->dev_id == IFACE_ID0)
		band = BAND_ON_5G;
	else if (padapter->dvobj->dev_id == IFACE_ID1)
		band = BAND_ON_24G;
#endif /* CONFIG_2G_ON_PCIE_SLOT0 */

	return band;
}
#endif /* CONFIG_RTW_MULTI_DEV_MULTI_BAND */

#if defined(CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT)
void rtw_crossband_update_status(_adapter *padapter)
{
	_adapter *vxd_padapter = NULL, *tmp_padapter = NULL;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct sta_info *crossband_psta = NULL;
	u16 ch_utilization;
	int i;

	for(i = 1; i < CONFIG_IFACE_NUMBER; i++)
	{
		tmp_padapter = dvobj->padapters[i];
		if(rtw_is_adapter_up(tmp_padapter) && MLME_IS_STA(tmp_padapter) && !is_primary_adapter(tmp_padapter))
		{
			vxd_padapter = tmp_padapter;
			break;
		}
	}
	if(vxd_padapter)
	{
		if(vxd_padapter->mlmeextpriv.mlmext_info.state & WIFI_FW_ASSOC_SUCCESS)
		{
			crossband_psta = rtw_get_stainfo(&vxd_padapter->stapriv, vxd_padapter->mlmepriv.cur_network.network.MacAddress);
			if(crossband_psta)
			{
				padapter->crossband.metric_log.rssi_metric = (padapter->crossband.metric_log.rssi_metric * 7 + rtw_phl_get_sta_rssi(crossband_psta->phl_sta) * 3)/10;
				padapter->registrypriv.wifi_mib.crossband_assoc = 1;
			}
			else
			{
				padapter->crossband.metric_log.rssi_metric = (padapter->crossband.metric_log.rssi_metric * 7)/10;
				padapter->registrypriv.wifi_mib.crossband_assoc = 0;
			}

#ifdef CONFIG_CROSSBAND_REPEATER_SUPPORT_WIFI5_WIFI6
			if(padapter->registrypriv.wifi_mib.crossband_assoc == 1 && crossband_check_assoc(CROSSBAND_5G))
#else
			if(padapter->registrypriv.wifi_mib.crossband_assoc == 1 && vxd_padapter->crossband.crossband_vxd_sc
				&& vxd_padapter->crossband.crossband_vxd_sc->crossband.primary_sc->registrypriv.wifi_mib.crossband_assoc == 1)
#endif
			{
				padapter->registrypriv.wifi_mib.crossband_pathReady = 1;
			}
			else
			{
				padapter->registrypriv.wifi_mib.crossband_pathReady = 0;
			}
			padapter->crossband.metric_log.noise_metric = 0;
			ch_utilization = rtw_get_ch_utilization(padapter);
			padapter->crossband.metric_log.cu_metric = (padapter->crossband.metric_log.cu_metric * 7 + ch_utilization * 3)/10;
		}
	}
	else
	{
		padapter->crossband.metric_log.rssi_metric = (padapter->crossband.metric_log.rssi_metric * 7)/10;
		padapter->registrypriv.wifi_mib.crossband_assoc = 0;
		padapter->registrypriv.wifi_mib.crossband_pathReady = 0;
		padapter->crossband.metric_log.noise_metric = 0;
		padapter->crossband.metric_log.cu_metric = (padapter->crossband.metric_log.cu_metric * 7)/10;
		padapter->registrypriv.wifi_mib.crossband_prefer = 0;
	}
	return;
}

int rtw_crossband_rx_check(_adapter *padapter, struct sk_buff *pkt, union recv_frame *rframe)
{
	_adapter *primary_adapter = NULL;
	struct ethhdr *ehdr = NULL;

	if (pkt) {
		ehdr = (struct ethhdr *)pkt->data;
		if (ehdr == NULL) {
			RTW_DBG("ehdr is NULL!\n");
			return -1;
		}
	}
	else
		return -1;

	primary_adapter = padapter->crossband.primary_sc;
#ifdef CONFIG_CROSSBAND_REPEATER_SUPPORT_WIFI5_WIFI6
	if(MacAddr_isBcst(ehdr->h_dest))
	{
		if(rtw_is_adapter_up(primary_adapter)
			&& primary_adapter->registrypriv.wifi_mib.crossband_enable
			&& primary_adapter->registrypriv.wifi_mib.crossband_pathReady
			&& !primary_adapter->registrypriv.wifi_mib.crossband_prefer)
		{
			RTW_DBG("RX DROP: Drop broadcast on non prefer band!\n");
			DBG_COUNTER(padapter->rx_logs.os_indicate_err);
			return -1;
		}
	}

	if(rtw_is_adapter_up(primary_adapter))
	{
		if(crossband_check_loopback(rframe->u.hdr.attrib.src, CROSSBAND_5G) || EQ_MAC_ADDR(rframe->u.hdr.attrib.src, primary_adapter->phl_role->mac_addr))
		{
			RTW_DBG("RX DROP: Drop loopback packet by crossband!\n");
			DBG_COUNTER(padapter->rx_logs.os_indicate_err);
			return -1;
		}
	}
#else
	if(padapter->crossband.crossband_vxd_sc && padapter->crossband.primary_sc)
	{
		primary_adapter = padapter->crossband.primary_sc;
		if(MacAddr_isBcst(ehdr->h_dest))
		{
			if(rtw_is_adapter_up(primary_adapter)
				&& primary_adapter->registrypriv.wifi_mib.crossband_enable
				&& primary_adapter->registrypriv.wifi_mib.crossband_pathReady
				&& !primary_adapter->registrypriv.wifi_mib.crossband_prefer)
			{
				RTW_DBG("RX DROP: Drop broadcast on non prefer band!\n");
				DBG_COUNTER(padapter->rx_logs.os_indicate_err);
				return -1;
			}
		}

		if(rtw_is_adapter_up(primary_adapter) && rtw_is_adapter_up(padapter->crossband.crossband_vxd_sc))
		{
			if(!memcmp(rframe->u.hdr.attrib.src, padapter->crossband.crossband_vxd_sc->phl_role->mac_addr, ETH_ALEN)
				|| !memcmp(rframe->u.hdr.attrib.src, primary_adapter->phl_role->mac_addr, ETH_ALEN))
			{
				RTW_DBG("RX DROP: Drop loopback packet by crossband!\n");
				DBG_COUNTER(padapter->rx_logs.os_indicate_err);
				return -1;
			}
		}
	}
#endif

	return 0;
}

int rtw_crossband_tx_check(_adapter *padapter, struct sk_buff *pkt)
{
	_adapter *primary_padapter = NULL;

#ifdef CONFIG_CROSSBAND_REPEATER_SUPPORT_WIFI5_WIFI6
	if(padapter->crossband.primary_sc)
	{
		primary_padapter = padapter->crossband.primary_sc;
		if(primary_padapter->registrypriv.wifi_mib.crossband_enable
			&& primary_padapter->registrypriv.wifi_mib.crossband_pathReady
			&& !primary_padapter->registrypriv.wifi_mib.crossband_prefer
			&& crossband_check_prefer(CROSSBAND_5G))
		{
			if(crossband_dev_xmit(pkt, CROSSBAND_5G) == 0)
			{
				return 1;
			}
		}
	}
#else
	if(padapter->crossband.primary_sc && padapter->crossband.crossband_vxd_sc)
	{
		primary_padapter = padapter->crossband.primary_sc;
		if(primary_padapter->registrypriv.wifi_mib.crossband_enable
			&& primary_padapter->registrypriv.wifi_mib.crossband_pathReady
			&& !primary_padapter->registrypriv.wifi_mib.crossband_prefer
			&& padapter->crossband.crossband_vxd_sc->crossband.primary_sc->registrypriv.wifi_mib.crossband_prefer)
		{
			if(rtw_is_adapter_up(padapter->crossband.crossband_vxd_sc) == _TRUE)
			{
				return 1;
			}
		}
	}
#endif
	return 0;
}

#if defined(CONFIG_CROSSBAND_REPEATER_SUPPORT_WIFI5_WIFI6)
int rtw_crossband_get_dev_status(struct net_device *dev, struct crossband_dev_status *dev_status)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	_adapter *primary_padapter = NULL;

	if(padapter && padapter->crossband.primary_sc)
	{
		primary_padapter = padapter->crossband.primary_sc;
		dev_status->crossband_assoc = primary_padapter->registrypriv.wifi_mib.crossband_assoc;
		dev_status->crossband_pathReady = primary_padapter->registrypriv.wifi_mib.crossband_pathReady;
		dev_status->crossband_prefer = primary_padapter->registrypriv.wifi_mib.crossband_prefer;
	}
	else
	{
		dev_status->crossband_assoc = 0;
		dev_status->crossband_pathReady = 0;
		dev_status->crossband_prefer = 0;
	}

	return 0;
}
#endif
#endif

void rtw_parse_vendor_info(struct sta_info *psta, u8 *start, sint left)
{
	u8 *mac_addr = psta->phl_sta->mac_addr;
	u8 i;
	u8 *pos = start;
	sint _left = left;
	u8 *oui;
	sint len;

	psta->vendor = HT_IOT_PEER_UNKNOWN;

	/* match vendor by ie */
	do {
		pos = rtw_get_ie(pos, WLAN_EID_VENDOR_SPECIFIC, &len, _left);

		if (!pos)
			break;

		oui = pos + 2;

		for (i = 0; i < BROADCOM_OUI_NUM; i++) {
			if (_rtw_memcmp(oui, BROADCOM_OUI[i], WLAN_IEEE_OUI_LEN) == _TRUE) {
				psta->vendor = HT_IOT_PEER_BROADCOM;
				return;
			}
		}

		pos = pos + len + 2;
		_left = left - (pos - start);
	} while (1);

	/* match vendor by mac address */
	for (i = 0; i < APPLE_OUI_NUM; i++) {
		if (_rtw_memcmp(mac_addr, APPLE_OUI_G6[i], WLAN_IEEE_OUI_LEN) == _TRUE) {
			psta->vendor = HT_IOT_PEER_APPLE;
			return;
		}
	}

	for (i = 0; i < VERIWAVE_OUI_NUM; i++) {
		if (_rtw_memcmp(mac_addr, VERIWAVE_OUI_G6[i], WLAN_IEEE_OUI_LEN) == _TRUE) {
			psta->vendor = HT_IOT_PEER_VERIWAVE;
			return;
		}
	}

	for (i = 0; i < SPIRENT_OUI_NUM; i++) {
		if (_rtw_memcmp(mac_addr, SPIRENT_OUI_G6[i], WLAN_IEEE_OUI_LEN) == _TRUE) {
			psta->vendor = HT_IOT_PEER_SPIRENT;
			return;
		}
	}

	for (i = 0; i < OCTOSCOPE_OUI_NUM; i++) {
		if (_rtw_memcmp(mac_addr, OCTOSCOPE_OUI[i], WLAN_IEEE_OUI_LEN) == _TRUE) {
			psta->vendor = HT_IOT_PEER_OCTOSCOPE;
			return;
		}
	}
}

u8 rtw_is_l2uf_frame(struct sk_buff *pskb)
{
	int i = 0;
	char l2ufHex[][6] = {
		{0x00, 0x01, 0xaf, 0x81, 0x01, 0x02}, //MTK l2uf
		{0x00, 0x00, 0xf5, 0x81, 0x80, 0x00}, //QCA l2uf
		{0x00, 0x01, 0xaf, 0x81, 0x01, 0x00}, //BCM l2uf
	};

	if(!pskb || !pskb->data)
		return _FALSE;

	for(i = 0; i < ARRAY_SIZE(l2ufHex); i++)
	{
		if(_rtw_memcmp(pskb->data + ETH_HLEN, l2ufHex[i], 6)){
			return _TRUE;
		}
	}

	return _FALSE;
}
