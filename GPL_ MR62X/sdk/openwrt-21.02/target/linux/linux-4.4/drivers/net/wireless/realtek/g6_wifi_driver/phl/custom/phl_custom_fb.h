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
#ifndef _PHL_CUSTOM_FB_H_
#define _PHL_CUSTOM_FB_H_

#ifdef CONFIG_PHL_CUSTOM_FEATURE_FB
#define TX_PKT_CYCLE (10)

#define	FC_QOS_BIT			BIT7
#define GET_80211_HDR_TO_DS(_hdr)	LE_BITS_TO_2BYTE(_hdr, 8, 1)
#define GET_80211_HDR_FROM_DS(_hdr)	LE_BITS_TO_2BYTE(_hdr, 9, 1)
#define GET_80211_HDR_WEP(_hdr)		LE_BITS_TO_2BYTE(_hdr, 14, 1)
#define GET_80211_HDR_ORDER(_hdr)	LE_BITS_TO_2BYTE(_hdr, 15, 1)
#define IS_DATA_FRAME(pdu)		( ((EF1Byte(pdu[0]) & 0x0C)==0x08) ? TRUE : FALSE )
#define	IS_QOS_DATA_FRAME(pdu)		(IS_DATA_FRAME(pdu) && (EF1Byte(pdu[0]) & FC_QOS_BIT) )

enum _fackbook_test_mode {
	FB_TEST_MODE_FAKE_TX_CYCLE = BIT0,
	FB_TEST_MODE_USE_STA_MAC = BIT1,
};

struct _facebook_init_param {
	u32 enable;
	u32 test_mode;
	struct rtw_wifi_role_t *wifi_role;
};

struct _custom_facebook_ctx {
	struct _facebook_init_param init;
};

enum phl_mdl_ret_code
phl_custom_hdl_fb_evt(void* dispr,
                      void* custom_ctx,
                      struct _custom_facebook_ctx* fb_ctx,
                      struct phl_msg* msg);
enum phl_mdl_ret_code
phl_custom_hdl_fb_fail_evt(void* dispr,
                           void* custom_ctx,
                           struct _custom_facebook_ctx* fb_ctx,
                           struct phl_msg* msg);
#endif

#endif  /*_PHL_CUSTOMIZE_FEATURE_H_*/

