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

u8 halrf_get_thermal_8832br(struct rf_info *rf, enum rf_path rf_path)
{
	halrf_wrf(rf, rf_path, 0x42, BIT(19), 0x1);
	halrf_wrf(rf, rf_path, 0x42, BIT(19), 0x0);
	halrf_wrf(rf, rf_path, 0x42, BIT(19), 0x1);

	halrf_delay_us(rf, 200);

	return (u8)halrf_rrf(rf, rf_path, 0x42, 0x0007e);
}

u32 halrf_mac_get_pwr_reg_8832br(struct rf_info *rf, enum phl_phy_idx phy,
	u32 addr, u32 mask)
{
	struct rtw_hal_com_t *hal = rf->hal_com;
	u32 result, ori_val, bit_shift, reg_val;

	result = rtw_hal_mac_get_pwr_reg(hal, phy, addr, &ori_val);
	if (result)
		RF_WARNING("=======>%s Get MAC(0x%x) fail, error code=%d\n",
			__func__, addr, result);
	else
		RF_DBG(rf, DBG_RF_POWER, "Get MAC(0x%x) ok!!! 0x%08x\n",
			addr, ori_val);

	bit_shift = halrf_cal_bit_shift(mask);
	reg_val = (ori_val & mask) >> bit_shift;

	return reg_val;
}

u32 halrf_mac_set_pwr_reg_8832br(struct rf_info *rf, enum phl_phy_idx phy,
	u32 addr, u32 mask, u32 val)
{
	struct rtw_hal_com_t *hal = rf->hal_com;
	u32 result;

	result = rtw_hal_mac_write_msk_pwr_reg(hal, phy, addr, mask, val);
	if (result) {
		RF_WARNING("=======>%s Set MAC(0x%x[0x%08x]) fail, error code=%d\n",
			__func__, addr, mask, result);
		return false;
	} else
		RF_DBG(rf, DBG_RF_POWER, "Set MAC(0x%x[0x%08x])=0x%08x ok!!! \n",
			addr, mask, val);

	return result;
}

bool halrf_wl_tx_power_control_8832br(struct rf_info *rf, u32 tx_power_val)
{
	struct halrf_pwr_info *pwr = &rf->pwr_info;
	u32 result; 
	s32 tmp_pwr;
	u8 phy = 0;
	u32 all_time_control = 0;
	u32 gnt_bt_control = 0;

	RF_DBG(rf, DBG_RF_POWER, "=======>%s\n", __func__);

	all_time_control = tx_power_val & 0xffff;
	gnt_bt_control = (tx_power_val & 0xffff0000) >> 16;

	RF_DBG(rf, DBG_RF_POWER, "[Pwr Ctrl]tx_power_val=0x%x   all_time_control=0x%x   gnt_bt_control=0x%x\n",
		tx_power_val, all_time_control, gnt_bt_control);

	if (all_time_control == 0xffff) {
		/*Coex Disable*/
		pwr->coex_pwr_ctl_enable = false;
		pwr->coex_pwr = 0;
		RF_DBG(rf, DBG_RF_POWER, "[Pwr Ctrl] Coex Disable all_time_control=0xffff!!!\n");
	} else if (all_time_control == 0xeeee) {
		/*DPK Disable*/
		pwr->dpk_pwr_ctl_enable = false;
		pwr->dpk_pwr = 0;
		RF_DBG(rf, DBG_RF_POWER, "[Pwr Ctrl] DPK Disable all_time_control=0xeeee\n");
	} else {
		if (all_time_control & BIT(15)) {
			/*DPK*/
			pwr->dpk_pwr_ctl_enable = true;
			pwr->dpk_pwr = all_time_control & 0x1ff;

			RF_DBG(rf, DBG_RF_POWER, "[Pwr Ctrl] DPK Enable Set pwr->dpk_pwr = %d\n",
				pwr->dpk_pwr);
		} else {
			/*Coex*/
			pwr->coex_pwr_ctl_enable = true;
			pwr->coex_pwr = all_time_control & 0x1ff;

			RF_DBG(rf, DBG_RF_POWER, "[Pwr Ctrl] Coex Enable Set pwr->coex_pwr = %d\n",
				pwr->coex_pwr);
		}
	}

	if (pwr->coex_pwr_ctl_enable == true && pwr->dpk_pwr_ctl_enable == false) {
		tmp_pwr = pwr->coex_pwr;

		RF_DBG(rf, DBG_RF_POWER, "[Pwr Ctrl] coex_pwr_ctl_enable=true dpk_pwr_ctl_enable=false tmp_pwr=%d\n",
			tmp_pwr);
	} else if (pwr->coex_pwr_ctl_enable == false && pwr->dpk_pwr_ctl_enable == true) {
		tmp_pwr = pwr->dpk_pwr;

		RF_DBG(rf, DBG_RF_POWER, "[Pwr Ctrl] coex_pwr_ctl_enable=false dpk_pwr_ctl_enable=true tmp_pwr=%d\n",
			tmp_pwr);
	} else if (pwr->coex_pwr_ctl_enable == true && pwr->dpk_pwr_ctl_enable == true) {
		if (pwr->coex_pwr > pwr->dpk_pwr)
			tmp_pwr = pwr->dpk_pwr;
		else
			tmp_pwr = pwr->coex_pwr;

		RF_DBG(rf, DBG_RF_POWER, "[Pwr Ctrl] coex_pwr_ctl_enable=true dpk_pwr_ctl_enable=true tmp_pwr=%d\n",
			tmp_pwr);
	} else
		tmp_pwr = 0;

	if (pwr->coex_pwr_ctl_enable == false && pwr->dpk_pwr_ctl_enable == false) {
		/*all-time control Disable*/
		result = halrf_mac_set_pwr_reg_8832br(rf, phy, 0xd200, 0xfffffc00, 0x0);

		if (result) {
			RF_WARNING("=======>%s Set MAC(0xd200) fail, error code=%d\n",
				__func__, result);
			return false;
		} else {
			RF_DBG(rf, DBG_RF_POWER, "Set MAC(0xd200) ok!!!\n");
			rf->is_coex = false;
		}
	} else {
		/*all-time control*/
		result = halrf_mac_set_pwr_reg_8832br(rf, phy, 0xd200, 0xfffffc00, ((tmp_pwr & 0x1ff) | BIT(9)));
		if (result) {
			RF_WARNING("=======>%s Set MAC(0xd200) fail, error code=%d\n",
				__func__, result);
			return false;
		} else {
			RF_DBG(rf, DBG_RF_POWER, "Set MAC(0xd200) ok!!!\n");
			rf->is_coex = true;
		}
	}

	if (gnt_bt_control == 0xffff) {
		/*GNT_BT control*/

		RF_DBG(rf, DBG_RF_POWER, "=======>%s   gnt_bt_control = 0x%x\n",
			__func__, gnt_bt_control);

		result = halrf_mac_set_pwr_reg_8832br(rf, phy, 0xd220, BIT(1), 0x0);
		result = halrf_mac_set_pwr_reg_8832br(rf, phy, 0xd220, 0xfffff007, 0x0);
		if (result) {
			RF_WARNING("=======>%s Set MAC(0xd220) fail, error code=%d\n",
				__func__, result);
			return false;
		} else {
			RF_DBG(rf, DBG_RF_POWER, "Set MAC(0xd220) ok!!!\n");
			rf->is_coex = false;
		}
	} else {
		/*GNT_BT control*/

		RF_DBG(rf, DBG_RF_POWER, "=======>%s   gnt_bt_control = 0x%x\n",
			__func__, gnt_bt_control);

		result = halrf_mac_set_pwr_reg_8832br(rf, phy, 0xd220, BIT(1), 0x1);
		result = halrf_mac_set_pwr_reg_8832br(rf, phy, 0xd220, 0xfffff007, ((gnt_bt_control & 0x1ff) << 3));
		if (result) {
			RF_WARNING("=======>%s Set MAC(0xd220) fail, error code=%d\n",
				__func__, result);
			return false;
		} else {
			RF_DBG(rf, DBG_RF_POWER, "Set MAC(0xd220) ok!!!\n");
			rf->is_coex = true;
		}
	}

	return true;
}

s8 halrf_get_ther_protected_threshold_8832br(struct rf_info *rf)
{
	u8 tmp_a, tmp_b, tmp;

	tmp_a = halrf_get_thermal(rf, RF_PATH_A);
	tmp_b = halrf_get_thermal(rf, RF_PATH_B);

	if (tmp_a > tmp_b)
		tmp = tmp_a;
	else
		tmp = tmp_b;

	if (tmp > 0x32)
		return -1;	/*Tx duty reduce*/
	else if (tmp < 0x31)
		return 1;	/*Tx duty up*/
	else 
		return 0;	/*Tx duty the same*/
}

s8 halrf_xtal_tracking_offset_8832br(struct rf_info *rf,
					enum phl_phy_idx phy)
{
	struct halrf_xtal_info *xtal_trk = &rf->xtal_track;
	u8 thermal_a = 0xff, thermal_b = 0xff;
	u8 tmp_a, tmp_b, tmp;
	s8 xtal_ofst = 0;

	RF_DBG(rf, DBG_RF_XTAL_TRACK, "======>%s   phy=%d\n",
		__func__, phy);

	tmp_a = halrf_get_thermal(rf, RF_PATH_A);
	tmp_b = halrf_get_thermal(rf, RF_PATH_B);
	halrf_efuse_get_info(rf, EFUSE_INFO_RF_THERMAL_A, &thermal_a, 1);
	halrf_efuse_get_info(rf, EFUSE_INFO_RF_THERMAL_B, &thermal_b, 1);

	if (thermal_a == 0xff || thermal_b == 0xff ||
		thermal_a == 0x0 || thermal_b == 0x0) {
		RF_DBG(rf, DBG_RF_XTAL_TRACK, "======>%s PG ThermalA=%d ThermalB=%d\n",
			__func__, thermal_a, thermal_b);
		return 0;
	}

	if (tmp_a > tmp_b) {
		if (tmp_a > thermal_a) {
			tmp = tmp_a - thermal_a;
			if (tmp > DELTA_SWINGIDX_SIZE)
				tmp = DELTA_SWINGIDX_SIZE - 1;
			xtal_ofst = xtal_trk->delta_swing_xtal_table_idx_p[tmp];
		} else {
			tmp = thermal_a - tmp_a;
			if (tmp > DELTA_SWINGIDX_SIZE)
				tmp = DELTA_SWINGIDX_SIZE - 1;
			xtal_ofst = xtal_trk->delta_swing_xtal_table_idx_n[tmp];
		}
	} else {
		if (tmp_b > thermal_b) {
			tmp = tmp_b - thermal_b;
			if (tmp > DELTA_SWINGIDX_SIZE)
				tmp = DELTA_SWINGIDX_SIZE - 1;
			xtal_ofst = xtal_trk->delta_swing_xtal_table_idx_p[tmp];
		} else {
			tmp = thermal_b - tmp_b;
			if (tmp > DELTA_SWINGIDX_SIZE)
				tmp = DELTA_SWINGIDX_SIZE - 1;
			xtal_ofst = xtal_trk->delta_swing_xtal_table_idx_n[tmp];
		}
	}

	RF_DBG(rf, DBG_RF_XTAL_TRACK, "PG ThermalA=%d   ThermalA=%d\n",
		thermal_a, tmp_a);

	RF_DBG(rf, DBG_RF_XTAL_TRACK, "PG ThermalB=%d   ThermalB=%d\n",
		thermal_b, tmp_b);

	RF_DBG(rf, DBG_RF_XTAL_TRACK, "xtal_ofst[%d]=%d\n",
		tmp, xtal_ofst);

	return xtal_ofst;
}

void halrf_txck_force_8832br(struct rf_info *rf, enum rf_path path, bool force, enum dac_ck ck)
{
	halrf_wreg(rf, 0x12a0 | (path <<13), BIT(15), 0x0);

	if (!force)
		return;

	halrf_wreg(rf, 0x12a0 | (path <<13), 0x7000, ck);
	halrf_wreg(rf, 0x12a0 | (path <<13), BIT(15), 0x1);
}

u32 ck480_cav_8832br[][6] = {{0x8, 0x2, 0x3, 0xc, 0xc, 0x9}, {0x5, 0x0, 0x3, 0xe, 0xf, 0x9}};
u32 ck960_cav_8832br[][6] = {{0x8, 0x2, 0x2, 0x5, 0x9, 0xa}, {0x8, 0x2, 0x2, 0x4, 0xf, 0xa}};
u32 ck1920_cav_8832br[] = {0x9, 0x0, 0x0, 0x3, 0xf, 0xa};
u32 ck960_cbv_8832br[] = {0x8, 0x2, 0x2, 0x4, 0xf, 0xa};
u32 ck1920_new_8832br[] = {0x9, 0x0, 0x0, 0x3, 0xf, 0xa};
void halrf_rxck_force_8832br(struct rf_info *rf, enum rf_path path, bool force, enum adc_ck ck)
{
	struct rtw_hal_com_t *hal_i = rf->hal_com;
	struct halrf_kfree_info *kfree = &rf->kfree_info;
	u8 efuse_idx;
	u32 *data;

	efuse_idx = kfree->efuse_content[0xf] & 0x7;
	halrf_wreg(rf, 0x12a0 | (path <<13), BIT(19), 0x0);
	if (!force)
		return;
	halrf_wreg(rf, 0x12a0 | (path <<13), 0x70000, ck);
	halrf_wreg(rf, 0x12a0 | (path <<13), BIT(19), 0x1);
	if (hal_i->cv == CAV) {
		switch (ck) {
		case ADC_480M:
			if (efuse_idx == 3)
				data = ck480_cav_8832br[0];
			else
				data = ck480_cav_8832br[1];
			break;
		case ADC_960M:
			if (efuse_idx == 3)
				data = ck960_cav_8832br[0];
			else
				data = ck960_cav_8832br[1];
			break;
		case ADC_1920M:
			data = ck1920_cav_8832br;
			break;
		default:
			data = ck1920_cav_8832br;
			break;
		}
	} else {
		switch (ck) {
		case ADC_960M:
			data = ck960_cbv_8832br;
			break;
		case ADC_1920M:
			data = ck1920_new_8832br;
			break;
		default:
			data = ck1920_new_8832br;
			break;
		}
	}
	halrf_wreg(rf, 0xc0d4 | (path << 8), 0x780, data[0]);
	halrf_wreg(rf, 0xc0d4 | (path << 8), 0x7800, data[1]);
	halrf_wreg(rf, 0xc0d4 | (path << 8), 0x0c000000, data[2]);
	halrf_wreg(rf, 0xc0d8 | (path << 8), 0x1e0, data[3]);
	halrf_wreg(rf, 0xc0c4 | (path << 8), 0x3e0000, data[4]);
	halrf_wreg(rf, 0xc0e8 | (path << 8), 0xffff0000, data[5]);
}

u32 bkup_kip_reg_8832br[] = {0x813c, 0x8124, 0x8120, 0xc0d4, 0xc0d8, 0xc0ec};
u32 bkup_bb_reg_8832br[] = {0x2344};
u32 bkup_rf_reg_8832br[] = {0x5, 0x10005, 0xdf};

u32 bkup_kip_val_8832br[RF_PATH_MAX_NUM][RF_BACKUP_KIP_REG_MAX_NUM] = {{0x0}};
u32 bkup_bb_val_8832br[RF_BACKUP_BB_REG_MAX_NUM] = {0x0};
u32 bkup_rf_val_8832br[RF_PATH_MAX_NUM][RF_BACKUP_RF_REG_MAX_NUM] = {{0x0}};

void do_bkup_kip_8832br(struct rf_info *rf, u8 path)
{
	u8 i;
	u32 num = ARRAY_SIZE(bkup_kip_reg_8832br);

	for (i = 0; i < num; i++) {
		if (i >= RF_BACKUP_KIP_REG_MAX_NUM) {
			RF_DBG(rf, DBG_RF_RFK,
			       "[RFK] %s backup size not enough\n", __func__);
			break;
		}
		bkup_kip_val_8832br[path][i] = halrf_rreg(rf, bkup_kip_reg_8832br[i] + (path << 8), MASKDWORD);
		
		RF_DBG(rf, DBG_RF_RFK, "[RFK] Backup KIP(S%d) 0x%x = %x\n", path, bkup_kip_reg_8832br[i] + (path << 8), bkup_kip_val_8832br[path][i]);
	}
}

void do_bkup_bb_8832br(struct rf_info *rf)
{
	u32 i;
	u32 num = ARRAY_SIZE(bkup_bb_reg_8832br);

	for (i = 0; i < num; i++) {
		if (i >= RF_BACKUP_BB_REG_MAX_NUM) {
			RF_DBG(rf, DBG_RF_RFK,
			       "[RFK] %s backup size not enough\n", __func__);
			break;
		}
		bkup_bb_val_8832br[i] =
			halrf_rreg(rf, bkup_bb_reg_8832br[i], MASKDWORD);

		RF_DBG(rf, DBG_RF_RFK, "[RFK]backup bb reg : %x, value =%x\n",
		      bkup_bb_reg_8832br[i], bkup_bb_val_8832br[i]);
	}
}


void do_bkup_rf_8832br(struct rf_info *rf, u8 path)
{
	u8 i;
	u32 num = ARRAY_SIZE(bkup_rf_reg_8832br);

	for (i = 0; i < num; i++) {
		if (i >= RF_BACKUP_RF_REG_MAX_NUM) {
			RF_DBG(rf, DBG_RF_RFK,
			       "[RFK] %s backup size not enough\n", __func__);
			break;
		}
		bkup_rf_val_8832br[path][i] = halrf_rrf(rf, path, bkup_rf_reg_8832br[i], MASKRF);
		
		RF_DBG(rf, DBG_RF_RFK, "[RFK] Backup RF S%d 0x%x = %x\n",
			path, bkup_rf_reg_8832br[i], bkup_rf_val_8832br[path][i]);
	}
}

void do_reload_kip_8832br(struct rf_info *rf, u8 path) 
{
	u8 i;
	u32 num = ARRAY_SIZE(bkup_kip_reg_8832br);

	for (i = 0; i < num; i++) {
		halrf_wreg(rf, bkup_kip_reg_8832br[i] + (path << 8), MASKDWORD, bkup_kip_val_8832br[path][i]);
		
		RF_DBG(rf, DBG_RF_RFK, "[RFK] Reload KIP(S%d) 0x%x = %x\n",path, bkup_kip_reg_8832br[i] + (path << 8),
			bkup_kip_val_8832br[path][i]);
	}
}

void do_reload_bb_8832br(struct rf_info *rf)
{
	u32 i;
	u32 num = ARRAY_SIZE(bkup_bb_reg_8832br);

	for (i = 0; i < num; i++) {
		halrf_wreg(rf,  bkup_bb_reg_8832br[i], MASKDWORD, bkup_bb_val_8832br[i]);

		RF_DBG(rf, DBG_RF_RFK, "[RFK] Reload BB 0x%x = 0x%x\n",
		       bkup_bb_reg_8832br[i], bkup_bb_val_8832br[i]);
	}
}


void do_reload_rf_8832br(struct rf_info *rf, u8 path)
{
	u8 i;
	u32 num = ARRAY_SIZE(bkup_rf_reg_8832br);

	for (i = 0; i < num; i++) {
		halrf_wrf(rf, path, bkup_rf_reg_8832br[i], MASKRF, bkup_rf_val_8832br[path][i]);
		
			RF_DBG(rf, DBG_RF_RFK, "[RFK] Reload RF S%d 0x%x = %x\n",
				path, bkup_rf_reg_8832br[i], bkup_rf_val_8832br[path][i]);
	}
}

void halrf_set_regulation_from_driver_8832br(struct rf_info *rf,
	u8 regulation_idx)
{
	struct halrf_pwr_info *pwr = &rf->pwr_info;

	pwr->regulation_idx = regulation_idx;
	RF_DBG(rf, DBG_RF_POWER, "Set regulation_idx=%d\n", pwr->regulation_idx);
}

void halrf_op5k_init_8832br(struct rf_info *rf)
{
	struct halrf_op5k_info *op5k = &rf->op5k_info;

	if (rf->hal_com->cv == CAV) {
		RF_DBG(rf, DBG_RF_OP5K, "[OP5K] =======> %s rf->hal_com->cv == CAV Return!!!\n", __func__);
		return;
	}

	op5k->thermal_base_op5k[RF_PATH_A] = 30;
	op5k->thermal_base_op5k[RF_PATH_B] = 30;

	op5k->thermal_op5k[RF_PATH_A] = op5k->thermal_base_op5k[RF_PATH_A];
	op5k->thermal_op5k[RF_PATH_B] = op5k->thermal_base_op5k[RF_PATH_B];
}

#define OP5K_MAX_RF_PATH_8832BR 2
void halrf_op5k_trigger_8832br(struct rf_info *rf)
{
	struct halrf_op5k_info *op5k = &rf->op5k_info;
	u32 i, j, k;
	u32 op5_diff = 5;
	s32 upper_op5[OP5K_MAX_RF_PATH_8832BR] = {0};
	s32 lower_op5[OP5K_MAX_RF_PATH_8832BR] = {0};
	u32 zero_rst_cnt[MAX_RF_PATH][OP5K_RESET_CNT_DATA];
	u32 zero_rst_cnt_start[MAX_RF_PATH][OP5K_RESET_CNT_DATA];
	u32 zero_rst_cnt_end[MAX_RF_PATH][OP5K_RESET_CNT_DATA];
	u32 max_zero[MAX_RF_PATH], max_zero_idx[MAX_RF_PATH];
	u32 retry[MAX_RF_PATH] = {0};

	if (rf->hal_com->cv == CAV) {
		RF_DBG(rf, DBG_RF_OP5K, "[OP5K] =======> %s rf->hal_com->cv == CAV Return!!!\n", __func__);
		return;
	}

	RF_DBG(rf, DBG_RF_OP5K, "[OP5K] =======> %s\n", __func__);

	if (op5k->op5k_progress == true) {
		RF_DBG(rf, DBG_RF_OP5K,
			"[OP5K] =======> %s op5k->op5k_progress == true Return!!!\n", __func__);
		return;
	}

	op5k->op5k_progress = true;

	while ((retry[RF_PATH_A] < 2 || retry[RF_PATH_B] < 2)) {
		RF_DBG(rf, DBG_RF_OP5K, "[OP5K] RetryA=%d   RetryB=%d\n", retry[RF_PATH_A], retry[RF_PATH_B]);
		halrf_mem_set(rf, op5k->rst_cnt, 0x0, sizeof(op5k->rst_cnt));
		halrf_mem_set(rf, op5k->rst_cnt_zero, 0x0, sizeof(op5k->rst_cnt_zero));
		halrf_mem_set(rf, op5k->rst_cnt_final, 0x0, sizeof(op5k->rst_cnt_final));
		halrf_mem_set(rf, op5k->op5k_backup, 0x0, sizeof(op5k->op5k_backup));
		halrf_mem_set(rf, zero_rst_cnt, 0x0, sizeof(zero_rst_cnt));
		halrf_mem_set(rf, zero_rst_cnt_start, 0x0, sizeof(zero_rst_cnt_start));
		halrf_mem_set(rf, zero_rst_cnt_end, 0x0, sizeof(zero_rst_cnt_end));
		halrf_mem_set(rf, max_zero, 0x0, sizeof(max_zero));
		halrf_mem_set(rf, max_zero_idx, 0x0, sizeof(max_zero_idx));

		op5k->op5k_backup[RF_PATH_A] = halrf_rreg(rf, 0xc0d8, 0x000001e0);
		op5k->op5k_backup[RF_PATH_B] = halrf_rreg(rf, 0xc1d8, 0x000001e0);

		RF_DBG(rf, DBG_RF_OP5K, "[OP5K] Last time OP5K result PathA=0x%x PathB=0x%x\n",
			op5k->op5k_backup[RF_PATH_A] , op5k->op5k_backup[RF_PATH_B]);

		/*01_ADC_input_short*/
		halrf_wreg(rf, 0xc0d4, 0x00000010, 0x1);
		halrf_wreg(rf, 0xc1d4, 0x00000010, 0x1);
		halrf_wreg(rf, 0x12b8, 0x40000000, 0x1);
		halrf_wreg(rf, 0x32b8, 0x40000000, 0x1);
		halrf_wreg(rf, 0x030c, 0xff000000, 0xf);
		halrf_delay_us(rf, 1);
		halrf_wreg(rf, 0x030c, 0xff000000, 0x3);
		halrf_wreg(rf, 0x032c, 0xffff0000, 0x0);
		halrf_delay_us(rf, 1);
		halrf_wreg(rf, 0x032c, 0xffff0000, 0x40);
		halrf_wreg(rf, 0xc0f8, 0x00000006, 0x3);
		halrf_wreg(rf, 0xc1f8, 0x00000006, 0x3);
		halrf_delay_us(rf, 1);
		halrf_wreg(rf, 0xc0f8, 0x00000006, 0x2);
		halrf_wreg(rf, 0xc1f8, 0x00000006, 0x2);
		halrf_delay_us(rf, 1);

		/*OP5K*/
		for (i = 0; i < OP5K_RESET_CNT_DATA; i++) {
			for (j = 0; j < OP5K_MAX_RF_PATH_8832BR; j++) {
				halrf_wreg(rf, (0xc0d8 | j << 8), 0x000001e0, i);
		 		op5k->rst_cnt[j][i] = halrf_rreg(rf, (0xc0fc | j << 8), 0x0ff00000);

#if 0
				if (op5k->rst_cot[j][i] == 0 && op5k->rst_cot_zero[j][0] == 0)
					op5k->rst_cot_zero[j][0] = i;
				else if (op5k->rst_cot[j][i] == 0)
					op5k->rst_cot_zero[j][1] = i;
#endif
				/*reset cnt*/
				halrf_wreg(rf, 0x030c, 0xff000000, 0xf);
				halrf_delay_us(rf, 1);
				halrf_wreg(rf, 0x030c, 0xff000000, 0x3);
				halrf_wreg(rf, 0x032c, 0xffff0000, 0x0);
				halrf_delay_us(rf, 1);
				halrf_wreg(rf, 0x032c, 0xffff0000, 0x40);
				halrf_wreg(rf, 0xc0f8, 0x00000006, 0x3);
				halrf_wreg(rf, 0xc1f8, 0x00000006, 0x3);
				halrf_delay_us(rf, 1);
				halrf_wreg(rf, 0xc0f8, 0x00000006, 0x2);
				halrf_wreg(rf, 0xc1f8, 0x00000006, 0x2);
				halrf_delay_us(rf, 1);
			}
		}

		/*Count the number of 0*/
		for (j = 0; j < OP5K_MAX_RF_PATH_8832BR; j++) {
			for (i = 0, k = 0; i < OP5K_RESET_CNT_DATA; i++) {
				if (zero_rst_cnt[j][k] == 0 && op5k->rst_cnt[j][i] == 0) {
					zero_rst_cnt[j][k]++;
					zero_rst_cnt_start[j][k] = i;
					zero_rst_cnt_end[j][k] = i;
				} else {
		 			if (op5k->rst_cnt[j][i - 1] == 0 && op5k->rst_cnt[j][i] == 0) {
						zero_rst_cnt[j][k]++;
						zero_rst_cnt_end[j][k] = i;
					} else if(zero_rst_cnt[j][k] != 0 && op5k->rst_cnt[j][i] == 0) {
						k++;
						zero_rst_cnt[j][k]++;
						zero_rst_cnt_start[j][k] = i;
						zero_rst_cnt_end[j][k] = i;
					}
				}
			}
		}

		for (j = 0; j < OP5K_MAX_RF_PATH_8832BR; j++) {
			for (k = 0; k < OP5K_RESET_CNT_DATA; k++) {
				if (zero_rst_cnt[j][k] != 0) {
					RF_DBG(rf, DBG_RF_OP5K, "[OP5K] Path:%d zero_rst_cnt[%d][%d]=%d\n",
						j, j, k, zero_rst_cnt[j][k]);
					RF_DBG(rf, DBG_RF_OP5K, "[OP5K] Path:%d zero_rst_cnt_start[%d][%d]=%d\n",
						j, j, k, zero_rst_cnt_start[j][k]);
					RF_DBG(rf, DBG_RF_OP5K, "[OP5K] Path:%d zero_rst_cnt_end[%d][%d]=%d\n",
						j, j, k, zero_rst_cnt_end[j][k]);
				}

				if (zero_rst_cnt[j][k] > max_zero[j]) {
					max_zero[j] = zero_rst_cnt[j][k];
					max_zero_idx[j] = k;
				}
			}

			op5k->rst_cnt_zero[j][0] = zero_rst_cnt_start[j][max_zero_idx[j]];
			op5k->rst_cnt_zero[j][1] = zero_rst_cnt_end[j][max_zero_idx[j]];
				
			RF_DBG(rf, DBG_RF_OP5K, "[OP5K] Path:%d MAX zero_rst_cnt[%d][%d]=%d\n",
				j, j, max_zero_idx[j], zero_rst_cnt[j][max_zero_idx[j]]);
			RF_DBG(rf, DBG_RF_OP5K, "[OP5K] Path:%d MAX zero_rst_cnt_start[%d][%d]=%d\n",
				j, j, max_zero_idx[j], zero_rst_cnt_start[j][max_zero_idx[j]]);
			RF_DBG(rf, DBG_RF_OP5K, "[OP5K] Path:%d MAX zero_rst_cnt_end[%d][%d]=%d\n",
				j, j, max_zero_idx[j], zero_rst_cnt_end[j][max_zero_idx[j]]);
		}

		for (j = 0; j < OP5K_MAX_RF_PATH_8832BR; j++) {
			upper_op5[j] = op5k->op5k_backup[j] + op5_diff;
			lower_op5[j] = op5k->op5k_backup[j] - op5_diff;

			op5k->rst_cnt_final[j] = (op5k->rst_cnt_zero[j][0] + op5k->rst_cnt_zero[j][1]) / 2;

			RF_DBG(rf, DBG_RF_OP5K, "[OP5K] Path:%d rst_cnt_final=%d upper_op5=%d lower_op5=%d zero_rst_cnt=%d\n",
				j, op5k->rst_cnt_final[j], upper_op5[j], lower_op5[j], zero_rst_cnt[j][max_zero_idx[j]]);

			if (op5k->rst_cnt_final[j] <= upper_op5[j] && op5k->rst_cnt_final[j] >= lower_op5[j] &&
				zero_rst_cnt[j][max_zero_idx[j]] >= 3) {
				halrf_wreg(rf, (0xc0d8 | j << 8), 0x000001e0, op5k->rst_cnt_final[j] & 0xf);
				retry[j] = 2;
				RF_DBG(rf, DBG_RF_OP5K, "[OP5K] Set New Value !!!\n");
			} else {
				halrf_wreg(rf, (0xc0d8 | j << 8), 0x000001e0, op5k->op5k_backup[j] & 0xf);
				op5k->rst_cnt_final[j] = op5k->op5k_backup[j];
				RF_DBG(rf, DBG_RF_OP5K, "[OP5K] Set Default Value !!!\n");
				retry[j]++;
			}

			op5k->thermal_op5k[j] = halrf_get_thermal(rf, j);
			
			for (i = 0; i < OP5K_RESET_CNT_DATA; i++)
				RF_DBG(rf, DBG_RF_OP5K, "[OP5K] Path:%d rst_cot[%d][%d]=0x%x\n", j, j, i, op5k->rst_cnt[j][i]);
			
			RF_DBG(rf, DBG_RF_OP5K, "Path:%d rst_cot_final(0x%x) = (rst_cot_zero_1(0x%x) + rst_cot_zero_2(0x%x)) / 2\n",
				j, op5k->rst_cnt_final[j], op5k->rst_cnt_zero[j][0], op5k->rst_cnt_zero[j][1]);
		}

		RF_DBG(rf, DBG_RF_OP5K, "[OP5K] OP5K 0xc0d8[8:5]=0x%x 0xc1d8[8:5]=0x%x\n",
			halrf_rreg(rf, 0xc0d8, 0x000001e0), halrf_rreg(rf, 0xc1d8, 0x000001e0));

		RF_DBG(rf, DBG_RF_OP5K, "[OP5K] OP5K thermal_op5k[RF_PATH_A]=%d op5k->thermal_op5k[RF_PATH_B]=%d\n",
			op5k->thermal_op5k[RF_PATH_A], op5k->thermal_op5k[RF_PATH_B]);

		/*03_ADC_input_short_Relaod*/
		halrf_wreg(rf, 0xc0d4, 0x00000010, 0x0);
		halrf_wreg(rf, 0xc1d4, 0x00000010, 0x0);
		halrf_wreg(rf, 0x12b8, 0x40000000, 0x0);
		halrf_wreg(rf, 0x32b8, 0x40000000, 0x0);
	}

	op5k->op5k_progress = false;
}

void halrf_op5k_tracking_8832br(struct rf_info *rf)
{
	struct halrf_op5k_info *op5k = &rf->op5k_info;
	u8 i = 0, path;
	u8 thermal_op5k_avg_count = 0, thermal_value[OP5K_MAX_RF_PATH_8832BR] = {0};
	u32 thermal_op5k_avg[OP5K_MAX_RF_PATH_8832BR] = {0};
	s8 delta_op5k[OP5K_MAX_RF_PATH_8832BR] = {0}, delta_base_op5k[OP5K_MAX_RF_PATH_8832BR] = {0};

#if 0
	if (op5k->thermal_op5k[RF_PATH_A] == 0 || op5k->thermal_op5k[RF_PATH_B] == 0) {
		RF_DBG(rf, DBG_RF_OP5K_TRACK,
			"[OP5K_TRK] op5k->thermal_op5k[A]=%d op5k->thermal_op5k[B]=%d Retrun !!!\n",
			op5k->thermal_op5k[RF_PATH_A], op5k->thermal_op5k[RF_PATH_B]);
		return;
	}
#endif

	if (rf->hal_com->cv == CAV) {
		RF_DBG(rf, DBG_RF_OP5K_TRACK, "[OP5K] =======> %s rf->hal_com->cv == CAV Return!!!\n", __func__);
		return;
	}

	if (op5k->op5k_set_ch_chk == false) {
		RF_DBG(rf, DBG_RF_OP5K_TRACK,
			"[OP5K_TRK] =======> %s   op5k->op5k_set_ch_chk=%d   Retrun !!!\n",
			__func__, op5k->op5k_set_ch_chk);
		return;
	}

	RF_DBG(rf, DBG_RF_OP5K_TRACK,
	       "[OP5K_TRK] ================ 8832BR OP5K [BW=%d]================\n",
	       rf->hal_com->band[HW_PHY_0].cur_chandef.bw);

	/*get thermal meter*/
	for (path = 0; path < 2; path++) {
		thermal_value[path] = halrf_get_thermal(rf, path);

		RF_DBG(rf, DBG_RF_OP5K_TRACK,
		       "[OP5K_TRK] S%d thermal now = %d\n", path, thermal_value[path]);
	}

	op5k->thermal_op5k_avg[RF_PATH_A][op5k->thermal_op5k_avg_index] =
		thermal_value[RF_PATH_A];
	op5k->thermal_op5k_avg[RF_PATH_B][op5k->thermal_op5k_avg_index] =
		thermal_value[RF_PATH_B];
	op5k->thermal_op5k_avg_index++;

	/*Average times */
	if (op5k->thermal_op5k_avg_index == OP5K_AVG_THERMAL_NUM)
		op5k->thermal_op5k_avg_index = 0;

	for (i = 0; i < OP5K_AVG_THERMAL_NUM; i++) {
		if (op5k->thermal_op5k_avg[RF_PATH_A][i] ||
		    op5k->thermal_op5k_avg[RF_PATH_B][i]) {
			thermal_op5k_avg[RF_PATH_A] += op5k->thermal_op5k_avg[RF_PATH_A][i];
			thermal_op5k_avg[RF_PATH_B] += op5k->thermal_op5k_avg[RF_PATH_B][i];
			thermal_op5k_avg_count++;
		}
	}

	/*Calculate Average ThermalValue after average enough times*/
	if (thermal_op5k_avg_count) {
#if 0
		RF_DBG(rf, DBG_RF_OP5K_TRACK,
		       "[OP5K_TRK] S0 ThermalValue_OP5K_AVG (count) = %d (%d))\n",
		       thermal_op5k_avg[RF_PATH_A], thermal_op5k_avg_count);

		RF_DBG(rf, DBG_RF_OP5K_TRACK,
		       "[OP5K_TRK] S1 ThermalValue_OP5K_AVG (count) = %d (%d))\n",
		       thermal_op5k_avg[RF_PATH_B], thermal_op5k_avg_count);
#endif
		thermal_value[RF_PATH_A] = (u8)(thermal_op5k_avg[RF_PATH_A] / thermal_op5k_avg_count);
		thermal_value[RF_PATH_B] = (u8)(thermal_op5k_avg[RF_PATH_B] / thermal_op5k_avg_count);
	}

	for (path = 0; path < OP5K_MAX_RF_PATH_8832BR; path++) {
		delta_op5k[path] = thermal_value[path] - op5k->thermal_op5k[path];
		delta_base_op5k[path] = thermal_value[path] - op5k->thermal_base_op5k[path];

		RF_DBG(rf, DBG_RF_OP5K_TRACK, "[OP5K_TRK] Path:%d (Org_ther, ther, base_ther, delta_OP5K) = (%d, %d, %d, %d)\n",
			path, delta_base_op5k[path], thermal_value[path], op5k->thermal_op5k[path], delta_op5k[path]);
	}

	if (delta_base_op5k[RF_PATH_A] < 0 || delta_base_op5k[RF_PATH_B] < 0) {
		if ((delta_op5k[RF_PATH_A] >= OP5K_THER_THRESHOLD || delta_op5k[RF_PATH_A] <= OP5K_THER_THRESHOLD * -1) ||
			(delta_op5k[RF_PATH_B] >= OP5K_THER_THRESHOLD || delta_op5k[RF_PATH_B] <= OP5K_THER_THRESHOLD * -1)) {
			halrf_op5k_trigger_8832br(rf);
			op5k->op5k_ther_chk = true;
			RF_DBG(rf, DBG_RF_OP5K_TRACK, "[OP5K_TRK] Do OP5K Trigger !!!\n");
		}
	} else if (op5k->op5k_ther_chk == true) {
		halrf_wreg(rf, 0xc0d8, 0x000001e0, op5k->op5k_default[RF_PATH_A] & 0xf);
		halrf_wreg(rf, 0xc1d8, 0x000001e0, op5k->op5k_default[RF_PATH_B] & 0xf);
		op5k->op5k_ther_chk = false;
		RF_DBG(rf, DBG_RF_OP5K_TRACK, "[OP5K_TRK] Set OP5K Default !!!\n");
		RF_DBG(rf, DBG_RF_OP5K_TRACK, "[OP5K_TRK] 0xc0d8=0x%x\n", halrf_rreg(rf, 0xc0d8, 0x000001e0));
		RF_DBG(rf, DBG_RF_OP5K_TRACK, "[OP5K_TRK] 0xc1d8=0x%x\n", halrf_rreg(rf, 0xc1d8, 0x000001e0));
	}
}

void halrf_bw_setting_op5k_8832br(struct rf_info *rf)
{
	struct halrf_op5k_info *op5k = &rf->op5k_info;

	RF_DBG(rf, DBG_RF_OP5K, "[OP5K] =======> %s\n", __func__);

	if (rf->hal_com->cv == CAV) {
		RF_DBG(rf, DBG_RF_OP5K, "[OP5K] =======> %s rf->hal_com->cv == CAV Return!!!\n", __func__);
		return;
	}

	op5k->op5k_default[RF_PATH_A] = halrf_rreg(rf, 0xc0d8, 0x000001e0);
	op5k->op5k_default[RF_PATH_B] = halrf_rreg(rf, 0xc1d8, 0x000001e0);

	if (!(rf->support_ability & HAL_RF_OP5K))
		return;

	op5k->op5k_set_ch_chk = true;

	if (op5k->op5k_ther_chk == true) {
		halrf_wreg(rf, 0xc0d8, 0x000001e0, op5k->rst_cnt_final[RF_PATH_A] & 0xf);
		halrf_wreg(rf, 0xc1d8, 0x000001e0, op5k->rst_cnt_final[RF_PATH_B] & 0xf);
		RF_DBG(rf, DBG_RF_OP5K, "[OP5K] =======> %s Set 0xc0d8[8:5]=0x%x  0xc1d8[8:5]=0x%x\n",
			__func__,
			halrf_rreg(rf, 0xc0d8, 0x000001e0),
			halrf_rreg(rf, 0xc1d8, 0x000001e0));
	}
}

void halrf_set_share_xtal_8832br(struct rf_info *rf)
{
	u8 value;
	u8 xtal_value = rf->hal_com->dev_hw_cap.xcap;

#if defined(CONFIG_AX_SHARE_XTAL) && defined(CONFIG_WLAN_SHARE_XTAL)

	RF_DBG(rf, DBG_RF_XTAL_TRACK, "======>%s share_xtal xtal_value=0x%x\n",
		__func__, xtal_value);

	harlf_mac_get_xsi(rf, 0x8, &value);
	value = value & 0xfc;
	value = value | 0x1;
	harlf_mac_set_xsi(rf, 0x8, value);

	harlf_mac_get_xsi(rf, 0x4, &value);
	value = value & 0x80;
	value = value | xtal_value;
	harlf_mac_set_xsi(rf, 0x4, value);

	harlf_mac_get_xsi(rf, 0x5, &value);
	value = value & 0x80;
	value = value | xtal_value;
	harlf_mac_set_xsi(rf, 0x5, value);
#else

	RF_DBG(rf, DBG_RF_XTAL_TRACK, "======>%s none share_xtal xtal_value=0x%x\n",
		__func__, xtal_value);

	harlf_mac_get_xsi(rf, 0x4, &value);
	value = value & 0x80;
	value = value | xtal_value;
	harlf_mac_set_xsi(rf, 0x4, value);

	harlf_mac_get_xsi(rf, 0x5, &value);
	value = value & 0x80;
	value = value | xtal_value;
	harlf_mac_set_xsi(rf, 0x5, value);
#endif

}

#endif
