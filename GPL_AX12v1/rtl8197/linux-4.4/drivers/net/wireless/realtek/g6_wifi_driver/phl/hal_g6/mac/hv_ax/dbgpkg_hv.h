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

#ifndef _MAC_AX_DBGPKG_HV_H_
#define _MAC_AX_DBGPKG_HV_H_
#include "../type.h"

#define get_priv(adapter) \
	((struct mac_ax_priv_ops **)(((struct mac_ax_adapter *)(adapter) + 1)))

#define adapter_to_priv_ops(adapter) \
	(*(get_priv(adapter)))

/**
 * @brief hv_get_dle_dfi_info
 *
 * @param *adapter
 * @param *dbg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_get_dle_dfi_info(struct mac_ax_adapter *adapter,
			struct hv_dbg_port *dbg);


/**
 * @brief hv_get_dbg_port_info
 *
 * @param *adapter
 * @param *dbg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_get_dbg_port_info(struct mac_ax_adapter *adapter,
			 struct hv_dbg_port *dbg);

/**
 * @brief hv_set_ctrl_frame_cnt
 *
 * @param *adapter
 * @param *ctrl
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_set_ctrl_frame_cnt(struct mac_ax_adapter *adapter,
			  struct hv_ctrl_frame_cnt *ctrl);

/**
 * @brief hv_set_rx_cnt
 *
 * @param *adapter
 * @param *cnt
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_set_rx_cnt(struct mac_ax_adapter *adapter,
		  struct hv_rx_cnt *cnt);

/**
 * @brief c2h_sys_fw_autotest
 *
 * @param *adapter
 * @param *buf
 * @param len
 * @param *info
 * @return Please Place Description here.
 * @retval u32
 */
u32 c2h_sys_fw_autotest(struct mac_ax_adapter *adapter,  u8 *buf, u32 len,
              struct rtw_c2h_info *info);

/**
 * @brief hv_get_mac_err_isr
 *
 * @param *adapter
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_get_mac_err_isr(struct mac_ax_adapter *adapter);

/**
 * @brief hv_c2h_log_test
 *
 * @param *adapter
 * @param *len
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_c2h_log_test(struct mac_ax_adapter *adapter, u32 len);

#endif
