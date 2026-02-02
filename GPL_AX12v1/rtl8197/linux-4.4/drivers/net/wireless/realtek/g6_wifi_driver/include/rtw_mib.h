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
#ifndef _RTW_MIB_H_
#define _RTW_MIB_H_

#define RTW_WIFI_MIB_VER_MAJOR "1"
#define RTW_WIFI_MIB_VER_MINOR "0"
#define RTW_WIFI_MIB_VERSION   RTW_WIFI_MIB_VER_MAJOR "." RTW_WIFI_MIB_VER_MINOR

/* flag of sta info */
#define STA_INFO_FLAG_AUTH_OPEN     			0x01
#define STA_INFO_FLAG_AUTH_WEP      			0x02
#define STA_INFO_FLAG_ASOC          			0x04
#define STA_INFO_FLAG_ASLEEP        			0x08

#define _NUM_SBWC			64
#define _NUM_GBWC			64

#define STBC_RX_EN BIT0
#define STBC_TX_EN BIT1

struct _SBWC_ENTRY {
	unsigned char mac[MAC_ADDR_LEN];
	unsigned int tx_lmt;
	unsigned int rx_lmt;
};

struct _StaBandwidthControl {
	unsigned int count;
	struct _SBWC_ENTRY entry[_NUM_SBWC];
};

struct _GBWC_ENTRY {
	unsigned char mac[MAC_ADDR_LEN];
};

struct _GroupBandwidthControl {
	unsigned int count;
	struct _GBWC_ENTRY entry[_NUM_GBWC];
};

enum wifi_pri_mapping_rule {
	TOS_MODE = 0,
	DSCP_MODE = 1,
};

enum qos_prio { BK, BE, VI, VO};
struct _ParaRecord {
	unsigned int	acm;
	unsigned int	aifsn;
	unsigned int	ecw_min;
	unsigned int	ecw_max;
	unsigned int	txop_limit;
};

struct wifi_mib_priv {
	unsigned char	rtw_mib_version[16];
	unsigned int	rtw_mib_size;
	unsigned int	func_off;
	unsigned int	func_off_prev;
	unsigned char	disable_protection;
	unsigned char	aggregation;
	unsigned char	iapp_enable;
	unsigned int	a4_enable;
	unsigned char	multiap_monitor_mode_disable;
	unsigned char	multiap_bss_type;
	unsigned char	multiap_profile;
	unsigned char	multiap_max_device_reached;
  	unsigned char	multiap_report_rcpi_threshold;
	unsigned char	multiap_report_rcpi_hysteris_margin;
	/* Steering Policy TLV parameters */
	unsigned char   multiap_steering_policy;
	unsigned char	multiap_cu_threshold;
	unsigned char   multiap_rcpi_steering_th;
	/*=================*/
	unsigned char   multiap_change_channel;
	unsigned char   multiap_dfs_ap_mib_channel;
	unsigned char   multiap_ext_cmd;
	unsigned char   multiap_report_fail_assoc;
	unsigned char	multiap_vlan_enable;
	unsigned int	multiap_vlan_id;
	unsigned char	max_tx_power;
	unsigned int	gbwcmode;
	unsigned int	gbwcthrd_tx;
	unsigned int	gbwcthrd_rx;
	struct _GroupBandwidthControl gbwcEntry;
	unsigned char	telco_selected;
	unsigned int	regdomain;
	unsigned int	dfs_regions;
	unsigned int	led_type;
	unsigned int	lifetime;
	unsigned int	manual_priority;/* default: 0xff is off */
	unsigned int	opmode;
	unsigned int	autorate;
	unsigned int	fixrate;
	unsigned int	deny_legacy;
	unsigned int	lgyEncRstrct;
	unsigned int	cts2self;
	unsigned int	coexist;
	unsigned char	ampdu;
	unsigned char	amsdu;
	unsigned int	bypass_deamsdu; /* CONFIG_RTW_BYPASS_DEAMSDU */
	unsigned char	crossband_enable;
	unsigned int	monitor_sta_enabled;
	unsigned int	txbf;
	unsigned int	txbfer;
	unsigned int	txbfee;
	unsigned char	txbf_state;
	unsigned char	txbf_apply;
	unsigned int	txbf_mu;
	unsigned char 	txbf_mu_state;
	unsigned char 	txbf_mu_apply;
	unsigned int	txbf_period;
	unsigned int 	txbf_mu_2ss;
	unsigned int 	txbf_mu_1ss;
	unsigned int 	txbf_tp_limit;
	unsigned int 	txbf_mu_amsdu;
	unsigned int 	txbf_csi_rate;
	unsigned char 	txbf_force_regrp;
	unsigned char 	txbf_offload;
	unsigned char 	txbf_auto_snd;
	unsigned char	roaming_switch;
	unsigned char	roaming_qos;
	unsigned char	fail_ratio;
	unsigned char	retry_ratio;
	unsigned char	RSSIThreshold;
	unsigned char	dfgw_mac[6];
	unsigned char	roaming_enable;
	unsigned int	roaming_start_time;
	unsigned char	roaming_rssi_th1;
	unsigned char	roaming_rssi_th2;
	unsigned int	roaming_wait_time;
	unsigned char	band;
	unsigned char   tpc_tx_power;
	unsigned char	rssi_dump;
	unsigned char	totaltp_dump;
	unsigned char	defer_tx_sched;
	unsigned char	defer_tx_cnt;
	unsigned int	defer_tx_tp;
	unsigned int    defertxtimeout;
	unsigned int	txforce;
	unsigned int	tx_mode_auto;
	/* bit0 (1): Dynamic DL+UL grouping
	   bit0 (1) + bit4 (16): Dynamic DL grouping only
	   bit0 (1) + bit5 (32): Dynamic UL grouping only

	   bit1 (2): UL_fix_mode
	   bit3 (8): DL_fix_mode

	   bit2 (4): UL_fix_mode_by_assoc
	   bit6 (64): DL_fix_mode_by_assoc
	*/
	unsigned char   ofdma_enable;
	unsigned char	ldpc;
	unsigned char	stbc; /* bit0: Rx; bit1: Tx */
	unsigned int	mirror_dump;
	unsigned char	mirror_txch;
	unsigned char   edcca_mode;
	unsigned char   bfrp_mode;
	unsigned char   su_trig_enable;
	unsigned char   en_timer_pfd;
	unsigned char   en_txrpt_pfd;
	unsigned char   set_timer_period;
	unsigned char   timer_period;
	unsigned char   timer_clr_period;

	unsigned char   guest_access;
	unsigned char	block_relay;
	unsigned char   mc2u_disable;
	unsigned char   disable_dfs;
	unsigned char	powerpercent;
	int             power_reference;
	struct _StaBandwidthControl sbwcEntry;
	unsigned int    aclmode;
	unsigned int    aclnum;	/*add for H userland no error*/
	unsigned char	scan_backop;
	unsigned char	scan_backop_div;
	unsigned int	scan_backop_dur;
	unsigned int	qos_enable;
	unsigned int	force_qos;
	/*===client mode===*/
	unsigned char	bssid[MAC_ADDR_LEN];
	unsigned char	connect_ssid[32];
	unsigned short	connect_ssid_len;
	unsigned int	connect_ch;
	unsigned char	crossband_pathReady;
	unsigned char	crossband_assoc;
	unsigned char	crossband_prefer;
	unsigned char	acs;
	unsigned char	autoch_1611_enable;
	unsigned char	autoch_3664157_enable;
	/*=================*/
	unsigned int	amsdu_pps;/* PPS */
	unsigned char	dacs;
	unsigned char	pacs;
	unsigned char	pacs_period;
	unsigned char 	fw_ext;
	unsigned char	sta_asoc_rssi_th;
	unsigned char	hiddenAP;
	/* setting value (not operating value) */
	unsigned char   set_channel;
	unsigned char	set_bwmode;
	unsigned char   set_ch_offset;
	/*=================*/
	unsigned char	mc2u_flood_ctrl;
	unsigned char	mc2u_ipv6_logo;
	unsigned char	pri_mapping_rule;
	unsigned int	txq_limit;
	unsigned char 	force_fw_tx;
	unsigned char 	rssi_ru_dump;
	unsigned char 	sleep_q_max_num;
	unsigned char	ther_dm;
	unsigned char	dbg;
	unsigned char	del_ther;
	unsigned char	ther_hi;
	unsigned char	ther_low;
	unsigned char	ther_max;
	unsigned int	func_off_by_thermal;
	unsigned int	func_off_by_other;
	struct _ParaRecord ap_manual_edca[4];
	struct _ParaRecord sta_manual_edca[4];
	unsigned char	WMM_IE[7];				// WMM STA, WMM IE
	unsigned char	WMM_PARA_IE[24];		// WMM EDCA Parameter IE
	struct _ParaRecord STA_AC_BE_paraRecord;
	struct _ParaRecord STA_AC_BK_paraRecord;
	struct _ParaRecord STA_AC_VI_paraRecord;
	struct _ParaRecord STA_AC_VO_paraRecord;
	unsigned int	bcnint;/* RTW_WKARD_BCNINT_DBG */
	/* RTW_WKARD_CUSTOM_PWRLMT_EN */
	unsigned char   txpwr_lmt_index;
	/* CONFIG_BW160M_EXTREME_THROUGHPUT_RX */
	unsigned int	tcpack_acc;
	unsigned int	tcpack_hithd;
	unsigned int	tcpack_lothd;
	unsigned int	kick_icverr;
	unsigned char	atf;
	unsigned char	fast_leave_thr;
	unsigned int	inactive_timeout;
	unsigned char	trx_path;
	/* CONFIG_ADPTVTY_CONTROL */
	unsigned char	adptvty_en;
	unsigned char	adptvty_try;
	unsigned int	adptvty_th_t;
	unsigned int	adptvty_th_u;
	unsigned char	adptvty_ratio_u;
	unsigned char	dig_opmode;
};

static const struct _ParaRecord rtl_ap_EDCA[] = {
//ACM,AIFSN, ECWmin, ECWmax, TXOplimit
	{0,     7,      4,      10,     0},
	{0,     3,      4,      6,      0},
	{0,     1,      3,      4,      188},
	{0,     1,      2,      3,      102},
	{0,     1,      3,      4,      94},
	{0,     1,      2,      3,      47},
};

static const struct _ParaRecord rtl_sta_EDCA[] = {
//ACM,AIFSN, ECWmin, ECWmax, TXOplimit
	{0,     7,      4,      10,     0},
	{0,     3,      4,      6,     0},
	{0,     1,      3,      4,      188},
	{0,     1,      2,      3,      102},
	{0,     1,      3,      4,      94},
	{0,     1,      2,      3,      47},
};

#endif//_RTW_MIB_H_

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
int set_rtw_mib_default_tbl(_adapter *padapter, struct wifi_mib_priv *rtw_priv_mib);
void core_mib_version(_adapter *padapter, char *extra, u32 oper);
void core_mib_size(_adapter *padapter, char *extra, u32 oper);
#ifdef CONFIG_80211AX_HE
#ifdef CONFIG_RTW_TWT
void core_mib_twt_enable(_adapter *padapter, char *extra, u32 oper);
#endif
void core_mib_fw_tx(_adapter *padapter, char *extra, u32 oper);
#endif
#ifdef CONFIG_RTW_A4_STA
void core_mib_a4_enable(_adapter *padapter, char *extra, u32 oper);
#endif
#ifdef CONFIG_RTW_MULTI_AP
void core_mib_multiap_monitor_mode_disable(_adapter *padapter, char *extra, u32 oper);
void core_mib_multiap_change_channel(_adapter *padapter, char *extra, u32 oper);
void core_mib_multiap_ext_cmd(_adapter *padapter, char *extra, u32 oper);
void core_mib_multiap_bss_type(_adapter *padapter, char *extra, u32 oper);
#endif
void	core_mib_ampdu(_adapter *padapter, char *extra, u32 oper);
void	core_mib_func_off(_adapter *padapter, char *extra, u32 oper);
#ifdef WIFI6_THER_CTRL
void	core_mib_func_off_thermal(_adapter *padapter, char *extra, u32 oper);
#endif
void	core_mib_auto_rate(_adapter *padapter, char *extra, u32 oper);
void	core_mib_fix_rate(_adapter *padapter, char *extra, u32 oper);
void	core_mib_deny_legacy(_adapter *padapter, char *extra, u32 oper);
void	core_mib_amsdu(_adapter *padapter, char *extra, u32 oper);
void	core_mib_rssi_dump(_adapter *padapter, char *extra, u32 oper);
void	core_mib_edcca_mode(_adapter *padapter, char *extra, u32 oper);
void	core_mib_bfrp_mode(_adapter *padapter, char *extra, u32 oper);
void	core_mib_su_trig_enable(_adapter *padapter, char *extra, u32 oper);
void	core_mib_txbf_auto_snd(_adapter *padapter, char *extra, u32 oper);
void	core_mib_en_timer_pfd(_adapter *padapter, char *extra, u32 oper);
void	core_mib_en_txrpt_pfd(_adapter *padapter, char *extra, u32 oper);
void	core_mib_set_timer_period(_adapter *padapter, char *extra, u32 oper);
void	core_mib_timer_period(_adapter *padapter, char *extra, u32 oper);
void	core_mib_timer_clr_period(_adapter *padapter, char *extra, u32 oper);

void	core_mib_txforce(_adapter *padapter, char *extra, u32 oper);
void	core_mib_band(_adapter *padapter, char *extra, u32 oper);
void	core_mib_lifetime(_adapter *padapter, char *extra, u32 oper);
void	core_mib_power_percent(_adapter *padapter, char *extra, u32 oper);
void	core_mib_power_ref(_adapter *padapter, char *extra, u32 oper);
void	core_mib_aclmode(_adapter *padapter, char *extra, u32 oper);
void	core_mib_aclnum(_adapter *padapter, char *extra, u32 oper);
#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
void	core_mib_tx_mode(_adapter *padapter, char *extra, u32 oper);
#endif
void	core_mib_channel(_adapter *padapter, char *extra, u32 oper);
void	core_mib_use40M(_adapter *padapter, char *extra, u32 oper);
void	core_mib_2ndchoffset(_adapter *padapter, char *extra, u32 oper);
void	core_mib_encmode(_adapter *padapter, char *extra, u32 oper);
void	core_mib_psk_enable(_adapter *padapter, char *extra, u32 oper);
void	core_mib_wpa_cipher(_adapter *padapter, char *extra, u32 oper);
void	core_mib_wpa2_cipher(_adapter *padapter, char *extra, u32 oper);
void	core_mib_authtype(_adapter *padapter, char *extra, u32 oper);
void	core_mib_multiap_max_device_reached(_adapter *padapter, char *extra, u32 oper);
#ifdef CONFIG_RTW_MIRROR_DUMP
void	core_mib_mirror_dump(_adapter *padapter, char *extra, u32 oper);
void	core_mib_mirror_txch(_adapter *padapter, char *extra, u32 oper);
#endif
void core_mib_tpc_tx_power(_adapter *padapter, char *extra, u32 oper);
void core_mib_hiddenAP(_adapter *padapter, char *extra, u32 oper);
void core_mib_bcnint(_adapter *padapter, char *extra, u32 oper);
#ifdef RTW_WKARD_CUSTOM_PWRLMT_EN
void core_mib_set_txpwr_lmt_index(_adapter *padapter, char *extra, u32 oper);
#endif
void core_mib_atf(_adapter *padapter, char *extra, u32 oper);
void core_mib_fast_leave(_adapter *padapter, char *extra, u32 oper);
void core_mib_inactive_timeout(_adapter *padapter, char *extra, u32 oper);
#ifdef CONFIG_RTW_MANUAL_EDCA
void core_mib_manual_edca(_adapter *padapter, char *extra, u32 oper);
void dynamic_EDCA_para(_adapter *padapter);
void default_WMM_para(_adapter *padapter);
void init_WMM_Para_Element(_adapter *padapter, unsigned char *temp, unsigned long len);
void get_AP_Qos_Info(_adapter *padapter, unsigned char *temp);
void get_STA_AC_Para_Record(_adapter *padapter, unsigned char *temp);

#define GET_MIB(padapter)		(padapter->registrypriv.wifi_mib)
#define GET_STA_AC_BE_PARA	((GET_MIB(padapter)).STA_AC_BE_paraRecord)
#define GET_STA_AC_BK_PARA	((GET_MIB(padapter)).STA_AC_BK_paraRecord)
#define GET_STA_AC_VI_PARA	((GET_MIB(padapter)).STA_AC_VI_paraRecord)
#define GET_STA_AC_VO_PARA	((GET_MIB(padapter)).STA_AC_VO_paraRecord)
#define GET_WMM_IE		((GET_MIB(padapter)).WMM_IE)
#define GET_WMM_PARA_IE		((GET_MIB(padapter)).WMM_PARA_IE)
#endif
void core_mib_trx_path(_adapter *padapter, char *extra, u32 oper);

void core_mib_set_dig_opmode(_adapter *padapter, char *extra, u32 oper);

#endif

