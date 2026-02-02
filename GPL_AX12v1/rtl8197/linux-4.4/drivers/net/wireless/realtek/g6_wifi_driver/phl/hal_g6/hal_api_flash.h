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
#ifndef _HAL_API_FLASH_H_
#define _HAL_API_FLASH_H_

#ifndef CONFIG_PHL_DRV_HAS_NVM
#include "flash/hal_flash.h"

struct flash_dict{
	enum rtw_flash_info id;
	u8 str[32];
	u32 offset;
	u8 size;
};

/* flash exported API */
enum rtw_hal_status rtw_hal_flash_init(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal_info);
void rtw_hal_flash_deinit(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal_info);
void rtw_hal_flash_process(struct hal_info_t *hal_info);
enum rtw_hal_status
rtw_hal_flash_shadow_load(struct hal_info_t *hal_info, bool is_limit, u8 *map);
enum rtw_hal_status
rtw_hal_flash_shadow_read(struct hal_info_t *hal_info, u8 byte_count,
						  u16 offset, u32 *value, bool is_limit);
enum rtw_hal_status
rtw_hal_flash_shadow_write(struct hal_info_t *hal_info, u8 byte_count,
						   u16 offset, u32 value, bool is_limit);
enum rtw_hal_status
rtw_hal_flash_shadow_update(struct hal_info_t *hal_info, bool is_limit);
enum rtw_hal_status rtw_hal_flash_shadow2buf(struct hal_info_t *hal_info,
	u8 *pbuf, u16 buflen);
enum rtw_hal_status rtw_hal_flash_file_map_load(struct hal_info_t *hal_info,
	char *file_path, u8 is_limit);
enum rtw_hal_status rtw_hal_flash_file_mask_load(struct hal_info_t *hal_info,
	char *file_path, u8 is_limit);
enum rtw_hal_status rtw_hal_flash_get_usage(struct hal_info_t *hal_info,
	u32 *usage);
enum rtw_hal_status rtw_hal_flash_get_logical_size(struct hal_info_t *hal_info,
	u32 *size);
enum rtw_hal_status rtw_hal_flash_get_size(struct hal_info_t *hal_info,
	u32 *size);
enum rtw_hal_status rtw_hal_flash_get_avl(struct hal_info_t *hal_info,
	u32 *size);
enum rtw_hal_status rtw_hal_flash_get_offset_mask(struct hal_info_t *hal_info,
	u16 offset, u8 *mask);

enum rtw_flash_info rtw_hal_flash_lookup_id(struct flash_dict *table, int table_len, u8 *cmd, u32 *offset, u8 *size);

enum rtw_hal_status rtw_hal_flash_read_map(struct flash_t *flash, u8 *map);

enum rtw_hal_status rtw_hal_flash_set_hw_cap(struct flash_t *flash);

int _atoi(char *s, int base);

#endif /* #ifndef CONFIG_PHL_DRV_HAS_NVM */

#endif /* _HAL_API_flash_H_ */
