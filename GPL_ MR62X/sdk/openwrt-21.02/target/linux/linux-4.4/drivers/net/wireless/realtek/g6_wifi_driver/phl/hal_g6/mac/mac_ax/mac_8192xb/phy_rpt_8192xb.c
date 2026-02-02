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

#include "phy_rpt_8192xb.h"
#include "../hw.h"

#if MAC_AX_8192XB_SUPPORT

#define MAC_AX_DISP_QID_HOST 0x0
#define MAC_AX_DISP_QID_WLCPU 0x8
#define MAC_AX_DISP_PID_HOST 0x0
#define MAC_AX_DISP_PID_WLCPU 0x0

u32 get_bbrpt_dle_cfg_8192xb(struct mac_ax_adapter *adapter,
			     u8 is2wlcpu, u32 *port_id, u32 *queue_id)
{
	if (is2wlcpu) {
		*port_id = MAC_AX_DISP_PID_WLCPU;
		*queue_id = MAC_AX_DISP_QID_WLCPU;
	} else {
		*port_id = MAC_AX_DISP_PID_HOST;
		*queue_id = MAC_AX_DISP_QID_HOST;
	}

	return MACSUCCESS;
}

u32 mac_cfg_per_pkt_phy_rpt_8192xb(struct mac_ax_adapter *adapter,
				   struct mac_ax_per_pkt_phy_rpt *rpt)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	enum mac_ax_drv_info_size *s = &adapter->hw_info->cmac0_drv_info;
	u32 drv_info, val, ori_drv_info;

	if (rpt->en) {
		switch (*s) {
		case MAC_AX_DRV_INFO_PHY_RPT:
		case MAC_AX_DRV_INFO_NONE:
			*s = MAC_AX_DRV_INFO_PHY_RPT;
			drv_info = MAC_AX_DRV_INFO_SIZE_16;
			break;
		case MAC_AX_DRV_INFO_PHY_RPT_BSSID:
		case MAC_AX_DRV_INFO_BSSID:
			*s = MAC_AX_DRV_INFO_PHY_RPT_BSSID;
			drv_info = MAC_AX_DRV_INFO_SIZE_80;
			break;
		case MAC_AX_DRV_INFO_PHY_RPT_MAC_HDR:
		case MAC_AX_DRV_INFO_MAC_HDR:
			*s = MAC_AX_DRV_INFO_PHY_RPT_MAC_HDR;
			drv_info = MAC_AX_DRV_INFO_SIZE_80;
			break;
		default:
			PLTFM_MSG_ERR("%s: wrong driver info size", __func__);
			return MACNOITEM;
		}
	} else {
		switch (*s) {
		case MAC_AX_DRV_INFO_NONE:
		case MAC_AX_DRV_INFO_PHY_RPT:
			*s = MAC_AX_DRV_INFO_NONE;
			drv_info = MAC_AX_DRV_INFO_SIZE_0;
			break;
		case MAC_AX_DRV_INFO_BSSID:
		case MAC_AX_DRV_INFO_PHY_RPT_BSSID:
			*s = MAC_AX_DRV_INFO_BSSID;
			drv_info = MAC_AX_DRV_INFO_SIZE_16;
			break;
		case MAC_AX_DRV_INFO_MAC_HDR:
		case MAC_AX_DRV_INFO_PHY_RPT_MAC_HDR:
			*s = MAC_AX_DRV_INFO_MAC_HDR;
			drv_info = MAC_AX_DRV_INFO_SIZE_80;
			break;
		default:
			PLTFM_MSG_ERR("%s: wrong driver info size", __func__);
			return MACNOITEM;
		}
	}

	val = MAC_REG_R32(R_AX_RCR);
	ori_drv_info = GET_FIELD(val, B_AX_DRV_INFO_SIZE);

	if (drv_info > ori_drv_info) {
		val = MAC_REG_R32(R_AX_RCR);
		val = SET_CLR_WORD(val, drv_info, B_AX_DRV_INFO_SIZE);
		MAC_REG_W32(R_AX_RCR, val);
	}

	val = MAC_REG_R32(R_AX_DRV_INFO_OPTION);
	val = rpt->en ? (val | B_AX_DRV_INFO_PHYRPT_EN) :
				(val & ~B_AX_DRV_INFO_PHYRPT_EN);
	MAC_REG_W32(R_AX_DRV_INFO_OPTION, val);

	return MACSUCCESS;
}

#endif /* MAC_AX_8192XB_SUPPORT */
