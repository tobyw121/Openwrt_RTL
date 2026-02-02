/******************************************************************************
 *
 * Copyright(c) 2019 - 2020 Realtek Corporation.
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
#include "rtw_mlme_ext.h"

#define PHL_RES2RES(a) (a == RTW_PHL_STATUS_SUCCESS)?_SUCCESS:_FAIL
#ifdef CONFIG_FSM
int rtw_phl_scan_abort(_adapter *padapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	rtw_phl_scan_cancel(dvobj->phl);
	return 0;
}
#endif

static u8 scan_p2p_find_check(_adapter *padapter)
{
	u8 is_p2p_find = false;

	RTW_INFO("%s, is_p2p_find=%d", __func__, is_p2p_find);
	return is_p2p_find;
}

static u8 scan_back_op_check(_adapter *padapter)
{
	u8 do_back_op = false;
	u8 connected = padapter->registrypriv.wifi_mib.scan_backop;

	/* need to check if doing back_op */
	if (connected)
		do_back_op = true;

	RTW_INFO("%s, do_back_op=%d\n", __func__, do_back_op);
	return do_back_op;
}

/**
 * prepare phl_channel list according to SCAN type
 *
 */
static u8 scan_channel_list_preparation(_adapter *padapter,
	struct rtw_phl_scan_param *dst, struct sitesurvey_parm *src)
{
#define BACKOP_CH_DIV 3
#define P2P_FIND_CH_DIV 3
#define TEST_MODE_CH_DIV 3
	u8 ch_num_t; /* temp ch num for internal operation */
	u8 ch_num = 0; /* return value : ch_num in final result*/
	struct phl_scan_channel *phl_ch = NULL;
	int phl_ch_sz = 0;
	int i, j;
	u8 ch_num_of_backop;
	u8 scan_mode;
	u8 backop_ch_div = BACKOP_CH_DIV;

	ch_num_t = src->ch_num;
	if ((padapter->registrypriv.wifi_mib.scan_backop_div > 0)
			&& (padapter->registrypriv.wifi_mib.scan_backop_div < ch_num_t))
		backop_ch_div = padapter->registrypriv.wifi_mib.scan_backop_div;

	/* normal scan with back op */
	if (!src->acs && scan_back_op_check(padapter) == true &&
		src->is_p2p == false) {

		if (padapter->registrypriv.wifi_mib.scan_backop_dur == 0)
			padapter->registrypriv.wifi_mib.scan_backop_dur = 100;
		ch_num_of_backop = ch_num_t + (ch_num_t / backop_ch_div);

		RTW_INFO("ch_num_of_backop=%d\n", ch_num_of_backop);

		if (phl_ch)
			rtw_mfree(phl_ch, phl_ch_sz);

		phl_ch_sz = sizeof(struct phl_scan_channel) *
			(ch_num_of_backop);

		phl_ch = rtw_malloc(phl_ch_sz);
		if (phl_ch == NULL) {
			RTW_ERR("scan: alloc phl scan ch fail\n");
			return ch_num;
		}
		_rtw_memset(phl_ch, 0, phl_ch_sz);

		i = j = 0;
		while (i < src->ch_num) {

			phl_ch[j].channel = src->ch[i].hw_value;

			if (src->ch[i].flags & RTW_IEEE80211_CHAN_PASSIVE_SCAN) {
				phl_ch[j].type = RTW_PHL_SCAN_PASSIVE;
				phl_ch[j].duration = src->duration;
			} else {
				phl_ch[j].type = RTW_PHL_SCAN_ACTIVE;
				if (src->acs)
					phl_ch[j].duration = src->duration;
				else
					phl_ch[j].duration = src->duration >> 1;
			}

			phl_ch[j].scan_mode = NORMAL_SCAN_MODE;
			phl_ch[j].bw = src->bw;
			i++;
			j++;

			if ((i % backop_ch_div) == 0) {
				/* TODO op channel,
				 * now harecode just for test only
				 */
				phl_ch[j].channel = padapter->mlmeextpriv.cur_channel;
				phl_ch[j].duration = padapter->registrypriv.wifi_mib.scan_backop_dur;
				phl_ch[j].type = RTW_PHL_SCAN_PASSIVE;
				phl_ch[j].scan_mode = BACKOP_MODE;

				/* bw for op channel,
				 * now harecode just for test only
				 */
				phl_ch[j].bw = 0;
				j++;
			}
		}

		if (ch_num_of_backop == j ) {
			ch_num_t = ch_num_of_backop;
			RTW_INFO("%s, ch_num_of_backop = %d\n",
				__func__, ch_num_of_backop);
		} else
			RTW_INFO("if got here, you'll know it's wrong\n");

	} else { /* normal scan without back op or p2p scan; */

		phl_ch_sz = sizeof(struct phl_scan_channel) *
			(ch_num_t + 1);

		phl_ch = rtw_malloc(phl_ch_sz);
		if (phl_ch == NULL) {
			RTW_ERR("scan: alloc phl scan ch fail\n");
			return ch_num;
		}
		_rtw_memset(phl_ch, 0, phl_ch_sz);

		if (src->is_p2p)
			scan_mode = P2P_SCAN_MODE;
		else
			scan_mode = NORMAL_SCAN_MODE;

		i = 0;
		while (i < src->ch_num) {

			phl_ch[i].channel = src->ch[i].hw_value;
			phl_ch[i].scan_mode = scan_mode;
			phl_ch[i].bw = src->bw;
			phl_ch[i].duration = src->duration;

			if (src->ch[i].flags & RTW_IEEE80211_CHAN_PASSIVE_SCAN) {
				phl_ch[i].type = RTW_PHL_SCAN_PASSIVE;

			} else {
				phl_ch[i].type = RTW_PHL_SCAN_ACTIVE;
				/* reduce scan time in active channel */
				if (src->is_p2p == false && src->acs == false)
					phl_ch[i].duration = src->duration >> 1;
			}
			i++;
		}
	}

	if (0) { /* TEST MODE */

		u8 ch_num_of_test;
		int sz;

		ch_num_of_test =  ch_num_t + (ch_num_t/TEST_MODE_CH_DIV);

		if (phl_ch)
			rtw_mfree(phl_ch, phl_ch_sz);

		sz = (sizeof(struct phl_scan_channel)*ch_num_of_test)+1;
		phl_ch = rtw_malloc(sz);
		if (phl_ch == NULL) {
			RTW_ERR("scan: alloc phl scan ch fail\n");
			return ch_num;
		}
		_rtw_memset(phl_ch, 0, sz);
		ch_num_t = ch_num_of_test;
	}

	/* finally, ... */
	dst->ch = phl_ch;
	dst->ch_sz = phl_ch_sz;
	dst->ch_num = ch_num_t;
	ch_num	=  ch_num_t;

	return ch_num;
}

#ifdef CONFIG_CMD_GENERAL/*for warkaround*/
static void phl_run_core_cmd(void *drv_priv, u8 *cmd, u32 cmd_len, enum rtw_phl_status status)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)drv_priv;
	struct cmd_obj *pcmd = (struct cmd_obj *)cmd;
	_adapter *padapter = pcmd->padapter;
	struct cmd_priv *pcmdpriv = &dvobj->cmdpriv;

	if (status == RTW_PHL_STATUS_CANNOT_IO ||
		status == RTW_PHL_STATUS_CMD_ERROR ||
		RTW_CANNOT_RUN(dvobj)) {
		RTW_INFO(FUNC_ADPT_FMT "%s FALSE -bDriverStopped(%s) bSurpriseRemoved(%s)\n"
			, FUNC_ADPT_ARG(padapter)
			, rtw_cmd_name(pcmd)
			, dev_is_drv_stopped(dvobj) ? "True" : "False"
			, dev_is_surprise_removed(dvobj) ? "True" : "False");

		if (pcmd->cmdcode == CMD_SET_DRV_EXTRA) {
			struct drvextra_cmd_parm *extra_parm =
				(struct drvextra_cmd_parm *)pcmd->parmbuf;

			if (extra_parm->pbuf && (extra_parm->size > 0))
				rtw_mfree(extra_parm->pbuf, extra_parm->size);
		}

		_rtw_mutex_lock(&pcmdpriv->sctx_mutex);
		if (pcmd->sctx) {
			if (0)
				RTW_PRINT(FUNC_ADPT_FMT" pcmd->sctx\n", FUNC_ADPT_ARG(pcmd->padapter));
			rtw_sctx_done_err(&pcmd->sctx, RTW_SCTX_DONE_CMD_DROP);
		}
		_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
		rtw_free_cmd_obj(pcmd);
		return;
	}

	if (status == RTW_PHL_STATUS_CMD_TIMEOUT)
		RTW_ERR("%s %s cmd timeout\n", __func__, rtw_cmd_name(pcmd));
	else if (status == RTW_PHL_STATUS_FAILURE) /*PHL fail due to 1. CMD_DROP:cmd abort or cancel 2.CMD_FAIL*/
		RTW_ERR("%s %s cmd failure\n", __func__, rtw_cmd_name(pcmd));

	rtw_run_cmd(padapter, pcmd, false);
}

u32 rtw_enqueue_phl_cmd(struct cmd_obj *pcmd)
{
	u32 res = _FAIL;
	_adapter *padapter = pcmd->padapter;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

	if(!padapter->netif_up)
		goto free_cmd;

	psts = rtw_phl_cmd_enqueue(dvobj->phl,
			padapter->phl_role->hw_band,
			MSG_EVT_LINUX_CMD_WRK,
			(u8 *)pcmd, sizeof(struct cmd_obj),
			phl_run_core_cmd,
			PHL_CMD_NO_WAIT, 0);

free_cmd:
	if (is_cmd_failure(psts)) {
		RTW_ERR(ADPT_FMT" cmd %u to PHL failed. (%d)",
		        ADPT_ARG(padapter), pcmd->cmdcode, psts);
		rtw_free_cmd_obj(pcmd);
		res = _FAIL;
	} else {
		res = _SUCCESS;
	}

	return res;
}

#else /*CONFIG_FSM*/

void phl_run_core_cmd(void *priv, void *parm, bool discard)
{
	_adapter *padapter = (_adapter *)priv;
	struct cmd_obj *pcmd = (struct cmd_obj *)parm;

	rtw_run_cmd(padapter, pcmd, discard);
}

u32 rtw_enqueue_phl_cmd(struct cmd_obj *pcmd)
{
	_adapter *padapter = pcmd->padapter;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	u32 res = RTW_PHL_STATUS_FAILURE;

	switch (pcmd->cmdcode) {
	case CMD_SITE_SURVEY:
	{
		struct rtw_phl_scan_param *phl_param;
		struct sitesurvey_parm *rtw_param;
		struct rtw_ieee80211_channel ch[RTW_CHANNEL_SCAN_AMOUNT];

		rtw_param = (struct sitesurvey_parm *)pcmd->parmbuf;

		if (rtw_param->duration == 0) {
			if (rtw_param->acs == 1)
				rtw_param->duration = 300; /* ms */
			else if(rtw_param->acs == 2) // quick scan enabled
				rtw_param->duration = 100; /* ms */
			else
				rtw_param->duration = 100; /* ms */
		}

		/* backup original ch list */
		_rtw_memcpy(ch, rtw_param->ch,
			sizeof(struct rtw_ieee80211_channel) *
			rtw_param->ch_num);

		/* modify ch list according to chanel plan */
		rtw_param->ch_num = rtw_scan_ch_decision(padapter,
			rtw_param->ch, RTW_CHANNEL_SCAN_AMOUNT,
			ch, rtw_param->ch_num, rtw_param->acs);

		phl_param = rtw_malloc(sizeof(*phl_param));
		if (phl_param == NULL) {
			RTW_ERR("scan: %s alloc param fail\n", __func__);
			goto free_cmd;
		}
		_rtw_memset(phl_param, 0, sizeof(*phl_param));

		/* transfer to rtw channel list to phl channel list */
		scan_channel_list_preparation(padapter, phl_param, rtw_param);

		phl_param->priv = padapter;
		if (rtw_param->is_p2p == true)
			phl_param->ops = &scan_ops_p2p_cb;
		else
			phl_param->ops = &scan_ops_cb;
		phl_param->wifi_role = padapter->phl_role;
		phl_param->acs = rtw_param->acs;

{
		u8 i = 0;
		phl_param->ssid_num = rtw_min(rtw_param->ssid_num, SCAN_SSID_AMOUNT);
		for (i = 0; i < phl_param->ssid_num; i ++) {
			_rtw_memcpy(phl_param->ssid[i].ssid, rtw_param->ssid[i].Ssid,
				rtw_param->ssid[i].SsidLength);
			phl_param->ssid[i].ssid_len = rtw_param->ssid[i].SsidLength;
		}
}

		res = rtw_phl_scan_request(dvobj->phl, phl_param, 0);
		padapter->cached_token = phl_param->token;
		rtw_mfree(phl_param->ch, phl_param->ch_sz);
		rtw_mfree(phl_param, sizeof(*phl_param));
	}
		break;

	default:
		res = rtw_phl_job_add_fptr(dvobj->phl,
				phl_run_core_cmd, padapter,
				pcmd, rtw_cmd_name(pcmd),
				(pcmd->no_io) ? PWR_NO_IO : PWR_BASIC_IO);

		if (res != RTW_PHL_STATUS_SUCCESS)
			goto free_cmd;

		return PHL_RES2RES(res);
		break;
	}

free_cmd:
	if (pcmd->cmdcode == CMD_SET_DRV_EXTRA) {
		struct drvextra_cmd_parm *extra_parm =
			(struct drvextra_cmd_parm *)pcmd->parmbuf;

		if (extra_parm->pbuf && extra_parm->size > 0)
			rtw_mfree(extra_parm->pbuf, extra_parm->size);
	}
	rtw_free_cmd_obj(pcmd);

	return PHL_RES2RES(res);
}

#endif
