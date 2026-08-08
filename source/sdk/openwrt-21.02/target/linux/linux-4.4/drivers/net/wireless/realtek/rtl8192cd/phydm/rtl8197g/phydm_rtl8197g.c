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
#include "mp_precomp.h"
#include "../phydm_precomp.h"

#if (RTL8197G_SUPPORT)

void phydm_tx_collsion_th_init_8197g(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	odm_set_bb_reg(dm, R_0x8f8, BIT(16), 1);
	odm_set_bb_reg(dm, R_0x8f8, BIT(17), 1);
	odm_set_bb_reg(dm, R_0x8f8, BIT(18) | BIT(19), 3);
	odm_set_bb_reg(dm, R_0x1c3c, BIT(7), 1);
	odm_set_bb_reg(dm, R_0x1c3c, BIT(5) | BIT(6), 1);
}

void phydm_tx_collsion_th_set_8197g(void *dm_void, u8 val_r2t, u8 val_t2r)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 reg_8f8 = 0;
	u32 reg_8fc = 0;
	u8 th_r2t = 0, th_t2r = 0;

	if (!(dm->support_ability & ODM_BB_RSSI_MONITOR))
		return;

	th_r2t = (val_r2t > 31) ? 31 : val_r2t;
	th_t2r = (val_t2r > 31) ? 31 : val_t2r;

	PHYDM_DBG(dm, DBG_RA, "TxCLS TH:{R2T, T2R} = {%d, %d}\n",
		  th_r2t, th_t2r);

	reg_8f8 = (th_t2r << 5) | th_r2t;
	reg_8fc = (th_t2r << 25) | (th_t2r << 20) | (th_t2r << 15)
		  | (th_t2r << 10) | (th_t2r << 5) | th_t2r;

	odm_set_bb_reg(dm, R_0x8f8, 0x7FE00000, reg_8f8);
	odm_set_bb_reg(dm, R_0x8fc, 0x3FFFFFFF, reg_8fc);
}

void phydm_tx_collsion_detect_8197g(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct ra_table *ra_tab = &dm->dm_ra_table;
	u8 th_r2t = 0, th_t2r = 0;

	if (!(dm->support_ability & ODM_BB_RSSI_MONITOR))
		return;

	//PHYDM_DBG(dm, DBG_RA, "Tx CLS Detect\n");

	if ((100 - dm->rssi_min) <= 45) {
		th_t2r = 0;
		th_r2t = 0;
	} else if ((100 - dm->rssi_min) <= 76) {
		th_t2r = 55 - dm->rssi_min;
		th_r2t = 55 - dm->rssi_min;
	} else {
		th_t2r = 31;
		th_r2t = 31;
	}

	if (ra_tab->ra_tx_cls_th == 255)
		phydm_tx_collsion_th_set_8197g(dm, th_r2t, th_t2r);
}

void phydm_hwsetting_8197g(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#ifdef CONFIG_DYNAMIC_TXCOLLISION_TH
	phydm_tx_collsion_detect_8197g(dm);
#endif

}
#endif
