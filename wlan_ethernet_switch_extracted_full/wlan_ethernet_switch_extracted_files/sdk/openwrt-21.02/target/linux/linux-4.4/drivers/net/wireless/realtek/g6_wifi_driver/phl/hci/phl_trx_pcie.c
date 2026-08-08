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
#define _PHL_TRX_PCIE_C_
#include "../phl_headers.h"
#include "phl_trx_pcie.h"

#define target_in_area(target, start, end) \
	((target < start || target > end) ? false : true)
void phl_recycle_payload(struct phl_info_t *phl_info, u8 dma_ch, u16 wp_seq,
			 u8 txsts, u8 code);

void _phl_dump_wp_stats(struct phl_info_t *phl_info)
{
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	struct rtw_wp_rpt_stats *rpt_stats = NULL;
	u8 ch = 0;

	rpt_stats = (struct rtw_wp_rpt_stats *)hal_com->trx_stat.wp_rpt_stats;

	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		  "\n== wp report statistics == \n");
	for (ch = 0; ch < hci_info->total_txch_num; ch++) {
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "ch			: %u\n", (int)ch);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "busy count		: %u\n",
			  (int)rpt_stats[ch].busy_cnt);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "ok count		: %u\n",
			  (int)rpt_stats[ch].tx_ok_cnt);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "retry fail count	: %u\n",
			  (int)rpt_stats[ch].rty_fail_cnt);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "lifetime drop count	: %u\n",
			  (int)rpt_stats[ch].lifetime_drop_cnt);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "macid drop count	: %u\n",
			  (int)rpt_stats[ch].macid_drop_cnt);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "sw drop count		: %u\n",
			  (int)rpt_stats[ch].sw_drop_cnt);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "recycle fail count	: %u\n",
			  (int)rpt_stats[ch].recycle_fail_cnt);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "delay ok count			: %u\n",
			  (int)rpt_stats[ch].delay_tx_ok_cnt);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "delay retry fail count		: %u\n",
			  (int)rpt_stats[ch].delay_rty_fail_cnt);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "delay lifetime drop count	: %u\n",
			  (int)rpt_stats[ch].delay_lifetime_drop_cnt);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
			  "delay macid drop count		: %u\n",
			  (int)rpt_stats[ch].delay_macid_drop_cnt);

	}
}

void _phl_dump_busy_wp(struct phl_info_t *phl_info)
{
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_wd_page_ring *wd_ring = NULL;
	struct rtw_xmit_req *treq = NULL;
	void *ptr = NULL;
	u16 wp_seq = 0;
	u8 ch = 0;

	wd_ring = (struct rtw_wd_page_ring *)hci_info->wd_ring;
	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		  "\n== dump busy wp == \n");
	for (ch = 0; ch < hci_info->total_txch_num; ch++) {

		for (wp_seq = 0; wp_seq < WP_MAX_SEQ_NUMBER; wp_seq++) {
			if (NULL != wd_ring[ch].wp_tag[wp_seq].ptr) {
				ptr = wd_ring[ch].wp_tag[wp_seq].ptr;
				treq = (struct rtw_xmit_req *)ptr;
				PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
					  "dma_ch = %d, wp_seq = 0x%x, ptr = %p!\n",
					  ch, wp_seq, ptr);
				PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
					  "wifi seq = %d\n",
					  treq->mdata.sw_seq);
			}
		}
	}


}

u8 _phl_check_recycle(u16 target, u16 rptr, u16 wptr, u16 bndy)
{
	u8 recycle = false;
	u8 init = 0;	/* starting point */

	if (wptr > rptr) {
		if (true == target_in_area(target, wptr, (bndy-1)))
			recycle = true;
		else if (true == target_in_area(target, init, rptr))
			recycle = true;
		else
			recycle = false;

	} else if (rptr > wptr) {
		if (true == target_in_area(target, wptr, rptr))
			recycle = true;
		else
			recycle = false;
	} else {
		recycle = true;
	}

	return recycle;
}

void phl_tx_start_pcie(struct phl_info_t *phl_info)
{
	void *drv = phl_to_drvpriv(phl_info);
	_os_atomic_set(drv, &phl_info->phl_sw_tx_sts, PHL_TX_STATUS_RUNNING);
}

void phl_tx_resume_pcie(struct phl_info_t *phl_info)
{
	void *drv = phl_to_drvpriv(phl_info);
	_os_atomic_set(drv, &phl_info->phl_sw_tx_sts, PHL_TX_STATUS_RUNNING);
}

void phl_req_tx_stop_pcie(struct phl_info_t *phl_info)
{
	void *drv = phl_to_drvpriv(phl_info);
	_os_atomic_set(drv, &phl_info->phl_sw_tx_sts,
				PHL_TX_STATUS_STOP_INPROGRESS);
}

void phl_tx_stop_pcie(struct phl_info_t *phl_info)
{
	void *drv = phl_to_drvpriv(phl_info);
	_os_atomic_set(drv, &phl_info->phl_sw_tx_sts, PHL_TX_STATUS_SW_PAUSE);
}

bool phl_is_tx_sw_pause_pcie(struct phl_info_t *phl_info)
{
	void *drvpriv = phl_to_drvpriv(phl_info);

	if (PHL_TX_STATUS_SW_PAUSE == _os_atomic_read(drvpriv,
								&phl_info->phl_sw_tx_sts))
		return true;
	else
		return false;

}

void phl_rx_start_pcie(struct phl_info_t *phl_info)
{
	void *drv = phl_to_drvpriv(phl_info);
	PHL_PRINT("%s\n", __FUNCTION__);
	_os_atomic_set(drv, &phl_info->phl_sw_rx_sts, PHL_RX_STATUS_RUNNING);
}

void phl_rx_resume_pcie(struct phl_info_t *phl_info)
{
	void *drv = phl_to_drvpriv(phl_info);
	PHL_PRINT("%s\n"
		  "\t1218 = 0x%08X - RX_BD_IDX\n"
		  "\t121C = 0x%08X - RP_BD_IDX\n"
		  "\t30B0 = 0x%08X - IMR0_V1\n"
		  "\t30B4 = 0x%08X - ISR0_V1\n"
		  "\t10B0 = 0x%08X - IMR0\n"
		  "\t10B4 = 0x%08X - ISR0\n",
	          __FUNCTION__,
	          rtw_phl_read32(phl_info, 0x1218),
	          rtw_phl_read32(phl_info, 0x121C),
	          rtw_phl_read32(phl_info, 0x30B0),
	          rtw_phl_read32(phl_info, 0x30B4),
	          rtw_phl_read32(phl_info, 0x10B0),
	          rtw_phl_read32(phl_info, 0x10B4));

	_os_atomic_set(drv, &phl_info->phl_sw_rx_sts, PHL_RX_STATUS_RUNNING);
}

void phl_req_rx_stop_pcie(struct phl_info_t *phl_info)
{
	void *drv = phl_to_drvpriv(phl_info);
	PHL_PRINT("%s\n"
		  "\t1218 = 0x%08X - RX_BD_IDX\n"
		  "\t121C = 0x%08X - RP_BD_IDX\n"
		  "\t30B0 = 0x%08X - IMR0_V1\n"
		  "\t30B4 = 0x%08X - ISR0_V1\n"
		  "\t10B0 = 0x%08X - IMR0\n"
		  "\t10B4 = 0x%08X - ISR0\n",
	          __FUNCTION__,
	          rtw_phl_read32(phl_info, 0x1218),
	          rtw_phl_read32(phl_info, 0x121C),
	          rtw_phl_read32(phl_info, 0x30B0),
	          rtw_phl_read32(phl_info, 0x30B4),
	          rtw_phl_read32(phl_info, 0x10B0),
	          rtw_phl_read32(phl_info, 0x10B4));
	_os_atomic_set(drv, &phl_info->phl_sw_rx_sts,
			PHL_RX_STATUS_STOP_INPROGRESS);
}

void phl_rx_stop_pcie(struct phl_info_t *phl_info)
{
	void *drv = phl_to_drvpriv(phl_info);
	PHL_PRINT("%s\n", __FUNCTION__);
	_os_atomic_set(drv, &phl_info->phl_sw_rx_sts, PHL_RX_STATUS_SW_PAUSE);
}

bool phl_is_rx_sw_pause_pcie(struct phl_info_t *phl_info)
{
	void *drvpriv = phl_to_drvpriv(phl_info);

	if (PHL_RX_STATUS_SW_PAUSE == _os_atomic_read(drvpriv,
								&phl_info->phl_sw_rx_sts)) {
		if (true == rtw_phl_is_phl_rx_idle(phl_info))
			return true;
		else
			return false;
	} else {
		return false;
	}
}

#ifdef RTW_WKARD_DYNAMIC_LTR
static bool _phl_judge_idle_ltr_switching_conditions(
	struct phl_info_t *phl_info, u16 macid)
{
	struct rtw_phl_stainfo_t *sta_info = NULL;
	struct rtw_stats *stats = &phl_info->phl_com->phl_stats;
	u16 ltr_thre = phl_info->phl_com->bus_sw_cap.ltr_sw_ctrl_thre;
	u8 tx_thre = 0, rx_thre = 0;
	u32 last_time = rtw_hal_ltr_get_last_trigger_time(phl_info->hal);

	tx_thre = ltr_thre >> 8;
	rx_thre = (u8)(ltr_thre & 0xFF);

	sta_info = rtw_phl_get_stainfo_by_macid(phl_info, macid);

	if (!rtw_hal_ltr_is_sw_ctrl(phl_info->hal))
		return false;

	if (sta_info == NULL)
		return false;

	if (sta_info->wrole == NULL)
		return false;

	if (stats->tx_traffic.lvl > tx_thre)
		return false;

	if (stats->rx_traffic.lvl > rx_thre)
		return false;

	if (RTW_PCIE_LTR_SW_IDLE == rtw_hal_ltr_get_cur_state(phl_info->hal))
		return false;

	if (phl_get_passing_time_us(last_time) < 500)
		return false;

	return true;

}
static bool _phl_judge_act_ltr_switching_conditions(
	struct phl_info_t *phl_info, u8 ch)
{
	u32 last_time = rtw_hal_ltr_get_last_trigger_time(phl_info->hal);
	u8 fwcmd_queue_idx = 0;

	fwcmd_queue_idx = rtw_hal_get_fwcmd_queue_idx(phl_info->hal);

	if (!rtw_hal_ltr_is_sw_ctrl(phl_info->hal))
		return true;

	if (ch == fwcmd_queue_idx)
		return true;

	if (RTW_PCIE_LTR_SW_ACT == rtw_hal_ltr_get_cur_state(phl_info->hal))
		return true;

	if (phl_get_passing_time_us(last_time) < 500)
		return false;

	return true;
}

static void _phl_act_ltr_update_stats(struct phl_info_t *phl_info,
		bool success, u8 ch, u16 pending_wd_page_cnt)
{
	static bool bdly = false;
	static u32 dly_start_time = 0;

	if (!rtw_hal_ltr_is_sw_ctrl(phl_info->hal))
		return;

	if (success) {
		/* only those have been delayed last time*/
		if (bdly) {
			PHL_INFO("%s() ch(%u), %u packets be transmitted after defering %uus\n"
				, __func__, ch,	pending_wd_page_cnt,
				phl_get_passing_time_us(dly_start_time));
			rtw_hal_ltr_update_stats(phl_info->hal, true);
		}
		bdly = false;
	} else {

		/* the first packet that is going to defer */
		if (false == bdly)
			dly_start_time = _os_get_cur_time_us();

		PHL_DBG("%s() ch(%u), %u packets be delayed\n", __func__,
							ch,	pending_wd_page_cnt);

		rtw_hal_ltr_update_stats(phl_info->hal, false);
		bdly = true;
		dly_start_time = _os_get_cur_time_us();
	}
}

static void _phl_switch_act_ltr(struct phl_info_t *phl_info, u8 tx_dma_ch)
{
	u8 fwcmd_queue_idx = 0;

	if (!rtw_hal_ltr_is_sw_ctrl(phl_info->hal))
		return;

	if (RTW_PCIE_LTR_SW_ACT == rtw_hal_ltr_get_cur_state(phl_info->hal))
		return;

	fwcmd_queue_idx = rtw_hal_get_fwcmd_queue_idx(phl_info->hal);

	if (tx_dma_ch != fwcmd_queue_idx)
		rtw_hal_ltr_sw_trigger(phl_info->hal, RTW_PCIE_LTR_SW_ACT);

}

static void _phl_switch_idle_ltr(struct phl_info_t *phl_info,
					struct rtw_wp_rpt_stats *rpt_stats)
{
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	u8 ch = 0;
	bool bempty = 1;
	u8 fwcmd_queue_idx = 0;

	fwcmd_queue_idx = rtw_hal_get_fwcmd_queue_idx(phl_info->hal);

	for (ch = 0; ch < hci_info->total_txch_num; ch++) {
		if (ch == fwcmd_queue_idx)
			continue;
		if (rpt_stats[ch].busy_cnt != 0)
			bempty = 0;
	}

	if (bempty)
		rtw_hal_ltr_sw_trigger(phl_info->hal, RTW_PCIE_LTR_SW_IDLE);

}
#endif

#ifdef RTW_WKARD_TXBD_UPD_LMT
static void
_phl_free_h2c_work_ring(struct phl_info_t *phl_info,
			struct rtw_wd_page_ring *wd_page_ring)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_info->phl_com);
	struct rtw_h2c_work *h2c_work = &wd_page_ring->h2c_work;
	struct rtw_h2c_pkt *cmd = h2c_work->cmd;
	struct rtw_h2c_pkt *data = h2c_work->data;
	struct rtw_h2c_pkt *ldata = h2c_work->ldata;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	u16 i = 0, buf_num = 0;

	buf_num = hal_spec->txbd_multi_tag;

	if (NULL != cmd) {
		for (i = 0; i < buf_num; i++) {
			if (NULL == cmd->vir_head)
				continue;
			hci_trx_ops->free_h2c_pkt_buf(phl_info, cmd);
			cmd->vir_head = NULL;
			cmd->cache = false;
			cmd++;
		}
		_os_mem_free(drv_priv, h2c_work->cmd,
			     buf_num * sizeof(*h2c_work->cmd));
	}
	if (NULL != data) {
		for (i = 0; i < buf_num; i++) {
			if (NULL == data->vir_head)
				continue;
			hci_trx_ops->free_h2c_pkt_buf(phl_info, data);
			data->vir_head = NULL;
			data->cache = false;
			data++;
		}
		_os_mem_free(drv_priv, h2c_work->data,
			     buf_num * sizeof(*h2c_work->data));
	}
	if (NULL != ldata) {
		for (i = 0; i < buf_num; i++) {
			if (NULL == ldata->vir_head)
				continue;
			hci_trx_ops->free_h2c_pkt_buf(phl_info, ldata);
			ldata->vir_head = NULL;
			ldata->cache = false;
			ldata++;
		}
		_os_mem_free(drv_priv, h2c_work->ldata,
			     buf_num * sizeof(*h2c_work->ldata));
	}

	if (NULL != h2c_work->cmd_ring) {
		_os_mem_free(drv_priv, h2c_work->cmd_ring,
			     buf_num * sizeof(struct rtw_h2c_pkt *));
        }
	if (NULL != h2c_work->data_ring) {
		_os_mem_free(drv_priv, h2c_work->data_ring,
			     buf_num * sizeof(struct rtw_h2c_pkt *));
        }
	if (NULL != h2c_work->ldata_ring) {
		_os_mem_free(drv_priv, h2c_work->ldata_ring,
			     buf_num * sizeof(struct rtw_h2c_pkt *));
        }
	h2c_work->cmd_cnt = 0;
	h2c_work->cmd_idx = 0;
	h2c_work->data_cnt = 0;
	h2c_work->data_idx = 0;
	h2c_work->ldata_cnt = 0;
	h2c_work->ldata_idx = 0;
	_os_spinlock_free(drv_priv,	&h2c_work->lock);
}


static enum rtw_phl_status
_phl_alloc_h2c_work_ring(struct phl_info_t *phl_info,
			 struct rtw_wd_page_ring *wd_page_ring)
{
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_info->phl_com);
	struct rtw_h2c_work *h2c_work = &wd_page_ring->h2c_work;
	struct rtw_h2c_pkt *cmd = NULL, *data =  NULL, *ldata = NULL;
	u16 buf_num = 0, i = 0;

	buf_num = hal_spec->txbd_multi_tag;
	_os_spinlock_init(drv_priv, &h2c_work->lock);

	h2c_work->cmd = _os_mem_alloc(drv_priv, buf_num * sizeof(*cmd));
	h2c_work->data = _os_mem_alloc(drv_priv, buf_num * sizeof(*data));
	h2c_work->ldata = _os_mem_alloc(drv_priv, buf_num * sizeof(*ldata));

	if (!h2c_work->cmd || !h2c_work->data || !h2c_work->ldata) {
		psts = RTW_PHL_STATUS_RESOURCE;
		goto out;
	}
	cmd = h2c_work->cmd;
	data = h2c_work->data;
	ldata = h2c_work->ldata;

	_os_mem_set(drv_priv, cmd, 0, buf_num * sizeof(*cmd));
	_os_mem_set(drv_priv, data, 0, buf_num * sizeof(*data));
	_os_mem_set(drv_priv, ldata, 0, buf_num * sizeof(*ldata));

	h2c_work->cmd_ring =
		_os_mem_alloc(drv_priv,
			       buf_num * sizeof(struct rtw_h2c_pkt *));
	h2c_work->data_ring =
		_os_mem_alloc(drv_priv,
			       buf_num * sizeof(struct rtw_h2c_pkt *));
	h2c_work->ldata_ring =
		_os_mem_alloc(drv_priv,
			       buf_num * sizeof(struct rtw_h2c_pkt *));

	if (!h2c_work->cmd_ring || !h2c_work->data_ring ||
	    !h2c_work->ldata_ring) {
		psts = RTW_PHL_STATUS_RESOURCE;
		goto out;
	}
	_os_mem_set(drv_priv, h2c_work->cmd_ring, 0,
		    buf_num * sizeof(struct rtw_h2c_pkt *));
	_os_mem_set(drv_priv, h2c_work->data_ring, 0,
		    buf_num * sizeof(struct rtw_h2c_pkt *));
	_os_mem_set(drv_priv, h2c_work->ldata_ring, 0,
		    buf_num * sizeof(struct rtw_h2c_pkt *));

	for (i = 0; i < buf_num; i++) {
		cmd->type = H2CB_TYPE_CMD;
		cmd->cache = false;
		cmd->buf_len = FWCMD_HDR_LEN + _WD_BODY_LEN + H2C_CMD_LEN;
		hci_trx_ops->alloc_h2c_pkt_buf(phl_info, cmd, cmd->buf_len);
		if (NULL == cmd->vir_head) {
			psts = RTW_PHL_STATUS_RESOURCE;
			goto out;
		}
		cmd->vir_data = cmd->vir_head + FWCMD_HDR_LEN + _WD_BODY_LEN;
		cmd->vir_tail = cmd->vir_data;
		cmd->vir_end = cmd->vir_data + H2C_CMD_LEN;
		INIT_LIST_HEAD(&cmd->list);
		h2c_work->cmd_ring[i] = cmd;
		h2c_work->cmd_cnt++;
		cmd++;
	}
	for (i = 0; i < buf_num; i++) {
		data->type = H2CB_TYPE_DATA;
		data->cache = false;
		data->buf_len = FWCMD_HDR_LEN + _WD_BODY_LEN + H2C_DATA_LEN;
		hci_trx_ops->alloc_h2c_pkt_buf(phl_info, data, data->buf_len);
		if (NULL == data->vir_head) {
			psts = RTW_PHL_STATUS_RESOURCE;
			goto out;
		}
		data->vir_data = data->vir_head + FWCMD_HDR_LEN + _WD_BODY_LEN;
		data->vir_tail = data->vir_data;
		data->vir_end = data->vir_data + H2C_DATA_LEN;
		INIT_LIST_HEAD(&data->list);
		h2c_work->data_ring[i] = data;
		h2c_work->data_cnt++;
		data++;
	}
	for (i = 0; i < buf_num; i++) {
		ldata->type = H2CB_TYPE_LONG_DATA;
		ldata->cache = false;
		ldata->buf_len = FWCMD_HDR_LEN + _WD_BODY_LEN +
				 H2C_LONG_DATA_LEN;
		hci_trx_ops->alloc_h2c_pkt_buf(phl_info, ldata, ldata->buf_len);
		if (NULL == ldata->vir_head) {
			psts = RTW_PHL_STATUS_RESOURCE;
			goto out;
		}
		ldata->vir_data = ldata->vir_head + FWCMD_HDR_LEN +
				 _WD_BODY_LEN;
		ldata->vir_tail = ldata->vir_data;
		ldata->vir_end = ldata->vir_data + H2C_LONG_DATA_LEN;
		INIT_LIST_HEAD(&ldata->list);
		h2c_work->ldata_ring[i] = ldata;
		h2c_work->ldata_cnt++;
		ldata++;
	}

	h2c_work->cmd_idx = 0;
	h2c_work->data_idx = 0;
	h2c_work->ldata_idx = 0;
	psts = RTW_PHL_STATUS_SUCCESS;

out:
	if (RTW_PHL_STATUS_SUCCESS != psts) {
		_phl_free_h2c_work_ring(phl_info, wd_page_ring);
		h2c_work->cmd = NULL;
		h2c_work->data = NULL;
		h2c_work->ldata = NULL;
		h2c_work->cmd_ring = NULL;
		h2c_work->data_ring = NULL;
		h2c_work->ldata_ring = NULL;
	}

	return psts;
}


static void
_phl_free_wd_work_ring(struct phl_info_t *phl_info,
		       struct rtw_wd_page_ring *wd_page_ring)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_info->phl_com);
	u16 i = 0, buf_num = 0;

	buf_num = hal_spec->txbd_multi_tag;

	if (NULL != wd_page_ring->wd_work) {
		for (i = 0; i < buf_num; i++) {

			if (NULL == wd_page_ring->wd_work[i].vir_addr)
				continue;

			wd_page_ring->wd_work[i].wp_seq = WP_RESERVED_SEQ;
			_os_shmem_free(drv_priv,
			       	wd_page_ring->wd_work[i].vir_addr,
			       	&wd_page_ring->wd_work[i].phy_addr_l,
			       	&wd_page_ring->wd_work[i].phy_addr_h,
			       	wd_page_ring->wd_work[i].buf_len,
				wd_page_ring->wd_work[i].cache,
				PCI_DMA_FROMDEVICE,
				wd_page_ring->wd_work[i].os_rsvd[0]);
			wd_page_ring->wd_work[i].vir_addr = NULL;
			wd_page_ring->wd_work[i].cache = 0;
		}

		_os_mem_free(drv_priv, wd_page_ring->wd_work,
			      buf_num * sizeof(*wd_page_ring->wd_work));
		wd_page_ring->wd_work = NULL;
	}

	if (NULL != wd_page_ring->wd_work_ring) {
		_os_mem_free(drv_priv, wd_page_ring->wd_work_ring,
			      buf_num * sizeof(struct rtw_wd_page *));
		wd_page_ring->wd_work_ring = NULL;
        }
	wd_page_ring->wd_work_cnt = 0;
	wd_page_ring->wd_work_idx = 0;

}

static enum rtw_phl_status
_phl_alloc_wd_work_ring(struct phl_info_t *phl_info,
			struct rtw_wd_page_ring *wd_page_ring)
{
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_info->phl_com);
	struct rtw_wd_page *wd_work = NULL;
	u32 buf_len = 0;
	u16 buf_num = 0, i = 0;

	buf_num = hal_spec->txbd_multi_tag;

	wd_page_ring->wd_work = _os_mem_alloc(drv_priv,
					       buf_num * sizeof(*wd_work));
	if (!wd_page_ring->wd_work) {
		psts = RTW_PHL_STATUS_RESOURCE;
		goto out;
	}
	wd_work = wd_page_ring->wd_work;
	_os_mem_set(drv_priv, wd_work, 0, buf_num * sizeof(*wd_work));

	wd_page_ring->wd_work_ring =
		_os_mem_alloc(drv_priv,
			       buf_num * sizeof(struct rtw_wd_page *));
	if (!wd_page_ring->wd_work_ring) {
		psts = RTW_PHL_STATUS_RESOURCE;
		goto out;
	}
	_os_mem_set(drv_priv, wd_page_ring->wd_work_ring, 0,
		    buf_num * sizeof(struct rtw_wd_page *));

	for (i = 0; i < buf_num; i++) {
		wd_work[i].cache = true;
		buf_len = WD_PAGE_SIZE;
		wd_work[i].vir_addr = _os_shmem_alloc(drv_priv,
					&wd_work[i].phy_addr_l,
					&wd_work[i].phy_addr_h,
					buf_len,
					wd_work[i].cache,
					PCI_DMA_TODEVICE,
					&wd_work[i].os_rsvd[0]);
 		if (NULL == wd_work[i].vir_addr) {
			psts = RTW_PHL_STATUS_RESOURCE;
			goto out;
		}
		wd_work[i].buf_len = buf_len;
		wd_work[i].wp_seq = WP_RESERVED_SEQ;
		INIT_LIST_HEAD(&wd_work[i].list);

		wd_page_ring->wd_work_ring[i] = &wd_work[i];
		wd_page_ring->wd_work_cnt++;
		/* hana_todo now check 4 byte align only */
		/* if ((unsigned long)wd_page_buf & 0xF) { */
		/* 	res = _FAIL; */
		/* 	break; */
		/* } */
	}

	wd_page_ring->wd_work_idx = 0;
	psts = RTW_PHL_STATUS_SUCCESS;

out:
	if (RTW_PHL_STATUS_SUCCESS != psts) {
		_phl_free_wd_work_ring(phl_info, wd_page_ring);
		wd_page_ring->wd_work = NULL;
		wd_page_ring->wd_work_ring = NULL;
	}

	return psts;
}
#else
#define _phl_free_h2c_work_ring(_phl, _ring)
#define _phl_alloc_h2c_work_ring(_phl, _ring) RTW_PHL_STATUS_SUCCESS
#define _phl_free_wd_work_ring(_phl, _ring)
#define _phl_alloc_wd_work_ring(_phl, _ring) RTW_PHL_STATUS_SUCCESS
#endif

static enum rtw_phl_status enqueue_pending_wd_page(struct phl_info_t *phl_info,
				struct rtw_wd_page_ring *wd_page_ring,
				struct rtw_wd_page *wd_page, u8 pos)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &wd_page_ring->pending_wd_page_list;

	if (wd_page != NULL) {
		_os_spinlock(drv_priv, &wd_page_ring->pending_lock, _bh, NULL);

		if (_tail == pos)
			list_add_tail(&wd_page->list, list);
		else if (_first == pos)
			list_add(&wd_page->list, list);

		wd_page_ring->pending_wd_page_cnt++;

		_os_spinunlock(drv_priv, &wd_page_ring->pending_lock, _bh, NULL);

		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}

static enum rtw_phl_status enqueue_busy_wd_page(struct phl_info_t *phl_info,
				struct rtw_wd_page_ring *wd_page_ring,
				struct rtw_wd_page *wd_page, u8 pos)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &wd_page_ring->busy_wd_page_list;

	if (wd_page != NULL) {
		_os_spinlock(drv_priv, &wd_page_ring->busy_lock, _bh, NULL);

		if (_tail == pos)
			list_add_tail(&wd_page->list, list);
		else if (_first == pos)
			list_add(&wd_page->list, list);

		wd_page_ring->busy_wd_page_cnt++;

		_os_spinunlock(drv_priv, &wd_page_ring->busy_lock, _bh, NULL);

		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}


static enum rtw_phl_status enqueue_idle_wd_page(
				struct phl_info_t *phl_info,
				struct rtw_wd_page_ring *wd_page_ring,
				struct rtw_wd_page *wd_page)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &wd_page_ring->idle_wd_page_list;

	if (wd_page != NULL) {
		wd_page->buf_len = WD_PAGE_SIZE;
		wd_page->wp_seq = WP_RESERVED_SEQ;
		wd_page->host_idx = 0;
		INIT_LIST_HEAD(&wd_page->list);

		_os_spinlock(drv_priv, &wd_page_ring->idle_lock, _bh, NULL);

		list_add_tail(&wd_page->list, list);
		wd_page_ring->idle_wd_page_cnt++;

		_os_spinunlock(drv_priv, &wd_page_ring->idle_lock, _bh, NULL);

		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}

#ifdef RTW_WKARD_TXBD_UPD_LMT
static enum rtw_phl_status enqueue_h2c_work_ring(
				struct phl_info_t *phl_info,
				struct rtw_h2c_pkt *h2c)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_wd_page_ring *wd_ring = NULL;
	struct rtw_h2c_work *h2c_work = NULL;
	struct rtw_h2c_pkt *work_done_h2c = NULL;
	struct rtw_h2c_pkt **ring = NULL;
	u16 *idx = 0, *cnt = 0;
	u8 fwcmd_qidx = 0;

	fwcmd_qidx = rtw_hal_get_fwcmd_queue_idx(phl_info->hal);
	wd_ring = (struct rtw_wd_page_ring *)hci_info->wd_ring;
	h2c_work = &wd_ring[fwcmd_qidx].h2c_work;

	if (h2c == NULL)
		goto out;

	_os_spinlock(drv_priv, &h2c_work->lock, _bh, NULL);

	if (H2CB_TYPE_CMD == h2c->type) {
		ring = h2c_work->cmd_ring;
		idx = &h2c_work->cmd_idx;
		cnt = &h2c_work->cmd_cnt;
	} else if (H2CB_TYPE_DATA == h2c->type) {
		ring = h2c_work->data_ring;
		idx = &h2c_work->data_idx;
		cnt = &h2c_work->data_cnt;
	} else if (H2CB_TYPE_LONG_DATA == h2c->type) {
		ring = h2c_work->ldata_ring;
		idx = &h2c_work->ldata_idx;
		cnt = &h2c_work->ldata_cnt;
	} else {
		_os_spinunlock(drv_priv, &h2c_work->lock, _bh, NULL);
		goto out;
	}

	work_done_h2c = ring[*idx];
	ring[*idx] = h2c;
	*idx = (*idx + 1) % *cnt;

	_os_spinunlock(drv_priv, &h2c_work->lock, _bh, NULL);

	pstatus = phl_enqueue_idle_h2c_pkt(phl_info, work_done_h2c);

out:
	return pstatus;
}

static enum rtw_phl_status enqueue_wd_work_ring(
				struct phl_info_t *phl_info,
				struct rtw_wd_page_ring *wd_page_ring,
				struct rtw_wd_page *wd_page)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_wd_page *work_done_wd = NULL;
	struct rtw_wd_page **ring = wd_page_ring->wd_work_ring;

	if (wd_page != NULL) {

		_os_spinlock(drv_priv, &wd_page_ring->work_lock, _bh, NULL);

		work_done_wd = ring[wd_page_ring->wd_work_idx];
		ring[wd_page_ring->wd_work_idx] = wd_page;
		wd_page_ring->wd_work_idx =
		    (wd_page_ring->wd_work_idx + 1) % wd_page_ring->wd_work_cnt;

		_os_spinunlock(drv_priv, &wd_page_ring->work_lock, _bh, NULL);

		enqueue_idle_wd_page(phl_info, wd_page_ring, work_done_wd);

		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}
#else
#define enqueue_h2c_work_ring(_phl, _h2c) RTW_PHL_STATUS_FAILURE
#define enqueue_wd_work_ring(_phl, _ring, _wd) RTW_PHL_STATUS_FAILURE
#endif


static struct rtw_wd_page *query_pending_wd_page(struct phl_info_t *phl_info,
				struct rtw_wd_page_ring *wd_page_ring)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *pending_list = &wd_page_ring->pending_wd_page_list;
	struct rtw_wd_page *wd_page = NULL;

	_os_spinlock(drv_priv, &wd_page_ring->pending_lock, _bh, NULL);

	if (true == list_empty(pending_list)) {
		wd_page = NULL;
	} else {
		wd_page = list_first_entry(pending_list, struct rtw_wd_page,
						list);
		wd_page_ring->pending_wd_page_cnt--;
		list_del(&wd_page->list);
	}

	_os_spinunlock(drv_priv, &wd_page_ring->pending_lock, _bh, NULL);

	return wd_page;
}


static struct rtw_wd_page *query_idle_wd_page(struct phl_info_t *phl_info,
				struct rtw_wd_page_ring *wd_page_ring)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *idle_list = &wd_page_ring->idle_wd_page_list;
	struct rtw_wd_page *wd_page = NULL;

	_os_spinlock(drv_priv, &wd_page_ring->idle_lock, _bh, NULL);

	if (true == list_empty(idle_list)) {
		wd_page = NULL;
	} else {
		wd_page = list_first_entry(idle_list, struct rtw_wd_page, list);
		wd_page_ring->idle_wd_page_cnt--;
		list_del(&wd_page->list);
	}

	_os_spinunlock(drv_priv, &wd_page_ring->idle_lock, _bh, NULL);

	return wd_page;
}

static enum rtw_phl_status rtw_release_target_wd_page(
					struct phl_info_t *phl_info,
					struct rtw_wd_page_ring *wd_page_ring,
					struct rtw_wd_page *wd_page)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if (wd_page_ring != NULL && wd_page != NULL) {
		enqueue_idle_wd_page(phl_info, wd_page_ring, wd_page);
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}

enum rtw_phl_status rtw_release_pending_wd_page(
				struct phl_info_t *phl_info,
				struct rtw_wd_page_ring *wd_page_ring,
				u16 release_num)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	_os_list *list = &wd_page_ring->pending_wd_page_list;
	struct rtw_wd_page *wd_page = NULL;

	if (wd_page_ring != NULL) {
		while (release_num > 0 && true != list_empty(list)) {

			wd_page = query_pending_wd_page(phl_info, wd_page_ring);

			enqueue_idle_wd_page(phl_info, wd_page_ring, wd_page);

			release_num--;
		}
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}
	return pstatus;
}

enum rtw_phl_status rtw_release_busy_wd_page(
				struct phl_info_t *phl_info,
				struct rtw_wd_page_ring *wd_page_ring,
				u8 ch,
				u16 release_num)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	_os_list *list = &wd_page_ring->busy_wd_page_list;
	struct rtw_wd_page *wd_page = NULL;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_info->phl_com);
	#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	#endif

	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct tx_base_desc *txbd = NULL;
	txbd = (struct tx_base_desc *)hci_info->txbd_buf;

	if (wd_page_ring != NULL) {
		_os_spinlock(drv_priv, &wd_page_ring->busy_lock, _bh, NULL);

		while (release_num > 0 && true != list_empty(list)) {

			wd_page = list_first_entry(list, struct rtw_wd_page,
							list);
			wd_page_ring->busy_wd_page_cnt--;
			list_del(&wd_page->list);

			#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
			//phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, wd_page->mac_id);
			phl_sta = rtw_phl_get_stainfo_by_macid_wo_lock(phl_info, wd_page->mac_id);
			if(phl_sta){
				if(phl_sta->active)
					phl_sta->wd_deq_busy++;
			}
			#endif

			_os_spinunlock(drv_priv, &wd_page_ring->busy_lock, _bh, NULL);
			if (true == hal_spec->txbd_upd_lmt) {
				pstatus = enqueue_wd_work_ring(phl_info,
							       wd_page_ring,
							       wd_page);
			} else {
				pstatus = enqueue_idle_wd_page(phl_info,
							       wd_page_ring,
							       wd_page);
			}
			_os_spinlock(drv_priv, &wd_page_ring->busy_lock, _bh, NULL);

			if (RTW_PHL_STATUS_SUCCESS != pstatus)
				break;
			_os_atomic_inc(NULL, &txbd[ch].avail_num);
			release_num--;
		}
		_os_spinunlock(drv_priv, &wd_page_ring->busy_lock, _bh, NULL);

	}
	return pstatus;
}

static void _phl_reset_txbd(struct phl_info_t *phl_info,
				struct tx_base_desc *txbd)
{
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	_os_mem_set(phl_to_drvpriv(phl_info), txbd->vir_addr, 0, txbd->buf_len);
	txbd->host_idx = 0;
	txbd->hw_idx = 0;
	_os_atomic_set(phl_to_drvpriv(phl_info), &txbd->avail_num, hal_com->bus_cap.txbd_num-1);
	//txbd->avail_num = (u16)hal_com->bus_cap.txbd_num;
}
static void _phl_reset_wp_tag(struct phl_info_t *phl_info,
			struct rtw_wd_page_ring *wd_page_ring, u8 dma_ch)
{
	u16 wp_seq = 0;

	for (wp_seq = 0; wp_seq < WP_MAX_SEQ_NUMBER; wp_seq++) {
		if (NULL != wd_page_ring->wp_tag[wp_seq].ptr)
			phl_recycle_payload(phl_info, dma_ch, wp_seq,
					    TX_STATUS_TX_FAIL_SW_DROP, 3);
	}
}


static void _phl_reinit_wp_tag(struct phl_info_t *phl_info)
{
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_wd_page_ring *wd_ring = (struct rtw_wd_page_ring *)hci_info->wd_ring;
	struct rtw_wd_page_ring *wd_page_ring = NULL;
	u16 wp_seq = 0;
	u8 ch = 0;

	for (ch = 0; ch < hci_info->total_txch_num; ch++) {
		wd_page_ring = &wd_ring[ch];
		for (wp_seq = 0; wp_seq < WP_MAX_SEQ_NUMBER; wp_seq++) {
			if (NULL != wd_page_ring->wp_tag[wp_seq].ptr) {
				PHL_INFO("%s : Addr :%p, wp_seq:%u, dma_ch:%u\n",
							__func__, wd_page_ring->wp_tag[wp_seq].ptr, wp_seq, ch);
				wd_page_ring->wp_tag[wp_seq].ptr = NULL;
			}
		}
	}
}


static enum rtw_phl_status enqueue_pending_h2c_pkt(struct phl_info_t *phl_info,
				struct phl_h2c_pkt_pool *h2c_pkt_pool,
				struct rtw_h2c_pkt *h2c_pkt, u8 pos)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
#if 0
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &wd_page_ring->pending_wd_page_list;

	if (wd_page != NULL) {
		_os_spinlock(drv_priv, &wd_page_ring->pending_lock, _bh, NULL);

		if (_tail == pos)
			list_add_tail(&wd_page->list, list);
		else if (_first == pos)
			list_add(&wd_page->list, list);

		wd_page_ring->pending_wd_page_cnt++;

		_os_spinunlock(drv_priv, &wd_page_ring->pending_lock, _bh, NULL);

		pstatus = RTW_PHL_STATUS_SUCCESS;
	}
#endif
	return pstatus;
}

static struct rtw_h2c_pkt *query_pending_h2c_pkt(struct phl_info_t *phl_info,
				struct phl_h2c_pkt_pool *h2c_pkt_pool)
{
	//void *drv_priv = phl_to_drvpriv(phl_info);
	//_os_list *pending_list = &wd_page_ring->pending_wd_page_list;
	struct rtw_h2c_pkt *h2c_pkt = NULL;
#if 0
	_os_spinlock(drv_priv, &wd_page_ring->pending_lock, _bh, NULL);

	if (true == list_empty(pending_list)) {
		wd_page = NULL;
	} else {
		wd_page = list_first_entry(pending_list, struct rtw_wd_page,
						list);
		wd_page_ring->pending_wd_page_cnt--;
		list_del(&wd_page->list);
	}

	_os_spinunlock(drv_priv, &wd_page_ring->pending_lock, _bh, NULL);
#endif
	return h2c_pkt;
}

static enum rtw_phl_status phl_release_busy_h2c_pkt(
				struct phl_info_t *phl_info,
				struct phl_h2c_pkt_pool *h2c_pkt_pool,
				u16 release_num)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	_os_list *list = &h2c_pkt_pool->busy_h2c_pkt_list.queue;
	struct rtw_h2c_pkt *h2c_pkt = NULL;
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_info->phl_com);
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct tx_base_desc *txbd = NULL;
	u8 ch_idx = rtw_hal_get_fwcmd_queue_idx(phl_info->hal);

	txbd = (struct tx_base_desc *)hci_info->txbd_buf;

	if (h2c_pkt_pool != NULL) {

		while (release_num > 0 && true != list_empty(list)) {
			h2c_pkt = phl_query_busy_h2c_pkt(phl_info);

			if (!h2c_pkt)
				break;

			if (true == hal_spec->txbd_upd_lmt) {
				pstatus = enqueue_h2c_work_ring(phl_info,
								h2c_pkt);
			} else {
				pstatus = phl_enqueue_idle_h2c_pkt(phl_info,
								   h2c_pkt);
			}

			if (RTW_PHL_STATUS_SUCCESS != pstatus)
				break;

			_os_atomic_inc(NULL, &txbd[ch_idx].avail_num);
			//printk("%s, ch=%d, avail_num=%d\n", __func__, ch_idx, _os_atomic_read(NULL, &txbd[ch_idx].avail_num));
			release_num--;
		}
	}
	return pstatus;
}

static void phl_tx_reset_pcie(struct phl_info_t *phl_info)
{
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_wd_page_ring *wd_ring = NULL;
	struct tx_base_desc *txbd = NULL;
	struct phl_h2c_pkt_pool *h2c_pool = NULL;
	u8 ch = 0;

	txbd = (struct tx_base_desc *)hci_info->txbd_buf;
	wd_ring = (struct rtw_wd_page_ring *)hci_info->wd_ring;
	h2c_pool = (struct phl_h2c_pkt_pool *)phl_info->h2c_pool;

	for (ch = 0; ch < hci_info->total_txch_num; ch++) {
		rtw_release_busy_wd_page(phl_info, &wd_ring[ch],
					 ch, wd_ring[ch].busy_wd_page_cnt);
		rtw_release_pending_wd_page(phl_info, &wd_ring[ch],
					 wd_ring[ch].pending_wd_page_cnt);
		_phl_reset_wp_tag(phl_info, &wd_ring[ch], ch);
		_phl_reset_txbd(phl_info, &txbd[ch]);
	}

	phl_release_busy_h2c_pkt(phl_info, h2c_pool,
				 (u16)h2c_pool->busy_h2c_pkt_list.cnt);

	phl_dump_h2c_pool_stats(phl_info->h2c_pool);
}



#ifdef CONFIG_DYNAMIC_RX_BUF
enum rtw_phl_status
_phl_alloc_dynamic_rxbuf_pcie(struct rtw_rx_buf *rx_buf,
                              struct rtw_rx_buf_ring *rx_buf_ring,
			      struct phl_info_t *phl_info)
{
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	/* Dynamic alloc RX buffer should happen on RX channe 0 */
	u32 buf_len = rtw_hal_get_rxbuf_size(phl_info->hal, 0);

	#ifdef CONFIG_WNIC_RECYCLED_SKB
	struct rtw_rx_buf_alloc_data alloc_data = { .drv_priv = drv_priv,
						    .pool_id = rx_buf_ring->pool_idx };
	void *mem_d = &alloc_data;
	#else
	void *mem_d = drv_priv;
	#endif /* CONFIG_WNIC_RECYCLED_SKB */

	#ifdef CONFIG_RTW_RX_BUF_SHARING
	struct phl_rx_buf_info *rx_buf_info;
	#endif

	if (rx_buf == NULL)
		return RTW_PHL_STATUS_FAILURE;

	if (rx_buf->reuse) {
		rx_buf->reuse = false;
		_os_pkt_buf_map_rx(drv_priv,
				&rx_buf->phy_addr_l,
				&rx_buf->phy_addr_h,
				rx_buf->buf_len,
				rx_buf->os_priv);
		return RTW_PHL_STATUS_SUCCESS;
	}

#ifdef CONFIG_RTW_RX_BUF_SHARING
	rx_buf_info = &phl_info->hci->rx_buf_info;
	buf_len *= RTW_RX_BUF_SHARE_FACTOR;

	_os_spinlock(phl_to_drvpriv(phl_info), &phl_info->hci->rx_buf_info_lock, _bh, NULL);
	if (rx_buf_info->rx_os_priv != NULL) {
		rx_buf->os_priv = rx_buf_info->rx_os_priv;
		rx_buf->phy_addr_l = rx_buf_info->rx_phy_addr_l;
		rx_buf->phy_addr_h = rx_buf_info->rx_phy_addr_h;
		rx_buf->buf_len = rx_buf_info->rx_buf_len;
		rx_buf->vir_addr = rx_buf_info->rx_data;
		_os_mem_set(drv_priv, rx_buf_info, 0, sizeof(*rx_buf_info));
	} else {
		rx_buf->vir_addr = _os_pkt_buf_alloc_rx_2(mem_d, buf_len,
							  &rx_buf->phy_addr_l,
							  &rx_buf->phy_addr_h,
							  &rx_buf->os_priv,
							  &rx_buf_info->rx_phy_addr_l,
							  &rx_buf_info->rx_phy_addr_h,
							  &rx_buf_info->rx_os_priv,
							  &rx_buf_info->rx_data);
		if(rx_buf->vir_addr)
			rx_buf_info->rx_buf_len = _os_pkt_get_rx_buf_sz(rx_buf_info->rx_os_priv);
		#if defined(CONFIG_DYNAMIC_RX_BUF) && defined(RTW_RX_CPU_BALANCE)
		phl_info->phl_com->rx_stats.refill_cpu[smp_processor_id()]++;
		#endif
	}
	_os_spinunlock(phl_to_drvpriv(phl_info), &phl_info->hci->rx_buf_info_lock, _bh, NULL);
#else /* CONFIG_RTW_RX_BUF_SHARING */

	rx_buf->vir_addr = _os_pkt_buf_alloc_rx(mem_d,
						&rx_buf->phy_addr_l,
						&rx_buf->phy_addr_h,
						buf_len,
						&rx_buf->os_priv);
	#if defined(CONFIG_DYNAMIC_RX_BUF) && defined(RTW_RX_CPU_BALANCE)
	phl_info->phl_com->rx_stats.refill_cpu[smp_processor_id()]++;
	#endif
#endif /* CONFIG_RTW_RX_BUF_SHARING */

	if (NULL == rx_buf->vir_addr) {
		sts = RTW_PHL_STATUS_RESOURCE;
	} else {
		rx_buf->cache = true;
		rx_buf->buf_len = _os_pkt_get_rx_buf_sz(rx_buf->os_priv);
		rx_buf->dynamic = 1;
		rx_buf->reuse = false;
		/* enqueue_idle_rx_buf(phl_info, rx_buf_ring, rx_buf); */
		sts = RTW_PHL_STATUS_SUCCESS;
	}

	return sts;
}
#endif /* CONFIG_DYNAMIC_RX_BUF */


static enum rtw_phl_status enqueue_busy_rx_buf(
				struct phl_info_t *phl_info,
				struct rtw_rx_buf_ring *rx_buf_ring,
				struct rtw_rx_buf *rx_buf, u8 pos)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	_os_list *list = &rx_buf_ring->busy_rxbuf_list;

	if (rx_buf != NULL) {
		_os_spinlock(phl_to_drvpriv(phl_info),
				&rx_buf_ring->busy_rxbuf_lock, _bh, NULL);
		if (_tail == pos)
			list_add_tail(&rx_buf->list, list);
		else if (_first == pos)
			list_add(&rx_buf->list, list);

		rx_buf_ring->busy_rxbuf_cnt++;
		_os_spinunlock(phl_to_drvpriv(phl_info),
				&rx_buf_ring->busy_rxbuf_lock, _bh, NULL);
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}

static void enqueue_empty_rx_buf(
				struct phl_info_t *phl_info,
				struct rtw_rx_buf_ring *rx_buf_ring,
				struct rtw_rx_buf *rx_buf)
{
	_os_list *list = &rx_buf_ring->empty_rxbuf_list;

	_os_spinlock(phl_to_drvpriv(phl_info),
			&rx_buf_ring->empty_rxbuf_lock, _bh, NULL);
	list_add_tail(&rx_buf->list, list);
	rx_buf_ring->empty_rxbuf_cnt++;
	_os_spinunlock(phl_to_drvpriv(phl_info),
			&rx_buf_ring->empty_rxbuf_lock, _bh, NULL);
}

static struct rtw_rx_buf *query_empty_rx_buf(struct phl_info_t *phl_info,
				struct rtw_rx_buf_ring *rx_buf_ring)
{
	_os_list *rxbuf_list = &rx_buf_ring->empty_rxbuf_list;
	struct rtw_rx_buf *rx_buf = NULL;

	_os_spinlock(phl_to_drvpriv(phl_info),
			&rx_buf_ring->empty_rxbuf_lock, _bh, NULL);
	if (!list_empty(rxbuf_list)) {
		rx_buf = list_first_entry(rxbuf_list, struct rtw_rx_buf, list);
		rx_buf_ring->empty_rxbuf_cnt--;
		list_del(&rx_buf->list);
	}
	_os_spinunlock(phl_to_drvpriv(phl_info),
			&rx_buf_ring->empty_rxbuf_lock, _bh, NULL);

	return rx_buf;
}

static enum rtw_phl_status enqueue_idle_rx_buf(
				struct phl_info_t *phl_info,
				struct rtw_rx_buf_ring *rx_buf_ring,
				struct rtw_rx_buf *rx_buf)
{
	_os_list *list;
	u32 clr_len;
	void *drvpriv;
	void *bdinfo;

	if (rx_buf == NULL)
		return RTW_PHL_STATUS_FAILURE;

	list = &rx_buf_ring->idle_rxbuf_list;
	drvpriv = phl_to_drvpriv(phl_info);

	/* Clear RX buffer before enqueue to idle Q */
	#if !defined(PHL_DMA_NONCOHERENT) && !defined(CONFIG_RTK_SOC)
	clr_len = rx_buf->buf_len;
	#else /* PHL_DMA_NONCOHERENT */
	/* Clear RXBD info only to minimize cost to clear RX buffer. */
	clr_len = phl_get_ic_spec(phl_info->phl_com)->rx_bd_info_sz;
	#endif /* PHL_DMA_NONCOHERENT */

	#ifdef HAL_TO_NONCACHE_ADDR
	bdinfo = (void *)HAL_TO_NONCACHE_ADDR(rx_buf->vir_addr);
	// bdinfo = rx_buf->vir_addr;
	#else
	bdinfo = rx_buf->vir_addr;
	#endif /* HAL_TO_NONCACHE_ADDR */

	_os_mem_set(drvpriv, bdinfo, 0, clr_len);

	#if defined(PHL_DMA_NONCOHERENT)
	#if !defined(HAL_TO_NONCACHE_ADDR)
	/* Bidirectional $ WB to clear RX buf and invalidate $ */
	if (rx_buf->cache) {
		#ifdef CONFIG_RTW_VM_CACHE_HANDLING
		_os_vm_cache_wback(drvpriv, rx_buf->vir_addr, clr_len,
		                   PCI_DMA_BIDIRECTIONAL);
		#else /* CONFIG_RTW_VM_CACHE_HANDLING */
		_os_cache_wback(drvpriv,
				&rx_buf->phy_addr_l,
				&rx_buf->phy_addr_h,
				clr_len, PCI_DMA_BIDIRECTIONAL);
		#endif /* CONFIG_RTW_VM_CACHE_HANDLING */
	}
	#else /* HAL_TO_NONCACHE_ADDR */
	#ifdef CONFIG_RTW_PLATFORM_NEED_MEMORY_FLUSH
	iob();
	#endif /* CONFIG_RTW_PLATFORM_NEED_MEMORY_FLUSH */
	#endif /* HAL_TO_NONCACHE_ADDR */
	#endif /* PHL_DMA_NONCOHERENT */

	#ifdef PHL_INV_CACHE_AT_RECYCLE
	rx_buf->dma_len = 0;
	#endif /* PHL_INV_CACHE_AT_RECYCLE */

	/* Enqueue to idle Q */
	INIT_LIST_HEAD(&rx_buf->list);

	_os_spinlock(drvpriv, &rx_buf_ring->idle_rxbuf_lock, _bh, NULL);
	list_add_tail(&rx_buf->list, list);
	rx_buf_ring->idle_rxbuf_cnt++;
	_os_spinunlock(drvpriv, &rx_buf_ring->idle_rxbuf_lock, _bh, NULL);

	return RTW_PHL_STATUS_SUCCESS;
}

static void refill_empty_rx_buf(struct phl_info_t *phl_info,
				struct rtw_rx_buf_ring *rx_buf_ring)
{
	enum rtw_phl_status pstatus;
	struct rtw_rx_buf *rx_buf = NULL;

	while (rx_buf_ring->empty_rxbuf_cnt) {
		rx_buf = query_empty_rx_buf(phl_info, rx_buf_ring);
		pstatus = _phl_alloc_dynamic_rxbuf_pcie(rx_buf, rx_buf_ring, phl_info);
		if (RTW_PHL_STATUS_SUCCESS != pstatus) {
			enqueue_empty_rx_buf(phl_info, rx_buf_ring, rx_buf);
			break;
		}
		enqueue_idle_rx_buf(phl_info, rx_buf_ring, rx_buf);
	}
}

static struct rtw_rx_buf *query_busy_rx_buf(struct phl_info_t *phl_info,
					struct rtw_rx_buf_ring *rx_buf_ring)
{
	_os_list *busy_list = &rx_buf_ring->busy_rxbuf_list;
	struct rtw_rx_buf *rx_buf = NULL;

	_os_spinlock(phl_to_drvpriv(phl_info),
			&rx_buf_ring->busy_rxbuf_lock, _bh, NULL);
	if (true == list_empty(busy_list)) {
		rx_buf = NULL;
	} else {
		rx_buf = list_first_entry(busy_list, struct rtw_rx_buf, list);
		rx_buf_ring->busy_rxbuf_cnt--;
		list_del_init(&rx_buf->list);
	}
	_os_spinunlock(phl_to_drvpriv(phl_info),
			&rx_buf_ring->busy_rxbuf_lock, _bh, NULL);
	return rx_buf;
}

static struct rtw_rx_buf *query_idle_rx_buf(struct phl_info_t *phl_info,
					struct rtw_rx_buf_ring *rx_buf_ring)
{
	_os_list *idle_list = &rx_buf_ring->idle_rxbuf_list;
	struct rtw_rx_buf *rx_buf = NULL;

	_os_spinlock(phl_to_drvpriv(phl_info),
			&rx_buf_ring->idle_rxbuf_lock, _bh, NULL);
	if (true == list_empty(idle_list)) {
		rx_buf = NULL;
	} else {
		rx_buf = list_first_entry(idle_list, struct rtw_rx_buf, list);
		rx_buf_ring->idle_rxbuf_cnt--;
		list_del(&rx_buf->list);
	}
	_os_spinunlock(phl_to_drvpriv(phl_info),
			&rx_buf_ring->idle_rxbuf_lock, _bh, NULL);

	return rx_buf;
}

enum rtw_phl_status
phl_release_target_rx_buf(struct phl_info_t *phl_info, void *r, u8 ch,
				enum rtw_rx_type type)
{
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_rx_buf_ring *rx_buf_ring;
	struct rtw_rx_buf *rx_buf = (struct rtw_rx_buf *)r;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	rx_buf_ring = (struct rtw_rx_buf_ring *)hci_info->rxbuf_pool;
	rx_buf_ring = &rx_buf_ring[ch];

	if (rx_buf) {
#ifdef CONFIG_DYNAMIC_RX_BUF
		if (type == RTW_RX_TYPE_WIFI)
			_phl_alloc_dynamic_rxbuf_pcie(rx_buf, rx_buf_ring, phl_info);
		else {
			#ifdef PHL_INV_CACHE_AT_RECYCLE
			/* Invalidate used RX buffer before reusing it. */
			phl_sync_rx_buf(phl_com->drv_priv, rx_buf,
			                (  rx_buf->dma_len
			              	 ? rx_buf->dma_len
			              	 : rx_buf->buf_len);
			#endif /* PHL_INV_CACHE_AT_RECYCLE */
		}

		if (NULL == rx_buf->vir_addr) {
			enqueue_empty_rx_buf(phl_info, rx_buf_ring, rx_buf);
#ifdef DEBUG_PHL_RX
			phl_info->phl_com->rx_stats.rxbuf_empty++;
#endif
		} else
#endif
		enqueue_idle_rx_buf(phl_info, rx_buf_ring, rx_buf);
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}

static enum rtw_phl_status phl_release_busy_rx_buf(
				struct phl_info_t *phl_info,
				struct rtw_rx_buf_ring *rx_buf_ring,
				u16 release_num)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_rx_buf *rx_buf = NULL;

	if (rx_buf_ring != NULL) {

		while (release_num > 0) {
			rx_buf = query_busy_rx_buf(phl_info, rx_buf_ring);
			if (NULL == rx_buf)
				break;
			enqueue_idle_rx_buf(phl_info, rx_buf_ring, rx_buf);
			release_num--;
		}
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}
	return pstatus;
}



/* static void rtl8852ae_free_wd_page_buf(_adapter *adapter, void *vir_addr, */
/* 				dma_addr_t *bus_addr, size_t size) */
/* { */
/* 	struct platform_ops *ops = &adapter->platform_func; */
/* 	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter); */
/* 	struct pci_dev *pdev = dvobj->ppcidev; */

/* 	FUNCIN(); */
/* 	ops->free_cache_mem(pdev,vir_addr, bus_addr, size, PCI_DMA_TODEVICE); */

/* 	/\* NONCACHE hana_todo */
/* 	 * ops->alloc_noncache_mem(pdev, vir_addr, bus_addr, size); */
/* 	 *\/ */
/* 	FUNCOUT(); */
/* } */
static void _phl_free_rxbuf_pcie(struct phl_info_t *phl_info,
				 struct rtw_rx_buf_ring *rx_buf_ring, u8 ch_idx)
{
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	u16 rxbuf_num = rtw_hal_get_rxbuf_num(phl_info->hal, ch_idx);
	u16 i = 0;
	struct rtw_rx_buf *rx_buf = rx_buf_ring->rx_buf;

	if (NULL != rx_buf) {
		void *drv_priv = phl_to_drvpriv(phl_info);

		#if defined(CONFIG_RTW_RX_BUF_SHARING)
		struct phl_rx_buf_info *rx_buf_info = &phl_info->hci->rx_buf_info;

                _os_spinlock(phl_to_drvpriv(phl_info),
                        &phl_info->hci->rx_buf_info_lock, _bh, NULL);

		if (rx_buf_info->rx_os_priv) {
			_os_pkt_buf_free_rx(drv_priv,
					    rx_buf_info->rx_data,
					    rx_buf_info->rx_phy_addr_l,
					    rx_buf_info->rx_phy_addr_h,
					    rx_buf_info->rx_buf_len,
					    rx_buf_info->rx_os_priv);
			_os_mem_set(drv_priv, rx_buf_info, 0, sizeof(*rx_buf_info));
		}

                _os_spinunlock(phl_to_drvpriv(phl_info),
                        &phl_info->hci->rx_buf_info_lock, _bh, NULL);

		#endif /* CONFIG_RTW_RX_BUF_SHARING */

		for (i = 0; i < rxbuf_num; i++) {

			if (NULL == rx_buf[i].vir_addr)
				continue;
			_os_pkt_buf_free_rx(drv_priv,
					    rx_buf[i].vir_addr,
					    rx_buf[i].phy_addr_l,
					    rx_buf[i].phy_addr_h,
					    rx_buf[i].buf_len,
					    rx_buf[i].os_priv);
			rx_buf[i].vir_addr = NULL;
			rx_buf[i].cache = 0;
		}

		_os_mem_free(phl_to_drvpriv(phl_info), rx_buf,
					sizeof(struct rtw_rx_buf) * rxbuf_num);
	}
#ifdef CONFIG_WNIC_RECYCLED_SKB
	if (rx_buf_ring->pool_idx > 0) {
		deinit_recycle_wnic_skb_buf(rx_buf_ring->pool_idx);
		rx_buf_ring->pool_idx = 0;
	}
#endif
}

static void _phl_free_rxbuf_pool_pcie(struct phl_info_t *phl_info,
						u8 *rxbuf_pool, u8 ch_num)
{
	struct rtw_rx_buf_ring *ring = NULL;
	void *drv_priv = phl_to_drvpriv(phl_info);
	u8 i = 0;

	FUNCIN();
	ring = (struct rtw_rx_buf_ring *)rxbuf_pool;
	if (NULL != ring) {
		for (i = 0; i < ch_num; i++, ring++) {
			ring->idle_rxbuf_cnt = 0;
#ifdef CONFIG_DYNAMIC_RX_BUF
			ring->empty_rxbuf_cnt = 0;
#endif

			if (NULL == ring->rx_buf)
				continue;

			_phl_free_rxbuf_pcie(phl_info, ring, i);
			ring->rx_buf = NULL;
			_os_spinlock_free(drv_priv,
			                  &ring->idle_rxbuf_lock);
			_os_spinlock_free(drv_priv,
			                  &ring->busy_rxbuf_lock);
#ifdef CONFIG_DYNAMIC_RX_BUF
			_os_spinlock_free(drv_priv,
			                  &ring->empty_rxbuf_lock);
#endif
		}
		_os_mem_free(drv_priv, rxbuf_pool,
		             sizeof(struct rtw_rx_buf_ring) * ch_num);
	}

	FUNCOUT();
}

/* static void *rtl8852ae_alloc_wd_page_buf(_adapter *adapter, */
/* 					 dma_addr_t *bus_addr, size_t size) */
/* { */
/* 	struct platform_ops *ops = &adapter->platform_func; */
/* 	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter); */
/* 	struct pci_dev *pdev = dvobj->ppcidev; */
/* 	void *vir_addr = NULL; */

/* 	FUNCIN(); */
/* 	vir_addr = ops->alloc_cache_mem(pdev, bus_addr, size, PCI_DMA_TODEVICE); */

/* 	/\* NONCACHE hana_todo */
/* 	 * vir_addr = ops->alloc_noncache_mem(pdev, bus_addr, size); */
/* 	 *\/ */

/* 	FUNCOUT(); */
/* 	return vir_addr; */
/* } */
static enum rtw_phl_status
_phl_alloc_rxbuf_pcie(struct phl_info_t *phl_info,
		      struct rtw_rx_buf_ring *rx_buf_ring, u8 ch_idx, u32 rx_buf_size)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	struct rtw_rx_buf *rx_buf = NULL;
	u32 buf_len = 0;
	u16 rxbuf_num = rtw_hal_get_rxbuf_num(phl_info->hal, ch_idx);
	void *drv_priv = phl_to_drvpriv(phl_info);
	int i = 0;
	#ifdef CONFIG_WNIC_RECYCLED_SKB
	struct rtw_rx_buf_alloc_data alloc_data = { .drv_priv = drv_priv,
						    .pool_id = rx_buf_ring->pool_idx };
	void *mem_d = &alloc_data;
	#else
	void *mem_d = drv_priv;
	#endif /* CONFIG_WNIC_RECYCLED_SKB */

	buf_len = sizeof(*rx_buf) * rxbuf_num;
	rx_buf = _os_mem_alloc(drv_priv, buf_len);
	if (rx_buf != NULL) {
#ifdef CONFIG_RTW_RX_BUF_SHARING
		u16 _max_rx_buf_num = rxbuf_num & ~(0x01);
		buf_len = rx_buf_size * RTW_RX_BUF_SHARE_FACTOR;

		for (; i < _max_rx_buf_num; i += 2) {
			struct rtw_rx_buf *rx_buf_1 = &rx_buf[i];
			struct rtw_rx_buf *rx_buf_2 = &rx_buf[i + 1];
			rx_buf_1->cache = true;
			rx_buf_2->cache = true;
			rx_buf_1->vir_addr = _os_pkt_buf_alloc_rx_2(
					mem_d, buf_len,
					&rx_buf_1->phy_addr_l,
					&rx_buf_1->phy_addr_h,
					&rx_buf_1->os_priv,
					&rx_buf_2->phy_addr_l,
					&rx_buf_2->phy_addr_h,
					&rx_buf_2->os_priv,
					(void **)&rx_buf_2->vir_addr);
			if (NULL == rx_buf_1->vir_addr) {
				pstatus = RTW_PHL_STATUS_RESOURCE;
				break;
			}
			rx_buf_1->buf_len = _os_pkt_get_rx_buf_sz(rx_buf_1->os_priv);
			rx_buf_1->dynamic = 0;
			rx_buf_2->buf_len = _os_pkt_get_rx_buf_sz(rx_buf_2->os_priv);
			rx_buf_2->dynamic = 0;
#ifdef CONFIG_DYNAMIC_RX_BUF
			rx_buf_1->reuse = false;
			rx_buf_2->reuse = false;
#endif /* CONFIG_DYNAMIC_RX_BUF */

			INIT_LIST_HEAD(&rx_buf_1->list);
			enqueue_idle_rx_buf(phl_info, rx_buf_ring, rx_buf_1);
			INIT_LIST_HEAD(&rx_buf_2->list);
			enqueue_idle_rx_buf(phl_info, rx_buf_ring, rx_buf_2);
			pstatus = RTW_PHL_STATUS_SUCCESS;
				/* hana_todo now check 4 byte align only */
			/* if ((unsigned long)wd_page_buf & 0xF) { */
			/* 	res = _FAIL; */
			/* 	break; */
			/* } */
		}
#else /* CONFIG_RTW_RX_BUF_SHARING */
		buf_len = rx_buf_size;
#endif /* CONFIG_RTW_RX_BUF_SHARING */
		for (; i < rxbuf_num; i++) {
			rx_buf[i].cache = true;
			rx_buf[i].vir_addr = _os_pkt_buf_alloc_rx(
					mem_d,
					&rx_buf[i].phy_addr_l,
					&rx_buf[i].phy_addr_h,
					buf_len,
					&rx_buf[i].os_priv);
			if (NULL == rx_buf[i].vir_addr) {
				pstatus = RTW_PHL_STATUS_RESOURCE;
				break;
			}
			rx_buf[i].buf_len = _os_pkt_get_rx_buf_sz(rx_buf[i].os_priv);
			rx_buf[i].dynamic = 0;
#ifdef CONFIG_DYNAMIC_RX_BUF
			rx_buf[i].reuse = false;
#endif

			INIT_LIST_HEAD(&rx_buf[i].list);
			enqueue_idle_rx_buf(phl_info, rx_buf_ring, &rx_buf[i]);
			pstatus = RTW_PHL_STATUS_SUCCESS;
				/* hana_todo now check 4 byte align only */
			/* if ((unsigned long)wd_page_buf & 0xF) { */
			/* 	res = _FAIL; */
			/* 	break; */
			/* } */
		}
	} else {
		pstatus = RTW_PHL_STATUS_RESOURCE;
	}

	rx_buf_ring->rx_buf = rx_buf;
	rx_buf_ring->idle_rxbuf_cnt = rxbuf_num;
	rx_buf_ring->busy_rxbuf_cnt = 0;

	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		_phl_free_rxbuf_pcie(phl_info, rx_buf_ring, ch_idx);
	} else {
		PHL_PRINT("CH[%d] Alloc %uB RX buffer, got available size %uB\n", ch_idx, rx_buf_size, rx_buf[0].buf_len);
	}
	return pstatus;
}

static enum rtw_phl_status
_phl_alloc_rxbuf_pool_pcie(struct phl_info_t *phl_info, u8 ch_num)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	struct rtw_rx_buf_ring *rx_buf_ring = NULL;
	struct rtw_rx_buf *rx_buf = NULL;
	u32 buf_len = 0;
	u16 rxbuf_num = 0;
	u8 i = 0;
	#ifdef CONFIG_WNIC_RECYCLED_SKB
	int pool_idx;
	u32 pkt_num_in_pool;
	u32 pkt_sz_in_pool;
	#endif

	FUNCIN_WSTS(pstatus);

	buf_len = sizeof(*rx_buf_ring) * ch_num;
	rx_buf_ring = _os_mem_alloc(phl_to_drvpriv(phl_info), buf_len);

	if (NULL != rx_buf_ring) {
		for (i = 0; i < ch_num; i++) {
			_os_spinlock_init(phl_to_drvpriv(phl_info),
					&rx_buf_ring[i].idle_rxbuf_lock);
			_os_spinlock_init(phl_to_drvpriv(phl_info),
					&rx_buf_ring[i].busy_rxbuf_lock);
			INIT_LIST_HEAD(&rx_buf_ring[i].idle_rxbuf_list);
			INIT_LIST_HEAD(&rx_buf_ring[i].busy_rxbuf_list);
#ifdef CONFIG_DYNAMIC_RX_BUF
			_os_spinlock_init(phl_to_drvpriv(phl_info),
					&rx_buf_ring[i].empty_rxbuf_lock);
			INIT_LIST_HEAD(&rx_buf_ring[i].empty_rxbuf_list);
#endif
			rxbuf_num = rtw_hal_get_rxbuf_num(phl_info->hal, i);
			buf_len = rtw_hal_get_rxbuf_size(phl_info->hal, i);

			#ifdef CONFIG_WNIC_RECYCLED_SKB
			/* 1/2 extra packet in pool for RX process delay above driver */
			pkt_num_in_pool = (  (i == 0)
				           ? ((rxbuf_num * 3) / 2)
				           : rxbuf_num);
			pkt_sz_in_pool = buf_len;

			#ifdef CONFIG_RTW_RX_BUF_SHARING
			pkt_num_in_pool /= RTW_RX_BUF_SHARE_FACTOR;
			pkt_sz_in_pool *= RTW_RX_BUF_SHARE_FACTOR;
			#endif /* CONFIG_RTW_RX_BUF_SHARING */

			pool_idx = init_recycle_wnic_skb_buf(pkt_num_in_pool,
			                                     pkt_sz_in_pool);

			if (pool_idx > 0) {
				PHL_PRINT("%u: SKB pool %u created for channel %u of %uB with #%u (%u).\n",
				          phl_info->dev_id, pool_idx, i, pkt_sz_in_pool,
				          pkt_num_in_pool, rxbuf_num);
			} else {
				PHL_ERR("%u: SKB pool %u created failed for channel %u of %uB with #%u (%u).\n",
					phl_info->dev_id, pool_idx, i, pkt_sz_in_pool,
					pkt_num_in_pool, rxbuf_num);
			}
			rx_buf_ring[i].pool_idx = pool_idx;
			#endif /* CONFIG_WNIC_RECYCLED_SKB */

			RTW_PRINT("[RXBUF][dev_id:%d][ch:%d] rx_buf: num=%d size=%d\n",
				phl_info->dev_id, i, rxbuf_num, buf_len);

			pstatus = _phl_alloc_rxbuf_pcie(phl_info,
							&rx_buf_ring[i], i, buf_len);
			if (pstatus != RTW_PHL_STATUS_SUCCESS)
				break;
		}
	}

	if (RTW_PHL_STATUS_SUCCESS == pstatus) {
		phl_info->hci->rxbuf_pool = (u8 *)rx_buf_ring;
	} else
		_phl_free_rxbuf_pool_pcie(phl_info, (u8 *)rx_buf_ring, ch_num);
	FUNCOUT_WSTS(pstatus);

	return pstatus;
}




/* static void rtl8852ae_free_wd_page_buf(_adapter *adapter, void *vir_addr, */
/* 				dma_addr_t *bus_addr, size_t size) */
/* { */
/* 	struct platform_ops *ops = &adapter->platform_func; */
/* 	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter); */
/* 	struct pci_dev *pdev = dvobj->ppcidev; */

/* 	FUNCIN(); */
/* 	ops->free_cache_mem(pdev,vir_addr, bus_addr, size, PCI_DMA_TODEVICE); */

/* 	/\* NONCACHE hana_todo */
/* 	 * ops->alloc_noncache_mem(pdev, vir_addr, bus_addr, size); */
/* 	 *\/ */
/* 	FUNCOUT(); */
/* } */

static void _phl_free_wd_page_pcie(struct phl_info_t *phl_info,
					struct rtw_wd_page *wd_page)
{
	u16 i = 0;

	if (NULL != wd_page) {
		if (NULL != wd_page[0].vir_addr) {
			_os_shmem_free(phl_to_drvpriv(phl_info),
						wd_page[0].vir_addr,
						&wd_page[0].phy_addr_l,
						&wd_page[0].phy_addr_h,
						(MAX_WD_PAGE_NUM * WD_PAGE_SIZE),
						wd_page[0].cache,
						PCI_DMA_FROMDEVICE,
						wd_page[0].os_rsvd[0]);
		}

		for (i = 0; i < MAX_WD_PAGE_NUM; i++) {
			wd_page[i].vir_addr = NULL;
			wd_page[i].cache = 0;
			wd_page[i].wp_seq = WP_RESERVED_SEQ;
		}

		_os_mem_free(phl_to_drvpriv(phl_info), wd_page,
					sizeof(struct rtw_wd_page) * MAX_WD_PAGE_NUM);
	}
}

static void _phl_free_wd_ring_pcie(struct phl_info_t *phl_info, u8 *wd_page_buf,
					u8 ch_num)
{
	struct rtw_wd_page_ring *wd_page_ring = NULL;
	void *drv_priv = phl_to_drvpriv(phl_info);
	u8 i = 0;
	FUNCIN();

	wd_page_ring = (struct rtw_wd_page_ring *)wd_page_buf;
	if (NULL != wd_page_ring) {
		for (i = 0; i < ch_num; i++) {

			wd_page_ring[i].idle_wd_page_cnt = 0;

			if (NULL == wd_page_ring[i].wd_page)
				continue;

			if (i == rtw_hal_get_fwcmd_queue_idx(phl_info->hal)) {
				_phl_free_h2c_work_ring(phl_info,
							&wd_page_ring[i]);
			}
			_phl_free_wd_work_ring(phl_info, &wd_page_ring[i]);
			_phl_free_wd_page_pcie(phl_info,
						wd_page_ring[i].wd_page);
			wd_page_ring[i].wd_page = NULL;
			_os_spinlock_free(drv_priv,
						&wd_page_ring[i].idle_lock);
			_os_spinlock_free(drv_priv,
						&wd_page_ring[i].busy_lock);
			_os_spinlock_free(drv_priv,
						&wd_page_ring[i].pending_lock);
			_os_spinlock_free(drv_priv,
						&wd_page_ring[i].work_lock);
			_os_spinlock_free(drv_priv,
						&wd_page_ring[i].wp_tag_lock);
		}
		_os_mem_free(phl_to_drvpriv(phl_info), wd_page_ring,
						sizeof(struct rtw_wd_page_ring) * ch_num);
	}
	FUNCOUT();
}

/* static void *rtl8852ae_alloc_wd_page_buf(_adapter *adapter, */
/* 					 dma_addr_t *bus_addr, size_t size) */
/* { */
/* 	struct platform_ops *ops = &adapter->platform_func; */
/* 	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter); */
/* 	struct pci_dev *pdev = dvobj->ppcidev; */
/* 	void *vir_addr = NULL; */

/* 	FUNCIN(); */
/* 	vir_addr = ops->alloc_cache_mem(pdev, bus_addr, size, PCI_DMA_TODEVICE); */

/* 	/\* NONCACHE hana_todo */
/* 	 * vir_addr = ops->alloc_noncache_mem(pdev, bus_addr, size); */
/* 	 *\/ */

/* 	FUNCOUT(); */
/* 	return vir_addr; */
/* } */

static struct rtw_wd_page *_phl_alloc_wd_page_pcie(
			struct phl_info_t *phl_info, _os_list *list)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_wd_page *wd_page = NULL;
	u32 buf_len = 0;
	int i;
	u8 cache = true;

	buf_len = sizeof(*wd_page) * MAX_WD_PAGE_NUM;
	wd_page = _os_mem_alloc(phl_to_drvpriv(phl_info), buf_len);
	if (wd_page != NULL) {
		wd_page[0].vir_addr = _os_shmem_alloc(
							phl_to_drvpriv(phl_info),
							&wd_page[0].phy_addr_l,
							&wd_page[0].phy_addr_h,
							(MAX_WD_PAGE_NUM * WD_PAGE_SIZE),
							cache,
							PCI_DMA_TODEVICE,
							&wd_page[0].os_rsvd[0]);

		if (NULL == wd_page[0].vir_addr) {
			pstatus = RTW_PHL_STATUS_RESOURCE;
			goto vir_addr_fail;
		}

		buf_len = WD_PAGE_SIZE;
		for (i = 0; i < MAX_WD_PAGE_NUM; i++) {
			wd_page[i].cache = cache;
			wd_page[i].vir_addr = wd_page[0].vir_addr + buf_len*i;
			wd_page[i].phy_addr_l = wd_page[0].phy_addr_l + buf_len*i;
			wd_page[i].phy_addr_h = wd_page[0].phy_addr_h;
			wd_page[i].buf_len = buf_len;
			wd_page[i].wp_seq = WP_RESERVED_SEQ;

			INIT_LIST_HEAD(&wd_page[i].list);

			list_add_tail(&wd_page[i].list, list);
				/* hana_todo now check 4 byte align only */
			/* if ((unsigned long)wd_page_buf & 0xF) { */
			/* 	res = _FAIL; */
			/* 	break; */
			/* } */
		}
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

vir_addr_fail:
	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		_phl_free_wd_page_pcie(phl_info, wd_page);
		wd_page = NULL;
	}

	return wd_page;
}



static enum rtw_phl_status
_phl_alloc_wd_ring_pcie(struct phl_info_t *phl_info, u8 ch_num)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_wd_page_ring *wd_page_ring = NULL;
	struct rtw_wd_page *wd_page = NULL;
	void *drv_priv = NULL;
	u32 buf_len = 0;
	int i;

	FUNCIN_WSTS(pstatus);
	drv_priv = phl_to_drvpriv(phl_info);
	buf_len = sizeof(struct rtw_wd_page_ring) * ch_num;
	wd_page_ring = _os_mem_alloc(phl_to_drvpriv(phl_info), buf_len);
	if (NULL != wd_page_ring) {
		for (i = 0; i < ch_num; i++) {
			if (!IS_DMA_CH_USED(i))
				continue;

			INIT_LIST_HEAD(&wd_page_ring[i].idle_wd_page_list);
			INIT_LIST_HEAD(&wd_page_ring[i].busy_wd_page_list);
			INIT_LIST_HEAD(&wd_page_ring[i].pending_wd_page_list);
			_os_spinlock_init(drv_priv,
						&wd_page_ring[i].idle_lock);
			_os_spinlock_init(drv_priv,
						&wd_page_ring[i].busy_lock);
			_os_spinlock_init(drv_priv,
						&wd_page_ring[i].pending_lock);
			_os_spinlock_init(drv_priv,
						&wd_page_ring[i].work_lock);
			_os_spinlock_init(drv_priv,
						&wd_page_ring[i].wp_tag_lock);

			wd_page = _phl_alloc_wd_page_pcie(phl_info,
					&wd_page_ring[i].idle_wd_page_list);
			if (NULL == wd_page) {
				pstatus = RTW_PHL_STATUS_RESOURCE;
				break;
			}

			pstatus = _phl_alloc_wd_work_ring(phl_info,
							  &wd_page_ring[i]);
			if (RTW_PHL_STATUS_SUCCESS != pstatus)
				break;

			if (i == rtw_hal_get_fwcmd_queue_idx(phl_info->hal)) {
				pstatus = _phl_alloc_h2c_work_ring(phl_info,
							     &wd_page_ring[i]);
				if (RTW_PHL_STATUS_SUCCESS != pstatus)
					break;
			}
			wd_page_ring[i].wd_page = wd_page;
			wd_page_ring[i].idle_wd_page_cnt = MAX_WD_PAGE_NUM;
			wd_page_ring[i].busy_wd_page_cnt = 0;
			wd_page_ring[i].pending_wd_page_cnt = 0;
			wd_page_ring[i].wp_seq = 1;
			pstatus = RTW_PHL_STATUS_SUCCESS;
		}
	}

	if (RTW_PHL_STATUS_SUCCESS == pstatus) {
		phl_info->hci->wd_ring = (u8 *)wd_page_ring;
	} else
		_phl_free_wd_ring_pcie(phl_info, (u8 *)wd_page_ring, ch_num);
	FUNCOUT_WSTS(pstatus);

	return pstatus;
}

static void _phl_free_h2c_pkt_buf_pcie(struct phl_info_t *phl_info,
				struct rtw_h2c_pkt *_h2c_pkt)
{
	struct rtw_h2c_pkt *h2c_pkt = _h2c_pkt;

	_os_shmem_free(phl_to_drvpriv(phl_info),
				h2c_pkt->vir_head,
				&h2c_pkt->phy_addr_l,
				&h2c_pkt->phy_addr_h,
				h2c_pkt->buf_len,
				h2c_pkt->cache,
				PCI_DMA_FROMDEVICE,
				h2c_pkt->os_rsvd[0]);
}

enum rtw_phl_status _phl_alloc_h2c_pkt_buf_pcie(struct phl_info_t *phl_info,
	struct rtw_h2c_pkt *_h2c_pkt, u32 buf_len)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_h2c_pkt *h2c_pkt = _h2c_pkt;

	h2c_pkt->vir_head = _os_shmem_alloc(
				phl_to_drvpriv(phl_info),
				&h2c_pkt->phy_addr_l,
				&h2c_pkt->phy_addr_h,
				buf_len,
				h2c_pkt->cache,
				PCI_DMA_TODEVICE,
				&h2c_pkt->os_rsvd[0]);

	if (h2c_pkt->vir_head)
		pstatus = RTW_PHL_STATUS_SUCCESS;

	return pstatus;
}

static void _phl_free_rxbd_pcie(struct phl_info_t *phl_info,
						u8 *rxbd_buf, u8 ch_num)
{
	struct rx_base_desc *rxbd = (struct rx_base_desc *)rxbd_buf;
	u8 i = 0;

	FUNCIN();

	if (NULL != rxbd) {
		for (i = 0; i < ch_num; i++) {

			if (NULL == rxbd[i].vir_addr)
				continue;
			_os_shmem_free(phl_to_drvpriv(phl_info),
						rxbd[i].vir_addr,
						&rxbd[i].phy_addr_l,
						&rxbd[i].phy_addr_h,
						rxbd[i].buf_len,
						rxbd[i].cache,
						PCI_DMA_FROMDEVICE,
						rxbd[i].os_rsvd[0]);
			rxbd[i].vir_addr = NULL;
			rxbd[i].cache = 0;
		}

		_os_mem_free(phl_to_drvpriv(phl_info), rxbd,
					sizeof(struct rx_base_desc) * ch_num);
	}
	FUNCOUT();
}


static enum rtw_phl_status
_phl_alloc_rxbd_pcie(struct phl_info_t *phl_info, u8 ch_num)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	struct rx_base_desc *rxbd = NULL;
	u32 buf_len = 0;
	u16 rxbd_num = 0;
	u8 addr_info_size = hal_com->bus_hw_cap.rxbd_len;
	u8 i = 0;

	FUNCIN_WSTS(pstatus);

	buf_len = sizeof(struct rx_base_desc) * ch_num;
	rxbd = _os_mem_alloc(phl_to_drvpriv(phl_info), buf_len);
	if (NULL != rxbd) {
		for (i = 0; i < ch_num; i++) {
			rxbd_num = rtw_hal_get_rxbd_num(phl_info->hal, i);
			rxbd[i].cache = false;
			buf_len = addr_info_size * rxbd_num;

			RTW_PRINT("[RXBUF][dev_id:%d][ch:%d] rxbd_num:%d buf_len:%d\n",
				phl_info->dev_id, i, rxbd_num, buf_len);

			rxbd[i].vir_addr = _os_shmem_alloc(
						phl_to_drvpriv(phl_info),
						&rxbd[i].phy_addr_l,
						&rxbd[i].phy_addr_h,
						buf_len,
						rxbd[i].cache,
						PCI_DMA_TODEVICE,
						&rxbd[i].os_rsvd[0]);
			if (NULL == rxbd[i].vir_addr) {
				pstatus = RTW_PHL_STATUS_RESOURCE;
				break;
			}
			rxbd[i].buf_len = buf_len;
			rxbd[i].host_idx = 0;
			rxbd[i].avail_num = rxbd_num;
			pstatus = RTW_PHL_STATUS_SUCCESS;
		}
	}

	if (RTW_PHL_STATUS_SUCCESS == pstatus)
		phl_info->hci->rxbd_buf = (u8 *)rxbd;
	else
		_phl_free_rxbd_pcie(phl_info, (u8 *)rxbd, ch_num);
	FUNCOUT_WSTS(pstatus);

	return pstatus;
}


static void _phl_free_txbd_pcie(struct phl_info_t *phl_info, u8 *txbd_buf,
				u8 ch_num)
{
	struct tx_base_desc *txbd = (struct tx_base_desc *)txbd_buf;
	u8 i = 0;
	FUNCIN();

	if (NULL != txbd) {
		for (i = 0; i < ch_num; i++) {

			if (NULL == txbd[i].vir_addr)
				continue;
			_os_shmem_free(phl_to_drvpriv(phl_info),
						txbd[i].vir_addr,
						&txbd[i].phy_addr_l,
						&txbd[i].phy_addr_h,
						txbd[i].buf_len,
						txbd[i].cache,
						PCI_DMA_FROMDEVICE,
						txbd[i].os_rsvd[0]);
			txbd[i].vir_addr = NULL;
			txbd[i].cache = 0;
			_os_spinlock_free(phl_to_drvpriv(phl_info),
						&txbd[i].txbd_lock);
		}

		_os_mem_free(phl_to_drvpriv(phl_info), txbd,
						sizeof(struct tx_base_desc) * ch_num);
	}

	FUNCOUT();
}



static enum rtw_phl_status
_phl_alloc_txbd_pcie(struct phl_info_t *phl_info, u8 ch_num)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	struct tx_base_desc *txbd = NULL;
	u32 buf_len = 0;
	u16 txbd_num = (u16)hal_com->bus_cap.txbd_num;
	u8 addr_info_size = hal_com->bus_hw_cap.txbd_len;
	u8 i = 0;
	FUNCIN_WSTS(pstatus);

	buf_len = sizeof(struct tx_base_desc) * ch_num;
	txbd = _os_mem_alloc(phl_to_drvpriv(phl_info), buf_len);
	if (NULL != txbd) {
		for (i = 0; i < ch_num; i++) {
			if (!IS_DMA_CH_USED(i))
				continue;

			txbd[i].cache = false;
			buf_len = addr_info_size * txbd_num;
			txbd[i].vir_addr = _os_shmem_alloc(
						phl_to_drvpriv(phl_info),
						&txbd[i].phy_addr_l,
						&txbd[i].phy_addr_h,
						buf_len,
						txbd[i].cache,
						PCI_DMA_TODEVICE,
						&txbd[i].os_rsvd[0]);
			if (NULL == txbd[i].vir_addr) {
				pstatus = RTW_PHL_STATUS_RESOURCE;
				break;
			}
			txbd[i].buf_len = buf_len;
			txbd[i].host_idx = 0;
			txbd[i].hw_idx = 0;
			_os_atomic_set(phl_to_drvpriv(phl_info), &txbd[i].avail_num, txbd_num - 1);
			//txbd[i].avail_num = txbd_num;
			_os_spinlock_init(phl_to_drvpriv(phl_info),
						&txbd[i].txbd_lock);
			pstatus = RTW_PHL_STATUS_SUCCESS;
		}
	}

	if (RTW_PHL_STATUS_SUCCESS == pstatus)
		phl_info->hci->txbd_buf = (u8 *)txbd;
	else
		_phl_free_txbd_pcie(phl_info, (u8 *)txbd, ch_num);
	FUNCOUT_WSTS(pstatus);

	return pstatus;
}

enum rtw_phl_status _phl_update_default_rx_bd(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_phl_com_t *phl_com = (struct rtw_phl_com_t *)phl_info->phl_com;
	struct rx_base_desc *rxbd = NULL;
	struct rtw_rx_buf_ring *ring = NULL;
	struct rtw_rx_buf *rxbuf = NULL;
	u8 i = 0;
	u16 rxbd_num = 0;
	u32 j = 0;

	rxbd = (struct rx_base_desc *)hci_info->rxbd_buf;
	ring = (struct rtw_rx_buf_ring *)hci_info->rxbuf_pool;
	for (i = 0; i < hci_info->total_rxch_num; i++) {
		rxbd_num = rtw_hal_get_rxbd_num(phl_info->hal, i);

		RTW_PRINT("[RXBUF][%s][ch:%d] rxbd_num:%d\n", __func__, i, rxbd_num);

		for (j = 0; j < rxbd_num; j++) {
			rxbuf = query_idle_rx_buf(phl_info, &ring[i]);
			if (NULL == rxbuf) {
				PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
					"[WARNING] there is no resource for rx bd default setting\n");
				pstatus = RTW_PHL_STATUS_RESOURCE;
				break;
			}

			hstatus = rtw_hal_update_rxbd(phl_info->hal, &rxbd[i],
								rxbuf, i);
			if (RTW_HAL_STATUS_SUCCESS == hstatus) {
				enqueue_busy_rx_buf(phl_info, &ring[i], rxbuf, _tail);
				pstatus = RTW_PHL_STATUS_SUCCESS;
			} else {
				enqueue_idle_rx_buf(phl_info, &ring[i], rxbuf);
				PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING] update rx bd fail\n");
				pstatus = RTW_PHL_STATUS_FAILURE;
				break;
			}
		}
		/* Set RPQ buffer size for RP aggregation */
		if ((i == 1) && (rxbuf != NULL)) {
			phl_info->phl_com->mr_ctrl.hal_com->bus_hw_cap.rp_buf_size = rxbuf->buf_len;
		}

	}

	return pstatus;
}

static void _phl_reset_rxbd(struct phl_info_t *phl_info,
					struct rx_base_desc *rxbd, u8 ch_idx)
{
	u16 rxbd_num = rtw_hal_get_rxbd_num(phl_info->hal, ch_idx);
	_os_mem_set(phl_to_drvpriv(phl_info), rxbd->vir_addr, 0, rxbd->buf_len);
	rxbd->host_idx = 0;
	rxbd->avail_num = rxbd_num;
}


static void phl_rx_reset_pcie(struct phl_info_t *phl_info)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct hal_spec_t *hal_spec = &phl_com->hal_spec;
	struct rx_base_desc *rxbd = NULL;
	struct rtw_rx_buf_ring *ring = NULL;
	u8 ch = 0;

#ifdef RTW_WKARD_AP_RESET_RX_LOCK
	_os_spinlock(phl_to_drvpriv(phl_info), &phl_info->rx_lock, _bh, NULL);
#endif

	rxbd = (struct rx_base_desc *)hci_info->rxbd_buf;
	ring = (struct rtw_rx_buf_ring *)hci_info->rxbuf_pool;

	for (ch = 0; ch < hci_info->total_rxch_num; ch++) {
		_phl_reset_rxbd(phl_info, &rxbd[ch], ch);
		phl_release_busy_rx_buf(phl_info, &ring[ch],
					ring[ch].busy_rxbuf_cnt);
	}

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	if (phl_info->pending_amsdu) {
		phl_recycle_rx_buf(phl_info, phl_info->pending_amsdu);
		phl_info->pending_amsdu = NULL;
	}
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	hal_spec->rx_tag[0] = 0;
	hal_spec->rx_tag[1] = 0;
	_phl_update_default_rx_bd(phl_info);

#ifdef RTW_WKARD_AP_RESET_RX_LOCK
	_os_spinunlock(phl_to_drvpriv(phl_info), &phl_info->rx_lock, _bh, NULL);
#endif
}


void _phl_sort_ring_by_hw_res(struct phl_info_t *phl_info)
{
	_os_list *t_fctrl_result = &phl_info->t_fctrl_result;
	struct phl_ring_status *ring_sts, *t;
	u16 hw_res = 0, host_idx = 0, hw_idx = 0;
	u32 avail = 0, no_res = 0;
	_os_list *no_res_first = NULL;

	phl_list_for_loop_safe(ring_sts, t, struct phl_ring_status,
					t_fctrl_result, list) {

		if (ring_sts->ring_ptr->dma_ch > 32)
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
			"[WARNING] dma channel number larger than record map\n");

		if (no_res & (BIT0 << ring_sts->ring_ptr->dma_ch)) {
			if (&ring_sts->list == no_res_first)
				break;
			list_del(&ring_sts->list);
			list_add_tail(&ring_sts->list, t_fctrl_result);
			continue;
		} else if (avail & (BIT0 << ring_sts->ring_ptr->dma_ch)) {
			continue;
		}

		hw_res = rtw_hal_tx_res_query(phl_info->hal,
						ring_sts->ring_ptr->dma_ch,
						&host_idx, &hw_idx);
		if (0 == hw_res) {
			if (no_res_first == NULL)
				no_res_first = &ring_sts->list;
			list_del(&ring_sts->list);
			list_add_tail(&ring_sts->list, t_fctrl_result);
			no_res = no_res | (ring_sts->ring_ptr->dma_ch < 32 ? (BIT0 << ring_sts->ring_ptr->dma_ch) : 0);
		} else {
			avail = avail | (ring_sts->ring_ptr->dma_ch < 32 ? (BIT0 << ring_sts->ring_ptr->dma_ch) : 0);
		}
	}
}

void _phl_tx_flow_ctrl_pcie(struct phl_info_t *phl_info, _os_list *sta_list)
{
	/* _phl_sort_ring_by_hw_res(phl_info); */
	phl_tx_flow_ctrl(phl_info, sta_list);
}

static enum rtw_phl_status _phl_handle_xmit_ring_pcie
						(struct phl_info_t *phl_info,
						struct phl_ring_status *ring_sts)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	struct rtw_phl_tx_ring *tring = ring_sts->ring_ptr;
	struct rtw_xmit_req *tx_req = NULL;
	u16 rptr = 0, next_idx = 0;
	void *drv_priv = phl_to_drvpriv(phl_info);
	#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	#endif

#ifdef CONFIG_WFA_OFDMA_Logo_Test
	struct ru_grp_table *rugrptable = &phl_info->phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	int cnt =0, quota;

	quota = rtw_phl_cal_ru_quota(phl_info, ring_sts->macid);
#endif

	while (0 != ring_sts->req_busy) {

		#ifdef CONFIG_WFA_OFDMA_Logo_Test
		if(ru_ctrl->tx_phase){
			if(ring_sts->macid >0 && cnt >= quota){
			//printk("macid_%d break: cnt:%d\n", ring_sts->macid, cnt);
				break;
			}
		}
		#endif

		rptr = (u16)_os_atomic_read(drv_priv, &tring->phl_next_idx);

		tx_req = (struct rtw_xmit_req *)tring->entry[rptr];
		if (NULL == tx_req || NULL == tx_req->os_priv)  {
			PHL_ERR("tx_req is NULL!\n");
			break;
		}
		tx_req->mdata.band = ring_sts->band;
		tx_req->mdata.wmm = ring_sts->wmm;
		tx_req->mdata.hal_port = ring_sts->port;
		/*tx_req->mdata.mbssid = ring_sts->mbssid;*/
		tx_req->mdata.dma_ch = tring->dma_ch;

#ifdef CONFIG_VW_REFINE
		phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, tx_req->mdata.macid);
		if ( phl_sta == NULL || (phl_sta && !phl_sta->active))  {
			struct rtw_phl_evt_ops *ops = &phl_info->phl_com->evt_ops;

			PHL_INFO("drop txreq macid:%d \n", tx_req->mdata.macid);

			if (   RTW_PHL_TREQ_TYPE_NORMAL == tx_req->treq_type
#if defined(CONFIG_CORE_TXSC) || defined(CONFIG_PHL_TXSC)
			   || RTW_PHL_TREQ_TYPE_CORE_TXSC == tx_req->treq_type
				|| RTW_PHL_TREQ_TYPE_PHL_ADD_TXSC == tx_req->treq_type
#endif
			) {
				if ( NULL != ops->tx_recycle) {
					ops->tx_recycle(phl_to_drvpriv(phl_info), tx_req);
					#ifdef DEBUG_PHL_TX
					phl_info->phl_com->tx_stats.phl_txreq_sta_leave_drop++;
					#endif
				}
			}
			pstatus = RTW_PHL_STATUS_SUCCESS;
		} else
#endif
		{
			pstatus = hci_trx_ops->prepare_tx(phl_info, tx_req);
			if (RTW_PHL_STATUS_FRAME_DROP  == pstatus) {
				struct rtw_phl_evt_ops *ops = &phl_info->phl_com->evt_ops;

				if (   RTW_PHL_TREQ_TYPE_NORMAL == tx_req->treq_type
				    #if defined(CONFIG_CORE_TXSC) || defined(CONFIG_PHL_TXSC)
				    || RTW_PHL_TREQ_TYPE_CORE_TXSC == tx_req->treq_type
				    || RTW_PHL_TREQ_TYPE_PHL_ADD_TXSC == tx_req->treq_type
				    #endif
				   ) {
					if (NULL != ops->tx_recycle) {
						ops->tx_recycle(phl_to_drvpriv(phl_info), tx_req);
					}
				}
				pstatus = RTW_PHL_STATUS_SUCCESS;
			 }
		}

		if (RTW_PHL_STATUS_SUCCESS == pstatus) {
			#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
			if(phl_sta){
				if(phl_sta->active)
					phl_sta->enq_wd_pending_ok++;
			}
			#endif

			ring_sts->req_busy--;
			atomic_dec(&ring_sts->ring_ptr->tx_req_cnt);

#ifdef CONFIG_WFA_OFDMA_Logo_Test
			cnt ++ ;
#endif

#ifdef CONFIG_VW_REFINE
			phl_info->vw_cnt_snd += tx_req->vw_cnt;
#endif

			/* hana_todo, workaround here to update phl_index */
			_os_atomic_set(drv_priv, &tring->phl_idx, rptr);

			if (0 != ring_sts->req_busy) {
				next_idx = rptr + 1;

				if (next_idx >= MAX_PHL_RING_ENTRY_NUM) {
					_os_atomic_set(drv_priv,
						       &tring->phl_next_idx, 0);

				} else {
					_os_atomic_inc(drv_priv,
						       &tring->phl_next_idx);
				}
			}
		} else {
			PHL_INFO("HCI prepare tx fail\n");
			phl_info->pretx_fail = (phl_info->pretx_fail + 1) % 4096;
			#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
			if(phl_sta){
				if(phl_sta->active)
					phl_sta->enq_wd_pending_fail++;
			}
			#endif
			break;
		}
	}

	return pstatus;
}

static void _phl_tx_callback_pcie(void *context)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_handler *phl_handler
		= (struct rtw_phl_handler *)phl_container_of(context,
							struct rtw_phl_handler,
							os_handler);
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_handler->context;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	struct phl_ring_status *ring_sts = NULL, *t;
	void *drvpriv = phl_to_drvpriv(phl_info);
	_os_list sta_list;
	bool tx_pause = false;

	phl_info->phltx_cnt = (phl_info->phltx_cnt + 1) % 4096;

	FUNCIN_WSTS(pstatus);
	INIT_LIST_HEAD(&sta_list);

	/* check datapath sw state */
	tx_pause = phl_datapath_chk_trx_pause(phl_info, PHL_CTRL_TX);
	if (true == tx_pause)
		goto end;

#ifdef CONFIG_POWER_SAVE
	/* check ps state when tx is not paused */
	if (false == phl_ps_is_datapath_allowed(phl_info)) {
		PHL_WARN("PS does not pause sw tx, but current ps state is invalid, maybe power switching.\n");
		goto chk_stop;
	}
#endif

	if (true == phl_check_xmit_ring_resource(phl_info, &sta_list)) {
		_phl_tx_flow_ctrl_pcie(phl_info, &sta_list);

		phl_list_for_loop_safe(ring_sts, t, struct phl_ring_status,
		                       &phl_info->t_fctrl_result, list) {
			list_del(&ring_sts->list);
			_phl_handle_xmit_ring_pcie(phl_info, ring_sts);
			phl_release_ring_sts(phl_info, ring_sts);
		}
	}

	pstatus = hci_trx_ops->tx(phl_info);
	if (RTW_PHL_STATUS_FAILURE == pstatus) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING] phl_tx fail!\n");
	}

#ifdef CONFIG_POWER_SAVE
chk_stop:
#endif
	if (PHL_TX_STATUS_STOP_INPROGRESS ==
	    _os_atomic_read(drvpriv, &phl_info->phl_sw_tx_sts)) {
		PHL_WARN("PHL_TX_STATUS_STOP_INPROGRESS, going to stop sw tx.\n");
		phl_tx_stop_pcie(phl_info);
	}

end:
	phl_free_deferred_tx_ring(phl_info);

	FUNCOUT_WSTS(pstatus);
}


static u8 _phl_check_rx_hw_resource(struct phl_info_t *phl_info)
{
	struct hci_info_t *hci_info = phl_info->hci;
	u16 hw_res = 0, host_idx = 0, hw_idx = 0;
	u8 i = 0;
	u8 avail = 0;
	for (i = 0; i < hci_info->total_rxch_num; i++) {
		hw_res = rtw_hal_rx_res_query(phl_info->hal,
							i,
							&host_idx, &hw_idx);

		if (0 != hw_res) {
			avail = true;
			break;
		} else {
			avail = false;
		}
	}

	return avail;
}

static void _phl_rx_callback_pcie(void *context)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_handler *phl_handler
		= (struct rtw_phl_handler *)phl_container_of(context,
							struct rtw_phl_handler,
							os_handler);
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_handler->context;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	void *drvpriv = phl_to_drvpriv(phl_info);
	bool rx_pause = false;
#ifdef CONFIG_SYNC_INTERRUPT
	struct rtw_phl_evt_ops *ops = &phl_info->phl_com->evt_ops;
#endif /* CONFIG_SYNC_INTERRUPT */

	FUNCIN_WSTS(pstatus);

	/* check datapath sw state */
	rx_pause = phl_datapath_chk_trx_pause(phl_info, PHL_CTRL_RX);
	if (true == rx_pause)
		goto end;

	do {
		if (false == phl_check_recv_ring_resource(phl_info))
			break;

		pstatus = hci_trx_ops->rx(phl_info);

		if (RTW_PHL_STATUS_FAILURE == pstatus) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING] phl_rx fail!\n");
		}
	} while (false);

	if (PHL_RX_STATUS_STOP_INPROGRESS ==
	    _os_atomic_read(drvpriv, &phl_info->phl_sw_rx_sts)) {
		phl_rx_stop_pcie(phl_info);
	}

end:
	/* restore int mask of rx */
	rtw_hal_restore_rx_interrupt(phl_info->hal);
#ifdef CONFIG_SYNC_INTERRUPT
	ops->interrupt_restore(phl_to_drvpriv(phl_info), true);
#endif /* CONFIG_SYNC_INTERRUPT */

	FUNCOUT_WSTS(pstatus);

}

void _phl_fill_tx_meta_data(struct rtw_xmit_req *tx_req,
                            u16 packet_len)
{
	tx_req->mdata.wp_offset = 56;
	tx_req->mdata.wd_page_size = 1;
	tx_req->mdata.addr_info_num = tx_req->pkt_cnt;
	tx_req->mdata.pktlen = packet_len;
}



void phl_trx_resume_pcie(struct phl_info_t *phl_info, u8 type)
{
	if (PHL_CTRL_TX & type)
		phl_tx_resume_pcie(phl_info);

	if (PHL_CTRL_RX & type)
		phl_rx_resume_pcie(phl_info);
}

void phl_trx_reset_pcie(struct phl_info_t *phl_info, u8 type)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_stats *phl_stats = &phl_com->phl_stats;

	PHL_INFO("%s\n", __func__);

	if (PHL_CTRL_TX & type) {
		phl_tx_reset_pcie(phl_info);
		phl_reset_tx_stats(phl_stats);
	}

	if (PHL_CTRL_RX & type) {
		phl_rx_reset_pcie(phl_info);
		phl_reset_rx_stats(phl_stats);
		wmb();
	}
}

void phl_trx_stop_pcie(struct phl_info_t *phl_info)
{
	phl_trx_reset_pcie(phl_info, PHL_CTRL_TX|PHL_CTRL_RX);
}

#ifdef CONFIG_RTW_MEMPOOL
void rtw_init_rxbuf_mempool(struct rtw_phl_com_t *phl_com);
void rtw_fini_rxbuf_mempool(struct rtw_phl_com_t *phl_com);
#endif /* CONFIG_RTW_MEMPOOL */

void phl_trx_deinit_pcie(struct phl_info_t *phl_info)
{
	struct hci_info_t *hci_info = phl_info->hci;
	FUNCIN();
	_phl_free_rxbuf_pool_pcie(phl_info, hci_info->rxbuf_pool,
					hci_info->total_rxch_num);
	hci_info->rxbuf_pool = NULL;

	#ifdef CONFIG_RTW_MEMPOOL
	rtw_fini_rxbuf_mempool(phl_info->phl_com);
	#endif /* CONFIG_RTW_MEMPOOL */

	_phl_free_rxbd_pcie(phl_info, hci_info->rxbd_buf,
					hci_info->total_rxch_num);
	hci_info->rxbd_buf = NULL;

	_phl_free_wd_ring_pcie(phl_info, hci_info->wd_ring,
					hci_info->total_txch_num);
	hci_info->wd_ring = NULL;

	_phl_free_txbd_pcie(phl_info, hci_info->txbd_buf,
					hci_info->total_txch_num);
	hci_info->txbd_buf = NULL;

	FUNCOUT();
}

#if defined(RTW_TX_CPU_BALANCE) && defined(PHL_TX_PRIO_HIGH)
void phl_schedule_tx_handler(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	phl_schedule_handler(phl_info->phl_com, &phl_info->phl_tx_handler);
}
#endif

enum rtw_phl_status phl_trx_init_pcie(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct hci_info_t *hci_info = phl_info->hci;
	struct rtw_phl_handler *tx_handler = &phl_info->phl_tx_handler;
	struct rtw_phl_handler *rx_handler = &phl_info->phl_rx_handler;
	void *drv_priv = phl_to_drvpriv(phl_info);

	u8 txch_num = 0, rxch_num = 0;
	u16 i = 0;

	FUNCIN_WSTS(pstatus);

	do {
		tx_handler->type = RTW_PHL_HANDLER_PRIO_HIGH; /* tasklet */
		tx_handler->callback = _phl_tx_callback_pcie;
		tx_handler->context = phl_info;
		tx_handler->drv_priv = drv_priv;

		#ifdef CONFIG_RTW_OS_HANDLER_EXT
		tx_handler->id = RTW_PHL_TX_HANDLER;
		#endif /* CONFIG_RTW_OS_HANDLER_EXT */

		pstatus = phl_register_handler(phl_info->phl_com, tx_handler);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;

		rx_handler->type = RTW_PHL_HANDLER_PRIO_HIGH;
		rx_handler->callback = _phl_rx_callback_pcie;
		rx_handler->context = phl_info;
		rx_handler->drv_priv = drv_priv;

		#ifdef CONFIG_RTW_OS_HANDLER_EXT
		rx_handler->id = RTW_PHL_RX_HANDLER;
		#endif /* CONFIG_RTW_OS_HANDLER_EXT */

		pstatus = phl_register_handler(phl_info->phl_com, rx_handler);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;

		/* pcie tx sw resource */
		txch_num = rtw_hal_query_txch_num(phl_info->hal);
		hci_info->total_txch_num = txch_num;
		/* allocate tx bd */
		pstatus = _phl_alloc_txbd_pcie(phl_info, txch_num);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;
		/* allocate wd page */
		pstatus = _phl_alloc_wd_ring_pcie(phl_info, txch_num);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;

		for (i = 0; i < PHL_MACID_MAX_NUM; i++)
			hci_info->wp_seq[i] = WP_RESERVED_SEQ;

#ifdef CONFIG_VW_REFINE
		for (i = 0; i <= WP_MAX_SEQ_NUMBER; i++)
			 phl_info->free_wp[i] = i;
		phl_info->fr_ptr = 0;
		phl_info->fw_ptr = 0;
#endif
		/* Disable RX INT mitigation by default */
		#if defined(PCIE_TRX_MIT_EN)
		hci_info->fixed_mitigation = 1;
		#endif

#ifdef CONFIG_RTW_RX_BUF_SHARING
                /* rx_buf_info_lock init */
                printk("[%s:%d] init lock rx_buf_info_lock \n", __FUNCTION__, __LINE__);
                _os_spinlock_init(phl_to_drvpriv(phl_info),
                        &hci_info->rx_buf_info_lock);
#endif
		/* pcie rx sw resource */
		rxch_num = rtw_hal_query_rxch_num(phl_info->hal);
		hci_info->total_rxch_num = rxch_num;
		/* allocate rx bd */
		pstatus = _phl_alloc_rxbd_pcie(phl_info, rxch_num);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;

		#ifdef CONFIG_RTW_MEMPOOL
		rtw_init_rxbuf_mempool(phl_info->phl_com);
		#endif /* CONFIG_RTW_MEMPOOL */

		/* allocate wd page */
		pstatus = _phl_alloc_rxbuf_pool_pcie(phl_info, rxch_num);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;

	} while (false);

	if (RTW_PHL_STATUS_SUCCESS != pstatus)
		phl_trx_deinit_pcie(phl_info);
	else
		pstatus = _phl_update_default_rx_bd(phl_info);

	FUNCOUT_WSTS(pstatus);
	return pstatus;
}


enum rtw_phl_status phl_trx_config_pcie(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct hci_info_t *hci_info = phl_info->hci;

	do {
		hstatus = rtw_hal_trx_init(phl_info->hal, hci_info->txbd_buf,
						hci_info->rxbd_buf);
		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_ERR("rtw_hal_trx_init fail with status 0x%08X\n",
				hstatus);
			pstatus = RTW_PHL_STATUS_FAILURE;
			break;
		}
		else {
			pstatus = RTW_PHL_STATUS_SUCCESS;
		}

		_phl_reinit_wp_tag(phl_info);
		phl_tx_start_pcie(phl_info);
		phl_rx_start_pcie(phl_info);

	} while (false);

	return pstatus;
}

#ifdef CONFIG_PHL_TXSC
u8 *_phl_txsc_apply_shortcut(struct phl_info_t *phl_info, struct rtw_xmit_req *tx_req,
							struct rtw_phl_stainfo_t *phl_sta, struct rtw_phl_pkt_req *phl_pkt_req)
{
	struct phl_txsc_entry *ptxsc = NULL;

	if (!phl_sta)
		return (u8 *)ptxsc;

	if (tx_req->shortcut_id >= PHL_TXSC_ENTRY_NUM) {
		PHL_ERR("[PHL][TXSC] wrong shortcut_id:%d, plz check !!!\n", tx_req->shortcut_id);
		return (u8 *)ptxsc;
	}

	ptxsc = &phl_sta->phl_txsc[tx_req->shortcut_id];

	if ((tx_req->treq_type == RTW_PHL_TREQ_TYPE_CORE_TXSC)
    #ifdef CONFIG_RTW_ENABLE_HW_TXSC
		|| (tx_req->treq_type == RTW_PHL_TREQ_TYPE_HW_TXSC)
    #endif /* CONFIG_RTW_ENABLE_HW_TXSC */
	   ) {

		if (!ptxsc) {
			PHL_ERR("[txsc][phl] fetal err: ptxsc = NULL, plz check.\n");
			return (u8 *)ptxsc;
		}

		if (!ptxsc->txsc_wd_cached || ptxsc->txsc_wd_seq_offset == 0) {
			PHL_ERR("[txsc][phl] phl_txsc re-update, txsc_wd_cached:%d, txsc_wd_seq_offset:%d\n",
				ptxsc->txsc_wd_cached, ptxsc->txsc_wd_seq_offset);
			ptxsc->txsc_wd_cached = ptxsc->txsc_wd_seq_offset = 0;
			tx_req->treq_type |= RTW_PHL_TREQ_TYPE_PHL_UPDATE_TXSC;
			return (u8 *)ptxsc;
		}

		_os_mem_cpy(phl_info, phl_pkt_req->wd_page, ptxsc->txsc_wd_cache, ptxsc->txsc_wd_seq_offset);
		phl_pkt_req->wd_len = ptxsc->txsc_wd_len;
		phl_pkt_req->wd_seq_offset = ptxsc->txsc_wd_seq_offset;
#ifdef DEBUG_PHL_TX
		phl_info->phl_com->tx_stats.phl_txsc_apply_cnt++;
#endif
		/* update pktlen in wd_page, wd_body[8:15] = pktsize */
		/* packet_len = cpu_to_le16(tx_req->mdata.pktlen);
		 * _os_mem_cpy(phl_info, phl_pkt_req.wd_page+8, &packet_len, sizeof(u16));
		*/

		ptxsc->txsc_cache_hit++;
	}

	return (u8 *)ptxsc;
}

enum rtw_phl_status
_phl_txsc_add_shortcut(struct phl_info_t *phl_info, struct rtw_xmit_req *tx_req,
								struct rtw_phl_pkt_req *phl_pkt_req, struct phl_txsc_entry *ptxsc)
{

	if (tx_req->shortcut_id >= PHL_TXSC_ENTRY_NUM) {
		RTW_ERR("[PHL][TXSC] wrong shortcut_id:%d, plz check.\n", tx_req->shortcut_id);
		return RTW_PHL_STATUS_FAILURE;
	}

	if (!ptxsc) {
		PHL_ERR("[txsc][phl] fetal err: ptxsc = NULL, shortcut_id = %d, plz check.\n", tx_req->shortcut_id);
		return RTW_PHL_STATUS_FAILURE;
	}

	if (tx_req->treq_type & RTW_PHL_TREQ_TYPE_PHL_UPDATE_TXSC) {

		_os_mem_set(phl_info, ptxsc, 0x0, sizeof(struct phl_txsc_entry));
		_os_mem_cpy(phl_info, ptxsc->txsc_wd_cache, phl_pkt_req->wd_page, phl_pkt_req->wd_seq_offset);

		ptxsc->txsc_wd_len = phl_pkt_req->wd_len;
		ptxsc->txsc_wd_seq_offset = phl_pkt_req->wd_seq_offset;
		ptxsc->txsc_wd_cached = 1;

		tx_req->treq_type &= ~RTW_PHL_TREQ_TYPE_PHL_UPDATE_TXSC;
		if (tx_req->treq_type != RTW_PHL_TREQ_TYPE_PHL_ADD_TXSC)
			PHL_PRINT("Updated WD for request type %u\n", tx_req->treq_type);
	}

	return RTW_PHL_STATUS_SUCCESS;
}
#endif

enum rtw_phl_status
phl_check_wp_tag_resource(struct phl_info_t *phl_info,
	struct rtw_wd_page_ring *wd_ring, u16 *wp_seq, u8 dma_ch)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	void *ptr = NULL;

	/* error handle when wp_tag out of resource */
	while (NULL != wd_ring[dma_ch].wp_tag[*wp_seq].ptr) {
		#ifdef DEBUG_PHL_TX
		phl_info->phl_com->tx_stats.wp_tg_out_of_resource++;
		#endif
		*wp_seq = (*wp_seq + 1) % WP_MAX_SEQ_NUMBER;
		if (0 == *wp_seq)
			*wp_seq = 1;
		if (*wp_seq == wd_ring[dma_ch].wp_seq) {
			#ifdef DEBUG_PHL_TX
			phl_info->phl_com->tx_stats.wp_tg_force_reuse++;
			#endif
			ptr = wd_ring[dma_ch].wp_tag[*wp_seq].ptr;
			PHL_ERR("wp_tag out of resource & force reuse!\n");
			PHL_ERR("stuck wp info:\n");
			PHL_ERR("dma_ch = %d, wp_seq = 0x%x, ptr = %p!\n",
				dma_ch, *wp_seq, ptr);
			PHL_ERR("wifi seq = %d\n",
				((struct rtw_xmit_req *)ptr)->mdata.sw_seq);
			_phl_dump_busy_wp(phl_info);
			/* force recycle this packet becoz there is no wp_tag */
			phl_recycle_payload(phl_info, dma_ch, *wp_seq,
				TX_STATUS_TX_FAIL_FORCE_DROP_BY_STUCK, 1);
			pstatus = RTW_PHL_STATUS_FAILURE;
			break;
		}
	}

	return pstatus;
}


__IMEM_WLAN_SECTION__
enum rtw_phl_status
phl_prepare_tx_pcie(struct phl_info_t *phl_info, struct rtw_xmit_req *tx_req)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
 	struct rtw_wp_rpt_stats *rpt_stats =
		(struct rtw_wp_rpt_stats *)hal_com->trx_stat.wp_rpt_stats;
	struct hci_info_t *hci_info = NULL;
	/* struct rtw_pkt_buf_list *pkt_buf = NULL; */
	struct rtw_wd_page_ring *wd_ring = NULL;
	struct rtw_wd_page *wd_page = NULL;
	struct rtw_phl_pkt_req phl_pkt_req;
	u16 /*packet_len = 0,*/ wp_seq = 0;
	u8 dma_ch = 0, i = 0;
	u16 mid = 0;
	static u8 dma_ch_old=0;
	static u16 wp_seq_old=0, not_recycled=0;
#ifdef CONFIG_PHL_TXSC
	struct phl_txsc_entry *ptxsc = NULL;
	struct rtw_phl_stainfo_t *phl_sta = NULL;
#endif
	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	void *drv_priv = phl_to_drvpriv(phl_info);
	#endif
	FUNCIN_WSTS(pstatus);
	do {
		if (NULL == phl_info->hci) {
			PHL_ERR("phl_info->hci is NULL!\n");
			break;
		}
		hci_info = (struct hci_info_t *)phl_info->hci;
		wd_ring = (struct rtw_wd_page_ring *)hci_info->wd_ring;

		if (NULL == tx_req)  {
			PHL_ERR("tx_req is NULL!\n");
			break;
		}
		mid = tx_req->mdata.macid;
		dma_ch = tx_req->mdata.dma_ch;
		#ifdef CONFIG_PHL_TXSC
		phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, tx_req->mdata.macid);
		#endif

#ifdef CONFIG_VW_REFINE
 		if ( 0 == dma_ch )  {
			u16 val;

			val = (phl_info->fr_ptr + 1) % WP_MAX_SEQ_NUMBER;
			if ( val == phl_info->fw_ptr)
   			     break;

			wp_seq = phl_info->free_wp[phl_info->fr_ptr];
			if ( wp_seq == WP_USED_SEQ ) {
   			     PHL_ERR("wp_tag %d: wrong rw ptr wp_seq:%d !!! \n", phl_info->fr_ptr, wp_seq);
   			     break;
			}
			// phl_info->fr_ptr = (phl_info->fr_ptr + 1) % WP_MAX_SEQ_NUMBER;
		} else
#endif
			wp_seq = wd_ring[dma_ch].wp_seq;

		#ifdef RTW_WKARD_HIQ_STUCK
 		if (dma_ch == 9) {
 			struct hw_band_ctl_t *band_ctrl = get_band_ctrl(phl_info, 0);
 			if (tx_req->total_len > 1600) {
 				PHL_WARN("TX %uB to CH9\n", tx_req->total_len);
 				return RTW_PHL_STATUS_FRAME_DROP;
 			}
 			if (tx_req->mdata.mbssid
 			    && !(BIT(tx_req->mdata.mbssid) & band_ctrl->mbssid_map)) {
 				PHL_ERR("TX disabled MBSSID %u to CH9\n", tx_req->mdata.mbssid);
 				return RTW_PHL_STATUS_FRAME_DROP;
 			}
 		}
		#endif /* RTW_WKARD_HIQ_STUCK */

		if (phl_check_wp_tag_resource(phl_info, wd_ring, &wp_seq, dma_ch) ==
				RTW_PHL_STATUS_FAILURE)
			break;

		/*
		total len will calc in core layer and svae in txreq
		pkt_buf = (struct rtw_pkt_buf_list *)&tx_req->pkt_list[0];
		for (i = 0; i < tx_req->pkt_cnt; i++) {
			packet_len += pkt_buf->length;
			pkt_buf++;
		}
		tx_req->total_len = packet_len;
		*/
		wd_page = query_idle_wd_page(phl_info, &wd_ring[dma_ch]);
		if (NULL == wd_page) {
			PHL_INFO("query idle wd page fail!\n");
			PHL_INFO("dma_ch = %d, idle wd num = %d, "
				"busy wd num = %d, pending wd num = %d\n",
				dma_ch,
				wd_ring[dma_ch].idle_wd_page_cnt,
				wd_ring[dma_ch].busy_wd_page_cnt,
				wd_ring[dma_ch].pending_wd_page_cnt);
			break;
		}

		/*
		RTW_CORE_TX_MSDU_TRANSFER_IN_PHL
		tx_dev_map move here so donot need check RTW_PHL_TREQ_TYPE_PHL_UPDATE_TXSC
		and becsause tx_req->total_len will be caculated in tx_dev_map,
		so it should move before _phl_fill_tx_meta_data
		*/
#ifdef CONFIG_VW_REFINE
		if ((tx_req->treq_type == RTW_PHL_TREQ_TYPE_CORE_TXSC
		/*|| tx_req->treq_type == (RTW_PHL_TREQ_TYPE_CORE_TXSC|RTW_PHL_TREQ_TYPE_PHL_UPDATE_TXSC)*/) &&
			(!tx_req->is_map) ) {
			struct rtw_phl_evt_ops *ops = &phl_info->phl_com->evt_ops;
			ops->tx_dev_map(phl_to_drvpriv(phl_info), tx_req);
		}
#endif

		/* hana_todo */
		_phl_fill_tx_meta_data(tx_req, tx_req->total_len);

		phl_pkt_req.wd_page = wd_page->vir_addr;

		phl_pkt_req.wp_seq = wp_seq;
		phl_pkt_req.tx_req = tx_req;

#ifdef CONFIG_PHL_TXSC
		phl_pkt_req.wd_len = phl_pkt_req.wd_seq_offset = 0;
		ptxsc = (struct phl_txsc_entry *)_phl_txsc_apply_shortcut(phl_info, tx_req, phl_sta, &phl_pkt_req);
#endif
		/* move ops->tx_dev_map to upper for */

		wd_page->mac_id = tx_req->mdata.macid;
		wd_page->tid = tx_req->mdata.tid;

		hstatus = rtw_hal_update_wd_page(phl_info->hal, &phl_pkt_req);
		wd_page->buf_len = phl_pkt_req.wd_len;

#ifdef DEBUG_PHL_TX
		if (phl_info->phl_com->tx_stats.flag_print_wdpage_once == 1) {
			if (RTW_HAL_STATUS_SUCCESS == hstatus)
				debug_dump_buf(phl_pkt_req.wd_page, (u16)phl_pkt_req.wd_len, "dump wd page");
			phl_info->phl_com->tx_stats.flag_print_wdpage_once = 0;
		}
#endif

		if ( ((wd_page->vir_addr[0] == 0 && wd_page->vir_addr[1] == 0 &&
			 wd_page->vir_addr[2] == 0 && wd_page->vir_addr[3] == 0)) ) {

#if 0
			 printk_ratelimited("pre-wd page:%x req buf:%x buf:%02x error:%d\n", \
				 wd_page->vir_addr, phl_pkt_req.wd_page, wd_page->vir_addr[0], hstatus);
#endif

			 rtw_release_target_wd_page(phl_info, &wd_ring[dma_ch],
										wd_page);
			 return RTW_PHL_STATUS_FRAME_DROP;
		}

		if (RTW_HAL_STATUS_SUCCESS == hstatus) {
			hci_info->wp_seq[mid] = phl_pkt_req.wp_seq;
			enqueue_pending_wd_page(phl_info, &wd_ring[dma_ch],
						wd_page, _tail);

			#ifdef CONFIG_WFA_OFDMA_Logo_Test
			if(wd_page->tid == 0){
				if(phl_sta)
					_os_atomic_inc(drv_priv, &phl_sta->cnt_pending_wd);
			}
			#endif

			#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
			if(phl_sta){
				if(phl_sta->active)
					phl_sta->wd_enq_pending++;
			}
			#endif

			tx_req->tx_time = _os_get_cur_time_ms();

			PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
				  "update tx req(%p) in ch(%d) with wp_seq(0x%x) and wifi seq(%d)!\n",
				  tx_req, dma_ch, wp_seq, tx_req->mdata.sw_seq);
			_os_spinlock(phl_to_drvpriv(phl_info),
				     &wd_ring[dma_ch].wp_tag_lock,
				     _bh, NULL);

			wd_ring[dma_ch].wp_tag[wp_seq].ptr = (u8 *)tx_req;
			wd_page->wp_seq = wp_seq;
			rpt_stats[dma_ch].busy_cnt++;
			_os_spinunlock(phl_to_drvpriv(phl_info),
				       &wd_ring[dma_ch].wp_tag_lock,
				       _bh, NULL);

			wp_seq = (wp_seq + 1) % WP_MAX_SEQ_NUMBER;
			if (0 == wp_seq)
				wp_seq = 1;

			wd_ring[dma_ch].wp_seq = wp_seq;

			pstatus = RTW_PHL_STATUS_SUCCESS;
			#ifdef CONFIG_RTW_MIRROR_DUMP
			phl_mirror_dump_wd(phl_info, dma_ch, wd_page->vir_addr, wd_page->buf_len);
			#endif

			//wb wd page
			if(wd_page->cache == true) {
				PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "[%s] wd page cache wback \n",
					__FUNCTION__);
				_os_cache_wback(phl_to_drvpriv(phl_info),
					&wd_page->phy_addr_l,
					&wd_page->phy_addr_h,
					wd_page->buf_len, PCI_DMA_TODEVICE);
			}

#ifdef CONFIG_PHL_TXSC
			_phl_txsc_add_shortcut(phl_info, tx_req, &phl_pkt_req, ptxsc);
#endif
#ifdef CONFIG_VW_REFINE
			if ( 0 == dma_ch ) {
				phl_info->free_wp[phl_info->fr_ptr] = WP_USED_SEQ;
				phl_info->fr_ptr = (phl_info->fr_ptr + 1) % WP_MAX_SEQ_NUMBER;
			}
#endif
			break;
		} else {
			rtw_release_target_wd_page(phl_info, &wd_ring[dma_ch],
						wd_page);
			pstatus = RTW_PHL_STATUS_FAILURE;
			break;
		}
	} while(false);
	FUNCOUT_WSTS(pstatus);
	return pstatus;
}

static enum rtw_phl_status
phl_handle_pending_wd(struct phl_info_t *phl_info,
				struct rtw_wd_page_ring *wd_ring,
				u16 txcnt, u8 ch)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct tx_base_desc *txbd = NULL;
	struct rtw_wd_page *wd = NULL;
	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_wifi_role_t *wrole = &(phl_com->wifi_roles[0]);
	struct rtw_phl_stainfo_t *psta, *n, *phl_sta;
	_os_spinlockfg sp_flags;
	u32 wd_ptr;
	#endif
	struct rtw_wd_page_list phl_wd_list;
	u16 wd_num = 0, pending_num = 0, j = 0;
	u16 cnt = 0;

#ifdef RTW_WKARD_DYNAMIC_LTR
	if (true != _phl_judge_act_ltr_switching_conditions(phl_info, ch)) {
		_phl_act_ltr_update_stats(phl_info, false, ch,
									wd_ring->pending_wd_page_cnt);
		return RTW_PHL_STATUS_FAILURE;
	} else {
		_phl_act_ltr_update_stats(phl_info, true, ch,
									wd_ring->pending_wd_page_cnt);
	}
#endif

#ifdef CONFIG_WFA_OFDMA_Logo_Test
	if (wrole && wrole->active) {
		_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
		phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
		                       &wrole->assoc_sta_queue.queue, list) {
			wd_ptr = _os_atomic_read(drv_priv, &psta->cnt_current_wd);
			psta->current_wd_record = wd_ptr;
			wd_ptr = _os_atomic_read(drv_priv, &psta->cnt_pending_wd);
			psta->pending_wd_record = wd_ptr;
		}
		_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	}
#endif

	txbd = (struct tx_base_desc *)hci_info->txbd_buf;
	pending_num = (txcnt > WD_NUM_IN_ONE_UPDATE_TXBD) ? WD_NUM_IN_ONE_UPDATE_TXBD : txcnt;
	while (txcnt > cnt) {
		while (wd_num < pending_num) {
		wd = query_pending_wd_page(phl_info, wd_ring);
			if (NULL == wd)
				break;

			wd->ls = 1;//tmp set LS=1

			phl_wd_list.wd_page[wd_num] = wd;
			wd_num++;
		}

		if (wd_num == 0) {
			pstatus = RTW_PHL_STATUS_RESOURCE;
			break;
		}

		if (NULL == wd) {
			pstatus = RTW_PHL_STATUS_RESOURCE;
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "query Tx pending WD fail!\n");
			break;
		}

		hstatus = rtw_hal_update_txbd(phl_info->hal, txbd, &phl_wd_list, ch, wd_num);
		if (RTW_HAL_STATUS_SUCCESS == hstatus) {
			for (j = 0; j < wd_num; j++) {
				wd = phl_wd_list.wd_page[j];

				enqueue_busy_wd_page(phl_info, wd_ring, phl_wd_list.wd_page[j], _tail);

				#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
					#ifdef CONFIG_TP_IMAGE
					if (wd)
					{
						phl_sta = rtw_phl_get_stainfo_by_macid_wo_lock(phl_info, wd->mac_id);
					}
					else
					{
						PHL_ERR("[TPDEBUG][L1]wd is NULL, set phl_sta to NULL\n");
						phl_sta = NULL;
					}
					#else /* !CONFIG_TP_IMAGE */
					phl_sta = rtw_phl_get_stainfo_by_macid_wo_lock(phl_info, wd->mac_id);
					#endif /* CONFIG_TP_IMAGE */
				phl_com->update_txbd_ok++;
				if (phl_sta && phl_sta->active) {
					phl_sta->wd_deq_pending++;
					phl_sta->wd_enq_busy++;
				}
				#endif

				#ifdef CONFIG_WFA_OFDMA_Logo_Test
				if (wd->tid == 0 && phl_sta) {
					//phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, wd->mac_id);
					_os_atomic_inc(drv_priv, &phl_sta->cnt_current_wd);
					_os_atomic_dec(drv_priv, &phl_sta->cnt_pending_wd);
				}
				#endif

				#ifdef RTW_WKARD_HIQ_STUCK
				if (ch == 9) {
					if (phl_sta == NULL
					    || phl_sta->wrole == NULL) {
						void *ptr;
						struct rtw_xmit_req *treq;
						ptr = wd_ring[ch].wp_tag[wd->wp_seq].ptr;
						treq = (struct rtw_xmit_req *)ptr;
						PHL_ERR("HiQ no STA! M%u WS:%u MBSSID:%u L:%u\n",
						        wd->mac_id,
						        wd->wp_seq,
						        (treq ? treq->mdata.mbssid : 0),
						        (treq ? treq->total_len : 0));
					} else {
						struct rtw_wifi_role_t *wrole;
						u32 hiq_busy_cnt;

						wrole = phl_sta->wrole;
						if (!IS_AP_ROLE_TYPE(wrole->type))
							PHL_ERR("Non-AP role type %u TX HiQ\n",
							        wrole->type);

						hiq_busy_cnt = _os_atomic_inc_return(drv_priv,
						                                     &wrole->hiq_busy_cnt);
						if (hiq_busy_cnt == 1) {
							wrole->last_hiq_wp_time = _os_get_cur_time_ms();
						}
					}
				}
				#endif /* RTW_WKARD_HIQ_STUCK*/
			}
			pstatus = RTW_PHL_STATUS_SUCCESS;
		} else {
			for(j = 0; j < phl_wd_list.handled_wd_num; j++) {
				wd = phl_wd_list.wd_page[j];
				enqueue_busy_wd_page(phl_info, wd_ring, wd, _tail);
				//phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, wd->mac_id);
				#ifdef CONFIG_TP_IMAGE
				if (wd)
				{
					phl_sta = rtw_phl_get_stainfo_by_macid_wo_lock(phl_info, wd->mac_id);
				}
				else
				{
					PHL_ERR("[TPDEBUG][L2]wd is NULL, set phl_sta to NULL\n");
					phl_sta = NULL;
				}
				#else /* !CONFIG_TP_IMAGE */
				phl_sta = rtw_phl_get_stainfo_by_macid_wo_lock(phl_info, wd->mac_id);
				#endif /* CONFIG_TP_IMAGE */
				#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
				phl_com->update_txbd_ok++;
				if (phl_sta && phl_sta->active) {
					phl_sta->wd_deq_pending++;
					phl_sta->wd_enq_busy++;
				}
				#endif
				#ifdef CONFIG_WFA_OFDMA_Logo_Test
				if(wd->tid == 0){
					//phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, wd->mac_id);
					if (phl_sta) {
						_os_atomic_inc(drv_priv, &phl_sta->cnt_current_wd);
						_os_atomic_dec(drv_priv, &phl_sta->cnt_pending_wd);
					}
				}
				#endif
			}

			for(j = wd_num - 1; j>= phl_wd_list.handled_wd_num; j--) {
				wd = phl_wd_list.wd_page[j];
				enqueue_pending_wd_page(phl_info, wd_ring, wd, _first);
				//phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, wd->mac_id);
				#ifdef CONFIG_TP_IMAGE
				if (wd)
				{
					phl_sta = rtw_phl_get_stainfo_by_macid_wo_lock(phl_info, wd->mac_id);
				}
				else
				{
					PHL_ERR("[TPDEBUG][L3]wd is NULL, set phl_sta to NULL\n");
					phl_sta = NULL;
				}
				#else /* !CONFIG_TP_IMAGE */
				phl_sta = rtw_phl_get_stainfo_by_macid_wo_lock(phl_info, wd->mac_id);
				#endif /* CONFIG_TP_IMAGE */
				#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
				phl_com->update_txbd_fail++;
				if(phl_sta)
					phl_sta->wd_enq_busy_fail++;
				#endif
			}

			if(phl_wd_list.handled_wd_num)
				pstatus = RTW_PHL_STATUS_SUCCESS;
			else
				pstatus = RTW_PHL_STATUS_RESOURCE;
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "update Tx BD fail!\n");
			break;
		}

		cnt += wd_num;
		wd_num = 0;
		pending_num = (txcnt - cnt > WD_NUM_IN_ONE_UPDATE_TXBD) ? WD_NUM_IN_ONE_UPDATE_TXBD : (txcnt - cnt);
	}

	#ifdef CONFIG_RTW_PLATFORM_NEED_MEMORY_FLUSH
	iob();
	#else
	#ifdef CONFIG_VW_REFINE
	wmb();
	#endif
	#endif /* CONFIG_RTW_PLATFORM_NEED_MEMORY_FLUSH */

	if (RTW_PHL_STATUS_SUCCESS == pstatus) {
#ifdef RTW_WKARD_DYNAMIC_LTR
		_phl_switch_act_ltr(phl_info, ch);
#endif
		hstatus = rtw_hal_trigger_txstart(phl_info->hal, txbd, ch);
		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]update Tx RW ptr fail!\n");

			#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
			phl_com->trigger_txstart_fail++;
			#endif

			#ifdef CONFIG_WFA_OFDMA_Logo_Test
			if (wrole && wrole->active) {
				_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
				phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
				                       &wrole->assoc_sta_queue.queue, list) {
					_os_atomic_set(drv_priv, &psta->cnt_current_wd, psta->current_wd_record);
					_os_atomic_set(drv_priv, &psta->cnt_pending_wd, psta->pending_wd_record);
				}
				_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
			}
			#endif /* CONFIG_WFA_OFDMA_Logo_Test */

			pstatus = RTW_PHL_STATUS_FAILURE;
		}else {
			#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
			phl_com->trigger_txstart_ok++;
			#endif
		}
	}

	return pstatus;
}


static enum rtw_phl_status
phl_handle_busy_wd(struct phl_info_t *phl_info,
			struct rtw_wd_page_ring *wd_ring, u8 ch, u16 hw_idx)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &wd_ring->busy_wd_page_list;
	struct rtw_wd_page *wd = NULL;
	u16 bndy = (u16)hal_com->bus_cap.txbd_num;
	u16 target = 0;
	u16 release_num = 0;

	do {
		_os_spinlock(drv_priv, &wd_ring->busy_lock, _bh, NULL);

		if (list_empty(list)) {
			pstatus = RTW_PHL_STATUS_SUCCESS;
			_os_spinunlock(drv_priv, &wd_ring->busy_lock, _bh, NULL);
			break;
		}

		if (wd_ring->busy_wd_page_cnt > (bndy - 1)) {
			release_num = wd_ring->busy_wd_page_cnt - (bndy - 1);
			_os_spinunlock(drv_priv, &wd_ring->busy_lock, _bh, NULL);
			pstatus = rtw_release_busy_wd_page(phl_info, wd_ring,
								ch, release_num);

			if (RTW_PHL_STATUS_SUCCESS != pstatus)
				break;
			else
				_os_spinlock(drv_priv, &wd_ring->busy_lock, _bh, NULL);
		}

		wd = list_first_entry(list, struct rtw_wd_page, list);
		target = wd->host_idx;

		if (hw_idx >= target)
			release_num = ((hw_idx - target) + 1) % bndy;
		else
			release_num = ((bndy - target) + (hw_idx + 1)) % bndy;

		_os_spinunlock(drv_priv, &wd_ring->busy_lock, _bh, NULL);

		pstatus = rtw_release_busy_wd_page(phl_info, wd_ring,
							ch, release_num);

		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;
	} while (false);

	return pstatus;
}

enum rtw_phl_status phl_recycle_busy_wd(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_wd_page_ring *wd_ring = NULL;
	u16 hw_res = 0, host_idx = 0, hw_idx = 0;
	u8 ch = 0;
	FUNCIN_WSTS(pstatus);
	wd_ring = (struct rtw_wd_page_ring *)hci_info->wd_ring;

	for (ch = 0; ch < hci_info->total_txch_num; ch++) {
		hw_res = rtw_hal_tx_res_query(phl_info->hal, ch, &host_idx,
							&hw_idx);
		pstatus = phl_handle_busy_wd(phl_info, &wd_ring[ch], ch, hw_idx);
	}

	FUNCOUT_WSTS(pstatus);
	return pstatus;
}

static enum rtw_phl_status
phl_handle_busy_h2c(struct phl_info_t *phl_info,
			struct phl_h2c_pkt_pool *h2c_pool, u16 hw_idx)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_queue *queue = &h2c_pool->busy_h2c_pkt_list;
	_os_list *list = &h2c_pool->busy_h2c_pkt_list.queue;
	struct rtw_h2c_pkt *h2c = NULL;
	u16 bndy = (u16)hal_com->bus_cap.txbd_num;
	u16 target = 0;
	u16 release_num = 0;
	u16 tmp_cnt = 0;

	do {
		_os_spinlock(drv_priv, &queue->lock, _bh, NULL);

		if (list_empty(list)) {
			pstatus = RTW_PHL_STATUS_SUCCESS;

			_os_spinunlock(drv_priv, &queue->lock, _bh, NULL);
			break;
		}
#if 0
        /* h2c corrput content on pcie bus because recycling h2c is too fast */
		tmp_cnt = (u16)queue->cnt;
		if (tmp_cnt > (bndy - 1)) {
			release_num = tmp_cnt - (bndy - 1);
			_os_spinunlock(drv_priv, &queue->lock, _bh, NULL);
			pstatus = phl_release_busy_h2c_pkt(phl_info, h2c_pool,
							release_num);

			if (RTW_PHL_STATUS_SUCCESS != pstatus)
				break;
			else
				_os_spinlock(drv_priv, &queue->lock, _bh, NULL);
		}
#endif

		h2c = list_first_entry(list, struct rtw_h2c_pkt, list);
		target = h2c->host_idx;

		if (hw_idx >= target)
			release_num = ((hw_idx - target) + 1) % bndy;
		else
			release_num = ((bndy - target) + (hw_idx + 1)) % bndy;

		_os_spinunlock(drv_priv, &queue->lock, _bh, NULL);

		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : release_num %d.\n", __func__, release_num);

		pstatus = phl_release_busy_h2c_pkt(phl_info, h2c_pool,
							release_num);

		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;
	} while (false);

	return pstatus;
}

enum rtw_phl_status phl_recycle_busy_h2c(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_h2c_pkt_pool *h2c_pool = NULL;
	u16 hw_res = 0, host_idx = 0, hw_idx = 0;
	u8 fwcmd_queue_idx = 0;

	FUNCIN_WSTS(pstatus);
	h2c_pool = (struct phl_h2c_pkt_pool *)phl_info->h2c_pool;
	_os_spinlock(phl_to_drvpriv(phl_info), &h2c_pool->recycle_lock, _bh, NULL);
	fwcmd_queue_idx = rtw_hal_get_fwcmd_queue_idx(phl_info->hal);

	hw_res = rtw_hal_tx_res_query(phl_info->hal, fwcmd_queue_idx, &host_idx,
						&hw_idx);
	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : host idx %d, hw_idx %d.\n",
			  __func__, host_idx, hw_idx);
	pstatus = phl_handle_busy_h2c(phl_info, h2c_pool, hw_idx);
	_os_spinunlock(phl_to_drvpriv(phl_info), &h2c_pool->recycle_lock, _bh, NULL);
	FUNCOUT_WSTS(pstatus);
	return pstatus;
}

enum rtw_phl_status phl_recycle_h2c(struct phl_info_t *phl_info, void *pkt)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_info->phl_com);
	struct rtw_h2c_pkt *h2c_pkt = (struct rtw_h2c_pkt *)pkt;

	pstatus = phl_enqueue_idle_h2c_pkt(phl_info,
								   h2c_pkt);
	return pstatus;
}

__IMEM_WLAN_SECTION__
static enum rtw_phl_status phl_tx_pcie(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	struct rtw_wd_page_ring *wd_ring = NULL;
	u16 hw_res = 0, host_idx = 0, hw_idx = 0, txcnt = 0;
	u8 ch = 0;
	struct tx_base_desc *txbd = NULL;

	FUNCIN_WSTS(pstatus);
	txbd = (struct tx_base_desc *)hci_info->txbd_buf;
	wd_ring = (struct rtw_wd_page_ring *)hci_info->wd_ring;

	for (ch = 0; ch < hci_info->total_txch_num; ch++) {
#ifndef RTW_WKARD_WIN_TRX_BALANCE
		/* if wd_ring is empty, do not read hw_idx for saving cpu cycle */
		if (wd_ring[ch].pending_wd_page_cnt == 0 && wd_ring[ch].busy_wd_page_cnt == 0){
			pstatus = RTW_PHL_STATUS_SUCCESS;
			continue;
		}
#endif
		/* hana_todo skip fwcmd queue */
		if(ch > 7)
			hw_res = rtw_hal_tx_res_query(phl_info->hal, ch, &host_idx,
							&hw_idx);
		else
			hw_res = rtw_hal_tx_res_query_sw(phl_info->hal, txbd, ch, &host_idx,
							&hw_idx);

		if(ch > 7)
			phl_handle_busy_wd(phl_info, &wd_ring[ch], ch, hw_idx);
		else {
			if(hw_res <  hal_com->bus_cap.txbd_num/4) {
				hw_res = rtw_hal_tx_res_query(phl_info->hal, ch, &host_idx,
								&hw_idx);
				txbd->hw_idx = hw_idx;
				phl_handle_busy_wd(phl_info, &wd_ring[ch], ch, hw_idx);
			}
		}

		if (list_empty(&wd_ring[ch].pending_wd_page_list))
			continue;

		if (RTW_PHL_STATUS_FAILURE == pstatus)
			continue;

		if (0 == hw_res) {
			continue;

		} else {
			txcnt = (hw_res < wd_ring[ch].pending_wd_page_cnt) ?
				hw_res : wd_ring[ch].pending_wd_page_cnt;

			pstatus = phl_handle_pending_wd(phl_info, &wd_ring[ch],
							txcnt, ch);

			if (RTW_PHL_STATUS_SUCCESS != pstatus)
				continue;
		}
	}
	FUNCOUT_WSTS(pstatus);
	return pstatus;
}

static void phl_tx_res_query(struct phl_info_t *phl_info,
						u8 ch, u16 *host_idx, u16 *hw_idx, u16 *hw_res)
{
	*hw_res = rtw_hal_tx_res_query(phl_info->hal, ch, host_idx,
							hw_idx);
}


static void phl_tx_res_query_sw(struct phl_info_t *phl_info, void *txbd,
						u8 ch, u16 *host_idx, u16 *hw_idx, u16 *hw_res)
{
	*hw_res = rtw_hal_tx_res_query_sw(phl_info->hal, txbd, ch, host_idx,
							hw_idx);
}

static void phl_rx_res_query(struct phl_info_t *phl_info,
						u8 ch, u16 *host_idx, u16 *hw_idx, u16 *hw_res)
{
	*hw_res = rtw_hal_rx_res_query(phl_info->hal, ch, host_idx,
							hw_idx);
}

enum rtw_phl_status _phl_refill_rxbd(struct phl_info_t *phl_info,
					void* rx_buf_ring,
					struct rx_base_desc *rxbd,
					u8 ch, u16 refill_cnt)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct rtw_rx_buf *rxbuf = NULL;
	u16 cnt = 0;

	for (cnt = 0; cnt < refill_cnt; cnt++) {
		rxbuf = query_idle_rx_buf(phl_info, rx_buf_ring);
		if (NULL == rxbuf) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
				"[WARNING] there is no resource for rx bd refill setting\n");
			pstatus = RTW_PHL_STATUS_RESOURCE;
			break;
		}
		hstatus = rtw_hal_update_rxbd(phl_info->hal, rxbd, rxbuf, ch);
		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
				"[WARNING] update rxbd fail\n");
			pstatus = RTW_PHL_STATUS_FAILURE;
			break;
		}
		enqueue_busy_rx_buf(phl_info, rx_buf_ring, rxbuf, _tail);
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	#ifdef CONFIG_RTW_PLATFORM_NEED_MEMORY_FLUSH
	iob();
	#endif /* CONFIG_RTW_PLATFORM_NEED_MEMORY_FLUSH */

	if (cnt) {
		hstatus = rtw_hal_notify_rxdone(phl_info->hal, rxbd, ch, cnt);
		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
				"[WARNING] notify rxdone fail\n");
			pstatus = RTW_PHL_STATUS_FAILURE;
		}
	}
	return pstatus;
}

/*
 *  Hardware A-MSDU cut result:
 *
 *  No A-MSDU cut in progress
 *  	Result		Packet is	pphl_rx		phl_info->pending_amsdu
 *  			A-MSDU cut
 *  	SUCCESS		No		RX packet	NULL
 *  	ENQ		Yes		NULL		Pending phl_rx without last_amsdu.
 *  			(whdr valid)
 *  			Yes (single)	RX packet	NULL
 *  	FALSE				NULL		NULL
 *  			(whdr invalid)	NULL		NULL
 *  	SUCCESS		non-Wifi	PX packet	NULL
 *
 *  A-MSDU cut in progress
 *  			Packet is	pphl_rx		phl_info->pending_amsdu
 *  			A-MSDU cut
 *	SUCCESS		Yes (last)	Complete A-MSDU NULL
 *	SUCCESS		Yes (Another	Complete A-MSDU Pending phl_rx without last_amsdu.
 *			start)
 *			No (valid)	RX packet	Complete A-MSDU (Force)
 *			No		NULL		Complete A-MSDU (Force)
 *	ENQ		Yes (non-last)	NULL		Accumulated A-MSDU
 *  	SUCCESS		non-Wifi	PX packet	Accumulated A-MSDU
 *
 */
enum rtw_phl_status phl_get_single_rx(struct phl_info_t *phl_info,
					 struct rtw_rx_buf_ring *rx_buf_ring,
					 u8 ch,
					 struct rtw_phl_rx_pkt **pphl_rx)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct rtw_phl_rx_pkt *phl_rx = NULL;
	struct rtw_rx_buf *rxbuf = NULL;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_com);
	u16 buf_size = 0;
	u8 rx_rdy = 0;
	int cont;
	#ifdef DEBUG_PHL_RX
	struct phl_rx_stats *rx_stats = &phl_com->rx_stats;
	#endif

	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	phl_rx = phl_info->pending_amsdu;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	if (phl_rx == NULL || (ch != 0)) {
		phl_rx = rtw_phl_query_phl_rx(phl_info);
	}

	rxbuf = query_busy_rx_buf(phl_info, rx_buf_ring);

	do {
		cont = 0;
		if (NULL == phl_rx) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s(%d) phl_rx out of resource, ch=%d \n",
				__func__, __LINE__, ch);
				pstatus = RTW_PHL_STATUS_FAILURE;
			break;
		}
		if (NULL == rxbuf) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s(%d) [WARNING] queried NULL rxbuf, ch=%d \n",
				__func__, __LINE__, ch);
				pstatus = RTW_PHL_STATUS_FAILURE;
			#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
			if (phl_rx == phl_info->pending_amsdu)
				phl_rx = NULL;
			#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
			break;
		}

		/* If RX buffer is ready checked, not check it again. */
		if (rx_rdy == 0) {
			rx_rdy = rtw_hal_check_rxrdy(phl_com,
			                             phl_info->hal,
			                             rxbuf,
			                             ch);

			#ifdef DEBUG_PHL_RX
			rx_stats->rx_chk_cnt++;
			#endif

			if (true != rx_rdy) {
				#ifdef DEBUG_PHL_RX
				rx_stats->rx_rdy_fail++;
				#endif

				#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
				if (phl_info->pending_amsdu != NULL) {
					phl_rx = NULL;
				} else {

				}
				#endif /* CONFIG_HW_RX_AMSDU_CUT */

				PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "RX:%s(%d) packet not ready\n",
						__func__, __LINE__);
					pstatus = RTW_PHL_STATUS_FAILURE;
				break;
			}

			phl_com->rx_stats.rx_cnt++;

			if (true != rtw_hal_handle_rxbd_info(phl_info->hal,
							     rxbuf->vir_addr,
							     &buf_size)) {
				#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
				if (phl_info->pending_amsdu == phl_rx) {
					phl_rx = NULL;
					#ifdef CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT
					rx_stats->amsdu_cut_chk_drop++;
					#endif /* CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT */
				}
				#endif /* CONFIG_HW_RX_AMSDU_CUT */

				if (phl_rx) {
					#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
					INIT_LIST_HEAD(&phl_rx->r.rx_buf_lst_head);
					list_add_tail(&rxbuf->list, &phl_rx->r.rx_buf_lst_head);
					phl_rx->r.rx_buf = (struct rtw_rx_buf_base *)rxbuf;
					#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
					phl_rx->rxbuf_ptr = (u8 *)rxbuf;
				}
				#ifdef DEBUG_PHL_RX
				rx_stats->rxbd_fail++;
				#endif

				goto drop;
			}

			#ifdef CONFIG_RTW_DEBUG_RX_SZ
			if (phl_rx->type != RTW_RX_TYPE_TX_WP_RELEASE_HOST) {
				u32 rx_sz = (4 + buf_size);

				if (   (phl_info->min_rx_sz == 0)
				    || (phl_info->min_rx_sz > rx_sz)) {
					phl_info->min_rx_sz = rx_sz;
					phl_info->min_rx_type = phl_rx->type;
				}

				if ((4 + buf_size) > phl_info->max_rx_sz)
					phl_info->max_rx_sz = rx_sz;
			} else {
				u32 rx_sz = (4 + buf_size);

				if (   (phl_info->min_rp_sz == 0)
				    || (phl_info->min_rp_sz > rx_sz))
					phl_info->min_rp_sz = rx_sz;

				if (rx_sz > phl_info->max_rp_sz)
					phl_info->max_rp_sz = rx_sz;
			}
			#endif /* CONFIG_RTW_DEBUG_RX_SZ */
		}

		#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
		if (   (ch == 0)
		    && phl_info->pending_amsdu
		    && !phl_info->pending_amsdu->r.mdata.last_msdu) {
			hstatus = rtw_hal_check_next_msdu(phl_info->phl_com,
			                                  phl_info->hal,
			                                  rxbuf,
			                                  buf_size,
			                                  phl_info->pending_amsdu);
			if (RTW_HAL_STATUS_SUCCESS == hstatus) {
				/* Put this RX buffer to pending A-MSDU */
				list_add_tail(&rxbuf->list, &phl_rx->r.rx_buf_lst_head);

				#ifdef DEBUG_PHL_RX
				rx_stats->amsdu_cut_cur_num++;
				#endif /* DEBUG_PHL_RX */

				/* A-MSDU done */
				if (phl_rx->r.mdata.last_msdu) {
					#ifdef DEBUG_PHL_RX
					rx_stats->amsdu_cut_ok++;
					if (  rx_stats->amsdu_cut_max_num
					    < rx_stats->amsdu_cut_cur_num)
						rx_stats->amsdu_cut_max_num
							= rx_stats->amsdu_cut_cur_num;
					#endif /* DEBUG_PHL_RX */

					phl_info->pending_amsdu = NULL;
					*pphl_rx = phl_rx;

					if (phl_rx->r.mdata.debug_dump)
						PHL_PRINT("RC %u\n\n", rx_stats->amsdu_cut_cur_num);

					pstatus = RTW_PHL_STATUS_SUCCESS;
				} else {
					/* A-MSDU in the middle */
					*pphl_rx = NULL;
					pstatus = RTW_PHL_STATUS_FRAME_ENQ;
				}

				return pstatus;
			} else if (RTW_HAL_STATUS_FAILURE == hstatus) {
				/* A-MSDU break. Mark A-MSDU finish and process this
				 * frame as a new one */
				phl_rx->r.mdata.last_msdu = 1;

				#ifdef DEBUG_PHL_RX
				rx_stats->amsdu_cut_break++;
				if (  rx_stats->amsdu_cut_max_num
				    < rx_stats->amsdu_cut_cur_num)
					rx_stats->amsdu_cut_max_num
						= rx_stats->amsdu_cut_cur_num;

				PHL_PRINT("A-MSDU cut break %u!\n", rx_stats->amsdu_cut_cur_num);
				#endif /* DEBUG_PHL_RX */

				/* Get new phl_rx for current rx_buf that
				 * is not part of the pendng A-MSDU */
				phl_rx = rtw_phl_query_phl_rx(phl_info);
				cont = 1;
				continue;
			}
			/* non wifi packet received. Process it */
			phl_rx = &phl_info->tmp_phl_rx;
		}
		#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
		{
			phl_rx->rxbuf_ptr = (u8 *)rxbuf;
			#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
			INIT_LIST_HEAD(&phl_rx->r.rx_buf_lst_head);
			list_add_tail(&rxbuf->list, &phl_rx->r.rx_buf_lst_head);
			phl_rx->r.rx_buf = (struct rtw_rx_buf_base *)rxbuf;
			#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
			#ifdef CONFIG_DYNAMIC_RX_BUF
			phl_rx->r.os_priv = rxbuf->os_priv;
			#endif

			hstatus = rtw_hal_handle_rx_buffer(phl_info->phl_com,
							   phl_info->hal,
							   rxbuf,
							   buf_size, phl_rx);

			if (RTW_HAL_STATUS_SUCCESS != hstatus) {
				PHL_ERR("%s: handle_rx_buffer %u\n", __FUNCTION__,
					hstatus);
				goto drop;
			}
		}

		#if defined(CONFIG_RTW_HW_RX_AMSDU_CUT) && !defined(CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV)
		/* Incomplete A-MSDU, return drop to continue to next */
		if (   (phl_rx->type == RTW_RX_TYPE_WIFI)
		    && phl_rx->r.mdata.amsdu_cut
		    && !phl_rx->r.mdata.last_msdu) {
			struct rtw_phl_rx_pkt *pending_amsdu;

			pending_amsdu = phl_info->pending_amsdu;

			phl_info->pending_amsdu = phl_rx;
			#ifdef DEBUG_PHL_RX
			phl_com->rx_stats.amsdu_cut_cur_num = 1;
			#endif /* DEBUG_PHL_RX */

			if (pending_amsdu) {
				pstatus = RTW_PHL_STATUS_SUCCESS;
				*pphl_rx = pending_amsdu;
			} else {
				pstatus = RTW_PHL_STATUS_FRAME_ENQ;
				*pphl_rx = NULL;
			}

			return pstatus;
		}
		#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
		pstatus = RTW_PHL_STATUS_SUCCESS;
	} while (cont);

	if (   (RTW_PHL_STATUS_SUCCESS != pstatus)
	    #ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	    && (RTW_PHL_STATUS_FRAME_ENQ != pstatus)
	    #endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	   ) {
		if (NULL != rxbuf) {
			#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
			if (rx_rdy) {
				rtw_hal_rewind_rx_tag(phl_com, phl_info->hal, ch);
			}
			RTW_PRINT("C%u PB\n", ch);
			#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
			enqueue_busy_rx_buf(phl_info, rx_buf_ring, rxbuf, _first);
		}

		if (NULL != phl_rx) {
			phl_release_phl_rx(phl_info, phl_rx);
			phl_rx = NULL;
		}
	}
	*pphl_rx = phl_rx;

	return pstatus;

drop:
#ifdef DEBUG_PHL_RX
	phl_com->rx_stats.rx_drop_get++;
#endif
	if (phl_rx) {
		phl_rx->r.mdata.dma_ch = ch;

		#ifdef CONFIG_DYNAMIC_RX_BUF
		/* avoid re-allocating buffer carried on rxbuf */
		phl_rx->type = RTW_RX_TYPE_MAX;
		#endif

		phl_recycle_rx_buf(phl_info, phl_rx);
	} else if (rxbuf) {
		struct phl_hci_trx_ops *hci_trx_ops;

		hci_trx_ops = phl_info->hci_trx_ops;

		if (hci_trx_ops->recycle_rx_buf)
			pstatus = hci_trx_ops->recycle_rx_buf(phl_info, rxbuf,
			                                      ch, RTW_RX_TYPE_MAX);
	}

	return RTW_PHL_STATUS_FRAME_DROP;
}


void phl_rx_handle_normal(struct phl_info_t *phl_info,
			  struct rtw_phl_rx_pkt *phl_rx)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	_os_list frames;
#ifdef CONFIG_DYNAMIC_RX_BUF
	struct rtw_rx_buf *rxbuf = (struct rtw_rx_buf *)phl_rx->rxbuf_ptr;
#endif
	FUNCIN_WSTS(pstatus);
	INIT_LIST_HEAD(&frames);

#if defined(CONFIG_DYNAMIC_RX_BUF) && defined(PHL_DMA_IOMMU)
	/* Not need to unmap RX buffer for non IOMMU platforms */
	_os_pkt_buf_unmap_rx(phl_to_drvpriv(phl_info), rxbuf->phy_addr_l,
	                     rxbuf->phy_addr_h, rxbuf->buf_len,
	                     rxbuf->os_priv);
#endif /* CONFIG_DYNAMIC_RX_BUF && PHL_DMA_IOMMU */

	/* stat : rx rate counter */
	if (phl_rx->r.mdata.rx_rate <= RTW_DATA_RATE_HE_NSS4_MCS11)
		phl_info->phl_com->phl_stats.rx_rate_nmr[phl_rx->r.mdata.rx_rate]++;

	pstatus = phl_rx_reorder(phl_info, phl_rx, &frames);
	if (pstatus == RTW_PHL_STATUS_SUCCESS)
		phl_handle_rx_frame_list(phl_info, &frames);
	else
		PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_, "[WARNING]handle normal rx error (0x%08X)!\n", pstatus);

	FUNCOUT_WSTS(pstatus);
}

void _phl_wp_rpt_statistics(struct phl_info_t *phl_info, u8 ch, u16 wp_seq,
			    u8 txsts, struct rtw_xmit_req *treq)
{
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	struct rtw_wp_rpt_stats *rpt_stats = NULL;
	u32 diff_t = 0, cur_time = _os_get_cur_time_ms();

	rpt_stats = (struct rtw_wp_rpt_stats *)hal_com->trx_stat.wp_rpt_stats;

	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		  "recycle tx req(%p) in ch(%d) with wp_seq(0x%x) and wifi seq(%d)!\n",
		  treq, ch, wp_seq, treq->mdata.sw_seq);

	if (cur_time >= treq->tx_time)
		diff_t = cur_time - treq->tx_time;
	else
		diff_t = RTW_U32_MAX - treq->tx_time + cur_time;

	if (diff_t > WP_DELAY_THRES_MS) {
		if (TX_STATUS_TX_DONE == txsts)
			rpt_stats[ch].delay_tx_ok_cnt++;
		else if (TX_STATUS_TX_FAIL_REACH_RTY_LMT == txsts)
			rpt_stats[ch].delay_rty_fail_cnt++;
		else if (TX_STATUS_TX_FAIL_LIFETIME_DROP == txsts)
			rpt_stats[ch].delay_lifetime_drop_cnt++;
		else if (TX_STATUS_TX_FAIL_MACID_DROP == txsts)
			rpt_stats[ch].delay_macid_drop_cnt++;
	} else {
		if (TX_STATUS_TX_DONE == txsts)
			rpt_stats[ch].tx_ok_cnt++;
		else if (TX_STATUS_TX_FAIL_REACH_RTY_LMT == txsts)
			rpt_stats[ch].rty_fail_cnt++;
		else if (TX_STATUS_TX_FAIL_LIFETIME_DROP == txsts)
			rpt_stats[ch].lifetime_drop_cnt++;
		else if (TX_STATUS_TX_FAIL_MACID_DROP == txsts)
			rpt_stats[ch].macid_drop_cnt++;
	}

	if (txsts != TX_STATUS_TX_DONE && txsts != TX_STATUS_TX_FAIL_SW_DROP) {
		rpt_stats[ch].tx_fail_cnt++;
	}
}


void _phl_wp_rpt_chk_txsts(struct phl_info_t *phl_info, u8 ch, u16 wp_seq,
			    u8 txsts, struct rtw_xmit_req *treq)
{
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	struct rtw_wp_rpt_stats *rpt_stats = NULL;
	struct rtw_pkt_buf_list *pkt_buf = (struct rtw_pkt_buf_list *)treq->pkt_list;
	int i;

	rpt_stats = (struct rtw_wp_rpt_stats *)hal_com->trx_stat.wp_rpt_stats;

	if(TX_STATUS_TX_DONE != txsts) {
		if (TX_STATUS_TX_FAIL_REACH_RTY_LMT == txsts) {
			PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_,
				"(REACH_RTY_LMT) : (%p) in ch(%d) with wp_seq(0x%x) and wifi seq(%d)!\n",
				treq, ch, wp_seq, treq->mdata.sw_seq);
		}
		else if (TX_STATUS_TX_FAIL_LIFETIME_DROP == txsts) {
			PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_,
				"(LIFETIME_DROP) : (%p) in ch(%d) with wp_seq(0x%x) and wifi seq(%d)!\n",
				treq, ch, wp_seq, treq->mdata.sw_seq);
		}
		else if (TX_STATUS_TX_FAIL_MACID_DROP == txsts) {
			PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_,
				"(MACID_DROP) : (%p) in ch(%d) with wp_seq(0x%x) and wifi seq(%d)!\n",
				treq, ch, wp_seq, treq->mdata.sw_seq);
		}
		else if(TX_STATUS_TX_FAIL_SW_DROP == txsts) {
			PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_,
				"(SW_DROP) : (%p) in ch(%d) with wp_seq(0x%x) and wifi seq(%d)!\n",
				treq, ch, wp_seq, treq->mdata.sw_seq);
		}

		/* dump tx fail mac hdr */
		if(MAC_HDR_LEN <= pkt_buf[0].length) {
			PHL_DATA(COMP_PHL_XMIT, _PHL_INFO_, "=== Dump Tx MAC HDR ===");
			for (i = 0; i < MAC_HDR_LEN; i++) {
				if (!(i % 8))
					PHL_DATA(COMP_PHL_XMIT, _PHL_INFO_, "\n");
				PHL_DATA(COMP_PHL_XMIT, _PHL_INFO_, "%02X ", pkt_buf[0].vir_addr[i]);
			}
			PHL_DATA(COMP_PHL_XMIT, _PHL_INFO_, "\n");
		}
	}

	if (treq->txfb) {
		treq->txfb->txsts = txsts;
		if (treq->txfb->txfb_cb)
			treq->txfb->txfb_cb(treq->txfb);
	}
}

void phl_recycle_payload(struct phl_info_t *phl_info, u8 dma_ch, u16 wp_seq,
			 u8 txsts, u8 code)
{
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	struct rtw_wp_rpt_stats *rpt_stats =
		(struct rtw_wp_rpt_stats *)hal_com->trx_stat.wp_rpt_stats;
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_phl_evt_ops *ops = &phl_info->phl_com->evt_ops;
	struct rtw_wd_page_ring *wd_ring = NULL;
	struct rtw_xmit_req *treq = NULL;
	u16 macid = 0;
	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	#endif
	u8 tid;

#ifdef CONFIG_VW_REFINE
	u8 my_vw_cnt =0;
#endif

	wd_ring = (struct rtw_wd_page_ring *)hci_info->wd_ring;
	treq = (struct rtw_xmit_req *)wd_ring[dma_ch].wp_tag[wp_seq].ptr;

	if (NULL == treq)
		goto end;

	macid = treq->mdata.macid;
	tid = treq->mdata.tid;

	_phl_wp_rpt_statistics(phl_info, dma_ch, wp_seq, txsts, treq);
	_phl_wp_rpt_chk_txsts(phl_info, dma_ch, wp_seq, txsts, treq);
#ifdef CONFIG_VW_REFINE
	my_vw_cnt = treq->vw_cnt;
#endif

	#ifdef RTW_WKARD_HIQ_STUCK
	if ((code == 0) && (dma_ch == 9)) {
		phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, macid);

		if (phl_sta != NULL
		    && phl_sta->wrole
		    && phl_sta->wrole->active) {
			struct rtw_wifi_role_t *wrole = phl_sta->wrole;
			void *drv_priv = phl_info->phl_com->drv_priv;
			u32 busy_hiq_cnt;

			busy_hiq_cnt = _os_atomic_dec_return(drv_priv, &wrole->hiq_busy_cnt);
			wrole->last_hiq_wp_time = _os_get_cur_time_ms();

			if (wrole->dropping_hiq) {
				if (wrole->dropping_hiq-- > 1) {
					PHL_PRINT("HiQ RC M%u S%u L%u\n", wrole->hw_mbssid,
					          txsts, treq->mdata.pktlen);
				}

 				if (busy_hiq_cnt == 0) {
					rtw_hal_rel_hiq_drop(phl_info->hal,
						             wrole->hw_port,
						             wrole->hw_mbssid);
					wrole->dropping_hiq = 0;
				}
			}
		}
	}
	#endif /* RTW_WKARD_HIQ_STUCK */

	if (RTW_PHL_TREQ_TYPE_TEST_PATTERN == treq->treq_type) {
		if (NULL == ops->tx_test_recycle)
			goto end;
		PHL_INFO("call tx_test_recycle\n");
		sts = ops->tx_test_recycle(phl_info, treq);
	} else if (RTW_PHL_TREQ_TYPE_NORMAL == treq->treq_type
#if defined(CONFIG_CORE_TXSC) || defined(CONFIG_PHL_TXSC)
		   || RTW_PHL_TREQ_TYPE_CORE_TXSC == treq->treq_type
		   || RTW_PHL_TREQ_TYPE_PHL_ADD_TXSC == treq->treq_type
#endif
	) {
		if (NULL == ops->tx_recycle)
			goto end;
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "call tx_recycle\n");
		/* Save TX status to no_ack for packet trace or keep alive
		   TX status check */
		treq->mdata.no_ack = txsts;
		sts = ops->tx_recycle(phl_to_drvpriv(phl_info), treq);
	}

end:
	if (RTW_PHL_STATUS_SUCCESS != sts) {
		PHL_WARN("tx recycle fail\n");
		rpt_stats[dma_ch].recycle_fail_cnt++;
	} else {
		#ifdef CONFIG_WFA_OFDMA_Logo_Test
		phl_sta = rtw_phl_get_stainfo_by_macid(phl_info, macid);
		if(phl_sta && tid == 0)
			_os_atomic_dec(drv_priv, &phl_sta->cnt_current_wd);
		#endif
		_os_spinlock(phl_to_drvpriv(phl_info),
			     &wd_ring[dma_ch].wp_tag_lock,
			     _bh, NULL);
		wd_ring[dma_ch].wp_tag[wp_seq].ptr = NULL;
		rpt_stats[dma_ch].busy_cnt--;

#ifdef RTW_WKARD_DYNAMIC_LTR
		if (true ==
		    _phl_judge_idle_ltr_switching_conditions(phl_info, macid))
			_phl_switch_idle_ltr(phl_info, rpt_stats);
#endif
#ifdef CONFIG_VW_REFINE
		if ( 0 == dma_ch )  {
			phl_info->free_wp[phl_info->fw_ptr] = wp_seq;
			phl_info->fw_ptr = (phl_info->fw_ptr + 1) % WP_MAX_SEQ_NUMBER;
		}
		if ( 0 == code)
			phl_info->vw_cnt_rev += my_vw_cnt;
		else
			phl_info->vw_cnt_err += my_vw_cnt;
#endif
		_os_spinunlock(phl_to_drvpriv(phl_info),
			       &wd_ring[dma_ch].wp_tag_lock,
			       _bh, NULL);
	}
	/* phl_indic_pkt_complete(phl_info); */
}

void _phl_rx_handle_wp_report(struct phl_info_t *phl_info,
                              struct rtw_phl_rx_pkt *phl_rx)
{
	struct rtw_recv_pkt *r = &phl_rx->r;
	u8 *pkt = NULL;
	u16 pkt_len = 0;
	u16 wp_seq = 0, rsize = 0;
	u8 sw_retry = 0, dma_ch = 0, txsts = 0;
	u8 macid = 0, ac_queue = 0;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	pkt = r->rx_buf->frame;
	pkt_len = r->rx_buf->frame_len;
	#else
	pkt = r->pkt_list[0].vir_addr;
	pkt_len = r->pkt_list[0].length;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	while (pkt_len > 0) {
		rsize = rtw_hal_handle_wp_rpt(phl_info->hal, pkt, pkt_len,
					      &sw_retry, &dma_ch, &wp_seq,
					      &macid, &ac_queue, &txsts);

		if (0 == rsize)
			break;

		#ifdef CONFIG_PHL_RELEASE_RPT_ENABLE
		phl_rx_wp_report_record_sts(phl_info, macid, ac_queue, txsts);
		#endif

		if (false == sw_retry) {
			phl_recycle_payload(phl_info, dma_ch, wp_seq, txsts, 0);
		} else {
			/* hana_todo handle sw retry */
			phl_recycle_payload(phl_info, dma_ch, wp_seq, txsts, 0);
		}
		pkt += rsize;
		pkt_len -= rsize;
	}
}


static void phl_rx_process_pcie(struct phl_info_t *phl_info,
							struct rtw_phl_rx_pkt *phl_rx)
{
#ifdef DEBUG_PHL_RX
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;

	phl_com->rx_stats.rx_type_all++;
#endif

	switch (phl_rx->type) {
	case RTW_RX_TYPE_WIFI:
#ifdef DEBUG_PHL_RX
		phl_com->rx_stats.rx_type_wifi++;
		#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
		if (phl_rx->r.rx_buf->frame_len == phl_com->cnt_rx_pktsz)
			phl_com->rx_stats.rx_pktsz_phl++;
		if (phl_rx->r.mdata.amsdu_cut)
			phl_com->rx_stats.rx_amsdu_cut++;
		if (phl_rx->r.mdata.hdr_conv)
			phl_com->rx_stats.rx_hdr_conv++;
		#else
		if (phl_rx->r.pkt_list[0].length == phl_com->cnt_rx_pktsz)
			phl_com->rx_stats.rx_pktsz_phl++;
		#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
		if (phl_rx->r.mdata.amsdu)
			phl_com->rx_stats.rx_amsdu++;
#endif
#ifdef CONFIG_PHL_RX_PSTS_PER_PKT
		if (false == phl_rx_proc_wait_phy_sts(phl_info, phl_rx)) {
			PHL_TRACE(COMP_PHL_PSTS, _PHL_DEBUG_,
				  "phl_rx_proc_wait_phy_sts() return false \n");
			phl_rx_handle_normal(phl_info, phl_rx);
		}
#else
		phl_rx_handle_normal(phl_info, phl_rx);
#endif
		break;
	case RTW_RX_TYPE_TX_WP_RELEASE_HOST:
#ifdef DEBUG_PHL_RX
		phl_com->rx_stats.rx_type_wp++;
#endif
		_phl_rx_handle_wp_report(phl_info, phl_rx);
		phl_recycle_rx_buf(phl_info, phl_rx);
		break;
	case RTW_RX_TYPE_PPDU_STATUS:
#ifdef DEBUG_PHL_RX
		phl_com->rx_stats.rx_type_ppdu++;
#endif
		phl_rx_proc_ppdu_sts(phl_info, phl_rx);
#ifdef CONFIG_PHL_RX_PSTS_PER_PKT
		phl_rx_proc_phy_sts(phl_info, phl_rx);
#endif
		phl_recycle_rx_buf(phl_info, phl_rx);
		break;
	case RTW_RX_TYPE_C2H:
#ifdef DEBUG_PHL_RX
		phl_com->rx_stats.rx_type_c2h++;
#endif
		phl_recycle_rx_buf(phl_info, phl_rx);
		break;
	case RTW_RX_TYPE_CHANNEL_INFO:
	case RTW_RX_TYPE_TX_RPT:
	case RTW_RX_TYPE_DFS_RPT:
	case RTW_RX_TYPE_MAX:
		PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_, "phl_rx_process_pcie(): Unsupported case:%d, please check it\n",
				phl_rx->type);
		phl_recycle_rx_buf(phl_info, phl_rx);
		break;
	default :
		PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_, "[WARNING] unrecognize rx type\n");
		phl_recycle_rx_buf(phl_info, phl_rx);
		break;
	}
}

static u16 _phl_get_idle_rxbuf_cnt(struct phl_info_t *phl_info,
					struct rtw_rx_buf_ring *rx_buf_ring)
{
	return rx_buf_ring->idle_rxbuf_cnt;
}

static enum rtw_phl_status phl_rx_pcie(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_rx_buf_ring *rx_buf_ring = NULL;
	struct rx_base_desc *rxbd = NULL;
	struct rtw_phl_rx_pkt *phl_rx = NULL;
	u16 i = 0, rxcnt = 0, host_idx = 0, hw_idx = 0, idle_rxbuf_cnt = 0;
	u8 ch = 0;
	#ifdef DEBUG_PHL_RX
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct phl_rx_stats *rx_stats = &phl_com->rx_stats;
	#endif /* DEBUG_PHL_RX */
	void *drvpriv = phl_to_drvpriv(phl_info);

	FUNCIN_WSTS(pstatus);

#ifdef RTW_WKARD_AP_RESET_RX_LOCK
	_os_spinlock(phl_to_drvpriv(phl_info), &phl_info->rx_lock, _bh, NULL);
#endif

#ifdef DEBUG_PHL_RX
	rx_stats->phl_rx++;
#endif

	rx_buf_ring = (struct rtw_rx_buf_ring *)hci_info->rxbuf_pool;
	rxbd = (struct rx_base_desc *)hci_info->rxbd_buf;

#ifdef CONFIG_DYNAMIC_RX_BUF
	/* The empty rxbuf (w/o available buffer) happen only on RTW_RX_TYPE_WIFI  */
	refill_empty_rx_buf(phl_info, &rx_buf_ring[0]);
#endif

	for (ch = 0; ch < hci_info->total_rxch_num; ch++) {

		idle_rxbuf_cnt = _phl_get_idle_rxbuf_cnt(phl_info, &rx_buf_ring[ch]);

		if (idle_rxbuf_cnt == 0) {
#ifdef DEBUG_PHL_RX
			rx_stats->idle_rxbuf_empty++;
#else
			PHL_WARN("%s, idle rxbuf is empty.\n", __func__);
#endif
#ifdef PHL_RX_BATCH_IND
			if (ch == 0)
				_phl_indic_new_rxpkt(phl_info);
#endif
			pstatus = RTW_PHL_STATUS_SUCCESS;
			continue;
		}
#ifdef PHL_RXSC_ISR
		if(!rtw_hal_rx_isr_check(phl_info->hal, ch)){
			pstatus = RTW_PHL_STATUS_SUCCESS;
			continue;
		}
#endif
		rxcnt = rtw_hal_rx_res_query(phl_info->hal, ch, &host_idx, &hw_idx);

		if (rxcnt == 0) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_,
				"no avail hw rx\n");
			pstatus = RTW_PHL_STATUS_SUCCESS;
			continue;
		}

		/* only handle affordable amount of rxpkt */
		if (rxcnt > idle_rxbuf_cnt) {
#ifdef DEBUG_PHL_RX
			rx_stats->idle_rxbuf_less++;
#else
			PHL_WARN("rxcnt %d is lager than idle rxbuf cnt %d.\n", rxcnt, idle_rxbuf_cnt);
#endif
			rxcnt = idle_rxbuf_cnt;
		}

		for (i = 0; i < rxcnt; i++) {
			if (   PHL_RX_STATUS_SW_PAUSE
			    == _os_atomic_read(drvpriv,
			                       &phl_info->phl_sw_rx_sts)) {
				rxcnt = i;
				break;
			}

			pstatus = phl_get_single_rx(phl_info, &rx_buf_ring[ch],
							ch, &phl_rx);
			if ((RTW_PHL_STATUS_FRAME_DROP == pstatus)
			    #ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
			    || (RTW_PHL_STATUS_FRAME_ENQ == pstatus)
			    #endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
			   )
				continue;

			#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
			/* Check finished pending AMSDU and process it */
			if (phl_info->pending_amsdu
			    && phl_info->pending_amsdu->r.mdata.last_msdu) {
				struct rtw_phl_rx_pkt *amsdu = phl_info->pending_amsdu;
				phl_info->pending_amsdu = NULL;
				amsdu->r.mdata.dma_ch = ch;
				phl_rx_process_pcie(phl_info, amsdu);
				#ifdef DEBUG_PHL_RX
				rx_stats->amsdu_cut_proc_pending++;
				#endif
			}
			#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

			if (NULL == phl_rx) {
				#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
				if (phl_info->pending_amsdu != NULL)
					continue;
				#endif
				rxcnt = i;
				break;
			}

			/* hana_todo */
			phl_rx->r.mdata.dma_ch = ch;
			phl_rx_process_pcie(phl_info, phl_rx);
		}

#ifdef PHL_RX_BATCH_IND
		if (ch == 0 && phl_info->rx_new_pending) {
			_phl_indic_new_rxpkt(phl_info);
		}
#endif

		pstatus = _phl_refill_rxbd(phl_info, &rx_buf_ring[ch],
							&rxbd[ch], ch, rxcnt);

		if (RTW_PHL_STATUS_RESOURCE == pstatus) {
			#ifdef DEBUG_PHL_RX
			rx_stats->rxbd_refill_fail++;
			#endif

			PHL_WARN("%s, rxcnt is not refilled %d.\n", __func__ , rxcnt);
		}

		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			continue;
	}

#ifdef RTW_WKARD_AP_RESET_RX_LOCK
	_os_spinunlock(phl_to_drvpriv(phl_info), &phl_info->rx_lock, _bh, NULL);
#endif
	FUNCOUT_WSTS(pstatus);

	return pstatus;
}

enum rtw_phl_status phl_pltfm_tx_pcie(struct phl_info_t *phl_info, void *pkt)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct rtw_h2c_pkt *h2c_pkt = (struct rtw_h2c_pkt *)pkt;
	struct tx_base_desc *txbd = NULL;
	struct phl_h2c_pkt_pool *h2c_pool = NULL;
	struct rtw_wd_page wd;
	u8 fwcmd_queue_idx = 0;
	struct rtw_wd_page_list phl_wd_list;
	u16 wd_num = 1;

	txbd = (struct tx_base_desc *)phl_info->hci->txbd_buf;
	h2c_pool = (struct phl_h2c_pkt_pool *)phl_info->h2c_pool;

	_os_mem_set(phl_to_drvpriv(phl_info), &wd, 0, sizeof(wd));
	/* fowart h2c pkt information into the format of wd page */
	wd.phy_addr_l = h2c_pkt->phy_addr_l + (u32)(h2c_pkt->vir_data - h2c_pkt->vir_head);
	wd.phy_addr_h= h2c_pkt->phy_addr_h;
	wd.vir_addr = h2c_pkt->vir_data;
	wd.buf_len = h2c_pkt->data_len;
	wd.ls = 1;

	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : wd.phy_addr_l %x, wd.phy_addr_h %x\n", __func__ , wd.phy_addr_l, wd.phy_addr_h);
	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : buf_len %x.\n", __func__ , wd.buf_len);

	fwcmd_queue_idx = rtw_hal_get_fwcmd_queue_idx(phl_info->hal);

	phl_wd_list.wd_page[0] = &wd;
	_os_spinlock(phl_to_drvpriv(phl_info), &txbd[fwcmd_queue_idx].txbd_lock, _bh, NULL);
	hstatus = rtw_hal_update_txbd(phl_info->hal, txbd, &phl_wd_list, fwcmd_queue_idx, wd_num);

	h2c_pkt->host_idx = wd.host_idx;

	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : h2c_pkt->host_idx %d.\n", __func__, h2c_pkt->host_idx);

	if (RTW_HAL_STATUS_SUCCESS == hstatus)
			pstatus = phl_enqueue_busy_h2c_pkt(phl_info, h2c_pkt, _tail);

	#ifdef CONFIG_RTW_PLATFORM_NEED_MEMORY_FLUSH
	iob();
	#endif /* CONFIG_RTW_PLATFORM_NEED_MEMORY_FLUSH */

	if (RTW_PHL_STATUS_SUCCESS == pstatus) {
		hstatus = rtw_hal_trigger_txstart(phl_info->hal, txbd, fwcmd_queue_idx);
		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]update Tx RW ptr fail!\n");
			pstatus = RTW_PHL_STATUS_FAILURE;
		}
	}

	_os_spinunlock(phl_to_drvpriv(phl_info), &txbd[fwcmd_queue_idx].txbd_lock, _bh, NULL);

	return pstatus;
}

void *phl_get_txbd_buf_pcie(struct phl_info_t *phl_info)
{
	struct hci_info_t *hci_info = phl_info->hci;

	return hci_info->txbd_buf;
}

void *phl_get_rxbd_buf_pcie(struct phl_info_t *phl_info)
{
	struct hci_info_t *hci_info = phl_info->hci;

	return hci_info->rxbd_buf;
}

void phl_recycle_rx_pkt_pcie(struct phl_info_t *phl_info,
				struct rtw_phl_rx_pkt *phl_rx)
{
#ifdef CONFIG_DYNAMIC_RX_BUF
	struct rtw_rx_buf *rx_buf = (struct rtw_rx_buf *)phl_rx->rxbuf_ptr;
#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	struct rtw_rx_buf *i, *j;
	struct list_head *rx_buf_lst = &phl_rx->r.rx_buf_lst_head;

	phl_list_for_loop_safe(i, j, struct rtw_rx_buf, rx_buf_lst, list) {
		i->reuse = true;
	}
#else
	rx_buf->reuse = true;
#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

#endif

	phl_recycle_rx_buf(phl_info, phl_rx);
}

enum rtw_phl_status phl_register_trx_hdlr_pcie(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_handler *tx_handler = &phl_info->phl_tx_handler;
	struct rtw_phl_handler *rx_handler = &phl_info->phl_rx_handler;
	void *drv_priv = phl_to_drvpriv(phl_info);

	tx_handler->type = RTW_PHL_HANDLER_PRIO_HIGH; /* tasklet */
	tx_handler->callback = _phl_tx_callback_pcie;
	tx_handler->context = phl_info;
	tx_handler->drv_priv = drv_priv;
	pstatus = phl_register_handler(phl_info->phl_com, tx_handler);
	if (RTW_PHL_STATUS_SUCCESS != pstatus)
		PHL_ERR("%s : register tx_handler fail.\n", __FUNCTION__);

	rx_handler->type = RTW_PHL_HANDLER_PRIO_HIGH;
	rx_handler->callback = _phl_rx_callback_pcie;
	rx_handler->context = phl_info;
	rx_handler->drv_priv = drv_priv;
	pstatus = phl_register_handler(phl_info->phl_com, rx_handler);
	if (RTW_PHL_STATUS_SUCCESS != pstatus)
		PHL_ERR("%s : register rx_handler fail.\n", __FUNCTION__);

	return pstatus;
}

void
phl_tx_watchdog_pcie(struct phl_info_t *phl_info)
{
	struct rtw_stats *phl_stats = NULL;

	phl_stats = &phl_info->phl_com->phl_stats;

	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		  "\n=== Tx statistics === \n");
	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		"\nunicast tx bytes	: %llu\n", phl_stats->tx_byte_uni);
	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		"total tx bytes		: %llu\n", phl_stats->tx_byte_total);
	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		 "tx throughput		: %d(kbps)\n",
			 (int)phl_stats->tx_tp_kbits);
	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		"last tx time		: %d(ms)\n",
			 (int)phl_stats->last_tx_time_ms);
	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		"tx request num to phl	: %d\n",
			 (int)phl_stats->txreq_num);

	#ifdef RTW_WKARD_DYNAMIC_LTR
	if (rtw_hal_ltr_is_sw_ctrl(phl_info->hal)) {
		PHL_INFO(
			"ltr sw ctrl 			: %u\n",
			rtw_hal_ltr_is_sw_ctrl(phl_info->hal) == true ? 1 : 0);
		PHL_INFO(
			"ltr current state 		: %u\n",
			rtw_hal_ltr_get_cur_state(phl_info->hal));
		PHL_INFO(
			"ltr active trigger cnt : %lu\n",
			rtw_hal_ltr_get_tri_cnt(phl_info->hal, RTW_PCIE_LTR_SW_ACT));
		PHL_INFO(
			"ltr idle trigger cnt   : %lu\n",
			rtw_hal_ltr_get_tri_cnt(phl_info->hal, RTW_PCIE_LTR_SW_IDLE));
		PHL_INFO(
			"ltr last trigger time  : %lu\n",
			rtw_hal_ltr_get_last_trigger_time(phl_info->hal));
	}
	#endif

	_phl_dump_wp_stats(phl_info);
	_phl_dump_busy_wp(phl_info);

	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		  "\n===================== \n");

}

#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
void phl_dump_link_list(void *phl, _os_list *list_head, u8 type)
{
	struct rtw_wd_page *wd_page = NULL, *t = NULL;
	struct rtw_h2c_pkt *h2c_pkt = NULL, *h2c_t = NULL;
	struct rtw_phl_tring_list *phl_tring_list = NULL, *phl_t = NULL;
	struct phl_ring_status *ring_sts = NULL, *rsts_t = NULL;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	u8 *vir_addr = NULL;
	u32 i = 0, j = 0;
	u16 phl_idx = 0, phl_next_idx = 0;

	switch (type) {
	case TYPE_WD_PAGE:
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "===Dump WD Page===\n");
		phl_list_for_loop_safe(wd_page, t, struct rtw_wd_page,
						list_head, list) {
			vir_addr = (u8 *)wd_page->vir_addr;
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "vir_addr = %p, %x; phy_addr_l = %x; phy_addr_h = %x\n",
					vir_addr, *vir_addr,
					wd_page->phy_addr_l,
					wd_page->phy_addr_h);
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "cache = %d; buf_len = %d, wp_seq = %d\n",
					wd_page->cache, wd_page->buf_len,
					wd_page->wp_seq);
		}
		break;
	case TYPE_PHL_RING:
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "===Dump PHL Ring===\n");
		phl_list_for_loop_safe(phl_tring_list, phl_t,
					struct rtw_phl_tring_list,
					list_head, list) {

			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
				"-- macid = %d, band = %d, wmm = %d --\n",
					phl_tring_list->macid,
					phl_tring_list->band,
					phl_tring_list->wmm);

			for (i = 0; i < MAX_PHL_RING_CAT_NUM; i++) {
				phl_idx = (u16)_os_atomic_read(drv_priv,
						&phl_tring_list->phl_ring[i].phl_idx);
				phl_next_idx = (u16)_os_atomic_read(drv_priv,
						&phl_tring_list->phl_ring[i].phl_next_idx);

				PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
						"cat = %d\n"
						"dma_ch = %d\n"
						"tx_thres = %d\n"
						"core_idx = %d\n"
						"phl_idx = %d\n"
						"phl_next_idx = %d\n",
						phl_tring_list->phl_ring[i].cat,
						phl_tring_list->phl_ring[i].dma_ch,
						phl_tring_list->phl_ring[i].tx_thres,
						phl_tring_list->phl_ring[i].core_idx,
						phl_idx,
						phl_next_idx);

				for (j = 0; j < 5; j++) {
					PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
							"entry[%d] = %p\n",
							j,
					phl_tring_list->phl_ring[i].entry[j]);
				}
			}
		}
		break;
	case TYPE_RING_STS:
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "===Dump PHL Ring status===\n");
		phl_list_for_loop_safe(ring_sts, rsts_t, struct phl_ring_status,
					list_head, list) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
					"req_busy = %d\n"
					"ring_ptr = %p\n",
					ring_sts->req_busy,
					ring_sts->ring_ptr);
		}
		break;
	case TYPE_H2C_PKT:
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "===Dump H2C PKT===\n");
		phl_list_for_loop_safe(h2c_pkt, h2c_t, struct rtw_h2c_pkt,
					list_head, list) {
			vir_addr = (u8 *)h2c_pkt->vir_head;
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "vir_addr = %p, %x; phy_addr_l = %x; phy_addr_h = %x\n",
					vir_addr, *vir_addr,
					h2c_pkt->phy_addr_l,
					h2c_pkt->phy_addr_h);
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "type = %d; cache = %d; buf_len = %d\n",
					h2c_pkt->type, h2c_pkt->cache, h2c_pkt->buf_len);
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "vir_head = %p; vir_data = %p; vir_tail = %p; vir_end = %p\n",
					(u8 *)h2c_pkt->vir_head,
					(u8 *)h2c_pkt->vir_data,
					(u8 *)h2c_pkt->vir_tail,
					(u8 *)h2c_pkt->vir_end);
		}
		break;
	default :
		break;
	}
}

void phl_get_wd_ring_wd_page_cnt(void *phl, void *m){

	int ch =0;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_wd_page_ring *wd_ring = NULL;
	wd_ring = (struct rtw_wd_page_ring *)hci_info->wd_ring;

	RTW_PRINT_SEL(m, "======= Dump WD Ring =======\n");
	for (ch = 0; ch < hci_info->total_txch_num; ch++) {
	//for (ch = 0; ch < 1; ch++) {
		if (wd_ring[ch].idle_wd_page_cnt != 0 ||
			wd_ring[ch].pending_wd_page_cnt !=0 ||
			wd_ring[ch].busy_wd_page_cnt != 0)
		RTW_PRINT_SEL(m, "DMA_ch[%d] idle:%d, pending:%d, busy:%d\n",
				ch, wd_ring[ch].idle_wd_page_cnt,
				wd_ring[ch].pending_wd_page_cnt,
				wd_ring[ch].busy_wd_page_cnt);
	}

}
#endif

static struct phl_hci_trx_ops ops= {0};
void phl_hci_trx_ops_init(void)
{
	ops.hci_trx_init = phl_trx_init_pcie;
	ops.hci_trx_deinit = phl_trx_deinit_pcie;
	ops.prepare_tx = phl_prepare_tx_pcie;
	ops.recycle_rx_buf = phl_release_target_rx_buf;
	ops.tx = phl_tx_pcie;
	ops.rx = phl_rx_pcie;
	ops.trx_cfg = phl_trx_config_pcie;
	ops.trx_stop = phl_trx_stop_pcie;
	ops.recycle_busy_wd = phl_recycle_busy_wd;
	ops.recycle_busy_h2c = phl_recycle_busy_h2c;
	ops.recycle_h2c = phl_recycle_h2c;
	ops.pltfm_tx = phl_pltfm_tx_pcie;
	ops.alloc_h2c_pkt_buf = _phl_alloc_h2c_pkt_buf_pcie;
	ops.free_h2c_pkt_buf = _phl_free_h2c_pkt_buf_pcie;
	ops.trx_reset = phl_trx_reset_pcie;
	ops.trx_resume = phl_trx_resume_pcie;
	ops.req_tx_stop = phl_req_tx_stop_pcie;
	ops.req_rx_stop = phl_req_rx_stop_pcie;
	ops.is_tx_pause = phl_is_tx_sw_pause_pcie;
	ops.is_rx_pause = phl_is_rx_sw_pause_pcie;
	ops.get_txbd_buf = phl_get_txbd_buf_pcie;
	ops.get_rxbd_buf = phl_get_rxbd_buf_pcie;
	ops.recycle_rx_pkt = phl_recycle_rx_pkt_pcie;
	ops.register_trx_hdlr = phl_register_trx_hdlr_pcie;
	ops.rx_handle_normal = phl_rx_handle_normal;
	ops.tx_watchdog = phl_tx_watchdog_pcie;
	ops.tx_res_query = phl_tx_res_query;
	ops.rx_res_query = phl_rx_res_query;
}


enum rtw_phl_status phl_hook_trx_ops_pci(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if (NULL != phl_info) {
		phl_hci_trx_ops_init();
		phl_info->hci_trx_ops = &ops;
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}

enum rtw_phl_status phl_cmd_set_l2_leave(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

#ifdef CONFIG_CMD_DISP
	pstatus = phl_cmd_enqueue(phl_info, HW_BAND_0, MSG_EVT_HAL_SET_L2_LEAVE, NULL, 0, NULL, PHL_CMD_WAIT, 0);
	if (is_cmd_failure(pstatus)) {
		PHL_ERR("%s : cmd fail (0x%x)!\n", __func__, pstatus);
		pstatus = RTW_PHL_STATUS_FAILURE;
	}
	else
		pstatus = RTW_PHL_STATUS_SUCCESS;
#else
	if (rtw_hal_set_l2_leave(phl_info->hal) == RTW_HAL_STATUS_SUCCESS)
		pstatus = RTW_PHL_STATUS_SUCCESS;
#endif
	return pstatus;
}

#ifdef CONFIG_PHL_PCI_TRX_RES_DBG
void rtw_phl_get_rxbd(void *phl, u8 ch, u16 *host_idx, u16 *hw_idx, u16 *hw_res)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	phl_info->hci_trx_ops->rx_res_query(phl_info, ch, host_idx, hw_idx, hw_res);

}

u8 rtw_phl_get_rxch_num(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	return hci_info->total_rxch_num;
}

void rtw_phl_get_txbd(void *phl, u8 ch, u16 *host_idx, u16 *hw_idx, u16 *hw_res)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	phl_info->hci_trx_ops->tx_res_query(phl_info, ch, host_idx, hw_idx, hw_res);
}

u8 rtw_phl_get_txch_num(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	return hci_info->total_txch_num;
}
#endif /* CONFIG_PHL_PCI_TRX_RES_DBG */
