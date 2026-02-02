/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#ifndef _PHL_MIB_H_
#define _PHL_MIB_H_

#ifdef CONFIG_LIFETIME_FEATURE
enum rtw_phl_status rtw_phl_set_lifetime(void *phl, u8 enable, u16 timeout);
enum rtw_phl_status rtw_phl_get_lifetime(void *phl);
#endif

#ifdef POWER_PERCENT_ADJUSTMENT
enum rtw_phl_status rtw_phl_set_ref_power(void *phl, u8 band, int txagc_ref);
#endif

#endif /*_PHL_ACS_H_*/
