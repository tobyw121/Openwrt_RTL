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
#define _PHL_CUSTOMIZE_FEATURE_C_
#include "../phl_headers.h"

#ifdef CONFIG_PHL_CUSTOM_FEATURE_FB
#include "phl_custom_fb.h"

#define LLC_HDR_LENGTH                  6
#define SNAP_HDR_LENGTH                 2

enum phl_mdl_ret_code
_is_fb_mode_valid(void* custom_ctx,
                  struct _custom_facebook_ctx* fb_ctx,
                  struct phl_msg* msg, u32 size)
{
	struct phl_custom_cmd *cmd = (struct phl_custom_cmd *)(msg->inbuf);
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;
	if (!fb_ctx->init.enable || cmd->len < size || fb_ctx->init.wifi_role == NULL) {
		PHL_INFO(" %s, evt_id(%d) not accepted\n",
		         __FUNCTION__,
		         MSG_EVT_ID_FIELD(msg->msg_id));
		ret = MDL_RET_FAIL;
		return ret;
	}
	//check chanctx if wifi_role exists
	if (fb_ctx->init.wifi_role->chanctx == NULL) {
		PHL_INFO(" %s, wifi_role->chanctx is NULL\n", __FUNCTION__);
		fb_ctx->init.wifi_role = NULL;
		ret = MDL_RET_FAIL;
	} else {
		ret = MDL_RET_SUCCESS;
	}
	return ret;
}

enum phl_mdl_ret_code
_custom_fb_feature_enable(void* custom_ctx,
                          struct _custom_facebook_ctx* fb_ctx,
                          struct phl_msg* msg)
{
	struct phl_custom_cmd *cmd = (struct phl_custom_cmd *)(msg->inbuf);

	if (cmd->len < sizeof(u32))
		return MDL_RET_FAIL;

	fb_ctx->init.enable = *(u32*)(cmd->val);
	fb_ctx->init.test_mode = 0;
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&(fb_ctx->init.enable),
	                           sizeof(u32));

	return MDL_RET_SUCCESS;
}

enum phl_mdl_ret_code
_custom_fb_feature_query(void* custom_ctx,
                         struct _custom_facebook_ctx* fb_ctx,
                         struct phl_msg* msg)
{
	struct phl_custom_cmd *cmd = (struct phl_custom_cmd *)(msg->inbuf);
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	if (cmd->len < sizeof(u32))
		return ret;
	PHL_INFO("%s, fb query feature enable(%d)\n",
	         __FUNCTION__,
	         fb_ctx->init.enable);
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&(fb_ctx->init.enable),
	                           sizeof(u32));
	ret = MDL_RET_SUCCESS;
	return ret;
}

enum phl_mdl_ret_code
_custom_fb_testmode_param(void* custom_ctx,
                          struct _custom_facebook_ctx* fb_ctx,
                          struct phl_msg* msg)
{
	struct phl_custom_cmd *cmd = (struct phl_custom_cmd *)(msg->inbuf);
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	if (cmd->len < sizeof(u32))
		return ret;
	fb_ctx->init.test_mode = *(u32*)(cmd->val);
	PHL_INFO("%s, test mode(0x%x)\n", __FUNCTION__,
	         fb_ctx->init.test_mode);
	ret = MDL_RET_SUCCESS;
	return ret;
}

enum phl_mdl_ret_code
_custom_fb_set_wifi_role(void* custom_ctx,
                         struct _custom_facebook_ctx* fb_ctx,
                         struct phl_msg* msg)
{
	struct phl_custom_cmd *cmd = (struct phl_custom_cmd *)(msg->inbuf);
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	u32 size = MAC_ADDRESS_LENGTH;
	if (cmd->len < MAC_ADDRESS_LENGTH)
		return ret;
	if (!(fb_ctx->init.test_mode & FB_TEST_MODE_USE_STA_MAC))
		cmd->val[0] |= (BIT1 | BIT7);
	PHL_INFO("val - MAC-Addr:%02x-%02x-%02x-%02x-%02x-%02x\n",
	         *(cmd->val),*(cmd->val+1),*(cmd->val+2),
	         *(cmd->val+3), *(cmd->val+4),*(cmd->val+5));
	fb_ctx->init.wifi_role = phl_get_wrole_by_addr(phl, cmd->val);

	ret = _is_fb_mode_valid(custom_ctx, fb_ctx, msg, size);
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));

	return ret;
}

enum phl_mdl_ret_code
_custom_fb_ampdu_cfg(void* custom_ctx,
                     struct _custom_facebook_ctx* fb_ctx,
                     struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	void *d = phl_to_drvpriv(phl);
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	struct phl_custom_cmd *cmd = (struct phl_custom_cmd *)(msg->inbuf);
	struct rtw_phl_custom_ampdu_cfg custom_fb_ampdu_cfg = {0};
	u32 size = sizeof(struct rtw_phl_custom_ampdu_cfg);

	ret = _is_fb_mode_valid(custom_ctx, fb_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}
	phl_sta = rtw_phl_get_stainfo_self(phl, fb_ctx->init.wifi_role);
	_os_mem_cpy(d, &custom_fb_ampdu_cfg, cmd->val, size);
	hal_status = rtw_hal_custom_cfg_tx_ampdu(phl->hal,
	                                         fb_ctx->init.wifi_role,
	                                         &custom_fb_ampdu_cfg);
	phl_sta->asoc_cap.num_ampdu = (u8)custom_fb_ampdu_cfg.max_agg_num;

	PHL_INFO("%s, halsta(%d) ampdu dur(%d) num(%d)\n",
	         __FUNCTION__,
	         hal_status,
	         custom_fb_ampdu_cfg.max_agg_time_32us,
	         custom_fb_ampdu_cfg.max_agg_num);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
		ret = MDL_RET_FAIL;
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));
	return ret;
}

enum phl_mdl_ret_code
_custom_fb_ampdu_query(void* custom_ctx,
                       struct _custom_facebook_ctx* fb_ctx,
                       struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct phl_custom_cmd *cmd = (struct phl_custom_cmd *)(msg->inbuf);
	struct rtw_phl_custom_ampdu_cfg custom_fb_ampdu_cfg = {0};
	u32 size = sizeof(struct rtw_phl_custom_ampdu_cfg);

	/* 0xffffffff is querying failed for SDK*/
	custom_fb_ampdu_cfg.max_agg_num = 0xffffffff;
	custom_fb_ampdu_cfg.max_agg_time_32us = 0xffffffff;

	ret = _is_fb_mode_valid(custom_ctx, fb_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}
	hal_status = rtw_hal_get_ampdu_cfg(phl->hal,
	                                   fb_ctx->init.wifi_role,
	                                   &custom_fb_ampdu_cfg);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
		ret = MDL_RET_FAIL;
	PHL_INFO(" %s, ampdu dur(%d) time(%d)\n",
	         __FUNCTION__,
	         custom_fb_ampdu_cfg.max_agg_time_32us,
	         custom_fb_ampdu_cfg.max_agg_num);
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&custom_fb_ampdu_cfg,
	                           sizeof(struct rtw_phl_custom_ampdu_cfg));
	return ret;
}

enum phl_mdl_ret_code
_custom_fb_pdthr_cfg(void* custom_ctx,
                     struct _custom_facebook_ctx* fb_ctx,
                     struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct phl_custom_cmd *cmd = (struct phl_custom_cmd *)(msg->inbuf);
	int pd_thr = 0xff;
	u32 size = sizeof(int);

	ret = _is_fb_mode_valid(custom_ctx, fb_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}
	pd_thr = *(int*)(cmd->val);
	PHL_INFO("%s, pd_thr(%d)\n", __FUNCTION__, pd_thr);
	PHL_INFO("%s, bw(%d) band(%d)\n", __FUNCTION__,
	         fb_ctx->init.wifi_role->chanctx->chan_def.bw,
	         fb_ctx->init.wifi_role->hw_band);
	hal_status = rtw_hal_set_pkt_detect_thold(phl->hal, (u32)pd_thr);
	PHL_INFO("%s, hal_status(%d)\n", __FUNCTION__, hal_status);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
		ret = MDL_RET_FAIL;
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));
	return ret;
}

enum phl_mdl_ret_code
_custom_fb_pdthr_query(void* custom_ctx,
                       struct _custom_facebook_ctx* fb_ctx,
                       struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct phl_custom_cmd *cmd = (struct phl_custom_cmd *)(msg->inbuf);
	int pd_thr = 0xff;
	u32 size = sizeof(int);

	ret = _is_fb_mode_valid(custom_ctx, fb_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}
	/*confirm whether pd thr is enabling or not*/
	pd_thr = rtw_hal_query_pkt_detect_thold(phl->hal,
	                                        true,
	                                        fb_ctx->init.wifi_role->hw_band);
	if (pd_thr == 0) {
		PHL_INFO("%s, disable! pd_thr(%d)\n", __FUNCTION__, pd_thr);
	} else {
		pd_thr = rtw_hal_query_pkt_detect_thold(phl->hal,
		                                        false,
		                                        fb_ctx->init.wifi_role->hw_band);
		PHL_INFO("%s, pd_thr(%d)\n", __FUNCTION__, pd_thr);
	}
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&pd_thr,
	                           sizeof(int));
	return ret;
}

enum phl_mdl_ret_code
_custom_fb_pop_cfg(void* custom_ctx,
                   struct _custom_facebook_ctx* fb_ctx,
                   struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct phl_custom_cmd *cmd = (struct phl_custom_cmd *)(msg->inbuf);
	u32 size = sizeof(u32);
	u32 pop_enable = 0xff;

	ret = _is_fb_mode_valid(custom_ctx, fb_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}
	pop_enable = *(u32*)(cmd->val);
	PHL_INFO("%s, pop_enable(%d)\n", __FUNCTION__, pop_enable);
	if (pop_enable != 0xff) {
		hal_status = rtw_hal_set_pop_en(phl->hal,
		                                (bool)pop_enable,
		                                fb_ctx->init.wifi_role->hw_band);
		if (hal_status != RTW_HAL_STATUS_SUCCESS)
			ret = MDL_RET_FAIL;
	}
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));
	return ret;
}

enum phl_mdl_ret_code
_custom_fb_pop_query(void* custom_ctx,
                     struct _custom_facebook_ctx* fb_ctx,
                     struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct phl_custom_cmd *cmd = (struct phl_custom_cmd *)(msg->inbuf);
	u32 size = sizeof(u32);
	u32 pop_enable = 0xff;

	ret = _is_fb_mode_valid(custom_ctx, fb_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS){
		goto exit;
	}

	pop_enable = rtw_hal_query_pop_en(phl->hal, fb_ctx->init.wifi_role->hw_band);
	PHL_INFO("%s, pop_en(%d)\n", __FUNCTION__, pop_enable);
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&pop_enable,
	                            sizeof(u32));
	return ret;
}

enum phl_mdl_ret_code
phl_custom_hdl_fb_evt(void* dispr,
                      void* custom_ctx,
                      struct _custom_facebook_ctx* fb_ctx,
                      struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	u8 prephase = (IS_MSG_IN_PRE_PHASE(msg->msg_id)) ? (true) : (false);

	if (prephase == true)
		return MDL_RET_SUCCESS;

	switch (MSG_EVT_ID_FIELD(msg->msg_id)) {
		case MSG_EVT_CUSTOME_FEATURE_ENABLE:
			ret = _custom_fb_feature_enable(custom_ctx, fb_ctx, msg);
			break;
		case MSG_EVT_CUSTOME_FEATURE_QUERY:
			ret = _custom_fb_feature_query(custom_ctx, fb_ctx, msg);
			break;
		case MSG_EVT_CUSTOME_TESTMODE_PARAM:
			ret = _custom_fb_testmode_param(custom_ctx, fb_ctx, msg);
			break;
		case MSG_EVT_CUSTOME_SET_WIFI_ROLE:
			ret = _custom_fb_set_wifi_role(custom_ctx, fb_ctx, msg);
			break;
		case MSG_EVT_AMPDU_CFG:
			ret = _custom_fb_ampdu_cfg(custom_ctx, fb_ctx, msg);
			break;
		case MSG_EVT_AMPDU_QUERY:
			ret = _custom_fb_ampdu_query(custom_ctx, fb_ctx, msg);
			break;
		case MSG_EVT_PDTHR_CFG:
			ret = _custom_fb_pdthr_cfg(custom_ctx, fb_ctx, msg);
			break;
		case MSG_EVT_PDTHR_QUERY:
			ret = _custom_fb_pdthr_query(custom_ctx, fb_ctx, msg);
			break;
		case MSG_EVT_POP_CFG:
			ret = _custom_fb_pop_cfg(custom_ctx, fb_ctx, msg);
			break;
		case MSG_EVT_POP_QUERY:
			ret = _custom_fb_pop_query(custom_ctx, fb_ctx, msg);
			break;
		default:
			ret = MDL_RET_SUCCESS;
			break;
	}
	PHL_INFO("%s, evt(%d), ret(%d)\n", __FUNCTION__,
	         MSG_EVT_ID_FIELD(msg->msg_id),
	         ret);
	return ret;
}
enum phl_mdl_ret_code
phl_custom_hdl_fb_fail_evt(void* dispr,
                           void* custom_ctx,
                           struct _custom_facebook_ctx* fb_ctx,
                           struct phl_msg* msg)
{
	return MDL_RET_IGNORE;
}
#endif
