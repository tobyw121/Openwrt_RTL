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
#ifndef __HALRF_8192XB_H__
#define __HALRF_8192XB_H__
#ifdef RF_8192XB_SUPPORT

#define RXDCK_VER_8192XB 0x3
#define RCK_VER_8192XB 0x1
#define NCTL_FINAL_LINE_8192XB 0x8000

#define LCK_TH_8192XB 8

void halrf_lo_test_8192xb(struct rf_info *rf, bool is_on, enum rf_path path);
u8 halrf_kpath_8192xb(struct rf_info *rf, enum phl_phy_idx phy_idx);
void halrf_set_rx_dck_8192xb(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path, bool is_afe);
void halrf_rx_dck_8192xb(struct rf_info *rf, enum phl_phy_idx phy, bool is_afe);
void halrf_rx_dck_onoff_8192xb(struct rf_info *rf, bool is_enable);
void halrf_rck_8192xb(struct rf_info *rf, enum rf_path path);
void halrf_rf_direct_cntrl_8192xb(struct rf_info *rf, enum rf_path path, bool is_bybb);
void halrf_drf_direct_cntrl_8192xb(struct rf_info *rf, enum rf_path path, bool is_bybb);
void halrf_bf_config_rf_8192xb(struct rf_info *rf);
extern struct rfk_iqk_info rf_iqk_hwspec_8192xb;
void halrf_dpk_init_8192xb(struct rf_info *rf);
bool halrf_ctrl_ch_8192xb(struct rf_info *rf,  enum phl_phy_idx phy, u8 central_ch, enum band_type band);
bool halrf_ctrl_bw_8192xb(struct rf_info *rf, enum phl_phy_idx phy, enum channel_width bw);
void halrf_rxbb_bw_8192xb(struct rf_info *rf, enum phl_phy_idx phy, enum channel_width bw);
void halrf_fw_ntfy_8192xb(struct rf_info *rf, enum phl_phy_idx phy_idx); 
void halrf_disconnect_notify_8192xb(struct rf_info *rf, struct rtw_chan_def *chandef  ) ;
bool halrf_check_mcc_ch_8192xb(struct rf_info *rf, struct rtw_chan_def *chandef) ;


void halrf_before_one_shot_enable_8192xb(struct rf_info *rf);
bool halrf_one_shot_nctl_done_check_8192xb(struct rf_info *rf, enum rf_path path);
bool halrf_do_one_shot_8192xb(struct rf_info *rf, enum rf_path path, u32 nctl_addr, u32 mask, u32 process_id);
void halrf_lck_8192xb(struct rf_info *rf);
void halrf_lck_tracking_8192xb(struct rf_info *rf);
#endif
#endif /*  __HALRF_8192xb_H__ */
