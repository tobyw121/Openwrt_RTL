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

#ifndef _MAC_SS2F_INFO_DL_PRI_H_
#define _MAC_SS2F_INFO_DL_PRI_H_

/* dword0 */
#define SS2F_INFO_DL_PRI_TX_LEN_SH		0
#define SS2F_INFO_DL_PRI_TX_LEN_MSK		0x1fffff
#define SS2F_INFO_DL_PRI_BSR_AC_TYPE_SH		21
#define SS2F_INFO_DL_PRI_BSR_AC_TYPE_MSK		0x3
#define SS2F_INFO_DL_PRI_NULL_WD		BIT(23)
#define SS2F_INFO_DL_PRI_MACID_SH		24
#define SS2F_INFO_DL_PRI_MACID_MSK		0xff

/* dword1 */
#define SS2F_INFO_DL_PRI_SU_TX_CNT_SH		0
#define SS2F_INFO_DL_PRI_SU_TX_CNT_MSK		0x3f
#define SS2F_INFO_DL_PRI_WDINFO_EN		BIT(6)
#define SS2F_INFO_DL_PRI_AGG_EN		BIT(7)
#define SS2F_INFO_DL_PRI_USERATE		BIT(8)
#define SS2F_INFO_DL_PRI_BMC		BIT(9)
#define SS2F_INFO_DL_PRI_TXPKTSIZE_256_SH		10
#define SS2F_INFO_DL_PRI_TXPKTSIZE_256_MSK		0xf
#define SS2F_INFO_DL_PRI_MSB_DL_QUOTA		BIT(14)
#define SS2F_INFO_DL_PRI_MSB_UL_QUOTA		BIT(15)
#define SS2F_INFO_DL_PRI_MU_TX_CNT_SH		16
#define SS2F_INFO_DL_PRI_MU_TX_CNT_MSK		0x7
#define SS2F_INFO_DL_PRI_MU_2ND_TX_CNT_SH		19
#define SS2F_INFO_DL_PRI_MU_2ND_TX_CNT_MSK		0x7
#define SS2F_INFO_DL_PRI_RU_TX_CNT_SH		22
#define SS2F_INFO_DL_PRI_RU_TX_CNT_MSK		0x7
#define SS2F_INFO_DL_PRI_WMM_VO_VLD		BIT(25)
#define SS2F_INFO_DL_PRI_WMM_VI_VLD		BIT(26)
#define SS2F_INFO_DL_PRI_WMM_BE_VLD		BIT(27)
#define SS2F_INFO_DL_PRI_WMM_BK_VLD		BIT(28)

/* dword2 */
#define SS2F_INFO_DL_PRI_BSR_LEN_SH		0
#define SS2F_INFO_DL_PRI_BSR_LEN_MSK		0x7fff

#pragma pack(1)
/**
 * @struct _ss2f_info_dl_pri_
 * @brief _ss2f_info_dl_pri_
 *
 * @var _ss2f_info_dl_pri_::tx_len
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::bsr_ac_type
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::null_wd
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::macid
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::su_tx_cnt
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::wdinfo_en
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::agg_en
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::userate
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::bmc
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::txpktsize_256
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::msb_dl_quota
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::msb_ul_quota
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::mu_tx_cnt
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::mu_2nd_tx_cnt
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::ru_tx_cnt
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::wmm_vo_vld
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::wmm_vi_vld
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::wmm_be_vld
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::wmm_bk_vld
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::rsvd0
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::bsr_len
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::rsvd1
 * Please Place Description here.
 * @var _ss2f_info_dl_pri_::rsvd2
 * Please Place Description here.
 */
struct _ss2f_info_dl_pri_ {
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
	u32 mu_tx_cnt:3;
	u32 mu_2nd_tx_cnt:3;
	u32 ru_tx_cnt:3;
	u32 wmm_vo_vld:1;
	u32 wmm_vi_vld:1;
	u32 wmm_be_vld:1;
	u32 wmm_bk_vld:1;
	u32 rsvd0:3;
	/* dword 2 */
	u32 bsr_len:15;
	u32 rsvd1:1;
	u32 rsvd2:16;
} ss2f_info_dl_pri, *pss2f_info_dl_pri;
#pragma pack()

#endif
