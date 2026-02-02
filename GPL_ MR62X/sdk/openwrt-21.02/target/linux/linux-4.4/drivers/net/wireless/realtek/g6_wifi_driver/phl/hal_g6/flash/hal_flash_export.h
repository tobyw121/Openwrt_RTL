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
#ifndef _HAL_FLASH_EXPORT_H_
#define _HAL_FLASH_EXPORT_H_

#ifndef CONFIG_PHL_DRV_HAS_NVM

/* flash exported API */
u32
rtw_flash_init(struct rtw_phl_com_t *phl_com, struct rtw_hal_com_t *hal_com,
			   void **flash);

void rtw_flash_deinit(struct rtw_hal_com_t *hal_com, void *flash);

enum rtw_hal_status
rtw_flash_get_info(void *flash, enum rtw_flash_info info_type, void *value,
				   u8 size);

enum rtw_hal_status
rtw_flash_set_info(void *flash, enum rtw_flash_info info_type, void *value,
				   u8 size, u32 offset);

void rtw_flash_process(void *flash);

enum rtw_hal_status
rtw_flash_shadow_load(void *flash, bool is_limit, u8 *map);

enum rtw_hal_status
rtw_flash_shadow_read(void *flash, u8 byte_count, u16 offset, u32 *value,
					  bool is_limit);

enum rtw_hal_status
rtw_flash_shadow_write(void *flash, u8 byte_count, u16 offset, u32 value,
					   bool is_limit);

enum rtw_hal_status
rtw_flash_shadow_update(void *flash, bool is_limit);

enum rtw_hal_status
rtw_flash_shadow2buf(void *flash, u8 *destbuf, u16 buflen);

enum rtw_hal_status
rtw_flash_file_map_load(void *flash, char *file_path, u8 is_limit);

enum rtw_hal_status
rtw_flash_file_mask_load(void *flash, char *file_path, u8 is_limit);

enum rtw_hal_status
rtw_flash_get_usage(void *flash, u32 *usage);

enum rtw_hal_status
rtw_flash_get_logical_size(void *flash, u32 *size, bool is_limited);

enum rtw_hal_status
rtw_flash_get_size(void *flash, u32 *size);

enum rtw_hal_status
rtw_flash_get_avl(void *flash, u32 *size);

enum rtw_hal_status
rtw_flash_get_offset_mask(void *flash, u16 offset, u8 *mask);

void deinit_flash_id_table(struct rtw_hal_com_t *hal_com, struct flash_dict *table, u16 len);

enum rtw_flash_info lookup_flash_id(struct flash_dict *table, int len, u8 *src, u32 *offset, u8 *size);

void rtw_flash_set_offset(void *flash, u32 offset, u32 value);

u8 rtw_flash_get_offset(void *flash, u32 offset);

void rtw_flash_dump(void *flash);

void rtw_flash_set_para_path(void *flash, u8 *path);

u8* rtw_flash_get_para_path(void *flash);

enum rtw_hal_status rtw_phl_flash_set_tssi_slp_info(void *flash, enum rtw_flash_info info_type, u8 *input_data, u8 len);
enum rtw_hal_status rtw_phl_flash_get_tssi_slp_info(struct rtw_hal_com_t *hal_com, void *flash, enum rtw_flash_info info_type, void *value, u8 size);

#endif /* #ifndef CONFIG_PHL_DRV_HAS_NVM */
#endif /* _HAL_flash_EXPORT_H_ */

