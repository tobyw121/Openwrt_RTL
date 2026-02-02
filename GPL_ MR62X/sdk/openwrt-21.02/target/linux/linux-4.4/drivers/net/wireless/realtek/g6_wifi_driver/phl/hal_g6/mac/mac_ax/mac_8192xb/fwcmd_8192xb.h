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
#if MAC_AX_8192XB_SUPPORT

#ifndef _MAC_AX_FW_CMD_8192XB_H_
#define _MAC_AX_FW_CMD_8192XB_H_

struct mac_ax_h2creg_offset *
get_h2creg_offset_8192xb(struct mac_ax_adapter *adapter);

struct mac_ax_c2hreg_offset *
get_c2hreg_offset_8192xb(struct mac_ax_adapter *adapter);

#endif /* #if MAC_AX_8192XB_SUPPORT */
#endif
