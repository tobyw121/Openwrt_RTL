/** @file */
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

#ifndef _MAC_SS2F_INFO_UL_RU_H_
#define _MAC_SS2F_INFO_UL_RU_H_

/* dword0 */
#define SS2F_INFO_UL_RU_BSR_LEN_SH		0
#define SS2F_INFO_UL_RU_BSR_LEN_MSK		0x7fff
#define SS2F_INFO_UL_RU_MSB_UL_QUOTA		BIT(15)
#define SS2F_INFO_UL_RU_BSR_AC_TYPE_SH		21
#define SS2F_INFO_UL_RU_BSR_AC_TYPE_MSK		0x3
#define SS2F_INFO_UL_RU_MACID_SH		24
#define SS2F_INFO_UL_RU_MACID_MSK		0xff

#pragma pack(1)
/**
 * @struct _ss2f_info_ul_ru_
 * @brief _ss2f_info_ul_ru_
 *
 * @var _ss2f_info_ul_ru_::bsr_len
 * Please Place Description here.
 * @var _ss2f_info_ul_ru_::msb_ul_quota
 * Please Place Description here.
 * @var _ss2f_info_ul_ru_::rsvd0
 * Please Place Description here.
 * @var _ss2f_info_ul_ru_::bsr_ac_type
 * Please Place Description here.
 * @var _ss2f_info_ul_ru_::rsvd1
 * Please Place Description here.
 * @var _ss2f_info_ul_ru_::macid
 * Please Place Description here.
 */
struct _ss2f_info_ul_ru_ {
	/* dword 0 */
	u32 bsr_len:15;
	u32 msb_ul_quota:1;
	u32 rsvd0:5;
	u32 bsr_ac_type:2;
	u32 rsvd1:1;
	u32 macid:8;
} ss2f_info_ul_ru, *pss2f_info_ul_ru;
#pragma pack()

#endif
