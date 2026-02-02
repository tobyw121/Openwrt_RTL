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
/*-------------------------------------------------------------------------------

	For type defines and data structure defines

--------------------------------------------------------------------------------*/
#ifndef __DRV_TYPES_H__
#define __DRV_TYPES_H__

#if defined(CONFIG_RX_WIFI_FF_AGG)
#if defined(PLATFORM_LINUX)
#include <net/rtl/aggQ.h>
#endif /* PLATFORM_LINUX */
#endif /* CONFIG_RX_WIFI_FF_AGG */

#include <basic_types.h>
#include <drv_conf.h>
#include <osdep_service.h>
#include <rtw_byteorder.h>
#include <wlan_bssdef.h>
#include <wifi.h>
#include <ieee80211.h>
#ifdef CONFIG_ARP_KEEP_ALIVE
	#include <net/neighbour.h>
	#include <net/arp.h>
#endif

typedef struct _ADAPTER _adapter;
/* connection interface of drv and hal */
#include "../phl/phl_version.h"
#include "../phl/rtw_general_def.h"
#include "../phl/phl_version.h"

#include <rtw_debug.h>
#include <rtw_sta_info.h>
#include <rtw_rf.h>
#include "../core/rtw_chplan.h"

#ifdef CONFIG_WLAN_MANAGER
#include <rtw_wlan_manager.h>
#endif
#ifdef CONFIG_BAND_STEERING
#include <rtw_band_steering.h>
#endif

#ifdef CONFIG_80211N_HT
	#include <rtw_ht.h>
#endif

#ifdef CONFIG_80211AC_VHT
	#include <rtw_vht.h>
#endif

#ifdef CONFIG_RTW_MBO
	#include <rtw_mbo.h>
#endif

#include <rtw_security.h>
//#include <rtw_xmit.h>
#include <xmit_osdep.h>
#ifdef RTW_CORE_PKT_TRACE
#include "rtw_pkt_trace.h"
#endif
#include <rtw_recv.h>
#include <rtw_rm.h>
#ifdef CONFIG_RTW_CORE_RXSC
#include <rtw_recv_shortcut.h>
#endif

#ifdef CONFIG_WFA_OFDMA_Logo_Test
#include <rtw_ru.h>
#endif

#ifdef CONFIG_RTW_A4_STA
#include "../os_dep/linux/rtw_rhashtable.h"
#include <rtw_a4.h>
#endif

#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_WLAN_DE_SUPPORT)
#include <rtw_multi_ap.h>
#endif

#ifdef CONFIG_RTW_COMMON_NETLINK
#include <rtw_netlink.h>
#endif

#ifdef CONFIG_80211AX_HE
	#include <rtw_he.h>
#endif

#ifdef TX_BEAMFORMING
	#include <rtw_beamforming.h>
#endif

#include <recv_osdep.h>
#include <rtw_sreset.h>

/*CONFIG_PHL_ARCH*/
#include "../phl/phl_headers_core.h"
#include "phl_api_tmp.h"
#include "rtw_phl.h"
#include <rtw_cmd.h>

#include <rtw_xmit.h> //move to here for compile

/*GEORGIA_TODO_FIXIT*/
#include "_hal_rate.h"
#include "_hal_api_tmp.h"
#ifdef CONFIG_CORE_TXSC
#include <rtw_xmit_shortcut.h>
#endif

#ifdef CONFIG_CPU_PROFILING
#include "../profiling/cpu_perf.h"
#endif

#include "platform_ops.h"

#include <rtw_qos.h>
#include <rtw_wow.h>
#include <rtw_pwrctrl.h>
#include <rtw_mlme.h>
#include <rtw_phl_cmd.h>
#include <mlme_osdep.h>
#include <rtw_io.h>


#include <rtw_ioctl.h>
#include <rtw_ioctl_set.h>
#include <rtw_ioctl_query.h>
#include <rtw_ioctl_ap.h>
#ifdef CONFIG_WLAN_DE_SUPPORT
#include <rtw_ioctl_de.h>
#endif
#include <rtw_mib.h>
#include "rtw_cfg.h"
#include <osdep_intf.h>
#include <sta_info.h>
#include <rtw_event.h>
#include <rtw_mlme_ext.h>
#include "../core/rtw_dfs.h"
#include <rtw_sec_cam.h>
#include <rtw_mi.h>
#include <rtw_ap.h>
#include <rtw_acs.h>
#ifdef CONFIG_RTW_MESH
#include "../core/mesh/rtw_mesh.h"
#endif
#ifdef CONFIG_WIFI_MONITOR
#include "../core/monitor/rtw_radiotap.h"
#endif

#include <rtw_version.h>

#ifdef CONFIG_PREALLOC_RX_SKB_BUFFER
	#include <rtw_mem.h>
#endif

#include <rtw_p2p.h>

#ifdef CONFIG_TDLS
	#include <rtw_tdls.h>
#endif /* CONFIG_TDLS */

#ifdef CONFIG_WAPI_SUPPORT
	#include <rtw_wapi.h>
#endif /* CONFIG_WAPI_SUPPORT */

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	#include <rtw_ap_wapi.h>
#endif

#ifdef CONFIG_MP_INCLUDED
	#include <rtw_mp.h>
	#include <rtw_efuse.h>
#endif /* CONFIG_MP_INCLUDED */

#ifdef CONFIG_BR_EXT
	#include <rtw_br_ext.h>
#endif /* CONFIG_BR_EXT */

#include <rtw_led.h>

#include <ip.h>
#include <if_ether.h>
#include <ethernet.h>
#include <circ_buf.h>
#include <igmp.h>
#include <ipv6.h>
#include <icmpv6.h>

#include <rtw_android.h>

#include <rtw_btc.h>

#ifdef CONFIG_MCC_MODE
	#include <rtw_mcc.h>
#endif /*CONFIG_MCC_MODE */

#ifdef CONFIG_RTW_TWT
	#include <rtw_twt.h>
#endif /*CONFIG_RTW_TWT */

//#ifdef CONFIG_VW_REFINE
#define RTL_MILISECONDS_TO_JIFFIES(x) (((x)*HZ-1)/1000+1)
//#endif

#define RTL_MS_TO_256US(x) (((x*1000)%256) ? (((x*1000)/256)+1) : ((x*1000)/256))

#define SPEC_DEV_ID_NONE BIT(0)
#define SPEC_DEV_ID_DISABLE_HT BIT(1)
#define SPEC_DEV_ID_ENABLE_PS BIT(2)
#define SPEC_DEV_ID_RF_CONFIG_1T1R BIT(3)
#define SPEC_DEV_ID_RF_CONFIG_2T2R BIT(4)
#define SPEC_DEV_ID_ASSIGN_IFNAME BIT(5)

#if defined(RTW_PHL_TX) || defined(RTW_PHL_RX)
//#define PHLRX_LOG(fmt, args...) printk("phl-rx [%s][%d]"fmt, __FUNCTION__,__LINE__, ## args)
#define PHLRX_LOG		printk("phl-rx [%s][%d] \n", __FUNCTION__, __LINE__);
#define PHLRX_ENTER		printk("phl-rx [%s][%d] ++\n", __FUNCTION__, __LINE__);
#define PHLRX_EXIT		printk("phl-rx [%s][%d] --\n", __FUNCTION__, __LINE__);
#endif

#ifdef CONFIG_RTW_A4_STA
#define adapter_en_a4(adapter) (adapter->a4_enable)
#define adapter_set_en_a4(adapter, en) do { \
		(adapter)->a4_enable = (en) ? 1 : 0; \
		RTW_INFO(FUNC_ADPT_FMT" set use_wds=%d\n", FUNC_ADPT_ARG(adapter), (adapter)->a4_enable); \
	} while (0)
#else
#define adapter_en_a4(adapter) 0
#endif

struct specific_device_id {

	u32		flags;

	u16		idVendor;
	u16		idProduct;

};

struct registry_priv {
	u8	chip_version;
	u8	rfintfs;
	u8	lbkmode;
	u8	hci;
	NDIS_802_11_SSID	ssid;
	u8	network_mode;	/* infra, ad-hoc, auto */
	u8	channel;/* ad-hoc support requirement */
	u8	wireless_mode;/* A, B, G, auto */
	u8	band_type;
	enum rtw_phl_scan_type	scan_mode;/*scan methods - active, passive */
	u8	radio_enable;
	u8	preamble;/* long, short, auto */
	u8	vrtl_carrier_sense;/* Enable, Disable, Auto */
	u8	vcs_type;/* RTS/CTS, CTS-to-self */
	u16	rts_thresh;
	u16  frag_thresh;
	u8	adhoc_tx_pwr;
	u8	soft_ap;
	u8	power_mgnt;
	u8	ips_mode;
	u8	lps_level;
#ifdef CONFIG_LPS_1T1R
	u8	lps_1t1r;
#endif
	u8	lps_chk_by_tp;
#ifdef CONFIG_WOWLAN
	u8	wow_power_mgnt;
	u8	wow_lps_level;
	#ifdef CONFIG_LPS_1T1R
	u8	wow_lps_1t1r;
	#endif
#endif /* CONFIG_WOWLAN */
	u8	smart_ps;
#ifdef CONFIG_WMMPS_STA
	u8	wmm_smart_ps;
#endif /* CONFIG_WMMPS_STA */
	u8   usb_rxagg_mode;
	u8	dynamic_agg_enable;
	u8	long_retry_lmt;
	u8	short_retry_lmt;
	u16	busy_thresh;
	u16	max_bss_cnt;
	u8	ack_policy;
	u8	mp_mode;
#if defined(CONFIG_MP_INCLUDED) && defined(CONFIG_RTW_CUSTOMER_STR)
	u8 mp_customer_str;
#endif
	u8  mp_dm;
	u8	software_encrypt;
	u8	software_decrypt;
#ifdef CONFIG_TX_EARLY_MODE
	u8   early_mode;
#endif
#ifdef CONFIG_NARROWBAND_SUPPORTING
	u8	rtw_nb_config;
#endif
	u8	acm_method;
	/* WMM */
	u8	wmm_enable;
#ifdef CONFIG_WMMPS_STA
	/* uapsd (unscheduled automatic power-save delivery) = a kind of wmmps */
	u8	uapsd_max_sp_len;
	/* BIT0: AC_VO UAPSD, BIT1: AC_VI UAPSD, BIT2: AC_BK UAPSD, BIT3: AC_BE UAPSD */
	u8	uapsd_ac_enable;
#endif /* CONFIG_WMMPS_STA */

	WLAN_BSSID_EX    dev_network;

	u8 tx_bw_mode;
#ifdef CONFIG_AP_MODE
	u8 bmc_tx_rate;
#endif
#ifdef CONFIG_80211N_HT
	u8	ht_enable;
	/* 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160MHz */
	/* 2.4G use bit 0 ~ 3, 5G use bit 4 ~ 7 */
	/* 0x21 means enable 2.4G 40MHz & 5G 80MHz */
	u8	bw_mode;
	u8	ampdu_enable;/* for tx */
	u8	rx_stbc;
	u8	rx_ampdu_amsdu;/* Rx A-MPDU Supports A-MSDU is permitted */
	u8	tx_ampdu_amsdu;/* Tx A-MPDU Supports A-MSDU is permitted */
	u8	tx_quick_addba_req;
	u8 rx_ampdu_sz_limit_by_nss_bw[4][4]; /* 1~4SS, BW20~BW160 */
	/* Short GI support Bit Map */
	/* BIT0 - 20MHz, 1: support, 0: non-support */
	/* BIT1 - 40MHz, 1: support, 0: non-support */
	/* BIT2 - 80MHz, 1: support, 0: non-support */
	/* BIT3 - 160MHz, 1: support, 0: non-support */
	u8	short_gi;
	/* BIT0: Enable VHT LDPC Rx, BIT1: Enable VHT LDPC Tx, BIT4: Enable HT LDPC Rx, BIT5: Enable HT LDPC Tx */
	u8	ldpc_cap;
	/* BIT0: Enable VHT STBC Rx, BIT1: Enable VHT STBC Tx, BIT4: Enable HT STBC Rx, BIT5: Enable HT STBC Tx */
	u8	stbc_cap;
	#if defined(CONFIG_RTW_TX_NPATH_EN)
	u8	tx_npath;
	#endif
	#if defined(CONFIG_RTW_PATH_DIV)
	u8 path_div;
	#endif
	/*
	 * BIT0: Enable VHT SU Beamformer
	 * BIT1: Enable VHT SU Beamformee
	 * BIT2: Enable VHT MU Beamformer, depend on VHT SU Beamformer
	 * BIT3: Enable VHT MU Beamformee, depend on VHT SU Beamformee
	 * BIT4: Enable HT Beamformer
	 * BIT5: Enable HT Beamformee
	 */
	u8	beamform_cap;
	u8	beamformer_rf_num;
	u8	beamformee_rf_num;
	u8  ht_20_40_coexist;
#endif /* CONFIG_80211N_HT */

#ifdef CONFIG_80211AC_VHT
	u8	vht_enable; /* 0:disable, 1:enable, 2:auto */
	u8	ampdu_factor;
	u8 vht_rx_mcs_map[2];
#endif /* CONFIG_80211AC_VHT */

#ifdef CONFIG_80211AX_HE
	u8	he_enable; /* 0:disable, 1:enable, 2:auto */
	u8 	fw_tx;
#ifdef CONFIG_RTW_TWT
	u8  twt_enable;
	u8  twt_dbgmode;
#endif
#endif

	u8	lowrate_two_xmit;

	u8	low_power ;

	u8	wifi_spec;/* !turbo_mode */

	u8 rf_path; /*rf_config*/
	u8 tx_nss;
	u8 rx_nss;

	char alpha2[2];
	u8	channel_plan;
	u8	excl_chs[MAX_CHANNEL_NUM_2G_5G];
#if CONFIG_IEEE80211_BAND_6GHZ
	u8 channel_plan_6g;
	u8 excl_chs_6g[MAX_CHANNEL_NUM_6G];
#endif
	u8	full_ch_in_p2p_handshake; /* 0: reply only softap channel, 1: reply full channel list*/

#ifdef CONFIG_BTC
	u8	btcoex;
	u8	bt_iso;
	u8	bt_sco;
	u8	bt_ampdu;
	u8	ant_num;
	u8	single_ant_path;
#endif
	BOOLEAN	bAcceptAddbaReq;

	u8	antdiv_cfg;
	u8	antdiv_type;
	u8	drv_ant_band_switch;

	u8	switch_usb_mode;

	u8	hw_wps_pbc;/* 0:disable,1:enable */

#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
	char	adaptor_info_caching_file_path[PATH_LENGTH_MAX];
#endif

#ifdef CONFIG_LAYER2_ROAMING
	u8	max_roaming_times; /* the max number driver will try to roaming */
#endif

#ifdef CONFIG_80211D
	u8 enable80211d;
#endif

	u8 ifname[16];
	u8 if2name[16];

	/* for pll reference clock selction */
	u8 pll_ref_clk_sel;

	/* define for tx power adjust */
#if CONFIG_TXPWR_LIMIT
	u8	RegEnableTxPowerLimit;
#endif
	u8	RegEnableTxPowerByRate;

	u8 target_tx_pwr_valid;
	s8 target_tx_pwr_2g[RF_PATH_MAX][RATE_SECTION_NUM];
#if CONFIG_IEEE80211_BAND_5GHZ
	s8 target_tx_pwr_5g[RF_PATH_MAX][RATE_SECTION_NUM - 1];
#endif

	s8	TxBBSwing_2G;
	s8	TxBBSwing_5G;
	u8	AmplifierType_2G;
	u8	AmplifierType_5G;
	u8	bEn_RFE;
	u8	RFE_Type;
	u8	PowerTracking_Type;
	u8	GLNA_Type;
	u8  check_fw_ps;
	u8	RegPwrTrimEnable;

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
	u8	load_phy_file;
	u8	RegDecryptCustomFile;
#endif
#ifdef CONFIG_CONCURRENT_MODE
#ifdef CONFIG_P2P
	u8 sel_p2p_iface;
#endif
#endif
	u8 qos_opt_enable;

	u8 hiq_filter;
	u8 adaptivity_en;
	u8 adaptivity_mode;
	s8 adaptivity_th_l2h_ini;
	s8 adaptivity_th_edcca_hl_diff;

	u8 boffefusemask;
	BOOLEAN bFileMaskEfuse;
	BOOLEAN bBTFileMaskEfuse;
#ifdef CONFIG_RTW_ACS
	u8 acs_auto_scan;
	u8 acs_mode;
#endif

	u32	reg_rxgain_offset_2g;
	u32	reg_rxgain_offset_5gl;
	u32	reg_rxgain_offset_5gm;
	u32	reg_rxgain_offset_5gh;

#ifdef CONFIG_DFS_MASTER
	u8 dfs_region_domain;
#endif

#ifdef CONFIG_MCC_MODE
	u8 en_mcc;
	u32 rtw_mcc_single_tx_cri;
	u32 rtw_mcc_ap_bw20_target_tx_tp;
	u32 rtw_mcc_ap_bw40_target_tx_tp;
	u32 rtw_mcc_ap_bw80_target_tx_tp;
	u32 rtw_mcc_sta_bw20_target_tx_tp;
	u32 rtw_mcc_sta_bw40_target_tx_tp;
	u32 rtw_mcc_sta_bw80_target_tx_tp;
	s8 rtw_mcc_policy_table_idx;
	u8 rtw_mcc_duration;
	u8 rtw_mcc_enable_runtime_duration;
	u8 rtw_mcc_phydm_offload;
#endif /* CONFIG_MCC_MODE */

#ifdef CONFIG_RTW_NAPI
	u8 en_napi;
#ifdef CONFIG_RTW_NAPI_DYNAMIC
	u32 napi_threshold;	/* unit: Mbps */
#endif /* CONFIG_RTW_NAPI_DYNAMIC */
#ifdef CONFIG_RTW_GRO
	u8 en_gro;
#endif /* CONFIG_RTW_GRO */
#endif /* CONFIG_RTW_NAPI */

#ifdef CONFIG_WOWLAN
	u8 wowlan_enable;
	u8 wakeup_event;
	u8 suspend_type;
#endif

#ifdef CONFIG_SUPPORT_TRX_SHARED
	u8 trx_share_mode;
#endif
	u8 check_hw_status;
	u8 wowlan_sta_mix_mode;

#ifdef CONFIG_PCI_HCI
	u32 pci_aspm_config;
	u32 pci_dynamic_aspm_linkctrl;
#endif

	u8 iqk_fw_offload;
	u8 ch_switch_offload;

#ifdef CONFIG_TDLS
	u8 en_tdls;
#endif

#ifdef CONFIG_FW_OFFLOAD_PARAM_INIT
	u8 fw_param_init;
#endif

#ifdef DBG_LA_MODE
	u8 la_mode_en;
#endif
	u32 phydm_ability;
	u32 halrf_ability;
#ifdef CONFIG_TDMADIG
	u8 tdmadig_en;
	u8 tdmadig_mode;
	u8 tdmadig_dynamic;
#endif/*CONFIG_TDMADIG*/
	u8 en_dyn_rrsr;
	u32 set_rrsr_value;
#ifdef CONFIG_RTW_MESH
	u8 peer_alive_based_preq;
#endif

#ifdef RTW_BUSY_DENY_SCAN
	/*
	 * scan_interval_thr means scan interval threshold which is used to
	 * judge if user is in scan page or not.
	 * If scan interval < scan_interval_thr we guess user is in scan page,
	 * and driver won't deny any scan request at that time.
	 * Its default value comes from compiler flag
	 * BUSY_TRAFFIC_SCAN_DENY_PERIOD, and unit is ms.
	 */
	u32 scan_interval_thr;
#endif

	/* TO DO */
	u8 radio_power;
	u8 dot11nAMPDU;
	u8 protectionDisabled;
#if defined(CONFIG_RTW_2G_40M_COEX)
	/* Consider co-existence of AP in secondary channel in 2G band
	 * 0: Don't care other AP. No scan in 2G band before AP started
	 * non-zero: otherwise.
	 */
	u8 coex_2g_40m;
#endif

#ifdef CONFIG_RTW_MANUAL_EDCA
	u8 manual_edca;
#endif
	struct wifi_mib_priv wifi_mib;

	u8 deny_legacy;
#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
	u8 tx_mode;
#endif
	u8 agg_num_buf;


#ifdef CTC_QOS_DSCP
	u8 ctc_dscp;
#endif

#ifdef CONFIG_IOCTL_CFG80211
	u16 roch_min_home_dur; /* min duration for op channel */
	u16 roch_max_away_dur; /* max acceptable away duration for remain on channel */
	u16 roch_extend_dur; /* minimum duration to stay in roch when mgnt tx */
#endif

};

#ifdef CONFIG_WIFI_DIAGNOSIS
#define WIFI_DIAG_ENTRY_NUM 48
#define WIFI_DIAG_TARGET_ENTRY_NUM 4
#define WIFI_DIAG_TIME 		3

enum wifi_diag_mode {
	DIAG_NONE = 0,
	DIAG_UNASSOC_STA = 1,
	DIAG_SPEC_STA = 2,
	DIAG_ADD_STA = 3,
	DIAG_MAX
};

struct wifi_diag_sta_entry {
	u8  channel;
	u8  mac[ETH_ALEN];
	u8  rssi;
	u8  bssid[ETH_ALEN];
	u32 time_stamp;
};

struct wifi_diag_bss_entry {
	u8  bssid[ETH_ALEN];
	u8  ssid[33];
	u8  ssid_length;
	u32 encrypt;  /* open/WEP/WPA/RSN */
	u32 pairwise_cipher;
	u32 group_cipher;
	u32 akm;
};

struct wifi_diag_sta_target_entry {
	u8 mac[ETH_ALEN];
};

struct wifi_diag_obj {
	s32 mode;
	s32 diag_ongoing;
	u8 diag_ch_switch;
	u8 sta_list_num;
	u8 bss_list_num;
	u8 target_list_num;
	struct wifi_diag_sta_entry sta_list[WIFI_DIAG_ENTRY_NUM];
	struct wifi_diag_bss_entry bss_list[WIFI_DIAG_ENTRY_NUM];
	struct wifi_diag_sta_target_entry target_list[WIFI_DIAG_TARGET_ENTRY_NUM];
};
#endif

enum EVENT_WLAN_IND_WITH_INFO {
	EVENT_CTC_ROAMING_STA_RSSI_ALARM = 143,
	EVENT_CTC_ROAMING_BSS_TRANSMIT_RESP = 145,
	EVENT_PROBE_RX_VSIE = 146,
	EVENT_ROAMING_INFO_RPT = 149,
	EVENT_UNASSOC_STA_RPT = 150,
};

#ifdef CONFIG_CTC_FEATURE
typedef struct _CTC_ROAMING_STA_RSSI_ALARM {
	unsigned char event_id;
	unsigned char more_event;
	unsigned char hwaddr[ETH_ALEN];
	unsigned char type;
	unsigned char rssi;
} CTC_ROAMING_STA_RSSI_ALARM;

typedef struct _CTC_ROAMING_BSS_TRANSMIT_RESP {
	unsigned char event_id;
	unsigned char more_event;
	unsigned char hwaddr[ETH_ALEN];
	unsigned char result_code;
} CTC_ROAMING_BSS_TRANSMIT_RESP;
#endif /* CONFIG_CTC_FEATURE */

#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
//#define EVENT_PROBE_RX_VSIE	146
typedef struct _PROBE_VSIE_RECORD {
	unsigned char EventId;
	unsigned char IsMoreEvent;
	unsigned char hwaddr[ETH_ALEN];
	unsigned char data_len;
	unsigned char ie_data[WLAN_MAX_VENDOR_IE_LEN];
} PROBE_VSIE_RECORD;
#endif

#ifdef RTL_LINK_ROAMING_REPORT
typedef struct _ONE_STA {
	u8 	addr[6];
	u8 	rssi;
	u16 rxRate;
	u16 txRate;
	u8	low_qos;
	u8 	channel;
	u8  bw;
	u8  retry_ratio;
	u8  fail_ratio;
	u32  tx_pkts;
	u8 	 wlanid;
	u8 	 used;
}ONE_STA;
typedef struct _ROAMING_INFO_RPT {
	u8 eventID;
	u32  sta_num;
	ONE_STA  sta_info[MACID_NUM_SW_LIMIT];
}ROAMING_INFO_RPT;
#endif

/* For registry parameters */
#define RGTRY_OFT(field) ((u32)FIELD_OFFSET(struct registry_priv, field))
#define RGTRY_SZ(field)   sizeof(((struct registry_priv *) 0)->field)

#define WOWLAN_IS_STA_MIX_MODE(_Adapter)	(_Adapter->registrypriv.wowlan_sta_mix_mode)
#define BSSID_OFT(field) ((u32)FIELD_OFFSET(WLAN_BSSID_EX, field))
#define BSSID_SZ(field)   sizeof(((PWLAN_BSSID_EX) 0)->field)

#define BW_MODE_2G(bw_mode) ((bw_mode) & 0x0F)
#define BW_MODE_5G(bw_mode) ((bw_mode) >> 4)
#ifdef CONFIG_80211N_HT
#define REGSTY_BW_2G(regsty) BW_MODE_2G((regsty)->bw_mode)
#define REGSTY_BW_5G(regsty) BW_MODE_5G((regsty)->bw_mode)
#else
#define REGSTY_BW_2G(regsty) CHANNEL_WIDTH_20
#define REGSTY_BW_5G(regsty) CHANNEL_WIDTH_20
#endif
#define REGSTY_IS_BW_2G_SUPPORT(regsty, bw) (REGSTY_BW_2G((regsty)) >= (bw))
#define REGSTY_IS_BW_5G_SUPPORT(regsty, bw) (REGSTY_BW_5G((regsty)) >= (bw))

#ifdef CONFIG_80211AC_VHT
#define REGSTY_IS_11AC_ENABLE(regsty) ((regsty)->vht_enable != 0)
#define REGSTY_IS_11AC_AUTO(regsty) ((regsty)->vht_enable == 2)
#define REGSTY_IS_11AC_24G_ENABLE(regsty) ((regsty)->vht_24g_enable != 0)
#else
#define REGSTY_IS_11AC_ENABLE(regsty) 0
#define REGSTY_IS_11AC_AUTO(regsty) 0
#define REGSTY_IS_11AC_24G_ENABLE(regsty) 0
#endif

#define REGSTY_IS_11AX_ENABLE(regsty) ((regsty)->he_enable != 0)
#define REGSTY_IS_11AX_AUTO(regsty) ((regsty)->he_enable == 2)

#ifdef CONFIG_SDIO_HCI
	#include <drv_types_sdio.h>
#endif
#ifdef CONFIG_GSPI_HCI
	#include <drv_types_gspi.h>
#endif
#ifdef CONFIG_PCI_HCI
	#include <drv_types_pci.h>
#endif
#ifdef CONFIG_USB_HCI
	#include <drv_types_usb.h>
#endif

#include <rtw_trx.h>

#ifdef CONFIG_CONCURRENT_MODE
	#define is_primary_adapter(adapter) (adapter->adapter_type == PRIMARY_ADAPTER)
	#define is_vir_adapter(adapter) (adapter->adapter_type == VIRTUAL_ADAPTER)
#else
	#define is_primary_adapter(adapter) (1)
	#define is_vir_adapter(adapter) (0)
#endif
#define GET_PRIMARY_ADAPTER(padapter) (((_adapter *)padapter)->dvobj->padapters[IFACE_ID0])
#define GET_IFACE_NUMS(padapter) (((_adapter *)padapter)->dvobj->iface_nums)
#define GET_ADAPTER(padapter, iface_id) (((_adapter *)padapter)->dvobj->padapters[iface_id])


#ifdef RTW_PHL_TX

#if 1
#define	PHLTX_ENTER
#define	PHLTX_LOG
#define	PHLTX_EXIT
#define	PHLTX_ERR
#else
#define	PHLTX_ENTER RTW_PRINT("[%s][%d] ++\n", __FUNCTION__, __LINE__)
#define	PHLTX_LOG 	RTW_PRINT("[%s][%d]\n", __FUNCTION__, __LINE__)
#define	PHLTX_EXIT 	RTW_PRINT("[%s][%d] --\n", __FUNCTION__, __LINE__)

#define	PHLTX_ERR 	RTW_PRINT("PHLTX_ERR [%s][%d]\n", __FUNCTION__, __LINE__)
#endif


#define SZ_TXREQ 	(sizeof(struct rtw_xmit_req))
#define SZ_HEAD_BUF	100
#define SZ_TAIL_BUF	30

#define NUM_PKT_LIST_PER_TXREQ	 (MAX_TXSC_SKB_NUM + 1)

#define SZ_PKT_LIST (sizeof(struct rtw_pkt_buf_list))

#ifdef PLATFORM_ECOS
#define SZ_TX_RING 		(SZ_TXREQ+SZ_HEAD_BUF+SZ_TAIL_BUF+(SZ_PKT_LIST*NUM_PKT_LIST_PER_TXREQ) + 3 * SMP_CACHE_BYTES)
#define SZ_MGT_RING		(SZ_TXREQ + SZ_PKT_LIST + SMP_CACHE_BYTES)/* MGT_TXREQ_QMGT */
#else
#define SZ_TX_RING 		(SZ_TXREQ+SZ_HEAD_BUF+SZ_TAIL_BUF+(SZ_PKT_LIST*NUM_PKT_LIST_PER_TXREQ)+12)
#define SZ_MGT_RING		(SZ_TXREQ + SZ_PKT_LIST)/* MGT_TXREQ_QMGT */
#endif

#ifdef CONFIG_VW_REFINE
#define MAX_TX_RING_NUM_2G 	1024
#define MAX_TX_RING_NUM_5G	2048
#else
#define MAX_TX_RING_NUM_2G 	1024  //2048
#define MAX_TX_RING_NUM_5G	1024
#endif
#endif

#if defined(CONFIG_RX_WIFI_FF_AGG)
#undef MAX_TX_RING_NUM_5G
#define MAX_TX_RING_NUM_5G	8192
#endif /* CONFIG_RX_WIFI_FF_AGG */

enum _IFACE_ID {
	IFACE_ID0, /*PRIMARY_ADAPTER*/
	IFACE_ID1,
	IFACE_ID2,
	IFACE_ID3,
	IFACE_ID4,
	IFACE_ID5,
	IFACE_ID6,
	IFACE_ID7,
	IFACE_ID_MAX,
};

#define VIF_START_ID	1

#ifdef CONFIG_DBG_COUNTER
struct rx_logs {
	u32 intf_rx;
	u32 intf_rx_err_recvframe;
	u32 intf_rx_err_skb;
	u32 intf_rx_report;
	u32 core_rx_prcss;
	u32 core_rx_prcss_succ;
	u32 core_rx_allc_recvf_err;
	u32 core_rx;
	u32 core_rx_err_recvframe;
	u32 core_rx_req;
	u32 core_rx_icv_err;
	u32 core_rx_crc_err;
	u32 core_rx_rxsc_done;
	u32 core_rx_next;
	u32 core_rx_err_stop;
	u32 core_rx_pre;
	u32 core_rx_pre_ver_err;
	u32 core_rx_pre_mgmt;
	u32 core_rx_pre_mgmt_err_80211w;
	u32 core_rx_pre_mgmt_err;
	u32 core_rx_pre_ctrl;
	u32 core_rx_pre_ctrl_err;
	u32 core_rx_pre_data;
	u32 core_rx_pre_data_wapi_seq_err;
	u32 core_rx_pre_data_wapi_key_err;
	u32 core_rx_pre_data_handled;
	u32 core_rx_pre_data_err;
	u32 core_rx_pre_data_unknown;
	u32 core_rx_pre_unknown;
	u32 core_rx_amsdu_cut;
	u32 core_rx_amsdu_cut_hdr_conv;
	u32 core_rx_hdr_conv;
	u32 core_rx_enqueue;
	u32 core_rx_dequeue;
	u32 core_rx_post;
	u32 core_rx_post_decrypt;
	u32 core_rx_post_decrypt_wep;
	u32 core_rx_post_decrypt_tkip;
	u32 core_rx_post_decrypt_aes;
	u32 core_rx_post_decrypt_wapi;
	u32 core_rx_post_decrypt_gcmp;
	u32 core_rx_post_decrypt_hw;
	u32 core_rx_post_decrypt_unknown;
	u32 core_rx_post_decrypt_err;
	u32 core_rx_post_defrag_err;
	u32 core_rx_post_portctrl_err;
	u32 core_rx_post_indicate;
	u32 core_rx_post_indicate_in_oder;
	u32 core_rx_post_indicate_reoder;
	u32 core_rx_post_indicate_err;
	#ifdef CONFIG_RTW_DEBUG_RX_CACHE
	u32 core_rx_ta_no_sta;
	#endif /* CONFIG_RTW_DEBUG_RX_CACHE */
	u32 os_indicate;
	u32 os_indicate_ap_mcast;
	u32 os_indicate_ap_mcast_drop;
	u32 os_indicate_ap_forward;
	u32 os_indicate_ap_self;
	u32 os_indicate_err;
	u32 os_netif_ok;
	u32 os_netif_err;
};

#ifdef CONFIG_DBG_HNDSK_MGMT
enum wifi_hndsk_cate {
	HNDSK_AUTH_PKT = 0,
	HNDSK_ASSOC_RSP_PKT = 1,
	HNDSK_EAPOL_4_1_PKT = 2,
	HNDSK_EAPOL_4_3_PKT = 3,
	HNDSK_MGMT_MAX
};
#endif

struct tx_logs {
	u32 os_tx;
	u32 os_uc_ip_pri[8];
	u32 os_uc_ip_pri_drop[8];
	u32 os_tx_bmc;
	u32 os_tx_uc;
	u32 os_tx_tcp;
	u32 os_tx_udp;
	u32 os_tx_normal;
	u32 os_tx_drop;
	u32 os_tx_err_up;
	u32 os_tx_err_xmit;
	u32 os_tx_m2u;
	u32 os_tx_m2u_ignore_fw_linked;
	u32 os_tx_m2u_ignore_self;
	u32 os_tx_m2u_entry;
	u32 os_tx_m2u_entry_err_xmit;
	u32 os_tx_m2u_entry_err_skb;
	u32 os_tx_m2u_stop;
	u32 os_tx_m2u_ps_enqueue;
	u32 os_tx_m2u_ps_drop;
	u32 core_tx;
	u32 core_tx_err_drop;
	u32 core_tx_ex_err_drop;
	u32 core_tx_err_pxmitframe;
	u32 core_tx_err_brtx;
	u32 core_tx_err_prepare_phl;
#ifdef CONFIG_CORE_TXSC
	u32 core_tx_txsc;
	u32 core_tx_err_txsc;
	u32 core_tx_direct_phl;
#ifdef CONFIG_TXSC_AMSDU
	u32 core_tx_txsc_amsdu_timeout_err;
#ifdef CONFIG_VW_REFINE
	u32 core_vw_entry;
	u32 core_vw_swq_enq;
	u32 core_vw_swq_enq_fail;
	u32 core_vw_txsc;
	u32 core_vw_txsc_fail;
	u32 core_vw_txsc2;
	u32 core_vw_slow;
	u32 core_vw_amsdu_enq_merg;
	u32 core_vw_amsdu_dir;
	u32 core_vw_amsdu_enq;
	u32 core_vw_amsdu_dnq;
	u32 core_vw_amsdu_dnq1;
	u32 core_vw_amsdu_dnq2;
	u32 core_vw_amsdu_timeout;
	u32 core_vw_add_tx_req;

	u32 core_vw_test0;
	u32 core_vw_test1;
	u32 core_vw_test2;
	u32 core_vw_test3;
	u32 core_vw_test4;
	u32 core_vw_test5;
	u32 core_vw_test6;
	u32 core_vw_test7;
	u32 core_vw_test8;
	u32 core_vw_test9;
	u32 core_vw_testa;

	u32 core_vw_testb;
	u32 core_vw_testc;
	u32 core_vw_testd;
	u32 core_vw_teste;
#endif
#endif
#endif
	u32 core_tx_upd_attrib;
	u32 core_tx_upd_attrib_adhoc;
	u32 core_tx_upd_attrib_sta;
	u32 core_tx_upd_attrib_ap;
	u32 core_tx_upd_attrib_unknown;
	u32 core_tx_upd_attrib_dhcp;
	u32 core_tx_upd_attrib_icmp;
	u32 core_tx_upd_attrib_active;
	u32 core_tx_upd_attrib_err_ucast_sta;
	u32 core_tx_upd_attrib_err_ucast_ap_link;
	u32 core_tx_upd_attrib_err_sta;
	u32 core_tx_upd_attrib_err_link;
	u32 core_tx_upd_attrib_err_sec;
	u32 core_tx_ap_enqueue_warn_fwstate;
	u32 core_tx_ap_enqueue_warn_sta;
	u32 core_tx_ap_enqueue_warn_nosta;
	u32 core_tx_ap_enqueue_warn_link;
	u32 core_tx_ap_enqueue_warn_trigger;
	u32 core_tx_ap_enqueue_mcast;
	u32 core_tx_ap_enqueue_ucast;
	u32 core_tx_ps_enqueue;
	u32 core_tx_ps_drop;
	u32 core_tx_swq_enq;
	u32 core_tx_swq_deq;
	u32 core_tx_add_req_data;
	u32 core_tx_add_req_mgnt;
	u32 core_tx_err_add_req;
	/* u32 intf_tx; */
	/* u32 intf_tx_pending_ac; */
	/* u32 intf_tx_pending_fw_under_survey; */
	/* u32 intf_tx_pending_fw_under_linking; */
	/* u32 intf_tx_pending_xmitbuf; */
	/* u32 intf_tx_enqueue; */
	u32 core_tx_enqueue;
	u32 core_tx_enqueue_class;
	u32 core_tx_enqueue_class_err_sta;
	u32 core_tx_enqueue_class_err_nosta;
	u32 core_tx_enqueue_class_err_fwlink;
	/* u32 intf_tx_direct; */
	/* u32 intf_tx_direct_err_coalesce; */
	/* u32 intf_tx_dequeue; */
	/* u32 intf_tx_dequeue_err_coalesce; */
	/* u32 intf_tx_dump_xframe; */
	/* u32 intf_tx_dump_xframe_err_txdesc; */
	/* u32 intf_tx_dump_xframe_err_port; */
	#ifdef CONFIG_RTW_DEBUG_SCTX_ALLOC
	u32 sctx_alloc;
	u32 sctx_free;
	#endif /* CONFIG_RTW_DEBUG_SCTX_ALLOC */
	#ifdef CONFIG_DBG_HNDSK_MGMT
	u32 core_hndsk_tx_cnt[HNDSK_MGMT_MAX];
	u32 core_hndsk_tx_ok_cnt[HNDSK_MGMT_MAX];
	u32 core_hndsk_tx_fail_cnt[HNDSK_MGMT_MAX];
	#endif
};

struct int_logs {
	u32 all;
	u32 err;
	u32 tbdok;
	u32 tbder;
	u32 bcnderr;
	u32 bcndma;
	u32 bcndma_e;
	u32 rx;
	u32 rx_rdu;
	u32 rx_fovw;
	u32 txfovw;
	u32 mgntok;
	u32 highdok;
	u32 bkdok;
	u32 bedok;
	u32 vidok;
	u32 vodok;
};

#endif /* CONFIG_DBG_COUNTER */

struct debug_priv {
	u32 dbg_sdio_free_irq_error_cnt;
	u32 dbg_sdio_alloc_irq_error_cnt;
	u32 dbg_sdio_free_irq_cnt;
	u32 dbg_sdio_alloc_irq_cnt;
	u32 dbg_sdio_deinit_error_cnt;
	u32 dbg_sdio_init_error_cnt;
	u32 dbg_suspend_error_cnt;
	u32 dbg_suspend_cnt;
	u32 dbg_resume_cnt;
	u32 dbg_resume_error_cnt;
	u32 dbg_deinit_fail_cnt;
	u32 dbg_carddisable_cnt;
	u32 dbg_carddisable_error_cnt;
	u32 dbg_ps_insuspend_cnt;
	u32 dbg_dev_unload_inIPS_cnt;
	u32 dbg_wow_leave_ps_fail_cnt;
	u32 dbg_scan_pwr_state_cnt;
	u32 dbg_downloadfw_pwr_state_cnt;
	u32 dbg_fw_read_ps_state_fail_cnt;
	u32 dbg_leave_ips_fail_cnt;
	u32 dbg_leave_lps_fail_cnt;
	u32 dbg_h2c_leave32k_fail_cnt;
	u32 dbg_diswow_dload_fw_fail_cnt;
	u32 dbg_enwow_dload_fw_fail_cnt;
	u32 dbg_ips_drvopen_fail_cnt;
	u32 dbg_poll_fail_cnt;
	u32 dbg_rpwm_toogle_cnt;
	u32 dbg_rpwm_timeout_fail_cnt;
	u32 dbg_sreset_cnt;
	u32 dbg_fw_mem_dl_error_cnt;
	u64 dbg_rx_fifo_last_overflow;
	u64 dbg_rx_fifo_curr_overflow;
	u64 dbg_rx_fifo_diff_overflow;
};

struct rtw_traffic_statistics {
	/* tx statistics */
	u64	tx_bytes;
	u64	tx_pkts;
	u64	tx_drop;
	u64	cur_tx_bytes;
	u64	last_tx_bytes;
	u32	cur_tx_tp; /* Tx throughput in Mbps. */

	/* rx statistics */
	u64	rx_bytes;
	u64	rx_pkts;
	u64	rx_drop;
	u64	cur_rx_bytes;
	u64	last_rx_bytes;
	u32	cur_rx_tp; /* Rx throughput in Mbps. */
};

struct rtw_hw_statistics {
	u32 fa_cnt;
	u32 last_fa_cnt;
	u32 cur_fa_cnt;
	u32 cur_cca_cnt;
};

#define SEC_CAP_CHK_EXTRA_SEC	BIT1 /* 256 bit */

#define KEY_FMT "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define KEY_ARG(x) ((u8 *)(x))[0], ((u8 *)(x))[1], ((u8 *)(x))[2], ((u8 *)(x))[3], ((u8 *)(x))[4], ((u8 *)(x))[5], \
	((u8 *)(x))[6], ((u8 *)(x))[7], ((u8 *)(x))[8], ((u8 *)(x))[9], ((u8 *)(x))[10], ((u8 *)(x))[11], \
	((u8 *)(x))[12], ((u8 *)(x))[13], ((u8 *)(x))[14], ((u8 *)(x))[15]



/* used for rf_ctl_t.rate_bmp_cck_ofdm */
#define RATE_BMP_CCK		0x000F
#define RATE_BMP_OFDM		0xFFF0
#define RATE_BMP_HAS_CCK(_bmp_cck_ofdm)		(_bmp_cck_ofdm & RATE_BMP_CCK)
#define RATE_BMP_HAS_OFDM(_bmp_cck_ofdm)	(_bmp_cck_ofdm & RATE_BMP_OFDM)
#define RATE_BMP_GET_CCK(_bmp_cck_ofdm)		(_bmp_cck_ofdm & RATE_BMP_CCK)
#define RATE_BMP_GET_OFDM(_bmp_cck_ofdm)	((_bmp_cck_ofdm & RATE_BMP_OFDM) >> 4)

/* used for rf_ctl_t.rate_bmp_ht_by_bw */
#define RATE_BMP_HT_1SS		0x000000FF
#define RATE_BMP_HT_2SS		0x0000FF00
#define RATE_BMP_HT_3SS		0x00FF0000
#define RATE_BMP_HT_4SS		0xFF000000
#define RATE_BMP_HAS_HT_1SS(_bmp_ht)		(_bmp_ht & RATE_BMP_HT_1SS)
#define RATE_BMP_HAS_HT_2SS(_bmp_ht)		(_bmp_ht & RATE_BMP_HT_2SS)
#define RATE_BMP_HAS_HT_3SS(_bmp_ht)		(_bmp_ht & RATE_BMP_HT_3SS)
#define RATE_BMP_HAS_HT_4SS(_bmp_ht)		(_bmp_ht & RATE_BMP_HT_4SS)
#define RATE_BMP_GET_HT_1SS(_bmp_ht)		(_bmp_ht & RATE_BMP_HT_1SS)
#define RATE_BMP_GET_HT_2SS(_bmp_ht)		((_bmp_ht & RATE_BMP_HT_2SS) >> 8)
#define RATE_BMP_GET_HT_3SS(_bmp_ht)		((_bmp_ht & RATE_BMP_HT_3SS) >> 16)
#define RATE_BMP_GET_HT_4SS(_bmp_ht)		((_bmp_ht & RATE_BMP_HT_4SS) >> 24)

/* used for rf_ctl_t.rate_bmp_vht_by_bw */
#define RATE_BMP_VHT_1SS	0x00000003FF
#define RATE_BMP_VHT_2SS	0x00000FFC00
#define RATE_BMP_VHT_3SS	0x003FF00000
#define RATE_BMP_VHT_4SS	0xFFC0000000
#define RATE_BMP_HAS_VHT_1SS(_bmp_vht)		(_bmp_vht & RATE_BMP_VHT_1SS)
#define RATE_BMP_HAS_VHT_2SS(_bmp_vht)		(_bmp_vht & RATE_BMP_VHT_2SS)
#define RATE_BMP_HAS_VHT_3SS(_bmp_vht)		(_bmp_vht & RATE_BMP_VHT_3SS)
#define RATE_BMP_HAS_VHT_4SS(_bmp_vht)		(_bmp_vht & RATE_BMP_VHT_4SS)
#define RATE_BMP_GET_VHT_1SS(_bmp_vht)		((u16)(_bmp_vht & RATE_BMP_VHT_1SS))
#define RATE_BMP_GET_VHT_2SS(_bmp_vht)		((u16)((_bmp_vht & RATE_BMP_VHT_2SS) >> 10))
#define RATE_BMP_GET_VHT_3SS(_bmp_vht)		((u16)((_bmp_vht & RATE_BMP_VHT_3SS) >> 20))
#define RATE_BMP_GET_VHT_4SS(_bmp_vht)		((u16)((_bmp_vht & RATE_BMP_VHT_4SS) >> 30))

#define TXPWR_LMT_REF_VHT_FROM_HT	BIT0
#define TXPWR_LMT_REF_HT_FROM_VHT	BIT1

#define TXPWR_LMT_HAS_CCK_1T	BIT0
#define TXPWR_LMT_HAS_CCK_2T	BIT1
#define TXPWR_LMT_HAS_CCK_3T	BIT2
#define TXPWR_LMT_HAS_CCK_4T	BIT3
#define TXPWR_LMT_HAS_OFDM_1T	BIT4
#define TXPWR_LMT_HAS_OFDM_2T	BIT5
#define TXPWR_LMT_HAS_OFDM_3T	BIT6
#define TXPWR_LMT_HAS_OFDM_4T	BIT7

#define OFFCHS_NONE			0
#define OFFCHS_LEAVING_OP	1
#define OFFCHS_LEAVE_OP		2
#define OFFCHS_BACKING_OP	3

struct rf_ctl_t {
	struct country_chplan *country_ent;
	u8 ChannelPlan;
#if CONFIG_IEEE80211_BAND_6GHZ
	u8 chplan_6g;
#endif
	u8 max_chan_nums;
	RT_CHANNEL_INFO channel_set[MAX_CHANNEL_NUM];
	struct op_class_pref_t **spt_op_class_ch;
	u8 cap_spt_op_class_num;
	u8 reg_spt_op_class_num;
	u8 cur_spt_op_class_num;
	struct p2p_channels channel_list;

	_mutex offch_mutex;
	u8 offch_state;

	/* used for debug or by tx power limit */
	u16 rate_bmp_cck_ofdm;		/* 20MHz */
	u32 rate_bmp_ht_by_bw[2];	/* 20MHz, 40MHz. 4SS supported */
	u64 rate_bmp_vht_by_bw[4];	/* 20MHz, 40MHz, 80MHz, 160MHz. 4SS supported */

#if CONFIG_TXPWR_LIMIT
	u8 highest_ht_rate_bw_bmp;
	u8 highest_vht_rate_bw_bmp;

	_mutex txpwr_lmt_mutex;
	_list reg_exc_list;
	u8 regd_exc_num;
	_list txpwr_lmt_list;
	u8 txpwr_lmt_num;
	const char *txpwr_lmt_name;

	u8 txpwr_lmt_2g_cck_ofdm_state;
	#if CONFIG_IEEE80211_BAND_5GHZ
	u8 txpwr_lmt_5g_cck_ofdm_state;
	u8 txpwr_lmt_5g_20_40_ref;
	#endif
#endif

	bool ch_sel_within_same_band;

	u8 adaptivity_en; /* runtime status, hook to phydm */
	u8 edcca_mode_2g;
#if CONFIG_IEEE80211_BAND_5GHZ
	u8 edcca_mode_5g;
#endif
#if CONFIG_IEEE80211_BAND_6GHZ
	u8 edcca_mode_6g;
#endif

#if CONFIG_DFS
	u8 csa_ch;
	u8 csa_cntdown;
	u8 csa_set_ie;

#ifdef CONFIG_DFS_MASTER
	u8 dfs_region_domain;
	_timer radar_detect_timer;
	bool radar_detect_by_others;
	u8 radar_detect_enabled;
	bool radar_detected;

	u8 radar_detect_ch;
	u8 radar_detect_bw;
	u8 radar_detect_offset;

	systime cac_start_time;
	systime cac_end_time;
	u8 cac_force_stop;

#if CONFIG_DFS_SLAVE_WITH_RADAR_DETECT
	u8 dfs_slave_with_rd;
#endif
	u8 dfs_ch_sel_e_flags;
	u8 dfs_ch_sel_d_flags;

	u8 dbg_dfs_fake_radar_detect_cnt;
	u8 dbg_dfs_radar_detect_trigger_non;
	u8 dbg_dfs_choose_dfs_ch_first;
#if (CONFIG_RTW_MULTI_AP_DFS_EN) && (CONFIG_DFS)
	u8 radar_detect_map_member;
	bool map_dfs_wo_cac;
#endif
#endif /* CONFIG_DFS_MASTER */
#endif /* CONFIG_DFS */
	struct rtw_trx_path_param trx_path;
};

struct wow_ctl_t {
	u8 wow_cap;
};

#define WOW_CAP_TKIP_OL BIT0

#ifdef CONFIG_USB_HCI

struct trx_urb_buf_q {
	_queue free_urb_buf_queue;
	u8 *alloc_urb_buf;
	u8 *urb_buf;
	uint free_urb_buf_cnt;
};

struct data_urb {
	_list	list;
	struct urb *urb;
	u8 bulk_id;
	u8 minlen;
};

#endif

struct trx_data_buf_q {
	_queue free_data_buf_queue;
	u8 *alloc_data_buf;
	u8 *data_buf;
	uint free_data_buf_cnt;
};


struct lite_data_buf {
	_list	list;
	struct dvobj_priv *dvobj;
	u16 buf_tag;
	u8 *pbuf;
	u8 *phl_buf_ptr; /*point to phl rtw_usb_buf from phl*/
#ifdef CONFIG_USB_HCI
	struct data_urb *dataurb;
#endif
	struct submit_ctx *sctx;

};

#ifdef CONFIG_DRV_FAKE_AP
struct fake_ap {
	struct sk_buff_head rxq;	/* RX queue */
	_workitem work;
	struct rtw_timer_list bcn_timer;
};
#endif /* CONFIG_DRV_FAKE_AP */


#define SMP_CALL_STATE_SCHED	0	/* scheduled for execution */

struct rtw_smp_call_data {
	call_single_data_t csd;
	_tasklet tasklet;
	unsigned long state;
	u8 cpuid;
};

#ifdef CONFIG_SMP_NETIF_RX
#define MAX_NETIF_RX_RING_ENTRY 4096	/* 2's power */
struct rtw_netif_rx_ring {
	struct sk_buff *entry[MAX_NETIF_RX_RING_ENTRY];
	u16 write_idx;
	u16 read_idx;
	u16 full;
};
#endif

#ifdef CONFIG_SMP_PHL_RX_RECYCLE
#define MAX_RX_RECYCLE_RING_ENTRY 4096
struct rtw_rx_recycle_ring {
	void *entry[MAX_RX_RECYCLE_RING_ENTRY];
	u16 write_idx;
	u16 read_idx;
	u16 full;
	u16 max_time;
	_lock lock;
};
#endif

struct mntr_result {
	u8 clm_ratio;
};

struct atm_stainfo {
	u8	hwaddr[MACADDRLEN];
	u32	atm_statime;
};

/*device object*/
struct dvobj_priv {
	/*-------- below is common data --------*/
	ATOMIC_T bSurpriseRemoved;
	ATOMIC_T bDriverStopped;

	s32	processing_dev_remove;

	_mutex hw_init_mutex;
	_mutex ioctrl_mutex;
	_mutex setch_mutex;
	_mutex setbw_mutex;
	_mutex rf_read_reg_mutex;

	_adapter *padapters[CONFIG_IFACE_NUMBER];/*IFACE_ID_MAX*/
	u8 virtual_iface_num;/*from registary*/
	u8 iface_nums; /* total number of ifaces used runtime */
	struct mi_state iface_state;

#ifdef CONFIG_SWQ_SKB_ARRAY
	_queue skb_q;
#endif

	u8 tx_mode;
	u8 force_tx_cmd_num;
	_queue ps_trigger_sta_queue;
	_queue tx_pending_sta_queue;
	#ifdef CONFIG_ONE_TXQ
	ATOMIC_T txq_total_len;
	u32 txq_full_drop;
	u32 txq_timeout_seq;
	u32 txq_timeout_seq_last;
	u32 txq_pkt_trigger;
	u32 txq_timeout_trigger;
	_tasklet txq_tasklet;
	u32 txq_max_enq_len;
	u32 txq_max_agg_num;
	u16 txq_hw_timeout;		/* unit of us */
	u32 txq_pkt_timeout;	/* unit of ms */
	u32 txq_max_serv_time;	/* unit of ms */
	u32 txq_serv_timeout;
	u32 txq_serv_group_exp;
	u16 txq_amsdu_merge;	/* for pkt size < 128B */
	u16 txq_tcpack_merge;
	u32 txq_ts_factor;
	u32 txq_deq_factor;
	u32 txq_deq_loop;
	u32 txq_debug;
	u32 txq_service;
	u32 txq_timeout_avg;
	systime atm_last_time;
	#endif /* CONFIG_ONE_TXQ */

	//#if !defined(CONFIG_ONE_TXQ)
	_tasklet ps_trigger_tasklet;
	//#endif

	#ifdef AP_NEIGHBOR_INFO
	struct ap_neighbor_info ap_neighbor;
	#endif

	#ifdef CONFIG_WIFI_DIAGNOSIS
	struct wifi_diag_obj wifi_diag;
	#endif

#ifdef CONFIG_RTW_MULTI_DEV_MULTI_BAND
	u8 dev_id;
#endif /* CONFIG_RTW_MULTI_DEV_MULTI_BAND */

	enum rtl_ic_id ic_id;
	enum rtw_hci_type interface_type;/*USB,SDIO,SPI,PCI*/

	/*CONFIG_PHL_ARCH*/
	void *phl;
	struct rtw_phl_com_t *phl_com;
	#ifdef DBG_PHL_MEM_ALLOC
	ATOMIC_T phl_mem;
	#endif

	struct rf_ctl_t rf_ctl;
	/* move to phl */
	/* struct macid_ctl_t macid_ctl; *//*shared HW resource*/
	struct cam_ctl_t cam_ctl;/*sec-cam shared HW resource*/
	struct sec_cam_ent cam_cache[SEC_CAM_ENT_NUM_SW_LIMIT];
	struct wow_ctl_t wow_ctl;

	#ifdef CONFIG_RTW_OS_HANDLER_EXT
	struct rtw_os_handler *handlers[RTW_HANDLER_NUM_MAX];
	#endif /* CONFIG_RTW_OS_HANDLER_EXT */

	/****** Band info may be x 2*********/
	unsigned char	oper_channel; /* saved channel info when call set_channel_bw */
	unsigned char	oper_bwmode;
	unsigned char	oper_ch_offset;/* PRIME_CHNL_OFFSET */
	systime 		on_oper_ch_time;

	/****** hal dep info*********/


	ATOMIC_T continual_io_error;
	ATOMIC_T disable_func;

	u8 xmit_block;
	_lock xmit_block_lock;

	struct pwrctrl_priv pwrctl_priv;

	struct cmd_priv	cmdpriv;

	#ifdef CONFIG_RTW_SW_LED
	struct led_priv ledpriv;
	#endif

	#ifdef CONFIG_RTW_ACS
	struct acs_priv acspriv;
	#endif

	struct rtw_traffic_statistics	traffic_stat;

	struct rtw_hw_statistics hw_stat;

	#if defined(PLATFORM_LINUX) || defined(PLATFORM_ECOS)
	_thread_hdl_ rtnl_lock_holder;

	#if defined(CONFIG_IOCTL_CFG80211)
	struct wiphy *wiphy;
	#endif

#ifdef CONFIG_SMP_NETIF_RX
	_tasklet netif_rx_task;
	struct rtw_netif_rx_ring netif_rx_ring;
#endif
#ifdef CONFIG_SMP_PHL_RX_RECYCLE
	_tasklet rx_recycle_task;
	struct rtw_rx_recycle_ring rx_recycle_ring;
#endif
	#endif /* PLATFORM_LINUX */

	_timer dynamic_chk_timer; /* dynamic/periodic check timer */
	_timer core_feature_sw_timer; /* 1 sec timer for core layer sw features */
	#ifdef CONFIG_RTW_NAPI_DYNAMIC
	u8 en_napi_dynamic;
	#endif /* CONFIG_RTW_NAPI_DYNAMIC */

	#ifdef CONFIG_RTW_WIFI_HAL
	u32 nodfs;
	#endif

	/*-------- below is for PCIE/USB/SDIO INTERFACE --------*/
	#ifdef CONFIG_SDIO_HCI
	SDIO_DATA sdio_data;
	#endif
	#ifdef CONFIG_GSPI_HCI
	GSPI_DATA gspi_data;
	#endif
	#ifdef CONFIG_PCI_HCI
	PCI_DATA pci_data;
	#endif
	#ifdef CONFIG_USB_HCI
	USB_DATA usb_data;
	#endif

	struct rtw_intf_ops *intf_ops;

	struct trx_data_buf_q  litexmitbuf_q;
	struct trx_data_buf_q  litexmit_extbuf_q;
	struct trx_data_buf_q  literecvbuf_q;

	/*-------- below is for USB INTERFACE --------*/
	#ifdef CONFIG_USB_HCI
	u8	Queue2Pipe[HW_QUEUE_ENTRY];/* for out pipe mapping */
	struct trx_urb_buf_q xmit_urb_q;
	struct trx_urb_buf_q recv_urb_q;
	#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	struct trx_data_buf_q  intin_buf_q;
	struct trx_urb_buf_q intin_urb_q;
	#endif
	#endif/* CONFIG_USB_HCI */

	/*-------- below is for PCIE INTERFACE --------*/
	#ifdef CONFIG_PCI_HCI


	#endif/* CONFIG_PCI_HCI */

	#ifdef CONFIG_MCC_MODE
	struct mcc_obj_priv mcc_objpriv;
	#endif /*CONFIG_MCC_MODE */

	/* also for RTK T/P Testing Mode */
	u8 scan_deny;


	#ifdef CONFIG_RTW_CUSTOMER_STR
	_mutex customer_str_mutex;
	struct submit_ctx *customer_str_sctx;
	u8 customer_str[RTW_CUSTOMER_STR_LEN];
	#endif

	struct debug_priv drv_dbg;

#ifdef CONFIG_DRV_FAKE_AP
	struct fake_ap fakeap;
#endif /* CONFIG_DRV_FAKE_AP */

	#ifdef CONFIG_FILE_FWIMG
	/* Placeholder for per physical adapter firmware file name.
	 * Freddie ToDo: Move to phl_com as PHL/HAL common feature
	 *               should be placed there.
	 */
	char fw_file[PATH_LENGTH_MAX];
	#endif
#ifdef CONFIG_DYNAMIC_THROUGHPUT_ENGINE
	u8 th_mode; //0:normal, 1:Tx
	struct sta_info *high_tp_sta;
#endif
#ifdef WIFI_LOGO_11N_WMM
	u8 wmm_test;
#endif
	u8 wmm_mode;
	u32 wmm_mode_to;
	u64 tx_wmm_pkts[4];
	u64 tx_wmm_pkts_prev[4];
	u32 ac_page_reg[4];
	struct mntr_result cur_mntr;
#ifdef DEBUG_CORE_RX
	u32 cnt_rx_pktsz;
	u32 num_rx_pktsz_os;
	u32 total_rx_pkt_os;
#endif
#ifdef CONFIG_RTW_DRV_HAS_NVM
	void *nvm;
#endif /* CONFIG_RTW_DRV_HAS_NVM */
#ifdef CONFIG_RTW_HANDLE_SER_L2
	u32 ser_L2_cnt;
	u8 ser_L2_inprogress;
#endif
struct rtw_env_report env_rpt;
#ifdef RTW_MI_SHARE_BSS_LIST
	u32 max_bss_cnt; /* The size of scan queue */
	_list *pscanned;
	_queue free_bss_pool;
	_queue scanned_queue;
	u8 *free_bss_buf;
	u32 num_of_scanned;
#endif
#ifdef CONFIG_AMSDU_HW_TIMER
	_lock amsdu_hw_timer_lock;
	_list amsdu_hw_timer_list;
#endif
};

#define DEV_STA_NUM(_dvobj)		MSTATE_STA_NUM(&((_dvobj)->iface_state))
#define DEV_STA_LD_NUM(_dvobj)		MSTATE_STA_LD_NUM(&((_dvobj)->iface_state))
#define DEV_STA_LG_NUM(_dvobj)		MSTATE_STA_LG_NUM(&((_dvobj)->iface_state))
#define DEV_TDLS_LD_NUM(_dvobj)		MSTATE_TDLS_LD_NUM(&((_dvobj)->iface_state))
#define DEV_AP_NUM(_dvobj)			MSTATE_AP_NUM(&((_dvobj)->iface_state))
#define DEV_AP_STARTING_NUM(_dvobj)	MSTATE_AP_STARTING_NUM(&((_dvobj)->iface_state))
#define DEV_AP_LD_NUM(_dvobj)		MSTATE_AP_LD_NUM(&((_dvobj)->iface_state))
#define DEV_ADHOC_NUM(_dvobj)		MSTATE_ADHOC_NUM(&((_dvobj)->iface_state))
#define DEV_ADHOC_LD_NUM(_dvobj)	MSTATE_ADHOC_LD_NUM(&((_dvobj)->iface_state))
#define DEV_MESH_NUM(_dvobj)		MSTATE_MESH_NUM(&((_dvobj)->iface_state))
#define DEV_MESH_LD_NUM(_dvobj)		MSTATE_MESH_LD_NUM(&((_dvobj)->iface_state))
#define DEV_P2P_DV_NUM(_dvobj)		MSTATE_P2P_DV_NUM(&((_dvobj)->iface_state))
#define DEV_P2P_GC_NUM(_dvobj)		MSTATE_P2P_GC_NUM(&((_dvobj)->iface_state))
#define DEV_P2P_GO_NUM(_dvobj)		MSTATE_P2P_GO_NUM(&((_dvobj)->iface_state))
#define DEV_SCAN_NUM(_dvobj)		MSTATE_SCAN_NUM(&((_dvobj)->iface_state))
#define DEV_WPS_NUM(_dvobj)			MSTATE_WPS_NUM(&((_dvobj)->iface_state))
#define DEV_ROCH_NUM(_dvobj)		MSTATE_ROCH_NUM(&((_dvobj)->iface_state))
#define DEV_MGMT_TX_NUM(_dvobj)		MSTATE_MGMT_TX_NUM(&((_dvobj)->iface_state))
#define DEV_U_CH(_dvobj)			MSTATE_U_CH(&((_dvobj)->iface_state))
#define DEV_U_BW(_dvobj)			MSTATE_U_BW(&((_dvobj)->iface_state))
#define DEV_U_OFFSET(_dvobj)		MSTATE_U_OFFSET(&((_dvobj)->iface_state))

#define dvobj_to_pwrctl(dvobj) (&(dvobj->pwrctl_priv))
#define dvobj_to_led(dvobj) (&(dvobj->ledpriv))
#define dvobj_to_acs(dvobj) (&(dvobj->acspriv))
#define pwrctl_to_dvobj(pwrctl) container_of(pwrctl, struct dvobj_priv, pwrctl_priv)
#define dvobj_to_macidctl(dvobj) (&(dvobj->macid_ctl))
#define dvobj_to_sec_camctl(dvobj) (&(dvobj->cam_ctl))
#define dvobj_to_regsty(dvobj) (&(dvobj->padapters[IFACE_ID0]->registrypriv))
#if defined(CONFIG_IOCTL_CFG80211)
#define dvobj_to_wiphy(dvobj) ((dvobj)->wiphy)
#endif
#define dvobj_to_rfctl(dvobj) (&(dvobj->rf_ctl))
#define rfctl_to_dvobj(rfctl) container_of((rfctl), struct dvobj_priv, rf_ctl)

#ifdef CONFIG_PCI_HCI
static inline PCI_DATA *dvobj_to_pci(struct dvobj_priv *dvobj)
{
	return &dvobj->pci_data;
}
#endif
#ifdef CONFIG_USB_HCI
static inline USB_DATA *dvobj_to_usb(struct dvobj_priv *dvobj)
{
	return &dvobj->usb_data;
}
#endif
#ifdef CONFIG_SDIO_HCI
static inline SDIO_DATA *dvobj_to_sdio(struct dvobj_priv *dvobj)
{
	return &dvobj->sdio_data;
}
#endif
#ifdef CONFIG_GSPI_HCI
static inline GSPI_DATA *dvobj_to_gspi(struct dvobj_priv *dvobj)
{
	return &dvobj->gspi_data;
}
#endif


#if defined(PLATFORM_LINUX) || defined(PLATFORM_ECOS)
static inline struct device *dvobj_to_dev(struct dvobj_priv *dvobj)
{
	/* todo: get interface type from dvobj and the return the dev accordingly */
#ifdef RTW_DVOBJ_CHIP_HW_TYPE
#endif

#ifdef CONFIG_USB_HCI
	return &dvobj->usb_data.pusbintf->dev;
#endif
#ifdef CONFIG_SDIO_HCI
	return &dvobj->sdio_data.func->dev;
#endif
#ifdef CONFIG_GSPI_HCI
	return &dvobj->gspi_data.func->dev;
#endif
#ifdef CONFIG_PCI_HCI
	return &dvobj->pci_data.ppcidev->dev;
#endif
}
#endif

_adapter *dvobj_get_unregisterd_adapter(struct dvobj_priv *dvobj);
_adapter *dvobj_get_adapter_by_addr(struct dvobj_priv *dvobj, u8 *addr);
#if defined(CONFIG_RTW_PERSIST_IF) || defined(CONFIG_RTW_PERSIST_IF)
_adapter *dvobj_get_adapter_by_name(struct dvobj_priv *dvobj, const char *ndev_name);
#endif /* CONFIG_RTW_PERSIST_IF */

#define dvobj_get_primary_adapter(dvobj)	((dvobj)->padapters[IFACE_ID0])


enum _ADAPTER_TYPE {
	PRIMARY_ADAPTER,
	VIRTUAL_ADAPTER,
	MAX_ADAPTER = 0xFF,
};

#ifdef CONFIG_RTW_NAPI
enum _NAPI_STATE {
	NAPI_DISABLE = 0,
	NAPI_ENABLE = 1,
};
#endif

#if 0 /*#ifdef CONFIG_MAC_LOOPBACK_DRIVER*/
typedef struct loopbackdata {
	_sema	sema;
	_thread_hdl_ lbkthread;
	u8 bstop;
	u32 cnt;
	u16 size;
	u16 txsize;
	u8 txbuf[0x8000];
	u16 rxsize;
	u8 rxbuf[0x8000];
	u8 msg[100];

} LOOPBACKDATA, *PLOOPBACKDATA;
#endif

#define ADAPTER_TX_BW_2G(adapter) BW_MODE_2G((adapter)->driver_tx_bw_mode)
#define ADAPTER_TX_BW_5G(adapter) BW_MODE_5G((adapter)->driver_tx_bw_mode)

#ifdef RTW_PHL_DBG_CMD
#define	CORE_LOG_NUM 	(100)
#define MAX_FRAG		(4)
#define INV_TXFORCE_VAL	(0x0FFFF)
#define LOG_DEAMSDU_PKTNUM 10

enum _CORE_REC_TYPE {
	REC_TX_MGMT = BIT(0),
	REC_TX_DATA = BIT(1),
	REC_TX_PHL = BIT(2),
	REC_TX_PHL_RCC = BIT(3),

	REC_RX_PHL = BIT(10),
	REC_RX_PHL_RCC = BIT(11),
	REC_RX_MGMT = BIT(12),
	REC_RX_DATA = BIT(13),
	REC_RX_DATA_RETRY = BIT(14),
	REC_RX_PER_IND = BIT(15),
};

struct core_record {
	u32 type;
	u32 totalSz;

	u32 wl_seq;
	u32 wl_type;
	u32 wl_subtype;

	u8 	fragNum;
	u32 fragLen[MAX_FRAG];
	void* virtAddr[MAX_FRAG];
	u32 phyAddrL[MAX_FRAG];
	u32 phyAddrH[MAX_FRAG];
};

struct core_logs {
	u32 txCnt_all;
	u32 txCnt_data;
	u32 txCnt_mgmt;
	u32 txCnt_phl;
	u32 txSize_phl;
	u32 txCnt_recycle;
	u32 txSize_recycle;
	struct core_record drvTx[CORE_LOG_NUM];
	struct core_record phlTx[CORE_LOG_NUM];
	struct core_record txRcycle[CORE_LOG_NUM];

	u32 rxCnt_phl;
	u32 rxSize_phl;
	u32 rxCnt_recycle;
	u32 rxSize_recycle;
	u32 rxCnt_data;
	u32 rxCnt_data_retry;
	u32 rxCnt_mgmt;
	u32 rxCnt_all;
	struct core_record drvRx[CORE_LOG_NUM];
	struct core_record phlRx[CORE_LOG_NUM];
	struct core_record rxRcycle[CORE_LOG_NUM];

#ifdef CONFIG_RTW_CORE_RXSC
	u32 rxCnt_data_orig;
	u32 rxCnt_data_shortcut;
	u32 rxCnt_prf_reuse;

	u32 rxsc_sta_get[2];
	u32 rxsc_entry_hit[2];
	u32 rxsc_alloc_entry[2];
#ifdef CORE_RXSC_AMSDU
	u32 rxCnt_amsdu_orig;
	u32 rxCnt_amsdu_shortcut;
#if defined(CONFIG_RTW_BYPASS_DEAMSDU) //&& defined(PLATFORM_LINUX)
	u32 rxCnt_deamsdu_bypass[4];
	u32 rxCnt_deamsdu_bypass_sz[4];
#endif
#endif
#endif
	u32 rxCnt_deamsdu_pktnum[LOG_DEAMSDU_PKTNUM];

	u32 rxCnt_coreInd;
	struct core_record rxPerInd[CORE_LOG_NUM];
};

#define MAX_TXBD_SIZE	40
#define MAX_TXWD_SIZE	128
#define MAX_RXWD_SIZE	32


enum _PHL_REC_TYPE {
	REC_TXBD = BIT(0),
	REC_TXWD = BIT(1),
	REC_RXWD = BIT(2),
	REC_WP_RCC = BIT(3),
	REC_RX_MAP = BIT(4),
	REC_RX_UNMAP = BIT(5),
	REC_RX_AMPDU = BIT(6),
	REC_RX_PER_ISR = BIT(7),
};

struct record_txbd {
	u32 bd_len;
	u8	bd_buf[MAX_TXBD_SIZE];
};

struct record_txwd {
	u32 wp_seq;
	u32 wd_len;
	u8	wd_buf[MAX_TXWD_SIZE];
};

struct record_rxwd {
	u32 wd_len;
	u8	wd_buf[MAX_RXWD_SIZE];
};

struct record_pci {
	u32 map_len;
	void *virtAddr;
	void* phyAddrL;
	void* phyAddrH;
};

struct record_wp_rcc {
	u32 wp_seq;
};

struct phl_logs {
	u32 txCnt_bd;
	u32 txCnt_wd;
	u32 txCnt_recycle;

	struct record_txbd txBd[CORE_LOG_NUM];
	struct record_txwd txWd[CORE_LOG_NUM];
	struct record_wp_rcc wpRecycle[CORE_LOG_NUM];

	u32 rxCnt_map;
	u32 rxSize_map;
	u32 rxCnt_unmap;
	u32 rxSize_unmap;
	struct record_pci rxPciMap[CORE_LOG_NUM];
	struct record_pci rxPciUnmap[CORE_LOG_NUM];

	u32 rxCnt_wd;
	struct record_rxwd rxWd[CORE_LOG_NUM];

	u32 rxCnt_ampdu;
	u32	rxAmpdu[CORE_LOG_NUM];
};

#endif

#ifdef DEBUG_MAP_UNASSOC
#define MAX_UNASSOC_STA_NUM (64)

struct unassoc_sta_info {
	_list list;
	u8 mac[ETH_ALEN];
	u8 used;
	s8 rssi;
	systime time;
};
#endif

#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
struct envinfo_data{
	unsigned int rssi_metric;
	unsigned int cu_metric;
	unsigned int noise_metric;
};

struct crossband_data{
	_adapter	*crossband_vxd_sc;
	_adapter	*primary_sc;
	struct envinfo_data metric_log;
};
#endif

#if defined(WFO_VIRT_MODULE)
#include <linux/smp.h>
extern int wfo_enable;

typedef struct wfo_mib_s {
	struct mlme_priv	mlmepriv;
	struct security_priv	securitypriv;
	struct registry_priv	registrypriv;
} wfo_mib_t;

_adapter *wfo_get_mib_offset(struct net_device *dev, char *mibname, void *_entry);
unsigned char is_g6_ndev(struct net_device *dev);
unsigned char is_g6_adapter(_adapter *padapter);

#if defined(WFO_VIRT_RECEIVER)
void alloc_wfo_mib(_adapter *padapter);
void update_wfo_mib(_adapter *padapter);
#endif /* WFO_VIRT_RECEIVER */

#include "../os_dep/linux/wfo/wfo_cmd_virt.h"
#else /* !WFO_VIRT_MODULE */
#define wiphy_to_ndev(wiphy) (((_adapter *)wiphy_to_adapter(wiphy))->rtw_wdev->netdev)
#endif /* WFO_VIRT_MODULE */

#if defined(WFO_RADIO_SENDER)
#include "../os_dep/linux/wfo/wfo_cmd_radio.h"
#endif /* WFO_RADIO_SENDER */

#ifdef CONFIG_RTW_OPCLASS_CHANNEL_SCAN


enum MAP_BAND_WIDTH{
	MAP_BAND_WIDTH_INVALID = 0,
	MAP_BAND_WIDTH_20M_HZ = 1,
	MAP_BAND_WIDTH_40M_HZ = 2,
	MAP_BAND_WIDTH_80M_HZ = 3,
	MAP_BAND_WIDTH_80P80M_HZ = 4,
	MAP_BAND_WIDTH_160M_HZ = 5,
};

struct channel_scan_neighbor {
	u8 timestamp[31];
	u8	bssid[6];
	u8	ssid_length;
	s8*	ssid;
	u8	signal_strength;
	u8	channel_band_width;	//20 or 40 or 80 or 80+80 or 160 MHz - 1 2 3 4 5  (0 -> invalid)
	u8	channel_utilization;
	u16	station_count;
};
struct channel_scan_result_per_channel {
	u8 channel;
	u8 scan_status;	//set this to be non zero to indicate an error
	s8 timestamp[31];	//always 31 char long in our implementation
	u8 channel_utilization;
	u8 noise;
	u16	neighbor_nr;
	struct channel_scan_neighbor* neighbors;
};
struct channel_scan_result_per_radio {
	u8 channel_nr;
	struct channel_scan_result_per_channel* channels;
};
#endif

#ifdef CONFIG_SWQ_SKB_ARRAY
struct skb_ptr {
	struct sk_buff *skb_array[VW_SWQ_SKB_NR];
	u16 wptr, rptr;
};
#endif

#ifdef CONFIG_ADPTVTY_CONTROL
struct adptvty_info {
	u8  try;
	u32 cnt_t;
	u32 cnt_t_last;
	u32 cnt_u;
	u32 cnt_u_last;
	u8  rto_u;
	u8  adptvty_test_cnt;
	u8  adptvty_test;
	u8  tp_test_cnt;
	u8  tp_test;
};
#endif /*CONFIG_ADPTVTY_CONTROL*/

struct _ADAPTER {
	int	pid[3];/*process id from UI, 0:wpa_supplicant, 1:hostapd, 2:dhcpcd*/

	/*extend to support multi interface*/
	u8 iface_id;
	u8 isprimary; /* is primary adapter or not */
	/* notes:
	**	if isprimary is true, the adapter_type value is 0, iface_id is IFACE_ID0 for PRIMARY_ADAPTER
	**	if isprimary is false, the adapter_type value is 1, iface_id is IFACE_ID1 for VIRTUAL_ADAPTER
	**	refer to iface_id if iface_nums>2 and isprimary is false and the adapter_type value is 0xff.*/
	u8 adapter_type;/*be used in  Multi-interface to recognize whether is PRIMARY_ADAPTER  or not(PRIMARY_ADAPTER/VIRTUAL_ADAPTER) .*/

	u8 mac_addr[ETH_ALEN];
	/* adapter up time in sec */
	u32 up_time;
	/*CONFIG_PHL_ARCH*/
	struct rtw_wifi_role_t *phl_role;
#ifdef SBWC
	_timer SBWC_timer;
#endif
#ifdef RTW_STA_BWC
	unsigned char		txduty; // 0: disable, 1: enable Tx ducy cycle limitation
	unsigned int		sta_bwc_total_tp; // kbps; used when txduty=1
	unsigned char		txduty_level; // 1: -10%, 2: -20% , ..., 9: -90%
	unsigned int  		tx_tp_base;  // kbps, throughput of the moment when Tx duty cycle mechansim kicks off
	unsigned int  		tx_tp_limit;  // kbps, the upper-bound throughput for limitation
	unsigned char 		got_limit_tp; // 0:hasn't got limit throughput base, 1: got
	u8					last_asoc_cnt;
	_timer sta_bwc_timer;
#endif
#ifdef GBWC
	_timer GBWC_timer;
	struct sk_buff_head		GBWC_tx_queue;
	struct sk_buff_head		GBWC_rx_queue;
	unsigned int			GBWC_tx_count;
	unsigned int			GBWC_rx_count;
	unsigned int			GBWC_consuming_Q;
	unsigned char			GBWC_timer_alive;
#endif

#ifdef CONFIG_WFA_OFDMA_Logo_Test
	u8 dynamic_grp_inited;
	_timer ofdma_timer;
#endif
	u8 run_cmd_en;

#ifdef CONFIG_HWSIM
	int bup_hwsim;
#endif

	u8 netif_up;

	u8 registered;
	u8 ndev_unregistering;


	struct dvobj_priv	*dvobj;
	struct mlme_priv	mlmepriv;
	struct mlme_ext_priv mlmeextpriv;
	struct evt_priv	evtpriv;
	struct xmit_priv	xmitpriv;
	struct recv_priv	recvpriv;
	struct sta_priv	stapriv;
	struct security_priv	securitypriv;
	_lock   security_key_mutex; /* add for CONFIG_IEEE80211W, none 11w also can use */
	struct registry_priv	registrypriv;

	struct sta_info *self_sta;

	#ifdef CONFIG_RTW_80211K
	struct rm_priv	rmpriv;
	#endif

	#ifdef CONFIG_MP_INCLUDED
	struct mp_priv	mppriv;
	#endif

	#ifdef CONFIG_AP_MODE
	struct hostapd_priv	*phostapdpriv;
	u8 bmc_tx_rate;
	#endif/*CONFIG_AP_MODE*/

	#ifdef CONFIG_BAND_STEERING
	struct b_steer_priv bsteerpriv;
	#endif

	u32	setband;
	ATOMIC_T bandskip;

	#ifdef CONFIG_P2P
	struct wifidirect_info	wdinfo;
	#endif /* CONFIG_P2P */

	#ifdef CONFIG_TDLS
	struct tdls_info	tdlsinfo;
	#endif /* CONFIG_TDLS */

	#ifdef CONFIG_WFD
	struct wifi_display_info wfd_info;
	#endif /* CONFIG_WFD */

	#ifdef CONFIG_RTW_NAPI
	struct	napi_struct napi;
	u8	napi_state;
	#endif

	#ifdef CONFIG_GPIO_API
	u8	pre_gpio_pin;
	struct gpio_int_priv {
		u8 interrupt_mode;
		u8 interrupt_enable_mask;
		void (*callback[8])(u8 level);
	} gpiointpriv;
	#endif

	#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
	_thread_hdl_ cmdThread;
	#endif
	#ifdef CONFIG_EVENT_THREAD_MODE
	_thread_hdl_ evtThread;
	#endif
	#if 0 /*def CONFIG_XMIT_THREAD_MODE*/
	_thread_hdl_ xmitThread;
	#endif
	#ifdef CONFIG_RECV_THREAD_MODE
	_thread_hdl_ recvThread;
	#endif

	#if defined(PLATFORM_LINUX) || defined(PLATFORM_ECOS)
	#ifdef CONFIG_IOCTL_CFG80211
	#ifdef CONFIG_P2P
	struct cfg80211_wifidirect_info	cfg80211_wdinfo;
	#endif /* CONFIG_P2P */
	#endif /* CONFIG_IOCTL_CFG80211 */

	_nic_hdl pnetdev;
	char old_ifname[IFNAMSIZ];

	/* used by rtw_rereg_nd_name related function */
	struct rereg_nd_name_data {
		_nic_hdl old_pnetdev;
		char old_ifname[IFNAMSIZ];
		u8 old_ips_mode;
		u8 old_bRegUseLed;
	} rereg_nd_name_priv;

	struct net_device_stats stats;
	struct iw_statistics iwstats;
	struct proc_dir_entry *dir_dev;/* for proc directory */
	struct proc_dir_entry *dir_odm;

	#ifdef CONFIG_MCC_MODE
	struct proc_dir_entry *dir_mcc;
	#endif /* CONFIG_MCC_MODE */

	#ifdef CTC_WIFI_DIAG
	struct proc_dir_entry *dir_ctcwifi;
	#endif

	#ifdef CONFIG_IOCTL_CFG80211
	struct wireless_dev *rtw_wdev;
	struct rtw_wdev_priv wdev_data;

	#endif /* CONFIG_IOCTL_CFG80211 */

	#endif /* PLATFORM_LINUX */

	#ifdef CONFIG_TX_AMSDU
	u8 tx_amsdu;
	u16 tx_amsdu_rate;
	#endif

	#ifdef CONFIG_MCC_MODE
	struct mcc_adapter_priv mcc_adapterpriv;
	#endif /* CONFIG_MCC_MODE */

	#ifdef CONFIG_RTW_MESH
	struct rtw_mesh_cfg mesh_cfg;
	struct rtw_mesh_info mesh_info;
	_timer mesh_path_timer;
	_timer mesh_path_root_timer;
	_timer mesh_atlm_param_req_timer; /* airtime link metrics param request timer */
	_workitem mesh_work;
	unsigned long wrkq_flags;
	#endif /* CONFIG_RTW_MESH */

	#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
	ATOMIC_T tbtx_tx_pause;
	ATOMIC_T tbtx_remove_tx_pause;
	u8 	tbtx_capability;
	u32	tbtx_duration;
	#endif /* CONFIG_RTW_TOKEN_BASED_XMIT */

	u32 vw_pc_cnt;
	u32 no_rts;
#ifdef CONFIG_VW_REFINE
	u32 max_enq_len;
	u32 max_deq_len;
	u32 xmit_dsr_num;
	u32 xmit_dsr_time;
	u32 sta_deq_len;
	u32 ring_lmt;
	u32 delay_test;
	u32 mgnt_cnt;
	u32 debug_mode;
	u32 set_hw_swq_timeout;
	u32 hw_swq_timeout;
	u32 hw_swq_cnt;
	u32 hw_irq_cnt;
	u32 set_swq_timeout;
	u32 swq_timeout;
	u32 tx_lmt;
	u32 lf_time;
	u32 with_bk;
	u32 no_wdinfo;
	u32 spec_pktsz;
	u32 sta_sn_gap;
	u32 direct_xmit;
	u32 is_map;
#if defined(RTW_CORE_TX_MSDU_TRANSFER_IN_PHL) || defined(WFO_STRUCT_ALIGNED)
	u32 is_msdu;
#endif
	u32 rtsrate_cck;
	u32 big_pkt;
	u32 small_pkt;

	u8 pre_pos;
	u8 vw_enable;
	u8 tc_enable;
	u32 amsdu_merge_cnt;
	/*
	manual_tx_amsdu[0]: 1=need to write
	manual_tx_amsdu[1]: tx_amsdu for big frame setting
	manual_tx_amsdu[2]: tx_amsdu for mid frame setting
	manual_tx_amsdu[3]: tx_amsdu for small frame setting
	*/
	u8 manual_tx_amsdu[4];
	/*
	current_tx_amsdu[0]: current tx_amsdu for big frame
	current_tx_amsdu[1]: current tx_amsdu for mid frame
	current_tx_amsdu[2]: current tx_amsdu for small frame
	*/
	u8 current_tx_amsdu[3];

#ifdef CONFIG_SWQ_SKB_ARRAY
	struct skb_ptr swq_skb_array[MAX_SKB_XMIT_QUEUE];
#endif

	ATOMIC_T skb_xmit_queue_len;
	struct sk_buff_head  skb_xmit_queue[MAX_SKB_XMIT_QUEUE];
	u32 skb_que_timestamp[MAX_SKB_XMIT_QUEUE];
	unsigned long skb_que_ts[MAX_SKB_XMIT_QUEUE];
	u32 skb_vw_cnt[MAX_SKB_XMIT_QUEUE];
	u32 skb_vw_rec_cnt[MAX_SKB_XMIT_QUEUE];
	u32 vw_retry_cnt[MAX_SKB_XMIT_QUEUE];

	u32 skb_max_que_len;
	u32 swq_amsdu_cnt;
	u32 tx_req_cnt;

	_tasklet swq_xmit_tasklet;
	struct sta_info *swq_psta[MAX_SKB_XMIT_QUEUE];
	u8 swq_staq_bitmap[MAX_SKB_XMIT_QUEUE];
#endif

	#ifdef CONFIG_WAPI_SUPPORT
	u8	WapiSupport;
	RT_WAPI_T wapiInfo;
	#endif

	#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	RTL_WAPI_AP_INFO wapiApInfo;
	#endif

	#ifdef CONFIG_BR_EXT
	_lock				br_ext_lock;
	/* unsigned int			macclone_completed; */
	struct nat25_network_db_entry	*nethash[NAT25_HASH_SIZE];
	int				pppoe_connection_in_progress;
	unsigned char			pppoe_addr[MAC_ADDR_LEN];
	unsigned char			scdb_mac[MAC_ADDR_LEN];
	unsigned char			scdb_ip[4];
	struct nat25_network_db_entry	*scdb_entry;
	unsigned char			br_mac[MAC_ADDR_LEN];
	unsigned char			br_ip[4];

	struct br_ext_info			ethBrExtInfo;
	#endif /* CONFIG_BR_EXT */

	#if 0 /*#ifdef CONFIG_MAC_LOOPBACK_DRIVER*/
	PLOOPBACKDATA ploopback;
	#endif

	#ifdef PLATFORM_FREEBSD
	_nic_hdl pifp;
	_lock glock;
	#endif /* PLATFORM_FREEBSD */

	/* for debug purpose */
#define NO_FIX_RATE		0xFFFF
#define GET_FIX_RATE(v)		((v) & 0x0FFF)
#define GET_FIX_RATE_SGI(v)	(((v) & 0x7000) >> 12)
	u16 fix_rate;
#define NO_FIX_BW		0xFF
	u8 fix_bw;
	u8 data_fb; /* data rate fallback, valid only when fix_rate is not 0xffff */
	u8 power_offset;
	u8 driver_tx_bw_mode;
	u8 rsvd_page_offset;
	u8 rsvd_page_num;
	u8 ch_clm_ratio;
	u8 ch_nhm_ratio;
	#ifdef CONFIG_SUPPORT_FIFO_DUMP
	u8 fifo_sel;
	u32 fifo_addr;
	u32 fifo_size;
	#endif

	u8 bLinkInfoDump;
	/*	Added by Albert 2012/10/26 */
	/*	The driver will show up the desired channel number when this flag is 1. */
	u8 bNotifyChannelChange;
	u8 bsta_tp_dump;
	#ifdef CONFIG_P2P
	/*	Added by Albert 2012/12/06 */
	/*	The driver will show the current P2P status when the upper application reads it. */
	u8 bShowGetP2PState;
	#endif
	u8 driver_vcs_en; /* Enable=1, Disable=0 driver control vrtl_carrier_sense for tx */
	u8 driver_vcs_type;/* force 0:disable VCS, 1:RTS-CTS, 2:CTS-to-self when vcs_en=1. */
	u8 driver_ampdu_spacing;/* driver control AMPDU Density for peer sta's rx */
	u8 driver_rx_ampdu_factor;/* 0xff: disable drv ctrl, 0:8k, 1:16k, 2:32k, 3:64k; */
	u8 driver_rx_ampdu_spacing;  /* driver control Rx AMPDU Density */
	u8 driver_rx_amsdu_size;  /* driver control Rx AMSDU size */
	u8 fix_rx_ampdu_accept;
	u8 fix_rx_ampdu_size; /* 0~127, TODO:consider each sta and each TID */
	u16 driver_tx_max_agg_num; /*fix tx desc max agg num , 0xffff: disable drv ctrl*/

	#ifdef DBG_RX_COUNTER_DUMP
	u8 dump_rx_cnt_mode;/*BIT0:drv,BIT1:mac,BIT2:phy*/
	u32 drv_rx_cnt_ok;
	u32 drv_rx_cnt_crcerror;
	u32 drv_rx_cnt_drop;
	#endif

	#ifdef CONFIG_DBG_COUNTER
	struct rx_logs rx_logs;
	struct tx_logs tx_logs;
	struct int_logs int_logs;
	#endif

#ifdef CONFIG_RTW_A4_STA
	/* for STA mode */
	struct core_a4_gptr_table *a4_gpt_records;
	ATOMIC_T a4_gpt_record_num;
#endif

#ifdef RTW_PHL_DBG_CMD
	u32 swdbg;
	u8 cca_rts_mode;

	struct core_logs core_logs;
	struct phl_logs phl_logs;

	u32 txForce_enable;
	u32 txForce_rate;
	u32 txForce_agg;
	u32 txForce_aggnum;
	u32 txForce_gi;
	u32 txForce_bw;
	u32 txForce_ampdu_density;

	u32 sniffer_enable;
	u8	record_enable;
#endif
#ifdef RTW_PHL_TX
	u8 *pxmit_txreq_buf;
	_queue	*pfree_txreq_queue;
	u32 txreq_full_cnt;

	u32 tx_ring_idx;
	u8 **tx_pool_ring_ptr;
	u32 max_tx_ring_cnt;
#endif
#ifdef DEBUG_PHL_RX
	u32 cnt_core_rx_mgmt;
	u32 cnt_core_rx_data;
	u32 cnt_core_rx_data_retry;
#endif
#ifdef CONFIG_RTW_CORE_RXSC
	u8 enable_rxsc;
	struct sta_info *cached_sta;
#ifdef CORE_RXSC_RFRAME
	union recv_frame *cached_prframe;
#endif
#endif
#ifdef RTW_CORE_PKT_TRACE
	u8 pkt_trace_enable;
	u8 pkt_trace_level;
	struct rtw_pkt_filter rtw_pkt_trace_filters[RTW_PKT_MAX_FILTERS];
#endif
	u32 cached_token;
#ifdef CONFIG_RTW_A4_STA
	u8 a4_enable;
	struct list_head			a4_sta_list;
#ifdef CONFIG_DYN_ALLOC_A4_TBL
	_lock		machash_lock[A4_STA_HASH_SIZE];
#else
	struct rtw_a4_tbl_entry 		a4_entry[MAX_A4_TBL_NUM];
#endif
	struct rtw_a4_db_entry			*machash[A4_STA_HASH_SIZE];
	u32	cnt_a4_tx;
	u32	cnt_a4_txsc;
	u32	cnt_a4_txsc_amsdu;

	u32	cnt_a4_rx;
	u32	cnt_a4_rxsc;
	u32	cnt_a4_rxsc_amsdu;
#if defined(CONFIG_A4_LOOPBACK) || defined(WFO_STRUCT_ALIGNED)
	u32								replace_idx;
	struct hlist_head				a4_loop_list[A4_LOOP_HASH_SIZE];
	struct rtw_a4_loopback_entry	*a4_loop_entry;
	struct rtw_a4_loopback_entry	*a4_loop_cache;
#endif
#endif
#ifdef CONFIG_RTW_MULTI_AP
	u8 multi_ap_mode;
	u8 multiap_last_cu;
	_queue  black_list;
	u8 multiap_cac_channel_hold;
	u8 multiap_cac_op_class;
	_timer multiap_cac_timer;
#ifdef CONFIG_RTW_MULTI_AP_R3
	u8 service_priority_enabled; // use the first bit, rest reserved.
	u8 service_priority_output; // 0x00 -- 0x09 rule output, rest reserved.
	u8 dscp_pcp_table_enabled;
	u8 *dscp_pcp_table;
	u8 dpp_enable;
#endif
#if (CONFIG_RTW_MULTI_AP_DFS_EN) && (CONFIG_DFS)
	u8 *map_neighbor_bss_tbl;
#endif /* CONFIG_RTW_MULTI_AP_DFS_EN */
#endif
#ifdef CONFIG_RTW_OPCLASS_CHANNEL_SCAN
	u8  opclass_channel_scan;
	u8  opclass_sync_result;
	u8  multiap_nl_send;
	u8  opclass_channel_proc_done;
	u8  opclass_requested_channel_nr;
	u8* opclass_requested_channels;
	struct channel_scan_result_per_radio* opclass_scan_result;
#endif
#ifdef DEBUG_MAP_UNASSOC
	u32 map_measure_state;
	u8 map_measure_op_class;
	u8 map_measure_channel;

	struct unassoc_sta_info			map_unassoc_sta[MAX_UNASSOC_STA_NUM];
	_timer map_unassoc_sta_timer;
	_queue unassoc_sta_used_queue;
	_queue unassoc_sta_free_queue;
#endif
	// 0: stop dump;
	// bit0(1): (SU) rssi dump;
	// bit1(2): (RU) rssi dump;
	u32 sta_dump_to;
	u32 sta_dump_bitmap;
	u32 sta_ru_dump_bitmap;
	u32 ru_c2h_intvl;

#ifdef CONFIG_LMT_TXREQ
	u8 lmt_txreq_dump;
#endif

#ifdef CONFIG_WFA_OFDMA_Logo_Test
#ifdef CONFIG_80211AX_HE
	struct he_ru_info ru_info;
#endif
#endif

	u32 tp_total_tx;
	u32 tp_total_rx;
	u32 tp_total_trx;
	u32 FS_LS_cnt;
#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
	struct crossband_data crossband;
#endif

#if defined(CONFIG_WFO_VIRT_MODULE) || defined(CONFIG_RTK_WFO_NO_VIRT)
	wfo_cfg80211_t cfg;
	_mutex cfg_mutex;
	u16 cfg_owner_id;
	char wfo_mapid;
	wfo_mib_t *wfo;
#endif /* CONFIG_WFO_VIRT_MODULE|CONFIG_RTK_WFO_NO_VIRT */
#ifdef CONFIG_RTW_TWT_DBG
	u8 twt_cmd_trigger;
	u8 twt_cmd_implicit;
	u8 twt_cmd_flow_type;
	u8 twt_cmd_flow_id;
	u8 twt_cmd_wake_int_exp;
	u8 twt_cmd_nom_min_twt_wake_dur;
	u32 twt_cmd_target_wake_t_h;
	u32 twt_cmd_target_wake_t_l;
	u16 twt_cmd_macid;
	u16 twt_cmd_twt_wake_int_mantissa;
	u8 twt_cmd_teardown_id;
	u8 twt_cmd_pwrbit;
	u16 twt_cmd_trigger_apep;
	u16 twt_cmd_trigger_rssi;
	u16 twt_cmd_trigger_mcs;
	u16 twt_cmd_data_rate;

	u8 twt_cmd_test_case;
	u8 twt_cmd_test_macid1;
	u8 twt_cmd_test_macid2;
	u8 twt_cmd_test_overlap;
	u32 twt_cmd_test_wait_second;
#endif
#ifdef CONFIG_CORE_TXSC
	u32 txsc_cache_thres;
	u32 txsc_time_duration;
#endif
#ifdef DBG_CONFIG_CMD_DISP
	enum phl_cmd_type cmd_type;
	u32 cmd_timeout;
#endif
#ifdef CONFIG_LMT_TXREQ
	u8 lmt_txreq_enable;
#endif
#if defined(CONFIG_RTW_DBG_TX_MGNT) || defined(WFO_STRUCT_ALIGNED)
	u8 dbg_tx_mgnt;
#endif /* CONFIG_RTW_DBG_TX_MGNT */
	unsigned long bcn_msg_hdl_state;
	u32 bcn_msg_hdl[2];
	u8 bcn_msg_hdl_idx;
	u8 dscp_mapping_enable;
	u8 dscp_mapping_table[DSCP_TABLE_SIZE];
#ifdef CONFIG_ONE_TXQ
	u32 atm_ifusetime;
	u32 atm_ifsettime;
	struct atm_stainfo atm_sta_info[NUM_STA];
#endif
#ifdef CONFIG_ADPTVTY_CONTROL
	struct adptvty_info adpt_info;
#endif /*CONFIG_ADPTVTY_CONTROL*/
#ifdef CONFIG_AMSDU_HW_TIMER
	_tasklet amsdu_xmit_tasklet;
	u32 amsdu_hw_timer_enable;
	u32 amsdu_hw_timeout; //ms
	u32 amsdu_hw_irq_cnt;
#endif
};

#define adapter_to_dvobj(adapter) ((adapter)->dvobj)
#define adapter_to_regsty(adapter) dvobj_to_regsty(adapter_to_dvobj((adapter)))
#define adapter_to_pwrctl(adapter) dvobj_to_pwrctl(adapter_to_dvobj((adapter)))
#define adapter_to_led(adapter) dvobj_to_led(adapter_to_dvobj((adapter)))
#define adapter_to_acs(adapter) dvobj_to_acs(adapter_to_dvobj((adapter)))
#define adapter_wdev_data(adapter) (&((adapter)->wdev_data))
#define adapter_to_wiphy(adapter) dvobj_to_wiphy(adapter_to_dvobj(adapter))

#define adapter_to_rfctl(adapter) dvobj_to_rfctl(adapter_to_dvobj((adapter)))
#define adapter_to_macidctl(adapter) dvobj_to_macidctl(adapter_to_dvobj((adapter)))

#define adapter_mac_addr(adapter) (adapter->mac_addr)

#ifdef CONFIG_RTW_CFGVENDOR_RANDOM_MAC_OUI
#define adapter_pno_mac_addr(adapter) \
	((adapter_wdev_data(adapter))->pno_mac_addr)
#endif

#define adapter_to_chset(adapter) (adapter_to_rfctl((adapter))->channel_set)

#define mlme_to_adapter(mlme) container_of((mlme), _adapter, mlmepriv)
#define tdls_info_to_adapter(tdls) container_of((tdls), _adapter, tdlsinfo)

#define rtw_get_chip_id(adapter) (((_adapter *)adapter)->dvobj->chip_id)
#define rtw_get_intf_type(adapter) (((_adapter *)adapter)->dvobj->interface_type)

#define rtw_get_mi_nums(adapter) (((_adapter *)adapter)->dvobj->iface_nums)

static inline void dev_set_surprise_removed(struct dvobj_priv *dvobj)
{
	ATOMIC_SET(&dvobj->bSurpriseRemoved, _TRUE);
}
static inline void dev_clr_surprise_removed(struct dvobj_priv *dvobj)
{
	ATOMIC_SET(&dvobj->bSurpriseRemoved, _FALSE);
}
static inline void dev_set_drv_stopped(struct dvobj_priv *dvobj)
{
	ATOMIC_SET(&dvobj->bDriverStopped, _TRUE);
}
static inline void dev_clr_drv_stopped(struct dvobj_priv *dvobj)
{
	ATOMIC_SET(&dvobj->bDriverStopped, _FALSE);
}
#define dev_is_surprise_removed(dvobj)	(ATOMIC_READ(&dvobj->bSurpriseRemoved) == _TRUE)
#define dev_is_drv_stopped(dvobj)		(ATOMIC_READ(&dvobj->bDriverStopped) == _TRUE)

/*
 * Function disabled.
 *   */
#define DF_TX_BIT		BIT0			/*rtw_usb_write_port_cancel*/
#define DF_RX_BIT		BIT1			/*rtw_usb_read_port_cancel*/
#define DF_IO_BIT		BIT2

/* #define RTW_DISABLE_FUNC(padapter, func) (ATOMIC_ADD(&dvobj->disable_func, (func))) */
/* #define RTW_ENABLE_FUNC(padapter, func) (ATOMIC_SUB(&dvobj->disable_func, (func))) */
__inline static void RTW_DISABLE_FUNC(struct dvobj_priv *dvobj, int func_bit)
{
	int df = ATOMIC_READ(&dvobj->disable_func);
	df |= func_bit;
	ATOMIC_SET(&dvobj->disable_func, df);
}

__inline static void RTW_ENABLE_FUNC(struct dvobj_priv *dvobj, int func_bit)
{
	int df = ATOMIC_READ(&dvobj->disable_func);
	df &= ~(func_bit);
	ATOMIC_SET(&dvobj->disable_func, df);
}

#define RTW_CANNOT_RUN(dvobj) \
	(dev_is_surprise_removed(dvobj) || \
	dev_is_drv_stopped(dvobj))

#define RTW_IS_FUNC_DISABLED(dvobj, func_bit) \
	(ATOMIC_READ(&dvobj->disable_func) & (func_bit))

#define RTW_CANNOT_IO(dvobj) \
	(dev_is_surprise_removed(dvobj) || \
	 RTW_IS_FUNC_DISABLED((dvobj), DF_IO_BIT))

#define RTW_CANNOT_RX(dvobj) \
	(RTW_CANNOT_RUN(dvobj) || \
	 RTW_IS_FUNC_DISABLED((dvobj), DF_RX_BIT))

#define RTW_CANNOT_TX(dvobj) \
	(RTW_CANNOT_RUN(dvobj) || \
	 RTW_IS_FUNC_DISABLED((dvobj), DF_TX_BIT))


/* HCI Related header file */
#ifdef CONFIG_USB_HCI
	#include <usb_ops.h>
#endif

#ifdef CONFIG_SDIO_HCI
	#include <sdio_ops.h>
#endif

#ifdef CONFIG_GSPI_HCI
	#include <gspi_ops.h>
#endif

#ifdef CONFIG_PCI_HCI
	#include <pci_ops.h>
#endif
#include <rtw_trx_ops.h>


#include "../phl/hal_g6/mac/mac_reg.h"
#ifdef CONFIG_PHL_015_devp_tmp
#include "../phl/hci/phl_trx_def_pcie.h"
#include "../phl/hci/phl_trx_pcie.h"
#endif

#include <rtw_nvm.h>

#if !defined(PLATFORM_ECOS) && defined(CONFIG_WFO_VIRT_MODULE)
#define WFO_ADAPTER_SIG1 0x0845E04C
#define WFO_ADAPTER_SIG2 0x598767DD

#define set_wfo_adapter_sig(X) \
	do { \
		X->sig	= WFO_ADAPTER_SIG1; \
		X->sig2 = WFO_ADAPTER_SIG2; \
	} while(0)

#define chk_wfo_adapter_sig(X) \
	((((wfo_adapter_t *)X)->sig == WFO_ADAPTER_SIG1) && \
	(((wfo_adapter_t *)X)->sig2 == WFO_ADAPTER_SIG2))

__inline static sint __check_fwstate(_adapter *padapter, sint state)
{
	struct mlme_priv *pmlmepriv = NULL;

	if (chk_wfo_adapter_sig(padapter)) {
		pmlmepriv = ((wfo_adapter_t *)padapter)->mlmepriv;
	} else {
		pmlmepriv = &padapter->mlmepriv;
	}

	if ((state == WIFI_NULL_STATE) &&
		(pmlmepriv->fw_state == WIFI_NULL_STATE))
		return _TRUE;

	if (pmlmepriv->fw_state & state)
		return _TRUE;

	return _FALSE;
}

__inline static struct mlme_priv *get_mlmepriv(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = NULL;

	if (chk_wfo_adapter_sig(padapter)) {
		pmlmepriv = ((wfo_adapter_t *)padapter)->mlmepriv;
	} else {
		pmlmepriv = &padapter->mlmepriv;
	}

	return pmlmepriv;
}

__inline static struct security_priv *get_securitypriv(_adapter *padapter)
{
	struct security_priv *securitypriv = NULL;

	if (chk_wfo_adapter_sig(padapter)) {
		securitypriv = ((wfo_adapter_t *)padapter)->securitypriv;
	} else {
		securitypriv = &padapter->securitypriv;
	}

	return securitypriv;
}

__inline static struct registry_priv *get_registrypriv(_adapter *padapter)
{
	struct registry_priv *registrypriv = NULL;

	if (chk_wfo_adapter_sig(padapter)) {
		registrypriv = ((wfo_adapter_t *)padapter)->registrypriv;
	} else {
		registrypriv = &padapter->registrypriv;
	}

	return registrypriv;
}

__inline static struct wifi_mib_priv *get_mibpriv(_adapter *padapter)
{
	struct wifi_mib_priv *pmibpriv = NULL;

	if (chk_wfo_adapter_sig(padapter)) {
		struct registry_priv *registrypriv =
			(struct registry_priv *)(((wfo_adapter_t *)padapter)->registrypriv);
		pmibpriv = &registrypriv->wifi_mib;
	} else {
		pmibpriv = &padapter->registrypriv.wifi_mib;
	}

	return pmibpriv;
}

#else /* PLATFORM_ECOS | !CONFIG_WFO_VIRT_MODULE */
__inline static struct mlme_priv *get_mlmepriv(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	return pmlmepriv;
}

__inline static struct security_priv *get_securitypriv(_adapter *padapter)
{
	struct security_priv *securitypriv = &padapter->securitypriv;
	return securitypriv;
}

__inline static struct registry_priv *get_registrypriv(_adapter *padapter)
{
	struct registry_priv *registrypriv = &padapter->registrypriv;
	return registrypriv;
}

__inline static struct wifi_mib_priv *get_mibpriv(_adapter *padapter)
{
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;
	return pmibpriv;
}
#endif /* !PLATFORM_ECOS & CONFIG_WFO_VIRT_MODULE */

#endif /* __DRV_TYPES_H__ */
