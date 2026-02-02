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

/*8832BR DPK ver:0xa 20220420*/

void _dpk_bkup_kip_8832br(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[][DPK_KIP_REG_NUM_8832BR],
	u8 path)
{
	u8 i;

	for (i = 0; i < DPK_KIP_REG_NUM_8832BR; i++) {
		reg_bkup[path][i] = halrf_rreg(rf, reg[i] + (path << 8), MASKDWORD);
		if (DPK_REG_DBG_8832BR)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Backup 0x%x = %x\n", reg[i]+ (path << 8), reg_bkup[path][i]);
	}
}

void _dpk_bkup_bb_8832br(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[DPK_BB_REG_NUM_8832BR])
{
	u8 i;

	for (i = 0; i < DPK_BB_REG_NUM_8832BR; i++) {
		reg_bkup[i] = halrf_rreg(rf, reg[i], MASKDWORD);
		if (DPK_REG_DBG_8832BR)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Backup 0x%x = %x\n", reg[i], reg_bkup[i]);
	}
}

void _dpk_bkup_rf_8832br(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 rf_bkup[][DPK_RF_REG_NUM_8832BR],
	u8 path)
{
	u8 i;

	for (i = 0; i < DPK_RF_REG_NUM_8832BR; i++) {
		rf_bkup[path][i] = halrf_rrf(rf, path, rf_reg[i], MASKRF);
		if (DPK_REG_DBG_8832BR)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Backup RF S%d 0x%x = %x\n",
				path, rf_reg[i], rf_bkup[path][i]);
	}
}

void _dpk_reload_kip_8832br(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[][DPK_KIP_REG_NUM_8832BR],
	u8 path) 
{
	u8 i;

	for (i = 0; i < DPK_KIP_REG_NUM_8832BR; i++) {
		halrf_wreg(rf, reg[i] + (path << 8), MASKDWORD, reg_bkup[path][i]);
		if (DPK_REG_DBG_8832BR)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Reload 0x%x = %x\n", reg[i] + (path << 8),
				   reg_bkup[path][i]);
	}
}

void _dpk_reload_bb_8832br(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[DPK_BB_REG_NUM_8832BR]) 
{
	u8 i;

	for (i = 0; i < DPK_BB_REG_NUM_8832BR; i++) {
		halrf_wreg(rf, reg[i], MASKDWORD, reg_bkup[i]);
		if (DPK_REG_DBG_8832BR)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Reload 0x%x = %x\n", reg[i],
				   reg_bkup[i]);
	}
}

void _dpk_reload_rf_8832br(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 rf_bkup[][DPK_RF_REG_NUM_8832BR],
	u8 path)
{
	u8 i;

	for (i = 0; i < DPK_RF_REG_NUM_8832BR; i++) {
		halrf_wrf(rf, path, rf_reg[i], MASKRF, rf_bkup[path][i]);
		if (DPK_REG_DBG_8832BR)
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Reload RF S%d 0x%x = %x\n",
				path, rf_reg[i], rf_bkup[path][i]);
	}
}

u8 _dpk_one_shot_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	enum dpk_id id)
{
	u8 phy_map;
	u16 dpk_cmd = 0x0, cnt1 = 0, cnt2 = 0;
	u32 r_bff8 = 0x0, r_80fc = 0x0;

	phy_map = (BIT(phy) << 4) | BIT(path);

	dpk_cmd = (u16)((id << 8) | (0x19 + path * 0x12));
#if 0
	halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_DPK, RFK_ONESHOT_START);
#endif
	halrf_wreg(rf, 0x8000, MASKDWORD, dpk_cmd);

	r_bff8 = halrf_rreg(rf, 0xbff8, MASKBYTE0);
	while (r_bff8 != 0x55 && cnt1 < 2000) {
		halrf_delay_us(rf, 10);
		r_bff8 = halrf_rreg(rf, 0xbff8, MASKBYTE0);
		cnt1++;
	}

	halrf_delay_us(rf, 1);

	r_80fc = halrf_rreg(rf, 0x80fc, MASKLWORD);
	while (r_80fc != 0x8000 && cnt2 < 2000) {
		halrf_delay_us(rf, 1);
		r_80fc = halrf_rreg(rf, 0x80fc, MASKLWORD);
		cnt2++;
	}

	halrf_wreg(rf, 0x8010, MASKBYTE0, 0x0);
#if 0
	halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_DPK, RFK_ONESHOT_STOP);
#endif
	RF_DBG(rf, DBG_RF_DPK, "[DPK] one-shot for %s = 0x%04x (cnt:%d/%d)\n",
	       id == 0x28 ? "KIP_PRESET" : (id == 0x29 ? "DPK_TXAGC" :
	       (id == 0x2a ? "DPK_RXAGC" : (id == 0x2b ? "SYNC" :
	       	(id == 0x2c ? "GAIN_LOSS" : (id == 0x2d ? "MDPK_IDL" :
		(id == 0x2f ? "DPK_GAIN_NORM" : (id == 0x31 ? "KIP_RESOTRE" :
		"Unknown id"))))))), dpk_cmd, cnt1, cnt2);

	if (cnt1 == 2000 || cnt2 == 2000) {
		RF_DBG(rf, DBG_RF_DPK, "[DPK] one-shot timeout!!!!\n");
		return 1;
	} else
		return 0;
}

void _dpk_init_8832br(
	struct rf_info *rf,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 kidx = dpk->cur_idx[path];

	dpk->bp[path][kidx].path_ok = 0;
	dpk->rek_cnt[path][0] = 0;
	dpk->rek_cnt[path][1] = 0;

	dpk->rek_chk[path][0][0] = 0;
	dpk->rek_chk[path][0][1] = 0;
	dpk->rek_chk[path][0][2] = 0;
	dpk->rek_chk[path][0][3] = 0;
	dpk->rek_chk[path][0][4] = 0;

	dpk->rek_chk[path][1][0] = 0;
	dpk->rek_chk[path][1][1] = 0;
	dpk->rek_chk[path][1][2] = 0;
	dpk->rek_chk[path][1][3] = 0;
	dpk->rek_chk[path][1][4] = 0;
}

void _dpk_information_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 kidx = dpk->cur_idx[path];

	dpk->bp[path][kidx].band = rf->hal_com->band[phy].cur_chandef.band;
	dpk->bp[path][kidx].ch = rf->hal_com->band[phy].cur_chandef.center_ch;
	dpk->bp[path][kidx].bw = rf->hal_com->band[phy].cur_chandef.bw;

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d[%d] (PHY%d): TSSI %s/ DBCC %s/ %s/ CH%d/ %s\n",
	       path, dpk->cur_idx[path], phy, rf->is_tssi_mode[path] ? "on" : "off",
	       rf->hal_com->dbcc_en ? "on" : "off",
	       dpk->bp[path][kidx].band == 0 ? "2G" : (dpk->bp[path][kidx].band == 1 ? "5G" : "6G"),
	       dpk->bp[path][kidx].ch,
	       dpk->bp[path][kidx].bw == 0 ? "20M" : (dpk->bp[path][kidx].bw == 1 ? "40M" : "80M"));
}

void _dpk_rxagc_onoff_8832br(
	struct rf_info *rf,
	enum rf_path path,
	bool turn_on)
{
	if (path == RF_PATH_A)
		halrf_wreg(rf, 0x4730, BIT(31), turn_on);
	else
		halrf_wreg(rf, 0x4a9c, BIT(31), turn_on);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d RXAGC is %s\n", path,
		turn_on ? "turn_on" : "turn_off");
}

void _dpk_bb_afe_setting_8832br(
	struct rf_info *rf,
	enum rf_path path)
{
	struct rtw_hal_com_t *hal_i = rf->hal_com;

	/*1. Keep ADC_fifo reset*/
	halrf_wreg(rf, 0x20fc, BIT(16 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(20 + path), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(24 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28 + path), 0x0);
	/*2. BB for IQK DBG mode*/
	halrf_wreg(rf, 0x5670 + (path << 13), MASKDWORD, 0xd801dffd); /*bit13 for gapk_offset*/
	/*3.Set DAC clk*/
	halrf_txck_force_8832br(rf, path, true, DAC_960M);
	/*4. Set ADC clk*/
	if (hal_i->cv == CAV) {
		halrf_wreg(rf, 0x12a0 | (path <<13), BIT(19), 0x0);
		halrf_wreg(rf, 0x12a0 | (path <<13), 0x70000, 3);
		halrf_wreg(rf, 0x12a0 | (path <<13), BIT(19), 0x1);
		halrf_wreg(rf, 0xc0d4 + (path << 8), BIT(27) | BIT(26), 0x0);
		halrf_wreg(rf, 0xc0d8 + (path << 8), 0x000001E0, 0x7); /*[8:5]*/
	} else {
		halrf_rxck_force_8832br(rf, path, true, ADC_1920M);
	}
	//halrf_wreg(rf, 0x5670 + (path << 13), BIT(30) | BIT(29), 0x2);

//	halrf_wreg(rf, 0xc0d4 + (path << 8), BIT(27) | BIT(26), 0x0);
//	halrf_wreg(rf, 0xc0d8 + (path << 8), 0x000001E0, 0x7); /*[8:5]*/
	halrf_wreg(rf, 0xc0ec + (path << 8), BIT(14) | BIT(13), 0x0); /*force CBW_idx 0*/
	halrf_wreg(rf, 0x12b8 + (path << 13), BIT(30), 0x1);
	halrf_wreg(rf, 0x030c, MASKBYTE3, 0x1f);
	halrf_wreg(rf, 0x030c, MASKBYTE3, 0x13);
	halrf_wreg(rf, 0x032c, MASKHWORD, 0x0001);
	halrf_wreg(rf, 0x032c, MASKHWORD, 0x0041);
	/*5. ADDA fifo rst*/
	halrf_wreg(rf, 0x20fc, BIT(20 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28 + path), 0x1);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d BB/AFE setting\n", path);
}

void _dpk_bb_afe_restore_8832br(
	struct rf_info *rf,
	enum rf_path path)
{
	halrf_wreg(rf, 0x12b8 + (path << 13), BIT(30), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(16 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(20 + path), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(24 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28 + path), 0x0);
	halrf_wreg(rf, 0x5670 + (path << 13), MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x12a0 + (path << 13), 0x000FF000, 0x00); /*[19:12]*/
	halrf_wreg(rf, 0x20fc, BIT(16 + path), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(24 + path), 0x0);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d BB/AFE restore\n", path);
}

void _dpk_tssi_pause_8832br(
	struct rf_info *rf,
	enum rf_path path,
	bool is_pause)
{
	halrf_wreg(rf, 0x5818 + (path << 13), BIT(30), is_pause);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d TSSI %s\n", path,
	       is_pause ? "pause" : "resume");
}

void _dpk_tpg_sel_8832br(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	if (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_160) {
		halrf_wreg(rf, 0x806c, BIT(2) | BIT (1), 0x3);
		halrf_wreg(rf, 0x8068, MASKDWORD, 0x0180ff30);
	} else if (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_80) {
		halrf_wreg(rf, 0x806c, BIT(2) | BIT (1), 0x0);
		halrf_wreg(rf, 0x8068, MASKDWORD, 0xffe0fa00);
	} else if (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_40) {
		halrf_wreg(rf, 0x806c, BIT(2) | BIT (1), 0x2);
		halrf_wreg(rf, 0x8068, MASKDWORD, 0xff4009e0);
	} else {
		halrf_wreg(rf, 0x806c, BIT(2) | BIT (1), 0x1);
		halrf_wreg(rf, 0x8068, MASKDWORD, 0xf9f007d0);
	}

	RF_DBG(rf, DBG_RF_DPK, "[DPK] TPG Select for %s\n",
	       dpk->bp[path][kidx].bw == CHANNEL_WIDTH_160 ? "160M" :
	       (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_80 ? "80M" : 
	       (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_40 ? "40M" : "20M")));
}

void _dpk_txpwr_bb_force_8832br(
	struct rf_info *rf,
	enum rf_path path,
	bool is_force)
{
	halrf_wreg(rf, 0x56cc + (path << 13), BIT(28), is_force); /*txpwr_bb_force_on*/
	halrf_wreg(rf, 0x580c + (path << 13), BIT(15), is_force); /*txpwr_bb_force_rdy*/

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d txpwr_bb_force %s\n", path, is_force ? "on" : "off");
}

void _dpk_kip_pwr_clk_onoff_8832br(
	struct rf_info *rf,
	bool turn_on)
{
	if (turn_on) {
		halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000080); /*cip power on*/
		halrf_wreg(rf, 0x8088, MASKDWORD, 0x807f030a); /*320M*/
	} else {
		halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000000);
		halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000000);
		halrf_wreg(rf, 0x80f4, BIT(18), 0x1);
	}

	//RF_DBG(rf, DBG_RF_DPK, "[DPK] KIP Power/CLK is %s\n", turn_on ? "turn_on" : "turn_off");
}

void _dpk_kip_control_rfc_8832br(
	struct rf_info *rf,
	enum rf_path path,
	bool ctrl_by_kip)
{
	halrf_wreg(rf, 0x5670, BIT(1), ctrl_by_kip); /*KIP control RFC*/

	RF_DBG(rf, DBG_RF_DPK, "[DPK] RFC is controlled by %s\n", ctrl_by_kip ? "KIP" : "BB");
}

void _dpk_kip_preset_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	/*must before kip control RFC*/
	halrf_wreg(rf, 0x8078, 0x000FFFFF, halrf_rrf(rf, path, 0x00, MASKRF)); /*[19:0]*/
	//RF_DBG(rf, DBG_RF_DPK, "[DPK] 0x8078 = 0x%x\n", halrf_rreg(rf, 0x8078, MASKDWORD));

	halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2), 0x00003F00, 0x02); /*[13:8] slope*/

	_dpk_kip_control_rfc_8832br(rf, path, true);
	halrf_wreg(rf, 0x8104 + (path << 8), BIT(8), kidx); /*ch0/ch1 selection*/

	_dpk_one_shot_8832br(rf, phy, path, D_KIP_PRESET);
}

void _dpk_kip_restore_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path)
{
	_dpk_one_shot_8832br(rf, phy, path, D_KIP_RESTORE);

	/*cip power on*/
	//halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000000);
	/*CFIR CLK restore*/
	//halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000000);

	//_dpk_kip_control_rfc_8832br(rf, path, false);
	_dpk_txpwr_bb_force_8832br(rf, path, false);
#if 0
	if (rf->hal_com->cv > 0x0) /*hw txagc_offset*/
		halrf_wreg(rf, 0x81c8 + (path << 8), BIT(15), 0x1);
#endif
	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d restore KIP\n", path);
}

u8 _dpk_dbm_convert_8832br(
	struct rf_info *rf,
	u8 dbm)
{
	u8 txagc_cw;

	txagc_cw = (dbm - 16) * 4 + 0x40; /*0.25dB/step*/

	RF_DBG(rf, DBG_RF_DPK, "[DPK] convert %ddBm to 0x%x\n", dbm, txagc_cw);

	return txagc_cw;
}

void _dpk_read_rxsram_8832br(
	struct rf_info *rf)
{
	u32 addr;

	halrf_wreg(rf, 0x80e8, BIT(7), 0x1);	/*web_iqrx*/
	halrf_wreg(rf, 0x8074, BIT(31), 0x1);	/*rxsram_ctrl_sel*/
	halrf_wreg(rf, 0x80d4, MASKDWORD, 0x00020000);	/*rpt_sel*/

	for (addr = 0; addr < 0x200; addr++) {
		halrf_wreg(rf, 0x80d8, MASKDWORD, 0x00010000 | addr);
		RF_DBG(rf, DBG_RF_DPK, "[DPK] RXSRAM[%03d] = 0x%x\n", addr,
			halrf_rreg(rf, 0x80fc, MASKDWORD));
	}
	halrf_wreg(rf, 0x80e8, BIT(7), 0x0);	/*web_iqrx*/
	halrf_wreg(rf, 0x8074, BIT(31), 0x0);	/*rxsram_ctrl_sel*/
}

void _dpk_rf_reg_query_8832br(
	struct rf_info *rf,
	enum rf_path path,
	u32 reg)
{
#if 1
	u32 ori_ctrl = halrf_rreg(rf, 0x5670 + (path << 13), BIT(1));

	_dpk_kip_control_rfc_8832br(rf, path, false);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] RF 0x%x = 0x%x\n", reg,
		halrf_rrf(rf, path, reg, MASKRF));

	_dpk_kip_control_rfc_8832br(rf, path, (bool)ori_ctrl);
#endif
}

void _dpk_kset_query_8832br(
	struct rf_info *rf,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	halrf_wreg(rf, 0x81d4 + (path << 8), 0x003F0000, 0x10);	/*rpt_sel*/

	dpk->cur_k_set = (u8)(halrf_rreg(rf, 0x81fc + (path << 8), 0xE0000000) - 1); /*[31:29]*/

	//RF_DBG(rf, DBG_RF_DPK, "[DPK] cur k_set = %d\n", dpk->cur_k_set);
}

void _dpk_para_query_8832br(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	/*[31:26]:t-meter, [25:16]:txagc_bb, [15:7]:txagc, [6:0]:gs*/
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};
	u32 para;

	para = halrf_rreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), MASKDWORD);

	dpk->bp[path][kidx].txagc_dpk = (para >> 10) & 0x3f;
	dpk->bp[path][kidx].ther_dpk = (para >> 26) & 0x3f;

	RF_DBG(rf, DBG_RF_DPK, "[DPK] thermal/ txagc_RF (K%d) = 0x%x/ 0x%x\n",
		dpk->cur_k_set, dpk->bp[path][kidx].ther_dpk, dpk->bp[path][kidx].txagc_dpk);
#if 0
	if (kidx == 0) { /*CH0*/
		RF_DBG(rf, DBG_RF_DPK, "[DPK] CH0_K0= 0x%x\n",
			halrf_rreg(rf, 0x8190 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[DPK] CH0_K1= 0x%x\n",
			halrf_rreg(rf, 0x8194 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[DPK] CH0_K2= 0x%x\n",
			halrf_rreg(rf, 0x8198 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[DPK] CH0_K3= 0x%x\n",
			halrf_rreg(rf, 0x81a4 + (path << 8), MASKDWORD));
	} else { /*CH1*/
		RF_DBG(rf, DBG_RF_DPK, "[DPK] CH0_K0= 0x%x\n",
			halrf_rreg(rf, 0x81a8 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[DPK] CH0_K1= 0x%x\n",
			halrf_rreg(rf, 0x81c4 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[DPK] CH0_K2= 0x%x\n",
			halrf_rreg(rf, 0x81c8 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[DPK] CH0_K3= 0x%x\n",
			halrf_rreg(rf, 0x81e8 + (path << 8), MASKDWORD));
	}
#endif
}

bool _dpk_sync_check_8832br(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u16 dc_i, dc_q;
	u8 corr_val, corr_idx, rxbb;

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x0);	/*rpt_sel*/

	corr_idx = (u8)halrf_rreg(rf, 0x80fc, 0x000000ff);
	corr_val = (u8)halrf_rreg(rf, 0x80fc, 0x0000ff00);

	dpk->corr_idx[path][kidx] = corr_idx;
	dpk->corr_val[path][kidx] = corr_val;

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x9);	/*rpt_sel*/

	dc_i = (u16)halrf_rreg(rf, 0x80fc, 0x0fff0000); /*[27:16]*/
	dc_q = (u16)halrf_rreg(rf, 0x80fc, 0x00000fff); /*[11:0]*/

	if (dc_i >> 11 == 1)
		dc_i = 0x1000 - dc_i;
	if (dc_q >> 11 == 1)
		dc_q = 0x1000 - dc_q;

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d Corr_idx/ Corr_val /DC I/Q, = %d / %d / %d / %d\n",
		path, corr_idx, corr_val, dc_i, dc_q);

	dpk->dc_i[path][kidx] = dc_i;
	dpk->dc_q[path][kidx] = dc_q;

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x8);	/*rpt_sel*/

	rxbb = (u8)halrf_rreg(rf, 0x80fc, 0x0000003F);	/*[5:0]*/

	//_dpk_rf_reg_query_8832br(rf, path, 0x00);

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x31);	/*rpt_sel*/

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d RXBB/ RXAGC_done /RXBB_ovlmt = %d / %d / %d\n",
		path, rxbb,
		halrf_rreg(rf, 0x80fc, BIT(0)),
		halrf_rreg(rf, 0x80fc, BIT(8)));

	if ((dc_i > 200) || (dc_q > 200) || (corr_val < 160))
		return true;
	else
		return false;
}

void _dpk_kip_set_txagc_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 dbm,
	bool set_from_bb)
{	
	if (set_from_bb) {
		if (dbm >= 24)
			dbm = 24;
		else if (dbm <= 7)
			dbm = 7;
		RF_DBG(rf, DBG_RF_DPK, "[DPK] set S%d txagc to %ddBm\n", path, dbm);
		halrf_wreg(rf, 0x56cc + (path << 13), 0x0FF80000, dbm << 2); /*[27:19]*/
	}

	_dpk_one_shot_8832br(rf, phy, path, D_TXAGC);
	_dpk_kset_query_8832br(rf, path);
	//_dpk_rf_reg_query_8832br(rf, path, 0x11);
}

bool _dpk_kip_set_rxagc_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	_dpk_kip_control_rfc_8832br(rf, path, false);
	halrf_wreg(rf, 0x8078, 0x000FFFFF, halrf_rrf(rf, path, 0x00, MASKRF)); /*[19:0]*/
	_dpk_kip_control_rfc_8832br(rf, path, true);

	_dpk_one_shot_8832br(rf, phy, path, D_RXAGC);
#if 0
	halrf_wreg(rf, 0x80d4, 0x000F0000, 0x8);
	RF_DBG(rf, DBG_RF_DPK, "[DPK] set RXBB = 0x%x\n",
		halrf_rreg(rf, 0x80fc, 0x0000001F));
#endif
	return _dpk_sync_check_8832br(rf, path, kidx);
}

void _dpk_lbk_rxiqk_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	bool is_bypass)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 i, cur_rxbb;

	halrf_wreg(rf, 0x81a0 + (path << 8), BIT(7), 0x1);

	if (is_bypass) {
		halrf_wreg(rf, 0x813c + (path << 8), MASKDWORD, 0x40000002);
		RF_DBG(rf, DBG_RF_DPK, "[DPK] Bypass RXIQC\n");
		return;
	}

	halrf_wreg(rf, 0x8074, BIT(31), 0x1); /*RxSRAM_ctrl_sel 0:MDPK; 1:IQK*/

	_dpk_kip_control_rfc_8832br(rf, path, false);

	cur_rxbb = (u8)halrf_rrf(rf, path, 0x00, MASKRFRXBB);

	/*RF setting*/
	halrf_wrf(rf, path, 0x1f, MASKRF, halrf_rrf(rf, path, 0x18, MASKRF));
	halrf_wrf(rf, path, 0x00, MASKRFMODE, 0xd);
	halrf_wrf(rf, path, 0x20, BIT(5), 0x1); /*IQKPLL_EN_BCN_A*/

	if (cur_rxbb >= 0x11)
		halrf_wrf(rf, path, 0x98, 0x0000007F, 0x13); /*[6:0] DPK_RXIQK Att*/
	else if (cur_rxbb <= 0xa)
		halrf_wrf(rf, path, 0x98, 0x0000007F, 0x00); /*[6:0] DPK_RXIQK Att*/
	else
		halrf_wrf(rf, path, 0x98, 0x0000007F, 0x05); /*[6:0] DPK_RXIQK Att*/

	halrf_wrf(rf, path, 0x85, BIT(1) | BIT(0), 0x0); /*lower LNA LDO out*/

	halrf_wrf(rf, path, 0x1e, BIT(19), 0x0);
	halrf_wrf(rf, path, 0x1e, MASKRF, 0x80014); /*POW IQKPLL, 9.25MHz offset for IQKPLL*/

	for (i = 0; i < 7; i++)
		halrf_delay_us(rf, 10); /*IQKPLL's settling time*/

	_dpk_kip_control_rfc_8832br(rf, path, true);

	halrf_wreg(rf, 0x802c, 0x0FFF0000, 0x025); /*[27:16] Rx_tone_idx=0x025 (9.25MHz)*/

	_dpk_one_shot_8832br(rf, phy, path, LBK_RXIQK);

	dpk->dpk_rxiqc[path] = halrf_rreg(rf, 0x813c + (path << 8), MASKDWORD);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d LBK RXIQC = 0x%x\n", path, dpk->dpk_rxiqc[path]);

	_dpk_kip_control_rfc_8832br(rf, path, false);

	halrf_wrf(rf, path, 0x20, BIT(5), 0x0); /*disable RXIQK PLL*/

	halrf_wreg(rf, 0x8074, BIT(31), 0x0); /*RxSRAM_ctrl_sel 0:MDPK; 1:IQK*/
	halrf_wreg(rf, 0x80d0, BIT(21) | BIT(20), 0x0);
	halrf_wreg(rf, 0x81dc + (path << 8), BIT(1), 0x1); /*auto*/
	halrf_wrf(rf, path, 0x00, MASKRFMODE, 0x5);

	_dpk_kip_control_rfc_8832br(rf, path, true);
}

void _dpk_get_thermal_8832br(struct rf_info *rf, u8 kidx, enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};

	dpk->bp[path][kidx].ther_dpk = halrf_get_thermal_8832br(rf, path);

	halrf_wreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0xFC000000,
			dpk->bp[path][kidx].ther_dpk); /*[31:26]*/

	RF_DBG(rf, DBG_RF_DPK, "[DPK] thermal@DPK (by RFC)= 0x%x\n", dpk->bp[path][kidx].ther_dpk);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] 0x%x = 0x%x\n", reg[kidx][dpk->cur_k_set] + (path << 8),
		halrf_rreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), MASKDWORD));
}

void _dpk_rf_setting_8832br(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	halrf_wrf(rf, path, 0x00, MASKRF, 0x50121);
	halrf_wrf(rf, path, 0x10000, MASKRFMODE, RF_DPK);
	/*TIA*/
	halrf_wrf(rf, path, 0xdf, BIT(12), 0x1);
	halrf_wrf(rf, path, 0x9e, BIT(8), 0x1);
	/*switch + att*/
	halrf_wrf(rf, path, 0x8c, 0x0FE00, 0x5); /*[15:9]*/
	/*RXCIM3*/
	halrf_wrf(rf, path, 0x8b, MASKRF, 0x920FC);
	halrf_wrf(rf, path, 0x90, MASKRF, 0x002C0);
	halrf_wrf(rf, path, 0x96, MASKRF, 0x0007b);
	halrf_wrf(rf, path, 0x97, MASKRF, 0x38800);
	/*debug rtxbw*/
	halrf_wrf(rf, path, 0xde, BIT(2), 0x1);
	/*txbb filter*/
	if (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_80)
		halrf_wrf(rf, path, 0x1a, BIT(14) | BIT(13) | BIT(12), 0x5);
	else
		halrf_wrf(rf, path, 0x1a, BIT(14) | BIT(13) | BIT(12), dpk->bp[path][kidx].bw + 1);
	/*rxbb filter*/
	halrf_wrf(rf, path, 0x1a, BIT(11) | BIT(10), 0x0);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] RF 0x0/0x83/0x9e/0x1a/0xdf/0x1001a = 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x\n",
	       halrf_rrf(rf, path, 0x00, MASKRF),
	       halrf_rrf(rf, path, 0x83, MASKRF),
	       halrf_rrf(rf, path, 0x9e, MASKRF),
	       halrf_rrf(rf, path, 0x1a, MASKRF),
	       halrf_rrf(rf, path, 0xdf, MASKRF),
	       halrf_rrf(rf, path, 0x1001a, MASKRF));
}

void _dpk_bypass_rxiqc_8832br(
	struct rf_info *rf,
	enum rf_path path)
{
	halrf_wreg(rf, 0x81a0 + (path << 8), BIT(7), 0x1);
	halrf_wreg(rf, 0x813c + (path << 8), MASKDWORD, 0x40000002);
	RF_DBG(rf, DBG_RF_DPK, "[DPK] Bypass RXIQC\n");
}

u16 _dpk_dgain_read_8832br(
	struct rf_info *rf)
{
	u16 dgain = 0x0;

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x0);	/*rpt_sel*/

	dgain = (u16)halrf_rreg(rf, 0x80fc, 0x0FFF0000);	/*[27:16]*/

	RF_DBG(rf, DBG_RF_DPK, "[DPK] DGain = 0x%x\n", dgain);

	return dgain;
}

s8 _dpk_dgain_mapping_8832br(
	struct rf_info *rf,
	u16 dgain)
{
	u16 bnd[15] = {0xbf1, 0xaa5, 0x97d, 0x875, 0x789,
			0x6b7, 0x5fc, 0x556, 0x4c1, 0x43d,
			0x3c7, 0x35e, 0x2ac, 0x262, 0x220};
	s8 offset = 0;

	if (dgain >= bnd[0])
		offset = 0x6;
	else if ((bnd[0] > dgain) && (dgain >= bnd[1]))
		offset = 0x6;
	else if ((bnd[1] > dgain) && (dgain >= bnd[2]))
		offset = 0x5;
	else if ((bnd[2] > dgain) && (dgain >= bnd[3]))
		offset = 0x4;
	else if ((bnd[3] > dgain) && (dgain >= bnd[4]))
		offset = 0x3;
	else if ((bnd[4] > dgain) && (dgain >= bnd[5]))
		offset = 0x2;
	else if ((bnd[5] > dgain) && (dgain >= bnd[6]))
		offset = 0x1;
	else if ((bnd[6] > dgain) && (dgain >= bnd[7]))
		offset = 0x0;
	else if ((bnd[7] > dgain) && (dgain >= bnd[8]))
		offset = 0xff;
	else if ((bnd[8] > dgain) && (dgain >= bnd[9]))
		offset = 0xfe;
	else if ((bnd[9] > dgain) && (dgain >= bnd[10]))
		offset = 0xfd;
	else if ((bnd[10] > dgain) && (dgain >= bnd[11]))
		offset = 0xfc;
	else if ((bnd[11] > dgain) && (dgain >= bnd[12]))
		offset = 0xfb;
	else if ((bnd[12] > dgain) && (dgain >= bnd[13]))
		offset = 0xfa;
	else if ((bnd[13] > dgain) && (dgain >= bnd[14]))
		offset = 0xf9;
	else if (bnd[14] > dgain)
		offset = 0xf8;
	else
		offset = 0x0;

	//RF_DBG(rf, DBG_RF_DPK, "[DPK] DGain offset = %d\n", offset);

	return offset;
}

u8 _dpk_pas_check_8832br(
	struct rf_info *rf)
{
	u8 fail = 0;

	halrf_wreg(rf, 0x80d4, MASKBYTE2, 0x06); /*0x80d6, ctrl_out_Kpack*/
	halrf_wreg(rf, 0x80bc, BIT(14), 0x0);	/*query status*/
	halrf_wreg(rf, 0x80c0, MASKBYTE2, 0x08);

	halrf_wreg(rf, 0x80c0, MASKBYTE3, 0x00); /*0x80C3*/
	if (halrf_rreg(rf, 0x80fc, MASKHWORD) == 0x0800) {
		fail = 1;
		RF_DBG(rf, DBG_RF_DPK, "[DPK] PAS check Fail!!\n");
	}
	
	return fail;
}

u8 _dpk_gainloss_read_8832br(
	struct rf_info *rf)
{
	u8 result;

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x6);	/*rpt_sel*/
	halrf_wreg(rf, 0x80bc, BIT(14), 0x1);		/*query status*/

	result = (u8)halrf_rreg(rf, 0x80fc, 0x000000F0); /*[7:4]*/

	RF_DBG(rf, DBG_RF_DPK, "[DPK] tmp GL = %d\n", result);

	return result;
}

u8 _dpk_gainloss_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	_dpk_one_shot_8832br(rf, phy, path, D_GAIN_LOSS);
	_dpk_kip_set_txagc_8832br(rf, phy, path, 0xff, false); /*set new tx power*/

	/*for k4 hw_bug*/
	halrf_wreg(rf, 0x81f0 + (path << 8), 0x0003FFFF, 0x0); /*[17:0]*/
	halrf_wreg(rf, 0x81f0 + (path << 8), 0xF0000000, 0x0); /*[31:28]*/

	return _dpk_gainloss_read_8832br(rf);
}

u8 _dpk_pas_read_8832br(
	struct rf_info *rf,
	enum rf_path path,
	u8 is_check)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 i;
	u32 val1_i = 0, val1_q = 0, val2_i = 0, val2_q = 0;

	halrf_wreg(rf, 0x80d4, MASKBYTE2, 0x06); /*0x80d6, ctrl_out_Kpack*/
	halrf_wreg(rf, 0x80bc, BIT(14), 0x0);	/*query status*/
	halrf_wreg(rf, 0x80c0, MASKBYTE2, 0x08);

	if (is_check) {
		halrf_wreg(rf, 0x80c0, MASKBYTE3, 0x00);
		val1_i = halrf_rreg(rf, 0x80fc, MASKHWORD);
		if (val1_i >= 0x800)
			val1_i = 0x1000 - val1_i;
		val1_q = halrf_rreg(rf, 0x80fc, MASKLWORD);
		if (val1_q >= 0x800)
			val1_q = 0x1000 - val1_q;
		halrf_wreg(rf, 0x80c0, MASKBYTE3, 0x1f);
		val2_i = halrf_rreg(rf, 0x80fc, MASKHWORD);
		if (val2_i >= 0x800)
			val2_i = 0x1000 - val2_i;
		val2_q = halrf_rreg(rf, 0x80fc, MASKLWORD);
		if (val2_q >= 0x800)
			val2_q = 0x1000 - val2_q;

		RF_DBG(rf, DBG_RF_DPK, "[DPK] PAS_delta = 0x%x\n",
			(val1_i * val1_i + val1_q * val1_q) / 
			(val2_i * val2_i + val2_q * val2_q));
	} else {
		for (i = 0; i < 32; i++) {
			halrf_wreg(rf, 0x80c0, MASKBYTE3, i); /*0x80C3*/
			RF_DBG(rf, DBG_RF_DPK, "[DPK] PAS_Read[%02d]= 0x%08x\n", i,
				   halrf_rreg(rf, 0x80fc, MASKDWORD));
			dpk->dpk_pas[path][i] = halrf_rreg(rf, 0x80fc, MASKDWORD);
		}
	}

	if ((val1_i * val1_i + val1_q * val1_q) < (val2_i * val2_i + val2_q * val2_q))
		return 2;
	else if ((val1_i * val1_i + val1_q * val1_q) >= ((val2_i * val2_i + val2_q * val2_q) * 3))
		return 1;
	else
		return 0;
}

u8 _dpk_agc_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx,
	u8 init_xdbm,
	u8 loss_only)
{
	u8 i = 0, tmp_dbm = init_xdbm, tmp_gl_idx = 0;
	u8 tmp_rxbb = 0,limited_rxbb = 0;
	u8 goout = 0, agc_cnt = 0;
	s8 offset = 0;
	u16 dgain = 0;
	bool is_fail = false;
	
	do {
		switch (i) {
		case 0: /*SYNC and Dgain*/
			is_fail = _dpk_kip_set_rxagc_8832br(rf, phy, path, kidx);

			if (DPK_RXSRAM_DBG_8832BR)
				_dpk_read_rxsram_8832br(rf);

			if (is_fail) {
				goout = 1;
				break;
			}

			dgain = _dpk_dgain_read_8832br(rf);

			if (agc_cnt == 0) {
				if (rf->hal_com->band[phy].cur_chandef.bw < 2)
					_dpk_lbk_rxiqk_8832br(rf, phy, path, true);
				else
					_dpk_lbk_rxiqk_8832br(rf, phy, path, false);
			}
			i = 2;
			break;

		case 1: /*Gain adjustment*/
			_dpk_kip_control_rfc_8832br(rf, path, false);
			tmp_rxbb = (u8)halrf_rrf(rf, path, 0x00, MASKRFRXBB);
			offset = _dpk_dgain_mapping_8832br(rf, dgain);

			if (tmp_rxbb + offset > 0x1f) {
				tmp_rxbb = 0x1f;
				limited_rxbb = 1;
			} else if (tmp_rxbb + offset < 0) {
				tmp_rxbb = 0;
				limited_rxbb = 1;
			} else
				tmp_rxbb = tmp_rxbb + offset;

			halrf_wrf(rf, path, 0x00, MASKRFRXBB, tmp_rxbb);
			
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Adjust RXBB (%+d) = 0x%x\n", offset, tmp_rxbb);
			_dpk_kip_control_rfc_8832br(rf, path, true);
#if 0
			if (offset != 0 || agc_cnt == 0) {
				if (rf->hal_com->band[phy].cur_chandef.bw < 2)
					_dpk_lbk_rxiqk_8832br(rf, phy, path, true);
				else
					_dpk_lbk_rxiqk_8832br(rf, phy, path, false);
			}
#endif
			if ((dgain > 1922) || (dgain < 342))
				i = 0;
			else
				i = 2;

			agc_cnt++;
			break;

		case 2: /*GAIN_LOSS and idx*/
			tmp_gl_idx = _dpk_gainloss_8832br(rf, phy, path, kidx);
			if (DPK_PAS_DBG_8832BR)
				_dpk_pas_read_8832br(rf, path, false);

			if ((_dpk_pas_read_8832br(rf, path, true) == 2) && (tmp_gl_idx > 0))
				i = 4;
			else if ((tmp_gl_idx == 0 && _dpk_pas_read_8832br(rf, path, true) == 1) || tmp_gl_idx >= 7)
				i = 3; /*GL > criterion*/
			else if (tmp_gl_idx == 0)
				i = 4; /*GL < criterion*/
			else 
				i = 5;
			break;

		case 3: /*GL > criterion*/
			if (tmp_dbm <= 7) {
				goout = 1;
				RF_DBG(rf, DBG_RF_DPK, "[DPK] Txagc@lower bound!!\n");
			} else {
				if (tmp_dbm - 3 <= 7)
					tmp_dbm = 7;
				else
				tmp_dbm = tmp_dbm - 3;
				_dpk_kip_set_txagc_8832br(rf, phy, path, tmp_dbm, true);
			}
			i = 0;
			agc_cnt++;
			break;

		case 4:	/*GL < criterion*/
			if (tmp_dbm >= 24) {
				goout = 1;
				RF_DBG(rf, DBG_RF_DPK, "[DPK] Txagc@upper bound!!\n");
			} else {
				if (tmp_dbm + 2 >= 24)
					tmp_dbm = 24;
				else
				tmp_dbm = tmp_dbm + 2;
				_dpk_kip_set_txagc_8832br(rf, phy, path, tmp_dbm, true);
			}
			i = 0;
			agc_cnt++;
			break;

		case 5:
			_dpk_kip_control_rfc_8832br(rf, path, false);
			tmp_rxbb = (u8)halrf_rrf(rf, path, 0x00, MASKRFRXBB);
			if (tmp_rxbb + tmp_gl_idx > 0x1f)
				tmp_rxbb = 0x1f;
			 else
				tmp_rxbb = tmp_rxbb + tmp_gl_idx;

			 halrf_wrf(rf, path, 0x00, MASKRFRXBB, tmp_rxbb);
			
			RF_DBG(rf, DBG_RF_DPK, "[DPK] Adjust RXBB (%+d) = 0x%x\n",
				tmp_gl_idx, tmp_rxbb);
			_dpk_kip_control_rfc_8832br(rf, path, true);
			goout = 1;
			break;
		default:
			goout = 1;
			break;
		}	
	} while (!goout && (agc_cnt < 6));

	return is_fail;
}

void _dpk_set_mdpd_para_8832br(
	struct rf_info *rf,
	u8 order)
{
	switch (order) {
	case 0: /*(5,3,1)*/
		halrf_wreg(rf, 0x80a0, BIT(1) | BIT(0), 0x0);
		halrf_wreg(rf, 0x809c, BIT(10) | BIT(9), 0x2);
		halrf_wreg(rf, 0x80a0, 0x00001F00, 0x4); /*[12:8] phase normalize tap*/
		halrf_wreg(rf, 0x8070, 0x70000000, 0x1); /*[30:28] tx_delay_man*/
		break;

	case 1: /*(5,3,0)*/
		halrf_wreg(rf, 0x80a0, BIT(1) | BIT(0), 0x1);
		halrf_wreg(rf, 0x809c, BIT(10) | BIT(9), 0x1);
		halrf_wreg(rf, 0x80a0, 0x00001F00, 0x0); /*[12:8] phase normalize tap*/
		halrf_wreg(rf, 0x8070, 0x70000000, 0x0); /*[30:28] tx_delay_man*/
		break;

	case 2: /*(5,0,0)*/
		halrf_wreg(rf, 0x80a0, BIT(1) | BIT(0), 0x2);
		halrf_wreg(rf, 0x809c, BIT(10) | BIT(9), 0x0);
		halrf_wreg(rf, 0x80a0, 0x00001F00, 0x0); /*[12:8] phase normalize tap*/
		halrf_wreg(rf, 0x8070, 0x70000000, 0x0); /*[30:28] tx_delay_man*/
		break;

	case 3: /*(7,3,1)*/
		halrf_wreg(rf, 0x80a0, BIT(1) | BIT(0), 0x3);
		halrf_wreg(rf, 0x809c, BIT(10) | BIT(9), 0x3);
		halrf_wreg(rf, 0x80a0, 0x00001F00, 0x4); /*[12:8] phase normalize tap*/
		halrf_wreg(rf, 0x8070, 0x70000000, 0x1); /*[30:28] tx_delay_man*/
		break;
	default:
		RF_DBG(rf, DBG_RF_DPK, "[DPK] Wrong MDPD order!!(0x%x)\n", order);
		break;
	}

	RF_DBG(rf, DBG_RF_DPK, "[DPK] Set %s for IDL\n", order == 0x0 ? "(5,3,1)" :
		(order == 0x1 ? "(5,3,0)" : (order == 0x2 ? "(5,0,0)" : "(7,3,1)")));
}

bool _dpk_rek_check_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx,
	u8 rek_cnt)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 corr_idx;
	bool result = false;

	dpk->ov_flag[path] = 0;

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x0);	/*rpt_sel*/
	dpk->dpk_sync[path] = halrf_rreg(rf, 0x80fc, MASKDWORD);
	corr_idx = dpk->dpk_sync[path] & 0xff; /*[7:0]*/

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x9);	/*rpt_sel*/
	dpk->dpk_dciq[path] = halrf_rreg(rf, 0x80fc, MASKDWORD);

	if (HALRF_ABS(corr_idx - 16, dpk->corr_idx[path][kidx]) >= 2) {
		RF_DBG(rf, DBG_RF_DPK, "[DPK] corr_idx diff(%d, %d) >=2 happen!!!\n",
			dpk->corr_idx[path][kidx], corr_idx);
		dpk->rek_chk[path][0][rek_cnt] = 0x1;
	}

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0xf);	/*rpt_sel*/
	dpk->ov_flag[path] = (u8)halrf_rreg(rf, 0x80fc, BIT(30));

	if (dpk->ov_flag[path]) {
		RF_DBG(rf, DBG_RF_DPK, "[DPK] MDPK OV happen!!!\n");
		dpk->rek_chk[path][0][rek_cnt] = 0x3;
	}

	if (dpk->rek_chk[path][0][rek_cnt] != 0)
		result = true;
	else {
		RF_DBG(rf, DBG_RF_DPK, "[DPK] ReK check OK!!\n");
		result = false;
	}

	return result;
}

void _dpk_rc_mtx_8832br(
	struct rf_info *rf,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 addr, rpt12, rpt13;

	halrf_wreg(rf, 0x80e8, BIT(6), 0x0);
	halrf_wreg(rf, 0x80d8, BIT(17), 0x1);

	for (addr = 0; addr < 0x1b2; addr++) {
		halrf_wreg(rf, 0x80d8, 0x000001FF, addr); /*[8:0]*/

		halrf_wreg(rf, 0x80d4, 0x003F0000, 0xc); /*rpt_sel*/
		rpt12 = halrf_rreg(rf, 0x80fc, MASKDWORD);
		dpk->rc_mtx[path][addr][0] = rpt12;

		halrf_wreg(rf, 0x80d4, 0x003F0000, 0xd); /*rpt_sel*/
		rpt13 = halrf_rreg(rf, 0x80fc, MASKDWORD);
		dpk->rc_mtx[path][addr][1] = rpt13;
#if 0
		RF_DBG(rf, DBG_RF_DPK, "[DPK] 0x%03x | 0x%08x 0x%08x 0x%08x\n",
			addr, rpt12, rpt13, rpt14);
#endif
	}
	halrf_wreg(rf, 0x80d8, BIT(17), 0x0);
}

void _dpk_rxsram_8832br(
	struct rf_info *rf,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 addr;

	halrf_wreg(rf, 0x80e8, BIT(7), 0x1);
	halrf_wreg(rf, 0x8074, BIT(31), 0x1);

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x2); /*rpt_sel*/

	for (addr = 0; addr < 0x200; addr++) {
		halrf_wreg(rf, 0x80d8, MASKDWORD, 0x00010000 | addr); /*[8:0]*/
		dpk->rx_sram[path][addr] = halrf_rreg(rf, 0x80fc, MASKDWORD);
#if 0
		RF_DBG(rf, DBG_RF_DPK, "[DPK][RXSRAM] 0x%03x | 0x%08x\n",
			addr, dpk->rx_sram[path][addr]);
#endif
	}
	halrf_wreg(rf, 0x80e8, BIT(7), 0x0);
	halrf_wreg(rf, 0x8074, BIT(31), 0x0);
}

void _dpk_coef_read_8832br(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 reg, reg_start, reg_stop;

	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00010000);

	reg_start = 0xa500 + kidx * 0x1c0 + path * 0x400 + dpk->cur_k_set * 0x70;

	reg_stop = reg_start + 0x70;

	RF_DBG(rf, DBG_RF_DPK, "[DPK] ===== [Coef of S%d_CH%d_K%d =====\n",
		path, kidx, dpk->cur_k_set);

	for (reg = reg_start; reg < reg_stop ; reg += 4) {
		RF_DBG(rf, DBG_RF_DPK, "[DPK][coef_r] 0x%x = 0x%08x\n", reg,
			   halrf_rreg(rf, reg, MASKDWORD));
	}
	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00000000);
}

bool _dpk_idl_mpa_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 cnt = 0;
	bool is_rek;

	halrf_wreg(rf, 0x80a0, BIT(16), 0x1); /*Phase Normalize enable*/

	if (halrf_rreg(rf, 0x80ec, BIT(8)) == 0x1)
		_dpk_set_mdpd_para_8832br(rf, 0x0); /*5,3,1*/
	else if (halrf_rreg(rf, 0x80ec, BIT(9)) == 0x1)
		_dpk_set_mdpd_para_8832br(rf, 0x1); /*5,3,0*/
	else
		_dpk_set_mdpd_para_8832br(rf, 0x2); /*5,0,0*/

	halrf_wreg(rf, 0x809c, BIT(8), 0x0);

	_dpk_one_shot_8832br(rf, phy, path, D_MDPK_IDL);

	if (DPK_MDPK_DBG_8832BR)
		_dpk_rxsram_8832br(rf, path);

	if (halrf_get_8832br_nctl_reg_ver() >= 0xd) {
		_dpk_one_shot_8832br(rf, phy, path, 0x33);
		if (DPK_MDPK_DBG_8832BR)
			_dpk_rc_mtx_8832br(rf, path);

		_dpk_one_shot_8832br(rf, phy, path, D_MDPK_LDL);
	}

	is_rek = _dpk_rek_check_8832br(rf, phy, path, kidx, dpk->rek_cnt[path][0]);
#if 1
	while (is_rek == true && dpk->rek_cnt[path][0] < 5) {
		_dpk_one_shot_8832br(rf, phy, path, D_MDPK_IDL);
		if (halrf_get_8832br_nctl_reg_ver() >= 0xd) {
			_dpk_one_shot_8832br(rf, phy, path, 0x33);
			_dpk_one_shot_8832br(rf, phy, path, D_MDPK_LDL);
		}
		is_rek = _dpk_rek_check_8832br(rf, phy, path, kidx, dpk->rek_cnt[path][0]);
		dpk->rek_cnt[path][0]++;
	}
#endif
	if (dpk->rek_cnt[path][0] == 5)
		return true; /*fail*/
	else
		return false;
}

u8 _dpk_order_convert_8832br(
	struct rf_info *rf)
{
	u8 val;

	switch (halrf_rreg(rf, 0x80a0, BIT(1) | BIT(0))) {
	case 0: /*(5,3,1)*/
		val = 0x6;
		break;

	case 1: /*(5,3,0)*/
		val = 0x2;
		break;

	case 2: /*(5,0,0)*/
		val = 0x0;
		break;

	case 3: /*(7,3,1)*/
		val = 0x7;
		break;
	default:
		val = 0xff;
		break;
	}
		
	/*0x80a0 [1:0] = 0x0 => 0x81bc[26:25] = 0x6   //(5,3,1)*/
	/*0x80a0 [1:0] = 0x1 => 0x81bc[26:25] = 0x2   //(5,3,0)*/
	/*0x80a0 [1:0] = 0x2 => 0x81bc[26:25] = 0x0   //(5,0,0)*/
	/*0x80a0 [1:0] = 0x3 => 0x81bc[26:25] = 0x7   //(7,3,1)*/

	RF_DBG(rf, DBG_RF_DPK, "[DPK] convert MDPD order to 0x%x\n", val);

	return val;
}

void _dpk_gs_defalut_8832br(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};

	halrf_wreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0x0000007F, 0x5b);
#if 0
	/*CH0*/
	halrf_wreg(rf, 0x8190 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K0*/
	halrf_wreg(rf, 0x8194 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K1*/
	halrf_wreg(rf, 0x8198 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K2*/
	halrf_wreg(rf, 0x81a4 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K3*/
	/*CH1*/
	halrf_wreg(rf, 0x81a8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K0*/
	halrf_wreg(rf, 0x81c4 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K1*/
	halrf_wreg(rf, 0x81c8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K2*/
	halrf_wreg(rf, 0x81e8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K3*/
#endif
}

void _dpk_gain_normalize_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx,
	bool is_execute)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};

	if (is_execute) {
		halrf_wreg(rf, 0x819c + (path << 8), 0x000003FF, 0x200); /*[9:0] pow_cal_start*/
		halrf_wreg(rf, 0x819c + (path << 8), BIT(17) | BIT(16), 0x3); /*pow_cal_len*/

		_dpk_one_shot_8832br(rf, phy, path, D_GAIN_NORM);
	} else
		halrf_wreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0x0000007F, 0x5b);

	dpk->bp[path][kidx].gs = (u8)halrf_rreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0x0000007F);
#if 0
	/*CH0*/
	halrf_wreg(rf, 0x8190 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K0*/
	halrf_wreg(rf, 0x8194 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K1*/
	halrf_wreg(rf, 0x8198 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K2*/
	halrf_wreg(rf, 0x81a4 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K3*/
	/*CH1*/
	halrf_wreg(rf, 0x81a8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K0*/
	halrf_wreg(rf, 0x81c4 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K1*/
	halrf_wreg(rf, 0x81c8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K2*/
	halrf_wreg(rf, 0x81e8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K3*/
#endif
}

void _dpk_on_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	/*coef load*/
	halrf_wreg(rf, 0x81dc + (path << 8), BIT(16), 0x1);
	halrf_wreg(rf, 0x81dc + (path << 8), BIT(16), 0x0);

	halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2),
			BIT(26) | BIT(25) | BIT(24), _dpk_order_convert_8832br(rf));

	dpk->bp[path][kidx].path_ok = dpk->bp[path][kidx].path_ok | BIT(dpk->cur_k_set);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d[%d] path_ok = 0x%x\n", path, kidx,
		dpk->bp[path][kidx].path_ok);

	/*MDPD enable, [31:28] = [k3,k2,k1,k0]*/
	halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2), 0xf0000000,
			dpk->bp[path][kidx].path_ok);

	_dpk_gain_normalize_8832br(rf, phy, path, kidx, false);
	//_dpk_para_query_8832br(rf, path, kidx);
}

bool _dpk_reload_check_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	bool is_reload = false;
	u8 idx, cur_band, cur_ch;

	cur_band = rf->hal_com->band[phy].cur_chandef.band;
	cur_ch = rf->hal_com->band[phy].cur_chandef.center_ch;

	for (idx = 0; idx < DPK_BKUP_NUM; idx++) {
		if ((cur_band == dpk->bp[path][idx].band) && (cur_ch == dpk->bp[path][idx].ch)) {
			halrf_wreg(rf, 0x8104 + (path << 8), BIT(8), idx);
			dpk->cur_idx[path] = idx;
			is_reload = true;
			RF_DBG(rf, DBG_RF_DPK, "[DPK] reload S%d[%d] success\n", path, idx);
		}
	}

	return is_reload;
}

bool _dpk_main_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 init_xdbm = 21;
	u8 kidx = dpk->cur_idx[path];
	bool is_fail = false;

	_dpk_kip_control_rfc_8832br(rf, path, false);
	halrf_rf_direct_cntrl_8832br(rf, path, false); /*switch control to direct write*/
	halrf_wrf(rf, path, 0x10005, MASKRF, 0x03ffd); /*only keep BB control TX_POWER*/

	_dpk_rf_setting_8832br(rf, path, kidx);
	halrf_set_rx_dck_8832br(rf, phy, path, false);

	_dpk_kip_pwr_clk_onoff_8832br(rf, true);
	_dpk_kip_preset_8832br(rf, phy, path, kidx);
	_dpk_txpwr_bb_force_8832br(rf, path, true);
	_dpk_kip_set_txagc_8832br(rf, phy, path, init_xdbm, true);
	_dpk_tpg_sel_8832br(rf, path, kidx);
#if 0
	is_fail = _dpk_kip_set_rxagc_8832br(rf, phy, path, kidx);

	if (DPK_RXSRAM_DBG_8832BR)
		_dpk_read_rxsram_8832br(rf);

	if (is_fail)
		goto _error;

	_dpk_dgain_read_8832br(rf);

	if (rf->hal_com->band[phy].cur_chandef.bw < 2)
		_dpk_lbk_rxiqk_8832br(rf, phy, path, true);
	else
		_dpk_lbk_rxiqk_8832br(rf, phy, path, false);

	_dpk_gainloss_8832br(rf, phy, path, kidx);
	//_dpk_para_query_8832br(rf, path, kidx);
#else
	is_fail = _dpk_agc_8832br(rf, phy, path, kidx, init_xdbm, false);

	if (is_fail)
		goto _error;
#endif
	/*_dpk_pas_read_8832br(rf, path, false);*/
	//_dpk_get_thermal_8832br(rf, kidx, path);

	is_fail = _dpk_idl_mpa_8832br(rf, phy, path, kidx);
	_dpk_para_query_8832br(rf, path, kidx);

	if (is_fail)
		goto _error;

	//_dpk_kip_control_rfc_8832br(rf, path, false);
	//_dpk_get_thermal_8832br(rf, kidx, path);
#if 0
	_dpk_coef_read_8832br(rf, path, kidx, gain);
#endif
	_dpk_on_8832br(rf, phy, path, kidx);
_error:
	_dpk_kip_control_rfc_8832br(rf, path, false);
	halrf_wrf(rf, path, 0x00, MASKRFMODE, RF_RX);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d[%d]_K%d %s\n", path, kidx,
		dpk->cur_k_set, is_fail ? "need Check" : "is Success");

	return is_fail;
}

void _dpk_cal_select_8832br(
	struct rf_info *rf,
	bool force,
	enum phl_phy_idx phy,
	u8 kpath)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 kip_bkup[DPK_RF_PATH_MAX_8832BR][DPK_KIP_REG_NUM_8832BR] = {{0}};
	//u32 bb_bkup[DPK_BB_REG_NUM_8832BR] = {0};
	u32 rf_bkup[DPK_RF_PATH_MAX_8832BR][DPK_RF_REG_NUM_8832BR] = {{0}};

	u32 kip_reg[] = {0x813c, 0x8124, 0x8120, 0xc0c4, 0xc0e8, 0xc0d4, 0xc0d8, 0xc0ec};
	//u32 bb_reg[] = {0x2344, 0x5800, 0x7800};
	u32 rf_reg[DPK_RF_REG_NUM_8832BR] = {0xde, 0xdf, 0x8b, 0x90, 0x96,
					     0x97, 0x85, 0x5, 0x10005};

	u8 path;
	bool is_fail = true;
	bool reloaded[DPK_RF_PATH_MAX_8832BR] = {false};

	if ((!phl_is_mp_mode(rf->phl_com)) && DPK_RELOAD_EN_8832BR) {
		for (path = 0; path < DPK_RF_PATH_MAX_8832BR; path++) {
			reloaded[path] = _dpk_reload_check_8832br(rf, phy, path);
			if ((reloaded[path] == false) && (dpk->bp[path][0].ch != 0))
				dpk->cur_idx[path] = !dpk->cur_idx[path];
			else
				halrf_dpk_onoff_8832br(rf, path, false);
		}
	} else {
		for (path = 0; path < DPK_RF_PATH_MAX_8832BR; path++)
			dpk->cur_idx[path] = 0;
	}

	for (path = 0; path < DPK_RF_PATH_MAX_8832BR; path++) {
		if (kpath & BIT(path)) {
			_dpk_bkup_kip_8832br(rf, kip_reg, kip_bkup, path);
			_dpk_bkup_rf_8832br(rf, rf_reg, rf_bkup, path);
			_dpk_information_8832br(rf, phy, path);
			_dpk_init_8832br(rf, path);
			if (rf->is_tssi_mode[path])
				_dpk_tssi_pause_8832br(rf, path, true);
		}
	}

	for (path = 0; path < DPK_RF_PATH_MAX_8832BR; path++) {
		if (kpath & BIT(path)) {
			RF_DBG(rf, DBG_RF_DPK, "[DPK] ========= S%d[%d] DPK Start =========\n", path, dpk->cur_idx[path]);
			_dpk_rxagc_onoff_8832br(rf, path, false);
			halrf_drf_direct_cntrl_8832br(rf, path, false);
			_dpk_bb_afe_setting_8832br(rf, path);
			is_fail = _dpk_main_8832br(rf, phy, path);
			halrf_dpk_onoff_8832br(rf, path, is_fail);
		}
	}

	for (path = 0; path < DPK_RF_PATH_MAX_8832BR; path++) {
		if (kpath & BIT(path)) {
			_dpk_kip_restore_8832br(rf, phy, path);
			_dpk_reload_kip_8832br(rf, kip_reg, kip_bkup, path);
			_dpk_reload_rf_8832br(rf, rf_reg, rf_bkup, path);
			_dpk_bb_afe_restore_8832br(rf, path);
			_dpk_rxagc_onoff_8832br(rf, path, true);
			if (rf->is_tssi_mode[path])
				_dpk_tssi_pause_8832br(rf, path, false);
		}
	}
	_dpk_kip_pwr_clk_onoff_8832br(rf, false);
}

u8 _dpk_bypass_check_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy)
{
	struct halrf_fem_info *fem = &rf->fem;

	u8 result;

	if (rf->hal_com->band[phy].cur_chandef.band == BAND_ON_24G) {
		RF_DBG(rf, DBG_RF_DPK, "[DPK] Skip DPK due to 2G!!\n");
		result = 1;
	} else if (fem->epa_5g && (rf->hal_com->band[phy].cur_chandef.band == BAND_ON_5G)) {
		RF_DBG(rf, DBG_RF_DPK, "[DPK] Skip DPK due to 5G_ext_PA exist!!\n");
		result = 1;
	} else
		result = 0;

	return result;
}

void _dpk_force_bypass_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy)
{
	u8 path, kpath;

	kpath = halrf_kpath_8832br(rf, phy);

	for (path = 0; path < DPK_RF_PATH_MAX_8832BR; path++) {
		if (kpath & BIT(path))
			halrf_dpk_onoff_8832br(rf, path, true);
	}
}

void halrf_dpk_8832br(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	bool force)
{
	RF_DBG(rf, DBG_RF_DPK, "[DPK] ****** DPK Start (Ver: 0x%x, Cv: %d, RF_para: %d, NCTL: 0x%x) ******\n",
		DPK_VER_8832BR, rf->hal_com->cv, halrf_get_radio_ver_from_reg(rf),
		halrf_get_8832br_nctl_reg_ver());

	RF_DBG(rf, DBG_RF_DPK, "[DPK] Driver mode = %d\n", rf->phl_com->drv_mode);
#if 1
	if (_dpk_bypass_check_8832br(rf, phy))
		_dpk_force_bypass_8832br(rf, phy);
	else
		_dpk_cal_select_8832br(rf, force, phy, halrf_kpath_8832br(rf, phy));
#else
	//_dpk_information_8832br(rf, 0, RF_PATH_A);
	//halrf_drf_direct_cntrl_8832br(rf, RF_PATH_A, false);
	//_dpk_bb_afe_setting_8832br(rf, RF_PATH_A);
	
	//_dpk_main_8832br(rf, phy, RF_PATH_A);
	//halrf_delay_us(rf, 100);
	halrf_rf_direct_cntrl_8832br(rf, RF_PATH_A, false); /*switch control to direct write*/
	//halrf_delay_us(rf, 100);
	//halrf_wrf(rf, RF_PATH_A, 0x10005, MASKRF, 0x03ffd); /*only keep BB control TX_POWER*/
	_dpk_rf_setting_8832br(rf, RF_PATH_A, 0);
	//halrf_delay_us(rf, 70);
	halrf_set_rx_dck_8832br(rf, phy, RF_PATH_A, false);
	//_dpk_kip_pwr_clk_onoff_8832br(rf, true);
	//_dpk_kip_preset_8832br(rf, phy, RF_PATH_A, 0);
	//_dpk_txpwr_bb_force_8832br(rf, RF_PATH_A, true);
	//_dpk_kip_set_txagc_8832br(rf, phy, RF_PATH_A, 21, true);
	//_dpk_tpg_sel_8832br(rf, RF_PATH_A, 0);
	//_dpk_kip_set_rxagc_8832br(rf, phy, RF_PATH_A, 0);


#endif
}

void halrf_dpk_onoff_8832br(
	struct rf_info *rf,
	enum rf_path path,
	bool off)
{
	struct halrf_dpk_info *dpk = &rf->dpk;
	bool off_reverse;
	u8 val, kidx = dpk->cur_idx[path];

	if (off)
		off_reverse = false;
	else
		off_reverse = true;

	val = dpk->is_dpk_enable * off_reverse * dpk->bp[path][kidx].path_ok;

	/*MDPD enable, [31:28] = [k3,k2,k1,k0]*/
	halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2), 0xf0000000, val);

	RF_DBG(rf, DBG_RF_DPK, "[DPK] S%d[%d] DPK %s !!!\n", path, kidx,
		   (val == 0) ? "disable" : "enable");
}

void halrf_dpk_track_8832br(
	struct rf_info *rf)
{
#if 1 
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 path, kidx;
	u8 txagc_rf;
	s8 txagc_bb, txagc_bb_tp, txagc_ofst;
	u8 cur_ther;
	s16 pwsf_tssi_ofst;
	s8 delta_ther = 0;
	//u8 tx_sf_addr;
	//s8 slope;

	for (path = 0; path < DPK_RF_PATH_MAX_8832BR; path++) {

		kidx = dpk->cur_idx[path];

		RF_DBG(rf, DBG_RF_DPK_TRACK,
		       "[DPK_TRK] ================[S%d[%d] (CH %d)]================\n",
		       path, kidx, dpk->bp[path][kidx].ch);

		/*rpt from BB*/
		txagc_rf = (u8)halrf_rreg(rf, 0x1c60 + (path << 13), 0x0000003f); /*[5:0]*/
		txagc_bb = (s8)halrf_rreg(rf, 0x1c60 + (path << 13), MASKBYTE2); /*[23:16]*/
		txagc_bb_tp = (u8)halrf_rreg(rf, 0x1ca0 + (path << 13), 0xFF000000); /*[31:24]*/

		/*rpt from KIP*/
		halrf_wreg(rf, 0x81d4 + (path << 8), 0x003F0000, 0xf);	/*rpt_sel*/
		cur_ther = (u8)halrf_rreg(rf, 0x81fc + (path << 8), 0x0000003F); /*[5:0]*/
		txagc_ofst = (s8)halrf_rreg(rf, 0x81fc + (path << 8), 0x0000FF00); /*[15:8]*/
		pwsf_tssi_ofst = (s16)halrf_rreg(rf, 0x81fc + (path << 8), 0x1FFF0000); /*[28:16]*/

		if (pwsf_tssi_ofst >= 0x1000)
			pwsf_tssi_ofst |= 0xE000;

		//halrf_wreg(rf, 0x81d4 + (path << 8), 0x003F0000, 0x11);	/*rpt_sel*/
		//tx_sf_addr = (u8)halrf_rreg(rf, 0x81fc + (path << 8), 0x000000FF); /*[7:0]*/

		//slope = (s8)halrf_rreg(rf, 0x81bc + (path << 8) + (kidx << 2), 0x00003F00); /*[13:8]*/

		delta_ther = cur_ther - dpk->bp[path][kidx].ther_dpk;
		
		delta_ther = delta_ther * 1 / 2;

		RF_DBG(rf, DBG_RF_DPK_TRACK, "[DPK_TRK] extra delta_ther = %d (0x%x / 0x%x @K)\n",
			delta_ther, cur_ther, dpk->bp[path][kidx].ther_dpk);

		RF_DBG(rf, DBG_RF_DPK_TRACK, "[DPK_TRK] delta_txagc = %d (0x%x / 0x%x @K)\n",
			txagc_rf - dpk->bp[path][kidx].txagc_dpk,
			txagc_rf, dpk->bp[path][kidx].txagc_dpk);

		RF_DBG(rf, DBG_RF_DPK_TRACK, "[DPK_TRK] txagc_offset / pwsf_tssi_ofst = 0x%x / %+d\n",
			txagc_ofst, pwsf_tssi_ofst);

		RF_DBG(rf, DBG_RF_DPK_TRACK, "[DPK_TRK] txagc_bb_tp / txagc_bb = 0x%x / 0x%x\n",
			txagc_bb_tp, txagc_bb);

		if (rf->rfk_is_processing != true && halrf_rreg(rf, 0x80f0, BIT(29)) == 0x0 && txagc_rf != 0) {
			RF_DBG(rf, DBG_RF_DPK_TRACK, "[DPK_TRK] New pwsf = 0x%x\n", 0x78 - delta_ther);

			halrf_wreg(rf, 0x81b4 + (path << 8) + (kidx << 2), 0x07FC0000, 0x78 - delta_ther); /*[26:18] k2*/
		}
	}
#endif
}

#endif
