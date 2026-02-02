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
#ifndef _RTW_FLASH_H_
#define _RTW_FLASH_H_

#define MAX_EFUSE_FILE_VERSION_LENGTH 6
#define EFUSE_MASK_FILE_LEN 96
#define MAX_EFUSE_MAP_LEN 1536
#define TSSI_2G_CCK_SIZE 6
#define TSSI_2G_SIZE 5
#define TSSI_5G_SIZE 14

enum FLASH_STATUS_FLAGS {
	FLASH_STATUS_PROCESS = BIT0,
	FLASH_STATUS_MAP_FILE_LOADED = BIT1,
	FLASH_STATUS_MASK_FILE_LOADED = BIT2
};

/*
 *	@phl_com
 *	@hal_com
 *	@shadow_map: Buffer pointer for limited size logical map
 *	@mask: Buffer pointer for limited size mask read from mask file
 *	@map_version: Buffer pointer for map version read from map file
 *	@mask_version: Buffer pointer for mask version read from mask file
 *	@log_efuse_size: Limited logical map size
 *	@mask_size: Limited mask size
 *	@version_len: Length of verion field in map/mask
 *	@status: Efuse status
 *	@is_map_valid: Flag to check autoload status
 *	@reserved
 */

struct rtw_rf_tssi_slp_info {
	u8 tssi_2g_cck_gain_diff_A[TSSI_2G_CCK_SIZE];
	u8 tssi_2g_cck_gain_diff_B[TSSI_2G_CCK_SIZE];
	u8 tssi_2g_cck_gain_diff_C[TSSI_2G_CCK_SIZE];
	u8 tssi_2g_cck_gain_diff_D[TSSI_2G_CCK_SIZE];
	u8 tssi_2g_cck_cw_diff_A[TSSI_2G_CCK_SIZE];
	u8 tssi_2g_cck_cw_diff_B[TSSI_2G_CCK_SIZE];
	u8 tssi_2g_cck_cw_diff_C[TSSI_2G_CCK_SIZE];
	u8 tssi_2g_cck_cw_diff_D[TSSI_2G_CCK_SIZE];

	u8 tssi_2g_gain_diff_A[TSSI_2G_SIZE];
	u8 tssi_2g_gain_diff_B[TSSI_2G_SIZE];
	u8 tssi_2g_gain_diff_C[TSSI_2G_SIZE];
	u8 tssi_2g_gain_diff_D[TSSI_2G_SIZE];
	u8 tssi_2g_cw_diff_A[TSSI_2G_SIZE];
	u8 tssi_2g_cw_diff_B[TSSI_2G_SIZE];
	u8 tssi_2g_cw_diff_C[TSSI_2G_SIZE];
	u8 tssi_2g_cw_diff_D[TSSI_2G_SIZE];

	u8 tssi_5g_gain_diff_A[TSSI_5G_SIZE];
	u8 tssi_5g_gain_diff_B[TSSI_5G_SIZE];
	u8 tssi_5g_gain_diff_C[TSSI_5G_SIZE];
	u8 tssi_5g_gain_diff_D[TSSI_5G_SIZE];
	u8 tssi_5g_cw_diff_A[TSSI_5G_SIZE];
	u8 tssi_5g_cw_diff_B[TSSI_5G_SIZE];
	u8 tssi_5g_cw_diff_C[TSSI_5G_SIZE];
	u8 tssi_5g_cw_diff_D[TSSI_5G_SIZE];
};

struct rtw_rf_tssi_diff_info {
	u8 tssi_5g_bw20_bw40_diff_A[TSSI_5G_SIZE];
	u8 tssi_5g_bw20_bw40_diff_B[TSSI_5G_SIZE];
	u8 tssi_5g_bw20_bw40_diff_C[TSSI_5G_SIZE];
	u8 tssi_5g_bw20_bw40_diff_D[TSSI_5G_SIZE];

	u8 tssi_5g_bw80_bw40_diff_A[TSSI_5G_SIZE];
	u8 tssi_5g_bw80_bw40_diff_B[TSSI_5G_SIZE];
	u8 tssi_5g_bw80_bw40_diff_C[TSSI_5G_SIZE];
	u8 tssi_5g_bw80_bw40_diff_D[TSSI_5G_SIZE];

	u8 tssi_5g_bw160_bw40_diff_A[TSSI_5G_SIZE];
	u8 tssi_5g_bw160_bw40_diff_B[TSSI_5G_SIZE];
	u8 tssi_5g_bw160_bw40_diff_C[TSSI_5G_SIZE];
	u8 tssi_5g_bw160_bw40_diff_D[TSSI_5G_SIZE];
};

struct flash_t {
	struct dvobj_priv *dvobj;
	struct flash_dict *dict;
	struct rtw_rf_tssi_slp_info tssi_slp_info;
	struct rtw_rf_tssi_diff_info tssi_diff_info;
	u8 *shadow_map;
	u8 *mask;
	u8 *map_version;
	u8 *mask_version;
	u32 log_efuse_size;
	u32 mask_size;
	u32 limit_efuse_size;
	u32 limit_mask_size;
	u8 version_len;
	u8 status;
	u8 is_map_valid;
	u8 reserved;
	u8 rfe;
	char para_path[128];
};

enum rtw_hal_status flash_set_hw_cap(struct flash_t *flash);


#endif /* _RTW_FLASH_H_ */
