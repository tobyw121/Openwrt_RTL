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
#include "btc_hv_8852b.h"
#include "../../mac_reg.h"
#define HV_DBG_PORT_SET_BTC 0x74

#if MAC_AX_8852B_SUPPORT

u32 hv_cfg_btc_dbg_port_8852b(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 reg;
	u32 val32;

	reg = MAC_REG_R8(R_AX_SYS_STATUS1 + 2);
	reg = (reg & 0xFC) | 1;
	MAC_REG_W8(R_AX_SYS_STATUS1 + 2, reg);

	MAC_REG_W8(R_AX_DBG_CTRL, HV_DBG_PORT_SET_BTC);
	MAC_REG_W8(R_AX_DBG_CTRL + 2, HV_DBG_PORT_SET_BTC);

	val32 = MAC_REG_R32(R_AX_PAD_CTRL1);
	val32 = val32 & ~(B_AX_BTGP_GPIO_EN | B_AX_BTGP_UART0_EN |
			  B_AX_BTGP_SPI_EN | B_AX_BTGP_JTAG_EN |
			  B_AX_WL_JTAG_EN | B_AX_BTGP_GPG3_FEN |
			  B_AX_BTGP_GPG2_FEN);
	MAC_REG_W32(R_AX_PAD_CTRL1, val32);

	val32 = MAC_REG_R32(R_AX_LED_CFG);
	val32 = val32 & ~(B_AX_BT_RF_GPIO_CFG | B_AX_BT_SDIO_INT_GPIO_CFG |
			  B_AX_GPIO13_14_WL_CTRL_EN);
	MAC_REG_W32(R_AX_LED_CFG, val32);

	val32 = MAC_REG_R32(R_AX_GPIO_MUXCFG);
	val32 = val32 & ~(B_AX_PO_WIFI_PTA_PINS | B_AX_BOOT_MODE |
			  B_AX_ENUARTRX | B_AX_ENUARTTX);
	val32 = val32 | B_AX_PO_BT_PTA_PINS;
	val32 = SET_CLR_WORD(val32, 1, B_AX_GPIOSEL);
	MAC_REG_W32(R_AX_GPIO_MUXCFG, val32);

	reg = MAC_REG_R8(R_AX_SYS_SDIO_CTRL + 3);
	reg = reg & ~BIT(3);
	MAC_REG_W8(R_AX_SYS_SDIO_CTRL + 3, reg);

	return MACSUCCESS;
}

u32 hv_en_rtk_mode_8852b(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u8 reg;

	reg = MAC_REG_R8(R_AX_GPIO_MUXCFG);
	reg = reg | B_AX_ENBT | B_AX_PO_BT_PTA_PINS;
	MAC_REG_W8(R_AX_GPIO_MUXCFG, reg);

	reg = MAC_REG_R8(R_AX_TDMA_MODE);
	MAC_REG_W8(R_AX_TDMA_MODE, reg | B_AX_RTK_BT_ENABLE);

	reg = MAC_REG_R8(R_AX_BT_COEX_CFG_2 + 1);
	MAC_REG_W8(R_AX_BT_COEX_CFG_2 + 1, reg | BIT(0));

	reg = MAC_REG_R8(R_AX_BT_COEX_CFG_5);
	reg = SET_CLR_WORD(reg, 5, B_AX_BT_RPT_SAMPLE_RATE);
	MAC_REG_W8(R_AX_BT_COEX_CFG_5, reg);

	return MACSUCCESS;
}

#endif