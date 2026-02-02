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

#include "pwr_seq_hv_8852a.h"
#include "../../mac_ax/mac_priv.h"
#include "../../mac_ax/mac_8852a/pwr_seq_8852a.h"
#include "../../mac_ax/pwr.h"
#if MAC_AX_8852A_SUPPORT

u32 hv_run_pwr_seq_8852a(struct mac_ax_adapter *adapter,
			  enum hv_ax_pwr_seq_sel sel)
{
	u32 ret = MACNOITEM;
	struct mac_pwr_cfg *tmp;
	struct mac_ax_priv_ops *p_ops = adapter_to_priv_ops(adapter);

	switch (sel) {
	case HV_AX_PWR_SEQ_SEL_ON_NIC:
		tmp = adapter->hw_info->pwr_on_seq[0];
		adapter->hw_info->pwr_on_seq[0] = mac_pwron_nic_8852a;
		ret = adapter->ops->pwr_switch(adapter, 1);
		adapter->hw_info->pwr_on_seq[0] = tmp;
		break;
	case HV_AX_PWR_SEQ_SEL_OFF_NIC:
		tmp = adapter->hw_info->pwr_off_seq[0];
		adapter->hw_info->pwr_off_seq[0] = mac_pwroff_nic_8852a;
		ret = adapter->ops->pwr_switch(adapter, 0);
		adapter->hw_info->pwr_off_seq[0] = tmp;
		break;
	case HV_AX_PWR_SEQ_SEL_CARD_DIS:
		ret = pwr_seq_start(adapter, card_disable_seq_8852a);
		break;
	case HV_AX_PWR_SEQ_SEL_ENTER_LPS:
		ret = pwr_seq_start(adapter, enter_lps_seq_8852a);
		adapter->mac_pwr_info.pwr_in_lps = 1;
		break;
	case HV_AX_PWR_SEQ_SEL_LEAVE_LPS:
		ret = adapter->mac_pwr_info.pwr_in_lps = 0;
		pwr_seq_start(adapter, leave_lps_seq_8852a);
		break;
	case HV_AX_PWR_SEQ_SEL_IPS:
		ret = pwr_seq_start(adapter, ips_seq_8852a);
		break;
	case HV_AX_PWR_SEQ_SEL_ON_AP:
		tmp = adapter->hw_info->pwr_on_seq[0];
		adapter->hw_info->pwr_on_seq[0] = mac_pwron_ap_8852a;
		ret = adapter->ops->pwr_switch(adapter, 1);
		adapter->hw_info->pwr_on_seq[0] = tmp;
		break;
	case HV_AX_PWR_SEQ_SEL_OFF_AP:
		tmp = adapter->hw_info->pwr_off_seq[0];
		adapter->hw_info->pwr_off_seq[0] = mac_pwroff_ap_8852a;
		ret = adapter->ops->pwr_switch(adapter, 0);
		adapter->hw_info->pwr_off_seq[0] = tmp;
		break;
	default:
		ret = MACNOITEM;
		break;
	}

	return ret;
}
#endif /* #if MAC_AX_8852A_SUPPORT */
