/** @file */
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

#ifndef _FWCMD_HV_H_
#define _FWCMD_HV_H_

#include "../type.h"

/**
 * @brief hv_ptn_h2c_common
 *
 * @param *adapter
 * @param *hdr
 * @param *pvalue
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_ptn_h2c_common(struct mac_ax_adapter *adapter,
			  struct rtw_g6_h2c_hdr *hdr, u32 *pvalue);

/**
 * @brief c2h_lps_onoff_rpt
 *
 * @param *adapter
 * @param len
 * @param *buf
 * @return Please Place Description here.
 * @retval u32
 */

u32 c2h_lps_onoff_rpt(struct mac_ax_adapter *adapter,
			  u16 len, u8 *buf);

#endif
