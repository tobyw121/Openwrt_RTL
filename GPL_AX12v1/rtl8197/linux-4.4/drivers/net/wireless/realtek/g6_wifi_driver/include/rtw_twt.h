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

#ifndef __RTW_TWT_H_
#define __RTW_TWT_H_

#ifdef CONFIG_RTW_TWT

struct rtw_twt_sta_setup_cb {
	struct sta_info *psta;
	u8 nego_id;
	enum rtw_phl_setup_cmd twt_cmd;
};

#define TWT_NEGO_TYPE(twt_ele) twt_ele.twt_ctrl.nego_type
#define TWT_I_FLOW_ID(twt_ele) twt_ele.info.i_twt_para_set.req_type.twt_flow_id
#define TWT_I_SETUP_CMD(twt_ele) twt_ele.info.i_twt_para_set.req_type.twt_setup_cmd

u8 rtw_core_twt_on_action(_adapter *padapter,
						u8 *raddr, u8 *pframe_body, u32 frame_len);
u8 rtw_issue_twt_setup(_adapter *padapter, struct sta_info *psta,
						struct rtw_phl_twt_setup_info twt_setup_info,
						enum rtw_phl_setup_cmd twt_cmd);
u8 rtw_core_twt_on_setup_ap(_adapter *padapter, u8 *raddr,
						u8 *twt_element, u32 frame_len);
u8 rtw_issue_twt_teardown(_adapter *padapter, u8 *da,
						struct rtw_phl_twt_flow_field twt_flow_info);
u8 rtw_core_twt_on_teardown(_adapter *padapter, u8 *raddr,
						u8 *pframe, u32 frame_len);
u32 rtw_core_twt_teardown_sta_all(_adapter *padapter, struct sta_info *psta);
u32 rtw_core_twt_sta_active(_adapter *padapter, struct sta_info *psta);
void rtw_core_twt_inform_announce(_adapter *padapter, struct sta_info *psta);
#ifdef CONFIG_RTW_TWT_DBG
void rtw_core_twt_test_cmd_setup(_adapter *padapter);
void rtw_core_twt_test_cmd_teardown(_adapter *padapter);
void rtw_core_twt_test_cmd_pwrbit(_adapter *padapter, u16 macid, u8 pwrbit);
void rtw_core_twt_test_cmd_announce(_adapter *padapter, u16 macid);
void rtw_core_twt_inform_announce(_adapter *padapter, struct sta_info *psta);
void rtw_core_twt_test_cmd_testcase1(_adapter *padapter);
void rtw_core_twt_test_cmd_testcase2(_adapter *padapter);
void rtw_core_twt_test_cmd_testcase3(_adapter *padapter);
#endif


#endif /*CONFIG_RTW_TWT */
#endif /* __RTW_TWT_H_ */
