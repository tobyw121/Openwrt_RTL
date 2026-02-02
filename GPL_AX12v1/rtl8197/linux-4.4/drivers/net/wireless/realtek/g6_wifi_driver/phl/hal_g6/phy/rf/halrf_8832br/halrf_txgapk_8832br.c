/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
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

void _txgapk_backup_bb_registers_8832br(
	struct rf_info *rf,
	u32 *reg,
	u32 *reg_bkup,
	u8 reg_num)
{
	u8 i;

	for (i = 0; i < reg_num; i++) {
		reg_bkup[i] = halrf_rreg(rf, reg[i], MASKDWORD);
		
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] Backup BB 0x%x = %x\n", reg[i], reg_bkup[i]);
	}
}

void _txgapk_backup_kip_8832br(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[][TXGAPK_KIP_REG_NUM_8832BR],
	u8 path,
	u8 reg_num)
{
	u8 i;

	for (i = 0; i < reg_num; i++) {
		reg_bkup[path][i] = halrf_rreg(rf, reg[i] + (path << 8), MASKDWORD);
		
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] Backup kip 0x%x = %x\n", reg[i]+ (path << 8), reg_bkup[path][i]);
	}
}


void _txgapk_bkup_rf_8832br(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 rf_bkup[][TXGAPK_RF_REG_NUM_8832BR],
	u8 path,
	u8 reg_num)
{
	u8 i;

	for (i = 0; i < reg_num; i++) {
		rf_bkup[path][i] = halrf_rrf(rf, path, rf_reg[i], MASKRF);
		
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] Backup RF S%d 0x%x = %x\n",
			path, rf_reg[i], rf_bkup[path][i]);
	}
}
	



void _txgapk_reload_bb_registers_8832br(
	struct rf_info *rf,
	u32 *reg,
	u32 *reg_backup,
	u8 reg_num)
{
	u8 i;

	for (i = 0; i < reg_num; i++) {
		halrf_wreg(rf, reg[i], MASKDWORD, reg_backup[i]);

		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] Reload BB 0x%x = 0x%x\n",
		       reg[i], reg_backup[i]);
	}
}


void _txgapk_reload_rf_8832br(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 rf_bkup[][TXGAPK_RF_REG_NUM_8832BR],
	u8 path,
	u8 reg_num)
{
	u8 i;

	for (i = 0; i < reg_num; i++) {
		halrf_wrf(rf, path, rf_reg[i], MASKRF, rf_bkup[path][i]);
		
			RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] Reload RF S%d 0x%x = %x\n",
				path, rf_reg[i], rf_bkup[path][i]);
	}
}


void _txgapk_reload_kip_8832br(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[][TXGAPK_KIP_REG_NUM_8832BR],
	u8 path,
	u8 reg_num) 
{
	u8 i;

	for (i = 0; i < reg_num; i++) {
		halrf_wreg(rf, reg[i] + (path << 8), MASKDWORD, reg_bkup[path][i]);
		
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] Reload KIP 0x%x = %x\n", reg[i] + (path << 8),
			reg_bkup[path][i]);
	}
}

void _halrf_txgapk_rxagc_onoff_8832br(
	struct rf_info *rf,
	enum rf_path path,
	bool turn_on)
{
	if (path == RF_PATH_A)
		halrf_wreg(rf, 0x4730, BIT(31), turn_on);
	else
		halrf_wreg(rf, 0x4a9c, BIT(31), turn_on);

	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] S%d RXAGC is %s\n", path,
		turn_on ? "turn_on" : "turn_off");
}



void _halrf_txgapk_bb_afe_by_mode_8832br(struct rf_info *rf,
					enum phl_phy_idx phy, enum rf_path path, bool is_dbcc)
{
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s\n", __func__);

	if (!is_dbcc) {
		/* nodbcc */
		if (path == RF_PATH_A) {			
			halrf_wrf(rf, RF_PATH_A, 0x10005, 0x00001, 0x0);
			halrf_wreg(rf, 0x20fc, 0x00010000, 0x1);
			halrf_wreg(rf, 0x20fc, 0x00100000, 0x0);
			halrf_wreg(rf, 0x20fc, 0x01000000, 0x1);
			halrf_wreg(rf, 0x20fc, 0x10000000, 0x0);
			
			halrf_wreg(rf, 0x5670, MASKDWORD, 0xf801fffd);
			
			#if 0
			halrf_wreg(rf, 0x5670, 0x00004000, 0x1);
			halrf_wreg(rf, 0x5670, 0x80000000, 0x1);
						
			halrf_wreg(rf, 0x12a0, 0x00008000, 0x0);
			halrf_wreg(rf, 0x12a0, 0x00007000, 0x7);
			halrf_wreg(rf, 0x12a0, 0x00008000, 0x1);
			#else
			halrf_txck_force_8832br(rf, path, true, DAC_960M);
			#endif
			
			#if 0
			halrf_wreg(rf, 0x5670, 0x00002000, 0x1);
				
			halrf_wreg(rf, 0x12a0, 0x00080000, 0x0);
			halrf_wreg(rf, 0x12a0, 0x00070000, 0x3);
			halrf_wreg(rf, 0x12a0, 0x00080000, 0x1);
						
			halrf_wreg(rf, 0x5670, 0x60000000, 0x2);
			#else
			halrf_rxck_force_8832br(rf, path, true, ADC_1920M);
			#endif
			
			
//			halrf_wreg(rf, 0xc0d4, 0x0c000000, 0x0);
//			halrf_wreg(rf, 0xc0d8, 0x000001e0, 0x7);
			
			// = halrf_rreg(rf, 0xc0ec, 0x00006000);
			halrf_wreg(rf, 0xc0ec, 0x00006000, 0x0); //force CBW_idx 0

			
			halrf_wreg(rf, 0x12b8, 0x40000000, 0x1);
			halrf_wreg(rf, 0x030c, 0xff000000, 0x1f);
			halrf_wreg(rf, 0x030c, 0xff000000, 0x13);
			halrf_wreg(rf, 0x032c, 0xffff0000, 0x0001);
			halrf_wreg(rf, 0x032c, 0xffff0000, 0x0041);
			halrf_wreg(rf, 0x20fc, 0x00100000, 0x1);
			halrf_wreg(rf, 0x20fc, 0x10000000, 0x1);
		} else if (path == RF_PATH_B) {
			halrf_wrf(rf, RF_PATH_B, 0x10005, 0x00001, 0x0);
			halrf_wreg(rf, 0x20fc, 0x00020000, 0x1);
			halrf_wreg(rf, 0x20fc, 0x00200000, 0x0);
			halrf_wreg(rf, 0x20fc, 0x02000000, 0x1);
			halrf_wreg(rf, 0x20fc, 0x20000000, 0x0);
			halrf_wreg(rf, 0x7670, MASKDWORD, 0xf801fffd);

			#if 0
			halrf_wreg(rf, 0x7670, 0x00004000, 0x1);
			halrf_wreg(rf, 0x7670, 0x80000000, 0x1);

			halrf_wreg(rf, 0x32a0, 0x00008000, 0x0);
			halrf_wreg(rf, 0x32a0, 0x00007000, 0x7);
			halrf_wreg(rf, 0x32a0, 0x00008000, 0x1);
			#else
			halrf_txck_force_8832br(rf, path, true, DAC_960M);
			#endif

			#if 0
			halrf_wreg(rf, 0x32a0, 0x00080000, 0x0);
			halrf_wreg(rf, 0x32a0, 0x00070000, 0x3);
			halrf_wreg(rf, 0x32a0, 0x00080000, 0x1);

			halrf_wreg(rf, 0x7670, 0x60000000, 0x2);
			#else
			halrf_rxck_force_8832br(rf, path, true, ADC_1920M);
			#endif

			halrf_wreg(rf, 0xc1d4, 0x0c000000, 0x0);
			halrf_wreg(rf, 0xc1d8, 0x000001e0, 0x7);

			// = halrf_rreg(rf, 0xc1ec, 0x00006000);
			halrf_wreg(rf, 0xc1ec, 0x00006000, 0x0); //force CBW_idx 0

			halrf_wreg(rf, 0x32b8, 0x40000000, 0x1);
			halrf_wreg(rf, 0x030c, 0xff000000, 0x1f);
			halrf_wreg(rf, 0x030c, 0xff000000, 0x13);
			halrf_wreg(rf, 0x032c, 0xffff0000, 0x0001);
			halrf_wreg(rf, 0x032c, 0xffff0000, 0x0041);
			halrf_wreg(rf, 0x20fc, 0x00200000, 0x1);
			halrf_wreg(rf, 0x20fc, 0x20000000, 0x1);			
		}				
	} else {
		if (phy == HW_PHY_0) {
			/* dbcc phy0 path 0 */
			//TBD
		} else if (phy == HW_PHY_1) {
			/* dbcc phy1 path 1 */
			//TBD
		}
	}
		
}




void _halrf_txgapk_iqk_preset_by_mode_8832br(struct rf_info *rf,
					enum phl_phy_idx phy, enum rf_path path, bool is_dbcc)
{
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s\n", __func__);

	if (!is_dbcc) {
		halrf_wrf(rf, path, 0x5, 0x00001, 0x0);
		halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000080);
		halrf_wreg(rf, 0x8088, MASKDWORD, 0x81ff010a);		
	} else {
		/* dbcc */
		//TBD
		//if (phy == HW_PHY_0)		
		//else if (phy == HW_PHY_1)
			
	}
	
}

void _halrf_txgapk_one_shot_nctl_done_check_8832br
	(struct rf_info *rf)
{
	/* for check status */
	u32 r_bff8 = 0;
	u32 r_80fc = 0;
	bool is_ready = false;
	u16 count = 1;

	
	/* for 0xbff8 check NCTL DONE */
	while (count < 2000)
	{	
		r_bff8 = halrf_rreg(rf, 0xbff8, MASKBYTE0);
				
		if (r_bff8 == 0x55)
		{
			is_ready = true;
			break;
		}	
		r_bff8 = 0;
		halrf_delay_us(rf, 10);
		count++;
	}
	
	halrf_delay_us(rf, 1);
	
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> check 0xBFF8[7:0] = 0x%x, IsReady = %d, ReadTimes = %d,delay 1 us\n", r_bff8, is_ready, count);

	

	/* for 0x80fc check NCTL DONE */	
	count = 1;
	is_ready = false;
	while (count < 2000)
	{			
		r_80fc = halrf_rreg(rf, 0x80fc, MASKLWORD);
	
		if (r_80fc == 0x8000)
		{
			is_ready = true;
			break;
		}	
		r_80fc = 0;
		halrf_delay_us(rf, 1);
		count++;
	}

	halrf_delay_us(rf, 1);
		
	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);

	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> check 0x80fc[15:0] = 0x%x, IsReady = %d, ReadTimes = %d, 0x%x= 0x%x \n", r_80fc, is_ready, count, 0x8010, halrf_rreg(rf, 0x8010, MASKDWORD) ); 
		
}

void _halrf_txgapk_rf_dpk_5g_8832br
	(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path)
{
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s\n", __func__);

	
	halrf_wrf(rf, path, 0x05, 0x00001, 0x0);
	halrf_wrf(rf, path, 0x10005, MASKRF, 0x3ffd);
	halrf_wrf(rf, path, 0x10000, 0xf0000, 0x5);
	halrf_wrf(rf, path, 0x00, MASKRF, 0x503e1);
	halrf_wrf(rf, path, 0xdf, 0x01000, 0x1);
	halrf_wrf(rf, path, 0x9e, 0x00100, 0x1);
	halrf_wrf(rf, path, 0x8c, 0x0e000, 0x0);
	halrf_wrf(rf, path, 0x8c, 0x01800, 0x1);
	halrf_wrf(rf, path, 0x8c, 0x00600, 0x1);
}

void _halrf_txgapk_rf_rxdck_5g_8832br
	(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path)
{
	u8 i = 0;

	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s\n", __func__);

	
	halrf_wrf(rf, path, 0x92, 0x00001, 0x0);
	halrf_wrf(rf, path, 0x92, 0x00001, 0x1);
	
	for (i = 0; i < 100; i++)
		halrf_delay_us(rf, 1);
	
	halrf_wrf(rf, path, 0x92, 0x00001, 0x0);
}


void _halrf_txgapk_track_table_nctl_5g_8832br
	(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path)
{
	struct halrf_gapk_info *txgapk_info = &rf->gapk;

	u32 i;
	u32 d[17] = {0};
	//u32 ta[17] = {0};

	u32 calcu_ta[17] = {0};
	
	u32 itqt[2] = {0x81cc, 0x82cc};
	
	u32 iqk_ctrl_rfc_addr[2] = {0x5670, 0x7670};
	
	u32 process_id1[2] = {0x00001019, 0x00001029};
	u32 process_id2[2] = {0x00001519, 0x00001529};

	u32 gapk_on_tbl_setting[2] = {0x8158, 0x8258};
	u32 gain_stage =  0x003550;

	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s\n", __func__);

	
	halrf_wreg(rf, 0x80e4, 0x0000003f, 0x3f);
	halrf_wreg(rf, 0x801c, 0x000e0000, 0x2);
	halrf_wreg(rf, 0x80e0, 0x000001f0, 0x07);
	halrf_wreg(rf, 0x80e0, 0x0000f000, 0x0);
	halrf_wreg(rf, 0x8038, 0x003f0000, 0x2d);
	
	halrf_wreg(rf, gapk_on_tbl_setting[path], 0x001fffff,  gain_stage);
	
	halrf_wreg(rf, iqk_ctrl_rfc_addr[path], 0x00000002, 0x1);
	halrf_wreg(rf, itqt[path], 0x0000003f, 0x12);
	halrf_wreg(rf, 0x802c, 0x0fff0000, 0x009);

	#if 0
	halrf_wreg(rf, 0x8000, MASKDWORD, 0x00001019);
	ODM_delay_us(100);
	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);
	#else
	halrf_do_one_shot_8832br(rf, path, 0x8000, MASKDWORD, process_id1[path]);

	txgapk_info->txgapk_chk_cnt[path][TXGAPK_TRACK][0] = rf->nctl_ck_times[0]; 
	txgapk_info->txgapk_chk_cnt[path][TXGAPK_TRACK][1] = rf->nctl_ck_times[1];
	#endif
		
	halrf_wreg(rf, itqt[path], 0x0000003f, 0x3f);
	
	#if 0
	halrf_wreg(rf, 0x8000, MASKDWORD, 0x00001519);
	ODM_delay_us(100);
	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);
	#else
	halrf_do_one_shot_8832br(rf, path, 0x8000, MASKDWORD, process_id2[path]);
	#endif
	
	
	halrf_wreg(rf, iqk_ctrl_rfc_addr[path], 0x00000002, 0x0);
	halrf_wreg(rf, 0x80e0, 0x00000001, 0x0);

	//  ========END : Do Track GapK =====


	// ==== check performance
	#if TXGAPK_DBG
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> rf0x1005c [17:11] = 0x%x\n", halrf_rrf(rf, path, 0x1005c, 0x3f800));
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> rf0x1005c [5:0] = 0x%x\n", halrf_rrf(rf, path, 0x1005c, 0x0003f));
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> rf0x1005e [17:12] = 0x%x\n", halrf_rrf(rf, path, 0x1005e, 0x3f000));
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> rf0x1005e [5:0] = 0x%x\n", halrf_rrf(rf, path, 0x1005e, 0x0003f));
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> rf0x11 [1:0] = 0x%x\n", halrf_rrf(rf, path, 0x11, 0x00003));
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> rf0x11 [6:4] = 0x%x\n", halrf_rrf(rf, path, 0x11, 0x00070));
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> rf0x11 [16:12] = 0x%x\n", halrf_rrf(rf, path, 0x11, 0x1f000));
	#endif

	// ===== Read GapK Results, Bcut resolution = 0.0625 dB =====
	//read report 
	halrf_wreg(rf, 0x80d4, MASKDWORD, 0x00130000);

	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x3);
	d[0] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[1] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[2] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[3] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x4);
	d[4] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[5] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[6] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[7] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x5);
	d[8] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[9] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[10] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[11] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x6);
	d[12] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[13] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[14] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[15] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x7);
	d[16] = halrf_rreg(rf, 0x80fc, 0x0000007f);


	#if 0
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x9);
	ta[0] = halrf_rreg(rf, 0x80fc, 0x000000ff);
	ta[1] = halrf_rreg(rf, 0x80fc, 0x0000ff00);
	ta[2] = halrf_rreg(rf, 0x80fc, 0x00ff0000);
	ta[3] = halrf_rreg(rf, 0x80fc, 0xff000000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xa);
	ta[4] = halrf_rreg(rf, 0x80fc, 0x000000ff);
	ta[5] = halrf_rreg(rf, 0x80fc, 0x0000ff00);
	ta[6] = halrf_rreg(rf, 0x80fc, 0x00ff0000);
	ta[7] = halrf_rreg(rf, 0x80fc, 0xff000000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xb);
	ta[8] = halrf_rreg(rf, 0x80fc, 0x000000ff);
	ta[9] = halrf_rreg(rf, 0x80fc, 0x0000ff00);
	ta[10] = halrf_rreg(rf, 0x80fc, 0x00ff0000);
	ta[11] = halrf_rreg(rf, 0x80fc, 0xff000000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xc);
	ta[12] = halrf_rreg(rf, 0x80fc, 0x000000ff);
	ta[13] = halrf_rreg(rf, 0x80fc, 0x0000ff00);
	ta[14] = halrf_rreg(rf, 0x80fc, 0x00ff0000);
	ta[15] = halrf_rreg(rf, 0x80fc, 0xff000000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xd);
	ta[16] = halrf_rreg(rf, 0x80fc, 0x000000ff);
	#endif

	// Get The Value --> Ta 6bit
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x9);
	calcu_ta[0] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[1] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[2] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[3] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xa);
	calcu_ta[4] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[5] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[6] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[7] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xb);
	calcu_ta[8] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[9] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[10] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[11] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xc);
	calcu_ta[12] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[13] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[14] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[15] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xd);
	calcu_ta[16] = halrf_rreg(rf, 0x80fc, 0x0000007e);



	#if TXGAPK_DBG	 
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x2);
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> 0x80fc [6:0] = 0x%x\n", halrf_rreg(rf, 0x80fc, 0x0000007f));
	 
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x0);
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> 0x80fc = 0x%x\n", halrf_rreg(rf, 0x80fc, MASKDWORD));
	 
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x2);
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> 0x80fc [22:16] = 0x%x\n", halrf_rreg(rf, 0x80fc, 0x007f0000));
	 
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x1);
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======>psd_pwr1 0x80fc = 0x%x\n", halrf_rreg(rf, 0x80fc, MASKDWORD));
	 
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xf);
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======>gain_out 0x80fc = 0x%x\n", halrf_rreg(rf, 0x80fc, MASKDWORD));

	// continuous single tone
	halrf_wreg(rf, itqt[path], 0x3f, 0x2d); //ItQt
	halrf_wreg(rf, 0x80d0, 0x00100000, 0x1); //tst_iqk2set
	#endif

	//============================
	for (i = 0; i < 17; i++) {
		if (d[i] & BIT(6))
			txgapk_info->track_d[path][i] = (s32)(d[i] | 0xffffff80);
		else
			txgapk_info->track_d[path][i] = (s32)(d[i]);

	#if 0
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	track	d[%d][%d]=0x%x\n", 
			path, i, txgapk_info->track_d[path][i]);
	#else
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	track	d[%d][%d](7bit)=0x%02x\n", 
			path, i, d[i]);

	#endif
	}

	#if 0
	for (i = 0; i < 17; i++) {
		if (ta[i] & BIT(7))
			txgapk_info->track_ta[path][i] = (s32)(ta[i] | 0xffffff00);
		else
			txgapk_info->track_ta[path][i] = (s32)(ta[i]);
		
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	track	ta[%d][%d]=0x%x\n",
			path, i, txgapk_info->track_ta[path][i]);
	}
	#endif
	
	for (i = 0; i < 17; i++) {
		if (calcu_ta[i] & BIT(5))
			txgapk_info->track_ta[path][i] = (s32)(calcu_ta[i] | 0xffffffc0);
		else
			txgapk_info->track_ta[path][i] = (s32)(calcu_ta[i]);
#if 0		
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	track	ta[%d][%d]=0x%x\n",
			path, i, txgapk_info->track_ta[path][i]);
#else
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	track	ta[%d][%d]=0x%x\n",
			path, i, calcu_ta[i]);
#endif
	}
	
}


void _halrf_txgapk_write_track_table_default_5g_8832br
	(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path)
{
	//New reset table method for 52c later series	
	struct halrf_gapk_info *txgapk_info = &rf->gapk;

	u32 i;
	u32 d[17] = {0};
	u32 calcu_ta[17] = {0};
	
	u32 itqt[2] = {0x81cc, 0x82cc};
	
	u32 iqk_ctrl_rfc_addr[2] = {0x5670, 0x7670};
	
	u32 process_id1[2] = {0x00001019, 0x00001029};
	u32 process_id2[2] = {0x00001519, 0x00001529};

	u32 gapk_on_tbl_setting[2] = {0x8158, 0x8258};
	u32 gain_stage =  0x0;

	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s\n", __func__);

	
	halrf_wreg(rf, 0x80e4, 0x0000003f, 0x3f);
	halrf_wreg(rf, 0x801c, 0x000e0000, 0x2);
	halrf_wreg(rf, 0x80e0, 0x000001f0, 0x07);
	halrf_wreg(rf, 0x80e0, 0x0000f000, 0x0);
	halrf_wreg(rf, 0x8038, 0x003f0000, 0x2d);

	//value 0 for reset table
	halrf_wreg(rf, gapk_on_tbl_setting[path], 0x001fffff, gain_stage);
	
	halrf_wreg(rf, iqk_ctrl_rfc_addr[path], 0x00000002, 0x1);
	halrf_wreg(rf, itqt[path], 0x0000003f, 0x12);
	halrf_wreg(rf, 0x802c, 0x0fff0000, 0x009);

	#if 0
	halrf_wreg(rf, 0x8000, MASKDWORD, 0x00001019);
	ODM_delay_us(100);
	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);
	#else
	halrf_do_one_shot_8832br(rf, path, 0x8000, MASKDWORD, process_id1[path]);
	#endif
		
	halrf_wreg(rf, itqt[path], 0x0000003f, 0x3f);
	
	#if 0
	halrf_wreg(rf, 0x8000, MASKDWORD, 0x00001519);
	ODM_delay_us(100);
	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);
	#else
	halrf_do_one_shot_8832br(rf, path, 0x8000, MASKDWORD, process_id2[path]);
	#endif
	
	
	halrf_wreg(rf, iqk_ctrl_rfc_addr[path], 0x00000002, 0x0);
	halrf_wreg(rf, 0x80e0, 0x00000001, 0x0);
	//  ========END : Do Track GapK =====


	// ==== check performance
	// ===== Read GapK Results, Bcut resolution = 0.0625 dB =====
	//read report 
	halrf_wreg(rf, 0x80d4, MASKDWORD, 0x00130000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x3);
	d[0] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[1] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[2] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[3] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x4);
	d[4] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[5] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[6] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[7] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x5);
	d[8] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[9] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[10] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[11] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x6);
	d[12] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[13] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[14] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[15] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x7);
	d[16] = halrf_rreg(rf, 0x80fc, 0x0000007f);


	// Get The Value --> Ta 6bit
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x9);
	calcu_ta[0] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[1] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[2] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[3] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xa);
	calcu_ta[4] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[5] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[6] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[7] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xb);
	calcu_ta[8] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[9] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[10] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[11] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xc);
	calcu_ta[12] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[13] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[14] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[15] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xd);
	calcu_ta[16] = halrf_rreg(rf, 0x80fc, 0x0000007e);

	//============================
	for (i = 0; i < 17; i++) {
		if (d[i] & BIT(6))
			txgapk_info->track_d[path][i] = (s32)(d[i] | 0xffffff80);
		else
			txgapk_info->track_d[path][i] = (s32)(d[i]);

		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	track	d[%d][%d]=0x%x\n", 
			path, i, txgapk_info->track_d[path][i]);
	}
	
	for (i = 0; i < 17; i++) {
		if (calcu_ta[i] & BIT(5))
			txgapk_info->track_ta[path][i] = (s32)(calcu_ta[i] | 0xffffffc0);
		else
			txgapk_info->track_ta[path][i] = (s32)(calcu_ta[i]);
#if 0		
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	track	ta[%d][%d]=0x%x\n",
			path, i, txgapk_info->track_ta[path][i]);
#else
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	track	ta[%d][%d]=0x%x\n",
			path, i, calcu_ta[i]);
#endif
	}
}


void _halrf_txgapk_power_table_nctl_5g_8832br
	(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path)
{
	struct halrf_gapk_info *txgapk_info = &rf->gapk;
	/* u8 rfe_type = rf->phl_com->dev_cap.rfe_type; */

	u32 i;
	u32 d[17] = {0};
	//u32 ta[17] = {0};
	u32 calcu_ta[17] = {0};
	
	u32 itqt[2] = {0x81cc, 0x82cc};
	u32 iqk_ctrl_rfc_addr[2] = {0x5670, 0x7670};

	u32 process_id1[2] = {0x00001119, 0x00001129};
	u32 process_id2[2] = {0x00001619, 0x00001629};

	u32 gapk_on_tbl_setting[2] = {0x8170, 0x8270};
	u32 gain_stage =  0x000540;
	
	

	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s\n", __func__);

	
	halrf_wreg(rf, 0x80e0, 0x000001f0, 0x07);
	halrf_wreg(rf, 0x8038, 0x003f0000, 0x24);
	
	halrf_wreg(rf, gapk_on_tbl_setting[path], 0x001fffff, gain_stage);
	halrf_wreg(rf, iqk_ctrl_rfc_addr[path], 0x00000002, 0x1);
	halrf_wreg(rf, itqt[path], MASKDWORD, 0x12);
	halrf_wreg(rf, 0x802c, 0x0fff0000, 0x009);

	#if 0
	halrf_wreg(rf, 0x8000, MASKDWORD, 0x00001119);
	ODM_delay_us(100);
	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);
	#else
	halrf_do_one_shot_8832br(rf, path, 0x8000, MASKDWORD, process_id1[path]);

	txgapk_info->txgapk_chk_cnt[path][TXGAPK_PWR][0] = rf->nctl_ck_times[0]; 
	txgapk_info->txgapk_chk_cnt[path][TXGAPK_PWR][1] = rf->nctl_ck_times[1];
	#endif
	
	halrf_wreg(rf, itqt[path], 0x0000003f, 0x3f);

	#if 0
	halrf_wreg(rf, 0x8000, MASKDWORD, 0x00001619);
	ODM_delay_us(100);
	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);
	#else
	halrf_do_one_shot_8832br(rf, path, 0x8000, MASKDWORD, process_id2[path]);
	#endif
	
	
	halrf_wreg(rf, iqk_ctrl_rfc_addr[path], 0x00000002, 0x0);
	halrf_wreg(rf, 0x801c, 0x000e0000, 0x0);	
	//	========END : Do PA GapK =====



	// ===== Read GapK Results, Bcut resolution = 0.0625 dB =====
	halrf_wreg(rf, 0x80d4, MASKDWORD, 0x00130000); //gapk_report

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x3);
	d[0] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[1] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[2] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[3] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x4);
	d[4] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[5] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[6] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[7] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x5);
	d[8] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[9] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[10] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[11] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x6);
	d[12] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[13] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[14] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[15] = halrf_rreg(rf, 0x80fc, 0x0fe00000);
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x7);
	d[16] = halrf_rreg(rf, 0x80fc, 0x0000007f);

#if 0
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x9);
	ta[0] = halrf_rreg(rf, 0x80fc, 0x000000ff);
	ta[1] = halrf_rreg(rf, 0x80fc, 0x0000ff00);
	ta[2] = halrf_rreg(rf, 0x80fc, 0x00ff0000);
	ta[3] = halrf_rreg(rf, 0x80fc, 0xff000000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xa);
	ta[4] = halrf_rreg(rf, 0x80fc, 0x000000ff);
	ta[5] = halrf_rreg(rf, 0x80fc, 0x0000ff00);
	ta[6] = halrf_rreg(rf, 0x80fc, 0x00ff0000);
	ta[7] = halrf_rreg(rf, 0x80fc, 0xff000000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xb);
	ta[8] = halrf_rreg(rf, 0x80fc, 0x000000ff);
	ta[9] = halrf_rreg(rf, 0x80fc, 0x0000ff00);
	ta[10] = halrf_rreg(rf, 0x80fc, 0x00ff0000);
	ta[11] = halrf_rreg(rf, 0x80fc, 0xff000000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xc);
	ta[12] = halrf_rreg(rf, 0x80fc, 0x000000ff);
	ta[13] = halrf_rreg(rf, 0x80fc, 0x0000ff00);
	ta[14] = halrf_rreg(rf, 0x80fc, 0x00ff0000);
	ta[15] = halrf_rreg(rf, 0x80fc, 0xff000000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xd);
	ta[16] = halrf_rreg(rf, 0x80fc, 0x000000ff);
#endif

	// Get The Value --> Ta 6bit

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x9);
	calcu_ta[0] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[1] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[2] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[3] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xa);
	calcu_ta[4] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[5] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[6] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[7] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xb);
	calcu_ta[8] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[9] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[10] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[11] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xc);
	calcu_ta[12] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[13] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[14] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[15] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xd);
	calcu_ta[16] = halrf_rreg(rf, 0x80fc, 0x0000007e);


	#if TXGAPK_DBG	 
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x2);
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> 0x80fc [6:0] = 0x%x\n", halrf_rreg(rf, 0x80fc, 0x0000007f));
	 
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x0);
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> 0x80fc = 0x%x\n", halrf_rreg(rf, 0x80fc, MASKDWORD));
	 
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x2);
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> 0x80fc [22:16] = 0x%x\n", halrf_rreg(rf, 0x80fc, 0x007f0000));
	 
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x1);
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======>psd_pwr1 0x80fc = 0x%x\n", halrf_rreg(rf, 0x80fc, MASKDWORD));
	 
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xf);
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======>gain_out 0x80fc = 0x%x\n", halrf_rreg(rf, 0x80fc, MASKDWORD));

	// continuous single tone
	halrf_wreg(rf, itqt[path], MASKDWORD, 0x24); //ItQt
	halrf_wreg(rf, 0x80d0, 0x00100000, 0x1); //tst_iqk2set
	#endif
	
	//==========
	for (i = 0; i < 17; i++) {
		if (d[i] & BIT(6))
			txgapk_info->power_d[path][i] = (s32)(d[i] | 0xffffff80);
		else
			txgapk_info->power_d[path][i] = (s32)(d[i]);
#if 0
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	power	d[%d][%d]=0x%x\n",
			path, i, txgapk_info->power_d[path][i]);
#else
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	power	d[%d][%d](7bit)=0x%02x\n", 
			path, i, d[i]);
#endif
	}

	#if 0
	for (i = 0; i < 17; i++) {
		if (ta[i] & BIT(7))
			txgapk_info->power_ta[path][i] = (s32)(ta[i] | 0xffffff00);
		else
			txgapk_info->power_ta[path][i] = (s32)(ta[i]);
		
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	power	ta[%d][%d]=0x%x\n",
			path, i, txgapk_info->power_ta[path][i]);
	}
	#endif

	for (i = 0; i < 17; i++) {
		if (calcu_ta[i] & BIT(5))
			txgapk_info->power_ta[path][i] = (s32)(calcu_ta[i] | 0xffffffc0);
		else
			txgapk_info->power_ta[path][i] = (s32)(calcu_ta[i]);
#if 0		
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	power	ta[%d][%d]=0x%x\n",
			path, i, txgapk_info->power_ta[path][i]);
#else
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	power	ta[%d][%d]=0x%x\n",
			path, i, calcu_ta[i]);
#endif
	}


}



void _halrf_txgapk_write_power_table_default_5g_8832br
	(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path)
{
	//New reset table method for 52c later series	
	struct halrf_gapk_info *txgapk_info = &rf->gapk;

	u32 i;
	u32 d[17] = {0};
	u32 calcu_ta[17] = {0};

	u32 itqt[2] = {0x81cc, 0x82cc};
	u32 iqk_ctrl_rfc_addr[2] = {0x5670, 0x7670};
	
	u32 gapk_on_tbl_setting[2] = {0x8170, 0x8270};
	u32 gain_stage =  0x0;
			
	u32 process_id1[2] = {0x00001119, 0x00001129};
	u32 process_id2[2] = {0x00001619, 0x00001629};
	

	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s\n", __func__);

	
	halrf_wreg(rf, 0x80e0, 0x000001f0, 0x07);
	halrf_wreg(rf, 0x8038, 0x003f0000, 0x24);

	//value 0 for reset table
	halrf_wreg(rf, gapk_on_tbl_setting[path], 0x001fffff, gain_stage);
	

	halrf_wreg(rf, iqk_ctrl_rfc_addr[path], 0x00000002, 0x1);
	halrf_wreg(rf, itqt[path], MASKDWORD, 0x12);
	halrf_wreg(rf, 0x802c, 0x0fff0000, 0x009);

	#if 0
	halrf_wreg(rf, 0x8000, MASKDWORD, 0x00001119);
	ODM_delay_us(100);
	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);
	#else
	halrf_do_one_shot_8832br(rf, path, 0x8000, MASKDWORD, process_id1[path]);
	#endif
	
	halrf_wreg(rf, itqt[path], 0x0000003f, 0x3f);

	#if 0
	halrf_wreg(rf, 0x8000, MASKDWORD, 0x00001619);
	ODM_delay_us(100);
	halrf_wreg(rf, 0x8010, 0x000000ff, 0x00);
	#else
	halrf_do_one_shot_8832br(rf, path, 0x8000, MASKDWORD, process_id2[path]);
	#endif
	
	
	halrf_wreg(rf, iqk_ctrl_rfc_addr[path], 0x00000002, 0x0);
	halrf_wreg(rf, 0x801c, 0x000e0000, 0x0);	
	//	========END : Do PA GapK =====



	// ===== Read GapK Results, Bcut resolution = 0.0625 dB =====
	halrf_wreg(rf, 0x80d4, MASKDWORD, 0x00130000); //gapk_report

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x3);
	d[0] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[1] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[2] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[3] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x4);
	d[4] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[5] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[6] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[7] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x5);
	d[8] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[9] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[10] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[11] = halrf_rreg(rf, 0x80fc, 0x0fe00000);

	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x6);
	d[12] = halrf_rreg(rf, 0x80fc, 0x0000007f);
	d[13] = halrf_rreg(rf, 0x80fc, 0x00003f80);
	d[14] = halrf_rreg(rf, 0x80fc, 0x001fc000);
	d[15] = halrf_rreg(rf, 0x80fc, 0x0fe00000);
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x7);
	d[16] = halrf_rreg(rf, 0x80fc, 0x0000007f);


	// Get The Value --> Ta 6bit
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0x9);
	calcu_ta[0] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[1] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[2] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[3] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xa);
	calcu_ta[4] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[5] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[6] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[7] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xb);
	calcu_ta[8] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[9] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[10] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[11] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xc);
	calcu_ta[12] = halrf_rreg(rf, 0x80fc, 0x0000007e);
	calcu_ta[13] = halrf_rreg(rf, 0x80fc, 0x00007e00);
	calcu_ta[14] = halrf_rreg(rf, 0x80fc, 0x007e0000);
	calcu_ta[15] = halrf_rreg(rf, 0x80fc, 0x7e000000);
	
	halrf_wreg(rf, 0x80e4, 0x00000f00, 0xd);
	calcu_ta[16] = halrf_rreg(rf, 0x80fc, 0x0000007e);

	//==========
	for (i = 0; i < 17; i++) {
		if (d[i] & BIT(6))
			txgapk_info->power_d[path][i] = (s32)(d[i] | 0xffffff80);
		else
			txgapk_info->power_d[path][i] = (s32)(d[i]);

		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	power	d[%d][%d]=0x%x\n",
			path, i, txgapk_info->power_d[path][i]);
	}

	for (i = 0; i < 17; i++) {
		if (calcu_ta[i] & BIT(5))
			txgapk_info->power_ta[path][i] = (s32)(calcu_ta[i] | 0xffffffc0);
		else
			txgapk_info->power_ta[path][i] = (s32)(calcu_ta[i]);
#if 0		
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	power	ta[%d][%d]=0x%x\n",
			path, i, txgapk_info->power_ta[path][i]);
#else
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]	power	ta[%d][%d]=0x%x\n",
			path, i, calcu_ta[i]);
#endif
	}

}

void _halrf_txgapk_iqk_bk_reg_by_mode_8832br
	(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path, bool is_dbcc)
{
	struct halrf_gapk_info *txgapk_info = &rf->gapk;
	u32 process_id[2] = {0x1219, 0x1229};
	
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s\n", __func__);
	
	if (!is_dbcc) {
		/* no dbcc */

#if 0
		halrf_before_one_shot_enable_8832br(rf);
		halrf_wreg(rf, 0x8000, MASKDWORD, path_setting[path]);

		halrf_one_shot_nctl_done_check_8832br(rf, path);
#else
		halrf_do_one_shot_8832br(rf, path, 0x8000, MASKDWORD, process_id[path]);

		txgapk_info->txgapk_chk_cnt[path][TXGAPK_IQKBK][0] = rf->nctl_ck_times[0]; 
		txgapk_info->txgapk_chk_cnt[path][TXGAPK_IQKBK][1] = rf->nctl_ck_times[1];
#endif		
		
		halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000000);
		halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000000);
		
	} else {
		/* dbcc */
		if (phy == HW_PHY_0) {
			//TBD
		} else if (phy == HW_PHY_1) {
			//TBD
		}
	}
		
	halrf_wrf(rf, path, 0xef, 0x00004, 0x0);
	halrf_wrf(rf, path, 0x0, 0xf0000, 0x3);	
	halrf_wrf(rf, path, 0x5, 0x00001, 0x1);
	
	
}



void _halrf_txgapk_restore_iqk_kmod_bb_8832br
	(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path, bool is_dbcc)
{
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s\n", __func__);

	if (!is_dbcc) {
		/* no dbcc */
		if (path == RF_PATH_A) {
		//	halrf_wreg(rf, 0xc0ec, 0x00006000, varfromtmp);
			halrf_wreg(rf, 0x12b8, 0x40000000, 0x0);
			halrf_wreg(rf, 0x20fc, 0x00010000, 0x1);
			halrf_wreg(rf, 0x20fc, 0x00100000, 0x0);
			halrf_wreg(rf, 0x20fc, 0x01000000, 0x1);
			halrf_wreg(rf, 0x20fc, 0x10000000, 0x0);
			halrf_wreg(rf, 0x5670, MASKDWORD, 0x00000000);
			halrf_wreg(rf, 0x12a0, 0x000ff000, 0x00);
			halrf_wreg(rf, 0x20fc, 0x00010000, 0x0);
			halrf_wreg(rf, 0x20fc, 0x01000000, 0x0);
			halrf_wrf(rf, RF_PATH_A, 0x10005, 0x00001, 0x1);		
		} else if (path == RF_PATH_B) {
		//	halrf_wreg(rf, 0xc1ec, 0x00006000, varfromtmp);
			halrf_wreg(rf, 0x32b8, 0x40000000, 0x0);
			halrf_wreg(rf, 0x20fc, 0x00020000, 0x1);
			halrf_wreg(rf, 0x20fc, 0x00200000, 0x0);
			halrf_wreg(rf, 0x20fc, 0x02000000, 0x1);
			halrf_wreg(rf, 0x20fc, 0x20000000, 0x0);
			halrf_wreg(rf, 0x7670, MASKDWORD, 0x00000000);
			halrf_wreg(rf, 0x32a0, 0x000ff000, 0x00);
			halrf_wreg(rf, 0x20fc, 0x00020000, 0x0);
			halrf_wreg(rf, 0x20fc, 0x02000000, 0x0);
			halrf_wrf(rf, RF_PATH_B, 0x10005, 0x00001, 0x1);			
		}
			
	} else {
		/* dbcc */
		if (phy == HW_PHY_0) {
			//TBD
		} else if (phy == HW_PHY_1) {
			//TBD
		}
	}
	
}





void _halrf_txgapk_main_8832br(struct rf_info *rf,
					enum phl_phy_idx phy, enum rf_path path, bool is_dbcc)
{
	#if 0 //it doesn't need to write default
	_halrf_txgapk_write_track_table_default_8832br(rf, phy, path);
	_halrf_txgapk_write_power_table_default_8832br(rf, phy, path);
	#endif

	//v01_BB_AFE_DPK_S0_20210913
	_halrf_txgapk_bb_afe_by_mode_8832br(rf, phy, path, is_dbcc);
	
	//v02_IQK_Preset_path0_20210804	
	_halrf_txgapk_iqk_preset_by_mode_8832br(rf, phy, path, is_dbcc);
	
	//v03_S0_RF_for_DPK_20210924.txt
	//5g only
	_halrf_txgapk_rf_dpk_5g_8832br(rf, phy, path);
	
	

	//v04_RF_RxDCK_S0_20210818.txt	
	_halrf_txgapk_rf_rxdck_5g_8832br(rf, phy, path);

	
	//v05a_A_S0_GapK_Track_32BR.txt
	_halrf_txgapk_track_table_nctl_5g_8832br(rf, phy, path);

	
	//v05b_A_S0_GapK_Power_32BR.txt
	_halrf_txgapk_power_table_nctl_5g_8832br(rf, phy, path);
	
	

	//V 98_S0_IQK_Reg_Restore_52C_20210804
	_halrf_txgapk_iqk_bk_reg_by_mode_8832br(rf, phy, path, is_dbcc);

	//V99_BB_AFE_DPK_S0_restore_20210913.txt		
	_halrf_txgapk_restore_iqk_kmod_bb_8832br(rf, phy, path, is_dbcc);

	
}



void _halrf_do_reset_tbl_txgapk_8832br(struct rf_info *rf,
					enum phl_phy_idx phy)
{
	struct halrf_gapk_info *txgapk_info = &rf->gapk;
	u8 path;
	
	u32 kip_bkup[TXGAPK_RF_PATH_MAX_8832BR][TXGAPK_KIP_REG_NUM_8832BR] = {{0}};
	//u32 bb_bkup[TXGAPK_BB_REG_NUM_8832BR] = {0};
	u32 rf_bkup[TXGAPK_RF_PATH_MAX_8832BR][TXGAPK_RF_REG_NUM_8832BR] = {{0}};

	u32 kip_reg[] = {0x813c, 0x8124, 0x8120, 0xc0d4, 0xc0d8, 0xc0ec};
	//u32 bb_reg[] = {0x2344, 0xc0d4, 0xc0d8, 0xc1d4, 0xc1d8};
	u32 rf_reg[] = {0x5, 0x10005, 0xdf};


	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s   phy=%d, table_idx = %d\n", __func__, phy,  txgapk_info->txgapk_table_idx);

	#if 0 //no need 
	_txgapk_backup_bb_registers_8832br(rf, bb_reg, bb_bkup, TXGAPK_BB_REG_NUM_8832BR);
	#endif

	for (path = 0; path < TXGAPK_RF_PATH_MAX_8832BR; path++) {	
		_txgapk_backup_kip_8832br(rf, kip_reg, kip_bkup, path, TXGAPK_KIP_REG_NUM_8832BR);
		_txgapk_bkup_rf_8832br(rf, rf_reg, rf_bkup, path, TXGAPK_RF_REG_NUM_8832BR);
	}				

	for (path = 0; path < TXGAPK_RF_PATH_MAX_8832BR; path++) {
		//halrf_wrf(rf, path, 0x18, 0x80000, txgapk_info->txgapk_table_idx);
		//halrf_wrf(rf, path, 0x10018, 0x80000, txgapk_info->txgapk_table_idx);
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> reset table index = %d\n", halrf_rrf(rf, path, 0x18, 0x80000));

		//follow dpk flow
		_halrf_txgapk_rxagc_onoff_8832br(rf, path, false);
	
		//main2 =========================
		//v01_BB_AFE_DPK_S0_20210913
		_halrf_txgapk_bb_afe_by_mode_8832br(rf, phy, path, false);
		
		//v02_IQK_Preset_path0_20210804	
		_halrf_txgapk_iqk_preset_by_mode_8832br(rf, phy, path, false);
		
		//v03_S0_RF_for_DPK_20210924.txt
		_halrf_txgapk_rf_dpk_5g_8832br(rf, phy, path);		

		//v04_RF_RxDCK_S0_20210818.txt
		_halrf_txgapk_rf_rxdck_5g_8832br(rf, phy, path);
		
		//v05a_A_S0_GapK_Track_32BR.txt
		_halrf_txgapk_write_track_table_default_5g_8832br(rf, phy, path);
		
		//v05b_A_S0_GapK_Power_32BR.txt
		_halrf_txgapk_write_power_table_default_5g_8832br(rf, phy, path);
	
		//V 98_S0_IQK_Reg_Restore_52C_20210804
		_halrf_txgapk_iqk_bk_reg_by_mode_8832br(rf, phy, path, false);

		//V99_BB_AFE_DPK_S0_restore_20210913.txt		
		_halrf_txgapk_restore_iqk_kmod_bb_8832br(rf, phy, path, false);
		//======= end =======	
	}

	
	for (path = 0; path < TXGAPK_RF_PATH_MAX_8832BR; path++) {
		_txgapk_reload_kip_8832br(rf, kip_reg, kip_bkup, path, TXGAPK_KIP_REG_NUM_8832BR);
		_txgapk_reload_rf_8832br(rf, rf_reg, rf_bkup, path, TXGAPK_RF_REG_NUM_8832BR);

		//follow dpk flow
		_halrf_txgapk_rxagc_onoff_8832br(rf, path, true);
	}

	#if 0 //no need 
	_txgapk_reload_bb_registers_8832br(rf, bb_reg, bb_bkup, TXGAPK_BB_REG_NUM_8832BR);
	#endif
}



void _halrf_do_non_dbcc_txgapk_8832br(struct rf_info *rf,
					enum phl_phy_idx phy)
{
	struct halrf_gapk_info *txgapk_info = &rf->gapk;
	u8 path;
	
	u32 kip_bkup[TXGAPK_RF_PATH_MAX_8832BR][TXGAPK_KIP_REG_NUM_8832BR] = {{0}};
	//u32 bb_bkup[TXGAPK_BB_REG_NUM_8832BR] = {0};
	u32 rf_bkup[TXGAPK_RF_PATH_MAX_8832BR][TXGAPK_RF_REG_NUM_8832BR] = {{0}};

	u32 kip_reg[] = {0x813c, 0x8124, 0x8120, 0xc0c4, 0xc0e8, 0xc0d4, 0xc0d8, 0xc0ec};
	//u32 bb_reg[] = {0x2344, 0xc0d4, 0xc0d8, 0xc1d4, 0xc1d8};
	u32 rf_reg[] = {0x5, 0x10005, 0xdf};


	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s   phy=%d, table_idx = %d\n", __func__, phy,  txgapk_info->txgapk_table_idx);

	#if 0 //no need 
	_txgapk_backup_bb_registers_8832br(rf, bb_reg, bb_bkup, TXGAPK_BB_REG_NUM_8832BR);
	#endif

	for (path = 0; path < TXGAPK_RF_PATH_MAX_8832BR; path++) {	
		_txgapk_backup_kip_8832br(rf, kip_reg, kip_bkup, path, TXGAPK_KIP_REG_NUM_8832BR);
		_txgapk_bkup_rf_8832br(rf, rf_reg, rf_bkup, path, TXGAPK_RF_REG_NUM_8832BR);
	}				

	for (path = 0; path < TXGAPK_RF_PATH_MAX_8832BR; path++) {
		halrf_wrf(rf, path, 0x18, 0x80000, txgapk_info->txgapk_table_idx);
		halrf_wrf(rf, path, 0x10018, 0x80000, txgapk_info->txgapk_table_idx);

		//follow dpk flow
		_halrf_txgapk_rxagc_onoff_8832br(rf, path, false);
	
		_halrf_txgapk_main_8832br(rf, phy, path, false);
	}

	
	for (path = 0; path < TXGAPK_RF_PATH_MAX_8832BR; path++) {
		_txgapk_reload_kip_8832br(rf, kip_reg, kip_bkup, path, TXGAPK_KIP_REG_NUM_8832BR);
		_txgapk_reload_rf_8832br(rf, rf_reg, rf_bkup, path, TXGAPK_RF_REG_NUM_8832BR);

		//follow dpk flow
		_halrf_txgapk_rxagc_onoff_8832br(rf, path, true);
	}

	#if 0 //no need 
	_txgapk_reload_bb_registers_8832br(rf, bb_reg, bb_bkup, TXGAPK_BB_REG_NUM_8832BR);
	#endif
}


void _halrf_do_dbcc_txgapk_8832br(struct rf_info *rf,
					enum phl_phy_idx phy)
{
	enum rf_path path = 0;
	
	if (phy == HW_PHY_0)
		path = RF_PATH_A;
	else if (phy == HW_PHY_1)
		path = RF_PATH_B;

	
	//DBCC use the same table
	/* 0:table_0, 1:table_1 */
	halrf_wrf(rf, path, 0x18, 0x80000, 0);
	halrf_wrf(rf, path, 0x10018, 0x80000, 0);
}
void _halrf_txgapk_get_ch_info_8832br(struct rf_info *rf, enum phl_phy_idx phy)
{
	struct halrf_gapk_info *txgapk_info = &rf->gapk;
	struct halrf_mcc_info *mcc_info = &rf->mcc_info;
	
	u8 idx = 0;
//	u8 get_empty_table = false;

	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s \n", __func__);

#if 0
	for  (idx = 0;  idx < 2; idx++) {
		if (txgapk_info->txgapk_mcc_ch[idx] == 0) {
			get_empty_table = true;
			break;
		}
	}
	//RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] (1)  idx = %x\n", idx);

	if (false == get_empty_table) {
		idx = txgapk_info->txgapk_table_idx + 1;
		if (idx > 1) {
			idx = 0;
		}		
		//RF_DBG(rf, DBG_RF_IQK, "[IQK]we will replace iqk table index(%d), !!!!! \n", idx);
	}	
	//RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK] (2)  idx = %x\n", idx);
#endif

	idx = mcc_info->table_idx;
	txgapk_info->txgapk_table_idx =  idx;

	txgapk_info->txgapk_mcc_ch[idx] = mcc_info->ch[idx];

	if (mcc_info->ch[idx] != rf->hal_com->band[phy].cur_chandef.center_ch)
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> warning!!!! mcc_info = %d, curr_ch=%d \n",
		mcc_info->ch[idx], rf->hal_com->band[phy].cur_chandef.center_ch);

	txgapk_info->ch[0] = rf->hal_com->band[phy].cur_chandef.center_ch;	
	
}


void  _halrf_sel_hw_table_txgapk_8832br(struct rf_info *rf, bool is_hw) 
{
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s \n", __func__);

	halrf_wrf(rf, RF_PATH_A, 0x10055, 0x80000, is_hw);
	halrf_wrf(rf, RF_PATH_B, 0x10055, 0x80000, is_hw);


}

void halrf_do_txgapk_8832br(struct rf_info *rf,
					enum phl_phy_idx phy)
{
	u8 rfe_type = rf->phl_com->dev_cap.rfe_type;
	
	_halrf_txgapk_get_ch_info_8832br(rf, phy);

	if (rfe_type > 50) { //efem
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s rfe_type = %d, efem, skip txgapk process\n", __func__, rfe_type);
		return;
	}
	
	//move to initial
	//0x10055[19] SW gapk table selection
	//_halrf_sel_hw_table_txgapk_8832br(rf, phy, false); //use sw control
	
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s   phy=%d, dbcc_en = %d\n", __func__, phy, rf->hal_com->dbcc_en);
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> version = 0x%x\n", TXGAPK_VER_8832BR); 
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> before GapK process, 0x%x= 0x%x\n", 0x8010, halrf_rreg(rf, 0x8010, MASKDWORD));
	
	
	if (rf->hal_com->dbcc_en)
		_halrf_do_dbcc_txgapk_8832br(rf, phy);
	else
		_halrf_do_non_dbcc_txgapk_8832br(rf, phy);
	
		
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> After GapK process, 0x%x= 0x%x\n", 0x8010, halrf_rreg(rf, 0x8010, MASKDWORD));
}


void halrf_txgapk_init_8832br(struct rf_info *rf) 
{	
	struct halrf_gapk_info *txgapk_info = &rf->gapk;

	if(!txgapk_info->is_gapk_init) {
		RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s \n", __func__);
		txgapk_info->is_gapk_init = true;

		_halrf_sel_hw_table_txgapk_8832br(rf, false); //use sw control			
	}
}


#if 0
void halrf_txgapk_enable_8832br
	(struct rf_info *rf, enum phl_phy_idx phy)
{
	u8 i;
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s   phy=%d, there is no such function!!\n", __func__, phy);
}
#endif


void halrf_txgapk_write_table_default_8832br
	(struct rf_info *rf, enum phl_phy_idx phy)
{
	RF_DBG(rf, DBG_RF_TXGAPK, "[TXGAPK]======> %s   phy=%d\n", __func__, phy);

	_halrf_do_reset_tbl_txgapk_8832br(rf, phy);
}

#endif
