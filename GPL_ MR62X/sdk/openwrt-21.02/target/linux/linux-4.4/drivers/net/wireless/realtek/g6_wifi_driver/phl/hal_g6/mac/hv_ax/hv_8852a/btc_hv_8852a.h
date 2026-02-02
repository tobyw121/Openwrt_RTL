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

#ifndef _MAC_AX_BTC_HV_8852A_H_
#define _MAC_AX_BTC_HV_8852A_H_

#include "../../type.h"
#include "../../mac_ax/hw.h"

/**
 * @brief hv_en_rtk_mode
 *
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_en_rtk_mode_8852a(struct mac_ax_adapter *adapter);

/**
 * @brief hv_cfg_btc_dbg_port
 *
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_cfg_btc_dbg_port_8852a(struct mac_ax_adapter *adapter);

#endif
