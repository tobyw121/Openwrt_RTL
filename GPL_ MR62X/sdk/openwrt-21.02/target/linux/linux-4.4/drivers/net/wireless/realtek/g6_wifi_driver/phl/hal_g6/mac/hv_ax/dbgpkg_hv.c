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
#include "dbgpkg_hv.h"
#include "../mac_ax/dbgpkg.h"
#include "../mac_reg.h"
#include "../mac_ax/mac_priv.h"

u32 hv_get_dle_dfi_info(struct mac_ax_adapter *adapter,
			struct hv_dbg_port *dbg)
{
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	struct mac_ax_dle_dfi_info *info;
	struct hv_dbg_port_info *dbg_info = dbg->info;
	struct dle_dfi_ctrl_t ctrl;
	u32 val32, ret, i;

	ret = p_ops->dle_dfi_sel(adapter, &info, &ctrl.target, dbg->dbg_sel);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] dle dfi sel %d %d\n", dbg->dbg_sel, ret);
		return ret;
	}

	if (dbg->dbg_sel <= MAC_AX_DLE_DFI_SEL_WDE_QUEMGN_QEMPTY) {
		ctrl.type = DLE_CTRL_TYPE_WDE;
	} else if (dbg->dbg_sel >= MAC_AX_DLE_DFI_SEL_PLE_BUFMGN_FREEPG &&
		 dbg->dbg_sel <= MAC_AX_DLE_DFI_SEL_PLE_QUEMGN_QEMPTY) {
		ctrl.type = DLE_CTRL_TYPE_PLE;
	} else {
		PLTFM_MSG_ERR("[ERR]unknown dle dfi sel-2 %d\n", dbg->dbg_sel);
		return MACFUNCINPUT;
	}

	dbg->read_addr = 0;
	dbg->sel_addr = 0;
	val32 = ((info->end - info->srt) / info->inc_num) + 2;
	if (val32 > dbg->len) {
		PLTFM_MSG_ERR("[ERR] dbg range %d > len %d\n", val32, dbg->len);
		goto end;
	}

	for (i = info->srt; i <= info->end; i += info->inc_num) {
		ctrl.addr = i;
		ret = p_ops->dle_dfi_ctrl(adapter, &ctrl);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]dle dfi ctrl %d\n", ret);
			return ret;
		}
		dbg_info->addr = ctrl.addr;
		dbg_info->val = ctrl.out_data;
		dbg_info++;
		PLTFM_MSG_TRACE("0x%08X\n", ctrl.out_data);
	}

end:
	dbg_info->addr = 0xFFFFFFFF;
	dbg_info->val = 0xFFFFFFFF;

	return MACSUCCESS;
}

u32 hv_get_dbg_port_info(struct mac_ax_adapter *adapter,
			 struct hv_dbg_port *dbg)
{
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
	struct mac_ax_dbg_port_info *info;
	u32 ret;
	u32 i;
	struct hv_dbg_port_info *dbg_info = dbg->info;
	u32 val32;
	u16 val16;
	u8 val8;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = p_ops->dbg_port_sel(adapter, &info, dbg->dbg_sel);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] dbg port sel %d %d\n", dbg->dbg_sel, ret);
		return ret;
	}

	dbg->read_addr = info->rd_addr;
	dbg->sel_addr = info->sel_addr;
	val32 = ((info->end - info->srt) / info->inc_num) + 2;
	if (val32 > dbg->len) {
		PLTFM_MSG_ERR("[ERR] dbg range %d > len %d\n", val32, dbg->len);
		goto end;
	}

	for (i = info->srt; i <= info->end; i += info->inc_num) {
		switch (info->sel_byte) {
		case MAC_AX_BYTE_SEL_1:
		default:
			val8 = SET_CLR_WOR2(MAC_REG_R8(info->sel_addr), i,
					    info->sel_sh,
					    info->sel_msk);
			MAC_REG_W8(info->sel_addr, val8);
			dbg_info->addr = val8;
			break;
		case MAC_AX_BYTE_SEL_2:
			val16 = SET_CLR_WOR2(MAC_REG_R16(info->sel_addr), i,
					     info->sel_sh,
					     info->sel_msk);
			MAC_REG_W16(info->sel_addr, val16);
			dbg_info->addr = val16;
			break;
		case MAC_AX_BYTE_SEL_4:
			val32 = SET_CLR_WOR2(MAC_REG_R32(info->sel_addr), i,
					     info->sel_sh,
					     info->sel_msk);
			MAC_REG_W32(info->sel_addr, val32);
			dbg_info->addr = val32;
			break;
		}

		PLTFM_DELAY_US(10);

		switch (info->rd_byte) {
		case MAC_AX_BYTE_SEL_1:
		default:
			val8 = GET_FIEL2(MAC_REG_R8(info->rd_addr),
					 info->rd_sh, info->rd_msk);
			dbg_info->val = val8;
			break;
		case MAC_AX_BYTE_SEL_2:
			val16 = GET_FIEL2(MAC_REG_R16(info->rd_addr),
					  info->rd_sh, info->rd_msk);
			dbg_info->val = val16;
			break;
		case MAC_AX_BYTE_SEL_4:
			val32 = GET_FIEL2(MAC_REG_R32(info->rd_addr),
					  info->rd_sh, info->rd_msk);
			PLTFM_MSG_TRACE("0x%08X\n", val32);
			dbg_info->val = val32;
			break;
		}
		dbg_info++;
	}

end:
	ret = p_ops->dbg_port_sel_rst(adapter, dbg->dbg_sel);
	if (ret != MACSUCCESS)
		PLTFM_MSG_ERR("[ERR] dbg sel reset %d\n", ret);

	dbg_info->addr = 0xFFFFFFFF;
	dbg_info->val = 0xFFFFFFFF;

	adapter->hw_info->dbg_port_cnt--;
	PLTFM_MUTEX_UNLOCK(&adapter->hw_info->dbg_port_lock);

	return ret;
}

u32 hv_set_ctrl_frame_cnt(struct mac_ax_adapter *adapter,
			  struct hv_ctrl_frame_cnt *ctrl)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = MACSUCCESS;
	u32 val32;
	u32 reg = ctrl->band ? R_AX_CTRL_FRAME_CNT_CTRL_C1 :
		R_AX_CTRL_FRAME_CNT_CTRL;
	u32 reg_cnt = ctrl->band ? R_AX_CTRL_FRAME_CNT_RPT_C1 :
		R_AX_CTRL_FRAME_CNT_RPT;
	u32 reg_sctrl = ctrl->band ? R_AX_CTRL_FRAME_CNT_SUBCTRL_C1 :
		R_AX_CTRL_FRAME_CNT_SUBCTRL;
	u8 val8;

	switch (ctrl->op) {
	case MAC_HV_CTRL_CNT_R:
		MAC_REG_W8(reg, ctrl->idx);
		ctrl->tval = MAC_REG_R16(reg_cnt);
		ctrl->rval = MAC_REG_R16(reg_cnt + 2);
		break;
	case MAC_HV_CTRL_CNT_W:
		MAC_REG_W8(reg, ctrl->idx);
		MAC_REG_W8(reg_sctrl, BIT(0));
		val32 = SET_WORD(ctrl->stype, B_AX_CTRL_SUBTYPE) |
			B_AX_WMAC_ALLCNT_EN | B_AX_WMAC_WDATA_EN |
			SET_WORD(ctrl->idx, B_AX_WMAC_CTRL_CNT_IDX);
		MAC_REG_W32(reg, val32);
		break;
	case MAC_HV_CTRL_CNT_RST:
		MAC_REG_W8(reg, ctrl->idx);
		MAC_REG_W8(reg_sctrl, BIT(0) | BIT(1));
		val8 = MAC_REG_R8(reg + 1);
		MAC_REG_W8(reg + 1, val8 | BIT(1));
		break;
	case MAC_HV_CTRL_CNT_RST_ALL:
		MAC_REG_W8(reg + 2, BIT(0));
		break;
	default:
		ret = MACNOITEM;
		break;
	}

	return ret;
}

u32 hv_set_rx_cnt(struct mac_ax_adapter *adapter,
		  struct hv_rx_cnt *cnt)
{
#define HV_RX_CNT_NUM 48
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret = MACSUCCESS;
	u32 reg = cnt->band ? R_AX_RX_DBG_CNT_SEL_C1 : R_AX_RX_DBG_CNT_SEL;
	u32 val;

	if (cnt->idx >= HV_RX_CNT_NUM) {
		PLTFM_MSG_ERR("Wrong RX counter index\n");
		ret = MACNOITEM;
		goto END;
	}

	switch (cnt->op) {
	case MAC_HV_RX_CNT_R:
		MAC_REG_W8(reg, cnt->idx);
		cnt->val = MAC_REG_R16(reg + 2);
		break;
	case MAC_HV_RX_CNT_W:
		if (cnt->idx >= 35 && cnt->idx <= 38) {
			MAC_REG_W8(reg, cnt->idx);
			val = SET_WORD(cnt->type, B_AX_UD_TYPE) |
				SET_WORD(cnt->subtype, B_AX_UD_SUB_TYPE) |
				SET_WORD(cnt->bssid, B_AX_UD_SELECT_BSSID) |
				SET_WORD(cnt->rate, B_AX_UD_RATE) |
				SET_WORD(cnt->gi_ltf, B_AX_UD_GI_TYPE) |
				SET_WORD(cnt->ru, B_AX_UD_RUTONE) |
				(cnt->msk & MAC_HV_RX_CNT_MSK_FC ?
				 B_AX_UD_MSK_FC : 0) |
				(cnt->msk & MAC_HV_RX_CNT_MSK_BSSID ?
				 B_AX_UD_MSK_BSSID : 0) |
				(cnt->msk & MAC_HV_RX_CNT_MSK_RATE ?
				 B_AX_UD_MSK_RATE : 0) |
				(cnt->msk & MAC_HV_RX_CNT_MSK_RU ?
				 B_AX_UD_MSK_RUTONE : 0) |
				B_AX_UD_W1S;
			MAC_REG_W32(reg + 4, val);
		}
		break;
	case MAC_HV_RX_CNT_RST:
		MAC_REG_W8(reg, cnt->idx);
		MAC_REG_W8(reg + 1, BIT(0));
		break;
	default:
		ret = MACNOITEM;
		break;
	}
END:
	return ret;
}

u32 c2h_sys_fw_autotest(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		     struct rtw_c2h_info *info)
{
	return MACSUCCESS;
}

u32 hv_get_mac_err_isr(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret;

	if (MAC_REG_R32(R_AX_DMAC_ERR_ISR) != 0)
		return MACHWDMACERR;
	ret = check_mac_en(adapter, 0, MAC_AX_CMAC_SEL);
	if (ret == MACSUCCESS) {
		if (MAC_REG_R32(R_AX_CMAC_ERR_ISR) != 0)
			return MACHWCMAC0ERR;
	}
	ret = check_mac_en(adapter, 1, MAC_AX_CMAC_SEL);
	if (ret == MACSUCCESS) {
		if (MAC_REG_R32(R_AX_CMAC_ERR_ISR_C1) != 0)
			return MACHWCMAC1ERR;
	}

	return MACSUCCESS;
}

u32 hv_c2h_log_test(struct mac_ax_adapter *adapter, u32 len)
{
	u32 ret = 0;
	u8 *buf;
	#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
	#else
	struct h2c_buf *h2cb;
	#endif
	struct fwcmd_log_test *log;

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_CMD);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, sizeof(*log));
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}
	PLTFM_MEMSET(buf, 0, sizeof(*log));

	log = (struct fwcmd_log_test *)buf;

	log->dword0 = cpu_to_le32(SET_WORD(len,
					   FWCMD_H2C_LOG_TEST_LEN));

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C,
			      FWCMD_H2C_CAT_TEST,
			      FWCMD_H2C_CL_CMD_PATH,
			      FWCMD_H2C_FUNC_LOG_TEST,
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
	if (ret)
		goto fail;

	h2cb_free(adapter, h2cb);

	h2c_end_flow(adapter);

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}