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
#include "../halrf_precomp.h"
#ifdef RF_8852BP_SUPPORT
bool halrf_bw_setting_8852bp(struct rf_info *rf, enum rf_path path, enum channel_width bw, bool is_dav)
{
	u32 rf_reg18 = 0;
	u32 reg_reg18_addr = 0x0;

	RF_DBG(rf, DBG_RF_RFK, "[RFK]===> %s\n", __func__);	
	if(is_dav)
		reg_reg18_addr =0x18;
	else
		reg_reg18_addr = 0x10018;

	rf_reg18 = halrf_rrf(rf, path, reg_reg18_addr, MASKRF);
	/*==== [Error handling] ====*/
	if (rf_reg18 == INVALID_RF_DATA) {		
		RF_DBG(rf, DBG_RF_RFK, "[RFK] Invalid RF_0x18 for Path-%d\n", path);
		return false;
	}
	rf_reg18 &= ~(BIT(11) | BIT(10));
	/*==== [Switch bandwidth] ====*/
	switch (bw) {
		case CHANNEL_WIDTH_5:
		case CHANNEL_WIDTH_10:
		case CHANNEL_WIDTH_20:
			rf_reg18 |= (BIT(11) | BIT(10));
			//halrf_wreg(rf, 0xc0d4 |( path << 8), BIT(27) | BIT(26), 0x3);
			//halrf_wreg(rf, 0xc0d8 |( path << 8), 0x00001e0, 0xf);
			break;
		case CHANNEL_WIDTH_40:
			rf_reg18 |= BIT(11);			
			//halrf_wreg(rf, 0xc0d4 |( path << 8), BIT(27) | BIT(26), 0x3);
			//halrf_wreg(rf, 0xc0d8 |( path << 8), 0x00001e0, 0xf);
			break;
		case CHANNEL_WIDTH_80:
			rf_reg18 |= BIT(10);
			//halrf_wreg(rf, 0xc0d4 |( path << 8), BIT(27) | BIT(26), 0x2);
			//halrf_wreg(rf, 0xc0d8 |( path << 8), 0x00001e0, 0xd);
			break;	
		case CHANNEL_WIDTH_160:
			//halrf_wreg(rf, 0xc0d4 |( path << 8), BIT(27) | BIT(26), 0x1);
			//halrf_wreg(rf, 0xc0d8 |( path << 8), 0x00001e0, 0xb);
			break;
		default:
			RF_DBG(rf, DBG_RF_RFK, "[RFK] Fail to set CH\n");
			break;
	}

	/*==== [Write RF register] ====*/
	halrf_wrf(rf, path, reg_reg18_addr, MASKRF, rf_reg18);
	RF_DBG(rf, DBG_RF_RFK, "[RFK] set %x at path%d, %x =0x%x\n",bw, path, reg_reg18_addr, halrf_rrf(rf, path, reg_reg18_addr, MASKRF));
	return true;
}

bool halrf_ctrl_bw_8852bp(struct rf_info *rf, enum phl_phy_idx phy, enum channel_width bw)
{
	bool is_dav;
	u8 kpath, path;
	u32 tmp = 0;
	//RF_DBG(rf, DBG_RF_RFK, "[RFK]===> %s\n", __func__);

	/*==== Error handling ====*/
	if (bw >= CHANNEL_WIDTH_MAX ) {
		RF_DBG(rf, DBG_RF_RFK,"[RFK]Fail to switch bw(bw:%d)\n", bw);
		return false;
	}

	kpath = halrf_kpath_8852bp(rf, phy);

	for (path = 0; path < 2; path++) {
		if (kpath & BIT(path)) {
			is_dav = true;
			halrf_bw_setting_8852bp(rf, path, bw, is_dav);
			is_dav = false;
			halrf_bw_setting_8852bp(rf, path, bw, is_dav);
			if(!rf->hal_com->dbcc_en) {
				if((path == RF_PATH_B) && (rf->hal_com->acv == CAV)){
					halrf_wrf(rf, RF_PATH_B, 0x05, 0x00001, 0x0);
					tmp = halrf_rrf(rf, RF_PATH_A, 0x18, 0xfffff);
					halrf_wrf(rf, RF_PATH_B, 0x19, 0x00030, 0x3);
					halrf_wrf(rf, RF_PATH_B, 0x18, 0xfffff, tmp);
					halrf_delay_us(rf,100);
					halrf_wrf(rf, RF_PATH_B, 0x5, 0x00001, 0x1);
				}
			}
			RF_DBG(rf, DBG_RF_RFK, "[RFK] BW: %d\n", bw);
			RF_DBG(rf, DBG_RF_RFK, "[RFK] 0x18 = 0x%x\n", halrf_rrf(rf, path, 0x18, MASKRF));
		}
	}

	return true;
}

bool halrf_ch_setting_8852bp(struct rf_info *rf,   enum rf_path path, u8 central_ch, enum band_type band, bool is_dav)
{
	u32 rf_reg18 = 0;	
	u32 reg_reg18_addr = 0x0;

	RF_DBG(rf, DBG_RF_RFK, "[RFK]===> %s\n", __func__);
	
	if(is_dav)
		reg_reg18_addr = 0x18;
	else
		reg_reg18_addr = 0x10018;

	rf_reg18 = halrf_rrf(rf, path, reg_reg18_addr, MASKRF);
	/*==== [Error handling] ====*/
	if (rf_reg18 == INVALID_RF_DATA) {
		RF_DBG(rf, DBG_RF_RFK, "[RFK] Invalid RF_0x18 for Path-%d\n", path);
		return false;
	}
	//is_2g_ch = (band == BAND_ON_24G) ? true : false;
	/*==== [Set RF Reg 0x18] ====*/
	rf_reg18 &= ~0x303ff; /*[17:16],[9:8],[7:0]*/
	rf_reg18 |= central_ch; /* Channel*/
	
	switch(band) {
	case BAND_ON_24G :
		/*==== [2G Setting] ====*/
		RF_DBG(rf, DBG_RF_RFK, "[RFK] 2G Setting\n");
		break;
	case BAND_ON_5G :		
		/*==== [5G Setting] ====*/
		RF_DBG(rf, DBG_RF_RFK, "[RFK] 5G Setting\n");
		rf_reg18 |= (BIT(16) | BIT(8)) ;
		break;	
	case BAND_ON_6G :
		/*==== [6G Setting] ====*/
		RF_DBG(rf, DBG_RF_RFK, "[RFK] 6G Setting\n");
		rf_reg18 |= (BIT(17) | BIT(16));
		break;
	default :
		break;
	}
	
	halrf_wrf(rf, path, reg_reg18_addr, MASKRF, rf_reg18);
	halrf_delay_us(rf, 100);
	//halrf_wrf(rf, path, 0xcf, BIT(0), 0);
	//halrf_wrf(rf, path, 0xcf, BIT(0), 1);
	RF_DBG(rf, DBG_RF_RFK, "[RFK] CH: %d for Path-%d, reg0x%x = 0x%x\n", central_ch, path, reg_reg18_addr, halrf_rrf(rf, path, reg_reg18_addr, MASKRF));
	return true;
}

bool halrf_ctrl_ch_8852bp(struct rf_info *rf,  enum phl_phy_idx phy, u8 central_ch, enum band_type band)
{
	bool is_dav;
	u8 kpath, path;

	//RF_DBG(rf, DBG_RF_RFK, "[RFK]===> %s\n", __func__);

	/*==== Error handling ====*/
	
	if (band != BAND_ON_6G) {
		if ((central_ch > 14 && central_ch < 36) ||
			    (central_ch > 64 && central_ch < 100) ||
			    (central_ch > 144 && central_ch < 149) ||
			central_ch > 177 ) {
			RF_DBG(rf, DBG_RF_RFK, "[RFK] Invalid 2G/5G CH:%d \n", central_ch);
			return false;
		}
	} else {
		if ((central_ch > 253) ||(central_ch == 2)){
			RF_DBG(rf, DBG_RF_RFK, "[RFK] Invalid 6G CH:%d \n", central_ch);
			return false;
		}
	}
	
	kpath = halrf_kpath_8852bp(rf, phy);
	
	for (path = 0; path < 2; path++) {
		if (kpath & BIT(path)) {
			is_dav = true;
			halrf_ch_setting_8852bp(rf, path, central_ch, band, is_dav);
			is_dav = false;
			halrf_ch_setting_8852bp(rf, path, central_ch, band, is_dav);	
		}
	}

	RF_DBG(rf, DBG_RF_RFK, "[RFK] CH: %d\n", central_ch);

	return true;
}

void halrf_set_lo_8852bp(struct rf_info *rf, bool is_on, enum rf_path path)
{
	u32 rf18;
	bool not2g = true;

	rf18 = halrf_rrf(rf, RF_PATH_A, 0x18, MASKRF);
	
	if (((rf18 & (BIT(16) | BIT(8))) == 0) && ((rf18 & (BIT(17) | BIT(16))) == 0))
		not2g = false;

	if (is_on) {
		if (not2g) {
			halrf_rf_direct_cntrl_8852bp(rf, path, false);
			halrf_wrf(rf, path, 0x0, 0x30000, 0x2);
			halrf_wrf(rf, path, 0x11, MASKRF, 0x30);
			halrf_wrf(rf, path, 0x62, BIT(4), 0x1);
		} else {
			halrf_rf_direct_cntrl_8852bp(rf, path, false);
			halrf_wrf(rf, path, 0x0, 0x30000, 0x2);
			halrf_wrf(rf, path, 0x11, MASKRF, 0x60);
			halrf_wrf(rf, path, 0x58, BIT(1), 0x1);
		}
	} else {
		halrf_wrf(rf, path, 0x62, BIT(4), 0x0);
		halrf_wrf(rf, path, 0x58, BIT(1), 0x0);
		halrf_rf_direct_cntrl_8852bp(rf, path, true);
	}
}

void halrf_rf_direct_cntrl_8852bp(struct rf_info *rf, enum rf_path path, bool is_bybb)
{
	if (is_bybb)
		halrf_wrf(rf, path, 0x5, BIT(0), 0x1);
	else
		halrf_wrf(rf, path, 0x5, BIT(0), 0x0);
}

void halrf_drf_direct_cntrl_8852bp(struct rf_info *rf, enum rf_path path, bool is_bybb)
{
	if (is_bybb)
		halrf_wrf(rf, path, 0x10005, BIT(0), 0x1);
	else
		halrf_wrf(rf, path, 0x10005, BIT(0), 0x0);
}


void halrf_lo_test_8852bp(struct rf_info *rf, bool is_on, enum rf_path path)
{
	switch (path) {
		case RF_PATH_A:
			halrf_set_lo_8852bp(rf, is_on, RF_PATH_A);
			halrf_set_lo_8852bp(rf, false, RF_PATH_B);
			break;
		case RF_PATH_B:
			halrf_set_lo_8852bp(rf, false, RF_PATH_A);
			halrf_set_lo_8852bp(rf, is_on, RF_PATH_B);
			break;
		case RF_PATH_AB:
			halrf_set_lo_8852bp(rf, is_on, RF_PATH_A);
			halrf_set_lo_8852bp(rf, is_on, RF_PATH_B);
			break;
		default:
			break;
	}
}

u8 halrf_kpath_8852bp(struct rf_info *rf, enum phl_phy_idx phy_idx) {

	RF_DBG(rf, DBG_RF_RFK, "[RFK]dbcc_en: %x,  PHY%d\n", rf->hal_com->dbcc_en, phy_idx);

	if (!rf->hal_com->dbcc_en) {
		return RF_AB;
	} else {
		if (phy_idx == HW_PHY_0)
			return RF_A;
		else
			return RF_B;
	}
}

void _rx_dck_info_8852bp(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path, bool is_afe)
{
	struct halrf_rx_dck_info *rx_dck = &rf->rx_dck;

	rx_dck->is_afe = is_afe;
	rx_dck->loc[path].cur_band = rf->hal_com->band[phy].cur_chandef.band;
	rx_dck->loc[path].cur_bw = rf->hal_com->band[phy].cur_chandef.bw;
	rx_dck->loc[path].cur_ch = rf->hal_com->band[phy].cur_chandef.center_ch;

	RF_DBG(rf, DBG_RF_RXDCK, "[RX_DCK] ==== S%d RX DCK (%s / CH%d / %s / by %s)====\n", path,
		rx_dck->loc[path].cur_band == 0 ? "2G" :
		(rx_dck->loc[path].cur_band == 1 ? "5G" : "6G"),
		rx_dck->loc[path].cur_ch,
	       rx_dck->loc[path].cur_bw == 0 ? "20M" :
	       (rx_dck->loc[path].cur_bw == 1 ? "40M" : 
	       	(rx_dck->loc[path].cur_bw == 2 ? "80M" : "160M")),
	       	rx_dck->is_afe ? "AFE" : "RFC");	
}

void halrf_rx_dck_toggle_8852bp(struct rf_info *rf, enum rf_path path)
{
	u32 cnt = 0;

	halrf_wrf(rf, path, 0x92, BIT(0), 0x0);
	halrf_wrf(rf, path, 0x92, BIT(0), 0x1);

	while ((halrf_rrf(rf, path, 0x93, BIT(5)) == 0x0) && (cnt < 500)) {
			halrf_delay_us(rf, 2);
		cnt++;
		}

	RF_DBG(rf, DBG_RF_RXDCK, "[RX_DCK] S%d RXDCK finish (cnt = %d)\n",
		path, cnt);

		halrf_wrf(rf, path, 0x92, BIT(0), 0x0);
}

void halrf_set_rx_dck_8852bp(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path, bool is_afe)
{
	struct halrf_rx_dck_info *rx_dck = &rf->rx_dck;

	u8 phy_map, res;

	phy_map = (BIT(phy) << 4) | BIT(path);

	_rx_dck_info_8852bp(rf, phy, path, is_afe);

	halrf_wrf(rf, path, 0x93, 0x0000f, 0x0); /*0: from RFC; 1: from AFE*/
#if 0
	halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_RXDCK, RFK_ONESHOT_START);
#endif
	halrf_rx_dck_toggle_8852bp(rf, path);
#if 0
		halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_RXDCK, RFK_ONESHOT_STOP);
#endif
#if 0
	RF_DBG(rf, DBG_RF_RFK, "[RX_DCK] 0x92 = 0x%x, 0x93 = 0x%x\n",
	       halrf_rrf(rf, path, 0x92, MASKRF),
	       halrf_rrf(rf, path, 0x93, MASKRF));
#endif
	if (rx_dck->is_auto_res) {
		res = (u8)halrf_rrf(rf, path, 0x92, BIT(7) | BIT(6) | BIT(5));
		RF_DBG(rf, DBG_RF_RXDCK, "[RX_DCK] S%d RXDCK resolution = 0x%x\n", path, res);
#if 1
		if (res > 1) {
			halrf_wrf(rf, path, 0x8f, BIT(11) | BIT(10) | BIT(9), res);
			halrf_rx_dck_toggle_8852bp(rf, path);
			halrf_wrf(rf, path, 0x8f, BIT(11) | BIT(10) | BIT(9), 0x1);
		}
#endif
	}
}

void halrf_rx_dck_8852bp(struct rf_info *rf, enum phl_phy_idx phy, bool is_afe) 
{
	struct halrf_rx_dck_info *rx_dck = &rf->rx_dck;

	u8 path, kpath;
	u32 rf_reg5;

	kpath = halrf_kpath_8852bp(rf, phy);

	RF_DBG(rf, DBG_RF_RXDCK, "[RX_DCK] ****** RXDCK Start (Ver: 0x%x, Cv: %d) ******\n",
		RXDCK_VER_8852BP, rf->hal_com->cv);

	for (path = 0; path < 2; path++) {
		rf_reg5 = halrf_rrf(rf, path, 0x5, MASKRF);
		if (kpath & BIT(path)) {
		if (rf->is_tssi_mode[path]) 
			halrf_wreg(rf, 0x5818 + (path << 13), BIT(30), 0x1); /*TSSI pause*/

		halrf_wrf(rf, path, 0x5, BIT(0), 0x0);
		halrf_wrf(rf, path, 0x00, MASKRFMODE, RF_RX);
		halrf_set_rx_dck_8852bp(rf, phy, path, is_afe);
		rx_dck->ther_rxdck[path] = halrf_get_thermal_8852bp(rf, path);
		halrf_wrf(rf, path, 0x5, MASKRF, rf_reg5);

		if (rf->is_tssi_mode[path])
			halrf_wreg(rf, 0x5818 + (path << 13), BIT(30), 0x0); /*TSSI resume*/
		}

	}
}

void halrf_rx_dck_onoff_8852bp(struct rf_info *rf, bool is_enable)
{
	u8 path;

	for (path = 0; path < 2; path++) {
		halrf_wrf(rf, path, 0x93, BIT(0), !is_enable);
		if (!is_enable) {
			halrf_wrf(rf, path, 0x92, 0xFFE00, 0x440); /*[19:9]*/
			halrf_wrf(rf, path, 0x93, 0xFFE00, 0x440); /*[19:9]*/
		}
	}
}

void halrf_rxdck_track_8852bp(
	struct rf_info *rf)
{
	struct halrf_rx_dck_info *rx_dck = &rf->rx_dck;

	RF_DBG(rf, DBG_RF_RXDCK, "[RX_DCK] S0 delta_ther = %d (0x%x / 0x%x@k)\n",
		rf->cur_ther_s0 - rx_dck->ther_rxdck[RF_PATH_A],
		rf->cur_ther_s0, rx_dck->ther_rxdck[RF_PATH_A]);
	RF_DBG(rf, DBG_RF_RXDCK, "[RX_DCK] S1 delta_ther = %d (0x%x / 0x%x@k)\n",
		rf->cur_ther_s1 - rx_dck->ther_rxdck[RF_PATH_B],
		rf->cur_ther_s1, rx_dck->ther_rxdck[RF_PATH_B]);

	if (HALRF_ABS(rf->cur_ther_s0, rx_dck->ther_rxdck[RF_PATH_A]) >= 8 ||
	    HALRF_ABS(rf->cur_ther_s1, rx_dck->ther_rxdck[RF_PATH_B]) >= 8)
			halrf_rx_dck_trigger(rf, HW_PHY_0, false);
}

void halrf_rck_8852bp(struct rf_info *rf, enum rf_path path)
{
	u8 cnt = 0;
	u32 rf_reg5;
	u32 rck_val = 0;

	RF_DBG(rf, DBG_RF_RFK, "[RCK] ====== S%d RCK ======\n", path);

	rf_reg5 = halrf_rrf(rf, path, 0x5, MASKRF);

	halrf_wrf(rf, path, 0x5, BIT(0), 0x0);
	halrf_wrf(rf, path, 0x0, MASKRFMODE, RF_RX);
	
	RF_DBG(rf, DBG_RF_RFK, "[RCK] RF0x00 = 0x%05x\n", halrf_rrf(rf, path, 0x00, MASKRF));

	/*RCK trigger*/
	halrf_wrf(rf, path, 0x1b, MASKRF, 0x00240);

	while ((halrf_rrf(rf, path, 0x1c, BIT(3)) == 0x00) && (cnt < 10)) {
		halrf_delay_us(rf, 2);
		cnt++;
	}

	rck_val = halrf_rrf(rf, path, 0x1b, 0x07C00); /*[14:10]*/

	RF_DBG(rf, DBG_RF_RFK, "[RCK] rck_val = 0x%x, count = %d\n", rck_val, cnt);

	halrf_wrf(rf, path, 0x1b, MASKRF, rck_val);

	halrf_wrf(rf, path, 0x5, MASKRF, rf_reg5);

	RF_DBG(rf, DBG_RF_RFK, "[RCK] RF 0x1b = 0x%x\n",
	       halrf_rrf(rf, path, 0x1b, MASKRF));
}

void iqk_backup_8852bp(struct rf_info *rf, enum rf_path path) 
{
	return;
}

void halrf_bf_config_rf_8852bp(struct rf_info *rf)
{
	u8 i;

	for (i = 0; i < 2; i++) {
		halrf_wrf(rf, (enum rf_path)i, 0xef, BIT(19), 0x1);
		halrf_wrf(rf, (enum rf_path)i, 0x33, 0xf, 0x1);
		halrf_wrf(rf, (enum rf_path)i, 0x3e, MASKRF, 0x00001);
		halrf_wrf(rf, (enum rf_path)i, 0x3f, MASKRF, 0xb2120);
		halrf_wrf(rf, (enum rf_path)i, 0x33, 0xf, 0x2);
		halrf_wrf(rf, (enum rf_path)i, 0x3e, MASKRF, 0x00001);
		halrf_wrf(rf, (enum rf_path)i, 0x3f, MASKRF, 0xfe124);
		halrf_wrf(rf, (enum rf_path)i, 0x33, 0xf, 0x3);
		halrf_wrf(rf, (enum rf_path)i, 0x3e, MASKRF, 0x00001);
		halrf_wrf(rf, (enum rf_path)i, 0x3f, MASKRF, 0x30d7c);
		halrf_wrf(rf, (enum rf_path)i, 0x33, 0xf, 0xa);
		halrf_wrf(rf, (enum rf_path)i, 0x3e, MASKRF, 0x00001);
		halrf_wrf(rf, (enum rf_path)i, 0x3f, MASKRF, 0x30d7e);
		halrf_wrf(rf, (enum rf_path)i, 0x33, 0xf, 0xb);
		halrf_wrf(rf, (enum rf_path)i, 0x3e, MASKRF, 0x00001);
		halrf_wrf(rf, (enum rf_path)i, 0x3f, MASKRF, 0x30d7d);
		halrf_wrf(rf, (enum rf_path)i, 0xef, BIT(19), 0x0);
	}
}

bool halrf_rfk_reg_check_8852bp(struct rf_info *rf)
{
	u32 i, reg, temp;
	bool fail = false;

	for (i = 0; i < KIP_REG; i++) {
		reg = 0x8000 + i*4;
		if (((reg >= 0x8000 && reg < 0x8300) || 
			(reg >= 0x8500 && reg < 0x9b60) || 
			(reg >= 0x9d00 && reg < 0xa360) ||
			(reg >= 0xa400 && reg < 0xa444) ||
			(reg >= 0xa500 && reg < 0xa880) ||
			(reg >= 0xa900 && reg < 0xac80) ||
			(reg >= 0xb100)) &&
			(reg != 0x8014) &&
			(reg != 0x80f8) && (reg != 0x80fc) &&
			(reg != 0x81f8) && (reg != 0x81fc) &&
			(reg != 0x82f8) && (reg != 0x82fc) &&
			(reg != 0xb1f8) && (reg != 0xb1fc) &&
			(reg != 0x81b4) && (reg != 0x82b4)) {
			temp = halrf_rreg(rf, reg, MASKDWORD);
			if (rf->rfk_reg[i] != temp) {
				RF_DBG(rf, DBG_RF_RFK,
					"[RFK]reg 0x%x b 0x%x/a 0x%x\n",
					reg,
					rf->rfk_reg[i],
					temp);
#if 0
				for (t =0; t < 10; t++) {
					temp = halrf_rreg(rf, reg, MASKDWORD);
					RF_DBG(rf, DBG_RF_RFK, "[RFK]reg 0x%x mismatch: before 0x%x/after 0x%x\n", reg, rf->rfk_reg[i], temp);
					halrf_delay_ms(rf, 1);
				}
#endif
				fail = true;
				rf->rfk_check_fail_count++;
			}
		}
	}
	return fail;
}


void halrf_rfk_reg_reload_8852bp(struct rf_info *rf)
{
	u32 i, reg;

#ifdef CF_PHL_BB_CTRL_RX_CCA
	halrf_bb_ctrl_rx_cca(rf, false, HW_PHY_0);
#else
	RF_WARNING("[RF]%s !! no CF_PHL_BB_CTRL_RX_CCA\n", __FUNCTION__);
#endif
	halrf_wreg(rf, 0x0a70, 0x3e0, 0x1f);
	halrf_wreg(rf, 0x8080, MASKDWORD, 0x00000005);
	halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000110);
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00010001);
	halrf_wreg(rf, 0x82d8, MASKDWORD, 0x00010001);
	RF_DBG(rf, DBG_RF_RFK, "[RFK]KIP_REG = %d\n", KIP_REG);
	for (i = 0; i < KIP_REG; i++) {
		reg = 0x8000 + i*4;
		halrf_wreg(rf, reg, MASKDWORD, rf->rfk_reg[i]);
	}


	
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x81dc, MASKDWORD, 0x00010003);
	halrf_wreg(rf, 0x81dc, MASKDWORD, 0x00000002);
	halrf_wreg(rf, 0x82d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x82dc, MASKDWORD, 0x00010003);
	halrf_wreg(rf, 0x82dc, MASKDWORD, 0x00000002);
	halrf_wreg(rf, 0x8080, MASKDWORD, 0x00000004);
	halrf_wreg(rf, 0x8080, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000000);
	halrf_wreg(rf, 0x0a70, 0x3e0, 0x0);
#ifdef CF_PHL_BB_CTRL_RX_CCA
	halrf_bb_ctrl_rx_cca(rf, true, HW_PHY_0);
#else
	RF_WARNING("[RF]%s !! no CF_PHL_BB_CTRL_RX_CCA\n", __FUNCTION__);
#endif
}



void halrf_rfk_reg_backup_8852bp(struct rf_info *rf)
{
	u32 i, reg;

#ifdef CF_PHL_BB_CTRL_RX_CCA
	halrf_bb_ctrl_rx_cca(rf, false, HW_PHY_0);
#else
	RF_WARNING("[RF]%s !! no CF_PHL_BB_CTRL_RX_CCA\n", __FUNCTION__);
#endif
	halrf_wreg(rf, 0x0a70, 0x3e0, 0x1f);
	halrf_wreg(rf, 0x8080, MASKDWORD, 0x00000005);
	halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000110);
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00010001);
	halrf_wreg(rf, 0x82d8, MASKDWORD, 0x00010001);
	RF_DBG(rf, DBG_RF_RFK, "[RFK]KIP_REG = %d\n", KIP_REG);
	for (i = 0; i < KIP_REG; i++) {
		reg = 0x8000 + i*4;
		rf->rfk_reg[i] = halrf_rreg(rf, reg, MASKDWORD);
	}

	rf->rfk_reg[1791] = rf->rfk_reg[1344];
	rf->rfk_reg[2303] = rf->rfk_reg[1856];

	rf->rfk_reg[2623] = rf->rfk_reg[2368];
	rf->rfk_reg[2879] = rf->rfk_reg[2624];

	for (i = 0; i < KIP_REG; i++) {
		reg = 0x8000 + i*4;
		RF_DBG(rf, DBG_RF_RFK,
			"[RFK]bk reg 0x%x = 0x%x\n",
			reg,
			rf->rfk_reg[i]);
	}

	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x81dc, MASKDWORD, 0x00010003);
	halrf_wreg(rf, 0x81dc, MASKDWORD, 0x00000002);
	halrf_wreg(rf, 0x82d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x82dc, MASKDWORD, 0x00010003);
	halrf_wreg(rf, 0x82dc, MASKDWORD, 0x00000002);
	halrf_wreg(rf, 0x8080, MASKDWORD, 0x00000004);
	halrf_wreg(rf, 0x8080, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000000);
	halrf_wreg(rf, 0x0a70, 0x3e0, 0x0);
#ifdef CF_PHL_BB_CTRL_RX_CCA
	halrf_bb_ctrl_rx_cca(rf, true, HW_PHY_0);
#else
	RF_WARNING("[RF]%s !! no CF_PHL_BB_CTRL_RX_CCA\n", __FUNCTION__);
#endif
}

bool halrf_rfk_reg_check_fail_8852bp(struct rf_info *rf)
{
	bool fail = false;

#ifdef CF_PHL_BB_CTRL_RX_CCA
	halrf_bb_ctrl_rx_cca(rf, false, HW_PHY_0);
#else
	RF_WARNING("[RF]%s !! no CF_PHL_BB_CTRL_RX_CCA\n", __FUNCTION__);
#endif
	rf->rfk_check_fail_count = 0;
	halrf_wreg(rf, 0x0a70, 0x3e0, 0x1f);
	halrf_wreg(rf, 0x8080, MASKDWORD, 0x00000005);
	halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000110);
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00010001);
	halrf_wreg(rf, 0x82d8, MASKDWORD, 0x00010001);
	fail = halrf_rfk_reg_check_8852bp(rf);
	halrf_wreg(rf, 0x81d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x81dc, MASKDWORD, 0x00010003);	
	halrf_wreg(rf, 0x81dc, MASKDWORD, 0x00000002);
	halrf_wreg(rf, 0x82d8, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x82dc, MASKDWORD, 0x00010003);
	halrf_wreg(rf, 0x82dc, MASKDWORD, 0x00000002);
	halrf_wreg(rf, 0x8080, MASKDWORD, 0x00000004);
	halrf_wreg(rf, 0x8080, MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000000);
	halrf_wreg(rf, 0x0a70, 0x3e0, 0x0);
	RF_DBG(rf, DBG_RF_RFK,
		"[RFK]fail count = %d\n",
		rf->rfk_check_fail_count);
#ifdef CF_PHL_BB_CTRL_RX_CCA
	halrf_bb_ctrl_rx_cca(rf, true, HW_PHY_0);
#else
	RF_WARNING("[RF]%s !! no CF_PHL_BB_CTRL_RX_CCA\n", __FUNCTION__);
#endif
	return fail;
}

static u32 r_d[] = {0x18, 0xb2, 0xc5};
static u32 r_m[] = {0xfffff, 0xfffff, 0x8000};

void halrf_rfc_reg_backup_8852bp(struct rf_info *rf)
{
	u8 i;

	for (i = 0; i < 3; i++) {
		rf->rfc_reg[0][i] = halrf_rrf(rf, RF_PATH_A, r_d[i], r_m[i]);
		rf->rfc_reg[1][i] = halrf_rrf(rf, RF_PATH_B, r_d[i], r_m[i]);  
	}
}

bool halrf_rfc_reg_check_fail_8852bp(struct rf_info *rf)
{
	u8 i;
	u32 temp;
	bool fail = false;

	for (i = 0; i < 3; i++) {
		temp = halrf_rrf(rf, RF_PATH_A, r_d[i], r_m[i]);
		if (rf->rfc_reg[0][i] !=temp) {
			RF_DBG(rf, DBG_RF_RFK,
				"[RFK]S0 0x%x mask=0x%x, old=0x%x, new=0x%x\n",
				r_d[i],
				r_m[i],
				rf->rfc_reg[0][i],
				temp);
			fail = true;
		}
	}
	for (i = 0; i < 3; i++) {
		temp = halrf_rrf(rf, RF_PATH_B, r_d[i], r_m[i]);
		if (rf->rfc_reg[1][i] !=temp) {
			RF_DBG(rf, DBG_RF_RFK,
				"[RFK]S1 0x%x mask=0x%x, old=0x%x, new=0x%x\n",
				r_d[i],
				r_m[i],
				rf->rfc_reg[1][i],
				temp);
			fail = true;
		}
	}
	return fail;
}




void halrf_set_rxbb_bw_8852bp(struct rf_info *rf, enum channel_width bw, enum rf_path path)
{
	halrf_wrf(rf, path, 0xee, BIT(2), 0x1);
	halrf_wrf(rf, path, 0x33, 0x0001F, 0xa); /*[4:0]*/

	if (bw == CHANNEL_WIDTH_20)
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0x1b); /*[5:0]*/
	else if (bw == CHANNEL_WIDTH_40)
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0x13); /*[5:0]*/
	else if (bw == CHANNEL_WIDTH_80)
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0xb); /*[5:0]*/
	else if (bw == CHANNEL_WIDTH_160)
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0x3); /*[5:0]*/
	else
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0x3); /*[5:0]*/

	RF_DBG(rf, DBG_RF_RFK, "[RFK] RXBB BW set 0x3F = 0x%x\n",
		halrf_rrf(rf, path, 0x3f, 0x0003F));

	halrf_wrf(rf, path, 0xee, BIT(2), 0x0);
}

void halrf_rxbb_bw_8852bp(struct rf_info *rf, enum phl_phy_idx phy, enum channel_width bw)
{
	u8 kpath, path;

	kpath = halrf_kpath_8852bp(rf, phy);
	
	for (path = 0; path < 2; path++) {
		if ((kpath & BIT(path)) && (rf->pre_rxbb_bw[path] != bw)) {
			halrf_set_rxbb_bw_8852bp(rf, bw, path);
			rf->pre_rxbb_bw[path] = bw;
		} else
			RF_DBG(rf, DBG_RF_RFK,
			       "[RFK] S%d RXBB BW unchanged (pre_bw = 0x%x)\n",
			       path, rf->pre_rxbb_bw[path]);
	}
}

void halrf_disconnect_notify_8852bp(struct rf_info *rf, struct rtw_chan_def *chandef) {

	struct halrf_gapk_info *txgapk_info = &rf->gapk;
	struct halrf_mcc_info *mcc_info = &rf->mcc_info;
	u8 ch;
	
	RF_DBG(rf, DBG_RF_RFK, "[IQK]===>%s\n", __func__);
#if 0
	/*[IQK disconnect]*/
	for (ch = 0; ch < 2; ch++) {
		for (path = 0; path < KPATH; path++) {
			if (iqk_info->iqk_mcc_ch[ch][path] == chandef->center_ch)
				iqk_info->iqk_mcc_ch[ch][path] = 0x0;
		}

	}
#endif
	/*TXGAPK*/
	for (ch = 0; ch < 2; ch++) {		
		if (txgapk_info->txgapk_mcc_ch[ch] == chandef->center_ch)
				txgapk_info->txgapk_mcc_ch[ch] = 0x0;
	}

	/*mcc info*/
	for (ch = 0; ch < 2; ch++) {		
		if ((mcc_info->ch[ch] == chandef->center_ch) && (mcc_info->band[ch] == chandef->band)) {
			mcc_info->ch[ch] = 0x0;
			mcc_info->band[ch] = 0x0;
		}
				
	}
	
}

bool halrf_check_mcc_ch_8852bp(struct rf_info *rf, struct rtw_chan_def *chandef) {

	struct halrf_mcc_info *mcc_info = &rf->mcc_info;
	u8 ch;
	bool check = false;

	RF_DBG(rf, DBG_RF_RFK, "[RFK]===>%s, center_ch(%d)\n", __func__, chandef->center_ch);
	/*[IQK check_mcc_ch]*/
	for (ch = 0; ch < 2; ch++) {
			if ((mcc_info->ch[ch] == chandef->center_ch) && ( mcc_info->band[ch] == chandef->band)) {
				check = true;
				return check;
			}
	}
	return check;
}

void halrf_fw_ntfy_8852bp(struct rf_info *rf, enum phl_phy_idx phy_idx) {
	u8 i = 0x0;
	struct halrf_mcc_info *mcc_info = &rf->mcc_info;
	u32 data_to_fw[6] = {0};
	u16 len = (u16) (sizeof(data_to_fw) / sizeof(u32))*4;

	data_to_fw[0] = (u32) mcc_info->ch[0];
	data_to_fw[1] = (u32) mcc_info->ch[1];
	data_to_fw[2] = (u32) mcc_info->band[0];
	data_to_fw[3] = (u32) mcc_info->band[1];
	data_to_fw[4] = rf->hal_com->band[phy_idx].cur_chandef.center_ch;
	data_to_fw[5] = rf->hal_com->band[phy_idx].cur_chandef.band;

	RF_DBG(rf, DBG_RF_RFK, "[IQK] len = 0x%x\n", len);
	//for (i =0; i < 5; i++)
	for (i =0; i < 6; i++)
		RF_DBG(rf, DBG_RF_RFK, "[IQK] data_to_fw[%x] = 0x%x\n", i, data_to_fw[i]);

	halrf_fill_h2c_cmd(rf, len, FWCMD_H2C_GET_MCCCH, 0xa, H2CB_TYPE_DATA, (u32 *) data_to_fw);	

	return;
}

void halrf_before_one_shot_enable_8852bp(struct rf_info *rf) {	

	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);

	/* set 0x80d4[21:16]=0x03 (before oneshot NCTL) to get report later */
	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x03);
		
	RF_DBG(rf, DBG_RF_RFK, "======> before set one-shot bit, 0x%x= 0x%x\n", 0x8010, halrf_rreg(rf, 0x8010, MASKDWORD));
}

bool halrf_one_shot_nctl_done_check_8852bp(struct rf_info *rf, enum rf_path path) {	
	/* for check status */
	u32 r_bff8 = 0;
	u32 r_80fc = 0;
	bool is_ready = false;
	u16 count = 1;

	rf->nctl_ck_times[0] = 0;
	rf->nctl_ck_times[1] = 0;

	
	/* for 0xbff8 check NCTL DONE */
	while (count < 2000) {	
		/* r_bff8 = 0; */
		r_bff8 = halrf_rreg(rf, 0xbff8, MASKBYTE0);
				
		if (r_bff8 == 0x55) {
			is_ready = true;
			break;
		}	
	
		halrf_delay_us(rf, 10);
		count++;
	}
	halrf_delay_us(rf, 1);
	rf->nctl_ck_times[0] = count;
	
	RF_DBG(rf, DBG_RF_RFK, "======> path = %d, check 0xBFF8[7:0] = 0x%x, IsReady = %d, ReadTimes = %d,delay 1 us\n", path, r_bff8, is_ready, count);

#if 0 /* wait for RDC info */	
	/* for 0x80fc check NCTL DONE */	
	/* for 52A last line 0x9c00 */
	/* for 52B last line 0x8000 */
	count = 1;
	is_ready = false;
	while (count < 2000) {	
		/* r_80fc = 0; */
		r_80fc = halrf_rreg(rf, 0x80fc, MASKLWORD);
	
		if (r_80fc == NCTL_FINAL_LINE_8852BP) {
			is_ready = true;
			break;
		}	
			
		halrf_delay_us(rf, 1);
		count++;
	}
	halrf_delay_us(rf, 1);
	rf->nctl_ck_times[1] = count;
		
	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);

	RF_DBG(rf, DBG_RF_RFK, "======> check 0x80fc[15:0] = 0x%x, IsReady = %d, ReadTimes = %d, 0x%x= 0x%x \n", r_80fc, is_ready, count, 0x8010, halrf_rreg(rf, 0x8010, MASKDWORD) ); 
#else 
	r_80fc = 0;
	halrf_delay_us(rf, 10);
	
	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);
#endif

	return is_ready;
}

bool halrf_do_one_shot_8852bp(struct rf_info *rf, enum rf_path path, u32 nctl_addr, u32 mask, u32 process_id)
{

	halrf_before_one_shot_enable_8852bp(rf);
	halrf_wreg(rf, nctl_addr, mask, process_id); //set cal_path, process id

	return halrf_one_shot_nctl_done_check_8852bp(rf, path);
}


void halrf_lck_8852bp(struct rf_info *rf)
{
	/*lck must be called by watchdog*/
	u32 temp18[2], path, i;

	RF_DBG(rf, DBG_RF_LCK, "[LCK]DO LCK\n");
	temp18[0] = halrf_rrf(rf, RF_PATH_A, 0x18, MASKRF);
	temp18[1] = halrf_rrf(rf, RF_PATH_B, 0x18, MASKRF);

	if (!rf->hal_com->dbcc_en)
		path = 1;
	else
		path = 2;

	for(i = 0; i < path; i++) {
		halrf_wrf(rf, i, 0xd3, BIT(8), 0x1);
		halrf_wrf(rf, i, 0x18, MASKRF, temp18[i]);
		halrf_wrf(rf, i, 0xd3, BIT(8), 0x0);		
	}
	rf->lck_ther_s0 = rf->cur_ther_s0;
	rf->lck_ther_s1 = rf->cur_ther_s1;
}

void halrf_lck_tracking_8852bp(struct rf_info *rf)
{
	u32 temp;

	RF_DBG(rf, DBG_RF_LCK,
		"[LCK]curT_s0(%d), curT_s1(%d), lckT_s0(%d), lckT_s1(%d)\n",
		rf->cur_ther_s0, rf->cur_ther_s1,
		rf->lck_ther_s0, rf->lck_ther_s1);
	if (((rf->lck_ther_s0 == 0) && (rf->lck_ther_s1 == 0))) {
		halrf_lck_trigger(rf);
		return;
	}
	if (HALRF_ABS(rf->lck_ther_s0, rf->cur_ther_s0) >
		HALRF_ABS(rf->lck_ther_s1, rf->cur_ther_s1)) {
		temp = HALRF_ABS(rf->lck_ther_s0, rf->cur_ther_s0);
	}else {
		temp = HALRF_ABS(rf->lck_ther_s1, rf->cur_ther_s1);
	}

	RF_DBG(rf, DBG_RF_LCK, "[LCK]MAX_delta = %d, LCK_TH=%x\n",
		temp, LCK_TH_8852BP);

	if(temp >= LCK_TH_8852BP)		
		halrf_lck_trigger(rf);
}

static u32 backup_mac_reg_8852bp[] = {0x0};
static u32 backup_bb_reg_8852bp[] = {0x2344,0xc0d4,0xc0d8, 0xc0e8, 0xc1d4,0xc1d8, 0xc1e8};
static u32 backup_rf_reg_8852bp[] = {0xdc, 0xef, 0xde, 0x0, 0x1e, 0x5, 0x10005};

static struct halrf_iqk_ops iqk_ops= {
    .iqk_kpath = halrf_kpath_8852bp,
    .iqk_mcc_page_sel = iqk_mcc_page_sel_8852bp,
    .iqk_get_ch_info = iqk_get_ch_info_8852bp,
    .iqk_preset = iqk_preset_8852bp,
    .iqk_macbb_setting = iqk_macbb_setting_8852bp,
    .iqk_start_iqk = iqk_start_iqk_8852bp,
    .iqk_restore = iqk_restore_8852bp,
    .iqk_afebb_restore = iqk_afebb_restore_8852bp,  
};

struct rfk_iqk_info rf_iqk_hwspec_8852bp = {
  	.rf_iqk_ops = &iqk_ops,
	.rf_max_path_num = 2,
	.rf_iqk_version = iqk_version_8852bp,
	.rf_iqk_ch_num = 2,
	.rf_iqk_path_num = 2,

#if 0
	.backup_mac_reg = backup_mac_reg_8852bp,
	.backup_mac_reg_num = ARRAY_SIZE(backup_mac_reg_8852bp),
#else
	.backup_mac_reg = backup_mac_reg_8852bp,
	.backup_mac_reg_num = 0,
#endif

    	.backup_bb_reg = backup_bb_reg_8852bp,
    	.backup_bb_reg_num = ARRAY_SIZE(backup_bb_reg_8852bp),
    	.backup_rf_reg = backup_rf_reg_8852bp,
    	.backup_rf_reg_num = ARRAY_SIZE(backup_rf_reg_8852bp),
};

#endif
