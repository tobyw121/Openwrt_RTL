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
#ifndef __HALRF_8832CRVU_H__
#define __HALRF_8832CRVU_H__
#ifdef RF_8832CRVU_SUPPORT

#define RXDCK_VER_8832CRVU 0x6
#define RCK_VER_8832CRVU 0x1
#define LCK_VER_8832CRVU 0x1
#define NCTL_FINAL_LINE_8832CRVU 0x8000
#define LCK_TH_8832CRVU 8

void halrf_lo_test_8832crvu(struct rf_info *rf, bool is_on, enum rf_path path);
u8 halrf_kpath_8832crvu(struct rf_info *rf, enum phl_phy_idx phy_idx);
void halrf_set_rx_dck_8832crvu(struct rf_info *rf, enum phl_phy_idx phy, enum rf_path path, bool is_afe);
void halrf_rx_dck_8832crvu(struct rf_info *rf, enum phl_phy_idx phy, bool is_afe);
void halrf_rx_dck_onoff_8832crvu(struct rf_info *rf, bool is_enable);
void halrf_rck_8832crvu(struct rf_info *rf, enum rf_path path);
void halrf_rxdck_track_8832crvu(struct rf_info *rf);
void halrf_rf_direct_cntrl_8832crvu(struct rf_info *rf, enum rf_path path, bool is_bybb);
void halrf_drf_direct_cntrl_8832crvu(struct rf_info *rf, enum rf_path path, bool is_bybb);
void halrf_bf_config_rf_8832crvu(struct rf_info *rf);
extern struct rfk_iqk_info rf_iqk_hwspec_8832crvu;
bool halrf_ctrl_ch_8832crvu(struct rf_info *rf,  enum phl_phy_idx phy, u8 central_ch, enum band_type band);
bool halrf_ctrl_bw_8832crvu(struct rf_info *rf, enum phl_phy_idx phy, enum channel_width bw);
void halrf_rxbb_bw_8832crvu(struct rf_info *rf, enum phl_phy_idx phy, enum channel_width bw);

void halrf_before_one_shot_enable_8832crvu(struct rf_info *rf);
bool halrf_one_shot_nctl_done_check_8832crvu(struct rf_info *rf, enum rf_path path);
bool halrf_do_one_shot_8832crvu(struct rf_info *rf, enum rf_path path, u32 nctl_addr, u32 mask, u32 process_id);

bool halrf_rfk_reg_check_8832crvu(struct rf_info *rf);
void halrf_rfk_reg_backup_8832crvu(struct rf_info *rf);
bool halrf_rfk_reg_check_fail_8832crvu(struct rf_info *rf);
void halrf_rfc_reg_backup_8832crvu(struct rf_info *rf);
bool halrf_rfc_reg_check_fail_8832crvu(struct rf_info *rf);
void halrf_rfk_reg_reload_8832crvu(struct rf_info *rf);
void halrf_disconnect_notify_8832crvu(struct rf_info *rf, struct rtw_chan_def *chandef);
bool halrf_check_mcc_ch_8832crvu(struct rf_info *rf, struct rtw_chan_def *chandef);
void halrf_fw_ntfy_8832crvu(struct rf_info *rf, enum phl_phy_idx phy_idx);
void halrf_lck_8832crvu(struct rf_info *rf);
void halrf_lck_tracking_8832crvu(struct rf_info *rf);
void halrf_quick_checkrf_8832crvu(struct rf_info *rf);
#endif
#endif /*  __HALRF_8832CRVU_H__ */
