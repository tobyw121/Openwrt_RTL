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
#include "halbb_precomp.h"
#ifdef HALBB_SNIF_SUPPORT

static const u8 sig_b_parsing_dbg_1[] = {
	0x95, //CH1
	0xb0,
	0xca,
	0xad,
	0x8e,
	0x4c,
	0x0d, //CH2
	0x5b,
	0x02,
	0xa8,
	0x00,
	0x00,
	0x00,
}; /*13 Byte*/

static const u8 he_sigb_n_user [256] = {
	9, 8, 8, 7, 8, 7, 7, 6, 		/*0 ~*/
	8, 7, 7, 6, 7, 6, 6, 5, 		/*8 ~*/
	3, 4, 5, 6, 7, 8, 9, 10, 		/*16 ~*/
	3, 4, 5, 6, 7, 8, 9, 10, 		/*24 ~*/
	6, 7, 8, 9,10, 11, 12, 13, 		/*32 ~*/
	5, 6, 7, 8, 9, 10, 11, 12, 		/*40 ~*/
	5, 6, 7, 8, 9, 10, 11, 12, 		/*48 ~*/
	4, 5, 6, 7, 8, 9, 10, 11, 		/*56 ~*/
	6, 7, 8, 9, 10, 11, 12, 13, 		/*64 ~*/
	5, 6, 7, 8, 9, 10, 11, 12, 		/*72 ~*/
	5, 6, 7, 8, 9, 10, 11, 12, 		/*80 ~*/
	4, 5, 6, 7, 8, 9, 10, 11, 		/*88 ~*/
	2, 3, 4, 5, 3, 4, 5, 6, 		/*96 ~*/
	4, 5, 6, 7, 5, 6 ,7, 8, 		/*104 ~*/
	4, 0, 0, 0, 0, 0, 0, 0, 		/*112 ~*/
	0, 0, 0, 0, 0, 0, 0, 0, 		/*120 ~*/
	3, 4, 5, 6, 7, 8, 9, 10,		/*128 ~*/
	4, 5, 6, 7, 8, 9, 10, 11,		/*136 ~*/
	5, 6, 7, 8, 9, 10, 11, 12,		/*144 ~*/
	6, 7, 8, 9, 10, 11, 12,	 13,		/*152 ~*/
	7, 8, 9, 10, 11, 12, 13, 14,		/*160 ~*/
	8, 9, 10, 11, 12, 13, 14, 15,		/*168 ~*/
	9, 10, 11, 12, 13, 14, 15, 16,		/*176 ~*/
	10, 11, 12, 13, 14, 15, 16, 17,	/*184 ~*/
	1, 2, 3, 4, 5, 6, 7, 8,			/*192 ~*/
	1, 2, 3, 4, 5, 6, 7, 8,			/*200 ~*/
	1, 2, 3, 4, 5, 6, 7, 8,			/*208 ~*/
	0, 0, 0, 0, 0, 0, 0, 0,			/*216 ~*/
	0, 0, 0, 0, 0, 0, 0, 0,			/*224 ~*/
	0, 0, 0, 0, 0, 0, 0, 0,			/*232 ~*/
	0, 0, 0, 0, 0, 0, 0, 0,			/*240 ~*/
	0, 0, 0, 0, 0, 0, 0, 0			/*248 ~*/
};

void halbb_sniffer_rpt_reset(struct bb_info *bb)
{
	struct bb_snif_info *snif = &bb->bb_cmn_hooker->bb_snif_i;

	snif->snif_l_sig_len = 0;
	snif->snif_sig_a1_len = 0;
	snif->snif_sig_a2_len = 0;
	snif->snif_sig_b_len = 0;
}

void halbb_sniffer_mode_en(struct bb_info *bb, bool en)
{
	struct bb_snif_info *snif = &bb->bb_cmn_hooker->bb_snif_i;
	struct bb_physts_info *physts = &bb->bb_physts_i;
	u32 u32_tmp = 0;
	u32 snif_ie_bitmap = BIT(IE09_FTR_PLCP_0) | BIT(IE10_FTR_PLCP_EXT);
	u8 i = 0;

	snif->snif_mode_en = en;

	/*Enable IE Pages 9 & 10*/
	for (i = 0; i < PHYSTS_BITMAP_NUM; i++) {
		u32_tmp = halbb_physts_ie_bitmap_get(bb, i) & ~snif_ie_bitmap;

		if (i >= HE_MU) {
			if (en) {
				if (i == HE_MU || i == VHT_MU) {
					u32_tmp |= snif_ie_bitmap;
				} else {
					u32_tmp |= BIT(IE09_FTR_PLCP_0);
				}
			}

			halbb_physts_ie_bitmap_set(bb, i, u32_tmp);
			u32_tmp = halbb_physts_ie_bitmap_get(bb, i);
			physts->bitmap_type[i] = u32_tmp;

			BB_DBG(bb, DBG_SNIFFER, "[IE:%02d] bit_map=0x%08x\n",
			       i, physts->bitmap_type[i]);
		}
	}
}

bool halbb_sniffer_phy_sts_ie_09(struct bb_info *bb)
{
	struct bb_snif_info *snif = &bb->bb_cmn_hooker->bb_snif_i;
	struct bb_physts_info	*physts = &bb->bb_physts_i;
	struct bb_physts_rslt_9_info *psts_9 = &physts->bb_physts_rslt_9_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;

	//BB_DBG(bb, DBG_SNIFFER, "[IE:09] LSIG=0x%06x, SIGA1=0x%08x, SIGA2=0x%08x\n",
	//       psts_9->l_sig, psts_9->sig_a1, psts_9->sig_a2);

	snif->snif_l_sig = &psts_9->l_sig;
	snif->snif_sig_a1 = &psts_9->sig_a1;
	snif->snif_sig_a2 = &psts_9->sig_a2;

	snif->snif_l_sig_len = 17;

	if (cmn_rpt->bb_rate_i.mode == BB_LEGACY_MODE) {
		snif->snif_sig_a1_len = 0;
		snif->snif_sig_a2_len = 0;
	} else if (cmn_rpt->bb_rate_i.mode == BB_HT_MODE ||
		   cmn_rpt->bb_rate_i.mode == BB_VHT_MODE) {
		
		snif->snif_sig_a1_len = 24; /*SIG_A1[23:0]*/
		snif->snif_sig_a2_len = 10; /*SIG_A2[9:0]*/

	} else if (cmn_rpt->bb_rate_i.mode == BB_HE_MODE) {
		snif->snif_sig_a1_len = 26; /*SIG_A1[25:0]*/
		snif->snif_sig_a2_len = 16; /*SIG_A2[15:0]*/
	} else { /*Lgcy*/
		BB_WARNING("[%s] \n", __func__);
		return false;
	}
	return true;
}

bool halbb_sniffer_phy_sts_ie_10_he_mu(struct bb_info *bb)
{
	struct bb_snif_info *snif = &bb->bb_cmn_hooker->bb_snif_i;
	struct bb_physts_info	*physts = &bb->bb_physts_i;
	struct bb_physts_rslt_hdr_info	*psts_h = &physts->bb_physts_rslt_hdr_i;
	struct bb_physts_rslt_1_info	*psts_1 = &physts->bb_physts_rslt_1_i;
	struct bb_physts_rslt_9_info	*psts_9 = &physts->bb_physts_rslt_9_i;
	struct bb_physts_rslt_10_info	*psts_10 = &physts->bb_physts_rslt_10_i;
	struct plcp_hdr_he_sig_a1_mu_info *he_sig_a1_mu = NULL;
	enum channel_width bw = psts_1->bw_idx;
	bool comp_field;
	u8 user_field_len_1user = 31; /*Bit*/
	u8 user_field_len_2user = 52; /*Bit*/
	u8 num_content_ch = (psts_1->bw_idx == CHANNEL_WIDTH_20) ? 1 : 2;
	u32 per_content_len_bit = 0; /*unit: bit*/
	u8 *addr = psts_10->sigb_raw_data_bits_addr;
	u16 len = psts_10->sigb_len;
	u8 user_num_total = 0xff;
	u8 user_num_ch_1 = 0xff, user_num_ch_2 = 0xff;
	u8 pair_block_ch_1 = 0xff, pair_block_ch_2 = 0xff;
	u8 last_block_single_user_ch_1 = 0xff, last_block_single_user_ch_2 = 0xff;
	u64 user_data_tmp64 = 0;
	u32 user_data_tmp32 = 0;
	u8 u_idx = 0;
	u8 i;
	bool is_odd_block;

	//return false; /*For Dbg*/

	if (psts_h->ie_map_type != HE_MU)
		return false;

	/*Get sigb_comp*/
	he_sig_a1_mu = (struct plcp_hdr_he_sig_a1_mu_info *)(&psts_9->sig_a1);
	comp_field = he_sig_a1_mu->sig_b_comp;

	BB_DBG(bb, DBG_SNIFFER, "[HE_MU_SIG_A] comp_field=%d, N_sym or N_user=%d\n",
		comp_field, he_sig_a1_mu->num_sig_b_sym_or_mu_user);

	if (comp_field == 0) {
		//addr += X;
		//user_num_total = he_sigb_n_user[N];
		return false; /*Not Support now*/
	} else {
		user_num_total = he_sig_a1_mu->num_sig_b_sym_or_mu_user - 1;

		if (num_content_ch == 2) {
			user_num_ch_2 = user_num_total >> 1;
			user_num_ch_1 = user_num_total - user_num_ch_2;

			pair_block_ch_2 = user_num_ch_2 >> 1;
			last_block_single_user_ch_2 = pair_block_ch_2 & 0x1;

			pair_block_ch_1 = user_num_ch_1 >> 1;
			last_block_single_user_ch_1 = pair_block_ch_1 & 0x1;
		} else {
			user_num_ch_1 = user_num_total;

			pair_block_ch_1 = user_num_ch_1 >> 1;
			last_block_single_user_ch_1 = pair_block_ch_1 & 0x1;
		}
	}

	BB_DBG(bb, DBG_SNIFFER, "num_content_ch=%d, user_num_all= %d\n",
		num_content_ch, user_num_total);

	BB_DBG(bb, DBG_SNIFFER, "[CH1]user_num=%d, pair_block=%d, last_block_single_user=%d\n",
		user_num_ch_1, pair_block_ch_1, last_block_single_user_ch_1);
	BB_DBG(bb, DBG_SNIFFER, "[CH2]user_num=%d, pair_block=%d, last_block_single_user=%d\n",
		user_num_ch_2, pair_block_ch_2, last_block_single_user_ch_2);

#if 1 //for driver development
	if (pair_block_ch_1 > 8) {
		BB_DBG(bb, DBG_SNIFFER, "Return[1] pair_block_ch_1=%d\n", pair_block_ch_1);
		return false;
	}
	
	if (user_num_total > 16) {
		BB_DBG(bb, DBG_SNIFFER, "Return[1] user_num_total=%d\n", user_num_total);
		return false;
	}
#endif

	if (num_content_ch == 2) {
		per_content_len_bit = len << 2; /* = (Byte*8)/2 */
	} else {
		per_content_len_bit = len << 3; /* = (Byte*8) */
	}

	BB_DBG(bb, DBG_SNIFFER, "Single per_content_len_bit=%d\n",
		per_content_len_bit);

	//return false; /*For Dbg*/

	/*=== 1st content_ch =================================================*/
	/*===Pair Block============*/
	u_idx = 0;
	for (i = 0; i < pair_block_ch_1; i++) {

		if ((i % 2) == 1) {
			is_odd_block = true;
		} else {
			is_odd_block = false;
		} 

		BB_DBG(bb, DBG_SNIFFER, "i=%d, is_odd_block=%d\n",
			i, is_odd_block);

		user_data_tmp64 = *((u64 *)addr);

		snif->plcp_hdr_he_sig_b_i[u_idx].user_idx = u_idx;
		snif->plcp_hdr_he_sig_b_i[u_idx + 1].user_idx = u_idx + 1;

		snif->plcp_hdr_he_sig_b_i[u_idx].is_mu = comp_field;
		snif->plcp_hdr_he_sig_b_i[u_idx + 1].is_mu = comp_field;

		if (is_odd_block) {
			snif->plcp_hdr_he_sig_b_i[u_idx].sig_b_ch1_u.val = (u32)((user_data_tmp64 & 0x1FFFFFF) >> 4);
			snif->plcp_hdr_he_sig_b_i[u_idx + 1].sig_b_ch1_u.val = (u32)((user_data_tmp64 & 0x3FFFFE000000) >> 35);
		} else {
			snif->plcp_hdr_he_sig_b_i[u_idx].sig_b_ch1_u.val = (u32)(user_data_tmp64 & 0x1FFFFF);
			snif->plcp_hdr_he_sig_b_i[u_idx + 1].sig_b_ch1_u.val = (u32)((user_data_tmp64 & 0x3FFFFE00000) >> 31);
		}

		if (i != pair_block_ch_1 - 1)
			u_idx += 2;

		if (is_odd_block)
			addr += 7; /*56 bits*/
		else
			addr += 6; /*48 bits*/

	}

	/*===Last Single Block=====*/
	if (last_block_single_user_ch_1) {
		u_idx++;
		user_data_tmp32 = *((u32 *)addr);

		snif->plcp_hdr_he_sig_b_i[u_idx].user_idx = u_idx;
		snif->plcp_hdr_he_sig_b_i[u_idx].is_mu = comp_field;
		snif->plcp_hdr_he_sig_b_i[u_idx].sig_b_ch1_u.val = user_data_tmp32 >> 11;
	}

#if 1
	for (i = 0; i < user_num_total; i++) {
		BB_DBG(bb, DBG_SNIFFER, "[%d]user_idx=%d, is_mu=%d, val=%d\n",
			i,
			snif->plcp_hdr_he_sig_b_i[u_idx].user_idx,
			snif->plcp_hdr_he_sig_b_i[u_idx].is_mu,
			snif->plcp_hdr_he_sig_b_i[u_idx].sig_b_ch1_u.val);
	}
#endif
	
	if (num_content_ch == 1)
		return true;

	/*=== 2nd content_ch =================================================*/

	return true;
}

bool halbb_sniffer_phy_sts_ie_10(struct bb_info *bb)
{
	struct bb_snif_info *snif = &bb->bb_cmn_hooker->bb_snif_i;
	struct plcp_hdr_vht_sig_b_info *vht_sig_b = &snif->plcp_hdr_vht_sig_b_i;
	struct bb_physts_info	*physts = &bb->bb_physts_i;
	struct bb_physts_rslt_hdr_info	*psts_h = &physts->bb_physts_rslt_hdr_i;
	struct bb_physts_rslt_1_info	*psts_1 = &physts->bb_physts_rslt_1_i;
	struct bb_physts_rslt_10_info	*psts_10 = &physts->bb_physts_rslt_10_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	u32 sigb_tmp = 0;
	u16 sig_b_len_tmp = 0;
	u32 sig_b_len = 0, vht_mcs = 0;
	bool ret = true;

	BB_DBG(bb, DBG_SNIFFER, "[%s]\n", __func__);

	if (cmn_rpt->bb_rate_i.mode == BB_VHT_MODE ||
	    cmn_rpt->bb_rate_i.mode == BB_HE_MODE) {
		snif->snif_sig_b = (u32 *)psts_10->sigb_raw_data_bits_addr;
		snif->snif_sig_b_len = psts_10->sigb_len;

		sigb_tmp = *snif->snif_sig_b;
		sig_b_len_tmp = snif->snif_sig_b_len;
	} else {
		BB_WARNING("[%s] rate_mode=%d\n", __func__, cmn_rpt->bb_rate_i.mode);
		return false;
	}

	if (cmn_rpt->bb_rate_i.mode == BB_VHT_MODE) {
		if (psts_h->ie_map_type == VHT_MU) {
			if (psts_1->bw_idx == CHANNEL_WIDTH_20) {
				vht_sig_b->sigb_len_l = sigb_tmp & 0xffff;
				vht_sig_b->vht_mcs = (sigb_tmp >> 16) & 0xf;
			} else if (psts_1->bw_idx == CHANNEL_WIDTH_40) {
				vht_sig_b->sigb_len_l = sigb_tmp & 0x1ffff;
				vht_sig_b->vht_mcs = (sigb_tmp >> 17) & 0xf;
			} else { /*80, 160, 80+80*/
				vht_sig_b->sigb_len_l = sigb_tmp & 0x7ffff;
				vht_sig_b->vht_mcs = (sigb_tmp >> 19) & 0xf;
			}
		} else { /*su*/
			vht_sig_b->vht_mcs = 0xff;
			if (psts_1->bw_idx == CHANNEL_WIDTH_20) {
				vht_sig_b->sigb_len_l = sigb_tmp & 0x1ffff;
			} else if (psts_1->bw_idx == CHANNEL_WIDTH_40) {
				vht_sig_b->sigb_len_l = sigb_tmp & 0x7ffff;
			} else { /*80, 160, 80+80*/
				vht_sig_b->sigb_len_l = sigb_tmp & 0x1fffff;
			}
		}
		
	} else { /*BB_HE_MODE*/
		ret = halbb_sniffer_phy_sts_ie_10_he_mu(bb);
	}

	BB_DBG(bb, DBG_SNIFFER, "snif_sig_b_len=%d\n", snif->snif_sig_b_len);

	return ret;
}

void halbb_sniffer_phy_sts(struct bb_info *bb, struct physts_result *rpt,
			   u32 physts_bitmap, struct physts_rxd *desc)
{
	struct bb_snif_info *snif = &bb->bb_cmn_hooker->bb_snif_i;
	struct bb_physts_info	*physts = &bb->bb_physts_i;
	struct bb_physts_rslt_hdr_info	*psts_h = &physts->bb_physts_rslt_hdr_i;
	struct bb_physts_rslt_9_info *psts_9 = &physts->bb_physts_rslt_9_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;

	rpt->snif_rpt_valid = false;

	if (!snif->snif_mode_en)
		return;

	if (physts->bb_physts_rslt_hdr_i.ie_map_type <= DL_MU_SPOOFING)
		return;

	if (psts_h->ie_map_type == CCK_PKT)
		return;

	halbb_print_rate_2_buff(bb, desc->data_rate, desc->gi_ltf, bb->dbg_buf, HALBB_SNPRINT_SIZE);

	BB_DBG(bb, DBG_SNIFFER, "[%d][%s] valid=%d, bitmap=0x%08x, Rate= %s (0x%x-%x), rate_mode=%d\n",
	       psts_h->ie_map_type, 
	       bb_physts_bitmap_type_t[psts_h->ie_map_type],
	       rpt->snif_rpt_valid, physts_bitmap, 
	       bb->dbg_buf, desc->data_rate, desc->gi_ltf,
	       cmn_rpt->bb_rate_i.mode);

	halbb_sniffer_rpt_reset(bb);
	snif->snif_rate = desc->data_rate;
	snif->snif_rate_mode = (u8)cmn_rpt->bb_rate_i.mode;
	snif->snif_ie_bitmap_type = psts_h->ie_map_type;	

	if (physts_bitmap & BIT(IE09_FTR_PLCP_0)) {
		rpt->snif_rpt_valid = halbb_sniffer_phy_sts_ie_09(bb);
		BB_DBG(bb, DBG_SNIFFER, "[IE:09] LSIG=0x%06x(%d), SIGA1=0x%08x(%d), SIGA2=0x%08x(%d)\n",
		       *snif->snif_l_sig, snif->snif_l_sig_len,
		       *snif->snif_sig_a1, snif->snif_sig_a1_len,
		       *snif->snif_sig_a2, snif->snif_sig_a2_len);

		if (physts_bitmap & BIT(IE10_FTR_PLCP_EXT))
			rpt->snif_rpt_valid &= halbb_sniffer_phy_sts_ie_10(bb);
	}

	rpt->bb_snif_i = snif;
}

void halbb_sniffer_phy_sts_init(struct bb_info *bb)
{
	struct bb_snif_info *snif = &bb->bb_cmn_hooker->bb_snif_i;

	BB_DBG(bb, DBG_SNIFFER, "[%s] \n", __func__);
}

void halbb_snif_dbg(struct bb_info *bb, char input[][16], u32 *_used,
		    char *output, u32 *_out_len)
{
	u32 val[10] = {0};

	if (_os_strcmp(input[1], "-h") == 0) {
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "en {val}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "ofdma_1 {bss_color} {sta_id}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "vht_mu {PMAC_Ctrl_user_idx_en} {user_idx}\n");
		return;
	}

	if (_os_strcmp(input[1], "en") == 0) {
		HALBB_SCAN(input[2], DCMD_HEX, &val[0]);
		halbb_sniffer_mode_en(bb, (bool)val[0]);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "Sniffer Mode en = %d\n", val[0]);
		return;
	} else if (_os_strcmp(input[1], "ofdma_1") == 0) {
		HALBB_SCAN(input[2], DCMD_HEX, &val[0]);
		HALBB_SCAN(input[3], DCMD_HEX, &val[1]);

		halbb_set_bss_color(bb, (u8)val[0], bb->bb_phy_idx);
		halbb_set_sta_id(bb, (u16)val[1], bb->bb_phy_idx);

		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "Set bss_color=%d, sta_id = %d\n", val[0], val[1]);
	} else if (_os_strcmp(input[1], "vht_mu") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[1]);

		halbb_set_vht_mu_user_idx(bb, (bool)val[0], (u8)val[1], bb->bb_phy_idx);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "Set vht_mu PMAC_Ctrl_en=%d user_idx= %d\n", val[0], val[1]);
	} else {
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "Err\n");
	}
}

#endif
