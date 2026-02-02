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

#ifndef _MAC_BE_PWR_SEQ_FUNC_1115E_H_
#define _MAC_BE_PWR_SEQ_FUNC_1115E_H_

#if MAC_AX_1115E_SUPPORT
#define PWR_SEQ_VER_1115E  000

u32 mac_pwr_on_sdio_1115e(struct mac_ax_adapter *adapter);
u32 mac_pwr_on_usb_1115e(struct mac_ax_adapter *adapter);
#ifdef PHL_PLATFORM_AP
u32 mac_pwr_on_ap_pcie_1115e(struct mac_ax_adapter *adapter);
#else
u32 mac_pwr_on_nic_pcie_1115e(struct mac_ax_adapter *adapter);
#endif

u32 mac_pwr_off_sdio_1115e(struct mac_ax_adapter *adapter);
u32 mac_pwr_off_usb_1115e(struct mac_ax_adapter *adapter);
#ifdef PHL_PLATFORM_AP
u32 mac_pwr_off_ap_pcie_1115e(struct mac_ax_adapter *adapter);
#else
u32 mac_pwr_off_nic_pcie_1115e(struct mac_ax_adapter *adapter);
#endif

#if MAC_BE_FEATURE_HV
u32 mac_enter_lps_sdio_1115e(struct mac_ax_adapter *adapter);
u32 mac_enter_lps_usb_1115e(struct mac_ax_adapter *adapter);
u32 mac_enter_lps_pcie_1115e(struct mac_ax_adapter *adapter);

u32 mac_leave_lps_sdio_1115e(struct mac_ax_adapter *adapter);
u32 mac_leave_lps_usb_1115e(struct mac_ax_adapter *adapter);
u32 mac_leave_lps_pcie_1115e(struct mac_ax_adapter *adapter);

#endif
#endif
#endif

