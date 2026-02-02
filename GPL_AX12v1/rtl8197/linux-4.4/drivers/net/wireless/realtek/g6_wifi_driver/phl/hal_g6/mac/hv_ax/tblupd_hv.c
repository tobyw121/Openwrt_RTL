/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
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

#include "tblupd_hv.h"
#include "../mac_ax/tblupd.h"
#include "../mac_ax/fwcmd.h"
#include "../mac_ax/trx_desc.h"
#include <windows.h>

u32 mac_plat_auto_test(struct mac_ax_adapter *adapter,
		       struct mac_ax_plat_auto_test *info,
		       enum mac_ax_plat_module test_module)
{
	u32 ret = 0;
	u32 i;
	u8 *buf;
	u32 *src, *dest;
	struct h2c_buf *h2cb;
	struct fwcmd_plat_auto_test *tbl;

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_DATA);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, sizeof(struct fwcmd_plat_auto_test));
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	tbl = (struct fwcmd_plat_auto_test *)buf;
	src = (u32 *)info;
	dest = (u32 *)(&tbl->dword0);
	for (i = 0; i < (sizeof(struct fwcmd_plat_auto_test) / 4); i++)
		*(dest++) = *(src++);

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C,
			      FWCMD_H2C_CAT_TEST,
			      FWCMD_H2C_CL_PLAT_AUTO_TEST,
			      test_module,
			      0,
			      0);
	if (ret)
		goto fail;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

	ret = PLTFM_TX(h2cb->data, h2cb->len);
	if (ret)
		goto fail;

	h2cb_free(adapter, h2cb);

	h2c_end_flow(adapter);

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}

u32 mac_long_run_test(struct mac_ax_adapter *adapter,
		      struct mac_ax_mac_test *info)
{
	u32 ret = 0;
	u32 i;
	u8 *buf;
	u32 *src, *dest;
	struct h2c_buf *h2cb;
	struct fwcmd_long_run *tbl;

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_DATA);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, sizeof(struct fwcmd_long_run));
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	tbl = (struct fwcmd_long_run *)buf;
	src = (u32 *)info;
	dest = (u32 *)(&tbl->dword0);
	for (i = 0; i < (sizeof(struct fwcmd_long_run) / 4); i++)
		*(dest++) = *(src++);

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C,
			      FWCMD_H2C_CAT_TEST,
			      FWCMD_H2C_CL_MAC_TEST,
			      FWCMD_H2C_FUNC_LONG_RUN,
			      0,
			      0);
	if (ret)
		goto fail;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

	ret = PLTFM_TX(h2cb->data, h2cb->len);
	if (ret)
		goto fail;

	h2cb_free(adapter, h2cb);

	h2c_end_flow(adapter);

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}

#define FLASH_PAGE_SIZE 1984
u32 mac_flash_burn_test(struct mac_ax_adapter *adapter, u8 *fw, u32 len)
{
	u8 *buf;
	u32 ret = 0, residue_len, pkt_len;
#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
#else
	struct h2c_buf *h2cb;
#endif
	residue_len = len;
	while (residue_len) {
		if (residue_len >= FLASH_PAGE_SIZE)
			pkt_len = FLASH_PAGE_SIZE;
		else
			pkt_len = residue_len;

		h2cb = h2cb_alloc(adapter, H2CB_CLASS_LONG_DATA);
		if (!h2cb)
			return MACNPTR; // Maybe set a timeout counter

		buf = h2cb_put(h2cb, pkt_len);
		if (!buf) {
			ret = MACNOBUF;
			goto fail;
		}
		PLTFM_MEMCPY(buf, fw, pkt_len);
		fw += pkt_len;
		ret = h2c_pkt_set_hdr_fwdl(adapter, h2cb,
					   FWCMD_TYPE_H2C,
					   FWCMD_H2C_CAT_TEST,
					   FWCMD_H2C_CL_PLAT_AUTO_TEST,
					   0x10,/*platform auto test*/
					   0,
					   0);
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
		if (ret) {
			PLTFM_MSG_ERR("[ERR]platform tx: %s\n");
			goto fail;
		}
		h2cb_free(adapter, h2cb);

		residue_len -= pkt_len;
		// a little delay for flash write
		Sleep(10000);
	}

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);
	adapter->fw_info.h2c_seq--;

	return ret;
}

