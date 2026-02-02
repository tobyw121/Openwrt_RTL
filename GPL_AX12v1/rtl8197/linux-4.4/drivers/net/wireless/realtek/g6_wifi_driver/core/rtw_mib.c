#include <drv_types.h>
#include <rtw_ap.h>
#include <rtw_mib.h>
#ifdef CONFIG_RTW_AP_EXT_SUPPORT
/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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
#define _RTW_MIB_C_

typedef enum {
	BYTE_T,
	INT_T,
	STRING_T,
	SHORT_T
} TYPE_T;

typedef enum {
	RTW_MIB_GET,
	RTW_MIB_SET,
} MIB_OPER;

struct iwpriv_arg {
	char name[32];		/* mib name */
	TYPE_T type;		/* Type and number of args */
	int offset_prev;	/* pram offset in driver previous */
	int offset_mib;		/* mib offset in struct wifi_mib_priv */
	int len;			/* mib byte len */
	int Default;		/* mib default value */
	void (*fun)(_adapter *, char *, u32);	/* call back function */
};

#define _OFFSET(field)	((uintptr_t)&(((_adapter *)0)->field))
#define _MIB_OFFSET(field)	_OFFSET(registrypriv.wifi_mib)+((uintptr_t)&(((struct wifi_mib_priv *)0)->field))
#define _SIZE(field)	sizeof(((_adapter *)0)->field)

#define MAX_MIB_DATA_SIZE	1024
#ifdef CONFIG_RTW_MULTI_AP
extern u32	rtw_map_user_pid;
#endif
/*******************************************************************************************************************************************
	Each entry in mib_table has SEVEN parms, including priv_mib_name, type, offset, mib_offset, size, default_value and call_back_functions.
	offset_prev: The offset of someone parm in other struct EXCEPT _adapter.registrypriv.wifi_mib.
	offset_mib: The offset of someone parm in _adapter.registrypriv.wifi_mib.
	Please be sure to add SEVEN relevant parms when adding one new entry.
********************************************************************************************************************************************/
static struct iwpriv_arg mib_table[] = {
	{"rtw_mib_version",		STRING_T,	0,													_MIB_OFFSET(rtw_mib_version),				_SIZE(registrypriv.wifi_mib.rtw_mib_version),	0,		core_mib_version},
	{"rtw_mib_size",		INT_T,		0,													_MIB_OFFSET(rtw_mib_size),					_SIZE(registrypriv.wifi_mib.rtw_mib_size),		0,		core_mib_size},
	{"ampdu",				BYTE_T,		_OFFSET(registrypriv.dot11nAMPDU),					_MIB_OFFSET(ampdu),							_SIZE(registrypriv.dot11nAMPDU),				1,		core_mib_ampdu},
	{"disable_protection",	BYTE_T,		_OFFSET(registrypriv.protectionDisabled),			_MIB_OFFSET(disable_protection),			_SIZE(registrypriv.protectionDisabled),			0,		NULL},
	{"func_off",			INT_T,		0,													_MIB_OFFSET(func_off),						_SIZE(registrypriv.wifi_mib.func_off),			0,		core_mib_func_off},
	{"aggregation",			BYTE_T,		0,													_MIB_OFFSET(aggregation),					_SIZE(registrypriv.wifi_mib.aggregation),		0,		NULL},
	{"iapp_enable",			BYTE_T,		0,													_MIB_OFFSET(iapp_enable),					_SIZE(registrypriv.wifi_mib.iapp_enable),		0,		NULL},
#ifdef CONFIG_80211N_HT
	{"coexist",				BYTE_T,		_OFFSET(registrypriv.ht_20_40_coexist),				_MIB_OFFSET(coexist)	,					_SIZE(registrypriv.ht_20_40_coexist),			0,		NULL},
#endif
#ifdef CONFIG_80211AX_HE
	{"fw_tx",				BYTE_T,		_OFFSET(registrypriv.fw_tx),						0,											_SIZE(registrypriv.fw_tx),						0,		core_mib_fw_tx},
#ifdef CONFIG_RTW_TWT
	{"twt_enable",			BYTE_T,		_OFFSET(registrypriv.twt_enable),					0,											_SIZE(registrypriv.twt_enable),					0,		core_mib_twt_enable},
	{"twt_dbgmode",			BYTE_T,		_OFFSET(registrypriv.twt_dbgmode),					0,											_SIZE(registrypriv.twt_dbgmode),					0,		NULL},
#endif
#endif
#ifdef CONFIG_RTW_A4_STA
	{"a4_enable",			INT_T,		0,													_MIB_OFFSET(a4_enable),						_SIZE(registrypriv.wifi_mib.a4_enable),			0,		core_mib_a4_enable},
#endif
#ifdef CONFIG_RTW_MULTI_AP
	{"multiap_monitor_mode_disable",	BYTE_T,	0,											_MIB_OFFSET(multiap_monitor_mode_disable),	_SIZE(registrypriv.wifi_mib.multiap_monitor_mode_disable),		0, core_mib_multiap_monitor_mode_disable},
	{"multiap_bss_type",				BYTE_T,	0,											_MIB_OFFSET(multiap_bss_type),				_SIZE(registrypriv.wifi_mib.multiap_bss_type),					0, core_mib_multiap_bss_type},
	{"multiap_profile",					BYTE_T,	0,											_MIB_OFFSET(multiap_profile),				_SIZE(registrypriv.wifi_mib.multiap_profile),					0, NULL},
	{"multiap_max_device_reached",		BYTE_T,	0,											_MIB_OFFSET(multiap_max_device_reached),	_SIZE(registrypriv.wifi_mib.multiap_max_device_reached),		0, core_mib_multiap_max_device_reached},
	{"multiap_steering_policy", 		BYTE_T,	0,											_MIB_OFFSET(multiap_steering_policy),		_SIZE(registrypriv.wifi_mib.multiap_steering_policy), 			0, NULL},
	{"multiap_rcpi_steering_th", 		BYTE_T,	0,											_MIB_OFFSET(multiap_rcpi_steering_th),		_SIZE(registrypriv.wifi_mib.multiap_rcpi_steering_th),	 	 	0, NULL},
	{"multiap_change_channel",			BYTE_T,	0,											_MIB_OFFSET(multiap_change_channel),		_SIZE(registrypriv.wifi_mib.multiap_change_channel),			0, core_mib_multiap_change_channel},
	{"multiap_dfs_ap_mib_channel",		BYTE_T,	0,											_MIB_OFFSET(multiap_dfs_ap_mib_channel),	_SIZE(registrypriv.wifi_mib.multiap_dfs_ap_mib_channel),		0, NULL},
	{"multiap_ext_cmd",					BYTE_T, 0,											_MIB_OFFSET(multiap_change_channel),		_SIZE(registrypriv.wifi_mib.multiap_change_channel),			0, core_mib_multiap_ext_cmd},
	{"multiap_report_fail_assoc",		BYTE_T, 0,											_MIB_OFFSET(multiap_report_fail_assoc),		_SIZE(registrypriv.wifi_mib.multiap_report_fail_assoc),			0, NULL},
	{"multiap_vlan_enable",				BYTE_T, 0,											_MIB_OFFSET(multiap_vlan_enable),			_SIZE(registrypriv.wifi_mib.multiap_vlan_enable),										0, NULL},
	{"multiap_vlan_id",					INT_T, 	0, 											_MIB_OFFSET(multiap_vlan_id),				_SIZE(registrypriv.wifi_mib.multiap_vlan_id),											0, NULL},
#endif
	{"max_tx_power",		BYTE_T, 	0,													_MIB_OFFSET(max_tx_power),					_SIZE(registrypriv.wifi_mib.max_tx_power),		20,		NULL},
#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
	{"tx_mode",				BYTE_T,		_OFFSET(registrypriv.tx_mode),						0,											_SIZE(registrypriv.tx_mode),					0,		core_mib_tx_mode},
	{"tx_mode_auto",		INT_T,		0,													_MIB_OFFSET(tx_mode_auto),					_SIZE(registrypriv.wifi_mib.tx_mode_auto),		1,		NULL},
#endif
	{"txforce",				INT_T,		0,													_MIB_OFFSET(txforce),						_SIZE(registrypriv.wifi_mib.txforce),			0xffff,	core_mib_txforce},
	{"gbwcmode",			INT_T,		0,													_MIB_OFFSET(gbwcmode),						_SIZE(registrypriv.wifi_mib.gbwcmode),			0,		NULL},
	{"gbwcthrd_tx",			INT_T,		0,													_MIB_OFFSET(gbwcthrd_tx),					_SIZE(registrypriv.wifi_mib.gbwcthrd_tx),		0,		NULL},
	{"gbwcthrd_rx",			INT_T,		0,													_MIB_OFFSET(gbwcthrd_rx),					_SIZE(registrypriv.wifi_mib.gbwcthrd_rx),		0,		NULL},
	{"telco_selected",		BYTE_T,		0,													_MIB_OFFSET(telco_selected),				_SIZE(registrypriv.wifi_mib.telco_selected),	0,		NULL},
	{"regdomain",			INT_T,		0,													_MIB_OFFSET(regdomain),						_SIZE(registrypriv.wifi_mib.regdomain),			0,		NULL},
	{"dfs_regions",			INT_T,		0,													_MIB_OFFSET(dfs_regions), 					_SIZE(registrypriv.wifi_mib.dfs_regions), 		3, 		NULL},
	{"led_type",			INT_T,		0,													_MIB_OFFSET(led_type),						_SIZE(registrypriv.wifi_mib.led_type),			0,		NULL},
	{"lifetime",			INT_T,		0,													_MIB_OFFSET(lifetime),						_SIZE(registrypriv.wifi_mib.lifetime),			0,		core_mib_lifetime},
	{"manual_priority",		INT_T,		0,													_MIB_OFFSET(manual_priority),				_SIZE(registrypriv.wifi_mib.manual_priority),	0xff,	NULL},
	{"opmode",				INT_T,		_OFFSET(mlmepriv.fw_state),							_MIB_OFFSET(opmode),						_SIZE(registrypriv.wifi_mib.opmode),			0,		NULL},
	{"autorate",			INT_T,		0,													_MIB_OFFSET(autorate),						_SIZE(registrypriv.wifi_mib.autorate),			1,		core_mib_auto_rate},
	{"fixrate",				INT_T,		0,													_MIB_OFFSET(fixrate),						_SIZE(registrypriv.wifi_mib.fixrate),			0xffff,	core_mib_fix_rate},
	{"deny_legacy",			INT_T,		0,													_MIB_OFFSET(deny_legacy),					_SIZE(registrypriv.wifi_mib.deny_legacy),		0,		core_mib_deny_legacy},
	{"lgyEncRstrct",		INT_T,		0,													_MIB_OFFSET(lgyEncRstrct),					_SIZE(registrypriv.wifi_mib.lgyEncRstrct),		0,		NULL},
	{"cts2self",			INT_T,		0,													_MIB_OFFSET(cts2self),						_SIZE(registrypriv.wifi_mib.cts2self),			0,		NULL},
	{"amsdu",				BYTE_T,		0,													_MIB_OFFSET(amsdu),							_SIZE(registrypriv.wifi_mib.amsdu),				0,		core_mib_amsdu},
#ifdef CONFIG_RTW_BYPASS_DEAMSDU
	{"bypass_deamsdu",		INT_T,		0,													_MIB_OFFSET(bypass_deamsdu),				_SIZE(registrypriv.wifi_mib.bypass_deamsdu),	1,		NULL},
#endif
	{"crossband_enable",	BYTE_T,		0,													_MIB_OFFSET(crossband_enable),				_SIZE(registrypriv.wifi_mib.crossband_enable),	0,		NULL},
	{"monitor_sta_enabled",	INT_T,		0,													_MIB_OFFSET(monitor_sta_enabled),			_SIZE(registrypriv.wifi_mib.monitor_sta_enabled), 0,	NULL},
	{"txbf",				INT_T,		0,													_MIB_OFFSET(txbf),							_SIZE(registrypriv.wifi_mib.txbf),				0,		NULL},
	{"txbfer",				INT_T,		0,													_MIB_OFFSET(txbfer),						_SIZE(registrypriv.wifi_mib.txbfer),			1,		NULL},
	{"txbfee",				INT_T,		0,													_MIB_OFFSET(txbfee),						_SIZE(registrypriv.wifi_mib.txbfee),			1,		NULL},
	{"txbf_state",			BYTE_T,		0,													_MIB_OFFSET(txbf_state),					_SIZE(registrypriv.wifi_mib.txbf_state),		0xF,	NULL},
	{"txbf_apply",			BYTE_T,		0,													_MIB_OFFSET(txbf_apply),					_SIZE(registrypriv.wifi_mib.txbf_apply),		0,		NULL},
	{"txbf_mu",				INT_T,		0,													_MIB_OFFSET(txbf_mu),						_SIZE(registrypriv.wifi_mib.txbf_mu),			0,		NULL},
	{"txbf_mu_state",		BYTE_T,		0,													_MIB_OFFSET(txbf_mu_state),					_SIZE(registrypriv.wifi_mib.txbf_mu_state),		0xF,	NULL},
	{"txbf_mu_apply",		BYTE_T,		0,													_MIB_OFFSET(txbf_mu_apply),					_SIZE(registrypriv.wifi_mib.txbf_mu_apply),		0,		NULL},
	{"txbf_period",			INT_T,		0,													_MIB_OFFSET(txbf_period),					_SIZE(registrypriv.wifi_mib.txbf_period),		30,		NULL},
	{"txbf_mu_2ss",			INT_T,		0,													_MIB_OFFSET(txbf_mu_2ss),					_SIZE(registrypriv.wifi_mib.txbf_mu_2ss),		0,		NULL},
	{"txbf_mu_1ss",			INT_T,		0,													_MIB_OFFSET(txbf_mu_1ss),					_SIZE(registrypriv.wifi_mib.txbf_mu_1ss),		0,		NULL},
	{"txbf_tp_limit",		INT_T,		0,													_MIB_OFFSET(txbf_tp_limit),					_SIZE(registrypriv.wifi_mib.txbf_tp_limit),		0,		NULL},
	{"txbf_mu_amsdu",		INT_T,		0,													_MIB_OFFSET(txbf_mu_amsdu),					_SIZE(registrypriv.wifi_mib.txbf_mu_amsdu),		1,		NULL},
	{"txbf_csi_rate",		INT_T,		0,													_MIB_OFFSET(txbf_csi_rate),					_SIZE(registrypriv.wifi_mib.txbf_csi_rate),		0,		NULL},
	{"txbf_force_regrp",	BYTE_T,		0,													_MIB_OFFSET(txbf_force_regrp),				_SIZE(registrypriv.wifi_mib.txbf_force_regrp),	0,		NULL},
	{"txbf_offload",		BYTE_T,		0,													_MIB_OFFSET(txbf_offload),					_SIZE(registrypriv.wifi_mib.txbf_offload),		0,		NULL},
	{"txbf_auto_snd",		BYTE_T,		0,													_MIB_OFFSET(txbf_auto_snd),					_SIZE(registrypriv.wifi_mib.txbf_auto_snd),		1,		core_mib_txbf_auto_snd},
	{"roaming_switch",		BYTE_T,		0,													_MIB_OFFSET(roaming_switch),				_SIZE(registrypriv.wifi_mib.roaming_switch),	0,		NULL},
	{"roaming_qos",			BYTE_T,		0,													_MIB_OFFSET(roaming_qos),					_SIZE(registrypriv.wifi_mib.roaming_qos),		0,		NULL},
	{"fail_ratio",			BYTE_T,		0,													_MIB_OFFSET(fail_ratio),					_SIZE(registrypriv.wifi_mib.fail_ratio),		0,		NULL},
	{"retry_ratio",			BYTE_T,		0,													_MIB_OFFSET(retry_ratio),					_SIZE(registrypriv.wifi_mib.retry_ratio),		0,		NULL},
	{"RSSIThreshold",		BYTE_T,		0,													_MIB_OFFSET(RSSIThreshold),					_SIZE(registrypriv.wifi_mib.RSSIThreshold),		0,		NULL},
	{"dfgw_mac",			STRING_T,	0,													_MIB_OFFSET(dfgw_mac),						_SIZE(registrypriv.wifi_mib.dfgw_mac),			0,		NULL},
	{"roaming_enable",		BYTE_T,		0,													_MIB_OFFSET(roaming_enable),				_SIZE(registrypriv.wifi_mib.roaming_enable),	0,		NULL},
	{"roaming_start_time",	INT_T,		0,													_MIB_OFFSET(roaming_start_time),			_SIZE(registrypriv.wifi_mib.roaming_start_time),0,		NULL},
	{"roaming_rssi_th1",	BYTE_T,		0,													_MIB_OFFSET(roaming_rssi_th1),				_SIZE(registrypriv.wifi_mib.roaming_rssi_th1),	0,		NULL},
	{"roaming_rssi_th2",	BYTE_T,		0,													_MIB_OFFSET(roaming_rssi_th2),				_SIZE(registrypriv.wifi_mib.roaming_rssi_th2),	0,		NULL},
	{"roaming_wait_time",	INT_T,		0,													_MIB_OFFSET(roaming_wait_time),				_SIZE(registrypriv.wifi_mib.roaming_wait_time),	0,		NULL},
	{"tpc_tx_power",		BYTE_T,		0,													_MIB_OFFSET(tpc_tx_power),					_SIZE(registrypriv.wifi_mib.tpc_tx_power),		0,		core_mib_tpc_tx_power},
	{"band",				BYTE_T,		0,													_MIB_OFFSET(band),							_SIZE(registrypriv.wifi_mib.band),				0,		core_mib_band},
	{"ssid",				STRING_T,	_OFFSET(mlmepriv.cur_network.network.Ssid.Ssid),	0,											_SIZE(mlmepriv.cur_network.network.Ssid.Ssid),	0,		NULL},
	{"authtype",			INT_T,		_OFFSET(securitypriv.dot11AuthAlgrthm),				0,											_SIZE(securitypriv.dot11AuthAlgrthm),			0,		core_mib_authtype},
	{"encmode",				BYTE_T,		_OFFSET(securitypriv.dot11PrivacyAlgrthm),			0,											_SIZE(securitypriv.dot11PrivacyAlgrthm),		0,		core_mib_encmode},
	{"channel",				BYTE_T,		0,													_MIB_OFFSET(set_channel),					_SIZE(registrypriv.wifi_mib.set_channel),		0,		core_mib_channel},
	{"use40M",				BYTE_T,		0,													_MIB_OFFSET(set_bwmode),					_SIZE(registrypriv.wifi_mib.set_bwmode),		0,		core_mib_use40M},
	{"2ndchoffset",			BYTE_T,		0,													_MIB_OFFSET(set_ch_offset),					_SIZE(registrypriv.wifi_mib.set_ch_offset),		0,		core_mib_2ndchoffset},
	{"rssi_dump",			BYTE_T,		0,													_MIB_OFFSET(rssi_dump),						_SIZE(registrypriv.wifi_mib.rssi_dump),			0,		core_mib_rssi_dump},
	{"totaltp",				BYTE_T,		0,													_MIB_OFFSET(totaltp_dump),					_SIZE(registrypriv.wifi_mib.totaltp_dump),		0,		NULL},
#if defined(CONFIG_TX_DEFER) || defined(WFO_STRUCT_ALIGNED)
	{"defertx",				BYTE_T,		0,													_MIB_OFFSET(defer_tx_sched),				_SIZE(registrypriv.wifi_mib.defer_tx_sched),	1,		NULL},
	{"defertxcnt",			BYTE_T,		0,													_MIB_OFFSET(defer_tx_cnt),					_SIZE(registrypriv.wifi_mib.defer_tx_cnt),		10,		NULL},
	{"defertxtp",			INT_T,		0,													_MIB_OFFSET(defer_tx_tp),					_SIZE(registrypriv.wifi_mib.defer_tx_tp),		500,	NULL},
	{"defertxto",			INT_T,		0,													_MIB_OFFSET(defertxtimeout),				_SIZE(registrypriv.wifi_mib.defertxtimeout),	500,		NULL},
#endif
	{"ofdma_enable",		BYTE_T,		0,													_MIB_OFFSET(ofdma_enable),					_SIZE(registrypriv.wifi_mib.ofdma_enable),		0,		NULL},
	{"edcca_mode",			BYTE_T,		0,													_MIB_OFFSET(edcca_mode),					_SIZE(registrypriv.wifi_mib.edcca_mode),		0,		core_mib_edcca_mode},
	{"bfrp_mode",			BYTE_T,		0,													_MIB_OFFSET(bfrp_mode),					_SIZE(registrypriv.wifi_mib.bfrp_mode),		0,		core_mib_bfrp_mode},
	{"su_trig_enable",			BYTE_T,		0,													_MIB_OFFSET(su_trig_enable),					_SIZE(registrypriv.wifi_mib.su_trig_enable),		0,		core_mib_su_trig_enable},

	{"en_timer_pfd",			BYTE_T,		0,													_MIB_OFFSET(en_timer_pfd),					_SIZE(registrypriv.wifi_mib.en_timer_pfd),		1,		core_mib_en_timer_pfd},
	{"en_txrpt_pfd",			BYTE_T,		0,													_MIB_OFFSET(en_txrpt_pfd),					_SIZE(registrypriv.wifi_mib.en_txrpt_pfd),		1,		core_mib_en_txrpt_pfd},
	{"set_timer_period",			BYTE_T,		0,													_MIB_OFFSET(set_timer_period),					_SIZE(registrypriv.wifi_mib.set_timer_period),		1,		core_mib_set_timer_period},
	{"timer_period",			BYTE_T,		0,													_MIB_OFFSET(timer_period),					_SIZE(registrypriv.wifi_mib.timer_period),		5,		core_mib_timer_period},
	{"timer_clr_period",			BYTE_T,		0,													_MIB_OFFSET(timer_clr_period),					_SIZE(registrypriv.wifi_mib.timer_clr_period),		1,		core_mib_timer_clr_period},

	{"guest_access",		BYTE_T,		0,													_MIB_OFFSET(guest_access),					_SIZE(registrypriv.wifi_mib.guest_access),		0,		NULL},
	{"block_relay",			BYTE_T,		0, 													_MIB_OFFSET(block_relay),					_SIZE(registrypriv.wifi_mib.block_relay),		0,		NULL},
	{"mc2u_disable",		BYTE_T,		0,													_MIB_OFFSET(mc2u_disable),					_SIZE(registrypriv.wifi_mib.mc2u_disable),		0,		NULL},
	{"disable_dfs",			BYTE_T,		0,													_MIB_OFFSET(disable_dfs),					_SIZE(registrypriv.wifi_mib.disable_dfs),		0,		NULL},
	{"powerpercent",		BYTE_T,		0,													_MIB_OFFSET(powerpercent),					_SIZE(registrypriv.wifi_mib.powerpercent),		100,	core_mib_power_percent}, //unit: 1~200 percnet
	{"power_ref",			INT_T,		0,													_MIB_OFFSET(power_reference),				_SIZE(registrypriv.wifi_mib.power_reference),	0,		core_mib_power_ref}, //unit: +-dB
	{"aclmode",				INT_T,		0,													_MIB_OFFSET(aclmode),						_SIZE(registrypriv.wifi_mib.aclmode),			0,		core_mib_aclmode},
	{"aclnum",				INT_T,		0,													_MIB_OFFSET(aclnum), _SIZE(registrypriv.wifi_mib.aclnum), 0, core_mib_aclnum},
	{"scan_backop",			BYTE_T,		0,													_MIB_OFFSET(scan_backop),					_SIZE(registrypriv.wifi_mib.scan_backop),		1,		NULL},
	{"scan_backop_div",		BYTE_T,		0,													_MIB_OFFSET(scan_backop_div),				_SIZE(registrypriv.wifi_mib.scan_backop_div),	3,		NULL},
	{"scan_backop_dur",		INT_T,		0,													_MIB_OFFSET(scan_backop_dur),				_SIZE(registrypriv.wifi_mib.scan_backop_dur),	400,	NULL},
	//client mode
	{"bssid",				INT_T,		0,													_MIB_OFFSET(bssid),							_SIZE(registrypriv.wifi_mib.bssid),				0,		NULL},
	{"connect_ssid",		INT_T,		0,													_MIB_OFFSET(connect_ssid),					_SIZE(registrypriv.wifi_mib.connect_ssid),		0,		NULL},
	{"connect_ssid_len",	INT_T,		0,													_MIB_OFFSET(connect_ssid_len),				_SIZE(registrypriv.wifi_mib.connect_ssid_len),	0,		NULL},
	{"connect_ch",			INT_T,		0,													_MIB_OFFSET(connect_ch),					_SIZE(registrypriv.wifi_mib.connect_ch),		0,		NULL},
	{"agg_num_buf",			BYTE_T,		_OFFSET(registrypriv.agg_num_buf),					0,											_SIZE(registrypriv.agg_num_buf),				0,		NULL},
	{"ldpc",				BYTE_T,		0,													_MIB_OFFSET(ldpc),							_SIZE(registrypriv.wifi_mib.ldpc),				1,		NULL},
	{"stbc",				BYTE_T,		0,													_MIB_OFFSET(stbc),							_SIZE(registrypriv.wifi_mib.stbc),				(STBC_RX_EN | STBC_TX_EN) ,		NULL},
#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
	{"crossband_pathReady",	BYTE_T,		0,													_MIB_OFFSET(crossband_pathReady),			_SIZE(registrypriv.wifi_mib.crossband_pathReady),	0,	NULL},
	{"crossband_assoc",		BYTE_T,		0,													_MIB_OFFSET(crossband_assoc),				_SIZE(registrypriv.wifi_mib.crossband_assoc),	0,		NULL},
	{"crossband_preferBand",BYTE_T,		0,													_MIB_OFFSET(crossband_prefer),				_SIZE(registrypriv.wifi_mib.crossband_prefer),	0,		NULL},
#endif
#ifdef CONFIG_RTW_ACS
	{"acs",						BYTE_T,		0,												_MIB_OFFSET(acs),							_SIZE(registrypriv.wifi_mib.acs),				0,		NULL},
#ifdef CONFIG_CTC_FEATURE
	{"autoch_1611_enable",		BYTE_T,		0,												_MIB_OFFSET(autoch_1611_enable),			_SIZE(registrypriv.wifi_mib.autoch_1611_enable),	0,	NULL},
#else
	{"autoch_1611_enable",		BYTE_T,		0,												_MIB_OFFSET(autoch_1611_enable),			_SIZE(registrypriv.wifi_mib.autoch_1611_enable),	0,	NULL},
#endif
#ifdef CONFIG_CTC_FEATURE
	{"autoch_3664157_enable",	BYTE_T,		0,												_MIB_OFFSET(autoch_3664157_enable),			_SIZE(registrypriv.wifi_mib.autoch_3664157_enable),	0,	NULL},
#else
	{"autoch_3664157_enable",	BYTE_T,		0,												_MIB_OFFSET(autoch_3664157_enable),			_SIZE(registrypriv.wifi_mib.autoch_3664157_enable),	0,	NULL},
#endif
#endif
#ifdef CONFIG_RTW_MIRROR_DUMP
	{"mirror_dump",			INT_T,		0,													_MIB_OFFSET(mirror_dump),					_SIZE(registrypriv.wifi_mib.mirror_dump),		0,		core_mib_mirror_dump},
	{"mirror_txch",			BYTE_T, 	0,													_MIB_OFFSET(mirror_txch),					_SIZE(registrypriv.wifi_mib.mirror_txch),		0,		core_mib_mirror_txch},
#endif
	{"sleep_q_max_num", 	BYTE_T, 	0,													_MIB_OFFSET(sleep_q_max_num),				_SIZE(registrypriv.wifi_mib.sleep_q_max_num),	32,		NULL},
	{"psk_enable", 			INT_T,	0,			0,											sizeof(unsigned int),		0,		core_mib_psk_enable},
	{"wpa_cipher",			INT_T,	0,			0,											sizeof(unsigned int),		0,		core_mib_wpa_cipher},
	{"wpa2_cipher",			INT_T,	0,			0,											sizeof(unsigned int),		0,		core_mib_wpa2_cipher},
#ifdef CONFIG_RTW_DACS
	{"dacs",				BYTE_T,		0,													_MIB_OFFSET(dacs),							_SIZE(registrypriv.wifi_mib.dacs),				0,					NULL},
#endif
#ifdef CONFIG_RTW_PACS
	{"pacs",				BYTE_T, 	0,													_MIB_OFFSET(pacs),							_SIZE(registrypriv.wifi_mib.pacs),				0,		NULL},
	{"pacs_period",			BYTE_T, 	0,													_MIB_OFFSET(pacs_period),						_SIZE(registrypriv.wifi_mib.pacs_period),			300,		NULL}, //unit:s
#endif
	{"amsdu_pps",			INT_T,		0,													_MIB_OFFSET(amsdu_pps),					_SIZE(registrypriv.wifi_mib.amsdu_pps),			0,		NULL},
#ifdef CONFIG_DYN_FW_LOAD
	{"fw_ext",				BYTE_T, 	0,													_MIB_OFFSET(fw_ext),						_SIZE(registrypriv.wifi_mib.fw_ext),			0,		NULL},
#endif
	{"sta_asoc_rssi_th",		BYTE_T, 	0,													_MIB_OFFSET(sta_asoc_rssi_th),				_SIZE(registrypriv.wifi_mib.sta_asoc_rssi_th),		0,		NULL},
	{"hiddenAP",			BYTE_T, 	0, 													_MIB_OFFSET(hiddenAP),				_SIZE(registrypriv.wifi_mib.hiddenAP), 			0, 		core_mib_hiddenAP},
	{"force_fw_tx",			BYTE_T, 	0,													_MIB_OFFSET(force_fw_tx),					_SIZE(registrypriv.wifi_mib.force_fw_tx),		0,		NULL},
	{"rssi_ru_dump",			BYTE_T, 	0,													_MIB_OFFSET(rssi_ru_dump),					_SIZE(registrypriv.wifi_mib.rssi_ru_dump),		0,		NULL},
#ifdef CONFIG_TX_MCAST2UNI
	{"mc2u_flood_ctrl",	BYTE_T,		0,													_MIB_OFFSET(mc2u_flood_ctrl),	_SIZE(registrypriv.wifi_mib.mc2u_flood_ctrl),	0,		NULL},
	{"mc2u_ipv6_logo",	BYTE_T,		0,													_MIB_OFFSET(mc2u_ipv6_logo),	_SIZE(registrypriv.wifi_mib.mc2u_ipv6_logo),	1,		NULL},
#endif
	{"pri_mapping_rule",	BYTE_T,		0,													_MIB_OFFSET(pri_mapping_rule),	_SIZE(registrypriv.wifi_mib.pri_mapping_rule),	DSCP_MODE,		NULL},
#ifdef CTC_QOS_DSCP
	{"ctc_dscp",				BYTE_T,		_OFFSET(registrypriv.ctc_dscp),			0,						_SIZE(registrypriv.ctc_dscp),					0,				NULL},
#endif
#ifdef CONFIG_RTW_MANUAL_EDCA
	{"manual_edca",			BYTE_T,		_OFFSET(registrypriv.manual_edca),					0,											_SIZE(registrypriv.manual_edca),				0,		core_mib_manual_edca},
	{"sta_bkq_acm", 		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[BK].acm), _SIZE(registrypriv.wifi_mib.sta_manual_edca[BK].acm), 0,		core_mib_manual_edca},
	{"sta_bkq_aifsn",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[BK].aifsn), _SIZE(registrypriv.wifi_mib.sta_manual_edca[BK].aifsn), 7,		core_mib_manual_edca},
	{"sta_bkq_cwmin",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[BK].ecw_min), _SIZE(registrypriv.wifi_mib.sta_manual_edca[BK].ecw_min), 4,		core_mib_manual_edca},
	{"sta_bkq_cwmax",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[BK].ecw_max), _SIZE(registrypriv.wifi_mib.sta_manual_edca[BK].ecw_max), 10,		core_mib_manual_edca},
	{"sta_bkq_txoplimit",	INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[BK].txop_limit), _SIZE(registrypriv.wifi_mib.sta_manual_edca[BK].txop_limit), 0,		core_mib_manual_edca},
	{"sta_beq_acm", 		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[BE].acm), _SIZE(registrypriv.wifi_mib.sta_manual_edca[BE].acm), 0,		core_mib_manual_edca},
	{"sta_beq_aifsn",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[BE].aifsn), _SIZE(registrypriv.wifi_mib.sta_manual_edca[BE].aifsn), 3,		core_mib_manual_edca},
	{"sta_beq_cwmin",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[BE].ecw_min), _SIZE(registrypriv.wifi_mib.sta_manual_edca[BE].ecw_min), 4,		core_mib_manual_edca},
	{"sta_beq_cwmax",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[BE].ecw_max), _SIZE(registrypriv.wifi_mib.sta_manual_edca[BE].ecw_max), 6,		core_mib_manual_edca},
	{"sta_beq_txoplimit",	INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[BE].txop_limit), _SIZE(registrypriv.wifi_mib.sta_manual_edca[BE].txop_limit), 0,		core_mib_manual_edca},
	{"sta_viq_acm", 		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[VI].acm), _SIZE(registrypriv.wifi_mib.sta_manual_edca[VI].acm), 0,		core_mib_manual_edca},
	{"sta_viq_aifsn",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[VI].aifsn), _SIZE(registrypriv.wifi_mib.sta_manual_edca[VI].aifsn), 2,		core_mib_manual_edca},
	{"sta_viq_cwmin",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[VI].ecw_min), _SIZE(registrypriv.wifi_mib.sta_manual_edca[VI].ecw_min), 3,		core_mib_manual_edca},
	{"sta_viq_cwmax",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[VI].ecw_max), _SIZE(registrypriv.wifi_mib.sta_manual_edca[VI].ecw_max), 4,		core_mib_manual_edca},
	{"sta_viq_txoplimit",	INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[VI].txop_limit), _SIZE(registrypriv.wifi_mib.sta_manual_edca[VI].txop_limit), 94,		core_mib_manual_edca},
	{"sta_voq_acm", 		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[VO].acm), _SIZE(registrypriv.wifi_mib.sta_manual_edca[VO].acm), 0,		core_mib_manual_edca},
	{"sta_voq_aifsn",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[VO].aifsn), _SIZE(registrypriv.wifi_mib.sta_manual_edca[VO].aifsn), 2,		core_mib_manual_edca},
	{"sta_voq_cwmin",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[VO].ecw_min), _SIZE(registrypriv.wifi_mib.sta_manual_edca[VO].ecw_min), 2,		core_mib_manual_edca},
	{"sta_voq_cwmax",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[VO].ecw_max), _SIZE(registrypriv.wifi_mib.sta_manual_edca[VO].ecw_max), 3,		core_mib_manual_edca},
	{"sta_voq_txoplimit",	INT_T,		0,													_OFFSET(registrypriv.wifi_mib.sta_manual_edca[VO].txop_limit), _SIZE(registrypriv.wifi_mib.sta_manual_edca[VO].txop_limit), 47,		core_mib_manual_edca},
	{"ap_bkq_aifsn",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[BK].aifsn),		_SIZE(registrypriv.wifi_mib.ap_manual_edca[BK].aifsn),			7,		core_mib_manual_edca},
	{"ap_bkq_cwmin",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[BK].ecw_min),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[BK].ecw_min),		4,		core_mib_manual_edca},
	{"ap_bkq_cwmax",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[BK].ecw_max),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[BK].ecw_max),		10,		core_mib_manual_edca},
	{"ap_bkq_txoplimit",	INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[BK].txop_limit),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[BK].txop_limit),		0,		core_mib_manual_edca},
	{"ap_beq_aifsn",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[BE].aifsn),		_SIZE(registrypriv.wifi_mib.ap_manual_edca[BE].aifsn),			3,		core_mib_manual_edca},
	{"ap_beq_cwmin",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[BE].ecw_min),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[BE].ecw_min),		4,		core_mib_manual_edca},
	{"ap_beq_cwmax",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[BE].ecw_max),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[BE].ecw_max),		6,		core_mib_manual_edca},
	{"ap_beq_txoplimit",	INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[BE].txop_limit),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[BE].txop_limit),		0,		core_mib_manual_edca},
	{"ap_viq_aifsn",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[VI].aifsn),		_SIZE(registrypriv.wifi_mib.ap_manual_edca[VI].aifsn),			1,		core_mib_manual_edca},
	{"ap_viq_cwmin",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[VI].ecw_min),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[VI].ecw_min),		3,		core_mib_manual_edca},
	{"ap_viq_cwmax",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[VI].ecw_max),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[VI].ecw_max),		4,		core_mib_manual_edca},
	{"ap_viq_txoplimit",	INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[VI].txop_limit),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[VI].txop_limit),		94,		core_mib_manual_edca},
	{"ap_voq_aifsn",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[VO].aifsn),		_SIZE(registrypriv.wifi_mib.ap_manual_edca[VO].aifsn),			1,		core_mib_manual_edca},
	{"ap_voq_cwmin",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[VO].ecw_min),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[VO].ecw_min),		2,		core_mib_manual_edca},
	{"ap_voq_cwmax",		INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[VO].ecw_max),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[VO].ecw_max),		3,		core_mib_manual_edca},
	{"ap_voq_txoplimit",	INT_T,		0,													_OFFSET(registrypriv.wifi_mib.ap_manual_edca[VO].txop_limit),	_SIZE(registrypriv.wifi_mib.ap_manual_edca[VO].txop_limit),		47,		core_mib_manual_edca},
#endif
#ifdef CONFIG_ONE_TXQ
	{"txq_limit",			INT_T,		0,		_MIB_OFFSET(txq_limit), 					_SIZE(registrypriv.wifi_mib.txq_limit), 		2048,	NULL},
#endif
	{"qos_enable",			INT_T,		0,													_MIB_OFFSET(qos_enable),					_SIZE(registrypriv.wifi_mib.qos_enable),		1,		NULL},
#ifdef CONFIG_FORCE_QOS_SUPPORT
	{"force_qos",			INT_T,		0,													_MIB_OFFSET(force_qos),						_SIZE(registrypriv.wifi_mib.force_qos),		0,		NULL},
#endif
#if defined(WIFI6_THER_CTRL)
	{"dbg", 				BYTE_T, 	0,													_MIB_OFFSET(dbg),							_SIZE(registrypriv.wifi_mib.dbg),				0,		NULL},
	{"ther_dm", 			BYTE_T, 	0,													_MIB_OFFSET(ther_dm),						_SIZE(registrypriv.wifi_mib.ther_dm),			3,		NULL},

	{"del_ther",			BYTE_T, 	0,													_MIB_OFFSET(del_ther),						_SIZE(registrypriv.wifi_mib.del_ther),			0,		NULL},
	{"ther_hi",				BYTE_T, 	0,													_MIB_OFFSET(ther_hi),						_SIZE(registrypriv.wifi_mib.ther_hi),			0,		NULL},
	{"ther_low",			BYTE_T, 	0,													_MIB_OFFSET(ther_low),						_SIZE(registrypriv.wifi_mib.ther_low),			0,		NULL},
	{"ther_max",			BYTE_T, 	0,													_MIB_OFFSET(ther_max),						_SIZE(registrypriv.wifi_mib.ther_max),			0,		NULL},
#endif /* WIFI6_THER_CTRL */
#ifdef RTW_WKARD_BCNINT_DBG
	{"bcnint",				INT_T,		0,													_MIB_OFFSET(bcnint),						_SIZE(registrypriv.wifi_mib.bcnint),			0,	core_mib_bcnint},
#endif
#ifdef RTW_WKARD_CUSTOM_PWRLMT_EN
	{"txpwr_lmt_index",		BYTE_T,		0,													_MIB_OFFSET(txpwr_lmt_index),				_SIZE(registrypriv.wifi_mib.txpwr_lmt_index),		0,	core_mib_set_txpwr_lmt_index},
#endif
#ifdef CONFIG_BW160M_EXTREME_THROUGHPUT_RX
	{"tcpack_acc",			INT_T,		0,													_MIB_OFFSET(tcpack_acc),					_SIZE(registrypriv.wifi_mib.tcpack_acc),		1,		NULL},
	{"tcpack_hithd",		INT_T,		0,													_MIB_OFFSET(tcpack_hithd),					_SIZE(registrypriv.wifi_mib.tcpack_hithd),		1500,	NULL},
	{"tcpack_lothd",		INT_T,		0,													_MIB_OFFSET(tcpack_lothd),					_SIZE(registrypriv.wifi_mib.tcpack_lothd),		1200,	NULL},
#endif
	{"kick_icverr",		INT_T, 	0,													_MIB_OFFSET(kick_icverr),					_SIZE(registrypriv.wifi_mib.kick_icverr),			64,		NULL},
	{"atf",				BYTE_T,		0,		_MIB_OFFSET(atf),	_SIZE(registrypriv.wifi_mib.atf),	0,	core_mib_atf},
	{"dhcp_bcst_disable",	INT_T,		_OFFSET(ethBrExtInfo.dhcp_bcst_disable),			0, 											_SIZE(ethBrExtInfo.dhcp_bcst_disable), 			0,		NULL},
	{"fast_leave_thr",				BYTE_T,		0,		_MIB_OFFSET(fast_leave_thr),	_SIZE(registrypriv.wifi_mib.fast_leave_thr),	10,	core_mib_fast_leave},
	{"inactive_timeout",	INT_T,		0,		_MIB_OFFSET(inactive_timeout),	_SIZE(registrypriv.wifi_mib.inactive_timeout),	120,	core_mib_inactive_timeout},
	{"trx_path",		BYTE_T, 	0,													_MIB_OFFSET(trx_path),					_SIZE(registrypriv.wifi_mib.trx_path),			0,		core_mib_trx_path},
#ifdef CONFIG_ADPTVTY_CONTROL
	{"adptvty_en",			BYTE_T,		0, _MIB_OFFSET(adptvty_en),			_SIZE(registrypriv.wifi_mib.adptvty_en),		1,		NULL},
	{"adptvty_try",			BYTE_T,		0, _MIB_OFFSET(adptvty_try),		_SIZE(registrypriv.wifi_mib.adptvty_try),		0,		NULL},
	{"adptvty_th_t",		INT_T,		0, _MIB_OFFSET(adptvty_th_t),		_SIZE(registrypriv.wifi_mib.adptvty_th_t),		3000,	NULL},
	{"adptvty_th_u",		INT_T,		0, _MIB_OFFSET(adptvty_th_u),		_SIZE(registrypriv.wifi_mib.adptvty_th_u),		3000,	NULL},
	{"adptvty_ratio_u",		BYTE_T,		0, _MIB_OFFSET(adptvty_ratio_u),	_SIZE(registrypriv.wifi_mib.adptvty_ratio_u),	95,		NULL},
#endif /* CONFIG_ADPTVTY_CONTROL */
	{"dig_opmode",			BYTE_T,		0,													_MIB_OFFSET(dig_opmode),			_SIZE(registrypriv.wifi_mib.dig_opmode),		0,	core_mib_set_dig_opmode},
};

#if defined(WFO_VIRT_MODULE)
unsigned int mib_table_size(void)
{
	return sizeof(mib_table)/sizeof(struct iwpriv_arg);
}
#endif /* WFO_VIRT_MODULE */

static struct iwpriv_arg *get_tbl_entry(char *pstr)
{
	int i=0;
	int arg_num = sizeof(mib_table)/sizeof(struct iwpriv_arg);
	volatile char name[128];

	while (*pstr && *pstr != '=') {
		if (i >= sizeof(name)-1)
			return NULL;
		name[i++] = *pstr++;
	}
	name[i] = '\0';

	for (i=0; i<arg_num; i++) {
		if (!strcmp((char *)name, mib_table[i].name)) {
			return &mib_table[i];
		}
	}
	return NULL;
}

/******************************the definition of rate in flash**************************************************

	BIT0 ~ BIT11		(CCK/OFDM rate) 1M 2M 5.5M 11M 6M 9M 12M 18M 24M 36M 48M 54M
	BIT12 ~ BIT27		(11N rate)MCS0~MSC15 //supported now
	BIT28+n(n<=15)		(11N rate)MCS16~MSC31 //NOT supported temporarily
	BIT31+n(n<=19)		(11AC rate)NSS1MCS0~NSS1MCS9 NSS2MCS0~NSS2MCS9 //supported now
	BIT31+n(19<n<=39)	(11AC rate)NSS3MCS0~NSS3MCS9 NSS4MCS0~NSS4MCS9//NOT supported temporarily
	BIT31+n(39<n<=63)	(11AX rate)HE_NSS1MCS0~HE_NSS1MCS11 HE_NSS2MCS0~HE_NSS2MCS11 //supported now
	BIT31+n(63<n<=87)	(11AX rate)HE_NSS3MCS0~HE_NSS3MCS11 HE_NSS2MCS4~HE_NSS4MCS11 //NOT supported temporarily

*****************************************************************************************************************/
int datarate_check(unsigned int rate)
{
	int i=0, flag=0;
	if(((rate & BIT31) && ((rate-BIT31) <= 87)) || ((rate & BIT28) && ((rate-BIT28) <= 15))) //NSS & HE-NSS rate
		flag = 1;
	else if(rate<=BIT27)
	{
		for(i=0; i<=27; i++)
		{
			if(!(rate^(1<<i))) {
				flag = 1;
				break;
			}
		}
	}

	if(flag == 1)
		return flag;
	else
		return 0;
}

/*********************************CALL BACK FUNCTION*********************************/
void core_mib_version(_adapter *padapter, char *extra, u32 oper)
{
	if(RTW_MIB_SET == oper){
	}
	else if(RTW_MIB_GET == oper) {
		char version[6] = {0};
		snprintf(version, sizeof(version), "%s", RTW_WIFI_MIB_VERSION);
		memcpy(extra, (unsigned char *)version, sizeof(version));
	}
}

void core_mib_size(_adapter *padapter, char *extra, u32 oper) {
	if(RTW_MIB_SET == oper){
	}
	else if(RTW_MIB_GET == oper) {
		u32 size=0;
		size = sizeof(struct wifi_mib_priv);
		memcpy(extra, (unsigned char *)(&size), sizeof(u32));
	}
}
#ifdef CONFIG_RTW_A4_STA
void core_mib_a4_enable(_adapter *padapter, char *extra, u32 oper)
{
	struct wifi_mib_priv *mib = &padapter->registrypriv.wifi_mib;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	DBGP("oper=%d value=%d \n", oper, mib->a4_enable);

	if (RTW_MIB_SET == oper) {
		padapter->a4_enable = mib->a4_enable;
	} else if (RTW_MIB_GET == oper) {
		RTW_INFO("%s(%d), a4_enable mib get here\n", __FUNCTION__, __LINE__);
		if (mib->a4_enable != padapter->a4_enable)
			DBGP("mib = %d, apply = %d \n",
				mib->a4_enable, padapter->a4_enable);
	}
}
#endif

#ifdef CONFIG_80211AX_HE
#ifdef CONFIG_RTW_TWT
void core_mib_twt_enable(_adapter *padapter, char *extra, u32 oper)
{
	if(RTW_MIB_SET == oper) {
		u8 fw_tx = padapter->registrypriv.twt_enable?1:0;
		RTW_INFO("twt_enable = %d, set fw_tx = %d\n", padapter->registrypriv.twt_enable, fw_tx);
		#ifndef CONFIG_RTW_LINK_PHL_MASTER
		// 540ddedd3bd68ff78b8ed3be09be754a44f232f4 ystang
		if(padapter->registrypriv.fw_tx)
			rtw_phl_tx_mode_sel(padapter->dvobj->phl, fw_tx, padapter->registrypriv.fw_tx);
		else
			rtw_phl_tx_mode_sel(padapter->dvobj->phl, fw_tx, 0x1);
		#endif /* CONFIG_RTW_LINK_PHL_MASTER */

		update_beacon(padapter, _HE_CAPABILITY_IE_, NULL, _TRUE, 0);
	}
	else if (RTW_MIB_GET == oper) {
	}
}
#endif

void core_mib_fw_tx(_adapter *padapter, char *extra, u32 oper)
{
	if(RTW_MIB_SET == oper) {
		u8 fw_tx = padapter->registrypriv.fw_tx?1:0;
		RTW_INFO("set fw_tx = %d, bitmap = %x\n", fw_tx, padapter->registrypriv.fw_tx);
		#ifndef CONFIG_RTW_LINK_PHL_MASTER
		// 540ddedd3bd68ff78b8ed3be09be754a44f232f4 ystang
		rtw_phl_tx_mode_sel(padapter->dvobj->phl, fw_tx, padapter->registrypriv.fw_tx);
		#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	}
	else if (RTW_MIB_GET == oper) {
	}
}
#endif

#ifdef CONFIG_RTW_MULTI_AP
void core_mib_multiap_monitor_mode_disable(_adapter *padapter, char *extra, u32 oper)
{
	struct wifi_mib_priv *mib = &padapter->registrypriv.wifi_mib;

	DBGP("oper=%d value=%d \n", oper, mib->multiap_monitor_mode_disable);

	if (RTW_MIB_SET == oper) {

	} else if (RTW_MIB_GET == oper) {

	}
}

void core_mib_multiap_change_channel(_adapter *padapter, char *extra, u32 oper)
{
	struct wifi_mib_priv *mib = &padapter->registrypriv.wifi_mib;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	DBGP("oper=%d value=%d \n", oper, mib->multiap_change_channel);

	if (RTW_MIB_SET == oper) {
		int i;
		u16 ifbmp = 0;
		s16 ch = mib->set_channel;
		u8  bw = mib->set_bwmode;
		u8  offset = mib->set_ch_offset;

		DBGP("ch=%u bw=%u offset=%u\n", ch, bw, offset);

		if(ch == 0)
		{
			DBGP("not change channel due to set channel to 0\n");
			return;
		}

		if (ch == padapter->mlmeextpriv.cur_channel
			&& bw == padapter->mlmeextpriv.cur_bwmode
			&& offset == padapter->mlmeextpriv.cur_ch_offset)
			return;

		ifbmp &= (1 << dvobj->iface_nums) - 1;
		for (i = 0; i < dvobj->iface_nums; i++) {
			if (!dvobj->padapters[i])
				continue;

			if (CHK_MLME_STATE(dvobj->padapters[i], WIFI_AP_STATE) && dvobj->padapters[i]->netif_up)
				ifbmp |= BIT(i);
		}

		if (ifbmp) {
			if(bw > REQ_BW_NONE && bw < CHANNEL_WIDTH_MAX){
				rtw_change_bss_chbw_cmd(padapter, RTW_CMDF_WAIT_ACK, ifbmp, 0, ch, bw, offset);
			}
		}
	}
	else if (RTW_MIB_GET == oper) {
		/* no need to do */
	}
}

void core_mib_multiap_ext_cmd(_adapter *padapter, char *extra, u32 oper)
{
	struct wifi_mib_priv *mib = &padapter->registrypriv.wifi_mib;

	DBGP("oper=%d value=%d \n", oper, mib->multiap_ext_cmd);

	if (RTW_MIB_SET == oper) {
		DBGP("set rtw_map_user_pid from %d to 0\n", rtw_map_user_pid);
		rtw_map_user_pid = 0;
	} else if (RTW_MIB_GET == oper) {

	}
}

void core_mib_multiap_bss_type(_adapter *padapter, char *extra, u32 oper)
{
	struct wifi_mib_priv *mib = &padapter->registrypriv.wifi_mib;
	unsigned char multiap_bss_type = 0;

	RTW_INFO("oper=%d value=%d \n", oper, mib->multiap_bss_type);

	if (RTW_MIB_SET == oper) {
		if (mib->multiap_bss_type & MULTI_AP_FRONTHAUL_BSS) {
			padapter->multi_ap_mode = MAP_MODE_FRONT_AP;
			if (mib->multiap_bss_type & MULTI_AP_BACKHAUL_BSS)
				padapter->multi_ap_mode = MAP_MODE_FRONT_BACKHAUL_AP;
		} else if (mib->multiap_bss_type & MULTI_AP_BACKHAUL_BSS) {
			padapter->multi_ap_mode = MAP_MODE_BACKHAL_AP;
		} else if (mib->multiap_bss_type == MULTI_AP_BACKHAUL_STA) {
			padapter->multi_ap_mode = MAP_MODE_BACKHAL_STA;
		} else {
			padapter->multi_ap_mode = MAP_MODE_NONE;
		}
		DBGP("%d %d\n", mib->multiap_bss_type, padapter->multi_ap_mode);
	} else if (RTW_MIB_GET == oper) {
		RTW_INFO("%s(%d), multiap_bss_type mib get here\n", __FUNCTION__, __LINE__);
		if (padapter->multi_ap_mode == MAP_MODE_FRONT_AP)
			multiap_bss_type = MULTI_AP_FRONTHAUL_BSS;
		else if (padapter->multi_ap_mode == MAP_MODE_BACKHAL_AP)
			multiap_bss_type = MULTI_AP_BACKHAUL_BSS;
		else if (padapter->multi_ap_mode == MAP_MODE_BACKHAL_STA)
			multiap_bss_type = MULTI_AP_BACKHAUL_STA;
		else if (padapter->multi_ap_mode == MAP_MODE_FRONT_BACKHAUL_AP)
			multiap_bss_type = MULTI_AP_BACKHAUL_BSS | MULTI_AP_FRONTHAUL_BSS;

	#if defined(CPTCFG_WFO_VIRT_MODULE)
		if (is_g6_adapter(padapter))
	#endif /* CPTCFG_WFO_VIRT_MODULE */
		if (mib->multiap_bss_type != multiap_bss_type)
			RTW_WARN("my value = %d, expected value = %d \n",
				mib->multiap_bss_type, multiap_bss_type);
	}
}
#endif /* CONFIG_RTW_MULTI_AP */

void core_mib_ampdu(_adapter *padapter, char *extra, u32 oper)
{
	u8 ampdu = padapter->registrypriv.wifi_mib.ampdu;
	struct registry_priv *pregpriv = &padapter->registrypriv;
	if(RTW_MIB_SET == oper){
		pregpriv->ampdu_enable = ampdu;
		RTW_INFO("[%s][ampdu] ampdu_enable = 0x%x\n", __func__, pregpriv->ampdu_enable);
	}
	else if(RTW_MIB_GET == oper) {
	}
}

#ifdef WIFI6_THER_CTRL
void core_mib_func_off_thermal(_adapter *padapter, char *extra, u32 oper)
{
	u32 func_off = padapter->registrypriv.wifi_mib.func_off;
	u32 func_off_prev = padapter->registrypriv.wifi_mib.func_off_prev;
	u32 func_off_by_other = padapter->registrypriv.wifi_mib.func_off_by_other;

	if(RTW_MIB_SET == oper) {
		padapter->registrypriv.wifi_mib.func_off_by_thermal = func_off;

		if ((func_off == 0) && (func_off_by_other == 1)) {
			padapter->registrypriv.wifi_mib.func_off = func_off_prev;
			return;
		}

		if (func_off != func_off_prev)
			rtw_bcn_drop_switch(padapter, func_off);

		padapter->registrypriv.wifi_mib.func_off_prev = func_off;
	}
	else if(RTW_MIB_GET == oper) {
	}
}

void core_mib_func_off(_adapter *padapter, char *extra, u32 oper)
{
	u32 func_off = padapter->registrypriv.wifi_mib.func_off;
	u32 func_off_prev = padapter->registrypriv.wifi_mib.func_off_prev;
	u32 func_off_by_thermal = padapter->registrypriv.wifi_mib.func_off_by_thermal;

	if(RTW_MIB_SET == oper) {
		padapter->registrypriv.wifi_mib.func_off_by_other = func_off;

		if ((func_off == 0) && (func_off_by_thermal == 1)) {
			padapter->registrypriv.wifi_mib.func_off = func_off_prev;
			return;
		}

		if (func_off != func_off_prev)
			rtw_bcn_drop_switch(padapter, func_off);

		padapter->registrypriv.wifi_mib.func_off_prev = func_off;
		if(MLME_IS_AP(padapter) && (func_off == 1))
		{
			if(check_fwstate(&(padapter->mlmepriv), WIFI_UNDER_WPS))
				_clr_fwstate_(&(padapter->mlmepriv), WIFI_UNDER_WPS);
		}
	}
	else if(RTW_MIB_GET == oper) {
	}
}
#else /* WIFI6_THER_CTRL */
void core_mib_func_off(_adapter *padapter, char *extra, u32 oper)
{
	u32 func_off = padapter->registrypriv.wifi_mib.func_off;
	u32 func_off_prev = padapter->registrypriv.wifi_mib.func_off_prev;

	if(RTW_MIB_SET == oper) {
		if (func_off != func_off_prev)
			rtw_bcn_drop_switch(padapter, func_off);

		padapter->registrypriv.wifi_mib.func_off_prev = func_off;
		if(MLME_IS_AP(padapter) && (func_off == 1))
		{
			if(check_fwstate(&(padapter->mlmepriv), WIFI_UNDER_WPS))
				_clr_fwstate_(&(padapter->mlmepriv), WIFI_UNDER_WPS);
		}
	}
	else if(RTW_MIB_GET == oper) {
	}
}
#endif /* WIFI6_THER_CTRL */

void core_mib_auto_rate(_adapter *padapter, char *extra, u32 oper)
{
	u8 autorate = padapter->registrypriv.wifi_mib.autorate;
	if(RTW_MIB_SET == oper) {
		if (autorate)
			padapter->fix_rate = 0xFFFF;
		RTW_INFO("[%s][autorate] autorate = %d, fix_rate = 0x%x\n", __func__, autorate, padapter->fix_rate);
	}
	else if(RTW_MIB_GET == oper) {
	}
}

void core_mib_fix_rate(_adapter *padapter, char *extra, u32 oper)
{
	int j = 0, k = 0;
	u32 fixrate = padapter->registrypriv.wifi_mib.fixrate;
	if(RTW_MIB_SET == oper) {
		if(fixrate)
		{
			if(datarate_check(fixrate)){
				padapter->registrypriv.wifi_mib.autorate = 0;
				padapter->fix_rate = 0;
				if(((fixrate>>31)&1) == 1)
				{
					if(fixrate >= (1<<31)+40) //HE_NSS rate
					{
						if(fixrate <= (1<<31)+51) {
							padapter->fix_rate |= 0x180;
							padapter->fix_rate += ((fixrate&0xff)%40);
						}
						else {
							padapter->fix_rate |= 0x190;
							padapter->fix_rate += (((fixrate&0xff)%50)-2);
						}
					}
					else //NSS rate
					{
						padapter->fix_rate |= 0x100;
						padapter->fix_rate += ((fixrate&0xff)/10)*16+((fixrate&0xff)%10);
					}
				}
				else if(((fixrate>>28)&1) == 1)
				{
					//MCS16~MCS31
					padapter->fix_rate |= 0x80;
					padapter->fix_rate += (fixrate&0x1f);
				}
				else
				{
					// 0~11:CCK/OFDM rate;12~27:MCS0~MCS15
					for(j=0; j<28; j++)
					{
						if(fixrate & (1<<j))
						{
							if(j>11)
							{
								//MCS0~MCS15
								padapter->fix_rate |= 0x80;
								padapter->fix_rate += (j-12);
							}
							else
							{
								//CCK/OFDM rate
								padapter->fix_rate = j;
							}
						}
					}
				}
			}
			else
			{
				RTW_ERR("invalid fixrate value!\n");
				padapter->fix_rate = 0xFFFF;
				padapter->registrypriv.wifi_mib.autorate = 1;
			}
		}
		else
		{
			padapter->fix_rate = 0xFFFF;
			padapter->registrypriv.wifi_mib.autorate = 1;
		}
		RTW_INFO("[%s][fixrate] fix_rate = 0x%x\n", __func__, padapter->fix_rate);
	}
	else if(RTW_MIB_GET == oper) {
	}
}

void core_mib_deny_legacy(_adapter *padapter, char *extra, u32 oper)
{
	u32 deny_legacy = padapter->registrypriv.wifi_mib.deny_legacy;
	struct registry_priv *pregpriv = &padapter->registrypriv;
	if(RTW_MIB_SET == oper) {
		/**/
		if(deny_legacy == WLAN_WEB_11A)
			pregpriv->deny_legacy = WLAN_MD_11A;
		else if(deny_legacy == (WLAN_WEB_11B | WLAN_WEB_11G))
			pregpriv->deny_legacy = WLAN_MD_11BG;
		else if(deny_legacy == WLAN_WEB_11G)
			pregpriv->deny_legacy = WLAN_MD_11G;
		else if(deny_legacy == WLAN_WEB_11B)
			pregpriv->deny_legacy = WLAN_MD_11B;
		else if(deny_legacy == (WLAN_WEB_11A | WLAN_WEB_11N))
			pregpriv->deny_legacy = WLAN_MD_11AN;
		else if(deny_legacy == (WLAN_WEB_11A | WLAN_WEB_11N | WLAN_WEB_11AC))
			pregpriv->deny_legacy = WLAN_MD_11ANAC;
		else if(deny_legacy == (WLAN_WEB_11B | WLAN_WEB_11G | WLAN_WEB_11N))
			pregpriv->deny_legacy = WLAN_MD_11BGN;
		else
			pregpriv->deny_legacy = 0;

		RTW_INFO("[%s][deny_legacy] deny_legacy = 0x%x\n", __func__, deny_legacy);
	}
	else if(RTW_MIB_GET == oper) {
		deny_legacy = 0;
		if(pregpriv->deny_legacy == WLAN_MD_11A)
			deny_legacy = WLAN_WEB_11A;
		else if(pregpriv->deny_legacy == WLAN_MD_11BG)
			deny_legacy = WLAN_WEB_11B | WLAN_WEB_11G;
		else if(pregpriv->deny_legacy == WLAN_MD_11G)
			deny_legacy = WLAN_WEB_11G;
		else if(pregpriv->deny_legacy == WLAN_MD_11B)
			deny_legacy = WLAN_WEB_11B;
		else if(pregpriv->deny_legacy == WLAN_MD_11AN)
			deny_legacy = WLAN_WEB_11A | WLAN_WEB_11N;
		else if(pregpriv->deny_legacy == WLAN_MD_11ANAC)
			deny_legacy = (WLAN_WEB_11A | WLAN_WEB_11N | WLAN_WEB_11AC);
		else if(pregpriv->deny_legacy == WLAN_MD_11BGN)
			deny_legacy = (WLAN_WEB_11B | WLAN_WEB_11G | WLAN_WEB_11N);

		RTW_INFO("[%s][deny_legacy] deny_legacy = 0x%x\n", __func__, deny_legacy);
		memcpy(extra, (unsigned char *)(&deny_legacy), sizeof(u32));
	}
}


int core_mib_should_enable_txsc(_adapter *padapter)
{
	u8 amsdu = padapter->registrypriv.wifi_mib.amsdu;
	u8 a4_enable = padapter->registrypriv.wifi_mib.a4_enable;
	/*now txsc support ap/repeater/a4 mode */
	return 1;
}

void core_mib_amsdu(_adapter *padapter, char *extra, u32 oper)
{
	u8 amsdu = padapter->registrypriv.wifi_mib.amsdu;

	if(RTW_MIB_SET == oper) {
#ifdef CONFIG_CORE_TXSC
		if (!padapter->xmitpriv.txsc_enable && amsdu && core_mib_should_enable_txsc(padapter))
			padapter->xmitpriv.txsc_enable = 1;

		padapter->xmitpriv.txsc_amsdu_enable = amsdu;

		/* reset txsc */
		txsc_clear(padapter, 1);

		RTW_INFO("[%s][amsdu] txsc_amsdu_enable = 0x%x\n", __func__, padapter->xmitpriv.txsc_amsdu_enable);
#endif
	}
	else if(RTW_MIB_GET == oper) {
	}
}

void core_mib_txbf_auto_snd(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	if(RTW_MIB_SET == oper) {
		dvobj->phl_com->txbf_auto_snd = padapter->registrypriv.wifi_mib.txbf_auto_snd;

		RTW_INFO("[%s][txbf] phl_com->txbf_auto_snd = 0x%x\n", __func__, dvobj->phl_com->txbf_auto_snd);
	}
	else if(RTW_MIB_GET == oper) {
	}
}


void core_mib_rssi_dump(_adapter *padapter, char *extra, u32 oper)
{
	u8 rssi_value = padapter->registrypriv.wifi_mib.rssi_dump;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	#ifdef CONFIG_WFA_OFDMA_Logo_Test
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	#endif

	if(RTW_MIB_SET == oper) {
		//padapter->sta_dump_to = rssi_value+rssi_value%2;
		if(padapter->sta_dump_to != 2 && rssi_value == 2){
			#ifdef CONFIG_WFA_OFDMA_Logo_Test
			if(padapter->ru_c2h_intvl == 0)
				rtw_core_mac_set_ru_fwc2h_en(padapter, 1, 1000);
			else
				rtw_core_mac_set_ru_fwc2h_en(padapter, 1, padapter->ru_c2h_intvl);

			if(ru_ctrl->ul_crc32){
				rtw_phl_write32(padapter->dvobj->phl, 0xce20, 0x3100c0e);
				padapter->sta_ru_dump_bitmap = padapter->sta_ru_dump_bitmap | RTW_STA_DUMP_RU_RX_CRC32_RATIO;
			}
			else
			#endif
			{
				rtw_phl_write32(padapter->dvobj->phl, 0xce20, 0x310040e);
				padapter->sta_ru_dump_bitmap = padapter->sta_ru_dump_bitmap & ~RTW_STA_DUMP_RU_RX_CRC32_RATIO;
			}
		}
		else if(padapter->sta_dump_to == 2 && rssi_value != 2){
			#ifdef CONFIG_WFA_OFDMA_Logo_Test
			rtw_core_mac_set_ru_fwc2h_en(padapter, 0, 0);

			if(ru_ctrl->ul_crc32) {
				rtw_phl_write32(padapter->dvobj->phl, 0xce20, 0x310040e);
				padapter->sta_ru_dump_bitmap = padapter->sta_ru_dump_bitmap & ~RTW_STA_DUMP_RU_RX_CRC32_RATIO;
			}
			#endif
		}

		padapter->sta_dump_to = rssi_value;

#if 0//def PLATFORM_ECOS
		if (padapter->sta_dump_to > 0) {
			log_cfg.pass_log.enable_linux_output = 1;
			log_cfg.pass_log.rssi_enable = 1;
		}
		else {
			if (log_cfg.pass_log.log_level_enable == 0)
				log_cfg.pass_log.enable_linux_output = 0;
			log_cfg.pass_log.rssi_enable = 0;
		}
#endif

		//RTW_INFO("[%s][rssi_dump] sta_dump_to = %d, sta_dump_bitmap = 0x%x\n", __func__, padapter->sta_dump_to, padapter->sta_dump_bitmap);
		RTW_INFO("[%s][rssi_dump] sta_dump_to = %d, sta_dump_bitmap = 0x%x, sta_ru_dump_bitmap = 0x%x\n", __func__, padapter->sta_dump_to, padapter->sta_dump_bitmap, padapter->sta_ru_dump_bitmap);
	}
	else if(RTW_MIB_GET == oper) {
	}
}

void core_mib_edcca_mode(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	if(RTW_MIB_SET == oper) {
		dvobj->phl_com->edcca_mode = padapter->registrypriv.wifi_mib.edcca_mode;

		RTW_INFO("[%s][edcca_mode] phl_com->edcca_mode = 0x%x\n", __func__, dvobj->phl_com->edcca_mode);
	}
	else if(RTW_MIB_GET == oper) {
	}
}

void core_mib_bfrp_mode(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	if(RTW_MIB_SET == oper) {
		dvobj->phl_com->bfrp_mode = padapter->registrypriv.wifi_mib.bfrp_mode;

		RTW_INFO("[%s][edcca_mode] phl_com->bfrp_mode = 0x%x\n", __func__, dvobj->phl_com->bfrp_mode);
	}
	else if(RTW_MIB_GET == oper) {
	}
}

void core_mib_su_trig_enable(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	if(RTW_MIB_SET == oper) {
		dvobj->phl_com->su_trig_enable = padapter->registrypriv.wifi_mib.su_trig_enable;

		RTW_INFO("[%s][edcca_mode] phl_com->su_trig_enable = 0x%x\n", __func__, dvobj->phl_com->su_trig_enable);
	}
	else if(RTW_MIB_GET == oper) {
	}
}


void core_mib_en_timer_pfd(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	if(RTW_MIB_SET == oper) {
		#ifndef CONFIG_RTW_LINK_PHL_MASTER
		// a02a82e1ae4ae8ffacb9401cb7d1b5053c3f95a7 charliecheng
		dvobj->phl_com->en_timer_pfd = padapter->registrypriv.wifi_mib.en_timer_pfd;

		RTW_INFO("[%s] phl_com->en_timer_pfd = 0x%x\n", __func__, dvobj->phl_com->en_timer_pfd);
		#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	}
	else if(RTW_MIB_GET == oper) {
	}
}
void core_mib_en_txrpt_pfd(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	if(RTW_MIB_SET == oper) {
		#ifndef CONFIG_RTW_LINK_PHL_MASTER
		// a02a82e1ae4ae8ffacb9401cb7d1b5053c3f95a7 charliecheng
		dvobj->phl_com->en_txrpt_pfd = padapter->registrypriv.wifi_mib.en_txrpt_pfd;

		RTW_INFO("[%s]phl_com->en_txrpt_pfd = 0x%x\n", __func__, dvobj->phl_com->en_txrpt_pfd);
		#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	}
	else if(RTW_MIB_GET == oper) {
	}
}
void core_mib_set_timer_period(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	if(RTW_MIB_SET == oper) {
		#ifndef CONFIG_RTW_LINK_PHL_MASTER
		// a02a82e1ae4ae8ffacb9401cb7d1b5053c3f95a7 charliecheng
		dvobj->phl_com->set_timer_period = padapter->registrypriv.wifi_mib.set_timer_period;

		RTW_INFO("[%s] phl_com->set_timer_period = 0x%x\n", __func__, dvobj->phl_com->set_timer_period);
		#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	}
	else if(RTW_MIB_GET == oper) {
	}
}
void core_mib_timer_period(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	if(RTW_MIB_SET == oper) {
		#ifndef CONFIG_RTW_LINK_PHL_MASTER
		// a02a82e1ae4ae8ffacb9401cb7d1b5053c3f95a7 charliecheng
		dvobj->phl_com->timer_period = padapter->registrypriv.wifi_mib.timer_period;

		RTW_INFO("[%s] phl_com->timer_period = 0x%x\n", __func__, dvobj->phl_com->timer_period);
		#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	}
	else if(RTW_MIB_GET == oper) {
	}
}

void core_mib_timer_clr_period(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	if(RTW_MIB_SET == oper) {
		#ifndef CONFIG_RTW_LINK_PHL_MASTER
		// a02a82e1ae4ae8ffacb9401cb7d1b5053c3f95a7 charliecheng
		dvobj->phl_com->timer_clr_period = padapter->registrypriv.wifi_mib.timer_clr_period;

		RTW_INFO("[%s] phl_com->timer_period = 0x%x\n", __func__, dvobj->phl_com->timer_clr_period);
		#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	}
	else if(RTW_MIB_GET == oper) {
	}
}



void core_mib_txforce(_adapter *padapter, char *extra, u32 oper)
{
	u32 txforce = padapter->registrypriv.wifi_mib.txforce & 0xffff;
	u32 macid = padapter->registrypriv.wifi_mib.txforce >> 16;
	struct sta_info *psta;

	if(RTW_MIB_SET == oper) {
		if (macid == 0) {
			padapter->fix_rate = txforce;
		} else {
			psta = rtw_get_stainfo_by_macid(&padapter->stapriv, (u16)macid);
			if (psta) { psta->fixRate = txforce; }
			else { printk("sta not found!!\n"); }
		}
		//if (txforce == 0xffff)
			txsc_clear(padapter, 1);

		RTW_PRINT("[%s][txforce] macid:%d, fix_rate = 0x%x\n", __func__, macid, padapter->fix_rate);
	}
	else if(RTW_MIB_GET == oper) {
	}
}

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
void update_wirelessmode_cap(u8 band, _adapter *padapter)
{
	u8 *p;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX	*cur_network = &(pmlmeinfo->network);

	if(!is_primary_adapter(padapter))
	{
#ifdef CONFIG_80211AX_HE
		if(pmlmeinfo->HE_enable == _TRUE && (band & WLAN_MD_11AX)==0 )
		{
			pmlmeinfo->HE_enable = _FALSE;
			pmlmepriv->hepriv.he_option = _FALSE;
			rtw_remove_rsv_bcn_ie(padapter, cur_network, WLAN_EID_EXTENSION, WLAN_EID_EXTENSION_HE_CAPABILITY);
			rtw_remove_rsv_bcn_ie(padapter, cur_network, WLAN_EID_EXTENSION, WLAN_EID_EXTENSION_HE_OPERATION);
			rtw_remove_rsv_bcn_ie(padapter, cur_network, WLAN_EID_EXTENSION, WLAN_EID_EXTENSION_MU_EDCA);
		}
#endif /* CONFIG_80211AX_HE */
#ifdef CONFIG_80211AC_VHT
		if(pmlmeinfo->VHT_enable == _TRUE && (band & WLAN_MD_11AC)==0 )
		{
			pmlmeinfo->VHT_enable = _FALSE;
			pmlmepriv->vhtpriv.vht_option = _FALSE;
			rtw_remove_bcn_ie(padapter, cur_network, WLAN_EID_VHT_CAPABILITY);
			rtw_remove_bcn_ie(padapter, cur_network, WLAN_EID_VHT_OPERATION);
			rtw_remove_bcn_ie(padapter, cur_network, WLAN_EID_VHT_TRANSMIT_POWER_ENVELOPE);
			rtw_remove_bcn_ie(padapter, cur_network, WLAN_EID_CHANNEL_SWITCH_WRAPPER);
		}
#endif /* CONFIG_80211AC_VHT */
#ifdef CONFIG_80211N_HT
		if(pmlmeinfo->HT_enable == _TRUE && (band & WLAN_MD_11N)==0 )
		{
			pmlmepriv->htpriv.ht_option = _FALSE;
			pmlmeinfo->HT_info_enable = _FALSE;
			pmlmeinfo->HT_caps_enable = _FALSE;
			pmlmeinfo->HT_enable = _FALSE;
			rtw_remove_bcn_ie(padapter, cur_network, WLAN_EID_SECONDARY_CHANNEL_OFFSET);
			rtw_remove_bcn_ie(padapter, cur_network, WLAN_EID_HT_CAP);
			rtw_remove_bcn_ie(padapter, cur_network, WLAN_EID_HT_OPERATION);
		}
	}
#endif /* CONFIG_80211N_HT */
}
#endif

void core_mib_band(_adapter *padapter, char *extra, u32 oper)
{
	u8 band=0, band_new=0, tmp_value1=0, tmp_value=0;
	struct sta_info *sta = NULL;
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;
#ifdef CONFIG_RTW_AP_EXT_SUPPORT
	u8 rate_len, total_rate_len, remainder_rate_len;
#endif

	band = pmibpriv->band;
	if(RTW_MIB_SET == oper) {
		if(band & WLAN_WEB_11B)
			band_new |= WLAN_MD_11B;
		if(band & WLAN_WEB_11G)
			band_new |= WLAN_MD_11G;
		if(band & WLAN_WEB_11A)
			band_new |= WLAN_MD_11A;
		if(band & WLAN_WEB_11N)
			band_new |= WLAN_MD_11N;
		if(band & WLAN_WEB_11AC)
			band_new |= WLAN_MD_11AC;
		if(band & WLAN_WEB_11AX)
			band_new |= WLAN_MD_11AX;

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
		if(MLME_IS_AP(padapter) && !is_primary_adapter(padapter))
		{
			update_wirelessmode_cap(band_new, padapter);
			if(padapter->mlmeextpriv.cur_channel <= 14)
			{
				rtw_set_supported_rate(padapter->mlmeextpriv.mlmext_info.network.SupportedRates, band_new, padapter->mlmeextpriv.mlmext_info.network.Configuration.DSConfig);
				rtw_set_supported_rate(padapter->mlmepriv.cur_network.network.SupportedRates, band_new, padapter->mlmepriv.cur_network.network.Configuration.DSConfig);
			}

			update_wireless_mode(padapter);

			if(padapter->mlmeextpriv.cur_channel <= 14)
			{
				total_rate_len = rtw_get_rateset_len(padapter->mlmeextpriv.mlmext_info.network.SupportedRates);
				if (total_rate_len > 8) {
					rate_len = 8;
					remainder_rate_len = total_rate_len - 8;
					if(padapter->mlmeextpriv.mlmext_info.network.wpa3_h2e_only == 1)
					{
						if(total_rate_len < NDIS_802_11_LENGTH_RATES_EX)
						{
							padapter->mlmeextpriv.mlmext_info.network.SupportedRates[total_rate_len] = _EXT_SUPPORTEDRATES_H2E_;
							remainder_rate_len += 1;
						}
					}
				} else {
					rate_len = total_rate_len;
					remainder_rate_len = 0;
				}

				rtw_add_bcn_ie(padapter, &(padapter->mlmeextpriv.mlmext_info.network), _SUPPORTEDRATES_IE_, padapter->mlmeextpriv.mlmext_info.network.SupportedRates, rate_len);

				if (remainder_rate_len)
					rtw_add_bcn_ie(padapter, &(padapter->mlmeextpriv.mlmext_info.network), _EXT_SUPPORTEDRATES_IE_, (padapter->mlmeextpriv.mlmext_info.network.SupportedRates + 8), remainder_rate_len);
				else
					rtw_remove_bcn_ie(padapter, &(padapter->mlmeextpriv.mlmext_info.network), _EXT_SUPPORTEDRATES_IE_);

				padapter->mlmeextpriv.mlmext_info.network.Length = get_WLAN_BSSID_EX_sz(&(padapter->mlmeextpriv.mlmext_info.network));

				if(band_new == WLAN_MD_11B)
				{
					rtw_remove_bcn_ie(padapter, &(padapter->mlmeextpriv.mlmext_info.network), _ERPINFO_IE_);
				}
			}

			sta = rtw_get_stainfo(&padapter->stapriv, adapter_mac_addr(padapter));
			if (!sta) {
				const u8 *mac_addr = adapter_mac_addr(padapter);
				RTW_ERR(FUNC_ADPT_FMT" !sta for macaddr="MAC_FMT"\n",
						FUNC_ADPT_ARG(padapter), MAC_ARG(mac_addr));
				rtw_warn_on(1);
				return;
			}
			update_ap_info(padapter, sta);
			if(padapter->mlmeextpriv.cur_channel <= 14)
				send_beacon(padapter);
		}
#endif
		pmibpriv->band = band_new;
		RTW_INFO("[%s][band] band = 0x%x\n", __func__, pmibpriv->band);
	}
	else if(RTW_MIB_GET == oper) {
		pmibpriv = get_mibpriv(padapter);

		RTW_INFO("%s(%d), band mib get here\n", __FUNCTION__, __LINE__);
		tmp_value1 = pmibpriv->band;
		if(tmp_value1 & WLAN_MD_11B)
			tmp_value += WLAN_WEB_11B;
		if(tmp_value1 & WLAN_MD_11A)
			tmp_value += WLAN_WEB_11A;
		if(tmp_value1 & WLAN_MD_11G)
			tmp_value += WLAN_WEB_11G;
		if(tmp_value1 & WLAN_MD_11N)
			tmp_value += WLAN_WEB_11N;
		if(tmp_value1 & WLAN_MD_11AC)
			tmp_value += WLAN_WEB_11AC;
		if(tmp_value1 & WLAN_MD_11AX)
			tmp_value += WLAN_WEB_11AX;

		if(tmp_value)
			memcpy(extra, &tmp_value, 1);
	}
}

void core_mib_lifetime(_adapter *padapter, char *extra, u32 oper)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if(RTW_MIB_SET == oper) {
#ifdef CONFIG_LIFETIME_FEATURE
		u16 us256 = RTL_MS_TO_256US(padapter->registrypriv.wifi_mib.lifetime);

		if(rtw_is_adapter_up(padapter))
		{
			if (check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY))
				RTW_PRINT("%s(%d) device is under survey\n", __FUNCTION__, __LINE__);
			else {
				if (us256)
					rtw_phl_set_lifetime(GET_HAL_INFO(padapter->dvobj), 1, us256);
				else
					rtw_phl_set_lifetime(GET_HAL_INFO(padapter->dvobj), 0, 0);

				rtw_phl_get_lifetime(GET_HAL_INFO(padapter->dvobj));
			}
		}

#ifdef CONFIG_CORE_TXSC
		txsc_clear(padapter, 1);
#endif
#endif
	}
	else if(RTW_MIB_GET == oper) {
	}
}

void core_mib_power_percent(_adapter *padapter, char *extra, u32 oper)
{
#ifdef POWER_PERCENT_ADJUSTMENT
	u8 percent = padapter->registrypriv.wifi_mib.powerpercent;
	int diff_level = txpower_percent_to_level(percent);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	if(RTW_MIB_SET == oper) {
		if (percent == 0)
			return;
		if(rtw_is_adapter_up(padapter))
		{
			if (check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY))
				RTW_PRINT("%s(%d) device is under survey\n", __FUNCTION__, __LINE__);
			else
			{
				RTW_PRINT("%s(%d)set ref power, percent: %d, diff: %d\n", __FUNCTION__, __LINE__, percent, diff_level);
				rtw_phl_set_ref_power(GET_HAL_INFO(padapter->dvobj), 0, diff_level);
			}
		}
		else
			RTW_PRINT("%s(%d), interface is not running\n", __FUNCTION__, __LINE__);
	}
	else if(RTW_MIB_GET == oper) {
	}
#endif
}

void core_mib_power_ref(_adapter *padapter, char *extra, u32 oper)
{
#ifdef POWER_PERCENT_ADJUSTMENT
	int ref = padapter->registrypriv.wifi_mib.power_reference;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if(RTW_MIB_SET == oper) {
		if(rtw_is_adapter_up(padapter)) {
			if (check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY))
				RTW_PRINT("%s(%d) device is under survey\n", __FUNCTION__, __LINE__);
			else
			{
				rtw_phl_set_ref_power(GET_HAL_INFO(padapter->dvobj), 0, (ref * 2));
				RTW_PRINT("%s(%d)set ref power, txagc_ref: %d\n", __FUNCTION__, __LINE__, ref);
			}
		}
		else
			RTW_PRINT("%s(%d), interface is not running\n", __FUNCTION__, __LINE__);
	}
	else if(RTW_MIB_GET == oper) {
	}
#endif
}

void core_mib_aclmode(_adapter *padapter, char *extra, u32 oper)
{
	u32 aclmode = padapter->registrypriv.wifi_mib.aclmode;

	if(RTW_MIB_SET == oper){

		rtw_set_macaddr_acl(padapter, RTW_ACL_PERIOD_BSS, aclmode);
		RTW_INFO("[%s][aclmode] aclmode = %d\n", __func__, aclmode);

	}else if(RTW_MIB_GET == oper){
	}
}

void core_mib_aclnum(_adapter *padapter, char *extra, u32 oper)
{
	return;
}
#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
extern void rtw_core_set_gt3(_adapter *padapter, u8 enable, long timeout);
void core_mib_tx_mode(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = padapter->dvobj;
	_adapter *prim_adp = dvobj_get_primary_adapter(dvobj);

	if(RTW_MIB_SET == oper) {
		dvobj->tx_mode = padapter->registrypriv.tx_mode;
		RTW_PRINT("set tx_mode:%d\n",  padapter->registrypriv.tx_mode);

		//trigger timer
		prim_adp->hw_swq_cnt = (prim_adp->hw_swq_cnt + 1) % 8192;

		if ( 0 != prim_adp->hw_swq_timeout )
		   rtw_core_set_gt3(prim_adp, 1, prim_adp->hw_swq_timeout );
		else
		   rtw_core_set_gt3(prim_adp, 1, 10000 );
	}
	else if(RTW_MIB_GET == oper) {
	}
}
#endif

void core_mib_channel(_adapter *padapter, char *extra, u32 oper)
{
	unsigned char set_channel = padapter->registrypriv.wifi_mib.set_channel;
	unsigned char cur_channel = padapter->mlmeextpriv.cur_channel;

	if(RTW_MIB_SET == oper) {
		/* no need to transform */
	}
	else if(RTW_MIB_GET == oper) {
		RTW_INFO("%s(%d), channel mib get here\n", __FUNCTION__, __LINE__);
		memcpy(extra, &cur_channel, 1);
	}
}

void core_mib_use40M(_adapter *padapter, char *extra, u32 oper)
{
	unsigned char set_bwmode = padapter->registrypriv.wifi_mib.set_bwmode;
	unsigned char cur_bwmode = padapter->mlmeextpriv.cur_bwmode;

	if(RTW_MIB_SET == oper) {
		/* no need to transform */
	}
	else if(RTW_MIB_GET == oper) {
		RTW_INFO("%s(%d), channel mib get here\n", __FUNCTION__, __LINE__);
		memcpy(extra, &cur_bwmode, 1);
	}
}

void core_mib_2ndchoffset(_adapter *padapter, char *extra, u32 oper)
{
	unsigned char set_ch_offset = padapter->registrypriv.wifi_mib.set_ch_offset;
	unsigned char cur_ch_offset = padapter->mlmeextpriv.cur_ch_offset;
	unsigned char get_cur_ch_offset = 0;

	if(RTW_MIB_SET == oper) {
		/* need to transform */
		if(set_ch_offset == 2)
			padapter->registrypriv.wifi_mib.set_ch_offset = CHAN_OFFSET_UPPER;
		else if(set_ch_offset == 1)
			padapter->registrypriv.wifi_mib.set_ch_offset = CHAN_OFFSET_LOWER;
	}
	else if(RTW_MIB_GET == oper) {
		RTW_INFO("%s(%d), 2ndchoffset mib get here\n", __FUNCTION__, __LINE__);
		/* need to transform */
		if(cur_ch_offset == CHAN_OFFSET_UPPER) // 2nd channel above
			get_cur_ch_offset = 2;
		else if(cur_ch_offset == CHAN_OFFSET_LOWER) // 2nd channel below
			get_cur_ch_offset = 1;
		memcpy(extra, &get_cur_ch_offset, 1);
	}
}

void core_mib_encmode(_adapter *padapter, char *extra, u32 oper)
{
	u8 tmp_value=0;

	if(RTW_MIB_SET == oper){
	}
	else if(RTW_MIB_GET == oper) {
		struct security_priv *securitypriv = get_securitypriv(padapter);

		RTW_INFO("%s(%d), encmode mib get here\n", __FUNCTION__, __LINE__);
		if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter))
		{
#ifdef CONFIG_AP_MODE
			tmp_value = 0;
			if(securitypriv->dot11PrivacyAlgrthm == _NO_PRIVACY_)
				tmp_value = 0;//NONE
			else if(securitypriv->dot11PrivacyAlgrthm & _WEP40_)
				tmp_value = 1;//WEP
			else
			{
				if(securitypriv->wpa_psk & BIT(0))
					tmp_value += 2;//WPA
				if(securitypriv->wpa_psk & BIT(1))
				{
					if(securitypriv->akmp & WLAN_AKM_TYPE_PSK)
						tmp_value += 4;//WPA2

					if(securitypriv->akmp & WLAN_AKM_TYPE_SAE)
						tmp_value += 16;//WPA3
				}
			}
			memcpy(extra, &tmp_value, 1);
#endif
		}
		else if (MLME_IS_STA(padapter) || MLME_IS_MP(padapter))
		{
			tmp_value = 0;
			if(securitypriv->dot11PrivacyAlgrthm == _NO_PRIVACY_)
				tmp_value = 0;//NONE
			else if(securitypriv->dot11PrivacyAlgrthm & _WEP40_)
				tmp_value = 1;//WEP
			else
			{
				if(securitypriv->ndisauthtype == Ndis802_11AuthModeWPAPSK)
					tmp_value = 2;//WPA
				else if(securitypriv->ndisauthtype == Ndis802_11AuthModeWPA2PSK)
					tmp_value = 4;//WPA2
			}

			memcpy(extra, &tmp_value, 1);
		}
	}
}

void core_mib_psk_enable(_adapter *padapter, char *extra, u32 oper)
{
	int tmp_value=0;
	u8 index;
	if(RTW_MIB_SET == oper) {
	}
	else if(RTW_MIB_GET == oper) {
		RTW_INFO("%s(%d), psk_enable mib get here\n", __FUNCTION__, __LINE__);
		if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter))
		{
			tmp_value = 0;
			if(padapter->securitypriv.dot11PrivacyAlgrthm == _NO_PRIVACY_)
				tmp_value = 0;//NONE
			else if(padapter->securitypriv.dot11PrivacyAlgrthm & _WEP40_)
				tmp_value = 0;//WEP
			else
			{
				if(padapter->securitypriv.wpa_psk & BIT(0))
					tmp_value += 1;//WPA
				if(padapter->securitypriv.wpa_psk & BIT(1))
				{
					if(padapter->securitypriv.akmp & WLAN_AKM_TYPE_PSK)
						tmp_value += 2;//WPA2

					if(padapter->securitypriv.akmp & WLAN_AKM_TYPE_SAE)
						tmp_value += 8;//WPA3
				}
			}

			memcpy(extra, (unsigned char *)(&tmp_value), sizeof(int));
		}
		else if (MLME_IS_STA(padapter) || MLME_IS_MP(padapter))
		{
			tmp_value = 0;
			if(padapter->securitypriv.dot11PrivacyAlgrthm == _NO_PRIVACY_)
				tmp_value = 0;//NONE
			else if(padapter->securitypriv.dot11PrivacyAlgrthm & _WEP40_)
				tmp_value = 0;//WEP
			else
			{
				if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeWPAPSK)
					tmp_value = 1;//WPA
				else if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeWPA2PSK)
				{
					tmp_value = 2;//WPA2
					for (index = 0; index < NUM_PMKID_CACHE; index++)
					{
						if(padapter->securitypriv.PMKIDList[index].bUsed)
						{
							tmp_value = 8;//WPA3
							break;
						}
					}
				}
			}

			memcpy(extra, (unsigned char *)(&tmp_value), sizeof(int));
		}
	}

}

void core_mib_wpa_cipher(_adapter *padapter, char *extra, u32 oper)
{
	int tmp_value=0;
	if(RTW_MIB_SET == oper) {
	}
	else if(RTW_MIB_GET == oper) {
		RTW_INFO("%s(%d), wpa_cipher mib get here\n", __FUNCTION__, __LINE__);
		if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter))
		{
			tmp_value = 0;
			if(padapter->securitypriv.wpa_pairwise_cipher & WPA_CIPHER_TKIP)
				tmp_value |= BIT1;
			if(padapter->securitypriv.wpa_pairwise_cipher & WPA_CIPHER_CCMP)
				tmp_value |= BIT3;

			memcpy(extra, (unsigned char *)(&tmp_value), sizeof(int));

		}
		else if (MLME_IS_STA(padapter) || MLME_IS_MP(padapter))
		{
			tmp_value = 0;
			if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeWPAPSK)
			{
				if(padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_)
					tmp_value = BIT1;
				else if(padapter->securitypriv.dot11PrivacyAlgrthm == _AES_)
					tmp_value = BIT3;
			}

			memcpy(extra, (unsigned char *)(&tmp_value), sizeof(int));

		}
	}
}

void core_mib_wpa2_cipher(_adapter *padapter, char *extra, u32 oper)
{
	int tmp_value=0;
	if(RTW_MIB_SET == oper) {
	}
	else if(RTW_MIB_GET == oper) {
		RTW_INFO("%s(%d), wpa2_cipher mib get here\n", __FUNCTION__, __LINE__);
		if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter))
		{
			tmp_value = 0;
			if(padapter->securitypriv.wpa2_pairwise_cipher & WPA_CIPHER_TKIP)
				tmp_value |= BIT1;
			if(padapter->securitypriv.wpa2_pairwise_cipher & WPA_CIPHER_CCMP)
				tmp_value |= BIT3;

			memcpy(extra, (unsigned char *)(&tmp_value), sizeof(int));

		}
		else if (MLME_IS_STA(padapter) || MLME_IS_MP(padapter))
		{
			tmp_value = 0;
			if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeWPA2PSK)
			{
				if(padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_)
					tmp_value = BIT1;
				else if(padapter->securitypriv.dot11PrivacyAlgrthm == _AES_)
					tmp_value = BIT3;
			}

			memcpy(extra, (unsigned char *)(&tmp_value), sizeof(int));

		}
	}
}

void core_mib_authtype(_adapter *padapter, char *extra, u32 oper)
{
	int tmp_value=0;
	if(RTW_MIB_SET == oper) {
	}
	else if(RTW_MIB_GET == oper) {
		struct security_priv *securitypriv = get_securitypriv(padapter);

		RTW_INFO("%s(%d), authtype mib get here\n", __FUNCTION__, __LINE__);
		if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter))
		{
			if(!(securitypriv->dot11PrivacyAlgrthm & _WEP40_))
			{
				tmp_value = 2;
				memcpy(extra, (unsigned char *)(&tmp_value), sizeof(int));
			}

		}
		else if (MLME_IS_STA(padapter) || MLME_IS_MP(padapter))
		{
			if(!(securitypriv->dot11PrivacyAlgrthm & _WEP40_))
			{
				tmp_value = 2;
				memcpy(extra, (unsigned char *)(&tmp_value), sizeof(int));
			}
		}
	}
}

void core_mib_multiap_max_device_reached(_adapter *padapter, char *extra, u32 oper)
{
	u8 tmp_value=0;
	if(RTW_MIB_SET == oper) {
	}
	else if(RTW_MIB_GET == oper) {
		RTW_INFO("%s(%d), multiap_max_device_reached mib get here\n", __FUNCTION__, __LINE__);
		tmp_value = *((unsigned char *)extra);
		if(tmp_value != 0)
			tmp_value = 0;
		memcpy(extra, &tmp_value, 1);
	}
}

#ifdef CONFIG_RTW_MIRROR_DUMP
void core_mib_mirror_dump(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	u32 mirror_dump = padapter->registrypriv.wifi_mib.mirror_dump;

	if(RTW_MIB_SET == oper){
		rtw_phl_set_mirror_dump_config(dvobj->phl, mirror_dump);
	}else if(RTW_MIB_GET == oper){
	}
}

void core_mib_mirror_txch(_adapter *padapter, char *extra, u32 oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	u8 mirror_txch = padapter->registrypriv.wifi_mib.mirror_txch;

	if(RTW_MIB_SET == oper){
		rtw_phl_set_mirror_dump_txch(dvobj->phl, mirror_txch);
	}else if(RTW_MIB_GET == oper){
	}
}
#endif

void core_mib_tpc_tx_power(_adapter *padapter, char *extra, u32 oper)
{
	struct wifi_mib_priv *mib = &padapter->registrypriv.wifi_mib;

	if(RTW_MIB_SET == oper) {
		if(rtw_is_adapter_up(padapter)) {
			//rtw_phl_adjust_pwr_dbm(GET_HAL_INFO(padapter->dvobj), mib->tpc_tx_power, 0);
		}
	}
}

void core_mib_hiddenAP(_adapter *padapter, char *extra, u32 oper){

	struct wifi_mib_priv *mib = &padapter->registrypriv.wifi_mib;

	if(RTW_MIB_SET == oper) {
		if (rtw_is_adapter_up(padapter) &&
			padapter->mlmeextpriv.mlmext_info.hidden_ssid_mode != mib->hiddenAP) {
			padapter->mlmeextpriv.mlmext_info.hidden_ssid_mode = mib->hiddenAP;
			send_beacon(padapter);
		}
	}else if(RTW_MIB_GET == oper){
		RTW_PRINT("%s(%d), hidden_ssid_mode=%d\n", __func__, __LINE__,
			padapter->mlmeextpriv.mlmext_info.hidden_ssid_mode);
	}
}

void core_mib_atf(_adapter *padapter, char *extra, u32 oper)
{
	u8 atf = padapter->registrypriv.wifi_mib.atf;

	RTW_INFO("%s(%d), atf=%d, oper:%d\n", __func__, __LINE__, atf, oper);
	if(RTW_MIB_SET == oper) {
		if (atf) { rtw_phl_write32(padapter->dvobj->phl, 0x9E14, 0x00000001); }
		else { rtw_phl_write32(padapter->dvobj->phl, 0x9E14, 0xf0000801); }
	}
}

void core_mib_fast_leave(_adapter *padapter, char *extra, u32 oper)
{
	u8 fast_leave_thr = padapter->registrypriv.wifi_mib.fast_leave_thr;

	RTW_INFO("%s(%d), fast_leave_thr=%d, oper:%d\n", __func__, __LINE__, fast_leave_thr, oper);

	if(RTW_MIB_SET == oper) {
		if (fast_leave_thr < 10 && fast_leave_thr > 0) {
			RTW_PRINT("%s(%d), Set fast_leave_thr to lowest value(10).\n", __func__, __LINE__);
			padapter->registrypriv.wifi_mib.fast_leave_thr = 10;
		}
	}
}

void core_mib_inactive_timeout(_adapter *padapter, char *extra, u32 oper)
{
	u8 inactive_timeout = padapter->registrypriv.wifi_mib.inactive_timeout;
	_list	*phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;

	RTW_INFO("%s(%d), inactive_timeout=%d, oper:%d\n", __func__, __LINE__, inactive_timeout, oper);
	if(RTW_MIB_SET == oper) {
#ifndef CONFIG_ACTIVE_KEEP_ALIVE_CHECK
		pstapriv->expire_to = inactive_timeout;
		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
		phead = &pstapriv->asoc_list;
		plist = get_next(phead);
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);
			psta->expire_to = inactive_timeout;
			if (psta->state & WIFI_STA_ALIVE_CHK_STATE)
				psta->state ^= WIFI_STA_ALIVE_CHK_STATE;
		}
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
#endif
	}
}

#ifdef RTW_WKARD_BCNINT_DBG
void core_mib_bcnint(_adapter *padapter, char *extra, u32 oper)
{
	struct wifi_mib_priv *mib = &GET_PRIMARY_ADAPTER(padapter)->registrypriv.wifi_mib;
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	int i;
	u16 bcnint;
	u8 bcnint_str[2];
	struct rtw_wifi_role_t	*wrole = padapter->phl_role;
	struct rtw_bcn_info_cmn *bcn_cmn = NULL;

	if (RTW_MIB_SET == oper) {
		/* if bcn_cmn is not init do not update beacon */
		if (wrole) {
			bcn_cmn = &wrole->bcn_cmn;
			if (!bcn_cmn->bcn_added)
				return;
		} else
			return;

		if (mib->bcnint)
			bcnint = cpu_to_le16((u16)mib->bcnint);
		else
			bcnint = cpu_to_le16(pmlmeinfo->bcn_interval);

		/* if bcnint is 0, do not update beacon */
		if (bcnint == 0)
			return;

		_rtw_memcpy(bcnint_str, (u8*)&bcnint, 2);
		for (i = 0; i < padapter->dvobj->iface_nums; i++) {
			if (rtw_is_adapter_up(dvobj->padapters[i]) && MLME_IS_AP(dvobj->padapters[i]))
				update_beacon(dvobj->padapters[i], 0xFE, bcnint_str, _TRUE, 0);
		}
	} else if(RTW_MIB_GET == oper) {
	}
}
#endif

#ifdef RTW_WKARD_CUSTOM_PWRLMT_EN
void core_mib_set_txpwr_lmt_index(_adapter *padapter, char *extra, u32 oper)
{
	struct wifi_mib_priv *mib = &padapter->registrypriv.wifi_mib;
	const u8 default_offset = 16;
	u8 tx_path, rx_path;

	if(RTW_MIB_SET == oper) {
		if (rtw_is_adapter_up(padapter)) {
			if (mib->txpwr_lmt_index)
				rtw_phl_set_rf_regulation_idx(GET_HAL_INFO(padapter->dvobj), default_offset + mib->txpwr_lmt_index);
		}
	}
}
#endif

#ifdef CONFIG_RTW_MANUAL_EDCA

void default_WMM_para(_adapter *padapter)
{
	if( padapter->registrypriv.manual_edca ) {
		GET_STA_AC_BE_PARA.acm = padapter->registrypriv.wifi_mib.sta_manual_edca[BE].acm;
		GET_STA_AC_BE_PARA.aifsn = padapter->registrypriv.wifi_mib.sta_manual_edca[BE].aifsn;
		GET_STA_AC_BE_PARA.ecw_min = padapter->registrypriv.wifi_mib.sta_manual_edca[BE].ecw_min;
		GET_STA_AC_BE_PARA.ecw_max = padapter->registrypriv.wifi_mib.sta_manual_edca[BE].ecw_max;
		GET_STA_AC_BE_PARA.txop_limit = padapter->registrypriv.wifi_mib.sta_manual_edca[BE].txop_limit;

		GET_STA_AC_BK_PARA.acm = padapter->registrypriv.wifi_mib.sta_manual_edca[BK].acm;
		GET_STA_AC_BK_PARA.aifsn = padapter->registrypriv.wifi_mib.sta_manual_edca[BK].aifsn;
		GET_STA_AC_BK_PARA.ecw_min = padapter->registrypriv.wifi_mib.sta_manual_edca[BK].ecw_min;
		GET_STA_AC_BK_PARA.ecw_max = padapter->registrypriv.wifi_mib.sta_manual_edca[BK].ecw_max;
		GET_STA_AC_BK_PARA.txop_limit = padapter->registrypriv.wifi_mib.sta_manual_edca[BK].txop_limit;

		GET_STA_AC_VI_PARA.acm = padapter->registrypriv.wifi_mib.sta_manual_edca[VI].acm;
		GET_STA_AC_VI_PARA.aifsn = padapter->registrypriv.wifi_mib.sta_manual_edca[VI].aifsn;
		GET_STA_AC_VI_PARA.ecw_min = padapter->registrypriv.wifi_mib.sta_manual_edca[VI].ecw_min;
		GET_STA_AC_VI_PARA.ecw_max = padapter->registrypriv.wifi_mib.sta_manual_edca[VI].ecw_max;
		GET_STA_AC_VI_PARA.txop_limit = padapter->registrypriv.wifi_mib.sta_manual_edca[VI].txop_limit; // 6.016ms

		GET_STA_AC_VO_PARA.acm = padapter->registrypriv.wifi_mib.sta_manual_edca[VO].acm;
		GET_STA_AC_VO_PARA.aifsn = padapter->registrypriv.wifi_mib.sta_manual_edca[VO].aifsn;
		GET_STA_AC_VO_PARA.ecw_min = padapter->registrypriv.wifi_mib.sta_manual_edca[VO].ecw_min;
		GET_STA_AC_VO_PARA.ecw_max = padapter->registrypriv.wifi_mib.sta_manual_edca[VO].ecw_max;
		GET_STA_AC_VO_PARA.txop_limit = padapter->registrypriv.wifi_mib.sta_manual_edca[VO].txop_limit; // 3.264ms
	} else
	{
		GET_STA_AC_BE_PARA.acm = rtl_sta_EDCA[BE].acm;
		GET_STA_AC_BE_PARA.aifsn = rtl_sta_EDCA[BE].aifsn;
		GET_STA_AC_BE_PARA.ecw_min = rtl_sta_EDCA[BE].ecw_min;
		GET_STA_AC_BE_PARA.ecw_max = rtl_sta_EDCA[BE].ecw_max;
		GET_STA_AC_BE_PARA.txop_limit = rtl_sta_EDCA[BE].txop_limit;

		GET_STA_AC_BK_PARA.acm = rtl_sta_EDCA[BK].acm;
		GET_STA_AC_BK_PARA.aifsn = rtl_sta_EDCA[BK].aifsn;
		GET_STA_AC_BK_PARA.ecw_min = rtl_sta_EDCA[BK].ecw_min;
		GET_STA_AC_BK_PARA.ecw_max = rtl_sta_EDCA[BK].ecw_max;
		GET_STA_AC_BK_PARA.txop_limit = rtl_sta_EDCA[BK].txop_limit;

		GET_STA_AC_VI_PARA.acm = rtl_sta_EDCA[VI].acm;
		GET_STA_AC_VI_PARA.aifsn = rtl_sta_EDCA[VI].aifsn;
		GET_STA_AC_VI_PARA.ecw_min = rtl_sta_EDCA[VI].ecw_min;
		GET_STA_AC_VI_PARA.ecw_max = rtl_sta_EDCA[VI].ecw_max;
		if (padapter->registrypriv.wireless_mode & (WLAN_MD_11G|WLAN_MD_11A))
			GET_STA_AC_VI_PARA.txop_limit = 94; // 3.008ms							GET_STA_AC_VI_PARA.TXOPlimit = rtl_sta_EDCA[VI_AG].TXOPlimit; // 3.008ms
		else
			GET_STA_AC_VI_PARA.txop_limit = 188; // 6.016ms								GET_STA_AC_VI_PARA.TXOPlimit = rtl_sta_EDCA[VI].TXOPlimit; // 6.016ms

		GET_STA_AC_VO_PARA.acm = rtl_sta_EDCA[VO].acm;
		GET_STA_AC_VO_PARA.aifsn = rtl_sta_EDCA[VO].aifsn;
		GET_STA_AC_VO_PARA.ecw_min = rtl_sta_EDCA[VO].ecw_min;
		GET_STA_AC_VO_PARA.ecw_max = rtl_sta_EDCA[VO].ecw_max;
		if (padapter->registrypriv.wireless_mode & (WLAN_MD_11G|WLAN_MD_11A))
			GET_STA_AC_VO_PARA.txop_limit = 47; // 1.504ms							GET_STA_AC_VO_PARA.TXOPlimit = rtl_sta_EDCA[VO_AG].TXOPlimit; // 1.504ms
		else
			GET_STA_AC_VO_PARA.txop_limit = 102; // 3.264ms								GET_STA_AC_VO_PARA.TXOPlimit = rtl_sta_EDCA[VO].TXOPlimit; // 3.264ms
	}
}

void get_AP_Qos_Info(_adapter *padapter, unsigned char *temp)
{
	temp[0] |= BIT(0);
	temp[0] |= BIT(7);
}


void get_STA_AC_Para_Record(_adapter *padapter, unsigned char *temp)
{
//BE
	temp[0] = GET_STA_AC_BE_PARA.aifsn;
	temp[0] &= 0x0f;
	if (GET_STA_AC_BE_PARA.acm)
		temp[0] |= BIT(4);
	temp[1] = GET_STA_AC_BE_PARA.ecw_max;
	temp[1] <<= 4;
	temp[1] |= GET_STA_AC_BE_PARA.ecw_min;
	temp[2] = GET_STA_AC_BE_PARA.txop_limit % 256;
	temp[3] = GET_STA_AC_BE_PARA.txop_limit / 256; // 2^8 = 256, for one byte's range

//BK
	temp[4] = GET_STA_AC_BK_PARA.aifsn;
	temp[4] &= 0x0f;
	if (GET_STA_AC_BK_PARA.acm)
		temp[4] |= BIT(4);
	temp[4] |= BIT(5);
	temp[5] = GET_STA_AC_BK_PARA.ecw_max;
	temp[5] <<= 4;
	temp[5] |= GET_STA_AC_BK_PARA.ecw_min;
	temp[6] = GET_STA_AC_BK_PARA.txop_limit % 256;
	temp[7] = GET_STA_AC_BK_PARA.txop_limit / 256;

//VI
	temp[8] = GET_STA_AC_VI_PARA.aifsn;
	temp[8] &= 0x0f;
	if (GET_STA_AC_VI_PARA.acm)
		temp[8] |= BIT(4);
	temp[8] |= BIT(6);
	temp[9] = GET_STA_AC_VI_PARA.ecw_max;
	temp[9] <<= 4;
	temp[9] |= GET_STA_AC_VI_PARA.ecw_min;
	temp[10] = GET_STA_AC_VI_PARA.txop_limit % 256;
	temp[11] = GET_STA_AC_VI_PARA.txop_limit / 256;

//VO
	temp[12] = GET_STA_AC_VO_PARA.aifsn;
	temp[12] &= 0x0f;
	if (GET_STA_AC_VO_PARA.acm)
		temp[12] |= BIT(4);
	temp[12] |= BIT(5)|BIT(6);
	temp[13] = GET_STA_AC_VO_PARA.ecw_max;
	temp[13] <<= 4;
	temp[13] |= GET_STA_AC_VO_PARA.ecw_min;
	temp[14] = GET_STA_AC_VO_PARA.txop_limit % 256;
	temp[15] = GET_STA_AC_VO_PARA.txop_limit / 256;
}

void init_WMM_Para_Element(_adapter *padapter, unsigned char *temp, unsigned long len)
{
	unsigned char WMM_PARA_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};
	unsigned char WMM_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};

	if (MLME_IS_AP(padapter)) {
		if (len < 24)
			return;

		_rtw_memcpy(temp, WMM_PARA_IE, 6);
//Qos Info field
		get_AP_Qos_Info(padapter, &temp[6]);
//AC Parameters
		get_STA_AC_Para_Record(padapter, &temp[8]);

 	}
//#ifdef CLIENT_MODE
	else if ((MLME_IS_STA(padapter)) ||(MLME_IS_ADHOC(padapter))) {  //  WMM STA
		if (len < 7)
			return;
		_rtw_memcpy(temp, WMM_IE, 6);
		temp[6] = 0x00;  //  set zero to WMM STA Qos Info field
	}
//#endif
}

void dynamic_EDCA_para(_adapter *padapter) {
	static unsigned int slot_time = 20, sifs_time = 10;
	struct _ParaRecord EDCA[4];

	if ((padapter->registrypriv.wireless_mode & WLAN_MD_11N ) ||
		(padapter->registrypriv.wireless_mode & WLAN_MD_11G))
		slot_time = 9;

	if (padapter->registrypriv.wireless_mode & WLAN_MD_11N)
		sifs_time = 16;

	if (padapter->registrypriv.manual_edca) {
		 memset(EDCA, 0, 4*sizeof(struct _ParaRecord));
		 if(MLME_IS_AP(padapter))
			 memcpy(EDCA, padapter->registrypriv.wifi_mib.ap_manual_edca, 4*sizeof(struct _ParaRecord));
		 else
			 memcpy(EDCA, padapter->registrypriv.wifi_mib.sta_manual_edca, 4*sizeof(struct _ParaRecord));

		// VO
		rtw_phl_write32(padapter->dvobj->phl, 0xc30C, (EDCA[VO].txop_limit << 16) | (EDCA[VO].ecw_max << 12) | (EDCA[VO].ecw_min << 8) | (sifs_time + EDCA[VO].aifsn * slot_time));

		// VI
		rtw_phl_write32(padapter->dvobj->phl, 0xc308, (EDCA[VI].txop_limit << 16) | (EDCA[VI].ecw_max << 12) | (EDCA[VI].ecw_min << 8) | (sifs_time + EDCA[VI].aifsn * slot_time));

		// BE
		rtw_phl_write32(padapter->dvobj->phl, 0xc300, (EDCA[BE].txop_limit << 16) | (EDCA[BE].ecw_max << 12) | (EDCA[BE].ecw_min << 8) | (sifs_time + EDCA[BE].aifsn * slot_time));

		// BK
		rtw_phl_write32(padapter->dvobj->phl, 0xc304, (EDCA[BK].txop_limit << 16) | (EDCA[BK].ecw_max << 12) | (EDCA[BK].ecw_min << 8) | (sifs_time + EDCA[BK].aifsn * slot_time));
	}

}

void core_mib_manual_edca(_adapter *padapter, char *extra, u32 oper){

	if(RTW_MIB_SET == oper) {

		if((MLME_IS_AP(padapter)) || (MLME_IS_ADHOC(padapter))) {
			default_WMM_para(padapter);
			init_WMM_Para_Element(padapter, padapter->registrypriv.wifi_mib.WMM_PARA_IE, sizeof(padapter->registrypriv.wifi_mib.WMM_PARA_IE));
		}
//#ifdef CLIENT_MODE
		else if(MLME_IS_STA(padapter)) {
			init_WMM_Para_Element(padapter, padapter->registrypriv.wifi_mib.WMM_IE, sizeof(padapter->registrypriv.wifi_mib.WMM_IE));	//	WMM STA
		}
//#endif
		dynamic_EDCA_para(padapter);

	}else if(RTW_MIB_GET == oper){
		RTW_PRINT("%s(%d)\n\n", __func__, __LINE__);
		RTW_PRINT("manual_edca=%d\n\n", padapter->registrypriv.manual_edca);

		RTW_PRINT("sta_bkq_acm=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[BK].acm);
		RTW_PRINT("sta_bkq_aifsn=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[BK].aifsn);
		RTW_PRINT("sta_bkq_cwmin=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[BK].ecw_min);
		RTW_PRINT("sta_bkq_cwmax=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[BK].ecw_max);
		RTW_PRINT("sta_bkq_txoplimit=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[BK].txop_limit);
		RTW_PRINT("sta_beq_acm=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[BE].acm);
		RTW_PRINT("sta_beq_aifsn=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[BE].aifsn);
		RTW_PRINT("sta_beq_cwmin=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[BE].ecw_min);
		RTW_PRINT("sta_beq_cwmax=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[BE].ecw_max);
		RTW_PRINT("sta_beq_txoplimit=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[BE].txop_limit);
		RTW_PRINT("sta_viq_acm=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[VI].acm);
		RTW_PRINT("sta_viq_aifsn=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[VI].aifsn);
		RTW_PRINT("sta_viq_cwmin=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[VI].ecw_min);
		RTW_PRINT("sta_viq_cwmax=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[VI].ecw_max);
		RTW_PRINT("sta_viq_txoplimit=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[VI].txop_limit);
		RTW_PRINT("sta_voq_acm=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[VO].acm);
		RTW_PRINT("sta_voq_aifsn=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[VO].aifsn);
		RTW_PRINT("sta_voq_cwmin=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[VO].ecw_min);
		RTW_PRINT("sta_voq_cwmax=%d\n", padapter->registrypriv.wifi_mib.sta_manual_edca[VO].ecw_max);
		RTW_PRINT("sta_voq_txoplimit=%d\n\n", padapter->registrypriv.wifi_mib.sta_manual_edca[VO].txop_limit);

		RTW_PRINT("ap_bkq_acm=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[BK].acm);
		RTW_PRINT("ap_bkq_aifsn=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[BK].aifsn);
		RTW_PRINT("ap_bkq_cwmin=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[BK].ecw_min);
		RTW_PRINT("ap_bkq_cwmax=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[BK].ecw_max);
		RTW_PRINT("ap_bkq_txoplimit=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[BK].txop_limit);
		RTW_PRINT("ap_beq_acm=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[BE].acm);
		RTW_PRINT("ap_beq_aifsn=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[BE].aifsn);
		RTW_PRINT("ap_beq_cwmin=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[BE].ecw_min);
		RTW_PRINT("ap_beq_cwmax=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[BE].ecw_max);
		RTW_PRINT("ap_beq_txoplimit=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[BE].txop_limit);
		RTW_PRINT("ap_viq_acm=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[VI].acm);
		RTW_PRINT("ap_viq_aifsn=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[VI].aifsn);
		RTW_PRINT("ap_viq_cwmin=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[VI].ecw_min);
		RTW_PRINT("ap_viq_cwmax=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[VI].ecw_max);
		RTW_PRINT("ap_viq_txoplimit=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[VI].txop_limit);
		RTW_PRINT("ap_voq_acm=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[VO].acm);
		RTW_PRINT("ap_voq_aifsn=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[VO].aifsn);
		RTW_PRINT("ap_voq_cwmin=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[VO].ecw_min);
		RTW_PRINT("ap_voq_cwmax=%d\n", padapter->registrypriv.wifi_mib.ap_manual_edca[VO].ecw_max);
		RTW_PRINT("ap_voq_txoplimit=%d\n\n", padapter->registrypriv.wifi_mib.ap_manual_edca[VO].txop_limit);

	}
}
#endif

enum rf_path _check_path_nss(struct dvobj_priv *dvobj, u32 path, u8 *nss) {
	enum rf_path path_result = RF_PATH_A;
	u8 path_limit = 1;
	static char path_map[16] = {-1, RF_PATH_A, RF_PATH_B, RF_PATH_AB,
			RF_PATH_C, RF_PATH_AC, RF_PATH_BC, RF_PATH_ABC,
			RF_PATH_D, RF_PATH_AD, RF_PATH_BD, RF_PATH_ABD,
			RF_PATH_CD, RF_PATH_ACD, RF_PATH_BCD, RF_PATH_ABCD};

	if (dvobj->phl_com->hal_spec.rf_reg_path_num)
		path_limit = 1 << dvobj->phl_com->hal_spec.rf_reg_path_num;

	*nss = 0;
	if (path == 0)
		path = path_limit - 1;

	if (path >= path_limit) {
		/*consider path under HW limitation only */
		path_result = 0;
	} else {
		path_result = path_map[path];
		while (path) {
			if (path & BIT0)
				*nss += 1;
			path = path >> 1;
		}
	}
	return path_result;
}

void _update_proto_cap_nss(_adapter *padapter, u8 tx_nss, u8 rx_nss)
{
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	struct protocol_cap_t *proto_cap = &(wrole->proto_role_cap);

	switch (rx_nss) {
		default:
			break;
		case 1:
			proto_cap->ht_rx_mcs[0] = 0xff;
			proto_cap->vht_rx_mcs[0] = 0xfe;
			proto_cap->vht_rx_mcs[1] = 0xff;
			proto_cap->he_rx_mcs[0] = 0xfe;
			proto_cap->he_rx_mcs[1] = 0xff;
			break;
		case 2:
			proto_cap->ht_rx_mcs[0] = 0xff;
			proto_cap->ht_rx_mcs[1] = 0xff;
			proto_cap->vht_rx_mcs[0] = 0xfa;
			proto_cap->vht_rx_mcs[1] = 0xff;
			proto_cap->he_rx_mcs[0] = 0xfa;
			proto_cap->he_rx_mcs[1] = 0xff;
			proto_cap->he_rx_mcs[2] = 0xfa;
			proto_cap->he_rx_mcs[3] = 0xff;
			break;
	}
	switch (tx_nss) {
		default:
			break;
		case 1:
			proto_cap->ht_tx_mcs[0] = 0xff;
			proto_cap->vht_tx_mcs[0] = 0xfe;
			proto_cap->vht_tx_mcs[1] = 0xff;
			proto_cap->he_tx_mcs[0] = 0xfe;
			proto_cap->he_tx_mcs[1] = 0xff;
			break;
		case 2:
			proto_cap->ht_tx_mcs[0] = 0xff;
			proto_cap->ht_tx_mcs[1] = 0xff;
			proto_cap->vht_tx_mcs[0] = 0xfa;
			proto_cap->vht_tx_mcs[1] = 0xff;
			proto_cap->he_tx_mcs[0] = 0xfa;
			proto_cap->he_tx_mcs[1] = 0xff;
			proto_cap->he_tx_mcs[2] = 0xfa;
			proto_cap->he_tx_mcs[3] = 0xff;
			break;
	}

	proto_cap->nss_tx = tx_nss;
	proto_cap->nss_rx = rx_nss;
}

void core_mib_trx_path(_adapter *padapter, char *extra, u32 oper)
{
	struct wifi_mib_priv *mib = &GET_PRIMARY_ADAPTER(padapter)->registrypriv.wifi_mib;
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct rtw_phl_com_t *phl_com = dvobj->phl_com;
	int i;
	struct rtw_wifi_role_t *wrole;
	u8 tx_rf_path = 0, rx_rf_path = 0;
	u8 phl_tx_nss_bk, phl_rx_nss_bk;
	struct rtw_trx_path_param trx_param;
	_list	*phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 he_cap_eid_ext = WLAN_EID_EXTENSION_HE_CAPABILITY;

	if (RTW_MIB_SET == oper) {
		tx_rf_path = mib->trx_path & 0xf;
		rx_rf_path = (mib->trx_path & 0xf0) >> 4;

		trx_param.tx = _check_path_nss(dvobj, tx_rf_path, &trx_param.tx_nss);
		trx_param.rx = _check_path_nss(dvobj, rx_rf_path, &trx_param.rx_nss);
		if (trx_param.tx_nss == 0 || trx_param.rx_nss == 0) {
			RTW_ERR("[%s]Invalid txnss = %d, rxnss = %d\n",
				__func__, trx_param.tx_nss, trx_param.rx_nss);
			return;
		}
		RTW_INFO("[%s]Set trx_path = 0x%02x, tx = %d, rx = %d, txnss = %d, rxnss = %d\n",
				__func__, mib->trx_path,
				trx_param.tx, trx_param.rx,
				trx_param.tx_nss, trx_param.rx_nss);

		/* check nss according to HW cap */
		phl_tx_nss_bk = phl_com->phy_cap[0].txss;
		phl_rx_nss_bk = phl_com->phy_cap[0].rxss;
		dvobj->phl_com->phy_sw_cap[0].txss = trx_param.tx_nss;
		dvobj->phl_com->phy_sw_cap[0].rxss = trx_param.rx_nss;
		rtw_phl_final_cap_decision(dvobj->phl);
		if ((trx_param.tx_nss > phl_com->phy_cap[0].txss) ||
			(trx_param.rx_nss > phl_com->phy_cap[0].rxss)) {
			RTW_ERR("[%s]Invalid txnss = %d(HW:%d), rxnss = %d(HW:%d)\n",
				__func__, trx_param.tx_nss, phl_com->phy_cap[0].txss,
				phl_com->phy_cap[0].rxss, trx_param.rx_nss);
			/* restore protocol cap */
			dvobj->phl_com->phy_sw_cap[0].txss = phl_tx_nss_bk;
			dvobj->phl_com->phy_sw_cap[0].rxss = phl_rx_nss_bk;
			rtw_phl_final_cap_decision(dvobj->phl);
			RTW_INFO("[%s]Restore txnss = %d, rxnss = %d\n",
				__func__, phl_com->phy_cap[0].txss, phl_com->phy_cap[0].rxss);
			return;
		}
		/* backup path setting to dvobj */
		if ((dvobj->phl_com->hal_spec.rf_reg_path_num == trx_param.tx_nss) &&
		    (dvobj->phl_com->hal_spec.rf_reg_path_num == trx_param.rx_nss)) {
		    dvobj->rf_ctl.trx_path.tx_nss = 0;
		    dvobj->rf_ctl.trx_path.rx_nss = 0;
		    dvobj->rf_ctl.trx_path.tx = trx_param.tx;
		    dvobj->rf_ctl.trx_path.rx = trx_param.rx;
		} else {
			_rtw_memcpy(&dvobj->rf_ctl.trx_path,
				&trx_param,
				sizeof(struct rtw_trx_path_param));
		}

		/* apply path setting on existing wifi role and associated STAs */
		for (i = 0; i < padapter->dvobj->iface_nums; i++) {
			if (!dvobj->padapters[i]->netif_up)
				continue;

			wrole = dvobj->padapters[i]->phl_role;
			if (!wrole)
				continue;
			_update_proto_cap_nss(dvobj->padapters[i], trx_param.tx_nss, trx_param.rx_nss);

			rtw_phl_cmd_wrole_change(dvobj->phl,
                         			wrole,
                         			WR_CHG_TRX_PATH,
                        			(u8 *)&trx_param,
                         			sizeof(trx_param),
                         			PHL_CMD_DIRECTLY,
                         			0);

			pstapriv = &dvobj->padapters[i]->stapriv;

                       	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
			phead = &pstapriv->asoc_list;
			plist = get_next(phead);
			while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
				psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
				plist = get_next(plist);

				if (psta->phl_sta->asoc_cap.nss_rx == trx_param.tx_nss)
					continue;
				else if (psta->nss_rx_assoc < trx_param.tx_nss)
					psta->phl_sta->asoc_cap.nss_rx = psta->nss_rx_assoc;
				else
					psta->phl_sta->asoc_cap.nss_rx = trx_param.tx_nss;

				rtw_phl_cmd_change_stainfo(dvobj->phl, psta->phl_sta,
					STA_CHG_NSS, NULL, 0, PHL_CMD_DIRECTLY, 0);
			}
			_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

			update_beacon(dvobj->padapters[i], _HT_CAPABILITY_IE_, NULL, _FALSE, 0);
			update_beacon(dvobj->padapters[i], WLAN_EID_VHT_CAPABILITY, NULL, _FALSE, 0);
			update_beacon(dvobj->padapters[i], 0xFF, &he_cap_eid_ext, _TRUE, 0);
		}
	} else if(RTW_MIB_GET == oper) {
		memcpy(extra, &mib->trx_path, 4);
	}
}

void core_mib_set_dig_opmode(_adapter *padapter, char *extra, u32 oper)
{
	struct wifi_mib_priv *mib = &GET_PRIMARY_ADAPTER(padapter)->registrypriv.wifi_mib;

	if(RTW_MIB_SET == oper) {
		if (rtw_is_adapter_up(GET_PRIMARY_ADAPTER(padapter))) {
			rtw_phl_set_dig_opmode(GET_HAL_INFO(padapter->dvobj), mib->dig_opmode);
			RTW_PRINT("set dig opmode: %d\n", mib->dig_opmode);
		}
	}
}

/************************************MIB GET FUNCTION************************************/
/*flag: To distinguish the type of info supported to user space.
0: Support origin char/int/short info. (ioctl call)
1: Change the char/int/short info to string type. (iwpriv call)*/
int rtw_mib_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra, unsigned char flag)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	int cmdlen, copy_len;
	struct iw_point *p;
	u8 *ptmp;
	struct iwpriv_arg *entry;
	unsigned char tmpbuf[256], tmpbuf2[256];
	void *mib_offset = 0;

#if !defined(PLATFORM_ECOS) && defined(CPTCFG_WFO_VIRT_MODULE)
	wfo_adapter_t *wfo_adapter = NULL;
#endif /* !PLATFORM_ECOS & CPTCFG_WFO_VIRT_MODULE */

	p = &wrqu->data;
	cmdlen = p->length;
	if (0 == cmdlen)
		return -EINVAL;

	if(cmdlen > sizeof(tmpbuf)) {
		RTW_ERR("invalid mib name len [%d] !\n", cmdlen);
		return -EINVAL;
	}

	ptmp = (u8 *)tmpbuf;
	memset(ptmp, 0, sizeof(tmpbuf));
	if (copy_from_user(ptmp, p->pointer, cmdlen)) {
		return -EFAULT;
	}

	entry = get_tbl_entry((char *)ptmp);
	if (entry == NULL) {
		RTW_ERR("invalid mib name [%s] !\n", ptmp);
		return -EINVAL;
	}

#if !defined(PLATFORM_ECOS) && defined(CPTCFG_WFO_VIRT_MODULE)
	if (wfo_enable) {
		padapter = (_adapter *)wfo_get_mib_offset(dev, ptmp, (void *)entry);
		if (padapter == NULL) {
			printk("%s(): invalid padapter! dev=%s\n", __func__, dev->name);
			return -EINVAL;
		}
		if (chk_wfo_adapter_sig(padapter)) {
			wfo_adapter = (wfo_adapter_t *)padapter;
			padapter = wfo_adapter->padapter;
		}
	}
#endif /* !PLATFORM_ECOS & CPTCFG_WFO_VIRT_MODULE */

	if(entry->offset_prev)
		mib_offset = (void *)padapter + entry->offset_prev;
	else if(entry->offset_mib)
		mib_offset = (void *)padapter + entry->offset_mib;

#if !defined(PLATFORM_ECOS) && defined(CPTCFG_WFO_VIRT_MODULE)
	if (wfo_enable && wfo_adapter)
		padapter = (_adapter *)wfo_adapter;
#endif /* !PLATFORM_ECOS & CPTCFG_WFO_VIRT_MODULE */

	copy_len = entry->len;
	//RTW_PRINT("entry->len : %d\n", entry->len);
	//RTW_PRINT("entry->offset_prev : %d\n", entry->offset_prev);
	//RTW_PRINT("entry->type : %d\n", entry->type);

	memset(&tmpbuf2, 0, sizeof(tmpbuf2));
	switch (entry->type)
	{
		case BYTE_T:
			if (mib_offset) memcpy(tmpbuf2, (unsigned char *)mib_offset,  1);

			if(entry->fun){
				entry->fun(padapter, tmpbuf2, RTW_MIB_GET);
			}
			if(flag){
				RTW_PRINT("byte data: %d\n", *((unsigned char *)tmpbuf2));
				snprintf(extra, sizeof(tmpbuf2), "%d", *((unsigned char *)tmpbuf2));
				copy_len = strlen(extra);
			}
			else
				memcpy(extra, tmpbuf2, 1);
			break;
		case INT_T:
			if (mib_offset) memcpy(tmpbuf2, (unsigned char *)mib_offset, sizeof(int));

			if(entry->fun){
				entry->fun(padapter, tmpbuf2, RTW_MIB_GET);
			}
			if(flag){
				RTW_PRINT("int data: %d\n", *((int *)tmpbuf2));
				snprintf(extra, sizeof(tmpbuf2), "%d", *((int *)tmpbuf2));
				copy_len = strlen(extra);
			}
			else
				memcpy(extra, tmpbuf2, sizeof(int));
			break;
		case STRING_T:
			if (entry->len > MAX_MIB_DATA_SIZE)
				break;
			else {
				if (mib_offset) strncpy(extra, (unsigned char *)mib_offset, entry->len);
			}

			if(entry->fun)
				entry->fun(padapter, extra, RTW_MIB_GET);
			if(flag)
				RTW_PRINT("string data: %s\n", extra);
			break;
		case SHORT_T:
			if (mib_offset) memcpy(tmpbuf2, (unsigned char *)mib_offset, sizeof(u16));

			if(flag){
				RTW_PRINT("short data: %d\n", *((u16 *)tmpbuf2));
				snprintf(extra, sizeof(tmpbuf2), "%d", *((int *)tmpbuf2));
				copy_len = strlen(extra);
			}
			else
				memcpy(extra, tmpbuf2, sizeof(u16));
			break;
		default:
			RTW_ERR("invalid mib type!\n");
			return -EINVAL;
	}

	wrqu->data.length = copy_len;

	return 0;

}
#if !defined(PLATFORM_ECOS) && defined(CPTCFG_WFO_VIRT_MODULE)
EXPORT_SYMBOL(rtw_mib_get);
#endif /* !PLATFORM_ECOS & CPTCFG_WFO_VIRT_MODULE */

int rtw_wifi_syn_registrypriv_to_priv_mib(struct net_device *dev)
{
	int i;
	int arg_num = sizeof(mib_table)/sizeof(struct iwpriv_arg);
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	unsigned char *p, *extra;

	for (i=0; i<arg_num; i++) {
		if(!strcmp(mib_table[i].name, "rtw_mib_version") || !strcmp(mib_table[i].name, "rtw_mib_size")){
			extra = rtw_zmalloc(mib_table[i].len);
			if (extra) {
				mib_table[i].fun(padapter, extra, RTW_MIB_GET);
				memcpy(((unsigned char *)padapter) + mib_table[i].offset_mib, extra, mib_table[i].len);
				rtw_mfree(extra, mib_table[i].len);
			}
		}
		if(mib_table[i].offset_prev)
		{
			if(mib_table[i].offset_mib)
			{
				switch (mib_table[i].type) {
				case BYTE_T:
					p = ((unsigned char *)padapter) + mib_table[i].offset_prev;
					//RTW_PRINT("%s = %d\n", mib_table[i].name, *p);
					break;

				case INT_T:
					p = ((unsigned char *)padapter) + mib_table[i].offset_prev;
					//RTW_PRINT("%s = %d\n", mib_table[i].name, *((int *)p));
					break;

				case SHORT_T:
					p = ((unsigned char *)padapter) + mib_table[i].offset_prev;
					//RTW_PRINT("%s = %d\n", mib_table[i].name, *((u16 *)p));
					break;

				case STRING_T:
					p = ((unsigned char *)padapter) + mib_table[i].offset_prev;
					//RTW_PRINT("%s = %s\n", mib_table[i].name, p);
					break;

				default:
					break;
				}
				memcpy(((unsigned char *)padapter) + mib_table[i].offset_mib, ((unsigned char *)padapter) + mib_table[i].offset_prev, mib_table[i].len);
			}
		}
	}

	return 0;
}

int rtw_wifi_priv_mib_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;
	unsigned char tmp_band = pmibpriv->band;
	rtw_wifi_syn_registrypriv_to_priv_mib(dev);
	core_mib_band(padapter, &(pmibpriv->band), RTW_MIB_GET);
	_rtw_memcpy((struct wifi_mib_priv *)extra, pmibpriv, sizeof(struct wifi_mib_priv));
	pmibpriv->band = tmp_band;
	wrqu->data.length = sizeof(struct wifi_mib_priv);
	return 0;
}

/************************************MIB SET FUNCTION *** ******************************/
/* FUNC set_rtw_mib_default_tbl only give default value of types of BYTE_T and INT_T here*/
/* Types STRING_T were given 0 as default value.*/
int set_rtw_mib_default_tbl(_adapter *padapter, struct wifi_mib_priv *rtw_priv_mib)
{
	int i=0, arg_num=0;

	arg_num = sizeof(mib_table)/sizeof(struct iwpriv_arg);
	for(i=0; i<arg_num; i++) {
		switch (mib_table[i].type)
		{
			case BYTE_T:
				*(((unsigned char *)padapter) + mib_table[i].offset_mib) = (unsigned char)mib_table[i].Default;
				break;
			case INT_T:
				memcpy(((unsigned char *)padapter) + mib_table[i].offset_mib, (unsigned char *)&mib_table[i].Default, sizeof(u32));
				break;
			case SHORT_T:
				memcpy(((unsigned char *)padapter) + mib_table[i].offset_mib, (unsigned char *)&mib_table[i].Default, sizeof(u16));
				break;
			case STRING_T:
				memset(((unsigned char *)padapter) + mib_table[i].offset_mib, 0, mib_table[i].len);
				break;
			default:
				break;
		}
	}
	return 0;
}

int rtw_mib_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct iwpriv_arg *entry;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char *arg_val;
	unsigned char byte_val;
	int is_hex_type = 0;
	int int_val=0;
	u16 short_val=0;
	u8 copy_len=0;
	u8 *ptr;

	entry = get_tbl_entry((char *)extra);
	if (entry == NULL) {
		RTW_ERR("invalid mib name [%s] !\n", extra);
		return -1;
	}

	DBGP("%s \n", extra);
	//RTW_PRINT("entry->len : %d\n", entry->len);
	//RTW_PRINT("entry->offset_prev : %d\n", entry->offset_prev);
	//RTW_PRINT("entry->type : %d\n", entry->type);

	// search value
	arg_val = (char *)extra;
	while (*arg_val && *arg_val != '='){
		arg_val++;
	}


	if (!*arg_val) {
		RTW_ERR("mib value empty [%s] !\n", extra);
		return -1;
	}

	arg_val++;


	// skip space
	while (*arg_val && *arg_val == 0x7f)
		arg_val++;


	switch (entry->type) {
	case BYTE_T:
		{
			//byte_val = (unsigned char)_atoi(arg_val, 10);
			sscanf(arg_val, "%d", &int_val);
			if(entry->offset_prev)
				*((unsigned char *)padapter + entry->offset_prev) = (u8)int_val;
			if(entry->offset_mib)
				*((unsigned char *)padapter + entry->offset_mib) = (u8)int_val;
			//RTW_PRINT("(%s)byte_val %d\n", entry->name, int_val);
			//memcpy((unsigned char *)padapter + entry->offset_prev, &byte_val,  1);
		}
		break;
	case INT_T:
		if(*arg_val=='0' && (*(arg_val+1)== 'x' || *(arg_val+1)== 'X')){
			is_hex_type=1;
			arg_val+=2;
			//RTW_PRINT("[%s %d]hex format\n",__FUNCTION__,__LINE__);
		}
		if(is_hex_type)
			sscanf(arg_val, "%x", &int_val);
		else
			sscanf(arg_val, "%d", &int_val);
		if(entry->offset_prev)
			memcpy((unsigned char *)padapter + entry->offset_prev, (unsigned char *)&int_val, sizeof(int));
		if(entry->offset_mib)
			memcpy((unsigned char *)padapter + entry->offset_mib, (unsigned char *)&int_val, sizeof(int));
#ifdef MONITOR_UNASSOC_STA
		if(strcmp("monitor_sta_enabled", entry->name)==0){
			if(1==int_val){
				rtw_hw_set_rx_mode(padapter, PHL_RX_MODE_SNIFFER);
			}else if(0==int_val){
				struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
				rtw_hw_set_rx_mode(padapter, PHL_RX_MODE_NORMAL);
				memset(&pmlmeext->monitor_sta_info, 0, sizeof(MONITOR_STA));
			}else{
				printk("unkown mib value %d\n", int_val);
			}
		}
#endif
		break;
	case STRING_T:
		if (strlen(arg_val) >= entry->len)
			copy_len = entry->len;
		else
			copy_len = strlen(arg_val);
		if(entry->offset_prev){
			ptr = (unsigned char *)padapter + entry->offset_prev;
		}else{
			ptr = (unsigned char *)padapter + entry->offset_mib;
		}
		strncpy(ptr, (unsigned char *)arg_val, copy_len);
		if(copy_len<32)
			ptr[copy_len] = '\0';
		break;
	case SHORT_T:
		if(*arg_val=='0' && (*(arg_val+1)== 'x' || *(arg_val+1)== 'X')){
			is_hex_type=1;
			arg_val+=2;
			//RTW_PRINT("[%s %d]hex format\n",__FUNCTION__,__LINE__);
		}
		if(is_hex_type)
			sscanf(arg_val, "%x", &int_val);
		else
			sscanf(arg_val, "%d", &int_val);
		if(entry->offset_prev)
		{
			short_val = (u16)int_val;
			memcpy((unsigned char *)padapter + entry->offset_prev, (unsigned char *)&short_val, sizeof(u16));
		}
		if(entry->offset_mib)
		{
			short_val = (u16)int_val;
			memcpy((unsigned char *)padapter + entry->offset_mib, (unsigned char *)&short_val, sizeof(u16));
		}
		break;
	default:
		RTW_ERR("invalid mib type!\n");
		break;
	}

	if (entry->fun){
		entry->fun(padapter, extra, RTW_MIB_SET);
	}

	return 0;
}

int rtw_wifi_syn_priv_mib_to_registrypriv(struct net_device *dev)
{
	int i;
	int arg_num = sizeof(mib_table)/sizeof(struct iwpriv_arg);
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char *extra = NULL;
	for (i=0; i<arg_num; i++) {
		if(mib_table[i].offset_mib)
		{
			if(mib_table[i].offset_prev)
			{
				unsigned char *p;
				switch (mib_table[i].type) {
				case BYTE_T:
					p = ((unsigned char *)padapter) + mib_table[i].offset_mib;
					//RTW_PRINT("%s = %d\n", mib_table[i].name, *p);
					break;

				case INT_T:
					p = ((unsigned char *)padapter) + mib_table[i].offset_mib;
					//RTW_PRINT("%s = %d\n", mib_table[i].name, *((int *)p));
					break;

				case SHORT_T:
					p = ((unsigned char *)padapter) + mib_table[i].offset_mib;
					//RTW_PRINT("%s = %d\n", mib_table[i].name, *((u16 *)p));
					break;

				case STRING_T:
					p = ((unsigned char *)padapter) + mib_table[i].offset_mib;
					//RTW_PRINT("%s = %s\n", mib_table[i].name, p);
					break;

				default:
					break;
				}
				memcpy(((unsigned char *)padapter) + mib_table[i].offset_prev, ((unsigned char *)padapter) + mib_table[i].offset_mib, mib_table[i].len);
			}

			if(mib_table[i].fun){
				if(strncmp(mib_table[i].name, "band", 4) || (!strncmp(mib_table[i].name, "band", 4) && MLME_IS_STA(padapter)))
					mib_table[i].fun(padapter, extra, RTW_MIB_SET);
			}
		}
	}
	return 0;
}
int rtw_wifi_priv_mib_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;
	_rtw_memcpy((void *)pmibpriv, (void *)extra, sizeof(struct wifi_mib_priv));
	rtw_wifi_syn_priv_mib_to_registrypriv(dev);
	return 0;
}

int rtw_wifi_dump_priv_mib(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	int i,j, len;
	unsigned char *p;
	int arg_num = sizeof(mib_table)/sizeof(struct iwpriv_arg);

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;

	for (i=0; i<arg_num; i++) {
		if(mib_table[i].offset_prev)
			p = ((unsigned char *)padapter) + mib_table[i].offset_prev;
		else
			p = ((unsigned char *)padapter) + mib_table[i].offset_mib;

		switch (mib_table[i].type) {
		case BYTE_T:
			RTW_PRINT("%s = %d\n", mib_table[i].name, *p);
			break;

		case INT_T:
			RTW_PRINT("%s = %d\n", mib_table[i].name, *((int *)p));
			break;

		case SHORT_T:
			RTW_PRINT("%s = %d\n", mib_table[i].name, *((u16 *)p));
			break;

		case STRING_T:
			RTW_PRINT("%s = %s\n", mib_table[i].name, p);
			break;

		default:
			break;
		}
	}

	return 0;
}

void rtw_core_recover_mibs(_adapter *padapter) {

	/* func_off */
	padapter->registrypriv.wifi_mib.func_off_prev = 0;
	core_mib_func_off(padapter, NULL, RTW_MIB_SET);

	if (is_primary_adapter(padapter)) {
		/* fw_tx */
		core_mib_fw_tx(padapter, NULL, RTW_MIB_SET);

		/* lifetime */
		if (padapter->registrypriv.wifi_mib.lifetime)
			core_mib_lifetime(padapter, NULL, RTW_MIB_SET);

		/* powerpercent */
		core_mib_power_percent(padapter, NULL, RTW_MIB_SET);
	}
}

#endif
