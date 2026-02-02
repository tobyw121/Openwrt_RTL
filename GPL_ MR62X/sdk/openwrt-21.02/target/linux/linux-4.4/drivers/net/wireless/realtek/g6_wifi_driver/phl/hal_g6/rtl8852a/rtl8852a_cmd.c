/******************************************************************************
 *
 * Copyright(c) 2016 - 2019 Realtek Corporation.
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
#define _RTL8852A_CMD_C_
#include "../hal_headers.h"

#ifdef CONFIG_RTW_DEBUG_TX_RPT
void hal_debug_tx_report_8852a(struct rtw_hal_com_t *hal_com, u32 rpt_cnt)
{
	u32 reg32 = hal_read32(hal_com, R_AX_PTCLRPT_FULL_HDL);

	PHL_PRINT("Enabling TX report to host driver.\n");

	reg32 &= ~(B_AX_BCN_RPT_PATH_MSK << B_AX_BCN_RPT_PATH_SH);

	if (rpt_cnt == 0) {
	        /* Set beacon TX report path to WLAN CPU */
		reg32 |= (1 << B_AX_BCN_RPT_PATH_SH);
		hal_com->tx_rpt_cnt = 0;
	} else {
		hal_com->tx_rpt_cnt = rpt_cnt;
        }

	hal_write32(hal_com, R_AX_PTCLRPT_FULL_HDL, reg32);
} /* hal_debug_tx_report_8852a */
#endif /* CONFIG_RTW_DEBUG_TX_RPT */
