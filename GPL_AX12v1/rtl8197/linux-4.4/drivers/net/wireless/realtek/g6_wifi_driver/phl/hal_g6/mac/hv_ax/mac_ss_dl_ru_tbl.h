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

#ifndef _MAC_SS_DL_RU_TBL_H_
#define _MAC_SS_DL_RU_TBL_H_

/* dword0 */
#define SS_DL_RU_TBL_AC_DIS7_SH		0
#define SS_DL_RU_TBL_AC_DIS7_MSK		0xf
#define SS_DL_RU_TBL_MACID7_SH		4
#define SS_DL_RU_TBL_MACID7_MSK		0xff
#define SS_DL_RU_TBL_AC_DIS6_SH		12
#define SS_DL_RU_TBL_AC_DIS6_MSK		0xf
#define SS_DL_RU_TBL_MACID6_SH		16
#define SS_DL_RU_TBL_MACID6_MSK		0xff
#define SS_DL_RU_TBL_AC_DIS5_SH		24
#define SS_DL_RU_TBL_AC_DIS5_MSK		0xf
#define SS_DL_RU_TBL_MACID5_L_SH		28
#define SS_DL_RU_TBL_MACID5_L_MSK		0xf

/* dword1 */
#define SS_DL_RU_TBL_MACID5_H_SH		0
#define SS_DL_RU_TBL_MACID5_H_MSK		0xf
#define SS_DL_RU_TBL_AC_DIS4_SH		4
#define SS_DL_RU_TBL_AC_DIS4_MSK		0xf
#define SS_DL_RU_TBL_MACID4_SH		8
#define SS_DL_RU_TBL_MACID4_MSK		0xff
#define SS_DL_RU_TBL_AC_DIS3_SH		16
#define SS_DL_RU_TBL_AC_DIS3_MSK		0xf
#define SS_DL_RU_TBL_MACID3_SH		20
#define SS_DL_RU_TBL_MACID3_MSK		0xff
#define SS_DL_RU_TBL_AC_DIS2_SH		28
#define SS_DL_RU_TBL_AC_DIS2_MSK		0xf

/* dword2 */
#define SS_DL_RU_TBL_MACID2_SH		0
#define SS_DL_RU_TBL_MACID2_MSK		0xff
#define SS_DL_RU_TBL_AC_DIS1_SH		8
#define SS_DL_RU_TBL_AC_DIS1_MSK		0xf
#define SS_DL_RU_TBL_MACID1_SH		12
#define SS_DL_RU_TBL_MACID1_MSK		0xff
#define SS_DL_RU_TBL_AC_DIS0_SH		20
#define SS_DL_RU_TBL_AC_DIS0_MSK		0xf
#define SS_DL_RU_TBL_MACID0_SH		24
#define SS_DL_RU_TBL_MACID0_MSK		0xff

#pragma pack(1)
/**
 * @struct _ss_dl_ru_tbl_
 * @brief _ss_dl_ru_tbl_
 *
 * @var _ss_dl_ru_tbl_::ac_dis7
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::macid7
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::ac_dis6
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::macid6
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::ac_dis5
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::macid5_l
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::macid5_h
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::ac_dis4
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::macid4
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::ac_dis3
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::macid3
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::ac_dis2
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::macid2
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::ac_dis1
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::macid1
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::ac_dis0
 * Please Place Description here.
 * @var _ss_dl_ru_tbl_::macid0
 * Please Place Description here.
 */
struct _ss_dl_ru_tbl_ {
	/* dword 0 */
	u32 ac_dis7:4;
	u32 macid7:8;
	u32 ac_dis6:4;
	u32 macid6:8;
	u32 ac_dis5:4;
	u32 macid5_l:4;
	/* dword 1 */
	u32 macid5_h:4;
	u32 ac_dis4:4;
	u32 macid4:8;
	u32 ac_dis3:4;
	u32 macid3:8;
	u32 ac_dis2:4;
	/* dword 2 */
	u32 macid2:8;
	u32 ac_dis1:4;
	u32 macid1:8;
	u32 ac_dis0:4;
	u32 macid0:8;
} ss_dl_ru_tbl, *pss_dl_ru_tbl;
#pragma pack()

#endif
