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
#include "phy_rpt_hv.h"
#include "../mac_reg.h"
#include "../mac_ax/fwcmd.h"
#include "../rxdesc.h"
#include "../mac_ax/trx_desc.h"

#define R_AX_PSPHY_CH_INFO_INIT_L 0x14710
#define R_AX_PSPHY_CH_INFO_INIT_H 0x14714
#define R_AX_PSPHY_PHY_STS 0x14700
#define R_AX_PSPHY_CH_INFO 0x14704

struct ppdu_rxd {
	u16 pkt_size;
	u16 rate;
	u8 ppdu_type;
	u8 gi;
	u8 bw;
	u8 bb_sel;
};

static u32 chk_plcp_cck(struct mac_ax_adapter *adapter,
			struct ppdu_rxd *rxd, struct mac_ax_ppdu_rpt *ppdu)
{
	struct plcp_cck *cck = (struct plcp_cck *)(ppdu->plcp_ptr + 4);
	u8 tmp = 0;

	switch (rxd->rate) {
	case MAC_AX_CCK1:
		tmp = 0xA;
		break;
	case MAC_AX_CCK2:
		tmp = 0x14;
		break;
	case MAC_AX_CCK5_5:
		tmp = 0x37;
		break;
	case MAC_AX_CCK11:
		tmp = 0x6E;
		break;
	default:
		PLTFM_MSG_ERR("wrong cck rate");
		break;
	}

	if (cck->cck_rate != tmp)
		PLTFM_MSG_ERR("cck rate cmp fail");

	return MACSUCCESS;
}

static u32 chk_plcp_ofdm(struct mac_ax_adapter *adapter,
			 struct ppdu_rxd *rxd, struct mac_ax_ppdu_rpt *ppdu)
{
	struct plcp_ofdm *ofdm = (struct plcp_ofdm *)(ppdu->plcp_ptr + 4);
	u8 ofdm_rate = 0;

	if (ofdm->lsig_len != ppdu->lsig_len)
		PLTFM_MSG_ERR("lsig len cmp fail");

	switch (rxd->rate) {
	case MAC_AX_OFDM6:
		ofdm_rate = 0xB;
		break;
	case MAC_AX_OFDM9:
		ofdm_rate = 0xF;
		break;
	case MAC_AX_OFDM12:
		ofdm_rate = 0xA;
		break;
	case MAC_AX_OFDM18:
		ofdm_rate = 0xE;
		break;
	case MAC_AX_OFDM24:
		ofdm_rate = 0x9;
		break;
	case MAC_AX_OFDM36:
		ofdm_rate = 0xD;
		break;
	case MAC_AX_OFDM48:
		ofdm_rate = 0x8;
		break;
	case MAC_AX_OFDM54:
		ofdm_rate = 0xC;
		break;
	default:
		PLTFM_MSG_ERR("wrong ofdm rate");
		break;
	}

	if (ofdm_rate != ofdm->rate)
		PLTFM_MSG_ERR("ofdm rate cmp fail");

	return MACSUCCESS;
}

static u32 chk_plcp_ht(struct mac_ax_adapter *adapter,
		       struct ppdu_rxd *rxd, struct mac_ax_ppdu_rpt *ppdu)
{
	struct plcp_ht *ht = (struct plcp_ht *)(ppdu->plcp_ptr + 4);
	u32 ret = 0;

	if (ht->lsig_len != ppdu->lsig_len)
		PLTFM_MSG_ERR("lsig len cmp fail");

	if (ht->mcs_rate != (rxd->rate & 0x1F))
		PLTFM_MSG_ERR("mcs rate cmp fail");

	if (ht->bw != rxd->bw)
		PLTFM_MSG_ERR("bw cmp fail");

	return MACSUCCESS;
}

static u32 chk_plcp_vhtsu(struct mac_ax_adapter *adapter,
			  struct ppdu_rxd *rxd, struct mac_ax_ppdu_rpt *ppdu)
{
	struct plcp_vht_su *vht = (struct plcp_vht_su *)(ppdu->plcp_ptr + 4);

	if (vht->lsig_len != ppdu->lsig_len)
		PLTFM_MSG_ERR("lsig len cmp fail");

	if (vht->vht_rate != (rxd->rate & 0xF))
		PLTFM_MSG_ERR("mcs rate cmp fail");

	if (vht->bw != rxd->bw)
		PLTFM_MSG_ERR("bw cmp fail");

	return MACSUCCESS;
}

static u32 chk_plcp_vhtmu(struct mac_ax_adapter *adapter,
			  struct ppdu_rxd *rxd, struct mac_ax_ppdu_rpt *ppdu)
{
	struct plcp_vht_mu *vht = (struct plcp_vht_mu *)(ppdu->plcp_ptr + 4);

	if (vht->lsig_len != ppdu->lsig_len)
		PLTFM_MSG_ERR("lsig len cmp fail");

	if (vht->bw != rxd->bw)
		PLTFM_MSG_ERR("bw cmp fail");

	return MACSUCCESS;
}

static u32 chk_plcp_hesu(struct mac_ax_adapter *adapter,
			 struct ppdu_rxd *rxd, struct mac_ax_ppdu_rpt *ppdu)
{
	struct plcp_he_su *he = (struct plcp_he_su *)(ppdu->plcp_ptr + 4);

	if (he->lsig_len != ppdu->lsig_len)
		PLTFM_MSG_ERR("lsig len cmp fail");

	if (he->mcs != (rxd->rate & 0xF))
		PLTFM_MSG_ERR("mcs cmp fail");

	if (he->bw != rxd->bw)
		PLTFM_MSG_ERR("bw cmp fail");

	return MACSUCCESS;
}

static u32 chk_plcp_hemu(struct mac_ax_adapter *adapter,
			 struct ppdu_rxd *rxd, struct mac_ax_ppdu_rpt *ppdu)
{
	struct plcp_he_mu *he = (struct plcp_he_mu *)(ppdu->plcp_ptr + 4);

	if (he->lsig_len != ppdu->lsig_len)
		PLTFM_MSG_ERR("lsig len compare fail");

	if (he->bw != rxd->bw)
		PLTFM_MSG_ERR("bw compare fail");

	return MACSUCCESS;
}

static u32 chk_ppdu_plcp(struct mac_ax_adapter *adapter,
			 struct mac_ax_ppdu_rpt *ppdu, struct ppdu_rxd *rxd)
{
	struct plcp_hdr *hdr = (struct plcp_hdr *)(ppdu->plcp_ptr);
	u32 ret = 0;

	if (hdr->ppdu_type != rxd->ppdu_type)
		return 1;

	switch (hdr->ppdu_type) {
	case PPDU_TYPE_LCCK:
	case PPDU_TYPE_SCCK:
		ret = chk_plcp_cck(adapter, rxd, ppdu);
		break;
	case PPDU_TYPE_OFDM:
		ret = chk_plcp_ofdm(adapter, rxd, ppdu);
		break;
	case PPDU_TYPE_HT:
	case PPDU_TYPE_HTGF:
		ret = chk_plcp_ht(adapter, rxd, ppdu);
		break;
	case PPDU_TYPE_VHTSU:
		ret = chk_plcp_vhtsu(adapter, rxd, ppdu);
		break;
	case PPDU_TYPE_VHTMU:
		ret = chk_plcp_vhtmu(adapter, rxd, ppdu);
		break;
	case PPDU_TYPE_HESU:
	case PPDU_TYPE_HEERSEU:
		ret = chk_plcp_hesu(adapter, rxd, ppdu);
		break;
	case PPDU_TYPE_HETB:
		break;
	case PPDU_TYPE_HEMU:
		ret = chk_plcp_hemu(adapter, rxd, ppdu);
		break;
	default:
		PLTFM_MSG_ERR("wrong data rate\n");
		ret = 1;
		break;
	}

	return ret;
}

static u32 chk_ps_dfs(struct mac_ax_adapter *adapter,
		      struct mac_ax_dfs_rpt *dfs)
{
	u64 *dfs_rpt = (u64 *)dfs->dfs_ptr;
	u64 prev_rpt;
	u64 curr_rpt;
	u32 i;
	u32 drop_num = 0;
	u32 tmp;
	u32 last_tmp;
	u32 diff_rpt;
	u32 diff_seq;
	u32 ret = 0;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val = MAC_REG_R32(R_AX_DFS_CFG0);
	u8 th = GET_FIELD(val, B_AX_DFS_NUM_TH);
	u8 time = GET_FIELD(val, B_AX_DFS_TIME_TH);
	u8 en = val & B_AX_DFS_RPT_EN;

	for (i = 0; i < (u32)(dfs->dfs_num - 1); i++) {
		prev_rpt = *dfs_rpt & 0x1FFFFFFFFFFFFFF;
		last_tmp = (*dfs_rpt >> 57) & 0x7F;
		dfs_rpt++;
		curr_rpt = *(dfs_rpt) & 0x1FFFFFFFFFFFFFF;
		tmp = (*dfs_rpt >> 57) & 0x7F;
		if (curr_rpt > prev_rpt)
			diff_rpt = (u32)(curr_rpt - prev_rpt);
		else
			diff_rpt = (u32)(curr_rpt +
					 (0x1FFFFFFFFFFFFFF - prev_rpt) + 1);
		if (diff_rpt > 1) {
			PLTFM_MSG_ERR("rpt num is not consecutive!\n");
			PLTFM_MSG_ERR("last = %lx, curr = %lx\n",
				      prev_rpt, curr_rpt);
		}

		if (tmp > last_tmp)
			diff_seq = tmp - last_tmp;
		else
			diff_seq = tmp + (0x7F - last_tmp) + 1;
		if (diff_seq > 1) {
			PLTFM_MSG_ERR("seq num is not consecutive!\n");
			PLTFM_MSG_ERR("last = %x, curr = %x\n", last_tmp, tmp);
		}

		if (diff_rpt < 0x7F && diff_rpt != diff_seq) {
			PLTFM_MSG_ERR("seq num and DFS is not consisten!\n");
			PLTFM_MSG_ERR("seq diff = %d, rpt diff = %d\n",
				      diff_seq, diff_rpt);
			ret = 1;
			goto END;
		}

		if (diff_rpt > 1)
			drop_num += diff_rpt - 1;
	}

	prev_rpt = *(u64 *)dfs->dfs_ptr & 0x1FFFFFFFFFFFFFF;
	if (curr_rpt > prev_rpt)
		diff_rpt = (u32)(curr_rpt - prev_rpt);
	else
		diff_rpt = (u32)(curr_rpt + (0x1FFFFFFFFFFFFFF - prev_rpt));

	if (diff_rpt + 1 != dfs->total_drop + dfs->dfs_num) {
		PLTFM_MSG_ERR("rpt num is incorrect!\n");
		PLTFM_MSG_ERR("rpt: %d, drop: %d, received: %d",
			      diff_rpt + 1, dfs->total_drop, dfs->dfs_num);
	}

	if (!time && en) {
		switch (th) {
		case 0:
			if (dfs->dfs_num != 29) {
				PLTFM_MSG_ERR("rpt num != CR config!\n");
				PLTFM_MSG_ERR("rpt: %d, received: %d",
					      29, dfs->dfs_num);
				ret = 1;
			}
			break;
		case 1:
			if (dfs->dfs_num != 61) {
				PLTFM_MSG_ERR("rpt num != CR config!\n");
				PLTFM_MSG_ERR("rpt: %d, received: %d",
					      61, dfs->dfs_num);

				ret = 1;
			}
			break;
		case 2:
			if (dfs->dfs_num != 93) {
				PLTFM_MSG_ERR("rpt num != CR config!\n");
				PLTFM_MSG_ERR("rpt: %d, received: %d",
					      93, dfs->dfs_num);
				ret = 1;
			}
			break;
		case 3:
			if (dfs->dfs_num != 125) {
				PLTFM_MSG_ERR("rpt num != CR config!\n");
				PLTFM_MSG_ERR("rpt: %d, received: %d",
					      125, dfs->dfs_num);
				ret = 1;
			}
			break;
		}
	}

END:
	return ret;
}

u32 hv_chk_ps_dfs(struct mac_ax_adapter *adapter, u8 *data, u32 len)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_ops *mac_ops = adapter_to_mac_ops(adapter);
	struct mac_ax_dfs_rpt dfs;
	struct mac_ax_rxpkt_info info;
	u32 ret = 0;

	ret = mac_ops->parse_rxdesc(adapter, &info, data, len);
	if (ret != MACSUCCESS)
		goto END;

	if (info.type != MAC_AX_PKT_DFS) {
		ret = MACFUNCINPUT;
		goto END;
	}

	ret = mac_ops->parse_dfs(adapter, data, len, &dfs);
	if (ret != MACSUCCESS)
		goto END;

	ret = chk_ps_dfs(adapter, &dfs);

END:
	return ret;
}

static u32 chk_ps_phy_sts(struct mac_ax_adapter *adapter,
			  struct mac_ax_ppdu_rpt *ppdu)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 curr_cnt = 0;
	u8 last_cnt;
	u8 *ppdu_ptr = ppdu->phy_st_ptr + 16;
	u32 i, val;
	s8 j;
	u32 ret = 0;

	/* assume the phy0 & phy1 settings are the same*/
	val = MAC_REG_R8(R_AX_PSPHY_PHY_STS + 1);

	/* phy status header = 16 bytes */
	if (val * 8 + 16 != ppdu->phy_st_size) {
		PLTFM_MSG_ERR("phy len is wrong\n");
		PLTFM_MSG_ERR("received: %x, expected: %x\n",
			      ppdu->phy_st_size, val * 8 + 16);
	}

	for (i = 0; i < (ppdu->phy_st_size - 16 - 8); i = i + 8) {
		for (j = 7; j >= 0; j--) {
			last_cnt = curr_cnt;
			curr_cnt = ppdu_ptr[i + j];
			if ((i > 0 || j != 7) &&
			    curr_cnt != (u8)(last_cnt + 1)) {
				PLTFM_MSG_ERR("phy data is not consecutive\n");
				PLTFM_MSG_ERR("last: %x, curr: %x\n",
					      last_cnt, curr_cnt);
				ret = 1;
				goto END;
			}
		}
	}
END:
	return ret;
}

static u32 parse_ppdu_rx_desc(u8 *data, struct ppdu_rxd *ppdu)
{
	if (GET_FIELD(*(u32 *)data, AX_RXD_RPKT_TYPE) != RXD_S_RPKT_TYPE_PPDU)
		return 1;
	ppdu->pkt_size = GET_FIELD(*(u32 *)data, AX_RXD_RPKT_LEN);
	ppdu->rate = GET_FIELD(*((u32 *)data + 1), AX_RXD_RX_DATARATE);
	ppdu->ppdu_type = GET_FIELD(*((u32 *)data + 1), AX_RXD_PPDU_TYPE);
	ppdu->gi = GET_FIELD(*((u32 *)data + 1), AX_RXD_RX_GI_LTF);
	ppdu->bw = GET_FIELD(*((u32 *)data + 1), AX_RXD_BW);
	ppdu->bb_sel = !!(*((u32 *)data + 1) & AX_RXD_BB_SEL);

	return MACSUCCESS;
}

static u32 chk_ppdu(struct mac_ax_adapter *adapter,
		    struct mac_ax_ppdu_rpt *ppdu, struct ppdu_rxd *rxd)
{
	u32 ret = 0;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 reg = (rxd->bb_sel == MAC_AX_BAND_0) ?
			R_AX_PPDU_STAT : R_AX_PPDU_STAT_C1;
	u8 val = MAC_REG_R8(reg);

	if (val & B_AX_APP_RX_CNT_RPT && !ppdu->rx_cnt_size)
		PLTFM_MSG_ERR("rx counter is NOT appended\n");
	if (val & B_AX_APP_PLCP_HDR_RPT && !ppdu->plcp_size)
		PLTFM_MSG_ERR("plcp header is NOT appended\n");
	if (ppdu->plcp_size) {
		ret = chk_ppdu_plcp(adapter, ppdu, rxd);
		if (ret != MACSUCCESS)
			goto END;
	}

	if (ppdu->phy_st_size)
		ret = chk_ps_phy_sts(adapter, ppdu);
END:
	return ret;
}

u32 hv_chk_ps_ppdu(struct mac_ax_adapter *adapter, u8 *data, u32 len)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_ops *mac_ops = adapter_to_mac_ops(adapter);
	struct mac_ax_ppdu_rpt ppdu;
	struct mac_ax_rxpkt_info info;
	u32 ret = 0;
	struct ppdu_rxd rxd;

	ret = mac_ops->parse_rxdesc(adapter, &info, data, len);
	if (ret != MACSUCCESS)
		goto END;

	if (info.type != MAC_AX_PKT_PPDU) {
		ret = MACFUNCINPUT;
		goto END;
	}

	ret = mac_ops->parse_ppdu(adapter, data, len,
				  info.u.ppdu.mac_info, &ppdu);
	if (ret != MACSUCCESS)
		goto END;

	ret = parse_ppdu_rx_desc(data, &rxd);
	if (ret != MACSUCCESS)
		goto END;

	ret = chk_ppdu(adapter, &ppdu, &rxd);
END:
	return ret;
}

u32 hv_get_ppdu(struct mac_ax_adapter *adapter, enum mac_ax_band band)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 reg = (band == MAC_AX_BAND_0) ?
			R_AX_PPDU_STAT : R_AX_PPDU_STAT_C1;

	MAC_REG_W8(reg + 1, MAC_REG_R8(reg + 1) | BIT(0));

	return MACSUCCESS;
}

u32 c2h_test_phy_rpt(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		     struct rtw_c2h_info *c2h_info)
{
	u32 hdr1;
	u32 c2h_len;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_ops *mac_ops = adapter_to_mac_ops(adapter);
	struct mac_ax_rxpkt_info info;
	u32 ret = 0;

	ret = mac_ops->parse_rxdesc(adapter, &info, buf + FWCMD_HDR_LEN, len);
	if (ret != MACSUCCESS)
		goto END;

	hdr1 = ((struct fwcmd_hdr *)buf)->hdr1;
	hdr1 = le32_to_cpu(hdr1);

	c2h_len = GET_FIELD(hdr1, C2H_HDR_TOTAL_LEN);

	switch (info.type) {
	case MAC_AX_PKT_PPDU:
		break;
	case MAC_AX_PKT_CH_INFO:
		ret = hv_chk_ps_ch_info(adapter, buf + FWCMD_HDR_LEN,
					c2h_len - FWCMD_HDR_LEN);
		break;
	case MAC_AX_PKT_DFS:
		ret = hv_chk_ps_dfs(adapter, buf + FWCMD_HDR_LEN,
				    c2h_len - FWCMD_HDR_LEN);
		break;
	default:
		PLTFM_MSG_ERR("Wrong C2H ID\n");
		ret = MACFUNCINPUT;
		break;
	}

END:
	return ret;
}

u32 hv_chk_ps_ch_info(struct mac_ax_adapter *adapter, u8 *buf, u32 len)
{
	static u16 ch_info_cnt;
	static u16 seg_num;
	u64 curr_val, last_val;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_ops *mac_ops = adapter_to_mac_ops(adapter);
	u64 init_val;
	u8 *data = buf + RXD_SHORT_LEN + 8;
	u32 i, seg_size = 0;
	u8 val;

	curr_val = *((u64 *)data);
	init_val = (u64)MAC_REG_R32(R_AX_PSPHY_CH_INFO_INIT_L) |
		(u64)MAC_REG_R32(R_AX_PSPHY_CH_INFO_INIT_H) << 32;
	if (init_val == curr_val) {
		if (ch_info_cnt != 0 && ch_info_cnt != seg_num)
			PLTFM_MSG_ERR("some segments are lost\n");
		seg_num = MAC_REG_R8(R_AX_PSPHY_CH_INFO + 2);
		ch_info_cnt = 0;
	}

	val = MAC_REG_R8(R_AX_PSPHY_CH_INFO + 1) >> 4;
	switch (val) {
	case 0:
		seg_size = 96;
		break;
	case 1:
		seg_size = 224;
		break;
	case 2:
		seg_size = 480;
		break;
	case 3:
		seg_size = 992;
		break;
	}

	if (seg_size != len - RXD_SHORT_LEN)
		PLTFM_MSG_ERR("wrong ch info size\n");

	ch_info_cnt++;
	for (i = 8; i < len - RXD_SHORT_LEN - 8; i += 8) {
		last_val = curr_val;
		curr_val = *((u64 *)(data + i));
		if (curr_val != last_val + 1) {
			PLTFM_MSG_ERR("data contents are not consecutive\n");
			PLTFM_MSG_ERR("last = %x, curr = %x\n",
				      last_val, curr_val);
		}
	}

	return MACSUCCESS;
}
