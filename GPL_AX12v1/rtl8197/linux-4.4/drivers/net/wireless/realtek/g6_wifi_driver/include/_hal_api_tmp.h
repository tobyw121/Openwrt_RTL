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
#ifndef _HAL_API_TMP_H_
#define _HAL_API_TMP_H_


/**** may be get from hal_com **********************************/

static inline s32 rtw_hal_is_disable_sw_channel_plan(_adapter *padapter)
{
	return _FALSE;
	//return GET_HAL_DATA(padapter)->bDisableSWChannelPlan;
}
#define H2C_MSR_ROLE_RSVD	0
#define H2C_MSR_ROLE_STA	1
#define H2C_MSR_ROLE_AP	2
#define H2C_MSR_ROLE_GC	3
#define H2C_MSR_ROLE_GO	4
#define H2C_MSR_ROLE_TDLS	5
#define H2C_MSR_ROLE_ADHOC	6
#define H2C_MSR_ROLE_MESH	7
#define H2C_MSR_ROLE_MAX	8

/*************************************************************************************/
typedef enum _HW_VARIABLES {
	HW_VAR_NET_TYPE,
	HW_VAR_SET_OPMODE,
	HW_VAR_MAC_ADDR,
	HW_VAR_BSSID,
	HW_VAR_BASIC_RATE,
	HW_VAR_TXPAUSE,
	HW_VAR_BCN_FUNC,
	HW_VAR_CORRECT_TSF,
	HW_VAR_RCR,
	HW_VAR_MLME_DISCONNECT,
	HW_VAR_MLME_SITESURVEY,
	HW_VAR_MLME_JOIN,
	HW_VAR_ON_RCR_AM,
	HW_VAR_OFF_RCR_AM,
	HW_VAR_BEACON_INTERVAL,
	HW_VAR_SLOT_TIME,
	HW_VAR_RESP_SIFS,
	HW_VAR_ACK_PREAMBLE,
	HW_VAR_SEC_CFG,
	HW_VAR_SEC_DK_CFG,
	HW_VAR_BCN_VALID,
	HW_VAR_FREECNT,
	HW_VAR_STOP_BCN,
	HW_VAR_RESUME_BCN,

	/* PHYDM odm->SupportAbility */
	HW_VAR_CAM_EMPTY_ENTRY,
	HW_VAR_CAM_INVALID_ALL,
	HW_VAR_AC_PARAM_VO,
	HW_VAR_AC_PARAM_VI,
	HW_VAR_AC_PARAM_BE,
	HW_VAR_AC_PARAM_BK,
	HW_VAR_ACM_CTRL,
#ifdef CONFIG_WMMPS_STA
	HW_VAR_UAPSD_TID,
#endif /* CONFIG_WMMPS_STA */
	HW_VAR_AMPDU_MIN_SPACE,
#ifdef CONFIG_80211N_HT
	HW_VAR_AMPDU_FACTOR,
#endif /* CONFIG_80211N_HT */
	HW_VAR_RXDMA_AGG_PG_TH,
	HW_VAR_SET_RPWM,
	HW_VAR_CPWM,
	HW_VAR_H2C_FW_PWRMODE,
	HW_VAR_H2C_INACTIVE_IPS,
	HW_VAR_H2C_FW_JOINBSSRPT,
	HW_VAR_FWLPS_RF_ON,
	HW_VAR_H2C_FW_P2P_PS_OFFLOAD,
#ifdef CONFIG_LPS_PG
	HW_VAR_LPS_PG_HANDLE,
#endif
	HW_VAR_TRIGGER_GPIO_0,
	HW_VAR_BT_SET_COEXIST,
	HW_VAR_BT_ISSUE_DELBA,
	HW_VAR_FIFO_CLEARN_UP,
	HW_VAR_RESTORE_HW_SEQ,
	HW_VAR_CHECK_TXBUF,
	HW_VAR_PCIE_STOP_TX_DMA,
	HW_VAR_APFM_ON_MAC, /* Auto FSM to Turn On, include clock, isolation, power control for MAC only */
	/* The valid upper nav range for the HW updating, if the true value is larger than the upper range, the HW won't update it. */
	/* Unit in microsecond. 0 means disable this function. */
#if defined(CONFIG_WOWLAN) || defined(CONFIG_AP_WOWLAN)
	HW_VAR_WOWLAN,
	HW_VAR_WAKEUP_REASON,
#endif
	HW_VAR_RPWM_TOG,
#ifdef CONFIG_GPIO_WAKEUP
	HW_VAR_WOW_OUTPUT_GPIO,
	HW_VAR_WOW_INPUT_GPIO,
	HW_SET_GPIO_WL_CTRL,
#endif
	HW_VAR_SYS_CLKR,
	HW_VAR_NAV_UPPER,
	HW_VAR_CHK_HI_QUEUE_EMPTY,
	HW_VAR_CHK_MGQ_CPU_EMPTY,
	HW_VAR_DL_BCN_SEL,
	HW_VAR_AMPDU_MAX_TIME,
	HW_VAR_WIRELESS_MODE,
	HW_VAR_USB_MODE,
	HW_VAR_PORT_SWITCH,
	HW_VAR_PORT_CFG,
	HW_VAR_DM_IN_LPS_LCLK,/*flag CONFIG_LPS_LCLK_WD_TIMER*/
	#ifdef DBG_CHECK_FW_PS_STATE
	HW_VAR_FW_PS_STATE,
	#endif
	HW_VAR_SOUNDING_ENTER,
	HW_VAR_SOUNDING_LEAVE,
	HW_VAR_SOUNDING_RATE,
	HW_VAR_SOUNDING_STATUS,
	HW_VAR_SOUNDING_FW_NDPA,
	HW_VAR_SOUNDING_CLK,
	HW_VAR_SOUNDING_SET_GID_TABLE,
	HW_VAR_SOUNDING_CSI_REPORT,
	HW_VAR_DL_RSVD_PAGE,
	HW_VAR_DUMP_MAC_QUEUE_INFO,
	HW_VAR_ASIX_IOT,
	HW_VAR_CH_SW_NEED_TO_TAKE_CARE_IQK_INFO,
	HW_VAR_CH_SW_IQK_INFO_BACKUP,
	HW_VAR_CH_SW_IQK_INFO_RESTORE,

	HW_VAR_DBI,
	HW_VAR_MDIO,
	HW_VAR_L1OFF_CAPABILITY,
	HW_VAR_L1OFF_NIC_SUPPORT,
#ifdef CONFIG_TDLS
#ifdef CONFIG_TDLS_CH_SW
	HW_VAR_TDLS_BCN_EARLY_C2H_RPT,
#endif
#endif
	HW_VAR_DUMP_MAC_TXFIFO,
	HW_VAR_PWR_CMD,

	HW_VAR_SET_SOML_PARAM,
	HW_VAR_ENABLE_RX_BAR,
	HW_VAR_TSF_AUTO_SYNC,
	HW_VAR_LPS_STATE_CHK,
	#ifdef CONFIG_RTS_FULL_BW
	HW_VAR_SET_RTS_BW,
	#endif
#if defined(CONFIG_PCI_HCI)
	HW_VAR_ENSWBCN,
#endif
	HW_VAR_ACKTO,
	HW_VAR_ACKTO_CCK,
} HW_VARIABLES;

static inline u8 rtw_hal_set_hwreg(_adapter *padapter, u8 var, u8 *val)
{
	return 0;
}
static inline void rtw_hal_get_hwreg(_adapter *padapter, u8 var, u8 *val)
{}
typedef enum _HAL_DEF_VARIABLE {
	HAL_DEF_IS_SUPPORT_ANT_DIV,
	HAL_DEF_MAX_RECVBUF_SZ,
	HAL_DEF_RX_PACKET_OFFSET,
	HAL_DEF_DBG_DUMP_RXPKT,/* for dbg */
	HAL_DEF_BEAMFORMER_CAP,
	HAL_DEF_BEAMFORMEE_CAP,
	HW_VAR_MAX_RX_AMPDU_FACTOR,
	HW_VAR_MAX_RX_AMPDU_NUM,
	HW_DEF_RA_INFO_DUMP,
	HAL_DEF_DBG_DUMP_TXPKT,
	HAL_DEF_TX_PAGE_SIZE,
	HW_VAR_BEST_AMPDU_DENSITY,
} HAL_DEF_VARIABLE;

static inline u8 rtw_hal_set_def_var(_adapter *padapter, HAL_DEF_VARIABLE def_var, void *val)
{
	return 0;
}

u8 rtw_hal_get_def_var(struct _ADAPTER *a,
		       enum _HAL_DEF_VARIABLE def_var, void *val);

typedef enum _HAL_PHYDM_VARIABLE {
	HAL_PHYDM_STA_INFO,
	HAL_PHYDM_P2P_STATE,
	HAL_PHYDM_WIFI_DISPLAY_STATE,
	HAL_PHYDM_REGULATION,
	HAL_PHYDM_IGI,
	HAL_PHYDM_IGI_W,
	HAL_PHYDM_RX_INFO_DUMP,
	HAL_PHYDM_RX_DATA_INFO,
#ifdef CONFIG_ANTENNA_DIVERSITY
	HAL_PHYDM_ANTDIV_SELECT
#endif
} HAL_PHYDM_VARIABLE;

static inline void rtw_hal_set_phydm_var(_adapter *padapter,
	HAL_PHYDM_VARIABLE var, void *val, BOOLEAN set)
{}
static inline void rtw_hal_get_phydm_var(_adapter *padapter,
	HAL_PHYDM_VARIABLE var, void *val_1, void *val_2)
{}

static inline u8 rtw_hal_check_ips_status(_adapter *padapter)
{
	return 0;
}


static inline void rtw_hal_sec_read_cam_ent(_adapter *adapter, u8 id, u8 *ctrl, u8 *mac, u8 *key)
{}
static inline void rtw_hal_sec_write_cam_ent(_adapter *adapter, u8 id, u16 ctrl, u8 *mac, u8 *key)
{}
static inline void rtw_hal_sec_clr_cam_ent(_adapter *adapter, u8 id)
{}
static inline bool rtw_hal_sec_read_cam_is_gk(_adapter *adapter, u8 id)
{
	return _TRUE;
}

static inline u8 rtw_hal_get_current_tx_rate(_adapter *adapter, struct sta_info *psta)
{
	return 0;
}
static u8 rtw_get_current_tx_sgi(_adapter *padapter, struct sta_info *psta)
{
	return 0;
}

static inline void rtw_hal_linked_info_dump(_adapter *padapter, u8 benable)
{}

static inline bool rtw_hal_get_phy_edcca_flag(_adapter *adapter)
{
	return _TRUE;
}

static inline u64 rtw_hal_get_tsftr_by_port(_adapter *adapter, u8 port)
{
	return 1;
}

static inline void rtw_hal_dump_rsvd_page(void *sel, _adapter *adapter, u8 page_offset, u8 page_num)
{}


/*u8 beamforming_get_htndp_tx_rate(void *dm_void, u8 bfer_str_num);*/
static inline u8 rtw_hal_get_htndp_tx_rate(_adapter *adapter, u8 bfer_str_num)
{
	return 0;
}
/*u8 beamforming_get_vht_ndp_tx_rate(void *dm_void, u8 bfer_str_num);*/
static inline u8 rtw_hal_get_vht_ndp_tx_rate(_adapter *adapter, u8 bfer_str_num)
{
	return 0;
}

/*u8 phydm_get_beamforming_sounding_info(void *dm_void, u16 *throughput,
				       u8 total_bfee_num, u8 *tx_rate);*/
static inline u8 rtw_hal_get_sounding_info(_adapter *adapter,u16 *throughput,
				       u8 total_bfee_num, u8 *tx_rate)
{
	return 0;
}

static inline bool rtw_hal_query_phy_status(_adapter *adapter,
			     struct phydm_phyinfo_struct *phy_info,
			     u8 *phy_sts,
			     struct phydm_perpkt_info_struct *pktinfo)
{
	return _TRUE;
}
/*u8 phydm_get_plcp(void *dm_void, u16 macid)*/
static inline u8 rtw_hal_phydm_get_plcp(_adapter *adapter, u16 macid)
{
	return 0;
}


static inline void rtw_hal_dump_target_tx_power(void *sel, _adapter *adapter)
{}

static inline void rtw_hal_dump_tx_power_by_rate(void *sel, _adapter *adapter)
{}
static inline void rtw_hal_dump_tx_power_ext_info(void *sel, _adapter *adapter)
{}

static inline void rtw_hal_dump_macaddr(void *sel, _adapter *adapter)
{}

static inline void rtw_hal_dump_trx_mode(void *sel, _adapter *adapter)
{}

static inline void rtw_hal_phy_adaptivity_parm_msg(void *sel, _adapter *adapter)
{}

static inline void rtw_hal_phy_adaptivity_parm_set(_adapter *adapter,
					s8 th_l2h_ini, s8 th_edcca_hl_diff)
{}

#ifdef CONFIG_PCI_HCI
static inline void rtw_hal_irp_reset(_adapter *padapter)
{}
static inline void rtw_hal_pci_dbi_write(_adapter *padapter, u16 addr, u8 data)
{}
static inline u8 rtw_hal_pci_dbi_read(_adapter *padapter, u16 addr)
{	return 0;}
static inline void rtw_hal_pci_mdio_write(_adapter *padapter, u8 addr, u16 data)
{}
static inline u16 rtw_hal_pci_mdio_read(_adapter *padapter, u8 addr)
{	return 0;}
static inline u8 rtw_hal_pci_l1off_nic_support(_adapter *padapter)
{	return 0;}

static inline u8 rtw_hal_pci_l1off_capability(_adapter *padapter)
{	return 0;}

static inline void rtw_hal_unmap_beacon_icf(_adapter *padapter)
{
	//hal->hal_ops.unmap_beacon_icf(padapter);
}
#endif

#if defined(CONFIG_PCI_HCI)
static inline u8 rtw_hal_check_nic_enough_desc_all(_adapter *padapter)
{ return _SUCCESS;}
static s32 rtw_hal_dump_xframe(_adapter *adapter, struct xmit_frame *pxmitframe)
{ return _SUCCESS;}


#endif

static inline void rtw_hal_sta_ra_registed(struct sta_info *psta)
{}
static inline s32 rtw_hal_macid_sleep(_adapter *adapter, u8 macid)
{	return 0;}
static inline s32 rtw_hal_macid_wakeup(_adapter *adapter, u8 macid)
{	return 0;}
static inline s32 rtw_hal_macid_sleep_all_used(_adapter *adapter)
{	return 0;}
static inline s32 rtw_hal_macid_wakeup_all_used(_adapter *adapter)
{	return 0;}

static void rtw_hal_c2h_pkt_hdl(_adapter *adapter, u8 *buf, u16 len)
{
	//adapter->dvobj->hal_func.hal_mac_c2h_handler(adapter, buf, len);
}
static inline s32 rtw_hal_fill_h2c_cmd(_adapter *padapter, u8 ElementID, u32 CmdLen, u8 *pCmdBuffer)
{
/*
	_adapter *pri_adapter = GET_PRIMARY_ADAPTER(padapter);

	if (GET_HAL_DATA(pri_adapter)->fw_ready == _TRUE)
		return hal->hal_ops.fill_h2c_cmd(padapter, ElementID, CmdLen, pCmdBuffer);
	else if (padapter->registrypriv.mp_mode == 0)
		RTW_PRINT(FUNC_ADPT_FMT" FW doesn't exit when no MP mode, by pass H2C id:0x%02x\n"
			  , FUNC_ADPT_ARG(padapter), ElementID);
*/
	return 0;
}
static inline void rtw_hal_dm_watchdog(_adapter *padapter)
{}
static inline void rtw_hal_dm_watchdog_in_lps(_adapter *padapter)
{}

static inline void rtw_hal_reqtxrpt(_adapter *padapter, u8 macid)
{
	//if (hal->hal_ops.reqtxrpt)
		//hal->hal_ops.reqtxrpt(padapter, macid);
}
static inline u8 rtw_hal_get_port(_adapter *adapter)
{	return 0;}

static inline void rtw_hal_read_edca(_adapter *adapter, u16 *vo_params, u16 *vi_params,
			u16 *be_params, u16 *bk_params)
{
	//hal->hal_func.read_wmmedca_reg(padapter, vo_params, vi_params, be_params, bk_params);
}

static inline void rtw_phydm_set_rrsr(_adapter *adapter, u32 rrsr_value, bool write_rrsr)
{}
static inline void rtw_phydm_dyn_rrsr_en(_adapter *adapter, bool en_rrsr)
{}
static inline void rtw_hal_update_iqk_fw_offload_cap(_adapter *adapter)
{}

static inline void rtw_hal_dump_sta_traffic(void *sel, _adapter *adapter, struct sta_info *psta)
{}
static inline s32 rtw_hal_set_FwMediaStatusRpt_single_cmd
	(_adapter *adapter, bool opmode, bool miracast, bool miracast_sink, u8 role, u8 macid)
{	return 0;}

static inline void rtw_hal_set_network_type(_adapter *adapter, u8 type)
{}
static inline void rtw_hal_rcr_set_chk_bssid(_adapter *adapter, u8 self_action)
{}
static inline void rtw_mi_ap_correct_tsf(_adapter *adapter, u8 mlme_state)
{}
/*
* This file provides utilities/wrappers for rtw driver to use ODM
*/
typedef enum _HAL_PHYDM_OPS {
	HAL_PHYDM_DIS_ALL_FUNC,
	HAL_PHYDM_FUNC_SET,
	HAL_PHYDM_FUNC_CLR,
	HAL_PHYDM_ABILITY_BK,
	HAL_PHYDM_ABILITY_RESTORE,
	HAL_PHYDM_ABILITY_SET,
	HAL_PHYDM_ABILITY_GET,
} HAL_PHYDM_OPS;

#define ODM_BB_ENV_MONITOR BIT15
#define ODM_BB_ADAPTIVITY BIT17

#define DYNAMIC_FUNC_DISABLE		(0x0)
static inline bool rtw_odm_adaptivity_needed(_adapter *adapter)
{
/*
	struct registry_priv *regsty = &adapter->registrypriv;
	bool ret = _FALSE;

	if (regsty->adaptivity_en == RTW_ADAPTIVITY_EN_ENABLE)
		ret = _TRUE;

	return ret;
*/
	return 0;
}

static inline u32 rtw_phydm_ability_ops(_adapter *adapter, HAL_PHYDM_OPS ops, u32 ability)
{	return 0;}

#define rtw_phydm_func_disable_all(adapter)	\
		rtw_phydm_ability_ops(adapter, HAL_PHYDM_DIS_ALL_FUNC, 0)

#ifdef CONFIG_RTW_ACS
#define rtw_phydm_func_for_offchannel(adapter) \
		do { \
			rtw_phydm_ability_ops(adapter, HAL_PHYDM_DIS_ALL_FUNC, 0); \
			if (rtw_odm_adaptivity_needed(adapter)) \
				rtw_phydm_ability_ops(adapter, HAL_PHYDM_FUNC_SET, ODM_BB_ADAPTIVITY); \
			if (IS_ACS_ENABLE(adapter))\
				rtw_phydm_ability_ops(adapter, HAL_PHYDM_FUNC_SET, ODM_BB_ENV_MONITOR); \
		} while (0)
#else
#define rtw_phydm_func_for_offchannel(adapter) \
		do { \
			rtw_phydm_ability_ops(adapter, HAL_PHYDM_DIS_ALL_FUNC, 0); \
			if (rtw_odm_adaptivity_needed(adapter)) \
				rtw_phydm_ability_ops(adapter, HAL_PHYDM_FUNC_SET, ODM_BB_ADAPTIVITY); \
		} while (0)
#endif

#define rtw_phydm_func_clr(adapter, ability)	\
		rtw_phydm_ability_ops(adapter, HAL_PHYDM_FUNC_CLR, ability)

#define rtw_phydm_ability_backup(adapter)	\
		rtw_phydm_ability_ops(adapter, HAL_PHYDM_ABILITY_BK, 0)

#define rtw_phydm_ability_restore(adapter)	\
		rtw_phydm_ability_ops(adapter, HAL_PHYDM_ABILITY_RESTORE, 0)

enum QSEL_ID {
	QSLT_BK_ID,
	QSLT_BE_ID,
	QSLT_VI_ID,
	QSLT_VO_ID,
	QSLT_BEACON_ID,
	QSLT_HIGH_ID,
	QSLT_MGNT_ID,
	QSLT_CMD_ID
};

static inline u8 rtw_hal_get_qsel(_adapter *adapter, enum QSEL_ID qsel)
{
	/*QSLT_HIGH*/
	return 0;
}
/************************ xmit *******************/


/*check_nic_enough_desc*/

static inline s32 rtw_hal_pci_dump_xframe(_adapter *adapter, struct xmit_frame *pxmitframe)
{
	return 0;
}
static inline u8 rtw_hal_busagg_qsel_check(_adapter *padapter,
			u8 pre_qsel, u8 next_qsel)
{
	return _FAIL;
}

static inline void rtw_hal_bcn_param_setting(_adapter *padapter)
{
	//hal->hal_ops.set_beacon_param_handler(padapter);
}
static inline void rtw_phydm_ra_registed(_adapter *adapter, struct sta_info *psta)
{}
static inline void rtw_hal_dump_txpwr_lmt(void *sel, _adapter *adapter)
{}
static inline void rtw_store_phy_info(_adapter *padapter, union recv_frame *prframe)
{}
static inline void rtw_hal_parse_rx_phy_status_chinfo(union recv_frame *rframe, u8 *phys)
{}
static inline void rtw_hal_set_tx_power_level(_adapter *adapter, u8 channel)
{}

static inline void rtw_phydm_reset(_adapter *adapter)
{}

/****************** GEORGIA_TODO_REDEFINE_IO ************************/
static inline u32 rtw_hal_get_htsf(_adapter *adapter)/*get tst high 4 bytes */
{
	return 0;
}
static inline u32 rtw_hal_get_ltsf(_adapter *adapter)/*get tst low 4 bytes */
{
	return 0;
}

static inline u32 rtw_hal_get_dma_statu(_adapter *adapter)
{
	return 0;
}
#ifdef DBG_TXBD_DESC_DUMP
static inline u32 rtw_hal_get_txbd_rwreg(_adapter *adapter)
{
	return 0;
}
#endif

#ifdef RTW_SUPPORT_PLATFORM_SHUTDOWN
static inline u8 rtw_hal_sdio_leave_suspend(_adapter *adapter)
{
	return 0;
}
#endif

#if defined(CONFIG_FWLPS_IN_IPS)
static inline void rtw_hal_set_fw_in_ips_mode(_adapter *padapter, u8 enable)
{}
#endif
#ifdef CONFIG_LPS_RPWM_TIMER
static inline bool rtw_hal_is_leave_ps(_adapter *padapter)
{
	return _FALSE;
}
#endif


static inline void rtw_hal_get_version(char *str, u32 len)
{
	//get hal version
	//rtw_halmac_get_version(str, 30);
	// get fw version
	// get phy (bb/rf) version
	// get btc version
}


#endif /*_HAL_API_TMP_H_*/
