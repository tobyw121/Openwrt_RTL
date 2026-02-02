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
#define _RTW_MBO_C

#include <drv_types.h>

#ifdef CONFIG_RTW_MBO

void rtw_mbo_ie_init(_adapter *padapter, struct mbo_priv *mbopriv)
{
	if(!mbopriv)
		return;
	mbopriv->assoc_disallow = 0;
	mbopriv->cellular_aware = 0;
	mbopriv->ch_list_num = 0;
	mbopriv->mbo_oce_element_len = 6;
	mbopriv->mbo_oce_element[0] = 0xdd;
	mbopriv->mbo_oce_element[1] = mbopriv->mbo_oce_element_len;
	mbopriv->mbo_oce_element[2] = 0x50;
	mbopriv->mbo_oce_element[3] = 0x6f;
	mbopriv->mbo_oce_element[4] = 0x9a;
	mbopriv->mbo_oce_element[5] = 0x16;
}

void rtw_mbo_fill_non_prefer_channel_list(_adapter *padapter, struct mbo_priv *mbopriv,
										const u8 *pbuf, u8 len)
{
	u8 op_class = 0;
	u8 preference = 0;
	int i;

	/* invalid length */
	if(len != 0 && len < 3)
		return;

	/* reset non-prefer channel list */
	mbopriv->ch_list_num = 0;
	op_class = *pbuf;
	preference = *(pbuf + len - 2);

	if (len == 3 && mbopriv->ch_list_num < MBO_CH_LIST_MAX_NUM) {
		mbopriv->ch_list[mbopriv->ch_list_num].op_class = op_class;
		mbopriv->ch_list[mbopriv->ch_list_num].preference = preference;
		mbopriv->ch_list[mbopriv->ch_list_num].channel = 0;
		mbopriv->ch_list_num += 1;
		RTW_INFO("[%s:%d]channel = %d, preference = %d\n", __func__, __LINE__, 0, preference);
	} else {
		for (i = 0; i < len - 3; i++) {
			if(mbopriv->ch_list_num >= MBO_CH_LIST_MAX_NUM)
				break;
			mbopriv->ch_list[mbopriv->ch_list_num].op_class = op_class;
			mbopriv->ch_list[mbopriv->ch_list_num].preference = preference;
			mbopriv->ch_list[mbopriv->ch_list_num].channel = *(pbuf + 1 + i);
			mbopriv->ch_list_num += 1;
			RTW_INFO("[%s:%d]channel = %d, preference = %d\n", __func__, __LINE__, 
									*(pbuf + 1 + i), preference);
		}
	}
	
}

void rtw_mbo_ie_handler(_adapter *padapter, struct mbo_priv *mbopriv, const u8 *pbuf, uint limit_len)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	uint total_len = 0;
	u8 attribute_id = 0;
	u8 attribute_len = 0;
	u8 mbo_hdr_len = 6;
#ifdef CONFIG_RTW_MULTI_AP_R2
	u8 last_assoc_disallow = 0, assoc_disallow_status = 0;
#endif
	const u8 *p = pbuf;

	if(!mbopriv)
		return;
	if (limit_len  > MBO_OCE_ELEMENT_MAX_LEN - mbo_hdr_len) {
		RTW_ERR("[%s] mbo_oce_element array migh be overrun\n", __func__);
		return ;
	}

#ifdef CONFIG_RTW_MULTI_AP_R2
	last_assoc_disallow = mbopriv->assoc_disallow;
#endif

	rtw_mbo_ie_init(padapter, mbopriv);
	_rtw_memcpy(mbopriv->mbo_oce_element + mbo_hdr_len, pbuf, limit_len);
	mbopriv->mbo_oce_element[1] = limit_len + 4;

	while (total_len <= limit_len) {
		attribute_id = *p;
		attribute_len = *(p + 1);
		total_len += (attribute_len + 2);
		if(total_len > limit_len)
			break;

		switch (attribute_id) {
			case MBO_AP_CAPABILITY:
				if(attribute_len == 1){
					RTW_INFO("[%s]Find attribute MBO_AP_CAPABILITY\n", __func__);
					if(*(p+2) & 0x40)
						mbopriv->cellular_aware = 1;
				}
				break;
			case ASSOCIATION_DISALLOW:
				if(attribute_len == 1){
					RTW_INFO("[%s]Find attribute ASSOCIATION_DISALLOW\n", __func__);
					mbopriv->assoc_disallow = *(p+2);
				}
				break;
			case NON_PREFER_CHANNEL_RPT:
				RTW_INFO("[%s]Find attribute NON_PREFER_CHANNEL_RPT\n", __func__);
				rtw_mbo_fill_non_prefer_channel_list(padapter, mbopriv, p + 2, attribute_len);
				break;
			case CELLULAR_DATA_CAPABILITY:
			case CELLULAR_DATA_CONNECT_PREFER:
			case TRANS_REASON_CODE:
			case TRANS_REJECT_REASON_CODE:
			case ASSOCIATION_RETRY_DELAY:
				break;
			default:
				RTW_ERR("[%s]Unknown MBO attribute %d\n", __func__, attribute_id);
		}

		p += (attribute_len + 2);
	}
#ifdef CONFIG_RTW_MULTI_AP_R2
	if(last_assoc_disallow != mbopriv->assoc_disallow) {
		assoc_disallow_status = !(mbopriv->assoc_disallow);
		core_map_send_association_status_notification(padapter, assoc_disallow_status);
	}
#endif
}

void rtw_ap_parse_sta_mbo_element(_adapter *padapter,
	struct sta_info *psta, u8 *ies_buf, u16 ies_len)
{
	uint ie_len = 0;
	u8 *p;
	u8 WIFI_ALLIANCE_OUI[] = {0x50, 0x6f, 0x9a};
	
	ie_len = 0;
	for (p = ies_buf; ; p += (ie_len + 2)) {
		p = rtw_get_ie(p, _SSN_IE_1_, &ie_len, (ies_len - (ie_len + 2)));
		if ((p) && (_rtw_memcmp(p + 2, WIFI_ALLIANCE_OUI, 3)) && (*(p+5) == MBO_OUI_TYPE)) {
			/* find MBO-OCE information element */
			psta->mbopriv.enable = _TRUE;
			rtw_mbo_ie_handler(padapter, &psta->mbopriv, p + 6, ie_len - 4);
			break;
		}
		if ((p == NULL) || (ie_len == 0))
			break;
	}
}

/* return _SUCCESS: neighbor report is valid
 * return _FAIL:    neighbor report is not valid
 */
int rtw_mbo_check_channel_valid(_adapter *padapter, struct nb_rpt_hdr report,
									struct sta_info *sta)
{
	int i;
	if(!sta)
		return _SUCCESS;
	
	for(i = 0 ; i < sta->mbopriv.ch_list_num; i++)
	{
		if(sta->mbopriv.ch_list[i].preference != MBO_CH_PREFER_NON_OP)
			continue;
		if(sta->mbopriv.ch_list[i].op_class != report.reg_class)
			continue;
		if(sta->mbopriv.ch_list[i].channel == 0) {
			;/* TODO: check all channels in the op_class*/ 
		} else if(sta->mbopriv.ch_list[i].channel == report.ch_num) {
			return _FAIL;
		}
	}
	return _SUCCESS;
}

void rtw_bss_termination_timeout_handler(void *ctx)
{
	_adapter *adapter = (_adapter *)ctx;
	struct	mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct rtw_wifi_role_t *wrole = adapter->phl_role;
	void *phl = GET_HAL_INFO(adapter_to_dvobj(adapter));
	pmlmepriv->mbopriv.bss_termination_phase = BSS_TERMINATION_DUR;
	RTW_INFO("[%s]disable beacon tx\n", __func__);
	
	/* disassoc all STA*/
#ifdef CONFIG_AP_MODE
	if (MLME_IS_AP(adapter))
		rtw_sta_flush(adapter, _TRUE, _TRUE);
#endif
	/* disable beacon */
	rtw_bcn_drop_switch(adapter, 1);
	_set_timer(&pmlmepriv->mbopriv.bss_termination_dur_timer, pmlmepriv->mbopriv.bss_termination_dur*60*1000);
	
}

void rtw_bss_termination_dur_timeout_handler(void *ctx)
{
	_adapter *adapter = (_adapter *)ctx;
	struct	mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct rtw_wifi_role_t *wrole = adapter->phl_role;
	void *phl = GET_HAL_INFO(adapter_to_dvobj(adapter));
	RTW_INFO("[%s]enable beacon tx\n", __func__);
	/* enable beacon */
	rtw_bcn_drop_switch(adapter, 0);

	pmlmepriv->mbopriv.bss_termination_phase = BSS_TERMINATION_NONE;
}
#endif /* CONFIG_RTW_MBO */

