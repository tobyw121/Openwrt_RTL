#ifndef _WLAN_MANAGER_H_
#define _WLAN_MANAGER_H_

#include "common.h"

#define PRIV_INFO_OUTPUT	 		"/var/run/priv_info.out"
#define WLAN_MANAGER_STR			"[WLAN_MANAGER]"

#define WLAN_MANAGER_VER			"v2.0"

#if 1 /* sync wifi driver rtw_wlan_manager.h */

/* Netlink RTK Protocol Number */
#define NL_RTK_PROTOCOL				27
#define NL_RTK_PROTOCOL_SEC_DRV		31


/* Netlink Daemon Protocol ID */
#define NL_WLAN_MANAGER_PID			5183

/* Netlink Max Message Size */
#define NL_MAX_MSG_SIZE				768

/* Netlink Message Type List */
#define NL_DAEMON_ON_TYPE					1
#define NL_DAEMON_OFF_TYPE					2
#define NL_DAEMON_ALIVE_TYPE				3
#define NL_DEL_STA_TYPE						4
#define NL_NEW_STA_TYPE						5
#define NL_INTF_RPT_TYPE					6
#define NL_STA_RPT_TYPE						7
#define NL_FRAME_RPT_TYPE					8
#define NL_TIME_TICK_TYPE					9
#define NL_PRIV_INFO_CMD_TYPE				10
#ifdef CONFIG_BAND_STEERING
#define NL_B_STEER_CMD_TYPE					11
#define NL_B_STEER_BLOCK_ADD_TYPE			12
#define NL_B_STEER_BLOCK_DEL_TYPE			13
#define NL_B_STEER_ROAM_TYPE				14
#endif
#ifdef CONFIG_CROSSBAND_REPEATER
#define NL_CROSSBAND_DUMP_TYPE				15
#endif
#define NL_GENERAL_CMD_TYPE					100
#define NL_CUSTOMER_TYPE					101
#define NL_CONFIG_UPDATE_TYPE				102
struct nl_message {
	u32 type;
	u32 len;
	u8  content[NL_MAX_MSG_SIZE];
};

/* Element ID */
#define ELM_INTF_ID				1
#define ELM_INTF_INFO_ID		2
#define ELM_FRAME_INFO_ID		3
#define ELM_STA_INFO_ID			4
#define ELM_ROAM_INFO_ID		5
#define ELM_BUFFER_ID			6
#define ELM_STA_INFO_EXT_ID		7

/* Element List */
struct elm_header {
	u8 id;
	u8 len;
};

struct elm_intf {
	u8  mac[6];
	u8  root;
	u8  band;
	u8  ssid;
	s8  name[16];
};

struct elm_intf_info {
	u16 ch;
	u8  ch_clm;
	u8  ch_noise;
	u32 tx_tp;
	u32 rx_tp;
	u32 assoc_sta_num;
	/* self neighbor report info */
	u32 bss_info;
	u8  reg_class;
	u8  phy_type;
};

struct elm_frame_info {
	u16 frame_type;
	u8  sa[6];
	u8  rssi;
};

struct elm_sta_info {
	u8  mac[6];
	u8  rssi;
	u32 link_time;
	u32 tx_tp; /* kbits */
	u32 rx_tp; /* kbits */
};

struct elm_roam_info {
	u8  sta_mac[6]; /* station mac */
	u8  bss_mac[6]; /* target bss mac */
	u16 bss_ch; /* target bss channel */
	u8  method; /* 0: 11V, 1: Deauth */
};

struct elm_buffer {
	u8 buf[255];
};

struct elm_sta_info_ext {
	u8 mac[6];
	u8 supported_band;
	u8 empty[119]; /* for future use */
};


/* Element Size List */
#define ELM_HEADER_LEN			(sizeof(struct elm_header))
#define ELM_INTF_LEN			(sizeof(struct elm_intf))
#define ELM_INTF_INFO_LEN		(sizeof(struct elm_intf_info))
#define ELM_FRAME_INFO_LEN		(sizeof(struct elm_frame_info))
#define ELM_STA_INFO_LEN		(sizeof(struct elm_sta_info))
#define ELM_ROAM_INFO_LEN		(sizeof(struct elm_roam_info))
#define ELM_BUFFER_LEN			(sizeof(struct elm_buffer))
#define ELM_STA_INFO_EXT_LEN	(sizeof(struct elm_sta_info_ext))


#endif /* sync wifi driver rtw_wlan_manager.h */


/* ---------------------------------------- */
/* ---------- wlan manager APIs  ---------- */
/* ---------------------------------------- */
void _wlan_manager_hostapd_cli_bss_tm_req(
	s8 *intf_name, u8 *sta_mac, u8 *bss_mac, u16 bss_ch,
	u16 bss_info, u8 bss_reg_class, u8 bss_phy_type,
	u8 disassoc_imminent, u32 disassoc_timer);
void _wlan_manager_hostapd_cli_deauth(s8 *intf_name, u8 *sta_mac);
void _wlan_manager_set_msg(
	struct nl_message *msg, u32 *msg_len, void *elm, u32 elm_len);
void _wlan_manager_send_drv(void *msg, u32 msg_len);
#ifdef CONFIG_USER_RTK_COMMON_NETLINK
void _wlan_manager_send_sec_drv(void *msg, u32 msg_len);
#endif
void _wlan_manager_send_daemon(void *msg, u32 msg_len, pid_t pid);

#endif

