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

#ifndef _STA_SCH_HV_H_
#define _STA_SCH_HV_H_

#include "../hv_type.h"
#include "../type.h"

#define CTRL1_R_QUOTA_SETTING	0
#define CTRL1_W_QUOTA_SETTING	1
#define CTRL1_R_QUOTA		2
#define CTRL1_W_QUOTA		3
#define CTRL1_R_TX_LEN		5
#define CTRL1_INCR_TX_LEN	6
#define CTRL1_DECR_TX_LEN	7
#define CTRL1_R_DL_MURU_DIS	8
#define CTRL1_W_DL_MURU_DIS	9
#define CTRL1_R_UL_TBL		10
#define CTRL1_W_UL_TBL		11
#define CTRL1_R_BSR_LEN		12
#define CTRL1_W_BSR_LEN		13
#define CTRL1_R_NEXT_LINK	20

#define SRAM_BMP		0
#define SRAM_LEN		1
#define SRAM_DLRU		2

#define BMP_WMM0_SH		4
#define BMP_WMM1_SH		5
#define BMP_WMM2_SH		6
#define BMP_WMM3_SH		7

#define TX_LEN_MSK		0x1FFFFF
#define BSR_LEN_MSK		0x7FFF
#define QUOTA_SETTING_MSK	0xF
#define QUOTA_MSK		0x1FF
#define UL_TBL_MSK		0x7FFF
#define DL_MURU_MSK		0x3

#define SEARCH_OK		0
#define SEARCH_FAIL		1

#define MACID_SH		8

#define STA_SCH_BITMAP_SIZE_8852A	16
#define STA_SCH_BITMAP_SIZE_8852B	6
#define STA_SCH_BITMAP_SIZE_8852C	16
#define STA_SCH_BITMAP_SIZE_8192XB	16
#define STA_SCH_BITMAP_SIZE_8851B	6

#define STA_SCH_WMM_NUM_8852A   4
#define STA_SCH_WMM_NUM_8852B   2
#define STA_SCH_WMM_NUM_8852C   4
#define STA_SCH_WMM_NUM_8192XB  4
#define STA_SCH_WMM_NUM_8851B   2

#define STA_SCH_UL_SUPPORT_8852A    1
#define STA_SCH_UL_SUPPORT_8852B    0
#define STA_SCH_UL_SUPPORT_8852C    1
#define STA_SCH_UL_SUPPORT_8192XB   1
#define STA_SCH_UL_SUPPORT_8851B    0

#define STA_SCH_MU_SUPPORT_8852A    1
#define STA_SCH_MU_SUPPORT_8852B    0
#define STA_SCH_MU_SUPPORT_8852C    1
#define STA_SCH_MU_SUPPORT_8192XB   1
#define STA_SCH_MU_SUPPORT_8851B    0

#define STA_SCH_RU_SUPPORT_8852A    1
#define STA_SCH_RU_SUPPORT_8852B    0
#define STA_SCH_RU_SUPPORT_8852C    1
#define STA_SCH_RU_SUPPORT_8192XB   1
#define STA_SCH_RU_SUPPORT_8851B    0

/**
 * @brief hv_sta_bmp_cfg
 *
 * @param *adapter
 * @param *ctrl
 * @param cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_sta_bmp_cfg(struct mac_ax_adapter *adapter,
		   struct hv_ax_sta_bmp_ctrl *ctrl, enum hv_ax_sta_bmp_cfg cfg);

/**
 * @brief hv_sta_len_cfg
 *
 * @param *adapter
 * @param *len
 * @param cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_sta_len_cfg(struct mac_ax_adapter *adapter, struct hv_ax_sta_len *len,
		   enum hv_ax_sta_len_cfg cfg);

/**
 * @brief hv_sta_dl_rugrp_cfg
 *
 * @param *adapter
 * @param *rugrp
 * @param cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_sta_dl_rugrp_cfg(struct mac_ax_adapter *adapter,
			struct hv_ax_sta_dl_rugrp_ctrl *rugrp,
			enum hv_ax_sta_muru_cfg cfg);

/**
 * @brief hv_sta_muru_cfg
 *
 * @param *adapter
 * @param *muru
 * @param cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_sta_muru_cfg(struct mac_ax_adapter *adapter,
		    struct hv_ax_sta_muru_ctrl *muru,
		    enum hv_ax_sta_muru_cfg cfg);

/**
 * @brief hv_sta_quota_cfg
 *
 * @param *adapter
 * @param *quota
 * @param cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_sta_quota_cfg(struct mac_ax_adapter *adapter,
		     struct hv_ax_sta_quota *quota,
		     enum hv_ax_sta_quota_cfg cfg);

/**
 * @brief hv_sta_link_cfg
 *
 * @param *adapter
 * @param *link
 * @param cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_sta_link_cfg(struct mac_ax_adapter *adapter,
		    struct hv_ax_ss_link_info *link,
		    enum hv_ax_ss_link_cfg cfg);

/**
 * @brief hv_ss_dl_rpt_cfg
 *
 * @param *adapter
 * @param *info
 * @param cfg
 * @return Please Place Description here.
 * @retval void
 */
void hv_ss_dl_rpt_cfg(struct mac_ax_adapter *adapter,
		      struct hv_ax_ss_dl_rpt_info *info,
		      enum hv_ax_ss_rpt_cfg cfg);

/**
 * @brief hv_ss_ul_rpt_cfg
 *
 * @param *adapter
 * @param *info
 * @param cfg
 * @return Please Place Description here.
 * @retval void
 */
void hv_ss_ul_rpt_cfg(struct mac_ax_adapter *adapter,
		      struct hv_ax_ss_ul_rpt_info *info,
		      enum hv_ax_ss_rpt_cfg cfg);

/**
 * @brief hv_ss_query_search
 *
 * @param *adapter
 * @param *info
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_ss_query_search(struct mac_ax_adapter *adapter,
		       struct hv_ax_ss_search_info *info);

/**
 * @brief hv_ss_rpt_path_cfg
 *
 * @param *adapter
 * @param cfg
 * @return Please Place Description here.
 * @retval void
 */
void hv_ss_rpt_path_cfg(struct mac_ax_adapter *adapter,
			enum hv_ax_ss_rpt_path_cfg cfg);

/**
 * @brief hv_ss_set_bsr_thold
 *
 * @param *adapter
 * @param thold_0
 * @param thold_1
 * @return Please Place Description here.
 * @retval void
 */
void hv_ss_set_bsr_thold(struct mac_ax_adapter *adapter, u16 thold_0,
			 u16 thold_1);

/**
 * @brief hv_ss_dlru_search_mode
 *
 * @param *adapter
 * @param mode
 * @return Please Place Description here.
 * @retval void
 */
void hv_ss_dlru_search_mode(struct mac_ax_adapter *adapter,
			    enum hv_ax_ss_dlru_search_mode mode);

/**
 * @brief hv_ss_set_delay_tx
 *
 * @param *adapter
 * @param *info
 * @return Please Place Description here.
 * @retval void
 */
void hv_ss_set_delay_tx(struct mac_ax_adapter *adapter,
			struct hv_ax_ss_delay_tx_info *info);

/**
 * @brief hv_sta_dl_mutbl_cfg
 *
 * @param *adapter
 * @param *mutbl
 * @param cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_sta_dl_mutbl_cfg(struct mac_ax_adapter *adapter,
			struct hv_ax_sta_dl_mutbl_ctrl *mutbl,
			enum hv_ax_sta_muru_cfg cfg);
/**
 * @brief hv_ss_dlmu_search_mode
 *
 * @param *adapter
 * @param mode
 * @param score_thr
 * @return Please Place Description here.
 * @retval void
 */
void hv_ss_dlmu_search_mode(struct mac_ax_adapter *adapter, u8 mode,
			    u8 score_thr);

/**
 * @brief hv_ss_quota_mode
 *
 * @param *adapter
 * @param *ctrl
 * @param cfg
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_ss_quota_mode(struct mac_ax_adapter *adapter,
		     struct hv_ax_ss_quota_mode_ctrl *ctrl,
		     enum hv_ax_ss_quota_mode_cfg cfg);

/**
 * @brief hv_ss_wmm_tbl_cfg
 *
 * @param *adapter
 * @param *ctrl
 * @param cfg
 * @return Please Place Description here.
 * @retval void
 */
void hv_ss_wmm_tbl_cfg(struct mac_ax_adapter *adapter,
		       struct mac_ax_ss_wmm_tbl_ctrl *ctrl,
		       enum hv_ax_ss_wmm_tbl_cfg cfg);

/**
 * @brief hv_ss_wmm_sta_move
 *
 * @param *adapter
 * @param src_wmm
 * @param dst_link
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_ss_wmm_sta_move(struct mac_ax_adapter *adapter,
		       enum hv_ax_ss_wmm src_wmm,
		       enum mac_ax_ss_wmm_tbl dst_link);

/**
 * @brief hv_ss_set_wmm_bmp
 *
 * @param *adapter
 * @param wmm
 * @param macid
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_ss_set_wmm_bmp(struct mac_ax_adapter *adapter, u8 wmm, u8 macid);

#endif
