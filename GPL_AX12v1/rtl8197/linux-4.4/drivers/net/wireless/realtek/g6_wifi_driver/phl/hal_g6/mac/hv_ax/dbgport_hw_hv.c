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
#include "dbgport_hw_hv.h"
#include "../mac_ax/dbgport_hw.h"
#include "../mac_reg.h"

/* This is just a sample code */
u32 mac_dbg_port_hw_set_sample(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_dbgport_hw dp_hw;
	u32 ret = 0;

	memset(&dp_hw, 0, sizeof(struct mac_ax_dbgport_hw));

	/* Case 0: For dump mode */
	dp_hw.dbg_sel[0] = MAC_AX_DP_SEL_STA_SCHEDULER;
	dp_hw.intn_idx[0] = MAC_AX_DP_INTN_IDX_STA_SCHEDULER_A;
	dp_hw.dbg_sel_16b[0] = MAC_AX_DP_SEL0_16B_0_15;
	dp_hw.dbg_sel_4b[0] = MAC_AX_DP_SEL_4B_0_7;
	dp_hw.dbg_sel[1] = MAC_AX_DP_SEL_PCIE_8;
	dp_hw.intn_idx[1] = MAC_AX_DP_INTN_IDX_PCIE_8_6;
	dp_hw.dbg_sel_16b[1] = MAC_AX_DP_SEL1_16B_0_15;
	dp_hw.dbg_sel_4b[1] = MAC_AX_DP_SEL_4B_0_7;
	dp_hw.mode = MAC_AX_DP_MODE_DUMP;

	ret = mac_dbgport_hw_set(adapter, &dp_hw);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]dbg_port_hw_set %d\n", ret);
		return ret;
	} else
		PLTFM_MSG_TRACE("dbg_port_hw_set get debug info: %x\n",
				dp_hw.rsp_val);

	/* Case 1: For LA mode */
	dp_hw.dbg_sel[0] = MAC_AX_DP_SEL_PCIE_0;
	dp_hw.intn_idx[0] = MAC_AX_DP_INTN_IDX_PCIE_0_0;
	dp_hw.dbg_sel_16b[0] = MAC_AX_DP_SEL0_16B_0_15;
	dp_hw.dbg_sel_4b[0] = MAC_AX_DP_SEL_4B_0_7;
	dp_hw.dbg_sel[1] = MAC_AX_DP_SEL_PCIE_0;
	dp_hw.intn_idx[1] = MAC_AX_DP_INTN_IDX_PCIE_0_0;
	dp_hw.dbg_sel_16b[1] = MAC_AX_DP_SEL1_16B_16_31;
	dp_hw.dbg_sel_4b[1] = MAC_AX_DP_SEL_4B_0_7;
	dp_hw.mode = MAC_AX_DP_MODE_LA;

	ret = mac_dbgport_hw_set(adapter, &dp_hw);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]dbg_port_hw_set %d\n", ret);
		return ret;
	} else
		PLTFM_MSG_TRACE("dbg_port_hw_set done\n");

	return ret;
}

