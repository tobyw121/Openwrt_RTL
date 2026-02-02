/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
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
 ******************************************************************************/

#include "hwamsdu_1115e.h"

#if MAC_AX_1115E_SUPPORT

u32 mac_enable_cut_hwamsdu_1115e(struct mac_ax_adapter *adapter,
				 u8 enable,
				 enum mac_ax_ex_shift aligned)
{
	return MACNOTSUP;
}

u32 mac_cut_hwamsdu_chk_mpdu_len_en_1115e(struct mac_ax_adapter *adapter,
					  u8 enable,
					  u8 low_th,
					  u16 high_th)
{
	return MACNOTSUP;
}
#endif