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
#ifndef __HALRF_IQK_8192XBH__
#define __HALRF_IQK_8192XBH__
#ifdef RF_8192XB_SUPPORT

#define iqk_version_8192xb 0x5
#define ss_8192xb 2
#define mac_reg_num_8192xb 2
#define bb_reg_num_8192xb 1
#define rf_reg_num_8192xb 6
#define iqk_delay_8192xb 1
#define iqk_step_8192xb 2
#define rxk_step_8192xb 4
#define path_baseaddr 0x8
#define txagck 0
#define lok 1
#define txk 2
#define rxk 3
#define IQK_THR_ReK 8


void halrf_iqk_8192xb(struct rf_info *rf, enum phl_phy_idx phy_idx, bool force);

//cv b
void iqk_init_8192xb(struct rf_info *rf);
void halrf_iqk_8192xb(struct rf_info *rf, enum phl_phy_idx phy_idx, bool force);
void halrf_doiqk_8192xb(struct rf_info *rf, bool force,
			enum phl_phy_idx phy_idx, u8 path);
void iqk_get_ch_info_8192xb(struct rf_info *rf, enum phl_phy_idx phy, u8 path);
bool iqk_mcc_page_sel_8192xb(struct rf_info *rf, enum phl_phy_idx phy, u8 path);
void iqk_macbb_setting_8192xb(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path);
void iqk_preset_8192xb(struct rf_info *rf, u8 path);
void iqk_start_iqk_8192xb(struct rf_info *rf, enum phl_phy_idx phy_idx,
			  u8 path);
void iqk_restore_8192xb(struct rf_info *rf, u8 path);
void iqk_afebb_restore_8192xb(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path);
void halrf_iqk_reload_8192xb(struct rf_info *rf, u8 path);
void halrf_nbiqk_enable_8192xb(struct rf_info *rf, bool nbiqk_en);
void halrf_iqk_toneleakage_8192xb(struct rf_info *rf, u8 path);
void halrf_iqk_sram_enable_8192xb(struct rf_info *rf, bool iqk_sram_en);
void halrf_iqk_xym_enable_8192xb(struct rf_info *rf, bool iqk_xym_en);
void halrf_iqk_fft_enable_8192xb(struct rf_info *rf, bool iqk_fft_en);
void halrf_iqk_cfir_enable_8192xb(struct rf_info *rf, bool iqk_cfir_en);
void halrf_iqk_onoff_8192xb(struct rf_info *rf, bool is_enable);
void halrf_iqk_lok_bypass_8192xb(struct rf_info *rf, u8 path);
void halrf_iqk_rx_bypass_8192xb(struct rf_info *rf, u8 path);
void halrf_iqk_tx_bypass_8192xb(struct rf_info *rf, u8 path);

void iqk_backup_mac_bb_8192xb(struct rf_info *rf, u8 path,
			     u32 backup_mac[mac_reg_num_8192xb],
			     u32 backup_bb[bb_reg_num_8192xb],
			     u32 backup_mac_reg[mac_reg_num_8192xb],
			     u32 backup_bb_reg[bb_reg_num_8192xb]);
void iqk_backup_rf_8192xb(struct rf_info *rf, u8 path,
			 u32 backup_rf[][rf_reg_num_8192xb],
			 u32 backup_rf_reg[][rf_reg_num_8192xb]);
void iqk_restore_mac_bb_8192xb(struct rf_info *rf, u8 path,
			      u32 backup_mac[mac_reg_num_8192xb],
			      u32 backup_bb[bb_reg_num_8192xb],
			      u32 backup_mac_reg[mac_reg_num_8192xb],
			      u32 backup_bb_reg[bb_reg_num_8192xb]);
void iqk_restore_rf_8192xb(struct rf_info *rf, u8 path,
			  u32 backup_rf[][rf_reg_num_8192xb],
			  u32 backup_rf_reg[][rf_reg_num_8192xb]);

void halrf_iqk_track_8192xb(struct rf_info *rf);

bool halrf_iqk_get_ther_rek_8192xb(struct rf_info *rf );
void halrf_iqk_dbcc_8192xb(struct rf_info *rf, u8 path);
u8 halrf_iqk_get_mcc_ch0_8192xb(struct rf_info *rf );
u8 halrf_iqk_get_mcc_ch1_8192xb(struct rf_info *rf );
bool halrf_check_fwiqk_done_8192xb(struct rf_info *rf);
void iqk_set_info_8192xb(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path);
void halrf_enable_fw_iqk_8192xb(struct rf_info *rf, bool is_fw_iqk);
u8 halrf_get_iqk_times_8192xb(struct rf_info *rf) ;
u32 halrf_get_iqk_ver_8192xb(void);
u8 halrf_iqk_get_rxevm_8192xb(struct rf_info *rf);
u32 halrf_iqk_get_rximr_8192xb(struct rf_info *rf, u8 path, u32 idx);


#endif
#endif /*  __HALRF_IQK_8192XBH__ */

