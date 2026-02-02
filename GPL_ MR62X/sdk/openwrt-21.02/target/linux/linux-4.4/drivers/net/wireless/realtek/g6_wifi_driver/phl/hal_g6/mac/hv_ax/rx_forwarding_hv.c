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
#include "rx_forwarding_hv.h"
#include "../mac_ax/rx_forwarding.h"
#include "../mac_reg.h"

/* This is just a sample code */
u32 rx_fwd_pm_cam_sample(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_rx_fwd_ctrl_t rf_ctrl_p;
	struct mac_ax_pm_cam_ctrl_t *pm_cam_ctrl_p;
	u32 val32, ret = 0;

	memset(&rf_ctrl_p, 0, sizeof(mac_ax_rx_fwd_ctrl_t));

	/* ===== For Payload Match CAM ===== */
	rf_ctrl_p.type = MAC_AX_FT_PM_CAM;
	pm_cam_ctrl_p = &rf_ctrl_p.pm_cam_ctrl;

	// Set as indirect access and enable PM CAM
	val32 = MAC_REG_R32(R_AX_PLD_CAM_CTRL);
	val32 = SET_CLR_WORD(val32, 0xf, B_AX_PLD_CAM_RANGE);
	val32 |= B_AX_PLD_CAM_ACC;
	val32 |= B_AX_PLD_CAM_EN;
	MAC_REG_W32(R_AX_PLD_CAM_CTRL, val32);

	//  Entry_0: data frame (QoS data)
	pm_cam_ctrl_p->pld_mask0 = 0;
	pm_cam_ctrl_p->pld_mask1 = 0;
	pm_cam_ctrl_p->pld_mask2 = 0;
	pm_cam_ctrl_p->pld_mask3 = 0;
	pm_cam_ctrl_p->entry_index = 0;
	pm_cam_ctrl_p->valid = 1;
	pm_cam_ctrl_p->type = 2;
	pm_cam_ctrl_p->subtype = 0x8;
	pm_cam_ctrl_p->skip_mac_iv_hdr = 1;
	pm_cam_ctrl_p->target_ind = 1;
	pm_cam_ctrl_p->crc16 = 0;
	ret = mac_set_rx_forwarding(adapter, &rf_ctrl_p);
	if (ret != 0)
		return ret;

	//  Entry_1: data frame (data)
	pm_cam_ctrl_p->pld_mask0 = 0;
	pm_cam_ctrl_p->pld_mask1 = 0;
	pm_cam_ctrl_p->pld_mask2 = 0;
	pm_cam_ctrl_p->pld_mask3 = 0;
	pm_cam_ctrl_p->entry_index = 1;
	pm_cam_ctrl_p->valid = 1;
	pm_cam_ctrl_p->type = 2;
	pm_cam_ctrl_p->subtype = 0;
	pm_cam_ctrl_p->skip_mac_iv_hdr = 1;
	pm_cam_ctrl_p->target_ind = 1;
	pm_cam_ctrl_p->crc16 = 0;
	ret = mac_set_rx_forwarding(adapter, &rf_ctrl_p);
	if (ret != 0)
		return ret;

	//  Entry_2: management frame (ATIM)
	pm_cam_ctrl_p->pld_mask0 = 0;
	pm_cam_ctrl_p->pld_mask1 = 0;
	pm_cam_ctrl_p->pld_mask2 = 0;
	pm_cam_ctrl_p->pld_mask3 = 0;
	pm_cam_ctrl_p->entry_index = 2;
	pm_cam_ctrl_p->valid = 1;
	pm_cam_ctrl_p->type = 0;
	pm_cam_ctrl_p->subtype = 9;
	pm_cam_ctrl_p->skip_mac_iv_hdr = 1;
	pm_cam_ctrl_p->target_ind = 1;
	pm_cam_ctrl_p->crc16 = 0;
	ret = mac_set_rx_forwarding(adapter, &rf_ctrl_p);
	if (ret != 0)
		return ret;

	//  Entry_3: management frame (Association request)
	pm_cam_ctrl_p->pld_mask0 = 1;
	pm_cam_ctrl_p->pld_mask1 = 0;
	pm_cam_ctrl_p->pld_mask2 = 0;
	pm_cam_ctrl_p->pld_mask3 = 0;
	pm_cam_ctrl_p->entry_index = 3;
	pm_cam_ctrl_p->valid = 1;
	pm_cam_ctrl_p->type = 0;
	pm_cam_ctrl_p->subtype = 0;
	pm_cam_ctrl_p->skip_mac_iv_hdr = 1;
	pm_cam_ctrl_p->target_ind = 1;
	pm_cam_ctrl_p->crc16 = 0;
	ret = mac_set_rx_forwarding(adapter, &rf_ctrl_p);
	if (ret != 0)
		return ret;

	return ret;
}

