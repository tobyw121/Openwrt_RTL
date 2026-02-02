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

#ifndef _MAC_SS2F_INFO_UL_HDR_H_
#define _MAC_SS2F_INFO_UL_HDR_H_

/* dword0 */
#define SS2F_INFO_UL_HDR_PKT_LEN_SH		0
#define SS2F_INFO_UL_HDR_PKT_LEN_MSK		0x3fff
#define SS2F_INFO_UL_HDR_SHIFT_SH		14
#define SS2F_INFO_UL_HDR_SHIFT_MSK		0x3
#define SS2F_INFO_UL_HDR_REASON_OF_RPT_SH		16
#define SS2F_INFO_UL_HDR_REASON_OF_RPT_MSK		0x7
#define SS2F_INFO_UL_HDR_PRI_STA_VLD		BIT(19)
#define SS2F_INFO_UL_HDR_SU_NUM_SH		20
#define SS2F_INFO_UL_HDR_SU_NUM_MSK		0xf
#define SS2F_INFO_UL_HDR_RPKT_TYPE_SH		24
#define SS2F_INFO_UL_HDR_RPKT_TYPE_MSK		0xf
#define SS2F_INFO_UL_HDR_DRV_INFO_SIZE_SH		28
#define SS2F_INFO_UL_HDR_DRV_INFO_SIZE_MSK		0x7
#define SS2F_INFO_UL_HDR_LONG_RXD		BIT(31)

/* dword1 */
#define SS2F_INFO_UL_HDR_GID_VLD_TBL_SH		0
#define SS2F_INFO_UL_HDR_GID_VLD_TBL_MSK		0x7fff
#define SS2F_INFO_UL_HDR_RU_TO		BIT(15)
#define SS2F_INFO_UL_HDR_WMM_SH		16
#define SS2F_INFO_UL_HDR_WMM_MSK		0x3
#define SS2F_INFO_UL_HDR_UL_SH		20
#define SS2F_INFO_UL_HDR_UL_MSK		0x3
#define SS2F_INFO_UL_HDR_SS_SEL_MODE_SH		22
#define SS2F_INFO_UL_HDR_SS_SEL_MODE_MSK		0x3
#define SS2F_INFO_UL_HDR_TWT_GRP_SH		24
#define SS2F_INFO_UL_HDR_TWT_GRP_MSK		0xf
#define SS2F_INFO_UL_HDR_RU_NUM_SH		28
#define SS2F_INFO_UL_HDR_RU_NUM_MSK		0xf

/* dword2 */

/* dword3 */

#pragma pack(1)
/**
 * @struct _ss2f_info_ul_hdr_
 * @brief _ss2f_info_ul_hdr_
 *
 * @var _ss2f_info_ul_hdr_::pkt_len
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::shift
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::reason_of_rpt
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::pri_sta_vld
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::su_num
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::rpkt_type
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::drv_info_size
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::long_rxd
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::gid_vld_tbl
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::ru_to
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::wmm
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::rsvd0
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::ul
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::ss_sel_mode
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::twt_grp
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::ru_num
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::rsvd1
 * Please Place Description here.
 * @var _ss2f_info_ul_hdr_::rsvd2
 * Please Place Description here.
 */
struct _ss2f_info_ul_hdr_ {
	/* dword 0 */
	u32 pkt_len:14;
	u32 shift:2;
	u32 reason_of_rpt:3;
	u32 pri_sta_vld:1;
	u32 su_num:4;
	u32 rpkt_type:4;
	u32 drv_info_size:3;
	u32 long_rxd:1;
	/* dword 1 */
	u32 gid_vld_tbl:15;
	u32 ru_to:1;
	u32 wmm:2;
	u32 rsvd0:2;
	u32 ul:2;
	u32 ss_sel_mode:2;
	u32 twt_grp:4;
	u32 ru_num:4;
	/* dword 2 */
	u32 rsvd1:32;
	/* dword 3 */
	u32 rsvd2:32;
} ss2f_info_ul_hdr, *pss2f_info_ul_hdr;
#pragma pack()

#endif
