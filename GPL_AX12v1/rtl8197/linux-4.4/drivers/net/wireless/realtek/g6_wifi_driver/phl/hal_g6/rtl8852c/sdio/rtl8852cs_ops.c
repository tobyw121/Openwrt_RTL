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
#define _RTL8852CS_OPS_C_
#include "../rtl8852c_hal.h"
#include "rtl8852cs.h"

void hal_set_ops_8852cs(struct rtw_phl_com_t *phl_com,
			struct hal_info_t *hal)
{
	struct hal_ops_t *ops = hal_get_ops(hal);

	hal_set_ops_8852c(phl_com, hal);

	ops->init_hal_spec = init_hal_spec_8852cs;
	ops->hal_get_efuse = hal_get_efuse_8852cs;
	ops->hal_init = hal_init_8852cs;
	ops->hal_deinit = hal_deinit_8852cs;
	ops->hal_start = hal_start_8852cs;
	ops->hal_stop = hal_stop_8852cs;
#ifdef CONFIG_WOWLAN
	ops->hal_wow_init = hal_wow_init_8852cs;
	ops->hal_wow_deinit = hal_wow_deinit_8852cs;
#endif /* CONFIG_WOWLAN */
	ops->hal_mp_init = hal_mp_init_8852cs;
	ops->hal_mp_deinit = hal_mp_deinit_8852cs;

	ops->hal_hci_configure = hal_hci_cfg_8852cs;
	ops->init_default_value = init_default_value_8852cs;
	ops->init_int_default_value = init_int_default_value_8852cs;

	ops->enable_interrupt = hal_enable_int_8852cs;
	ops->disable_interrupt = hal_disable_int_8852cs;
	ops->config_interrupt = hal_config_int_8852cs;
	ops->recognize_interrupt = hal_recognize_int_8852cs;
	ops->recognize_halt_c2h_interrupt = hal_recognize_halt_c2h_int_8852cs;
	ops->clear_interrupt = hal_clear_interrupt_8852cs;
	ops->interrupt_handler = hal_int_hdler_8852cs;
	ops->restore_interrupt = hal_enable_int_8852cs;
}

/*
 * This function copied from 8852cu, maybe we should refine it later...
 */
static u8 hal_mapping_hw_tx_chnl_8852cs(u16 macid, enum rtw_phl_ring_cat cat,
					u8 band)
{
	u8 dma_ch = 0;


	if (0 == band) {
		switch (cat) {
		case RTW_PHL_RING_CAT_TID0:/*AC_BE*/
		case RTW_PHL_RING_CAT_TID3:
		case RTW_PHL_RING_CAT_TID6:/*AC_VO*/
		case RTW_PHL_RING_CAT_TID7:
			dma_ch = ACH0_QUEUE_IDX_8852C;
			break;
		case RTW_PHL_RING_CAT_TID1:/*AC_BK*/
		case RTW_PHL_RING_CAT_TID2:
		case RTW_PHL_RING_CAT_TID4:/*AC_VI*/
		case RTW_PHL_RING_CAT_TID5:
			dma_ch = ACH2_QUEUE_IDX_8852C;
			break;
		case RTW_PHL_RING_CAT_MGNT:
			dma_ch = MGQ_B0_QUEUE_IDX_8852C;
			break;
		case RTW_PHL_RING_CAT_HIQ:
			dma_ch = HIQ_B0_QUEUE_IDX_8852C;
			break;
		default:
			dma_ch = ACH0_QUEUE_IDX_8852C;
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
				  "[WARNING]unknown category (%d)\n", cat);
           			break;
		}
	} else if (1 == band) {
		switch (cat) {
		case RTW_PHL_RING_CAT_TID0:/*AC_BE*/
		case RTW_PHL_RING_CAT_TID3:
		case RTW_PHL_RING_CAT_TID6:/*AC_VO*/
		case RTW_PHL_RING_CAT_TID7:
			dma_ch = ACH4_QUEUE_IDX_8852C;
			break;
		case RTW_PHL_RING_CAT_TID1:/*AC_BK*/
		case RTW_PHL_RING_CAT_TID2:
		case RTW_PHL_RING_CAT_TID4:/*AC_VI*/
		case RTW_PHL_RING_CAT_TID5:
			dma_ch = ACH6_QUEUE_IDX_8852C;
			break;
		case RTW_PHL_RING_CAT_MGNT:
			dma_ch = MGQ_B1_QUEUE_IDX_8852C;
			break;
		case RTW_PHL_RING_CAT_HIQ:
			dma_ch = HIQ_B1_QUEUE_IDX_8852C;
			break;
		default:
			dma_ch = ACH0_QUEUE_IDX_8852C;
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
				  "[WARNING]unknown category (%d)\n", cat);
			break;
		}
	} else {
		dma_ch = ACH0_QUEUE_IDX_8852C;
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
			  "[WARNING]unknown band (%d)\n", band);
	}

	return dma_ch;
}

u16 hal_get_avail_page_8852cs(struct rtw_hal_com_t *hal_com, u8 dma_ch,
			u16 *host_idx, u16 *hw_idx)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	return hstatus;
}
static void hal_trx_deinit_8852cs(struct hal_info_t *hal)
{
}

enum rtw_hal_status hal_trx_init_8852cs(struct hal_info_t *hal)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	return hstatus;
}

static u8 hal_get_fwcmd_queue_idx_8852cs(void)
{
	return FWCMD_QUEUE_IDX_8852C;
}
static struct hal_trx_ops ops = {
	.init = hal_trx_init_8852cs,
	.deinit = hal_trx_deinit_8852cs,
	.query_tx_res = hal_get_avail_page_8852cs,
	.map_hw_tx_chnl = hal_mapping_hw_tx_chnl_8852cs,
	.get_fwcmd_queue_idx = hal_get_fwcmd_queue_idx_8852cs,
	.handle_rx_buffer = hal_handle_rx_buffer_8852c,
};

u32 hal_hook_trx_ops_8852cs(struct hal_info_t *hal_info)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	if (NULL != hal_info) {
		hal_info->trx_ops = &ops;
		hstatus = RTW_HAL_STATUS_SUCCESS;
	}

	return hstatus;
}
