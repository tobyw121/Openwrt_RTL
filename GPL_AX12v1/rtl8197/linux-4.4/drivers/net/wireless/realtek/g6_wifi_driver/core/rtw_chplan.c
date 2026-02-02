/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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
#define _RTW_CHPLAN_C_

#include <drv_types.h>

#ifndef USE_RTW_REGD
#define USE_RTW_REGD 0
#endif

#define RTW_DOMAIN_MAP_VER		"57"
#define RTW_DOMAIN_MAP_M_VER	"g"
#define RTW_COUNTRY_MAP_VER		"31"

struct ch_list_t {
	u8 *len_ch_attr;
};

#define CLA_2G_12_14_PASSIVE	BIT0

#define CLA_5G_B1_PASSIVE		BIT0
#define CLA_5G_B2_PASSIVE		BIT1
#define CLA_5G_B3_PASSIVE		BIT2
#define CLA_5G_B4_PASSIVE		BIT3
#define CLA_5G_B2_DFS			BIT4
#define CLA_5G_B3_DFS			BIT5
#define CLA_5G_B4_DFS			BIT6

#define CLA_6G_B1_PASSIVE		BIT0
#define CLA_6G_B2_PASSIVE		BIT1
#define CLA_6G_B3_PASSIVE		BIT2
#define CLA_6G_B4_PASSIVE		BIT3

#define CH_LIST_ENT(_len, arg...) \
	{.len_ch_attr = (u8[_len + 2]) {_len, ##arg}, }

#define CH_LIST_LEN(_ch_list) (_ch_list.len_ch_attr[0])
#define CH_LIST_CH(_ch_list, _i) (_ch_list.len_ch_attr[_i + 1])
#define CH_LIST_ATTRIB(_ch_list) (_ch_list.len_ch_attr[CH_LIST_LEN(_ch_list) + 1])

enum rtw_chd_2g {
	RTW_CHD_2G_INVALID = 0,

	RTW_CHD_2G_00,
	RTW_CHD_2G_01,
	RTW_CHD_2G_02,
	RTW_CHD_2G_03,
	RTW_CHD_2G_04,
	RTW_CHD_2G_05,
	RTW_CHD_2G_06,

	RTW_CHD_2G_MAX,
	RTW_CHD_2G_NULL = RTW_CHD_2G_00,
};

enum rtw_chd_5g {
	RTW_CHD_5G_INVALID = 0,

	RTW_CHD_5G_00,
	RTW_CHD_5G_01,
	RTW_CHD_5G_02,
	RTW_CHD_5G_03,
	RTW_CHD_5G_04,
	RTW_CHD_5G_05,
	RTW_CHD_5G_06,
	RTW_CHD_5G_07,
	RTW_CHD_5G_08,
	RTW_CHD_5G_09,
	RTW_CHD_5G_10,
	RTW_CHD_5G_11,
	RTW_CHD_5G_12,
	RTW_CHD_5G_13,
	RTW_CHD_5G_14,
	RTW_CHD_5G_15,
	RTW_CHD_5G_16,
	RTW_CHD_5G_17,
	RTW_CHD_5G_18,
	RTW_CHD_5G_19,
	RTW_CHD_5G_20,
	RTW_CHD_5G_21,
	RTW_CHD_5G_22,
	RTW_CHD_5G_23,
	RTW_CHD_5G_24,
	RTW_CHD_5G_25,
	RTW_CHD_5G_26,
	RTW_CHD_5G_27,
	RTW_CHD_5G_28,
	RTW_CHD_5G_29,
	RTW_CHD_5G_30,
	RTW_CHD_5G_31,
	RTW_CHD_5G_32,
	RTW_CHD_5G_33,
	RTW_CHD_5G_34,
	RTW_CHD_5G_35,
	RTW_CHD_5G_36,
	RTW_CHD_5G_37,
	RTW_CHD_5G_38,
	RTW_CHD_5G_39,
	RTW_CHD_5G_40,
	RTW_CHD_5G_41,
	RTW_CHD_5G_42,
	RTW_CHD_5G_43,
	RTW_CHD_5G_44,
	RTW_CHD_5G_45,
	RTW_CHD_5G_46,
	RTW_CHD_5G_47,
	RTW_CHD_5G_48,
	RTW_CHD_5G_49,
	RTW_CHD_5G_50,
	RTW_CHD_5G_51,
	RTW_CHD_5G_52,

	RTW_CHD_5G_MAX,
	RTW_CHD_5G_NULL = RTW_CHD_5G_00,
};

static const struct ch_list_t rtw_channel_def_2g[] = {
	/* RTW_CHD_2G_INVALID */	CH_LIST_ENT(0, 0),
	/* RTW_CHD_2G_00 */	CH_LIST_ENT(0, 0),
	/* RTW_CHD_2G_01 */	CH_LIST_ENT(13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, CLA_2G_12_14_PASSIVE),
	/* RTW_CHD_2G_02 */	CH_LIST_ENT(13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0),
	/* RTW_CHD_2G_03 */	CH_LIST_ENT(11, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0),
	/* RTW_CHD_2G_04 */	CH_LIST_ENT(14, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0),
	/* RTW_CHD_2G_05 */	CH_LIST_ENT(4, 10, 11, 12, 13, 0),
	/* RTW_CHD_2G_06 */	CH_LIST_ENT(14, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, CLA_2G_12_14_PASSIVE),
};

#if CONFIG_IEEE80211_BAND_5GHZ
static const struct ch_list_t rtw_channel_def_5g[] = {
	/* RTW_CHD_5G_INVALID */	CH_LIST_ENT(0, 0),
	/* RTW_CHD_5G_00 */	CH_LIST_ENT(0, 0),
	/* RTW_CHD_5G_01 */	CH_LIST_ENT(21, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_02 */	CH_LIST_ENT(19, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_03 */	CH_LIST_ENT(24, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_04 */	CH_LIST_ENT(22, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 149, 153, 157, 161, 165, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_05 */	CH_LIST_ENT(19, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 149, 153, 157, 161, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_06 */	CH_LIST_ENT(9, 36, 40, 44, 48, 149, 153, 157, 161, 165, 0),
	/* RTW_CHD_5G_07 */	CH_LIST_ENT(13, 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165, CLA_5G_B2_DFS),
	/* RTW_CHD_5G_08 */	CH_LIST_ENT(12, 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, CLA_5G_B2_DFS),
	/* RTW_CHD_5G_09 */	CH_LIST_ENT(5, 149, 153, 157, 161, 165, 0),
	/* RTW_CHD_5G_10 */	CH_LIST_ENT(8, 36, 40, 44, 48, 52, 56, 60, 64, CLA_5G_B2_DFS),
	/* RTW_CHD_5G_11 */	CH_LIST_ENT(11, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, CLA_5G_B3_DFS),
	/* RTW_CHD_5G_12 */	CH_LIST_ENT(16, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_13 */	CH_LIST_ENT(8, 56, 60, 64, 149, 153, 157, 161, 165, CLA_5G_B2_DFS),
	/* RTW_CHD_5G_14 */	CH_LIST_ENT(4, 36, 40, 44, 48, 0),
	/* RTW_CHD_5G_15 */	CH_LIST_ENT(4, 149, 153, 157, 161, 0),
	/* RTW_CHD_5G_16 */	CH_LIST_ENT(11, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0),
	/* RTW_CHD_5G_17 */	CH_LIST_ENT(16, 36, 40, 44, 48, 52, 56, 60, 64, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_18 */	CH_LIST_ENT(17, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_19 */	CH_LIST_ENT(16, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_20 */	CH_LIST_ENT(20, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_21 */	CH_LIST_ENT(11, 36, 40, 44, 48, 52, 56, 60, 64, 132, 136, 140, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_22 */	CH_LIST_ENT(25, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 161, 165, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_23 */	CH_LIST_ENT(21, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 149, 153, 157, 161, 165, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_24 */	CH_LIST_ENT(24, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B2_PASSIVE | CLA_5G_B3_PASSIVE | CLA_5G_B4_PASSIVE),
	/* RTW_CHD_5G_25 */	CH_LIST_ENT(24, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B2_PASSIVE | CLA_5G_B3_PASSIVE),
	/* RTW_CHD_5G_26 */	CH_LIST_ENT(24, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B1_PASSIVE | CLA_5G_B2_PASSIVE | CLA_5G_B3_PASSIVE),
	/* RTW_CHD_5G_27 */	CH_LIST_ENT(21, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B2_PASSIVE | CLA_5G_B3_PASSIVE),
	/* RTW_CHD_5G_28 */	CH_LIST_ENT(13, 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165, CLA_5G_B2_PASSIVE),
	/* RTW_CHD_5G_29 */	CH_LIST_ENT(13, 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165, CLA_5G_B1_PASSIVE | CLA_5G_B2_PASSIVE),
	/* RTW_CHD_5G_30 */	CH_LIST_ENT(9, 36, 40, 44, 48, 149, 153, 157, 161, 165, CLA_5G_B1_PASSIVE | CLA_5G_B4_PASSIVE),
	/* RTW_CHD_5G_31 */	CH_LIST_ENT(24, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B1_PASSIVE | CLA_5G_B2_PASSIVE | CLA_5G_B3_PASSIVE | CLA_5G_B4_PASSIVE),
	/* RTW_CHD_5G_32 */	CH_LIST_ENT(9, 52, 56, 60, 64, 149, 153, 157, 161, 165, CLA_5G_B2_DFS),
	/* RTW_CHD_5G_33 */	CH_LIST_ENT(22, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 144, 149, 153, 157, 161, 165, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_34 */	CH_LIST_ENT(13, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B3_DFS),
	/* RTW_CHD_5G_35 */	CH_LIST_ENT(8, 100, 104, 108, 112, 116, 132, 136, 140, CLA_5G_B3_DFS),
	/* RTW_CHD_5G_36 */	CH_LIST_ENT(25, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 161, 165, CLA_5G_B1_PASSIVE | CLA_5G_B2_PASSIVE | CLA_5G_B2_DFS | CLA_5G_B3_PASSIVE | CLA_5G_B3_DFS | CLA_5G_B4_PASSIVE),
	/* RTW_CHD_5G_37 */	CH_LIST_ENT(8, 36, 40, 44, 48, 52, 56, 60, 64, CLA_5G_B1_PASSIVE | CLA_5G_B2_PASSIVE),
	/* RTW_CHD_5G_38 */	CH_LIST_ENT(16, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_39 */	CH_LIST_ENT(21, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 149, 153, 157, 161, 165, CLA_5G_B1_PASSIVE | CLA_5G_B2_DFS | CLA_5G_B3_DFS | CLA_5G_B4_DFS),
	/* RTW_CHD_5G_40 */	CH_LIST_ENT(21, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B1_PASSIVE | CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_41 */	CH_LIST_ENT(24, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B2_DFS | CLA_5G_B3_DFS | CLA_5G_B4_PASSIVE),
	/* RTW_CHD_5G_42 */	CH_LIST_ENT(24, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B1_PASSIVE | CLA_5G_B2_DFS | CLA_5G_B3_DFS | CLA_5G_B4_PASSIVE),
	/* RTW_CHD_5G_43 */	CH_LIST_ENT(23, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B1_PASSIVE | CLA_5G_B2_PASSIVE | CLA_5G_B3_PASSIVE | CLA_5G_B4_PASSIVE),
	/* RTW_CHD_5G_44 */	CH_LIST_ENT(21, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165, CLA_5G_B1_PASSIVE | CLA_5G_B2_PASSIVE | CLA_5G_B3_PASSIVE | CLA_5G_B4_PASSIVE),
	/* RTW_CHD_5G_45 */	CH_LIST_ENT(13, 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165, CLA_5G_B1_PASSIVE | CLA_5G_B2_PASSIVE | CLA_5G_B4_PASSIVE),
	/* RTW_CHD_5G_46 */	CH_LIST_ENT(12, 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, CLA_5G_B2_PASSIVE),
	/* RTW_CHD_5G_47 */	CH_LIST_ENT(19, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, CLA_5G_B2_PASSIVE | CLA_5G_B3_PASSIVE),
	/* RTW_CHD_5G_48 */	CH_LIST_ENT(20, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_49 */	CH_LIST_ENT(17, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_50 */	CH_LIST_ENT(17, 36, 40, 44, 48, 52, 56, 60, 64, 132, 136, 140, 144, 149, 153, 157, 161, 165, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_51 */	CH_LIST_ENT(13, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
	/* RTW_CHD_5G_52 */	CH_LIST_ENT(28, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 161, 165, 169, 173, 177, CLA_5G_B2_DFS | CLA_5G_B3_DFS),
};

#endif /* CONFIG_IEEE80211_BAND_5GHZ */

struct chplan_ent_t {
	u8 regd_2g; /* value of enum rtw_regd */
	u8 chd_2g;
#if CONFIG_IEEE80211_BAND_5GHZ
	u8 regd_5g; /* value of enum rtw_regd */
	u8 chd_5g;
#endif
};

#if CONFIG_IEEE80211_BAND_5GHZ
#define CHPLAN_ENT(_regd_2g, _chd_2g, _regd_5g, _chd_5g) {.regd_2g = RTW_REGD_##_regd_2g, .chd_2g = RTW_CHD_2G_##_chd_2g, .regd_5g = RTW_REGD_##_regd_5g, .chd_5g = RTW_CHD_5G_##_chd_5g}
#else
#define CHPLAN_ENT(_regd_2g, _chd_2g, _regd_5g, _chd_5g) {.regd_2g = RTW_REGD_##_regd_2g, .chd_2g = RTW_CHD_2G_##_chd_2g}
#endif

#define CHPLAN_ENT_NOT_DEFINED CHPLAN_ENT(NA, INVALID, NA, INVALID)

static const struct chplan_ent_t RTW_ChannelPlanMap[] = {
	[0x00] = CHPLAN_ENT(ETSI , 02, ETSI , 49),
	[0x01] = CHPLAN_ENT(ETSI , 02, ETSI , 50),
	[0x02] = CHPLAN_ENT(ETSI , 03, ETSI , 07),
	[0x03] = CHPLAN_ENT(ACMA , 02, ACMA , 33),
	[0x04] = CHPLAN_ENT(ETSI , 02, ETSI , 51),
	[0x05] = CHPLAN_ENT(ETSI , 02, ETSI , 06),
	[0x06] = CHPLAN_ENT(ETSI , 02, ETSI , 07),
	[0x07] = CHPLAN_ENT(ETSI , 02, ETSI , 23),
	[0x08] = CHPLAN_ENT(ETSI , 02, ETSI , 21),
	[0x09] = CHPLAN_ENT(ETSI , 02, ETSI , 17),
	[0x1A] = CHPLAN_ENT(NA   , 00, NA   , 00),
	[0x1B] = CHPLAN_ENT(FCC  , 02, FCC  , 52),
	[0x20] = CHPLAN_ENT(WW   , 01, NA   , 00),
	[0x21] = CHPLAN_ENT(ETSI , 02, NA   , 00),
	[0x22] = CHPLAN_ENT(FCC  , 03, NA   , 00),
	[0x23] = CHPLAN_ENT(MKK  , 04, NA   , 00),
	[0x24] = CHPLAN_ENT(ETSI , 05, NA   , 00),
	[0x25] = CHPLAN_ENT(FCC  , 03, FCC  , 03),
	[0x26] = CHPLAN_ENT(ETSI , 01, ETSI , 02),
	[0x27] = CHPLAN_ENT(MKK  , 04, MKK  , 02),
	[0x28] = CHPLAN_ENT(KCC  , 01, KCC  , 05),
	[0x29] = CHPLAN_ENT(FCC  , 01, FCC  , 06),
	[0x2A] = CHPLAN_ENT(FCC  , 02, NA   , 00),
	[0x2B] = CHPLAN_ENT(IC   , 02, IC   , 33),
	[0x2C] = CHPLAN_ENT(MKK  , 02, NA   , 00),
	[0x2D] = CHPLAN_ENT(CHILE, 01, CHILE, 22),
	[0x2E] = CHPLAN_ENT(WW   , 03, WW   , 37),
	[0x2F] = CHPLAN_ENT(CHILE, 01, CHILE, 38),
	[0x30] = CHPLAN_ENT(FCC  , 01, FCC  , 07),
	[0x31] = CHPLAN_ENT(FCC  , 01, FCC  , 08),
	[0x32] = CHPLAN_ENT(FCC  , 01, FCC  , 09),
	[0x33] = CHPLAN_ENT(FCC  , 01, FCC  , 10),
	[0x34] = CHPLAN_ENT(FCC  , 03, FCC  , 01),
	[0x35] = CHPLAN_ENT(ETSI , 01, ETSI , 03),
	[0x36] = CHPLAN_ENT(ETSI , 01, ETSI , 04),
	[0x37] = CHPLAN_ENT(MKK  , 04, MKK  , 10),
	[0x38] = CHPLAN_ENT(MKK  , 04, MKK  , 11),
	[0x39] = CHPLAN_ENT(NCC  , 03, NCC  , 12),
	[0x3A] = CHPLAN_ENT(ETSI , 02, ETSI , 02),
	[0x3B] = CHPLAN_ENT(ACMA , 02, ACMA , 01),
	[0x3C] = CHPLAN_ENT(ETSI , 02, ETSI , 10),
	[0x3D] = CHPLAN_ENT(ETSI , 02, ETSI , 15),
	[0x3E] = CHPLAN_ENT(KCC  , 02, KCC  , 03),
	[0x3F] = CHPLAN_ENT(FCC  , 03, FCC  , 22),
	[0x40] = CHPLAN_ENT(NCC  , 03, NCC  , 13),
	[0x41] = CHPLAN_ENT(WW   , 06, NA   , 00),
	[0x42] = CHPLAN_ENT(ETSI , 02, ETSI , 14),
	[0x43] = CHPLAN_ENT(FCC  , 03, FCC  , 06),
	[0x44] = CHPLAN_ENT(NCC  , 03, NCC  , 09),
	[0x45] = CHPLAN_ENT(ACMA , 01, ACMA , 01),
	[0x46] = CHPLAN_ENT(FCC  , 03, FCC  , 15),
	[0x47] = CHPLAN_ENT(ETSI , 01, ETSI , 10),
	[0x48] = CHPLAN_ENT(ETSI , 01, ETSI , 07),
	[0x49] = CHPLAN_ENT(ETSI , 01, ETSI , 06),
	[0x4A] = CHPLAN_ENT(IC   , 03, IC   , 33),
	[0x4B] = CHPLAN_ENT(KCC  , 02, KCC  , 22),
	[0x4C] = CHPLAN_ENT(FCC  , 03, FCC  , 28),
	[0x4D] = CHPLAN_ENT(MEX  , 02, MEX  , 01),
	[0x4E] = CHPLAN_ENT(ETSI , 02, ETSI , 42),
	[0x4F] = CHPLAN_ENT(NA   , 00, MKK  , 43),
	[0x50] = CHPLAN_ENT(ETSI , 01, ETSI , 16),
	[0x51] = CHPLAN_ENT(ETSI , 01, ETSI , 09),
	[0x52] = CHPLAN_ENT(ETSI , 01, ETSI , 17),
	[0x53] = CHPLAN_ENT(NCC  , 03, NCC  , 18),
	[0x54] = CHPLAN_ENT(ETSI , 01, ETSI , 15),
	[0x55] = CHPLAN_ENT(FCC  , 03, FCC  , 01),
	[0x56] = CHPLAN_ENT(ETSI , 01, ETSI , 19),
	[0x57] = CHPLAN_ENT(FCC  , 03, FCC  , 20),
	[0x58] = CHPLAN_ENT(MKK  , 02, MKK  , 14),
	[0x59] = CHPLAN_ENT(ETSI , 01, ETSI , 21),
	[0x5A] = CHPLAN_ENT(NA   , 00, FCC  , 44),
	[0x5B] = CHPLAN_ENT(NA   , 00, FCC  , 45),
	[0x5C] = CHPLAN_ENT(NA   , 00, FCC  , 43),
	[0x5D] = CHPLAN_ENT(ETSI , 02, ETSI , 08),
	[0x5E] = CHPLAN_ENT(ETSI , 02, ETSI , 03),
	[0x5F] = CHPLAN_ENT(MKK  , 02, MKK  , 47),
	[0x60] = CHPLAN_ENT(FCC  , 03, FCC  , 09),
	[0x61] = CHPLAN_ENT(FCC  , 02, FCC  , 01),
	[0x62] = CHPLAN_ENT(FCC  , 02, FCC  , 03),
	[0x63] = CHPLAN_ENT(ETSI , 01, ETSI , 23),
	[0x64] = CHPLAN_ENT(MKK  , 02, MKK  , 24),
	[0x65] = CHPLAN_ENT(ETSI , 02, ETSI , 24),
	[0x66] = CHPLAN_ENT(FCC  , 03, FCC  , 27),
	[0x67] = CHPLAN_ENT(FCC  , 03, FCC  , 25),
	[0x68] = CHPLAN_ENT(FCC  , 02, FCC  , 27),
	[0x69] = CHPLAN_ENT(FCC  , 02, FCC  , 25),
	[0x6A] = CHPLAN_ENT(ETSI , 02, ETSI , 25),
	[0x6B] = CHPLAN_ENT(FCC  , 01, FCC  , 29),
	[0x6C] = CHPLAN_ENT(FCC  , 01, FCC  , 26),
	[0x6D] = CHPLAN_ENT(FCC  , 02, FCC  , 28),
	[0x6E] = CHPLAN_ENT(FCC  , 01, FCC  , 25),
	[0x6F] = CHPLAN_ENT(NA   , 00, ETSI , 06),
	[0x70] = CHPLAN_ENT(NA   , 00, ETSI , 30),
	[0x71] = CHPLAN_ENT(NA   , 00, ETSI , 25),
	[0x72] = CHPLAN_ENT(NA   , 00, ETSI , 31),
	[0x73] = CHPLAN_ENT(FCC  , 01, FCC  , 01),
	[0x74] = CHPLAN_ENT(FCC  , 02, FCC  , 19),
	[0x75] = CHPLAN_ENT(ETSI , 01, ETSI , 32),
	[0x76] = CHPLAN_ENT(FCC  , 02, FCC  , 22),
	[0x77] = CHPLAN_ENT(ETSI , 01, ETSI , 34),
	[0x78] = CHPLAN_ENT(FCC  , 03, FCC  , 35),
	[0x79] = CHPLAN_ENT(MKK  , 02, MKK  , 02),
	[0x7A] = CHPLAN_ENT(ETSI , 02, ETSI , 28),
	[0x7B] = CHPLAN_ENT(ETSI , 02, ETSI , 46),
	[0x7C] = CHPLAN_ENT(ETSI , 02, ETSI , 47),
	[0x7D] = CHPLAN_ENT(MKK  , 04, MKK  , 48),
	[0x7E] = CHPLAN_ENT(MKK  , 02, MKK  , 48),
	[0x7F] = CHPLAN_ENT(FCC  , 01, FCC  , 03),
};

const int RTW_ChannelPlanMap_size = sizeof(RTW_ChannelPlanMap) / sizeof(RTW_ChannelPlanMap[0]);

u8 rtw_chplan_get_default_regd_2g(u8 id)
{
	if (id < RTW_ChannelPlanMap_size)
		return RTW_ChannelPlanMap[id].regd_2g;
	return RTW_REGD_NA;
}

u8 rtw_chplan_get_default_regd_5g(u8 id)
{
#if CONFIG_IEEE80211_BAND_5GHZ
	if (id < RTW_ChannelPlanMap_size)
		return RTW_ChannelPlanMap[id].regd_5g;
#endif
	return RTW_REGD_NA;
}

u8 rtw_chplan_get_default_regd(u8 id)
{
	u8 regd_2g = rtw_chplan_get_default_regd_2g(id);
	u8 regd_5g = rtw_chplan_get_default_regd_5g(id);

	if (regd_2g != RTW_REGD_NA && regd_5g != RTW_REGD_NA) {
		if (regd_2g != regd_5g)
			RTW_WARN("channel_plan:0x%02x, regd_2g:%u, regd_5g:%u not the same\n", id, regd_2g, regd_5g);
		return regd_5g;
	}
	return regd_2g != RTW_REGD_NA ? regd_2g : regd_5g;
}

bool rtw_is_channel_plan_valid(u8 id)
{
#if defined(PHL_FEATURE_NIC) && !USE_RTW_REGD
	if (rtw_phl_valid_regulation_domain(id))
		return _TRUE;
	else
		return _FALSE;
#else
	if (id < RTW_ChannelPlanMap_size) {
		const struct chplan_ent_t *chplan_map = &RTW_ChannelPlanMap[id];

		if (chplan_map->chd_2g != RTW_CHD_2G_INVALID
			#if CONFIG_IEEE80211_BAND_5GHZ
			&& chplan_map->chd_5g != RTW_CHD_5G_INVALID
			#endif
		)
			return _TRUE;
	}

	return _FALSE;
#endif
}

bool rtw_regsty_is_excl_chs(struct registry_priv *regsty, u8 ch)
{
	int i;

	for (i = 0; i < MAX_CHANNEL_NUM_2G_5G; i++) {
		if (regsty->excl_chs[i] == 0)
			break;
		if (regsty->excl_chs[i] == ch)
			return _TRUE;
	}
	return _FALSE;
}

#if CONFIG_IEEE80211_BAND_6GHZ
enum rtw_chd_6g {
	RTW_CHD_6G_INVALID = 0,

	RTW_CHD_6G_00,
	RTW_CHD_6G_01,
	RTW_CHD_6G_02,
	RTW_CHD_6G_03,
	RTW_CHD_6G_04,
	RTW_CHD_6G_05,	/* 6G Worldwide */

	RTW_CHD_6G_MAX,
	RTW_CHD_6G_NULL = RTW_CHD_6G_00,
};

static const struct ch_list_t rtw_channel_def_6g[] = {
	/* RTW_CHD_6G_INVALID */	CH_LIST_ENT(0, 0),
	/* RTW_CHD_6G_00 */	CH_LIST_ENT(0, 0),
	/* RTW_CHD_6G_01 */	CH_LIST_ENT(24, 1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 0),
	/* RTW_CHD_6G_02 */	CH_LIST_ENT(6, 97, 101, 105, 109, 113, 117, 0),
	/* RTW_CHD_6G_03 */	CH_LIST_ENT(18, 121, 125, 129, 133, 137, 141, 145, 149, 153, 157, 161, 165, 169, 173, 177, 181, 185, 189, 0),
	/* RTW_CHD_6G_04 */	CH_LIST_ENT(11, 193, 197, 201, 205, 209, 213, 217, 221, 225, 229, 233, 0),
	/* RTW_CHD_6G_05 */	CH_LIST_ENT(59, 1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 129, 133, 137, 141, 145, 149, 153, 157, 161, 165, 169, 173, 177, 181, 185, 189, 193, 197, 201, 205, 209, 213, 217, 221, 225, 229, 233, CLA_6G_B1_PASSIVE | CLA_6G_B2_PASSIVE | CLA_6G_B3_PASSIVE | CLA_6G_B4_PASSIVE),
};

struct chplan_6g_ent_t {
	u8 regd; /* value of enum rtw_regd */
	u8 chd;
};

#define CHPLAN_6G_ENT(_regd, _chd) {.regd = RTW_REGD_##_regd, .chd = RTW_CHD_6G_##_chd}

#define CHPLAN_6G_ENT_NOT_DEFINED CHPLAN_6G_ENT(NA, INVALID)

static const struct chplan_6g_ent_t rtw_chplan_6g_map[] = {
	[0x00] = CHPLAN_6G_ENT(NA   , 00),
	[0x01] = CHPLAN_6G_ENT(FCC  , 01),
	[0x02] = CHPLAN_6G_ENT(FCC  , 02),
	[0x03] = CHPLAN_6G_ENT(FCC  , 03),
	[0x04] = CHPLAN_6G_ENT(FCC  , 04),
	[0x7F] = CHPLAN_6G_ENT(WW   , 05),
};

const int rtw_chplan_6g_map_size = sizeof(rtw_chplan_6g_map) / sizeof(rtw_chplan_6g_map[0]);

u8 rtw_chplan_get_default_regd_6g(u8 id)
{
	if (id < rtw_chplan_6g_map_size)
		return rtw_chplan_6g_map[id].regd;
	return RTW_REGD_NA;
}

bool rtw_is_channel_plan_6g_valid(u8 id)
{
#if defined(PHL_FEATURE_NIC) && !USE_RTW_REGD
	/* TODO: wait PHL ready */
	return _FALSE;
#else
	if (id < rtw_chplan_6g_map_size) {
		const struct chplan_6g_ent_t *chplan_map = &rtw_chplan_6g_map[id];

		if (chplan_map->chd != RTW_CHD_6G_INVALID)
			return _TRUE;
	}

	return _FALSE;
#endif
}

bool rtw_regsty_is_excl_chs_6g(struct registry_priv *regsty, u8 ch)
{
	int i;

	for (i = 0; i < MAX_CHANNEL_NUM_6G; i++) {
		if (regsty->excl_chs_6g[i] == 0)
			break;
		if (regsty->excl_chs_6g[i] == ch)
			return _TRUE;
	}
	return _FALSE;
}
#endif /* CONFIG_IEEE80211_BAND_6GHZ */
static u8 init_channel_set_from_rtk_priv_2g_5g(_adapter *adapter, RT_CHANNEL_INFO *channel_set, u8 *ch_num)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	struct registry_priv *regsty = adapter_to_regsty(adapter);
	u8 chplan = rfctl->ChannelPlan;
	u8 index, chanset_size = *ch_num;
	u8 b5GBand = _FALSE, b2_4GBand = _FALSE;
	u8 ch, attrib;

	if (!rtw_is_channel_plan_valid(chplan)) {
		RTW_ERR(FUNC_ADPT_FMT" invalid chplan:0x%02X\n", FUNC_ADPT_ARG(adapter), chplan);
		goto exit;
	}

	RTW_INFO(FUNC_ADPT_FMT" chplan:0x%02X\n", FUNC_ADPT_ARG(adapter), chplan);

	if (is_supported_24g(regsty->band_type) && rtw_hw_chk_band_cap(dvobj, BAND_CAP_2G))
		b2_4GBand = _TRUE;

	if (is_supported_5g(regsty->band_type) && rtw_hw_chk_band_cap(dvobj, BAND_CAP_5G))
		b5GBand = _TRUE;

	if (b2_4GBand) {
		u8 chd_2g = RTW_ChannelPlanMap[chplan].chd_2g;
		if (chd_2g >= ARRAY_SIZE(rtw_channel_def_2g)) {
			RTW_ERR(FUNC_ADPT_FMT" invalid chd_2g:%d\n", FUNC_ADPT_ARG(adapter), chd_2g);
			goto exit;
		}

		attrib = CH_LIST_ATTRIB(rtw_channel_def_2g[chd_2g]);

		for (index = 0; index < CH_LIST_LEN(rtw_channel_def_2g[chd_2g]); index++) {
			ch = CH_LIST_CH(rtw_channel_def_2g[chd_2g], index);
			if (rtw_regsty_is_excl_chs(regsty, ch) == _TRUE)
				continue;

			if (chanset_size >= MAX_CHANNEL_NUM) {
				RTW_WARN("chset size can't exceed MAX_CHANNEL_NUM(%u)\n", MAX_CHANNEL_NUM);
				break;
			}

			channel_set[chanset_size].band = BAND_ON_24G;
			channel_set[chanset_size].ChannelNum = ch;

			if (ch >= 12 && ch <= 14 && (attrib & CLA_2G_12_14_PASSIVE))
				channel_set[chanset_size].flags |= RTW_CHF_NO_IR;

			if (channel_set[chanset_size].flags & RTW_CHF_NO_IR) {
				if (rfctl->country_ent || ch <= 11)
					RTW_INFO("ch%u is PASSIVE\n", ch);
			}

			chanset_size++;
		}
	}

#if CONFIG_IEEE80211_BAND_5GHZ
	if (b5GBand) {
		bool dfs;
		u8 chd_5g = RTW_ChannelPlanMap[chplan].chd_5g;

		if (chd_5g >= ARRAY_SIZE(rtw_channel_def_5g)) {
			RTW_ERR(FUNC_ADPT_FMT" invalid chd_5g:%d\n", FUNC_ADPT_ARG(adapter), chd_5g);
			goto exit;
		}
		attrib = CH_LIST_ATTRIB(rtw_channel_def_5g[chd_5g]);

		for (index = 0; index < CH_LIST_LEN(rtw_channel_def_5g[chd_5g]); index++) {
			ch = CH_LIST_CH(rtw_channel_def_5g[chd_5g], index);
			if (rtw_regsty_is_excl_chs(regsty, ch) == _TRUE)
				continue;
			dfs = (rtw_is_5g_band2(ch) && (attrib & CLA_5G_B2_DFS))
				|| (rtw_is_5g_band3(ch) && (attrib & CLA_5G_B3_DFS))
				|| (rtw_is_5g_band4(ch) && (attrib & CLA_5G_B4_DFS));
			#if !CONFIG_DFS
			if (dfs)
				continue;
			#endif

			if (chanset_size >= MAX_CHANNEL_NUM) {
				RTW_WARN("chset size can't exceed MAX_CHANNEL_NUM(%u)\n", MAX_CHANNEL_NUM);
				break;
			}

			channel_set[chanset_size].band = BAND_ON_5G;
			channel_set[chanset_size].ChannelNum = ch;

			if ((rtw_is_5g_band1(ch) && (attrib & CLA_5G_B1_PASSIVE)) /* band1 passive */
				|| (rtw_is_5g_band2(ch) && (attrib & CLA_5G_B2_PASSIVE)) /* band2 passive */
				|| (rtw_is_5g_band3(ch) && (attrib & CLA_5G_B3_PASSIVE)) /* band3 passive */
				|| (rtw_is_5g_band4(ch) && (attrib & CLA_5G_B4_PASSIVE)) /* band4 passive */
			)
				channel_set[chanset_size].flags |= RTW_CHF_NO_IR;

			if (dfs)
				channel_set[chanset_size].flags |= RTW_CHF_DFS;

			if (channel_set[chanset_size].flags & RTW_CHF_NO_IR) {
				if (rfctl->country_ent || (channel_set[chanset_size].flags & RTW_CHF_DFS))
					RTW_INFO("ch%u is PASSIVE%s\n", ch, dfs ? " DFS" : "");
			}

			chanset_size++;
		}
	}
#endif /* CONFIG_IEEE80211_BAND_5GHZ */

exit:
	*ch_num = chanset_size;

	return chanset_size;
}

#if CONFIG_IEEE80211_BAND_6GHZ
static u8 init_channel_set_from_rtk_priv_6g(_adapter *adapter, RT_CHANNEL_INFO *channel_set, u8 *ch_num)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	struct registry_priv *regsty = adapter_to_regsty(adapter);
	u8 chplan_6g = rfctl->chplan_6g;
	u8 index, chanset_size = *ch_num;
	bool band_6g = _FALSE;
	u8 chd_6g;
	u8 ch, attrib;

	if (!rtw_is_channel_plan_6g_valid(chplan_6g)) {
		RTW_ERR(FUNC_ADPT_FMT" invalid chplan_6g:0x%02X\n", FUNC_ADPT_ARG(adapter), chplan_6g);
		goto exit;
	}
	RTW_INFO(FUNC_ADPT_FMT" chplan_6g:0x%02X\n", FUNC_ADPT_ARG(adapter), chplan_6g);

	if (is_supported_6g(regsty->band_type) && rtw_hw_chk_band_cap(dvobj, BAND_CAP_6G))
		band_6g = _TRUE;
	if (!band_6g)
		goto exit;

	chd_6g = rtw_chplan_6g_map[chplan_6g].chd;

	attrib = CH_LIST_ATTRIB(rtw_channel_def_6g[chd_6g]);

	for (index = 0; index < CH_LIST_LEN(rtw_channel_def_6g[chd_6g]); index++) {
		ch = CH_LIST_CH(rtw_channel_def_6g[chd_6g], index);
		if (rtw_regsty_is_excl_chs_6g(regsty, ch) == _TRUE)
			continue;
		if (chanset_size >= MAX_CHANNEL_NUM) {
			RTW_WARN("chset size can't exceed MAX_CHANNEL_NUM(%u)\n", MAX_CHANNEL_NUM);
			break;
		}

		channel_set[chanset_size].band = BAND_ON_6G;
		channel_set[chanset_size].ChannelNum = ch;

		if ((rtw_is_6g_band1(ch) && (attrib & CLA_6G_B1_PASSIVE)) /* band1 passive */
			|| (rtw_is_6g_band2(ch) && (attrib & CLA_6G_B2_PASSIVE)) /* band2 passive */
			|| (rtw_is_6g_band3(ch) && (attrib & CLA_6G_B3_PASSIVE)) /* band3 passive */
			|| (rtw_is_6g_band4(ch) && (attrib & CLA_6G_B4_PASSIVE)) /* band4 passive */
		)
			channel_set[chanset_size].flags |= RTW_CHF_NO_IR;

		chanset_size++;
	}

exit:
	*ch_num = chanset_size;

	return chanset_size;
}
#endif /* CONFIG_IEEE80211_BAND_6GHZ */

static u8 init_channel_set_from_rtk_priv(_adapter *adapter, RT_CHANNEL_INFO *channel_set)
{
	u8 chanset_size = 0;
	int i;
	systime non_ocp_arr[MAX_CHANNEL_NUM] = {0};

#ifdef CONFIG_DFS_MASTER
	rtw_dfs_backup_non_ocp_time(channel_set,non_ocp_arr);
#endif
	_rtw_memset(channel_set, 0, sizeof(RT_CHANNEL_INFO) * MAX_CHANNEL_NUM);

	init_channel_set_from_rtk_priv_2g_5g(adapter, channel_set, &chanset_size);

#if CONFIG_IEEE80211_BAND_6GHZ
	init_channel_set_from_rtk_priv_6g(adapter, channel_set, &chanset_size);
#endif

#ifdef CONFIG_DFS_MASTER
	for (i = 0; i < chanset_size; i++)
		channel_set[i].non_ocp_end_time = rtw_get_current_time();

	rtw_dfs_restore_non_ocp_time(channel_set,non_ocp_arr);
#endif

	if (chanset_size)
		RTW_INFO(FUNC_ADPT_FMT" ch num:%d\n", FUNC_ADPT_ARG(adapter), chanset_size);
	else
		RTW_WARN(FUNC_ADPT_FMT" final chset has no channel\n", FUNC_ADPT_ARG(adapter));

	return chanset_size;
}

u8 init_channel_set(_adapter *adapter)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);


	return init_channel_set_from_rtk_priv(adapter, rfctl->channel_set);

	return 0;
}


bool rtw_chset_is_dfs_range(struct _RT_CHANNEL_INFO *chset, u32 hi, u32 lo)
{
	u8 hi_ch = rtw_freq2ch(hi);
	u8 lo_ch = rtw_freq2ch(lo);
	int i;

	for (i = 0; i < MAX_CHANNEL_NUM && chset[i].ChannelNum != 0; i++){
		if (!(chset[i].flags & RTW_CHF_DFS))
			continue;
		if (hi_ch > chset[i].ChannelNum && lo_ch < chset[i].ChannelNum)
			return 1;
	}

	return 0;
}

bool rtw_chset_is_dfs_ch(struct _RT_CHANNEL_INFO *chset, u8 ch)
{
	int i;

	for (i = 0; i < MAX_CHANNEL_NUM && chset[i].ChannelNum != 0; i++){
		if (chset[i].ChannelNum == ch)
			return chset[i].flags & RTW_CHF_DFS ? 1 : 0;
	}

	return 0;
}

bool rtw_chset_is_dfs_chbw(struct _RT_CHANNEL_INFO *chset, u8 ch, u8 bw, u8 offset)
{
	u32 hi, lo;

	if (!rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo))
		return 0;

	return rtw_chset_is_dfs_range(chset, hi, lo);
}

u32 rtw_get_rx_count_from_chset(struct _RT_CHANNEL_INFO *chset, u8 ch)
{
	int idx;

	idx = rtw_chset_search_ch(chset, ch);

	if (idx < 0)
		return 0;

	return chset[idx].rx_count;
}


u8 rtw_process_beacon_hint(_adapter *adapter, WLAN_BSSID_EX *bss)
{
#ifndef RTW_CHPLAN_BEACON_HINT_NON_WORLD_WIDE
#define RTW_CHPLAN_BEACON_HINT_NON_WORLD_WIDE 0
#endif

#ifndef RTW_CHPLAN_BEACON_HINT_ON_2G_CH_1_11
#define RTW_CHPLAN_BEACON_HINT_ON_2G_CH_1_11 0
#endif

#ifndef RTW_CHPLAN_BEACON_HINT_ON_DFS_CH
#define RTW_CHPLAN_BEACON_HINT_ON_DFS_CH 0
#endif

	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	RT_CHANNEL_INFO *chset = rfctl->channel_set;
	u8 ch = bss->Configuration.DSConfig;
	int chset_idx = rtw_chset_search_ch(chset, ch);
	u8 act_cnt = 0;

	if (chset_idx < 0)
		goto exit;

	if ((chset[chset_idx].flags & RTW_CHF_NO_IR)
		&& (RTW_CHPLAN_BEACON_HINT_NON_WORLD_WIDE || !rfctl->country_ent)
		&& (RTW_CHPLAN_BEACON_HINT_ON_2G_CH_1_11 || !(ch <= 11))
		&& (RTW_CHPLAN_BEACON_HINT_ON_DFS_CH || !(chset[chset_idx].flags & RTW_CHF_DFS))
	) {
		RTW_INFO("%s: change ch:%d to active\n", __func__, ch);
		chset[chset_idx].flags &= ~RTW_CHF_NO_IR;
		act_cnt++;
	}

exit:
	return act_cnt;
}

const char *const _regd_str[] = {
	[RTW_REGD_NA]		= "NA",
	[RTW_REGD_FCC]		= "FCC",
	[RTW_REGD_MKK]		= "MKK",
	[RTW_REGD_ETSI]		= "ETSI",
	[RTW_REGD_IC]		= "IC",
	[RTW_REGD_KCC]		= "KCC",
	[RTW_REGD_NCC]		= "NCC",
	[RTW_REGD_ACMA]		= "ACMA",
	[RTW_REGD_CHILE]	= "CHILE",
	[RTW_REGD_MEX]		= "MEX",
	[RTW_REGD_WW]		= "WW",
};
const char *const _txpwr_lmt_str[] = {
	[TXPWR_LMT_NONE]	= "NONE",
	[TXPWR_LMT_FCC]		= "FCC",
	[TXPWR_LMT_MKK]		= "MKK",
	[TXPWR_LMT_ETSI]	= "ETSI",
	[TXPWR_LMT_IC]		= "IC",
	[TXPWR_LMT_KCC]		= "KCC",
	[TXPWR_LMT_NCC]		= "NCC",
	[TXPWR_LMT_ACMA]	= "ACMA",
	[TXPWR_LMT_CHILE]	= "CHILE",
	[TXPWR_LMT_UKRAINE]	= "UKRAINE",
	[TXPWR_LMT_MEXICO]	= "MEXICO",
	[TXPWR_LMT_CN]		= "CN",
	[TXPWR_LMT_QATAR]	= "QATAR",
	[TXPWR_LMT_WW]		= "WW",
};

const REGULATION_TXPWR_LMT _txpwr_lmt_alternate[] = {
	[TXPWR_LMT_NONE]	= TXPWR_LMT_NONE,
	[TXPWR_LMT_FCC]		= TXPWR_LMT_FCC,
	[TXPWR_LMT_MKK]		= TXPWR_LMT_MKK,
	[TXPWR_LMT_ETSI]	= TXPWR_LMT_ETSI,
	[TXPWR_LMT_WW]		= TXPWR_LMT_WW,

	[TXPWR_LMT_IC]		= TXPWR_LMT_FCC,
	[TXPWR_LMT_KCC]		= TXPWR_LMT_ETSI,
	[TXPWR_LMT_NCC]		= TXPWR_LMT_FCC,
	[TXPWR_LMT_ACMA]	= TXPWR_LMT_ETSI,
	[TXPWR_LMT_CHILE]	= TXPWR_LMT_FCC,
	[TXPWR_LMT_UKRAINE]	= TXPWR_LMT_ETSI,
	[TXPWR_LMT_MEXICO]	= TXPWR_LMT_FCC,
	[TXPWR_LMT_CN]		= TXPWR_LMT_ETSI,
	[TXPWR_LMT_QATAR]	= TXPWR_LMT_ETSI,
};

const REGULATION_TXPWR_LMT _rtw_regd_to_txpwr_lmt[] = {
	[RTW_REGD_NA]		= TXPWR_LMT_NONE,
	[RTW_REGD_FCC]		= TXPWR_LMT_FCC,
	[RTW_REGD_MKK]		= TXPWR_LMT_MKK,
	[RTW_REGD_ETSI]		= TXPWR_LMT_ETSI,
	[RTW_REGD_IC]		= TXPWR_LMT_IC,
	[RTW_REGD_KCC]		= TXPWR_LMT_KCC,
	[RTW_REGD_NCC]		= TXPWR_LMT_NCC,
	[RTW_REGD_ACMA]		= TXPWR_LMT_ACMA,
	[RTW_REGD_CHILE]	= TXPWR_LMT_CHILE,
	[RTW_REGD_MEX]		= TXPWR_LMT_MEXICO,
	[RTW_REGD_WW]		= TXPWR_LMT_WW,
};
#if CONFIG_IEEE80211_BAND_6GHZ
#define COUNTRY_CHPLAN_ASSIGN_CHPLAN_6G(_val) , .chplan_6g = (_val)
#else
#define COUNTRY_CHPLAN_ASSIGN_CHPLAN_6G(_val)
#endif

#if defined(CONFIG_80211AC_VHT) || defined(CONFIG_80211AX_HE)
#define COUNTRY_CHPLAN_ASSIGN_PROTO_EN(_val) , .proto_en = (_val)
#else
#define COUNTRY_CHPLAN_ASSIGN_PROTO_EN(_val)
#endif

#define COUNTRY_CHPLAN_ENT(_alpha2, _chplan, _chplan_6g, _txpwr_lmt_override, _en_11ax, _en_11ac) \
	{.alpha2 = (_alpha2), .chplan = (_chplan) \
		COUNTRY_CHPLAN_ASSIGN_CHPLAN_6G(_chplan_6g) \
		, .txpwr_lmt_override = TXPWR_LMT_##_txpwr_lmt_override \
		COUNTRY_CHPLAN_ASSIGN_PROTO_EN((_en_11ax ? CCHPLAN_PROTO_EN_AX : 0) | (_en_11ac ? CCHPLAN_PROTO_EN_AC : 0)) \
	}

#ifdef CONFIG_CUSTOMIZED_COUNTRY_CHPLAN_MAP

#include "../platform/custom_country_chplan.h"

#elif RTW_DEF_MODULE_REGULATORY_CERT

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8821AE_HMC_M2) /* 2013 certify */
static const struct country_chplan RTL8821AE_HMC_M2_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AE", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AN", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AR", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AU", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BH", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BO", 0x73, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BR", 0x62, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BW", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CA", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CI", 0x42, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CL", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CN", 0x51, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CO", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CR", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DO", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DZ", 0x00, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EC", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EG", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ES", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GT", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HK", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HN", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ID", 0x3D, 0x00, DEF    , 0, 0),
	COUNTRY_CHPLAN_ENT("IE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IL", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JM", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JO", 0x49, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KE", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KR", 0x28, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MA", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MC", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ME", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MX", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MY", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NG", 0x75, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NI", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NZ", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("OM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PA", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PE", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PK", 0x51, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PR", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PY", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("QA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RU", 0x59, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SV", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TT", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x39, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UA", 0x36, 0x00, DEF    , 0, 0),
	COUNTRY_CHPLAN_ENT("UG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("US", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UY", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VE", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VN", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZW", 0x26, 0x00, DEF    , 0, 1),
};
#endif

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8821AU) /* 2014 certify */
static const struct country_chplan RTL8821AU_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AE", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AR", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AU", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CA", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ES", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HK", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ID", 0x3D, 0x00, DEF    , 0, 0),
	COUNTRY_CHPLAN_ENT("IE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JO", 0x49, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KR", 0x28, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MC", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NZ", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RU", 0x59, 0x00, DEF    , 0, 0),
	COUNTRY_CHPLAN_ENT("SA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x39, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UA", 0x36, 0x00, DEF    , 0, 0),
	COUNTRY_CHPLAN_ENT("US", 0x34, 0x00, DEF    , 0, 1),
};
#endif

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8812AENF_NGFF) /* 2014 certify */
static const struct country_chplan RTL8812AENF_NGFF_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x39, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("US", 0x34, 0x00, DEF    , 0, 1),
};
#endif

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8812AEBT_HMC) /* 2013 certify */
static const struct country_chplan RTL8812AEBT_HMC_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AE", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AU", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CA", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ES", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HK", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JO", 0x49, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KE", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KR", 0x28, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MC", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NG", 0x75, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NZ", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("OM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("QA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RU", 0x59, 0x00, DEF    , 0, 0),
	COUNTRY_CHPLAN_ENT("SA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x39, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UA", 0x36, 0x00, DEF    , 0, 0),
	COUNTRY_CHPLAN_ENT("US", 0x34, 0x00, DEF    , 0, 1),
};
#endif

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8188EE_HMC_M2) /* 2012 certify */
static const struct country_chplan RTL8188EE_HMC_M2_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AE", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AN", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AR", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AU", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AW", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BB", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BH", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BO", 0x73, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BR", 0x62, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BW", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CA", 0x20, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CI", 0x42, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CL", 0x2D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CO", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CR", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DO", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DZ", 0x00, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EC", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EG", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ES", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ET", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GD", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GQ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GT", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HK", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HN", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HT", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ID", 0x5D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IL", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JM", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JO", 0x49, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KE", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KR", 0x28, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MA", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MC", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ME", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ML", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MX", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MY", 0x63, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NG", 0x75, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NI", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NP", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NZ", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("OM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PA", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PE", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PK", 0x51, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PR", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PY", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("QA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RU", 0x59, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SC", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SV", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TT", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x39, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("US", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UY", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UZ", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VC", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VE", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VN", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZW", 0x26, 0x00, DEF    , 0, 1),
};
#endif

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8723BE_HMC_M2) /* 2013 certify */
static const struct country_chplan RTL8723BE_HMC_M2_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AE", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AN", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AO", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AR", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AU", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AW", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BH", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BO", 0x73, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BR", 0x62, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BS", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BW", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CA", 0x20, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CI", 0x42, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CL", 0x2D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CO", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CR", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DO", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DZ", 0x00, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EC", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EG", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ES", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ET", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GD", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GQ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GT", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HK", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HN", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ID", 0x5D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IL", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JM", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JO", 0x49, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KE", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KR", 0x28, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MA", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MC", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ME", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ML", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MX", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MY", 0x63, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NG", 0x75, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NI", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NP", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NZ", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("OM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PA", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PE", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PK", 0x51, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PR", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PY", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("QA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RU", 0x59, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SV", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TT", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x39, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("US", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UY", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UZ", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VE", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VN", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZW", 0x26, 0x00, DEF    , 0, 1),
};
#endif

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8723BS_NGFF1216) /* 2014 certify */
static const struct country_chplan RTL8723BS_NGFF1216_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AE", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AN", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AO", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AR", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AU", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BB", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BH", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BO", 0x73, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BR", 0x62, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BW", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CA", 0x20, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CI", 0x42, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CL", 0x2D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CO", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CR", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DO", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DZ", 0x00, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EC", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EG", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ES", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GT", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HK", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HN", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HT", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ID", 0x5D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IL", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JM", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JO", 0x49, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KE", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KR", 0x28, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MA", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MC", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ME", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MQ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MX", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MY", 0x63, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NG", 0x75, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NI", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NP", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NZ", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("OM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PA", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PE", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PK", 0x51, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PR", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PY", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("QA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RU", 0x59, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SV", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TT", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x39, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("US", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UY", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UZ", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VE", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VN", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("YE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZW", 0x26, 0x00, DEF    , 0, 1),
};
#endif

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8192EEBT_HMC_M2) /* 2013 certify */
static const struct country_chplan RTL8192EEBT_HMC_M2_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AE", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AN", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AO", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AR", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AU", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AW", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BH", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BO", 0x73, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BR", 0x62, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BW", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CA", 0x20, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CI", 0x42, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CL", 0x2D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CO", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CR", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DO", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DZ", 0x00, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EC", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EG", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EH", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ES", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ET", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GD", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GQ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GT", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HK", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HN", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ID", 0x5D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IL", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JM", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JO", 0x49, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KE", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KR", 0x28, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MA", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MC", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ME", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ML", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MX", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MY", 0x63, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NG", 0x75, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NI", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NP", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NZ", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("OM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PA", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PE", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PK", 0x51, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PR", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PY", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("QA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RU", 0x59, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SC", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ST", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SV", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TT", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x39, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("US", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UY", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UZ", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VE", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VN", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("YT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZW", 0x26, 0x00, DEF    , 0, 1),
};
#endif

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8723DE_NGFF1630) /* 2016 certify */
static const struct country_chplan RTL8723DE_NGFF1630_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AE", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AN", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AR", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AU", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BH", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BO", 0x73, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BR", 0x62, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CA", 0x2A, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CI", 0x42, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CL", 0x2D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CO", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CR", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DO", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DZ", 0x00, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EC", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EG", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ES", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GT", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HK", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HN", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ID", 0x5D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IL", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JM", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JO", 0x49, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KE", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KR", 0x28, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MA", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MC", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ME", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MX", 0x34, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MY", 0x63, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NG", 0x75, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NI", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NZ", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("OM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PA", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PE", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PK", 0x51, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PR", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PY", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("QA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RU", 0x59, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SV", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TT", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("US", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UY", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VE", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VN", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZW", 0x26, 0x00, DEF    , 0, 1),
};
#endif

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8822BE) /* 2016 certify */
static const struct country_chplan RTL8822BE_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AE", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AN", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AO", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AR", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AU", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BB", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BH", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BM", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BO", 0x73, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BR", 0x62, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BS", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BW", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CA", 0x2B, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CI", 0x42, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CL", 0x2D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CO", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CR", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DO", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DZ", 0x00, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EC", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EG", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EH", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ES", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FJ", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GP", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GQ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GT", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GU", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HK", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HN", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HT", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ID", 0x3D, 0x00, DEF    , 0, 0),
	COUNTRY_CHPLAN_ENT("IE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IL", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JM", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JO", 0x49, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KE", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KR", 0x28, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MA", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MC", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ME", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ML", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MO", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MQ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MX", 0x4D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MY", 0x63, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NG", 0x75, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NI", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NP", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NZ", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("OM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PA", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PE", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PK", 0x51, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PR", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PY", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("QA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RU", 0x59, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SC", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ST", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SV", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TT", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UA", 0x35, 0x00, UKRAINE, 0, 1),
	COUNTRY_CHPLAN_ENT("UG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("US", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UY", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UZ", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VE", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VN", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("YT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZW", 0x26, 0x00, DEF    , 0, 1),
};
#endif

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8821CE) /* 2016 certify */
static const struct country_chplan RTL8821CE_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AE", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AN", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AO", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AR", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AU", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BB", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BH", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BM", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BO", 0x73, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BR", 0x62, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BS", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BW", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CA", 0x2B, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CI", 0x42, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CL", 0x2D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CO", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CR", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DO", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DZ", 0x00, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EC", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EG", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EH", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ES", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ET", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FJ", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GP", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GQ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GT", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GU", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HK", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HN", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HT", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ID", 0x3D, 0x00, DEF    , 0, 0),
	COUNTRY_CHPLAN_ENT("IE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IL", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JM", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JO", 0x49, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KE", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KR", 0x28, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MA", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MC", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ME", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ML", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MO", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MQ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MX", 0x4D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MY", 0x63, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NG", 0x75, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NI", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NP", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NZ", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("OM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PA", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PE", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PK", 0x51, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PR", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PY", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("QA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RU", 0x59, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SC", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ST", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SV", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UA", 0x35, 0x00, UKRAINE, 0, 1),
	COUNTRY_CHPLAN_ENT("UG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("US", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UY", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UZ", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VE", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VN", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("YT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZW", 0x26, 0x00, DEF    , 0, 1),
};
#endif

#if (RTW_DEF_MODULE_REGULATORY_CERT & RTW_MODULE_RTL8822CE) /* 2018 certify */
static const struct country_chplan RTL8822CE_country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AE", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AN", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AO", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AR", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AU", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AW", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("AZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BB", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BH", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BM", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BO", 0x73, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BR", 0x62, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BS", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BW", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("BY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CA", 0x2B, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CI", 0x42, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CL", 0x2D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CN", 0x48, 0x00, CN     , 0, 1),
	COUNTRY_CHPLAN_ENT("CO", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CR", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CY", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("CZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DO", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("DZ", 0x00, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EC", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EG", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("EH", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ES", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ET", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FJ", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("FR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GP", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GQ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GT", 0x61, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GU", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("GW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HK", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HN", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HT", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("HU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IL", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IN", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("IT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JM", 0x32, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JO", 0x49, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("JP", 0x27, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KE", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KH", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KR", 0x4B, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("KZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LB", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("LV", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MA", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MC", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ME", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ML", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MO", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MQ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MU", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MW", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MX", 0x4D, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MY", 0x63, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("MZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NA", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NG", 0x75, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NI", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NP", 0x48, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("NZ", 0x45, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("OM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PA", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PE", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PK", 0x51, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PR", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("PY", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("QA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RO", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RS", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("RU", 0x59, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SC", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SE", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SG", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SI", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SK", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SL", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SN", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ST", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SV", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("SZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TD", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TF", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TH", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TJ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TN", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TR", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TW", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("TZ", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UA", 0x35, 0x00, UKRAINE, 0, 1),
	COUNTRY_CHPLAN_ENT("UG", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("US", 0x76, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UY", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("UZ", 0x47, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VE", 0x30, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("VN", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("YT", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZA", 0x35, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZM", 0x26, 0x00, DEF    , 0, 1),
	COUNTRY_CHPLAN_ENT("ZW", 0x26, 0x00, DEF    , 0, 1),
};
#endif

/**
 * rtw_def_module_country_chplan_map -
 * @hal_map: returned map
 * @return: size of map
 */
static u16 rtw_def_module_country_chplan_map(const struct country_chplan **hal_map)
{
	u16 hal_map_sz = 0;

	/* TODO: runtime selection for multi driver */
#if (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8821AE_HMC_M2)
	*hal_map = RTL8821AE_HMC_M2_country_chplan_map;
	hal_map_sz = sizeof(RTL8821AE_HMC_M2_country_chplan_map) / sizeof(struct country_chplan);
#elif (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8821AU)
	*hal_map = RTL8821AU_country_chplan_map;
	hal_map_sz = sizeof(RTL8821AU_country_chplan_map) / sizeof(struct country_chplan);
#elif (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8812AENF_NGFF)
	*hal_map = RTL8812AENF_NGFF_country_chplan_map;
	hal_map_sz = sizeof(RTL8812AENF_NGFF_country_chplan_map) / sizeof(struct country_chplan);
#elif (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8812AEBT_HMC)
	*hal_map = RTL8812AEBT_HMC_country_chplan_map;
	hal_map_sz = sizeof(RTL8812AEBT_HMC_country_chplan_map) / sizeof(struct country_chplan);
#elif (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8188EE_HMC_M2)
	*hal_map = RTL8188EE_HMC_M2_country_chplan_map;
	hal_map_sz = sizeof(RTL8188EE_HMC_M2_country_chplan_map) / sizeof(struct country_chplan);
#elif (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8723BE_HMC_M2)
	*hal_map = RTL8723BE_HMC_M2_country_chplan_map;
	hal_map_sz = sizeof(RTL8723BE_HMC_M2_country_chplan_map) / sizeof(struct country_chplan);
#elif (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8723BS_NGFF1216)
	*hal_map = RTL8723BS_NGFF1216_country_chplan_map;
	hal_map_sz = sizeof(RTL8723BS_NGFF1216_country_chplan_map) / sizeof(struct country_chplan);
#elif (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8192EEBT_HMC_M2)
	*hal_map = RTL8192EEBT_HMC_M2_country_chplan_map;
	hal_map_sz = sizeof(RTL8192EEBT_HMC_M2_country_chplan_map) / sizeof(struct country_chplan);
#elif (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8723DE_NGFF1630)
	*hal_map = RTL8723DE_NGFF1630_country_chplan_map;
	hal_map_sz = sizeof(RTL8723DE_NGFF1630_country_chplan_map) / sizeof(struct country_chplan);
#elif (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8822BE)
	*hal_map = RTL8822BE_country_chplan_map;
	hal_map_sz = sizeof(RTL8822BE_country_chplan_map) / sizeof(struct country_chplan);
#elif (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8821CE)
	*hal_map = RTL8821CE_country_chplan_map;
	hal_map_sz = sizeof(RTL8821CE_country_chplan_map) / sizeof(struct country_chplan);
#elif (RTW_DEF_MODULE_REGULATORY_CERT == RTW_MODULE_RTL8822CE)
	*hal_map = RTL8822CE_country_chplan_map;
	hal_map_sz = sizeof(RTL8822CE_country_chplan_map) / sizeof(struct country_chplan);
#endif

	return hal_map_sz;
}
#else

static const struct country_chplan country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AD", 0x3A, 0x00, DEF    , 1, 1), /* Andorra */
	COUNTRY_CHPLAN_ENT("AE", 0x5E, 0x00, DEF    , 1, 1), /* United Arab Emirates */
	COUNTRY_CHPLAN_ENT("AF", 0x42, 0x00, DEF    , 1, 1), /* Afghanistan */
	COUNTRY_CHPLAN_ENT("AG", 0x76, 0x00, DEF    , 1, 1), /* Antigua & Barbuda */
	COUNTRY_CHPLAN_ENT("AI", 0x5E, 0x00, DEF    , 1, 1), /* Anguilla(UK) */
	COUNTRY_CHPLAN_ENT("AL", 0x5E, 0x00, DEF    , 1, 1), /* Albania */
	COUNTRY_CHPLAN_ENT("AM", 0x5E, 0x00, DEF    , 1, 1), /* Armenia */
	COUNTRY_CHPLAN_ENT("AN", 0x76, 0x00, DEF    , 1, 1), /* Netherlands Antilles */
	COUNTRY_CHPLAN_ENT("AO", 0x5E, 0x00, DEF    , 1, 1), /* Angola */
	COUNTRY_CHPLAN_ENT("AQ", 0x26, 0x00, DEF    , 1, 1), /* Antarctica */
	COUNTRY_CHPLAN_ENT("AR", 0x4D, 0x00, DEF    , 1, 1), /* Argentina */
	COUNTRY_CHPLAN_ENT("AS", 0x76, 0x00, DEF    , 1, 1), /* American Samoa */
	COUNTRY_CHPLAN_ENT("AT", 0x5E, 0x00, DEF    , 1, 1), /* Austria */
	COUNTRY_CHPLAN_ENT("AU", 0x03, 0x00, DEF    , 1, 1), /* Australia */
	COUNTRY_CHPLAN_ENT("AW", 0x76, 0x00, DEF    , 1, 1), /* Aruba */
	COUNTRY_CHPLAN_ENT("AZ", 0x5E, 0x00, DEF    , 1, 1), /* Azerbaijan */
	COUNTRY_CHPLAN_ENT("BA", 0x5E, 0x00, DEF    , 1, 1), /* Bosnia & Herzegovina */
	COUNTRY_CHPLAN_ENT("BB", 0x76, 0x00, DEF    , 1, 1), /* Barbados */
	COUNTRY_CHPLAN_ENT("BD", 0x5E, 0x00, DEF    , 1, 1), /* Bangladesh */
	COUNTRY_CHPLAN_ENT("BE", 0x5E, 0x00, DEF    , 1, 1), /* Belgium */
	COUNTRY_CHPLAN_ENT("BF", 0x5E, 0x00, DEF    , 1, 1), /* Burkina Faso */
	COUNTRY_CHPLAN_ENT("BG", 0x5E, 0x00, DEF    , 1, 1), /* Bulgaria */
	COUNTRY_CHPLAN_ENT("BH", 0x06, 0x00, DEF    , 1, 1), /* Bahrain */
	COUNTRY_CHPLAN_ENT("BI", 0x3A, 0x00, DEF    , 1, 1), /* Burundi */
	COUNTRY_CHPLAN_ENT("BJ", 0x3A, 0x00, DEF    , 1, 1), /* Benin */
	COUNTRY_CHPLAN_ENT("BM", 0x76, 0x00, DEF    , 1, 1), /* Bermuda (UK) */
	COUNTRY_CHPLAN_ENT("BN", 0x06, 0x00, DEF    , 1, 1), /* Brunei */
	COUNTRY_CHPLAN_ENT("BO", 0x61, 0x00, DEF    , 1, 1), /* Bolivia */
	COUNTRY_CHPLAN_ENT("BR", 0x62, 0x00, DEF    , 1, 1), /* Brazil */
	COUNTRY_CHPLAN_ENT("BS", 0x76, 0x00, DEF    , 1, 1), /* Bahamas */
	COUNTRY_CHPLAN_ENT("BT", 0x5E, 0x00, DEF    , 1, 1), /* Bhutan */
	COUNTRY_CHPLAN_ENT("BV", 0x5E, 0x00, DEF    , 1, 1), /* Bouvet Island (Norway) */
	COUNTRY_CHPLAN_ENT("BW", 0x5E, 0x00, DEF    , 1, 1), /* Botswana */
	COUNTRY_CHPLAN_ENT("BY", 0x08, 0x00, DEF    , 0, 1), /* Belarus */
	COUNTRY_CHPLAN_ENT("BZ", 0x76, 0x00, DEF    , 1, 1), /* Belize */
	COUNTRY_CHPLAN_ENT("CA", 0x2B, 0x00, DEF    , 1, 1), /* Canada */
	COUNTRY_CHPLAN_ENT("CC", 0x3A, 0x00, DEF    , 1, 1), /* Cocos (Keeling) Islands (Australia) */
	COUNTRY_CHPLAN_ENT("CD", 0x3A, 0x00, DEF    , 1, 1), /* Congo, Republic of the */
	COUNTRY_CHPLAN_ENT("CF", 0x3A, 0x00, DEF    , 1, 1), /* Central African Republic */
	COUNTRY_CHPLAN_ENT("CG", 0x3A, 0x00, DEF    , 1, 1), /* Congo, Democratic Republic of the. Zaire */
	COUNTRY_CHPLAN_ENT("CH", 0x5E, 0x00, DEF    , 1, 1), /* Switzerland */
	COUNTRY_CHPLAN_ENT("CI", 0x42, 0x00, DEF    , 1, 1), /* Cote d'Ivoire */
	COUNTRY_CHPLAN_ENT("CK", 0x5E, 0x00, DEF    , 1, 1), /* Cook Islands */
	COUNTRY_CHPLAN_ENT("CL", 0x76, 0x00, CHILE  , 1, 1), /* Chile */
	COUNTRY_CHPLAN_ENT("CM", 0x5E, 0x00, DEF    , 1, 1), /* Cameroon */
	COUNTRY_CHPLAN_ENT("CN", 0x06, 0x00, CN     , 1, 1), /* China */
	COUNTRY_CHPLAN_ENT("CO", 0x76, 0x00, DEF    , 1, 1), /* Colombia */
	COUNTRY_CHPLAN_ENT("CR", 0x76, 0x00, DEF    , 1, 1), /* Costa Rica */
	COUNTRY_CHPLAN_ENT("CV", 0x5E, 0x00, DEF    , 1, 1), /* Cape Verde */
	COUNTRY_CHPLAN_ENT("CX", 0x03, 0x00, DEF    , 1, 1), /* Christmas Island (Australia) */
	COUNTRY_CHPLAN_ENT("CY", 0x5E, 0x00, DEF    , 1, 1), /* Cyprus */
	COUNTRY_CHPLAN_ENT("CZ", 0x5E, 0x00, DEF    , 1, 1), /* Czech Republic */
	COUNTRY_CHPLAN_ENT("DE", 0x5E, 0x00, DEF    , 1, 1), /* Germany */
	COUNTRY_CHPLAN_ENT("DJ", 0x3A, 0x00, DEF    , 1, 1), /* Djibouti */
	COUNTRY_CHPLAN_ENT("DK", 0x5E, 0x00, DEF    , 1, 1), /* Denmark */
	COUNTRY_CHPLAN_ENT("DM", 0x76, 0x00, DEF    , 1, 1), /* Dominica */
	COUNTRY_CHPLAN_ENT("DO", 0x76, 0x00, DEF    , 1, 1), /* Dominican Republic */
	COUNTRY_CHPLAN_ENT("DZ", 0x00, 0x00, DEF    , 1, 1), /* Algeria */
	COUNTRY_CHPLAN_ENT("EC", 0x76, 0x00, DEF    , 1, 1), /* Ecuador */
	COUNTRY_CHPLAN_ENT("EE", 0x5E, 0x00, DEF    , 1, 1), /* Estonia */
	COUNTRY_CHPLAN_ENT("EG", 0x3C, 0x00, DEF    , 1, 1), /* Egypt */
	COUNTRY_CHPLAN_ENT("EH", 0x3C, 0x00, DEF    , 1, 1), /* Western Sahara */
	COUNTRY_CHPLAN_ENT("ER", 0x3A, 0x00, DEF    , 1, 1), /* Eritrea */
	COUNTRY_CHPLAN_ENT("ES", 0x5E, 0x00, DEF    , 1, 1), /* Spain, Canary Islands, Ceuta, Melilla */
	COUNTRY_CHPLAN_ENT("ET", 0x3A, 0x00, DEF    , 1, 1), /* Ethiopia */
	COUNTRY_CHPLAN_ENT("FI", 0x5E, 0x00, DEF    , 1, 1), /* Finland */
	COUNTRY_CHPLAN_ENT("FJ", 0x76, 0x00, DEF    , 1, 1), /* Fiji */
	COUNTRY_CHPLAN_ENT("FK", 0x3A, 0x00, DEF    , 1, 1), /* Falkland Islands (Islas Malvinas) (UK) */
	COUNTRY_CHPLAN_ENT("FM", 0x76, 0x00, DEF    , 1, 1), /* Micronesia, Federated States of (USA) */
	COUNTRY_CHPLAN_ENT("FO", 0x3A, 0x00, DEF    , 1, 1), /* Faroe Islands (Denmark) */
	COUNTRY_CHPLAN_ENT("FR", 0x5E, 0x00, DEF    , 1, 1), /* France */
	COUNTRY_CHPLAN_ENT("GA", 0x5E, 0x00, DEF    , 1, 1), /* Gabon */
	COUNTRY_CHPLAN_ENT("GB", 0x5E, 0x00, DEF    , 1, 1), /* Great Britain (United Kingdom; England) */
	COUNTRY_CHPLAN_ENT("GD", 0x76, 0x00, DEF    , 1, 1), /* Grenada */
	COUNTRY_CHPLAN_ENT("GE", 0x5E, 0x00, DEF    , 1, 1), /* Georgia */
	COUNTRY_CHPLAN_ENT("GF", 0x3A, 0x00, DEF    , 1, 1), /* French Guiana */
	COUNTRY_CHPLAN_ENT("GG", 0x5E, 0x00, DEF    , 1, 1), /* Guernsey (UK) */
	COUNTRY_CHPLAN_ENT("GH", 0x5E, 0x00, DEF    , 1, 1), /* Ghana */
	COUNTRY_CHPLAN_ENT("GI", 0x3A, 0x00, DEF    , 1, 1), /* Gibraltar (UK) */
	COUNTRY_CHPLAN_ENT("GL", 0x5E, 0x00, DEF    , 1, 1), /* Greenland (Denmark) */
	COUNTRY_CHPLAN_ENT("GM", 0x3A, 0x00, DEF    , 1, 1), /* Gambia */
	COUNTRY_CHPLAN_ENT("GN", 0x3A, 0x00, DEF    , 1, 1), /* Guinea */
	COUNTRY_CHPLAN_ENT("GP", 0x3A, 0x00, DEF    , 1, 1), /* Guadeloupe (France) */
	COUNTRY_CHPLAN_ENT("GQ", 0x5E, 0x00, DEF    , 1, 1), /* Equatorial Guinea */
	COUNTRY_CHPLAN_ENT("GR", 0x5E, 0x00, DEF    , 1, 1), /* Greece */
	COUNTRY_CHPLAN_ENT("GS", 0x3A, 0x00, DEF    , 1, 1), /* South Georgia and the Sandwich Islands (UK) */
	COUNTRY_CHPLAN_ENT("GT", 0x76, 0x00, DEF    , 1, 1), /* Guatemala */
	COUNTRY_CHPLAN_ENT("GU", 0x76, 0x00, DEF    , 1, 1), /* Guam (USA) */
	COUNTRY_CHPLAN_ENT("GW", 0x3A, 0x00, DEF    , 1, 1), /* Guinea-Bissau */
	COUNTRY_CHPLAN_ENT("GY", 0x44, 0x00, DEF    , 1, 1), /* Guyana */
	COUNTRY_CHPLAN_ENT("HK", 0x5E, 0x00, DEF    , 1, 1), /* Hong Kong */
	COUNTRY_CHPLAN_ENT("HM", 0x03, 0x00, DEF    , 1, 1), /* Heard and McDonald Islands (Australia) */
	COUNTRY_CHPLAN_ENT("HN", 0x32, 0x00, DEF    , 1, 1), /* Honduras */
	COUNTRY_CHPLAN_ENT("HR", 0x5E, 0x00, DEF    , 1, 1), /* Croatia */
	COUNTRY_CHPLAN_ENT("HT", 0x76, 0x00, DEF    , 0, 1), /* Haiti */
	COUNTRY_CHPLAN_ENT("HU", 0x5E, 0x00, DEF    , 1, 1), /* Hungary */
	COUNTRY_CHPLAN_ENT("ID", 0x5D, 0x00, DEF    , 1, 1), /* Indonesia */
	COUNTRY_CHPLAN_ENT("IE", 0x5E, 0x00, DEF    , 1, 1), /* Ireland */
	COUNTRY_CHPLAN_ENT("IL", 0x5E, 0x00, DEF    , 1, 1), /* Israel */
	COUNTRY_CHPLAN_ENT("IM", 0x3A, 0x00, DEF    , 1, 1), /* Isle of Man (UK) */
	COUNTRY_CHPLAN_ENT("IN", 0x06, 0x00, DEF    , 1, 1), /* India */
	COUNTRY_CHPLAN_ENT("IO", 0x3A, 0x00, DEF    , 1, 1), /* British Indian Ocean Territory (UK) */
	COUNTRY_CHPLAN_ENT("IQ", 0x05, 0x00, DEF    , 1, 1), /* Iraq */
	COUNTRY_CHPLAN_ENT("IR", 0x3A, 0x00, DEF    , 0, 0), /* Iran */
	COUNTRY_CHPLAN_ENT("IS", 0x5E, 0x00, DEF    , 1, 1), /* Iceland */
	COUNTRY_CHPLAN_ENT("IT", 0x5E, 0x00, DEF    , 1, 1), /* Italy */
	COUNTRY_CHPLAN_ENT("JE", 0x3A, 0x00, DEF    , 1, 1), /* Jersey (UK) */
	COUNTRY_CHPLAN_ENT("JM", 0x76, 0x00, DEF    , 1, 1), /* Jamaica */
	COUNTRY_CHPLAN_ENT("JO", 0x05, 0x00, DEF    , 1, 1), /* Jordan */
	COUNTRY_CHPLAN_ENT("JP", 0x7D, 0x00, DEF    , 1, 1), /* Japan- Telec */
	COUNTRY_CHPLAN_ENT("KE", 0x5E, 0x00, DEF    , 1, 1), /* Kenya */
	COUNTRY_CHPLAN_ENT("KG", 0x5E, 0x00, DEF    , 1, 1), /* Kyrgyzstan */
	COUNTRY_CHPLAN_ENT("KH", 0x5E, 0x00, DEF    , 1, 1), /* Cambodia */
	COUNTRY_CHPLAN_ENT("KI", 0x5E, 0x00, DEF    , 1, 1), /* Kiribati */
	COUNTRY_CHPLAN_ENT("KM", 0x3A, 0x00, DEF    , 1, 1), /* Comoros */
	COUNTRY_CHPLAN_ENT("KN", 0x76, 0x00, DEF    , 1, 1), /* Saint Kitts and Nevis */
	COUNTRY_CHPLAN_ENT("KR", 0x4B, 0x00, DEF    , 1, 1), /* South Korea */
	COUNTRY_CHPLAN_ENT("KW", 0x5E, 0x00, DEF    , 1, 1), /* Kuwait */
	COUNTRY_CHPLAN_ENT("KY", 0x76, 0x00, DEF    , 1, 1), /* Cayman Islands (UK) */
	COUNTRY_CHPLAN_ENT("KZ", 0x5E, 0x00, DEF    , 1, 1), /* Kazakhstan */
	COUNTRY_CHPLAN_ENT("LA", 0x5E, 0x00, DEF    , 1, 1), /* Laos */
	COUNTRY_CHPLAN_ENT("LB", 0x5E, 0x00, DEF    , 1, 1), /* Lebanon */
	COUNTRY_CHPLAN_ENT("LC", 0x76, 0x00, DEF    , 1, 1), /* Saint Lucia */
	COUNTRY_CHPLAN_ENT("LI", 0x5E, 0x00, DEF    , 1, 1), /* Liechtenstein */
	COUNTRY_CHPLAN_ENT("LK", 0x5E, 0x00, DEF    , 1, 1), /* Sri Lanka */
	COUNTRY_CHPLAN_ENT("LR", 0x5E, 0x00, DEF    , 1, 1), /* Liberia */
	COUNTRY_CHPLAN_ENT("LS", 0x5E, 0x00, DEF    , 1, 1), /* Lesotho */
	COUNTRY_CHPLAN_ENT("LT", 0x5E, 0x00, DEF    , 1, 1), /* Lithuania */
	COUNTRY_CHPLAN_ENT("LU", 0x5E, 0x00, DEF    , 1, 1), /* Luxembourg */
	COUNTRY_CHPLAN_ENT("LV", 0x5E, 0x00, DEF    , 1, 1), /* Latvia */
	COUNTRY_CHPLAN_ENT("LY", 0x5E, 0x00, DEF    , 1, 1), /* Libya */
	COUNTRY_CHPLAN_ENT("MA", 0x3C, 0x00, DEF    , 1, 1), /* Morocco */
	COUNTRY_CHPLAN_ENT("MC", 0x5E, 0x00, DEF    , 1, 1), /* Monaco */
	COUNTRY_CHPLAN_ENT("MD", 0x5E, 0x00, DEF    , 1, 1), /* Moldova */
	COUNTRY_CHPLAN_ENT("ME", 0x3A, 0x00, DEF    , 1, 1), /* Montenegro */
	COUNTRY_CHPLAN_ENT("MF", 0x76, 0x00, DEF    , 1, 1), /* Saint Martin */
	COUNTRY_CHPLAN_ENT("MG", 0x5E, 0x00, DEF    , 1, 1), /* Madagascar */
	COUNTRY_CHPLAN_ENT("MH", 0x76, 0x00, DEF    , 1, 1), /* Marshall Islands (USA) */
	COUNTRY_CHPLAN_ENT("MK", 0x3A, 0x00, DEF    , 1, 1), /* Republic of Macedonia (FYROM) */
	COUNTRY_CHPLAN_ENT("ML", 0x5E, 0x00, DEF    , 1, 1), /* Mali */
	COUNTRY_CHPLAN_ENT("MM", 0x3A, 0x00, DEF    , 1, 1), /* Burma (Myanmar) */
	COUNTRY_CHPLAN_ENT("MN", 0x5E, 0x00, DEF    , 1, 1), /* Mongolia */
	COUNTRY_CHPLAN_ENT("MO", 0x5E, 0x00, DEF    , 1, 1), /* Macau */
	COUNTRY_CHPLAN_ENT("MP", 0x76, 0x00, DEF    , 1, 1), /* Northern Mariana Islands (USA) */
	COUNTRY_CHPLAN_ENT("MQ", 0x3A, 0x00, DEF    , 1, 1), /* Martinique (France) */
	COUNTRY_CHPLAN_ENT("MR", 0x5E, 0x00, DEF    , 1, 1), /* Mauritania */
	COUNTRY_CHPLAN_ENT("MS", 0x3A, 0x00, DEF    , 1, 1), /* Montserrat (UK) */
	COUNTRY_CHPLAN_ENT("MT", 0x5E, 0x00, DEF    , 1, 1), /* Malta */
	COUNTRY_CHPLAN_ENT("MU", 0x5E, 0x00, DEF    , 1, 1), /* Mauritius */
	COUNTRY_CHPLAN_ENT("MV", 0x3C, 0x00, DEF    , 1, 1), /* Maldives */
	COUNTRY_CHPLAN_ENT("MW", 0x5E, 0x00, DEF    , 1, 1), /* Malawi */
	COUNTRY_CHPLAN_ENT("MX", 0x4D, 0x00, DEF    , 1, 1), /* Mexico */
	COUNTRY_CHPLAN_ENT("MY", 0x07, 0x00, DEF    , 1, 1), /* Malaysia */
	COUNTRY_CHPLAN_ENT("MZ", 0x5E, 0x00, DEF    , 1, 1), /* Mozambique */
	COUNTRY_CHPLAN_ENT("NA", 0x5E, 0x00, DEF    , 1, 1), /* Namibia */
	COUNTRY_CHPLAN_ENT("NC", 0x3A, 0x00, DEF    , 1, 1), /* New Caledonia */
	COUNTRY_CHPLAN_ENT("NE", 0x5E, 0x00, DEF    , 1, 1), /* Niger */
	COUNTRY_CHPLAN_ENT("NF", 0x03, 0x00, DEF    , 1, 1), /* Norfolk Island (Australia) */
	COUNTRY_CHPLAN_ENT("NG", 0x75, 0x00, DEF    , 1, 1), /* Nigeria */
	COUNTRY_CHPLAN_ENT("NI", 0x76, 0x00, DEF    , 1, 1), /* Nicaragua */
	COUNTRY_CHPLAN_ENT("NL", 0x5E, 0x00, DEF    , 1, 1), /* Netherlands */
	COUNTRY_CHPLAN_ENT("NO", 0x5E, 0x00, DEF    , 1, 1), /* Norway */
	COUNTRY_CHPLAN_ENT("NP", 0x06, 0x00, DEF    , 1, 1), /* Nepal */
	COUNTRY_CHPLAN_ENT("NR", 0x3A, 0x00, DEF    , 1, 1), /* Nauru */
	COUNTRY_CHPLAN_ENT("NU", 0x03, 0x00, DEF    , 1, 1), /* Niue */
	COUNTRY_CHPLAN_ENT("NZ", 0x03, 0x00, DEF    , 1, 1), /* New Zealand */
	COUNTRY_CHPLAN_ENT("OM", 0x5E, 0x00, DEF    , 1, 1), /* Oman */
	COUNTRY_CHPLAN_ENT("PA", 0x76, 0x00, DEF    , 1, 1), /* Panama */
	COUNTRY_CHPLAN_ENT("PE", 0x76, 0x00, DEF    , 1, 1), /* Peru */
	COUNTRY_CHPLAN_ENT("PF", 0x3A, 0x00, DEF    , 1, 1), /* French Polynesia (France) */
	COUNTRY_CHPLAN_ENT("PG", 0x5E, 0x00, DEF    , 1, 1), /* Papua New Guinea */
	COUNTRY_CHPLAN_ENT("PH", 0x5E, 0x00, DEF    , 1, 1), /* Philippines */
	COUNTRY_CHPLAN_ENT("PK", 0x51, 0x00, DEF    , 1, 1), /* Pakistan */
	COUNTRY_CHPLAN_ENT("PL", 0x5E, 0x00, DEF    , 1, 1), /* Poland */
	COUNTRY_CHPLAN_ENT("PM", 0x3A, 0x00, DEF    , 1, 1), /* Saint Pierre and Miquelon (France) */
	COUNTRY_CHPLAN_ENT("PR", 0x76, 0x00, DEF    , 1, 1), /* Puerto Rico */
	COUNTRY_CHPLAN_ENT("PS", 0x5E, 0x00, DEF    , 1, 1), /* Palestine */
	COUNTRY_CHPLAN_ENT("PT", 0x5E, 0x00, DEF    , 1, 1), /* Portugal */
	COUNTRY_CHPLAN_ENT("PW", 0x76, 0x00, DEF    , 1, 1), /* Palau */
	COUNTRY_CHPLAN_ENT("PY", 0x76, 0x00, DEF    , 1, 1), /* Paraguay */
	COUNTRY_CHPLAN_ENT("QA", 0x5E, 0x00, QATAR  , 1, 1), /* Qatar */
	COUNTRY_CHPLAN_ENT("RE", 0x3A, 0x00, DEF    , 1, 1), /* Reunion (France) */
	COUNTRY_CHPLAN_ENT("RO", 0x5E, 0x00, DEF    , 1, 1), /* Romania */
	COUNTRY_CHPLAN_ENT("RS", 0x5E, 0x00, DEF    , 1, 1), /* Serbia */
	COUNTRY_CHPLAN_ENT("RU", 0x09, 0x00, DEF    , 1, 1), /* Russia(fac/gost), Kaliningrad */
	COUNTRY_CHPLAN_ENT("RW", 0x5E, 0x00, DEF    , 1, 1), /* Rwanda */
	COUNTRY_CHPLAN_ENT("SA", 0x5E, 0x00, DEF    , 1, 1), /* Saudi Arabia */
	COUNTRY_CHPLAN_ENT("SB", 0x3A, 0x00, DEF    , 1, 1), /* Solomon Islands */
	COUNTRY_CHPLAN_ENT("SC", 0x76, 0x00, DEF    , 1, 1), /* Seychelles */
	COUNTRY_CHPLAN_ENT("SE", 0x5E, 0x00, DEF    , 1, 1), /* Sweden */
	COUNTRY_CHPLAN_ENT("SG", 0x5E, 0x00, DEF    , 1, 1), /* Singapore */
	COUNTRY_CHPLAN_ENT("SH", 0x3A, 0x00, DEF    , 1, 1), /* Saint Helena (UK) */
	COUNTRY_CHPLAN_ENT("SI", 0x5E, 0x00, DEF    , 1, 1), /* Slovenia */
	COUNTRY_CHPLAN_ENT("SJ", 0x3A, 0x00, DEF    , 1, 1), /* Svalbard (Norway) */
	COUNTRY_CHPLAN_ENT("SK", 0x5E, 0x00, DEF    , 1, 1), /* Slovakia */
	COUNTRY_CHPLAN_ENT("SL", 0x5E, 0x00, DEF    , 1, 1), /* Sierra Leone */
	COUNTRY_CHPLAN_ENT("SM", 0x3A, 0x00, DEF    , 1, 1), /* San Marino */
	COUNTRY_CHPLAN_ENT("SN", 0x3A, 0x00, DEF    , 1, 1), /* Senegal */
	COUNTRY_CHPLAN_ENT("SO", 0x3A, 0x00, DEF    , 1, 1), /* Somalia */
	COUNTRY_CHPLAN_ENT("SR", 0x74, 0x00, DEF    , 1, 1), /* Suriname */
	COUNTRY_CHPLAN_ENT("ST", 0x76, 0x00, DEF    , 1, 1), /* Sao Tome and Principe */
	COUNTRY_CHPLAN_ENT("SV", 0x76, 0x00, DEF    , 1, 1), /* El Salvador */
	COUNTRY_CHPLAN_ENT("SX", 0x76, 0x00, DEF    , 1, 1), /* Sint Marteen */
	COUNTRY_CHPLAN_ENT("SZ", 0x5E, 0x00, DEF    , 1, 1), /* Swaziland */
	COUNTRY_CHPLAN_ENT("TC", 0x3A, 0x00, DEF    , 1, 1), /* Turks and Caicos Islands (UK) */
	COUNTRY_CHPLAN_ENT("TD", 0x3A, 0x00, DEF    , 1, 1), /* Chad */
	COUNTRY_CHPLAN_ENT("TF", 0x3A, 0x00, DEF    , 1, 1), /* French Southern and Antarctic Lands (FR Southern Territories) */
	COUNTRY_CHPLAN_ENT("TG", 0x5E, 0x00, DEF    , 1, 1), /* Togo */
	COUNTRY_CHPLAN_ENT("TH", 0x5E, 0x00, DEF    , 1, 1), /* Thailand */
	COUNTRY_CHPLAN_ENT("TJ", 0x5E, 0x00, DEF    , 1, 1), /* Tajikistan */
	COUNTRY_CHPLAN_ENT("TK", 0x03, 0x00, DEF    , 1, 1), /* Tokelau */
	COUNTRY_CHPLAN_ENT("TM", 0x3A, 0x00, DEF    , 1, 1), /* Turkmenistan */
	COUNTRY_CHPLAN_ENT("TN", 0x04, 0x00, DEF    , 1, 1), /* Tunisia */
	COUNTRY_CHPLAN_ENT("TO", 0x3A, 0x00, DEF    , 1, 1), /* Tonga */
	COUNTRY_CHPLAN_ENT("TR", 0x5E, 0x00, DEF    , 1, 1), /* Turkey, Northern Cyprus */
	COUNTRY_CHPLAN_ENT("TT", 0x76, 0x00, DEF    , 1, 1), /* Trinidad & Tobago */
	COUNTRY_CHPLAN_ENT("TV", 0x21, 0x00, DEF    , 0, 0), /* Tuvalu */
	COUNTRY_CHPLAN_ENT("TW", 0x76, 0x00, DEF    , 1, 1), /* Taiwan */
	COUNTRY_CHPLAN_ENT("TZ", 0x5E, 0x00, DEF    , 1, 1), /* Tanzania */
	COUNTRY_CHPLAN_ENT("UA", 0x5E, 0x00, UKRAINE, 1, 1), /* Ukraine */
	COUNTRY_CHPLAN_ENT("UG", 0x3A, 0x00, DEF    , 1, 1), /* Uganda */
	COUNTRY_CHPLAN_ENT("US", 0x76, 0x7F, DEF    , 1, 1), /* United States of America (USA) */
	COUNTRY_CHPLAN_ENT("UY", 0x30, 0x00, DEF    , 1, 1), /* Uruguay */
	COUNTRY_CHPLAN_ENT("UZ", 0x3C, 0x00, DEF    , 1, 1), /* Uzbekistan */
	COUNTRY_CHPLAN_ENT("VA", 0x3A, 0x00, DEF    , 1, 1), /* Holy See (Vatican City) */
	COUNTRY_CHPLAN_ENT("VC", 0x76, 0x00, DEF    , 1, 1), /* Saint Vincent and the Grenadines */
	COUNTRY_CHPLAN_ENT("VE", 0x30, 0x00, DEF    , 1, 1), /* Venezuela */
	COUNTRY_CHPLAN_ENT("VG", 0x76, 0x00, DEF    , 1, 1), /* British Virgin Islands (UK) */
	COUNTRY_CHPLAN_ENT("VI", 0x76, 0x00, DEF    , 1, 1), /* United States Virgin Islands (USA) */
	COUNTRY_CHPLAN_ENT("VN", 0x5E, 0x00, DEF    , 1, 1), /* Vietnam */
	COUNTRY_CHPLAN_ENT("VU", 0x26, 0x00, DEF    , 1, 1), /* Vanuatu */
	COUNTRY_CHPLAN_ENT("WF", 0x3A, 0x00, DEF    , 1, 1), /* Wallis and Futuna (France) */
	COUNTRY_CHPLAN_ENT("WS", 0x76, 0x00, DEF    , 1, 1), /* Samoa */
	COUNTRY_CHPLAN_ENT("XK", 0x3A, 0x00, DEF    , 1, 1), /* Kosovo */
	COUNTRY_CHPLAN_ENT("YE", 0x3A, 0x00, DEF    , 1, 1), /* Yemen */
	COUNTRY_CHPLAN_ENT("YT", 0x5E, 0x00, DEF    , 1, 1), /* Mayotte (France) */
	COUNTRY_CHPLAN_ENT("ZA", 0x5E, 0x00, DEF    , 1, 1), /* South Africa */
	COUNTRY_CHPLAN_ENT("ZM", 0x5E, 0x00, DEF    , 1, 1), /* Zambia */
	COUNTRY_CHPLAN_ENT("ZW", 0x5E, 0x00, DEF    , 1, 1), /* Zimbabwe */
};
#endif /* CONFIG_CUSTOMIZED_COUNTRY_CHPLAN_MAP or RTW_DEF_MODULE_REGULATORY_CERT or newest */

/*
* rtw_get_chplan_from_country -
* @country_code: string of country code
*
* Return pointer of struct country_chplan entry or NULL when unsupported country_code is given
*/
bool rtw_get_chplan_from_country(const char *country_code, struct country_chplan *ent)
{
	const struct country_chplan *map = NULL;
	u16 map_sz = 0;
	char code[2];
	int i;

	code[0] = alpha_to_upper(country_code[0]);
	code[1] = alpha_to_upper(country_code[1]);

#ifdef CONFIG_CUSTOMIZED_COUNTRY_CHPLAN_MAP
	map = CUSTOMIZED_country_chplan_map;
	map_sz = sizeof(CUSTOMIZED_country_chplan_map) / sizeof(struct country_chplan);
#elif RTW_DEF_MODULE_REGULATORY_CERT
	map_sz = rtw_def_module_country_chplan_map(&map);
#else
	map = country_chplan_map;
	map_sz = sizeof(country_chplan_map) / sizeof(struct country_chplan);
#endif

	for (i = 0; i < map_sz; i++) {
		if (strncmp(code, map[i].alpha2, 2) == 0) {
			_rtw_memcpy(ent, &map[i], sizeof(*ent));
			return _TRUE;
		}
	}

	return _FALSE;
}

void rtw_chplan_ioctl_input_mapping(u16 *chplan, u16 *chplan_6g)
{
	if (chplan) {
		if (*chplan == RTW_CHPLAN_IOCTL_UNSPECIFIED)
			*chplan = RTW_CHPLAN_UNSPECIFIED;
		else if (*chplan == RTW_CHPLAN_IOCTL_NULL)
			*chplan = RTW_CHPLAN_NULL;
	}

	if (chplan_6g) {
		if (*chplan_6g == RTW_CHPLAN_IOCTL_UNSPECIFIED)
			*chplan_6g = RTW_CHPLAN_6G_UNSPECIFIED;
		else if (*chplan_6g == RTW_CHPLAN_IOCTL_NULL)
			*chplan_6g = RTW_CHPLAN_6G_NULL;
	}
}
void dump_country_chplan(void *sel, const struct country_chplan *ent)
{
	u8 txpwr_lmt;
	char buf[16];
	char *pos = buf;

	if (ent->chplan == RTW_CHPLAN_UNSPECIFIED)
		pos += sprintf(pos, "NA");
	else
		pos += sprintf(pos, "0x%02X", ent->chplan);

#if CONFIG_IEEE80211_BAND_6GHZ
	if (ent->chplan_6g == RTW_CHPLAN_6G_UNSPECIFIED)
		pos += sprintf(pos, " NA");
	else
		pos += sprintf(pos, " 0x%02X", ent->chplan_6g);
#endif



	if (ent->txpwr_lmt_override != TXPWR_LMT_DEF)
		_RTW_PRINT_SEL(sel, " %-7s", txpwr_lmt_str(ent->txpwr_lmt_override));
	else if (ent->chplan != RTW_CHPLAN_UNSPECIFIED) {
		txpwr_lmt = rtw_regd_to_txpwr_lmt(rtw_chplan_get_default_regd(ent->chplan));
		_RTW_PRINT_SEL(sel, " %-7s", txpwr_lmt_str(txpwr_lmt));
	}
	_RTW_PRINT_SEL(sel, " %s", COUNTRY_CHPLAN_EN_11AX(ent) ? "ax" : "  ");
	_RTW_PRINT_SEL(sel, " %s", COUNTRY_CHPLAN_EN_11AC(ent) ? "ac" : "  ");

	_RTW_PRINT_SEL(sel, "\n");
}

void dump_country_chplan_map(void *sel)
{
	struct country_chplan ent;
	u8 code[2];

	_rtw_memset(&ent, 0, sizeof(struct country_chplan));

#if RTW_DEF_MODULE_REGULATORY_CERT
	RTW_PRINT_SEL(sel, "RTW_DEF_MODULE_REGULATORY_CERT:0x%x\n", RTW_DEF_MODULE_REGULATORY_CERT);
#endif
#ifdef CONFIG_CUSTOMIZED_COUNTRY_CHPLAN_MAP
	RTW_PRINT_SEL(sel, "CONFIG_CUSTOMIZED_COUNTRY_CHPLAN_MAP\n");
#endif

	for (code[0] = 'A'; code[0] <= 'Z'; code[0]++) {
		for (code[1] = 'A'; code[1] <= 'Z'; code[1]++) {
			if (rtw_get_chplan_from_country(code, &ent) == _FALSE)
				continue;

			dump_country_chplan(sel, &ent);
		}
	}
}

void dump_country_list(void *sel)
{
	struct country_chplan ent;
	u8 code[2];
	u8 first = 1;

	for (code[0] = 'A'; code[0] <= 'Z'; code[0]++) {
		for (code[1] = 'A'; code[1] <= 'Z'; code[1]++) {
			if (!rtw_get_chplan_from_country(code, &ent))
				continue;

			if (first) {
				RTW_PRINT_SEL(sel, "%c%c ", code[0], code[1]);
				first = 0;
			} else
				_RTW_PRINT_SEL(sel, "%c%c ", code[0], code[1]);
		}
	}
	if (first == 0)
		_RTW_PRINT_SEL(sel, "\n");
}

void dump_chplan_id_list(void *sel)
{
	u8 id_search_max = RTW_ChannelPlanMap_size - 1;
	u8 first = 1;
	int i;

	for (i = 0; i <= id_search_max; i++) {
		if (!rtw_is_channel_plan_valid(i))
			continue;

		if (first) {
			RTW_PRINT_SEL(sel, "0x%02X ", i);
			first = 0;
		} else
			_RTW_PRINT_SEL(sel, "0x%02X ", i);
	}
	if (first == 0)
		_RTW_PRINT_SEL(sel, "\n");
}

void dump_chplan_country_list(void *sel)
{
#if defined(PHL_FEATURE_NIC) && !USE_RTW_REGD
	u8 id_search_max = 255;
#else
	u8 id_search_max = RTW_ChannelPlanMap_size - 1;
#endif
	int i;

	for (i = 0; i <= id_search_max; i++) {
		struct country_chplan ent;
		u8 code[2];
		u8 first;

		if (!rtw_is_channel_plan_valid(i))
			continue;

		first = 1;
		for (code[0] = 'A'; code[0] <= 'Z'; code[0]++) {
			for (code[1] = 'A'; code[1] <= 'Z'; code[1]++) {
				if (!rtw_get_chplan_from_country(code, &ent) || ent.chplan != i)
					continue;

				if (first) {
					RTW_PRINT_SEL(sel, "0x%02X %c%c ", i, code[0], code[1]);
					first = 0;
				} else
					_RTW_PRINT_SEL(sel, "%c%c ", code[0], code[1]);
			}
		}
		if (first == 0)
			_RTW_PRINT_SEL(sel, "\n");
	}
}

#if CONFIG_IEEE80211_BAND_6GHZ
void dump_chplan_6g_id_list(void *sel)
{
	u8 id_search_max = rtw_chplan_6g_map_size - 1;
	u8 first = 1;
	int i;

	for (i = 0; i <= id_search_max; i++) {
		if (!rtw_is_channel_plan_6g_valid(i))
			continue;

		if (first) {
			RTW_PRINT_SEL(sel, "0x%02X ", i);
			first = 0;
		} else
			_RTW_PRINT_SEL(sel, "0x%02X ", i);
	}
	if (first == 0)
		_RTW_PRINT_SEL(sel, "\n");
}

void dump_chplan_6g_country_list(void *sel)
{
	u8 id_search_max = rtw_chplan_6g_map_size - 1;
	int i;

	for (i = 0; i <= id_search_max; i++) {
		struct country_chplan ent;
		u8 code[2];
		u8 first;

		if (!rtw_is_channel_plan_6g_valid(i))
			continue;

		first = 1;
		for (code[0] = 'A'; code[0] <= 'Z'; code[0]++) {
			for (code[1] = 'A'; code[1] <= 'Z'; code[1]++) {
				if (!rtw_get_chplan_from_country(code, &ent) || ent.chplan_6g != i)
					continue;

				if (first) {
					RTW_PRINT_SEL(sel, "0x%02X %c%c ", i, code[0], code[1]);
					first = 0;
				} else
					_RTW_PRINT_SEL(sel, "%c%c ", code[0], code[1]);
			}
		}
		if (first == 0)
			_RTW_PRINT_SEL(sel, "\n");
	}
}
#endif /* CONFIG_IEEE80211_BAND_6GHZ */
void dump_chplan_test(void *sel)
{
	int i, j;

	/* check 2G CHD redundent */
	for (i = RTW_CHD_2G_00; i < RTW_CHD_2G_MAX; i++) {
		for (j = RTW_CHD_2G_00; j < i; j++) {
			if (CH_LIST_LEN(rtw_channel_def_2g[i]) == CH_LIST_LEN(rtw_channel_def_2g[j])
				&& _rtw_memcmp(&CH_LIST_CH(rtw_channel_def_2g[i], 0), &CH_LIST_CH(rtw_channel_def_2g[j], 0), CH_LIST_LEN(rtw_channel_def_2g[i]) + 1) == _TRUE)
				RTW_PRINT_SEL(sel, "2G chd:%u and %u is the same\n", i, j);
		}
	}

	/* check 2G CHD invalid channel */
	for (i = RTW_CHD_2G_00; i < RTW_CHD_2G_MAX; i++) {
		for (j = 0; j < CH_LIST_LEN(rtw_channel_def_2g[i]); j++) {
			if (rtw_ch2freq(CH_LIST_CH(rtw_channel_def_2g[i], j)) == 0)
				RTW_PRINT_SEL(sel, "2G invalid ch:%u at (%d,%d)\n", CH_LIST_CH(rtw_channel_def_2g[i], j), i, j);
		}
	}

#if CONFIG_IEEE80211_BAND_5GHZ
	/* check 5G CHD redundent */
	for (i = RTW_CHD_5G_00; i < RTW_CHD_5G_MAX; i++) {
		for (j = RTW_CHD_5G_00; j < i; j++) {
			if (CH_LIST_LEN(rtw_channel_def_5g[i]) == CH_LIST_LEN(rtw_channel_def_5g[j])
				&& _rtw_memcmp(&CH_LIST_CH(rtw_channel_def_5g[i], 0), &CH_LIST_CH(rtw_channel_def_5g[j], 0), CH_LIST_LEN(rtw_channel_def_5g[i]) + 1) == _TRUE)
				RTW_PRINT_SEL(sel, "5G chd:%u and %u is the same\n", i, j);
		}
	}

	/* check 5G CHD invalid channel */
	for (i = RTW_CHD_5G_00; i < RTW_CHD_5G_MAX; i++) {
		for (j = 0; j < CH_LIST_LEN(rtw_channel_def_5g[i]); j++) {
			if (rtw_ch2freq(CH_LIST_CH(rtw_channel_def_5g[i], j)) == 0)
				RTW_PRINT_SEL(sel, "5G invalid ch:%u at (%d,%d)\n", CH_LIST_CH(rtw_channel_def_5g[i], j), i, j);
		}
	}
#endif

#if CONFIG_IEEE80211_BAND_6GHZ
	/* check 6G CHD redundent */
	for (i = RTW_CHD_6G_00; i < RTW_CHD_6G_MAX; i++) {
		for (j = RTW_CHD_6G_00; j < i; j++) {
			if (CH_LIST_LEN(rtw_channel_def_6g[i]) == CH_LIST_LEN(rtw_channel_def_6g[j])
				&& _rtw_memcmp(&CH_LIST_CH(rtw_channel_def_6g[i], 0), &CH_LIST_CH(rtw_channel_def_6g[j], 0), CH_LIST_LEN(rtw_channel_def_6g[i]) + 1) == _TRUE)
				RTW_PRINT_SEL(sel, "6G chd:%u and %u is the same\n", i, j);
		}
	}

	/* check 6G CHD invalid channel */
	for (i = RTW_CHD_6G_00; i < RTW_CHD_6G_MAX; i++) {
		for (j = 0; j < CH_LIST_LEN(rtw_channel_def_6g[i]); j++) {
			if (rtw_6gch2freq(CH_LIST_CH(rtw_channel_def_6g[i], j)) == 0)
				RTW_PRINT_SEL(sel, "6G invalid ch:%u at (%d,%d)\n", CH_LIST_CH(rtw_channel_def_6g[i], j), i, j);
		}
	}
#endif

	/* check chplan 2G_5G redundent */
	for (i = 0; i < RTW_ChannelPlanMap_size; i++) {
		if (!rtw_is_channel_plan_valid(i))
			continue;
		for (j = 0; j < i; j++) {
			if (!rtw_is_channel_plan_valid(j))
				continue;
			if (_rtw_memcmp(&RTW_ChannelPlanMap[i], &RTW_ChannelPlanMap[j], sizeof(RTW_ChannelPlanMap[i])) == _TRUE)
				RTW_PRINT_SEL(sel, "channel plan 0x%02x and 0x%02x is the same\n", i, j);
		}
	}

#if CONFIG_IEEE80211_BAND_6GHZ
	/* check chplan 6G redundent */
	for (i = 0; i < rtw_chplan_6g_map_size; i++) {
		if (!rtw_is_channel_plan_6g_valid(i))
			continue;
		for (j = 0; j < i; j++) {
			if (!rtw_is_channel_plan_6g_valid(j))
				continue;
			if (_rtw_memcmp(&rtw_chplan_6g_map[i], &rtw_chplan_6g_map[j], sizeof(rtw_chplan_6g_map[i])) == _TRUE)
				RTW_PRINT_SEL(sel, "channel plan 6g 0x%02x and 0x%02x is the same\n", i, j);
		}
	}
#endif


	/* check country invalid chplan/chplan_6g */
	{
		struct country_chplan ent;
		u8 code[2];

		for (code[0] = 'A'; code[0] <= 'Z'; code[0]++) {
			for (code[1] = 'A'; code[1] <= 'Z'; code[1]++) {
				if (!rtw_get_chplan_from_country(code, &ent))
					continue;
				if (!rtw_is_channel_plan_valid(ent.chplan))
					RTW_PRINT_SEL(sel, "country \"%c%c\" has invalid chplan:0x%02X\n", code[0], code[1], ent.chplan);
				#if CONFIG_IEEE80211_BAND_6GHZ
				if (!rtw_is_channel_plan_6g_valid(ent.chplan_6g))
					RTW_PRINT_SEL(sel, "country \"%c%c\" has invalid chplan_6g:0x%02X\n", code[0], code[1], ent.chplan_6g);
				#endif
			}
		}
	}
}

void dump_chplan_ver(void *sel, struct _ADAPTER *padapter)
{
	RTW_PRINT_SEL(sel, "%s%s-%s\n", RTW_DOMAIN_MAP_VER, RTW_DOMAIN_MAP_M_VER, RTW_COUNTRY_MAP_VER);
}
