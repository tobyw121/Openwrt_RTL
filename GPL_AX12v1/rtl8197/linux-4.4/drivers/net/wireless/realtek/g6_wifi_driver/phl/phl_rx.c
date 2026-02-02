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
#define _PHL_RX_C_
#include "phl_headers.h"


u8 DBG_PRINT_RXPKT_ONCE;

struct rtw_phl_rx_pkt *rtw_phl_query_phl_rx(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_rx_pkt_pool *rx_pkt_pool = NULL;
	struct rtw_phl_rx_pkt *phl_rx = NULL;

	rx_pkt_pool = (struct phl_rx_pkt_pool *)phl_info->rx_pkt_pool;

	_os_spinlock(drv_priv, &rx_pkt_pool->idle_lock, _bh, NULL);

	if (false == list_empty(&rx_pkt_pool->idle)) {
		phl_rx = list_first_entry(&rx_pkt_pool->idle,
					struct rtw_phl_rx_pkt, list);
		list_del(&phl_rx->list);
		rx_pkt_pool->idle_cnt--;
	}


	_os_spinunlock(drv_priv, &rx_pkt_pool->idle_lock, _bh, NULL);

	return phl_rx;
}

u8 rtw_phl_is_phl_rx_idle(struct phl_info_t *phl_info)
{
	struct phl_rx_pkt_pool *rx_pkt_pool = NULL;
	u8 res = false;

	rx_pkt_pool = (struct phl_rx_pkt_pool *)phl_info->rx_pkt_pool;

	_os_spinlock(phl_to_drvpriv(phl_info), &rx_pkt_pool->idle_lock, _bh, NULL);

	if (MAX_PHL_RING_RX_PKT_NUM == rx_pkt_pool->idle_cnt)
		res = true;
	else
		res = false;

	_os_spinunlock(phl_to_drvpriv(phl_info), &rx_pkt_pool->idle_lock, _bh, NULL);

	return res;
}

void phl_parse_rx_ppdu_stats(struct phl_info_t *phl_info, struct rtw_stats *stats, struct rtw_recv_pkt *rx_pkt)
{
	struct rtw_r_meta_data *mdata = &rx_pkt->mdata;
	struct rtw_wifi_role_t *wrole;
	//struct rtw_phl_stainfo_t *sta = NULL;
	u16 rate;
	u8 idx = 0;

	//if(mdata->rpkt_type != RX_8852A_DESC_PKT_T_WIFI){
	if(mdata->rpkt_type != 0){
		return;
	}

	if(mdata->ppdu_type != RTW_RX_8852A_DESC_PPDU_T_HE_SU){
		return;
	}

	wrole = phl_get_wrole_by_addr(phl_info, mdata->mac_addr);
	if(wrole == NULL)
		return;
	//sta = rtw_phl_get_stainfo_by_addr(phl_info, wrole, meta->ta);

	if(phl_is_he_rate(mdata->rx_rate)){
		rate = phl_cal_he_rate_level(mdata->rx_rate, &idx);
		if(idx >= 4)
			printk("%s: wrong rx rate idx", __func__);
		else{
			stats->HE_rx_cnt[idx]++;
			stats->HE_rx_rate[idx] += rate;
		}

		stats->HE_rx_bw = mdata->bw;

		stats->HE_rx_GI_LTF = mdata->rx_gi_ltf;

		if (mdata->ampdu && (stats->last_ppdu_cnt != mdata->ppdu_cnt)) {

			stats->last_ppdu_cnt = mdata->ppdu_cnt;

			if (stats->ampdu_agg > 20) {
				stats->ampdu_cnt++;

				stats->ampdu_agg_total += stats->ampdu_agg;
				stats->ampdu_size_total += stats->ampdu_size;
			}

			stats->ampdu_agg = 0;
			stats->ampdu_size = 0;
		} else if (mdata->ampdu && (stats->last_ppdu_cnt == mdata->ppdu_cnt)) {
			stats->ampdu_agg++;
			stats->ampdu_size += mdata->pktlen;
		}

		if (DBG_PRINT_RXPKT_ONCE == 1) {
			printk("====== [mdata] ======\n");
			printk("mdata.macid: %d\n", mdata->macid);
			printk("mdata.ta: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
				mdata->ta[0],mdata->ta[1],mdata->ta[2],
				mdata->ta[3],mdata->ta[4],mdata->ta[5]);
			printk("mdata.mac_addr: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
				mdata->mac_addr[0],mdata->mac_addr[1],mdata->mac_addr[2],
				mdata->mac_addr[3],mdata->mac_addr[4],mdata->mac_addr[5]);
			printk("mdata.rpkt_type: 0x%x\n", mdata->rpkt_type);
			printk("mdata.ppdu_type: 0x%x\n", mdata->ppdu_type);
			printk("mdata.ppdu_cnt: 0x%x\n", mdata->ppdu_cnt);
			printk("mdata.ampdu: 0x%x\n", mdata->ampdu);
			printk("mdata.ampdu_end_pkt: 0x%x\n", mdata->ampdu_end_pkt);
			printk("mdata.rx_rate: 0x%x\n", mdata->rx_rate);
			printk("mdata.rx_gi_ltf: 0x%x\n", mdata->rx_gi_ltf);
			printk("mdata.bw: 0x%x\n", mdata->bw);
			printk("mdata.frame_type: 0x%x\n", mdata->frame_type);
			printk("mdata.qos: 0x%x\n", mdata->qos);

			printk("====== [rx stats] ======\n");
			printk("average_HE_rx_rate: 0x%x\n", stats->average_HE_rx_rate);
			printk("average_HE_rx_rssi: 0x%x\n", stats->average_HE_rx_rssi);
			printk("HE_rx_ss: 0x%x\n", stats->HE_rx_avg_ss);
			printk("HE_rx_bw: 0x%x\n", stats->HE_rx_bw);
			printk("HE_rx_GI_LTF: 0x%x\n", stats->HE_rx_GI_LTF);
			DBG_PRINT_RXPKT_ONCE = 0;
		}
	}
}

void phl_parse_rx_HE_TB_stats(struct rtw_stats *stats, struct rtw_recv_pkt *rx_pkt)
{
	struct rtw_r_meta_data *mdata = &rx_pkt->mdata;
	u16 rate;
	u8 idx=0;

	//if(mdata->rpkt_type != RX_8852A_DESC_PKT_T_WIFI){
	if(mdata->rpkt_type != 0){
		return;
	}

	if(mdata->ppdu_type != RTW_RX_8852A_DESC_PPDU_T_HE_TB){
		return;
	}

	if(mdata->q_null)
		stats->HE_TB_stat.QoSnull_cnt++;

	if(phl_is_he_rate(mdata->rx_rate)){
		rate = phl_cal_he_rate_level(mdata->rx_rate, &idx);
		if(idx >= 4)
			printk("%s: wrong rx rate idx", __func__);
		else{
			stats->HE_TB_stat.HE_TB_rx_rate[idx] += rate;
			stats->HE_TB_stat.HE_TB_rate_cnt_per_sec[idx] ++;
		}
	}

	if(stats->HE_TB_stat.last_ppdu_cnt != mdata->ppdu_cnt){
		stats->HE_TB_stat.last_ppdu_cnt = mdata->ppdu_cnt;

		stats->HE_TB_stat.ampdu_cnt++;
		stats->HE_TB_stat.ampdu_cnt_2++;

		stats->HE_TB_stat.ampdu_agg_total_1 += stats->HE_TB_stat.ampdu_agg;
		stats->HE_TB_stat.ampdu_agg_total_2 += stats->HE_TB_stat.ampdu_agg;
		stats->HE_TB_stat.ampdu_agg = 0;

		stats->HE_TB_stat.ampdu_size_total_1 += stats->HE_TB_stat.ampdu_size;
		stats->HE_TB_stat.ampdu_size_total_2 += stats->HE_TB_stat.ampdu_size;
		stats->HE_TB_stat.ampdu_size = 0;

		stats->HE_TB_stat.zero_padding_total_1 += stats->HE_TB_stat.zero_padding;
		stats->HE_TB_stat.zero_padding_total_2 += stats->HE_TB_stat.zero_padding;
		stats->HE_TB_stat.zero_padding = 0;

		if((stats->HE_TB_stat.ampdu_cnt % 50) == 0){
			stats->HE_TB_stat.avg_ampdu_agg_1 =
				(stats->HE_TB_stat.ampdu_agg_total_1 / 49);
			//if(stats->macid == 1 || stats->macid == 2)
			//	PHL_TRACE(COMP_PHL_GROUP, _PHL_ALWAYS_, "macid:%d, ampdu_agg_total_1:%d(%d), avg_ampdu_agg_1:%d\n", stats->macid,
			//		stats->HE_TB_stat.ampdu_agg_total_1, stats->HE_TB_stat.ampdu_agg_total_2, stats->HE_TB_stat.avg_ampdu_agg_1);

			stats->HE_TB_stat.avg_ampdu_size_1 =
				(stats->HE_TB_stat.ampdu_size_total_1 / 49);

			stats->HE_TB_stat.avg_zero_padding_1 =
				//(stats->HE_TB_stat.zero_padding_total_1 / 50);
				do_div(stats->HE_TB_stat.zero_padding_total_1, 49);
			//if(stats->macid == 1 || stats->macid == 2)
			//	PHL_TRACE(COMP_PHL_GROUP, _PHL_ALWAYS_, "macid:%d, avg_zero_padding_1:%d, zero_padding_total_1:%d\n", stats->macid,
			//		stats->HE_TB_stat.avg_zero_padding_1, stats->HE_TB_stat.zero_padding_total_1);

			stats->HE_TB_stat.ampdu_agg_total_1 = 0;
			stats->HE_TB_stat.ampdu_size_total_1 = 0;
			stats->HE_TB_stat.zero_padding_total_1 = 0;
		}
	}else if(mdata->ampdu && (stats->HE_TB_stat.last_ppdu_cnt == mdata->ppdu_cnt)){
		stats->HE_TB_stat.ampdu_agg++;
		stats->HE_TB_stat.ampdu_size += mdata->pktlen;
		//stats->HE_TB_stat.zero_padding += (mdata->freerun_cnt*4);
		stats->HE_TB_stat.zero_padding += (mdata->freerun_cnt);
	}

}

void phl_dump_rx_stats(struct rtw_stats *stats)
{
	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		  "Dump Rx statistics\n"
		  "rx_byte_uni = %lld\n"
		  "rx_byte_total = %lld\n"
		  "rx_tp_kbits = %d\n"
		  "last_rx_time_ms = %d\n",
		  stats->rx_byte_uni,
		  stats->rx_byte_total,
		  stats->rx_tp_kbits,
		  stats->last_rx_time_ms);
}

void phl_reset_rx_stats(struct rtw_stats *stats)
{
	stats->rx_byte_uni = 0;
	stats->rx_byte_total = 0;
	stats->rx_tp_kbits = 0;
	stats->last_rx_time_ms = 0;
	stats->rxtp.last_calc_time_ms = 0;
	stats->rxtp.last_calc_time_ms = 0;
	stats->rx_traffic.lvl = RTW_TFC_IDLE;
	stats->rx_traffic.sts = 0;
	stats->rx_tf_cnt = 0;
}

void
phl_rx_traffic_upd(struct rtw_stats *sts)
{
	u32 tp_k = 0, tp_m = 0;
	enum rtw_tfc_lvl rx_tfc_lvl = RTW_TFC_IDLE;
	tp_k = sts->rx_tp_kbits;
	tp_m = sts->rx_tp_kbits >> 10;

	if (tp_m >= RX_HIGH_TP_THRES_MBPS)
		rx_tfc_lvl = RTW_TFC_HIGH;
	else if (tp_m >= RX_MID_TP_THRES_MBPS)
		rx_tfc_lvl = RTW_TFC_MID;
	else if (tp_m >= RX_LOW_TP_THRES_MBPS)
		rx_tfc_lvl = RTW_TFC_LOW;
	else if (tp_k >= RX_ULTRA_LOW_TP_THRES_KBPS)
		rx_tfc_lvl = RTW_TFC_ULTRA_LOW;
	else
		rx_tfc_lvl = RTW_TFC_IDLE;

	if (sts->rx_traffic.lvl > rx_tfc_lvl) {
		sts->rx_traffic.sts = (TRAFFIC_CHANGED | TRAFFIC_DECREASE);
		sts->rx_traffic.lvl = rx_tfc_lvl;
	} else if (sts->rx_traffic.lvl < rx_tfc_lvl) {
		sts->rx_traffic.sts = (TRAFFIC_CHANGED | TRAFFIC_INCREASE);
		sts->rx_traffic.lvl = rx_tfc_lvl;
	} else if (sts->rx_traffic.sts &
		(TRAFFIC_CHANGED | TRAFFIC_INCREASE | TRAFFIC_DECREASE)) {
		sts->rx_traffic.sts &= ~(TRAFFIC_CHANGED | TRAFFIC_INCREASE |
					 TRAFFIC_DECREASE);
	}
}

void phl_update_rx_stats(struct rtw_stats *stats, struct rtw_recv_pkt *rx_pkt)
{
	u32 diff_t = 0, cur_time = _os_get_cur_time_ms();
	u64 diff_bits = 0;

	stats->last_rx_time_ms = cur_time;
	stats->rx_byte_total += rx_pkt->mdata.pktlen;
	if (rx_pkt->mdata.bc == 0 && rx_pkt->mdata.mc == 0)
		stats->rx_byte_uni += rx_pkt->mdata.pktlen;

	if (0 == stats->rxtp.last_calc_time_ms ||
		0 == stats->rxtp.last_calc_bits) {
		stats->rxtp.last_calc_time_ms = stats->last_rx_time_ms;
		stats->rxtp.last_calc_bits = stats->rx_byte_uni * 8;
	} else {
		if (cur_time >= stats->rxtp.last_calc_time_ms) {
			diff_t = cur_time - stats->rxtp.last_calc_time_ms;
		} else {
			diff_t = RTW_U32_MAX - stats->rxtp.last_calc_time_ms +
				cur_time + 1;
		}
		if (diff_t > RXTP_CALC_DIFF_MS && stats->rx_byte_uni != 0) {
			diff_bits = (stats->rx_byte_uni * 8) -
				stats->rxtp.last_calc_bits;
			stats->rx_tp_kbits = (u32)_os_division64(diff_bits,
								 diff_t);
			stats->rxtp.last_calc_bits = stats->rx_byte_uni * 8;
			stats->rxtp.last_calc_time_ms = cur_time;
		}
	}
}

void phl_rx_statistics(struct phl_info_t *phl_info, struct rtw_recv_pkt *rx_pkt)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_stats *phl_stats = &phl_com->phl_stats;
	struct rtw_stats *sta_stats = NULL;
	struct rtw_phl_stainfo_t *sta = NULL;
	u16 macid = rx_pkt->mdata.macid;
	struct rtw_wifi_role_t *wrole;

	if (!phl_macid_is_valid(phl_info, macid))
		goto dev_stat;

	sta = rtw_phl_get_stainfo_by_macid(phl_info, macid);

	if (NULL == sta)
		goto dev_stat;
	sta_stats = &sta->stats;
	sta_stats->macid = sta->macid;

	phl_update_rx_stats(sta_stats, rx_pkt);

	phl_parse_rx_ppdu_stats(phl_info, sta_stats, rx_pkt);

	phl_parse_rx_HE_TB_stats(sta_stats, rx_pkt);

dev_stat:
	phl_update_rx_stats(phl_stats, rx_pkt);
}

void phl_release_phl_rx(struct phl_info_t *phl_info,
				struct rtw_phl_rx_pkt *phl_rx)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_rx_pkt_pool *rx_pkt_pool = NULL;

	_os_mem_set(phl_to_drvpriv(phl_info), &phl_rx->r, 0, sizeof(phl_rx->r));
	phl_rx->type = RTW_RX_TYPE_MAX;
	phl_rx->rxbuf_ptr = NULL;
	INIT_LIST_HEAD(&phl_rx->list);

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	INIT_LIST_HEAD(&phl_rx->r.rx_buf_lst_head);

	if (phl_rx == &phl_info->tmp_phl_rx) {
		return;
	}
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	rx_pkt_pool = (struct phl_rx_pkt_pool *)phl_info->rx_pkt_pool;

	_os_spinlock(drv_priv, &rx_pkt_pool->idle_lock, _bh, NULL);
	list_add_tail(&phl_rx->list, &rx_pkt_pool->idle);
	rx_pkt_pool->idle_cnt++;
	_os_spinunlock(drv_priv, &rx_pkt_pool->idle_lock, _bh, NULL);
}

static void phl_free_recv_pkt_pool(struct phl_info_t *phl_info)
{
	struct phl_rx_pkt_pool *rx_pkt_pool = NULL;
	u32 buf_len = 0;
	FUNCIN();

	rx_pkt_pool = (struct phl_rx_pkt_pool *)phl_info->rx_pkt_pool;
	if (NULL != rx_pkt_pool) {
		_os_spinlock_free(phl_to_drvpriv(phl_info),
					&rx_pkt_pool->idle_lock);
		_os_spinlock_free(phl_to_drvpriv(phl_info),
					&rx_pkt_pool->busy_lock);

		buf_len = sizeof(*rx_pkt_pool);
		_os_mem_free(phl_to_drvpriv(phl_info), rx_pkt_pool, buf_len);
	}

	FUNCOUT();
}

void phl_rx_deinit(struct phl_info_t *phl_info)
{
	/* TODO: rx reorder deinit */

	/* TODO: peer info deinit */

	phl_free_recv_pkt_pool(phl_info);
}


static enum rtw_phl_status phl_alloc_recv_pkt_pool(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_rx_pkt_pool *rx_pkt_pool = NULL;
	struct rtw_phl_rx_pkt *phl_rx = NULL;
	u32 buf_len = 0, i = 0;
	FUNCIN_WSTS(pstatus);

	buf_len = sizeof(*rx_pkt_pool);
	rx_pkt_pool = _os_mem_alloc(phl_to_drvpriv(phl_info), buf_len);

	if (NULL != rx_pkt_pool) {
		_os_mem_set(phl_to_drvpriv(phl_info), rx_pkt_pool, 0, buf_len);
		INIT_LIST_HEAD(&rx_pkt_pool->idle);
		INIT_LIST_HEAD(&rx_pkt_pool->busy);
		_os_spinlock_init(phl_to_drvpriv(phl_info),
					&rx_pkt_pool->idle_lock);
		_os_spinlock_init(phl_to_drvpriv(phl_info),
					&rx_pkt_pool->busy_lock);
		rx_pkt_pool->idle_cnt = 0;

		for (i = 0; i < MAX_PHL_RING_RX_PKT_NUM; i++) {
			phl_rx = &rx_pkt_pool->phl_rx[i];
			INIT_LIST_HEAD(&phl_rx->list);
			list_add_tail(&phl_rx->list, &rx_pkt_pool->idle);
			rx_pkt_pool->idle_cnt++;
		}

		phl_info->rx_pkt_pool = rx_pkt_pool;

		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	if (RTW_PHL_STATUS_SUCCESS != pstatus)
		phl_free_recv_pkt_pool(phl_info);
	FUNCOUT_WSTS(pstatus);

	return pstatus;
}

enum rtw_phl_status phl_rx_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status status;

	/* Allocate rx packet pool */
	status = phl_alloc_recv_pkt_pool(phl_info);
	if (status != RTW_PHL_STATUS_SUCCESS)
		return status;

	/* TODO: Peer info init */


	/* TODO: Rx reorder init */

	return RTW_PHL_STATUS_SUCCESS;
}

#if(defined(CONFIG_RTW_HW_RX_AMSDU_CUT) && !defined(CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV))
static inline enum rtw_phl_status _recycle_rx_buf_list(struct phl_info_t *phl_info,
                                                       struct rtw_recv_pkt *rx_pkt,
                                                       enum rtw_rx_type type)
{
	struct list_head *rx_buf_lst = &rx_pkt->rx_buf_lst_head;
	u8 ch = rx_pkt->mdata.dma_ch;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct rtw_rx_buf *i, *j;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	u32 num = 0;

	if (rx_buf_lst->next == NULL || rx_buf_lst->prev == NULL) {
		PHL_ERR("rx buf list head invalid %pX %pX\n", rx_buf_lst->next,
		        rx_buf_lst->prev);
		debug_dump_buf((u8 *)rx_pkt, sizeof(*rx_pkt), "rtw_recv_pkt:");
		dump_stack();
		return hci_trx_ops->recycle_rx_buf(phl_info, rx_pkt->rx_buf,
		                                   ch, type);
	}

	if (list_empty(rx_buf_lst))
		return RTW_PHL_STATUS_SUCCESS;

	phl_list_for_loop_safe(i, j, struct rtw_rx_buf, rx_buf_lst, list) {
		if (i == NULL || i->list.next == NULL || i->list.prev == NULL) {
			PHL_ERR("Invalid rx buf list!\n");
			dump_stack();
			return RTW_PHL_STATUS_FAILURE;
		}
		num++;
		list_del_init(&i->list);
		pstatus = pstatus ||
			  hci_trx_ops->recycle_rx_buf(phl_info, i, ch, type);
	}

	if ((num > 1) && (rx_pkt->mdata.amsdu_cut == 0))
		PHL_ERR("Too many RX buffers (%u) for non A-MSDU cut packets.\n",
		        num);

	return pstatus;
}

void phl_recycle_rx_buf(struct phl_info_t *phl_info,
			struct rtw_phl_rx_pkt *phl_rx)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if (NULL == phl_rx) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_,
		          "[WARNING]phl_rx is NULL!\n");
		return;
	}

	if (list_empty(&phl_rx->r.rx_buf_lst_head)) {
		struct rtw_rx_buf *rx_buf;
		struct phl_hci_trx_ops *hci_trx_ops;

		rx_buf = (struct rtw_rx_buf *)phl_rx->rxbuf_ptr;
		hci_trx_ops = phl_info->hci_trx_ops;

		if (rx_buf && hci_trx_ops->recycle_rx_buf) {
			pstatus = hci_trx_ops->recycle_rx_buf(phl_info, rx_buf,
								phl_rx->r.mdata.dma_ch,
								phl_rx->type);
		}
	} else {
		pstatus = _recycle_rx_buf_list(phl_info,
					       &phl_rx->r,
					       phl_rx->type);
	}
	if (RTW_PHL_STATUS_SUCCESS != pstatus)
		PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_,
		          "[WARNING]recycle hci rx buf error!\n");

	phl_release_phl_rx(phl_info, phl_rx);
}
#else /* CONFIG_RTW_HW_RX_AMSDU_CUT */
void phl_recycle_rx_buf(struct phl_info_t *phl_info,
				struct rtw_phl_rx_pkt *phl_rx)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	struct rtw_rx_buf *rx_buf = NULL;

	do {
		if (NULL == phl_rx) {
			PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_, "[WARNING]phl_rx is NULL!\n");
			break;
		}

		rx_buf = (struct rtw_rx_buf *)phl_rx->rxbuf_ptr;

		PHL_TRACE(COMP_PHL_RECV, _PHL_DEBUG_, "[4] %s:: [%p]\n",
								__FUNCTION__, rx_buf);
		if (phl_rx->rxbuf_ptr) {
			pstatus = hci_trx_ops->recycle_rx_buf(phl_info, rx_buf,
								phl_rx->r.mdata.dma_ch,
								phl_rx->type);
		}
		if (RTW_PHL_STATUS_SUCCESS != pstatus && phl_rx->rxbuf_ptr)
			PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_, "[WARNING]recycle hci rx buf error!\n");

		phl_release_phl_rx(phl_info, phl_rx);

	} while (false);

}
#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

void _phl_indic_new_rxpkt(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct rtw_evt_info_t *evt_info = &phl_info->phl_com->evt_info;
	void *drv_priv = phl_to_drvpriv(phl_info);
	FUNCIN_WSTS(pstatus);

	do {
		_os_spinlock(drv_priv, &evt_info->evt_lock, _bh, NULL);
		if (!(evt_info->evt_bitmap & RTW_PHL_EVT_RX)) {
			evt_info->evt_bitmap |= RTW_PHL_EVT_RX;
			_os_spinunlock(drv_priv, &evt_info->evt_lock, _bh, NULL);
			pstatus = phl_schedule_handler(phl_info->phl_com,
			                               &phl_info->phl_event_handler);
		} else
			_os_spinunlock(drv_priv, &evt_info->evt_lock, _bh, NULL);
	} while (false);

	if (RTW_PHL_STATUS_SUCCESS != pstatus)
		PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_, "[WARNING] Trigger rx indic event fail!\n");

	FUNCOUT_WSTS(pstatus);

#ifdef PHL_RX_BATCH_IND
	phl_info->rx_new_pending = 0;
#endif
}

void _phl_record_rx_stats(struct rtw_recv_pkt *recvpkt)
{
	if(NULL == recvpkt)
		return;
	if (recvpkt->tx_sta)
		recvpkt->tx_sta->stats.rx_rate = recvpkt->mdata.rx_rate;
}

enum rtw_phl_status _phl_add_rx_pkt(struct phl_info_t *phl_info,
				    struct rtw_phl_rx_pkt *phl_rx)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_rx_ring *ring = &phl_info->phl_rx_ring;
	struct rtw_recv_pkt *recvpkt = &phl_rx->r;
	//u16 macid = recvpkt->mdata.macid;
	u16 ring_res = 0, wptr = 0, rptr = 0;
	void *drv = phl_to_drvpriv(phl_info);

	FUNCIN_WSTS(pstatus);
	_os_spinlock(drv, &phl_info->rx_ring_lock, _bh, NULL);

	if (!ring)
		goto out;

	wptr = ring->phl_idx;
	rptr = ring->core_idx;

	ring_res = phl_calc_avail_wptr(rptr, wptr, MAX_PHL_RING_ENTRY_NUM);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_,
		"[3] _phl_add_rx_pkt::[Query] phl_idx =%d , core_idx =%d , ring_res =%d\n",
		ring->phl_idx,
		ring->core_idx,
		ring_res);
	if (ring_res <= 0) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "no ring resource to add new rx pkt!\n");
		pstatus = RTW_PHL_STATUS_RESOURCE;
		goto out;
	}

	wptr = wptr + 1;
	if (wptr >= MAX_PHL_RING_ENTRY_NUM)
		wptr = 0;

	ring->entry[wptr] = recvpkt;
	ring->phl_idx = wptr;

#ifdef DEBUG_PHL_RX
	phl_info->phl_com->rx_stats.rx_pkt_core++;
	if (recvpkt->mdata.pktlen == phl_info->phl_com->cnt_rx_pktsz)
		phl_info->phl_com->rx_stats.rx_pktsz_core++;
#endif
#ifdef PHL_RX_BATCH_IND
	phl_info->rx_new_pending = 1;
	pstatus = RTW_PHL_STATUS_SUCCESS;
#endif

out:
	_os_spinunlock(drv, &phl_info->rx_ring_lock, _bh, NULL);

	if(pstatus == RTW_PHL_STATUS_SUCCESS)
		_phl_record_rx_stats(recvpkt);

	FUNCOUT_WSTS(pstatus);

	return pstatus;
}

void
phl_sta_ps_enter(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta,
                 struct rtw_wifi_role_t *role)
{
	void *d = phl_to_drvpriv(phl_info);
	/* enum rtw_hal_status hal_status; */
	struct rtw_phl_evt_ops *ops = &phl_info->phl_com->evt_ops;

	_os_atomic_set(d, &sta->ps_sta, 1);

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
	          "STA %02X:%02X:%02X:%02X:%02X:%02X enters PS mode, AID=%u, macid=%u, sta=0x%p\n",
	          sta->mac_addr[0], sta->mac_addr[1], sta->mac_addr[2],
	          sta->mac_addr[3], sta->mac_addr[4], sta->mac_addr[5],
	          sta->aid, sta->macid, sta);

	/* TODO: comment out because beacon may stop if we do this frequently */
	/* hal_status = rtw_hal_set_macid_pause(phl_info->hal, */
	/*                                         sta->macid, true); */
	/* if (RTW_HAL_STATUS_SUCCESS != hal_status) { */
	/*         PHL_WARN("%s(): failed to pause macid tx, macid=%u\n", */
	/*                  __FUNCTION__, sta->macid); */
	/* } */

	if (ops->ap_ps_sta_ps_change)
		ops->ap_ps_sta_ps_change(d, role->id, sta->mac_addr, true);
}

void
phl_sta_ps_exit(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta,
                struct rtw_wifi_role_t *role)
{
	void *d = phl_to_drvpriv(phl_info);
	/* enum rtw_hal_status hal_status; */
	struct rtw_phl_evt_ops *ops = &phl_info->phl_com->evt_ops;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
	          "STA %02X:%02X:%02X:%02X:%02X:%02X leaves PS mode, AID=%u, macid=%u, sta=0x%p\n",
	          sta->mac_addr[0], sta->mac_addr[1], sta->mac_addr[2],
	          sta->mac_addr[3], sta->mac_addr[4], sta->mac_addr[5],
	          sta->aid, sta->macid, sta);

	_os_atomic_set(d, &sta->ps_sta, 0);

	/* TODO: comment out because beacon may stop if we do this frequently */
	/* hal_status = rtw_hal_set_macid_pause(phl_info->hal, */
	/*                                         sta->macid, false); */
	/* if (RTW_HAL_STATUS_SUCCESS != hal_status) { */
	/*         PHL_WARN("%s(): failed to resume macid tx, macid=%u\n", */
	/*                  __FUNCTION__, sta->macid); */
	/* } */

	if (ops->ap_ps_sta_ps_change)
		ops->ap_ps_sta_ps_change(d, role->id, sta->mac_addr, false);
}

void
phl_rx_handle_sta_process(struct phl_info_t *phl_info,
                          struct rtw_phl_rx_pkt *rx)
{
	struct rtw_r_meta_data *m = &rx->r.mdata;
	struct rtw_wifi_role_t *role = rx->r.rx_role;
	struct rtw_phl_stainfo_t *sta = rx->r.tx_sta;
	void *d = phl_to_drvpriv(phl_info);

	if (!phl_info->phl_com->dev_sw_cap.ap_ps)
		return;

	if ((sta == NULL || role == NULL) && m->addr_cam_vld) {
		sta = rtw_phl_get_stainfo_by_macid(phl_info, m->macid);
		if (sta && sta->wrole)
			role = sta->wrole;
	}

	if (!sta) {
		role = phl_get_wrole_by_addr(phl_info, m->mac_addr);
		if (role)
			sta = rtw_phl_get_stainfo_by_addr(phl_info,
			                                  role, m->ta);
	}

	if (!role || !sta)
		return;

	rx->r.tx_sta = sta;
	rx->r.rx_role = role;

	PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
	          "ap-ps: more_frag=%u, frame_type=%u, role_type=%d, pwr_bit=%u, seq=%u\n",
	          m->more_frag, m->frame_type, role->type, m->pwr_bit, m->seq);

	/*
	 * Change STA PS state based on the PM bit in frame control
	 */
	if (!m->more_frag &&
	    (m->frame_type == RTW_FRAME_TYPE_DATA ||
	     m->frame_type == RTW_FRAME_TYPE_CTRL) &&
	    (IS_AP_ROLE_TYPE(role->type) ||
	     role->type == PHL_RTYPE_P2P_GO)) {
		if (_os_atomic_read(d, &sta->ps_sta)) {
			if (!m->pwr_bit)
				phl_sta_ps_exit(phl_info, sta, role);
		} else {
			if (m->pwr_bit)
				phl_sta_ps_enter(phl_info, sta, role);
		}
	}
}

void
phl_handle_rx_frame_list(struct phl_info_t *phl_info,
                         _os_list *frames)
{
	struct rtw_phl_rx_pkt *pos, *n;
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;

	phl_list_for_loop_safe(pos, n, struct rtw_phl_rx_pkt, frames, list) {
		list_del(&pos->list);
		phl_rx_handle_sta_process(phl_info, pos);
		status = _phl_add_rx_pkt(phl_info, pos);
		if (RTW_PHL_STATUS_RESOURCE == status) {
			phl_info->phl_com->rx_stats.rx_drop_no_res++;
			hci_trx_ops->recycle_rx_pkt(phl_info, pos);
		}
	}
#ifndef PHL_RX_BATCH_IND
	_phl_indic_new_rxpkt(phl_info);
#endif

}


#define SEQ_MODULO 0x1000
#define SEQ_MASK	0xfff

static inline int seq_less(u16 sq1, u16 sq2)
{
	return ((sq1 - sq2) & SEQ_MASK) > (SEQ_MODULO >> 1);
}

static inline u16 seq_inc(u16 sq)
{
	return (sq + 1) & SEQ_MASK;
}

static inline u16 seq_sub(u16 sq1, u16 sq2)
{
	return (sq1 - sq2) & SEQ_MASK;
}

static inline u16 reorder_index(struct phl_tid_ampdu_rx *r, u16 seq)
{
	return seq % r->buf_size;
}

static void phl_release_reorder_frame(struct phl_info_t *phl_info,
									struct phl_tid_ampdu_rx *r,
									int index, _os_list *frames)
{
	struct rtw_phl_rx_pkt *pkt = r->reorder_buf[index];

	if (!pkt)
		goto out;

	/* release the frame from the reorder ring buffer */
	r->stored_mpdu_num--;
	r->reorder_buf[index] = NULL;
	list_add_tail(&pkt->list, frames);

out:
	r->head_seq_num = seq_inc(r->head_seq_num);
}

#define HT_RX_REORDER_BUF_TIMEOUT_MS 100

/*
 * If the MPDU at head_seq_num is ready,
 *     1. release all subsequent MPDUs with consecutive SN and
 *     2. if there's MPDU that is ready but left in the reordering
 *        buffer, find it and set reorder timer according to its reorder
 *        time
 *
 * If the MPDU at head_seq_num is not ready and there is no MPDU ready
 * in the buffer at all, return.
 *
 * If the MPDU at head_seq_num is not ready but there is some MPDU in
 * the buffer that is ready, check whether any frames in the reorder
 * buffer have timed out in the following way.
 *
 * Basically, MPDUs that are not ready are purged and MPDUs that are
 * ready are released.
 *
 * The process goes through all the buffer but the one at head_seq_num
 * unless
 *     - there's a MPDU that is ready AND
 *     - there are one or more buffers that are not ready.
 * In this case, the process is stopped, the head_seq_num becomes the
 * first buffer that is not ready and the reorder_timer is reset based
 * on the reorder_time of that ready MPDU.
 */
static void phl_reorder_release(struct phl_info_t *phl_info,
								struct phl_tid_ampdu_rx *r, _os_list *frames)
{
	/* ref ieee80211_sta_reorder_release() and wil_reorder_release() */

	int index, i, j;

	/* release the buffer until next missing frame */
	index = reorder_index(r, r->head_seq_num);
	if (!r->reorder_buf[index] && r->stored_mpdu_num) {
		/*
		 * No buffers ready to be released, but check whether any
		 * frames in the reorder buffer have timed out.
		 */
		u32 cur_time = _os_get_cur_time_ms();
		int skipped = 1;
		for (j = (index + 1) % r->buf_size; j != index;
			j = (j + 1) % r->buf_size) {
			if (!r->reorder_buf[j]) {
				skipped++;
				continue;
			}
			if (skipped && (s32)(r->reorder_time[j] +
				HT_RX_REORDER_BUF_TIMEOUT_MS - cur_time) > 0)
				goto set_release_timer;

			PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "release an RX reorder frame due to timeout on earlier frames\n");

			phl_release_reorder_frame(phl_info, r, j, frames);

			/*
			 * Increment the head seq# also for the skipped slots.
			 */
			r->head_seq_num =
				(r->head_seq_num + skipped) & SEQ_MASK;
			skipped = 0;
		}
	} else while (r->reorder_buf[index]) {
		phl_release_reorder_frame(phl_info, r, index, frames);
		index = reorder_index(r, r->head_seq_num);
	}

	if (r->stored_mpdu_num) {

set_release_timer:

		if (!r->removed)
			_os_set_timer(r->drv_priv, &r->sta->reorder_timer,
			              HT_RX_REORDER_BUF_TIMEOUT_MS);
	}
}

void phl_sta_rx_reorder_timer_expired(void *t)
{
	/* ref sta_rx_agg_reorder_timer_expired() */

	struct rtw_phl_stainfo_t *sta = (struct rtw_phl_stainfo_t *)t;
	struct rtw_phl_com_t *phl_com = sta->wrole->phl_com;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_com->phl_priv;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list frames;
	u8 i = 0;

	PHL_INFO("Rx reorder timer expired, sta=0x%p\n", sta);

	INIT_LIST_HEAD(&frames);

	_os_spinlock(drv_priv, &sta->tid_rx_lock, _bh, NULL);
	for (i = 0; i < ARRAY_SIZE(sta->tid_rx); i++) {
		if (sta->tid_rx[i])
			phl_reorder_release(phl_info, sta->tid_rx[i], &frames);
	}
	_os_spinunlock(drv_priv, &sta->tid_rx_lock, _bh, NULL);

	phl_handle_rx_frame_list(phl_info, &frames);
#ifdef PHL_RX_BATCH_IND
	_phl_indic_new_rxpkt(phl_info);
#endif

	_os_event_set(drv_priv, &sta->comp_sync);
}

static void phl_release_reorder_frames(struct phl_info_t *phl_info,
										struct phl_tid_ampdu_rx *r,
										u16 head_seq_num, _os_list *frames)
{
	/* ref ieee80211_release_reorder_frames() and
		wil_release_reorder_frames() */

	int index;

	/* note: this function is never called with
	 * hseq preceding r->head_seq_num, i.e it is always true
	 * !seq_less(hseq, r->head_seq_num)
	 * and thus on loop exit it should be
	 * r->head_seq_num == hseq
	 */
	while (seq_less(r->head_seq_num, head_seq_num) &&
		r->stored_mpdu_num) { /* Note: do we need to check this? */
		index = reorder_index(r, r->head_seq_num);
		phl_release_reorder_frame(phl_info, r, index, frames);
	}
	r->head_seq_num = head_seq_num;
}

#ifdef PHL_RXSC_AMPDU
static void _phl_rxsc_cache_entry(struct phl_info_t *phl_info,
				struct phl_tid_ampdu_rx *r,
				struct rtw_r_meta_data *meta)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_rxsc_cache_entry *rxsc_entry = &phl_com->rxsc_entry;

	_os_spinlock(phl_com->drv_priv, &rxsc_entry->rxsc_lock, _bh, NULL);
	rxsc_entry->cached_rx_macid = meta->macid;
	rxsc_entry->cached_rx_tid = meta->tid;
	rxsc_entry->cached_rx_ppdu_cnt = meta->ppdu_cnt;
	rxsc_entry->cached_rx_seq = meta->seq;
	rxsc_entry->cached_r = r;
	_os_spinunlock(phl_com->drv_priv, &rxsc_entry->rxsc_lock, _bh, NULL);

#ifdef DEBUG_PHL_RX
	phl_info->phl_com->rx_stats.rxsc_ampdu[2]++;
#endif
}

static bool _phl_rxsc_cache_check(struct phl_info_t *phl_info,
				struct rtw_phl_rx_pkt *phl_rx,
				struct rtw_r_meta_data *meta)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_rxsc_cache_entry *rxsc_entry = &phl_com->rxsc_entry;
	struct phl_tid_ampdu_rx *r;
	u8 res = false;
	u16 mpdu_seq_num = meta->seq;
	int index;
	u8 is_match_seq = true;

	if (RXSC_INVALID_MACID != rxsc_entry->cached_rx_macid) {
		_os_spinlock(phl_com->drv_priv,
				&rxsc_entry->rxsc_lock, _bh, NULL);
		/* for those pkts not enter rxsc in parse wd, but may enter rxsc in rx reorder */
		if (!phl_rx->r.mdata.rxsc_parse_wd_matched) {
			if (((rxsc_entry->cached_rx_seq + 1) & SEQ_MASK) != phl_rx->r.mdata.seq) {
				/* go back to normal reorder */
				is_match_seq = false;
			} else {
				/* if seq matches rxsc entry, update rxsc entry seq */
				rxsc_entry->cached_rx_seq = phl_rx->r.mdata.seq;
				is_match_seq = true;
			}
		}
		r = rxsc_entry->cached_r;
		if (r && is_match_seq == true) {
			r->head_seq_num = seq_inc(r->head_seq_num);
			phl_rx->r.rx_role = r->sta->wrole;

			index = reorder_index(r, mpdu_seq_num);
			r->reorder_sn[index] = mpdu_seq_num;

			res = true;
		} else {
			if(r == NULL)
				PHL_ERR("[%s]RXSC: cached_r is NULL! (is_match_seq:%d)\n", __func__, is_match_seq);
			else
				PHL_ERR("[%s]RXSC: cached_r is not NULL! (is_match_seq:%d)\n", __func__, is_match_seq);

			/* reset cached macid & sta due to illegal cached_r */
			rxsc_entry->cached_rx_macid = RXSC_INVALID_MACID;
			rxsc_entry->cached_r = NULL;
			res = false;
		}
		_os_spinunlock(phl_com->drv_priv,
				&rxsc_entry->rxsc_lock, _bh, NULL);
#ifdef DEBUG_PHL_RX
		phl_com->rx_stats.rxsc_ampdu[1]++;
#endif
	} else {
#ifdef DEBUG_PHL_RX
		if (meta->ampdu)
			phl_com->rx_stats.rxsc_ampdu[0]++;
#endif
	}

	return res;
}
#endif

static bool phl_manage_sta_reorder_buf(struct phl_info_t *phl_info,
										struct rtw_phl_rx_pkt *pkt,
										struct phl_tid_ampdu_rx *r,
										_os_list *frames)
{
	/* ref ieee80211_sta_manage_reorder_buf() and wil_rx_reorder() */

	struct rtw_r_meta_data *meta = &pkt->r.mdata;
	u16 mpdu_seq_num = meta->seq;
	u16 head_seq_num, buf_size;
	int index;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	#ifdef DEBUG_PHL_RX
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	#endif /* DEBUG_PHL_RX */

	buf_size = r->buf_size;
	head_seq_num = r->head_seq_num;

	/*
	 * If the current MPDU's SN is smaller than the SSN, it shouldn't
	 * be reordered.
	 */
	if (!r->started) {
		if (seq_less(mpdu_seq_num, head_seq_num))
			return false;
		r->started = true;
	}

#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV
	//seq repeat one due to amsdu_cut function
	if(meta->amsdu_cut && seq_less(mpdu_seq_num, head_seq_num))
		return false;
#endif

	/* frame with out of data sequence number */
	if (seq_less(mpdu_seq_num, head_seq_num)) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "Rx drop: old seq 0x%03x head 0x%03x\n",
				meta->seq, r->head_seq_num);
		index = reorder_index(r, mpdu_seq_num);
		if (r->reorder_sn[index] != mpdu_seq_num) {
			r->reorder_sn[index] = mpdu_seq_num;
#ifdef DEBUG_PHL_RX
			phl_com->rx_stats.reorder_indicate++;
#endif
			return false;
		}
		hci_trx_ops->recycle_rx_pkt(phl_info, pkt);
#ifdef DEBUG_PHL_RX
		phl_com->rx_stats.rx_drop_reorder++;
		phl_com->rx_stats.reorder_seq_less++;
#endif
		return true;
	}

	/*
	 * If frame the sequence number exceeds our buffering window
	 * size release some previous frames to make room for this one.
	 */
	if (!seq_less(mpdu_seq_num, head_seq_num + buf_size)) {
		head_seq_num = seq_inc(seq_sub(mpdu_seq_num, buf_size));
		/* release stored frames up to new head to stack */
		phl_release_reorder_frames(phl_info, r, head_seq_num, frames);
	}

	/* Now the new frame is always in the range of the reordering buffer */

	index = reorder_index(r, mpdu_seq_num);

	/* check if we already stored this frame */
	if (r->reorder_buf[index]) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "Rx drop: old seq 0x%03x head 0x%03x\n",
				meta->seq, r->head_seq_num);
		hci_trx_ops->recycle_rx_pkt(phl_info, pkt);
#ifdef DEBUG_PHL_RX
		phl_com->rx_stats.rx_drop_reorder++;
		phl_com->rx_stats.reorder_dup++;
#endif
		return true;
	}

	/*
	 * If the current MPDU is in the right order and nothing else
	 * is stored we can process it directly, no need to buffer it.
	 * If it is first but there's something stored, we may be able
	 * to release frames after this one.
	 */
	if (mpdu_seq_num == r->head_seq_num &&
		r->stored_mpdu_num == 0) {
		r->head_seq_num = seq_inc(r->head_seq_num);
		r->reorder_sn[index] = mpdu_seq_num;
#ifdef PHL_RXSC_AMPDU
		_phl_rxsc_cache_entry(phl_info, r, meta);
#endif
		return false;
	}

	/* put the frame in the reordering buffer */
	r->reorder_buf[index] = pkt;
	r->reorder_time[index] = _os_get_cur_time_ms();
	r->reorder_sn[index] = mpdu_seq_num;
	r->stored_mpdu_num++;
	phl_reorder_release(phl_info, r, frames);
#ifdef DEBUG_PHL_RX
	phl_com->rx_stats.rx_put_reorder++;
#endif
	return true;

}

enum rtw_phl_status phl_rx_reorder(struct phl_info_t *phl_info,
                                   struct rtw_phl_rx_pkt *phl_rx,
                                   _os_list *frames)
{
	/* ref wil_rx_reorder() and ieee80211_rx_reorder_ampdu() */

	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_r_meta_data *meta = &phl_rx->r.mdata;
	u16 tid = meta->tid;
	struct rtw_phl_stainfo_t *sta = NULL;
	struct rtw_phl_stainfo_t *sta_cached = NULL;
	struct phl_tid_ampdu_rx *r;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	struct rtw_stats *stats;

	/* Use MAC ID from address CAM if this packet is address CAM matched */
	if (meta->addr_cam_vld) {
		sta = rtw_phl_get_stainfo_by_macid(phl_info, meta->macid);

		if (sta) {
			stats = &sta->stats;
			stats->macid = meta->macid;
			stats->rx_cnt++;
			if(meta->ppdu_type == RTW_RX_8852A_DESC_PPDU_T_HE_TB)
				stats->HE_TB_stat.HE_TB_rx_cnt++;
		}
	}

	/*
	 * Remove FCS if it is appended
	 */
	while (phl_info->phl_com->append_fcs) {
		#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
		if (meta->amsdu_cut)
			break;

		if (phl_rx->r.rx_buf->frame_len <= 4) {
			PHL_ERR("%s, frame length %u too short\n",
			        __func__, phl_rx->r.rx_buf->frame_len);
			goto drop_frame;
		}
		phl_rx->r.rx_buf->frame_len -= 4;
		#else /* CONFIG_RTW_HW_RX_AMSDU_CUT */
		if (phl_rx->r.pkt_list[0].length <= 4) {
			if (!phl_is_mp_mode(phl_info->phl_com))
				PHL_ERR("%s, pkt_list[0].length(%d) too short\n",
			        	__func__, phl_rx->r.pkt_list[0].length);
			goto drop_frame;
		}
		phl_rx->r.pkt_list[0].length -= 4;
		#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
		break;
	}

#ifdef PHL_RXSC_AMPDU
	if (_phl_rxsc_cache_check(phl_info, phl_rx, meta))
		goto dont_reorder;
#endif
	if(true == phl_is_mp_mode(phl_info->phl_com) && meta->crc32)
		goto drop_frame;

	if (meta->crc32) {
		if(sta){
			stats->rx_crc32_cnt++;
			if(meta->ppdu_type == RTW_RX_8852A_DESC_PPDU_T_HE_TB)
				stats->HE_TB_stat.crc32_cnt++;
		}
		goto drop_frame;
	}

	if (phl_is_mp_mode(phl_info->phl_com))
		goto dont_reorder;

	if (meta->bc || meta->mc)
		goto dont_reorder;

	if (!meta->qos)
		goto dont_reorder;

	if (meta->q_null)
		goto dont_reorder;

	/* TODO: check ba policy is either ba or normal */

	/* if the mpdu is fragmented, don't reorder */
	if (meta->more_frag || meta->frag_num) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_ERR_,
		          "Receive QoS Data with more_frag=%u, frag_num=%u\n",
		          meta->more_frag, meta->frag_num);
		goto dont_reorder;
	}

	/* Use MAC ID from address CAM if this packet is address CAM matched */
	if (meta->addr_cam_vld)
		sta = rtw_phl_get_stainfo_by_macid(phl_info, meta->macid);

	if (!sta) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_,
		          "%s(): stainfo not found, cam=%u, macid=%u\n",
		          __FUNCTION__, meta->addr_cam, meta->macid);
		#ifdef CONFIG_RTW_DEBUG_RX_CACHE
		/* Address CAM valid, but no sta info found! */
		if (meta->addr_cam_vld) {
			u8 *wlan_hdr;

			#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
			wlan_hdr = phl_rx->r.rx_buf->hdr;
			#else /* CONFIG_RTW_HW_RX_AMSDU_CUT */
			wlan_hdr = phl_rx->r.pkt_list[0].vir_addr;
			#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

			phl_info->phl_com->rx_stats.macid_no_sta++;
			PHL_DBG("M%u no STA. %pM\n", meta->macid, wlan_hdr + 10);
		}
		#endif /* CONFIG_RTW_DEBUG_RX_CACHE */
		goto dont_reorder;
	}
	#ifdef CONFIG_RTW_DEBUG_RX_CACHE
	/* Check TA is correct for frames from associated STA */
	if (   _os_mem_cmp(drv_priv, sta->wrole->mac_addr,
	                   sta->mac_addr, MAC_ALEN)
	    !=  0) {
		u8 *wlan_hdr;

		#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
		wlan_hdr = phl_rx->r.rx_buf->hdr;
		#else /* CONFIG_RTW_HW_RX_AMSDU_CUT */
		wlan_hdr = phl_rx->r.pkt_list[0].vir_addr;
		#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

		if (   _os_mem_cmp(drv_priv, wlan_hdr + 10, sta->mac_addr, MAC_ALEN)
		    != 0) {
			phl_info->phl_com->rx_stats.ta_mismatch++;
			PHL_DBG("TA %pM != %pM (M%u)\n", wlan_hdr + 10, 
				  sta->mac_addr, meta->macid);
		}
	}
	#endif /* CONFIG_RTW_DEBUG_RX_CACHE */

	phl_rx->r.tx_sta = sta;
	phl_rx->r.rx_role = sta->wrole;

	rtw_hal_set_sta_rx_sts(sta, false, meta);

	if (tid >= ARRAY_SIZE(sta->tid_rx)) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_ERR_, "Fail: tid (%u) index out of range (%u)\n",
			tid, (u32)ARRAY_SIZE(sta->tid_rx));
		goto dont_reorder;
	}

	_os_spinlock(drv_priv, &sta->tid_rx_lock, _bh, NULL);

	r = sta->tid_rx[tid];
	if (!r) {
		_os_spinunlock(drv_priv, &sta->tid_rx_lock, _bh, NULL);
		goto dont_reorder;
	}

	if (!phl_manage_sta_reorder_buf(phl_info, phl_rx, r, frames)) {
		_os_spinunlock(drv_priv, &sta->tid_rx_lock, _bh, NULL);
		goto dont_reorder;
	}

	_os_spinunlock(drv_priv, &sta->tid_rx_lock, _bh, NULL);

	return RTW_PHL_STATUS_SUCCESS;

drop_frame:
#ifdef DEBUG_PHL_RX
	phl_info->phl_com->rx_stats.rx_drop_reorder++;
#endif
	hci_trx_ops->recycle_rx_pkt(phl_info, phl_rx);
	return RTW_PHL_STATUS_FAILURE;

dont_reorder:
#ifdef DEBUG_PHL_RX
	phl_info->phl_com->rx_stats.rx_dont_reorder++;
#endif
	list_add_tail(&phl_rx->list, frames);
	return RTW_PHL_STATUS_SUCCESS;
}


u8 phl_check_recv_ring_resource(struct phl_info_t *phl_info)
{
	struct rtw_phl_rx_ring *ring = &phl_info->phl_rx_ring;
	u16 avail = 0, wptr = 0, rptr = 0;

	wptr = ring->phl_idx;
	rptr = ring->core_idx;
	avail = phl_calc_avail_wptr(rptr, wptr, MAX_PHL_RING_ENTRY_NUM);

	if (0 == avail)
		return false;
	else
		return true;
}

void dump_phl_rx_ring(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	s16	diff = 0;
	u16 idx = 0, endidx = 0;
	u16 phl_idx = 0, core_idx = 0;

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "===Dump PHL RX Ring===\n");
	phl_idx = phl_info->phl_rx_ring.phl_idx;
	core_idx = phl_info->phl_rx_ring.core_idx;
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_,
			"core_idx = %d\n"
			"phl_idx = %d\n",
			core_idx,
			phl_idx);

	diff= phl_idx-core_idx;
	if(diff < 0)
		diff= MAX_PHL_RING_ENTRY_NUM+diff;

	endidx = diff > 5 ? (core_idx+6): phl_idx;
	for (idx = core_idx+1; idx < endidx; idx++) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "entry[%d] = %p\n", idx,
				phl_info->phl_rx_ring.entry[idx%MAX_PHL_RING_ENTRY_NUM]);
	}
}


void phl_event_indicator(void *context)
{
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_handler *phl_handler
		= (struct rtw_phl_handler *)phl_container_of(context,
							struct rtw_phl_handler,
							os_handler);
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_handler->context;
	struct rtw_phl_evt_ops *ops = NULL;
	struct rtw_evt_info_t *evt_info = NULL;
	void *drv_priv = NULL;
	enum rtw_phl_evt evt_bitmap = 0;
	FUNCIN_WSTS(sts);

	if (NULL != phl_info) {
		ops = &phl_info->phl_com->evt_ops;
		evt_info = &phl_info->phl_com->evt_info;
		drv_priv = phl_to_drvpriv(phl_info);

		_os_spinlock(drv_priv, &evt_info->evt_lock, _bh, NULL);
		evt_bitmap = evt_info->evt_bitmap;
		evt_info->evt_bitmap = 0;
		_os_spinunlock(drv_priv, &evt_info->evt_lock, _bh, NULL);

		if (RTW_PHL_EVT_RX & evt_bitmap) {
			if (NULL != ops->rx_process) {
				sts = ops->rx_process(drv_priv);
			}
			//dump_phl_rx_ring(phl_info);
		}
	}
	FUNCOUT_WSTS(sts);

}

void _phl_rx_statistics_reset(struct phl_info_t *phl_info)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_phl_stainfo_t *sta = NULL;
	struct rtw_wifi_role_t *role = NULL;
	void *drv = phl_to_drvpriv(phl_info);
	struct phl_queue *sta_queue;
	u8 i;

	for (i = 0; i< MAX_WIFI_ROLE_NUMBER; i++) {
		role = &phl_com->wifi_roles[i];
		if (role->active && (role->mstate == MLME_LINKED)) {
			sta_queue = &role->assoc_sta_queue;
			_os_spinlock(drv, &sta_queue->lock, _bh, NULL);
			phl_list_for_loop(sta, struct rtw_phl_stainfo_t,
						&sta_queue->queue, list) {
				if (sta)
					rtw_hal_set_sta_rx_sts(sta, true, NULL);
			}
			_os_spinunlock(drv, &sta_queue->lock, _bh, NULL);
		}
	}
}

void
phl_rx_watchdog(struct phl_info_t *phl_info)
{
	struct rtw_stats *phl_stats = &phl_info->phl_com->phl_stats;

	phl_rx_traffic_upd(phl_stats);
	phl_dump_rx_stats(phl_stats);
	_phl_rx_statistics_reset(phl_info);
}

u16 rtw_phl_query_new_rx_num(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_rx_ring *ring = NULL;
	u16 new_rx = 0, wptr = 0, rptr = 0;

	if (NULL != phl_info) {
		ring = &phl_info->phl_rx_ring;
		wptr = ring->phl_idx;
		rptr = ring->core_idx;
		new_rx = phl_calc_avail_rptr(rptr, wptr,
						MAX_PHL_RING_ENTRY_NUM);
	}

	return new_rx;
}

struct rtw_recv_pkt *rtw_phl_query_rx_pkt(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_rx_ring *ring = NULL;
	struct rtw_recv_pkt *recvpkt = NULL;
	u16 ring_res = 0, wptr = 0, rptr = 0;

	if (NULL != phl_info) {
		ring = &phl_info->phl_rx_ring;
		wptr = ring->phl_idx;
		rptr = ring->core_idx;

		ring_res = phl_calc_avail_rptr(rptr, wptr,
							MAX_PHL_RING_ENTRY_NUM);

		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_,
			"[4] %s::[Query] phl_idx =%d , core_idx =%d , ring_res =%d\n",
			__FUNCTION__,
			ring->phl_idx,
			ring->core_idx,
			ring_res);

		if (ring_res > 0) {
			rptr = rptr + 1;
			if (rptr >= MAX_PHL_RING_ENTRY_NUM)
				rptr=0;
			recvpkt = (struct rtw_recv_pkt *)ring->entry[rptr];
			ring->entry[rptr]=NULL;
			ring->core_idx = rptr;
			if (NULL == recvpkt)
				PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_, "recvpkt is NULL!\n");
			else
				phl_rx_statistics(phl_info, recvpkt);
		} else {
			PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "no available rx packet to query!\n");
		}
	}

	return recvpkt;
}


enum rtw_phl_status rtw_phl_return_rxbuf(void *phl, u8* recvpkt)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_rx_pkt *phl_rx = NULL;
	struct rtw_recv_pkt *r = (struct rtw_recv_pkt *)recvpkt;

	do {
		if (NULL == recvpkt)
			break;

		phl_rx = phl_container_of(r, struct rtw_phl_rx_pkt, r);
		phl_recycle_rx_buf(phl_info, phl_rx);
		pstatus = RTW_PHL_STATUS_SUCCESS;
	} while (false);

	return pstatus;
}

enum rtw_phl_status rtw_phl_start_rx_process(void *phl)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	FUNCIN_WSTS(pstatus);

	pstatus = phl_schedule_handler(phl_info->phl_com,
	                               &phl_info->phl_rx_handler);

	FUNCOUT_WSTS(pstatus);

	return pstatus;
}

void rtw_phl_rx_bar(void *phl, struct rtw_phl_stainfo_t *sta, u8 tid, u16 seq)
{
	/* ref ieee80211_rx_h_ctrl() and wil_rx_bar() */

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_tid_ampdu_rx *r;
	_os_list frames;

	if (tid >= ARRAY_SIZE(sta->tid_rx))
		return;

	INIT_LIST_HEAD(&frames);

	_os_spinlock(drv_priv, &sta->tid_rx_lock, _bh, NULL);

	r = sta->tid_rx[tid];
	if (!r) {
		_os_spinunlock(drv_priv, &sta->tid_rx_lock, _bh, NULL);
		PHL_TRACE(COMP_PHL_RECV, _PHL_ERR_, "BAR for non-existing TID %d\n", tid);
		return;
	}

	if (seq_less(seq, r->head_seq_num)) {
		_os_spinunlock(drv_priv, &sta->tid_rx_lock, _bh, NULL);
		PHL_TRACE(COMP_PHL_RECV, _PHL_ERR_, "BAR Seq 0x%03x preceding head 0x%03x\n",
					seq, r->head_seq_num);
		return;
	}

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "BAR: TID %d Seq 0x%03x head 0x%03x\n",
				tid, seq, r->head_seq_num);

	phl_release_reorder_frames(phl_info, r, seq, &frames);

	_os_spinunlock(drv_priv, &sta->tid_rx_lock, _bh, NULL);

	phl_handle_rx_frame_list(phl_info, &frames);

}

enum rtw_phl_status
rtw_phl_enter_mon_mode(void *phl, struct rtw_wifi_role_t *wrole)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status status;

	status = rtw_hal_enter_mon_mode(phl_info->hal, wrole->hw_band);
	if (status != RTW_HAL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_ERR_,
		          "%s(): rtw_hal_enter_mon_mode() failed, status=%d",
		          __FUNCTION__, status);
		return RTW_PHL_STATUS_FAILURE;
	}

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
rtw_phl_leave_mon_mode(void *phl, struct rtw_wifi_role_t *wrole)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status status;

	status = rtw_hal_leave_mon_mode(phl_info->hal, wrole->hw_band);
	if (status != RTW_HAL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_ERR_,
		          "%s(): rtw_hal_leave_mon_mode() failed, status=%d",
		          __FUNCTION__, status);
		return RTW_PHL_STATUS_FAILURE;
	}

	return RTW_PHL_STATUS_SUCCESS;
}

#ifdef CONFIG_PHL_RX_PSTS_PER_PKT
void
_phl_rx_proc_frame_list(struct phl_info_t *phl_info, struct phl_queue *pq)
{
	void *d = phl_to_drvpriv(phl_info);
	_os_list *pkt_list = NULL;
	struct rtw_phl_rx_pkt *phl_rx = NULL;

	if (NULL == pq)
		return;
	if (0 == pq->cnt)
		return;

	PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
		  "_phl_rx_proc_frame_list : queue ele cnt = %d\n",
		   pq->cnt);

	while (true == pq_pop(d, pq, &pkt_list, _first, _bh)) {
		phl_rx = (struct rtw_phl_rx_pkt *)pkt_list;
		phl_info->hci_trx_ops->rx_handle_normal(phl_info, phl_rx);
	}
}

void
_phl_rx_copy_phy_sts(struct rtw_phl_ppdu_phy_info *src, struct rtw_phl_ppdu_phy_info *dest)
{
	u8 i = 0;

	dest->is_valid = src->is_valid;
	dest->rssi = src->rssi;
	for (i = 0; i < RTW_PHL_MAX_RF_PATH; i++) {
		dest->rssi_path[i] = src->rssi_path[i];
		dest->snr_fd[i] = src->snr_fd[i];
		dest->snr_td[i] = src->snr_td[i];
	}
	/* dest->ch_idx = src->ch_idx; */
	dest->tx_bf = src->tx_bf;
	dest->frame_type = src->frame_type;
	dest->snr_fd_avg = src->snr_fd_avg;
	dest->snr_td_avg = src->snr_td_avg;
}

enum rtw_phl_status
phl_rx_proc_phy_sts(struct phl_info_t *phl_info, struct rtw_phl_rx_pkt *ppdu_sts)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_ppdu_sts_info *psts_info = &(phl_info->phl_com->ppdu_sts_info);
	struct rtw_phl_ppdu_sts_ent *sts_entry = NULL;
	struct rtw_phl_rx_pkt *phl_rx = NULL;
	struct rtw_r_meta_data *mdata;
	void *d = phl_to_drvpriv(phl_info);
	struct rtw_phl_rssi_stat *rssi_stat = &phl_info->phl_com->rssi_stat;
	_os_list *frame = NULL;
	bool upt_psts = true;
	u8 i = 0;
	enum phl_band_idx band = HW_BAND_0;

	if (NULL == ppdu_sts)
		return pstatus;

	if (false == psts_info->en_psts_per_pkt) {
		return pstatus;
	}

	if (ppdu_sts->r.mdata.ppdu_cnt >= PHL_MAX_PPDU_CNT) {
		PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
			  "ppdu_sts->r.mdata.ppdu_cnt >= PHL_MAX_PPDU_CNT!\n");
		return pstatus;
	}

	band = (ppdu_sts->r.mdata.bb_sel > 0) ? HW_BAND_1 : HW_BAND_0;

	if (false == psts_info->en_ppdu_sts[band])
		return pstatus;

	if (ppdu_sts->r.mdata.ppdu_cnt != psts_info->cur_ppdu_cnt[band]) {
		PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
			  "ppdu_sts->r.mdata.ppdu_cnt != psts_info->cur_ppdu_cnt!\n");
		upt_psts = false;
	}

	sts_entry = &psts_info->sts_ent[band][psts_info->cur_ppdu_cnt[band]];
	/* check list empty */
	if (0 == sts_entry->frames.cnt) {
		PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
			  "cur_ppdu_cnt %d --> sts_entry->frames.cnt = 0\n",
			  psts_info->cur_ppdu_cnt[band]);
		pstatus = RTW_PHL_STATUS_SUCCESS;
		return pstatus;
	}

	/* start update phy info to per pkt*/
	if (false == pq_get_front(d, &sts_entry->frames, &frame, _bh)) {
		PHL_ERR(" %s list empty\n", __FUNCTION__);
		return pstatus;
	}
	/**
	 * TODO : How to filter the case :
	 *	pkt(ppdu_cnt = 0) --> missing :psts(ppdu_cnt = 0) --> (all of the pkt, psts dropped/missing)
	 *	--> ppdu_sts(ppdu_cnt = 0)(not for the current buffered pkt.)
	 * workaround : check rate/bw/ppdu_type/... etc
	 **/
	phl_rx = (struct rtw_phl_rx_pkt *)frame;
	mdata = &phl_rx->r.mdata;
	if (upt_psts &&
	   ((mdata->rx_rate != ppdu_sts->r.mdata.rx_rate) ||
	    (mdata->bw != ppdu_sts->r.mdata.bw) ||
	    (mdata->rx_gi_ltf != ppdu_sts->r.mdata.rx_gi_ltf) ||
	    (mdata->ppdu_type != ppdu_sts->r.mdata.ppdu_type))) {
		    /**
		     * ppdu status is not for the buffered pkt,
		     * skip update phy status to phl_rx
		     **/
		    upt_psts = false;
	}
	/* Get Frame Type */
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	ppdu_sts->r.phy_info.frame_type =
			PHL_GET_80211_HDR_TYPE(phl_rx->r.rx_buf->hdr);
	#else
	ppdu_sts->r.phy_info.frame_type =
		PHL_GET_80211_HDR_TYPE(phl_rx->r.pkt_list[0].vir_addr);
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	if ((false == ppdu_sts->r.phy_info.is_valid) &&
	    (true == psts_info->en_fake_psts)) {
		if (phl_rx->r.phy_info.is_drvinfo_vld) {
			ppdu_sts->r.phy_info.rssi = phl_rx->r.phy_info.signal_strength;
			for(i = 0; i< RTW_PHL_MAX_RF_PATH ; i++) {
				ppdu_sts->r.phy_info.rssi_path[i] =
						phl_rx->r.phy_info.signal_strength;
			}
		} else {
			if (RTW_FRAME_TYPE_MGNT == phl_rx->r.mdata.frame_type) {
				ppdu_sts->r.phy_info.rssi =
					rssi_stat->ma_rssi[RTW_RSSI_MGNT_ACAM_A1M];
			} else if (RTW_FRAME_TYPE_DATA == phl_rx->r.mdata.frame_type) {
				ppdu_sts->r.phy_info.rssi =
					rssi_stat->ma_rssi[RTW_RSSI_DATA_ACAM_A1M];
			} else if (RTW_FRAME_TYPE_CTRL == phl_rx->r.mdata.frame_type) {
				ppdu_sts->r.phy_info.rssi =
					rssi_stat->ma_rssi[RTW_RSSI_CTRL_ACAM_A1M];
			} else {
				ppdu_sts->r.phy_info.rssi =
					rssi_stat->ma_rssi[RTW_RSSI_UNKNOWN];
			}
			for(i = 0; i< RTW_PHL_MAX_RF_PATH ; i++) {
				ppdu_sts->r.phy_info.rssi_path[i] =
						ppdu_sts->r.phy_info.rssi;
			}
			ppdu_sts->r.phy_info.ch_idx = rtw_hal_get_cur_ch(phl_info->hal,
							phl_rx->r.mdata.bb_sel);
		}
		ppdu_sts->r.phy_info.is_valid = true;
	}

	do {
		if (false == upt_psts)
			break;
		phl_rx = (struct rtw_phl_rx_pkt *)frame;
		if (phl_rx->r.phy_info.is_drvinfo_vld) {
			/* only copy the ppdu sts, avoid to override drvinfo */
			_phl_rx_copy_phy_sts(&(ppdu_sts->r.phy_info) ,&(phl_rx->r.phy_info));
		} else {
			_os_mem_cpy(d, &(phl_rx->r.phy_info), &(ppdu_sts->r.phy_info),
				    sizeof(struct rtw_phl_ppdu_phy_info));
		}
	} while ((true == psts_info->psts_ampdu) &&
		 (pq_get_next(d, &sts_entry->frames, frame, &frame, _bh)));

	/*2. indicate the frame list*/
	_phl_rx_proc_frame_list(phl_info, &sts_entry->frames);
	/*3. reset the queue */
	pq_reset(d, &(sts_entry->frames), _bh);

	return pstatus;
}

bool _phl_rx_proc_wait_phy_sts_check(struct phl_info_t *phl_info,
			 struct rtw_phl_rx_pkt *phl_rx)
{
	struct rtw_phl_ppdu_sts_info *psts_info = &(phl_info->phl_com->ppdu_sts_info);
	if (0 == (psts_info->ppdu_sts_filter &
		BIT(phl_rx->r.mdata.frame_type)))
		return false;
	/* data frames which a1 do not match */
	else if ((true == psts_info->data_only_invalid_macid) &&
		(phl_rx->r.mdata.frame_type == RTW_FRAME_TYPE_DATA) &&
		(phl_rx->r.mdata.macid != 0xff)) {
			return false;
		}

	return true;
}

bool
phl_rx_proc_wait_phy_sts(struct phl_info_t *phl_info,
			 struct rtw_phl_rx_pkt *phl_rx)
{
	struct rtw_phl_ppdu_sts_info *psts_info = &(phl_info->phl_com->ppdu_sts_info);
	struct rtw_phl_ppdu_sts_ent *sts_entry = NULL;
	void *d = phl_to_drvpriv(phl_info);
	u8 i = 0;
	bool ret = false;
	enum phl_band_idx band = HW_BAND_0;

	if (false == psts_info->en_psts_per_pkt) {
		return ret;
	}

	if (phl_rx->r.mdata.ppdu_cnt >= PHL_MAX_PPDU_CNT) {
		PHL_ASSERT("phl_rx->r.mdata.ppdu_cnt >= PHL_MAX_PPDU_CNT!");
		return ret;
	}

	band = (phl_rx->r.mdata.bb_sel > 0) ? HW_BAND_1 : HW_BAND_0;

	if (false == psts_info->en_ppdu_sts[band])
		return ret;

	if (psts_info->cur_ppdu_cnt[band] != phl_rx->r.mdata.ppdu_cnt) {
		/* start of PPDU */
		/* 1. Check all of the buffer list is empty */
		/* only check the target rx pkt band */
		for (i = 0; i < PHL_MAX_PPDU_CNT; i++) {
			sts_entry = &psts_info->sts_ent[band][i];
			if (0 != sts_entry->frames.cnt) {
				/* need indicate first */
				PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
					  "band %d ; ppdu_cnt %d queue is not empty \n",
					  band, i);
				_phl_rx_proc_frame_list(phl_info,
						&sts_entry->frames);
				pq_reset(d, &(sts_entry->frames), _bh);
			}
		}

		/* 2. check ppdu status filter condition */
		/* Filter function is supportted only if rxd = long_rxd */
		if ((1 == phl_rx->r.mdata.long_rxd) &&
			(true == _phl_rx_proc_wait_phy_sts_check(phl_info, phl_rx))) {
			/* 3. add new rx pkt to the tail of the queue */
			sts_entry = &psts_info->sts_ent[band][phl_rx->r.mdata.ppdu_cnt];
			pq_reset(d, &(sts_entry->frames), _bh);
			pq_push(d, &(sts_entry->frames), &phl_rx->list,
				_tail, _bh);
			ret = true;
		}
		psts_info->cur_ppdu_cnt[band] = phl_rx->r.mdata.ppdu_cnt;
	} else {
		/* 1. check ppdu status filter condition */
		/* Filter function is supportted only if rxd = long_rxd */
		if ((1 == phl_rx->r.mdata.long_rxd) &&
			(true == _phl_rx_proc_wait_phy_sts_check(phl_info, phl_rx))) {
			/* 2. add to frame list */
			sts_entry = &psts_info->sts_ent[band][phl_rx->r.mdata.ppdu_cnt];
			if (0 == sts_entry->frames.cnt) {
				PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
					  "MPDU is not the start of PPDU, but the queue is empty!!!\n");
			}
			pq_push(d, &(sts_entry->frames), &phl_rx->list,
				_tail, _bh);
			ret = true;
		}
	}

	return ret;
}
#endif

#ifdef CONFIG_PHY_INFO_NTFY
void _phl_rx_post_proc_ppdu_sts(void* priv, struct phl_msg* msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	if (msg->inbuf && msg->inlen){
		_os_kmem_free(phl_to_drvpriv(phl_info), msg->inbuf, msg->inlen);
	}
}

bool
_phl_rx_proc_aggr_psts_ntfy(struct phl_info_t *phl_info,
			    struct rtw_phl_ppdu_sts_ent *ppdu_sts_ent)
{
	struct rtw_phl_ppdu_sts_info *ppdu_info =
			&phl_info->phl_com->ppdu_sts_info;
	struct  rtw_phl_ppdu_sts_ntfy *psts_ntfy = NULL;
	u8 i = 0;
	bool ret = false;

	if (ppdu_info->msg_aggr_cnt == 0) {
		/* reset entry valid status  */
		for (i = 0; i < MAX_PSTS_MSG_AGGR_NUM; i++) {
			ppdu_info->msg_aggr_buf[i].vld = false;
		}
	}
	/* copy to the buf */
	psts_ntfy = &ppdu_info->msg_aggr_buf[ppdu_info->msg_aggr_cnt];
	psts_ntfy->frame_type = ppdu_sts_ent->frame_type;
	_os_mem_cpy(phl_info->phl_com->drv_priv,
		    &psts_ntfy->phy_info,
		    &ppdu_sts_ent->phy_info,
		    sizeof(struct rtw_phl_ppdu_phy_info));
	_os_mem_cpy(phl_info->phl_com->drv_priv,
		    psts_ntfy->src_mac_addr,
		    ppdu_sts_ent->src_mac_addr,
		    MAC_ADDRESS_LENGTH);
	psts_ntfy->vld = true;

	/* update counter */
	ppdu_info->msg_aggr_cnt++;
	if (ppdu_info->msg_aggr_cnt >= MAX_PSTS_MSG_AGGR_NUM) {
		ppdu_info->msg_aggr_cnt = 0;
		ret = true;
	}

	return ret;
}
#endif

void
phl_rx_proc_ppdu_sts(struct phl_info_t *phl_info, struct rtw_phl_rx_pkt *phl_rx)
{
	u8 i = 0;
	struct rtw_phl_ppdu_sts_info *ppdu_info = NULL;
	struct rtw_phl_ppdu_sts_ent *ppdu_sts_ent = NULL;
	struct rtw_phl_stainfo_t *psta = NULL;
#ifdef CONFIG_PHY_INFO_NTFY
	struct  rtw_phl_ppdu_sts_ntfy *psts_ntfy;
	void *d = phl_to_drvpriv(phl_info);
#endif
	enum phl_band_idx band = HW_BAND_0;
	struct rtw_rssi_info *rssi_sts;

	if ((NULL == phl_info) || (NULL == phl_rx))
		return;

	band = (phl_rx->r.mdata.bb_sel > 0) ? HW_BAND_1 : HW_BAND_0;
	ppdu_info = &phl_info->phl_com->ppdu_sts_info;
	ppdu_sts_ent = &ppdu_info->sts_ent[band][phl_rx->r.mdata.ppdu_cnt];

	if (false == ppdu_sts_ent->valid)
		return;

	if (true == ppdu_sts_ent->phl_done)
		return;

	ppdu_sts_ent->phl_done = true;

	/* update phl self varibles */
	for(i = 0 ; i < ppdu_sts_ent->usr_num; i++) {
		if (ppdu_sts_ent->sta[i].vld) {
			psta = rtw_phl_get_stainfo_by_macid(phl_info,
				 ppdu_sts_ent->sta[i].macid);
			if (psta == NULL)
				continue;
			rssi_sts = &psta->hal_sta->rssi_stat;
			STA_UPDATE_MA_RSSI(psta->hal_sta->rssi_stat,
					    ppdu_sts_ent->phy_info.rssi);
			/* update (re)associate req/resp pkt rssi */
			if (RTW_IS_ASOC_PKT(ppdu_sts_ent->frame_type)) {
				rssi_sts->assoc_rssi =
						ppdu_sts_ent->phy_info.rssi;
			}

			if (RTW_IS_BEACON_OR_PROBE_RESP_PKT(
						ppdu_sts_ent->frame_type)) {
				if (0 == rssi_sts->ma_rssi_mgnt) {
					rssi_sts->ma_rssi_mgnt =
						ppdu_sts_ent->phy_info.rssi;
				} else {
					STA_UPDATE_MA_RSSI_FAST(
						rssi_sts->ma_rssi_mgnt,
						ppdu_sts_ent->phy_info.rssi);
				}
			}
		}
		else {
			if (RTW_IS_ASOC_REQ_PKT(ppdu_sts_ent->frame_type) &&
				(ppdu_sts_ent->usr_num == 1)) {
				psta = rtw_phl_get_stainfo_by_addr_ex(phl_info,
						ppdu_sts_ent->src_mac_addr);
				if (psta) {
					psta->hal_sta->rssi_stat.assoc_rssi =
						ppdu_sts_ent->phy_info.rssi;

					#ifdef DBG_AP_CLIENT_ASSOC_RSSI
					PHL_INFO("%s [Rx-ASOC_REQ] - macid:%d, MAC-Addr:%02x-%02x-%02x-%02x-%02x-%02x, assoc_rssi:%d\n",
						__func__,
						psta->macid,
						ppdu_sts_ent->src_mac_addr[0],
						ppdu_sts_ent->src_mac_addr[1],
						ppdu_sts_ent->src_mac_addr[2],
						ppdu_sts_ent->src_mac_addr[3],
						ppdu_sts_ent->src_mac_addr[4],
						ppdu_sts_ent->src_mac_addr[5],
						psta->hal_sta->rssi_stat.assoc_rssi);
					#endif
				}
			}
		}
	}

#ifdef CONFIG_PHY_INFO_NTFY
	/*2. prepare and send psts notify to core */
	if((RTW_FRAME_TYPE_BEACON == ppdu_sts_ent->frame_type) ||
	   (RTW_FRAME_TYPE_PROBE_RESP == ppdu_sts_ent->frame_type)) {

		if (false == _phl_rx_proc_aggr_psts_ntfy(phl_info,
							 ppdu_sts_ent)) {
			return;
		}

		/* send aggr psts ntfy*/
		psts_ntfy = (struct rtw_phl_ppdu_sts_ntfy *)_os_kmem_alloc(d,
				MAX_PSTS_MSG_AGGR_NUM * sizeof(*psts_ntfy));
		if (psts_ntfy == NULL) {
			PHL_ERR("%s: alloc ppdu sts for ntfy fail.\n", __func__);
			return;
		}

		_os_mem_cpy(phl_info->phl_com->drv_priv,
			    psts_ntfy,
			    &ppdu_info->msg_aggr_buf,
			    (MAX_PSTS_MSG_AGGR_NUM *
			     sizeof(struct rtw_phl_ppdu_sts_ntfy)));

		msg.inbuf = (u8 *)psts_ntfy;
		msg.inlen = (MAX_PSTS_MSG_AGGR_NUM *
			     sizeof(struct rtw_phl_ppdu_sts_ntfy));
		SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_PSTS);
		SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_RX_PSTS);
		attr.completion.completion = _phl_rx_post_proc_ppdu_sts;
		attr.completion.priv = phl_info;
		if (phl_msg_hub_send(phl_info, &attr, &msg) != RTW_PHL_STATUS_SUCCESS) {
			PHL_ERR("%s: send msg_hub failed\n", __func__);
			_os_kmem_free(d, psts_ntfy,
				      (MAX_PSTS_MSG_AGGR_NUM *
				       sizeof(struct rtw_phl_ppdu_sts_ntfy)));
		}
	}
#endif
}

void phl_rx_wp_report_record_sts(struct phl_info_t *phl_info,
				 u8 macid, u16 ac_queue, u8 txsts)
{
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	struct rtw_hal_stainfo_t *hal_sta = NULL;
	struct rtw_wp_rpt_stats *wp_rpt_stats= NULL;

	phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, macid);

	if (phl_sta) {
		hal_sta = phl_sta->hal_sta;

		if (hal_sta->trx_stat.wp_rpt_stats == NULL) {
			PHL_ERR("rtp_stats NULL\n");
			return;
		}
		/* Record Per ac queue statistics */
		wp_rpt_stats = &hal_sta->trx_stat.wp_rpt_stats[ac_queue];

		_os_spinlock(phl_to_drvpriv(phl_info), &hal_sta->trx_stat.tx_sts_lock, _bh, NULL);
		if (TX_STATUS_TX_DONE == txsts) {
			/* record total tx ok*/
			hal_sta->trx_stat.tx_ok_cnt++;
			/* record per ac queue tx ok*/
			wp_rpt_stats->tx_ok_cnt++;
		} else {
			/* record total tx fail*/
			hal_sta->trx_stat.tx_fail_cnt++;
			/* record per ac queue tx fail*/
			if (TX_STATUS_TX_FAIL_REACH_RTY_LMT == txsts)
				wp_rpt_stats->rty_fail_cnt++;
			else if (TX_STATUS_TX_FAIL_LIFETIME_DROP == txsts)
				wp_rpt_stats->lifetime_drop_cnt++;
			else if (TX_STATUS_TX_FAIL_MACID_DROP == txsts)
				wp_rpt_stats->macid_drop_cnt++;
		}
		_os_spinunlock(phl_to_drvpriv(phl_info), &hal_sta->trx_stat.tx_sts_lock, _bh, NULL);

		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_,
			  "macid: %u, ac_queue: %u, tx_ok_cnt: %u, rty_fail_cnt: %u, "
			  "lifetime_drop_cnt: %u, macid_drop_cnt: %u\n"
			, macid, ac_queue, wp_rpt_stats->tx_ok_cnt, wp_rpt_stats->rty_fail_cnt
			, wp_rpt_stats->lifetime_drop_cnt, wp_rpt_stats->macid_drop_cnt);
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_,"totoal tx ok: %u \n totoal tx fail: %u\n"
			, hal_sta->trx_stat.tx_ok_cnt, hal_sta->trx_stat.tx_fail_cnt);
	} else {
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s: PHL_STA not found\n",
				__FUNCTION__);
	}
}
#ifdef CONFIG_PCI_HCI
u32 rtw_phl_get_hw_cnt_rdu(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);

	return hal_com->trx_stat.rx_rdu_cnt;
}
#endif
