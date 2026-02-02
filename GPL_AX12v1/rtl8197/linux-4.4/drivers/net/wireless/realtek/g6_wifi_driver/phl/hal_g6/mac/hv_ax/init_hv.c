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

#include "init_hv.h"
#include "hv_8852a/init_hv_8852a.h"
#include "hv_8852b/init_hv_8852b.h"
#include "hv_8852c/init_hv_8852c.h"
#include "hv_8192xb/init_hv_8192xb.h"
#include "hv_8851b/init_hv_8851b.h"

struct hv_ax_ops *get_hv_ax_ops(struct mac_ax_adapter *adapter)
{
	struct mac_ax_hw_info *hw_info = adapter->hw_info;
	struct hv_ax_ops *hv_ops;

	switch (hw_info->chip_id) {
#if MAC_AX_8852A_SUPPORT
	case MAC_AX_CHIP_ID_8852A:
		hv_ops = get_hv_8852a_ops();
		break;
#endif
#if MAC_AX_8852B_SUPPORT
	case MAC_AX_CHIP_ID_8852B:
		hv_ops = get_hv_8852b_ops();
		break;
#endif
#if MAC_AX_8852C_SUPPORT
	case MAC_AX_CHIP_ID_8852C:
		hv_ops = get_hv_8852c_ops();
		break;
#endif
#if MAC_AX_8192XB_SUPPORT
	case MAC_AX_CHIP_ID_8192XB:
		hv_ops = get_hv_8192xb_ops();
		break;
#endif
#if MAC_AX_8851B_SUPPORT
	case MAC_AX_CHIP_ID_8851B:
		hv_ops = get_hv_8851b_ops();
		break;
#endif /* #if MAC_AX_8851B_SUPPORT */
	default:
		return NULL;
	}

	return hv_ops;
}
