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
#ifdef RF_8832BR_SUPPORT
bool halrf_bw_setting_8832br(struct rf_info *rf, enum rf_path path, enum channel_width bw, bool is_dav)
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
//		halrf_wreg(rf, 0xc0d4 |( path << 8), BIT(27) | BIT(26), 0x2);
//		halrf_wreg(rf, 0xc0d8 |( path << 8), 0x00001e0, 0x7);
		break;
	case CHANNEL_WIDTH_40:
		rf_reg18 |= BIT(11);
//		halrf_wreg(rf, 0xc0d4 |( path << 8), BIT(27) | BIT(26), 0x2);
//		halrf_wreg(rf, 0xc0d8 |( path << 8), 0x00001e0, 0x7);
		break;
	case CHANNEL_WIDTH_80:
		rf_reg18 |= BIT(10);
//		halrf_wreg(rf, 0xc0d4 |( path << 8), BIT(27) | BIT(26), 0x2);
//		halrf_wreg(rf, 0xc0d8 |( path << 8), 0x00001e0, 0x7);
		break;
	case CHANNEL_WIDTH_160:
//		halrf_wreg(rf, 0xc0d4 |( path << 8), BIT(27) | BIT(26), 0x0);
//		halrf_wreg(rf, 0xc0d8 |( path << 8), 0x00001e0, 0x7);
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

bool halrf_ctrl_bw_8832br(struct rf_info *rf, enum phl_phy_idx phy, enum channel_width bw)
{
	bool is_dav;
	u8 kpath, path;
	//RF_DBG(rf, DBG_RF_RFK, "[RFK]===> %s\n", __func__);

	/*==== Error handling ====*/
	if (bw >= CHANNEL_WIDTH_MAX ) {
		RF_DBG(rf, DBG_RF_RFK,"[RFK]Fail to switch bw(bw:%d)\n", bw);
		return false;
	}

	kpath = halrf_kpath_8832br(rf, phy);

	for (path = 0; path < 2; path++) {
		if (kpath & BIT(path)) {
			is_dav = true;
			halrf_bw_setting_8832br(rf, path, bw, is_dav);
			is_dav = false;
			halrf_bw_setting_8832br(rf, path, bw, is_dav);
			RF_DBG(rf, DBG_RF_RFK, "[RFK] BW: %d\n", bw);
			RF_DBG(rf, DBG_RF_RFK, "[RFK] 0x18 = 0x%x\n", halrf_rrf(rf, path, 0x18, MASKRF));
		}
	}

	halrf_bw_setting_op5k_8832br(rf);

	return true;
}

bool halrf_set_s0_arfc18_8832br(struct rf_info *rf, u32 val)
{
	u32 c = 1000;
	bool timeout = false;

	halrf_wrf(rf, RF_PATH_A, 0xd3, BIT(8), 0x1);
	halrf_wrf(rf, RF_PATH_A, 0x18, MASKRF, val);
	c = 0;
	while (c < 1000) {
		if (halrf_rrf(rf, RF_PATH_A, 0xb7, BIT(8)) == 0)
			break;
		c++;
		halrf_delay_us(rf, 1);
	}
	if (c == 1000) {
		timeout = true;
		RF_WARNING("[LCK][RFK]LCK timeout\n");
	}
	halrf_wrf(rf, RF_PATH_A, 0xd3, BIT(8), 0x0);
	return timeout;
}

bool halrf_do_lck_check_8832br(struct rf_info *rf)
{
	u32 step = 1;
	bool lck_fail = false, mask_step2 =  false;
	u32 temp_18, temp_a0, temp_af, temp_b1;

	//RF_WARNING("[LCK]====>%s\n", __func__);

	for(step = 1; step <4; step++) {
		switch (step) {
		case 1:
			if (halrf_rrf(rf, RF_PATH_A, 0xc5, BIT(15)) == 0) {
				RF_WARNING("[LCK]SYN MMD reset\n");
				halrf_wrf(rf, RF_PATH_A, 0xd5, BIT(8), 0x1);
				halrf_wrf(rf, RF_PATH_A, 0xd5, BIT(6), 0x0);
				halrf_wrf(rf, RF_PATH_A, 0xd5, BIT(6), 0x1);
				halrf_wrf(rf, RF_PATH_A, 0xd5, BIT(8), 0x0);
				halrf_delay_us(rf, 10);
				lck_fail = true;
			}
			break;
		case 2:
			if (mask_step2)
				break;
			if (halrf_rrf(rf, RF_PATH_A, 0xc5, BIT(15)) == 0) {
				RF_WARNING("[LCK]re-set RF 0x18\n");
				temp_18 = halrf_rrf(rf, RF_PATH_A, 0x18, MASKRF);
				temp_b1 = halrf_rrf(rf, RF_PATH_A, 0xb1, MASKRF);
				halrf_wrf(rf, RF_PATH_A, 0xb1, 0x1c0, 0x1);
				halrf_set_s0_arfc18_8832br(rf, temp_18);
				halrf_wrf(rf, RF_PATH_A, 0xb1, MASKRF, temp_b1);
				lck_fail = true;
			} else {
				lck_fail = false;
			}
			break;
		case 3:
			mask_step2 = true;
			if (halrf_rrf(rf, RF_PATH_A, 0xc5, BIT(15)) == 0) {
				RF_WARNING("[LCK]SYN off/on\n");
				temp_18 = halrf_rrf(rf, RF_PATH_A, 0x18, MASKRF);
				temp_a0 = halrf_rrf(rf, RF_PATH_A, 0xa0, MASKRF);
				temp_af = halrf_rrf(rf, RF_PATH_A, 0xaf, MASKRF);
				temp_b1 = halrf_rrf(rf, RF_PATH_A, 0xb1, MASKRF);
				halrf_wrf(rf, RF_PATH_A, 0xa0, MASKRF, temp_a0);
				halrf_wrf(rf, RF_PATH_A, 0xaf, MASKRF, temp_af);
				halrf_wrf(rf, RF_PATH_A, 0xdd, BIT(4), 0x1);
				halrf_wrf(rf, RF_PATH_A, 0xa0, 0xc, 0x0);
				halrf_delay_us(rf, 10);
				halrf_wrf(rf, RF_PATH_A, 0xa0, 0xc, 0x3);
				halrf_wrf(rf, RF_PATH_A, 0xdd, BIT(4), 0x0);
				halrf_delay_us(rf, 40);
				halrf_wrf(rf, RF_PATH_A, 0xb1, 0x1c0, 0x1);
				halrf_set_s0_arfc18_8832br(rf, temp_18);
				halrf_wrf(rf, RF_PATH_A, 0xb1, MASKRF, temp_b1);
				lck_fail = true;
			} else { 
				lck_fail = false;
			}
			break;
		default:
			break;
		}
		if (!lck_fail)
			break;
	}
	return lck_fail;
}

void halrf_lck_check_8832br(struct rf_info *rf)
{
	u32 c = 0;
	bool lck_fail = false;

	RF_DBG(rf, DBG_RF_RFK, "[RFK]===>%s\n", __func__);
	while (c < 20) {
		c++;
		lck_fail = halrf_do_lck_check_8832br(rf);
		if (!lck_fail)
			break;
	}

	if (lck_fail) {
		RF_WARNING("[LCK]0xb2=%x, 0xc5=%x\n",
		halrf_rrf(rf, RF_PATH_A, 0xb2, MASKRF),
		halrf_rrf(rf, RF_PATH_A, 0xc5, MASKRF));
	}
}

void halrf_set_ch_8832br(struct rf_info *rf, u32 val) {

	bool timeout;

	RF_DBG(rf, DBG_RF_RFK, "[RFK]===>%s\n", __func__);	

	halrf_wrf(rf, RF_PATH_A, 0xb1, 0x1c0, 0x1);	
	timeout = halrf_set_s0_arfc18_8832br(rf, val);
	halrf_wrf(rf, RF_PATH_A, 0xb1, 0x1c0, 0x7);
	if (!timeout)
		halrf_lck_check_8832br(rf);
}

bool halrf_ch_setting_8832br(struct rf_info *rf,   enum rf_path path, u8 central_ch, enum band_type band, bool is_dav)
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
	if ((path == RF_PATH_A) && is_dav) {
		halrf_set_ch_8832br(rf, rf_reg18);
	} else
	halrf_wrf(rf, path, reg_reg18_addr, MASKRF, rf_reg18);
//	halrf_delay_us(rf, 100);
	//halrf_wrf(rf, path, 0xcf, BIT(0), 0);
	//halrf_wrf(rf, path, 0xcf, BIT(0), 1);
	RF_DBG(rf, DBG_RF_RFK, "[RFK] CH: %d for Path-%d, reg0x%x = 0x%x\n", central_ch, path, reg_reg18_addr, halrf_rrf(rf, path, reg_reg18_addr, MASKRF));
	return true;
}

bool halrf_ctrl_ch_8832br(struct rf_info *rf,  enum phl_phy_idx phy, u8 central_ch, enum band_type band)
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
	
	kpath = halrf_kpath_8832br(rf, phy);
	
	for (path = 0; path < 2; path++) {
		if (kpath & BIT(path)) {
			is_dav = true;
			halrf_ch_setting_8832br(rf, path, central_ch, band, is_dav);
			is_dav = false;
			halrf_ch_setting_8832br(rf, path, central_ch, band, is_dav);	
		}
	}

	RF_DBG(rf, DBG_RF_RFK, "[RFK] CH: %d\n", central_ch);

	return true;
}

void halrf_set_lo_8832br(struct rf_info *rf, bool is_on, enum rf_path path)
{
	if (is_on) {
		halrf_rf_direct_cntrl_8832br(rf, path, false);
		halrf_wrf(rf, path, 0x0, MASKRFMODE, 0x2);
		halrf_wrf(rf, path, 0x58, BIT(1), 0x1);
		halrf_wrf(rf, path, 0xde, 0x1800, 0x3);
		halrf_wrf(rf, path, 0x56, 0x1c00, 0x1);
		halrf_wrf(rf, path, 0x56, 0x1e0, 0x1);
	} else {
		halrf_wrf(rf, path, 0x58, BIT(1), 0x0);
		halrf_rf_direct_cntrl_8832br(rf, path, true);
		halrf_wrf(rf, path, 0xde, 0x1800, 0x0);
	}
}

void halrf_rf_direct_cntrl_8832br(struct rf_info *rf, enum rf_path path, bool is_bybb)
{
	if (is_bybb)
		halrf_wrf(rf, path, 0x5, BIT(0), 0x1);
	else
		halrf_wrf(rf, path, 0x5, BIT(0), 0x0);
}

void halrf_drf_direct_cntrl_8832br(struct rf_info *rf, enum rf_path path, bool is_bybb)
{
	if (is_bybb)
		halrf_wrf(rf, path, 0x10005, BIT(0), 0x1);
	else
		halrf_wrf(rf, path, 0x10005, BIT(0), 0x0);
}


void halrf_lo_test_8832br(struct rf_info *rf, bool is_on, enum rf_path path)
{
	switch (path) {
		case RF_PATH_A:
			halrf_set_lo_8832br(rf, is_on, RF_PATH_A);
			halrf_set_lo_8832br(rf, false, RF_PATH_B);
			break;
		case RF_PATH_B:
			halrf_set_lo_8832br(rf, false, RF_PATH_A);
			halrf_set_lo_8832br(rf, is_on, RF_PATH_B);
			break;
		case RF_PATH_AB:
			halrf_set_lo_8832br(rf, is_on, RF_PATH_A);
			halrf_set_lo_8832br(rf, is_on, RF_PATH_B);
			break;
		default:
			break;
	}
}

u8 halrf_kpath_8832br(struct rf_info *rf, enum phl_phy_idx phy_idx) {

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

void _rx_dck_info_8832br(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path, bool is_afe)
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
	       (rx_dck->loc[path].cur_bw == 1 ? "40M" : "80M"),
	       	rx_dck->is_afe ? "AFE" : "RFC");	
}

void halrf_set_rx_dck_8832br(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path, bool is_afe)
{
	u8 phy_map;
	u32 ori_val, i = 0;

	phy_map = (BIT(phy) << 4) | BIT(path);

	_rx_dck_info_8832br(rf, phy, path, is_afe);

	halrf_wrf(rf, path, 0x93, 0x0000f, 0x0); /*0: from RFC; 1: from AFE*/
#if 0
	halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_RXDCK, RFK_ONESHOT_START);
#endif
	halrf_wrf(rf, path, 0x92, BIT(0), 0x0);
	halrf_wrf(rf, path, 0x92, BIT(0), 0x1);

	for (i = 0; i < 30; i++) /*delay 600us*/
		halrf_delay_us(rf, 20);
#if 0
	halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_RXDCK, RFK_ONESHOT_STOP);
#endif
#if 0
	RF_DBG(rf, DBG_RF_RFK, "[RX_DCK] 0x92 = 0x%x, 0x93 = 0x%x\n",
	       halrf_rrf(rf, path, 0x92, MASKRF),
	       halrf_rrf(rf, path, 0x93, MASKRF));
#endif
}

bool halrf_rx_dck_check_8832br(struct rf_info *rf, enum rf_path path)
{
	u8 addr;
	bool is_fail = false;

	if (halrf_rreg(rf, 0xc400 + path * 0x1000, 0xF0000) == 0x0)
		 return is_fail = true;
	else if (halrf_rreg(rf, 0xc400 + path * 0x1000, 0x0F000) == 0x0)
		return is_fail = true;
	else if (halrf_rreg(rf, 0xc440 + path * 0x1000, 0xF0000) == 0x0)
		return is_fail = true;
	else if (halrf_rreg(rf, 0xc440 + path * 0x1000, 0x0F000) == 0x0)
		return is_fail = true;
	else {
		for (addr = 0x0; addr < 0x20; addr++) {
			if (halrf_rreg(rf, 0xc400 + path * 0x1000 + addr * 4, 0x00FC0) == 0x0)
				return is_fail = true;
		}

		for (addr = 0x0; addr < 0x20; addr++) {
			if (halrf_rreg(rf, 0xc400 + path * 0x1000 + addr * 4, 0x0003F) == 0x0)
				return is_fail = true;
		}
	}

	return is_fail;
}

void halrf_rx_dck_8832br(struct rf_info *rf, enum phl_phy_idx phy, bool is_afe) 
{
	u8 path, dck_tune;
	u32 rf_reg5;

	RF_DBG(rf, DBG_RF_RXDCK, "[RX_DCK] ****** RXDCK Start (Ver: 0x%x, Cv: %d) ******\n",
		RXDCK_VER_8832BR, rf->hal_com->cv);

	for (path = 0; path < 2; path++) {
		rf_reg5 = halrf_rrf(rf, path, 0x5, MASKRF);
		dck_tune = (u8)halrf_rrf(rf, path, 0x92, BIT(1));

		if (rf->is_tssi_mode[path]) 
			halrf_wreg(rf, 0x5818 + (path << 13), BIT(30), 0x1); /*TSSI pause*/

		halrf_wrf(rf, path, 0x5, BIT(0), 0x0);
		halrf_wrf(rf, path, 0x92, BIT(1), 0x0);
		//halrf_wrf(rf, path, 0x8f, BIT(11) | BIT(10), 0x1); /*EN_TIA_IDAC_LSB[1:0]*/
		halrf_wrf(rf, path, 0x00, MASKRFMODE, RF_RX);
		halrf_set_rx_dck_8832br(rf, phy, path, is_afe);
#if 0
		if (halrf_rx_dck_check_8832br(rf, path)) {
			RF_DBG(rf, DBG_RF_RFK, "[RX_DCK] S%d RX_DCK value = 0 happen!!!\n", path);
			halrf_wrf(rf, path, 0x8f, BIT(11) | BIT(10), 0x2); /*EN_TIA_IDAC_LSB[1:0]*/
			halrf_set_rx_dck_8832br(rf, phy, path, is_afe);
		}
#endif
		halrf_wrf(rf, path, 0x92, BIT(1), dck_tune);
		halrf_wrf(rf, path, 0x5, MASKRF, rf_reg5);

		if (rf->is_tssi_mode[path])
			halrf_wreg(rf, 0x5818 + (path << 13), BIT(30), 0x0); /*TSSI resume*/

	}
}

void halrf_rx_dck_onoff_8832br(struct rf_info *rf, bool is_enable)
{
	u8 path;

	for (path = 0; path < 2; path++) {
		halrf_wrf(rf, path, 0x93, BIT(0), !is_enable);
		if (!is_enable) {
			halrf_wrf(rf, path, 0x92, 0xFFC00, 0x220); /*[19:10]*/
			halrf_wrf(rf, path, 0x93, 0xFFC00, 0x220); /*[19:10]*/
		}
	}
}

void halrf_rck_8832br(struct rf_info *rf, enum rf_path path)
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

void iqk_backup_8832br(struct rf_info *rf, enum rf_path path) 
{
	return;
}

void halrf_bf_config_rf_8832br(struct rf_info *rf)
{
	halrf_wrf(rf, RF_PATH_A, 0xef, BIT(19), 0x1);
	halrf_wrf(rf, RF_PATH_A, 0x33, 0xf, 0x1);
	halrf_wrf(rf, RF_PATH_A, 0x3e, MASKRF, 0x000c6);
	halrf_wrf(rf, RF_PATH_A, 0x3f, MASKRF, 0x00082);
	halrf_wrf(rf, RF_PATH_A, 0x33, 0xf, 0x3);
	halrf_wrf(rf, RF_PATH_A, 0x3e, MASKRF, 0x000c6);
	halrf_wrf(rf, RF_PATH_A, 0x3f, MASKRF, 0x035e7);
	halrf_wrf(rf, RF_PATH_A, 0x33, 0xf, 0xa);
	halrf_wrf(rf, RF_PATH_A, 0x3e, MASKRF, 0x000c6);
	halrf_wrf(rf, RF_PATH_A, 0x3f, MASKRF, 0x035f7);
	halrf_wrf(rf, RF_PATH_A, 0x33, 0xf, 0xb);
	halrf_wrf(rf, RF_PATH_A, 0x3e, MASKRF, 0x000c6);
	halrf_wrf(rf, RF_PATH_A, 0x3f, MASKRF, 0x035ef);
	halrf_wrf(rf, RF_PATH_A, 0xef, BIT(19), 0x0);

	halrf_wrf(rf, RF_PATH_B, 0xef, BIT(19), 0x1);
	halrf_wrf(rf, RF_PATH_B, 0x33, 0xf, 0x1);
	halrf_wrf(rf, RF_PATH_B, 0x3e, MASKRF, 0x00031);
	halrf_wrf(rf, RF_PATH_B, 0x3f, MASKRF, 0x00020);
	halrf_wrf(rf, RF_PATH_B, 0x33, 0xf, 0x3);
	halrf_wrf(rf, RF_PATH_B, 0x3e, MASKRF, 0x00031);
	halrf_wrf(rf, RF_PATH_B, 0x3f, MASKRF, 0x80d79);
	halrf_wrf(rf, RF_PATH_B, 0x33, 0xf, 0xa);
	halrf_wrf(rf, RF_PATH_B, 0x3e, MASKRF, 0x00031);
	halrf_wrf(rf, RF_PATH_B, 0x3f, MASKRF, 0x00d7d);
	halrf_wrf(rf, RF_PATH_B, 0x33, 0xf, 0xb);
	halrf_wrf(rf, RF_PATH_B, 0x3e, MASKRF, 0x00031);
	halrf_wrf(rf, RF_PATH_B, 0x3f, MASKRF, 0x00d7b);
	halrf_wrf(rf, RF_PATH_B, 0xef, BIT(19), 0x0);
}

void halrf_set_dpd_backoff_8832br(struct rf_info *rf, enum phl_phy_idx phy)
{
	struct halrf_dpk_info *dpk = &rf->dpk;
	u8 tx_scale, ofdm_bkof, path, kpath;

	kpath = halrf_kpath_8832br(rf, phy);

	ofdm_bkof = (u8)halrf_rreg(rf, 0x44a0 + (phy << 13), 0x0001F000); /*[16:12]*/
	tx_scale = (u8)halrf_rreg(rf, 0x44a0 + (phy << 13), 0x0000007F); /*[6:0]*/

	if ((ofdm_bkof + tx_scale) >= 44) { /*move dpd backoff to bb, and set dpd backoff to 0*/
		dpk->dpk_gs[phy] = 0x7f;
		for (path = 0; path < 2; path++) {
			if (kpath & BIT(path)) {
				halrf_wreg(rf, 0x81bc + (path << 8), 0x007FFFFF, 0x7f7f7f); /*[22:0]*/
				RF_DBG(rf, DBG_RF_RFK, "[RFK] Set S%d DPD backoff to 0dB\n", path);
			}
		}
	} else
		dpk->dpk_gs[phy] = 0x5b;
}

void halrf_dpk_init_8832br(struct rf_info *rf)
{
	halrf_set_dpd_backoff_8832br(rf, HW_PHY_0);
}

void halrf_set_rxbb_bw_8832br(struct rf_info *rf, enum channel_width bw, enum rf_path path)
{
	halrf_wrf(rf, path, 0xee, BIT(2), 0x1);
	halrf_wrf(rf, path, 0x33, 0x0001F, 0x12); /*[4:0]*/

	if (bw == CHANNEL_WIDTH_20)
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0x1b); /*[5:0]*/
	else if (bw == CHANNEL_WIDTH_40)
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0x13); /*[5:0]*/
	else if (bw == CHANNEL_WIDTH_80)
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0xb); /*[5:0]*/
	else
		halrf_wrf(rf, path, 0x3f, 0x0003F, 0x3); /*[5:0]*/

	RF_DBG(rf, DBG_RF_RFK, "[RFK] set S%d RXBB BW 0x3F = 0x%x\n", path,
		halrf_rrf(rf, path, 0x3f, 0x0003F));

	halrf_wrf(rf, path, 0xee, BIT(2), 0x0);
}

void halrf_rxbb_bw_8832br(struct rf_info *rf, enum phl_phy_idx phy, enum channel_width bw)
{
	u8 kpath, path;

	kpath = halrf_kpath_8832br(rf, phy);
	
	for (path = 0; path < 2; path++) {
		if ((kpath & BIT(path)) && (rf->pre_rxbb_bw[path] != bw)) {
			halrf_set_rxbb_bw_8832br(rf, bw, path);
			rf->pre_rxbb_bw[path] = bw;
		} else
			RF_DBG(rf, DBG_RF_RFK,
			       "[RFK] S%d RXBB BW unchanged (pre_bw = 0x%x)\n",
			       path, rf->pre_rxbb_bw[path]);
	}
}

void halrf_disconnect_notify_8832br(struct rf_info *rf, struct rtw_chan_def *chandef  ) {

	struct halrf_iqk_info *iqk_info = &rf->iqk;
	struct halrf_gapk_info *txgapk_info = &rf->gapk;
	u8 path, ch;
	
	RF_DBG(rf, DBG_RF_RFK, "[IQK]===>%s\n", __func__);
	/*[IQK disconnect]*/
	for (ch = 0; ch < 2; ch++) {
		for (path = 0; path < KPATH; path++) {
			if (iqk_info->iqk_mcc_ch[ch][path] == chandef->center_ch)
				iqk_info->iqk_mcc_ch[ch][path] = 0x0;
		}

	}
	/*TXGAPK*/
	for (ch = 0; ch < 2; ch++) {		
		if (txgapk_info->txgapk_mcc_ch[ch] == chandef->center_ch)
				txgapk_info->txgapk_mcc_ch[ch] = 0x0;
	}
}

bool halrf_check_mcc_ch_8832br(struct rf_info *rf, struct rtw_chan_def *chandef) {

	struct halrf_iqk_info *iqk_info = &rf->iqk;
	u8 path, ch;

	bool check = false;
	RF_DBG(rf, DBG_RF_RFK, "[IQK]===>%s, center_ch(%d)\n", __func__, chandef->center_ch);
	/*[IQK check_mcc_ch]*/
	for (ch = 0; ch < 2; ch++) {
		for (path = 0; path < KPATH; path++) {
			if (iqk_info->iqk_mcc_ch[ch][path] == chandef->center_ch) {
				check = true;
				return check;
			}
		}
	}
	return check;
}

void halrf_fw_ntfy_8832br(struct rf_info *rf, enum phl_phy_idx phy_idx) {
	struct halrf_iqk_info *iqk_info = &rf->iqk;
	u8 i = 0x0;
	u32 data_to_fw[5] = {0};
	u16 len = (u16) (sizeof(data_to_fw) / sizeof(u32))*4;
	
	data_to_fw[0] = (u32) iqk_info->iqk_mcc_ch[0][0];
	data_to_fw[1] = (u32) iqk_info->iqk_mcc_ch[0][1];
	data_to_fw[2] = (u32) iqk_info->iqk_mcc_ch[1][0];
	data_to_fw[3] = (u32) iqk_info->iqk_mcc_ch[1][1];
	data_to_fw[4] = rf->hal_com->band[phy_idx].cur_chandef.center_ch;

	RF_DBG(rf, DBG_RF_RFK, "[IQK] len = 0x%x\n", len);
	for (i =0; i < 5; i++)
		RF_DBG(rf, DBG_RF_RFK, "[IQK] data_to_fw[%x] = 0x%x\n", i, data_to_fw[i]);

	halrf_fill_h2c_cmd(rf, len, FWCMD_H2C_GET_MCCCH, 0xa, H2CB_TYPE_DATA, (u32 *) data_to_fw);	

	return;
}

void halrf_before_one_shot_enable_8832br(struct rf_info *rf) {

	if (halrf_rreg(rf, 0xbff8, MASKBYTE0) != 0x0) {
		halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);

		RF_DBG(rf, DBG_RF_RFK, "======> before set one-shot bit, 0x%x= 0x%x\n", 0x8010, halrf_rreg(rf, 0x8010, MASKDWORD)); 
	}
	
	/* set 0x80d4[21:16]=0x03 (before oneshot NCTL) to get report later */
	//halrf_wreg(rf, 0x80d4, 0x003F0000, 0x03);	 
}


bool halrf_one_shot_nctl_done_check_8832br(struct rf_info *rf, enum rf_path path) {
	
	/* for check status */
	u32 r_bff8 = 0;
	u32 r_80fc = 0;
	bool is_ready = false;
	u16 count = 1;

	rf->nctl_ck_times[0] = 0;
	rf->nctl_ck_times[1] = 0;
	
	/* for 0xbff8 check NCTL DONE */
	while (count < 2000) {	
		r_bff8 = halrf_rreg(rf, 0xbff8, MASKBYTE0);
				
		if (r_bff8 == 0x55) {
			is_ready = true;
			break;
		}	
		halrf_delay_us(rf, 10);
		count++;
	}
	
	halrf_delay_us(rf, 1);
	/* txgapk_info->txgapk_chk_cnt[path][id][0] = count; */
	rf->nctl_ck_times[0] = count;
	
	RF_DBG(rf, DBG_RF_RFK, "======> path = %d, check 0xBFF8[7:0] = 0x%x, IsReady = %d, ReadTimes = %d,delay 1 us\n", path, r_bff8, is_ready, count);

	
	/* for 0x80fc check NCTL DONE */	
	/* for 52A last line 0x9c00 */
	/* for 52B last line 0x8000 */
	count = 1;
	is_ready = false;
	while (count < 2000) {			
		r_80fc = halrf_rreg(rf, 0x80fc, MASKLWORD);
	
		if (r_80fc == NCTL_FINAL_LINE_8832BR) {
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

	return is_ready;
}

bool halrf_do_one_shot_8832br(struct rf_info *rf, enum rf_path path, u32 nctl_addr, u32 mask, u32 process_id)
{

	halrf_before_one_shot_enable_8832br(rf);
	halrf_wreg(rf, nctl_addr, mask, process_id); //set cal_path, process id

	return halrf_one_shot_nctl_done_check_8832br(rf, path);
}

#if 0
bool halrf_set_s0_arfc18_8852br(struct rf_info *rf, u32 val)
{
	u32 temp, c = 0;
	bool timeout = false;

	temp = halrf_rrf(rf, RF_PATH_A,0xb1, MASKRF);
	halrf_wrf(rf, RF_PATH_A, 0xb1, 0x1c0, 0x1);
	halrf_wrf(rf, RF_PATH_A, 0x18, MASKRF, val);
	while (c < 1000) {
		if (halrf_rrf(rf, RF_PATH_A, 0xb7, BIT(8)) == 0)
			break;
		c++;
		halrf_delay_us(rf, 1);
	}
		halrf_wrf(rf, RF_PATH_A, 0xb1, MASKRF, temp);
	if (c == 1000) {
		timeout = true;
		RF_WARNING("[LCK]LCK timeout\n");
	}
	return timeout;
}
#endif

void halrf_lck_8832br(struct rf_info *rf)
{
	/*lck must be called by watchdog*/
	u32 temp05[2], temp18, path, i;

	RF_DBG(rf, DBG_RF_LCK, "[LCK]DO LCK\n");

	temp05[0] = halrf_rrf(rf, 0, 0x5, MASKRF);
	temp05[1] = halrf_rrf(rf, 1, 0x5, MASKRF);
	temp18 = halrf_rrf(rf, 0, 0x18, MASKRF);
	halrf_wrf(rf, 0, 0x0, MASKRFMODE, 0x3);
	halrf_wrf(rf, 1, 0x0, MASKRFMODE, 0x3);
	halrf_wrf(rf, 0, 0x5, MASKRF, 0x0);
	halrf_wrf(rf, 1, 0x5, MASKRF, 0x0);
	halrf_wrf(rf, 0, 0x0, MASKRFMODE, 0x3);
	halrf_wrf(rf, 1, 0x0, MASKRFMODE, 0x3);
	halrf_wrf(rf, 0, 0xd3, BIT(8), 0x1);
//	halrf_set_s0_arfc18_8832br(rf, temp18);
	halrf_set_ch_8832br(rf, temp18);
	halrf_wrf(rf, 0, 0xd3, BIT(8), 0x0);
	halrf_wrf(rf, 0, 0x5, MASKRF, temp05[0]);
	halrf_wrf(rf, 1, 0x5, MASKRF, temp05[1]);

	rf->lck_ther_s0 = rf->cur_ther_s0;
	rf->lck_ther_s1 = rf->cur_ther_s1;
}

void halrf_lck_tracking_8832br(struct rf_info *rf)
{
	u32 c = 0, temp;
	u32 i, ther = 0, ther_avg = 0;

	halrf_do_lck_check_8832br(rf);
	
	if (((rf->lck_ther_s0 == 0) && (rf->lck_ther_s1 == 0))) {
		halrf_lck_trigger_g6(rf);
		return;
	}

	if (HALRF_ABS(rf->lck_ther_s0, rf->cur_ther_s0) >
		HALRF_ABS(rf->lck_ther_s1, rf->cur_ther_s1)) {
		temp = HALRF_ABS(rf->lck_ther_s0, rf->cur_ther_s0);
	}else {
		temp = HALRF_ABS(rf->lck_ther_s1, rf->cur_ther_s1);
	}

	RF_DBG(rf, DBG_RF_LCK, "[LCK]MAX_delta = %d, LCK_TH=%x\n",
		temp, LCK_TH_8832BR);

	if(temp >= LCK_TH_8832BR)		
		halrf_lck_trigger_g6(rf);
#if 0
	/*thermal FIFO*/
	for (i = 0; i < 3; i++) {
		temp [3 - i] = temp[2 - i];
	}
	temp[0] = rf->cur_ther;
	if (c < 4)
		c++;
	else
		c = 4;

	for (i = 0 ; i < 4; i++){
		ther += temp[i];
	}

	RF_DBG(rf, DBG_RF_LCK, "[LCK]temp[i]=0x%x, 0x%x, 0x%x, 0x%x\n",
		temp[0], temp[1], temp[2], temp[3]);

	/*avg*/
	if(c)
		ther_avg = ther / c;
	RF_DBG(rf, DBG_RF_LCK, "[LCK]pre_lck_ther = 0x%x,ther_avg=0x%x\n", rf->lck_ther, ther_avg);

	if(HALRF_ABS(rf->lck_ther, (u8)ther_avg) >= LCK_TH_8832BR)		
		halrf_lck_trigger_g6(rf);
#endif
}

static u32 backup_mac_reg_8832br[] = {0x0};
static u32 backup_bb_reg_8832br[] = {0x2344,0xc0d4,0xc0d8, 0xc0c4, 0xc0e8, 0xc0ec, 0xc1d4, 0xc1d8, 0xc1c4, 0xc1e8, 0xc1ec};
static u32 backup_rf_reg_8832br[] = {0xdc, 0xef, 0xde, 0x0, 0x1e, 0x5, 0x10005};

#if 1
static struct halrf_iqk_ops iqk_ops= {
    .iqk_kpath = halrf_kpath_8832br,
    .iqk_mcc_page_sel = iqk_mcc_page_sel_8832br,
    .iqk_get_ch_info = iqk_get_ch_info_8832br,
    .iqk_preset = iqk_preset_8832br,
    .iqk_macbb_setting = iqk_macbb_setting_8832br,
    .iqk_start_iqk = iqk_start_iqk_8832br,
    .iqk_restore = iqk_restore_8832br,
    .iqk_afebb_restore = iqk_afebb_restore_8832br,  
};

struct rfk_iqk_info rf_iqk_hwspec_8832br = {
  	.rf_iqk_ops = &iqk_ops,
	.rf_max_path_num = 2,
	.rf_iqk_version = iqk_version_8832br,
	.rf_iqk_ch_num = 2,
	.rf_iqk_path_num = 2,

#if 0
	.backup_mac_reg = backup_mac_reg_8832br,
	.backup_mac_reg_num = ARRAY_SIZE(backup_mac_reg_8832br),
#else
	.backup_mac_reg = backup_mac_reg_8832br,
	.backup_mac_reg_num = 0,
#endif

    	.backup_bb_reg = backup_bb_reg_8832br,
    	.backup_bb_reg_num = ARRAY_SIZE(backup_bb_reg_8832br),
    	.backup_rf_reg = backup_rf_reg_8832br,
    	.backup_rf_reg_num = ARRAY_SIZE(backup_rf_reg_8832br),
};

#endif
#endif
