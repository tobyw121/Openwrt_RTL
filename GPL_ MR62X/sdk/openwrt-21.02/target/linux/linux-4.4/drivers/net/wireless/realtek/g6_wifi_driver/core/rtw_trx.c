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
#define _RTW_TRX_C_
#include <drv_types.h>		/* struct dvobj_priv and etc. */


#ifdef RTW_PHL_TX
s32 rtw_core_tx_mgmt(_adapter *padapter, struct xmit_frame *pxframe)
{
	if (padapter->registrypriv.wifi_mib.func_off)
	{
#ifdef RTW_CORE_PKT_TRACE
		if(padapter->pkt_trace_enable)
			RTW_TX_TRACE(padapter,&pxframe->attrib.pktinfo);
#endif
		goto abort_core_tx;
	}

	XF_TYPE = RTW_TX_DRV_MGMT;

#ifdef RTW_PHL_DBG_CMD
	core_add_record(padapter, padapter->record_enable, REC_TX_MGMT, pxframe);
#endif

	if(core_tx_prepare_phl(padapter, pxframe) == FAIL)
	{
#ifdef RTW_CORE_PKT_TRACE
		if(padapter->pkt_trace_enable)
			RTW_TX_TRACE(padapter,&pxframe->attrib.pktinfo);;
#endif
		goto abort_core_tx;
	}

#ifdef CONFIG_XMIT_MGMT_ACK
	if (pxframe->ack_rpt.txfb.txfb_cb) {
		pxframe->phl_txreq[0].txfb = &pxframe->ack_rpt.txfb;
	}
#endif

#ifdef CONFIG_RTW_HANDLE_SER_L2
	if (padapter->dvobj->ser_L2_inprogress &&
	    XF_SEC_TYPE != _NO_PRIVACY_ && XF_SWENC != _TRUE) {
	    pxframe->attrib.bswenc = _TRUE;
	}
#endif

	if(core_tx_call_phl(padapter, pxframe, NULL) == FAIL)
	{
#ifdef RTW_CORE_PKT_TRACE
		if(padapter->pkt_trace_enable)
			RTW_TX_TRACE(padapter,&pxframe->attrib.pktinfo);
#endif
		goto abort_core_tx;
	}

	return _SUCCESS;

abort_core_tx:
	RTW_WARN(FUNC_ADPT_FMT" abort TX MGNT. (%pX)\n",
		 FUNC_ADPT_ARG(padapter), pxframe);
	core_tx_free_xmitframe(padapter, pxframe);
	return _FAIL;
}
#endif

#ifdef CONFIG_DRV_FAKE_AP
int rtw_fakeap_tx(struct _ADAPTER*, struct xmit_frame*);
#endif /* CONFIG_DRV_FAKE_AP */
/*rtw_hal_mgnt_xmit*/
s32 rtw_mgnt_xmit(_adapter *adapter, struct xmit_frame *pmgntframe)
{
	s32 ret = _FAIL;
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	update_mgntframe_attrib_addr(adapter, pmgntframe);

#if defined(CONFIG_IEEE80211W) || defined(CONFIG_RTW_MESH)
	if ((!MLME_IS_MESH(adapter) && SEC_IS_BIP_KEY_INSTALLED(&adapter->securitypriv) == _TRUE)
		#ifdef CONFIG_RTW_MESH
		|| (MLME_IS_MESH(adapter) && adapter->mesh_info.mesh_auth_id)
		#endif
	)
		rtw_mgmt_xmitframe_coalesce(adapter, pmgntframe->pkt, pmgntframe);
#endif

#ifdef RTW_CORE_PKT_TRACE
	if(adapter->pkt_trace_enable)
	{
		rtw_wifi_tx_prepare_pkt_trace(adapter, pmgntframe);
		RTW_TX_TRACE(adapter,&pmgntframe->attrib.pktinfo);
	}
#endif

#ifdef CONFIG_DRV_FAKE_AP
#ifndef RTW_PHL_TEST_FPGA
	if (rtw_fakeap_tx(adapter, pmgntframe) == _SUCCESS)
	{
#ifdef RTW_CORE_PKT_TRACE
		if(adapter->pkt_trace_enable)
			RTW_TX_TRACE(adapter,&pmgntframe->attrib.pktinfo);
#endif
		return _SUCCESS;
	}
#endif
#endif /* CONFIG_DRV_FAKE_AP */

#if defined(WIFI_LOGO_MBO_4_2_5_3)
	if(pmlmeinfo->is_MBO_4_2_5_3)
		goto direct_xmit;
#endif
#if defined(WIFI_LOGO_MBO_4_2_5_4)
	if(pmlmeinfo->is_MBO_4_2_5_4)
		goto direct_xmit;
#endif

#if defined(CONFIG_AP_MODE) || defined(CONFIG_TDLS)
	if (mgnt_tx_enqueue_for_sleeping_sta(adapter, pmgntframe) == _TRUE)
	{
#ifdef RTW_CORE_PKT_TRACE
		if(adapter->pkt_trace_enable)
			RTW_TX_TRACE(adapter,&pmgntframe->attrib.pktinfo);
#endif
		return _SUCCESS;
	}
#endif

direct_xmit:
#ifdef RTW_PHL_TX
	ret = rtw_core_tx_mgmt(adapter, pmgntframe);
#else
	ret = dvobj->intf_ops->mgnt_xmit(adapter, pmgntframe);
#endif
	return ret;
}


struct lite_data_buf *rtw_alloc_litedatabuf(struct trx_data_buf_q *data_buf_q)
{
	struct lite_data_buf *litedatabuf =  NULL;
	_list *list, *head;
	_queue *free_litedatabuf_q = &data_buf_q->free_data_buf_queue;
	unsigned long sp_flags;

	/* RTW_INFO("+rtw_alloc_litexmitbuf\n"); */

	_rtw_spinlock_irq(&free_litedatabuf_q->lock, &sp_flags);

	if (_rtw_queue_empty(free_litedatabuf_q) == _TRUE)
		litedatabuf = NULL;
	else {

		head = get_list_head(free_litedatabuf_q);

		list = get_next(head);

		litedatabuf = LIST_CONTAINOR(list,
			struct lite_data_buf, list);

		rtw_list_delete(&(litedatabuf->list));
	}

	if (litedatabuf !=  NULL) {
		data_buf_q->free_data_buf_cnt--;


		if (litedatabuf->sctx) {
			RTW_INFO("%s plitexmitbuf->sctx is not NULL\n",
				__func__);
			rtw_sctx_done_err(&litedatabuf->sctx,
				RTW_SCTX_DONE_BUF_ALLOC);
		}
	}

	_rtw_spinunlock_irq(&free_litedatabuf_q->lock, &sp_flags);


	return litedatabuf;
}

s32 rtw_free_litedatabuf(struct trx_data_buf_q *data_buf_q,
		struct lite_data_buf *lite_data_buf)
{
	_queue *free_litedatabuf_q = &data_buf_q->free_data_buf_queue;
	unsigned long sp_flags;

	/* RTW_INFO("+rtw_free_litexmitbuf\n"); */

	if (data_buf_q == NULL)
		return _FAIL;

	if (lite_data_buf == NULL)
		return _FAIL;

	if (lite_data_buf->sctx) {
		RTW_INFO("%s lite_data_buf->sctx is not NULL\n", __func__);
		rtw_sctx_done_err(&lite_data_buf->sctx, RTW_SCTX_DONE_BUF_FREE);
		return _FAIL;
	}

	_rtw_spinlock_irq(&free_litedatabuf_q->lock, &sp_flags);

	rtw_list_delete(&lite_data_buf->list);

	rtw_list_insert_tail(&(lite_data_buf->list),
		get_list_head(free_litedatabuf_q));

	data_buf_q->free_data_buf_cnt++;

	_rtw_spinunlock_irq(&free_litedatabuf_q->lock, &sp_flags);


	return _SUCCESS;
}

