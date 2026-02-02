/******************************************************************************
 *
 * Copyright(c) 2019 - 2020 Realtek Corporation.
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
#define _RTW_PHL_C_
#include <drv_types.h>

/***************** export API to osdep/core*****************/

static const char *const _band_cap_str[] = {
	/* BIT0 */"2G",
	/* BIT1 */"5G",
	/* BIT2 */"6G",
};

static const char *const _bw_cap_str[] = {
	/* BIT0 */"20M",
	/* BIT1 */"40M",
	/* BIT2 */"80M",
	/* BIT3 */"160M",
	/* BIT4 */"80_80M",
	/* BIT5 */"5M",
	/* BIT6 */"10M",
};

static const char *const _proto_cap_str[] = {
	/* BIT0 */"b",
	/* BIT1 */"g",
	/* BIT2 */"n",
	/* BIT3 */"ac",
};

static const char *const _wl_func_str[] = {
	/* BIT0 */"P2P",
	/* BIT1 */"MIRACAST",
	/* BIT2 */"TDLS",
	/* BIT3 */"FTM",
};

static const char *const hw_cap_str = "[HW-CAP]";
void rtw_hw_dump_hal_spec(void *sel, struct dvobj_priv *dvobj)
{
	struct hal_spec_t *hal_spec = GET_HAL_SPEC(dvobj);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	struct phy_cap_t *phy_cap = phl_com->phy_cap;

	int i;

	RTW_PRINT_SEL(sel, "%s ic_name:%s\n", hw_cap_str, hal_spec->ic_name);
	RTW_PRINT_SEL(sel, "%s macid_num:%u\n", hw_cap_str, hal_spec->macid_num);
	RTW_PRINT_SEL(sel, "%s sec_cap:0x%02x\n", hw_cap_str, hal_spec->sec_cap);
	RTW_PRINT_SEL(sel, "%s sec_cam_ent_num:%u\n", hw_cap_str, hal_spec->sec_cam_ent_num);

	RTW_PRINT_SEL(sel, "%s rfpath_num_2g:%u\n", hw_cap_str, hal_spec->rfpath_num_2g);
	RTW_PRINT_SEL(sel, "%s rfpath_num_5g:%u\n", hw_cap_str, hal_spec->rfpath_num_5g);
	RTW_PRINT_SEL(sel, "%s rf_reg_path_num:%u\n", hw_cap_str, hal_spec->rf_reg_path_num);
	RTW_PRINT_SEL(sel, "%s max_tx_cnt:%u\n", hw_cap_str, hal_spec->max_tx_cnt);

	RTW_PRINT_SEL(sel, "%s tx_nss_num:%u\n", hw_cap_str, phy_cap[0].txss);
	RTW_PRINT_SEL(sel, "%s rx_nss_num:%u\n", hw_cap_str, phy_cap[0].rxss);

	RTW_PRINT_SEL(sel, "%s band_cap:", hw_cap_str);
	for (i = 0; i < BAND_CAP_BIT_NUM; i++) {
		if (((hal_spec->band_cap) >> i) & BIT0 && _band_cap_str[i])
			_RTW_PRINT_SEL(sel, "%s ", _band_cap_str[i]);
	}
	_RTW_PRINT_SEL(sel, "\n");

	RTW_PRINT_SEL(sel, "%s bw_cap:", hw_cap_str);
	for (i = 0; i < BW_CAP_BIT_NUM; i++) {
		if (((phl_com->dev_cap.bw_sup) >> i) & BIT0 && _bw_cap_str[i])
			_RTW_PRINT_SEL(sel, "%s ", _bw_cap_str[i]);
	}
	_RTW_PRINT_SEL(sel, "\n");

	RTW_PRINT_SEL(sel, "%s proto_cap:", hw_cap_str);
	for (i = 0; i < PROTO_CAP_BIT_NUM; i++) {
		if (((hal_spec->proto_cap) >> i) & BIT0 && _proto_cap_str[i])
			_RTW_PRINT_SEL(sel, "%s ", _proto_cap_str[i]);
	}
	_RTW_PRINT_SEL(sel, "\n");

#if 0 /*GEORGIA_TODO_FIXIT*/
	RTW_PRINT_SEL(sel, "%s txgi_max:%u\n", hw_cap_str, hal_spec->txgi_max);
	RTW_PRINT_SEL(sel, "%s txgi_pdbm:%u\n", hw_cap_str, hal_spec->txgi_pdbm);
#endif
	RTW_PRINT_SEL(sel, "%s wl_func:", hw_cap_str);
	for (i = 0; i < WL_FUNC_BIT_NUM; i++) {
		if (((hal_spec->wl_func) >> i) & BIT0 && _wl_func_str[i])
			_RTW_PRINT_SEL(sel, "%s ", _wl_func_str[i]);
	}
	_RTW_PRINT_SEL(sel, "\n");

#if 0 /*GEORGIA_TODO_FIXIT*/

	RTW_PRINT_SEL(sel, "%s pg_txpwr_saddr:0x%X\n", hw_cap_str, hal_spec->pg_txpwr_saddr);
	RTW_PRINT_SEL(sel, "%s pg_txgi_diff_factor:%u\n", hw_cap_str, hal_spec->pg_txgi_diff_factor);
#endif
}

void rtw_dump_phl_sta_info(void *sel, struct sta_info *sta)
{
	struct rtw_phl_stainfo_t *phl_sta = sta->phl_sta;

	if (!phl_sta)
		return;

#ifdef RTW_WKARD_REDUCE_CONNECT_LOG
	RTW_PRINT_SEL(sel, "[STA] r-idx:%d mac:%pM aid:%d mid:%d band:%d ch-bw-ofst:%d-%d-%d\n",
			phl_sta->wrole->id, phl_sta->mac_addr,
			phl_sta->aid, phl_sta->macid,
			phl_sta->chandef.band, phl_sta->chandef.chan, phl_sta->chandef.bw, phl_sta->chandef.offset);
#else
	RTW_PRINT_SEL(sel, "[PHL STA]- role-idx: %d\n", phl_sta->wrole->id);

	RTW_PRINT_SEL(sel, "[PHL STA]- mac_addr:"MAC_FMT"\n", MAC_ARG(phl_sta->mac_addr));
	RTW_PRINT_SEL(sel, "[PHL STA]- aid: %d\n", phl_sta->aid);
	RTW_PRINT_SEL(sel, "[PHL STA]- macid: %d\n", phl_sta->macid);

	RTW_PRINT_SEL(sel, "[PHL STA]- wifi_band: %d\n", phl_sta->chandef.band);
	RTW_PRINT_SEL(sel, "[PHL STA]- bw: %d\n", phl_sta->chandef.bw);
	RTW_PRINT_SEL(sel, "[PHL STA]- chan: %d\n", phl_sta->chandef.chan);
	RTW_PRINT_SEL(sel, "[PHL STA]- offset: %d\n", phl_sta->chandef.offset);
#endif
}

inline bool rtw_hw_chk_band_cap(struct dvobj_priv *dvobj, u8 cap)
{
	return GET_HAL_DATA(dvobj)->dev_cap.bw_sup & cap;
}

inline bool rtw_hw_chk_bw_cap(struct dvobj_priv *dvobj, u8 cap)
{
	return GET_HAL_DATA(dvobj)->dev_cap.bw_sup & cap;
}

inline bool rtw_hw_chk_proto_cap(struct dvobj_priv *dvobj, u8 cap)
{
	return GET_HAL_SPEC(dvobj)->proto_cap & cap;
}

inline bool rtw_hw_chk_wl_func(struct dvobj_priv *dvobj, u8 func)
{
	return GET_HAL_SPEC(dvobj)->wl_func & func;
}

inline bool rtw_hw_is_band_support(struct dvobj_priv *dvobj, u8 band)
{
	return GET_HAL_DATA(dvobj)->dev_cap.bw_sup & band_to_band_cap(band);
}

inline bool rtw_hw_is_bw_support(struct dvobj_priv *dvobj, u8 bw)
{
	return GET_HAL_DATA(dvobj)->dev_cap.bw_sup & ch_width_to_bw_cap(bw);
}

inline bool rtw_hw_is_wireless_mode_support(struct dvobj_priv *dvobj, u8 mode)
{
	u8 proto_cap = GET_HAL_SPEC(dvobj)->proto_cap;

	if (mode == WLAN_MD_11B)
		if ((proto_cap & PROTO_CAP_11B) && rtw_hw_chk_band_cap(dvobj, BAND_CAP_2G))
			return 1;

	if (mode == WLAN_MD_11G)
		if ((proto_cap & PROTO_CAP_11G) && rtw_hw_chk_band_cap(dvobj, BAND_CAP_2G))
			return 1;

	if (mode == WLAN_MD_11A)
		if ((proto_cap & PROTO_CAP_11G) && rtw_hw_chk_band_cap(dvobj, BAND_CAP_5G))
			return 1;

	#ifdef CONFIG_80211N_HT
	if (mode == WLAN_MD_11N)
		if (proto_cap & PROTO_CAP_11N)
			return 1;
	#endif

	#ifdef CONFIG_80211AC_VHT
	if (mode == WLAN_MD_11AC)
		if ((proto_cap & PROTO_CAP_11AC) && rtw_hw_chk_band_cap(dvobj, BAND_CAP_5G))
			return 1;
	#endif

	#ifdef CONFIG_80211AX_HE
	if (mode == WLAN_MD_11AX)
		if (proto_cap & PROTO_CAP_11AX)
			return 1;
	#endif
	return 0;
}


inline u8 rtw_hw_get_wireless_mode(struct dvobj_priv *dvobj)
{
	u8 proto_cap = GET_HAL_SPEC(dvobj)->proto_cap;
	u8 band_cap = GET_HAL_SPEC(dvobj)->band_cap;
	u8 wireless_mode = 0;

	if(proto_cap & PROTO_CAP_11B)
		wireless_mode |= WLAN_MD_11B;

	if(proto_cap & PROTO_CAP_11G)
		wireless_mode |= WLAN_MD_11G;

	if(band_cap & BAND_CAP_5G)
		wireless_mode |= WLAN_MD_11A;

	#ifdef CONFIG_80211N_HT
	if(proto_cap & PROTO_CAP_11N)
		wireless_mode |= WLAN_MD_11N;
	#endif

	#ifdef CONFIG_80211AC_VHT
	if(proto_cap & PROTO_CAP_11AC)
		wireless_mode |= WLAN_MD_11AC;
	#endif

	#ifdef CONFIG_80211AX_HE
	if(proto_cap & PROTO_CAP_11AX) {
			wireless_mode |= WLAN_MD_11AX;
	}
	#endif

	return wireless_mode;
}

inline u8 rtw_hw_get_band_type(struct dvobj_priv *dvobj)
{
	u8 band_cap = GET_HAL_SPEC(dvobj)->band_cap;
	u8 band_type = 0;

	if(band_cap & BAND_CAP_2G)
		band_type |= BAND_CAP_2G;

#if CONFIG_IEEE80211_BAND_5GHZ
	if(band_cap & BAND_CAP_5G)
		band_type |= BAND_CAP_5G;
#endif

#if CONFIG_IEEE80211_BAND_6GHZ
	if(band_cap & BAND_CAP_6G)
		band_type |= BAND_CAP_6G;
#endif

	return band_type;
}

inline bool rtw_hw_is_mimo_support(struct dvobj_priv *dvobj)
{
	if ((GET_HAL_TX_NSS(dvobj) == 1) &&
		(GET_HAL_RX_NSS(dvobj) == 1))
		return 0;
	return 1;
}

/*
* rtw_hw_largest_bw - starting from in_bw, get largest bw supported by HAL
* @adapter:
* @in_bw: starting bw, value of enum channel_width
*
* Returns: value of enum channel_width
*/
u8 rtw_hw_largest_bw(struct dvobj_priv *dvobj, u8 in_bw)
{
	for (; in_bw > CHANNEL_WIDTH_20; in_bw--) {
		if (rtw_hw_is_bw_support(dvobj, in_bw))
			break;
	}

	if (!rtw_hw_is_bw_support(dvobj, in_bw))
		rtw_warn_on(1);

	return in_bw;
}

u8 rtw_hw_get_mac_addr(struct dvobj_priv *dvobj, u8 *hw_mac_addr)
{
	if (rtw_phl_get_mac_addr_efuse(dvobj->phl, hw_mac_addr) != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR("%s failed\n", __func__);
		return _FAIL;
	}
	return _SUCCESS;
}


/***************** register hw *****************/
#if 0 /*GEORGIA_TODO_ADDIT*/

#define hal_trx_error_msg(ops_fun)		\
	RTW_PRINT("### %s - Error : Please hook hal_trx_ops.%s ###\n", __FUNCTION__, ops_fun)
static u8 rtw_hw_trx_ops_check(struct hal_com_t *hal_com)
{
	u8 rst = _SUCCESS;

	if (!hal_com->trx_ops.intf_hal_configure) {
		hal_trx_error_msg("intf_hal_configure");
		rst = _FAIL;
	}

	if (!hal_com->trx_ops.get_txdesc_len) {
		hal_trx_error_msg("get_txdesc_len");
		rst = _FAIL;
	}
	if (!hal_com->trx_ops.fill_txdesc_h2c) {
		hal_trx_error_msg("fill_txdesc_h2c");
		rst = _FAIL;
	}
	if (!hal_com->trx_ops.fill_txdesc_fwdl) {
		hal_trx_error_msg("fill_txdesc_fwdl");
		rst = _FAIL;
	}
	if (!hal_com->trx_ops.fill_txdesc_pkt) {
		hal_trx_error_msg("fill_txdesc_pkt");
		rst = _FAIL;
	}

#if defined(CONFIG_USB_HCI)
	if (!hal_com->trx_ops.get_bulkout_id) {
		hal_trx_error_msg("get_bulkout_id");
		rst = _FAIL;
	}
#endif

#if 0 /*GEORGIA_TODO_ADDIT*/
	if (!hal_com->trx_ops.init_xmit) {
		hal_trx_error_msg("init_xmit");
		rst = _FAIL;
	}

	if (!hal_com->trx_ops.init_recv) {
		hal_trx_error_msg("init_recv");
		rst = _FAIL;
	}

	#if defined(CONFIG_PCI_HCI)
	if (!hal_com->trx_ops.check_enough_txdesc) {
		hal_trx_error_msg("check_enough_txdesc");
		rst = _FAIL;
	}
	if (!hal_com->trx_ops.trxbd_init) {
		hal_trx_error_msg("trxbd_init");
		rst = _FAIL;
	}
	if (!hal_com->trx_ops.trxbd_deinit) {
		hal_trx_error_msg("trxbd_deinit");
		rst = _FAIL;
	}
	if (!hal_com->trx_ops.trxbd_reset) {
		hal_trx_error_msg("trxbd_reset");
		rst = _FAIL;
	}
	if (!hal_com->trx_ops.interrupt_handler) {
		hal_trx_error_msg("interrupt_handler");
		rst = _FAIL;
	}
	#endif

	#if defined(CONFIG_USB_HCI)
	#ifdef CONFIG_SUPPORT_USB_INT
	if (!hal_com->trx_ops.interrupt_handler) {
		hal_trx_error_msg("interrupt_handler");
		rst = _FAIL;
	}
	#endif
	#endif

	if (!hal_com->trx_ops.enable_interrupt) {
		hal_trx_error_msg("enable_interrupt");
		rst = _FAIL;
	}
	if (!hal_com->trx_ops.disable_interrupt) {
		hal_trx_error_msg("disable_interrupt");
		rst = _FAIL;
	}

	#if defined(CONFIG_SDIO_HCI)
	if (!hal_com->trx_ops.interrupt_handler) {
		hal_trx_error_msg("interrupt_handler");
		rst = _FAIL;
	}
	if (!hal_com->trx_ops.get_tx_addr) {
		hal_trx_error_msg("get_tx_addr");
		rst = _FAIL;
	}
	#endif
#endif
	return rst;
}
#endif

#ifdef CONFIG_RTW_MEMPOOL
extern void rtw_create_mem_pool_ex(char *name, int max, int min, int size);
extern int rtw_remove_mem_pool_ex(char *name);
void rtw_init_rxbuf_mempool(struct rtw_phl_com_t *phl_com)
{
	char name_buf[32];
	int max_num;
	int min_num;
	int size;

#ifdef CONFIG_RTW_RX_BUF_SHARING
	size = phl_com->bus_sw_cap.rxbuf_size_effective * 2;
	min_num = phl_com->bus_sw_cap.rxbuf_num_effective / 2;
#else
	size = phl_com->bus_sw_cap.rxbuf_size_effective;
	min_num = phl_com->bus_sw_cap.rxbuf_num_effective;
#endif
	max_num = min_num * 4;

	snprintf(name_buf,sizeof(name_buf), "%s%d_rxq_%u", "wifi",
	         phl_com->dev_id, size);

	rtw_create_mem_pool_ex(name_buf, max_num, min_num, size);
	RTW_PRINT("rtw_create_mem_pool_ex %s, %u, %u ,%u\n",
	          name_buf, max_num, min_num, size);

#ifdef CONFIG_RTW_RX_BUF_SHARING
	size = phl_com->bus_sw_cap.rpbuf_size_effective * 2;
	min_num = phl_com->bus_sw_cap.rpbuf_num_effective / 2;
#else
	size = phl_com->bus_sw_cap.rpbuf_size_effective;
	min_num = phl_com->bus_sw_cap.rpbuf_num_effective;
#endif
	max_num = min_num;

	snprintf(name_buf,sizeof(name_buf), "%s%d_rpq_%u", "wifi", phl_com->dev_id,
	         phl_com->bus_sw_cap.rpbuf_size_effective*2);

	rtw_create_mem_pool_ex(name_buf, max_num, min_num, size);
	RTW_PRINT("rtw_create_mem_pool_ex %s, %u, %u ,%u\n",
	          name_buf, max_num, min_num, size);
}

void rtw_fini_rxbuf_mempool(struct rtw_phl_com_t *phl_com)
{
	char name_buf[32];

	snprintf(name_buf, sizeof(name_buf), "%s%d_rxq_%u", "wifi",
	         phl_com->dev_id,
		 #ifdef CONFIG_RTW_RX_BUF_SHARING
	         phl_com->bus_sw_cap.rxbuf_size_effective*2);
		 #else
	         phl_com->bus_sw_cap.rxbuf_size_effective);
		 #endif
	RTW_PRINT("Removing memory pool %s ...\n", name_buf);
	rtw_remove_mem_pool_ex(name_buf);

	snprintf(name_buf,sizeof(name_buf), "%s%d_rpq_%u", "wifi",
	         phl_com->dev_id,
		 #ifdef CONFIG_RTW_RX_BUF_SHARING
	         phl_com->bus_sw_cap.rpbuf_size_effective*2);
		 #else
	         phl_com->bus_sw_cap.rpbuf_size_effective);
		 #endif
 	RTW_PRINT("Removing memory pool %s ...\n", name_buf);
	rtw_remove_mem_pool_ex(name_buf);
}
#endif


u8 rtw_core_deregister_phl_msg(struct dvobj_priv *dvobj)
{
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

	psts = rtw_phl_msg_hub_deregister_recver(dvobj->phl, MSG_RECV_CORE);
	if(psts	== RTW_PHL_STATUS_FAILURE) {
		RTW_ERR("%s failed\n", __func__);
		return _FAIL;
	}
	return _SUCCESS;
}

void rtw_hw_deinit(struct dvobj_priv *dvobj)
{
	if (dvobj->phl) {
		rtw_phl_trx_free(dvobj->phl);
#if 0 // def CONFIG_RTW_MEMPOOL
		if(dvobj->phl_com)
			rtw_fini_rxbuf_mempool(dvobj->phl_com);
#endif
		rtw_core_deregister_phl_msg(dvobj);
		rtw_phl_watchdog_deinit(dvobj->phl);
		rtw_phl_deinit(dvobj->phl);
		dvobj->phl = NULL;
	}

	#ifdef DBG_PHL_MEM_ALLOC
	RTW_INFO("[PHL-MEM] %s PHL memory :%d\n", __func__,
					ATOMIC_READ(&(dvobj->phl_mem)));
	#endif
}

#if 0
void dump_ic_spec(struct dvobj_priv *dvobj)
{
	struct hal_com_t *hal_com = dvobj->hal_com;
	struct hal_spec_t *hal_spec = &hal_com->hal_spec;

	RTW_INFO("dvobj:%p,hal:%p(size:%d), hal_com:%p, hal_spec:%p\n",
		dvobj, dvobj->hal_info, dvobj->hal_info_sz, hal_com, hal_spec);
	RTW_INFO("dvobj:%p, hal_com:%p, hal_spec:%p\n", dvobj, GET_HAL_DATA(dvobj), GET_HAL_SPEC(dvobj));

	RTW_INFO("[IC-SPEC]- band_cap: %x\n", GET_HAL_SPEC(dvobj)->band_cap);

	RTW_INFO("[IC-SPEC]- max_xmitbuf_nr: %d\n", GET_HAL_SPEC(dvobj)->max_xmitbuf_nr);
	RTW_INFO("[IC-SPEC]- max_xmitbuf_sz: %d\n", GET_HAL_SPEC(dvobj)->max_xmitbuf_sz);
	RTW_INFO("[IC-SPEC]- max_xmit_extbuf_nr: %d\n", GET_HAL_SPEC(dvobj)->max_xmit_extbuf_nr);
	RTW_INFO("[IC-SPEC]- max_xmit_extbuf_sz: %d\n", GET_HAL_SPEC(dvobj)->max_xmit_extbuf_sz);

	RTW_INFO("[IC-SPEC]- max_recvbuf_nr: %d\n", GET_HAL_SPEC(dvobj)->max_recvbuf_nr);
	RTW_INFO("[IC-SPEC]- max_recvbuf_sz: %d\n", GET_HAL_SPEC(dvobj)->max_recvbuf_sz);
}
#endif

#if 0 /*GEORGIA_TODO_FIXIT*/
void rtw_hw_intf_cfg(struct dvobj_priv *dvobj, struct hal_com_t *hal_com)
{
	struct hci_info_st hci_info;

	#ifdef CONFIG_PCI_HCI
	if (dvobj->interface_type == RTW_HCI_PCIE) {
		PPCI_DATA pci = dvobj_to_pci(dvobj);
		//hci_info.
	}
	#endif

	#ifdef CONFIG_USB_HCI
	if (dvobj->interface_type == RTW_HCI_USB) {
		PUSB_DATA usb = dvobj_to_usb(dvobj);
		#if 0
		u8 usb_speed; /* 1.1, 2.0 or 3.0 */
		u16 usb_bulkout_size;
		u8 nr_endpoint; /*MAX_ENDPOINT_NUM*/

		/* Bulk In , Out Pipe information */
		int RtInPipe[MAX_BULKIN_NUM];
		u8 RtNumInPipes;
		int RtOutPipe[MAX_BULKOUT_NUM];
		u8 RtNumOutPipes;
		#endif
		//hci_info
	}
	#endif

	#ifdef CONFIG_SDIO_HCI
	if (dvobj->interface_type == RTW_HCI_SDIO) {
		PSDIO_DATA sdio = dvobj_to_sdio(dvobj);

		hci_info.clock = sdio->clock;
		hci_info.timing = sdio->timing;
		hci_info.sd3_bus_mode = sdio->sd3_bus_mode;
		hci_info.block_sz = sdio->block_transfer_len;
		hci_info.align_sz = sdio->block_transfer_len;
	}
	#endif

	rtw_hal_intf_config(hal_com, &hci_info);
}
#endif

static void _hw_ic_info_cfg(struct dvobj_priv *dvobj, struct rtw_ic_info *ic_info)
{
	_rtw_memset(ic_info, 0,sizeof(struct rtw_ic_info));

	ic_info->ic_id = dvobj->ic_id;
	ic_info->hci_type = dvobj->interface_type;

	#ifdef CONFIG_PCI_HCI
	if (dvobj->interface_type == RTW_HCI_PCIE) {
		PPCI_DATA pci = dvobj_to_pci(dvobj);

	}
	#endif

	#ifdef CONFIG_USB_HCI
	if (dvobj->interface_type == RTW_HCI_USB) {
		PUSB_DATA usb = dvobj_to_usb(dvobj);

		ic_info->usb_info.usb_speed = usb->usb_speed;
		ic_info->usb_info.usb_bulkout_size = usb->usb_bulkout_size;
		ic_info->usb_info.inep_num = usb->RtNumInPipes;
		ic_info->usb_info.outep_num = usb->RtNumOutPipes;
	}
	#endif

	#ifdef CONFIG_SDIO_HCI
	if (dvobj->interface_type == RTW_HCI_SDIO) {
		PSDIO_DATA sdio = dvobj_to_sdio(dvobj);

		ic_info->sdio_info.clock = sdio->clock;
		ic_info->sdio_info.timing = sdio->timing;
		ic_info->sdio_info.sd3_bus_mode = sdio->sd3_bus_mode;
		ic_info->sdio_info.io_align_sz = 4;
		ic_info->sdio_info.block_sz = sdio->block_transfer_len;
		ic_info->sdio_info.tx_align_sz = sdio->block_transfer_len;
	}
	#endif
}
static void core_hdl_phl_evt(struct dvobj_priv *dvobj, struct phl_msg *msg)
{
	_adapter *iface;
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	struct rf_ctl_t *rfctl;
	u16 evt_id = MSG_EVT_ID_FIELD(msg->msg_id);
	u8 i = 0;

	if (evt_id == MSG_EVT_BCN_RESEND) {
		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			if(!rtw_is_adapter_up(iface))
				continue;

			if(MLME_IS_MESH(iface)
				|| MLME_IS_AP(iface)
				|| MLME_IS_ADHOC_MASTER(iface)) {
				if (send_beacon(iface) == _FAIL)
					RTW_ERR(ADPT_FMT" issue_beacon, fail!\n",
								ADPT_ARG(iface));
			}
		}
	}
	else if (evt_id == MSG_EVT_SER_L2) {
		#ifdef CONFIG_RTW_HANDLE_SER_L2
		rtw_recover_dev_cmd(dvobj_get_primary_adapter(dvobj));
		#else
		RTW_INFO("RECV PHL MSG_EVT_SER_L2\n");
		#endif /* CONFIG_RTW_HANDLE_SER_L2 */

	}
#ifdef CONFIG_DFS_CSA_IE
	else if (evt_id == MSG_EVT_CSA_COUNTDOWN_ZERO) {
		struct rtw_csa_cntdown_rpt *csa_rpt = (struct rtw_csa_cntdown_rpt *)msg->inbuf;
		//RTW_PRINT("csa countdown msg, inbuf: %x, inlen: %u\n", (msg->inlen > 0) ? *(msg->inbuf) : 255, msg->inlen);

		if (csa_rpt->band == 0 && csa_rpt->port == 0) {
			// bnad0 root CSA C2H
			iface = dvobj_get_primary_adapter(dvobj);
			rfctl = adapter_to_rfctl(iface);
			if (rtw_set_csa_cmd(iface) != _SUCCESS) {
				rfctl->csa_ch = 0;
				rfctl->csa_cntdown = 0;
				rfctl->csa_set_ie = 0;
			}
		}
	}
#endif
	else if (evt_id == MSG_EVT_DFS_RD_IS_DETECTING) {
		RTW_INFO("MSG_EVT_DFS_RD_IS_DETECTING\n");
		iface = dvobj_get_primary_adapter(dvobj);
		rfctl = adapter_to_rfctl(iface);

		/* if nor under cac time and cur channle is dfs ch, do tx pause */
		if (!IS_UNDER_CAC(rfctl) && rtw_chset_is_dfs_ch(rfctl->channel_set, iface->mlmeextpriv.cur_channel))
			rtw_dfs_csa_hw_tx_pause(iface, _TRUE);

		/* workaounrd for non-dfs channel but DFS detecting is on */
		if (!rtw_chset_is_dfs_ch(rfctl->channel_set, iface->mlmeextpriv.cur_channel)) {
			phl_com->dfs_info.is_radar_detectd = false;
			RTW_PRINT("DFS state false detect, cur_ch:%d\n", iface->mlmeextpriv.cur_channel);
		}
	}
	else {
		RTW_INFO("%s evt_id :%d\n", __func__, evt_id);
	}
}

void core_handler_phl_msg(void *drv_priv, struct phl_msg *msg)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)drv_priv;
	u8 mdl_id = MSG_MDL_ID_FIELD(msg->msg_id);

	switch (mdl_id)
	{
	case PHL_MDL_RX:
	case PHL_MDL_SER:
	case PHL_MDL_WOW:
		core_hdl_phl_evt(dvobj, msg);
		break;
	default:
		RTW_ERR("%s mdl_id :%d not support\n", __func__, mdl_id);
		break;
	}
}

u8 rtw_core_register_phl_msg(struct dvobj_priv *dvobj)
{
	struct phl_msg_receiver ctx = {0};
	u8 imr[] = {PHL_MDL_RX, PHL_MDL_SER, PHL_MDL_WOW};
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

	ctx.incoming_evt_notify = core_handler_phl_msg;
	ctx.priv = (void*)dvobj;

	psts = rtw_phl_msg_hub_register_recver(dvobj->phl, &ctx, MSG_RECV_CORE);
	if(psts	== RTW_PHL_STATUS_FAILURE) {
		RTW_ERR("phl_msg_hub_register failed\n");
		return _FAIL;
	}

	psts = rtw_phl_msg_hub_update_recver_mask(dvobj->phl,
					MSG_RECV_CORE, imr, sizeof(imr), false);
	if(psts	== RTW_PHL_STATUS_FAILURE) {
		RTW_ERR("phl_msg_hub_update_recver_mask failed\n");
		return _FAIL;
	}
	return _SUCCESS;
}

/* Temporary solution to fix mutex in non-sleep context issue
 * before completely applied command dispatcher flow */
#if (PHL_VER_CODE >= PHL_VERSION(0001, 0017, 0000, 0000))
#ifdef CONFIG_RTW_HALMAC_USE_SPINLOCK
static void _hal_mac_lock_init(void *h, void *lock)
{
	_rtw_spinlock_init((_os_lock *)lock);
}

static void _hal_mac_lock_deinit(void *h, void *lock)
{
	_rtw_spinlock_free((_os_lock *)lock);
}

static void _hal_mac_lock(void *h, void *lock)
{
	_rtw_spinlock_bh((_os_lock *)lock);
}

static void _hal_mac_unlock(void *h, void *lock)
{
	_rtw_spinunlock_bh((_os_lock *)lock);
}

#include "../phl/hal_g6/mac/mac_def.h"
#include "../phl/phl_regulation.h"
#include "../phl/phl_cmd_dispatch.h"
#include "../phl/phl_watchdog.h"
#include "../phl/phl_wow.h"
#include "../phl/phl_struct.h"
#include "../phl/hal_g6/hal_struct.h"

typedef void (*halmac_lock_init_f)(void *drv_adapter, mac_ax_mutex *mutex);
typedef void (*halmac_lock_deinit_f)(void *drv_adapter, mac_ax_mutex *mutex);
typedef void (*halmac_lock_f)(void *drv_adapter, mac_ax_mutex *mutex);
typedef void (*halmac_unlock_f)(void *drv_adapter, mac_ax_mutex *mutex);

extern struct mac_ax_pltfm_cb rtw_plt_cb;
struct hal_info_t;
struct mac_ax_adapter;
struct mac_ax_ops;

extern u32 mac_ax_ops_exit(struct mac_ax_adapter *adapter);
extern u32 mac_ax_ops_init(void *drv_adapter,
                           struct mac_ax_pltfm_cb *pltfm_cb,
                           enum mac_ax_intf intf,
                           struct mac_ax_adapter **mac_adapter,
                           struct mac_ax_ops **mac_ops);
extern u32 mac_ax_phl_init(void *phl_adapter,
                           struct mac_ax_adapter *mac_adapter);
#endif /* CONFIG_RTW_HALMAC_USE_SPINLOCK */
#endif /* PHL_VER_CODE */


#ifdef CONFIG_PLATFORM_RTL8198D
/* for eFem board check */
static void _check_efem_status(void) {
#if defined(CONFIG_WLAN0_RFE_TYPE_51) || defined(CONFIG_WLAN1_RFE_TYPE_51) || \
		defined(CONFIG_WLAN0_RFE_TYPE_52) || defined(CONFIG_WLAN1_RFE_TYPE_52) || \
		defined(CONFIG_WLAN0_RFE_TYPE_53) || defined(CONFIG_WLAN1_RFE_TYPE_53) || \
		defined(CONFIG_WLAN0_RFE_TYPE_54) || defined(CONFIG_WLAN1_RFE_TYPE_54)

#define REG32(reg) (*(volatile unsigned int *)(reg))
#define RTL_98Dx_PABCD_DIR 0xb8003308
#define RTL_98Dx_PABCD_DAT 0xb800330C

	//Enable the eFEM Power 5V
	REG32(RTL_98Dx_PABCD_DIR) = REG32(RTL_98Dx_PABCD_DIR) | 0x10;
	REG32(RTL_98Dx_PABCD_DAT) = REG32(RTL_98Dx_PABCD_DAT) | 0x10;
#endif
}
#endif

u8 rtw_hw_init(struct dvobj_priv *dvobj)
{
	u8 rst = _FAIL;
	enum rtw_phl_status phl_status;
	struct rtw_ic_info ic_info;
	struct rtw_phl_evt_ops *evt_ops;

#ifdef DBG_PHL_MEM_ALLOC
	ATOMIC_SET(&dvobj->phl_mem, 0);
#endif

	_hw_ic_info_cfg(dvobj, &ic_info);
	phl_status = rtw_phl_init(dvobj, &(dvobj->phl), &ic_info);

	if ((phl_status != RTW_PHL_STATUS_SUCCESS) || (dvobj->phl == NULL)) {
		RTW_ERR("%s - rtw_phl_init failed status(%d), dvobj->phl(%p)\n",
			__func__, phl_status, dvobj->phl);
		return rst;
	}
	/*init hw cap from efuse,fw defeature,mac,phy*/
	/*rtw_hal_read_chip_info()*/

	dvobj->phl_com = rtw_phl_get_com(dvobj->phl);

	#if (PHL_VER_CODE >= PHL_VERSION(0001, 0017, 0000, 0000))
	#ifdef CONFIG_RTW_HALMAC_USE_SPINLOCK
	/* Temporary solution before completely applied command dispatcher flow */
	do {
		struct phl_info_t *phl_info = (struct phl_info_t *)dvobj->phl;
		struct hal_info_t *hal_info = (struct hal_info_t *)phl_info->hal;
		u32 hal_ret;
		enum mac_ax_intf intf = MAC_AX_INTF_PCIE;
		struct mac_ax_adapter *mac = NULL;
		struct mac_ax_ops *mac_ops;

		/* De-init halmac before change lock APIs */
		hal_ret = mac_ax_ops_exit(hal_info->mac);
		if (hal_ret != 0)
			RTW_ERR(FUNC_DEVID_FMT": De-init halmac failed. (%u)\n",
			        FUNC_DEVID_ARG(dvobj), hal_ret);

		rtw_plt_cb.rtl_mutex_init = (halmac_lock_init_f)_hal_mac_lock_init;
		rtw_plt_cb.rtl_mutex_deinit = (halmac_lock_deinit_f)_hal_mac_lock_deinit;
		rtw_plt_cb.rtl_mutex_lock = (halmac_lock_f)_hal_mac_lock;
		rtw_plt_cb.rtl_mutex_unlock = (halmac_unlock_f)_hal_mac_unlock;

		/* Re-init halmac to apply new lock APIs */
		hal_ret = mac_ax_ops_init(hal_info->hal_com,
		                          &rtw_plt_cb, intf, &mac, &mac_ops);
		if (hal_ret != 0)
			RTW_ERR(FUNC_DEVID_FMT": Re-init halmac failed. (%u)\n",
			        FUNC_DEVID_ARG(dvobj), hal_ret);
		#if MAC_AX_PHL_H2C
		if (hal_ret == 0 && mac != NULL)
			mac_ax_phl_init(dvobj->phl_com, mac);
		#endif
		hal_info->mac = mac;
	} while (0);
	#endif /* CONFIG_RTW_HALMAC_USE_SPINLOCK */
	#endif /* PHL_VER_CODE */

	#ifdef CONFIG_RTW_DRV_HAS_NVM
	/* Initialize NVM efuse map after PHL is initialized */
	rtw_nvm_efuse_init(dvobj);
	#endif /* CONFIG_RTW_DRV_HAS_NVM */

	/*init sw cap from registary*/
	/* sw & hw cap*/
	rtw_core_update_default_setting(dvobj);

	rtw_phl_cap_pre_config(dvobj->phl);

#if 0 // def CONFIG_RTW_MEMPOOL
	rtw_init_rxbuf_mempool(dvobj->phl_com);
#endif

	rtw_phl_trx_alloc(dvobj->phl);

	evt_ops = &(dvobj->phl_com->evt_ops);
	evt_ops->rx_process = rtw_core_rx_process;
	evt_ops->tx_recycle = rtw_core_tx_recycle;
	evt_ops->issue_null_data = rtw_core_issu_null_data;
	rtw_core_register_phl_msg(dvobj);
#ifdef CONFIG_VW_REFINE
	evt_ops->tx_dev_map = rtw_core_tx_dev_map;
#endif

	/* load wifi feature or capability from efuse*/
	//rtw_phl_preload(dvobj->phl); /* disable access efuse on AP SOC platform */

	rtw_phl_final_cap_decision(dvobj->phl);

	rtw_hw_dump_hal_spec(RTW_DBGDUMP, dvobj);

#ifdef CONFIG_CMD_GENERAL
	rtw_phl_watchdog_init(dvobj->phl,
				2000,
				rtw_core_watchdog_sw_hdlr,
				rtw_core_watchdog_hw_hdlr);
#else
	rtw_phl_job_reg_wdog(dvobj->phl,
			rtw_dynamic_check_handlder,
                        dvobj, NULL, 0, "rtw_dm", PWR_BASIC_IO);
#endif

#ifdef CONFIG_PLATFORM_RTL8198D
	/* for FEM check */
	_check_efem_status();
#endif

	rst = _SUCCESS;
	return rst;
}

u8 rtw_hw_start(struct dvobj_priv *dvobj)
{
	u32 dw0, dw1;

#ifdef CONFIG_RTW_DRV_HAS_NVM
	/* Get HW calibration info from flash */
	RTW_PRINT ("[%s %d] Start flash process !!\n", __FUNCTION__, __LINE__);
	rtw_hal_flash_process(dvobj);
#endif
	if (rtw_phl_start(GET_HAL_INFO(dvobj)) != RTW_PHL_STATUS_SUCCESS)
		return _FAIL;

	#ifdef CONFIG_PCI_HCI
	//intr init flag
	dvobj_to_pci(dvobj)->irq_enabled = 1;
	#endif

#ifdef CONFIG_CMD_GENERAL
	rtw_phl_watchdog_start(dvobj->phl);
#endif

	dw0 = _rtw_phl_read32_(rtw_init_mask0);
	dw1 = _rtw_phl_read32_(rtw_init_mask1);
	if (rtw_phl_prepare_fw_init(dvobj->phl, dw0, dw1) != RTW_PHL_STATUS_SUCCESS)
		return _FAIL;

	return _SUCCESS;
}
void rtw_hw_stop(struct dvobj_priv *dvobj)
{
#ifdef CONFIG_CMD_GENERAL
	rtw_phl_watchdog_stop(dvobj->phl);
#endif
	rtw_phl_stop(GET_HAL_INFO(dvobj));

	#ifdef CONFIG_PCI_HCI
	//intr init flag
	dvobj_to_pci(dvobj)->irq_enabled = 0;
	#endif
}

#ifdef TX_BEAMFORMING
void rtw_core_snd_stop(struct dvobj_priv *dvobj)
{
	rtw_phl_snd_stop(GET_HAL_INFO(dvobj));
}
#endif

bool rtw_hw_get_init_completed(struct dvobj_priv *dvobj)
{
	return rtw_phl_is_init_completed(GET_HAL_INFO(dvobj));
}

bool rtw_hw_is_init_completed(struct dvobj_priv *dvobj)
{
	return (rtw_phl_is_init_completed(GET_HAL_INFO(dvobj))) ? _TRUE : _FALSE;
}

#define NSS_VALID(nss) (nss > 0)
void rtw_hw_cap_init(struct dvobj_priv *dvobj)
{
	struct hal_spec_t *hal_spec = GET_HAL_SPEC(dvobj);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	struct phy_cap_t *phy_cap = phl_com->phy_cap;
	struct registry_priv  *regpriv =
		&(dvobj_get_primary_adapter(dvobj)->registrypriv);

#ifdef DIRTY_FOR_WORK

	dvobj->phl_com->rf_path_num = hal_spec->rf_reg_path_num; /*GET_HAL_RFPATH_NUM*/
	dvobj->phl_com->rf_type = RF_2T2R; /*GET_HAL_RFPATH*/

	/* GEORGIA_TODO move related control module to phl layer*/
	/* macid_ctl moved to phl */
	/* dvobj->macid_ctl.num = rtw_min(hal_spec->macid_num, MACID_NUM_SW_LIMIT); */
	// Freddie ToDo: check macid_number from PHL?
	dvobj->wow_ctl.wow_cap = hal_spec->wow_cap;

	dvobj->cam_ctl.sec_cap = hal_spec->sec_cap;
	dvobj->cam_ctl.num = rtw_min(hal_spec->sec_cam_ent_num, SEC_CAM_ENT_NUM_SW_LIMIT);
#endif
}


/*
 * _ch_offset_drv2phl() - Convert driver channel offset to PHL type
 * @ch_offset:	channel offset, ref: HAL_PRIME_CHNL_OFFSET_*
 *
 * Return PHL channel offset type "enum chan_offset"
 */
static enum chan_offset _ch_offset_drv2phl(u8 ch_offset)
{
	if (ch_offset == CHAN_OFFSET_UPPER)
		return CHAN_OFFSET_UPPER;
	if (ch_offset == CHAN_OFFSET_LOWER)
		return CHAN_OFFSET_LOWER;

	return CHAN_OFFSET_NO_EXT;
}

/*
 * rtw_hw_set_ch_bw() - Set channel, bandwidth and channel offset
 * @a:		pointer of struct _ADAPTER
 * @ch:		channel
 * @bw:		bandwidth
 * @offset:	channel offset, ref: HAL_PRIME_CHNL_OFFSET_*
 *
 * Set channel, bandwidth and channel offset.
 *
 * Return 0 for success, otherwise fail
 */
int rtw_hw_set_ch_bw(struct _ADAPTER *a, u8 ch, enum channel_width bw,
		      u8 offset, u8 do_rfk)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	int err = 0;
	struct rtw_chan_def chdef = {0};
	enum phl_cmd_type cmd_type = PHL_CMD_DIRECTLY;
#ifdef RTW_WKARD_CUSTOM_PWRLMT_EN
	struct wifi_mib_priv *mib = &a->registrypriv.wifi_mib;
#endif
	u32 cmd_timeout = 0;

	chdef.chan = (u8)ch;
	chdef.bw = bw;
	chdef.offset = offset;
	chdef.band = rtw_phl_get_band_type(ch);
	chdef.center_ch = rtw_phl_get_center_ch(&chdef);

	status = rtw_phl_cmd_set_ch_bw(a->phl_role,
					&chdef, do_rfk,
					cmd_type, cmd_timeout);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		err = -1;
		RTW_ERR("%s: set ch(%u) bw(%u) offset(%u) FAIL!\n",
			__func__, ch, bw, offset);
	}

#ifdef RTW_WKARD_CUSTOM_PWRLMT_EN
	if (mib->txpwr_lmt_index)
		rtw_phl_set_rf_regulation_idx(GET_HAL_INFO(a->dvobj), (mib->txpwr_lmt_index + 16));
#endif

	return err;
}

void rtw_hw_update_chan_def(_adapter *adapter)
{
	struct mlme_ext_priv *mlmeext = &(adapter->mlmeextpriv);

	/*update chan_def*/
	adapter->phl_role->chandef.band = rtw_phl_get_band_type(mlmeext->cur_channel);
	adapter->phl_role->chandef.chan = mlmeext->cur_channel;
	adapter->phl_role->chandef.bw = mlmeext->cur_bwmode;
	adapter->phl_role->chandef.offset = mlmeext->cur_ch_offset;
	adapter->phl_role->chandef.center_ch = rtw_get_center_ch(adapter->phl_role->chandef.chan,
	                                                         adapter->phl_role->chandef.bw,
	                                                         adapter->phl_role->chandef.offset);
}

static void _dump_phl_role_info(struct rtw_wifi_role_t *wrole)
{
	RTW_INFO("[WROLE]- role-idx: %d\n", wrole->id);

	RTW_INFO("[WROLE]- type: %d\n", wrole->type);
	RTW_INFO("[WROLE]- mstate: %d\n", wrole->mstate);
	RTW_INFO("[WROLE]- mac_addr:"MAC_FMT"\n", MAC_ARG(wrole->mac_addr));
	RTW_INFO("[WROLE]- hw_band: %d\n", wrole->hw_band);
	RTW_INFO("[WROLE]- hw_port: %d\n", wrole->hw_port);
	RTW_INFO("[WROLE]- hw_wmm: %d\n", wrole->hw_wmm);

	RTW_INFO("[WROLE]- band: %d\n", wrole->chandef.band);
	RTW_INFO("[WROLE]- chan: %d\n", wrole->chandef.chan);
	RTW_INFO("[WROLE]- bw: %d\n", wrole->chandef.bw);
	RTW_INFO("[WROLE]- offset: %d\n", wrole->chandef.offset);
#ifdef CONFIG_RTW_SUPPORT_MBSSID_VAP
	RTW_INFO("[WROLE]- mbssid: %d\n", wrole->hw_mbssid);
#endif /* CONFIG_RTW_SUPPORT_MBSSID_VAP */
}

#define HW_CAP_STBC_ALL_TX_80M	(  HW_CAP_STBC_HT_TX \
                       		 | HW_CAP_STBC_VHT_TX \
				 | HW_CAP_STBC_HE_TX)
#define HW_CAP_STBC_ALL_TX	(  HW_CAP_STBC_ALL_TX_80M \
				 | HW_CAP_STBC_HE_TX_GT_80M)
#define HW_CAP_STBC_ALL_RX_80M	(  HW_CAP_STBC_HT_RX \
                       		 | HW_CAP_STBC_VHT_RX \
				 | HW_CAP_STBC_HE_RX)
#define HW_CAP_STBC_ALL_RX	(  HW_CAP_STBC_ALL_RX_80M \
				 | HW_CAP_STBC_HE_RX_GT_80M)

void rtw_update_sw_cap(_adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
#ifdef TX_BEAMFORMING
	dvobj->phl_com->role_sw_cap.bf_cap = 0;

	if(adapter->registrypriv.wifi_mib.txbf) {
		if (adapter->registrypriv.wifi_mib.txbfer) {
			dvobj->phl_com->role_sw_cap.bf_cap |= (HW_CAP_BFER_VHT_SU |
				HW_CAP_BFER_HE_SU);
		}
		if (adapter->registrypriv.wifi_mib.txbfee) {
			dvobj->phl_com->role_sw_cap.bf_cap |= (HW_CAP_BFEE_VHT_SU |
				HW_CAP_BFEE_HE_SU);
		}
	}

	if(adapter->registrypriv.wifi_mib.txbf_mu)
	{
		dvobj->phl_com->role_sw_cap.bf_cap |= (HW_CAP_BFER_VHT_MU |
			HW_CAP_BFEE_VHT_MU |
			HW_CAP_BFER_HE_MU |
			HW_CAP_BFEE_HE_MU);
	}
	RTW_PRINT("%s : sw_role_cap->bf_cap %d\r\n", __func__, dvobj->phl_com->role_sw_cap.bf_cap);
#endif

	dvobj->phl_com->role_sw_cap.stbc_cap = 0;
	/* STBC Rx */
	if (adapter->registrypriv.wifi_mib.stbc & STBC_RX_EN) {
		if (dvobj->phl_com->proto_sw_cap[0].stbc_rx_greater_80mhz)
			dvobj->phl_com->role_sw_cap.stbc_cap |= HW_CAP_STBC_ALL_RX;
		else
			dvobj->phl_com->role_sw_cap.stbc_cap |= HW_CAP_STBC_ALL_RX_80M;
	}

	/* STBC Tx */
	if (adapter->registrypriv.wifi_mib.stbc & STBC_TX_EN) {
		if (dvobj->phl_com->proto_sw_cap[0].stbc_tx_greater_80mhz)
			dvobj->phl_com->role_sw_cap.stbc_cap |= HW_CAP_STBC_ALL_TX;
		else
			dvobj->phl_com->role_sw_cap.stbc_cap |= HW_CAP_STBC_ALL_TX_80M;
	}
}


#ifdef CONFIG_RTW_LINK_PHL_MASTER
static inline bool IS_AP_ROLE_TYPE(enum role_type t)
{
	return (   (t == PHL_RTYPE_AP)
#ifdef CONFIG_RTW_SUPPORT_MBSSID_VAP
		|| (t == PHL_RTYPE_VAP)
#endif
	       );
}
#endif /* CONFIG_RTW_LINK_PHL_MASTER */

u8 rtw_hw_iface_init(_adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct net_device *ndev = adapter->pnetdev;
	enum role_type    role_type =   (   ndev->ieee80211_ptr->iftype
					 == NL80211_IFTYPE_AP)
				      ? PHL_RTYPE_AP
				      : PHL_RTYPE_STATION;
	u8 phl_role_idx = INVALID_WIFI_ROLE_IDX;
	u8 rst = _FAIL;
	_adapter *primary_adapter = GET_PRIMARY_ADAPTER(adapter);
	struct rtw_chan_def chdef = {0};

	#ifdef CONFIG_RTW_SUPPORT_MBSSID_VAP
	/* Creating VAP (M-BSSID) if primary adapter is an AP */
	if (role_type == PHL_RTYPE_AP && !is_primary_adapter(adapter)) {
		_adapter *prim_adp = GET_PRIMARY_ADAPTER(adapter);
		if (MLME_IS_AP(prim_adp))
			role_type = PHL_RTYPE_VAP;
	}
	#endif /* CONFIG_RTW_SUPPORT_MBSSID_VAP */

	/* Change sw_cap from mib */
	rtw_update_sw_cap(adapter);

	RTW_INFO("Initializing HW interface of type %d...", role_type);

	/* will allocate phl self sta info */
	phl_role_idx = rtw_phl_wifi_role_alloc(GET_HAL_INFO(dvobj),
					       adapter_mac_addr(adapter),
					       role_type, adapter->iface_id,
					       &(adapter->phl_role), _FALSE);

	if ((phl_role_idx == INVALID_WIFI_ROLE_IDX) ||
		(adapter->phl_role == NULL)) {
		RTW_ERR("rtw_phl_wifi_role_alloc failed\n");
		rtw_warn_on(1);
		goto _error;
	}

	/*init default value*/
	if (adapter != primary_adapter) {
		adapter->mlmeextpriv.cur_channel = primary_adapter->mlmeextpriv.cur_channel;
		adapter->mlmeextpriv.cur_bwmode = primary_adapter->mlmeextpriv.cur_bwmode;
		adapter->mlmeextpriv.cur_ch_offset = primary_adapter->mlmeextpriv.cur_ch_offset;
	}
	rtw_hw_update_chan_def(adapter);

	/* Put adapter to role for RX process */
	adapter->phl_role->core_data = adapter;

	#ifdef CONFIG_RTW_SUPPORT_MBSSID_VAP
	/* AP's channel is set when configuraing beacon */
	if (!IS_AP_ROLE_TYPE(role_type))
	#endif /* CONFIG_RTW_SUPPORT_MBSSID_VAP */
	{
		chdef.chan = (u8)adapter->phl_role->chandef.chan;
		chdef.bw = adapter->phl_role->chandef.bw;
		chdef.offset = adapter->phl_role->chandef.offset;
		chdef.band = rtw_phl_get_band_type(chdef.chan);
		chdef.center_ch = rtw_phl_get_center_ch(&chdef);
		rtw_phl_cmd_set_ch_bw(adapter->phl_role,
					&chdef, _TRUE,
					PHL_CMD_WAIT, 0);
	}
	#if 0
	RTW_PRINT(FUNC_ADPT_FMT "C%u B%u O%u (P: C%u B%u O%u)\n",
		  FUNC_ADPT_ARG(adapter),
		  adapter->phl_role->chandef.chan,
		  adapter->phl_role->chandef.bw,
		  adapter->phl_role->chandef.offset,
		  primary_adapter->mlmeextpriv.cur_channel,
		  primary_adapter->mlmeextpriv.cur_bwmode,
		  primary_adapter->mlmeextpriv.cur_ch_offset);
	#endif
	_dump_phl_role_info(adapter->phl_role);

	/* init self staion info after wifi role alloc */
#ifdef RTW_WKARD_AP_CMD_DISPATCH
	rst = rtw_init_self_stainfo(adapter, PHL_CMD_DIRECTLY);
#else
	rst = rtw_init_self_stainfo(adapter, PHL_CMD_WAIT);
#endif



	if (rst != _SUCCESS) {
		RTW_ERR("rtw_init_self_stainfo failed\n");
		rtw_warn_on(1);
		goto _free_wrole;
	}

#ifdef RTW_WKARD_98D_INTR_EN_TIMING
	if (is_primary_adapter(adapter)) {
		RTW_PRINT("[%s] enable interrupt in the END of init\n", __func__);
		rtw_phl_enable_interrupt(GET_HAL_INFO(dvobj));
	}
#endif

#ifdef CONFIG_CORE_TXSC
	txsc_clear(adapter, 1);
#endif

#ifdef TX_BEAMFORMING
	if (/*role_type == PHL_RTYPE_AP &&*/ is_primary_adapter(adapter)) {
		if(adapter->registrypriv.wifi_mib.txbf_auto_snd){
		if(adapter->registrypriv.wifi_mib.txbf&&
			adapter->registrypriv.wifi_mib.txbfer) {
			int sounding_flag = 0;
			if(adapter->registrypriv.wifi_mib.txbf_mu_2ss)
				sounding_flag |= SND_ENABLE_TXBF_MU_2SS;
			if(adapter->registrypriv.wifi_mib.txbf_tp_limit)
				sounding_flag |= SND_DISABLE_TP_LIMIT;
			if(adapter->registrypriv.wifi_mib.txbf_offload)
				sounding_flag |= SND_OFFLOAD;
			rtw_phl_enable_sounding(GET_HAL_INFO(dvobj),
				adapter->registrypriv.wifi_mib.txbf_period,
				adapter->registrypriv.wifi_mib.txbf_mu,
				sounding_flag);

			//MU DEMO
			if(adapter->registrypriv.wifi_mib.txbf_mu_1ss)
			{
				rtw_phl_mu_change_proto_cap(GET_HAL_INFO(dvobj), adapter->phl_role);
			}
			}
		}
	}
#endif

	return _SUCCESS;

_free_wrole:
	rtw_phl_wifi_role_free(GET_HAL_INFO(dvobj), phl_role_idx);
	adapter->phl_role = NULL;

_error:
	return rst;
}

u8 rtw_hw_iface_type_change(_adapter *adapter, u8 iface_type)
{
	void *phl = GET_HAL_INFO(adapter_to_dvobj(adapter));
	struct rtw_wifi_role_t *wrole = adapter->phl_role;
	enum role_type rtype = PHL_RTYPE_NONE;
	enum rtw_phl_status status;
	struct sta_info *sta = NULL;

	if (wrole == NULL) {
		RTW_ERR("%s - wrole = NULL\n", __func__);
		rtw_warn_on(1);
		return _FAIL;
	}

	switch (iface_type) {
	case _HW_STATE_ADHOC_:
		rtype = PHL_RTYPE_ADHOC;
		break;
	case _HW_STATE_STATION_:
		rtype = PHL_RTYPE_STATION;
		break;
	case _HW_STATE_AP_:
		rtype = PHL_RTYPE_AP;
		break;
#ifdef CONFIG_RTW_SUPPORT_MBSSID_VAP
	case _HW_STATE_VAP_:
		rtype = PHL_RTYPE_VAP;
		break;
#endif /* CONFIG_RTW_SUPPORT_MBSSID_VAP */
	case _HW_STATE_MONITOR_:
		rtype = PHL_RTYPE_MONITOR;
		break;
	case _HW_STATE_NOLINK_:
	default:
		/* TBD */
		break;
	}

	if (wrole->type == PHL_RTYPE_AP && rtype == PHL_RTYPE_STATION) {
		status = RTW_PHL_STATUS_SUCCESS;
		wrole->type = rtype;
	} else {
		status = rtw_phl_cmd_wrole_change(phl, wrole,
				WR_CHG_TYPE, (u8*)&rtype, sizeof(enum role_type),
				PHL_CMD_DIRECTLY, 0);
	}

	if (status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR("%s - change to phl role type = %d fail with error = %d\n",
			__func__, rtype, status);
		rtw_warn_on(1);
		return _FAIL;
	}

	/* AP allocates self-station and changes broadcast-station before hostapd adds key */
	if (IS_AP_ROLE_TYPE(rtype)) {
		if (_FAIL == rtw_init_self_stainfo(adapter, PHL_CMD_DIRECTLY)) {
			RTW_ERR("%s - allocate AP self-station failed\n", __func__);
			rtw_warn_on(1);
			return _FAIL;
		}
	}

	RTW_INFO("%s - change to type = %d success !\n", __func__, iface_type);

	return _SUCCESS;
}

void rtw_hw_iface_deinit(_adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	if (0 != adapter->cached_token) { // ALL_TOKEN shall equal to 0
#ifdef CONFIG_FSM
		rtw_phl_scan_del_request(GET_HAL_INFO(dvobj), adapter->cached_token);
#endif
		adapter->cached_token = 0;
	}

	if (adapter->phl_role) {
		rtw_free_self_stainfo(adapter);
		rtw_phl_wifi_role_free(GET_HAL_INFO(dvobj), adapter->phl_role->id);
		adapter->phl_role = NULL;
	}
}

/*
 * _sec_algo_drv2phl() - Convert security algorithm to PHL's definition
 * @drv_algo:		security algorithm
 * @phl_algo:		security algorithm for PHL, ref to enum rtw_enc_algo
 * @phl_key_len:	key length
 *
 * Convert driver's security algorithm defintion to PHL's type.
 *
 */
static void _sec_algo_drv2phl(enum security_type drv_algo,
			      u8 *algo, u8 *key_len)
{
	u8 phl_algo = RTW_ENC_NONE;
	u8 phl_key_len = 0;

	switch(drv_algo) {
	case _NO_PRIVACY_:
		phl_algo = RTW_ENC_NONE;
		phl_key_len = 0;
		break;
	case _WEP40_:
		phl_algo = RTW_ENC_WEP40;
		phl_key_len = 5;
		break;
	case _TKIP_:
	case _TKIP_WTMIC_:
		phl_algo = RTW_ENC_TKIP;
		phl_key_len = 16;
		break;
	case _AES_:
		phl_algo = RTW_ENC_CCMP;
		phl_key_len = 16;
		break;
	case _WEP104_:
		phl_algo = RTW_ENC_WEP104;
		phl_key_len = 13;
		break;
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	case _SMS4_:
		phl_algo = RTW_ENC_WAPI;
		phl_key_len = 32;
		break;
	case _GCM_SM4_:
		phl_algo = RTW_ENC_GCMSMS4;
		phl_key_len = 32;
		break;
#endif
	case _GCMP_:
		phl_algo = RTW_ENC_GCMP;
		phl_key_len = 16;
		break;
	case _CCMP_256_:
		phl_algo = RTW_ENC_CCMP256;
		phl_key_len = 32;
		break;
	case _GCMP_256_:
		phl_algo = RTW_ENC_GCMP256;
		phl_key_len = 32;
		break;
#ifdef CONFIG_IEEE80211W
	case _BIP_CMAC_128_:
		phl_algo = RTW_ENC_BIP_CCMP128;
		phl_key_len = 16;
		break;
#endif /* CONFIG_IEEE80211W */
	default:
		RTW_ERR("%s: No rule to covert drv algo(0x%x) to phl!!\n",
			__func__, drv_algo);
		phl_algo = RTW_ENC_MAX;
		phl_key_len = 0;
		break;
	}

	if(algo)
		*algo = phl_algo;
	if(key_len)
		*key_len = phl_key_len;
}

u8 rtw_sec_algo_drv2phl(enum security_type drv_algo)
{
	u8 algo = 0;

	_sec_algo_drv2phl(drv_algo, &algo, NULL);
	return algo;
}

static int rtw_hw_chk_sec_mode(struct _ADAPTER *a, struct sta_info *sta,
			enum phl_cmd_type cmd_type,  u32 cmd_timeout)
{
	struct dvobj_priv *d;
	void *phl;
	enum rtw_phl_status status;
#ifdef CONFIG_RTL_CFG80211_WAPI_SUPPORT
	u8 wapi = 0;
#endif
	u8 sec_mode = 0;
	struct security_priv *psecuritypriv = &a->securitypriv;

	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);

	if (!phl)
		return _FAIL;

#ifdef CONFIG_RTL_CFG80211_WAPI_SUPPORT
	if (psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_WAPI)
		wapi = 1;
#endif

	sec_mode = rtw_phl_trans_sec_mode(
		rtw_sec_algo_drv2phl(psecuritypriv->dot11PrivacyAlgrthm),
		rtw_sec_algo_drv2phl(psecuritypriv->dot118021XGrpPrivacy));

	RTW_INFO("After phl trans_sec_mode = %d\n", sec_mode);

	if ((sec_mode != sta->phl_sta->sec_mode)
#ifdef CONFIG_RTL_CFG80211_WAPI_SUPPORT
	    || (wapi != sta->phl_sta->wapi)
#endif
	   ) {
		RTW_INFO("%s: original sec_mode %d, update sec_mode to %d.\n",
			 __func__, sta->phl_sta->sec_mode, sec_mode);
#ifdef CONFIG_RTL_CFG80211_WAPI_SUPPORT
		sta->phl_sta->wapi = wapi;
#endif
		status = rtw_phl_cmd_change_stainfo(phl, sta->phl_sta, STA_CHG_SEC_MODE,
				&sec_mode, sizeof(u8), cmd_type, cmd_timeout);
		/* TODO: check the return status */
	} else {
		RTW_INFO("%s: sec_mode remains the same, skip update.\n", __func__);
	}

	return _SUCCESS;
}

void _rtw_update_key_installed(struct sta_info *sta, u8 keyid, u8 keytype, u8 add)
{
	if (keytype >= 3 || keyid >= 6) {
		RTW_ERR("Invalid keytype or keyid = %d %d\n", keytype, keyid);
		return;
	}
	if (add)
		sta->key_installed[keytype] |= BIT(keyid);
	else
		sta->key_installed[keytype] &= ~BIT(keyid);
}

u8 _rtw_check_key_installed(struct sta_info *sta, u8 keyid, u8 keytype)
{
	if (keytype >= 3 || keyid >= 6) {
		RTW_ERR("Invalid keytype or keyid = %d %d\n", keytype, keyid);
		return _TRUE;
	}

	if (sta->key_installed[keytype] & BIT(keyid))
		return _TRUE;

	return _FALSE;
}

/*
 * rtw_hw_add_key() - Add security key
 * @a:		pointer of struct _ADAPTER
 * @sta:	pointer of struct sta_info
 * @keyid:	key index
 * @keyalgo:	key algorithm
 * @keytype:	0: unicast / 1: multicast / 2: bip (ref: enum SEC_CAM_KEY_TYPE)
 * @key:	key content
 *
 * Add security key.
 *
 * Return 0 for success, otherwise fail.
 */
int rtw_hw_add_key(struct _ADAPTER *a, struct sta_info *sta,
		   u8 keyid, enum security_type keyalgo, u8 keytype, u8 *key,
		   enum phl_cmd_type cmd_type,  u32 cmd_timeout)
{
	struct dvobj_priv *d;
	void *phl;
	struct phl_sec_param_h crypt = {0};
	enum rtw_phl_status status;
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	struct phl_sec_param_h del_crypt = {0};
#endif

	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);
	if (!phl)
		return -1;

	if (rtw_hw_chk_sec_mode(a, sta, cmd_type, cmd_timeout) == _FAIL)
		return -1;

	crypt.keyid = keyid;
	crypt.key_type= keytype;
	_sec_algo_drv2phl(keyalgo, &crypt.enc_type, &crypt.key_len);

	/* delete key before adding key */
	rtw_phl_cmd_del_key(phl, sta->phl_sta, &crypt, cmd_type, cmd_timeout);
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	if (keyalgo == _SMS4_ || keyalgo == _GCM_SM4_) {
		_rtw_memcpy(&del_crypt, &crypt, sizeof(struct phl_sec_param_h));
		del_crypt.keyid = !crypt.keyid;
		RTW_INFO("\n%s crypt.keyid=%d del_crypt.keyid=%d\n", __FUNCTION__, crypt.keyid, del_crypt.keyid);
		rtw_phl_cmd_del_key(phl, sta->phl_sta, &del_crypt, cmd_type, cmd_timeout);
		_rtw_update_key_installed(sta, !keyid, keytype, 0);
	}
#endif
	_rtw_update_key_installed(sta, keyid, keytype, 1);
	status = rtw_phl_cmd_add_key(phl, sta->phl_sta, &crypt, key, cmd_type, cmd_timeout);
	if (status != RTW_PHL_STATUS_SUCCESS)
		return -1;

	return 0;
}

/*
 * rtw_hw_del_key() - Delete security key
 * @a:		pointer of struct _ADAPTER
 * @sta:	pointer of struct sta_info
 * @keyid:	key index
 * @keytype:	0: unicast / 1: multicast / 2: bip (ref: enum SEC_CAM_KEY_TYPE)
 *
 * Delete security key by macid, keyid and keytype.
 *
 * Return 0 for success, otherwise fail.
 */
int rtw_hw_del_key(struct _ADAPTER *a, struct sta_info *sta,
		   u8 keyid, u8 keytype, enum phl_cmd_type cmd_type, u32 cmd_timeout)
{
	struct dvobj_priv *d;
	void *phl;
	struct phl_sec_param_h crypt = {0};
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;


	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);
	if (!phl)
		return -1;

	if (_rtw_check_key_installed(sta, keyid, keytype) == _FALSE)
		return -1;

	crypt.keyid = keyid;
	crypt.key_type= keytype;
	status = rtw_phl_cmd_del_key(phl, sta->phl_sta, &crypt, cmd_type, cmd_timeout);
	if (status != RTW_PHL_STATUS_SUCCESS)
		return -1;

	_rtw_update_key_installed(sta, keyid, keytype, 0);
	return 0;
}

/*
 * rtw_hw_del_all_key() - Delete all security key for this STA
 * @a:		pointer of struct _ADAPTER
 * @sta:	pointer of struct sta_info
 *
 * Delete all security keys belong to this STA.
 *
 * Return 0 for success, otherwise fail.
 */
int rtw_hw_del_all_key(struct _ADAPTER *a, struct sta_info *sta,
						enum phl_cmd_type cmd_type, u32 cmd_timeout)
{
	struct dvobj_priv *d;
	void *phl;
	u8 keyid;
	u8 keytype;
	struct phl_sec_param_h crypt = {0};
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;


	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);
	if (!phl)
		return -1;

	/* Delete Group and Pairwise key */
	for (keytype = 0; keytype < 2; keytype++) {
		for (keyid = 0; keyid < 4; keyid++) {
			crypt.keyid = keyid;
			crypt.key_type = keytype;
			if (_rtw_check_key_installed(sta, keyid, keytype) == _FALSE)
				continue;
			rtw_phl_cmd_del_key(phl, sta->phl_sta, &crypt, cmd_type, cmd_timeout);
			_rtw_update_key_installed(sta, keyid, keytype, 0);
		}
	}

	/* Delete BIP key */
	crypt.key_type = 2;
	for (keyid = 4; keyid <= BIP_MAX_KEYID; keyid++) {
		crypt.keyid = keyid;
		if (_rtw_check_key_installed(sta, keyid, crypt.key_type) == _FALSE)
			continue;
		rtw_phl_cmd_del_key(phl, sta->phl_sta, &crypt, cmd_type, cmd_timeout);
		_rtw_update_key_installed(sta, keyid, crypt.key_type, 0);
	}

	return 0;
}

int rtw_hw_start_bss_network(struct _ADAPTER *a)
{
	/* some hw related ap settings */

	return _SUCCESS;
}

/* connect */
int rtw_hw_prepare_connect(struct _ADAPTER *a, struct sta_info *sta, u8 *target_addr)
{
	/*adapter->phl_role.mac_addr*/
	struct dvobj_priv *d;
	void *phl;
	enum rtw_phl_status status;


	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);

	#if (PHL_VER_CODE >= PHL_VERSION(0001, 0017, 0000, 0000))
	status = rtw_phl_connect_prepare(phl, 0, a->phl_role, target_addr);
	#else
	status = rtw_phl_connect_prepare(phl, a->phl_role, target_addr);
	#endif /* PHL_VER_CODE */
	if (status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR("%s: Fail to setup hardware for connecting!(%d)\n",
			__func__, status);
		return -1;
	}
	/* Todo: Enable TSF update */
	/* Todo: Set support short preamble or not by beacon capability */
	/* Todo: Set slot time */

	return 0;
}

/* Handle connect fail case */
int rtw_hw_connect_abort(struct _ADAPTER *a, struct sta_info *sta)
{
	struct dvobj_priv *d;
	void *phl;
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;


	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);
	if (!phl)
		return -1;

	/* Todo: Change hardware setting to Disconnect */
#if 0
	join_type = 1;
	rtw_hal_set_hwreg(padapter, HW_VAR_MLME_JOIN, (u8 *)(&join_type));
#endif
	rtw_hw_del_all_key(a, sta, PHL_CMD_DIRECTLY, 0);
	status = rtw_phl_cmd_update_media_status(phl, sta->phl_sta, NULL, false,
							PHL_CMD_DIRECTLY, 0);
	if (status != RTW_PHL_STATUS_SUCCESS)
		return -1;

	/* Todo: clear BSSID */
#if 0
	rtw_hal_set_hwreg(padapter, HW_VAR_BSSID, null_addr);
#endif
	/* Todo: Set RCR setting to disconnect */
#if 0
	if (MLME_IS_STATE(pmlmeinfo, WIFI_FW_STATION_STATE))
		rtw_hal_rcr_set_chk_bssid(padapter, MLME_STA_DISCONNECTED);
#endif
	/* Todo: btcoex disconnect event notify */
#if 0
	rtw_btcoex_connect_notify(padapter, join_type);
#endif

	rtw_join_done_chk_ch(a, -1);

	/* free connecting AP sta info */
	 rtw_free_stainfo(a, sta);
	 rtw_init_self_stainfo(a, PHL_CMD_DIRECTLY);

	return 0;
}

#ifdef CONFIG_80211N_HT
#ifdef CONFIG_80211AC_VHT
static void update_phl_sta_cap_vht(struct _ADAPTER *a, struct sta_info *sta,
			           struct protocol_cap_t *cap)
{
	struct vht_priv *vht;


	vht = &sta->vhtpriv;

	if (cap->ampdu_len_exp < vht->ampdu_len)
		cap->ampdu_len_exp = vht->ampdu_len;
	if (cap->max_amsdu_len < vht->max_mpdu_len)
		cap->max_amsdu_len = vht->max_mpdu_len;
	else {
		RTW_PRINT("[WARNING] HT_cap amsdu:0x%x > VHT_cap amsdu:0x%x, sta something wrong.\n",
			cap->max_amsdu_len, vht->max_mpdu_len);
		/* workaround, because amsdu_len of ht_cap is larger than vht_cap,
			it seems a wrong setting from station, so we set amsdu_len from vht_cap */
		cap->max_amsdu_len = vht->max_mpdu_len;
	}

	cap->sgi_80 = (vht->sgi_80m == _TRUE) ? 1 : 0;

	cap->vht_ldpc = vht->ldpc_cap ? 1 : 0;



	_rtw_memcpy(cap->vht_rx_mcs, vht->vht_mcs_map, 2);
	/* Todo: cap->vht_tx_mcs[2]; */
	if (vht->op_present)
		_rtw_memcpy(cap->vht_basic_mcs, &vht->vht_op[3], 2);
}
#endif /* CONFIG_80211AC_VHT */
static void update_phl_sta_cap_ht(struct _ADAPTER *a, struct sta_info *sta,
			          struct protocol_cap_t *cap)
{
	struct mlme_ext_info *info;
	struct ht_priv *ht;


	info = &a->mlmeextpriv.mlmext_info;
	ht = &sta->htpriv;

	cap->num_ampdu = 64;	/* Set to MAX */

	cap->ampdu_density = ht->rx_ampdu_min_spacing;
	cap->ampdu_len_exp = GET_HT_CAP_ELE_MAX_AMPDU_LEN_EXP(&ht->ht_cap);
	cap->amsdu_in_ampdu = 1;
	cap->max_amsdu_len = GET_HT_CAP_ELE_MAX_AMSDU_LENGTH(&ht->ht_cap);

	/*GET_HT_CAP_ELE_SM_PS(&info->HT_caps.u.HT_cap_element.HT_caps_info);*/
	//cap->sm_ps = info->SM_PS;

	cap->sgi_20 = (ht->sgi_20m == _TRUE) ? 1 : 0;
	cap->sgi_40 = (ht->sgi_40m == _TRUE) ? 1 : 0;

	cap->ht_ldpc = ht->ldpc_cap ? 1 : 0;

	_rtw_memcpy(cap->ht_rx_mcs, ht->ht_cap.supp_mcs_set, 4);
	/* Todo: cap->ht_tx_mcs[4]; */
	if (info->HT_info_enable)
		_rtw_memcpy(cap->ht_basic_mcs, info->HT_info.MCS_rate, 4);
}
#endif /* CONFIG_80211N_HT */

static void update_phl_sta_cap(struct _ADAPTER *a, struct sta_info *sta,
			       struct protocol_cap_t *cap)
{
	struct mlme_ext_info *info;


	info = &a->mlmeextpriv.mlmext_info;

	/* MAC related */
	/* update beacon interval */
	cap->bcn_interval = info->bcn_interval;
#if 0
	cap->num_ampdu;		/* HT, VHT, HE */
	cap->ampdu_density:3;	/* HT, VHT, HE */
	cap->ampdu_len_exp;	/* HT, VHT, HE */
	cap->amsdu_in_ampdu:1;	/* HT, VHT, HE */
	cap->max_amsdu_len:2;	/* HT, VHT, HE */
	cap->htc_rx:1;
	cap->sm_ps:2;		/* HT */
	cap->trig_padding:2;
	cap->twt:6;
	cap->all_ack:1;
	cap->a_ctrl:3;
	cap->ops:1;
	cap->ht_vht_trig_rx:1;
#endif
	cap->short_slot = (info->slotTime == SHORT_SLOT_TIME) ? 1 : 0;
	cap->preamble = (info->preamble_mode == PREAMBLE_SHORT) ? 1 : 0;
#if 0
	cap->sgi_20:1;		/* HT */
	cap->sgi_40:1;		/* HT */
	cap->sgi_80:1;		/* VHT */
	cap->sgi_160:1		/* VHT, HE */

	/* BB related */
	cap->ht_ldpc:1;		/* HT */
	cap->vht_ldpc:1;	/* VHT */
	cap->he_ldpc:1;		/* HE */
	cap->sgi:1;
	cap->su_bfmr:1;
	cap->su_bfme:1;
	cap->mu_bfmr:1;
	cap->mu_bfme:1;
	cap->bfme_sts:3;
	cap->num_snd_dim:3;
#endif
	_rtw_memset(cap->supported_rates, 0, 12);
	_rtw_memcpy(cap->supported_rates, sta->bssrateset,
		    sta->bssratelen < 12 ? sta->bssratelen : 12);
#if 0
	cap->ht_rx_mcs[4];	/* HT */
	cap->ht_tx_mcs[4];	/* HT */
	cap->ht_basic_mcs[4];	/* Basic rate of HT */
	cap->vht_rx_mcs[2];	/* VHT */
	cap->vht_tx_mcs[2];	/* VHT */
	cap->vht_basic_mcs[2];	/* Basic rate of VHT */
#endif
#if 0
	/* HE done */
	cap->he_rx_mcs[2];
	cap->he_tx_mcs[2];
	cap->he_basic_mcs[2];	/* Basic rate of HE */
	cap->stbc_tx:1;
	cap->stbc_rx:3;
	cap->ltf_gi;
	cap->doppler_tx:1;
	cap->doppler_rx:1;
	cap->dcm_max_const_tx:2;
	cap->dcm_max_nss_tx:1;
	cap->dcm_max_const_rx:2;
	cap->dcm_max_nss_rx:1;
	cap->partial_bw_su_in_mu:1;
	cap->bfme_sts_greater_80mhz:3;
	cap->num_snd_dim_greater_80mhz:3;
	cap->stbc_tx_greater_80mhz:1;
	cap->stbc_rx_greater_80mhz:1;
	cap->ng_16_su_fb:1;
	cap->ng_16_mu_fb:1;
	cap->cb_sz_su_fb:1;
	cap->cb_sz_mu_fb:1;
	cap->trig_su_bfm_fb:1;
	cap->trig_mu_bfm_fb:1;
	cap->trig_cqi_fb:1;
	cap->partial_bw_su_er:1;
	cap->pkt_padding:2;
	cap->ppe_th[24];
	cap->pwr_bst_factor:1;
	cap->max_nc:3;
	cap->dcm_max_ru:2;
	cap->long_sigb_symbol:1;
	cap->non_trig_cqi_fb:1;
	cap->tx_1024q_ru:1;
	cap->rx_1024q_ru:1;
	cap->fbw_su_using_mu_cmprs_sigb:1;
	cap->fbw_su_using_mu_non_cmprs_sigb:1;
	cap->er_su:1;
	cap->tb_pe:3;
	cap->txop_du_rts_th;
#endif

#ifdef CONFIG_80211N_HT
	if (sta->htpriv.ht_option) {
		update_phl_sta_cap_ht(a, sta, cap);
#ifdef CONFIG_80211AC_VHT
		if (sta->vhtpriv.vht_option)
			update_phl_sta_cap_vht(a, sta, cap);
#endif /* CONFIG_80211AC_VHT */
#ifdef CONFIG_80211AX_HE
		if (sta->hepriv.he_option) {
			/* Set A-MPDU number to role's cap as it is resolved
			   by ADDBA req/resp handshaking */
			cap->num_ampdu = a->phl_role->proto_role_cap.num_ampdu;
			RTW_INFO("[%s %d] Updated %pM's A-MPDU number to %u\n", __func__, __LINE__, sta->phl_sta->mac_addr, cap->num_ampdu);
		}
#endif
	}
#endif /* CONFIG_80211N_HT */
}

/**
 * rtw_hw_set_edca() - setup WMM EDCA parameter
 * @a:		struct _ADAPTER *
 * @ac:		Access Category, 0:BE, 1:BK, 2:VI, 3:VO
 * @param:	AIFS:BIT[7:0], CWMIN:BIT[11:8], CWMAX:BIT[15:12],
 *		TXOP:BIT[31:16]
 *
 * Setup WMM EDCA parameter set.
 *
 * Return 0 for SUCCESS, otherwise fail.
 */
int rtw_hw_set_edca(struct _ADAPTER *a, u8 ac, u32 param)
{
	struct dvobj_priv *d;
	void *phl;
	struct rtw_edca_param edca = {0};
	enum rtw_phl_status status;

	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);
	if ( !phl || !a->phl_role ){
		RTW_WARN("%s-%s, phl or phl_role null!\n",__func__, a->pnetdev->name);
		return -1;
	}

	edca.ac = ac;
	edca.param = param;

	status = rtw_phl_cmd_wrole_change(phl, a->phl_role,
				WR_CHG_EDCA_PARAM, (u8*)&edca, sizeof(struct rtw_edca_param),
				PHL_CMD_DIRECTLY, 0);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR("%s: fail to set edca parameter, ac(%u), "
			"param(0x%08x)\n",
			__func__, ac, param);
		return -1;
	}

	return 0;
}

int rtw_hw_connected(struct _ADAPTER *a, struct sta_info *sta)
{

	struct dvobj_priv *d;
	void *phl;
	enum rtw_phl_status status;
	struct security_priv *psecuritypriv = &a->securitypriv;

	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);
	if (!phl)
		return -1;

	update_phl_sta_cap(a, sta, &sta->phl_sta->asoc_cap);
	status = rtw_phl_cmd_update_media_status(phl, sta->phl_sta, sta->phl_sta->mac_addr, true,
							PHL_CMD_DIRECTLY, 0);
	if (status != RTW_PHL_STATUS_SUCCESS)
		return -1;
#ifdef RTW_WKARD_REDUCE_CONNECT_LOG
	if(!a->vw_enable)
#endif
		rtw_dump_phl_sta_info(RTW_DBGDUMP, sta);
	/* Todo: update IOT-releated issue */
#if 0
	update_IOT_info(a);
#endif
	/* Todo: RTS full bandwidth setting */
#if 0
#ifdef CONFIG_RTS_FULL_BW
	rtw_set_rts_bw(a);
#endif /* CONFIG_RTS_FULL_BW */
#endif
	/* Todo: Basic rate setting */
#if 0
	rtw_hal_set_hwreg(a, HW_VAR_BASIC_RATE, cur_network->SupportedRates);
#endif
	/* Todo: udpate capability: short preamble, slot time */
	update_capinfo(a, a->mlmeextpriv.mlmext_info.capability);

	WMMOnAssocRsp(a);

	/* Todo: HT: AMPDU factor, min space, max time and related parameters */
#if 0
#ifdef CONFIG_80211N_HT
	HTOnAssocRsp(a);
#endif /* CONFIG_80211N_HT */
#endif
	/* Todo: VHT */
#if 0
#ifdef CONFIG_80211AC_VHT
	VHTOnAssocRsp(a);
#endif
#endif
	/* Todo: Set Data rate and RA */
#if 0
	set_sta_rate(a, psta);
#endif
	/* Todo: Firmware media status report */
#if 0
	rtw_sta_media_status_rpt(a, psta, 1);
#endif
	/* Todo: IC specific hardware setting */
#if 0
	join_type = 2;
	rtw_hal_set_hwreg(a, HW_VAR_MLME_JOIN, (u8 *)(&join_type));
#endif
	if (MLME_IS_STATE(&a->mlmeextpriv.mlmext_info, WIFI_FW_STATION_STATE)) {
		/* Todo: Correct TSF */
#if 0
		correct_TSF(a, MLME_STA_CONNECTED);
#endif
	}

	/* Todo: btcoex connect event notify */
#if 0
	rtw_btcoex_connect_notify(a, join_type);
#endif
	/* Todo: Beamforming setting */
#if 0
	beamforming_wk_cmd(a, BEAMFORMING_CTRL_ENTER, (u8 *)psta, sizeof(struct sta_info), 0);
#endif
	#if (PHL_VER_CODE >= PHL_VERSION(0001, 0017, 0000, 0000))
	rtw_phl_connected(phl, 0, a->phl_role, sta->phl_sta);
	#else
	rtw_phl_connected(phl, a->phl_role, sta->phl_sta);
	#endif /* PHL_VER_CODE */

	rtw_join_done_chk_ch(a, 1);

	return 0;
}

int rtw_hw_disconnect(struct _ADAPTER *a, struct sta_info *sta)
{
	struct dvobj_priv *d;
	void *phl;
	enum rtw_phl_status status;
	int tid;
	struct mlme_priv *pmlmepriv = &(a->mlmepriv);
	u8 is_ap_self = _FALSE;

	if ( !sta )
		return -1;

	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);
	if (!phl)
		return -1;

	if (MLME_IS_AP(a) &&
		_rtw_memcmp(a->phl_role->mac_addr, sta->phl_sta->mac_addr, ETH_ALEN))
		is_ap_self = _TRUE;

	/* Check and reset setting related to rx ampdu resources of PHL. */
	for (tid = 0; tid < TID_NUM; tid++) {
		if(sta->recvreorder_ctrl[tid].enable == _TRUE) {
			sta->recvreorder_ctrl[tid].enable =_FALSE;
			rtw_phl_stop_rx_ba_session(phl, sta->phl_sta, tid);
			RTW_INFO(FUNC_ADPT_FMT"stop process tid %d \n",
				FUNC_ADPT_ARG(a), tid);
		}
	}

	/*reset sec setting and clean all connection setting*/
	rtw_hw_del_all_key(a, sta, PHL_CMD_DIRECTLY, 0);

	if (!is_ap_self) {
		status = rtw_phl_cmd_update_media_status(phl, sta->phl_sta, NULL, false,
						PHL_CMD_DIRECTLY, 0);
		if (status != RTW_PHL_STATUS_SUCCESS)
			return -1;
	}
#ifdef RTW_WKARD_REDUCE_CONNECT_LOG
	if(!a->vw_enable)
#endif
		rtw_dump_phl_sta_info(RTW_DBGDUMP, sta);

	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	if (a->registrypriv.wifi_mib.ofdma_enable & dl_fix_mode_by_assoc) {
		rtw_establish_dl_grp(a);
	}

	if (a->registrypriv.wifi_mib.ofdma_enable & ul_fix_mode_by_assoc) {
		rtw_establish_ul_grp(a);
	}
	#endif

	return 0;
}

int rtw_hw_connected_apmode(struct _ADAPTER *a, struct sta_info *sta)
{
	struct dvobj_priv *d;
	void *phl;
	struct mlme_priv *pmlmepriv = &(a->mlmepriv);

	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);
	if (!phl)
		return -1;

	rtw_ap_set_sta_wmode(a, sta);

	update_phl_sta_cap(a, sta, &sta->phl_sta->asoc_cap);

	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// b0ca025955436b53632f25904b0b759b4fce1d1a john_lei
	rtw_phl_free_rx_reorder(phl, sta->phl_sta);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */

	if (RTW_PHL_STATUS_SUCCESS != rtw_phl_cmd_update_media_status(
		phl, sta->phl_sta, sta->phl_sta->mac_addr, true,
		PHL_CMD_DIRECTLY, 0))
		return -1;

	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	if (a->registrypriv.wifi_mib.ofdma_enable & dl_fix_mode_by_assoc) {
		rtw_establish_dl_grp(a);
	}

	if (a->registrypriv.wifi_mib.ofdma_enable & ul_fix_mode_by_assoc) {
		rtw_establish_ul_grp(a);
	}
	#endif


#ifdef RTW_WKARD_REDUCE_CONNECT_LOG
	if(!a->vw_enable)
#endif
		rtw_dump_phl_sta_info(RTW_DBGDUMP, sta);

	return 0;
}

u8 rtw_hal_get_def_var(struct _ADAPTER *a,
		       enum _HAL_DEF_VARIABLE def_var, void *val)
{
	switch (def_var) {
	case HAL_DEF_IS_SUPPORT_ANT_DIV:
		*(u8*)val = _FALSE;
		break;
	case HAL_DEF_MAX_RECVBUF_SZ:
#ifdef CONFIG_PHL_015_devp_tmp
		*(u32*)val = GET_HAL_RECVBUF_SZ(adapter_to_dvobj(a));
#else
*(u32*)val = 0;//GET_HAL_RECVBUF_SZ(adapter_to_dvobj(a));
#endif
		break;
	case HAL_DEF_RX_PACKET_OFFSET:
		*(u32*)val = 0;
		break;
	case HAL_DEF_DBG_DUMP_RXPKT:
		*(u8*)val = 0;
		break;
	case HAL_DEF_BEAMFORMER_CAP:
		*(u8*)val = a->phl_role->proto_role_cap.num_snd_dim;
		break;
	case HAL_DEF_BEAMFORMEE_CAP:
		*(u8*)val = a->phl_role->proto_role_cap.bfme_sts;
		break;
	case HW_VAR_MAX_RX_AMPDU_FACTOR:
		/* HT only */
		*(enum _HT_CAP_AMPDU_FACTOR*)val = MAX_AMPDU_FACTOR_64K;
		break;
	case HW_VAR_MAX_RX_AMPDU_NUM:
		if (a->phl_role->proto_role_cap.num_ampdu >= 256)
			*(u8*)val = a->phl_role->proto_role_cap.num_ampdu - 1;
		else
			*(u8*)val = a->phl_role->proto_role_cap.num_ampdu;
		break;
	case HW_DEF_RA_INFO_DUMP:
		/* do nothing */
		break;
	case HAL_DEF_DBG_DUMP_TXPKT:
		*(u8*)val = 0;
		break;
	case HAL_DEF_TX_PAGE_SIZE:
		/* would be removed later */
		break;
	case HW_VAR_BEST_AMPDU_DENSITY:
		*(u8*)val = 0;
		break;
	default:
		break;
	}

	return 0;
}

#ifdef CONFIG_RTW_ACS
enum band_type rtw_acs_get_band_by_idx(struct _ADAPTER *a, u8 acs_idx)
{
	void *phl = GET_HAL_INFO(adapter_to_dvobj(a));
	struct rtw_acs_info_parm parm = {0};
	enum band_type band = 0;

	parm.idx = acs_idx;
	parm.type = RTW_ACS_INFO_BAND;
	parm.info = &band;

	if (rtw_phl_get_acs_info(phl, &parm) == RTW_PHL_STATUS_SUCCESS)
		return band;
	else
		return BAND_MAX;
}

u8 rtw_acs_get_channel_by_idx(struct _ADAPTER *a, u8 acs_idx)
{
	void *phl = GET_HAL_INFO(adapter_to_dvobj(a));
	struct rtw_acs_info_parm parm = {0};
	u8 channel = 0;

	parm.idx = acs_idx;
	parm.type = RTW_ACS_INFO_CHANNEL;
	parm.info = &channel;

	if (rtw_phl_get_acs_info(phl, &parm) == RTW_PHL_STATUS_SUCCESS)
		return channel;
	else
		return 0;
}

u8 rtw_acs_get_clm_ratio_by_idx(struct _ADAPTER *a, u8 acs_idx)
{
	void *phl = GET_HAL_INFO(adapter_to_dvobj(a));
	struct rtw_acs_info_parm parm = {0};
	u8 clm_ratio = 0;

	parm.idx = acs_idx;
	parm.type = RTW_ACS_INFO_CLM_RATIO;
	parm.info = &clm_ratio;

	if (rtw_phl_get_acs_info(phl, &parm) == RTW_PHL_STATUS_SUCCESS)
		return clm_ratio;
	else
		return 0;
}

s8 rtw_acs_get_noise_by_idx(struct _ADAPTER *a, u8 acs_idx)
{
	void *phl = GET_HAL_INFO(adapter_to_dvobj(a));
	struct rtw_acs_info_parm parm = {0};
	u8 nhm_pwr = 0;

	parm.idx = acs_idx;
	parm.type = RTW_ACS_INFO_NHM_PWR;
	parm.info = &nhm_pwr;

	if (rtw_phl_get_acs_info(phl, &parm) == RTW_PHL_STATUS_SUCCESS)
		return nhm_pwr - 110;
	else
		return -110;
}

void rtw_acs_get_nhm_rpt_by_idx(struct _ADAPTER *a, u8 acs_idx, u8 *nhm_rpt)
{
	void *phl = GET_HAL_INFO(adapter_to_dvobj(a));
	struct rtw_acs_info_parm parm = {0};
	enum rtw_phl_status ret = RTW_PHL_STATUS_SUCCESS;

	parm.idx = acs_idx;
	parm.type = RTW_ACS_INFO_NHM_RPT;
	parm.info = (void *)nhm_rpt;

	ret = rtw_phl_get_acs_info(phl, &parm);
}

u8 rtw_get_acs_chnl_tbl_idx(struct _ADAPTER *a, enum band_type band, u8 channel)
{
	void *phl = GET_HAL_INFO(adapter_to_dvobj(a));

	return rtw_phl_get_acs_chnl_tbl_idx(phl, band, channel);
}
#endif /* CONFIG_RTW_ACS */

void rtw_get_env_rpt(struct _ADAPTER *a)
{
	struct rtw_wifi_role_t *wrole;
	struct dvobj_priv *d;
	void *phl;

	wrole = a->phl_role;
	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);

	rtw_phl_get_env_rpt(phl, &(d->env_rpt), wrole);
}

void rtw_bcn_drop_switch(struct _ADAPTER *a, u8 enable)
{
	struct dvobj_priv *d;
	void *phl;
	struct rtw_wifi_role_t *wrole;
	struct rtw_ap_param param;
	enum rtw_phl_status status;

	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);
	wrole = a->phl_role;

	if (!wrole)
		return;
	param.cfg_id = CFG_BCN_DRP_ALL;
	param.value = enable;
	status = rtw_phl_cmd_wrole_change(phl, a->phl_role,
				WR_CHG_AP_PARAM, (u8*)&param, sizeof(struct rtw_ap_param),
				PHL_CMD_NO_WAIT, 0);

	if (enable == 0)
		send_beacon(a);
	return;
}

#if defined(DEBUG_MAP_UNASSOC) || defined(MONITOR_UNASSOC_STA) || defined(CONFIG_WIFI_DIAGNOSIS)
void rtw_hw_set_rx_mode(struct _ADAPTER *a, u8 rx_mode)
{
	struct dvobj_priv *d;
	void *phl;
	struct rtw_wifi_role_t *wrole;

	d = adapter_to_dvobj(a);
	phl = GET_HAL_INFO(d);
	wrole = a->phl_role;

	if(!phl || !wrole)
		return;
#if 0
	DBGP("skip \n");
#else
	if (rx_mode == PHL_RX_MODE_SNIFFER)
		rtw_phl_enter_mon_mode(phl, wrole);
	else
		rtw_phl_leave_mon_mode(phl, wrole);
#endif
}
#endif


#ifdef CONFIG_DFS_MASTER
bool rtw_dfs_hal_radar_detect_disable(_adapter *padapter)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	int ret = true;
	enum phl_cmd_type cmd_type = PHL_CMD_DIRECTLY;
	u32 cmd_timeout = 0;

	status = rtw_phl_cmd_dfs_rd_disable(padapter->dvobj->phl,
						HW_BAND_0,
						cmd_type,
						cmd_timeout);

	if (status != RTW_PHL_STATUS_SUCCESS) {
		ret = -1;
		RTW_ERR("%s\n",__func__);
	}

	return ret;
}

/* called after ch, bw is set */
bool rtw_dfs_hal_radar_detect_enable(_adapter *padapter)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	int ret = true;
	enum phl_cmd_type cmd_type = PHL_CMD_DIRECTLY;
	u32 cmd_timeout = 0;

	status = rtw_phl_cmd_dfs_rd_enable_all_range(padapter->dvobj->phl,
							HW_BAND_0,
							cmd_type,
							cmd_timeout);

	if (status != RTW_PHL_STATUS_SUCCESS) {
		ret = -1;
		RTW_ERR("%s\n",__func__);
	}

	return ret;

}

BOOLEAN rtw_dfs_hal_radar_detect(_adapter *padapter)
{
	bool ret = _FALSE;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	u8 is_rd_detect = false;

	if (phl_com->dfs_info.is_radar_detectd) {
		is_rd_detect = true;
	} else {
		is_rd_detect = false;
	}

#if defined (CONFIG_DFS_CHAN_SEL_G_RANDOM) || defined (CONFIG_DFS_CHAN_SEL_R_SHORTEST_WAIT)
	if(is_rd_detect)
		ret = _TRUE;
	else
		ret = _FALSE;
#endif

	if(padapter->registrypriv.wifi_mib.disable_dfs)
		ret = _FALSE;

	phl_com->dfs_info.is_radar_detectd = false;

	return ret;

}

u8 rtw_dfs_hal_radar_detect_polling_int_ms(struct dvobj_priv *dvobj)
{
	return 100;
}

BOOLEAN rtw_dfs_csa_hw_tx_pause(_adapter *padapter, bool enable)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	int err = 0;
	enum phl_cmd_type cmd_type = PHL_CMD_DIRECTLY;
	u32 cmd_timeout = 0;
	bool csa = true;

	status = rtw_phl_cmd_dfs_csa_tx_pause(padapter->dvobj->phl,
						HW_BAND_0,
						enable,
						csa,
						cmd_type,
						cmd_timeout);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		err = -1;
		RTW_ERR("%s: set enable(%d) csa(%d) FAIL!\n",
			__func__, enable, csa);
	}

	return err;

}

u8 rtw_dfs_get_region_domain(struct dvobj_priv *dvobj)
{
	struct registry_priv *registry = dvobj_to_regsty(dvobj);

	return registry->dfs_region_domain;
}
#endif /* CONFIG_DFS_MASTER */

#ifdef CONFIG_RTW_SW_LED
void rtw_led_set_ctrl_mode(_adapter *a, enum rtw_led_ctrl_mode ctrl_mode)
{
	struct led_priv *ledpriv = adapter_to_led(a);
	void *phl = GET_HAL_INFO(adapter_to_dvobj(a));

	rtw_phl_led_set_ctrl_mode(phl, ledpriv->led_id, ctrl_mode);
}

void rtw_led_register_event(_adapter *a, enum rtw_led_event event, enum rtw_led_state state,
		struct rtw_led_action_args_t *action_args, u32 interval)
{
	void *phl = GET_HAL_INFO(adapter_to_dvobj(a));

	rtw_phl_led_set_action(phl, event, state, action_args, 1, interval);
}

void _rtw_led_control(_adapter *a, enum rtw_led_event event)
{
	void *phl = GET_HAL_INFO(adapter_to_dvobj(a));
	struct led_priv *ledpriv = adapter_to_led(a);

	if (a->netif_up == _FALSE && ledpriv->manual_ctrl == _FALSE) {
		rtw_phl_led_manual_mode_switch(phl, ledpriv->led_id, _TRUE);
		rtw_phl_led_manual_control(phl, ledpriv->led_id, RTW_LED_OPT_HIGH);
		rtw_phl_led_manual_mode_switch(phl, ledpriv->led_id, _FALSE);
	} else if (ledpriv->manual_ctrl) {
		rtw_phl_led_manual_mode_switch(phl, ledpriv->led_id, _TRUE);
		rtw_phl_led_manual_control(phl, ledpriv->led_id, ledpriv->manual_opt);
	} else {
		rtw_phl_led_manual_mode_switch(phl, ledpriv->led_id, _FALSE);
		rtw_phl_led_control(phl, event);
	}
}

void rtw_led_set_toggle_intervals(_adapter *a, u8 idx, u32 *arr, u8 len)
{
	void *phl = GET_HAL_INFO(adapter_to_dvobj(a));

	rtw_phl_led_set_toggle_intervals(phl, idx, arr, len);
}
#endif /* CONFIG_RTW_SW_LED */

