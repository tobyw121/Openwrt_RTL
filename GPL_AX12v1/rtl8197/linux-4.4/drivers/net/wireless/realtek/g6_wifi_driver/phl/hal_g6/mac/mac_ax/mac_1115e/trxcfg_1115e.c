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
#include "trxcfg_1115e.h"
#include "../mac_priv.h"

#if MAC_AX_1115E_SUPPORT
static u32 _patch_cmac_dma_err_fa_1115e(struct mac_ax_adapter *adapter)
{
#if MAC_AX_8852A_SUPPORT || MAC_AX_8852B_SUPPORT
	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
/*
 * AP  Disable B_AX_STS_FSM_HANG_ERROR_IMR
 * STA Enable  B_AX_STS_FSM_HANG_ERROR_IMR (Wait for "Scan+SER L0")
 */
		if (!chk_ax_patch_cmac_dma_err_fa(adapter))
			return B_AX_RXDATA_FSM_HANG_ERROR_IMR;
	}
#endif
	return 0;
}

static u32 _patch_ss2f_path_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	/*52C&92XB has wrong hw default value*/
	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) || is_chip_id(adapter, MAC_AX_CHIP_ID_8852B))
		return MACSUCCESS;

	val32 = MAC_REG_R32(R_AX_SS2FINFO_PATH);
	val32 = SET_CLR_WORD(val32, SS2F_PATH_WLCPU, B_AX_SS_DEST_QUEUE);
	MAC_REG_W32(R_AX_SS2FINFO_PATH, val32);

	return MACSUCCESS;
}

static u32 sta_sch_init_1115e(struct mac_ax_adapter *adapter,
			      struct mac_ax_trx_info *info)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 val8;
	u32 cnt, ret, val32;

	ret = check_mac_en(adapter, 0, MAC_AX_DMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	val8 = MAC_REG_R8(R_AX_SS_CTRL);
	val8 |= B_AX_SS_EN;
	MAC_REG_W8(R_AX_SS_CTRL, val8);

	cnt = TRXCFG_WAIT_CNT;
	while (cnt--) {
		if (MAC_REG_R32(R_AX_SS_CTRL) & B_AX_SS_INIT_DONE_1)
			break;
		PLTFM_DELAY_US(TRXCFG_WAIT_US);
	}

	if (!++cnt) {
		PLTFM_MSG_ERR("[ERR]STA scheduler init\n");
		return MACPOLLTO;
	}

	MAC_REG_W32(R_AX_SS_CTRL,
		    MAC_REG_R32(R_AX_SS_CTRL) | B_AX_SS_WARM_INIT_FLG);

	val32 = MAC_REG_R32(R_AX_SS_CTRL);
	if (info->trx_mode == MAC_AX_TRX_SW_MODE)
		val32 |= B_AX_SS_NONEMPTY_SS2FINFO_EN;
	else
		val32 &= ~B_AX_SS_NONEMPTY_SS2FINFO_EN;
	MAC_REG_W32(R_AX_SS_CTRL, val32);

	ret = _patch_ss2f_path_1115e(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]_patch_ss2f_path\n");
		return ret;
	}

	return MACSUCCESS;
}

static u32 scheduler_init_1115e(struct mac_ax_adapter *adapter, u8 band)
{
	u32 reg, val32, ret;
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct mac_ax_edca_param edca_para;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	reg = band == MAC_AX_BAND_1 ? R_AX_PREBKF_CFG_1_C1 : R_AX_PREBKF_CFG_1;
	val32 = MAC_REG_R32(reg);
	val32 = SET_CLR_WORD(val32, SIFS_MACTXEN_T1_V0, B_AX_SIFS_MACTXEN_T1);
	MAC_REG_W32(reg, val32);

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
#if MAC_AX_8852B_SUPPORT
		reg = band == MAC_AX_BAND_1 ?
		      R_AX_SCH_EXT_CTRL_C1 : R_AX_SCH_EXT_CTRL;
		val32 = MAC_REG_R32(reg) | B_AX_PORT_RST_TSF_ADV;
		MAC_REG_W32(reg, val32);
#endif
	}

#if MAC_AX_ASIC_TEMP
	reg = band == MAC_AX_BAND_1 ? R_AX_CCA_CFG_0_C1 : R_AX_CCA_CFG_0;
	val32 = MAC_REG_R32(reg) & ~(B_AX_BTCCA_EN);
	MAC_REG_W32(reg, val32);
#endif

#ifdef PHL_FEATURE_AP
	reg = band == MAC_AX_BAND_1 ? R_AX_PREBKF_CFG_0_C1 : R_AX_PREBKF_CFG_0;
	val32 = MAC_REG_R32(reg);
	val32 = SET_CLR_WORD(val32, SCH_PREBKF_16US, B_AX_PREBKF_TIME);
	MAC_REG_W32(reg, val32);

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
#if MAC_AX_8852A_SUPPORT || MAC_AX_8852B_SUPPORT
		reg = band == MAC_AX_BAND_1 ?
		      R_AX_CCA_CFG_0_C1 : R_AX_CCA_CFG_0;
		val32 = MAC_REG_R32(reg);
		val32 = SET_CLR_WORD(val32, 0x6a, B_AX_R_SIFS_AGGR_TIME);
		MAC_REG_W32(reg, val32);
#endif
	} else {
#if MAC_AX_8852C_SUPPORT || MAC_AX_8192XB_SUPPORT || MAC_AX_1115E_SUPPORT
		reg = band == MAC_AX_BAND_1 ?
		      R_AX_CCA_CFG_0_C1 : R_AX_CCA_CFG_0;
		val32 = MAC_REG_R32(reg);
		val32 = SET_CLR_WORD(val32, 0x6a, B_AX_R_SIFS_AGGR_TIME_V1);
		MAC_REG_W32(reg, val32);
#endif
	}
#else /*for NIC mode setting*/
	reg = band == MAC_AX_BAND_1 ? R_AX_PREBKF_CFG_0_C1 : R_AX_PREBKF_CFG_0;
	val32 = MAC_REG_R32(reg);
	val32 = SET_CLR_WORD(val32, SCH_PREBKF_24US, B_AX_PREBKF_TIME);
	MAC_REG_W32(reg, val32);
#endif
	edca_para.band = band;
	edca_para.path = MAC_AX_CMAC_PATH_SEL_BCN;
	edca_para.ecw_min = 2;
	edca_para.ecw_max = 3;
	edca_para.aifs_us = BCN_IFS_25US;
	ret = set_hw_edca_param(adapter, &edca_para);

	return ret;
}

static u32 mpdu_proc_init_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;

	ret = check_mac_en(adapter, 0, MAC_AX_DMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	MAC_REG_W32(R_AX_ACTION_FWD0, TRXCFG_MPDU_PROC_ACT_FRWD);
	MAC_REG_W32(R_AX_TF_FWD, TRXCFG_MPDU_PROC_TF_FRWD);
	val32 = MAC_REG_R32(R_AX_MPDU_PROC);
	val32 |= (B_AX_APPEND_FCS | B_AX_A_ICV_ERR);
	MAC_REG_W32(R_AX_MPDU_PROC, val32);
	MAC_REG_W32(R_AX_CUT_AMSDU_CTRL, TRXCFG_MPDU_PROC_CUT_CTRL);

	return MACSUCCESS;
}

static u32 sec_eng_init_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32 = 0;
	u32 ret;

	ret = check_mac_en(adapter, 0, MAC_AX_DMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	val32 = MAC_REG_R32(R_AX_SEC_ENG_CTRL);
	// init clock
	val32 |= (B_AX_CLK_EN_CGCMP | B_AX_CLK_EN_WAPI | B_AX_CLK_EN_WEP_TKIP);
	// init TX encryption
	val32 |= (B_AX_SEC_TX_ENC | B_AX_SEC_RX_DEC);
	val32 |= (B_AX_MC_DEC | B_AX_BC_DEC);
	val32 |= (B_AX_BMC_MGNT_DEC | B_AX_UC_MGNT_DEC);
	val32 &= ~B_AX_TX_PARTIAL_MODE;
	MAC_REG_W32(R_AX_SEC_ENG_CTRL, val32);

	//init MIC ICV append
	val32 = MAC_REG_R32(R_AX_SEC_MPDU_PROC);
	val32 |= (B_AX_APPEND_ICV | B_AX_APPEND_MIC);

	// option init
	MAC_REG_W32(R_AX_SEC_MPDU_PROC, val32);

	return MACSUCCESS;
}

static u32 tmac_init_1115e(struct mac_ax_adapter *adapter, u8 band,
			   struct mac_ax_trx_info *info)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 reg, val32, ret;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	reg = band == MAC_AX_BAND_1 ? R_AX_MAC_LOOPBACK_C1 : R_AX_MAC_LOOPBACK;
	val32 = MAC_REG_R32(reg);
	if (info->trx_mode == MAC_AX_TRX_LOOPBACK)
		val32 |= B_AX_MACLBK_EN;
	else
		val32 &= ~B_AX_MACLBK_EN;
	MAC_REG_W32(reg, val32);

#if MAC_AX_8852A_SUPPORT || MAC_AX_8852B_SUPPORT || MAC_AX_8852C_SUPPORT || MAC_AX_8192XB_SUPPORT
	reg = band == MAC_AX_BAND_1 ? R_AX_TCR0_C1 : R_AX_TCR0;
	val32 = MAC_REG_R32(reg);
	val32 = SET_CLR_WORD(val32, TCR_UDF_THSD, B_AX_TCR_UDF_THSD);
	MAC_REG_W32(reg, val32);

	reg = band == MAC_AX_BAND_1 ? R_AX_TXD_FIFO_CTRL_C1 : R_AX_TXD_FIFO_CTRL;
	val32 = MAC_REG_R32(reg);
	val32 = SET_CLR_WORD(val32, TXDFIFO_HIGH_MCS_THRE, B_AX_TXDFIFO_HIGH_MCS_THRE);
	val32 = SET_CLR_WORD(val32, TXDFIFO_LOW_MCS_THRE, B_AX_TXDFIFO_LOW_MCS_THRE);
	MAC_REG_W32(reg, val32);
#endif
#if MAC_AX_FPGA_TEST
	reg = band == MAC_AX_BAND_1 ? R_AX_MAC_LOOPBACK_C1 : R_AX_MAC_LOOPBACK;
	val32 = MAC_REG_R32(reg);
	if (info->trx_mode == MAC_AX_TRX_LOOPBACK)
		val32 = SET_CLR_WORD(val32, LBK_PLCP_DLY_FPGA,
				     B_AX_MACLBK_PLCP_DLY);
	else
		val32 = SET_CLR_WORD(val32, LBK_PLCP_DLY_DEF,
				     B_AX_MACLBK_PLCP_DLY);
	MAC_REG_W32(reg, val32);
#endif
	return MACSUCCESS;
}

static u32 trxptcl_init_1115e(struct mac_ax_adapter *adapter, u8 band,
			      struct mac_ax_trx_info *info)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 reg, val32, ret;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	reg = band == MAC_AX_BAND_1 ?
	      R_AX_TRXPTCL_RESP_0_C1 : R_AX_TRXPTCL_RESP_0;
	val32 = MAC_REG_R32(reg);
	val32 = SET_CLR_WORD(val32, WMAC_SPEC_SIFS_CCK,
			     B_AX_WMAC_SPEC_SIFS_CCK);
	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A))
		val32 = SET_CLR_WORD(val32, WMAC_SPEC_SIFS_OFDM_52A,
				     B_AX_WMAC_SPEC_SIFS_OFDM);
	else if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852B))
		val32 = SET_CLR_WORD(val32, WMAC_SPEC_SIFS_OFDM_52B,
				     B_AX_WMAC_SPEC_SIFS_OFDM);
	else
		val32 = SET_CLR_WORD(val32, WMAC_SPEC_SIFS_OFDM_52C,
				     B_AX_WMAC_SPEC_SIFS_OFDM);
	MAC_REG_W32(reg, val32);

	reg = band == MAC_AX_BAND_1 ?
	      R_AX_RXTRIG_TEST_USER_2_C1 : R_AX_RXTRIG_TEST_USER_2;
	val32 = MAC_REG_R32(reg);
	val32 |= B_AX_RXTRIG_FCSCHK_EN;
	MAC_REG_W32(reg, val32);

	return MACSUCCESS;
}

static u32 rmac_init_1115e(struct mac_ax_adapter *adapter, u8 band,
			   struct mac_ax_trx_info *info)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	struct rst_bacam_info rst_info;
	u32 ret;
	u8 val8;
	u16 val16;
	u32 val32, rx_max_len, rx_max_pg, reg;
	u32 rx_min_qta, rx_max_lenb;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	if (band == MAC_AX_BAND_0) {
		rst_info.type = BACAM_RST_ALL;
		rst_info.ent = 0;
		ret = rst_bacam(adapter, &rst_info);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]bacam rst %d\n", ret);
			return ret;
		}
	}

	reg = band == MAC_AX_BAND_1 ?
	      R_AX_RESPBA_CAM_CTRL_C1 : R_AX_RESPBA_CAM_CTRL;
	val8 = MAC_REG_R8(reg) | B_AX_SSN_SEL;
	MAC_REG_W8(reg, val8);

	reg = band == MAC_AX_BAND_1 ?
	      R_AX_DLK_PROTECT_CTL_C1 : R_AX_DLK_PROTECT_CTL;
	val16 = MAC_REG_R16(reg);
	val16 = SET_CLR_WORD(val16, TRXCFG_RMAC_DATA_TO,
			     B_AX_RX_DLK_DATA_TIME);
	val16 = SET_CLR_WORD(val16, TRXCFG_RMAC_CCA_TO,
			     B_AX_RX_DLK_CCA_TIME);
	val16 |= B_AX_RX_DLK_RST_EN;
	MAC_REG_W16(reg, val16);

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
#if MAC_AX_8852A_SUPPORT || MAC_AX_8852B_SUPPORT
		reg = band == MAC_AX_BAND_1 ? R_AX_RCR_C1 : R_AX_RCR;
		val8 = MAC_REG_R8(reg);
		if (band == MAC_AX_BAND_0 &&
		    info->trx_mode == MAC_AX_TRX_SW_MODE)
			val8 = SET_CLR_WORD(val8, 0xF, B_AX_CH_EN);
		else
			val8 = SET_CLR_WORD(val8, 0x1, B_AX_CH_EN);
		MAC_REG_W8(reg, val8);
#endif
	} else {
#if MAC_AX_8852C_SUPPORT || MAC_AX_8192XB_SUPPORT || MAC_AX_1115E_SUPPORT
		reg = band == MAC_AX_BAND_1 ? R_AX_RCR_C1 : R_AX_RCR;
		val8 = MAC_REG_R8(reg);
		if (band == MAC_AX_BAND_0 &&
		    info->trx_mode == MAC_AX_TRX_SW_MODE)
			val8 = (u8)SET_CLR_WORD(val8, 0xF, B_AX_CH_EN_V1);
		else
			val8 = (u8)SET_CLR_WORD(val8, 0x1, B_AX_CH_EN_V1);
		MAC_REG_W8(reg, val8);
#endif
	}
	rx_min_qta = band == MAC_AX_BAND_1 ?
		     adapter->dle_info.c1_rx_qta : adapter->dle_info.c0_rx_qta;
	rx_max_pg = rx_min_qta > PLD_RLS_MAX_PG ? PLD_RLS_MAX_PG : rx_min_qta;
	rx_max_lenb = rx_max_pg * adapter->dle_info.ple_pg_size;
	if (rx_max_lenb < RX_SPEC_MAX_LEN)
		PLTFM_MSG_ERR("[ERR]B%dRX max len %d illegal\n",
			      band, rx_max_lenb);
	else
		rx_max_lenb = RX_SPEC_MAX_LEN;

	/* rx_max_len shall not be larger than B_AX_RX_MPDU_MAX_LEN_MSK */
	rx_max_len = rx_max_lenb / RX_MAX_LEN_UNIT;

	reg = band == MAC_AX_BAND_1 ? R_AX_RX_FLTR_OPT_C1 : R_AX_RX_FLTR_OPT;
	val32 = MAC_REG_R32(reg);
	val32 = SET_CLR_WORD(val32, rx_max_len, B_AX_RX_MPDU_MAX_LEN);
	MAC_REG_W32(reg, val32);

	/* Add drv_info dbg size as dummy (SDIO) */
	if (adapter->hw_info->intf == MAC_AX_INTF_SDIO &&
	    adapter->hw_info->chip_id == MAC_AX_CHIP_ID_8852A) {
		val16 = MAC_REG_R16(R_AX_RCR);
		MAC_REG_W16(R_AX_RCR, val16 |
			    SET_WORD(SDIO_DRV_INFO_SIZE, B_AX_DRV_INFO_SIZE));
	}

	/* NOT ALL vendors calculate VHT SIG-B's CRC */
	reg = band == MAC_AX_BAND_1 ?
	      R_AX_PLCP_HDR_FLTR_C1 : R_AX_PLCP_HDR_FLTR;
	val8 = MAC_REG_R8(reg) & ~B_AX_VHT_SU_SIGB_CRC_CHK;
	MAC_REG_W8(reg, val8);

	return MACSUCCESS;
}

static u32 cmac_com_init_1115e(struct mac_ax_adapter *adapter, u8 band,
			       struct mac_ax_trx_info *info)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	if (band == MAC_AX_BAND_0) {
		val32 = MAC_REG_R32(R_BE_TX_SUB_BAND_VALUE);
		val32 = SET_CLR_WORD(val32, S_BE_TXSB_20M_8, B_BE_TXSB_20M);
		val32 = SET_CLR_WORD(val32, S_BE_TXSB_40M_4, B_BE_TXSB_40M);
		val32 = SET_CLR_WORD(val32, S_BE_TXSB_80M_2, B_BE_TXSB_80M);
		val32 = SET_CLR_WORD(val32, S_BE_TXSB_160M_1, B_BE_TXSB_160M);
		MAC_REG_W32(R_BE_TX_SUB_BAND_VALUE, val32);
	} else {
		val32 = MAC_REG_R32(R_BE_TX_SUB_BAND_VALUE_C1);
		val32 = SET_CLR_WORD(val32, S_BE_TXSB_20M_2, B_BE_TXSB_20M);
		val32 = SET_CLR_WORD(val32, S_BE_TXSB_40M_1, B_BE_TXSB_40M);
		val32 = SET_CLR_WORD(val32, S_BE_TXSB_80M_0, B_BE_TXSB_80M);
		val32 = SET_CLR_WORD(val32, S_BE_TXSB_160M_0, B_BE_TXSB_160M);
		MAC_REG_W32(R_BE_TX_SUB_BAND_VALUE_C1, val32);
	}

	return MACSUCCESS;
}

static u32 ptcl_init_1115e(struct mac_ax_adapter *adapter, u8 band,
			   struct mac_ax_trx_info *info)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	u8 val8;
	u32 reg;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	if (adapter->hw_info->intf == MAC_AX_INTF_PCIE) {
		ret = is_qta_poh(adapter, info->qta_mode, &val8);
		if (ret)
			return ret;
		if (val8) {
			reg = band == MAC_AX_BAND_1 ?
			      R_AX_SIFS_SETTING_C1 : R_AX_SIFS_SETTING;
			val32 = MAC_REG_R32(reg);
			val32 = SET_CLR_WORD(val32, S_AX_CTS2S_TH_1K,
					     B_AX_HW_CTS2SELF_PKT_LEN_TH);
			val32 = SET_CLR_WORD(val32, S_AX_CTS2S_TH_SEC_256B,
					     B_AX_HW_CTS2SELF_PKT_LEN_TH_TWW);
			val32 |= B_AX_HW_CTS2SELF_EN;
			MAC_REG_W32(reg, val32);

			reg = band == MAC_AX_BAND_1 ?
			      R_AX_PTCL_FSM_MON_C1 : R_AX_PTCL_FSM_MON;
			val32 = MAC_REG_R32(reg);
			val32 = SET_CLR_WORD(val32, S_AX_PTCL_TO_2MS,
					     B_AX_PTCL_TX_ARB_TO_THR);
			val32 &= ~B_AX_PTCL_TX_ARB_TO_MODE;
			MAC_REG_W32(reg, val32);
		}
	}

	if (band == MAC_AX_BAND_0) {
		val8 = MAC_REG_R8(R_AX_PTCL_COMMON_SETTING_0);
		if (info->trx_mode == MAC_AX_TRX_SW_MODE) {
			val8 &= ~(B_AX_CMAC_TX_MODE_0 | B_AX_CMAC_TX_MODE_1);
			val8 |= B_AX_PTCL_TRIGGER_SS_EN_0 |
				B_AX_PTCL_TRIGGER_SS_EN_1 |
				B_AX_PTCL_TRIGGER_SS_EN_UL;
		} else {
			val8 |= B_AX_CMAC_TX_MODE_0 | B_AX_CMAC_TX_MODE_1;
			val8 &= ~(B_AX_PTCL_TRIGGER_SS_EN_0 |
				  B_AX_PTCL_TRIGGER_SS_EN_1 |
				  B_AX_PTCL_TRIGGER_SS_EN_UL);
		}
		MAC_REG_W8(R_AX_PTCL_COMMON_SETTING_0, val8);

		val8 = MAC_REG_R8(R_AX_PTCLRPT_FULL_HDL);
		val8 = SET_CLR_WORD(val8, FWD_TO_WLCPU, B_AX_SPE_RPT_PATH);
		MAC_REG_W8(R_AX_PTCLRPT_FULL_HDL, val8);
	} else if (band == MAC_AX_BAND_1) {
		val8 = MAC_REG_R8(R_AX_PTCLRPT_FULL_HDL_C1);
		val8 = SET_CLR_WORD(val8, FWD_TO_WLCPU, B_AX_SPE_RPT_PATH);
		MAC_REG_W8(R_AX_PTCLRPT_FULL_HDL_C1, val8);
	}

	return MACSUCCESS;
}

static u32 cmac_dma_init_1115e(struct mac_ax_adapter *adapter, u8 band)
{
	return MACSUCCESS;
}

static u32 cca_ctrl_init_1115e(struct mac_ax_adapter *adapter, u8 band)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32, reg;
	u32 ret;

	ret = check_mac_en(adapter, band, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS)
		return ret;

	reg = band == MAC_AX_BAND_1 ? R_AX_CCA_CONTROL_C1 : R_AX_CCA_CONTROL;
	val32 = MAC_REG_R32(reg);
	val32 |= (B_AX_TB_CHK_BASIC_NAV | B_AX_TB_CHK_BTCCA |
		  B_AX_TB_CHK_EDCCA | B_AX_TB_CHK_CCA_P20 |
		  B_AX_SIFS_CHK_BTCCA | B_AX_SIFS_CHK_CCA_P20 |
		  B_AX_CTN_CHK_INTRA_NAV |
		  B_AX_CTN_CHK_BASIC_NAV | B_AX_CTN_CHK_BTCCA |
		  B_AX_CTN_CHK_EDCCA | B_AX_CTN_CHK_CCA_S80 |
		  B_AX_CTN_CHK_CCA_S40 | B_AX_CTN_CHK_CCA_S20 |
		  B_AX_CTN_CHK_CCA_P20 | B_AX_SIFS_CHK_EDCCA);

	val32 &= (~B_AX_TB_CHK_TX_NAV & ~B_AX_TB_CHK_CCA_S80 &
		  ~B_AX_TB_CHK_CCA_S40 & ~B_AX_TB_CHK_CCA_S20 &
		  ~B_AX_SIFS_CHK_CCA_S80 & ~B_AX_SIFS_CHK_CCA_S40 &
		  ~B_AX_SIFS_CHK_CCA_S20 & ~B_AX_CTN_CHK_TXNAV);

	MAC_REG_W32(reg, val32);

	return MACSUCCESS;
}

u32 mac_two_nav_cfg_1115e(struct mac_ax_adapter *adapter,
			  struct mac_ax_2nav_info *info)

{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32 = 0;

	val32 = MAC_REG_R32(R_AX_WMAC_NAV_CTL);
	val32 |= ((info->plcp_upd_nav_en ? B_AX_WMAC_PLCP_UP_NAV_EN : 0)) |
		 ((info->tgr_fram_upd_nav_en ? B_AX_WMAC_TF_UP_NAV_EN : 0));

	val32 = SET_CLR_WORD(val32, info->nav_up, B_AX_WMAC_NAV_UPPER);

	if (info->nav_up > NAV_UPPER_DEFAULT)
		val32 |= B_AX_WMAC_NAV_UPPER_EN;

	MAC_REG_W32(R_AX_WMAC_NAV_CTL, val32);
	return MACSUCCESS;
}

static u32 nav_ctrl_init_1115e(struct mac_ax_adapter *adapter, u8 band)
{
	struct mac_ax_2nav_info info;
	u32 ret;

	info.plcp_upd_nav_en = 1;
	info.tgr_fram_upd_nav_en = 1;
	info.nav_up = NAV_12MS;
	ret = mac_two_nav_cfg_1115e(adapter, &info);

	return MACSUCCESS;
}

u32 dmac_init_1115e(struct mac_ax_adapter *adapter, struct mac_ax_trx_info *info,
		    enum mac_ax_band band)
{
	u32 ret = 0;

	ret = dle_init(adapter, info->qta_mode, MAC_AX_QTA_INVALID);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]DLE init %d\n", ret);
		return ret;
	}

	ret = preload_init(adapter, band, info->qta_mode);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]preload init B%d %d\n", band, ret);
		return ret;
	}

	ret = hfc_init(adapter, 1, 1, 1);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]HCI FC init %d\n", ret);
		return ret;
	}

	ret = sta_sch_init_1115e(adapter, info);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]STA SCH init %d\n", ret);
		return ret;
	}

	ret = mpdu_proc_init_1115e(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]MPDU Proc init %d\n", ret);
		return ret;
	}

	ret = sec_eng_init_1115e(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]Security Engine init %d\n", ret);
		return ret;
	}

	return ret;
}

u32 cmac_init_1115e(struct mac_ax_adapter *adapter, struct mac_ax_trx_info *info,
		    enum mac_ax_band band)
{
	u32 ret;

	ret = scheduler_init_1115e(adapter, band);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d SCH init %d\n", band, ret);
		return ret;
	}

	ret = rst_port_info(adapter, band);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d rst port info %d\n", band, ret);
		return ret;
	}

	ret = addr_cam_init(adapter, band);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d ADDR_CAM reset %d\n", band, ret);
		return ret;
	}

	ret = rx_fltr_init(adapter, band);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d RX filter init %d\n", band, ret);
		return ret;
	}

	ret = cca_ctrl_init_1115e(adapter, band);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d CCA CTRL init %d\n", band, ret);
		return ret;
	}

	ret = nav_ctrl_init_1115e(adapter, band);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d NAV CTRL init %d\n", band, ret);
		return ret;
	}

	ret = spatial_reuse_init(adapter, band);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d Spatial Reuse init %d\n", band, ret);
		return ret;
	}

	ret = tmac_init_1115e(adapter, band, info);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d TMAC init %d\n", band, ret);
		return ret;
	}

	ret = trxptcl_init_1115e(adapter, band, info);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d TRXPTCL init %d\n", band, ret);
		return ret;
	}

	ret = rmac_init_1115e(adapter, band, info);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d RMAC init %d\n", band, ret);
		return ret;
	}

	ret = cmac_com_init_1115e(adapter, band, info);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d Com init %d\n", band, ret);
		return ret;
	}

	ret = ptcl_init_1115e(adapter, band, info);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d PTCL init %d\n", band, ret);
		return ret;
	}

	ret = cmac_dma_init_1115e(adapter, band);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d DMA init %d\n", band, ret);
		return ret;
	}

	return ret;
}

u32 mac_enable_imr_1115e(struct mac_ax_adapter *adapter, u8 band,
			 enum mac_ax_hwmod_sel sel)
{
	u32 ret;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	ret = p_ops->ser_imr_config(adapter, band, sel);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]ser_imr_config : %d\n", ret);
		return ret;
	}
	return MACSUCCESS;
}

u32 mac_trx_init_1115e(struct mac_ax_adapter *adapter, struct mac_ax_trx_info *info)
{
	u32 ret = 0;
	struct mac_ax_ops *mac_ops = adapter_to_mac_ops(adapter);
#if MAC_AX_COEX_INIT_EN
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);
#endif
	u8 val8;

	/* Check TRX status is idle later. */
	ret = dmac_init_1115e(adapter, info, MAC_AX_BAND_0);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]DMAC init %d\n", ret);
		return ret;
	}

	ret = cmac_init_1115e(adapter, info, MAC_AX_BAND_0);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]CMAC%d init %d\n", MAC_AX_BAND_0, ret);
		return ret;
	}

#if MAC_AX_COEX_INIT_EN
	ret = p_ops->coex_mac_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] update coex setting %d\n", ret);
		return ret;
	}
#endif

	ret = is_qta_dbcc(adapter, info->qta_mode, &val8);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] is_qta_dbcc %d\n", ret);
		return ret;
	}
	if (val8) {
		ret = mac_ops->dbcc_enable(adapter, info, 1);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]dbcc_enable init %d\n", ret);
			return ret;
		}
	}

	ret = mac_enable_imr_1115e(adapter, MAC_AX_BAND_0, MAC_AX_DMAC_SEL);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] enable DMAC IMR %d\n", ret);
		return ret;
	}

	ret = mac_enable_imr_1115e(adapter, MAC_AX_BAND_0, MAC_AX_CMAC_SEL);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR] enable CMAC0 IMR %d\n", ret);
		return ret;
	}

	ret = mac_err_imr_ctrl(adapter, MAC_AX_FUNC_EN);
	if (ret) {
		PLTFM_MSG_ERR("[ERR] enable err IMR %d\n", ret);
		return ret;
	}

	ret = set_host_rpr(adapter, info->rpr_cfg);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]set host rpr %d\n", ret);
		return ret;
	}

	ret = set_l2_status(adapter);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]%s %d\n", __func__, ret);
		return ret;
	}

	return MACSUCCESS;
}
#endif
