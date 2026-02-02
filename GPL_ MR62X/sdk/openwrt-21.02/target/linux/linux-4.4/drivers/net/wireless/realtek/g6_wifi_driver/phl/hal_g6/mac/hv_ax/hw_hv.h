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

#ifndef _MAC_AX_HW_HV_H_
#define _MAC_AX_HW_HV_H_

#include "../type.h"

/**
 * @brief get the unit of drv info in RX DESC
 *
 * @param *adapter
 * @return the unit of drv info
 * @retval u32
 */
u32 hv_get_rxd_drv_info_unit(struct mac_ax_adapter *adapter);

#endif
