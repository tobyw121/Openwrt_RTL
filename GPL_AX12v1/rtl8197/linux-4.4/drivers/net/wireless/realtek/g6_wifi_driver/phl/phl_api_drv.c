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
#define _PHL_API_DRV_C_
#include "phl_headers.h"

void *rtw_phl_get_txbd_buf(struct rtw_phl_com_t *phl_com)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_com->phl_priv;

	return phl_info->hci_trx_ops->get_txbd_buf(phl_info);
}

void *rtw_phl_get_rxbd_buf(struct rtw_phl_com_t *phl_com)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_com->phl_priv;

	return phl_info->hci_trx_ops->get_rxbd_buf(phl_info);
}

struct rtw_h2c_pkt *rtw_phl_query_h2c_pkt(struct rtw_phl_com_t *phl_com,
											enum rtw_h2c_pkt_type type)
{
	struct rtw_h2c_pkt *h2c_pkt = NULL;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_com->phl_priv;

	h2c_pkt = (struct rtw_h2c_pkt *)phl_query_idle_h2c_pkt(phl_info, type);

	if (!h2c_pkt)
		phl_dump_h2c_pool_stats(phl_info->h2c_pool);

	return h2c_pkt;
}

enum rtw_phl_status
rtw_phl_recycle_h2c_pkt(struct rtw_phl_com_t *phl_com, struct rtw_h2c_pkt * h2c_pkt)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_com->phl_priv;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	pstatus = phl_info->hci_trx_ops->recycle_h2c(phl_info, h2c_pkt);
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status rtw_phl_pltfm_tx(struct rtw_phl_com_t *phl_com,
										struct rtw_h2c_pkt *pkt)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_com->phl_priv;

	#ifdef CONFIG_PCI_HCI
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	hci_trx_ops->recycle_busy_h2c(phl_info);
	#endif

	#ifdef CONFIG_RTW_MIRROR_DUMP
	phl_mirror_dump_h2c(phl_com->phl_priv, pkt);
	#endif

	pstatus = phl_info->hci_trx_ops->pltfm_tx(phl_info, pkt);

	return pstatus;
}

void rtw_phl_proc_cmd(void *phl, char proc_cmd,
		      struct rtw_proc_cmd *incmd,
		      char *output, u32 out_len)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hal_info_t *hal_info = (struct hal_info_t *)phl_info->hal;
	enum rtw_hal_status status = RTW_HAL_STATUS_SUCCESS;

	/* Avoid string comparison mismatch since extra char '\n' was appended to buf */
	if (incmd->in_type == RTW_ARG_TYPE_BUF
		&& incmd->in_cnt_len
		&& incmd->in.buf[incmd->in_cnt_len -1] == '\n'
		) {
		incmd->in_cnt_len--;
		incmd->in.buf[incmd->in_cnt_len] = '\0';
	}

	if (RTW_PROC_CMD_PHL == proc_cmd)
		status = rtw_phl_dbg_proc_cmd(phl_info, incmd, output, out_len);
	else if (RTW_PROC_CMD_CORE == proc_cmd)
		status = rtw_phl_dbg_core_cmd(phl_info, incmd, output, out_len);
	else
		status = rtw_hal_proc_cmd(hal_info, proc_cmd, incmd, output, out_len);
}

void rtw_phl_get_halmac_ver(char *buf, u16 buf_len)
{
	rtw_hal_get_mac_version(buf, buf_len);
}

void rtw_phl_get_fw_ver(void *phl, char *ver_str, u16 len)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_get_fw_ver(phl_info->hal, ver_str, len);
}

u32 rtw_phl_get_rf_radio_ver(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_get_radio_ver(phl_info->hal);
}

enum rtw_phl_status rtw_phl_msg_hub_hal_send(struct rtw_phl_com_t *phl_com,
							struct phl_msg_attribute* attr, struct phl_msg* msg)
{
	return phl_msg_hub_send((struct phl_info_t*)phl_com->phl_priv, attr, msg);
}
void rtw_phl_test_txtb_cfg(struct rtw_phl_com_t* phl_com,
	void *buf, u32 buf_len, u8 *cfg_bssid, u16 cfg_aid, u8 cfg_bsscolor)
{
	/* Get parameters from MP UL */
	u8 ui_bssid[6]={0x00, 0x09, 0x08, 0x07, 0x06, 0x05};
	u16 ui_aid = 2;
	u8 ui_bsscolor = 1;
	u8 cfg;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_com->phl_priv;
	struct rtw_wifi_role_t *role = &phl_com->wifi_roles[0];
	struct rtw_phl_stainfo_t *sta = rtw_phl_get_stainfo_self(
		phl_com->phl_priv, role);

	cfg = (u8)*((u8 *)buf);
	switch (cfg) {
		case 0:
			/* disable */
			role->mstate = MLME_NO_LINK;
			_os_mem_cpy(phlcom_to_drvpriv(phl_com), sta->mac_addr,
				role->mac_addr, MAC_ALEN);
			rtw_hal_update_sta_entry(phl_info->hal, sta, false);
			rtw_hal_role_cfg(phl_info->hal, role);
		break;

		case 1:
			role->mstate = MLME_LINKED;
			_os_mem_cpy(phlcom_to_drvpriv(phl_com), sta->mac_addr,
				cfg_bssid, MAC_ALEN);
			sta->wmode = WLAN_MD_MAX;
			sta->aid = cfg_aid;
			sta->asoc_cap.bsscolor = cfg_bsscolor;
			sta->asoc_cap.pkt_padding = 2;
			rtw_hal_update_sta_entry(phl_info->hal, sta, false);
			rtw_hal_role_cfg(phl_info->hal, role);
		break;

		case 2:
			role->mstate = MLME_LINKED;
			_os_mem_cpy(phlcom_to_drvpriv(phl_com), sta->mac_addr,
				ui_bssid, MAC_ALEN);
			sta->wmode = WLAN_MD_MAX;
			sta->aid = ui_aid;
			sta->asoc_cap.bsscolor = ui_bsscolor;
			sta->asoc_cap.pkt_padding = 2;
			rtw_hal_update_sta_entry(phl_info->hal, sta, false);
			rtw_hal_role_cfg(phl_info->hal, role);
		break;		default:
		break;
	}
}
u32
rtw_phl_get_hw_seq(void *phl, struct rtw_phl_stainfo_t *phl_sta, u8 tid)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_get_hw_seq(phl_info->hal, phl_sta->macid, tid);
}


#ifdef TX_BEAMFORMING
enum rtw_phl_status
rtw_phl_mac_set_rxd_zld_en(void *phl, u8 value)
{

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if (RTW_HAL_STATUS_SUCCESS ==rtw_hal_mac_set_rxd_zld_en(phl_info->hal, value)) {
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}
#endif /* TX_BEAMFORMING */

#ifdef CONFIG_RTW_DEBUG_CAM
void rtl_phl_dump_cam(void *phl, enum rtw_cam_table_e cam_id, u32 idx, u8 *buf)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	rtl_hal_dump_cam(phl_info->hal, cam_id, idx, buf);
}
#endif /* CONFIG_RTW_DEBUG_CAM */

void rtw_phl_pkt_ofld_reset_all_entry(struct rtw_phl_com_t* phl_com)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl_com->phl_priv;

	phl_pkt_ofld_reset_all_entry(phl_info);
}

#ifndef CONFIG_PHL_DRV_HAS_NVM
void rtw_phl_flash_set_info(void *phl, char *extra)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = (struct rtw_phl_com_t *)phl_info->phl_com;
	struct hal_info_t *hal_info = (struct hal_info_t *)(phl_info->hal);

	u8 get_cmd[32] = {0};
	u8 get_data[32] = {0};
	u8 value;

	u8 delim[] = "=";
	u8 *token;

	token = _os_strsep((char**)&extra, delim);
	if (token)
		_os_strncpy(get_cmd, token, sizeof(get_cmd));
	token = _os_strsep((char**)&extra, delim);
	if (token)
		_os_strncpy(get_data, token, sizeof(get_cmd));


	rtw_hal_flash_set_info(hal_info, get_cmd, get_data, phl_com->hal_spec.ic_name);
}

void rtw_phl_flash_set_by_offset(void *phl, u32 offset, u32 value)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hal_info_t *hal_info = (struct hal_info_t *)(phl_info->hal);

	rtw_hal_flash_set_by_offset(hal_info, offset, value);
}

u8 rtw_phl_flash_get_by_offset(void *phl, u32 offset)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hal_info_t *hal_info = (struct hal_info_t *)(phl_info->hal);

	return rtw_hal_flash_get_by_offset(hal_info, offset);
}

void rtw_phl_flash_dump(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hal_info_t *hal_info = (struct hal_info_t *)(phl_info->hal);

	rtw_hal_flash_dump(hal_info);
}

u8* rtw_phl_flash_get_para_path(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hal_info_t *hal_info = (struct hal_info_t *)(phl_info->hal);

	return rtw_hal_flash_get_para_path(hal_info);
}
#endif

#ifdef CONFIG_PHL_DRV_HAS_NVM
enum rtw_phl_status
rtw_phl_extract_efuse_info(void *phl, u8 *efuse_map, u32 info_type,
                           void *value, u8 size, u8 map_valid)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_extract_efuse_info(phl_info->hal, efuse_map,
	                                  (enum rtw_efuse_info)info_type,
					  				  value, size, map_valid);
}

enum rtw_phl_status
rtw_phl_get_efuse_size(void *phl, u32 *log_efuse_size, u32 *limit_efuse_size,
                       u32 *mask_size, u32 *limit_mask_size)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_get_efuse_size(phl_info->hal, log_efuse_size,
								  limit_efuse_size, mask_size, limit_mask_size);
}

enum rtw_phl_status
rtw_phl_nvm_apply_dev_cap(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_nvm_apply_dev_cap(phl_info->hal, phl_info->phl_com);
}
#endif /* CONFIG_PHL_DRV_HAS_NVM */

void rtw_phl_dump_tpu(void * phl){

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hal_info_t *hal_info = (struct hal_info_t *)(phl_info->hal);

	rtw_hal_bb_dump_tpu_info(hal_info);
}


void rtw_phl_get_hw_cnt_tx_sts(void *phl, u32 *rtylmt, u32 *lt_drop,
								u32 *macid_drop, u32 *wp_polluted)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	*rtylmt = *lt_drop = *macid_drop = *wp_polluted = 0;
	rtw_hal_get_hw_cnt(phl_info->hal, HAL_CNT_TX_REACH_RTYLMT, rtylmt);
	rtw_hal_get_hw_cnt(phl_info->hal, HAL_CNT_TX_LIFETIME_DROP, lt_drop);
	rtw_hal_get_hw_cnt(phl_info->hal, HAL_CNT_TX_MACID_DROP, macid_drop);
	rtw_hal_get_hw_cnt(phl_info->hal, HAL_CNT_WP_POLLUTED, wp_polluted);
}

void rtw_phl_set_rf_regulation(void *phl, u8 *regular_str)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hal_info_t *hal_info = (struct hal_info_t *)(phl_info->hal);
	struct rtw_phl_com_t *phl_com = (struct rtw_phl_com_t *) phl_info->phl_com;
	struct phy_sw_cap_t *phy_sw_cap = NULL;
	u8 i;
	bool find;

	phy_sw_cap = &phl_com->phy_sw_cap[0];

	/* Query existing index here*/
	find = false;
	for (i=0; i<=phy_sw_cap->rf_txpwrlmt_info.ext_regd_arridx; i++) {

		PHL_INFO("[%d] %s\n", i, phy_sw_cap->rf_txpwrlmt_info.ext_regd_name[i]);

		if (strncmp(phy_sw_cap->rf_txpwrlmt_info.ext_regd_name[i], regular_str, 10) == 0) {
			find = true;
			PHL_PRINT("match [%d][%s]\n", i, regular_str);
			break;
		}
	}

	if (!find) {
		PHL_PRINT("input error\n");
		return;
	}

	rtw_hal_rf_set_regulation(hal_info, i);
}

s8
rtw_phl_get_tx_pwr_by_txrate(void *phl, enum channel_width bw, u8 mcs_idx){
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hal_info_t *hal_info = (struct hal_info_t *)(phl_info->hal);

	return rtw_hal_get_tx_pwr_by_txrate(hal_info, bw, mcs_idx);
}

void rtw_phl_set_rf_regulation_idx(void *phl, u8 idx)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hal_info_t *hal_info = (struct hal_info_t *)(phl_info->hal);

	rtw_hal_rf_set_regulation(hal_info, idx);
}

void rtw_phl_adjust_pwr_dbm(void *phl, s16 offset, u8 phy_idx){

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_bb_adjust_ref_offset(phl_info->hal, offset, phy_idx);
}

void rtw_phl_set_power_to_struct(void *phl, s8 dbm, u8 phy_idx){

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_rf_set_power_to_struct(phl_info->hal, phy_idx, dbm);
}

void rtw_phl_tssi_do_slope(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	PHL_INFO ("%s\n", __func__);
	rtw_hal_rf_tssi_do_slope(phl_info->hal);
}

void rtw_phl_tssi_finish_slope(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	PHL_INFO ("%s\n", __func__);
	rtw_hal_rf_tssi_finish_slope(phl_info->hal);
}

u32 rtw_phl_tssi_get_cw(void *phl, u8 path)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	PHL_INFO ("%s path=%d\n", __func__, path);
	return rtw_hal_rf_tssi_get_cw(phl_info->hal, path);
}

void rtw_phl_tssi_current_slope_apply(void *phl,
				u32 gain_diff, u32 cw_diff, u8 path)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	PHL_INFO ("%s: gain_diff=0x%x, cw_diff=0x%x, path=%d\n", __func__,
				gain_diff, cw_diff, path);
	rtw_hal_rf_tssi_current_slope_apply(phl_info->hal,
						gain_diff, cw_diff, path);
}

void rtw_phl_tssi_slope_apply(void *phl,
			u32 gain_1st, u32 gain_2nd,
			u32 cw_1st, u32 cw_2nd, u8 path,
			u32* gain_diff, u32* cw_diff)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	PHL_INFO ("%s gain_1st=%d, gain_2nd=%d, cw_1st=%d, cw_2nd=%d, path=%d\n", __func__,
				gain_1st, gain_2nd, cw_1st, cw_2nd, path);
	rtw_hal_rf_tssi_slope_apply(phl_info->hal,
				gain_1st, gain_2nd, cw_1st, cw_2nd,
				path, gain_diff, cw_diff);
	PHL_INFO ("%s: gain_diff=%d, cw_diff=%d\n", __func__, *gain_diff, *cw_diff);
}

#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
enum rtw_phl_status rtw_phl_enable_rx_amsdu_cut(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status phl_status;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;

	if (phl_info->phl_com->dev_state == RTW_DEV_WORKING) {
		RTW_ERR("Can not switch RX A-MSDU cut when device is enabled.\n");
		return RTW_PHL_STATUS_FAILURE;
	}

	phl_info->phl_com->dev_sw_cap.rx_amsdu_cut = true;
	#ifndef CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV
	phl_info->phl_com->dev_sw_cap.rx_hdr_conv = true;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV */

	PHL_PRINT("H2C Pool: %pX, %u, %pX, %pX\n",
	          phl_info->h2c_pool,
	          phl_info->h2c_pool->busy_h2c_pkt_list.cnt,
	          phl_info->h2c_pool->busy_h2c_pkt_list.queue.next,
	          phl_info->h2c_pool->busy_h2c_pkt_list.queue.prev);

	phl_datapath_deinit(phl_info);

	rtw_hal_final_cap_decision(phl_info->phl_com, phl_info->hal);

	phl_status = phl_datapath_init(phl_info);

	PHL_PRINT("H2C Pool: %pX, %u, %pX, %pX\n",
	          phl_info->h2c_pool,
	          phl_info->h2c_pool->busy_h2c_pkt_list.cnt,
	          phl_info->h2c_pool->busy_h2c_pkt_list.queue.next,
	          phl_info->h2c_pool->busy_h2c_pkt_list.queue.prev);

	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		phl_info->phl_com->dev_sw_cap.rx_amsdu_cut = false;
		#ifndef CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV
		phl_info->phl_com->dev_sw_cap.rx_hdr_conv = false;
		#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV */

	}

	return phl_status;
}

enum rtw_phl_status rtw_phl_disable_rx_amsdu_cut(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status phl_status;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;

	if (phl_info->phl_com->dev_state == RTW_DEV_WORKING) {
		RTW_ERR("Can not switch RX A-MSDU cut when device is enabled.\n");
		return RTW_PHL_STATUS_FAILURE;
	}

	phl_info->phl_com->dev_sw_cap.rx_amsdu_cut = false;
	#ifndef CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV
	phl_info->phl_com->dev_sw_cap.rx_hdr_conv = false;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV */

	phl_datapath_deinit(phl_info);

	rtw_hal_final_cap_decision(phl_info->phl_com, phl_info->hal);

	phl_status = phl_datapath_init(phl_info);

	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		phl_info->phl_com->dev_sw_cap.rx_amsdu_cut = true;
		#ifndef CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV
		phl_info->phl_com->dev_sw_cap.rx_hdr_conv = true;
		#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV */

	}

	return phl_status;
}

enum rtw_phl_status rtw_phl_enable_rx_hdr_conv(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	phl_info->phl_com->dev_sw_cap.rx_hdr_conv = true;

	rtw_hal_enable_rx_hdr_conv(phl_info->phl_com, phl_info->hal);
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status rtw_phl_disable_rx_hdr_conv(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_disable_rx_hdr_conv(phl_info->phl_com, phl_info->hal);

	phl_info->phl_com->dev_sw_cap.rx_hdr_conv = false;

	return RTW_PHL_STATUS_SUCCESS;
}

#endif /*CONFIG_RTW_HW_RX_AMSDU_CUT */

void rtw_phl_rf_get_thermal(void *phl, u8 rfpath, u8 *thermal)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	PHL_INFO("%s: rf path = %d\n", __func__, rfpath);

	hal_status = rtw_hal_get_thermal(phl_info->hal, rfpath, thermal);
	PHL_INFO("%s: status = %d\n", __func__, hal_status);
	PHL_INFO("%s: thermal = %d\n", __func__, *thermal);

}

void rtw_phl_rf_efuse_update(void *phl, u8 path)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);

	PHL_INFO ("%s path=%d\n", __func__, path);
	if (phl_is_mp_mode(phl_info->phl_com))
		rtw_hal_rf_get_efuse_ex(hal_com, path);
	else
		PHL_INFO ("Not in MP mode, would not reflash rf status immediately.\n");
}

#ifdef DEBUG_PHL_RX
enum rtw_phl_status rtw_phl_cmd_debug(void *phl, void* m)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	u32 idx=0;

	RTW_PRINT_SEL(m, " ========= \n");

	//===SER

	RTW_PRINT_SEL(m, "phl_tx_status=%d phl_rx_status=%d \n",
	       _os_atomic_read(NULL, &phl_info->phl_tx_status),
	       _os_atomic_read(NULL, &phl_info->phl_rx_status));

	//===TX

	//RTW_PRINT_SEL(m, "cnt_tx_sw_pause=%d \n", phl_info->cnt_tx_sw_pause);

	//===INT
	RTW_PRINT_SEL(m, "evt_bitmap = %08X\n", phl_info->phl_com->evt_info.evt_bitmap);

	if (1) {
		struct rtw_hal_com_t *hal_com = phl_info->phl_com->mr_ctrl.hal_com;
		RTW_PRINT_SEL(m, "int_mask = %08X, %08X, %08X, %08X\n",
		       hal_com->int_mask[0], hal_com->int_mask[1],
		       hal_com->int_mask[2], hal_com->int_mask[3]);
		RTW_PRINT_SEL(m, "int_mask_default = %08X, %08X, %08X, %08X\n",
		       hal_com->int_mask_default[0],
		       hal_com->int_mask_default[1],
		       hal_com->int_mask_default[2],
		       hal_com->int_mask_default[3]);
		RTW_PRINT_SEL(m, "int_array = %08X, %08X, %08X, %08X\n",
		       hal_com->int_array[0], hal_com->int_array[1],
		       hal_com->int_array[2], hal_com->int_array[3]);
		#ifdef CONFIG_RTW_DEBUG_INT
		RTW_PRINT_SEL(m, "in_int = %u, is_int_enabled = %u, int_not_handled = %u\n",
		       hal_com->in_int, hal_com->is_int_enabled,
		       hal_com->int_not_handled);
		#endif /* CONFIG_RTW_DEBUG_INT */
	}
	//===RX
	/*
	RTW_PRINT_SEL(m, "cnt_rx_int = %d \n", phl_info->cnt_rx_int);
	RTW_PRINT_SEL(m, "cnt_rx_isr = %d, empty = %d %d \n",
		phl_info->cnt_rx_isr, phl_info->cnt_rx_isr_empty[0], phl_info->cnt_rx_isr_empty[1]);
	RTW_PRINT_SEL(m, "cnt_rx_chk_skip: rxq=%d rpq=%d \n", phl_info->cnt_rx_chk_skip[0], phl_info->cnt_rx_chk_skip[1]);
	RTW_PRINT_SEL(m, "cnt_rx_pkt: all=%d, data=%d wp=%d ppdu=%d \n",
		phl_info->cnt_rx_pkt_all,
		phl_info->cnt_rx_pkt_data, phl_info->cnt_rx_pkt_wp, phl_info->cnt_rx_pkt_ppdu);
	RTW_PRINT_SEL(m, "amsdu = %d \n", phl_info->cnt_rx_pkt_amsdu);
	RTW_PRINT_SEL(m, "rx_pkt_per_isr = %d %d \n",
		phl_info->rx_pkt_per_isr[0], phl_info->rx_pkt_per_isr[1]);
	RTW_PRINT_SEL(m, "rx_handle_per_isr = %d %d \n",
		phl_info->rx_handle_per_isr[0], phl_info->rx_handle_per_isr[1]);
	*/

{
	struct rtw_rx_buf_ring *rx_buf_ring = NULL;
	rx_buf_ring = (struct rtw_rx_buf_ring *)phl_info->hci->rxbuf_pool;

	RTW_PRINT_SEL(m, "[0][%p]idle_rxbuf_cnt=%d busy_rxbuf_cnt=%d empty_rxbuf_cnt=%d\n",
		rx_buf_ring, rx_buf_ring->idle_rxbuf_cnt, rx_buf_ring->busy_rxbuf_cnt, rx_buf_ring->empty_rxbuf_cnt);

	rx_buf_ring++;

	RTW_PRINT_SEL(m, "[1][%p]idle_rxbuf_cnt=%d busy_rxbuf_cnt=%d empty_rxbuf_cnt=%d\n",
		rx_buf_ring, rx_buf_ring->idle_rxbuf_cnt, rx_buf_ring->busy_rxbuf_cnt, rx_buf_ring->empty_rxbuf_cnt);
}
	/*
	RTW_PRINT_SEL(m, "cnt_rx_no_hw_buf=%d \n", phl_info->cnt_rx_no_hw_buf);
	RTW_PRINT_SEL(m, "cnt_rx_rdy_fail=%d cnt_rx_buf_empty=%d less=%d\n",
		phl_info->cnt_rx_rdy_fail, phl_info->cnt_rx_buf_empty, phl_info->cnt_rx_buf_less);
	RTW_PRINT_SEL(m, "idle_rx_buf: out=%d in=%d (diff=%d) \n",
		phl_info->cnt_idle_rx_buf_out, phl_info->cnt_idle_rx_buf_in, (phl_info->cnt_idle_rx_buf_in-phl_info->cnt_idle_rx_buf_out));
	RTW_PRINT_SEL(m, "busy_rx_buf: out=%d in=%d (diff=%d)\n",
		phl_info->cnt_busy_rx_buf_out, phl_info->cnt_busy_rx_buf_in, (phl_info->cnt_busy_rx_buf_in-phl_info->cnt_busy_rx_buf_out));
	RTW_PRINT_SEL(m, "cnt_drop: get=%d, reorder:\n",
		phl_info->cnt_rx_drop_get);
	for (idx = 0; idx < ARRAY_SIZE(phl_info->cnt_rx_drop_reorder); idx++) {
		RTW_PRINT_SEL(m, "\t%u: %u\n", idx,
			      phl_info->cnt_rx_drop_reorder[idx]);
	}
	RTW_PRINT_SEL(m, "ampdu: orig=%d rxsc=%d (add=%d)\n",
		phl_info->cnt_rxsc_ampdu[0], phl_info->cnt_rxsc_ampdu[1], phl_info->cnt_rxsc_ampdu[2]);
	RTW_PRINT_SEL(m, "reorder: dont=%d put=%d (total=%d) (diff=%d)\n",
		phl_info->cnt_rx_dont_reorder, phl_info->cnt_rx_put_reorder, (phl_info->cnt_rx_dont_reorder + phl_info->cnt_rx_put_reorder),
		((phl_info->cnt_rx_dont_reorder + phl_info->cnt_rx_put_reorder) - phl_info->cnt_rx_pkt_add));

	RTW_PRINT_SEL(m, "re-loop: hit=%d miss=%d \n", phl_info->cnt_rx_loop_hit, phl_info->cnt_rx_loop_miss);
	RTW_PRINT_SEL(m, "cnt_rx_pkt_indicate = %d (%d, %d, %d) \n", phl_info->cnt_rx_pkt_indicate,
		phl_info->cnt_rx_pkt_indicate_type[0], phl_info->cnt_rx_pkt_indicate_type[1],
		phl_info->cnt_rx_pkt_indicate_type[2]);
	RTW_PRINT_SEL(m, "pkt: toCore=%d coreReturn=%d (diff=%d)\n",
		phl_info->cnt_rx_pkt_add, phl_info->cnt_rx_buf_return, (phl_info->cnt_rx_pkt_add-phl_info->cnt_rx_buf_return));
	*/
	RTW_PRINT_SEL(m, "phl_rx_pkt_pool: idle_cnt=%d\n", ((struct phl_rx_pkt_pool *)phl_info->rx_pkt_pool)->idle_cnt);
{
	struct rtw_phl_rx_ring *ring = NULL;
	//_os_atomic *ring_phl_idx, *ring_core_idx;
	u16 new_rx = 0, wptr = 0, rptr = 0;


	ring = &phl_info->phl_rx_ring;
	wptr = (u16)_os_atomic_read(phl_to_drvpriv(phl_info), (_os_atomic *)&ring->phl_idx);
	rptr = (u16)_os_atomic_read(phl_to_drvpriv(phl_info), (_os_atomic *)&ring->core_idx);

	/*
	//ring = &phl_info->phl_rx_ring.phl_idx;
	ring_phl_idx = (_os_atomic *)&phl_info->phl_rx_ring.phl_idx;
	ring_core_idx = (_os_atomic *)&phl_info->phl_rx_ring.core_idx;

	wptr = (u16)_os_atomic_read(phl_to_drvpriv(phl_info), ring_phl_idx);
	rptr = (u16)_os_atomic_read(phl_to_drvpriv(phl_info), ring_core_idx);
	*/
	new_rx = phl_calc_avail_rptr(rptr, wptr, MAX_PHL_RING_ENTRY_NUM);
	RTW_PRINT_SEL(m, "phl_rx_ring: phl_idx=%d, core_idx=%d, rx_pkt_num=%d\n", wptr, rptr, new_rx);
}
	//for(idx=0; idx<12; idx++)
	//	RTW_PRINT_SEL(m,"cnt_rx_rate[%d] = %d \n", idx, phl_info->cnt_rx_rate[idx]);

	RTW_PRINT_SEL(m,"\n \n");

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
rtw_phl_gpio_ctl(void *phl)
{

	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t* phl_com = phl_info->phl_com;
	struct rtw_gpio_info_t gpio_info = phl_com->gpio_info;

	switch (gpio_info.ctl_mode) {
	case RTW_PHL_GPIO_CTL_SET_MODE:
		if (gpio_info.mode_type == RTW_AX_SW_IO_MODE_INPUT)
			rtw_hal_set_sw_gpio_mode(phl_info->phl_com, phl_info->hal,
						RTW_AX_SW_IO_MODE_INPUT,gpio_info.ping_num);
		if (gpio_info.mode_type == RTW_AX_SW_IO_MODE_OUTPUT_PP)
			rtw_hal_set_sw_gpio_mode(phl_info->phl_com, phl_info->hal,
						RTW_AX_SW_IO_MODE_OUTPUT_PP,gpio_info.ping_num);
		break;
	case RTW_PHL_GPIO_CTL_READ:
		rtw_hal_set_sw_gpio_mode(phl_info->phl_com, phl_info->hal,
					RTW_AX_SW_IO_MODE_INPUT,gpio_info.ping_num);

		rtw_hal_get_sw_gpio_value(phl_info->phl_com, phl_info->hal,
					&(phl_com->gpio_info.ping_val), gpio_info.ping_num);


		break;
	case RTW_PHL_GPIO_CTL_WRITE:
		rtw_hal_set_sw_gpio_mode(phl_info->phl_com, phl_info->hal,
				RTW_AX_SW_IO_MODE_OUTPUT_PP,gpio_info.ping_num);

		rtw_hal_sw_gpio_ctrl(phl_info->phl_com, phl_info->hal,
					phl_com->gpio_info.ping_val, gpio_info.ping_num);
		break;

	default:
		printk("%s:wrong gpio ctl\n",__func__);
		phl_status = RTW_PHL_STATUS_FAILURE;
		break;
	}


	return phl_status;
}


#endif

void rtw_phl_set_dig_opmode(void *phl, u8 opmode)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct hal_info_t *hal_info = (struct hal_info_t *)(phl_info->hal);

	rtw_hal_bb_set_dig_opmode(hal_info, opmode);
}
