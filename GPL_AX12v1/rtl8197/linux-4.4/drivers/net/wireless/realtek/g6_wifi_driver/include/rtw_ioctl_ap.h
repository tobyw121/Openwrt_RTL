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
#ifndef _RTW_IOCTL_AP_H_
#define _RTW_IOCTL_AP_H_

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
#if defined(CONFIG_AP_NEIGHBOR_INFO) || defined(CPTCFG_AP_NEIGHBOR_INFO)
#ifndef AP_NEIGHBOR_INFO
#define AP_NEIGHBOR_INFO
#endif
#endif
#define BEACON_VS_IE
#ifdef CONFIG_RTW_80211V
#define CONFIG_IEEE80211V //= CONFIG_RTW_WNM
#endif
#ifdef MAC_ADDR_LEN
#define MAC_ADDR_LEN		MAC_ADDR_LEN
#else
#define MAC_ADDR_LEN		6
#endif /* MAC_ADDR_LEN */

#define DRV_VERSION_H	2
#define DRV_VERSION_L	6
#define DRV_VERSION_SUBL	35
#endif /* CONFIG_RTW_AP_EXT_SUPPORT */

typedef enum _wlan_mac_state {
    STATE_DISABLED=0, STATE_IDLE, STATE_SCANNING, STATE_STARTED, STATE_CONNECTED, STATE_WAITFORKEY
} wlan_mac_state;

enum wlan_mode_2_web {
	WLAN_WEB_INVALID = 0,
	WLAN_WEB_11B	= BIT0,
	WLAN_WEB_11G	= BIT1,
	WLAN_WEB_11A	= BIT2,
	WLAN_WEB_11N	= BIT3,
	WLAN_WEB_11AC	= BIT6,
	WLAN_WEB_11AX	= BIT7,
};

/* ***************************************************************** */
/* The structures in wifi_common.h are also used by the upper layer. */
/* ***************************************************************** */
#include <wifi_common.h>
/* ***************************************************************** */

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
#ifdef RTW_BLOCK_STA_CONNECT
typedef struct rtk_wlan_block_sta {
	unsigned char mac[6];
	unsigned int  block_time;
} RTK_WLAN_BLOCK_STA;
#endif /* RTW_BLOCK_STA_CONNECT */

#ifdef AP_NEIGHBOR_INFO
#define MAX_AP_NEIGHBOR_INFO_NUM 64

struct ap_neighbor_info
{
	int sta_entry_num;
	struct ap_neighbor_info_entry ap_neighbor_info_ent[MAX_AP_NEIGHBOR_INFO_NUM];
};

/* ------------------------- */
/* IOCTL AP: NEIGHOR AP INFO */
/* ------------------------- */
#define IOCTL_AP_NEIGHBOR_INFO
typedef struct _ap_neighbor_info_2_web {
	struct ap_neighbor_info_entry ap_nb_info_ent[MAX_AP_NEIGHBOR_INFO_NUM];
} ap_neighbor_info_2_web;
#endif // AP_NEIGHBOR_INFO

#ifdef CONFIG_RTW_80211K
#define MAX_BEACON_SUBLEMENT_LEN    226

#pragma pack(1)
struct dot11k_beacon_measurement_report_info
{
	unsigned char op_class;
	unsigned char channel;
	unsigned int  measure_time_hi;
	unsigned int  measure_time_lo;
	unsigned short measure_duration;
	unsigned char frame_info;
	unsigned char RCPI;
	unsigned char RSNI;
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char antenna_id;
	unsigned int  parent_tsf;
};

struct dot11k_beacon_measurement_report
{
	struct dot11k_beacon_measurement_report_info info;
	unsigned char subelements_len;
	unsigned char subelements[MAX_BEACON_SUBLEMENT_LEN];
};
#pragma pack()
#endif /* CONFIG_RTW_80211K */
#endif /* CONFIG_RTW_AP_EXT_SUPPORT */

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
enum{
	RTL8192CD_IOCTL_SET_MIB = 0x89F1,
	RTL8192CD_IOCTL_GET_MIB = 0x89F2,
	RTL8192CD_IOCTL_DEL_STA = 0x89F7,
	RTL8192CD_IOCTL_USER_DAEMON_REQUEST = 0x89FF,

	SIOCGIWRTLSTAINFO = 0x8B30,
	SIOCGIWRTLSTANUM = 0x8B31,		// get the number of stations in table
	SIOCGIWRTLDRVVERSION = 0x8B32,

	SIOCGIWRTLSCANREQ = 0x8B33,  // scan request
	SIOCGIWRTLGETBSSDB = 0x8B34,  	// get bss data base
	SIOCGIWRTLJOINREQ = 0x8B35,  // join request
	SIOCGIWRTLJOINREQSTATUS = 0x8B36,  // get status of join request

	SIOCGIWRTLGETBSSINFO = 0x8B37,  // get currnet bss info

	SIOCGIWRTLSTAEXTRAINFO = 0x8B40,	//for extera info

	SIOCMIBINIT = 0x8B42,
	SIOCMIBSYNC = 0x8B43,
	SIOCGMISCDATA = 0x8B48,			//get_misc_data
#ifdef CONFIG_RTW_MULTI_AP
	RTL8192CD_IOCTL_ASSOC_CONTROL = 0x8B38,
	RTL8192CD_IOCTL_UPDATE_BSS = 0x8B4A,
	RTL8192CD_IOCTL_SEND_DISASSOC = 0x8B4B,
	SIOCMAP_UPDATEPOLICY = 0x8B4E,
	SIOCMAP_BACKHAULSTEER = 0x8B53,
	SIOCMAP_GETASSOCSTAMETRIC = 0x8B55,
	SIOCMAP_GETUNASSOCSTAMETRIC = 0x8B56,
	RTL8192CD_IOCTL_AGENT_STEERING = 0x8B57,
	SIOCMAP_SEND_DISASSOC_VXD = 0x8B59,
	SIOCMAP_SET_TXMAXPOWER = 0x8B95,
	SIOCMAP_CAC = 0x8B72,
#endif
#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_RTW_OPCLASS_CHANNEL_SCAN)
	SIOCMAP_GET_AVAILABLE_CHANNELS = 0x8B94,
#endif
#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_WLAN_DE_SUPPORT)
	SIOCMAP_GETAPCAPABILITY = 0x8B51,
	SIOCMAP_GETCLIENTCAPABILITY = 0x8B52,
	SIOCMAP_GETAPMETRIC = 0x8B54,
	SIOCMAP_GENERAL_IOCTL = 0x8B83,
#endif
#ifdef CONFIG_RTW_OPCLASS_CHANNEL_SCAN
	SIOCOPCLASS_CHANNELSCAN_REQ = 0x8B5A,
	SIOCOPCLASS_CHANNELSCAN_RESP = 0x8B5B,
#endif

#ifdef AP_NEIGHBOR_INFO
	SIOCGAPNEIGHBORINFO = 0x8B7A,
#endif

#ifdef CONFIG_RTW_80211K
	SIOC11KBEACONREQ = 0x8BD2,
 	SIOC11KBEACONREP = 0x8BD3,
#ifdef CONFIG_RTW_WNM
	SIOC11VBSSTRANSREQ = 0x8BF5,
#endif
#endif

#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
	SIOCROSSBANDINFOREQ = 0x8BB2,
#endif

#ifdef BEACON_VS_IE
	SIOCGISETBCNVSIE = 0x8BBD,
	SIOCGISETPRBVSIE = 0x8BBE,
	SIOCGISETPRBRSPVSIE = 0x8BBF,
#endif

#ifdef CONFIG_IEEE80211V
	SIOCGICTCBSSTRANSREQ = 0x8B23,
#endif
#ifdef RTW_BLOCK_STA_CONNECT
	RTL8192CD_IOCTL_BLOCK_STA = 0x8B25,
#endif
};

enum _HT_CURRENT_TX_INFO_ {
	TX_USE_40M_MODE		= BIT(0),
	TX_USE_SHORT_GI		= BIT(1),
	TX_USE_80M_MODE     = BIT(2),
	TX_USE_160M_MODE    = BIT(3)
};

#ifdef CONFIG_RTW_MULTI_AP
enum MAP_GENERAL_OPERATION {
	MAP_RESERVED,
	MAP_GET_GENERAL,
	MAP_SET_GENERAL,
	MAP_SEND_5G_BSS_MACS,
	MAP_GET_LINK_METRICS,
	MAP_GET_STA_INFO,
	MAP_GET_CLIENTS_RSSI
};
#endif

int rtw_ioctl_private(struct net_device *dev, struct iwreq *wrq);
int rtw_bss_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_sta_extra_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_sta_num_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_drive_version_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_ss_status_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_join_status_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_sta_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_misc_data_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#ifdef AP_NEIGHBOR_INFO
int rtw_ap_neighbor_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif
#ifdef CONFIG_RTW_80211K
int rtw_rm_beacon_report_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif
#ifdef CONFIG_WIFI_DIAGNOSIS
int rtw_wifi_diag_result(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_wifi_diag_start(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif
int rtw_sta_entry_get(struct net_device *dev, struct iw_request_info *info,	union iwreq_data *wrqu,	char *extra);
int rtw_site_survey_result(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_site_survey_start(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_mib_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra, unsigned char flag);
int rtw_mib_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_wifi_priv_mib_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_wifi_priv_mib_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_wifi_dump_priv_mib(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
void rtw_core_recover_mibs(_adapter *padapter);
#ifdef BEACON_VS_IE
int rtw_vsie_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif
#ifdef SBWC
int rtw_sbwc_setting(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif
#ifdef RTW_STA_BWC
int sta_bwc_limit_tp(struct net_device *dev, int level);
void decide_limit_tp(_adapter *padapter, int level);
#endif
extern int rtw_del_sta(struct net_device *dev, struct ieee_param *param);
int rtw_dynamic_acl_add_sta(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_dynami_acl_remove_sta_(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_dynami_acl_clear_table(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_dynami_acl_check_table(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);

int rtw_ioctl_del_sta(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_ioctl_daemon_req(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_wifi_ss_req(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_wifi_join(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#ifdef BEACON_VS_IE
int rtw_beacon_vsie_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_probe_vsie_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_probersp_vsie_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif
#ifdef CONFIG_RTW_80211K
int rtw_rm_beacon_measurement_request(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#ifdef CONFIG_RTW_WNM
int rtw_bss_trans_request(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif
#endif
#ifdef CONFIG_IEEE80211V
int rtw_ctc_bss_trans_request(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif
#ifdef CONFIG_RTW_MULTI_AP
int rtw_ioctl_unassoc_sta_metric_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_ioctl_update_bss(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_ioctl_send_disassoc(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_ioctl_send_disassoc_vxd(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_ioctl_set_tx_max_power(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#ifdef CONFIG_RTW_MULTI_AP_R2
int rtw_ioctl_process_cac_request(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif /* defined(CONFIG_RTW_MULTI_AP_R2) */
#endif /* CONFIG_RTW_MULTI_AP */
#endif
#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_RTW_OPCLASS_CHANNEL_SCAN)
int rtw_ioctl_available_channels_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif
#ifdef CONFIG_RTW_OPCLASS_CHANNEL_SCAN
void rtw_opclass_site_survey_free(_adapter *padapter);
int rtw_opclass_trigger_site_survey(_adapter *padapter);
int rtw_opclass_do_site_survey(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_opclass_obtain_site_survey_result(_adapter *padapter);
int rtw_opclass_scan_rusult_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
#endif
int rtw_query_ther(struct net_device *dev, unsigned char *data, unsigned int data_len);
void rtw_ther_ctrl_set_func_off(struct net_device *dev);
void rtw_ther_ctrl_release_func_off(struct net_device *dev);
#ifdef WIFI6_THER_CTRL
#include <ther_ctrl.h>
void sync_mib_wifi6(struct net_device *dev, struct ther_info_s *info);
void set_limit_tp_wifi6(struct net_device *dev, int level);
void set_bandwidth_wifi6(struct net_device *dev, int bw);
void set_txduty_wifi6(struct net_device *dev, int level);
void set_power_wifi6(struct net_device *dev, int low_power);
void set_path_wifi6(struct net_device *dev, int path);
void set_funcoff_wifi6(struct net_device *dev, int enable);
#endif /* WIFI6_THER_CTRL */

#endif /* CONFIG_RTW_AP_EXT_SUPPORT */
