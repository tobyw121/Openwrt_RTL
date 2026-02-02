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

#ifndef _TRX_DESC_HV_H_
#define _TRX_DESC_HV_H_

#include "../../hv_type.h"
#if MAC_AX_8852A_SUPPORT

/**
 * @struct wd_body
 * @brief wd_body
 *
 * @var wd_body::dword0
 * Please Place Description here.
 * @var wd_body::dword1
 * Please Place Description here.
 * @var wd_body::dword2
 * Please Place Description here.
 * @var wd_body::dword3
 * Please Place Description here.
 * @var wd_body::dword4
 * Please Place Description here.
 * @var wd_body::dword5
 * Please Place Description here.
 */
struct wd_body {
	u32 dword0;
	u32 dword1;
	u32 dword2;
	u32 dword3;
	u32 dword4;
	u32 dword5;
};

/**
 * @brief hv_tx_post_agg
 *
 * @param *adapter
 * @param *agg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_tx_post_agg_8852a(struct mac_ax_adapter *adapter,
			 struct hv_aggregator_t *agg);

#endif /* #if MAC_AX_8852A_SUPPORT */
#endif
