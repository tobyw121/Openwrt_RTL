/******************************************************************************
 *
 * Copyright(c) 2020 Realtek Corporation. All rights reserved.
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
 ******************************************************************************/

#include "fwcmd_hv.h"
#include "../mac_ax/fwcmd.h"
#include "../mac_ax/trx_desc.h"


u32 hv_ptn_h2c_common(struct mac_ax_adapter* adapter,
	struct rtw_g6_h2c_hdr* hdr, u32* pvalue)
{
#define HV_PTN_H2C_MAX_LEN 60
	u32 ret = 0;
	u8* buf;
#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt* h2cb;
#else
	struct h2c_buf* h2cb;
#endif

	h2cb = h2cb_alloc(adapter, (enum h2c_buf_class)hdr->type);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, hdr->content_len);
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	PLTFM_MEMCPY(buf, pvalue, hdr->content_len);

	ret = h2c_pkt_set_hdr(adapter, h2cb,
		FWCMD_TYPE_H2C,
		FWCMD_H2C_CAT_TEST,
		FWCMD_H2C_CL_FW_AUTO_TEST,
		hdr->h2c_func,
		hdr->rec_ack,
		hdr->done_ack);
	if (ret)
		goto fail;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
#endif
	if (ret)
		goto fail;

	h2cb_free(adapter, h2cb);

	h2c_end_flow(adapter);

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}

u32 c2h_lps_onoff_rpt(struct mac_ax_adapter *adapter,
	u16 len, u8 *buf)
{
	struct fwcmd_lps_onoff_test *rpt;
	u32 type;
	u32 ret;

	rpt = (struct fwcmd_lps_onoff_test *)buf;
	type = le32_to_cpu(rpt->dword0);
	ret = le32_to_cpu(rpt->dword1);
	PLTFM_MSG_TRACE("LPS test type (%d), result (%d).\n", type, ret);

    if (TRUE == ret) {
        return MACSUCCESS;
    } else {
        return MACFWTESTFAIL;
    }
}

