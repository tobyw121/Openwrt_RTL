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

#ifndef _MAC_SS2F_INFO_DL_MU_H_
#define _MAC_SS2F_INFO_DL_MU_H_

/* dword0 */
#define SS2F_INFO_DL_MU_TX_LEN_SH		0
#define SS2F_INFO_DL_MU_TX_LEN_MSK		0x1fffff
#define SS2F_INFO_DL_MU_AC_SH		21
#define SS2F_INFO_DL_MU_AC_MSK		0x3
#define SS2F_INFO_DL_MU_NULL_WD		BIT(23)
#define SS2F_INFO_DL_MU_MACID_SH		24
#define SS2F_INFO_DL_MU_MACID_MSK		0xff

/* dword1 */
#define SS2F_INFO_DL_MU_MU_TX_CNT_SH		0
#define SS2F_INFO_DL_MU_MU_TX_CNT_MSK		0x7
#define SS2F_INFO_DL_MU_MU_2ND_TX_CNT_SH		3
#define SS2F_INFO_DL_MU_MU_2ND_TX_CNT_MSK		0x7
#define SS2F_INFO_DL_MU_WDINFO_EN		BIT(6)
#define SS2F_INFO_DL_MU_AGG_EN		BIT(7)
#define SS2F_INFO_DL_MU_USERATE		BIT(8)
#define SS2F_INFO_DL_MU_BMC		BIT(9)
#define SS2F_INFO_DL_MU_TXPKTSIZE_256_SH		10
#define SS2F_INFO_DL_MU_TXPKTSIZE_256_MSK		0xf
#define SS2F_INFO_DL_MU_MSB_DL_QUOTA		BIT(14)

#pragma pack(1)
/**
 * @struct _ss2f_info_dl_mu_
 * @brief _ss2f_info_dl_mu_
 *
 * @var _ss2f_info_dl_mu_::tx_len
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::ac
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::null_wd
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::macid
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::mu_tx_cnt
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::mu_2nd_tx_cnt
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::wdinfo_en
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::agg_en
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::userate
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::bmc
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::txpktsize_256
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::msb_dl_quota
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::rsvd0
 * Please Place Description here.
 * @var _ss2f_info_dl_mu_::rsvd1
 * Please Place Description here.
 */
struct _ss2f_info_dl_mu_ {
	/* dword 0 */
	u32 tx_len:21;
	u32 ac:2;
	u32 null_wd:1;
	u32 macid:8;
	/* dword 1 */
	u32 mu_tx_cnt:3;
	u32 mu_2nd_tx_cnt:3;
	u32 wdinfo_en:1;
	u32 agg_en:1;
	u32 userate:1;
	u32 bmc:1;
	u32 txpktsize_256:4;
	u32 msb_dl_quota:1;
	u32 rsvd0:1;
	u32 rsvd1:16;
} ss2f_info_dl_mu, *pss2f_info_dl_mu;
#pragma pack()

#endif
