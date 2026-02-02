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
#define _RTW_RECV_C_

#include <drv_types.h>

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
static void rtw_signal_stat_timer_hdl(void *ctx);

enum {
	SIGNAL_STAT_CALC_PROFILE_0 = 0,
	SIGNAL_STAT_CALC_PROFILE_1,
	SIGNAL_STAT_CALC_PROFILE_MAX
};

u8 signal_stat_calc_profile[SIGNAL_STAT_CALC_PROFILE_MAX][2] = {
	{4, 1},	/* Profile 0 => pre_stat : curr_stat = 4 : 1 */
	{3, 7}	/* Profile 1 => pre_stat : curr_stat = 3 : 7 */
};

#ifndef RTW_SIGNAL_STATE_CALC_PROFILE
	#define RTW_SIGNAL_STATE_CALC_PROFILE SIGNAL_STAT_CALC_PROFILE_1
#endif

#endif /* CONFIG_NEW_SIGNAL_STAT_PROCESS */

u8 rtw_bridge_tunnel_header[] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8 };
u8 rtw_rfc1042_header[] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };
static u8 SNAP_ETH_TYPE_IPX[2] = {0x81, 0x37};
static u8 SNAP_ETH_TYPE_APPLETALK_AARP[2] = {0x80, 0xf3};
#ifdef CONFIG_TDLS
static u8 SNAP_ETH_TYPE_TDLS[2] = {0x89, 0x0d};
#endif

void _rtw_init_sta_recv_priv(struct sta_recv_priv *psta_recvpriv)
{

	_rtw_memset((u8 *)psta_recvpriv, 0, sizeof(struct sta_recv_priv));

	_rtw_spinlock_init(&psta_recvpriv->lock);

	/* for(i=0; i<MAX_RX_NUMBLKS; i++) */
	/*	_rtw_init_queue(&psta_recvpriv->blk_strms[i]); */

	_rtw_init_queue(&psta_recvpriv->defrag_q);


}

sint _rtw_init_recv_priv(struct recv_priv *precvpriv, _adapter *padapter)
{
	sint i;

	union recv_frame *precvframe;
	sint	res = _SUCCESS;


	/* We don't need to memset padapter->XXX to zero, because adapter is allocated by rtw_zvmalloc(). */
	/* _rtw_memset((unsigned char *)precvpriv, 0, sizeof (struct  recv_priv)); */

	_rtw_spinlock_init(&precvpriv->lock);

#ifdef CONFIG_RECV_THREAD_MODE
	_rtw_init_sema(&precvpriv->recv_sema, 0);

#endif

	_rtw_init_queue(&precvpriv->free_recv_queue);
	_rtw_init_queue(&precvpriv->recv_pending_queue);
	_rtw_init_queue(&precvpriv->uc_swdec_pending_queue);

	precvpriv->adapter = padapter;

	precvpriv->free_recvframe_cnt = NR_RECVFRAME;

	precvpriv->sink_udpport = 0;
	precvpriv->pre_rtp_rxseq = 0;
	precvpriv->cur_rtp_rxseq = 0;

#ifdef DBG_RX_SIGNAL_DISPLAY_RAW_DATA
	precvpriv->store_law_data_flag = 1;
#else
	precvpriv->store_law_data_flag = 0;
#endif

	rtw_os_recv_resource_init(precvpriv, padapter);

	precvpriv->pallocated_frame_buf = rtw_zvmalloc(NR_RECVFRAME * sizeof(union recv_frame) + RXFRAME_ALIGN_SZ);

	if (precvpriv->pallocated_frame_buf == NULL) {
		res = _FAIL;
		goto exit;
	}
	/* _rtw_memset(precvpriv->pallocated_frame_buf, 0, NR_RECVFRAME * sizeof(union recv_frame) + RXFRAME_ALIGN_SZ); */

	precvpriv->precv_frame_buf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(precvpriv->pallocated_frame_buf), RXFRAME_ALIGN_SZ);
	/* precvpriv->precv_frame_buf = precvpriv->pallocated_frame_buf + RXFRAME_ALIGN_SZ - */
	/*						((SIZE_PTR) (precvpriv->pallocated_frame_buf) &(RXFRAME_ALIGN_SZ-1)); */

	precvframe = (union recv_frame *) precvpriv->precv_frame_buf;


	for (i = 0; i < NR_RECVFRAME ; i++) {
		_rtw_init_listhead(&(precvframe->u.list));

		rtw_list_insert_tail(&(precvframe->u.list), &(precvpriv->free_recv_queue.queue));

		rtw_os_recv_resource_alloc(padapter, precvframe);

		precvframe->u.hdr.len = 0;

		precvframe->u.hdr.adapter = padapter;
		precvframe++;

	}

#ifdef CONFIG_USB_HCI

	ATOMIC_SET(&(precvpriv->rx_pending_cnt), 1);

	_rtw_init_sema(&precvpriv->allrxreturnevt, 0);

#endif
	res = rtw_intf_init_recv_priv(padapter);

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	rtw_init_timer(&precvpriv->signal_stat_timer, rtw_signal_stat_timer_hdl, padapter);

	precvpriv->signal_stat_sampling_interval = 2000; /* ms */
	/* precvpriv->signal_stat_converging_constant = 5000; */ /* ms */

	rtw_set_signal_stat_timer(precvpriv);
#endif /* CONFIG_NEW_SIGNAL_STAT_PROCESS */

exit:


	return res;

}

void rtw_mfree_recv_priv_lock(struct recv_priv *precvpriv)
{
	_rtw_spinlock_free(&precvpriv->lock);
#ifdef CONFIG_RECV_THREAD_MODE
	_rtw_free_sema(&precvpriv->recv_sema);
#endif

	_rtw_spinlock_free(&precvpriv->free_recv_queue.lock);
	_rtw_spinlock_free(&precvpriv->recv_pending_queue.lock);

	_rtw_spinlock_free(&precvpriv->free_recv_buf_queue.lock);

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
	_rtw_spinlock_free(&precvpriv->recv_buf_pending_queue.lock);
#endif /* CONFIG_USE_USB_BUFFER_ALLOC_RX */
}

void _rtw_free_recv_priv(struct recv_priv *precvpriv)
{
	_adapter	*padapter = precvpriv->adapter;


	rtw_free_uc_swdec_pending_queue(padapter);

	rtw_mfree_recv_priv_lock(precvpriv);

	rtw_os_recv_resource_free(precvpriv);

	if (precvpriv->pallocated_frame_buf)
		rtw_vmfree(precvpriv->pallocated_frame_buf, NR_RECVFRAME * sizeof(union recv_frame) + RXFRAME_ALIGN_SZ);

	rtw_intf_free_recv_priv(padapter);


}

bool rtw_rframe_del_wfd_ie(union recv_frame *rframe, u8 ies_offset)
{
#define DBG_RFRAME_DEL_WFD_IE 0
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *whdr = rframe->u.hdr.wlan_hdr;
	#else
	u8 *whdr = rframe->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	u8 *ies = whdr + sizeof(struct rtw_ieee80211_hdr_3addr) + ies_offset;
	uint ies_len_ori = rframe->u.hdr.len - (ies - whdr);
	uint ies_len;

	ies_len = rtw_del_wfd_ie(ies, ies_len_ori, DBG_RFRAME_DEL_WFD_IE ? __func__ : NULL);
	rframe->u.hdr.len -= ies_len_ori - ies_len;

	return ies_len_ori != ies_len;
}

union recv_frame *_rtw_alloc_recvframe(_queue *pfree_recv_queue)
{

	union recv_frame  *precvframe;
	_list	*plist, *phead;
	_adapter *padapter;
	struct recv_priv *precvpriv;

	if (_rtw_queue_empty(pfree_recv_queue) == _TRUE)
		precvframe = NULL;
	else {
		phead = get_list_head(pfree_recv_queue);

		plist = get_next(phead);

		precvframe = LIST_CONTAINOR(plist, union recv_frame, u);

		rtw_list_delete(&precvframe->u.hdr.list);
		padapter = precvframe->u.hdr.adapter;
		if (padapter != NULL) {
			precvpriv = &padapter->recvpriv;
			if (pfree_recv_queue == &precvpriv->free_recv_queue)
				precvpriv->free_recvframe_cnt--;
		}
	}


	return precvframe;

}

union recv_frame *rtw_alloc_recvframe(_queue *pfree_recv_queue)
{
	union recv_frame  *precvframe;

	_rtw_spinlock_bh(&pfree_recv_queue->lock);

	precvframe = _rtw_alloc_recvframe(pfree_recv_queue);

	_rtw_spinunlock_bh(&pfree_recv_queue->lock);

	return precvframe;
}

void rtw_init_recvframe(union recv_frame *precvframe, struct recv_priv *precvpriv)
{
	/* Perry: This can be removed */
	_rtw_init_listhead(&precvframe->u.hdr.list);

	precvframe->u.hdr.len = 0;
}

int rtw_free_recvframe(union recv_frame *precvframe, _queue *pfree_recv_queue)
{
	_adapter *padapter;
	struct dvobj_priv *dvobj;
	struct recv_priv *precvpriv;
	struct rtw_recv_pkt *rx_req;

	if (!precvframe) {
		RTW_ERR("%s precvframe is NULL\n", __func__);
		rtw_warn_on(1);
		return _FAIL;
	}

	padapter = precvframe->u.hdr.adapter;
	precvpriv = &padapter->recvpriv;
	rx_req = precvframe->u.hdr.rx_req;
	dvobj = adapter_to_dvobj(padapter);

#ifdef RTW_PHL_DBG_CMD
	core_add_record(padapter, padapter->record_enable, REC_RX_PHL_RCC, precvframe->u.hdr.rx_req);
#endif

	if (   rx_req
#ifdef CONFIG_CONCURRENT_MODE
	    && (precvframe->u.hdr.cloned == 0)
#endif /* CONFIG_CONCURRENT_MODE */
	   ) {
		rtw_return_phl_rx_pkt(dvobj, (u8*)rx_req);
	}

#ifdef CONFIG_CONCURRENT_MODE
	padapter = GET_PRIMARY_ADAPTER(padapter);
	precvpriv = &padapter->recvpriv;
	pfree_recv_queue = &precvpriv->free_recv_queue;
	precvframe->u.hdr.adapter = padapter;
#endif

	rtw_os_free_recvframe(precvframe);

	_rtw_spinlock_bh(&pfree_recv_queue->lock);

	rtw_list_delete(&(precvframe->u.hdr.list));

	precvframe->u.hdr.len = 0;
	precvframe->u.hdr.attrib.phy_info.physts_rpt_valid = _FALSE;
	precvframe->u.hdr.rx_req = NULL;
#ifdef CONFIG_CONCURRENT_MODE
	precvframe->u.hdr.cloned = 0;
#endif /* CONFIG_CONCURRENT_MODE */

	rtw_list_insert_tail(&(precvframe->u.hdr.list), get_list_head(pfree_recv_queue));

	if (padapter != NULL) {
		if (pfree_recv_queue == &precvpriv->free_recv_queue)
			precvpriv->free_recvframe_cnt++;
	}

	_rtw_spinunlock_bh(&pfree_recv_queue->lock);


	return _SUCCESS;
}


sint _rtw_enqueue_recvframe(union recv_frame *precvframe, _queue *queue)
{

	_adapter *padapter = precvframe->u.hdr.adapter;
	struct recv_priv *precvpriv = &padapter->recvpriv;


	/* _rtw_init_listhead(&(precvframe->u.hdr.list)); */
	rtw_list_delete(&(precvframe->u.hdr.list));


	rtw_list_insert_tail(&(precvframe->u.hdr.list), get_list_head(queue));

	if (padapter != NULL) {
		if (queue == &precvpriv->free_recv_queue)
			precvpriv->free_recvframe_cnt++;
	}


	return _SUCCESS;
}

sint rtw_enqueue_recvframe(union recv_frame *precvframe, _queue *queue)
{
	sint ret;

	/* _spinlock(&pfree_recv_queue->lock); */
	_rtw_spinlock_bh(&queue->lock);
	ret = _rtw_enqueue_recvframe(precvframe, queue);
	/* _rtw_spinunlock(&pfree_recv_queue->lock); */
	_rtw_spinunlock_bh(&queue->lock);

	return ret;
}

/*
sint	rtw_enqueue_recvframe(union recv_frame *precvframe, _queue *queue)
{
	return rtw_free_recvframe(precvframe, queue);
}
*/




/*
caller : defrag ; recvframe_chk_defrag in recv_thread  (passive)
pframequeue: defrag_queue : will be accessed in recv_thread  (passive)

using spinlock to protect

*/

void rtw_free_recvframe_queue(_queue *pframequeue,  _queue *pfree_recv_queue)
{
	union	recv_frame	*precvframe;
	_list	*plist, *phead;

	_rtw_spinlock(&pframequeue->lock);

	phead = get_list_head(pframequeue);
	plist = get_next(phead);

	while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		precvframe = LIST_CONTAINOR(plist, union recv_frame, u);

		plist = get_next(plist);

		/* rtw_list_delete(&precvframe->u.hdr.list); */ /* will do this in rtw_free_recvframe() */

		rtw_free_recvframe(precvframe, pfree_recv_queue);
	}

	_rtw_spinunlock(&pframequeue->lock);


}

u32 rtw_free_uc_swdec_pending_queue(_adapter *adapter)
{
	u32 cnt = 0;
	union recv_frame *pending_frame;
	while ((pending_frame = rtw_alloc_recvframe(&adapter->recvpriv.uc_swdec_pending_queue))) {
		rtw_free_recvframe(pending_frame, &adapter->recvpriv.free_recv_queue);
		cnt++;
	}

	if (cnt)
		RTW_INFO(FUNC_ADPT_FMT" dequeue %d\n", FUNC_ADPT_ARG(adapter), cnt);

	return cnt;
}


sint rtw_enqueue_recvbuf_to_head(struct recv_buf *precvbuf, _queue *queue)
{
	_rtw_spinlock_bh(&queue->lock);

	rtw_list_delete(&precvbuf->list);
	rtw_list_insert_head(&precvbuf->list, get_list_head(queue));

	_rtw_spinunlock_bh(&queue->lock);

	return _SUCCESS;
}

sint rtw_enqueue_recvbuf(struct recv_buf *precvbuf, _queue *queue)
{
#ifndef CONFIG_SDIO_HCI
	unsigned long sp_flags;
#endif

#ifdef CONFIG_SDIO_HCI
	_rtw_spinlock_bh(&queue->lock);
#else
	_rtw_spinlock_irq(&queue->lock, &sp_flags);
#endif/*#ifdef CONFIG_SDIO_HCI*/

	rtw_list_delete(&precvbuf->list);

	rtw_list_insert_tail(&precvbuf->list, get_list_head(queue));
#ifdef CONFIG_SDIO_HCI
	_rtw_spinunlock_bh(&queue->lock);
#else
	_rtw_spinunlock_irq(&queue->lock, &sp_flags);
#endif/*#ifdef CONFIG_SDIO_HCI*/
	return _SUCCESS;

}

struct recv_buf *rtw_dequeue_recvbuf(_queue *queue)
{
	struct recv_buf *precvbuf;
	_list	*plist, *phead;
#ifndef CONFIG_SDIO_HCI
	unsigned long sp_flags;
#endif

#ifdef CONFIG_SDIO_HCI
	_rtw_spinlock_bh(&queue->lock);
#else
	_rtw_spinlock_irq(&queue->lock, &sp_flags);
#endif/*#ifdef CONFIG_SDIO_HCI*/

	if (_rtw_queue_empty(queue) == _TRUE)
		precvbuf = NULL;
	else {
		phead = get_list_head(queue);

		plist = get_next(phead);

		precvbuf = LIST_CONTAINOR(plist, struct recv_buf, list);

		rtw_list_delete(&precvbuf->list);

	}

#ifdef CONFIG_SDIO_HCI
	_rtw_spinunlock_bh(&queue->lock);
#else
	_rtw_spinunlock_irq(&queue->lock, &sp_flags);
#endif/*#ifdef CONFIG_SDIO_HCI*/

	return precvbuf;

}

sint recvframe_chkmic(_adapter *adapter,  union recv_frame *precvframe)
{

	sint	i, res = _SUCCESS;
	u32	datalen;
	u8	miccode[8];
	u8	bmic_err = _FALSE, brpt_micerror = _TRUE;
	u8	*pframe, *payload, *pframemic;
	u8	*mickey;
	/* u8	*iv,rxdata_key_idx=0; */
	struct	sta_info		*stainfo;
	struct	rx_pkt_attrib	*prxattrib = &precvframe->u.hdr.attrib;
	struct	security_priv	*psecuritypriv = &adapter->securitypriv;

	struct mlme_ext_priv	*pmlmeext = &adapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	stainfo = rtw_get_stainfo(&adapter->stapriv , &prxattrib->ta[0]);

	if (prxattrib->encrypt == _TKIP_) {

		/* calculate mic code */
		if (stainfo != NULL) {
			if (IS_MCAST(prxattrib->ra)) {
				/* mickey=&psecuritypriv->dot118021XGrprxmickey.skey[0]; */
				/* iv = precvframe->u.hdr.wlan_hdr+prxattrib->hdrlen; */
				/* rxdata_key_idx =( ((iv[3])>>6)&0x3) ; */
				mickey = &psecuritypriv->dot118021XGrprxmickey[prxattrib->key_index].skey[0];

				/* RTW_INFO("\n recvframe_chkmic: bcmc key psecuritypriv->dot118021XGrpKeyid(%d),pmlmeinfo->key_index(%d) ,recv key_id(%d)\n", */
				/*								psecuritypriv->dot118021XGrpKeyid,pmlmeinfo->key_index,rxdata_key_idx); */

				if (psecuritypriv->binstallGrpkey == _FALSE) {
					res = _FAIL;
					RTW_INFO("\n recvframe_chkmic:didn't install group key!!!!!!!!!!\n");
					goto exit;
				}
			} else {
				mickey = &stainfo->dot11tkiprxmickey.skey[0];
			}

			datalen = precvframe->u.hdr.len - prxattrib->hdrlen - prxattrib->iv_len - prxattrib->icv_len - 8; /* icv_len included the mic code */
			pframe = precvframe->u.hdr.rx_data;
			payload = pframe + prxattrib->hdrlen + prxattrib->iv_len;

			/* rtw_seccalctkipmic(&stainfo->dot11tkiprxmickey.skey[0],pframe,payload, datalen ,&miccode[0],(unsigned char)prxattrib->priority); */ /* care the length of the data */

			rtw_seccalctkipmic(mickey, pframe, payload, datalen , &miccode[0], (unsigned char)prxattrib->priority); /* care the length of the data */

			pframemic = payload + datalen;

			bmic_err = _FALSE;

			for (i = 0; i < 8; i++) {
				if (miccode[i] != *(pframemic + i)) {
					bmic_err = _TRUE;
				}
			}


			if (bmic_err == _TRUE) {



				/* double check key_index for some timing issue , */
				/* cannot compare with psecuritypriv->dot118021XGrpKeyid also cause timing issue */
				if ((IS_MCAST(prxattrib->ra) == _TRUE)  && (prxattrib->key_index != pmlmeinfo->key_index))
					brpt_micerror = _FALSE;

				if ((prxattrib->bdecrypted == _TRUE) && (brpt_micerror == _TRUE)) {
					rtw_handle_tkip_mic_err(adapter, stainfo, (u8)IS_MCAST(prxattrib->ra));
					RTW_INFO(" mic error :prxattrib->bdecrypted=%d\n", prxattrib->bdecrypted);
				} else {
					RTW_INFO(" mic error :prxattrib->bdecrypted=%d\n", prxattrib->bdecrypted);
				}

				res = _FAIL;

			} else {
				/* mic checked ok */
				if ((psecuritypriv->bcheck_grpkey == _FALSE) && (IS_MCAST(prxattrib->ra) == _TRUE)) {
					psecuritypriv->bcheck_grpkey = _TRUE;
				}
			}

		}

		recvframe_pull_tail(precvframe, 8);

	}

exit:


	return res;

}

/*#define DBG_RX_SW_DECRYPTOR*/

/* decrypt and set the ivlen,icvlen of the recv_frame */
union recv_frame *decryptor(_adapter *padapter, union recv_frame *precv_frame)
{

	struct rx_pkt_attrib *prxattrib = &precv_frame->u.hdr.attrib;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	union recv_frame *return_packet = precv_frame;
	struct sta_info *psta = precv_frame->u.hdr.psta;
	u32  res = _SUCCESS;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *whdr = precv_frame->u.hdr.wlan_hdr;
	#else
	u8 *whdr = precv_frame->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	DBG_COUNTER(padapter->rx_logs.core_rx_post_decrypt);


	if (prxattrib->encrypt > 0) {
		u8 *iv = whdr + prxattrib->hdrlen;
		prxattrib->key_index = (((iv[3]) >> 6) & 0x3) ;

		if (prxattrib->key_index > WEP_KEYS) {
			RTW_INFO("prxattrib->key_index(%d) > WEP_KEYS\n", prxattrib->key_index);

			switch (prxattrib->encrypt) {
			case _WEP40_:
			case _WEP104_:
				prxattrib->key_index = psecuritypriv->dot11PrivacyKeyIndex;
				break;
			case _TKIP_:
			case _AES_:
			case _GCMP_:
			case _GCMP_256_:
			case _CCMP_256_:
			default:
				prxattrib->key_index = psecuritypriv->dot118021XGrpKeyid;
				break;
			}
		}

		switch (prxattrib->encrypt) {
		case _WEP40_:
		case _WEP104_:
			prxattrib->pn._byte_.TSC0 = iv[0];
			prxattrib->pn._byte_.TSC1 = iv[1];
			prxattrib->pn._byte_.TSC2 = iv[2];
			break;
		case _TKIP_:
			prxattrib->pn._byte_.TSC0 = iv[2];
			prxattrib->pn._byte_.TSC1 = iv[0];
			prxattrib->pn._byte_.TSC2 = iv[4];
			prxattrib->pn._byte_.TSC3 = iv[5];
			prxattrib->pn._byte_.TSC4 = iv[6];
			prxattrib->pn._byte_.TSC5 = iv[7];
			break;
		case _AES_:
		case _GCMP_:
		case _GCMP_256_:
		case _CCMP_256_:
			prxattrib->pn._byte_.TSC0 = iv[0];
			prxattrib->pn._byte_.TSC1 = iv[1];
			prxattrib->pn._byte_.TSC2 = iv[4];
			prxattrib->pn._byte_.TSC3 = iv[5];
			prxattrib->pn._byte_.TSC4 = iv[6];
			prxattrib->pn._byte_.TSC5 = iv[7];
			break;
		}

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
		if (prxattrib->encrypt == _SMS4_ || prxattrib->encrypt == _GCM_SM4_) {

			if (IS_MCAST(prxattrib->ra))
				prxattrib->key_index = padapter->wapiApInfo.keyIdx;
			else
				prxattrib->key_index = psta->wapiStaInfo.keyIdx;

		}
#endif


	}

	if (prxattrib->encrypt && !prxattrib->bdecrypted) {
		if (GetFrameType(get_recvframe_data(precv_frame)) == WIFI_DATA
			#ifdef CONFIG_CONCURRENT_MODE
			&& !IS_MCAST(prxattrib->ra) /* bc/mc packets may use sw decryption for concurrent mode */
			#endif
		) {
			if (IS_MCAST(prxattrib->ra))
				psecuritypriv->hw_decrypted = _FALSE;
			else
				psta->hw_decrypted = _FALSE;
		}

#ifdef DBG_RX_SW_DECRYPTOR
		RTW_INFO(ADPT_FMT" - sec_type:%s DO SW decryption\n",
			ADPT_ARG(padapter), security_type_str(prxattrib->encrypt));
#endif

#ifdef DBG_RX_DECRYPTOR
		RTW_INFO("[%s] %d: PKT decrypted(%d), PKT encrypt(%d), Set %pM hw_decrypted(%d)\n",
			 __FUNCTION__,
			 __LINE__,
			 prxattrib->bdecrypted,
			 prxattrib->encrypt,
			 psta->phl_sta->mac_addr,
			 psta->hw_decrypted);
#endif

		switch (prxattrib->encrypt) {
		case _WEP40_:
		case _WEP104_:
			DBG_COUNTER(padapter->rx_logs.core_rx_post_decrypt_wep);
			rtw_wep_decrypt(padapter, (u8 *)precv_frame);
			break;
		case _TKIP_:
			DBG_COUNTER(padapter->rx_logs.core_rx_post_decrypt_tkip);
			res = rtw_tkip_decrypt(padapter, (u8 *)precv_frame);
			break;
		case _AES_:
		case _CCMP_256_:
			DBG_COUNTER(padapter->rx_logs.core_rx_post_decrypt_aes);
			res = rtw_aes_decrypt(padapter, (u8 *)precv_frame);
			break;
		case _GCMP_:
		case _GCMP_256_:
			DBG_COUNTER(padapter->rx_logs.core_rx_post_decrypt_gcmp);
			res = rtw_gcmp_decrypt(padapter, (u8 *)precv_frame);
			break;
#if defined(CONFIG_WAPI_SUPPORT) ||  defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
		case _SMS4_:
			DBG_COUNTER(padapter->rx_logs.core_rx_post_decrypt_wapi);
			res = rtw_sms4_decrypt(padapter, (u8 *)precv_frame);
			break;
#endif
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
		case _GCM_SM4_:
			DBG_COUNTER(padapter->rx_logs.core_rx_post_decrypt_wapi);
			res = rtw_gcm_sm4_decrypt(padapter, (u8 *)precv_frame);
			break;
#endif
		default:
			break;
		}
	} else if (prxattrib->bdecrypted == 1
		   && prxattrib->encrypt > 0
		&& (psecuritypriv->busetkipkey == 1 || prxattrib->encrypt != _TKIP_)
		  ) {
#if 0
		if ((prxstat->icv == 1) && (prxattrib->encrypt != _AES_)) {
			psecuritypriv->hw_decrypted = _FALSE;


			rtw_free_recvframe(precv_frame, &padapter->recvpriv.free_recv_queue);

			return_packet = NULL;

		} else
#endif
		{
			DBG_COUNTER(padapter->rx_logs.core_rx_post_decrypt_hw);

			psecuritypriv->hw_decrypted = _TRUE;
			psta->hw_decrypted = _TRUE;
#ifdef DBG_RX_DECRYPTOR
			RTW_INFO("[%s] %d: PKT decrypted(%d), PKT encrypt(%d), Set %pM hw_decrypted(%d)\n",
			 __FUNCTION__,
			 __LINE__,
			 prxattrib->bdecrypted,
			 prxattrib->encrypt,
			 psta->phl_sta->mac_addr,
			 psta->hw_decrypted);
#endif
		}
	} else {
		DBG_COUNTER(padapter->rx_logs.core_rx_post_decrypt_unknown);
#ifdef DBG_RX_DECRYPTOR
		RTW_INFO("[%s] %d: PKT decrypted(%d), PKT encrypt(%d), Set %pM hw_decrypted(%d)\n",
			 __FUNCTION__,
			 __LINE__,
			 prxattrib->bdecrypted,
			 prxattrib->encrypt,
			 psta->phl_sta->mac_addr,
			 psta->hw_decrypted);
#endif
	}

	#ifdef CONFIG_RTW_MESH
	if (res != _FAIL
		&& !prxattrib->amsdu
		&& prxattrib->mesh_ctrl_present)
		res = rtw_mesh_rx_validate_mctrl_non_amsdu(padapter, precv_frame);
	#endif

	if (res == _FAIL) {
		return_packet = NULL;
	} else
		prxattrib->bdecrypted = _TRUE;
	/* recvframe_chkmic(adapter, precv_frame);   */ /* move to recvframme_defrag function */


	return return_packet;

}

/* ###set the security information in the recv_frame */
union recv_frame *portctrl(_adapter *adapter, union recv_frame *precv_frame)
{
	u8 *psta_addr = NULL;
	uint  auth_alg;
	struct recv_frame_hdr *pfhdr;
	struct sta_info *psta;
	struct sta_priv *pstapriv ;
	union recv_frame *prtnframe;
	u16	ether_type = 0;
	u16  eapol_type = 0x888e;/* for Funia BD's WPA issue  */
	struct rx_pkt_attrib *pattrib;

	pstapriv = &adapter->stapriv;

	auth_alg = adapter->securitypriv.dot11AuthAlgrthm;

	pfhdr = &precv_frame->u.hdr;
	pattrib = &pfhdr->attrib;
	psta_addr = pattrib->ta;

	prtnframe = NULL;

	psta = rtw_get_stainfo(pstapriv, psta_addr);


	if ((auth_alg == dot11AuthAlgrthm_8021X)
#ifdef CONFIG_RTL_CFG80211_WAPI_SUPPORT
	    || (auth_alg == dot11AuthAlgrthm_WAPI)
#endif
	   ) {
		if ((psta != NULL) && (psta->ieee8021x_blocked)) {
			#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
			struct rtw_recv_pkt *rx_req = precv_frame->u.hdr.rx_req;
                	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
			u8 *ptr = get_recvframe_data(precv_frame);

			/* blocked */
			/* only accept EAPOL frame */

			prtnframe = precv_frame;

			/* get ether_type */
			#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
			if (rx_req->mdata.hdr_conv)
				ptr = ptr + ETH_ALEN * 2;
			else
			#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
				ptr = ptr + pfhdr->attrib.hdrlen + pfhdr->attrib.iv_len + LLC_HEADER_SIZE;
			_rtw_memcpy(&ether_type, ptr, 2);

#ifdef CONFIG_RTW_MULTI_AP_R2
			/* if EAPOL frame carries 802.1Q VLAN tag*/
			if(ether_type == __constant_htons(ETH_P_8021Q))
			{
				ptr = ptr + VLAN_TAG_LEN;
				_rtw_memcpy(&ether_type, ptr, 2);
			}
#endif

			ether_type = ntohs((unsigned short)ether_type);

			if ((ether_type == eapol_type)
#ifdef CONFIG_RTL_CFG80211_WAPI_SUPPORT
			    || (ether_type == 0x88B4)
#endif
			   )
				prtnframe = precv_frame;
			else {
				prtnframe = NULL;
			}
		} else {
			/* allowed */
			/* check decryption status, and decrypt the frame if needed */

			prtnframe = precv_frame;
			/* check is the EAPOL frame or not (Rekey) */
			/* if(ether_type == eapol_type){ */
			/* check Rekey */

			/*	prtnframe=precv_frame; */
			/* } */
		}
	} else
		prtnframe = precv_frame;

	return prtnframe;
}

/* VALID_PN_CHK
 * Return true when PN is legal, otherwise false.
 * Legal PN:
 *	1. If old PN is 0, any PN is legal
 *	2. PN > old PN
 */
#define PN_LESS_CHK(a, b)	(((a-b) & 0x800000000000) != 0)
#define VALID_PN_CHK(new, old)	(((old) == 0) || PN_LESS_CHK(old, new))
#define CCMPH_2_KEYID(ch)	(((ch) & 0x00000000c0000000) >> 30)

sint recv_ucast_pn_decache(union recv_frame *precv_frame)
{
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	struct sta_info *sta = precv_frame->u.hdr.psta;
	struct stainfo_rxcache *prxcache = &sta->sta_recvpriv.rxcache;
	u8 *pdata = precv_frame->u.hdr.rx_data;
	sint tid = precv_frame->u.hdr.attrib.priority;
	u64 tmp_iv_hdr = 0;
	u64 curr_pn = 0, pkt_pn = 0;

	if (tid > 15)
		return _FAIL;

	if (pattrib->encrypt == _AES_) {
#ifdef PLATFORM_ECOS
		tmp_iv_hdr = le64_to_cpu(get_unaligned((u64*)(pdata + pattrib->hdrlen)));
#else /* ! PLATFORM_ECOS */
		tmp_iv_hdr = le64_to_cpu(*(u64*)(pdata + pattrib->hdrlen));
#endif /*  PLATFORM_ECOS */
		pkt_pn = CCMPH_2_PN(tmp_iv_hdr);
		tmp_iv_hdr = le64_to_cpu(*(u64*)prxcache->iv[tid]);
		curr_pn = CCMPH_2_PN(tmp_iv_hdr);

		if (!VALID_PN_CHK(pkt_pn, curr_pn)) {
			/* return _FAIL; */
		} else {
			prxcache->last_tid = tid;
			_rtw_memcpy(prxcache->iv[tid],
				    (pdata + pattrib->hdrlen),
				    sizeof(prxcache->iv[tid]));
		}
	}

	return _SUCCESS;
}

sint recv_bcast_pn_decache(union recv_frame *precv_frame)
{
	_adapter *padapter = precv_frame->u.hdr.adapter;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *pdata = precv_frame->u.hdr.wlan_hdr;
	#else
	u8 *pdata = precv_frame->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	u64 tmp_iv_hdr = 0;
	u64 curr_pn = 0, pkt_pn = 0;
	u8 key_id;

	if ((pattrib->encrypt == _AES_) && (MLME_IS_STA(padapter))) {
#ifdef PLATFORM_ECOS
		tmp_iv_hdr = le64_to_cpu(get_unaligned((u64*)(pdata + pattrib->hdrlen)));
#else /* ! PLATFORM_ECOS */
		tmp_iv_hdr = le64_to_cpu(*(u64*)(pdata + pattrib->hdrlen));
#endif /*  PLATFORM_ECOS */
		key_id = CCMPH_2_KEYID(tmp_iv_hdr);
		pkt_pn = CCMPH_2_PN(tmp_iv_hdr);

		curr_pn = le64_to_cpu(*(u64*)psecuritypriv->iv_seq[key_id]);
		curr_pn &= 0x0000ffffffffffff;

		if (!VALID_PN_CHK(pkt_pn, curr_pn))
			return _FAIL;

		*(u64*)psecuritypriv->iv_seq[key_id] = cpu_to_le64(pkt_pn);
	}

	return _SUCCESS;
}

sint recv_decache(union recv_frame *precv_frame)
{
	struct sta_info *psta = precv_frame->u.hdr.psta;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	_adapter *adapter = psta->padapter;
	sint tid = pattrib->priority;
	u16 seq_ctrl = ((precv_frame->u.hdr.attrib.seq_num & 0xffff) << 4) |
		       (precv_frame->u.hdr.attrib.frag_num & 0xf);
	u16 *prxseq;

	if (tid > 15)
		return _FAIL;

	/* currently we don't handle fragment pkt */
	if (pattrib->frag_num || pattrib->mfrag)
		return _FAIL;

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV
	if(pattrib->amsdu_cut)
		return _SUCCESS;
	#endif

	if (pattrib->qos) {
		if (IS_MCAST(pattrib->ra))
			prxseq = &psta->sta_recvpriv.bmc_tid_rxseq[tid];
		else
			prxseq = &psta->sta_recvpriv.rxcache.tid_rxseq[tid];
	} else {
		if (IS_MCAST(pattrib->ra)) {
			prxseq = &psta->sta_recvpriv.nonqos_bmc_rxseq;
			#ifdef DBG_RX_SEQ
			RTW_INFO("DBG_RX_SEQ "FUNC_ADPT_FMT" nonqos bmc seq_num:%d\n"
				, FUNC_ADPT_ARG(adapter), pattrib->seq_num);
			#endif

		} else {
			prxseq = &psta->sta_recvpriv.nonqos_rxseq;
			#ifdef DBG_RX_SEQ
			RTW_INFO("DBG_RX_SEQ "FUNC_ADPT_FMT" nonqos seq_num:%d\n"
				, FUNC_ADPT_ARG(adapter), pattrib->seq_num);
			#endif
		}
	}

	if (seq_ctrl == *prxseq) {
		/* for non-AMPDU case	*/
		psta->sta_stats.duplicate_cnt++;
		if (psta->sta_stats.duplicate_cnt % 100 == 0)
			RTW_INFO("%s: tid=%u seq=%d frag=%d\n", __func__
				, tid, precv_frame->u.hdr.attrib.seq_num
				, precv_frame->u.hdr.attrib.frag_num);

		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" recv_decache _FAIL for sta="MAC_FMT"\n"
			, FUNC_ADPT_ARG(adapter), MAC_ARG(psta->phl_sta->mac_addr));
		#endif
		return _FAIL;
	}
	*prxseq = seq_ctrl;

	return _SUCCESS;
}

void process_pwrbit_data(_adapter *padapter, union recv_frame *precv_frame, struct sta_info *psta)
{
#ifdef CONFIG_AP_MODE
	unsigned char pwrbit;

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	struct rtw_recv_pkt *rx_req = precv_frame->u.hdr.rx_req;
	pwrbit = rx_req->mdata.pwr_bit;
	#else
	u8 *ptr = precv_frame->u.hdr.rx_data;
	pwrbit = GetPwrMgt(ptr);
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	if (rtw_core_twt_sta_active(padapter, psta))
		pwrbit = 0;

	if (pwrbit) {
		if (!(psta->state & WIFI_SLEEP_STATE)) {
			/* psta->state |= WIFI_SLEEP_STATE; */
			/* rtw_tim_map_set(padapter, pstapriv->sta_dz_bitmap, BIT(psta->phl_sta->aid)); */

			stop_sta_xmit(padapter, psta);
			/* RTW_INFO_DUMP("to sleep, sta_dz_bitmap=", pstapriv->sta_dz_bitmap, pstapriv->aid_bmp_len); */
		}
	} else {
		if (psta->state & WIFI_SLEEP_STATE) {
			/* psta->state ^= WIFI_SLEEP_STATE; */
			/* rtw_tim_map_clear(padapter, pstapriv->sta_dz_bitmap, BIT(psta->phl_sta->aid)); */

			wakeup_sta_to_xmit(padapter, psta);
			/* RTW_INFO_DUMP("to wakeup, sta_dz_bitmap=", pstapriv->sta_dz_bitmap, pstapriv->aid_bmp_len); */
		}
	}
#endif
}

void process_a_control_subfield(_adapter *padapter,  union recv_frame *precv_frame)
{
	struct	sta_priv		*pstapriv = &padapter->stapriv;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	struct sta_info *psta = NULL;
	u8 is_OM_ID=0;
	u32 ht_control;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *ptr = precv_frame->u.hdr.wlan_hdr;
	#else
	u8 *ptr = precv_frame->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	u8 *htc_prt = (u8 *) ptr + WLAN_HDR_A3_QOS_LEN;
	void *phl = padapter->dvobj->phl;
	struct rtw_wifi_role_t *role = padapter->phl_role;

	_rtw_memcpy(pattrib->ta, get_addr2_ptr(ptr), ETH_ALEN);
	psta = rtw_get_stainfo(pstapriv, pattrib->ta);
	if (!psta)
		return;

#ifdef CONFIG_RTW_A4_STA
	if (pattrib->to_fr_ds == 3)
		htc_prt = (u8 *) ptr + WLAN_HDR_A4_QOS_LEN;
	else
#endif
		htc_prt = (u8 *) ptr + WLAN_HDR_A3_QOS_LEN;

	memcpy(&ht_control, (u8 *) htc_prt, 4);
	ht_control = le32_to_cpu(ht_control);

	if ((ht_control & ACONTROL_MASK) == 0x03 ) {
		u8 actrl_type;
		u8 rx_ss, tx_ss, ch_bw, ul_mu_disable;
		u8 ori_phl_sta_bw, ori_phl_sta_nss_rx, ori_phl_sta_nss_tx;

		actrl_type = ( (ht_control & ACONTROL_ID_MASK) >> 2);

		if ( OM_ID == actrl_type )  {
			rx_ss = ( (ht_control & OM_RX_SS_MASK) >> 6);
			ch_bw = ( (ht_control & OM_CH_WIDTH_MASK) >> 9);
			ul_mu_disable = ( (ht_control & OM_UL_MU_DISABLE) >> 11);
			tx_ss = ( (ht_control & OM_TX_SS_MASK) >> 12);
			is_OM_ID = 1;
		} else {
			if ((actrl_type == UPH_ID) || (actrl_type == CAS_ID)){
				if (((ht_control >> 14) & 0xf) == OM_ID) { //14=(2+4+8)
					rx_ss = (ht_control >> 18) & 0x7;
					ch_bw = (ht_control >> 21) & 0x3;
					ul_mu_disable = (ht_control >> 23) & 0x1;
					tx_ss = (ht_control >> 24) & 0x7;
					is_OM_ID = 1;
				}
			}else if(actrl_type== BQR_ID){
				if (((ht_control >> 16) & 0xf) == OM_ID){ //16=(2+4+10)
					rx_ss = (ht_control >> 18) & 0x7;
					ch_bw = (ht_control >> 23) & 0x3;
					ul_mu_disable = (ht_control >> 25) & 0x1;
					tx_ss = (ht_control >> 26) & 0x7;
					is_OM_ID=1;
				}
			}
		 }

		if (is_OM_ID) {

			if (pattrib->to_fr_ds == 3)
				RTW_PRINT("[%s:%d]: recv a a4 control_subfield\n", __func__, __LINE__);

			ori_phl_sta_nss_rx = psta->phl_sta->asoc_cap.nss_tx;
			ori_phl_sta_bw = psta->phl_sta->chandef.bw;
			ori_phl_sta_nss_tx = psta->phl_sta->asoc_cap.nss_rx;

			psta->phl_sta->asoc_cap.nss_tx = rx_ss + 1;
			psta->phl_sta->chandef.bw = ch_bw;
			psta->phl_sta->asoc_cap.nss_rx = tx_ss + 1;

			if(!(role->phl_com->dev_cap.bw_sup & BIT(psta->phl_sta->chandef.bw)) ||
				(psta->phl_sta->asoc_cap.nss_tx > role->phl_com->phy_cap[0].txss) ||
				(psta->phl_sta->asoc_cap.nss_rx > role->phl_com->phy_cap[0].rxss))
		   	{
	     		RTW_PRINT("[%s:%d]: iface %s parse STA %pm OM ID error txss:%d rxss:%d bw:%d\n",
					__func__, __LINE__,
					padapter->pnetdev->name, psta->phl_sta->mac_addr,
					psta->phl_sta->asoc_cap.nss_tx, psta->phl_sta->asoc_cap.nss_rx,
					psta->phl_sta->chandef.bw);

				/* restore the station setting */
				psta->phl_sta->asoc_cap.nss_tx = ori_phl_sta_nss_tx;
				psta->phl_sta->asoc_cap.nss_rx = ori_phl_sta_nss_rx;
				psta->phl_sta->chandef.bw = ori_phl_sta_bw;
	     		return;
		   	}

			if(((psta->phl_sta->chandef.bw != ori_phl_sta_bw)
				|| (psta->phl_sta->asoc_cap.nss_rx != ori_phl_sta_nss_tx))
				&& (psta->phl_sta->asoc_cap.nss_tx == ori_phl_sta_nss_rx)){
				rtw_phl_write8(phl, 0xc01, 0); //stop to xmit trigger frame
			}
			if((psta->phl_sta->chandef.bw != ori_phl_sta_bw)
				|| (psta->phl_sta->asoc_cap.nss_tx != ori_phl_sta_nss_rx)) {
				rtw_phl_cmd_set_macid_pause(role, psta->phl_sta, true, PHL_CMD_DIRECTLY, 0);
				rtw_phl_cmd_set_macid_pkt_drop(role, psta->phl_sta, 0, PHL_CMD_DIRECTLY, 0);
				rtw_phl_cmd_set_macid_pkt_drop(role, psta->phl_sta, 1, PHL_CMD_DIRECTLY, 0);
				rtw_phl_cmd_change_stainfo(phl,
				                           psta->phl_sta,
				                           STA_CHG_RAMASK,
				                           NULL,
				                           0,
				                           PHL_CMD_DIRECTLY,
				                           0);
				rtw_phl_cmd_set_macid_pause(role, psta->phl_sta, false, PHL_CMD_DIRECTLY, 0);
			}

			if(((psta->phl_sta->chandef.bw != ori_phl_sta_bw)
				|| (psta->phl_sta->asoc_cap.nss_rx != ori_phl_sta_nss_tx))
				&& (psta->phl_sta->asoc_cap.nss_tx == ori_phl_sta_nss_rx)) {

				#ifdef CONFIG_WFA_OFDMA_Logo_Test
				if (padapter->registrypriv.wifi_mib.ofdma_enable & ul_fix_mode){
					//rtw_ap_update_sta_trigger_info(padapter);

					rtw_phl_write8(phl, 0xc600, 0x3);

					rtw_clean_ofdma_grp(padapter);
					rtw_add_ul_ru_ofdma_grp(padapter);
					rtw_add_ul_ru_ofdma_grp_2(padapter);

					rtw_set_ru_ulmacid_cfg(padapter, 1);

					rtw_phl_write8(phl, 0xc600, 0x1c);
				}
				#endif

				if (get_frame_sub_type(ptr) & BIT(6))
				  	RTW_PRINT("OM carried in Qos Null data\n");
				else
				    RTW_PRINT("OM carried in Qos data\n");

				RTW_PRINT("ori_phl_sta_nss_rx:%d , new nss_rx:%d\n", ori_phl_sta_nss_rx, psta->phl_sta->asoc_cap.nss_tx);
				RTW_PRINT("ori_phl_sta_bw:%d , new chandef.bw:%d\n", ori_phl_sta_bw, psta->phl_sta->chandef.bw);
				RTW_PRINT("ori_phl_sta_nss_tx:%d , new nss_tx:%d\n", ori_phl_sta_nss_tx, psta->phl_sta->asoc_cap.nss_rx);
				RTW_PRINT("new ul_mu_disable:%d\n", ul_mu_disable);
			  }
		 }
    }
}

/* CONFIG_VW_REFINE */
void process_wmmps_data(_adapter *padapter, union recv_frame *precv_frame, struct sta_info *psta)
{
#if defined(CONFIG_AP_MODE) || defined(CONFIG_TDLS)
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;

	if (!psta->uapsd_bitmap)
		return;

	if (psta->state & WIFI_SLEEP_STATE) {
		int q_idx = rtw_get_txq_idx(pattrib->priority);

		if (psta->uapsd_bitmap & BIT(q_idx)) {
			struct sta_xmit_priv *pstaxmitpriv = &psta->sta_xmitpriv;
			struct dvobj_priv *dvobj = padapter->dvobj;
			_queue *sta_queue = &dvobj->ps_trigger_sta_queue;

			_rtw_spinlock_bh(&sta_queue->lock);
			pstaxmitpriv->ps_trigger_type |= BIT1;
			if (rtw_is_list_empty(&pstaxmitpriv->ps_trigger) == _TRUE) {
				rtw_list_insert_tail(&pstaxmitpriv->ps_trigger, &sta_queue->queue);
				sta_queue->qlen++;
//#if !defined(CONFIG_ONE_TXQ)
				if (dvobj->tx_mode != 2)
					rtw_tasklet_hi_schedule(&dvobj->ps_trigger_tasklet);
//#endif
			}
			_rtw_spinunlock_bh(&sta_queue->lock);
		}
	}
#endif

}

#ifdef CONFIG_TDLS
sint OnTDLS(_adapter *adapter, union recv_frame *precv_frame)
{
	struct rx_pkt_attrib	*pattrib = &precv_frame->u.hdr.attrib;
	sint ret = _SUCCESS;
	u8 *paction = get_recvframe_data(precv_frame);
	u8 category_field = 1;
#ifdef CONFIG_WFD
	u8 WFA_OUI[3] = { 0x50, 0x6f, 0x9a };
#endif /* CONFIG_WFD */
	struct tdls_info *ptdlsinfo = &(adapter->tdlsinfo);
	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct sta_priv *pstapriv = &(adapter->stapriv);
	struct sta_info *ptdls_sta = NULL;

	/* point to action field */
	paction += pattrib->hdrlen
		   + pattrib->iv_len
		   + SNAP_SIZE
		   + ETH_TYPE_LEN
		   + PAYLOAD_TYPE_LEN
		   + category_field;

	RTW_INFO("[TDLS] Recv %s from "MAC_FMT" with SeqNum = %d\n", rtw_tdls_action_txt(*paction), MAC_ARG(pattrib->src), GetSequence(get_recvframe_data(precv_frame)));

	if (rtw_hw_chk_wl_func(adapter_to_dvobj(adapter), WL_FUNC_TDLS) == _FALSE) {
		RTW_INFO("Ignore tdls frame since hal doesn't support tdls\n");
		ret = _FAIL;
		return ret;
	}

	if (rtw_is_tdls_enabled(adapter) == _FALSE) {
		RTW_INFO("recv tdls frame, "
			 "but tdls haven't enabled\n");
		ret = _FAIL;
		return ret;
	}

	ptdls_sta = rtw_get_stainfo(pstapriv, get_sa(ptr));
	if (ptdls_sta == NULL) {
		switch (*paction) {
		case TDLS_SETUP_REQUEST:
		case TDLS_DISCOVERY_REQUEST:
			break;
		default:
			RTW_INFO("[TDLS] %s - Direct Link Peer = "MAC_FMT" not found for action = %d\n", __func__, MAC_ARG(get_sa(ptr)), *paction);
			ret = _FAIL;
			goto exit;
		}
	}

	switch (*paction) {
	case TDLS_SETUP_REQUEST:
		ret = On_TDLS_Setup_Req(adapter, precv_frame, ptdls_sta);
		break;
	case TDLS_SETUP_RESPONSE:
		ret = On_TDLS_Setup_Rsp(adapter, precv_frame, ptdls_sta);
		break;
	case TDLS_SETUP_CONFIRM:
		ret = On_TDLS_Setup_Cfm(adapter, precv_frame, ptdls_sta);
		break;
	case TDLS_TEARDOWN:
		ret = On_TDLS_Teardown(adapter, precv_frame, ptdls_sta);
		break;
	case TDLS_DISCOVERY_REQUEST:
		ret = On_TDLS_Dis_Req(adapter, precv_frame);
		break;
	case TDLS_PEER_TRAFFIC_INDICATION:
		ret = On_TDLS_Peer_Traffic_Indication(adapter, precv_frame, ptdls_sta);
		break;
	case TDLS_PEER_TRAFFIC_RESPONSE:
		ret = On_TDLS_Peer_Traffic_Rsp(adapter, precv_frame, ptdls_sta);
		break;
#ifdef CONFIG_TDLS_CH_SW
	case TDLS_CHANNEL_SWITCH_REQUEST:
		ret = On_TDLS_Ch_Switch_Req(adapter, precv_frame, ptdls_sta);
		break;
	case TDLS_CHANNEL_SWITCH_RESPONSE:
		ret = On_TDLS_Ch_Switch_Rsp(adapter, precv_frame, ptdls_sta);
		break;
#endif
#ifdef CONFIG_WFD
	/* First byte of WFA OUI */
	case 0x50:
		if (_rtw_memcmp(WFA_OUI, paction, 3)) {
			/* Probe request frame */
			if (*(paction + 3) == 0x04) {
				/* WFDTDLS: for sigma test, do not setup direct link automatically */
				ptdlsinfo->dev_discovered = _TRUE;
				RTW_INFO("recv tunneled probe request frame\n");
				issue_tunneled_probe_rsp(adapter, precv_frame);
			}
			/* Probe response frame */
			if (*(paction + 3) == 0x05) {
				/* WFDTDLS: for sigma test, do not setup direct link automatically */
				ptdlsinfo->dev_discovered = _TRUE;
				RTW_INFO("recv tunneled probe response frame\n");
			}
		}
		break;
#endif /* CONFIG_WFD */
	default:
		RTW_INFO("receive TDLS frame %d but not support\n", *paction);
		ret = _FAIL;
		break;
	}

exit:
	return ret;

}
#endif /* CONFIG_TDLS */

void count_rx_stats(_adapter *padapter, union recv_frame *prframe, struct sta_info *sta)
{
	int	sz;
	struct sta_info		*psta = NULL;
	struct stainfo_stats	*pstats = NULL;
	struct rx_pkt_attrib	*pattrib = &prframe->u.hdr.attrib;
	struct recv_priv		*precvpriv = &padapter->recvpriv;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *ptr = prframe->u.hdr.wlan_hdr;
	#else
	u8 *ptr = prframe->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	sz = get_recvframe_len(prframe);
	precvpriv->rx_bytes += sz;
	precvpriv->rx_pkts++;

#if defined(WFO_VIRT_RECEIVER)
	if (wfo_check_offload(padapter)) {
		sync_rx_stats(padapter->iface_id, precvpriv);
	}
#endif /* WFO_VIRT_RECEIVER */

	padapter->mlmepriv.LinkDetectInfo.NumRxOkInPeriod++;

	if (MacAddr_isBcst(pattrib->dst)) {
		precvpriv->rx_bc_pkts++;
		precvpriv->rx_bc_bytes += sz;
	}
	else if (IS_MCAST(pattrib->dst)) {
		precvpriv->rx_mc_pkts++;
		precvpriv->rx_mc_bytes += sz;
	}
	else {
		padapter->mlmepriv.LinkDetectInfo.NumRxUnicastOkInPeriod++;
		precvpriv->rx_uc_pkts++;
		precvpriv->rx_uc_bytes += sz;
	}

	if (sta)
		psta = sta;
	else
		psta = prframe->u.hdr.psta;

	if (psta) {
		u8 is_ra_bmc = IS_MCAST(pattrib->ra);

		pstats = &psta->sta_stats;

		pstats->last_rx_time = rtw_get_current_time();
		pstats->rx_data_pkts++;
		pstats->rx_bytes += sz;
		if (is_broadcast_mac_addr(pattrib->ra)) {
			pstats->rx_data_bc_pkts++;
			pstats->rx_bc_bytes += sz;
		} else if (is_ra_bmc) {
			pstats->rx_data_mc_pkts++;
			pstats->rx_mc_bytes += sz;
		}

		if (!is_ra_bmc) {
			pstats->rxratecnt[pattrib->data_rate]++;
			/*record rx packets for every tid*/
			pstats->rx_data_qos_pkts[pattrib->priority]++;
		}

		if(GetRetry(ptr))
			pstats->rx_data_retry_pkts++;

#if defined(CONFIG_CHECK_LEAVE_LPS) && defined(CONFIG_LPS_CHK_BY_TP)
		if (adapter_to_pwrctl(padapter)->lps_chk_by_tp)
			traffic_check_for_leave_lps_by_tp(padapter, _FALSE, psta);
#endif /* CONFIG_LPS */
		psta->cur_rx_data_rate = pattrib->data_rate;
		#ifdef CONFIG_WFA_OFDMA_Logo_Test
		psta->phl_sta->cur_rx_data_rate = pattrib->data_rate;
		#endif
		psta->cur_rx_gi_ltf = pattrib->sgi;
		psta->cur_rx_bw = pattrib->bw;
	}

#ifdef CONFIG_CHECK_LEAVE_LPS
#ifdef CONFIG_LPS_CHK_BY_TP
	if (!adapter_to_pwrctl(padapter)->lps_chk_by_tp)
#endif
		traffic_check_for_leave_lps(padapter, _FALSE, 0);
#endif /* CONFIG_CHECK_LEAVE_LPS */

}

sint sta2sta_data_frame(
	_adapter *adapter,
	union recv_frame *precv_frame,
	struct sta_info **psta
)
{
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *ptr = precv_frame->u.hdr.wlan_hdr;
	#else
	u8 *ptr = precv_frame->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	sint ret = _SUCCESS;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	struct	sta_priv		*pstapriv = &adapter->stapriv;
	struct	mlme_priv	*pmlmepriv = &adapter->mlmepriv;
	u8 *mybssid  = get_bssid(pmlmepriv);
	u8 *myhwaddr = adapter_mac_addr(adapter);
	u8 *sta_addr = pattrib->ta;
	sint bmcast = IS_MCAST(pattrib->dst);

#ifdef CONFIG_TDLS
	struct tdls_info *ptdlsinfo = &adapter->tdlsinfo;
#ifdef CONFIG_TDLS_CH_SW
	struct tdls_ch_switch *pchsw_info = &ptdlsinfo->chsw_info;
#endif
	struct sta_info *ptdls_sta = NULL;
	u8 *psnap_type = ptr + pattrib->hdrlen + pattrib->iv_len + SNAP_SIZE;
	/* frame body located after [+2]: ether-type, [+1]: payload type */
	u8 *pframe_body = psnap_type + 2 + 1;
#endif


	/* RTW_INFO("[%s] %d, seqnum:%d\n", __FUNCTION__, __LINE__, pattrib->seq_num); */

	if ((check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE) ||
	    (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE)) {

		/* filter packets that SA is myself or multicast or broadcast */
		if (_rtw_memcmp(myhwaddr, pattrib->src, ETH_ALEN)) {
			ret = _FAIL;
			goto exit;
		}

		if ((!_rtw_memcmp(myhwaddr, pattrib->dst, ETH_ALEN))	&& (!bmcast)) {
			ret = _FAIL;
			goto exit;
		}

		if (_rtw_memcmp(pattrib->bssid, "\x0\x0\x0\x0\x0\x0", ETH_ALEN) ||
		    _rtw_memcmp(mybssid, "\x0\x0\x0\x0\x0\x0", ETH_ALEN) ||
		    (!_rtw_memcmp(pattrib->bssid, mybssid, ETH_ALEN))) {
			ret = _FAIL;
			goto exit;
		}

	} else if (MLME_IS_STA(adapter)) {
#ifdef CONFIG_TDLS

		/* direct link data transfer */
		if (ptdlsinfo->link_established == _TRUE) {
			*psta = ptdls_sta = rtw_get_stainfo(pstapriv, pattrib->ta);
			if (ptdls_sta == NULL) {
				ret = _FAIL;
				goto exit;
			} else if (ptdls_sta->tdls_sta_state & TDLS_LINKED_STATE) {
				/* filter packets that SA is myself or multicast or broadcast */
				if (_rtw_memcmp(myhwaddr, pattrib->src, ETH_ALEN)) {
					ret = _FAIL;
					goto exit;
				}
				/* da should be for me */
				if ((!_rtw_memcmp(myhwaddr, pattrib->dst, ETH_ALEN)) && (!bmcast)) {
					ret = _FAIL;
					goto exit;
				}
				/* check BSSID */
				if (_rtw_memcmp(pattrib->bssid, "\x0\x0\x0\x0\x0\x0", ETH_ALEN) ||
				    _rtw_memcmp(mybssid, "\x0\x0\x0\x0\x0\x0", ETH_ALEN) ||
				    (!_rtw_memcmp(pattrib->bssid, mybssid, ETH_ALEN))) {
					ret = _FAIL;
					goto exit;
				}

#ifdef CONFIG_TDLS_CH_SW
				if (ATOMIC_READ(&pchsw_info->chsw_on) == _TRUE) {
					if (adapter->mlmeextpriv.cur_channel != rtw_get_oper_ch(adapter)) {
						pchsw_info->ch_sw_state |= TDLS_PEER_AT_OFF_STATE;
						if (!(pchsw_info->ch_sw_state & TDLS_CH_SW_INITIATOR_STATE))
							_cancel_timer_ex(&ptdls_sta->ch_sw_timer);
						/* On_TDLS_Peer_Traffic_Rsp(adapter, precv_frame); */
					}
				}
#endif

				/* process UAPSD tdls sta */
				process_pwrbit_data(adapter, precv_frame, ptdls_sta);

				/* if NULL-frame, check pwrbit */
				if ((get_frame_sub_type(ptr) & WIFI_DATA_NULL) == WIFI_DATA_NULL) {
					/* NULL-frame with pwrbit=1, buffer_STA should buffer frames for sleep_STA */
					if (GetPwrMgt(ptr)) {
						/* it would be triggered when we are off channel and receiving NULL DATA */
						/* we can confirm that peer STA is at off channel */
						RTW_INFO("TDLS: recv peer null frame with pwr bit 1\n");
						/* ptdls_sta->tdls_sta_state|=TDLS_PEER_SLEEP_STATE; */
					}

					/* TODO: Updated BSSID's seq. */
					/* RTW_INFO("drop Null Data\n"); */
					ptdls_sta->tdls_sta_state &= ~(TDLS_WAIT_PTR_STATE);
					ret = _FAIL;
					goto exit;
				}

				/* receive some of all TDLS management frames, process it at ON_TDLS */
				if (_rtw_memcmp(psnap_type, SNAP_ETH_TYPE_TDLS, 2)) {
					ret = OnTDLS(adapter, precv_frame);
					goto exit;
				}

				if ((get_frame_sub_type(ptr) & WIFI_QOS_DATA_TYPE) == WIFI_QOS_DATA_TYPE)
					process_wmmps_data(adapter, precv_frame, ptdls_sta);

				ptdls_sta->tdls_sta_state &= ~(TDLS_WAIT_PTR_STATE);

			}
		} else
#endif /* CONFIG_TDLS */
		{
			/* For Station mode, sa and bssid should always be BSSID, and DA is my mac-address */
			if (!_rtw_memcmp(pattrib->bssid, pattrib->src, ETH_ALEN)) {
				ret = _FAIL;
				goto exit;
			}
		}

	} else if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) {
		if (bmcast) {
			/* For AP mode, if DA == MCAST, then BSSID should be also MCAST */
			if (!IS_MCAST(pattrib->bssid)) {
				ret = _FAIL;
				goto exit;
			}
		} else { /* not mc-frame */
			if (!_rtw_memcmp(pattrib->bssid, mybssid, ETH_ALEN)) {
				ret = _FAIL;
				goto exit;
			}
			/* For AP mode, if DA is non-MCAST, then it must be BSSID, and bssid == BSSID */
			if (!_rtw_memcmp(pattrib->bssid, pattrib->dst, ETH_ALEN)) {
				ret = _FAIL;
				goto exit;
			}
		}

	} else if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == _TRUE) {
		_rtw_memcpy(pattrib->dst, GetAddr1Ptr(ptr), ETH_ALEN);
		_rtw_memcpy(pattrib->src, get_addr2_ptr(ptr), ETH_ALEN);
		_rtw_memcpy(pattrib->bssid, GetAddr3Ptr(ptr), ETH_ALEN);
		_rtw_memcpy(pattrib->ra, pattrib->dst, ETH_ALEN);
		_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

		sta_addr = mybssid;
	} else
		ret  = _FAIL;

#ifdef CONFIG_TDLS
	if (ptdls_sta == NULL)
#endif
		*psta = rtw_get_stainfo(pstapriv, sta_addr);

	if (*psta == NULL) {
#ifdef CONFIG_MP_INCLUDED
		if (adapter->registrypriv.mp_mode == 1) {
			if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == _TRUE)
				adapter->mppriv.rx_pktloss++;
		}
#endif
		ret = _FAIL;
		goto exit;
	}

exit:
	return ret;

}

sint ap2sta_data_frame(
	_adapter *adapter,
	union recv_frame *precv_frame,
	struct sta_info **psta)
{
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *ptr = precv_frame->u.hdr.wlan_hdr;
	#else
	u8 *ptr = precv_frame->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	sint ret = _SUCCESS;
	struct	sta_priv		*pstapriv = &adapter->stapriv;
	struct	mlme_priv	*pmlmepriv = &adapter->mlmepriv;
	u8 *mybssid  = get_bssid(pmlmepriv);
	u8 *myhwaddr = adapter_mac_addr(adapter);
	sint bmcast = IS_MCAST(pattrib->dst);


	if (MLME_IS_STA(adapter)
	    && (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE
		|| check_fwstate(pmlmepriv, WIFI_UNDER_LINKING) == _TRUE)
	   ) {

		/* filter packets that SA is myself or multicast or broadcast */
		if (_rtw_memcmp(myhwaddr, pattrib->src, ETH_ALEN)) {
			#ifdef DBG_RX_DROP_FRAME
			RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" SA="MAC_FMT", myhwaddr="MAC_FMT"\n"
				, FUNC_ADPT_ARG(adapter), MAC_ARG(pattrib->src), MAC_ARG(myhwaddr));
			#endif
			ret = _FAIL;
			goto exit;
		}

		/* da should be for me */
		if ((!_rtw_memcmp(myhwaddr, pattrib->dst, ETH_ALEN)) && (!bmcast)) {
			#ifdef DBG_RX_DROP_FRAME
			RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" DA="MAC_FMT"\n"
				, FUNC_ADPT_ARG(adapter), MAC_ARG(pattrib->dst));
			#endif
			ret = _FAIL;
			goto exit;
		}


		/* check BSSID */
		if (_rtw_memcmp(pattrib->bssid, "\x0\x0\x0\x0\x0\x0", ETH_ALEN) ||
		    _rtw_memcmp(mybssid, "\x0\x0\x0\x0\x0\x0", ETH_ALEN) ||
		    (!_rtw_memcmp(pattrib->bssid, mybssid, ETH_ALEN))) {
			#ifdef DBG_RX_DROP_FRAME
			RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" BSSID="MAC_FMT", mybssid="MAC_FMT"\n"
				, FUNC_ADPT_ARG(adapter), MAC_ARG(pattrib->bssid), MAC_ARG(mybssid));
			#endif
#ifndef CONFIG_CUSTOMER_ALIBABA_GENERAL
			if (!bmcast
				&& !IS_RADAR_DETECTED(adapter_to_rfctl(adapter))
			) {
				RTW_INFO(ADPT_FMT" -issue_deauth to the nonassociated ap=" MAC_FMT " for the reason(7)\n", ADPT_ARG(adapter), MAC_ARG(pattrib->bssid));
				issue_deauth(adapter, pattrib->bssid, WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA);
			}
#endif
			ret = _FAIL;
			goto exit;
		}

		*psta = rtw_get_stainfo(pstapriv, pattrib->ta);
		if (*psta == NULL) {
			#ifdef DBG_RX_DROP_FRAME
			RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" can't get psta under STATION_MODE ; drop pkt\n"
				, FUNC_ADPT_ARG(adapter));
			#endif
			ret = _FAIL;
			goto exit;
		}

		/*if ((get_frame_sub_type(ptr) & WIFI_QOS_DATA_TYPE) == WIFI_QOS_DATA_TYPE) {
		}
		*/

		if (get_frame_sub_type(ptr) & BIT(6)) {
			/* No data, will not indicate to upper layer, temporily count it here */
			/* count_rx_stats(adapter, precv_frame, *psta); */
			ret = RTW_RX_HANDLED;
			goto exit;
		}

	} else if ((check_fwstate(pmlmepriv, WIFI_MP_STATE) == _TRUE) &&
		   (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE)) {
		_rtw_memcpy(pattrib->dst, GetAddr1Ptr(ptr), ETH_ALEN);
		_rtw_memcpy(pattrib->src, get_addr2_ptr(ptr), ETH_ALEN);
		_rtw_memcpy(pattrib->bssid, GetAddr3Ptr(ptr), ETH_ALEN);
		_rtw_memcpy(pattrib->ra, pattrib->dst, ETH_ALEN);
		_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);


		*psta = rtw_get_stainfo(pstapriv, pattrib->bssid); /* get sta_info */
		if (*psta == NULL) {
			#ifdef DBG_RX_DROP_FRAME
			RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" can't get psta under WIFI_MP_STATE ; drop pkt\n"
				, FUNC_ADPT_ARG(adapter));
			#endif
			ret = _FAIL;
			goto exit;
		}


	} else if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) {
		/* Special case */
		ret = RTW_RX_HANDLED;
		goto exit;
	} else {
		if (_rtw_memcmp(myhwaddr, pattrib->dst, ETH_ALEN) && (!bmcast)) {
			*psta = rtw_get_stainfo(pstapriv, pattrib->ta);
			if (*psta == NULL) {

				/* for AP multicast issue , modify by yiwei */
				static systime send_issue_deauth_time = 0;

				/* RTW_INFO("After send deauth , %u ms has elapsed.\n", rtw_get_passing_time_ms(send_issue_deauth_time)); */

				if (rtw_get_passing_time_ms(send_issue_deauth_time) > 10000 || send_issue_deauth_time == 0) {
					send_issue_deauth_time = rtw_get_current_time();

					RTW_INFO("issue_deauth to the ap=" MAC_FMT " for the reason(7)\n", MAC_ARG(pattrib->bssid));

					issue_deauth(adapter, pattrib->bssid, WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA);
				}
			}
		}

		ret = _FAIL;
		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" fw_state:0x%x\n"
			, FUNC_ADPT_ARG(adapter), get_fwstate(pmlmepriv));
		#endif
	}

exit:


	return ret;

}

sint sta2ap_data_frame(
	_adapter *adapter,
	union recv_frame *precv_frame,
	struct sta_info **psta)
{
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *ptr = precv_frame->u.hdr.wlan_hdr;
	#else
	u8 *ptr = precv_frame->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	struct	sta_priv		*pstapriv = &adapter->stapriv;
	struct	mlme_priv	*pmlmepriv = &adapter->mlmepriv;
	unsigned char *mybssid  = get_bssid(pmlmepriv);
	sint ret = _SUCCESS;


	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) {
		/* For AP mode, RA=BSSID, TX=STA(SRC_ADDR), A3=DST_ADDR */
		if (!_rtw_memcmp(pattrib->bssid, mybssid, ETH_ALEN)) {
			ret = _FAIL;
			goto exit;
		}

		*psta = rtw_get_stainfo(pstapriv, pattrib->ta);
		if (*psta == NULL) {
			#ifdef CONFIG_RTW_DEBUG_RX_CACHE
			struct rtw_recv_pkt *rx_req = precv_frame->u.hdr.rx_req;
			static const u8 dummy_mac[6] = {0x01, 0x10, 0x02, 0x20, 0x03, 0x30};

			RTW_DBG("no STA A/H/S "MAC_FMT" / "MAC_FMT" / "MAC_FMT"\n",
				MAC_ARG(pattrib->ta), MAC_ARG(ptr + 10),
				MAC_ARG((rx_req->tx_sta ? rx_req->tx_sta->mac_addr : dummy_mac)));
			adapter->rx_logs.core_rx_ta_no_sta++;
			#endif /* CONFIG_RTW_DEBUG_RX_CACHE */

			if (!IS_RADAR_DETECTED(adapter_to_rfctl(adapter))) {
#ifndef CONFIG_CUSTOMER_ALIBABA_GENERAL
				RTW_INFO("issue_deauth to sta=" MAC_FMT " for the reason(7)\n", MAC_ARG(pattrib->src));
				issue_deauth(adapter, pattrib->src, WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA);
#endif
			}

			ret = RTW_RX_HANDLED;
			goto exit;
		}

		process_pwrbit_data(adapter, precv_frame, *psta);

		if ((get_frame_sub_type(ptr) & WIFI_QOS_DATA_TYPE) == WIFI_QOS_DATA_TYPE) {
			process_wmmps_data(adapter, precv_frame, *psta);
#ifdef CONFIG_RTW_TWT
			rtw_core_twt_inform_announce(adapter, *psta);
#endif
		}

		if (get_frame_sub_type(ptr) & BIT(6)) {
			/* only count pkts which will pass to os */
			/* count_rx_stats(adapter, precv_frame, *psta); */
			ret = RTW_RX_HANDLED;
			goto exit;
		}
	} else if ((check_fwstate(pmlmepriv, WIFI_MP_STATE) == _TRUE) &&
		   (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE)) {
		/* RTW_INFO("%s ,in WIFI_MP_STATE\n",__func__); */
		_rtw_memcpy(pattrib->dst, GetAddr1Ptr(ptr), ETH_ALEN);
		_rtw_memcpy(pattrib->src, get_addr2_ptr(ptr), ETH_ALEN);
		_rtw_memcpy(pattrib->bssid, GetAddr3Ptr(ptr), ETH_ALEN);
		_rtw_memcpy(pattrib->ra, pattrib->dst, ETH_ALEN);
		_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);


		*psta = rtw_get_stainfo(pstapriv, pattrib->bssid); /* get sta_info */
		if (*psta == NULL) {
			#ifdef DBG_RX_DROP_FRAME
			RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" can't get psta under WIFI_MP_STATE ; drop pkt\n"
				, FUNC_ADPT_ARG(adapter));
			#endif
			ret = _FAIL;
			goto exit;
		}

	} else {
		u8 *myhwaddr = adapter_mac_addr(adapter);
		if (!_rtw_memcmp(pattrib->ra, myhwaddr, ETH_ALEN)) {
			ret = RTW_RX_HANDLED;
			goto exit;
		}
#ifndef CONFIG_CUSTOMER_ALIBABA_GENERAL
		RTW_INFO("issue_deauth to sta=" MAC_FMT " for the reason(7)\n", MAC_ARG(pattrib->src));
		issue_deauth(adapter, pattrib->src, WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA);
#endif
		ret = RTW_RX_HANDLED;
		goto exit;
	}

exit:


	return ret;

}

#ifdef CONFIG_AP_MODE
/* CONFIG_VW_REFINE */
sint rtw_proccess_pspoll(_adapter *adapter, union recv_frame *precv_frame, struct sta_info *psta)
{
	u8 *pframe = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	struct sta_priv *pstapriv = &adapter->stapriv;
	u16 aid;
	u8 wmmps_ac = 0;
	u8 q_idx;

	aid = GetAid(pframe);
	if (psta->phl_sta->aid != aid)
		return _FAIL;

	q_idx = rtw_get_txq_idx(pattrib->priority);
	if (psta->uapsd_bitmap & BIT(q_idx))
		return _FAIL;

	if (psta->state & WIFI_STA_ALIVE_CHK_STATE) {
		RTW_INFO("%s alive check-rx ps-poll\n", __func__);
		psta->expire_to = pstapriv->expire_to;
		psta->state ^= WIFI_STA_ALIVE_CHK_STATE;
	}

	/* notify fw to release a packet by pspoll action using h2c */
	rtw_phl_host_getpkt(adapter->dvobj->phl, psta->phl_sta->macid, 0);

	if (psta->state & WIFI_SLEEP_STATE) {
		struct sta_xmit_priv *pstaxmitpriv = &psta->sta_xmitpriv;
		_queue *sta_queue = &adapter->dvobj->ps_trigger_sta_queue;

		_rtw_spinlock_bh(&sta_queue->lock);
		pstaxmitpriv->ps_trigger_type |= BIT0;
		if (rtw_is_list_empty(&pstaxmitpriv->ps_trigger) == _TRUE) {
			rtw_list_insert_tail(&pstaxmitpriv->ps_trigger, &sta_queue->queue);
			sta_queue->qlen++;
//#if !defined(CONFIG_ONE_TXQ)
			if (adapter->dvobj->tx_mode != 2)
				rtw_tasklet_hi_schedule(&adapter->dvobj->ps_trigger_tasklet);
//#endif
		}
		_rtw_spinunlock_bh(&sta_queue->lock);
	}
	return _SUCCESS;
}
#endif /*CONFIG_AP_MODE*/
sint validate_recv_ctrl_frame(_adapter *padapter, union recv_frame *precv_frame)
{
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	struct sta_info *psta = NULL;
	/* uint len = precv_frame->u.hdr.len; */

	/* RTW_INFO("+validate_recv_ctrl_frame\n"); */

#ifdef RTW_CORE_PKT_TRACE
	if(padapter->pkt_trace_enable)
	{
		rtw_wifi_rx_prepare_pkt_trace(padapter,precv_frame);
		RTW_RX_TRACE(padapter,&precv_frame->u.hdr.attrib.pktinfo);
	}
#endif

	if (GetFrameType(pframe) != WIFI_CTRL_TYPE)
	{
#ifdef RTW_CORE_PKT_TRACE
		if(padapter->pkt_trace_enable)
		{
			RTW_RX_TRACE(padapter,&precv_frame->u.hdr.attrib.pktinfo);
		}
#endif
		return _FAIL;
	}

	/* receive the frames that ra(a1) is my address */
	if (!_rtw_memcmp(GetAddr1Ptr(pframe), adapter_mac_addr(padapter), ETH_ALEN))
	{
#ifdef RTW_CORE_PKT_TRACE
		if(padapter->pkt_trace_enable)
		{
			RTW_RX_TRACE(padapter,&precv_frame->u.hdr.attrib.pktinfo);
		}
#endif
		return _FAIL;
	}

	psta = rtw_get_stainfo(pstapriv, get_addr2_ptr(pframe));
	if (psta == NULL)
	{
#ifdef RTW_CORE_PKT_TRACE
		if(padapter->pkt_trace_enable)
		{
			RTW_RX_TRACE(padapter,&precv_frame->u.hdr.attrib.pktinfo);
		}
#endif
		return _FAIL;
	}

	/* for rx pkt statistics */
	psta->sta_stats.last_rx_time = rtw_get_current_time();
	psta->sta_stats.rx_ctrl_pkts++;

	switch (get_frame_sub_type(pframe)) {
	#ifdef CONFIG_AP_MODE
	case WIFI_PSPOLL :
		{
			sint rst;

			rst = rtw_proccess_pspoll(padapter, precv_frame, psta);
			/*RTW_INFO(FUNC_ADPT_FMT" pspoll handle %d\n", FUNC_ADPT_ARG(padapter), rst);*/
		}
		break;
	#endif
	#ifdef CONFIG_BEAMFORMING
	case WIFI_NDPA :
		rtw_beamforming_get_ndpa_frame(padapter, precv_frame);
		break;
	#endif
	case WIFI_BAR :
		rtw_process_bar_frame(padapter, precv_frame);
		break;
	default :
		break;
	}
	return _FAIL;

}

#if defined(CONFIG_IEEE80211W) || defined(CONFIG_RTW_MESH)
static sint validate_mgmt_protect(_adapter *adapter, union recv_frame *precv_frame)
{
#define DBG_VALIDATE_MGMT_PROTECT 0
#define DBG_VALIDATE_MGMT_DEC 0

	struct security_priv *sec = &adapter->securitypriv;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	struct sta_info	*psta = precv_frame->u.hdr.psta;
	u8 *ptr;
	u8 type;
	u8 subtype;
	u8 is_bmc;
	u8 category = 0xFF;

#ifdef CONFIG_IEEE80211W
	const u8 *igtk;
	u16 igtk_id;
	u64* ipn;
	enum security_type bip_cipher;
#endif

	sint ret;

#ifdef CONFIG_RTW_MESH
	if (MLME_IS_MESH(adapter)) {
		if (!adapter->mesh_info.mesh_auth_id)
			return pattrib->privacy ? _FAIL : _SUCCESS;
	} else
#endif
	if (SEC_IS_BIP_KEY_INSTALLED(sec) == _FALSE)
		return _SUCCESS;

	ptr = precv_frame->u.hdr.rx_data;
	type = GetFrameType(ptr);
	subtype = get_frame_sub_type(ptr); /* bit(7)~bit(2) */
	is_bmc = IS_MCAST(GetAddr1Ptr(ptr));

#if DBG_VALIDATE_MGMT_PROTECT
	if (subtype == WIFI_DEAUTH) {
		RTW_INFO(FUNC_ADPT_FMT" bmc:%u, deauth, privacy:%u, encrypt:%u, bdecrypted:%u\n"
			, FUNC_ADPT_ARG(adapter)
			, is_bmc, pattrib->privacy, pattrib->encrypt, pattrib->bdecrypted);
	} else if (subtype == WIFI_DISASSOC) {
		RTW_INFO(FUNC_ADPT_FMT" bmc:%u, disassoc, privacy:%u, encrypt:%u, bdecrypted:%u\n"
			, FUNC_ADPT_ARG(adapter)
			, is_bmc, pattrib->privacy, pattrib->encrypt, pattrib->bdecrypted);
	} if (subtype == WIFI_ACTION) {
		if (pattrib->privacy) {
			RTW_INFO(FUNC_ADPT_FMT" bmc:%u, action(?), privacy:%u, encrypt:%u, bdecrypted:%u\n"
				, FUNC_ADPT_ARG(adapter)
				, is_bmc, pattrib->privacy, pattrib->encrypt, pattrib->bdecrypted);
		} else {
			RTW_INFO(FUNC_ADPT_FMT" bmc:%u, action(%u), privacy:%u, encrypt:%u, bdecrypted:%u\n"
				, FUNC_ADPT_ARG(adapter), is_bmc
				, *(ptr + sizeof(struct rtw_ieee80211_hdr_3addr))
				, pattrib->privacy, pattrib->encrypt, pattrib->bdecrypted);
		}
	}
#endif

	if (!pattrib->privacy) {
		if (!psta || !(psta->flags & WLAN_STA_MFP)) {
			/* peer is not MFP capable, no need to check */
			goto exit;
		}

		if (subtype == WIFI_ACTION)
			category = *(ptr + sizeof(struct rtw_ieee80211_hdr_3addr));

		if (is_bmc) { /* broadcast cases */
			if (subtype == WIFI_ACTION) {
				if (CATEGORY_IS_GROUP_PRIVACY(category)) {
					/* drop broadcast group privacy action frame without encryption */
					#if DBG_VALIDATE_MGMT_PROTECT
					RTW_INFO(FUNC_ADPT_FMT" broadcast gp action(%u) w/o encrypt\n"
						, FUNC_ADPT_ARG(adapter), category);
					#endif
					goto fail;
				}
				if (CATEGORY_IS_ROBUST(category)) {
					/* broadcast robust action frame need BIP check */
					goto bip_verify;
				}
			}
			if (subtype == WIFI_DEAUTH || subtype == WIFI_DISASSOC) {
				/* broadcast deauth or disassoc frame need BIP check */
				goto bip_verify;
			}
			goto exit;

		} else { /* unicast cases */
#if defined(CONFIG_IEEE80211W)
			if (subtype == WIFI_DEAUTH || subtype == WIFI_DISASSOC) {
				if (!MLME_IS_STA(adapter)) {
					unsigned short reason = le16_to_cpu(*(unsigned short *)(ptr + WLAN_HDR_A3_LEN));

					#if DBG_VALIDATE_MGMT_PROTECT
					RTW_INFO(FUNC_ADPT_FMT" unicast %s, reason=%d w/o encrypt\n"
						, FUNC_ADPT_ARG(adapter), subtype == WIFI_DEAUTH ? "deauth" : "disassoc", reason);
					#endif
#if !defined(CONFIG_IOCTL_CFG80211)
					if (reason == 6 || reason == 7) {
						/* issue sa query request */
						if (psta->sa_query_cnt == 0 && psta->sa_query_timed_out == 0) {
							psta->sa_query_cnt++;
							psta->sa_query_timed_out = 0;
							psta->sa_query_end = jiffies + RTL_MILISECONDS_TO_JIFFIES(SA_QUERY_MAX_TO);
							issue_action_SA_Query(adapter, psta->phl_sta->mac_addr, 0, 0, IEEE80211W_RIGHT_KEY);
							_set_timer(&psta->dot11w_expire_timer, SA_QUERY_RETRY_TO);
						}
					}
#endif
					goto fail;
				}
			}
#endif

			if (subtype == WIFI_ACTION && CATEGORY_IS_ROBUST(category)) {
				if (psta->bpairwise_key_installed == _TRUE) {
					#if DBG_VALIDATE_MGMT_PROTECT
					RTW_INFO(FUNC_ADPT_FMT" unicast robust action(%d) w/o encrypt\n"
						, FUNC_ADPT_ARG(adapter), category);
					#endif
					goto fail;
				}
			}
			goto exit;
		}

bip_verify:
#ifdef CONFIG_IEEE80211W
		#ifdef CONFIG_RTW_MESH
		if (MLME_IS_MESH(adapter)) {
			if (psta->igtk_bmp) {
				bip_cipher = psta->dot11wCipher;
				igtk = psta->igtk.skey;
				igtk_id = psta->igtk_id;
				ipn = &psta->igtk_pn.val;
			} else {
				/* mesh MFP without IGTK */
				goto exit;
			}
		} else
		#endif
		{
			bip_cipher = sec->dot11wCipher;
			igtk = sec->dot11wBIPKey[sec->dot11wBIPKeyid].skey;
			igtk_id = sec->dot11wBIPKeyid;
			ipn = &sec->dot11wBIPrxpn.val;
#ifdef CONFIG_RTL_CFG80211_WAPI_SUPPORT
			if (bip_cipher == _BIP_CMAC_SM4_128_)
				ipn = (u64 *)adapter->wapiApInfo.imk_rx_pn;
#endif
		}

		/* verify BIP MME IE */
		ret = rtw_bip_verify(bip_cipher,
				     get_recvframe_data(precv_frame),
				     get_recvframe_len(precv_frame),
				     igtk, igtk_id, ipn, (u8 *)precv_frame);

		if (ret == _FAIL) {
			/* RTW_INFO("802.11w BIP verify fail\n"); */
			goto fail;

		} else if (ret == RTW_RX_HANDLED) {
			#if DBG_VALIDATE_MGMT_PROTECT
			RTW_INFO(FUNC_ADPT_FMT" none protected packet\n", FUNC_ADPT_ARG(adapter));
			#endif
			goto fail;
		}
#endif /* CONFIG_IEEE80211W */
		goto exit;
	}

	if (!psta || !(psta->flags & WLAN_STA_MFP)) {
		/* not peer or peer is not MFP capable, drop it */
		goto fail;
	}

	/* cases to decrypt mgmt frame */
#if 0
	pattrib->bdecrypted = 0;
#endif
#ifdef CONFIG_RTW_MESH
	if (is_bmc)
		pattrib->encrypt = psta->group_privacy;
	else
#endif
	pattrib->encrypt = psta->dot118021XPrivacy;
	pattrib->hdrlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	/* set iv and icv length */
	SET_ICE_IV_LEN(pattrib->iv_len, pattrib->icv_len, pattrib->encrypt);
	_rtw_memcpy(pattrib->ra, GetAddr1Ptr(ptr), ETH_ALEN);
	_rtw_memcpy(pattrib->ta, get_addr2_ptr(ptr), ETH_ALEN);

#ifdef CONFIG_RTL_CFG80211_WAPI_SUPPORT
	if (sec->dot11AuthAlgrthm == dot11AuthAlgrthm_WAPI) {
		if (rtw_validate_wapi_frame(adapter, precv_frame) == _FAIL) {
			RTW_INFO("%s: Invalid WAPI frame!\n", __func__);
			goto fail;
		}
	}
#endif

#if DBG_VALIDATE_MGMT_DEC
	RTW_PRINT_DUMP("MGMT before decryption", ptr, precv_frame->u.hdr.len);
#endif

	precv_frame = decryptor(adapter, precv_frame);
	if (!precv_frame) {
		#if DBG_VALIDATE_MGMT_PROTECT
		RTW_INFO(FUNC_ADPT_FMT" mgmt descrypt fail  !!!!!!!!!\n", FUNC_ADPT_ARG(adapter));
		#endif
		goto fail;
	}

	recvframe_pull_tail(precv_frame, pattrib->icv_len);
	ptr = precv_frame->u.hdr.rx_data;
	_rtw_memmove(ptr + pattrib->iv_len, ptr, pattrib->hdrlen);
	recvframe_pull(precv_frame, pattrib->iv_len);
	ptr = precv_frame->u.hdr.rx_data;

#if DBG_VALIDATE_MGMT_DEC
	RTW_PRINT_DUMP("MGMT after decryption", ptr, precv_frame->u.hdr.len);
#endif

exit:
	return _SUCCESS;

fail:
	return _FAIL;
}
#endif /* defined(CONFIG_IEEE80211W) || defined(CONFIG_RTW_MESH) */

s32 recvframe_chk_defrag(_adapter *padapter, union recv_frame **pprecv_frame);

sint validate_recv_mgnt_frame(_adapter *padapter, union recv_frame *precv_frame)
{
	struct recv_priv  *precvpriv = &padapter->recvpriv;
	struct sta_info *psta = precv_frame->u.hdr.psta
		= rtw_get_stainfo(&padapter->stapriv, get_addr2_ptr(precv_frame->u.hdr.rx_data));

#if defined(CONFIG_IEEE80211W) || defined(CONFIG_RTW_MESH)
	if (validate_mgmt_protect(padapter, precv_frame) == _FAIL) {
		DBG_COUNTER(padapter->rx_logs.core_rx_pre_mgmt_err_80211w);
		goto exit;
	}
#endif

	if (recvframe_chk_defrag(padapter, &precv_frame) != CORE_RX_CONTINUE)
		return _SUCCESS;

	/* for rx pkt statistics */
	if (psta) {
		psta->sta_stats.last_rx_time = rtw_get_current_time();
		psta->sta_stats.rx_mgnt_pkts++;
		if (get_frame_sub_type(precv_frame->u.hdr.rx_data) == WIFI_BEACON)
			psta->sta_stats.rx_beacon_pkts++;
		else if (get_frame_sub_type(precv_frame->u.hdr.rx_data) == WIFI_PROBEREQ)
			psta->sta_stats.rx_probereq_pkts++;
		else if (get_frame_sub_type(precv_frame->u.hdr.rx_data) == WIFI_PROBERSP) {
			if (_rtw_memcmp(adapter_mac_addr(padapter), GetAddr1Ptr(precv_frame->u.hdr.rx_data), ETH_ALEN) == _TRUE)
				psta->sta_stats.rx_probersp_pkts++;
			else if (is_broadcast_mac_addr(GetAddr1Ptr(precv_frame->u.hdr.rx_data))
				|| is_multicast_mac_addr(GetAddr1Ptr(precv_frame->u.hdr.rx_data)))
				psta->sta_stats.rx_probersp_bm_pkts++;
			else
				psta->sta_stats.rx_probersp_uo_pkts++;
		}
		process_pwrbit_data(padapter, precv_frame, psta);
	}

	precvpriv->rx_mgmt_pkts++;

#ifdef RTW_CORE_PKT_TRACE
	if(padapter->pkt_trace_enable)
	{
		rtw_wifi_rx_prepare_pkt_trace(padapter,precv_frame);
		RTW_RX_TRACE(padapter,&precv_frame->u.hdr.attrib.pktinfo);
	}
#endif

	mgt_dispatcher(padapter, precv_frame);

#if defined(CONFIG_IEEE80211W) || defined(CONFIG_RTW_MESH)
exit:
#endif
	return _SUCCESS;

}

sint validate_recv_data_frame(_adapter *adapter, union recv_frame *precv_frame)
{
	u8 bretry, a4_shift;
	struct sta_info *psta = NULL;
	u8 *ptr = precv_frame->u.hdr.rx_data;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	struct rtw_recv_pkt	*rx_req = precv_frame->u.hdr.rx_req;
	u8 *whdr = precv_frame->u.hdr.wlan_hdr;
	#else
	u8 *whdr = ptr;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	struct rx_pkt_attrib	*pattrib = &precv_frame->u.hdr.attrib;
	struct security_priv	*psecuritypriv = &adapter->securitypriv;
	sint ret = _SUCCESS;

	bretry = GetRetry(whdr);
	a4_shift = (pattrib->to_fr_ds == 3) ? ETH_ALEN : 0;

#ifdef DEBUG_PHL_RX
	if(bretry)
		adapter->cnt_core_rx_data_retry++;
#endif

	/* some address fields are different when using AMSDU */
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV
	if(pattrib->amsdu_cut)
		pattrib->amsdu = 0;
	else
	#endif
	if (pattrib->qos)
		pattrib->amsdu = GetAMsdu(whdr + WLAN_HDR_A3_LEN + a4_shift);
	else
		pattrib->amsdu = 0;

#ifdef CONFIG_RTW_MESH
	if (MLME_IS_MESH(adapter)) {
		ret = rtw_mesh_rx_data_validate_hdr(adapter, precv_frame, &psta);
		goto pre_validate_status_chk;
	}
#endif

	switch (pattrib->to_fr_ds) {
	case 0:
		_rtw_memcpy(pattrib->ra, GetAddr1Ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->ta, get_addr2_ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->dst, GetAddr1Ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->src, get_addr2_ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->bssid, GetAddr3Ptr(whdr), ETH_ALEN);
		ret = sta2sta_data_frame(adapter, precv_frame, &psta);
		break;

	case 1:
		_rtw_memcpy(pattrib->ra, GetAddr1Ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->ta, get_addr2_ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->dst, GetAddr1Ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->src, GetAddr3Ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->bssid, get_addr2_ptr(whdr), ETH_ALEN);
		ret = ap2sta_data_frame(adapter, precv_frame, &psta);
		break;

	case 2:
		_rtw_memcpy(pattrib->ra, GetAddr1Ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->ta, get_addr2_ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->dst, GetAddr3Ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->src, get_addr2_ptr(whdr), ETH_ALEN);
		_rtw_memcpy(pattrib->bssid, GetAddr1Ptr(whdr), ETH_ALEN);
		ret = sta2ap_data_frame(adapter, precv_frame, &psta);
		break;

	case 3:
#ifdef CONFIG_RTW_A4_STA
		/* rtw_ap_rx_data_validate_hdr */
		ret = core_a4_data_validate_hdr(adapter, precv_frame, &psta);
		break;
#endif
	default:
		/* WDS is not supported */
		ret = _FAIL;
		break;
	}

#if defined(RTK_WLAN_EVENT_INDICATE) && defined(CONFIG_RTW_MULTI_AP)
	if((pattrib->src[0] ==0xaa) && (pattrib->src[1] ==0xbb) && (pattrib->src[2] ==0xcc)
		&& (pattrib->src[3] == adapter->mac_addr[3]) && (pattrib->src[4] == adapter->mac_addr[4]) && (pattrib->src[5] == adapter->mac_addr[5])){

#if 0
		printk("%s[%d] rx from sta[%02X:%02X:%02X:%02X:%02X:%02X]\n", __FUNCTION__, __LINE__,
			pattrib->src[0], pattrib->src[1], pattrib->src[2],
			pattrib->src[3], pattrib->src[4], pattrib->src[5]);

		printk("arp packet:");
		for(j=0;j<64;j++){
			printk("%02x",ptr[j]);
		}
		printk("\n");
#endif
		RTW_INFO("[%s %d] receive WIFI_EZMESH_ARP_LOOP in driver on if:%s, send event!!!\n",__FUNCTION__,__LINE__,adapter->pnetdev->name);

		rtk_wlan_event_indicate(adapter->pnetdev->name, WIFI_EZMESH_ARP_LOOP, adapter->mac_addr, 0);

		ret = _FAIL;
		goto exit;
	}
#endif

#ifdef CONFIG_RTW_MESH
pre_validate_status_chk:
#endif
	if (ret == _FAIL) {
		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" case:%d, res:%d, ra="MAC_FMT", ta="MAC_FMT"\n"
			, FUNC_ADPT_ARG(adapter), pattrib->to_fr_ds, ret, MAC_ARG(GetAddr1Ptr(ptr)), MAC_ARG(get_addr2_ptr(ptr)));
		#endif
		goto exit;
	} else if (ret == RTW_RX_HANDLED)
		goto exit;


	if (psta == NULL) {
		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" psta == NULL, ra="MAC_FMT", ta="MAC_FMT"\n"
			, FUNC_ADPT_ARG(adapter), MAC_ARG(GetAddr1Ptr(ptr)), MAC_ARG(get_addr2_ptr(ptr)));
		#endif
		ret = _FAIL;
		goto exit;
	}

	precv_frame->u.hdr.psta = psta;
	precv_frame->u.hdr.preorder_ctrl = NULL;
	pattrib->ack_policy = 0;

	/* parsing QC field */
	if (pattrib->qos == 1) {
		pattrib->priority = GetPriority((whdr + WLAN_HDR_A3_LEN + a4_shift)); /* point to Qos field*/
		pattrib->ack_policy = GetAckpolicy((whdr + WLAN_HDR_A3_LEN + a4_shift));
		pattrib->hdrlen = WLAN_HDR_A3_QOS_LEN + a4_shift;
		if (pattrib->priority != 0 && pattrib->priority != 3)
			adapter->recvpriv.is_any_non_be_pkts = _TRUE;
		else
			adapter->recvpriv.is_any_non_be_pkts = _FALSE;
	} else {
		pattrib->priority = 0;
		pattrib->hdrlen = WLAN_HDR_A3_LEN + a4_shift;
	}

	if (pattrib->order) /* HT-CTRL 11n */
		pattrib->hdrlen += 4;

	/* decache, drop duplicate recv packets */
	ret = recv_decache(precv_frame);
	if (ret  == _FAIL)
		goto exit;

#ifdef CONFIG_TDLS
	if ((psta->tdls_sta_state & TDLS_LINKED_STATE) && (psta->dot118021XPrivacy == _AES_))
		pattrib->encrypt = psta->dot118021XPrivacy;
	else
#endif /* CONFIG_TDLS */
		GET_ENCRY_ALGO(psecuritypriv, psta, pattrib->encrypt, IS_MCAST(pattrib->ra));

	if (pattrib->privacy) {
		if (!pattrib->encrypt) {
			RTW_WARN("RX drop: encrypted packet!\n");
			ret = _FAIL;
			goto exit;
		}
		#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
		if(pattrib->amsdu_cut)
			pattrib->iv_len = pattrib->icv_len = 0;
		else
		#endif
			SET_ICE_IV_LEN(pattrib->iv_len, pattrib->icv_len, pattrib->encrypt);
	} else {
		if (pattrib->encrypt) {
			u16 eth_type;
			_rtw_memcpy(&eth_type, ptr + pattrib->hdrlen + RATTRIB_GET_MCTRL_LEN(pattrib) + LLC_HEADER_SIZE, 2);
			eth_type = ntohs(eth_type);
			if (eth_type == ETH_P_8021Q) {
				RTW_INFO("[%s %d]RX VLAN data,eth_type shifts 4 bytes(%04X)\n", __FUNCTION__, __LINE__, eth_type);
				_rtw_memcpy(&eth_type, ptr + pattrib->hdrlen + RATTRIB_GET_MCTRL_LEN(pattrib) + LLC_HEADER_SIZE + 4, 2);
				eth_type = ntohs(eth_type);
			}
		#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
			if (pattrib->amsdu || (eth_type != ETH_P_PAE && eth_type != ETH_P_WAPI)) {
		#else
			if (pattrib->amsdu || eth_type != ETH_P_PAE) {
		#endif
				RTW_WARN("RX drop: unencrypted packet!\n");
				ret = _FAIL;
				goto exit;
			}
		}
		pattrib->encrypt = 0;
		pattrib->iv_len = pattrib->icv_len = 0;
	}

	if (IS_MCAST(pattrib->ra)) {
		if (recv_bcast_pn_decache(precv_frame) == _FAIL) {
			#ifdef DBG_RX_DROP_FRAME
			RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" recv_bcast_pn_decache return _FAIL for sta="MAC_FMT"\n"
				, FUNC_ADPT_ARG(adapter), MAC_ARG(psta->phl_sta->mac_addr));
			#endif
			ret = _FAIL;
			goto exit;
		}
	}

#ifdef CONFIG_RTW_MESH
	if (!pattrib->amsdu
		&& pattrib->mesh_ctrl_present
		&& (!pattrib->encrypt || pattrib->bdecrypted))
		ret = rtw_mesh_rx_validate_mctrl_non_amsdu(adapter, precv_frame);
#endif

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	if (psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_WAPI) {
		if (rtw_validate_wapi_frame(adapter, precv_frame) == _FAIL) {
			RTW_INFO("RX drop: invalid WAPI frame!\n");
			ret = _FAIL;
			goto exit;
		}
	}
#endif

exit:

#ifdef RTW_PHL_TEST_FPGA
	return _SUCCESS;
#endif
	return ret;
}

static inline void dump_rx_packet(u8 *ptr)
{
	int i;

	RTW_INFO("#############################\n");
	for (i = 0; i < 64; i = i + 8)
		RTW_INFO("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:\n", *(ptr + i),
			*(ptr + i + 1), *(ptr + i + 2) , *(ptr + i + 3) , *(ptr + i + 4), *(ptr + i + 5), *(ptr + i + 6), *(ptr + i + 7));
	RTW_INFO("#############################\n");
}

#ifdef CONFIG_SNR_RPT
static void rx_process_snr_info(union recv_frame *precvframe)
{
	_adapter *padapter = precvframe->u.hdr.adapter;
	struct rx_pkt_attrib *pattrib = &precvframe->u.hdr.attrib;
	struct phydm_phyinfo_struct *phy_info = &pattrib->phy_info;
	u8 *wlanhdr = NULL;
	u8 *ta, *ra;
	u8 is_ra_bmc;
	struct sta_priv *pstapriv;
	struct sta_info *psta = NULL;
	int i;

	wlanhdr = precvframe->u.hdr.rx_data;
	ta = get_ta(wlanhdr);
	pstapriv = &padapter->stapriv;
	psta = rtw_get_stainfo(pstapriv, ta);

	if (psta) {
		_rtw_spinlock_bh(&psta->lock);
		if (phy_info->is_valid) {
			psta->snr_num++;
			for ( i = 0; i < RTW_PHL_MAX_RF_PATH; i++) {
				psta->snr_fd_total[i] += pattrib->phy_info.snr_fd[i];
				psta->snr_td_total[i] += pattrib->phy_info.snr_td[i];
				psta->snr_fd_avg[i] = psta->snr_fd_total[i]/psta->snr_num;
				psta->snr_td_avg[i] = psta->snr_td_total[i]/psta->snr_num;
				#if 0
				RTW_INFO("path = %d, AVG_SNR_FD = %d, AVG_SNR_TD = %d\n",
				i, psta->snr_fd_avg[i], psta->snr_td_avg[i]);
				#endif
			}

		}
		_rtw_spinunlock_bh(&psta->lock);
	}

}
#endif /* CONFIG_SNR_RPT */

sint validate_recv_frame(_adapter *adapter, union recv_frame *precv_frame)
{
	/* shall check frame subtype, to / from ds, da, bssid */

	/* then call check if rx seq/frag. duplicated. */

	u8 type;
	u8 subtype;
	sint retval = _SUCCESS;

	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	struct recv_priv  *precvpriv = &adapter->recvpriv;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	struct rtw_recv_pkt *rx_req = (struct rtw_recv_pkt *)precv_frame->u.hdr.rx_req;
	struct rtw_r_meta_data *rx_mdata;
	u8 *whdr = precv_frame->u.hdr.wlan_hdr;
	#else
	u8 *whdr = precv_frame->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	u8 *ptr = precv_frame->u.hdr.rx_data;
	u8  ver = (unsigned char)(*whdr) & 0x3 ;
#if 1 /*def CONFIG_FIND_BEST_CHANNEL*/
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;
#endif

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	if (rx_req == NULL) {
		RTW_ERR(ADPT_FMT ": NULL rx_req\n", ADPT_ARG(adapter));
		return _FAIL;
	}
	rx_mdata = &rx_req->mdata;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

#ifdef CONFIG_TDLS
	struct tdls_info *ptdlsinfo = &adapter->tdlsinfo;
#endif /* CONFIG_TDLS */
#ifdef CONFIG_WAPI_SUPPORT
	PRT_WAPI_T	pWapiInfo = &adapter->wapiInfo;
	struct recv_frame_hdr *phdr = &precv_frame->u.hdr;
	u8 wai_pkt = 0;
	u16 sc;
	u8	external_len = 0;
#endif

#if 1 /*def CONFIG_FIND_BEST_CHANNEL*/
	if (pmlmeext->sitesurvey_res.state == SCAN_PROCESS) {
		int ch_set_idx = rtw_chset_search_ch(rfctl->channel_set, rtw_phl_get_cur_ch(adapter->phl_role));
		if (ch_set_idx >= 0)
			rfctl->channel_set[ch_set_idx].rx_count++;
	}
#endif

#ifdef CONFIG_TDLS
	if (ptdlsinfo->ch_sensing == 1 && ptdlsinfo->cur_channel != 0)
		ptdlsinfo->collect_pkt_num[ptdlsinfo->cur_channel - 1]++;
#endif /* CONFIG_TDLS */

#ifdef RTK_DMP_PLATFORM
	if (0) {
		RTW_INFO("++\n");
		{
			int i;
			for (i = 0; i < 64; i = i + 8)
				RTW_INFO("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:", *(ptr + i),
					*(ptr + i + 1), *(ptr + i + 2) , *(ptr + i + 3) , *(ptr + i + 4), *(ptr + i + 5), *(ptr + i + 6), *(ptr + i + 7));

		}
		RTW_INFO("--\n");
	}
#endif /* RTK_DMP_PLATFORM */

	/* add version chk */
	if (ver != 0) {
		retval = _FAIL;
		DBG_COUNTER(adapter->rx_logs.core_rx_pre_ver_err);
		goto exit;
	}

	type = GetFrameType(whdr);
	subtype = get_frame_sub_type(whdr); /* bit(7)~bit(2) */

	pattrib->to_fr_ds = get_tofr_ds(whdr);

	pattrib->frag_num = GetFragNum(whdr);
	pattrib->seq_num = GetSequence(whdr);

	pattrib->pw_save = GetPwrMgt(whdr);
	pattrib->mfrag = GetMFrag(whdr);
	pattrib->mdata = GetMData(whdr);
	pattrib->privacy = GetPrivacy(whdr);
	pattrib->order = GetOrder(whdr);
#ifdef CONFIG_WAPI_SUPPORT
	sc = (pattrib->seq_num << 4) | pattrib->frag_num;
#endif

#ifdef RTW_PHL_DBG_CMD
	pattrib->wl_type = type;
	pattrib->wl_subtype = subtype;
#ifdef DEBUG_PHL_RX
	if (type == WIFI_DATA_TYPE)
		adapter->cnt_core_rx_data++;
	else
		adapter->cnt_core_rx_mgmt++;
#endif /* DEBUG_PHL_RX */
#endif /* RTW_PHL_DBG_CMD */

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	pattrib->amsdu_cut = rx_mdata->amsdu_cut;
	#endif

#if 0 /* Dump rx packets */
	{
		u8 bDumpRxPkt = 0;

		rtw_hal_get_def_var(adapter, HAL_DEF_DBG_DUMP_RXPKT, &(bDumpRxPkt));
		if (bDumpRxPkt == 1) /* dump all rx packets */
			dump_rx_packet(ptr);
		else if ((bDumpRxPkt == 2) && (type == WIFI_MGT_TYPE))
			dump_rx_packet(ptr);
		else if ((bDumpRxPkt == 3) && (type == WIFI_DATA_TYPE))
			dump_rx_packet(ptr);
	}
#endif

#ifdef CONFIG_SNR_RPT
	rx_process_snr_info(precv_frame);
#endif /* CONFIG_SNR_RPT */

	switch (type) {
	case WIFI_MGT_TYPE: /* mgnt */
		DBG_COUNTER(adapter->rx_logs.core_rx_pre_mgmt);
		retval = validate_recv_mgnt_frame(adapter, precv_frame);
		if (retval == _FAIL) {
			DBG_COUNTER(adapter->rx_logs.core_rx_pre_mgmt_err);
		}
		retval = _FAIL; /* only data frame return _SUCCESS */
		break;
	case WIFI_CTRL_TYPE: /* ctrl */
		DBG_COUNTER(adapter->rx_logs.core_rx_pre_ctrl);
		retval = validate_recv_ctrl_frame(adapter, precv_frame);
		if (retval == _FAIL) {
			DBG_COUNTER(adapter->rx_logs.core_rx_pre_ctrl_err);
		}
		retval = _FAIL; /* only data frame return _SUCCESS */
		break;
	case WIFI_DATA_TYPE: /* data */
		DBG_COUNTER(adapter->rx_logs.core_rx_pre_data);
#ifdef CONFIG_WAPI_SUPPORT
		if (pattrib->qos)
			external_len = 2;
		else
			external_len = 0;

		wai_pkt = rtw_wapi_is_wai_packet(adapter, whdr);

		phdr->bIsWaiPacket = wai_pkt;

		if (wai_pkt != 0) {
			if (sc != adapter->wapiInfo.wapiSeqnumAndFragNum)
				adapter->wapiInfo.wapiSeqnumAndFragNum = sc;
			else {
				retval = _FAIL;
				DBG_COUNTER(adapter->rx_logs.core_rx_pre_data_wapi_seq_err);
				break;
			}
		} else {

			if (rtw_wapi_drop_for_key_absent(adapter, get_addr2_ptr(whdr))) {
				retval = _FAIL;
				WAPI_TRACE(WAPI_RX, "drop for key absent for rx\n");
				DBG_COUNTER(adapter->rx_logs.core_rx_pre_data_wapi_key_err);
				break;
			}
		}

#endif

		pattrib->qos = (subtype & BIT(7)) ? 1 : 0;
		if (pattrib->qos && pattrib->order)
			process_a_control_subfield(adapter, precv_frame);
		retval = validate_recv_data_frame(adapter, precv_frame);
		if (retval == _FAIL) {
			precvpriv->dbg_rx_drop_count++;
			DBG_COUNTER(adapter->rx_logs.core_rx_pre_data_err);
		} else if (retval == _SUCCESS) {
			#ifdef DBG_RX_DUMP_EAP
			if (!pattrib->encrypt || pattrib->bdecrypted) {
				u8 bDumpRxPkt;
				u16 eth_type;

				/* dump eapol */
				rtw_hal_get_def_var(adapter, HAL_DEF_DBG_DUMP_RXPKT, &(bDumpRxPkt));
				/* get ether_type */
				#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
				if (rx_mdata->hdr_conv)
					_rtw_memcpy(&eth_type, pt + (2 * ETH_ALEN), 2);
				else
				#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
					_rtw_memcpy(&eth_type, ptr + pattrib->hdrlen + pattrib->iv_len + RATTRIB_GET_MCTRL_LEN(pattrib) + LLC_HEADER_SIZE, 2);
				eth_type = ntohs((unsigned short) eth_type);
				if ((bDumpRxPkt == 4) && (eth_type == 0x888e))
					dump_rx_packet(ptr);
			}
			#endif

			#ifdef RTW_CORE_PKT_TRACE
			if (!pattrib->encrypt || pattrib->bdecrypted) {
				if (adapter->pkt_trace_enable) {
					rtw_wifi_rx_prepare_pkt_trace(adapter,precv_frame);
					RTW_RX_TRACE(adapter,&pattrib->pktinfo);
				}
			}
			#endif
			if (!pattrib->amsdu) {
				if (EQ_MAC_ADDR(adapter->br_mac, pattrib->src)) {
					retval = _FAIL;
					precvpriv->dbg_rx_drop_count++;
					DBG_COUNTER(adapter->rx_logs.core_rx_pre_data_err);
					RTW_PRINT("RX drop: SA equals br0 mac.\n");

#if defined(RTK_WLAN_EVENT_INDICATE) && defined(CONFIG_RTW_MULTI_AP)
					RTW_INFO("[%s %d] receive WIFI_SA_EQUAL_BR0_MAC in driver on if:%s, send event!!!\n",__FUNCTION__,__LINE__,adapter->pnetdev->name);
					rtk_wlan_event_indicate(adapter->pnetdev->name, WIFI_SA_EQUAL_BR0_MAC, adapter->mac_addr, 0);
#endif
				}
			}
		} else
			DBG_COUNTER(adapter->rx_logs.core_rx_pre_data_handled);
		break;
	default:
		DBG_COUNTER(adapter->rx_logs.core_rx_pre_unknown);
		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" fail! type=0x%x\n"
			, FUNC_ADPT_ARG(adapter), type);
		#endif
		retval = _FAIL;
		break;
	}

exit:
	return retval;
}


/* remove the wlanhdr and add the eth_hdr */
sint wlanhdr_to_ethhdr(union recv_frame *precvframe)
{
	sint	rmv_len;
	u16	eth_type, len;
	u8	bsnaphdr;
	u8	*psnap_type;
	struct ieee80211_snap_hdr	*psnap;

	sint ret = _SUCCESS;
	_adapter	*adapter = precvframe->u.hdr.adapter;
	struct mlme_priv	*pmlmepriv = &adapter->mlmepriv;

	u8	*ptr = get_recvframe_data(precvframe) ; /* point to frame_ctrl field */
	struct rx_pkt_attrib *pattrib = &precvframe->u.hdr.attrib;
#ifdef CONFIG_RTW_CORE_RXSC
	struct core_rxsc_entry *rxsc_entry = NULL;
#endif

	if (pattrib->encrypt)
		recvframe_pull_tail(precvframe, pattrib->icv_len);

	psnap = (struct ieee80211_snap_hdr *)(ptr + pattrib->hdrlen + pattrib->iv_len + RATTRIB_GET_MCTRL_LEN(pattrib));
	psnap_type = ptr + pattrib->hdrlen + pattrib->iv_len + RATTRIB_GET_MCTRL_LEN(pattrib) + SNAP_SIZE;
	/* convert hdr + possible LLC headers into Ethernet header */
	/* eth_type = (psnap_type[0] << 8) | psnap_type[1]; */
	if ((_rtw_memcmp(psnap, rtw_rfc1042_header, SNAP_SIZE) &&
	     (_rtw_memcmp(psnap_type, SNAP_ETH_TYPE_IPX, 2) == _FALSE) &&
	     (_rtw_memcmp(psnap_type, SNAP_ETH_TYPE_APPLETALK_AARP, 2) == _FALSE)) ||
	    /* eth_type != ETH_P_AARP && eth_type != ETH_P_IPX) || */
	    _rtw_memcmp(psnap, rtw_bridge_tunnel_header, SNAP_SIZE)) {
		/* remove RFC1042 or Bridge-Tunnel encapsulation and replace EtherType */
		bsnaphdr = _TRUE;
	} else {
		/* Leave Ethernet header part of hdr and full payload */
		bsnaphdr = _FALSE;
	}

	rmv_len = pattrib->hdrlen + pattrib->iv_len + RATTRIB_GET_MCTRL_LEN(pattrib) + (bsnaphdr ? SNAP_SIZE : 0);
	len = precvframe->u.hdr.len - rmv_len;


	_rtw_memcpy(&eth_type, ptr + rmv_len, 2);
	if(eth_type == ntohs(ETH_P_8021Q))
		_rtw_memcpy(&eth_type, ptr + rmv_len + VLAN_TAG_LEN, 2);

	eth_type = ntohs((unsigned short)eth_type); /* pattrib->ether_type */
	pattrib->eth_type = eth_type;

#ifdef CONFIG_RTW_CORE_RXSC
	pattrib->bsnaphdr = bsnaphdr;
	rxsc_entry = core_rxsc_alloc_entry(adapter, precvframe);

	if (rxsc_entry) {
		rxsc_entry->adapter = adapter;
		rxsc_entry->is_amsdu = 0;

		/* cache offset of payload */
		rxsc_entry->rxsc_payload_offset = (rmv_len - sizeof(struct ethhdr) + (bsnaphdr ? 2 : 0));
		if (rxsc_entry->is_htc)
			rxsc_entry->rxsc_payload_offset -= 4;

		/* cache padding size of tail */
		if (pattrib->encrypt)
			rxsc_entry->rxsc_trim_pad = pattrib->icv_len;
		else
			rxsc_entry->rxsc_trim_pad = 0;

		/* cache WLAN header */
		_rtw_memcpy((void *)&rxsc_entry->rxsc_wlanhdr, ptr, pattrib->hdrlen);
	}
#endif

	if ((check_fwstate(pmlmepriv, WIFI_MP_STATE) == _TRUE)) {
		ptr += rmv_len ;
		*ptr = 0x87;
		*(ptr + 1) = 0x12;

		eth_type = 0x8712;
		/* append rx status for mp test packets */
		ptr = recvframe_pull(precvframe, (rmv_len - sizeof(struct ethhdr) + 2) - 24);
		if (!ptr) {
			ret = _FAIL;
			goto exiting;
		}
		_rtw_memcpy(ptr, get_rxmem(precvframe), 24);
		ptr += 24;
	} else {
		ptr = recvframe_pull(precvframe, (rmv_len - sizeof(struct ethhdr) + (bsnaphdr ? 2 : 0)));
		if (!ptr) {
			ret = _FAIL;
			goto exiting;
		}
	}

	if (ptr) {
		_rtw_memcpy(ptr, pattrib->dst, ETH_ALEN);
		_rtw_memcpy(ptr + ETH_ALEN, pattrib->src, ETH_ALEN);

		if (!bsnaphdr) {
			len = htons(len);
			_rtw_memcpy(ptr + 12, &len, 2);
		}

		rtw_rframe_set_os_pkt(precvframe);
	}

#ifdef CONFIG_RTW_CORE_RXSC
	if (rxsc_entry) {
		/* Cache ETH header */
		_rtw_memcpy((void *)&rxsc_entry->rxsc_ethhdr, ptr, sizeof(rxsc_entry->rxsc_ethhdr));

		/* Cache Rx Attribute */
		_rtw_memcpy((void *)&rxsc_entry->rxsc_attrib, pattrib, sizeof(rxsc_entry->rxsc_attrib));
		if (rxsc_entry->is_htc)
			rxsc_entry->rxsc_attrib.hdrlen -= 4;

		rxsc_entry->status = RXSC_ENTRY_VALID;
	}
#ifdef RTW_PHL_DBG_CMD
	adapter->core_logs.rxCnt_data_orig++;
#endif
#endif

exiting:
	return ret;

}

#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
#ifndef CONFIG_SDIO_RX_COPY
#ifdef PLATFORM_LINUX
static void recvframe_expand_pkt(
	_adapter *padapter,
	union recv_frame *prframe)
{
	struct recv_frame_hdr *pfhdr;
	struct sk_buff *ppkt;
	u8 shift_sz;
	u32 alloc_sz;
	u8 *ptr;


	pfhdr = &prframe->u.hdr;

	/*	6 is for IP header 8 bytes alignment in QoS packet case. */
	if (pfhdr->attrib.qos)
		shift_sz = 6;
	else
		shift_sz = 0;

	/* for first fragment packet, need to allocate */
	/* (1536 + RXDESC_SIZE + drvinfo_sz) to reassemble packet */
	/*	8 is for skb->data 8 bytes alignment.
	*	alloc_sz = _RND(1536 + RXDESC_SIZE + pfhdr->attrib.drvinfosize + shift_sz + 8, 128); */
	alloc_sz = 1664; /* round (1536 + 24 + 32 + shift_sz + 8) to 128 bytes alignment */

	/* 3 1. alloc new skb */
	/* prepare extra space for 4 bytes alignment */
	ppkt = rtw_skb_alloc(alloc_sz);

	if (!ppkt)
		return; /* no way to expand */

	/* 3 2. Prepare new skb to replace & release old skb */
	/* force ppkt->data at 8-byte alignment address */
	skb_reserve(ppkt, 8 - ((SIZE_PTR)ppkt->data & 7));
	/* force ip_hdr at 8-byte alignment address according to shift_sz */
	skb_reserve(ppkt, shift_sz);

	/* copy data to new pkt */
	ptr = skb_put(ppkt, pfhdr->len);
	if (ptr)
		_rtw_memcpy(ptr, pfhdr->rx_data, pfhdr->len);

	rtw_skb_free(pfhdr->pkt);

	/* attach new pkt to recvframe */
	pfhdr->pkt = ppkt;
	pfhdr->rx_head = ppkt->head;
	pfhdr->rx_data = ppkt->data;
	pfhdr->rx_tail = skb_tail_pointer(ppkt);
	pfhdr->rx_end = skb_end_pointer(ppkt);
}
#else /*!= PLATFORM_LINUX*/
#warning "recvframe_expand_pkt not implement, defrag may crash system"
#endif
#endif /*#ifndef CONFIG_SDIO_RX_COPY*/
#endif

/* perform defrag */
union recv_frame *recvframe_defrag(_adapter *adapter, _queue *defrag_q)
{
	_list	*plist, *phead;
	u8	*data, wlanhdr_offset;
	u8	curfragnum;
	struct recv_frame_hdr *pfhdr, *pnfhdr;
	union recv_frame *prframe, *pnextrframe;
	_queue	*pfree_recv_queue;


	curfragnum = 0;
	pfree_recv_queue = &adapter->recvpriv.free_recv_queue;

	phead = get_list_head(defrag_q);
	plist = get_next(phead);
	prframe = LIST_CONTAINOR(plist, union recv_frame, u);
	pfhdr = &prframe->u.hdr;
	rtw_list_delete(&(prframe->u.list));

	if (curfragnum != pfhdr->attrib.frag_num) {
		/* the first fragment number must be 0 */
		/* free the whole queue */
		rtw_free_recvframe(prframe, pfree_recv_queue);
		rtw_free_recvframe_queue(defrag_q, pfree_recv_queue);

		return NULL;
	}

#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
#ifndef CONFIG_SDIO_RX_COPY
	recvframe_expand_pkt(adapter, prframe);
#endif
#endif

	curfragnum++;

	plist = get_list_head(defrag_q);

	plist = get_next(plist);

	data = get_recvframe_data(prframe);

	while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		pnextrframe = LIST_CONTAINOR(plist, union recv_frame , u);
		pnfhdr = &pnextrframe->u.hdr;

		/* CVE-2020-24587, The keytrack of the fragment is supposed to be the same with other's	*/
		if (pfhdr->keytrack != pnfhdr->keytrack) {
			RTW_INFO("Inconsistent key track, drop fragmented frame!\n");
			rtw_free_recvframe(prframe, pfree_recv_queue);
			rtw_free_recvframe_queue(defrag_q, pfree_recv_queue);
			return NULL;
		}

		/* check the fragment sequence  (2nd ~n fragment frame) */

		if (curfragnum != pnfhdr->attrib.frag_num) {
			/* the fragment number must be increasing  (after decache) */
			/* release the defrag_q & prframe */
			rtw_free_recvframe(prframe, pfree_recv_queue);
			rtw_free_recvframe_queue(defrag_q, pfree_recv_queue);
			return NULL;
		}

		curfragnum++;

		/* copy the 2nd~n fragment frame's payload to the first fragment */
		/* get the 2nd~last fragment frame's payload */

		wlanhdr_offset = pnfhdr->attrib.hdrlen + pnfhdr->attrib.iv_len;

		recvframe_pull(pnextrframe, wlanhdr_offset);

		/* append  to first fragment frame's tail (if privacy frame, pull the ICV) */
		recvframe_pull_tail(prframe, pfhdr->attrib.icv_len);

		/* _rtw_memcpy */
		_rtw_memcpy(pfhdr->rx_tail, pnfhdr->rx_data, pnfhdr->len);

		recvframe_put(prframe, pnfhdr->len);

		pfhdr->attrib.icv_len = pnfhdr->attrib.icv_len;
		plist = get_next(plist);

	};

	/* free the defrag_q queue and return the prframe */
	rtw_free_recvframe_queue(defrag_q, pfree_recv_queue);



	return prframe;
}

/* check if need to defrag, if needed queue the frame to defrag_q */
s32 recvframe_chk_defrag(_adapter *padapter, union recv_frame **pprecv_frame)
{
	struct sta_priv       *pstapriv = &(padapter->stapriv);
	union recv_frame 	  *precv_frame = *pprecv_frame;
	struct recv_frame_hdr *pfhdr = &(precv_frame->u.hdr);
	struct sta_info       *psta = pfhdr->psta;
	u8                     ismfrag = pfhdr->attrib.mfrag;
	u8                     fragnum = pfhdr->attrib.frag_num;
	//_adapter_link         *adapter_link = pfhdr->adapter_link;
	_list                 *phead;
	union recv_frame      *prtnframe = NULL;
	_queue                *pfree_recv_queue = &(padapter->recvpriv.free_recv_queue);
	_queue                *pdefrag_q = NULL;
	s32 				   ret = CORE_RX_CONTINUE;
	union pn48 			  *pdefrag_pn = NULL, next_pn;
	u8		      *psta_addr;

	/* need to define struct of wlan header frame ctrl */
	ismfrag = pfhdr->attrib.mfrag;
	fragnum = pfhdr->attrib.frag_num;

	psta_addr = pfhdr->attrib.ta;
	psta = rtw_get_stainfo(pstapriv, psta_addr);
	if (psta == NULL) {
		u8 type = GetFrameType(pfhdr->rx_data);
		if (type != WIFI_DATA_TYPE) {
			psta = rtw_get_bcmc_stainfo(padapter);
			if (psta)
				pdefrag_q = &psta->sta_recvpriv.defrag_q;
		} else
			pdefrag_q = NULL;
	} else
		pdefrag_q = &psta->sta_recvpriv.defrag_q;

	if (pdefrag_q)
		pdefrag_pn = &psta->sta_recvpriv.defrag_pn;

	if ((ismfrag == 0) && (fragnum == 0)) {
		ret = CORE_RX_CONTINUE;
	} else {
		/* CVE-2020-26145, group addressed frame cannot use fragmentation!! */
		if (IS_MCAST(pfhdr->attrib.ra)) {
			RTW_INFO("DROP group addressed fragment!\n");
			ret = CORE_RX_DROP;
		}
		/* CVE-2020-24587 */
		if ((psta) && (pdefrag_q))
			precv_frame->u.hdr.keytrack = ATOMIC_READ(&psta->keytrack);
	}

	if (ismfrag == 1) {
		/* 0~(n-1) fragment frame */
		/* enqueue to defraf_g */
		if (pdefrag_q != NULL) {
			if (fragnum == 0) {
				/* the first fragment */
				if (_rtw_queue_empty(pdefrag_q) == _FALSE) {
					/* free current defrag_q */
					rtw_free_recvframe_queue(pdefrag_q, pfree_recv_queue);
				}
				if (pfhdr->attrib.pn.val > 0)
					*pdefrag_pn = pfhdr->attrib.pn;
			}
			else {
				/* check sequencial pn */
				if (pfhdr->attrib.pn.val > 0) {
					next_pn.val = (pdefrag_pn->val == 0xffffffffffffULL ? 0 : pdefrag_pn->val + 1);
					if (pfhdr->attrib.pn.val == next_pn.val)
						*pdefrag_pn = pfhdr->attrib.pn;
					else
						ret = CORE_RX_DROP;
				}
			}

			if (ret != CORE_RX_DROP) {
				/* Then enqueue the 0~(n-1) fragment into the defrag_q */

				/* _rtw_spinlock(&pdefrag_q->lock); */
				phead = get_list_head(pdefrag_q);
				rtw_list_insert_tail(&pfhdr->list, phead);
				/* _rtw_spinunlock(&pdefrag_q->lock); */
				ret = CORE_RX_DEFRAG;
			}
		} else {
			/* can't find this ta's defrag_queue, so free this recv_frame */
			ret = CORE_RX_DROP;
		}

	}

	if ((ismfrag == 0) && (fragnum != 0)) {
		/* the last fragment frame */
		/* enqueue the last fragment */
		if (pdefrag_q != NULL) {
			if (pfhdr->attrib.pn.val > 0) {
				next_pn.val = (pdefrag_pn->val == 0xffffffffffffULL ? 0 : pdefrag_pn->val + 1);
				if (pfhdr->attrib.pn.val != next_pn.val)
					ret = CORE_RX_DROP;
			}

			if (ret != CORE_RX_DROP) {
				/* _rtw_spinlock(&pdefrag_q->lock); */
				phead = get_list_head(pdefrag_q);
				rtw_list_insert_tail(&pfhdr->list, phead);
				/* _rtw_spinunlock(&pdefrag_q->lock); */

				/* call recvframe_defrag to defrag */
				precv_frame = recvframe_defrag(padapter, pdefrag_q);
				if (precv_frame == NULL)
					ret = CORE_RX_DROP;
				else {
					*pprecv_frame = precv_frame;
					ret = CORE_RX_CONTINUE;
				}
			}
			else {
				if (_rtw_queue_empty(pdefrag_q) == _FALSE) {
					/* free current defrag_q */
					rtw_free_recvframe_queue(pdefrag_q, pfree_recv_queue);
				}
			}
		} else {
			/* can't find this ta's defrag_queue, so free this recv_frame */
			ret = CORE_RX_DROP;
		}

	}

	return ret;

}

static int rtw_recv_indicatepkt_check(union recv_frame *rframe, u8 *ehdr_pos, u32 pkt_len)
{
	_adapter *adapter = rframe->u.hdr.adapter;
	struct recv_priv *recvpriv = &adapter->recvpriv;
	struct ethhdr *ehdr = (struct ethhdr *)ehdr_pos;
	struct rx_pkt_attrib *pattrib = &rframe->u.hdr.attrib;
#ifdef DBG_IP_R_MONITOR
	int i;
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;
	struct mlme_priv	*pmlmepriv = &adapter->mlmepriv;
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
#endif/*DBG_IP_R_MONITOR*/
	enum eap_type eapol_type;
	int ret = _FAIL;
	u8 vlan_offset = 0;
	u16 org_h_proto = 0;

	if (ntohs(ehdr->h_proto) == ETH_P_8021Q) {
		org_h_proto = ehdr->h_proto;
		ehdr->h_proto = *(u16*)(ehdr_pos + 2*ETH_ALEN + VLAN_TAG_LEN);
		vlan_offset = VLAN_TAG_LEN;
	}

#ifdef CONFIG_WAPI_SUPPORT
	if (rtw_wapi_check_for_drop(adapter, rframe, ehdr_pos + vlan_offset)) {
		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" rtw_wapi_check_for_drop\n"
			, FUNC_ADPT_ARG(adapter));
		#endif
		goto exit;
	}
#endif

	if (rframe->u.hdr.psta)
		rtw_st_ctl_rx(rframe->u.hdr.psta, ehdr_pos + vlan_offset);

	if (ntohs(ehdr->h_proto) == 0x888e) {
		eapol_type = parsing_eapol_packet(adapter, ehdr_pos + ETH_HLEN, rframe->u.hdr.psta, 0);
		if ((eapol_type == EAPOL_1_4 || eapol_type == EAPOL_3_4) && pattrib->encrypt == 0) {
			rframe->u.hdr.psta->resp_nonenc_eapol_key_starttime = rtw_get_current_time();
			RTW_INFO(ADPT_FMT" receive unencrypted eapol key\n",
				 ADPT_ARG(adapter));
		}
	}
#ifdef DBG_ARP_DUMP
	else if (ntohs(ehdr->h_proto) == ETH_P_ARP)
		dump_arp_pkt(RTW_DBGDUMP, ehdr->h_dest, ehdr->h_source, ehdr_pos + ETH_HLEN + vlan_offset, 0);
#endif

	if (recvpriv->sink_udpport > 0)
		rtw_sink_rtp_seq_dbg(adapter, ehdr_pos + vlan_offset);

#ifdef DBG_UDP_PKT_LOSE_11AC
	#define PAYLOAD_LEN_LOC_OF_IP_HDR 0x10 /*ethernet payload length location of ip header (DA + SA+eth_type+(version&hdr_len)) */

	if (ntohs(ehdr->h_proto) == ETH_P_ARP) {
		/* ARP Payload length will be 42bytes or 42+18(tailer)=60bytes*/
		if (pkt_len != (42 + vlan_offset) && pkt_len != 60)
			RTW_INFO("Error !!%s,ARP Payload length %u not correct\n" , __func__ , pkt_len);
	} else if (ntohs(ehdr->h_proto) == ETH_P_IP) {
		if (be16_to_cpu(*((u16 *)(ehdr_pos + PAYLOAD_LEN_LOC_OF_IP_HDR + vlan_offset))) != (pkt_len - ETH_HLEN - vlan_offset)) {
			RTW_INFO("Error !!%s,Payload length not correct\n" , __func__);
			RTW_INFO("%s, IP header describe Total length=%u\n" , __func__ , be16_to_cpu(*((u16 *)(ehdr_pos + PAYLOAD_LEN_LOC_OF_IP_HDR + vlan_offset))));
			RTW_INFO("%s, Pkt real length=%u\n" , __func__ , (pkt_len) - ETH_HLEN - vlan_offset);
		}
	}
#endif

#ifdef DBG_IP_R_MONITOR
	#define LEN_ARP_OP_HDR 7 /*ARP OERATION */
	if (ntohs(ehdr->h_proto) == ETH_P_ARP) {

		if(check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE){
			if(ehdr_pos[ETHERNET_HEADER_SIZE+LEN_ARP_OP_HDR+vlan_offset] == 2) {

				RTW_INFO("%s,[DBG_ARP] Rx ARP RSP Packet;SeqNum = %d !\n",
					__FUNCTION__, pattrib->seq_num);

				dump_arp_pkt(RTW_DBGDUMP, ehdr->h_dest, ehdr->h_source, ehdr_pos + ETH_HLEN + vlan_offset, 0);

			}
		}
	}
#endif/*DBG_IP_R_MONITOR*/

#ifdef CONFIG_AUTO_AP_MODE
	if (ntohs(ehdr->h_proto) == 0x8899)
		rtw_auto_ap_rx_msg_dump(adapter, rframe, ehdr_pos + vlan_offset);
#endif

	ret = _SUCCESS;

#ifdef CONFIG_WAPI_SUPPORT
exit:
#endif

	if(org_h_proto)
		ehdr->h_proto = org_h_proto;

	return ret;
}

#ifdef CONFIG_RTW_MESH
static void recv_free_fwd_resource(_adapter *adapter, struct xmit_frame *fwd_frame, _list *b2u_list)
{
	struct xmit_priv *xmitpriv = &adapter->xmitpriv;

	if (fwd_frame)
		rtw_free_xmitframe(xmitpriv, fwd_frame);

#if CONFIG_RTW_MESH_DATA_BMC_TO_UC
	if (!rtw_is_list_empty(b2u_list)) {
		struct xmit_frame *b2uframe;
		_list *list;

		list = get_next(b2u_list);
		while (rtw_end_of_queue_search(b2u_list, list) == _FALSE) {
			b2uframe = LIST_CONTAINOR(list, struct xmit_frame, list);
			list = get_next(list);
			rtw_list_delete(&b2uframe->list);
			rtw_free_xmitframe(xmitpriv, b2uframe);
		}
	}
#endif
}

static void recv_fwd_pkt_hdl(_adapter *adapter, struct sk_buff *pkt
	, u8 act, struct xmit_frame *fwd_frame, _list *b2u_list)
{
	struct xmit_priv *xmitpriv = &adapter->xmitpriv;
	struct sk_buff *fwd_pkt = pkt;

	if (act & RTW_RX_MSDU_ACT_INDICATE) {
		fwd_pkt = rtw_skb_copy(pkt);
		if (!fwd_pkt) {
			#ifdef DBG_TX_DROP_FRAME
			RTW_INFO("DBG_TX_DROP_FRAME %s rtw_skb_copy fail\n", __func__);
			#endif
			recv_free_fwd_resource(adapter, fwd_frame, b2u_list);
			goto exit;
		}
	}

#if CONFIG_RTW_MESH_DATA_BMC_TO_UC
	if (!rtw_is_list_empty(b2u_list)) {
		_list *list = get_next(b2u_list);
		struct xmit_frame *b2uframe;

		while (rtw_end_of_queue_search(b2u_list, list) == _FALSE) {
			b2uframe = LIST_CONTAINOR(list, struct xmit_frame, list);
			list = get_next(list);
			rtw_list_delete(&b2uframe->list);

			if (!fwd_frame && rtw_is_list_empty(b2u_list)) /* the last fwd_pkt */
				b2uframe->pkt = fwd_pkt;
			else
				b2uframe->pkt = rtw_skb_copy(fwd_pkt);
			if (!b2uframe->pkt) {
				rtw_free_xmitframe(xmitpriv, b2uframe);
				continue;
			}

			rtw_xmit_posthandle(adapter, b2uframe, b2uframe->pkt);
		}
	}
#endif

	if (fwd_frame) {
		fwd_frame->pkt = fwd_pkt;
		if (rtw_xmit_posthandle(adapter, fwd_frame, fwd_pkt) < 0) {
			#ifdef DBG_TX_DROP_FRAME
			RTW_INFO("DBG_TX_DROP_FRAME %s rtw_xmit_posthandle fail\n", __func__);
			#endif
			xmitpriv->tx_drop++;
		}
	}

exit:
	return;
}
#endif /* CONFIG_RTW_MESH */

#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
u8 DBG_PRINT_RCV_ONCE;

u8 amsdu_check_subpkt_a1_mixed(_adapter *padapter,
												union recv_frame *prframe)
{
	struct sk_buff *pkt;
	int a_len = 0, padding_len = 0;
	u16 nSubframe_Length = 0;
	u8 is_mixed = 0, subpkt_cnt = 0;
	u8 *ptr = NULL;
	unsigned char a1_addr[ETH_ALEN] = {00 ,00 ,00 , 00, 00 ,00};

	pkt = prframe->u.hdr.pkt;
	pkt->data = ptr = prframe->u.hdr.rx_data;
	a_len = prframe->u.hdr.len;

	while (a_len > ETH_HLEN) {

		if (DBG_PRINT_RCV_ONCE)
			txsc_dump_data(ptr, ETH_HLEN, "subframe");

		if (subpkt_cnt == 0)
			CP_MAC_ADDR(a1_addr, ptr);
		else {
			if (!EQ_MAC_ADDR(ptr, a1_addr)) {
				is_mixed = 1;
				if (DBG_PRINT_RCV_ONCE)
					RTW_PRINT("subframe[%d] a1:%pM mismatch to [1]a1_addr:%pM \n", subpkt_cnt, ptr, a1_addr);
				break;
			}
		}

		#if 0
		if(!EQ_MAC_ADDR(ptr, rxsc_entry->rxsc_ethhdr.h_dest) ||
			!EQ_MAC_ADDR(ptr + 6, rxsc_entry->rxsc_ethhdr.h_source)) {
			is_mixed = 1;
			if (DBG_PRINT_RCV_ONCE)
				RTW_PRINT("subframe mismatch da:%pM sa:%pM rxsc_dest:%pM rxsc_src:%pM\n",
					ptr, ptr+6, rxsc_entry->rxsc_ethhdr.h_dest, rxsc_entry->rxsc_ethhdr.h_source);
			break;
		}
		#endif

		nSubframe_Length = RTW_GET_BE16(ptr + 12);

		ptr += (ETH_HLEN + nSubframe_Length);
		a_len -= (ETH_HLEN + nSubframe_Length);
		if (a_len != 0) {
			padding_len = 4 - ((nSubframe_Length + ETH_HLEN) & (4 - 1));
			if (padding_len == 4)
				padding_len = 0;

			if (a_len < padding_len) {
				RTW_INFO("ParseSubframe(): a_len < padding_len !\n");
				break;
			}
			ptr += padding_len;
			a_len -= padding_len;
		}

		subpkt_cnt++;
	}

	return is_mixed;
}

u8 amsdu_check_subpkt_a1_valid(u8 *a1_addr, struct sta_info *psta)
{
	u8 i = 0, hit = 0;

	for (i = 0 ; i < psta->subpkt_cnt; i++) {
		if (EQ_MAC_ADDR(a1_addr, (u8 *)&psta->subpkt_addr1[i])) {
			hit = 1;
			break;
		}
	}

	return hit;
}
#endif

int amsdu_to_msdu(_adapter *padapter, union recv_frame *prframe)
{
	struct rx_pkt_attrib *rattrib = &prframe->u.hdr.attrib;
	int	a_len, padding_len;
	u16	nSubframe_Length;
	u8	nr_subframes, i;
	u8	*pdata;
	struct sk_buff *sub_pkt, *subframes[MAX_SUBFRAME_COUNT];
	struct recv_priv *precvpriv = &padapter->recvpriv;
	_queue *pfree_recv_queue = &(precvpriv->free_recv_queue);
	const u8 *da, *sa;
	int act;
#ifdef CONFIG_RTW_MESH /* TODO: move AP mode forward & b2u logic here */
	struct xmit_frame *fwd_frame;
	_list b2u_list;
#endif
	u8 mctrl_len = 0;
	u32 pkt_len = 0;
	int	ret = _SUCCESS;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	struct rtw_r_meta_data *mdata = (struct rtw_r_meta_data *)prframe->u.hdr.mdata;
	#endif
	#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
	struct sk_buff *pkt = NULL;
	struct sta_info *psta = prframe->u.hdr.psta;
	u8 is_subpkt_a1_match = 0, is_a1_mixed = 0;
	#endif


/* CONFIG_RTW_A4_STA, todo: amsdu */

#ifdef CORE_RXSC_RFRAME
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
#endif

#ifdef CORE_RXSC_AMSDU
	struct core_rxsc_entry *rxsc_entry = NULL;
#endif

	padapter->rx_logs.core_rx_amsdu_cut++;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	if (mdata->hdr_conv)
		padapter->rx_logs.core_rx_amsdu_cut_hdr_conv++;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

#ifdef CORE_RXSC_AMSDU
	if (!prframe->u.hdr.rxsc_entry)
		rxsc_entry = core_rxsc_alloc_entry(padapter, prframe);
	else
		rxsc_entry = prframe->u.hdr.rxsc_entry;

	if (rxsc_entry &&
		RXSC_ENTRY_CREATING == rxsc_entry->status) {

		//DBGP("add rxsc_entry \n");
		rxsc_entry->adapter = padapter;
		rxsc_entry->is_amsdu = 1;

		/* cache WLAN header */
		_rtw_memcpy((void *)&rxsc_entry->rxsc_wlanhdr,
			get_recvframe_data(prframe), rattrib->hdrlen);

		/* Cache Rx Attribute */
		_rtw_memcpy((void *)&rxsc_entry->rxsc_attrib,
			rattrib, sizeof(rxsc_entry->rxsc_attrib));

		if (rxsc_entry->is_htc)
			rxsc_entry->rxsc_attrib.hdrlen -= 4;
	}

#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
	if(rxsc_entry && DBG_PRINT_RCV_ONCE) {
		RTW_PRINT("[%s:%d]\n[%d] forward_to:%d, status:%d, is_amsdu:%d, bypass_deamsdu:%d\n",
			__func__, __LINE__, DBG_PRINT_RCV_ONCE,
			rxsc_entry->forward_to, rxsc_entry->status, rxsc_entry->is_amsdu,
			padapter->registrypriv.wifi_mib.bypass_deamsdu);
		DBG_PRINT_RCV_ONCE--;
	}
#endif
#endif /* CORE_RXSC_AMSDU */

	nr_subframes = 0;

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	if (!mdata->hdr_conv)
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
		recvframe_pull(prframe, rattrib->hdrlen + rattrib->iv_len);

#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
	if(rxsc_entry && DBG_PRINT_RCV_ONCE) {
		pdata = prframe->u.hdr.rx_data;
		RTW_PRINT("[%d] hdr_len=%d\n", DBG_PRINT_RCV_ONCE, prframe->u.hdr.len);
		txsc_dump_data((u8 *)&rxsc_entry->rxsc_ethhdr, 14, "rxsc_ethhdr");
		txsc_dump_data(pdata, 14, "RX pkt->data");
		DBG_PRINT_RCV_ONCE--;
	}

	if (!padapter->registrypriv.wifi_mib.bypass_deamsdu || rattrib->to_fr_ds == 3)
		goto skip_bypass_deamsdu;

	is_subpkt_a1_match = amsdu_check_subpkt_a1_valid(prframe->u.hdr.rx_data, psta);
#ifdef CORE_RXSC_AMSDU
	if (rxsc_entry && rxsc_entry->forward_to == RXSC_FWD_KERNEL &&
		/*EQ_MAC_ADDR(prframe->u.hdr.rx_data, rxsc_entry->rxsc_ethhdr.h_dest)*/
		is_subpkt_a1_match)
	{
#ifdef CONFIG_VW_REFINE
		if (padapter->tc_enable == 0)
#endif
		{
			is_a1_mixed = amsdu_check_subpkt_a1_mixed(padapter, prframe);
			if (is_a1_mixed) {
				padapter->core_logs.rxCnt_deamsdu_bypass[3]++;
				goto skip_bypass_deamsdu;
			}
		}

		sub_pkt = prframe->u.hdr.pkt;
		sub_pkt->data = prframe->u.hdr.rx_data;
		if (padapter->securitypriv.dot11PrivacyAlgrthm != _NO_PRIVACY_)
			sub_pkt->len = prframe->u.hdr.len - RTW_TKIP_MIC_LEN;
		else
			sub_pkt->len = prframe->u.hdr.len;
		pkt_len = sub_pkt->len;
		skb_set_tail_pointer(sub_pkt, prframe->u.hdr.len);

		pkt = sub_pkt;

		//pkt->protocol = eth_type_trans(pkt, padapter->pnetdev);
		pkt->dev = padapter->pnetdev;
		pkt->ip_summed = CHECKSUM_NONE; /* CONFIG_TCP_CSUM_OFFLOAD_RX */
	#ifdef RTW_CORE_PKT_TRACE
		if(padapter->pkt_trace_enable)
			RTW_RX_TRACE(padapter,&rattrib->pktinfo);
	#endif
		skb_reset_mac_header(sub_pkt);
		skb_pull(sub_pkt, 14);
	#ifdef CONFIG_SMP_NETIF_RX
		rtw_netif_rx_enq(padapter, pkt);
	#else
		rtw_netif_rx(padapter->pnetdev, pkt);
	#endif
		prframe->u.hdr.pkt = NULL;
		padapter->core_logs.rxCnt_deamsdu_bypass[0]++;

		if(pkt_len <= AMSDU_0_SIZE)
			padapter->core_logs.rxCnt_deamsdu_bypass_sz[0]++;
		else if(pkt_len <= AMSDU_1_SIZE)
			padapter->core_logs.rxCnt_deamsdu_bypass_sz[1]++;
		else if(pkt_len <= AMSDU_2_SIZE)
			padapter->core_logs.rxCnt_deamsdu_bypass_sz[2]++;
		else
			padapter->core_logs.rxCnt_deamsdu_bypass_sz[3]++;

		if(DBG_PRINT_RCV_ONCE)
			RTW_PRINT("skip deamsdu\n\n");
		goto skip_deamsdu;
	}
#endif
skip_bypass_deamsdu:
#endif /* defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX) */

	a_len = prframe->u.hdr.len;
	pdata = prframe->u.hdr.rx_data;

	/* mitigate A-MSDU aggregation injection attacks */
	if (a_len > ETH_HLEN && _rtw_memcmp(pdata, rtw_rfc1042_header, SNAP_SIZE)) {
		RTW_PRINT("RX drop: The DA of 1st AMSDU subframe contains the start of an RFC1042 header\n");
		return _FAIL;
	}

	while (a_len > ETH_HLEN) {
		/* Offset 12 denote 2 mac address */
		nSubframe_Length = RTW_GET_BE16(pdata + 12);
#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
		if(DBG_PRINT_RCV_ONCE) {
			RTW_PRINT("[%d] a_len=%d, nSubframe_Length=%d\n", DBG_PRINT_RCV_ONCE, a_len, nSubframe_Length);
		}
#endif

		if (a_len < (ETHERNET_HEADER_SIZE + nSubframe_Length)) {
			RTW_INFO("nRemain_Length is %d and nSubframe_Length is : %d\n", a_len, nSubframe_Length);
			break;
		}
		/* Not process this A-MSDU if subframe has abnormal length */
		if (IEEE80211_DATA_LEN < (nSubframe_Length)) {
			RTW_WARN("nSubframe_Length %u > MSDU length %u\n",
			         nSubframe_Length, IEEE80211_DATA_LEN);
			break;
		}

		act = RTW_RX_MSDU_ACT_INDICATE;

		#ifdef CONFIG_RTW_MESH
		fwd_frame = NULL;

		if (MLME_IS_MESH(padapter)) {
			u8 *mda = pdata, *msa = pdata + ETH_ALEN;
			struct rtw_ieee80211s_hdr *mctrl = (struct rtw_ieee80211s_hdr *)(pdata + ETH_HLEN);
			int v_ret;

			v_ret = rtw_mesh_rx_data_validate_mctrl(padapter, prframe
				, mctrl, mda, msa, &mctrl_len, &da, &sa);
			if (v_ret != _SUCCESS)
				goto move_to_next;

			act = rtw_mesh_rx_msdu_act_check(prframe
				, mda, msa, da, sa, mctrl, &fwd_frame, &b2u_list);
		} else
		#endif
		{
			da = pdata;
			sa = pdata + ETH_ALEN;
		#ifdef CONFIG_RTW_A4_STA
			if ((3 == rattrib->to_fr_ds) && MLME_IS_STA(padapter))
				act = core_a4_rx_amsdu_act_check(padapter, da, sa);
		#endif
		}

		#ifdef CONFIG_RTW_MESH
		if (!act)
			goto move_to_next;
		#endif

		#ifdef CONFIG_RTW_SW_LED
		rtw_led_rx_control(padapter, da);
		#endif

		/* A4_AMSDU */ //?? will da/sa diff to mac header ?? //todo: short amsdu
		#ifdef CONFIG_RTW_A4_STA
		if (3 == rattrib->to_fr_ds) {
			memcpy(rattrib->src, sa, ETH_ALEN);
			memcpy(rattrib->dst, da, ETH_ALEN);
			if (EQ_MAC_ADDR(padapter->br_mac, sa)){
				RTW_PRINT("RX: SA is br0 mac in a4 amsdu subframe\n");
			}else
				act = core_a4_rx_msdu_act_check(prframe);
		}else
		#endif
		{
			if (EQ_MAC_ADDR(padapter->br_mac, sa)){
				RTW_PRINT("RX: SA is br0 mac in a3 amsdu subframe\n");
			}
		}

		sub_pkt = rtw_os_alloc_msdu_pkt(prframe, da, sa
			, pdata + ETH_HLEN + mctrl_len, nSubframe_Length - mctrl_len);
		if (sub_pkt == NULL) {
			if (act & RTW_RX_MSDU_ACT_INDICATE) {
				#ifdef DBG_RX_DROP_FRAME
				RTW_INFO("DBG_RX_DROP_FRAME %s rtw_os_alloc_msdu_pkt fail\n", __func__);
				#endif
			}
			#ifdef CONFIG_RTW_MESH
			if (act & RTW_RX_MSDU_ACT_FORWARD) {
				#ifdef DBG_TX_DROP_FRAME
				RTW_INFO("DBG_TX_DROP_FRAME %s rtw_os_alloc_msdu_pkt fail\n", __func__);
				#endif
				recv_free_fwd_resource(padapter, fwd_frame, &b2u_list);
			}
			#endif
			break;
		}

		#ifdef CONFIG_RTW_MESH
		if (act & RTW_RX_MSDU_ACT_FORWARD) {
			recv_fwd_pkt_hdl(padapter, sub_pkt, act, fwd_frame, &b2u_list);
			if (!(act & RTW_RX_MSDU_ACT_INDICATE))
				goto move_to_next;
		}
		#endif

		if (rtw_recv_indicatepkt_check(prframe, rtw_skb_data(sub_pkt), rtw_skb_len(sub_pkt)) == _SUCCESS)
			subframes[nr_subframes++] = sub_pkt;
		else
			rtw_skb_free(sub_pkt);

#ifdef CONFIG_RTW_MESH
move_to_next:
#endif
		/* move the data point to data content */
		pdata += ETH_HLEN;
		a_len -= ETH_HLEN;

		if (nr_subframes >= MAX_SUBFRAME_COUNT) {
			RTW_WARN("ParseSubframe(): Too many Subframes! Packets dropped!\n");
			break;
		}

		pdata += nSubframe_Length;
		a_len -= nSubframe_Length;
		if (a_len != 0) {
			padding_len = 4 - ((nSubframe_Length + ETH_HLEN) & (4 - 1));
			if (padding_len == 4)
				padding_len = 0;

			if (a_len < padding_len) {
				RTW_INFO("ParseSubframe(): a_len < padding_len !\n");
				break;
			}
			pdata += padding_len;
			a_len -= padding_len;
		}

#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
		if(DBG_PRINT_RCV_ONCE) {
			txsc_dump_data(sub_pkt->data, 16, "subpkt->data:");
			DBG_PRINT_RCV_ONCE--;
		}
#endif
	}

#ifdef RTW_PHL_DBG_CMD
	if(nr_subframes >= LOG_DEAMSDU_PKTNUM)
		padapter->core_logs.rxCnt_deamsdu_pktnum[(LOG_DEAMSDU_PKTNUM-1)]++;
	else if (nr_subframes > 0)
		padapter->core_logs.rxCnt_deamsdu_pktnum[(nr_subframes-1)]++;
	else
		RTW_DBG("nr_subframes is 0");
#endif

	for (i = 0; i < nr_subframes; i++) {
		sub_pkt = subframes[i];

		/* Indicat the packets to upper layer */
		if (sub_pkt) {
#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
		if (padapter->registrypriv.wifi_mib.bypass_deamsdu && rattrib->to_fr_ds != 3) {
#ifdef CORE_RXSC_AMSDU
			if (rxsc_entry && rxsc_entry->forward_to == RXSC_FWD_KERNEL &&
				/*EQ_MAC_ADDR(sub_pkt->data, rxsc_entry->rxsc_ethhdr.h_dest)*/
				is_subpkt_a1_match) {
				pkt = sub_pkt;
				pkt->protocol = eth_type_trans(pkt, padapter->pnetdev);
				pkt->dev = padapter->pnetdev;
				pkt->ip_summed = CHECKSUM_NONE; /* CONFIG_TCP_CSUM_OFFLOAD_RX */
				#ifdef RTW_CORE_PKT_TRACE
				if(padapter->pkt_trace_enable)
					RTW_RX_TRACE(padapter,&rattrib->pktinfo);
				#endif
				#ifdef CONFIG_SMP_NETIF_RX
				rtw_netif_rx_enq(padapter, pkt);
				#else
				rtw_netif_rx(padapter->pnetdev, pkt);
				#endif
				padapter->core_logs.rxCnt_deamsdu_bypass[1]++;
				#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
					if(DBG_PRINT_RCV_ONCE) {
						RTW_PRINT("[%d] is_subpkt_a1_match:%d is_a1_mixed:%d\n",
							i, is_subpkt_a1_match, is_a1_mixed);
					}
				#endif
			} else {
				if(rxsc_entry && rxsc_entry->forward_to == RXSC_FWD_KERNEL) {
					//RTW_PRINT("CP %pM to h_dest:%pM\n", sub_pkt->data, rxsc_entry->rxsc_ethhdr.h_dest);
					//CP_MAC_ADDR(rxsc_entry->rxsc_ethhdr.h_dest, sub_pkt->data);
					CP_MAC_ADDR(psta->subpkt_addr1[psta->subpkt_idx], sub_pkt->data);
					psta->subpkt_idx = (psta->subpkt_idx + 1) % NUM_RXSC_ENTRY;/* Next */
					if (psta->subpkt_cnt >= NUM_RXSC_ENTRY)
						psta->subpkt_cnt = NUM_RXSC_ENTRY;
					else
						psta->subpkt_cnt++;
				}
				rtw_os_recv_indicate_pkt(padapter, sub_pkt, prframe);
				padapter->core_logs.rxCnt_deamsdu_bypass[2]++;
			}
#endif
		} else
			rtw_os_recv_indicate_pkt(padapter, sub_pkt, prframe);
#else
			rtw_os_recv_indicate_pkt(padapter, sub_pkt, prframe);
#endif /* CONFIG_RTW_BYPASS_DEAMSDU && PLATFORM_LINUX */
		}
	}

#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
skip_deamsdu:
#endif
	prframe->u.hdr.len = 0;
#ifdef CORE_RXSC_RFRAME
	if (rxsc_entry &&
		RXSC_ENTRY_VALID == rxsc_entry->status &&
		prframe->u.hdr.rx_req) {
		rtw_return_phl_rx_pkt(dvobj, (u8*)prframe->u.hdr.rx_req);
		rtw_os_free_recvframe(prframe);
	} else
#endif
	{
		rtw_free_recvframe(prframe, pfree_recv_queue);/* free this recv_frame */
	}

#ifdef CORE_RXSC_AMSDU
	if (rxsc_entry) {

		if (RXSC_ENTRY_VALID == rxsc_entry->status)
			padapter->core_logs.rxCnt_amsdu_shortcut++;
		else
			padapter->core_logs.rxCnt_amsdu_orig++;

		if (_SUCCESS == ret)
			rxsc_entry->status = RXSC_ENTRY_VALID;
		else
			rxsc_entry->status = RXSC_ENTRY_INVALID;
	} else {
		padapter->core_logs.rxCnt_amsdu_orig++;
	}
#endif

	return ret;
}

#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
static struct sk_buff *_proc_subframe(_adapter *padapter,
                                      union recv_frame *prframe,
                                      struct rx_pkt_attrib *rattrib,
                                      struct rtw_rx_buf_base *rx_buf)
{
	struct sk_buff *sub_pkt;
	u8 *frame = rx_buf->frame;
	const u8 *da, *sa;
	int act;

	#ifdef CONFIG_RTW_MESH /* TODO: move AP mode forward & b2u logic here */
	struct xmit_frame *fwd_frame;
	_list b2u_list;
	#endif

	#ifdef CONFIG_RTW_MESH
	fwd_frame = NULL;

	if (MLME_IS_MESH(padapter)) {
		u8 *mda = pdata, *msa = pdata + ETH_ALEN;
		struct rtw_ieee80211s_hdr *mctrl = (struct rtw_ieee80211s_hdr *)(pdata + ETH_HLEN);
		int v_ret;

		v_ret = rtw_mesh_rx_data_validate_mctrl(padapter, prframe,
		                                        mctrl, mda, msa,
		                                        &mctrl_len, &da, &sa);
		return NULL;

		act = rtw_mesh_rx_msdu_act_check(prframe, mda, msa, da, sa,
		                                 mctrl, &fwd_frame, &b2u_list);
	} else
	#endif /* CONFIG_RTW_MESH */
	{
		da = frame;
		sa = frame + ETH_ALEN;
		#ifdef CONFIG_RTW_A4_STA
		if (MLME_IS_STA(padapter))
			act = core_a4_rx_amsdu_act_check(padapter, da, sa);
		#endif /* CONFIG_RTW_A4_STA */
	}

	#ifdef CONFIG_RTW_MESH
	if (!act)
		return NULL;
	#endif /* CONFIG_RTW_MESH */

	#ifdef CONFIG_RTW_SW_LED
	rtw_led_rx_control(padapter, da);
	#endif

	/* A4_AMSDU */ //?? will da/sa diff to mac header ?? //todo: short amsdu
	#ifdef CONFIG_RTW_A4_STA
	if (3 == rattrib->to_fr_ds) {
		memcpy(rattrib->src, sa, ETH_ALEN);
		memcpy(rattrib->dst, da, ETH_ALEN);
		act = core_a4_rx_msdu_act_check(prframe);
	}
	#endif /* CONFIG_RTW_A4_STA */

	sub_pkt = (struct sk_buff *)rx_buf->os_priv;
	sub_pkt->data = rx_buf->frame;
	sub_pkt->len = rx_buf->frame_len;
	skb_reset_tail_pointer(sub_pkt);

	#ifdef CONFIG_RTW_MESH
	if (act & RTW_RX_MSDU_ACT_FORWARD) {
		recv_fwd_pkt_hdl(padapter, sub_pkt, act, fwd_frame, &b2u_list);
		if (!(act & RTW_RX_MSDU_ACT_INDICATE))
			return NULL;
	}
	#endif /* CONFIG_RTW_MESH */

	if (rtw_recv_indicatepkt_check(prframe, rtw_skb_data(sub_pkt),
	                               rtw_skb_len(sub_pkt))
	    != _SUCCESS)
		return NULL;

	/* Set os_priv to NULL as this skb would be RXed */
	rx_buf->os_priv = NULL;

	return sub_pkt;
}

static int _amsdu_lst_to_msdu(_adapter *padapter, union recv_frame *prframe)
{
	struct rtw_recv_pkt *rx_req = prframe->u.hdr.rx_req;
	struct list_head *rx_buf_lst = &rx_req->rx_buf_lst_head;
	struct rtw_rx_buf_base *rx_buf_p, *rx_buf_q;
	struct rx_pkt_attrib *rattrib = &prframe->u.hdr.attrib;
	u8	nr_subframes, i;
	u8	*pdata;
	struct sk_buff *sub_pkt, *subframes[MAX_SUBFRAME_COUNT];
	struct recv_priv *precvpriv = &padapter->recvpriv;
	_queue *pfree_recv_queue = &(precvpriv->free_recv_queue);
	const u8 *da, *sa;
	int act;
	struct rtw_r_meta_data *mdata = (struct rtw_r_meta_data *)prframe->u.hdr.mdata;

#ifdef CONFIG_RTW_MESH /* TODO: move AP mode forward & b2u logic here */
	struct xmit_frame *fwd_frame;
	_list b2u_list;
#endif
	u8 mctrl_len = 0;
	int	ret = _SUCCESS;

#if 0 /* def CORE_RXSC_RFRAME */
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
#endif

#if 0 /* def CORE_RXSC_AMSDU */
	struct core_rxsc_entry *rxsc_entry = NULL;
#endif

#if 0 /* def CORE_RXSC_AMSDU */
	if (!prframe->u.hdr.rxsc_entry) {
		rxsc_entry = core_rxsc_alloc_entry(padapter, prframe);
		if (rxsc_entry) {
			//DBGP("add rxsc_entry \n");
			rxsc_entry->adapter = padapter;
			rxsc_entry->is_amsdu = 1;

			/* cache WLAN header */
			_rtw_memcpy((void *)&rxsc_entry->rxsc_wlanhdr,
			            prframe->u.hdr.wlan_hdr,
			            rattrib->hdrlen);

			/* Cache Rx Attribute */
			_rtw_memcpy((void *)&rxsc_entry->rxsc_attrib,
			            rattrib, sizeof(rxsc_entry->rxsc_attrib));
			if (rxsc_entry->is_htc)
				rxsc_entry->rxsc_attrib.hdrlen -= 4;
		}
	} else
		rxsc_entry = prframe->u.hdr.rxsc_entry;
#endif /* CORE_RXSC_AMSDU */

	/* mitigate A-MSDU aggregation injection attacks */
	if (   (rx_req->rx_buf->frame_len > ETH_HLEN)
	    && _rtw_memcmp(rx_req->rx_buf->frame,
	                   rtw_rfc1042_header, SNAP_SIZE)) {
		RTW_PRINT("RX drop: The DA of 1st AMSDU subframe contains the start of an RFC1042 header\n");
		return _FAIL;
	}

	nr_subframes = 0;

	list_for_each_entry_safe(rx_buf_p, rx_buf_q, rx_buf_lst, list) {
		sub_pkt = _proc_subframe(padapter, prframe, rattrib, rx_buf_p);
		if (sub_pkt == NULL) {
			continue;
		}
		act = RTW_RX_MSDU_ACT_INDICATE;
		subframes[nr_subframes++] = sub_pkt;
	}

	if (mdata->debug_dump) {
		RTW_PRINT("%s: %u\n", __FUNCTION__, nr_subframes);
	}

	for (i = 0; i < nr_subframes; i++) {
		if (mdata->debug_dump) {
			RTW_PRINT("\t%u: %uB DA:"MAC_FMT" SA:" MAC_FMT"\n",
			          i, subframes[i]->len,
			          MAC_ARG(subframes[i]->data),
			          MAC_ARG(subframes[i]->data + 6));
		}
		/* Indicat the packets to upper layer */
		rtw_os_recv_indicate_pkt(padapter, subframes[i], prframe);
	}

	prframe->u.hdr.len = 0;
	prframe->u.hdr.pkt = NULL;

#if 0 /* def CORE_RXSC_RFRAME */
	if (rxsc_entry &&
		RXSC_ENTRY_VALID == rxsc_entry->status &&
		prframe->u.hdr.rx_req) {
		rtw_return_phl_rx_pkt(dvobj, (u8*)prframe->u.hdr.rx_req);
		rtw_os_free_recvframe(prframe);
	} else
#endif
	{
		rtw_free_recvframe(prframe, pfree_recv_queue);/* free this recv_frame */
	}

#if 0 /* def CORE_RXSC_AMSDU */
	if (rxsc_entry) {
		if (RXSC_ENTRY_VALID == rxsc_entry->status)
			padapter->core_logs.rxCnt_amsdu_shortcut++;
		else
			padapter->core_logs.rxCnt_amsdu_orig++;

		if (_SUCCESS == ret)
			rxsc_entry->status = RXSC_ENTRY_VALID;
		else
			rxsc_entry->status = RXSC_ENTRY_INVALID;
	} else {
		padapter->core_logs.rxCnt_amsdu_orig++;
	}
#endif

	return ret;
}
#endif /* CONFIG_HW_RX_AMSDU_CUT */

static int recv_process_mpdu(_adapter *padapter, union recv_frame *prframe)
{
	_queue *pfree_recv_queue = &padapter->recvpriv.free_recv_queue;
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	int ret;

	if (pattrib->amsdu) {
		ret = amsdu_to_msdu(padapter, prframe);
		if (ret != _SUCCESS) {
			#ifdef DBG_RX_DROP_FRAME
			RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" amsdu_to_msdu fail\n"
				, FUNC_ADPT_ARG(padapter));
			#endif
			rtw_free_recvframe(prframe, pfree_recv_queue);
			goto exit;
		}
	} else {
		int act = RTW_RX_MSDU_ACT_INDICATE;

		#ifdef CONFIG_RTW_MESH /* TODO: move AP mode forward & b2u logic here */
		struct xmit_frame *fwd_frame = NULL;
		_list b2u_list;

		if (MLME_IS_MESH(padapter) && pattrib->mesh_ctrl_present) {
			act = rtw_mesh_rx_msdu_act_check(prframe
				, pattrib->mda, pattrib->msa
				, pattrib->dst, pattrib->src
				, (struct rtw_ieee80211s_hdr *)(get_recvframe_data(prframe) + pattrib->hdrlen + pattrib->iv_len)
				, &fwd_frame, &b2u_list);
		}
		#endif

		#ifdef CONFIG_RTW_MESH
		if (!act) {
			rtw_free_recvframe(prframe, pfree_recv_queue);
			ret = _FAIL;
			goto exit;
		}
		#endif

		#ifdef CONFIG_RTW_SW_LED
		rtw_led_rx_control(padapter, pattrib->dst);
		#endif

		ret = wlanhdr_to_ethhdr(prframe);
		if (ret != _SUCCESS) {
			if (act & RTW_RX_MSDU_ACT_INDICATE) {
				#ifdef DBG_RX_DROP_FRAME
				RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" wlanhdr_to_ethhdr: drop pkt\n"
					, FUNC_ADPT_ARG(padapter));
				#endif
			}
			#ifdef CONFIG_RTW_MESH
			if (act & RTW_RX_MSDU_ACT_FORWARD) {
				#ifdef DBG_TX_DROP_FRAME
				RTW_INFO("DBG_TX_DROP_FRAME %s wlanhdr_to_ethhdr fail\n", __func__);
				#endif
				recv_free_fwd_resource(padapter, fwd_frame, &b2u_list);
			}
			#endif
			rtw_free_recvframe(prframe, pfree_recv_queue);
			goto exit;
		}

		#ifdef CONFIG_RTW_MESH
		if (act & RTW_RX_MSDU_ACT_FORWARD) {
			recv_fwd_pkt_hdl(padapter, prframe->u.hdr.pkt, act, fwd_frame, &b2u_list);
			if (!(act & RTW_RX_MSDU_ACT_INDICATE)) {
				prframe->u.hdr.pkt = NULL;
				rtw_free_recvframe(prframe, pfree_recv_queue);
				goto exit;
			}
		}
		#endif

		if (!RTW_CANNOT_RUN(adapter_to_dvobj(padapter))) {
			ret = rtw_recv_indicatepkt_check(prframe
				, get_recvframe_data(prframe), get_recvframe_len(prframe));
			if (ret != _SUCCESS) {
				rtw_free_recvframe(prframe, pfree_recv_queue);
				goto exit;
			}

			/* indicate this recv_frame */
			ret = rtw_recv_indicatepkt(padapter, prframe);
			if (ret != _SUCCESS) {
				#ifdef DBG_RX_DROP_FRAME
				RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" rtw_recv_indicatepkt fail!\n"
					, FUNC_ADPT_ARG(padapter));
				#endif
				rtw_free_recvframe(prframe, pfree_recv_queue);
				goto exit;
			}
		} else {
			#ifdef DBG_RX_DROP_FRAME
			RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" DS:%u SR:%u\n"
				, FUNC_ADPT_ARG(padapter)
				, dev_is_drv_stopped(adapter_to_dvobj(padapter))
				, dev_is_surprise_removed(adapter_to_dvobj(padapter)));
			#endif
			ret = _SUCCESS; /* don't count as packet drop */
			rtw_free_recvframe(prframe, pfree_recv_queue);
		}
	}

exit:
	return ret;
}

#if defined(CONFIG_80211N_HT) && defined(CONFIG_RECV_REORDERING_CTRL)
static int check_indicate_seq(struct recv_reorder_ctrl *preorder_ctrl, u16 seq_num)
{
	_adapter *padapter = preorder_ctrl->padapter;
	struct recv_priv  *precvpriv = &padapter->recvpriv;
	u8	wsize = preorder_ctrl->wsize_b;
	u16	wend;

	/* Rx Reorder initialize condition. */
	if (preorder_ctrl->indicate_seq == 0xFFFF) {
		preorder_ctrl->indicate_seq = seq_num;
		#ifdef DBG_RX_SEQ
		RTW_INFO("DBG_RX_SEQ "FUNC_ADPT_FMT" tid:%u SN_INIT indicate_seq:%d, seq_num:%d\n"
			, FUNC_ADPT_ARG(padapter), preorder_ctrl->tid, preorder_ctrl->indicate_seq, seq_num);
		#endif
	}
	wend = (preorder_ctrl->indicate_seq + wsize - 1) & 0xFFF; /* % 4096; */

	/* Drop out the packet which SeqNum is smaller than WinStart */
	if (SN_LESS(seq_num, preorder_ctrl->indicate_seq)) {
		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO(FUNC_ADPT_FMT" tid:%u indicate_seq:%d > seq_num:%d\n"
			, FUNC_ADPT_ARG(padapter), preorder_ctrl->tid, preorder_ctrl->indicate_seq, seq_num);
		#endif
		return _FALSE;
	}

	/*
	* Sliding window manipulation. Conditions includes:
	* 1. Incoming SeqNum is equal to WinStart =>Window shift 1
	* 2. Incoming SeqNum is larger than the WinEnd => Window shift N
	*/
	if (SN_EQUAL(seq_num, preorder_ctrl->indicate_seq)) {
		preorder_ctrl->indicate_seq = (preorder_ctrl->indicate_seq + 1) & 0xFFF;
		#ifdef DBG_RX_SEQ
		RTW_INFO("DBG_RX_SEQ "FUNC_ADPT_FMT" tid:%u SN_EQUAL indicate_seq:%d, seq_num:%d\n"
			, FUNC_ADPT_ARG(padapter), preorder_ctrl->tid, preorder_ctrl->indicate_seq, seq_num);
		#endif

	} else if (SN_LESS(wend, seq_num)) {
		/* boundary situation, when seq_num cross 0xFFF */
		if (seq_num >= (wsize - 1))
			preorder_ctrl->indicate_seq = seq_num + 1 - wsize;
		else
			preorder_ctrl->indicate_seq = 0xFFF - (wsize - (seq_num + 1)) + 1;

		precvpriv->dbg_rx_ampdu_window_shift_cnt++;
		#ifdef DBG_RX_SEQ
		RTW_INFO("DBG_RX_SEQ "FUNC_ADPT_FMT" tid:%u SN_LESS(wend, seq_num) indicate_seq:%d, seq_num:%d\n"
			, FUNC_ADPT_ARG(padapter), preorder_ctrl->tid, preorder_ctrl->indicate_seq, seq_num);
		#endif
	}

	return _TRUE;
}

static int enqueue_reorder_recvframe(struct recv_reorder_ctrl *preorder_ctrl, union recv_frame *prframe)
{
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	_queue *ppending_recvframe_queue = &preorder_ctrl->pending_recvframe_queue;
	_list	*phead, *plist;
	union recv_frame *pnextrframe;
	struct rx_pkt_attrib *pnextattrib;

	/* DbgPrint("+enqueue_reorder_recvframe()\n"); */

	/* _rtw_spinlock_irq(&ppending_recvframe_queue->lock, &sp_flags); */

	phead = get_list_head(ppending_recvframe_queue);
	plist = get_next(phead);

	while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		pnextrframe = LIST_CONTAINOR(plist, union recv_frame, u);
		pnextattrib = &pnextrframe->u.hdr.attrib;

		if (SN_LESS(pnextattrib->seq_num, pattrib->seq_num))
			plist = get_next(plist);
		else if (SN_EQUAL(pnextattrib->seq_num, pattrib->seq_num)) {
			/* Duplicate entry is found!! Do not insert current entry. */

			/* _rtw_spinunlock_irq(&ppending_recvframe_queue->lock, &sp_flags); */

			return _FALSE;
		} else
			break;

		/* DbgPrint("enqueue_reorder_recvframe():while\n"); */

	}


	/* _rtw_spinlock_irq(&ppending_recvframe_queue->lock, &sp_flags); */

	rtw_list_delete(&(prframe->u.hdr.list));

	rtw_list_insert_tail(&(prframe->u.hdr.list), plist);

	/* _rtw_spinunlock_irq(&ppending_recvframe_queue->lock, &sp_flags); */


	return _TRUE;

}

static void recv_indicatepkts_pkt_loss_cnt(_adapter *padapter, u64 prev_seq, u64 current_seq)
{
	struct recv_priv *precvpriv = &padapter->recvpriv;

	if (current_seq < prev_seq) {
		precvpriv->dbg_rx_ampdu_loss_count += (4096 + current_seq - prev_seq);
		precvpriv->rx_drop += (4096 + current_seq - prev_seq);
	} else {
		precvpriv->dbg_rx_ampdu_loss_count += (current_seq - prev_seq);
		precvpriv->rx_drop += (current_seq - prev_seq);
	}
}

static int recv_indicatepkts_in_order(_adapter *padapter, struct recv_reorder_ctrl *preorder_ctrl, int bforced)
{
	_list	*phead, *plist;
	union recv_frame *prframe;
	struct rx_pkt_attrib *pattrib;
	/* u8 index = 0; */
	int bPktInBuf = _FALSE;
	struct recv_priv *precvpriv = &padapter->recvpriv;
	_queue *ppending_recvframe_queue = &preorder_ctrl->pending_recvframe_queue;

	DBG_COUNTER(padapter->rx_logs.core_rx_post_indicate_in_oder);

	/* DbgPrint("+recv_indicatepkts_in_order\n"); */

	/* _rtw_spinlock_irq(&ppending_recvframe_queue->lock, &sp_flags); */

	phead =	get_list_head(ppending_recvframe_queue);
	plist = get_next(phead);

#if 0
	/* Check if there is any other indication thread running. */
	if (pTS->RxIndicateState == RXTS_INDICATE_PROCESSING)
		return;
#endif

	/* Handling some condition for forced indicate case. */
	if (bforced == _TRUE) {
		precvpriv->dbg_rx_ampdu_forced_indicate_count++;
		if (rtw_is_list_empty(phead)) {
			/* _rtw_spinunlock_irq(&ppending_recvframe_queue->lock, &sp_flags); */
			return _TRUE;
		}

		prframe = LIST_CONTAINOR(plist, union recv_frame, u);
		pattrib = &prframe->u.hdr.attrib;

		#ifdef DBG_RX_SEQ
		RTW_INFO("DBG_RX_SEQ "FUNC_ADPT_FMT" tid:%u FORCE indicate_seq:%d, seq_num:%d\n"
			, FUNC_ADPT_ARG(padapter), preorder_ctrl->tid, preorder_ctrl->indicate_seq, pattrib->seq_num);
		#endif
		recv_indicatepkts_pkt_loss_cnt(padapter, preorder_ctrl->indicate_seq, pattrib->seq_num);
		preorder_ctrl->indicate_seq = pattrib->seq_num;
	}

	/* Prepare indication list and indication. */
	/* Check if there is any packet need indicate. */
	while (!rtw_is_list_empty(phead)) {

		prframe = LIST_CONTAINOR(plist, union recv_frame, u);
		pattrib = &prframe->u.hdr.attrib;

		if (!SN_LESS(preorder_ctrl->indicate_seq, pattrib->seq_num)) {

#if 0
			/* This protect buffer from overflow. */
			if (index >= REORDER_WIN_SIZE) {
				RT_ASSERT(FALSE, ("IndicateRxReorderList(): Buffer overflow!!\n"));
				bPktInBuf = TRUE;
				break;
			}
#endif

			plist = get_next(plist);
			rtw_list_delete(&(prframe->u.hdr.list));

			if (SN_EQUAL(preorder_ctrl->indicate_seq, pattrib->seq_num)) {
				preorder_ctrl->indicate_seq = (preorder_ctrl->indicate_seq + 1) & 0xFFF;
				#ifdef DBG_RX_SEQ
				RTW_INFO("DBG_RX_SEQ "FUNC_ADPT_FMT" tid:%u SN_EQUAL indicate_seq:%d, seq_num:%d\n"
					, FUNC_ADPT_ARG(padapter), preorder_ctrl->tid, preorder_ctrl->indicate_seq, pattrib->seq_num);
				#endif
			}

#if 0
			index++;
			if (index == 1) {
				/* Cancel previous pending timer. */
				/* PlatformCancelTimer(adapter, &pTS->RxPktPendingTimer); */
				if (bforced != _TRUE) {
					/* RTW_INFO("_cancel_timer_ex(&preorder_ctrl->reordering_ctrl_timer);\n"); */
					_cancel_timer_ex(&preorder_ctrl->reordering_ctrl_timer);
				}
			}
#endif

			/* Set this as a lock to make sure that only one thread is indicating packet. */
			/* pTS->RxIndicateState = RXTS_INDICATE_PROCESSING; */

			/* Indicate packets */
			/* RT_ASSERT((index<=REORDER_WIN_SIZE), ("RxReorderIndicatePacket(): Rx Reorder buffer full!!\n")); */


			/* indicate this recv_frame */
			/* DbgPrint("recv_indicatepkts_in_order, indicate_seq=%d, seq_num=%d\n", precvpriv->indicate_seq, pattrib->seq_num); */
			if (recv_process_mpdu(padapter, prframe) != _SUCCESS)
				precvpriv->dbg_rx_drop_count++;

			/* Update local variables. */
			bPktInBuf = _FALSE;

		} else {
			bPktInBuf = _TRUE;
			break;
		}

		/* DbgPrint("recv_indicatepkts_in_order():while\n"); */

	}

	/* _rtw_spinunlock_irq(&ppending_recvframe_queue->lock, sp_flags); */

#if 0
	/* Release the indication lock and set to new indication step. */
	if (bPktInBuf) {
		/*  Set new pending timer. */
		/* pTS->RxIndicateState = RXTS_INDICATE_REORDER; */
		/* PlatformSetTimer(adapter, &pTS->RxPktPendingTimer, pHTInfo->RxReorderPendingTime); */

		_set_timer(&preorder_ctrl->reordering_ctrl_timer, REORDER_WAIT_TIME);
	} else {
		/* pTS->RxIndicateState = RXTS_INDICATE_IDLE; */
	}
#endif
	/* _rtw_spinunlock_irq(&ppending_recvframe_queue->lock, sp_flags); */

	/* return _TRUE; */
	return bPktInBuf;

}

static int recv_indicatepkt_reorder(_adapter *padapter, union recv_frame *prframe)
{
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	struct recv_reorder_ctrl *preorder_ctrl = prframe->u.hdr.preorder_ctrl;
	_queue *ppending_recvframe_queue = preorder_ctrl ? &preorder_ctrl->pending_recvframe_queue : NULL;
	struct recv_priv  *precvpriv = &padapter->recvpriv;

	if (!pattrib->qos || !preorder_ctrl || preorder_ctrl->enable == _FALSE)
		goto _success_exit;

	DBG_COUNTER(padapter->rx_logs.core_rx_post_indicate_reoder);

	_rtw_spinlock_bh(&ppending_recvframe_queue->lock);

	if(rtw_test_and_clear_bit(RTW_RECV_ACK_OR_TIMEOUT, &preorder_ctrl->rec_abba_rsp_ack))
		preorder_ctrl->indicate_seq = 0xFFFF;
	#ifdef DBG_RX_SEQ
	RTW_INFO("DBG_RX_SEQ %s:preorder_ctrl->rec_abba_rsp_ack = %u,indicate_seq = %d\n"
		, __func__
		, preorder_ctrl->rec_abba_rsp_ack
		, preorder_ctrl->indicate_seq);
	#endif

	/* s2. check if winstart_b(indicate_seq) needs to been updated */
	if (!check_indicate_seq(preorder_ctrl, pattrib->seq_num)) {
		precvpriv->dbg_rx_ampdu_drop_count++;
		/* pHTInfo->RxReorderDropCounter++; */
		/* ReturnRFDList(adapter, pRfd); */
		/* _rtw_spinunlock_irq(&ppending_recvframe_queue->lock, sp_flags); */
		/* return _FAIL; */

		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" check_indicate_seq fail\n"
			, FUNC_ADPT_ARG(padapter));
		#endif
#if 0
		rtw_recv_indicatepkt(padapter, prframe);

		_rtw_spinunlock_bh(&ppending_recvframe_queue->lock);

		goto _success_exit;
#else
		goto _err_exit;
#endif
	}


	/* s3. Insert all packet into Reorder Queue to maintain its ordering. */
	if (!enqueue_reorder_recvframe(preorder_ctrl, prframe)) {
		/* DbgPrint("recv_indicatepkt_reorder, enqueue_reorder_recvframe fail!\n"); */
		/* _rtw_spinunlock_irq(&ppending_recvframe_queue->lock, sp_flags); */
		/* return _FAIL; */
		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" enqueue_reorder_recvframe fail\n"
			, FUNC_ADPT_ARG(padapter));
		#endif
		goto _err_exit;
	}


	/* s4. */
	/* Indication process. */
	/* After Packet dropping and Sliding Window shifting as above, we can now just indicate the packets */
	/* with the SeqNum smaller than latest WinStart and buffer other packets. */
	/*  */
	/* For Rx Reorder condition: */
	/* 1. All packets with SeqNum smaller than WinStart => Indicate */
	/* 2. All packets with SeqNum larger than or equal to WinStart => Buffer it. */
	/*  */

	/* recv_indicatepkts_in_order(padapter, preorder_ctrl, _TRUE); */
	if (recv_indicatepkts_in_order(padapter, preorder_ctrl, _FALSE) == _TRUE) {
		if (!preorder_ctrl->bReorderWaiting) {
			preorder_ctrl->bReorderWaiting = _TRUE;
			_set_timer(&preorder_ctrl->reordering_ctrl_timer, REORDER_WAIT_TIME);
		}
		_rtw_spinunlock_bh(&ppending_recvframe_queue->lock);
	} else {
		preorder_ctrl->bReorderWaiting = _FALSE;
		_rtw_spinunlock_bh(&ppending_recvframe_queue->lock);
		_cancel_timer_ex(&preorder_ctrl->reordering_ctrl_timer);
	}

	return RTW_RX_HANDLED;

_success_exit:

	return _SUCCESS;

_err_exit:

	_rtw_spinunlock_bh(&ppending_recvframe_queue->lock);

	return _FAIL;
}


void rtw_reordering_ctrl_timeout_handler(void *pcontext)
{
	struct recv_reorder_ctrl *preorder_ctrl = (struct recv_reorder_ctrl *)pcontext;
	_adapter *padapter = NULL;
	_queue *ppending_recvframe_queue = NULL;


	if (preorder_ctrl == NULL)
		return;

	padapter = preorder_ctrl->padapter;
	if (RTW_CANNOT_RUN(adapter_to_dvobj(padapter)))
		return;

	ppending_recvframe_queue = &preorder_ctrl->pending_recvframe_queue;

	/* RTW_INFO("+rtw_reordering_ctrl_timeout_handler()=>\n"); */

	_rtw_spinlock_bh(&ppending_recvframe_queue->lock);

	preorder_ctrl->bReorderWaiting = _FALSE;

	if (recv_indicatepkts_in_order(padapter, preorder_ctrl, _TRUE) == _TRUE)
		_set_timer(&preorder_ctrl->reordering_ctrl_timer, REORDER_WAIT_TIME);

	_rtw_spinunlock_bh(&ppending_recvframe_queue->lock);

}
#endif /* defined(CONFIG_80211N_HT) && defined(CONFIG_RECV_REORDERING_CTRL) */

static void recv_set_iseq_before_mpdu_process(union recv_frame *rframe, u16 seq_num, const char *caller)
{
#if defined(CONFIG_80211N_HT) && defined(CONFIG_RECV_REORDERING_CTRL)
	struct recv_reorder_ctrl *reorder_ctrl = rframe->u.hdr.preorder_ctrl;

	if (reorder_ctrl) {
		reorder_ctrl->indicate_seq = seq_num;
		#ifdef DBG_RX_SEQ
		RTW_INFO("DBG_RX_SEQ %s("ADPT_FMT")-B tid:%u indicate_seq:%d, seq_num:%d\n"
			, caller, ADPT_ARG(reorder_ctrl->padapter)
			, reorder_ctrl->tid, reorder_ctrl->indicate_seq, seq_num);
		#endif
	}
#endif
}

static void recv_set_iseq_after_mpdu_process(union recv_frame *rframe, u16 seq_num, const char *caller)
{
#if defined(CONFIG_80211N_HT) && defined(CONFIG_RECV_REORDERING_CTRL)
	struct recv_reorder_ctrl *reorder_ctrl = rframe->u.hdr.preorder_ctrl;

	if (reorder_ctrl) {
		reorder_ctrl->indicate_seq = (reorder_ctrl->indicate_seq + 1) % 4096;
		#ifdef DBG_RX_SEQ
		RTW_INFO("DBG_RX_SEQ %s("ADPT_FMT")-A tid:%u indicate_seq:%d, seq_num:%d\n"
			, caller, ADPT_ARG(reorder_ctrl->padapter)
			, reorder_ctrl->tid, reorder_ctrl->indicate_seq, seq_num);
		#endif
	}
#endif
}

#ifdef CONFIG_MP_INCLUDED
int validate_mp_recv_frame(_adapter *adapter, union recv_frame *precv_frame)
{
	int ret = _SUCCESS;
	u8 *ptr = precv_frame->u.hdr.rx_data;
	u8 type, subtype;
	struct mp_priv *pmppriv = &adapter->mppriv;
	struct mp_tx		*pmptx;
	unsigned char	*sa , *da, *bs;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	u32 i = 0;
	u8 rtk_prefix[]={0x52, 0x65, 0x61, 0x6C, 0x4C, 0x6F, 0x76, 0x65, 0x54, 0x65, 0x6B};
	u8 *prx_data;
	pmptx = &pmppriv->tx;


	if (pmppriv->mplink_brx == _FALSE) {

		u8 bDumpRxPkt = 0;
		type =  GetFrameType(ptr);
		subtype = get_frame_sub_type(ptr); /* bit(7)~bit(2)	 */

		RTW_INFO("hdr len = %d iv_len=%d \n", pattrib->hdrlen , pattrib->iv_len);
		prx_data = ptr + pattrib->hdrlen + pattrib->iv_len;

		for (i = 0; i < precv_frame->u.hdr.len; i++) {
			if (precv_frame->u.hdr.len < (11 + i))
				break;

			if (_rtw_memcmp(prx_data + i, (void *)&rtk_prefix, 11) == _FALSE) {
				bDumpRxPkt = 0;
				RTW_DBG("prx_data = %02X != rtk_prefix[%d] = %02X \n", *(prx_data + i), i , rtk_prefix[i]);
				} else {
				bDumpRxPkt = 1;
				RTW_DBG("prx_data = %02X = rtk_prefix[%d] = %02X \n", *(prx_data + i), i , rtk_prefix[i]);
				break;
				}
		}

		if (bDumpRxPkt == 1) { /* dump all rx packets */
			int i;
			RTW_INFO("############ type:0x%02x subtype:0x%02x #################\n", type, subtype);

			for (i = 0; i < precv_frame->u.hdr.len; i = i + 8)
				RTW_INFO("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:\n", *(ptr + i),
					*(ptr + i + 1), *(ptr + i + 2) , *(ptr + i + 3) , *(ptr + i + 4), *(ptr + i + 5), *(ptr + i + 6), *(ptr + i + 7));
				RTW_INFO("#############################\n");
				_rtw_memset(pmppriv->mplink_buf, '\0' , sizeof(pmppriv->mplink_buf));
				_rtw_memcpy(pmppriv->mplink_buf, ptr, precv_frame->u.hdr.len);
				pmppriv->mplink_rx_len = precv_frame->u.hdr.len;
				pmppriv->mplink_brx =_TRUE;
		}
	}
	if (pmppriv->bloopback) {
		if (_rtw_memcmp(ptr + 24, pmptx->buf + 24, precv_frame->u.hdr.len - 24) == _FALSE) {
			RTW_INFO("Compare payload content Fail !!!\n");
			ret = _FAIL;
		}
	}
 	if (pmppriv->bSetRxBssid == _TRUE) {

		sa = get_addr2_ptr(ptr);
		da = GetAddr1Ptr(ptr);
		bs = GetAddr3Ptr(ptr);
		type =	GetFrameType(ptr);
		subtype = get_frame_sub_type(ptr); /* bit(7)~bit(2)  */

		if (_rtw_memcmp(bs, adapter->mppriv.network_macaddr, ETH_ALEN) == _FALSE)
			ret = _FAIL;

		RTW_DBG("############ type:0x%02x subtype:0x%02x #################\n", type, subtype);
		RTW_DBG("A2 sa %02X:%02X:%02X:%02X:%02X:%02X \n", *(sa) , *(sa + 1), *(sa+ 2), *(sa + 3), *(sa + 4), *(sa + 5));
		RTW_DBG("A1 da %02X:%02X:%02X:%02X:%02X:%02X \n", *(da) , *(da + 1), *(da+ 2), *(da + 3), *(da + 4), *(da + 5));
		RTW_DBG("A3 bs %02X:%02X:%02X:%02X:%02X:%02X \n --------------------------\n", *(bs) , *(bs + 1), *(bs+ 2), *(bs + 3), *(bs + 4), *(bs + 5));
	}

	if (!adapter->mppriv.bmac_filter)
		return ret;

	if (_rtw_memcmp(get_addr2_ptr(ptr), adapter->mppriv.mac_filter, ETH_ALEN) == _FALSE)
		ret = _FAIL;

	return ret;
}

static sint MPwlanhdr_to_ethhdr(union recv_frame *precvframe)
{
	sint	rmv_len;
	u16 eth_type, len;
	u8	bsnaphdr;
	u8	*psnap_type;
	u8 mcastheadermac[] = {0x01, 0x00, 0x5e};

	struct ieee80211_snap_hdr	*psnap;

	sint ret = _SUCCESS;
	_adapter	*adapter = precvframe->u.hdr.adapter;

	u8	*ptr = get_recvframe_data(precvframe) ; /* point to frame_ctrl field */
	struct rx_pkt_attrib *pattrib = &precvframe->u.hdr.attrib;


	if (pattrib->encrypt)
		recvframe_pull_tail(precvframe, pattrib->icv_len);

	psnap = (struct ieee80211_snap_hdr *)(ptr + pattrib->hdrlen + pattrib->iv_len);
	psnap_type = ptr + pattrib->hdrlen + pattrib->iv_len + SNAP_SIZE;
	/* convert hdr + possible LLC headers into Ethernet header */
	/* eth_type = (psnap_type[0] << 8) | psnap_type[1]; */
	if ((_rtw_memcmp(psnap, rtw_rfc1042_header, SNAP_SIZE) &&
	     (_rtw_memcmp(psnap_type, SNAP_ETH_TYPE_IPX, 2) == _FALSE) &&
	     (_rtw_memcmp(psnap_type, SNAP_ETH_TYPE_APPLETALK_AARP, 2) == _FALSE)) ||
	    /* eth_type != ETH_P_AARP && eth_type != ETH_P_IPX) || */
	    _rtw_memcmp(psnap, rtw_bridge_tunnel_header, SNAP_SIZE)) {
		/* remove RFC1042 or Bridge-Tunnel encapsulation and replace EtherType */
		bsnaphdr = _TRUE;
	} else {
		/* Leave Ethernet header part of hdr and full payload */
		bsnaphdr = _FALSE;
	}

	rmv_len = pattrib->hdrlen + pattrib->iv_len + (bsnaphdr ? SNAP_SIZE : 0);
	len = precvframe->u.hdr.len - rmv_len;


	_rtw_memcpy(&eth_type, ptr + rmv_len, 2);
	if(eth_type == ntohs(ETH_P_8021Q))
		_rtw_memcpy(&eth_type, ptr + rmv_len + VLAN_TAG_LEN, 2);

	eth_type = ntohs((unsigned short)eth_type); /* pattrib->ether_type */
	pattrib->eth_type = eth_type;

	{
		ptr = recvframe_pull(precvframe, (rmv_len - sizeof(struct ethhdr) + (bsnaphdr ? 2 : 0)));
	}

	_rtw_memcpy(ptr, pattrib->dst, ETH_ALEN);
	_rtw_memcpy(ptr + ETH_ALEN, pattrib->src, ETH_ALEN);

	if (!bsnaphdr) {
		len = htons(len);
		_rtw_memcpy(ptr + 12, &len, 2);
	}


	len = htons(pattrib->seq_num);
	/* RTW_INFO("wlan seq = %d ,seq_num =%x\n",len,pattrib->seq_num); */
	_rtw_memcpy(ptr + 12, &len, 2);
	if (adapter->mppriv.bRTWSmbCfg == _TRUE) {
		/* if(_rtw_memcmp(mcastheadermac, pattrib->dst, 3) == _TRUE) */ /* SimpleConfig Dest. */
		/*			_rtw_memcpy(ptr+ETH_ALEN, pattrib->bssid, ETH_ALEN); */

		if (_rtw_memcmp(mcastheadermac, pattrib->bssid, 3) == _TRUE) /* SimpleConfig Dest. */
			_rtw_memcpy(ptr, pattrib->bssid, ETH_ALEN);

	}


	return ret;

}


int mp_recv_frame(_adapter *padapter, union recv_frame *rframe)
{
	int ret = _SUCCESS;
	struct rx_pkt_attrib *pattrib = &rframe->u.hdr.attrib;
	_queue *pfree_recv_queue = &padapter->recvpriv.free_recv_queue;
#ifdef CONFIG_MP_INCLUDED
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mp_priv *pmppriv = &padapter->mppriv;
#endif /* CONFIG_MP_INCLUDED */
	u8 type;
	u8 *ptr = rframe->u.hdr.rx_data;
	u8 *psa, *pda, *pbssid;
	struct sta_info *psta = NULL;
	DBG_COUNTER(padapter->rx_logs.core_rx_pre);

	if ((check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE)) { /* &&(padapter->mppriv.check_mp_pkt == 0)) */
		if (pattrib->crc_err == 1)
			padapter->mppriv.rx_crcerrpktcount++;
		else {
			if (_SUCCESS == validate_mp_recv_frame(padapter, rframe))
				padapter->mppriv.rx_pktcount++;
			else
				padapter->mppriv.rx_pktcount_filter_out++;
		}

		if (pmppriv->rx_bindicatePkt == _FALSE) {
			ret = _FAIL;
			rtw_free_recvframe(rframe, pfree_recv_queue);/* free this recv_frame */
			goto exit;
		} else {
			type =	GetFrameType(ptr);
			pattrib->to_fr_ds = get_tofr_ds(ptr);
			pattrib->frag_num = GetFragNum(ptr);
			pattrib->seq_num = GetSequence(ptr);
			pattrib->pw_save = GetPwrMgt(ptr);
			pattrib->mfrag = GetMFrag(ptr);
			pattrib->mdata = GetMData(ptr);
			pattrib->privacy = GetPrivacy(ptr);
			pattrib->order = GetOrder(ptr);

			if (type == WIFI_DATA_TYPE) {
				pda = get_da(ptr);
				psa = get_sa(ptr);
				pbssid = get_hdr_bssid(ptr);

				_rtw_memcpy(pattrib->dst, pda, ETH_ALEN);
				_rtw_memcpy(pattrib->src, psa, ETH_ALEN);
				_rtw_memcpy(pattrib->bssid, pbssid, ETH_ALEN);

				switch (pattrib->to_fr_ds) {
				case 0:
					_rtw_memcpy(pattrib->ra, pda, ETH_ALEN);
					_rtw_memcpy(pattrib->ta, psa, ETH_ALEN);
					ret = sta2sta_data_frame(padapter, rframe, &psta);
					break;

				case 1:
					_rtw_memcpy(pattrib->ra, pda, ETH_ALEN);
					_rtw_memcpy(pattrib->ta, pbssid, ETH_ALEN);
					ret = ap2sta_data_frame(padapter, rframe, &psta);
					break;

				case 2:
					_rtw_memcpy(pattrib->ra, pbssid, ETH_ALEN);
					_rtw_memcpy(pattrib->ta, psa, ETH_ALEN);
					ret = sta2ap_data_frame(padapter, rframe, &psta);
					break;
				case 3:
					_rtw_memcpy(pattrib->ra, GetAddr1Ptr(ptr), ETH_ALEN);
					_rtw_memcpy(pattrib->ta, get_addr2_ptr(ptr), ETH_ALEN);
					ret = _FAIL;
					break;
				default:
					ret = _FAIL;
					break;
				}

				if (ret != _SUCCESS) {
#ifdef DBG_RX_DROP_FRAME
					RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" 2_data_frame fail: drop pkt\n"
															, FUNC_ADPT_ARG(padapter));
#endif
					ret = _FAIL;
					goto exit;
				}

				ret = MPwlanhdr_to_ethhdr(rframe);

				if (ret != _SUCCESS) {
					#ifdef DBG_RX_DROP_FRAME
					RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" wlanhdr_to_ethhdr: drop pkt\n"
						, FUNC_ADPT_ARG(padapter));
					#endif
					ret = _FAIL;
					goto exit;
				}
				if (!RTW_CANNOT_RUN(adapter_to_dvobj(padapter))) {
					/* indicate this recv_frame */
					ret = rtw_recv_indicatepkt(padapter, rframe);
					if (ret != _SUCCESS) {
						#ifdef DBG_RX_DROP_FRAME
						RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" rtw_recv_indicatepkt fail!\n"
							, FUNC_ADPT_ARG(padapter));
						#endif
						ret = _FAIL;
						goto exit;
					}
				} else {
					#ifdef DBG_RX_DROP_FRAME
					RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" bDriverStopped(%s) OR bSurpriseRemoved(%s)\n"
						, FUNC_ADPT_ARG(padapter)
						, dev_is_drv_stopped(adapter_to_dvobj(padapter)) ? "True" : "False"
						, dev_is_surprise_removed(adapter_to_dvobj(padapter)) ? "True" : "False");
					#endif
					ret = _FAIL;
					goto exit;
				}

			}
		}
	}
exit:
	rtw_free_recvframe(rframe, pfree_recv_queue);/* free this recv_frame */
	ret = _FAIL;
	return ret;

}
#endif


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
int recv_frame_monitor(_adapter *padapter, union recv_frame *rframe)
{
	int ret = _SUCCESS;
	_queue *pfree_recv_queue = &padapter->recvpriv.free_recv_queue;

#ifdef CONFIG_WIFI_MONITOR
	struct net_device *ndev = padapter->pnetdev;
	struct sk_buff *pskb = NULL;

	if (rframe == NULL)
		goto exit;

	/* read skb information from recv frame */
	pskb = rframe->u.hdr.pkt;
	pskb->len = rframe->u.hdr.len;
	pskb->data = rframe->u.hdr.rx_data;
	skb_set_tail_pointer(pskb, rframe->u.hdr.len);

	if (ndev->type == ARPHRD_IEEE80211_RADIOTAP) {
		/* fill radiotap header */
		if (rtw_fill_radiotap_hdr(padapter, &rframe->u.hdr.attrib, (u8 *)pskb) == _FAIL) {
			ret = _FAIL;
			goto exit;
		}
	}

	/* write skb information to recv frame */
	skb_reset_mac_header(pskb);
	rframe->u.hdr.len = pskb->len;
	rframe->u.hdr.rx_data = pskb->data;
	rframe->u.hdr.rx_head = pskb->head;
	rframe->u.hdr.rx_tail = skb_tail_pointer(pskb);
	rframe->u.hdr.rx_end = skb_end_pointer(pskb);

	if (!RTW_CANNOT_RUN(adapter_to_dvobj(padapter))) {
		/* indicate this recv_frame */
		ret = rtw_recv_monitor(padapter, rframe);
	} else
		ret = _FAIL;

exit:
#endif /* CONFIG_WIFI_MONITOR */

	if (rframe) /* free this recv_frame */
		rtw_free_recvframe(rframe, pfree_recv_queue);

	return ret;
}
#endif
int recv_func_prehandle(_adapter *padapter, union recv_frame *rframe)
{
	int ret = _SUCCESS;
#ifdef DBG_RX_COUNTER_DUMP
	struct rx_pkt_attrib *pattrib = &rframe->u.hdr.attrib;
#endif
	_queue *pfree_recv_queue = &padapter->recvpriv.free_recv_queue;

#ifdef DBG_RX_COUNTER_DUMP
	if (padapter->dump_rx_cnt_mode & DUMP_DRV_RX_COUNTER) {
		if (pattrib->crc_err == 1)
			padapter->drv_rx_cnt_crcerror++;
		else
			padapter->drv_rx_cnt_ok++;
	}
#endif

#ifdef CONFIG_MP_INCLUDED
	if (padapter->registrypriv.mp_mode == 1 || padapter->mppriv.bRTWSmbCfg == _TRUE) {
		mp_recv_frame(padapter, rframe);
		ret = _FAIL;
		goto exit;
	} else
#endif
	{
		/* check the frame crtl field and decache */
		ret = validate_recv_frame(padapter, rframe);
		if (ret != _SUCCESS) {
			rtw_free_recvframe(rframe, pfree_recv_queue);/* free this recv_frame */
			goto exit;
		}
	}
exit:
	return ret;
}

/*#define DBG_RX_BMC_FRAME*/
int recv_func_posthandle(_adapter *padapter, union recv_frame *prframe)
{
	int ret = _SUCCESS;
	union recv_frame *orig_prframe = prframe;
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	struct recv_priv *precvpriv = &padapter->recvpriv;
	_queue *pfree_recv_queue = &padapter->recvpriv.free_recv_queue;
#ifdef CONFIG_TDLS
	u8 *psnap_type, *pcategory;
#endif /* CONFIG_TDLS */

	DBG_COUNTER(padapter->rx_logs.core_rx_post);

	prframe = decryptor(padapter, prframe);
	if (prframe == NULL) {
		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" decryptor: drop pkt\n"
			, FUNC_ADPT_ARG(padapter));
		#endif
		ret = _FAIL;
		DBG_COUNTER(padapter->rx_logs.core_rx_post_decrypt_err);
		goto _recv_data_drop;
	}

#ifdef DBG_RX_BMC_FRAME
	if (IS_MCAST(pattrib->ra))
		RTW_INFO("%s =>"ADPT_FMT" Rx BC/MC from "MAC_FMT"\n", __func__, ADPT_ARG(padapter), MAC_ARG(pattrib->ta));
#endif

#if 0
	if (is_primary_adapter(padapter)) {
		RTW_INFO("+++\n");
		{
			int i;
			u8	*ptr = get_recvframe_data(prframe);
			for (i = 0; i < 140; i = i + 8)
				RTW_INFO("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:", *(ptr + i),
					*(ptr + i + 1), *(ptr + i + 2) , *(ptr + i + 3) , *(ptr + i + 4), *(ptr + i + 5), *(ptr + i + 6), *(ptr + i + 7));

		}
		RTW_INFO("---\n");
	}
#endif

#ifdef CONFIG_TDLS
	/* check TDLS frame */
	psnap_type = get_recvframe_data(orig_prframe) + pattrib->hdrlen + pattrib->iv_len + SNAP_SIZE;
	pcategory = psnap_type + ETH_TYPE_LEN + PAYLOAD_TYPE_LEN;

	if ((_rtw_memcmp(psnap_type, SNAP_ETH_TYPE_TDLS, ETH_TYPE_LEN)) &&
	    ((*pcategory == RTW_WLAN_CATEGORY_TDLS) || (*pcategory == RTW_WLAN_CATEGORY_P2P))) {
		ret = OnTDLS(padapter, prframe);
		if (ret == _FAIL)
			goto _exit_recv_func;
	}
#endif /* CONFIG_TDLS */

	ret = recvframe_chk_defrag(padapter, &prframe);
	if (ret != CORE_RX_CONTINUE)	{
		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" recvframe_chk_defrag: drop pkt\n"
			, FUNC_ADPT_ARG(padapter));
		#endif
		DBG_COUNTER(padapter->rx_logs.core_rx_post_defrag_err);
		goto _recv_data_drop;
	}

	prframe = portctrl(padapter, prframe);
	if (prframe == NULL) {
		#ifdef DBG_RX_DROP_FRAME
		RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" portctrl: drop pkt\n"
			, FUNC_ADPT_ARG(padapter));
		#endif
		ret = _FAIL;
		DBG_COUNTER(padapter->rx_logs.core_rx_post_portctrl_err);
		goto _recv_data_drop;
	}

	count_rx_stats(padapter, prframe, NULL);

#ifdef CONFIG_WAPI_SUPPORT
	rtw_wapi_update_info(padapter, prframe);
#endif

#if defined(CONFIG_80211N_HT) && defined(CONFIG_RECV_REORDERING_CTRL)
	/* including perform A-MPDU Rx Ordering Buffer Control */
	ret = recv_indicatepkt_reorder(padapter, prframe);
	if (ret == _FAIL) {
		goto _recv_data_drop;
	} else if (ret == RTW_RX_HANDLED) /* queued OR indicated in order */
		goto _exit_recv_func;
#endif

	recv_set_iseq_before_mpdu_process(prframe, pattrib->seq_num, __func__);
	ret = recv_process_mpdu(padapter, prframe);
	recv_set_iseq_after_mpdu_process(prframe, pattrib->seq_num, __func__);
	if (ret == _FAIL)
		goto _recv_data_drop;

_exit_recv_func:
	return ret;

_recv_data_drop:
	precvpriv->dbg_rx_drop_count++;
	rtw_free_recvframe(orig_prframe, pfree_recv_queue);
	return ret;
}

int recv_func(_adapter *padapter, union recv_frame *rframe)
{
	int ret;
	struct rx_pkt_attrib *prxattrib = &rframe->u.hdr.attrib;
	struct recv_priv *recvpriv = &padapter->recvpriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_priv *mlmepriv = &padapter->mlmepriv;
#ifdef CONFIG_CUSTOMER_ALIBABA_GENERAL
	u8 type;
	u8 *ptr = rframe->u.hdr.rx_data;
#endif

	if (check_fwstate(mlmepriv, WIFI_MONITOR_STATE)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
		recv_frame_monitor(padapter, rframe);
#endif
		ret = _SUCCESS;
		goto exit;
	}

#ifdef CONFIG_CUSTOMER_ALIBABA_GENERAL
	type = GetFrameType(ptr);
	if ((type == WIFI_DATA_TYPE)&& MLME_IS_STA(padapter)) {
		struct wlan_network *cur_network = &(mlmepriv->cur_network);
		if ( _rtw_memcmp(get_addr2_ptr(ptr), cur_network->network.MacAddress, ETH_ALEN)==0) {
			recv_frame_monitor(padapter, rframe);
			ret = _SUCCESS;
			goto exit;
		}
	}
#endif
	/* check if need to handle uc_swdec_pending_queue*/
	if (MLME_IS_STA(padapter) && psecuritypriv->busetkipkey) {
		union recv_frame *pending_frame;
		int cnt = 0;

		while ((pending_frame = rtw_alloc_recvframe(&padapter->recvpriv.uc_swdec_pending_queue))) {
			cnt++;
			DBG_COUNTER(padapter->rx_logs.core_rx_dequeue);
			recv_func_posthandle(padapter, pending_frame);
		}

		if (cnt)
			RTW_INFO(FUNC_ADPT_FMT" dequeue %d from uc_swdec_pending_queue\n",
				 FUNC_ADPT_ARG(padapter), cnt);
	}

	DBG_COUNTER(padapter->rx_logs.core_rx);
	ret = recv_func_prehandle(padapter, rframe);

	if (ret == _SUCCESS) {

		/* check if need to enqueue into uc_swdec_pending_queue*/
		if (MLME_IS_STA(padapter) &&
		    !IS_MCAST(prxattrib->ra) && prxattrib->encrypt > 0 &&
		    (prxattrib->bdecrypted == 0 || psecuritypriv->sw_decrypt == _TRUE) &&
		    psecuritypriv->ndisauthtype == Ndis802_11AuthModeWPAPSK &&
		    !psecuritypriv->busetkipkey) {
			DBG_COUNTER(padapter->rx_logs.core_rx_enqueue);
			rtw_enqueue_recvframe(rframe, &padapter->recvpriv.uc_swdec_pending_queue);
			/* RTW_INFO("%s: no key, enqueue uc_swdec_pending_queue\n", __func__); */

			if (recvpriv->free_recvframe_cnt < NR_RECVFRAME / 4) {
				/* to prevent from recvframe starvation, get recvframe from uc_swdec_pending_queue to free_recvframe_cnt */
				rframe = rtw_alloc_recvframe(&padapter->recvpriv.uc_swdec_pending_queue);
				if (rframe)
					goto do_posthandle;
			}
			goto exit;
		}

do_posthandle:
		ret = recv_func_posthandle(padapter, rframe);
	}

exit:
	return ret;
}


s32 rtw_recv_entry(union recv_frame *precvframe)
{
	_adapter *padapter;
	struct recv_priv *precvpriv;
	s32 ret = _SUCCESS;



	padapter = precvframe->u.hdr.adapter;

	precvpriv = &padapter->recvpriv;


	ret = recv_func(padapter, precvframe);
	if (ret == _FAIL) {
		goto _recv_entry_drop;
	}


	precvpriv->rx_pkts++;


	return ret;

_recv_entry_drop:

#ifdef CONFIG_MP_INCLUDED
	if (padapter->registrypriv.mp_mode == 1)
		padapter->mppriv.rx_pktloss = precvpriv->rx_drop;
#endif



	return ret;
}

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
static void rtw_signal_stat_timer_hdl(void *ctx)
{
	_adapter *adapter = (_adapter *)ctx;
	struct recv_priv *recvpriv = &adapter->recvpriv;

	u32 tmp_s, tmp_q;
	u8 avg_signal_strength = 0;
	u8 avg_signal_qual = 0;
	u32 num_signal_strength = 0;
	u32 num_signal_qual = 0;
	u8 ratio_pre_stat = 0, ratio_curr_stat = 0, ratio_total = 0, ratio_profile = SIGNAL_STAT_CALC_PROFILE_0;

	if (adapter->recvpriv.is_signal_dbg) {
		/* update the user specific value, signal_strength_dbg, to signal_strength, rssi */
		adapter->recvpriv.signal_strength = adapter->recvpriv.signal_strength_dbg;
		adapter->recvpriv.rssi = (s8)translate_percentage_to_dbm((u8)adapter->recvpriv.signal_strength_dbg);
	} else {

		if (recvpriv->signal_strength_data.update_req == 0) { /* update_req is clear, means we got rx */
			avg_signal_strength = recvpriv->signal_strength_data.avg_val;
			num_signal_strength = recvpriv->signal_strength_data.total_num;
			/* after avg_vals are accquired, we can re-stat the signal values */
			recvpriv->signal_strength_data.update_req = 1;
		}

		if (recvpriv->signal_qual_data.update_req == 0) { /* update_req is clear, means we got rx */
			avg_signal_qual = recvpriv->signal_qual_data.avg_val;
			num_signal_qual = recvpriv->signal_qual_data.total_num;
			/* after avg_vals are accquired, we can re-stat the signal values */
			recvpriv->signal_qual_data.update_req = 1;
		}

		if (num_signal_strength == 0) {
			if (rtw_get_on_cur_ch_time(adapter) == 0
			    || rtw_get_passing_time_ms(rtw_get_on_cur_ch_time(adapter)) < 2 * adapter->mlmeextpriv.mlmext_info.bcn_interval
			   )
				goto set_timer;
		}

		if (check_fwstate(&adapter->mlmepriv, WIFI_UNDER_SURVEY) == _TRUE
		    || check_fwstate(&adapter->mlmepriv, WIFI_ASOC_STATE) == _FALSE
		   )
			goto set_timer;

#ifdef CONFIG_CONCURRENT_MODE
		if (rtw_mi_buddy_check_fwstate(adapter, WIFI_UNDER_SURVEY) == _TRUE)
			goto set_timer;
#endif

		if (RTW_SIGNAL_STATE_CALC_PROFILE < SIGNAL_STAT_CALC_PROFILE_MAX)
			ratio_profile = RTW_SIGNAL_STATE_CALC_PROFILE;

		ratio_pre_stat = signal_stat_calc_profile[ratio_profile][0];
		ratio_curr_stat = signal_stat_calc_profile[ratio_profile][1];
		ratio_total = ratio_pre_stat + ratio_curr_stat;

		/* update value of signal_strength, rssi, signal_qual */
		tmp_s = (ratio_curr_stat * avg_signal_strength + ratio_pre_stat * recvpriv->signal_strength);
		if (tmp_s % ratio_total)
			tmp_s = tmp_s / ratio_total + 1;
		else
			tmp_s = tmp_s / ratio_total;
		if (tmp_s > 100)
			tmp_s = 100;

		tmp_q = (ratio_curr_stat * avg_signal_qual + ratio_pre_stat * recvpriv->signal_qual);
		if (tmp_q % ratio_total)
			tmp_q = tmp_q / ratio_total + 1;
		else
			tmp_q = tmp_q / ratio_total;
		if (tmp_q > 100)
			tmp_q = 100;

		recvpriv->signal_strength = tmp_s;
		recvpriv->rssi = (s8)translate_percentage_to_dbm(tmp_s);
		recvpriv->signal_qual = tmp_q;

#if defined(DBG_RX_SIGNAL_DISPLAY_PROCESSING) && 1
		RTW_INFO(FUNC_ADPT_FMT" signal_strength:%3u, rssi:%3d, signal_qual:%3u"
			 ", num_signal_strength:%u, num_signal_qual:%u"
			 ", on_cur_ch_ms:%d"
			 "\n"
			 , FUNC_ADPT_ARG(adapter)
			 , recvpriv->signal_strength
			 , recvpriv->rssi
			 , recvpriv->signal_qual
			 , num_signal_strength, num_signal_qual
			, rtw_get_on_cur_ch_time(adapter) ? rtw_get_passing_time_ms(rtw_get_on_cur_ch_time(adapter)) : 0
			);
#endif
	}

set_timer:
	rtw_set_signal_stat_timer(recvpriv);

}
#endif /* CONFIG_NEW_SIGNAL_STAT_PROCESS */

static void rx_process_rssi(_adapter *padapter, union recv_frame *prframe)
{
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	struct signal_stat *signal_stat = &padapter->recvpriv.signal_strength_data;
#else /* CONFIG_NEW_SIGNAL_STAT_PROCESS */
	u32 last_rssi, tmp_val;
#endif /* CONFIG_NEW_SIGNAL_STAT_PROCESS */

	/* RTW_INFO("process_rssi=> pattrib->rssil(%d) signal_strength(%d)\n ",pattrib->recv_signal_power,pattrib->signal_strength); */
	/* if(pRfd->Status.bPacketToSelf || pRfd->Status.bPacketBeacon) */
	{
#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
		if (signal_stat->update_req) {
			signal_stat->total_num = 0;
			signal_stat->total_val = 0;
			signal_stat->update_req = 0;
		}

		signal_stat->total_num++;
		signal_stat->total_val  += pattrib->phy_info.signal_strength;
		signal_stat->avg_val = signal_stat->total_val / signal_stat->total_num;
#else /* CONFIG_NEW_SIGNAL_STAT_PROCESS */

		/* adapter->RxStats.RssiCalculateCnt++;	 */ /* For antenna Test */
		if (padapter->recvpriv.signal_strength_data.total_num++ >= PHY_RSSI_SLID_WIN_MAX) {
			padapter->recvpriv.signal_strength_data.total_num = PHY_RSSI_SLID_WIN_MAX;
			last_rssi = padapter->recvpriv.signal_strength_data.elements[padapter->recvpriv.signal_strength_data.index];
			padapter->recvpriv.signal_strength_data.total_val -= last_rssi;
		}
		padapter->recvpriv.signal_strength_data.total_val  += pattrib->phy_info.signal_strength;

		padapter->recvpriv.signal_strength_data.elements[padapter->recvpriv.signal_strength_data.index++] = pattrib->phy_info.signal_strength;
		if (padapter->recvpriv.signal_strength_data.index >= PHY_RSSI_SLID_WIN_MAX)
			padapter->recvpriv.signal_strength_data.index = 0;


		tmp_val = padapter->recvpriv.signal_strength_data.total_val / padapter->recvpriv.signal_strength_data.total_num;

		if (padapter->recvpriv.is_signal_dbg) {
			padapter->recvpriv.signal_strength = padapter->recvpriv.signal_strength_dbg;
			padapter->recvpriv.rssi = (s8)translate_percentage_to_dbm(padapter->recvpriv.signal_strength_dbg);
		} else {
			padapter->recvpriv.signal_strength = tmp_val;
			padapter->recvpriv.rssi = (s8)translate_percentage_to_dbm(tmp_val);
		}

#endif /* CONFIG_NEW_SIGNAL_STAT_PROCESS */
	}
}

static void rx_process_link_qual(_adapter *padapter, union recv_frame *prframe)
{
	struct rx_pkt_attrib *pattrib;
#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	struct signal_stat *signal_stat;
#else /* CONFIG_NEW_SIGNAL_STAT_PROCESS */
	u32 last_evm = 0, tmpVal;
#endif /* CONFIG_NEW_SIGNAL_STAT_PROCESS */

	if (prframe == NULL || padapter == NULL)
		return;

	pattrib = &prframe->u.hdr.attrib;
#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	signal_stat = &padapter->recvpriv.signal_qual_data;
#endif /* CONFIG_NEW_SIGNAL_STAT_PROCESS */

	/* RTW_INFO("process_link_qual=> pattrib->signal_qual(%d)\n ",pattrib->signal_qual); */

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	if (signal_stat->update_req) {
		signal_stat->total_num = 0;
		signal_stat->total_val = 0;
		signal_stat->update_req = 0;
	}

	signal_stat->total_num++;
	signal_stat->total_val  += pattrib->phy_info.signal_quality;
	signal_stat->avg_val = signal_stat->total_val / signal_stat->total_num;

#else /* CONFIG_NEW_SIGNAL_STAT_PROCESS */
	if (pattrib->phy_info.signal_quality != 0) {
		/*  */
		/* 1. Record the general EVM to the sliding window. */
		/*  */
		if (padapter->recvpriv.signal_qual_data.total_num++ >= PHY_LINKQUALITY_SLID_WIN_MAX) {
			padapter->recvpriv.signal_qual_data.total_num = PHY_LINKQUALITY_SLID_WIN_MAX;
			last_evm = padapter->recvpriv.signal_qual_data.elements[padapter->recvpriv.signal_qual_data.index];
			padapter->recvpriv.signal_qual_data.total_val -= last_evm;
		}
		padapter->recvpriv.signal_qual_data.total_val += pattrib->phy_info.signal_quality;

		padapter->recvpriv.signal_qual_data.elements[padapter->recvpriv.signal_qual_data.index++] = pattrib->phy_info.signal_quality;
		if (padapter->recvpriv.signal_qual_data.index >= PHY_LINKQUALITY_SLID_WIN_MAX)
			padapter->recvpriv.signal_qual_data.index = 0;


		/* <1> Showed on UI for user, in percentage. */
		tmpVal = padapter->recvpriv.signal_qual_data.total_val / padapter->recvpriv.signal_qual_data.total_num;
		padapter->recvpriv.signal_qual = (u8)tmpVal;

	}
#endif /* CONFIG_NEW_SIGNAL_STAT_PROCESS */
}

void rx_process_phy_info(_adapter *padapter, union recv_frame *rframe)
{
	/* Check RSSI */
	rx_process_rssi(padapter, rframe);

	/* Check PWDB */
	/* process_PWDB(padapter, rframe); */

	/* UpdateRxSignalStatistics8192C(adapter, pRfd); */

	/* Check EVM */
	rx_process_link_qual(padapter, rframe);
	rtw_store_phy_info(padapter, rframe);
}

void rx_query_phy_status(
	union recv_frame	*precvframe,
	u8 *pphy_status)
{
	_adapter *padapter = precvframe->u.hdr.adapter;
	struct rx_pkt_attrib	*pattrib = &precvframe->u.hdr.attrib;
	struct phydm_phyinfo_struct *p_phy_info = &pattrib->phy_info;
	u8					*wlanhdr;
	struct phydm_perpkt_info_struct pkt_info;
	u8 *ta, *ra;
	u8 is_ra_bmc;
	struct sta_priv *pstapriv;
	struct sta_info *psta = NULL;
	struct recv_priv  *precvpriv = &padapter->recvpriv;

	pkt_info.is_packet_match_bssid = _FALSE;
	pkt_info.is_packet_to_self = _FALSE;
	pkt_info.is_packet_beacon = _FALSE;
	pkt_info.ppdu_cnt = pattrib->ppdu_cnt;
	pkt_info.station_id = 0xFF;

	wlanhdr = get_recvframe_data(precvframe);

	ta = get_ta(wlanhdr);
	ra = get_ra(wlanhdr);
	is_ra_bmc = IS_MCAST(ra);

	if (_rtw_memcmp(adapter_mac_addr(padapter), ta, ETH_ALEN) == _TRUE) {
		static systime start_time = 0;

#if 0 /*For debug */
		if (IsFrameTypeCtrl(wlanhdr)) {
			RTW_INFO("-->Control frame: Y\n");
			RTW_INFO("-->pkt_len: %d\n", pattrib->pkt_len);
			RTW_INFO("-->Sub Type = 0x%X\n", get_frame_sub_type(wlanhdr));
		}

		/* Dump first 40 bytes of header */
		int i = 0;

		for (i = 0; i < 40; i++)
			RTW_INFO("%d: %X\n", i, *((u8 *)wlanhdr + i));

		RTW_INFO("\n");
#endif

		if ((start_time == 0) || (rtw_get_passing_time_ms(start_time) > 5000)) {
			RTW_PRINT("Warning!!! %s: Confilc mac addr!!\n", __func__);
			start_time = rtw_get_current_time();
		}
		precvpriv->dbg_rx_conflic_mac_addr_cnt++;
	} else {
		pstapriv = &padapter->stapriv;
		psta = rtw_get_stainfo(pstapriv, ta);
		if (psta)
			pkt_info.station_id = psta->phl_sta->macid;
	}

	pkt_info.is_packet_match_bssid = (!IsFrameTypeCtrl(wlanhdr))
		&& (!pattrib->icv_err) && (!pattrib->crc_err)
		&& ((!MLME_IS_MESH(padapter) && _rtw_memcmp(get_hdr_bssid(wlanhdr), get_bssid(&padapter->mlmepriv), ETH_ALEN))
			|| (MLME_IS_MESH(padapter) && psta));

	pkt_info.is_to_self = (!pattrib->icv_err) && (!pattrib->crc_err)
		&& _rtw_memcmp(ra, adapter_mac_addr(padapter), ETH_ALEN);

	pkt_info.is_packet_to_self = pkt_info.is_packet_match_bssid
		&& _rtw_memcmp(ra, adapter_mac_addr(padapter), ETH_ALEN);

	pkt_info.is_packet_beacon = pkt_info.is_packet_match_bssid
				 && (get_frame_sub_type(wlanhdr) == WIFI_BEACON);

	if (psta && IsFrameTypeData(wlanhdr)) {
		if (is_ra_bmc)
			psta->curr_rx_rate_bmc = pattrib->data_rate;
		else
			psta->curr_rx_rate = pattrib->data_rate;
	}
	pkt_info.data_rate = pattrib->data_rate;

	rtw_hal_query_phy_status(padapter, p_phy_info, pphy_status, &pkt_info);

	/* If bw is initial value, get from phy status */
	if (pattrib->bw == CHANNEL_WIDTH_MAX)
		pattrib->bw = p_phy_info->band_width;

	if (p_phy_info->physts_rpt_valid == _TRUE) {
		precvframe->u.hdr.psta = NULL;
		if (padapter->registrypriv.mp_mode != 1) {
			if ((!MLME_IS_MESH(padapter) && pkt_info.is_packet_match_bssid)
				|| (MLME_IS_MESH(padapter) && psta)) {
				if (psta) {
					precvframe->u.hdr.psta = psta;
					rx_process_phy_info(padapter, precvframe);
				}
			} else if (pkt_info.is_packet_to_self || pkt_info.is_packet_beacon) {
				if (psta)
					precvframe->u.hdr.psta = psta;
				rx_process_phy_info(padapter, precvframe);
			}
		} else {
#ifdef CONFIG_MP_INCLUDED
			if (padapter->mppriv.brx_filter_beacon == _TRUE) {
				if (pkt_info.is_packet_beacon) {
					RTW_INFO("in MP Rx is_packet_beacon\n");
					if (psta)
						precvframe->u.hdr.psta = psta;
					rx_process_phy_info(padapter, precvframe);
				}
			} else
#endif
			{
					if (psta)
						precvframe->u.hdr.psta = psta;
					rx_process_phy_info(padapter, precvframe);
			}
		}
	}

	rtw_hal_parse_rx_phy_status_chinfo(precvframe, pphy_status);
}
/*
* Increase and check if the continual_no_rx_packet of this @param pmlmepriv is larger than MAX_CONTINUAL_NORXPACKET_COUNT
* @return _TRUE:
* @return _FALSE:
*/
int rtw_inc_and_chk_continual_no_rx_packet(struct sta_info *sta, int tid_index)
{

	int ret = _FALSE;
	int value = ATOMIC_INC_RETURN(&sta->continual_no_rx_packet[tid_index]);

	if (value >= MAX_CONTINUAL_NORXPACKET_COUNT)
		ret = _TRUE;

	return ret;
}

/*
* Set the continual_no_rx_packet of this @param pmlmepriv to 0
*/
void rtw_reset_continual_no_rx_packet(struct sta_info *sta, int tid_index)
{
	ATOMIC_SET(&sta->continual_no_rx_packet[tid_index], 0);
}

u8 adapter_allow_bmc_data_rx(_adapter *adapter)
{
	if (check_fwstate(&adapter->mlmepriv, WIFI_MONITOR_STATE | WIFI_MP_STATE) == _TRUE)
		return 1;

	if (MLME_IS_AP(adapter))
		return 0;

	if (rtw_linked_check(adapter) == _FALSE)
		return 0;

	return 1;
}

s32 pre_recv_entry(union recv_frame *precvframe, u8 *pphy_status)
{
	s32 ret = _SUCCESS;
	u8 *pbuf = precvframe->u.hdr.rx_data;
	u8 *pda = get_ra(pbuf);
	u8 ra_is_bmc = IS_MCAST(pda);
	_adapter *primary_padapter = precvframe->u.hdr.adapter;
#ifdef CONFIG_CONCURRENT_MODE
	_adapter *iface = NULL;

	#ifdef CONFIG_MP_INCLUDED
	if (rtw_mp_mode_check(primary_padapter))
		goto bypass_concurrent_hdl;
	#endif

	if (ra_is_bmc == _FALSE) { /*unicast packets*/
		iface = rtw_get_iface_by_macddr(primary_padapter , pda);
		if (NULL == iface) {
			#ifdef CONFIG_RTW_CFGVENDOR_RANDOM_MAC_OUI
			if (_rtw_memcmp(pda, adapter_pno_mac_addr(primary_padapter),
					ETH_ALEN) != _TRUE)
			#endif
			RTW_INFO("%s [WARN] Cannot find appropriate adapter - mac_addr : "MAC_FMT"\n", __func__, MAC_ARG(pda));
			/*rtw_warn_on(1);*/
		} else {
			precvframe->u.hdr.adapter = iface;
		}
	} else { /* Handle BC/MC Packets*/
		rtw_mi_buddy_clone_bcmc_packet(primary_padapter, precvframe, pphy_status);
	}
#ifdef CONFIG_MP_INCLUDED
bypass_concurrent_hdl:
#endif
#endif /* CONFIG_CONCURRENT_MODE */
	if (primary_padapter->registrypriv.mp_mode != 1) {
		/* skip unnecessary bmc data frame for primary adapter */
		if (ra_is_bmc == _TRUE && GetFrameType(pbuf) == WIFI_DATA_TYPE
			&& !adapter_allow_bmc_data_rx(precvframe->u.hdr.adapter)
		) {
			rtw_free_recvframe(precvframe, &precvframe->u.hdr.adapter->recvpriv.free_recv_queue);
			goto exit;
		}
	}

	if (pphy_status) {
		rx_query_phy_status(precvframe, pphy_status);

#ifdef CONFIG_WIFI_MONITOR
		rx_query_moinfo(&precvframe->u.hdr.attrib, pphy_status);
#endif
	}
	ret = rtw_recv_entry(precvframe);

exit:
	return ret;
}

#ifdef CONFIG_RECV_THREAD_MODE
thread_return rtw_recv_thread(thread_context context)
{
	_adapter *adapter = (_adapter *)context;
	struct recv_priv *recvpriv = &adapter->recvpriv;
	s32 err = _SUCCESS;
#ifdef RTW_RECV_THREAD_HIGH_PRIORITY
#ifdef PLATFORM_LINUX
	struct sched_param param = { .sched_priority = 1 };

	sched_setscheduler(current, SCHED_FIFO, &param);
#endif /* PLATFORM_LINUX */
#endif /*RTW_RECV_THREAD_HIGH_PRIORITY*/
	rtw_thread_enter("RTW_RECV_THREAD");

	RTW_INFO(FUNC_ADPT_FMT" enter\n", FUNC_ADPT_ARG(adapter));

	do {
		err = _rtw_down_sema(&recvpriv->recv_sema);
		if (_FAIL == err) {
			RTW_ERR(FUNC_ADPT_FMT" down recv_sema fail!\n", FUNC_ADPT_ARG(adapter));
			goto exit;
		}

		if (RTW_CANNOT_RUN(adapter_to_dvobj(adapter))) {
			RTW_DBG(FUNC_ADPT_FMT "- bDriverStopped(%s) bSurpriseRemoved(%s)\n",
				FUNC_ADPT_ARG(adapter),
				dev_is_drv_stopped(adapter_to_dvobj(adapter)) ? "True" : "False",
				dev_is_surprise_removed(adapter_to_dvobj(adapter)) ? "True" : "False");
			goto exit;
		}

		err = rtw_intf_recv_hdl(adapter);

		if (err == RTW_RFRAME_UNAVAIL
			|| err == RTW_RFRAME_PKT_UNAVAIL
		) {
			rtw_msleep_os(1);
			_rtw_up_sema(&recvpriv->recv_sema);
		}

		flush_signals_thread();

	} while (err != _FAIL);

exit:

	RTW_INFO(FUNC_ADPT_FMT " Exit\n", FUNC_ADPT_ARG(adapter));

	rtw_thread_wait_stop();

	return 0;
}
#endif /* CONFIG_RECV_THREAD_MODE */

#if DBG_RX_BH_TRACKING
void rx_bh_tk_set_stage(struct recv_priv *recv, u32 s)
{
	recv->rx_bh_stage = s;
}

void rx_bh_tk_set_buf(struct recv_priv *recv, void *buf, void *data, u32 dlen)
{
	if (recv->rx_bh_cbuf)
		recv->rx_bh_lbuf = recv->rx_bh_cbuf;
	recv->rx_bh_cbuf = buf;
	if (buf) {
		recv->rx_bh_cbuf_data = data;
		recv->rx_bh_cbuf_dlen = dlen;
		recv->rx_bh_buf_dq_cnt++;
	} else {
		recv->rx_bh_cbuf_data = NULL;
		recv->rx_bh_cbuf_dlen = 0;
	}
}

void rx_bh_tk_set_buf_pos(struct recv_priv *recv, void *pos)
{
	if (recv->rx_bh_cbuf) {
		recv->rx_bh_cbuf_pos = pos - recv->rx_bh_cbuf_data;
	} else {
		rtw_warn_on(1);
		recv->rx_bh_cbuf_pos = 0;
	}
}

void rx_bh_tk_set_frame(struct recv_priv *recv, void *frame)
{
	recv->rx_bh_cframe = frame;
}

void dump_rx_bh_tk(void *sel, struct recv_priv *recv)
{
	RTW_PRINT_SEL(sel, "[RXBHTK]s:%u, buf_dqc:%u, lbuf:%p, cbuf:%p, dlen:%u, pos:%u, cframe:%p\n"
		, recv->rx_bh_stage
		, recv->rx_bh_buf_dq_cnt
		, recv->rx_bh_lbuf
		, recv->rx_bh_cbuf
		, recv->rx_bh_cbuf_dlen
		, recv->rx_bh_cbuf_pos
		, recv->rx_bh_cframe
	);
}
#endif /* DBG_RX_BH_TRACKING */

u8 rtw_init_lite_recv_resource(struct dvobj_priv *dvobj)
{

	/* Elwin_todo need use correct literecvbuf_nr recvurb_nr */
	u8 ret = _SUCCESS;
	u32 literecvbuf_nr = RTW_LITERECVBUF_NR;
	struct lite_data_buf *literecvbuf;
	struct trx_data_buf_q  *literecvbuf_q = &dvobj->literecvbuf_q;
	int i;
#ifdef CONFIG_USB_HCI
	struct data_urb *recvurb;
	struct trx_urb_buf_q *recv_urb_q = &dvobj->recv_urb_q;
	u32 recvurb_nr = RTW_RECVURB_NR;
#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	struct lite_data_buf *intinbuf;
	struct trx_data_buf_q  *intin_buf_q = &dvobj->intin_buf_q;
	u32 intin_buf_nr = RTW_INTINBUF_NR;
	struct data_urb *intin_urb;
	struct trx_urb_buf_q *intin_urb_q = &dvobj->intin_urb_q;
	u32 intin_urb_nr = RTW_INTINURB_NR;
#endif
#endif

	/* init lite_recv_buf */
	_rtw_init_queue(&literecvbuf_q->free_data_buf_queue);

	literecvbuf_q->alloc_data_buf =
		rtw_zvmalloc(literecvbuf_nr * sizeof(struct lite_data_buf) + 4);

	if (literecvbuf_q->alloc_data_buf  == NULL) {
		ret = _FAIL;
		goto exit;
	}

	literecvbuf_q->data_buf=
	(u8 *)N_BYTE_ALIGNMENT((SIZE_PTR)(literecvbuf_q->alloc_data_buf), 4);

	literecvbuf = (struct lite_data_buf *)literecvbuf_q->data_buf;

	for (i = 0; i < literecvbuf_nr; i++) {
		_rtw_init_listhead(&literecvbuf->list);
		rtw_list_insert_tail(&literecvbuf->list,
			&(literecvbuf_q->free_data_buf_queue.queue));
		literecvbuf++;
	}
	literecvbuf_q->free_data_buf_cnt = literecvbuf_nr;


#ifdef CONFIG_USB_HCI
	/* init recv_urb */
	_rtw_init_queue(&recv_urb_q->free_urb_buf_queue);
	recv_urb_q->alloc_urb_buf=
		rtw_zvmalloc(recvurb_nr * sizeof(struct data_urb) + 4);
	if (recv_urb_q->alloc_urb_buf== NULL) {
		ret = _FAIL;
		goto exit;
	}

	recv_urb_q->urb_buf =
		(u8 *)N_BYTE_ALIGNMENT((SIZE_PTR)(recv_urb_q->alloc_urb_buf), 4);

	recvurb = (struct data_urb *)recv_urb_q->urb_buf;
	for (i = 0; i < recvurb_nr; i++) {
		_rtw_init_listhead(&recvurb->list);
		ret = rtw_os_urb_resource_alloc(recvurb);
		rtw_list_insert_tail(&recvurb->list,
			&(recv_urb_q->free_urb_buf_queue.queue));
		recvurb++;
	}
	recv_urb_q->free_urb_buf_cnt = recvurb_nr;

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE

	/* init int_in_buf */
	_rtw_init_queue(&intin_buf_q->free_data_buf_queue);

	intin_buf_q->alloc_data_buf =
		rtw_zvmalloc(intin_buf_nr * sizeof(struct lite_data_buf) + 4);

	if (intin_buf_q->alloc_data_buf  == NULL) {
		ret = _FAIL;
		goto exit;
	}

	intin_buf_q->data_buf=
	(u8 *)N_BYTE_ALIGNMENT((SIZE_PTR)(intin_buf_q->alloc_data_buf), 4);

	intinbuf = (struct lite_data_buf *)intin_buf_q->data_buf;

	for (i = 0; i < intin_buf_nr; i++) {
		_rtw_init_listhead(&intinbuf->list);
		rtw_list_insert_tail(&intinbuf->list,
			&(intin_buf_q->free_data_buf_queue.queue));
		intinbuf++;
	}
	intin_buf_q->free_data_buf_cnt = intin_buf_nr;

	/* init int_in_urb */
	_rtw_init_queue(&intin_urb_q->free_urb_buf_queue);
	intin_urb_q->alloc_urb_buf=
		rtw_zvmalloc(intin_urb_nr * sizeof(struct data_urb) + 4);
	if (intin_urb_q->alloc_urb_buf== NULL) {
		ret = _FAIL;
		goto exit;
	}

	intin_urb_q->urb_buf =
		(u8 *)N_BYTE_ALIGNMENT((SIZE_PTR)(intin_urb_q->alloc_urb_buf), 4);

	intin_urb = (struct data_urb *)intin_urb_q->urb_buf;
	for (i = 0; i < intin_urb_nr; i++) {
		_rtw_init_listhead(&intin_urb->list);
		ret = rtw_os_urb_resource_alloc(intin_urb);
		rtw_list_insert_tail(&intin_urb->list,
			&(intin_urb_q->free_urb_buf_queue.queue));
		intin_urb++;
	}
	intin_urb_q->free_urb_buf_cnt = intin_urb_nr;
#endif
#endif

exit:
	return ret;
}

void rtw_free_lite_recv_resource(struct dvobj_priv *dvobj)
{
	u8 ret = _SUCCESS;
	u32 literecvbuf_nr = RTW_LITERECVBUF_NR;
	struct lite_data_buf *literecvbuf;
	struct trx_data_buf_q  *literecvbuf_q = &dvobj->literecvbuf_q;
	int i;
#ifdef CONFIG_USB_HCI
	struct data_urb *recvurb;
	struct trx_urb_buf_q *recv_urb_q = &dvobj->recv_urb_q;
	u32 recvurb_nr = RTW_RECVURB_NR;
#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	struct lite_data_buf *intinbuf;
	struct trx_data_buf_q *intin_buf_q = &dvobj->intin_buf_q;
	u32 intin_buf_nr = RTW_INTINBUF_NR;
	struct data_urb *intin_urb;
	struct trx_urb_buf_q *intin_urb_q = &dvobj->intin_urb_q;
	u32 intin_urb_nr = RTW_INTINURB_NR;
#endif
#endif

	if (literecvbuf_q->alloc_data_buf)
		rtw_vmfree(literecvbuf_q->alloc_data_buf,
			literecvbuf_nr * sizeof(struct lite_data_buf) + 4);

#ifdef CONFIG_USB_HCI
	recvurb = (struct data_urb *)recv_urb_q->urb_buf;
	for (i = 0; i < recvurb_nr; i++) {
		rtw_os_urb_resource_free(recvurb);
		recvurb++;
	}

	if (recv_urb_q->alloc_urb_buf)
		rtw_vmfree(recv_urb_q->alloc_urb_buf,
			recvurb_nr * sizeof(struct data_urb) + 4);

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE

	if (intin_buf_q->alloc_data_buf)
		rtw_vmfree(intin_buf_q->alloc_data_buf,
			intin_buf_nr * sizeof(struct lite_data_buf) + 4);

	intin_urb = (struct data_urb *)intin_urb_q->urb_buf;
	for (i = 0; i < intin_urb_nr; i++) {
		rtw_os_urb_resource_free(intin_urb);
		intin_urb++;
	}

	if (intin_urb_q->alloc_urb_buf)
		rtw_vmfree(intin_urb_q->alloc_urb_buf,
			intin_urb_nr * sizeof(struct data_urb) + 4);
#endif
#endif

}

#ifdef RTW_PHL_RX
void rx_dump_skb(struct sk_buff *skb)
{
	int idx=0;
	u8 *tmp=skb->data;
	printk("===");
	printk("[%s]skb=%p len=%d\n", __FUNCTION__, skb, skb->len);

#if 0
	printk("data-tail=0x%x-0x%x(%d)\n",
		skb->data, skb->tail, (skb->tail - skb->data));
	printk("head-end=0x%x-0x%x(%d)\n",
		skb->head, skb->end, (skb->end - skb->head));
#endif

	for(idx=0; idx<skb->len; idx++){
		printk("%02x ", tmp[idx]);
		if(idx%20==19)
			printk("\n");
	}
	printk("\n===\n");
}

void dump_rxreq(_adapter *adapter, union recv_frame *prframe)
{


}

void dump_recv_frame(_adapter *adapter, union recv_frame *prframe)
{
	struct recv_frame_hdr *hdr = &(prframe->u.hdr);
	struct rx_pkt_attrib *rxattr = &(prframe->u.hdr.attrib);

	printk("[%s]prframe=0x%p len=%d\n", __FUNCTION__, prframe, hdr->len);

	printk("head-tail=0x%p-0x%p\n", hdr->rx_head, hdr->rx_tail);
	printk("data-end=0x%p-0x%p\n", hdr->rx_data, hdr->rx_end);

	printk("dst=%pM\n", rxattr->dst);
	printk("src=%pM\n", rxattr->src);
	printk("ra=%pM\n", rxattr->ra);
	printk("ta=%pM\n", rxattr->ta);
	printk("bssid=%pM\n", rxattr->bssid);
}

void core_update_recvframe_pkt(union recv_frame *prframe, void *req)
{
	struct rtw_recv_pkt *rx_req = (struct rtw_recv_pkt *)req;
	struct sk_buff *skb = prframe->u.hdr.pkt;
	#ifndef CONFIG_RTW_HW_RX_AMSDU_CUT
	struct rtw_pkt_buf_list *pkt = rx_req->pkt_list;
	u8 *frame = pkt->vir_addr;
	u32 frame_len = pkt->length;
	u8 *wlan_hdr = frame;
	#else
	struct rtw_rx_buf_base *rx_buf = rx_req->rx_buf;
	u8 *frame = rx_buf->frame;
	u32 frame_len = rx_buf->frame_len;
	u8 *wlan_hdr = rx_buf->hdr;

	if (   (!rx_req->mdata.hdr_conv && (frame != wlan_hdr))
	    || (rx_req->mdata.hdr_conv && (frame == wlan_hdr)))
		RTW_ERR("Wlan header error! (%u, %pX, %pX)\n",
		        rx_req->mdata.hdr_conv, frame, wlan_hdr);
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	skb_reserve(skb, frame - skb->data);
	skb_put(skb, frame_len);

	prframe->u.hdr.rx_data = skb->data;
	prframe->u.hdr.rx_tail = skb_tail_pointer(skb);
	prframe->u.hdr.len = skb->len;
	prframe->u.hdr.rx_head = skb->head;
	prframe->u.hdr.rx_end = skb_end_pointer(skb);
	prframe->u.hdr.wlan_hdr = wlan_hdr;
	prframe->u.hdr.mdata = (u8 *)&rx_req->mdata;
	prframe->u.hdr.rx_req = req;
}

static int core_alloc_recvframe_pkt(union recv_frame *prframe,
				    struct rtw_recv_pkt *phlrx)
{
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	struct rtw_rx_buf_base *rx_buf;
	#else
	struct rtw_pkt_buf_list *pktbuf;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	u8 *frame;
	u32 frame_len;
	u8 *wlan_hdr;
	u8 shift_sz;
	u32 alloc_sz;
	struct sk_buff *pkt = NULL;
	u8 *pbuf = NULL;

	#ifndef CONFIG_RTW_HW_RX_AMSDU_CUT
	rtw_warn_on(phlrx->pkt_cnt > 1);
	#endif

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	rx_buf = phlrx->rx_buf;
	frame = rx_buf->frame;
	frame_len = rx_buf->frame_len;
	wlan_hdr = rx_buf->hdr;

	if ((!phlrx->mdata.hdr_conv && (frame != wlan_hdr)) ||
	    (phlrx->mdata.hdr_conv && (frame == wlan_hdr)))
		RTW_ERR("WLAN header error: (%u, %pX, %pX)\n",
			phlrx->mdata.hdr_conv, frame, wlan_hdr);
	#else
	pktbuf = phlrx->pkt_list; /* &phlrx->pkt_list[0] */
	frame = pktbuf->vir_addr;
	frame_len = pktbuf->length;
	wlan_hdr = frame;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	/* For 8 bytes IP header alignment. */
	if (phlrx->mdata.qos)
		/* Qos data, wireless lan header length is 26 */
		shift_sz = 6;
	else
		shift_sz = 0;

	/*
	 * For first fragment packet, driver need allocate 1536 to
	 * defrag packet.
	 * And need 8 is for skb->data 8 bytes alignment.
	 * Round (1536 + shift_sz + 8) to 128 bytes alignment,
	 * and finally get 1664.
	 */

	alloc_sz = frame_len;
	if ((phlrx->mdata.more_frag == 1) && (phlrx->mdata.frag_num == 0)) {
		if (alloc_sz <= 1650)
			alloc_sz = 1664;
		else
			alloc_sz += 14;
	} else {
		/*
		 * 6 is for IP header 8 bytes alignment in QoS packet case.
		 * 8 is for skb->data 4 bytes alignment.
		 */
		alloc_sz += 14;
	}

	pkt = rtw_skb_alloc(alloc_sz);
	if (!pkt) {
		RTW_ERR("%s: alloc skb fail! sz=%u (mfrag=%u, frag_num=%u)\n",
			__FUNCTION__, alloc_sz, phlrx->mdata.more_frag,
			phlrx->mdata.frag_num);
		return -1;
	}

	/* force pkt->data at 8-byte alignment address */
	skb_reserve(pkt, 8 - ((SIZE_PTR)pkt->data & 7));
	/* force ip_hdr at 8-byte alignment address according to shift_sz. */
	skb_reserve(pkt, shift_sz);
	pbuf = skb_put(pkt, frame_len);
	_rtw_memcpy(pbuf, frame, frame_len);

	prframe->u.hdr.pkt = pkt;
	prframe->u.hdr.rx_data = pkt->data;
	prframe->u.hdr.rx_tail = skb_tail_pointer(pkt);
	prframe->u.hdr.len = pkt->len;
	prframe->u.hdr.rx_head = pkt->head;
	prframe->u.hdr.rx_end = skb_end_pointer(pkt);
	prframe->u.hdr.wlan_hdr = wlan_hdr;
	prframe->u.hdr.mdata = (u8 *)&phlrx->mdata;
	prframe->u.hdr.rx_req = phlrx;

	return 0;
}

void core_update_recvframe_mdata(struct dvobj_priv *dvobj, union recv_frame *prframe, struct rtw_recv_pkt *rx_req)
{
	struct rx_pkt_attrib *prxattrib = &prframe->u.hdr.attrib;
	struct rtw_r_meta_data *mdata = &rx_req->mdata;
	void *phl =dvobj->phl;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *pbuf = prframe->u.hdr.wlan_hdr;
	#else
	u8 *pbuf = prframe->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	u8 *mac = get_ta(pbuf);

#ifdef CONFIG_SNR_RPT
	struct rtw_phl_ppdu_phy_info *phy_info = &rx_req->phy_info;
	int i;
#endif
#ifdef TX_BEAMFORMING
	if (rtw_phl_read8(phl, 0xce30) & BIT(0)){ //ensure zero length delimiter count enable
	if ((mdata->rpkt_type == 0) && ((mdata->freerun_cnt >> 24) & 0xff))
			prxattrib->is_zld_exist = true;
	else
			prxattrib->is_zld_exist = false;
	}
#endif

	prxattrib->bdecrypted = !(mdata->sw_dec);
	prxattrib->pkt_len = mdata->pktlen;
	prxattrib->icv_err = mdata->icverr;
	prxattrib->crc_err = mdata->crc32;
	prxattrib->bw = mdata->bw;
	prxattrib->data_rate = mdata->rx_rate;
	prxattrib->sgi = mdata->rx_gi_ltf;
	/* HW supports Rx Drvinfo(PhyRpt) in RxDesc */
	if (RTW_DEV_CAP_ENABLE == dvobj->phl_com->dev_cap.drv_info_sup)
		prxattrib->phy_info.signal_strength = rx_req->phy_info.signal_strength;
	else
		prxattrib->phy_info.signal_strength = rx_req->phy_info.rssi;
#if defined(DEBUG_MAP_UNASSOC) || defined(MONITOR_UNASSOC_STA)
	prxattrib->rssi = prxattrib->phy_info.signal_strength;
#endif
#if 0 //todo
//Security (sw-decrypt & calculate payload offset)
	u8	bdecrypted;
	u8	encrypt; /* when 0 indicate no encrypt. when non-zero, indicate the encrypt algorith */
	u8	iv_len;
	u8	icv_len;
	u8	crc_err;
	u8	icv_err;
#endif


#ifdef CONFIG_SNR_RPT
	if (phy_info->is_valid) {
		prxattrib->phy_info.is_valid = true;
		/* snr info */
		prxattrib->phy_info.snr_fd_avg = phy_info->snr_fd_avg;
		prxattrib->phy_info.snr_td_avg = phy_info->snr_td_avg;
		for ( i = 0; i < RTW_PHL_MAX_RF_PATH; i++) {
			prxattrib->phy_info.snr_fd[i] = phy_info->snr_fd[i];
			prxattrib->phy_info.snr_td[i] = phy_info->snr_td[i];
		}
	} else {
		prxattrib->phy_info.is_valid = false;
		prxattrib->phy_info.snr_fd_avg = 0;
		prxattrib->phy_info.snr_td_avg = 0;
		for (i = 0; i < RTW_PHL_MAX_RF_PATH; i++) {
			prxattrib->phy_info.snr_fd[i] = 0;
			prxattrib->phy_info.snr_td[i] = 0;
		}
	}
#endif

	return;
}


s32 core_rx_process_amsdu(_adapter *adapter, union recv_frame *prframe)
{
	u32 ret;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	struct rtw_r_meta_data *mdata = (struct rtw_r_meta_data *)prframe->u.hdr.mdata;

	if (mdata->debug_dump) {
		RTW_PRINT("Cut %u\n", mdata->amsdu_cut);
	}
	if (mdata->amsdu_cut)
		ret = _amsdu_lst_to_msdu(adapter, prframe);
	else
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
		ret = amsdu_to_msdu(adapter, prframe);

	return (ret == _SUCCESS) ? CORE_RX_DONE : CORE_RX_DROP;
}

s32 core_rx_process_msdu(_adapter *adapter, union recv_frame *prframe)
{
#if defined(CONFIG_RTW_A4_STA) || defined(RTW_CORE_PKT_TRACE)
	struct rx_pkt_attrib *rattrib = &prframe->u.hdr.attrib;
#endif
#ifdef CONFIG_RTW_A4_STA
	int act = RTW_RX_MSDU_ACT_INDICATE;
#endif
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	struct rtw_r_meta_data *mdata = (struct rtw_r_meta_data *)prframe->u.hdr.mdata;

	if (mdata->hdr_conv)
		adapter->rx_logs.core_rx_hdr_conv++;
#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

#ifdef CONFIG_RTW_A4_STA
/*
	if (MLME_IS_AP(padapter))
		act = rtw_ap_rx_msdu_act_check(prframe, pattrib->dst, pattrib->src
			, msdu, llc_hdl, &fwd_frame, &b2u_list);
*/
	if (3 == rattrib->to_fr_ds) {
		if (MLME_IS_AP(adapter))
			act = core_a4_rx_msdu_act_check(prframe);
		else if(MLME_IS_STA(adapter))
			act = core_a4_rx_amsdu_act_check(adapter, rattrib->dst, rattrib->src);

		/* A4_CNT */
		if (act != RTW_RX_MSDU_ACT_NONE) {
			core_a4_count_stats(adapter,
				prframe->u.hdr.attrib.src, 1,
				prframe->u.hdr.pkt->len);
		}
	}

	if (RTW_RX_MSDU_ACT_NONE == act)
		return CORE_RX_DROP;
	else if (RTW_RX_MSDU_ACT_FORWARD == act) {
		prframe->u.hdr.pkt = NULL;
		return CORE_RX_DROP;
	}
#endif

#ifdef CONFIG_RTW_MESH

#endif

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	if (mdata->hdr_conv) {
		u16 eth_type;
		u32 offset;
		u8 *frame = prframe->u.hdr.rx_data;

		if (memcmp(frame + ETH_ALEN * 2 + 2, rtw_rfc1042_header, SNAP_SIZE) == 0) {
			offset = 14 + SNAP_SIZE;
		} else {
			offset = 12;
		}

		_rtw_memcpy(&eth_type, frame + offset, 2);

		if (eth_type == ntohs(ETH_P_8021Q))
			_rtw_memcpy(&eth_type, frame + offset + VLAN_TAG_LEN, 2);

		eth_type = ntohs((unsigned short)eth_type);
		rattrib->eth_type = eth_type;
		/* IV is appended if in security mode. Remove it. */
		if (rattrib->icv_len)
			recvframe_pull_tail(prframe, rattrib->icv_len);
		rtw_rframe_set_os_pkt(prframe);
	} else
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	if (wlanhdr_to_ethhdr(prframe) != _SUCCESS) {
		#ifdef RTW_CORE_PKT_TRACE
		if(adapter->pkt_trace_enable)
			RTW_RX_TRACE(adapter,&rattrib->pktinfo);
		#endif
		return CORE_RX_DROP;
	}

	if (rtw_recv_indicatepkt_check(prframe,
		get_recvframe_data(prframe), get_recvframe_len(prframe)) != _SUCCESS) {
#ifdef RTW_CORE_PKT_TRACE
		if(adapter->pkt_trace_enable)
			RTW_RX_TRACE(adapter,&rattrib->pktinfo);
#endif
		return CORE_RX_DROP;
	}

	if (rtw_recv_indicatepkt(adapter, prframe) != _SUCCESS) {
#ifdef RTW_CORE_PKT_TRACE
		if(adapter->pkt_trace_enable)
			RTW_RX_TRACE(adapter,&rattrib->pktinfo);
#endif
		return CORE_RX_DROP;
	}

	return CORE_RX_DONE;
}


s32 rtw_core_rx_data_post_process(_adapter *adapter, union recv_frame *prframe)
{
	s32 status = CORE_RX_DONE;

	//amsdu
	//make eth hdr
	//forward

	//recv_process_mpdu
	  //amsdu_to_msdu
	  //wlanhdr_to_ethhdr
	  //rtw_recv_indicatepkt_check
	  //rtw_recv_indicatepkt

#ifdef SBWC
	int ret = sbwc_rx(adapter, prframe);
	if (ret == CORE_RX_ENQUEUED){
		return CORE_RX_DONE;
	}else if(ret == CORE_RX_DROP){
		return CORE_RX_DROP;
	}
#endif

#ifdef GBWC
	status = gbwc_rx(adapter, prframe);
	if (status == CORE_RX_ENQUEUED){
		return CORE_RX_DONE;
	}else if(status == CORE_RX_DROP){
		return CORE_RX_DROP;
	}
#endif

	if (prframe->u.hdr.attrib.amsdu)
		status = core_rx_process_amsdu(adapter, prframe);
	else
		status = core_rx_process_msdu(adapter, prframe);
#ifdef CONFIG_RTW_CORE_RXSC
	if (prframe->u.hdr.rxsc_entry) {
		if (CORE_RX_DONE != status)
			prframe->u.hdr.rxsc_entry->status = RXSC_ENTRY_INVALID;
		else
			prframe->u.hdr.rxsc_entry->status = RXSC_ENTRY_VALID;
	}
#endif

	return status;
}

s32 rtw_core_rx_data_pre_process(_adapter *adapter, union recv_frame **prframe)
{
	//recv_func_posthandle
	  //decryptor
	  //portctrl
	  //count_rx_stats
#ifdef CONFIG_TDLS
#endif
#ifdef DBG_RX_BMC_FRAME
#endif
#ifdef CONFIG_WAPI_SUPPORT
#endif

	union recv_frame * ret_frame = NULL;

	s32 ret = CORE_RX_CONTINUE;

	ret_frame = decryptor(adapter, *prframe);
	if (ret_frame == NULL)
		return CORE_RX_DROP;
	else
		*prframe = ret_frame;

	ret = recvframe_chk_defrag(adapter, prframe);
	if (ret != CORE_RX_CONTINUE) {
		if (ret == CORE_RX_DROP) {
		#ifdef DBG_RX_DROP_FRAME
			RTW_INFO("DBG_RX_DROP_FRAME "FUNC_ADPT_FMT" recvframe_chk_defrag: drop pkt\n"
				, FUNC_ADPT_ARG(adapter));
		#endif
			DBG_COUNTER(adapter->rx_logs.core_rx_post_defrag_err);
		}
		return ret;
	}

	/* Rx TKIP MIC */
	if ((*prframe)->u.hdr.attrib.privacy) {
		if (recvframe_chkmic(adapter,  *prframe) == _FAIL) {
			return CORE_RX_DROP;
		}
	}

	ret_frame = portctrl(adapter, *prframe);
	if (ret_frame == NULL)
		return CORE_RX_DROP;
	else
		*prframe = ret_frame;

	count_rx_stats(adapter, *prframe, NULL);

	return CORE_RX_CONTINUE;
}

#ifdef CONFIG_CONCURRENT_MODE
static s32 _clone_bcmc_packet(struct dvobj_priv *dvobj, union recv_frame *prframe,
                              union recv_frame **prframes)
{
	int			i, dup_num = 0;
	_adapter		*primary_padapter = dvobj_get_primary_adapter(dvobj);
	struct recv_priv	*precvpriv = &primary_padapter->recvpriv;
	_queue			*pfree_recv_queue = &precvpriv->free_recv_queue;
	u8			*fhead = get_recvframe_data(prframe);
	u8			type = GetFrameType(fhead);

	for (i = 0; i < dvobj->iface_nums; i++) {
		struct rx_pkt_attrib	*pattrib;
		union recv_frame	*pcloneframe;
		_adapter		*iface;

		iface = dvobj->padapters[i];
		if (   (iface->registered == 0)
		    || (rtw_is_adapter_up(iface) == _FALSE)
		    || iface->registrypriv.wifi_mib.func_off
		    || (   (type == WIFI_DATA_TYPE)
			&& !adapter_allow_bmc_data_rx(iface)))
			continue;

		/* Set first valid adapter to the original RX frame */
		if (prframe->u.hdr.adapter == NULL) {
			prframe->u.hdr.adapter = iface;
			prframe->u.hdr.pkt->dev = iface->pnetdev;
			continue;
		}

		pcloneframe = rtw_alloc_recvframe(pfree_recv_queue);
		if (pcloneframe == NULL) {
			RTW_WARN(FUNC_ADPT_FMT "No RX frame avaiable", FUNC_ADPT_ARG(iface));
			break;
		}

		pcloneframe->u.hdr.adapter = iface;
		_rtw_init_listhead(&pcloneframe->u.hdr.list);
		pcloneframe->u.hdr.precvbuf = NULL;	/*can't access the precvbuf for new arch.*/
		pcloneframe->u.hdr.len = 0;
		pcloneframe->u.hdr.cloned = 1;

		_rtw_memcpy(&pcloneframe->u.hdr.attrib, &prframe->u.hdr.attrib,
		            sizeof(struct rx_pkt_attrib));

		pattrib = &pcloneframe->u.hdr.attrib;

		#ifdef CONFIG_SKB_ALLOCATED
		if (   rtw_os_alloc_recvframe(iface, pcloneframe, pbuf, NULL)
		    == _SUCCESS)
		#else
		if (    rtw_os_recvframe_duplicate_skb(iface, pcloneframe, prframe->u.hdr.pkt)
		    == _SUCCESS)
		#endif
		{
			//RTW_INFO(FUNC_ADPT_FMT "Dup BMC.\n", FUNC_ADPT_ARG(iface));
			#ifdef CONFIG_SKB_ALLOCATED
			recvframe_put(pcloneframe, pattrib->pkt_len);
			#endif

			#ifdef DBG_SKB_PROCESS
			rtw_dbg_skb_process(adapter, precvframe, pcloneframe);
			#endif
			pcloneframe->u.hdr.rx_req = prframe->u.hdr.rx_req;
			#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
			pcloneframe->u.hdr.wlan_hdr = prframe->u.hdr.wlan_hdr;
			#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
			prframes[dup_num++] = pcloneframe;
		} else {
			RTW_INFO(FUNC_ADPT_FMT "Failed to duplicate BMC.\n", FUNC_ADPT_ARG(iface));
			rtw_free_recvframe(pcloneframe, pfree_recv_queue);
		}
	}

	if (prframe->u.hdr.adapter == NULL) {
		/* RTW_WARN("No valid adapter to RX BMC.\n"); */
		return CORE_RX_DROP;
	}

	/* Always process the original one lastly */
	prframes[dup_num++] = prframe;
	/* NULL terminator */
	if (dup_num < dvobj->iface_nums) {
		prframes[dup_num] = NULL;
	}

	return CORE_RX_CONTINUE;
}
#endif /* CONFIG_CONCURRENT_MODE */

#ifdef CONFIG_WIFI_DIAGNOSIS
static void rtw_wifi_diag_unassoc_sta(_adapter *adapter, union recv_frame *prframe)
{
	_adapter *iface = NULL;
	struct rx_pkt_attrib *prxattrib = &prframe->u.hdr.attrib;
	struct wifi_diag_obj *wifi_diag = &(adapter->dvobj->wifi_diag);
	struct wifi_diag_sta_entry *ent = NULL;
	u8 *pbuf = prframe->u.hdr.rx_data;
	u8 *mac = get_ta(pbuf);
	u8 *bssid = get_ra(pbuf);
	u8  type = get_frame_sub_type(pbuf);
	u8  rssi = prxattrib->phy_info.signal_strength;
	u8  i;
	u8 is_target_sta = 0;

	if (wifi_diag->mode != DIAG_UNASSOC_STA
	    && wifi_diag->mode != DIAG_SPEC_STA)
		return;

	if (!wifi_diag->diag_ongoing)
		return;

	/* only check probereq and sta2ap packet */
	if (type != WIFI_PROBEREQ && get_tofr_ds(pbuf) != 2)
		return;

	iface = rtw_get_iface_by_macddr(adapter, bssid);
	if (type != WIFI_PROBEREQ)
		if (iface && (rtw_get_stainfo(&(iface->stapriv), mac)))
			return;

	if (wifi_diag->mode == DIAG_SPEC_STA) {
		for (i = 0; i < wifi_diag->target_list_num; i++) {
			if (_rtw_memcmp(wifi_diag->target_list[i].mac, mac, ETH_ALEN) == _TRUE) {
				is_target_sta = 1;
				break;
			}
		}
		if(!is_target_sta)
			return;
	}

	for (i = 0; i < wifi_diag->sta_list_num; i++) {
		ent = &(wifi_diag->sta_list[i]);
		/* already in the list */
		if (_rtw_memcmp(ent->mac, mac, ETH_ALEN)) {
			break;
		}
	}

	if (i < wifi_diag->sta_list_num) {
		u8 is_bcast_1 = is_broadcast_mac_addr(ent->bssid);
		u8 is_bcast_2 = is_broadcast_mac_addr(bssid);

		if (rssi != 0 && ent->rssi == 0)
			ent->rssi = rssi;
		else if (rssi != 0 && ent->rssi != 0)
			ent->rssi = (ent->rssi * 7 + rssi * 3) / 10;

		if (is_bcast_1 && !is_bcast_2)
			_rtw_memcpy(ent->bssid, bssid, ETH_ALEN);
		ent->time_stamp = jiffies;
	}
	else if (wifi_diag->sta_list_num < WIFI_DIAG_ENTRY_NUM) {
		ent = &(wifi_diag->sta_list[wifi_diag->sta_list_num]);
		ent->channel = rtw_phl_get_cur_ch(adapter->phl_role);
		_rtw_memcpy(ent->mac, mac, ETH_ALEN);
		ent->rssi = rssi;
		_rtw_memcpy(ent->bssid, bssid, ETH_ALEN);
		ent->time_stamp = jiffies;
		wifi_diag->sta_list_num++;
	}

	return;
}
#endif

void _count_icv_error(_adapter * adapter, struct rtw_recv_pkt *phlrx, u8 *whdr)
{
	_adapter *iface = NULL;
	u8 *pda = NULL;
	struct	mlme_priv	*pmlmepriv;
	struct	sta_priv	*pstapriv;
	struct sta_info *psta;

	if (!adapter->registrypriv.wifi_mib.kick_icverr)
		return;

	if ((phlrx->rx_role == NULL)
		|| (phlrx->rx_role->core_data == NULL)) {
		pda = get_ra(whdr);
		iface = rtw_get_iface_by_macddr(adapter, pda);
	} else {
		iface = (_adapter *)phlrx->rx_role->core_data;
	}

	if (!iface)
		return;

	pmlmepriv = &iface->mlmepriv;
	pstapriv = &iface->stapriv;

	if (MLME_IS_AP(iface)) {
		psta = rtw_get_stainfo(pstapriv, get_ta(whdr));
		if (psta) {
			psta->sta_stats.rx_icverr_pkts++;
		}
	}
}

static s32 rtw_core_update_recvframe(struct dvobj_priv *dvobj,
                                     union recv_frame *prframe,
                                     void *rx_req,
                                     union recv_frame **prframes)
{
	struct rtw_recv_pkt *phlrx = (struct rtw_recv_pkt *)rx_req;
	_adapter *iface = NULL;
	u8 is_bmc = _FALSE;
	enum rtw_core_rx_state rx_state = CORE_RX_CONTINUE;
	int err;
	_adapter *primary_padapter = dvobj_get_primary_adapter(dvobj);
#ifdef MONITOR_UNASSOC_STA
	struct registry_priv *pregpriv;
#endif
	u8 *whdr = NULL;

	if (phlrx->mdata.bc || phlrx->mdata.mc)
		is_bmc = _TRUE;

	if (phlrx->os_priv) {
#ifdef CONFIG_RTW_CORE_RXSC
		if (!prframe->u.hdr.pkt)
#endif
		{
			#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
			prframe->u.hdr.pkt = phlrx->rx_buf->os_priv;
			#else
			prframe->u.hdr.pkt = phlrx->os_priv; /*skb*/
			#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
			core_update_recvframe_pkt(prframe, rx_req);
		}
	} else {
		err = core_alloc_recvframe_pkt(prframe, phlrx);
		if (err) {
			rx_state = CORE_RX_FAIL;
			goto exit;
		}
	}

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	whdr = (prframe->u.hdr.wlan_hdr ? prframe->u.hdr.wlan_hdr : prframe->u.hdr.rx_data);
	#else
	whdr = prframe->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	/* If sec_type is RTW_ENC_WAPI, HW compute MIC error and set icverr to 1 when QOS bit 4~6 and 8~15 is not 0.
	   But HW decrypt data is right. In this case, SW set icverr to 0 and does not drop packet. */
	if (phlrx->mdata.icverr &&
	    phlrx->mdata.qos &&
	    phlrx->mdata.hw_dec &&
	    ((phlrx->mdata.sec_type == RTW_ENC_WAPI) ||
	     (phlrx->mdata.sec_type == RTW_ENC_GCMSMS4)) &&
	    rtw_wapi_check_frame_qos(whdr, prframe->u.hdr.len))
		phlrx->mdata.icverr = 0;
#endif

	if (phlrx->mdata.icverr) {
		DBG_COUNTER(primary_padapter->rx_logs.core_rx_icv_err);
		primary_padapter->recvpriv.rx_errors++;
		rx_state = CORE_RX_FAIL;
		_count_icv_error(primary_padapter, phlrx, whdr);
		goto exit;
	}

	if (phlrx->mdata.crc32) {
		DBG_COUNTER(primary_padapter->rx_logs.core_rx_crc_err);
		primary_padapter->recvpriv.rx_errors++;
		rx_state = CORE_RX_FAIL;
		goto exit;
	}
	core_update_recvframe_mdata(dvobj, prframe, rx_req);

#ifdef CONFIG_WIFI_DIAGNOSIS
	rtw_wifi_diag_unassoc_sta(primary_padapter, prframe);
#endif

#ifdef DEBUG_MAP_UNASSOC
	if (core_map_check_state(primary_padapter, MAP_STATE_MEASURE_UNASSOC))
		core_map_update_unassoc_sta_metric(primary_padapter, prframe);
#endif

#ifdef MONITOR_UNASSOC_STA
	pregpriv = &primary_padapter->registrypriv;
	if(pregpriv->wifi_mib.monitor_sta_enabled)
	{
		rtk_monitor_sta_info(primary_padapter, prframe);
	}
#endif

	if (!is_bmc) {
		if (   (phlrx->rx_role == NULL)
		    || (phlrx->rx_role->core_data == NULL)) {
		    if (whdr) {
				u8 *pda = get_ra(whdr);
				iface = rtw_get_iface_by_macddr(primary_padapter, pda);
			}
		} else {
			iface = (_adapter *)phlrx->rx_role->core_data;
		}

		if (iface) {
			prframe->u.hdr.adapter = iface;
			prframe->u.hdr.pkt->dev = iface->pnetdev;
			prframes[0] = prframe;
			if (dvobj->iface_nums > 1)
				prframes[1] = NULL;
		} else {
			rx_state = CORE_RX_FAIL;
		}
	} else {
		#ifdef CONFIG_CONCURRENT_MODE
		prframe->u.hdr.adapter = NULL;
		rx_state = _clone_bcmc_packet(dvobj, prframe, prframes);
		if (rx_state != CORE_RX_CONTINUE) {
			/* For free it */
			prframe->u.hdr.adapter = dvobj_get_primary_adapter(dvobj);
		}
		#else

		prframe->u.hdr.adapter = primary_padapter;
		prframe->u.hdr.pkt->dev = primary_padapter->pnetdev;

		prframes[0] = prframe;
		if (dvobj->iface_nums > 1)
			prframes[1] = NULL;
		if (!rtw_is_adapter_up(primary_padapter))
			rx_state = CORE_RX_DROP;
		#endif /* CONFIG_CONCURRENT_MODE */
	}

exit:
	prframe->u.hdr.rx_req = rx_req;

	return rx_state;
}

s32 rtw_core_alloc_recvframe(_adapter *adapter, union recv_frame **prframe)
{
	struct recv_priv *r_priv = &adapter->recvpriv;
	_queue *pfree_recv_queue = &r_priv->free_recv_queue;
	union recv_frame  *precvframe;

#ifdef CORE_RXSC_RFRAME
	if (adapter->cached_prframe) {
#ifdef RTW_PHL_DBG_CMD
		adapter->core_logs.rxCnt_prf_reuse++;
#endif
		adapter->cached_prframe->u.hdr.rx_req = NULL;
		*prframe = adapter->cached_prframe;
		return SUCCESS;
	}
#endif

	_rtw_spinlock_bh(&pfree_recv_queue->lock);

	precvframe = _rtw_alloc_recvframe(pfree_recv_queue);

	_rtw_spinunlock_bh(&pfree_recv_queue->lock);

	if(precvframe){
		precvframe->u.hdr.rx_req = NULL;
		*prframe = precvframe;

		#ifdef CONFIG_CONCURRENT_MODE
		precvframe->u.hdr.cloned = 0;
		precvframe->u.hdr.adapter = adapter;
		#endif /* CONFIG_CONCURRENT_MODE */

		return SUCCESS;
	}
	else
		return FAIL;
}


void rtw_core_free_recvframe(_adapter *adapter, union recv_frame *prframe)
{
	struct recv_priv *precvpriv = &(adapter->recvpriv);
	_queue	*pfree_recv_queue = &(precvpriv->free_recv_queue);

	rtw_free_recvframe(prframe, pfree_recv_queue);
}

u32 rtw_core_rx_process(void *drv_priv)
{
	struct dvobj_priv       *dvobj = (struct dvobj_priv *)drv_priv;
	struct rtw_recv_pkt     *rx_req = NULL;
	struct rtw_pkt_buf_list *pkt = NULL;
	union recv_frame        *prframe = NULL;
	union recv_frame        *prframes[CONFIG_IFACE_NUMBER];
	u16                      rx_pkt_num = 0;
	int                      i;
	_adapter                *primary_adapter = dvobj_get_primary_adapter(dvobj);
	struct recv_priv        *precvpriv = &(primary_adapter->recvpriv);
	_queue                  *pfree_recv_queue = &(precvpriv->free_recv_queue);
	s32 pre_process_ret = CORE_RX_CONTINUE;

	DBG_COUNTER(primary_adapter->rx_logs.core_rx_prcss);

#ifdef CONFIG_RTW_HANDLE_SER_L2
	if (dvobj->ser_L2_inprogress)
		return RTW_PHL_STATUS_SUCCESS;
#endif

	rx_pkt_num = rtw_phl_query_new_rx_num(GET_HAL_INFO(dvobj));

	while (rx_pkt_num--) {
		if (rtw_core_alloc_recvframe(primary_adapter, &prframe)
		    != SUCCESS) {
			DBG_COUNTER(primary_adapter->rx_logs.core_rx_allc_recvf_err);
			break;
		}

		rx_req = rtw_phl_query_rx_pkt(GET_HAL_INFO(dvobj));
		if (rx_req == NULL)
			goto rx_stop;

//CONFIG_RTW_CORE_RXSC: too many risks...
		DBG_COUNTER(primary_adapter->rx_logs.core_rx_req);

#ifdef CONFIG_RTW_CORE_RXSC
		if (core_rxsc_apply_check(primary_adapter, prframe, rx_req) == CORE_RX_GO_SHORTCUT) {
			s32 status;
			status = core_rxsc_apply_shortcut(prframe->u.hdr.adapter, prframe);
			if (CORE_RX_DONE == status) {
				DBG_COUNTER(primary_adapter->rx_logs.core_rx_rxsc_done);
				continue;
			}
			if (CORE_RX_DROP == status) {
#ifdef CORE_RXSC_RFRAME
				primary_adapter->cached_prframe = NULL;
#endif
				goto rx_next;
			}
		}
#endif

		if (rtw_core_update_recvframe(dvobj, prframe, rx_req, prframes)
		    != CORE_RX_CONTINUE) {
			goto rx_next;
		}

		for (i = 0; i < dvobj->iface_nums; i++) {
			union recv_frame *cur_rframe = prframes[i];
			_adapter *adapter = NULL;

			if (cur_rframe == NULL)
				break;

			adapter = cur_rframe->u.hdr.adapter;

			if ((adapter->registrypriv.wifi_mib.func_off)
				|| (adapter->netif_up == _FALSE)) {
				rtw_free_recvframe(cur_rframe, pfree_recv_queue);
				continue;
			}

			//recv_func_prehandle
			//mgt_dispatcher exist here && sw decrypt mgmt
			//?? todo power save
			if(validate_recv_frame(adapter, cur_rframe) != CORE_RX_CONTINUE) {
				rtw_free_recvframe(cur_rframe, pfree_recv_queue);
				continue;
			}

			pre_process_ret = rtw_core_rx_data_pre_process(adapter, &cur_rframe);
			if (pre_process_ret == CORE_RX_DEFRAG)
				continue;
			if (pre_process_ret != CORE_RX_CONTINUE) {
				rtw_free_recvframe(cur_rframe, pfree_recv_queue);
				continue;
			}

			if(rtw_core_rx_data_post_process(adapter, cur_rframe) != CORE_RX_DONE) {
				rtw_free_recvframe(cur_rframe, pfree_recv_queue);
			}
		}
		continue;

rx_next:
		DBG_COUNTER(primary_adapter->rx_logs.core_rx_next);
		rtw_free_recvframe(prframe, pfree_recv_queue);
		continue;
rx_stop:
		DBG_COUNTER(primary_adapter->rx_logs.core_rx_err_stop);
		rtw_free_recvframe(prframe, pfree_recv_queue);
		break;
	}

	DBG_COUNTER(primary_adapter->rx_logs.core_rx_prcss_succ);

	return RTW_PHL_STATUS_SUCCESS;
}
#endif /*RTW_PHL_RX*/
