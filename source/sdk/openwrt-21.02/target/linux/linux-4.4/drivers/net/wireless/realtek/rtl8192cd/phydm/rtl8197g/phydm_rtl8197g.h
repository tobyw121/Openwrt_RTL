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
#if (RTL8197G_SUPPORT)
#ifndef __ODM_RTL8197G_H__
#define __ODM_RTL8197G_H__

/* 2019.09.06: modify code structure*/
#define HW_SETTING_VERSION_8197G "1.0"

void phydm_hwsetting_8197g(void *dm_void);
void phydm_tx_collsion_th_init_8197g(void *dm_void);
void phydm_tx_collsion_th_set_8197g(void *dm_void, u8 th_r2t, u8 th_t2r);
#endif
#endif
