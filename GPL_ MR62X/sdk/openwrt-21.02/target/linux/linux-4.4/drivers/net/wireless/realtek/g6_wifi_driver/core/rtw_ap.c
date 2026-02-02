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
#define _RTW_AP_C_

#include <drv_types.h>
#include "../phl/phl_headers.h"

#ifdef CONFIG_AP_MODE

extern unsigned char	RTW_WPA_OUI[];
extern unsigned char	WMM_OUI[];
extern unsigned char	WPS_OUI[];
extern unsigned char	P2P_OUI[];
extern unsigned char	WFD_OUI[];
extern unsigned char	OWE_TRANSITION_OUI[];

void init_mlme_ap_info(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	_rtw_spinlock_init(&pmlmepriv->bcn_update_lock);
	/* pmlmeext->bstart_bss = _FALSE; */
}

void free_mlme_ap_info(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	stop_ap_mode(padapter);
	_rtw_spinlock_free(&pmlmepriv->bcn_update_lock);

}

/*
* Set TIM IE
* return length of total TIM IE
*/
u8 rtw_set_tim_ie(u8 dtim_cnt, u8 dtim_period
	, const u8 *tim_bmp, u8 tim_bmp_len, u8 *tim_ie)
{
	u8 *p = tim_ie;
	u8 i, n1, n2;
	u8 bmp_len;

	if (rtw_bmp_not_empty(tim_bmp, tim_bmp_len)) {
		/* find the first nonzero octet in tim_bitmap */
		for (i = 0; i < tim_bmp_len; i++)
			if (tim_bmp[i])
				break;
		n1 = i & 0xFE;

		/* find the last nonzero octet in tim_bitmap, except octet 0 */
		for (i = tim_bmp_len - 1; i > 0; i--)
			if (tim_bmp[i])
				break;
		n2 = i;
		bmp_len = n2 - n1 + 1;
	} else {
		n1 = n2 = 0;
		bmp_len = 1;
	}

	*p++ = WLAN_EID_TIM;
	*p++ = 2 + 1 + bmp_len;
	*p++ = dtim_cnt;
	*p++ = dtim_period;
	*p++ = (rtw_bmp_is_set(tim_bmp, tim_bmp_len, 0) ? BIT0 : 0) | n1;
	_rtw_memcpy(p, tim_bmp + n1, bmp_len);

#if 0
	RTW_INFO("n1:%u, n2:%u, bmp_offset:%u, bmp_len:%u\n", n1, n2, n1 / 2, bmp_len);
	RTW_INFO_DUMP("tim_ie: ", tim_ie + 2, 2 + 1 + bmp_len);
#endif
	return 2 + 2 + 1 + bmp_len;
}

static void update_BCNTIM(_adapter *padapter)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork_mlmeext = &(pmlmeinfo->network);
	unsigned char *pie = pnetwork_mlmeext->IEs;

#if 0


	/* update TIM IE */
	/* if(rtw_tim_map_anyone_be_set(padapter, pstapriv->tim_bitmap)) */
#endif
	if (_TRUE) {
		u8 *p, *dst_ie, *premainder_ie = NULL, *pbackup_remainder_ie = NULL;
		uint offset, tmp_len, tim_ielen, tim_ie_offset, remainder_ielen;

		p = rtw_get_ie(pie + _FIXED_IE_LENGTH_, _TIM_IE_, &tim_ielen, pnetwork_mlmeext->IELength - _FIXED_IE_LENGTH_);
		if (p != NULL && tim_ielen > 0) {
			tim_ielen += 2;

			premainder_ie = p + tim_ielen;

			tim_ie_offset = (sint)(p - pie);

			remainder_ielen = pnetwork_mlmeext->IELength - tim_ie_offset - tim_ielen;

			/*append TIM IE from dst_ie offset*/
			dst_ie = p;
		} else {
			tim_ielen = 0;

			/*calculate head_len*/
			offset = _FIXED_IE_LENGTH_;

			/* get ssid_ie len */
			p = rtw_get_ie(pie + _BEACON_IE_OFFSET_, _SSID_IE_, &tmp_len, (pnetwork_mlmeext->IELength - _BEACON_IE_OFFSET_));
			if (p != NULL)
				offset += tmp_len + 2;

			/*get supported rates len*/
			p = rtw_get_ie(pie + _BEACON_IE_OFFSET_, _SUPPORTEDRATES_IE_, &tmp_len, (pnetwork_mlmeext->IELength - _BEACON_IE_OFFSET_));
			if (p !=  NULL)
				offset += tmp_len + 2;

			/*DS Parameter Set IE, len=3*/
			offset += 3;

			premainder_ie = pie + offset;

			remainder_ielen = pnetwork_mlmeext->IELength - offset - tim_ielen;

			/*append TIM IE from offset*/
			dst_ie = pie + offset;

		}

		if (remainder_ielen > 0) {
			pbackup_remainder_ie = rtw_malloc(remainder_ielen);
			if (pbackup_remainder_ie && premainder_ie)
				_rtw_memcpy(pbackup_remainder_ie, premainder_ie, remainder_ielen);
		}

		/* append TIM IE */
		dst_ie += rtw_set_tim_ie(0, pmlmeinfo->dtim_period, pstapriv->tim_bitmap, pstapriv->aid_bmp_len, dst_ie);

		/*copy remainder IE*/
		if (pbackup_remainder_ie) {
			_rtw_memcpy(dst_ie, pbackup_remainder_ie, remainder_ielen);

			rtw_mfree(pbackup_remainder_ie, remainder_ielen);
		}

		offset = (uint)(dst_ie - pie);
		pnetwork_mlmeext->IELength = offset + remainder_ielen;

	}
}

void rtw_add_bcn_ie(_adapter *padapter, WLAN_BSSID_EX *pnetwork, u8 index, u8 *data, u8 len)
{
	PNDIS_802_11_VARIABLE_IEs	pIE;
	u8	bmatch = _FALSE;
	u8	*pie = pnetwork->IEs;
	u8	*p = NULL, *dst_ie = NULL, *premainder_ie = NULL, *pbackup_remainder_ie = NULL;
	u32	i, offset, ielen = 0, ie_offset, remainder_ielen = 0;

	for (i = sizeof(NDIS_802_11_FIXED_IEs); i < pnetwork->IELength;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(pnetwork->IEs + i);

		if (pIE->ElementID > index)
			break;
		else if (pIE->ElementID == index) { /* already exist the same IE */
			p = (u8 *)pIE;
			ielen = pIE->Length;
			bmatch = _TRUE;
			break;
		}

		p = (u8 *)pIE;
		ielen = pIE->Length;
		i += (pIE->Length + 2);
	}

	if (p != NULL && ielen > 0) {
		ielen += 2;

		premainder_ie = p + ielen;

		ie_offset = (sint)(p - pie);

		remainder_ielen = pnetwork->IELength - ie_offset - ielen;

		if (bmatch)
			dst_ie = p;
		else
			dst_ie = (p + ielen);
	}

	if (dst_ie == NULL)
		return;

	if (remainder_ielen > 0) {
		pbackup_remainder_ie = rtw_malloc(remainder_ielen);
		if (pbackup_remainder_ie && premainder_ie)
			_rtw_memcpy(pbackup_remainder_ie, premainder_ie, remainder_ielen);
	}

	*dst_ie++ = index;
	*dst_ie++ = len;

	_rtw_memcpy(dst_ie, data, len);
	dst_ie += len;

	/* copy remainder IE */
	if (pbackup_remainder_ie) {
		_rtw_memcpy(dst_ie, pbackup_remainder_ie, remainder_ielen);

		rtw_mfree(pbackup_remainder_ie, remainder_ielen);
	}

	offset = (uint)(dst_ie - pie);
	pnetwork->IELength = offset + remainder_ielen;
}

void rtw_remove_bcn_ie(_adapter *padapter, WLAN_BSSID_EX *pnetwork, u8 index)
{
	u8 *p, *dst_ie = NULL, *premainder_ie = NULL, *pbackup_remainder_ie = NULL;
	uint offset, ielen, ie_offset, remainder_ielen = 0;
	u8	*pie = pnetwork->IEs;

	p = rtw_get_ie(pie + _FIXED_IE_LENGTH_, index, &ielen, pnetwork->IELength - _FIXED_IE_LENGTH_);
	if (p != NULL && ielen > 0) {
		ielen += 2;

		premainder_ie = p + ielen;

		ie_offset = (sint)(p - pie);

		remainder_ielen = pnetwork->IELength - ie_offset - ielen;

		dst_ie = p;
	} else
		return;

	if (remainder_ielen > 0) {
		pbackup_remainder_ie = rtw_malloc(remainder_ielen);
		if (pbackup_remainder_ie && premainder_ie)
			_rtw_memcpy(pbackup_remainder_ie, premainder_ie, remainder_ielen);
	}

	/* copy remainder IE */
	if (pbackup_remainder_ie) {
		_rtw_memcpy(dst_ie, pbackup_remainder_ie, remainder_ielen);

		rtw_mfree(pbackup_remainder_ie, remainder_ielen);
	}

	offset = (uint)(dst_ie - pie);
	pnetwork->IELength = offset + remainder_ielen;
}

void rtw_remove_rsv_bcn_ie(_adapter *padapter, WLAN_BSSID_EX *pnetwork, u8 index, u8 flag)
{
	u8 *p, *dst_ie = NULL, *premainder_ie = NULL, *pbackup_remainder_ie = NULL;
	uint offset, ielen, ie_offset, remainder_ielen = 0;
	u8	*pie = pnetwork->IEs;
	u8	del=0;

	p = rtw_get_ie(pie + _FIXED_IE_LENGTH_, index, &ielen, pnetwork->IELength - _FIXED_IE_LENGTH_);
	if (p != NULL && ielen > 0) {
		ielen += 2;

		premainder_ie = p + ielen;

		ie_offset = (sint)(p - pie);

		remainder_ielen = pnetwork->IELength - ie_offset - ielen;

		dst_ie = p;

		if(index == WLAN_EID_EXTENSION && *(p+2) == flag)
			del = 1;
	} else
		return;

	if (remainder_ielen > 0 && del == 1) {
		pbackup_remainder_ie = rtw_malloc(remainder_ielen);
		if (pbackup_remainder_ie && premainder_ie)
			_rtw_memcpy(pbackup_remainder_ie, premainder_ie, remainder_ielen);
	}
	else
		return;

	/* copy remainder IE */
	if (pbackup_remainder_ie) {
		_rtw_memcpy(dst_ie, pbackup_remainder_ie, remainder_ielen);

		rtw_mfree(pbackup_remainder_ie, remainder_ielen);
	}

	offset = (uint)(dst_ie - pie);
	pnetwork->IELength = offset + remainder_ielen;
}

void rtw_remove_bcn_ie_ex(_adapter *padapter, WLAN_BSSID_EX *pnetwork, u8 index, u8* pindex_ex, u8 index_ex_len)
{
	u8 *p, *dst_ie = NULL, *premainder_ie = NULL, *pbackup_remainder_ie = NULL;
	uint offset, ielen, ie_offset, remainder_ielen = 0;
	u8	*pie = pnetwork->IEs;

	p = rtw_get_ie_ex(pie + _FIXED_IE_LENGTH_, pnetwork->IELength - _FIXED_IE_LENGTH_, index, pindex_ex, index_ex_len, NULL, &ielen);
	if (p != NULL && ielen > 0) {
		premainder_ie = p + ielen;

		ie_offset = (sint)(p - pie);

		remainder_ielen = pnetwork->IELength - ie_offset - ielen;

		dst_ie = p;
	} else
		return;

	if (remainder_ielen > 0) {
		pbackup_remainder_ie = rtw_malloc(remainder_ielen);
		if (pbackup_remainder_ie && premainder_ie)
			_rtw_memcpy(pbackup_remainder_ie, premainder_ie, remainder_ielen);
	}

	/* copy remainder IE */
	if (pbackup_remainder_ie) {
		_rtw_memcpy(dst_ie, pbackup_remainder_ie, remainder_ielen);

		rtw_mfree(pbackup_remainder_ie, remainder_ielen);
	}

	offset = (uint)(dst_ie - pie);
	pnetwork->IELength = offset + remainder_ielen;
}

u32 rtw_no_sta_alive_check = 0;

u8 chk_sta_is_alive(struct sta_info *psta);
u8 chk_sta_is_alive(struct sta_info *psta)
{
	u8 ret = _FALSE;
#ifdef DBG_EXPIRATION_CHK
	RTW_INFO("sta:"MAC_FMT", rssi:%d, rx:"STA_PKTS_FMT", expire_to:%u, %s%ssq_len:%u\n"
		 , MAC_ARG(psta->phl_sta->mac_addr)
		 , 0 /* TODO: psta->phl_sta->hal_sta->rssi_stat.rssi */
		 /* , STA_RX_PKTS_ARG(psta) */
		 , STA_RX_PKTS_DIFF_ARG(psta)
		 , psta->expire_to
		 , psta->state & WIFI_SLEEP_STATE ? "PS, " : ""
		 , psta->state & WIFI_STA_ALIVE_CHK_STATE ? "SAC, " : ""
		 , ATOMIC_READ(&psta->sta_xmitpriv.txq_total_len)
		);
#endif
	/* if(sta_last_rx_pkts(psta) == sta_rx_pkts(psta)) */
	if (   (   psta->sta_stats.last_rx_data_pkts
	        != psta->sta_stats.rx_data_pkts)
	    || (   psta->sta_stats.last_rx_ctrl_pkts
		!= psta->sta_stats.rx_ctrl_pkts)
		|| (   psta->sta_stats.last_tx_ok_cnt
		!= psta->sta_stats.tx_ok_cnt)
	    ||  rtw_no_sta_alive_check) {
		ret = _TRUE;
	} else {
		#if 0
		if (psta->state & WIFI_SLEEP_STATE)
			ret = _TRUE;
		#endif
	}

#ifdef CONFIG_RTW_MESH
	if (MLME_IS_MESH(psta->padapter)) {
		u8 bcn_alive, hwmp_alive;

		hwmp_alive = (psta->sta_stats.rx_hwmp_pkts !=
			      psta->sta_stats.last_rx_hwmp_pkts);
		bcn_alive = (psta->sta_stats.rx_beacon_pkts !=
			     psta->sta_stats.last_rx_beacon_pkts);
		/* The reference for nexthop_lookup */
		psta->alive = ret || hwmp_alive || bcn_alive;
		/* The reference for expire_timeout_chk */
		/* Exclude bcn_alive to avoid a misjudge condition
		   that a peer unexpectedly leave and restart quickly*/
		ret = ret || hwmp_alive;
	}
#endif /* CONFIG_RTW_MESH */

	sta_update_last_rx_pkts_apmode(psta);

	return ret;
}

/**
 * issue_aka_chk_frame - issue active keep alive check frame
 *	aka = active keep alive
 */
#ifdef CONFIG_ACTIVE_KEEP_ALIVE_CHECK
static int issue_aka_chk_frame(_adapter *adapter, struct sta_info *psta)
{
	int ret = _FAIL;
	u8 *target_addr = psta->phl_sta->mac_addr;

	if (MLME_IS_AP(adapter)) {
		/* issue null data to check sta alive */
		if (psta->state & WIFI_SLEEP_STATE)
			ret = issue_nulldata(adapter, target_addr, 0, 1, 50);
		else
			ret = issue_nulldata(adapter, target_addr, 0, 3, 50);
	}

#ifdef CONFIG_RTW_MESH
	if (MLME_IS_MESH(adapter)) {
		struct rtw_mesh_path *mpath;

		rtw_rcu_read_lock();
		mpath = rtw_mesh_path_lookup(adapter, target_addr);
		if (!mpath) {
			mpath = rtw_mesh_path_add(adapter, target_addr);
			if (IS_ERR(mpath)) {
				rtw_rcu_read_unlock();
				RTW_ERR(FUNC_ADPT_FMT" rtw_mesh_path_add for "MAC_FMT" fail.\n",
					FUNC_ADPT_ARG(adapter), MAC_ARG(target_addr));
				return _FAIL;
			}
		}
		if (mpath->flags & RTW_MESH_PATH_ACTIVE)
			ret = _SUCCESS;
		else {
			u8 flags = RTW_PREQ_Q_F_START | RTW_PREQ_Q_F_PEER_AKA;
			/* issue PREQ to check peer alive */
			rtw_mesh_queue_preq(mpath, flags);
			ret = _FALSE;
		}
		rtw_rcu_read_unlock();
	}
#endif
	return ret;
}
#endif

#ifdef CONFIG_CTC_FEATURE
void _ctc_sta_rssi_check(_adapter *padapter)
{
	_list	*phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct wifi_mib_priv *wifi_mib = &padapter->registrypriv.wifi_mib;
	u8 rssi;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	/* check asoc_queue */
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		if (wifi_mib->roaming_enable
			&& (psta->expire_to > 0)) {
			rssi = rtw_phl_get_sta_rssi(psta->phl_sta);
			if(psta->link_time >= wifi_mib->roaming_start_time){
				if(rssi < wifi_mib->roaming_rssi_th1
					&& !psta->roaming_indicate){
					if(psta->roaming_wait_cnt >= wifi_mib->roaming_wait_time){
						psta->roaming_indicate = 1;
#ifdef CONFIG_WLAN_EVENT_INDICATE_GENL
						genl_wlan_IndicateEvent(padapter, EVENT_CTC_ROAMING_STA_RSSI_ALARM, psta);
#endif
						psta->roaming_wait_cnt = 0;
					} else {
						psta->roaming_wait_cnt += 2;
					}
				} else if(rssi > wifi_mib->roaming_rssi_th2 && psta->roaming_indicate == 1){
					if(psta->roaming_wait_cnt >= wifi_mib->roaming_wait_time){
						psta->roaming_indicate = 0;
#ifdef CONFIG_WLAN_EVENT_INDICATE_GENL
						genl_wlan_IndicateEvent(padapter, EVENT_CTC_ROAMING_STA_RSSI_ALARM, psta);
#endif
						psta->roaming_wait_cnt = 0;
					} else{
						psta->roaming_wait_cnt += 2;
					}
				} else{
					psta->roaming_wait_cnt = 0;
				}
			} else {
				if (rssi < wifi_mib->roaming_rssi_th1){
					psta->roaming_wait_cnt += 2;
				} else{
					psta->roaming_wait_cnt = 0;
				}
			}
		} else{
			psta->roaming_wait_cnt = 0;
		}
	}

	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
}
#endif

#ifdef CONFIG_ONE_TXQ
u32 _cal_iface_atmtime(_adapter *padapter)
{
	struct dvobj_priv *dvobj = padapter->dvobj;
	int i;
	u32 total_atm_time = 0;
	u32 cal_atm_time = 100;
	u8 zero_atm_iface_nums = 0;

	if (padapter->atm_ifsettime)
		return padapter->atm_ifsettime;

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (dvobj->padapters[i]->netif_up && (dvobj->padapters[i]->stapriv.asoc_sta_count > 1)) {
			if (dvobj->padapters[i]->atm_ifsettime == 0)
				zero_atm_iface_nums++;
			else
				total_atm_time += dvobj->padapters[i]->atm_ifsettime;
		}
	}

	if (zero_atm_iface_nums && total_atm_time < 100)
		cal_atm_time = (100 - total_atm_time)/zero_atm_iface_nums;

	return cal_atm_time?cal_atm_time:1;
}


void _atm_check_statime(_adapter *padapter)
{
	_list	*phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct dvobj_priv *dvobj = padapter->dvobj;
	u32 atm_sta_nonzero_num = 0;
	u32 atm_sta_total_num = 0;
	u32 atm_sta_total_time = 0;
	int i;
	u32 atm_if_time;

	if (padapter->dvobj->tx_mode != 2)
		return;

	padapter->atm_ifusetime = _cal_iface_atmtime(padapter);
	atm_if_time = dvobj->txq_hw_timeout*padapter->atm_ifusetime/100;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	/* check asoc_queue */
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		atm_sta_total_num++;
		for (i = 0; i < NUM_STA; i++) {
			if (padapter->atm_sta_info[i].atm_statime) {
				if (_rtw_memcmp(padapter->atm_sta_info[i].hwaddr,
						psta->phl_sta->mac_addr, ETH_ALEN) == _TRUE) {
					psta->sta_xmitpriv.ts_limit = atm_if_time*padapter->atm_sta_info[i].atm_statime/100;
					atm_sta_nonzero_num++;
					atm_sta_total_time += padapter->atm_sta_info[i].atm_statime;
					break;
				}
			}
		}
		if (i == NUM_STA)
			psta->sta_xmitpriv.ts_limit = 0;
	}

	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	/* check asoc_queue */
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		if (atm_sta_total_time >= 100) {
			if (psta->sta_xmitpriv.ts_limit)
				psta->sta_xmitpriv.ts_limit = psta->sta_xmitpriv.ts_limit*100/atm_sta_total_time;
			else
				psta->sta_xmitpriv.ts_limit = atm_if_time/atm_sta_total_time;

		} else if (atm_sta_total_time >= 0 && atm_sta_total_time < 100) {
			if (atm_sta_nonzero_num == atm_sta_total_num)
				psta->sta_xmitpriv.ts_limit = psta->sta_xmitpriv.ts_limit*100/atm_sta_total_time;
			else if (!psta->sta_xmitpriv.ts_limit)
				psta->sta_xmitpriv.ts_limit = atm_if_time*(100 - atm_sta_total_time)/(atm_sta_total_num - atm_sta_nonzero_num)/100;
		}

		if (psta->sta_xmitpriv.ts_limit == 0)
			psta->sta_xmitpriv.ts_limit = 1;

		psta->sta_xmitpriv.txq_limit = dvobj->txq_max_enq_len*psta->sta_xmitpriv.ts_limit/dvobj->txq_hw_timeout;
		psta->sta_xmitpriv.ts_ratio = psta->sta_xmitpriv.ts_limit*padapter->atm_ifusetime/atm_if_time;
	}

	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
}
#endif

#ifdef CONFIG_RTW_PACS
u8 period_count=0;
void check_period_ss_status(_adapter *padapter)
{
	struct wifi_mib_priv *mib = NULL;
	struct dvobj_priv *dvobj = NULL;
	struct sta_priv *psta_priv = NULL;
	u8 idx=0;
	_adapter *iface = NULL;

	mib = &padapter->registrypriv.wifi_mib;
	if(mib->dacs == 1) { //is scanning
		return;
	} else if(mib->dacs == 0) {
		period_count ++;
	} else {
		RTW_INFO("[DACS] Don't support this dacs mode!\n");
		return;
	}

	dvobj = adapter_to_dvobj(padapter);
	if(mib->pacs_period < 60)
		mib->pacs_period = 60; //unit: s

	if(period_count*2 >= mib->pacs_period) {
		period_count = 0;
		for (idx = 0; idx < dvobj->iface_nums; idx++) {
			iface = dvobj->padapters[idx];

			if (!iface)
				continue;

			psta_priv = &iface->stapriv;

			if(psta_priv->asoc_sta_count-1 > 0) {
				RTW_INFO("[DACS] Change mib dacs to 1!\n");
				mib->dacs = 1;
				break;
			}
		}
	}

	return;
}
#endif

void feature_expire_timer(_adapter *padapter)
{
	struct mlme_ext_priv *mlmeextpriv = &padapter->mlmeextpriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	if (is_primary_adapter(padapter)) {
		rtw_get_env_rpt(padapter);
#ifdef CONFIG_RTW_ACS
#ifdef CONFIG_RTW_PACS
		if (padapter->registrypriv.wifi_mib.pacs == 1) {
			check_period_ss_status(padapter);
		}
#endif
#ifdef CONFIG_RTW_DACS
		if (padapter->registrypriv.wifi_mib.dacs == 1) {
			rtw_dacs_mnt_trigger(padapter);
		}
#endif /* CONFIG_RTW_DACS */
#endif /* CONFIG_RTW_ACS */
	}

#ifdef CONFIG_WLAN_MANAGER
	rtw_netlink_send_intf_rpt_msg(padapter);
	rtw_netlink_send_time_tick_msg(padapter);
#endif
#ifdef CONFIG_BAND_STEERING
	_band_steering_expire(padapter);
#endif
#ifdef CONFIG_RTW_MULTI_AP
	if(padapter->registrypriv.wifi_mib.multiap_cu_threshold != 0)
		core_map_ch_util_trigger(padapter);
#endif
#ifdef CONFIG_RTW_80211K
	rtw_rm_nb_rpt_expire(padapter);
#endif
#ifdef CONFIG_CTC_FEATURE
	_ctc_sta_rssi_check(padapter);
#endif
#ifdef CONFIG_ONE_TXQ
	_atm_check_statime(padapter);
#endif
	return;
}

#if 0
static void sta_dynamic_control(_adapter *adapter, struct sta_info* sta)
{
#ifdef CONFIG_TXSC_AMSDU
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	u32 tp_th_high = (WIFI_ROLE_IS_ON_5G(adapter)) ? 200 : 100;
	u32 tp_th_mid = (WIFI_ROLE_IS_ON_5G(adapter)) ? 10 : 10;
	u32 cur_tp = sta->sta_stats.tx_tp_kbits >> 10;
	u8 rssi = sta->phl_sta->hal_sta->rssi_stat.rssi >> 1;
	u16 tx_rate = sta->cur_tx_data_rate;
#endif

	if (!adapter || !adapter->phl_role)
		return;

#ifdef CONFIG_TXSC_AMSDU
	if (sta->txsc_amsdu_max == 0) /* disable amsdu */
		sta->txsc_amsdu_num = 0;
	else {
		if (pxmitpriv->txsc_amsdu_force_num)
			sta->txsc_amsdu_num = pxmitpriv->txsc_amsdu_force_num;
			//(sta->txsc_amsdu_max <= pxmitpriv->txsc_amsdu_force_num) ? sta->txsc_amsdu_max : pxmitpriv->txsc_amsdu_force_num;
		else if (cur_tp > tp_th_high || rssi >= 30)
			sta->txsc_amsdu_num = sta->txsc_amsdu_max;
		else if (cur_tp > tp_th_mid || (rssi < 30 && rssi >= 20)) {
			sta->txsc_amsdu_num = sta->txsc_amsdu_max / 2;
			if (sta->txsc_amsdu_num == 1)
				sta->txsc_amsdu_num = 0;
		} else
			sta->txsc_amsdu_num = 0; /* disable amsdu over ampdu when low tp */
	}
#endif
}

void _update_sta_tx_status(_adapter *adapter, struct sta_info *sta)
{
	u32 cur_tx_rate = 0;
	u32 cur_tx_gi = 0;
	u8 res = _FAIL;

	/*  ToDo: should use API to get tx rate & gi from hal */
	res = get_cmac_8852ae(adapter, sta->phl_sta->macid, "DATARATE", &cur_tx_rate);
	if(res == _SUCCESS)
		sta->cur_tx_data_rate = cur_tx_rate;
	res = get_cmac_8852ae(adapter, sta->phl_sta->macid, "DATA_GI_LTF", &cur_tx_gi);
	if (res == _SUCCESS)

		sta->cur_tx_gi_ltf = cur_tx_gi;
}


/*TP_avg(t) = (1/10) * TP_avg(t-1) + (9/10) * TP(t) MBps*/
static void collect_sta_traffic_statistics(_adapter *adapter)
{
	_list	*phead, *plist;
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct	mlme_priv	*pmlmepriv = &adapter->mlmepriv;
	u64 curr_tx_bytes = 0, curr_rx_bytes = 0;
	u32 curr_tx_mbytes = 0, curr_rx_mbytes = 0;
	u64 curr_rx_pkts = 0, curr_rx_retry_pkts = 0;
	struct sta_info *sta = NULL;
	u8 *mybssid  = get_bssid(pmlmepriv);

	if (MLME_IS_STA(adapter)
		&& (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE
		|| check_fwstate(pmlmepriv, WIFI_UNDER_LINKING) == _TRUE))
	{
		if(!_rtw_memcmp(mybssid, "\x0\x0\x0\x0\x0\x0", ETH_ALEN))
		{
			sta = rtw_get_stainfo(pstapriv, mybssid);
			if(sta && sta->phl_sta && !is_broadcast_mac_addr(sta->phl_sta->mac_addr))
			{
				curr_tx_bytes = 0;
				curr_rx_bytes = 0;
				if (sta->sta_stats.last_tx_bytes > sta->sta_stats.tx_bytes)
					sta->sta_stats.last_tx_bytes = sta->sta_stats.tx_bytes;
				if (sta->sta_stats.last_rx_bytes > sta->sta_stats.rx_bytes)
					sta->sta_stats.last_rx_bytes = sta->sta_stats.rx_bytes;

				curr_tx_bytes = sta->sta_stats.tx_bytes - sta->sta_stats.last_tx_bytes;
				curr_rx_bytes = sta->sta_stats.rx_bytes - sta->sta_stats.last_rx_bytes;

				sta->sta_stats.tx_tp_kbits = (curr_tx_bytes * 8 / 2) >> 10;/*Kbps*/
				sta->sta_stats.rx_tp_kbits = (curr_rx_bytes * 8 / 2) >> 10;/*Kbps*/

				sta->sta_stats.last_tx_bytes = sta->sta_stats.tx_bytes;
				sta->sta_stats.last_rx_bytes = sta->sta_stats.rx_bytes;

			}
		}

	}
	else
	{
		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

		phead = &pstapriv->asoc_list;
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			sta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);

			if (sta && sta->phl_sta && !is_broadcast_mac_addr(sta->phl_sta->mac_addr)) {
				/* reset counters */
				curr_tx_bytes = 0;
				curr_rx_bytes = 0;
				curr_tx_mbytes = 0;
				curr_rx_mbytes = 0;
				curr_rx_pkts = 0;
				curr_rx_retry_pkts = 0;
				#ifndef CONFIG_RTW_LINK_PHL_MASTER
				// 6d31e273d960455dae3c6f10186cbef456703961 ystang
				sta->sta_stats.tx_data_retry_pkts = rtw_phl_get_hw_cnt_tx_retry_sta(adapter->dvobj->phl, sta->phl_sta->macid);
				// 1bde794936d38fb11d75d96df62872d6fd410ad3 ystang
				sta->sta_stats.tx_data_fail_pkts = rtw_phl_get_hw_cnt_tx_fail_sta(adapter->dvobj->phl, sta->phl_sta->macid);
				#endif /* CONFIG_RTW_LINK_PHL_MASTER */
				if (sta->sta_stats.last_tx_bytes > sta->sta_stats.tx_bytes)
					sta->sta_stats.last_tx_bytes =  sta->sta_stats.tx_bytes;
				if (sta->sta_stats.last_rx_bytes > sta->sta_stats.rx_bytes)
					sta->sta_stats.last_rx_bytes = sta->sta_stats.rx_bytes;
				if (sta->sta_stats.last_rx_bc_bytes > sta->sta_stats.rx_bc_bytes)
					sta->sta_stats.last_rx_bc_bytes = sta->sta_stats.rx_bc_bytes;
				if (sta->sta_stats.last_rx_mc_bytes > sta->sta_stats.rx_mc_bytes)
					sta->sta_stats.last_rx_mc_bytes = sta->sta_stats.rx_mc_bytes;
				if (sta->sta_stats.last_rx_data_pkts > sta->sta_stats.rx_data_pkts)
					sta->sta_stats.last_rx_data_pkts = sta->sta_stats.rx_data_pkts;
				if (sta->sta_stats.last_tx_pkts > sta->sta_stats.tx_pkts)
					sta->sta_stats.last_tx_pkts = sta->sta_stats.tx_pkts;
				if (sta->sta_stats.last_rx_data_retry_pkts > sta->sta_stats.rx_data_retry_pkts)
					sta->sta_stats.last_rx_data_retry_pkts = sta->sta_stats.rx_data_retry_pkts;
				if (sta->sta_stats.last_tx_data_retry_pkts > sta->sta_stats.tx_data_retry_pkts)
					sta->sta_stats.last_tx_data_retry_pkts = sta->sta_stats.tx_data_retry_pkts;
				if (sta->sta_stats.last_tx_data_fail_pkts > sta->sta_stats.tx_data_fail_pkts)
					sta->sta_stats.last_tx_data_fail_pkts = sta->sta_stats.tx_data_fail_pkts;
				curr_tx_bytes = sta->sta_stats.tx_bytes - sta->sta_stats.last_tx_bytes;
				curr_rx_bytes = sta->sta_stats.rx_bytes - sta->sta_stats.last_rx_bytes;
				sta->sta_stats.rx_data_pkts_cur = sta->sta_stats.rx_data_pkts - sta->sta_stats.last_rx_data_pkts;
				sta->sta_stats.tx_data_pkts_cur = sta->sta_stats.tx_pkts - sta->sta_stats.last_tx_pkts;
				sta->sta_stats.rx_data_retry_pkts_cur = sta->sta_stats.rx_data_retry_pkts - sta->sta_stats.last_rx_data_retry_pkts;
				sta->sta_stats.tx_data_retry_pkts_cur = sta->sta_stats.tx_data_retry_pkts - sta->sta_stats.last_tx_data_retry_pkts;
				sta->sta_stats.tx_data_fail_pkts_cur = sta->sta_stats.tx_data_fail_pkts - sta->sta_stats.last_tx_data_fail_pkts;

				sta->sta_stats.tx_tp_kbits = (curr_tx_bytes * 8 / 2) >> 10;/*Kbps*/
				sta->sta_stats.rx_tp_kbits = (curr_rx_bytes * 8 / 2) >> 10;/*Kbps*/

				sta->sta_stats.smooth_tx_tp_kbits = (sta->sta_stats.smooth_tx_tp_kbits * 6 / 10) + (sta->sta_stats.tx_tp_kbits * 4 / 10);/*Kbps*/
				sta->sta_stats.smooth_rx_tp_kbits = (sta->sta_stats.smooth_rx_tp_kbits * 6 / 10) + (sta->sta_stats.rx_tp_kbits * 4 / 10);/*Kbps*/

				curr_tx_mbytes = (curr_tx_bytes / 2) >> 20;/*MBps*/
				curr_rx_mbytes = (curr_rx_bytes / 2) >> 20;/*MBps*/

				sta->phl_sta->stats.tx_moving_average_tp =
					(sta->phl_sta->stats.tx_moving_average_tp / 10) + (curr_tx_mbytes * 9 / 10); /*MBps*/

				sta->phl_sta->stats.rx_moving_average_tp =
					(sta->phl_sta->stats.rx_moving_average_tp / 10) + (curr_rx_mbytes * 9 /10); /*MBps*/

				sta->sta_stats.last_tx_bytes = sta->sta_stats.tx_bytes;
				sta->sta_stats.last_rx_bytes = sta->sta_stats.rx_bytes;
				sta->sta_stats.last_rx_bc_bytes = sta->sta_stats.rx_bc_bytes;
				sta->sta_stats.last_rx_mc_bytes = sta->sta_stats.rx_mc_bytes;
				sta->sta_stats.last_rx_data_pkts = sta->sta_stats.rx_data_pkts;
				sta->sta_stats.last_tx_pkts = sta->sta_stats.tx_pkts;
				sta->sta_stats.last_rx_data_retry_pkts = sta->sta_stats.rx_data_retry_pkts;
				sta->sta_stats.last_tx_data_retry_pkts = sta->sta_stats.tx_data_retry_pkts;
				sta->sta_stats.last_tx_data_fail_pkts = sta->sta_stats.tx_data_fail_pkts;

				_update_sta_tx_status(adapter, sta);
				sta_dynamic_control(adapter, sta);
			}

		}

		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	}
}
#else //_VW_REFINE_
static u8 sta_amsdu_num_by_rate(_adapter *adapter, struct sta_info *sta)
{
	u16 tx_rate = sta->cur_tx_data_rate;
	u8 ret = 0;
	if (adapter->txForce_enable &&
			adapter->txForce_rate != INV_TXFORCE_VAL)
		tx_rate = adapter->txForce_rate;

	switch (tx_rate) {
		case RTW_DATA_RATE_MCS7:
		case RTW_DATA_RATE_MCS14:
		case RTW_DATA_RATE_MCS15:
		case RTW_DATA_RATE_VHT_NSS2_MCS7:
		case RTW_DATA_RATE_VHT_NSS2_MCS8:
		case RTW_DATA_RATE_VHT_NSS2_MCS9:
		case RTW_DATA_RATE_HE_NSS2_MCS7:
		case RTW_DATA_RATE_HE_NSS2_MCS8:
		case RTW_DATA_RATE_HE_NSS2_MCS9:
		case RTW_DATA_RATE_HE_NSS2_MCS10:
		case RTW_DATA_RATE_HE_NSS2_MCS11:
			ret = 2;
			break;
		case RTW_DATA_RATE_VHT_NSS1_MCS7:
		case RTW_DATA_RATE_VHT_NSS1_MCS8:
		case RTW_DATA_RATE_VHT_NSS1_MCS9:
		case RTW_DATA_RATE_HE_NSS1_MCS7:
		case RTW_DATA_RATE_HE_NSS1_MCS8:
		case RTW_DATA_RATE_HE_NSS1_MCS9:
		case RTW_DATA_RATE_HE_NSS1_MCS10:
		case RTW_DATA_RATE_HE_NSS1_MCS11:
			ret = 1;
			break;
		default:
			ret = 0;
			break;
	}

	/* for MacPro IOT RX TP issue */
	if (sta->traffic_mode == TRAFFIC_MODE_RX)
		ret = 2;
#ifdef TX_BEAMFORMING
		if(adapter->registrypriv.wifi_mib.txbf_mu
			&& adapter->registrypriv.wifi_mib.txbf_mu_amsdu)
		{
			if(!rtw_phl_check_bf_entry(adapter->dvobj->phl, sta->phl_sta))
			{
				switch (tx_rate) {
					case RTW_DATA_RATE_VHT_NSS1_MCS7:
					case RTW_DATA_RATE_VHT_NSS1_MCS8:
					case RTW_DATA_RATE_VHT_NSS1_MCS9:
					case RTW_DATA_RATE_HE_NSS1_MCS7:
					case RTW_DATA_RATE_HE_NSS1_MCS8:
					case RTW_DATA_RATE_HE_NSS1_MCS9:
					case RTW_DATA_RATE_HE_NSS1_MCS10:
					case RTW_DATA_RATE_HE_NSS1_MCS11:
						ret = 2;
						break;
					default:
						break;
				}
			}
		}
#endif
	return ret;
}

#ifdef CONFIG_AMSDU_HW_TIMER
void sta_update_hw_timer_timeout(_adapter *adapter, struct sta_info *sta)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	_adapter *pri_adapter = dvobj_get_primary_adapter(dvobj);

	if(pri_adapter->amsdu_hw_timer_enable)
	{
		if(adapter->amsdu_hw_timeout)
		{
			sta->txsc_amsdu_hw_timeout = adapter->amsdu_hw_timeout;
		}
		else
		{
#if 1
			if(sta->sta_stats.tx_data_pkts_cur < 10000)
				sta->txsc_amsdu_hw_timeout = 1;
			else
				sta->txsc_amsdu_hw_timeout = 3;
#else
			if(sta->sta_stats.tx_data_pkts_cur < 1000)
			{
				sta->txsc_amsdu_hw_timeout = 1;
				sta->sta_stats.last_smooth_tx_data_pkts_pps = 0;
				sta->sta_stats.smooth_tx_data_pkts_pps = 0;
				sta->sta_stats.hw_timer_ignore_cnt = 0;
			}
			else if(sta->sta_stats.smooth_tx_data_pkts_pps < (sta->sta_stats.last_smooth_tx_data_pkts_pps * 19) / 20)
			{
				if(sta->txsc_amsdu_hw_timeout <= 2)
					sta->txsc_amsdu_hw_timeout = 1;
				else
					sta->txsc_amsdu_hw_timeout -= 2;
				sta->sta_stats.last_smooth_tx_data_pkts_pps = sta->sta_stats.smooth_tx_data_pkts_pps;
				sta->sta_stats.hw_timer_ignore_cnt = 5;
			}
			else if(sta->sta_stats.hw_timer_ignore_cnt)
			{
				sta->sta_stats.hw_timer_ignore_cnt--;
			}
			else if(sta->sta_stats.smooth_tx_data_pkts_pps > sta->sta_stats.last_smooth_tx_data_pkts_pps)
			{
				sta->txsc_amsdu_hw_timeout = sta->txsc_amsdu_hw_timeout * 2;
				if(sta->txsc_amsdu_hw_timeout > 10)
					sta->txsc_amsdu_hw_timeout = 10;
				sta->sta_stats.last_smooth_tx_data_pkts_pps = sta->sta_stats.smooth_tx_data_pkts_pps;
			}
#endif
		}
	}
}
#endif

void sta_dynamic_control(_adapter *adapter, struct sta_info *sta)
{

	struct dvobj_priv *pdvobjpriv = NULL;
	struct stainfo_stats *pstats = &sta->sta_stats;
#ifdef CONFIG_CORE_TXSC
#ifdef CONFIG_TXSC_AMSDU
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	u32 throughput_th_high = 0;
	u32 throughput_th_medium = 0;
	u32 cur_tp = 0;
#endif
#endif
#ifdef CONFIG_DYNAMIC_THROUGHPUT_ENGINE
	u32 sta_total_th = 0;
	u32 dev_total_th = 0;
#endif
	u8 is_amsdu_on = 0;
	u8 sta_amsdu_num = 0;
	u32 sta_amsdu_size = 0;

	if(!adapter || !adapter->phl_role)
		return;

	if (pstats->smooth_tx_tp_kbits > (8 << 10) &&
		pstats->smooth_tx_tp_kbits > (pstats->smooth_rx_tp_kbits << 3)) {
		sta->traffic_mode = TRAFFIC_MODE_TX;
	} else if (pstats->smooth_rx_tp_kbits > (8 << 10) &&
			   pstats->smooth_rx_tp_kbits > (pstats->smooth_tx_tp_kbits << 3)) {
		sta->traffic_mode = TRAFFIC_MODE_RX;
	} else if (pstats->rx_data_pkts_cur > 800 && pstats->tx_data_pkts_cur > 800) {
		sta->traffic_mode = TRAFFIC_MODE_BIDIR;
	} else {
		sta->traffic_mode = 0;
	}

#ifdef CONFIG_CORE_TXSC
#ifdef CONFIG_TXSC_AMSDU

	throughput_th_high = (WIFI_ROLE_IS_ON_5G(adapter)) ? 600 : 300;
	throughput_th_medium = (WIFI_ROLE_IS_ON_5G(adapter)) ? 300 : 100;
	cur_tp = sta->sta_stats.tx_tp_kbits>>10;

	/* check if amsdu is already on */
	if (sta->txsc_amsdu_num > 0)
		is_amsdu_on =  1;

	/* set amsdu num by txsc_amsdu_force_num, sta->fix_amsdu, vw_enaebl, cur_tp and tx rate */
	if (pxmitpriv->txsc_amsdu_force_num)
		sta_amsdu_num = pxmitpriv->txsc_amsdu_force_num;
	else if (sta->fix_amsdu)
		sta_amsdu_num = sta->fix_amsdu;
#ifdef CONFIG_VW_REFINE
	else if (adapter->vw_enable)
		sta_amsdu_num = adapter->tx_amsdu;
#endif
#ifdef RTW_WKARD_REFINE_DBCC_TXTP
	else if (sta->phl_sta->asoc_cap.max_amsdu_len == 2 &&
				sta->phl_sta->chandef.bw == CHANNEL_WIDTH_160 &&
				cur_tp > 1200) {
		sta_amsdu_num = AMSDU_2_MAX_NUM;
	}
#endif
	else if (cur_tp > throughput_th_high || sta_amsdu_num_by_rate(adapter, sta) == 2) {
		#if 1
		sta_amsdu_num = sta->txsc_amsdu_max;
		#else
		/* for AC STA, do not use max amsdu num when TP < High */
		if (cur_tp > throughput_th_high ||
			sta->phl_sta->asoc_cap.max_amsdu_len == 0 ||
			sta->hepriv.he_option == _TRUE)
			sta_amsdu_num = sta->txsc_amsdu_max;
		else
			sta_amsdu_num = sta->txsc_amsdu_max / 2;
		#endif
	} else if (cur_tp > throughput_th_medium || sta_amsdu_num_by_rate(adapter, sta) == 1)
		sta_amsdu_num = sta->txsc_amsdu_max / 2;
	else
		sta_amsdu_num = 1;

	/* set amsdu num mapping to amsdu size, AMSDU_BY_SIZE */
	if (sta_amsdu_num <= AMSDU_0_MAX_NUM)
		sta_amsdu_size = AMSDU_0_SIZE - CORE_TXSC_WLHDR_SIZE;
	else if (sta_amsdu_num <= AMSDU_1_MAX_NUM)
		sta_amsdu_size = AMSDU_1_SIZE - CORE_TXSC_WLHDR_SIZE;
	else
		sta_amsdu_size = AMSDU_2_SIZE - CORE_TXSC_WLHDR_SIZE;

	/* if amsdu_num = 1, disable amsdu to ampdu */
	if (sta_amsdu_num == 1)
		sta_amsdu_num = sta_amsdu_size = 0;

     /* PPS, turn off amsdu*/
	if (pxmitpriv->txsc_amsdu_force_num == 0 && sta->fix_amsdu == 0) {
		if (is_amsdu_on) {
			/* if amsdu is on and tx_pkt < amsdu_pps/2 than turn off amsdu */
			if ((sta->sta_stats.tx_data_pkts_cur / 2) < (adapter->registrypriv.wifi_mib.amsdu_pps / 2))
				sta_amsdu_num = sta_amsdu_size = 0;
		} else {
			/* if amsdu is off and tx_pkt < amsdu_pps, do not turn on amsdu */
			if ((sta->sta_stats.tx_data_pkts_cur / 2) < adapter->registrypriv.wifi_mib.amsdu_pps)
				sta_amsdu_num = sta_amsdu_size = 0;
		}
	}

#if defined(CONFIG_ETHER_PKT_AGG) && (AGGQ_SET_AGGQ_RULE==1)
	if ((WIFI_ROLE_IS_ON_5G(adapter))) {
		if ((sta->txsc_amsdu_num  != sta_amsdu_num) ||
			(sta->txsc_amsdu_size != sta_amsdu_size)) {
			set_aggQ_rule(sta->phl_sta->mac_addr, sta_amsdu_num, sta_amsdu_size);
		}
	}
#endif /* CONFIG_ETHER_PKT_AGG && AGGQ_SET_AGGQ_RULE */

	/* asign amsdu num/size to psta */
	sta->txsc_amsdu_num = sta_amsdu_num;
	sta->txsc_amsdu_size = sta_amsdu_size;
#endif
#endif

#ifdef CONFIG_BW160M_EXTREME_THROUGHPUT_RX
	if (GET_PRIMARY_ADAPTER(adapter)->registrypriv.wifi_mib.tcpack_acc) {
		if (sta->phl_sta->chandef.bw == CHANNEL_WIDTH_160) {
			if ((sta->sta_stats.rx_tp_kbits >> 10) > GET_PRIMARY_ADAPTER(adapter)->registrypriv.wifi_mib.tcpack_hithd)
				sta->extreme_rx_traffic = 1;
			else if ((sta->sta_stats.rx_tp_kbits >> 10) < GET_PRIMARY_ADAPTER(adapter)->registrypriv.wifi_mib.tcpack_lothd)
				sta->extreme_rx_traffic = 0;
		}
	} else
		sta->extreme_rx_traffic = 0;
#endif

#ifdef CONFIG_DYNAMIC_THROUGHPUT_ENGINE
	pdvobjpriv = adapter_to_dvobj(adapter);
	sta_total_th = (sta->sta_stats.tx_tp_kbits + sta->sta_stats.rx_tp_kbits) >> 10;
	dev_total_th = pdvobjpriv->traffic_stat.cur_tx_tp + pdvobjpriv->traffic_stat.cur_rx_tp + 1;

	if (adapter->vw_enable) {
		adapter->dvobj->high_tp_sta = sta;
	} else {
		if (dev_total_th && (sta_total_th*100/dev_total_th >= 25))
			adapter->dvobj->high_tp_sta = sta;
	}
#endif
#ifdef CONFIG_AMSDU_HW_TIMER
	sta_update_hw_timer_timeout(adapter, sta);
#endif

}

#ifdef CONFIG_DYNAMIC_THROUGHPUT_ENGINE
void throughput_dynamic_control(_adapter *adapter)
{
	struct sta_info* sta = NULL;
	_adapter *primary_adapter = NULL;
	extern uint rtw_wifi_mode;
	void *phl = NULL;
	struct sta_priv *pstapriv = NULL;
	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	struct rtw_phl_com_t *phl_com = NULL;
	struct ru_grp_table *rugrptable = NULL;
	struct ru_common_ctrl *ru_ctrl = NULL;
	#endif

	if(!adapter || !adapter->dvobj)
		return;

	if (rtw_wifi_mode || adapter->dvobj->wmm_mode)
		return;

	if (!adapter->dvobj->high_tp_sta)
		return;

	primary_adapter = dvobj_get_primary_adapter(adapter->dvobj);
	if (!primary_adapter || primary_adapter->registrypriv.manual_edca)
		return;

	phl = adapter->dvobj->phl;
	pstapriv = &adapter->stapriv;
	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	phl_com = GET_HAL_DATA(adapter->dvobj);
	rugrptable = &phl_com->rugrptable;
	ru_ctrl = &rugrptable->ru_ctrl;
	#endif

	sta = adapter->dvobj->high_tp_sta;
	if (sta->sta_stats.tx_tp_kbits > 4 * sta->sta_stats.rx_tp_kbits)
		adapter->dvobj->th_mode = 1;
	else
		adapter->dvobj->th_mode = 0;

	/* John TODO: should change to phl API later */
	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	if (ru_ctrl->tbl_exist == 0)
	#endif
	{
		if (adapter->dvobj->th_mode == 1) {
			#ifdef CONFIG_VW_REFINE
			if (adapter->vw_enable != 0) {
				if (adapter->vw_enable == 2)
					rtw_hw_set_edca(adapter, 0, 0x1109);
				else
					rtw_hw_set_edca(adapter, 0, 0x5e4425);
			}
			else
			#endif
			{
				if (WIFI_ROLE_IS_ON_24G(adapter))
					rtw_hw_set_edca(adapter, 0, 0x138641f);
				else {
				    if (sta->phl_sta->wmode == WLAN_MD_11A_AC) {
					#ifdef RTW_WKARD_MBA_AC_TX_IOT
					    if (sta->vendor == HT_IOT_PEER_APPLE)
						    rtw_hw_set_edca(adapter, 0, 0x5ea44f);
					    else
					#endif
					#ifdef RTW_WKARD_APSEC_MAXTXTP
						if (MLME_IS_AP(adapter) &&
							adapter->dvobj->ic_id == RTL8832BR &&
							adapter->securitypriv.dot11PrivacyAlgrthm != _NO_PRIVACY_ &&
							(sta->sta_stats.tx_tp_kbits>>10 > 433))
							rtw_hw_set_edca(adapter, 0, 0x5e652b);
						else
					#endif
							rtw_hw_set_edca(adapter, 0, 0x138641f);
					}
					#ifdef CONFIG_BW160M_EXTREME_THROUGHPUT_TX
					else if (sta->phl_sta->chandef.bw == CHANNEL_WIDTH_160 &&
							(sta->sta_stats.tx_tp_kbits>>10) > 1200) {
						#ifdef RTW_WKARD_APSEC_MAXTXTP
						if (adapter->securitypriv.dot11PrivacyAlgrthm == _NO_PRIVACY_)
							rtw_hw_set_edca(adapter, 0, 0x5e642f);
						else
						#endif
							rtw_hw_set_edca(adapter, 0, 0x5e652f);
					}
					#endif
					else {
					#ifdef RTW_WKARD_APSEC_MAXTXTP
						if (MLME_IS_AP(adapter) &&
							adapter->dvobj->ic_id == RTL8832BR &&
							adapter->securitypriv.dot11PrivacyAlgrthm != _NO_PRIVACY_ &&
							(sta->sta_stats.tx_tp_kbits>>10 > 600))
							rtw_hw_set_edca(adapter, 0, 0x5e652b);
						else
					#endif
							rtw_hw_set_edca(adapter, 0, 0x5e642b);
					}
				}
			}
		} else {
			if (sta->vendor == HT_IOT_PEER_APPLE &&
				sta->phl_sta->wmode == WLAN_MD_11A_AC)
				rtw_hw_set_edca(adapter, 0, 0x321f);
			else
				rtw_hw_set_edca(adapter, 0, 0x642b);
		}
	}
}
#endif

void _update_sta_tx_status(_adapter *adapter, struct sta_info *sta)
{
	struct rtw_phl_rainfo ra_info;
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
#ifdef CONFIG_ONE_TXQ
	u32 rate;
	unsigned char tmp_rate[20] = {0};
	u32 temp_rate_mbps_retry;
#endif

	status = rtw_phl_query_rainfo(adapter->dvobj->phl, sta->phl_sta, &ra_info);
	if (status == RTW_PHL_STATUS_SUCCESS) {
		sta->cur_tx_data_rate = ra_info.rate;
		sta->cur_tx_gi_ltf = ra_info.gi_ltf;
		sta->cur_tx_bw = ra_info.bw;
		#ifdef CONFIG_WFA_OFDMA_Logo_Test
		rtw_phl_ru_cal_he_su_tx_info(sta->phl_sta, ra_info.rate);
		#endif
	}

#ifdef CONFIG_ONE_TXQ
	get_current_rate(sta, 1, &rate, tmp_rate);
	if (0 == rate) {
		if (WIFI_ROLE_IS_ON_24G(adapter))
			rate = 1;
		else
			rate = 6;
	}
	sta->sta_xmitpriv.tx_rate_mbps = rate;
	temp_rate_mbps_retry = sta->sta_xmitpriv.tx_rate_mbps*100/(100 + sta->sta_stats.tx_retry_ratio);
	if (temp_rate_mbps_retry)
		sta->sta_xmitpriv.tx_rate_mbps_retry = temp_rate_mbps_retry;
	else
		sta->sta_xmitpriv.tx_rate_mbps_retry = 1;

	if (sta->sta_xmitpriv.txq_service_update != sta->sta_xmitpriv.txq_service_previous)
		sta->sta_xmitpriv.txq_avg_dequeue = sta->sta_xmitpriv.txq_cur_dequeue/(sta->sta_xmitpriv.txq_service_update - sta->sta_xmitpriv.txq_service_previous);
	sta->sta_xmitpriv.txq_service_previous = sta->sta_xmitpriv.txq_service_update;
	sta->sta_xmitpriv.txq_cur_dequeue = 0;
#endif
}

void _check_sta_continuous_icverr(_adapter *adapter, struct sta_info *sta)
{
	struct wifi_mib_priv *wifi_mib = &adapter->registrypriv.wifi_mib;
	struct stainfo_stats *sta_stats = &sta->sta_stats;

	if (wifi_mib->kick_icverr && (sta->sta_stats.rx_data_pkts_cur == 0)) {
		if (sta_stats->rx_icverr_pkts) {
			sta_stats->rx_conti_icverr_pkts += sta_stats->rx_icverr_pkts;
			sta_stats->rx_conti_icverr_times++;
		}
		/* If continuous icv error pkts larger than threshold, and over 10 seconds no normal rx pkts,
		   delete this STA */
		if ((sta_stats->rx_conti_icverr_pkts > wifi_mib->kick_icverr) &&
			(sta_stats->rx_conti_icverr_times >= 5) &&
			(sta->leave_conti_icverr == 0)) {
			sta->leave_conti_icverr = 1;
			RTW_WARN("["MAC_FMT"]rx_conti_icv_error = (%d, %d), AP is going to delete STA\n",
				MAC_ARG(sta->phl_sta->mac_addr),
				sta_stats->rx_conti_icverr_times,
				sta_stats->rx_conti_icverr_pkts);
			report_sta_timeout_event(adapter,
						sta->phl_sta->mac_addr,
						WLAN_REASON_UNSPECIFIED);
			sta_stats->rx_conti_icverr_pkts = 0;
		}
	} else {
		sta->leave_conti_icverr = 0;
		sta_stats->rx_conti_icverr_pkts = 0;
		sta_stats->rx_conti_icverr_times = 0;
	}
	sta_stats->rx_icverr_pkts = 0;
}

void fast_check_sta_disconnect(_adapter *adapter, struct sta_info *sta)
{
	u32 fail_cnt_thr = adapter->registrypriv.wifi_mib.fast_leave_thr;
	u32 cur_tp = sta->sta_stats.tx_tp_kbits >> 10;
	unsigned long passtime = rtw_get_passing_time_ms(sta->sta_stats.last_rx_time);

	RTW_INFO("[%s] (macid/txnok/tp/passtime : %d/%d/%d/%ld)\n", __func__,
				sta->phl_sta->macid, sta->fast_txnok_chk, cur_tp, passtime);
	if (!fail_cnt_thr || passtime < 2000
#ifdef CONFIG_RTW_MULTI_AP
	|| adapter->multi_ap_mode == MAP_MODE_FRONT_BACKHAUL_AP
#endif
	) {
		sta->fast_txnok_chk = 0;
		return;
	}

	if (sta->fast_txnok_chk > fail_cnt_thr && cur_tp == 0) {
		RTW_PRINT("[%s] check STA(macid/txnok:%d/%d) status is disconnected\n",
					__func__, sta->phl_sta->macid, sta->fast_txnok_chk);
		ap_free_sta(adapter, sta, _FALSE, WLAN_REASON_DEAUTH_LEAVING, _TRUE, _FALSE);
	}

}


/*TP_avg(t) = (1/10) * TP_avg(t-1) + (9/10) * TP(t) MBps*/
void collect_sta_traffic_statistics(_adapter *adapter, struct sta_info *sta)
{
	u64 curr_tx_bytes = 0, curr_rx_bytes = 0;
	u32 curr_tx_mbytes = 0, curr_rx_mbytes = 0;
	u64 curr_rx_pkts = 0, curr_rx_retry_pkts = 0;
	u32 tx_ok_cnt[PHL_AC_QUEUE_TOTAL];
	u32 tx_fail_cnt[PHL_AC_QUEUE_TOTAL];
	u32 tx_ra_retry_cnt[PHL_AC_QUEUE_TOTAL];
	u32 tx_ra_ok_cnt[PHL_AC_QUEUE_TOTAL];
	u32 tx_ok_cnt_total = 0;
	u32 tx_fail_cnt_total = 0;
	u32 tx_ra_retry_total_cnt = 0;
	u32 tx_ra_ok_total_cnt = 0;
	u8 i;

	#ifdef CTC_WIFI_DIAG
	u32 maxTxFailCnt = 300; 	// MAX Tx fail packet count
	u32 minTxFailCnt = 30;		// MIN Tx fail packet count; this value should be less than maxTxFailCnt.
	u32 txFailSecThr = 3;		// threshold of Tx Fail Time (in second)
	#endif

	_rtw_memset(&tx_ok_cnt, 0, sizeof(u32)*PHL_AC_QUEUE_TOTAL);
	_rtw_memset(&tx_fail_cnt, 0, sizeof(u32)*PHL_AC_QUEUE_TOTAL);
	_rtw_memset(&tx_ra_retry_cnt, 0, sizeof(u32)*PHL_AC_QUEUE_TOTAL);
	_rtw_memset(&tx_ra_ok_cnt, 0, sizeof(u32)*PHL_AC_QUEUE_TOTAL);

	if (sta && !is_broadcast_mac_addr(sta->phl_sta->mac_addr)) {
		rtw_phl_get_tx_ra_retry_rpt(adapter->dvobj->phl, sta->phl_sta,
					    tx_ra_retry_cnt, PHL_AC_QUEUE_TOTAL, 1);
		rtw_phl_get_tx_ra_ok_rpt(adapter->dvobj->phl, sta->phl_sta,
					 tx_ra_ok_cnt, PHL_AC_QUEUE_TOTAL, 1);
		for (i = 0; i < PHL_AC_QUEUE_TOTAL; i++) {
			tx_ra_retry_total_cnt += tx_ra_retry_cnt[i];
			tx_ra_ok_total_cnt += tx_ra_ok_cnt[i];
		}
		if (tx_ra_ok_total_cnt + tx_ra_retry_total_cnt)
			sta->sta_stats.tx_retry_ratio = (tx_ra_retry_total_cnt * 100) / (tx_ra_ok_total_cnt + tx_ra_retry_total_cnt);
		else
			sta->sta_stats.tx_retry_ratio = 0;
		if (sta->sta_stats.last_tx_bytes > sta->sta_stats.tx_bytes)
			sta->sta_stats.last_tx_bytes =  sta->sta_stats.tx_bytes;
		if (sta->sta_stats.last_rx_bytes > sta->sta_stats.rx_bytes)
			sta->sta_stats.last_rx_bytes = sta->sta_stats.rx_bytes;
		if (sta->sta_stats.last_rx_bc_bytes > sta->sta_stats.rx_bc_bytes)
			sta->sta_stats.last_rx_bc_bytes = sta->sta_stats.rx_bc_bytes;
		if (sta->sta_stats.last_rx_mc_bytes > sta->sta_stats.rx_mc_bytes)
			sta->sta_stats.last_rx_mc_bytes = sta->sta_stats.rx_mc_bytes;
		if (sta->sta_stats.last_rx_data_pkts > sta->sta_stats.rx_data_pkts)
			sta->sta_stats.last_rx_data_pkts = sta->sta_stats.rx_data_pkts;
		if (sta->sta_stats.last_tx_pkts > sta->sta_stats.tx_pkts)
			sta->sta_stats.last_tx_pkts = sta->sta_stats.tx_pkts;
		if (sta->sta_stats.last_rx_data_retry_pkts > sta->sta_stats.rx_data_retry_pkts)
			sta->sta_stats.last_rx_data_retry_pkts = sta->sta_stats.rx_data_retry_pkts;

		curr_tx_bytes = sta->sta_stats.tx_bytes - sta->sta_stats.last_tx_bytes;
		curr_rx_bytes = sta->sta_stats.rx_bytes - sta->sta_stats.last_rx_bytes;
		sta->sta_stats.rx_data_pkts_cur = sta->sta_stats.rx_data_pkts - sta->sta_stats.last_rx_data_pkts;
		sta->sta_stats.tx_data_pkts_cur = sta->sta_stats.tx_pkts - sta->sta_stats.last_tx_pkts;
		sta->sta_stats.rx_data_retry_pkts_cur = sta->sta_stats.rx_data_retry_pkts - sta->sta_stats.last_rx_data_retry_pkts;
		sta->sta_stats.tx_data_retry_pkts_cur = tx_ra_retry_total_cnt;

		sta->sta_stats.tx_tp_kbits = (curr_tx_bytes * 8 / 2) >> 10;/*Kbps*/
		sta->sta_stats.rx_tp_kbits = (curr_rx_bytes * 8 / 2) >> 10;/*Kbps*/

		sta->sta_stats.smooth_tx_tp_kbits = (sta->sta_stats.smooth_tx_tp_kbits * 6 / 10) + (sta->sta_stats.tx_tp_kbits * 4 / 10);/*Kbps*/
		sta->sta_stats.smooth_rx_tp_kbits = (sta->sta_stats.smooth_rx_tp_kbits * 6 / 10) + (sta->sta_stats.rx_tp_kbits * 4 / 10);/*Kbps*/
#ifdef CONFIG_AMSDU_HW_TIMER
		sta->sta_stats.smooth_tx_data_pkts_pps = (sta->sta_stats.smooth_tx_data_pkts_pps * 3 / 10) + (sta->sta_stats.tx_data_pkts_cur * 7 / 10);/*Kbps*/
#endif

		curr_tx_mbytes = (curr_tx_bytes / 2) >> 20;/*MBps*/
		curr_rx_mbytes = (curr_rx_bytes / 2) >> 20;/*MBps*/

		sta->phl_sta->stats.tx_moving_average_tp =
			(sta->phl_sta->stats.tx_moving_average_tp / 10) + (curr_tx_mbytes * 9 / 10); /*MBps*/

		sta->phl_sta->stats.rx_moving_average_tp =
			(sta->phl_sta->stats.rx_moving_average_tp / 10) + (curr_rx_mbytes * 9 /10); /*MBps*/

		sta->sta_stats.last_tx_bytes = sta->sta_stats.tx_bytes;
		sta->sta_stats.last_rx_bytes = sta->sta_stats.rx_bytes;
		sta->sta_stats.last_rx_bc_bytes = sta->sta_stats.rx_bc_bytes;
		sta->sta_stats.last_rx_mc_bytes = sta->sta_stats.rx_mc_bytes;
		sta->sta_stats.last_rx_data_pkts = sta->sta_stats.rx_data_pkts;
		sta->sta_stats.last_tx_pkts = sta->sta_stats.tx_pkts;
		sta->sta_stats.last_rx_data_retry_pkts = sta->sta_stats.rx_data_retry_pkts;
		sta->sta_stats.last_tx_data_retry_pkts = sta->sta_stats.tx_data_retry_pkts;
		sta->sta_stats.tx_data_retry_pkts += tx_ra_retry_total_cnt;
		adapter->xmitpriv.tx_data_retry += tx_ra_retry_total_cnt;
		#ifdef CONFIG_VW_REFINE
		adapter->vw_retry_cnt[sta->phl_sta->macid] = sta->sta_stats.tx_data_retry_pkts;
		#endif /* CONFIG_VW_REFINE */
		rtw_phl_get_tx_ok_rpt(adapter->dvobj->phl, sta->phl_sta, tx_ok_cnt, PHL_AC_QUEUE_TOTAL);
		rtw_phl_get_tx_fail_rpt(adapter->dvobj->phl, sta->phl_sta, tx_fail_cnt, PHL_AC_QUEUE_TOTAL);
		for (i = 0; i < PHL_AC_QUEUE_TOTAL; i++) {
			tx_ok_cnt_total += tx_ok_cnt[i];
			tx_fail_cnt_total += tx_fail_cnt[i];
		}

		fast_check_sta_disconnect(adapter, sta);

#ifdef CTC_WIFI_DIAG
		if (tx_ok_cnt_total != 0) {
			sta->sta_stats.tx_conti_fail = 0;
			sta->sta_stats.tx_conti_fail_cnt = 0;
			sta->sta_stats.tx_last_good_time = adapter->up_time;
			sta->sta_stats.leave = 0;
		} else if (tx_fail_cnt_total != 0) {
			sta->sta_stats.tx_conti_fail++;
			sta->sta_stats.tx_conti_fail_cnt += tx_fail_cnt_total;

			ctcwifi_assoc_err(adapter, sta->phl_sta->mac_addr, "WiFi frames losing [tx_fail increases]\n");
			RTW_INFO( "detect: txfail=%d, tx_conti_fail_cnt=%d\n", tx_fail_cnt_total, sta->sta_stats.tx_conti_fail_cnt);

			if(((sta->sta_stats.tx_conti_fail_cnt >= maxTxFailCnt) ||
				(sta->sta_stats.tx_conti_fail_cnt >= minTxFailCnt && adapter->up_time >= (sta->sta_stats.tx_last_good_time + txFailSecThr))) &&
				(sta->sta_stats.tx_conti_fail >= 3)) {

				RTW_INFO( "** tx_conti_fail_cnt=%d (min=%d,max=%d)\n", sta->sta_stats.tx_conti_fail_cnt, minTxFailCnt, maxTxFailCnt);
				RTW_INFO( "** tx_last_good_time=%d, up_time=%d (Thr:%d)\n", (int)sta->sta_stats.tx_last_good_time, (int)adapter->up_time, txFailSecThr );
				RTW_INFO( "AP is going to del_sta %02X:%02X:%02X:%02X:%02X:%02X\n", sta->phl_sta->mac_addr[0], sta->phl_sta->mac_addr[1] ,sta->phl_sta->mac_addr[2], sta->phl_sta->mac_addr[3], sta->phl_sta->mac_addr[4], sta->phl_sta->mac_addr[5]);
				ctcwifi_assoc_err(adapter, sta->phl_sta->mac_addr, "WiFi frames losing [STA is considered as disappeared]\n");

				sta->sta_stats.tx_conti_fail = 0;
				sta->sta_stats.tx_conti_fail_cnt = 0;
				sta->sta_stats.tx_last_good_time = adapter->up_time;

				if (sta->sta_stats.leave == 0) {
					sta->sta_stats.leave = 1;
					sta->expire_to = 3;
				}
			}
		}
#endif
		sta->sta_stats.last_tx_ok_cnt = sta->sta_stats.tx_ok_cnt;
		sta->sta_stats.last_tx_fail_cnt = sta->sta_stats.tx_fail_cnt;
		sta->sta_stats.tx_ok_cnt += tx_ok_cnt_total;
		sta->sta_stats.tx_fail_cnt += tx_fail_cnt_total;

#ifdef CONFIG_PHL_TX_STATS_CORE
		rtw_phl_sync_tx_statistics(sta->phl_sta,
				sta->sta_stats.last_tx_bytes, sta->sta_stats.last_tx_bytes, sta->sta_stats.tx_tp_kbits);
#endif

		_check_sta_continuous_icverr(adapter, sta);

		_update_sta_tx_status(adapter, sta);
		sta_dynamic_control(adapter, sta);
	}

#ifdef CONFIG_DYNAMIC_THROUGHPUT_ENGINE
	throughput_dynamic_control(adapter);
#endif
}
#endif

#ifdef CONFIG_SNR_RPT
void rtw_reset_snr_statistics(struct sta_info *psta)
{
	int i;

	if(psta) {
		psta->snr_num = 0;
		for ( i = 0; i < RTW_PHL_MAX_RF_PATH; i++) {
			psta->snr_fd_total[i] = 0;
			psta->snr_td_total[i] = 0;
			psta->snr_fd_avg[i] = 0;
			psta->snr_td_avg[i]= 0;
		}
	}
}
#endif

u8 associated_sta_rssi_checked(_adapter *padapter, int *low_rssi_sta_list)
{
	u8 low_rssi_sta_num = 0;
	int stainfo_offset;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info * psta=NULL;
	struct list_head *phead=NULL, *plist=NULL;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = phead->next;
	while ((plist != phead))
	{
		psta = list_entry(plist, struct sta_info, asoc_list);
		if(psta->phl_sta &&
			(padapter->registrypriv.wifi_mib.sta_asoc_rssi_th > 5) &&
			(padapter->registrypriv.wifi_mib.sta_asoc_rssi_th - 5 > rtw_phl_get_sta_rssi(psta->phl_sta))){

			RTW_PRINT("%s : refused sta "MAC_FMT", because rssi(%d) lower than rssi_th(%d-5)!\n",
				__func__, MAC_ARG(psta->phl_sta->mac_addr), rtw_phl_get_sta_rssi(psta->phl_sta),
				padapter->registrypriv.wifi_mib.sta_asoc_rssi_th);

			stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
			if (stainfo_offset_valid(stainfo_offset))
				low_rssi_sta_list[low_rssi_sta_num++] = stainfo_offset;
		}

		plist = plist->next;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	return low_rssi_sta_num;
}

static u8 chk_auth_queue_sta_alive (_adapter *padapter)
{
	_list	*phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 beacon_updated = _FALSE;
	u8 flush_num = 0, i = 0;
	char flush_list[NUM_STA]={0};
	int stainfo_offset = 0;

	_rtw_spinlock_bh(&pstapriv->auth_list_lock);
	phead = &pstapriv->auth_list;
	plist = get_next(phead);

	/* check auth_queue */
#ifdef DBG_EXPIRATION_CHK
	if (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		RTW_INFO(FUNC_ADPT_FMT" auth_list, cnt:%u\n"
			, FUNC_ADPT_ARG(padapter), pstapriv->auth_list_cnt);
	}
#endif
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, auth_list);
		plist = get_next(plist);
		stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
		if ((--psta->expire_to <= 0) &&
			 (stainfo_offset_valid(stainfo_offset))) {
			flush_list[flush_num++] = stainfo_offset;
		}

#ifdef CONFIG_ATMEL_RC_PATCH
		if (_rtw_memcmp((void *)(pstapriv->atmel_rc_pattern), (void *)(psta->phl_sta->mac_addr), ETH_ALEN) == _TRUE)
			continue;
		if (psta->flag_atmel_rc)
			continue;
#endif
	}
	_rtw_spinunlock_bh(&pstapriv->auth_list_lock);

	for (i = 0; i < flush_num; i++) {
		psta = rtw_get_stainfo_by_offset(pstapriv, flush_list[i]);
		RTW_INFO(FUNC_ADPT_FMT" auth expire "MAC_FMT"\n"
			, FUNC_ADPT_ARG(padapter), MAC_ARG(psta->phl_sta->mac_addr));
		beacon_updated |= ap_free_sta(padapter, psta, _FALSE, WLAN_REASON_DEAUTH_LEAVING, _TRUE, _FALSE);
		psta = NULL;
	}

	return beacon_updated;
}

static u8 chk_assoc_queue_sta_alive(_adapter *padapter)
{
	_list	*phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 beacon_updated = _FALSE;
	const u8 snd_null_intvl = padapter->registrypriv.wifi_mib.inactive_timeout / 4;
	int stainfo_offset = 0, i = 0;
	u8 chk_sta_num = 0;
	char chk_sta_list[NUM_STA];
	u8 sta_addr[ETH_ALEN];
	u32 num_txreq = GET_HAL_SPEC(padapter->dvobj)->band_cap & BAND_CAP_5G ? MAX_TX_RING_NUM_5G : MAX_TX_RING_NUM_2G;
#ifdef CONFIG_ACTIVE_KEEP_ALIVE_CHECK
	u8 backup_ch = 0, backup_bw = 0, backup_offset = 0;
	u8 union_ch = 0, union_bw = 0, union_offset = 0;
	u8 switch_channel_by_drv = _TRUE;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv
#endif

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

#ifdef DBG_EXPIRATION_CHK
	if (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		RTW_INFO(FUNC_ADPT_FMT" asoc_list, cnt:%u\n"
			, FUNC_ADPT_ARG(padapter), pstapriv->asoc_list_cnt);
	}
#endif

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		if (chk_sta_is_alive(psta)
			|| (1 == padapter->registrypriv.wifi_spec)
#ifdef CONFIG_VW_REFINE
			|| (0 != padapter->vw_enable)
#endif
			) {
			psta->expire_to = pstapriv->expire_to;
			if (psta->state & WIFI_STA_ALIVE_CHK_STATE)
				psta->state ^= WIFI_STA_ALIVE_CHK_STATE;
			continue;
		}

		if (--psta->expire_to > 0) {
			if ((pstapriv->expire_to - psta->expire_to) % snd_null_intvl == 0) {
				if (MLME_IS_AP(padapter)) {
					RTW_PRINT(FUNC_ADPT_FMT" Issue null data to sta "MAC_FMT", snd_null_intvl%d\n",
						    FUNC_ADPT_ARG(padapter), MAC_ARG(psta->phl_sta->mac_addr), snd_null_intvl);
#ifdef CONFIG_ACTIVE_KEEP_ALIVE_CHECK
					stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
					if (stainfo_offset_valid(stainfo_offset))
						chk_sta_list[chk_sta_num++] = stainfo_offset;
#else
					/* issue null data to check sta alive */
					issue_nulldata(padapter,
									psta->phl_sta->mac_addr,
									1, /* Set sleep bit to wake up station in
					                    mgnt_tx_enqueue_for_sleeping_sta */
									1, 0);
#endif
					psta->state |= WIFI_STA_ALIVE_CHK_STATE;
				}
			} else {
				if (psta->state & WIFI_SLEEP_STATE
				&& ATOMIC_READ(&psta->sta_xmitpriv.txq_total_len) > (num_txreq / pstapriv->asoc_list_cnt)
			    && padapter->pfree_txreq_queue->qlen < ((num_txreq / pstapriv->asoc_list_cnt) / 2)) {
					RTW_INFO(FUNC_ADPT_FMT" sta:"MAC_FMT", sleepq_len:%u, free_txreq_cnt:%u, asoc_list_cnt:%u, clear sleep_q\n",
						FUNC_ADPT_ARG(padapter), MAC_ARG(psta->phl_sta->mac_addr),
						ATOMIC_READ(&psta->sta_xmitpriv.txq_total_len),
						padapter->pfree_txreq_queue->qlen, pstapriv->asoc_list_cnt);
					wakeup_sta_to_xmit(padapter, psta);
				}
			}

			if (psta->expire_to == 5) {
				if (psta->state & WIFI_SLEEP_STATE) {
					/* to check if alive by another methods if staion is at ps mode.
					   to update bcn with tim_bitmap for this station */
					RTW_INFO(FUNC_ADPT_FMT" Alive chk, sta:" MAC_FMT " is at ps mode!\n",
								FUNC_ADPT_ARG(padapter), MAC_ARG(psta->phl_sta->mac_addr));
					rtw_tim_map_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);
					update_beacon(padapter, _TIM_IE_, NULL, _TRUE, 0);
				}
			}
		} else {
			stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
			if (stainfo_offset_valid(stainfo_offset))
				chk_sta_list[chk_sta_num++] = stainfo_offset;
		}
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	/* check del sta list loop */
	if (chk_sta_num) {
#ifdef CONFIG_ACTIVE_KEEP_ALIVE_CHECK
		if (pmlmeext->active_keep_alive_check) {
			#ifdef CONFIG_MCC_MODE
			if (MCC_EN(padapter)) {
			/* driver doesn't switch channel under MCC */
				if (rtw_hal_check_mcc_status(padapter, MCC_STATUS_DOING_MCC))
					switch_channel_by_drv = _FALSE;
			}
			#endif
			if (!rtw_mi_get_ch_setting_union(padapter, &union_ch, &union_bw, &union_offset)
				|| pmlmeext->cur_channel != union_ch)
				switch_channel_by_drv = _FALSE;

			/* switch to correct channel of current network  before issue keep-alive frames */
			if (switch_channel_by_drv == _TRUE && rtw_get_oper_ch(padapter) != pmlmeext->cur_channel) {
				backup_ch = rtw_get_oper_ch(padapter);
				backup_bw = rtw_get_oper_bw(padapter);
				backup_offset = rtw_get_oper_choffset(padapter);
				set_channel_bwmode(padapter, union_ch, union_offset, union_bw, _FALSE);
			}
		}
#endif

		for (i = 0; i < chk_sta_num; i++) {
			psta = rtw_get_stainfo_by_offset(pstapriv, chk_sta_list[i]);

			#ifdef CONFIG_ATMEL_RC_PATCH
			if (_rtw_memcmp(pstapriv->atmel_rc_pattern, psta->phl_sta->mac_addr, ETH_ALEN) == _TRUE)
				continue;
			if (psta->flag_atmel_rc)
				continue;
			#endif

			if (!(psta->state & WIFI_ASOC_STATE))
				continue;

#ifdef CONFIG_ACTIVE_KEEP_ALIVE_CHECK
			if (pmlmeext->active_keep_alive_check) {
				/* issue active keep alive frame to check */
				if (_SUCCESS == issue_aka_chk_frame(padapter, psta)) {
					RTW_INFO(FUNC_ADPT_FMT" asoc check, "MAC_FMT" is alive\n"
								, FUNC_ADPT_ARG(padapter), MAC_ARG(psta->phl_sta->mac_addr));
					psta->expire_to = pstapriv->expire_to;
					if (psta->state & WIFI_STA_ALIVE_CHK_STATE)
						psta->state ^= WIFI_STA_ALIVE_CHK_STATE;
					continue;
				}
				if (psta->expire_to > 0) {
					continue;
				}
			}
#endif

#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
			if (psta->tbtx_enable)
				pstapriv->tbtx_asoc_list_cnt--;
#endif
			STA_SET_MESH_PLINK(psta, NULL);

			_rtw_memcpy(sta_addr, psta->phl_sta->mac_addr, ETH_ALEN);
			RTW_INFO(FUNC_ADPT_FMT" asoc expire "MAC_FMT", state=0x%x\n"
				, FUNC_ADPT_ARG(padapter), MAC_ARG(psta->phl_sta->mac_addr), psta->state);
			beacon_updated |= ap_free_sta(padapter, psta, _FALSE, WLAN_REASON_DEAUTH_LEAVING, _TRUE, _FALSE);
#ifdef CONFIG_RTW_MESH
			if (MLME_IS_MESH(padapter))
				rtw_mesh_expire_peer(padapter, sta_addr);
#endif
		}

#ifdef CONFIG_ACTIVE_KEEP_ALIVE_CHECK
		if (pmlmeext->active_keep_alive_check) {
			/* back to the original operation channel */
			if (switch_channel_by_drv == _TRUE && backup_ch > 0)
				set_channel_bwmode(padapter, backup_ch, backup_offset, backup_bw, _FALSE);
		}
#endif
	}

	return beacon_updated;
}

#ifdef WIFI_LOGO_HE_4_52_1
static void wifi_HE_logo_4_52_1_chk(_adapter *padapter, struct sta_info *psta)
{
	void *phl = padapter->dvobj->phl;

	if (!padapter->mlmeextpriv.mlmext_info.is_HE_4_52_1) {
		return;
	}

	if (psta->om_step % 2 == 0 && (psta->sta_stats.tx_tp_kbits >> 10) > 25) {
		psta->om_time++;
		if (psta->om_time >= 5) {
			psta->om_time = 0;
			psta->om_step++;
		}
	}

	if (psta->om_step % 2 == 1 && (psta->sta_stats.tx_tp_kbits >> 10) < 5) {
		psta->om_time++;
		if (psta->om_time >= 10) {
			psta->om_time = 0;
			psta->om_step++;
			switch (psta->om_step) {
			case 2:
				psta->phl_sta->asoc_cap.nss_rx = 1;
				psta->phl_sta->chandef.bw = 0;
				rtw_phl_cmd_change_stainfo(phl, psta->phl_sta, STA_CHG_RAMASK,
											NULL, 0, PHL_CMD_DIRECTLY, 0);
				break;
			case 4:
				psta->phl_sta->asoc_cap.nss_rx = 2;
				psta->phl_sta->chandef.bw = 0;
				rtw_phl_cmd_change_stainfo(phl, psta->phl_sta, STA_CHG_RAMASK,
										NULL, 0, PHL_CMD_DIRECTLY, 0);
				break;
			case 6:
				psta->phl_sta->asoc_cap.nss_rx = 1;
					psta->phl_sta->chandef.bw = 2;
					rtw_phl_cmd_change_stainfo(phl, psta->phl_sta, STA_CHG_RAMASK,
											NULL, 0, PHL_CMD_DIRECTLY, 0);
				break;
			default:
				break;
			}
		}
	}
}
#endif

#ifdef CONFIG_ADPTVTY_CONTROL
//#define U_MATCH_AGE 60
#define U_MATCH_CNT_TH 2
#define T_MATCH_CNT_TH 2
void adaptivity_dynamic_control(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &(padapter->stapriv);
	struct wifi_mib_priv *mib = &padapter->registrypriv.wifi_mib;
	struct adptvty_info *adpt = &(padapter->adpt_info);
	struct rtw_phl_com_t *phl_com = padapter->dvobj->phl_com;
	_list	*phead = NULL, *plist = NULL;
	u8  in_adptvty_test = _FALSE, in_tp_test = _FALSE;
	u8  u_match_cnt = 0, t_match_cnt = 0;
	u32 last_tcp_cnt = 0, last_udp_cnt = 0;

	u8  adaptivity_enable = mib->adptvty_en;
	u8  adaptivity_try = mib->adptvty_try;
	u32 tcp_cnt_th = mib->adptvty_th_t;
	u32 udp_cnt_th = mib->adptvty_th_u;
	u8  udp_ratio_th = mib->adptvty_ratio_u;

	u32 os_tcp_cnt = 0, os_udp_cnt = 0;
	u32 cur_tcp_cnt = 0, cur_udp_cnt = 0;
	u8  cur_udp_ratio = 0;

	u32 scan_ap_num = 0;
	u8  sta_trfc = 0;

	if(padapter == NULL)
		return;

	if (!MLME_IS_AP(padapter))
		return;

	u_match_cnt = adpt->adptvty_test_cnt;
	t_match_cnt = adpt->tp_test_cnt;

	if(adaptivity_enable != _TRUE) {
		if(u_match_cnt != 0 || t_match_cnt != 0) {
			RTW_PRINT("%s: force stop. (u:%d, t:%d)\n", __func__, u_match_cnt, t_match_cnt);
			u_match_cnt = t_match_cnt = 0;
			adpt->cnt_t_last = 0;
			adpt->cnt_u_last = 0;
			goto force_stop;
		}
		return;
	}

	/* calculate traffic statistics */
	last_tcp_cnt = adpt->cnt_t_last;
	last_udp_cnt = adpt->cnt_u_last;
	os_tcp_cnt = padapter->tx_logs.os_tx_tcp;
	os_udp_cnt = padapter->tx_logs.os_tx_udp;

	if(last_tcp_cnt > os_tcp_cnt)
		last_tcp_cnt = os_tcp_cnt;
	if(last_udp_cnt > os_udp_cnt)
		last_udp_cnt = os_udp_cnt;

	cur_tcp_cnt = os_tcp_cnt - last_tcp_cnt;
	cur_udp_cnt = os_udp_cnt - last_udp_cnt;
	if(cur_udp_cnt > 0)
		cur_udp_ratio = (u8)((cur_udp_cnt*100)/(cur_tcp_cnt+cur_udp_cnt));
	else
		cur_udp_ratio = 0;

	adpt->cnt_t = cur_tcp_cnt;
	adpt->cnt_u = cur_udp_cnt;
	adpt->rto_u = cur_udp_ratio;
	adpt->cnt_t_last = os_tcp_cnt;
	adpt->cnt_u_last = os_udp_cnt;

	RTW_DBG("%s: t=(o=%d, l=%d, c=%d, t=%d) u=(o=%d, l=%d, c=%d, t=%d) rto=(c=%d, t=%d)\n",
		__func__,
		os_tcp_cnt, last_tcp_cnt, cur_tcp_cnt, tcp_cnt_th,
		os_udp_cnt, last_udp_cnt, cur_udp_cnt, udp_cnt_th,
		cur_udp_ratio, udp_ratio_th);

	/* adaptivity check condition */
#ifdef RTW_MI_SHARE_BSS_LIST
	scan_ap_num = padapter->dvobj->num_of_scanned;
#else
	scan_ap_num = padapter->mlmepriv->num_of_scanned;
#endif /*RTW_MI_SHARE_BSS_LIST*/
	if((pstapriv->asoc_list_cnt == 1)
		|| ((pstapriv->asoc_list_cnt == 0) && (u_match_cnt != 0))
		) {

		RTW_DBG("%s: [AP] asoc_list_cnt=%d, scan_ap_num=%d\n",
			__func__, pstapriv->asoc_list_cnt, scan_ap_num);

		if(scan_ap_num == 0) {
			if(pstapriv->asoc_list_cnt == 0) {
				sta_trfc = TRAFFIC_MODE_UNKNOWN;
			} else {
				/* sta info */
				_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

				phead = &pstapriv->asoc_list;
				plist = get_next(phead);
				psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
				if(psta) {
					sta_trfc = psta->traffic_mode;
					RTW_DBG("%s: [STA] psta=%pM, trfc=%u\n", __func__, psta->phl_sta->mac_addr, sta_trfc);
				} else {
					sta_trfc = TRAFFIC_MODE_UNKNOWN;
					RTW_ERR("%s: [STA] psta is null\n", __func__);
				}

				_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
			}

			RTW_DBG("%s: [STA] sta_trfc=%d\n", __func__, sta_trfc);

			if(sta_trfc == TRAFFIC_MODE_TX) {
				if((cur_udp_cnt > udp_cnt_th) && (cur_udp_ratio >= udp_ratio_th)) {
					if(u_match_cnt < U_MATCH_CNT_TH) u_match_cnt++;
					//if(u_match_cnt >= u_match_cnt_th) u_match_cnt = U_MATCH_AGE;
					if(t_match_cnt > 0) t_match_cnt--;
				} else if(cur_tcp_cnt > tcp_cnt_th) {
					u_match_cnt = 0;
					if(t_match_cnt < T_MATCH_CNT_TH) t_match_cnt++;
				} else {
					if(u_match_cnt > 0) u_match_cnt--;
					if(t_match_cnt > 0) t_match_cnt--;
				}
			} else if (sta_trfc == TRAFFIC_MODE_UNKNOWN) {
				if(u_match_cnt > 0) u_match_cnt--;
				if(t_match_cnt > 0) t_match_cnt--;
			} else {
				u_match_cnt = t_match_cnt = 0;
			}
		} else {
			u_match_cnt = t_match_cnt = 0;
		}
	} else {
		u_match_cnt = t_match_cnt = 0;
	}

	if(adaptivity_try == _TRUE) {
		if(adpt->try == _FALSE) {
			adpt->try = _TRUE;
			RTW_PRINT("%s: try: on\n", __func__);
		}
		u_match_cnt = U_MATCH_CNT_TH/*U_MATCH_AGE*/;
		t_match_cnt = 0;
	} else {
		if(adpt->try == _TRUE) {
			adpt->try = _FALSE;
			u_match_cnt = t_match_cnt = 0;
			RTW_PRINT("%s: try: off\n", __func__);
		}
	}

force_stop:
	in_adptvty_test = adpt->adptvty_test;
	in_tp_test = adpt->tp_test;

	if((u_match_cnt == 0) && (in_adptvty_test == _TRUE))
		in_adptvty_test = _FALSE;
	else if((u_match_cnt >= U_MATCH_CNT_TH) && (in_adptvty_test == _FALSE))
		in_adptvty_test = _TRUE;

	if((t_match_cnt == 0) && (in_tp_test == _TRUE))
		in_tp_test = _FALSE;
	else if((t_match_cnt == T_MATCH_CNT_TH) && (in_tp_test == _FALSE))
		in_tp_test = _TRUE;

	RTW_DBG("%s: adptvty=%u(c:%u), tp=%u(c:%u)\n",
		__func__, in_adptvty_test, u_match_cnt, in_tp_test, t_match_cnt);

	adpt->adptvty_test_cnt = u_match_cnt;
	adpt->tp_test_cnt = t_match_cnt;

	if(in_adptvty_test != adpt->adptvty_test) {
		RTW_PRINT("%s: adptvty: %u->%u\n",
			__func__, adpt->adptvty_test, in_adptvty_test);

		adpt->adptvty_test = in_adptvty_test;
		if(in_adptvty_test == _TRUE) {
			/* dis txop */
			padapter->registrypriv.manual_edca = 1;
			rtw_hw_set_edca(padapter, 0, 0x642b);
			/* en edcca */
			phl_com->edcca_mode = 1;
			mib->edcca_mode = 1;
		}
		else
		{
			/* en txop */
			padapter->registrypriv.manual_edca = 0;
			/* dis edcca */
			phl_com->edcca_mode = 0;
			mib->edcca_mode = 0;
		}
	}

	if(in_tp_test != adpt->tp_test) {
		RTW_PRINT("%s: tp: %u->%u\n",
			__func__, adpt->tp_test, in_tp_test);

		adpt->tp_test = in_tp_test;
	}

	return;
}
#endif /*CONFIG_ADPTVTY_CONTROL*/

__IMEM_WLAN_SECTION__
void expire_timeout_chk(_adapter *padapter)
{
	_list	*phead, *plist;
	u8 updated = _FALSE;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 low_rssi_sta_num = 0;
	int low_rssi_sta_list[NUM_STA]={0};
#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
	_adapter *vxd_padapter = NULL, *tmp_padapter = NULL;
	struct sta_info *crossband_psta = NULL;
	u16 clm_result;
	u16 ch_utilization;
#endif
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
#ifdef CONFIG_WFA_OFDMA_Logo_Test
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
#endif
	void *phl = GET_HAL_INFO(dvobj);
	u32 sleep_map = 0;
	u32 offset = 0;
	u32 trx_wdg_fail_cnt = rtw_phl_get_trx_wdg_fail_cnt(phl);

#ifdef CONFIG_RTW_MESH
	if (MLME_IS_MESH(padapter)
		&& check_fwstate(&padapter->mlmepriv, WIFI_ASOC_STATE)) {
		struct rtw_mesh_cfg *mcfg = &padapter->mesh_cfg;
		rtw_mesh_path_expire(padapter);

		/* TBD: up layer timeout mechanism */
		/* if (!mcfg->plink_timeout)
			return; */
#ifndef CONFIG_ACTIVE_KEEP_ALIVE_CHECK
		return;
#endif
	}
#endif

#ifdef CONFIG_MCC_MODE
	/*	then driver may check fail due to not recv client's frame under sitesurvey,
	 *	don't expire timeout chk under MCC under sitesurvey */
	if (rtw_hal_mcc_link_status_chk(padapter, __func__) == _FALSE)
		return;
#endif

	padapter->up_time += 2;
#ifdef CONFIG_VW_REFINE
	padapter->small_pkt = 0;
	padapter->big_pkt = 0;
#endif

#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
	if (is_primary_adapter(padapter)) {
			rtw_crossband_update_status(padapter);
	}
#endif

	reset_accumulated_tp(padapter);

	/*check every assoc station status*/
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
#ifdef DBG_EXPIRATION_CHK
	if (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		RTW_INFO(FUNC_ADPT_FMT" asoc_list, cnt:%u\n"
			, FUNC_ADPT_ARG(padapter), pstapriv->asoc_list_cnt);
	}
#endif

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		psta->link_time += 2; /* link time add 2 secs */

#ifdef CONFIG_ATMEL_RC_PATCH
		RTW_INFO("%s:%d  psta=%p, %02x,%02x||%02x,%02x  \n\n", __func__,  __LINE__,
			psta, pstapriv->atmel_rc_pattern[0], pstapriv->atmel_rc_pattern[5], psta->phl_sta->mac_addr[0], psta->phl_sta->mac_addr[5]);
		if (_rtw_memcmp((void *)pstapriv->atmel_rc_pattern, (void *)(psta->phl_sta->mac_addr), ETH_ALEN) == _TRUE)
			continue;
		if (psta->flag_atmel_rc)
			continue;
		RTW_INFO("%s: debug line:%d\n", __func__, __LINE__);
#endif
#ifdef CONFIG_AUTO_AP_MODE
		if (psta->isrc)
			continue;
#endif

		if(trx_wdg_fail_cnt){
			if (!(psta->state & WIFI_SLEEP_STATE)){
				if(psta->phl_sta->macid < 32){
					sleep_map = rtw_phl_read32(phl, R_AX_MACID_SLEEP_0);
					offset = psta->phl_sta->macid;
				}else if(psta->phl_sta->macid < 64){
					sleep_map = rtw_phl_read32(phl, R_AX_MACID_SLEEP_1);
					offset = psta->phl_sta->macid - 32;
				}else if((psta->phl_sta->macid < 96)){
					sleep_map = rtw_phl_read32(phl, R_AX_MACID_SLEEP_2);
					offset = psta->phl_sta->macid - 64;
				}else if((psta->phl_sta->macid < 128)){
					sleep_map = rtw_phl_read32(phl, R_AX_MACID_SLEEP_3);
					offset = psta->phl_sta->macid - 96;
				}
				if(sleep_map & (BIT(offset))){
					RTW_PRINT("%s-%d, clr macid %d pause/sleep!\n", __func__, __LINE__, psta->phl_sta->macid);
					rtw_phl_host_getpkt(padapter->dvobj->phl, psta->phl_sta->macid, 1);
				}
			}
		}

		/* CONFIG_VW_REFINE */
		collect_sta_traffic_statistics(padapter, psta);
		//VCS_update(padapter, psta);
		if(!padapter->registrypriv.wifi_mib.rssi_ru_dump)
			display_sta_dump(padapter, psta);
#ifdef CONFIG_WFA_OFDMA_Logo_Test
		else
			display_sta_ofdma_info_dump(padapter, psta);
#endif
#ifdef CONFIG_SNR_RPT
		rtw_reset_snr_statistics(psta);
#endif

		accumulate_whole_tp(padapter, psta);
#ifdef TX_BEAMFORMING
		rtw_bf_chk_per_sta(padapter, psta);
#endif
#ifdef CONFIG_WLAN_MANAGER
		rtw_netlink_send_sta_rpt_msg(padapter, psta);
#endif
#ifdef CONFIG_RTW_MULTI_AP
	if (padapter->registrypriv.wifi_mib.multiap_report_rcpi_threshold != 0) {
		core_map_ap_sta_rssi_trigger(padapter, psta);
	}
#endif
#ifdef WIFI_LOGO_HE_4_52_1
		wifi_HE_logo_4_52_1_chk(padapter, psta);
#endif

	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

#ifdef CONFIG_PHL_TX_STATS_CORE
	rtw_phl_acc_device_tx_statistics(padapter->dvobj->phl,
			padapter->xmitpriv.tx_bytes, padapter->xmitpriv.tx_uc_pkts, padapter->tp_total_tx);
#endif
	if ((padapter->registrypriv.wifi_mib.totaltp_dump) &&
		(padapter->up_time % padapter->registrypriv.wifi_mib.totaltp_dump == 0)) {
			RTW_PRINT("(TotalTP %d)(T:%d, R:%d)\n",
				(padapter->tp_total_trx >> 10),
				(padapter->tp_total_tx >> 10),
				(padapter->tp_total_rx >> 10));
	}

#ifdef CONFIG_TX_DEFER
	{
		struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
		if(padapter->registrypriv.wifi_mib.defer_tx_sched) {

			if(pxmitpriv->defer_tx_flag) {
				rtw_phl_tx_req_notify(padapter->dvobj->phl);
				ATOMIC_SET(&pxmitpriv->defer_tx_cnt, 0);
			}

			if((padapter->tp_total_tx >> 10) > padapter->registrypriv.wifi_mib.defer_tx_tp) {
				pxmitpriv->defer_tx_flag = 1;
				_set_timer(&pxmitpriv->tx_defer_timer, 500);
			} else {
				ATOMIC_SET(&pxmitpriv->defer_tx_cnt, 0);
				pxmitpriv->defer_tx_flag = 0;
				_cancel_timer_ex(&pxmitpriv->tx_defer_timer);
			}
		} else {
			pxmitpriv->defer_tx_flag = 0;
		}
	}
#endif

#ifdef CONFIG_RTW_MULTI_AP
	rtw_blacklist_expire(&padapter->black_list);
#endif /* CONFIG_RTW_MULTI_AP */
#if 0
	if(padapter->registrypriv.wifi_mib.sta_asoc_rssi_th){
		low_rssi_sta_num = associated_sta_rssi_checked(padapter, low_rssi_sta_list);
		for (i = 0; i < low_rssi_sta_num; i++) {
			psta = rtw_get_stainfo_by_offset(pstapriv, low_rssi_sta_list[i]);
			updated |= ap_free_sta(padapter, psta, _FALSE, WLAN_REASON_DEAUTH_LEAVING, _TRUE, _FALSE);
			psta = NULL;
		}
	}
#endif

	updated |= chk_auth_queue_sta_alive(padapter);
	updated |= chk_assoc_queue_sta_alive(padapter);
	if (updated)
		associated_clients_update(padapter, updated, STA_INFO_UPDATE_ALL);
	else
		associated_clients_update(padapter, 1, STA_INFO_UPDATE_PROTECTION_MODE);

#ifdef CONFIG_ADPTVTY_CONTROL
	adaptivity_dynamic_control(padapter);
#endif /* CONFIG_ADPTVTY_CONTROL */
}

void rtw_ap_update_sta_ra_info(_adapter *padapter, struct sta_info *psta)
{
	unsigned char sta_band = 0;
	u64 tx_ra_bitmap = 0;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX *pcur_network = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;

	if (!psta)
		return;

	if (!(psta->state & WIFI_ASOC_STATE))
		return;
/* ToDo: RA may move to hal_com */
#if 0
	update_sta_ra_info(padapter, psta);
	tx_ra_bitmap = psta->phl_sta->ra_info.ramask;

	if (pcur_network->Configuration.DSConfig > 14) {

		if (tx_ra_bitmap & 0xffff000)
			sta_band |= WIRELESS_11_5N;

		if (tx_ra_bitmap & 0xff0)
			sta_band |= WIRELESS_11A;

		/* 5G band */
#ifdef CONFIG_80211AC_VHT
		if (psta->vhtpriv.vht_option)
			sta_band = WIRELESS_11_5AC;
#endif
	} else {
		if (tx_ra_bitmap & 0xffff000)
			sta_band |= WIRELESS_11_24N;

		if (tx_ra_bitmap & 0xff0)
			sta_band |= WIRELESS_11G;

		if (tx_ra_bitmap & 0x0f)
			sta_band |= WIRELESS_11B;
	}

	psta->wireless_mode = sta_band;
	update_sta_wset(padapter, psta);
	RTW_INFO("%s=> mac_id:%d , tx_ra_bitmap:0x%016llx, networkType:0x%02x\n",
			__FUNCTION__, psta->phl_sta->macid, tx_ra_bitmap, psta->wireless_mode);
#endif
}

#ifdef CONFIG_BMC_TX_RATE_SELECT
u8 rtw_ap_find_mini_tx_rate(_adapter *adapter)
{
	_list	*phead, *plist;
	u8 mini_tx_rate = DESC_RATEVHTSS4MCS9, sta_tx_rate;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &adapter->stapriv;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		sta_tx_rate = psta->phl_sta->ra_info.curr_tx_rate & 0x7F;
		if (sta_tx_rate < mini_tx_rate)
			mini_tx_rate = sta_tx_rate;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	return mini_tx_rate;
}

u8 rtw_ap_find_bmc_rate(_adapter *adapter, u8 tx_rate)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter_to_dvobj(adapter));
	u8 tx_ini_rate = DESC_RATE6M;

	switch (tx_rate) {
	case DESC_RATEVHTSS3MCS9:
	case DESC_RATEVHTSS3MCS8:
	case DESC_RATEVHTSS3MCS7:
	case DESC_RATEVHTSS3MCS6:
	case DESC_RATEVHTSS3MCS5:
	case DESC_RATEVHTSS3MCS4:
	case DESC_RATEVHTSS3MCS3:
	case DESC_RATEVHTSS2MCS9:
	case DESC_RATEVHTSS2MCS8:
	case DESC_RATEVHTSS2MCS7:
	case DESC_RATEVHTSS2MCS6:
	case DESC_RATEVHTSS2MCS5:
	case DESC_RATEVHTSS2MCS4:
	case DESC_RATEVHTSS2MCS3:
	case DESC_RATEVHTSS1MCS9:
	case DESC_RATEVHTSS1MCS8:
	case DESC_RATEVHTSS1MCS7:
	case DESC_RATEVHTSS1MCS6:
	case DESC_RATEVHTSS1MCS5:
	case DESC_RATEVHTSS1MCS4:
	case DESC_RATEVHTSS1MCS3:
	case DESC_RATEMCS15:
	case DESC_RATEMCS14:
	case DESC_RATEMCS13:
	case DESC_RATEMCS12:
	case DESC_RATEMCS11:
	case DESC_RATEMCS7:
	case DESC_RATEMCS6:
	case DESC_RATEMCS5:
	case DESC_RATEMCS4:
	case DESC_RATEMCS3:
	case DESC_RATE54M:
	case DESC_RATE48M:
	case DESC_RATE36M:
	case DESC_RATE24M:
		tx_ini_rate = DESC_RATE24M;
		break;
	case DESC_RATEVHTSS3MCS2:
	case DESC_RATEVHTSS3MCS1:
	case DESC_RATEVHTSS2MCS2:
	case DESC_RATEVHTSS2MCS1:
	case DESC_RATEVHTSS1MCS2:
	case DESC_RATEVHTSS1MCS1:
	case DESC_RATEMCS10:
	case DESC_RATEMCS9:
	case DESC_RATEMCS2:
	case DESC_RATEMCS1:
	case DESC_RATE18M:
	case DESC_RATE12M:
		tx_ini_rate = DESC_RATE12M;
		break;
	case DESC_RATEVHTSS3MCS0:
	case DESC_RATEVHTSS2MCS0:
	case DESC_RATEVHTSS1MCS0:
	case DESC_RATEMCS8:
	case DESC_RATEMCS0:
	case DESC_RATE9M:
	case DESC_RATE6M:
		tx_ini_rate = DESC_RATE6M;
		break;
	case DESC_RATE11M:
	case DESC_RATE5_5M:
	case DESC_RATE2M:
	case DESC_RATE1M:
		tx_ini_rate = DESC_RATE1M;
		break;
	default:
		tx_ini_rate = DESC_RATE6M;
		break;
	}

	if (hal_data->current_band_type == BAND_ON_5G)
		if (tx_ini_rate < DESC_RATE6M)
			tx_ini_rate = DESC_RATE6M;

	return tx_ini_rate;
}

void rtw_update_bmc_sta_tx_rate(_adapter *adapter)
{
	struct sta_info *psta = NULL;
	u8 tx_rate;

	psta = rtw_get_bcmc_stainfo(adapter);
	if (psta == NULL) {
		RTW_ERR(ADPT_FMT "could not get bmc_sta !!\n", ADPT_ARG(adapter));
		return;
	}

	if (adapter->bmc_tx_rate != MGN_UNKNOWN) {
		psta->init_rate = adapter->bmc_tx_rate;
		goto _exit;
	}

	if (adapter->stapriv.asoc_sta_count <= 2)
		goto _exit;

	tx_rate = rtw_ap_find_mini_tx_rate(adapter);
	#ifdef CONFIG_BMC_TX_LOW_RATE
	tx_rate = rtw_ap_find_bmc_rate(adapter, tx_rate);
	#endif

	psta->init_rate = hwrate_to_mrate(tx_rate);

_exit:
	RTW_INFO(ADPT_FMT" BMC Tx rate - %s\n", ADPT_ARG(adapter), MGN_RATE_STR(psta->init_rate));
}
#endif

void rtw_init_bmc_sta_tx_rate(_adapter *padapter, struct sta_info *psta)
{
/* ToDo: need API to query hal_sta->ra_info.ramask */
#if 0
#ifdef CONFIG_BMC_TX_LOW_RATE
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
#endif
	u8 rate_idx = 0;
	u8 brate_table[] = {MGN_1M, MGN_2M, MGN_5_5M, MGN_11M,
		MGN_6M, MGN_9M, MGN_12M, MGN_18M, MGN_24M, MGN_36M, MGN_48M, MGN_54M};

	if (!MLME_IS_AP(padapter) && !MLME_IS_MESH(padapter))
		return;

	if (padapter->bmc_tx_rate != MGN_UNKNOWN)
		psta->init_rate = padapter->bmc_tx_rate;
	else {
		#ifdef CONFIG_BMC_TX_LOW_RATE
		if (IsEnableHWOFDM(pmlmeext->cur_wireless_mode) && (psta->phl_sta->ra_info.ramask && 0xFF0))
			rate_idx = get_lowest_rate_idx_ex(psta->phl_sta->ra_info.ramask, 4); /*from basic rate*/
		else
			rate_idx = get_lowest_rate_idx(psta->phl_sta->ra_info.ramask); /*from basic rate*/
		#else
		rate_idx = get_highest_rate_idx(psta->phl_sta->ra_info.ramask); /*from basic rate*/
		#endif
		if (rate_idx < 12)
			psta->init_rate = brate_table[rate_idx];
		else
			psta->init_rate = MGN_1M;
	}
#endif
	RTW_INFO(ADPT_FMT" BMC Init Tx rate - %s\n", ADPT_ARG(padapter), MGN_RATE_STR(psta->init_rate));
}

void update_bmc_sta(_adapter *padapter)
{
	unsigned char	network_type;
	int supportRateNum = 0;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX *pcur_network = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;
	struct sta_info *psta = rtw_get_bcmc_stainfo(padapter);

	if (psta) {
		psta->phl_sta->aid = 0;/* default set to 0 */
#ifdef CONFIG_RTW_MESH
		if (MLME_IS_MESH(padapter))
			psta->qos_option = 1;
		else
#endif
			psta->qos_option = 0;
#ifdef CONFIG_80211N_HT
		psta->htpriv.ht_option = _FALSE;
#endif /* CONFIG_80211N_HT */

		psta->ieee8021x_blocked = 0;

		_rtw_memset((void *)&psta->sta_stats, 0, sizeof(struct stainfo_stats));

		/* psta->dot118021XPrivacy = _NO_PRIVACY_; */ /* !!! remove it, because it has been set before this. */

		supportRateNum = rtw_get_rateset_len((u8 *)&pcur_network->SupportedRates);
		network_type = rtw_check_network_type((u8 *)&pcur_network->SupportedRates, supportRateNum, pcur_network->Configuration.DSConfig);
		if (is_supported_tx_cck(network_type))
			network_type = WLAN_MD_11B;
		else if (network_type == WLAN_MD_INVALID) { /* error handling */
			if (pcur_network->Configuration.DSConfig > 14)
				network_type = WLAN_MD_11A;
			else
				network_type = WLAN_MD_11B;
		}
		update_sta_basic_rate(psta, network_type);
		psta->phl_sta->wmode = network_type;

		update_sta_ra_info(padapter, psta);

		_rtw_spinlock_bh(&psta->lock);
		psta->state = WIFI_ASOC_STATE;
		_rtw_spinunlock_bh(&psta->lock);

		rtw_sta_media_status_rpt(padapter, psta, 1);
		rtw_init_bmc_sta_tx_rate(padapter, psta);

	} else
		RTW_INFO("add_RATid_bmc_sta error!\n");

}

#if defined(CONFIG_80211N_HT) && defined(CONFIG_BEAMFORMING)
void update_sta_info_apmode_ht_bf_cap(_adapter *padapter, struct sta_info *psta)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct ht_priv	*phtpriv_ap = &pmlmepriv->htpriv;
	struct ht_priv	*phtpriv_sta = &psta->htpriv;

	u8 cur_beamform_cap = 0;

	/*Config Tx beamforming setting*/
	if (TEST_FLAG(phtpriv_ap->beamform_cap, BEAMFORMING_HT_BEAMFORMEE_ENABLE) &&
		GET_HT_CAP_TXBF_EXPLICIT_COMP_STEERING_CAP((u8 *)(&phtpriv_sta->ht_cap))) {
		SET_FLAG(cur_beamform_cap, BEAMFORMING_HT_BEAMFORMER_ENABLE);
		/*Shift to BEAMFORMING_HT_BEAMFORMEE_CHNL_EST_CAP*/
		SET_FLAG(cur_beamform_cap, GET_HT_CAP_TXBF_CHNL_ESTIMATION_NUM_ANTENNAS((u8 *)(&phtpriv_sta->ht_cap)) << 6);
	}

	if (TEST_FLAG(phtpriv_ap->beamform_cap, BEAMFORMING_HT_BEAMFORMER_ENABLE) &&
		GET_HT_CAP_TXBF_EXPLICIT_COMP_FEEDBACK_CAP((u8 *)(&phtpriv_sta->ht_cap))) {
		SET_FLAG(cur_beamform_cap, BEAMFORMING_HT_BEAMFORMEE_ENABLE);
		/*Shift to BEAMFORMING_HT_BEAMFORMER_STEER_NUM*/
		SET_FLAG(cur_beamform_cap, GET_HT_CAP_TXBF_COMP_STEERING_NUM_ANTENNAS((u8 *)(&phtpriv_sta->ht_cap)) << 4);
	}
	if (cur_beamform_cap)
		RTW_INFO("Client STA(%d) HT Beamforming Cap = 0x%02X\n", psta->phl_sta->aid, cur_beamform_cap);

	phtpriv_sta->beamform_cap = cur_beamform_cap;
	psta->phl_sta->bf_info.ht_beamform_cap = cur_beamform_cap;

}
#endif /*CONFIG_80211N_HT && CONFIG_BEAMFORMING*/

static u8 check_agg_en_apmode(_adapter *padapter)
{
	if(IS_DATA_RATE_CCK(GET_FIX_RATE(padapter->fix_rate)) ||
		IS_DATA_RATE_OFDM(GET_FIX_RATE(padapter->fix_rate))){
		return _FALSE;
	}else{
		return _TRUE;
	}
}

/* notes:
 * AID: 1~MAX for sta and 0 for bc/mc in ap/adhoc mode  */
void update_sta_info_apmode(_adapter *padapter, struct sta_info *psta)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
#ifdef CONFIG_80211N_HT
	struct ht_priv	*phtpriv_ap = &pmlmepriv->htpriv;
	struct ht_priv	*phtpriv_sta = &psta->htpriv;
#endif /* CONFIG_80211N_HT */
	u8	cur_ldpc_cap = 0;
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	struct protocol_cap_t *role_cap = &(wrole->proto_role_cap);
	u8 check_agg_en = 0;
	/* set intf_tag to if1 */
	/* psta->intf_tag = 0; */

	RTW_INFO("%s\n", __FUNCTION__);
	if(!psta || !psta->phl_sta)
		return;

	/*alloc macid when call rtw_alloc_stainfo(),release macid when call rtw_free_stainfo()*/

	if (!MLME_IS_MESH(padapter) &&
	    ((psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X)
#ifdef CONFIG_RTL_CFG80211_WAPI_SUPPORT
	     || (psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_WAPI)
#endif
	    ))
		psta->ieee8021x_blocked = _TRUE;
	else
		psta->ieee8021x_blocked = _FALSE;

	/* update sta's cap */

	psta->phl_sta->chandef.chan = pmlmeext->cur_channel;
	psta->phl_sta->chandef.band = (psta->phl_sta->chandef.chan > 14) ? BAND_ON_5G : BAND_ON_24G;

	/* ERP */
	VCS_update(padapter, psta);

	check_agg_en = check_agg_en_apmode(padapter);
	if(check_agg_en){
		padapter->xmitpriv.txsc_amsdu_enable = 1;
	}else{
		padapter->xmitpriv.txsc_amsdu_enable = 0;
	}
#ifdef CONFIG_80211N_HT
	/* HT related cap */
	if (phtpriv_sta->ht_option && check_agg_en) {
		/* check if sta supports rx ampdu */
		phtpriv_sta->ampdu_enable = phtpriv_ap->ampdu_enable;

		phtpriv_sta->rx_ampdu_min_spacing = (phtpriv_sta->ht_cap.ampdu_params_info & IEEE80211_HT_CAP_AMPDU_DENSITY) >> 2;

		RTW_INFO("A-MPDU min spacing: %u\n", phtpriv_sta->rx_ampdu_min_spacing);
		psta->phl_sta->asoc_cap.ampdu_density = psta->htpriv.rx_ampdu_min_spacing;

		/* bwmode */
		if ((phtpriv_sta->ht_cap.cap_info & phtpriv_ap->ht_cap.cap_info) & cpu_to_le16(IEEE80211_HT_CAP_SUP_WIDTH))
			psta->phl_sta->chandef.bw = CHANNEL_WIDTH_40;
		else
			psta->phl_sta->chandef.bw = CHANNEL_WIDTH_20;

		if (phtpriv_sta->op_present
			&& !GET_HT_OP_ELE_STA_CHL_WIDTH(phtpriv_sta->ht_op))
			psta->phl_sta->chandef.bw = CHANNEL_WIDTH_20;

		if (psta->ht_40mhz_intolerant)
			psta->phl_sta->chandef.bw = CHANNEL_WIDTH_20;

		if (pmlmeext->cur_bwmode < psta->phl_sta->chandef.bw)
			psta->phl_sta->chandef.bw = pmlmeext->cur_bwmode;

		phtpriv_sta->ch_offset = pmlmeext->cur_ch_offset;


		/* check if sta support s Short GI 20M */
		if ((phtpriv_sta->ht_cap.cap_info & phtpriv_ap->ht_cap.cap_info) & cpu_to_le16(IEEE80211_HT_CAP_SGI_20))
			phtpriv_sta->sgi_20m = _TRUE;

		/* check if sta support s Short GI 40M */
		if ((phtpriv_sta->ht_cap.cap_info & phtpriv_ap->ht_cap.cap_info) & cpu_to_le16(IEEE80211_HT_CAP_SGI_40)) {
			if (psta->phl_sta->chandef.bw == CHANNEL_WIDTH_40) /* according to psta->bw_mode */
				phtpriv_sta->sgi_40m = _TRUE;
			else
				phtpriv_sta->sgi_40m = _FALSE;
		}

		psta->qos_option = _TRUE;

		/* B0 Config LDPC Coding Capability */
		if (TEST_FLAG(phtpriv_ap->ldpc_cap, LDPC_HT_ENABLE_TX) &&
		    GET_HT_CAP_ELE_LDPC_CAP((u8 *)(&phtpriv_sta->ht_cap))) {
			SET_FLAG(cur_ldpc_cap, (LDPC_HT_ENABLE_TX | LDPC_HT_CAP_TX));
			RTW_INFO("Enable HT Tx LDPC for STA(%d)\n", psta->phl_sta->aid);
		}

		/* B7 B8 B9 Config STBC setting */
		if (TEST_FLAG(phtpriv_ap->stbc_cap, STBC_HT_ENABLE_TX))
			psta->phl_sta->asoc_cap.stbc_ht_rx = GET_HT_CAP_ELE_RX_STBC((u8 *)(&phtpriv_sta->ht_cap));
		else
			psta->phl_sta->asoc_cap.stbc_ht_rx = 0;

		#ifdef CONFIG_BEAMFORMING
		update_sta_info_apmode_ht_bf_cap(padapter, psta);
		#endif

		psta->phl_sta->asoc_cap.nss_rx = psta->phl_sta->asoc_cap.nss_tx = rtw_ht_mcsset_to_nss(psta->htpriv.ht_cap.supp_mcs_set);
	} else {
		phtpriv_sta->ampdu_enable = _FALSE;

		phtpriv_sta->sgi_20m = _FALSE;
		phtpriv_sta->sgi_40m = _FALSE;
		psta->phl_sta->chandef.bw = CHANNEL_WIDTH_20;
		phtpriv_sta->ch_offset = CHAN_OFFSET_NO_EXT;
	}

	phtpriv_sta->ldpc_cap = cur_ldpc_cap;

	/* Rx AMPDU */
	send_delba(padapter, 0, psta->phl_sta->mac_addr);/* recipient */

	/* TX AMPDU */
	send_delba(padapter, 1, psta->phl_sta->mac_addr);/*  */ /* originator */
	phtpriv_sta->agg_enable_bitmap = 0x0;/* reset */
	phtpriv_sta->candidate_tid_bitmap = 0x0;/* reset */
#endif /* CONFIG_80211N_HT */

#ifdef CONFIG_80211AC_VHT
	update_sta_vht_info_apmode(padapter, psta);
#endif

#ifdef CONFIG_80211AX_HE
	update_sta_he_info_apmode(padapter, psta);
#endif

	psta->nss_rx_assoc = psta->phl_sta->asoc_cap.nss_rx;
	if (psta->phl_sta->asoc_cap.sm_ps == SM_PS_STATIC)
		psta->phl_sta->asoc_cap.nss_rx = 1;

	psta->phl_sta->chandef.offset = (psta->phl_sta->chandef.bw > CHANNEL_WIDTH_20) ?
		pmlmeext->cur_ch_offset : CHAN_OFFSET_NO_EXT;

	/* ToDo: need API to inform hal_sta->ra_info.is_support_sgi */
	/* psta->phl_sta->ra_info.is_support_sgi = query_ra_short_GI(psta, rtw_get_tx_bw_mode(padapter, psta)); */
	update_ldpc_stbc_cap(psta);

	/* todo: init other variables */

	_rtw_memset((void *)&psta->sta_stats, 0, sizeof(struct stainfo_stats));
	psta->link_time = 0;


	/* add ratid */
	/* add_RATid(padapter, psta); */ /* move to ap_sta_info_defer_update() */

	/* ap mode */
	rtw_hal_set_phydm_var(padapter, HAL_PHYDM_STA_INFO, psta, _TRUE);

	_rtw_spinlock_bh(&psta->lock);

	/* Check encryption */
	if (!MLME_IS_MESH(padapter) && psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X)
		psta->state |= WIFI_UNDER_KEY_HANDSHAKE;

	psta->state |= WIFI_ASOC_STATE;

	_rtw_spinunlock_bh(&psta->lock);
}

void update_ap_info(_adapter *padapter, struct sta_info *psta)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX *pnetwork = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
#ifdef CONFIG_80211N_HT
	struct ht_priv	*phtpriv_ap = &pmlmepriv->htpriv;
#endif /* CONFIG_80211N_HT */

	psta->phl_sta->wmode = pmlmeext->cur_wireless_mode;

	psta->bssratelen = rtw_get_rateset_len(pnetwork->SupportedRates);
	_rtw_memcpy(psta->bssrateset, pnetwork->SupportedRates, psta->bssratelen);

#ifdef CONFIG_80211N_HT
	/* HT related cap */
	if (phtpriv_ap->ht_option) {
		/* check if sta supports rx ampdu */
		/* phtpriv_ap->ampdu_enable = phtpriv_ap->ampdu_enable; */

		/* check if sta support s Short GI 20M */
		if ((phtpriv_ap->ht_cap.cap_info) & cpu_to_le16(IEEE80211_HT_CAP_SGI_20))
			phtpriv_ap->sgi_20m = _TRUE;
		/* check if sta support s Short GI 40M */
		if ((phtpriv_ap->ht_cap.cap_info) & cpu_to_le16(IEEE80211_HT_CAP_SGI_40))
			phtpriv_ap->sgi_40m = _TRUE;

		psta->qos_option = _TRUE;
	} else {
		phtpriv_ap->ampdu_enable = _FALSE;

		phtpriv_ap->sgi_20m = _FALSE;
		phtpriv_ap->sgi_40m = _FALSE;
	}

	psta->phl_sta->chandef.bw = pmlmeext->cur_bwmode;
	phtpriv_ap->ch_offset = pmlmeext->cur_ch_offset;

	phtpriv_ap->agg_enable_bitmap = 0x0;/* reset */
	phtpriv_ap->candidate_tid_bitmap = 0x0;/* reset */

	_rtw_memcpy(&psta->htpriv, &pmlmepriv->htpriv, sizeof(struct ht_priv));

#ifdef CONFIG_80211AC_VHT
	_rtw_memcpy(&psta->vhtpriv, &pmlmepriv->vhtpriv, sizeof(struct vht_priv));

#ifdef CONFIG_80211AX_HE
	_rtw_memcpy(&psta->hepriv, &pmlmepriv->hepriv, sizeof(struct he_priv));
#endif /* CONFIG_80211AX_HE */

#endif /* CONFIG_80211AC_VHT */

#endif /* CONFIG_80211N_HT */

	psta->state |= WIFI_AP_STATE; /* Aries, add,fix bug of flush_cam_entry at STOP AP mode , 0724 */
}

static void rtw_set_hw_wmm_param(_adapter *padapter)
{
	u8	AIFS, ECWMin, ECWMax, aSifsTime;
	u8	acm_mask;
	u16	TXOP;
	u32	acParm, i;
	u32	edca[4], inx[4];
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct xmit_priv		*pxmitpriv = &padapter->xmitpriv;
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	acm_mask = 0;
#ifdef CONFIG_80211N_HT
	if (pregpriv->ht_enable &&
		(WIFI_ROLE_IS_ON_5G(padapter) ||
	    (pmlmeext->cur_wireless_mode >= WLAN_MD_11N)))
		aSifsTime = 16;
	else
#endif /* CONFIG_80211N_HT */
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

		/*TODO:*/
		acm_mask = 0;
		padapter->mlmepriv.acm_mask = acm_mask;

#if 0
		/* BK */
		/* AIFS = AIFSN * slot time + SIFS - r2t phy delay */
#endif
		AIFS = (7 * pmlmeinfo->slotTime) + aSifsTime;
		ECWMin = 4;
		ECWMax = 10;
		TXOP = 0;
		acParm = AIFS | (ECWMin << 8) | (ECWMax << 12) | (TXOP << 16);
		rtw_hw_set_edca(padapter, 1, acParm);
		edca[XMIT_BK_QUEUE] = acParm;
		RTW_INFO("WMM(BK): %x\n", acParm);

		/* BE */
		AIFS = (3 * pmlmeinfo->slotTime) + aSifsTime;
		ECWMin = 4;
		ECWMax = 6;
		TXOP = 0;
		acParm = AIFS | (ECWMin << 8) | (ECWMax << 12) | (TXOP << 16);
		rtw_hw_set_edca(padapter, 0, acParm);
		edca[XMIT_BE_QUEUE] = acParm;
		RTW_INFO("WMM(BE): %x\n", acParm);

		/* VI */
		AIFS = (1 * pmlmeinfo->slotTime) + aSifsTime;
		ECWMin = 3;
		ECWMax = 4;
		TXOP = 94;
		acParm = AIFS | (ECWMin << 8) | (ECWMax << 12) | (TXOP << 16);
		rtw_hw_set_edca(padapter, 2, acParm);
		edca[XMIT_VI_QUEUE] = acParm;
		RTW_INFO("WMM(VI): %x\n", acParm);

		/* VO */
		AIFS = (1 * pmlmeinfo->slotTime) + aSifsTime;
		ECWMin = 2;
		ECWMax = 3;
		TXOP = 47;
		acParm = AIFS | (ECWMin << 8) | (ECWMax << 12) | (TXOP << 16);
		rtw_hw_set_edca(padapter, 3, acParm);
		edca[XMIT_VO_QUEUE] = acParm;
		RTW_INFO("WMM(VO): %x\n", acParm);


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
			for (i = 0 ; i < 4 ; i++) {
				for (j = i + 1 ; j < 4 ; j++) {
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

		for (i = 0 ; i < 4 ; i++) {
			pxmitpriv->wmm_para_seq[i] = inx[i];
			RTW_INFO("wmm_para_seq(%d): %d\n", i, pxmitpriv->wmm_para_seq[i]);
		}

	}

}
#ifdef CONFIG_80211N_HT
static void update_hw_ht_param(_adapter *padapter)
{
	unsigned char		max_AMPDU_len;
	unsigned char		min_MPDU_spacing;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	RTW_INFO("%s\n", __FUNCTION__);


	/* handle A-MPDU parameter field */
	/*
		AMPDU_para [1:0]:Max AMPDU Len => 0:8k , 1:16k, 2:32k, 3:64k
		AMPDU_para [4:2]:Min MPDU Start Spacing
	*/
	max_AMPDU_len = pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para & 0x03;

	min_MPDU_spacing = (pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para & 0x1c) >> 2;

	rtw_hal_set_hwreg(padapter, HW_VAR_AMPDU_MIN_SPACE, (u8 *)(&min_MPDU_spacing));

	rtw_hal_set_hwreg(padapter, HW_VAR_AMPDU_FACTOR, (u8 *)(&max_AMPDU_len));

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
	/* pmlmeinfo->HT_protection = pmlmeinfo->HT_info.infos[1] & 0x3; */

}
#endif /* CONFIG_80211N_HT */
static void rtw_ap_check_scan(_adapter *padapter)
{
	_list		*plist, *phead;
	u32	delta_time, lifetime;
	struct	wlan_network	*pnetwork = NULL;
	WLAN_BSSID_EX *pbss = NULL;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
#ifdef RTW_MI_SHARE_BSS_LIST
	_queue *queue = &padapter->dvobj->scanned_queue;
#else
	_queue	*queue	= &(pmlmepriv->scanned_queue);
#endif
	u8 do_scan = _FALSE;
	u8 reason = RTW_AUTO_SCAN_REASON_UNSPECIFIED;

	lifetime = SCANQUEUE_LIFETIME; /* 20 sec */

	_rtw_spinlock_bh(&(queue->lock));
	phead = get_list_head(queue);
	if (rtw_end_of_queue_search(phead, get_next(phead)) == _TRUE)
		if (padapter->registrypriv.wifi_spec) {
			do_scan = _TRUE;
			reason |= RTW_AUTO_SCAN_REASON_2040_BSS;
		}
	_rtw_spinunlock_bh(&(queue->lock));

#ifdef CONFIG_RTW_ACS
	if (padapter->registrypriv.acs_auto_scan) {
		do_scan = _TRUE;
		reason |= RTW_AUTO_SCAN_REASON_ACS;
		rtw_acs_start(padapter);
	}
#endif/*CONFIG_RTW_ACS*/

	if (_TRUE == do_scan) {
		RTW_INFO("%s : drv scans by itself and wait_completed\n", __func__);
		rtw_drv_scan_by_self(padapter, reason);
		rtw_scan_wait_completed(padapter);
	}

#ifdef CONFIG_RTW_ACS
	if (padapter->registrypriv.acs_auto_scan)
		rtw_acs_stop(padapter);
#endif

	_rtw_spinlock_bh(&(queue->lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1) {

		if (rtw_end_of_queue_search(phead, plist) == _TRUE)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		if (rtw_chset_search_ch(adapter_to_chset(padapter), pnetwork->network.Configuration.DSConfig) >= 0
		    && rtw_mlme_band_check(padapter, pnetwork->network.Configuration.DSConfig) == _TRUE
		    && _TRUE == rtw_validate_ssid(&(pnetwork->network.Ssid))) {
			delta_time = (u32) rtw_get_passing_time_ms(pnetwork->last_scanned);

			if (delta_time < lifetime) {

				uint ie_len = 0;
				u8 *pbuf = NULL;
				u8 *ie = NULL;

				pbss = &pnetwork->network;
				ie = pbss->IEs;

				/*check if HT CAP INFO IE exists or not*/
				pbuf = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _HT_CAPABILITY_IE_, &ie_len, (pbss->IELength - _BEACON_IE_OFFSET_));
				if (pbuf == NULL) {
					/* HT CAP INFO IE don't exist, it is b/g mode bss.*/

					if (_FALSE == ATOMIC_READ(&pmlmepriv->olbc_ap))
						ATOMIC_SET(&pmlmepriv->olbc_ap, _TRUE);

					if (padapter->registrypriv.wifi_spec)
						RTW_INFO("%s: %s is a/b/g ap\n", __func__, pnetwork->network.Ssid.Ssid);
				}
			}
		}

		plist = get_next(plist);

	}

	_rtw_spinunlock_bh(&(queue->lock));
#ifdef CONFIG_80211N_HT
	pmlmepriv->num_sta_no_ht = 0; /* reset to 0 after ap do scanning*/
#endif
}

void rtw_start_bss_hdl_after_chbw_decided(_adapter *adapter)
{
	WLAN_BSSID_EX *pnetwork = &(adapter->mlmepriv.cur_network.network);
	struct sta_info *sta = NULL;

	/* update cur_wireless_mode */
	update_wireless_mode(adapter);

	/* update RRSR and RTS_INIT_RATE register after set channel and bandwidth */
	UpdateBrateTbl(adapter, pnetwork->SupportedRates);
	rtw_hal_set_hwreg(adapter, HW_VAR_BASIC_RATE, pnetwork->SupportedRates);

	/* update capability after cur_wireless_mode updated */
	update_capinfo(adapter, rtw_get_capability(pnetwork));

	/* update bc/mc sta_info */
	update_bmc_sta(adapter);

	/* update AP's sta info */
	sta = rtw_get_stainfo(&adapter->stapriv, adapter_mac_addr(adapter));
	if (!sta) {
		const u8 *mac_addr = adapter_mac_addr(adapter);
		RTW_ERR(FUNC_ADPT_FMT" !sta for macaddr="MAC_FMT"\n",
		        FUNC_ADPT_ARG(adapter), MAC_ARG(mac_addr));
		rtw_warn_on(1);
		return;
	}

	update_ap_info(adapter, sta);
}

static void _rtw_iface_undersurvey_chk(const char *func, _adapter *adapter)
{
	int i;
	_adapter *iface;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct mlme_priv *pmlmepriv;

	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if ((iface) && rtw_is_adapter_up(iface)) {
			pmlmepriv = &iface->mlmepriv;
			if (check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY))
				RTW_ERR("%s ("ADPT_FMT") under survey\n", func, ADPT_ARG(iface));
		}
	}
}

void wifi_alliance_logo_check(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork_mlmeext = &(pmlmeinfo->network);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

#ifdef WIFI_LOGO_HT_4_2_47
	if (pnetwork_mlmeext->Ssid.SsidLength == 2
		&& _rtw_memcmp(pnetwork_mlmeext->Ssid.Ssid, "ps", 2))
		pmlmeinfo->is_HT_4_2_47 = 1;
	else
		pmlmeinfo->is_HT_4_2_47 = 0;
#endif
#ifdef WIFI_LOGO_HE_4_7_1
	if (pmlmepriv->hepriv.he_option == _TRUE
		&& pnetwork_mlmeext->Ssid.SsidLength >= 8
		&& _rtw_memcmp(pnetwork_mlmeext->Ssid.Ssid, "HE-4.7.1", 8))
		pmlmeinfo->is_HE_4_7_1 = 1;
	else
		pmlmeinfo->is_HE_4_7_1 = 0;
#endif
#ifdef WIFI_LOGO_HE_4_20_1
	if (pmlmepriv->hepriv.he_option == _TRUE
		&& pnetwork_mlmeext->Ssid.SsidLength == 13
		&& _rtw_memcmp(pnetwork_mlmeext->Ssid.Ssid, "HE-4.20.1_24G", 13))
		pmlmeinfo->is_HE_4_20_1_24G = 1;
	else
		pmlmeinfo->is_HE_4_20_1_24G = 0;
#endif
#ifdef WIFI_LOGO_HE_4_52_1
	if (pmlmepriv->hepriv.he_option == _TRUE
		&& pnetwork_mlmeext->Ssid.SsidLength >= 9
		&& _rtw_memcmp(pnetwork_mlmeext->Ssid.Ssid, "HE-4.52.1", 9)) {
		pmlmeinfo->is_HE_4_52_1 = 1;
		pxmitpriv->txsc_enable = 0;
	}
	else {
		pmlmeinfo->is_HE_4_52_1 = 0;
	}
#endif
#ifdef WIFI_LOGO_HE_4_56_1
	if (pmlmepriv->hepriv.he_option == _TRUE
		&& pnetwork_mlmeext->Ssid.SsidLength >= 9
		&& _rtw_memcmp(pnetwork_mlmeext->Ssid.Ssid, "HE-4.56.1", 9)) {
		pmlmeinfo->is_HE_4_56_1 = 1;
		padapter->registrypriv.twt_enable = 1;
		padapter->registrypriv.vcs_type = 0;
		padapter->no_rts = 1;
		rtw_phl_write32(padapter->dvobj->phl, 0x9e28, 0xff0000);
		rtw_phl_write8(padapter->dvobj->phl, 0xc600, 0x1c);
		rtw_phl_write8(padapter->dvobj->phl, 0x9e13, 0xb0);
		rtw_phl_write32(padapter->dvobj->phl, 0x9e48, 0x800200ff);

		rtw_phl_write32(padapter->dvobj->phl, 0xCC24,
			rtw_phl_read32(padapter->dvobj->phl, 0xCC24)&(~(BIT14|BIT15|BIT12)));
		/* 0xCC08[11:10]=2, duplicate ACL/BA */
		rtw_phl_write32(padapter->dvobj->phl, 0xCC08,
			rtw_phl_read32(padapter->dvobj->phl, 0xCC08)&(~(BIT11|BIT10)));
	 } else {
		pmlmeinfo->is_HE_4_56_1 = 0;
	 }
#endif
#ifdef WIFI_LOGO_MBO_4_2_5_3
	if (pmlmepriv->mbopriv.enable == _TRUE
		&& pnetwork_mlmeext->Ssid.SsidLength == 13
		&& _rtw_memcmp(pnetwork_mlmeext->Ssid.Ssid, "WiFi1-4.2.5.3", 13))
		pmlmeinfo->is_MBO_4_2_5_3 = 1;
	else
		pmlmeinfo->is_MBO_4_2_5_3 = 0;
#endif
#ifdef WIFI_LOGO_MBO_4_2_5_4
	if (pmlmepriv->mbopriv.enable == _TRUE
		&& pnetwork_mlmeext->Ssid.SsidLength == 13
		&& _rtw_memcmp(pnetwork_mlmeext->Ssid.Ssid, "WiFi1-4.2.5.4", 13))
		pmlmeinfo->is_MBO_4_2_5_4 = 1;
	else
		pmlmeinfo->is_MBO_4_2_5_4 = 0;
#endif
#ifdef WIFI_LOGO_11N_WMM
	if ((pmlmeinfo->network.Ssid.SsidLength == 8)
			&& _rtw_memcmp(pmlmeinfo->network.Ssid.Ssid, "NVCX@7.N", 8) == _TRUE) {
		padapter->dvobj->wmm_test = 22;
		rtw_hw_set_edca(padapter, 0, 0xf642b);
	} else if ((pmlmeinfo->network.Ssid.SsidLength == 6)
			&& _rtw_memcmp(pmlmeinfo->network.Ssid.Ssid, "4.2.23", 6) == _TRUE) {
		padapter->dvobj->wmm_test = 23;
		padapter->registrypriv.vcs_type = 0;
		rtw_phl_write16(padapter->dvobj->phl, 0xc626, 0);
	} else if ((pmlmeinfo->network.Ssid.SsidLength == 6)
			&& _rtw_memcmp(pmlmeinfo->network.Ssid.Ssid, "4.2.21", 6) == _TRUE) {
		padapter->dvobj->wmm_test = 21;
	} else {
		extern int rtw_vcs_type;
		padapter->dvobj->wmm_test = 0;
		padapter->registrypriv.vcs_type = (u8)rtw_vcs_type;
	}

#ifdef CONFIG_LMT_TXREQ
	if (padapter->dvobj->wmm_test > 0)
		padapter->lmt_txreq_enable = 1;
#endif
#endif
#ifdef WIFI_LOGO_VHT_4_2_44
	if (pmlmepriv->vhtpriv.vht_option == _TRUE
		&& pnetwork_mlmeext->Ssid.SsidLength == 10
		&& _rtw_memcmp(pnetwork_mlmeext->Ssid.Ssid, "VHT-4.2.44", 10)) {
		pmlmeinfo->is_VHT_4_2_44 = 1;
	} else {
		pmlmeinfo->is_VHT_4_2_44 = 0;
	}
#endif

	return;
}

void rtw_ap_upt_all_chanctx(_adapter *padapter,
	struct createbss_parm *parm, struct rtw_chan_def *new_chdef)
{
	_adapter *iface = NULL;
	struct dvobj_priv *pdvobj = padapter->dvobj;
	struct rtw_mr_chctx_info mr_cc_info = {0};
	bool is_chctx_add = false;
	int i = 0, chanctx_num = 0;

	RTW_INFO(FUNC_ADPT_FMT"new chdef: CH:%d, BW:%d OFF:%d\n",
			FUNC_ADPT_ARG(padapter), new_chdef->chan, new_chdef->bw, new_chdef->offset);

	/* delete chanctx */
	for (i = 0; i < pdvobj->iface_nums; i++) {
		iface = pdvobj->padapters[i];
		if (!(parm->ifbmp & BIT(i)) || !iface)
			continue;

		chanctx_num = rtw_phl_chanctx_del(pdvobj->phl, iface->phl_role, NULL);

		RTW_INFO(FUNC_ADPT_FMT"chctx_del: num:%d\n",
			FUNC_ADPT_ARG(padapter), chanctx_num);
	}

	/* add chanctx */
	for (i = 0; i < pdvobj->iface_nums; i++) {
		iface = pdvobj->padapters[i];
		if (!(parm->ifbmp & BIT(i)) || !iface)
			continue;

		is_chctx_add = rtw_phl_chanctx_add(pdvobj->phl, iface->phl_role,
					new_chdef, &mr_cc_info);

		RTW_INFO(FUNC_ADPT_FMT"chctx_add:%s\n",
			FUNC_ADPT_ARG(padapter), (is_chctx_add) ? "Y" : "N");
		RTW_INFO(FUNC_ADPT_FMT"PHL- CH:%d, BW:%d OFF:%d\n",
			FUNC_ADPT_ARG(padapter), new_chdef->chan, new_chdef->bw, new_chdef->offset);
	}
}

void start_bss_network(_adapter *padapter, struct createbss_parm *parm)
{
#define DUMP_ADAPTERS_STATUS 0
	u8 mlme_act = MLME_ACTION_UNKNOWN;
	u16 bcn_interval;
	u32	acparm;
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct security_priv *psecuritypriv = &(padapter->securitypriv);
	WLAN_BSSID_EX *pnetwork = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network; /* used as input */
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork_mlmeext = &(pmlmeinfo->network);
	struct dvobj_priv *pdvobj = padapter->dvobj;
	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	void *phl = padapter->dvobj->phl;		// Mark.CS_update
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobj);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	#endif
	s16 req_ch = REQ_CH_NONE, req_bw = REQ_BW_NONE, req_offset = REQ_OFFSET_NONE;
	u8 ch_to_set = 0, bw_to_set, offset_to_set;
	u8 do_rfk = _TRUE;
	/* use for check ch bw offset can be allowed or not */
	u8 chbw_allow = _TRUE;
	int i;
	u16 ifbmp_changed = 0;
	_adapter *tmp_adapter;
	struct rtw_chan_def chdef_to_set = {0};
	struct rtw_chan_def new_chdef = {0};
	struct rtw_mr_chctx_info mr_cc_info = {0};
	bool is_chctx_add = false;
	RT_CHANNEL_INFO *chset = adapter_to_chset(padapter);
	struct rtw_chan_def chandef = {0};

	#if defined(CONFIG_RTW_DEBUG_MBSSID_VAP) && (CONFIG_RTW_DEBUG_MBSSID_VAP > 0)
	RTW_PRINT(FUNC_ADPT_FMT" C(%d) B(%d) O(%d)\n", FUNC_ADPT_ARG(padapter),
	          parm->req_ch, parm->req_bw, parm->req_offset);
	#endif /* CONFIG_RTW_DEBUG_MBSSID_VAP */

	if (parm->req_ch != 0) {
		/* bypass other setting, go checking ch, bw, offset */
		mlme_act = MLME_OPCH_SWITCH;
		req_ch = parm->req_ch;
		req_bw = parm->req_bw;
		req_offset = parm->req_offset;
		goto chbw_decision;
	} else {
		/* request comes from upper layer */
		if (MLME_IS_AP(padapter))
			mlme_act = MLME_AP_STARTED;
		else if (MLME_IS_MESH(padapter))
			mlme_act = MLME_MESH_STARTED;
		else
			rtw_warn_on(1);
		req_ch = 0;
		_rtw_memcpy(pnetwork_mlmeext, pnetwork, pnetwork->Length);
	}

	bcn_interval = (u16)pnetwork->Configuration.BeaconPeriod;
	pmlmeinfo->bcn_interval = bcn_interval;

	/* hostapd only update beacon one time when starting hostapd. */
	pmlmeext->bstart_bss = _TRUE;

	/* todo: update wmm, ht cap */
	/* pmlmeinfo->WMM_enable; */
	/* pmlmeinfo->HT_enable; */
	if (pmlmepriv->qospriv.qos_option)
		pmlmeinfo->WMM_enable = _TRUE;
#ifdef CONFIG_80211N_HT
	if (pmlmepriv->htpriv.ht_option) {
		pmlmeinfo->WMM_enable = _TRUE;
		pmlmeinfo->HT_enable = _TRUE;
		/* pmlmeinfo->HT_info_enable = _TRUE; */
		/* pmlmeinfo->HT_caps_enable = _TRUE; */

		update_hw_ht_param(padapter);
	} else
		pmlmeinfo->HT_enable = _FALSE;
#endif /* #CONFIG_80211N_HT */

#ifdef CONFIG_80211AC_VHT
	if (pmlmepriv->vhtpriv.vht_option) {
		pmlmeinfo->VHT_enable = _TRUE;
		update_hw_vht_param(padapter);
	} else
		pmlmeinfo->VHT_enable = _FALSE;
#endif /* CONFIG_80211AC_VHT */

#ifdef CONFIG_80211AX_HE
	if (pmlmepriv->hepriv.he_option) {
		pmlmeinfo->HE_enable = _TRUE;
		update_hw_he_param(padapter);
	} else
		pmlmeinfo->HE_enable = _FALSE;
#endif /* CONFIG_80211AX_HE */

	wifi_alliance_logo_check(padapter);

	if ((pmlmepriv->cur_network.join_res != _TRUE) /* setting only at  first time */
			&& (pmlmepriv->cur_network.join_res != -2)) { /* setkey invoked before start_bss_network */
		/* WEP Key will be set before this function, do not clear CAM. */
		if ((psecuritypriv->dot11PrivacyAlgrthm != _WEP40_) && (psecuritypriv->dot11PrivacyAlgrthm != _WEP104_)
			&& !MLME_IS_MESH(padapter) /* mesh group key is set before this function */
		)
			flush_all_cam_entry(padapter);	/* clear CAM */
	}

#if 0
	/* set network type to AP_Mode		 */
	rtw_hal_set_network_type(padapter, _HW_STATE_AP_);

	/* Set BSSID REG */
	rtw_hal_set_hwreg(padapter, HW_VAR_BSSID, pnetwork->MacAddress);

	/* Set Security */
	rtw_hal_set_hwreg(padapter, HW_VAR_SEC_CFG, NULL);

	/* Beacon Control related register */
	rtw_hal_set_hwreg(padapter, HW_VAR_BEACON_INTERVAL, (u8 *)(&bcn_interval));

	rtw_hal_rcr_set_chk_bssid(padapter, mlme_act);
#else
	rtw_hw_start_bss_network(padapter);
#endif

chbw_decision:
	#if 0//def CONFIG_DBCC_SUPPORT
	new_chdef.chan = pmlmepriv->ori_chandef.chan;
	new_chdef.bw = pmlmepriv->ori_chandef.bw;
	new_chdef.offset = pmlmepriv->ori_chandef.offset;
	new_chdef.band = rtw_phl_get_band_type(new_chdef.chan);

	is_chctx_add = rtw_phl_chanctx_chk(pdvobj->phl, padapter->phl_role,
						&new_chdef, &mr_cc_info);

	RTW_INFO("%s => chctx_chk:%s\n", __func__, (is_chctx_add) ? "Y" : "N");
	RTW_INFO("PHL- CH:%d, BW:%d OFF:%d\n", new_chdef.chan, new_chdef.bw, new_chdef.offset);
	if (is_chctx_add == false && mr_cc_info.sugg_opmode == MR_OP_DBCC) {
		_rtw_memcpy(&pmlmeext->chandef, &pmlmepriv->ori_chandef, sizeof(struct rtw_chan_def));
		_rtw_memcpy(&chdef_to_set, &new_chdef, sizeof(struct rtw_chan_def));
		do_rfk = _TRUE;
		rtw_phl_mr_trig_dbcc_enable(dvobj->phl);
		goto after_chbw_decision;
	}
	#endif
	ifbmp_changed = rtw_ap_chbw_decision(padapter, parm->ifbmp, parm->excl_ifbmp
						, req_ch, req_bw, req_offset
						, &ch_to_set, &bw_to_set, &offset_to_set, &chbw_allow);

	for (i = 0; i < pdvobj->iface_nums; i++) {
		if (!(parm->ifbmp & BIT(i)) || !pdvobj->padapters[i])
			continue;

		/* let pnetwork_mlme == pnetwork_mlmeext */
		_rtw_memcpy(&(pdvobj->padapters[i]->mlmepriv.cur_network.network)
			, &(pdvobj->padapters[i]->mlmeextpriv.mlmext_info.network)
			, pdvobj->padapters[i]->mlmeextpriv.mlmext_info.network.Length);

		rtw_start_bss_hdl_after_chbw_decided(pdvobj->padapters[i]);

		/* Set EDCA param reg after update cur_wireless_mode & update_capinfo */
		if (pregpriv->wifi_spec == 1)
			rtw_set_hw_wmm_param(pdvobj->padapters[i]);
	}

#if defined(CONFIG_DFS_MASTER)
	rtw_dfs_rd_en_decision(padapter, mlme_act, parm->excl_ifbmp);
#endif

	new_chdef.chan = pmlmeext->cur_channel;
	new_chdef.bw = pmlmeext->cur_bwmode;
	new_chdef.offset = pmlmeext->cur_ch_offset;
	new_chdef.band = rtw_phl_get_band_type(new_chdef.chan);

	if (parm->is_change_chbw) {
		rtw_ap_upt_all_chanctx(padapter, parm, &new_chdef);
	} else {
		is_chctx_add = rtw_phl_chanctx_add(pdvobj->phl, padapter->phl_role,
						&new_chdef, &mr_cc_info);

		RTW_INFO("%s => chctx_add:%s\n", __func__, (is_chctx_add) ? "Y" : "N");
		RTW_INFO("PHL- CH:%d, BW:%d OFF:%d\n", new_chdef.chan, new_chdef.bw, new_chdef.offset);
	}

after_chbw_decision:

#ifdef CONFIG_MCC_MODE
	if (MCC_EN(padapter)) {
		/*
		* due to check under rtw_ap_chbw_decision
		* if under MCC mode, means req channel setting is the same as current channel setting
		* if not under MCC mode, mean req channel setting is not the same as current channel setting
		*/
		if (rtw_hal_check_mcc_status(padapter, MCC_STATUS_DOING_MCC)) {
				RTW_INFO(FUNC_ADPT_FMT": req channel setting is the same as current channel setting, go to update BCN\n"
				, FUNC_ADPT_ARG(padapter));

				goto update_beacon;

		}
	}

	/* issue null data to AP for all interface connecting to AP before switch channel setting for softap */
	rtw_hal_mcc_issue_null_data(padapter, chbw_allow, 1);
#endif /* CONFIG_MCC_MODE */

	if (ch_to_set != 0) {
#if 0
		if (!rtw_chset_is_dfs_chbw(chset, ch_to_set, bw_to_set, offset_to_set))	{
			rtw_phl_get_cur_hal_chdef(padapter->phl_role, &chandef);
			if ((ch_to_set == chandef.chan) && (bw_to_set == chandef.bw) && (offset_to_set == chandef.offset))
				do_rfk = _FALSE;
		}
#endif
		rtw_hw_update_chan_def(padapter);
		set_channel_bwmode(padapter, ch_to_set, offset_to_set, bw_to_set, do_rfk);
		rtw_mi_update_union_chan_inf(padapter, ch_to_set, offset_to_set, bw_to_set);

		if (MLME_IS_AP(padapter)) {
		 	struct sta_info *psta_bmc;
			psta_bmc = rtw_get_bcmc_stainfo(padapter);
			if (psta_bmc) {
				if (WIFI_ROLE_IS_ON_24G(padapter))
					psta_bmc->cur_tx_data_rate = RTW_DATA_RATE_CCK1;
				else
					psta_bmc->cur_tx_data_rate = RTW_DATA_RATE_OFDM6;
			}
		}
	}

#ifdef CONFIG_WLAN_MANAGER
	rtw_netlink_hook(padapter, padapter->phl_role->chandef.band, padapter->iface_id);
#endif

#ifdef CONFIG_MCC_MODE
	/* after set_channel_bwmode for backup IQK */
	rtw_hal_set_mcc_setting_start_bss_network(padapter, chbw_allow);
#endif

#if defined(CONFIG_IOCTL_CFG80211) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0))
	for (i = 0; i < pdvobj->iface_nums; i++) {
		if ((!(ifbmp_changed & BIT(i)) || !pdvobj->padapters[i]) &&
			!(pdvobj->padapters[i]->registrypriv.wifi_mib.acs == 1 &&
				pdvobj->acspriv.best_channel == 0))
			continue;

		{
			u8 ht_option = 0;

			#ifdef CONFIG_80211N_HT
			ht_option = pdvobj->padapters[i]->mlmepriv.htpriv.ht_option;
			#endif

			rtw_cfg80211_ch_switch_notify(pdvobj->padapters[i]
				, pdvobj->padapters[i]->mlmeextpriv.cur_channel
				, pdvobj->padapters[i]->mlmeextpriv.cur_bwmode
				, pdvobj->padapters[i]->mlmeextpriv.cur_ch_offset
				, ht_option, 0);
		}
	}
#endif /* defined(CONFIG_IOCTL_CFG80211) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)) */

	if(check_fwstate(&padapter->mlmepriv, WIFI_AP_STATE) == _TRUE && is_primary_adapter(padapter))
	{
		for (i = 0; i < pdvobj->iface_nums; i++) {
			if(!pdvobj->padapters[i])
				continue;
			tmp_adapter = pdvobj->padapters[i];
			if(rtw_is_adapter_up(tmp_adapter))
			{
				if(check_fwstate(&tmp_adapter->mlmepriv, WIFI_STATION_STATE) == _TRUE)
				{
					tmp_adapter->mlmeextpriv.cur_channel = padapter->mlmeextpriv.cur_channel;
					tmp_adapter->phl_role->chandef.chan = tmp_adapter->mlmeextpriv.cur_channel;
				}
			}
		}

#if defined(CONFIG_WFA_OFDMA_Logo_Test)
		rtw_ru_set_ch_bw(padapter);
		rtw_ru_set_pwrtbl(padapter);

		#if 0
		switch (pmlmeext->cur_bwmode) {
			case CHANNEL_WIDTH_160:
				rtw_ofdma_grp_bw160_setting(padapter);
				break;
			case CHANNEL_WIDTH_80:
				rtw_ofdma_grp_bw80_setting(padapter);
				break;
			case CHANNEL_WIDTH_40:
				rtw_ofdma_grp_bw40_setting(padapter);
				break;
			default:
				rtw_ofdma_grp_bw20_setting(padapter);
				break;
		}
		#else  // Mark.CS_update
		rtw_phl_grp_bw_setting_apply(phl, pmlmeext->cur_bwmode);
		#endif

		_set_timer(&padapter->ofdma_timer, 1000);
#endif
	}

	if (DUMP_ADAPTERS_STATUS) {
		RTW_INFO(FUNC_ADPT_FMT" done\n", FUNC_ADPT_ARG(padapter));
		dump_adapters_status(RTW_DBGDUMP , adapter_to_dvobj(padapter));
	}

#ifdef CONFIG_MCC_MODE
update_beacon:
#endif

	for (i = 0; i < pdvobj->iface_nums; i++) {
		struct mlme_priv *mlme;

		if (!(parm->ifbmp & BIT(i)) || !pdvobj->padapters[i])
			continue;

		/* update beacon content only if bstart_bss is _TRUE */
		if (pdvobj->padapters[i]->mlmeextpriv.bstart_bss != _TRUE)
			continue;

		mlme = &(pdvobj->padapters[i]->mlmepriv);

		#ifdef CONFIG_80211N_HT
		if ((ATOMIC_READ(&mlme->olbc_sta) == _TRUE) || (ATOMIC_READ(&mlme->olbc_ap) == _TRUE)) {
			/* AP is not starting a 40 MHz BSS in presence of an 802.11g BSS. */
			mlme->ht_op_mode &= (~HT_INFO_OPERATION_MODE_OP_MODE_MASK);
			mlme->ht_op_mode |= OP_MODE_MAY_BE_LEGACY_STAS;
			update_beacon(pdvobj->padapters[i], _HT_ADD_INFO_IE_, NULL, _FALSE, 0);
		}
		#endif

		update_beacon(pdvobj->padapters[i], _TIM_IE_, NULL, _FALSE, 0);
	}

	if (mlme_act != MLME_OPCH_SWITCH && pmlmeext->bstart_bss == _TRUE)
		rtw_mi_ap_correct_tsf(padapter, mlme_act);

	rtw_scan_wait_completed(padapter);

	_rtw_iface_undersurvey_chk(__func__, padapter);
	/* send beacon */
	rtw_hal_set_hwreg(padapter, HW_VAR_RESUME_BCN, NULL);
	{
#if !defined(CONFIG_INTERRUPT_BASED_TXBCN)
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI) || defined(CONFIG_PCI_BCN_POLLING) || defined(CONFIG_HWSIM)
		u8 timing_update = 0;
		ifbmp_changed = parm->ifbmp;
		for (i = 0; i < pdvobj->iface_nums; i++) {
			if ((!(ifbmp_changed & BIT(i)) || !pdvobj->padapters[i]) && !timing_update)
				continue;

			if (!pdvobj->padapters[i]->netif_up)
				continue;

			/*When init primary iface will reset Port 0, and MBSSID will be reset as well
			  Once we reset port 0, take care of VAPs is needed*/
			if (is_primary_adapter(pdvobj->padapters[i])) { timing_update = 1; }

			pdvobj->padapters[i]->registrypriv.wifi_mib.func_off_prev =
				pdvobj->padapters[i]->registrypriv.wifi_mib.func_off;

			if (pdvobj->padapters[i]->registrypriv.wifi_mib.func_off)
				continue;

			if (send_beacon(pdvobj->padapters[i]) == _FAIL)
				RTW_INFO(ADPT_FMT" issue_beacon, fail!\n", ADPT_ARG(pdvobj->padapters[i]));
		}
#endif
#endif /* !defined(CONFIG_INTERRUPT_BASED_TXBCN) */

	}
#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
	if (MLME_IS_AP(padapter) && padapter->tbtx_capability == _TRUE) {
		_set_timer(&pmlmeext->tbtx_token_dispatch_timer, 1);
		RTW_INFO("Start token dispatch\n");
	}
#endif
#ifdef CONFIG_RTW_80211K
	/* construct self neighbor report */
	if(padapter->rmpriv.enable == _TRUE)
		rtw_rm_construct_self_nb_report(padapter);
#endif
}

int rtw_check_beacon_data(_adapter *padapter, u8 *pbuf,  int len)
{
	int ret = _SUCCESS;
	u8 *p;
	u8 *pHT_caps_ie = NULL;
	u8 *pHT_info_ie = NULL;
	u16 cap, ht_cap = _FALSE;
	uint ie_len = 0;
	int group_cipher, pairwise_cipher, gmcs;
	u32 akm;
	u8 mfp_opt = MFP_NO;
	u8	channel, network_type;
	u8 OUI1[] = {0x00, 0x50, 0xf2, 0x01};
	u8 WMM_PARA_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};
	u8 WIFI_ALLIANCE_OUI[] = {0x50, 0x6f, 0x9a};
	HT_CAP_AMPDU_DENSITY best_ampdu_density = 0;
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX *pbss_network = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;
	u8 *ie = pbss_network->IEs;
	u8 vht_cap = _FALSE;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	u8 rf_num = 0;
	int ret_rm;
	u8 bw_mode = BW_CAP_20M;
#ifdef RTW_WKARD_BCNINT_DBG
	u16 bcnint;
#endif
	int tmp_idx;
	/* SSID */
	/* Supported rates */
	/* DS Params */
	/* WLAN_EID_COUNTRY */
	/* ERP Information element */
	/* Extended supported rates */
	/* WPA/WPA2 */
	/* Wi-Fi Wireless Multimedia Extensions */
	/* ht_capab, ht_oper */
	/* WPS IE */

	RTW_INFO("%s, len=%d\n", __FUNCTION__, len);

	if (!MLME_IS_AP(padapter) && !MLME_IS_MESH(padapter))
		return _FAIL;


	if (len > MAX_IE_SZ)
		return _FAIL;

	pbss_network->IELength = len;

	_rtw_memset(ie, 0, MAX_IE_SZ);

	_rtw_memcpy(ie, pbuf, pbss_network->IELength);

	#ifdef CONFIG_RTW_SUPPORT_MBSSID_VAP
	#if 0 && defined(CONFIG_RTW_DEBUG_MBSSID_VAP) && (CONFIG_RTW_DEBUG_MBSSID_VAP >= 2)
	RTW_PRINT_DUMP("cur_network", (u8 *)pbss_network, sizeof(*pbss_network));
	RTW_PRINT_DUMP("mlmext_info", (u8 *)&pmlmeinfo->network, sizeof(pmlmeinfo->network));

	RTW_PRINT_DUMP("Beacon Data:", ie, pbss_network->IELength);
	#endif /* CONFIG_RTW_DEBUG_MBSSID_VAP */

	pmlmeinfo->network.IELength = pbss_network->IELength;
	_rtw_memcpy(pmlmeinfo->network.IEs, ie, MAX_IE_SZ);
	#endif /* CONFIG_RTW_SUPPORT_MBSSID_VAP */

	if (pbss_network->InfrastructureMode != Ndis802_11APMode
		&& pbss_network->InfrastructureMode != Ndis802_11_mesh
	) {
		rtw_warn_on(1);
		return _FAIL;
	}

	/* reset registry */
	pregistrypriv->short_gi = 0;
	pregistrypriv->bw_mode = 0;

	rtw_ap_check_scan(padapter);

	pbss_network->Rssi = 0;

	_rtw_memcpy(pbss_network->MacAddress, adapter_mac_addr(padapter), ETH_ALEN);

	#if defined(CONFIG_RTW_DEBUG_MBSSID_VAP) && (CONFIG_RTW_DEBUG_MBSSID_VAP >= 2)
	RTW_PRINT("Beacon BSS MAC: %pM\n", pbss_network->MacAddress);
	// rtw_dump_ie(ie + _BEACON_IE_OFFSET_, pbss_network->IELength - _BEACON_IE_OFFSET_);
	#endif /* CONFIG_RTW_DEBUG_MBSSID_VAP */

	/* beacon interval */
	p = rtw_get_beacon_interval_from_ie(ie);/* ie + 8;	 */ /* 8: TimeStamp, 2: Beacon Interval 2:Capability */
	/* pbss_network->Configuration.BeaconPeriod = le16_to_cpu(*(unsigned short*)p); */

	pbss_network->Configuration.BeaconPeriod = RTW_GET_LE16(p);
	#ifdef RTW_WKARD_BCNINT_DBG
	/* replace bcnint with mib setting */
	if (GET_PRIMARY_ADAPTER(padapter)->registrypriv.wifi_mib.bcnint) {
		bcnint = cpu_to_le16((u16)GET_PRIMARY_ADAPTER(padapter)->registrypriv.wifi_mib.bcnint);
		_rtw_memcpy(p, (u8 *)&bcnint, 2);
	}
	#endif

	/* capability */
	p = rtw_get_capability_from_ie(ie);
	cap = RTW_GET_LE16(p);
	pmlmepriv->capab_ie_data = cap;

	/* SSID */
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _SSID_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if (p && ie_len > 0) {
		_rtw_memset(&pbss_network->Ssid, 0, sizeof(NDIS_802_11_SSID));
		_rtw_memcpy(pbss_network->Ssid.Ssid, (p + 2), ie_len);
		pbss_network->Ssid.SsidLength = ie_len;
#ifdef CONFIG_P2P
		_rtw_memcpy(padapter->wdinfo.p2p_group_ssid, pbss_network->Ssid.Ssid, pbss_network->Ssid.SsidLength);
		padapter->wdinfo.p2p_group_ssid_len = pbss_network->Ssid.SsidLength;
#endif
	}

#ifdef CONFIG_RTW_MESH
	/* Mesh ID */
	if (MLME_IS_MESH(padapter)) {
		p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, WLAN_EID_MESH_ID, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
		if (p && ie_len > 0) {
			_rtw_memset(&pbss_network->mesh_id, 0, sizeof(NDIS_802_11_SSID));
			_rtw_memcpy(pbss_network->mesh_id.Ssid, (p + 2), ie_len);
			pbss_network->mesh_id.SsidLength = ie_len;
		}
	}
#endif

	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _EXT_CAP_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if (p && ie_len > 0) {
		RTW_INFO("[%s]Get _EXT_CAP_IE_, len = %d\n", __func__, ie_len);
		pmlmepriv->ext_capab_ie_len = MIN(ie_len, sizeof(pmlmepriv->ext_capab_ie_data));
		if (pmlmepriv->ext_capab_ie_len != ie_len)
			RTW_ERR(FUNC_ADPT_FMT" size of ext_capab_ie_data is NOT enough.(%d)\n", FUNC_ADPT_ARG(padapter), ie_len);
		_rtw_memcpy(pmlmepriv->ext_capab_ie_data, p+2, pmlmepriv->ext_capab_ie_len);
	}

	/* chnnel */
	channel = 0;
	pbss_network->Configuration.Length = 0;
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _DSSET_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if (p && ie_len > 0)
		channel = *(p + 2);

	pbss_network->Configuration.DSConfig = channel;

	/*check if H2E existed*/
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if(p && ie_len > 0){
		for(tmp_idx = 0; tmp_idx < ie_len; tmp_idx++)
		{
			if(p[tmp_idx+2] == _EXT_SUPPORTEDRATES_H2E_)
			{
				pbss_network->wpa3_h2e_only=1;
			}
		}
	}

	/*	support rate ie & ext support ie & IElen & SupportedRates	*/
	network_type = rtw_update_rate_bymode(pbss_network, pregistrypriv->wireless_mode);

	/* parsing ERP_IE */
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _ERPINFO_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if (p && ie_len > 0)  {
		if(padapter->registrypriv.wireless_mode == WLAN_MD_11B) {

			pbss_network->IELength = pbss_network->IELength - *(p+1) - 2;
			ret_rm = rtw_ies_remove_ie(ie , &len, _BEACON_IE_OFFSET_, _ERPINFO_IE_,NULL,0);
			RTW_DBG("%s, remove_ie of ERP_IE=%d\n", __FUNCTION__, ret_rm);
		} else
			ERP_IE_handler(padapter, (PNDIS_802_11_VARIABLE_IEs)p);

	}

	/* update privacy/security */
	if (cap & BIT(4))
		pbss_network->Privacy = 1;
	else {
		pbss_network->Privacy = 0;
		/* Reset security when beacon shows no security */
		rtw_reset_securitypriv(padapter);
	}

	psecuritypriv->wpa_psk = 0;

	/* wpa2 */
	akm = 0;
	gmcs = 0;
	group_cipher = 0;
	pairwise_cipher = 0;
	psecuritypriv->wpa2_group_cipher = _NO_PRIVACY_;
	psecuritypriv->wpa2_pairwise_cipher = _NO_PRIVACY_;
	psecuritypriv->akmp = 0;
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _RSN_IE_2_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if (p && ie_len > 0) {
		if (rtw_parse_wpa2_ie(p, ie_len + 2, &group_cipher, &pairwise_cipher, &gmcs, &akm, &mfp_opt) == _SUCCESS) {
			psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_8021X;
			psecuritypriv->ndisauthtype = Ndis802_11AuthModeWPA2PSK;
			psecuritypriv->dot8021xalg = 1;/* psk,  todo:802.1x */
			psecuritypriv->wpa_psk |= BIT(1);

			psecuritypriv->wpa2_group_cipher = group_cipher;
			psecuritypriv->wpa2_pairwise_cipher = pairwise_cipher;
			psecuritypriv->akmp = akm;

#ifdef CONFIG_IOCTL_CFG80211
			/**
			 * Kernel < v5.x, the auth_type set as
			 * NL80211_AUTHTYPE_AUTOMATIC in
			 * cfg80211_rtw_start_ap(). if the AKM SAE in the RSN
			 * IE, we have to update the auth_type for SAE in
			 * rtw_check_beacon_data()
			 */
			if (CHECK_BIT(WLAN_AKM_TYPE_SAE, akm)) {
				RTW_INFO("%s: Auth type as SAE\n", __func__);
				psecuritypriv->auth_type = MLME_AUTHTYPE_SAE;
				psecuritypriv->auth_alg = WLAN_AUTH_SAE;
			}
#endif /* CONFIG_IOCTL_CFG80211 */
		}
	}

	/*RSNXE WPA3 R3*/
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _RSN_EX_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if (p && ie_len > 0) {
		RTW_INFO("[%s]Get _RSN_EX_IE_, len = %d\n", __func__, ie_len);
		if(ie_len+2 > MAX_RSN_XE_IE_LEN){
			RTW_ERR(FUNC_ADPT_FMT" size of ext_capab_ie_data is NOT enough.(%d)\n", FUNC_ADPT_ARG(padapter), ie_len);
		}else{
			psecuritypriv->rsn_xe_len = ie_len+2;
			_rtw_memcpy(psecuritypriv->rsn_xe, p, ie_len+2);

		}
	}

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	/* wapi */
	akm = 0;
	gmcs = 0;
	group_cipher = 0;
	pairwise_cipher = 0;
	psecuritypriv->wapi_group_cipher = _NO_PRIVACY_;
	psecuritypriv->wapi_pairwise_cipher = _NO_PRIVACY_;
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _WAPI_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if (p && ie_len > 0) {
		if (rtw_parse_wapi_ie(p, ie_len + 2, &group_cipher, &pairwise_cipher, &gmcs, &akm, &mfp_opt) == _SUCCESS) {
			psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_WAPI;
			psecuritypriv->ndisauthtype = Ndis802_11AuthModeWAPI;
			psecuritypriv->dot8021xalg = 1; /* psk, TODO: 802.1x */
			psecuritypriv->wpa_psk |= BIT(2);

			psecuritypriv->wapi_group_cipher = group_cipher;
			psecuritypriv->wapi_pairwise_cipher = pairwise_cipher;
			psecuritypriv->akmp = akm;
		}
	}
#endif

	/* wpa */
	ie_len = 0;
	group_cipher = 0;
	pairwise_cipher = 0;
	psecuritypriv->wpa_group_cipher = _NO_PRIVACY_;
	psecuritypriv->wpa_pairwise_cipher = _NO_PRIVACY_;
	for (p = ie + _BEACON_IE_OFFSET_; ; p += (ie_len + 2)) {
		p = rtw_get_ie(p, _SSN_IE_1_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_ - (ie_len + 2)));
		if ((p) && (_rtw_memcmp(p + 2, OUI1, 4))) {
			if (rtw_parse_wpa_ie(p, ie_len + 2, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS) {
				psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_8021X;
				psecuritypriv->ndisauthtype = Ndis802_11AuthModeWPAPSK;
				psecuritypriv->dot8021xalg = 1;/* psk,  todo:802.1x */

				psecuritypriv->wpa_psk |= BIT(0);

				psecuritypriv->wpa_group_cipher = group_cipher;
				psecuritypriv->wpa_pairwise_cipher = pairwise_cipher;

			}

			break;

		}

		if ((p == NULL) || (ie_len == 0))
			break;

	}

	if (mfp_opt == MFP_INVALID) {
		RTW_INFO(FUNC_ADPT_FMT" invalid MFP setting\n", FUNC_ADPT_ARG(padapter));
		return _FAIL;
	}
	psecuritypriv->mfp_opt = mfp_opt;

	/* wmm */
	ie_len = 0;
	pmlmepriv->qospriv.qos_option = 0;
#ifdef CONFIG_RTW_MESH
	if (MLME_IS_MESH(padapter))
		pmlmepriv->qospriv.qos_option = 1;
#endif
	if (pregistrypriv->wmm_enable) {
		for (p = ie + _BEACON_IE_OFFSET_; ; p += (ie_len + 2)) {
			p = rtw_get_ie(p, _VENDOR_SPECIFIC_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_ - (ie_len + 2)));
			if ((p) && _rtw_memcmp(p + 2, WMM_PARA_IE, 6)) {
				pmlmepriv->qospriv.qos_option = 1;

				/* QoS Info, support U-APSD */
				if (*(p + 8) & AP_SUPPORTED_UAPSD)
					pmlmepriv->qospriv.uapsd_enable = 1;
				else
					pmlmepriv->qospriv.uapsd_enable = 0;

#ifdef CONFIG_RTW_MANUAL_EDCA
				if(pregistrypriv->manual_edca) {
					_rtw_memcpy(p+2, GET_WMM_PARA_IE, 24);
					if (pmlmepriv->qospriv.uapsd_enable)
						*(p + 8) |= AP_SUPPORTED_UAPSD;
					else
						*(p + 8) &= ~AP_SUPPORTED_UAPSD;
				}
#endif
				WMM_param_handler(padapter, (PNDIS_802_11_VARIABLE_IEs)p);

				break;
			}

			if ((p == NULL) || (ie_len == 0))
				break;
		}
	}
#ifdef CONFIG_80211N_HT
	if(padapter->registrypriv.ht_enable &&
		is_supported_ht(padapter->registrypriv.wireless_mode)) {
		/* parsing HT_CAP_IE */
		p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _HT_CAPABILITY_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
		if (p && ie_len > 0) {
			HT_CAP_AMPDU_FACTOR max_rx_ampdu_factor = MAX_AMPDU_FACTOR_64K;
			struct rtw_ieee80211_ht_cap *pht_cap = (struct rtw_ieee80211_ht_cap *)(p + 2);

			if (0) {
				RTW_INFO(FUNC_ADPT_FMT" HT_CAP_IE from upper layer:\n", FUNC_ADPT_ARG(padapter));
				dump_ht_cap_ie_content(RTW_DBGDUMP, p + 2, ie_len);
			}

			pHT_caps_ie = p;

			ht_cap = _TRUE;
			network_type |= WLAN_MD_11N;

			/* parse shortGI 20/40Mhz */
			if (pht_cap->cap_info & cpu_to_le16(IEEE80211_HT_CAP_SGI_20))
				pregistrypriv->short_gi |= BIT0;

			if (pht_cap->cap_info & cpu_to_le16(IEEE80211_HT_CAP_SGI_40))
				pregistrypriv->short_gi |= BIT1;

			if (pht_cap->cap_info & cpu_to_le16(IEEE80211_HT_CAP_SUP_WIDTH))
				bw_mode = BW_CAP_40M;

			pregistrypriv->bw_mode = (bw_mode << 4) | bw_mode;

			rtw_ht_use_default_setting(padapter);
#ifdef CONFIG_RTW_AP_EXT_SUPPORT
			rtw_ht_apply_mib_setting(padapter);
#endif

			/* Update HT Capabilities Info field */
			if (pmlmepriv->htpriv.sgi_20m == _FALSE)
				pht_cap->cap_info &= ~ cpu_to_le16(IEEE80211_HT_CAP_SGI_20);

			if (pmlmepriv->htpriv.sgi_40m == _FALSE)
				pht_cap->cap_info &= ~ cpu_to_le16(IEEE80211_HT_CAP_SGI_40);

			if (!TEST_FLAG(pmlmepriv->htpriv.ldpc_cap, LDPC_HT_ENABLE_RX))
				pht_cap->cap_info &= ~ cpu_to_le16(IEEE80211_HT_CAP_LDPC_CODING);

			if (!TEST_FLAG(pmlmepriv->htpriv.stbc_cap, STBC_HT_ENABLE_TX))
				pht_cap->cap_info &= ~ cpu_to_le16(IEEE80211_HT_CAP_TX_STBC);

			if (!TEST_FLAG(pmlmepriv->htpriv.stbc_cap, STBC_HT_ENABLE_RX))
				pht_cap->cap_info &= ~ cpu_to_le16(IEEE80211_HT_CAP_RX_STBC_3R);

			/* Update HT AMSDU field */
			if (padapter->phl_role->proto_role_cap.max_amsdu_len > 0) {
				if (padapter->driver_rx_amsdu_size == 0xF)
					pht_cap->cap_info |=  cpu_to_le16(IEEE80211_HT_CAP_MAX_AMSDU);
				else { /* customize rx amsdu size */
					if (padapter->driver_rx_amsdu_size > 0)
						pht_cap->cap_info |= cpu_to_le16(IEEE80211_HT_CAP_MAX_AMSDU);
					else
						pht_cap->cap_info &=  ~ cpu_to_le16(IEEE80211_HT_CAP_MAX_AMSDU);
				}
			} else
				pht_cap->cap_info &=  ~ cpu_to_le16(IEEE80211_HT_CAP_MAX_AMSDU);

			/* As no greenfield support currently or in the future,
			 * clear greenfield RX in case of wrong setting from user. */
			pht_cap->cap_info &= ~cpu_to_le16(IEEE80211_HT_CAP_GRN_FLD);

			/* Update A-MPDU Parameters field */
			pht_cap->ampdu_params_info &= ~(IEEE80211_HT_CAP_AMPDU_FACTOR | IEEE80211_HT_CAP_AMPDU_DENSITY);

			if ((psecuritypriv->wpa_pairwise_cipher & WPA_CIPHER_CCMP) ||
				(psecuritypriv->wpa2_pairwise_cipher & WPA_CIPHER_CCMP)) {
				rtw_hal_get_def_var(padapter, HW_VAR_BEST_AMPDU_DENSITY, &best_ampdu_density);
				pht_cap->ampdu_params_info |= (IEEE80211_HT_CAP_AMPDU_DENSITY & (best_ampdu_density << 2));
			} else
				pht_cap->ampdu_params_info |= (IEEE80211_HT_CAP_AMPDU_DENSITY & 0x00);

			rtw_hal_get_def_var(padapter, HW_VAR_MAX_RX_AMPDU_FACTOR, &max_rx_ampdu_factor);
			pht_cap->ampdu_params_info |= (IEEE80211_HT_CAP_AMPDU_FACTOR & max_rx_ampdu_factor); /* set  Max Rx AMPDU size  to 64K */

			_rtw_memcpy(&(pmlmeinfo->HT_caps), pht_cap, sizeof(struct HT_caps_element));

			/* Update Supported MCS Set field */
			{
				u8 rx_nss = 0;
				int i;

				rx_nss = GET_HAL_RX_NSS(adapter_to_dvobj(padapter));

				/* RX MCS Bitmask */
				switch (rx_nss) {
				case 1:
					set_mcs_rate_by_mask(HT_CAP_ELE_RX_MCS_MAP(pht_cap), MCS_RATE_1R);
					break;
				case 2:
					set_mcs_rate_by_mask(HT_CAP_ELE_RX_MCS_MAP(pht_cap), MCS_RATE_2R);
					break;
				case 3:
					set_mcs_rate_by_mask(HT_CAP_ELE_RX_MCS_MAP(pht_cap), MCS_RATE_3R);
					break;
				case 4:
					set_mcs_rate_by_mask(HT_CAP_ELE_RX_MCS_MAP(pht_cap), MCS_RATE_4R);
					break;
				default:
					RTW_WARN("rf_type:%d or rx_nss:%u is not expected\n",
						GET_HAL_RFPATH(adapter_to_dvobj(padapter)), rx_nss);
				}
				for (i = 0; i < 10; i++)
					*(HT_CAP_ELE_RX_MCS_MAP(pht_cap) + i) &= padapter->mlmeextpriv.default_supported_mcs_set[i];
			}

#ifdef CONFIG_BEAMFORMING
			/* Use registry value to enable HT Beamforming. */
			/* ToDo: use configure file to set these capability. */
			pht_cap->tx_BF_cap_info = 0;

			/* HT Beamformer */
			if (TEST_FLAG(pmlmepriv->htpriv.beamform_cap, BEAMFORMING_HT_BEAMFORMER_ENABLE)) {
				/* Transmit NDP Capable */
				SET_HT_CAP_TXBF_TRANSMIT_NDP_CAP(pht_cap, 1);
				/* Explicit Compressed Steering Capable */
				SET_HT_CAP_TXBF_EXPLICIT_COMP_STEERING_CAP(pht_cap, 1);
				/* Compressed Steering Number Antennas */
				SET_HT_CAP_TXBF_COMP_STEERING_NUM_ANTENNAS(pht_cap, 1);
				rtw_hal_get_def_var(padapter, HAL_DEF_BEAMFORMER_CAP, (u8 *)&rf_num);
				if (rf_num > 3)
					rf_num = 3;
				SET_HT_CAP_TXBF_CHNL_ESTIMATION_NUM_ANTENNAS(pht_cap, rf_num);
			}

			/* HT Beamformee */
			if (TEST_FLAG(pmlmepriv->htpriv.beamform_cap, BEAMFORMING_HT_BEAMFORMEE_ENABLE)) {
				/* Receive NDP Capable */
				SET_HT_CAP_TXBF_RECEIVE_NDP_CAP(pht_cap, 1);
				/* Explicit Compressed Beamforming Feedback Capable */
				SET_HT_CAP_TXBF_EXPLICIT_COMP_FEEDBACK_CAP(pht_cap, 2);
				rtw_hal_get_def_var(padapter, HAL_DEF_BEAMFORMEE_CAP, (u8 *)&rf_num);
				if (rf_num > 3)
					rf_num = 3;
				SET_HT_CAP_TXBF_COMP_STEERING_NUM_ANTENNAS(pht_cap, rf_num);
			}
#endif /* CONFIG_BEAMFORMING */

			_rtw_memcpy(&pmlmepriv->htpriv.ht_cap, p + 2, ie_len);

			if (0) {
				RTW_INFO(FUNC_ADPT_FMT" HT_CAP_IE driver masked:\n", FUNC_ADPT_ARG(padapter));
				dump_ht_cap_ie_content(RTW_DBGDUMP, p + 2, ie_len);
			}
		}

		/* parsing HT_INFO_IE */
		p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _HT_ADD_INFO_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
		if (p && ie_len > 0) {
			pHT_info_ie = p;
			if (channel == 0)
				pbss_network->Configuration.DSConfig = GET_HT_OP_ELE_PRI_CHL(pHT_info_ie + 2);
			else if (channel != GET_HT_OP_ELE_PRI_CHL(pHT_info_ie + 2)) {
				RTW_INFO(FUNC_ADPT_FMT" ch inconsistent, DSSS:%u, HT primary:%u\n"
					, FUNC_ADPT_ARG(padapter), channel, GET_HT_OP_ELE_PRI_CHL(pHT_info_ie + 2));
			}
			if (GET_CFG80211_REPORT_MGMT(adapter_wdev_data(padapter), IEEE80211_STYPE_PROBE_REQ) == _TRUE)
				memcpy(pmlmepriv->htpriv.ht_op, pHT_info_ie+2, HT_OP_IE_LEN);
		}
	}
#endif /* CONFIG_80211N_HT */
	pmlmepriv->cur_network.network_type = network_type;

#ifdef CONFIG_80211N_HT
	pmlmepriv->htpriv.ht_option = _FALSE;

	if ((psecuritypriv->wpa2_pairwise_cipher & WPA_CIPHER_TKIP) ||
	    (psecuritypriv->wpa_pairwise_cipher & WPA_CIPHER_TKIP)) {
		/* todo: */
		/* ht_cap = _FALSE; */
	}

	/* ht_cap	 */
	if (padapter->registrypriv.ht_enable &&
		is_supported_ht(padapter->registrypriv.wireless_mode) && ht_cap == _TRUE) {

		pmlmepriv->htpriv.ht_option = _TRUE;
		pmlmepriv->qospriv.qos_option = 1;

		pmlmepriv->htpriv.ampdu_enable = pregistrypriv->ampdu_enable ? _TRUE : _FALSE;

		HT_caps_handler(padapter, (PNDIS_802_11_VARIABLE_IEs)pHT_caps_ie);

		HT_info_handler(padapter, (PNDIS_802_11_VARIABLE_IEs)pHT_info_ie);
	}
#endif

#ifdef CONFIG_80211AC_VHT
	pmlmepriv->ori_vht_en = 0;
	pmlmepriv->vhtpriv.vht_option = _FALSE;

	if (pmlmepriv->htpriv.ht_option == _TRUE
#ifndef CONFIG_24G_256QAM
		&& pbss_network->Configuration.DSConfig > 14
#endif /* CONFIG_24G_256QAM */
		&& REGSTY_IS_11AC_ENABLE(pregistrypriv)
		&& is_supported_vht(pregistrypriv->wireless_mode)
		&& (!rfctl->country_ent || COUNTRY_CHPLAN_EN_11AC(rfctl->country_ent))
	) {
		/* Parsing VHT CAP IE */
		p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, EID_VHTCapability, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
		if (p && ie_len > 0) {
			struct rtw_ieee80211_vht_cap *pvht_cap = (struct rtw_ieee80211_vht_cap *)(p + 2);

			vht_cap = _TRUE;
			/* parsing shortGI 80/160 BW */  //charlie
			if (((cpu_to_le32(pvht_cap->vht_cap_info) & IEEE80211_VHT_CAP_SUPPORTED_CHANNEL_WIDTH) >> 2) == 1)
				bw_mode = BW_CAP_160M;
			else if (((cpu_to_le32(pvht_cap->vht_cap_info) & IEEE80211_VHT_CAP_SUPPORTED_CHANNEL_WIDTH) >> 2) == 2)
				bw_mode = BW_CAP_80_80M;


			/* parsing shortGI 80/160 Mhz */
			if (cpu_to_le32(pvht_cap->vht_cap_info) & IEEE80211_VHT_CAP_SGI_80)
				pregistrypriv->short_gi |= BIT2;

			if (cpu_to_le32(pvht_cap->vht_cap_info) & IEEE80211_VHT_CAP_SGI_160)
				pregistrypriv->short_gi |= BIT3;
		}

		/* Parsing VHT OPERATION IE */
		p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, EID_VHTOperation, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
		if (p && ie_len > 0) {
			struct rtw_ieee80211_vht_operation *pvht_op = (struct rtw_ieee80211_vht_operation *)(p+2);
#if 0
			if (pvht_op->chan_width == IEEE80211_VHT_CHANWIDTH_80MHZ)
				bw_mode = CHANNEL_WIDTH_80;
			else if (pvht_op->chan_width == IEEE80211_VHT_CHANWIDTH_160MHZ)
				bw_mode = CHANNEL_WIDTH_160;
			else if (pvht_op->chan_width == IEEE80211_VHT_CHANWIDTH_80P80MHZ)
				bw_mode = CHANNEL_WIDTH_80_80;
#endif
			if ((pvht_op->chan_width == IEEE80211_VHT_CHANWIDTH_80MHZ) && (pvht_op->center_freq_seg1_idx ==0))
				bw_mode = BW_CAP_80M;

		}

		pregistrypriv->bw_mode = (bw_mode << 4) | bw_mode;

		if (vht_cap == _TRUE
			&& MLME_IS_MESH(padapter) /* allow only mesh temporarily before VHT IE checking is ready */
		) {
			rtw_check_for_vht20(padapter, ie + _BEACON_IE_OFFSET_, pbss_network->IELength - _BEACON_IE_OFFSET_);
			pmlmepriv->ori_vht_en = 1;
			pmlmepriv->vhtpriv.vht_option = _TRUE;
		} else if (vht_cap == _TRUE && REGSTY_IS_11AC_AUTO(pregistrypriv)) {
			rtw_vht_ies_detach(padapter, pbss_network);
			rtw_vht_ies_attach(padapter, pbss_network);
		}
	}

	if (pmlmepriv->vhtpriv.vht_option == _FALSE)
		rtw_vht_ies_detach(padapter, pbss_network);
#endif /* CONFIG_80211AC_VHT */

	/* get ch/bw/offset */
	rtw_ies_get_chbw(pbss_network->IEs + _BEACON_IE_OFFSET_, pbss_network->IELength - _BEACON_IE_OFFSET_
		, &pmlmepriv->ori_ch, &pmlmepriv->ori_bw, &pmlmepriv->ori_offset, 1, 1);

#ifdef CONFIG_80211AX_HE
	pmlmepriv->hepriv.he_option = _FALSE;
	if (pmlmepriv->htpriv.ht_option == _TRUE
		&& REGSTY_IS_11AX_ENABLE(pregistrypriv)
		&& is_supported_he(pregistrypriv->wireless_mode)
		/* CONFIG_80211AX_HE_TODO
		&& (!rfctl->country_ent || COUNTRY_CHPLAN_EN_11AX(rfctl->country_ent)) */
	) {
		u8 he_cap = _FALSE;
		u8 he_cap_eid_ext = WLAN_EID_EXTENSION_HE_CAPABILITY;

		p = rtw_get_ie_ex(ie + _BEACON_IE_OFFSET_, pbss_network->IELength - _BEACON_IE_OFFSET_,
			WLAN_EID_EXTENSION, &he_cap_eid_ext, 1, NULL, &ie_len);
		if (p && ie_len > 0)
			he_cap = _TRUE;

		/* If He capability is in beacon IE, enable he_option */
		pmlmepriv->hepriv.he_option = he_cap;

		if (he_cap == _TRUE && REGSTY_IS_11AX_AUTO(pregistrypriv)) {
			rtw_he_ies_detach(padapter, pbss_network);
			rtw_he_ies_attach(padapter, pbss_network);
		}
	}
	if (pmlmepriv->hepriv.he_option == _FALSE)
		rtw_he_ies_detach(padapter, pbss_network);
	else if (padapter->phl_role->proto_role_cap.twt & BIT(1)) {
		RTW_INFO("SET_EXT_CAPABILITY_ELE_TWT_RESPONDER\n");
		pmlmepriv->ext_capab_ie_len = 12;
		SET_EXT_CAPABILITY_ELE_TWT_RESPONDER(pmlmepriv->ext_capab_ie_data, 1);
	}
#endif

#ifdef CONFIG_80211N_HT
	if(padapter->registrypriv.ht_enable &&
		is_supported_ht(padapter->registrypriv.wireless_mode) &&
		pbss_network->Configuration.DSConfig <= 14 /*&& padapter->registrypriv.wifi_spec == 1*/) {
		/* check BSS coexist bit is carried from hostapd */
		if (GET_EXT_CAPABILITY_ELE_BSS_COEXIST(pmlmepriv->ext_capab_ie_data))
			pmlmepriv->htpriv.bss_coexist = 1;
		/* If BSS coexist is carried by hostapd, or driver mib coexist is set */
		if (pmlmepriv->htpriv.bss_coexist || padapter->registrypriv.coex_2g_40m) {
			SET_EXT_CAPABILITY_ELE_BSS_COEXIST(pmlmepriv->ext_capab_ie_data, 1);
			RTW_INFO("[%s]20/40 coexist set!\n", __func__);
			if (pmlmepriv->ext_capab_ie_len == 0)
				pmlmepriv->ext_capab_ie_len = 1;
		}
	}
#endif /* CONFIG_80211N_HT */

	if (pmlmepriv->ext_capab_ie_len) {
		rtw_remove_bcn_ie(padapter, pbss_network, EID_EXTCapability);
		rtw_remove_bcn_ie(padapter, pbss_network, EID_EXTCapability); /* hostapd sends two EXT_CAP */
		if (pbss_network->IELength + pmlmepriv->ext_capab_ie_len + 2 > MAX_IE_SZ)
			RTW_ERR(FUNC_ADPT_FMT" Total IE length(%d) exceed MAX_IE_SZ.\n",
				FUNC_ADPT_ARG(padapter), pbss_network->IELength + pmlmepriv->ext_capab_ie_len + 2);
		else
			rtw_set_ie(pbss_network->IEs + pbss_network->IELength, EID_EXTCapability,
				pmlmepriv->ext_capab_ie_len, pmlmepriv->ext_capab_ie_data,
				&pbss_network->IELength);
	}

#ifdef CONFIG_RTW_80211K
	padapter->rmpriv.enable = _FALSE;
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, EID_RMEnabledCapability, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if (p && ie_len) {
		RTW_INFO("[%s]Get EID_RMEnabledCapability, len = %d\n", __func__, ie_len);
		padapter->rmpriv.enable = _TRUE;
		_rtw_memcpy(padapter->rmpriv.rm_en_cap_def, p + 2, ie_len);
	} else {
		_rtw_memset(padapter->rmpriv.rm_en_cap_def, 0, 5);
	}
#endif

#ifdef CONFIG_RTW_MBO
	ie_len = 0;
	pmlmepriv->mbopriv.enable = _FALSE;
	for (p = ie + _BEACON_IE_OFFSET_; ; p += (ie_len + 2)) {
		p = rtw_get_ie(p, _SSN_IE_1_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_ - (ie_len + 2)));
		if ((p) && (_rtw_memcmp(p + 2, WIFI_ALLIANCE_OUI, 3)) && (*(p+5) == MBO_OUI_TYPE)) {
			/* find MBO-OCE information element */
			pmlmepriv->mbopriv.enable = _TRUE;
			rtw_mbo_ie_handler(padapter, &pmlmepriv->mbopriv, p + 6, ie_len - 4);
			break;
		}
		if ((p == NULL) || (ie_len == 0))
			break;
	}
#endif

#ifdef CONFIG_RTW_80211R
	pmlmepriv->ft_roam.ft_flags = 0;
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, EID_MDIE, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if (p && ie_len) {
		RTW_PRINT("[FT] enable 802.11r\n");
		rtw_ft_set_flags(padapter, RTW_FT_EN);

		pmlmepriv->ft_roam.mdid = *(u16 *)(p + 2);
		pmlmepriv->ft_roam.ft_cap = *(p + 4);

		if (pmlmepriv->ft_roam.ft_cap & 0x1)
			rtw_ft_set_flags(padapter, RTW_FT_OTD_EN);
	}
#endif

	/* country */
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _COUNTRY_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if (p && ie_len) {
		u8 *countrycode = (p + 2);

		rtw_set_country_cmd(padapter, RTW_CMDF_WAIT_ACK, countrycode, 1);
		if (memcmp(countrycode, "RU", 2) == 0){
			memcpy(countrycode, "US", 2);
		}
	}

	pbss_network->Length = get_WLAN_BSSID_EX_sz((WLAN_BSSID_EX *)pbss_network);

	/* move upper for he cap update
	rtw_ies_get_chbw(pbss_network->IEs + _BEACON_IE_OFFSET_, pbss_network->IELength - _BEACON_IE_OFFSET_
		, &pmlmepriv->ori_ch, &pmlmepriv->ori_bw, &pmlmepriv->ori_offset, 1, 1);
	*/

	#if defined(CONFIG_RTW_DEBUG_MBSSID_VAP) && (CONFIG_RTW_DEBUG_MBSSID_VAP > 0)
	RTW_PRINT("IE CH(%d) BW(%d) OF(%d)\n", pmlmepriv->ori_ch,
	          pmlmepriv->ori_bw, pmlmepriv->ori_offset);
	#endif /* CONFIG_RTW_DEBUG_MBSSID_VAP */

	rtw_warn_on(pmlmepriv->ori_ch == 0);

	rtw_startbss_cmd(padapter, RTW_CMDF_WAIT_ACK);
	{
		int sk_band = RTW_GET_SCAN_BAND_SKIP(padapter);

		if (sk_band)
			RTW_CLR_SCAN_BAND_SKIP(padapter, sk_band);
	}

	#if (PHL_VER_CODE >= PHL_VERSION(0001, 0017, 0000, 0000))
	#ifdef CONFIG_AP_CMD_DISPR
	rtw_phl_ap_started(padapter->dvobj->phl, 0, padapter->phl_role);
	#else
	rtw_phl_ap_started(padapter->dvobj->phl, padapter->phl_role);
	#endif /* CONFIG_AP_CMD_DISPR */
	#endif /* PHL_VER_CODE */

	rtw_indicate_connect(padapter);

	pmlmepriv->cur_network.join_res = _TRUE;/* for check if already set beacon */

	/* update bc/mc sta_info */
	/* update_bmc_sta(padapter); */

	return ret;

}

int rtw_update_probersp_data(_adapter *padapter, u8 tx_ch, u8 *pbuf,  int *len)
{
	int ret = _SUCCESS;
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	u8 *tmpbuf;
#endif

#ifdef CONFIG_80211N_HT
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	sint ie_len = 0;
	u8 *p = NULL;

	if (pmlmepriv->htpriv.ht_option == _TRUE) {
		p = rtw_get_ie(pbuf + WLAN_HDR_A3_LEN + _FIXED_IE_LENGTH_, EID_HTInfo, &ie_len,
				*len -WLAN_HDR_A3_LEN - _FIXED_IE_LENGTH_);
		if (p && ie_len == HT_OP_IE_LEN) {
			/* HT Info 1 - 2nd Channel Offset & STA Channel width */
			*(p+3) = pmlmepriv->htpriv.ht_op[1];
			/* HT Info 2 - HT Protection */
			*(p+4) = pmlmepriv->htpriv.ht_op[2];
		}
		if (pmlmepriv->ext_capab_ie_len) {
			rtw_ies_remove_ie(pbuf, len, WLAN_HDR_A3_LEN + _FIXED_IE_LENGTH_,
					EID_EXTCapability, NULL, 0);
			rtw_set_ie(pbuf + *len, EID_EXTCapability, pmlmepriv->ext_capab_ie_len,
					pmlmepriv->ext_capab_ie_data, len);
		}
#ifdef CONFIG_80211AC_VHT
		if (pmlmepriv->vhtpriv.vht_option == _TRUE
			&& tx_ch > 14
			&& REGSTY_IS_11AC_ENABLE(pregistrypriv)
			&& is_supported_vht(pregistrypriv->wireless_mode)
		) {
			p = rtw_get_ie(pbuf + WLAN_HDR_A3_LEN + _FIXED_IE_LENGTH_, EID_VHTCapability, &ie_len,
					*len -WLAN_HDR_A3_LEN - _FIXED_IE_LENGTH_);
			if (!p) {
				rtw_set_ie(pbuf + *len, EID_VHTCapability, VHT_CAP_IE_LEN, pmlmepriv->vhtpriv.vht_cap, len);
				rtw_set_ie(pbuf + *len, EID_VHTOperation, VHT_OP_IE_LEN, pmlmepriv->vhtpriv.vht_op, len);
			} else {
				if (p && ie_len == VHT_CAP_IE_LEN)
					memcpy(p+2, pmlmepriv->vhtpriv.vht_cap, VHT_CAP_IE_LEN);

				p = rtw_get_ie(pbuf + WLAN_HDR_A3_LEN + _FIXED_IE_LENGTH_, EID_VHTOperation, &ie_len,
						*len -WLAN_HDR_A3_LEN - _FIXED_IE_LENGTH_);
				if (p && ie_len == VHT_OP_IE_LEN)
					memcpy(p+2, pmlmepriv->vhtpriv.vht_op, VHT_OP_IE_LEN);
			}
		}
#endif /* CONFIG_80211AC_VHT */
	}
#endif /* CONFIG_80211N_HT */

#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	tmpbuf = pbuf + *len;
	*len += rtw_build_vendor_ie(padapter , &tmpbuf , WIFI_PROBERESP_VENDOR_IE_BIT);
#endif

	return ret;
}

#if CONFIG_RTW_MACADDR_ACL
void rtw_macaddr_acl_init(_adapter *adapter, u8 period)
{
	struct sta_priv *stapriv = &adapter->stapriv;
	struct wlan_acl_pool *acl;
	_queue *acl_node_q;
	int i;

	if (period >= RTW_ACL_PERIOD_NUM) {
		rtw_warn_on(1);
		return;
	}

	acl = &stapriv->acl_list[period];
	acl_node_q = &acl->acl_node_q;

	_rtw_spinlock_init(&(acl_node_q->lock));

	_rtw_spinlock_bh(&(acl_node_q->lock));
	_rtw_init_listhead(&(acl_node_q->queue));
	acl->num = 0;
	acl->mode = RTW_ACL_MODE_DISABLED;
	for (i = 0; i < NUM_ACL; i++) {
		_rtw_init_listhead(&acl->aclnode[i].list);
		acl->aclnode[i].valid = _FALSE;
		acl->aclnode[i].hit = 0;
	}
	_rtw_spinunlock_bh(&(acl_node_q->lock));
}

static void _rtw_macaddr_acl_deinit(_adapter *adapter, u8 period, bool clear_only)
{
	struct sta_priv *stapriv = &adapter->stapriv;
	struct wlan_acl_pool *acl;
	_queue *acl_node_q;
	_list *head, *list;
	struct rtw_wlan_acl_node *acl_node;

	if (period >= RTW_ACL_PERIOD_NUM) {
		rtw_warn_on(1);
		return;
	}

	acl = &stapriv->acl_list[period];
	acl_node_q = &acl->acl_node_q;

	_rtw_spinlock_bh(&(acl_node_q->lock));
	head = get_list_head(acl_node_q);
	list = get_next(head);
	while (rtw_end_of_queue_search(head, list) == _FALSE) {
		acl_node = LIST_CONTAINOR(list, struct rtw_wlan_acl_node, list);
		list = get_next(list);

		if (acl_node->valid == _TRUE) {
			acl_node->valid = _FALSE;
			acl_node->hit = 0;
			rtw_list_delete(&acl_node->list);
			acl->num--;
		}
	}
	_rtw_spinunlock_bh(&(acl_node_q->lock));

	if (!clear_only)
		_rtw_spinlock_free(&(acl_node_q->lock));

	rtw_warn_on(acl->num);
	acl->mode = RTW_ACL_MODE_DISABLED;
}

void rtw_macaddr_acl_deinit(_adapter *adapter, u8 period)
{
	_rtw_macaddr_acl_deinit(adapter, period, 0);
}

void rtw_macaddr_acl_clear(_adapter *adapter, u8 period)
{
	_rtw_macaddr_acl_deinit(adapter, period, 1);
}

void rtw_set_macaddr_acl(_adapter *adapter, u8 period, int mode)
{
	struct sta_priv *stapriv = &adapter->stapriv;
	struct wlan_acl_pool *acl;

	if (period >= RTW_ACL_PERIOD_NUM) {
		rtw_warn_on(1);
		return;
	}

	acl = &stapriv->acl_list[period];

	RTW_INFO(FUNC_ADPT_FMT" p=%u, mode=%d\n"
		, FUNC_ADPT_ARG(adapter), period, mode);

	acl->mode = mode;
}

int rtw_acl_add_sta(_adapter *adapter, u8 period, const u8 *addr, u32 valid_time)
{
	_list *list, *head;
	u8 existed = 0;
	int i = -1, ret = 0;
	struct rtw_wlan_acl_node *acl_node;
	struct sta_priv *stapriv = &adapter->stapriv;
	struct wlan_acl_pool *acl;
	_queue *acl_node_q;

	if (period >= RTW_ACL_PERIOD_NUM) {
		rtw_warn_on(1);
		ret = -1;
		goto exit;
	}

	acl = &stapriv->acl_list[period];
	acl_node_q = &acl->acl_node_q;

	_rtw_spinlock_bh(&(acl_node_q->lock));

	head = get_list_head(acl_node_q);
	list = get_next(head);

	/* search for existed entry */
	while (rtw_end_of_queue_search(head, list) == _FALSE) {
		acl_node = LIST_CONTAINOR(list, struct rtw_wlan_acl_node, list);
		list = get_next(list);

		if (_rtw_memcmp(acl_node->addr, addr, ETH_ALEN)) {
			if (acl_node->valid == _TRUE) {
				existed = 1;
				acl_node->valid_time = valid_time;
				break;
			}
		}
	}
	if (existed)
		goto release_lock;

	if (acl->num >= NUM_ACL)
		goto release_lock;

	/* find empty one and use */
	for (i = 0; i < NUM_ACL; i++) {

		acl_node = &acl->aclnode[i];
		if (acl_node->valid == _FALSE) {

			_rtw_init_listhead(&acl_node->list);
			_rtw_memcpy(acl_node->addr, addr, ETH_ALEN);
			acl_node->valid = _TRUE;
			acl_node->hit = 0;
			acl_node->valid_time = valid_time;

			rtw_list_insert_tail(&acl_node->list, get_list_head(acl_node_q));
			acl->num++;
			break;
		}
	}

release_lock:
	_rtw_spinunlock_bh(&(acl_node_q->lock));

	if (!existed && (i < 0 || i >= NUM_ACL))
		ret = -1;

	RTW_INFO(FUNC_ADPT_FMT" p=%u "MAC_FMT" %s (acl_num=%d)\n"
		 , FUNC_ADPT_ARG(adapter), period, MAC_ARG(addr)
		, (existed ? "existed" : ((i < 0 || i >= NUM_ACL) ? "no room" : "added"))
		 , acl->num);
exit:
	return ret;
}

int rtw_acl_remove_sta(_adapter *adapter, u8 period, const u8 *addr)
{
	_list *list, *head;
	int ret = 0;
	struct rtw_wlan_acl_node *acl_node;
	struct sta_priv *stapriv = &adapter->stapriv;
	struct wlan_acl_pool *acl;
	_queue	*acl_node_q;
	u8 is_baddr = is_broadcast_mac_addr(addr);
	u8 match = 0;

	if (period >= RTW_ACL_PERIOD_NUM) {
		rtw_warn_on(1);
		goto exit;
	}

	acl = &stapriv->acl_list[period];
	acl_node_q = &acl->acl_node_q;

	_rtw_spinlock_bh(&(acl_node_q->lock));

	head = get_list_head(acl_node_q);
	list = get_next(head);

	while (rtw_end_of_queue_search(head, list) == _FALSE) {
		acl_node = LIST_CONTAINOR(list, struct rtw_wlan_acl_node, list);
		list = get_next(list);

		if (is_baddr || _rtw_memcmp(acl_node->addr, addr, ETH_ALEN)) {
			if (acl_node->valid == _TRUE) {
				acl_node->valid = _FALSE;
				acl_node->hit = 0;
				rtw_list_delete(&acl_node->list);
				acl->num--;
				match = 1;
			}
		}
	}

	_rtw_spinunlock_bh(&(acl_node_q->lock));

	RTW_INFO(FUNC_ADPT_FMT" p=%u "MAC_FMT" %s (acl_num=%d)\n"
		 , FUNC_ADPT_ARG(adapter), period, MAC_ARG(addr)
		 , is_baddr ? "clear all" : (match ? "match" : "no found")
		 , acl->num);

exit:
	return ret;
}

int rtw_acl_expire_sta(_adapter *adapter)
{
	_list *list, *head;
	int ret = 0;
	struct rtw_wlan_acl_node *acl_node;
	struct sta_priv *stapriv = &adapter->stapriv;
	struct wlan_acl_pool *acl;
	_queue	*acl_node_q;
	int period;

	for (period = 0; period < RTW_ACL_PERIOD_NUM; period++) {
		acl = &stapriv->acl_list[period];
		acl_node_q = &acl->acl_node_q;

		_rtw_spinlock_bh(&(acl_node_q->lock));

		head = get_list_head(acl_node_q);
		list = get_next(head);

		while (rtw_end_of_queue_search(head, list) == _FALSE) {
			acl_node = LIST_CONTAINOR(list, struct rtw_wlan_acl_node, list);
			list = get_next(list);
			if (acl_node->valid == _TRUE) {
				if (acl_node->valid_time == 1) {
					acl_node->valid = _FALSE;
					acl_node->hit = 0;
					rtw_list_delete(&acl_node->list);
					acl->num--;
				} else if (acl_node->valid_time) {
					acl_node->valid_time--;
				}
			}
		}

		_rtw_spinunlock_bh(&(acl_node_q->lock));
	}

	return ret;
}
#endif /* CONFIG_RTW_MACADDR_ACL */

#ifdef CONFIG_CMD_DISP
u8 rtw_ap_set_sta_key(_adapter *adapter, const u8 *addr, u8 alg, const u8 *key, u8 keyid, u8 gk)
{
	struct set_stakey_parm param;
	u8	res = _SUCCESS;

	_rtw_memcpy(param.addr, addr, ETH_ALEN);
	param.algorithm = alg;
	param.keyid = keyid;
	if (!!(alg & _SEC_TYPE_256_))
		_rtw_memcpy(param.key, key, 32);
	else
		_rtw_memcpy(param.key, key, 16);
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	if ((alg == _SMS4_) || (alg == _GCM_SM4_))
		_rtw_memcpy(param.key, key, 32);
#endif
	param.gk = gk;

	set_stakey_hdl(adapter, &param, PHL_CMD_NO_WAIT, 0);

exit:
	return res;
}

static int rtw_ap_set_key(_adapter *padapter, u8 *key, u8 alg, int keyid, u8 set_tx)
{
	u8 keylen;
	struct setkey_parm setkeyparm;
	int res = _SUCCESS;

	/* RTW_INFO("%s\n", __FUNCTION__); */

	_rtw_memset(&setkeyparm, 0, sizeof(struct setkey_parm));

	setkeyparm.keyid = (u8)keyid;
	if (is_wep_enc(alg))
		padapter->securitypriv.key_mask |= BIT(setkeyparm.keyid);

	setkeyparm.algorithm = alg;

	setkeyparm.set_tx = set_tx;

	switch (alg) {
	case _WEP40_:
		keylen = 5;
		break;
	case _WEP104_:
		keylen = 13;
		break;
	case _GCMP_256_:
	case _CCMP_256_:
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	case _SMS4_:
	case _GCM_SM4_:
#endif
		keylen = 32;
		break;
	case _TKIP_:
	case _TKIP_WTMIC_:
	case _AES_:
	case _GCMP_:
	#ifdef CONFIG_IEEE80211W
	case _BIP_CMAC_128_:
	#endif
	default:
		keylen = 16;
	}

	_rtw_memcpy(&(setkeyparm.key[0]), key, keylen);
	setkey_hdl(padapter, &setkeyparm, PHL_CMD_NO_WAIT, 0);

exit:

	return res;
}
#else
u8 rtw_ap_set_sta_key(_adapter *adapter, const u8 *addr, u8 alg, const u8 *key, u8 keyid, u8 gk)
{
	struct cmd_priv *cmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct cmd_obj *cmd;
	struct set_stakey_parm *param;
	u8	res = _SUCCESS;

	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmd->padapter = adapter;

	param = (struct set_stakey_parm *)rtw_zmalloc(sizeof(struct set_stakey_parm));
	if (param == NULL) {
		rtw_mfree((u8 *) cmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(cmd, param, CMD_SET_STAKEY);

	_rtw_memcpy(param->addr, addr, ETH_ALEN);
	param->algorithm = alg;
	param->keyid = keyid;
	if (!!(alg & _SEC_TYPE_256_))
		_rtw_memcpy(param->key, key, 32);
	else
		_rtw_memcpy(param->key, key, 16);

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	if ((alg == _SMS4_) || (alg == _GCM_SM4_))
		_rtw_memcpy(param->key, key, 32);
#endif

	param->gk = gk;

	res = rtw_enqueue_cmd(cmdpriv, cmd);

exit:
	return res;
}

static int rtw_ap_set_key(_adapter *padapter, u8 *key, u8 alg, int keyid, u8 set_tx)
{
	u8 keylen;
	struct cmd_obj *pcmd;
	struct setkey_parm *psetkeyparm;
	struct cmd_priv	*pcmdpriv = &(adapter_to_dvobj(padapter)->cmdpriv);
	int res = _SUCCESS;

	/* RTW_INFO("%s\n", __FUNCTION__); */

	pcmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	pcmd->padapter = padapter;

	psetkeyparm = (struct setkey_parm *)rtw_zmalloc(sizeof(struct setkey_parm));
	if (psetkeyparm == NULL) {
		rtw_mfree((unsigned char *)pcmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	_rtw_memset(psetkeyparm, 0, sizeof(struct setkey_parm));

	psetkeyparm->keyid = (u8)keyid;
	if (is_wep_enc(alg))
		padapter->securitypriv.key_mask |= BIT(psetkeyparm->keyid);

	psetkeyparm->algorithm = alg;

	psetkeyparm->set_tx = set_tx;

	switch (alg) {
	case _WEP40_:
		keylen = 5;
		break;
	case _WEP104_:
		keylen = 13;
		break;
	case _GCMP_256_:
	case _CCMP_256_:
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	case _SMS4_:
	case _GCM_SM4_:
#endif
		keylen = 32;
		break;
	case _TKIP_:
	case _TKIP_WTMIC_:
	case _AES_:
	case _GCMP_:
	#ifdef CONFIG_IEEE80211W
	case _BIP_CMAC_128_:
	#endif
	default:
		keylen = 16;
	}

	_rtw_memcpy(&(psetkeyparm->key[0]), key, keylen);

	pcmd->cmdcode = CMD_SET_KEY; /*_SetKey_CMD_*/
	pcmd->parmbuf = (u8 *)psetkeyparm;
	pcmd->cmdsz = (sizeof(struct setkey_parm));
	pcmd->rsp = NULL;
	pcmd->rspsz = 0;


	_rtw_init_listhead(&pcmd->list);

	res = rtw_enqueue_cmd(pcmdpriv, pcmd);

exit:

	return res;
}
#endif

u8 rtw_ap_set_pairwise_key(_adapter *padapter, struct sta_info *psta)
{
	return rtw_ap_set_sta_key(padapter
		, psta->phl_sta->mac_addr
		, psta->dot118021XPrivacy
		, psta->dot118021x_UncstKey.skey
		, 0
		, 0
	);
}


int rtw_ap_set_group_key(_adapter *padapter, u8 *key, u8 alg, int keyid)
{
	RTW_INFO("%s\n", __FUNCTION__);

	return rtw_ap_set_key(padapter, key, alg, keyid, 1);
}

int rtw_ap_set_wep_key(_adapter *padapter, u8 *key, u8 keylen, int keyid, u8 set_tx)
{
	u8 alg;

	switch (keylen) {
	case 5:
		alg = _WEP40_;
		break;
	case 13:
		alg = _WEP104_;
		break;
	default:
		alg = _NO_PRIVACY_;
	}

	RTW_INFO("%s\n", __FUNCTION__);

	return rtw_ap_set_key(padapter, key, alg, keyid, set_tx);
}

#if 0//CONFIG_VW_REFINE
u8 rtw_ap_bmc_frames_hdl(_adapter *padapter)
{
#define HIQ_XMIT_COUNTS (6)
	struct sta_info *psta_bmc;
	_list	*xmitframe_plist, *xmitframe_phead;
	struct xmit_frame *pxmitframe = NULL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_priv  *pstapriv = &padapter->stapriv;
	bool update_tim = _FALSE;


	if (padapter->registrypriv.wifi_spec != 1)
		return H2C_SUCCESS;


	psta_bmc = rtw_get_bcmc_stainfo(padapter);
	if (!psta_bmc)
		return H2C_SUCCESS;


	_rtw_spinlock_bh(&pxmitpriv->lock);

	if ((rtw_tim_map_is_set(padapter, pstapriv->tim_bitmap, 0)) && (psta_bmc->sleepq_len > 0)) {
		int tx_counts = 0;

		_update_beacon(padapter, _TIM_IE_, NULL, _FALSE, 0, "update TIM with TIB=1");

		RTW_INFO("sleepq_len of bmc_sta = %d\n", psta_bmc->sleepq_len);

		xmitframe_phead = get_list_head(&psta_bmc->sleep_q);
		xmitframe_plist = get_next(xmitframe_phead);

		while ((rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist)) == _FALSE) {
			pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);

			xmitframe_plist = get_next(xmitframe_plist);

			rtw_list_delete(&pxmitframe->list);

			psta_bmc->sleepq_len--;
			tx_counts++;

			if (psta_bmc->sleepq_len > 0)
				pxmitframe->attrib.mdata = 1;
			else
				pxmitframe->attrib.mdata = 0;

			if (tx_counts == HIQ_XMIT_COUNTS)
				pxmitframe->attrib.mdata = 0;

			pxmitframe->attrib.triggered = 1;

			if (xmitframe_hiq_filter(pxmitframe) == _TRUE)
				pxmitframe->attrib.qsel = rtw_hal_get_qsel(padapter,QSLT_HIGH_ID);/*HIQ*/

			rtw_intf_xmitframe_enqueue(padapter, pxmitframe);

			if (tx_counts == HIQ_XMIT_COUNTS)
				break;

		}

	} else {
		if (psta_bmc->sleepq_len == 0) {

			/*RTW_INFO("sleepq_len of bmc_sta = %d\n", psta_bmc->sleepq_len);*/

			if (rtw_tim_map_is_set(padapter, pstapriv->tim_bitmap, 0))
				update_tim = _TRUE;

			rtw_tim_map_clear(padapter, pstapriv->tim_bitmap, 0);
			rtw_tim_map_clear(padapter, pstapriv->sta_dz_bitmap, 0);

			if (update_tim == _TRUE) {
				RTW_INFO("clear TIB\n");
				_update_beacon(padapter, _TIM_IE_, NULL, _TRUE, 0, "bmc sleepq and HIQ empty");
			}
		}
	}

	_rtw_spinunlock_bh(&pxmitpriv->lock);

#if 0
	/* HIQ Check */
	rtw_hal_get_hwreg(padapter, HW_VAR_CHK_HI_QUEUE_EMPTY, &empty);

	while (_FALSE == empty && rtw_get_passing_time_ms(start) < 3000) {
		rtw_msleep_os(100);
		rtw_hal_get_hwreg(padapter, HW_VAR_CHK_HI_QUEUE_EMPTY, &empty);
	}


	printk("check if hiq empty=%d\n", empty);
#endif

	return H2C_SUCCESS;
}
#endif

#ifdef CONFIG_NATIVEAP_MLME

static void associated_stainfo_update(_adapter *padapter, struct sta_info *psta, u32 sta_info_type)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	//RTW_INFO("%s: "MAC_FMT", updated_type=0x%x\n", __func__, MAC_ARG(psta->phl_sta->mac_addr), sta_info_type);
#ifdef CONFIG_80211N_HT
	if (sta_info_type & STA_INFO_UPDATE_BW) {

		if ((psta->flags & WLAN_STA_HT) && !psta->ht_20mhz_set) {
			if (pmlmepriv->sw_to_20mhz) {
				psta->phl_sta->chandef.bw = CHANNEL_WIDTH_20;
				/*psta->htpriv.ch_offset = CHAN_OFFSET_NO_EXT;*/
				psta->htpriv.sgi_40m = _FALSE;
				rtw_phl_cmd_change_stainfo(padapter->dvobj->phl, psta->phl_sta,
							STA_CHG_BW, NULL, 0, PHL_CMD_DIRECTLY, 0);
			} else {
				/*TODO: Switch back to 40MHZ?80MHZ*/
			}
		}
	}
#endif /* CONFIG_80211N_HT */
	/*
		if (sta_info_type & STA_INFO_UPDATE_RATE) {

		}
	*/

	if (sta_info_type & STA_INFO_UPDATE_PROTECTION_MODE) {
#ifdef RTW_WKARD_REDUCE_CONNECT_LOG
		if(!padapter->vw_enable)
#endif
		{
			if (VCS_update(padapter, psta)) {
				rtw_phl_cmd_change_stainfo(padapter->dvobj->phl, psta->phl_sta,
							STA_CHG_VCS, NULL, 0, PHL_CMD_DIRECTLY, 0);
			}
		}
	}

	/*
		if (sta_info_type & STA_INFO_UPDATE_CAP) {

		}

		if (sta_info_type & STA_INFO_UPDATE_HT_CAP) {

		}

		if (sta_info_type & STA_INFO_UPDATE_VHT_CAP) {

		}
	*/

}

static void update_bcn_ext_capab_ie(_adapter *padapter)
{
	sint ie_len = 0;
	unsigned char	*pbuf;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork = &(pmlmeinfo->network);
	u8 *ie = pnetwork->IEs;
	u8 null_extcap_data[8] = {0};

	pbuf = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _EXT_CAP_IE_, &ie_len, (pnetwork->IELength - _BEACON_IE_OFFSET_));
	if (pbuf && ie_len > 0)
		rtw_remove_bcn_ie(padapter, pnetwork, _EXT_CAP_IE_);

	if ((pmlmepriv->ext_capab_ie_len > 0) &&
	    (_rtw_memcmp(pmlmepriv->ext_capab_ie_data, null_extcap_data, sizeof(null_extcap_data)) == _FALSE))
		rtw_add_bcn_ie(padapter, pnetwork, _EXT_CAP_IE_, pmlmepriv->ext_capab_ie_data, pmlmepriv->ext_capab_ie_len);

}

static void update_bcn_erpinfo_ie(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork = &(pmlmeinfo->network);
	unsigned char *p, *ie = pnetwork->IEs;
	u32 len = 0;

	RTW_INFO("%s, ERP_enable=%d\n", __FUNCTION__, pmlmeinfo->ERP_enable);

	if (!pmlmeinfo->ERP_enable)
		return;

	/* parsing ERP_IE */
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _ERPINFO_IE_, &len, (pnetwork->IELength - _BEACON_IE_OFFSET_));
	if (p && len > 0) {
		PNDIS_802_11_VARIABLE_IEs pIE = (PNDIS_802_11_VARIABLE_IEs)p;

		if (pmlmepriv->num_sta_non_erp == 1)
			pIE->data[0] |= RTW_ERP_INFO_NON_ERP_PRESENT | RTW_ERP_INFO_USE_PROTECTION;
		else
			pIE->data[0] &= ~(RTW_ERP_INFO_NON_ERP_PRESENT | RTW_ERP_INFO_USE_PROTECTION);

		if (pmlmepriv->num_sta_no_short_preamble > 0)
			pIE->data[0] |= RTW_ERP_INFO_BARKER_PREAMBLE_MODE;
		else
			pIE->data[0] &= ~(RTW_ERP_INFO_BARKER_PREAMBLE_MODE);

		ERP_IE_handler(padapter, pIE);
	}

}

static void update_bcn_htcap_ie(_adapter *padapter)
{
#ifdef CONFIG_80211N_HT
	u8 rx_nss = 0;
	int i;
	u8 *p;
	uint ie_len = 0;
	struct rtw_ieee80211_ht_cap *pht_cap = NULL;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork_mlmeext = &(pmlmeinfo->network);

	if (pmlmepriv->htpriv.ht_option == _FALSE)
		return;

	p = rtw_get_ie(pnetwork_mlmeext->IEs + _BEACON_IE_OFFSET_, _HT_CAPABILITY_IE_, &ie_len, (pnetwork_mlmeext->IELength - _BEACON_IE_OFFSET_));
	if (!p || ie_len <= 0)
		return;

	pht_cap = (struct rtw_ieee80211_ht_cap *)(p + 2);

	/* fill default supported_mcs_set */
	_rtw_memcpy(pht_cap->supp_mcs_set, padapter->mlmeextpriv.default_supported_mcs_set, 16);

	rx_nss = GET_HAL_RX_NSS(adapter_to_dvobj(padapter));

	/* RX MCS Bitmask */
	switch (rx_nss) {
		case 1:
			set_mcs_rate_by_mask(HT_CAP_ELE_RX_MCS_MAP(pht_cap), MCS_RATE_1R);
			break;
		case 2:
			set_mcs_rate_by_mask(HT_CAP_ELE_RX_MCS_MAP(pht_cap), MCS_RATE_2R);
			break;
		case 3:
			set_mcs_rate_by_mask(HT_CAP_ELE_RX_MCS_MAP(pht_cap), MCS_RATE_3R);
			break;
		case 4:
			set_mcs_rate_by_mask(HT_CAP_ELE_RX_MCS_MAP(pht_cap), MCS_RATE_4R);
			break;
		default:
			RTW_WARN("rf_type:%d or rx_nss:%u is not expected\n",
				GET_HAL_RFPATH(adapter_to_dvobj(padapter)), rx_nss);
	}

#endif
	RTW_INFO("%s\n", __FUNCTION__);

}

static void update_bcn_hecap_ie(_adapter *padapter)
{
#ifdef CONFIG_80211AX_HE
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork = &(pmlmeinfo->network);
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	unsigned char *p = pnetwork->IEs;
	unsigned char *he_mac_cap;
//	unsigned char he_phy_cap;
	u32 len = 0;

	RTW_INFO("%s\n", __FUNCTION__);
	if (pmlmepriv->hepriv.he_option == _FALSE)
		return;

	if (REGSTY_IS_11AX_AUTO(pregistrypriv)) {
		rtw_he_ies_detach(padapter, pnetwork);
		rtw_he_ies_attach(padapter, pnetwork);
	}

	while (1) {
		p = rtw_get_ie(p + _BEACON_IE_OFFSET_, _HE_CAPABILITY_IE_, &len, (pnetwork->IELength - _BEACON_IE_OFFSET_));
		if (p == NULL) {
			RTW_INFO("Cannot find HE Capability IE\n");
			return;
		}
		if (*(p + 2) == WLAN_EID_EXTENSION_HE_CAPABILITY) { break; }
	}

	he_mac_cap = p + 3;
	if (padapter->registrypriv.twt_enable)
		SET_HE_MAC_CAP_TWT_RESPONDER_SUPPORT(he_mac_cap, 1);
	else
		SET_HE_MAC_CAP_TWT_RESPONDER_SUPPORT(he_mac_cap, 0);

//	he_phy_cap = he_mac_cap + HE_CAP_ELE_MAC_CAP_LEN;
#endif
}

static void update_bcn_htinfo_ie(_adapter *padapter)
{
#ifdef CONFIG_80211N_HT
	/*
	u8 beacon_updated = _FALSE;
	u32 sta_info_update_type = STA_INFO_UPDATE_NONE;
	*/
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork = &(pmlmeinfo->network);
	unsigned char *p, *ie = pnetwork->IEs;
	u32 len = 0;

	if (pmlmepriv->htpriv.ht_option == _FALSE)
		return;

	if (pmlmeinfo->HT_info_enable != 1)
		return;


	RTW_INFO("%s current operation mode=0x%X\n",
		 __FUNCTION__, pmlmepriv->ht_op_mode);

	RTW_INFO("num_sta_40mhz_intolerant(%d), 20mhz_width_req(%d), intolerant_ch_rpt(%d)\n",
		pmlmepriv->num_sta_40mhz_intolerant, pmlmepriv->ht_20mhz_width_req, pmlmepriv->ht_intolerant_ch_reported);

	/*parsing HT_INFO_IE, currently only update ht_op_mode - pht_info->infos[1] & pht_info->infos[2] for wifi logo test*/
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _HT_ADD_INFO_IE_, &len, (pnetwork->IELength - _BEACON_IE_OFFSET_));
	if (p && len > 0) {
		struct HT_info_element *pht_info = NULL;

		pht_info = (struct HT_info_element *)(p + 2);

		/* for STA Channel Width/Secondary Channel Offset*/
		if (pmlmepriv->htpriv.bss_coexist) {
			if ((pmlmepriv->sw_to_20mhz == 0) && (pmlmeext->cur_channel <= 14)) {
				if ((pmlmepriv->num_sta_40mhz_intolerant > 0) || (pmlmepriv->ht_20mhz_width_req == _TRUE)
				    || (pmlmepriv->ht_intolerant_ch_reported == _TRUE)) {
					SET_HT_OP_ELE_2ND_CHL_OFFSET(pht_info, 0);
					SET_HT_OP_ELE_STA_CHL_WIDTH(pht_info, 0);

					pmlmepriv->sw_to_20mhz = 1;
					/*
					sta_info_update_type |= STA_INFO_UPDATE_BW;
					beacon_updated = _TRUE;
					*/

					RTW_INFO("%s:switching to 20Mhz\n", __FUNCTION__);

					/*TODO : cur_bwmode/cur_ch_offset switches to 20Mhz*/
				}
			} else {

				if ((pmlmepriv->num_sta_40mhz_intolerant == 0) && (pmlmepriv->ht_20mhz_width_req == _FALSE)
				    && (pmlmepriv->ht_intolerant_ch_reported == _FALSE)) {

					if (pmlmeext->cur_bwmode >= CHANNEL_WIDTH_40) {

						SET_HT_OP_ELE_STA_CHL_WIDTH(pht_info, 1);

						SET_HT_OP_ELE_2ND_CHL_OFFSET(pht_info,
							(pmlmeext->cur_ch_offset == CHAN_OFFSET_UPPER) ?
							HT_INFO_HT_PARAM_SECONDARY_CHNL_ABOVE : HT_INFO_HT_PARAM_SECONDARY_CHNL_BELOW);

						pmlmepriv->sw_to_20mhz = 0;
						/*
						sta_info_update_type |= STA_INFO_UPDATE_BW;
						beacon_updated = _TRUE;
						*/

						RTW_INFO("%s:switching back to 40Mhz\n", __FUNCTION__);
					}
				}
			}
		}

		/* to update  ht_op_mode*/
#ifdef PLATFORM_ECOS
		put_unaligned(cpu_to_le16(pmlmepriv->ht_op_mode), (u16 *)(pht_info->infos + 1));
#else
		*(u16 *)(pht_info->infos + 1) = cpu_to_le16(pmlmepriv->ht_op_mode);
#endif

#ifdef CONFIG_IOCTL_CFG80211
		if (GET_CFG80211_REPORT_MGMT(adapter_wdev_data(padapter), IEEE80211_STYPE_PROBE_REQ) == _TRUE)
			memcpy(pmlmepriv->htpriv.ht_op, p+2, HT_OP_IE_LEN);
#endif

	}

	/*associated_clients_update(padapter, beacon_updated, sta_info_update_type);*/
#endif /* CONFIG_80211N_HT */
}

static void update_bcn_rsn_ie(_adapter *padapter)
{
	RTW_INFO("%s\n", __FUNCTION__);

}

static void update_bcn_wpa_ie(_adapter *padapter)
{
	RTW_INFO("%s\n", __FUNCTION__);

}

static void update_bcn_wmm_ie(_adapter *padapter)
{
	RTW_INFO("%s\n", __FUNCTION__);

}

static void update_bcn_wps_ie(_adapter *padapter)
{
	u8 *pwps_ie = NULL, *pwps_ie_src, *premainder_ie, *pbackup_remainder_ie = NULL;
	uint wps_ielen = 0, wps_offset, remainder_ielen;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork = &(pmlmeinfo->network);
	unsigned char *ie = pnetwork->IEs;
	u32 ielen = pnetwork->IELength;


	RTW_INFO("%s\n", __FUNCTION__);

	pwps_ie = rtw_get_wps_ie(ie + _FIXED_IE_LENGTH_, ielen - _FIXED_IE_LENGTH_, NULL, &wps_ielen);

	if (pwps_ie == NULL || wps_ielen == 0)
		return;

	pwps_ie_src = pmlmepriv->wps_beacon_ie;
	if (pwps_ie_src == NULL)
		return;

	wps_offset = (uint)(pwps_ie - ie);

	premainder_ie = pwps_ie + wps_ielen;

	remainder_ielen = ielen - wps_offset - wps_ielen;

	if (remainder_ielen > 0) {
		pbackup_remainder_ie = rtw_malloc(remainder_ielen);
		if (pbackup_remainder_ie)
			_rtw_memcpy(pbackup_remainder_ie, premainder_ie, remainder_ielen);
	}

	wps_ielen = (uint)pwps_ie_src[1];/* to get ie data len */
	if ((wps_offset + wps_ielen + 2 + remainder_ielen) <= MAX_IE_SZ) {
		_rtw_memcpy(pwps_ie, pwps_ie_src, wps_ielen + 2);
		pwps_ie += (wps_ielen + 2);

		if (pbackup_remainder_ie)
			_rtw_memcpy(pwps_ie, pbackup_remainder_ie, remainder_ielen);

		/* update IELength */
		pnetwork->IELength = wps_offset + (wps_ielen + 2) + remainder_ielen;
	}

	if (pbackup_remainder_ie)
		rtw_mfree(pbackup_remainder_ie, remainder_ielen);

	/* deal with the case without set_tx_beacon_cmd() in update_beacon() */
#if defined(CONFIG_INTERRUPT_BASED_TXBCN) || defined(CONFIG_PCI_HCI)
	if (MLME_IN_AP_STATE(pmlmeinfo)) {
		u8 sr = 0;
		rtw_get_wps_attr_content(pwps_ie_src,  wps_ielen, WPS_ATTR_SELECTED_REGISTRAR, (u8 *)(&sr), NULL);

		if (sr) {
			set_fwstate(pmlmepriv, WIFI_UNDER_WPS);
			RTW_INFO("%s, set WIFI_UNDER_WPS\n", __func__);
		} else {
			clr_fwstate(pmlmepriv, WIFI_UNDER_WPS);
			RTW_INFO("%s, clr WIFI_UNDER_WPS\n", __func__);
		}
	}
#endif
}

static void update_bcn_p2p_ie(_adapter *padapter)
{

}

static void update_bcn_vendor_spec_ie(_adapter *padapter, u8 *oui)
{
	RTW_INFO("%s\n", __FUNCTION__);

	if (_rtw_memcmp(RTW_WPA_OUI, oui, 4))
		update_bcn_wpa_ie(padapter);
	else if (_rtw_memcmp(WMM_OUI, oui, 4))
		update_bcn_wmm_ie(padapter);
	else if (_rtw_memcmp(WPS_OUI, oui, 4))
		update_bcn_wps_ie(padapter);
	else if (_rtw_memcmp(P2P_OUI, oui, 4))
		update_bcn_p2p_ie(padapter);
	else
		RTW_INFO("unknown OUI type!\n");
}

static void update_bcn_vhtcap_ie(_adapter *padapter)
{
#ifdef CONFIG_80211AC_VHT
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork_mlmeext = &(pmlmeinfo->network);

	RTW_INFO("%s\n", __FUNCTION__);
	if (pmlmepriv->vhtpriv.vht_option == _FALSE)
		return;
	if (REGSTY_IS_11AC_AUTO(pregistrypriv)) {
		rtw_vht_ies_detach(padapter, pnetwork_mlmeext);
		rtw_vht_ies_attach(padapter, pnetwork_mlmeext);
	}
#endif
}


static void update_bcn_int(_adapter *padapter, u8 *bcnint)
{
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork_mlmeext = &(pmlmeinfo->network);
	unsigned char *pie = pnetwork_mlmeext->IEs;

	_rtw_memcpy(pie + 8, bcnint, 2);
}

void _update_beacon(_adapter *padapter, u16 ie_id, u8 *oui, u8 tx, u8 flags, const char *tag)
{
	struct mlme_priv *pmlmepriv;
	struct mlme_ext_priv *pmlmeext;
	bool updated = 1; /* treat as upadated by default */
	bool func_off = 0;

	if (!padapter)
		return;

	func_off = padapter->registrypriv.wifi_mib.func_off;
	pmlmepriv = &(padapter->mlmepriv);
	pmlmeext = &(padapter->mlmeextpriv);

	if (pmlmeext->bstart_bss == _FALSE)
		return;

	_rtw_spinlock_bh(&pmlmepriv->bcn_update_lock);

	switch (ie_id) {
	case _TIM_IE_:
		update_BCNTIM(padapter);
		break;

	case _ERPINFO_IE_:
		update_bcn_erpinfo_ie(padapter);
		break;

	case _HT_CAPABILITY_IE_:
		update_bcn_htcap_ie(padapter);
		break;
	case _HE_CAPABILITY_IE_:
		update_bcn_hecap_ie(padapter);
		break;
	case _RSN_IE_2_:
		update_bcn_rsn_ie(padapter);
		break;

	case _HT_ADD_INFO_IE_:
		update_bcn_htinfo_ie(padapter);
		break;

	case _EXT_CAP_IE_:
		update_bcn_ext_capab_ie(padapter);
		break;

#ifdef CONFIG_RTW_MESH
	case WLAN_EID_MESH_CONFIG:
		updated = rtw_mesh_update_bss_peering_status(padapter, &(pmlmeext->mlmext_info.network));
		updated |= rtw_mesh_update_bss_formation_info(padapter, &(pmlmeext->mlmext_info.network));
		updated |= rtw_mesh_update_bss_forwarding_state(padapter, &(pmlmeext->mlmext_info.network));
		break;
#endif
	case WLAN_EID_VHT_CAPABILITY:
		update_bcn_vhtcap_ie(padapter);
		break;

	case _VENDOR_SPECIFIC_IE_:
		update_bcn_vendor_spec_ie(padapter, oui);
		break;

	case 0xFE:
		update_bcn_int(padapter, oui);
		break;
	case 0xFFF:
	default:
		break;
	}

	if (updated)
		pmlmepriv->update_bcn = _TRUE;

	_rtw_spinunlock_bh(&pmlmepriv->bcn_update_lock);

#ifndef CONFIG_INTERRUPT_BASED_TXBCN
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI) || defined(CONFIG_PCI_BCN_POLLING)
	if (tx && updated && !func_off) {
		/* send_beacon(padapter); */ /* send_beacon must execute on TSR level */
		if (0)
			RTW_INFO(FUNC_ADPT_FMT" ie_id:%u - %s\n", FUNC_ADPT_ARG(padapter), ie_id, tag);
		if(flags == RTW_CMDF_WAIT_ACK)
			set_tx_beacon_cmd(padapter, RTW_CMDF_WAIT_ACK);
		else
			set_tx_beacon_cmd(padapter, 0);
	}
#else
	{
		/* PCI will issue beacon when BCN interrupt occurs.		 */
	}
#endif
#endif /* !CONFIG_INTERRUPT_BASED_TXBCN */
}

#ifdef CONFIG_80211N_HT

void rtw_process_public_act_bsscoex(_adapter *padapter, u8 *pframe, uint frame_len)
{
	struct sta_info *psta;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 beacon_updated = _FALSE;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u8 *frame_body = pframe + sizeof(struct rtw_ieee80211_hdr_3addr);
	uint frame_body_len = frame_len - sizeof(struct rtw_ieee80211_hdr_3addr);
	u8 category, action;

	psta = rtw_get_stainfo(pstapriv, get_addr2_ptr(pframe));
	if (psta == NULL)
		return;

	RTW_INFO("[%s]\n", __FUNCTION__);
	if(!pmlmepriv->htpriv.bss_coexist)
		return;

	category = frame_body[0];
	action = frame_body[1];

	if (frame_body_len > 0) {
		if ((frame_body[2] == EID_BSSCoexistence) && (frame_body[3] > 0)) {
			u8 ie_data = frame_body[4];

			if (ie_data & RTW_WLAN_20_40_BSS_COEX_40MHZ_INTOL) {
				if (psta->ht_40mhz_intolerant == 0) {
					psta->ht_40mhz_intolerant = 1;
					pmlmepriv->num_sta_40mhz_intolerant++;
					beacon_updated = _TRUE;
				}
			} else if (ie_data & RTW_WLAN_20_40_BSS_COEX_20MHZ_WIDTH_REQ)	{
				if (pmlmepriv->ht_20mhz_width_req == _FALSE) {
					pmlmepriv->ht_20mhz_width_req = _TRUE;
					beacon_updated = _TRUE;
				}
			} else
				beacon_updated = _FALSE;
		}
	}

	if (frame_body_len > 8) {
		/* if EID_BSSIntolerantChlReport ie exists */
		if ((frame_body[5] == EID_BSSIntolerantChlReport) && (frame_body[6] > 0)) {
			/*todo:*/
			if (pmlmepriv->ht_intolerant_ch_reported == _FALSE) {
				pmlmepriv->ht_intolerant_ch_reported = _TRUE;
				beacon_updated = _TRUE;
			}
		}
	}

	if (beacon_updated) {

		update_beacon(padapter, _HT_ADD_INFO_IE_, NULL, _TRUE, 0);

		associated_clients_update(padapter, _TRUE, STA_INFO_UPDATE_BW);
	}



}

void rtw_process_ht_action_smps(_adapter *padapter, u8 *ta, u8 ctrl_field)
{
	u8 e_field, m_field;
	struct sta_info *psta;
	struct sta_priv *pstapriv = &padapter->stapriv;

	psta = rtw_get_stainfo(pstapriv, ta);
	if (psta == NULL)
		return;

	e_field = (ctrl_field & BIT(0)) ? 1 : 0; /*SM Power Save Enabled*/
	m_field = (ctrl_field & BIT(1)) ? 1 : 0; /*SM Mode, 0:static SMPS, 1:dynamic SMPS*/

	if (e_field) {
		if (m_field) { /*mode*/
			psta->htpriv.smps_cap = WLAN_HT_CAP_SM_PS_DYNAMIC;
		}
		else
			psta->htpriv.smps_cap = WLAN_HT_CAP_SM_PS_STATIC;
	} else {
		/*disable*/
		psta->htpriv.smps_cap = WLAN_HT_CAP_SM_PS_DISABLED;
	}

	rtw_ssmps_wk_cmd(padapter, psta, psta->htpriv.smps_cap, 1);
}

/*
op_mode
Set to 0 (HT pure) under the followign conditions
	- all STAs in the BSS are 20/40 MHz HT in 20/40 MHz BSS or
	- all STAs in the BSS are 20 MHz HT in 20 MHz BSS
Set to 1 (HT non-member protection) if there may be non-HT STAs
	in both the primary and the secondary channel
Set to 2 if only HT STAs are associated in BSS,
	however and at least one 20 MHz HT STA is associated
Set to 3 (HT mixed mode) when one or more non-HT STAs are associated
	(currently non-GF HT station is considered as non-HT STA also)
*/
int rtw_ht_operation_update(_adapter *padapter)
{
	u16 cur_op_mode, new_op_mode;
	int op_mode_changes = 0;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct ht_priv	*phtpriv_ap = &pmlmepriv->htpriv;
	u8 ht_protection = 0;
	u16 op_mode_prev;

	if (pmlmepriv->htpriv.ht_option == _FALSE)
		return 0;

	ht_protection = ATOMIC_READ(&pmlmepriv->olbc_ap) | ATOMIC_READ(&pmlmepriv->olbc_sta);
	if(pmlmepriv->olbc_count && pmlmepriv->olbc_count < 60)
		ht_protection = 1;

	op_mode_prev = pmlmepriv->ht_op_mode;

	/*if (!iface->conf->ieee80211n || iface->conf->ht_op_mode_fixed)
		return 0;*/

	if (!(pmlmepriv->ht_op_mode & HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT)
	    && pmlmepriv->num_sta_ht_no_gf) {
		pmlmepriv->ht_op_mode |=
			HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT;
		op_mode_changes++;
	} else if ((pmlmepriv->ht_op_mode &
		    HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT) &&
		   pmlmepriv->num_sta_ht_no_gf == 0) {
		pmlmepriv->ht_op_mode &=
			~HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT;
		op_mode_changes++;
	}

	if (!(pmlmepriv->ht_op_mode & HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT) &&
	    (pmlmepriv->num_sta_no_ht || ht_protection)) {
		pmlmepriv->ht_op_mode |= HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT;
		op_mode_changes++;
	} else if ((pmlmepriv->ht_op_mode &
		    HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT) &&
		   (pmlmepriv->num_sta_no_ht == 0 && !ht_protection)) {
		pmlmepriv->ht_op_mode &=
			~HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT;
		op_mode_changes++;
	}

	/* Note: currently we switch to the MIXED op mode if HT non-greenfield
	 * station is associated. Probably it's a theoretical case, since
	 * it looks like all known HT STAs support greenfield.
	 */
	new_op_mode = 0;
	if (pmlmepriv->num_sta_no_ht /*||
	    (pmlmepriv->ht_op_mode & HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT)*/)
		new_op_mode = OP_MODE_MIXED;
	else if ((phtpriv_ap->ht_cap.cap_info & cpu_to_le16(IEEE80211_HT_CAP_SUP_WIDTH))
		 && pmlmepriv->num_sta_ht_20mhz)
		new_op_mode = OP_MODE_20MHZ_HT_STA_ASSOCED;
	else if (ht_protection)
		new_op_mode = OP_MODE_MAY_BE_LEGACY_STAS;
	else
		new_op_mode = OP_MODE_PURE;

	cur_op_mode = pmlmepriv->ht_op_mode & HT_INFO_OPERATION_MODE_OP_MODE_MASK;
	if (cur_op_mode != new_op_mode) {
		pmlmepriv->ht_op_mode &= ~HT_INFO_OPERATION_MODE_OP_MODE_MASK;
		pmlmepriv->ht_op_mode |= new_op_mode;
		op_mode_changes++;
	}

	if (pmlmepriv->ht_op_mode != op_mode_prev)
		RTW_INFO("%s new operation mode=0x%X, original is 0x%X, changes=%d\n",
		 	__FUNCTION__, pmlmepriv->ht_op_mode,
		 	op_mode_prev, op_mode_changes);

	return op_mode_changes;

}

#endif /* CONFIG_80211N_HT */

void associated_clients_update(_adapter *padapter, u8 updated, u32 sta_info_type)
{
	/* update associcated stations cap. */
	if (updated == _TRUE) {
		_list	*phead, *plist;
		struct sta_info *psta = NULL;
		struct sta_priv *pstapriv = &padapter->stapriv;

		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

		phead = &pstapriv->asoc_list;
		plist = get_next(phead);

		/* check asoc_queue */
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);

			plist = get_next(plist);

			associated_stainfo_update(padapter, psta, sta_info_type);
		}

		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	}

}

/* called > TSR LEVEL for USB or SDIO Interface*/
void bss_cap_update_on_sta_join(_adapter *padapter, struct sta_info *psta)
{
	u8 beacon_updated = _FALSE;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);


#if 0
	if (!(psta->capability & WLAN_CAPABILITY_SHORT_PREAMBLE) &&
	    !psta->no_short_preamble_set) {
		psta->no_short_preamble_set = 1;
		pmlmepriv->num_sta_no_short_preamble++;
		if ((pmlmeext->cur_wireless_mode > WIRELESS_11B) &&
		    (pmlmepriv->num_sta_no_short_preamble == 1))
			ieee802_11_set_beacons(hapd->iface);
	}
#endif


	if (!(psta->flags & WLAN_STA_SHORT_PREAMBLE)) {
		if (!psta->no_short_preamble_set) {
			psta->no_short_preamble_set = 1;

			pmlmepriv->num_sta_no_short_preamble++;

			if ((pmlmeext->cur_wireless_mode > WLAN_MD_11B) &&
			    (pmlmepriv->num_sta_no_short_preamble == 1))
				beacon_updated = _TRUE;
		}
	} else {
		if (psta->no_short_preamble_set) {
			psta->no_short_preamble_set = 0;

			pmlmepriv->num_sta_no_short_preamble--;

			if ((pmlmeext->cur_wireless_mode > WLAN_MD_11B) &&
			    (pmlmepriv->num_sta_no_short_preamble == 0))
				beacon_updated = _TRUE;
		}
	}

#if 0
	if (psta->flags & WLAN_STA_NONERP && !psta->nonerp_set) {
		psta->nonerp_set = 1;
		pmlmepriv->num_sta_non_erp++;
		if (pmlmepriv->num_sta_non_erp == 1)
			ieee802_11_set_beacons(hapd->iface);
	}
#endif

	if (psta->flags & WLAN_STA_NONERP) {
		if (!psta->nonerp_set) {
			psta->nonerp_set = 1;

			pmlmepriv->num_sta_non_erp++;

			if (pmlmepriv->num_sta_non_erp == 1) {
				beacon_updated = _TRUE;
				update_beacon(padapter, _ERPINFO_IE_, NULL, _FALSE, 0);
			}
		}

	} else {
		if (psta->nonerp_set) {
			psta->nonerp_set = 0;

			pmlmepriv->num_sta_non_erp--;

			if (pmlmepriv->num_sta_non_erp == 0) {
				beacon_updated = _TRUE;
				update_beacon(padapter, _ERPINFO_IE_, NULL, _FALSE, 0);
			}
		}

	}


#if 0
	if (!(psta->capability & WLAN_CAPABILITY_SHORT_SLOT) &&
	    !psta->no_short_slot_time_set) {
		psta->no_short_slot_time_set = 1;
		pmlmepriv->num_sta_no_short_slot_time++;
		if ((pmlmeext->cur_wireless_mode > WIRELESS_11B) &&
		    (pmlmepriv->num_sta_no_short_slot_time == 1))
			ieee802_11_set_beacons(hapd->iface);
	}
#endif

	if (!(psta->capability & WLAN_CAPABILITY_SHORT_SLOT)) {
		if (!psta->no_short_slot_time_set) {
			psta->no_short_slot_time_set = 1;

			pmlmepriv->num_sta_no_short_slot_time++;

			if ((pmlmeext->cur_wireless_mode > WLAN_MD_11B) &&
			    (pmlmepriv->num_sta_no_short_slot_time == 1))
				beacon_updated = _TRUE;
		}
	} else {
		if (psta->no_short_slot_time_set) {
			psta->no_short_slot_time_set = 0;

			pmlmepriv->num_sta_no_short_slot_time--;

			if ((pmlmeext->cur_wireless_mode > WLAN_MD_11B) &&
			    (pmlmepriv->num_sta_no_short_slot_time == 0))
				beacon_updated = _TRUE;
		}
	}

#ifdef CONFIG_80211N_HT
	if(padapter->registrypriv.ht_enable &&
		is_supported_ht(padapter->registrypriv.wireless_mode)) {
		if (psta->flags & WLAN_STA_HT) {
			u16 ht_capab = le16_to_cpu(psta->htpriv.ht_cap.cap_info);

			RTW_INFO("HT: STA " MAC_FMT " HT Capabilities Info: 0x%04x\n",
				MAC_ARG(psta->phl_sta->mac_addr), ht_capab);

			if (psta->no_ht_set) {
				psta->no_ht_set = 0;
				pmlmepriv->num_sta_no_ht--;
			}

			if ((ht_capab & IEEE80211_HT_CAP_GRN_FLD) == 0) {
				if (!psta->no_ht_gf_set) {
					psta->no_ht_gf_set = 1;
					pmlmepriv->num_sta_ht_no_gf++;
				}
				RTW_INFO("%s STA " MAC_FMT " - no "
					 "greenfield, num of non-gf stations %d\n",
					 __FUNCTION__, MAC_ARG(psta->phl_sta->mac_addr),
					 pmlmepriv->num_sta_ht_no_gf);
			}

			if ((ht_capab & IEEE80211_HT_CAP_SUP_WIDTH) == 0) {
				if (!psta->ht_20mhz_set) {
					psta->ht_20mhz_set = 1;
					pmlmepriv->num_sta_ht_20mhz++;
				}
				RTW_INFO("%s STA " MAC_FMT " - 20 MHz HT, "
					 "num of 20MHz HT STAs %d\n",
					 __FUNCTION__, MAC_ARG(psta->phl_sta->mac_addr),
					 pmlmepriv->num_sta_ht_20mhz);
			}

			if (((ht_capab & RTW_IEEE80211_HT_CAP_40MHZ_INTOLERANT) != 0) &&
				(psta->ht_40mhz_intolerant == 0)) {
				psta->ht_40mhz_intolerant = 1;
				pmlmepriv->num_sta_40mhz_intolerant++;
				RTW_INFO("%s STA " MAC_FMT " - 40MHZ_INTOLERANT, ",
					   __FUNCTION__, MAC_ARG(psta->phl_sta->mac_addr));
			}

		} else {
			if (!psta->no_ht_set) {
				psta->no_ht_set = 1;
				pmlmepriv->num_sta_no_ht++;
			}
			if (pmlmepriv->htpriv.ht_option == _TRUE) {
				RTW_INFO("%s STA " MAC_FMT
					 " - no HT, num of non-HT stations %d\n",
					 __FUNCTION__, MAC_ARG(psta->phl_sta->mac_addr),
					 pmlmepriv->num_sta_no_ht);
			}
		}

		if (rtw_ht_operation_update(padapter) > 0) {
			update_beacon(padapter, _HT_CAPABILITY_IE_, NULL, _FALSE, 0);
			update_beacon(padapter, _HT_ADD_INFO_IE_, NULL, _FALSE, 0);
			beacon_updated = _TRUE;
		}
	}
#endif /* CONFIG_80211N_HT */

#ifdef CONFIG_RTW_MESH
	if (MLME_IS_MESH(padapter)) {
		struct sta_priv *pstapriv = &padapter->stapriv;

		update_beacon(padapter, WLAN_EID_MESH_CONFIG, NULL, _FALSE, 0);
		if (pstapriv->asoc_list_cnt == 1)
			_set_timer(&padapter->mesh_atlm_param_req_timer, 0);
		beacon_updated = _TRUE;
	}
#endif

	if (beacon_updated)
		update_beacon(padapter, 0xFFF, NULL, _TRUE, 0);

	/* update associcated stations cap. */
	associated_clients_update(padapter,  beacon_updated, STA_INFO_UPDATE_ALL);

	RTW_INFO("%s, updated=%d\n", __func__, beacon_updated);

}

u8 bss_cap_update_on_sta_leave(_adapter *padapter, struct sta_info *psta)
{
	u8 beacon_updated = _FALSE;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);

	if (!psta)
		return beacon_updated;

	if (rtw_tim_map_is_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid)) {
		rtw_tim_map_clear(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);
		beacon_updated = _TRUE;
		update_beacon(padapter, _TIM_IE_, NULL, _FALSE, 0);
	}

	if (psta->no_short_preamble_set) {
		psta->no_short_preamble_set = 0;
		pmlmepriv->num_sta_no_short_preamble--;
		if (pmlmeext->cur_wireless_mode > WLAN_MD_11B
		    && pmlmepriv->num_sta_no_short_preamble == 0)
			beacon_updated = _TRUE;
	}

	if (psta->nonerp_set) {
		psta->nonerp_set = 0;
		pmlmepriv->num_sta_non_erp--;
		if (pmlmepriv->num_sta_non_erp == 0) {
			beacon_updated = _TRUE;
			update_beacon(padapter, _ERPINFO_IE_, NULL, _FALSE, 0);
		}
	}

	if (psta->no_short_slot_time_set) {
		psta->no_short_slot_time_set = 0;
		pmlmepriv->num_sta_no_short_slot_time--;
		if (pmlmeext->cur_wireless_mode > WLAN_MD_11B
		    && pmlmepriv->num_sta_no_short_slot_time == 0)
			beacon_updated = _TRUE;
	}

#ifdef CONFIG_80211N_HT
	if (psta->no_ht_gf_set) {
		psta->no_ht_gf_set = 0;
		pmlmepriv->num_sta_ht_no_gf--;
	}

	if (psta->no_ht_set) {
		psta->no_ht_set = 0;
		pmlmepriv->num_sta_no_ht--;
	}

	if (psta->ht_20mhz_set) {
		psta->ht_20mhz_set = 0;
		pmlmepriv->num_sta_ht_20mhz--;
	}

	if (psta->ht_40mhz_intolerant) {
		psta->ht_40mhz_intolerant = 0;
		if (pmlmepriv->num_sta_40mhz_intolerant > 0)
			pmlmepriv->num_sta_40mhz_intolerant--;
		else
			rtw_warn_on(1);
	}

	if (rtw_ht_operation_update(padapter) > 0) {
		update_beacon(padapter, _HT_CAPABILITY_IE_, NULL, _FALSE, 0);
		update_beacon(padapter, _HT_ADD_INFO_IE_, NULL, _FALSE, 0);
		beacon_updated = _TRUE;
	}
#endif /* CONFIG_80211N_HT */

#ifdef CONFIG_RTW_MESH
	if (MLME_IS_MESH(padapter)) {
		update_beacon(padapter, WLAN_EID_MESH_CONFIG, NULL, _FALSE, 0);
		if (pstapriv->asoc_list_cnt == 0)
			_cancel_timer_ex(&padapter->mesh_atlm_param_req_timer);
		beacon_updated = _TRUE;
	}
#endif

	if (beacon_updated == _TRUE)
		update_beacon(padapter, 0xFFF, NULL, _TRUE, 0);

#if 0
	/* update associated stations cap. */
	associated_clients_update(padapter,  beacon_updated, STA_INFO_UPDATE_ALL); /* move it to avoid deadlock */
#endif

	RTW_INFO("%s, updated=%d\n", __func__, beacon_updated);

	return beacon_updated;

}

#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
extern int rtl_wifi_fdb_del_hook(const unsigned char *addr);
#endif
u8 ap_free_sta(_adapter *padapter, struct sta_info *psta, bool active, u16 reason, bool enqueue, u8 disassoc)
{
	u8 beacon_updated = _FALSE;

	if (!psta)
		return beacon_updated;

	RTW_INFO("%s sta "MAC_FMT"\n", __func__, MAC_ARG(psta->phl_sta->mac_addr));

	if (active == _TRUE) {
#ifdef CONFIG_80211N_HT
		/* tear down Rx AMPDU */
		send_delba(padapter, 0, psta->phl_sta->mac_addr);/* recipient */

		/* tear down TX AMPDU */
		send_delba(padapter, 1, psta->phl_sta->mac_addr);/*  */ /* originator */

#endif /* CONFIG_80211N_HT */

		if (!MLME_IS_MESH(padapter)) {
			if (disassoc == _TRUE)
				issue_disassoc_g6(padapter, psta->phl_sta->mac_addr, reason);
			else
				issue_deauth_g6(padapter, psta->phl_sta->mac_addr, reason);
		}
	}

#ifdef CONFIG_RTW_MESH
	if (MLME_IS_MESH(padapter))
		rtw_mesh_path_flush_by_nexthop(psta);
#endif

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	_rtw_memset(&psta->wapiStaInfo, 0, sizeof(RTL_WAPI_STA_INFO));
#endif

#ifdef CONFIG_BEAMFORMING
	beamforming_wk_cmd(padapter, BEAMFORMING_CTRL_LEAVE, psta->phl_sta->mac_addr, ETH_ALEN, 1);
#endif

#ifdef CONFIG_80211N_HT
	psta->htpriv.agg_enable_bitmap = 0x0;/* reset */
	psta->htpriv.candidate_tid_bitmap = 0x0;/* reset */
#endif

	/* clear cam entry / key */
	rtw_clearstakey_cmd(padapter, psta, enqueue);


	_rtw_spinlock_bh(&psta->lock);
	psta->state &= ~(WIFI_ASOC_STATE | WIFI_UNDER_KEY_HANDSHAKE);
	psta->state |= (WIFI_DELETE_STATE);

#ifdef CONFIG_IOCTL_CFG80211
	if ((psta->auth_len != 0) && (psta->pauth_frame != NULL)) {
		rtw_mfree(psta->pauth_frame, psta->auth_len);
		psta->pauth_frame = NULL;
		psta->auth_len = 0;
	}
#endif /* CONFIG_IOCTL_CFG80211 */
	_rtw_spinunlock_bh(&psta->lock);

	if (!MLME_IS_MESH(padapter)) {
#ifdef CONFIG_IOCTL_CFG80211
		#ifdef COMPAT_KERNEL_RELEASE
		rtw_cfg80211_indicate_sta_disassoc(padapter, psta->phl_sta->mac_addr, reason);
		#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && !defined(CONFIG_CFG80211_FORCE_COMPATIBLE_2_6_37_UNDER)
		rtw_cfg80211_indicate_sta_disassoc(padapter, psta->phl_sta->mac_addr, reason);
		#else /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && !defined(CONFIG_CFG80211_FORCE_COMPATIBLE_2_6_37_UNDER) */
		/* will call rtw_cfg80211_indicate_sta_disassoc() in cmd_thread for old API context */
		#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && !defined(CONFIG_CFG80211_FORCE_COMPATIBLE_2_6_37_UNDER) */
#else
		rtw_indicate_sta_disassoc_event(padapter, psta);
#endif
	}

	beacon_updated = bss_cap_update_on_sta_leave(padapter, psta);

#ifdef CONFIG_RTW_MULTI_AP
	core_map_send_client_leave_notify(padapter->mac_addr, psta->phl_sta->mac_addr);
#ifdef CONFIG_RTW_MULTI_AP_R2
	//Client Disassociation Stats message
	core_map_send_sta_disassoc_event(psta, reason);
#endif
#endif

#ifdef CONFIG_WLAN_MANAGER
	rtw_netlink_send_del_sta_msg(padapter, psta->phl_sta->mac_addr);
#endif

	report_del_sta_event(padapter, psta->phl_sta->mac_addr, reason, enqueue, _FALSE);

#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
		rtl_wifi_fdb_del_hook(psta->phl_sta->mac_addr);
#endif

	return beacon_updated;

}

int rtw_ap_inform_ch_switch(_adapter *padapter, u8 new_ch, u8 ch_offset)
{
	_list	*phead, *plist;
	int ret = 0;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 bc_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	if (MLME_IN_AP_STATE(pmlmeinfo))
		return ret;

	RTW_INFO(FUNC_NDEV_FMT" with ch:%u, offset:%u\n",
		 FUNC_NDEV_ARG(padapter->pnetdev), new_ch, ch_offset);

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	/* for each sta in asoc_queue */
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		issue_action_spct_ch_switch(padapter, psta->phl_sta->mac_addr, new_ch, ch_offset);
		psta->expire_to = ((pstapriv->expire_to * 2) > 5) ? 5 : (pstapriv->expire_to * 2);
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	issue_action_spct_ch_switch(padapter, bc_addr, new_ch, ch_offset);

	return ret;
}

int rtw_sta_flush(_adapter *padapter, bool enqueue, u8 disassoc)
{
	_list	*phead, *plist;
	int ret = 0;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 bc_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	u8 flush_num = 0;
	char flush_list[NUM_STA];
	int i;
	bool no_sta = false;

	if (!MLME_IS_AP(padapter) && !MLME_IS_MESH(padapter))
		return ret;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	no_sta = rtw_is_list_empty(&pstapriv->asoc_list);
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	/* Issue deauth before delete STAs */
	if (!MLME_IS_MESH(padapter) && !disassoc) {
		if (!no_sta)
			issue_deauth_g6(padapter, bc_addr, WLAN_REASON_DEAUTH_LEAVING);
		else
			RTW_PRINT(ADPT_FMT" has no STA. Not issue BC deauth.\n",
			          ADPT_ARG(padapter));
	}

	/* pick sta from sta asoc_queue */
	if (!no_sta) {
		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
		phead = &pstapriv->asoc_list;
		plist = get_next(phead);
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			int stainfo_offset;

			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);

			rtw_list_delete(&psta->asoc_list);
			pstapriv->asoc_list_cnt--;
			#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
			if (psta->tbtx_enable)
				pstapriv->tbtx_asoc_list_cnt--;
			#endif
			STA_SET_MESH_PLINK(psta, NULL);

			stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
			if (stainfo_offset_valid(stainfo_offset))
				flush_list[flush_num++] = stainfo_offset;
			else
				rtw_warn_on(1);
		}
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	}
	RTW_INFO(FUNC_NDEV_FMT" flush_num:%d\n", FUNC_NDEV_ARG(padapter->pnetdev), flush_num);

	/* call ap_free_sta() for each sta picked */
	for (i = 0; i < flush_num; i++) {
		u8 sta_addr[ETH_ALEN];

		psta = rtw_get_stainfo_by_offset(pstapriv, flush_list[i]);
		_rtw_memcpy(sta_addr, psta->phl_sta->mac_addr, ETH_ALEN);

		ap_free_sta(padapter, psta, _TRUE, WLAN_REASON_DEAUTH_LEAVING, enqueue, disassoc);
		#ifdef CONFIG_RTW_MESH
		if (MLME_IS_MESH(padapter))
			rtw_mesh_expire_peer(padapter, sta_addr);
		#endif
	}

	associated_clients_update(padapter, _TRUE, STA_INFO_UPDATE_ALL);

	return ret;
}

/* called > TSR LEVEL for USB or SDIO Interface*/
void sta_info_update(_adapter *padapter, struct sta_info *psta)
{
	int flags = psta->flags;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);


	/* update wmm cap. */
	if (WLAN_STA_WME & flags)
		psta->qos_option = 1;
	else
		psta->qos_option = 0;

	if (pmlmepriv->qospriv.qos_option == 0)
		psta->qos_option = 0;


#ifdef CONFIG_80211N_HT
	/* update 802.11n ht cap. */
	if (WLAN_STA_HT & flags) {
		psta->htpriv.ht_option = _TRUE;
		psta->qos_option = 1;

		psta->htpriv.smps_cap = (le16_to_cpu(psta->htpriv.ht_cap.cap_info) & IEEE80211_HT_CAP_SM_PS) >> 2;
		psta->phl_sta->asoc_cap.sm_ps = psta->htpriv.smps_cap;
		if (psta->phl_sta->asoc_cap.sm_ps == SM_PS_DYNAMIC)
			VCS_update(padapter, psta);
	} else
		psta->htpriv.ht_option = _FALSE;

	if (pmlmepriv->htpriv.ht_option == _FALSE)
		psta->htpriv.ht_option = _FALSE;
#endif

#ifdef CONFIG_80211AC_VHT
	/* update 802.11AC vht cap. */
	if (WLAN_STA_VHT & flags)
		psta->vhtpriv.vht_option = _TRUE;
	else
		psta->vhtpriv.vht_option = _FALSE;

	if (pmlmepriv->vhtpriv.vht_option == _FALSE)
		psta->vhtpriv.vht_option = _FALSE;
#endif

#ifdef CONFIG_80211AX_HE
	/* update 802.11AX he cap. */
	if (WLAN_STA_HE & flags)
		psta->hepriv.he_option = _TRUE;
	else
		psta->hepriv.he_option = _FALSE;

	if (pmlmepriv->hepriv.he_option == _FALSE)
		psta->hepriv.he_option = _FALSE;
#endif

	update_sta_info_apmode(padapter, psta);
}

/* called >= TSR LEVEL for USB or SDIO Interface*/
void ap_sta_info_defer_update(_adapter *padapter, struct sta_info *psta)
{
	if (psta->state & WIFI_ASOC_STATE)
		rtw_hal_sta_ra_registed(psta);/* DM_RATR_STA_INIT- update_ra_mask */
}
/* restore hw setting from sw data structures */
void rtw_ap_restore_network(_adapter *padapter)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta;
	struct security_priv *psecuritypriv = &(padapter->securitypriv);
	_list	*phead, *plist;
	u8 chk_alive_num = 0;
	char chk_alive_list[NUM_STA];
	int i;

	rtw_setopmode_cmd(padapter
		, MLME_IS_AP(padapter) ? Ndis802_11APMode : Ndis802_11_mesh
		, RTW_CMDF_DIRECTLY
	);

	set_channel_bwmode(padapter,
			pmlmeext->cur_channel,
			pmlmeext->cur_ch_offset,
			pmlmeext->cur_bwmode,
			_FALSE);

	rtw_startbss_cmd(padapter, RTW_CMDF_DIRECTLY);

	if ((padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_) ||
	    (padapter->securitypriv.dot11PrivacyAlgrthm == _AES_)) {
		/* restore group key, WEP keys is restored in ips_leave() */
		rtw_set_key(padapter, psecuritypriv, psecuritypriv->dot118021XGrpKeyid, 0, _FALSE);
	}

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		int stainfo_offset;

		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
		if (stainfo_offset_valid(stainfo_offset))
			chk_alive_list[chk_alive_num++] = stainfo_offset;
	}

	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	for (i = 0; i < chk_alive_num; i++) {
		psta = rtw_get_stainfo_by_offset(pstapriv, chk_alive_list[i]);

		if (psta == NULL){
			RTW_INFO(FUNC_ADPT_FMT" sta_info is null\n", FUNC_ADPT_ARG(padapter));
		} else if (psta->state & WIFI_ASOC_STATE) {
			rtw_sta_media_status_rpt(padapter, psta, 1);
			rtw_hal_sta_ra_registed(psta);
			/* pairwise key */
			/* per sta pairwise key and settings */
			if ((padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_) ||
			    (padapter->securitypriv.dot11PrivacyAlgrthm == _AES_))
				rtw_setstakey_cmd(padapter, psta, UNICAST_KEY, _FALSE);
		}
	}

}

void start_ap_mode(_adapter *padapter)
{
	int i;
	struct sta_info *psta = NULL;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
#ifdef CONFIG_CONCURRENT_MODE
	struct security_priv *psecuritypriv = &padapter->securitypriv;
#endif

	pmlmepriv->update_bcn = _FALSE;

	/*init_mlme_ap_info(padapter);*/

	pmlmeext->bstart_bss = _FALSE;

	pmlmepriv->num_sta_non_erp = 0;

	pmlmepriv->num_sta_no_short_slot_time = 0;

	pmlmepriv->num_sta_no_short_preamble = 0;

	pmlmepriv->num_sta_ht_no_gf = 0;
#ifdef CONFIG_80211N_HT
	pmlmepriv->num_sta_no_ht = 0;
#endif /* CONFIG_80211N_HT */
	pmlmeinfo->HT_info_enable = 0;
	pmlmeinfo->HT_caps_enable = 0;
	pmlmeinfo->HT_enable = 0;

	pmlmepriv->num_sta_ht_20mhz = 0;
	pmlmepriv->num_sta_40mhz_intolerant = 0;
	ATOMIC_SET(&pmlmepriv->olbc_sta, _FALSE);
	ATOMIC_SET(&pmlmepriv->olbc_ap, _FALSE);

#ifdef CONFIG_80211N_HT
	pmlmepriv->ht_20mhz_width_req = _FALSE;
	pmlmepriv->ht_intolerant_ch_reported = _FALSE;
	pmlmepriv->ht_op_mode = 0;
	pmlmepriv->sw_to_20mhz = 0;
#endif

	_rtw_memset(pmlmepriv->ext_capab_ie_data, 0, sizeof(pmlmepriv->ext_capab_ie_data));
	pmlmepriv->ext_capab_ie_len = 0;

#ifdef CONFIG_CONCURRENT_MODE
	psecuritypriv->dot118021x_bmc_cam_id = INVALID_SEC_MAC_CAM_ID;
#endif

	for (i = 0 ;  i < pstapriv->max_aid; i++)
		pstapriv->sta_aid[i] = NULL;

	if (rtw_mi_get_ap_num(padapter))
		RTW_SET_SCAN_BAND_SKIP(padapter, BAND_5G);
}

void stop_ap_mode(_adapter *padapter)
{
	u8 self_action = MLME_ACTION_UNKNOWN;
	struct sta_info *psta = NULL;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	RTW_INFO("%s -"ADPT_FMT"\n", __func__, ADPT_ARG(padapter));

	if (MLME_IS_AP(padapter))
		self_action = MLME_AP_STOPPED;
	else if (MLME_IS_MESH(padapter))
		self_action = MLME_MESH_STOPPED;
	else
		rtw_warn_on(1);

	pmlmepriv->update_bcn = _FALSE;
	/*pmlmeext->bstart_bss = _FALSE;*/
	// padapter->netif_up = _FALSE;
	/* _rtw_spinlock_free(&pmlmepriv->bcn_update_lock); */

	/* reset and init security priv , this can refine with rtw_reset_securitypriv */
	_rtw_memset((unsigned char *)&padapter->securitypriv, 0, sizeof(struct security_priv));
	padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeOpen;
	padapter->securitypriv.ndisencryptstatus = Ndis802_11WEPDisabled;

#ifdef CONFIG_DFS_MASTER
	rtw_dfs_rd_en_decision(padapter, self_action, 0);
#endif

	/* free scan queue */
	rtw_free_network_queue(padapter, _TRUE);

#if CONFIG_RTW_MACADDR_ACL
	rtw_macaddr_acl_clear(padapter, RTW_ACL_PERIOD_BSS);
#endif

	//rtw_sta_flush(padapter, _TRUE, _FALSE);
	rtw_sta_flush(padapter, _FALSE, _FALSE);

	/* free_assoc_sta_resources	 */
	rtw_free_all_stainfo(padapter);

	rtw_free_disabled_stainfo(padapter);

	rtw_free_mlme_priv_ie_data(pmlmepriv);

	pmlmeext->bstart_bss = _FALSE;

	rtw_hal_rcr_set_chk_bssid(padapter, self_action);

	rtw_mi_ap_correct_tsf(padapter, self_action);
#ifndef CONFIG_RTW_LINK_PHL_MASTER
	/* 5cda1b4f8916f87c102063787b895f92b3d77dfe nathan.lin */
	if (padapter->phl_role)
		rtw_phl_reset_chdef(padapter->phl_role);
#endif /* CONFIG_RTW_LINK_PHL_MASTER */
}

#endif /* CONFIG_NATIVEAP_MLME */

void rtw_ap_update_bss_chbw(_adapter *adapter, WLAN_BSSID_EX *bss, u8 ch, u8 bw, u8 offset)
{
#define UPDATE_VHT_CAP 1
#define UPDATE_HT_CAP 1
#ifdef CONFIG_80211AC_VHT
	struct vht_priv *vhtpriv = &adapter->mlmepriv.vhtpriv;
#endif
#ifdef CONFIG_80211AX_HE
	struct he_priv *hepriv = &adapter->mlmepriv.hepriv;
#endif
	{
		u8 *p;
		int ie_len;
		u8 old_ch = bss->Configuration.DSConfig;
		bool change_band = _FALSE;

		if ((ch <= 14 && old_ch >= 36) || (ch >= 36 && old_ch <= 14))
			change_band = _TRUE;

		/* update channel in IE */
		p = rtw_get_ie((bss->IEs + sizeof(NDIS_802_11_FIXED_IEs)), _DSSET_IE_, &ie_len, (bss->IELength - sizeof(NDIS_802_11_FIXED_IEs)));
		if (p && ie_len > 0)
			*(p + 2) = ch;

		bss->Configuration.DSConfig = ch;

		/* band is changed, update ERP, support rate, ext support rate IE */
		if (change_band == _TRUE)
			change_band_update_ie(adapter, bss, ch);
	}

#ifdef CONFIG_80211AX_HE
	if (hepriv->he_option == _TRUE) {
		/* CONFIG_80211AX_HE_TODO */
	}
#endif

#ifdef CONFIG_80211AC_VHT
	if (vhtpriv->vht_option == _TRUE) {
		u8 *vht_cap_ie, *vht_op_ie;
		int vht_cap_ielen, vht_op_ielen;
		u8	center_freq;

		vht_cap_ie = rtw_get_ie((bss->IEs + sizeof(NDIS_802_11_FIXED_IEs)), EID_VHTCapability, &vht_cap_ielen, (bss->IELength - sizeof(NDIS_802_11_FIXED_IEs)));
		vht_op_ie = rtw_get_ie((bss->IEs + sizeof(NDIS_802_11_FIXED_IEs)), EID_VHTOperation, &vht_op_ielen, (bss->IELength - sizeof(NDIS_802_11_FIXED_IEs)));
		center_freq = rtw_get_center_ch(ch, bw, offset);

		/* update vht cap ie */
		if (vht_cap_ie && vht_cap_ielen) {
			#if UPDATE_VHT_CAP
			 if (bw == CHANNEL_WIDTH_160 && vhtpriv->sgi_80m)
				SET_VHT_CAPABILITY_ELE_SHORT_GI160M(vht_cap_ie + 2, 1);
			else
				SET_VHT_CAPABILITY_ELE_SHORT_GI160M(vht_cap_ie + 2, 0);

			if (bw >= CHANNEL_WIDTH_80 && vhtpriv->sgi_80m)
				SET_VHT_CAPABILITY_ELE_SHORT_GI80M(vht_cap_ie + 2, 1);
			else
				SET_VHT_CAPABILITY_ELE_SHORT_GI80M(vht_cap_ie + 2, 0);
			#endif
		}

		/* update vht op ie */
		if (vht_op_ie && vht_op_ielen) {
			if (bw < CHANNEL_WIDTH_80) {
				SET_VHT_OPERATION_ELE_CHL_WIDTH(vht_op_ie + 2, 0);
				SET_VHT_OPERATION_ELE_CHL_CENTER_FREQ1(vht_op_ie + 2, 0);
				SET_VHT_OPERATION_ELE_CHL_CENTER_FREQ2(vht_op_ie + 2, 0);
			} else if (bw == CHANNEL_WIDTH_80) {
				SET_VHT_OPERATION_ELE_CHL_WIDTH(vht_op_ie + 2, 1);
				SET_VHT_OPERATION_ELE_CHL_CENTER_FREQ1(vht_op_ie + 2, center_freq);

				SET_VHT_OPERATION_ELE_CHL_CENTER_FREQ2(vht_op_ie + 2, 0);
			}else if (bw == CHANNEL_WIDTH_160) {
				SET_VHT_OPERATION_ELE_CHL_WIDTH(vht_op_ie + 2, 1);
				SET_VHT_OPERATION_ELE_CHL_CENTER_FREQ1(vht_op_ie + 2, rtw_get_center_ch(ch, CHANNEL_WIDTH_80, offset));
				SET_VHT_OPERATION_ELE_CHL_CENTER_FREQ2(vht_op_ie + 2, rtw_get_center_ch(ch, bw, offset));

			} else {
				RTW_ERR(FUNC_ADPT_FMT" unsupported BW:%u\n", FUNC_ADPT_ARG(adapter), bw);
				rtw_warn_on(1);
			}
		}
	}
#endif /* CONFIG_80211AC_VHT */
#ifdef CONFIG_80211N_HT
	{
		struct ht_priv	*htpriv = &adapter->mlmepriv.htpriv;
		u8 *ht_cap_ie, *ht_op_ie;
		int ht_cap_ielen, ht_op_ielen;

		ht_cap_ie = rtw_get_ie((bss->IEs + sizeof(NDIS_802_11_FIXED_IEs)), EID_HTCapability, &ht_cap_ielen, (bss->IELength - sizeof(NDIS_802_11_FIXED_IEs)));
		ht_op_ie = rtw_get_ie((bss->IEs + sizeof(NDIS_802_11_FIXED_IEs)), EID_HTInfo, &ht_op_ielen, (bss->IELength - sizeof(NDIS_802_11_FIXED_IEs)));

		/* update ht cap ie */
		if (ht_cap_ie && ht_cap_ielen) {
			#if UPDATE_HT_CAP
			if (bw >= CHANNEL_WIDTH_40)
				SET_HT_CAP_ELE_CHL_WIDTH(ht_cap_ie + 2, 1);
			else
				SET_HT_CAP_ELE_CHL_WIDTH(ht_cap_ie + 2, 0);

			if (bw >= CHANNEL_WIDTH_40 && htpriv->sgi_40m)
				SET_HT_CAP_ELE_SHORT_GI40M(ht_cap_ie + 2, 1);
			else
				SET_HT_CAP_ELE_SHORT_GI40M(ht_cap_ie + 2, 0);

			if (htpriv->sgi_20m)
				SET_HT_CAP_ELE_SHORT_GI20M(ht_cap_ie + 2, 1);
			else
				SET_HT_CAP_ELE_SHORT_GI20M(ht_cap_ie + 2, 0);
			#endif
		}

		/* update ht op ie */
		if (ht_op_ie && ht_op_ielen) {
			SET_HT_OP_ELE_PRI_CHL(ht_op_ie + 2, ch);
			switch (offset) {
			case CHAN_OFFSET_UPPER:
				SET_HT_OP_ELE_2ND_CHL_OFFSET(ht_op_ie + 2, IEEE80211_SCA);
				break;
			case CHAN_OFFSET_LOWER:
				SET_HT_OP_ELE_2ND_CHL_OFFSET(ht_op_ie + 2, IEEE80211_SCB);
				break;
			case CHAN_OFFSET_NO_EXT:
			default:
				SET_HT_OP_ELE_2ND_CHL_OFFSET(ht_op_ie + 2, IEEE80211_SCN);
				break;
			}

			if (bw >= CHANNEL_WIDTH_40)
				SET_HT_OP_ELE_STA_CHL_WIDTH(ht_op_ie + 2, 1);
			else
				SET_HT_OP_ELE_STA_CHL_WIDTH(ht_op_ie + 2, 0);
		}
	}
#endif /* CONFIG_80211N_HT */
}

static u16 rtw_ap_update_chbw_by_ifbmp(struct dvobj_priv *dvobj, u16 ifbmp
	, u8 cur_ie_ch[], u8 cur_ie_bw[], u8 cur_ie_offset[]
	, u8 dec_ch[], u8 dec_bw[], u8 dec_offset[]
	, const char *caller)
{
	_adapter *iface;
	struct mlme_ext_priv *mlmeext;
	WLAN_BSSID_EX *network;
#if CONFIG_DFS
	_adapter *primary_adapter = dvobj_get_primary_adapter(dvobj);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(primary_adapter);
#endif
	u16 ifbmp_ch_changed = 0;
	int i;

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (!(ifbmp & BIT(i)) || !dvobj->padapters[i])
			continue;

		iface = dvobj->padapters[i];
		mlmeext = &(iface->mlmeextpriv);

		if (MLME_IS_ASOC(iface)) {
			RTW_INFO(FUNC_ADPT_FMT" %u,%u,%u => %u,%u,%u%s\n", caller, ADPT_ARG(iface)
				, mlmeext->cur_channel, mlmeext->cur_bwmode, mlmeext->cur_ch_offset
				, dec_ch[i], dec_bw[i], dec_offset[i]
				, MLME_IS_OPCH_SW(iface) ? " OPCH_SW" : "");
		} else {
			RTW_INFO(FUNC_ADPT_FMT" %u,%u,%u => %u,%u,%u%s\n", caller, ADPT_ARG(iface)
				, cur_ie_ch[i], cur_ie_bw[i], cur_ie_offset[i]
				, dec_ch[i], dec_bw[i], dec_offset[i]
				, MLME_IS_OPCH_SW(iface) ? " OPCH_SW" : "");
		}
	}

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (!(ifbmp & BIT(i)) || !dvobj->padapters[i])
			continue;

		iface = dvobj->padapters[i];
		mlmeext = &(iface->mlmeextpriv);
		network = &(mlmeext->mlmext_info.network);

		/* ch setting differs from mlmeext.network IE */
		if (cur_ie_ch[i] != dec_ch[i]
			|| cur_ie_bw[i] != dec_bw[i]
			|| cur_ie_offset[i] != dec_offset[i])
			ifbmp_ch_changed |= BIT(i);

		/* ch setting differs from existing one */
		if (MLME_IS_ASOC(iface)
			&& (mlmeext->cur_channel != dec_ch[i]
				|| mlmeext->cur_bwmode != dec_bw[i]
				|| mlmeext->cur_ch_offset != dec_offset[i])
		) {
			if (rtw_linked_check(iface) == _TRUE) {
#ifdef CONFIG_SPCT_CH_SWITCH
				if (1)
					rtw_ap_inform_ch_switch(iface, dec_ch[i], dec_offset[i]);
				else
#endif
#if CONFIG_DFS
				if (rfctl->csa_ch == 0)
#endif
					rtw_sta_flush(iface, _FALSE, _FALSE);
			}
		}

		mlmeext->cur_channel = dec_ch[i];
		mlmeext->cur_bwmode = dec_bw[i];
		mlmeext->cur_ch_offset = dec_offset[i];

		rtw_ap_update_bss_chbw(iface, network, dec_ch[i], dec_bw[i], dec_offset[i]);
	}

	return ifbmp_ch_changed;
}

static u8 rtw_ap_ch_specific_chk(_adapter *adapter, u8 ch, u8 *bw, u8 *offset, const char *caller)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	RT_CHANNEL_INFO *chset = adapter_to_chset(adapter);
	int ch_idx;
	u8 ret = _SUCCESS;

	ch_idx = rtw_chset_search_ch(chset, ch);
	if (ch_idx < 0) {
		RTW_WARN("%s ch:%u doesn't fit in chplan\n", caller, ch);
		ret = _FAIL;
		goto exit;
	}
	if (chset[ch_idx].flags & RTW_CHF_NO_IR) {
		RTW_WARN("%s ch:%u is passive\n", caller, ch);
		ret = _FAIL;
		goto exit;
	}

	rtw_adjust_chbw(adapter, ch, bw, offset);

	if (!rtw_get_offset_by_chbw(ch, *bw, offset)) {
		RTW_WARN("%s %u,%u has no valid offset\n", caller, ch, *bw);
		ret = _FAIL;
		goto exit;
	}

	while (!rtw_chset_is_chbw_valid(chset, ch, *bw, *offset, 0, 0)
		|| (rtw_rfctl_dfs_domain_unknown_g6(rfctl) && rtw_chset_is_dfs_chbw(chset, ch, *bw, *offset))
	) {
		if (*bw > CHANNEL_WIDTH_20)
			(*bw)--;
		if (*bw == CHANNEL_WIDTH_20) {
			*offset = CHAN_OFFSET_NO_EXT;
			break;
		}
	}


	if (rtw_rfctl_dfs_domain_unknown_g6(rfctl) && rtw_chset_is_dfs_chbw(chset, ch, *bw, *offset)) {
		RTW_WARN("%s DFS channel %u can't be used\n", caller, ch);
		ret = _FAIL;
		goto exit;
	}

exit:
	return ret;
}

static bool rtw_ap_choose_chbw(_adapter *adapter, u8 sel_ch, u8 max_bw, u8 cur_ch
	, u8 *ch, u8 *bw, u8 *offset, bool by_int_info, u8 mesh_only, const char *caller)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	bool ch_avail = _FALSE;

#if defined(CONFIG_DFS_MASTER)
	if (!rtw_rfctl_dfs_domain_unknown_g6(rfctl)) {
		if (rfctl->radar_detected
			&& rfctl->dbg_dfs_choose_dfs_ch_first
		) {
			ch_avail = rtw_choose_shortest_waiting_ch(rfctl, sel_ch, max_bw
						, ch, bw, offset
						, RTW_CHF_DFS, 0
						, cur_ch, by_int_info, mesh_only);
			if (ch_avail == _TRUE) {
				RTW_INFO("%s choose 5G DFS channel for debug\n", caller);
				goto exit;
			}
		}

		if (rfctl->radar_detected
			&& (rfctl->dfs_ch_sel_e_flags || rfctl->dfs_ch_sel_d_flags)
		) {
			ch_avail = rtw_choose_shortest_waiting_ch(rfctl, sel_ch, max_bw
						, ch, bw, offset
						, rfctl->dfs_ch_sel_e_flags, rfctl->dfs_ch_sel_d_flags
						, cur_ch, by_int_info, mesh_only);
			if (ch_avail == _TRUE) {
				RTW_INFO("%s choose with dfs_ch_sel_e_flags:0x%02x d_flags:0x%02x for debug\n"
					, caller, rfctl->dfs_ch_sel_e_flags, rfctl->dfs_ch_sel_d_flags);
				goto exit;
			}
		}


		#ifdef CONFIG_DFS_CUSTOMER_CHAN_SEL
		ch_avail = rtw_customer_req_channel_sel(adapter, rfctl, sel_ch, max_bw
					, ch, bw, offset
					, rfctl->dfs_ch_sel_e_flags, rfctl->dfs_ch_sel_d_flags
					, cur_ch, by_int_info, mesh_only);
		#endif

	} else
#endif /* defined(CONFIG_DFS_MASTER) */
	{
		ch_avail = rtw_choose_shortest_waiting_ch(rfctl, sel_ch, max_bw
					, ch, bw, offset
					, 0, RTW_CHF_DFS
					, cur_ch, by_int_info, mesh_only);
	}
#if defined(CONFIG_DFS_MASTER)
exit:
#endif
	if (ch_avail == _FALSE)
		RTW_WARN("%s no available channel\n", caller);

	return ch_avail;
}

u16 rtw_ap_chbw_decision(_adapter *adapter, u16 ifbmp, u16 excl_ifbmp
	, s16 req_ch, s8 req_bw, s8 req_offset
	, u8 *ch, u8 *bw, u8 *offset, u8 *chbw_allow)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	RT_CHANNEL_INFO *chset = adapter_to_chset(adapter);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	bool ch_avail = _FALSE;
	u8 cur_ie_ch[CONFIG_IFACE_NUMBER] = {0};
	u8 cur_ie_bw[CONFIG_IFACE_NUMBER] = {0};
	u8 cur_ie_offset[CONFIG_IFACE_NUMBER] = {0};
	u8 dec_ch[CONFIG_IFACE_NUMBER] = {0};
	u8 dec_bw[CONFIG_IFACE_NUMBER] = {0};
	u8 dec_offset[CONFIG_IFACE_NUMBER] = {0};
	u8 u_ch = 0, u_bw = 0, u_offset = 0;
	struct mlme_ext_priv *mlmeext;
	WLAN_BSSID_EX *network;
	struct mi_state mstate;
	struct mi_state mstate_others;
	bool set_u_ch = _FALSE;
	u16 ifbmp_others = (-1);
	u16 ifbmp_ch_changed = 0;
	bool ifbmp_all_mesh = 0;
	_adapter *iface;
	int i;
	int if_idx = adapter->iface_id;

	ifbmp_others >>= (16 - dvobj->iface_nums);
	ifbmp_others &= ~(ifbmp | excl_ifbmp);

#ifdef CONFIG_RTW_MESH
	for (i = 0; i < dvobj->iface_nums; i++)
		if ((ifbmp & BIT(i)) && dvobj->padapters)
			if (!MLME_IS_MESH(dvobj->padapters[i]))
				break;
	ifbmp_all_mesh = i >= dvobj->iface_nums ? 1 : 0;
#endif

	RTW_INFO("%s ifbmp:0x%04x excl_ifbmp:0x%04x req:%d,%d,%d\n", __func__
		, ifbmp, excl_ifbmp, req_ch, req_bw, req_offset);
	rtw_mi_status_by_ifbmp(dvobj, ifbmp, &mstate);
	ifbmp_others = rtw_mi_status_by_ifbmp(dvobj, ifbmp_others, &mstate_others);
	RTW_INFO("%s others ld_sta_num:%u, lg_sta_num:%u, ap_num:%u, mesh_num:%u\n"
		, __func__, MSTATE_STA_LD_NUM(&mstate_others), MSTATE_STA_LG_NUM(&mstate_others)
		, MSTATE_AP_NUM(&mstate_others), MSTATE_MESH_NUM(&mstate_others));

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (!(ifbmp & BIT(i)) || !dvobj->padapters[i])
			continue;
		iface = dvobj->padapters[i];
		mlmeext = &(iface->mlmeextpriv);
		network = &(mlmeext->mlmext_info.network);

		/* get current IE channel settings */
		rtw_ies_get_chbw(BSS_EX_TLV_IES(network), BSS_EX_TLV_IES_LEN(network)
			, &cur_ie_ch[i], &cur_ie_bw[i], &cur_ie_offset[i], 1, 1);

		RTW_PRINT(FUNC_ADPT_FMT" IE ch (%u)=%d bw=%d off=%d\n",
		          FUNC_ADPT_ARG(iface), req_ch, cur_ie_ch[i], cur_ie_bw[i], cur_ie_offset[i]);

		/* prepare temporary channel setting decision */
		if (req_ch == 0) {
			/* request comes from upper layer, use cur_ie values */
			dec_ch[i] = cur_ie_ch[i];
			dec_bw[i] = cur_ie_bw[i];
			dec_offset[i] = cur_ie_offset[i];
		} else {
			/* use chbw of cur_ie updated with specifying req as temporary decision */
			dec_ch[i] = (req_ch <= REQ_CH_NONE) ? cur_ie_ch[i] : req_ch;
			if (req_bw <= REQ_BW_NONE) {
				if (req_bw == REQ_BW_ORI)
					dec_bw[i] = iface->mlmepriv.ori_bw;
				else
					dec_bw[i] = cur_ie_bw[i];
			} else
				dec_bw[i] = req_bw;
			dec_offset[i] = (req_offset <= REQ_OFFSET_NONE) ? cur_ie_offset[i] : req_offset;
		}
	}

	RTW_PRINT(FUNC_ADPT_FMT" ifbmp=0x%04x cur: ch=%d bw=%d off=%d, dec: ch=%d bw=%d off=%d \n",
		  FUNC_ADPT_ARG(adapter), ifbmp,
		  cur_ie_ch[if_idx], cur_ie_bw[if_idx], cur_ie_offset[if_idx],
		  dec_ch[if_idx], dec_bw[if_idx], dec_offset[if_idx]);

	if (MSTATE_STA_LD_NUM(&mstate_others) || MSTATE_STA_LG_NUM(&mstate_others)
		|| MSTATE_AP_NUM(&mstate_others) || MSTATE_MESH_NUM(&mstate_others)
	) {
		/* has linked/linking STA or has AP/Mesh mode */
		rtw_warn_on(!rtw_mi_get_ch_setting_union_by_ifbmp(dvobj, ifbmp_others, &u_ch, &u_bw, &u_offset));
		if ((u_ch == 0) && !is_primary_adapter(adapter)) {
			_adapter *primary = dvobj_get_primary_adapter(dvobj);

			if (MLME_IS_AP(primary) && primary->netif_up) {
				mlmeext = &primary->mlmeextpriv;
				u_ch = mlmeext->cur_channel;
				u_bw = mlmeext->cur_bwmode;
				u_offset = mlmeext->cur_ch_offset;

				RTW_PRINT("Primary AP is up, follow its setting (C%u/B%u/I%u).\n",
				          u_ch, u_bw, u_offset);
			} else {
				RTW_WARN(FUNC_ADPT_FMT" No union channel! (C%u/B%u/I%u).\n",
				         FUNC_ADPT_ARG(adapter), u_ch, u_bw, u_offset);
			}
		}
		RTW_PRINT("Others(%04X) union:%u,%u,%u\n", ifbmp_others, u_ch, u_bw, u_offset);
	}

#ifdef CONFIG_MCC_MODE
	if (MCC_EN(adapter) && req_ch == 0) {
		if (rtw_hal_check_mcc_status(adapter, MCC_STATUS_DOING_MCC)) {
			u8 if_id = adapter->iface_id;

			mlmeext = &(adapter->mlmeextpriv);

			/* check channel settings are the same */
			if (cur_ie_ch[if_id] == mlmeext->cur_channel
				&& cur_ie_bw[if_id] == mlmeext->cur_bwmode
				&& cur_ie_offset[if_id] == mlmeext->cur_ch_offset) {

				RTW_INFO(FUNC_ADPT_FMT"req ch settings are the same as current ch setting, go to exit\n"
					, FUNC_ADPT_ARG(adapter));

				*chbw_allow = _FALSE;
				goto exit;
			} else {
				RTW_INFO(FUNC_ADPT_FMT"request channel settings are not the same as current channel setting(%d,%d,%d,%d,%d,%d), restart MCC\n"
					, FUNC_ADPT_ARG(adapter)
					, cur_ie_ch[if_id], cur_ie_bw[if_id], cur_ie_offset[if_id]
					, mlmeext->cur_channel, mlmeext->cur_bwmode, mlmeext->cur_ch_offset);

				rtw_hal_set_mcc_setting_disconnect(adapter);
			}
		}
	}
#endif /* CONFIG_MCC_MODE */

	if (MSTATE_STA_LG_NUM(&mstate_others) && !MSTATE_STA_LD_NUM(&mstate_others)) {
		/* has linking STA but no linked STA */
		DBGP("1: ifbmp=0x%04x cur: ch=%d bw=%d off=%d, dec: ch=%d bw=%d off=%d \n",
		     ifbmp,
			cur_ie_ch[if_idx], cur_ie_bw[if_idx], cur_ie_offset[if_idx],
			dec_ch[if_idx], dec_bw[if_idx], dec_offset[if_idx]);

		for (i = 0; i < dvobj->iface_nums; i++) {
			if (!(ifbmp & BIT(i)) || !dvobj->padapters[i])
				continue;
			iface = dvobj->padapters[i];

			rtw_adjust_chbw(iface, dec_ch[i], &dec_bw[i], &dec_offset[i]);
			#ifdef CONFIG_RTW_MESH
			if (MLME_IS_MESH(iface))
				rtw_mesh_adjust_chbw(dec_ch[i], &dec_bw[i], &dec_offset[i]);
			#endif

			if (rtw_is_chbw_grouped(u_ch, u_bw, u_offset, dec_ch[i], dec_bw[i], dec_offset[i])) {
				rtw_chset_sync_chbw(chset
					, &dec_ch[i], &dec_bw[i], &dec_offset[i]
					, &u_ch, &u_bw, &u_offset, 1, 0);
				set_u_ch = _TRUE;

				/* channel bw offset can be allowed, not need MCC */
				*chbw_allow = _TRUE;
			} else {
				#ifdef CONFIG_MCC_MODE
				if (MCC_EN(iface)) {
					mlmeext = &(iface->mlmeextpriv);
					mlmeext->cur_channel = *ch = dec_ch[i];
					mlmeext->cur_bwmode = *bw = dec_bw[i];
					mlmeext->cur_ch_offset = *offset = dec_offset[i];

					/* channel bw offset can not be allowed, need MCC */
					*chbw_allow = _FALSE;
					RTW_INFO(FUNC_ADPT_FMT" enable mcc: %u,%u,%u\n", FUNC_ADPT_ARG(iface)
						 , *ch, *bw, *offset);
					goto exit;
				}
				#endif /* CONFIG_MCC_MODE */

				/* set this for possible ch change when join down*/
				set_fwstate(&iface->mlmepriv, WIFI_OP_CH_SWITCHING);
			}
		}

	} else if (MSTATE_STA_LD_NUM(&mstate_others)
		|| MSTATE_AP_NUM(&mstate_others) || MSTATE_MESH_NUM(&mstate_others)
	) {
		/* has linked STA mode or AP/Mesh mode */
		DBGP("2: ifbmp=0x%04x cur: ch=%d bw=%d off=%d, dec: ch=%d bw=%d off=%d \n",
		     ifbmp,
			cur_ie_ch[if_idx], cur_ie_bw[if_idx], cur_ie_offset[if_idx],
			dec_ch[if_idx], dec_bw[if_idx], dec_offset[if_idx]);

		for (i = 0; i < dvobj->iface_nums; i++) {
			if (!(ifbmp & BIT(i)) || !dvobj->padapters[i])
				continue;
			iface = dvobj->padapters[i];
			if (is_primary_adapter(iface) && MSTATE_AP_NUM(&mstate_others) &&
				 !MSTATE_STA_LD_NUM(&mstate_others) && !MSTATE_MESH_NUM(&mstate_others)) {
				DBGP("ifbmp:%d, u_ch:%d, u_bw:%d ,u_offset:%d, dec_ch:%d, dec_bw:%d, dec_offset:%d\n",
						ifbmp, u_ch, u_bw, u_offset, dec_ch[i], dec_bw[i], dec_offset[i]);
				u_ch = dec_ch[i];
				u_bw = dec_bw[i];
				u_offset = dec_offset[i];
			}

			rtw_adjust_chbw(iface, u_ch, &dec_bw[i], &dec_offset[i]);
			#ifdef CONFIG_RTW_MESH
			if (MLME_IS_MESH(iface))
				rtw_mesh_adjust_chbw(u_ch, &dec_bw[i], &dec_offset[i]);
			#endif

			#ifdef CONFIG_MCC_MODE
			if (MCC_EN(iface)) {
				if (!rtw_is_chbw_grouped(u_ch, u_bw, u_offset, dec_ch[i], dec_bw[i], dec_offset[i])) {
					mlmeext = &(iface->mlmeextpriv);
					mlmeext->cur_channel = *ch = dec_ch[i] = cur_ie_ch[i];
					mlmeext->cur_bwmode = *bw = dec_bw[i] = cur_ie_bw[i];
					mlmeext->cur_ch_offset = *offset = dec_offset[i] = cur_ie_offset[i];
					/* channel bw offset can not be allowed, need MCC */
					*chbw_allow = _FALSE;
					RTW_INFO(FUNC_ADPT_FMT" enable mcc: %u,%u,%u\n", FUNC_ADPT_ARG(iface)
						 , *ch, *bw, *offset);
					goto exit;
				} else
					/* channel bw offset can be allowed, not need MCC */
					*chbw_allow = _TRUE;
			}
			#endif /* CONFIG_MCC_MODE */

			if (req_ch == 0 && dec_bw[i] > u_bw
				&& rtw_chset_is_dfs_chbw(chset, u_ch, u_bw, u_offset)
			) {
				/* request comes from upper layer, prevent from additional channel waiting */
				dec_bw[i] = u_bw;
				if (dec_bw[i] == CHANNEL_WIDTH_20)
					dec_offset[i] = CHAN_OFFSET_NO_EXT;
			}

			/* follow */
			rtw_chset_sync_chbw(chset
				, &dec_ch[i], &dec_bw[i], &dec_offset[i]
				, &u_ch, &u_bw, &u_offset, 1, 0);
		}

		set_u_ch = _TRUE;

	} else {
		/* autonomous decision */
		u8 ori_ch = 0;
		u8 max_bw;
		bool by_int_info;

		DBGP("3: ifbmp=0x%04x cur: ch=%d bw=%d off=%d, dec: ch=%d bw=%d off=%d \n",
		     ifbmp,
			cur_ie_ch[0], cur_ie_bw[0], cur_ie_offset[0],
			dec_ch[0], dec_bw[0], dec_offset[0]);

		/* autonomous decision, not need MCC */
		*chbw_allow = _TRUE;

		if (req_ch <= REQ_CH_NONE) /* channel is not specified */
			goto choose_chbw;

		/* get tmp dec union of ifbmp */
		for (i = 0; i < dvobj->iface_nums; i++) {
			if (!(ifbmp & BIT(i)) || !dvobj->padapters[i])
				continue;
			if (u_ch == 0) {
				u_ch = dec_ch[i];
				u_bw = dec_bw[i];
				u_offset = dec_offset[i];
				rtw_adjust_chbw(adapter, u_ch, &u_bw, &u_offset);
				rtw_get_offset_by_chbw(u_ch, u_bw, &u_offset);
			} else {
				u8 tmp_ch = dec_ch[i];
				u8 tmp_bw = dec_bw[i];
				u8 tmp_offset = dec_offset[i];

				rtw_adjust_chbw(adapter, tmp_ch, &tmp_bw, &tmp_offset);
				rtw_get_offset_by_chbw(tmp_ch, tmp_bw, &tmp_offset);

				rtw_warn_on(!rtw_is_chbw_grouped(u_ch, u_bw, u_offset, tmp_ch, tmp_bw, tmp_offset));
				rtw_sync_chbw(&tmp_ch, &tmp_bw, &tmp_offset, &u_ch, &u_bw, &u_offset);
			}
		}

		#ifdef CONFIG_RTW_MESH
		/* if ifbmp are all mesh, apply bw restriction */
		if (ifbmp_all_mesh)
			rtw_mesh_adjust_chbw(u_ch, &u_bw, &u_offset);
		#endif
		DBGP("4: ifbmp=0x%04x cur: ch=%d bw=%d off=%d, dec: ch=%d bw=%d off=%d \n",
		     ifbmp,
			cur_ie_ch[if_idx], cur_ie_bw[if_idx], cur_ie_offset[if_idx],
			dec_ch[if_idx], dec_bw[if_idx], dec_offset[if_idx]);

		RTW_INFO("%s ifbmp:0x%04x tmp union:%u,%u,%u\n", __func__, ifbmp, u_ch, u_bw, u_offset);

		/* check if tmp dec union is usable */
		if (rtw_ap_ch_specific_chk(adapter, u_ch, &u_bw, &u_offset, __func__) == _FAIL) {
			/* channel can't be used */
			if (req_ch > 0) {
				/* specific channel and not from IE => don't change channel setting */
				goto exit;
			}
			goto choose_chbw;
		} else if (rtw_chset_is_chbw_non_ocp(chset, u_ch, u_bw, u_offset)) {
			RTW_WARN("%s DFS channel %u,%u under non ocp\n", __func__, u_ch, u_bw);
			if (req_ch > 0 && req_bw > REQ_BW_NONE) {
				/* change_chbw with specific channel and specific bw, goto update_bss_chbw directly */
				goto update_bss_chbw;
			}
		} else
			goto update_bss_chbw;

		DBGP("5: ifbmp=0x%04x cur: ch=%d bw=%d off=%d, dec: ch=%d bw=%d off=%d \n",
		     ifbmp,
			cur_ie_ch[if_idx], cur_ie_bw[if_idx], cur_ie_offset[if_idx],
			dec_ch[if_idx], dec_bw[if_idx], dec_offset[if_idx]);

choose_chbw:
		by_int_info = req_ch == REQ_CH_INT_INFO ? 1 : 0;
		req_ch = req_ch > 0 ? req_ch : 0;
		max_bw = req_bw > REQ_BW_NONE ? req_bw : CHANNEL_WIDTH_20;

		DBGP("6: ifbmp=0x%06x cur: ch=%d bw=%d off=%d, dec: ch=%d bw=%d off=%d \n", ifbmp,
			cur_ie_ch[if_idx], cur_ie_bw[if_idx], cur_ie_offset[if_idx],
			dec_ch[if_idx], dec_bw[if_idx], dec_offset[if_idx]);

		for (i = 0; i < dvobj->iface_nums; i++) {
			if (!(ifbmp & BIT(i)) || !dvobj->padapters[i])
				continue;
			iface = dvobj->padapters[i];
			mlmeext = &(iface->mlmeextpriv);

			if (req_bw <= REQ_BW_NONE) {
				if (req_bw == REQ_BW_ORI) {
					if (max_bw < iface->mlmepriv.ori_bw)
						max_bw = iface->mlmepriv.ori_bw;
				} else {
					if (max_bw < cur_ie_bw[i])
						max_bw = cur_ie_bw[i];
				}
			}

			if (MSTATE_AP_NUM(&mstate) || MSTATE_MESH_NUM(&mstate)) {
				if (ori_ch == 0)
					ori_ch = mlmeext->cur_channel;
				else if (ori_ch != mlmeext->cur_channel)
					rtw_warn_on(1);
			} else {
				if (ori_ch == 0)
					ori_ch = cur_ie_ch[i];
				else if (ori_ch != cur_ie_ch[i])
					rtw_warn_on(1);
			}
		}

		DBGP("7: ifbmp=0x%04x cur: ch=%d bw=%d off=%d, dec: ch=%d bw=%d off=%d \n",
		     ifbmp,
			cur_ie_ch[if_idx], cur_ie_bw[if_idx], cur_ie_offset[if_idx],
			dec_ch[if_idx], dec_bw[if_idx], dec_offset[if_idx]);

		ch_avail = rtw_ap_choose_chbw(adapter, req_ch, max_bw
			, ori_ch, &u_ch, &u_bw, &u_offset, by_int_info, ifbmp_all_mesh, __func__);
		if (ch_avail == _FALSE)
			goto exit;

update_bss_chbw:
		for (i = 0; i < dvobj->iface_nums; i++) {
			if (!(ifbmp & BIT(i)) || !dvobj->padapters[i])
				continue;
			iface = dvobj->padapters[i];

			dec_ch[i] = u_ch;
			if (dec_bw[i] > u_bw)
				dec_bw[i] = u_bw;
			if (dec_bw[i] == CHANNEL_WIDTH_20)
				dec_offset[i] = CHAN_OFFSET_NO_EXT;
			else
				dec_offset[i] = u_offset;

			#ifdef CONFIG_RTW_MESH
			if (MLME_IS_MESH(iface))
				rtw_mesh_adjust_chbw(dec_ch[i], &dec_bw[i], &dec_offset[i]);
			#endif
		}

		set_u_ch = _TRUE;
	}


	DBGP("8: ifbmp=0x%04x cur: ch=%d bw=%d off=%d, dec: ch=%d bw=%d off=%d \n",
	     ifbmp,
		cur_ie_ch[if_idx], cur_ie_bw[if_idx], cur_ie_offset[if_idx],
		dec_ch[if_idx], dec_bw[if_idx], dec_offset[if_idx]);

	ifbmp_ch_changed = rtw_ap_update_chbw_by_ifbmp(dvobj, ifbmp
							, cur_ie_ch, cur_ie_bw, cur_ie_offset
							, dec_ch, dec_bw, dec_offset
							, __func__);

	RTW_PRINT(ADPT_FMT" cur: ch=%d bw=%d off=%d\n",
			 ADPT_ARG(adapter), adapter->mlmeextpriv.cur_channel,
			 adapter->mlmeextpriv.cur_bwmode,
			 adapter->mlmeextpriv.cur_ch_offset);

	if (u_ch != 0)
		RTW_INFO("%s union:%u,%u,%u\n", __func__, u_ch, u_bw, u_offset);

	if (rtw_mi_check_fwstate(adapter, WIFI_UNDER_SURVEY)) {
		/* scanning, leave ch setting to scan state machine */
		set_u_ch = _FALSE;
	}

	if (set_u_ch == _TRUE) {
		*ch = u_ch;
		*bw = u_bw;
		*offset = u_offset;
	}
exit:
	RTW_PRINT(FUNC_ADPT_FMT" Final: S%u C%u B%u O%u I%04X\n",
	          FUNC_ADPT_ARG(adapter),
	          set_u_ch, *ch, *bw, *offset, ifbmp_ch_changed);

	return ifbmp_ch_changed;
}

u8 rtw_ap_sta_states_check(_adapter *adapter)
{
	struct sta_info *psta;
	struct sta_priv *pstapriv = &adapter->stapriv;
	_list *plist, *phead;
	u8 rst = _FALSE;

	if (!MLME_IS_AP(adapter) && !MLME_IS_MESH(adapter))
		return _FALSE;

	if (pstapriv->auth_list_cnt !=0)
		return _TRUE;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {

		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		if (!(psta->state & WIFI_ASOC_STATE)) {
			RTW_INFO(ADPT_FMT"- SoftAP/Mesh - sta under linking, its state = 0x%x\n", ADPT_ARG(adapter), psta->state);
			rst = _TRUE;
			break;
		} else if (psta->state & WIFI_UNDER_KEY_HANDSHAKE) {
			RTW_INFO(ADPT_FMT"- SoftAP/Mesh - sta under key handshaking, its state = 0x%x\n", ADPT_ARG(adapter), psta->state);
			rst = _TRUE;
			break;
		}
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	return rst;
}

void rtw_ap_parse_sta_capability(_adapter *adapter, struct sta_info *sta, u8 *cap)
{
	sta->capability = RTW_GET_LE16(cap);
	if (sta->capability & WLAN_CAPABILITY_SHORT_PREAMBLE)
		sta->flags |= WLAN_STA_SHORT_PREAMBLE;
	else
		sta->flags &= ~WLAN_STA_SHORT_PREAMBLE;
}

void rtw_ap_parse_sta_ext_capability(_adapter *adapter,
									struct sta_info *sta,
									struct rtw_ieee802_11_elems *elems)
{
	u8 copy_len = elems->ext_cap_len;
	if (copy_len > 12)
		copy_len = 12;
	_rtw_memset(sta->ext_capab_ie_data, 0, 12);
	if(copy_len)
		_rtw_memcpy(sta->ext_capab_ie_data, elems->ext_cap, copy_len);
	sta->ext_capab_ie_len = copy_len;
}

u16 rtw_ap_parse_sta_supported_rates(_adapter *adapter, struct sta_info *sta, u8 *tlv_ies, u16 tlv_ies_len)
{
	u8 rate_set[12];
	u8 rate_num;
	int i;
	u16 status = _STATS_SUCCESSFUL_;

	rtw_ies_get_supported_rate(tlv_ies, tlv_ies_len, rate_set, &rate_num);
	if (rate_num == 0) {
		RTW_INFO(FUNC_ADPT_FMT" sta "MAC_FMT" with no supported rate\n"
			, FUNC_ADPT_ARG(adapter), MAC_ARG(sta->phl_sta->mac_addr));
		status = _STATS_FAILURE_;
		goto exit;
	}

	_rtw_memcpy(sta->bssrateset, rate_set, rate_num);
	sta->bssratelen = rate_num;

	if (MLME_IS_AP(adapter)) {
		/* this function force only CCK rates to be bassic rate... */
		UpdateBrateTblForSoftAP(sta->bssrateset, sta->bssratelen);
	}

	/* if (hapd->iface->current_mode->mode == HOSTAPD_MODE_IEEE80211G) */ /* ? */
	sta->flags |= WLAN_STA_NONERP;
	for (i = 0; i < sta->bssratelen; i++) {
		if ((sta->bssrateset[i] & 0x7f) > 22) {
			sta->flags &= ~WLAN_STA_NONERP;
			break;
		}
	}

exit:
	return status;
}

u16 rtw_ap_parse_sta_security_ie(_adapter *adapter, struct sta_info *sta, struct rtw_ieee802_11_elems *elems)
{
	struct security_priv *sec = &adapter->securitypriv;
	u8 *wpa_ie;
	int wpa_ie_len;
	int group_cipher = 0, pairwise_cipher = 0, gmcs = 0;
	u32 akm = 0;
	u8 mfp_opt = MFP_NO;
	u16 status = _STATS_SUCCESSFUL_;

	sta->dot8021xalg = 0;
	sta->wpa_psk = 0;
	sta->wpa_group_cipher = 0;
	sta->wpa2_group_cipher = 0;
	sta->wpa_pairwise_cipher = 0;
	sta->wpa2_pairwise_cipher = 0;
	sta->akm_suite_type = 0;
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	sta->wapi_group_cipher = 0;
	sta->wapi_pairwise_cipher =0;
#endif

	_rtw_memset(sta->wpa_ie, 0, sizeof(sta->wpa_ie));

	if ((sec->wpa_psk & BIT(1)) && elems->rsn_ie) {
		wpa_ie = elems->rsn_ie;
		wpa_ie_len = elems->rsn_ie_len;

		if (rtw_parse_wpa2_ie(wpa_ie - 2, wpa_ie_len + 2, &group_cipher, &pairwise_cipher, &gmcs, &akm, &mfp_opt) == _SUCCESS) {
			sta->dot8021xalg = 1;/* psk, todo:802.1x */
			sta->wpa_psk |= BIT(1);

			/* RSN optional field absent; the validation is already checked in rtw_rsne_info_parse() */
			if (!group_cipher) {
				RTW_INFO("STA lacks WPA2 Group Suite Cipher --> Default\n");
				group_cipher = sec->wpa2_group_cipher;
			}
			if (!pairwise_cipher) {
				RTW_INFO("STA lacks WPA2 Pairwise Suite Cipher --> Default\n");
				pairwise_cipher = sec->wpa2_pairwise_cipher;
			}
			if (!akm) {
				RTW_INFO("STA lacks WPA2 AKM Cipher --> Default\n");
				akm = sec->akmp;
			}

			sta->wpa2_group_cipher = group_cipher & sec->wpa2_group_cipher;
			sta->wpa2_pairwise_cipher = pairwise_cipher & sec->wpa2_pairwise_cipher;
			sta->akm_suite_type = akm;

			if (MLME_IS_AP(adapter) && (CHECK_BIT(WLAN_AKM_TYPE_SAE, akm)) && (MFP_NO == mfp_opt)) {
				status = WLAN_STATUS_ROBUST_MGMT_FRAME_POLICY_VIOLATION;
				goto exit;
			}

			/* RSN optional field exists but no match */
			if (!sta->wpa2_group_cipher) {
				status = WLAN_STATUS_GROUP_CIPHER_NOT_VALID;
				goto exit;
			}
			if (!sta->wpa2_pairwise_cipher) {
				status = WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID;
				goto exit;
			}
			if (MLME_IS_AP(adapter) && (!CHECK_BIT(sec->akmp, akm))) {
				status = WLAN_STATUS_AKMP_NOT_VALID;
				goto exit;
			}
		}
		else {
			status = WLAN_STATUS_INVALID_IE;
			goto exit;
		}
	}
	else if ((sec->wpa_psk & BIT(0)) && elems->wpa_ie) {
		wpa_ie = elems->wpa_ie;
		wpa_ie_len = elems->wpa_ie_len;

		if (rtw_parse_wpa_ie(wpa_ie - 2, wpa_ie_len + 2, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS) {
			sta->dot8021xalg = 1;/* psk, todo:802.1x */
			sta->wpa_psk |= BIT(0);

			sta->wpa_group_cipher = group_cipher & sec->wpa_group_cipher;
			sta->wpa_pairwise_cipher = pairwise_cipher & sec->wpa_pairwise_cipher;

			if (!sta->wpa_group_cipher) {
				status = WLAN_STATUS_GROUP_CIPHER_NOT_VALID;
				goto exit;
			}

			if (!sta->wpa_pairwise_cipher) {
				status = WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID;
				goto exit;
			}
		} else {
			status = WLAN_STATUS_INVALID_IE;
			goto exit;
		}
	}
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	else if ((sec->wpa_psk & BIT(2)) && elems->wapi_ie) {
		wpa_ie = elems->wapi_ie;
		wpa_ie_len = elems->wapi_ie_len;

		if (rtw_parse_wapi_ie(wpa_ie - 2, wpa_ie_len + 2, &group_cipher, &pairwise_cipher, &gmcs, &akm, &mfp_opt) == _SUCCESS) {
			sta->dot8021xalg = 1; /* psk, TODO: 802.1x */
			sta->wpa_psk = BIT(2);

			if (!CHECK_BIT(group_cipher, sec->wapi_group_cipher)) {
				status = WLAN_STATUS_GROUP_CIPHER_NOT_VALID;
				goto exit;
			}
			if (!CHECK_BIT(pairwise_cipher, sec->wapi_pairwise_cipher)) {
				status = WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID;
				goto exit;
			}
			if (MLME_IS_AP(adapter) && (!CHECK_BIT(akm, sec->akmp))) {
				status = WLAN_STATUS_AKMP_NOT_VALID;
				goto exit;
			}

			sta->wapi_group_cipher = group_cipher & sec->wapi_group_cipher;
			sta->wapi_pairwise_cipher = pairwise_cipher & sec->wapi_pairwise_cipher;
			sta->akm_suite_type = akm;
		}
		else {
			status = WLAN_STATUS_INVALID_IE;
			goto exit;
		}
	}
#endif
	else {
		wpa_ie = NULL;
		wpa_ie_len = 0;
	}

	if (wpa_ie != NULL) {
		if ((sec->mfp_opt == MFP_REQUIRED && mfp_opt < MFP_OPTIONAL)
			|| (mfp_opt == MFP_REQUIRED && sec->mfp_opt < MFP_OPTIONAL)
		) {
			status = WLAN_STATUS_ROBUST_MGMT_FRAME_POLICY_VIOLATION;
			goto exit;
		}
	}

#ifdef CONFIG_RTW_MESH
	if (MLME_IS_MESH(adapter)) {
		/* MFP is mandatory for secure mesh */
		if (adapter->mesh_info.mesh_auth_id)
			sta->flags |= WLAN_STA_MFP;
	} else
#endif
	if (sec->mfp_opt >= MFP_OPTIONAL && mfp_opt >= MFP_OPTIONAL)
		sta->flags |= WLAN_STA_MFP;

#ifdef CONFIG_IEEE80211W
	if ((sta->flags & WLAN_STA_MFP)
		&& (sec->mfp_opt >= MFP_OPTIONAL && mfp_opt >= MFP_OPTIONAL)
		&& security_type_bip_to_gmcs(sec->dot11wCipher) != gmcs
	) {
		status = WLAN_STATUS_CIPHER_REJECTED_PER_POLICY;
		goto exit;
	}
#endif

#ifndef CONFIG_IOCTL_CFG80211 //PMKID maintain by hostapd
	if (MLME_IS_AP(adapter) &&
		(sec->auth_type == MLME_AUTHTYPE_SAE) &&
		(CHECK_BIT(WLAN_AKM_TYPE_SAE, sta->akm_suite_type)) &&
		(WLAN_AUTH_OPEN == sta->authalg)) {
		/* WPA3-SAE, PMK caching */
		if (rtw_cached_pmkid(adapter, sta->phl_sta->mac_addr) == -1) {
			RTW_INFO("SAE: No PMKSA cache entry found\n");
			status = WLAN_STATUS_INVALID_PMKID;
			goto exit;
		} else {
			RTW_INFO("SAE: PMKSA cache entry found\n");
		}
	}
#endif /* CONFIG_IOCTL_CFG80211 */

	if (!MLME_IS_AP(adapter))
		goto exit;

	sta->flags &= ~(WLAN_STA_WPS | WLAN_STA_MAYBE_WPS);
	/* if (hapd->conf->wps_state && wpa_ie == NULL) { */ /* todo: to check ap if supporting WPS */
	if (wpa_ie == NULL) {
		if (elems->wps_ie) {
			RTW_INFO("STA included WPS IE in "
				 "(Re)Association Request - assume WPS is "
				 "used\n");
			sta->flags |= WLAN_STA_WPS;
			/* wpabuf_free(sta->wps_ie); */
			/* sta->wps_ie = wpabuf_alloc_copy(elems.wps_ie + 4, */
			/*				elems.wps_ie_len - 4); */
		} else {
			RTW_INFO("STA did not include WPA/RSN IE "
				 "in (Re)Association Request - possible WPS "
				 "use\n");
			sta->flags |= WLAN_STA_MAYBE_WPS;
		}

		/* AP support WPA/RSN, and sta is going to do WPS, but AP is not ready */
		/* that the selected registrar of AP is _FLASE */
		if ((sec->wpa_psk > 0)
			&& (sta->flags & (WLAN_STA_WPS | WLAN_STA_MAYBE_WPS))
		) {
			struct mlme_priv *mlme = &adapter->mlmepriv;

			if (mlme->wps_beacon_ie) {
				u8 selected_registrar = 0;

				rtw_get_wps_attr_content(mlme->wps_beacon_ie, mlme->wps_beacon_ie_len, WPS_ATTR_SELECTED_REGISTRAR, &selected_registrar, NULL);

				if (!selected_registrar) {
					RTW_INFO("selected_registrar is _FALSE , or AP is not ready to do WPS\n");
					status = _STATS_UNABLE_HANDLE_STA_;
					goto exit;
				}
			}
		}

	} else {
		int copy_len;

		if (sec->wpa_psk == 0) {
			RTW_INFO("STA " MAC_FMT
				": WPA/RSN IE in association request, but AP don't support WPA/RSN\n",
				MAC_ARG(sta->phl_sta->mac_addr));
			status = WLAN_STATUS_INVALID_IE;
			goto exit;
		}

		if (elems->wps_ie) {
			RTW_INFO("STA included WPS IE in "
				 "(Re)Association Request - WPS is "
				 "used\n");
			sta->flags |= WLAN_STA_WPS;
			copy_len = 0;
		} else
			copy_len = ((wpa_ie_len + 2) > sizeof(sta->wpa_ie)) ? (sizeof(sta->wpa_ie)) : (wpa_ie_len + 2);

		if (copy_len > 0)
			_rtw_memcpy(sta->wpa_ie, wpa_ie - 2, copy_len);
	}

exit:
	return status;
}

void rtw_ap_parse_sta_wmm_ie(_adapter *adapter, struct sta_info *sta, u8 *tlv_ies, u16 tlv_ies_len)
{
	struct mlme_priv *mlme = &adapter->mlmepriv;
	unsigned char WMM_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
	u8 *p;

	sta->flags &= ~WLAN_STA_WME;
	sta->qos_option = 0;
	sta->qos_info = 0;
	sta->uapsd_bitmap = 0;

	if (!mlme->qospriv.qos_option)
		goto exit;

#ifdef CONFIG_RTW_MESH
	if (MLME_IS_MESH(adapter)) {
		/* QoS is mandatory in mesh */
		sta->flags |= WLAN_STA_WME;
	}
#endif

	p = rtw_get_ie_ex(tlv_ies, tlv_ies_len, WLAN_EID_VENDOR_SPECIFIC, WMM_IE, 6, NULL, NULL);
	if (!p)
		goto exit;

	sta->flags |= WLAN_STA_WME;
	sta->qos_option = 1;
	sta->qos_info = *(p + 8);
	sta->max_sp_len = (sta->qos_info >> 5) & 0x3;

	if (mlme->qospriv.uapsd_enable && (sta->qos_info & 0xf)) {
		if (sta->qos_info & BIT0)
			sta->uapsd_bitmap |= BIT(TXQ_VO);
		if (sta->qos_info & BIT1)
			sta->uapsd_bitmap |= BIT(TXQ_VI);
		if (sta->qos_info & BIT2)
			sta->uapsd_bitmap |= BIT(TXQ_BK);
		if (sta->qos_info & BIT3)
			sta->uapsd_bitmap |= BIT(TXQ_BE);
	}

exit:
	return;
}

void rtw_ap_parse_sta_ht_ie(_adapter *adapter, struct sta_info *sta, struct rtw_ieee802_11_elems *elems)
{
	struct mlme_priv *mlme = &adapter->mlmepriv;

	sta->flags &= ~WLAN_STA_HT;

#ifdef CONFIG_80211N_HT
	if (mlme->htpriv.ht_option == _FALSE)
		goto exit;

	/* save HT capabilities in the sta object */
	_rtw_memset(&sta->htpriv.ht_cap, 0, sizeof(struct rtw_ieee80211_ht_cap));
	if (elems->ht_capabilities && elems->ht_capabilities_len >= sizeof(struct rtw_ieee80211_ht_cap)) {
		sta->flags |= WLAN_STA_HT;
		sta->flags |= WLAN_STA_WME;
		_rtw_memcpy(&sta->htpriv.ht_cap, elems->ht_capabilities, sizeof(struct rtw_ieee80211_ht_cap));

		if (elems->ht_operation && elems->ht_operation_len == HT_OP_IE_LEN) {
			_rtw_memcpy(sta->htpriv.ht_op, elems->ht_operation, HT_OP_IE_LEN);
			sta->htpriv.op_present = 1;
		}
	}
exit:
#endif

	return;
}

void rtw_ap_parse_sta_vht_ie(_adapter *adapter, struct sta_info *sta, struct rtw_ieee802_11_elems *elems)
{
	struct mlme_priv *mlme = &adapter->mlmepriv;

	sta->flags &= ~WLAN_STA_VHT;

#ifdef CONFIG_80211AC_VHT
	if (mlme->vhtpriv.vht_option == _FALSE)
		goto exit;

	_rtw_memset(&sta->vhtpriv, 0, sizeof(struct vht_priv));
	if (elems->vht_capabilities && elems->vht_capabilities_len == VHT_CAP_IE_LEN) {
		sta->flags |= WLAN_STA_VHT;
		_rtw_memcpy(sta->vhtpriv.vht_cap, elems->vht_capabilities, VHT_CAP_IE_LEN);

		if (elems->vht_operation && elems->vht_operation_len== VHT_OP_IE_LEN) {
			_rtw_memcpy(sta->vhtpriv.vht_op, elems->vht_operation, VHT_OP_IE_LEN);
			sta->vhtpriv.op_present = 1;
		}

		if (elems->vht_op_mode_notify && elems->vht_op_mode_notify_len == 1) {
			_rtw_memcpy(&sta->vhtpriv.vht_op_mode_notify, elems->vht_op_mode_notify, 1);
			sta->vhtpriv.notify_present = 1;
		}
	}
exit:
#endif

	return;
}

void rtw_ap_parse_sta_he_ie(_adapter *adapter, struct sta_info *sta, struct rtw_ieee802_11_elems *elems)
{
	struct mlme_priv *mlme = &adapter->mlmepriv;

	sta->flags &= ~WLAN_STA_HE;

#ifdef CONFIG_80211AX_HE
	if (mlme->hepriv.he_option == _FALSE)
		goto exit;

	_rtw_memset(&sta->hepriv, 0, sizeof(struct he_priv));
	if (elems->he_capabilities && (elems->he_capabilities_len <= HE_CAP_ELE_MAX_LEN)) {
		sta->flags |= WLAN_STA_HE;
		_rtw_memcpy(sta->hepriv.he_cap, elems->he_capabilities, elems->he_capabilities_len);

		if (elems->he_operation && (elems->he_operation_len <= HE_OPER_ELE_MAX_LEN)) {
			_rtw_memcpy(sta->hepriv.he_op, elems->he_operation, elems->he_operation_len);
			sta->hepriv.op_present = 1;
		}

#if 0
		if (elems->vht_op_mode_notify && elems->vht_op_mode_notify_len == 1) {
			_rtw_memcpy(&sta->vhtpriv.vht_op_mode_notify, elems->vht_op_mode_notify, 1);
			sta->vhtpriv.notify_present = 1;
		}
#endif
	}
exit:
#endif

	return;
}

#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
void rtw_issue_action_token_req(_adapter *padapter, struct sta_info *pstat)
{
	/* Token Request Format
	 	Category code :		1 Byte
		Action code : 		1 Byte
		Element field: 		4 Bytes, the duration of data transmission requested for the station.
	*/

	u8 val = 0x0;
	u8 category = RTW_WLAN_CATEGORY_TBTX;
	u32 tbtx_duration = TBTX_TX_DURATION*1000;
	u8 *pframe;
	unsigned short *fctrl;
	struct xmit_frame		*pmgntframe;
	struct pkt_attrib		*pattrib;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	struct xmit_priv *pxmitpriv = &(padapter->xmitpriv);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork = &(pmlmeinfo->network);


	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		return;

	RTW_DBG("%s: %6ph\n", __FUNCTION__, pstat->phl_sta->mac_addr);
	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgnt_tx_rate(padapter, IEEE80211_OFDM_RATE_24MB); // issue action request using OFDM rate? 20190320 Bruce add
	if (update_mgntframe_attrib(padapter, pattrib) != _SUCCESS) {
		rtw_free_xmitframe(&padapter->xmitpriv, pmgntframe);
		return;
	}

	_rtw_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_rtw_memcpy((void *)GetAddr1Ptr(pwlanhdr), pstat->phl_sta->mac_addr, ETH_ALEN);
	_rtw_memcpy((void *)get_addr2_ptr(pwlanhdr), adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy((void *)GetAddr3Ptr(pwlanhdr), get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);


	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(val), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 4, (unsigned char *)&(tbtx_duration), &(pattrib->pktlen));

	pattrib->last_txcmdsz = pattrib->pktlen;
	padapter->stapriv.last_token_holder = pstat;
	dump_mgntframe(padapter, pmgntframe);

}
#endif	/* CONFIG_RTW_TOKEN_BASED_XMIT */

void rtw_ap_set_sta_wmode(_adapter *padapter, struct sta_info *sta)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX *pcur_network = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;
	enum wlan_mode network_type = WLAN_MD_INVALID;

#ifdef CONFIG_80211AX_HE
	if (sta->hepriv.he_option == _TRUE)
		network_type = WLAN_MD_11AX;
#endif
#ifdef CONFIG_80211AC_VHT
	if (network_type == WLAN_MD_INVALID) {
		if (sta->vhtpriv.vht_option == _TRUE)
			network_type = WLAN_MD_11AC;
	}
#endif
#ifdef CONFIG_80211N_HT
	if (network_type == WLAN_MD_INVALID) {
		if (sta->htpriv.ht_option == _TRUE)
			network_type =  WLAN_MD_11N;
	}
#endif

	if (pcur_network->Configuration.DSConfig > 14)
		network_type |= WLAN_MD_11A;
	else {
		if ((cckratesonly_included(sta->bssrateset, sta->bssratelen)) == _TRUE || padapter->mlmeextpriv.cur_wireless_mode == WLAN_MD_11B)
			network_type |= WLAN_MD_11B;
		else if ((cckrates_included(sta->bssrateset, sta->bssratelen)) == _TRUE)
			network_type |= WLAN_MD_11BG;
		else
			network_type |= WLAN_MD_11G;
	}

	sta->phl_sta->wmode = network_type;
}

#if defined(CONFIG_RTW_ACS) && defined(WKARD_ACS)
void rtw_acs_start(_adapter *padapter)
{
	RTW_INFO(FUNC_ADPT_FMT" not support\n", FUNC_ADPT_ARG(padapter));
}

void rtw_acs_stop(_adapter *padapter)
{
	RTW_INFO(FUNC_ADPT_FMT" not support\n", FUNC_ADPT_ARG(padapter));
}
#endif /* defined(CONFIG_RTW_ACS) && defined(WKARD_ACS) */

#ifdef AP_NEIGHBOR_INFO
#define MAC_NOT_FOUND -1
#define MAC_INVALID -2

static int ap_neighbor_lookup(_adapter *adapter, unsigned char *macaddr)
{
	int i;


	if(macaddr == NULL)
		return MAC_INVALID;

	for(i=0; i<MAX_AP_NEIGHBOR_INFO_NUM; i++)
	{
		if(adapter->dvobj->ap_neighbor.ap_neighbor_info_ent[i].valid == 1)
		{
			if(_rtw_memcmp(adapter->dvobj->ap_neighbor.ap_neighbor_info_ent[i].mac, macaddr, MAC_ADDR_LEN))
				return i;	//found, return index
			else
				continue;
		}
		else
		{
			break;
		}
	}

	return MAC_NOT_FOUND;		//not found
}


void rtw_add_ap_neighbor_info(_adapter *adapter, union recv_frame *precv_frame)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter_to_dvobj(adapter));
	struct beacon_keys recv_bcn;

	int search_result = 0, index = 0, i, tmp_index = 0;
	unsigned long max=0, sec=0;
	unsigned char mac[MAC_ADDR_LEN] = {0};
	WLAN_BSSID_EX pbss;
	u8 cur_channel,cur_bwmode,cur_ch_offset;
	enum band_type cur_band_type;

	u8  *pframe_data = NULL;
	u32  len = 0;

	if(precv_frame == NULL)
		return;

	len = precv_frame->u.hdr.len;
	pframe_data = precv_frame->u.hdr.rx_data;

	_rtw_memset(&pbss, 0, sizeof(WLAN_BSSID_EX));

	if (collect_bss_info_g6(adapter, precv_frame, &pbss) == _FAIL){
		RTW_ERR("[%s]collect_bss_info_g6 = fail \n",__FUNCTION__);
		return;
	}

	if(pbss.PhyInfo.SignalStrength <= 0)
	{
		return;
	}

	if (rtw_get_bcn_keys(adapter, pframe_data, len, &recv_bcn) == _FAIL){
		RTW_ERR("[%s]rtw_get_bcn_keys = fail \n",__FUNCTION__);
		return;
	}

	_rtw_memcpy(mac, pbss.MacAddress, MAC_ADDR_LEN);
	rtw_bss_get_chbw(&pbss, &cur_channel, &cur_bwmode, &cur_ch_offset, 1, 1);
	rtw_adjust_chbw(adapter, cur_channel, &cur_bwmode,&cur_ch_offset);

	search_result = ap_neighbor_lookup(adapter, mac);

	if(search_result>=0) // found, update entry
	{
		index = search_result;

	}else if(search_result==MAC_NOT_FOUND){ //not in the list, insert

		if(dvobj->ap_neighbor.sta_entry_num==MAX_AP_NEIGHBOR_INFO_NUM )
		{
			for(i=0; i<MAX_AP_NEIGHBOR_INFO_NUM; ++i)
			{
				if(jiffies/HZ >= dvobj->ap_neighbor.ap_neighbor_info_ent[i].sec)
					sec = jiffies/HZ - dvobj->ap_neighbor.ap_neighbor_info_ent[i].sec;
				else
					sec = jiffies/HZ + ~(unsigned long)0/HZ - dvobj->ap_neighbor.ap_neighbor_info_ent[i].sec;

				if( sec>=max ){
					max = sec;
					tmp_index = i;
				}
			}
			index = tmp_index;
		}else {
			index = dvobj->ap_neighbor.sta_entry_num;
			dvobj->ap_neighbor.sta_entry_num++;
		}

		dvobj->ap_neighbor.ap_neighbor_info_ent[index].valid = 1;
		_rtw_memcpy(dvobj->ap_neighbor.ap_neighbor_info_ent[index].mac, pbss.MacAddress, MAC_ADDR_LEN);

		cur_band_type = (cur_channel > 14) ? BAND_ON_5G : BAND_ON_24G;
		dvobj->ap_neighbor.ap_neighbor_info_ent[index].band = cur_band_type;

	}

	if (strlen(pbss.Ssid.Ssid) <= WLAN_SSID_MAXLEN)	{
		_rtw_memcpy(dvobj->ap_neighbor.ap_neighbor_info_ent[index].ssid, pbss.Ssid.Ssid, strlen(pbss.Ssid.Ssid));
		dvobj->ap_neighbor.ap_neighbor_info_ent[index].ssid[strlen(pbss.Ssid.Ssid)] = '\0';
	} else {
		_rtw_memcpy(dvobj->ap_neighbor.ap_neighbor_info_ent[index].ssid, pbss.Ssid.Ssid, WLAN_SSID_MAXLEN);
		dvobj->ap_neighbor.ap_neighbor_info_ent[index].ssid[WLAN_SSID_MAXLEN] = '\0';
	}
	dvobj->ap_neighbor.ap_neighbor_info_ent[index].channel = cur_channel;
	dvobj->ap_neighbor.ap_neighbor_info_ent[index].rssi = ((u32)(pbss.PhyInfo.SignalStrength) + (u32)(pbss.PhyInfo.SignalStrength)*4)/5;
	dvobj->ap_neighbor.ap_neighbor_info_ent[index].standard = recv_bcn.proto_cap;
	dvobj->ap_neighbor.ap_neighbor_info_ent[index].sec = jiffies/HZ;
	dvobj->ap_neighbor.ap_neighbor_info_ent[index].networktype = pbss.InfrastructureMode;
	dvobj->ap_neighbor.ap_neighbor_info_ent[index].bandwidth= cur_bwmode;

	return;
}
#endif

#endif	/* CONFIG_AP_MODE */

