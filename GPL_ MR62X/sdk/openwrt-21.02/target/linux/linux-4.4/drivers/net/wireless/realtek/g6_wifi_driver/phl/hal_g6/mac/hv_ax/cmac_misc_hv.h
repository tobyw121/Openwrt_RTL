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

#ifndef _CMAC_MISC_HV_H_
#define _CMAC_MISC_HV_H_

#include "../hv_type.h"
#include "../type.h"

/**
 * @brief hv_set_freerun_cfg
 *
 * @param *adapter
 * @param cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_set_freerun_cfg(struct mac_ax_adapter *adapter,
		       enum hv_ax_freerun_cfg cfg);

/**
 * @brief hv_get_freerun_info
 *
 * @param *adapter
 * @param *cnt_low
 * @param *cnt_high
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_get_freerun_info(struct mac_ax_adapter *adapter, u32 *cnt_low,
			u32 *cnt_high);

/**
 * @brief hv_set_lifetime_mg2
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_set_lifetime_mg2(struct mac_ax_adapter *adapter,
			struct hv_ax_lifetime_mg2_cfg *cfg);

/**
 * @brief hv_get_lifetime_mg2
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_get_lifetime_mg2(struct mac_ax_adapter *adapter,
			struct hv_ax_lifetime_mg2_cfg *cfg);

u32 hv_get_ampdu_cfg(struct mac_ax_adapter *adapter,
		     struct mac_ax_ampdu_cfg *cfg);

u32 hv_get_edca_param(struct mac_ax_adapter *adapter,
		      struct mac_ax_edca_param *param);

u32 hv_get_muedca_param(struct mac_ax_adapter *adapter,
			struct mac_ax_muedca_param *param);

u32 hv_get_muedca_timer(struct mac_ax_adapter *adapter,
			struct mac_ax_muedca_timer *timer);

u32 hv_get_muedca_ctrl(struct mac_ax_adapter *adapter,
		       struct mac_ax_muedca_cfg *cfg);

u32 hv_get_ch_stat_cnt(struct mac_ax_adapter *adapter,
		       struct mac_ax_ch_stat_cnt *cnt);

u32 hv_get_lifetime_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_lifetime_cfg *cfg);
/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup TX_Config
 * @{
 */

/**
 * @brief get_hw_edcca_param
 *
 * @param *adapter
 * @param *param
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_get_hw_edcca_param(struct mac_ax_adapter *adapter,
			  struct mac_ax_edcca_param *param);
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

 * @brief set_hw_ofld_cfg
 *
 * @param *adapter
 * @param *param
 * @return success: MACSUCCESS
 * @retval u32
 */
u32 hv_set_ofld_cfg(struct mac_ax_adapter *adapter,
		    struct mac_ax_ofld_cfg *param);
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
 * @brief hv_get_macid_pause
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_get_macid_pause(struct mac_ax_adapter *adapter,
		       struct mac_ax_macid_pause_cfg *cfg);

/**
 * @brief hv_get_hw_sch_tx_en
 *
 * @param *adapter
 * @param *cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_get_hw_sch_tx_en(struct mac_ax_adapter *adapter,
			struct mac_ax_sch_tx_en_cfg *cfg);

u32 hv_set_hw_muedca_timer(struct mac_ax_adapter *adapter,
			   struct mac_ax_muedca_timer *timer);

u32 hv_set_hw_ch_busy_cnt(struct mac_ax_adapter *adapter,
			  struct mac_ax_ch_busy_cnt_cfg *cfg);
#endif
