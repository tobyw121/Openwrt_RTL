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
#define _RTW_ACS_C_

#include <drv_types.h>

#ifdef CONFIG_RTW_ACS

#define BAN_SCORE -1
#define OVERLAP_SCORE -2
#define MAX_SCORE 100

#define MASK_CH_2G(_ch, _overlap) if ((1) <= (_ch) && (_ch) <= (14)) _overlap[_ch] = _TRUE;
#define MASK_CH_5G(_ch, _overlap) if ((36) <= (_ch) && (_ch) <= (165)) _overlap[_ch] = _TRUE;
#define CLM_SCORE(_clm_ratio) (_clm_ratio)
#define NOISE_SCORE(_noise) ((_noise) + 110)

void acs_parm_init(_adapter *adapter, u8 ch_num, struct phl_scan_channel *ch)
{
	struct acs_priv *acs = adapter_to_acs(adapter);
	struct acs_parm *parm = &acs->parm;
	u8 idx, acs_idx;

	_rtw_memset(parm->acs_idx, 0, MAX_ACS_INFO);

	parm->num = (ch_num > MAX_ACS_INFO) ? MAX_ACS_INFO : ch_num;

	for (idx = 0; idx < parm->num; idx++) {
		acs_idx = rtw_get_acs_chnl_tbl_idx(adapter, ch[idx].band, ch[idx].channel);
		ch[idx].acs_idx = acs_idx;
		parm->acs_idx[idx] = acs_idx;
	}
}

void acs_reset_info(_adapter *adapter)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	RT_CHANNEL_INFO *ch_set = rfctl->channel_set;
	struct acs_priv *acspriv = adapter_to_acs(adapter);
	u8 idx;

	for (idx = 0; idx < MAX_CHANNEL_NUM; idx++)
		ch_set[idx].rx_count = 0;

}

u8 get_center_channel_2g(u8 channel, u32 bw, u32 offset)
{
	u8 center_ch = channel;

	if (bw == CHANNEL_WIDTH_40) {
		if (offset == CHAN_OFFSET_LOWER)
			center_ch -= 2;
		else if (offset == CHAN_OFFSET_UPPER)
			center_ch += 2;
	}

	return center_ch;
}

u8 get_center_channel_5g(u8 channel, u8 bw)
{
	u8 center_ch = channel;

	if (bw == CHANNEL_WIDTH_160) {
		if (channel <= 64)
			center_ch = 50;
		else
			center_ch = 114;
	} else if (bw == CHANNEL_WIDTH_80) {
		if(channel <= 48)
			center_ch = 42;
		else if(channel <= 64)
			center_ch = 58;
		else if(channel <= 112)
			center_ch = 106;
		else if(channel <= 128)
			center_ch = 122;
		else if(channel <= 144)
			center_ch = 138;
		else
			center_ch = 155;
	} else if (bw == CHANNEL_WIDTH_40) {
		if (channel <= 40)
			center_ch = 38;
		else if (channel <= 48)
			center_ch = 46;
		else if (channel <= 56)
			center_ch = 54;
		else if (channel <= 64)
			center_ch = 62;
		else if (channel <= 104)
			center_ch = 102;
		else if (channel <= 112)
			center_ch = 110;
		else if (channel <= 120)
			center_ch = 118;
		else if (channel <= 128)
			center_ch = 126;
		else if (channel <= 136)
			center_ch = 134;
		else if (channel <= 144)
			center_ch = 142;
		else if (channel <= 153)
			center_ch = 151;
		else
			center_ch = 159;
	}

	return center_ch;
}

u8 get_center_channel_g6(u8 channel, u8 bw, u8 offset)
{
	return (channel > 14) ?
			get_center_channel_5g(channel, bw) :
			get_center_channel_2g(channel, bw, offset);
}

void set_overlap_5g(struct wlan_network	*pnetwork, bool *overlap)
{
	u8 bw = pnetwork->network.Configuration.Bw;
	u8 offset = pnetwork->network.Configuration.Offset;
	u8 channel = pnetwork->network.Configuration.DSConfig;
	s8 i, j;

	u8 center_ch = get_center_channel_g6(channel, bw, offset);

	if (bw == CHANNEL_WIDTH_20) {
		MASK_CH_5G(center_ch, overlap);
	} else if (bw == CHANNEL_WIDTH_40) {
		for (i = -1; i <= 1; i++) {
			if (i == 0)
				continue;

			j = (i > 0) ? -1 : 1;
			MASK_CH_5G(center_ch + (4 * i + j * 2), overlap);
		}
	} else if (bw == CHANNEL_WIDTH_80) {
		for (i = -2; i <= 2; i++) {
			if (i == 0)
				continue;

			j = (i > 0) ? -1 : 1;
			MASK_CH_5G(center_ch + (4 * i + j * 2), overlap);
		}
	} else if (bw == CHANNEL_WIDTH_160) {
		for (i = -4; i <= 4; i++) {
			if (i == 0)
				continue;

			j = (i > 0) ? -1 : 1;
			MASK_CH_5G(center_ch + (4 * i + j * 2), overlap);
		}
	} else {
		RTW_INFO("[ACS] unsupport bw (%d)\n", bw);
	}
}

void set_overlap_2g(struct wlan_network	*pnetwork, bool *overlap)
{
	u8 bw = pnetwork->network.Configuration.Bw;
	u8 offset = pnetwork->network.Configuration.Offset;
	s32 i, channel = pnetwork->network.Configuration.DSConfig;

	if (bw == CHANNEL_WIDTH_20) {
		for (i = -2; i <= 2; i++)
			MASK_CH_2G(channel + i, overlap);
	} else if (bw == CHANNEL_WIDTH_40) {
		if (offset == CHAN_OFFSET_UPPER) {
			for (i = -2; i <= 6; i++)
				MASK_CH_2G(channel + i, overlap);
		} else if (offset == CHAN_OFFSET_LOWER) {
			for (i = -6; i <= 2; i++)
				MASK_CH_2G(channel + i, overlap);
		}
	} else {
		RTW_INFO("[ACS] unsupport bw (%d)\n", bw);
	}
}

void acs_set_channel_overlap(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
#ifdef RTW_MI_SHARE_BSS_LIST
	_queue *queue = &padapter->dvobj->scanned_queue;
#else
	_queue *queue = &(pmlmepriv->scanned_queue);
#endif
	_list *plist, *phead;
	struct wlan_network *pnetwork;
	struct acs_priv *acs = adapter_to_acs(padapter);

	memset(acs->overlap, 0, sizeof(acs->overlap));

	_rtw_spinlock_bh(&(queue->lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (!rtw_end_of_queue_search(phead, plist)) {
		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		if (pnetwork->network.Configuration.DSConfig > 14)
			set_overlap_5g(pnetwork, acs->overlap);
		else
			set_overlap_2g(pnetwork, acs->overlap);

		plist = get_next(plist);
	}

	_rtw_spinunlock_bh(&(queue->lock));
}

bool check_overlap_2g(u8 channel, u8 bw, bool *overlap)
{
	s8 i, j;
	bool is_overlap = _FALSE;

	i = (bw == CHANNEL_WIDTH_40) ? 4 : 2;

	for (j = -i; j <= i; j++) {
		if ((0 <= channel + j) && (channel + j < 14)) {
			if (overlap[channel + j]) {
				is_overlap = _TRUE;
				break;
			}
		}
	}

	return is_overlap;
}

bool _check_overlap_5g(u8 ch, bool *overlap)
{
	if ((36 <= ch) && (ch <= 165))
		return overlap[ch];

	return _FALSE;
}

bool check_overlap_5g(u8 channel, u8 bw, bool *overlap)
{
	u8 center_ch = get_center_channel_g6(channel, bw, 0);
	s8 i, j;
	bool is_overlap = _FALSE;

	if (bw == CHANNEL_WIDTH_20) {
		if (_check_overlap_5g(center_ch, overlap))
			is_overlap = _TRUE;
	} else if (bw == CHANNEL_WIDTH_40) {
		for (i = -1; i <= 1; i++) {
			if (i == 0)
				continue;

			j = (i > 0) ? -1 : 1;

			if (_check_overlap_5g(center_ch + (4 * i + j * 2), overlap)) {
				is_overlap = _TRUE;
				break;
			}
		}
	} else if (bw == CHANNEL_WIDTH_80) {
		for (i = -2; i <= 2; i++) {
			if (i == 0)
				continue;

			j = (i > 0) ? -1 : 1;

			if (_check_overlap_5g(center_ch + (4 * i + j * 2), overlap)) {
				is_overlap = _TRUE;
				break;
			}
		}
	} else if (bw == CHANNEL_WIDTH_160) {
		for (i = -4; i <= 4; i++) {
			if (i == 0)
				continue;

			j = (i > 0) ? -1 : 1;

			if (_check_overlap_5g(center_ch + (4 * i + j * 2), overlap)) {
				is_overlap = _TRUE;
				break;
			}
		}
	} else {
		RTW_INFO("[ACS] unsupport bw (%d)\n", bw);
	}

	return is_overlap;
}

bool is_acs_ban_channel(_adapter *adapter, u8 channel, u8 bw)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);

	if (channel > 14) { /* 5g */
		#ifdef WKARD_ACS_DISABLE_DFS_CHANNEL
		if (rtw_chset_is_dfs_ch(rfctl->channel_set, channel))
			return _TRUE;
		#endif

		if (adapter->registrypriv.wifi_mib.autoch_3664157_enable == 1 &&
			channel != 36 && channel != 64 && channel != 157)
			return _TRUE;

		if (channel == 165 && bw != CHANNEL_WIDTH_20)
			return _TRUE;

		if (channel > 128 && bw == CHANNEL_WIDTH_160)
			return _TRUE;
	} else { /* 2g */
		if (channel != 1 &&
			channel != 6 &&
			channel != 11 &&
			adapter->registrypriv.wifi_mib.autoch_1611_enable == 1)
			return _TRUE;

		if (channel == 12 ||
			channel == 13 ||
			channel == 14)
			return _TRUE;
	}

	return _FALSE;
}

void acs_find_clean_channel(_adapter *adapter, u8 bw)
{
	struct acs_priv *acs = adapter_to_acs(adapter);
	struct acs_result *acs_r = &(acs->result[0]);
	u8 idx, channel;
	bool exist_clean_channel = _FALSE;

	for (idx = 0; idx < MAX_ACS_INFO; idx++) {
		channel = acs_r[idx].channel;

		if (!channel)
			break;

		acs_r[idx].overlap = (channel > 14) ?
							  check_overlap_5g(channel, bw, acs->overlap) :
							  check_overlap_2g(channel, bw, acs->overlap);

		if (!acs_r[idx].overlap && !is_acs_ban_channel(adapter, channel, bw))
			exist_clean_channel = _TRUE;
	}

	acs->exist_clean_channel = exist_clean_channel;
}

s8 calculate_channel_score_by_rx_count(struct acs_result acs_r)
{
	if (acs_r.overlap)
		return OVERLAP_SCORE;

	if (acs_r.rx_count > MAX_SCORE)
		return 0;

	return MAX_SCORE - acs_r.rx_count;
}

s8 calculate_channel_score_by_env(struct acs_result acs_r)
{
	/* 80% clm & 20% noise */
	u8 env_score = ((CLM_SCORE(acs_r.clm_ratio) * 4 / 5) + (NOISE_SCORE(acs_r.noise) * 1 / 5)) / 2;

	if (env_score > MAX_SCORE)
		return 0;

	return MAX_SCORE - env_score;
}

s8 calculate_channel_score_by_nhm(struct acs_result acs_r, u32 max_nhm_score)
{
	u8 env_score = (acs_r.nhm_score * 100) / max_nhm_score;

	if (env_score > MAX_SCORE)
		return 0;

	return MAX_SCORE - env_score;
}

s8 calculate_channel_score_by_nhm_ctc(struct acs_result acs_r, u32 max_nhm_score)
{
	u8 env_score = (acs_r.nhm_score * 100) / max_nhm_score;

	/* 2.4G, preferred choice 1/6/11 */
	if (acs_r.channel < 14 &&
		!(acs_r.channel == 1 ||
		  acs_r.channel == 6 ||
		  acs_r.channel == 11))
		env_score += 40;

	if (env_score > MAX_SCORE)
		return 0;

	return MAX_SCORE - env_score;
}

u32 get_max_nhm_score(struct acs_result *acs_r)
{
	u8 idx;
	u32 max_nhm_score = 0;

	for (idx = 0; idx < MAX_ACS_INFO; idx++) {
		if (max_nhm_score < acs_r[idx].nhm_score)
			max_nhm_score = acs_r[idx].nhm_score;
	}

	return max_nhm_score;
}

void acs_calculate_channel_score(_adapter *adapter)
{
	struct acs_priv *acs = adapter_to_acs(adapter);
	struct acs_result *acs_r = &(acs->result[0]);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	u8 idx, channel;
	s8 score;

	RTW_INFO("[ACS] exist_clean_channel = %s, calculate score by %s\n",
		acs->exist_clean_channel ? "TRUE" : "FALSE",
		acs->exist_clean_channel ? "RX_COUNT" : "ENV");

	if (!acs->exist_clean_channel &&
		#if !defined(CONFIG_CTC_FEATURE)
		acs->use_nhm &&
		#endif
		1) {
		acs->max_nhm_score = get_max_nhm_score(acs_r);
	}

	for (idx = 0; idx < MAX_ACS_INFO; idx++) {

		channel = acs_r[idx].channel;

		if (!channel)
			break;

		/* get clm & nhm fail */
		if (acs_r[idx].clm_ratio == 0 && acs_r[idx].noise == -110) {
			acs_r[idx].score = BAN_SCORE;
			continue;
		}

		if (acs->exist_clean_channel)
			score = calculate_channel_score_by_rx_count(acs_r[idx]);
		else {
			#if defined(CONFIG_CTC_FEATURE)
			score = calculate_channel_score_by_nhm_ctc(acs_r[idx], acs->max_nhm_score);
			#else
			if (acs->use_nhm)
				score = calculate_channel_score_by_nhm(acs_r[idx], acs->max_nhm_score);
			else
				score = calculate_channel_score_by_env(acs_r[idx]);
			#endif
		}

		acs_r[idx].score = (score >= 0) ? score : 0;
	}
}


void acs_select_clean_channel(_adapter	*adapter, u8 bw)
{
	struct acs_priv *acs = adapter_to_acs(adapter);
	struct acs_result *acs_r = &(acs->result[0]);
	u8 idx, max_idx = 0;

	for (idx = 0; idx < MAX_ACS_INFO; idx++) {
		if (!acs_r[idx].channel)
			break;

		if (is_acs_ban_channel(adapter, acs_r[idx].channel, bw)) {
			continue;
		}

		if (acs_r[idx].score > acs_r[max_idx].score)
			max_idx = idx;
	}

	acs->best_channel = acs_r[max_idx].channel;
}

void acs_change_bss_chbw(_adapter *adapter, u8 with_csa_ie, s8 csa_cnt)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct acs_priv *acs = adapter_to_acs(adapter);
	struct mlme_ext_priv *pmlmeextpriv = &adapter->mlmeextpriv;
	_adapter *iface;
	u32 ifbmp = 0;
	u8 idx;
	u8 channel = 0;
#ifdef CONFIG_DFS_CSA_IE
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
#endif
	acs_find_clean_channel(adapter, pmlmeextpriv->cur_bwmode);
	acs_calculate_channel_score(adapter);
	acs_select_clean_channel(adapter, pmlmeextpriv->cur_bwmode);

	channel = acs->best_channel;
	acs->best_channel = 0;

	if (channel == 0 ||
		channel == rtw_phl_get_cur_ch(adapter->phl_role)) {
		return;
	}

#ifdef CONFIG_DFS_CSA_IE
	if(with_csa_ie){
		rfctl->csa_ch = channel;
		rfctl->csa_cntdown = (csa_cnt == -1) ? 5 : csa_cnt ;
		rfctl->csa_set_ie = 1;

		rtw_mi_tx_beacon_hdl(adapter);
	}else
#endif
	{
		for (idx = 0; idx < dvobj->iface_nums; idx++) {
			iface = dvobj->padapters[idx];
			if (!iface)
				continue;

			if (CHK_MLME_STATE(iface, WIFI_AP_STATE) && iface->netif_up)
				ifbmp |= BIT(idx);
		}

		if (ifbmp)
			rtw_change_bss_chbw_cmd(adapter, RTW_CMDF_WAIT_ACK, ifbmp, 0, channel, REQ_BW_ORI, REQ_OFFSET_NONE);
	}

	RTW_PRINT("[ACS] switch to channel=%d\n", channel);
}

u32 rtw_get_nhm_score(u8 *nhm_rpt)
{
	u8 i, j;
	u32 score = 0, tmp = 0;

	for (i = 0; i < NHM_RPT_NUM ; i++) {
		tmp = nhm_rpt[i];
		for (j = 0; j < i; j++)
			tmp *= 3;
		score += tmp;
	}

	return score;
}

void acs_get_survey_result(_adapter *adapter)
{
	struct acs_priv *acs = adapter_to_acs(adapter);
	struct acs_result *acs_r = &(acs->result[0]);
	struct acs_parm *parm = &acs->parm;
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	u8 idx, acs_idx;

	_rtw_memset(acs_r, 0, sizeof(struct acs_result) * MAX_ACS_INFO);

	for (idx = 0; idx < parm->num; idx++) {
		acs_idx = parm->acs_idx[idx];

		acs_r[idx].band = rtw_acs_get_band_by_idx(adapter, acs_idx);
		acs_r[idx].channel = rtw_acs_get_channel_by_idx(adapter, acs_idx);
		acs_r[idx].clm_ratio = rtw_acs_get_clm_ratio_by_idx(adapter, acs_idx);
		acs_r[idx].noise = rtw_acs_get_noise_by_idx(adapter, acs_idx);
		acs_r[idx].rx_count = rtw_get_rx_count_from_chset(rfctl->channel_set, acs_r[idx].channel);
		rtw_acs_get_nhm_rpt_by_idx(adapter, acs_idx, acs_r[idx].nhm_rpt);
		acs_r[idx].nhm_score = rtw_get_nhm_score(acs_r[idx].nhm_rpt);
	}
}

bool check_acs_keep_result(_adapter *adapter)
{
	struct acs_priv *acs = adapter_to_acs(adapter);

	if (acs->last_acs_time &&
	    (rtw_get_passing_time_ms(acs->last_acs_time) < KEEP_PREVIOS_ACS_RESULT_INTERVAL)) {
	    RTW_INFO("[ACS] keep previous acs result (%lu)\n", jiffies);
	    return _TRUE;
	} else {
		acs->last_acs_time = rtw_get_current_time();
		RTW_INFO("[ACS] new timestamp = %lu\n", acs->last_acs_time);
		return _FAIL;
	}
}

void rtw_select_clean_channel(_adapter *adapter)
{
	struct mlme_ext_priv *pmlmeextpriv = &adapter->mlmeextpriv;

	if (check_acs_keep_result(adapter))
		return;

	acs_set_channel_overlap(adapter);
	acs_get_survey_result(adapter);
	acs_find_clean_channel(adapter, pmlmeextpriv->cur_bwmode);
	acs_calculate_channel_score(adapter);
}

void acs_replace_hapd_channel(_adapter *adapter, const u8 *head, size_t head_len,
                          const u8 *bcn_ies, size_t bcn_ies_len,
                          const u8 *tail, size_t tail_len)
{
	u8 *p, *channel, ch = 0, bw = 0, offset = 0;
	uint ie_len = 0;
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);
	struct acs_priv *acs = adapter_to_acs(adapter);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return;

	if (head_len < 24)
		return;

	rtw_ies_get_chbw((u8 *)tail, (int)(tail_len), &ch, &bw, &offset, 1, 1);
	acs_find_clean_channel(adapter, bw);
	acs_calculate_channel_score(adapter);
	acs_select_clean_channel(adapter, bw);

	p = rtw_get_ie(head + 24 + _BEACON_IE_OFFSET_, _DSSET_IE_, &ie_len, (head_len - _BEACON_IE_OFFSET_));
	if (p && ie_len > 0) {
		channel = p + 2;
		RTW_INFO("[%s] IE channel :%d, ACS best channel :%d\n", __func__, *channel, acs->best_channel);
		if (*channel != acs->best_channel) {
			*channel = acs->best_channel;
			acs->best_channel = 0;
		}
	}

	return;
}

#ifdef CONFIG_RTW_PACS
void pacs_change_bss_chbw(_adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct acs_priv *acs = adapter_to_acs(adapter);
	struct mlme_ext_priv *pmlmeextpriv = &adapter->mlmeextpriv;
	struct sta_priv *psta_priv = NULL;
	_adapter *iface;
	u32 ifbmp = 0;
	u8 idx;
	u8 channel = 0;
#ifdef CONFIG_DFS_CSA_IE
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	u8 with_csa_ie = 1;
	u8 csa_cnt = 5;
#endif

	for (idx = 0; idx < dvobj->iface_nums; idx++) {
		iface = dvobj->padapters[idx];

		if (!iface)
			continue;
		psta_priv = &iface->stapriv;

		if (CHK_MLME_STATE(iface, WIFI_AP_STATE) && iface->netif_up)
			ifbmp |= BIT(idx);

		if(psta_priv->asoc_sta_count-1 > 0) {
			RTW_INFO("[%s %d]the client status is FALSE and cannot change channel\n", __FUNCTION__, __LINE__);
			ifbmp = 0;
			break;
		}
	}
	if(ifbmp == 0) {
		RTW_INFO("[PACS] Currently not suitable for switching channels.\n");
		return;
	}

	acs_find_clean_channel(adapter, pmlmeextpriv->cur_bwmode);
	acs_calculate_channel_score(adapter);
	acs_select_clean_channel(adapter, pmlmeextpriv->cur_bwmode);

	channel = acs->best_channel;
	acs->best_channel = 0;

	if (channel == 0 ||
		channel == pmlmeextpriv->cur_channel) {
		RTW_INFO("[PACS] The best_channel is equal to current channel and return.\n");
		return;
	}

#ifdef CONFIG_DFS_CSA_IE
	if(with_csa_ie){
		rfctl->csa_ch = channel;
		rfctl->csa_cntdown = csa_cnt ;
		rfctl->csa_set_ie = 1;
		RTW_INFO("[PACS] [%s %d] After %d cntdown, change the channel to %d.\n", __FUNCTION__, __LINE__, rfctl->csa_cntdown, rfctl->csa_ch);
		rtw_mi_tx_beacon_hdl(adapter);
	}else
#endif
	{
		rtw_change_bss_chbw_cmd(adapter, RTW_CMDF_WAIT_ACK, ifbmp, 0, channel, REQ_BW_ORI, REQ_OFFSET_NONE);
	}

	RTW_PRINT("[ACS] switch to channel=%d\n", channel);
}

#endif
#ifdef CONFIG_RTW_DACS
static void rtw_print_dcs_result(_adapter *adapter)
{
	struct acs_priv *acs = adapter_to_acs(adapter);
	struct acs_result *dacs_r = &(acs->dacs_result[0]);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	u8 idx;

	for (idx = 0; idx < rfctl->max_chan_nums; idx++) {
		RTW_PRINT("[%s] dacs_idx:%d ch:%d rx_cnt:%d clm:%d, nhm:%d, score:%d\n",
				__func__,
				idx,
				dacs_r[idx].channel,
				dacs_r[idx].rx_count,
				dacs_r[idx].clm_ratio,
				dacs_r[idx].noise,
				dacs_r[idx].nhm_score);
	}
}

void rtw_dacs_mnt_result(_adapter *adapter)
{
	struct acs_priv *acs = adapter_to_acs(adapter);
	struct acs_result *dacs_r = &(acs->dacs_result[0]);
	struct acs_parm *parm = &acs->parm;
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	u8 acs_idx = parm->acs_idx[0];
#ifdef CONFIG_RTW_PACS
	struct acs_result *acs_r = &(acs->result[0]);
#endif

	dacs_r[acs->dacs_idx].band = rtw_acs_get_band_by_idx(adapter, acs_idx);
	dacs_r[acs->dacs_idx].channel = rtw_acs_get_channel_by_idx(adapter, acs_idx);
	dacs_r[acs->dacs_idx].clm_ratio = rtw_acs_get_clm_ratio_by_idx(adapter, acs_idx);
	dacs_r[acs->dacs_idx].noise = rtw_acs_get_noise_by_idx(adapter, acs_idx);
	dacs_r[acs->dacs_idx].rx_count = rtw_get_rx_count_from_chset(rfctl->channel_set, dacs_r[acs->dacs_idx].channel);
	rtw_acs_get_nhm_rpt_by_idx(adapter, acs_idx, dacs_r[acs->dacs_idx].nhm_rpt);
	dacs_r[acs->dacs_idx].nhm_score = rtw_get_nhm_score(dacs_r[acs->dacs_idx].nhm_rpt);

#ifdef CONFIG_RTW_PACS
	memcpy(&acs_r[acs->dacs_idx], &dacs_r[acs->dacs_idx], sizeof(struct acs_result));
#endif

	RTW_INFO("[%s] dacs_idx:%d ch:%d rx_cnt:%d clm:%d, nhm:%d, score:%d\n",
				__func__,
				acs->dacs_idx,
				dacs_r[acs->dacs_idx].channel,
				dacs_r[acs->dacs_idx].rx_count,
				dacs_r[acs->dacs_idx].clm_ratio,
				dacs_r[acs->dacs_idx].noise,
				dacs_r[acs->dacs_idx].nhm_score);

	acs->dacs_idx++;
	return;
}

void rtw_dacs_mnt_trigger(_adapter *adapter)
{
	struct acs_priv *acs = adapter_to_acs(adapter);
	struct sitesurvey_parm parm;
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);

	if (acs->dacs_idx == rfctl->max_chan_nums) {
		acs->dacs_idx = 0;
		RTW_INFO("[DACS] Change mib dacs to 0!\n");
		adapter->registrypriv.wifi_mib.dacs = 0;
		rtw_print_dcs_result(adapter);
#ifdef CONFIG_RTW_PACS
		pacs_change_bss_chbw(adapter);
#endif
		return;
	}

	if (rtw_sitesurvey_condition_check(adapter, _FALSE) != SS_ALLOW)
		return;

	rtw_ps_deny(adapter, PS_DENY_SCAN);
	if (!rtw_is_adapter_up(adapter) || _FAIL == rtw_pwr_wakeup(adapter)) {
		RTW_PRINT("[%s]scan abort!! adapter cannot use\n", __func__);
		goto cancel_ps_deny;
	}

	RTW_INFO("[%s] acs->dacs_idx:%d ch:%d max_ch_num:%d\n",
				__func__,
				acs->dacs_idx,
				rfctl->channel_set[acs->dacs_idx].ChannelNum,
				rfctl->max_chan_nums);

	_rtw_memset(&parm, 0, sizeof(struct sitesurvey_parm));
	parm.scan_mode = RTW_PHL_SCAN_PASSIVE;
	parm.bw = CHANNEL_WIDTH_20;
	parm.duration = 150;
	parm.ch[0].hw_value = rfctl->channel_set[acs->dacs_idx].ChannelNum;
	parm.ch[0].flags = RTW_IEEE80211_CHAN_PASSIVE_SCAN;
	parm.ch_num = 1;
	parm.acs = 1;

	rtw_set_802_11_bssid_list_scan(adapter, &parm);

cancel_ps_deny:
	rtw_ps_deny_cancel(adapter, PS_DENY_SCAN);
	return;
}
#endif /* CONFIG_RTW_DACS*/
#endif /* CONFIG_RTW_ACS */

