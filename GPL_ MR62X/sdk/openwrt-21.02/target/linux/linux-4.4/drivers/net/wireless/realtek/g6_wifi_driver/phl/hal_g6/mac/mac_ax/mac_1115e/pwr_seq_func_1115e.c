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
 * v000
 ******************************************************************************/

#include "../pwr.h"
#include "../pwr_seq_func.h"
#if MAC_AX_1115E_SUPPORT
u32 mac_pwr_on_sdio_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	u8 val8;

	/* 0x4086[0] = 0 == 0x4084[16] = 0 */
	val32 = MAC_REG_R32(R_BE_SDIO_BUS_CTRL);
	MAC_REG_W32(R_BE_SDIO_BUS_CTRL, val32 & ~B_BE_HCI_SUS_REQ);

	/* polling 0x4086[1] = 1 */
	ret = pwr_poll_u32(adapter, R_BE_SDIO_BUS_CTRL, B_BE_HCI_RESUME_RDY,
			   B_BE_HCI_RESUME_RDY);
	if (ret)
		return ret;

	/* 0x04[12:11] = 2'b00 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~(B_BE_AFSM_WLSUS_EN |
			B_BE_AFSM_PCIE_SUS_EN));

	/* 0x04[15] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~B_BE_APDM_HPDN);

	/* 0x04[10] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~B_BE_APFM_SWLPS);

	/* polling 0x04[17] = 1*/
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_RDY_SYSPWR,
			   B_BE_RDY_SYSPWR);
	if (ret)
		return ret;

	/* 0x04[16] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_EN_WLON);

	/* 0x04[8] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_APFN_ONMAC);

	/* polling 0x04[8] = 0 */
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_APFN_ONMAC, 0);
	if (ret)
		return ret;

	/* 0x88[0] = 1 */
	val8 = MAC_REG_R8(R_BE_PLATFORM_ENABLE);
	MAC_REG_W8(R_BE_PLATFORM_ENABLE, val8 | B_BE_PLATFORM_EN);
	val8 = MAC_REG_R8(R_BE_PLATFORM_ENABLE);

	/* CMAC1 Power off */
	/* 0x80[30] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 & ~B_BE_CMAC1_FEN);

	/* 0x80[5] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_CMAC12PP);

	/* 0x24[4:0] = 0 */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1,
		    val32 & ~B_BE_R_SYM_WLCMAC1_P4_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P3_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P2_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P1_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_PC_EN);

	/*enable dmac , 0x8400*/
	val32 = MAC_REG_R32(R_BE_DMAC_FUNC_EN);
	MAC_REG_W32(R_BE_DMAC_FUNC_EN,
		    val32 | B_BE_MAC_EN |
			B_BE_DMAC_EN |
			B_BE_MPDU_EN |
			B_BE_WDRLS_EN |
			B_BE_WDE_DLE_EN |
			B_BE_TXPKTCTL_EN |
			B_BE_STA_SCH_EN |
			B_BE_PLE_DEL_EN |
			B_BE_PKTBUF_EN |
			B_BE_DMAC_TABLE_EN |
			B_BE_PKTIN_EN |
			B_BE_DLE_CPUIO_EN |
			B_BE_DISPATCHER_EN |
			B_BE_BBRPT_EN |
			B_BE_WSEC_EN |
			B_BE_FORCE_DMAREG_GCKEN |
			B_BE_H_AXIDMA_EN);

	/*enable cmac , 0xC000*/
	val32 = MAC_REG_R32(R_BE_CMAC_FUNC_EN);
	MAC_REG_W32(R_BE_CMAC_FUNC_EN,
		    val32 | B_BE_CMAC_EN |
			B_BE_CMAC_TXEN |
			B_BE_CMAC_RXEN |
			B_BE_FORCE_CMACREG_GCKEN |
			B_BE_PHYINTF_EN |
			B_BE_CMAC_DMA_EN |
			B_BE_PTCLTOP_EN |
			B_BE_SCHEDULER_EN |
			B_BE_TMAC_EN |
			B_BE_RMAC_EN);

	return MACSUCCESS;
}

u32 mac_pwr_on_usb_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	u8 val8;

	/* 0x04[12:11] = 2'b00 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~(B_BE_AFSM_WLSUS_EN |
			B_BE_AFSM_PCIE_SUS_EN));

	/* 0x04[15] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~B_BE_APDM_HPDN);

	/* 0x04[10] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~B_BE_APFM_SWLPS);

	/* polling 0x04[17] = 1*/
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_RDY_SYSPWR,
			   B_BE_RDY_SYSPWR);
	if (ret)
		return ret;

	/* 0x04[16] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_EN_WLON);

	/* 0x04[8] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_APFN_ONMAC);

	/* polling 0x04[8] = 0 */
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_APFN_ONMAC, 0);
	if (ret)
		return ret;

	/* 0x88[0] = 1 */
	val8 = MAC_REG_R8(R_BE_PLATFORM_ENABLE);
	MAC_REG_W8(R_BE_PLATFORM_ENABLE, val8 | B_BE_PLATFORM_EN);
	val8 = MAC_REG_R8(R_BE_PLATFORM_ENABLE);

	/* CMAC1 Power off */
	/* 0x80[30] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 & ~B_BE_CMAC1_FEN);

	/* 0x80[5] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_CMAC12PP);

	/* 0x24[4:0] = 0 */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1,
		    val32 & ~B_BE_R_SYM_WLCMAC1_P4_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P3_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P2_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P1_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_PC_EN);

	/*enable dmac , 0x8400*/
	val32 = MAC_REG_R32(R_BE_DMAC_FUNC_EN);
	MAC_REG_W32(R_BE_DMAC_FUNC_EN,
		    val32 | B_BE_MAC_EN |
			B_BE_DMAC_EN |
			B_BE_MPDU_EN |
			B_BE_WDRLS_EN |
			B_BE_WDE_DLE_EN |
			B_BE_TXPKTCTL_EN |
			B_BE_STA_SCH_EN |
			B_BE_PLE_DEL_EN |
			B_BE_PKTBUF_EN |
			B_BE_DMAC_TABLE_EN |
			B_BE_PKTIN_EN |
			B_BE_DLE_CPUIO_EN |
			B_BE_DISPATCHER_EN |
			B_BE_BBRPT_EN |
			B_BE_WSEC_EN |
			B_BE_FORCE_DMAREG_GCKEN |
			B_BE_H_AXIDMA_EN);

	/*enable cmac , 0xC000*/
	val32 = MAC_REG_R32(R_BE_CMAC_FUNC_EN);
	MAC_REG_W32(R_BE_CMAC_FUNC_EN,
		    val32 | B_BE_CMAC_EN |
			B_BE_CMAC_TXEN |
			B_BE_CMAC_RXEN |
			B_BE_FORCE_CMACREG_GCKEN |
			B_BE_PHYINTF_EN |
			B_BE_CMAC_DMA_EN |
			B_BE_PTCLTOP_EN |
			B_BE_SCHEDULER_EN |
			B_BE_TMAC_EN |
			B_BE_RMAC_EN);

	return MACSUCCESS;
}

#ifdef PHL_FEATURE_AP
u32 mac_pwr_on_ap_pcie_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	u8 val8;

	/* 0x04[12:11] = 2'b00 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~(B_BE_AFSM_WLSUS_EN |
			B_BE_AFSM_PCIE_SUS_EN));

	/* 0x04[15] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~B_BE_APDM_HPDN);

	/* 0x04[10] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~B_BE_APFM_SWLPS);

	/* polling 0x04[17] = 1*/
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_RDY_SYSPWR,
			   B_BE_RDY_SYSPWR);
	if (ret)
		return ret;

	/* 0x04[16] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_EN_WLON);

	/* 0x04[8] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_APFN_ONMAC);

	/* polling 0x04[8] = 0 */
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_APFN_ONMAC, 0);
	if (ret)
		return ret;

	/* 0x88[0] = 1 */
	val8 = MAC_REG_R8(R_BE_PLATFORM_ENABLE);
	MAC_REG_W8(R_BE_PLATFORM_ENABLE, val8 | B_BE_PLATFORM_EN);
	val8 = MAC_REG_R8(R_BE_PLATFORM_ENABLE);

	/* CMAC1 Power off */
	/* 0x80[30] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 & ~B_BE_CMAC1_FEN);

	/* 0x80[5] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_CMAC12PP);

	/* 0x24[4:0] = 0 */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1,
		    val32 & ~B_BE_R_SYM_WLCMAC1_P4_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P3_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P2_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P1_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_PC_EN);

	/*enable dmac , 0x8400*/
	val32 = MAC_REG_R32(R_BE_DMAC_FUNC_EN);
	MAC_REG_W32(R_BE_DMAC_FUNC_EN,
		    val32 | B_BE_MAC_EN |
			B_BE_DMAC_EN |
			B_BE_MPDU_EN |
			B_BE_WDRLS_EN |
			B_BE_WDE_DLE_EN |
			B_BE_TXPKTCTL_EN |
			B_BE_STA_SCH_EN |
			B_BE_PLE_DEL_EN |
			B_BE_PKTBUF_EN |
			B_BE_DMAC_TABLE_EN |
			B_BE_PKTIN_EN |
			B_BE_DLE_CPUIO_EN |
			B_BE_DISPATCHER_EN |
			B_BE_BBRPT_EN |
			B_BE_WSEC_EN |
			B_BE_FORCE_DMAREG_GCKEN |
			B_BE_H_AXIDMA_EN);

	/*enable cmac , 0xC000*/
	val32 = MAC_REG_R32(R_BE_CMAC_FUNC_EN);
	MAC_REG_W32(R_BE_CMAC_FUNC_EN,
		    val32 | B_BE_CMAC_EN |
			B_BE_CMAC_TXEN |
			B_BE_CMAC_RXEN |
			B_BE_FORCE_CMACREG_GCKEN |
			B_BE_PHYINTF_EN |
			B_BE_CMAC_DMA_EN |
			B_BE_PTCLTOP_EN |
			B_BE_SCHEDULER_EN |
			B_BE_TMAC_EN |
			B_BE_RMAC_EN);

	return MACSUCCESS;
}
#else
u32 mac_pwr_on_nic_pcie_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	u8 val8;

	/* 0x04[12:11] = 2'b00 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~(B_BE_AFSM_WLSUS_EN |
			B_BE_AFSM_PCIE_SUS_EN));

	/* 0x04[15] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~B_BE_APDM_HPDN);

	/* 0x04[10] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~B_BE_APFM_SWLPS);

	/* polling 0x04[17] = 1*/
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_RDY_SYSPWR,
			   B_BE_RDY_SYSPWR);
	if (ret)
		return ret;

	/* 0x04[16] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_EN_WLON);

	/* 0x04[8] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_APFN_ONMAC);

	/* polling 0x04[8] = 0 */
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_APFN_ONMAC, 0);
	if (ret)
		return ret;

	/* Platform Enable */
	/* 0x88[0] = 1 */
	val8 = MAC_REG_R8(R_BE_PLATFORM_ENABLE);
	MAC_REG_W8(R_BE_PLATFORM_ENABLE, val8 | B_BE_PLATFORM_EN);
	val8 = MAC_REG_R8(R_BE_PLATFORM_ENABLE);

	/* 0x70[12] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_SDIO_CTRL);
	MAC_REG_W32(R_BE_SYS_SDIO_CTRL, val32 & ~B_BE_PCIE_FORCE_IBX_EN_V1);

	/* CMAC1 Power off */
	/* 0x80[30] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 & ~B_BE_CMAC1_FEN);

	/* 0x80[5] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_CMAC12PP);

	/* 0x24[4:0] = 0 */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1,
		    val32 & ~B_BE_R_SYM_WLCMAC1_P4_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P3_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P2_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_P1_PC_EN &
		    ~B_BE_R_SYM_WLCMAC1_PC_EN);

	/*enable dmac , 0x8400*/
	val32 = MAC_REG_R32(R_BE_DMAC_FUNC_EN);
	MAC_REG_W32(R_BE_DMAC_FUNC_EN,
		    val32 | B_BE_MAC_EN |
			B_BE_DMAC_EN |
			B_BE_MPDU_EN |
			B_BE_WDRLS_EN |
			B_BE_WDE_DLE_EN |
			B_BE_TXPKTCTL_EN |
			B_BE_STA_SCH_EN |
			B_BE_PLE_DEL_EN |
			B_BE_PKTBUF_EN |
			B_BE_DMAC_TABLE_EN |
			B_BE_PKTIN_EN |
			B_BE_DLE_CPUIO_EN |
			B_BE_DISPATCHER_EN |
			B_BE_BBRPT_EN |
			B_BE_WSEC_EN |
			B_BE_FORCE_DMAREG_GCKEN |
			B_BE_H_AXIDMA_EN);

	/*enable cmac , 0xC000*/
	val32 = MAC_REG_R32(R_BE_CMAC_FUNC_EN);
	MAC_REG_W32(R_BE_CMAC_FUNC_EN,
		    val32 | B_BE_CMAC_EN |
			B_BE_CMAC_TXEN |
			B_BE_CMAC_RXEN |
			B_BE_FORCE_CMACREG_GCKEN |
			B_BE_PHYINTF_EN |
			B_BE_CMAC_DMA_EN |
			B_BE_PTCLTOP_EN |
			B_BE_SCHEDULER_EN |
			B_BE_TMAC_EN |
			B_BE_RMAC_EN);

	return MACSUCCESS;
}
#endif

u32 mac_pwr_off_sdio_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u8 val8;
	u32 ret;

	/* 0x04[16] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_EN_WLON);

	/* 0x02[1:0] = 0 */
	val8 = MAC_REG_R8(R_BE_SYS_FUNC_EN);
	MAC_REG_W8(R_BE_SYS_FUNC_EN,
		   val8 & ~B_BE_FEN_BB_GLB_RSTN &
		   ~B_BE_FEN_BBRSTB);

	/* 0x82[1:0] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND,
		    val32 & ~B_BE_R_SYM_FEN_WLBBGLB_1 &
		    ~B_BE_R_SYM_FEN_WLBBFUN_1);

	/* 0x04[9] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_APFM_OFFMAC);

	/* polling 0x04[9] = 0 */
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_APFM_OFFMAC, 0);
	if (ret)
		return ret;

	/* 0x04[28][30] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~B_BE_SOP_EDSWR &
			~B_BE_SOP_EASWR);

	/* 0x04[22] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_XTAL_OFF_A_DIE);

	/* 0x04[12:11] = 2'b01 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, (val32 | B_BE_AFSM_WLSUS_EN) &
			~B_BE_AFSM_PCIE_SUS_EN);

	/* 0x4086[0] = 1 */
	val32 = MAC_REG_R32(R_BE_SDIO_BUS_CTRL);
	MAC_REG_W32(R_BE_SDIO_BUS_CTRL, val32 | B_BE_HCI_SUS_REQ);

	/* polling 0x4086[1] = 0 */
	ret = pwr_poll_u32(adapter,
			   R_BE_SDIO_BUS_CTRL, B_BE_HCI_RESUME_RDY, 0);
	if (ret)
		return ret;

	return MACSUCCESS;
}

u32 mac_pwr_off_usb_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u8 val8;
	u32 ret;

	/* 0x04[16] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_EN_WLON);

	/* 0x02[1:0] = 0 */
	val8 = MAC_REG_R8(R_BE_SYS_FUNC_EN);
	MAC_REG_W8(R_BE_SYS_FUNC_EN,
		   val8 & ~B_BE_FEN_BB_GLB_RSTN & ~B_BE_FEN_BBRSTB);

	/* 0x82[1:0] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND,
		    val32 & ~B_BE_R_SYM_FEN_WLBBGLB_1 &
		    ~B_BE_R_SYM_FEN_WLBBFUN_1);

	/* 0x04[9] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_APFM_OFFMAC);

	/* polling 0x04[9] = 0 */
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_APFM_OFFMAC, 0);
	if (ret)
		return ret;

	/* 0x04[28] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 & ~B_BE_SOP_EDSWR);

	/* 0x04[22] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_XTAL_OFF_A_DIE);

	/* 0x04[12:11] = 2'b01 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, (val32 | B_BE_AFSM_WLSUS_EN) &
			~B_BE_AFSM_PCIE_SUS_EN);

	return MACSUCCESS;
}

#ifdef PHL_PLATFORM_AP
u32 mac_pwr_off_ap_pcie_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u8 val8;
	u32 ret;

	/* 0x04[16] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_EN_WLON);

	/* 0x02[1:0] = 0 */
	val8 = MAC_REG_R8(R_BE_SYS_FUNC_EN);
	MAC_REG_W8(R_BE_SYS_FUNC_EN,
		   val8 & ~B_BE_FEN_BB_GLB_RSTN & ~B_BE_FEN_BBRSTB);

	/* 0x82[1:0] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND,
		    val32 & ~B_BE_R_SYM_FEN_WLBBGLB_1 &
		    ~B_BE_R_SYM_FEN_WLBBFUN_1);

	/* 0x04[9] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_APFM_OFFMAC);

	/* polling 0x04[9] = 0 */
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_APFM_OFFMAC, 0);
	if (ret)
		return ret;

	/* 0x91[0] = 0 == 0x90[8]=0 */
	val32 = MAC_REG_R32(R_BE_WLLPS_CTRL);
	MAC_REG_W32(R_BE_WLLPS_CTRL, val32 & ~B_BE_LPSOP_DSWR);

	/* 0x04[10] = 1 */
	//val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	//MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_APFM_SWLPS);

	return MACSUCCESS;
}
#else
u32 mac_pwr_off_nic_pcie_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u8 val8;
	u32 ret;

	/* 0x04[16] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_EN_WLON);

	/* 0x02[1:0] = 0 */
	val8 = MAC_REG_R8(R_BE_SYS_FUNC_EN);
	MAC_REG_W8(R_BE_SYS_FUNC_EN,
		   val8 & ~B_BE_FEN_BB_GLB_RSTN & ~B_BE_FEN_BBRSTB);

	/* 0x82[1:0] = 0 */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND,
		    val32 & ~B_BE_R_SYM_FEN_WLBBGLB_1 &
		    ~B_BE_R_SYM_FEN_WLBBFUN_1);

	/* 0x04[9] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_APFM_OFFMAC);

	/* polling 0x04[9] = 0 */
	ret = pwr_poll_u32(adapter, R_BE_SYS_PW_CTRL, B_BE_APFM_OFFMAC, 0);
	if (ret)
		return ret;

	/* 0x90[31:0] = 0x00_01_A0_B0 */
	MAC_REG_W32(R_BE_WLLPS_CTRL, 0x0001A0B0);

	/* 0x04[22] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_XTAL_OFF_A_DIE);

	/* 0x04[10] = 1 */
	val32 = MAC_REG_R32(R_BE_SYS_PW_CTRL);
	MAC_REG_W32(R_BE_SYS_PW_CTRL, val32 | B_BE_APFM_SWLPS);

	return MACSUCCESS;
}
#endif

#if MAC_BE_FEATURE_HV
u32 mac_enter_lps_sdio_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;

	/* 0x7800[0] = 1  */
	val32 = MAC_REG_R32(R_BE_FWD1IMR);
	MAC_REG_W32(R_BE_FWD1IMR, val32 | B_BE_FS_RPWM_INT_EN);

	/* 0x82[4] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_IMEM22PP);
	/* 0x25[2] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_IMEM2_PC_EN);

	/* 0x82[5] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_IMEM32PP);
	/* 0x25[3] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_IMEM3_PC_EN);

	/* 0x82[6] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_IMEM42PP);
	/* 0x25[4] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_IMEM4_PC_EN);

	/* 0x83[2] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_DMEM32PP);
	/* 0x25[5] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_DMEM1_PC_EN);

	/* 0x83[3] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_DMEM42PP);
	/* 0x25[6] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_DMEM2_PC_EN);

	/* 0x83[4] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_DMEM52PP);
	/* 0x25[7] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_DMEM3_PC_EN);

	/* 0x90[31:0] = 0x04_80_A1_B0 */
	MAC_REG_W32(R_BE_WLLPS_CTRL, 0x0480A1B0);

	/* 0x90[0] = 1 */
	val32 = MAC_REG_R32(R_BE_WLLPS_CTRL);
	MAC_REG_W32(R_BE_WLLPS_CTRL, val32 | B_BE_WL_LPS_EN);

	/* polling 0x90[0] = 0 */
	ret = pwr_poll_u32(adapter, R_BE_WLLPS_CTRL, B_BE_WL_LPS_EN, 0);
	if (ret)
		return ret;

	return MACSUCCESS;
}

u32 mac_enter_lps_usb_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;

	/* 0x7800[0] = 1  */
	val32 = MAC_REG_R32(R_BE_FWD1IMR);
	MAC_REG_W32(R_BE_FWD1IMR, val32 | B_BE_FS_RPWM_INT_EN);

	/* 0x82[4] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_IMEM22PP);
	/* 0x25[2] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_IMEM2_PC_EN);

	/* 0x82[5] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_IMEM32PP);
	/* 0x25[3] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_IMEM3_PC_EN);

	/* 0x82[6] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_IMEM42PP);
	/* 0x25[4] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_IMEM4_PC_EN);

	/* 0x83[2] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_DMEM32PP);
	/* 0x25[5] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_DMEM1_PC_EN);

	/* 0x83[3] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_DMEM42PP);
	/* 0x25[6] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_DMEM2_PC_EN);

	/* 0x83[4] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_DMEM52PP);
	/* 0x25[7] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_DMEM3_PC_EN);

	/* 0x90[31:0] = 0x04_81_A1_B0 */
	MAC_REG_W32(R_BE_WLLPS_CTRL, 0x0481A1B0);

	/* 0x90[0] = 1 */
	val32 = MAC_REG_R32(R_BE_WLLPS_CTRL);
	MAC_REG_W32(R_BE_WLLPS_CTRL, val32 | B_BE_WL_LPS_EN);

	/* polling 0x90[0] = 0 */
	ret = pwr_poll_u32(adapter, R_BE_WLLPS_CTRL, B_BE_WL_LPS_EN, 0);
	if (ret)
		return ret;

	return MACSUCCESS;
}

u32 mac_enter_lps_pcie_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;

	/* 0x7800[0] = 1  */
	val32 = MAC_REG_R32(R_BE_FWD1IMR);
	MAC_REG_W32(R_BE_FWD1IMR, val32 | B_BE_FS_RPWM_INT_EN);

	/* 0x82[4] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_IMEM22PP);
	/* 0x25[2] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_IMEM2_PC_EN);

	/* 0x82[5] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_IMEM32PP);
	/* 0x25[3] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_IMEM3_PC_EN);

	/* 0x82[6] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_IMEM42PP);
	/* 0x25[4] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_IMEM4_PC_EN);

	/* 0x83[2] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_DMEM32PP);
	/* 0x25[5] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_DMEM1_PC_EN);

	/* 0x83[3] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_DMEM42PP);
	/* 0x25[6] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_DMEM2_PC_EN);

	/* 0x83[4] = 1  */
	val32 = MAC_REG_R32(R_BE_SYS_ISO_CTRL_EXTEND);
	MAC_REG_W32(R_BE_SYS_ISO_CTRL_EXTEND, val32 | B_BE_R_SYM_ISO_DMEM52PP);
	/* 0x25[7] = 0  */
	val32 = MAC_REG_R32(R_BE_AFE_CTRL1);
	MAC_REG_W32(R_BE_AFE_CTRL1, val32 & ~B_BE_DMEM3_PC_EN);

	/* 0x90[31:0] = 0x04_81_A1_B0 */
	MAC_REG_W32(R_BE_WLLPS_CTRL, 0x0481A1B0);

	/* 0x90[0] = 1 */
	val32 = MAC_REG_R32(R_BE_WLLPS_CTRL);
	MAC_REG_W32(R_BE_WLLPS_CTRL, val32 | B_BE_WL_LPS_EN);

	/* polling 0x90[0] = 0 */
	ret = pwr_poll_u32(adapter, R_BE_WLLPS_CTRL, B_BE_WL_LPS_EN, 0);
	if (ret)
		return ret;

	return MACSUCCESS;
}

u32 mac_leave_lps_sdio_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	u32 cnt = 320;

	/* 0x4083[7] = 1 == 0x4080[31] = 1 */
	val32 = MAC_REG_R32(R_BE_SDIO_HRPWM1_V1);
	MAC_REG_W32(R_BE_SDIO_HRPWM1_V1, val32 | BIT(31));

	/* polling 0x1E5[7] = 1 == 0x1E4[15] = 1 */
	ret = pwr_poll_u32(adapter, R_BE_RPWM,
			   B_BE_RPWM_TOGGLE, B_BE_RPWM_TOGGLE);
	if (ret)
		return ret;

	/* delay 0x10 ms */
	while (--cnt)
		PLTFM_DELAY_US(50);

	/* 0x4083[7] = 0 == 0x4080[31] = 0 */
	val32 = MAC_REG_R32(R_BE_SDIO_HRPWM1_V1);
	MAC_REG_W32(R_BE_SDIO_HRPWM1_V1, val32 & ~(BIT(31)));

	/* 0x7804[0] = 1 */
	val32 = MAC_REG_R32(R_BE_FWD1ISR);
	MAC_REG_W32(R_BE_FWD1ISR, val32 | B_BE_FS_RPWM_INT);

	return MACSUCCESS;
}

u32 mac_leave_lps_usb_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	u32 cnt = 320;

	/* 0x5203[7] = 1 == 0x5200[31] = 1 */
	val32 = MAC_REG_R32(R_BE_USB_D2F_F2D_INFO_V1);
	MAC_REG_W32(R_BE_USB_D2F_F2D_INFO_V1, val32 | BIT(31));

	/* polling 0x1E5[7] = 1 == 0x1E4[15] = 1 */
	ret = pwr_poll_u32(adapter, R_BE_RPWM,
			   B_BE_RPWM_TOGGLE, B_BE_RPWM_TOGGLE);
	if (ret)
		return ret;

	/* delay 0x10 ms */
	while (--cnt)
		PLTFM_DELAY_US(50);

	/* 0x5203[7] = 0 == 0x5200[31] = 0 */
	val32 = MAC_REG_R32(R_BE_USB_D2F_F2D_INFO_V1);
	MAC_REG_W32(R_BE_USB_D2F_F2D_INFO_V1, val32 & ~(BIT(31)));

	/* 0x7804[0] = 1 */
	val32 = MAC_REG_R32(R_BE_FWD1ISR);
	MAC_REG_W32(R_BE_FWD1ISR, val32 | B_BE_FS_RPWM_INT);

	return MACSUCCESS;
}

u32 mac_leave_lps_pcie_1115e(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret;
	u32 cnt = 320;

	/* 0x30C1[7] = 1 */
	val32 = MAC_REG_R32(R_BE_PCIE_HRPWM_V1);
	MAC_REG_W32(R_BE_PCIE_HRPWM_V1, val32 | BIT(15));

	/* polling 0x1E5[7] = 1 == 0x1E4[15] = 1 */
	ret = pwr_poll_u32(adapter, R_BE_RPWM,
			   B_BE_RPWM_TOGGLE, B_BE_RPWM_TOGGLE);
	if (ret)
		return ret;

	/* delay 0x10 ms */
	while (--cnt)
		PLTFM_DELAY_US(50);

	/* 0x30C1[7] = 0 */
	val32 = MAC_REG_R32(R_BE_PCIE_HRPWM_V1);
	MAC_REG_W32(R_BE_PCIE_HRPWM_V1, val32 & ~(BIT(15)));

	/* 0x7804[0] = 1 */
	val32 = MAC_REG_R32(R_BE_FWD1ISR);
	MAC_REG_W32(R_BE_FWD1ISR, val32 | B_BE_FS_RPWM_INT);

	return MACSUCCESS;
}

#endif
#endif

