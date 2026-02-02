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
#define _HAL_API_FLASH_C_

#ifndef CONFIG_RTW_DRV_HAS_NVM
#include "hal_headers.h"
#include "flash/hal_flash.h"
#include "flash/hal_flash_export.h"

struct flash_dict *flash_id_table = NULL;

struct flash_dict flash_id_table_8852a[] = {
	/*MAC part*/

	/*BB part*/

	/*RF part*/
	{FLASH_INFO_RF_XTAL, "xcap", 0x2b9, 1},
	{FLASH_INFO_RF_RFE, "rfe", 0x2ca, 1},
	{FLASH_INFO_RF_THERMAL_A, "thermalA", 0x2d0, 1},
	{FLASH_INFO_RF_THERMAL_B, "thermalB", 0x2d1, 1},
	{FLASH_INFO_RF_THERMAL_C, "thermalC", 0x2d2, 1},
	{FLASH_INFO_RF_THERMAL_D, "thermalD", 0x2d3, 1},

	{FLASH_INFO_RF_2G_CCK_A_TSSI_DE_1, "2G_cck_tssi_A", 0x210, 1},
	{FLASH_INFO_RF_2G_CCK_B_TSSI_DE_1, "2G_cck_tssi_B", 0x23a, 1},
	{FLASH_INFO_RF_2G_CCK_C_TSSI_DE_1, "2G_cck_tssi_C", 0x264, 1},
	{FLASH_INFO_RF_2G_CCK_D_TSSI_DE_1, "2G_cck_tssi_D", 0x28e, 1},

	{FLASH_INFO_RF_2G_BW40M_A_TSSI_DE_1, "2G_bw40_1s_tssi_A", 0x216, 1},
	{FLASH_INFO_RF_2G_BW40M_B_TSSI_DE_1, "2G_bw40_1s_tssi_B", 0x240, 1},
	{FLASH_INFO_RF_2G_BW40M_C_TSSI_DE_1, "2G_bw40_1s_tssi_C", 0x26a, 1},
	{FLASH_INFO_RF_2G_BW40M_D_TSSI_DE_1, "2G_bw40_1s_tssi_D", 0x294, 1},

	{FLASH_INFO_RF_5G_BW40M_A_TSSI_DE_1, "5G_bw40_1s_tssi_A", 0x222, 1},
	{FLASH_INFO_RF_5G_BW40M_B_TSSI_DE_1, "5G_bw40_1s_tssi_B", 0x24c, 1},
	{FLASH_INFO_RF_5G_BW40M_C_TSSI_DE_1, "5G_bw40_1s_tssi_C", 0x276, 1},
	{FLASH_INFO_RF_5G_BW40M_D_TSSI_DE_1, "5G_bw40_1s_tssi_D", 0x2a0, 1},

	{FLASH_INFO_RF_RX_GAIN_K_A_2G_CCK, "2G_rx_gain_cck", 0x2d6, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_2G_OFMD, "2G_rx_gain_ofdm", 0x2d4, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_5GL, "5G_rx_gain_low", 0x2d8, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_5GM, "5G_rx_gain_mid", 0x2da, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_5GH, "5G_rx_gain_high", 0x2dc, 1},

	{FLASH_INFO_RF_CHAN_PLAN, "chan_plan", 0x2b8, 1},

	{FLASH_INFO_COSTUM_PARA_PATH, "para_path", 0x0, 0},
};

struct flash_dict flash_id_table_8852c[] = {
	/*MAC part*/

	/*BB part*/

	/*RF part*/
	{FLASH_INFO_RF_XTAL, "xcap", 0x2b9, 1},
	{FLASH_INFO_RF_RFE, "rfe", 0x2ca, 1},
	{FLASH_INFO_RF_THERMAL_A, "thermalA", 0x2d0, 1},
	{FLASH_INFO_RF_THERMAL_B, "thermalB", 0x2d1, 1},
	{FLASH_INFO_RF_THERMAL_C, "thermalC", 0x2d2, 1},
	{FLASH_INFO_RF_THERMAL_D, "thermalD", 0x2d3, 1},

	{FLASH_INFO_RF_2G_CCK_A_TSSI_DE_1, "2G_cck_tssi_A", 0x210, 1},
	{FLASH_INFO_RF_2G_CCK_B_TSSI_DE_1, "2G_cck_tssi_B", 0x23a, 1},
	{FLASH_INFO_RF_2G_CCK_C_TSSI_DE_1, "2G_cck_tssi_C", 0x264, 1},
	{FLASH_INFO_RF_2G_CCK_D_TSSI_DE_1, "2G_cck_tssi_D", 0x28e, 1},

	{FLASH_INFO_RF_2G_BW40M_A_TSSI_DE_1, "2G_bw40_1s_tssi_A", 0x216, 1},
	{FLASH_INFO_RF_2G_BW40M_B_TSSI_DE_1, "2G_bw40_1s_tssi_B", 0x240, 1},
	{FLASH_INFO_RF_2G_BW40M_C_TSSI_DE_1, "2G_bw40_1s_tssi_C", 0x26a, 1},
	{FLASH_INFO_RF_2G_BW40M_D_TSSI_DE_1, "2G_bw40_1s_tssi_D", 0x294, 1},

	{FLASH_INFO_RF_5G_BW40M_A_TSSI_DE_1, "5G_bw40_1s_tssi_A", 0x222, 1},
	{FLASH_INFO_RF_5G_BW40M_B_TSSI_DE_1, "5G_bw40_1s_tssi_B", 0x24c, 1},
	{FLASH_INFO_RF_5G_BW40M_C_TSSI_DE_1, "5G_bw40_1s_tssi_C", 0x276, 1},
	{FLASH_INFO_RF_5G_BW40M_D_TSSI_DE_1, "5G_bw40_1s_tssi_D", 0x2a0, 1},

	{FLASH_INFO_RF_RX_GAIN_K_A_2G_CCK, "2G_rx_gain_cck", 0x2d6, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_2G_OFMD, "2G_rx_gain_ofdm", 0x2d4, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_5GL, "5G_rx_gain_low", 0x2d8, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_5GM, "5G_rx_gain_mid", 0x2da, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_5GH, "5G_rx_gain_high", 0x2dc, 1},

	{FLASH_INFO_RF_CHAN_PLAN, "chan_plan", 0x2b8, 1},

	{FLASH_INFO_COSTUM_PARA_PATH, "para_path", 0x0, 0},
};

struct flash_dict flash_id_table_8192xb[] = {
	/*MAC part*/

	/*BB part*/

	/*RF part*/
	{FLASH_INFO_RF_XTAL, "xcap", 0x2b9, 1},
	{FLASH_INFO_RF_RFE, "rfe", 0x2ca, 1},
	{FLASH_INFO_RF_THERMAL_A, "thermalA", 0x2d0, 1},
	{FLASH_INFO_RF_THERMAL_B, "thermalB", 0x2d1, 1},
	{FLASH_INFO_RF_THERMAL_C, "thermalC", 0x2d2, 1},
	{FLASH_INFO_RF_THERMAL_D, "thermalD", 0x2d3, 1},

	{FLASH_INFO_RF_2G_CCK_A_TSSI_DE_1, "2G_cck_tssi_A", 0x210, 1},
	{FLASH_INFO_RF_2G_CCK_B_TSSI_DE_1, "2G_cck_tssi_B", 0x23a, 1},
	{FLASH_INFO_RF_2G_CCK_C_TSSI_DE_1, "2G_cck_tssi_C", 0x264, 1},
	{FLASH_INFO_RF_2G_CCK_D_TSSI_DE_1, "2G_cck_tssi_D", 0x28e, 1},

	{FLASH_INFO_RF_2G_BW40M_A_TSSI_DE_1, "2G_bw40_1s_tssi_A", 0x216, 1},
	{FLASH_INFO_RF_2G_BW40M_B_TSSI_DE_1, "2G_bw40_1s_tssi_B", 0x240, 1},
	{FLASH_INFO_RF_2G_BW40M_C_TSSI_DE_1, "2G_bw40_1s_tssi_C", 0x26a, 1},
	{FLASH_INFO_RF_2G_BW40M_D_TSSI_DE_1, "2G_bw40_1s_tssi_D", 0x294, 1},

	{FLASH_INFO_RF_5G_BW40M_A_TSSI_DE_1, "5G_bw40_1s_tssi_A", 0x222, 1},
	{FLASH_INFO_RF_5G_BW40M_B_TSSI_DE_1, "5G_bw40_1s_tssi_B", 0x24c, 1},
	{FLASH_INFO_RF_5G_BW40M_C_TSSI_DE_1, "5G_bw40_1s_tssi_C", 0x276, 1},
	{FLASH_INFO_RF_5G_BW40M_D_TSSI_DE_1, "5G_bw40_1s_tssi_D", 0x2a0, 1},

	{FLASH_INFO_RF_RX_GAIN_K_A_2G_CCK, "2G_rx_gain_cck", 0x2d6, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_2G_OFMD, "2G_rx_gain_ofdm", 0x2d4, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_5GL, "5G_rx_gain_low", 0x2d8, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_5GM, "5G_rx_gain_mid", 0x2da, 1},
	{FLASH_INFO_RF_RX_GAIN_K_A_5GH, "5G_rx_gain_high", 0x2dc, 1},

	{FLASH_INFO_RF_CHAN_PLAN, "chan_plan", 0x2b8, 1},

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
};

int _atoi(char *s, int base)
{
	int k = 0;
	int sign = 1;
	if (NULL == s){
		return 0;
	}

	k = 0;
	if (base == 10) {
		if(*s== '-') {
			sign = -1;
			s++;
		}
		while (*s != '\0' && *s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
		k *= sign;
	}
	else {
		while (*s != '\0') {
			int v;
			if ( *s >= '0' && *s <= '9')
				v = *s - '0';
			else if ( *s >= 'a' && *s <= 'f')
				v = *s - 'a' + 10;
			else if ( *s >= 'A' && *s <= 'F')
				v = *s - 'A' + 10;
			else {
				//RTW_ERR("error hex format! %c\n", s);
				return k;
			}

			k = 16 * k + v;
			s++;
		}
	}
	return k;
}

enum rtw_hal_status rtw_hal_flash_get_info(struct rtw_hal_com_t *hal_com,
	enum rtw_flash_info info_type, void *value, u8 size)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = hal_com->hal_priv;

	status = rtw_flash_get_info(hal_info->flash, info_type, value, size);

	return status;
}

enum rtw_hal_status rtw_hal_flash_set_info(void *hal, u8 *cmd, u8 *data, u8 *ic_name)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	enum rtw_flash_info query_id;
	static u32 content_size;

	u32 query_offset;
	u8 query_size;
	u8 i, value, len = 1;
	u8 temp[3];

	if (strcmp(ic_name, "rtl8852a") == 0 && flash_id_table == NULL){

		PHL_PRINT("mount rtl8852a id table\n");
		flash_id_table = flash_id_table_8852a;
		content_size = sizeof(flash_id_table_8852a);

	}

	else if (strcmp(ic_name, "rtl8852c") == 0 && flash_id_table == NULL){

		PHL_PRINT("mount rtl8852c id table\n");
		flash_id_table = flash_id_table_8852c;
		content_size = sizeof(flash_id_table_8852c);

	}
	else if ((strcmp(ic_name, "rtl8192xb") == 0  || strcmp(ic_name, "rtl8832br") == 0) && flash_id_table == NULL){

		PHL_PRINT("mount rtl8192xb id table\n");
		flash_id_table = flash_id_table_8192xb;
		content_size = sizeof(flash_id_table_8192xb);

	}

	query_id = rtw_hal_flash_lookup_id(flash_id_table, content_size/sizeof(struct flash_dict), cmd, &query_offset, &query_size);

	if(query_id != FLASH_INFO_ID_ERROR){

		switch(query_id){

		case FLASH_INFO_COSTUM_PARA_PATH:
			rtw_flash_set_para_path(hal_info->flash, data);
			break;

		case FLASH_INFO_RF_2G_CCK_A_TSSI_DE_1 ... FLASH_INFO_RF_2G_CCK_A_TSSI_DE_6:
		case FLASH_INFO_RF_2G_CCK_B_TSSI_DE_1 ... FLASH_INFO_RF_2G_CCK_B_TSSI_DE_6:
		case FLASH_INFO_RF_2G_CCK_C_TSSI_DE_1 ... FLASH_INFO_RF_2G_CCK_C_TSSI_DE_6:
		case FLASH_INFO_RF_2G_CCK_D_TSSI_DE_1 ... FLASH_INFO_RF_2G_CCK_D_TSSI_DE_6:

			len = 6;

			for(i=0; i<len; i++){
				_rtw_memset(temp, 0, sizeof(temp));
				strncpy(temp, data+i*2, 2);
				value = _atoi(temp, 16);
				rtw_flash_set_info(hal_info->flash, query_id, &value, query_size, query_offset+i);
			}

			break;

		case FLASH_INFO_RF_2G_BW40M_A_TSSI_DE_1 ... FLASH_INFO_RF_2G_BW40M_A_TSSI_DE_5:
		case FLASH_INFO_RF_2G_BW40M_B_TSSI_DE_1 ... FLASH_INFO_RF_2G_BW40M_B_TSSI_DE_5:
		case FLASH_INFO_RF_2G_BW40M_C_TSSI_DE_1 ... FLASH_INFO_RF_2G_BW40M_C_TSSI_DE_5:
		case FLASH_INFO_RF_2G_BW40M_D_TSSI_DE_1 ... FLASH_INFO_RF_2G_BW40M_D_TSSI_DE_5:

			len = 5;

			for(i=0; i<len; i++){
				_rtw_memset(temp, 0, sizeof(temp));
				strncpy(temp, data+i*2, 2);
				value = _atoi(temp, 16);
				rtw_flash_set_info(hal_info->flash, query_id, &value, query_size, query_offset+i);
			}

			break;

		case FLASH_INFO_RF_5G_BW40M_A_TSSI_DE_1 ... FLASH_INFO_RF_5G_BW40M_A_TSSI_DE_14:
		case FLASH_INFO_RF_5G_BW40M_B_TSSI_DE_1 ... FLASH_INFO_RF_5G_BW40M_B_TSSI_DE_14:
		case FLASH_INFO_RF_5G_BW40M_C_TSSI_DE_1 ... FLASH_INFO_RF_5G_BW40M_C_TSSI_DE_14:
		case FLASH_INFO_RF_5G_BW40M_D_TSSI_DE_1 ... FLASH_INFO_RF_5G_BW40M_D_TSSI_DE_14:

			len=14;

			for(i=0; i<len; i++){
				_rtw_memset(temp, 0, sizeof(temp));
				strncpy(temp, data+i*2, 2);
				value = _atoi(temp, 16);
				rtw_flash_set_info(hal_info->flash, query_id, &value, query_size, query_offset+i);
			}

			break;

		/* RF 2G CCK TSSI SLOPE K */
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_6:
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_6:
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_6:
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_6:
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_6:
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_6:
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_6:
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_6:
			if (strlen(data) != 12)
				PHL_ERR("data length error !!\n");
			else
				rtw_phl_flash_set_tssi_slp_info(hal_info->flash, query_id, data, TSSI_2G_CCK_SIZE);
			break;

		/* RF 2G TSSI SLOPE K */
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_5:
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_5:
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_5:
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_5:
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_5:
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_5:
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_5:
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_5:
			if (strlen(data) != 10)
				PHL_ERR("data length error !!\n");
			else
				rtw_phl_flash_set_tssi_slp_info(hal_info->flash, query_id, data, TSSI_2G_SIZE);
			break;

		/* RF 5G TSSI SLOPE K */
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_14:
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_14:
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_14:
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_14:
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_14:
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_14:
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_14:
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_14:
			if (strlen(data) != 28)
				PHL_ERR("data length error !!\n");
			else
				rtw_phl_flash_set_tssi_slp_info(hal_info->flash, query_id, data, TSSI_5G_SIZE);
			break;

		default:
			if (0 == strncmp(data, "0x", 2)){
				value = _atoi(data+2, 16);
			} else {
				value = _atoi(data, 10);
			}
			rtw_flash_set_info(hal_info->flash, query_id, &value, query_size, query_offset);

			break;
		}

	}

	else {
		PHL_PRINT("ic_name = %s, Not found query id: %d !! \n", ic_name, query_id);
	}


	return status;
}

void rtw_hal_flash_process(struct hal_info_t *hal_info)
{
	rtw_flash_process(hal_info->flash);
}

enum rtw_hal_status rtw_hal_flash_init(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal_info)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;

	status = rtw_flash_init(phl_com, hal_com, &(hal_info->flash));

	return status;
}

void rtw_hal_flash_deinit(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal_info)
{
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;

	//deinit_flash_id_table(hal_com, flash_id_table, FLASH_TABLE_SIZE); // deinit mapping table

	rtw_flash_deinit(hal_com, hal_info->flash);
}

enum rtw_flash_info rtw_hal_flash_lookup_id(struct flash_dict *table, int table_len, u8 *cmd, u32 *offset, u8 *size)
{

	return lookup_flash_id(table, table_len, cmd, offset, size);
}

enum rtw_hal_status rtw_hal_flash_read_map(struct flash_t *flash, u8 *map)
{

	memcpy(flash->shadow_map, map, flash->log_efuse_size);

	return RTW_HAL_STATUS_SUCCESS;
}

enum rtw_hal_status rtw_hal_flash_set_hw_cap(struct flash_t *flash)
{

	return flash_set_hw_cap(flash);
}

void rtw_hal_flash_set_by_offset(void *hal, u32 offset, u32 value)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct flash_t *flash_info = (struct flash_t *) hal_info->flash;

	rtw_flash_set_offset(flash_info, offset, value);
}

u8 rtw_hal_flash_get_by_offset(void *hal, u32 offset)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct flash_t *flash_info = (struct flash_t *) hal_info->flash;

	return rtw_flash_get_offset(flash_info, offset);

}

void rtw_hal_flash_dump(void *hal)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct flash_t *flash_info = (struct flash_t*)hal_info->flash;

	rtw_flash_dump(flash_info);
}

void rtw_hal_flash_set_para_path(void *hal, u8 *path)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct flash_t *flash_info = (struct flash_t*)hal_info->flash;


	rtw_flash_set_para_path(flash_info, path);
}

u8* rtw_hal_flash_get_para_path(void *hal)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct flash_t *flash_info = (struct flash_t*)hal_info->flash;


	return rtw_flash_get_para_path(flash_info);
}

#endif /* #ifndef CONFIG_PHL_DRV_HAS_NVM */
