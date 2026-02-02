/******************************************************************************
 *
 * Copyright(c) 2015 - 2019 Realtek Corporation.
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
#define _RTW_TRX_SDIO_C_

#include <drv_types.h>		/* struct dvobj_priv and etc. */
#include <drv_types_sdio.h>	/* RTW_SDIO_ADDR_CMD52_GEN */
#include <rtw_sdio.h>

#define CONFIG_NEW_SDIO_WP_FUNC
#ifdef CONFIG_NEW_SDIO_WP_FUNC
/*
 * Description:
 *	Write to TX FIFO
 *	Align write size to block size,
 *	and check enough FIFO size to write.
 *
 * Parameters:
 *	addr		not use
 *	cnt		size to write
 *	mem		struct xmit_buf*
 *
 * Return:
 *	_SUCCESS(1)	Success
 *	_FAIL(0)	Fail
 */
static u32 rtw_sdio_xmit(struct dvobj_priv *d, u32 txaddr, u32 cnt, u8 *mem)
{
	u32 txsize;
	u32 ret = _FAIL;

	cnt = _RND4(cnt);

	/* align size to guarantee I/O would be done in one command */
	txsize = rtw_sdio_cmd53_align_size(d, cnt);

	ret = rtw_sdio_write_cmd53(d, txaddr, mem, txsize);

	/*GEORGIA_TODO_FIXIT_MOVE_TO_CALLER*/
	/*rtw_sctx_done_err(&xmitbuf->sctx,
		(_FAIL == ret) ? RTW_SCTX_DONE_WRITE_PORT_ERR : RTW_SCTX_DONE_SUCCESS);*/
exit :
	return ret;
}
#else
static u32 rtw_sdio_xmit(struct dvobj_priv *d, u32 txaddr, u32 cnt, u8 *mem)
{
	s32 err;
	u32 txaddr;

	cnt = _RND4(cnt);
	cnt = rtw_sdio_cmd53_align_size(d, cnt);

	err = sd_write(d, txaddr, cnt, mem);

	/*GEORGIA_TODO_FIXIT_MOVE_TO_CALLER*/
	/*rtw_sctx_done_err(&xmitbuf->sctx,
		 err ? RTW_SCTX_DONE_WRITE_PORT_ERR : RTW_SCTX_DONE_SUCCESS);*/

	if (err) {
		RTW_ERR("%s, error=%d\n", __func__, err);
		return _FAIL;
	}
	return _SUCCESS;
}
#endif


/********************************xmit section*******************************/
s32 sdio_init_xmit_priv(_adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct xmit_priv *xmitpriv;
	xmitpriv = &adapter->xmitpriv;

	_rtw_init_sema(&xmitpriv->SdioXmitSema, 0);
	return _SUCCESS;
}

void sdio_free_xmit_priv(_adapter *adapter)
{
#if 0 /*CONFIG_CORE_XMITBUF*/
	struct xmit_priv *pxmitpriv;
	struct xmit_buf *pxmitbuf;
	_queue *pqueue;
	_list *plist, *phead;
	_list tmplist;

	pxmitpriv = &adapter->xmitpriv;
	pqueue = &pxmitpriv->pending_xmitbuf_queue;
	phead = get_list_head(pqueue);
	_rtw_init_listhead(&tmplist);

	_rtw_spinlock_bh(&pqueue->lock);
	if (_rtw_queue_empty(pqueue) == _FALSE) {
		/*
		 * Insert tmplist to end of queue, and delete phead
		 * then tmplist become head of queue.
		 */
		rtw_list_insert_tail(&tmplist, phead);
		rtw_list_delete(phead);
	}
	_rtw_spinunlock_bh(&pqueue->lock);

	phead = &tmplist;
	while (rtw_is_list_empty(phead) == _FALSE) {
		plist = get_next(phead);
		rtw_list_delete(plist);

		pxmitbuf = LIST_CONTAINOR(plist, struct xmit_buf, list);
		rtw_free_xmitframe(pxmitpriv, (struct xmit_frame *)pxmitbuf->priv_data);
		pxmitbuf->priv_data = NULL;
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
	}
#endif
}

/*
 * Description:
 *	Handle xmitframe(packet) come from rtw_xmit()
 *
 * Return:
 *	_TRUE	handle packet directly, maybe ok or drop
 *	_FALSE	enqueue, temporary can't transmit packets to hardware
 */
s32 sdio_data_xmit(_adapter *adapter, struct xmit_frame *pxmitframe)
{
	struct xmit_priv *pxmitpriv;
	s32 err;

	pxmitframe->attrib.qsel = pxmitframe->attrib.priority;
	pxmitpriv = &adapter->xmitpriv;

#ifdef CONFIG_80211N_HT
	if ((pxmitframe->frame_tag == DATA_FRAMETAG)
		&& (pxmitframe->attrib.ether_type != 0x0806)
		&& (pxmitframe->attrib.ether_type != 0x888e)
		&& (pxmitframe->attrib.dhcp_pkt != 1)) {
		rtw_issue_addbareq_cmd(adapter, pxmitframe, _TRUE);
	}
#endif /* CONFIG_80211N_HT */

	_rtw_spinlock_bh(&pxmitpriv->lock);
	err = rtw_xmitframe_enqueue(adapter, pxmitframe);
	_rtw_spinunlock_bh(&pxmitpriv->lock);
	if (err != _SUCCESS) {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		pxmitpriv->tx_drop++;
		return _TRUE;
	}

#ifdef CONFIG_SDIO_TX_TASKLET
	rtw_tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
#else
	_rtw_up_sema(&pxmitpriv->SdioXmitSema);
#endif

	return _FALSE;
}

/*
 * Description:
 *	Transmit manage frame
 *
 * Return:
 *	_SUCCESS	ok or enqueue
 *	_FAIL		fail
 */
s32 sdio_mgnt_xmit(_adapter *adapter, struct xmit_frame *pmgntframe)
{
	s32 ret = _SUCCESS;
#if 0 /*GEORGIA_TODO_FIXIT*/
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	struct pkt_attrib *pattrib = &pmgntframe->attrib;
	struct xmit_buf *pxmitbuf = pmgntframe->pxmitbuf;
	u32 page_size, desc_size;
	u32 txaddr;
	u16 subtype;
	u8 *pframe;


	rtw_hal_get_def_var(adapter, HAL_DEF_TX_PAGE_SIZE, &page_size);
	desc_size = rtl8822b_get_tx_desc_size(adapter);

	rtl8822b_update_txdesc(pmgntframe, pmgntframe->buf_addr);

	pxmitbuf->len = desc_size + pattrib->last_txcmdsz;
	pxmitbuf->pg_num = PageNum(pxmitbuf->len, page_size);
	pxmitbuf->ptail = pmgntframe->buf_addr + pxmitbuf->len;

	pframe = pmgntframe->buf_addr + desc_size;
	subtype = get_frame_sub_type(pframe);

	rtw_count_tx_stats(adapter, pmgntframe, pattrib->last_txcmdsz);

	rtw_free_xmitframe(pxmitpriv, pmgntframe);
	pxmitbuf->priv_data = NULL;

	if (subtype == WIFI_BEACON) {
		/* dump beacon directly */
		txaddr = rtw_hal_sdio_get_tx_addr(adapter_to_dvobj(adapter)
				,u8 * desc,u32 size);
	
		ret = rtw_sdio_xmit(adapter_to_dvobj(adapter),
			txaddr, pxmitbuf->len, (u8 *)pxmitbuf);

		rtw_sctx_done_err(&pxmitbuf->sctx,
			(_FAIL == ret)	? RTW_SCTX_DONE_WRITE_PORT_ERR
					: RTW_SCTX_DONE_SUCCESS);
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
	} else
		enqueue_pending_xmitbuf(pxmitpriv, pxmitbuf);
#endif
	return ret;
}



/*
 * Description:
 *	Aggregation packets and send to hardware
 *
 * Return:
 *	0	Success
 *	-1	Hardware resource(TX FIFO) not ready
 *	-2	Software resource(xmitbuf) not ready
 */
 /*SDIO bus-agg*/
static s32 _sdio_xmit_xmitframes(_adapter *adapter, struct xmit_priv *pxmitpriv)
{
	s32 err = 0;

	return err;
}

/*
 * Description
 *	Transmit xmitframe from queue
 *
 * Return
 *	_SUCCESS	ok
 *	_FAIL		something error
 */
static s32 _sdio_xmit_frame_handler(_adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	s32 ret;

	ret = _rtw_down_sema(&pxmitpriv->SdioXmitSema);
	if (_FAIL == ret) {
		RTW_ERR("%s: down sema fail!\n", __FUNCTION__);
		return _FAIL;
	}

next:
	if (RTW_CANNOT_RUN(dvobj)) {
		RTW_DBG(FUNC_ADPT_FMT "- bDriverStopped(%s) bSurpriseRemoved(%s)\n",
			 FUNC_ADPT_ARG(adapter),
			 dev_is_drv_stopped(dvobj) ? "True" : "False",
			 dev_is_surprise_removed(dvobj) ? "True" : "False");
		return _FAIL;
	}

	_rtw_spinlock_bh(&pxmitpriv->lock);
	ret = rtw_txframes_pending(adapter);
	_rtw_spinunlock_bh(&pxmitpriv->lock);
	/* All queues are empty! */
	if (ret == 0)
		return _SUCCESS;

	/* Dequeue frame and agg-tx then enqueue pending xmitbuf-queue */
	ret = _sdio_xmit_xmitframes(adapter, pxmitpriv);
	if (ret == -2) {
		/* here sleep 1ms will cause big TP loss of TX */
		/* from 50+ to 40+ */
		if (adapter->registrypriv.wifi_spec)
		rtw_msleep_os(1);
		else
			#ifdef CONFIG_REDUCE_TX_CPU_LOADING
			rtw_msleep_os(1);
			#else
			rtw_yield_os();
			#endif

		goto next;
	}
	_rtw_spinlock_bh(&pxmitpriv->lock);
	ret = rtw_txframes_pending(adapter);
	_rtw_spinunlock_bh(&pxmitpriv->lock);
	if (ret == 1)
		goto next;

	return _SUCCESS;
}

thread_return _sdio_xmit_frame_thread(thread_context context)
{
	s32 ret;
	_adapter *adapter;
	struct xmit_priv *pxmitpriv;
	u8 thread_name[20] = {0};


	ret = _SUCCESS;
	adapter = (_adapter *)context;
	pxmitpriv = &adapter->xmitpriv;

	rtw_sprintf(thread_name, 20, "RTWHALXT-"ADPT_FMT, ADPT_ARG(adapter));
	rtw_thread_enter(thread_name);

	RTW_INFO("start "FUNC_ADPT_FMT"\n", FUNC_ADPT_ARG(adapter));

	do {
		ret = _sdio_xmit_frame_handler(adapter);
		flush_signals_thread();
	} while (_SUCCESS == ret);

	RTW_INFO(FUNC_ADPT_FMT " Exit\n", FUNC_ADPT_ARG(adapter));

	rtw_thread_wait_stop();

	return 0;
}
u8 sdio_start_xmit_frame_thread(_adapter *adapter)
{
	u8 _status = _SUCCESS;

#ifndef CONFIG_SDIO_TX_TASKLET
	struct xmit_priv *xmitpriv = &adapter->xmitpriv;

	if (xmitpriv->SdioXmitThread == NULL) {
		RTW_INFO(FUNC_ADPT_FMT " start RTWHALXT\n", FUNC_ADPT_ARG(adapter));
		xmitpriv->SdioXmitThread = rtw_thread_start(_sdio_xmit_frame_thread, adapter, "RTWHALXT");
		if (xmitpriv->SdioXmitThread == NULL) {
			RTW_ERR("%s: start _sdio_xmit_frame_thread FAIL!!\n", __FUNCTION__);
			_status = _FAIL;
		}
	}
#endif /* !CONFIG_SDIO_TX_TASKLET */
	return _status;
}

void sdio_cancel_xmit_frame_thread(_adapter *adapter)
{
#ifndef CONFIG_SDIO_TX_TASKLET
	struct xmit_priv *xmitpriv = &adapter->xmitpriv;

	/* stop xmit_buf_thread */
	if (xmitpriv->SdioXmitThread) {
		_rtw_up_sema(&xmitpriv->SdioXmitSema);
		rtw_thread_stop(xmitpriv->SdioXmitThread);
		xmitpriv->SdioXmitThread = NULL;
	}
#endif /* !CONFIG_SDIO_TX_TASKLET */
}

s32 sdio_dequeue_xmit(_adapter *adapter)
{
#if 0
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct xmit_buf *pxmitbuf;
	u32 polling_num = 0;
	u32 txaddr = 0;

#ifdef CONFIG_SDIO_TX_ENABLE_AVAL_INT

#endif
	pxmitbuf = select_and_dequeue_pending_xmitbuf(adapter);

	if (pxmitbuf == NULL)
		return _TRUE;

#ifdef CONFIG_SDIO_TX_ENABLE_AVAL_INT
query_free_page:

	/* Total number of page is NOT available, so update current FIFO status*/
	u8	bUpdatePageNum = _FALSE;

	if (!bUpdatePageNum) {
		HalQueryTxBufferStatus8821CSdio(adapter);
		bUpdatePageNum = _TRUE;
		goto query_free_page;
	} else {
		bUpdatePageNum = _FALSE;
		enqueue_pending_xmitbuf_to_head(pxmitpriv, pxmitbuf);
		return _TRUE;
	}
#endif
	if (_TRUE == rtw_is_xmit_blocked(adapter)) {
		enqueue_pending_xmitbuf_to_head(pxmitpriv, pxmitbuf);
		/*rtw_msleep_os(1);*/
		return _FALSE;
	}

	/* check if hardware tx fifo page is enough */
	while (rtw_halmac_sdio_tx_allowed(pdvobjpriv, pxmitbuf->pdata, pxmitbuf->len)) {
		if (RTW_CANNOT_RUN(adapter_to_dvobj(adapter))) {
			RTW_INFO("%s: bSurpriseRemoved(write port)\n", __func__);
			goto free_xmitbuf;
		}

		polling_num++;
		/* Only polling (0x7F / 10) times here, since rtw_halmac_sdio_tx_allowed() has polled 10 times within */
		if ((polling_num % 6) == 0) {
			enqueue_pending_xmitbuf_to_head(pxmitpriv, pxmitbuf);
			rtw_msleep_os(1);
			return _FALSE;
		}
	}

#ifdef CONFIG_CHECK_LEAVE_LPS
	#ifdef CONFIG_LPS_CHK_BY_TP
	if (!adapter_to_pwrctl(adapter)->lps_chk_by_tp)
	#endif
		traffic_check_for_leave_lps(adapter, _TRUE, pxmitbuf->agg_num);
#endif

	if (_TRUE == rtw_is_xmit_blocked(adapter)) {
		enqueue_pending_xmitbuf_to_head(pxmitpriv, pxmitbuf);
		/*rtw_msleep_os(1);*/
		return _FALSE;
	}
	/*sdio_write_port(adapter, pxmitbuf->len, (u8 *)pxmitbuf);*/
	txaddr = rtw_hal_sdio_get_tx_addr(GET_HAL_DATA(pdvobjpriv),
					pxmitbuf->pdata, pxmitbuf->len);
	rtw_sdio_xmit(adapter, txaddr, pxmitbuf->len, (u8 *)pxmitbuf);

free_xmitbuf:
	rtw_free_xmitbuf(pxmitpriv, pxmitbuf);

#ifdef CONFIG_SDIO_TX_TASKLET
	rtw_tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
#endif

#endif
	return _FALSE;
}

/*
 * Description
 *	Transmit xmitbuf to hardware tx fifo
 *
 * Return
 *	_SUCCESS	ok
 *	_FAIL		something error
 */
#if 0 /*def CONFIG_XMIT_THREAD_MODE*/
s32 sdio_xmit_buf_handler(_adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct xmit_priv *pxmitpriv;
	u8 queue_empty, queue_pending;
	s32 ret;


	pxmitpriv = &adapter->xmitpriv;

	ret = _rtw_down_sema(&pxmitpriv->xmit_sema);
	if (_FAIL == ret) {
		RTW_ERR("%s: down SdioXmitBufSema fail!\n", __FUNCTION__);
		return _FAIL;
	}

	if (RTW_CANNOT_RUN(dvobj)) {
		RTW_DBG(FUNC_ADPT_FMT "- bDriverStopped(%s) bSurpriseRemoved(%s)\n",
			 FUNC_ADPT_ARG(adapter),
			 dev_is_drv_stopped(dvobj) ? "True" : "False",
			 dev_is_surprise_removed(dvobj) ? "True" : "False");
		return _FAIL;
	}

	if (rtw_mi_check_pending_xmitbuf(adapter) == 0)
		return _SUCCESS;
#ifdef CONFIG_LPS_LCLK
	ret = rtw_register_tx_alive(adapter);
	if (ret != _SUCCESS)
		return _SUCCESS;
#endif

	do {
		queue_empty = rtw_mi_sdio_dequeue_xmit(adapter);

	} while (!queue_empty);

#ifdef CONFIG_LPS_LCLK
	rtw_unregister_tx_alive(adapter);
#endif

	return _SUCCESS;
}
#endif
/*
 * Description:
 *	Enqueue xmitframe
 *
 * Return:
 *	_TRUE	enqueue ok
 *	_FALSE	fail
 */
s32 sdio_xmitframe_enqueue(_adapter *adapter, struct xmit_frame *pxmitframe)
{
	struct xmit_priv *pxmitpriv;
	s32 ret;


	pxmitpriv = &adapter->xmitpriv;

	ret = rtw_xmitframe_enqueue(adapter, pxmitframe);
	if (ret != _SUCCESS) {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);
		pxmitpriv->tx_drop++;
		return _FALSE;
	}

#ifdef CONFIG_SDIO_TX_TASKLET
	rtw_tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
#else /* !CONFIG_SDIO_TX_TASKLET */
	_rtw_up_sema(&pxmitpriv->SdioXmitSema);
#endif /* !CONFIG_SDIO_TX_TASKLET */

	return _TRUE;
}
/******************************** recv section******************************/

/*
 * Initialize recv private variable for hardware dependent
 * 1. recv buf
 * 2. recv tasklet
 */
static s32 _init_recvbuf(struct recv_buf *precvbuf, _adapter *adapter)
{
	_rtw_init_listhead(&precvbuf->list);
	_rtw_spinlock_init(&precvbuf->recvbuf_lock);

	precvbuf->adapter = adapter;

	return _SUCCESS;
}

static void _free_recvbuf(struct recv_buf *precvbuf)
{
	_rtw_spinlock_free(&precvbuf->recvbuf_lock);
}

#ifdef CONFIG_SDIO_RX_COPY
static s32 sdio_recv_hdl(_adapter *adapter)
{
#if 0
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct recv_priv *precvpriv = &adapter->recvpriv;
	struct recv_buf	*precvbuf;
	union recv_frame *precvframe;
	struct recv_frame_hdr *phdr;
	struct rx_pkt_attrib *pattrib;
	u8 *ptr;
	u32 desc_size;
	u32 pkt_offset;
	s32 transfer_len;
	u8 *pphy_status = NULL;
	u8 rx_report_sz = 0;

	rtw_halmac_get_rx_desc_size(dvobj, &desc_size);

	do {
		if (RTW_CANNOT_RUN(dvobj)) {
			RTW_INFO("%s => bDriverStopped or bSurpriseRemoved\n", __func__);
			break;
		}

		precvbuf = rtw_dequeue_recvbuf(&precvpriv->recv_buf_pending_queue);
		if (NULL == precvbuf)
			break;

		transfer_len = (s32)precvbuf->len;
		ptr = precvbuf->pdata;

		do {
			precvframe = rtw_alloc_recvframe(&precvpriv->free_recv_queue);
			if (precvframe == NULL) {
				rtw_enqueue_recvbuf_to_head(precvbuf, &precvpriv->recv_buf_pending_queue);
				return RTW_RFRAME_UNAVAIL;
			}

			/*rx desc parsing*/
			pattrib = &precvframe->u.hdr.attrib;
			rtl8821c_rxdesc2attribute(pattrib, ptr);

			/* fix Hardware RX data error, drop whole recv_buffer*/
			if (!rtw_hal_rcr_check(adapter, BIT_ACRC32_8821C) && pattrib->crc_err) {

				if (adapter->registrypriv.mp_mode == 0)
					RTW_INFO("%s()-%d: RX Warning! rx CRC ERROR !!\n", __func__, __LINE__);

				rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
				break;
			}

			/*if (rtl8821c_rx_ba_ssn_appended(p))*/
			if (rtw_hal_rcr_check(adapter, BIT_APP_BASSN_8821C))
				rx_report_sz = desc_size + RTW_HALMAC_BA_SSN_RPT_SIZE + pattrib->drvinfo_sz;
			else
				rx_report_sz = desc_size + pattrib->drvinfo_sz;

			pkt_offset = rx_report_sz + pattrib->shift_sz + pattrib->pkt_len;

			if ((pattrib->pkt_len == 0) || (pkt_offset > transfer_len)) {
				RTW_INFO("%s()-%d: RX Warning!, pkt_len==0 or pkt_offset(%d)> transfoer_len(%d)\n", __func__, __LINE__, pkt_offset, transfer_len);
				rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
				break;
			}

			if ((pattrib->crc_err) || (pattrib->icv_err)) {
#ifdef CONFIG_MP_INCLUDED
				if (adapter->registrypriv.mp_mode == 1) {
					if (check_fwstate(&adapter->mlmepriv, WIFI_MP_STATE)) { /*&&(padapter->mppriv.check_mp_pkt == 0))*/
						if (pattrib->crc_err == 1)
							adapter->mppriv.rx_crcerrpktcount++;
					}
				}
#endif

				RTW_INFO("%s: crc_err=%d icv_err=%d, skip!\n", __func__, pattrib->crc_err, pattrib->icv_err);
				rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
			} else {

				if (pattrib->pkt_rpt_type == NORMAL_RX) { /*Normal rx packet*/

#ifdef CONFIG_RX_PACKET_APPEND_FCS
				if (check_fwstate(&adapter->mlmepriv, WIFI_MONITOR_STATE) == _FALSE)
					if ((pattrib->pkt_rpt_type == NORMAL_RX) && rtw_hal_rcr_check(adapter, BIT_APP_FCS_8821C))
						pattrib->pkt_len -= IEEE80211_FCS_LEN;
#endif

				if (rtw_os_alloc_recvframe(adapter, precvframe,
					(ptr + rx_report_sz + pattrib->shift_sz), precvbuf->pskb) == _FAIL) {
					rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
					break;
				}

				recvframe_put(precvframe, pattrib->pkt_len);
				/*recvframe_pull(precvframe, drvinfo_sz + RXDESC_SIZE);*/


				/* update drv info*/
#if 0
				if (rtw_hal_rcr_check(padapter, BIT_APP_BASSN_8821C)) {
					/*rtl8821c_update_bassn(padapter, (ptr + RXDESC_SIZE));*/
				}
#endif

					pre_recv_entry(precvframe, pattrib->physt ? (ptr + rx_report_sz - pattrib->drvinfo_sz) : NULL);

				} else { /* C2H_PACKET */

					c2h_pre_handler_rtl8821c(adapter, ptr, transfer_len);
					rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);

				}
			}

			/* Page size of receive package is 128 bytes alignment =>DMA AGG*/

			pkt_offset = _RND8(pkt_offset);
			transfer_len -= pkt_offset;
			ptr += pkt_offset;
			precvframe = NULL;

		} while (transfer_len > 0);

		precvbuf->len = 0;

		rtw_enqueue_recvbuf(precvbuf, &precvpriv->free_recv_buf_queue);
	} while (1);

#ifdef CONFIG_RTW_NAPI
#ifdef CONFIG_RTW_NAPI_V2
	if (adapter->registrypriv.en_napi) {
		_adapter *iface;
		u8 i;

		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			precvpriv = &iface->recvpriv; 
			if (rtw_if_up(iface) == _TRUE
				&& skb_queue_len(&precvpriv->rx_napi_skb_queue))
				napi_schedule(&iface->napi);
		}
	}
#endif /* CONFIG_RTW_NAPI_V2 */
#endif /* CONFIG_RTW_NAPI */

#endif
	return _SUCCESS;

}
#ifndef CONFIG_RECV_THREAD_MODE
static void _sdio_recv_tasklet(void *priv)
{
	_adapter *adapter = (_adapter *)priv;
	s32 ret;

	ret = sdio_recv_hdl(adapter);
	if (ret == RTW_RFRAME_UNAVAIL
		|| ret == RTW_RFRAME_PKT_UNAVAIL
	) {
		/* schedule again and hope recvframe/packet is available next time. */
		rtw_tasklet_schedule(&adapter->recvpriv.recv_tasklet);
	}
}
#endif /*#ifndef CONFIG_RECV_THREAD_MODE*/
#else
#ifndef CONFIG_RECV_THREAD_MODE
static void _sdio_recv_tasklet(void *priv)
{
#if 0
	_adapter *adapter = (_adapter *)priv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct recv_priv *precvpriv = &adapter->recvpriv;
	struct recv_buf	*precvbuf;
	union recv_frame *precvframe;
	struct recv_frame_hdr *phdr;
	struct rx_pkt_attrib *pattrib;
	u8 *ptr;
	struct sk_buff *ppkt;
	u32 desc_size;
	u32 pkt_offset;

	rtw_halmac_get_rx_desc_size(dvobj, &desc_size);

	do {
		precvbuf = rtw_dequeue_recvbuf(&precvpriv->recv_buf_pending_queue);
		if (NULL == precvbuf)
			break;

		ptr = precvbuf->pdata;

		while (ptr < precvbuf->ptail) {
			precvframe = rtw_alloc_recvframe(&precvpriv->free_recv_queue);
			if (precvframe == NULL) {
				RTW_ERR("%s: no enough recv frame!\n", __FUNCTION__);
				rtw_enqueue_recvbuf_to_head(precvbuf, &precvpriv->recv_buf_pending_queue);

				/* The case of can't allocate recvframe should be temporary,
				schedule again and hope recvframe is available next time.*/
				rtw_tasklet_schedule(&precvpriv->recv_tasklet);

				return;
			}

			phdr = &precvframe->u.hdr;
			pattrib = &phdr->attrib;

			/*rx desc parsing*/
			rtl8821c_rxdesc2attribute(pattrib, ptr);

			/* fix Hardware RX data error, drop whole recv_buffer*/
			if (!rtw_hal_rcr_check(adapter, BIT_ACRC32_8821C) && pattrib->crc_err) {
				/*#if !(MP_DRIVER==1)*/
				if (adapter->registrypriv.mp_mode == 0)
					RTW_INFO("%s()-%d: RX Warning! rx CRC ERROR !!\n", __FUNCTION__, __LINE__);
				/*#endif*/
				rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
				break;
			}

			pkt_offset = desc_size + pattrib->drvinfo_sz + pattrib->pkt_len;

			if ((ptr + pkt_offset) > precvbuf->ptail) {
				RTW_INFO("%s()-%d: : next pkt len(%p,%d) exceed ptail(%p)!\n", __FUNCTION__, __LINE__, ptr, pkt_offset, precvbuf->ptail);
				rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
				break;
			}

			if ((pattrib->crc_err) || (pattrib->icv_err)) {
#ifdef CONFIG_MP_INCLUDED
				if (adapter->registrypriv.mp_mode == 1) {
					if (check_fwstate(&adapter->mlmepriv, WIFI_MP_STATE)) { /*&&(padapter->mppriv.check_mp_pkt == 0))*/
						if (pattrib->crc_err == 1)
							adapter->mppriv.rx_crcerrpktcount++;
					}
				}
#endif

				RTW_INFO("%s: crc_err=%d icv_err=%d, skip!\n", __func__, pattrib->crc_err, pattrib->icv_err);
				rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
			} else {
				ppkt = rtw_skb_clone(precvbuf->pskb);
				if (ppkt == NULL) {
					rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
					rtw_enqueue_recvbuf_to_head(precvbuf, &precvpriv->recv_buf_pending_queue);

					/* The case of can't allocate skb is serious and may never be recovered,
					 once bDriverStopped is enable, this task should be stopped.*/
					if (!rtw_is_drv_stopped(adapter))
						rtw_tasklet_schedule(&precvpriv->recv_tasklet);

					return;
				}

				phdr->pkt = ppkt;
				phdr->len = 0;
				phdr->rx_head = precvbuf->phead;
				phdr->rx_data = phdr->rx_tail = precvbuf->pdata;
				phdr->rx_end = precvbuf->pend;

				recvframe_put(precvframe, pkt_offset);
				recvframe_pull(precvframe, desc_size + pattrib->drvinfo_sz);
				skb_pull(ppkt, desc_size + pattrib->drvinfo_sz);

#ifdef CONFIG_RX_PACKET_APPEND_FCS
				if (check_fwstate(&adapter->mlmepriv, WIFI_MONITOR_STATE) == _FALSE) {
					if ((pattrib->pkt_rpt_type == NORMAL_RX) && rtw_hal_rcr_check(adapter, BIT_APP_FCS_8821C)) {
						recvframe_pull_tail(precvframe, IEEE80211_FCS_LEN);
						pattrib->pkt_len -= IEEE80211_FCS_LEN;
						ppkt->len = pattrib->pkt_len;
					}
				}
#endif

				/* move to drv info position*/
				ptr += desc_size;

				/* update drv info*/
				if (rtw_hal_rcr_check(adapter, BIT_APP_BASSN_8821C)) {
					/*rtl8821cs_update_bassn(padapter, pdrvinfo);*/
					ptr += RTW_HALMAC_BA_SSN_RPT_SIZE;
				}

				if (pattrib->pkt_rpt_type == NORMAL_RX) /*Normal rx packet*/
					pre_recv_entry(precvframe, pattrib->physt ? ptr : NULL);
				else { /* C2H_PACKET*/
					c2h_pre_handler_rtl8821c(adapter, ptr, transfer_len);
					rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
				}
			}

			/* Page size of receive package is 128 bytes alignment =>DMA AGG*/

			pkt_offset = _RND8(pkt_offset);
			precvbuf->pdata += pkt_offset;
			ptr = precvbuf->pdata;

		}

		rtw_skb_free(precvbuf->pskb);
		precvbuf->pskb = NULL;
		rtw_enqueue_recvbuf(precvbuf, &precvpriv->free_recv_buf_queue);

	} while (1);

#endif
}
#endif /*#ifndef CONFIG_RECV_THREAD_MODE*/
#endif


s32 sdio_init_recv_priv(_adapter *adapter)
{
	s32 res = _SUCCESS;
	u32 i, n;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct recv_priv *precvpriv = &adapter->recvpriv;
	struct recv_buf *precvbuf;
	u8 recvbuf_nr = GET_HAL_RECVBUF_NR(dvobj);
	u16 recvbuf_sz = GET_HAL_RECVBUF_SZ(dvobj);

	/* 1. init recv buffer */
	_rtw_init_queue(&precvpriv->free_recv_buf_queue);
	_rtw_init_queue(&precvpriv->recv_buf_pending_queue);

	n = recvbuf_nr * sizeof(struct recv_buf) + 4;
	precvpriv->pallocated_recv_buf = rtw_zmalloc(n);
	if (precvpriv->pallocated_recv_buf == NULL) {
		res = _FAIL;
		goto exit;
	}

	precvpriv->precv_buf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(precvpriv->pallocated_recv_buf), 4);

	/* init each recv buffer */
	precvbuf = (struct recv_buf *)precvpriv->precv_buf;
	for (i = 0; i < recvbuf_nr; i++) {
		res = _init_recvbuf(precvbuf, adapter);
		if (res == _FAIL)
			break;

		res = rtw_os_recvbuf_resource_alloc(adapter, precvbuf);
		if (res == _FAIL) {
			_free_recvbuf(precvbuf);
			break;
		}
#ifdef CONFIG_SDIO_RX_COPY
		if (precvbuf->pskb == NULL) {
			SIZE_PTR tmpaddr = 0;
			SIZE_PTR alignment = 0;

			precvbuf->pskb = rtw_skb_alloc(recvbuf_sz + RECVBUFF_ALIGN_SZ);

			if (precvbuf->pskb) {
				precvbuf->pskb->dev = adapter->pnetdev;

				tmpaddr = (SIZE_PTR)precvbuf->pskb->data;
				alignment = tmpaddr & (RECVBUFF_ALIGN_SZ - 1);
				skb_reserve(precvbuf->pskb, (RECVBUFF_ALIGN_SZ - alignment));

				precvbuf->phead = precvbuf->pskb->head;
				precvbuf->pdata = precvbuf->pskb->data;
				precvbuf->ptail = skb_tail_pointer(precvbuf->pskb);
				precvbuf->pend = skb_end_pointer(precvbuf->pskb);
				precvbuf->len = 0;
			}

			if (precvbuf->pskb == NULL)
				RTW_INFO("%s: alloc_skb fail!\n", __FUNCTION__);
		}
#endif

#if 0
		res = os_recvbuf_resource_alloc(adapter, precvbuf);
		if (res == _FAIL) {
			freerecvbuf(precvbuf);
			break;
		}
#endif
		rtw_list_insert_tail(&precvbuf->list, &precvpriv->free_recv_buf_queue.queue);

		precvbuf++;
	}
	precvpriv->free_recv_buf_queue_cnt = i;

	if (res == _FAIL)
		goto initbuferror;

	/* 2. init tasklet */
#ifndef CONFIG_RECV_THREAD_MODE
	rtw_tasklet_init(&precvpriv->recv_tasklet,
		     (void(*)(unsigned long))_sdio_recv_tasklet,
		     (unsigned long)adapter);
#endif
	goto exit;

initbuferror:
	precvbuf = (struct recv_buf *)precvpriv->precv_buf;
	if (precvbuf) {
		n = precvpriv->free_recv_buf_queue_cnt;
		precvpriv->free_recv_buf_queue_cnt = 0;
		for (i = 0; i < n ; i++) {
			rtw_list_delete(&precvbuf->list);
			rtw_os_recvbuf_resource_free(adapter, precvbuf);
			_free_recvbuf(precvbuf);
			precvbuf++;
		}
		precvpriv->precv_buf = NULL;
	}

	if (precvpriv->pallocated_recv_buf) {
		n = recvbuf_nr * sizeof(struct recv_buf) + 4;
		rtw_mfree(precvpriv->pallocated_recv_buf, n);
		precvpriv->pallocated_recv_buf = NULL;
	}

exit:
	return res;
}

/*
 * Free recv private variable of hardware dependent
 * 1. recv buf
 * 2. recv tasklet
 */
void sdio_free_recv_priv(_adapter *adapter)
{
	u32 i, n;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct recv_priv *precvpriv = &adapter->recvpriv;
	struct recv_buf *precvbuf;
	u8 recvbuf_nr = GET_HAL_RECVBUF_NR(dvobj);
	u16 recvbuf_sz = GET_HAL_RECVBUF_SZ(dvobj);

	/* 1. kill tasklet */
#ifndef CONFIG_RECV_THREAD_MODE
	rtw_tasklet_kill(&adapter->recvpriv.recv_tasklet);
#endif

	/* 2. free all recv buffers */
	precvbuf = (struct recv_buf *)precvpriv->precv_buf;
	if (precvbuf) {
		n = recvbuf_nr;
		precvpriv->free_recv_buf_queue_cnt = 0;
		for (i = 0; i < n ; i++) {
			rtw_list_delete(&precvbuf->list);
			rtw_os_recvbuf_resource_free(adapter, precvbuf);
			_free_recvbuf(precvbuf);
			precvbuf++;
		}
		precvpriv->precv_buf = NULL;
	}

	if (precvpriv->pallocated_recv_buf) {
		n = recvbuf_nr * sizeof(struct recv_buf) + 4;
		rtw_mfree(precvpriv->pallocated_recv_buf, n);
		precvpriv->pallocated_recv_buf = NULL;
	}
}


struct rtw_intf_ops sdio_ops = {
	.read		= rtw_sdio_raw_read,
	.write		= rtw_sdio_raw_write,

	/****************** data path *****************/

	/****************** xmit *********************/
	.init_xmit_priv = sdio_init_xmit_priv,
	.free_xmit_priv = sdio_free_xmit_priv,
	.data_xmit	= sdio_data_xmit,
	.xmitframe_enqueue = sdio_xmitframe_enqueue,
	.start_xmit_frame_thread	= sdio_start_xmit_frame_thread,
	.cancel_xmit_frame_thread	= sdio_cancel_xmit_frame_thread,
	#if 0 /*def CONFIG_XMIT_THREAD_MODE*/
	.xmit_buf_handler	= sdio_xmit_buf_handler,
	#endif

	/******************  recv *********************/
	.init_recv_priv = sdio_init_recv_priv,
	.free_recv_priv = sdio_free_recv_priv,
	#ifdef CONFIG_RECV_THREAD_MODE
	.recv_hdl = sdio_recv_hdl,
	#endif
};

