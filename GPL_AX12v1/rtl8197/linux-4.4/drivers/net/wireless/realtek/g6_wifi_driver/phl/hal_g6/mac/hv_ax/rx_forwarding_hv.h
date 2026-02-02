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

#ifndef _MAC_AX_RX_FORWARDING_HV_H_
#define _MAC_AX_RX_FORWARDING_HV_H_

#include "../type.h"

/*--------------------Define ----------------------------------------*/

/*--------------------Define Enum------------------------------------*/

/*--------------------Define MACRO----------------------------------*/

/*--------------------Define Struct-----------------------------------*/

/*--------------------Export global variable----------------------------*/

/*--------------------Function declaration-----------------------------*/

/**
 * @brief rx_fwd_pm_cam_sample
 *
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 rx_fwd_pm_cam_sample(struct mac_ax_adapter *adapter);

#endif
