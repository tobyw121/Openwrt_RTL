/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#define _RTW_TRX_PCI_C_
#include <drv_types.h>		/* struct dvobj_priv and etc. */

static void rtw_mi_pci_tasklets_kill(_adapter *padapter)
{
	int i;
	_adapter *iface;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if ((iface) && rtw_is_adapter_up(iface)) {
			rtw_tasklet_kill(&(padapter->recvpriv.recv_tasklet));
			rtw_tasklet_kill(&(padapter->recvpriv.irq_prepare_beacon_tasklet));
			rtw_tasklet_kill(&(padapter->xmitpriv.xmit_tasklet));
		}
	}
}

#if 0 /*def CONFIG_TX_AMSDU*/
static s32 xmitframe_amsdu_direct(_adapter *padapter,
					struct xmit_frame *pxmitframe)
{
	struct xmit_buf *pxmitbuf = pxmitframe->pxmitbuf;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	s32 res = _SUCCESS;

	res = rtw_xmitframe_coalesce_amsdu(padapter, pxmitframe, NULL);

	if (res == _SUCCESS) {
#ifdef CONFIG_XMIT_THREAD_MODE
		enqueue_pending_xmitbuf(pxmitpriv, pxmitframe->pxmitbuf);
#else
		res = rtw_hal_dump_xframe(padapter, pxmitframe);
#endif
	} else {
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
		rtw_free_xmitframe(pxmitpriv, pxmitframe);
	}

	return res;
}
#endif

/********************************xmit section*****************************/

#if 0/* CONFIG_VW_REFINE */

#ifdef RTW_PHL_TX
static struct xmit_frame *core_dz_dequeue(struct xmit_priv *pxmitpriv)
{
	_queue *sleep_q = &pxmitpriv->sleep_q;
	struct xmit_frame *pxmitframe;

	_rtw_spinlock_bh(&sleep_q->lock);
	pxmitframe = core_dequeue_one_xframe(sleep_q);
	_rtw_spinunlock_bh(&sleep_q->lock);

	return pxmitframe;
}

#if !defined(USE_HIQ)
static struct xmit_frame *core_bmc_dz_dequeue(struct xmit_priv *pxmitpriv)
{
	_list *psta_head, *psta_list;
	struct xmit_frame *pxmitframe = NULL;
	struct sta_info *psta_bmc;
	_queue *sleep_q;

	_rtw_spinlock_bh(&pxmitpriv->lock);

	psta_head = &pxmitpriv->bmc_list;
	psta_list = psta_head->next;

	while (psta_list != psta_head) {
		psta_bmc = LIST_CONTAINOR(psta_list, struct sta_info, bmc_list);
		psta_list = psta_list->next;

		sleep_q = &psta_bmc->sleep_q;
		pxmitframe = core_dequeue_one_xframe(sleep_q);
		if (sleep_q->qlen == 0)
			rtw_list_delete(&psta_bmc->bmc_list);
		if (pxmitframe) {
			if (sleep_q->qlen)
				pxmitframe->attrib.mdata = 1;
			else
				pxmitframe->attrib.mdata = 0;
			break;
		}
	}

	_rtw_spinunlock_bh(&pxmitpriv->lock);

	return pxmitframe;
}
#endif

static struct xmit_frame *core_wmm_dz_dequeue(struct xmit_priv *pxmitpriv)
{
	_list *psta_head, *psta_list;
	_list *phead, *plist;
	struct xmit_frame *pxmitframe = NULL;
	struct sta_info *psta;
	struct sta_xmit_priv *pstaxmitpriv;
	struct tx_servq *ptxservq;
	u16 tx_pending;
	int q_idx;

	_rtw_spinlock_bh(&pxmitpriv->lock);

	psta_head = &pxmitpriv->trigger_list;
	psta_list = psta_head->next;

	while (psta_list != psta_head) {
		psta = LIST_CONTAINOR(psta_list, struct sta_info, trigger_list);
		psta_list = psta_list->next;

		pstaxmitpriv = &psta->sta_xmitpriv;
		tx_pending = pstaxmitpriv->tx_pending & psta->uapsd_bitmap;

		for (q_idx = 0; q_idx < TXQ_MAX; q_idx++) {
			if (!(tx_pending & BIT(q_idx)))
				continue;
			ptxservq = &pstaxmitpriv->swq[q_idx];
			pxmitframe = core_dequeue_one_xframe(&ptxservq->queue);
			if (tx_servq_len(ptxservq) == 0) {
				pstaxmitpriv->tx_pending &= ~ BIT(q_idx);
				tx_pending &= ~ BIT(q_idx);
				if (!tx_pending)
					rtw_list_delete(&psta->trigger_list);
			}
			if (pxmitframe) {
				pstaxmitpriv->swq_total_len--;
				if (tx_pending) {
					pxmitframe->attrib.mdata = 1;
					pxmitframe->attrib.eosp = 0;
				} else {
					pxmitframe->attrib.mdata = 0;
					pxmitframe->attrib.eosp = 1;
				}
				goto exit;
			}
		}
	}
exit:

	if (pxmitframe) {
#ifdef CONFIG_TDLS
		if (!(psta->tdls_sta_state & TDLS_LINKED_STATE))
#endif
		if (!tx_pending) {
			_adapter *padapter = pxmitpriv->adapter;
			struct sta_priv *pstapriv = &padapter->stapriv;

			rtw_tim_map_clear(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);
			update_beacon(padapter, _TIM_IE_, NULL, _TRUE, 0);
		}
	}
	_rtw_spinunlock_bh(&pxmitpriv->lock);

	return pxmitframe;
}

static void core_pci_xmit_tasklet(_adapter *padapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct xmit_frame *pxmitframe;

	while (1) {
		if (RTW_CANNOT_RUN(dvobj)) {
			RTW_INFO("%s => bDriverStopped or bSurpriseRemoved\n",
				 __func__);
			break;
		}

		do {
#if !defined(USE_HIQ)
			pxmitframe = core_bmc_dz_dequeue(pxmitpriv);
			if (pxmitframe)
				break;
#endif
			pxmitframe = core_dz_dequeue(pxmitpriv);
			if (pxmitframe)
				break;
			pxmitframe = core_wmm_dz_dequeue(pxmitpriv);
			if (pxmitframe)
				break;
			pxmitframe = core_dequeue_xframe(pxmitpriv);

		} while (0);

		if (!pxmitframe)
			break;

		if (core_tx_prepare_phl(padapter, pxmitframe) == FAIL) {
			core_tx_free_xmitframe(padapter, pxmitframe);
			continue;
		}

		if (core_tx_call_phl(padapter, pxmitframe, NULL) == FAIL) {
			core_tx_free_xmitframe(padapter, pxmitframe);
			continue;
		}
	}

}
#endif /* RTW_PHL_TX */
#endif

static void pci_xmit_tasklet(_adapter *padapter)
{
#ifdef CONFIG_TX_AMSDU_SW_MODE
	core_tx_amsdu_tasklet(padapter);
#endif
}

s32 pci_init_xmit_priv(_adapter *adapter)
{
	s32 ret = _SUCCESS;
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	_rtw_spinlock_init(&dvobj_to_pci(dvobj)->irq_th_lock);

	rtw_tasklet_init(&pxmitpriv->xmit_tasklet,
		     (void(*)(unsigned long))pci_xmit_tasklet,
		     (unsigned long)adapter);

	return ret;
}

void pci_free_xmit_priv(_adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	_rtw_spinlock_free(&dvobj_to_pci(dvobj)->irq_th_lock);
}

static s32 pci_dump_xframe(_adapter *adapter, struct xmit_frame *pxmitframe)
{
	#ifdef CONFIG_80211N_HT
	if ((pxmitframe->frame_tag == DATA_FRAMETAG) &&
		(pxmitframe->attrib.ether_type != 0x0806) &&
		(pxmitframe->attrib.ether_type != 0x888e) &&
		(pxmitframe->attrib.dhcp_pkt != 1))
		rtw_issue_addbareq_cmd(adapter, pxmitframe, _FALSE);
#endif /* CONFIG_80211N_HT */
	return rtw_hal_pci_dump_xframe(adapter, pxmitframe);

}

static s32 pci_xmit_direct(_adapter *adapter, struct xmit_frame *pxmitframe)
{
#ifdef CONFIG_XMIT_THREAD_MODE
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
#endif
	s32 res = _SUCCESS;

	res = rtw_xmitframe_coalesce(adapter, pxmitframe->pkt, pxmitframe);
	if (res == _SUCCESS) {
	#if 0 /*def CONFIG_XMIT_THREAD_MODE*/
		enqueue_pending_xmitbuf(pxmitpriv, pxmitframe->pxmitbuf);
	#else
		pci_dump_xframe(adapter, pxmitframe);
	#endif
	}
	return res;
}

static s32 pci_data_xmit(_adapter *adapter, struct xmit_frame *pxmitframe)
{
	s32 res;
	#if 0 /*CONFIG_CORE_XMITBUF*/
	struct xmit_buf *pxmitbuf = NULL;
	#endif
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
#ifdef CONFIG_TX_AMSDU
	int tx_amsdu = adapter->tx_amsdu;
	u8 amsdu_timeout = 0;
#endif

	_rtw_spinlock_bh(&pxmitpriv->lock);

	if (rtw_txframes_sta_ac_pending(adapter, pattrib) > 0)
		goto enqueue;

#ifndef CONFIG_XMIT_THREAD_MODE
	if (rtw_hal_pci_check_enough_txdesc(GET_HAL_DATA(dvobj), pattrib->qsel) == _FALSE)
		goto enqueue;

	if (rtw_xmit_ac_blocked(adapter) == _TRUE)
		goto enqueue;
#endif

	if (DEV_STA_LG_NUM(adapter->dvobj))
		goto enqueue;

#ifdef CONFIG_TX_AMSDU
	if (MLME_IS_STA(adapter) &&
		check_amsdu_tx_support(adapter)) {

		if (IS_AMSDU_AMPDU_VALID(pattrib))
			goto enqueue;
	}
#endif
#if 0 /*CONFIG_CORE_XMITBUF*/
	pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
	if (pxmitbuf == NULL)
		goto enqueue;

	_rtw_spinunlock_bh(&pxmitpriv->lock);

	pxmitframe->pxmitbuf = pxmitbuf;
	pxmitframe->buf_addr = pxmitbuf->pbuf;
	pxmitbuf->priv_data = pxmitframe;
#else
	_rtw_spinunlock_bh(&pxmitpriv->lock);
#endif
	if (pci_xmit_direct(adapter, pxmitframe) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pxmitframe);
	}

	return _TRUE;

enqueue:
	res = rtw_xmitframe_enqueue(adapter, pxmitframe);

#ifdef CONFIG_TX_AMSDU
	if(res == _SUCCESS && tx_amsdu == 2)
	{
		amsdu_timeout = rtw_amsdu_get_timer_status(adapter, pattrib->priority);
		if(amsdu_timeout == RTW_AMSDU_TIMER_SETTING)
		{
			rtw_amsdu_cancel_timer(adapter, pattrib->priority);
			rtw_amsdu_set_timer_status(adapter, pattrib->priority,
				RTW_AMSDU_TIMER_UNSET);
		}
	}
#endif

	_rtw_spinunlock_bh(&pxmitpriv->lock);

	if (res != _SUCCESS) {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		pxmitpriv->tx_drop++;
		return _TRUE;
	}

#ifdef CONFIG_TX_AMSDU
	rtw_tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
#endif
	return _FALSE;
}

s32 pci_mgnt_xmit(_adapter *adapter, struct xmit_frame *pmgntframe)
{

#if 0 /*def CONFIG_XMIT_THREAD_MODE*/
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	struct pkt_attrib	*pattrib = &pmgntframe->attrib;
	s32 ret = _SUCCESS;

	/* For FW download rsvd page and H2C pkt */
	if ((pattrib->qsel == QSLT_CMD) || (pattrib->qsel == QSLT_BEACON))
		ret = pci_dump_xframe(adapter, pmgntframe);
	else
		enqueue_pending_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
	return ret;

#else
	return pci_dump_xframe(adapter, pmgntframe);
#endif
}
#if 0 /*def CONFIG_XMIT_THREAD_MODE*/
/*
 * Description
 *	Transmit xmitbuf to hardware tx fifo
 *
 * Return
 *	_SUCCESS	ok
 *	_FAIL		something error
 */
s32 pci_xmit_buf_handler(_adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct xmit_priv *pxmitpriv;
	struct xmit_buf *pxmitbuf;
	struct xmit_frame *pxmitframe;
	s32 ret;

	pxmitpriv = &adapter->xmitpriv;

	ret = _rtw_down_sema(&pxmitpriv->xmit_sema);

	if (ret == _FAIL) {
		RTW_ERR("%s: down XmitBufSema fail!\n", __FUNCTION__);
		return _FAIL;
	}

	if (RTW_CANNOT_RUN(dvobj)) {
		RTW_INFO("%s: bDriverStopped(%s) bSurpriseRemoved(%s)!\n"
			, __func__
			, dev_is_drv_stopped(dvobj) ? "True" : "False"
			, dev_is_surprise_removed(dvobj) ? "True" : "False");
		return _FAIL;
	}

	if (check_pending_xmitbuf(pxmitpriv) == _FALSE)
		return _SUCCESS;

#ifdef CONFIG_LPS_LCLK
	ret = rtw_register_tx_alive(adapter);
	if (ret != _SUCCESS) {
		RTW_INFO("%s: wait to leave LPS_LCLK\n", __FUNCTION__);
		return _SUCCESS;
	}
#endif

	do {
		pxmitbuf = select_and_dequeue_pending_xmitbuf(adapter);

		if (pxmitbuf == NULL)
			break;
		pxmitframe = (struct xmit_frame *)pxmitbuf->priv_data;

		if (rtw_hal_pci_check_enough_txdesc(GET_HAL_DATA(dvobj),
					pxmitframe->attrib.qsel) == _FALSE) {
			enqueue_pending_xmitbuf_to_head(pxmitpriv, pxmitbuf);
			break;
		}
		pci_dump_xframe(adapter, pxmitframe);
	} while (1);


	return _SUCCESS;
}
#endif
s32 pci_xmitframe_enqueue(_adapter *adapter,
				    struct xmit_frame *pxmitframe)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	s32 err;

	err = rtw_xmitframe_enqueue(adapter, pxmitframe);
	if (err != _SUCCESS) {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);
		pxmitpriv->tx_drop++;
	} else {
		if (rtw_hal_pci_check_enough_txdesc(GET_HAL_DATA(dvobj),
					  pxmitframe->attrib.qsel) == _TRUE)
			rtw_tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
	}

	return err;
}
/******************************** recv section*******************************/
static void pci_rx_mpdu(_adapter *adapter)
{
#if 1
	/*GEORGIA_TODO_FIXIT*/
	/*rtw_hal_pci_rx_mpdu(); ?????*/
#else
	struct recv_priv *r_priv = &padapter->recvpriv;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	_queue *pfree_recv_queue = &r_priv->free_recv_queue;
	union recv_frame *precvframe = NULL;
	struct rx_pkt_attrib *pattrib = NULL;
	int rx_q_idx = RX_MPDU_QUEUE;
	u32 count = r_priv->rxringcount;
	u16 remaing_rxdesc = 0;
	u8 *rx_bd;
	struct sk_buff *skb;
	u32 desc_size;


	desc_size = rtl8822b_get_rx_desc_size(padapter);

	/* RX NORMAL PKT */

	remaing_rxdesc = rtl8822be_check_rxdesc_remain(padapter, rx_q_idx);
	while (remaing_rxdesc) {

		/* rx descriptor */
		rx_bd = (u8 *)&r_priv->rx_ring[rx_q_idx].buf_desc[r_priv->rx_ring[rx_q_idx].idx];

		/* rx packet */
		skb = r_priv->rx_ring[rx_q_idx].rx_buf[r_priv->rx_ring[rx_q_idx].idx];

		buf_desc_debug("RX:%s(%d), rx_bd addr = %x, total_len = %d, ring idx = %d\n",
			       __func__, __LINE__, (u32)rx_bd,
			       GET_RX_BD_TOTALRXPKTSIZE(rx_bd),
			       r_priv->rx_ring[rx_q_idx].idx);

		buf_desc_debug("RX:%s(%d), skb(rx_buf)=%x, buf addr(virtual = %x, phisycal = %x)\n",
			       __func__, __LINE__, (u32)skb,
			       (u32)(skb_tail_pointer(skb)),
			       GET_RX_BD_PHYSICAL_ADDR_LOW(rx_bd));

		/* wait until packet is ready. this operation is similar to
		 * check own bit and should be called before pci_unmap_single
		 * which release memory mapping
		 */

		if (rtl8822be_wait_rxrdy(padapter, rx_bd, rx_q_idx) !=
		    _SUCCESS)
			buf_desc_debug("RX:%s(%d) packet not ready\n",
				       __func__, __LINE__);

		{
			precvframe = rtw_alloc_recvframe(pfree_recv_queue);

			if (precvframe == NULL) {
				goto done;
			}

			_rtw_init_listhead(&precvframe->u.hdr.list);
			precvframe->u.hdr.len = 0;

			pci_unmap_single(pdvobjpriv->ppcidev,
					 *((dma_addr_t *)skb->cb),
					 r_priv->rxbuffersize,
					 PCI_DMA_FROMDEVICE);

			rtl8822b_query_rx_desc(precvframe, skb->data);
			pattrib = &precvframe->u.hdr.attrib;

#ifdef CONFIG_RX_PACKET_APPEND_FCS
			{
				struct mlme_priv *mlmepriv =
						&padapter->mlmepriv;

				if (check_fwstate(mlmepriv,
						  WIFI_MONITOR_STATE) == _FALSE)
					if (pattrib->pkt_rpt_type == NORMAL_RX)
						pattrib->pkt_len -=
							IEEE80211_FCS_LEN;
			}
#endif

			buf_desc_debug("RX:%s(%d), pkt_len = %d, pattrib->drvinfo_sz = %d, pattrib->qos = %d, pattrib->shift_sz = %d\n",
				       __func__, __LINE__, pattrib->pkt_len,
				       pattrib->drvinfo_sz, pattrib->qos,
				       pattrib->shift_sz);

			if (rtw_os_alloc_recvframe(padapter, precvframe,
				   (skb->data + desc_size +
				    pattrib->drvinfo_sz + pattrib->shift_sz),
						   skb) == _FAIL) {

				rtw_free_recvframe(precvframe,
						   &r_priv->free_recv_queue);

				RTW_INFO("rtl8822be_rx_mpdu:can't allocate memory for skb copy\n");
				*((dma_addr_t *) skb->cb) =
					pci_map_single(pdvobjpriv->ppcidev,
						       skb_tail_pointer(skb),
						       r_priv->rxbuffersize,
						       PCI_DMA_FROMDEVICE);
				goto done;
			}

			recvframe_put(precvframe, pattrib->pkt_len);

			if (pattrib->pkt_rpt_type == NORMAL_RX) {
				/* Normal rx packet */
				pre_recv_entry(precvframe, pattrib->physt ? ((u8 *)(skb->data) + desc_size) : NULL);
			} else {
				if (pattrib->pkt_rpt_type == C2H_PACKET)
					rtl8822b_c2h_handler_no_io(padapter,
							     skb->data,
							     desc_size +
							     pattrib->pkt_len);

				rtw_free_recvframe(precvframe,
						   pfree_recv_queue);
			}
			*((dma_addr_t *) skb->cb) =
				pci_map_single(pdvobjpriv->ppcidev,
					       skb_tail_pointer(skb),
					       r_priv->rxbuffersize,
					       PCI_DMA_FROMDEVICE);
		}
done:


		SET_RX_BD_PHYSICAL_ADDR_LOW(rx_bd, *((dma_addr_t *)skb->cb));
		SET_RX_BD_RXBUFFSIZE(rx_bd, r_priv->rxbuffersize);

		r_priv->rx_ring[rx_q_idx].idx =
			(r_priv->rx_ring[rx_q_idx].idx + 1) %
			r_priv->rxringcount;

		rtw_write16(padapter, REG_RXQ_RXBD_IDX,
			    r_priv->rx_ring[rx_q_idx].idx);

		buf_desc_debug("RX:%s(%d) reg_value %x\n", __func__, __LINE__,
			       rtw_read32(padapter, REG_RXQ_RXBD_IDX));

		remaing_rxdesc--;
	}
#endif
}

static void pci_recv_tasklet(void *priv)
{
	_adapter	*padapter = (_adapter *)priv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	pci_rx_mpdu(padapter);
#if 0 /*GEORGIA_TODO_FIXIT*/
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	_rtw_spinlock_irq(&dvobj_to_pci(dvobj)->irq_th_lock);
	pHalData->IntrMask[0] |= (BIT_RXOK_MSK_8822B | BIT_RDU_MSK_8822B);
	pHalData->IntrMask[1] |= BIT_FOVW_MSK_8822B;
	rtw_write32(padapter, REG_HIMR0_8822B, pHalData->IntrMask[0]);
	rtw_write32(padapter, REG_HIMR1_8822B, pHalData->IntrMask[1]);
	_rtw_spinunlock_irq(&dvobj_to_pci(dvobj)->irq_th_lock);
#endif
}


static void pci_prepare_bcn_tasklet(void *priv)
{
	#if defined(CONFIG_AP_MODE) && defined(CONFIG_NATIVEAP_MLME)
	_adapter *adapter = (_adapter *)priv;
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;

	if (MLME_IS_AP(adapter) || MLME_IS_MESH(adapter)) {
		/* send_beacon(Adapter); */
		if (pmlmepriv->update_bcn == _TRUE)
			tx_beacon_hdl(adapter, NULL);
	}
	#endif

}
s32 pci_init_recv_priv(_adapter *adapter)
{
	struct recv_priv	*precvpriv = &adapter->recvpriv;
	s32	ret = _SUCCESS;

	rtw_tasklet_init(&precvpriv->recv_tasklet,
		     (void(*)(unsigned long))pci_recv_tasklet,
		     (unsigned long)adapter);

	rtw_tasklet_init(&precvpriv->irq_prepare_beacon_tasklet,
		     (void(*)(unsigned long))pci_prepare_bcn_tasklet,
		     (unsigned long)adapter);

	return ret;
}

void pci_free_recv_priv(_adapter *adapter)
{

}


struct rtw_intf_ops pci_ops = {
	.init_xmit_priv = pci_init_xmit_priv,
	.free_xmit_priv = pci_free_xmit_priv,
	.data_xmit	= pci_data_xmit,
	.xmitframe_enqueue = pci_xmitframe_enqueue,
	.mgnt_xmit	= pci_mgnt_xmit,
	#if 0 /*def CONFIG_XMIT_THREAD_MODE*/
	.xmit_buf_handler = pci_xmit_buf_handler
	#endif

	.init_recv_priv = pci_init_recv_priv,
	.free_recv_priv = pci_free_recv_priv,
};

