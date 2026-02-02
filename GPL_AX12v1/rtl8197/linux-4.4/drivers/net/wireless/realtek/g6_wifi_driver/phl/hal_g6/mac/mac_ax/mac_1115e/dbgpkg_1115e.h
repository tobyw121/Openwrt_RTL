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

#ifndef _MAC_AX_DBGPKG_1115E_H_
#define _MAC_AX_DBGPKG_1115E_H_

#include "../../mac_def.h"

#if MAC_AX_1115E_SUPPORT

/**
 * @brief tx_dbg_dump_1115e
 * mac tx_dbg_dump_1115e
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 tx_dbg_dump_1115e(struct mac_ax_adapter *adapter);
/**
 * @}
 * @}
 */

/**
 * @brief crit_dbg_dump_1115e
 * mac crit_dbg_dump_1115e
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 crit_dbg_dump_1115e(struct mac_ax_adapter *adapter);
/**
 * @}
 * @}
 */

/**
 * @brief cmac_dbg_dump_1115e
 * mac cmac_dbg_dump_1115e
 * @param *adapter
 * @param *band
 * @return Please Place Description here.
 * @retval u32
 */
u32 cmac_dbg_dump_1115e(struct mac_ax_adapter *adapter, enum mac_ax_band band);
/**
 * @}
 * @}
 */

/**
 * @brief dmac_dbg_dump_1115e
 * mac dmac_dbg_dump_1115e
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 dmac_dbg_dump_1115e(struct mac_ax_adapter *adapter);
/**
 * @}
 * @}
 */

/**
 * @brief dbg_port_sel_1115e
 * mac debug port sel 1115e
 * @param *adapter
 * @param *mac_ax_dbg_port_info
 * @param *sel
 * @return Please Place Description here.
 * @retval u32
 */
u32 dbg_port_sel_1115e(struct mac_ax_adapter *adapter,
		       struct mac_ax_dbg_port_info **info, u32 sel);
/**
 * @}
 * @}
 */

/**
 * @brief dbg_port_sel_rst_1115e
 * mac debug port sel rst 1115e
 * @param *adapter
 * @param sel
 * @return Please Place Description here.
 * @retval u32
 */
u32 dbg_port_sel_rst_1115e(struct mac_ax_adapter *adapter, u32 sel);
/**
 * @}
 * @}
 */

/**
 * @brief tx_flow_ptcl_dbg_port_1115e
 * mac tx_flow_ptcl_dbg_port_1115e
 * @param *adapter
 * @param u8 band
 * @return Please Place Description here.
 * @retval u32
 */
u32 tx_flow_ptcl_dbg_port_1115e(struct mac_ax_adapter *adapter, u8 band);
/**
 * @}
 * @}
 */

/**
 * @brief tx_flow_sch_dbg_port_1115e
 * mac tx_flow_sch_dbg_port_1115e
 * @param *adapter
 * @param u8 band
 * @return Please Place Description here.
 * @retval u32
 */
u32 tx_flow_sch_dbg_port_1115e(struct mac_ax_adapter *adapter, u8 band);
/**
 * @}
 * @}
 */

/**
 * @brief ss_stat_chk_1115e
 * mac ss_stat_chk
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 ss_stat_chk_1115e(struct mac_ax_adapter *adapter)
;
/**
 * @}
 * @}
 */

/**
 * @brief get_mem_base_addr_1115e
 *
 * @param *adapter
 * @param *base_addr
 * @param sel
 * @return Please Place Description here.
 * @retval u32
 */
u32 get_mem_base_addr_1115e(struct mac_ax_adapter *adapter, u32 *base_addr,
			    enum mac_ax_mem_sel sel);
/**
 * @}
 * @}
 */

/**
 * @brief sram_dbg_write_1115e
 *
 * @param *adapter
 * @param offset
 * @param val
 * @param sel
 * @return Please Place Description here.
 * @retval u32
 */
u32 sram_dbg_write_1115e(struct mac_ax_adapter *adapter, u32 offset,
			 u32 val, enum mac_ax_sram_dbg_sel sel);
/**
 * @}
 * @}
 */

/**
 * @brief sram_dbg_read_1115e
 *
 * @param *adapter
 * @param offset
 * @param *val
 * @param sel
 * @return Please Place Description here.
 * @retval u32
 */
u32 sram_dbg_read_1115e(struct mac_ax_adapter *adapter, u32 offset, u32 *val,
			enum mac_ax_sram_dbg_sel sel);
/**
 * @}
 * @}
 */

/**
 * @brief is_dbg_port_not_valid_1115e
 *
 * @param *adapter
 * @param dbg_sel
 * @return Please Place Description here.
 * @retval u8
 */
u8 is_dbg_port_not_valid_1115e(struct mac_ax_adapter *adapter, u32 dbg_sel);
/**
 * @}
 * @}
 */

#endif /* MAC_BE_1115E_SUPPORT */
#endif /* _MAC_AX_DBGPKG_1115E_H_ */
