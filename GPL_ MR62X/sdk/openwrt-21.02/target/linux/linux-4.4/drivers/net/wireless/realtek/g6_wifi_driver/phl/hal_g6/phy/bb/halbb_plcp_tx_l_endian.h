/******************************************************************************
 *
 * Copyright(c) 2007 - 2020  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/
#ifndef __HALBB_PLCP_TX_L_ENDIAN_H__
#define __HALBB_PLCP_TX_L_ENDIAN_H__

 /*@--------------------------[Define] ---------------------------------------*/
#define DL_STA_LIST_MAX_NUM    8
 /*@--------------------------[Enum]------------------------------------------*/

 /*@--------------------------[Structure]-------------------------------------*/
 struct cr_address_t {
	u8 address_0;
	u8 address_1;
	u8 address_2;
	u8 address_3;
};

struct sigb_usr_info {
    	u8 coding;
	u8 nsts;
	u8 ru_position;
	u8 dcm;
    	u8 mcs;
	u8 rsvd;
};

struct sigb_info {
	u8 ppdu_bw;
	u8 sta_list_num;
	u8 rsvd1;
	u8 rsvd2;
	struct sigb_usr_info usr_info[DL_STA_LIST_MAX_NUM];
};

struct bb_h2c_he_sigb {
	struct sigb_info sigb_i;
	struct cr_address_t n_sym_sigb_ch1[16];
	struct cr_address_t n_sym_sigb_ch2[16];
};
 /*@--------------------------[Prptotype]-------------------------------------*/
#endif
