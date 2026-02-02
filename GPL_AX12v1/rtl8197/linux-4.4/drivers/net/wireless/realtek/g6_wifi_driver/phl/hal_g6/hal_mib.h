/******************************************************************************
 *
 * Copyright(c)2019 Realtek Corporation.
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
#ifndef _HAL_MIB_H_
#define _HAL_MIB_H_

enum rtw_hal_status rtw_hal_set_lifetime(void *hal, u8 en, u16 to);
enum rtw_hal_status rtw_hal_get_lifetime(void *hal);
enum rtw_hal_status rtw_hal_set_ref_power(void *hal, u8 band, u16 txagc_ref);

#endif