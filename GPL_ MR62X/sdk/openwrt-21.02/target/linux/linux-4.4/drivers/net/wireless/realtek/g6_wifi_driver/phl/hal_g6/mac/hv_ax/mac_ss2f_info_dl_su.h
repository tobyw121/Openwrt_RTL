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

#ifndef _MAC_SS2F_INFO_DL_SU_H_
#define _MAC_SS2F_INFO_DL_SU_H_

/* dword0 */
#define SS2F_INFO_DL_SU_TX_LEN_SH		0
#define SS2F_INFO_DL_SU_TX_LEN_MSK		0x1fffff
#define SS2F_INFO_DL_SU_BSR_AC_TYPE_SH		21
#define SS2F_INFO_DL_SU_BSR_AC_TYPE_MSK		0x3
#define SS2F_INFO_DL_SU_NULL_WD		BIT(23)
#define SS2F_INFO_DL_SU_MACID_SH		24
#define SS2F_INFO_DL_SU_MACID_MSK		0xff

/* dword1 */
#define SS2F_INFO_DL_SU_SU_TX_CNT_SH		0
#define SS2F_INFO_DL_SU_SU_TX_CNT_MSK		0x3f
#define SS2F_INFO_DL_SU_WDINFO_EN		BIT(6)
#define SS2F_INFO_DL_SU_AGG_EN		BIT(7)
#define SS2F_INFO_DL_SU_USERATE		BIT(8)
#define SS2F_INFO_DL_SU_BMC		BIT(9)
#define SS2F_INFO_DL_SU_TXPKTSIZE_256_SH		10
#define SS2F_INFO_DL_SU_TXPKTSIZE_256_MSK		0xf
#define SS2F_INFO_DL_SU_MSB_DL_QUOTA		BIT(14)
#define SS2F_INFO_DL_SU_MSB_UL_QUOTA		BIT(15)
#define SS2F_INFO_DL_SU_BSR_LEN_SH		16
#define SS2F_INFO_DL_SU_BSR_LEN_MSK		0x7fff

#pragma pack(1)
/**
 * @struct _ss2f_info_dl_su_
 * @brief _ss2f_info_dl_su_
 *
 * @var _ss2f_info_dl_su_::tx_len
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::bsr_ac_type
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::null_wd
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::macid
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::su_tx_cnt
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::wdinfo_en
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::agg_en
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::userate
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::bmc
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::txpktsize_256
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::msb_dl_quota
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::msb_ul_quota
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::bsr_len
 * Please Place Description here.
 * @var _ss2f_info_dl_su_::rsvd0
 * Please Place Description here.
 */
struct _ss2f_info_dl_su_ {
	/* dword 0 */
	u32 tx_len:21;
	u32 bsr_ac_type:2;
	u32 null_wd:1;
	u32 macid:8;
	/* dword 1 */
	u32 su_tx_cnt:6;
	u32 wdinfo_en:1;
	u32 agg_en:1;
	u32 userate:1;
	u32 bmc:1;
	u32 txpktsize_256:4;
	u32 msb_dl_quota:1;
	u32 msb_ul_quota:1;
	u32 bsr_len:15;
	u32 rsvd0:1;
} ss2f_info_dl_su, *pss2f_info_dl_su;
#pragma pack()

#endif
