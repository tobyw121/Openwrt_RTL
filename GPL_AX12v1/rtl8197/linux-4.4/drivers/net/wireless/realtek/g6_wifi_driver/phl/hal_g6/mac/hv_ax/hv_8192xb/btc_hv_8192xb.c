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
#include "btc_hv_8192xb.h"
#include "../../mac_reg.h"
#define HV_DBG_PORT_SET_BTC 0x74

#if MAC_AX_8192XB_SUPPORT

u32 hv_cfg_btc_dbg_port_8192xb(struct mac_ax_adapter *adapter)
{
	return MACNOTSUP;
}

u32 hv_en_rtk_mode_8192xb(struct mac_ax_adapter *adapter)
{
	return MACNOTSUP;
}

#endif