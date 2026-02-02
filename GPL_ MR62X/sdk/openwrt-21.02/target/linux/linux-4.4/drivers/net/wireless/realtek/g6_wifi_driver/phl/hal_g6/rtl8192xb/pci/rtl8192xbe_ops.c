/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#define _RTL8192XBE_OPS_C_
#include "../rtl8192xb_hal.h"
#include "rtl8192xbe.h"

void hal_set_ops_8192xbe(struct rtw_phl_com_t *phl_com,
			struct hal_info_t *hal)
{
	struct hal_ops_t *ops = hal_get_ops(hal);

	hal_set_ops_8192xb(phl_com, hal);

	ops->init_hal_spec = init_hal_spec_8192xbe;
	ops->hal_get_efuse = hal_get_efuse_8192xbe;
	//ops->hal_get_flash = hal_get_flash_8192xbe;
	ops->hal_init = hal_init_8192xbe;
	ops->hal_deinit = hal_deinit_8192xbe;
	ops->hal_start = hal_start_8192xbe;
	ops->hal_stop = hal_stop_8192xbe;
#ifdef CONFIG_WOWLAN
	ops->hal_wow_init = hal_wow_init_8192xbe;
	ops->hal_wow_deinit = hal_wow_deinit_8192xbe;
#endif /* CONFIG_WOWLAN */
	ops->hal_mp_init = hal_mp_init_8192xbe;
	ops->hal_mp_deinit = hal_mp_deinit_8192xbe;

	ops->hal_hci_configure = hal_hci_cfg_8192xbe;
	ops->init_default_value = hal_init_default_value_8192xbe;
	ops->disable_interrupt_isr = hal_disable_int_isr_8192xbe;
	ops->init_int_default_value = hal_init_int_default_value_8192xbe;
	ops->enable_interrupt = hal_enable_int_8192xbe;
	ops->disable_interrupt = hal_disable_int_8192xbe;
	ops->recognize_interrupt = hal_recognize_int_8192xbe;
	ops->clear_interrupt = hal_clear_int_8192xbe;
	ops->interrupt_handler = hal_int_hdler_8192xbe;
	ops->restore_interrupt = hal_restore_int_8192xbe;
	ops->restore_rx_interrupt = hal_rx_int_restore_8192xbe;
}

