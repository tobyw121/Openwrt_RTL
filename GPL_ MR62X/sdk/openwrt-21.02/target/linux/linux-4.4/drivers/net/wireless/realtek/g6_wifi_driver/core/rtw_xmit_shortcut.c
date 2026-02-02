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
#ifdef CONFIG_PLATFORM_RTL8198D
#ifdef CONFIG_CPU_PROFILING
#include "../profiling/cpu_perf.h"
#endif
#endif /* CONFIG_PLATFORM_RTL8198D */

#ifdef CONFIG_CORE_TXSC
u8 DBG_PRINT_MDATA_ONCE;
void _print_txreq_mdata(struct rtw_t_meta_data *mdata, const char *func)
{
		printk("[%s]\n", func);

		printk("band: %d\n", mdata->band);
		printk("type: %d\n", mdata->type);

		printk("hw_seq_mode: %d\n", mdata->hw_seq_mode);
		printk("hw_ssn_sel: %d\n", mdata->hw_ssn_sel);
		printk("smh_en: %d\n", mdata->smh_en);
		printk("hw_amsdu: %d\n", mdata->hw_amsdu);
		printk("hw_sec_iv: %d\n", mdata->hw_sec_iv);
		printk("wd_page_size: %d\n", mdata->wd_page_size);
		printk("hdr_len: %d\n", mdata->hdr_len);
		printk("dma_ch: %d\n", mdata->dma_ch);
		printk("usb_pkt_ofst: %d\n", mdata->usb_pkt_ofst);
		printk("wdinfo_en: %d\n", mdata->wdinfo_en);
		printk("wp_offset: %d\n", mdata->wp_offset);
		printk("shcut_camid: %d\n", mdata->shcut_camid);
		printk("usb_txagg_num: %d\n", mdata->usb_txagg_num);
		printk("pktlen: %d\n", mdata->pktlen);
		printk("tid: %d\n", mdata->tid);
		printk("macid: %d\n", mdata->macid);
		printk("sw_seq: %d\n", mdata->sw_seq);
		printk("ampdu_en: %d\n", mdata->ampdu_en);
		printk("bk: %d\n", mdata->bk);
		printk("mbssid: %d\n", mdata->mbssid);
		printk("hal_port: %d\n", mdata->hal_port);
		printk("data_bw_er: %d\n", mdata->data_bw_er);
		printk("dis_rts_rate_fb: %d\n", mdata->dis_rts_rate_fb);
		printk("dis_data_rate_fb: %d\n", mdata->dis_data_rate_fb);
		printk("f_ldpc: %d\n", mdata->f_ldpc);
		printk("f_stbc: %d\n", mdata->f_stbc);
		printk("f_dcm: %d\n", mdata->f_dcm);
		printk("f_er: %d\n", mdata->f_er);
		printk("f_rate: %d\n", mdata->f_rate);
		printk("f_gi_ltf: %d\n", mdata->f_gi_ltf);
		printk("f_bw: %d\n", mdata->f_bw);
		printk("userate_sel: %d\n", mdata->userate_sel);
		printk("ack_ch_info: %d\n", mdata->ack_ch_info);
		printk("max_agg_num: %d\n", mdata->max_agg_num);
		printk("nav_use_hdr: %d\n", mdata->nav_use_hdr);
		printk("bc: %d\n", mdata->bc);
		printk("mc: %d\n", mdata->mc);
		printk("a_ctrl_bqr: %d\n", mdata->a_ctrl_bqr);
		printk("a_ctrl_uph: %d\n", mdata->a_ctrl_uph);
		printk("a_ctrl_bsr: %d\n", mdata->a_ctrl_bsr);
		printk("a_ctrl_cas: %d\n", mdata->a_ctrl_cas);
		printk("data_rty_lowest_rate: %d\n", mdata->data_rty_lowest_rate);
		printk("data_tx_cnt_lmt: %d\n", mdata->data_tx_cnt_lmt);
		printk("data_tx_cnt_lmt_en: %d\n", mdata->data_tx_cnt_lmt_en);
		printk("sec_cam_idx: %d\n", mdata->sec_cam_idx);
		printk("sec_hw_enc: %d\n", mdata->sec_hw_enc);
		printk("sec_type: %d\n", mdata->sec_type);
		printk("life_time_sel: %d\n", mdata->life_time_sel);
		printk("ampdu_density: %d\n", mdata->ampdu_density);
		printk("no_ack: %d\n", mdata->no_ack);
		printk("ndpa: %d\n", mdata->ndpa);
		printk("snd_pkt_sel: %d\n", mdata->snd_pkt_sel);
		printk("sifs_tx: %d\n", mdata->sifs_tx);
		printk("rtt_en: %d\n", mdata->rtt_en);
		printk("spe_rpt: %d\n", mdata->spe_rpt);
		printk("sw_define: %d\n", mdata->sw_define);
		printk("rts_en: %d\n", mdata->rts_en);
		printk("cts2self: %d\n", mdata->cts2self);
		printk("rts_cca_mode: %d\n", mdata->rts_cca_mode);
		printk("hw_rts_en: %d\n", mdata->hw_rts_en);

}

u8 DBG_PRINT_TXREQ_ONCE;
void _print_txreq_pklist(struct xmit_frame *pxframe, struct rtw_xmit_req *ptxreq, u8 *data, const char *func)
{
	struct rtw_pkt_buf_list *pkt_list = NULL;
	struct rtw_xmit_req *txreq = NULL;
	u8 pkt_cnt = 0, i;

	printk("[%s] pxframe=%p txreq=%p\n", func, pxframe, ptxreq);

	if (data)
		txsc_dump_data(data, ETH_HLEN, "ETHHDR");

	if (ptxreq != NULL)
		txreq = ptxreq;
	else
		txreq = pxframe->phl_txreq;

	pkt_list = (struct rtw_pkt_buf_list *)txreq->pkt_list;
	pkt_cnt = txreq->pkt_cnt;

	printk("os_priv:%p, treq_type:%d, pkt_cnt:%d, total_len:%d, shortcut_id:%d\n\n",
			txreq->os_priv, txreq->treq_type, txreq->pkt_cnt, txreq->total_len, txreq->shortcut_id);

	for (i = 0; i < pkt_cnt; i++) {
		printk("pkt_list[%d]\n", i);
		txsc_dump_data(pkt_list->vir_addr, pkt_list->length, "pkt_list");
		pkt_list++;
	}

}

#ifdef CONFIG_TXSC_AMSDU
#ifdef CONFIG_AMSDU_HW_TIMER
extern void rtw_core_set_gt3(_adapter *padapter, u8 enable, long timeout);
void rtw_txsc_amsdu_xmit_tasklet(struct dvobj_priv *dvobj)
{
	_list	*phead, *plist;
	struct txsc_amsdu_swq *txq = NULL;
	struct txsc_amsdu_swq *txq_deq_list[NUM_STA];
	u8 deq_num = 0, first_timer = 1;
	int i;

	spin_lock_bh(&dvobj->amsdu_hw_timer_lock);
	phead = &dvobj->amsdu_hw_timer_list;
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		txq = LIST_CONTAINOR(plist, struct txsc_amsdu_swq, list);
		plist = get_next(plist);

		if(first_timer)
		{
			if(txq->hw_timer_cnt > 0)
				txq->hw_timer_cnt--;
			first_timer = 0;
		}
		if(deq_num >= NUM_STA)
			break;
		if(txq->hw_timer_cnt == 0)
		{
			txq_deq_list[deq_num] = txq;
			deq_num++;
			list_del_init(&txq->list);
			txq->is_hw_timer_pending = 0;
		}
		else
			break;
	}

	spin_unlock_bh(&dvobj->amsdu_hw_timer_lock);

	if(deq_num)
	{
		for(i = 0; i < deq_num; i++)
		{
			txq = txq_deq_list[i];
			txsc_amsdu_timeout_tx(txq->psta, txq->ac);
		}
	}
}

void rtw_amsdu_hw_timeout(unsigned long data)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)data;
	u8 i = 0, trigger_timer = 1;
	_adapter *padapter = dvobj_get_primary_adapter(dvobj);

	rtw_core_set_gt3(padapter, 1, 1000);
	if (RTW_CANNOT_RUN(dvobj)) {
		printk_ratelimited("%s => bDriverStopped(%s) bSurpriseRemoved(%s)\n",
			__func__, dev_is_drv_stopped(dvobj) ? "True" : "False",
			dev_is_surprise_removed(dvobj) ? "True" : "False");
		return;
	}
	
	rtw_txsc_amsdu_xmit_tasklet(dvobj);

}

static void txsc_amsdu_set_hw_timer(struct sta_info *psta, u8 ac)
{
	_adapter *padapter = psta->padapter;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct txsc_amsdu_swq *txq = NULL;
	struct txsc_amsdu_swq *cur_txq = &psta->amsdu_txq[ac];
	_list	*phead, *plist;
	u32 hw_timeout_cnt = 0;
	u8 found = 0;

	spin_lock_bh(&dvobj->amsdu_hw_timer_lock);
	phead = &dvobj->amsdu_hw_timer_list;
	plist = get_next(phead);

	if(cur_txq->is_hw_timer_pending)
	{
		spin_unlock_bh(&dvobj->amsdu_hw_timer_lock);
		return;
	}
	hw_timeout_cnt = psta->txsc_amsdu_hw_timeout;
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		txq = LIST_CONTAINOR(plist, struct txsc_amsdu_swq, list);
		plist = get_next(plist);

		if(txq == cur_txq)
		{
			spin_unlock_bh(&dvobj->amsdu_hw_timer_lock);
			return;
		}

		if(txq->hw_timer_cnt < hw_timeout_cnt)
		{
			hw_timeout_cnt -= txq->hw_timer_cnt;
		}
		else
		{
			found = 1;
			break;
		}
	}

	cur_txq->hw_timer_cnt = hw_timeout_cnt;
	if(found)
	{
		list_add_tail(&cur_txq->list, &txq->list);
		txq->hw_timer_cnt -= cur_txq->hw_timer_cnt;
	}
	else
	{
		list_add_tail(&cur_txq->list, phead);
	}

	cur_txq->is_hw_timer_pending = 1;
	spin_unlock_bh(&dvobj->amsdu_hw_timer_lock);
	return;
}

static void txsc_amsdu_cancel_hw_timer(struct sta_info *psta, u8 ac)
{
	_adapter *padapter = psta->padapter;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct txsc_amsdu_swq *txq = NULL;
	struct txsc_amsdu_swq *cur_txq = &psta->amsdu_txq[ac];
	_list	*phead, *plist;
	u8 found = 0;

	spin_lock_bh(&dvobj->amsdu_hw_timer_lock);
	phead = &dvobj->amsdu_hw_timer_list;
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		txq = LIST_CONTAINOR(plist, struct txsc_amsdu_swq, list);
		plist = get_next(plist);

		if(txq == cur_txq)
		{
			found = 1;
			break;
		}
	}

	if(found)
	{
		if(plist != phead)
		{
			txq = LIST_CONTAINOR(plist, struct txsc_amsdu_swq, list);
			txq->hw_timer_cnt += cur_txq->hw_timer_cnt;
		}
		list_del_init(&cur_txq->list);
	}

	cur_txq->hw_timer_cnt = 0;
	cur_txq->is_hw_timer_pending = 0;
	spin_unlock_bh(&dvobj->amsdu_hw_timer_lock);
	return;
}
#endif

static void txsc_amsdu_vo_timeout_handler(void *func_ontext)
{
	struct sta_info *psta = (struct sta_info *)func_ontext;
	u8 ac = 0;

	//psta->txsc_amsdu_vo_timeout_sts = TXSC_AMSDU_TIMER_TIMEOUT;

	/* do amsdu tx */
	txsc_amsdu_timeout_tx(psta, ac);
}

static void txsc_amsdu_vi_timeout_handler(void *func_ontext)
{
	struct sta_info *psta = (struct sta_info *)func_ontext;
	u8 ac = 1;

	//psta->txsc_amsdu_vi_timeout_sts = TXSC_AMSDU_TIMER_TIMEOUT;

	/* do amsdu tx */
	txsc_amsdu_timeout_tx(psta, ac);
}

static void txsc_amsdu_be_timeout_handler(void *func_ontext)
{
	struct sta_info *psta = (struct sta_info *)func_ontext;
	u8 ac = 2;

	//psta->txsc_amsdu_be_timeout_sts = TXSC_AMSDU_TIMER_TIMEOUT;
	#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
	psta->core_txsc_amsdu_timeout++;
	#endif

	/* do amsdu tx */
	txsc_amsdu_timeout_tx(psta, ac);
}

static void txsc_amsdu_bk_timeout_handler(void *func_ontext)
{
	struct sta_info *psta = (struct sta_info *)func_ontext;
	u8 ac = 3;

	//psta->txsc_amsdu_bk_timeout_sts = TXSC_AMSDU_TIMER_TIMEOUT;

	/* do amsdu tx */
	txsc_amsdu_timeout_tx(psta, ac);
}

void txsc_amsdu_queue_init(_adapter *padapter, struct sta_info *psta)
{
	struct sk_buff *pskb = NULL;
	u32 i, j;
#ifdef CONFIG_AMSDU_HW_TIMER
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
#endif

	for (i = 0; i < 4; i++) {
		psta->amsdu_txq[i].cnt = 0;
		psta->amsdu_txq[i].wptr = 0;
		psta->amsdu_txq[i].rptr = 0;
		psta->amsdu_txq[i].skb_qlen = 0; /* AMSDU_BY_SIZE */
		for (j = 0; j < MAX_AMSDU_ENQ_NUM; j++) {
			pskb = psta->amsdu_txq[i].skb_q[j];
			if (pskb)
				rtw_os_pkt_complete(padapter, pskb);
			psta->amsdu_txq[i].skb_q[j] = NULL;
		}

		/* init lock */
		_rtw_spinlock_init(&psta->amsdu_txq[i].txsc_amsdu_lock);
#ifdef CONFIG_AMSDU_HW_TIMER
		spin_lock_bh(&dvobj->amsdu_hw_timer_lock);
		psta->amsdu_txq[i].psta = psta;
		psta->amsdu_txq[i].ac = i;
		psta->amsdu_txq[i].hw_timer_cnt = 0;
		psta->amsdu_txq[i].is_hw_timer_pending = 0;
		_rtw_init_listhead(&psta->amsdu_txq[i].list);
		spin_unlock_bh(&dvobj->amsdu_hw_timer_lock);
#endif
	}

	/* init timer */
	rtw_init_timer(&(psta->txsc_amsdu_vo_timer), txsc_amsdu_vo_timeout_handler, psta);
	psta->txsc_amsdu_vo_timeout_sts = TXSC_AMSDU_TIMER_UNSET;

	rtw_init_timer(&(psta->txsc_amsdu_vi_timer), txsc_amsdu_vi_timeout_handler, psta);
	psta->txsc_amsdu_vi_timeout_sts = TXSC_AMSDU_TIMER_UNSET;

	rtw_init_timer(&(psta->txsc_amsdu_be_timer), txsc_amsdu_be_timeout_handler, psta);
	psta->txsc_amsdu_be_timeout_sts = TXSC_AMSDU_TIMER_UNSET;

	rtw_init_timer(&(psta->txsc_amsdu_bk_timer), txsc_amsdu_bk_timeout_handler, psta);
	psta->txsc_amsdu_bk_timeout_sts = TXSC_AMSDU_TIMER_UNSET;

	psta->txsc_amsdu_num = padapter->tx_amsdu;
}

void txsc_amsdu_queue_free(_adapter *padapter, struct sta_info *psta, u8 reset)
{
	struct sk_buff *pskb = NULL;
	struct txsc_amsdu_swq *txq = NULL;
	u32 i, j;
#ifdef CONFIG_AMSDU_HW_TIMER
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	_adapter *pri_adapter = dvobj_get_primary_adapter(dvobj);
#endif

	for (i = 0; i < 4; i++) {
		txq = &psta->amsdu_txq[i];
		txq->cnt = 0;
		txq->wptr = 0;
		txq->rptr = 0;
		txq->skb_qlen = 0;/* AMSDU_BY_SIZE */

#ifdef CONFIG_AMSDU_HW_TIMER
		if(pri_adapter->amsdu_hw_timer_enable && txq->is_hw_timer_pending)
			txsc_amsdu_cancel_hw_timer(psta, i);
#endif
		_rtw_spinlock_bh(&txq->txsc_amsdu_lock);
		for (j = 0; j < MAX_AMSDU_ENQ_NUM; j++) {
			pskb = txq->skb_q[j];
			if (pskb != NULL) {
				txq->skb_q[j] = NULL;
#ifdef CONFIG_VW_REFINE
				if (padapter->dvobj->tx_mode == 1) {
					padapter->swq_amsdu_cnt--;
					if ( pskb->cb[_SKB_CB_AMSDU_TXSC] != 0)  {
					     padapter->swq_amsdu_cnt -= (pskb->cb[_SKB_CB_AMSDU_TXSC] & 0x0F);
					     pskb->cb[_SKB_CB_AMSDU_TXSC] = 0;
					}
				}
#endif
				rtw_os_pkt_complete(padapter, pskb);
			}
		}
		_rtw_spinunlock_bh(&txq->txsc_amsdu_lock);

		if (!reset)
			_rtw_spinlock_free(&txq->txsc_amsdu_lock);
	}

	/* cancel timer */
	_cancel_timer_ex(&psta->txsc_amsdu_vo_timer);
	_cancel_timer_ex(&psta->txsc_amsdu_vi_timer);
	_cancel_timer_ex(&psta->txsc_amsdu_be_timer);
	_cancel_timer_ex(&psta->txsc_amsdu_bk_timer);
}

static void txsc_amsdu_set_timer(struct sta_info *psta, u8 ac)
{
	_timer *txsc_amsdu_timer = NULL;

	switch (ac) {
	case 0:
		txsc_amsdu_timer = &psta->txsc_amsdu_vo_timer;
		break;

	case 1:
		txsc_amsdu_timer = &psta->txsc_amsdu_vi_timer;
		break;

	case 2:
		txsc_amsdu_timer = &psta->txsc_amsdu_be_timer;
		break;

	case 3:
		txsc_amsdu_timer = &psta->txsc_amsdu_bk_timer;
		break;

	default:
		txsc_amsdu_timer = &psta->txsc_amsdu_be_timer;
		break;
	}

	_set_timer(txsc_amsdu_timer, 1);
}

static u8 txsc_amsdu_get_timer_status(struct sta_info *psta, u8 ac)
{
	u8 status = TXSC_AMSDU_TIMER_UNSET;

	switch (ac) {
	case 0:
		status = psta->txsc_amsdu_vo_timeout_sts;
		break;

	case 1:
		status = psta->txsc_amsdu_vi_timeout_sts;
		break;

	case 2:
		status = psta->txsc_amsdu_be_timeout_sts;
		break;

	case 3:
		status = psta->txsc_amsdu_bk_timeout_sts;
		break;

	default:
		status = psta->txsc_amsdu_be_timeout_sts;
		break;
	}

	return status;
}

static void txsc_amsdu_set_timer_status(struct sta_info *psta, u8 ac, u8 status)
{
	switch (ac) {
	case 0:
		psta->txsc_amsdu_vo_timeout_sts = status;
		break;

	case 1:
		psta->txsc_amsdu_vi_timeout_sts = status;
		break;

	case 2:
		psta->txsc_amsdu_be_timeout_sts = status;
		break;

	case 3:
		psta->txsc_amsdu_bk_timeout_sts = status;
		break;

	default:
		psta->txsc_amsdu_be_timeout_sts = status;
		break;
	}
}

__IMEM_WLAN_SECTION__
static u8 _up_to_qid(u8 up)
{
	u8 ac = 0;

	switch (up) {

	case 1:
	case 2:
		ac = 3;/* bk */
		break;

	case 4:
	case 5:
		ac = 1;/* vi */
		break;

	case 6:
	case 7:
		ac = 0;/* vo */
		break;

	case 0:
	case 3:
	default:
		ac = 2;/* be */
		break;
	}

	return ac;
}

#ifdef CONFIG_VW_REFINE
static inline void set_timeout_by_tp(_adapter *padapter, u8 mode)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	int sta_nr = pstapriv->asoc_sta_count - 1; /* exclude AP itself for ap mode */
	u16 hw_timer = 1000;

#ifdef CONFIG_SWQ_SKB_ARRAY
	if (1 == mode) {
		if (sta_nr > 20) {
			padapter->swq_timeout = 50;
			hw_timer = 2000;
		} else if (sta_nr > 10) {
			padapter->swq_timeout = 20;
		} else if (sta_nr > 5)
			padapter->swq_timeout = 10;
		else
			padapter->swq_timeout = 5;
	} else if (2 == mode) {
		if (sta_nr > 20) {
			padapter->swq_timeout = 30;
			hw_timer = 2000;
		} else if (sta_nr > 10) {
			padapter->swq_timeout = 20;
		} else if (sta_nr > 5)
			padapter->swq_timeout = 10;
		else
			padapter->swq_timeout = 5;
	} else {
		if (sta_nr > 20) {
			padapter->swq_timeout = 25;
			hw_timer = 2000;
		} else if (sta_nr > 10) {
			padapter->swq_timeout = 10;
		} else if (sta_nr > 5)
			padapter->swq_timeout = 5;
		else
			padapter->swq_timeout = 5;
	}
#endif

	if (0 != padapter->set_swq_timeout)
		padapter->swq_timeout = padapter->set_swq_timeout;

	if (0 != padapter->set_hw_swq_timeout)
		padapter->hw_swq_timeout = padapter->set_hw_swq_timeout;
	else
		padapter->hw_swq_timeout = hw_timer;
}

static inline void vw_latency_swq_set(_adapter *adapter, int pkt_len)
{
	if ( pkt_len > 1000 ) {
		adapter->tx_amsdu = 1;
		adapter->sta_deq_len = 20;
		adapter->txForce_aggnum = 19;
	} else if ( pkt_len > 500 ) {
		adapter->tx_amsdu = 2;
		adapter->sta_deq_len = 128;
		adapter->txForce_aggnum = 31;
	} else {
		adapter->tx_amsdu = 2;
		adapter->sta_deq_len = 128;
		adapter->txForce_aggnum = 47;
	}
}

static inline void txsc_apply_amsdu_setting(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt)
{
	struct sk_buff *pskb = txsc_pkt->xmit_skb[0];
	u8 tx_amsdu_big, tx_amsdu_mid, tx_amsdu_small;

	if (1 == padapter->vw_enable) {
		if (WIFI_ROLE_IS_ON_5G(padapter)) {
#if defined(RTW_VW_TXAMSDU_ENHANCE) && (MAX_TXSC_SKB_NUM == 9)
			if(padapter->tc_enable) {	// set specific tx_amsdu when under testcenter test
				tx_amsdu_big = 6;
				tx_amsdu_mid = 9;
				tx_amsdu_small = 9;
			}
			else {						// set specific tx_amsdu when under vw test
				tx_amsdu_big = 6;
				tx_amsdu_mid = 8;
				tx_amsdu_small = 8;
			}
#else
			tx_amsdu_big = 4;			// normal vw & tc test
			tx_amsdu_mid = 6;
			tx_amsdu_small = 6;
#endif

			if(padapter->manual_tx_amsdu[0]) {		// set tx_amsdu by command
				tx_amsdu_big = padapter->manual_tx_amsdu[1];
				tx_amsdu_mid = padapter->manual_tx_amsdu[2];
				tx_amsdu_small = padapter->manual_tx_amsdu[3];
			}

			 if ((pskb->len > 1000) && (padapter->big_pkt > padapter->small_pkt)) {
				padapter->tx_amsdu = tx_amsdu_big;
				padapter->max_enq_len = 28000;
				padapter->sta_deq_len = 63;
				padapter->tx_lmt =  256;
				padapter->max_deq_len = 5120;
				set_timeout_by_tp(padapter, 1);
			} else if ((pskb->len > 500) && (padapter->small_pkt > padapter->big_pkt)) {
				padapter->tx_amsdu = tx_amsdu_mid;
				padapter->max_enq_len = 22000;
				padapter->sta_deq_len = 63;
				padapter->tx_lmt = 384;
				padapter->max_deq_len = 3840;
				set_timeout_by_tp(padapter, 2);
			} else if (padapter->small_pkt > padapter->big_pkt) {
				padapter->tx_amsdu = tx_amsdu_small;
				padapter->max_enq_len = 13000;
				padapter->sta_deq_len = 63;
				padapter->tx_lmt = 384;
				padapter->max_deq_len = 3840;
				set_timeout_by_tp(padapter, 3);
			}
		} else {
			tx_amsdu_big = 2;		// normal vw & tc test
			tx_amsdu_mid = 6;
			tx_amsdu_small = 6;

			if(padapter->manual_tx_amsdu[0]) {		// set tx_amsdu by command
				tx_amsdu_big = padapter->manual_tx_amsdu[1];
				tx_amsdu_mid = padapter->manual_tx_amsdu[2];
				tx_amsdu_small = padapter->manual_tx_amsdu[3];
			}

			if ((pskb->len > 1000) && (padapter->big_pkt > padapter->small_pkt)) {
				padapter->tx_amsdu = tx_amsdu_big;
				padapter->max_enq_len = 8192;
			} else if ((pskb->len > 500) && (padapter->small_pkt > padapter->big_pkt)) {
				padapter->tx_amsdu = tx_amsdu_mid;
				padapter->max_enq_len = 8192;
			} else if (padapter->small_pkt > padapter->big_pkt) {
				padapter->tx_amsdu = tx_amsdu_small;
				padapter->max_enq_len = 13000;
			}
			padapter->tx_lmt = 128;
		}
		padapter->current_tx_amsdu[0] = tx_amsdu_big;
		padapter->current_tx_amsdu[1] = tx_amsdu_mid;
		padapter->current_tx_amsdu[2] = tx_amsdu_small;
	} else if (2 == padapter->vw_enable) {
		vw_latency_swq_set(padapter, pskb->len);
	}
}

static inline s8 amsdu_merge(struct sk_buff *h_skb, struct sk_buff *n_skb, bool add_pad)
{
	void *data;
	int pad;
	__be16 len;
	const int headroom = SNAP_SIZE + 2;
	u16 protocol;

	if (!n_skb || !h_skb)
		return _FAIL;

	/* fixbug 756995, tail room for h_skb should be bigger more than (n_skb->len + 12)
	 * because ieee8023_header_to_rfc1042_txsc will add snap header (8 bytes) to n_skb.
	 */
	if (skb_tailroom(h_skb) > (n_skb->len + 12)) {
		ieee8023_header_to_rfc1042_txsc(n_skb, add_pad);
		memcpy(skb_put(h_skb, n_skb->len), n_skb->data, n_skb->len);
		rtw_skb_free(n_skb);
		return _SUCCESS;
	} else {
		printk_ratelimited("amsdu merge : no tail room:%d need:%d\n", skb_tailroom(h_skb), n_skb->len);
		return _FAIL;
	}
}

static inline u8 start_amsdu_merge(_adapter *padapter, u8 cur, struct sk_buff *pskb_first)
{
	struct sk_buff *pskb = NULL;
	u8 i =0, j=0 ;
	s8 mrg_sts = 0;
	u8 merge_cnt = padapter->amsdu_merge_cnt;

	i = 1;
	if (0x10 & pskb_first->cb[_SKB_VW_FLAG])
		j = 1;

	ieee8023_header_to_rfc1042_txsc(pskb_first, false);

	while (merge_cnt > 0) {
		pskb = vw_skb_deq(padapter, cur);
		if (pskb == NULL)
			break;

		/* fixbug 756995, tail room for pskb_first should be bigger more than (pskb->len + 16)
		 * because both pskb_first and pskb will do 4 bytes aligned and pskb will add snap header (8 bytes).
		 */
		if ((pskb->len > 128) || (skb_headroom(pskb_first) < 8) ||
			(skb_tailroom(pskb_first) < (pskb->len + 16))) {
#ifdef CONFIG_SWQ_SKB_ARRAY
			vw_skb_enq(padapter, cur, pskb);
#else
			skb_queue_head(&padapter->skb_xmit_queue[cur], pskb);
			ATOMIC_INC(&padapter->skb_xmit_queue_len);
#endif
			break;
		}

		if (pskb_first->len & 0x3)
			skb_put(pskb_first, 4 - (pskb_first->len & 0x3));

		mrg_sts = amsdu_merge(pskb_first, pskb, false);
		if (_FAIL == mrg_sts) {
			rtw_skb_free(pskb);
			break;
		}

		padapter->tx_logs.core_vw_test5++;
		merge_cnt--;

		i++;
		if (0x10 & pskb->cb[_SKB_VW_FLAG])
			j++;
	}

	pskb_first->cb[_SKB_CB_AMSDU_TXSC] = (0x80 | i);
	pskb_first->cb[_SKB_VW_FLAG] = (0x10 | j);

	return j;
}

static u8 txsc_check_sc(_adapter *padapter, struct sk_buff *pskb, struct sta_info *psta)
{
	struct sta_priv	*pstapriv = &padapter->stapriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	u8 *ptxsc_ethdr = NULL;
	u8 i, res, da[6], offset, sta_hit = 0;
	u8 idx, comp_len, is_urgent = 0;
        enum txsc_action_type step = TXSC_NONE;

	res = _FAIL;
	offset = ETH_ALEN;

	if (psta->txsc_cache_num == 0) {
		res = _FAIL;
		goto exit;
	}

#ifdef CONFIG_VW_REFINE
	/* txsc_ethdr is amsdu format, so only compare source address */
	if (padapter->dvobj->tx_mode == 1)
		comp_len = ETH_ALEN;
	else
#endif
		comp_len = ETH_HLEN - offset;

	for (i = 0, idx = psta->txsc_cache_idx; i < CORE_TXSC_ENTRY_NUM; i++) {
		ptxsc_ethdr = (u8 *)&psta->txsc_entry_cache[idx].txsc_ethdr;

		if (psta->txsc_entry_cache[idx].txsc_is_used
			&& _rtw_memcmp((pskb->data + offset), (ptxsc_ethdr + offset), comp_len)
			#if	!defined(CONFIG_VW_REFINE) && !defined(CONFIG_ONE_TXQ)
			&& (pskb->len <= psta->txsc_entry_cache[idx].txsc_frag_len)
			#endif
			) {

			#ifdef CONFIG_TXSC_AMSDU
			if ( ( pxmitpriv->txsc_amsdu_enable &&
			      psta->txsc_entry_cache[idx].txsc_amsdu  &&
			      psta->txsc_amsdu_num > 0 ) || padapter->vw_enable )
			        step = TXSC_AMSDU_APPLY;
			else
			#endif
				step = TXSC_APPLY;

			//#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
			//padapter->core_txsc_macid[psta->phl_sta->macid]++;
			//#endif

			res = _SUCCESS;
			break;
		}

		idx = (idx + 1) % CORE_TXSC_ENTRY_NUM;
	}

	/* skip power saving mode */
	if (res == _SUCCESS && (psta->state & WIFI_SLEEP_STATE)) {
		#if 0/* _VW_ */
		if (!txsc_pkt->amsdu) {
			res = _FAIL;
			txsc_pkt->step = TXSC_SKIP;
		}
		#endif
		psta->txsc_path_ps++;
	}

exit:
#ifdef CONFIG_VW_REFINE
        if ( 0 != pskb->cb[_SKB_VW_FLAG] ) {
	   if ( res == _SUCCESS )
      	        DBG_COUNTER(padapter->tx_logs.core_vw_txsc);
           else
                DBG_COUNTER(padapter->tx_logs.core_vw_txsc_fail);
        }
#endif

	return res;
}

u8 txsc_amsdu_dequeue_swq(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt)
{
	struct sta_info *psta = txsc_pkt->psta;
	u16 qlen, qidx;
	u8 deq_cnt, i = 0, vw_cnt = 0;
	u8 small_pkt_merge = 0;
	u8 skb_cnt = 0;

	txsc_apply_amsdu_setting(padapter, txsc_pkt);

	qidx = psta->phl_sta->macid % MAX_SKB_XMIT_QUEUE;

	qlen = vw_skb_len(padapter, qidx);

	deq_cnt = (qlen > padapter->tx_amsdu) ? padapter->tx_amsdu : qlen;
	if (deq_cnt > 0)
		deq_cnt = deq_cnt - 1;
	if (deq_cnt >= MAX_TXSC_SKB_NUM)
		deq_cnt = MAX_TXSC_SKB_NUM - 1;

	if ((txsc_pkt->xmit_skb[0]->len < 256) &&
		(padapter->amsdu_merge_cnt > 0) &&
		(padapter->small_pkt > padapter->big_pkt))
		small_pkt_merge = 1;

	do {
		/* xmit_skb[0] has filled */
		if (i > 0) {
			txsc_pkt->xmit_skb[i] = vw_skb_deq(padapter, qidx);
			if (txsc_pkt->xmit_skb[i] == NULL) {
				DBG_COUNTER(padapter->tx_logs.core_vw_amsdu_dnq2);
				break;
			}
		}

		skb_cnt++;

		if (small_pkt_merge) {
			u8 cnt = start_amsdu_merge(padapter, qidx, txsc_pkt->xmit_skb[i]);
			vw_cnt += cnt;
			DBG_COUNTER_NUM(padapter->tx_logs.core_vw_test5, cnt);
		} else {
			ieee8023_header_to_rfc1042_txsc(txsc_pkt->xmit_skb[i], false);
			if (0x10 & txsc_pkt->xmit_skb[i]->cb[_SKB_VW_FLAG]) {
				txsc_pkt->xmit_skb[i]->cb[_SKB_VW_FLAG] = 0x11;
				vw_cnt++;
				DBG_COUNTER(padapter->tx_logs.core_vw_test5);
			}

			txsc_pkt->xmit_skb[i]->cb[_SKB_CB_AMSDU_TXSC] = (0x81);
		}
	} while (++i <= deq_cnt);

	txsc_pkt->priority = txsc_pkt->xmit_skb[0]->cb[_SKB_CB_PRIORITY];
	txsc_pkt->skb_cnt = skb_cnt;
	return skb_cnt;
}

u8 txsc_amsdu_dequeue_swq1(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt, u8 from)
{
	u8 ac = txsc_pkt->ac;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_info *psta = txsc_pkt->psta;
	struct txsc_amsdu_swq *txq = &psta->amsdu_txq[ac];
	u8 deq_cnt= 0, deq_cnt1= 0, vw_cnt =0, i, timer_sts;
	u8 amsdu_num = 0;

    u32 amsdu_merge_cnt = padapter->amsdu_merge_cnt;
   	u32 my_deq_cnt = 0, my_meg_cnt;

	_rtw_spinlock_bh(&txq->txsc_amsdu_lock);

#ifdef CONFIG_VW_REFINE
	if ( padapter->vw_enable) {
            amsdu_num = padapter->tx_amsdu;
    } else
#endif
	{
	    amsdu_num = psta->txsc_amsdu_num;
	    if(amsdu_num == 0)
			amsdu_num = padapter->tx_amsdu;
     }

    if (txq->cnt < amsdu_num)
	   	amsdu_num = txq->cnt;

	if( amsdu_num > 0 ) {
		for(i = 0; i < amsdu_num; i++) {
			txsc_pkt->xmit_skb[i] = txq->skb_q[txq->rptr];
			if (txsc_pkt->xmit_skb[i] == NULL) {
				RTW_PRINT("[%s:%d] fetal err, plz check.\n", __func__, __LINE__);
				break;
			}
			txq->skb_q[txq->rptr] = NULL;
			txq->rptr = (txq->rptr + 1) % MAX_AMSDU_ENQ_NUM;
                        txq->cnt--;

		txq->skb_qlen -= txsc_pkt->xmit_skb[i]->len; /* SIZE_BY_AMSDU */

#ifdef CONFIG_VW_REFINE
			//deq_cnt1++;

            if ( txsc_pkt->xmit_skb[i]->cb[_SKB_VW_FLAG] & 0x10 ) {
			      // vw_cnt++;
			     vw_cnt += (txsc_pkt->xmit_skb[i]->cb[_SKB_VW_FLAG] & 0x0F);
			}

			if (txsc_pkt->xmit_skb[i]->cb[_SKB_CB_AMSDU_TXSC] != 0 )
			    deq_cnt1 += ( txsc_pkt->xmit_skb[i]->cb[_SKB_CB_AMSDU_TXSC] & 0x0F);
#endif
			//RTW_PRINT("[%s] txsc_pkt->xmit_skb[%d]: %p\n", __FUNCTION__, i, txsc_pkt->xmit_skb[i]);
			deq_cnt++;
		}
		//RTW_PRINT("[%s] amsdu_timeout: deq_cnt=%d\n", __FUNCTION__, deq_cnt);

		pxmitpriv->cnt_txsc_amsdu_timeout_dump[deq_cnt]++;
	} else {
			;//RTW_PRINT("[TXSC][AMSDU] no timeout and txq->cnt:%d\n", txq->cnt);
	}

	//update priority
	if(deq_cnt > 0) {
		txsc_pkt->priority = txsc_pkt->xmit_skb[0]->cb[_SKB_CB_PRIORITY];

#ifdef CONFIG_VW_REFINE
            if ( 0 != txsc_pkt->xmit_skb[0]->cb[_SKB_VW_FLAG] ) {
				struct rtw_xmit_req *txreq = txsc_pkt->ptxreq;

                DBG_COUNTER_NUM(padapter->tx_logs.core_vw_amsdu_dnq, vw_cnt);
                if ( 1 == from ) {
                    DBG_COUNTER_NUM(padapter->tx_logs.core_vw_amsdu_timeout, vw_cnt);
		}
            }
#endif
	}

#ifdef CONFIG_VW_REFINE
	padapter->swq_amsdu_cnt -= deq_cnt1;
	pxmitpriv->cnt_txsc_amsdu_deq[ac] += deq_cnt1;
#else
	timer_sts = txsc_amsdu_get_timer_status(psta, ac);
	if(txq->cnt > 0 && timer_sts != TXSC_AMSDU_TIMER_SETTING) {
		txsc_amsdu_set_timer(psta, ac);
		txsc_amsdu_set_timer_status(psta, ac, TXSC_AMSDU_TIMER_SETTING);
	}
        padapter->cnt_txsc_amsdu_deq[ac] += deq_cnt;
#endif

	txsc_pkt->skb_cnt = deq_cnt;

	_rtw_spinunlock_bh(&txq->txsc_amsdu_lock);

	return deq_cnt;
}
#endif /* CONFIG_VW_REFINE */

inline u16 _calc_avail_rptr(u16 rptr, u16 wptr, u16 bndy)
{
	u16 avail_rptr = 0;

	if (wptr >= rptr)
		avail_rptr = wptr - rptr;
	else if (rptr > wptr)
		avail_rptr = wptr + (bndy - rptr);

	return avail_rptr;
}

void txsc_amsdu_process_check(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt)
{
	struct sta_info *psta = txsc_pkt->psta;
	struct txsc_amsdu_swq *txq = NULL;

	if (!psta)
		return;

	txq = &psta->amsdu_txq[txsc_pkt->ac];

	_txsc_spinlock_bh(&txq->txsc_amsdu_lock);
	if (txq->is_normal_deq == 1)
		txq->is_normal_deq = 0;
	_txsc_spinunlock_bh(&txq->txsc_amsdu_lock);
}

static u8 txsc_amsdu_dequeue(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_info *psta = txsc_pkt->psta;
	struct txsc_amsdu_swq *txq = NULL;
	u8 deq_cnt = 0, i, timer_sts, big_pkt_cnt = 0;
	u8 ac = txsc_pkt->ac;
	u8 amsdu_num = 0;
	u32 amsdu_size = 0;
	u8 amsdu_force_num = pxmitpriv->txsc_amsdu_force_num;
	u32 amsdu_deq_size = 0;/* AMSDU_BY_SIZE */
	u16 avail = 0;
#ifdef CONFIG_AMSDU_HW_TIMER
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	_adapter *pri_adapter = dvobj_get_primary_adapter(dvobj);
#endif

	if (!psta)
		return deq_cnt;

	txq = &psta->amsdu_txq[ac];

	_txsc_spinlock_bh(&txq->txsc_amsdu_lock);

	if (txsc_pkt->is_amsdu_timeout && txq->is_normal_deq) {
		pxmitpriv->cnt_txsc_amsdu_timeout_abort[ac]++;
		goto abort_deq;
	}

	/* _VW_ */
	if (psta->state & WIFI_SLEEP_STATE) {
		pxmitpriv->cnt_txsc_amsdu_deq_ps++;
		//goto abort_deq;
	}

	amsdu_num = psta->txsc_amsdu_num;
	if (amsdu_num == 0)
		amsdu_num = padapter->tx_amsdu;

	/* for amsdu timout */
	if (txsc_pkt->is_amsdu_timeout && txq->cnt < amsdu_num)
		amsdu_num = txq->cnt;

	amsdu_size = psta->txsc_amsdu_size;
	if (amsdu_size == 0)
		amsdu_size = AMSDU_0_SIZE - CORE_TXSC_WLHDR_SIZE;

	/* AMSDU_BY_SIZE */
	if (pxmitpriv->txsc_amsdu_mode == 1) {
		for (i = 0; i < MAX_TXSC_SKB_NUM; i++) {
			
			avail = _calc_avail_rptr(txq->rptr, txq->wptr, MAX_AMSDU_ENQ_NUM);
			if (!avail)
				break;
			
			if (txq->skb_q[txq->rptr] == NULL){
				RTW_ERR("[%s:%d] out of bounds, [amsdu] ac[%d] cnt:%d, rptr:%d, wptr:%d, qlen: %d!\n",
					__func__, __LINE__, ac, txq->cnt, txq->rptr, txq->wptr, txq->skb_qlen);
				break;
			}


			if((amsdu_deq_size + txq->skb_q[txq->rptr]->len) > amsdu_size)
				break;

			if (big_pkt_cnt == amsdu_num && txq->skb_q[txq->rptr]->len > 1000)
				break;

			/* for amsdu force num */
			if (amsdu_force_num && deq_cnt >= amsdu_force_num)
				break;

			txsc_pkt->xmit_skb[i] = txq->skb_q[txq->rptr];
			txq->skb_q[txq->rptr] = NULL;
			smp_wmb();
			txq->rptr = (txq->rptr + 1) % MAX_AMSDU_ENQ_NUM;
			txq->cnt--;

			/* count dequeue size */
			txq->skb_qlen -= txsc_pkt->xmit_skb[i]->len;

			amsdu_deq_size += txsc_pkt->xmit_skb[i]->len;

			deq_cnt++;

			#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
			psta->core_txsc_amsdu_deq++;
			#endif

			if (txsc_pkt->xmit_skb[i]->len > 1000)
				big_pkt_cnt++;
		}
	} else {
		for (i = 0; i < amsdu_num; i++) {
			avail = _calc_avail_rptr(txq->rptr, txq->wptr, MAX_AMSDU_ENQ_NUM);
			if (!avail)
				break;

			if (txq->skb_q[txq->rptr] == NULL) {
				RTW_ERR("[%s:%d] out of bounds, [amsdu] ac[%d] cnt:%d, rptr:%d, wptr:%d, qlen: %d!\n",
					__func__, __LINE__, ac, txq->cnt, txq->rptr, txq->wptr, txq->skb_qlen);
				break;
			}

			txsc_pkt->xmit_skb[i] = txq->skb_q[txq->rptr];
			txq->skb_q[txq->rptr] = NULL;
			smp_wmb();
			txq->rptr = (txq->rptr + 1) % MAX_AMSDU_ENQ_NUM;
			txq->cnt--;

			txq->skb_qlen -= txsc_pkt->xmit_skb[i]->len;

			deq_cnt++;

			#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
			psta->core_txsc_amsdu_deq++;
			#endif
		}
	}

	/* update priority when timeout */
    if (deq_cnt > 0)
	    txsc_pkt->priority = txsc_pkt->xmit_skb[0]->cb[_SKB_CB_PRIORITY];

abort_deq:

#ifdef CONFIG_AMSDU_HW_TIMER
	if(pri_adapter->amsdu_hw_timer_enable)
	{
		if(txq->is_hw_timer_pending)
			txsc_amsdu_cancel_hw_timer(psta, ac);
		if(txq->cnt > 0)
			txsc_amsdu_set_hw_timer(psta, ac);
	}
	else
#endif
	{
		timer_sts = txsc_amsdu_get_timer_status(psta, ac);
		if (txq->cnt > 0 && timer_sts != TXSC_AMSDU_TIMER_SETTING) {
			txsc_amsdu_set_timer(psta, ac);
			txsc_amsdu_set_timer_status(psta, ac, TXSC_AMSDU_TIMER_SETTING);
		}
	}

	pxmitpriv->cnt_txsc_amsdu_deq[ac] += deq_cnt;

	txsc_pkt->skb_cnt = deq_cnt;

	_txsc_spinunlock_bh(&txq->txsc_amsdu_lock);

	return deq_cnt;
}

#ifdef CONFIG_BW160M_EXTREME_THROUGHPUT_RX
u8 txsc_amsdu_deque_check(struct xmit_priv *pxmitpriv, struct sta_info *psta, u8 ac)
{
	u8 deq_again;
	struct txsc_amsdu_swq *txq = NULL;
	u8 amsdu_num = psta->txsc_amsdu_num;

	deq_again = 1;
	txq = &psta->amsdu_txq[ac];
	if ((!psta->extreme_rx_traffic) ||
		(pxmitpriv->txsc_amsdu_mode == 1 && ((txq->skb_qlen + txq->cnt * 8) < psta->txsc_amsdu_size)  && txq->cnt < MAX_TXSC_SKB_NUM) ||
		(txq->cnt < amsdu_num))
		deq_again = 0;

	return deq_again;

}

__IMEM_WLAN_SECTION__
int tcp_ack_check(struct sk_buff *skb, int *ip_ofst)
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned long seq;

	if (*(unsigned short *)(skb->data + ETH_ALEN * 2) != __constant_htons(ETH_P_IP))
		return false;

	iph = (struct iphdr *)(skb->data + ETH_HLEN);
	if (iph->protocol != IPPROTO_TCP) //IPPROTO_TCP 6
		return false;

	tcph = (struct tcphdr *)((unsigned long)iph + (iph->ihl << 2));
	if (iph->ihl != 5)
		return false;
	if (!tcph->ack || tcph->syn)
		return false;
	if ((tcph->doff << 2) + (iph->ihl << 2) != ntohs(iph->tot_len))
		return false;

	seq = ntohl(tcph->ack_seq);
	if (seq == 1)
		return false;

	*ip_ofst = ETH_HLEN + offsetof(struct iphdr, saddr);
	return true;
}

__IMEM_WLAN_SECTION__
void tcp_ack_acc(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_info *psta = txsc_pkt->psta;
	struct txsc_amsdu_swq *txq = &psta->amsdu_txq[txsc_pkt->ac];
	struct sk_buff *pskb = txsc_pkt->xmit_skb[0];
	struct sk_buff *pskb_prev = NULL;
	int ip_ofst;

	/* in rx throughput mode, check if previous tx packet is also tcp ack,
		if yes, free previous tcp ack and use current one */
	if (psta->extreme_rx_traffic) {
		if (txq->cnt && tcp_ack_check(pskb, &ip_ofst)) {
			pskb_prev = txq->skb_q[(txq->wptr-1) % MAX_AMSDU_ENQ_NUM];
			if (memcmp(&pskb->data[ip_ofst], &pskb_prev->data[ip_ofst], 12) == 0) {
				txq->wptr = (txq->wptr - 1) % MAX_AMSDU_ENQ_NUM;
				txq->cnt--;
				txq->skb_qlen -= pskb_prev->len;
				pxmitpriv->cnt_txsc_amsdu_enq[txsc_pkt->ac]--;
				rtw_skb_free(pskb_prev);
			}
		}
	}
}
#endif

inline u16 _calc_avail_wptr(u16 rptr, u16 wptr, u16 bndy)
{
	u16 avail_wptr = 0;

	if (rptr > wptr)
		avail_wptr = rptr - wptr - 1;
	else if (wptr >= rptr)
		avail_wptr = rptr + (bndy - wptr) - 1;

	return avail_wptr;
}

__IMEM_WLAN_SECTION__
u8 txsc_amsdu_enqueue(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt, u8 *status)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_info *psta = txsc_pkt->psta;
	struct sk_buff *pskb = txsc_pkt->xmit_skb[0];
	struct txsc_amsdu_swq *txq = NULL;
	u8 i, ac = 0, res = _SUCCESS, timer_sts;
	u8 amsdu_num = 0;
	u16 avail = 0;
#ifdef CONFIG_AMSDU_HW_TIMER
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	_adapter *pri_adapter = dvobj_get_primary_adapter(dvobj);
#endif

	if (!pxmitpriv->txsc_amsdu_enable || !pskb || !psta)
		return _FAIL;

#ifdef CONFIG_ETHER_PKT_AGG
	if(txsc_pkt->isAggPkt) {
		return _FAIL; // agg pkt don't do amsdu enq
	}
#endif /* CONFIG_ETHER_PKT_AGG */
	//if (txsc_pkt->step != TXSC_AMSDU_APPLY)
	//	return _FAIL;

	{
		 amsdu_num = psta->txsc_amsdu_num;
		 if(amsdu_num == 0)
			 amsdu_num = padapter->tx_amsdu;
	}

	/* get tx amsdu swq */
	ac = (txsc_pkt->ac < 4) ? txsc_pkt->ac : 2; /* default BE */
	txq = &psta->amsdu_txq[ac];

	_txsc_spinlock_bh(&txq->txsc_amsdu_lock);

	avail = _calc_avail_wptr(txq->rptr, txq->wptr, MAX_AMSDU_ENQ_NUM);
	if (avail) {

		#ifdef CONFIG_BW160M_EXTREME_THROUGHPUT_RX
		tcp_ack_acc(padapter, txsc_pkt);
		#endif

		txq->skb_q[txq->wptr] = pskb;
		smp_wmb();
		txq->wptr = (txq->wptr + 1) % MAX_AMSDU_ENQ_NUM;
		txq->cnt++;
		txq->skb_qlen += pskb->len;/* AMSDU_BY_SIZE */

		pxmitpriv->cnt_txsc_amsdu_enq[ac]++;

		txsc_pkt->xmit_skb[0] = NULL; /* _VW_ */

	#ifdef CONFIG_BW160M_EXTREME_THROUGHPUT_RX
		if (psta->extreme_rx_traffic) {
			*status = TXSC_AMSDU_ENQ_SUCCESS;
		} else
	#endif
		{
			if (psta->txsc_amsdu_num == 0)
				; /* break, flush remaining frames in TXQ */
			/* if sta in ps mode, keep queuing and nit deq */
			/* AMSDU_BY_SIZE */
			else if (pxmitpriv->txsc_amsdu_mode == 1 &&
				((txq->skb_qlen + txq->cnt * 8) < psta->txsc_amsdu_size) &&
				txq->cnt < MAX_TXSC_SKB_NUM)
				*status = TXSC_AMSDU_ENQ_SUCCESS;
			else if (txq->cnt < amsdu_num)
				*status = TXSC_AMSDU_ENQ_SUCCESS;
		}
		if (psta->state & WIFI_SLEEP_STATE)
			pxmitpriv->cnt_txsc_amsdu_enq_ps++;
	} else {
		pxmitpriv->cnt_txsc_amsdu_enq_abort[ac]++;
		*status = TXSC_AMSDU_ENQ_ABORT;
		txsc_pkt->amsdu = false;
	}

	/* set amsdu timer */
#if defined(CONFIG_VW_REFINE)
	if (padapter->dvobj->tx_mode == 0)
#endif
	{
#ifdef CONFIG_AMSDU_HW_TIMER
		if(pri_adapter->amsdu_hw_timer_enable)
		{
			if(txq->cnt > 0 && !txq->is_hw_timer_pending)
				txsc_amsdu_set_hw_timer(psta, ac);
		}
		else
#endif
		{
			timer_sts = txsc_amsdu_get_timer_status(psta, ac);
			if (txq->cnt > 0 && timer_sts != TXSC_AMSDU_TIMER_SETTING) {
				txsc_amsdu_set_timer(psta, ac);
				txsc_amsdu_set_timer_status(psta, ac, TXSC_AMSDU_TIMER_SETTING);
			}
		}
	}
	if (*status == TXSC_AMSDU_NEED_DEQ)
		txq->is_normal_deq = 1;

	_txsc_spinunlock_bh(&txq->txsc_amsdu_lock);

abort:
	return _SUCCESS;
}

static void txsc_amsdu_timeout_init_pkt_entry(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt, struct sta_info *psta, u8 ac)
{
	u8 i = 0;

	txsc_pkt->step = TXSC_NONE;
	txsc_pkt->txsc_id = 0xff;
	txsc_pkt->ptxreq = NULL;

	for (i = 0; i < MAX_TXSC_SKB_NUM; i++) {
		txsc_pkt->xmit_skb[i] = NULL;
	}

	RTW_TX_TRACE_TXSC_RESET(padapter, txsc_pkt);

	txsc_pkt->psta = psta;
	txsc_pkt->skb_cnt = 0;/* this is for amsdu timeout */
#ifdef CONFIG_ETHER_PKT_AGG
	txsc_pkt->isAggPkt = 0;
#endif /* CONFIG_ETHER_PKT_AGG */
	txsc_pkt->amsdu = false;
	txsc_pkt->ac = ac;
	txsc_pkt->is_amsdu_timeout = true;
}

void ieee8023_header_to_rfc1042_shortcut(_adapter *padapter,
	struct sk_buff *skb, struct txsc_entry *txsc, struct sta_info *psta, bool add_pad)
{
	void *data;
	u8 *data1;
	int pad;
	__be16 len;
	const int headroom = SNAP_SIZE + 2;
	u16 *da, *sa;

	if (!skb)
		return;

	if (skb_headroom(skb) < headroom) {
		RTW_WARN("%s: headroom=%d isn't enough\n", __func__, skb_headroom(skb));
		if (pskb_expand_head(skb, headroom, 0, GFP_ATOMIC)) {
			RTW_ERR("%s: no headroom=%d for skb\n",
				__func__, headroom);

			return;
		}
	}

	data = skb_push(skb, SNAP_SIZE + 2);

	da = (data + SNAP_SIZE + 2);
	sa = (data + SNAP_SIZE + 2 + ETH_ALEN);
	CP_MAC_ADDR((u8*)data, (u8*)da);

	/* because NAT.25, A3 client mode MSDU subframe hdr should use mybssid as SA */
	/* because NAT.25, A3 client mode MSDU subframe hdr should use mybssid as SA */
	if (MLME_IS_AP(padapter)
	#ifdef CONFIG_RTW_A4_STA
		|| (psta->flags & WLAN_STA_A4)
	#endif
	)
		CP_MAC_ADDR((u8*)data + ETH_ALEN, (u8*)sa);
	else
		CP_MAC_ADDR((u8*)data + ETH_ALEN, (u8*)txsc->txsc_ethdr + ETH_ALEN);

	//memcpy(data, txsc->txsc_ethdr, 2 * ETH_ALEN);
	data1 = (u8*)(data + 2 * ETH_ALEN);
	len = cpu_to_be16(skb->len - 2 * ETH_ALEN - 2);
	//memcpy(data, &len, 2);
	data1[0] = *(u8 *)(&len);
	data1[1] = *((u8 *)&len + 1);
	//memcpy(data + 2, rtw_rfc1042_header, SNAP_SIZE);
	data1[2] = rtw_rfc1042_header[0];
	data1[3] = rtw_rfc1042_header[1];
	data1[4] = rtw_rfc1042_header[2];
	data1[5] = rtw_rfc1042_header[3];
	data1[6] = rtw_rfc1042_header[4];
	data1[7] = rtw_rfc1042_header[5];

	if (add_pad && (skb->len & (4 - 1))) {
		pad = 4 - (skb->len & (4 - 1));
		if (skb_tailroom(skb) < pad) {
			RTW_WARN("%s: tailroom=%d isn't enough\n", __func__, skb_tailroom(skb));
			if (pskb_expand_head(skb, 0, pad, GFP_ATOMIC)) {
				RTW_ERR("%s-%s: no tailroom=%d for skb\n",
					__func__, padapter->pnetdev->name, pad);
				return;
			}
		}
		skb_put(skb, pad); // do padding, not zero padding for CPU loading
		//rtw_skb_put_zero(skb, pad);
	}
}


static void ieee8023_header_to_rfc1042(struct sk_buff *skb, bool add_pad)
{
	void *data;
	int pad;
	__be16 len;
	const int headroom = SNAP_SIZE + 2;

	if (!skb)
		return;

	if (skb_headroom(skb) < headroom) {
		RTW_WARN("%s: headroom=%d isn't enough\n", __func__, skb_headroom(skb));
		if (pskb_expand_head(skb, headroom, 0, GFP_ATOMIC)) {
			RTW_ERR("%s: no headroom=%d for skb\n",
				__func__, headroom);
			return;
		}
	}

	data = skb_push(skb, SNAP_SIZE + 2);
	memmove(data, data + SNAP_SIZE + 2, 2 * ETH_ALEN);

	data += 2 * ETH_ALEN;
	len = cpu_to_be16(skb->len - 2 * ETH_ALEN - 2);
	memcpy(data, &len, 2);
	memcpy(data + 2, rtw_rfc1042_header, SNAP_SIZE);

	if (add_pad && (skb->len & (4 - 1))) {
		pad = 4 - (skb->len & (4 - 1));
		if (skb_tailroom(skb) < pad) {
			RTW_ERR("%s: no tailroom=%d for skb\n",
				__func__, pad);
			return;
		}
		rtw_skb_put_zero(skb, pad);
	}
}

void txsc_amsdu_sta_init(_adapter *padapter, struct sta_info* psta)
{
	/* AMSDU_BY_SIZE */
	/* default amsdu_2 3.9K for 2.4g and N-mode */
	u8 amsdu_num = AMSDU_0_MAX_NUM;
	u32 amsdu_size = AMSDU_0_SIZE - CORE_TXSC_WLHDR_SIZE; /* 3.9K */

	/* if he/vht && 5g && sta amsdu_cap > 0, use amsdu_4 7.9K */
	if (psta->hepriv.he_option == _TRUE || psta->vhtpriv.vht_option == _TRUE) {
		if (psta->phl_sta->asoc_cap.max_amsdu_len == 1) {
			if (padapter->tx_amsdu >= AMSDU_1_MAX_NUM)
				amsdu_num = AMSDU_1_MAX_NUM;
			else
				amsdu_num = padapter->tx_amsdu;
			amsdu_size = AMSDU_1_SIZE - CORE_TXSC_WLHDR_SIZE;/* HE, 7.9K */
		} else if (psta->phl_sta->asoc_cap.max_amsdu_len == 2) {
			if (psta->hepriv.he_option == _FALSE) {
				/* if ac mode, tx amsdu limit to 7.9k */
				if (padapter->tx_amsdu >= AMSDU_1_MAX_NUM)
					amsdu_num = AMSDU_1_MAX_NUM;
				else
					amsdu_num = padapter->tx_amsdu;/* AMSDU_MAX_NUM */
				amsdu_size = AMSDU_1_SIZE - CORE_TXSC_WLHDR_SIZE;/* VHT, 7.9K */
			} else {
				amsdu_num = padapter->tx_amsdu; /* AMSDU_MAX_NUM */
				amsdu_size = AMSDU_2_SIZE - CORE_TXSC_WLHDR_SIZE;/* HE, 11K */
			}
		}
	}

	/* for 2.4g repeater/client mode, set amsdu_2 for becoz amsdu_4 tp will degrade */
	if (MLME_IS_STA(padapter) && is_supported_24g(padapter->registrypriv.band_type)) {
		amsdu_num = 2;
		amsdu_size = AMSDU_0_SIZE - CORE_TXSC_WLHDR_SIZE; /* 3.9K */
	}

	/* legacy mode do not turn on amsdu */
	if (psta->hepriv.he_option == _FALSE &&
		psta->vhtpriv.vht_option == _FALSE &&
		psta->htpriv.ht_option == _FALSE) {
		amsdu_num = amsdu_size = 0;
	}

	psta->txsc_amsdu_max = psta->txsc_amsdu_num = amsdu_num;
	psta->txsc_amsdu_max_size = psta->txsc_amsdu_size = amsdu_size;
#ifdef CONFIG_AMSDU_HW_TIMER
	psta->txsc_amsdu_hw_timeout = 1;
#endif

#if defined(CONFIG_ETHER_PKT_AGG) && (AGGQ_SET_AGGQ_RULE==1)
	if (WIFI_ROLE_IS_ON_5G(padapter)) {
		set_aggQ_rule(psta->phl_sta->mac_addr, psta->txsc_amsdu_num, psta->txsc_amsdu_size);
	}
#endif /* CONFIG_ETHER_PKT_AGG && AGGQ_SET_AGGQ_RULE */
}
#endif /* CONFIG_TXSC_AMSDU */

void txsc_init(_adapter *padapter)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	pxmitpriv->txsc_enable = 1; /* default TXSC on */

	padapter->swdbg = 0;

#ifdef CONFIG_TXSC_AMSDU
	/* default SW amsdu on */
	pxmitpriv->txsc_amsdu_enable = 1;
	/* default SW amsdu num = 2 */
	//padapter->tx_amsdu = 2;
	/* AMSDU_BY_SIZE */
	pxmitpriv->txsc_amsdu_mode = 1;

	/* defasult transfer msdu hdr in core layer */
#if defined(CONFIG_VW_REFINE) && defined(RTW_CORE_TX_MSDU_TRANSFER_IN_PHL)
	/* defasult transfer msdu hdr in core layer */
	if (padapter->is_map == 0)
		padapter->is_msdu = 1;/* 0:CORE, 1:PHL */
	else
		padapter->is_msdu = 0;
#endif
#endif

	RTW_INFO("[TXSC_INIT][%s] txsc:%d, txsc_amsdu:%d amsdu_mode:%d\n",
			padapter->pnetdev->name, pxmitpriv->txsc_enable,
			pxmitpriv->txsc_amsdu_enable, pxmitpriv->txsc_amsdu_mode);

	if(padapter->txsc_time_duration == 0 && padapter->txsc_cache_thres == 0)
	{
		padapter->txsc_time_duration = DEFAULT_CORE_TXSC_TIME_DURATION;
		padapter->txsc_cache_thres = (DEFAULT_CORE_TXSC_THRES * DEFAULT_CORE_TXSC_TIME_DURATION) / HZ;
	}
}

void txsc_clear(_adapter *padapter, u8 free_self)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	struct sta_info *psta = NULL;
	int i, j, k, tmp = 0;
	_list *plist, *phead;

	RTW_INFO("[TXSC] clear txsc entry\n");

	_rtw_spinlock_bh(&pxmitpriv->txsc_lock);

	if (pxmitpriv->txsc_enable) {
		tmp = pxmitpriv->txsc_enable;
		pxmitpriv->txsc_enable = 0;
	}

	#ifdef DEBUG_PHL_TX
	if (phl_com && free_self)
		phl_com->tx_stats.phl_txsc_apply_cnt = 0;
	#endif

	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);

			psta->txsc_cache_num = 0;
			psta->txsc_cur_idx = 0;
			psta->txsc_cache_idx = 0;
			psta->txsc_cache_hit = 0;
				psta->txsc_a4_hit = 0;
			#ifdef CONFIG_TXSC_AMSDU
			psta->txsc_amsdu_hit = 0;
				psta->txsc_a4_amsdu_hit = 0;
			#endif
			psta->txsc_cache_miss = 0;
			psta->txsc_path_slow = 0;
			#ifdef CONFIG_RTW_ENABLE_HW_TXSC
			psta->hw_txsc_hit = 0;
			psta->hw_txsc_set = 0;
			#endif

			/* reset txsc entry */
			for (k = 0; k < CORE_TXSC_ENTRY_NUM; k++) {
#ifdef USE_ONE_WLHDR
				for (j = 0; j < (WMM_NUM * WLHDR_NUM); j++) {
					if (psta->txsc_entry_cache[k].txsc_wlhdr[j]) {
						rtw_mfree(psta->txsc_entry_cache[k].txsc_wlhdr[j], CORE_TXSC_WLHDR_SIZE);
						psta->txsc_entry_cache[k].txsc_wlhdr[j] = NULL;
					}
				}
#else
				_rtw_memset((u8 *)&psta->txsc_entry_cache[k].txsc_wlhdr, 0x0, CORE_TXSC_WLHDR_SIZE);
#endif
			}
			_rtw_memset(psta->txsc_entry_cache, 0x0, sizeof(struct txsc_entry) * CORE_TXSC_ENTRY_NUM);
			if (psta->phl_sta)
				_rtw_memset(psta->phl_sta->phl_txsc, 0x0, sizeof(struct phl_txsc_entry) * PHL_TXSC_ENTRY_NUM);
		}
	}

	if (free_self) {
		if (!padapter->phl_role)
			goto exit;

		psta = rtw_get_stainfo(pstapriv, padapter->phl_role->mac_addr);
		if (psta) {
			psta->txsc_cache_num = 0;
			psta->txsc_cur_idx = 0;
			psta->txsc_cache_idx = 0;
			psta->txsc_cache_hit = 0;
			psta->txsc_a4_hit = 0;
			psta->txsc_cache_miss = 0;
			psta->txsc_path_slow = 0;
		#ifdef CONFIG_RTW_ENABLE_HW_TXSC
			psta->hw_txsc_hit = 0;
			psta->hw_txsc_set = 0;
		#endif
			/* reset txsc entry */
			for (k = 0; k < CORE_TXSC_ENTRY_NUM; k++) {
#ifdef USE_ONE_WLHDR
				for (j = 0; j < (WMM_NUM * WLHDR_NUM); j++) {
					if (psta->txsc_entry_cache[k].txsc_wlhdr[j]) {
						rtw_mfree(psta->txsc_entry_cache[k].txsc_wlhdr[j], CORE_TXSC_WLHDR_SIZE);
						psta->txsc_entry_cache[k].txsc_wlhdr[j] = NULL;
					}
				}
#else
				_rtw_memset((u8 *)&psta->txsc_entry_cache[k].txsc_wlhdr, 0x0, CORE_TXSC_WLHDR_SIZE);
#endif
			}
			_rtw_memset(psta->txsc_entry_cache, 0x0, sizeof(struct txsc_entry) * CORE_TXSC_ENTRY_NUM);
			if (psta->phl_sta)
			    _rtw_memset(psta->phl_sta->phl_txsc, 0x0, sizeof(struct phl_txsc_entry) * PHL_TXSC_ENTRY_NUM);
		}
	}
exit:
	pxmitpriv->txsc_enable = tmp;

	_rtw_spinunlock_bh(&pxmitpriv->txsc_lock);
}

void txsc_dump(_adapter *padapter, void *m)
{
	struct sta_priv	*pstapriv = &padapter->stapriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	struct sta_info *psta = NULL;
	int i, j;
	_list *plist, *phead;

	RTW_PRINT_SEL(m, "[txsc][core] (txsc,enable) txsc_enable:%d\n", pxmitpriv->txsc_enable);
	#ifdef DEBUG_PHL_TX
	RTW_PRINT_SEL(m, "[txsc][phl] phl_txsc_apply_cnt: %d\n", phl_com->tx_stats.phl_txsc_apply_cnt);
	#endif
	RTW_PRINT_SEL(m, "\n");

	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);

		if (!psta || (psta == padapter->self_sta))
			continue;

		RTW_PRINT_SEL(m, "[%d] STA[%02x:%02x:%02x:%02x:%02x:%02x]\n", i,
			psta->phl_sta->mac_addr[0], psta->phl_sta->mac_addr[1], psta->phl_sta->mac_addr[2],
			psta->phl_sta->mac_addr[3], psta->phl_sta->mac_addr[4], psta->phl_sta->mac_addr[5]);
		RTW_PRINT_SEL(m, "[txsc] cur_idx:%d\n", psta->txsc_cur_idx);
		RTW_PRINT_SEL(m, "[txsc][core] txsc_path_slow:%d\n", psta->txsc_path_slow);
		RTW_PRINT_SEL(m, "[txsc][core] txsc_path_ps:%d\n", psta->txsc_path_ps);
		RTW_PRINT_SEL(m, "[txsc][core] txsc_cache_hit:%d\n", psta->txsc_cache_hit);
		#ifdef CONFIG_RTW_A4_STA
		RTW_PRINT_SEL(m, "[txsc][core] txsc_a4_hit:%d\n", psta->txsc_a4_hit);
		#endif
		RTW_PRINT_SEL(m, "[txsc][core] txsc_wlhdr_copy:%d\n", psta->txsc_wlhdr_copy);
		RTW_PRINT_SEL(m, "[txsc][core] txsc_priority_update:%d\n", psta->txsc_priority_update);
		#ifdef CONFIG_TXSC_AMSDU
		RTW_PRINT_SEL(m, "[txsc][core] txsc_amsdu_hit:%d\n", psta->txsc_amsdu_hit);
		RTW_PRINT_SEL(m, "[txsc][core] txsc_a4_amsdu_hit:%d\n", psta->txsc_a4_amsdu_hit);
		#endif
		#ifdef CONFIG_RTW_ENABLE_HW_TXSC
		RTW_PRINT_SEL(m, "[txsc][core] hw_txsc_hit:%d\n", psta->hw_txsc_hit);
		#endif
		RTW_PRINT_SEL(m, "[txsc][core] txsc_cache_miss:%d\n", psta->txsc_cache_miss);
		RTW_PRINT_SEL(m, "[txsc][core] txsc_entry_full:%d\n", psta->txsc_entry_full);
		RTW_PRINT_SEL(m, "\n");
		for (j = 0 ; j < CORE_TXSC_ENTRY_NUM; j++) {
			if (!psta->txsc_entry_cache[j].txsc_is_used)
				continue;

			//txsc_dump_data(psta->txsc_entry_cache[j].txsc_ethdr, ETH_HLEN, "ETHDR:");
			//txsc_dump_data(psta->txsc_entry_cache[j].txsc_wlhdr[0], psta->txsc_entry_cache[j].txsc_wlhdr_len, "WLHDR:");
			RTW_PRINT_SEL(m, " [%d][txsc][core] use_cnt:%u\n", j, psta->txsc_entry_cache[j].use_cnt);
			RTW_PRINT_SEL(m, " [%d][txsc][core] txsc_core_hit:%d\n", j, psta->txsc_entry_cache[j].txsc_cache_hit);
			#ifdef CONFIG_RTW_A4_STA
			RTW_PRINT_SEL(m, " [%d][txsc][core] txsc_a4_hit:%d\n", j, psta->txsc_entry_cache[j].txsc_a4_hit);
			#endif
			#ifdef CONFIG_PHL_TXSC
			RTW_PRINT_SEL(m, " [%d][txsc][phl]  txsc_phl_hit:%d\n", j, psta->phl_sta->phl_txsc[j].txsc_cache_hit);
			#endif
			#ifdef CONFIG_TXSC_AMSDU
			RTW_PRINT_SEL(m, " [%d][txsc] txsc_amsdu:%d\n", j, psta->txsc_entry_cache[j].txsc_amsdu);
			#endif
			RTW_PRINT_SEL(m, " [%d][txsc][core] last_hit_time:%lu\n", j, psta->txsc_entry_cache[j].last_hit_time);
			RTW_PRINT_SEL(m, " [%d][txsc][core] last_hit_cnt:%u\n", j, psta->txsc_entry_cache[j].last_hit_cnt);
			RTW_PRINT_SEL(m, " [%d][txsc][core] hit_average:%u\n", j, psta->txsc_entry_cache[j].cache_hit_average);
			RTW_PRINT_SEL(m, "\n");
		}
	}
}
}

void txsc_dump_data(u8 *buf, u16 buf_len, const char *prefix)
{
	u32 i = 0, j = 0;

	printk("[%s (%uB)]@%px:\n", prefix, buf_len, buf);

	if (buf == NULL) {
		printk("[txsc_dump] NULL!\n");
		return;
	}

	/* only print 64b data */
	if (buf_len >= 64)
		buf_len = 64;

	while (i < buf_len) {
		for(j = 0; (j < 4) && (i < buf_len); j++, i += 16)
			printk("%04X - %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", i,
				buf[i], buf[i+1], buf[i+2], buf[i+3],
				buf[i+4], buf[i+5], buf[i+6], buf[i+7],
				buf[i+8], buf[i+9], buf[i+10], buf[i+11],
				buf[i+12], buf[i+13], buf[i+14], buf[i+15]);
	}
	printk("\n");
}


#ifdef CONFIG_PCI_HCI
void txsc_recycle_txreq_phyaddr(_adapter *padapter, struct rtw_xmit_req *txreq)
{
	PPCI_DATA pci_data = dvobj_to_pci(padapter->dvobj);
	struct pci_dev *pdev = pci_data->ppcidev;
	struct rtw_pkt_buf_list *pkt_list = (struct rtw_pkt_buf_list *)txreq->pkt_list;
	dma_addr_t phy_addr = 0;
	struct xmit_txreq_buf *ptxreq_buf = txreq->os_priv;
	u8 idx = 0;

	if (ptxreq_buf) {
		if (ptxreq_buf->pkt_cnt == 0) {
			//RTW_ERR("[%s:%d] skb_cnt = 0\n", __func__, __LINE__);
			return;
		}

		/* do not recycle pkt_list[0] = wlhdr for SW TXSC */
		pkt_list++;
		for (idx = 1; idx < txreq->pkt_cnt; idx++) {
			phy_addr = (pkt_list->phy_addr_l);
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
			phy_addr |= ((u64)pkt_list->phy_addr_h << 32);
#endif
			pci_unmap_bus_addr(pdev, &phy_addr, pkt_list->length, PCI_DMA_TODEVICE);
			pkt_list++;
		}
	}
}

void txsc_fill_txreq_phyaddr(_adapter *padapter, struct rtw_pkt_buf_list *pkt_list)
{
	PPCI_DATA pci_data = dvobj_to_pci(padapter->dvobj);
	struct pci_dev *pdev = pci_data->ppcidev;
	dma_addr_t phy_addr = 0;

	pci_get_bus_addr(pdev, pkt_list->vir_addr, &phy_addr, pkt_list->length, PCI_DMA_TODEVICE);
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
	pkt_list->phy_addr_h =  phy_addr >> 32;
#else
	pkt_list->phy_addr_h = 0x0;
#endif
	pkt_list->phy_addr_l = phy_addr & 0xFFFFFFFF;
}
#endif

__IMEM_WLAN_SECTION__
static void txsc_init_pkt_entry(_adapter *padapter, struct sk_buff *pskb, struct txsc_pkt_entry *txsc_pkt)
{
	u8 i;

	txsc_pkt->step = TXSC_NONE;
	txsc_pkt->txsc_id = 0xff;
	txsc_pkt->ptxreq = NULL;

	txsc_pkt->xmit_skb[0] = pskb;
	for (i = 1; i < MAX_TXSC_SKB_NUM; i++) {
		txsc_pkt->xmit_skb[i] = NULL;
	}

	RTW_TX_TRACE_TXSC_RESET(padapter, txsc_pkt);

	//txsc_pkt->psta = NULL;
	txsc_pkt->skb_cnt = 1;
	txsc_pkt->priority = pskb->cb[_SKB_CB_PRIORITY];

#ifdef CONFIG_TXSC_AMSDU
	txsc_pkt->amsdu = false;
	txsc_pkt->ac = _up_to_qid(txsc_pkt->priority);
	txsc_pkt->is_amsdu_timeout = false;
#endif

#ifdef CONFIG_ETHER_PKT_AGG
	if (pskb->cb[_SKB_CB_ETH_AGG] == _PKT_TYPE_AGG_MULTI_PKT)
		txsc_pkt->isAggPkt = _AGG_TYPE_MULTI_PKT;
	else if (pskb->cb[_SKB_CB_ETH_AGG] == _PKT_TYPE_AGG_PKTLIST)
		txsc_pkt->isAggPkt = _AGG_TYPE_PKTLIST;
	else
		txsc_pkt->isAggPkt = 0;
#endif /* CONFIG_ETHER_PKT_AGG */

#ifdef CONFIG_VW_REFINE
	txsc_pkt->vw_cnt = 0;
#endif
    txsc_pkt->err = 0;
}

static void txsc_add_sc_check(_adapter *padapter, struct xmit_frame *pxframe, struct txsc_pkt_entry *txsc_pkt)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct pkt_attrib *pattrib = &pxframe->attrib;
	struct sta_info *psta = pxframe->attrib.psta;

	if (!pxmitpriv->txsc_enable) {
		if (psta->txsc_cache_num > 0)
			txsc_pkt->step = TXSC_SKIP;
		else
			txsc_pkt->step = TXSC_NONE;
		txsc_pkt->err |= 0x10;
		goto exit;
	}

#if 0//def CONFIG_RTW_A4_STA /* A4_TXSC */
	if (psta->flags & WLAN_STA_A4) {
		RTW_PRINT("This is a A4 STA\n");
		txsc_pkt->step = TXSC_NONE;
		goto exit;
	}
#endif

	if (pxframe->attrib.nr_frags > 1 || pxframe->attrib.bswenc == 1) {
		txsc_pkt->err |= 0x20;
		goto exit;
	}

	if (txsc_pkt->step != TXSC_NONE) {
		txsc_pkt->err |= 0x30;
		goto exit;
	}

	if (pattrib->qos_en &&
		pattrib->ampdu_en == 1 &&
		(pattrib->ether_type == ETH_P_IP || pattrib->ether_type == ETH_P_IPV6) &&
		pattrib->mdata == 0 && /* CONFIG_ONE_TXQ */
		!IS_MCAST(pattrib->ra) &&
		!pattrib->icmp_pkt &&
		!pattrib->icmpv6_pkt &&
		!pattrib->dhcp_pkt) {

		//RTW_PRINT("[%s] sta[%02x] add eth_type=0x%x pkt to txsc\n",
		//	__func__, pattrib->psta->phl_sta->mac_addr[5], pattrib->ether_type);

                #if 0//def CONFIG_RTW_A4_STA
		if (psta->flags & WLAN_STA_A4)
			RTW_INFO("This is a A4 STA\n");
                #endif
#ifdef CONFIG_RTW_A4_STA
		/* A4 mode, A3 addr shoule not be bmcast */
		if((psta->flags & WLAN_STA_A4) && IS_MCAST(pattrib->dst))
			goto exit;
#endif
		txsc_pkt->step = TXSC_ADD;
	} else
		txsc_pkt->err |= 0x40;

exit:
	return;
}

enum {
	TXSC_COMP_SA,
	TXSC_COMP_DA,
	TXSC_COMP_SA_ETH_TYPE,
	TXSC_COMP_DA_ETH_TYPE,
	TXSC_COMP_ALL,
};

static inline int txsc_comp_eth_hdr(const u8 *cs, const u8 *ct, u8 comp_case)
{
	switch (comp_case)
	{
	case TXSC_COMP_SA:
		return (*(u16 *)(cs+6) == *(u16 *)(ct+6) && *(u16 *)(cs+8) == *(u16 *)(ct+8) && \
			*(u16 *)(cs+10) == *(u16 *)(ct+10));
	case TXSC_COMP_SA_ETH_TYPE:
		return (*(u16 *)(cs+6) == *(u16 *)(ct+6) && *(u16 *)(cs+8) == *(u16 *)(ct+8) && \
			*(u16 *)(cs+10) == *(u16 *)(ct+10) && *(u16 *)(cs+12) == *(u16 *)(ct+12));
#ifdef CONFIG_ETHER_PKT_AGG
	case TXSC_COMP_DA:
		return (*(u16 *)(cs) == *(u16 *)(ct) && *(u16 *)(cs+2) == *(u16 *)(ct+2) && \
			*(u16 *)(cs+4) == *(u16 *)(ct+4));
#endif /* CONFIG_ETHER_PKT_AGG */
	case TXSC_COMP_DA_ETH_TYPE:
		return (*(u16 *)(cs) == *(u16 *)(ct) && *(u16 *)(cs+2) == *(u16 *)(ct+2) && \
			*(u16 *)(cs+4) == *(u16 *)(ct+4) && *(u16 *)(cs+12) == *(u16 *)(ct+12));
	case TXSC_COMP_ALL:
		return (*(u16 *)(cs) == *(u16 *)(ct) && *(u16 *)(cs+2) == *(u16 *)(ct+2) && \
			*(u16 *)(cs+4) == *(u16 *)(ct+4) && *(u16 *)(cs+6) == *(u16 *)(ct+6) && \
			*(u16 *)(cs+8) == *(u16 *)(ct+8) && *(u16 *)(cs+10) == *(u16 *)(ct+10) && \
			*(u16 *)(cs+12) == *(u16 *)(ct+12));
	}

	return 0;
}

__IMEM_WLAN_SECTION__
static u8 txsc_get_sc_entry(_adapter *padapter, struct sk_buff *pskb, struct txsc_pkt_entry *txsc_pkt)
{
	struct sta_priv	*pstapriv = &padapter->stapriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_info *psta = NULL;
	u8 *ptxsc_ethdr = NULL;
	u8 i, res = _FAIL;
	u8 idx, comp_case;
	u8 is_client_mode = MLME_IS_STA(padapter);
	u8 is_a4_enable = 0;
	u8 ac = 0;
	struct txsc_amsdu_swq *txq = NULL;

#ifdef CONFIG_RTW_A4_STA
	is_a4_enable = padapter->a4_enable;
#endif

	if (!pxmitpriv->txsc_enable || !pskb) {
		txsc_pkt->err = 1;
		return res;
	}

	if (pskb->cb[_SKB_CB_FLAGS] & _PKT_TYPE_URGENT) {
		txsc_pkt->step = TXSC_SKIP;
		goto exit;
	}

	/* TXSC_STA_MODE , if client mode and data is bmcast */
	if (is_client_mode && IS_MCAST(pskb->data)) {
		txsc_pkt->step = TXSC_SKIP;
		txsc_pkt->err = 2;
		goto exit;
	}

	if (txsc_pkt->psta) {
		psta = txsc_pkt->psta;
#ifdef CONFIG_TXSC_AMSDU
		if (txsc_pkt->is_amsdu_timeout) {
			if (!psta->phl_sta)
				goto exit;
		}
#endif
		goto get_txsc_entry;
	}

	if (IS_MCAST(pskb->data)) {
		txsc_pkt->err = 4;
		goto exit;
	}

	psta = rtw_get_stainfo(pstapriv, pskb->data);
	if (!psta) {
		txsc_pkt->err = 5;
		goto exit;
	}

get_txsc_entry:

	if (psta->txsc_cache_num == 0) {
		txsc_pkt->err = 6;
		goto exit;
	}

	ac = (txsc_pkt->ac < 4) ? txsc_pkt->ac : 2; /* default BE */
	txq = &psta->amsdu_txq[ac];

#ifdef CONFIG_VW_REFINE
	/* txsc_ethdr is amsdu format, so only compare source address */
	if (padapter->dvobj->tx_mode == 1)
		comp_case = TXSC_COMP_SA;
	else
#endif
		comp_case = TXSC_COMP_SA_ETH_TYPE;

	/* TXSC_STA_MODE, A3_VXD, A3 client mode only check ra */
	if (is_client_mode && !is_a4_enable)
		comp_case = TXSC_COMP_DA_ETH_TYPE;

#ifdef CONFIG_RTW_A4_STA/* A4_TXSC */
	/* TXSC_STA_MODE, A4, A4 mode check HLEN */
	if (is_a4_enable && (psta->flags & WLAN_STA_A4))
		comp_case = TXSC_COMP_ALL;
#endif

#ifdef CONFIG_ETHER_PKT_AGG
	if(txsc_pkt->isAggPkt) { // ethernet agg packet
		comp_case = TXSC_COMP_DA;
	}
#endif /* CONFIG_ETHER_PKT_AGG */

	_txsc_spinlock_bh(&pxmitpriv->txsc_lock);

	for (i = 0, idx = psta->txsc_cache_idx; i < CORE_TXSC_ENTRY_NUM; i++) {
		ptxsc_ethdr = (u8 *)&psta->txsc_entry_cache[idx].txsc_ethdr;

		if (psta->txsc_entry_cache[idx].txsc_is_used
			&& txsc_comp_eth_hdr(pskb->data, ptxsc_ethdr, comp_case)
#if	!defined(CONFIG_VW_REFINE) && !defined(CONFIG_ONE_TXQ)
			&& (pskb->len <= psta->txsc_entry_cache[idx].txsc_frag_len)
#endif
			) {
#ifdef CONFIG_ETHER_PKT_AGG
			if (txsc_pkt->isAggPkt)
				txsc_pkt->step = TXSC_AMSDU_APPLY;
			else // When enable ETHER_PKT_AGG, skip txsc amsdu for single packet
				txsc_pkt->step = TXSC_APPLY;
#else /* !CONFIG_ETHER_PKT_AGG */

#ifdef CONFIG_TXSC_AMSDU
			if (   pxmitpriv->txsc_amsdu_enable
				&& psta->txsc_entry_cache[idx].txsc_amsdu
				&& (   psta->txsc_amsdu_num
					|| txq->cnt
#ifdef CONFIG_VW_REFINE
					|| padapter->vw_enable
#endif
					|| txsc_pkt->is_amsdu_timeout))
				txsc_pkt->step = TXSC_AMSDU_APPLY;
			else
#endif
				txsc_pkt->step = TXSC_APPLY;
#endif /* CONFIG_ETHER_PKT_AGG */

			txsc_pkt->psta = psta;
			txsc_pkt->txsc_id = idx;
			psta->txsc_cache_idx = idx;
			psta->txsc_entry_cache[idx].use_cnt++;

			res = _SUCCESS;

			break;
		}

		idx = (idx + 1) % CORE_TXSC_ENTRY_NUM;
	}

#ifdef CONFIG_TXSC_AMSDU
	if (txsc_pkt->step == TXSC_AMSDU_APPLY)
		txsc_pkt->amsdu = true;
	else {
		txsc_pkt->amsdu = false;
		/* Exception case */
		if (txsc_pkt->is_amsdu_timeout) {
			if (res == _SUCCESS) {
				txsc_pkt->txsc_id = 0xff;
				psta->txsc_entry_cache[idx].use_cnt--;
				res = _FAIL;
			}

			if (txsc_pkt->step == TXSC_APPLY)
				txsc_pkt->err = 7;
			else
				txsc_pkt->err = 8;
		}
	}
#endif

	_txsc_spinunlock_bh(&pxmitpriv->txsc_lock);

	/* skip power saving mode */
	if (res == _SUCCESS && (psta->state & WIFI_SLEEP_STATE)) {
		#if 0/* _VW_ */
		if (!txsc_pkt->amsdu) {
			res = _FAIL;
			txsc_pkt->step = TXSC_SKIP;
		}
		#endif
		psta->txsc_path_ps++;
	}

#ifdef CONFIG_VW_REFINE
	if ( padapter->vw_enable) {
		if ((res == _SUCCESS) && (0 != pskb->cb[_SKB_VW_FLAG])) {
			if (true == txsc_pkt->amsdu)
			   DBG_COUNTER(padapter->tx_logs.core_vw_txsc);
			else
			   DBG_COUNTER(padapter->tx_logs.core_vw_txsc2);
		}
	}
#endif

exit:

	return res;
}

static void txsc_prepare_sc_entry(_adapter *padapter, struct xmit_frame *pxframe, struct txsc_pkt_entry *txsc_pkt)
{
	struct sta_priv	*pstapriv = &padapter->stapriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct sta_info *psta = pxframe->attrib.psta;
	struct pkt_attrib *pattrib = &pxframe->attrib;
	struct rtw_xmit_req *txreq = pxframe->phl_txreq;
	struct rtw_pkt_buf_list *pkt_list = (struct rtw_pkt_buf_list *)txreq->pkt_list;
	struct rtw_t_meta_data *ptxsc_mdata = NULL;
	struct sk_buff *pskb = txsc_pkt->xmit_skb[0];
	#ifdef CONFIG_RTW_ENABLE_HW_TXSC
	enum rtw_phl_status phl_ret;
	#endif
	u8 i, idx;
	u8 *ptxsc_ethdr = NULL;
	u8 *ptxsc_wlhdr_len = NULL;
	#ifdef USE_ONE_WLHDR
	u16 *qc, *a3, *a4;
	u8 *mybssid;
	u8 *ptxsc_wlhdr[WMM_NUM * WLHDR_NUM] = {NULL};
	struct rtw_pkt_buf_list *ptxsc_pkt_list0[WMM_NUM * WLHDR_NUM] = {NULL};
	u8 ps = 0;
	#else
	u8 *ptxsc_wlhdr = NULL;
	#endif
	int prefer_idx = -1;
	u32 min_cache_average = 0xFFFFFFFF;

	if (!psta) {
		RTW_ERR("%s: fetal err, XF_STA = NULL, please check.\n", __func__);
		return;
	}

	if (txsc_pkt->step == TXSC_SKIP) {
		if (pxmitpriv->txsc_enable && psta->txsc_cache_num > 0)
			psta->txsc_cache_miss++;
		else
			psta->txsc_path_slow++;
	} else if (txsc_pkt->step == TXSC_NONE) {
		psta->txsc_path_slow++;
	}

	if (txsc_pkt->step != TXSC_ADD)
		return;

	_rtw_spinlock_bh(&pxmitpriv->txsc_lock);

	if(psta->txsc_cur_idx >= CORE_TXSC_ENTRY_NUM)
	{
		for(i = 0; i < CORE_TXSC_ENTRY_NUM; i++)
		{
			if (psta->txsc_entry_cache[i].use_cnt)
				continue;

			if(jiffies != psta->txsc_entry_cache[i].last_hit_time)
			{
				if(CIRC_CNT_RTW(jiffies, psta->txsc_entry_cache[i].last_hit_time, MAX_TIME_SIZE) > (padapter->txsc_time_duration << 1))
				{
					prefer_idx = i;
					break;
				}
			}

			if(psta->txsc_entry_cache[i].cache_hit_average && (psta->txsc_entry_cache[i].cache_hit_average < padapter->txsc_cache_thres))
			{
				if(min_cache_average > psta->txsc_entry_cache[i].cache_hit_average)
				{
					prefer_idx = i;
					min_cache_average = psta->txsc_entry_cache[i].cache_hit_average;
				}
			}
		}
		if(prefer_idx < 0)
		{
			txsc_pkt->step = TXSC_NONE;
			_rtw_spinunlock_bh(&pxmitpriv->txsc_lock);
			return;
		}
		idx = prefer_idx;
		RTW_INFO("[CORE_TXSC] txsc entry is full, replace rentry[%d]\n", idx);
		psta->txsc_entry_full++;
	}
	else
	{
		if (psta->txsc_entry_cache[psta->txsc_cur_idx].use_cnt) {
			txsc_pkt->step = TXSC_NONE;
			_rtw_spinunlock_bh(&pxmitpriv->txsc_lock);
			return;
		}
		idx = psta->txsc_cur_idx;
	}

	psta->txsc_entry_cache[idx].use_cnt = 0;
	txsc_pkt->txsc_id = 0xff;
	txsc_pkt->psta = psta;

	ptxsc_ethdr = (u8 *)&psta->txsc_entry_cache[idx].txsc_ethdr;
	#ifdef USE_ONE_WLHDR
	for (i = 0; i < (WMM_NUM * WLHDR_NUM); i++) {
		ptxsc_wlhdr[i] = psta->txsc_entry_cache[idx].txsc_wlhdr[i];
		ptxsc_pkt_list0[i] = &psta->txsc_entry_cache[idx].txsc_pkt_list0[i];
	}
	#else
	ptxsc_wlhdr = (u8 *)&psta->txsc_entry_cache[idx].txsc_wlhdr;
	#endif
	ptxsc_mdata = &psta->txsc_entry_cache[idx].txsc_mdata;
	ptxsc_wlhdr_len = &psta->txsc_entry_cache[idx].txsc_wlhdr_len;

#if 0
	if (psta->txsc_entry_cache[idx].txsc_is_used == 1)
		RTW_PRINT("[CORE_TXSC] txsc entry is full, replace rentry[%d]\n", idx);
#endif

	/* ALLOC WLHDR in DMA addr */
	#ifdef USE_ONE_WLHDR
	for (i = 0; i < (WMM_NUM * WLHDR_NUM); i++) {
		if (!ptxsc_wlhdr[i]) {
			ptxsc_wlhdr[i] = rtw_zmalloc(CORE_TXSC_WLHDR_SIZE);
			if (ptxsc_wlhdr[i] == NULL) {
				RTW_ERR("%s: ptxsc_wlhdr memory allocate failed.\n", __func__);
				txsc_pkt->err |= 0x60;
				_rtw_spinunlock_bh(&pxmitpriv->txsc_lock);
				return;
			}
			psta->txsc_entry_cache[idx].txsc_wlhdr[i] = ptxsc_wlhdr[i];
		}
	}
	#endif

	/* ETH HDR */
#ifdef CONFIG_ETHER_PKT_AGG
	if(pxframe->attrib.isAggPkt) {
		_rtw_memcpy(ptxsc_ethdr, pskb->data, ETH_HLEN - 2);
		_rtw_memcpy(ptxsc_ethdr + ETH_HLEN - 2, pskb->data + ETH_HLEN + 6, 2); // copy protocol type
	#if 0
		GDEBUG("ptxsc_ethdr=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
		ptxsc_ethdr[0],ptxsc_ethdr[1],ptxsc_ethdr[2],ptxsc_ethdr[3],ptxsc_ethdr[4],ptxsc_ethdr[5],ptxsc_ethdr[6],
		ptxsc_ethdr[7],ptxsc_ethdr[8],ptxsc_ethdr[9],ptxsc_ethdr[10],ptxsc_ethdr[11],ptxsc_ethdr[12],ptxsc_ethdr[13]);
	#endif
	} else
#endif /* CONFIG_ETHER_PKT_AGG */
	{
		_rtw_memcpy(ptxsc_ethdr, pskb->data, ETH_HLEN);
	#if 0
		GDEBUG("ptxsc_ethdr=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
		ptxsc_ethdr[0],ptxsc_ethdr[1],ptxsc_ethdr[2],ptxsc_ethdr[3],ptxsc_ethdr[4],ptxsc_ethdr[5],ptxsc_ethdr[6],
		ptxsc_ethdr[7],ptxsc_ethdr[8],ptxsc_ethdr[9],ptxsc_ethdr[10],ptxsc_ethdr[11],ptxsc_ethdr[12],ptxsc_ethdr[13]);
	#endif
	}
	//txsc_dump_data(ptxsc_ethdr, ETH_HLEN, "ETHDR:");

	/* WLAN HDR + LLC */

#ifdef CONFIG_ETHER_PKT_AGG
	if(pxframe->attrib.isAggPkt) {
	#ifdef USE_ONE_WLHDR
		_rtw_memcpy(ptxsc_wlhdr[0], pkt_list->vir_addr, pkt_list->length);
		_rtw_memcpy(ptxsc_wlhdr[0] + pkt_list->length, rtw_rfc1042_header, SNAP_SIZE);
		_rtw_memcpy(ptxsc_wlhdr[0] + pkt_list->length + SNAP_SIZE, pxframe->pkt->data + 20, 2); // protocol
		a3 = (u16 *)(ptxsc_wlhdr[0] + 4 + ETH_ALEN*2);
		qc = (u16 *)(ptxsc_wlhdr[0] + (pattrib->hdrlen - 2)); /* WLAN_HDR_A3_LEN */
		_rtw_memcpy(a3, pxframe->pkt->data + ETH_ALEN, ETH_ALEN); // modify a3 = SA
		SetAMsdu(qc, 0);
	#if 0
		GDEBUG("ptxsc_wlhdr=");
		for(i=0;i<pkt_list->length + SNAP_SIZE + 2;i++) {
			printk("%02x", ptxsc_wlhdr[0][i]);
		}
		printk("\n");
	#endif
		for (i = 1; i < (WMM_NUM * WLHDR_NUM); i++)
			_rtw_memcpy(ptxsc_wlhdr[i], ptxsc_wlhdr[0], pkt_list->length + SNAP_SIZE + 2);
	#else
		// TODO
	#endif

		*ptxsc_wlhdr_len = pkt_list->length + SNAP_SIZE + 2;
	} else
#endif /* CONFIG_ETHER_PKT_AGG */
	{
		#ifdef USE_ONE_WLHDR
		for (i = 0; i < (WMM_NUM * WLHDR_NUM); i++)
			_rtw_memcpy(ptxsc_wlhdr[i], pkt_list->vir_addr, pkt_list->length);
		#else
		_rtw_memcpy(ptxsc_wlhdr, pkt_list->vir_addr, pkt_list->length);
		#endif
		*ptxsc_wlhdr_len = pkt_list->length;
	}

	//txsc_dump_data(ptxsc_wlhdr[0], pkt_list->length, "WLHDR:");

	#ifdef CONFIG_RTW_ENABLE_HW_TXSC
	/* ToDo: Conditions to upload WLAN header to HW? */
	/* QoS data and HW sequence mode only */
	do {

		if (!pattrib->qos_en
		    || txreq->mdata.hw_seq_mode == 0) {
			txsc_pkt->err |= 0x70;
			break;
		}

		/* Update QoS data WHDR to FW */
		if (!psta->hw_txsc_set) {
			u32	sc_len = pkt_list->length - RTW_SZ_LLC; /* WLAN header w.o LLC */
			#ifdef CONFIG_RTW_ENABLE_HW_TX_AMSDU
			u16 *qc = (u16 *)(pkt_list->vir_addr + 24);

			SetAMsdu(qc, 1);
			#endif /* CONFIG_RTW_ENABLE_HW_TX_AMSDU */
			phl_ret = rtw_phl_update_shcut_hdr(padapter->dvobj->phl,
							   psta->phl_sta->macid, /* Use MAC ID as shortcut ID */
							   sc_len,
							   pkt_list->vir_addr);
			if (phl_ret == RTW_PHL_STATUS_SUCCESS) {
				RTW_PRINT("Download header shortcut#%u to FW OK.\n",
					psta->phl_sta->macid);
			} else {
				txsc_pkt->err |= 0x80;
				RTW_PRINT("Failed to download header shortcut to FW\n");
				break;
			}
			psta->hw_txsc_set = 1;
			memcpy(psta->txsc_data, pkt_list->vir_addr, sc_len);
			psta->txsc_data_len = sc_len;
		}
		/* Set meta data to use HW shortcut */
		txreq->mdata.smh_en = 1;
		txreq->mdata.shcut_camid = psta->phl_sta->macid;
		/*
		 * Set hdr_len to half of ether header to
		 * tell HW this is an ETHER frame and modification
		 * from ether header to SNAP LLC is required.
		 */
		txreq->mdata.hdr_len = ETH_HLEN / 2;
		#ifdef CONFIG_RTW_ENABLE_HW_TX_AMSDU
		txreq->mdata.hw_amsdu = 1;
		#endif /* CONFIG_RTW_ENABLE_HW_TX_AMSDU */
		/* WD info must be disabled for HW TX A-MSDU.
		 *  Disable WD info to test CMAC table setting when
		 *  using HW header conversion.
		 */
		txreq->mdata.wdinfo_en = 0;
	} while (0);
	#endif /* CONFIG_RTW_ENABLE_HW_TXSC */

	/* pkt_list[0] */
	#ifdef USE_ONE_WLHDR
	for (i = 0; i < (WMM_NUM * WLHDR_NUM); i++) {

		/* set different tid and amsdu */
		qc = (u16 *)(ptxsc_wlhdr[i] + (pattrib->hdrlen - 2)); /* WLAN_HDR_A3_LEN */
		/* set hw tid */
		SetPriority(qc, hwswq_tid_remap[i / 6]);
		/* set amsdu */
		if (i % 2)
			SetAMsdu(qc, 1);

		/* set mdata and esop */
        ps = i % 6;
        if (ps == 0 || ps == 1) {
            /* mdata=0, eosp = 0 */
            ClearMData(ptxsc_wlhdr[i]);
            SetEOSP(qc, 0);
        } else if (ps == 2 || ps == 3) {
            /* mdata=1, eosp = 0 */
            SetMData(ptxsc_wlhdr[i]);
            SetEOSP(qc, 0);
        } else if (ps == 4 || ps == 5) {
            /* mdata=0, eosp = 1 */
            ClearMData(ptxsc_wlhdr[i]);
            SetEOSP(qc, 1);
        }

		ptxsc_pkt_list0[i]->vir_addr = ptxsc_wlhdr[i];
		if (!(i % 2)) /* ampdu */
			ptxsc_pkt_list0[i]->length = *ptxsc_wlhdr_len;
		else /* amsdu */ {
			ptxsc_pkt_list0[i]->length = *ptxsc_wlhdr_len - (SNAP_SIZE + sizeof(u16));

			a3 = (u16 *)(ptxsc_wlhdr[i] + 4 + ETH_ALEN*2);
			a4 = (u16 *)(ptxsc_wlhdr[i] + 4 + ETH_ALEN*3 + 2);
			if (MLME_IS_AP(padapter))
				mybssid = GET_MY_HWADDR(padapter);
			else
				mybssid = get_bssid(pmlmepriv);
			_rtw_memcpy(a3, mybssid, ETH_ALEN);

			#ifdef CONFIG_RTW_A4_STA/* A4_TXSC */
			if (padapter->a4_enable && (psta->flags & WLAN_STA_A4))
				_rtw_memcpy(a4, mybssid, ETH_ALEN);
			#endif
			//RTW_INFO("[%d] a3:%p a4:%p %pM\n", i, a3, a4, get_bssid(pmlmepriv));
		}
	#ifdef CONFIG_PCI_HCI
		txsc_fill_txreq_phyaddr(padapter, ptxsc_pkt_list0[i]);
		#endif
	}
	//txsc_dump_data(ptxsc_wlhdr[4], pkt_list->length, "WLHDR_AMSU[4]:");
	#endif

	psta->txsc_entry_cache[idx].ampdu_wlhdr_len = *ptxsc_wlhdr_len;
	psta->txsc_entry_cache[idx].amsdu_wlhdr_len = *ptxsc_wlhdr_len - (SNAP_SIZE + sizeof(u16));
	psta->txsc_entry_cache[idx].is_amsdu_pkt_list0 = 0; /* init as a ampdu header */

	/* META DATA */
	_rtw_memcpy(ptxsc_mdata, &txreq->mdata, sizeof(*ptxsc_mdata));

	/* FRAGE_LEN */
	psta->txsc_entry_cache[idx].txsc_frag_len = pxframe->attrib.frag_len_txsc;

	psta->txsc_entry_cache[idx].txsc_is_used = 1;
	psta->txsc_cache_idx = idx;
	if(psta->txsc_cur_idx < CORE_TXSC_ENTRY_NUM)
		psta->txsc_cur_idx = psta->txsc_cur_idx + 1;
	if (psta->txsc_cache_num < CORE_TXSC_ENTRY_NUM)
		psta->txsc_cache_num++;

	psta->txsc_entry_cache[idx].last_hit_time = jiffies;
	psta->txsc_entry_cache[idx].last_hit_cnt = psta->txsc_entry_cache[idx].txsc_cache_hit;
	psta->txsc_entry_cache[idx].cache_hit_average = 0;

	psta->txsc_path_slow++;

	txreq->treq_type = RTW_PHL_TREQ_TYPE_PHL_ADD_TXSC | RTW_PHL_TREQ_TYPE_PHL_UPDATE_TXSC;

	/* set shortcut id  */
	txreq->shortcut_id = idx;
	psta->txsc_entry_cache[idx].txsc_phl_id = idx;

	//RTW_PRINT("[CORE_TXSC][ADD] core_txsc_idx:%d(cur_idx:%d), txreq_sc_id:%d, txsc_frag_len:%d\n",
	//	idx, psta->txsc_cur_idx, txreq->shortcut_id, psta->txsc_entry_cache[idx].txsc_frag_len);

#ifdef CONFIG_TXSC_AMSDU
	if (pxmitpriv->txsc_amsdu_enable && pxframe->attrib.amsdu_ampdu_en)
		psta->txsc_entry_cache[idx].txsc_amsdu = true;
	else {
		txsc_pkt->err |= 0x90;
		psta->txsc_entry_cache[idx].txsc_amsdu = false;
	}
#endif

	_rtw_spinunlock_bh(&pxmitpriv->txsc_lock);

	#ifdef CONFIG_RTW_ENABLE_HW_TXSC
	/* Set current one to non-HW-TXSC because SC is just sent
	 *  and might not be applied yet.
	 */
	if ((pxmitpriv->txsc_enable != 2)
	    || !psta->hw_txsc_set) {
		txreq->mdata.smh_en = 0;
		txreq->mdata.hdr_len = ptxsc_pkt_list0->length;
		txreq->mdata.hw_amsdu = 0;
		/* ToDo: disable WD info for TXSC */
		txreq->mdata.wdinfo_en = 1;
	}
	#endif /* CONFIG_RTW_ENABLE_HW_TXSC */

#ifdef CONFIG_CORE_TXSC
	/* for debug */
	print_txreq_mdata(&txreq->mdata, __func__);
#endif
}

u8 txsc_get_sc_cached_entry(_adapter *padapter, struct sk_buff *pskb, struct txsc_pkt_entry *txsc_pkt, struct sta_info *psta)
{
	txsc_init_pkt_entry(padapter, pskb, txsc_pkt);
	txsc_pkt->psta = psta;
	return txsc_get_sc_entry(padapter, pskb, txsc_pkt);
}

void txsc_add_sc_cache_entry(_adapter *padapter, struct xmit_frame *pxframe, struct txsc_pkt_entry *txsc_pkt)
{
	txsc_add_sc_check(padapter, pxframe, txsc_pkt);
	txsc_prepare_sc_entry(padapter, pxframe, txsc_pkt);
}

#ifdef CONFIG_ONE_TXQ
extern u8 txsc_amsdu_dequeue_txq(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt);
extern u8 txsc_amsdu_combine(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt);
#endif

u8 _txsc_get_wlhdr_idx(u8 priority, bool amsdu, u8 more_data, u8 eosp)
{
	u8 idx = 0;

	switch (priority) {
		case 0:
		//case 3:
			idx = 0;
			break;
		case 1:
		//case 2:
			idx = 1;
			break;
		case 4:
		//case 5:
			idx = 2;
			break;
		case 6:
		//case 7:
			idx = 3;
			break;
		default:
			RTW_PRINT("[%s] wrong pkt priority=%d , plz check.\n", __func__, priority);
			break;
	}

	idx = 6*idx + amsdu + 2*more_data + 4*eosp;

	return idx;
}

static void _txsc_update_sec_iv(_adapter *padapter, struct sta_info *psta, struct rtw_xmit_req *txreq)
{
	if (!psta)
		return;

	switch (txreq->mdata.sec_type) {
	case RTW_ENC_WEP40:
	case RTW_ENC_WEP104:
		psta->dot11txpn.val = (psta->dot11txpn.val == 0xffffff) ? 0 : (psta->dot11txpn.val + 1);
		break;

	case RTW_ENC_TKIP:
		psta->dot11txpn.val = (psta->dot11txpn.val == 0xffffffffffffULL) ? 0 : (psta->dot11txpn.val + 1);
		break;

	case RTW_ENC_CCMP:
		psta->dot11txpn.val = (psta->dot11txpn.val == 0xffffffffffffULL) ? 0 : (psta->dot11txpn.val + 1);
		break;

	case RTW_ENC_GCMP:
	case RTW_ENC_GCMP256:
		psta->dot11txpn.val = (psta->dot11txpn.val == 0xffffffffffffULL) ? 0 : (psta->dot11txpn.val + 1);
		break;

	case RTW_ENC_CCMP256:
		psta->dot11txpn.val = (psta->dot11txpn.val == 0xffffffffffffULL) ? 0 : (psta->dot11txpn.val + 1);
		break;
	default:
		break;
	}

	return;
}

inline void txsc_issue_addba(_adapter *padapter, u8 priority, struct sta_info *psta)
{
	u8 issued;
	struct ht_priv	*phtpriv;
	phtpriv = &psta->htpriv;
	issued = (phtpriv->agg_enable_bitmap >> priority) & 0x1;
	issued |= (phtpriv->candidate_tid_bitmap >> priority) & 0x1;
	if(issued == 0)
		txsc_issue_addbareq_cmd(padapter, priority, psta, _TRUE);

}

__IMEM_WLAN_SECTION__
u8 txsc_apply_sc_cached_entry(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt, u8 vw_pkt)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	#endif
	struct sta_xmit_priv *pstaxmitpriv = NULL;
	struct rtw_pkt_buf_list	*pkt_list = NULL;
	struct rtw_pkt_buf_list *pkt_list0 = NULL;
	struct rtw_t_meta_data	*mdata = NULL;
	struct rtw_xmit_req	*txreq = NULL;
	struct xmit_txreq_buf	*txreq_buf = NULL;
	struct txsc_entry	*txsc;
	struct sta_info *psta = txsc_pkt->psta;
	struct sk_buff **xmit_skb;
	u16 *qc;
	int wlhdr_copy_len = 0, payload_offset = ETH_HLEN;
	u8 idx, priority = 0, res = _SUCCESS;
	u8 i, skb_cnt = 0, is_hdr_need_update = 0, txsc_wlhdr_len, pads;
	u8 *ptxsc_wlhdr, *head;
	#ifdef CONFIG_RTW_ENABLE_HW_TXSC
	u32	use_hw_txsc = 0;
	#endif
	#ifdef CONFIG_VW_REFINE
	u8 my_vw_cnt = 0;
	u8 my_last_flag =0;
	#endif
	u8 wlhdr_idx = 0;
#ifdef USE_ONE_WLHDR
	u8 more_data = 0;
	u8 eosp = 0;
	u8 priority_idx = 0;
#endif

	if (!pxmitpriv->txsc_enable || !psta) {
		#ifdef CONFIG_VW_REFINE
		if ( vw_pkt )
			DBG_COUNTER(padapter->tx_logs.core_vw_testb);
		#endif /* CONFIG_VW_REFINE */
		return _FAIL;
	}

	if (txsc_pkt->step != TXSC_APPLY &&
		txsc_pkt->step != TXSC_AMSDU_APPLY) {
		#ifdef CONFIG_VW_REFINE
		if ( vw_pkt )
			DBG_COUNTER(padapter->tx_logs.core_vw_testc);
		#endif /* CONFIG_VW_REFINE */
		return _FAIL;
	}

	//_rtw_memset(xmit_skb, 0x0, sizeof(xmit_skb));
	pstaxmitpriv = &psta->sta_xmitpriv;

	/* get cached entry */
	idx = txsc_pkt->txsc_id;
	txsc = &psta->txsc_entry_cache[idx];
	mdata = &txsc->txsc_mdata;

	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	if(ru_ctrl->tbl_exist){
		mdata->rts_en = 0;
		mdata->cts2self = 0;
		mdata->hw_rts_en = 0;
	}
	#endif

	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	if(ru_ctrl->tx_phase){
		if(psta->core_current_txreq_cnt >= (padapter->max_tx_ring_cnt/(padapter->stapriv.asoc_sta_count-1)))
		{
			psta->core_txreq_abort++;
			res = _FAIL;
			return res;
		}
	}
	#endif

	#ifdef USE_ONE_WLHDR
	if (mdata->hw_seq_mode == 1)
		txreq_buf = (struct xmit_txreq_buf *)get_txreq_buffer(padapter, (u8 **)&txreq, (u8 **)&pkt_list, NULL, NULL, 1, psta, NULL);
	else
		txreq_buf = (struct xmit_txreq_buf *)get_txreq_buffer(padapter, (u8 **)&txreq, (u8 **)&pkt_list, (u8 **)&head, NULL, 1, psta, NULL);
	#else
		txreq_buf = (struct xmit_txreq_buf *)get_txreq_buffer(padapter, (u8 **)&txreq, (u8 **)&pkt_list, (u8 **)&head, NULL, 1, psta, NULL);
	#endif

	if (txreq_buf == NULL) {
#if defined(CONFIG_ONE_TXQ) && defined(CONFIG_TXSC_AMSDU)
		if (padapter->dvobj->tx_mode == 2)
			txsc_pkt->amsdu = false;
#endif

		#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
		psta->core_txsc_apply_no_txreq++;
		#endif

		#ifdef CONFIG_VW_REFINE
		if ( vw_pkt )
			DBG_COUNTER(padapter->tx_logs.core_vw_testd);
		#endif /* CONFIG_VW_REFINE */
		res = _FAIL;
		//goto exit;
		return res;
	}

	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	psta->core_current_txreq_cnt++;
	#endif

	/* init txreq buf */
	txreq_buf->adapter = padapter;

	/* fill txreq other */
	txreq->os_priv = (void *)txreq_buf;
	txreq->pkt_list = (u8 *)pkt_list;
	txreq->pkt_cnt = 0;
	txreq->total_len = 0;

	/* fill txsc pkt entry */
	txsc_pkt->ptxreq = txreq;

#ifdef CONFIG_TXSC_AMSDU
	if (txsc_pkt->amsdu
	#ifdef CONFIG_ETHER_PKT_AGG
		&& !txsc_pkt->isAggPkt
	#endif /* CONFIG_ETHER_PKT_AGG */
		) {
#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
		struct dvobj_priv *dvobj = padapter->dvobj;
#endif
		payload_offset = 0;

		xmit_skb = txsc_pkt->xmit_skb;

#ifdef CONFIG_VW_REFINE
		if ( (dvobj->tx_mode == 1) && (NULL == xmit_skb[0]) )
		{
		       res = _FAIL;
		       if ( vw_pkt )
		         DBG_COUNTER(padapter->tx_logs.core_vw_teste);
		       goto exit;
		}
#endif

#ifdef CONFIG_ONE_TXQ
		if (dvobj->tx_mode == 2) {
			if (dvobj->txq_amsdu_merge) {
				skb_cnt = txsc_amsdu_combine(padapter, txsc_pkt);
			} else {
				skb_cnt = txsc_amsdu_dequeue_txq(padapter, txsc_pkt);
				for (i = 0; i < skb_cnt; i++) {
					ieee8023_header_to_rfc1042_txsc(xmit_skb[i],((i + 1) == skb_cnt)? false : true);
				}
			}
		} else
#endif
		{
#ifdef CONFIG_VW_REFINE
		if (dvobj->tx_mode == 1) {
			if (txsc_pkt->is_amsdu_timeout) {
				skb_cnt = txsc_pkt->skb_cnt;
				DBG_COUNTER(padapter->tx_logs.core_vw_test8);
			} else {
				if ( vw_pkt )
				    DBG_COUNTER(padapter->tx_logs.core_vw_test2);
				skb_cnt = txsc_amsdu_dequeue_swq(padapter, txsc_pkt);
			}
		} else
			skb_cnt = txsc_amsdu_dequeue(padapter, txsc_pkt);
#else/* _VW_ */
		skb_cnt = txsc_amsdu_dequeue(padapter, txsc_pkt);
#endif

		if (skb_cnt > 0) {
#ifdef CONFIG_VW_REFINE
			if (dvobj->tx_mode == 1) {
				for (i = 0; i < skb_cnt; i++) {
					if ( xmit_skb[i]->cb[_SKB_VW_LAST] )
						my_last_flag = 1;

					if ( 0x80 & xmit_skb[i]->cb[_SKB_CB_AMSDU_TXSC] ) {
						u8 cnt = (xmit_skb[i]->cb[_SKB_VW_FLAG] & 0x0F);
						u8 all_cnt = (xmit_skb[i]->cb[_SKB_CB_AMSDU_TXSC] & 0x0F);

						my_vw_cnt += cnt;
						all_cnt -= cnt;
						DBG_COUNTER_NUM(padapter->tx_logs.core_vw_test3, cnt);
						DBG_COUNTER_NUM(padapter->tx_logs.core_vw_test4, all_cnt);
						if (((i + 1) != skb_cnt) && xmit_skb[i]->len & 0x3) {
							skb_put(xmit_skb[i], 4-(xmit_skb[i]->len & 0x3));
						}
						xmit_skb[i]->cb[_SKB_CB_AMSDU_TXSC] = 0;
						xmit_skb[i]->cb[_SKB_VW_FLAG] = 0;
					} else {
							if ( xmit_skb[i]->cb[_SKB_VW_FLAG]  != 0 )  {
							     DBG_COUNTER(padapter->tx_logs.core_vw_test3);
							     my_vw_cnt++;
							} else
							     DBG_COUNTER(padapter->tx_logs.core_vw_test4);
						    ieee8023_header_to_rfc1042_txsc(xmit_skb[i],((i + 1) == skb_cnt)? false : true);
					   }
				}
			} else
#endif
			{/* AMSDU normal tx flow */
				if (skb_cnt > 1) {
					#ifdef RTW_CORE_TX_MSDU_TRANSFER_IN_PHL
					if (!padapter->is_msdu)
					#endif
					{
						for (i = 0; i < skb_cnt; i++) {
							#ifdef CONFIG_VW_REFINE
							if ( xmit_skb[i]->cb[_SKB_VW_FLAG]  != 0 ) {
							      DBG_COUNTER(padapter->tx_logs.core_vw_test3);
							      my_vw_cnt++;
							} else
							      DBG_COUNTER(padapter->tx_logs.core_vw_test4);
							#endif /* CONFIG_VW_REFINE */
							ieee8023_header_to_rfc1042_shortcut(padapter, xmit_skb[i],  txsc, psta, (((i + 1) == skb_cnt) ? false : true));
						}
					}
				} else {
					/* if amsdu deq one pkt, do not send single amsdu */
					txsc_pkt->amsdu = false;
					txsc_pkt->step = TXSC_APPLY;
					payload_offset = ETH_HLEN;
					pxmitpriv->cnt_txsc_amsdu_deq_ampdu++;
				}
			}

			if (txsc_pkt->is_amsdu_timeout)
				pxmitpriv->cnt_txsc_amsdu_timeout_dump[skb_cnt]++;
			else
				pxmitpriv->cnt_txsc_amsdu_dump[skb_cnt]++;
		} else {
			if (txsc_pkt->is_amsdu_timeout)
				pxmitpriv->cnt_txsc_amsdu_timeout_dump[skb_cnt]++;
			else
				pxmitpriv->cnt_txsc_amsdu_dump[skb_cnt]++;

			pxmitpriv->cnt_txsc_amsdu_deq_empty++;
			#ifdef CONFIG_VW_REFINE
			DBG_COUNTER(padapter->tx_logs.core_vw_test7);
			#endif /* CONFIG_VW_REFINE */
			res = _FAIL;
			goto exit;
		}
		}
	} else
#endif
	{
		xmit_skb = txsc_pkt->xmit_skb;

		/* for no tx_amsdu case */
		#ifdef CONFIG_ETHER_PKT_AGG
		if (txsc_pkt->isAggPkt == _AGG_TYPE_MULTI_PKT) {
			txsc_pkt->skb_cnt = 1;
			payload_offset = 0;
		} else if (txsc_pkt->isAggPkt == _AGG_TYPE_PKTLIST) {
			for (i = 0; i < MAX_TXSC_SKB_NUM; i++) {
				if (xmit_skb[i]->next == NULL)
					break;
				xmit_skb[i + 1] = xmit_skb[i]->next;
			}

			if (i == MAX_TXSC_SKB_NUM && xmit_skb[i-1]->next != NULL) {
				RTW_PRINT("[ERR][%s:%d] skb list num > %d, please check", __func__, __LINE__, MAX_TXSC_SKB_NUM);
				txsc_pkt->skb_cnt = i;
				// TODO : free remaining packets
			} else
				txsc_pkt->skb_cnt = i + 1;

			payload_offset = 0;
		}
		#endif /* CONFIG_ETHER_PKT_AGG */
		skb_cnt = txsc_pkt->skb_cnt;
	}

	if (skb_cnt == 0)
		RTW_PRINT("[ERR][%s:%d] skb_cnt = 0 is a fatel error, plz check\n", __func__, __LINE__);

	RTW_TX_TRACE_ETH_TXSC_ALL(padapter, txsc_pkt, i);

#ifdef CONFIG_VW_REFINE
	txreq->my_macid = psta->phl_sta->macid;
	padapter->skb_vw_cnt[txreq->my_macid] += my_vw_cnt;
	txreq->vw_cnt = txsc_pkt->vw_cnt = my_vw_cnt;

#endif

#ifdef CONFIG_TXSC_AMSDU
	/* for avoid amsdu exchange */
	if (!pxmitpriv->txsc_amsdu_enable)
		txsc->txsc_amsdu = false;
#endif

	#ifdef USE_ONE_WLHDR
	if (mdata->hw_seq_mode == 1)
		priority_idx = txsc_pkt->priority;
	else
		priority_idx = 0;
	if (psta->state & WIFI_SLEEP_STATE) {
		if (psta->uapsd_bitmap & BIT(rtw_get_txq_idx(txsc_pkt->priority))) {
			if (pstaxmitpriv->tx_pending_bitmap & psta->uapsd_bitmap)
				more_data = 1;
			else
				eosp = 1;
		} else {
			if (pstaxmitpriv->tx_pending_bitmap & ~ psta->uapsd_bitmap)
				more_data = 1;
		}
		wlhdr_idx = _txsc_get_wlhdr_idx(priority_idx, txsc_pkt->amsdu, more_data, eosp);
	}
	else
		wlhdr_idx = _txsc_get_wlhdr_idx(priority_idx, txsc_pkt->amsdu, 0, 0);

	if (mdata->hw_seq_mode == 1) {
		pkt_list0 = &txsc->txsc_pkt_list0[wlhdr_idx];
		head = ptxsc_wlhdr = pkt_list0->vir_addr;
	} else {
		pkt_list0 = &txsc->txsc_pkt_list0[wlhdr_idx];
		_rtw_memcpy(head, pkt_list0->vir_addr, pkt_list0->length);
		ptxsc_wlhdr = head;
	}

	txsc_wlhdr_len = pkt_list0->length;
	#else
	ptxsc_wlhdr = (u8 *)txsc->txsc_wlhdr;
	txsc_wlhdr_len = txsc->txsc_wlhdr_len;
	#endif

	/* fill_txreq_mdata */
	_rtw_memcpy(&txreq->mdata, mdata, sizeof(txreq->mdata));

	/* Update TID from IP header */
	/* priority = *(xmit_skb[0]->data + ETH_HLEN + 1); */
	/*txreq->mdata.tid = tos_to_up(priority); */
	priority = txreq->mdata.tid = txsc_pkt->priority;
	txreq->mdata.cat = priority;//rtw_phl_cvt_tid_to_cat(priority);

#ifdef CONFIG_VW_REFINE
	if ( my_last_flag )
		txreq->mdata.bk = 1;

	txreq->is_map = padapter->is_map;
#ifdef RTW_CORE_TX_MSDU_TRANSFER_IN_PHL
	txreq->is_msdu = padapter->is_msdu;
#endif
#endif
#ifdef DYNAMIC_RTS_CONTROL
	check_rts(padapter, psta, &txreq->mdata);
#endif

	#ifdef CONFIG_ETHER_PKT_AGG
	txreq->isAggPkt = txsc_pkt->isAggPkt;
	#endif /* CONFIG_ETHER_PKT_AGG */
	if (txreq->mdata.sec_hw_enc) {
		txreq->mdata.sec_cam_idx = psta->txsc_sec_cam_idx;
		txreq->mdata.force_key_en = 1;
	}

	#ifdef CONFIG_RTW_ENABLE_HW_TXSC
	/* Verify SC in HW is correct */
	if ((pxmitpriv->txsc_enable == 2)
	    && (psta->hw_txsc_set)
	    && (txsc_pkt->step == TXSC_APPLY)) {
		if (!psta->hw_entry_used) {
			u8 sc_data[64] = {0};

			RTW_PRINT("Verifying TXSC of %u ...\n", psta->phl_sta->macid);
			#ifndef CONFIG_RTW_LINK_PHL_MASTER
			// 74b8420fe3902923e3a50f668c20b0b3c08beacd freddie.ho
			rtl_phl_dump_cam(padapter->dvobj->phl, RTW_CAM_SHCUT_MACHDR,
				psta->phl_sta->macid, sc_data);
			#endif /* CONFIG_RTW_LINK_PHL_MASTER */
			if ((sc_data[0] == psta->txsc_data_len)
			    && _rtw_memcmp(&sc_data[1], psta->txsc_data, psta->txsc_data_len)) {
				psta->hw_entry_used = 1;
				use_hw_txsc = 1;
				txreq->treq_type = RTW_PHL_TREQ_TYPE_HW_TXSC
						   | RTW_PHL_TREQ_TYPE_PHL_UPDATE_TXSC;
				RTW_PRINT("HW TXSC checked OK. Update WD.\n");
			} else {
				RTW_ERR("TXSC of %u mismatch!\n", psta->phl_sta->macid);
				DUMP_DATA(psta->txsc_data, psta->txsc_data_len, "Local:");
				DUMP_DATA(sc_data, psta->txsc_data_len + 1, "HW:");
			}
		} else {
			use_hw_txsc = 1;
			txreq->treq_type = RTW_PHL_TREQ_TYPE_HW_TXSC;
		}
		#ifdef CONFIG_RTW_DEBUG_HW_TXSC
		/* check HW entry content matches software's */
		if (psta->hw_entry_used) {
			if (!_rtw_memcmp(psta->txsc_data, pkt_list0->vir_addr, 16)) {
				RTW_ERR("HW TXSC does not match software cache!\n");
				DUMP_DATA(psta->txsc_data, 16, "HW:");
				DUMP_DATA(pkt_list0->vir_addr, 16, "SW:");
				use_hw_txsc = 0;
			}
		}
		#endif /* CONFIG_RTW_DEBUG_HW_TXSC */
	}

	if (use_hw_txsc) {
		/* HW shortcut */
		/* 802.3 frame */
		pkt_list->vir_addr = (xmit_skb[0]->data);
		pkt_list->length = xmit_skb[0]->len;
		psta->hw_txsc_hit++;
		#if 0
		RTW_PRINT("HWSC (%u) :%u, %u\n", idx,
			  txreq->mdata.smh_en, txreq->mdata.shcut_camid);
		#endif
		#ifdef CONFIG_RTW_ENABLE_HW_TX_AMSDU
		/* ToDo: condition to enable A-MSDU */
		if (1 /*&& psta->htpriv.tx_amsdu_enable */) {
			// RTW_PRINT("TS %u %u!\n", txreq->mdata.smh_en, txreq->mdata.hw_amsdu);
			txreq->mdata.hw_amsdu = 1;
			txreq->mdata.smh_en = 1;
			txreq->mdata.wdinfo_en = 0;
		}
		#endif /* CONFIG_RTW_ENABLE_HW_TX_AMSDU */
		txreq->mdata.wdinfo_en = 0;
	} else
	#endif /* CONFIG_RTW_ENABLE_HW_TXSC */
	{
		/* SW shortcut */

		/* rtw_core_wlan_fill_head */
		if (txreq->mdata.hw_seq_mode == 0) {
			/* generate sw seq */
			//priority = txreq->mdata.tid;
			psta->sta_xmitpriv.txseq_tid[priority]++;
			psta->sta_xmitpriv.txseq_tid[priority] &= 0xFFF;
			txreq->mdata.sw_seq = psta->sta_xmitpriv.txseq_tid[priority];

			/* if HW_HDR_CONV is on, define USE_ONE_WLHDR, update sw_seq in mdata
			   it will will update seq by hw_hdr_conv*/
			/* update sw_seq, amsdu bit in txsc_wlhdr cache */
			SetSeqNum(ptxsc_wlhdr, txreq->mdata.sw_seq);

			#ifdef CONFIG_RTW_A4_STA
			if(psta->flags & WLAN_STA_A4)
				qc = (u16 *)(ptxsc_wlhdr + WLAN_HDR_A4_LEN);
			else
			#endif
				qc = (u16 *)(ptxsc_wlhdr + WLAN_HDR_A3_LEN);
			SetPriority(qc, priority);

			#ifdef CONFIG_TXSC_AMSDU
			if (txsc_pkt->amsdu && txsc->txsc_amsdu)
				SetAMsdu(qc, 1);
			else
				SetAMsdu(qc, 0);
			#endif
			is_hdr_need_update = 1;
		}

#ifdef CONFIG_RTL_CFG80211_WAPI_SUPPORT
		/* Update PN and IV */
		if ((txreq->mdata.sec_type == RTW_ENC_WAPI) ||
		    (txreq->mdata.sec_type == RTW_ENC_GCMSMS4)) {
			rtw_wapi_get_iv(padapter, psta, psta->wapiStaInfo.keyIdx,
					ptxsc_wlhdr + (txreq->mdata.hdr_len << 1));
			is_hdr_need_update = 1;
		}
#endif

	/* WLAN header from cache */
	#ifdef USE_ONE_WLHDR

		wlhdr_copy_len = pkt_list0->length;

		#ifdef CONFIG_TXSC_AMSDU
		if (is_hdr_need_update) {
			pkt_list->vir_addr = ptxsc_wlhdr;
			pkt_list->length = wlhdr_copy_len;
			#ifdef CONFIG_PCI_HCI
			txsc_fill_txreq_phyaddr(padapter, pkt_list);
			#endif
			psta->txsc_wlhdr_copy++;
		} else
		#endif
		_rtw_memcpy(pkt_list, pkt_list0, sizeof(struct rtw_pkt_buf_list));

	#else /* USE_ONE_WLHDR */

		#ifdef CONFIG_TXSC_AMSDU
		if (txsc_pkt->amsdu && txsc->txsc_amsdu)
			wlhdr_copy_len = txsc->amsdu_wlhdr_len;/*(txsc_wlhdr_len - (SNAP_SIZE + sizeof(u16)));*/
		else
		#endif
			wlhdr_copy_len = txsc->ampdu_wlhdr_len;/*txsc_wlhdr_len;*/

		_rtw_memcpy(head, ptxsc_wlhdr, wlhdr_copy_len);


		if (psta->state & WIFI_SLEEP_STATE) {
//#ifdef CONFIG_ONE_TXQ
//		u8 q_idx = xmit_skb[0]->cb[_SKB_CB_QNUM];
//#else
			u8 q_idx = rtw_get_txq_idx(priority);
//#endif
			if (psta->uapsd_bitmap & BIT(q_idx)) {
				if (pstaxmitpriv->tx_pending_bitmap & psta->uapsd_bitmap)
					SetMData(head);
				else {
					qc = (u16 *)(head + WLAN_HDR_A3_LEN);
					SetEOSP(qc, 1);
				}
			} else {
				if (pstaxmitpriv->tx_pending_bitmap & ~ psta->uapsd_bitmap)
					SetMData(head);
			}
		}

		#ifdef CONFIG_RTW_A4_STA
		if(psta->flags & WLAN_STA_A4)
			qc = (u16 *)(head + WLAN_HDR_A4_LEN);
		else
		#endif
			qc = (u16 *)(head + WLAN_HDR_A3_LEN);
		if (GetPriority(qc) != priority)
			psta->txsc_priority_update++;
		SetPriority(qc, priority);

		/* fill wlhdr in pkt_list[0] */
		pkt_list->vir_addr = head;
		pkt_list->length = wlhdr_copy_len;
		#ifdef CONFIG_PCI_HCI
		#ifdef CONFIG_VW_REFINE
		if (txreq->is_map)
			txsc_fill_txreq_phyaddr(padapter, pkt_list);
		#endif
		#endif
		psta->txsc_wlhdr_copy++;

	#endif/* USE_ONE_WLHDR */

		txreq->total_len += pkt_list->length;
		txreq->pkt_cnt++;

		/* Payload w.o. ether header */ /* CONFIG_TXSC_AMSDU for multiple skb */
		for (i = 0; i < skb_cnt; i++) {
			pkt_list++;

			pkt_list->vir_addr = xmit_skb[i]->data + payload_offset;
			pkt_list->length = xmit_skb[i]->len - payload_offset;

			txreq->total_len += pkt_list->length;
			txreq->pkt_cnt++;
			#ifdef CONFIG_PCI_HCI
			#ifdef CONFIG_VW_REFINE
			if (txreq->is_map)
			#endif
			txsc_fill_txreq_phyaddr(padapter, pkt_list);
			#endif

			txreq_buf->pkt[i] = (u8 *)xmit_skb[i];
		}

		txreq->treq_type = RTW_PHL_TREQ_TYPE_CORE_TXSC;

		if (txsc_pkt->step == TXSC_APPLY) {
			psta->txsc_cache_hit++;
			txsc->txsc_cache_hit++;
			#ifdef CONFIG_RTW_A4_STA/* A4_TXSC */
			if (psta->flags & WLAN_STA_A4) {
				padapter->cnt_a4_txsc++;
				psta->txsc_a4_hit++;
				txsc->txsc_a4_hit++;
			}
			#endif
		}
		#ifdef CONFIG_TXSC_AMSDU
		else if (txsc_pkt->step == TXSC_AMSDU_APPLY) {
			psta->txsc_amsdu_hit++;
			txsc->txsc_cache_hit+=skb_cnt;
			#ifdef CONFIG_RTW_A4_STA/* A4_TXSC */
			if (psta->flags & WLAN_STA_A4) {
				padapter->cnt_a4_txsc_amsdu++;
				psta->txsc_a4_amsdu_hit++;
				txsc->txsc_a4_hit++;
			}
			#endif
		}
		#endif
		else
			psta->txsc_path_slow++;

		if(txsc_pkt->step == TXSC_APPLY
		#ifdef CONFIG_TXSC_AMSDU
			|| txsc_pkt->step == TXSC_AMSDU_APPLY
		#endif
			)
		{
			if(jiffies != txsc->last_hit_time)
			{
				unsigned long diff_time = 0;
				diff_time = CIRC_CNT_RTW(jiffies, txsc->last_hit_time, MAX_TIME_SIZE);
				if(diff_time >= padapter->txsc_time_duration)
				{
					txsc->cache_hit_average = txsc->txsc_cache_hit - txsc->last_hit_cnt;
					if(diff_time >= ((padapter->txsc_time_duration << 1) + padapter->txsc_time_duration))
					{
						txsc->cache_hit_average = 1;
					}
					else if(diff_time >= (padapter->txsc_time_duration << 1))
					{
						txsc->cache_hit_average = txsc->cache_hit_average >> 1;
						if(txsc->cache_hit_average == 0)
							txsc->cache_hit_average = 1;
					}
					txsc->last_hit_cnt = txsc->txsc_cache_hit;
					txsc->last_hit_time = jiffies;
				}
			}
		}

		#ifdef CONFIG_RTW_ENABLE_HW_TXSC
		/* Set to non-HW-TXSC if meta data was prepared for HW TXSC */
		if (1 || psta->hw_txsc_set) {
			txreq->mdata.smh_en = 0;
			txreq->mdata.hdr_len = pkt_list0->length;
			txreq->mdata.hw_amsdu = 0;
			txreq->mdata.wdinfo_en = 1;
		}
		#endif /* CONFIG_RTW_ENABLE_HW_TXSC */
	}
	/* SW shortcut --- */

	txreq_buf->pkt_cnt = skb_cnt;/* for recycle multiple skb */
	txreq->mdata.pktlen = txreq->total_len;
	txreq->shortcut_id = psta->txsc_entry_cache[idx].txsc_phl_id;

    /* send addbareq */
	txsc_issue_addbareq_cmd(padapter, priority, psta, _TRUE);

	/* fix rate */
	if (padapter->fix_rate != NO_FIX_RATE) {
		mdata->userate_sel = 1;
		mdata->f_rate = GET_FIX_RATE(padapter->fix_rate);
		mdata->f_gi_ltf = GET_FIX_RATE_SGI(padapter->fix_rate);
		if (!padapter->data_fb)
			mdata->dis_data_rate_fb = 1;
	} else if (psta->fixRate != 0 && psta->fixRate != NO_FIX_RATE) {
		mdata->userate_sel = 1;
		mdata->f_rate = GET_FIX_RATE(psta->fixRate);
		mdata->f_gi_ltf = GET_FIX_RATE_SGI(psta->fixRate);
		if (!padapter->data_fb)
			mdata->dis_data_rate_fb = 1;
	}
#ifdef CONFIG_LIFETIME_FEATURE
	if(padapter->registrypriv.wifi_mib.lifetime)
		txreq->mdata.life_time_sel = 1;
	else
		txreq->mdata.life_time_sel = 0;
#endif
#ifdef RTW_PHL_DBG_CMD
	/* Update force rate settings so force rate takes effects
	 * after shortcut cached
	 */
	if (padapter->txForce_enable) {
		if (padapter->txForce_rate != INV_TXFORCE_VAL) {
			txreq->mdata.f_rate = padapter->txForce_rate;
			txreq->mdata.userate_sel = 1;
			txreq->mdata.dis_data_rate_fb = 1;
			txreq->mdata.dis_rts_rate_fb = 1;
		}
		if (padapter->txForce_agg != INV_TXFORCE_VAL)
			txreq->mdata.ampdu_en = padapter->txForce_agg;
		if (padapter->txForce_aggnum != INV_TXFORCE_VAL)
			txreq->mdata.max_agg_num = padapter->txForce_aggnum;
		if (padapter->txForce_gi != INV_TXFORCE_VAL)
			txreq->mdata.f_gi_ltf = padapter->txForce_gi;
		if (padapter->txForce_ampdu_density != INV_TXFORCE_VAL)
			txreq->mdata.ampdu_density = padapter->txForce_ampdu_density;
		if (padapter->txForce_bw != INV_TXFORCE_VAL)
			txreq->mdata.f_bw = padapter->txForce_bw;
	} else {
		if (   padapter->txForce_aggnum != INV_TXFORCE_VAL
		    && padapter->txForce_aggnum != 0
		    && txreq->mdata.max_agg_num != (padapter->txForce_aggnum)) {
			if (txreq->mdata.max_agg_num != (padapter->txForce_aggnum)) {
				RTW_PRINT("max_agg_num <= %u (STA: %u)\n",
				          padapter->txForce_aggnum,
				          psta->phl_sta->asoc_cap.num_ampdu);
			}
			txreq->mdata.max_agg_num = padapter->txForce_aggnum;
			mdata->max_agg_num = padapter->txForce_aggnum;
		}

		if (    padapter->txForce_ampdu_density != INV_TXFORCE_VAL
		    && padapter->txForce_ampdu_density != txreq->mdata.ampdu_density) {
			if (txreq->mdata.ampdu_density != padapter->txForce_ampdu_density) {
				RTW_PRINT("ampdu_density <= %u (STA: %u/%u)\n",
				          padapter->txForce_ampdu_density,
				          psta->phl_sta->asoc_cap.ampdu_density,
				          psta->htpriv.rx_ampdu_min_spacing);
			}
			txreq->mdata.ampdu_density = padapter->txForce_ampdu_density;
			mdata->ampdu_density = padapter->txForce_ampdu_density;
		}
	}
#endif /* RTW_PHL_DBG_CMD */

	if (txreq->mdata.pktlen > 2300) {
        if (psta->cur_tx_data_rate > 0x180) /* MAC_AX_HE_NSS1_MCS0 */
                txreq->mdata.data_rty_lowest_rate = 0x180;
        else if (psta->cur_tx_data_rate > 0x100) /* MAC_AX_VHT_NSS1_MCS0 */
                txreq->mdata.data_rty_lowest_rate = 0x100;
        else { /* MAC_AX_MCS0 */
                txreq->mdata.data_rty_lowest_rate = 0x80;
        }
    }

	if (padapter->registrypriv.wifi_mib.stbc & BIT1)
		if (psta && psta->phl_sta){
			mdata->f_stbc =  _stbc_mapping(padapter, psta);
#ifdef TX_BEAMFORMING

	if (/*role_type == PHL_RTYPE_AP &&*/ is_primary_adapter(padapter)) {
		if(padapter->registrypriv.wifi_mib.txbf&&
			padapter->registrypriv.wifi_mib.txbfer && (psta->phl_sta->wmode & WLAN_MD_11AC)) {

			if (mdata->f_stbc)
				if (rtw_phl_check_bf_entry(padapter->dvobj->phl, psta->phl_sta) ==RTW_PHL_STATUS_SUCCESS)
					mdata->f_stbc= 0;
		}
	}
#endif		

		}
	_txsc_update_sec_iv(padapter, psta, txreq);

	/* for tx mdata debug */
	print_txreq_mdata(&txreq->mdata, __func__);
	/* for txreq debug */
#ifdef RTW_CORE_TX_MSDU_TRANSFER_IN_PHL
	if (!txreq->is_msdu)
#endif
	{
		extern u8 DBG_PRINT_TXREQ_ONCE;
		if(DBG_PRINT_TXREQ_ONCE)
			printk("txsc_pkt->amsdu:%d\n", txsc_pkt->amsdu);
		print_txreq_pklist(NULL, txsc_pkt->ptxreq, (u8 *)xmit_skb[0]->data, __func__);
	}

exit:
	#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
	if(res == _FAIL){
		psta->core_txsc_apply_fail++;
	}
	#endif
	return res;
}

void txsc_free_txreq(_adapter *padapter, struct rtw_xmit_req *txreq)
{
	struct xmit_txreq_buf *ptxreq_buf = NULL;
	struct sta_info *psta = NULL;
	struct sk_buff *tx_skb = NULL;
	_queue *queue = NULL;
	u8 i;

	if (txreq != NULL)
		ptxreq_buf = txreq->os_priv;
	else
		return;

	#ifdef CONFIG_PCI_HCI
	#ifdef USE_ONE_WLHDR
	if (txreq->mdata.hw_seq_mode == 1)
		txsc_recycle_txreq_phyaddr(padapter, txreq);
	else
	#endif
		core_recycle_txreq_phyaddr(padapter, txreq);
	#endif

	txsc_recycle_entry_use_cnt(padapter, txreq);

	if (!ptxreq_buf) {
		RTW_ERR("%s: NULL ptxreq_buf !!\n", __func__);
		rtw_warn_on(1);
		return;
	} else
		psta = ptxreq_buf->psta;

	txreq->os_priv = NULL;
	txreq->pkt_list = NULL;
	txreq->treq_type = RTW_PHL_TREQ_TYPE_NORMAL;
	ptxreq_buf->psta = NULL;

	/* this must be protected in  spinlock section
	 * if (ptxreq_buf->pkt_cnt == 0)
 	 *	RTW_PRINT("[ERR][%s:%d] pkt_cnt = 0 is a fatel error, plz check\n", __func__, __LINE__);
         */

#ifdef CONFIG_VW_REFINE
	if (padapter->dvobj->tx_mode == 1 && ptxreq_buf->pkt_cnt == 0)
		RTW_PRINT("[ERR][%s:%d] pkt_cnt = 0 is a fatel error, plz check\n", __FUNCTION__, __LINE__);
#endif

	for (i = 0; i < ptxreq_buf->pkt_cnt; i++) {
		tx_skb = (struct sk_buff *)ptxreq_buf->pkt[i];
		if (tx_skb) {
			rtw_os_pkt_complete(padapter, tx_skb);
		}
		else
			RTW_DBG("%s:tx recyele: tx_skb=NULL\n", __func__);
		/* ptxreq_buf->pkt[i] = NULL; */
	}

#ifdef CONFIG_WFA_OFDMA_Logo_Test
	if(psta) {
		psta->core_current_txreq_cnt--;
		if(psta->core_current_txreq_cnt < 0)
			psta->core_current_txreq_cnt = 0;
	}
#endif

	queue = padapter->pfree_txreq_queue;
	_rtw_spinlock_bh(&queue->lock);
	rtw_list_insert_tail(&ptxreq_buf->list, get_list_head(queue));
	queue->qlen++;
	_rtw_spinunlock_bh(&queue->lock);
}

#if 0/* tmp mark this debug function */
void txsc_debug_sc_entry(_adapter *padapter, struct xmit_frame *pxframe, struct txsc_pkt_entry *txsc_pkt)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct rtw_pkt_buf_list *pkt_list = NULL;
	struct rtw_pkt_buf_list *txsc_pkt_list = NULL;
	struct rtw_xmit_req *txreq = pxframe->phl_txreq;
	struct rtw_xmit_req *ptxsc_txreq = txsc_pkt->ptxreq;
	u8 i;

	if (txsc_pkt->step != TXSC_DEBUG)
		return;

	if (pxmitpriv->txsc_debug_mask&BIT2) {

		RTW_PRINT("\n\nNormal Path TXREQ: %p\n\n", txreq);
		txsc_dump_data((u8 *)txreq, sizeof(struct rtw_xmit_req), "txreq:");
		RTW_PRINT("os_priv=%p\n", txreq->os_priv);
		RTW_PRINT("treq_type=%d shortcut_id=%d\n", txreq->treq_type, txreq->shortcut_id);
		RTW_PRINT("total_len=%d pkt_cnt=%d\n", txreq->total_len, txreq->pkt_cnt);
		pkt_list = (struct rtw_pkt_buf_list *)txreq->pkt_list;
		for (i = 0; i < txreq->pkt_cnt; i++) {
			RTW_PRINT("[%d]", i);
			txsc_dump_data((u8 *)pkt_list, sizeof(struct rtw_pkt_buf_list), "pklist");
			txsc_dump_data((u8 *)pkt_list->vir_addr, pkt_list->length, "pkt_list->vir_addr");
			pkt_list++;
		}
		RTW_PRINT("mdata: pktlen=%d sw_seq=%d\n", txreq->mdata.pktlen, txreq->mdata.sw_seq);

		RTW_PRINT("\n\nShortcut Path TXREQ: %p\n\n", ptxsc_txreq);
		txsc_dump_data((u8 *)ptxsc_txreq, sizeof(struct rtw_xmit_req), "ptxsc_txreq:");
		RTW_PRINT("os_priv=%p\n", ptxsc_txreq->os_priv);
		RTW_PRINT("treq_type=%d shortcut_id=%d\n", ptxsc_txreq->treq_type, ptxsc_txreq->shortcut_id);
		RTW_PRINT("total_len=%d pkt_cnt=%d\n", ptxsc_txreq->total_len, ptxsc_txreq->pkt_cnt);
		txsc_pkt_list = (struct rtw_pkt_buf_list *)ptxsc_txreq->pkt_list;
		for (i = 0; i < txreq->pkt_cnt; i++) {
			RTW_PRINT("[%d]", i);
			txsc_dump_data((u8 *)txsc_pkt_list, sizeof(struct rtw_pkt_buf_list), "pklist");
			txsc_dump_data((u8 *)txsc_pkt_list->vir_addr, txsc_pkt_list->length, "pkt_list->vir_addr");
			txsc_pkt_list++;
		}
		RTW_PRINT("mdata: pktlen=%d sw_seq=%d\n", ptxsc_txreq->mdata.pktlen, ptxsc_txreq->mdata.sw_seq);

	} else {
		if (!_rtw_memcmp(&txreq->mdata, &ptxsc_txreq->mdata, sizeof(struct rtw_t_meta_data))) {
			txsc_dump_data((u8 *)&txreq->mdata, sizeof(struct rtw_t_meta_data), "txreq->mdata");
			txsc_dump_data((u8 *)&ptxsc_txreq->mdata, sizeof(struct rtw_t_meta_data), "ptxsc_txreq->mdata");
		}

		pkt_list = (struct rtw_pkt_buf_list *)txreq->pkt_list;
		txsc_pkt_list = (struct rtw_pkt_buf_list *)ptxsc_txreq->pkt_list;
		if (pkt_list->length != txsc_pkt_list->length) {
			txsc_dump_data((u8 *)pkt_list, sizeof(struct rtw_pkt_buf_list), "pkt_list[0]");
			txsc_dump_data((u8 *)txsc_pkt_list, sizeof(struct rtw_pkt_buf_list), "txsc_pkt_list[0]");
			txsc_dump_data((u8 *)pkt_list->vir_addr, pkt_list->length, "pkt_list[0]->vir_addr");
				txsc_dump_data(txsc_pkt_list->vir_addr, pkt_list->length, "txsc_pkt_list[0]->vir_addr");
		} else if (pkt_list->length == txsc_pkt_list->length) {
			if (!_rtw_memcmp(pkt_list->vir_addr, txsc_pkt_list->vir_addr, pkt_list->length)) {
				txsc_dump_data((u8 *)pkt_list->vir_addr, pkt_list->length, "pkt_list[0]->vir_addr");
				txsc_dump_data((u8 *)txsc_pkt_list->vir_addr, pkt_list->length, "txsc_pkt_list[0]->vir_addr");
			}
		}
	}

	/* DO NOT WD CACHE */
	txreq->shortcut_id = 0;
	txreq->treq_type = RTW_PHL_TREQ_TYPE_NORMAL;
}
#endif

#ifdef CONFIG_TXSC_AMSDU
void txsc_amsdu_clear(_adapter *padapter)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	int i;

	RTW_PRINT("[amsdu] clear amsdu counter\n");

	for (i = 0; i < 4; i++) {
		pxmitpriv->cnt_txsc_amsdu_enq[i] = 0;
		pxmitpriv->cnt_txsc_amsdu_enq_abort[i] = 0;
		pxmitpriv->cnt_txsc_amsdu_deq[i] = 0;
	}
	pxmitpriv->cnt_txsc_amsdu_deq_empty = 0;
	pxmitpriv->cnt_txsc_amsdu_deq_ampdu = 0;
	pxmitpriv->cnt_txsc_amsdu_timeout_deq_empty = 0;

	for (i = 0; i < (MAX_TXSC_SKB_NUM + 1); i++) {
		pxmitpriv->cnt_txsc_amsdu_dump[i] = 0;
		pxmitpriv->cnt_txsc_amsdu_timeout_dump[i] = 0;
	}
	for(i = 0; i < ARRAY_SIZE(pxmitpriv->cnt_txsc_amsdu_timeout_ok); i++) {
		pxmitpriv->cnt_txsc_amsdu_timeout_ok[i] = 0;
		pxmitpriv->cnt_txsc_amsdu_timeout_fail[i] = 0;
	}
}
void txsc_amsdu_dump(_adapter *padapter, void *m)
{
	struct sta_priv	*pstapriv = &padapter->stapriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_info *psta = NULL;
	u8 i, j;
	_list *plist, *phead;

	RTW_PRINT_SEL(m, "[amsdu] txsc amsdu enable: %d\n", pxmitpriv->txsc_amsdu_enable);
	RTW_PRINT_SEL(m, "[amsdu] txsc amsdu mode: %d\n", pxmitpriv->txsc_amsdu_mode);
	RTW_PRINT_SEL(m, "[amsdu] amsdu num: %d\n", padapter->tx_amsdu);
	RTW_PRINT_SEL(m, "[amsdu] amsdu rate: %d\n", padapter->tx_amsdu_rate);
	RTW_PRINT_SEL(m, "[amsdu] txsc_amsdu_force_num: %d\n", pxmitpriv->txsc_amsdu_force_num);
	RTW_PRINT_SEL(m, "[amsdu] amsdu max num: %d\n", MAX_TXSC_SKB_NUM);
	RTW_PRINT_SEL(m, "[amsdu] amsdu max queue num: %d\n", MAX_AMSDU_ENQ_NUM);
#ifdef RTW_CORE_TX_MSDU_TRANSFER_IN_PHL
	RTW_PRINT_SEL(m, "[amsdu] is_map: %d, is_msdu: %d\n", padapter->is_map, padapter->is_msdu);
#else
	RTW_PRINT_SEL(m, "[amsdu] is_map: %d\n", padapter->is_map);
#endif
#ifdef CONFIG_AMSDU_HW_TIMER
	RTW_PRINT_SEL(m, "[amsdu] hw timer enable: %d\n", padapter->amsdu_hw_timer_enable);
	RTW_PRINT_SEL(m, "[amsdu] hw timer timeout value: %d\n", padapter->amsdu_hw_timeout);
	RTW_PRINT_SEL(m, "[amsdu] hw timer irq cnt: %d\n", padapter->amsdu_hw_irq_cnt);
#endif
	RTW_PRINT_SEL(m, "\n");
	for (i = 0; i < 4; i++) {
		if (pxmitpriv->cnt_txsc_amsdu_enq[i] != 0 ||
			pxmitpriv->cnt_txsc_amsdu_deq[i] != 0 ||
			pxmitpriv->cnt_txsc_amsdu_enq_abort[i] != 0) {
			RTW_PRINT_SEL(m, "[amsdu] ac[%d] enq/deq/abort: %d / %d / %d\n", i,
				pxmitpriv->cnt_txsc_amsdu_enq[i],
				pxmitpriv->cnt_txsc_amsdu_deq[i],
				pxmitpriv->cnt_txsc_amsdu_enq_abort[i]);
		}
	}

	RTW_PRINT_SEL(m, "\n");
	for (i = 0; i < (MAX_TXSC_SKB_NUM + 1); i++) {
		if (pxmitpriv->cnt_txsc_amsdu_dump[i] != 0)
			RTW_PRINT_SEL(m, "[amsdu] pkt_num:%d tx: %d\n", i, pxmitpriv->cnt_txsc_amsdu_dump[i]);
	}
	RTW_PRINT_SEL(m, "[amsdu] deq empty/ampdu: %d / %d\n",
		pxmitpriv->cnt_txsc_amsdu_deq_empty, pxmitpriv->cnt_txsc_amsdu_deq_ampdu);
	RTW_PRINT_SEL(m, "[amsdu] ps enq/deq: %d / %d\n",
		pxmitpriv->cnt_txsc_amsdu_enq_ps, pxmitpriv->cnt_txsc_amsdu_deq_ps);

	RTW_PRINT_SEL(m, "\n");
	for (i = 0; i < (MAX_TXSC_SKB_NUM + 1); i++) {
		if (pxmitpriv->cnt_txsc_amsdu_timeout_dump[i] != 0)
			RTW_PRINT_SEL(m, "[amsdu] pkt_num:%d timout_tx: %d\n", i, pxmitpriv->cnt_txsc_amsdu_timeout_dump[i]);
	}
	RTW_PRINT_SEL(m, "[amsdu] timeout_deq_empty: %d\n", pxmitpriv->cnt_txsc_amsdu_timeout_deq_empty);

	RTW_PRINT_SEL(m, "\n");
	for (i = 0; i < ARRAY_SIZE(pxmitpriv->cnt_txsc_amsdu_timeout_ok); i++) {
		if (pxmitpriv->cnt_txsc_amsdu_timeout_ok[i] != 0 ||
			pxmitpriv->cnt_txsc_amsdu_timeout_fail[i] != 0 ||
			pxmitpriv->cnt_txsc_amsdu_timeout_bypass[i] != 0 ||
			pxmitpriv->cnt_txsc_amsdu_timeout_abort[i] != 0)
			RTW_PRINT_SEL(m, "[amsdu] ac[%d] timeout ok/fail/bypass/abort: %d / %d / %d / %d\n", i,
				pxmitpriv->cnt_txsc_amsdu_timeout_ok[i],
				pxmitpriv->cnt_txsc_amsdu_timeout_fail[i],
				pxmitpriv->cnt_txsc_amsdu_timeout_bypass[i],
				pxmitpriv->cnt_txsc_amsdu_timeout_abort[i]);
	}

	RTW_PRINT_SEL(m, "\n");
	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);

			if (!psta || (psta == padapter->self_sta))
				continue;

			RTW_PRINT_SEL(m, "[%d] STA[%pM]->txq[ac] ampdu_num tx:%d rx:%d\n", i,
				psta->phl_sta->mac_addr, psta->phl_sta->asoc_cap.num_ampdu, psta->recvreorder_ctrl[0].ampdu_size);
			/* AMSDU_BY_SIZE */
			RTW_PRINT_SEL(m, "	[amsdu] cap:%d, num:%d, sz:%d (max num:%d, sz:%d)\n",
				psta->phl_sta->asoc_cap.max_amsdu_len,
				psta->txsc_amsdu_num, psta->txsc_amsdu_size,
				psta->txsc_amsdu_max, psta->txsc_amsdu_max_size);
			#ifdef CONFIG_AMSDU_HW_TIMER
			RTW_PRINT_SEL(m, "	[amsdu] hw timeout: %d\n", psta->txsc_amsdu_hw_timeout);
			#endif
			for (j = 0; j < 4; j++) {
				if (psta->amsdu_txq[j].cnt != 0 ||
					psta->amsdu_txq[j].rptr !=0 ||
					psta->amsdu_txq[j].wptr !=0 ||
					psta->amsdu_txq[j].skb_qlen != 0)
					RTW_PRINT_SEL(m, "	amsdu_txq[%d]: cnt:%d, rptr:%d, wptr:%d, qlen: %d\n",
						j, psta->amsdu_txq[j].cnt, psta->amsdu_txq[j].rptr, psta->amsdu_txq[j].wptr, psta->amsdu_txq[j].skb_qlen);
			}
			RTW_PRINT_SEL(m, "\n");
		}
	}
}

void txsc_amsdu_reset(_adapter *padapter)
{
	struct sta_priv	*pstapriv = &padapter->stapriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_info *psta = NULL;
	u8 i, j;

	for (i = 0; i < 4; i++) {
		pxmitpriv->cnt_txsc_amsdu_enq[i] = 0;
		pxmitpriv->cnt_txsc_amsdu_deq[i] = 0;
		pxmitpriv->cnt_txsc_amsdu_enq_abort[i] = 0;
	}

	RTW_PRINT("\n");
	for (i = 0; i < (MAX_TXSC_SKB_NUM + 1); i++) {
		pxmitpriv->cnt_txsc_amsdu_dump[i] = 0;
	}

}

s32 txsc_amsdu_timeout_tx(struct sta_info *psta, u8 ac)
{
	struct xmit_frame *pxframe = NULL;
	_adapter *padapter = psta->padapter;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct txsc_pkt_entry txsc_pkt;
	struct sk_buff *pskb = NULL;
	struct txsc_amsdu_swq *txq = &psta->amsdu_txq[ac];
#ifdef CONFIG_AMSDU_HW_TIMER
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	_adapter *pri_adapter = dvobj_get_primary_adapter(dvobj);
#endif
	u8 i, res;
	u8 deq_again = 0, bypass_timeout_tx = 0;
	u32 throughput_th_high = (WIFI_ROLE_IS_ON_5G(padapter)) ? 600 : 300;;

#ifdef CONFIG_VW_REFINE
	if (padapter->dvobj->tx_mode == 0)
#endif
	{
		/* amsdu timer status change should be protected by lock*/
		_txsc_one_spinlock_bh(&pxmitpriv->txsc_lock, &txq->txsc_amsdu_lock);
		txsc_amsdu_set_timer_status(psta, ac, TXSC_AMSDU_TIMER_TIMEOUT);
		if (txq->cnt > 0)
			pskb = txq->skb_q[txq->rptr];

#ifdef CONFIG_AMSDU_HW_TIMER
		if(!pri_adapter->amsdu_hw_timer_enable)
#endif
		{
			/* if in tx max throughput test, do not timeout tx when pkt is not reach max amsdu num */
			if (psta->txsc_amsdu_num >= psta->txsc_amsdu_max &&
				txq->cnt < psta->txsc_amsdu_num &&
				((psta->sta_stats.tx_tp_kbits>>10) > throughput_th_high
#if defined(CONFIG_RTW_MULTI_AP)
				|| (((GET_MAP_BSS_TYPE(padapter) & MULTI_AP_BACKHAUL_STA) || (GET_MAP_BSS_TYPE(padapter) & MULTI_AP_BACKHAUL_BSS)) 
					&& ((psta->sta_stats.tx_tp_kbits>>10) > (throughput_th_high/2)))
#endif
			)) {
				bypass_timeout_tx = 1;
				_txsc_one_spinunlock_bh(&pxmitpriv->txsc_lock, &txq->txsc_amsdu_lock);
				goto exit_timeout_tx;
			}
		}

		if (pskb == NULL || bypass_timeout_tx == 1) {
			_txsc_one_spinunlock_bh(&pxmitpriv->txsc_lock, &txq->txsc_amsdu_lock);
			return SUCCESS;
		}
		_txsc_one_spinunlock_bh(&pxmitpriv->txsc_lock, &txq->txsc_amsdu_lock);
	}

	txsc_amsdu_timeout_init_pkt_entry(padapter, &txsc_pkt, psta, ac);

#ifdef CONFIG_VW_REFINE
	if (padapter->dvobj->tx_mode == 1) {
		return SUCCESS;
	}
#endif

	barrier();

	_txsc_one_spinlock_bh(&pxmitpriv->txsc_lock, NULL);

	if (txsc_get_sc_entry(padapter, pskb /*txsc_pkt.xmit_skb[0]*/, &txsc_pkt) != _SUCCESS) {
		RTW_TX_TRACE_CORE(padapter, pxframe, txsc_pkt, i, txsc_pkt.skb_cnt);
		goto abort_core_tx;
	}

	/* this is a workaround for double free skb */
	if (txsc_pkt.txsc_id == 0xff) {
		RTW_TX_TRACE_CORE(padapter, pxframe, txsc_pkt, i, txsc_pkt.skb_cnt);
		padapter->xmitpriv.cnt_txsc_amsdu_dfree++;
		txsc_release_entry_use_cnt(padapter, &txsc_pkt);
		goto abort_core_tx;
	}

	#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
	psta->core_txsc_apply_cnt_2++;
	#endif

	do {
		deq_again = 0;
		res = txsc_apply_sc_cached_entry(padapter, &txsc_pkt, 0);
		if (res == _FAIL) {
			RTW_TX_TRACE_CORE(padapter, pxframe, txsc_pkt, i, txsc_pkt.skb_cnt);
			txsc_release_entry_use_cnt(padapter, &txsc_pkt);
			goto abort_core_tx;
		}

		if (core_tx_call_phl(padapter, NULL, &txsc_pkt) == FAIL) {
			RTW_TX_TRACE_CORE(padapter, pxframe, txsc_pkt, i, txsc_pkt.skb_cnt);
			txsc_release_entry_use_cnt(padapter, &txsc_pkt);
			goto abort_core_tx;
		}

	#ifdef CONFIG_BW160M_EXTREME_THROUGHPUT_RX
		if (txsc_pkt.amsdu == true) {
			deq_again = txsc_amsdu_deque_check(pxmitpriv, psta, txsc_pkt.ac);
		}

		if (deq_again) {
			// reset some variables in txsc_pkt struct
			txsc_pkt.ptxreq = NULL;
			for (i = 0; i < MAX_TXSC_SKB_NUM; i++) {
				txsc_pkt.xmit_skb[i] = NULL;
			}
			txsc_pkt.skb_cnt = 0;

			/* apply the same TXSC entry more than once */
			txsc_acquire_entry_use_cnt(padapter, &txsc_pkt);
		}
	#endif
	} while(deq_again);

	_txsc_one_spinunlock_bh(&pxmitpriv->txsc_lock, NULL);

	pxmitpriv->cnt_txsc_amsdu_timeout_ok[ac]++;

	return SUCCESS;

abort_core_tx:
	_txsc_one_spinunlock_bh(&pxmitpriv->txsc_lock, NULL);

	if(txsc_pkt.skb_cnt != 0) {
		DBG_COUNTER_NUM(padapter->tx_logs.core_tx_txsc_amsdu_timeout_err, txsc_pkt.skb_cnt);
		DBG_COUNTER_NUM(padapter->tx_logs.core_tx_err_drop, txsc_pkt.skb_cnt);
		DBG_COUNTER_NUM(padapter->tx_logs.core_tx_ex_err_drop, txsc_pkt.skb_cnt);
		padapter->xmitpriv.tx_drop += txsc_pkt.skb_cnt;
	}

	if (txsc_pkt.ptxreq) {
		txsc_pkt.ptxreq->mdata.macid = txsc_pkt.psta->phl_sta->macid;
		txsc_free_txreq(padapter, txsc_pkt.ptxreq);
	} else {
		for (i = 0; i < txsc_pkt.skb_cnt; i++) {
			if (txsc_pkt.xmit_skb[i])
				rtw_os_pkt_complete(padapter, txsc_pkt.xmit_skb[i]);
		}
	}

exit_timeout_tx:

#ifdef CONFIG_VW_REFINE
	if (padapter->dvobj->tx_mode == 0)
#endif
	{
#ifdef CONFIG_AMSDU_HW_TIMER
		if(pri_adapter->amsdu_hw_timer_enable)
		{
			_rtw_spinlock_bh(&txq->txsc_amsdu_lock);
			if(txq->cnt > 0 && !txq->is_hw_timer_pending)
				txsc_amsdu_set_hw_timer(psta, ac);
			_rtw_spinunlock_bh(&txq->txsc_amsdu_lock);
		}
		else
#endif
		{
			//padapter->cnt_txsc_amsdu_timeout_fail[ac]++;
			_rtw_spinlock_bh(&txq->txsc_amsdu_lock);
			if(txq->cnt > 0 && txsc_amsdu_get_timer_status(psta, ac) != TXSC_AMSDU_TIMER_SETTING) {
				txsc_amsdu_set_timer_status(psta, ac, TXSC_AMSDU_TIMER_SETTING);
				txsc_amsdu_set_timer(psta, ac);
			}
			_rtw_spinunlock_bh(&txq->txsc_amsdu_lock);
		}
	}

	if (bypass_timeout_tx == 1) {
		pxmitpriv->cnt_txsc_amsdu_timeout_bypass[ac]++;
		return SUCCESS;
	} else {
		pxmitpriv->cnt_txsc_amsdu_timeout_fail[ac]++;
		return FAIL;
	}
}
#endif

#ifdef CONFIG_80211N_HT
static u8 txsc_issue_addbareq_check(_adapter *padapter, u8 issue_when_busy)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct registry_priv *pregistry = &padapter->registrypriv;

	if (pregistry->tx_quick_addba_req == 0) {
		if ((issue_when_busy == _TRUE) && (pmlmepriv->LinkDetectInfo.bBusyTraffic == _FALSE))
			return _FALSE;

		if (pmlmepriv->LinkDetectInfo.NumTxOkInPeriod < 100)
			return _FALSE;
	}

	return _TRUE;
	}

void txsc_issue_addbareq_cmd(_adapter *padapter, u8 priority, struct sta_info *psta, u8 issue_when_busy)
{
	u8 issued;
	struct ht_priv	*phtpriv;
	u8 *mac_addr = psta->phl_sta->mac_addr;

	if(padapter->self_sta == psta)
		return;

	if (txsc_issue_addbareq_check(padapter, issue_when_busy) == _FALSE)
		return;

	phtpriv = &psta->htpriv;

	if ((phtpriv->ht_option == _TRUE) && (phtpriv->ampdu_enable == _TRUE)) {
		issued = (phtpriv->agg_enable_bitmap >> priority) & 0x1;
		issued |= (phtpriv->candidate_tid_bitmap >> priority) & 0x1;

		if (issued == 0 && mac_addr) {
			RTW_INFO("rtw_issue_addbareq_cmd, p=%d\n", priority);
			psta->htpriv.candidate_tid_bitmap |= BIT((u8)priority);
			rtw_addbareq_cmd(padapter, (u8) priority, mac_addr);
		}
	}

}
#endif /* CONFIG_80211N_HT */

void ieee8023_header_to_rfc1042_txsc(struct sk_buff *skb, bool add_pad)
{
	void *data;
	u8 *data1;
	int pad;
	__be16 len;
	const int headroom = SNAP_SIZE + 2;
	u16 *da, *sa;

	if (!skb)
		return;

	if (skb_headroom(skb) < headroom) {
		RTW_WARN("%s: headroom=%d isn't enough\n", __func__, skb_headroom(skb));
		if (pskb_expand_head(skb, headroom, 0, GFP_ATOMIC)) {
			RTW_ERR("%s: no headroom=%d for skb\n",
				__func__, headroom);
			return;
		}
	} else if (skb_cloned(skb)) {
		if (pskb_expand_head(skb, 0, 0, GFP_ATOMIC)) {
			RTW_ERR("%s: failed to pskb_expand_head\n",
				__func__);
			return;
		}
	}

	data = skb_push(skb, SNAP_SIZE + 2);

	da = (data + SNAP_SIZE + 2);
	sa = (data + SNAP_SIZE + 2 + ETH_ALEN);
	CP_MAC_ADDR((u8*)data, (u8*)da);
	CP_MAC_ADDR((u8*)data+ETH_ALEN, (u8*)sa);

	data1 = (u8*)(data + 2 * ETH_ALEN);
	len = cpu_to_be16(skb->len - 2 * ETH_ALEN - 2);
	//memcpy(data, &len, 2);
	data1[0] = *(u8 *)(&len);
	data1[1] = *((u8 *)&len + 1);
	//memcpy(data + 2, rtw_rfc1042_header, SNAP_SIZE);
	data1[2] = rtw_rfc1042_header[0];
	data1[3] = rtw_rfc1042_header[1];
	data1[4] = rtw_rfc1042_header[2];
	data1[5] = rtw_rfc1042_header[3];
	data1[6] = rtw_rfc1042_header[4];
	data1[7] = rtw_rfc1042_header[5];

	if (add_pad && (skb->len & (4 - 1))) {
		pad = 4 - (skb->len & (4 - 1));
		if (skb_tailroom(skb) < pad) {
			RTW_ERR("%s: no tailroom=%d for skb\n",
				__func__, pad);
			return;
		}
		skb_put(skb, pad); // do padding, not zero padding for CPU loading
		//rtw_skb_put_zero(skb, pad);
	}
}

u8 txsc_update_sec_cam_idx(_adapter *padapter, struct sta_info *psta, u8 keyid)
{
	struct dvobj_priv *d = adapter_to_dvobj(padapter);
	void *phl = GET_HAL_INFO(d);
	u8 sec_cam_id = 0;

	if (keyid >= 4)
		sec_cam_id = rtw_phl_get_sec_cam_idx(phl, psta->phl_sta, keyid,
			RTW_SEC_KEY_BIP);
	else
		sec_cam_id = rtw_phl_get_sec_cam_idx(phl, psta->phl_sta, keyid,
			RTW_SEC_KEY_UNICAST);

	return sec_cam_id;
}

void txsc_acquire_entry_use_cnt(_adapter *padapter,
				struct txsc_pkt_entry *txsc)
{
	struct txsc_entry *entry = NULL;

	if (!padapter || !txsc)
		return;

	if (txsc->psta && (txsc->txsc_id < CORE_TXSC_ENTRY_NUM)) {
		entry = &txsc->psta->txsc_entry_cache[txsc->txsc_id];

		_txsc_spinlock_bh(&padapter->xmitpriv.txsc_lock);

		if (entry->txsc_is_used) {
			entry->use_cnt++;
		} else {
			RTW_ERR("%s: not used!", __func__);
			rtw_dump_stack();
		}

		_txsc_spinunlock_bh(&padapter->xmitpriv.txsc_lock);
	}
}

void txsc_release_entry_use_cnt(_adapter *padapter,
				struct txsc_pkt_entry *txsc)
{
	struct txsc_entry *entry = NULL;

	if (!padapter || !txsc)
		return;

	if (txsc->psta && (txsc->txsc_id < CORE_TXSC_ENTRY_NUM)) {
		entry = &txsc->psta->txsc_entry_cache[txsc->txsc_id];

		_txsc_spinlock_bh(&padapter->xmitpriv.txsc_lock);

		if (entry->txsc_is_used && entry->use_cnt) {
			entry->use_cnt--;
		} else {
			RTW_ERR("%s: not used or overflow!", __func__);
			rtw_dump_stack();
		}

		_txsc_spinunlock_bh(&padapter->xmitpriv.txsc_lock);
	}

	txsc->txsc_id = 0xff;
	if (txsc->ptxreq)
		txsc->ptxreq->shortcut_id = 0xff;
}

void txsc_recycle_entry_use_cnt(_adapter *padapter,
				struct rtw_xmit_req *txreq)
{
	struct xmit_txreq_buf *txreq_buf = NULL;
	struct txsc_entry *entry = NULL;

	if (!padapter || !txreq)
		return;

	txreq_buf = (struct xmit_txreq_buf *)txreq->os_priv;
	if (txreq_buf && txreq_buf->psta &&
	    (txreq->treq_type == RTW_PHL_TREQ_TYPE_CORE_TXSC) &&
	    (txreq->shortcut_id < CORE_TXSC_ENTRY_NUM)) {
		entry = &txreq_buf->psta->txsc_entry_cache[txreq->shortcut_id];

		_rtw_spinlock_bh(&padapter->xmitpriv.txsc_lock);

		if (entry->txsc_is_used && entry->use_cnt) {
			entry->use_cnt--;
		} else {
			RTW_ERR("%s: not used or overflow!", __func__);
			//rtw_dump_stack();
		}

		_rtw_spinunlock_bh(&padapter->xmitpriv.txsc_lock);
	}

	txreq->shortcut_id = 0xff;
}

#ifdef CONFIG_PHL_TXSC
void _rtw_clear_phl_txsc(struct rtw_phl_stainfo_t *phl_sta)
{
	if (phl_sta != NULL)
		_rtw_memset(phl_sta->phl_txsc, 0x0, sizeof(struct phl_txsc_entry) * PHL_TXSC_ENTRY_NUM);
}
#endif

void core_txsc_clean_entry(_adapter *adapter, struct sta_info *sta)
{
#ifdef USE_ONE_WLHDR
	int i = 0, j = 0;
#endif /* USE_ONE_WLHDR */

	_rtw_spinlock_bh(&adapter->xmitpriv.txsc_lock);

	sta->txsc_cache_num = 0;
	sta->txsc_cur_idx = 0;
	sta->txsc_cache_idx = 0;
	sta->txsc_cache_hit = 0;
	sta->txsc_a4_hit = 0;
#ifdef CONFIG_TXSC_AMSDU
	sta->txsc_amsdu_hit = 0;
	sta->txsc_a4_amsdu_hit = 0;
#endif /* CONFIG_TXSC_AMSDU */
	sta->txsc_cache_miss = 0;
	sta->txsc_path_slow = 0;
#ifdef CONFIG_RTW_ENABLE_HW_TXSC
	sta->hw_txsc_hit = 0;
	sta->hw_txsc_set = 0;
#endif /* CONFIG_RTW_ENABLE_HW_TXSC */

	/* reset txsc entry */
#ifdef USE_ONE_WLHDR
	for (i = 0; i < CORE_TXSC_ENTRY_NUM; i++) {
		for (j = 0; j < WMM_NUM * WLHDR_NUM; j++) {
			if (sta->txsc_entry_cache[i].txsc_wlhdr[j]) {
				rtw_mfree(sta->txsc_entry_cache[i].txsc_wlhdr[j], CORE_TXSC_WLHDR_SIZE);
				sta->txsc_entry_cache[i].txsc_wlhdr[j] = NULL;
			}
		}
	}
#endif /* USE_ONE_WLHDR */

	_rtw_memset(sta->txsc_entry_cache, 0, sizeof(struct txsc_entry) * CORE_TXSC_ENTRY_NUM);
#ifdef CONFIG_PHL_TXSC
	_rtw_clear_phl_txsc(sta->phl_sta);
#endif /* CONFIG_PHL_TXSC */

	_rtw_spinunlock_bh(&adapter->xmitpriv.txsc_lock);
}

#endif /* CONFIG_CORE_TXSC */
