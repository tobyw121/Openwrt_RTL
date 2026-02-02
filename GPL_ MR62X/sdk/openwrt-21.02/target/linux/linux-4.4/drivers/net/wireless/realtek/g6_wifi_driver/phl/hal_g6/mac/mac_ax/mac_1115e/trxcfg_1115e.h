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

#ifndef _MAC_AX_TRXCFG_1115E_H_
#define _MAC_AX_TRXCFG_1115E_H_
#include "../trxcfg.h"

#define S_BE_TXSB_20M_8		8
#define S_BE_TXSB_20M_4		4
#define S_BE_TXSB_20M_2		2
#define S_BE_TXSB_40M_0		0
#define S_BE_TXSB_40M_1		1
#define S_BE_TXSB_40M_4		4
#define S_BE_TXSB_80M_0		0
#define S_BE_TXSB_80M_2		2
#define S_BE_TXSB_80M_4		4
#define S_BE_TXSB_160M_0	0
#define S_BE_TXSB_160M_1	1

#if MAC_AX_1115E_SUPPORT
/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */

/**
 * @brief cmac_init_1115e
 *
 * @param *adapter
 * @param *info
 * @param band
 * @return Please Place Description here.
 * @retval u32
 */
u32 cmac_init_1115e(struct mac_ax_adapter *adapter, struct mac_ax_trx_info *info,
		    enum mac_ax_band band);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */

/**
 * @brief mac_trx_init_1115e
 *
 * @param *adapter
 * @param *info
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_trx_init_1115e(struct mac_ax_adapter *adapter, struct mac_ax_trx_info *info);
/**
 * @}
 * @}
 */

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */

/**
 * @brief mac_two_nav_cfg_1115e
 *
 * @param *adapter
 * @param *info
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_two_nav_cfg_1115e(struct mac_ax_adapter *adapter,
			  struct mac_ax_2nav_info *info);
/**
 * @}
 * @}
 */
#endif //MAC_AX_1115E_SUPPORT
#endif // _MAC_AX_TRXCFG_1115E_H_
