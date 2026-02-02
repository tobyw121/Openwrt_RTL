/******************************************************************************
 *
 * Copyright(c) 2007 - 2020 Realtek Corporation.
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

#ifndef _RTW_MAP_H_
#define _RTW_MAP_H_

#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_WLAN_DE_SUPPORT)
int core_map_get_metric(_adapter *padapter, u8 *tmpbuf, u8* result);
int core_map_get_ap_capability(_adapter *padapter, u8 *tmpbuf, u8* result);
int core_map_get_client_capability(_adapter *padapter, u8 *tmpbuf, u8* result);
#endif

#ifdef CONFIG_RTW_MULTI_AP

#if defined(DEBUG_MAP_NL) && !defined(RTW_COMMON_NETLINK)
#define NETLINK_RTW_MAP (31)
#elif defined(DEBUG_MAP_NL) && defined(RTW_COMMON_NETLINK)
#define NETLINK_RTW_MAP (27)
#endif

#ifdef DEBUG_MAP_NL
#define RTW_MAP_PREFIX "rtk_multi_ap"
#define RTW_MAP_MAX_PAYLOAD_SZ (2048)
#define RTW_MAP_MAX_BCN_RPT_SZ (1472) //1500(MAX_NETWORK_SEGMENT_SIZE)- 25 -3(1 tlv hdr + 2 tlv len)
#endif

#define MULTI_AP_SUB_ELEM_TYPE (0x06)
#define MULTI_AP_PROFILE_SUB_ELEM_TYPE (0x07)
#define MULTI_AP_802_1_Q_SUB_ELEM_TYPE (0x08)

#define MULTI_AP_TEAR_DOWN BIT(4)
#define MULTI_AP_FRONTHAUL_BSS BIT(5)
#define MULTI_AP_BACKHAUL_BSS BIT(6)
#define MULTI_AP_BACKHAUL_STA BIT(7)

#define MULTI_AP_UNASSOC_STA_TIMEOUT (7000)  /*unit: ms*/
#define MULTI_AP_CAC_TIMEOUT         (30000) /*unit: ms*/

#define TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE (152)

#ifdef CONFIG_RTW_MULTI_AP_R2
#define MAP_TX_CONTINUE (0)
#define MAP_TX_DROP (1)
#define MAP_RX_CONTINUE (0)
#define MAP_RX_DROP (1)
#endif

enum rtw_map_mode  {
	MAP_MODE_NONE = 0,
	MAP_MODE_FRONT_AP = 1,
	MAP_MODE_BACKHAL_AP = 2,
	MAP_MODE_FRONT_BACKHAUL_AP = 3,
	MAP_MODE_BACKHAL_STA = 4,
};

enum rtw_map_state  {
	MAP_STATE_NONE = 0,
	MAP_STATE_MEASURE_UNASSOC,
};

enum rtw_map_rx_mode  {
	MAP_RX_NORMAL = 0,
	MAP_RX_MONITOR,
};

#define TUNNELED_MSG_ASSOREQ 	0x00
#define TUNNELED_MSG_REASSOREQ  0x01
#define TUNNELED_MSG_BTMQUERY  	0x02
#define TUNNELED_MSG_WNMNOTIFYREQ		0x03
#define TUNNELED_MSG_ANQPREQ	0x04
#define TUNNELED_MSG_RESERVED	0x05

typedef struct multi_ap_tunnel_msg{
	u8  tlv_type;
	u16 tlv_len;
	u8  macaddr[ETH_ALEN];
	u8 	msg_type;
	u16	frame_body_len;
	u8  frame_body[0];
}__attribute__ ((aligned(1), packed)) MULTI_AP_TUNNEL_MSG_STRU;

u8 core_map_get_multi_ap_ie(const u8 *ies, int ies_len, u8 *profile, u16 *vlan_id);
u8 *core_map_append_multi_ap_ie(u8 *pbuf, uint *frlen, u8 val, u8 profile, u16 primary_vid);
void core_map_check_sta_ie(_adapter *adapter, struct sta_info *sta, u8 *ies, int ies_len);
int core_map_update_assoc_control(_adapter *padapter, u8 *tmpbuf);
int core_map_update_steering_policy(_adapter *padapter, u8 *tmpbuf);
int core_map_update_metric_policy(_adapter *padapter, u8 *tmpbuf);
int core_map_update_bss(_adapter *padapter, u8 *buffer, u16 lens, u16 maximum_limit, int *processed);
int core_map_get_asso_sta_metric(_adapter *padapter, u8 *tmpbuf, u8* result);
void core_map_cfg_init(_adapter *adapter);
void core_map_cfg_free(_adapter *adapter);

#ifdef DEBUG_MAP_NL
void core_map_nl_event_send(u8 *data, u32 data_len);
#ifdef CONFIG_RTW_80211K
void core_map_report_beacon_metrics(struct sta_info *psta);
#endif
#endif
#ifdef DEBUG_MAP_UNASSOC
#if 0
void core_map_init_unassoc_sta(_adapter *adapter);
void core_map_deinit_unassoc_sta(_adapter *adapter);
#endif
u8 core_map_check_state(_adapter *adapter, u32 state);
void core_map_update_unassoc_sta_metric(_adapter *adapter, union recv_frame *prframe);
s32 core_map_ioctl_get_unassoc_metric(_adapter *adapter, u8 *data);
#endif

#ifdef CONFIG_RTW_MULTI_AP_R2
void core_map_send_sta_disassoc_event(struct sta_info *psta, u16 reason);
void core_map_send_sta_failed_conn_event(struct sta_info *psta, u16 status, u16 reason);
void core_map_send_tunneled_message(_adapter *adapter, unsigned char* mac_address, unsigned char message_type, unsigned short payload_length, unsigned char *payload);
int core_map_update_channel_scan_result(_adapter * padapter);
u8 core_map_tx_vlan_process(_adapter *padapter, struct sk_buff *pskb, struct sta_info *psta);
u8 core_map_rx_vlan_process(_adapter *padapter, struct sk_buff **ppskb);
void core_map_send_association_status_notification(_adapter *padapter, u8 association_allowance_status);
void core_map_trigger_cac(_adapter *padapter, char *tmpbuf);
void core_map_terminate_cac(_adapter *padapter);
#ifdef CONFIG_RTW_MULTI_AP_R3
void _core_map_get_assoc_wifi6_sta_status_report(_adapter *padapter, u8 *tmpbuf, u8 *result, unsigned int *len);
void _support_map_wifi6_ap_cap_fill_role(_adapter *padapter, u8 *result_buf, unsigned int *offset, u8 roleFlag);
void _core_map_get_wifi6_ap_cap(_adapter *padapter, u8 *result_buf, unsigned int *len);
int map_get_ap_wifi6_capabilities(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes);
int map_get_ap_wifi6_cap_fill_role(_adapter *padapter, u8 *tlv_content, int *offset, u16 maximum_limit, u8 roleFlag);
int map_get_assoc_wifi6_sta_status_report(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes);
int map_set_service_prioritization_rule(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes);
int map_set_service_prioritization_rule_apply(_adapter *padapter, u8 add_remove,u8 output,u8 always_match);
int map_set_dscp_mapping_table(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes);
#endif
#endif

void core_map_ch_util_trigger(_adapter * padapter);
void core_map_ap_sta_rssi_trigger(_adapter * padapter, struct sta_info *psta);
void core_map_send_btm_response_notify(_adapter *padapter, u8 bssid[ETH_ALEN], u8 mac[ETH_ALEN], u8 target_bssid[ETH_ALEN], u8 status);
void core_map_send_client_notify(unsigned char event, unsigned char mac[ETH_ALEN], unsigned char bssid[ETH_ALEN]);
void core_map_send_client_join_notify(unsigned char bssid[ETH_ALEN], unsigned char mac[ETH_ALEN]);
void core_map_send_client_leave_notify(unsigned char bssid[ETH_ALEN], unsigned char mac[ETH_ALEN]);
void core_map_send_channel_change_notify(void);
int core_map_get_general(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed);
int core_map_set_general(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed);
int core_map_get_link_metric(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes);
int core_map_get_sta_info(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes);
int core_map_get_clients_rssi(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes);
#endif /* CONFIG_RTW_MULTI_AP */

#endif /* _RTW_MAP_H_ */
