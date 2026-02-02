#ifndef SYSTEMD_COMMON_H
#define SYSTEMD_COMMON_H

#define WPS_HW_PUSHBTN_SUPPORT

#define WLAN_IF_PREFIX	"wlan"
#define VXD_IF_SUFFIX	"-vxd"
#define VAP_IF_SUFFIX	"-vap"
#define WLAN_IF_S		"wlan%d"
#define WLAN_VXD_S		"wlan%d-vxd"

#define WEB_PID_FILENAME	("/var/run/webs.pid")
#define WPS_ETC_CONF		"/etc/wps.conf"
#define PROC_GPIO			"/proc/gpio"

#define CONF_PATH_WLAN0 "/var/run/hapd_conf_wlan0"
#define CONF_PATH_WLAN1 "/var/run/hapd_conf_wlan1"
#define WPAS_CONF_PATH_WLAN0 "/var/run/wpas_conf_wlan0"
#define WPAS_CONF_PATH_WLAN1 "/var/run/wpas_conf_wlan1"
#define WPAS_CONF_PATH_WLAN0_VXD "/var/run/wpas_conf_wlan0_vxd"
#define WPAS_CONF_PATH_WLAN1_VXD "/var/run/wpas_conf_wlan1_vxd"

#define HOSTAPD_ACTION_PATH "/tmp/hostapd_action_systemd_sock"
#if defined(CONFIG_APP_MULTI_AP)
#define HOSTAPD_MAP_ACTION_PATH "/tmp/hostapd_action_map_sock"
#endif
#define HOSTAPD_RECV_MAX_DATA_SIZE 2048
#define AP_STA_CONNECTED "AP-STA-CONNECTED"
#define AP_STA_DISCONNECTED "AP-STA-DISCONNECTED"
#define AP_STA_POSSIBLE_PSK_MISMATCH "AP-STA-POSSIBLE-PSK-MISMATCH"
#define BSS_TM_RESP "BSS-TM-RESP"
#define STA_CONNECTED "CONNECTED"
#define AP_STA_POSSIBLE_PSK_MISMATCH "AP-STA-POSSIBLE-PSK-MISMATCH"

#ifdef WLAN_WPS_HAPD
#define PBC_STATUS_ACTIVE		"PBC Status: Active\n"
#define PBC_STATUS_TIMEOUT		"PBC Status: Timed-out\n"
#define PBC_STATUS_OVERLAP		"PBC Status: Overlap\n"
#define PBC_STATUS_DISABLED		"PBC Status: Disabled\n"
#define LAST_WPS_RES_NONE		"Last WPS result: None\n"
#define LAST_WPS_RES_FAILED		"Last WPS result: Failed\n"
#define LAST_WPS_RES_SUCCESS	"Last WPS result: Success\n"

#define WPS_CONFIGURE_START		"WPS configuration - START"
#define	WPS_CONFIGURE_END		"WPS configuration - END"

#define WPS_START_LED_GPIO_number 2			// twinkle
#define WPS_PBC_overlapping_GPIO_number 0	// off
#define WPS_SUCCESS_LED_GPIO_number 1		// on
#define WPS_ERROR_LED_GPIO_number 0
#define WPS_END_LED_unconfig_GPIO_number 0
#define WPS_END_LED_config_GPIO_number 0

#define LED_WSC_START     		-1
#define LED_WSC_TIMEOUT   		-2
#define LED_PBC_OVERLAPPED		-3
#define LED_WSC_ERROR     		-4
#define LED_WSC_SUCCESS   		-5
#define LED_WSC_NOP       		-6
#define LED_WSC_FAIL       		-7

enum {WPS_BTN_WLAN0=0, WPS_BTN_WLAN1, WPS_BTN_ALL};
enum {WPS_STATE_NONE=0, WPS_STATE_PROCESS_ALL=1, WPS_STATE_PROCESS_WLAN0=2, WPS_STATE_PROCESS_WLAN1=4};

typedef struct wps_config_setting{
	unsigned int  wsc_configured;
	unsigned char ssid[MAX_SSID_LEN];
	unsigned int  authType;
	unsigned int  encrypt;
	unsigned int  wpaAuth;
	unsigned int  sha256;
	unsigned int  dotIEEE80211W;
	unsigned int  unicastCipher;
	unsigned int  wpa2UnicastCipher;
	unsigned int  wpaPSKFormat;
	unsigned char wpaPSK[MAX_PSK_LEN+1];
	unsigned int  wsc_auth;
	unsigned int  wsc_enc;
	unsigned char wscPsk[MAX_PSK_LEN+1];
}WPS_CONF_SETTING;

typedef struct context{
	unsigned int wps_state;
	unsigned int pb_pressed_time;
	unsigned int is_not_dualband;
	unsigned int is_support_vxd;
}CONTEXT_S;
#endif

extern int hostapd_action_client_conn(char *sun_path);
extern int hostapd_action_send_client_msg(int sockfd, char *buf, int size);
extern int init_hostapd_action_socket(int *fd);
extern int recv_hostapd_action(int sockfd, char *buf, int buf_len);
extern int exit_hostapd_action_socket(int fd);

#endif //SYSTEMD_COMMON_H
