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

#ifndef _MAC_SS2F_INFO_DL_RU_H_
#define _MAC_SS2F_INFO_DL_RU_H_

/* dword0 */
#define SS2F_INFO_DL_RU_TX_LEN_SH		0
#define SS2F_INFO_DL_RU_TX_LEN_MSK		0x1fffff
#define SS2F_INFO_DL_RU_AC_SH		21
#define SS2F_INFO_DL_RU_AC_MSK		0x3
#define SS2F_INFO_DL_RU_NULL_WD		BIT(23)
#define SS2F_INFO_DL_RU_MACID_SH		24
#define SS2F_INFO_DL_RU_MACID_MSK		0xff

/* dword1 */
#define SS2F_INFO_DL_RU_RU_TX_CNT_SH		0
#define SS2F_INFO_DL_RU_RU_TX_CNT_MSK		0x7
#define SS2F_INFO_DL_RU_WDINFO_EN		BIT(6)
#define SS2F_INFO_DL_RU_AGG_EN		BIT(7)
#define SS2F_INFO_DL_RU_USERATE		BIT(8)
#define SS2F_INFO_DL_RU_BMC		BIT(9)
#define SS2F_INFO_DL_RU_TXPKTSIZE_256_SH		10
#define SS2F_INFO_DL_RU_TXPKTSIZE_256_MSK		0xf
#define SS2F_INFO_DL_RU_MSB_DL_QUOTA		BIT(14)

#pragma pack(1)
/**
 * @struct _ss2f_info_dl_ru_
 * @brief _ss2f_info_dl_ru_
 *
 * @var _ss2f_info_dl_ru_::tx_len
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::ac
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::null_wd
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::macid
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::ru_tx_cnt
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::rsvd0
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::wdinfo_en
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::agg_en
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::userate
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::bmc
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::txpktsize_256
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::msb_dl_quota
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::rsvd1
 * Please Place Description here.
 * @var _ss2f_info_dl_ru_::rsvd2
 * Please Place Description here.
 */
struct _ss2f_info_dl_ru_ {
	/* dword 0 */
	u32 tx_len:21;
	u32 ac:2;
	u32 null_wd:1;
	u32 macid:8;
	/* dword 1 */
	u32 ru_tx_cnt:3;
	u32 rsvd0:3;
	u32 wdinfo_en:1;
	u32 agg_en:1;
	u32 userate:1;
	u32 bmc:1;
	u32 txpktsize_256:4;
	u32 msb_dl_quota:1;
	u32 rsvd1:1;
	u32 rsvd2:16;
} ss2f_info_dl_ru, *pss2f_info_dl_ru;
#pragma pack()

#endif
