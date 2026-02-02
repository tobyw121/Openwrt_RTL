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
#include "cmac_misc_hv.h"
#include "../mac_ax/trxcfg.h"

static u32 h2c_ofld_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_ofld_cfg *param);
static u32 get_muedca_timer_addr(struct mac_ax_adapter *adapter,
				 struct mac_ax_muedca_timer *timer,
				 u32 *reg_timer);

u32 hv_set_freerun_cfg(struct mac_ax_adapter *adapter,
		       enum hv_ax_freerun_cfg cfg)
{
	u16 val16;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret;

	ret = check_mac_en(adapter, MAC_AX_BAND_0, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	val16 = MAC_REG_R16(R_AX_MISC_0);
	switch (cfg) {
	case HV_AX_FREERUN_EN:
		MAC_REG_W16(R_AX_MISC_0, val16 | B_AX_EN_FREERUN);
		break;
	case HV_AX_FREERUN_DIS:
		MAC_REG_W16(R_AX_MISC_0, val16 & ~B_AX_EN_FREERUN);
		break;
	case HV_AX_FREERUN_RST:
		MAC_REG_W16(R_AX_MISC_0, val16 | B_AX_RST_FREERUN_P);
		break;
	default:
		return MACNOITEM;
	}

	return MACSUCCESS;
}

u32 hv_get_freerun_info(struct mac_ax_adapter *adapter, u32 *cnt_low,
			u32 *cnt_high)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 ret;

	ret = check_mac_en(adapter, MAC_AX_BAND_0, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	*cnt_low = MAC_REG_R32(R_AX_FREERUN_CNT_LOW);
	*cnt_high = MAC_REG_R32(R_AX_FREERUN_CNT_HIGH);

	return MACSUCCESS;
}

u32 hv_set_lifetime_mg2(struct mac_ax_adapter *adapter,
			struct hv_ax_lifetime_mg2_cfg *cfg)
{
	u32 ret;
	u8 band;
	u16 val16;
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	band = cfg->band;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	val32 = MAC_REG_R32(band ? R_AX_LIFETIME_2_C1 : R_AX_LIFETIME_2);
	val32 = SET_CLR_WORD(val32, cfg->val, B_AX_CPUMGQ_LIFETIME);
	MAC_REG_W32(band ? R_AX_LIFETIME_2_C1 : R_AX_LIFETIME_2, val32);

	val16 = MAC_REG_R16(band ? R_AX_PTCL_COMMON_SETTING_0_C1 :
			    R_AX_PTCL_COMMON_SETTING_0);
	val16 &= ~B_AX_CPUMGQ_LIFETIME_EN;
	val16 |= cfg->en ? B_AX_CPUMGQ_LIFETIME_EN : 0;
	MAC_REG_W16(band ? R_AX_PTCL_COMMON_SETTING_0_C1 :
		    R_AX_PTCL_COMMON_SETTING_0, val16);

	return MACSUCCESS;
}

u32 hv_get_lifetime_mg2(struct mac_ax_adapter *adapter,
			struct hv_ax_lifetime_mg2_cfg *cfg)
{
	u32 ret;
	u8 band;
	u16 val16;
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	band = cfg->band;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	val16 = MAC_REG_R16(band ? R_AX_PTCL_COMMON_SETTING_0_C1 :
			    R_AX_PTCL_COMMON_SETTING_0);
	cfg->en = (val16 & B_AX_CPUMGQ_LIFETIME_EN) ? 1 : 0;

	val32 = MAC_REG_R32(band ? R_AX_LIFETIME_2_C1 : R_AX_LIFETIME_2);
	cfg->val = GET_FIELD(val32, B_AX_CPUMGQ_LIFETIME);

	return MACSUCCESS;
}

u32 hv_get_ampdu_cfg(struct mac_ax_adapter *adapter,
		     struct mac_ax_ampdu_cfg *cfg)
{
	u32 val32;
	u8 val8;
	u8 band;
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	band = cfg->band;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	val8 = MAC_REG_R8(band ? R_AX_AGG_BK_0_C1 : R_AX_AGG_BK_0);
	cfg->wdbk_mode = val8 & B_AX_WDBK_CFG ?
			 MAC_AX_WDBK_MODE_GRP_BK : MAC_AX_WDBK_MODE_SINGLE_BK;

	switch (val8 & (B_AX_EN_RTY_BK | B_AX_EN_RTY_BK_COD)) {
	case 0x0:
		cfg->rty_bk_mode = MAC_AX_RTY_BK_MODE_AGG;
		break;
	case 0x1:
		cfg->rty_bk_mode = MAC_AX_RTY_BK_MODE_RATE_FB;
		break;
	case 0x2:
	case 0x3:
		cfg->rty_bk_mode = MAC_AX_RTY_BK_MODE_BK;
		break;
	}

	val32 = MAC_REG_R32(band ?
			    R_AX_AMPDU_AGG_LIMIT_C1 : R_AX_AMPDU_AGG_LIMIT);
	cfg->max_agg_num = GET_FIELD(val32, B_AX_MAX_AGG_NUM) + 1;
	cfg->max_agg_time_32us = GET_FIELD(val32, B_AX_AMPDU_MAX_TIME);

	return MACSUCCESS;
}

u32 hv_get_edca_param(struct mac_ax_adapter *adapter,
		      struct mac_ax_edca_param *param)
{
	u32 val32;
	u32 reg_edca;
	u32 ret;
	u16 val16;
	enum mac_ax_cmac_path_sel path;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, param->band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	ret = get_edca_addr(adapter, param, &reg_edca);
	if (ret != MACSUCCESS)
		return ret;

	path = param->path;

	if (path == MAC_AX_CMAC_PATH_SEL_MG0_1 ||
	    path == MAC_AX_CMAC_PATH_SEL_MG2 ||
	    path == MAC_AX_CMAC_PATH_SEL_BCN) {
		val16 = MAC_REG_R16(reg_edca);
		param->txop_32us = 0;
		param->aifs_us = GET_FIELD(val16, B_AX_BE_0_AIFS);
		param->ecw_max = (GET_FIELD(val16, B_AX_BE_0_CW) & 0xF0) >> 4;
		param->ecw_min = GET_FIELD(val16, B_AX_BE_0_CW) & 0x0F;
	} else {
		val32 = MAC_REG_R32(reg_edca);
		param->txop_32us = GET_FIELD(val32, B_AX_BE_0_TXOPLMT);
		param->aifs_us = GET_FIELD(val32, B_AX_BE_0_AIFS);
		param->ecw_max = (GET_FIELD(val32, B_AX_BE_0_CW) & 0xF0) >> 4;
		param->ecw_min = GET_FIELD(val32, B_AX_BE_0_CW) & 0x0F;
	}

	return MACSUCCESS;
}

u32 hv_get_muedca_param(struct mac_ax_adapter *adapter,
			struct mac_ax_muedca_param *param)
{
	u32 val32;
	u32 reg_edca;
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, param->band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	ret = get_muedca_param_addr(adapter, param, &reg_edca);
	if (ret != MACSUCCESS)
		return ret;

	val32 = MAC_REG_R32(reg_edca);

	param->muedca_timer_32us =
				GET_FIELD(val32, B_AX_MUEDCA_BE_PARAM_0_TIMER);
	param->aifs_us = GET_FIELD(val32, B_AX_MUEDCA_BE_PARAM_0_AIFS);
	param->ecw_max =
		(GET_FIELD(val32, B_AX_MUEDCA_BE_PARAM_0_CW) & 0xF0) >> 4;
	param->ecw_min = GET_FIELD(val32, B_AX_MUEDCA_BE_PARAM_0_CW) & 0x0F;

	return MACSUCCESS;
}

u32 hv_get_muedca_timer(struct mac_ax_adapter *adapter,
			struct mac_ax_muedca_timer *timer)
{
	u32 reg_timer;
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, timer->band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	ret = get_muedca_timer_addr(adapter, timer, &reg_timer);
	if (ret != MACSUCCESS)
		return ret;

	timer->muedca_timer_32us = MAC_REG_R16(reg_timer);

	return MACSUCCESS;
}

u32 hv_get_muedca_ctrl(struct mac_ax_adapter *adapter,
		       struct mac_ax_muedca_cfg *cfg)
{
	u32 ret;
	u8 band;
	u16 val16;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	band = cfg->band;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	val16 = MAC_REG_R16(band ? R_AX_MUEDCA_EN_C1 : R_AX_MUEDCA_EN);

	if (val16 & B_AX_MUEDCA_WMM_SEL)
		cfg->wmm_sel = MAC_AX_CMAC_WMM1_SEL;
	else
		cfg->wmm_sel = MAC_AX_CMAC_WMM0_SEL;

	if (val16 & B_AX_MUEDCA_EN_0)
		cfg->countdown_en = 1;
	else
		cfg->countdown_en = 0;

	if (val16 & B_AX_SET_MUEDCATIMER_TF_0)
		cfg->tb_update_en = 1;
	else
		cfg->tb_update_en = 0;

	return MACSUCCESS;
}

u32 hv_get_ch_stat_cnt(struct mac_ax_adapter *adapter,
		       struct mac_ax_ch_stat_cnt *cnt)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 band;
	u32 ret;

	band = cnt->band;
	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	cnt->busy_cnt = MAC_REG_R32(band ? R_AX_CHNL_BUSY_TIME_0_C1 :
				    R_AX_CHNL_BUSY_TIME_0);
	cnt->idle_cnt = MAC_REG_R32(band ? R_AX_CHNL_IDLE_TIME_0_C1 :
				    R_AX_CHNL_IDLE_TIME_0);

	return MACSUCCESS;
}

u32 hv_get_lifetime_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_lifetime_cfg *cfg)
{
	u32 ret;
	u8 band;
	u8 val8;
	u32 val32;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	band = cfg->band;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	val8 = MAC_REG_R8(band ? R_AX_PTCL_COMMON_SETTING_0_C1 :
			  R_AX_PTCL_COMMON_SETTING_0);
	cfg->en.acq_en = (val8 & B_AX_LIFETIME_EN) ? 1 : 0;
	cfg->en.mgq_en = (val8 & B_AX_MGQ_LIFETIME_EN) ? 1 : 0;

	val32 = MAC_REG_R32(band ? R_AX_LIFETIME_0_C1 : R_AX_LIFETIME_0);
	cfg->val.acq_val_1 = GET_FIELD(val32, B_AX_PKT_LIFETIME_1);
	cfg->val.acq_val_2 = GET_FIELD(val32, B_AX_PKT_LIFETIME_2);

	val32 = MAC_REG_R32(band ? R_AX_LIFETIME_1_C1 : R_AX_LIFETIME_1);
	cfg->val.acq_val_3 = GET_FIELD(val32, B_AX_PKT_LIFETIME_3);
	cfg->val.acq_val_4 = GET_FIELD(val32, B_AX_PKT_LIFETIME_4);

	cfg->val.mgq_val = MAC_REG_R16(band ? R_AX_LIFETIME_2_C1 :
				       R_AX_LIFETIME_2);

	return MACSUCCESS;
}

u32 hv_get_hw_edcca_param(struct mac_ax_adapter *adapter,
			  struct mac_ax_edcca_param *param)
{
	u32 reg_cca_ctl;
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, param->band, MAC_AX_CMAC_SEL);
	if (ret)
		return ret;

	if (param->band)
		reg_cca_ctl = MAC_REG_R32(R_AX_CCA_CONTROL_C1);
	else
		reg_cca_ctl = MAC_REG_R32(R_AX_CCA_CONTROL);

	param->tb_check_en = (reg_cca_ctl & B_AX_TB_CHK_EDCCA) ? 1 : 0;
	param->sifs_check_en = (reg_cca_ctl & B_AX_SIFS_CHK_EDCCA) ? 1 : 0;
	param->ctn_check_en = (reg_cca_ctl & B_AX_CTN_CHK_EDCCA) ? 1 : 0;

	return MACSUCCESS;
}

u32 hv_set_ofld_cfg(struct mac_ax_adapter *adapter,
		    struct mac_ax_ofld_cfg *param)
{
	u32 ret;

	ret = h2c_ofld_cfg(adapter, param);

	if (ret != MACSUCCESS)
		return ret;

	return MACSUCCESS;
}

u32 hv_get_macid_pause(struct mac_ax_adapter *adapter,
		       struct mac_ax_macid_pause_cfg *cfg)
{
	u32 ret;

	ret = get_macid_pause(adapter, cfg);
	if (ret != MACSUCCESS)
		return ret;

	return MACSUCCESS;
}

u32 hv_get_hw_sch_tx_en(struct mac_ax_adapter *adapter,
			struct mac_ax_sch_tx_en_cfg *cfg)
{
	u32 ret;

	ret = get_hw_sch_tx_en(adapter, cfg);
	if (ret != MACSUCCESS)
		return ret;

	return MACSUCCESS;
}

u32 hv_set_hw_muedca_timer(struct mac_ax_adapter *adapter,
			   struct mac_ax_muedca_timer *timer)
{
	u32 reg_timer;
	u32 ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	ret = check_mac_en(adapter, timer->band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	ret = get_muedca_timer_addr(adapter, timer, &reg_timer);
	if (ret != MACSUCCESS)
		return ret;

	MAC_REG_W16(reg_timer, timer->muedca_timer_32us);

	return MACSUCCESS;
}

u32 hv_set_hw_ch_busy_cnt(struct mac_ax_adapter *adapter,
			  struct mac_ax_ch_busy_cnt_cfg *cfg)
{
	u8 band;
	u32 ret;
	u32 reg_addr;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;

	band = cfg->band;
	reg_addr = (band ? R_AX_PTCL_ATM_C1 : R_AX_PTCL_ATM);

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	switch (cfg->cnt_ctrl) {
	case MAC_AX_CH_BUSY_CNT_CTRL_CNT_EN:
		MAC_REG_W32(reg_addr,
			    MAC_REG_R32(reg_addr) | B_AX_CHNL_INFO_EN);
		break;
	case MAC_AX_CH_BUSY_CNT_CTRL_CNT_DIS:
		MAC_REG_W32(reg_addr,
			    MAC_REG_R32(reg_addr) & ~B_AX_CHNL_INFO_EN);
		break;
	case MAC_AX_CH_BUSY_CNT_CTRL_CNT_BUSY_RST:
		val32 = MAC_REG_R32(reg_addr);
		MAC_REG_W32(reg_addr, val32 | B_AX_RST_CHNL_BUSY);
		MAC_REG_W32(reg_addr, val32 & ~B_AX_RST_CHNL_BUSY);
		break;
	case MAC_AX_CH_BUSY_CNT_CTRL_CNT_IDLE_RST:
		val32 = MAC_REG_R32(reg_addr);
		MAC_REG_W32(reg_addr, val32 | B_AX_RST_CHNL_IDLE);
		MAC_REG_W32(reg_addr, val32 & ~B_AX_RST_CHNL_IDLE);
		break;
	case MAC_AX_CH_BUSY_CNT_CTRL_CNT_REF:
		val32 = MAC_REG_R32(reg_addr) &
			~(B_AX_CHNL_REF_RX_BASIC_NAV |
			  B_AX_CHNL_REF_RX_INTRA_NAV |
			  B_AX_CHNL_REF_DATA_ON |
			  B_AX_CHNL_REF_EDCCA_P20 |
			  B_AX_CHNL_REF_CCA_P20 |
			  B_AX_CHNL_REF_CCA_S20 |
			  B_AX_CHNL_REF_CCA_S40 |
			  B_AX_CHNL_REF_CCA_S80);
		val32 |= (cfg->ref.basic_nav ? B_AX_CHNL_REF_RX_BASIC_NAV : 0) |
			(cfg->ref.intra_nav ? B_AX_CHNL_REF_RX_INTRA_NAV : 0) |
			(cfg->ref.data_on ? B_AX_CHNL_REF_DATA_ON : 0) |
			(cfg->ref.edcca_p20 ? B_AX_CHNL_REF_EDCCA_P20 : 0) |
			(cfg->ref.cca_p20 ? B_AX_CHNL_REF_CCA_P20 : 0) |
			(cfg->ref.cca_s20 ? B_AX_CHNL_REF_CCA_S20 : 0) |
			(cfg->ref.cca_s40 ? B_AX_CHNL_REF_CCA_S40 : 0) |
			(cfg->ref.cca_s80 ? B_AX_CHNL_REF_CCA_S80 : 0);
		MAC_REG_W32(reg_addr, val32);
		break;
	default:
		return MACNOITEM;
	}

	return MACSUCCESS;
}

static u32 h2c_ofld_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_ofld_cfg *param)
{
	u8 *buf;
#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
#else
	struct h2c_buf *h2cb;
#endif
	struct fwcmd_ofld_cfg *fwcmd_tbl;
	u32 ret;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY) {
		PLTFM_MSG_WARN("%s fw not ready\n", __func__);
		return MACFWNONRDY;
	}

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_CMD);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, sizeof(struct fwcmd_ofld_cfg));
	if (!buf) {
		h2cb_free(adapter, h2cb);
		return MACNOBUF;
	}

	fwcmd_tbl = (struct fwcmd_ofld_cfg *)buf;
	fwcmd_tbl->dword0 =
	cpu_to_le32(SET_WORD(param->mode, FWCMD_H2C_OFLD_CFG_MODE) |
		    (param->usr_txop_be ? FWCMD_H2C_OFLD_CFG_USR_TXOP_BE : 0));
	fwcmd_tbl->dword1 =
	cpu_to_le32(SET_WORD(param->usr_txop_be_val,
			     FWCMD_H2C_OFLD_CFG_USR_TXOP_BE_VAL));

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C,
			      FWCMD_H2C_CAT_MAC,
			      FWCMD_H2C_CL_FW_OFLD,
			      FWCMD_H2C_FUNC_OFLD_CFG,
			      0,
			      0);

	if (ret != MACSUCCESS) {
		h2cb_free(adapter, h2cb);
		return ret;
	}

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret != MACSUCCESS) {
		h2cb_free(adapter, h2cb);
		return ret;
	}

#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
#endif
	if (ret != MACSUCCESS) {
		h2cb_free(adapter, h2cb);
		return ret;
	}

	h2cb_free(adapter, h2cb);
	return MACSUCCESS;
}

static u32 get_muedca_timer_addr(struct mac_ax_adapter *adapter,
				 struct mac_ax_muedca_timer *timer,
				 u32 *reg_timer)
{
	u8 band;
	u32 ret;
	enum mac_ax_cmac_ac_sel ac;

	band = timer->band;
	ac = timer->ac;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	switch (ac) {
	case MAC_AX_CMAC_AC_SEL_BE:
		*reg_timer =
			band ? R_AX_MUEDCATIMER_0_C1 :
			R_AX_MUEDCATIMER_0;
		break;
	case MAC_AX_CMAC_AC_SEL_BK:
		*reg_timer =
			band ? (R_AX_MUEDCATIMER_0_C1 + 2) :
			(R_AX_MUEDCATIMER_0 + 2);
		break;
	case MAC_AX_CMAC_AC_SEL_VI:
		*reg_timer =
			band ? R_AX_MUEDCATIMER_1_C1 :
			R_AX_MUEDCATIMER_1;
		break;
	case MAC_AX_CMAC_AC_SEL_VO:
		*reg_timer =
			band ? (R_AX_MUEDCATIMER_1_C1 + 2) :
			(R_AX_MUEDCATIMER_1 + 2);
		break;
	default:
		return MACNOITEM;
	}

	return MACSUCCESS;
}

