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

#include "pwr_seq_hv_8851b.h"
#include "../../mac_ax/mac_priv.h"
#include "../../mac_ax/mac_8851b/pwr_seq_func_8851b.h"
#if MAC_AX_8851B_SUPPORT

u32 hv_run_pwr_seq_8851b(struct mac_ax_adapter *adapter,
			  enum hv_ax_pwr_seq_sel sel)
{
	u32 ret = MACNOITEM;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	switch (sel) {
	case HV_AX_PWR_SEQ_SEL_ON_NIC:
	case HV_AX_PWR_SEQ_SEL_ON_AP:
		ret = adapter->ops->pwr_switch(adapter, 1);
		break;
	case HV_AX_PWR_SEQ_SEL_OFF_NIC:
	case HV_AX_PWR_SEQ_SEL_OFF_AP:
		ret = adapter->ops->pwr_switch(adapter, 0);
		break;
	case HV_AX_PWR_SEQ_SEL_ENTER_LPS:
		if (adapter->hw_info->intf == MAC_AX_INTF_PCIE)
			ret = mac_enter_lps_pcie_8851b(adapter);
		else if (adapter->hw_info->intf == MAC_AX_INTF_USB)
			ret = mac_enter_lps_usb_8851b(adapter);
		else if (adapter->hw_info->intf == MAC_AX_INTF_SDIO)
			ret = mac_enter_lps_sdio_8851b(adapter);
		else
			ret = MACINTF;
		adapter->mac_pwr_info.pwr_in_lps = 1;
		break;
	case HV_AX_PWR_SEQ_SEL_LEAVE_LPS:
		adapter->mac_pwr_info.pwr_in_lps = 0;
		if (adapter->hw_info->intf == MAC_AX_INTF_PCIE)
			mac_leave_lps_pcie_8851b(adapter);
		else if (adapter->hw_info->intf == MAC_AX_INTF_USB)
			mac_leave_lps_usb_8851b(adapter);
		else if (adapter->hw_info->intf == MAC_AX_INTF_SDIO)
			mac_leave_lps_sdio_8851b(adapter);
		else
			ret = MACINTF;
		break;
	default:
		ret= MACNOITEM;
		break;
	}

	return ret;
}
#endif /* #if MAC_AX_8851B_SUPPORT */
