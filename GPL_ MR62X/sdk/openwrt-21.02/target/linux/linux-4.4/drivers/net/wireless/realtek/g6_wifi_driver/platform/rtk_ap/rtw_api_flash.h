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
#ifndef _RTW_API_FLASH_H_
#define _RTW_API_FLASH_H_

#include "flash/rtw_flash.h"
#include "flash/rtw_flash_export.h"

struct flash_dict{
	enum rtw_flash_info id;
	u8 str[32];
	u32 offset;
	u8 size;
};

/* flash exported API */
u32 rtw_hal_flash_init(struct dvobj_priv *dvobj);

void rtw_hal_flash_deinit(struct dvobj_priv *dvobj);

void rtw_hal_flash_process(struct dvobj_priv *dvobj);
enum rtw_hal_status
rtw_hal_flash_shadow_load(struct dvobj_priv *dvobj,
                          bool is_limit, u8 *map);
enum rtw_hal_status
rtw_hal_flash_shadow_read(struct dvobj_priv *dvobj, u8 byte_count,
                          u16 offset, u32 *value, bool is_limit);
enum rtw_hal_status
rtw_hal_flash_shadow_write(struct dvobj_priv *dvobj, u8 byte_count,
                           u16 offset, u32 value, bool is_limit);
enum rtw_hal_status
rtw_hal_flash_shadow_update(struct dvobj_priv *dvobj, bool is_limit);

enum rtw_hal_status
rtw_hal_flash_shadow2buf(struct dvobj_priv *dvobj,
                         u8 *pbuf, u16 buflen);
enum rtw_hal_status
rtw_hal_flash_file_map_load(struct dvobj_priv *dvobj,
                            char *file_path, u8 is_limit);
enum rtw_hal_status
rtw_hal_flash_file_mask_load(struct dvobj_priv *dvobj,
                             char *file_path, u8 is_limit);
enum rtw_hal_status
rtw_hal_flash_get_usage(struct dvobj_priv *dvobj,
                        u32 *usage);
u32
rtw_hal_flash_get_logical_size(struct dvobj_priv *dvobj, u32 *size);
u32
rtw_hal_flash_get_size(struct dvobj_priv *dvobj, u32 *size);
u32
rtw_hal_flash_get_avl(struct dvobj_priv *dvobj, u32 *size);
u32
rtw_hal_flash_get_offset_mask(struct dvobj_priv *dvobj,
                              u16 offset, u8 *mask);

enum rtw_flash_info
rtw_hal_flash_lookup_id(struct dvobj_priv *dvobj,
                        u8 *cmd, u32 *offset, u8 *size);

u32
rtw_hal_flash_read_map(struct flash_t *flash, u8 *map);

u32
rtw_hal_flash_set_hw_cap(struct flash_t *flash);

int _atoi_g6(char *s, int base);

#endif /* _RTW_API_flash_H_ */
