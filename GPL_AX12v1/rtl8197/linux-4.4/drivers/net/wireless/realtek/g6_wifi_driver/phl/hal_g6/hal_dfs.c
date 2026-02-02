/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
 *****************************************************************************/
#define _HAL_DFS_C_
#include "hal_headers.h"

#ifdef CONFIG_PHL_DFS
enum rtw_hal_status
rtw_hal_radar_detect_cfg(void *hal, bool dfs_enable)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_mac_dfs_rpt_cfg conf = {
		.rpt_en = dfs_enable,
		.rpt_num_th = MAC_AX_DFS_TH_61,
		.rpt_en_to = dfs_enable,
		.rpt_to = MAC_AX_DFS_TO_80MS,
	};

	PHL_INFO("====>%s dfs_en:%d ============\n", __func__, dfs_enable);
	if (dfs_enable) {
		rtw_hal_mac_dfs_rpt_cfg(hal_info, &conf);
		rtw_hal_bb_dfs_rpt_cfg(hal_info, true);
	}
	else {
		rtw_hal_mac_dfs_rpt_cfg(hal_info, &conf);
		rtw_hal_bb_dfs_rpt_cfg(hal_info, false);
	}
	return RTW_HAL_STATUS_SUCCESS;
}

#endif

