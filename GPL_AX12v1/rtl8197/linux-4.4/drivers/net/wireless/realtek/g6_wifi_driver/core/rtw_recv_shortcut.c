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

#include <drv_types.h>

#ifdef CONFIG_RTW_CORE_RXSC
#define is_equal_mac_addr(__l, __r) \
	({ \
		const unsigned char *l = (const unsigned char *)(__l); \
		const unsigned char *r = (const unsigned char *)(__r); \
		((l[5] == r[5]) && (l[4] == r[4]) && (l[3] == r[3]) && \
		 (l[2] == r[2]) && (l[1] == r[1]) && (l[0] == r[0])); \
	})

u8 core_rxsc_alloc_check(_adapter *adapter, union recv_frame *prframe)
{
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;

	if (!adapter->enable_rxsc)
		return _FAIL;

	if (!pattrib->bdecrypted ||
		pattrib->mfrag /*||
		pattrib->to_fr_ds == 3*/) /* A4_RXSC */
		return _FAIL;

	if (!prframe->u.hdr.psta)
		return _FAIL;

	if (IS_MCAST(pattrib->ra) || IS_MCAST(pattrib->dst))
		return _FAIL;

#ifdef CONFIG_RTW_MESH
	if (pattrib->mesh_ctrl_present)
		return _FAIL;
#endif

#ifdef CONFIG_WAPI_SUPPORT
	if (prframe->u.hdr.bIsWaiPacket)
		return _FAIL;
#endif

#ifdef CORE_RXSC_AMSDU
	if (pattrib->amsdu)
		return _SUCCESS;
#else
	if (pattrib->amsdu)
		return _FAIL;
#endif

	if (!pattrib->bsnaphdr)
		return _FAIL;

	if (pattrib->eth_type == ETH_P_ARP ||
		pattrib->eth_type == 0x888e ||
		pattrib->eth_type == 0x8899
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
		|| pattrib->eth_type == ETH_P_WAPI
#endif
		)
		return _FAIL;

	return _SUCCESS;
}

struct core_rxsc_entry *core_rxsc_alloc_entry(_adapter *adapter, union recv_frame *prframe)
{
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	struct sta_info *psta = prframe->u.hdr.psta;
	struct rtw_recv_pkt *rx_req = prframe->u.hdr.rx_req;
	struct core_rxsc_entry *rxsc_entry = NULL;
	u8 rxsc_idx, cnt = 0;

	if (core_rxsc_alloc_check(adapter, prframe)!=_SUCCESS)
		return NULL;

	rxsc_idx = psta->rxsc_idx_new;

	for (cnt=0; cnt<NUM_RXSC_ENTRY; cnt++) {
		if (psta->rxsc_entry[rxsc_idx].status != RXSC_ENTRY_APPLYING)
			break;
		rxsc_idx = (rxsc_idx +1) % NUM_RXSC_ENTRY;
	}
	if (NUM_RXSC_ENTRY == cnt)
		return NULL;

	psta->rxsc_idx_new = (rxsc_idx + 1) % NUM_RXSC_ENTRY;
	rxsc_entry = &psta->rxsc_entry[rxsc_idx];
	rxsc_entry->forward_to = RXSC_FWD_UNKNOWN;
	rxsc_entry->status = RXSC_ENTRY_CREATING;

	/* A4_RXSC */
	if (3 == pattrib->to_fr_ds)
		rxsc_entry->is_a4 = 1;
	else
		rxsc_entry->is_a4 = 0;

	/* +HTC */
	if (rx_req->mdata.htc)
		rxsc_entry->is_htc = 1;
	else
		rxsc_entry->is_htc = 0;

	/* reset hit counter */
	rxsc_entry->hit = 0;

	prframe->u.hdr.rxsc_entry = rxsc_entry;

	if (pattrib->amsdu)
		adapter->core_logs.rxsc_alloc_entry[1]++;
	else
		adapter->core_logs.rxsc_alloc_entry[0]++;

	return rxsc_entry;
}

sint core_rxsc_get_entry(_adapter *adapter, union recv_frame *prframe, u8 is_amsdu)
{
	struct sta_info *psta = NULL;
	u8 *pframe = prframe->u.hdr.rx_data;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *whdr = prframe->u.hdr.wlan_hdr;
	#else
	u8 *whdr = pframe;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT*/
	u8 *lhs, *rhs = NULL;
	u8 idx = 0;
	u8 is_fit_rxsc = 0;
	struct core_rxsc_entry *rxsc_entry = NULL;
	_adapter *pri_adapter = adapter;
	_adapter *cur_adapter = adapter;
	struct rtw_recv_pkt	*rx_req = prframe->u.hdr.rx_req;
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	struct sta_priv *psta_priv = NULL;
	u8 ret = _FAIL;
	u8 *pda;
	int rxsc_payload_offset;

	/* get actual rx interface adapter */
	if (rx_req->rx_role && rx_req->rx_role->core_data) {
		cur_adapter = (_adapter *)rx_req->rx_role->core_data;
	} else {
		pda = get_ra(whdr);
		cur_adapter = rtw_get_iface_by_macddr(pri_adapter, pda);
		if (cur_adapter == NULL)
			cur_adapter = pri_adapter;
	}
	psta_priv = &cur_adapter->stapriv;
	adapter = cur_adapter;

	/* cached sta move to rtw_get_stainfo */
	psta = rtw_get_stainfo(psta_priv, get_addr2_ptr(whdr));

	if (!psta)
		return ret;

#ifdef GBWC
	if (((adapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_MAC_INNER) && psta->GBWC_in_group)
		|| ((adapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_MAC_OUTTER) && !psta->GBWC_in_group)
		|| (adapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_IF_RX)
		|| (adapter->registrypriv.wifi_mib.gbwcmode == GBWC_MODE_LIMIT_IF_TRX)) {
		return ret;
	}
#endif /* GBWC */

	lhs = GetAddr1Ptr(whdr);

	for (idx=0; idx < NUM_RXSC_ENTRY; idx++) {
		rxsc_entry = &psta->rxsc_entry[(psta->rxsc_idx_cached+idx)%NUM_RXSC_ENTRY];
		rxsc_payload_offset = rxsc_entry->rxsc_payload_offset + (rx_req->mdata.htc ? 4 : 0);

		if (RXSC_ENTRY_VALID != rxsc_entry->status)
			continue;

		if (is_amsdu != rxsc_entry->is_amsdu) {
			//DBGP("is_amsdu: pkt=%d rxsc=%d \n", is_amsdu, rxsc_entry->is_amsdu);
			continue;
		}

		rhs = (u8*)(&(rxsc_entry->rxsc_wlanhdr)) +
				offsetof(struct rxsc_wlan_hdr, addr1);
#if 0
		if (0)
		{
			DBGP("(A1)lhs=%pM rhs=%pM isEqual=%d %d\n", lhs, rhs, is_equal_mac_addr(lhs,rhs), is_equal_mac_addr(lhs+1,rhs));
			DBGP("(A2)lhs=%pM rhs=%pM\n", lhs+6, rhs+6);
			DBGP("(A3)lhs=%pM rhs=%pM\n", lhs+12, rhs+12);
			DBGP("rxsc_payload_offset=%d \n", rxsc_entry->rxsc_payload_offset);
			DBGP("(LLC)lhs=%pM rhs=%pM\n",
				&pframe[rxsc_entry->rxsc_payload_offset+(sizeof(struct ethhdr))-SNAP_SIZE-2],  &rtw_rfc1042_header);
			DBGP("(eth_type)lhs=0x%x rhs=0x%x\n",
				*(unsigned short *)(&pframe[rxsc_entry->rxsc_payload_offset+(ETH_ALEN*2)]), *(unsigned short *)(&rxsc_entry->rxsc_ethhdr.h_proto));

			DBGP("%d %d %d %d %d \n",
				is_equal_mac_addr(lhs,rhs),
				is_equal_mac_addr(lhs+6,rhs+6),
				is_equal_mac_addr(lhs+12,rhs+12),
				!memcmp(&pframe[rxsc_entry->rxsc_payload_offset+(sizeof(struct ethhdr))-SNAP_SIZE-2], &rtw_rfc1042_header, ETH_ALEN),
				(*(unsigned short *)(&pframe[rxsc_entry->rxsc_payload_offset+(ETH_ALEN*2)]) == *(unsigned short *)(&rxsc_entry->rxsc_ethhdr.h_proto))
				);
		}
#endif

#ifdef CORE_RXSC_AMSDU
		if (is_amsdu &&
				is_equal_mac_addr(lhs,rhs) &&
				is_equal_mac_addr(lhs + 6, rhs + 6) &&
				is_equal_mac_addr(lhs + 12, rhs + 12))
			is_fit_rxsc = 1;
		else
#endif
		if (!is_amsdu &&
				is_equal_mac_addr(lhs,rhs) &&
				is_equal_mac_addr(lhs + 6, rhs + 6) &&
				is_equal_mac_addr(lhs + 12, rhs + 12) &&
				!memcmp(&pframe[rxsc_payload_offset+(sizeof(struct ethhdr)) - SNAP_SIZE - 2], &rtw_rfc1042_header, ETH_ALEN) &&
				(*(unsigned short *)(&pframe[rxsc_payload_offset+(ETH_ALEN * 2)]) == *(unsigned short *)(&rxsc_entry->rxsc_ethhdr.h_proto)))
			is_fit_rxsc = 1;


		/* A4_RXSC */
#ifdef CONFIG_RTW_A4_STA
		if (rxsc_entry->is_a4
				&& !rxsc_entry->is_amsdu
				&& memcmp(rxsc_entry->rxsc_ethhdr.h_source, GetAddr4Ptr(pframe), ETH_ALEN))
			is_fit_rxsc = 0;
#endif

		if (is_fit_rxsc) {
			prframe->u.hdr.psta = psta;
			prframe->u.hdr.rxsc_entry = rxsc_entry;
			prframe->u.hdr.adapter = rxsc_entry->adapter;
			psta->rxsc_idx_cached = idx;
			rxsc_entry->hit++;
			ret = _SUCCESS;
			break;
		}
	}

	if (is_fit_rxsc)
		adapter->core_logs.rxsc_entry_hit[1]++;
	else
		adapter->core_logs.rxsc_entry_hit[0]++;

	return ret;
}

s32 core_rxsc_apply_check(_adapter *adapter, union recv_frame *prframe, void *rx_req)
{
	struct rtw_recv_pkt *phlrx = (struct rtw_recv_pkt *)rx_req;
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	struct core_rxsc_entry *rxsc_entry = NULL;
	u8 *whdr = NULL;
	#ifdef SBWC//sbwc bypass rxsc
	struct sta_info *psta = NULL;
	#endif

#ifdef CORE_RXSC_RFRAME
	adapter->cached_prframe = NULL;
#endif

	prframe->u.hdr.pkt = NULL;
	prframe->u.hdr.rxsc_entry = NULL;

	if (!adapter->enable_rxsc)
		return CORE_RX_CONTINUE;

	if (phlrx->mdata.bc || phlrx->mdata.mc)
		return CORE_RX_CONTINUE;

	if (phlrx->mdata.more_frag || phlrx->mdata.frag_num)
		return CORE_RX_CONTINUE;

	if (phlrx->mdata.crc32)
		return CORE_RX_CONTINUE;

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	if (phlrx->os_priv) {
		prframe->u.hdr.pkt = phlrx->rx_buf->os_priv;
	#else
	if (phlrx->os_priv) {
		prframe->u.hdr.pkt = phlrx->os_priv; /*skb*/
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
		core_update_recvframe_pkt(prframe, rx_req);
	} else {
		return CORE_RX_CONTINUE;
	}

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	whdr = prframe->u.hdr.wlan_hdr;
	#else
	whdr = prframe->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	if ((GetFrameType(whdr) != WIFI_DATA_TYPE) ||
		(get_frame_sub_type(whdr) & BIT(6)))
		return CORE_RX_CONTINUE;

#ifdef SBWC//sbwc bypass rxsc
	if (phlrx->tx_sta)
		psta = (struct sta_info *)phlrx->tx_sta->core_data;
	if (psta == NULL)
		psta = rtw_get_stainfo(&adapter->stapriv,
				       get_addr2_ptr(whdr));
	if (psta && (psta->SBWC_mode & SBWC_MODE_LIMIT_STA_RX))
		return CORE_RX_CONTINUE;
#endif /* SBWC */


	prframe->u.hdr.rx_req = rx_req;

	if (core_rxsc_get_entry(adapter, prframe, phlrx->mdata.amsdu) != _SUCCESS ||
			prframe->u.hdr.rxsc_entry == NULL)
		return CORE_RX_CONTINUE;

	rxsc_entry = prframe->u.hdr.rxsc_entry;

	rxsc_entry->status = RXSC_ENTRY_APPLYING;
	rxsc_entry->rxsc_attrib.data_rate = phlrx->mdata.rx_rate;
	rxsc_entry->rxsc_attrib.bw = phlrx->mdata.bw;
	wmb();
	_rtw_memcpy(pattrib, &rxsc_entry->rxsc_attrib,
	            sizeof(rxsc_entry->rxsc_attrib));
	wmb();
	if (phlrx->mdata.htc)
		pattrib->hdrlen += 4;
	rxsc_entry->status = RXSC_ENTRY_VALID;

#ifdef CORE_RXSC_RFRAME
	adapter->cached_prframe = prframe;
#endif

	return CORE_RX_GO_SHORTCUT;
}

s32 core_rxsc_apply_shortcut(_adapter *adapter, union recv_frame *prframe)
{
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	struct core_rxsc_entry *rxsc_entry = prframe->u.hdr.rxsc_entry;
	struct rtw_recv_pkt *phlrx = (struct rtw_recv_pkt *)prframe->u.hdr.rx_req;
	struct sta_info *psta = prframe->u.hdr.psta;
	struct recv_priv *precvpriv = &(adapter->recvpriv);
#if defined(CORE_RXSC_RFRAME) || defined(DEBUG_CORE_RX)
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
#endif
	_queue *pfree_recv_queue = &(precvpriv->free_recv_queue);
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *pframe = prframe->u.hdr.wlan_hdr;
	#else
	u8 *pframe = prframe->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	u8 *ptr = NULL;
	s32 status = CORE_RX_DONE;
#ifdef CONFIG_BR_EXT
	void *br_port = NULL;
#endif
#ifdef CONFIG_RTW_A4_STA
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	struct ethhdr *ehdr;
#endif
#ifdef CONFIG_RTL_VLAN_8021Q
	extern int linux_vlan_enable;
#endif
	extern u8 DBG_PRINT_RCV_ONCE;

	pattrib->order = GetOrder(pframe);
	pattrib->privacy = GetPrivacy(pframe);
	if (pattrib->privacy) {
		if (!pattrib->encrypt) {
			RTW_WARN("RX drop: encrypted packet!\n");
			status = CORE_RX_DROP;
			goto exit_shortcut;
		}
	} else if (pattrib->encrypt) {
		RTW_WARN("RX drop: unencrypted packet!\n");
		status = CORE_RX_DROP;
		goto exit_shortcut;
	}

/* Shortcut of validate_recv_frame */
	if (pattrib->qos) {
		/* A4_RXSC */
		if (3 == pattrib->to_fr_ds)
			pattrib->priority = GetPriority((pframe + WLAN_HDR_A4_LEN));
		else
			pattrib->priority = GetPriority((pframe + WLAN_HDR_A3_LEN));
	} else
		pattrib->priority = 0;

	/* recv_ucast_pn_decache is unavailable. */
	/* recv_bcast_pn_decache is available only for mcast. */

	process_pwrbit_data(adapter, prframe, psta);

	if ((get_frame_sub_type(pframe) & WIFI_QOS_DATA_TYPE) == WIFI_QOS_DATA_TYPE) {
		process_wmmps_data(adapter, prframe, psta);
#ifdef CONFIG_RTW_TWT
		rtw_core_twt_inform_announce(adapter, psta);
#endif
	}
	if (pattrib->qos && pattrib->order)
		process_a_control_subfield(adapter, prframe);

#ifdef RTW_CORE_PKT_TRACE
	if(adapter->pkt_trace_enable)
	{
		rtw_wifi_rx_prepare_pkt_trace(adapter,prframe);
		RTW_RX_TRACE(adapter,&pattrib->pktinfo);
	}
#endif

/* Shortcut of rtw_core_rx_data_pre_process */
	count_rx_stats(adapter, prframe, NULL);

/* Shortcut of rtw_core_rx_data_post_process */
#ifdef CORE_RXSC_AMSDU
/* amsdu */
	if (rxsc_entry->is_amsdu) {
		status = core_rx_process_amsdu(adapter, prframe);
		goto exit_shortcut;
	}
#endif

/* 802.11 -> 802.3 */
	if (rxsc_entry->rxsc_trim_pad)
		recvframe_pull_tail(prframe, rxsc_entry->rxsc_trim_pad);

	ptr = recvframe_pull(prframe, rxsc_entry->rxsc_payload_offset + (phlrx->mdata.htc ? 4 : 0));
	_rtw_memcpy(ptr, (u8 *)(&rxsc_entry->rxsc_ethhdr), sizeof(rxsc_entry->rxsc_ethhdr));
	rtw_rframe_set_os_pkt(prframe);

/* ampdu debug */

#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
	if(DBG_PRINT_RCV_ONCE) {
		RTW_PRINT("[%s:%d]\n[%d] forward_to:%d, is_amsdu:%d, status:%d\n",
			__func__, __LINE__, DBG_PRINT_RCV_ONCE,
			rxsc_entry->forward_to, rxsc_entry->is_amsdu, rxsc_entry->status);

		if (prframe->u.hdr.pkt) {
			RTW_PRINT("[%d] hdr_len=%d\n", DBG_PRINT_RCV_ONCE, prframe->u.hdr.len);
			txsc_dump_data((u8 *)prframe->u.hdr.pkt->data, 14, "pkt->data");
		}
		DBG_PRINT_RCV_ONCE--;
	}
#endif

/* Packet forward to OS */
	if (RXSC_FWD_KERNEL == rxsc_entry->forward_to
	    && (MLME_IS_AP(adapter) && prframe->u.hdr.pkt
		&& is_equal_mac_addr(prframe->u.hdr.pkt->data,
		                     adapter_mac_addr(adapter)))) {
		struct sk_buff *pkt = prframe->u.hdr.pkt;

#ifdef CONFIG_RTL_VLAN_8021Q
		if (linux_vlan_enable)
			rtw_linux_vlan_rx_process(adapter, pkt);
#endif

#ifdef CONFIG_BR_EXT
#ifdef CONFIG_RTW_A4_STA /* what if STA TX A3+A4 mixed ?? */
		if (3 != prframe->u.hdr.attrib.to_fr_ds)
#endif
			if (MLME_IS_STA(adapter) || MLME_IS_ADHOC(adapter)) {
#ifdef CONFIG_RTW_A4_STA
				if (!((adapter->a4_enable == 1) && (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)))
#endif
				{
				/* Insert NAT2.5 RX here! */
				#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 35))
					br_port = adapter->pnetdev->br_port;
				#else
					rcu_read_lock();
					br_port = rcu_dereference(adapter->pnetdev->rx_handler_data);
					rcu_read_unlock();
				#endif

					if (br_port) {
						int nat25_handle_frame(_adapter *priv, struct sk_buff *skb);
						if (nat25_handle_frame(adapter, pkt) == -1) {
							/* bypass this frame to upper layer!! */
						}
					}
				}
			}

#ifdef CONFIG_RTW_A4_STA /* what if STA TX A3+A4 mixed ?? */
		if (MLME_IS_STA(adapter)) {
			if(adapter->registrypriv.wifi_mib.a4_enable == 1) {
				if (pkt) {
					ehdr = (struct ethhdr *)pkt->data;
					if(_rtw_memcmp(ehdr->h_source, adapter->br_mac, MAC_ADDR_LEN) == _TRUE) {
#ifdef RTW_CORE_PKT_TRACE
						if(adapter->pkt_trace_enable)
							RTW_RX_TRACE(adapter,&pattrib->pktinfo);
#endif
						status = CORE_RX_DROP;
						goto exit_shortcut;
					}
				}
			}
		}
#endif
#endif /* CONFIG_BR_EXT */
		rtw_process_u2mc(adapter, pkt);

		pkt->protocol = eth_type_trans(pkt, adapter->pnetdev);
		pkt->dev = adapter->pnetdev;
		pkt->ip_summed = CHECKSUM_NONE; /* CONFIG_TCP_CSUM_OFFLOAD_RX */
#ifdef RTW_CORE_PKT_TRACE
		if(adapter->pkt_trace_enable)
			RTW_RX_TRACE(adapter,&pattrib->pktinfo);
#endif
#ifdef DEBUG_CORE_RX
		if (dvobj->cnt_rx_pktsz == pkt->len+ETH_HLEN+4)
			dvobj->num_rx_pktsz_os++;
		dvobj->total_rx_pkt_os++;
#endif
#ifdef CONFIG_SMP_NETIF_RX
		rtw_netif_rx_enq(adapter, pkt);
#else
		rtw_netif_rx(adapter->pnetdev, pkt);
#endif
	}
	else{
		if (prframe->u.hdr.pkt){
			rtw_os_recv_indicate_pkt(adapter, prframe->u.hdr.pkt, prframe);
		}
	}

#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
	if(DBG_PRINT_RCV_ONCE) {
		RTW_PRINT("[%s:%d]\n[%d] forward_to:%d, is_amsdu:%d, status:%d\n",
			__func__, __LINE__, DBG_PRINT_RCV_ONCE,
			rxsc_entry->forward_to, rxsc_entry->is_amsdu, rxsc_entry->status);
		DBG_PRINT_RCV_ONCE--;
	}
#endif

	prframe->u.hdr.pkt = NULL;
#ifdef CORE_RXSC_RFRAME
	if (prframe->u.hdr.rx_req)
		rtw_return_phl_rx_pkt(dvobj, (u8*)prframe->u.hdr.rx_req);
#else
	rtw_free_recvframe(prframe, pfree_recv_queue);
#endif

	status = CORE_RX_DONE;

exit_shortcut:

	if (CORE_RX_DONE != status)
		DBGP("error \n");
	else {
#ifdef RTW_PHL_DBG_CMD
		adapter->core_logs.rxCnt_data_shortcut++;
#endif
	}

	/* A4_RXSC */
    #ifdef CONFIG_RTW_A4_STA
	if (3 == pattrib->to_fr_ds && CORE_RX_DONE == status) {
		if (rxsc_entry->is_amsdu)
			adapter->cnt_a4_rxsc_amsdu++;
		else
			adapter->cnt_a4_rxsc++;
	}
	#endif

	return status;
}

void core_rxsc_clear_entry(_adapter *adapter, struct sta_info *psta)
{
	u32 idx = 0;
	for (idx=0; idx<NUM_RXSC_ENTRY; idx++)
		psta->rxsc_entry[idx].status = RXSC_ENTRY_INVALID;
}

void core_rxsc_clean_entry(_adapter *adapter, const u8 *mac_addr, struct sta_info *psta)
{
	struct sta_priv *pstapriv = &(adapter->stapriv);
	struct sta_info *assoc_sta = NULL;
	struct core_rxsc_entry *rxsc_entry = NULL;
	int i, j;

	/* clean associated sta --> psta rxsc entry */
	for (i = 0; i < pstapriv->max_num_sta; i++) {
		assoc_sta = pstapriv->sta_aid[i];
		if (!assoc_sta || !assoc_sta->phl_sta)
			continue;

		for (j = 0; j < NUM_RXSC_ENTRY; j++) {
			rxsc_entry = &(assoc_sta->rxsc_entry[j]);
			if (_rtw_memcmp(rxsc_entry->rxsc_wlanhdr.addr3, mac_addr, 6) == _TRUE) {
				rxsc_entry->status = RXSC_ENTRY_INVALID;
				rxsc_entry->forward_to = RXSC_FWD_UNKNOWN;
				rxsc_entry->rxsc_payload_offset = 0;
				rxsc_entry->rxsc_trim_pad = 0;
				_rtw_memset(&(rxsc_entry->rxsc_wlanhdr), 0, sizeof(struct rxsc_wlan_hdr));
				_rtw_memset(&(rxsc_entry->rxsc_ethhdr), 0, sizeof(struct ethhdr));
				rxsc_entry->is_amsdu = 0;

				RTW_INFO("%s: "MAC_FMT" "MAC_FMT" idx=%d\n",
					__FUNCTION__, MAC_ARG(assoc_sta->phl_sta->mac_addr), MAC_ARG(mac_addr), j);
			}
		}
	}

	/* check if cached */
	if (adapter->cached_sta == psta)
		adapter->cached_sta = NULL;

	/* clean psta rxsc entry */
	for (j = 0; j < NUM_RXSC_ENTRY; j++) {
		rxsc_entry = &(psta->rxsc_entry[j]);
		rxsc_entry->status = RXSC_ENTRY_INVALID;
		rxsc_entry->forward_to = RXSC_FWD_UNKNOWN;
		rxsc_entry->rxsc_payload_offset = 0;
		rxsc_entry->rxsc_trim_pad = 0;
		_rtw_memset(&(rxsc_entry->rxsc_wlanhdr), 0, sizeof(struct rxsc_wlan_hdr));
		_rtw_memset(&(rxsc_entry->rxsc_ethhdr), 0, sizeof(struct ethhdr));
		rxsc_entry->is_amsdu = 0;
	}
	psta->rxsc_idx_new = 0;
	psta->rxsc_idx_cached = 0;

	/* reset subpkt a1 cache */
	#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
	_rtw_memset(psta->subpkt_addr1, 0x0, ETH_ALEN * NUM_RXSC_ENTRY);
	psta->subpkt_cnt = 0;
	psta->subpkt_idx = 0;
	#endif
}

void core_rxsc_init(_adapter *padapter)
{
	padapter->enable_rxsc = 1;
	padapter->cached_sta = NULL;
#ifdef CORE_RXSC_RFRAME
	padapter->cached_prframe = NULL;
#endif

#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX) && defined(CONFIG_RTW_A4_STA)
	/* because nat25, disable bypass_deamsdu in client mode,
	   and also disable bypass_deamsdu in a4 mode */
	if (MLME_IS_STA(padapter) || padapter->a4_enable) {
		padapter->registrypriv.wifi_mib.bypass_deamsdu = 0;
	}
#endif

	RTW_INFO("[RXSC_INIT][%s] rxsc:%d\n", padapter->pnetdev->name, padapter->enable_rxsc);
}

void core_rxsc_deinit(_adapter *padapter)
{
#ifdef CORE_RXSC_RFRAME
	struct recv_priv *precvpriv = &(padapter->recvpriv);
	_queue *pfree_recv_queue = &(precvpriv->free_recv_queue);
	union recv_frame *pcached_rframe = padapter->cached_prframe;
#endif

#ifdef CORE_RXSC_RFRAME
	if(pcached_rframe) {
		RTW_PRINT("[RXSC_DEINIT][%s] rxsc:%d, free cached rframe\n", 
			padapter->pnetdev->name, padapter->enable_rxsc);

		pcached_rframe->u.hdr.rx_req = NULL;
		rtw_free_recvframe(pcached_rframe, pfree_recv_queue);
		padapter->cached_prframe = NULL;
	}
#endif

	return;
}

#endif

