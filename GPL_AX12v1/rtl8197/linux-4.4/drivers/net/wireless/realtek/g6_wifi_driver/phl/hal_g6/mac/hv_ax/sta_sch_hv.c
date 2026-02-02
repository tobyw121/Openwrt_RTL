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
#include "../../hal_general_def.h"
#include "../mac_reg.h"
#include "sta_sch_hv.h"
#include "../mac_ax/trxcfg.h"
#include "../mac_ax/role.h"
#include "../mac_ax/hw.h"

static u32 sram0_ctrl_r(struct mac_ax_adapter *adapter, u32 cmd, u32 *r_val);
static u32 sram0_ctrl_w(struct mac_ax_adapter *adapter, u32 cmd, u32 mask,
			u32 w_val);
static u32 sram1_ctrl_chk(struct mac_ax_adapter *adapter, u32 cmd);
static u32 sram1_ctrl_val(struct mac_ax_adapter *adapter, u32 cmd, u32 *r_val);
static u32 get_link_info(struct mac_ax_adapter *adapter, u32 cmd, u32 *r_val);
static u32 add_to_link(struct mac_ax_adapter *adapter,
		       struct hv_ax_ss_link_info *link);
static u32 del_from_link(struct mac_ax_adapter *adapter,
			 struct hv_ax_ss_link_info *link);
static u32 clean_sta_link(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_link_info *link);
static u32 poll_own_chk(struct mac_ax_adapter *adapter, u32 addr);
static u32 poll_own_val(struct mac_ax_adapter *adapter, u32 addr, u32 *r_val);
static u32 sta_len_ctrl(struct mac_ax_adapter *adapter,
			struct hv_ax_sta_len_ctrl *ctrl);
static u32 get_sta_len_indir(struct mac_ax_adapter *adapter,
			     struct hv_ax_sta_len *len);
static u32 set_sta_len_indir(struct mac_ax_adapter *adapter,
			     struct hv_ax_sta_len *len);
#ifdef NEVER
static u32 get_sta_len(struct mac_ax_adapter *adapter,
		       struct hv_ax_sta_len *len);
static u32 set_sta_len(struct mac_ax_adapter *adapter,
		       struct hv_ax_sta_len *len);
#endif /* Never */
static u32 get_sta_quota(struct mac_ax_adapter *adapter,
			 struct hv_ax_sta_quota *quota);
static u32 set_sta_quota(struct mac_ax_adapter *adapter,
			 struct hv_ax_sta_quota *quota);
static u32 get_sta_quota_setting(struct mac_ax_adapter *adapter,
				 struct hv_ax_sta_quota *quota);
static u32 set_sta_quota_setting(struct mac_ax_adapter *adapter,
				 struct hv_ax_sta_quota *quota);
static u32 get_sta_muru(struct mac_ax_adapter *adapter,
			struct hv_ax_sta_muru_ctrl *muru);
static u32 set_sta_muru(struct mac_ax_adapter *adapter,
			struct hv_ax_sta_muru_ctrl *muru);
static u32 get_sta_dl_rugrp(struct mac_ax_adapter *adapter,
			    struct hv_ax_sta_dl_rugrp_ctrl *rugrp);
static u32 set_sta_dl_rugrp(struct mac_ax_adapter *adapter,
			    struct hv_ax_sta_dl_rugrp_ctrl *rugrp);
static u32 wmm_vld_chk(struct mac_ax_adapter *adapter, u8 *vld, u8 wmm);
static u32 ul_vld_chk(struct mac_ax_adapter *adapter, u8 *vld);
static u32 mu_vld_chk(struct mac_ax_adapter *adapter, u8 *vld);
static u32 ru_vld_chk(struct mac_ax_adapter *adapter, u8 *vld);
static u32 get_sta_link(struct mac_ax_adapter *adapter,
			struct hv_ax_ss_link_info *link);
static void get_dl_su_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info);
static void set_dl_su_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info);
static void get_dl_mu_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info);
static void set_dl_mu_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info);
static void get_dl_ru_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info);
static void set_dl_ru_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info);
static void get_ul_rpt(struct mac_ax_adapter *adapter,
		       struct hv_ax_ss_ul_rpt_info *info);
static void set_ul_rpt(struct mac_ax_adapter *adapter,
		       struct hv_ax_ss_ul_rpt_info *info);
static u32 get_sta_bmp_size(struct mac_ax_adapter *adapter, u8 *bmp_size);
static u32 get_sta_bmp(struct mac_ax_adapter *adapter,
		       struct hv_ax_sta_bmp_ctrl *ctrl);
static u32 set_sta_bmp(struct mac_ax_adapter *adapter,
		       struct hv_ax_sta_bmp_ctrl *ctrl);
#if MAC_AX_8852A_SUPPORT || MAC_AX_8852B_SUPPORT || MAC_AX_8851B_SUPPORT
static u32 get_sta_dl_mutbl(struct mac_ax_adapter *adapter,
			    struct hv_ax_sta_dl_mutbl_ctrl *mutbl);
static u32 set_sta_dl_mutbl(struct mac_ax_adapter *adapter,
			    struct hv_ax_sta_dl_mutbl_ctrl *mutbl);
#endif
static void get_ss_quota_mode(struct mac_ax_adapter *adapter,
			      struct hv_ax_ss_quota_mode_ctrl *ctrl);
static u32 set_ss_quota_mode(struct mac_ax_adapter *adapter,
			     struct hv_ax_ss_quota_mode_ctrl *ctrl);
static void set_ss_wmm_tbl(struct mac_ax_adapter *adapter,
			   struct mac_ax_ss_wmm_tbl_ctrl *ctrl);
static u32 switch_wmm_link(struct mac_ax_adapter *adapter,
			   enum mac_ax_ss_wmm_tbl src_link,
			   enum mac_ax_ss_wmm_tbl dst_link,
			   enum hv_ax_ss_wmm src_wmm);
static u32 switch_wmm_macid(struct mac_ax_adapter *adapter,
			    struct hv_ax_ss_link_info *link,
			    enum mac_ax_ss_wmm_tbl src_link,
			    enum mac_ax_ss_wmm_tbl dst_link);

static u32 sram0_ctrl_r(struct mac_ax_adapter *adapter, u32 cmd, u32 *r_val)
{
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	MAC_REG_W32(R_AX_SS_SRAM_CTRL_0, cmd);
	ret = poll_own_chk(adapter, R_AX_SS_SRAM_CTRL_0);

	*r_val = MAC_REG_R32(R_AX_SS_SRAM_DATA);

	return ret;
}

static u32 sram0_ctrl_w(struct mac_ax_adapter *adapter, u32 cmd, u32 mask,
			u32 w_val)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	MAC_REG_W32(R_AX_SS_SRAM_W_EN, mask);
	MAC_REG_W32(R_AX_SS_SRAM_DATA, w_val);

	MAC_REG_W32(R_AX_SS_SRAM_CTRL_0, cmd);

	return poll_own_chk(adapter, R_AX_SS_SRAM_CTRL_0);
}

static u32 sram1_ctrl_chk(struct mac_ax_adapter *adapter, u32 cmd)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	MAC_REG_W32(R_AX_SS_SRAM_CTRL_1, cmd);

	return poll_own_chk(adapter, R_AX_SS_SRAM_CTRL_1);
}

static u32 sram1_ctrl_val(struct mac_ax_adapter *adapter, u32 cmd, u32 *r_val)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	MAC_REG_W32(R_AX_SS_SRAM_CTRL_1, cmd);

	return poll_own_val(adapter, R_AX_SS_SRAM_CTRL_1, r_val);
}

static u32 get_link_info(struct mac_ax_adapter *adapter, u32 cmd, u32 *r_val)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	MAC_REG_W32(R_AX_SS_LINK_INFO, cmd);

	return poll_own_val(adapter, R_AX_SS_LINK_INFO, r_val);
}

static u32 add_to_link(struct mac_ax_adapter *adapter,
		       struct hv_ax_ss_link_info *link)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	MAC_REG_W32(R_AX_SS_LINK_ADD, B_AX_SS_OWN |
		    (link->ul ? B_AX_SS_UL : 0) |
		    SET_WORD(link->wmm, B_AX_SS_WMM) |
		    SET_WORD(link->ac, B_AX_SS_AC) |
		    SET_WORD(link->macid2, B_AX_SS_MACID_2) |
		    SET_WORD(link->macid1, B_AX_SS_MACID_1) |
		    SET_WORD(link->macid0, B_AX_SS_MACID_0));
	return poll_own_chk(adapter, R_AX_SS_LINK_ADD);
}

static u32 del_from_link(struct mac_ax_adapter *adapter,
			 struct hv_ax_ss_link_info *link)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	MAC_REG_W32(R_AX_SS_LINK_DEL, B_AX_SS_OWN |
		    (link->ul ? B_AX_SS_UL : 0) |
		    SET_WORD(link->wmm, B_AX_SS_WMM) |
		    SET_WORD(link->ac, B_AX_SS_AC) |
		    SET_WORD(link->macid2, B_AX_SS_MACID_2) |
		    SET_WORD(link->macid1, B_AX_SS_MACID_1) |
		    SET_WORD(link->macid0, B_AX_SS_MACID_0));
	return poll_own_chk(adapter, R_AX_SS_LINK_DEL);
}

static u32 clean_sta_link(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_link_info *link)
{
	u32 val32;
	u8 wmm, ac;
	u32 cnt = adapter->hw_info->macid_num + 1;
	u32 cmd;
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct hv_ax_ops *hv_ops = adapter->hv_ops;

	if (link->ul) {
		wmm = 0;
		ac = 0;
	} else {
		wmm = link->wmm;
		ac = link->ac;
	}

	do {
		cmd = B_AX_SS_OWN | (link->ul ? B_AX_SS_UL : 0) |
		      SET_WORD(wmm, B_AX_SS_WMM) | SET_WORD(ac, B_AX_SS_AC);
		ret = get_link_info(adapter, cmd, &val32);
		if (ret != MACSUCCESS)
			return ret;

		link->link_head = GET_FIELD(val32, B_AX_SS_LINK_HEAD);
		link->link_tail = GET_FIELD(val32, B_AX_SS_LINK_TAIL);
		link->link_len = GET_FIELD(val32, B_AX_SS_LINK_LEN);
		link->macid0 = link->link_head;

		if (link->macid0 != 0xFF) {
			ret = hv_ops->sta_link_cfg(adapter, link,
						   HV_AX_SS_LINK_CFG_DEL);
			if (ret != MACSUCCESS)
				return ret;
			cnt--;
		} else {
			break;
		}
	} while (link->link_len || cnt);

	if (!cnt) {
		PLTFM_MSG_ERR("link error!!");
		return MACSSLINK;
	}

	return MACSUCCESS;
}

static u32 get_sta_quota(struct mac_ax_adapter *adapter,
			 struct hv_ax_sta_quota *quota)
{
	u32 sel, val32;
	u32 cmd;
	u32 ret;
	u8 macid;
	u32 dw[4];
	u8 vld;

	macid = quota->macid;

	for (sel = 0; sel < 4; sel++) {
		cmd = B_AX_SS_OWN |
		      SET_WORD(CTRL1_R_QUOTA, B_AX_SS_CMD_SEL) |
		      SET_WORD(sel, B_AX_SS_AC) | macid;
		ret = sram1_ctrl_val(adapter, cmd, &dw[sel]);
		if (ret != MACSUCCESS)
			return ret;
	}
	quota->be_quota = (dw[0] >> MACID_SH) & QUOTA_MSK;
	quota->bk_quota = (dw[1] >> MACID_SH) & QUOTA_MSK;
	quota->vi_quota = (dw[2] >> MACID_SH) & QUOTA_MSK;
	quota->vo_quota = (dw[3] >> MACID_SH) & QUOTA_MSK;

	ret = ul_vld_chk(adapter, &vld);
	if (ret != MACSUCCESS)
		return ret;

	if (vld == 0) {
		quota->ul_quota = 0;
	} else {
		cmd = B_AX_SS_OWN | SET_WORD(CTRL1_R_QUOTA, B_AX_SS_CMD_SEL) |
		      BIT(23) | macid;
		ret = sram1_ctrl_val(adapter, cmd, &val32);
		if (ret != MACSUCCESS)
			return ret;

		quota->ul_quota = (val32 >> MACID_SH) & QUOTA_MSK;
	}

	return MACSUCCESS;
}

static u32 set_sta_quota(struct mac_ax_adapter *adapter,
			 struct hv_ax_sta_quota *quota)
{
	u32 sel;
	u32 dw[4];
	u32 ret;
	u8 macid;
	u32 cmd;
	u8 vld;

	macid = quota->macid;

	dw[0] = quota->be_quota;
	dw[1] = quota->bk_quota;
	dw[2] = quota->vi_quota;
	dw[3] = quota->vo_quota;

	for (sel = 0; sel < 4; sel++) {
		cmd = B_AX_SS_OWN | SET_WORD(CTRL1_W_QUOTA, B_AX_SS_CMD_SEL) |
		      SET_WORD(sel, B_AX_SS_AC) |
		      SET_WORD(dw[sel], B_AX_SS_VALUE) | macid;
		ret = sram1_ctrl_chk(adapter, cmd);
		if (ret != MACSUCCESS)
			return ret;
	}

	ret = ul_vld_chk(adapter, &vld);
	if (ret != MACSUCCESS)
		return ret;
	if (vld == 0)
		return MACSUCCESS;

	cmd = B_AX_SS_OWN | SET_WORD(CTRL1_W_QUOTA, B_AX_SS_CMD_SEL) | BIT(23) |
	      SET_WORD(quota->ul_quota, B_AX_SS_VALUE) | macid;
	ret = sram1_ctrl_chk(adapter, cmd);
	if (ret != MACSUCCESS)
		return ret;

	return MACSUCCESS;
}

static u32 get_sta_quota_setting(struct mac_ax_adapter *adapter,
				 struct hv_ax_sta_quota *quota)
{
	u32 sel, val32;
	u32 ret;
	u8 macid;
	u32 cmd;
	u32 dw[4];
	u8 vld;

	macid = quota->macid;

	for (sel = 0; sel < 4; sel++) {
		cmd = B_AX_SS_OWN |
		      SET_WORD(CTRL1_R_QUOTA_SETTING, B_AX_SS_CMD_SEL) |
		      SET_WORD(sel, B_AX_SS_AC) | macid;
		ret = sram1_ctrl_val(adapter, cmd, &dw[sel]);
		if (ret != MACSUCCESS)
			return ret;
	}
	quota->be_quota = (dw[0] >> MACID_SH) & QUOTA_SETTING_MSK;
	quota->bk_quota = (dw[1] >> MACID_SH) & QUOTA_SETTING_MSK;
	quota->vi_quota = (dw[2] >> MACID_SH) & QUOTA_SETTING_MSK;
	quota->vo_quota = (dw[3] >> MACID_SH) & QUOTA_SETTING_MSK;

	ret = ul_vld_chk(adapter, &vld);
	if (ret != MACSUCCESS)
		return ret;
	if (vld == 0) {
		quota->ul_quota = 0;
	} else {
		cmd = B_AX_SS_OWN |
		      SET_WORD(CTRL1_R_QUOTA_SETTING, B_AX_SS_CMD_SEL) |
		      BIT(23) | macid;
		ret = sram1_ctrl_val(adapter, cmd, &val32);
		if (ret != MACSUCCESS)
			return ret;

		quota->ul_quota = (val32 >> MACID_SH) & QUOTA_SETTING_MSK;
	}

	return MACSUCCESS;
}

static u32 set_sta_quota_setting(struct mac_ax_adapter *adapter,
				 struct hv_ax_sta_quota *quota)
{
	u32 sel;
	u32 dw[4];
	u32 ret;
	u8 macid;
	u32 cmd;
	u8 vld;

	macid = quota->macid;

	dw[0] = quota->be_quota;
	dw[1] = quota->bk_quota;
	dw[2] = quota->vi_quota;
	dw[3] = quota->vo_quota;

	for (sel = 0; sel < 4; sel++) {
		cmd = B_AX_SS_OWN |
		      SET_WORD(CTRL1_W_QUOTA_SETTING, B_AX_SS_CMD_SEL) |
		      SET_WORD(sel, B_AX_SS_AC) |
		      SET_WORD(dw[sel], B_AX_SS_VALUE) | macid;
		ret = sram1_ctrl_chk(adapter, cmd);
		if (ret != MACSUCCESS)
			return ret;
	}

	ret = ul_vld_chk(adapter, &vld);
	if (ret != MACSUCCESS)
		return ret;
	if (vld == 0)
		return MACSUCCESS;

	cmd = B_AX_SS_OWN |
	      SET_WORD(CTRL1_W_QUOTA_SETTING, B_AX_SS_CMD_SEL) | BIT(23) |
	      SET_WORD(quota->ul_quota, B_AX_SS_VALUE) | macid;
	ret = sram1_ctrl_chk(adapter, cmd);
	if (ret != MACSUCCESS)
		return ret;

	return MACSUCCESS;
}

static u32 get_sta_len_indir(struct mac_ax_adapter *adapter,
			     struct hv_ax_sta_len *len)
{
	u32 sel, val32;
	u32 ret;
	u8 macid;
	u32 cmd;
	u32 dw[4];
	u8 vld;

	macid = len->macid;

	for (sel = 0; sel < 4; sel++) {
		cmd = B_AX_SS_OWN | SET_WORD(CTRL1_R_TX_LEN, B_AX_SS_CMD_SEL) |
		      SET_WORD(sel, B_AX_SS_AC) | macid;
		ret = sram1_ctrl_val(adapter, cmd, &dw[sel]);
		if (ret != MACSUCCESS)
			return ret;
	}
	len->be_len = (dw[0] >> MACID_SH) & TX_LEN_MSK;
	len->bk_len = (dw[1] >> MACID_SH) & TX_LEN_MSK;
	len->vi_len = (dw[2] >> MACID_SH) & TX_LEN_MSK;
	len->vo_len = (dw[3] >> MACID_SH) & TX_LEN_MSK;

	ret = ul_vld_chk(adapter, &vld);
	if (ret != MACSUCCESS)
		return ret;
	if (vld == 0) {
		len->bsr_len = 0;
		len->bsr_ac_type = 0;
	} else {
		cmd = B_AX_SS_OWN | SET_WORD(CTRL1_R_BSR_LEN, B_AX_SS_CMD_SEL) |
		      macid;
		ret = sram1_ctrl_val(adapter, cmd, &val32);
		if (ret != MACSUCCESS)
			return ret;

		len->bsr_len = (val32 >> MACID_SH) & BSR_LEN_MSK;
		len->bsr_ac_type = GET_FIELD(val32, B_AX_SS_AC);
	}

	return MACSUCCESS;
}

static u32 set_sta_len_indir(struct mac_ax_adapter *adapter,
			     struct hv_ax_sta_len *len)
{
	u32 ret;
	u8 macid;
	u32 cmd;
	u8 vld;
	struct hv_ax_sta_len get_len;
	struct hv_ax_sta_len_ctrl ctrl;

	macid = len->macid;
	ctrl.macid = macid;

	get_len.macid = macid;
	ret = get_sta_len_indir(adapter, &get_len);
	if (ret != MACSUCCESS)
		return ret;

	ctrl.ac = MAC_AX_CMAC_AC_SEL_VO;
	if (get_len.vo_len > len->vo_len) {
		ctrl.len = get_len.vo_len - len->vo_len;
		ctrl.cmd = HV_AX_STA_LEN_DECR;
	} else {
		ctrl.len = len->vo_len - get_len.vo_len;
		ctrl.cmd = HV_AX_STA_LEN_INCR;
	}
	sta_len_ctrl(adapter, &ctrl);

	ctrl.ac = MAC_AX_CMAC_AC_SEL_VI;
	if (get_len.vi_len > len->vi_len) {
		ctrl.len = get_len.vi_len - len->vi_len;
		ctrl.cmd = HV_AX_STA_LEN_DECR;
	} else {
		ctrl.len = len->vi_len - get_len.vi_len;
		ctrl.cmd = HV_AX_STA_LEN_INCR;
	}
	sta_len_ctrl(adapter, &ctrl);

	ctrl.ac = MAC_AX_CMAC_AC_SEL_BE;
	if (get_len.be_len > len->be_len) {
		ctrl.len = get_len.be_len - len->be_len;
		ctrl.cmd = HV_AX_STA_LEN_DECR;
	} else {
		ctrl.len = len->be_len - get_len.be_len;
		ctrl.cmd = HV_AX_STA_LEN_INCR;
	}
	sta_len_ctrl(adapter, &ctrl);

	ctrl.ac = MAC_AX_CMAC_AC_SEL_BK;
	if (get_len.bk_len > len->bk_len) {
		ctrl.len = get_len.bk_len - len->bk_len;
		ctrl.cmd = HV_AX_STA_LEN_DECR;
	} else {
		ctrl.len = len->bk_len - get_len.bk_len;
		ctrl.cmd = HV_AX_STA_LEN_INCR;
	}
	sta_len_ctrl(adapter, &ctrl);

	ret = ul_vld_chk(adapter, &vld);
	if (ret != MACSUCCESS)
		return ret;
	if (vld == 0)
		return MACSUCCESS;

	cmd = B_AX_SS_OWN | SET_WORD(CTRL1_W_BSR_LEN, B_AX_SS_CMD_SEL) |
	      SET_WORD(len->bsr_ac_type, B_AX_SS_AC) |
	      SET_WORD(len->bsr_len, B_AX_SS_VALUE) | macid;
	ret = sram1_ctrl_chk(adapter, cmd);
	if (ret != MACSUCCESS)
		return ret;

	return MACSUCCESS;
}

#ifdef NEVER
static u32 get_sta_len(struct mac_ax_adapter *adapter,
		       struct hv_ax_sta_len *len)
{
	u32 sel;
	u32 dw[4];
	u32 ret;
	u8 macid;
	u32 cmd;

	macid = len->macid;

	for (sel = 0; sel < 4; sel++) {
		cmd = B_AX_SS_OWN | SET_WORD(SRAM_LEN, B_AX_SS_CMD) |
		      SET_WORD(sel, B_AX_SS_OFFSET) | macid;
		ret = sram0_ctrl_r(adapter, cmd, &dw[sel]);
		if (ret != MACSUCCESS)
			return ret;
	}
	len->be_len = dw[0] & 0x1FFFFF;
	len->bk_len = ((dw[0] & 0xFFE00000) >> 21) | ((dw[1] & 0x3FF) << 11);
	len->vi_len = (dw[1] & 0x7FFFFC00) >> 10;
	len->vo_len = ((dw[1] & 0x80000000) >> 31) | ((dw[2] & 0xFFFFF) << 1);
	len->bsr_len = ((dw[2] & 0xFFF00000) >> 20) | ((dw[3] & 0x7) << 12);
	len->bsr_ac_type = (dw[3] & 0x18) >> 3;

	return MACSUCCESS;
}

static u32 set_sta_len(struct mac_ax_adapter *adapter,
		       struct hv_ax_sta_len *len)
{
	u32 sel;
	u32 dw[4];
	u32 ret;
	u8 macid;
	u32 cmd;

	macid = len->macid;

	dw[0] = len->be_len | ((len->bk_len & 0x7FF) << 21);
	dw[1] = ((len->bk_len & 0x1FF800) >> 11) | (len->vi_len << 10) |
		((len->vo_len & 0x1) << 31);
	dw[2] = ((len->vo_len & 0x1FFFFE) >> 1) |
		((len->bsr_len & 0xFFF) << 20);
	dw[3] = ((len->bsr_len & 0x7000) >> 12) | (len->bsr_ac_type << 3);

	for (sel = 0; sel < 4; sel++) {
		cmd = B_AX_SS_OWN | B_AX_SS_RW |
		      SET_WORD(SRAM_LEN, B_AX_SS_CMD) |
		      SET_WORD(sel, B_AX_SS_OFFSET) | macid;
		ret = sram0_ctrl_w(adapter, cmd, 0xFFFFFFFF, dw[sel]);
		if (ret != MACSUCCESS)
			return ret;
	}

	return MACSUCCESS;
}
#endif /* NEVER */

static u32 poll_own_chk(struct mac_ax_adapter *adapter, u32 addr)
{
	u32 cnt = 100;
	u32 ck;
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (addr == R_AX_SS_SRAM_CTRL_1 || addr == R_AX_SS_LINK_INFO ||
	    addr == R_AX_SS_SRAM_CTRL_0 || addr == R_AX_SS_LINK_ADD ||
	    addr == R_AX_SS_LINK_DEL || addr == R_AX_SS_LINK_SEARCH)
		ck = B_AX_SS_OWN;
	else
		ck = 0;

	while (--cnt) {
		val32 = MAC_REG_R32(addr);
		if ((val32 & ck) == 0)
			break;
		PLTFM_DELAY_US(1);
	}
	if (!cnt) {
		PLTFM_MSG_ERR("poll_own fail!!");
		return MACPOLLTO;
	}

	if (addr == R_AX_SS_SRAM_CTRL_1 || addr == R_AX_SS_LINK_INFO ||
	    addr == R_AX_SS_LINK_ADD || addr == R_AX_SS_LINK_DEL) {
		if (val32 & (BIT(29) | BIT(30))) {
			PLTFM_MSG_ERR("poll status fail!!");
			return MACPROCERR;
		}
	}
	return MACSUCCESS;
}

static u32 poll_own_val(struct mac_ax_adapter *adapter, u32 addr, u32 *r_val)
{
	u32 cnt = 100;
	u32 ck;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (addr == R_AX_SS_SRAM_CTRL_1 || addr == R_AX_SS_LINK_INFO ||
	    addr == R_AX_SS_SRAM_CTRL_0 || addr == R_AX_SS_LINK_ADD ||
	    addr == R_AX_SS_LINK_DEL || addr == R_AX_SS_LINK_SEARCH)
		ck = B_AX_SS_OWN;
	else
		ck = 0;

	while (--cnt) {
		*r_val = MAC_REG_R32(addr);
		if ((*r_val & ck) == 0)
			break;
		PLTFM_DELAY_US(1);
	}
	if (!cnt) {
		PLTFM_MSG_ERR("poll_own fail!!");
		return MACPOLLTO;
	}

	if (addr == R_AX_SS_SRAM_CTRL_1 || addr == R_AX_SS_LINK_INFO ||
	    addr == R_AX_SS_LINK_ADD || addr == R_AX_SS_LINK_DEL) {
		if (*r_val & (BIT(29) | BIT(30))) {
			PLTFM_MSG_ERR("poll status fail!!");
			return MACPROCERR;
		}
	}
	return MACSUCCESS;
}

static u32 get_sta_bmp_size(struct mac_ax_adapter *adapter, u8 *bmp_size)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8852A:
		*bmp_size = STA_SCH_BITMAP_SIZE_8852A;
		break;
	case MAC_AX_CHIP_ID_8852B:
		*bmp_size = STA_SCH_BITMAP_SIZE_8852B;
		break;
	case MAC_AX_CHIP_ID_8852C:
		*bmp_size = STA_SCH_BITMAP_SIZE_8852C;
		break;
	case MAC_AX_CHIP_ID_8192XB:
		*bmp_size = STA_SCH_BITMAP_SIZE_8852C;
		break;
	case MAC_AX_CHIP_ID_8851B:
		*bmp_size = STA_SCH_BITMAP_SIZE_8851B;
		break;
	default:
		return MACCHIPID;
	}
	return MACSUCCESS;
}

static u32 get_sta_bmp(struct mac_ax_adapter *adapter,
		       struct hv_ax_sta_bmp_ctrl *ctrl)
{
	u32 val32, sel;
	u8 macid_sh, macid_offset, set;
	u32 ret;
	u32 cmd;
	u8 macid;
	u32 bmp;
	u8 bmp_size;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_macid_pause_cfg cfg;

	macid = ctrl->macid;
	cfg.macid = macid;
	cfg.pause = 0;

	macid_offset = macid & (32 - 1);
	macid_sh = macid >> 5;
	bmp = 0;

	ret = get_sta_bmp_size(adapter, &bmp_size);
	if (ret != MACSUCCESS)
		return ret;

	ret = get_macid_pause(adapter, &cfg);
	if (ret != MACSUCCESS)
		return ret;
	bmp |= cfg.pause;

	for (sel = 1; sel < bmp_size; sel++) {
		cmd = B_AX_SS_OWN | SET_WORD(SRAM_BMP, B_AX_SS_CMD) |
		      (macid_sh << B_AX_SS_OFFSET_SH) | sel;
		ret = sram0_ctrl_r(adapter, cmd, &val32);
		if (ret != MACSUCCESS)
			return ret;

		set = (u8)((val32 & BIT(macid_offset)) ? 1 : 0);
		bmp |= set << sel;
	}

	ctrl->bmp = bmp;

	return MACSUCCESS;
}

static u32 set_sta_bmp(struct mac_ax_adapter *adapter,
		       struct hv_ax_sta_bmp_ctrl *ctrl)
{
	u32 sel;
	u8 macid_sh, macid_offset;
	u32 ret;
	u32 w_val;
	u32 cmd;
	u8 macid;
	u32 mask;
	u32 bmp;
	u8 bmp_size;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_macid_pause_cfg cfg;

	bmp = ctrl->bmp;
	mask = ctrl->mask;
	macid = ctrl->macid;
	macid_offset = macid & (32 - 1);
	macid_sh = macid >> 5;

	ret = get_sta_bmp_size(adapter, &bmp_size);
	if (ret != MACSUCCESS)
		return ret;

	if (mask & BIT(0)) {
		cfg.macid = macid;
		cfg.pause = bmp & BIT(0) ? 1 : 0;
		ret = set_macid_pause(adapter, &cfg);
		if (ret != MACSUCCESS)
			return ret;
	}

	for (sel = 1; sel < bmp_size; sel++) {
		if (mask & BIT(sel)) {
			if (bmp & BIT(sel))
				w_val = BIT(macid_offset);
			else
				w_val = 0;
			cmd =  B_AX_SS_OWN | B_AX_SS_RW |
			       SET_WORD(SRAM_BMP, B_AX_SS_CMD) |
			       (macid_sh << B_AX_SS_OFFSET_SH) | sel;
			ret = sram0_ctrl_w(adapter, cmd, BIT(macid_offset),
					   w_val);
			if (ret != MACSUCCESS)
				return ret;
		}
	}

	return MACSUCCESS;
}

static u32 sta_len_ctrl(struct mac_ax_adapter *adapter,
			struct hv_ax_sta_len_ctrl *ctrl)
{
	u32 cmd_sel;
	u32 cmd;

	switch (ctrl->cmd) {
	case HV_AX_STA_LEN_INCR:
		cmd_sel = CTRL1_INCR_TX_LEN;
		break;
	case HV_AX_STA_LEN_DECR:
		cmd_sel = CTRL1_DECR_TX_LEN;
		break;
	}

	cmd = B_AX_SS_OWN | SET_WORD(cmd_sel, B_AX_SS_CMD_SEL) |
	      SET_WORD(ctrl->ac, B_AX_SS_AC) |
	      SET_WORD(ctrl->len, B_AX_SS_VALUE) | ctrl->macid;

	return sram1_ctrl_chk(adapter, cmd);
}

static u32 get_sta_muru(struct mac_ax_adapter *adapter,
			struct hv_ax_sta_muru_ctrl *muru)
{
	u32 val32;
	u32 cmd;
	u32 ret;
	u8 macid;
	u8 vld;

	macid = muru->macid;

	ret = ul_vld_chk(adapter, &vld);
	if (ret != MACSUCCESS)
		return ret;
	if (vld != 0) {
		cmd = B_AX_SS_OWN | SET_WORD(CTRL1_R_UL_TBL, B_AX_SS_CMD_SEL) |
		      macid;
	ret = sram1_ctrl_val(adapter, cmd, &val32);
	if (ret != MACSUCCESS)
		return ret;

	muru->ul_tbl = (val32 >> MACID_SH) & UL_TBL_MSK;
	}

	ret = ru_vld_chk(adapter, &vld);
	if (ret != MACSUCCESS)
		return ret;
	if (vld == 0) {
		ret = mu_vld_chk(adapter, &vld);
		if (ret != MACSUCCESS)
			return ret;
		if (vld == 0) {
			muru->dl_muru_dis = 0;
			return MACSUCCESS;
		}
	}
	cmd = B_AX_SS_OWN |
	      SET_WORD(CTRL1_R_DL_MURU_DIS, B_AX_SS_CMD_SEL) |
	      macid;
	ret = sram1_ctrl_val(adapter, cmd, &val32);
	if (ret != MACSUCCESS)
		return ret;

	muru->dl_muru_dis = (val32 >> MACID_SH) & DL_MURU_MSK;

	return MACSUCCESS;
}

static u32 set_sta_muru(struct mac_ax_adapter *adapter,
			struct hv_ax_sta_muru_ctrl *muru)
{
	u32 cmd;
	u32 ret;
	u8 macid;
	u8 vld;

	macid = muru->macid;

	ret = ul_vld_chk(adapter, &vld);
	if (ret != MACSUCCESS)
		return ret;
	if (vld != 0) {
		cmd = B_AX_SS_OWN | SET_WORD(CTRL1_W_UL_TBL, B_AX_SS_CMD_SEL) |
		      SET_WORD(muru->ul_tbl, B_AX_SS_VALUE) | macid;
		ret = sram1_ctrl_chk(adapter, cmd);
	if (ret != MACSUCCESS)
		return ret;
	}

	ret = ru_vld_chk(adapter, &vld);
	if (ret != MACSUCCESS)
		return ret;
	if (vld == 0) {
		ret = mu_vld_chk(adapter, &vld);
		if (ret != MACSUCCESS)
			return ret;
		if (vld == 0)
			return MACSUCCESS;
	}
	cmd = B_AX_SS_OWN | SET_WORD(CTRL1_W_DL_MURU_DIS, B_AX_SS_CMD_SEL) |
	      SET_WORD(muru->dl_muru_dis, B_AX_SS_VALUE) | macid;
	ret = sram1_ctrl_chk(adapter, cmd);
	if (ret != MACSUCCESS)
		return ret;

	return MACSUCCESS;
}

static u32 get_sta_dl_rugrp(struct mac_ax_adapter *adapter,
			    struct hv_ax_sta_dl_rugrp_ctrl *rugrp)
{
	u32 sel;
	u32 dw[3];
	u8 grpid;
	u32 cmd;
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	grpid = rugrp->grpid;
	for (sel = 0; sel < 3; sel++) {
		cmd = B_AX_SS_OWN | SET_WORD(SRAM_DLRU, B_AX_SS_CMD) |
		      SET_WORD(sel, B_AX_SS_OFFSET) | grpid;
		ret = sram0_ctrl_r(adapter, cmd, &dw[sel]);
		if (ret != MACSUCCESS)
			return ret;
	}
	rugrp->dis_ac[7] = dw[0] & 0xF;
	rugrp->macid[7] = (dw[0] & 0xFF0) >> 4;
	rugrp->dis_ac[6] = (dw[0] & (0xF << 12)) >> 12;
	rugrp->macid[6] = (dw[0] & (0xFF0 << 12)) >> 16;
	rugrp->dis_ac[5] = (dw[0] & (0xF << 24)) >> 24;
	rugrp->macid[5] = ((dw[0] & (0xF0 << 24)) >> 28) |
			  ((dw[1] & 0xF) << 4);
	rugrp->dis_ac[4] = (dw[1] & (0xF << 4)) >> 4;
	rugrp->macid[4] = (dw[1] & (0xFF0 << 4)) >> 8;
	rugrp->dis_ac[3] = (dw[1] & (0xF << 16)) >> 16;
	rugrp->macid[3] = (dw[1] & (0xFF0 << 16)) >> 20;
	rugrp->dis_ac[2] = (dw[1] & (0xF << 28)) >> 28;
	rugrp->macid[2] = dw[2] & 0xFF;
	rugrp->dis_ac[1] = (dw[2] & (0xF << 8)) >> 8;
	rugrp->macid[1] = (dw[2] & (0xFF0 << 8)) >> 12;
	rugrp->dis_ac[0] = (dw[2] & (0xF << 20)) >> 20;
	rugrp->macid[0] = (dw[2] & (0xFF0 << 20)) >> 24;

	rugrp->grp_vld =
		(MAC_REG_R16(R_AX_SS_RU_CTRL + 2) & BIT(grpid)) >> grpid;

	return MACSUCCESS;
}

static u32 set_sta_dl_rugrp(struct mac_ax_adapter *adapter,
			    struct hv_ax_sta_dl_rugrp_ctrl *rugrp)
{
	u32 sel;
	u32 dw[3];
	u8 grpid;
	u32 cmd;
	u32 ret;
	u16 val16;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	grpid = rugrp->grpid;
	MAC_REG_W16(R_AX_SS_RU_CTRL + 2,
		    MAC_REG_R16(R_AX_SS_RU_CTRL + 2) & ~(BIT(grpid)));

	dw[0] = rugrp->dis_ac[7] | (rugrp->macid[7] << 4) |
		(rugrp->dis_ac[6] << 12) | (rugrp->macid[6] << 16) |
		(rugrp->dis_ac[5] << 24) | ((rugrp->macid[5] & 0x0F) << 28);
	dw[1] = ((rugrp->macid[5] & 0xF0) >> 4) |
		(rugrp->dis_ac[4] << 4) | (rugrp->macid[4] << 8) |
		(rugrp->dis_ac[3] << 16) | (rugrp->macid[3] << 20) |
		(rugrp->dis_ac[2] << 28);
	dw[2] = rugrp->macid[2] |
		(rugrp->dis_ac[1] << 8) | (rugrp->macid[1] << 12) |
		(rugrp->dis_ac[0] << 20) | (rugrp->macid[0] << 24);

	for (sel = 0; sel < 3; sel++) {
		cmd = B_AX_SS_OWN | B_AX_SS_RW |
		      SET_WORD(SRAM_DLRU, B_AX_SS_CMD) |
		      SET_WORD(sel, B_AX_SS_OFFSET) | grpid;
		ret = sram0_ctrl_w(adapter, cmd, 0xFFFFFFFF, dw[sel]);
		if (ret != MACSUCCESS)
			return ret;
	}

	val16 = MAC_REG_R16(R_AX_SS_RU_CTRL + 2);
	if (((val16 & BIT(grpid)) >> grpid) !=
	    rugrp->grp_vld) {
		if (rugrp->grp_vld)
			MAC_REG_W16(R_AX_SS_RU_CTRL + 2, val16 | BIT(grpid));
		else
			MAC_REG_W16(R_AX_SS_RU_CTRL + 2,
				    val16 & ~(BIT(grpid)));
	}

	return MACSUCCESS;
}

static u32 wmm_vld_chk(struct mac_ax_adapter *adapter, u8 *vld, u8 wmm)
{
	u8 wmm_num;

	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8852A:
		wmm_num = STA_SCH_WMM_NUM_8852A;
		break;
	case MAC_AX_CHIP_ID_8852B:
		wmm_num = STA_SCH_WMM_NUM_8852B;
		break;
	case MAC_AX_CHIP_ID_8852C:
		wmm_num = STA_SCH_WMM_NUM_8852C;
		break;
	case MAC_AX_CHIP_ID_8192XB:
		wmm_num = STA_SCH_WMM_NUM_8192XB;
		break;
	case MAC_AX_CHIP_ID_8851B:
		wmm_num = STA_SCH_WMM_NUM_8851B;
		break;
	default:
		return MACCHIPID;
	}

	if (wmm < wmm_num)
		*vld = 1;
	else
		*vld = 0;

	return MACSUCCESS;
}

static u32 ul_vld_chk(struct mac_ax_adapter *adapter, u8 *vld)
{
	u8 ul_support;

	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8852A:
		ul_support = STA_SCH_UL_SUPPORT_8852A;
		break;
	case MAC_AX_CHIP_ID_8852B:
		ul_support = STA_SCH_UL_SUPPORT_8852B;
		break;
	case MAC_AX_CHIP_ID_8852C:
		ul_support = STA_SCH_UL_SUPPORT_8852C;
		break;
	case MAC_AX_CHIP_ID_8192XB:
		ul_support = STA_SCH_UL_SUPPORT_8192XB;
		break;
	case MAC_AX_CHIP_ID_8851B:
		ul_support = STA_SCH_UL_SUPPORT_8851B;
		break;
	default:
		return MACCHIPID;
	}

	if (ul_support)
		*vld = 1;
	else
		*vld = 0;

	return MACSUCCESS;
}

static u32 mu_vld_chk(struct mac_ax_adapter *adapter, u8 *vld)
{
	u8 mu_support;

	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8852A:
		mu_support = STA_SCH_MU_SUPPORT_8852A;
		break;
	case MAC_AX_CHIP_ID_8852B:
		mu_support = STA_SCH_MU_SUPPORT_8852B;
		break;
	case MAC_AX_CHIP_ID_8852C:
		mu_support = STA_SCH_MU_SUPPORT_8852C;
		break;
	case MAC_AX_CHIP_ID_8192XB:
		mu_support = STA_SCH_MU_SUPPORT_8192XB;
		break;
	case MAC_AX_CHIP_ID_8851B:
		mu_support = STA_SCH_MU_SUPPORT_8851B;
		break;
	default:
		return MACCHIPID;
	}

	if (mu_support)
		*vld = 1;
	else
		*vld = 0;

	return MACSUCCESS;
}

static u32 ru_vld_chk(struct mac_ax_adapter *adapter, u8 *vld)
{
	u8 ru_support;

	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8852A:
		ru_support = STA_SCH_RU_SUPPORT_8852A;
		break;
	case MAC_AX_CHIP_ID_8852B:
		ru_support = STA_SCH_RU_SUPPORT_8852B;
		break;
	case MAC_AX_CHIP_ID_8852C:
		ru_support = STA_SCH_RU_SUPPORT_8852C;
		break;
	case MAC_AX_CHIP_ID_8192XB:
		ru_support = STA_SCH_RU_SUPPORT_8192XB;
		break;
	case MAC_AX_CHIP_ID_8851B:
		ru_support = STA_SCH_RU_SUPPORT_8851B;
		break;
	default:
		return MACCHIPID;
	}

	if (ru_support)
		*vld = 1;
	else
		*vld = 0;

	return MACSUCCESS;
}

static u32 get_sta_link(struct mac_ax_adapter *adapter,
			struct hv_ax_ss_link_info *link)
{
	u32 val32, i;
	u8 macid, wmm, ac;
	u32 ret;
	u32 cmd;
	u16 id_empty = adapter->hw_info->sta_empty_flg;

	if (link->ul) {
		wmm = 0;
		ac = 0;
	} else {
		wmm = link->wmm;
		ac = link->ac;
	}

	cmd = B_AX_SS_OWN | (link->ul ? B_AX_SS_UL : 0) |
	      SET_WORD(wmm, B_AX_SS_WMM) | SET_WORD(ac, B_AX_SS_AC);
	ret = get_link_info(adapter, cmd, &val32);
	if (ret != MACSUCCESS)
		return ret;

	link->link_head = GET_FIELD(val32, B_AX_SS_LINK_HEAD);
	link->link_tail = GET_FIELD(val32, B_AX_SS_LINK_TAIL);
	link->link_len = GET_FIELD(val32, B_AX_SS_LINK_LEN);
	macid = link->link_head;

	if (link->link_head == id_empty) {
		if (link->link_len) {
			PLTFM_MSG_ERR("empty link_len error!!");
			return MACSSLINK;
		}
	} else {
		i = 0;
		do {
			link->link_list[i] = macid;
			link->link_bitmap[macid] = 1;
			cmd = B_AX_SS_OWN |
			      SET_WORD(CTRL1_R_NEXT_LINK, B_AX_SS_CMD_SEL) |
			      SET_WORD(ac, B_AX_SS_AC) |
			      (link->ul ? BIT(23) : 0) | macid;
			ret = sram1_ctrl_val(adapter, cmd, &val32);
			if (ret != MACSUCCESS)
				return ret;

			macid = GET_FIELD(val32, B_AX_SS_VALUE);
			if (macid == id_empty) {
				if (link->link_list[i] != link->link_tail) {
					PLTFM_MSG_ERR("link_tail error!!");
					return MACSSLINK;
				}
				if (i >= link->link_len) {
					PLTFM_MSG_ERR("link_len error!!");
					return MACSSLINK;
				}
				break;
			}
			i++;
		} while (i < SS_LINK_SIZE);
	}

	return MACSUCCESS;
}

static void get_dl_su_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info)
{
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32 = MAC_REG_R32(R_AX_SS_DL_RPT_CRTL);
	info->wmm0_max = GET_FIELD(val32, B_AX_SS_MAX_SU_NUM_0);
	info->wmm1_max = GET_FIELD(val32, B_AX_SS_MAX_SU_NUM_1);
	info->twt_wmm0_max = GET_FIELD(val32, B_AX_SS_TWT_MAX_SU_NUM_0);
	info->twt_wmm1_max = GET_FIELD(val32, B_AX_SS_TWT_MAX_SU_NUM_1);
}

static void set_dl_su_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info)
{
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32 = MAC_REG_R32(R_AX_SS_DL_RPT_CRTL);
	val32 = SET_CLR_WORD(val32, info->wmm0_max, B_AX_SS_MAX_SU_NUM_0);
	val32 = SET_CLR_WORD(val32, info->wmm1_max, B_AX_SS_MAX_SU_NUM_1);
	val32 =	SET_CLR_WORD(val32, info->twt_wmm0_max,
			     B_AX_SS_TWT_MAX_SU_NUM_0);
	val32 =	SET_CLR_WORD(val32, info->twt_wmm1_max,
			     B_AX_SS_TWT_MAX_SU_NUM_1);
	MAC_REG_W32(R_AX_SS_DL_RPT_CRTL, val32);
}

static void get_dl_mu_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info)
{
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32 = MAC_REG_R32(R_AX_SS_DL_MU_RPT_CRTL);
	info->wmm0_max = GET_FIELD(val32, B_AX_SS_MAX_MU_NUM_0);
	info->wmm1_max = GET_FIELD(val32, B_AX_SS_MAX_MU_NUM_1);
	info->twt_wmm0_max = GET_FIELD(val32, B_AX_SS_TWT_MAX_MU_NUM_0);
	info->twt_wmm1_max = GET_FIELD(val32, B_AX_SS_TWT_MAX_MU_NUM_1);
}

static void set_dl_mu_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info)
{
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32 = MAC_REG_R32(R_AX_SS_DL_MU_RPT_CRTL);
	val32 = SET_CLR_WORD(val32, info->wmm0_max, B_AX_SS_MAX_MU_NUM_0);
	val32 = SET_CLR_WORD(val32, info->wmm1_max, B_AX_SS_MAX_MU_NUM_1);
	val32 =	SET_CLR_WORD(val32, info->twt_wmm0_max,
			     B_AX_SS_TWT_MAX_MU_NUM_0);
	val32 =	SET_CLR_WORD(val32, info->twt_wmm1_max,
			     B_AX_SS_TWT_MAX_MU_NUM_1);
	MAC_REG_W32(R_AX_SS_DL_MU_RPT_CRTL, val32);
}

static void get_dl_ru_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info)
{
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32 = MAC_REG_R32(R_AX_SS_DL_RU_RPT_CRTL);
	info->wmm0_max = GET_FIELD(val32, B_AX_SS_MAX_RU_NUM_0);
	info->wmm1_max = GET_FIELD(val32, B_AX_SS_MAX_RU_NUM_1);
	info->twt_wmm0_max = GET_FIELD(val32, B_AX_SS_TWT_MAX_RU_NUM_0);
	info->twt_wmm1_max = GET_FIELD(val32, B_AX_SS_TWT_MAX_RU_NUM_1);
}

static void set_dl_ru_rpt(struct mac_ax_adapter *adapter,
			  struct hv_ax_ss_dl_rpt_info *info)
{
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32 = MAC_REG_R32(R_AX_SS_DL_RU_RPT_CRTL);
	val32 = SET_CLR_WORD(val32, info->wmm0_max, B_AX_SS_MAX_RU_NUM_0);
	val32 = SET_CLR_WORD(val32, info->wmm1_max, B_AX_SS_MAX_RU_NUM_1);
	val32 =	SET_CLR_WORD(val32, info->twt_wmm0_max,
			     B_AX_SS_TWT_MAX_RU_NUM_0);
	val32 =	SET_CLR_WORD(val32, info->twt_wmm1_max,
			     B_AX_SS_TWT_MAX_RU_NUM_1);
	MAC_REG_W32(R_AX_SS_DL_RU_RPT_CRTL, val32);
}

static void get_ul_rpt(struct mac_ax_adapter *adapter,
		       struct hv_ax_ss_ul_rpt_info *info)
{
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32 = MAC_REG_R32(R_AX_SS_UL_RPT_CRTL);
	info->ul_wmm_sel = GET_FIELD(val32, B_AX_SS_UL_WMM);
	info->ul_su_max = GET_FIELD(val32, B_AX_SS_MAX_SU_NUM_UL);
	info->twt_ul_su_max = GET_FIELD(val32, B_AX_SS_TWT_MAX_SU_NUM_UL);
	info->ul_ru_max = GET_FIELD(val32, B_AX_SS_MAX_RU_NUM_UL);
}

static void set_ul_rpt(struct mac_ax_adapter *adapter,
		       struct hv_ax_ss_ul_rpt_info *info)
{
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32 = MAC_REG_R32(R_AX_SS_UL_RPT_CRTL);
	val32 = SET_CLR_WORD(val32, info->ul_wmm_sel, B_AX_SS_UL_WMM);
	val32 = SET_CLR_WORD(val32, info->ul_su_max, B_AX_SS_MAX_SU_NUM_UL);
	val32 = SET_CLR_WORD(val32, info->twt_ul_su_max,
			     B_AX_SS_TWT_MAX_SU_NUM_UL);
	val32 = SET_CLR_WORD(val32, info->ul_ru_max, B_AX_SS_MAX_RU_NUM_UL);
	MAC_REG_W32(R_AX_SS_UL_RPT_CRTL, val32);
}

#if MAC_AX_8852A_SUPPORT || MAC_AX_8852B_SUPPORT || MAC_AX_8851B_SUPPORT
static u32 get_sta_dl_mutbl(struct mac_ax_adapter *adapter,
			    struct hv_ax_sta_dl_mutbl_ctrl *mutbl)
{
	u32 reg;
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8851B)) {
		switch (mutbl->tbl_id) {
		case 0:
			reg = R_AX_SS_MU_TBL_0;
			break;
		case 1:
			reg = R_AX_SS_MU_TBL_1;
			break;
		case 2:
			reg = R_AX_SS_MU_TBL_2;
			break;
		case 3:
			reg = R_AX_SS_MU_TBL_3;
			break;
		case 4:
			reg = R_AX_SS_MU_TBL_4;
			break;
		case 5:
			reg = R_AX_SS_MU_TBL_5;
			break;
		default:
			return MACFUNCINPUT;
		}

		val32 = MAC_REG_R32(reg);
		mutbl->macid = GET_FIELD(val32, B_AX_SS_MU_MACID);
		mutbl->tbl_vld = val32 & B_AX_SS_TBL_VLD ? 1 : 0;
		mutbl->score[0] = GET_FIELD(val32, B_AX_SS_SCORE_0);
		mutbl->score[1] = GET_FIELD(val32, B_AX_SS_SCORE_1);
		mutbl->score[2] = GET_FIELD(val32, B_AX_SS_SCORE_2);
		mutbl->score[3] = GET_FIELD(val32, B_AX_SS_SCORE_3);
		mutbl->score[4] = GET_FIELD(val32, B_AX_SS_SCORE_4);
	}
	return MACSUCCESS;
}

static u32 set_sta_dl_mutbl(struct mac_ax_adapter *adapter,
			    struct hv_ax_sta_dl_mutbl_ctrl *mutbl)
{
	u32 reg;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8851B)) {
		switch (mutbl->tbl_id) {
		case 0:
			reg = R_AX_SS_MU_TBL_0;
			break;
		case 1:
			reg = R_AX_SS_MU_TBL_1;
			break;
		case 2:
			reg = R_AX_SS_MU_TBL_2;
			break;
		case 3:
			reg = R_AX_SS_MU_TBL_3;
			break;
		case 4:
			reg = R_AX_SS_MU_TBL_4;
			break;
		case 5:
			reg = R_AX_SS_MU_TBL_5;
			break;
		default:
			return MACFUNCINPUT;
		}

		MAC_REG_W32(reg, SET_WORD(mutbl->macid, B_AX_SS_MU_MACID) |
			    (mutbl->tbl_vld ? B_AX_SS_TBL_VLD : 0) |
			    SET_WORD(mutbl->score[0], B_AX_SS_SCORE_0) |
			    SET_WORD(mutbl->score[1], B_AX_SS_SCORE_1) |
			    SET_WORD(mutbl->score[2], B_AX_SS_SCORE_2) |
			    SET_WORD(mutbl->score[3], B_AX_SS_SCORE_3) |
			    SET_WORD(mutbl->score[4], B_AX_SS_SCORE_4));
	}

	return MACSUCCESS;
}
#endif

static void get_ss_quota_mode(struct mac_ax_adapter *adapter,
			      struct hv_ax_ss_quota_mode_ctrl *ctrl)
{
	u32 val32_wmm;
	u32 val32_ul;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32_wmm = MAC_REG_R32(R_AX_SS_DL_QUOTA_CTRL);
	val32_ul = MAC_REG_R32(R_AX_SS_UL_QUOTA_CTRL);
	switch (ctrl->wmm) {
	case HV_AX_SS_WMM0:
		ctrl->mode = val32_wmm & B_AX_SS_QUOTA_MODE_0 ?
			     HV_AX_SS_QUOTA_MODE_CNT : HV_AX_SS_QUOTA_MODE_TIME;
		break;
	case HV_AX_SS_WMM1:
		ctrl->mode = val32_wmm & B_AX_SS_QUOTA_MODE_1 ?
			     HV_AX_SS_QUOTA_MODE_CNT : HV_AX_SS_QUOTA_MODE_TIME;
		break;
	case HV_AX_SS_WMM2:
		ctrl->mode = val32_wmm & B_AX_SS_QUOTA_MODE_2 ?
			     HV_AX_SS_QUOTA_MODE_CNT : HV_AX_SS_QUOTA_MODE_TIME;
		break;
	case HV_AX_SS_WMM3:
		ctrl->mode = val32_wmm & B_AX_SS_QUOTA_MODE_3 ?
			     HV_AX_SS_QUOTA_MODE_CNT : HV_AX_SS_QUOTA_MODE_TIME;
		break;
	case HV_AX_SS_UL:
		ctrl->mode = val32_ul & B_AX_SS_QUOTA_MODE_UL ?
			     HV_AX_SS_QUOTA_MODE_CNT : HV_AX_SS_QUOTA_MODE_TIME;
		break;
	}
}

static u32 set_ss_quota_mode(struct mac_ax_adapter *adapter,
			     struct hv_ax_ss_quota_mode_ctrl *ctrl)
{
	u32 val32_wmm;
	u32 val32_ul;
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32_wmm = MAC_REG_R32(R_AX_SS_DL_QUOTA_CTRL);
	val32_ul = MAC_REG_R32(R_AX_SS_UL_QUOTA_CTRL);
	switch (ctrl->wmm) {
	case HV_AX_SS_WMM0:
		if (ctrl->mode == HV_AX_SS_QUOTA_MODE_CNT)
			MAC_REG_W32(R_AX_SS_DL_QUOTA_CTRL,
				    val32_wmm | B_AX_SS_QUOTA_MODE_0);
		else
			MAC_REG_W32(R_AX_SS_DL_QUOTA_CTRL,
				    val32_wmm & ~B_AX_SS_QUOTA_MODE_0);
		break;
	case HV_AX_SS_WMM1:
		if (ctrl->mode == HV_AX_SS_QUOTA_MODE_CNT)
			MAC_REG_W32(R_AX_SS_DL_QUOTA_CTRL,
				    val32_wmm | B_AX_SS_QUOTA_MODE_1);
		else
			MAC_REG_W32(R_AX_SS_DL_QUOTA_CTRL,
				    val32_wmm & ~B_AX_SS_QUOTA_MODE_1);
		break;
	case HV_AX_SS_WMM2:
		if (ctrl->mode == HV_AX_SS_QUOTA_MODE_CNT)
			MAC_REG_W32(R_AX_SS_DL_QUOTA_CTRL,
				    val32_wmm | B_AX_SS_QUOTA_MODE_2);
		else
			MAC_REG_W32(R_AX_SS_DL_QUOTA_CTRL,
				    val32_wmm & ~B_AX_SS_QUOTA_MODE_2);
		break;
	case HV_AX_SS_WMM3:
		if (ctrl->mode == HV_AX_SS_QUOTA_MODE_CNT)
			MAC_REG_W32(R_AX_SS_DL_QUOTA_CTRL,
				    val32_wmm | B_AX_SS_QUOTA_MODE_3);
		else
			MAC_REG_W32(R_AX_SS_DL_QUOTA_CTRL,
				    val32_wmm & ~B_AX_SS_QUOTA_MODE_3);
		break;
	case HV_AX_SS_UL:
		if (ctrl->mode == HV_AX_SS_QUOTA_MODE_CNT)
			MAC_REG_W32(R_AX_SS_UL_QUOTA_CTRL,
				    val32_ul | B_AX_SS_QUOTA_MODE_UL);
		else
			MAC_REG_W32(R_AX_SS_UL_QUOTA_CTRL,
				    val32_ul & ~B_AX_SS_QUOTA_MODE_UL);
		break;
	}

	switch (ctrl->wmm) {
	case HV_AX_SS_WMM0:
	case HV_AX_SS_WMM1:
	case HV_AX_SS_UL:
		ret = check_mac_en(adapter, 0, MAC_AX_CMAC_SEL);
		if (ret != MACSUCCESS)
			return ret;
		val32_wmm = MAC_REG_R32(R_AX_PTCL_ATM);
		if (ctrl->mode == HV_AX_SS_QUOTA_MODE_TIME)
			MAC_REG_W32(R_AX_PTCL_ATM,
				    val32_wmm | B_AX_ATM_AIRTIME_EN);
		break;
	case HV_AX_SS_WMM2:
	case HV_AX_SS_WMM3:
		ret = check_mac_en(adapter, 1, MAC_AX_CMAC_SEL);
		if (ret != MACSUCCESS)
			return ret;
		val32_wmm = MAC_REG_R32(R_AX_PTCL_ATM_C1);
		if (ctrl->mode == HV_AX_SS_QUOTA_MODE_TIME)
			MAC_REG_W32(R_AX_PTCL_ATM_C1,
				    val32_wmm | B_AX_ATM_AIRTIME_EN);
		break;
	}

	return MACSUCCESS;
}

static void set_ss_wmm_tbl(struct mac_ax_adapter *adapter,
			   struct mac_ax_ss_wmm_tbl_ctrl *ctrl)
{
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	val32 = MAC_REG_R32(R_AX_SS_CTRL);
	switch (ctrl->wmm) {
	case 0:
		val32 = SET_CLR_WORD(val32, ctrl->wmm_mapping,
				     B_AX_SS_WMM_SEL_0);
		break;
	case 1:
		val32 = SET_CLR_WORD(val32, ctrl->wmm_mapping,
				     B_AX_SS_WMM_SEL_1);
		break;
	case 2:
		val32 = SET_CLR_WORD(val32, ctrl->wmm_mapping,
				     B_AX_SS_WMM_SEL_2);
		break;
	case 3:
		val32 = SET_CLR_WORD(val32, ctrl->wmm_mapping,
				     B_AX_SS_WMM_SEL_3);
		break;
	default:
		break;
	}
	MAC_REG_W32(R_AX_SS_CTRL, val32);
}

static u32 switch_wmm_link(struct mac_ax_adapter *adapter,
			   enum mac_ax_ss_wmm_tbl src_link,
			   enum mac_ax_ss_wmm_tbl dst_link,
			   enum hv_ax_ss_wmm src_wmm)
{
	u32 ret;
	u32 i;
	struct hv_ax_ss_link_info link;
	struct hv_ax_ops *hv_ops = adapter->hv_ops;
	struct mac_role_tbl *role;

	link.ul = 0;
	for (link.ac = 0; link.ac < 4; link.ac++) {
		PLTFM_MEMSET(link.link_list, 0xFF, SS_LINK_SIZE);
		link.wmm = src_link;
		ret = hv_ops->sta_link_cfg(adapter, &link,
					   HV_AX_SS_LINK_CFG_GET);
		if (ret != MACSUCCESS)
			return ret;
		for (i = 0; i < link.link_len; i += 3) {
			link.macid0 = link.link_list[i];
			link.macid1 = link.link_list[i + 1];
			link.macid2 = link.link_list[i + 2];
			if (link.macid0 != 0xFF) {
				role = mac_role_srch(adapter, link.macid0);
				if (!role)
					return MACNOITEM;
				if ((hv_ax_ss_wmm)role->wmm != src_wmm)
					link.macid0 = 0xFF;
			}
			if (link.macid1 != 0xFF) {
				role = mac_role_srch(adapter, link.macid1);
				if (!role)
					return MACNOITEM;
				if ((hv_ax_ss_wmm)role->wmm != src_wmm)
					link.macid1 = 0xFF;
			}
			if (link.macid2 != 0xFF) {
				role = mac_role_srch(adapter, link.macid2);
				if (!role)
					return MACNOITEM;
				if ((hv_ax_ss_wmm)role->wmm != src_wmm)
					link.macid2 = 0xFF;
			}
			if (link.macid0 != 0xFF || link.macid1 != 0xFF ||
			    link.macid2 != 0xFF) {
				ret = switch_wmm_macid(adapter, &link, src_link,
						       dst_link);
				if (ret != MACSUCCESS)
					return ret;
			}
		}
	}
	return MACSUCCESS;
}

static u32 switch_wmm_macid(struct mac_ax_adapter *adapter,
			    struct hv_ax_ss_link_info *link,
			    enum mac_ax_ss_wmm_tbl src_link,
			    enum mac_ax_ss_wmm_tbl dst_link)
{
	u32 ret;
	struct hv_ax_ops *hv_ops = adapter->hv_ops;

	link->wmm = src_link;
	ret = hv_ops->sta_link_cfg(adapter, link,
				   HV_AX_SS_LINK_CFG_DEL);
	if (ret != MACSUCCESS)
		return ret;
	link->wmm = dst_link;
	ret = hv_ops->sta_link_cfg(adapter, link,
				   HV_AX_SS_LINK_CFG_ADD);
	if (ret != MACSUCCESS)
		return ret;
	if (link->macid0 != 0xFF) {
		ret = hv_ops->ss_set_wmm_bmp(adapter, link->wmm, link->macid0);
		if (ret != MACSUCCESS)
			return ret;
	}
	if (link->macid1 != 0xFF) {
		ret = hv_ops->ss_set_wmm_bmp(adapter, link->wmm, link->macid1);
		if (ret != MACSUCCESS)
			return ret;
	}
	if (link->macid2 != 0xFF) {
		ret = hv_ops->ss_set_wmm_bmp(adapter, link->wmm, link->macid2);
		if (ret != MACSUCCESS)
			return ret;
	}

	return MACSUCCESS;
}

u32 hv_sta_dl_mutbl_cfg(struct mac_ax_adapter *adapter,
			struct hv_ax_sta_dl_mutbl_ctrl *mutbl,
			enum hv_ax_sta_muru_cfg cfg)
{
	u32 ret = MACSUCCESS;

#if MAC_AX_8852A_SUPPORT || MAC_AX_8852B_SUPPORT || MAC_AX_8851B_SUPPORT
	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8851B)) {
		switch (cfg) {
		case HV_AX_STA_MURU_CFG_GET:
			ret = get_sta_dl_mutbl(adapter, mutbl);
			break;
		case HV_AX_STA_MURU_CFG_SET:
			ret = set_sta_dl_mutbl(adapter, mutbl);
			break;
		}
	}
#endif

	return ret;
}

u32 hv_sta_bmp_cfg(struct mac_ax_adapter *adapter,
		   struct hv_ax_sta_bmp_ctrl *ctrl, enum hv_ax_sta_bmp_cfg cfg)
{
	u32 ret;

	switch (cfg) {
	case HV_AX_STA_BMP_CFG_GET:
		ret = get_sta_bmp(adapter, ctrl);
		break;
	case HV_AX_STA_BMP_CFG_SET:
		ret = set_sta_bmp(adapter, ctrl);
		break;
	}

	return ret;
}

u32 hv_sta_dl_rugrp_cfg(struct mac_ax_adapter *adapter,
			struct hv_ax_sta_dl_rugrp_ctrl *rugrp,
			enum hv_ax_sta_muru_cfg cfg)
{
	u32 ret;
	u8 vld;

	ret = ru_vld_chk(adapter, &vld);
	if (ret != MACSUCCESS)
		return ret;
	if (vld == 0) {
		if (cfg == HV_AX_STA_MURU_CFG_GET)
			PLTFM_MEMSET(rugrp, 0, sizeof(hv_ax_sta_dl_rugrp_ctrl));
		return MACSUCCESS;
	}

	switch (cfg) {
	case HV_AX_STA_MURU_CFG_GET:
		ret = get_sta_dl_rugrp(adapter, rugrp);
		break;
	case HV_AX_STA_MURU_CFG_SET:
		ret = set_sta_dl_rugrp(adapter, rugrp);
		break;
	}

	return ret;
}

u32 hv_sta_muru_cfg(struct mac_ax_adapter *adapter,
		    struct hv_ax_sta_muru_ctrl *muru,
		    enum hv_ax_sta_muru_cfg cfg)
{
	u32 ret;

	switch (cfg) {
	case HV_AX_STA_MURU_CFG_GET:
		ret = get_sta_muru(adapter, muru);
		break;
	case HV_AX_STA_MURU_CFG_SET:
		ret = set_sta_muru(adapter, muru);
		break;
	}

	return ret;
}

u32 hv_sta_len_cfg(struct mac_ax_adapter *adapter, struct hv_ax_sta_len *len,
		   enum hv_ax_sta_len_cfg cfg)
{
	u32 ret;

	switch (cfg) {
	case HV_AX_STA_LEN_CFG_GET:
		ret = get_sta_len_indir(adapter, len);
		break;
	case HV_AX_STA_LEN_CFG_SET:
		ret = set_sta_len_indir(adapter, len);
		break;
	case HV_AX_STA_LEN_CFG_GET_INDIR:
		ret = get_sta_len_indir(adapter, len);
		break;
	case HV_AX_STA_LEN_CFG_SET_INDIR:
		ret = set_sta_len_indir(adapter, len);
		break;
	}

	return ret;
}

u32 hv_sta_quota_cfg(struct mac_ax_adapter *adapter,
		     struct hv_ax_sta_quota *quota,
		     enum hv_ax_sta_quota_cfg cfg)
{
	u32 ret;

	switch (cfg) {
	case HV_AX_STA_QUOTA_CFG_VAL_GET:
		ret = get_sta_quota(adapter, quota);
		break;
	case HV_AX_STA_QUOTA_CFG_VAL_SET:
		ret = set_sta_quota(adapter, quota);
		break;
	case HV_AX_STA_QUOTA_CFG_SETTING_GET:
		ret = get_sta_quota_setting(adapter, quota);
		break;
	case HV_AX_STA_QUOTA_CFG_SETTING_SET:
		ret = set_sta_quota_setting(adapter, quota);
		break;
	}

	return ret;
}

u32 hv_sta_link_cfg(struct mac_ax_adapter *adapter,
		    struct hv_ax_ss_link_info *link, enum hv_ax_ss_link_cfg cfg)
{
	u32 ret;
	u8 vld;

	if (link->ul) {
		ret = ul_vld_chk(adapter, &vld);
		if (ret != MACSUCCESS)
			return ret;
		if (vld == 0)
			return MACSUCCESS;
	}

	ret = wmm_vld_chk(adapter, &vld, link->wmm);
	if (ret != MACSUCCESS)
		return ret;
	if (vld == 0)
		return MACSUCCESS;

	switch (cfg) {
	case HV_AX_SS_LINK_CFG_GET:
		ret = get_sta_link(adapter, link);
		break;
	case HV_AX_SS_LINK_CFG_ADD:
		ret = add_to_link(adapter, link);
		break;
	case HV_AX_SS_LINK_CFG_DEL:
		ret = del_from_link(adapter, link);
		break;
	case HV_AX_SS_LINK_CFG_CLEAN:
		ret = clean_sta_link(adapter, link);
		break;
	}

	return ret;
}

void hv_ss_dl_rpt_cfg(struct mac_ax_adapter *adapter,
		      struct hv_ax_ss_dl_rpt_info *info,
		      enum hv_ax_ss_rpt_cfg cfg)
{
	switch (cfg) {
	case HV_AX_SS_DL_SU_RPT_CFG_GET:
		get_dl_su_rpt(adapter, info);
		break;
	case HV_AX_SS_DL_SU_RPT_CFG_SET:
		set_dl_su_rpt(adapter, info);
		break;
	case HV_AX_SS_DL_MU_RPT_CFG_GET:
		get_dl_mu_rpt(adapter, info);
		break;
	case HV_AX_SS_DL_MU_RPT_CFG_SET:
		set_dl_mu_rpt(adapter, info);
		break;
	case HV_AX_SS_DL_RU_RPT_CFG_GET:
		get_dl_ru_rpt(adapter, info);
		break;
	case HV_AX_SS_DL_RU_RPT_CFG_SET:
		set_dl_ru_rpt(adapter, info);
		break;
	}
}

void hv_ss_ul_rpt_cfg(struct mac_ax_adapter *adapter,
		      struct hv_ax_ss_ul_rpt_info *info,
		      enum hv_ax_ss_rpt_cfg cfg)
{
	switch (cfg) {
	case HV_AX_SS_UL_RPT_CFG_GET:
		get_ul_rpt(adapter, info);
		break;
	case HV_AX_SS_UL_RPT_CFG_SET:
		set_ul_rpt(adapter, info);
		break;
	default:
		break;
	}
}

u32 hv_ss_query_search(struct mac_ax_adapter *adapter,
		       struct hv_ax_ss_search_info *info)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 wmm, ac;
	u32 ret;
	u32 poll_val;

	if (info->ul) {
		wmm = 0;
		ac = 0;
	} else {
		wmm = info->wmm;
		ac = info->ac;
	}

	MAC_REG_W32(R_AX_SS_LINK_SEARCH, B_AX_SS_OWN |
		    SET_WORD(ac, B_AX_SS_AC) | SET_WORD(wmm, B_AX_SS_WMM) |
		    (info->ul ? B_AX_SS_UL : 0) |
		    SET_WORD(info->twt_grp, B_AX_SS_TWT_GROUP) |
		    SET_WORD(info->mode_sel, B_AX_SS_MODE_SEL) |
		    SET_WORD(info->macid, B_AX_SS_MACID_0));
	ret = poll_own_val(adapter, R_AX_SS_LINK_SEARCH, &poll_val);
	if (ret != MACSUCCESS)
		return ret;

	if ((poll_val & (BIT(29) | BIT(30))) != 0)
		info->search_fail = SEARCH_FAIL;
	else
		info->search_fail = SEARCH_OK;

	return MACSUCCESS;
}

void hv_ss_rpt_path_cfg(struct mac_ax_adapter *adapter,
			enum hv_ax_ss_rpt_path_cfg cfg)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;

	val32 = MAC_REG_R32(R_AX_SS2FINFO_PATH);

	if (cfg == HV_AX_SS_RPT_PATH_HOST) {
		val32 = SET_CLR_WORD(val32, 0x2, B_AX_SS_DEST_QUEUE);
	} else {
		val32 = MAC_REG_R32(R_AX_SS2FINFO_PATH);
		val32 = SET_CLR_WORD(val32, 0xA, B_AX_SS_DEST_QUEUE);
		MAC_REG_W32(R_AX_SS2FINFO_PATH, val32);
	}

	MAC_REG_W32(R_AX_SS2FINFO_PATH, val32);
}

void hv_ss_set_bsr_thold(struct mac_ax_adapter *adapter, u16 thold_0,
			 u16 thold_1)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;

	val32 = MAC_REG_R32(R_AX_SS_BSR_CTRL);
	val32 = SET_CLR_WORD(val32, thold_0, B_AX_SS_BSR_THR_0);
	val32 = SET_CLR_WORD(val32, thold_1, B_AX_SS_BSR_THR_1);
	MAC_REG_W32(R_AX_SS_BSR_CTRL, val32);
}

void hv_ss_dlru_search_mode(struct mac_ax_adapter *adapter,
			    enum hv_ax_ss_dlru_search_mode mode)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 val8;

	val8 = MAC_REG_R8(R_AX_SS_RU_CTRL);
	val8 = SET_CLR_WORD(val8, mode, B_AX_SS_RU_SEARCH_MODE);
	MAC_REG_W8(R_AX_SS_RU_CTRL, val8);
}

void hv_ss_set_delay_tx(struct mac_ax_adapter *adapter,
			struct hv_ax_ss_delay_tx_info *info)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;

	val32 = MAC_REG_R32(R_AX_SS_CTRL);
	val32 = SET_CLR_WORD(val32, info->band_sel, B_AX_SS_DELAY_TX_BAND_SEL);
	MAC_REG_W32(R_AX_SS_CTRL, val32);

	MAC_REG_W32(R_AX_SS_DELAYTX_TO, info->vovi_to_0 |
		    (info->bebk_to_0 << 8) | (info->vovi_to_1 << 16) |
		    (info->vovi_to_1 << 24));
	MAC_REG_W32(R_AX_SS_DELAYTX_LEN_THR, info->vovi_len_0 |
		    (info->bebk_len_0 << 8) | (info->vovi_len_1 << 16) |
		    (info->vovi_len_1 << 24));
}

void hv_ss_dlmu_search_mode(struct mac_ax_adapter *adapter, u8 mode,
			    u8 score_thr)
{
#if MAC_AX_8852A_SUPPORT || MAC_AX_8852B_SUPPORT || MAC_AX_8851B_SUPPORT
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8851B)) {
		MAC_REG_W8(R_AX_SS_MU_CTRL, (mode ? B_AX_SS_MU_OPT : 0) |
			   SET_WORD(score_thr, B_AX_SS_SCORE_THR));
	}
#endif
}

u32 hv_ss_quota_mode(struct mac_ax_adapter *adapter,
		     struct hv_ax_ss_quota_mode_ctrl *ctrl,
		     enum hv_ax_ss_quota_mode_cfg cfg)
{
	u32 ret;

	switch (cfg) {
	case HV_AX_SS_QUOTA_MODE_GET:
		get_ss_quota_mode(adapter, ctrl);
		break;
	case HV_AX_SS_QUOTA_MODE_SET:
		ret = set_ss_quota_mode(adapter, ctrl);
		if (ret != MACSUCCESS)
			return ret;
		break;
	}

	return MACSUCCESS;
}

void hv_ss_wmm_tbl_cfg(struct mac_ax_adapter *adapter,
		       struct mac_ax_ss_wmm_tbl_ctrl *ctrl,
		       enum hv_ax_ss_wmm_tbl_cfg cfg)
{
	set_ss_wmm_tbl(adapter, ctrl);
}

u32 hv_ss_wmm_sta_move(struct mac_ax_adapter *adapter,
		       enum hv_ax_ss_wmm src_wmm,
		       enum mac_ax_ss_wmm_tbl dst_link)
{
	struct mac_ax_ss_wmm_tbl_ctrl ctrl;
	u32 ret;
	struct hv_ax_ops *hv_ops = adapter->hv_ops;
	struct mac_ax_ops *mac_ops = adapter_to_mac_ops(adapter);

	switch (src_wmm) {
	case HV_AX_SS_WMM0:
		ctrl.wmm = 0;
		break;
	case HV_AX_SS_WMM1:
		ctrl.wmm = 1;
		break;
	case HV_AX_SS_WMM2:
		ctrl.wmm = 2;
		break;
	case HV_AX_SS_WMM3:
		ctrl.wmm = 3;
		break;
	default:
		return MACNOITEM;
	}
	ret = get_ss_wmm_tbl(adapter, &ctrl);
	if (ret != MACSUCCESS)
		return ret;

	ret = switch_wmm_link(adapter, ctrl.wmm_mapping, dst_link, src_wmm);
	if (ret != MACSUCCESS)
		return ret;
	ctrl.wmm_mapping = dst_link;
	hv_ops->ss_wmm_tbl_cfg(adapter, &ctrl, HV_AX_SS_WMM_TBL_SET);

	return MACSUCCESS;
}

u32 hv_ss_set_wmm_bmp(struct mac_ax_adapter *adapter, u8 wmm, u8 macid)
{
	u32 bmp;
	u32 ret;
	struct hv_ax_ops *hv_ops = adapter->hv_ops;
	struct hv_ax_sta_bmp_ctrl ctrl;

	ctrl.macid = macid;
	ctrl.mask = BIT(BMP_WMM0_SH) | BIT(BMP_WMM1_SH) | BIT(BMP_WMM2_SH) |
		    BIT(BMP_WMM3_SH);
	ret = hv_ops->sta_bmp_cfg(adapter, &ctrl, HV_AX_STA_BMP_CFG_GET);
	if (ret != MACSUCCESS)
		return ret;

	bmp = ctrl.bmp;
	if (wmm == 0) {
		bmp |= BIT(BMP_WMM0_SH);
		bmp &= ~(BIT(BMP_WMM1_SH));
		bmp &= ~(BIT(BMP_WMM2_SH));
		bmp &= ~(BIT(BMP_WMM3_SH));
	} else if (wmm == 1) {
		bmp &= ~(BIT(BMP_WMM0_SH));
		bmp |= BIT(BMP_WMM1_SH);
		bmp &= ~(BIT(BMP_WMM2_SH));
		bmp &= ~(BIT(BMP_WMM3_SH));
	} else if (wmm == 2) {
		bmp &= ~(BIT(BMP_WMM0_SH));
		bmp &= ~(BIT(BMP_WMM1_SH));
		bmp |= BIT(BMP_WMM2_SH);
		bmp &= ~(BIT(BMP_WMM3_SH));
	} else if (wmm == 3) {
		bmp &= ~(BIT(BMP_WMM0_SH));
		bmp &= ~(BIT(BMP_WMM1_SH));
		bmp &= ~(BIT(BMP_WMM2_SH));
		bmp |= (BIT(BMP_WMM3_SH));
	}

	ctrl.bmp = bmp;
	ret = hv_ops->sta_bmp_cfg(adapter, &ctrl, HV_AX_STA_BMP_CFG_SET);
	if (ret != MACSUCCESS)
		return ret;
	return MACSUCCESS;
}
