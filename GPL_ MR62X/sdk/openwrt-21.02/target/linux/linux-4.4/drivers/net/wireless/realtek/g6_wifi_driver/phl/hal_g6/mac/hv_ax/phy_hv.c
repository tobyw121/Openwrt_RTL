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
#include "phy_hv.h"
#include "../mac_reg.h"

u32 hv_phy_cfg(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val, val16, val8;
	u32 ret = 0;

	val8 = MAC_REG_R8(R_AX_DFS_CFG0);
	MAC_REG_W8(R_AX_DFS_CFG0, val8);

	val16 = MAC_REG_R16(R_AX_CH_INFO);
	if (val16 & B_AX_CH_INFO_EN) {
		ret = 1;
		goto END;
	}
	MAC_REG_W16(R_AX_CH_INFO, val16);

	val = MAC_REG_R32(R_AX_BBRPT_COEX_CFG);
	MAC_REG_W32(R_AX_BBRPT_COEX_CFG, val);

	PLTFM_MSG_ERR("Error log");

END:
	return ret;
}
