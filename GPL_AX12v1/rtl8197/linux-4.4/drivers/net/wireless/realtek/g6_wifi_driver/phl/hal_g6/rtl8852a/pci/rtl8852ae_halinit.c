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
#define _RTL8852AE_HALINIT_C_
#include "../../hal_headers.h"
#include "../rtl8852a_hal.h"
#include "hal_trx_8852ae.h"
static enum   mac_ax_pcie_func_ctrl
_hal_set_each_pcicfg(enum rtw_pcie_bus_func_cap_t ctrl)
{
	switch(ctrl) {

		case RTW_PCIE_BUS_FUNC_DISABLE:
			return MAC_AX_PCIE_DISABLE;
		case RTW_PCIE_BUS_FUNC_ENABLE:
			return MAC_AX_PCIE_ENABLE;
		default:
			return MAC_AX_PCIE_DEFAULT;
	}

}
enum rtw_hal_status _hal_set_pcicfg_8852ae(struct hal_info_t *hal_info)
{
	struct mac_ax_pcie_cfgspc_param pcicfg;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;

	_os_mem_set(hal_to_drvpriv(hal_info), &pcicfg, 0, sizeof(pcicfg));
	pcicfg.write = 1;
	pcicfg.l0s_ctrl = _hal_set_each_pcicfg(hal_com->bus_cap.l0s_ctrl);
	pcicfg.l1_ctrl = _hal_set_each_pcicfg(hal_com->bus_cap.l1_ctrl);
	pcicfg.l1ss_ctrl = _hal_set_each_pcicfg(hal_com->bus_cap.l1ss_ctrl);
	pcicfg.wake_ctrl = _hal_set_each_pcicfg(hal_com->bus_cap.wake_ctrl);
	pcicfg.crq_ctrl = _hal_set_each_pcicfg(hal_com->bus_cap.crq_ctrl);
	pcicfg.clkdly_ctrl = hal_com->bus_cap.clkdly_ctrl;
	pcicfg.l0sdly_ctrl = hal_com->bus_cap.l0sdly_ctrl;
	pcicfg.l1dly_ctrl = hal_com->bus_cap.l1dly_ctrl;


	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
		"%s : l0s/l1/l1ss/wake/crq/l0sdly/l1dly/clkdly = %#X/%#X/%#X/%#X/%#X/%#X/%#X/%#X \n",
		__func__, pcicfg.l0s_ctrl, pcicfg.l1_ctrl, pcicfg.l1ss_ctrl, pcicfg.wake_ctrl,
		pcicfg.crq_ctrl, pcicfg.l0sdly_ctrl, pcicfg.l1dly_ctrl, pcicfg.clkdly_ctrl);

	hsts = rtw_hal_mac_set_pcicfg(hal_info, &pcicfg);

	return hsts;
}

enum rtw_hal_status _hal_ltr_sw_init_state_8852ae(struct hal_info_t *hal_info)
{

	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;
	enum rtw_pcie_ltr_state state;

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
		"%s :sw state = %u \n",__func__, hal_com->bus_cap.ltr_init_state);

	if (hal_com->bus_cap.ltr_init_state > 0) {

		state = hal_com->bus_cap.ltr_init_state;
		hsts = rtw_hal_mac_ltr_sw_trigger(hal_info,state);

	} else {

		hsts = RTW_HAL_STATUS_SUCCESS;
	}
	return hsts;
}

enum rtw_hal_status _hal_ltr_set_pcie_8852ae(struct hal_info_t *hal_info)
{

	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
		"%s :idle_ctrl/idle_val/act_ctrl/act_val = %u/%#X/%u/%#X \n",
		__func__, hal_com->bus_cap.ltr_idle.ctrl, hal_com->bus_cap.ltr_idle.val,
				hal_com->bus_cap.ltr_act.ctrl, hal_com->bus_cap.ltr_act.val);


	hsts = rtw_hal_mac_ltr_set_pcie(hal_info,
		hal_com->bus_cap.ltr_idle.ctrl, hal_com->bus_cap.ltr_idle.val,
				hal_com->bus_cap.ltr_act.ctrl, hal_com->bus_cap.ltr_act.val);


	return hsts;
}

static void _hal_aspm_hw_cfg(struct hal_info_t *hal)
{

	struct rtw_hal_com_t *hal_com = hal->hal_com;

	/* func */
	hal_com->bus_hw_cap.l0s_ctrl = RTW_PCIE_BUS_FUNC_DEFAULT;
	hal_com->bus_hw_cap.l1_ctrl = RTW_PCIE_BUS_FUNC_ENABLE;
	hal_com->bus_hw_cap.l1ss_ctrl = RTW_PCIE_BUS_FUNC_DEFAULT;
	hal_com->bus_hw_cap.wake_ctrl = RTW_PCIE_BUS_FUNC_DEFAULT;
	hal_com->bus_hw_cap.crq_ctrl = RTW_PCIE_BUS_FUNC_DEFAULT;

	/*delay*/
	hal_com->bus_hw_cap.l0sdly_ctrl = MAC_AX_PCIE_L0SDLY_DEF;
	hal_com->bus_hw_cap.l1dly_ctrl = MAC_AX_PCIE_L1DLY_DEF;
	hal_com->bus_hw_cap.clkdly_ctrl = MAC_AX_PCIE_CLKDLY_DEF;

}

static void _hal_tx_ch_config_8852ae(struct hal_info_t *hal)
{
	struct mac_ax_txdma_ch_map *txch_map = (struct mac_ax_txdma_ch_map *)hal->txch_map;

	txch_map->ch0 = MAC_AX_PCIE_ENABLE;
	txch_map->ch1 = MAC_AX_PCIE_ENABLE;
	txch_map->ch2 = MAC_AX_PCIE_ENABLE;
	txch_map->ch3 = MAC_AX_PCIE_ENABLE;
	txch_map->ch4 = MAC_AX_PCIE_ENABLE;
	txch_map->ch5 = MAC_AX_PCIE_ENABLE;
	txch_map->ch6 = MAC_AX_PCIE_ENABLE;
	txch_map->ch7 = MAC_AX_PCIE_ENABLE;
	txch_map->ch8 = MAC_AX_PCIE_ENABLE;
	txch_map->ch9 = MAC_AX_PCIE_ENABLE;
	txch_map->ch10 = MAC_AX_PCIE_ENABLE;
	txch_map->ch11 = MAC_AX_PCIE_ENABLE;
	txch_map->ch12 = MAC_AX_PCIE_ENABLE;
}

static void _hal_pre_init_8852ae(struct rtw_phl_com_t *phl_com,
				 struct hal_info_t *hal_info,
				 struct hal_init_info_t *init_52ae)
{
	struct mac_ax_trx_info *trx_info = &init_52ae->trx_info;
	struct mac_ax_intf_info *intf_info = &init_52ae->intf_info;
	struct mac_ax_host_rpr_cfg *rpr_cfg = (struct mac_ax_host_rpr_cfg *)hal_info->rpr_cfg;
	void *txbd_buf = NULL;
	void *rxbd_buf = NULL;

	/* trx_info */
	//if (true == phl_com->dev_cap.tx_mu_ru)
	if (true == phl_com->dev_cap.fw_tx_mode)
		trx_info->trx_mode = MAC_AX_TRX_SW_MODE;
	else
		trx_info->trx_mode = MAC_AX_TRX_HW_MODE;

	if (hal_info->hal_com->dbcc_en == false)
		trx_info->qta_mode = MAC_AX_QTA_SCC;
	else
		trx_info->qta_mode = MAC_AX_QTA_DBCC;

#ifdef RTW_WKARD_LAMODE
	PHL_INFO("%s : la_mode %d\n", __func__,	phl_com->dev_cap.la_mode);
	if (phl_com->dev_cap.la_mode){
		PHL_WARN("set qta_mode = MAC_AX_QTA_LAMODE\n");
		trx_info->qta_mode = MAC_AX_QTA_LAMODE;
	}

#endif

	/* Calculate release report aggregation number from RPQ buffer size */
	if (hal_info->hal_com->bus_hw_cap.rp_buf_size) {
		u32 agg =   hal_info->hal_com->bus_hw_cap.rp_buf_size
			  - RX_BD_INFO_SIZE
		          - RX_DESC_S_SIZE_8852A;
		agg = agg / 4 - 1;
		if (agg > 121)
			agg = 121;
		rpr_cfg->agg = (u8)agg;
		rpr_cfg->agg_def = 0;
		PHL_PRINT("RP agg is %u for %u.\n",
		          rpr_cfg->agg,
		          hal_info->hal_com->bus_hw_cap.rp_buf_size);
	} else
	rpr_cfg->agg_def = 1;
	rpr_cfg->tmr_def = 1;
	#ifdef CONFIG_PHL_RELEASE_RPT_ENABLE
	rpr_cfg->txok_en = MAC_AX_FUNC_EN;
	rpr_cfg->rty_lmt_en = MAC_AX_FUNC_EN;
	rpr_cfg->lft_drop_en = MAC_AX_FUNC_EN;
	rpr_cfg->macid_drop_en = MAC_AX_FUNC_EN;
	#else
	rpr_cfg->txok_en = MAC_AX_FUNC_DEF;
	rpr_cfg->rty_lmt_en = MAC_AX_FUNC_DEF;
	rpr_cfg->lft_drop_en = MAC_AX_FUNC_DEF;
	rpr_cfg->macid_drop_en = MAC_AX_FUNC_DEF;
	#endif /* CONFIG_PHL_RELEASE_RPT_ENABLE */
	trx_info->rpr_cfg = rpr_cfg;

	/* intf_info */
	txbd_buf = rtw_phl_get_txbd_buf(phl_com);
	rxbd_buf = rtw_phl_get_rxbd_buf(phl_com);

	intf_info->txbd_trunc_mode = MAC_AX_BD_TRUNC;
	intf_info->rxbd_trunc_mode = MAC_AX_BD_TRUNC;

	intf_info->rxbd_mode = MAC_AX_RXBD_PKT;
	intf_info->tag_mode = MAC_AX_TAG_MULTI;
	intf_info->tx_burst = MAC_AX_TX_BURST_DEF;
	intf_info->rx_burst = MAC_AX_RX_BURST_DEF;
	intf_info->wd_dma_idle_intvl = MAC_AX_WD_DMA_INTVL_DEF;
	intf_info->wd_dma_act_intvl = MAC_AX_WD_DMA_INTVL_DEF;
	intf_info->multi_tag_num = MAC_AX_TAG_NUM_DEF;
	intf_info->rx_sep_append_len = 0;
	intf_info->txbd_buf = txbd_buf;
	intf_info->rxbd_buf = rxbd_buf;
	intf_info->skip_all = false;
	intf_info->autok_en = MAC_AX_FUNC_DIS;

	_hal_tx_ch_config_8852ae(hal_info);
	intf_info->txch_map = (struct mac_ax_txdma_ch_map *)hal_info->txch_map;

	intf_info->lbc_en = MAC_AX_PCIE_DEFAULT;
	intf_info->lbc_tmr = MAC_AX_LBC_TMR_DEF;

	/* others */
	init_52ae->ic_name = "rtl8852ae";
}

void init_hal_spec_8852ae(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal)
{
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_com);
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	struct bus_hw_cap_t *bus_hw_cap = &hal_com->bus_hw_cap;

	init_hal_spec_8852a(phl_com, hal);
	hal_spec->rx_bd_info_sz = RX_BD_INFO_SIZE;
	hal_spec->rx_tag[0] = 0;
	hal_spec->rx_tag[1] = 0;
	if (hal_com->cv == CAV) {
		bus_hw_cap->max_txbd_num = 0x3FFF;
		bus_hw_cap->max_rxbd_num = 0x3FFF;
		bus_hw_cap->max_rpbd_num = 0x3FFF;
		bus_hw_cap->max_rxbuf_size = RX_BUF_SIZE;
		bus_hw_cap->max_rpbuf_size = RX_BUF_SIZE;
	} else if (hal_com->cv >= CBV) {
		bus_hw_cap->max_txbd_num = 0x3FF;
		bus_hw_cap->max_rxbd_num = 0x3FF;
		bus_hw_cap->max_rpbd_num = 0x3FF;
		bus_hw_cap->max_rxbuf_size = RX_BUF_SIZE;
		bus_hw_cap->max_rpbuf_size = RX_BUF_SIZE;
	}
	bus_hw_cap->max_wd_page_size = 128;
	bus_hw_cap->wdb_size = 24;
	bus_hw_cap->wdi_size = 24;
	bus_hw_cap->txbd_len = 8;
	bus_hw_cap->rxbd_len = 8;
	bus_hw_cap->seq_info_size = 8;
	bus_hw_cap->addr_info_size = 8;
	/* phyaddr num = (wd page - (wdb + wdi + seqinfo)) / addrinfo */
#ifdef RTW_WKARD_BUSCAP_IN_HALSPEC
	hal_spec->phyaddr_num = 9;
#endif
	phl_com->dev_cap.hw_sup_flags |= HW_SUP_PCIE_PLFH;/*PCIe payload from host*/

	_hal_aspm_hw_cfg(hal);

	if (hal_com->cv <= CBV)
		bus_hw_cap->ltr_sw_ctrl = true;
	else
		bus_hw_cap->ltr_sw_ctrl = false;
	hal_spec->ser_cfg_int = false;
}

enum rtw_hal_status hal_get_efuse_8852ae(struct rtw_phl_com_t *phl_com,
					 struct hal_info_t *hal_info)
{
	struct hal_init_info_t init_52ae;
	_os_mem_set(hal_to_drvpriv(hal_info), &init_52ae, 0, sizeof(init_52ae));
	_hal_pre_init_8852ae(phl_com, hal_info, &init_52ae);

	return hal_get_efuse_8852a(phl_com, hal_info, &init_52ae);
}


static u32 _hal_read32_8852ae(struct rtw_hal_com_t *hal_com, u32 addr)
{
	u32 r_val = _os_read32_pcie(hal_com->drv_priv, addr);

	if ((addr >= R_AX_CMAC_FUNC_EN) && (addr < 0x10000)) {
		#define REG_RETRY_CNT	(10)
		//#define CMAC_CLK_ALLEN	(0xFFFFFFFF)
		int retry_cnt = REG_RETRY_CNT;

		while ((r_val == 0xDEADBEEF) && (retry_cnt > 0)) {
			/* Write register first to unlock bus read lock */
			_os_write32_pcie(hal_com->drv_priv, R_AX_CK_EN, 0xFFFFFFFF);

			r_val = _os_read32_pcie(hal_com->drv_priv, addr);
			retry_cnt--;
		}

		if (r_val == 0xDEADBEEF) {
			PHL_ERR("Read %04X failed.\n", addr);
			r_val = 0xEAEAEAEA;
		} else if (retry_cnt != REG_RETRY_CNT) {
			PHL_WARN("Read %04X error %u times\n", addr,
				 REG_RETRY_CNT - retry_cnt);
		}
	}

	return r_val;
}

static u16 _hal_read16_8852ae(struct rtw_hal_com_t *hal_com, u32 addr)
{
	u32 r_val = _hal_read32_8852ae(hal_com, (addr & ~3));
	return (u16)(r_val >> (8 * (addr & 2)));
}

static u8 _hal_read8_8852ae(struct rtw_hal_com_t *hal_com, u32 addr)
{
	u32 r_val = _hal_read32_8852ae(hal_com, (addr & ~3));
	return (u8)(r_val >> (8 * (addr & 3)));
}

enum rtw_hal_status hal_init_8852ae(struct rtw_phl_com_t *phl_com,
				    struct hal_info_t *hal_info)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;
	void *drv = phlcom_to_drvpriv(phl_com);
	u32 len = 0;
	u8 txch_num = 0;

	struct hal_io_priv *io_priv = &hal_com->iopriv;
	/* Use chip level register read handling */
	PHL_PRINT("Use chip layer register read functions!\n");
	io_priv->io_ops._read32 = _hal_read32_8852ae;
	io_priv->io_ops._read16 = _hal_read16_8852ae;
	io_priv->io_ops._read8 = _hal_read8_8852ae;

	txch_num = rtw_hal_query_txch_num(hal_info);

	len = sizeof(struct rtw_wp_rpt_stats) * txch_num;
	hal_com->trx_stat.wp_rpt_stats = _os_mem_alloc(drv, len);
	if (hal_com->trx_stat.wp_rpt_stats == NULL) {
		hal_status = RTW_HAL_STATUS_RESOURCE;
		PHL_ERR("%s: alloc txch_map failed\n", __func__);
		goto error_trx_stats_wp_rpt;
	}

	hal_info->txch_map = _os_mem_alloc(drv,
					   sizeof(struct mac_ax_txdma_ch_map));
	if (hal_info->txch_map == NULL) {
		hal_status = RTW_HAL_STATUS_RESOURCE;
		PHL_ERR("%s: alloc txch_map failed\n", __func__);
		goto error_txch_map;
	}

	hal_info->rpr_cfg = _os_mem_alloc(drv,
					  sizeof(struct mac_ax_host_rpr_cfg));
	if (hal_info->rpr_cfg == NULL) {
		hal_status = RTW_HAL_STATUS_RESOURCE;
		PHL_ERR("%s: alloc rpr_cfg failed\n", __func__);
		goto error_rpr_cfg;
	}

	hal_status = RTW_HAL_STATUS_SUCCESS;

	return hal_status;

error_rpr_cfg:
	_os_mem_free(drv,
		     hal_info->txch_map,
		     sizeof(struct mac_ax_txdma_ch_map));
error_txch_map:
	_os_mem_free(drv,
		     hal_com->trx_stat.wp_rpt_stats,
		     sizeof(struct rtw_wp_rpt_stats) * txch_num);
error_trx_stats_wp_rpt:
	return hal_status;
}

void hal_deinit_8852ae(struct rtw_phl_com_t *phl_com,
		       struct hal_info_t *hal_info)
{
	u8 txch_num = 0;

	txch_num = rtw_hal_query_txch_num(hal_info);

	_os_mem_free(phlcom_to_drvpriv(phl_com),
		     hal_info->hal_com->trx_stat.wp_rpt_stats,
		     sizeof(struct rtw_wp_rpt_stats) * txch_num);
	_os_mem_free(phlcom_to_drvpriv(phl_com),
		     hal_info->txch_map,
		     sizeof(struct mac_ax_txdma_ch_map));
	_os_mem_free(phlcom_to_drvpriv(phl_com),
		     hal_info->rpr_cfg,
		     sizeof(struct mac_ax_host_rpr_cfg));
}

enum rtw_hal_status hal_start_8852ae(struct rtw_phl_com_t *phl_com,
				    struct hal_info_t *hal_info)
{
	struct hal_init_info_t init_52ae;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	_os_mem_set(hal_to_drvpriv(hal_info), &init_52ae, 0, sizeof(init_52ae));
	_hal_pre_init_8852ae(phl_com, hal_info, &init_52ae);

	hal_status = hal_start_8852a(phl_com, hal_info, &init_52ae);
	if(RTW_HAL_STATUS_SUCCESS != hal_status) {

		PHL_ERR("hal_init_8852a: status = %u\n",hal_status);
		return hal_status;
	}

	hal_status = _hal_set_pcicfg_8852ae(hal_info);
	if(RTW_HAL_STATUS_SUCCESS != hal_status) {
		PHL_ERR("_hal_set_pcicfg_8852ae: status = %u\n",hal_status);
		return hal_status;
	}

	hal_status = _hal_ltr_set_pcie_8852ae(hal_info);
	if(RTW_HAL_STATUS_SUCCESS != hal_status) {
		PHL_ERR("_hal_ltr_set_pcie_8852ae: status = %u\n",hal_status);
		return hal_status;
	}

	hal_status = _hal_ltr_sw_init_state_8852ae(hal_info);
	if(RTW_HAL_STATUS_SUCCESS != hal_status) {
		PHL_ERR("_hal_ltr_sw_init_state_8852ae: status = %u\n",hal_status);
		return hal_status;
	}

	return hal_status;
}

enum rtw_hal_status hal_stop_8852ae(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	hal_status = hal_stop_8852a(phl_com, hal);
	return hal_status;
}

#ifdef CONFIG_WOWLAN

enum rtw_hal_status
hal_wow_init_8852ae(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal_info,
							struct rtw_phl_stainfo_t *sta)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct hal_init_info_t init_52ae;
	struct mac_ax_trx_info *trx_info = &init_52ae.trx_info;

	FUNCIN_WSTS(hal_status);

	_os_mem_set(hal_to_drvpriv(hal_info), &init_52ae, 0, sizeof(init_52ae));
	if (true == phl_com->dev_cap.tx_mu_ru)
		trx_info->trx_mode = MAC_AX_TRX_SW_MODE;
	else
		trx_info->trx_mode = MAC_AX_TRX_HW_MODE;
	trx_info->qta_mode = MAC_AX_QTA_SCC;
	/*
	if (hal_info->hal_com->dbcc_en == false)
		trx_info->qta_mode = MAC_AX_QTA_SCC;
	else
		trx_info->qta_mode = MAC_AX_QTA_DBCC;
	*/
	init_52ae.ic_name = "rtl8852ae";

	hal_status = hal_wow_init_8852a(phl_com, hal_info, sta, &init_52ae);

	FUNCOUT_WSTS(hal_status);
	return hal_status;
}

enum rtw_hal_status
hal_wow_deinit_8852ae(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal_info,
								struct rtw_phl_stainfo_t *sta)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct hal_init_info_t init_52ae;
	struct mac_ax_trx_info *trx_info = &init_52ae.trx_info;

	FUNCIN_WSTS(hal_status);

	_os_mem_set(hal_to_drvpriv(hal_info), &init_52ae, 0, sizeof(init_52ae));
	if (true == phl_com->dev_cap.tx_mu_ru)
		trx_info->trx_mode = MAC_AX_TRX_SW_MODE;
	else
		trx_info->trx_mode = MAC_AX_TRX_HW_MODE;
	trx_info->qta_mode = MAC_AX_QTA_SCC;
	/*
	if (hal_info->hal_com->dbcc_en == false)
		trx_info->qta_mode = MAC_AX_QTA_SCC;
	else
		trx_info->qta_mode = MAC_AX_QTA_DBCC;
	*/
	init_52ae.ic_name = "rtl8852ae";

	hal_status = hal_wow_deinit_8852a(phl_com, hal_info, sta, &init_52ae);

	if (RTW_HAL_STATUS_SUCCESS != hal_status) {

		PHL_ERR("hal_wow_deinit_8852ae: status = %u\n", hal_status);
		return hal_status;
	}

	if (RTW_HAL_STATUS_SUCCESS != _hal_set_pcicfg_8852ae(hal_info))
		PHL_ERR("_hal_set_pcicfg_8852ae: status = %u\n", hal_status);


	FUNCOUT_WSTS(hal_status);
	return hal_status;
}

#endif /* CONFIG_WOWLAN */

u32 hal_hci_cfg_8852ae(struct rtw_phl_com_t *phl_com,
		struct hal_info_t *hal, struct rtw_ic_info *ic_info)
{
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_com);
	hal_spec->cts2_thres_en = true;
	hal_spec->cts2_thres = 1792;

	hal_spec->txbd_multi_tag = 8;
#ifdef RTW_WKARD_TXBD_UPD_LMT
	hal_spec->txbd_upd_lmt = true;
#else
	hal_spec->txbd_upd_lmt = false;
#endif
	return RTW_HAL_STATUS_SUCCESS;
}

void hal_init_default_value_8852ae(struct hal_info_t *hal)
{
	init_default_value_8852a(hal);

	hal_init_int_default_value_8852ae(hal, INT_SET_OPT_HAL_INIT);
}

void hal_init_int_default_value_8852ae(struct hal_info_t *hal, enum rtw_hal_int_set_opt opt)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_com->int_mask_default[0] = (u32)(
		B_AX_RPQBD_FULL_INT_EN | /* RPQBD full */
		B_AX_RDU_INT_EN | /* Rx Descriptor Unavailable */
#if 0
		B_AX_RXDMA_STUCK_INT_EN | /* PCIE RX DMA Stuck */
		B_AX_TXDMA_STUCK_INT_EN | /* PCIE TX DMA Stuck */
		B_AX_PCIE_HOTRST_INT_EN | /* PCIE HOT Reset */
		B_AX_PCIE_FLR_INT_EN | /* PCIE FLR */
		B_AX_PCIE_PERST_INT_EN | /* PCIE PERST */
#endif
		B_AX_RPQDMA_INT_EN | /* RPQ DMA OK */
		B_AX_RXDMA_INT_EN | /* RXQ (Packet mode or Part2) DMA OK */
		B_AX_HS0ISR_IND_INT_EN | /* FW C2H*/
		B_AX_HD0ISR_IND_INT_EN | /* watchdog timer*/
#if 0
		B_AX_TXDMA_CH12_INT_EN | /* FWCMDQ */
		B_AX_TXDMA_CH9_INT_EN |	/*	*/
		B_AX_TXDMA_CH8_INT_EN |	/*	*/
		B_AX_TXDMA_ACH7_INT_EN | /*	*/
		B_AX_TXDMA_ACH6_INT_EN | /*	*/
		B_AX_TXDMA_ACH5_INT_EN | /*	*/
		B_AX_TXDMA_ACH4_INT_EN | /*	*/
		B_AX_TXDMA_ACH3_INT_EN | /*	*/
		B_AX_TXDMA_ACH2_INT_EN | /*	*/
		B_AX_TXDMA_ACH1_INT_EN | /*	*/
		B_AX_TXDMA_ACH0_INT_EN | /*	*/
#endif
		0);

	hal_com->int_mask_default[1] = (u32)(
#if 0
		B_AX_TXDMA_CH11_INT |
		B_AX_TXDMA_CH10_INT |
#endif
		0);

	hal_com->intr.halt_c2h_int.val_default = (u32)(
		(opt == INT_SET_OPT_WOW_RESUME_HNDL_RX ? 0 : B_AX_HALT_C2H_INT_EN) |
		0);

	hal_com->intr.watchdog_timer_int.val_default = (u32)(
		B_AX_WDT_PTFM_INT_EN |
		B_AX_GT3_INT_EN | /* CONFIG_VW_REFINE || CONFIG_ONE_TXQ */
		0);

	hal_com->int_mask[0] = hal_com->int_mask_default[0];
	hal_com->int_mask[1] = hal_com->int_mask_default[1];
	hal_com->intr.halt_c2h_int.val_mask = hal_com->intr.halt_c2h_int.val_default;
	hal_com->intr.watchdog_timer_int.val_mask = hal_com->intr.watchdog_timer_int.val_default;

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s : 0x%08X, 0x%08X, 0x%08X, 0x%08X\n", __func__,
			  hal_com->int_mask[0], hal_com->int_mask[1], hal_com->intr.halt_c2h_int.val_mask,
			  hal_com->intr.watchdog_timer_int.val_mask);
}

void hal_enable_int_8852ae(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_write32(hal_com, R_AX_PCIE_HIMR00, hal_com->int_mask[0]);
	hal_write32(hal_com, R_AX_PCIE_HIMR10, hal_com->int_mask[1]);
	hal_write32(hal_com, R_AX_HIMR0, hal_com->intr.halt_c2h_int.val_mask);
	hal_write32(hal_com, R_AX_HD0IMR, hal_com->intr.watchdog_timer_int.val_mask);

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s : 0x%04X : 0x%08X, 0x%04X : 0x%08X, 0x%04X : 0x%08X, 0x%04X : 0x%08X\n",
			__func__,
			R_AX_PCIE_HIMR00, hal_read32(hal_com, R_AX_PCIE_HIMR00),
			R_AX_PCIE_HIMR10, hal_read32(hal_com, R_AX_PCIE_HIMR10),
			R_AX_HIMR0, hal_read32(hal_com, R_AX_HIMR0),
			R_AX_HD0IMR, hal_read32(hal_com, R_AX_HD0IMR));
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s : 0x%04X : 0x%08X, 0x%04X : 0x%08X, 0x%04X : 0x%08X, 0x%04X : 0x%08X\n",
			__func__,
			R_AX_PCIE_HISR00, hal_read32(hal_com, R_AX_PCIE_HISR00),
			R_AX_PCIE_HISR10, hal_read32(hal_com, R_AX_PCIE_HISR10),
			R_AX_HISR0, hal_read32(hal_com, R_AX_HISR0),
			R_AX_HD0ISR, hal_read32(hal_com, R_AX_HD0ISR));

}

void hal_disable_int_8852ae(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_write32(hal_com, R_AX_PCIE_HIMR00, 0);
	hal_write32(hal_com, R_AX_PCIE_HIMR10, 0);
	hal_write32(hal_com, R_AX_HIMR0, 0);
	hal_write32(hal_com, R_AX_HD0IMR, 0);
}

void hal_disable_int_isr_8852ae(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_write32(hal_com, R_AX_PCIE_HIMR00, 0);
	hal_write32(hal_com, R_AX_PCIE_HIMR10, 0);
	hal_write32(hal_com, R_AX_HIMR0, 0);
}

bool hal_recognize_int_8852ae(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	bool recognized = false;

#ifndef CONFIG_SYNC_INTERRUPT
	/* disable imr before cleaning isr */
	hal_write32(hal_com, R_AX_PCIE_HIMR00, 0);
#ifndef CONFIG_DIS_TX_INT_CHECK
	hal_write32(hal_com, R_AX_PCIE_HIMR10, 0);
#endif

#endif /* CONFIG_SYNC_INTERRUPT */

	/* save isr for sw usage, handling interrupt */
	/* isr00 */
	hal_com->int_array[0] = hal_read32(hal_com, R_AX_PCIE_HISR00);
	hal_com->int_array[0] &= hal_com->int_mask[0];

#ifndef CONFIG_DIS_TX_INT_CHECK
	/* isr10 */
	hal_com->int_array[1] = hal_read32(hal_com, R_AX_PCIE_HISR10);
	hal_com->int_array[1] &= hal_com->int_mask[1];
#endif

	/* isr0 */
	if(hal_com->int_array[0] & B_AX_HS0ISR_IND_INT) {
	hal_com->intr.halt_c2h_int.intr = hal_read32(hal_com, R_AX_HISR0);
	hal_com->intr.halt_c2h_int.intr &= hal_com->intr.halt_c2h_int.val_mask;
#ifndef CONFIG_SYNC_INTERRUPT
		hal_write32(hal_com, R_AX_HIMR0, 0);
#endif
	} else {
		hal_com->intr.halt_c2h_int.intr = 0;
	}

	/* isr0 */
	if (hal_com->int_array[0] & B_AX_HD0ISR_IND_INT_EN) {
		hal_com->intr.watchdog_timer_int.intr = hal_read32(hal_com, R_AX_HD0ISR);
		hal_com->intr.watchdog_timer_int.intr &= hal_com->intr.watchdog_timer_int.val_mask;
#ifndef CONFIG_SYNC_INTERRUPT
		hal_write32(hal_com, R_AX_HD0IMR, 0);
#endif
	} else {
		hal_com->intr.watchdog_timer_int.intr = 0;
	}

	if (hal_com->int_array[0] || hal_com->int_array[1] || hal_com->intr.halt_c2h_int.intr || hal_com->intr.watchdog_timer_int.intr)
		recognized = true;

	if (hal_com->intr.halt_c2h_int.intr)
		PHL_WARN("%s : halt c2h interrupt is recognized (0x%x).\n", __func__, hal_com->intr.halt_c2h_int.intr);

#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
	if (hal_com->intr.watchdog_timer_int.intr & ~ B_AX_GT3_INT_EN)
#else
	if (hal_com->intr.watchdog_timer_int.intr)
#endif
		PHL_WARN("%s : watchdog timer is recognized (0x%x).\n", __func__, hal_com->intr.watchdog_timer_int.intr);

#ifndef CONFIG_SYNC_INTERRUPT
	/* clear isr */
	hal_write32(hal_com, R_AX_PCIE_HISR00, hal_com->int_array[0]);
#ifndef CONFIG_DIS_TX_INT_CHECK
	hal_write32(hal_com, R_AX_PCIE_HISR10, hal_com->int_array[1]);
#endif
	hal_write32(hal_com, R_AX_HISR0, hal_com->intr.halt_c2h_int.intr);
	hal_write32(hal_com, R_AX_HD0ISR, hal_com->intr.watchdog_timer_int.intr);

	/* restore imr */
	hal_write32(hal_com, R_AX_PCIE_HIMR00, hal_com->int_mask[0] & 0xFFFFFFFF);
#ifndef CONFIG_DIS_TX_INT_CHECK
	hal_write32(hal_com, R_AX_PCIE_HIMR10, hal_com->int_mask[1] & 0xFFFFFFFF);
#endif
	hal_write32(hal_com, R_AX_HIMR0, hal_com->intr.halt_c2h_int.val_mask & 0xFFFFFFFF);
	hal_write32(hal_com, R_AX_HD0IMR, hal_com->intr.watchdog_timer_int.val_mask & 0xFFFFFFFF);
#endif /* CONFIG_SYNC_INTERRUPT */
	return recognized;
}

void hal_clear_int_8852ae(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_write32(hal_com, R_AX_PCIE_HISR00, hal_com->int_array[0]);
	hal_write32(hal_com, R_AX_PCIE_HISR10, hal_com->int_array[1]);
	hal_write32(hal_com, R_AX_HISR0, hal_com->intr.halt_c2h_int.intr);
	hal_write32(hal_com, R_AX_HD0ISR, hal_com->intr.watchdog_timer_int.intr);
}

void hal_restore_int_8852ae(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_write32(hal_com, R_AX_PCIE_HIMR00, hal_com->int_mask[0] & 0xFFFFFFFF);
	hal_write32(hal_com, R_AX_PCIE_HIMR10, hal_com->int_mask[1] & 0xFFFFFFFF);
	hal_write32(hal_com, R_AX_HIMR0, hal_com->intr.halt_c2h_int.val_mask & 0xFFFFFFFF);
	hal_write32(hal_com, R_AX_HD0IMR, hal_com->intr.watchdog_timer_int.val_mask & 0xFFFFFFFF);
}

#if 0
#define BIT_BCNDERR0_MSK BIT(16)
#define BIT_BCNDMAINT0_MSK BIT(20)
#define BIT_TXBCN0OK_MSK BIT(25)
#define BIT_TXBCN0ERR_MSK BIT(26)

static u32 hal_bcn_handler_8852ae(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	if (hal_com->int_array[0] & BIT_TXBCN0OK_MSK){
		handled[0] |= BIT_TXBCN0OK_MSK;
		ret = 1;
	}

	if (hal_com->int_array[0] & BIT_TXBCN0ERR_MSK){
		handled[0] |= BIT_TXBCN0ERR_MSK;
		ret = 1;
	}

	if (hal_com->int_array[0] & BIT_BCNDERR0_MSK){
		handled[0] |= BIT_BCNDERR0_MSK;
		ret = 1;
	}

	if (hal_com->int_array[0] & BIT_BCNDMAINT0_MSK){
		handled[0] |= BIT_BCNDMAINT0_MSK;
		ret = 1;
	}

	return ret;
}
#endif

static u32 hal_rx_handler_8852ae(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	static const u32 rx_handle_irq = (
					B_AX_RXDMA_INT_EN |
					B_AX_RPQDMA_INT_EN|
					B_AX_RDU_INT_EN |
					B_AX_RPQBD_FULL_INT_EN);
	u32	handled0 = (hal_com->int_array[0] & rx_handle_irq);

	if (handled0 == 0)
		return ret;

	/* disable rx related IMR, rx thread will restore them */
	hal_com->int_mask[0] &= ~rx_handle_irq;
#ifndef CONFIG_SYNC_INTERRUPT
	hal_write32(hal_com, R_AX_PCIE_HIMR00, hal_com->int_mask[0]);
#endif /* CONFIG_SYNC_INTERRUPT */
#ifdef PHL_RXSC_ISR
	hal_com->rx_int_array = handled0;
#endif
	handled[0] |= handled0;
	ret = 1;

	if (handled0 & B_AX_RDU_INT_EN)
		hal_com->trx_stat.rx_rdu_cnt++;

	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : rx handle 0x%08X, int_array 0x%08X\n",
			  __func__, handled0, hal_com->int_array[0]);

	return ret;

}

static u32 hal_tx_handler_8852ae(struct hal_info_t *hal, u32 *handled)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	u32 handled0 = 0x0;
	u32 handled1 = 0x0;
	u32 ret = 0;

	handled0 = hal_com->int_array[0] & (
			B_AX_TXDMA_ACH0_INT_EN |
			B_AX_TXDMA_ACH1_INT_EN |
			B_AX_TXDMA_ACH2_INT_EN |
			B_AX_TXDMA_ACH3_INT_EN |
			B_AX_TXDMA_ACH4_INT_EN |
			B_AX_TXDMA_ACH5_INT_EN |
			B_AX_TXDMA_ACH6_INT_EN |
			B_AX_TXDMA_ACH7_INT_EN |
			B_AX_TXDMA_CH8_INT_EN |
			B_AX_TXDMA_CH9_INT_EN |
			B_AX_TXDMA_CH12_INT_EN);

	handled1 = hal_com->int_array[1] & (
			B_AX_TXDMA_CH10_INT |
			B_AX_TXDMA_CH11_INT);

	if(handled0 != 0 || handled1 != 0)
		ret = 1;

	handled[0] |= handled0;
	handled[1] |= handled1;

	return ret;
}

#if 0
#define BIT_SETH2CDOK_MASK BIT(16)
static u32 hal_cmd_handler_8852ae(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	if (hal_com->int_array[3] & BIT_SETH2CDOK_MASK) {
		handled[3] |= BIT_SETH2CDOK_MASK;
		ret = 1;
	}

	return ret;
}
#endif

static u32 hal_halt_c2h_handler_8852ae(struct hal_info_t *hal, u32 *handled)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	u32 ret = 0;

	if (hal_com->intr.halt_c2h_int.intr & B_AX_HALT_C2H_INT_EN) {
		handled[0] |= B_AX_HALT_C2H_INT_EN;
		ret = 1;
	}

	return ret;
}

/* CONFIG_VW_REFINE || CONFIG_ONE_TXQ */
static u32 hal_gt3_handler_8852ae(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;

	struct rtw_hal_com_t *hal_com = hal->hal_com;

	if (hal_com->intr.watchdog_timer_int.intr & B_AX_GT3_INT_EN) {
		handled[1] |= B_AX_GT3_INT_EN;
		ret = 1;
	}
	return ret;
}

static u32 hal_watchdog_timer_handler_8852ae(struct hal_info_t *hal, u32 *handled)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	u32 ret = 0;

	if (hal_com->intr.watchdog_timer_int.intr & B_AX_WDT_PTFM_INT_EN) {
		handled[1] |= B_AX_WDT_PTFM_INT_EN;
		ret = 1;
	}

	return ret;
}

__IMEM_WLAN_SECTION__
u32 hal_int_hdler_8852ae(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	u32 int_hdler_msk = 0x0;
	u32 handled[4] = {0};
	u32 generalhandled[8] = {0};
#if 0
	/* <1> beacon related */
	int_hdler_msk |= (hal_bcn_handler_8852ae(hal, handled)<<0);
#endif

	/* bit 1 : rx related */
	int_hdler_msk |= (hal_rx_handler_8852ae(hal, handled) << 1);

	/* bit 2 : tx related */
	int_hdler_msk |= (hal_tx_handler_8852ae(hal, handled) << 2);

#if 0
	/* <4> Cmd related */
	int_hdler_msk |= (hal_cmd_handler_8852ae(hal, handled)<<3);
#endif

	/*Start General interrupt type*/

	/* Ignore indicator*/
	handled[0] |= (B_AX_HD0ISR_IND_INT | B_AX_HS0ISR_IND_INT);

	/* bit 4 : halt c2h */
	int_hdler_msk |= (hal_halt_c2h_handler_8852ae(hal, generalhandled) << 4);

	/* bit 5 : watchdog timeout */
	int_hdler_msk |= (hal_watchdog_timer_handler_8852ae(hal, generalhandled) << 5);

	/* <8> gt3 timer related; CONFIG_VW_REFINE || CONFIG_ONE_TXQ */
	int_hdler_msk |= (hal_gt3_handler_8852ae(hal, generalhandled) << 7);

	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : int_hdler_msk = 0x%x\n", __func__, int_hdler_msk);

	if ((hal_com->int_array[0] & (~handled[0])) ||
		(hal_com->int_array[1] & (~handled[1])) ||
		(hal_com->int_array[2] & (~handled[2])) ||
		(hal_com->int_array[3] & (~handled[3]))) {
		PHL_WARN("%s : unhandled ISR => 0x%08X, 0x%08X, 0x%08X, 0x%08X. (handled[0-3])\n",
				__func__,
				(hal_com->int_array[0] & (~handled[0])),
				(hal_com->int_array[1] & (~handled[1])),
				(hal_com->int_array[2] & (~handled[2])),
				(hal_com->int_array[3] & (~handled[3])));
	}

	if ((hal_com->intr.halt_c2h_int.intr & (~generalhandled[0])) ||
		(hal_com->intr.watchdog_timer_int.intr & (~generalhandled[1]))) {
		PHL_WARN("%s : unhandled ISR => 0x%08X (generalhandled[0])\n",
				__func__,
				(hal_com->intr.halt_c2h_int.intr & (~generalhandled[0])));
		PHL_WARN("%s : unhandled ISR => 0x%08X (generalhandled[1])\n",
				__func__,
				(hal_com->intr.watchdog_timer_int.intr & (~generalhandled[1])));
	}

	return int_hdler_msk;
}


void hal_rx_int_restore_8852ae(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	hal_com->int_mask[0] |= (B_AX_RXDMA_INT_EN | B_AX_RPQDMA_INT_EN |
							 B_AX_RDU_INT_EN | B_AX_RPQBD_FULL_INT_EN);
#ifndef CONFIG_SYNC_INTERRUPT
	hal_write32(hal_com, R_AX_PCIE_HIMR00, hal_com->int_mask[0]);
#if 0
	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : \n%04X : %08X\n%04X : %08X\n",
			  __func__,
			  R_AX_PCIE_HIMR00, hal_read32(hal_com, R_AX_PCIE_HIMR00),
			  R_AX_PCIE_HIMR10, hal_read32(hal_com, R_AX_PCIE_HIMR10)
			  );
#endif
#endif /* CONFIG_SYNC_INTERRUPT */
}

enum rtw_hal_status
hal_mp_init_8852ae(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal_info)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct hal_init_info_t init_52ae;

	FUNCIN_WSTS(hal_status);

	_os_mem_set(hal_to_drvpriv(hal_info), &init_52ae, 0, sizeof(init_52ae));

	init_52ae.ic_name = "rtl8852ae";

	hal_status = hal_mp_init_8852a(phl_com, hal_info, &init_52ae);

	FUNCOUT_WSTS(hal_status);
	return hal_status;
}

enum rtw_hal_status
hal_mp_deinit_8852ae(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal_info)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct hal_init_info_t init_52ae;

	FUNCIN_WSTS(hal_status);

	_os_mem_set(hal_to_drvpriv(hal_info), &init_52ae, 0, sizeof(init_52ae));

	init_52ae.ic_name = "rtl8852ae";

	hal_status = hal_mp_deinit_8852a(phl_com, hal_info, &init_52ae);

	if (RTW_HAL_STATUS_SUCCESS != hal_status) {

		PHL_ERR("hal_mp_deinit_8852ae: status = %u\n", hal_status);
		return hal_status;
	}

	FUNCOUT_WSTS(hal_status);
	return hal_status;
}
