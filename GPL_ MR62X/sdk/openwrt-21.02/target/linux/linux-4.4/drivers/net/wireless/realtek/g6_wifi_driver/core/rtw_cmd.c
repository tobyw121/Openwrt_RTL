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
#define _RTW_CMD_C_

#include <drv_types.h>
#include "rtw_phl_cmd.h"

#ifndef DBG_CMD_EXECUTE
	#define DBG_CMD_EXECUTE 0
#endif

/*
Caller and the rtw_cmd_thread can protect cmd_q by spin_lock.
No irqsave is necessary.
*/
u32 rtw_init_cmd_priv(struct dvobj_priv *dvobj)
{
	u32 res = _SUCCESS;
	struct cmd_priv *pcmdpriv = &dvobj->cmdpriv;

	pcmdpriv->dvobj = dvobj;
	#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
	_rtw_init_sema(&(pcmdpriv->cmd_queue_sema), 0);
	_rtw_init_sema(&(pcmdpriv->start_cmdthread_sema), 0);
	_rtw_init_queue(&(pcmdpriv->cmd_queue));
	#endif

	/* allocate DMA-able/Non-Page memory for cmd_buf and rsp_buf */

	pcmdpriv->cmd_seq = 1;

	pcmdpriv->cmd_allocated_buf = rtw_zmalloc(MAX_CMDSZ + CMDBUFF_ALIGN_SZ);

	if (pcmdpriv->cmd_allocated_buf == NULL) {
		res = _FAIL;
		goto exit;
	}

	pcmdpriv->cmd_buf = pcmdpriv->cmd_allocated_buf + CMDBUFF_ALIGN_SZ - ((SIZE_PTR)(pcmdpriv->cmd_allocated_buf) & (CMDBUFF_ALIGN_SZ - 1));

	pcmdpriv->rsp_allocated_buf = rtw_zmalloc(MAX_RSPSZ + 4);

	if (pcmdpriv->rsp_allocated_buf == NULL) {
		res = _FAIL;
		goto exit;
	}

	pcmdpriv->rsp_buf = pcmdpriv->rsp_allocated_buf  +  4 - ((SIZE_PTR)(pcmdpriv->rsp_allocated_buf) & 3);

	pcmdpriv->cmd_issued_cnt = 0;

	_rtw_mutex_init(&pcmdpriv->sctx_mutex);
exit:
	return res;

}


sint _rtw_init_evt_priv(struct evt_priv *pevtpriv)
{
	sint res = _SUCCESS;


#ifdef CONFIG_H2CLBK
	_rtw_init_sema(&(pevtpriv->lbkevt_done), 0);
	pevtpriv->lbkevt_limit = 0;
	pevtpriv->lbkevt_num = 0;
	pevtpriv->cmdevt_parm = NULL;
#endif

	/* allocate DMA-able/Non-Page memory for cmd_buf and rsp_buf */
	ATOMIC_SET(&pevtpriv->event_seq, 0);
	pevtpriv->evt_done_cnt = 0;

#ifdef CONFIG_EVENT_THREAD_MODE

	_rtw_init_sema(&(pevtpriv->evt_notify), 0);

	pevtpriv->evt_allocated_buf = rtw_zmalloc(MAX_EVTSZ + 4);
	if (pevtpriv->evt_allocated_buf == NULL) {
		res = _FAIL;
		goto exit;
	}
	pevtpriv->evt_buf = pevtpriv->evt_allocated_buf  +  4 - ((unsigned int)(pevtpriv->evt_allocated_buf) & 3);


#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
	pevtpriv->allocated_c2h_mem = rtw_zmalloc(C2H_MEM_SZ + 4);

	if (pevtpriv->allocated_c2h_mem == NULL) {
		res = _FAIL;
		goto exit;
	}

	pevtpriv->c2h_mem = pevtpriv->allocated_c2h_mem +  4\
			    - ((u32)(pevtpriv->allocated_c2h_mem) & 3);
#endif /* end of CONFIG_SDIO_HCI */

	_rtw_init_queue(&(pevtpriv->evt_queue));

exit:

#endif /* end of CONFIG_EVENT_THREAD_MODE */

	return res;
}

void _rtw_free_evt_priv(struct	evt_priv *pevtpriv)
{


#ifdef CONFIG_EVENT_THREAD_MODE
	_rtw_free_sema(&(pevtpriv->evt_notify));

	if (pevtpriv->evt_allocated_buf)
		rtw_mfree(pevtpriv->evt_allocated_buf, MAX_EVTSZ + 4);
#endif

}

void rtw_free_cmd_priv(struct dvobj_priv *dvobj)
{
	struct cmd_priv *pcmdpriv = &dvobj->cmdpriv;

	#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
	_rtw_spinlock_free(&(pcmdpriv->cmd_queue.lock));
	_rtw_free_sema(&(pcmdpriv->cmd_queue_sema));
	_rtw_free_sema(&(pcmdpriv->start_cmdthread_sema));
	#endif
	if (pcmdpriv->cmd_allocated_buf)
		rtw_mfree(pcmdpriv->cmd_allocated_buf, MAX_CMDSZ + CMDBUFF_ALIGN_SZ);

	if (pcmdpriv->rsp_allocated_buf)
		rtw_mfree(pcmdpriv->rsp_allocated_buf, MAX_RSPSZ + 4);

	_rtw_mutex_free(&pcmdpriv->sctx_mutex);
}


/*
Calling Context:

rtw_enqueue_cmd can only be called between kernel thread,
since only spin_lock is used.

ISR/Call-Back functions can't call this sub-function.

*/
#ifdef DBG_CMD_QUEUE
extern u8 dump_cmd_id;
#endif

#if 0
u32 _rtw_enqueue_phl_cmd(struct cmd_priv *pcmdpriv, struct cmd_obj *obj, bool to_head)
{
	u32 res;
	_queue *queue = &pcmdpriv->cmd_queue;

	res = rtw_enqueue_phl_cmd(pcmdpriv, obj);

#ifdef DBG_CMD_QUEUE
	if (dump_cmd_id) {
		RTW_INFO("%s===> cmdcode:0x%02x\n", __FUNCTION__, obj->cmdcode);
		if (obj->cmdcode == CMD_SET_MLME_EVT) {
			if (obj->parmbuf) {
				struct rtw_evt_header *evt_hdr = (struct rtw_evt_header *)(obj->parmbuf);
				RTW_INFO("evt_hdr->id:%d\n", evt_hdr->id);
			}
		}
		if (obj->cmdcode == CMD_SET_DRV_EXTRA) {
			if (obj->parmbuf) {
				struct drvextra_cmd_parm *pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)(obj->parmbuf);
				RTW_INFO("pdrvextra_cmd_parm->ec_id:0x%02x\n", pdrvextra_cmd_parm->ec_id);
			}
		}
	}

	if (queue->queue.prev->next != &queue->queue) {
		RTW_INFO("[%d] head %p, tail %p, tail->prev->next %p[tail], tail->next %p[head]\n", __LINE__,
			&queue->queue, queue->queue.prev, queue->queue.prev->prev->next, queue->queue.prev->next);

		RTW_INFO("==========%s============\n", __FUNCTION__);
		RTW_INFO("head:%p,obj_addr:%p\n", &queue->queue, obj);
		RTW_INFO("padapter: %p\n", obj->padapter);
		RTW_INFO("cmdcode: 0x%02x\n", obj->cmdcode);
		RTW_INFO("res: %d\n", obj->res);
		RTW_INFO("parmbuf: %p\n", obj->parmbuf);
		RTW_INFO("cmdsz: %d\n", obj->cmdsz);
		RTW_INFO("rsp: %p\n", obj->rsp);
		RTW_INFO("rspsz: %d\n", obj->rspsz);
		RTW_INFO("sctx: %p\n", obj->sctx);
		RTW_INFO("list->next: %p\n", obj->list.next);
		RTW_INFO("list->prev: %p\n", obj->list.prev);
	}
#endif /* DBG_CMD_QUEUE */
	return res;
}
#endif

#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
sint _rtw_enqueue_cmd(_queue *queue, struct cmd_obj *obj, bool to_head)
{
	unsigned long sp_flags;

	if (obj == NULL)
		goto exit;

	/* _rtw_spinlock_bh(&queue->lock); */
	_rtw_spinlock_irq(&queue->lock, &sp_flags);

	if (to_head)
		rtw_list_insert_head(&obj->list, &queue->queue);
	else
		rtw_list_insert_tail(&obj->list, &queue->queue);

#ifdef DBG_CMD_QUEUE
	if (dump_cmd_id) {
		RTW_INFO("%s===> cmdcode:0x%02x\n", __FUNCTION__, obj->cmdcode);
		if (obj->cmdcode == CMD_SET_MLME_EVT) {
			if (obj->parmbuf) {
				struct rtw_evt_header *evt_hdr = (struct rtw_evt_header *)(obj->parmbuf);
				RTW_INFO("evt_hdr->id:%d\n", evt_hdr->id);
			}
		}
		if (obj->cmdcode == CMD_SET_DRV_EXTRA) {
			if (obj->parmbuf) {
				struct drvextra_cmd_parm *pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)(obj->parmbuf);
				RTW_INFO("pdrvextra_cmd_parm->ec_id:0x%02x\n", pdrvextra_cmd_parm->ec_id);
			}
		}
	}

	if (queue->queue.prev->next != &queue->queue) {
		RTW_INFO("[%d] head %p, tail %p, tail->prev->next %p[tail], tail->next %p[head]\n", __LINE__,
			&queue->queue, queue->queue.prev, queue->queue.prev->prev->next, queue->queue.prev->next);

		RTW_INFO("==========%s============\n", __FUNCTION__);
		RTW_INFO("head:%p,obj_addr:%p\n", &queue->queue, obj);
		RTW_INFO("padapter: %p\n", obj->padapter);
		RTW_INFO("cmdcode: 0x%02x\n", obj->cmdcode);
		RTW_INFO("res: %d\n", obj->res);
		RTW_INFO("parmbuf: %p\n", obj->parmbuf);
		RTW_INFO("cmdsz: %d\n", obj->cmdsz);
		RTW_INFO("rsp: %p\n", obj->rsp);
		RTW_INFO("rspsz: %d\n", obj->rspsz);
		RTW_INFO("sctx: %p\n", obj->sctx);
		RTW_INFO("list->next: %p\n", obj->list.next);
		RTW_INFO("list->prev: %p\n", obj->list.prev);
	}
#endif /* DBG_CMD_QUEUE */

	/* _rtw_spinunlock_bh(&queue->lock);	 */
	_rtw_spinunlock_irq(&queue->lock, &sp_flags);

exit:


	return _SUCCESS;
}
#else
static sint _rtw_enqueue_cmd(struct cmd_obj *obj, bool to_head)
{
	u32 res;

	res = rtw_enqueue_phl_cmd(obj);

#ifdef DBG_CMD_QUEUE
	if (dump_cmd_id) {
		RTW_INFO("%s===> cmdcode:0x%02x\n", __FUNCTION__, obj->cmdcode);
		if (obj->cmdcode == CMD_SET_MLME_EVT) {
			if (obj->parmbuf) {
				struct rtw_evt_header *evt_hdr = (struct rtw_evt_header *)(obj->parmbuf);
				RTW_INFO("evt_hdr->id:%d\n", evt_hdr->id);
			}
		}
		if (obj->cmdcode == CMD_SET_DRV_EXTRA) {
			if (obj->parmbuf) {
				struct drvextra_cmd_parm *pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)(obj->parmbuf);
				RTW_INFO("pdrvextra_cmd_parm->ec_id:0x%02x\n", pdrvextra_cmd_parm->ec_id);
			}
		}
	}
#endif /* DBG_CMD_QUEUE */
	return res;
}
#endif

#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
struct	cmd_obj	*_rtw_dequeue_cmd(_queue *queue)
{
	struct cmd_obj *obj;
	unsigned long sp_flags;

	/* _rtw_spinlock_bh(&(queue->lock)); */
	_rtw_spinlock_irq(&queue->lock, &sp_flags);

#ifdef DBG_CMD_QUEUE
	if (queue->queue.prev->next != &queue->queue) {
		RTW_INFO("[%d] head %p, tail %p, tail->prev->next %p[tail], tail->next %p[head]\n", __LINE__,
			&queue->queue, queue->queue.prev, queue->queue.prev->prev->next, queue->queue.prev->next);
	}
#endif /* DBG_CMD_QUEUE */


	if (rtw_is_list_empty(&(queue->queue)))
		obj = NULL;
	else {
		obj = LIST_CONTAINOR(get_next(&(queue->queue)), struct cmd_obj, list);

#ifdef DBG_CMD_QUEUE
		if (queue->queue.prev->next != &queue->queue) {
			RTW_INFO("==========%s============\n", __FUNCTION__);
			RTW_INFO("head:%p,obj_addr:%p\n", &queue->queue, obj);
			RTW_INFO("padapter: %p\n", obj->padapter);
			RTW_INFO("cmdcode: 0x%02x\n", obj->cmdcode);
			RTW_INFO("res: %d\n", obj->res);
			RTW_INFO("parmbuf: %p\n", obj->parmbuf);
			RTW_INFO("cmdsz: %d\n", obj->cmdsz);
			RTW_INFO("rsp: %p\n", obj->rsp);
			RTW_INFO("rspsz: %d\n", obj->rspsz);
			RTW_INFO("sctx: %p\n", obj->sctx);
			RTW_INFO("list->next: %p\n", obj->list.next);
			RTW_INFO("list->prev: %p\n", obj->list.prev);
		}

		if (dump_cmd_id) {
			RTW_INFO("%s===> cmdcode:0x%02x\n", __FUNCTION__, obj->cmdcode);
			if (obj->cmdcode == CMD_SET_DRV_EXTRA) {
				if (obj->parmbuf) {
					struct drvextra_cmd_parm *pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)(obj->parmbuf);
					printk("pdrvextra_cmd_parm->ec_id:0x%02x\n", pdrvextra_cmd_parm->ec_id);
				}
			}

		}
#endif /* DBG_CMD_QUEUE */

		rtw_list_delete(&obj->list);
	}

	/* _rtw_spinunlock_bh(&(queue->lock)); */
	_rtw_spinunlock_irq(&queue->lock, &sp_flags);


	return obj;
}

struct	cmd_obj	*rtw_dequeue_cmd(struct cmd_priv *pcmdpriv)
{
	struct cmd_obj *cmd_obj;


	cmd_obj = _rtw_dequeue_cmd(&pcmdpriv->cmd_queue);

	return cmd_obj;
}
#endif

u32	rtw_init_evt_priv(struct	evt_priv *pevtpriv)
{
	int	res;
	res = _rtw_init_evt_priv(pevtpriv);
	return res;
}

void rtw_free_evt_priv(struct	evt_priv *pevtpriv)
{
	_rtw_free_evt_priv(pevtpriv);
}

int rtw_cmd_filter(struct cmd_priv *pcmdpriv, struct cmd_obj *cmd_obj)
{
	u8 bAllow = _FALSE; /* set to _TRUE to allow enqueuing cmd when hw_init_completed is _FALSE */
	struct dvobj_priv *dvobj = pcmdpriv->dvobj;


	if (cmd_obj->cmdcode == CMD_SET_CHANPLAN)
		bAllow = _TRUE;

	if (cmd_obj->no_io)
		bAllow = _TRUE;

	if ((!rtw_hw_is_init_completed(dvobj) && (bAllow == _FALSE))
#if 0 //#ifndef CONFIG_PHL_ARCH
	    || ATOMIC_READ(&(pcmdpriv->cmdthd_running)) == _FALSE	/* com_thread not running */
#endif
	   ) {
	   	#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
		if (DBG_CMD_EXECUTE)
			RTW_INFO(ADPT_FMT" drop "CMD_FMT" hw_init_completed:%u, cmdthd_running:%u\n",
				ADPT_ARG(cmd_obj->padapter),
				CMD_ARG(cmd_obj), rtw_hw_get_init_completed(dvobj),
				ATOMIC_READ(&pcmdpriv->cmdthd_running));
		#endif
		if (0)
			rtw_warn_on(1);

		return _FAIL;
	}
	return _SUCCESS;
}



u32 rtw_enqueue_cmd(struct cmd_priv *pcmdpriv, struct cmd_obj *cmd_obj)
{
	int res = _FAIL;

	if (cmd_obj == NULL)
		goto exit;

	res = rtw_cmd_filter(pcmdpriv, cmd_obj);
	if ((_FAIL == res) || (cmd_obj->cmdsz > MAX_CMDSZ)) {
		if (cmd_obj->cmdsz > MAX_CMDSZ) {
			RTW_INFO("%s failed due to obj->cmdsz(%d) > MAX_CMDSZ(%d)\n", __func__, cmd_obj->cmdsz, MAX_CMDSZ);
			rtw_warn_on(1);
		}

		if (cmd_obj->cmdcode == CMD_SET_DRV_EXTRA) {
			struct drvextra_cmd_parm *extra_parm = (struct drvextra_cmd_parm *)cmd_obj->parmbuf;

			if (extra_parm->pbuf && extra_parm->size > 0)
				rtw_mfree(extra_parm->pbuf, extra_parm->size);
		}
		rtw_free_cmd_obj(cmd_obj);
		goto exit;
	}

	res = _rtw_enqueue_cmd(cmd_obj, 0);
#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
	if (res == _SUCCESS)
		_rtw_up_sema(&pcmdpriv->cmd_queue_sema);
#endif

exit:


	return res;
}
void rtw_free_cmd_obj(struct cmd_obj *pcmd)
{
	if (pcmd->parmbuf != NULL) {
		/* free parmbuf in cmd_obj */
		rtw_mfree((unsigned char *)pcmd->parmbuf, pcmd->cmdsz);
	}
	if (pcmd->rsp != NULL) {
		if (pcmd->rspsz != 0) {
			/* free rsp in cmd_obj */
			rtw_mfree((unsigned char *)pcmd->rsp, pcmd->rspsz);
		}
	}

	/* free cmd_obj */
	rtw_mfree((unsigned char *)pcmd, sizeof(struct cmd_obj));
}
void rtw_run_cmd(_adapter *padapter, struct cmd_obj *pcmd, bool discard)
{
	u8 ret;
	u8 *pcmdbuf;
	systime cmd_start_time;
	u32 cmd_process_time;
	u32 len_wlancmds = (sizeof(wlancmds) / sizeof(struct rtw_cmd));
	u8(*cmd_hdl)(_adapter *padapter, u8 *pbuf);
	void (*pcmd_callback)(_adapter *dev, struct cmd_obj *pcmd);
	struct cmd_priv *pcmdpriv = &(adapter_to_dvobj(padapter)->cmdpriv);
	struct drvextra_cmd_parm *extra_parm = NULL;

	cmd_start_time = rtw_get_current_time();
	pcmdpriv->cmd_issued_cnt++;
	#ifndef CONFIG_RTW_SUPPORT_MBSSID_VAP
	pcmd->padapter = padapter;
	#endif

	if (discard)
		goto post_process;

	if (pcmd->cmdsz > MAX_CMDSZ) {
		RTW_ERR("%s cmdsz:%d > MAX_CMDSZ:%d\n", __func__, pcmd->cmdsz, MAX_CMDSZ);
		pcmd->res = H2C_PARAMETERS_ERROR;
		goto post_process;
	}

	if (pcmd->cmdcode >= len_wlancmds) {
		RTW_ERR("%s undefined cmdcode:%d\n", __func__, pcmd->cmdcode);
		pcmd->res = H2C_PARAMETERS_ERROR;
		goto post_process;
	}

	if (!pcmd->padapter->run_cmd_en && pcmd->cmdcode != CMD_SET_RUN_CMD_EN) {
		RTW_WARN("[%s %d] run_cmd_en disable, drop cmdcode: %u\n", __FUNCTION__, __LINE__, pcmd->cmdcode);
		pcmd->res = H2C_DROPPED;
		goto post_process;
	}

	cmd_hdl = wlancmds[pcmd->cmdcode].cmd_hdl;
	if (!cmd_hdl) {
		RTW_ERR("%s no cmd_hdl for cmdcode:%d\n", __func__, pcmd->cmdcode);
		pcmd->res = H2C_PARAMETERS_ERROR;
		goto post_process;
	}

	if (DBG_CMD_EXECUTE)
		RTW_INFO(ADPT_FMT" "CMD_FMT" %sexecute\n", ADPT_ARG(pcmd->padapter), CMD_ARG(pcmd)
			, pcmd->res == H2C_ENQ_HEAD ? "ENQ_HEAD " : (pcmd->res == H2C_ENQ_HEAD_FAIL ? "ENQ_HEAD_FAIL " : ""));

	pcmdbuf = pcmdpriv->cmd_buf;
	_rtw_memcpy(pcmdbuf, pcmd->parmbuf, pcmd->cmdsz);
	ret = cmd_hdl(pcmd->padapter, pcmdbuf);
	pcmd->res = ret;

	pcmdpriv->cmd_seq++;

post_process:

	_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
	if (pcmd->sctx) {
		if (0)
			RTW_PRINT(FUNC_ADPT_FMT" pcmd->sctx\n", FUNC_ADPT_ARG(pcmd->padapter));
		if (pcmd->res == H2C_SUCCESS)
			rtw_sctx_done(&pcmd->sctx);
		else
			rtw_sctx_done_err(&pcmd->sctx, RTW_SCTX_DONE_CMD_ERROR);
	}
	_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);

	cmd_process_time = rtw_get_passing_time_ms(cmd_start_time);
	if (cmd_process_time > 1000) {
		RTW_INFO(ADPT_FMT" "CMD_FMT" process_time=%d\n", ADPT_ARG(pcmd->padapter), CMD_ARG(pcmd), cmd_process_time);
		if (0)
			rtw_warn_on(1);
	}

	/* call callback function for post-processed */
	if (pcmd->cmdcode < len_wlancmds)
		pcmd_callback = wlancmds[pcmd->cmdcode].callback;
	else
		pcmd_callback = NULL;

	if (pcmd_callback == NULL) {
		rtw_free_cmd_obj(pcmd);
	} else {
		/* todo: !!! fill rsp_buf to pcmd->rsp if (pcmd->rsp!=NULL) */
		pcmd_callback(pcmd->padapter, pcmd);/* need conider that free cmd_obj in rtw_cmd_callback */
	}
}
#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
void rtw_stop_cmd_thread(_adapter *adapter)
{
	if (adapter->cmdThread) {
		_rtw_up_sema(&adapter->cmdpriv.cmd_queue_sema);
		rtw_thread_stop(adapter->cmdThread);
		adapter->cmdThread = NULL;
	}
}

thread_return rtw_cmd_thread(thread_context context)
{
	u8 ret;
	struct cmd_obj *pcmd;
	u8 *pcmdbuf, *prspbuf;
	systime cmd_start_time;
	u32 cmd_process_time;
	u8(*cmd_hdl)(_adapter *padapter, u8 *pbuf);
	void (*pcmd_callback)(_adapter *dev, struct cmd_obj *pcmd);
	_adapter *padapter = (_adapter *)context;
	struct cmd_priv *pcmdpriv = &(padapter->cmdpriv);
	struct drvextra_cmd_parm *extra_parm = NULL;
	unsigned long sp_flags;

	rtw_thread_enter("RTW_CMD_THREAD");

	pcmdbuf = pcmdpriv->cmd_buf;
	prspbuf = pcmdpriv->rsp_buf;
	ATOMIC_SET(&(pcmdpriv->cmdthd_running), _TRUE);
	_rtw_up_sema(&pcmdpriv->start_cmdthread_sema);


	while (1) {
		if (_rtw_down_sema(&pcmdpriv->cmd_queue_sema) == _FAIL) {
			RTW_PRINT(FUNC_ADPT_FMT" _rtw_down_sema(&pcmdpriv->cmd_queue_sema) return _FAIL, break\n", FUNC_ADPT_ARG(padapter));
			break;
		}

		if (RTW_CANNOT_RUN(adapter_to_dvobj(padapter))) {
			RTW_DBG(FUNC_ADPT_FMT "- bDriverStopped(%s) bSurpriseRemoved(%s)\n",
				FUNC_ADPT_ARG(padapter),
				dev_is_drv_stopped(adapter_to_dvobj(padapter)) ? "True" : "False",
				dev_is_surprise_removed(adapter_to_dvobj(padapter)) ? "True" : "False");
			break;
		}

		_rtw_spinlock_irq(&pcmdpriv->cmd_queue.lock, &sp_flags);
		if (rtw_is_list_empty(&(pcmdpriv->cmd_queue.queue))) {
			/* RTW_INFO("%s: cmd queue is empty!\n", __func__); */
			_rtw_spinunlock_irq(&pcmdpriv->cmd_queue.lock, &sp_flags);
			continue;
		}
		_rtw_spinunlock_irq(&pcmdpriv->cmd_queue.lock, &sp_flags);

_next:
		if (RTW_CANNOT_RUN(adapter_to_dvobj(padapter))) {
			RTW_PRINT("%s: DriverStopped(%s) SurpriseRemoved(%s) break at line %d\n",
				  __func__
				, dev_is_drv_stopped(adapter_to_dvobj(padapter)) ? "True" : "False"
				, dev_is_surprise_removed(adapter_to_dvobj(padapter)) ? "True" : "False"
				  , __LINE__);
			break;
		}

		pcmd = rtw_dequeue_cmd(pcmdpriv);
		if (!pcmd) {
#ifdef CONFIG_LPS_LCLK
			rtw_unregister_cmd_alive(padapter);
#endif
			continue;
		}

		cmd_start_time = rtw_get_current_time();
		pcmdpriv->cmd_issued_cnt++;

		if (pcmd->cmdsz > MAX_CMDSZ) {
			RTW_ERR("%s cmdsz:%d > MAX_CMDSZ:%d\n", __func__, pcmd->cmdsz, MAX_CMDSZ);
			pcmd->res = H2C_PARAMETERS_ERROR;
			goto post_process;
		}

		if (pcmd->cmdcode >= (sizeof(wlancmds) / sizeof(struct rtw_cmd))) {
			RTW_ERR("%s undefined cmdcode:%d\n", __func__, pcmd->cmdcode);
			pcmd->res = H2C_PARAMETERS_ERROR;
			goto post_process;
		}

		cmd_hdl = wlancmds[pcmd->cmdcode].cmd_hdl;
		if (!cmd_hdl) {
			RTW_ERR("%s no cmd_hdl for cmdcode:%d\n", __func__, pcmd->cmdcode);
			pcmd->res = H2C_PARAMETERS_ERROR;
			goto post_process;
		}

		if (_FAIL == rtw_cmd_filter(pcmdpriv, pcmd)) {
			pcmd->res = H2C_DROPPED;
			if (pcmd->cmdcode == CMD_SET_DRV_EXTRA) {
				extra_parm = (struct drvextra_cmd_parm *)pcmd->parmbuf;
				if (extra_parm && extra_parm->pbuf && extra_parm->size > 0)
					rtw_mfree(extra_parm->pbuf, extra_parm->size);
			}
			#if CONFIG_DFS
			else if (pcmd->cmdcode == CMD_SET_CHANSWITCH)
				adapter_to_rfctl(padapter)->csa_ch = 0;
			#endif
			goto post_process;
		}

#ifdef CONFIG_LPS_LCLK
		if (pcmd->no_io)
			rtw_unregister_cmd_alive(padapter);
		else {
			if (rtw_register_cmd_alive(padapter) != _SUCCESS) {
				if (DBG_CMD_EXECUTE)
					RTW_PRINT("%s: wait to leave LPS_LCLK\n", __func__);

				pcmd->res = H2C_ENQ_HEAD;
				ret = _rtw_enqueue_cmd(&pcmdpriv->cmd_queue, pcmd, 1);
				if (ret == _SUCCESS) {
					if (DBG_CMD_EXECUTE)
						RTW_INFO(ADPT_FMT" "CMD_FMT" ENQ_HEAD\n", ADPT_ARG(pcmd->padapter), CMD_ARG(pcmd));
					continue;
				}

				RTW_INFO(ADPT_FMT" "CMD_FMT" ENQ_HEAD_FAIL\n", ADPT_ARG(pcmd->padapter), CMD_ARG(pcmd));
				pcmd->res = H2C_ENQ_HEAD_FAIL;
				rtw_warn_on(1);
			}
		}
#endif /* CONFIG_LPS_LCLK */

		if (DBG_CMD_EXECUTE)
			RTW_INFO(ADPT_FMT" "CMD_FMT" %sexecute\n", ADPT_ARG(pcmd->padapter), CMD_ARG(pcmd)
				, pcmd->res == H2C_ENQ_HEAD ? "ENQ_HEAD " : (pcmd->res == H2C_ENQ_HEAD_FAIL ? "ENQ_HEAD_FAIL " : ""));

		_rtw_memcpy(pcmdbuf, pcmd->parmbuf, pcmd->cmdsz);
		ret = cmd_hdl(pcmd->padapter, pcmdbuf);
		pcmd->res = ret;

		pcmdpriv->cmd_seq++;

post_process:

		_rtw_mutex_lock_interruptible(&(pcmd->padapter->cmdpriv.sctx_mutex));
		if (pcmd->sctx) {
			if (0)
				RTW_PRINT(FUNC_ADPT_FMT" pcmd->sctx\n", FUNC_ADPT_ARG(pcmd->padapter));
			if (pcmd->res == H2C_SUCCESS)
				rtw_sctx_done(&pcmd->sctx);
			else
				rtw_sctx_done_err(&pcmd->sctx, RTW_SCTX_DONE_CMD_ERROR);
		}
		_rtw_mutex_unlock(&(pcmd->padapter->cmdpriv.sctx_mutex));

		cmd_process_time = rtw_get_passing_time_ms(cmd_start_time);
		if (cmd_process_time > 1000) {
			RTW_INFO(ADPT_FMT" "CMD_FMT" process_time=%d\n", ADPT_ARG(pcmd->padapter), CMD_ARG(pcmd), cmd_process_time);
			if (0)
				rtw_warn_on(1);
		}

		/* call callback function for post-processed */
		if (pcmd->cmdcode < (sizeof(wlancmds) / sizeof(struct rtw_cmd)))
			pcmd_callback = wlancmds[pcmd->cmdcode].callback;
		else
			pcmd_callback = NULL;

		if (pcmd_callback == NULL) {
			rtw_free_cmd_obj(pcmd);
		} else {
			/* todo: !!! fill rsp_buf to pcmd->rsp if (pcmd->rsp!=NULL) */
			pcmd_callback(pcmd->padapter, pcmd);/* need conider that free cmd_obj in rtw_cmd_callback */
		}

		flush_signals_thread();

		goto _next;

	}

#ifdef CONFIG_LPS_LCLK
	rtw_unregister_cmd_alive(padapter);
#endif

	/* to avoid enqueue cmd after free all cmd_obj */
	ATOMIC_SET(&(pcmdpriv->cmdthd_running), _FALSE);

	/* free all cmd_obj resources */
	do {
		pcmd = rtw_dequeue_cmd(pcmdpriv);
		if (pcmd == NULL)
			break;

		if (0)
			RTW_INFO("%s: leaving... drop "CMD_FMT"\n", __func__, CMD_ARG(pcmd));

		if (pcmd->cmdcode == CMD_SET_DRV_EXTRA) {
			extra_parm = (struct drvextra_cmd_parm *)pcmd->parmbuf;
			if (extra_parm->pbuf && extra_parm->size > 0)
				rtw_mfree(extra_parm->pbuf, extra_parm->size);
		}
		#if CONFIG_DFS
		else if (pcmd->cmdcode == CMD_SET_CHANSWITCH)
			adapter_to_rfctl(padapter)->csa_ch = 0;
		#endif

		_rtw_mutex_lock_interruptible(&(pcmd->padapter->cmdpriv.sctx_mutex));
		if (pcmd->sctx) {
			if (0)
				RTW_PRINT(FUNC_ADPT_FMT" pcmd->sctx\n", FUNC_ADPT_ARG(pcmd->padapter));
			rtw_sctx_done_err(&pcmd->sctx, RTW_SCTX_DONE_CMD_DROP);
		}
		_rtw_mutex_unlock(&(pcmd->padapter->cmdpriv.sctx_mutex));

		rtw_free_cmd_obj(pcmd);
	} while (1);

	RTW_INFO(FUNC_ADPT_FMT " Exit\n", FUNC_ADPT_ARG(padapter));

	rtw_thread_wait_stop();

	return 0;
}
#endif


#ifdef CONFIG_EVENT_THREAD_MODE
u32 rtw_enqueue_evt(struct evt_priv *pevtpriv, struct evt_obj *obj)
{
	int	res;
	_queue *queue = &pevtpriv->evt_queue;


	res = _SUCCESS;

	if (obj == NULL) {
		res = _FAIL;
		goto exit;
	}

	_rtw_spinlock_bh(&queue->lock);

	rtw_list_insert_tail(&obj->list, &queue->queue);

	_rtw_spinunlock_bh(&queue->lock);

	/* rtw_evt_notify_isr(pevtpriv); */

exit:


	return res;
}

struct evt_obj *rtw_dequeue_evt(_queue *queue)
{
	struct	evt_obj	*pevtobj;


	_rtw_spinlock_bh(&queue->lock);

	if (rtw_is_list_empty(&(queue->queue)))
		pevtobj = NULL;
	else {
		pevtobj = LIST_CONTAINOR(get_next(&(queue->queue)), struct evt_obj, list);
		rtw_list_delete(&pevtobj->list);
	}

	_rtw_spinunlock_bh(&queue->lock);


	return pevtobj;
}

void rtw_free_evt_obj(struct evt_obj *pevtobj)
{

	if (pevtobj->parmbuf)
		rtw_mfree((unsigned char *)pevtobj->parmbuf, pevtobj->evtsz);

	rtw_mfree((unsigned char *)pevtobj, sizeof(struct evt_obj));

}

void rtw_evt_notify_isr(struct evt_priv *pevtpriv)
{
	pevtpriv->evt_done_cnt++;
	_rtw_up_sema(&(pevtpriv->evt_notify));
}
#endif

void rtw_init_sitesurvey_parm(_adapter *padapter, struct sitesurvey_parm *pparm)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;


	_rtw_memset(pparm, 0, sizeof(struct sitesurvey_parm));
	pparm->scan_mode = pmlmepriv->scan_mode;
}

static inline bool _rtw_scan_abort_check(_adapter *adapter, const char *caller)
{
	struct	mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;
	struct submit_ctx *sctx = &pmlmeext->sitesurvey_res.sctx;

	RTW_INFO(FUNC_ADPT_FMT "- %s....scan_abort:%d\n",
			FUNC_ADPT_ARG(adapter), __func__, pmlmeext->scan_abort);

	if (pmlmeext->scan_abort == _FALSE)
		return _FALSE;

	if (pmlmeext->scan_abort_to) {
		RTW_WARN("%s scan abort timeout\n", caller);
	}

	pmlmeext->scan_abort = _FALSE;
	pmlmeext->scan_abort_to = _FALSE;
	if (sctx) {
		RTW_INFO("%s scan abort .....(%d ms)\n", caller, rtw_get_passing_time_ms(sctx->submit_time));
		rtw_sctx_done(&sctx);
	}
	return _TRUE;
}

#ifdef CONFIG_CMD_SCAN
static struct rtw_phl_scan_param *_alloc_phl_param(_adapter *adapter, u8 scan_ch_num)
{
	struct rtw_phl_scan_param *phl_param = NULL;
	struct scan_priv *scan_priv = NULL;

	if (scan_ch_num == 0) {
		RTW_ERR("%s scan_ch_num = 0\n", __func__);
		goto _err_exit;
	}
	/*create mem of PHL Scan parameter*/
	phl_param = rtw_zmalloc(sizeof(*phl_param));
	if (phl_param == NULL) {
		RTW_ERR("%s alloc phl_param fail\n", __func__);
		goto _err_exit;
	}

	scan_priv = rtw_zmalloc(sizeof(*scan_priv));
	if (scan_priv == NULL) {
		RTW_ERR("%s alloc scan_priv fail\n", __func__);
		goto _err_scanpriv;
	}
	scan_priv->padapter = adapter;
	phl_param->priv = scan_priv;
	phl_param->wifi_role = adapter->phl_role;
	phl_param->back_op_mode = SCAN_BKOP_NONE;

	phl_param->ch_sz = sizeof(struct phl_scan_channel) * (scan_ch_num + 1);
	phl_param->ch = rtw_zmalloc(phl_param->ch_sz);
	if (phl_param->ch == NULL) {
		RTW_ERR("%s: alloc phl scan ch fail\n", __func__);
		goto _err_param_ch;
	}

	return phl_param;

_err_param_ch:
	if (scan_priv)
		rtw_mfree(scan_priv, sizeof(*scan_priv));
_err_scanpriv:
	if (phl_param)
		rtw_mfree(phl_param, sizeof(*phl_param));
_err_exit:
	rtw_warn_on(1);
	return phl_param;
}

static u8 _free_phl_param(_adapter *adapter, struct rtw_phl_scan_param *phl_param)
{
	u8 res = _FAIL;

	if (!phl_param)
		return res;

	if (phl_param->ch)
		rtw_mfree(phl_param->ch, phl_param->ch_sz);
	if (phl_param->priv)
		rtw_mfree(phl_param->priv, sizeof(struct scan_priv));
	rtw_mfree(phl_param, sizeof(struct rtw_phl_scan_param));

	res = _SUCCESS;
	return res;
}

static void scan_channel_list_filled(_adapter *padapter,
	struct rtw_phl_scan_param *phl_param, struct sitesurvey_parm *param)
{
	struct phl_scan_channel *phl_ch = phl_param->ch;
	u8 i = 0;

	for (i = 0; i < param->ch_num; i++) {
		/* should be consider 6G */
		phl_ch[i].band = (param->ch[i].hw_value > 14) ? BAND_ON_5G : BAND_ON_24G;
		phl_ch[i].channel = param->ch[i].hw_value;
		phl_ch[i].scan_mode = NORMAL_SCAN_MODE;
		phl_ch[i].bw = param->bw;
		phl_ch[i].duration = param->duration;

		if (param->ch[i].flags & RTW_IEEE80211_CHAN_PASSIVE_SCAN) {
			phl_ch[i].type = RTW_PHL_SCAN_PASSIVE;

		} else {
			phl_ch[i].type = RTW_PHL_SCAN_ACTIVE;

			/* reduce scan time in active channel */
			if (param->scan_type == RTW_SCAN_NORMAL && !param->acs)
				phl_ch[i].duration = param->duration >> 1;
		}
	}
	phl_param->ch_num = param->ch_num;
}

static int scan_issue_pbreq_cb(void *priv, struct rtw_phl_scan_param *param)
{
	struct scan_priv *scan_priv = (struct scan_priv *)priv;
	_adapter *padapter = scan_priv->padapter;
	NDIS_802_11_SSID ssid;
	int i;


	/* active scan behavior */
	for (i = 0; i < param->ssid_num; i++) {
		if (param->ssid[i].ssid_len == 0)
			continue;

		ssid.SsidLength = param->ssid[i].ssid_len;
		_rtw_memcpy(ssid.Ssid, &param->ssid[i].ssid, ssid.SsidLength);
		/* IOT issue,
		 * Send one probe req without WPS IE,
		 * when not wifi_spec
		 */
		if (padapter->registrypriv.wifi_spec)
			issue_probereq(padapter, &ssid, NULL);
		else
			issue_probereq_ex(padapter, &ssid, NULL, 0, 0, 0, 0);

		issue_probereq(padapter, &ssid, NULL);
	}

	if (padapter->registrypriv.wifi_spec)
		issue_probereq(padapter, NULL, NULL);
	else
		issue_probereq_ex(padapter, NULL, NULL, 0, 0, 0, 0);

	issue_probereq(padapter, NULL, NULL);

	return 0;
}

static int scan_complete_cb(void *priv, struct rtw_phl_scan_param *param)
{
	struct scan_priv *scan_priv = (struct scan_priv *)priv;
	_adapter *padapter = scan_priv->padapter;
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	bool acs = _FALSE;
	int ret = _FAIL;

	if (!rtw_is_adapter_up(padapter))
		goto _exit;

	#if defined(CONFIG_RTW_CFGVENDOR_RANDOM_MAC_OUI) || defined(CONFIG_RTW_SCAN_RAND)
	{
		struct rtw_wdev_priv *pwdev_priv = adapter_wdev_data(padapter);

		if (pwdev_priv->random_mac_enabled
		    && (MLME_IS_STA(padapter))
		    && (check_fwstate(&padapter->mlmepriv, WIFI_ASOC_STATE) == _FALSE))
			rtw_set_mac_addr_hw(padapter, adapter_mac_addr(padapter));
	}
	#endif /* CONFIG_RTW_SCAN_RAND */

	mlmeext_set_scan_state(pmlmeext, SCAN_DISABLE);

	report_surveydone_event(padapter, param->acs, RTW_CMDF_DIRECTLY);

	ret = _SUCCESS;

_exit:
	RTW_INFO(FUNC_ADPT_FMT" takes %d ms to scan %d/%d channels\n",
			FUNC_ADPT_ARG(padapter), param->total_scan_time,
			#ifdef CONFIG_CMD_SCAN
			param->ch_idx,
			#else
			param->ch_idx + 1,
			#endif
			param->ch_num);
	_rtw_scan_abort_check(padapter, __func__);

#ifdef CONFIG_CMD_SCAN
	_free_phl_param(padapter, param);
	pmlmeext->sitesurvey_res.scan_param = NULL;
#else
	rtw_mfree(scan_priv, sizeof(*scan_priv));
#endif

	return ret;
}

static int scan_start_cb(void *priv, struct rtw_phl_scan_param *param)
{
	struct scan_priv *scan_priv = (struct scan_priv *)priv;
	_adapter *padapter = scan_priv->padapter;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	#if defined(CONFIG_RTW_CFGVENDOR_RANDOM_MAC_OUI) || defined(CONFIG_RTW_SCAN_RAND)
	{
		struct rtw_wdev_priv *pwdev_priv = adapter_wdev_data(padapter);

		if (pwdev_priv->random_mac_enabled
		    && (MLME_IS_STA(padapter))
		    && (check_fwstate(&padapter->mlmepriv, WIFI_ASOC_STATE) == _FALSE)) {
			u16 seq_num;

			rtw_set_mac_addr_hw(padapter, pwdev_priv->pno_mac_addr);
			get_random_bytes(&seq_num, 2);
			pwdev_priv->pno_scan_seq_num = seq_num & 0xFFF;
		}
	}
	#endif /* CONFIG_RTW_SCAN_RAND */

	pmlmeext->sitesurvey_res.bss_cnt = 0;
	pmlmeext->sitesurvey_res.activate_ch_cnt = 0;
	//TODO remove
	mlmeext_set_scan_state(pmlmeext, SCAN_PROCESS);
	#ifdef CONFIG_CMD_SCAN
	pmlmeext->sitesurvey_res.scan_param = param;
	#endif
	return 0;
}



#ifdef CONFIG_P2P
static int scan_issue_p2p_pbreq_cb(void *priv, struct rtw_phl_scan_param *param)
{
	struct scan_priv *scan_priv = (struct scan_priv *)priv;
	_adapter *padapter = scan_priv->padapter;

	issue_probereq_p2p(padapter, NULL);
	issue_probereq_p2p(padapter, NULL);
	issue_probereq_p2p(padapter, NULL);
	return 0;
}
#endif

static int scan_ch_ready_cb(void *priv, struct rtw_phl_scan_param *param)
{
	struct scan_priv *scan_priv = (struct scan_priv *)priv;
	_adapter *padapter = scan_priv->padapter;

	RTW_INFO("%s ch:%d\n", __func__, param->scan_ch->channel);
	return 0;
}

#ifdef CONFIG_P2P
static struct rtw_phl_scan_ops scan_ops_p2p_cb = {
	.scan_start = scan_start_cb,
	.scan_ch_ready = scan_ch_ready_cb,
	.scan_complete = scan_complete_cb,
	.scan_issue_pbreq = scan_issue_p2p_pbreq_cb,
	/*.scan_issue_null_data = scan_issu_null_data_cb*/
};
#endif

#ifdef CONFIG_RTW_80211K
static int scan_complete_rrm_cb(void *priv, struct rtw_phl_scan_param *param)
{
	struct scan_priv *scan_priv = (struct scan_priv *)priv;
	_adapter *padapter = scan_priv->padapter;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	int ret = _FAIL;

	if (!rtw_is_adapter_up(padapter))
		goto _exit;

	mlmeext_set_scan_state(pmlmeext, SCAN_DISABLE);
	_rtw_spinlock_bh(&pmlmepriv->lock);
	_clr_fwstate_(pmlmepriv, WIFI_UNDER_SURVEY);
	_rtw_spinunlock_bh(&pmlmepriv->lock);

	/* inform RRM scan complete */
	rm_post_event(padapter, scan_priv->rrm_token, RM_EV_survey_done);
	ret = _SUCCESS;

_exit:
	RTW_INFO(FUNC_ADPT_FMT" takes %d ms to scan %d/%d channels\n",
		FUNC_ADPT_ARG(padapter), param->total_scan_time,
		param->ch_idx + 1, param->ch_num);
	_rtw_scan_abort_check(padapter, __func__);

#ifdef CONFIG_CMD_SCAN
	_free_phl_param(padapter, param);
	pmlmeext->sitesurvey_res.scan_param = NULL;
#else
	rtw_mfree(scan_priv, sizeof(*scan_priv));
#endif
	return ret;
}

static inline bool _is_scan_abort(_adapter *adapter, const char *caller)
{
	struct	mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;

	if (pmlmeext->scan_abort == _FALSE)
		return _FALSE;

	_rtw_spinlock_bh(&pmlmepriv->lock);
	_clr_fwstate_(pmlmepriv, WIFI_UNDER_SURVEY);
	_rtw_spinunlock_bh(&pmlmepriv->lock);
	if (pmlmeext->scan_abort_to) {
		RTW_ERR("%s scan abort timeout\n", caller);
		rtw_warn_on(1);
	}
	_cancel_timer_ex(&pmlmepriv->scan_to_timer);
	RTW_ERR("%s scan abort\n", caller);
	pmlmeext->scan_abort = _FALSE;
	pmlmeext->scan_abort_to = _FALSE;
	return _TRUE;
}

#ifdef CONFIG_P2P
static int p2p_roch_complete_cb(void *priv, struct rtw_phl_scan_param *param)
{
	struct scan_priv *roch_priv = (struct scan_priv *)priv;
	_adapter *padapter = roch_priv->padapter;

	int ret = H2C_SUCCESS;
	struct rtw_wdev_priv *pwdev_priv = adapter_wdev_data(padapter);
	struct cfg80211_wifidirect_info *pcfg80211_wdinfo = &padapter->cfg80211_wdinfo;
	struct wifidirect_info *pwdinfo = &padapter->wdinfo;
	u8 ch, bw, offset;

	mlmeext_set_scan_state(&padapter->mlmeextpriv, SCAN_DISABLE);

	if (_is_scan_abort(padapter, __func__))
		goto _exit;


#if defined(RTW_ROCH_BACK_OP) && defined(CONFIG_CONCURRENT_MODE)
	_cancel_timer_ex(&pwdinfo->ap_p2p_switch_timer);
	ATOMIC_SET(&pwdev_priv->switch_ch_to, 1);
#endif

	if (rtw_mi_get_ch_setting_union(padapter, &ch, &bw, &offset) != 0) {
		if (0)
			RTW_INFO(FUNC_ADPT_FMT" back to linked/linking union - ch:%u, bw:%u, offset:%u\n",
				 FUNC_ADPT_ARG(padapter), ch, bw, offset);
	} else if (adapter_wdev_data(padapter)->p2p_enabled && pwdinfo->listen_channel) {
		ch = pwdinfo->listen_channel;
		bw = CHANNEL_WIDTH_20;
		offset = CHAN_OFFSET_NO_EXT;
		if (0)
			RTW_INFO(FUNC_ADPT_FMT" back to listen ch - ch:%u, bw:%u, offset:%u\n",
				 FUNC_ADPT_ARG(padapter), ch, bw, offset);
	} else {
		ch = roch_priv->restore_ch;
		bw = CHANNEL_WIDTH_20;
		offset = CHAN_OFFSET_NO_EXT;
		if (0)
			RTW_INFO(FUNC_ADPT_FMT" back to restore ch - ch:%u, bw:%u, offset:%u\n",
				 FUNC_ADPT_ARG(padapter), ch, bw, offset);
	}

	set_channel_bwmode(padapter, ch, offset, bw, _FALSE);
	rtw_back_opch(padapter);
	rtw_p2p_set_state(pwdinfo, rtw_p2p_pre_state(pwdinfo));
#ifdef CONFIG_DEBUG_CFG80211
	RTW_INFO("%s, role=%d, p2p_state=%d\n", __func__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo));
#endif

	/* TODO remove; for original p2p state use, phl doesn't need them */
	rtw_cfg80211_set_is_roch(padapter, _FALSE);
	pcfg80211_wdinfo->ro_ch_wdev = NULL;
	rtw_cfg80211_set_last_ro_ch_time(padapter);

	/* callback to cfg80211 */
	rtw_cfg80211_remain_on_channel_expired(roch_priv->wdev
		, roch_priv->cookie
		, &roch_priv->channel
		, roch_priv->channel_type, GFP_KERNEL);

	RTW_INFO("cfg80211_remain_on_channel_expired cookie:0x%llx\n"
		, pcfg80211_wdinfo->remain_on_ch_cookie);

_exit:
#ifdef CONFIG_CMD_SCAN
	_free_phl_param(padapter, param);
	padapter->mlmeextpriv.sitesurvey_res.scan_param = NULL;
#else
	rtw_mfree(roch_priv, sizeof(*roch_priv));
#endif

	return 0;
}

/* inform caller phl_scan are ready on remain channel */
static int roch_ready_cb(void *priv, struct rtw_phl_scan_param *param)
{
	struct scan_priv *roch_priv = (struct scan_priv *)priv;
	_adapter *padapter = roch_priv->padapter;
	struct cfg80211_wifidirect_info *pcfg80211_wdinfo = &padapter->cfg80211_wdinfo;

	rtw_set_oper_ch(padapter, param->scan_ch->channel);

	RTW_INFO("%s:0x%llx\n",  __func__,
		pcfg80211_wdinfo->remain_on_ch_cookie);

	rtw_cfg80211_ready_on_channel(
		roch_priv->wdev,
		roch_priv->cookie,
		&roch_priv->channel,
		roch_priv->type,
		roch_priv->duration,
		GFP_KERNEL);
	return 0;
}

static int p2p_roch_start_cb(void *priv, struct rtw_phl_scan_param *param)
{
	struct scan_priv *roch_priv = (struct scan_priv *)priv;
	_adapter *padapter = roch_priv->padapter;
	struct cfg80211_wifidirect_info *pcfg80211_wdinfo;
	struct wifidirect_info *pwdinfo = &padapter->wdinfo;

#ifdef CONFIG_CMD_SCAN
	padapter->mlmeextpriv.sitesurvey_res.scan_param = param;
#endif
	pcfg80211_wdinfo = &padapter->cfg80211_wdinfo;

	//TODO remove
	mlmeext_set_scan_state(&padapter->mlmeextpriv, SCAN_PROCESS);

	if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)
#if defined(CONFIG_CONCURRENT_MODE) && defined(CONFIG_P2P)
		&& ((padapter->iface_id == padapter->registrypriv.sel_p2p_iface))
#endif
	) {
		rtw_p2p_enable(padapter, P2P_ROLE_DEVICE);
		padapter->wdinfo.listen_channel = param->ch->channel;
		RTW_INFO(FUNC_ADPT_FMT" init listen_channel %u\n"
			, FUNC_ADPT_ARG(padapter), padapter->wdinfo.listen_channel);
	}

	/* TODO remove; for original p2p state use, phl doesn't need them */
	rtw_cfg80211_set_is_roch(padapter, _TRUE);
	pcfg80211_wdinfo->ro_ch_wdev = roch_priv->wdev;
	pcfg80211_wdinfo->remain_on_ch_cookie = roch_priv->cookie;
	pcfg80211_wdinfo->duration = roch_priv->duration;
	rtw_cfg80211_set_last_ro_ch_time(padapter);
	_rtw_memcpy(&pcfg80211_wdinfo->remain_on_ch_channel,
		&roch_priv->channel, sizeof(struct ieee80211_channel));
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
	pcfg80211_wdinfo->remain_on_ch_type = scan_priv->channel_type;
	#endif
	pcfg80211_wdinfo->restore_channel = roch_priv->restore_ch;

	rtw_p2p_set_state(pwdinfo, P2P_STATE_LISTEN);
	return 0;
}

static int roch_start_cb(void *priv, struct rtw_phl_scan_param *param)
{
	_adapter *padapter = (_adapter *)priv;

	mlmeext_set_scan_state(&padapter->mlmeextpriv, SCAN_PROCESS);
#ifdef CONFIG_CMD_SCAN
	padapter->mlmeextpriv.sitesurvey_res.scan_param = param;
#endif
	rtw_cfg80211_set_is_roch(padapter, _TRUE);

	return 0;
}

static int roch_complete_cb(void *priv, struct rtw_phl_scan_param *param)
{
	struct scan_priv *roch_priv = (struct scan_priv *)priv;
	_adapter *padapter = roch_priv->padapter;

	struct rtw_wdev_priv *pwdev_priv = adapter_wdev_data(padapter);
	struct cfg80211_wifidirect_info *pcfg80211_wdinfo = &padapter->cfg80211_wdinfo;
	struct wifidirect_info *pwdinfo = &padapter->wdinfo;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	u8 ch, bw, offset;
	int ret = H2C_SUCCESS;

	mlmeext_set_scan_state(pmlmeext, SCAN_DISABLE);

	if (_is_scan_abort(padapter, __func__))
		goto _exit;

	if (rtw_mi_get_ch_setting_union(padapter, &ch, &bw, &offset) != 0) {
		if (0)
			RTW_INFO(FUNC_ADPT_FMT" back to linked/linking union - ch:%u, bw:%u, offset:%u\n",
				 FUNC_ADPT_ARG(padapter), ch, bw, offset);
	} else {
		ch = roch_priv->restore_ch;
		bw = CHANNEL_WIDTH_20;
		offset = CHAN_OFFSET_NO_EXT;
		if (0)
			RTW_INFO(FUNC_ADPT_FMT" back to restore ch - ch:%u, bw:%u, offset:%u\n",
				 FUNC_ADPT_ARG(padapter), ch, bw, offset);
	}
	/* TODO remove; Should be done in phl scan according to wifi_role */
	set_channel_bwmode(padapter, ch, offset, bw, _FALSE);

	#ifdef CONFIG_P2P
	/* TODO remove; for original p2p state use, phl doesn't need them */
	rtw_cfg80211_set_is_roch(padapter, _FALSE);
	#endif
	/* callback to cfg80211 */
	rtw_cfg80211_remain_on_channel_expired(roch_priv->wdev
		, roch_priv->cookie
		, &roch_priv->channel
		, roch_priv->channel_type, GFP_KERNEL);

	RTW_INFO("cfg80211_remain_on_channel_expired cookie:0x%llx\n"
		, pcfg80211_wdinfo->remain_on_ch_cookie);
_exit:
	#ifdef CONFIG_CMD_SCAN
	_free_phl_param(padapter, param);
	pmlmeext->sitesurvey_res.scan_param = NULL;
	#else
	rtw_mfree(roch_priv, sizeof(*roch_priv));
	#endif
	return 0;
}
#endif /* CONFIG_P2P */

static struct rtw_phl_scan_ops scan_ops_rrm_cb = {
	.scan_start = scan_start_cb,
	.scan_ch_ready = scan_ch_ready_cb,
	.scan_complete = scan_complete_rrm_cb,
	.scan_issue_pbreq = scan_issue_pbreq_cb,
	/*.scan_issue_null_data = scan_issu_null_data_cb*/
};
#endif /* CONFIG_RTW_80211K */

static struct rtw_phl_scan_ops scan_ops_cb = {
	.scan_start = scan_start_cb,
	.scan_ch_ready = scan_ch_ready_cb,
	.scan_complete = scan_complete_cb,
	.scan_issue_pbreq = scan_issue_pbreq_cb,
	/* .scan_issue_null_data = scan_issu_null_data_cb */
};

#ifdef CONFIG_P2P
/* p2p remain on channel */
static struct rtw_phl_scan_ops p2p_remain_ops_cb = {
	.scan_start = p2p_roch_start_cb,
	.scan_ch_ready = roch_ready_cb,
	.scan_complete = p2p_roch_complete_cb,
};

/* normal remain on channel */
static struct rtw_phl_scan_ops remain_ops_cb = {
	.scan_start = roch_start_cb,
	.scan_ch_ready = roch_ready_cb,
	.scan_complete = roch_complete_cb,
	/* .scan_issue_null_data = scan_issu_null_data_cb */
};
#endif /* CONFIG_P2P */

u8 rtw_sitesurvey_cmd(_adapter *padapter, struct sitesurvey_parm *pparm)
{
	u8 res = _FAIL;
	u8 i;
	u32 scan_timeout_ms;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct rtw_phl_scan_param *phl_param = NULL;
	struct rtw_ieee80211_channel ch[RTW_CHANNEL_SCAN_AMOUNT] = {{0}};
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct sitesurvey_parm *tmp_parm = NULL;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct ss_res *ss = &pmlmeext->sitesurvey_res;
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);

	if (pparm == NULL) {
		tmp_parm = rtw_zmalloc(sizeof(struct sitesurvey_parm));
		if (tmp_parm == NULL) {
			RTW_ERR("%s alloc tmp_parm fail\n", __func__);
			goto _err_exit;
		}
		rtw_init_sitesurvey_parm(padapter, tmp_parm);
		pparm = tmp_parm;
	}

	/* backup original ch list */
	_rtw_memcpy(ch, pparm->ch,
		sizeof(struct rtw_ieee80211_channel) * pparm->ch_num);

	/* modify ch list according to chanel plan */
	pparm->ch_num = rtw_scan_ch_decision(padapter,
					pparm->ch, RTW_CHANNEL_SCAN_AMOUNT,
					ch, pparm->ch_num, pparm->acs);

	if (pparm->duration == 0) {
		if (pparm->acs == 1)
			pparm->duration = 300; /* ms */
		else if(pparm->acs == 2)  /* quick scan enabled */
 			pparm->duration = SURVEY_TO; /* ms */
		else
			pparm->duration = SURVEY_TO; /* ms */
	}

	/*create mem of PHL Scan parameter*/

	phl_param = _alloc_phl_param(padapter, pparm->ch_num);
	if (phl_param == NULL) {
		RTW_ERR("%s alloc phl_param fail\n", __func__);
		goto _err_param;
	}

	/* STEP_1 transfer to rtw channel list to phl channel list */
	scan_channel_list_filled(padapter, phl_param, pparm);

	#ifdef CONFIG_RTW_ACS
	if (pparm->acs) {
		acs_parm_init(padapter, phl_param->ch_num, phl_param->ch);
		phl_param->acs = _TRUE;
		phl_param->nhm_include_cca = _TRUE;
		pparm->scan_type = RTW_SCAN_ACS;
	}
	#endif

	/* STEP_2 copy the ssid info to phl param */
	phl_param->ssid_num = rtw_min(pparm->ssid_num, SCAN_SSID_AMOUNT);
	for (i = 0; i < phl_param->ssid_num; ++i) {
		phl_param->ssid[i].ssid_len = pparm->ssid[i].SsidLength;
		_rtw_memcpy(&phl_param->ssid[i].ssid, &pparm->ssid[i].Ssid, phl_param->ssid[i].ssid_len);
	}

#ifdef RTW_WKARD_CMD_SCAN_EXTEND_ACTIVE_SCAN
	/* STEP_2.1 set EXT_ACT_SCAN_ENABLE for hidden AP scan */
	if (phl_param->ssid[0].ssid_len) {
		phl_param->ext_act_scan_period = RTW_EXTEND_ACTIVE_SCAN_PERIOD;
		for (i = 0; i < phl_param->ch_num; i++) {
			int chset_idx;
			chset_idx = rtw_chset_search_ch(rfctl->channel_set,
							phl_param->ch[i].channel);
			if (chset_idx < 0) {
				RTW_ERR(FUNC_ADPT_FMT ": cann't find ch %u in chset!\n",
					FUNC_ADPT_ARG(padapter), phl_param->ch[i].channel);
				continue;
			}

			if ((phl_param->ch[i].type == RTW_PHL_SCAN_PASSIVE)
				&& (!IS_DFS_SLAVE_WITH_RD(rfctl)
				|| rtw_rfctl_dfs_domain_unknown_g6(rfctl)
				|| !CH_IS_NON_OCP(&rfctl->channel_set[chset_idx]))) {
				phl_param->ch[i].ext_act_scan = EXT_ACT_SCAN_ENABLE;
			}
		}
	}
#endif /* RTW_WKARD_CMD_SCAN_EXTEND_ACTIVE_SCAN */


	/* STEP_3 set ops according to scan_type */
	switch (pparm->scan_type) {
	#ifdef CONFIG_P2P
	case RTW_SCAN_P2P:
		phl_param->ops = &scan_ops_p2p_cb;
	break;
	#endif

	#ifdef CONFIG_RTW_80211K
	case RTW_SCAN_RRM:
		phl_param->ops = &scan_ops_rrm_cb;
		if (pparm->ch_num > 13) {
			phl_param->back_op_mode = SCAN_BKOP_CNT;
			phl_param->back_op_ch_cnt = 3;
			phl_param->back_op_ch_dur_ms = SURVEY_TO;
		}
	break;
	#endif

	case RTW_SCAN_ACS:
		phl_param->ops = &scan_ops_cb;
	break;

	case RTW_SCAN_NORMAL:
	default:
		phl_param->ops = &scan_ops_cb;
		phl_param->back_op_mode = SCAN_BKOP_CNT;
		phl_param->back_op_ch_cnt = 3;
		phl_param->back_op_ch_dur_ms = SURVEY_TO;
	break;
	}

	phl_param->ch_idx = -1;

	/* STEP_4 reset variables for each scan */
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
		rfctl->channel_set[i].hidden_bss_cnt = 0;

	set_fwstate(pmlmepriv, WIFI_UNDER_SURVEY);

	if(rtw_phl_cmd_scan_request(dvobj->phl, phl_param, true) != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR("%s request scam_cmd failed\n", __func__);
		_clr_fwstate_(pmlmepriv, WIFI_UNDER_SURVEY);
		goto _err_req_param;
	}
	rtw_free_network_queue(padapter, _FALSE);

	pmlmepriv->scan_start_time = rtw_get_current_time();
	ss->duration = pparm->duration;
	scan_timeout_ms = rtw_scan_timeout_decision(padapter, phl_param);
	mlme_set_scan_to_timer(pmlmepriv,scan_timeout_ms);

	rtw_led_control(padapter, LED_CTL_SITE_SURVEY);
	if (tmp_parm)
		rtw_mfree(tmp_parm, sizeof(*tmp_parm));
	res = _SUCCESS;
	return res;

_err_req_param:
	_free_phl_param(padapter, phl_param);
_err_param:
	if (tmp_parm)
		rtw_mfree(tmp_parm, sizeof(*tmp_parm));
_err_exit:
	rtw_warn_on(1);
	return res;
}

#if defined(CONFIG_IOCTL_CFG80211) && defined(CONFIG_P2P)
u8 rtw_phl_remain_on_ch_cmd(_adapter *padapter,
	u64 cookie, struct wireless_dev *wdev,
	struct ieee80211_channel *ch, u8 ch_type,
	unsigned int duration, struct back_op_param *bkop_parm,
	u8 is_p2p)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_phl_scan_param *phl_param = NULL;
	struct scan_priv *scan_priv = NULL;
	u16 remain_ch;
	u8 chan_num;
	u8 res = _FAIL;

	/* prepare remain channel - check channel */
	remain_ch = (u16)ieee80211_frequency_to_channel(ch->center_freq);
	if (roch_stay_in_cur_chan(padapter) == _TRUE) { /*???*/
		remain_ch = rtw_mi_get_union_chan(padapter);
		RTW_INFO(FUNC_ADPT_FMT" stay in union ch:%d\n",
			FUNC_ADPT_ARG(padapter), remain_ch);
	}
	chan_num = 1;

	phl_param = _alloc_phl_param(padapter, chan_num);
	if (phl_param == NULL) {
		RTW_ERR("%s alloc phl_param fail\n", __func__);
		goto _err_exit;
	}

	/*** fill phl parameter - scan_priv ***/
	scan_priv = (struct scan_priv *)phl_param->priv;
	scan_priv->padapter = padapter;
	scan_priv->wdev = wdev;
	_rtw_memcpy(&scan_priv->channel, ch, sizeof(*ch));
	scan_priv->channel_type = ch_type;
	scan_priv->cookie = cookie;
	scan_priv->duration = duration;
	scan_priv->restore_ch = rtw_get_oper_ch(padapter);

	/* fill phl param - chan */
	phl_param->ch->channel = remain_ch;
	phl_param->ch->duration = duration;
	phl_param->ch->scan_mode = NORMAL_SCAN_MODE;
	phl_param->ch->bw = CHANNEL_WIDTH_20;
	phl_param->ch_num = chan_num;

	/* fill back op param */
	//phl_param->back_op_mode = bkop_parm->mode;
	phl_param->back_op_mode = SCAN_BKOP_TIMER;
	phl_param->back_op_ch_dur_ms = bkop_parm->on_ch_dur;
	phl_param->back_op_off_ch_dur_ms = bkop_parm->off_ch_dur;
	phl_param->back_op_off_ch_ext_dur_ms = bkop_parm->off_ch_ext_dur;

#ifdef CONFIG_P2P
	/* set ops according to is_p2p */
	if (is_p2p)
		phl_param->ops = &p2p_remain_ops_cb;
	else
#endif /* CONFIG_P2P */
		phl_param->ops = &remain_ops_cb;

	if(rtw_phl_cmd_scan_request(dvobj->phl, phl_param, true) == RTW_PHL_STATUS_FAILURE) {
		RTW_ERR("%s request scam_cmd failed\n", __func__);
		goto _err_req_param;
	}

	RTW_INFO(FUNC_ADPT_FMT" ch:%u duration:%d, cookie:0x%llx\n"
			, FUNC_ADPT_ARG(padapter), remain_ch,	duration, cookie);
	res = _SUCCESS;
	return res;

_err_req_param:
	_free_phl_param(padapter, phl_param);
_err_exit:
	rtw_warn_on(1);
	return res;
}

#endif /*CONFIG_IOCTL_CFG80211*/


#else /* CONFIG_CMD_SCAN */
/*
rtw_sitesurvey_cmd(~)
	### NOTE:#### (!!!!)
	MUST TAKE CARE THAT BEFORE CALLING THIS FUNC, YOU SHOULD HAVE LOCKED pmlmepriv->lock
*/
u8 rtw_sitesurvey_cmd(_adapter *padapter, struct sitesurvey_parm *pparm)
{
	u8 res = _FAIL;
	struct cmd_obj		*cmd;
	struct sitesurvey_parm	*psurveyPara;
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct ss_res *ss = &pmlmeext->sitesurvey_res;

#ifdef CONFIG_LPS
	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE)
		rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_SCAN, 0);
#endif

#ifdef CONFIG_P2P_PS
	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE)
		p2p_ps_wk_cmd(padapter, P2P_PS_SCAN, 1);
#endif /* CONFIG_P2P_PS */

	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL)
		return _FAIL;
	cmd->padapter = padapter;

	psurveyPara = (struct sitesurvey_parm *)rtw_zmalloc(sizeof(struct sitesurvey_parm));
	if (psurveyPara == NULL) {
		rtw_mfree((unsigned char *) cmd, sizeof(struct cmd_obj));
		return _FAIL;
	}

	if (pparm)
		_rtw_memcpy(psurveyPara, pparm, sizeof(struct sitesurvey_parm));
	else
		psurveyPara->scan_mode = pmlmepriv->scan_mode;

	if (psurveyPara->acs)
		rtw_free_network_queue(padapter, _TRUE);
	else
		rtw_free_network_queue(padapter, _FALSE);

	init_h2fwcmd_w_parm_no_rsp(cmd, psurveyPara, CMD_SITE_SURVEY);

	set_fwstate(pmlmepriv, WIFI_UNDER_SURVEY);

	res = rtw_enqueue_cmd(pcmdpriv, cmd);

	if (res == _SUCCESS) {
		u32 scan_timeout_ms;
		ss->duration = psurveyPara->duration;
		pmlmepriv->scan_start_time = rtw_get_current_time();
		scan_timeout_ms = rtw_scan_timeout_decision(padapter);
		mlme_set_scan_to_timer(pmlmepriv,scan_timeout_ms);
#ifdef CONFIG_RTW_SW_LED
		rtw_led_control(padapter, LED_CTL_SITE_SURVEY);
#endif
	} else
		_clr_fwstate_(pmlmepriv, WIFI_UNDER_SURVEY);


	return res;
}

#endif /* CONFIG_CMD_SCAN */

void rtw_readtssi_cmdrsp_callback(_adapter	*padapter,  struct cmd_obj *pcmd)
{

	rtw_mfree((unsigned char *) pcmd->parmbuf, pcmd->cmdsz);
	rtw_mfree((unsigned char *) pcmd, sizeof(struct cmd_obj));

#ifdef CONFIG_MP_INCLUDED
	if (padapter->registrypriv.mp_mode == 1)
		padapter->mppriv.workparam.bcompleted = _TRUE;
#endif

}

static u8 rtw_createbss_cmd(_adapter  *adapter, int flags, bool adhoc
	, u16 ifbmp, u16 excl_ifbmp, s16 req_ch, s8 req_bw, s8 req_offset, u8 is_change_chbw)
{
	struct cmd_obj *cmdobj;
	struct createbss_parm *parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct submit_ctx sctx;
	u8 res = _SUCCESS;

	if (req_ch > 0 && req_bw >= 0 && req_offset >= 0) {
		if (!rtw_chset_is_chbw_valid(adapter_to_chset(adapter), req_ch, req_bw, req_offset, 0, 0)) {
			res = _FAIL;
			goto exit;
		}
	}

	/* prepare cmd parameter */
	parm = (struct createbss_parm *)rtw_zmalloc(sizeof(*parm));
	if (parm == NULL) {
		res = _FAIL;
		goto exit;
	}

	if (adhoc) {
		/* for now, adhoc doesn't support ch,bw,offset request */
		parm->adhoc = 1;
	} else {
		parm->adhoc = 0;
		parm->ifbmp = ifbmp;
		parm->excl_ifbmp = excl_ifbmp;
		parm->req_ch = req_ch;
		parm->req_bw = req_bw;
		parm->req_offset = req_offset;
		parm->is_change_chbw = is_change_chbw;
	}

	if (flags & RTW_CMDF_DIRECTLY) {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (H2C_SUCCESS != createbss_hdl(adapter, (u8 *)parm))
			res = _FAIL;
		rtw_mfree((u8 *)parm, sizeof(*parm));
	} else {
		/* need enqueue, prepare cmd_obj and enqueue */
		cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(*cmdobj));
		if (cmdobj == NULL) {
			res = _FAIL;
			rtw_mfree((u8 *)parm, sizeof(*parm));
			goto exit;
		}
		cmdobj->padapter = adapter;

		init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_CREATE_BSS);

		if (flags & RTW_CMDF_WAIT_ACK) {
			cmdobj->sctx = &sctx;
			rtw_sctx_init(&sctx, 2000);
		}

		res = rtw_enqueue_cmd(pcmdpriv, cmdobj);

		if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				cmdobj->sctx = NULL;
			_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
		}
	}

exit:
	return res;
}

inline u8 rtw_create_ibss_cmd(_adapter *adapter, int flags)
{
	return rtw_createbss_cmd(adapter, flags
		, 1
		, 0, 0
		, 0, REQ_BW_NONE, REQ_OFFSET_NONE /* for now, adhoc doesn't support ch,bw,offset request */
		, _FALSE
	);
}

inline u8 rtw_startbss_cmd(_adapter *adapter, int flags)
{
	return rtw_createbss_cmd(adapter, flags
		, 0
		, BIT(adapter->iface_id), 0
		, 0, REQ_BW_NONE, REQ_OFFSET_NONE /* excute entire AP setup cmd */
		, _FALSE
	);
}

inline u8 rtw_change_bss_chbw_cmd(_adapter *adapter, int flags
	, u16 ifbmp, u16 excl_ifbmp, s16 req_ch, s8 req_bw, s8 req_offset)
{
	return rtw_createbss_cmd(adapter, flags
		, 0
		, ifbmp, excl_ifbmp
		, req_ch, req_bw, req_offset
		, _TRUE
	);
}

#ifdef CONFIG_RTW_80211R
static void rtw_ft_validate_akm_type(_adapter  *padapter,
	struct wlan_network *pnetwork)
{
	struct security_priv *psecuritypriv = &(padapter->securitypriv);
	struct ft_roam_info *pft_roam = &(padapter->mlmepriv.ft_roam);
	u32 tmp_len;
	u8 *ptmp;

	/*IEEE802.11-2012 Std. Table 8-101-AKM suite selectors*/
	if (rtw_ft_valid_akm(padapter, psecuritypriv->rsn_akm_suite_type)) {
		ptmp = rtw_get_ie(&pnetwork->network.IEs[12],
				_MDIE_, &tmp_len, (pnetwork->network.IELength-12));
		if (ptmp) {
			pft_roam->mdid = *(u16 *)(ptmp+2);
			pft_roam->ft_cap = *(ptmp+4);

			RTW_INFO("FT: target " MAC_FMT " mdid=(0x%2x), capacity=(0x%2x)\n",
				MAC_ARG(pnetwork->network.MacAddress), pft_roam->mdid, pft_roam->ft_cap);
			rtw_ft_set_flags(padapter, RTW_FT_PEER_EN);

			if (rtw_ft_otd_roam_en(padapter))
				rtw_ft_set_flags(padapter, RTW_FT_PEER_OTD_EN);
		} else {
			/* Don't use FT roaming if target AP cannot support FT */
			rtw_ft_clr_flags(padapter, (RTW_FT_PEER_EN|RTW_FT_PEER_OTD_EN));
			rtw_ft_reset_status(padapter);
		}
	} else {
		/* It could be a non-FT connection */
		rtw_ft_clr_flags(padapter, (RTW_FT_PEER_EN|RTW_FT_PEER_OTD_EN));
		rtw_ft_reset_status(padapter);
	}
}
#endif

u8 rtw_joinbss_cmd(_adapter  *padapter, struct wlan_network *pnetwork)
{
	u8	*auth, res = _SUCCESS;
	uint	t_len = 0;
	WLAN_BSSID_EX		*psecnetwork;
	struct cmd_obj		*pcmd;
	struct cmd_priv		*pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct qos_priv		*pqospriv = &pmlmepriv->qospriv;
	struct security_priv	*psecuritypriv = &padapter->securitypriv;
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
#ifdef CONFIG_80211N_HT
	struct ht_priv			*phtpriv = &pmlmepriv->htpriv;
#endif /* CONFIG_80211N_HT */
#ifdef CONFIG_80211AC_VHT
	struct vht_priv		*pvhtpriv = &pmlmepriv->vhtpriv;
#endif /* CONFIG_80211AC_VHT */
#ifdef CONFIG_80211AX_HE
	struct he_priv		*phepriv = &pmlmepriv->hepriv;
#endif /* CONFIG_80211AX_HE */
	NDIS_802_11_NETWORK_INFRASTRUCTURE ndis_network_mode = pnetwork->network.InfrastructureMode;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct registry_priv *regsty = adapter_to_regsty(padapter);
	u32 tmp_len;
	u8 *ptmp = NULL;

#ifdef CONFIG_RTW_SW_LED
	rtw_led_control(padapter, LED_CTL_START_TO_LINK);
#endif

	pcmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	pcmd->padapter = padapter;

	/* for IEs is fix buf size */
	t_len = sizeof(WLAN_BSSID_EX);


	/* for hidden ap to set fw_state here */
	if (!MLME_IS_STA(padapter) || !MLME_IS_ADHOC(padapter)) {
		switch (ndis_network_mode) {
		case Ndis802_11IBSS:
			set_fwstate(pmlmepriv, WIFI_ADHOC_STATE);
			break;

		case Ndis802_11Infrastructure:
			set_fwstate(pmlmepriv, WIFI_STATION_STATE);
			break;

		default:
			rtw_warn_on(1);
			break;
		}
	}

	pmlmeinfo->assoc_AP_vendor = check_assoc_AP(pnetwork->network.IEs, pnetwork->network.IELength);

#ifdef CONFIG_80211AC_VHT
	/* save AP beamform_cap info for BCM IOT issue */
	if (pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_BROADCOM)
		pvhtpriv->ap_is_mu_bfer =
			get_vht_mu_bfer_cap(pnetwork->network.IEs,
				pnetwork->network.IELength);
#endif
	/*
		Modified by Arvin 2015/05/13
		Solution for allocating a new WLAN_BSSID_EX to avoid race condition issue between disconnect and joinbss
	*/
	psecnetwork = (WLAN_BSSID_EX *)rtw_zmalloc(sizeof(WLAN_BSSID_EX));
	if (psecnetwork == NULL) {
		if (pcmd != NULL)
			rtw_mfree((unsigned char *)pcmd, sizeof(struct	cmd_obj));

		res = _FAIL;


		goto exit;
	}

	_rtw_memset(psecnetwork, 0, t_len);

	_rtw_memcpy(psecnetwork, &pnetwork->network, get_WLAN_BSSID_EX_sz(&pnetwork->network));

	auth = &psecuritypriv->authenticator_ie[0];
	psecuritypriv->authenticator_ie[0] = (unsigned char)psecnetwork->IELength;

	if ((psecnetwork->IELength - 12) < (256 - 1))
		_rtw_memcpy(&psecuritypriv->authenticator_ie[1], &psecnetwork->IEs[12], psecnetwork->IELength - 12);
	else
		_rtw_memcpy(&psecuritypriv->authenticator_ie[1], &psecnetwork->IEs[12], (256 - 1));

	psecnetwork->IELength = 0;
	/* Added by Albert 2009/02/18 */
	/* If the the driver wants to use the bssid to create the connection. */
	/* If not,  we have to copy the connecting AP's MAC address to it so that */
	/* the driver just has the bssid information for PMKIDList searching. */

	if (pmlmepriv->assoc_by_bssid == _FALSE)
		_rtw_memcpy(&pmlmepriv->assoc_bssid[0], &pnetwork->network.MacAddress[0], ETH_ALEN);

	/* copy fixed ie */
	_rtw_memcpy(psecnetwork->IEs, pnetwork->network.IEs, 12);
	psecnetwork->IELength = 12;

	psecnetwork->IELength += rtw_restruct_sec_ie(padapter, psecnetwork->IEs + psecnetwork->IELength);


	pqospriv->qos_option = 0;

	if (pregistrypriv->wmm_enable) {
#ifdef CONFIG_WMMPS_STA
		rtw_uapsd_use_default_setting(padapter);
#endif /* CONFIG_WMMPS_STA */
		tmp_len = rtw_restruct_wmm_ie(padapter, &pnetwork->network.IEs[0], &psecnetwork->IEs[0], pnetwork->network.IELength, psecnetwork->IELength);

		if (psecnetwork->IELength != tmp_len) {
			psecnetwork->IELength = tmp_len;
			pqospriv->qos_option = 1; /* There is WMM IE in this corresp. beacon */
		} else {
			pqospriv->qos_option = 0;/* There is no WMM IE in this corresp. beacon */
		}
	}

#ifdef CONFIG_80211N_HT
	phtpriv->ht_option = _FALSE;
	if (pregistrypriv->ht_enable && is_supported_ht(pregistrypriv->wireless_mode)) {
		ptmp = rtw_get_ie(&pnetwork->network.IEs[12], _HT_CAPABILITY_IE_, &tmp_len, pnetwork->network.IELength - 12);
		if (ptmp && tmp_len > 0) {
			/*	Added by Albert 2010/06/23 */
			/*	For the WEP mode, we will use the bg mode to do the connection to avoid some IOT issue. */
			/*	Especially for Realtek 8192u SoftAP. */
			if ((padapter->securitypriv.dot11PrivacyAlgrthm != _WEP40_) &&
			    (padapter->securitypriv.dot11PrivacyAlgrthm != _WEP104_) &&
			    (padapter->securitypriv.dot11PrivacyAlgrthm != _TKIP_)) {
				rtw_ht_use_default_setting(padapter);
#ifdef CONFIG_RTW_AP_EXT_SUPPORT
				rtw_ht_apply_mib_setting(padapter);
#endif

				/* rtw_restructure_ht_ie */
				rtw_restructure_ht_ie(padapter, &pnetwork->network.IEs[12], &psecnetwork->IEs[0],
					pnetwork->network.IELength - 12, &psecnetwork->IELength,
					pnetwork->network.Configuration.DSConfig);
			}
		}
	}

#ifdef CONFIG_80211AC_VHT
	pvhtpriv->vht_option = _FALSE;
	if (phtpriv->ht_option
		&& REGSTY_IS_11AC_ENABLE(pregistrypriv)
		&& is_supported_vht(pregistrypriv->wireless_mode)
		&& (!rfctl->country_ent || COUNTRY_CHPLAN_EN_11AC(rfctl->country_ent))
		&& ((padapter->registrypriv.wifi_spec == 0) || (pnetwork->network.Configuration.DSConfig > 14))
	) {
		rtw_restructure_vht_ie(padapter, &pnetwork->network.IEs[0], &psecnetwork->IEs[0],
			pnetwork->network.IELength, &psecnetwork->IELength);
	}

#ifdef CONFIG_80211AX_HE
	phepriv->he_option = _FALSE;
	if ((pvhtpriv->vht_option || (is_supported_24g(regsty->band_type) && rtw_hw_chk_band_cap(dvobj, BAND_CAP_2G)))
		&& REGSTY_IS_11AX_ENABLE(pregistrypriv)
		&& is_supported_he(pregistrypriv->wireless_mode)
		/* CONFIG_80211AX_HE_TODO
		&& (!rfctl->country_ent || COUNTRY_CHPLAN_EN_11AX(rfctl->country_ent)) */
		&& ((padapter->registrypriv.wifi_spec == 0) /*|| (pnetwork->network.Configuration.DSConfig > 14)*/)
	) {
		rtw_restructure_he_ie(padapter, &pnetwork->network.IEs[0], &psecnetwork->IEs[0],
			pnetwork->network.IELength, &psecnetwork->IELength);
		}
#endif /* CONFIG_80211AX_HE */

#endif /* CONFIG_80211AC_VHT */
#endif /* CONFIG_80211N_HT */

	rtw_append_exented_cap(padapter, &psecnetwork->IEs[0], &psecnetwork->IELength);

#ifdef CONFIG_RTW_80211R
	rtw_ft_validate_akm_type(padapter, pnetwork);
#endif

#if 0
	psecuritypriv->supplicant_ie[0] = (u8)psecnetwork->IELength;

	if (psecnetwork->IELength < (256 - 1))
		_rtw_memcpy(&psecuritypriv->supplicant_ie[1], &psecnetwork->IEs[0], psecnetwork->IELength);
	else
		_rtw_memcpy(&psecuritypriv->supplicant_ie[1], &psecnetwork->IEs[0], (256 - 1));
#endif

	pcmd->cmdsz = sizeof(WLAN_BSSID_EX);

	_rtw_init_listhead(&pcmd->list);
	pcmd->cmdcode = CMD_JOINBSS; /*_JoinBss_CMD_;*/
	pcmd->parmbuf = (unsigned char *)psecnetwork;
	pcmd->rsp = NULL;
	pcmd->rspsz = 0;

	res = rtw_enqueue_cmd(pcmdpriv, pcmd);

exit:


	return res;
}

u8 rtw_disassoc_cmd(_adapter *padapter, u32 deauth_timeout_ms, int flags) /* for sta_mode */
{
	struct cmd_obj *cmdobj = NULL;
	struct disconnect_parm *param = NULL;
	struct cmd_priv *cmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;

	struct submit_ctx sctx;
	u8 res = _SUCCESS;

	/* prepare cmd parameter */
	param = (struct disconnect_parm *)rtw_zmalloc(sizeof(*param));
	if (param == NULL) {
		res = _FAIL;
		goto exit;
	}
	param->deauth_timeout_ms = deauth_timeout_ms;

	if (flags & RTW_CMDF_DIRECTLY) {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (disconnect_hdl(padapter, (u8 *)param) != H2C_SUCCESS)
			res = _FAIL;
		rtw_mfree((u8 *)param, sizeof(*param));

	} else {
		cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(*cmdobj));
		if (cmdobj == NULL) {
			res = _FAIL;
			rtw_mfree((u8 *)param, sizeof(*param));
			goto exit;
		}
		cmdobj->padapter = padapter;
		init_h2fwcmd_w_parm_no_rsp(cmdobj, param, CMD_DISCONNECT);
		if (flags & RTW_CMDF_WAIT_ACK) {
			cmdobj->sctx = &sctx;
			rtw_sctx_init(&sctx, 2000);
		}
		res = rtw_enqueue_cmd(cmdpriv, cmdobj);
		if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&cmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				cmdobj->sctx = NULL;
			_rtw_mutex_unlock(&cmdpriv->sctx_mutex);
		}
	}

exit:


	return res;
}


u8 rtw_stop_ap_cmd(_adapter *adapter, u8 flags)
{
#ifdef CONFIG_AP_MODE
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct submit_ctx sctx;
	u8 res = _SUCCESS;

	if (flags & RTW_CMDF_DIRECTLY) {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (H2C_SUCCESS != stop_ap_hdl(adapter))
			res = _FAIL;
	} else {
		parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
		if (parm == NULL) {
			res = _FAIL;
			goto exit;
		}

		parm->ec_id = STOP_AP_WK_CID;
		parm->type = 0;
		parm->size = 0;
		parm->pbuf = NULL;

		/* need enqueue, prepare cmd_obj and enqueue */
		cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(*cmdobj));
		if (cmdobj == NULL) {
			rtw_mfree(parm, sizeof(struct drvextra_cmd_parm));
			res = _FAIL;
			goto exit;
		}
		cmdobj->padapter = adapter;

		init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_SET_DRV_EXTRA);

		if (flags & RTW_CMDF_WAIT_ACK) {
			cmdobj->sctx = &sctx;
			rtw_sctx_init(&sctx, 2000);
		}

		res = rtw_enqueue_cmd(pcmdpriv, cmdobj);

		if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				cmdobj->sctx = NULL;
			_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
		}
	}

exit:
	return res;
#endif
}

#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
u8 rtw_tx_control_cmd(_adapter *adapter)
{
	struct cmd_obj *cmd;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;

	u8 res = _SUCCESS;

	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL){
		res = _FAIL;
		goto exit;
	}
	cmd->padapter = adapter;

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = TBTX_CONTROL_TX_WK_CID;
	pdrvextra_cmd_parm->type = 0;
	pdrvextra_cmd_parm->size = 0;
	pdrvextra_cmd_parm->pbuf = NULL;
	init_h2fwcmd_w_parm_no_rsp(cmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);

	res = rtw_enqueue_cmd(pcmdpriv, cmd);

exit:
	return res;
}
#endif

u8 rtw_setopmode_cmd(_adapter  *adapter, NDIS_802_11_NETWORK_INFRASTRUCTURE networktype, u8 flags)
{
	struct cmd_obj *cmdobj;
	struct setopmode_parm *parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct submit_ctx sctx;
	u8 res = _SUCCESS;

	/* prepare cmd parameter */
	parm = (struct setopmode_parm *)rtw_zmalloc(sizeof(*parm));
	if (parm == NULL) {
		res = _FAIL;
		goto exit;
	}
	parm->mode = (u8)networktype;

	if (flags & RTW_CMDF_DIRECTLY) {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (H2C_SUCCESS != setopmode_hdl(adapter, (u8 *)parm))
			res = _FAIL;
		rtw_mfree((u8 *)parm, sizeof(*parm));
	} else {
		/* need enqueue, prepare cmd_obj and enqueue */
		cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(*cmdobj));
		if (cmdobj == NULL) {
			res = _FAIL;
			rtw_mfree((u8 *)parm, sizeof(*parm));
			goto exit;
		}
		cmdobj->padapter = adapter;

		init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_SET_OPMODE);

		if (flags & RTW_CMDF_WAIT_ACK) {
			cmdobj->sctx = &sctx;
			rtw_sctx_init(&sctx, 2000);
		}

		res = rtw_enqueue_cmd(pcmdpriv, cmdobj);

		if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				cmdobj->sctx = NULL;
			_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
		}
	}

exit:
	return res;
}

#ifdef CONFIG_CMD_DISP
u8 rtw_setstakey_cmd(_adapter *padapter, struct sta_info *sta, u8 key_type, bool enqueue)
{
	struct cmd_obj			*ph2c;
	struct set_stakey_parm	setstakey_para;
	struct set_stakey_rsp		*psetstakey_rsp = NULL;

	struct mlme_priv			*pmlmepriv = &padapter->mlmepriv;
	struct security_priv		*psecuritypriv = &padapter->securitypriv;
	u8 key_len =16;
	u8	res = _SUCCESS;

	_rtw_memset(&setstakey_para, 0, sizeof(struct set_stakey_parm));
	_rtw_memcpy(setstakey_para.addr, sta->phl_sta->mac_addr, ETH_ALEN);

	if (MLME_IS_STA(padapter))
		setstakey_para.algorithm = (unsigned char) psecuritypriv->dot11PrivacyAlgrthm;
	else
		GET_ENCRY_ALGO(psecuritypriv, sta, setstakey_para.algorithm, _FALSE);

	if ((setstakey_para.algorithm == _GCMP_256_) || (setstakey_para.algorithm == _CCMP_256_))
		key_len = 32;

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	if ((setstakey_para.algorithm == _SMS4_) || (setstakey_para.algorithm == _GCM_SM4_)) {
		key_len = 32;
		setstakey_para.keyid = sta->wapiStaInfo.keyIdx;
	}
#endif

	if (key_type == GROUP_KEY) {
		_rtw_memcpy(&setstakey_para.key, &psecuritypriv->dot118021XGrpKey[psecuritypriv->dot118021XGrpKeyid].skey, key_len);
		setstakey_para.gk = 1;
		setstakey_para.keyid = (u8)psecuritypriv->dot118021XGrpKeyid;
	} else if (key_type == UNICAST_KEY)
		_rtw_memcpy(&setstakey_para.key, &sta->dot118021x_UncstKey, key_len);
#ifdef CONFIG_TDLS
	else if (key_type == TDLS_KEY) {
		_rtw_memcpy(&setstakey_para.key, sta->tpk.tk, key_len);
		setstakey_para.algorithm = (u8)sta->dot118021XPrivacy;
	}
#endif /* CONFIG_TDLS */

	/* jeff: set this becasue at least sw key is ready */
	padapter->securitypriv.busetkipkey = _TRUE;

	if (enqueue) {
		set_stakey_hdl(padapter, &setstakey_para, PHL_CMD_NO_WAIT, 0);
	} else {
		set_stakey_hdl(padapter, &setstakey_para, PHL_CMD_DIRECTLY, 0);
	}
exit:


	return res;
}

u8 rtw_clearstakey_cmd(_adapter *padapter, struct sta_info *sta, u8 enqueue)
{
	u8	res = _SUCCESS;

	if (!sta) {
		RTW_ERR("%s sta == NULL\n", __func__);
		goto exit;
	}

	if (!enqueue)
		rtw_hw_del_all_key(padapter, sta, PHL_CMD_DIRECTLY, 0);
	else
		rtw_hw_del_all_key(padapter, sta, PHL_CMD_NO_WAIT, 0);

exit:
	return res;
}
#else
u8 rtw_setstakey_cmd(_adapter *padapter, struct sta_info *sta, u8 key_type, bool enqueue)
{
	struct cmd_obj			*pcmd;
	struct set_stakey_parm	*psetstakey_para;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	struct set_stakey_rsp *psetstakey_rsp = NULL;

	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	u8 key_len =16;
	u8 res = _SUCCESS;


	psetstakey_para = (struct set_stakey_parm *)rtw_zmalloc(sizeof(struct set_stakey_parm));
	if (psetstakey_para == NULL) {
		res = _FAIL;
		goto exit;
	}

	_rtw_memcpy(psetstakey_para->addr, sta->phl_sta->mac_addr, ETH_ALEN);

	if (MLME_IS_STA(padapter))
		psetstakey_para->algorithm = (unsigned char) psecuritypriv->dot11PrivacyAlgrthm;
	else
		GET_ENCRY_ALGO(psecuritypriv, sta, psetstakey_para->algorithm, _FALSE);

	if ((psetstakey_para->algorithm == _GCMP_256_) || (psetstakey_para->algorithm == _CCMP_256_))
		key_len = 32;

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	if ((psetstakey_para->algorithm == _SMS4_) || (psetstakey_para->algorithm == _GCM_SM4_)) {
		key_len = 32;
		psetstakey_para->keyid = sta->wapiStaInfo.keyIdx;
	}
#endif

	if (key_type == GROUP_KEY) {
		_rtw_memcpy(&psetstakey_para->key, &psecuritypriv->dot118021XGrpKey[psecuritypriv->dot118021XGrpKeyid].skey, key_len);
		psetstakey_para->gk = 1;
	} else if (key_type == UNICAST_KEY)
		_rtw_memcpy(&psetstakey_para->key, &sta->dot118021x_UncstKey, key_len);
#ifdef CONFIG_TDLS
	else if (key_type == TDLS_KEY) {
		_rtw_memcpy(&psetstakey_para->key, sta->tpk.tk, key_len);
		psetstakey_para->algorithm = (u8)sta->dot118021XPrivacy;
	}
#endif /* CONFIG_TDLS */

	/* jeff: set this becasue at least sw key is ready */
	padapter->securitypriv.busetkipkey = _TRUE;

	if (enqueue) {
		pcmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (pcmd == NULL) {
			rtw_mfree((u8 *) psetstakey_para, sizeof(struct set_stakey_parm));
			res = _FAIL;
			goto exit;
		}
		pcmd->padapter = padapter;

		psetstakey_rsp = (struct set_stakey_rsp *)rtw_zmalloc(sizeof(struct set_stakey_rsp));
		if (psetstakey_rsp == NULL) {
			rtw_mfree((u8 *) pcmd, sizeof(struct cmd_obj));
			rtw_mfree((u8 *) psetstakey_para, sizeof(struct set_stakey_parm));
			res = _FAIL;
			goto exit;
		}

		init_h2fwcmd_w_parm_no_rsp(pcmd, psetstakey_para, CMD_SET_STAKEY);
		pcmd->rsp = (u8 *) psetstakey_rsp;
		pcmd->rspsz = sizeof(struct set_stakey_rsp);
		res = rtw_enqueue_cmd(pcmdpriv, pcmd);
	} else {
		set_stakey_hdl(padapter, (u8 *)psetstakey_para);
		rtw_mfree((u8 *) psetstakey_para, sizeof(struct set_stakey_parm));
	}
exit:
	return res;
}

u8 rtw_clearstakey_cmd(_adapter *padapter, struct sta_info *sta, u8 enqueue)
{
	struct cmd_obj *cmd;
	struct set_stakey_parm	*psetstakey_para;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	struct set_stakey_rsp *psetstakey_rsp = NULL;
	s16 cam_id = 0;
	u8 res = _SUCCESS;

	if (!sta) {
		RTW_ERR("%s sta == NULL\n", __func__);
		goto exit;
	}

	if (!enqueue) {
		rtw_hw_del_all_key(padapter, sta, PHL_CMD_DIRECTLY, 0);
	} else {
		cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (cmd == NULL) {
			res = _FAIL;
			goto exit;
		}

		psetstakey_para = (struct set_stakey_parm *)rtw_zmalloc(sizeof(struct set_stakey_parm));
		if (psetstakey_para == NULL) {
			rtw_mfree((u8 *)cmd, sizeof(struct cmd_obj));
			res = _FAIL;
			goto exit;
		}
		cmd->padapter = padapter;

		psetstakey_rsp = (struct set_stakey_rsp *)rtw_zmalloc(sizeof(struct set_stakey_rsp));
		if (psetstakey_rsp == NULL) {
			rtw_mfree((u8 *)cmd, sizeof(struct cmd_obj));
			rtw_mfree((u8 *)psetstakey_para, sizeof(struct set_stakey_parm));
			res = _FAIL;
			goto exit;
		}

		init_h2fwcmd_w_parm_no_rsp(cmd, psetstakey_para, CMD_SET_STAKEY);
		cmd->rsp = (u8 *) psetstakey_rsp;
		cmd->rspsz = sizeof(struct set_stakey_rsp);

		_rtw_memcpy(psetstakey_para->addr, sta->phl_sta->mac_addr, ETH_ALEN);

		psetstakey_para->algorithm = _NO_PRIVACY_;

		res = rtw_enqueue_cmd(pcmdpriv, cmd);

	}

exit:


	return res;
}
#endif

u8 rtw_addbareq_cmd(_adapter *padapter, u8 tid, u8 *addr)
{
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	struct cmd_obj *cmd;
	struct addBaReq_parm *paddbareq_parm;

	u8	res = _SUCCESS;


	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmd->padapter = padapter;

	paddbareq_parm = (struct addBaReq_parm *)rtw_zmalloc(sizeof(struct addBaReq_parm));
	if (paddbareq_parm == NULL) {
		rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	paddbareq_parm->tid = tid;
	_rtw_memcpy(paddbareq_parm->addr, addr, ETH_ALEN);

	init_h2fwcmd_w_parm_no_rsp(cmd, paddbareq_parm, CMD_ADD_BAREQ);

	/* RTW_INFO("rtw_addbareq_cmd, tid=%d\n", tid); */

	/* rtw_enqueue_cmd(pcmdpriv, ph2c);	 */
	res = rtw_enqueue_cmd(pcmdpriv, cmd);

exit:
	return res;
}

u8 rtw_addbarsp_cmd(_adapter *padapter, u8 *addr, u16 tid,
		    struct ADDBA_request *paddba_req, u8 status,
		    u8 size, u16 start_seq)
{
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	struct cmd_obj *cmd;
	struct addBaRsp_parm *paddBaRsp_parm;
	u8 res = _SUCCESS;


	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmd->padapter = padapter;

	paddBaRsp_parm = (struct addBaRsp_parm *)rtw_zmalloc(sizeof(struct addBaRsp_parm));

	if (paddBaRsp_parm == NULL) {
		rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	_rtw_memcpy(paddBaRsp_parm->addr, addr, ETH_ALEN);
	_rtw_memcpy(&(paddBaRsp_parm->preq), paddba_req, sizeof(struct ADDBA_request));
	paddBaRsp_parm->tid = tid;
	paddBaRsp_parm->status = status;
	paddBaRsp_parm->size = size;
	paddBaRsp_parm->start_seq = start_seq;

	init_h2fwcmd_w_parm_no_rsp(cmd, paddBaRsp_parm, CMD_ADD_BARSP);

	res = rtw_enqueue_cmd(pcmdpriv, cmd);

exit:


	return res;
}

u8 rtw_delba_cmd(struct _ADAPTER *a, u8 *addr, u16 tid)
{
	struct cmd_priv *cmdpriv = &adapter_to_dvobj(a)->cmdpriv;
	struct cmd_obj *cmd = NULL;
	struct addBaReq_parm *parm = NULL;


	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (!cmd)
		return _FAIL;
	cmd->padapter = a;

	parm = (struct addBaReq_parm *)rtw_zmalloc(sizeof(struct addBaReq_parm));
	if (!parm) {
		rtw_mfree(cmd, sizeof(struct cmd_obj));
		return _FAIL;
	}

	parm->tid = tid;
	_rtw_memcpy(parm->addr, addr, ETH_ALEN);
	init_h2fwcmd_w_parm_no_rsp(cmd, parm, CMD_DELBA);

	return rtw_enqueue_cmd(cmdpriv, cmd);
}

/* add for CONFIG_IEEE80211W, none 11w can use it */
u8 rtw_reset_securitypriv_cmd(_adapter *padapter)
{
	struct cmd_obj *cmd;
	struct drvextra_cmd_parm  *pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	u8 res = _SUCCESS;


	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmd->padapter = padapter;

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = RESET_SECURITYPRIV;
	pdrvextra_cmd_parm->type = 0;
	pdrvextra_cmd_parm->size = 0;
	pdrvextra_cmd_parm->pbuf = NULL;

	init_h2fwcmd_w_parm_no_rsp(cmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);


	/* rtw_enqueue_cmd(pcmdpriv, ph2c);	 */
	res = rtw_enqueue_cmd(pcmdpriv, cmd);

exit:
	return res;

}

void free_assoc_resources_hdl(_adapter *padapter, u8 lock_scanned_queue)
{
	rtw_free_assoc_resources(padapter, lock_scanned_queue);
}

u8 rtw_free_assoc_resources_cmd(_adapter *padapter, u8 lock_scanned_queue, int flags)
{
	struct cmd_obj *cmd;
	struct drvextra_cmd_parm  *pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	struct submit_ctx sctx;
	u8	res = _SUCCESS;

	if (flags & RTW_CMDF_DIRECTLY) {
		free_assoc_resources_hdl(padapter, lock_scanned_queue);
	}
	else {
		cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (cmd == NULL) {
			res = _FAIL;
			goto exit;
		}
		cmd->padapter = padapter;

		pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
		if (pdrvextra_cmd_parm == NULL) {
			rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
			res = _FAIL;
			goto exit;
		}

		pdrvextra_cmd_parm->ec_id = FREE_ASSOC_RESOURCES;
		pdrvextra_cmd_parm->type = lock_scanned_queue;
		pdrvextra_cmd_parm->size = 0;
		pdrvextra_cmd_parm->pbuf = NULL;

		init_h2fwcmd_w_parm_no_rsp(cmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);
		if (flags & RTW_CMDF_WAIT_ACK) {
			cmd->sctx = &sctx;
			rtw_sctx_init(&sctx, 2000);
		}

		res = rtw_enqueue_cmd(pcmdpriv, cmd);

		if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				cmd->sctx = NULL;
			_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
		}
	}
exit:
	return res;

}

u8 rtw_dynamic_chk_wk_cmd(_adapter *padapter)
{
	struct cmd_obj *cmd;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	u8	res = _SUCCESS;


	/* only  primary padapter does this cmd */

	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmd->padapter = padapter;

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = DYNAMIC_CHK_WK_CID;
	pdrvextra_cmd_parm->type = 0;
	pdrvextra_cmd_parm->size = 0;
	pdrvextra_cmd_parm->pbuf = NULL;
	init_h2fwcmd_w_parm_no_rsp(cmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);


	/* rtw_enqueue_cmd(pcmdpriv, ph2c);	 */
	res = rtw_enqueue_cmd(pcmdpriv, cmd);

exit:
	return res;
}

u8 rtw_set_chbw_cmd(_adapter *padapter, u8 ch, u8 bw, u8 ch_offset, u8 flags)
{
	struct cmd_obj *pcmdobj;
	struct set_ch_parm *set_ch_parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	struct submit_ctx sctx;
	u8 res = _SUCCESS;


	RTW_INFO(FUNC_NDEV_FMT" ch:%u, bw:%u, ch_offset:%u\n",
		 FUNC_NDEV_ARG(padapter->pnetdev), ch, bw, ch_offset);

	/* check input parameter */

	/* prepare cmd parameter */
	set_ch_parm = (struct set_ch_parm *)rtw_zmalloc(sizeof(*set_ch_parm));
	if (set_ch_parm == NULL) {
		res = _FAIL;
		goto exit;
	}
	set_ch_parm->ch = ch;
	set_ch_parm->bw = bw;
	set_ch_parm->ch_offset = ch_offset;
	set_ch_parm->do_rfk = _FALSE;/*TODO - Need check if do_rfk*/

	if (flags & RTW_CMDF_DIRECTLY) {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (H2C_SUCCESS != rtw_set_chbw_hdl(padapter, (u8 *)set_ch_parm))
			res = _FAIL;

		rtw_mfree((u8 *)set_ch_parm, sizeof(*set_ch_parm));
	} else {
		/* need enqueue, prepare cmd_obj and enqueue */
		pcmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (pcmdobj == NULL) {
			rtw_mfree((u8 *)set_ch_parm, sizeof(*set_ch_parm));
			res = _FAIL;
			goto exit;
		}
		pcmdobj->padapter = padapter;

		init_h2fwcmd_w_parm_no_rsp(pcmdobj, set_ch_parm, CMD_SET_CHANNEL);

		if (flags & RTW_CMDF_WAIT_ACK) {
			pcmdobj->sctx = &sctx;
			rtw_sctx_init(&sctx, 10 * 1000);
		}

		res = rtw_enqueue_cmd(pcmdpriv, pcmdobj);

		if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				pcmdobj->sctx = NULL;
			_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
		}
	}

	/* do something based on res... */
exit:
	RTW_INFO(FUNC_NDEV_FMT" res:%u\n", FUNC_NDEV_ARG(padapter->pnetdev), res);
	return res;
}

u8 _rtw_set_chplan_cmd(_adapter *adapter, int flags, u8 chplan, u8 chplan_6g, const struct country_chplan *country_ent, u8 swconfig)
{
	struct cmd_obj *cmdobj;
	struct	SetChannelPlan_param *parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct submit_ctx sctx;
	u8 res = _SUCCESS;


	/* check if allow software config */
	if (swconfig && rtw_hal_is_disable_sw_channel_plan(adapter) == _TRUE) {
		res = _FAIL;
		goto exit;
	}

	/* if country_entry is provided, replace chplan */
	if (country_ent) {
		chplan = country_ent->chplan;
		#if CONFIG_IEEE80211_BAND_6GHZ
		chplan_6g = country_ent->chplan_6g;
		#endif
	}

	/* check input parameter */
	if (!rtw_is_channel_plan_valid(chplan)) {
		res = _FAIL;
		goto exit;
	}

	/* prepare cmd parameter */
	parm = (struct SetChannelPlan_param *)rtw_zmalloc(sizeof(*parm));
	if (parm == NULL) {
		res = _FAIL;
		goto exit;
	}
	parm->country_ent = country_ent;
	parm->channel_plan = chplan;
#if CONFIG_IEEE80211_BAND_6GHZ
	parm->channel_plan_6g = chplan_6g;
#endif

	if (flags & RTW_CMDF_DIRECTLY) {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (H2C_SUCCESS != set_chplan_hdl(adapter, (u8 *)parm))
			res = _FAIL;
		rtw_mfree((u8 *)parm, sizeof(*parm));
	} else {
		/* need enqueue, prepare cmd_obj and enqueue */
		cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(*cmdobj));
		if (cmdobj == NULL) {
			res = _FAIL;
			rtw_mfree((u8 *)parm, sizeof(*parm));
			goto exit;
		}
		cmdobj->padapter = adapter;

		init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_SET_CHANPLAN);

		if (flags & RTW_CMDF_WAIT_ACK) {
			cmdobj->sctx = &sctx;
			rtw_sctx_init(&sctx, 2000);
		}

		res = rtw_enqueue_cmd(pcmdpriv, cmdobj);

		if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				cmdobj->sctx = NULL;
			_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
			if (sctx.status != RTW_SCTX_DONE_SUCCESS)
				res = _FAIL;
		}

		/* allow set channel plan when cmd_thread is not running */
		if (res != _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			parm = (struct SetChannelPlan_param *)rtw_zmalloc(sizeof(*parm));
			if (parm == NULL) {
				res = _FAIL;
				goto exit;
			}
			parm->country_ent = country_ent;
			parm->channel_plan = chplan;
			#if CONFIG_IEEE80211_BAND_6GHZ
			parm->channel_plan_6g = chplan_6g;
			#endif

			if (H2C_SUCCESS != set_chplan_hdl(adapter, (u8 *)parm))
				res = _FAIL;
			else
				res = _SUCCESS;
			rtw_mfree((u8 *)parm, sizeof(*parm));
		}
	}

exit:
	return res;
}

inline u8 rtw_set_chplan_cmd(_adapter *adapter, int flags, u8 chplan, u8 chplan_6g, u8 swconfig)
{
	return _rtw_set_chplan_cmd(adapter, flags, chplan, chplan_6g, NULL, swconfig);
}

inline u8 rtw_set_country_cmd(_adapter *adapter, int flags, const char *country_code, u8 swconfig)
{
	struct country_chplan ent;

	if (is_alpha(country_code[0]) == _FALSE
	    || is_alpha(country_code[1]) == _FALSE
	   ) {
		RTW_PRINT("%s input country_code is not alpha2\n", __func__);
		return _FAIL;
	}

	if (rtw_get_chplan_from_country(country_code, &ent) == _FALSE) {
		RTW_PRINT("%s unsupported country_code:\"%c%c\"\n", __func__, country_code[0], country_code[1]);
		return _FAIL;
	}

	RTW_PRINT("%s country_code:\"%c%c\" mapping to chplan:0x%02x\n", __func__, country_code[0], country_code[1], ent.chplan);

	return _rtw_set_chplan_cmd(adapter, flags, RTW_CHPLAN_UNSPECIFIED, RTW_CHPLAN_6G_UNSPECIFIED, &ent, swconfig);
}

#ifdef CONFIG_RTW_LED_HANDLED_BY_CMD_THREAD
u8 rtw_led_blink_cmd(_adapter *padapter, void *pLed)
{
	struct cmd_obj	*pcmdobj;
	struct LedBlink_param *ledBlink_param;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	u8 res = _SUCCESS;

	pcmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmdobj == NULL) {
		res = _FAIL;
		goto exit;
	}
	pcmdobj->padapter = padapter;

	ledBlink_param = (struct LedBlink_param *)rtw_zmalloc(sizeof(struct LedBlink_param));
	if (ledBlink_param == NULL) {
		rtw_mfree((u8 *)pcmdobj, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	ledBlink_param->pLed = pLed;

	init_h2fwcmd_w_parm_no_rsp(pcmdobj, ledBlink_param, CMD_LEDBLINK);
	res = rtw_enqueue_cmd(pcmdpriv, pcmdobj);

exit:
	return res;
}
#endif /*CONFIG_RTW_LED_HANDLED_BY_CMD_THREAD*/

u8 rtw_set_csa_cmd(_adapter *adapter)
{
	struct cmd_obj *cmdobj;
	struct cmd_priv *cmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	u8 res = _SUCCESS;

	cmdobj = rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmdobj == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmdobj->padapter = adapter;

	init_h2fwcmd_w_parm_no_parm_rsp(cmdobj, CMD_SET_CHANSWITCH);
	res = rtw_enqueue_cmd(cmdpriv, cmdobj);

exit:
	return res;
}

u8 rtw_tdls_cmd(_adapter *padapter, u8 *addr, u8 option)
{
	u8 res = _SUCCESS;
#ifdef CONFIG_TDLS
	struct	cmd_obj	*pcmdobj;
	struct	TDLSoption_param *TDLSoption;
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct	cmd_priv *pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;

	pcmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmdobj == NULL) {
		res = _FAIL;
		goto exit;
	}
	pcmdobj->padapter = padapter;

	TDLSoption = (struct TDLSoption_param *)rtw_zmalloc(sizeof(struct TDLSoption_param));
	if (TDLSoption == NULL) {
		rtw_mfree((u8 *)pcmdobj, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	_rtw_spinlock(&(padapter->tdlsinfo.cmd_lock));
	if (addr != NULL)
		_rtw_memcpy(TDLSoption->addr, addr, 6);
	TDLSoption->option = option;
	_rtw_spinunlock(&(padapter->tdlsinfo.cmd_lock));
	init_h2fwcmd_w_parm_no_rsp(pcmdobj, TDLSoption, CMD_TDLS);
	res = rtw_enqueue_cmd(pcmdpriv, pcmdobj);

exit:
#endif /* CONFIG_TDLS */

	return res;
}

u8 rtw_set_run_cmd_en(_adapter *padapter, u8 value, u8 flags)
{
	struct cmd_obj *pcmdobj;
	struct SetRunCmdEn_param *param;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	struct submit_ctx sctx;
	u8 res = _SUCCESS;

	pcmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmdobj == NULL) {
		res = _FAIL;
		goto exit;
	}
	pcmdobj->padapter = padapter;

	param = (struct SetRunCmdEn_param *)rtw_zmalloc(sizeof(struct SetRunCmdEn_param));
	if (param == NULL) {
		rtw_mfree((u8 *)pcmdobj, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	param->value = value;

	init_h2fwcmd_w_parm_no_rsp(pcmdobj, param, CMD_SET_RUN_CMD_EN);

	if (flags & RTW_CMDF_WAIT_ACK) {
		pcmdobj->sctx = &sctx;
		rtw_sctx_init(&sctx, 2000);
	}

	res = rtw_enqueue_cmd(pcmdpriv, pcmdobj);

	if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
		rtw_sctx_wait(&sctx, __func__);
		_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
		if (sctx.status == RTW_SCTX_SUBMITTED)
			pcmdobj->sctx = NULL;
		_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
	}

exit:

	return res;
}

u8 rtw_ssmps_wk_hdl(_adapter *adapter, struct ssmps_cmd_parm *ssmp_param)
{
	u8 res = _SUCCESS;
	struct sta_info *sta = ssmp_param->sta;
	u8 smps = ssmp_param->smps;

	if (sta == NULL)
		return _FALSE;

	if (smps == SM_PS_STATIC)
		rtw_ssmps_enter_static(adapter, sta);
	else if (smps == SM_PS_DYNAMIC)
		rtw_ssmps_enter_dynamic(adapter, sta);
	else
		rtw_ssmps_leave(adapter, sta);
	return res;
}

u8 rtw_ssmps_wk_cmd(_adapter *adapter, struct sta_info *sta, u8 smps, u8 enqueue)
{
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *cmd_parm;
	struct ssmps_cmd_parm *ssmp_param;
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	u8 res = _SUCCESS;

	if (enqueue) {
		cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (cmdobj == NULL) {
			res = _FAIL;
			goto exit;
		}
		cmdobj->padapter = adapter;

		cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
		if (cmd_parm == NULL) {
			rtw_mfree((unsigned char *)cmdobj, sizeof(struct cmd_obj));
			res = _FAIL;
			goto exit;
		}

		ssmp_param = (struct ssmps_cmd_parm *)rtw_zmalloc(sizeof(struct ssmps_cmd_parm));
		if (ssmp_param == NULL) {
			rtw_mfree((u8 *)cmdobj, sizeof(struct cmd_obj));
			rtw_mfree((u8 *)cmd_parm, sizeof(struct drvextra_cmd_parm));
			res = _FAIL;
			goto exit;
		}

		ssmp_param->smps = smps;
		ssmp_param->sta = sta;

		cmd_parm->ec_id = SSMPS_WK_CID;
		cmd_parm->type = 0;
		cmd_parm->size = sizeof(struct ssmps_cmd_parm);
		cmd_parm->pbuf = (u8 *)ssmp_param;

		init_h2fwcmd_w_parm_no_rsp(cmdobj, cmd_parm, CMD_SET_DRV_EXTRA);

		res = rtw_enqueue_cmd(pcmdpriv, cmdobj);
	} else {
		struct ssmps_cmd_parm tmp_ssmp_param;

		tmp_ssmp_param.smps = smps;
		tmp_ssmp_param.sta = sta;
		rtw_ssmps_wk_hdl(adapter, &tmp_ssmp_param);
	}

exit:
	return res;
}

#ifdef CONFIG_SUPPORT_STATIC_SMPS
u8 _ssmps_chk_by_tp(_adapter *adapter, u8 from_timer)
{
	u8 enter_smps = _FALSE;
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct sta_info *psta;
	u32 tx_tp_mbits, rx_tp_mbits;

	if (!MLME_IS_STA(adapter) ||
		!rtw_hw_is_mimo_support(adapter_to_dvobj(adapter)) ||
		!pmlmeext->ssmps_en ||
		(pmlmeext->cur_channel > 14)
	)
		return enter_smps;

	psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
	if (psta == NULL) {
		RTW_ERR(ADPT_FMT" sta == NULL\n", ADPT_ARG(adapter));
		rtw_warn_on(1);
		return enter_smps;
	}

	if (psta->phl_sta->mimo_type == RF_1T1R)
		return enter_smps;

	tx_tp_mbits = psta->sta_stats.tx_tp_kbits >> 10;
	rx_tp_mbits = psta->sta_stats.rx_tp_kbits >> 10;

	#ifdef DBG_STATIC_SMPS
	if (pmlmeext->ssmps_test) {
		enter_smps = (pmlmeext->ssmps_test_en == 1) ? _TRUE : _FALSE;
	}
	else
	#endif
	{
		if ((tx_tp_mbits <= pmlmeext->ssmps_tx_tp_th) &&
			(rx_tp_mbits <= pmlmeext->ssmps_rx_tp_th))
			enter_smps = _TRUE;
		else
			enter_smps = _FALSE;
	}

	if (1) {
		RTW_INFO(FUNC_ADPT_FMT" tx_tp:%d [%d], rx_tp:%d [%d] , SSMPS enter :%s\n",
			FUNC_ADPT_ARG(adapter),
			tx_tp_mbits, pmlmeext->ssmps_tx_tp_th,
			rx_tp_mbits, pmlmeext->ssmps_rx_tp_th,
			(enter_smps == _TRUE) ? "True" : "False");
		#ifdef DBG_STATIC_SMPS
		RTW_INFO(FUNC_ADPT_FMT" test:%d test_en:%d\n",
			FUNC_ADPT_ARG(adapter),
			pmlmeext->ssmps_test,
			pmlmeext->ssmps_test_en);
		#endif
	}

	if (enter_smps) {
		if (!from_timer && psta->phl_sta->sm_ps != SM_PS_STATIC)
			rtw_ssmps_enter_static(adapter, psta);
	} else {
		if (!from_timer && psta->phl_sta->sm_ps != SM_PS_DISABLE)
			rtw_ssmps_leave(adapter, psta);
		else {
			u8 ps_change = _FALSE;

			if (enter_smps && psta->phl_sta->sm_ps != SM_PS_STATIC)
				ps_change = _TRUE;
			else if (!enter_smps && psta->phl_sta->sm_ps != SM_PS_DISABLE)
				ps_change = _TRUE;

			if (ps_change)
				rtw_ssmps_wk_cmd(adapter, psta, psta->phl_sta->sm_ps, 1);
		}
	}

	return enter_smps;
}
#endif /*CONFIG_SUPPORT_STATIC_SMPS*/

#ifdef CONFIG_CTRL_TXSS_BY_TP
void rtw_ctrl_txss_update_mimo_type(_adapter *adapter, struct sta_info *sta)
{
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);

	pmlmeext->txss_momi_type_bk = sta->phl_sta->mimo_type;
}

u8 rtw_ctrl_txss(_adapter *adapter, struct sta_info *sta, bool tx_1ss)
{
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(adapter);
	u8 lps_changed = _FALSE;
	u8 rst = _SUCCESS;

	if (pmlmeext->txss_1ss == tx_1ss)
		return _FALSE;

	if (pwrpriv->bLeisurePs && pwrpriv->pwr_mode != PM_PS_MODE_ACTIVE) {
		lps_changed = _TRUE;
		LPS_Leave(adapter, "LPS_CTRL_TXSS");
	}

	RTW_INFO(ADPT_FMT" STA [" MAC_FMT "] set tx to %d ss\n",
		ADPT_ARG(adapter), MAC_ARG(sta->phl_sta->mac_addr),
		(tx_1ss) ? 1 : rtw_get_sta_tx_nss(adapter, sta));

	/*ra re-registed*/
	sta->phl_sta->mimo_type = (tx_1ss) ? RF_1T1R : pmlmeext->txss_momi_type_bk;
	rtw_phydm_ra_registed(adapter, sta);

	/*configure trx mode*/
	rtw_phydm_trx_cfg(adapter, tx_1ss);
	pmlmeext->txss_1ss = tx_1ss;

	if (lps_changed)
		LPS_Enter(adapter, "LPS_CTRL_TXSS");

	return rst;
}

u8 rtw_ctrl_txss_wk_hdl(_adapter *adapter, struct txss_cmd_parm *txss_param)
{
	if (!txss_param->sta)
		return _FALSE;

	return rtw_ctrl_txss(adapter, txss_param->sta, txss_param->tx_1ss);
}

u8 rtw_ctrl_txss_wk_cmd(_adapter *adapter, struct sta_info *sta, bool tx_1ss, u8 flag)
{
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *cmd_parm;
	struct txss_cmd_parm *txss_param;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct submit_ctx sctx;
	u8	res = _SUCCESS;

	txss_param = (struct txss_cmd_parm *)rtw_zmalloc(sizeof(struct txss_cmd_parm));
	if (txss_param == NULL) {
		res = _FAIL;
		goto exit;
	}

	txss_param->tx_1ss = tx_1ss;
	txss_param->sta = sta;

	if (flag & RTW_CMDF_DIRECTLY) {
		res = rtw_ctrl_txss_wk_hdl(adapter, txss_param);
		rtw_mfree((u8 *)txss_param, sizeof(*txss_param));
	} else {
		cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (cmdobj == NULL) {
			res = _FAIL;
			goto exit;
		}
		cmdobj->padapter = adapter;

		cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
		if (cmd_parm == NULL) {
			rtw_mfree((u8 *)cmdobj, sizeof(struct cmd_obj));
			res = _FAIL;
			goto exit;
		}

		cmd_parm->ec_id = TXSS_WK_CID;
		cmd_parm->type = 0;
		cmd_parm->size = sizeof(struct txss_cmd_parm);
		cmd_parm->pbuf = (u8 *)txss_param;

		init_h2fwcmd_w_parm_no_rsp(cmdobj, cmd_parm, CMD_SET_DRV_EXTRA);

		if (flag & RTW_CMDF_WAIT_ACK) {
			cmdobj->sctx = &sctx;
			rtw_sctx_init(&sctx, 10 * 1000);
		}

		res = rtw_enqueue_cmd(pcmdpriv, cmdobj);
		if (res == _SUCCESS && (flag & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				cmdobj->sctx = NULL;
			_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
			if (sctx.status != RTW_SCTX_DONE_SUCCESS)
				res = _FAIL;
		}
	}

exit:
	return res;
}

void rtw_ctrl_tx_ss_by_tp(_adapter *adapter, u8 from_timer)
{
	bool tx_1ss  = _FALSE; /*change tx from 2ss to 1ss*/
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct sta_info *psta;
	u32 tx_tp_mbits;

	if (!MLME_IS_STA(adapter) ||
		!rtw_hw_is_mimo_support(adapter_to_dvobj(adapter)) ||
		!pmlmeext->txss_ctrl_en
	)
		return;

	psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
	if (psta == NULL) {
		RTW_ERR(ADPT_FMT" sta == NULL\n", ADPT_ARG(adapter));
		rtw_warn_on(1);
		return;
	}

	tx_tp_mbits = psta->sta_stats.tx_tp_kbits >> 10;
	if (tx_tp_mbits >= pmlmeext->txss_tp_th) {
		tx_1ss = _FALSE;
	} else {
		if (pmlmeext->txss_tp_chk_cnt && --pmlmeext->txss_tp_chk_cnt)
			tx_1ss = _FALSE;
		else
			tx_1ss = _TRUE;
	}

	if (1) {
		RTW_INFO(FUNC_ADPT_FMT" tx_tp:%d [%d] tx_1ss(%d):%s\n",
			FUNC_ADPT_ARG(adapter),
			tx_tp_mbits, pmlmeext->txss_tp_th,
			pmlmeext->txss_tp_chk_cnt,
			(tx_1ss == _TRUE) ? "True" : "False");
	}

	if (pmlmeext->txss_1ss != tx_1ss) {
		if (from_timer)
			rtw_ctrl_txss_wk_cmd(adapter, psta, tx_1ss, 0);
		else
			rtw_ctrl_txss(adapter, psta, tx_1ss);
	}
}
#ifdef DBG_CTRL_TXSS
void dbg_ctrl_txss(_adapter *adapter, bool tx_1ss)
{
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct sta_info *psta;

	if (!MLME_IS_STA(adapter) ||
		!rtw_hw_is_mimo_support(adapter_to_dvobj(adapter))
	)
		return;

	psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
	if (psta == NULL) {
		RTW_ERR(ADPT_FMT" sta == NULL\n", ADPT_ARG(adapter));
		rtw_warn_on(1);
		return;
	}

	rtw_ctrl_txss(adapter, psta, tx_1ss);
}
#endif
#endif /*CONFIG_CTRL_TXSS_BY_TP*/

#ifdef CONFIG_LPS
#ifdef CONFIG_LPS_CHK_BY_TP
#ifdef LPS_BCN_CNT_MONITOR
static u8 _bcn_cnt_expected(struct sta_info *psta)
{
	_adapter *adapter = psta->padapter;
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 dtim = rtw_get_bcn_dtim_period(adapter);
	u8 bcn_cnt = 0;

	if ((pmlmeinfo->bcn_interval !=0) && (dtim != 0))
		bcn_cnt = 2000 / pmlmeinfo->bcn_interval / dtim * 4 / 5; /*2s*/
	if (0)
		RTW_INFO("%s bcn_cnt:%d\n", __func__, bcn_cnt);

	if (bcn_cnt == 0) {
		RTW_ERR(FUNC_ADPT_FMT" bcn_cnt == 0\n", FUNC_ADPT_ARG(adapter));
		rtw_warn_on(1);
	}

	return bcn_cnt;
}
#endif
u8 _lps_chk_by_tp(_adapter *adapter, u8 from_timer)
{
	u8 enter_ps = _FALSE;
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct sta_info *psta;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(adapter);
	u32 tx_tp_mbits, rx_tp_mbits, bi_tp_mbits;
	u8 rx_bcn_cnt;

	psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
	if (psta == NULL) {
		RTW_ERR(ADPT_FMT" sta == NULL\n", ADPT_ARG(adapter));
		rtw_warn_on(1);
		return enter_ps;
	}

	rx_bcn_cnt = rtw_get_bcn_cnt(psta->padapter);
	psta->sta_stats.acc_tx_bytes = psta->sta_stats.tx_bytes;
	psta->sta_stats.acc_rx_bytes = psta->sta_stats.rx_bytes;

#if 1
	tx_tp_mbits = psta->sta_stats.tx_tp_kbits >> 10;
	rx_tp_mbits = psta->sta_stats.rx_tp_kbits >> 10;
	bi_tp_mbits = tx_tp_mbits + rx_tp_mbits;
#else
	tx_tp_mbits = psta->sta_stats.smooth_tx_tp_kbits >> 10;
	rx_tp_mbits = psta->sta_stats.smooth_rx_tp_kbits >> 10;
	bi_tp_mbits = tx_tp_mbits + rx_tp_mbits;
#endif

	if ((bi_tp_mbits >= pwrpriv->lps_bi_tp_th) ||
		(tx_tp_mbits >= pwrpriv->lps_tx_tp_th) ||
		(rx_tp_mbits >= pwrpriv->lps_rx_tp_th)) {
		enter_ps = _FALSE;
		pwrpriv->lps_chk_cnt = pwrpriv->lps_chk_cnt_th;
	}
	else {
#ifdef LPS_BCN_CNT_MONITOR
		u8 bcn_cnt = _bcn_cnt_expected(psta);

		if (bcn_cnt && (rx_bcn_cnt < bcn_cnt)) {
			pwrpriv->lps_chk_cnt = 2;
			RTW_ERR(FUNC_ADPT_FMT" BCN_CNT:%d(%d) invalid\n",
				FUNC_ADPT_ARG(adapter), rx_bcn_cnt, bcn_cnt);
		}
#endif

		if (pwrpriv->lps_chk_cnt && --pwrpriv->lps_chk_cnt)
			enter_ps = _FALSE;
		else
			enter_ps = _TRUE;
	}

	if (1) {
		RTW_INFO(FUNC_ADPT_FMT" tx_tp:%d [%d], rx_tp:%d [%d], bi_tp:%d [%d], enter_ps(%d):%s\n",
			FUNC_ADPT_ARG(adapter),
			tx_tp_mbits, pwrpriv->lps_tx_tp_th,
			rx_tp_mbits, pwrpriv->lps_rx_tp_th,
			bi_tp_mbits, pwrpriv->lps_bi_tp_th,
			pwrpriv->lps_chk_cnt,
			(enter_ps == _TRUE) ? "True" : "False");
		RTW_INFO(FUNC_ADPT_FMT" tx_pkt_cnt :%d [%d], rx_pkt_cnt :%d [%d]\n",
			FUNC_ADPT_ARG(adapter),
			pmlmepriv->LinkDetectInfo.NumTxOkInPeriod,
			pwrpriv->lps_tx_pkts,
			pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod,
			pwrpriv->lps_rx_pkts);
		if (!adapter->bsta_tp_dump)
			RTW_INFO(FUNC_ADPT_FMT" bcn_cnt:%d (per-%d second)\n",
			FUNC_ADPT_ARG(adapter),
			rx_bcn_cnt,
			2);
	}

	if (enter_ps) {
		if (!from_timer)
			LPS_Enter(adapter, "TRAFFIC_IDLE");
	} else {
		if (!from_timer)
			LPS_Leave(adapter, "TRAFFIC_BUSY");
		else {
			#ifdef CONFIG_CONCURRENT_MODE
			#ifndef CONFIG_FW_MULTI_PORT_SUPPORT
			if (adapter->hw_port == HW_PORT0)
			#endif
			#endif
				rtw_lps_ctrl_wk_cmd(adapter, LPS_CTRL_TRAFFIC_BUSY, 0);
		}
	}

	return enter_ps;
}
#endif

static u8 _lps_chk_by_pkt_cnts(_adapter *padapter, u8 from_timer, u8 bBusyTraffic)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u8	bEnterPS = _FALSE;

	/* check traffic for  powersaving. */
	if (((pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod + pmlmepriv->LinkDetectInfo.NumTxOkInPeriod) > 8) ||
		#ifdef CONFIG_LPS_SLOW_TRANSITION
		(pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod > 2)
		#else /* CONFIG_LPS_SLOW_TRANSITION */
		(pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod > 4)
		#endif /* CONFIG_LPS_SLOW_TRANSITION */
	) {
		#ifdef DBG_RX_COUNTER_DUMP
		if (padapter->dump_rx_cnt_mode & DUMP_DRV_TRX_COUNTER_DATA)
			RTW_INFO("(-)Tx = %d, Rx = %d\n", pmlmepriv->LinkDetectInfo.NumTxOkInPeriod, pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod);
		#endif

		bEnterPS = _FALSE;
		#ifdef CONFIG_LPS_SLOW_TRANSITION
		if (bBusyTraffic == _TRUE) {
			if (pmlmepriv->LinkDetectInfo.TrafficTransitionCount <= 4)
				pmlmepriv->LinkDetectInfo.TrafficTransitionCount = 4;

			pmlmepriv->LinkDetectInfo.TrafficTransitionCount++;

			/* RTW_INFO("Set TrafficTransitionCount to %d\n", pmlmepriv->LinkDetectInfo.TrafficTransitionCount); */

			if (pmlmepriv->LinkDetectInfo.TrafficTransitionCount > 30/*TrafficTransitionLevel*/)
				pmlmepriv->LinkDetectInfo.TrafficTransitionCount = 30;
		}
		#endif /* CONFIG_LPS_SLOW_TRANSITION */
	} else {
		#ifdef DBG_RX_COUNTER_DUMP
		if (padapter->dump_rx_cnt_mode & DUMP_DRV_TRX_COUNTER_DATA)
			RTW_INFO("(+)Tx = %d, Rx = %d\n", pmlmepriv->LinkDetectInfo.NumTxOkInPeriod, pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod);
		#endif

		#ifdef CONFIG_LPS_SLOW_TRANSITION
		if (pmlmepriv->LinkDetectInfo.TrafficTransitionCount >= 2)
			pmlmepriv->LinkDetectInfo.TrafficTransitionCount -= 2;
		else
			pmlmepriv->LinkDetectInfo.TrafficTransitionCount = 0;

		if (pmlmepriv->LinkDetectInfo.TrafficTransitionCount == 0)
			bEnterPS = _TRUE;
		#else /* CONFIG_LPS_SLOW_TRANSITION */
			bEnterPS = _TRUE;
		#endif /* CONFIG_LPS_SLOW_TRANSITION */
	}

	#ifdef CONFIG_DYNAMIC_DTIM
	if (pmlmepriv->LinkDetectInfo.LowPowerTransitionCount == 8)
		bEnterPS = _FALSE;

	RTW_INFO("LowPowerTransitionCount=%d\n", pmlmepriv->LinkDetectInfo.LowPowerTransitionCount);
	#endif /* CONFIG_DYNAMIC_DTIM */

	/* LeisurePS only work in infra mode. */
	if (bEnterPS) {
		if (!from_timer) {
			#ifdef CONFIG_DYNAMIC_DTIM
			if (pmlmepriv->LinkDetectInfo.LowPowerTransitionCount < 8)
				adapter_to_pwrctl(padapter)->dtim = 1;
			else
				adapter_to_pwrctl(padapter)->dtim = 3;
			#endif /* CONFIG_DYNAMIC_DTIM */
			LPS_Enter(padapter, "TRAFFIC_IDLE");
		} else {
			/* do this at caller */
			/* rtw_lps_ctrl_wk_cmd(adapter, LPS_CTRL_ENTER, 0); */
			/* rtw_hal_dm_watchdog_in_lps(padapter); */
		}

		#ifdef CONFIG_DYNAMIC_DTIM
		if (adapter_to_pwrctl(padapter)->bFwCurrentInPSMode == _TRUE)
			pmlmepriv->LinkDetectInfo.LowPowerTransitionCount++;
		#endif /* CONFIG_DYNAMIC_DTIM */
	} else {
		#ifdef CONFIG_DYNAMIC_DTIM
		if (pmlmepriv->LinkDetectInfo.LowPowerTransitionCount != 8)
			pmlmepriv->LinkDetectInfo.LowPowerTransitionCount = 0;
		else
			pmlmepriv->LinkDetectInfo.LowPowerTransitionCount++;
		#endif /* CONFIG_DYNAMIC_DTIM */

		if (!from_timer)
			LPS_Leave(padapter, "TRAFFIC_BUSY");
		else {
			#ifdef CONFIG_CONCURRENT_MODE
			#ifndef CONFIG_FW_MULTI_PORT_SUPPORT
			if (padapter->hw_port == HW_PORT0)
			#endif
			#endif
				rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_TRAFFIC_BUSY, 0);
		}
	}

	return bEnterPS;
}
#endif /* CONFIG_LPS */

/* from_timer == 1 means driver is in LPS */
u8 traffic_status_watchdog(_adapter *padapter, u8 from_timer)
{
	u8	bEnterPS = _FALSE;
	u16 BusyThresholdHigh;
	u16	BusyThresholdLow;
	u16	BusyThreshold;
	u8	bBusyTraffic = _FALSE, bTxBusyTraffic = _FALSE, bRxBusyTraffic = _FALSE;
	u8	bHigherBusyTraffic = _FALSE, bHigherBusyRxTraffic = _FALSE, bHigherBusyTxTraffic = _FALSE;

	struct mlme_priv		*pmlmepriv = &(padapter->mlmepriv);
#ifdef CONFIG_TDLS
	struct tdls_info *ptdlsinfo = &(padapter->tdlsinfo);
	struct tdls_txmgmt txmgmt;
	u8 baddr[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#endif /* CONFIG_TDLS */
#ifdef CONFIG_TRAFFIC_PROTECT
	RT_LINK_DETECT_T *link_detect = &pmlmepriv->LinkDetectInfo;
#endif

#ifdef CONFIG_BTC
	if (padapter->registrypriv.wifi_spec != 1) {
		BusyThresholdHigh = 25;
		BusyThresholdLow = 10;
	} else
#endif /* CONFIG_BTC */
	{
		BusyThresholdHigh = 100;
		BusyThresholdLow = 75;
	}
	BusyThreshold = BusyThresholdHigh;


	/*  */
	/* Determine if our traffic is busy now */
	/*  */
	if ((check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE)
	    /*&& !MgntInitAdapterInProgress(pMgntInfo)*/) {
		/* if we raise bBusyTraffic in last watchdog, using lower threshold. */
		if (pmlmepriv->LinkDetectInfo.bBusyTraffic)
			BusyThreshold = BusyThresholdLow;

		if (pmlmepriv->LinkDetectInfo.NumRxOkInPeriod > BusyThreshold ||
		    pmlmepriv->LinkDetectInfo.NumTxOkInPeriod > BusyThreshold) {
			bBusyTraffic = _TRUE;

			if (pmlmepriv->LinkDetectInfo.NumRxOkInPeriod > pmlmepriv->LinkDetectInfo.NumTxOkInPeriod)
				bRxBusyTraffic = _TRUE;
			else
				bTxBusyTraffic = _TRUE;
		}

		/* Higher Tx/Rx data. */
		if (pmlmepriv->LinkDetectInfo.NumRxOkInPeriod > 4000 ||
		    pmlmepriv->LinkDetectInfo.NumTxOkInPeriod > 4000) {
			bHigherBusyTraffic = _TRUE;

			if (pmlmepriv->LinkDetectInfo.NumRxOkInPeriod > pmlmepriv->LinkDetectInfo.NumTxOkInPeriod)
				bHigherBusyRxTraffic = _TRUE;
			else
				bHigherBusyTxTraffic = _TRUE;
		}

#ifdef CONFIG_TRAFFIC_PROTECT
#define TX_ACTIVE_TH 10
#define RX_ACTIVE_TH 20
#define TRAFFIC_PROTECT_PERIOD_MS 4500

		if (link_detect->NumTxOkInPeriod > TX_ACTIVE_TH
		    || link_detect->NumRxUnicastOkInPeriod > RX_ACTIVE_TH) {

			RTW_INFO(FUNC_ADPT_FMT" acqiure wake_lock for %u ms(tx:%d,rx_unicast:%d)\n",
				 FUNC_ADPT_ARG(padapter),
				 TRAFFIC_PROTECT_PERIOD_MS,
				 link_detect->NumTxOkInPeriod,
				 link_detect->NumRxUnicastOkInPeriod);

			rtw_lock_traffic_suspend_timeout(TRAFFIC_PROTECT_PERIOD_MS);
		}
#endif

#ifdef CONFIG_TDLS
#ifdef CONFIG_TDLS_AUTOSETUP
		/* TDLS_WATCHDOG_PERIOD * 2sec, periodically send */
		if (rtw_hw_chk_wl_func(adapter_to_dvobj(padapter), WL_FUNC_TDLS) == _TRUE) {
			if ((ptdlsinfo->watchdog_count % TDLS_WATCHDOG_PERIOD) == 0) {
				_rtw_memcpy(txmgmt.peer, baddr, ETH_ALEN);
				issue_tdls_dis_req(padapter, &txmgmt);
			}
			ptdlsinfo->watchdog_count++;
		}
#endif /* CONFIG_TDLS_AUTOSETUP */
#endif /* CONFIG_TDLS */

#ifdef CONFIG_SUPPORT_STATIC_SMPS
		_ssmps_chk_by_tp(padapter, from_timer);
#endif
#ifdef CONFIG_CTRL_TXSS_BY_TP
		rtw_ctrl_tx_ss_by_tp(padapter, from_timer);
#endif

#ifdef CONFIG_LPS
		if (adapter_to_pwrctl(padapter)->bLeisurePs && MLME_IS_STA(padapter)) {
			#ifdef CONFIG_LPS_CHK_BY_TP
			if (adapter_to_pwrctl(padapter)->lps_chk_by_tp)
				bEnterPS = _lps_chk_by_tp(padapter, from_timer);
			else
			#endif /*CONFIG_LPS_CHK_BY_TP*/
				bEnterPS = _lps_chk_by_pkt_cnts(padapter, from_timer, bBusyTraffic);
		}
#endif /* CONFIG_LPS */

	} else {
#ifdef CONFIG_LPS
		if (!from_timer && rtw_mi_get_assoc_if_num(padapter) == 0)
			LPS_Leave(padapter, "NON_LINKED");
#endif
	}

	session_tracker_chk_cmd(padapter, NULL);

#ifdef CONFIG_BEAMFORMING
	rtw_bf_update_traffic(padapter);
#endif /* CONFIG_BEAMFORMING */

	pmlmepriv->LinkDetectInfo.NumRxOkInPeriod = 0;
	pmlmepriv->LinkDetectInfo.NumTxOkInPeriod = 0;
	pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod = 0;
	pmlmepriv->LinkDetectInfo.bBusyTraffic = bBusyTraffic;
	pmlmepriv->LinkDetectInfo.bTxBusyTraffic = bTxBusyTraffic;
	pmlmepriv->LinkDetectInfo.bRxBusyTraffic = bRxBusyTraffic;
	pmlmepriv->LinkDetectInfo.bHigherBusyTraffic = bHigherBusyTraffic;
	pmlmepriv->LinkDetectInfo.bHigherBusyRxTraffic = bHigherBusyRxTraffic;
	pmlmepriv->LinkDetectInfo.bHigherBusyTxTraffic = bHigherBusyTxTraffic;

	return bEnterPS;

}


/* for 11n Logo 4.2.31/4.2.32 */
static void dynamic_update_bcn_check(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

#if 0
	if (!padapter->registrypriv.wifi_spec)
		return;
#endif
	if (!padapter->registrypriv.ht_enable || !is_supported_ht(padapter->registrypriv.wireless_mode))
		return;

	if (!MLME_IS_AP(padapter))
		return;

	if (pmlmeext->bstart_bss) {
		/* In 60 * 2 = 120s, there are no legacy AP, update HT info  */

		if (pmlmepriv->olbc_count && pmlmepriv->olbc_count % 60 == 0) {
#ifdef CONFIG_80211N_HT
			if (_FALSE == ATOMIC_READ(&pmlmepriv->olbc_sta)
				&& _FALSE == ATOMIC_READ(&pmlmepriv->olbc_ap)) {

				if (rtw_ht_operation_update(padapter) > 0) {
					update_beacon(padapter, _HT_CAPABILITY_IE_, NULL, _FALSE, 0);
					update_beacon(padapter, _HT_ADD_INFO_IE_, NULL, _TRUE, 0);
				}
			}
#endif /* CONFIG_80211N_HT */
			pmlmepriv->olbc_count = 0;
		}

#ifdef CONFIG_80211N_HT
		/* In 2s, there are any legacy AP or legacy STA, update HT info, and then reset count  */

		if (_FALSE != ATOMIC_READ(&pmlmepriv->olbc_sta)
			|| _FALSE != ATOMIC_READ(&pmlmepriv->olbc_ap)) {

			if (rtw_ht_operation_update(padapter) > 0) {
				update_beacon(padapter, _HT_CAPABILITY_IE_, NULL, _FALSE, 0);
				update_beacon(padapter, _HT_ADD_INFO_IE_, NULL, _TRUE, 0);

			}
			ATOMIC_SET(&pmlmepriv->olbc_sta, _FALSE);
			ATOMIC_SET(&pmlmepriv->olbc_ap, _FALSE);
			pmlmepriv->olbc_count = 1;
		}
#endif /* CONFIG_80211N_HT */
		if(pmlmepriv->olbc_count)
			pmlmepriv->olbc_count ++;
	}
}

static void collect_sta_traffic_statistics_for_sta_mode(_adapter *adapter)
{
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct	mlme_priv	*pmlmepriv = &adapter->mlmepriv;
	u64 curr_tx_bytes = 0, curr_rx_bytes = 0;
	struct sta_info *sta = NULL;
	u8 *mybssid  = get_bssid(pmlmepriv);

	adapter->up_time += 2;

	if (MLME_IS_STA(adapter)
		&& (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE
		|| check_fwstate(pmlmepriv, WIFI_UNDER_LINKING) == _TRUE))
	{
		if(!_rtw_memcmp(mybssid, "\x0\x0\x0\x0\x0\x0", ETH_ALEN))
		{
			sta = rtw_get_stainfo(pstapriv, mybssid);
			if(sta && sta->phl_sta && !is_broadcast_mac_addr(sta->phl_sta->mac_addr))
			{
				sta->link_time += 2;
				curr_tx_bytes = 0;
				curr_rx_bytes = 0;
				if (sta->sta_stats.last_tx_bytes > sta->sta_stats.tx_bytes)
					sta->sta_stats.last_tx_bytes = sta->sta_stats.tx_bytes;
				if (sta->sta_stats.last_rx_bytes > sta->sta_stats.rx_bytes)
					sta->sta_stats.last_rx_bytes = sta->sta_stats.rx_bytes;
				if (sta->sta_stats.last_tx_pkts > sta->sta_stats.tx_pkts)/* PPS */
					sta->sta_stats.last_tx_pkts = sta->sta_stats.tx_pkts;

				curr_tx_bytes = sta->sta_stats.tx_bytes - sta->sta_stats.last_tx_bytes;
				curr_rx_bytes = sta->sta_stats.rx_bytes - sta->sta_stats.last_rx_bytes;
				sta->sta_stats.tx_data_pkts_cur = sta->sta_stats.tx_pkts - sta->sta_stats.last_tx_pkts;/* PPS */

				sta->sta_stats.tx_tp_kbits = (curr_tx_bytes * 8 / 2) >> 10;/*Kbps*/
				sta->sta_stats.rx_tp_kbits = (curr_rx_bytes * 8 / 2) >> 10;/*Kbps*/

				sta->sta_stats.last_tx_bytes = sta->sta_stats.tx_bytes;
				sta->sta_stats.last_rx_bytes = sta->sta_stats.rx_bytes;
				sta->sta_stats.last_tx_pkts = sta->sta_stats.tx_pkts;/* PPS */


				/* for Clinet mode dynsmic mechnism */
				_update_sta_tx_status(adapter, sta);
				sta_dynamic_control(adapter, sta);
				#ifdef CONFIG_DYNAMIC_THROUGHPUT_ENGINE
				throughput_dynamic_control(adapter);
				#endif
				//VCS_update(adapter, sta);
				display_sta_dump(adapter, sta);
			}
		}

	}
}

void rtw_iface_dynamic_chk_wk_hdl(_adapter *padapter)
{
	#if 1//def CONFIG_ACTIVE_KEEP_ALIVE_CHECK
	#ifdef CONFIG_AP_MODE
	if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter)) {
		expire_timeout_chk(padapter);
		#ifdef CONFIG_RTW_MESH
		if (MLME_IS_MESH(padapter) && MLME_IS_ASOC(padapter))
			rtw_mesh_peer_status_chk(padapter);
		#endif
	}
	#endif
	#endif /* CONFIG_ACTIVE_KEEP_ALIVE_CHECK */

	/* A4_CNT */
	#ifdef CONFIG_RTW_A4_STA
	if (adapter_en_a4(padapter))
		core_a4_sta_expire(padapter);
	#endif

	if(MLME_IS_STA(padapter))
	{
		collect_sta_traffic_statistics_for_sta_mode(padapter);
	}

	#ifdef CONFIG_AP_MODE
	if (MLME_IS_AP(padapter))
		feature_expire_timer(padapter);
	#endif

	dynamic_update_bcn_check(padapter);

	linked_status_chk(padapter, 0);
	traffic_status_watchdog(padapter, 0);

	/* for debug purpose */
	_linked_info_dump(padapter);

#ifdef CONFIG_RTW_CFGVENDOR_RSSIMONITOR
        rtw_cfgvendor_rssi_monitor_evt(padapter);
#endif
}

#ifdef CONFIG_USB_RX_AGGREGATION
extern void rtw_hal_usb_adjust_txagg(void *h);
#endif

void rtw_switch_wmm_mode(_adapter *padapter)
{
	struct dvobj_priv *dvobj = padapter->dvobj;
	u32 i, stream = 0;
	u8 wmm_mode = dvobj->wmm_mode;
	extern uint rtw_wifi_mode;
	extern uint rtw_wmm_dm;

	if (rtw_wmm_dm) {
		for (i = 0; i < 4; i++) {
			if (dvobj->tx_wmm_pkts[i] - dvobj->tx_wmm_pkts_prev[i] > 100)
				stream++;
		}
	}

	if (stream > 1) {
		wmm_mode = 1;
		dvobj->wmm_mode_to = 120;
		#ifdef CONFIG_LMT_TXREQ
		padapter->lmt_txreq_enable = 1;
		#endif
	} else {
		if (dvobj->wmm_mode_to)
			dvobj->wmm_mode_to -= 2;
		if (!dvobj->wmm_mode_to) {
			wmm_mode = 0;
			#ifdef CONFIG_LMT_TXREQ
			padapter->lmt_txreq_enable = 1;
			#endif
		}
	}

	if (wmm_mode != dvobj->wmm_mode) {
		dvobj->wmm_mode = wmm_mode;

		/* page ctrl - auto adjust mode */
		if (rtw_wifi_mode == 0) {
			if (wmm_mode) {
				dvobj->ac_page_reg[0] = rtw_phl_read32(padapter->dvobj->phl, 0x8a10);
				dvobj->ac_page_reg[1] = rtw_phl_read32(padapter->dvobj->phl, 0x8a14);
				dvobj->ac_page_reg[2] = rtw_phl_read32(padapter->dvobj->phl, 0x8a18);
				dvobj->ac_page_reg[3] = rtw_phl_read32(padapter->dvobj->phl, 0x8a1c);

                rtw_phl_write16(padapter->dvobj->phl, 0x8a12, dvobj->ac_page_reg[0] >> 18);
                rtw_phl_write16(padapter->dvobj->phl, 0x8a16, dvobj->ac_page_reg[1] >> 18);
                rtw_phl_write16(padapter->dvobj->phl, 0x8a1a, dvobj->ac_page_reg[2] >> 18);
                rtw_phl_write16(padapter->dvobj->phl, 0x8a1e, dvobj->ac_page_reg[3] >> 18);

#ifdef WIFI_LOGO_11N_WMM
				if (dvobj->wmm_test == 22)
					rtw_hw_set_edca(padapter, 0, 0xf642b);
				else
#endif
					rtw_hw_set_edca(padapter, 0, 0x642b);
			}
			else {
				rtw_phl_write32(padapter->dvobj->phl, 0x8a10, dvobj->ac_page_reg[0]);
				rtw_phl_write32(padapter->dvobj->phl, 0x8a14, dvobj->ac_page_reg[1]);
				rtw_phl_write32(padapter->dvobj->phl, 0x8a18, dvobj->ac_page_reg[2]);
				rtw_phl_write32(padapter->dvobj->phl, 0x8a1c, dvobj->ac_page_reg[3]);
			}
		}
	}
}

#ifdef CONFIG_WIFI_DIAGNOSIS
void _wifi_diag_expire(_adapter *padapter) {
	struct wifi_diag_obj *wifi_diag = &(padapter->dvobj->wifi_diag);
	int i;

	if (!wifi_diag->diag_ongoing)
		return;

	wifi_diag->diag_ongoing--;
	if (wifi_diag->diag_ongoing == 0) {
		rtw_hw_set_rx_mode(padapter, PHL_RX_MODE_NORMAL);
		if (wifi_diag->mode == DIAG_SPEC_STA)
			wifi_diag->target_list_num = 0;

		if (wifi_diag->diag_ch_switch) {
			wifi_diag->diag_ch_switch = 0;
			rtw_hw_set_ch_bw(padapter,
					rtw_get_oper_ch(padapter),
					rtw_get_oper_bw(padapter),
					rtw_get_oper_choffset(padapter),
					_TRUE);
		}
		wifi_diag->mode = DIAG_NONE;
	}
}
#endif

void rtw_dynamic_chk_wk_hdl(_adapter *padapter)
{
	rtw_switch_wmm_mode(padapter);

	rtw_mi_dynamic_chk_wk_hdl(padapter);

#ifdef DBG_CONFIG_ERROR_DETECT
	rtw_hal_sreset_xmit_status_check(padapter);
	rtw_hal_sreset_linked_status_check(padapter);
#endif

	/* if(check_fwstate(pmlmepriv, WIFI_UNDER_LINKING|WIFI_UNDER_SURVEY)==_FALSE) */
	{
#ifdef DBG_RX_COUNTER_DUMP
		rtw_dump_rx_counters(padapter);
#endif
#ifdef CONFIG_USB_RX_AGGREGATION
		rtw_hal_usb_adjust_txagg(GET_HAL_INFO(adapter_to_dvobj(padapter)));
#endif
	}
	rtw_hal_dm_watchdog(padapter);

	/* check_hw_pbc(padapter, pdrvextra_cmd->pbuf, pdrvextra_cmd->type); */

#ifdef CONFIG_IPS_CHECK_IN_WD
	/* always call rtw_ps_processor() at last one. */
	rtw_ps_processor(padapter);
#endif

#ifdef CONFIG_MCC_MODE
	rtw_hal_mcc_sw_status_check(padapter);
#endif /* CONFIG_MCC_MODE */

#ifdef CONFIG_RTW_SW_LED
	rtw_led_traffic_update(padapter);
#endif
}

void rtw_dynamic_chk_wk_sw_hdl(_adapter *padapter)
{

}

void rtw_dynamic_chk_wk_hw_hdl(_adapter *padapter)
{
	rtw_switch_wmm_mode(padapter);

	rtw_mi_dynamic_chk_wk_hdl(padapter);

#ifdef DBG_CONFIG_ERROR_DETECT
	rtw_hal_sreset_xmit_status_check(padapter);
	rtw_hal_sreset_linked_status_check(padapter);
#endif

	/* if(check_fwstate(pmlmepriv, WIFI_UNDER_LINKING|WIFI_UNDER_SURVEY)==_FALSE) */
	{
#ifdef DBG_RX_COUNTER_DUMP
		rtw_dump_rx_counters(padapter);
#endif
	}

#ifdef CONFIG_IPS_CHECK_IN_WD
	/* always call rtw_ps_processor() at last one. */
	rtw_ps_processor(padapter);
#endif

#ifdef CONFIG_MCC_MODE
	rtw_hal_mcc_sw_status_check(padapter);
#endif /* CONFIG_MCC_MODE */

#ifdef CONFIG_RTW_SW_LED
	rtw_led_traffic_update(padapter);
#endif

#ifdef CONFIG_WIFI_DIAGNOSIS
    	_wifi_diag_expire(padapter);
#endif
}


#ifdef CONFIG_LPS
struct lps_ctrl_wk_parm {
	s8 lps_level;
	#ifdef CONFIG_LPS_1T1R
	s8 lps_1t1r;
	#endif
};

void lps_ctrl_wk_hdl(_adapter *padapter, u8 lps_ctrl_type, u8 *buf)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct lps_ctrl_wk_parm *parm = (struct lps_ctrl_wk_parm *)buf;
	u8	mstatus;

	if ((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE)
	    || (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE))
		return;

	switch (lps_ctrl_type) {
	case LPS_CTRL_SCAN:
		/* RTW_INFO("LPS_CTRL_SCAN\n"); */
		if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE) {
			/* connect */
			LPS_Leave(padapter, "LPS_CTRL_SCAN");
		}
		break;
	case LPS_CTRL_JOINBSS:
		/* RTW_INFO("LPS_CTRL_JOINBSS\n"); */
		LPS_Leave(padapter, "LPS_CTRL_JOINBSS");
		break;
	case LPS_CTRL_CONNECT:
		/* RTW_INFO("LPS_CTRL_CONNECT\n"); */
		mstatus = 1;/* connect */
		/* Reset LPS Setting */
		pwrpriv->LpsIdleCount = 0;
		rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_JOINBSSRPT, (u8 *)(&mstatus));
		break;
	case LPS_CTRL_DISCONNECT:
		/* RTW_INFO("LPS_CTRL_DISCONNECT\n"); */
		mstatus = 0;/* disconnect */
		LPS_Leave(padapter, "LPS_CTRL_DISCONNECT");
		rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_JOINBSSRPT, (u8 *)(&mstatus));
		break;
	case LPS_CTRL_SPECIAL_PACKET:
		/* RTW_INFO("LPS_CTRL_SPECIAL_PACKET\n"); */
		rtw_set_lps_deny(padapter, LPS_DELAY_MS);
		LPS_Leave(padapter, "LPS_CTRL_SPECIAL_PACKET");
		break;
	case LPS_CTRL_LEAVE:
		LPS_Leave(padapter, "LPS_CTRL_LEAVE");
		break;
	case LPS_CTRL_LEAVE_SET_OPTION:
		LPS_Leave(padapter, "LPS_CTRL_LEAVE_SET_OPTION");
		if (parm) {
			if (parm->lps_level >= 0)
				pwrpriv->lps_level = parm->lps_level;
			#ifdef CONFIG_LPS_1T1R
			if (parm->lps_1t1r >= 0)
				pwrpriv->lps_1t1r = parm->lps_1t1r;
			#endif
		}
		break;
	case LPS_CTRL_LEAVE_CFG80211_PWRMGMT:
		LPS_Leave(padapter, "CFG80211_PWRMGMT");
		break;
	case LPS_CTRL_TRAFFIC_BUSY:
		LPS_Leave(padapter, "LPS_CTRL_TRAFFIC_BUSY");
		break;
	case LPS_CTRL_TX_TRAFFIC_LEAVE:
		LPS_Leave(padapter, "LPS_CTRL_TX_TRAFFIC_LEAVE");
		break;
	case LPS_CTRL_RX_TRAFFIC_LEAVE:
		LPS_Leave(padapter, "LPS_CTRL_RX_TRAFFIC_LEAVE");
		break;
	case LPS_CTRL_ENTER:
		LPS_Enter(padapter, "TRAFFIC_IDLE_1");
		break;
	default:
		break;
	}

}

static u8 _rtw_lps_ctrl_wk_cmd(_adapter *adapter, u8 lps_ctrl_type, s8 lps_level, s8 lps_1t1r, u8 flags)
{
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *parm;
	struct lps_ctrl_wk_parm *wk_parm = NULL;
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct submit_ctx sctx;
	u8	res = _SUCCESS;

	if (lps_ctrl_type == LPS_CTRL_LEAVE_SET_OPTION) {
		wk_parm = rtw_zmalloc(sizeof(*wk_parm));
		if (wk_parm == NULL) {
			res = _FAIL;
			goto exit;
		}
		wk_parm->lps_level = lps_level;
		#ifdef CONFIG_LPS_1T1R
		wk_parm->lps_1t1r = lps_1t1r;
		#endif
	}

	if (flags & RTW_CMDF_DIRECTLY) {
		/* no need to enqueue, do the cmd hdl directly */
		lps_ctrl_wk_hdl(adapter, lps_ctrl_type, (u8 *)wk_parm);
		if (wk_parm)
			rtw_mfree(wk_parm, sizeof(*wk_parm));
	} else {
		/* need enqueue, prepare cmd_obj and enqueue */
		parm = rtw_zmalloc(sizeof(*parm));
		if (parm == NULL) {
			if (wk_parm)
				rtw_mfree(wk_parm, sizeof(*wk_parm));
			res = _FAIL;
			goto exit;
		}

		parm->ec_id = LPS_CTRL_WK_CID;
		parm->type = lps_ctrl_type;
		parm->size = wk_parm ? sizeof(*wk_parm) : 0;
		parm->pbuf = (u8 *)wk_parm;

		cmdobj = rtw_zmalloc(sizeof(*cmdobj));
		if (cmdobj == NULL) {
			rtw_mfree(parm, sizeof(*parm));
			if (wk_parm)
				rtw_mfree(wk_parm, sizeof(*wk_parm));
			res = _FAIL;
			goto exit;
		}
		cmdobj->padapter = adapter;

		init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_SET_DRV_EXTRA);

		if (flags & RTW_CMDF_WAIT_ACK) {
			cmdobj->sctx = &sctx;
			rtw_sctx_init(&sctx, 2000);
		}

		res = rtw_enqueue_cmd(pcmdpriv, cmdobj);

		if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				cmdobj->sctx = NULL;
			_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
			if (sctx.status != RTW_SCTX_DONE_SUCCESS)
				res = _FAIL;
		}
	}

exit:
	return res;
}

u8 rtw_lps_ctrl_wk_cmd(_adapter *adapter, u8 lps_ctrl_type, u8 flags)
{
	return _rtw_lps_ctrl_wk_cmd(adapter, lps_ctrl_type, -1, -1, flags);
}

u8 rtw_lps_ctrl_leave_set_level_cmd(_adapter *adapter, u8 lps_level, u8 flags)
{
	return _rtw_lps_ctrl_wk_cmd(adapter, LPS_CTRL_LEAVE_SET_OPTION, lps_level, -1, flags);
}

#ifdef CONFIG_LPS_1T1R
u8 rtw_lps_ctrl_leave_set_1t1r_cmd(_adapter *adapter, u8 lps_1t1r, u8 flags)
{
	return _rtw_lps_ctrl_wk_cmd(adapter, LPS_CTRL_LEAVE_SET_OPTION, -1, lps_1t1r, flags);
}
#endif

void rtw_dm_in_lps_hdl(_adapter *padapter)
{
	rtw_hal_set_hwreg(padapter, HW_VAR_DM_IN_LPS_LCLK, NULL);
}

u8 rtw_dm_in_lps_wk_cmd(_adapter *padapter)
{
	struct cmd_obj	*cmd;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	u8	res = _SUCCESS;


	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmd->padapter = padapter;

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = DM_IN_LPS_WK_CID;
	pdrvextra_cmd_parm->type = 0;
	pdrvextra_cmd_parm->size = 0;
	pdrvextra_cmd_parm->pbuf = NULL;

	init_h2fwcmd_w_parm_no_rsp(cmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);

	res = rtw_enqueue_cmd(pcmdpriv, cmd);

exit:
	return res;
}

void rtw_lps_change_dtim_hdl(_adapter *padapter, u8 dtim)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);

	if (dtim <= 0 || dtim > 16)
		return;

#ifdef CONFIG_LPS_LCLK
	_enter_pwrlock(&pwrpriv->lock);
#endif

	if (pwrpriv->dtim != dtim) {
		RTW_INFO("change DTIM from %d to %d, bFwCurrentInPSMode=%d, ps_mode=%d\n", pwrpriv->dtim, dtim,
			 pwrpriv->bFwCurrentInPSMode, pwrpriv->pwr_mode);

		pwrpriv->dtim = dtim;
	}

	if ((pwrpriv->bFwCurrentInPSMode == _TRUE) && (pwrpriv->pwr_mode > PM_PS_MODE_ACTIVE)) {
		u8 ps_mode = pwrpriv->pwr_mode;

		/* RTW_INFO("change DTIM from %d to %d, ps_mode=%d\n", pwrpriv->dtim, dtim, ps_mode); */

		rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_PWRMODE, (u8 *)(&ps_mode));
	}

#ifdef CONFIG_LPS_LCLK
	_exit_pwrlock(&pwrpriv->lock);
#endif

}

#endif

u8 rtw_lps_change_dtim_cmd(_adapter *padapter, u8 dtim)
{
	struct cmd_obj	*cmd;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	u8 res = _SUCCESS;
	/*
	#ifdef CONFIG_CONCURRENT_MODE
		if (padapter->hw_port != HW_PORT0)
			return res;
	#endif
	*/
	{
		cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (cmd == NULL) {
			res = _FAIL;
			goto exit;
		}
		cmd->padapter = padapter;

		pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
		if (pdrvextra_cmd_parm == NULL) {
			rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
			res = _FAIL;
			goto exit;
		}

		pdrvextra_cmd_parm->ec_id = LPS_CHANGE_DTIM_CID;
		pdrvextra_cmd_parm->type = dtim;
		pdrvextra_cmd_parm->size = 0;
		pdrvextra_cmd_parm->pbuf = NULL;

		init_h2fwcmd_w_parm_no_rsp(cmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);

		res = rtw_enqueue_cmd(pcmdpriv, cmd);
	}
exit:
	return res;
}

#ifdef CONFIG_ANTENNA_DIVERSITY
void antenna_select_wk_hdl(_adapter *padapter, u8 antenna)
{
	rtw_hal_set_phydm_var(padapter, HAL_PHYDM_ANTDIV_SELECT, &antenna, _TRUE);
}

u8 rtw_antenna_select_cmd(_adapter *padapter, u8 antenna, u8 enqueue)
{
	struct cmd_obj *cmd;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct cmd_priv	*pcmdpriv = &dvobj->cmdpriv;
	u8	bSupportAntDiv = _FALSE;
	u8	res = _SUCCESS;
	int	i;

	rtw_hal_get_def_var(padapter, HAL_DEF_IS_SUPPORT_ANT_DIV, &(bSupportAntDiv));
	if (_FALSE == bSupportAntDiv)
		return _FAIL;

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (rtw_linked_check(dvobj->padapters[i]))
			return _FAIL;
	}

	if (_TRUE == enqueue) {
		cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (cmd == NULL) {
			res = _FAIL;
			goto exit;
		}
		cmd->padapter = padapter;

		pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
		if (pdrvextra_cmd_parm == NULL) {
			rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
			res = _FAIL;
			goto exit;
		}

		pdrvextra_cmd_parm->ec_id = ANT_SELECT_WK_CID;
		pdrvextra_cmd_parm->type = antenna;
		pdrvextra_cmd_parm->size = 0;
		pdrvextra_cmd_parm->pbuf = NULL;
		init_h2fwcmd_w_parm_no_rsp(cmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);

		res = rtw_enqueue_cmd(pcmdpriv, cmd);
	} else
		antenna_select_wk_hdl(padapter, antenna);
exit:
	return res;
}
#endif

void rtw_dm_ra_mask_hdl(_adapter *padapter, struct sta_info *psta)
{
	if (psta)
		rtw_hal_sta_ra_registed(psta);
}

u8 rtw_dm_ra_mask_wk_cmd(_adapter *padapter, u8 *psta)
{
	struct cmd_obj	*cmd;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	u8 res = _SUCCESS;


	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmd->padapter = padapter;

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = DM_RA_MSK_WK_CID;
	pdrvextra_cmd_parm->type = 0;
	pdrvextra_cmd_parm->size = 0;
	pdrvextra_cmd_parm->pbuf = psta;

	init_h2fwcmd_w_parm_no_rsp(cmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);

	res = rtw_enqueue_cmd(pcmdpriv, cmd);

exit:
	return res;
}
#ifdef CONFIG_POWER_SAVING
void power_saving_wk_hdl(_adapter *padapter)
{
	rtw_ps_processor(padapter);
}
#endif
/* add for CONFIG_IEEE80211W, none 11w can use it */
void reset_securitypriv_hdl(_adapter *padapter)
{
	rtw_reset_securitypriv(padapter);
}

#ifdef CONFIG_P2P
u8 p2p_protocol_wk_cmd(_adapter *padapter, int intCmdType)
{
	struct cmd_obj	*cmd;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	u8 res = _SUCCESS;


	if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
		return res;

	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmd->padapter = padapter;

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = P2P_PROTO_WK_CID;
	pdrvextra_cmd_parm->type = intCmdType;	/*	As the command tppe. */
	pdrvextra_cmd_parm->size = 0;
	pdrvextra_cmd_parm->pbuf = NULL;		/*	Must be NULL here */

	init_h2fwcmd_w_parm_no_rsp(cmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);

	res = rtw_enqueue_cmd(pcmdpriv, cmd);

exit:
	return res;

}

#ifdef CONFIG_IOCTL_CFG80211
static u8 _p2p_roch_cmd(_adapter *adapter
	, u64 cookie, struct wireless_dev *wdev
	, struct ieee80211_channel *ch, enum nl80211_channel_type ch_type
	, unsigned int duration
	, u8 flags
)
{
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *parm;
	struct p2p_roch_parm *roch_parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct submit_ctx sctx;
	u8 cancel = duration ? 0 : 1;
	u8 res = _SUCCESS;

	roch_parm = (struct p2p_roch_parm *)rtw_zmalloc(sizeof(struct p2p_roch_parm));
	if (roch_parm == NULL) {
		res = _FAIL;
		goto exit;
	}

	roch_parm->cookie = cookie;
	roch_parm->wdev = wdev;
	if (!cancel) {
		_rtw_memcpy(&roch_parm->ch, ch, sizeof(struct ieee80211_channel));
		roch_parm->ch_type = ch_type;
		roch_parm->duration = duration;
	}

	if (flags & RTW_CMDF_DIRECTLY) {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (H2C_SUCCESS != p2p_protocol_wk_hdl(adapter, cancel ? P2P_CANCEL_RO_CH_WK : P2P_RO_CH_WK, (u8 *)roch_parm))
			res = _FAIL;
		rtw_mfree((u8 *)roch_parm, sizeof(*roch_parm));
	} else {
		/* need enqueue, prepare cmd_obj and enqueue */
		parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
		if (parm == NULL) {
			rtw_mfree((u8 *)roch_parm, sizeof(*roch_parm));
			res = _FAIL;
			goto exit;
		}

		parm->ec_id = P2P_PROTO_WK_CID;
		parm->type = cancel ? P2P_CANCEL_RO_CH_WK : P2P_RO_CH_WK;
		parm->size = sizeof(*roch_parm);
		parm->pbuf = (u8 *)roch_parm;

		cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(*cmdobj));
		if (cmdobj == NULL) {
			res = _FAIL;
			rtw_mfree((u8 *)roch_parm, sizeof(*roch_parm));
			rtw_mfree((u8 *)parm, sizeof(*parm));
			goto exit;
		}
		cmdobj->padapter = adapter;

		init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_SET_DRV_EXTRA);

		if (flags & RTW_CMDF_WAIT_ACK) {
			cmdobj->sctx = &sctx;
			rtw_sctx_init(&sctx, 10 * 1000);
		}

		res = rtw_enqueue_cmd(pcmdpriv, cmdobj);

		if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				cmdobj->sctx = NULL;
			_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
			if (sctx.status != RTW_SCTX_DONE_SUCCESS)
				res = _FAIL;
		}
	}

exit:
	return res;
}
#if 0
inline u8 p2p_roch_cmd(_adapter *adapter
	, u64 cookie, struct wireless_dev *wdev
	, struct ieee80211_channel *ch, enum nl80211_channel_type ch_type
	, unsigned int duration
	, u8 flags
)
{
#ifdef CONFIG_PHL_ARCH
	rtw_phl_remain_on_ch_cmd(adapter,
		wdev, ch,ch_type, duration, cookie, flags);
	return _SUCCESS;
#else
	return _p2p_roch_cmd(adapter, cookie, wdev, ch, ch_type, duration, flags);
#endif
}
#endif

inline u8 p2p_cancel_roch_cmd(_adapter *adapter, u64 cookie, struct wireless_dev *wdev, u8 flags)
{
#ifdef CONFIG_PHL_ARCH
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
#ifdef CONFIG_FSM
	rtw_phl_scan_abort(adapter);
#endif
	return _SUCCESS;
#else /* !CONFIG_PHL_ARCH */
	return _p2p_roch_cmd(adapter, cookie, wdev, NULL, 0, 0, flags);
#endif
}

#endif /* CONFIG_IOCTL_CFG80211 */
#endif /* CONFIG_P2P */

#ifdef CONFIG_IOCTL_CFG80211
inline u8 rtw_mgnt_tx_cmd(_adapter *adapter, u8 tx_ch, u8 no_cck,
			  const u8 *buf, size_t len, u32 wait_ack,
			 u32 wait_ms, u8 flags)
{
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *parm;
	struct mgnt_tx_parm *mgnt_parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct submit_ctx sctx;
	u8	res = _SUCCESS;

	mgnt_parm = (struct mgnt_tx_parm *)rtw_zmalloc(sizeof(struct mgnt_tx_parm));
	if (mgnt_parm == NULL) {
		res = _FAIL;
		goto exit;
	}

	mgnt_parm->tx_ch = tx_ch;
	mgnt_parm->no_cck = no_cck;
	mgnt_parm->buf = buf;
	mgnt_parm->len = len;
	mgnt_parm->wait_ack = wait_ack;
	mgnt_parm->wait_ms = wait_ms;

	if (flags & RTW_CMDF_DIRECTLY) {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (H2C_SUCCESS != rtw_mgnt_tx_handler(adapter, (u8 *)mgnt_parm))
			res = _FAIL;
		rtw_mfree((u8 *)mgnt_parm, sizeof(*mgnt_parm));
	} else {
		/* need enqueue, prepare cmd_obj and enqueue */
		parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
		if (parm == NULL) {
			rtw_mfree((u8 *)mgnt_parm, sizeof(*mgnt_parm));
			res = _FAIL;
			goto exit;
		}

		parm->ec_id = MGNT_TX_WK_CID;
		parm->type = 0;
		parm->size = sizeof(*mgnt_parm);
		parm->pbuf = (u8 *)mgnt_parm;

		cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(*cmdobj));
		if (cmdobj == NULL) {
			res = _FAIL;
			rtw_mfree((u8 *)mgnt_parm, sizeof(*mgnt_parm));
			rtw_mfree((u8 *)parm, sizeof(*parm));
			goto exit;
		}
		cmdobj->padapter = adapter;

		init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_SET_DRV_EXTRA);

		if (flags & RTW_CMDF_WAIT_ACK) {
			cmdobj->sctx = &sctx;
			rtw_sctx_init(&sctx, 10 * 1000);
		}

		res = rtw_enqueue_cmd(pcmdpriv, cmdobj);

		if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				cmdobj->sctx = NULL;
			_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
			if (sctx.status != RTW_SCTX_DONE_SUCCESS)
				res = _FAIL;
		}
	}

exit:
	return res;
}
#endif

#ifdef CONFIG_POWER_SAVING
u8 rtw_ps_cmd(_adapter *padapter)
{
	struct cmd_obj	*ppscmd;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;

	u8	res = _SUCCESS;

#ifdef CONFIG_CONCURRENT_MODE
	if (!is_primary_adapter(padapter))
		goto exit;
#endif

	ppscmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ppscmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	ppscmd->padapter = padapter;

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)ppscmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = POWER_SAVING_CTRL_WK_CID;
	pdrvextra_cmd_parm->type = 0;
	pdrvextra_cmd_parm->size = 0;
	pdrvextra_cmd_parm->pbuf = NULL;
	init_h2fwcmd_w_parm_no_rsp(ppscmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);

	res = rtw_enqueue_cmd(pcmdpriv, ppscmd);

exit:


	return res;

}
#endif /*CONFIG_POWER_SAVING*/

#if CONFIG_DFS
void rtw_dfs_ch_switch_hdl(struct dvobj_priv *dvobj)
{
	struct rf_ctl_t *rfctl = dvobj_to_rfctl(dvobj);
	_adapter *pri_adapter = dvobj_get_primary_adapter(dvobj);
	u16 ifbmp_m = rtw_mi_get_ap_mesh_ifbmp(pri_adapter);
	u16 ifbmp_s = rtw_mi_get_ld_sta_ifbmp(pri_adapter);
	s16 req_ch;

#ifdef CONFIG_RTW_MULTI_AP
	if (GET_MAP_BSS_TYPE(pri_adapter))
		core_map_send_channel_change_notify();
#endif

	rtw_hal_macid_sleep_all_used(pri_adapter);

	if (rtw_chset_search_ch(rfctl->channel_set, rfctl->csa_ch) >= 0
		&& !rtw_chset_is_ch_non_ocp(rfctl->channel_set, rfctl->csa_ch)
	) {
		/* CSA channel available and valid */
		req_ch = rfctl->csa_ch;
		RTW_INFO("%s valid CSA ch%u\n", __func__, rfctl->csa_ch);
	} else if (ifbmp_m) {
		/* no available or valid CSA channel, having AP/MESH ifaces */
		req_ch = REQ_CH_NONE;
		RTW_INFO("%s ch sel by AP/MESH ifaces\n", __func__);
	} else {
		/* no available or valid CSA channel and no AP/MESH ifaces */
		if (!is_supported_24g(dvobj_to_regsty(dvobj)->band_type)
			#ifdef CONFIG_DFS_MASTER
			|| rfctl->radar_detected
			#endif
		)
			req_ch = 36;
		else
			req_ch = 1;
		RTW_INFO("%s switch to ch%d\n", __func__, req_ch);
	}

#ifdef CONFIG_DFS_CSA_IE
	// in the case of CSA, don't trigger disconnection
	if (rfctl->csa_ch == 0)
#endif
	{
		/*  issue deauth for all asoc STA ifaces */
		if (ifbmp_s) {
			_adapter *iface;
			int i;

			for (i = 0; i < dvobj->iface_nums; i++) {
				iface = dvobj->padapters[i];
				if (!iface || !(ifbmp_s & BIT(iface->iface_id)))
					continue;
				set_fwstate(&iface->mlmepriv, WIFI_OP_CH_SWITCHING);

				/* TODO: true op ch switching */
				issue_deauth_g6(iface, get_bssid(&iface->mlmepriv), WLAN_REASON_DEAUTH_LEAVING);
			}
		}

		/* make asoc STA ifaces disconnect */
		/* TODO: true op ch switching */
		if (ifbmp_s) {
			_adapter *iface;
			int i;

			for (i = 0; i < dvobj->iface_nums; i++) {
				iface = dvobj->padapters[i];
				if (!iface || !(ifbmp_s & BIT(iface->iface_id)))
					continue;
				rtw_disassoc_cmd(iface, 0, RTW_CMDF_DIRECTLY);
				rtw_indicate_disconnect(iface, 0, _FALSE);
				rtw_free_assoc_resources(iface, _TRUE);
				rtw_free_network_queue(iface, _TRUE);
			}
		}
	}

	rfctl->csa_set_ie = 0;

#ifdef CONFIG_AP_MODE
	if (ifbmp_m) {
		/* trigger channel selection without consideraton of asoc STA ifaces */
		rtw_change_bss_chbw_cmd(dvobj_get_primary_adapter(dvobj), RTW_CMDF_DIRECTLY
			, ifbmp_m, ifbmp_s, req_ch, REQ_BW_ORI, REQ_OFFSET_NONE);
	} else
#endif
	{
		/* no AP/MESH iface, switch DFS status and channel directly */
		rtw_warn_on(req_ch <= 0);

#ifdef CONFIG_DFS_MASTER
		rtw_dfs_rd_en_decision(pri_adapter, MLME_OPCH_SWITCH, ifbmp_s);		// pure client mode need ?
#endif

#ifdef CONFIG_DFS_CSA_IE
		set_channel_bwmode(pri_adapter,
					req_ch,
					rtw_get_oper_choffset(pri_adapter),
					rtw_get_oper_bw(pri_adapter),
					_FALSE);
#endif
	}

	if(rfctl->csa_ch) {
		rtw_dfs_csa_hw_tx_pause(pri_adapter, _FALSE);
	}

	rfctl->csa_ch = 0;
	rfctl->csa_cntdown = 0;

	rtw_hal_macid_wakeup_all_used(pri_adapter);
	rtw_mi_os_xmit_schedule(pri_adapter);
}
#endif /* CONFIG_DFS */

#ifdef CONFIG_AP_MODE

static void rtw_chk_hi_queue_hdl(_adapter *padapter)
{
	struct sta_info *psta_bmc;
	struct sta_priv *pstapriv = &padapter->stapriv;
	systime start = rtw_get_current_time();
	u8 empty = _FALSE;

	psta_bmc = rtw_get_bcmc_stainfo(padapter);
	if (!psta_bmc)
		return;

	rtw_hal_get_hwreg(padapter, HW_VAR_CHK_HI_QUEUE_EMPTY, &empty);

	while (_FALSE == empty && rtw_get_passing_time_ms(start) < rtw_get_wait_hiq_empty_ms()) {
		rtw_msleep_os(100);
		rtw_hal_get_hwreg(padapter, HW_VAR_CHK_HI_QUEUE_EMPTY, &empty);
	}

	if (psta_bmc->sleep_q.qlen == 0) {
		if (empty == _SUCCESS) {
			bool update_tim = _FALSE;

			if (rtw_tim_map_is_set(padapter, pstapriv->tim_bitmap, 0))
				update_tim = _TRUE;

			rtw_tim_map_clear(padapter, pstapriv->tim_bitmap, 0);
			rtw_tim_map_clear(padapter, pstapriv->sta_dz_bitmap, 0);

			if (update_tim == _TRUE)
				_update_beacon(padapter, _TIM_IE_, NULL, _TRUE, 0,"bmc sleepq and HIQ empty");
		} else /* re check again */
			rtw_chk_hi_queue_cmd(padapter);

	}

}

u8 rtw_chk_hi_queue_cmd(_adapter *padapter)
{
	struct cmd_obj	*cmd;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	u8	res = _SUCCESS;

	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmd->padapter = padapter;

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = CHECK_HIQ_WK_CID;
	pdrvextra_cmd_parm->type = 0;
	pdrvextra_cmd_parm->size = 0;
	pdrvextra_cmd_parm->pbuf = NULL;

	init_h2fwcmd_w_parm_no_rsp(cmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);

	res = rtw_enqueue_cmd(pcmdpriv, cmd);

exit:
	return res;

}

#endif /* CONFIG_AP_MODE */

#ifdef RTW_PHL_DBG_CMD

u32 core_record_add_idx = 0xffffffff;
u32 core_record_dump_idx = 0; //REC_RX_PER_IND;

u32 phl_record_add_idx = 0xffffffff;
u32 phl_reocrd_dump_idx = 0; //REC_RXWD;

u8 sample_txwd[] =
{
/*dword 00*/	0x80, 0x00, 0x40, 0x00,
/*dword 01*/	0x00, 0x00, 0x00, 0x00,
/*dword 02*/	0xF2, 0x05, 0x00, 0x00,
/*dword 03*/	0x3E, 0x11, 0x00, 0x00,
/*dword 04*/	0x00, 0x00, 0x00, 0x00,
/*dword 05*/	0x00, 0x00, 0x00, 0x00,
/*dword 06*/	0x00, 0x07, 0x9B, 0x63,
/*dword 07*/	0x3F, 0x00, 0x00, 0x00,
/*dword 08*/	0x00, 0x00, 0x00, 0x00,
/*dword 09*/	0x00, 0x00, 0x00, 0x00,
/*dword 10*/	0x0C, 0x00, 0x00, 0x00,
/*dword 11*/	0x00, 0x00, 0x00, 0x00,
};

#define WD_SPEC(_name, _idx_dw, _bit_start, _bit_end) {              \
	.name        	= (_name),                   \
	.idx_dw         = (_idx_dw),                     \
	.bit_start      = (_bit_start),                   \
	.bit_end 		= (_bit_end)  \
}

struct parse_wd {
	char name[32];
	u8 idx_dw;
	u8 bit_start;
	u8 bit_end;
};

#define MAX_PHL_CMD_LEN 200
#define MAX_PHL_CMD_NUM 10
#define MAX_PHL_CMD_STR_LEN 200

char *get_next_para_str(char *para)
{
	return (para+MAX_PHL_CMD_STR_LEN);
}

static struct parse_wd parse_txwd_8852ae_full[] = {
	WD_SPEC("EN_HWSEQ_MODE"			, 0, 0, 1),
	WD_SPEC("HW_SSN_SEL"			, 0, 2, 3),
	WD_SPEC("SMH_EN"				, 0, 4, 4),
	WD_SPEC("HWAMSDU"				, 0, 5, 5),
	WD_SPEC("HW_AES_IV"				, 0, 6, 6),
	WD_SPEC("WD page"				, 0, 7, 7),
	WD_SPEC("CHK_EN"				, 0, 8, 8),
	WD_SPEC("WP_INT"				, 0, 9, 9),
	WD_SPEC("STF mode"				, 0, 10, 10),
	WD_SPEC("HEADERwLLC_LEN"		, 0, 11, 15),
	WD_SPEC("CHANNEL_DMA"			, 0, 16, 19),
	WD_SPEC("FW_download"			, 0, 20, 20),
	WD_SPEC("PKT_OFFSET"			, 0, 21, 21),
	WD_SPEC("WDINFO_EN"				, 0, 22, 22),
	WD_SPEC("MOREDATA"				, 0, 23, 23),
	WD_SPEC("WP_OFFSET" 			, 0, 24, 31),

	WD_SPEC("SHCUT_CAMID"			, 1, 0, 7),
	WD_SPEC("DMA_TXAGG_NUM"			, 1, 8, 15),
	WD_SPEC("PLD(Packet ID)"		, 1, 16, 31),

	WD_SPEC("TXPKTSIZE"			, 2, 0, 13),
	WD_SPEC("RU_TC"				, 2, 14, 16),
	WD_SPEC("QSEL"				, 2, 17, 22),
	WD_SPEC("TID_indicate"		, 2, 23, 23),
	WD_SPEC("MACID"				, 2, 24, 30),
	WD_SPEC("RSVD"				, 2, 31, 31),

	WD_SPEC("Wifi_SEQ"			, 3, 0, 11),
	WD_SPEC("AGG_EN"			, 3, 12, 12),
	WD_SPEC("BK"				, 3, 13, 13),
	WD_SPEC("RTS_TC"			, 3, 14, 19),
	WD_SPEC("DATA_TC"			, 3, 20, 25),
	WD_SPEC("MU_2nd_TC"			, 3, 26, 28),
	WD_SPEC("MU_TC"				, 3, 29, 31),

	WD_SPEC("TIMESTAMP"			, 4, 0, 15),
	WD_SPEC("AES_IV_L"			, 4, 16, 31),

	WD_SPEC("AES_IV_H"			, 5, 0, 31),

	WD_SPEC("MBSSID"			, 6, 0, 3),
	WD_SPEC("Multiport_ID"		, 6, 4, 6),
	WD_SPEC("RSVD"				, 6, 7, 7),
	WD_SPEC("DATA_BW_ER"		, 6, 8, 8),
	WD_SPEC("DISRTSFB"			, 6, 9, 9),
	WD_SPEC("DISDATAFB"			, 6, 10, 10),
	WD_SPEC("DATA_LDPC"			, 6, 11, 11),
	WD_SPEC("DATA_STBC"			, 6, 12, 13),
	WD_SPEC("DATA_DCM"			, 6, 14, 14),
	WD_SPEC("DATA_ER"			, 6, 15, 15),
	WD_SPEC("DataRate"			, 6, 16, 24),
	WD_SPEC("GI_LTF"			, 6, 25, 27),
	WD_SPEC("DATA_BW"			, 6, 28, 29),
	WD_SPEC("USERATE_SEL" 		, 6, 30, 30),
	WD_SPEC("ACK_CH_INFO"		, 6, 31, 31),

	WD_SPEC("MAX_AGG_NUM"				, 7, 0, 7),
	WD_SPEC("BCN_SRCH_SEQ"				, 7, 8, 9),
	WD_SPEC("NAVUSEHDR"					, 7, 10, 10),
	WD_SPEC("BMC"						, 7, 11, 11),
	WD_SPEC("A_CTRL_BQR"				, 7, 12, 12),
	WD_SPEC("A_CTRL_UPH"				, 7, 13, 13),
	WD_SPEC("A_CTRL_BSR"				, 7, 14, 14),
	WD_SPEC("A_CTRL_CAS"				, 7, 15, 15),
	WD_SPEC("DATA_RTY_LOWEST_RATE"		, 7, 16, 24),
	WD_SPEC("DATA_TXCNT_LMT"			, 7, 25, 30),
	WD_SPEC("DATA_TXCNT_LMT_SEL"		, 7, 31, 31),

	WD_SPEC("SEC_CAM_IDX"				, 8, 0, 7),
	WD_SPEC("SEC_HW_ENC"				, 8, 8, 8),
	WD_SPEC("SECTYPE"					, 8, 9, 12),
	WD_SPEC("lifetime_sel"				, 8, 13, 15),
	WD_SPEC("RSVD"						, 8, 16, 16),
	WD_SPEC("FORCE_TXOP"				, 8, 17, 17),
	WD_SPEC("AMPDU_DENSITY"				, 8, 18, 20),
	WD_SPEC("LSIG_TXOP_EN"				, 8, 21, 21),
	WD_SPEC("TXPWR_OFSET_TYPE"			, 8, 22, 24),
	WD_SPEC("RSVD"						, 8, 25, 25),
	WD_SPEC("obw_cts2self_dup_type"		, 8, 26, 29),
	WD_SPEC("RSVD"						, 8, 30, 31),

	WD_SPEC("Signaling_TA_PKT_EN"		, 9, 0, 0),
	WD_SPEC("NDPA"						, 9, 1, 2),
	WD_SPEC("SND_PKT_SEL"				, 9, 3, 5),
	WD_SPEC("SIFS_Tx"					, 9, 6, 6),
	WD_SPEC("HT_DATA_SND" 				, 9, 7, 7),
	WD_SPEC("RSVD"						, 9, 8, 8),
	WD_SPEC("RTT_EN" 					, 9, 9, 9),
	WD_SPEC("SPE_RPT" 					, 9, 10, 10),
	WD_SPEC("BT_NULL" 					, 9, 11, 11),
	WD_SPEC("TRI_FRAME"					, 9, 12, 12),
	WD_SPEC("NULL_1" 					, 9, 13, 13),
	WD_SPEC("NULL_0" 					, 9, 14, 14),
	WD_SPEC("RAW" 						, 9, 15, 15),
	WD_SPEC("Group_bit"					, 9, 16, 23),
	WD_SPEC("RSVD" 						, 9, 24, 25),
	WD_SPEC("BCNPKT_TSF_CTRL" 			, 9, 26, 26),
	WD_SPEC("Signaling_TA_PKT_SC" 		, 9, 27, 30),
	WD_SPEC("FORCE_BSS_CLR" 			, 9, 31, 31),

	WD_SPEC("SW_DEFINE"					, 10, 0, 3),
	WD_SPEC("RSVD"						, 10, 4, 26),
	WD_SPEC("RTS_EN"					, 10, 27, 27),
	WD_SPEC("CTS2SELF"					, 10, 28, 28),
	WD_SPEC("CCA_RTS" 					, 10, 29, 30),
	WD_SPEC("HW_RTS_EN" 				, 10, 31, 31),

	WD_SPEC("RSVD"						, 11, 0, 3),
	WD_SPEC("NDPA_duration"				, 11, 4, 26),
};

static struct parse_wd parse_rxwd_8852ae[] = {
	WD_SPEC("PKT_LEN"				, 0, 0, 13),
	WD_SPEC("SHIFT"					, 0, 14, 15),
	WD_SPEC("WL_HD_IV_LEN"			, 0, 16, 21),
	WD_SPEC("BB_SEL"				, 0, 22, 22),
	WD_SPEC("MAC_INFO_VLD"			, 0, 23, 23),
	WD_SPEC("RPKT_TYPE"				, 0, 24, 27),
	WD_SPEC("DRV_INFO_SIZE"			, 0, 28, 30),
	WD_SPEC("LONG_RXD"				, 0, 31, 31),

	WD_SPEC("PPDU_TYPE"			, 1, 0, 3),
	WD_SPEC("PPDU_CNT"			, 1, 4, 6),
	WD_SPEC("SR_EN"				, 1, 7, 7),
	WD_SPEC("USER_ID"			, 1, 8, 15),
	WD_SPEC("RX_DATARATE" 		, 1, 16, 24),
	WD_SPEC("RX_GI_LTF"			, 1, 25, 27),
	WD_SPEC("NON_SRG_PPDU"		, 1, 28, 28),
	WD_SPEC("INTER_PPDU" 		, 1, 29, 29),
	WD_SPEC("BW"				, 1, 30, 31),

	WD_SPEC("FREERUN_CNT"		, 2, 0, 31),

	WD_SPEC("A1_MATCH"			, 3, 1, 1),
	WD_SPEC("SW_DEC"			, 3, 2, 2),
	WD_SPEC("HW_DEC"			, 3, 3, 3),
	WD_SPEC("AMPDU"				, 3, 4, 4),
	WD_SPEC("AMPDU_END_PKT"		, 3, 5, 5),
	WD_SPEC("AMSDU"				, 3, 6, 6),
	WD_SPEC("AMSDU_CUT"			, 3, 7, 7),
	WD_SPEC("LAST_MSDU"			, 3, 8, 8),
	WD_SPEC("BYPASS"			, 3, 9, 9),
	WD_SPEC("CRC32" 			, 3, 10, 10),
	WD_SPEC("MAGIC_WAKE" 		, 3, 11, 11),
	WD_SPEC("UNICAST_WAKE"		, 3, 12, 12),
	WD_SPEC("PATTERN_WAKE"		, 3, 13, 13),
	WD_SPEC("GET_CH_INFO" 		, 3, 14, 14),
	WD_SPEC("RX_STATISTICS" 	, 3, 15, 15),
	WD_SPEC("PATTERN_IDX"		, 3, 16, 20),
	WD_SPEC("TARGET_IDC"		, 3, 21, 23),
	WD_SPEC("CHKSUM_OFFLOAD_EN"	, 3, 24, 24),
	WD_SPEC("WITH_LLC"			, 3, 25, 25),
	WD_SPEC("RSVD" 				, 3, 26, 31),

	WD_SPEC("TYPE"			, 4, 0, 1),
	WD_SPEC("MC"			, 4, 2, 2),
	WD_SPEC("BC" 			, 4, 3, 3),
	WD_SPEC("MD"			, 4, 4, 4),
	WD_SPEC("MF" 			, 4, 5, 5),
	WD_SPEC("PWR"			, 4, 6, 6),
	WD_SPEC("QOS" 			, 4, 7, 7),
	WD_SPEC("TID"			, 4, 8, 11),
	WD_SPEC("EOSP" 			, 4, 12, 12),
	WD_SPEC("HTC"			, 4, 13, 13),
	WD_SPEC("QNULL"			, 4, 14, 14),
	WD_SPEC("RSVD" 			, 4, 15, 15),
	WD_SPEC("SEQ"			, 4, 16, 27),
	WD_SPEC("FRAG" 			, 4, 28, 31),

	WD_SPEC("SEC_CAM_IDX"		, 5, 0, 7),
	WD_SPEC("ADDR_CAM"			, 5, 8, 15),
	WD_SPEC("MAC_ID"			, 5, 16, 23),
	WD_SPEC("RX_PL_ID"			, 5, 24, 27),
	WD_SPEC("ADDR_CAM_VLD"		, 5, 28, 28),
	WD_SPEC("ADDR_FWD_EN"		, 5, 29, 29),
	WD_SPEC("RX_PL_MATCH"		, 5, 30, 30),
	WD_SPEC("RSVD"				, 5, 31, 31),
};

#ifdef CONFIG_RTW_DEBUG_CAM
static struct parse_wd parse_cmac_tbl_8852ae_full[] = {
	WD_SPEC("DATARATE"				, 0, 0, 8),
	WD_SPEC("FORCE_TXOP"			, 0, 9, 9),
	WD_SPEC("DATA_BW"				, 0, 10, 11),
	WD_SPEC("DATA_GI_LTF"			, 0, 12, 14),
	WD_SPEC("DARF_TC index"			, 0, 15, 15),
	WD_SPEC("ARFR_CTRL"				, 0, 16, 19),
	WD_SPEC("ACQ_RPT_EN"			, 0, 20, 20),
	WD_SPEC("MGQ_RPT_EN"			, 0, 21, 21),
	WD_SPEC("ULQ_RPT_EN"			, 0, 22, 22),
	WD_SPEC("TWTQ_RPT_EN"			, 0, 23, 23),
	WD_SPEC("RSVD"					, 0, 24, 24),
	WD_SPEC("DISRTSFB"				, 0, 25, 25),
	WD_SPEC("DISDATAFB"				, 0, 26, 26),
	WD_SPEC("TRYRATE"				, 0, 27, 27),
	WD_SPEC("AMPDU density"			, 0, 28, 31),

	WD_SPEC("DATA_RTY_LOWEST_RATE"		, 1, 0, 8),
	WD_SPEC("AMPDU_TIME_SEL"			, 1, 9, 9),
	WD_SPEC("AMPDU_LEN_SEL"				, 1, 10, 10),
	WD_SPEC("RTS_TXCNT_LMT_SEL"			, 1, 11, 11),
	WD_SPEC("RTS_TXCNT_LMT"				, 1, 12, 15),
	WD_SPEC("RTS_RATE"					, 1, 16, 24),
	WD_SPEC("RSVD"						, 1, 25, 26),
	WD_SPEC("VCS_STBC"					, 1, 27, 27),
	WD_SPEC("RTS_RTY_LOWEST_RATE"		, 1, 28, 31),

	WD_SPEC("DATA_TXCNT_LMT"		, 2, 0, 5),
	WD_SPEC("DATA_TXCNT_LMT_SEL"	, 2, 6, 6),
	WD_SPEC("MAX_AGG_NUM_SEL" 		, 2, 7, 7),
	WD_SPEC("RTS_EN" 				, 2, 8, 8),
	WD_SPEC("CTS2Self" 				, 2, 9, 9),
	WD_SPEC("CCA_RTS"				, 2, 10, 11),
	WD_SPEC("HW_RTS_EN"				, 2, 12, 12),
	WD_SPEC("RTS_DROP_DATA_MODE"	, 2, 13, 14),
	WD_SPEC("RSVD"					, 2, 15, 15),
	WD_SPEC("AMPDU_MAX_LEN"			, 2, 16, 26),
	WD_SPEC("UL_MU_DIS"				, 2, 27, 27),
	WD_SPEC("AMPDU_MAX_TIME" 		, 2, 28, 31),

	WD_SPEC("MAX_AGG_NUM"		, 3, 0, 7),
	WD_SPEC("BA_BMAP"			, 3, 8, 9),
	WD_SPEC("RSVD"				, 3, 10, 15),
	WD_SPEC("VO_LFTIME_SEL"		, 3, 16, 18),
	WD_SPEC("VI_LFTIME_SEL"		, 3, 19, 21),
	WD_SPEC("BE_LFTIME_SEL"		, 3, 22, 24),
	WD_SPEC("BK_LFTIME_SEL"		, 3, 25, 27),
	WD_SPEC("SECTYPE"			, 3, 28, 31),

	WD_SPEC("multi-port ID"		, 4, 0, 2),
	WD_SPEC("BMC"				, 4, 3, 3),
	WD_SPEC("mbssid"			, 4, 4, 7),
	WD_SPEC("NAVUSEHDR"			, 4, 8, 8),
	WD_SPEC("TXPWR_MODE"		, 4, 9, 11),
	WD_SPEC("DATA_DCM"			, 4, 12, 12),
	WD_SPEC("DATA_ER"			, 4, 13, 13),
	WD_SPEC("DATA_LDPC"			, 4, 14, 14),
	WD_SPEC("DATA_STBC"			, 4, 15, 15),
	WD_SPEC("A_CTRL_BQR"		, 4, 16, 16),
	WD_SPEC("A_CTRL_UPH"		, 4, 17, 17),
	WD_SPEC("A_CTRL_BSR"		, 4, 18, 18),
	WD_SPEC("A_CTRL_CAS"		, 4, 19, 19),
	WD_SPEC("DATA_BW_ER"		, 4, 20, 20),
	WD_SPEC("LSIG_TXOP_EN"		, 4, 21, 21),
	WD_SPEC("RSVD"				, 4, 22, 26),
	WD_SPEC("CTRL_CNT_VLD"		, 4, 27, 27),
	WD_SPEC("CTRL_CNT"			, 4, 28, 31),

 	WD_SPEC("RESP_REF_RATE"			, 5, 0, 8),
	WD_SPEC("RSVD"					, 5, 9, 11),
	WD_SPEC("All_Ack_Support"		, 5, 12, 12),
	WD_SPEC("BSR_QUEUE_SIZE_FORMAT"	, 5, 13, 13),
	WD_SPEC("BSR_OM_UPD_EN"			, 5, 14, 14),
	WD_SPEC("RSVD"					, 5, 15, 15),
	WD_SPEC("NTX_PATH_EN"			, 5, 16, 19),
	WD_SPEC("PATH_MAP_A"			, 5, 20, 21),
	WD_SPEC("PATH_MAP_B"			, 5, 22, 23),
	WD_SPEC("PATH_MAP_C"			, 5, 24, 25),
	WD_SPEC("PATH_MAP_C"			, 5, 26, 27),
	WD_SPEC("ANTSEL_A"				, 5, 28, 28),
	WD_SPEC("ANTSEL_B"				, 5, 29, 29),
	WD_SPEC("ANTSEL_C"				, 5, 30, 30),
	WD_SPEC("ANTSEL_D"				, 5, 31, 31),

	WD_SPEC("ADDR_CAM_INDEX"		, 6, 0, 7),
	WD_SPEC("PAID"					, 6, 8, 16),
	WD_SPEC("ULDL"					, 6, 17, 17),
	WD_SPEC("Doppler_CTRL"			, 6, 18, 19),
	WD_SPEC("Nominal_PKT_Padding"	, 6, 20, 21),
	WD_SPEC("Nominal_PKT_Padding40"	, 6, 22, 23),
	WD_SPEC("txpwr_tolerence"		, 6, 24, 27),
	WD_SPEC("RSVD"					, 6, 28, 29),
	WD_SPEC("Nominal_PKT_Padding80"	, 6, 30, 31),

	WD_SPEC("Nc"						, 7, 0, 2),
	WD_SPEC("Nr"						, 7, 3, 5),
	WD_SPEC("Ng"						, 7, 6, 7),
	WD_SPEC("CB"						, 7, 8, 9),
	WD_SPEC("CS"						, 7, 10, 11),
	WD_SPEC("CSI_TXBF_EN"				, 7, 12, 12),
	WD_SPEC("CSI_STBC_EN"				, 7, 13, 13),
	WD_SPEC("CSI_LDPC_EN"				, 7, 14, 14),
	WD_SPEC("CSI_PARA_EN"				, 7, 15, 15),
	WD_SPEC("CSI_FIX_RATE"				, 7, 16, 24),
	WD_SPEC("CSI_GI_LTF"				, 7, 25, 27),
	WD_SPEC("Nominal_PKT_Padding160"	, 7, 28, 29),
	WD_SPEC("CSI_BW"					, 7, 30, 31),
};
#endif /* CONFIG_RTW_DEBUG_CAM */

enum WD_TYPE {
	TXWD_INFO = 0,
	TXWD_INFO_BODY,
	RXWD,

};

u32 get_txdesc_element_val(u32 val_dw, u8 bit_start, u8 bit_end)
{
	u32 mask = 0;
	u32 i = 0;

	if(bit_start>31
		|| bit_end>31
		|| (bit_start>bit_end)){
		printk("[%s] error %d %d\n", __FUNCTION__, bit_start, bit_end);
		return 0;
	}

	for(i = bit_start; i<=bit_end; i++){
		mask |= (1<<i);
	}

	return ((val_dw & mask)>>bit_start);
}


void parse_wd_8852ae(_adapter *adapter, u32 type, u32 idx_wd, u8 *wd)
{
	u32 i, val = 0;
	u32 cur_dw = 0xFF;
	u32 idx, val_dw = 0;
	u32 array_size = 0;
	struct parse_wd *parser = NULL;

	if(wd==NULL)
		return;

	if(type == TXWD_INFO_BODY){
		parser = parse_txwd_8852ae_full;
		array_size = ARRAY_SIZE(parse_txwd_8852ae_full);
	}
	else if(type == RXWD){
		parser = parse_rxwd_8852ae;
		array_size = ARRAY_SIZE(parse_rxwd_8852ae);
	}

	for(i = 0; i<array_size; i++){
			if(cur_dw != parser[i].idx_dw){
				cur_dw = parser[i].idx_dw;
				idx = (parser[i].idx_dw*4);
				val_dw = wd[idx] + (wd[idx+1]<<8) + (wd[idx+2]<<16) + (wd[idx+3]<<24);
				printk(">>>> WD[%03d].dw%02d = 0x%08x \n", idx_wd, cur_dw, val_dw);
			}

			val = get_txdesc_element_val(val_dw,
				parser[i].bit_start, parser[i].bit_end);

			printk("%s[%d:%d] = (0x%x)\n",
				parser[i].name,
				parser[i].bit_end, parser[i].bit_start, val);
		}
		printk("\n");

}


void compare_wd_8852ae(_adapter *adapter, u32 type, u8 *wd1, u8 *wd2)
{
	u32 i, val1, val2 = 0;
	u32 cur_dw = 0xFF;
	u32 idx, val_dw1 = 0, val_dw2 = 0;
	u32 array_size = 0;
	struct parse_wd *parser = NULL;

	if((wd1==NULL) ||(wd2==NULL))
		return;

	if(type == TXWD_INFO_BODY){
		parser = parse_txwd_8852ae_full;
		array_size = ARRAY_SIZE(parse_txwd_8852ae_full);
	}

	for(i = 0; i<array_size; i++){
			if(cur_dw != parser[i].idx_dw){
				cur_dw = parser[i].idx_dw;
				idx = (parser[i].idx_dw*4);
				val_dw1 = wd1[idx] + (wd1[idx+1]<<8) + (wd1[idx+2]<<16) + (wd1[idx+3]<<24);
				val_dw2 = wd2[idx] + (wd2[idx+1]<<8) + (wd2[idx+2]<<16) + (wd2[idx+3]<<24);
			}

			val1 = get_txdesc_element_val(val_dw1,
				parser[i].bit_start, parser[i].bit_end);
			val2 = get_txdesc_element_val(val_dw2,
				parser[i].bit_start, parser[i].bit_end);

			if(val1 != val2){
				printk("Diff dw%02d: %s[%d:%d] = (0x%x) vs (0x%x)\n", cur_dw,
					parser[i].name,
					parser[i].bit_end, parser[i].bit_start, val1, val2);
			}
		}
		printk("\n");

}

u8 get_cmac_8852ae(_adapter *adapter, u32 macid, char *target_attr, u32 *oval)
{
	u32 i;
	u32 cur_dw = 0xFF;
	u32 idx, val_dw = 0;
	u32 array_size = 0;
	struct parse_wd *parser = NULL;
	void *phl = adapter->dvobj->phl;
	u8 wd[100];
	u8 res = _SUCCESS;

	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// 74b8420fe3902923e3a50f668c20b0b3c08beacd freddie.ho
	rtl_phl_dump_cam(phl, RTW_CAM_CMAC_TBL, macid, wd);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	parser = parse_cmac_tbl_8852ae_full;
	array_size = ARRAY_SIZE(parse_cmac_tbl_8852ae_full);

	for (i = 0; i < array_size; i++) {
		if (!strcmp(target_attr, parser[i].name)) {
			if (cur_dw != parser[i].idx_dw) {
				cur_dw = parser[i].idx_dw;
				idx = (parser[i].idx_dw * 4);
				val_dw = wd[idx] + (wd[idx + 1] << 8) + (wd[idx + 2] << 16) + (wd[idx + 3] << 24);
			}

			*oval = get_txdesc_element_val(val_dw,
				parser[i].bit_start, parser[i].bit_end);

			return _SUCCESS;
		}
	}
	return _FAIL;
}

void get_cmac_many_8852ae(_adapter *adapter, u32 macid, char **target_attr, u32 *oval)
{
	u32 i;
	u32 cur_dw = 0xFF;
	u32 idx, val_dw = 0;
	u32 array_size = 0;
	struct parse_wd *parser = NULL;
	void *phl = adapter->dvobj->phl;
	u8 wd[100];
	u8 res = _SUCCESS;

	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// 74b8420fe3902923e3a50f668c20b0b3c08beacd freddie.ho
	rtl_phl_dump_cam(phl, RTW_CAM_CMAC_TBL, macid, wd);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	parser = parse_cmac_tbl_8852ae_full;
	array_size = ARRAY_SIZE(parse_cmac_tbl_8852ae_full);

	while (*target_attr) {
		for (i = 0; i < array_size; i++){
			if (!strcmp(*target_attr, parser[i].name)) {
				idx = (parser[i].idx_dw*4);
				val_dw = wd[idx] + (wd[idx+1]<<8) + (wd[idx+2]<<16) + (wd[idx+3]<<24);

				*oval = get_txdesc_element_val(val_dw,
					parser[i].bit_start, parser[i].bit_end);
				break;
			}
		}
		target_attr++;
		oval++;
	}
}


void core_dump_map_tx(_adapter *adapter)
{
	struct core_logs *log = &adapter->core_logs;
	u32 idx = 0;

 	printk("drvTx MAP");
	if(core_record_dump_idx & (REC_TX_MGMT|REC_TX_DATA))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct core_record *record = &log->drvTx[idx];
		if(idx >= log->txCnt_all)
			break;
		printk("[drvTx %03d]\n", idx);
		printk("type=%d totalSz=%d virtAddr=%p\n", record->type, record->totalSz, record->virtAddr[0]);
	}

	printk("========= \n\n");
	printk("phlTx MAP");
	if(core_record_dump_idx & (REC_TX_PHL))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct core_record *record = &log->phlTx[idx];
		u32 idx1 = 0;
		if(idx >= log->txCnt_phl)
			break;
		printk("[phlTx %03d]\n", idx);
		printk("type=%d totalSz=%d fragNum=%d\n", record->type, record->totalSz, record->fragNum);
		for(idx1=0; idx1<record->fragNum; idx1++){
			printk("frag#%d: len=%d virtaddr=%p \n", idx1,
				record->fragLen[idx1], record->virtAddr[idx1]);
			printk("frag#%d: phyAddrH=%08X phyAddrL=%08X \n", idx1,
				record->phyAddrH[idx1], record->phyAddrL[idx1]);
		}
	}

	printk("========= \n\n");
	printk("TxRcycle MAP");
	if(core_record_dump_idx & (REC_TX_PHL_RCC))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct core_record *record = &log->txRcycle[idx];
		u32 idx1 = 0;
		if(idx >= log->txCnt_recycle)
			break;
		printk("[TxRcycle %03d]\n", idx);
		printk("type=%d totalSz=%d fragNum=%d\n", record->type, record->totalSz, record->fragNum);
		for(idx1=0; idx1<record->fragNum; idx1++){
			printk("frag#%d: len=%d virtaddr=%p \n", idx1,
				record->fragLen[idx1], record->virtAddr[idx1]);
			printk("frag#%d: phyAddrH=%08X phyAddrL=%08X \n", idx1,
				record->phyAddrH[idx1], record->phyAddrL[idx1]);
		}
	}
}

void core_dump_map_rx(_adapter *adapter)
{
	struct core_logs *log = &adapter->core_logs;
	u32 idx = 0;

 	printk("drvRx MAP");
	if(core_record_dump_idx & (REC_RX_MGMT|REC_RX_DATA))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct core_record *record = &log->drvRx[idx];
		if(idx >= log->rxCnt_all)
			break;
		printk("[drvRx %03d]\n", idx);
		printk("type=%d totalSz=%d virtAddr=%p\n", record->type, record->totalSz, record->virtAddr[0]);
		printk("wl_seq=%d wl_type=0x%x wl_subtype=0x%x\n", record->wl_seq, record->wl_type, record->wl_subtype);
	}

	printk("========= \n\n");
	printk("phlRx MAP");
	if(core_record_dump_idx & (REC_RX_PHL))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct core_record *record = &log->phlRx[idx];
		if(idx >= log->rxCnt_phl)
			break;
		printk("[phlRx %03d]\n", idx);
		printk("type=%d totalSz=%d virtAddr=%p\n", record->type, record->totalSz, record->virtAddr[0]);
	}

	printk("========= \n\n");
	printk("rxRcycle MAP");
	if(core_record_dump_idx & (REC_RX_PHL_RCC))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct core_record *record = &log->rxRcycle[idx];
		if(idx >= log->rxCnt_recycle)
			break;
		printk("[rxRcycle %03d]\n", idx);
		printk("type=%d totalSz=%d virtAddr=%p\n", record->type, record->totalSz, record->virtAddr[0]);
	}

	printk("rxPerInd MAP");
	if(core_record_dump_idx & (REC_RX_PER_IND))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct core_record *record = &log->rxPerInd[idx];
		if(idx >= log->rxCnt_coreInd)
			break;
		printk("[rxPerInd %03d] totalSz=%d \n", idx, record->totalSz);
	}

}


void core_dump_record(_adapter *adapter, u8 dump_type, void *m)
{
	struct core_logs *log = &adapter->core_logs;
	u32 idx = 0;

	RTW_PRINT_SEL(m, "record_enable=%d \n", adapter->record_enable); //eric-asic
	RTW_PRINT_SEL(m, "txCnt_all: %d (%d) \n", log->txCnt_all, log->txCnt_all%CORE_LOG_NUM);
	RTW_PRINT_SEL(m, "txCnt_data: %d \n", log->txCnt_data);
	RTW_PRINT_SEL(m, "txCnt_mgmt: %d \n", log->txCnt_mgmt);
	RTW_PRINT_SEL(m, "txCnt_phl: %d (%d), Sz=%d \n", log->txCnt_phl, log->txCnt_phl%CORE_LOG_NUM, log->txSize_phl);
	RTW_PRINT_SEL(m, "txCnt_recycle: %d (%d), Sz=%d \n", log->txCnt_recycle, log->txCnt_recycle%CORE_LOG_NUM, log->txSize_recycle);

	RTW_PRINT_SEL(m, "rxCnt_all: %d (%d) \n", log->rxCnt_all, log->rxCnt_all%CORE_LOG_NUM);
	RTW_PRINT_SEL(m, "rxCnt_mgmt: %d, rxCnt_data: %d (retry=%d)\n", log->rxCnt_mgmt, log->rxCnt_data, log->rxCnt_data_retry);

#ifdef CONFIG_RTW_CORE_RXSC
	RTW_PRINT_SEL(m, "enable_rxsc: %d \n", adapter->enable_rxsc);
	RTW_PRINT_SEL(m, "rxCnt_data: orig=%d shortcut=%d(ratio=%d)\n",
		log->rxCnt_data_orig, log->rxCnt_data_shortcut,
		log->rxCnt_data_shortcut/(((log->rxCnt_data_orig+log->rxCnt_data_shortcut)/100)?:1));
#ifdef CORE_RXSC_AMSDU
	RTW_PRINT_SEL(m, "rxCnt_amsdu: orig=%d shortcut=%d(ratio=%d) \n",
		log->rxCnt_amsdu_orig, log->rxCnt_amsdu_shortcut,
		log->rxCnt_amsdu_shortcut/(((log->rxCnt_amsdu_orig+log->rxCnt_amsdu_shortcut)/100)?:1));
#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
	RTW_PRINT_SEL(m, "rxCnt_deamsdu_bypass: [0]=%d(skip) [1]=%d(fwdchg) [2]=%d(deamsdu) [3]=%d(mixed)\n",
		log->rxCnt_deamsdu_bypass[0], log->rxCnt_deamsdu_bypass[1],
		log->rxCnt_deamsdu_bypass[2], log->rxCnt_deamsdu_bypass[3]);
	for(idx = 0; idx < 4; idx++) {
		if(log->rxCnt_deamsdu_bypass_sz[idx] > 0) {
			RTW_PRINT_SEL(m, "rxCnt_deamsdu_bypass(sz:%d): %d\n", idx, log->rxCnt_deamsdu_bypass_sz[idx]);
		}
	}
#endif
#endif
	for(idx = 0; idx < LOG_DEAMSDU_PKTNUM; idx++) {
		if(log->rxCnt_deamsdu_pktnum[idx] > 0)
			RTW_PRINT_SEL(m, "rxCnt_demsdu(num%c%2d): %d\n",
				(idx == (LOG_DEAMSDU_PKTNUM-1))?'>':':', (idx+1), log->rxCnt_deamsdu_pktnum[idx]);
	}
	RTW_PRINT_SEL(m, "rxCnt_coreInd: %d \n", log->rxCnt_coreInd);
	RTW_PRINT_SEL(m, "rxsc_sta_get: get=%d cache=%d \n", log->rxsc_sta_get[1], log->rxsc_sta_get[0]);
	RTW_PRINT_SEL(m, "rxsc_entry_get: miss=%d hit=%d\n", log->rxsc_entry_hit[0], log->rxsc_entry_hit[1]);
	RTW_PRINT_SEL(m, "rxsc_entry_alloc: ampdu=%d amsdu=%d\n", log->rxsc_alloc_entry[0], log->rxsc_alloc_entry[1]);
#ifdef CORE_RXSC_RFRAME
	RTW_PRINT_SEL(m, "rxCnt_prf_reuse: %d \n", log->rxCnt_prf_reuse);
#endif

#endif

	RTW_PRINT_SEL(m, "rxCnt_mgmt: %d \n", log->rxCnt_mgmt);
	RTW_PRINT_SEL(m, "rxCnt_phl: %d (%d), Sz=%d \n", log->rxCnt_phl, log->rxCnt_phl%CORE_LOG_NUM, log->rxSize_phl);
	RTW_PRINT_SEL(m, "rxCnt_recycle: %d (%d), Sz=%d\n", log->rxCnt_recycle, log->rxCnt_recycle%CORE_LOG_NUM, log->rxSize_recycle);
	//core_dump_map_tx(adapter);
	//core_dump_map_rx(adapter);
}

void _core_add_record(_adapter *adapter, u32 type, void *p)
{
	struct core_logs *log = &adapter->core_logs;

	if((type & core_record_add_idx) == 0)
		return;

	if(type == REC_RX_PER_IND){
		u32 idx = log->rxCnt_coreInd%CORE_LOG_NUM;
		struct core_record *record = &(log->rxPerInd[idx]);
		u16 val = *(u16 *)p;
		record->totalSz = val;
	}

	if(type == REC_TX_DATA){
		u32 idx = log->txCnt_all%CORE_LOG_NUM;
		struct core_record *record = &(log->drvTx[idx]);
		struct sk_buff *skb = p;

		log->txCnt_data++;
		record->type = type;
		record->totalSz = skb->len;
		record->virtAddr[0] = skb->data;
	}

	if(type == REC_TX_MGMT){
		u32 idx = log->txCnt_all%CORE_LOG_NUM;
		struct core_record *record = &(log->drvTx[idx]);
		struct xmit_frame *pxframe = p;

		log->txCnt_mgmt++;
		record->type = type;
		record->totalSz = XF_LEN_ORIG;
		record->virtAddr[0] = XF_MGMTBUF;
	}

	if(type == REC_TX_PHL || type == REC_TX_PHL_RCC){
		u32 idx = 0;
		struct core_record *record = NULL;
		struct rtw_xmit_req *txreq = p;

		if(type == REC_TX_PHL){
			idx = log->txCnt_phl%CORE_LOG_NUM;
			record = &(log->phlTx[idx]);
			log->txCnt_phl++;
		}

		if(type == REC_TX_PHL_RCC){
			idx = log->txCnt_recycle%CORE_LOG_NUM;
			record = &(log->txRcycle[idx]);
			log->txCnt_recycle++;
		}

		record->type = type;
		record->totalSz = 0;
		record->fragNum = txreq->pkt_cnt;

		{
			struct rtw_pkt_buf_list *pkt_list =(struct rtw_pkt_buf_list *)txreq->pkt_list;
			u32 idx1 = 0;
			for(idx1=0; idx1<txreq->pkt_cnt; idx1++){
				if(idx1 >= MAX_FRAG){
					printk("!! WARN[%s][%d] type=%d frag>= %d \n",
						__FUNCTION__, __LINE__, type, MAX_FRAG);
					break;
				}
				record->totalSz += pkt_list->length;
				record->fragLen[idx1] = pkt_list->length;
				record->virtAddr[idx1] = pkt_list->vir_addr;
				record->phyAddrL[idx1] = pkt_list->phy_addr_l;
				record->phyAddrH[idx1] = pkt_list->phy_addr_h;
				pkt_list++;
			}
		}

		if(type == REC_TX_PHL)
			log->txSize_phl += record->totalSz;
		else if(type == REC_TX_PHL_RCC)
			log->txSize_recycle += record->totalSz;

	}

	if(type == REC_RX_PHL || type == REC_RX_PHL_RCC){
		u32 idx = 0;
		struct core_record *record = NULL;
		struct rtw_recv_pkt *rx_req = p;
		#ifndef CONFIG_RTW_HW_RX_AMSDU_CUT
		struct rtw_pkt_buf_list *pkt = rx_req->pkt_list;
		#else
		struct rtw_rx_buf_base *rx_buf = rx_req->rx_buf;
		#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

		if(type == REC_RX_PHL){
			idx = log->rxCnt_phl%CORE_LOG_NUM;
			record = &(log->phlRx[idx]);
			log->rxCnt_phl++;
		}

		if(type == REC_RX_PHL_RCC){
			idx = log->rxCnt_recycle%CORE_LOG_NUM;
			record = &(log->rxRcycle[idx]);
			log->rxCnt_recycle++;
		}

		record->type = type;
		#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
		record->totalSz = rx_buf->frame_len;
		record->virtAddr[0] = rx_buf->frame;
		#else
		record->totalSz = pkt->length;
		record->virtAddr[0] = pkt->vir_addr;
		#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT*/
		if(type == REC_RX_PHL)
			log->rxSize_phl += record->totalSz;
		else if(type == REC_RX_PHL_RCC)
			log->rxSize_recycle += record->totalSz;
	}

	if(type == REC_RX_DATA || type == REC_RX_MGMT){
		u32 idx = log->rxCnt_all%CORE_LOG_NUM;
		struct core_record *record = &(log->drvRx[idx]);
		union recv_frame *prframe = p;
		struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;

		if(type == REC_RX_DATA){
			log->rxCnt_data++;
		}

		if(type == REC_RX_MGMT){
			log->rxCnt_mgmt++;
		}

		record->type = type;
		record->totalSz = prframe->u.hdr.len;
		record->virtAddr[0] = prframe->u.hdr.rx_data;

		record->wl_seq = pattrib->seq_num;
		record->wl_type = pattrib->wl_type;
		record->wl_subtype = pattrib->wl_subtype;

	}

	if(type == REC_RX_DATA_RETRY){
		log->rxCnt_data_retry++;
	}

	log->txCnt_all = log->txCnt_mgmt + log->txCnt_data;
	log->rxCnt_all = log->rxCnt_mgmt + log->rxCnt_data;
}


void phl_dump_map_tx(_adapter *adapter)
{
	struct phl_logs *log = &adapter->phl_logs;
	u32 idx = 0;

 	printk("txBd MAP");
	if(phl_reocrd_dump_idx & (REC_TXBD))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct record_txbd *record = &log->txBd[idx];
		if(idx >= log->txCnt_bd)
			break;
		printk("[txBd %03d]\n", idx);
		{
			u8 *tmp=record->bd_buf;
			u32 len = record->bd_len;
			u32 idx1 = 0;
			if(tmp == NULL)
				break;
			for(idx1=0; idx1<len; idx1++){
				if(idx1%8==0) {
					printk("[%03d] %02x %02x %02x %02x %02x %02x %02x %02x \n",
						idx1, tmp[idx1], tmp[idx1+1], tmp[idx1+2], tmp[idx1+3],
						tmp[idx1+4], tmp[idx1+5], tmp[idx1+6], tmp[idx1+7]);
				}
			}
			printk("\n");
		}
	}

	printk("========= \n\n");
	printk("txWd MAP");
	if(phl_reocrd_dump_idx & (REC_TXWD))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct record_txwd *record = &log->txWd[idx];
		if(idx >= log->txCnt_wd)
			break;
		printk("[txWd %03d]\n", idx);
		{
			u8 *tmp=record->wd_buf;
			u32 len = record->wd_len;
			u32 idx1 = 0;
			if(tmp == NULL)
				break;
			for(idx1=0; idx1<len; idx1++){
				if(idx1%8==0) {
					printk("[%03d] %02x %02x %02x %02x %02x %02x %02x %02x \n",
						idx1, tmp[idx1], tmp[idx1+1], tmp[idx1+2], tmp[idx1+3],
						tmp[idx1+4], tmp[idx1+5], tmp[idx1+6], tmp[idx1+7]);
				}
			}
			printk("\n");
		}
		parse_wd_8852ae(adapter, TXWD_INFO_BODY, idx, record->wd_buf);
		//compare_wd_8852ae(adapter, TXWD_INFO_BODY, record->wd_buf, sample_txwd);
	}

	printk("========= \n\n");
 	printk("wpRecycle MAP");
	if(phl_reocrd_dump_idx & (REC_WP_RCC))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct record_wp_rcc *record = &log->wpRecycle[idx];
		if(idx >= log->txCnt_recycle)
			break;
		printk("[txRecycle %03d]\n", idx);
		printk("wp_seq=%d \n", record->wp_seq);
	}
}

void phl_dump_map_rx(_adapter *adapter)
{
	struct phl_logs *log = &adapter->phl_logs;
	u32 idx = 0;

	printk("rxPciMap MAP");
	if(phl_reocrd_dump_idx & (REC_RX_MAP))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct record_pci *record = &log->rxPciMap[idx];
		if(idx >= log->rxCnt_map)
			break;
		printk("[rxPciMap %03d]\n", idx);
		printk("phyAddrL=%p len=%d\n", record->phyAddrL, record->map_len);
	}


	printk("========= \n\n");
	printk("rxPciUnmap MAP");
	if(phl_reocrd_dump_idx & (REC_RX_UNMAP))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct record_pci *record = &log->rxPciUnmap[idx];
		if(idx >= log->rxCnt_unmap)
			break;
		printk("[rxPciUnmap %03d]\n", idx);
		printk("phyAddrL=%p len=%d\n", record->phyAddrL, record->map_len);
	}

	printk("========= \n\n");
	printk("rxWd MAP");
	if(phl_reocrd_dump_idx & (REC_RXWD))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		struct record_rxwd *record = &log->rxWd[idx];
		if(idx >= log->rxCnt_wd)
			break;
		printk("[rxWd %03d]\n", idx);
		{
			u8 *tmp = record->wd_buf;
			u32 len = record->wd_len;
			u32 idx1 = 0;
			if(tmp == NULL)
				break;
			for(idx1=0; idx1<len; idx1++){
				if(idx1%8==0) {
					printk("[%03d] %02x %02x %02x %02x %02x %02x %02x %02x \n",
						idx1, tmp[idx1], tmp[idx1+1], tmp[idx1+2], tmp[idx1+3],
						tmp[idx1+4], tmp[idx1+5], tmp[idx1+6], tmp[idx1+7]);
				}
			}
			printk("\n");
		}
		parse_wd_8852ae(adapter, RXWD, idx, record->wd_buf);
	}

	printk("========= \n\n");
	printk("rxAmpdu MAP");
	if(phl_reocrd_dump_idx & (REC_RX_AMPDU))
	for(idx=0; idx<CORE_LOG_NUM; idx++){
		if(idx >= log->rxCnt_ampdu)
			break;
		printk("[rxAmpdu %03d] = %d\n", idx, log->rxAmpdu[idx]);
	}

}

void phl_dump_record(_adapter *adapter, u8 dump_type)
{
	struct phl_logs *log = &adapter->phl_logs;
	u32 idx = 0;

	printk("txBd: %d (%d) \n", log->txCnt_bd, log->txCnt_bd%CORE_LOG_NUM);
	printk("txWd: %d (%d) \n", log->txCnt_wd, log->txCnt_wd%CORE_LOG_NUM);
	printk("wpCnt_recycle: %d (%d) \n", log->txCnt_recycle, log->txCnt_recycle%CORE_LOG_NUM);

	printk("rxMap: %d (%d), Sz=%d \n", log->rxCnt_map, log->rxCnt_map%CORE_LOG_NUM, log->rxSize_map);
	printk("rxUnmap: %d (%d), Sz=%d \n", log->rxCnt_unmap, log->txCnt_wd%CORE_LOG_NUM, log->rxSize_map);
	printk("rxWd: %d (%d) \n", log->rxCnt_wd, log->rxCnt_wd%CORE_LOG_NUM);
	printk("rxCnt_ampdu: %d (%d) \n", log->rxCnt_ampdu, log->rxCnt_ampdu%CORE_LOG_NUM);

		phl_dump_map_tx(adapter);
		phl_dump_map_rx(adapter);
}

u32 tmp_rx_last_ppdu = 0;

void phl_add_record(void *d, u32 type, void *p, u32 num)
{
	struct dvobj_priv *pobj = (struct dvobj_priv *)d;
	_adapter *adapter = dvobj_get_primary_adapter(pobj);
	struct phl_logs *log = &adapter->phl_logs;

	if(!adapter->record_enable)
		return;

	if((type & phl_record_add_idx) == 0)
		return;

	if(type == REC_TXWD){
		u32 idx = log->txCnt_wd%CORE_LOG_NUM;
		struct record_txwd *record = &(log->txWd[idx]);

		log->txCnt_wd++;
		memset((u8 *)record->wd_buf, 0, MAX_TXWD_SIZE);
		if(num < MAX_TXWD_SIZE)
		{
			memcpy((u8 *)record->wd_buf, p, num);
			record->wd_len = num;
		}
		else
		{
			memcpy((u8 *)record->wd_buf, p, MAX_TXWD_SIZE);
			record->wd_len = MAX_TXWD_SIZE;
			RTW_ERR("In %s(%d), overrunning record->wd_buf\n", __FUNCTION__, __LINE__);
		}
	}

	if(type == REC_TXBD){
		u32 idx = log->txCnt_bd%CORE_LOG_NUM;
		struct record_txbd *record = &(log->txBd[idx]);

		log->txCnt_bd++;
		memset((u8 *)record->bd_buf, 0, MAX_TXBD_SIZE);
		if(num < MAX_TXBD_SIZE)
		{
			memcpy((u8 *)record->bd_buf, p, num);
			record->bd_len = num;
		}
		else
		{
			memcpy((u8 *)record->bd_buf, p, MAX_TXBD_SIZE);
			record->bd_len = MAX_TXBD_SIZE;
			RTW_ERR("In %s(%d), overrunning record->bd_buf\n", __FUNCTION__, __LINE__);
		}
	}

	if(type == REC_WP_RCC){
		u32 idx = log->txCnt_recycle%CORE_LOG_NUM;
		struct record_wp_rcc *record = &(log->wpRecycle[idx]);

		log->txCnt_recycle++;
		record->wp_seq = num;
	}

	if(type == REC_RX_MAP || type == REC_RX_UNMAP){
		struct record_pci *record = NULL;
		if(type == REC_RX_MAP) {
			u32 idx = log->rxCnt_map%CORE_LOG_NUM;
			record = &(log->rxPciMap[idx]);
			log->rxCnt_map++;
			log->rxSize_map+=num;
		}
		else if(type == REC_RX_UNMAP) {
			u32 idx = log->rxCnt_unmap%CORE_LOG_NUM;
			record = &(log->rxPciUnmap[idx]);
			log->rxCnt_unmap++;
			log->rxSize_map+=num;
		}
		record->phyAddrL = p;
		record->map_len = num;
	}

	if(type == REC_RXWD){
		u32 idx = log->rxCnt_wd%CORE_LOG_NUM;
		struct record_rxwd *record = &(log->rxWd[idx]);

		log->rxCnt_wd++;
		memset((u8 *)record->wd_buf, 0, MAX_RXWD_SIZE);
		if(num < MAX_RXWD_SIZE)
		{
			memcpy((u8 *)record->wd_buf, p, num);
			record->wd_len = num;
		}
		else
		{
			memcpy((u8 *)record->wd_buf, p, MAX_RXWD_SIZE);
			record->wd_len = MAX_RXWD_SIZE;
			RTW_ERR("In %s(%d), overrunning record->wd_buf\n", __FUNCTION__, __LINE__);
		}
	}

	if(type == REC_RX_AMPDU){
		u32 idx = 0;

		if(log->rxCnt_ampdu == 0 && (log->rxAmpdu[0] == 0))
			tmp_rx_last_ppdu = num;

		if(tmp_rx_last_ppdu != num){
			tmp_rx_last_ppdu = num;

			log->rxCnt_ampdu ++;
			idx = log->rxCnt_ampdu%CORE_LOG_NUM;
			log->rxAmpdu[idx] = 1;
		} else {
			idx = log->rxCnt_ampdu%CORE_LOG_NUM;
			log->rxAmpdu[idx]++;
	}
}
}

void core_cmd_record_trx(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	char *para = (char *)cmd_para;

	if(para_num<=0)
		return;

	if(!strcmp(para, "start")){
		u8 *log = NULL;
		log = (u8*)&adapter->core_logs;
		memset(log, 0, sizeof(struct core_logs));
		log = (u8*)&adapter->phl_logs;
		memset(log, 0, sizeof(struct phl_logs));
		adapter->record_enable = 1;
	}else if(!strcmp(para, "stop")){
		adapter->record_enable = 0;
	}else if(!strcmp(para, "dump")){
		u32 dump_type = 0;
		para=get_next_para_str(para);
		sscanf(para, "%d", &dump_type);
		core_dump_record(adapter, (u8)dump_type, RTW_DBGDUMP);
		//phl_dump_record(adapter, (u8)dump_type);
	}
}

#define MAX_TRACK_REG_NUM	10
u32 track_reg_list[MAX_TRACK_REG_NUM];
u8 track_reg_enable = 0;
u8 track_reg_num = 0;
u8 track_reg_inited = 0;
_timer timer_track_reg;

#define TRACK_BB_RESET		0xffff0
#define TRACK_BB_CNT		0xffff1

u32 tmp_cca=0;
u32 tmp_crc_ok[4];
u32 tmp_crc_err[4];

void core_cmd_dump_track_reg(_adapter *adapter)
{
	u32 idx = 0;

	if(track_reg_enable==0)
		return;

	for(idx=0; idx<track_reg_num; idx++){
		printk("0x%x=0x%x\n", track_reg_list[idx], rtw_phl_read32(adapter->dvobj->phl, track_reg_list[idx]));
	}
		printk("\n");

track_timer:
	_set_timer(&timer_track_reg, 1000);
}

void core_cmd_track_reg(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	u32 *para = (u32 *)cmd_para;
	DBGP("para_num=%d\n", para_num);

	if(track_reg_enable == 0)
		track_reg_enable = 1;
	else {
		track_reg_enable = 0;
		track_reg_num = 0;
		return;
	}

	if(para_num < 1 || para_num > 10)
		return;
	else
		track_reg_num = para_num;

	for(idx=0; idx<para_num; idx++){
		track_reg_list[idx] = para[idx];
		DBGP("track_reg_list[%d]=0x%x\n", idx, track_reg_list[idx]);
	}

	if(track_reg_inited == 0){
		track_reg_inited = 1;
		rtw_init_timer(&timer_track_reg, core_cmd_dump_track_reg, adapter);
	}

	_set_timer(&timer_track_reg, 1000);
}

#define MAX_TRACK_CMD_NUM	10
u32 track_cmd_list[MAX_TRACK_CMD_NUM][10];
u8 track_cmd_enable = 0;
u8 track_cmd_num = 0;
u8 track_cmd_inited = 0;
_timer timer_track_cmd;

void core_cmd_dump_track_cmd(_adapter *adapter)
{
	u32 idx = 0;

	if(track_cmd_enable==0)
		return;

	for(idx=0; idx<track_cmd_num; idx++) {
		char *cmd = (char *)&track_cmd_list[idx];
		if(!strcmp(cmd, "bb_reset")) {
			DBGP("bb_reset \n");
			rtw_phl_write16(adapter->dvobj->phl, 0x10704, 0);
			mdelay(50);
			rtw_phl_write16(adapter->dvobj->phl, 0x10704, 0x1FF);
		}
		else if(!strcmp(cmd, "bb_cnt")) {
			u16 cur_cca = rtw_phl_read16(adapter->dvobj->phl, 0x14124);
			u32 cur_crc = 0;
			u16 cur_crc_ok, cur_crc_err = 0;
			u16 total_crc_ok = 0, total_crc_err = 0;
			u32 crc_reg[4] = {0x14164, 0x14160, 0x1415c, 0x14158};
			u8 idx1 = 0;

			printk("cca = 0x%x-0x%x = %d \n", cur_cca, tmp_cca, (cur_cca-tmp_cca));
			tmp_cca = cur_cca;

			for(idx1=0; idx1<4; idx1++){
				cur_crc = rtw_phl_read32(adapter->dvobj->phl, crc_reg[idx1]);
				cur_crc_ok = cur_crc & 0xffff;
				cur_crc_err = (cur_crc & 0xffff0000) >> 16;
				printk("crc_ok [%d] = 0x%x-0x%x = %d \n", idx1, cur_crc_ok, tmp_crc_ok[idx1], (cur_crc_ok-tmp_crc_ok[idx1]));
				printk("crc_err[%d] = 0x%x-0x%x = %d \n", idx1, cur_crc_err, tmp_crc_err[idx1], (cur_crc_err-tmp_crc_err[idx1]));
				tmp_crc_ok[idx1] = cur_crc_ok;
				tmp_crc_err[idx1] = cur_crc_err;
				total_crc_ok+=cur_crc_ok;
				total_crc_err+=cur_crc_err;
			}
			printk("total_crc = %d + %d = %d \n\n", total_crc_ok, total_crc_err, (total_crc_ok+total_crc_err));
		}
#ifdef DEBUG_PHL_RX
		else if(!strcmp(cmd, "phl")) {
			printk("core rx: mgmt=%d data=%d(retry=%d, %d) \n",
				adapter->cnt_core_rx_mgmt, adapter->cnt_core_rx_data,
				adapter->cnt_core_rx_data_retry, (adapter->cnt_core_rx_data_retry*100)/adapter->cnt_core_rx_data);
			rtw_phl_cmd_phl_rx_dump(adapter, RTW_DBGDUMP);
			printk("\n\n");
		}
#endif
#ifdef DEBUG_RFK
		else if(!strcmp(cmd, "rkk")) {
			rtw_phl_rf_chl_rfk_trigger(adapter->dvobj->phl, 0);
		}
		else if(!strcmp(cmd, "iqk")) {
			rtw_phl_rf_iqk_trigger(adapter->dvobj->phl, 0);
		}
#endif
	}

track_timer:
	_set_timer(&timer_track_cmd, 1000);
}

#ifdef DEBUG_PHL_RX

#ifdef CONFIG_VW_REFINE
extern enum rtw_phl_status rtw_phl_cmd_debug_wd_release(void *phl, u32 value);
extern void rtw_phl_cmd_dump(void *phl, u32 value);
extern void rtw_phl_cmd_wd_info(void *phl, u32 value);
extern void rtw_phl_show_vw_cnt(void *phl, u32 value);

void core_show_phl_vw_cnt(_adapter *adapter, u32 value)
{
        rtw_phl_show_vw_cnt(adapter->dvobj->phl, value);
}

void core_cmd_debug_dump(_adapter *adapter, void *cmd_para, u32 para_num)
{
        u32 value;
        char *para = (char *)cmd_para;

        para = get_next_para_str(para);
        sscanf(para, "%d", &value);

        rtw_phl_cmd_dump(adapter->dvobj->phl, value);
}

void core_cmd_debug_wd(_adapter *adapter, void *cmd_para, u32 para_num)
{
        u32 value;
        char *para = (char *)cmd_para;

        para = get_next_para_str(para);
        sscanf(para, "%d", &value);

        rtw_phl_cmd_wd_info(adapter->dvobj->phl, value);
}

#ifdef CONFIG_RTW_DBG_TX_MGNT
void core_cmd_debug_tx_mgnt(_adapter *adapter, void *cmd_para, u32 para_num)
{
        u32 value;
        char *para = (char *)cmd_para;

        para = get_next_para_str(para);
        sscanf(para, "%d", &value);

        adapter->dbg_tx_mgnt = value;
        RTW_PRINT(ADPT_FMT" %s TX MGNT debug.\n", ADPT_ARG(adapter),
                  adapter->dbg_tx_mgnt ? "enable" : "disable");
}
#endif /* CONFIG_RTW_DBG_TX_MGNT */

void core_cmd_debug_phl_wd_release(_adapter *adapter, void *cmd_para, u32 para_num)
{
        u32 value;
        char *para = (char *)cmd_para;

        para = get_next_para_str(para);
        sscanf(para, "%d", &value);

        rtw_phl_cmd_debug_wd_release(adapter->dvobj->phl, value);
}
#endif

void rtw_phl_cmd_phl_rx_dump(_adapter *adapter, void* m)
{
	struct rtw_proc_cmd cmd;
	char *out_buf;
	u32 out_len = 1024;

	out_buf = rtw_malloc(out_len);
	if (!out_buf) {
		printk("rtw_malloc(%d) failed!!\n", out_len);
		return;
	}

	cmd.in_type = RTW_ARG_TYPE_ARRAY;
	cmd.in_cnt_len = 1;
	strcpy(cmd.in.vector[0], "phl_rx");
	rtw_phl_proc_cmd(GET_HAL_INFO(adapter_to_dvobj(adapter)),
			RTW_PROC_CMD_PHL, &cmd, out_buf, out_len);
#if defined(_seqdump)
	if (m)
		_seqdump(m, "%s", out_buf);
	else
#endif
		_dbgdump("%s", out_buf);

	rtw_mfree(out_buf, out_len);
}

void rtw_phl_cmd_phl_rx_clear(_adapter *adapter)
{
	struct rtw_proc_cmd cmd;
	char *out_buf;
	u32 out_len = 256;

	out_buf = rtw_vmalloc(out_len);
	if (!out_buf) {
		printk("_rtw_vmalloc(%d) failed!!\n", out_len);
		return;
	}

	cmd.in_type = RTW_ARG_TYPE_ARRAY;
	cmd.in_cnt_len = 2;
	strcpy(cmd.in.vector[0], "phl_rx");
	strcpy(cmd.in.vector[1], "clear");
	rtw_phl_proc_cmd(GET_HAL_INFO(adapter_to_dvobj(adapter)),
			RTW_PROC_CMD_PHL, &cmd, out_buf, out_len);

	rtw_vmfree(out_buf, out_len);
}

void rtw_phl_cmd_phl_rx_debug(_adapter *adapter,
				char *cmd_para, u32 para_num)
{
	struct rtw_proc_cmd cmd;
	char *out_buf;
	u32 out_len = 256;
	int i;

	out_buf = rtw_vmalloc(out_len);
	if (!out_buf) {
		printk("_rtw_vmalloc(%d) failed!!\n", out_len);
		return;
	}

	cmd.in_type = RTW_ARG_TYPE_ARRAY;
	cmd.in_cnt_len = 1;
	strcpy(cmd.in.vector[0], "phl_rx");
	for (i = 0; i < para_num; i++) {
		strncpy(cmd.in.vector[cmd.in_cnt_len], cmd_para, MAX_ARGV-1);
		cmd.in.vector[cmd.in_cnt_len][MAX_ARGV-1] = '\0';
		cmd.in_cnt_len++;
		cmd_para = get_next_para_str(cmd_para);
	}
	rtw_phl_proc_cmd(GET_HAL_INFO(adapter_to_dvobj(adapter)),
			RTW_PROC_CMD_PHL, &cmd, out_buf, out_len);
	printk("%s", out_buf);

	rtw_vmfree(out_buf, out_len);
}

#endif /* DEBUG_PHL_RX */

void core_cmd_track(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	char *para = (char *)cmd_para;
	DBGP("para_num=%d\n", para_num);

	if(track_cmd_enable == 0)
		track_cmd_enable = 1;
	else {
		track_cmd_enable = 0;
		return;
	}

	if(para_num < 1 || para_num > MAX_TRACK_CMD_NUM)
		return;
	else
		track_cmd_num = para_num;

	for(idx=0; idx<para_num; idx++, para+=MAX_PHL_CMD_STR_LEN){
		char *cmd = (char *)&track_cmd_list[idx];
		strncpy(cmd, para, sizeof(*cmd));
		DBGP("track_reg_list[%d]=%s\n", idx, cmd);
	}

	if(track_cmd_inited == 0){
		track_cmd_inited = 1;
		rtw_init_timer(&timer_track_cmd, core_cmd_dump_track_cmd, adapter);
	}

	_set_timer(&timer_track_cmd, 1000);
}

#ifdef CONFIG_VW_REFINE
void core_cmd_swq(_adapter *adapter, void *cmd_para, u32 para_num)
{
        u32 idx = 0, value = 0;
        char *para = (char *)cmd_para;
		struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

        DBGP("\n");

        if( para_num <= 0 )
             return;

        if(!strcmp(para, "max_enq_len")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->max_enq_len = value;
        } else if(!strcmp(para, "max_deq_len")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->max_deq_len = value;
        } else if(!strcmp(para, "sta_deq_len")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->sta_deq_len = value;
        }  else if(!strcmp(para, "ring_lmt")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->ring_lmt = value;
        }  else if(!strcmp(para, "dsr_time")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->xmit_dsr_time = value;
        }  else if(!strcmp(para, "merge")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->amsdu_merge_cnt = value;
        }  else if(!strcmp(para, "tx_amsdu")) {
				para = get_next_para_str(para);
                sscanf(para, "%d", &value);
				adapter->manual_tx_amsdu[0] = value;
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->manual_tx_amsdu[1] = value;
				para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->manual_tx_amsdu[2] = value;
				para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->manual_tx_amsdu[3] = value;

				printk("manual_tx_amsdu_enable:   %d\n", adapter->manual_tx_amsdu[0]);
				printk("manual_tx_amsdu_big:   %d\n", adapter->manual_tx_amsdu[1]);
				printk("manual_tx_amsdu_mid:   %d\n", adapter->manual_tx_amsdu[2]);
				printk("manual_tx_amsdu_small:   %d\n", adapter->manual_tx_amsdu[3]);
        }  else if(!strcmp(para, "tx_lmt")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->tx_lmt = value;
        }  else if(!strcmp(para, "delay")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->delay_test = value;
        }  else if(!strcmp(para, "hw_time")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->set_hw_swq_timeout = value;
        }  else if(!strcmp(para, "swq_time")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->set_swq_timeout = value;
        }  else if(!strcmp(para, "lf_time")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->lf_time = value;
        }  else if(!strcmp(para, "no_wdinfo")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->no_wdinfo = value;
        }  else if(!strcmp(para, "with_bk")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->with_bk = value;
        }  else if(!strcmp(para, "no_rts")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->no_rts = value;
        }  else if(!strcmp(para, "vcs")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->registrypriv.vcs_type = value;
        }  else if(!strcmp(para, "sp_sz")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->spec_pktsz = value;
        }  else if(!strcmp(para, "sn_gap")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->sta_sn_gap = value;
        } else if(!strcmp(para, "direct")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
                adapter->direct_xmit = value;
        } else if(!strcmp(para, "rtsrate")) {
                para = get_next_para_str(para);
                sscanf(para, "%d", &value);
		if (BAND_ON_5G != rtw_get_phyband_on_dev(adapter))
                	adapter->rtsrate_cck = value;
		else
			RTW_PRINT(FUNC_DEV_FMT" should not set rtsrate=CCK1M\n",
				  FUNC_DEV_ARG(adapter->dvobj));
        } else if(!strcmp(para, "debug")) {
        	_queue *pfree_txreq_queue = adapter->pfree_txreq_queue;
            struct dvobj_priv *dvobj = adapter->dvobj;
            u8 i;
            u32 skb_qlen;

            skb_qlen = ATOMIC_READ(&adapter->skb_xmit_queue_len);

            printk("max_skb_que:%d merge:%d sta_deq:%d max_deq:%d\n", \
               adapter->max_enq_len, adapter->amsdu_merge_cnt, adapter->sta_deq_len,  adapter->max_deq_len);

            printk("adapter:%p hw_cnt:%u hw_tick:%u vw_cnt:%u\n", \
                       adapter, adapter->hw_swq_cnt,  adapter->hw_irq_cnt, adapter->vw_pc_cnt);

            printk("free_txq:%d amsdu_cnt:%d swq_cnt:%d\n", pfree_txreq_queue->qlen, adapter->swq_amsdu_cnt, skb_qlen);

            for (i = 0; i < CONFIG_IFACE_NUMBER; i++)
                printk("apt:%i addr=%p \n", i, dvobj->padapters[i]);
        } else if(!strcmp(para, "vw_cnt")) {
                u8 i = 0;
#ifdef CONFIG_DBG_COUNTER
                printk("vw_entry:%d \n", adapter->tx_logs.core_vw_entry);
                printk("vw_swq_enq:%d \n", adapter->tx_logs.core_vw_swq_enq);
                printk("vw_swq_enq_fail:%d \n", adapter->tx_logs.core_vw_swq_enq_fail);
                printk("vw_slow:%d \n", adapter->tx_logs.core_vw_slow);
                printk("vw_txsc_amsdu:%d \n", adapter->tx_logs.core_vw_txsc);
                printk("vw_txsc_amsdu_fail:%d \n", adapter->tx_logs.core_vw_txsc_fail);
                printk("vw_txsc_ampdu:%d \n", adapter->tx_logs.core_vw_txsc2);
                printk("vw_amsdu_enq:%d \n", adapter->tx_logs.core_vw_amsdu_enq);
                printk("vw_amsdu_deq:%d \n", adapter->tx_logs.core_vw_amsdu_dnq);
                printk("vw_amsdu_deq1:%d \n", adapter->tx_logs.core_vw_amsdu_dnq1);
                printk("vw_amsdu_deq2:%d \n", adapter->tx_logs.core_vw_amsdu_dnq2);
                printk("vw_amsdu_enq_merg:%d \n", adapter->tx_logs.core_vw_amsdu_enq_merg);
                printk("core_vw_amsdu_dir:%d \n", adapter->tx_logs.core_vw_amsdu_dir);
                printk("vw_amsdu_timeout:%d \n", adapter->tx_logs.core_vw_amsdu_timeout);
                printk("core_tx_drop:%d\n", adapter->tx_logs.core_tx_err_drop);
                printk("core_tx_ex_drop:%d\n", adapter->tx_logs.core_tx_ex_err_drop);
                printk("vw_add_tx_req:%d \n", adapter->tx_logs.core_vw_add_tx_req);
                core_show_phl_vw_cnt(adapter, 1);

                printk("core_vw_test0-org:%d \n", adapter->tx_logs.core_vw_test0);
                printk("core_vw_test1-org:%d \n", adapter->tx_logs.core_vw_test1);
                printk("core_vw_test2-apply:%d \n", adapter->tx_logs.core_vw_test2);
                printk("core_vw_test3-vw-d:%d \n", adapter->tx_logs.core_vw_test3);
                printk("core_vw_test4-non-vw-d:%d \n", adapter->tx_logs.core_vw_test4);
                printk("core_vw_test5-vw-deq:%d \n", adapter->tx_logs.core_vw_test5);
                printk("core_vw_test6-all-deq:%d \n", adapter->tx_logs.core_vw_test6);
                printk("core_vw_test7-null-d:%d \n", adapter->tx_logs.core_vw_test7);
                printk("core_vw_test8-amsdu_time:%d \n", adapter->tx_logs.core_vw_test8);
                printk("core_vw_test9-0-sbk_cnt:%d \n", adapter->tx_logs.core_vw_test9);

                printk("core_vw_testa-0-sbk_cnt:%d \n", adapter->tx_logs.core_vw_testa);
                printk("core_vw_testb-apply-err1:%d \n", adapter->tx_logs.core_vw_testb);
                printk("core_vw_testc-apply-err2:%d \n", adapter->tx_logs.core_vw_testc);
                printk("core_vw_testd-apply-err3:%d \n", adapter->tx_logs.core_vw_testd);
                printk("core_vw_testd-apply-err4:%d \n", adapter->tx_logs.core_vw_teste);
#endif
                for (i = 0; i < MAX_SKB_XMIT_QUEUE; i++)
                    printk("m-id:%d snd_cnt:%d rec_cnt:%d retry:%d\n", i,
                        adapter->skb_vw_cnt[i], adapter->skb_vw_rec_cnt[i], adapter->vw_retry_cnt[i]);
        } else if(!strcmp(para, "vw_cnt_clear")) {
                u8 i;
                for (i = 0; i < MAX_SKB_XMIT_QUEUE; i++) {
                       adapter->skb_vw_cnt[i] = 0;
                       adapter->skb_vw_rec_cnt[i] = 0;
                }

#ifdef CONFIG_DBG_COUNTER
                adapter->tx_logs.core_vw_entry = 0;
                adapter->tx_logs.core_vw_txsc = 0;
                adapter->tx_logs.core_vw_txsc_fail = 0;
                adapter->tx_logs.core_vw_txsc2 = 0;
                adapter->tx_logs.core_vw_slow = 0;
                adapter->tx_logs.core_vw_swq_enq = 0;
                adapter->tx_logs.core_vw_swq_enq_fail = 0;
                adapter->tx_logs.core_vw_amsdu_enq = 0;
                adapter->tx_logs.core_vw_amsdu_dnq = 0;
                adapter->tx_logs.core_vw_amsdu_dnq1 = 0;
                adapter->tx_logs.core_vw_amsdu_dnq2 = 0;
                adapter->tx_logs.core_vw_amsdu_enq_merg = 0;
                adapter->tx_logs.core_vw_amsdu_dir = 0;
                adapter->tx_logs.core_vw_amsdu_timeout = 0;
                adapter->tx_logs.core_vw_add_tx_req = 0;
                adapter->tx_logs.core_tx_err_drop = 0;
                adapter->tx_logs.core_tx_ex_err_drop = 0;
                core_show_phl_vw_cnt(adapter, 0);

                adapter->tx_logs.core_vw_test0 = 0;
                adapter->tx_logs.core_vw_test1 = 0;
                adapter->tx_logs.core_vw_test2 = 0;
                adapter->tx_logs.core_vw_test3 = 0;
                adapter->tx_logs.core_vw_test4 = 0;
                adapter->tx_logs.core_vw_test5 = 0;
                adapter->tx_logs.core_vw_test6 = 0;
                adapter->tx_logs.core_vw_test7 = 0;
                adapter->tx_logs.core_vw_test8 = 0;
                adapter->tx_logs.core_vw_test9 = 0;

                adapter->tx_logs.core_vw_testa = 0;
                adapter->tx_logs.core_vw_testb = 0;
                adapter->tx_logs.core_vw_testc = 0;
                adapter->tx_logs.core_vw_testd = 0;
                adapter->tx_logs.core_vw_teste = 0;
#endif
        } else if(!strcmp(para, "dump")) {
				printk("max_enq_len: %d\n", adapter->max_enq_len);
				printk("max_deq_len: %d\n", adapter->max_deq_len);
				printk("sta_deq_len: %d\n", adapter->sta_deq_len);
				printk("amsdu_num:   %d\n", adapter->tx_amsdu);
				printk("current tx_amsdu_big:   %d\n", adapter->current_tx_amsdu[0]);
				printk("current tx_amsdu_mid:   %d\n", adapter->current_tx_amsdu[1]);
				printk("current tx_amsdu_small:   %d\n", adapter->current_tx_amsdu[2]);
				printk("ring_lmt:    %d\n", adapter->ring_lmt);
				printk("dsr_time:    %d\n", adapter->xmit_dsr_time);
				printk("merge:       %d\n", adapter->amsdu_merge_cnt);
				printk("tx_lmt:      %d\n", adapter->tx_lmt);
				printk("delay:       %d\n", adapter->delay_test);
				printk("hw_time:     %d\n", adapter->hw_swq_timeout);
				printk("swq_time:    %d\n", adapter->swq_timeout);
				printk("no_wdinfo:   %d\n", adapter->no_wdinfo);
                printk("with_bk:     %d\n", adapter->with_bk);
				printk("no_rts:      %d\n", adapter->no_rts);
				printk("sp_sz:       %d\n", adapter->spec_pktsz);
				printk("sn_gap:      %d\n", adapter->sta_sn_gap);
				printk("direct:      %d\n", adapter->direct_xmit);
                printk("HZ:          %d\n", HZ);
                printk("tx_mode:     %d\n", dvobj->tx_mode);
                printk("vw:          %d\n", adapter->vw_enable);
                printk("phl_1_txring:%d\n", rtw_phl_get_one_txring_mode(adapter->dvobj->phl));
				printk("vcs:         %d\n", adapter->registrypriv.vcs_type);
				printk("rtsrate_cck: %d\n", adapter->rtsrate_cck);
        }
}
#endif

#ifdef CONFIG_ONE_TXQ
void core_cmd_txq(_adapter *adapter, void *cmd_para, u32 para_num)
{
	struct dvobj_priv *dvobj = adapter->dvobj;
	u32 idx = 0, value = 0;
	char *para = (char *)cmd_para;

	DBGP("\n");

	if (para_num <= 0)
		return;

	if (!strcmp(para, "max_enq_len")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_max_enq_len = value;
	} else if (!strcmp(para, "max_agg_num")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_max_agg_num = value;
	} else if (!strcmp(para, "hw_timeout")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_hw_timeout = value;
	} else if (!strcmp(para, "pkt_timeout")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_pkt_timeout = value;
	} else if (!strcmp(para, "max_serv_time")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_max_serv_time = value;
	} else if (!strcmp(para, "amsdu_merge")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_amsdu_merge = value;
	} else if (!strcmp(para, "tcpack_merge")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_tcpack_merge = value;
	} else if (!strcmp(para, "ts_factor")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_ts_factor = value;
	} else if (!strcmp(para, "deq_factor")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_deq_factor = value;
	} else if (!strcmp(para, "deq_loop")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_deq_loop = value;
	} else if (!strcmp(para, "debug")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_debug = value;
	} else if (!strcmp(para, "serv_group_exp")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		dvobj->txq_serv_group_exp = value;
	} else if (!strcmp(para, "dump")) {
		printk("max_enq_len=%u\n", dvobj->txq_max_enq_len);
		printk("max_agg_num=%u\n", dvobj->txq_max_agg_num);
		printk("hw_timeout=%u\n", dvobj->txq_hw_timeout);
		printk("pkt_timeout=%u\n", dvobj->txq_pkt_timeout);
		printk("max_serv_time=%u\n", dvobj->txq_max_serv_time);
		printk("amsdu_merge=%u\n", dvobj->txq_amsdu_merge);
		printk("tcpack_merge=%u\n", dvobj->txq_tcpack_merge);
		printk("deq_factor=%u\n", dvobj->txq_deq_factor);
		printk("ts_factor=%u\n", dvobj->txq_ts_factor);
		printk("deq_loop=%u\n", dvobj->txq_deq_loop);
		printk("serv_group_exp=%u\n", dvobj->txq_serv_group_exp);
	}
}
#endif

void reset_txforce_para(_adapter *adapter)
{
	adapter->txForce_rate 	= INV_TXFORCE_VAL;
	adapter->txForce_bw 	= INV_TXFORCE_VAL;
	adapter->txForce_agg 	= INV_TXFORCE_VAL;
	adapter->txForce_aggnum = INV_TXFORCE_VAL;
	adapter->txForce_gi 	= INV_TXFORCE_VAL;
	adapter->txForce_ampdu_density 	= INV_TXFORCE_VAL;
}

#ifdef CONFIG_CPU_PROFILING

#ifdef CONFIG_CPU_PMU_PROFILING
u32 cpu_grpid		= 0;
/* cpu cycles, inst cnt, load cnt, STORE cnt, L1T cache, STALL_BACKEND */
u32 cpu_events[6]	= {17, 8, 6, 7, 20, 36};
#else
u32 cpu_grpid		= 0;
u32 cpu_events[2]	= {CNT0_CYCLE, CNT1_DATA_CACHE_MISS};
u8  cpu_vpeid		= VPEID_INVALID;
u8  cpu_tcid		= TCID_INVALID;
#endif

void core_cmd_cpu_list(_adapter *padapter)
{
	dump_cpu_event_name();
}

void core_cmd_cpu_config_event(_adapter *padapter)
{
#ifdef CONFIG_CPU_PMU_PROFILING
	profile_config_event(cpu_grpid, cpu_events, ARRAY_SIZE(cpu_events));
	profile_stop();
	profile_start(1);
#else
	u32 cpu_event = (cpu_events[1]<<8)|cpu_events[0];
	hwperf_reinit(cpu_event, cpu_grpid, cpu_vpeid, cpu_tcid);
#endif
}

#ifdef CONFIG_CPU_PMU_PROFILING
void core_cmd_cpu_start_profiling(_adapter *padapter)
{
	profile_start(1);
}

void core_cmd_cpu_start_irq_profiling(_adapter *padapter)
{
	profile_start(-1);
}

void core_cmd_cpu_stop_profiling(_adapter *padapter)
{
	profile_stop();
}
#endif

void core_cmd_cpu_dump_result(_adapter *padapter)
{
#ifdef CONFIG_CPU_PMU_PROFILING
	profile_dump();
#else
	hwperf_dump();
#endif
}

void core_cmd_cpu_reset_result(_adapter *padapter)
{
#ifdef CONFIG_CPU_PMU_PROFILING
	profile_stop();
	profile_start(1);
#else
	//padapter->core_logs.rxCnt_data_orig=0;
	//padapter->core_logs.rxCnt_data_shortcut=0;
	hwperf_clear_all();
#endif
}
void core_cmd_profile(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0, value = 0;
	char *para = (char *)cmd_para;

	DBGP("\n");

	if (para_num <= 0) {
		RTW_PRINT("para error: \n");
		return;
	}

	if (!strcmp(para, "list")) {
		DBGP("profile list \n");
		core_cmd_cpu_list(adapter);
	} else if (!strcmp(para, "event0")) {
		DBGP("event0 \n");
		para = get_next_para_str(para);
		sscanf(para, "%u", &value);
		cpu_events[0] = value;
		core_cmd_cpu_config_event(adapter);
	} else if (!strcmp(para, "event1")) {
		DBGP("event1 \n");
		para = get_next_para_str(para);
		sscanf(para, "%u", &value);
		cpu_events[1] = value;
		core_cmd_cpu_config_event(adapter);
	}
#ifdef CONFIG_CPU_PMU_PROFILING
	else if (!strcmp(para, "event2")) {
		DBGP("event2 \n");
		para = get_next_para_str(para);
		sscanf(para, "%u", &value);
		cpu_events[2] = value;
		core_cmd_cpu_config_event(adapter);
	}
	else if (!strcmp(para, "event3")) {
		DBGP("event3 \n");
		para = get_next_para_str(para);
		sscanf(para, "%u", &value);
		cpu_events[3] = value;
		core_cmd_cpu_config_event(adapter);
	}
	else if (!strcmp(para, "event4")) {
		DBGP("event4 \n");
		para = get_next_para_str(para);
		sscanf(para, "%u", &value);
		cpu_events[4] = value;
		core_cmd_cpu_config_event(adapter);
	}
	else if (!strcmp(para, "event5")) {
		DBGP("event5 \n");
		para = get_next_para_str(para);
		sscanf(para, "%u", &value);
		cpu_events[5] = value;
		core_cmd_cpu_config_event(adapter);
	}
#endif
	else if (!strcmp(para, "group")) {
		DBGP("group \n");
		para = get_next_para_str(para);
		sscanf(para, "%u", &value);
		cpu_grpid = value;
		core_cmd_cpu_config_event(adapter);
	}
#ifdef CONFIG_CPU_PMU_PROFILING
	else if (!strcmp(para, "start")) {
		DBGP("start \n");
		core_cmd_cpu_start_profiling(adapter);
	}
	else if (!strcmp(para, "start_irq")) {
		DBGP("start_irq \n");
		core_cmd_cpu_start_irq_profiling(adapter);
	}
	else if (!strcmp(para, "stop")) {
		DBGP("stop \n");
		core_cmd_cpu_stop_profiling(adapter);
	}
#else
	else if (!strcmp(para, "vpe")) {
		DBGP("vpe \n");
		para = get_next_para_str(para);
		sscanf(para, "%x", &value);
		cpu_vpeid = value;
		core_cmd_cpu_config_event(adapter);
	} else if (!strcmp(para, "tc")) {
		DBGP("tc \n");
		para = get_next_para_str(para);
		sscanf(para, "%x", &value);
#if 0
		cpu_tcid = value;
#endif
		core_cmd_cpu_config_event(adapter);
	}
#endif
	else if (!strcmp(para, "dump")) {
		DBGP("dump \n");
		core_cmd_cpu_dump_result(adapter);
	} else if (!strcmp(para, "reset")) {
		DBGP("reset \n");
		core_cmd_cpu_reset_result(adapter);
	} else {
		DBGP("else \n");
	}


}
#endif
void core_cmd_txforce(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0, value = 0;
	char *para = (char *)cmd_para;
	u8 is_txforce_apply = 1;

	DBGP("\n");

	if (para_num <= 0) {
		RTW_PRINT("TX Force: \n"
			  "\tEnable: %u\n"
			  "\trate: %X\n"
			  "\tbw: %X\n"
			  "\tagg: %X\n"
			  "\taggnum: %X\n"
			  "\tgi: %X\n"
			  "\tampdu_density: %X\n",
			adapter->txForce_enable,
			adapter->txForce_rate,
			adapter->txForce_bw,
			adapter->txForce_agg,
			adapter->txForce_aggnum,
			adapter->txForce_gi,
			adapter->txForce_ampdu_density);
		return;
	}

	if(!strcmp(para, "start")){
		DBGP("txforce start \n");
		adapter->txForce_enable = 1;
		reset_txforce_para(adapter);
	}else if(!strcmp(para, "stop")){
		DBGP("txforce stop \n");
		adapter->txForce_enable = 0;
		reset_txforce_para(adapter);
	}else if(!strcmp(para, "rate")){
		para=get_next_para_str(para);
		sscanf(para, "%x", &value);
		DBGP("rate=0x%x \n", value);
		adapter->txForce_rate = value;
	}else if(!strcmp(para, "bw")){
		para=get_next_para_str(para);
		sscanf(para, "%x", &value);
		DBGP("bw=0x%x \n", value);
		adapter->txForce_bw = value;
	}else if(!strcmp(para, "agg")){
		para=get_next_para_str(para);
		sscanf(para, "%x", &value);
		DBGP("agg=0x%x \n", value);
		adapter->txForce_agg = value;
	}else if(!strcmp(para, "aggnum")){
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("aggnum=0x%x \n", value);
		adapter->txForce_aggnum = value;
	}else if(!strcmp(para, "gi")){
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("gi=0x%x \n", value);
		adapter->txForce_gi = value;
	}else if(!strcmp(para, "density")){
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ampdu_density=0x%x \n", value);
		adapter->txForce_ampdu_density = value;
	} else {
		is_txforce_apply = 0;
		RTW_PRINT("TX Force: \n"
			  "\tEnable: %u\n"
			  "\trate: %X\n"
			  "\tbw: %X\n"
			  "\tagg: %X\n"
			  "\taggnum: %X\n"
			  "\tgi: %X\n"
			  "\tampdu_density: %X\n",
			adapter->txForce_enable,
			adapter->txForce_rate,
			adapter->txForce_bw,
			adapter->txForce_agg,
			adapter->txForce_aggnum,
			adapter->txForce_gi,
			adapter->txForce_ampdu_density);
	}

#ifdef CONFIG_CORE_TXSC
	if(is_txforce_apply)
		txsc_clear(adapter, 1);
#endif
}

#ifdef CONFIG_RTW_A4_STA
void cmd_a4_dump_sta(_adapter *adapter, void *m)
{
	struct list_head *phead, *plist;
	struct sta_info *psta_a4;
	u32 idx = 0;

	phead = &adapter->a4_sta_list;
	plist = phead->next;

	RTW_PRINT_SEL(m, "=== A4 STA List === \n");
	while ((plist != phead) && (plist != NULL)) {

		psta_a4 = list_entry(plist, struct sta_info, a4_sta_list);

		if (psta_a4 && psta_a4->phl_sta) {
			RTW_PRINT_SEL(m, "[%d]a4_sta(%d):"MAC_FMT" \n", idx,
				psta_a4->phl_sta->macid, MAC_ARG(psta_a4->phl_sta->mac_addr));
		} else {
			if (!psta_a4)
				RTW_PRINT_SEL(m, "null psta_a4 \n");
			else if (!psta_a4->phl_sta)
				RTW_PRINT_SEL(m, "null phl_sta \n");
		}
		plist = plist->next;
		idx++;
	}
}

void cmd_a4_dump_db(_adapter *adapter, void *m)
{
	struct rtw_a4_db_entry *db;
#ifdef CONFIG_A4_LOOPBACK
	struct rtw_a4_loopback_entry *entry;
#endif
	u32 i;
	RTW_PRINT_SEL(m, "=== A4 Database === \n");
	for (i = 0 ; i < A4_STA_HASH_SIZE; i++) {
		machash_spinlock_bh(i);
		db = adapter->machash[i];

		while (db != NULL) {
			RTW_PRINT_SEL(m, "[%d] ", i);
			RTW_PRINT_SEL(m, "source:"MAC_FMT" ", MAC_ARG(db->mac));

			if (db->psta) {
				if (db->psta->phl_sta)
					RTW_PRINT_SEL(m, "with a4_sta(%d) \n",
						db->psta->phl_sta->macid);
				else
					RTW_PRINT_SEL(m, "with null phl_sta %p\n",
						db->psta);
				/* A4_CNT */
				RTW_PRINT_SEL(m, "	tx_bytes:	%lld\n", db->tx_bytes);
				RTW_PRINT_SEL(m, "	tx_count:	%d\n", db->tx_count);
				RTW_PRINT_SEL(m, "	rx_bytes:	%lld\n", db->rx_bytes);
				RTW_PRINT_SEL(m, "	rx_count:	%d\n", db->rx_count);
				RTW_PRINT_SEL(m, "	link_time:	%d\n", db->link_time);
				RTW_PRINT_SEL(m, "	aging_time:	%ld\n", (jiffies - db->ageing_timer)/HZ);
			} else {
				RTW_PRINT_SEL(m, "with null psta !! \n");
			}

			//printk("Aging time: %ld", (jiffies - db->ageing_timer)/HZ);

			db = db->next_hash;
		}
		machash_spinunlock_bh(i);
	}
#ifdef CONFIG_A4_LOOPBACK
	RTW_PRINT_SEL(m, "=== A4 loop entry : %d=== \n", adapter->replace_idx);
	for (i = 0 ; i < A4_LOOP_HASH_SIZE; i++)
	{
		hlist_for_each_entry(entry,&adapter->a4_loop_list[i], hnode)
		{
			RTW_PRINT_SEL(m, "[%d]	mac:"MAC_FMT", time: %u\n", i, MAC_ARG(entry->mac), rtw_systime_to_ms(entry->stime));
		}
	}
#endif
}

void core_cmd_a4(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	char *para = (char *)cmd_para;

	DBGP("\n");

	if(para_num<=0)
		return;

	if (!strcmp(para, "dump")) {
		printk("a4_enable = %d \n", adapter->a4_enable);
		printk("cnt_a4: tx = %d, txsc = %d, txsc_amsdu = %d \n", adapter->cnt_a4_tx,
				adapter->cnt_a4_txsc, adapter->cnt_a4_txsc_amsdu);
		printk("cnt_a4: rx = %d, rxsc = %d, rxsc_amsdu = %d \n", adapter->cnt_a4_rx,
				adapter->cnt_a4_rxsc, adapter->cnt_a4_rxsc_amsdu);
		if (adapter->a4_enable) {
			cmd_a4_dump_sta(adapter, RTW_DBGDUMP);
			cmd_a4_dump_db(adapter, RTW_DBGDUMP);
		}
	}
}
#endif

#if defined (CONFIG_RTW_MULTI_AP) && defined (DEBUG_MAP_NL)
void core_cmd_map(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	char *para = (char *)cmd_para;

	DBGP("\n");

	if(para_num<=0)
		return;

	if (!strcmp(para, "event")) {
		u8 map_evt_buf[100];
		_rtw_memset(map_evt_buf, 0, sizeof(map_evt_buf));
		core_map_nl_event_send(map_evt_buf, 100);
	} else if (!strcmp(para, "unassoc_sta")) {

	}
}
#endif

#ifdef CONFIG_RTW_CORE_RXSC
void core_cmd_rxsc(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	char *para = (char *)cmd_para;
	struct sta_priv	*pstapriv = &adapter->stapriv;
	struct sta_info *psta = NULL;
	int i, j;
	_list	*plist, *phead;
	struct core_rxsc_entry *rxsc_entry = NULL;
	struct rxsc_wlan_hdr *rxsc_wlanhdr = NULL;

	DBGP("\n");

	if(para_num<=0)
		return;

	if(!strcmp(para, "enable")){
		DBGP("enable\n");
		adapter->enable_rxsc = 1;
	}else if(!strcmp(para, "disable")){
		DBGP("disable\n");
		adapter->enable_rxsc = 0;
	}else if(!strcmp(para, "dump")){
		struct core_logs *log = &adapter->core_logs;
		DBGP("dump\n");
		printk("enable_rxsc: %d \n", adapter->enable_rxsc);
		printk("rxCnt_data: orig=%d shortcut=%d(ratio=%d)\n",
			log->rxCnt_data_orig, log->rxCnt_data_shortcut,
			log->rxCnt_data_shortcut*100/((log->rxCnt_data_orig+log->rxCnt_data_shortcut)?:1));
	}else if(!strcmp(para, "debug")){
		DBGP("debug\n");
		for (i = 0; i < NUM_STA; i++) {
			phead = &(pstapriv->sta_hash[i]);
			plist = get_next(phead);

			while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
				psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
				plist = get_next(plist);

				printk("[%d] STA[%pM] %p next_idx %d\n",
						i, psta->phl_sta->mac_addr, psta, psta->rxsc_idx_new);
				for (j = 0 ; j < NUM_RXSC_ENTRY; j++) {
					rxsc_entry = &psta->rxsc_entry[j];
					rxsc_wlanhdr = &rxsc_entry->rxsc_wlanhdr;
					if (rxsc_entry->status == RXSC_ENTRY_INVALID)
						continue;
					printk("[%d] st %d amsdu %d htc %d hdrlen %d plyd_ofst %d [ hit %d ]\n",
							j, rxsc_entry->status, rxsc_entry->is_amsdu, rxsc_entry->is_htc,
							rxsc_entry->rxsc_attrib.hdrlen, rxsc_entry->rxsc_payload_offset,
							rxsc_entry->hit);
					printk("  > %04x  %pM  %pM  %pM  ",
							rxsc_wlanhdr->fmctrl, rxsc_wlanhdr->addr1,
							rxsc_wlanhdr->addr2, rxsc_wlanhdr->addr3);
					if (rxsc_entry->is_amsdu == 0)
						printk("proto %04x\n", rxsc_entry->rxsc_ethhdr.h_proto);
					else
						printk("\n");
				}
				printk("\n");
			}
		}
		if (adapter->cached_sta)
			printk("cached_sta: %pM %p\n",
					adapter->cached_sta->phl_sta->mac_addr, adapter->cached_sta);
		else
			printk("cached_sta: none\n");
	}
}
#endif


#ifdef RTW_CORE_PKT_TRACE
void core_cmd_pktrace(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	char *para = (char *)cmd_para;
	char *value = NULL;
	int error=0;
	struct rtw_pkt_filter filter;
	//DBGP("\n");

	if(para_num<=0)
		return;

	memset(&filter,0,sizeof(filter));

	if(0==strncmp(para,"enable",5)) {
		adapter->pkt_trace_enable = 1;
	} else if(0==strncmp(para,"disable",7))	{
		adapter->pkt_trace_enable = 0;
	} else if(0==strncmp(para,"dumphdr",7)) {
		adapter->pkt_trace_level = RTW_PKT_DUMP_HEADER;
	} else if(0==strncmp(para,"dumpall",7)) {
		adapter->pkt_trace_level = RTW_PKT_DUMP_ALL;
	} else if(0==strncmp(para,"dumpnone",8)) {
		adapter->pkt_trace_level = RTW_PKT_DUMP_NONE;
	} else if(0==strncmp(para,"flush",5)) {
		rtw_flush_pkt_filter(adapter);
	} else if(0==strncmp(para,"show",4)) {
		RTW_PRINT("pkt_trace_enable %d pkt_trace_level %d\n",adapter->pkt_trace_enable,adapter->pkt_trace_level);
		rtw_dump_pkt_filter(adapter);
	}
	else
	{

		while(para_num > 0)
		{
			para_num--;
			if(0 == rtw_get_filter_flag_by_name(para)) {
				RTW_ERR("not supported cmd %s\n",para);
				error = 1;
				break;
			}
			if(para_num > 0) {
				value = get_next_para_str(para);
				para_num--;
			} else {
				value = NULL;
			}
			if (value) {
				if(rtw_generate_pkt_filter(&filter, para, value) < 0) {
					error = 1;
					break;
				}
			}
			if((para_num > 0) && value) {
				para = get_next_para_str(value);
			}
		}

		if((para_num == 0) && (error ==0)) {
			rtw_add_pkt_filter(adapter,&filter);
		}
	}
}

#endif


void core_sniffer_rx(_adapter *adapter, u8 *pkt, u32 pktlen)
{
	struct sk_buff* pskb = NULL;

	if(!adapter->sniffer_enable)
		return;

	if(pkt==NULL)
		return;

	pskb = dev_alloc_skb(pktlen+200);

	if(pskb == NULL){
	return;
}

	_rtw_memcpy(pskb->data, pkt, pktlen);
	pskb->len = pktlen;

	skb_reset_mac_header(pskb);
	pskb->dev = adapter->pnetdev;
	pskb->dev->type = ARPHRD_IEEE80211;
	pskb->ip_summed = CHECKSUM_UNNECESSARY;
	pskb->pkt_type = PACKET_OTHERHOST;
	pskb->protocol = htons(ETH_P_802_2);
	netif_receive_skb(pskb);

	return;
}

void core_cmd_sniffer(_adapter *adapter, void *cmd_para, u32 para_num)
	{
		u32 idx=0;
	char *para = (char *)cmd_para;

	if(para_num<=0)
	return;

	if(!strcmp(para, "start")){
		adapter->sniffer_enable = 1;
	}else if(!strcmp(para, "stop")){
		adapter->sniffer_enable = 0;
	}
}


#define LEN_TEST_BUF 2000
u8 test_buf[LEN_TEST_BUF];

#ifdef CONFIG_PCI_HCI
#include <rtw_trx_pci.h>
#endif

#include "../phl/phl_headers.h"
#include "../phl/phl_api.h"
#include "../phl/hal_g6/hal_headers.h"
//#include "../phl/hal_g6/hal_api_mac.h"
#include "../phl/hal_g6/mac/mac_reg.h"

void _show_RX_counter(_adapter *adapter, void *m)
{
	/* Show RX PPDU counters */
	int i;
	u32 reg32 = rtw_phl_read32(adapter->dvobj->phl, R_AX_RX_DBG_CNT_SEL);
	static const char *cnt_name[] = {"OFDM MPDU OK counter",
            						 "OFDM MPDU Fail counter",
	                                 "OFDM False Alarm counter",
	                                 "CCK MPDU OK counter",
	                                 "CCK MPDU Fail counter",
	                                 "CCK False Alarm counter",
	                                 "HT MPDU OK counter",
	                                 "HT MPDU Fail counter",
	                                 "HT PPDU counter",
	                                 "HT False Alarm counter",
	                                 "VHT SU MPDU OK counter",
	                                 "VHT SU MPDU Fail counter",
	                                 "VHT SU PPDU counter",
	                                 "VHT SU False Alarm counter",
	                                 "VHT MU MPDU OK counter",
	                                 "VHT MU MPDU Fail counter",
	                                 "VHT MU PPDU counter",
	                                 "VHT MU False Alarm counter",
	                                 "HE SU MPDU OK counter",
	                                 "HE SU MPDU Fail counter",
	                                 "HE SU PPDU counter",
	                                 "HE SU False Alarm counter",
	                                 "HE MU MPDU OK counter",
	                                 "HE MU MPDU Fail counter",
	                                 "HE MU PPDU counter",
	                                 "HE MU False Alarm counter",
	                                 "HE TB MPDU OK counter",
	                                 "HE TB MPDU Fail counter",
	                                 "HE TB PPDU counter",
	                                 "HE TB False Alarm counter",
	                                 "Invalid packet",
	                                 "RE-CCA",
	                                 "RX FIFO overflow",
	                                 "RX packet full drop",
	                                 "RX packet dma OK",
	                                 "UD 0",
	                                 "UD 1",
	                                 "UD 2",
	                                 "UD 3",
	                                 "continuous FCS error",
	                                 "RX packet filter drop",
	                                 "CSI packet DMA OK",
	                                 "CSI packet DMA drop",
	                                 "RX MAC stop"
	};

	RTW_PRINT_SEL(m, "CMAC0 RX PPDU Counters @%04X:\n", R_AX_RX_DBG_CNT_SEL);

	reg32 &= ~(B_AX_RX_CNT_IDX_MSK << B_AX_RX_CNT_IDX_SH);
	for (i = 0; i < 44; i++) {
		rtw_phl_write32(adapter->dvobj->phl, R_AX_RX_DBG_CNT_SEL,
		            reg32 | (i << B_AX_RX_CNT_IDX_SH));
	RTW_PRINT_SEL(m, "    %02X: %d - %s\n", i,
	         (
	            (   rtw_phl_read32(adapter->dvobj->phl, R_AX_RX_DBG_CNT_SEL)
	             >> B_AX_RX_DBG_CNT_SH)
	          & B_AX_RX_DBG_CNT_MSK),
	          cnt_name[i]);
	}
	// SCC for now
	#if 0
	reg32 = rtw_phl_read32(adapter->dvobj->phl, R_AX_RX_DBG_CNT_SEL_C1);
	printk("CMAC1 RX PPDU Counters @%04X:\n", R_AX_RX_DBG_CNT_SEL_C1);
	reg32 &= ~(B_AX_RX_CNT_IDX_MSK << B_AX_RX_CNT_IDX_SH);
	for (i = 0; i < 44; i++) {
		rtw_phl_write32(adapter->dvobj->phl, R_AX_RX_DBG_CNT_SEL_C1,
		            reg32 | (i << B_AX_RX_CNT_IDX_SH));
		printk("    %02X: %d - %s\n", i,
		         (
		            (   rtw_phl_read32(adapter->dvobj->phl, R_AX_RX_DBG_CNT_SEL_C1)
		             >> B_AX_RX_DBG_CNT_SH)
		          & B_AX_RX_DBG_CNT_MSK),
		          cnt_name[i]);
	}
	#endif
} /* _show_RX_counter */

void _show_TX_dbg_status(_adapter *adapter, void *m)
{
	u32	reg32 = rtw_phl_read32(adapter->dvobj->phl, 0x9F1C);


	RTW_PRINT_SEL(m, "TX Debug: 0x%08X\n", reg32);
}

void _show_BCN_dbg_status(_adapter *adapter, void *m)
{
	RTW_INFO("Beacon regsiters:\n");
	SHOW_REG32_MSG(adapter, R_AX_PORT_CFG_P0,		"PORT_CFG_P0", m);
	SHOW_REG32_MSG(adapter, R_AX_TBTT_PROHIB_P0,	"TBTT_PROHIB_P0", m);
	SHOW_REG32_MSG(adapter, R_AX_EN_HGQ_NOLIMIT,	"EN_HGQ_NOLIMIT", m);
	SHOW_REG16_MSG(adapter, R_AX_TBTT_AGG_P0,		"TBTT_AGG_P0", m);
	SHOW_REG32_MSG(adapter, R_AX_MBSSID_CTRL,	"MBSSID_CTRL", m);
	// SCC for now
	#if 0
	SHOW_REG32_MSG(adapter, R_AX_PORT_CFG_P0_C1,	"PORT_CFG_P0_C1");
	SHOW_REG32_MSG(adapter, R_AX_TBTT_PROHIB_P0_C1,	"TBTT_PROHIB_P0_C1");
	SHOW_REG32_MSG(adapter, R_AX_EN_HGQ_NOLIMIT_C1,	"EN_HGQ_NOLIMIT_C1");
	SHOW_REG32_MSG(adapter, R_AX_TBTT_AGG_P0_C1,	"TBTT_AGG_P0_C1");
	#endif
	SHOW_REG32_MSG(adapter, R_AX_WCPU_FW_CTRL,		"R_AX_WCPU_FW_CTRL", m);
}


void core_cmd_dump_debug(_adapter *adapter, void *cmd_para, u32 para_num)
{
	printk("TX path registers: \n");

	/* ToDo: Move to chip's HAL layer */
#if defined(CONFIG_RTL8852AE) || defined (CONFIG_RTL8852BE)
	SHOW_REG32_MSG(adapter, R_AX_RXQ_RXBD_IDX, "RX_BD_IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_RPQ_RXBD_IDX, "RP_BD_IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH0_TXBD_IDX, "ACH0 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH0_PAGE_INFO, "ACH0 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH0_BDRAM_RWPTR, "ACH0 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH1_TXBD_IDX, "ACH1 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH1_PAGE_INFO, "ACH1 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH1_BDRAM_RWPTR, "ACH1 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH2_TXBD_IDX, "ACH2 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH2_PAGE_INFO, "ACH2 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH2_BDRAM_RWPTR, "ACH2 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH3_TXBD_IDX, "ACH3 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH3_PAGE_INFO, "ACH3 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH3_BDRAM_RWPTR, "ACH3 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH4_TXBD_IDX, "ACH4 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH4_PAGE_INFO, "ACH4 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH4_BDRAM_RWPTR, "ACH4 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH5_TXBD_IDX, "ACH5 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH5_PAGE_INFO, "ACH5 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH5_BDRAM_RWPTR, "ACH5 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH6_TXBD_IDX, "ACH6 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH6_PAGE_INFO, "ACH6 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH6_BDRAM_RWPTR, "ACH6 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH7_TXBD_IDX, "ACH7 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH7_PAGE_INFO, "ACH7 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH7_BDRAM_RWPTR, "ACH7 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH8_TXBD_IDX, "CH8 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH8_PAGE_INFO, "CH8 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_CH8_BDRAM_RWPTR, "CH8 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH9_TXBD_IDX, "CH9 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH9_PAGE_INFO, "CH9 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_CH9_BDRAM_RWPTR, "CH9 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH10_TXBD_IDX, "CH10 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH10_PAGE_INFO, "CH10 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_CH10_BDRAM_RWPTR, "CH10 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH11_TXBD_IDX, "CH11 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH11_PAGE_INFO, "CH11 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_CH11_BDRAM_RWPTR, "CH11 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH12_TXBD_IDX, "CH12 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH12_PAGE_INFO, "CH12 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_CH12_BDRAM_RWPTR, "CH12 BD RWPTR", RTW_DBGDUMP);
#else
	SHOW_REG32_MSG(adapter, R_AX_RXQ_RXBD_IDX_V1, "RX_BD_IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_RPQ_RXBD_IDX_V1, "RP_BD_IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH0_TXBD_IDX, "ACH0 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH0_PAGE_INFO_V1, "ACH0 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH0_BDRAM_RWPTR, "ACH0 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH1_TXBD_IDX, "ACH1 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH1_PAGE_INFO_V1, "ACH1 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH1_BDRAM_RWPTR, "ACH1 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH2_TXBD_IDX, "ACH2 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH2_PAGE_INFO_V1, "ACH2 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH2_BDRAM_RWPTR, "ACH2 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH3_TXBD_IDX, "ACH3 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH3_PAGE_INFO_V1, "ACH3 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH3_BDRAM_RWPTR, "ACH3 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH4_TXBD_IDX, "ACH4 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH4_PAGE_INFO_V1, "ACH4 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH4_BDRAM_RWPTR, "ACH4 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH5_TXBD_IDX, "ACH5 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH5_PAGE_INFO_V1, "ACH5 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH5_BDRAM_RWPTR, "ACH5 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH6_TXBD_IDX, "ACH6 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH6_PAGE_INFO_V1, "ACH6 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH6_BDRAM_RWPTR, "ACH6 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH7_TXBD_IDX, "ACH7 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_ACH7_PAGE_INFO_V1, "ACH7 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_ACH7_BDRAM_RWPTR, "ACH7 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH8_TXBD_IDX, "CH8 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH8_PAGE_INFO_V1, "CH8 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_CH8_BDRAM_RWPTR, "CH8 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH9_TXBD_IDX, "CH9 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH9_PAGE_INFO_V1, "CH9 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_CH9_BDRAM_RWPTR, "CH9 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH10_TXBD_IDX, "CH10 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH10_PAGE_INFO_V1, "CH10 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_CH10_BDRAM_RWPTR, "CH10 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH11_TXBD_IDX, "CH11 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH11_PAGE_INFO_V1, "CH11 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_CH11_BDRAM_RWPTR, "CH11 BD RWPTR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH12_TXBD_IDX, "CH12 IDX", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CH12_PAGE_INFO_V1, "CH12 PG INFO", RTW_DBGDUMP);
	SHOW_REG16_MSG(adapter, R_AX_CH12_BDRAM_RWPTR, "CH12 BD RWPTR", RTW_DBGDUMP);
#endif
#ifdef R_AX_PCIE_DBG_CTRL
	SHOW_REG32_MSG(adapter, R_AX_PCIE_DBG_CTRL, "DBG_CTRL", RTW_DBGDUMP);
#else
	SHOW_REG32_MSG(adapter, 0x11C0, "DBG_CTRL", RTW_DBGDUMP);
#endif
	SHOW_REG32_MSG(adapter, R_AX_DBG_ERR_FLAG, "DBG_ERR", RTW_DBGDUMP);
#if defined(CONFIG_RTL8852AE) || defined (CONFIG_RTL8852BE)
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HIMR00, "IMR0", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HISR00, "ISR0", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HIMR10, "IMR1", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HISR10, "ISR1", RTW_DBGDUMP);
#else
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HIMR00_V1, "IMR0_V1", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HISR00_V1, "ISR0_V1", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HIMR00, "IMR0", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HISR00, "ISR0", RTW_DBGDUMP);
#endif


	SHOW_REG32_MSG(adapter, R_AX_PCIE_DMA_STOP1, "DMA_STOP1", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_DMA_BUSY1, "DMA_BUSY1", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_DMA_STOP2, "DMA_STOP2", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_DMA_BUSY2, "DMA_BUSY2", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CTN_TXEN, "CTN_TXEN", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_CTN_DRV_TXEN, "CTN_DRV_TXEN", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_WCPU_FW_CTRL, "R_AX_WCPU_FW_CTRL", RTW_DBGDUMP);

	SHOW_REG32_MSG(adapter, R_AX_CMAC_ERR_ISR, "CMAC_ERR", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, R_AX_PTCL_COMMON_SETTING_0, "PTCL_COMMON_SETTING_0", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, 0x11a24, "BB_CCA", RTW_DBGDUMP);

	SHOW_REG32_MSG(adapter, 0x11a64, "BB_LEGACY", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, 0x11a60, "BB_HT", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, 0x11a5C, "BB_VHT", RTW_DBGDUMP);
	SHOW_REG32_MSG(adapter, 0x11a58, "BB_HE", RTW_DBGDUMP);

	SHOW_REG32(adapter, 0x8840, RTW_DBGDUMP);
	SHOW_REG32(adapter, 0x8844, RTW_DBGDUMP);
	SHOW_REG32(adapter, 0x8854, RTW_DBGDUMP);
	SHOW_REG16(adapter, 0xCA22, RTW_DBGDUMP);
	SHOW_REG32(adapter, 0x8AA8, RTW_DBGDUMP);

	/* Show TX PPDU counters */
	do {
		int i;
		u32 reg32 = rtw_phl_read32(adapter->dvobj->phl, R_AX_TX_PPDU_CNT);

		printk("CMAC0 TX PPDU Counters @%04X:\n", R_AX_TX_PPDU_CNT);

		reg32 &= ~(B_AX_PPDU_CNT_IDX_MSK << B_AX_PPDU_CNT_IDX_SH);
		for (i = 0; i < 11; i++) {
			rtw_phl_write32(adapter->dvobj->phl, R_AX_TX_PPDU_CNT,
			            reg32 | (i << B_AX_PPDU_CNT_IDX_SH));
			printk("    %02X: %d\n", i,
			         (
			            (   rtw_phl_read32(adapter->dvobj->phl, R_AX_TX_PPDU_CNT)
			             >> B_AX_TX_PPDU_CNT_SH)
			          & B_AX_TX_PPDU_CNT_MSK));
		}
		// SCC for now
		#if 0
		reg32 = rtw_phl_read32(adapter->dvobj->phl, R_AX_TX_PPDU_CNT_C1);

		printk("CMAC1 TX PPDU Counters @%04X:\n", R_AX_TX_PPDU_CNT_C1);

		reg32 &= ~(B_AX_PPDU_CNT_IDX_MSK << B_AX_PPDU_CNT_IDX_SH);
		for (i = 0; i < 11; i++) {
			rtw_phl_write32(adapter->dvobj->phl, R_AX_TX_PPDU_CNT_C1,
			            reg32 | (i << B_AX_PPDU_CNT_IDX_SH));
			printk("    %02X: %d\n", i,
			         (
			            (   rtw_phl_read32(adapter->dvobj->phl, R_AX_TX_PPDU_CNT_C1)
			             >> B_AX_TX_PPDU_CNT_SH)
			          & B_AX_TX_PPDU_CNT_MSK));
		}
		#endif
	} while (0);

	/* Show RX PPDU counters */
	_show_RX_counter(adapter, RTW_DBGDUMP);

	_show_TX_dbg_status(adapter, RTW_DBGDUMP);

	_show_BCN_dbg_status(adapter, RTW_DBGDUMP);

}

void core_cmd_dump_reg(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 *para = (u32 *)cmd_para;
	void *phl = adapter->dvobj->phl;
	u32 reg_start, reg_end;
	u32 idx = 0;

	reg_start = para[0];
	reg_end = reg_start + para[1];

	while(1) {
		u32	reg0, reg1, reg2, reg3;
		if((reg_start >= reg_end) /*|| (reg_start >= 0x1ffff)*/)
			break;

		reg0 = rtw_phl_read32(phl, reg_start);
		reg1 = rtw_phl_read32(phl, reg_start+4);
		reg2 = rtw_phl_read32(phl, reg_start+8);
		reg3 = rtw_phl_read32(phl, reg_start+12);

		printk("[%04x] %08x %08x %08x %08x \n",
			reg_start,
			rtw_phl_read32(phl, reg_start), rtw_phl_read32(phl, reg_start+4),
			rtw_phl_read32(phl, reg_start+8), rtw_phl_read32(phl, reg_start+12));

		reg_start+=16;
	}
}


void Get_macid_WD_by_AC(_adapter *adapter, u8 macid, u8 AC)
{
	void *phl = adapter->dvobj->phl;

    u32 trigger_val = (0x1 << 31) | (0x6 << 16) | ( macid << 3) | (AC << 1);
    int timeout = 500;

	u32 reg;

	//printk("trigger_val:%d\n", trigger_val);

    //WriteMACRegDWord(0x8d10, trigger_val);
	rtw_phl_write32(phl, 0x8d10, trigger_val);

	//printk("write ok\n");

	reg = rtw_phl_read32(phl, 0x8d14);

    while((reg & (0x1<<31)) && timeout)
    {
        timeout --;
    }
    if(timeout == 0)
    {
        return;
    }

    //return (ReadMACRegDWord(0x8d14) & 0Xfff);
    printk("macid:%d  %d\n", macid, (rtw_phl_read32(phl, 0x8d14) & 0xfff));
}

void core_cmd_dump_wd(_adapter *adapter, void *cmd_para, u32 para_num)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	void *phl = adapter->dvobj->phl;
	char *para = (char *)cmd_para;

	rtw_phl_dump_wd_balance_status(phl, RTW_DBGDUMP);
	RTW_PRINT("\n");

	return;
}

void core_cmd_dump_cnt(_adapter *adapter, void *cmd_para, u32 para_num)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	#endif

	void *phl = adapter->dvobj->phl;
	char *para = (char *)cmd_para;
	int i = 0, j = 0;
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct sta_info *psta = NULL;
	_list	*phead, *plist;
	u32 nr_xmitframe_cnt = GET_HAL_SPEC(pdvobjpriv)->band_cap & BAND_CAP_5G ? NR_XMITFRAME_5G : NR_XMITFRAME_2G;

	struct seq_file *m;

	if(!strcmp(para, "reset") || !strcmp(para, "ofdma"))
	{
		m = RTW_DBGDUMP;
	}
	else
	{
		m = (struct seq_file *)cmd_para;
		if(strlen(para)==0)	m = RTW_DBGDUMP;
	}

	phead = &pstapriv->asoc_list;

	/* FS/LS debug */
	if(!strcmp(para, "reset"))
		adapter->FS_LS_cnt = 0;

	if(!strcmp(para, "reset")){
		pxmitpriv->os_tx_pkts = 0;
		pxmitpriv->os_tx_drop = 0;

		pxmitpriv->core_tx_pkts = 0;
		pxmitpriv->core_tx_drop = 0;

		for(i = 0; i < 10; i++)
			pxmitpriv->core_tx_abort[i] = 0;

		#ifdef DEBUG_PHL_TX
		phl_com->tx_stats.wp_tg_out_of_resource = 0;
		phl_com->tx_stats.wp_tg_force_reuse = 0;
		phl_com->tx_stats.phl_txreq_sta_leave_drop = 0;
		#endif

		adapter->txreq_full_cnt = 0;

		#ifdef CONFIG_WFA_OFDMA_Logo_Test
		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
		plist = get_next(phead);
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);

			psta->core_xframe_abort = 0;
			psta->core_txreq_abort = 0;

			rtw_phl_reset_wd_balance_status(phl, adapter->phl_role);
		}
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
		#endif

		#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
		plist = get_next(phead);
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);

			psta->os_tx_cnt	= 0;
			psta->os_tx_drop_cnt = 0;

			psta->core_xframe_total_cnt = 0;
			psta->core_txsc_amsdu_need_enq = 0;
			psta->core_txsc_amsdu_need_deq = 0;
			psta->core_txsc_amsdu_abort = 0;
			psta->core_txsc_amsdu_timeout = 0;
			psta->core_txsc_amsdu_deq = 0;
			psta->core_txsc_apply_cnt_1 = 0;
			psta->core_txsc_apply_cnt_2 = 0;
			psta->core_txsc_apply_no_txreq = 0;
			psta->core_txsc_apply_fail = 0;
		}
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

		rtw_phl_reset_tx_status(phl, adapter->phl_role);

		phl_com->update_txbd_ok = 0;
		phl_com->update_txbd_fail = 0;
		phl_com->trigger_txstart_ok = 0;
		phl_com->trigger_txstart_fail = 0;
		#endif
	}

	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "	cur_tx_tp: %d\n", pdvobjpriv->traffic_stat.cur_tx_tp);

	/* XMITFRAME */
	RTW_PRINT_SEL(m, "[XMITFRAME]\n");
	RTW_PRINT_SEL(m, "	xf free / full : %d / %d\n", pxmitpriv->free_xmitframe_cnt, pxmitpriv->full_xmitframe_cnt);
#ifdef CONFIG_DYN_ALLOC_XMITFRAME
	RTW_PRINT_SEL(m, "	xf alloc_fail: %d\n", pxmitpriv->alloc_fail_xmitframe_cnt);
#endif
	RTW_PRINT_SEL(m, "[XMITFRAME_EXT]\n");
	RTW_PRINT_SEL(m, "	xf_ext free / full : %d / %d\n", pxmitpriv->free_xframe_ext_cnt, pxmitpriv->full_xframe_ext_cnt);
#ifdef CONFIG_DYN_ALLOC_XMITFRAME
	RTW_PRINT_SEL(m, "	xf_ext alloc_fail / alloc_txreq_fail: %d / %d\n",
		pxmitpriv->alloc_fail_xmitframe_ext_cnt, pxmitpriv->alloc_fail_txreq_ext_cnt);
#endif
#ifdef CONFIG_TX_DEFER
	RTW_PRINT_SEL(m, "	defer_tx flag / cnt: %d / %d\n", pxmitpriv->defer_tx_flag, ATOMIC_READ(&pxmitpriv->defer_tx_cnt));
#endif

	/* TXREQ */
	RTW_PRINT_SEL(m, "[TXREQ]\n");
	RTW_PRINT_SEL(m, "	txreq free / full: %d / %d\n", adapter->pfree_txreq_queue->qlen, adapter->txreq_full_cnt);
	RTW_PRINT_SEL(m, "	resv_cnt: %d\n", adapter->registrypriv.wifi_mib.res_txreq);/* RESERVE_TXREQ */

	/* FS/LS debug */
	RTW_PRINT_SEL(m, "[RX PACKET DROP]: \n");
	RTW_PRINT_SEL(m, "	RX FS/LS drop cnt: %d\n", adapter->FS_LS_cnt);

	if (phl_com->tx_stats.wp_tg_out_of_resource > 0)
		RTW_PRINT_SEL(m, "	wp_tg_out_of_resource: %d\n", phl_com->tx_stats.wp_tg_out_of_resource);
	if (phl_com->tx_stats.wp_tg_force_reuse > 0)
		RTW_PRINT_SEL(m, "	wp_tg_force_reuse: %d\n", phl_com->tx_stats.wp_tg_force_reuse);
	if (phl_com->tx_stats.phl_txreq_sta_leave_drop > 0)
		RTW_PRINT_SEL(m, "	phl_txreq_sta_leave_drop: %d\n", phl_com->tx_stats.phl_txreq_sta_leave_drop);
	if (adapter->xmitpriv.cnt_txsc_amsdu_dfree > 0)
		RTW_PRINT_SEL(m, "	cnt_txsc_amsdu_dfree: %d\n", adapter->xmitpriv.cnt_txsc_amsdu_dfree);


#ifdef CONFIG_WFA_OFDMA_Logo_Test
	RTW_PRINT_SEL(m, "[core_xframe_macid_current]:\n");
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		if (psta->core_current_xframe_cnt > 0)
			RTW_PRINT_SEL(m, "	macid[%d] core_current_xframe_cnt: %d\n",
				psta->phl_sta->macid, psta->core_current_xframe_cnt);
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	//RTW_PRINT("\n");

	RTW_PRINT_SEL(m, "[core_txreq_macid_current]:\n");
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		if (psta->core_current_txreq_cnt > 0)
			RTW_PRINT_SEL(m, "	macid[%d] core_current_txreq_cnt: %d\n",
				psta->phl_sta->macid, psta->core_current_txreq_cnt);
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	//RTW_PRINT("\n");
#endif

	RTW_PRINT_SEL(m, "======= [os tx] =======: \n");
	RTW_PRINT_SEL(m, "	total os_tx pkts / drop: %lld /  %lld\n", pxmitpriv->os_tx_pkts, pxmitpriv->os_tx_drop);
#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		if (psta->os_tx_cnt > 0 || psta->os_tx_drop_cnt > 0)
		RTW_PRINT_SEL(m, "	macid[%d] os_tx_cnt: %d (drop: %d) ==> %d\n",
				psta->phl_sta->macid, psta->os_tx_cnt, psta->os_tx_drop_cnt,
				(psta->os_tx_cnt - psta->os_tx_drop_cnt));
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	//RTW_PRINT("\n");
#endif

	RTW_PRINT_SEL(m, "======= [core tx] =======: \n");
	RTW_PRINT_SEL(m, "	core_tx pkts / drop: %d / %d\n", pxmitpriv->core_tx_pkts, pxmitpriv->core_tx_drop);
	for(i = 0; i < 10; i++)
		if(pxmitpriv->core_tx_abort[i] != 0)
			RTW_PRINT_SEL(m, "	core_tx_abort[%d]: %d\n", i, pxmitpriv->core_tx_abort[i]);

#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
	if (!pxmitpriv->txsc_enable) {
		RTW_PRINT_SEL(m, "[slow path][core_xframe_macid]:	netif_drop_thd:%d\n", (nr_xmitframe_cnt /((adapter->stapriv.asoc_sta_count-1)?:1)));
		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
		plist = get_next(phead);
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);
			RTW_PRINT_SEL(m, "	macid[%d] core_xframe_total_cnt: %d (abort:%d)\n",
					psta->phl_sta->macid, psta->core_xframe_total_cnt, psta->core_xframe_abort);
		}
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
		//RTW_PRINT("\n");
	} else {
		RTW_PRINT_SEL(m, "[txsc][core_txsc_apply_macid]:	netif_drop_thd:%d\n", (adapter->max_tx_ring_cnt/((adapter->stapriv.asoc_sta_count-1)?:1)));
		RTW_PRINT_SEL(m, "	mid[x] txsc_apply - (no_txreq[abort6] + fc abort + fail)\n	(amsdu need_enq/need_deq/abort[abort5]/amsdu to/amsdu deq) \n");
		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
		plist = get_next(phead);
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);
			RTW_PRINT_SEL(m, "	macid[%d] txsc_apply_cnt: (%d + %d) - (%d + %d + %d)\n		(amsdu need_enq/need_deq/abort/to/deq: %d, %d, %d, %d, %d)\n",
				psta->phl_sta->macid, psta->core_txsc_apply_cnt_1, psta->core_txsc_apply_cnt_2,
				psta->core_txsc_apply_no_txreq, psta->core_txreq_abort, psta->core_txsc_apply_fail,
				psta->core_txsc_amsdu_need_enq, psta->core_txsc_amsdu_need_deq, psta->core_txsc_amsdu_abort,
				psta->core_txsc_amsdu_timeout, psta->core_txsc_amsdu_deq);
		}
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
		//RTW_PRINT("\n");
	}


	RTW_PRINT_SEL(m, "======= [add into phl tx tring] =======:\n");
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		RTW_PRINT_SEL(m, "	macid[%d] add txreq ok / fail: %d, %d\n",
			psta->phl_sta->macid, psta->phl_sta->add_tx_ring_ok, psta->phl_sta->add_tx_ring_fail);
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	//RTW_PRINT("\n");


	RTW_PRINT_SEL(m, "======= [enquque WD pending] =======:\n");
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		RTW_PRINT_SEL(m, "	macid[%d] first_deq_cnt: %d, enquque ok/fail: %d, %d\n",
			psta->phl_sta->macid, psta->phl_sta->phl_tx_ring_start_cnt,
			psta->phl_sta->enq_wd_pending_ok, psta->phl_sta->enq_wd_pending_fail);
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, " update_txbd ok / fail: %d / %d\n", phl_com->update_txbd_ok, phl_com->update_txbd_fail);
	RTW_PRINT_SEL(m, " trigger_txstart ok / fail: %d / %d\n", phl_com->trigger_txstart_ok, phl_com->trigger_txstart_fail);
	RTW_PRINT_SEL(m, "\n");

	rtw_phl_dump_tx_ring(phl, m);
	//RTW_PRINT("\n");

	rtw_phl_dump_wd_balance_status(phl, m);
	//RTW_PRINT("\n");

	phl_get_wd_ring_wd_page_cnt(phl, m);
	//RTW_PRINT("\n");

	RTW_PRINT_SEL(m, "=============== MAC reg ===============\n");

	SHOW_REG32_MSG(adapter, R_AX_ACH0_PAGE_INFO, "ACH0 PG INFO", m);

	SHOW_REG32_MSG(adapter, R_AX_ACH1_PAGE_INFO, "ACH1 PG INFO", m);

	SHOW_REG32_MSG(adapter, R_AX_ACH2_PAGE_INFO, "ACH2 PG INFO", m);

	SHOW_REG32_MSG(adapter, R_AX_ACH3_PAGE_INFO, "ACH3 PG INFO", m);

	Get_macid_WD_by_AC(adapter, 1, 0);
	Get_macid_WD_by_AC(adapter, 2, 0);
	Get_macid_WD_by_AC(adapter, 3, 0);
	Get_macid_WD_by_AC(adapter, 4, 0);
	RTW_PRINT_SEL(m, "\n");

	#if 0
	//rtw_phl_dump_wd_ring_ist(phl, 0, 0);   // wd idle page ring status
	//rtw_phl_dump_wd_ring_ist(phl, 1, 0);   // wd pending page ring status
	//rtw_phl_dump_wd_ring_ist(phl, 2, 0);   // wd busy page ring status
	RTW_PRINT("\n");
	#endif
#endif
}

void rtw_dump_sta_free_list(_adapter *adapter, void *cmd_para, u32 para_num)
{
	_list *plist, *phead;
	struct sta_info *psta;
	struct  sta_priv *pstapriv = &adapter->stapriv;
	_queue *pfree_sta_queue = pstapriv->pfree_sta_queue;
	char free_sta_list[NUM_STA];
	int stainfo_offset = 0;
	u32 idx = 0;
	u8 free_sta_num = 0;

	_rtw_spinlock_bh(&(pfree_sta_queue->lock));
	phead = &(pfree_sta_queue->queue);
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, free_list);
		plist = get_next(plist);

		if (psta->phl_sta) {
			stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
			if (stainfo_offset_valid(stainfo_offset))
				free_sta_list[free_sta_num++] = stainfo_offset;
		}
	}
	_rtw_spinunlock_bh(&(pfree_sta_queue->lock));

	RTW_PRINT(FUNC_ADPT_FMT" free_list, cnt:%u\n"
			, FUNC_ADPT_ARG(adapter), pfree_sta_queue->qlen);

	for (idx = 0; idx < free_sta_num; idx++) {
		psta = rtw_get_stainfo_by_offset(pstapriv, free_sta_list[idx]);
		DBGP("[%d] sta's macaddr:" MAC_FMT ", psta:%p\n",
				idx, MAC_ARG(psta->phl_sta->mac_addr), psta);
	}
}

void rtw_dump_sta_asoc_list(_adapter *adapter, void *cmd_para, u32 para_num)
{
	_list	*phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &adapter->stapriv;
	int stainfo_offset = 0, sta_num = 0, idx = 0;
	char sta_list[NUM_STA];

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	if (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		RTW_PRINT(FUNC_ADPT_FMT" asoc_list, cnt:%u\n"
			, FUNC_ADPT_ARG(adapter), pstapriv->asoc_list_cnt);
	}

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
		if (stainfo_offset_valid(stainfo_offset))
				sta_list[sta_num++] = stainfo_offset;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	for (idx = 0; idx < sta_num; idx++) {
		psta = rtw_get_stainfo_by_offset(pstapriv, sta_list[idx]);
		DBGP("[%d] sta's macaddr:" MAC_FMT " stats:0x%x aid:%d, macid:%d\n",
			idx, MAC_ARG(psta->phl_sta->mac_addr), psta->state,
			psta->phl_sta->aid, psta->phl_sta->macid);
	}
}

void rtw_dump_sta_auth_list(_adapter *adapter, void *cmd_para, u32 para_num)
{
	_list	*phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &adapter->stapriv;
	int stainfo_offset = 0, sta_num = 0, idx = 0;
	char sta_list[NUM_STA];

	_rtw_spinlock_bh(&pstapriv->auth_list_lock);
	phead = &pstapriv->auth_list;
	plist = get_next(phead);

	if (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		RTW_PRINT(FUNC_ADPT_FMT" auth_list, cnt:%u\n"
			, FUNC_ADPT_ARG(adapter), pstapriv->auth_list_cnt);
	}

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, auth_list);
		plist = get_next(plist);
		stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
		if (stainfo_offset_valid(stainfo_offset))
				sta_list[sta_num++] = stainfo_offset;

	}
	_rtw_spinunlock_bh(&pstapriv->auth_list_lock);

	for (idx = 0; idx < sta_num; idx++) {
		psta = rtw_get_stainfo_by_offset(pstapriv, sta_list[idx]);
		DBGP("[%d] sta's macaddr:" MAC_FMT " stats:0x%x aid:%d, macid:%d\n",
			idx, MAC_ARG(psta->phl_sta->mac_addr), psta->state,
			psta->phl_sta->aid, psta->phl_sta->macid);
	}
}


#ifdef CONFIG_WFA_OFDMA_Logo_Test
#if 0	// Mark.CS_update
void core_cmd_wfa_ru_test(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 *para = (u32 *)cmd_para;
	void *phl = adapter->dvobj->phl;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct phl_info_t *phl_info = (struct phl_info_t *)(dvobj->phl);

	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);

	struct rtw_phl_ax_ul_fixinfo tbl_b;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &adapter->stapriv;
	u32 macid_0, macid_1, pstatus, pstatus_fix;
	_list	*phead, *plist;
	int i=0;

	_rtw_memset(&tbl_b, 0, sizeof(struct rtw_phl_ax_ul_fixinfo));


	if (para[0] == 99){  //debug cmd mode

		phead = &pstapriv->asoc_list;
		plist = get_next(phead);

		tbl_b.ulrua.sta[0].coding=1;
		tbl_b.ulrua.sta[1].coding=1;
		tbl_b.ulrua.sta[2].coding=1;
		tbl_b.ulrua.sta[3].coding=1;

		if (para_num != 22){
			DBGP(">>>>>>>>>para_num %d\n",para_num);
			DBGP("[Invalid cmd!!!] wrong input parameter >>>> para_num= %d\n",para_num);
			return;
		}

	/*
	para[0] = 99 // ID
	para[1] = APEP_Length
	para[2] = UL BW
	para[3] = STBC
	para[4] = RU Allocation[0] //user0
	para[5] = RU Allocation[1] //user1
	para[6] = RU Allocation[2] //user2
	para[7] = RU Allocation[3] //user3
	para[8] = FEC coding
	para[9] = MCS
	para[10] = DCM
	para[11] = SS Allocation
	para[12] = SS Allocation
	para[13] = SS Allocation
	para[14] = SS Allocation
	para[15] = UL Target RSSI
	para[16] = UL Target RSSI
	para[17] = UL Target RSSI
	para[18] = UL Target RSSI
	para[19] = trigger frame's data_rate
	para[20] = rf_gain_fix
	para[21] = rf_gain_idx

	iwpriv wlan0 phl_test wfa_ru_test,99,APEP_Length,UL BW,STBC,RU Allocation[0],RU Allocation[1],RU Allocation[2],RU Allocation[3],FEC coding,MCS,DCM,SS Allocation,UL Target RSSI
	iwpriv wlan0 phl_test wfa_ru_test,99,2,2,0,61,62,63,64,1,0,0,0,60,60,60,60,8,0,0

	*/
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);
			if ((psta->hepriv.he_option==0) || (pmlmepriv->hepriv.he_option==0))
			  continue;
			DBGP("macid %d , aid %d  mac_addr %x:%x:%x:%x:%x:%x\n",psta->phl_sta->macid,psta->phl_sta->aid, psta->phl_sta->mac_addr[0], psta->phl_sta->mac_addr[1], psta->phl_sta->mac_addr[2], psta->phl_sta->mac_addr[3], psta->phl_sta->mac_addr[4], psta->phl_sta->mac_addr[5]);

			tbl_b.sta[i].macid = psta->phl_sta->macid;
			tbl_b.sta[i].ul_mu_dis= psta->phl_sta->ul_mu_disable;
			tbl_b.sta[i].ul_mu_dis= 0;
			//tbl_b.ulrua.sta[i].tgt_rssi = para[12];
	    	tbl_b.ulrua.sta[i].mac_id = psta->phl_sta->macid;
	    	tbl_b.ulrua.sta[i].coding = para[8]; //LDPC
	    	tbl_b.ulrua.sta[i].rate.dcm = para[10];
			tbl_b.ulrua.sta[i].rate.mcs = para[9];   //fix mcs0

			i++;
		}

		tbl_b.ulrua.sta_num = i;
		if (tbl_b.ulrua.sta_num==0)
			return;

		tbl_b.tf_type = 0;
	    tbl_b.gi_ltf = 0;
	    tbl_b.data_rate = para[19]; //24M
	    tbl_b.data_bw = MAC_AX_BW_20M; //Trigger frame's BW
	    tbl_b.data_ldpc = 0;
	    tbl_b.apep_len = para[1]; //UL length 24x64 byte
	    tbl_b.more_tf= 0;
		tbl_b.ulrua.ppdu_bw = para[2];
	    tbl_b.ulrua.gi_ltf = 0;
	    tbl_b.ulrua.tb_t_pe_nom = 2;

		tbl_b.ulrua.sta[0].rate.ss = para[11];
		tbl_b.ulrua.sta[1].rate.ss = para[12];
		tbl_b.ulrua.sta[2].rate.ss = para[13];
		tbl_b.ulrua.sta[3].rate.ss = para[14];
		tbl_b.store_idx= 0;
		tbl_b.ulfix_usage= 3;
		tbl_b.cfg.storemode = 1;
		if (tbl_b.ulrua.sta[0].rate.ss || tbl_b.ulrua.sta[1].rate.ss ||tbl_b.ulrua.sta[2].rate.ss ||tbl_b.ulrua.sta[3].rate.ss){
			tbl_b.ulrua.n_ltf_and_ma = 1; //<=4 or !=0
		}else{
			tbl_b.ulrua.n_ltf_and_ma = 0; //<=4 or !=0
		}

	    tbl_b.ulrua.grp_id = 0;
	    tbl_b.ulrua.fix_mode = 1;
	    tbl_b.ulrua.doppler = 0;
	    tbl_b.ulrua.stbc = para[3];
		tbl_b.ulrua.ru2su= 0;
	    tbl_b.ulrua.rf_gain_fix = para[20];
	    tbl_b.ulrua.rf_gain_idx = para[21];

		tbl_b.ulrua.sta[0].ru_pos = para[4] << 1;
		tbl_b.ulrua.sta[1].ru_pos = para[5] << 1;
		tbl_b.ulrua.sta[2].ru_pos = para[6] << 1;
		tbl_b.ulrua.sta[3].ru_pos = para[7] << 1;

		tbl_b.ulrua.sta[0].tgt_rssi = para[15];
		tbl_b.ulrua.sta[1].tgt_rssi = para[16];
		tbl_b.ulrua.sta[2].tgt_rssi = para[17];
		tbl_b.ulrua.sta[3].tgt_rssi = para[18];

		rtw_phl_mac_set_upd_ul_fixinfo(phl, &tbl_b);
	}
	else if (para[0] == 1){
		phead = &pstapriv->asoc_list;
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);
			if ((psta->hepriv.he_option==0) || (pmlmepriv->hepriv.he_option==0))
			  continue;
			DBGP("macid %d , aid %d  mac_addr %x:%x:%x:%x:%x:%x\n",psta->phl_sta->macid,psta->phl_sta->aid, psta->phl_sta->mac_addr[0], psta->phl_sta->mac_addr[1], psta->phl_sta->mac_addr[2], psta->phl_sta->mac_addr[3], psta->phl_sta->mac_addr[4], psta->phl_sta->mac_addr[5]);


			tbl_b.sta[i].macid = psta->phl_sta->macid;
			tbl_b.ulrua.sta[i].tgt_rssi = 70;
	    	tbl_b.ulrua.sta[i].mac_id = psta->phl_sta->macid;

	    	if (psta->phl_sta->asoc_cap.he_ldpc)
	    		tbl_b.ulrua.sta[i].coding = 1; //LDPC
	    	else
				tbl_b.ulrua.sta[i].coding = 0; //BCC


	    	tbl_b.ulrua.sta[i].rate.dcm = 0;//psta->phl_sta->dcm_en;

			if ((psta->phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC11)
				tbl_b.ulrua.sta[i].rate.mcs = 11;
			else if((psta->phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC9)
				tbl_b.ulrua.sta[i].rate.mcs = 9;
			else
				tbl_b.ulrua.sta[i].rate.mcs = 7;

			tbl_b.ulrua.sta[i].rate.ss = psta->phl_sta->asoc_cap.nss_rx-1;

			i++;
			if (i>4){
				DBGP("Only support 4 sta for UL-OFDMA temporarily\n");
				break;
		}

		}
		tbl_b.ulrua.sta_num = i;
		if (tbl_b.ulrua.sta_num==0)
			return;

		tbl_b.tf_type = 0;
	    tbl_b.gi_ltf = 0;
	    tbl_b.data_rate = 0x8; //24M
	    tbl_b.data_bw = 0; //Trigger frame's BW 20M
	    tbl_b.data_ldpc = 0;

		//if (psta->phl_sta->chandef.band== BAND_ON_24G)
	    	//tbl_b.apep_len = 150; //UL length 24x64 byte
	    //else
	    	//tbl_b.apep_len = 200; //UL length 24x64 byte


	    tbl_b.more_tf= 0;
		tbl_b.ulrua.ppdu_bw = psta->phl_sta->chandef.bw;


	    tbl_b.apep_len = 800;
	    tbl_b.ulrua.gi_ltf = 0;
	    tbl_b.ulrua.tb_t_pe_nom = 2;

		tbl_b.store_idx= 0;
		tbl_b.ulfix_usage= 3;
		tbl_b.cfg.storemode = 1;
		if (tbl_b.ulrua.sta[0].rate.ss || tbl_b.ulrua.sta[1].rate.ss ||tbl_b.ulrua.sta[2].rate.ss ||tbl_b.ulrua.sta[3].rate.ss){
			tbl_b.ulrua.n_ltf_and_ma = 1; //<=4 or !=0
		}else{
			tbl_b.ulrua.n_ltf_and_ma = 0; //<=4 or !=0
		}

	    tbl_b.ulrua.grp_id = 0;
	    tbl_b.ulrua.fix_mode = 1;
	    tbl_b.ulrua.doppler = 0;
	    tbl_b.ulrua.stbc = 0;
		tbl_b.ulrua.ru2su= 0;
	    tbl_b.ulrua.rf_gain_fix = 0;
	    tbl_b.ulrua.rf_gain_idx = 0;

		if (tbl_b.ulrua.ppdu_bw == CHANNEL_WIDTH_80) { //80M
			if (tbl_b.ulrua.sta_num ==1){
				tbl_b.ulrua.sta[0].ru_pos = 67 << 1;
				tbl_b.ulrua.sta[1].ru_pos = 0 << 1;
				tbl_b.ulrua.sta[2].ru_pos = 0 << 1;
				tbl_b.ulrua.sta[3].ru_pos = 0 << 1;
			}else if (tbl_b.ulrua.sta_num ==2){
				tbl_b.ulrua.sta[0].ru_pos = 65 << 1;
				tbl_b.ulrua.sta[1].ru_pos = 66 << 1;
				tbl_b.ulrua.sta[2].ru_pos = 0 << 1;
				tbl_b.ulrua.sta[3].ru_pos = 0 << 1;
			}else if (tbl_b.ulrua.sta_num ==4){
			/*5G 4sta 2x2 LDPC workaround*/

				tbl_b.ulrua.sta[0].ru_pos = 61 << 1;
				tbl_b.ulrua.sta[1].ru_pos = 62 << 1;
				tbl_b.ulrua.sta[2].ru_pos = 63 << 1;
				tbl_b.ulrua.sta[3].ru_pos = 64 << 1;


				if (tbl_b.ulrua.sta[0].rate.ss ==0 ||
					tbl_b.ulrua.sta[1].rate.ss ==0 ||
					tbl_b.ulrua.sta[2].rate.ss ==0 ||
					tbl_b.ulrua.sta[3].rate.ss ==0){
					tbl_b.apep_len = 600;
				}

			}

		}else if(tbl_b.ulrua.ppdu_bw == CHANNEL_WIDTH_20) { //20M
			if (tbl_b.ulrua.sta_num ==1){
				tbl_b.ulrua.sta[0].ru_pos = 61 << 1;
				tbl_b.ulrua.sta[1].ru_pos = 0 << 1;
				tbl_b.ulrua.sta[2].ru_pos = 0 << 1;
				tbl_b.ulrua.sta[3].ru_pos = 0 << 1;

				tbl_b.apep_len = 700;
			}else if (tbl_b.ulrua.sta_num ==2){
				tbl_b.ulrua.sta[0].ru_pos = 53 << 1;
				tbl_b.ulrua.sta[1].ru_pos = 54 << 1;
				tbl_b.ulrua.sta[2].ru_pos = 0 << 1;
				tbl_b.ulrua.sta[3].ru_pos = 0 << 1;

				tbl_b.apep_len = 230;
			}else if (tbl_b.ulrua.sta_num ==4){
				/*2G 4sta 2x2 BCC workaround*/
				if ((psta->phl_sta->chandef.band== BAND_ON_24G) &&
					(tbl_b.ulrua.sta[0].coding ==0)&&
					(tbl_b.ulrua.sta[1].coding ==0)&&
					(tbl_b.ulrua.sta[2].coding ==0)&&
					(tbl_b.ulrua.sta[3].coding ==0)&&
					((tbl_b.ulrua.sta[0].rate.ss ==1)||
					(tbl_b.ulrua.sta[1].rate.ss ==1)||
					(tbl_b.ulrua.sta[2].rate.ss ==1)||
					(tbl_b.ulrua.sta[3].rate.ss ==1))){

					tbl_b.ulrua.sta[0].ru_pos = 53 << 1;
					tbl_b.ulrua.sta[1].ru_pos = 39 << 1;
					tbl_b.ulrua.sta[2].ru_pos = 7 << 1;
					tbl_b.ulrua.sta[3].ru_pos = 8 << 1;

					tbl_b.apep_len = 150;

				}else{
					tbl_b.ulrua.sta[0].ru_pos = 37 << 1;
					tbl_b.ulrua.sta[1].ru_pos = 38 << 1;
					tbl_b.ulrua.sta[2].ru_pos = 39 << 1;
					tbl_b.ulrua.sta[3].ru_pos = 40 << 1;

					tbl_b.apep_len = 150;
				}
			}
		}
		rtw_phl_mac_set_upd_ul_fixinfo(phl, &tbl_b);

	}
	else if (para[0] == 2){
		phead = &pstapriv->asoc_list;
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);
			if ((psta->hepriv.he_option==0) || (pmlmepriv->hepriv.he_option==0))
			  continue;
			DBGP("macid %d , aid %d  mac_addr %x:%x:%x:%x:%x:%x\n",psta->phl_sta->macid,psta->phl_sta->aid, psta->phl_sta->mac_addr[0], psta->phl_sta->mac_addr[1], psta->phl_sta->mac_addr[2], psta->phl_sta->mac_addr[3], psta->phl_sta->mac_addr[4], psta->phl_sta->mac_addr[5]);


			tbl_b.sta[i].macid = psta->phl_sta->macid;
	    	tbl_b.ulrua.sta[i].mac_id = psta->phl_sta->macid;
	    	tbl_b.ulrua.sta[i].coding = 0;
			tbl_b.ulrua.sta[i].rate.mcs = 7;
			//tbl_b.ulrua.sta[i].rate.mcs = 0;
	    	tbl_b.ulrua.sta[i].rate.ss = 0;
			tbl_b.ulrua.sta[i].ru_pos = 0x4a + i*2;
			tbl_b.ulrua.sta[i].tgt_rssi = 70;

			i++;
			if (i>4){
				DBGP("Only support 4 sta for UL-OFDMA temporarily\n");
				break;
			}

		}

		tbl_b.ulrua.sta_num = i;
		//if (tbl_b.ulrua.sta_num	< 1){
		//	DBGP("less than two sta, cna't do 2RU UL-OFDMA \n");
		//	return;
		//}

		//cfg
		tbl_b.cfg.mode = 0x2;
		tbl_b.cfg.storemode = 0x2;
		tbl_b.store_idx = 0x0;
		tbl_b.ulfix_usage = 0x3;
		tbl_b.cfg.interval = 60; //microseconds

		// tf
		tbl_b.data_rate = 0x8;
		tbl_b.data_bw = 0x0;
		tbl_b.gi_ltf = 0x0;
		tbl_b.tf_type = 0x1;

		// common
		//tbl_b.ulrua.sta_num = 0x2;
		tbl_b.ulrua.gi_ltf = 0x0;
		tbl_b.ulrua.n_ltf_and_ma = 0x0;
		tbl_b.ulrua.ppdu_bw = 0x0;
		tbl_b.apep_len = 0x20;

		rtw_phl_mac_set_upd_ul_fixinfo(phl, &tbl_b);

	}
}

void core_cmd_fw_tx(_adapter *adapter, void *cmd_para, u32 para_num){
	u32 *para = (u32 *)cmd_para;
	void *phl = adapter->dvobj->phl;

	u32 tmp8, tmp16, tmp32;

	if(para_num < 1){
		DBGP("please enter cmd : fw_tx,<0/1> to disable/enable FW tx (default is disable)\n");
		return;
	}

	if(para[0]){
#if 0
		rtw_phl_write32(phl, 0x0c04, 0x18840000);
		tmp32 = rtw_phl_read32(phl, 0x40000);
		tmp32 = tmp32 | BIT20 | BIT22 | BIT23;
		DBGP("18840000 set 0x40000 to 0x%04x\n", tmp32);
		rtw_phl_write32(phl, 0x40000, tmp32);

		rtw_phl_write32(phl, 0x0c04, 0x18840020);
		tmp32 = rtw_phl_read32(phl, 0x40000);
		tmp32 = tmp32 | BIT20 | BIT22 | BIT23;
		DBGP("18840020 set 0x40000 to 0x%04x\n", tmp32);
		rtw_phl_write32(phl, 0x40000, tmp32);

		rtw_phl_write32(phl, 0x0c04, 0x18840040);
		tmp32 = rtw_phl_read32(phl, 0x40000);
		tmp32 = tmp32 | BIT20 | BIT22 | BIT23;
		DBGP("18840040 set 0x40000 to 0x%04x\n", tmp32);
		rtw_phl_write32(phl, 0x40000, tmp32);

		DBGP("c600: 0x%0x  9e13: 0x%0x  9e48: 0x%04x\n",
			rtw_phl_read8(phl, 0xc600), rtw_phl_read8(phl, 0x9e13), rtw_phl_read32(phl, 0x9e48));

		rtw_phl_write8(phl, 0xc600, 0x3c);
		rtw_phl_write8(phl, 0x9e13, 0x90);
		rtw_phl_write32(phl, 0x9e48, 0x800200ff);

		DBGP("after setting:\n");
		DBGP("c600: 0x%0x  9e13: 0x%0x  9e48: 0x%04x\n",
			rtw_phl_read8(phl, 0xc600), rtw_phl_read8(phl, 0x9e13), rtw_phl_read32(phl, 0x9e48));
#else
		rtw_phl_write8(phl, 0xc600, 0x3c);
		rtw_phl_write8(phl, 0x9e13, 0x90);
		rtw_phl_write16(phl, 0x9e1c, 0xffff);
		rtw_phl_write8(phl, 0xce20, 0x2e);
		rtw_phl_write32(phl, 0xc624, 0x4001010);
		rtw_phl_write32(phl, 0xc648, 0x0);
		rtw_phl_write32(phl, 0x9e48, 0x800200ff);
		rtw_phl_write32(phl, 0x9e48, 0x810200ff);
		rtw_phl_write32(phl, 0x9e48, 0x820200ff);
		rtw_phl_write32(phl, 0x9e48, 0x830200ff);
#endif
	}
	else {
		rtw_phl_write8(phl, 0xc600, 0x3);

		DBGP("after setting:\n");
		DBGP("c600: 0x%0x  9e13: 0x%0x  9e48: 0x%04x\n",
			rtw_phl_read8(phl, 0xc600), rtw_phl_read8(phl, 0x9e13), rtw_phl_read32(phl, 0x9e48));
	}

}
#endif

// iwpriv wlan0 phl_test ru_para,ru_common,<para>,<value>
// iwpriv wlan0 phl_test ru_para,dl_rp,<para>,<value>
// iwpriv wlan0 phl_test ru_para,dl_fix_grp,<para>,<value>
// iwpriv wlan0 phl_test ru_para,ul_grp,<para>,<value>
// iwpriv wlan0 phl_test ru_para,ul_fix_grp,<para>,<value>
void core_cmd_ru_para(_adapter *adapter, void *cmd_para, u32 para_num){

	u32 idx = 0, value = 0;
	int i=0;
	void *phl = adapter->dvobj->phl;

	char *para = (char *)cmd_para;

	DBGP("\n");

	if(para_num<=0){
		DBGP("please enter cmd : ru_para,<ru_common>,<para>,<value>\n");
		DBGP("please enter cmd : ru_para,<dl_grp>,<para>,<value>\n");
		DBGP("please enter cmd : ru_para,<dl_fix_grp>,<para>,<value>\n");
		DBGP("please enter cmd : ru_para,<ul_grp>,<para>,<value>\n");
		DBGP("please enter cmd : ru_para,<ul_fix_grp>,<para>,<value>\n");
		DBGP("please enter cmd : ru_para,<ulmacid_cfg>,<para>,<value>\n");
		return;
	}

	if(!strcmp(para, "ru_common")){
		para=get_next_para_str(para);
		_rtw_set_ru_common(adapter, para, para_num-1);
		return;
	}
	else if (!strcmp(para, "dl_grp")){
		para=get_next_para_str(para);
		_rtw_set_dl_grp(adapter, para, para_num-1);
		return;
	}else if (!strcmp(para, "dl_fix_grp")){
		para=get_next_para_str(para);
		_rtw_set_dl_fix_grp(adapter, para, para_num-1);
		return;
	}else if (!strcmp(para, "ul_grp")){
		para=get_next_para_str(para);
		_rtw_set_ul_grp(adapter, para, para_num-1);
		return;
	}else if (!strcmp(para, "ul_fix_grp")){
		para=get_next_para_str(para);
		_rtw_set_ul_fix_grp(adapter, para, para_num-1);
		return;
	}else if (!strcmp(para, "ulmacid_cfg")){
		para=get_next_para_str(para);
		_rtw_set_ulmacid_cfg(adapter, para, para_num-1);
		return;
	}

	return;
}

void rtw_dump_ofdma_group(_adapter *padapter)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;

	struct sta_info *psta;
	int i;

	printk("nr_ru_sta:%d\n", padapter->ru_info.nr_ru_sta);

	for (i = 0; i < padapter->ru_info.nr_ru_sta; i++) {
		psta = padapter->ru_info.psta_ru[i];
		if (psta && psta->phl_sta)
			printk("psta[%d]: macid:%d, mac_addr:"MAC_FMT", nss:%d\n",
				i, psta->phl_sta->macid, MAC_ARG(psta->phl_sta->mac_addr), dl_ru_fix_grp->sta_info[i].ss);
	}
}

void core_cmd_dump_ring(_adapter *adapter, void *cmd_para, u32 para_num){
	u32 *para = (u32 *)cmd_para;
	void *phl = adapter->dvobj->phl;

	rtw_phl_dump_tx_ring(phl, RTW_DBGDUMP);

}

void rtw_dump_sta_hash(_adapter *adapter, void *cmd_para, u32 para_num)
{

	struct sta_priv *pstapriv = &adapter->stapriv;

	_list *plist, *phead;

	struct sta_info *psta = NULL;

	u32	i;

	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);
	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

			plist = get_next(plist);
			DBGP("==============================\n");
			DBGP("sta's macaddr:" MAC_FMT "\n", MAC_ARG(psta->phl_sta->mac_addr));
			DBGP("state=0x%x, aid=%d, macid=%d\n",
				psta->state, psta->phl_sta->aid, psta->phl_sta->macid);
		}
	}
	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

}

void rtw_dump_sta(_adapter *adapter, void *cmd_para, u32 para_num)
{

	struct sta_priv *pstapriv = &adapter->stapriv;

	_list *plist, *phead;
	struct sta_info *psta = NULL;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {

		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		DBGP("psta: macid:%d, mac_addr:"MAC_FMT" (state:0x%x, last_rx:%u)\n",
			psta->phl_sta->macid, MAC_ARG(psta->phl_sta->mac_addr),
			psta->state,
			rtw_get_passing_time_ms(psta->sta_stats.last_rx_time));

		plist = get_next(plist);
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
}

void default_ul_grp_setting (_adapter *adapter){

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	void *phl = adapter->dvobj->phl;
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct ul_ru_grp_table_para *ul_ru_grp = &rugrptable->ul_ru_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	ul_ru_fix_grp->max_sta_num = 2;
	ul_ru_fix_grp->min_sta_num = 2;

	ul_ru_fix_grp->gi_ltf = 0;
	ul_ru_fix_grp->fixru_flag = 1;

	phl_bw80_init_8ru_pos(phl);

	ul_ru_fix_grp->sta_info[0].mcs = 4;
	ul_ru_fix_grp->sta_info[0].ss = 0;
	ul_ru_fix_grp->sta_info[0].fix_rate = 1;
	ul_ru_fix_grp->sta_info[0].coding = 1;
	ul_ru_fix_grp->sta_info[0].tgt_rssi[0] = 43;
	ul_ru_fix_grp->sta_info[0].tgt_rssi[1] = 40;
	ul_ru_fix_grp->sta_info[0].tgt_rssi[2] = 40;

	ul_ru_fix_grp->sta_info[1].mcs = 4;
	ul_ru_fix_grp->sta_info[1].ss = 0;
	ul_ru_fix_grp->sta_info[1].fix_rate = 1;
	ul_ru_fix_grp->sta_info[1].coding = 1;
	ul_ru_fix_grp->sta_info[1].tgt_rssi[0] = 43;
	ul_ru_fix_grp->sta_info[1].tgt_rssi[1] = 40;
	ul_ru_fix_grp->sta_info[1].tgt_rssi[2] = 40;

	ul_ru_fix_grp->sta_info[2].mcs = 4;
	ul_ru_fix_grp->sta_info[2].ss = 0;
	ul_ru_fix_grp->sta_info[2].fix_rate = 1;
	ul_ru_fix_grp->sta_info[2].coding = 1;
	ul_ru_fix_grp->sta_info[2].tgt_rssi[0] = 40;
	ul_ru_fix_grp->sta_info[2].tgt_rssi[1] = 40;
	ul_ru_fix_grp->sta_info[2].tgt_rssi[2] = 40;

	ul_ru_fix_grp->sta_info[3].mcs = 4;
	ul_ru_fix_grp->sta_info[3].ss = 0;
	ul_ru_fix_grp->sta_info[3].fix_rate = 1;
	ul_ru_fix_grp->sta_info[3].coding = 1;
	ul_ru_fix_grp->sta_info[3].tgt_rssi[0] = 0;
	ul_ru_fix_grp->sta_info[3].tgt_rssi[1] = 0;
	ul_ru_fix_grp->sta_info[3].tgt_rssi[2] = 40;

}

void default_5g_dl_grp_logo_setting(_adapter *adapter){

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	void *phl = adapter->dvobj->phl;

	/* default */
	ru_ctrl->rotate = 1;

	dl_ru_grp->tx_pwr = 0x3c;
	dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_80;

	dl_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;
	dl_ru_fix_grp->rupos_csht_flag = 0;
	dl_ru_fix_grp->ru_swp_flg = 0;

	dl_ru_fix_grp->sta_info[0].fix_rate = 1;
	dl_ru_fix_grp->sta_info[1].fix_rate = 1;
	dl_ru_fix_grp->sta_info[2].fix_rate = 1;
	dl_ru_fix_grp->sta_info[3].fix_rate = 1;
	dl_ru_fix_grp->sta_info[4].fix_rate = 1;
	dl_ru_fix_grp->sta_info[5].fix_rate = 1;
	dl_ru_fix_grp->sta_info[6].fix_rate = 1;
	dl_ru_fix_grp->sta_info[7].fix_rate = 1;

	dl_ru_fix_grp->sta_info[0].coding = 0;
	dl_ru_fix_grp->sta_info[1].coding = 0;
	dl_ru_fix_grp->sta_info[2].coding = 0;
	dl_ru_fix_grp->sta_info[3].coding = 0;
	dl_ru_fix_grp->sta_info[4].coding = 0;
	dl_ru_fix_grp->sta_info[5].coding = 0;
	dl_ru_fix_grp->sta_info[6].coding = 0;
	dl_ru_fix_grp->sta_info[7].coding = 0;

	dl_ru_fix_grp->sta_info[0].ss = 0;
	dl_ru_fix_grp->sta_info[1].ss = 0;
	dl_ru_fix_grp->sta_info[2].ss = 0;
	dl_ru_fix_grp->sta_info[3].ss = 0;
	dl_ru_fix_grp->sta_info[4].ss = 0;
	dl_ru_fix_grp->sta_info[5].ss = 0;
	dl_ru_fix_grp->sta_info[6].ss = 0;
	dl_ru_fix_grp->sta_info[7].ss = 0;

	dl_ru_fix_grp->sta_info[0].mcs = 7;
	dl_ru_fix_grp->sta_info[1].mcs = 7;
	dl_ru_fix_grp->sta_info[2].mcs = 7;
	dl_ru_fix_grp->sta_info[3].mcs = 7;
	dl_ru_fix_grp->sta_info[4].mcs = 7;
	dl_ru_fix_grp->sta_info[5].mcs = 7;
	dl_ru_fix_grp->sta_info[6].mcs = 7;
	dl_ru_fix_grp->sta_info[7].mcs = 7;

	phl_bw80_init_8ru_pos(phl);
}

void default_2g_dl_grp_logo_setting(_adapter *adapter){

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;

	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;

	void *phl = adapter->dvobj->phl;

	/* default */
	ru_ctrl->rotate = 1;

	dl_ru_grp->tx_pwr = 0x3c;
	dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_20;

	dl_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;
	dl_ru_fix_grp->rupos_csht_flag = 1;
	dl_ru_fix_grp->ru_swp_flg = 1;

	dl_ru_fix_grp->sta_info[0].fix_rate = 1;
	dl_ru_fix_grp->sta_info[1].fix_rate = 1;
	dl_ru_fix_grp->sta_info[2].fix_rate = 1;
	dl_ru_fix_grp->sta_info[3].fix_rate = 1;
	dl_ru_fix_grp->sta_info[4].fix_rate = 1;
	dl_ru_fix_grp->sta_info[5].fix_rate = 1;
	dl_ru_fix_grp->sta_info[6].fix_rate = 1;
	dl_ru_fix_grp->sta_info[7].fix_rate = 1;

	dl_ru_fix_grp->sta_info[0].coding = 0;
	dl_ru_fix_grp->sta_info[1].coding = 0;
	dl_ru_fix_grp->sta_info[2].coding = 0;
	dl_ru_fix_grp->sta_info[3].coding = 0;
	dl_ru_fix_grp->sta_info[4].coding = 0;
	dl_ru_fix_grp->sta_info[5].coding = 0;
	dl_ru_fix_grp->sta_info[6].coding = 0;
	dl_ru_fix_grp->sta_info[7].coding = 0;

	dl_ru_fix_grp->sta_info[0].ss = 0;
	dl_ru_fix_grp->sta_info[1].ss = 0;
	dl_ru_fix_grp->sta_info[2].ss = 0;
	dl_ru_fix_grp->sta_info[3].ss = 0;
	dl_ru_fix_grp->sta_info[4].ss = 0;
	dl_ru_fix_grp->sta_info[5].ss = 0;
	dl_ru_fix_grp->sta_info[6].ss = 0;
	dl_ru_fix_grp->sta_info[7].ss = 0;

	dl_ru_fix_grp->sta_info[0].mcs = 7;
	dl_ru_fix_grp->sta_info[1].mcs = 7;
	dl_ru_fix_grp->sta_info[2].mcs = 7;
	dl_ru_fix_grp->sta_info[3].mcs = 7;
	dl_ru_fix_grp->sta_info[4].mcs = 7;
	dl_ru_fix_grp->sta_info[5].mcs = 7;
	dl_ru_fix_grp->sta_info[6].mcs = 7;
	dl_ru_fix_grp->sta_info[7].mcs = 7;

	phl_bw20_init_8ru_pos(phl);
}

struct rtw_ru_cfg_list _rtw_ru_cfg_list[] = {
	{"4.29.1_5G"},
	{"4.30.1_5G_1"},
	{"4.30.1_5G_2"},
	{"4.30.1_5G_3"},
	{"4.36.1_5G_1NSS"},
	{"4.36.1_5G_2NSS"},
	{"4.37.1_5G_1NSS"},
	{"4.37.1_5G_2NSS"},
	{"4.69.1_5G_1"},
	{"4.69.1_5G_2"},
	{"DL_BW20"},
	{"DL_BW40"},
	{"4.29.1_2G"},
	{"4.30.1_2G_1"},
	{"4.30.1_2G_2"},
	{"4.30.1_2G_3"},
	{"4.36.1_2G_1NSS"},
	{"4.36.1_2G_2NSS"},
	{"UL_2RU"},
	{"UL_4RU"},
	{"UL_20BW"},
	{"UL_40BW"},
};

void core_cmd_ru_cfg(_adapter *adapter, void *cmd_para, u32 para_num){

	u32 idx = 0, value = 0;
	int i=0;
	char *para = (char *)cmd_para;

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	void *phl = adapter->dvobj->phl;

	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	struct ul_ru_grp_table_para *ul_ru_grp = &rugrptable->ul_ru_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;
	struct rtw_ru_cfg_list *ru_cfg = _rtw_ru_cfg_list;
	u32 array_size = ARRAY_SIZE(_rtw_ru_cfg_list);

	DBGP("core_cmd_ru_cfg\n");

	if(para_num<=0){
		DBGP("please enter cmd : ru_cfg,<test_item>\n");
		DBGP("<test_item>:\n");
		for (i = 0; i<array_size; i++, ru_cfg++) {
			PHL_PRINT("%s\n", ru_cfg->name);
		}
		return;
	}

	// ================ [5G] ================ //
	if(!strcmp(para, "4.29.1_5G")){
		DBGP("cfg 4.29.1_5G APUT \n");
		default_5g_dl_grp_logo_setting(adapter);
	}else if(!strcmp(para, "4.30.1_5G_1")){
		DBGP("cfg 4.30.1_5G_1 APUT \n");
		default_5g_dl_grp_logo_setting(adapter);
		dl_ru_fix_grp->gi_ltf = 3;
	}else if(!strcmp(para, "4.30.1_5G_2")){
		DBGP("cfg 4.30.1_5G_2 APUT \n");
		default_5g_dl_grp_logo_setting(adapter);
		dl_ru_fix_grp->gi_ltf = 2;
	}else if(!strcmp(para, "4.30.1_5G_3")){
		DBGP("cfg 4.30.1_5G_3 APUT \n");
		default_5g_dl_grp_logo_setting(adapter);
		dl_ru_fix_grp->sta_info[0].ss = 0;
		dl_ru_fix_grp->sta_info[1].ss = 0;
		dl_ru_fix_grp->sta_info[2].ss = 1;
		dl_ru_fix_grp->sta_info[3].ss = 1;
	}else if(!strcmp(para, "4.36.1_5G_1NSS")){
		DBGP("cfg 4.36.1_5G_1NSS APUT \n");
		default_5g_dl_grp_logo_setting(adapter);
		dl_ru_grp->tx_pwr = 0x28;
		dl_ru_fix_grp->sta_info[0].mcs = 9;
		dl_ru_fix_grp->sta_info[1].mcs = 9;
		dl_ru_fix_grp->sta_info[2].mcs = 9;
		dl_ru_fix_grp->sta_info[3].mcs = 9;
		dl_ru_fix_grp->sta_info[4].mcs = 9;
		dl_ru_fix_grp->sta_info[5].mcs = 9;
		dl_ru_fix_grp->sta_info[6].mcs = 9;
		dl_ru_fix_grp->sta_info[7].mcs = 9;
	}else if(!strcmp(para, "4.36.1_5G_2NSS")){
		DBGP("cfg 4.36.1_5G_2NSS APUT \n");
		default_5g_dl_grp_logo_setting(adapter);
		dl_ru_grp->tx_pwr = 0x28;
		dl_ru_fix_grp->sta_info[0].ss = 1;
		dl_ru_fix_grp->sta_info[1].ss = 1;
		dl_ru_fix_grp->sta_info[2].ss = 1;
		dl_ru_fix_grp->sta_info[3].ss = 1;
		dl_ru_fix_grp->sta_info[4].ss = 1;
		dl_ru_fix_grp->sta_info[5].ss = 1;
		dl_ru_fix_grp->sta_info[6].ss = 1;
		dl_ru_fix_grp->sta_info[7].ss = 1;

		dl_ru_fix_grp->sta_info[0].mcs = 9;
		dl_ru_fix_grp->sta_info[1].mcs = 9;
		dl_ru_fix_grp->sta_info[2].mcs = 9;
		dl_ru_fix_grp->sta_info[3].mcs = 9;
		dl_ru_fix_grp->sta_info[4].mcs = 9;
		dl_ru_fix_grp->sta_info[5].mcs = 9;
		dl_ru_fix_grp->sta_info[6].mcs = 9;
		dl_ru_fix_grp->sta_info[7].mcs = 9;
	}else if(!strcmp(para, "4.37.1_5G_1NSS")){
		DBGP("cfg 4.37.1_5G_1NSS APUT \n");
		default_5g_dl_grp_logo_setting(adapter);
		dl_ru_grp->tx_pwr = 0x24;
		//dl_ru_grp->tx_pwr = 0x28;
		dl_ru_fix_grp->sta_info[0].coding = 1;
		dl_ru_fix_grp->sta_info[1].coding = 1;
		dl_ru_fix_grp->sta_info[2].coding = 1;
		dl_ru_fix_grp->sta_info[3].coding = 1;
		dl_ru_fix_grp->sta_info[4].coding = 1;
		dl_ru_fix_grp->sta_info[5].coding = 1;
		dl_ru_fix_grp->sta_info[6].coding = 1;
		dl_ru_fix_grp->sta_info[7].coding = 1;

		dl_ru_fix_grp->sta_info[0].mcs = 11;
		dl_ru_fix_grp->sta_info[1].mcs = 11;
		dl_ru_fix_grp->sta_info[2].mcs = 11;
		dl_ru_fix_grp->sta_info[3].mcs = 11;
		dl_ru_fix_grp->sta_info[4].mcs = 11;
		dl_ru_fix_grp->sta_info[5].mcs = 11;
		dl_ru_fix_grp->sta_info[6].mcs = 11;
		dl_ru_fix_grp->sta_info[7].mcs = 11;
	}else if(!strcmp(para, "4.37.1_5G_2NSS")){
		DBGP("cfg 4.37.1_5G_2NSS APUT \n");
		default_5g_dl_grp_logo_setting(adapter);
		dl_ru_grp->tx_pwr = 0x24;
		//dl_ru_grp->tx_pwr = 0x28;
		dl_ru_fix_grp->sta_info[0].ss = 1;
		dl_ru_fix_grp->sta_info[1].ss = 1;
		dl_ru_fix_grp->sta_info[2].ss = 1;
		dl_ru_fix_grp->sta_info[3].ss = 1;
		dl_ru_fix_grp->sta_info[4].ss = 1;
		dl_ru_fix_grp->sta_info[5].ss = 1;
		dl_ru_fix_grp->sta_info[6].ss = 1;
		dl_ru_fix_grp->sta_info[7].ss = 1;

		dl_ru_fix_grp->sta_info[0].coding = 1;
		dl_ru_fix_grp->sta_info[1].coding = 1;
		dl_ru_fix_grp->sta_info[2].coding = 1;
		dl_ru_fix_grp->sta_info[3].coding = 1;
		dl_ru_fix_grp->sta_info[4].coding = 1;
		dl_ru_fix_grp->sta_info[5].coding = 1;
		dl_ru_fix_grp->sta_info[6].coding = 1;
		dl_ru_fix_grp->sta_info[7].coding = 1;

		dl_ru_fix_grp->sta_info[0].mcs = 11;
		dl_ru_fix_grp->sta_info[1].mcs = 11;
		dl_ru_fix_grp->sta_info[2].mcs = 11;
		dl_ru_fix_grp->sta_info[3].mcs = 11;
		dl_ru_fix_grp->sta_info[4].mcs = 11;
		dl_ru_fix_grp->sta_info[5].mcs = 11;
		dl_ru_fix_grp->sta_info[6].mcs = 11;
		dl_ru_fix_grp->sta_info[7].mcs = 11;
	}else if(!strcmp(para, "4.69.1_5G_1")){
		DBGP("cfg 4.69.1_5G_1 APUT \n");
		default_5g_dl_grp_logo_setting(adapter);
		dl_ru_fix_grp->sta_info[0].ru_pos[0] = RTW_HE_RU484_1; //130;
		dl_ru_fix_grp->sta_info[0].ru_pos[1] = RTW_HE_RU242_1; //122;
		dl_ru_fix_grp->sta_info[0].ru_pos[2] = RTW_HE_RU106_2; //108;

		dl_ru_fix_grp->sta_info[1].ru_pos[0] = RTW_HE_RU484_2; //132;
		dl_ru_fix_grp->sta_info[1].ru_pos[1] = RTW_HE_RU242_2; //124;
		dl_ru_fix_grp->sta_info[1].ru_pos[2] = RTW_HE_RU106_4; //112;

		dl_ru_fix_grp->sta_info[2].ru_pos[0] = RTW_HE_RU484_2; //132;
		dl_ru_fix_grp->sta_info[2].ru_pos[1] = RTW_HE_RU242_2; //124;
		dl_ru_fix_grp->sta_info[2].ru_pos[2] = RTW_HE_RU106_6; //116;

		dl_ru_fix_grp->sta_info[3].ru_pos[0] = RTW_HE_RU26_1; //0;
		dl_ru_fix_grp->sta_info[3].ru_pos[1] = RTW_HE_RU26_1; //0;
		dl_ru_fix_grp->sta_info[3].ru_pos[2] = RTW_HE_RU106_8; //120;

		dl_ru_fix_grp->rupos_csht_flag = 0;
		dl_ru_fix_grp->ru_swp_flg = 0;

		ru_ctrl->rotate = 0;
	}else if(!strcmp(para, "4.69.1_5G_2")){
		DBGP("cfg 4.69.1_5G_2 APUT \n");
		default_5g_dl_grp_logo_setting(adapter);
		dl_ru_grp->tx_pwr = 0x34;

		dl_ru_fix_grp->sta_info[0].mcs = 5;
		dl_ru_fix_grp->sta_info[1].mcs = 5;
		dl_ru_fix_grp->sta_info[2].mcs = 5;
		dl_ru_fix_grp->sta_info[3].mcs = 5;

		dl_ru_fix_grp->rupos_csht_flag = 0;
		dl_ru_fix_grp->ru_swp_flg = 0;

		ru_ctrl->rotate = 0;
	}
	else if(!strcmp(para, "DL_BW20")){
		DBGP("cfg DL_BW20 APUT \n");
		//default_5g_dl_grp_logo_setting(adapter);
		dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_20;
		dl_ru_grp->tf.tb_ppdu_bw = CHANNEL_WIDTH_20;
		dl_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;

		phl_bw20_init_8ru_pos(phl);
	}
	else if(!strcmp(para, "DL_BW40")){
		DBGP("cfg DL_BW40 APUT \n");
		//default_5g_dl_grp_logo_setting(adapter);
		dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_40;
		dl_ru_grp->tf.tb_ppdu_bw = CHANNEL_WIDTH_40;
		dl_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;

		phl_bw40_init_8ru_pos(phl);
	}
	// ================ [2G] ================ //
	else if(!strcmp(para, "4.29.1_2G")){
		DBGP("cfg 4.29.1_2G APUT \n");
		default_2g_dl_grp_logo_setting(adapter);
	}else if(!strcmp(para, "4.30.1_2G_1")){
		DBGP("cfg 4.30.1_2G_1 APUT \n");
		default_2g_dl_grp_logo_setting(adapter);
		dl_ru_fix_grp->gi_ltf = 3;
	}else if(!strcmp(para, "4.30.1_2G_2")){
		DBGP("cfg 4.30.1_2G_2 APUT \n");
		default_2g_dl_grp_logo_setting(adapter);
		dl_ru_fix_grp->gi_ltf = 2;
	}else if(!strcmp(para, "4.30.1_2G_3")){
		DBGP("cfg 4.30.1_2G_3 APUT \n");
		default_2g_dl_grp_logo_setting(adapter);
		dl_ru_fix_grp->sta_info[0].ss = 0;
		dl_ru_fix_grp->sta_info[1].ss = 0;
		dl_ru_fix_grp->sta_info[2].ss = 1;
		dl_ru_fix_grp->sta_info[3].ss = 1;
	}else if(!strcmp(para, "4.36.1_2G_1NSS")){
		DBGP("cfg 4.36.1_2G_1NSS APUT \n");
		default_2g_dl_grp_logo_setting(adapter);
		dl_ru_grp->tx_pwr = 0x28;
		dl_ru_fix_grp->sta_info[0].mcs = 9;
		dl_ru_fix_grp->sta_info[1].mcs = 9;
		dl_ru_fix_grp->sta_info[2].mcs = 9;
		dl_ru_fix_grp->sta_info[3].mcs = 9;
		dl_ru_fix_grp->sta_info[4].mcs = 9;
		dl_ru_fix_grp->sta_info[5].mcs = 9;
		dl_ru_fix_grp->sta_info[6].mcs = 9;
		dl_ru_fix_grp->sta_info[7].mcs = 9;
	}else if(!strcmp(para, "4.36.1_2G_2NSS")){
		DBGP("cfg 4.36.1_2G_2NSS APUT \n");
		default_2g_dl_grp_logo_setting(adapter);
		dl_ru_grp->tx_pwr = 0x28;
		dl_ru_fix_grp->sta_info[0].ss = 1;
		dl_ru_fix_grp->sta_info[1].ss = 1;
		dl_ru_fix_grp->sta_info[2].ss = 1;
		dl_ru_fix_grp->sta_info[3].ss = 1;
		dl_ru_fix_grp->sta_info[4].ss = 1;
		dl_ru_fix_grp->sta_info[5].ss = 1;
		dl_ru_fix_grp->sta_info[6].ss = 1;
		dl_ru_fix_grp->sta_info[7].ss = 1;
		dl_ru_fix_grp->sta_info[0].mcs = 9;
		dl_ru_fix_grp->sta_info[1].mcs = 9;
		dl_ru_fix_grp->sta_info[2].mcs = 9;
		dl_ru_fix_grp->sta_info[3].mcs = 9;
		dl_ru_fix_grp->sta_info[4].mcs = 9;
		dl_ru_fix_grp->sta_info[5].mcs = 9;
		dl_ru_fix_grp->sta_info[6].mcs = 9;
		dl_ru_fix_grp->sta_info[7].mcs = 9;
	// ================ [UL] ================ //
	}else if(!strcmp(para, "UL_2RU")){
		DBGP("cfg UL_2RU APUT \n");
		default_ul_grp_setting(adapter);
	}else if(!strcmp(para, "UL_4RU")){
		DBGP("cfg UL_4RU APUT \n");
		default_ul_grp_setting(adapter);
		ul_ru_fix_grp->gi_ltf = 2;
		ul_ru_fix_grp->sta_info[0].tgt_rssi[0] = 72;
		ul_ru_fix_grp->sta_info[0].tgt_rssi[1] = 74;
		ul_ru_fix_grp->sta_info[0].tgt_rssi[2] = 74;
		ul_ru_fix_grp->sta_info[0].mcs = 11;
		ul_ru_fix_grp->sta_info[0].ss = 1;

		ul_ru_fix_grp->sta_info[1].tgt_rssi[0] = 72;
		ul_ru_fix_grp->sta_info[1].tgt_rssi[1] = 74;
		ul_ru_fix_grp->sta_info[1].tgt_rssi[2] = 74;
		ul_ru_fix_grp->sta_info[1].mcs = 11;
		ul_ru_fix_grp->sta_info[1].ss = 1;

		ul_ru_fix_grp->sta_info[2].tgt_rssi[0] = 0;
		ul_ru_fix_grp->sta_info[2].tgt_rssi[1] = 72;
		ul_ru_fix_grp->sta_info[2].tgt_rssi[2] = 74;
		ul_ru_fix_grp->sta_info[2].mcs = 11;
		ul_ru_fix_grp->sta_info[2].ss = 1;

		ul_ru_fix_grp->sta_info[3].tgt_rssi[2] = 0;
		ul_ru_fix_grp->sta_info[3].tgt_rssi[2] = 0;
		ul_ru_fix_grp->sta_info[3].tgt_rssi[2] = 74;
		ul_ru_fix_grp->sta_info[3].mcs = 11;
		ul_ru_fix_grp->sta_info[3].ss = 1;
	}
	else if(!strcmp(para, "UL_20BW")){
		DBGP("cfg UL_20BW APUT \n");

		ru_ctrl->ul_psd = 0;
		ul_ru_grp->grp_psd_max = 0xd1;
		ul_ru_grp->grp_psd_max = 0xd1;
		ul_ru_grp->ppdu_bw = CHANNEL_WIDTH_20;
		ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM24;

		ul_ru_fix_grp->gi_ltf = RTW_GILTF_2XHE16;

		phl_bw20_init_8ru_pos(phl);

		ul_ru_fix_grp->sta_info[0].macid = 1;

		ul_ru_fix_grp->sta_info[1].macid = 2;

		ul_ru_fix_grp->sta_info[2].macid = 0xff;

		ul_ru_fix_grp->sta_info[3].macid = 0xff;
	}
	else if(!strcmp(para, "UL_40BW")){
		DBGP("cfg UL_40BW APUT \n");

		ru_ctrl->ul_psd = 0;
		ul_ru_grp->grp_psd_max = 0xd1;
		ul_ru_grp->grp_psd_max = 0xd1;
		ul_ru_grp->ppdu_bw = CHANNEL_WIDTH_40;
		ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM24;

		ul_ru_fix_grp->gi_ltf = RTW_GILTF_2XHE16;

		phl_bw40_init_8ru_pos(phl);

		ul_ru_fix_grp->sta_info[0].macid = 1;

		ul_ru_fix_grp->sta_info[1].macid = 2;

		ul_ru_fix_grp->sta_info[2].macid = 0xff;

		ul_ru_fix_grp->sta_info[3].macid = 0xff;
	}
}

void core_cmd_ru_stop(_adapter *adapter, void *cmd_para, u32 para_num){

	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	void *phl = adapter->dvobj->phl;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	DBGP("%s:\n", __func__);

	rtw_stop_ofdma_grp(adapter);
	ru_ctrl->tx_phase = 0;
	ru_ctrl->tbl_exist = 0;
	adapter->no_rts = 0;
	pxmitpriv->txsc_enable = 1;
	pxmitpriv->txsc_amsdu_enable = 1;
	rtw_phl_write8(phl, 0xca00, 0x10);	// ca00 bit[4]  0: zero padding; 1: EOF Padding
	return;
}

void core_cmd_ru_clean(_adapter *adapter, void *cmd_para, u32 para_num){

	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	void *phl = adapter->dvobj->phl;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	DBGP("%s:\n",__func__);

	rtw_clean_ofdma_grp(adapter);

	ru_ctrl->tx_phase = 0;
	ru_ctrl->tbl_exist = 0;
	adapter->no_rts = 0;
	pxmitpriv->txsc_enable = 1;
	pxmitpriv->txsc_amsdu_enable = 1;
	rtw_phl_write8(phl, 0xca00, 0x10);	// ca00 bit[4]  0: zero padding; 1: EOF Padding
	return;
}

void core_cmd_dump_ru_group(_adapter *adapter, void *cmd_para, u32 para_num)
{
	void *phl = adapter->dvobj->phl;
	u32 *para = (u32 *)cmd_para;

	if(para_num < 1){
		phl_grp_dump_info_dlru(phl, adapter->phl_role);
		printk("----------------------------------------\n");
		phl_grp_dump_info_ulru(phl, adapter->phl_role);
	}else {
		if(para[0] == 1)
			phl_grp_dump_info_dlru(phl, adapter->phl_role);
		else if(para[0] == 2)
			phl_grp_dump_info_ulru(phl, adapter->phl_role);
	}

	printk("----------------------------------------\n");
	phl_grp_dump_assoc_info(phl, adapter->phl_role);

	return;
}

void core_cmd_dump_HETB(_adapter *adapter, void *cmd_para, u32 para_num)
{
	void *phl = adapter->dvobj->phl;
	char *para = (char *)cmd_para;
	u32 value = 0;

	if(para_num<=0){
		DBGP("please enter cmd : dump_hetb,reset\n");
		DBGP("please enter cmd : dump_hetb,all\n");
		DBGP("please enter cmd : dump_hetb,<macid>\n");
		return;
	}

	if(!strcmp(para, "reset")){
		phl_grp_clean_info_HETB(phl, adapter->phl_role);
		//phl_grp_dump_info_HETB(phl, adapter->phl_role, 0);
	} else if(!strcmp(para, "all")){
		phl_grp_dump_info_HETB(phl, adapter->phl_role, 0);
	} else {
		//para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("dump macid=%d hetb status:\n", value);
		phl_grp_dump_info_HETB(phl, adapter->phl_role, value);
	}

	return;
}

void core_cmd_ru_c2h_intvl(_adapter *adapter, void *cmd_para, u32 para_num)
{
	void *phl = adapter->dvobj->phl;
	u32 *para = (u32 *)cmd_para;
	//u16 intvl = 1000;

	if(para_num <=0){
		DBGP("please enter cmd : ru_c2h_intvl,<intvl(ms)>\n");
		return;
	}

	adapter->ru_c2h_intvl = para[0];
	DBGP("%s: adapter->ru_c2h_intvl:%d\n", __func__, adapter->ru_c2h_intvl);

}

void core_cmd_ru_c2h_en(_adapter *adapter, void *cmd_para, u32 para_num)
{
	void *phl = adapter->dvobj->phl;
	u32 *para = (u32 *)cmd_para;
	u16 fw_c2h_en;
	u16 intvl=0;

	if(para_num <=0){
		DBGP("please enter cmd : ru_c2h_en,<0/1>\n");
		return;
	}

	fw_c2h_en = para[0];

	if(fw_c2h_en == 1){
		intvl = adapter->ru_c2h_intvl? adapter->ru_c2h_intvl:1000;

		DBGP("%s: ru_c2h_en:%d, intvl %d (ms)\n", __func__, fw_c2h_en, intvl);
	} else {
		fw_c2h_en = 0;
		DBGP("%s: ru_c2h_en:%d, intvl %d (ms)\n", __func__, fw_c2h_en, intvl);

	}
	rtw_core_mac_set_ru_fwc2h_en(adapter, fw_c2h_en, intvl);

	return;
}

void core_cmd_dump_group(_adapter *adapter, void *cmd_para, u32 para_num){

	rtw_dump_ofdma_group(adapter);

	return;
}

bool core_is_he_rate(u16 rate)
{
	return ((((rate & 0x1ff) >= HRATE_HE_NSS1_MCS0) && ((rate & 0x1ff) <= HRATE_HE_NSS1_MCS11)) ||
			(((rate & 0x1ff) >= HRATE_HE_NSS2_MCS0) && ((rate & 0x1ff) <= HRATE_HE_NSS2_MCS11)) ||
			(((rate & 0x1ff) >= HRATE_HE_NSS3_MCS0) && ((rate & 0x1ff) <= HRATE_HE_NSS3_MCS11)) ||
			(((rate & 0x1ff) >= HRATE_HE_NSS4_MCS0) &&	((rate & 0x1ff) <= HRATE_HE_NSS4_MCS11))
			) ? true : false;
}

void core_cmd_cur_rate(_adapter *adapter, void *cmd_para, u32 para_num){

	struct sta_priv *pstapriv = &adapter->stapriv;
	void *phl = adapter->dvobj->phl;

	_list *plist, *phead;
	struct sta_info *psta = NULL;

	//u8 dl_ru_rate_grp;
	u8 ru_rate_group=0, mcs_idx;
	u8 tx_cap_grp_idx = 0;
	u16 tx_pwr = 0;

	struct rtw_stats *sta_stats = NULL;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {

		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);

		sta_stats = &psta->phl_sta->stats;

		mcs_idx = psta->phl_sta->cur_tx_data_rate % 16;

		ru_rate_group = (!core_is_he_rate(psta->phl_sta->cur_tx_data_rate))? 255 :
				(11 >= mcs_idx && mcs_idx >= 9) ? 0 :
				(9 > mcs_idx && mcs_idx >=5) ? 1 :
				(5 > mcs_idx && mcs_idx >=0) ? 2 : 255;

		if(ru_rate_group != 255){

			ru_rate_group = (ru_rate_group & 0x3);
			if(ru_rate_group == 0)
				//tx_pwr = rtw_phl_get_tx_pwr_by_txrate(phl, adapter->phl_role->chandef.bw, 11);
				tx_pwr = 0x24;
			else if(ru_rate_group == 1)
				//tx_pwr = rtw_phl_get_tx_pwr_by_txrate(phl, adapter->phl_role->chandef.bw, 8);
				tx_pwr = 0x3c;
			else if(ru_rate_group == 2)
				//tx_pwr = rtw_phl_get_tx_pwr_by_txrate(phl, adapter->phl_role->chandef.bw, 4);
				tx_pwr = 0x36;
			else
				DBGP("ru_rate_group:%d\n",ru_rate_group);
		}

		DBGP("psta: macid:%d, mac_addr:"MAC_FMT" (state:0x%x), cur_trx_rate:(0x%x,0x%x), avg_trx_rate:(0x%x,0x%x)\n",
			psta->phl_sta->macid, MAC_ARG(psta->phl_sta->mac_addr), psta->state,
			psta->phl_sta->cur_tx_data_rate, psta->phl_sta->cur_rx_data_rate,
			sta_stats->average_HE_tx_rate_new, sta_stats->average_HE_rx_rate);
		DBGP("bw:%d, rate_group:%d, tx_pwr:0x%x\n", adapter->phl_role->chandef.bw, ru_rate_group, tx_pwr);

		plist = get_next(plist);
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	return;
}

#endif

void core_cmd_fw_rsvd_dump(_adapter *adapter, void *cmd_para, u32 para_num){
	u32 *para = (u32 *)cmd_para;
	void *phl = adapter->dvobj->phl;
#ifdef CONFIG_FSM
	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// cc10b15eaa1ebe975bd127598e49613491f3be2d mark_cs_lin
	rtw_phl_ser_fw_rsvd_dump(phl);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */
#endif
	return;
}

#ifdef CONFIG_CORE_TXSC
void core_cmd_txsc(_adapter *adapter, void *cmd_para, u32 para_num)
{
	struct	mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	u32 idx = 0;
	char *para = (char *)cmd_para;
	u32 value = 0;

	if(para_num<=0)
		return;

	if (!strcmp(para, "enable")){
		u8 old_value = pxmitpriv->txsc_enable;
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);

		RTW_PRINT("[txsc] enable:%d\n", value);
		if (1) {//(check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
			pxmitpriv->txsc_enable = value;
		#ifdef CONFIG_TXSC_AMSDU
			if(pxmitpriv->txsc_enable == 0)
				pxmitpriv->txsc_amsdu_enable = 0;
		#endif
		} else
			RTW_PRINT("[TXSC][WARNING] only AP mode support tx shortcut now !!\n");

		if (value != old_value)
			txsc_clear(adapter, 1);
	}else if(!strcmp(para, "debug")){

		RTW_PRINT("[txsc] debug info\n");
		txsc_dump(adapter, RTW_DBGDUMP);

	}else if(!strcmp(para, "clear")){

		RTW_PRINT("[TXSC] clear shortcut\n");
		txsc_clear(adapter, 1);
	}
}

extern enum rtw_phl_status rtw_phl_dump_sh_buf(void *phl, u8 *buf, u8 type);

s32 rtw_core_dump_sh_buf(_adapter *padapter, char *buf, u8 type)
{
     void *phl = padapter->dvobj->phl;
#ifndef CONFIG_RTW_LINK_PHL_MASTER
	 // 4d53f02319dec52b18f493f699cdbfafabb33d45 ystang
     rtw_phl_dump_sh_buf(phl, buf, type);
#endif /* 4d53f02319dec52b18f493f699cdbfafabb33d45 */
      // rtw_hal_mac_get_buffer_data
     return 0;
}

void rtw_core_dump_buf_cap(_adapter *padapter, void *m)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)(padapter->dvobj->phl);
	struct hal_info_t *hal_info = (struct hal_info_t *)phl_info->hal;
	struct rtw_phl_com_t *phl_com = (struct rtw_phl_com_t *)phl_info->phl_com;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;
	struct bus_sw_cap_t *bus_sw = &phl_info->phl_com->bus_sw_cap;
	struct bus_hw_cap_t *bus_hw = &hal_com->bus_hw_cap;
	struct bus_cap_t *bus_cap = &hal_com->bus_cap;
	struct dev_cap_t *dev_sw_cap = &phl_com->dev_sw_cap;
	struct dev_cap_t *dev_hw_cap = &hal_com->dev_hw_cap;
	struct dev_cap_t *dev_cap = &phl_com->dev_cap;

	RTW_PRINT_SEL(m, "====SW_CAP=========================\n");
	RTW_PRINT_SEL(m, "    TXQ tx bd num: %d\n\n" , bus_sw->txbd_num);
	RTW_PRINT_SEL(m, "    RXQ rx bd num: %d\n" , bus_sw->rxbd_num);
	RTW_PRINT_SEL(m, "    RXQ rx buf num: %d\n" , bus_sw->rxbuf_num);
	RTW_PRINT_SEL(m, "    RXQ rx buf size: %d\n" , bus_sw->rxbuf_size);
	RTW_PRINT_SEL(m, "    RPQ rx bd num: %d\n" , bus_sw->rpbd_num);
	RTW_PRINT_SEL(m, "    RPQ rx buf num: %d\n" , bus_sw->rpbuf_num);
	RTW_PRINT_SEL(m, "    RPQ rx buf size: %d\n\n" , bus_sw->rpbuf_size);
	RTW_PRINT_SEL(m, "    BAND: 0x%x, BW: 0x%x\n" , dev_sw_cap->band_sup, dev_sw_cap->bw_sup);

	RTW_PRINT_SEL(m, "====HW_CAP=========================\n");
	RTW_PRINT_SEL(m, "    TXQ tx bd num: %d\n\n" , bus_hw->max_txbd_num);
	RTW_PRINT_SEL(m, "    RXQ rx bd num: %d\n" , bus_hw->max_rxbd_num);
	RTW_PRINT_SEL(m, "    RXQ rx buf size: %d\n" , bus_hw->max_rxbuf_size);
	RTW_PRINT_SEL(m, "    RPQ rx bd num: %d\n" , bus_hw->max_rpbd_num);
	RTW_PRINT_SEL(m, "    RPQ rx buf size: %d\n\n" , bus_hw->max_rpbuf_size);
	RTW_PRINT_SEL(m, "    BAND: 0x%x, BW: 0x%x\n" , dev_hw_cap->band_sup, dev_hw_cap->bw_sup);

	RTW_PRINT_SEL(m, "====BUS_CAP========================\n");
	RTW_PRINT_SEL(m, "    TXQ tx bd num: %d\n\n" , bus_cap->txbd_num);
	RTW_PRINT_SEL(m, "    RXQ rx bd num: %d\n" , bus_cap->rxbd_num);
	RTW_PRINT_SEL(m, "    RXQ rx buf num: %d\n" , bus_cap->rxbuf_num);
	RTW_PRINT_SEL(m, "    RXQ rx buf size: %d\n" , bus_cap->rxbuf_size);
	RTW_PRINT_SEL(m, "    RPQ rx bd num: %d\n" , bus_cap->rpbd_num);
	RTW_PRINT_SEL(m, "    RPQ rx buf num: %d\n" , bus_cap->rpbuf_num);
	RTW_PRINT_SEL(m, "    RPQ rx buf size: %d\n\n" , bus_cap->rpbuf_size);
	RTW_PRINT_SEL(m, "    BAND: 0x%x, BW: 0x%x\n" , dev_cap->band_sup, dev_cap->bw_sup);

}


#if defined(CONFIG_VW_REFINE)
void vw_latency_set(_adapter *adapter)
{
	_adapter *primary_adapter = dvobj_get_primary_adapter(adapter->dvobj);
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct sta_info *psta = rtw_get_stainfo(pstapriv, cur_network->network.MacAddress);
	u32 idx = 0;
#ifdef CONFIG_RTW_DEBUG
	extern u8 phl_log_level;
	extern uint rtw_drv_log_level;

	phl_log_level = 0;
	rtw_drv_log_level = 0;
#endif
#ifdef CONFIG_DYNAMIC_THROUGHPUT_ENGINE
	primary_adapter->registrypriv.manual_edca = 1;
#endif
	rtw_hw_set_edca(adapter, 0, 0x1109);
	rtw_hw_set_edca(adapter, 3, 0x1109);

	/* airtime mode */
	rtw_phl_write32(adapter->dvobj->phl, 0x9E14, 0x00000001);

	/* disable hw cts2self */
	rtw_phl_write32(adapter->dvobj->phl, 0xC624, 0x04041010);

	adapter->max_enq_len = 10240;
	adapter->sta_deq_len = 20;
	adapter->tx_lmt = 1;
	adapter->max_deq_len = 4096;
	adapter->swq_timeout = 1;
	adapter->hw_swq_timeout = 200;
	adapter->amsdu_merge_cnt = 6;
	pxmitpriv->txsc_amsdu_enable = 1;
	adapter->tx_amsdu = 1;
	adapter->sta_sn_gap = 256;

	adapter->txForce_enable = 1;
	reset_txforce_para(adapter);
	if (psta && psta->phl_sta) {
		if (psta->phl_sta->wmode & WLAN_MD_11AX) {
			adapter->txForce_rate = 0x19b;
			adapter->txForce_gi = 3;
		} else if (psta->phl_sta->wmode & WLAN_MD_11AC) {
			adapter->txForce_rate = 0x119;
			adapter->txForce_gi = 1;
		} else if (psta->phl_sta->wmode & WLAN_MD_11N) {
			adapter->txForce_rate = 0x8f;
			adapter->txForce_gi = 1;
		} else {
			adapter->txForce_rate = 0x19b;
			adapter->txForce_gi = 3;
		}
	}
	adapter->txForce_agg = 1;
	adapter->txForce_aggnum = 19;
	adapter->registrypriv.vcs_type = CTS_TO_SELF;
}

void vw_latency_txforce_set(_adapter *adapter, struct sta_info *psta)
{
	_adapter *primary_adapter = dvobj_get_primary_adapter(adapter->dvobj);

	if ( psta->phl_sta->wmode & WLAN_MD_11AX ) {
		adapter->txForce_rate = 0x19b;
		adapter->txForce_gi = 3;
	} else if ( psta->phl_sta->wmode & WLAN_MD_11AC ) {
		adapter->txForce_rate = 0x119;
		adapter->txForce_gi = 1;
	} else if ( psta->phl_sta->wmode & WLAN_MD_11N ) {
		adapter->txForce_rate = 0x8f;
		adapter->txForce_gi = 1;
	}
}
#endif

void core_cmd_debug(_adapter *adapter, void *cmd_para, u32 para_num)
{
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	u32 idx = 0;
	char *para = (char *)cmd_para;
	u32 value = 0;

	extern u8 DBG_PRINT_MDATA_ONCE;
	extern u8 DBG_PRINT_TXREQ_ONCE;
	extern u8 DBG_PRINT_RXPKT_ONCE;
#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
	extern u8 DBG_PRINT_RCV_ONCE;
#endif

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	extern u8 DBG_WAPI_USK_UPDATE_ONCE;
	extern u8 DBG_WAPI_MSK_UPDATE_ONCE;
#endif
	if(para_num<=0){
		RTW_PRINT("mdata\n");
		RTW_PRINT("txreq\n");
		RTW_PRINT("wdpage\n");
		RTW_PRINT("rx\n");
		RTW_PRINT("rxpkt\n");
		return;
	}

	if (!strcmp(para, "mdata"))
		DBG_PRINT_MDATA_ONCE = 1;
	else if (!strcmp(para, "txreq"))
		DBG_PRINT_TXREQ_ONCE = 1;
#ifdef DEBUG_PHL_TX
	else if (!strcmp(para, "wdpage"))
		phl_com->tx_stats.flag_print_wdpage_once = 1;
#endif
#if defined(CONFIG_RTW_BYPASS_DEAMSDU) && defined(PLATFORM_LINUX)
	else if (!strcmp(para, "rx"))
		DBG_PRINT_RCV_ONCE = 10;
#endif
	else if (!strcmp(para, "rxpkt")){
		DBG_PRINT_RXPKT_ONCE = 1;
	}else if (!strcmp(para, "swdbg")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->swdbg = value;
	} else if (!strcmp(para, "chandef")) {
		rtw_phl_mr_dump_cur_chandef(adapter->dvobj->phl, adapter->phl_role);
	} else if (!strcmp(para, "mrinfo")) {
		rtw_phl_mr_dump_info(adapter->dvobj->phl, true);
	} else if (!strcmp(para, "cca_rts")) { //_CCA_RTS_MODE_
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->cca_rts_mode = value;
		if (value == 3) {
			/* 0xCC24[12]=0 0xcc24[15:14]=1, follow RXSC/duplicate CTS */
			rtw_phl_write32(adapter->dvobj->phl, 0xCC24,
				(rtw_phl_read32(adapter->dvobj->phl, 0xCC24)|BIT14)&(~BIT12));
			RTW_PRINT("0xCC24: 0x%08x\n", rtw_phl_read32(adapter->dvobj->phl, 0xCC24));
			/* 0xCC08[11:10]=2, duplicate ACL/BA */
			rtw_phl_write32(adapter->dvobj->phl, 0xCC08,
				rtw_phl_read32(adapter->dvobj->phl, 0xCC08)|BIT11);
			RTW_PRINT("0xCC08: 0x%08x\n", rtw_phl_read32(adapter->dvobj->phl, 0xCC08));
		} else if (value == 0) {
			/* 0xCC24[12]=0 0xcc24[15:14]=1, follow RXSC/duplicate CTS */
			rtw_phl_write32(adapter->dvobj->phl, 0xCC24,
				rtw_phl_read32(adapter->dvobj->phl, 0xCC24)&(~(BIT14|BIT15|BIT12)));
			RTW_PRINT("0xCC24: 0x%08x\n", rtw_phl_read32(adapter->dvobj->phl, 0xCC24));
			/* 0xCC08[11:10]=2, duplicate ACL/BA */
			rtw_phl_write32(adapter->dvobj->phl, 0xCC08,
				rtw_phl_read32(adapter->dvobj->phl, 0xCC08)&(~(BIT11|BIT10)));
			RTW_PRINT("0xCC08: 0x%08x\n", rtw_phl_read32(adapter->dvobj->phl, 0xCC08));
		}
	} else if(!strcmp(para, "shbuf")){
		rtw_core_dump_sh_buf(adapter, NULL, 0);
	} else if(!strcmp(para, "axdma")){
		rtw_core_dump_sh_buf(adapter, NULL, 1);
	} else if(!strcmp(para, "bufcap")){
		rtw_core_dump_buf_cap(adapter, NULL);
	}
#ifdef CONFIG_VW_REFINE
	else if(!strcmp(para, "vw")){
		_adapter *primary_adapter = NULL;
			primary_adapter = dvobj_get_primary_adapter(adapter->dvobj);

		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->vw_enable = value;

		if (0 == adapter->vw_enable) {
#ifdef CONFIG_DYNAMIC_THROUGHPUT_ENGINE
			primary_adapter->registrypriv.manual_edca = 0;
#endif
			adapter->dvobj->tx_mode = 0;
		} else if (1 == adapter->vw_enable) {

#ifdef CONFIG_DYNAMIC_THROUGHPUT_ENGINE
			primary_adapter->registrypriv.manual_edca = 1;
#endif
			rtw_hw_set_edca(adapter, 0, 0x5e4425);

			/* airtime mode */
			// rtw_phl_write32(adapter->dvobj->phl, 0x9E14, 0x0808);
			// adapter->no_wdinfo = 1;
		} else if (2 == adapter->vw_enable) {
			vw_latency_set(adapter);
		}

		if ( 3 == adapter->vw_enable) {
		    adapter->vw_enable = 1;
#ifdef CONFIG_DYNAMIC_THROUGHPUT_ENGINE
			primary_adapter->registrypriv.manual_edca = 1;
#endif
			rtw_hw_set_edca(adapter, 0, 0x5e4220);
		    rtw_phl_write32(adapter->dvobj->phl, 0x9E14, 0x0808);
		    //no_ra = 0;
		}

		if ( 4 == adapter->vw_enable) {
		    rtw_phl_write32(adapter->dvobj->phl, 0x9E14, 0x0808);
		    //no_ra = 0;
		}

		if (adapter->vw_enable)
			rtw_phl_set_one_txring_mode(adapter->dvobj->phl, 1);

		//set_no_bb_h2c(adapter, no_ra);
	} else if(!strcmp(para, "map")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->is_map = value;
#ifdef RTW_CORE_TX_MSDU_TRANSFER_IN_PHL
		if (adapter->is_map == 1)
			adapter->is_msdu = 0;
		RTW_PRINT("is_msdu:%d (CORE:0 PHL:1)\n", adapter->is_msdu);
#endif
		RTW_PRINT("is_map:%d (CORE:1 PHL:0)\n", adapter->is_map);
	}
#ifdef RTW_CORE_TX_MSDU_TRANSFER_IN_PHL
	else if(!strcmp(para, "msdu")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->is_msdu = value;
		if (adapter->is_msdu == 1)
			adapter->is_map = 0;
		RTW_PRINT("is_map:%d (CORE:1 PHL:0)\n", adapter->is_map);
		RTW_PRINT("is_msdu:%d (CORE:0 PHL:1)\n", adapter->is_msdu);
	}
#endif
	else if(!strcmp(para, "release")) {
		core_cmd_debug_phl_wd_release(adapter, cmd_para, para_num);
	} else if(!strcmp(para, "wd")) {
		core_cmd_debug_wd(adapter, cmd_para, para_num);
	}  else if(!strcmp(para, "dump")) {
		core_cmd_debug_dump(adapter, cmd_para, para_num);
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	} else if (!strcmp(para, "wapi_usk_update")) {
		DBG_WAPI_USK_UPDATE_ONCE = 1;
	} else if (!strcmp(para, "wapi_msk_update")) {
		DBG_WAPI_MSK_UPDATE_ONCE = 1;
#endif
#if defined(CONFIG_RTW_DBG_TX_MGNT)
	} else if (!strcmp(para, "tx_mgnt")) {
		core_cmd_debug_tx_mgnt(adapter, cmd_para, para_num);
#endif /* CONFIG_RTW_DBG_TX_MGNT */
	} else if (!strcmp(para, "mac_dbg_cfg")) {
		para = get_next_para_str(para);
		sscanf(para, "%x", &value);
		RTW_PRINT("set mac_dbg_cfg:0x%x\n", value);
		rtw_phl_cfg_mac_dump_setting(adapter->dvobj->phl, (u8)value);
	}
#endif
}

/*
  usage:
      iwpriv wlanX phl_test hfc,012pa
        0: ACH0 ~ ACH3, 1: ACH4 ~ ACH7, 2: ACH8 ~ ACH11
        p: PUB, a: all
*/
void core_cmd_hfc(_adapter *adapter, void *cmd_para, u32 para_num)
{
	void *phl = adapter->dvobj->phl;
	char *para = (char *)cmd_para;
	u8 idx;
	u32 ctrl, info, ctrl2, info2;
	u32 ch[2][12], pub[4], wp[3];
	u32 ctrl_addr, info_addr;

	if(para_num<=0)
		return;

	if(strchr(para, '0') || strchr(para, 'a')) {
		ctrl_addr = 0x8a10;
		info_addr = 0x8a50;
		for (idx = 0; idx < 4; idx++, ctrl_addr += 4, info_addr += 4) {
			ctrl = rtw_phl_read32(phl, ctrl_addr);
			info = rtw_phl_read32(phl, info_addr);
			printk("%d: %d %d | a %d | u %d\n", idx,
					(ctrl>>16)&0x1fff, ctrl&0x1fff, (info>>16)&0x1fff, info&0x1fff);
		}
	}
	if(strchr(para, '1') || strchr(para, 'a')) {
		ctrl_addr = 0x8a20;
		info_addr = 0x8a60;
		for (idx = 4; idx < 8; idx++, ctrl_addr += 4, info_addr += 4) {
			ctrl = rtw_phl_read32(phl, ctrl_addr);
			info = rtw_phl_read32(phl, info_addr);
			printk("%d: %d %d | a %d | u %d\n", idx,
					(ctrl>>16)&0x1fff, ctrl&0x1fff, (info>>16)&0x1fff, info&0x1fff);
		}
	}
	if(strchr(para, '2') || strchr(para, 'a')) {
		ctrl_addr = 0x8a30;
		info_addr = 0x8a70;
		for (idx = 8; idx < 12; idx++, ctrl_addr += 4, info_addr += 4) {
			ctrl = rtw_phl_read32(phl, ctrl_addr);
			info = rtw_phl_read32(phl, info_addr);
			printk("%d: %d %d | a %d | u %d\n", idx,
					(ctrl>>16)&0x1fff, ctrl&0x1fff, (info>>16)&0x1fff, info&0x1fff);
		}
	}
	if(strchr(para, 'p') || strchr(para, 'a')) {
		ctrl = rtw_phl_read32(phl, 0x8a90);
		info = rtw_phl_read32(phl, 0x8a8c);
		info2 = rtw_phl_read32(phl, 0x8a98);
		ctrl2 = rtw_phl_read32(phl, 0x8a94);
		printk("P-0: max %d | a %d | u %d\n",
				ctrl&0x1fff, info&0x1fff, info2&0x1fff);
		printk("P-1: max %d | a %d | u %d\n",
				(ctrl>>16)&0x1fff, (info>>16)&0x1fff, (info2>>16)&0x1fff);
		printk("0+1: max %d\n", ctrl2&0x1fff);
	}
}

void core_cmd_wmm(_adapter *adapter, void *cmd_para, u32 para_num)
{
	void *phl = adapter->dvobj->phl;
	u8 idx;
	u32 reg, val32;
	char *name[]={"BE", "BK", "VI", "VO"};

	for (idx = 0; idx < 4; idx++) {
		val32 = rtw_phl_read32(phl, 0xc300 + (idx * 4));
		printk("  %s %08x | TXOP %d CW (%d, %d) AIFS %d\n", name[idx], val32,
			((val32>>16)&0x7ff)*32, (val32>>12)&0xf, (val32>>8)&0xf, val32&0xff);
	}
}

void core_cmd_wmmdbg(_adapter *adapter, void *cmd_para, u32 para_num)
{
	void *phl = adapter->dvobj->phl;
	char *para = (char *)cmd_para;
	int macid=-1, reset=0, i, j;
	struct sta_info *psta=NULL;
	int txq_cnt=0;
	char *str_msdu_q[4] = {"VO", "VI", "BE", "BK"};

	for (i=0; i<para_num; i++) {
		if (!strcmp(para, "reset"))
			reset = 1;
		else
			sscanf(para, "%d", &macid);
		para = get_next_para_str(para);
	}

#ifdef CONFIG_DBG_COUNTER
	printk("[OS]\n");
	printk("%3s %10s %10s\n", "TID", "TxCnt", "TxDrop");
	for (i=0; i<ARRAY_SIZE(adapter->tx_logs.os_uc_ip_pri); i++)
		printk("%3d %10u %10u\n", i, adapter->tx_logs.os_uc_ip_pri[i], adapter->tx_logs.os_uc_ip_pri_drop[i]);
	printk("\n");
#endif

	printk("[CORE] amsdu queue, macid %d\n", macid);
	psta = rtw_get_stainfo_by_macid(&adapter->stapriv, (u16)macid);
	if (psta) {
		for (i=0; i<4; i++)
			printk("msdu_q %d [%s]: %4u\n", i, str_msdu_q[i], psta->amsdu_txq[i].cnt);
	}
	printk("\n");

	printk("[PHL] phl ring, macid %d\n", macid);
	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// f75eb059acc393fa745d08b33c7f5e00d4ed1a3f skylin
	if (macid >= 0)
		rtw_phl_dump_ring(phl, (u16)macid, reset);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	printk("\n");

	printk("[PHL] wd ring\n");
	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// f75eb059acc393fa745d08b33c7f5e00d4ed1a3f skylin
	rtw_phl_dump_idle_wd_cnt(phl);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */

#ifdef CONFIG_DBG_COUNTER
	if (reset){
		 memset((void *)adapter->tx_logs.os_uc_ip_pri, 0, sizeof(adapter->tx_logs.os_uc_ip_pri));
		 memset((void *)adapter->tx_logs.os_uc_ip_pri_drop, 0, sizeof(adapter->tx_logs.os_uc_ip_pri_drop));
	}
#endif
}

#ifdef CONFIG_TXSC_AMSDU
#ifdef CONFIG_AMSDU_HW_TIMER
extern void rtw_core_set_gt3(_adapter *padapter, u8 enable, long timeout);
#endif
void core_cmd_amsdu(_adapter *adapter, void *cmd_para, u32 para_num)
{
	struct	mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	u32 idx = 0;
	char *para = (char *)cmd_para;
	u32 value = 0, macid = 0;
	u8 *ra = NULL;
	struct sta_info *psta = NULL;

	if (para_num<=0)
		return;

	if (!strcmp(para, "enable")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);

		RTW_PRINT("[amsdu] set txsc_amsdu_enable:%d\n", value);

		if (value == 1) {
			pxmitpriv->txsc_enable = 0;
			txsc_clear(adapter, 1);
			txsc_amsdu_clear(adapter);
			pxmitpriv->txsc_enable = 1;
			RTW_PRINT("[txsc] set txsc_enable:%d\n", pxmitpriv->txsc_enable);
		} else if(value == 0) {
			txsc_clear(adapter, 1);
			txsc_amsdu_clear(adapter);
		}

		pxmitpriv->txsc_amsdu_enable = value;
		#if 0
		if (pxmitpriv->txsc_amsdu_enable)
			rtw_phl_write32(adapter->dvobj->phl, 0x9b00, 0);
		else
			rtw_phl_write32(adapter->dvobj->phl, 0x9b00, 0x7);
		#endif
	} else if (!strcmp(para, "num") ) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);

		if (value > MAX_TXSC_SKB_NUM) {
			value = MAX_TXSC_SKB_NUM;
			RTW_PRINT("[amsdu] error !!! amsdu num should not over %d\n", MAX_TXSC_SKB_NUM);
		}

		RTW_PRINT("[amsdu] set tx_amsdu num:%d\n", value);
		adapter->tx_amsdu = value;

	} else if (!strcmp(para, "tp")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);

		RTW_PRINT("[amsdu] set tx_amsdu_rate:%d\n", value);
		adapter->tx_amsdu_rate = value;

	} else if(!strcmp(para, "debug")) {

		RTW_PRINT("[amsdu] show info\n");
		txsc_amsdu_dump(adapter, RTW_DBGDUMP);
	} else if(!strcmp(para, "reset")) {

		RTW_PRINT("[amsdu] reset amsdu cnt\n");
		txsc_amsdu_reset(adapter);
#ifdef CONFIG_AMSDU_HW_TIMER
	} else if(!strcmp(para, "hw_timer_enable")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		if(adapter->amsdu_hw_timer_enable != value) {
			adapter->amsdu_hw_timer_enable = value;
			if(adapter->amsdu_hw_timer_enable)
				rtw_core_set_gt3(adapter, 1, 1000);
		}
	} else if(!strcmp(para, "hw_timer_timeout")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->amsdu_hw_timeout = value;
#endif
	} else if (!strcmp(para, "force_num")) {

		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		if (value > MAX_TXSC_SKB_NUM)
			value = MAX_TXSC_SKB_NUM;

		if (para_num == 3) {
			para = get_next_para_str(para);
			sscanf(para, "%d", &macid);
		}

		RTW_PRINT("[amsdu] set txsc_amsdu_force_num/fix_amsdu:%d, macid:%d\n", value, macid);

		if (macid) {
			psta = rtw_get_stainfo_by_macid(&adapter->stapriv, (u16)macid);
			if (psta) { psta->fix_amsdu = value; }
			else { printk("sta not found!!\n"); }
		} else { pxmitpriv->txsc_amsdu_force_num = value; }

		/* A4_TXSC */
		if (MLME_IS_STA(adapter)) {
			ra = get_bssid(&adapter->mlmepriv);
			psta = rtw_get_stainfo(&adapter->stapriv, ra);
			if (psta) {
				if (value)
					psta->txsc_amsdu_num = pxmitpriv->txsc_amsdu_force_num;
				else
					psta->txsc_amsdu_num = adapter->tx_amsdu;
			}
		}
	} else if(!strcmp(para, "mode")) { /* AMSDU_BY_SIZE */
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		RTW_PRINT("[amsdu] set amsdu mode = %d\n", value);
		pxmitpriv->txsc_amsdu_mode = value;
	}
}
#endif/* CONFIG_TXSC_AMSDU */
#endif /* CONFIG_CORE_TXSC */

#ifdef CONFIG_LMT_TXREQ
void core_cmd_lmt_txreq(_adapter *adapter, void *cmd_para, u32 para_num)
{
	char *para = (char *)cmd_para;
	u32 value = 0;

	if (para_num <= 0)
		return;

	if (!strcmp(para, "dump")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->lmt_txreq_dump = value;
	} else if (!strcmp(para, "lmt")) {
		void *phl = adapter->dvobj->phl;
		u32 macid, limit;
		struct sta_info *psta;

		para = get_next_para_str(para);
		sscanf(para, "%d", &macid);
		para = get_next_para_str(para);
		sscanf(para, "%d", &limit);
		printk("limit pending txreq to %d for macid%d\n", limit, macid);

		psta = rtw_get_stainfo_by_macid(&adapter->stapriv, macid);
		if (psta) {
			psta->lmt_pending_txreq = limit;
			if (limit > 0)
				adapter->lmt_txreq_enable = 1;
		}
	} else if (!strcmp(para, "lmt_ac")) {
		void *phl = adapter->dvobj->phl;
		u32 macid, limit;
		u32 ac;
		struct sta_info *psta;

		para = get_next_para_str(para);
		sscanf(para, "%d", &macid);
		para = get_next_para_str(para);
		sscanf(para, "%d", &ac);
		para = get_next_para_str(para);
		sscanf(para, "%d", &limit);
		printk("limit pending txreq[%d] to %d for macid%d\n", ac, limit, macid);

		psta = rtw_get_stainfo_by_macid(&adapter->stapriv, macid);
		if (psta) {
			psta->lmt_pending_txreq_ac[ac] = limit;
			if (limit > 0)
				adapter->lmt_txreq_enable = 1;
		}
	}
}
#endif

#ifdef TX_BEAMFORMING
void core_cmd_sound(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	char *para = (char *)cmd_para;
	int role_idx = 0;
	int ctrl;
	int period;

	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
    struct phl_info_t *phl_info = (struct phl_info_t *)(dvobj->phl);

	if(para_num < 1)
	{
		printk("para_num < 1\r\n");
		return;
	}

	sscanf(para, "%d", &ctrl);

	printk("ctrl = %d\r\n", ctrl);

	if (1 == ctrl) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &period);
		printk("period = %d\r\n", period);
		rtw_phl_sound_start(phl_info, (u8)role_idx, 0, period,
				    PHL_SND_TEST_F_ONE_TIME |
				    //PHL_SND_TEST_F_PASS_STS_CHK |
				    PHL_SND_TEST_F_GRP_EN_BF_FIX |
				    PHL_SND_TEST_F_ENABLE_MU_SND |
				    //PHL_SND_TEST_F_FIX_SU_DO_MU_SND |
				    //PHL_SND_TEST_F_ENABLE_MU_NONTB_SND |
				    0);
		RTW_PRINT("SOUND START(once) : wrole %d !!\n", (int)role_idx);
	} else if (2 == ctrl) {
		rtw_phl_sound_abort(phl_info);
		RTW_PRINT("SOUND ABORT!!\n");
	} else if (3 == ctrl) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &period);
		printk("period = %d\r\n", period);
		rtw_phl_sound_start(phl_info, (u8)role_idx, 0, period,
				    //PHL_SND_TEST_F_PASS_STS_CHK |
				    PHL_SND_TEST_F_GRP_EN_BF_FIX |
				    PHL_SND_TEST_F_ENABLE_MU_SND |
				    //PHL_SND_TEST_F_FIX_SU_DO_MU_SND |
				    //PHL_SND_TEST_F_ENABLE_MU_NONTB_SND |
				    0);
		RTW_PRINT("SOUND START(loop) : wrole %d !!\n", (int)role_idx);
	} else if(9 == ctrl) {
		RTW_PRINT("DUMP BF Entry in debugview\n");
			rtw_hal_bf_dbg_dump_entry_all(phl_info->hal);
	} else if(4 == ctrl) {
		struct hal_info_t *hal_info = (struct hal_info_t *)phl_info->hal;
		struct rtw_hal_com_t *hal_com = hal_info->hal_com;
		u32 cmdid;
		para = get_next_para_str(para);
		sscanf(para, "%d", &cmdid);
		printk("cmdid = %d\r\n", cmdid);
		#ifndef CONFIG_RTW_LINK_PHL_MASTER
		// 204005ba98bafc558a924f5112c85a42e5b3c9ce yumiya.tu
		rtw_hal_mac_dumpwlanc(hal_com, cmdid);
		#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	}else if(5 == ctrl) {
		struct hal_info_t *hal_info = (struct hal_info_t *)phl_info->hal;
		struct rtw_hal_com_t *hal_com = hal_info->hal_com;
		u32 cmdid;
		u32 macid;
		para = get_next_para_str(para);
		sscanf(para, "%d", &cmdid);
		printk("cmdid = %d\r\n", cmdid);

		para = get_next_para_str(para);
		sscanf(para, "%d", &macid);
		printk("macid = %d\r\n", macid);
		#ifndef CONFIG_RTW_LINK_PHL_MASTER
		// 204005ba98bafc558a924f5112c85a42e5b3c9ce yumiya.tu
		rtw_hal_mac_dumpwlans(hal_com, cmdid, macid);
		#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	}else if(6 == ctrl) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &period);
		printk("period = %d\r\n", period);
		rtw_phl_sound_start(phl_info, (u8)role_idx, 0, period,
				    //PHL_SND_TEST_F_PASS_STS_CHK |
				    PHL_SND_TEST_F_GRP_EN_BF_FIX |
				    PHL_SND_TEST_F_ENABLE_MU_SND |
				    //PHL_SND_TEST_F_FIX_SU_DO_MU_SND |
				    PHL_SND_TEST_F_ENABLE_MU_NONTB_SND |
				    0);
		RTW_PRINT("SOUND START(loop) : wrole %d !!\n", (int)role_idx);
	}else {
		RTW_PRINT("SOUND TEST CMD not found!\n");
	}

}
#endif

void core_cmd_fw_debug(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	char *para = (char *)cmd_para;
	int ctrl;
	u32 cmdid;
	u32 macid;

	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
    struct phl_info_t *phl_info = (struct phl_info_t *)(dvobj->phl);
	struct hal_info_t *hal_info = (struct hal_info_t *)phl_info->hal;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;

	if(para_num < 1)
	{
		printk("para_num < 1\r\n");
		return;
	}

	sscanf(para, "%d", &ctrl);

	printk("ctrl = %d\r\n", ctrl);

	if (1 == ctrl) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &cmdid);
		printk("cmdid = %d\r\n", cmdid);
		#ifndef CONFIG_RTW_LINK_PHL_MASTER
		// 204005ba98bafc558a924f5112c85a42e5b3c9ce yumiya.tu
		rtw_hal_mac_dumpwlanc(hal_com, cmdid);
		#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	} else if (2 == ctrl) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &cmdid);
		printk("cmdid = %d\r\n", cmdid);

		para = get_next_para_str(para);
		sscanf(para, "%d", &macid);
		printk("macid = %d\r\n", macid);
		#ifndef CONFIG_RTW_LINK_PHL_MASTER
		// 204005ba98bafc558a924f5112c85a42e5b3c9ce yumiya.tu
		rtw_hal_mac_dumpwlans(hal_com, cmdid, macid);
		#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	}
}

extern u8 rtw_bf_send_vht_gid_mgnt_packet_plus(_adapter *adapter, u8 *ra, u8 idx);
#ifdef TX_BEAMFORMING
void core_cmd_gid(_adapter *adapter, void *cmd_para, u32 para_num)
{
	char *para = (char *)cmd_para;
	int macid;
	struct sta_info *psta=NULL;

	sscanf(para, "%d", &macid);

	psta = rtw_get_stainfo_by_macid(&adapter->stapriv, (u16)macid);

	if (psta)
		rtw_bf_send_vht_gid_mgnt_packet_plus(adapter, psta->phl_sta->mac_addr, macid-1);
}
#endif
void core_cmd_sta_dump(_adapter *adapter, void *cmd_para, u32 para_num)
{
	struct	mlme_priv *pmlmepriv = &adapter->mlmepriv;
	u32 idx = 0;
	char *para = (char *)cmd_para;
	u32 value1 = 0, value2 = 0;

	printk("[core_cmd_sta_dump]para_num = %d\n", para_num);
	if (para_num <= 0)
		return;

	/* get timeout value */
	sscanf(para, "%d", &value1);
	adapter->sta_dump_to = value1 + value1%2;

	if (para_num >= 2) {
		para = get_next_para_str(para);
		sscanf(para, "%x", &value2);
		adapter->sta_dump_bitmap = value2;
	}

	RTW_INFO("[%s][STA_DUMP] TO = %d, bitmap = 0x%x\n", __func__, adapter->sta_dump_to, adapter->sta_dump_bitmap);
}

enum _CORE_CMD_PARA_TYPE {
	CMD_PARA_DEC = 0,
	CMD_PARA_HEX,
	CMD_PARA_STR,
};

struct test_cmd_list {
	const char *name;
	void (*fun)(_adapter *, void *, u32);
	enum _CORE_CMD_PARA_TYPE para_type;
};

void test_dump_dec(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	u32 *para = (u32 *)cmd_para;
	DBGP("para_num=%d\n", para_num);

	for(idx=0; idx<para_num; idx++)
		DBGP("para[%d]=%d\n", idx, para[idx]);
	}

void test_dump_hex(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	u32 *para = (u32 *)cmd_para;
	DBGP("para_num=%d\n", para_num);

	for(idx=0; idx<para_num; idx++)
		DBGP("para[%d]=0x%x\n", idx, para[idx]);
}

void test_dump_str(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 idx = 0;
	char *para = (char *)cmd_para;
	DBGP("para_num=%d\n", para_num);

	for(idx=0; idx<para_num; idx++, para+=MAX_PHL_CMD_STR_LEN)
		DBGP("para[%d]=%s\n", idx, para);
}

void get_all_cmd_para_value(_adapter *adapter, char *buf, u32 len, u32 *para, u8 type, u32 *num)
{
	u8 *tmp = NULL;

	if(!buf || !len)
		return;

		DBGP("type=%d buf=%s para=%p num=%d\n", type, buf, para, *num);

	if(len > 0){
		tmp = strsep(&buf, ",");

		if(tmp){
			if(type == CMD_PARA_HEX)
				sscanf(tmp, "%x", para);
			else if(type == CMD_PARA_DEC)
				sscanf(tmp, "%d", para);
			para += 1;
			*num = *num+1;
		}
		else
			return;

		if(buf && (len>strlen(tmp)))
			get_all_cmd_para_value(adapter, buf, strlen(buf), para, type, num);
		else
			return;
	}
	}

void get_all_cmd_para_str(_adapter *adapter, char *buf, u32 len, char *para, u8 type, u32* num)
{
	u8 *tmp = NULL;

	if(!buf || !len)
		return;

	DBGP("type=%d buf=%s para=%p num=%d\n", type, buf, para, *num);

	if(len > 0){
		tmp = strsep(&buf, ",");

		if(tmp){
			strcpy(para, tmp);
			para += MAX_PHL_CMD_STR_LEN;
			*num = *num+1;
	}
		else
			return;

		if(buf && (len>strlen(tmp)))
			get_all_cmd_para_str(adapter, buf, strlen(buf), para, type, num);
		else
			return;
	}
}

void core_cmd_dump_cmac_tbl(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32	*para = (u32 *)cmd_para;
	void	*phl = adapter->dvobj->phl;
	u32	macid;

	macid = para[0];

	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// 74b8420fe3902923e3a50f668c20b0b3c08beacd freddie.ho
	rtl_phl_dump_cam(phl, RTW_CAM_CMAC_TBL, macid, NULL);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */
}

void core_cmd_dump_dmac_tbl(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32	*para = (u32 *)cmd_para;
	void	*phl = adapter->dvobj->phl;
	u32	macid;

	macid = para[0];

	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// 74b8420fe3902923e3a50f668c20b0b3c08beacd freddie.ho
	rtl_phl_dump_cam(phl, RTW_CAM_DMAC_TBL, macid, NULL);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */
}

void core_cmd_dump_addr_cam(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32	*para = (u32 *)cmd_para;
	void	*phl = adapter->dvobj->phl;
	u32	index;

	index = para[0];

	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// 74b8420fe3902923e3a50f668c20b0b3c08beacd freddie.ho
	rtl_phl_dump_cam(phl, RTW_CAM_ADDR, index, NULL);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */
}

void core_cmd_dump_sec_cam(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32	*para = (u32 *)cmd_para;
	void	*phl = adapter->dvobj->phl;
	u32	index;

	index = para[0];

	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// 74b8420fe3902923e3a50f668c20b0b3c08beacd freddie.ho
	rtl_phl_dump_cam(phl, RTW_CAM_SECURITY, index, NULL);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */
}

#ifdef RTW_STA_BWC
void core_cmd_sta_bwc(_adapter *adapter, void *cmd_para, u32 para_num)
{
	u32 *para = (u32 *)cmd_para;
	int level = 0;

	if(para_num < 1){
		DBGP("invalid cmd\n");
		return;
	}

	level = para [0];
	sta_bwc_limit_tp(adapter->pnetdev, level);
	return;
}
#endif

#ifdef CONFIG_RTW_TWT_DBG
void core_cmd_twt(_adapter *adapter, void *cmd_para, u32 para_num)
{
	char *para = (char *)cmd_para;
	u32 value = 0;

	if(para_num == 0)
		return;

	if (!strcmp(para, "send_setup")) {
		DBGP("twt send setup\n");
		rtw_core_twt_test_cmd_setup(adapter);
	} else if (!strcmp(para, "send_teardown")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("twt send teardown, id = %d\n", value);
		adapter->twt_cmd_teardown_id = value;
		rtw_core_twt_test_cmd_teardown(adapter);
	} else if (!strcmp(para, "pwrbit")) {
		u32 value2 = 0;
		if (para_num < 3)
			return;
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		para=get_next_para_str(para);
		sscanf(para, "%d", &value2);
		DBGP("twt set pwrbit of macid %d to %d\n", value, value2);
		adapter->twt_cmd_pwrbit = value2;
		rtw_core_twt_test_cmd_pwrbit(adapter, value, value2);
	} else if (!strcmp(para, "announce")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("twt set announce, macid = %d\n", value);
		rtw_core_twt_test_cmd_announce(adapter, value);
	} else if(!strcmp(para, "macid")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid = %d\n", value);
		adapter->twt_cmd_macid = value;
	} else if(!strcmp(para, "trigger")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("trigger = %d\n", value);
		adapter->twt_cmd_trigger = value;
	} else if(!strcmp(para, "implicit")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("implicit = %d\n", value);
		adapter->twt_cmd_implicit = value;
	} else if(!strcmp(para, "flow_type")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("flow_type = %d\n", value);
		adapter->twt_cmd_flow_type = value;
	} else if(!strcmp(para, "flow_id")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("flow_id = %d\n", value);
		adapter->twt_cmd_flow_id = value;
	} else if(!strcmp(para, "wake_int_exp")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("wake_int_exp = %d\n", value);
		adapter->twt_cmd_wake_int_exp = value;
	} else if(!strcmp(para, "twt_h")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("twt_h = %d\n", value);
		adapter->twt_cmd_target_wake_t_h = value;
	} else if(!strcmp(para, "twt_l")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("twt_l = %d\n", value);
		adapter->twt_cmd_target_wake_t_l = value;
	} else if(!strcmp(para, "wake_dur")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("wake_dur = %d\n", value);
		adapter->twt_cmd_nom_min_twt_wake_dur = value;
	} else if(!strcmp(para, "wake_int_mantissa")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("wake_int_mantissa = %d\n", value);
		adapter->twt_cmd_twt_wake_int_mantissa = value;
	} else if(!strcmp(para, "apep")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("apep = %d\n", value);
		adapter->twt_cmd_trigger_apep = value;
	} else if(!strcmp(para, "rssi")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("rssi = %d\n", value);
		adapter->twt_cmd_trigger_rssi = value;
	} else if(!strcmp(para, "data_rate")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("data_rate = %d\n", value);
		adapter->twt_cmd_data_rate = value;
	} else if(!strcmp(para, "mcs")) {
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs = %d\n", value);
		adapter->twt_cmd_trigger_mcs = value;
	} else if(!strcmp(para, "dump")) {
		printk("Current TWT parameters for AP issue setup:\n");
		printk("  macid: %d\n", adapter->twt_cmd_macid);
		printk("  trigger: %d\n", adapter->twt_cmd_trigger);
		printk("  implicit: %d\n", adapter->twt_cmd_implicit);
		printk("  flow_type: %d\n", adapter->twt_cmd_flow_type);
		printk("  flow_id: %d\n", adapter->twt_cmd_flow_id);
		printk("  wake_int_exp: %d\n", adapter->twt_cmd_wake_int_exp);
		printk("  target_wake_t_h: 0x%08x\n", adapter->twt_cmd_target_wake_t_h);
		printk("  target_wake_t_l: 0x%08x\n", adapter->twt_cmd_target_wake_t_l);
		printk("  twt_wake_int_mantissa: %d\n", adapter->twt_cmd_twt_wake_int_mantissa);
		printk("  twt_wake_dur: %d\n", adapter->twt_cmd_nom_min_twt_wake_dur);
	} else if(!strcmp(para, "case1")) {
		/* usage: iwpriv wlanX phl_test twt,case1,(macid) */
		if (para_num < 2)
			return;
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		printk("twt testcase1, macid = %d\n", value);
		adapter->twt_cmd_test_macid1 = value;
		adapter->twt_cmd_test_wait_second = 2;
		rtw_core_twt_test_cmd_testcase1(adapter);
	} else if(!strcmp(para, "case2")) {
		/* usage: iwpriv wlanX phl_test twt,case2,(macid1),(macid2),(wait_second) */
		if (para_num < 4)
			return;
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->twt_cmd_test_macid1 = value;
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->twt_cmd_test_macid2 = value;
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->twt_cmd_test_wait_second = value;
		printk("twt testcase2, macid1 = %d, macid2 = %d, wait_second = %d\n",
				adapter->twt_cmd_test_macid1, adapter->twt_cmd_test_macid2, adapter->twt_cmd_test_wait_second);
		rtw_core_twt_test_cmd_testcase2(adapter);
	} else if(!strcmp(para, "case3")) {
		/* usage: iwpriv wlanX phl_test twt,case3,(macid1),(macid2),(wait_second),(overlap, 0 or 1) */
		if (para_num < 5)
			return;
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->twt_cmd_test_macid1 = value;
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->twt_cmd_test_macid2 = value;
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->twt_cmd_test_wait_second = value;
		para=get_next_para_str(para);
		sscanf(para, "%d", &value);
		adapter->twt_cmd_test_overlap = value;
		printk("twt testcase2, macid1 = %d, macid2 = %d, wait_second = %d, overlap = %d\n",
				adapter->twt_cmd_test_macid1, adapter->twt_cmd_test_macid2, adapter->twt_cmd_test_wait_second, adapter->twt_cmd_test_overlap);
		rtw_core_twt_test_cmd_testcase3(adapter);
	}
}
#endif

void core_cmd_gpio(_adapter *adapter, void *cmd_para, u32 para_num)
{
	struct	mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);

	char *para = (char *)cmd_para;
	u32 value = 0;
	u8 ping_num = 0;
	u8 ping_val = 0; // 0 = low, 1 = high
	u8 ctl_mode = RTW_PHL_GPIO_CTL_MAX;
	u8 mode_type = RTW_AX_SW_IO_MODE_MAX;

	if (para_num<=0) {
		printk("parameter is wrong\n");
		return;
	}

	if(!strcmp(para, "setmode")) {
		para=get_next_para_str(para);
		if(!strcmp(para, "input")){

			para = get_next_para_str(para);
			sscanf(para, "%d", &value);
			ping_num = (u8)value;

			ctl_mode = RTW_PHL_GPIO_CTL_SET_MODE;
			mode_type = RTW_AX_SW_IO_MODE_INPUT;
			printk("setmode = input, ping_num = %d\n",ping_num);

		}else if (!strcmp(para, "output")){
			para = get_next_para_str(para);
			sscanf(para, "%d", &value);
			ping_num = (u8)value;

			ctl_mode = RTW_PHL_GPIO_CTL_SET_MODE;
			mode_type = RTW_AX_SW_IO_MODE_OUTPUT_PP;
			printk("setmode = output, ping_num = %d\n",ping_num);
		}
		else
			printk("wrong input parameter\n");

	} else if(!strcmp(para, "read")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		ping_num = (u8)value;
		ctl_mode = RTW_PHL_GPIO_CTL_READ;

		printk("gpio read, ping_num = %d\n",ping_num);

	} else if(!strcmp(para, "write")) {
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		ping_num = (u8)value;
		para = get_next_para_str(para);
		sscanf(para, "%d", &value);
		ping_val = (u8)value;

		ctl_mode = RTW_PHL_GPIO_CTL_WRITE;
		printk("gpio write, ping_num = %d, ping_val = 0x%x \n",ping_num,ping_val);
	}

	phl_com->gpio_info.ctl_mode = ctl_mode;
	phl_com->gpio_info.mode_type = mode_type;
	phl_com->gpio_info.ping_val = ping_val;
	phl_com->gpio_info.ping_num = ping_num;

	rtw_phl_gpio_ctl(dvobj->phl);
	if (ctl_mode == RTW_PHL_GPIO_CTL_READ)
		printk("read ping_num = %d, ping_val = 0x%x\n",
			phl_com->gpio_info.ping_num,phl_com->gpio_info.ping_val);
}



struct test_cmd_list core_test_cmd_list[] = {
	{"dec", test_dump_dec, CMD_PARA_DEC},
	{"hex", test_dump_hex, CMD_PARA_HEX},
	{"str", test_dump_str, CMD_PARA_STR},
	{"dump_reg", core_cmd_dump_reg, CMD_PARA_HEX},
	{"dump_debug", core_cmd_dump_debug, CMD_PARA_DEC},
	{"dump_cmac",	core_cmd_dump_cmac_tbl,	CMD_PARA_DEC},
	{"dump_dmac",	core_cmd_dump_dmac_tbl,	CMD_PARA_DEC},
	{"dump_sec",	core_cmd_dump_sec_cam,	CMD_PARA_DEC},
	{"dump_addr",	core_cmd_dump_addr_cam,	CMD_PARA_DEC},
	{"record", core_cmd_record_trx, CMD_PARA_STR},
	{"txforce", core_cmd_txforce, CMD_PARA_STR},
#ifdef CONFIG_CPU_PROFILING
	{"profile", core_cmd_profile, CMD_PARA_STR},
#endif
	{"sniffer",	core_cmd_sniffer, CMD_PARA_STR},
	{"sta_dump", core_cmd_sta_dump, CMD_PARA_STR},
#ifdef CONFIG_VW_REFINE
	{"swq", core_cmd_swq, CMD_PARA_STR},
#endif
#ifdef CONFIG_ONE_TXQ
	{"txq", core_cmd_txq, CMD_PARA_STR},
#endif
	{"dump_cnt", core_cmd_dump_cnt,	CMD_PARA_STR},
	{"dump_wd", core_cmd_dump_wd,	CMD_PARA_STR},
	{"hfc", 	core_cmd_hfc,		CMD_PARA_STR},
	{"wmm", 	core_cmd_wmm,		CMD_PARA_STR},
	{"wmmdbg", 	core_cmd_wmmdbg,	CMD_PARA_STR},
#ifdef CONFIG_CORE_TXSC
	{"txsc", core_cmd_txsc,	CMD_PARA_STR},
	{"debug", core_cmd_debug, CMD_PARA_STR},
#ifdef CONFIG_TXSC_AMSDU
	{"amsdu", core_cmd_amsdu, CMD_PARA_STR},
#endif /* CONFIG_TXSC_AMSDU */
#endif /* CONFIG_CORE_TXSC */
#ifdef CONFIG_LMT_TXREQ
	{"lmt_txreq",	core_cmd_lmt_txreq,		CMD_PARA_STR},
#endif
	{"track", core_cmd_track, CMD_PARA_STR},
#ifdef CONFIG_RTW_CORE_RXSC
	{"rxsc", core_cmd_rxsc,	CMD_PARA_STR},
#endif
#ifdef RTW_CORE_PKT_TRACE
	{"pktrace", core_cmd_pktrace,	CMD_PARA_STR},
#endif

#ifdef CONFIG_RTW_TWT_DBG
	{"twt", 	core_cmd_twt, 		CMD_PARA_STR},
#endif
#ifdef CONFIG_WFA_OFDMA_Logo_Test
	//{"wfa_ru_test", core_cmd_wfa_ru_test, CMD_PARA_DEC},		// Mark.CS_update
	//{"fw_tx",	core_cmd_fw_tx,		CMD_PARA_DEC},
	{"dump_sta_hash",	rtw_dump_sta_hash, 	CMD_PARA_STR},
   	{"dump_sta",	rtw_dump_sta, 		CMD_PARA_STR},
	{"dump_ring",	core_cmd_dump_ring,	CMD_PARA_DEC},
	{"ru_para",	core_cmd_ru_para,	CMD_PARA_STR},
	{"ru_cfg",	core_cmd_ru_cfg,		CMD_PARA_STR},
	{"ru_stop",		core_cmd_ru_stop,		CMD_PARA_STR},
	{"ru_clean",	core_cmd_ru_clean,		CMD_PARA_STR},
	{"dump_ru_grp",	core_cmd_dump_ru_group,		CMD_PARA_DEC},
	{"dump_hetb",	core_cmd_dump_HETB,	CMD_PARA_STR},
	{"ru_c2h_intvl",	core_cmd_ru_c2h_intvl, 	CMD_PARA_DEC},
	{"ru_c2h_en",	core_cmd_ru_c2h_en, 	CMD_PARA_DEC},
	{"dump_group",	core_cmd_dump_group,	CMD_PARA_STR},
	{"cur_rate",	core_cmd_cur_rate,	CMD_PARA_STR},
#endif
	{"fw_debug",	core_cmd_fw_debug,	CMD_PARA_STR},
	{"fw_rsvd",	core_cmd_fw_rsvd_dump,	CMD_PARA_DEC},
#ifdef CONFIG_RTW_A4_STA
	{"a4",		core_cmd_a4,		CMD_PARA_STR},
#endif
#if defined (CONFIG_RTW_MULTI_AP) && defined (DEBUG_MAP_NL)
	{"map",		core_cmd_map,		CMD_PARA_STR},
#endif
#ifdef TX_BEAMFORMING
	{"sound",	core_cmd_sound,		CMD_PARA_STR},
  	{"gid",	core_cmd_gid, 	CMD_PARA_STR},
#endif

#ifdef RTW_STA_BWC
	{"sta_bwc",	core_cmd_sta_bwc,		CMD_PARA_DEC},
#endif
	{"gpio", core_cmd_gpio, CMD_PARA_STR},
	{"dump_asoc_list", rtw_dump_sta_asoc_list, CMD_PARA_STR},
	{"dump_auth_list", rtw_dump_sta_auth_list, CMD_PARA_STR},
	{"dump_free_list", rtw_dump_sta_free_list, CMD_PARA_STR},
};

void core_cmd_phl_handler(_adapter *adapter, char *extra)
{
	u32 *para;
	char **para_str;
	char *cmd_name, *cmd_para = NULL;
	struct test_cmd_list *cmd = core_test_cmd_list;
	u32 array_size = ARRAY_SIZE(core_test_cmd_list);
	u32 i = 0;
	char *type[] = {"CMD_PARA_DEC", "CMD_PARA_HEX", "CMD_PARA_STR"};

	para = (u32 *)rtw_malloc(MAX_PHL_CMD_NUM*sizeof(u32));
	if(para == NULL){
		RTW_ERR("[%s:%d]rtw_malloc failed!\n", __FUNCTION__,__LINE__);
		return;
	}

	para_str = (char **)rtw_malloc(MAX_PHL_CMD_NUM*MAX_PHL_CMD_STR_LEN*sizeof(char));
	if(para_str == NULL){
		RTW_ERR("[%s:%d]rtw_malloc failed!\n", __FUNCTION__,__LINE__);
		rtw_mfree(para, MAX_PHL_CMD_NUM*sizeof(u32));
		return;
	}

	_rtw_memset(para, 0x0, MAX_PHL_CMD_NUM*sizeof(u32));
	_rtw_memset(para_str, 0x0, MAX_PHL_CMD_NUM*MAX_PHL_CMD_STR_LEN*sizeof(char));

	cmd_name = strsep(&extra, ",");

	if(!cmd_name){
		for(i = 0; i<array_size; i++, cmd++)
			printk(" - %s\n", cmd->name);
		rtw_mfree(para, MAX_PHL_CMD_NUM*sizeof(u32));
		rtw_mfree(para_str, MAX_PHL_CMD_NUM*MAX_PHL_CMD_STR_LEN*sizeof(char));
		return;
	}


	if(!strcmp("help", cmd_name)){
		DBGP("phl_test cmd: \n");

		for(i = 0; i<array_size; i++, cmd++){
			DBGP("%s (%s) \n", cmd->name, type[cmd->para_type]);
		}
		rtw_mfree(para, MAX_PHL_CMD_NUM*sizeof(u32));
		rtw_mfree(para_str, MAX_PHL_CMD_NUM*MAX_PHL_CMD_STR_LEN*sizeof(char));
		return;
	}

	for(i = 0; i<array_size; i++, cmd++){
		if(!strcmp(cmd->name, cmd_name)){
			void *cmd_para = NULL;
			u32 cmd_para_num = 0;
			if(cmd->para_type == CMD_PARA_DEC || cmd->para_type == CMD_PARA_HEX){
				cmd_para = para;
				if(extra)
				get_all_cmd_para_value(adapter, extra, strlen(extra), para, cmd->para_type, &cmd_para_num);
			} else {
				cmd_para = para_str;
				if(extra)
				get_all_cmd_para_str(adapter, extra, strlen(extra), (char *)para_str, cmd->para_type, &cmd_para_num);
	}

			cmd->fun(adapter, cmd_para, cmd_para_num);
			break;
	}
	}
	rtw_mfree(para, MAX_PHL_CMD_NUM*sizeof(u32));
	rtw_mfree(para_str, MAX_PHL_CMD_NUM*MAX_PHL_CMD_STR_LEN*sizeof(char));
	return;
}

#endif

u8 rtw_test_h2c_cmd(_adapter *adapter, u8 *buf, u8 len)
{
	struct cmd_obj *pcmdobj;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	u8 *ph2c_content;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	u8	res = _SUCCESS;

	pcmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmdobj == NULL) {
		res = _FAIL;
		goto exit;
	}
	pcmdobj->padapter = adapter;

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((u8 *)pcmdobj, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	ph2c_content = rtw_zmalloc(len);
	if (ph2c_content == NULL) {
		rtw_mfree((u8 *)pcmdobj, sizeof(struct cmd_obj));
		rtw_mfree((u8 *)pdrvextra_cmd_parm, sizeof(struct drvextra_cmd_parm));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = TEST_H2C_CID;
	pdrvextra_cmd_parm->type = 0;
	pdrvextra_cmd_parm->size = len;
	pdrvextra_cmd_parm->pbuf = ph2c_content;

	_rtw_memcpy(ph2c_content, buf, len);

	init_h2fwcmd_w_parm_no_rsp(pcmdobj, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);

	res = rtw_enqueue_cmd(pcmdpriv, pcmdobj);

exit:
	return res;
}

#ifdef CONFIG_MP_INCLUDED
static s32 rtw_mp_cmd_hdl(_adapter *padapter, u8 mp_cmd_id)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	int ret = H2C_SUCCESS;
	uint status = _SUCCESS;
	u8 igi;

	if (mp_cmd_id == MP_START) {
		if (padapter->registrypriv.mp_mode == 0) {

#if 0
			rtw_hw_stop(dvobj);
			padapter->registrypriv.mp_mode = 1;
			rtw_reset_drv_sw(padapter);
			if (!rtw_hw_is_init_completed(dvobj)) {
				status = rtw_hw_start(dvobj);
				if (status == _FAIL) {
					ret = H2C_REJECTED;
					goto exit;
				}
				rtw_hw_iface_init(padapter);
			}
#endif
			padapter->registrypriv.mp_mode = 1;
			MPT_InitializeAdapter(padapter, 1);

		}

		if (padapter->registrypriv.mp_mode == 0) {
			ret = H2C_REJECTED;
			goto exit;
		}

		if (padapter->mppriv.mode == MP_OFF) {
			if (g6_mp_start_test(padapter) == _FAIL) {
				ret = H2C_REJECTED;
				goto exit;
			}
			padapter->mppriv.mode = MP_ON;
			/*MPT_PwrCtlDM(padapter, 0);To be needed confirm RF@James, for default setting*/
		}
		padapter->mppriv.bmac_filter = _FALSE;
		igi = 0x20;
		rtw_hal_set_phydm_var(padapter, HAL_PHYDM_IGI_W, &igi, _FALSE);

	} else if (mp_cmd_id == MP_STOP) {
		if (padapter->registrypriv.mp_mode == 1) {
#if 0
			MPT_DeInitAdapter(padapter);
			rtw_hw_stop(dvobj);
			padapter->registrypriv.mp_mode = 0;

			rtw_reset_drv_sw(padapter);

			if (!rtw_hw_is_init_completed(dvobj)) {
				status = rtw_hw_start(dvobj);
				if (status == _FAIL) {
					ret = H2C_REJECTED;
					goto exit;
				}
				rtw_hw_iface_init(padapter);
			}
#endif
		MPT_DeInitAdapter(padapter);
		padapter->registrypriv.mp_mode = 0;
		}

		if (padapter->mppriv.mode != MP_OFF) {
			g6_mp_stop_test(padapter);
			padapter->mppriv.mode = MP_OFF;
		}

	} else {
		RTW_INFO(FUNC_ADPT_FMT"invalid id:%d\n", FUNC_ADPT_ARG(padapter), mp_cmd_id);
		ret = H2C_PARAMETERS_ERROR;
		rtw_warn_on(1);
	}

exit:
	return ret;
}

u8 rtw_mp_cmd(_adapter *adapter, u8 mp_cmd_id, u8 flags)
{
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct submit_ctx sctx;
	u8	res = _SUCCESS;

	parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (parm == NULL) {
		res = _FAIL;
		goto exit;
	}

	parm->ec_id = MP_CMD_WK_CID;
	parm->type = mp_cmd_id;
	parm->size = 0;
	parm->pbuf = NULL;

	if (flags & RTW_CMDF_DIRECTLY) {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (H2C_SUCCESS != rtw_mp_cmd_hdl(adapter, mp_cmd_id))
			res = _FAIL;
		rtw_mfree((u8 *)parm, sizeof(*parm));
	} else {
		/* need enqueue, prepare cmd_obj and enqueue */
		cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(*cmdobj));
		if (cmdobj == NULL) {
			res = _FAIL;
			rtw_mfree((u8 *)parm, sizeof(*parm));
			goto exit;
		}
		cmdobj->padapter = adapter;

		init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_SET_DRV_EXTRA);

		if (flags & RTW_CMDF_WAIT_ACK) {
			cmdobj->sctx = &sctx;
			rtw_sctx_init(&sctx, 10 * 1000);
		}

		res = rtw_enqueue_cmd(pcmdpriv, cmdobj);

		if (res == _SUCCESS && (flags & RTW_CMDF_WAIT_ACK)) {
			rtw_sctx_wait(&sctx, __func__);
			_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
			if (sctx.status == RTW_SCTX_SUBMITTED)
				cmdobj->sctx = NULL;
			_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
			if (sctx.status != RTW_SCTX_DONE_SUCCESS)
				res = _FAIL;
		}
	}

exit:
	return res;
}
#endif	/*CONFIG_MP_INCLUDED*/

#ifdef CONFIG_RTW_CUSTOMER_STR
static s32 rtw_customer_str_cmd_hdl(_adapter *adapter, u8 write, const u8 *cstr)
{
	int ret = H2C_SUCCESS;

	if (write)
		ret = rtw_hal_h2c_customer_str_write(adapter, cstr);
	else
		ret = rtw_hal_h2c_customer_str_req(adapter);

	return ret == _SUCCESS ? H2C_SUCCESS : H2C_REJECTED;
}

static u8 rtw_customer_str_cmd(_adapter *adapter, u8 write, const u8 *cstr)
{
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *parm;
	u8 *str = NULL;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct submit_ctx sctx;
	u8 res = _SUCCESS;

	parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (parm == NULL) {
		res = _FAIL;
		goto exit;
	}

	if (write) {
		str = rtw_zmalloc(RTW_CUSTOMER_STR_LEN);
		if (str == NULL) {
			rtw_mfree((u8 *)parm, sizeof(struct drvextra_cmd_parm));
			res = _FAIL;
			goto exit;
		}
	}

	parm->ec_id = CUSTOMER_STR_WK_CID;
	parm->type = write;
	parm->size = write ? RTW_CUSTOMER_STR_LEN : 0;
	parm->pbuf = write ? str : NULL;

	if (write)
		_rtw_memcpy(str, cstr, RTW_CUSTOMER_STR_LEN);

	/* need enqueue, prepare cmd_obj and enqueue */
	cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(*cmdobj));
	if (cmdobj == NULL) {
		res = _FAIL;
		rtw_mfree((u8 *)parm, sizeof(*parm));
		if (write)
			rtw_mfree(str, RTW_CUSTOMER_STR_LEN);
		goto exit;
	}
	cmdobj->padapter = adapter;

	init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_SET_DRV_EXTRA);

	cmdobj->sctx = &sctx;
	rtw_sctx_init(&sctx, 2 * 1000);

	res = rtw_enqueue_cmd(pcmdpriv, cmdobj);

	if (res == _SUCCESS) {
		rtw_sctx_wait(&sctx, __func__);
		_rtw_mutex_lock_interruptible(&pcmdpriv->sctx_mutex);
		if (sctx.status == RTW_SCTX_SUBMITTED)
			cmdobj->sctx = NULL;
		_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
		if (sctx.status != RTW_SCTX_DONE_SUCCESS)
			res = _FAIL;
	}

exit:
	return res;
}

inline u8 rtw_customer_str_req_cmd(_adapter *adapter)
{
	return rtw_customer_str_cmd(adapter, 0, NULL);
}

inline u8 rtw_customer_str_write_cmd(_adapter *adapter, const u8 *cstr)
{
	return rtw_customer_str_cmd(adapter, 1, cstr);
}
#endif /* CONFIG_RTW_CUSTOMER_STR */

u8 rtw_c2h_wk_cmd(_adapter *padapter, u8 *pbuf, u16 length, u8 type)
{
	struct cmd_obj *cmd;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;
	u8 *extra_cmd_buf;
	u8 res = _SUCCESS;

	cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmd == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmd->padapter = padapter;

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((u8 *)cmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	extra_cmd_buf = rtw_zmalloc(length);
	if (extra_cmd_buf == NULL) {
		rtw_mfree((u8 *)cmd, sizeof(struct cmd_obj));
		rtw_mfree((u8 *)pdrvextra_cmd_parm, sizeof(struct drvextra_cmd_parm));
		res = _FAIL;
		goto exit;
	}

	_rtw_memcpy(extra_cmd_buf, pbuf, length);
	pdrvextra_cmd_parm->ec_id = C2H_WK_CID;
	pdrvextra_cmd_parm->type = type;
	pdrvextra_cmd_parm->size = length;
	pdrvextra_cmd_parm->pbuf = extra_cmd_buf;

	init_h2fwcmd_w_parm_no_rsp(cmd, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);

	res = rtw_enqueue_cmd(pcmdpriv, cmd);

exit:
	return res;
}

#define C2H_TYPE_PKT 1
inline u8 rtw_c2h_packet_wk_cmd(_adapter *adapter, u8 *c2h_evt, u16 length)
{
	return rtw_c2h_wk_cmd(adapter, c2h_evt, length, C2H_TYPE_PKT);
}

static u8 _rtw_run_in_thread_cmd(_adapter *adapter, void (*func)(void *), void *context, s32 timeout_ms)
{
	struct cmd_priv *cmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct cmd_obj *cmdobj;
	struct RunInThread_param *parm;
	struct submit_ctx sctx;
	s32 res = _SUCCESS;

	cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (NULL == cmdobj) {
		res = _FAIL;
		goto exit;
	}
	cmdobj->padapter = adapter;

	parm = (struct RunInThread_param *)rtw_zmalloc(sizeof(struct RunInThread_param));
	if (NULL == parm) {
		rtw_mfree((u8 *)cmdobj, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	parm->func = func;
	parm->context = context;
	init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_RUN_INTHREAD);

	if (timeout_ms >= 0) {
		cmdobj->sctx = &sctx;
		rtw_sctx_init(&sctx, timeout_ms);
	}

	res = rtw_enqueue_cmd(cmdpriv, cmdobj);

	if (res == _SUCCESS && timeout_ms >= 0) {
		rtw_sctx_wait(&sctx, __func__);
		_rtw_mutex_lock_interruptible(&cmdpriv->sctx_mutex);
		if (sctx.status == RTW_SCTX_SUBMITTED)
			cmdobj->sctx = NULL;
		_rtw_mutex_unlock(&cmdpriv->sctx_mutex);
		if (sctx.status != RTW_SCTX_DONE_SUCCESS)
			res = _FAIL;
	}

exit:
	return res;
}
u8 rtw_run_in_thread_cmd(_adapter *adapter, void (*func)(void *), void *context)
{
	return _rtw_run_in_thread_cmd(adapter, func, context, -1);
}

u8 rtw_run_in_thread_cmd_wait(_adapter *adapter, void (*func)(void *), void *context, s32 timeout_ms)
{
	return _rtw_run_in_thread_cmd(adapter, func, context, timeout_ms);
}


u8 session_tracker_cmd(_adapter *adapter, u8 cmd, struct sta_info *sta, u8 *local_naddr, u8 *local_port, u8 *remote_naddr, u8 *remote_port)
{
	struct cmd_priv	*cmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *cmd_parm;
	struct st_cmd_parm *st_parm;
	u8	res = _SUCCESS;

	cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmdobj == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmdobj->padapter = adapter;

	cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (cmd_parm == NULL) {
		rtw_mfree((u8 *)cmdobj, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	st_parm = (struct st_cmd_parm *)rtw_zmalloc(sizeof(struct st_cmd_parm));
	if (st_parm == NULL) {
		rtw_mfree((u8 *)cmdobj, sizeof(struct cmd_obj));
		rtw_mfree((u8 *)cmd_parm, sizeof(struct drvextra_cmd_parm));
		res = _FAIL;
		goto exit;
	}

	st_parm->cmd = cmd;
	st_parm->sta = sta;
	if (cmd != ST_CMD_CHK) {
		_rtw_memcpy(&st_parm->local_naddr, local_naddr, 4);
		_rtw_memcpy(&st_parm->local_port, local_port, 2);
		_rtw_memcpy(&st_parm->remote_naddr, remote_naddr, 4);
		_rtw_memcpy(&st_parm->remote_port, remote_port, 2);
	}

	cmd_parm->ec_id = SESSION_TRACKER_WK_CID;
	cmd_parm->type = 0;
	cmd_parm->size = sizeof(struct st_cmd_parm);
	cmd_parm->pbuf = (u8 *)st_parm;
	init_h2fwcmd_w_parm_no_rsp(cmdobj, cmd_parm, CMD_SET_DRV_EXTRA);
	cmdobj->no_io = 1;

	res = rtw_enqueue_cmd(cmdpriv, cmdobj);

exit:
	return res;
}

inline u8 session_tracker_chk_cmd(_adapter *adapter, struct sta_info *sta)
{
	return session_tracker_cmd(adapter, ST_CMD_CHK, sta, NULL, NULL, NULL, NULL);
}

inline u8 session_tracker_add_cmd(_adapter *adapter, struct sta_info *sta, u8 *local_naddr, u8 *local_port, u8 *remote_naddr, u8 *remote_port)
{
	return session_tracker_cmd(adapter, ST_CMD_ADD, sta, local_naddr, local_port, remote_naddr, remote_port);
}

inline u8 session_tracker_del_cmd(_adapter *adapter, struct sta_info *sta, u8 *local_naddr, u8 *local_port, u8 *remote_naddr, u8 *remote_port)
{
	return session_tracker_cmd(adapter, ST_CMD_DEL, sta, local_naddr, local_port, remote_naddr, remote_port);
}

void session_tracker_chk_for_sta(_adapter *adapter, struct sta_info *sta)
{
	struct st_ctl_t *st_ctl = &sta->st_ctl;
	int i;
	_list *plist, *phead, *pnext;
	_list dlist;
	struct session_tracker *st = NULL;
	u8 op_wfd_mode = MIRACAST_DISABLED;

	if (DBG_SESSION_TRACKER)
		RTW_INFO(FUNC_ADPT_FMT" sta:%p\n", FUNC_ADPT_ARG(adapter), sta);

	if (!(sta->state & WIFI_ASOC_STATE))
		goto exit;

	for (i = 0; i < SESSION_TRACKER_REG_ID_NUM; i++) {
		if (st_ctl->reg[i].s_proto != 0)
			break;
	}
	if (i >= SESSION_TRACKER_REG_ID_NUM)
		goto chk_sta;

	_rtw_init_listhead(&dlist);

	_rtw_spinlock_bh(&st_ctl->tracker_q.lock);

	phead = &st_ctl->tracker_q.queue;
	plist = get_next(phead);
	pnext = get_next(plist);
	while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		st = LIST_CONTAINOR(plist, struct session_tracker, list);
		plist = pnext;
		pnext = get_next(pnext);

		if (st->status != ST_STATUS_ESTABLISH
			&& rtw_get_passing_time_ms(st->set_time) > ST_EXPIRE_MS
		) {
			rtw_list_delete(&st->list);
			rtw_list_insert_tail(&st->list, &dlist);
		}

		/* TODO: check OS for status update */
		if (st->status == ST_STATUS_CHECK)
			st->status = ST_STATUS_ESTABLISH;

		if (st->status != ST_STATUS_ESTABLISH)
			continue;

		#ifdef CONFIG_WFD
		if (0)
			RTW_INFO(FUNC_ADPT_FMT" local:%u, remote:%u, rtsp:%u, %u, %u\n", FUNC_ADPT_ARG(adapter)
				, ntohs(st->local_port), ntohs(st->remote_port), adapter->wfd_info.rtsp_ctrlport, adapter->wfd_info.tdls_rtsp_ctrlport
				, adapter->wfd_info.peer_rtsp_ctrlport);
		if (ntohs(st->local_port) == adapter->wfd_info.rtsp_ctrlport)
			op_wfd_mode |= MIRACAST_SINK;
		if (ntohs(st->local_port) == adapter->wfd_info.tdls_rtsp_ctrlport)
			op_wfd_mode |= MIRACAST_SINK;
		if (ntohs(st->remote_port) == adapter->wfd_info.peer_rtsp_ctrlport)
			op_wfd_mode |= MIRACAST_SOURCE;
		#endif
	}

	_rtw_spinunlock_bh(&st_ctl->tracker_q.lock);

	plist = get_next(&dlist);
	while (rtw_end_of_queue_search(&dlist, plist) == _FALSE) {
		st = LIST_CONTAINOR(plist, struct session_tracker, list);
		plist = get_next(plist);
		rtw_mfree((u8 *)st, sizeof(struct session_tracker));
	}

chk_sta:
	if (STA_OP_WFD_MODE(sta) != op_wfd_mode) {
		STA_SET_OP_WFD_MODE(sta, op_wfd_mode);
		rtw_sta_media_status_rpt_cmd(adapter, sta, 1);
	}

exit:
	return;
}

void session_tracker_chk_for_adapter(_adapter *adapter)
{
	struct sta_priv *stapriv = &adapter->stapriv;
	struct sta_info *sta;
	int i;
	_list *plist, *phead;
	u8 op_wfd_mode = MIRACAST_DISABLED;

	_rtw_spinlock_bh(&stapriv->sta_hash_lock);

	for (i = 0; i < NUM_STA; i++) {
		phead = &(stapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			sta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);

			session_tracker_chk_for_sta(adapter, sta);

			op_wfd_mode |= STA_OP_WFD_MODE(sta);
		}
	}

	_rtw_spinunlock_bh(&stapriv->sta_hash_lock);

#ifdef CONFIG_WFD
	adapter->wfd_info.op_wfd_mode = MIRACAST_MODE_REVERSE(op_wfd_mode);
#endif
}

void session_tracker_cmd_hdl(_adapter *adapter, struct st_cmd_parm *parm)
{
	u8 cmd = parm->cmd;
	struct sta_info *sta = parm->sta;

	if (cmd == ST_CMD_CHK) {
		if (sta)
			session_tracker_chk_for_sta(adapter, sta);
		else
			session_tracker_chk_for_adapter(adapter);

		goto exit;

	} else if (cmd == ST_CMD_ADD || cmd == ST_CMD_DEL) {
		struct st_ctl_t *st_ctl;
		u32 local_naddr = parm->local_naddr;
		u16 local_port = parm->local_port;
		u32 remote_naddr = parm->remote_naddr;
		u16 remote_port = parm->remote_port;
		struct session_tracker *st = NULL;
		_list *plist, *phead;
		u8 free_st = 0;
		u8 alloc_st = 0;

		if (DBG_SESSION_TRACKER)
			RTW_INFO(FUNC_ADPT_FMT" cmd:%u, sta:%p, local:"IP_FMT":"PORT_FMT", remote:"IP_FMT":"PORT_FMT"\n"
				, FUNC_ADPT_ARG(adapter), cmd, sta
				, IP_ARG(&local_naddr), PORT_ARG(&local_port)
				, IP_ARG(&remote_naddr), PORT_ARG(&remote_port)
			);

		if (!(sta->state & WIFI_ASOC_STATE))
			goto exit;

		st_ctl = &sta->st_ctl;

		_rtw_spinlock_bh(&st_ctl->tracker_q.lock);

		phead = &st_ctl->tracker_q.queue;
		plist = get_next(phead);
		while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
			st = LIST_CONTAINOR(plist, struct session_tracker, list);

			if (st->local_naddr == local_naddr
				&& st->local_port == local_port
				&& st->remote_naddr == remote_naddr
				&& st->remote_port == remote_port)
				break;

			plist = get_next(plist);
		}

		if (rtw_end_of_queue_search(phead, plist) == _TRUE)
			st = NULL;

		switch (cmd) {
		case ST_CMD_DEL:
			if (st) {
				rtw_list_delete(plist);
				free_st = 1;
			}
			goto unlock;
		case ST_CMD_ADD:
			if (!st)
				alloc_st = 1;
		}

unlock:
		_rtw_spinunlock_bh(&st_ctl->tracker_q.lock);

		if (free_st) {
			rtw_mfree((u8 *)st, sizeof(struct session_tracker));
			goto exit;
		}

		if (alloc_st) {
			st = (struct session_tracker *)rtw_zmalloc(sizeof(struct session_tracker));
			if (!st)
				goto exit;

			st->local_naddr = local_naddr;
			st->local_port = local_port;
			st->remote_naddr = remote_naddr;
			st->remote_port = remote_port;
			st->set_time = rtw_get_current_time();
			st->status = ST_STATUS_CHECK;

			_rtw_spinlock_bh(&st_ctl->tracker_q.lock);
			rtw_list_insert_tail(&st->list, phead);
			_rtw_spinunlock_bh(&st_ctl->tracker_q.lock);
		}
	}

exit:
	return;
}

#if defined(CONFIG_RTW_MESH) && defined(RTW_PER_CMD_SUPPORT_FW)
static s32 rtw_req_per_cmd_hdl(_adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct macid_ctl_t *macid_ctl = dvobj_to_macidctl(dvobj);
	struct macid_bmp req_macid_bmp, *macid_bmp;
	u8 i, ret = _FAIL;

	macid_bmp = &macid_ctl->if_g[adapter->iface_id];
	_rtw_memcpy(&req_macid_bmp, macid_bmp, sizeof(struct macid_bmp));

	/* Clear none mesh's macid */
	for (i = 0; i < macid_ctl->num; i++) {
		u8 role;
		role = GET_H2CCMD_MSRRPT_PARM_ROLE(&macid_ctl->h2c_msr[i]);
		if (role != H2C_MSR_ROLE_MESH)
			rtw_macid_map_clr(&req_macid_bmp, i);
	}

	/* group_macid: always be 0 in NIC, so only pass macid_bitmap.m0
	 * rpt_type: 0 includes all info in 1, use 0 for now
	 * macid_bitmap: pass m0 only for NIC
	 */
	ret = rtw_hal_set_req_per_rpt_cmd(adapter, 0, 0, req_macid_bmp.m0);

	return ret;
}

u8 rtw_req_per_cmd(_adapter *adapter)
{
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	struct submit_ctx sctx;
	u8 res = _SUCCESS;

	parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (parm == NULL) {
		res = _FAIL;
		goto exit;
	}

	parm->ec_id = REQ_PER_CMD_WK_CID;
	parm->type = 0;
	parm->size = 0;
	parm->pbuf = NULL;

	cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(*cmdobj));
	if (cmdobj == NULL) {
		res = _FAIL;
		rtw_mfree((u8 *)parm, sizeof(*parm));
		goto exit;
	}
	cmdobj->padapter = adapter;

	init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_SET_DRV_EXTRA);

	res = rtw_enqueue_cmd(pcmdpriv, cmdobj);

exit:
	return res;
}
#endif


void rtw_ac_parm_cmd_hdl(_adapter *padapter, u8 *_ac_parm_buf, int ac_type)
{

	u32 ac_parm_buf;

	_rtw_memcpy(&ac_parm_buf, _ac_parm_buf, sizeof(ac_parm_buf));
	switch (ac_type) {
	case XMIT_VO_QUEUE:
		RTW_INFO(FUNC_NDEV_FMT" AC_VO = 0x%08x\n", FUNC_ADPT_ARG(padapter), (unsigned int) ac_parm_buf);
		rtw_hw_set_edca(padapter, 3, ac_parm_buf);
		break;

	case XMIT_VI_QUEUE:
		RTW_INFO(FUNC_NDEV_FMT" AC_VI = 0x%08x\n", FUNC_ADPT_ARG(padapter), (unsigned int) ac_parm_buf);
		rtw_hw_set_edca(padapter, 2, ac_parm_buf);
		break;

	case XMIT_BE_QUEUE:
		RTW_INFO(FUNC_NDEV_FMT" AC_BE = 0x%08x\n", FUNC_ADPT_ARG(padapter), (unsigned int) ac_parm_buf);
		rtw_hw_set_edca(padapter, 0, ac_parm_buf);
		break;

	case XMIT_BK_QUEUE:
		RTW_INFO(FUNC_NDEV_FMT" AC_BK = 0x%08x\n", FUNC_ADPT_ARG(padapter), (unsigned int) ac_parm_buf);
		rtw_hw_set_edca(padapter, 1, ac_parm_buf);
		break;

	default:
		break;
	}

}

char UNKNOWN_CID[16] = "UNKNOWN_EXTRA";
char *rtw_extra_name(struct drvextra_cmd_parm *pdrvextra_cmd)
{
	switch(pdrvextra_cmd->ec_id) {
	case NONE_WK_CID:
		return "NONE_WK_CID";
		break;
	case STA_MSTATUS_RPT_WK_CID:
		return "STA_MSTATUS_RPT_WK_CID";
		break;
	case DYNAMIC_CHK_WK_CID:
		return "DYNAMIC_CHK_WK_CID";
		break;
	case DM_CTRL_WK_CID:
		return "DM_CTRL_WK_CID";
		break;
	case PBC_POLLING_WK_CID:
		return "PBC_POLLING_WK_CID";
		break;
	#ifdef CONFIG_POWER_SAVING
	case POWER_SAVING_CTRL_WK_CID:
		return "POWER_SAVING_CTRL_WK_CID";
	#endif
		break;
	case LPS_CTRL_WK_CID:
		return "LPS_CTRL_WK_CID";
		break;
	case ANT_SELECT_WK_CID:
		return "ANT_SELECT_WK_CID";
		break;
	case P2P_PS_WK_CID:
		return "P2P_PS_WK_CID";
		break;
	case P2P_PROTO_WK_CID:
		return "P2P_PROTO_WK_CID";
		break;
	case CHECK_HIQ_WK_CID:
		return "CHECK_HIQ_WK_CID";
		break;
	case C2H_WK_CID:
		return "C2H_WK_CID";
		break;
	case RESET_SECURITYPRIV:
		return "RESET_SECURITYPRIV";
		break;
	case FREE_ASSOC_RESOURCES:
		return "FREE_ASSOC_RESOURCES";
		break;
	case DM_IN_LPS_WK_CID:
		return "DM_IN_LPS_WK_CID";
		break;
	case DM_RA_MSK_WK_CID:
		return "DM_RA_MSK_WK_CID";
		break;
	case BEAMFORMING_WK_CID:
		return "BEAMFORMING_WK_CID";
		break;
	case LPS_CHANGE_DTIM_CID:
		return "LPS_CHANGE_DTIM_CID";
		break;
	case DFS_RADAR_DETECT_WK_CID:
		return "DFS_RADAR_DETECT_WK_CID";
		break;
	case DFS_RADAR_DETECT_EN_DEC_WK_CID:
		return "DFS_RADAR_DETECT_EN_DEC_WK_CID";
		break;
	case SESSION_TRACKER_WK_CID:
		return "SESSION_TRACKER_WK_CID";
		break;
	case TEST_H2C_CID:
		return "TEST_H2C_CID";
		break;
	case MP_CMD_WK_CID:
		return "MP_CMD_WK_CID";
		break;
	case CUSTOMER_STR_WK_CID:
		return "CUSTOMER_STR_WK_CID";
		break;
	case MGNT_TX_WK_CID:
		return "MGNT_TX_WK_CID";
		break;
	case REQ_PER_CMD_WK_CID:
		return "REQ_PER_CMD_WK_CID";
		break;
	case SSMPS_WK_CID:
		return "SSMPS_WK_CID";
		break;
#ifdef CONFIG_CTRL_TXSS_BY_TP
	case TXSS_WK_CID:
		return "TXSS_WK_CID";
		break;
#endif
	case AC_PARM_CMD_WK_CID:
		return "AC_PARM_CMD_WK_CID";
		break;
#ifdef CONFIG_AP_MODE
	case STOP_AP_WK_CID:
		return "STOP_AP_WK_CID";
		break;
#endif
#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
	case TBTX_CONTROL_TX_WK_CID:
		return "TBTX_CONTROL_TX_WK_CID";
		break;
#endif
	case MAX_WK_CID:
		return "MAX_WK_CID";
		break;
	default:
		return UNKNOWN_CID;
		break;
	}
	return UNKNOWN_CID;
}

u8 rtw_drvextra_cmd_hdl(_adapter *padapter, unsigned char *pbuf)
{
	int ret = H2C_SUCCESS;
	struct drvextra_cmd_parm *pdrvextra_cmd;

	if (!pbuf)
		return H2C_PARAMETERS_ERROR;

	pdrvextra_cmd = (struct drvextra_cmd_parm *)pbuf;

	switch (pdrvextra_cmd->ec_id) {
	case STA_MSTATUS_RPT_WK_CID:
		rtw_sta_media_status_rpt_cmd_hdl(padapter, (struct sta_media_status_rpt_cmd_parm *)pdrvextra_cmd->pbuf);
		break;

	case DYNAMIC_CHK_WK_CID:/*only  primary padapter go to this cmd, but execute dynamic_chk_wk_hdl() for two interfaces */
		rtw_dynamic_chk_wk_hdl(padapter);
		break;
#ifdef CONFIG_POWER_SAVING
	case POWER_SAVING_CTRL_WK_CID:
		power_saving_wk_hdl(padapter);
		break;
#endif
#ifdef CONFIG_LPS
	case LPS_CTRL_WK_CID:
		lps_ctrl_wk_hdl(padapter, (u8)pdrvextra_cmd->type, pdrvextra_cmd->pbuf);
		break;
	case DM_IN_LPS_WK_CID:
		rtw_dm_in_lps_hdl(padapter);
		break;
	case LPS_CHANGE_DTIM_CID:
		rtw_lps_change_dtim_hdl(padapter, (u8)pdrvextra_cmd->type);
		break;
#endif
#ifdef CONFIG_ANTENNA_DIVERSITY
	case ANT_SELECT_WK_CID:
		antenna_select_wk_hdl(padapter, pdrvextra_cmd->type);
		break;
#endif
#ifdef CONFIG_P2P_PS
	case P2P_PS_WK_CID:
		p2p_ps_wk_hdl(padapter, pdrvextra_cmd->type);
		break;
#endif
#ifdef CONFIG_P2P
	case P2P_PROTO_WK_CID:
		/*
		* Commented by Albert 2011/07/01
		* I used the type_size as the type command
		*/
		ret = p2p_protocol_wk_hdl(padapter, pdrvextra_cmd->type, pdrvextra_cmd->pbuf);
		break;
#endif
#ifdef CONFIG_AP_MODE
	case CHECK_HIQ_WK_CID:
		rtw_chk_hi_queue_hdl(padapter);
		break;
#endif
	/* add for CONFIG_IEEE80211W, none 11w can use it */
	case RESET_SECURITYPRIV:
		reset_securitypriv_hdl(padapter);
		break;
	case FREE_ASSOC_RESOURCES:
		free_assoc_resources_hdl(padapter, (u8)pdrvextra_cmd->type);
		break;
	case C2H_WK_CID:
		switch (pdrvextra_cmd->type) {
		case C2H_TYPE_PKT:
			rtw_hal_c2h_pkt_hdl(padapter, pdrvextra_cmd->pbuf, pdrvextra_cmd->size);
			break;
		default:
			RTW_ERR("unknown C2H type:%d\n", pdrvextra_cmd->type);
			rtw_warn_on(1);
			break;
		}
		break;
#ifdef CONFIG_BEAMFORMING
	case BEAMFORMING_WK_CID:
		beamforming_wk_hdl(padapter, pdrvextra_cmd->type, pdrvextra_cmd->pbuf);
		break;
#endif
	case DM_RA_MSK_WK_CID:
		rtw_dm_ra_mask_hdl(padapter, (struct sta_info *)pdrvextra_cmd->pbuf);
		break;
#ifdef CONFIG_DFS_MASTER
	case DFS_RADAR_DETECT_WK_CID:
		rtw_dfs_rd_hdl(padapter);
		break;
	case DFS_RADAR_DETECT_EN_DEC_WK_CID:
		rtw_dfs_rd_en_decision(padapter, MLME_ACTION_NONE, 0);
		break;
#endif
	case SESSION_TRACKER_WK_CID:
		session_tracker_cmd_hdl(padapter, (struct st_cmd_parm *)pdrvextra_cmd->pbuf);
		break;
	case TEST_H2C_CID:
		rtw_hal_fill_h2c_cmd(padapter, pdrvextra_cmd->pbuf[0], pdrvextra_cmd->size - 1, &pdrvextra_cmd->pbuf[1]);
		break;
	case MP_CMD_WK_CID:
#ifdef CONFIG_MP_INCLUDED
		ret = rtw_mp_cmd_hdl(padapter, pdrvextra_cmd->type);
#endif
		break;
#ifdef CONFIG_RTW_CUSTOMER_STR
	case CUSTOMER_STR_WK_CID:
		ret = rtw_customer_str_cmd_hdl(padapter, pdrvextra_cmd->type, pdrvextra_cmd->pbuf);
		break;
#endif

#ifdef CONFIG_IOCTL_CFG80211
	case MGNT_TX_WK_CID:
		ret = rtw_mgnt_tx_handler(padapter, pdrvextra_cmd->pbuf);
		break;
#endif /* CONFIG_IOCTL_CFG80211 */
#ifdef CONFIG_MCC_MODE
	case MCC_CMD_WK_CID:
		ret = rtw_mcc_cmd_hdl(padapter, pdrvextra_cmd->type, pdrvextra_cmd->pbuf);
		break;
#endif /* CONFIG_MCC_MODE */
#if defined(CONFIG_RTW_MESH) && defined(RTW_PER_CMD_SUPPORT_FW)
	case REQ_PER_CMD_WK_CID:
		ret = rtw_req_per_cmd_hdl(padapter);
		break;
#endif
#if defined(CONFIG_SUPPORT_STATIC_SMPS) || defined(CONFIG_AP_MODE)
	case SSMPS_WK_CID :
		rtw_ssmps_wk_hdl(padapter, (struct ssmps_cmd_parm *)pdrvextra_cmd->pbuf);
		break;
#endif
#ifdef CONFIG_CTRL_TXSS_BY_TP
	case TXSS_WK_CID :
		rtw_ctrl_txss_wk_hdl(padapter, (struct txss_cmd_parm *)pdrvextra_cmd->pbuf);
		break;
#endif
	case AC_PARM_CMD_WK_CID:
		rtw_ac_parm_cmd_hdl(padapter, pdrvextra_cmd->pbuf, pdrvextra_cmd->type);
		break;
#ifdef CONFIG_AP_MODE
	case STOP_AP_WK_CID:
		stop_ap_hdl(padapter);
		break;
#endif
#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
	case TBTX_CONTROL_TX_WK_CID:
		tx_control_hdl(padapter);
		break;
#endif
	default:
		break;
	}

	if (pdrvextra_cmd->pbuf && pdrvextra_cmd->size > 0)
		rtw_mfree(pdrvextra_cmd->pbuf, pdrvextra_cmd->size);

	return ret;
}

void rtw_survey_cmd_callback(_adapter *padapter ,  struct cmd_obj *pcmd)
{
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;


	if (pcmd->res == H2C_DROPPED) {
		/* TODO: cancel timer and do timeout handler directly... */
		/* need to make timeout handlerOS independent */
		mlme_set_scan_to_timer(pmlmepriv, 1);
	} else if (pcmd->res != H2C_SUCCESS) {
		mlme_set_scan_to_timer(pmlmepriv, 1);
	}

	/* free cmd */
	rtw_free_cmd_obj(pcmd);

}
void rtw_disassoc_cmd_callback(_adapter *padapter,  struct cmd_obj *pcmd)
{
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;


	if (pcmd->res != H2C_SUCCESS) {
		_rtw_spinlock_bh(&pmlmepriv->lock);
		set_fwstate(pmlmepriv, WIFI_ASOC_STATE);
		_rtw_spinunlock_bh(&pmlmepriv->lock);
		goto exit;
	}
#ifdef CONFIG_BR_EXT
	else /* clear bridge database */
		nat25_db_cleanup_g6(padapter);
#endif /* CONFIG_BR_EXT */

	/* free cmd */
	rtw_free_cmd_obj(pcmd);

exit:
	return;
}
void rtw_joinbss_cmd_callback(_adapter *padapter,  struct cmd_obj *pcmd)
{
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;


	if (pcmd->res == H2C_DROPPED) {
		/* TODO: cancel timer and do timeout handler directly... */
		/* need to make timeout handlerOS independent */
		_set_timer(&pmlmepriv->assoc_timer, 1);
	} else if (pcmd->res != H2C_SUCCESS)
		_set_timer(&pmlmepriv->assoc_timer, 1);

	rtw_free_cmd_obj(pcmd);

}

void rtw_create_ibss_post_hdl(_adapter *padapter, int status)
{
	struct wlan_network *pwlan = NULL;
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;
#ifdef RTW_MI_SHARE_BSS_LIST
	_queue *queue = &padapter->dvobj->scanned_queue;
#else
	_queue *queue = &(pmlmepriv->scanned_queue);
#endif
	WLAN_BSSID_EX *pdev_network = &padapter->registrypriv.dev_network;
	struct wlan_network *mlme_cur_network = &(pmlmepriv->cur_network);

	if (status != H2C_SUCCESS)
		_set_timer(&pmlmepriv->assoc_timer, 1);

	_cancel_timer_ex(&pmlmepriv->assoc_timer);

	_rtw_spinlock_bh(&pmlmepriv->lock);

	{
		pwlan = _rtw_alloc_network(pmlmepriv);
		_rtw_spinlock_bh(&(queue->lock));
		if (pwlan == NULL) {
			pwlan = rtw_get_oldest_wlan_network(queue);
			if (pwlan == NULL) {
				_rtw_spinunlock_bh(&(queue->lock));
				goto createbss_cmd_fail;
			}
			pwlan->last_scanned = rtw_get_current_time();
		} else
			rtw_list_insert_tail(&(pwlan->list), &queue->queue);

		pdev_network->Length = get_WLAN_BSSID_EX_sz(pdev_network);
		_rtw_memcpy(&(pwlan->network), pdev_network, pdev_network->Length);
		/* pwlan->fixed = _TRUE; */

		/* copy pdev_network information to pmlmepriv->cur_network */
		_rtw_memcpy(&mlme_cur_network->network, pdev_network, (get_WLAN_BSSID_EX_sz(pdev_network)));

#if 0
		/* reset DSConfig */
		mlme_cur_network->network.Configuration.DSConfig = (u32)rtw_ch2freq(pdev_network->Configuration.DSConfig);
#endif

		_clr_fwstate_(pmlmepriv, WIFI_UNDER_LINKING);
		_rtw_spinunlock_bh(&(queue->lock));
		/* we will set WIFI_ASOC_STATE when there is one more sat to join us (rtw_stassoc_event_callback) */
	}

createbss_cmd_fail:
	_rtw_spinunlock_bh(&pmlmepriv->lock);
	return;
}



void rtw_setstaKey_cmdrsp_callback(_adapter *padapter ,  struct cmd_obj *pcmd)
{

	struct sta_priv *pstapriv = &padapter->stapriv;
	struct set_stakey_rsp *psetstakey_rsp = (struct set_stakey_rsp *)(pcmd->rsp);
	struct sta_info	*psta = rtw_get_stainfo(pstapriv, psetstakey_rsp->addr);


	if (psta == NULL) {
		goto exit;
	}

	/* psta->phl_sta->aid = psta->phl_sta->macid = psetstakey_rsp->keyid; */ /* CAM_ID(CAM_ENTRY) */

exit:

	rtw_free_cmd_obj(pcmd);


}

void rtw_getrttbl_cmd_cmdrsp_callback(_adapter *padapter,  struct cmd_obj *pcmd)
{

	rtw_free_cmd_obj(pcmd);
#ifdef CONFIG_MP_INCLUDED
	if (padapter->registrypriv.mp_mode == 1)
		padapter->mppriv.workparam.bcompleted = _TRUE;
#endif


}

u8 set_txq_params_cmd(_adapter *adapter, u32 ac_parm, u8 ac_type)
{
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	u8 *ac_parm_buf = NULL;
	u8 sz;
	u8 res = _SUCCESS;


	cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmdobj == NULL) {
		res = _FAIL;
		goto exit;
	}
	cmdobj->padapter = adapter;

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((u8 *)cmdobj, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	sz = sizeof(ac_parm);
	ac_parm_buf = rtw_zmalloc(sz);
	if (ac_parm_buf == NULL) {
		rtw_mfree((u8 *)cmdobj, sizeof(struct cmd_obj));
		rtw_mfree((u8 *)pdrvextra_cmd_parm, sizeof(struct drvextra_cmd_parm));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = AC_PARM_CMD_WK_CID;
	pdrvextra_cmd_parm->type = ac_type;
	pdrvextra_cmd_parm->size = sz;
	pdrvextra_cmd_parm->pbuf = ac_parm_buf;

	_rtw_memcpy(ac_parm_buf, &ac_parm, sz);

	init_h2fwcmd_w_parm_no_rsp(cmdobj, pdrvextra_cmd_parm, CMD_SET_DRV_EXTRA);
	res = rtw_enqueue_cmd(pcmdpriv, cmdobj);

exit:
	return res;
}

char UNKNOWN_CMD[16] = "UNKNOWN_CMD";
char *rtw_cmd_name(struct cmd_obj *pcmd)
{
	struct rtw_evt_header *pev;

	if (pcmd->cmdcode >= (sizeof(wlancmds) / sizeof(struct rtw_cmd)))
		return UNKNOWN_CMD;

	if (pcmd->cmdcode == CMD_SET_MLME_EVT)
		return rtw_evt_name((struct rtw_evt_header*)pcmd->parmbuf);

	if (pcmd->cmdcode == CMD_SET_DRV_EXTRA)
		return rtw_extra_name((struct drvextra_cmd_parm*)pcmd->parmbuf);

	return wlancmds[pcmd->cmdcode].name;
}

#ifdef CONFIG_RTW_HANDLE_SER_L2
thread_return rtw_dev_recovery_thread(thread_context context)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)context;
	rtw_thread_enter("RTW_DEV_RECOVERY_THREAD");
	RTW_PRINT("Running device recovery thread...\n");
	rtw_recover_device(dvobj);

	return 0;
}

u8 rtw_recover_dev_cmd(_adapter *padapter)
{
	rtw_thread_start(rtw_dev_recovery_thread, padapter->dvobj,
	                 "RTW_DEV_RECOVERY_THREAD");
	return _SUCCESS;
}
#endif /* CONFIG_RTW_HANDLE_SER_L2 */

