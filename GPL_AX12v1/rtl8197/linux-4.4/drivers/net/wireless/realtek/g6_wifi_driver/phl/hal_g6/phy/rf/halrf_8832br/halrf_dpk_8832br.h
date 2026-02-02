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
#ifndef __HALRF_DPK_8832BRH__
#define __HALRF_DPK_8832BRH__
#ifdef RF_8832BR_SUPPORT
/*--------------------------Define Parameters-------------------------------*/
#define DPK_VER_8832BR 0xa
#define DPK_RF_PATH_MAX_8832BR 2
#define DPK_KIP_REG_NUM_8832BR 8
#define DPK_BB_REG_NUM_8832BR 3
#define DPK_RF_REG_NUM_8832BR 9
#define DPK_PATH_A_8832BR 1
#define DPK_PATH_B_8832BR 1
#define DPK_RELOAD_EN_8832BR 0
#define DPK_REG_DBG_8832BR 0
#define DPK_RXSRAM_DBG_8832BR 0
#define DPK_PAS_DBG_8832BR 0
#define DPK_MDPK_DBG_8832BR 0

/*---------------------------End Define Parameters----------------------------*/

void halrf_dpk_8832br(struct rf_info *rf, enum phl_phy_idx phy_idx, bool force);

void halrf_dpk_onoff_8832br(struct rf_info *rf, enum rf_path path, bool off);

void halrf_dpk_track_8832br(struct rf_info *rf);

#endif
#endif /*  __HALRF_DPK_8832BRH__ */
