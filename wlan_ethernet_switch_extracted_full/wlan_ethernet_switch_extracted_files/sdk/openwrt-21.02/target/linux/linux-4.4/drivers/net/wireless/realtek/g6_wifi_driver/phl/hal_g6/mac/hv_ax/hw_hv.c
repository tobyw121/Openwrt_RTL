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
#include "hw_hv.h"

#define HV_AX_RXD_DRV_INFO_UNIT_8852A 8
#define HV_AX_RXD_DRV_INFO_UNIT_8852B 8
#define HV_AX_RXD_DRV_INFO_UNIT_8852C 16
#define HV_AX_RXD_DRV_INFO_UNIT_8192XB 16
#define HV_AX_RXD_DRV_INFO_UNIT_8851B 8
#define HV_AX_RXD_DRV_INFO_UNIT_8851E 16
#define HV_AX_RXD_DRV_INFO_UNIT_8852D 16

u32 hv_get_rxd_drv_info_unit(struct mac_ax_adapter *adapter)
{
	struct mac_ax_hw_info *hw_info = adapter->hw_info;
	u32 unit = 0;

	switch (hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8852A:
		unit = HV_AX_RXD_DRV_INFO_UNIT_8852A;
		break;
	case MAC_AX_CHIP_ID_8852B:
		unit = HV_AX_RXD_DRV_INFO_UNIT_8852B;
		break;
	case MAC_AX_CHIP_ID_8852C:
		unit = HV_AX_RXD_DRV_INFO_UNIT_8852C;
		break;
	case MAC_AX_CHIP_ID_8192XB:
		unit = HV_AX_RXD_DRV_INFO_UNIT_8192XB;
		break;
	case MAC_AX_CHIP_ID_8851B:
		unit = HV_AX_RXD_DRV_INFO_UNIT_8851B;
		break;
	case MAC_AX_CHIP_ID_8851E:
		unit = HV_AX_RXD_DRV_INFO_UNIT_8851E;
		break;
	case MAC_AX_CHIP_ID_8852D:
		unit = HV_AX_RXD_DRV_INFO_UNIT_8852D;
		break;
	default:
		PLTFM_MSG_ERR("%s: Wrong CHIP %d\n", __func__, hw_info->chip_id);
		break;
	}

	return unit;
}
