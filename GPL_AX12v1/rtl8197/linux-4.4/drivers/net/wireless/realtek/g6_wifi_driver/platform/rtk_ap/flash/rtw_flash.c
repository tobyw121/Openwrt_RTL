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
/*
The purpose of rtw_flash.c

Provide flash operations.
a. flash init function
b. flash shadow map read/write/update
c. flash information query, map size/used bytes...

*/

#define _HAL_FLASH_C_
//#include "../hal_headers.h"
#include <drv_types.h>
#include <../phl/hal_g6/hal_general_def.h>
#include <../phl/phl_headers.h>

#include "rtw_flash.h"
#include "../rtw_api_flash.h"
#include "rtw_flash_export.h"

struct flash_dict flash_id_table_8852a[] = {
	/*MAC part*/

	/*BB part*/

	/*RF part*/
	{EFUSE_INFO_RF_XTAL, "xcap", 0x2b9, 1},
	{EFUSE_INFO_RF_RFE, "rfe", 0x2ca, 1},
	{EFUSE_INFO_RF_THERMAL_A, "thermalA", 0x2d0, 1},
	{EFUSE_INFO_RF_THERMAL_B, "thermalB", 0x2d1, 1},
	{EFUSE_INFO_RF_THERMAL_C, "thermalC", 0x2d2, 1},
	{EFUSE_INFO_RF_THERMAL_D, "thermalD", 0x2d3, 1},

	{EFUSE_INFO_RF_2G_CCK_A_TSSI_DE_1, "2G_cck_tssi_A", 0x210, 1},
	{EFUSE_INFO_RF_2G_CCK_B_TSSI_DE_1, "2G_cck_tssi_B", 0x23a, 1},
	{EFUSE_INFO_RF_2G_CCK_C_TSSI_DE_1, "2G_cck_tssi_C", 0x264, 1},
	{EFUSE_INFO_RF_2G_CCK_D_TSSI_DE_1, "2G_cck_tssi_D", 0x28e, 1},

	{EFUSE_INFO_RF_2G_BW40M_A_TSSI_DE_1, "2G_bw40_1s_tssi_A", 0x216, 1},
	{EFUSE_INFO_RF_2G_BW40M_B_TSSI_DE_1, "2G_bw40_1s_tssi_B", 0x240, 1},
	{EFUSE_INFO_RF_2G_BW40M_C_TSSI_DE_1, "2G_bw40_1s_tssi_C", 0x26a, 1},
	{EFUSE_INFO_RF_2G_BW40M_D_TSSI_DE_1, "2G_bw40_1s_tssi_D", 0x294, 1},

	{EFUSE_INFO_RF_5G_BW40M_A_TSSI_DE_1, "5G_bw40_1s_tssi_A", 0x222, 1},
	{EFUSE_INFO_RF_5G_BW40M_B_TSSI_DE_1, "5G_bw40_1s_tssi_B", 0x24c, 1},
	{EFUSE_INFO_RF_5G_BW40M_C_TSSI_DE_1, "5G_bw40_1s_tssi_C", 0x276, 1},
	{EFUSE_INFO_RF_5G_BW40M_D_TSSI_DE_1, "5G_bw40_1s_tssi_D", 0x2a0, 1},

	{EFUSE_INFO_RF_RX_GAIN_K_A_2G_CCK, "2G_rx_gain_cck", 0x2d6, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_2G_OFMD, "2G_rx_gain_ofdm", 0x2d4, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_5GL, "5G_rx_gain_low", 0x2d8, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_5GM, "5G_rx_gain_mid", 0x2da, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_5GH, "5G_rx_gain_high", 0x2dc, 1},

	{EFUSE_INFO_RF_CHAN_PLAN, "chan_plan", 0x2b8, 1},

	{FLASH_INFO_COSTUM_PARA_PATH, "para_path", 0x0, 0},
	{FLASH_INFO_ID_ERROR, "", 0, 0}
};

struct flash_dict flash_id_table_8852c[] = {
	/*MAC part*/

	/*BB part*/

	/*RF part*/
	{EFUSE_INFO_RF_XTAL, "xcap", 0x2b9, 1},
	{EFUSE_INFO_RF_RFE, "rfe", 0x2ca, 1},
	{EFUSE_INFO_RF_THERMAL_A, "thermalA", 0x2d0, 1},
	{EFUSE_INFO_RF_THERMAL_B, "thermalB", 0x2d1, 1},
	{EFUSE_INFO_RF_THERMAL_C, "thermalC", 0x2d2, 1},
	{EFUSE_INFO_RF_THERMAL_D, "thermalD", 0x2d3, 1},

	{EFUSE_INFO_RF_2G_CCK_A_TSSI_DE_1, "2G_cck_tssi_A", 0x210, 1},
	{EFUSE_INFO_RF_2G_CCK_B_TSSI_DE_1, "2G_cck_tssi_B", 0x23a, 1},
	{EFUSE_INFO_RF_2G_CCK_C_TSSI_DE_1, "2G_cck_tssi_C", 0x264, 1},
	{EFUSE_INFO_RF_2G_CCK_D_TSSI_DE_1, "2G_cck_tssi_D", 0x28e, 1},

	{EFUSE_INFO_RF_2G_BW40M_A_TSSI_DE_1, "2G_bw40_1s_tssi_A", 0x216, 1},
	{EFUSE_INFO_RF_2G_BW40M_B_TSSI_DE_1, "2G_bw40_1s_tssi_B", 0x240, 1},
	{EFUSE_INFO_RF_2G_BW40M_C_TSSI_DE_1, "2G_bw40_1s_tssi_C", 0x26a, 1},
	{EFUSE_INFO_RF_2G_BW40M_D_TSSI_DE_1, "2G_bw40_1s_tssi_D", 0x294, 1},

	{EFUSE_INFO_RF_5G_BW40M_A_TSSI_DE_1, "5G_bw40_1s_tssi_A", 0x222, 1},
	{EFUSE_INFO_RF_5G_BW40M_B_TSSI_DE_1, "5G_bw40_1s_tssi_B", 0x24c, 1},
	{EFUSE_INFO_RF_5G_BW40M_C_TSSI_DE_1, "5G_bw40_1s_tssi_C", 0x276, 1},
	{EFUSE_INFO_RF_5G_BW40M_D_TSSI_DE_1, "5G_bw40_1s_tssi_D", 0x2a0, 1},

	{EFUSE_INFO_RF_RX_GAIN_K_A_2G_CCK, "2G_rx_gain_cck", 0x2d6, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_2G_OFMD, "2G_rx_gain_ofdm", 0x2d4, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_5GL, "5G_rx_gain_low", 0x2d8, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_5GM, "5G_rx_gain_mid", 0x2da, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_5GH, "5G_rx_gain_high", 0x2dc, 1},

	{EFUSE_INFO_RF_CHAN_PLAN, "chan_plan", 0x2b8, 1},

	{EFUSE_INFO_RF_5G_BW160M_BW40M_DIFF, "5G_BW160M_PWR_DIFF", 0x2e8, 1},
	{EFUSE_INFO_RF_6G_BW160M_BW40M_DIFF, "6G_BW160M_PWR_DIFF", 0x3ca, 1},

	{FLASH_INFO_COSTUM_PARA_PATH, "para_path", 0x0, 0},

	{FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_1, "2G_CCK_GAIN_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_1, "2G_CCK_GAIN_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_1, "2G_CCK_GAIN_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_1, "2G_CCK_GAIN_DIFF_D", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_1, "2G_CCK_CW_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_1, "2G_CCK_CW_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_1, "2G_CCK_CW_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_1, "2G_CCK_CW_DIFF_D", 0x0, 1},

	{FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_1, "2G_GAIN_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_1, "2G_GAIN_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_1, "2G_GAIN_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_1, "2G_GAIN_DIFF_D", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_1, "2G_CW_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_1, "2G_CW_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_1, "2G_CW_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_1, "2G_CW_DIFF_D", 0x0, 1},

	{FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_1, "5G_GAIN_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_1, "5G_GAIN_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_1, "5G_GAIN_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_1, "5G_GAIN_DIFF_D", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_1, "5G_CW_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_1, "5G_CW_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_1, "5G_CW_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_1, "5G_CW_DIFF_D", 0x0, 1},

	{FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_A_1, "5G_BW20M_40M_TSSI_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_B_1, "5G_BW20M_40M_TSSI_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_C_1, "5G_BW20M_40M_TSSI_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_D_1, "5G_BW20M_40M_TSSI_DIFF_D", 0x0, 1},
	{FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_A_1, "5G_BW80M_40M_TSSI_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_B_1, "5G_BW80M_40M_TSSI_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_C_1, "5G_BW80M_40M_TSSI_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_D_1, "5G_BW80M_40M_TSSI_DIFF_D", 0x0, 1},
	{FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_A_1, "5G_BW160M_40M_TSSI_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_B_1, "5G_BW160M_40M_TSSI_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_C_1, "5G_BW160M_40M_TSSI_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_D_1, "5G_BW160M_40M_TSSI_DIFF_D", 0x0, 1},

	{FLASH_INFO_ID_ERROR, "", 0, 0}
};

struct flash_dict flash_id_table_8192xb[] = {
	/*MAC part*/

	/*BB part*/

	/*RF part*/
	{EFUSE_INFO_RF_XTAL, "xcap", 0x2b9, 1},
	{EFUSE_INFO_RF_RFE, "rfe", 0x2ca, 1},
	{EFUSE_INFO_RF_THERMAL_A, "thermalA", 0x2d0, 1},
	{EFUSE_INFO_RF_THERMAL_B, "thermalB", 0x2d1, 1},
	{EFUSE_INFO_RF_THERMAL_C, "thermalC", 0x2d2, 1},
	{EFUSE_INFO_RF_THERMAL_D, "thermalD", 0x2d3, 1},

	{EFUSE_INFO_RF_2G_CCK_A_TSSI_DE_1, "2G_cck_tssi_A", 0x210, 1},
	{EFUSE_INFO_RF_2G_CCK_B_TSSI_DE_1, "2G_cck_tssi_B", 0x23a, 1},
	{EFUSE_INFO_RF_2G_CCK_C_TSSI_DE_1, "2G_cck_tssi_C", 0x264, 1},
	{EFUSE_INFO_RF_2G_CCK_D_TSSI_DE_1, "2G_cck_tssi_D", 0x28e, 1},

	{EFUSE_INFO_RF_2G_BW40M_A_TSSI_DE_1, "2G_bw40_1s_tssi_A", 0x216, 1},
	{EFUSE_INFO_RF_2G_BW40M_B_TSSI_DE_1, "2G_bw40_1s_tssi_B", 0x240, 1},
	{EFUSE_INFO_RF_2G_BW40M_C_TSSI_DE_1, "2G_bw40_1s_tssi_C", 0x26a, 1},
	{EFUSE_INFO_RF_2G_BW40M_D_TSSI_DE_1, "2G_bw40_1s_tssi_D", 0x294, 1},

	{EFUSE_INFO_RF_5G_BW40M_A_TSSI_DE_1, "5G_bw40_1s_tssi_A", 0x222, 1},
	{EFUSE_INFO_RF_5G_BW40M_B_TSSI_DE_1, "5G_bw40_1s_tssi_B", 0x24c, 1},
	{EFUSE_INFO_RF_5G_BW40M_C_TSSI_DE_1, "5G_bw40_1s_tssi_C", 0x276, 1},
	{EFUSE_INFO_RF_5G_BW40M_D_TSSI_DE_1, "5G_bw40_1s_tssi_D", 0x2a0, 1},

	{EFUSE_INFO_RF_RX_GAIN_K_A_2G_CCK, "2G_rx_gain_cck", 0x2d6, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_2G_OFMD, "2G_rx_gain_ofdm", 0x2d4, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_5GL, "5G_rx_gain_low", 0x2d8, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_5GM, "5G_rx_gain_mid", 0x2da, 1},
	{EFUSE_INFO_RF_RX_GAIN_K_A_5GH, "5G_rx_gain_high", 0x2dc, 1},

	{EFUSE_INFO_RF_CHAN_PLAN, "chan_plan", 0x2b8, 1},

	{EFUSE_INFO_RF_5G_BW160M_BW40M_DIFF, "5G_BW160M_PWR_DIFF", 0x2e8, 1},
	{EFUSE_INFO_RF_6G_BW160M_BW40M_DIFF, "6G_BW160M_PWR_DIFF", 0x3ca, 1},

	{FLASH_INFO_COSTUM_PARA_PATH, "para_path", 0x0, 0},

	{FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_1, "2G_CCK_GAIN_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_1, "2G_CCK_GAIN_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_1, "2G_CCK_GAIN_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_1, "2G_CCK_GAIN_DIFF_D", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_1, "2G_CCK_CW_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_1, "2G_CCK_CW_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_1, "2G_CCK_CW_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_1, "2G_CCK_CW_DIFF_D", 0x0, 1},

	{FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_1, "2G_GAIN_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_1, "2G_GAIN_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_1, "2G_GAIN_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_1, "2G_GAIN_DIFF_D", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_1, "2G_CW_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_1, "2G_CW_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_1, "2G_CW_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_1, "2G_CW_DIFF_D", 0x0, 1},

	{FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_1, "5G_GAIN_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_1, "5G_GAIN_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_1, "5G_GAIN_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_1, "5G_GAIN_DIFF_D", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_1, "5G_CW_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_1, "5G_CW_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_1, "5G_CW_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_1, "5G_CW_DIFF_D", 0x0, 1},

	{FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_A_1, "5G_BW20M_40M_TSSI_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_B_1, "5G_BW20M_40M_TSSI_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_C_1, "5G_BW20M_40M_TSSI_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_D_1, "5G_BW20M_40M_TSSI_DIFF_D", 0x0, 1},
	{FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_A_1, "5G_BW80M_40M_TSSI_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_B_1, "5G_BW80M_40M_TSSI_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_C_1, "5G_BW80M_40M_TSSI_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_D_1, "5G_BW80M_40M_TSSI_DIFF_D", 0x0, 1},
	{FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_A_1, "5G_BW160M_40M_TSSI_DIFF_A", 0x0, 1},
	{FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_B_1, "5G_BW160M_40M_TSSI_DIFF_B", 0x0, 1},
	{FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_C_1, "5G_BW160M_40M_TSSI_DIFF_C", 0x0, 1},
	{FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_D_1, "5G_BW160M_40M_TSSI_DIFF_D", 0x0, 1},

	{FLASH_INFO_ID_ERROR, "", 0, 0}
};

void flash_shadow_read_one_byte(struct flash_t *flash, u16 offset, u8 *value)
{
	*value = flash->shadow_map[offset];
}

void flash_shadow_read_two_byte(struct flash_t *flash, u16 offset, u16 *value)
{
	*value = flash->shadow_map[offset];
	*value |= flash->shadow_map[offset+1] << 8;
}

void flash_shadow_read_four_byte(struct flash_t *flash, u16 offset, u32 *value)
{
	*value = flash->shadow_map[offset];
	*value |= flash->shadow_map[offset+1] << 8;
	*value |= flash->shadow_map[offset+2] << 16;
	*value |= flash->shadow_map[offset+3] << 24;
}

void flash_shadow_write_one_byte(struct flash_t *flash, u16 offset, u16 value)
{
	flash->shadow_map[offset] = (u8)(value&0x00FF);
}

void flash_shadow_write_two_byte(struct flash_t *flash, u16 offset, u16 value)
{
	flash->shadow_map[offset] = (u8)(value&0x00FF);
	flash->shadow_map[offset+1] = (u8)((value&0xFF00) >> 8);
}

void flash_shadow_write_four_byte(struct flash_t *flash, u16 offset, u32 value)
{
	flash->shadow_map[offset] = (u8)(value&0x000000FF);
	flash->shadow_map[offset+1] = (u8)((value&0x0000FF00) >> 8);
	flash->shadow_map[offset+2] = (u8)((value&0x00FF0000) >> 16);
	flash->shadow_map[offset+3] = (u8)((value&0xFF000000) >> 24);
}

u32 flash_set_hw_cap(struct flash_t *flash)
{
	u32 status = _FAIL;
	struct rtw_phl_com_t *phl_com = flash->dvobj->phl_com;
	struct phy_hw_cap_t *phy_hw_cap;

	u8 i = 0;
	u8 pkg_type = 0xFF;
	u8 rfe_type = 0xFF;
	u8 xcap = 0xFF;
	u8 domain = 0xFF;

	status = rtw_flash_get_info(flash, EFUSE_INFO_RF_PKG_TYPE, &pkg_type,
								sizeof(pkg_type));

	if (status != _SUCCESS) {
		RTW_WARN(FUNC_DEV_FMT" Get pkg type fail! Status(%x)\n",
		         FUNC_DEV_ARG(flash->dvobj), status);
	}

	status = rtw_flash_get_info(flash, EFUSE_INFO_RF_RFE,
	                            &rfe_type, sizeof(rfe_type));

	if (status != _SUCCESS) {
		RTW_WARN(FUNC_DEV_FMT" Get rfe type fail! Status(%x)\n",
		         FUNC_DEV_ARG(flash->dvobj), status);
	}

	status = rtw_flash_get_info(flash, EFUSE_INFO_RF_XTAL,
	                            &xcap, sizeof(xcap));

	if (status != _SUCCESS) {
		RTW_WARN(FUNC_DEV_FMT" Get xcap type fail! Status(%x)\n",
		         FUNC_DEV_ARG(flash->dvobj), status);
	}

	status = rtw_flash_get_info(flash, EFUSE_INFO_RF_CHAN_PLAN, &domain,
	                            sizeof(domain));

	if (status != _SUCCESS) {
		RTW_WARN(FUNC_DEV_FMT" Get domain fail! Status(%x)\n",
		         FUNC_DEV_ARG(flash->dvobj), status);
	}

#ifdef WKARD_PON_PLATFORM
	rfe_type = 50;
#endif
	phl_com->dev_sw_cap.pkg_type = pkg_type;
	phl_com->dev_sw_cap.xcap = xcap;
	phl_com->dev_sw_cap.domain = domain;

	if (   RTW_PHL_STATUS_SUCCESS
	    != rtw_phl_nvm_apply_dev_cap(phl_com->phl_priv))
		return _FAIL;

	return _SUCCESS;
}

u32 rtw_flash_shadow_load(void *flash, bool is_limit, u8 *map)
{
	u32 status = _SUCCESS;
	struct flash_t *flash_info = flash;

	status = rtw_hal_flash_read_map(flash_info, map);

	return status;
}

u32 rtw_flash_shadow_update(void *flash, bool is_limit)
{
	u32 status = _FAIL;
	u32 reload_status = _FAIL;
	struct flash_t *flash_info = flash;
	u32 map_size = 0, mask_size = 0;

	if (   TEST_STATUS_FLAG(flash_info->status, FLASH_STATUS_PROCESS)
	    == false) {
		RTW_ERR(FUNC_DEV_FMT" flash map not load yet!\n",
		        FUNC_DEV_ARG(flash_info->dvobj));
		goto exit;
	}

	/* Load flash mask file before PG */
	if (   TEST_STATUS_FLAG(flash_info->status,
	                        FLASH_STATUS_MASK_FILE_LOADED)
	    == false) {
		RTW_ERR(FUNC_DEV_FMT" flash mask not load yet!\n",
			FUNC_DEV_ARG(flash_info->dvobj));
		goto exit;
	}

	status = _SUCCESS;
exit:
	return status;
}

u32
rtw_flash_shadow_read(void *flash, u8 byte_count, u16 offset, u32 *value,
                      bool is_limit)
{
	u32 status = _FAIL;
	struct flash_t *flash_info = flash;
	u32	flash_size = 0;

	if (is_limit)
		flash_size = flash_info->limit_efuse_size;
	else
		flash_size = flash_info->log_efuse_size;

	if ((u32)(offset+byte_count) > flash_size) {
		RTW_ERR(FUNC_DEV_FMT" Invalid offset! (%u/%u/%u)\n",
		        FUNC_DEV_ARG(flash_info->dvobj),
		        offset, byte_count, flash_size);
		goto exit;
	}

	if (byte_count == 1)
		flash_shadow_read_one_byte(flash_info, offset, (u8 *)value);
	else if (byte_count == 2)
		flash_shadow_read_two_byte(flash_info, offset, (u16 *)value);
	else if (byte_count == 4)
		flash_shadow_read_four_byte(flash_info, offset, (u32 *)value);
	else {
		RTW_ERR(FUNC_DEV_FMT" invalid byte count! (%u)\n",
		        FUNC_DEV_ARG(flash_info->dvobj), byte_count);
		goto exit;
	}

	status = _SUCCESS;
exit:
	return status;
}

u32
rtw_flash_shadow_write(void *flash, u8 byte_count, u16 offset, u32 value,
					   bool is_limit)
{
	u32 status = _FAIL;
	struct flash_t *flash_info = flash;
	u32	flash_size = 0;

	if (is_limit)
		flash_size = flash_info->limit_efuse_size;
	else
		flash_size = flash_info->log_efuse_size;

	if ((u32)(offset+byte_count) > flash_size) {
		RTW_ERR(FUNC_DEV_FMT" Invalid offset! (%u/%u/%u)\n",
		        FUNC_DEV_ARG(flash_info->dvobj),
		        offset, byte_count, flash_size);
		goto exit;
	}

	if (byte_count == 1)
		flash_shadow_write_one_byte(flash_info, offset, (u8)value);
	else if (byte_count == 2)
		flash_shadow_write_two_byte(flash_info, offset, (u16)value);
	else if (byte_count == 4)
		flash_shadow_write_four_byte(flash_info, offset, (u32)value);
	else {
		RTW_ERR(FUNC_DEV_FMT" invalid byte count! (%u)\n",
		        FUNC_DEV_ARG(flash_info->dvobj), byte_count);
		goto exit;
	}

	status = _SUCCESS;
exit:
	return status;
}


u32
rtw_flash_get_info(void *flash, enum rtw_efuse_info info_type,
                   void *value, u8 size)
{
	struct flash_t *flash_info = flash;
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	u32 status = _FAIL;

	if ((enum rtw_flash_info)info_type <= FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_14 &&
	    (enum rtw_flash_info)info_type >= FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_1)
		rtw_flash_get_tssi_slp_info(flash_info->dvobj,
					    flash_info,
					    (enum rtw_flash_info) info_type,
					    value,
					    size);
	else if ((enum rtw_flash_info)info_type <= FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_D_14 &&
	    (enum rtw_flash_info)info_type >= FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_A_1)
		rtw_flash_get_tssi_diff_info(flash_info->dvobj,
					    flash_info,
					    (enum rtw_flash_info) info_type,
					    value,
					    size);
	else
		phl_status = rtw_phl_extract_efuse_info(flash_info->dvobj->phl,
							flash_info->shadow_map,
							info_type,
							value,
							size,
							1);

	return (phl_status == RTW_PHL_STATUS_SUCCESS) ? _SUCCESS : _FAIL;
}

u32
rtw_flash_set_info(void *flash, enum rtw_efuse_info info_type,
                   void *value, u8 size, u32 offset)
{
	struct flash_t *flash_info = flash;
	u32 status = _SUCCESS;

	_rtw_memcpy(flash_info->shadow_map + offset, value, size);

	return status;
}

void rtw_flash_process(void *flash)
{
	struct flash_t *flash_info = (struct flash_t *)flash;
	flash_set_hw_cap(flash_info);
}

u32 rtw_flash_init(struct dvobj_priv *dvobj, void **flash)
{
	u32 ret = _FAIL;
	struct flash_t *flash_info = NULL;
	enum rtw_phl_status phl_status;

	flash_info = (struct flash_t *)rtw_zvmalloc(sizeof(struct flash_t));

	if (flash_info == NULL) {
		ret = _FAIL;
		goto error_init;
	}

	flash_info->dvobj = dvobj;

	*flash = flash_info;

#ifdef WKARD_PON_PLATFORM
	/*default path when insmod*/
	rtw_flash_set_para_path(flash_info, REALTEK_CONFIG_PATH);
#else
	rtw_flash_set_para_path(flash_info, rtw_phy_file_path);
#endif /* WKARD_PON_PLATFORM */

	return _SUCCESS;

error_init:
	if (flash_info != NULL) {
		rtw_vmfree(flash_info, sizeof(struct flash_t));
	}
	return ret;
}

u32 rtw_flash_efuse_init(struct dvobj_priv *dvobj)
{
	u32 ret = _FAIL;
	struct flash_t *flash_info = (struct flash_t *)dvobj->nvm;
	enum rtw_phl_status phl_status;
	const char *ic_name = dvobj->phl_com->hal_spec.ic_name;

	flash_info->dict = NULL;

	if (strcmp(ic_name, "rtl8852a") == 0) {
		flash_info->dict = flash_id_table_8852a;
	}
	if (strcmp(ic_name, "rtl8852c") == 0) {
		flash_info->dict = flash_id_table_8852c;
	}
	if (strcmp(ic_name, "rtl8192xb") == 0 || strcmp(ic_name, "rtl8832br") == 0 ) {
		flash_info->dict = flash_id_table_8192xb;
	}

	if (flash_info->dict != NULL) {
		RTW_INFO(FUNC_DEVID_FMT" Use %s's efuse dictionary.\n",
		          FUNC_DEVID_ARG(dvobj), ic_name);
	} else {
		RTW_ERR(FUNC_DEVID_FMT" flash dictionary is not found for \"%s\".\n",
		        FUNC_DEVID_ARG(dvobj), ic_name);
	}

	/* Allocate shadow map memory */
	phl_status = rtw_phl_get_efuse_size(dvobj->phl,
	                                    &(flash_info->log_efuse_size),
	                                    &(flash_info->limit_efuse_size),
	                                    &(flash_info->mask_size),
	                                    &(flash_info->limit_mask_size));


	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR(FUNC_DEV_FMT": Get efuse size fail!\n",
		        FUNC_DEV_ARG(dvobj));
		goto error_init;
	}

	flash_info->shadow_map = (u8 *)rtw_zvmalloc(flash_info->log_efuse_size);

	if (flash_info->shadow_map == NULL) {
		RTW_ERR(FUNC_DEV_FMT": Allocate shadow efuse map fail!\n",
			FUNC_DEV_ARG(dvobj));
		goto error_init;
	}

	/* Allocate mask memory */
	flash_info->mask = (u8 *)rtw_zvmalloc(flash_info->mask_size);

	if(flash_info->mask == NULL) {
		RTW_ERR(FUNC_DEV_FMT": Allocate efuse mask fail!\n",
					FUNC_DEV_ARG(dvobj));
		goto error_init;
	}

	/* There will be a MAC API to get version field length */
	flash_info->version_len = MAX_EFUSE_FILE_VERSION_LENGTH;

	flash_info->map_version = (u8 *)rtw_zvmalloc(flash_info->version_len);

	if(flash_info->map_version == NULL) {
		ret = _FAIL;
		goto error_init;
	}

	flash_info->mask_version = rtw_zvmalloc(flash_info->version_len);
	if(flash_info->mask_version == NULL) {
		ret = _FAIL;
		goto error_init;
	}

	flash_info->is_map_valid = 1;

	return _SUCCESS;

error_init:
	if (flash_info->map_version != NULL)
		rtw_vmfree(flash_info->map_version, flash_info->version_len);

	if (flash_info->mask != NULL)
		rtw_vmfree(flash_info->mask, flash_info->mask_size);

	if (flash_info->shadow_map != NULL)
		rtw_vmfree(flash_info->shadow_map,
		           flash_info->log_efuse_size);

	return ret;
}

void rtw_flash_deinit(struct dvobj_priv *dvobj, void *flash)
{
	struct flash_t *flash_info = flash;

	if (flash_info == NULL)
		return;

	if (flash_info->mask)
		rtw_vmfree(flash_info->mask, flash_info->mask_size);
	if (flash_info->map_version)
		rtw_vmfree(flash_info->map_version, flash_info->version_len);
	if (flash_info->mask_version)
		rtw_vmfree(flash_info->mask_version, flash_info->version_len);
	if (flash_info->shadow_map)
		rtw_vmfree(flash_info->shadow_map, flash_info->log_efuse_size);

	rtw_vmfree(flash_info, sizeof(struct flash_t));
}

void deinit_flash_id_table(struct dvobj_priv *dvobj, struct flash_dict *table, u16 len)
{
	RTW_INFO(FUNC_DEV_FMT"\n", FUNC_DEV_ARG(dvobj));
	rtw_vmfree(table, len * sizeof(struct flash_dict));
}

enum rtw_flash_info
lookup_flash_id(struct flash_dict *table, u8 *cmd, u32 *offset, u8 *size)
{
	enum rtw_flash_info ret = FLASH_INFO_ID_ERROR;
	int i;

	for (i = 0; table[i].id != FLASH_INFO_ID_ERROR; i++) {
		if (0 == strcmp(table[i].str, cmd)) {
			*size = table[i].size;
			*offset = table[i].offset;
			return table[i].id;
		}
	}

	RTW_ERR("%s is not defined in flash_dict table!\n", cmd);
	return ret;
}

void rtw_flash_set_offset(void *flash, u32 offset, u32 value)
{
	struct flash_t *flash_info = flash;
	if(offset >= flash_info->log_efuse_size){
		RTW_PRINT("%s offset:%u, log_efuse_size:%u\n", __func__, offset, flash_info->log_efuse_size);
		return;
	}else{
		flash_info->shadow_map[offset] = value;
		RTW_INFO(FUNC_DEV_FMT" shadowmap[%d] = %d\n",
		         FUNC_DEV_ARG(flash_info->dvobj),
		         offset, flash_info->shadow_map[offset]);
	}
}

u8 rtw_flash_get_offset(void *flash, u32 offset)
{
	struct flash_t *flash_info = flash;
	u8 ret = 0;

	if(offset >= flash_info->log_efuse_size){
		RTW_PRINT("%s offset:%u, log_efuse_size:%u\n", __func__, offset, flash_info->log_efuse_size);
		return ret;
	}else
		return flash_info->shadow_map[offset];
}

void rtw_flash_dump(void *flash)
{
	struct flash_t *flash_info = flash;
	int i;

	RTW_PRINT(FUNC_DEV_FMT": flash_info size = %u\n",
	          FUNC_DEV_ARG(flash_info->dvobj),
	          flash_info->log_efuse_size);
	RTW_PRINT_DUMP("Shadow:", flash_info->shadow_map,
	               flash_info->log_efuse_size);
}

void rtw_flash_set_para_path(void *flash, const char *path)
{
	struct flash_t *flash_info = flash;
	strncpy(flash_info->para_path, path, sizeof(flash_info->para_path) - 1);
}

const char *rtw_flash_get_para_path(void *flash)
{
	struct flash_t *flash_info = flash;
	return flash_info->para_path;
}

void rtw_flash_set_tssi_slp_info(void *flash, enum rtw_flash_info info_type,
                                 u8 *input_data, u8 len)
{
	struct flash_t *flash_info = flash;
	u8 i = 0, value = 0;
	u8 temp[3] = {0};

	RTW_INFO ("%s: input_data=%s, len=%d\n", __func__, input_data, strlen(input_data));
	for (i = 0; i < len; i++) {
		_rtw_memset(temp, 0, sizeof(temp));
		_os_strncpy(temp, input_data+i*2, 2);
		value = _atoi(temp, 16);

		switch (info_type) {
			case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_6:
				if (i < TSSI_2G_CCK_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_A[i] = value;
				break;
			case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_6:
				if (i < TSSI_2G_CCK_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_B[i] = value;
				break;
			case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_6:
				if (i < TSSI_2G_CCK_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_C[i] = value;
				break;
			case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_6:
				if (i < TSSI_2G_CCK_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_D[i] = value;
				break;
			case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_5:
				if (i < TSSI_2G_SIZE)
					flash_info->tssi_slp_info.tssi_2g_gain_diff_A[i] = value;
				break;
			case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_5:
				if (i < TSSI_2G_SIZE)
					flash_info->tssi_slp_info.tssi_2g_gain_diff_B[i] = value;
				break;
			case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_5:
				if (i < TSSI_2G_SIZE)
					flash_info->tssi_slp_info.tssi_2g_gain_diff_C[i] = value;
				break;
			case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_5:
				if (i < TSSI_2G_SIZE)
					flash_info->tssi_slp_info.tssi_2g_gain_diff_D[i] = value;
				break;
			case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_slp_info.tssi_5g_gain_diff_A[i] = value;
				break;
			case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_slp_info.tssi_5g_gain_diff_B[i] = value;
				break;
			case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_slp_info.tssi_5g_gain_diff_C[i] = value;
				break;
			case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_slp_info.tssi_5g_gain_diff_D[i] = value;
				break;
			case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_6:
				if (i < TSSI_2G_CCK_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_A[i] = value;
				break;
			case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_6:
				if (i < TSSI_2G_CCK_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_B[i] = value;
				break;
			case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_6:
				if (i < TSSI_2G_CCK_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_C[i] = value;
				break;
			case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_6:
				if (i < TSSI_2G_CCK_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_D[i] = value;
				break;
			case FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_5:
				if (i < TSSI_2G_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cw_diff_A[i] = value;
				break;
			case FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_5:
				if (i < TSSI_2G_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cw_diff_B[i] = value;
				break;
			case FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_5:
				if (i < TSSI_2G_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cw_diff_C[i] = value;
				break;
			case FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_5:
				if (i < TSSI_2G_SIZE)
					flash_info->tssi_slp_info.tssi_2g_cw_diff_D[i] = value;
				break;
			case FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_slp_info.tssi_5g_cw_diff_A[i] = value;
				break;
			case FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_slp_info.tssi_5g_cw_diff_B[i] = value;
				break;
			case FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_slp_info.tssi_5g_cw_diff_C[i] = value;
				break;
			case FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_slp_info.tssi_5g_cw_diff_D[i] = value;
				break;
			default:
				break;
		}
	}
}

void rtw_flash_get_tssi_slp_info(struct dvobj_priv *d, void *flash,
                                 enum rtw_flash_info info_type,
                                 void *value, u8 size)
{
	struct flash_t *flash_info = flash;
	u8 offset = 0;

	RTW_INFO ("%s: info_type=%d\n", __func__, info_type);
	switch (info_type) {
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_A[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_B[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_C[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_D[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_gain_diff_A[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_gain_diff_B[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_gain_diff_C[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_gain_diff_D[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_5g_gain_diff_A[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_5g_gain_diff_B[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_5g_gain_diff_C[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_5g_gain_diff_D[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_A[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_B[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_C[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_D[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cw_diff_A[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cw_diff_B[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cw_diff_C[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_2g_cw_diff_D[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_5g_cw_diff_A[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_5g_cw_diff_B[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_5g_cw_diff_C[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_slp_info.tssi_5g_cw_diff_D[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		default:
			break;
	}
}

void rtw_flash_set_tssi_diff_info(void *flash, enum rtw_flash_info info_type,
                                 u8 *input_data, u8 len)
{
	struct flash_t *flash_info = flash;
	u8 i = 0, value = 0;
	u8 temp[3] = {0};

	RTW_INFO ("%s: input_data=%s, len=%d\n", __func__, input_data, strlen(input_data));
	for (i = 0; i < len; i++) {
		_rtw_memset(temp, 0, sizeof(temp));
		_os_strncpy(temp, input_data+i*2, 2);
		value = _atoi(temp, 16);

		switch (info_type) {
			case FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_A_1 ... FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_A_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw20_bw40_diff_A[i] = value;
				break;
			case FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_B_1 ... FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_B_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw20_bw40_diff_B[i] = value;
				break;
			case FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_C_1 ... FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_C_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw20_bw40_diff_C[i] = value;
				break;
			case FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_D_1 ... FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_D_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw20_bw40_diff_D[i] = value;
				break;
			case FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_A_1 ... FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_A_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw80_bw40_diff_A[i] = value;
				break;
			case FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_B_1 ... FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_B_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw80_bw40_diff_B[i] = value;
				break;
			case FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_C_1 ... FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_C_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw80_bw40_diff_C[i] = value;
				break;
			case FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_D_1 ... FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_D_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw80_bw40_diff_D[i] = value;
				break;
			case FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_A_1 ... FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_A_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw160_bw40_diff_A[i] = value;
				break;
			case FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_B_1 ... FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_B_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw160_bw40_diff_B[i] = value;
				break;
			case FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_C_1 ... FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_C_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw160_bw40_diff_C[i] = value;
				break;
			case FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_D_1 ... FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_D_14:
				if (i < TSSI_5G_SIZE)
					flash_info->tssi_diff_info.tssi_5g_bw160_bw40_diff_D[i] = value;
				break;

			default:
				break;
		}
	}
}

void rtw_flash_get_tssi_diff_info(struct dvobj_priv *d, void *flash,
                                 enum rtw_flash_info info_type,
                                 void *value, u8 size)
{
	struct flash_t *flash_info = flash;
	u8 offset = 0;

	RTW_INFO ("%s: info_type=%d\n", __func__, info_type);
	switch (info_type) {
		case FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_A_1 ... FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_A_14:
			offset = info_type - FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_A_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw20_bw40_diff_A[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_B_1 ... FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_B_14:
			offset = info_type - FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_B_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw20_bw40_diff_B[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_C_1 ... FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_C_14:
			offset = info_type - FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_C_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw20_bw40_diff_C[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_D_1 ... FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_D_14:
			offset = info_type - FLASH_INFO_RF_5G_BW20M_BW40M_TSSI_DIFF_D_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw20_bw40_diff_D[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_A_1 ... FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_A_14:
			offset = info_type - FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_A_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw80_bw40_diff_A[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_B_1 ... FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_B_14:
			offset = info_type - FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_B_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw80_bw40_diff_B[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_C_1 ... FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_C_14:
			offset = info_type - FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_C_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw80_bw40_diff_C[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_D_1 ... FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_D_14:
			offset = info_type - FLASH_INFO_RF_5G_BW80M_BW40M_TSSI_DIFF_D_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw80_bw40_diff_D[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_A_1 ... FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_A_14:
			offset = info_type - FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_A_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw160_bw40_diff_A[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_B_1 ... FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_B_14:
			offset = info_type - FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_B_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw160_bw40_diff_B[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_C_1 ... FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_C_14:
			offset = info_type - FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_C_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw160_bw40_diff_C[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_D_1 ... FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_D_14:
			offset = info_type - FLASH_INFO_RF_5G_BW160M_BW40M_TSSI_DIFF_D_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(d, value, &(flash_info->tssi_diff_info.tssi_5g_bw160_bw40_diff_D[offset]), size);
			else
				RTW_ERR ("info_type error, info_type=%d\n", info_type);
			break;

		default:
			break;
	}
}
