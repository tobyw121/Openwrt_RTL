/******************************************************************************
 *
 * Copyright(c) 2007 - 2020  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/
#include "../halbb_precomp.h"

#ifdef BB_8192XB_SUPPORT

bool halbb_chk_pkg_valid_8192xb(struct bb_info *bb, u8 bb_ver, u8 rf_ver)
{
	bool valid = true;

#if 0
	if (bb_ver >= X && rf_ver >= Y)
		valid = true;
	else if (bb_ver < X && rf_ver < Y)
		valid = true;
	else
		valid = false;
#endif

	if (!valid) {
		/*halbb_set_reg(bb, 0x1c3c, (BIT(0) | BIT(1)), 0x0);*/
		BB_WARNING("[%s] Pkg_ver{bb, rf}={%d, %d} disable all BB block\n",
			 __func__, bb_ver, rf_ver);
	}

	return valid;
}

bool halbb_chk_tx_idle_8192xb(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
	u8 tx_state = 0;
	bool idle = 0;

	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);

	if (phy_idx == HW_PHY_1)
		return false;

	halbb_set_reg(bb, 0x20f8, BIT(31), 1);

	if (phy_idx == HW_PHY_0)
		halbb_set_reg(bb, 0x20f0, MASKDWORD, 0x30002);

	tx_state = (u8)halbb_get_reg(bb, 0x1730, 0x3f);

	if (tx_state == 0)
		idle = true;
	else
		idle = false;

	return idle;
}

void halbb_stop_pmac_tx_8192xb(struct bb_info *bb,
			      struct halbb_pmac_info *tx_info,
			      enum phl_phy_idx phy_idx)
{
	if (tx_info->is_cck) { // CCK
		if (tx_info->mode == CONT_TX) {
			halbb_set_reg(bb, 0x231c, BIT(1), 1);
			halbb_set_reg(bb, 0x2318, BIT(8), 0);
			halbb_set_reg(bb, 0x231c, BIT(21), 0);
			halbb_set_reg(bb, 0x231c, BIT(1), 0);
		} else if (tx_info->mode == PKTS_TX) {
			halbb_set_reg(bb, 0x9c4, BIT(4), 0);
		} else if (tx_info->mode == CCK_CARRIER_SIPPRESSION_TX) {
			halbb_set_reg(bb, 0x9c4, BIT(4), 0);
			/*Carrier Suppress Tx*/
			halbb_set_reg(bb, 0x2318, BIT(9), 0);
			/*Enable scrambler at payload part*/
			halbb_set_reg(bb, 0x231c, BIT(7), 0);
		}
	} else { // OFDM
		if (tx_info->mode == CONT_TX)
			halbb_set_reg(bb, 0x9c4, BIT(0), 0);
		else if (tx_info->mode == PKTS_TX)
			halbb_set_reg(bb, 0x9c4, BIT(4), 0);
	}
}

void halbb_start_pmac_tx_8192xb(struct bb_info *bb,
			       struct halbb_pmac_info *tx_info,
			       enum halbb_pmac_mode mode, u32 pkt_cnt,u16 period,
			       enum phl_phy_idx phy_idx)
{
	if (mode == CONT_TX) {
		if (tx_info->is_cck) {
			halbb_set_reg(bb, 0x2318, BIT(8), 1);
			halbb_set_reg(bb, 0x231c, BIT(21), 0);
		} else {
			halbb_set_reg(bb, 0x9c4, BIT(0), 1);
		}
	} else if (mode == PKTS_TX) {
		/*Tx_N_PACKET_EN */
		halbb_set_reg(bb, 0x9c4, BIT(4), 1);
		/*Tx_N_PERIOD */
		halbb_set_reg(bb, 0x9c4, 0xffffff00, period);
		/*Tx_N_PACKET */
		halbb_set_reg(bb, 0x9c8, 0xffffffff, pkt_cnt);
	} else if (mode == CCK_CARRIER_SIPPRESSION_TX) {
		if (tx_info->is_cck) {
			/*Carrier Suppress Tx*/
			halbb_set_reg(bb, 0x2318, BIT(9), 1);
			/*Disable scrambler at payload part*/
			halbb_set_reg(bb, 0x231c, BIT(7), 1);
		} else {
			return;
		}
		/*Tx_N_PACKET_EN */
		halbb_set_reg(bb, 0x9c4, BIT(4), 1);
		/*Tx_N_PERIOD */
		halbb_set_reg(bb, 0x9c4, 0xffffff00, period);
		/*Tx_N_PACKET */
		halbb_set_reg(bb, 0x9c8, 0xffffffff, pkt_cnt);
	}
	/*Tx_EN */
	halbb_set_reg(bb, 0x9c0, BIT(0), 1);
	halbb_set_reg(bb, 0x9c0, BIT(0), 0);
}

void halbb_set_pmac_tx_8192xb(struct bb_info *bb, struct halbb_pmac_info *tx_info,
			     enum phl_phy_idx phy_idx)
{
	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);

	if (!tx_info->en_pmac_tx) {
		halbb_stop_pmac_tx_8192xb(bb, tx_info, phy_idx);
		/* PD hit enable */
		halbb_set_reg(bb, 0xc3c, BIT(9), 0);
		if (bb->hal_com->band[phy_idx].cur_chandef.band == BAND_ON_24G)
			halbb_set_reg(bb, 0x2320, BIT(0), 0);
		return;
	}
	/*Turn on PMAC */
	/* Tx */
	halbb_set_reg(bb, 0x0980, BIT(0), 1);
	/* Rx */
	halbb_set_reg(bb, 0x0980, BIT(16), 1);
	halbb_set_reg(bb, 0x0988, 0x3f, 0x3f);

	/* PD hit enable */
	halbb_set_reg(bb, 0x704, BIT(1), 0);
	halbb_set_reg(bb, 0xc3c, BIT(9), 1);
	if (tx_info->cck_lbk_en)
		halbb_set_reg(bb, 0x2320, BIT(0), 0);
	else
		halbb_set_reg(bb, 0x2320, BIT(0), 1);
	halbb_set_reg(bb, 0x704, BIT(1), 1);

	halbb_start_pmac_tx_8192xb(bb, tx_info, tx_info->mode, tx_info->tx_cnt,
		       tx_info->period, phy_idx);

	BB_DBG(bb, DBG_PHY_CONFIG, "[PMAC Tx] cck_lbk_en=%d\n", tx_info->cck_lbk_en);
}

void halbb_set_tmac_tx_8192xb(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
	/* To do: 0x0d80[16] [25] / 0x0d88[5:0] Should be set to default value in parameter package*/
	/* Turn on TMAC */
	halbb_set_reg(bb, 0x0980, BIT(0), 0);
	halbb_set_reg(bb, 0x0980, BIT(16), 0);
	halbb_set_reg(bb, 0x0988, 0xfff, 0);
	halbb_set_reg(bb, 0x0994, 0xf0, 0);
	// PDP bypass from TMAC
	halbb_set_reg(bb, 0x09a4, BIT(10), 0);
	// TMAC Tx path
	halbb_set_reg(bb, 0x09a4, 0x1c, 0);
	// TMAC Tx power
	halbb_set_reg(bb, 0x09a4, BIT(16), 0);
	// TMAC Tx OFDM triangular shaping filter
	halbb_set_reg(bb, 0x09a4, BIT(31), 0);
}

void halbb_dpd_bypass_8192xb(struct bb_info *bb, bool pdp_bypass,
			      enum phl_phy_idx phy_idx)
{
	halbb_set_reg(bb, 0x09a4, BIT(10), 1);
	halbb_set_reg(bb, 0x45b8, BIT(16), pdp_bypass);
}

void halbb_ic_hw_setting_init_8192xb(struct bb_info *bb)
{
	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);
	// T2R time = 0xf*0.4us + 0x9*0.4us = 9.6us
	halbb_set_reg(bb, 0x0624, 0x3f, 0xf);
	halbb_set_reg(bb, 0x0608, 0x3f00, 0x9);

	bb->bb_cmn_backup_i.cck_ps_th_bk = (s32)halbb_get_reg(bb, 0x4b48, MASKBYTE0);
	bb->bb_cmn_backup_i.cck_rssi_ofst_bk = (s32)halbb_get_reg(bb, 0x4b48, MASKBYTE2);
	bb->bb_cmn_backup_i.cck_sbd_th_bk = (s32)halbb_get_reg(bb, 0x4ba4, MASKBYTE0);
	halbb_phy_efuse_get_info(bb, FT_EFUSE_ADDR_8192XB, 1, &(bb->bb_efuse_i.efuse_ft));
	halbb_phy_efuse_get_info(bb, TD_HIDE_EFUSE_ADDR_8192XB, 1, &(bb->bb_efuse_i.efuse_adc_td));
}

void halbb_ic_hw_setting_8192xb(struct bb_info *bb)
{
	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);
}

bool halbb_set_pd_lower_bound_8192xb(struct bb_info *bb, u8 bound,
				      enum channel_width bw,
				      enum phl_phy_idx phy_idx)
{
	/*
	Range of bound value:
	BW20: 95~33
	BW40: 92~30
	BW80: 89~27
	*/
	u8 bw_attenuation = 0;
	u8 subband_filter_atteniation = 7;
	u8 bound_idx = 0;
	bool rpt = true;

	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);

	if (bound == 0) {
		halbb_set_reg_cmn(bb, 0x481C, BIT(29), 0, phy_idx);
		BB_DBG(bb, DBG_PHY_CONFIG,
		       "[PD Bound] Set Boundary to default!\n");
		return true;
	}

	bb->bb_cmn_backup_i.cur_pd_lower_bound = bound;

	if (bw == CHANNEL_WIDTH_20) {
		bw_attenuation = 0;
	} else if (bw == CHANNEL_WIDTH_40) {
		bw_attenuation = 3;
	} else if (bw == CHANNEL_WIDTH_80) {
		bw_attenuation = 6;
	} else if (bw == CHANNEL_WIDTH_160) {
		bw_attenuation = 9;
	} else {
		BB_DBG(bb, DBG_PHY_CONFIG,
		       "[PD Bound] Only support BW20/40/80/160 !\n");
		return false;
	}

	bound += (bw_attenuation + subband_filter_atteniation);
	// If Boundary dbm is odd, set it to even number
	bound = bound % 2 ? bound + 1 : bound;

	if (bound < 40) {
		BB_DBG(bb, DBG_PHY_CONFIG,
		       "[PD Bound] Threshold too high, set to highest level!\n");
		bound = 40;
		rpt = false;
	}

	if (bound > 102) {
		BB_DBG(bb, DBG_PHY_CONFIG,
		       "[PD Bound] Threshold too low, disable PD lower bound function!\n");
		halbb_set_reg_cmn(bb, 0x481C, BIT(29), 0, phy_idx);
		return true;
	}

	bound_idx =  (102 - bound) >> 1;

	halbb_set_reg_cmn(bb, 0x481C, 0x7c0, bound_idx, phy_idx);
	halbb_set_reg_cmn(bb, 0x481C, BIT(29), 1, phy_idx);

	BB_DBG(bb, DBG_PHY_CONFIG, "[PD Bound] Set Boundary Success!\n");

	return rpt;
}

bool halbb_set_pd_lower_bound_cck_8192xb(struct bb_info *bb, u8 bound,
				      enum channel_width bw,
				      enum phl_phy_idx phy_idx)
{
	u8 bw_attenuation = 0;
	u8 subband_filter_atteniation = 5;
	s8 bound_tmp = 0;

	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);

	if (bound == 0) {
		halbb_set_reg(bb, 0x4b74, BIT(30), 0);
		BB_DBG(bb, DBG_PHY_CONFIG,
			"[PD Bound] Set Boundary to default!\n");
		return true;
	}

	if (bw == CHANNEL_WIDTH_20) {
		bw_attenuation = 0;
	} else if (bw == CHANNEL_WIDTH_40) {
		bw_attenuation = 3;
	} else if (bw == CHANNEL_WIDTH_80) {
		bw_attenuation = 6;
	} else if (bw == CHANNEL_WIDTH_160) {
		bw_attenuation = 9;
	} else {
		BB_DBG(bb, DBG_PHY_CONFIG,
			"[PD Bound] Only support BW20/40/80/160 !\n");
		return false;
	}

	bound += (bw_attenuation + subband_filter_atteniation);
	bound_tmp = (-1) * MIN_2(bound, 128);

	halbb_set_reg(bb, 0x4b64, 0xff000000, bound_tmp);
	halbb_set_reg(bb, 0x4b64, 0xff0000, 0x7f);
	halbb_set_reg(bb, 0x4b74, BIT(30), 1);

	BB_DBG(bb, DBG_PHY_CONFIG, "[PD Bound] Set CCK Boundary Success!\n");

	return true;
}

u8 halbb_querry_pd_lower_bound_8192xb(struct bb_info *bb, bool get_en_info, enum phl_phy_idx phy_idx)
{
	u8 tmp;

	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);

	if (get_en_info)
		tmp = (u8)halbb_get_reg(bb, 0x481C, BIT(29));
	else
		tmp = bb->bb_cmn_backup_i.cur_pd_lower_bound;

	return tmp;
}

void halbb_pop_en_8192xb(struct bb_info *bb, bool en, enum phl_phy_idx phy_idx)
{
	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);

	if (en) {
		halbb_set_reg(bb, 0x4798, BIT(8), 1);

		halbb_set_reg(bb, 0xc54, 0x6, 0);
		halbb_set_reg(bb, 0xc68, 0xff, 0x50);
		halbb_set_reg(bb, 0x4794, 0xF8000, 0xc);
		halbb_set_reg(bb, 0x4794, 0x1F00000, 8);
		halbb_set_reg(bb, 0x4798, BIT(9), 0);
	} else {
		halbb_set_reg(bb, 0x4798, BIT(8), 0);
	}
}

bool halbb_querry_pop_en_8192xb(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
	bool en;

	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);

	en = (bool)halbb_get_reg(bb, 0x4798, BIT(8));

	return en;
}

u16 halbb_get_per_8192xb(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
	u16 per;
	u32 crc_ok, crc_err, brk_cnt;
	u32 numer, denomer;

	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);

	crc_ok = halbb_mp_get_rx_crc_ok(bb, phy_idx);
	crc_err = halbb_mp_get_rx_crc_err(bb, phy_idx);
	brk_cnt = halbb_get_reg(bb, 0x1a08, MASKHWORD);

	if ((crc_ok > 0xffff) || (crc_err > 0xffff)) {
		BB_DBG(bb, DBG_DBG_API, "[PER] Value > Brk cnt upper bound!\n");
		return 0xffff;
	}

	denomer = crc_ok + crc_err + brk_cnt;
	numer = crc_err + brk_cnt + (denomer >> 1);
	per = (u16)HALBB_DIV(numer * 100, denomer);

	return per;
}

u8 halbb_get_losel_8192xb(struct bb_info *bb)
{
	u8 tmp = 0;

	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);

	tmp = (u8)halbb_get_reg(bb, 0x35c, BIT(11) | BIT(10));
	return tmp;
}

#endif
