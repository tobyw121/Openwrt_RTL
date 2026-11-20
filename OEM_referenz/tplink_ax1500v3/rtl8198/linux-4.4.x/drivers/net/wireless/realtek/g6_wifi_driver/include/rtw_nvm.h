/******************************************************************************
 *
 * Copyright(c) 2021 Realtek Corporation.
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
#ifndef __RTW_NVM_H_
#define __RTW_NVM_H_

#ifdef CONFIG_RTW_DRV_HAS_NVM
u32 rtw_nvm_init(void *d);
u32 rtw_nvm_efuse_init(void *d);
void rtw_nvm_deinit(void *d);
u32 rtw_nvm_get_info(void *d, u32 info_type, void *value, u8 size);
u32 rtw_nvm_set_info(void *d, u8 *cmd, u8 *data);
u8 rtw_nvm_get_by_offset(void *d, u32 offset);
void rtw_nvm_set_by_offset(void *d, u32 offset, u32 value);
void rtw_nvm_dump(void *d);
void rtw_get_fem_name(void *d, u8* fem_name, u8 name_len);
const char *rtw_nvm_get_para_path(void *dev);
void rtw_nvm_fill_full_para_path(void *dev, char *para_path, u32 path_size);
void rtw_nvm_get_sub_dir_name(void *phl, u8 rfe_type, char *sub_dir, u32 rfe_size);
void rtw_nvm_get_ic_name(void *d, char *ic_name, u32 ic_name_size);
void rtw_hal_flash_process(struct dvobj_priv *dvobj);
#else /* CONFIG_RTW_DRV_HAS_NVM */

#define rtw_get_fem_name(dvobj, fem_name, name_len) \
        rtw_gen_fem_name((dvobj)->phl, fem_name)

#define rtw_nvm_get_by_offset(dvobj, offset) \
        rtw_phl_flash_get_by_offset((dvobj)->phl, offset)

#define rtw_nvm_set_by_offset(dvobj, offset, value) \
	rtw_phl_flash_set_by_offset((dvobj)->phl, offset, value)

#define rtw_nvm_dump(dvobj) \
	rtw_phl_flash_dump((dvobj)->phl)

#define rtw_nvm_fill_full_para_path(dvobj, para_path, para_max_len)

#define rtw_nvm_set_info(dvobj, key, value_str) \
	rtw_phl_flash_set_info(dvobj->phl, para_path, para_max_len)

#endif /* CONFIG_RTW_DRV_HAS_NVM */

#endif /* __RTW_NVM_H_ */
