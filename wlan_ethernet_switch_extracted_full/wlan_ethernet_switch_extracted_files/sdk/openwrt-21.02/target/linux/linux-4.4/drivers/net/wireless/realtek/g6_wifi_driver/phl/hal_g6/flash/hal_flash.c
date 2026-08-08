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
The purpose of hal_flash.c

Provide flash operations.
a. flash init function
b. flash shadow map read/write/update
c. flash information query, map size/used bytes...

*/

#define _HAL_FLASH_C_

#ifndef CONFIG_RTW_DRV_HAS_NVM

#include "../hal_headers.h"
#include "hal_flash.h"
#include "hal_flash_export.h"

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

enum rtw_hal_status flash_set_hw_cap(struct flash_t *flash)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = flash->hal_com;
	struct phy_hw_cap_t *phy_hw_cap;

	u8 i = 0;
	u8 pkg_type = 0xFF;
	u8 rfe_type = 0xFF;
	u8 xcap = 0xFF;
	u8 domain = 0xFF;

	status = rtw_flash_get_info(flash, FLASH_INFO_RF_PKG_TYPE, &pkg_type,
								sizeof(pkg_type));

	if(status != RTW_HAL_STATUS_SUCCESS) {
		PHL_WARN("%s: Get pkg type fail! Status(%x)\n", __FUNCTION__, status);
	}

	status = rtw_flash_get_info(flash, FLASH_INFO_RF_RFE, &rfe_type,
								sizeof(rfe_type));

	if(status != RTW_HAL_STATUS_SUCCESS) {
		PHL_WARN("%s: Get rfe type fail! Status(%x)\n", __FUNCTION__, status);
	}

	status = rtw_flash_get_info(flash, FLASH_INFO_RF_XTAL, &xcap,
								sizeof(xcap));

	if(status != RTW_HAL_STATUS_SUCCESS) {
		PHL_WARN("%s: Get xcap type fail! Status(%x)\n", __FUNCTION__, status);
	}


	status = rtw_flash_get_info(flash, FLASH_INFO_RF_CHAN_PLAN, &domain,
						sizeof(domain));

	if(status != RTW_HAL_STATUS_SUCCESS) {
		PHL_WARN("%s: Get domain fail! Status(%x)\n", __FUNCTION__, status);
	}
#ifdef WKARD_PON_PLATFORM
	rfe_type = 50;
#endif
	hal_com->dev_hw_cap.pkg_type = pkg_type;
	hal_com->dev_hw_cap.rfe_type = rfe_type;
	hal_com->dev_hw_cap.xcap = xcap;
	hal_com->dev_hw_cap.domain = domain;
	if (hal_com->chip_id == CHIP_WIFI6_8852A) {
		rtw_hal_mac_set_xcap(hal_com, 0, hal_com->dev_hw_cap.xcap);
		rtw_hal_mac_set_xcap(hal_com, 1, hal_com->dev_hw_cap.xcap);
	}

#ifdef WKARD_DBCC

	/*if(hal_com->dev_hw_cap.rfe_type == 0xFF){
		DBGP("flash not assign RFE, call phl_set_rfe_type\n");
		rtw_phl_set_rfe_type(flash->phl_com->phl_priv, false);
	} else{
		DBGP("flash assign RFE = %d\n", hal_com->dev_hw_cap.rfe_type);
		rtw_phl_set_rfe_type(flash->phl_com->phl_priv, true);
	}*/

#ifdef CONFIG_SHARE_XSTAL
	rtw_phl_set_share_xstal(flash->phl_com->phl_priv, true);
#else
	rtw_phl_set_share_xstal(flash->phl_com->phl_priv, false);
#endif /*CONFIG_SHARE_XSTAL*/

#endif /*WKARD_DBCC*/

	return status;
}

enum rtw_hal_status rtw_flash_shadow_load(void *flash, bool is_limit, u8 *map)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_SUCCESS;
	struct flash_t *flash_info = flash;

	status = rtw_hal_flash_read_map(flash_info, map);

	return status;
}

enum rtw_hal_status rtw_flash_shadow_update(void *flash, bool is_limit)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_EFUSE_PG_FAIL;
	enum rtw_hal_status reload_status = RTW_HAL_STATUS_FAILURE;
	struct flash_t *flash_info = flash;
	u32 map_size = 0, mask_size = 0;

	if(TEST_STATUS_FLAG(flash_info->status, FLASH_STATUS_PROCESS) == false) {
		PHL_WARN("%s: flash map not load yet!\n", __FUNCTION__);
		status = RTW_HAL_STATUS_EFUSE_UNINIT;
		goto exit;
	}

	/* Load flash mask file before PG */
	if(TEST_STATUS_FLAG(flash_info->status, FLASH_STATUS_MASK_FILE_LOADED) == false) {
		PHL_WARN("%s: flash mask not load yet!\n", __FUNCTION__);
		status = RTW_HAL_STATUS_EFUSE_PG_FAIL;
		goto exit;
	}


exit:
	return status;
}

enum rtw_hal_status
rtw_flash_shadow_read(void *flash, u8 byte_count, u16 offset, u32 *value,
					  bool is_limit)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;
	struct flash_t *flash_info = flash;
	u32	flash_size = 0;

	if(is_limit)
		flash_size = flash_info->limit_efuse_size;
	else
		flash_size = flash_info->log_efuse_size;

	if((u32)(offset+byte_count) > flash_size) {
		PHL_WARN("%s: Invalid offset!\n", __FUNCTION__);
		status = RTW_HAL_STATUS_EFUSE_IVALID_OFFSET;
		goto exit;
	}

	if (byte_count == 1)
		flash_shadow_read_one_byte(flash_info, offset, (u8 *)value);
	else if (byte_count == 2)
		flash_shadow_read_two_byte(flash_info, offset, (u16 *)value);
	else if (byte_count == 4)
		flash_shadow_read_four_byte(flash_info, offset, (u32 *)value);

	status = RTW_HAL_STATUS_SUCCESS;
exit:
	return status;
}

enum rtw_hal_status
rtw_flash_shadow_write(void *flash, u8 byte_count, u16 offset, u32 value,
					   bool is_limit)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;
	struct flash_t *flash_info = flash;
	u32	flash_size = 0;

	if(is_limit)
		flash_size = flash_info->limit_efuse_size;
	else
		flash_size = flash_info->log_efuse_size;

	if((u32)(offset+byte_count) > flash_size) {
		PHL_WARN("%s: Invalid offset!\n", __FUNCTION__);
		status = RTW_HAL_STATUS_EFUSE_IVALID_OFFSET;
		goto exit;
	}

	if (byte_count == 1)
		flash_shadow_write_one_byte(flash_info, offset, (u8)value);
	else if (byte_count == 2)
		flash_shadow_write_two_byte(flash_info, offset, (u16)value);
	else if (byte_count == 4)
		flash_shadow_write_four_byte(flash_info, offset, (u32)value);

	status = RTW_HAL_STATUS_SUCCESS;
exit:
	return status;
}

enum rtw_hal_status rtw_flash_get_info(void *flash,
									   enum rtw_flash_info info_type,
									   void *value,
									   u8 size)
{
	struct flash_t *flash_info = flash;
	struct rtw_hal_com_t *hal_com = flash_info->hal_com;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	if(info_type <= FLASH_INFO_MAC_MAX)
		hal_status = rtw_hal_mac_get_efuse_info(hal_com,
												flash_info->shadow_map,
												info_type,
												value,
												size,
												1);
	else if (info_type <= FLASH_INFO_BB_MAX)
		hal_status = rtw_hal_bb_get_efuse_info(hal_com,
											   flash_info->shadow_map,
											   info_type,
											   value,
											   size,
											   1);
	else if (info_type <= FLASH_INFO_RF_MAX)
		hal_status = rtw_hal_rf_get_efuse_info(hal_com,
											   flash_info->shadow_map,
											   info_type,
											   value,
											   size,
											   1);
	else if (info_type <= FLASH_INFO_BTCOEX_MAX)
		hal_status = rtw_hal_btc_get_efuse_info(hal_com,
											flash_info->shadow_map,
											info_type,
											value,
											size,
											1);
	else if (info_type == FLASH_INFO_COSTUM_PARA_PATH)
		;
	else if (info_type <= FLASH_INFO_RF_TSSI_MAX)
		hal_status = rtw_phl_flash_get_tssi_slp_info(hal_com,
					flash_info,
					info_type,
					value,
					size);

	return hal_status;
}

enum rtw_hal_status rtw_flash_set_info(void *flash,
									   enum rtw_flash_info info_type,
									   void *value,
									   u8 size, u32 offset)
{
	struct flash_t *flash_info = flash;
	struct rtw_hal_com_t *hal_com = flash_info->hal_com;
	struct hal_info_t *hal_info = hal_com->hal_priv;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

	hal_mem_cpy(hal_com, flash_info->shadow_map+offset, value, size);

	return hal_status;
}

void rtw_flash_process(void *flash)
{
	struct flash_t *flash_info = (struct flash_t *)flash;
	flash_set_hw_cap(flash_info);
}

u32 rtw_flash_init(struct rtw_phl_com_t *phl_com,
				   struct rtw_hal_com_t *hal_com, void **flash)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct flash_t *flash_info = NULL;

	flash_info = _os_mem_alloc(hal_com->drv_priv, sizeof(struct flash_t));
	if(flash_info == NULL) {
		hal_status = RTW_HAL_STATUS_RESOURCE;
		goto error_flash_init;
	}
	flash_info->is_map_valid = 1;

	/* Allocate shadow map memory */
	hal_status = rtw_hal_mac_get_log_efuse_size(hal_com,
												&(flash_info->log_efuse_size),
												false);

	if(hal_status != RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s Get full logical flash map size fail!\n",__FUNCTION__);
		goto error_flash_shadow_init;
	}

	flash_info->shadow_map = _os_mem_alloc(hal_com->drv_priv,
										   flash_info->log_efuse_size);

	if(flash_info->shadow_map == NULL) {
		hal_status = RTW_HAL_STATUS_RESOURCE;
		PHL_ERR("%s Allocate shadow efuse map fail!\n", __FUNCTION__);
		goto error_flash_shadow_init;
	}

	hal_status = rtw_hal_mac_get_log_efuse_size(hal_com,
												&(flash_info->limit_efuse_size),
												true);

	if(hal_status != RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s Get limited logical efuse map size fail!\n", __FUNCTION__);
		goto error_flash_shadow_init;
	}

	/* Allocate mask memory */
	hal_status = rtw_hal_mac_get_efuse_mask_size(hal_com,
												 &(flash_info->mask_size),
												 false);

	if(hal_status != RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s Get full efuse mask size fail!\n", __FUNCTION__);
		goto error_flash_mask_init;
	}

	flash_info->mask = _os_mem_alloc(hal_com->drv_priv,
									 flash_info->mask_size);

	if(flash_info->mask == NULL) {
		hal_status = RTW_HAL_STATUS_RESOURCE;
		PHL_ERR("%s Allocate efuse mask fail!\n", __FUNCTION__);
		goto error_flash_mask_init;
	}

	hal_status = rtw_hal_mac_get_efuse_mask_size(hal_com,
												 &(flash_info->limit_mask_size),
												 true);

	if(hal_status != RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s Get limited efuse mask size fail!\n", __FUNCTION__);
		goto error_flash_mask_init;
	}

	/* There will be a MAC API to get version field length */
	flash_info->version_len = MAX_EFUSE_FILE_VERSION_LENGTH;

	flash_info->map_version = _os_mem_alloc(hal_com->drv_priv,
											flash_info->version_len);

	if(flash_info->map_version == NULL) {
		hal_status = RTW_HAL_STATUS_RESOURCE;
		goto error_map_version_init;
	}

	flash_info->mask_version = _os_mem_alloc(hal_com->drv_priv,
											 flash_info->version_len);
	if(flash_info->mask_version == NULL) {
		hal_status = RTW_HAL_STATUS_RESOURCE;
		goto error_mask_version_init;
	}

	flash_info->phl_com = phl_com;
	flash_info->hal_com = hal_com;

	*flash = flash_info;

	/*default path when insmod*/
	rtw_flash_set_para_path(flash_info, HAL_FILE_CONFIG_PATH);

	hal_status = RTW_HAL_STATUS_SUCCESS;
	return hal_status;

error_mask_version_init:
	_os_mem_free(hal_com->drv_priv, flash_info->map_version,
				 flash_info->version_len);

error_map_version_init:
	_os_mem_free(hal_com->drv_priv, flash_info->mask,
				 flash_info->mask_size);

error_flash_mask_init:
	_os_mem_free(hal_com->drv_priv, flash_info->shadow_map,
				 flash_info->log_efuse_size);

error_flash_shadow_init:
	_os_mem_free(hal_com->drv_priv, flash_info, sizeof(struct flash_t));

error_flash_init:
	return hal_status;
}

void rtw_flash_deinit(struct rtw_hal_com_t *hal_com, void *flash)
{
	struct flash_t *flash_info = flash;

	if (flash_info) {
		if(flash_info->mask) {
			_os_mem_free(hal_com->drv_priv, flash_info->mask, flash_info->mask_size);
			flash_info->mask = NULL;
		}

		if(flash_info->map_version) {
			_os_mem_free(hal_com->drv_priv, flash_info->map_version, flash_info->version_len);
			flash_info->map_version = NULL;
		}

		if(flash_info->mask_version) {
			 _os_mem_free(hal_com->drv_priv, flash_info->mask_version, flash_info->version_len);
			 flash_info->mask_version = NULL;
		}

		if(flash_info->shadow_map) {
			_os_mem_free(hal_com->drv_priv, flash_info->shadow_map, flash_info->log_efuse_size);
			flash_info->shadow_map = NULL;
		}

		_os_mem_free(hal_com->drv_priv, flash_info, sizeof(struct flash_t));
		flash_info = NULL;
	}
}

void deinit_flash_id_table(struct rtw_hal_com_t *hal_com, struct flash_dict *table, u16 len){

	PHL_WARN("%s\n", __FUNCTION__);
	_os_mem_free(hal_com->drv_priv, table, len*sizeof(struct flash_dict));

}

enum rtw_flash_info lookup_flash_id(struct flash_dict *table, int len, u8 *cmd, u32 *offset, u8 *size){

	enum rtw_flash_info ret = FLASH_INFO_ID_ERROR;
	int i;

	for(i=0; i<len; i++){
		if(0 == _os_strcmp(table[i].str, cmd)){
			*size = table[i].size;
			*offset = table[i].offset;
			return table[i].id;
		}
	}

	PHL_ERR("%s is not defined in flash_dict table!\n", cmd);
	return ret;
}

void rtw_flash_set_offset(void *flash, u32 offset, u32 value)
{
	struct flash_t *flash_info = flash;
	flash_info->shadow_map[offset] = value;

	PHL_WARN("shadowmap[%d] = %d\n", offset, flash_info->shadow_map[offset]);
}

u8 rtw_flash_get_offset(void *flash, u32 offset)
{
	struct flash_t *flash_info = flash;
	return flash_info->shadow_map[offset];
}

void rtw_flash_dump(void *flash)
{
	struct flash_t *flash_info = flash;
	int i;

	PHL_WARN("flash_info size = %d\n", flash_info->log_efuse_size);
	for (i=0; i<flash_info->log_efuse_size; i++){

		if(i % 20 == 0){
			printk("\n");
		}
		printk("%02x,", flash_info->shadow_map[i]);
	}

	printk("\n");
}

void rtw_flash_set_para_path(void *flash, u8 *path)
{
	struct flash_t *flash_info = flash;
	_os_strncpy(flash_info->para_path, path, sizeof(flash_info->para_path));
}

u8* rtw_flash_get_para_path(void *flash)
{
	struct flash_t *flash_info = flash;
	return flash_info->para_path;
}

enum rtw_hal_status rtw_phl_flash_set_tssi_slp_info(void *flash, enum rtw_flash_info info_type, u8 *input_data, u8 len)
{
	struct flash_t *flash_info = flash;
	u8 i = 0, value = 0;
	u8 temp[3] = {0};

	for (i = 0; i < len; i++) {
		_rtw_memset(temp, 0, sizeof(temp));
		_os_strncpy(temp, input_data+i*2, 2);
		value = _atoi_g6(temp, 16);

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
	return RTW_HAL_STATUS_SUCCESS;
}

enum rtw_hal_status rtw_phl_flash_get_tssi_slp_info(struct rtw_hal_com_t *hal_com, void *flash, enum rtw_flash_info info_type, void *value, u8 size)
{
	struct flash_t *flash_info = flash;
	u8 offset = 0;

	switch (info_type) {
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_A[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_B[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_C[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cck_gain_diff_D[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_A_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_gain_diff_A[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_B_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_gain_diff_B[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_C_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_gain_diff_C[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_GAIN_DIFF_D_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_gain_diff_D[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_A_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_5g_gain_diff_A[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_B_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_5g_gain_diff_B[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_C_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_5g_gain_diff_C[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_1 ... FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_GAIN_DIFF_D_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_5g_gain_diff_D[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_A[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_B[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_C[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_6:
			offset = info_type - FLASH_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_1;
			if (offset < TSSI_2G_CCK_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cck_cw_diff_D[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_CW_DIFF_A_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cw_diff_A[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_CW_DIFF_B_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cw_diff_B[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_CW_DIFF_C_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cw_diff_C[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_5:
			offset = info_type - FLASH_INFO_RF_2G_TSSI_CW_DIFF_D_1;
			if (offset < TSSI_2G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_2g_cw_diff_D[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_CW_DIFF_A_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_5g_cw_diff_A[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_CW_DIFF_B_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_5g_cw_diff_B[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_CW_DIFF_C_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_5g_cw_diff_C[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		case FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_1 ... FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_14:
			offset = info_type - FLASH_INFO_RF_5G_TSSI_CW_DIFF_D_1;
			if (offset < TSSI_5G_SIZE)
				_os_mem_cpy(hal_com->drv_priv, value, &(flash_info->tssi_slp_info.tssi_5g_cw_diff_D[offset]), size);
			else
				PHL_ERR ("info_type error, info_type=%d\n", info_type);
			break;
		default:
			break;
	}
	return RTW_HAL_STATUS_SUCCESS;
}

#endif
