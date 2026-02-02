#ifndef HAPD_CONF_H
#define HAPD_CONF_H

#include "wifi6_priv_conf.h"

#define CONF_PATH_WLAN0 "/var/run/hapd_conf_wlan0"
#define CONF_PATH_WLAN1 "/var/run/hapd_conf_wlan1"
#define CONF_PATH(if_idx) ((if_idx==0) ? CONF_PATH_WLAN0 : CONF_PATH_WLAN1)

#define LOG_PATH_WLAN0 "/var/run/hapd_log_wlan0"
#define LOG_PATH_WLAN1 "/var/run/hapd_log_wlan1"
#define LOG_PATH(if_idx) ((if_idx==0) ? LOG_PATH_WLAN0 : LOG_PATH_WLAN1)

#define HOSTAPD_GLOBAL_CTRL_PATH "/var/run/hostapd/global"

#define PID_PATH_WLAN0	"/var/run/hapd_wlan0_pid.pid"
#define PID_PATH_WLAN1	"/var/run/hapd_wlan1_pid.pid"
#define PID_PATH(if_idx) ((if_idx==0) ? PID_PATH_WLAN0 : PID_PATH_WLAN1)

#define HOSTAPD_GLOBAL_PID_PATH "/var/run/hapd_pid.pid"

#define HOSTAPD_GLOBAL "global"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#if defined(CONFIG_WLAN_CLIENT_MODE) || defined(UNIVERSAL_REPEATER)
#define WPAS_CONF_PATH_WLAN0 "/var/run/wpas_conf_wlan0"
#define WPAS_CONF_PATH_WLAN1 "/var/run/wpas_conf_wlan1"
#define WPAS_CONF_PATH_WLAN0_VXD "/var/run/wpas_conf_wlan0_vxd"
#define WPAS_CONF_PATH_WLAN1_VXD "/var/run/wpas_conf_wlan1_vxd"
#define WPAS_CONF_PATH(if_idx) ((if_idx==0) ? WPAS_CONF_PATH_WLAN0 : WPAS_CONF_PATH_WLAN1)
#define WPAS_CONF_PATH_VXD(if_idx) ((if_idx==0) ? WPAS_CONF_PATH_WLAN0_VXD : WPAS_CONF_PATH_WLAN1_VXD)

#define WPAS_PID_PATH_WLAN0 "/var/run/wpas_wlan0_pid.pid"
#define WPAS_PID_PATH_WLAN1 "/var/run/wpas_wlan1_pid.pid"
#define WPAS_PID_PATH_WLAN0_VXD "/var/run/wpas_wlan0_vxd_pid.pid"
#define WPAS_PID_PATH_WLAN1_VXD "/var/run/wpas_wlan1_vxd_pid.pid"

#define WPAS_NAME_CTRL_IFACE "/var/run/wpa_supplicant"
#endif

#define NAME_DRIVER			"nl80211"

#define NAME_IFACE_WLAN0	"wlan0"
#define NAME_IFACE_WLAN1	"wlan1"


#define NAME_INTF(if_idx) (if_idx==0 ? NAME_IFACE_WLAN0 : NAME_IFACE_WLAN1)

#define NAME_CTRL_IFACE		"/var/run/hostapd"
#define NAME_BR				"br0"
const char IFCONFIG[] = "/bin/ifconfig";

#ifdef WLAN_WPS_HAPD
#define WPS_MODEL_NAME      	"RTL8671"
#define WPS_MODEL_NUMBER    	"EV-2010-09-20"
#define WPS_SERIAL_NUMBER   	"123456789012347"
#define WPS_DEVICE_TYPE     	"6-0050F204-1"
#define WPS_FRIENDLY_NAME   	"WLAN Access Point"
#define WPS_MANUFACTURER_URL	"http://www.realtek.com/"
#define WPS_MODEL_URL       	"http://www.realtek.com/"
#define WPS_MANUFACTURER    	"Realtek"
#endif

#define HAPD_GET_OK			0
#define HAPD_GET_FAIL		1

#define _IEEE8021X_MGT_			1		// WPA
#define _IEEE8021X_PSK_			2		// WPA with pre-shared key

#define _NO_PRIVACY_			0
#define _WEP_40_PRIVACY_		1
#define _TKIP_PRIVACY_			2
#define _WRAP_PRIVACY_			3
#define _CCMP_PRIVACY_			4
#define _WEP_104_PRIVACY_		5
#define _WEP_WPA_MIXED_PRIVACY_ 6		// WEP + WPA
#define _WAPI_SMS4_				7

#define _GCMP256_PRIVACY_		9
#define _BIP_GMAC256_PRIVACY_	12

enum WIFI_AUTH_ALGM {
	_AUTH_ALGM_OPEN_					= 0,
	_AUTH_ALGM_SHARED_					= 1,
	_AUTH_ALGM_FT_						= 2,
	_AUTH_ALGM_SAE_						= 3,
};

enum NETWORK_TYPE {
	WIRELESS_11B = 1,
	WIRELESS_11G = 2,
	WIRELESS_11A = 4,
	WIRELESS_11N = 8,
	WIRELESS_11AC = 64,
	WIRELESS_11AX = 128
};

enum channel_width {
	CHANNEL_WIDTH_20		= 0,
	CHANNEL_WIDTH_40		= 1,
	CHANNEL_WIDTH_80		= 2,
	CHANNEL_WIDTH_160		= 3,
	CHANNEL_WIDTH_80_80	= 4,
	CHANNEL_WIDTH_5		= 5,
	CHANNEL_WIDTH_10	= 6,
	CHANNEL_WIDTH_MAX	= 7,
};

enum CONTROL_CHANNEL_OFFSET {
	HT_CONTROL_UPPER = 0,
	HT_CONTROL_LOWER = 1,
};

enum RADIUS_APPLY {
	NO_RADIUS = 0,
	RADIUS_OPEN,
	RADIUS_WEP,
	RADIUS_WPA,
};

enum PMF_STATUS{
	NONE = 0,
	CAPABLE = 1,
	REQUIRED = 2,
};

enum { PSK_WPA=1, PSK_WPA2=2, PSK_WPA3=8};
enum { CIPHER_TKIP=1, CIPHER_AES=2, CIPHER_MIXED=3};

#define HAPD_CONF_RESULT_MAX_LEN 512

#define MAX_ACL_NUM 32
#define MAC_ADDR_LEN 6
struct macaddr {
	unsigned char _macaddr[MAC_ADDR_LEN];
	unsigned char used;
};

#define MAX_WLFTKH_NUM 32
struct wlftkh {
	unsigned char addr[6];
	unsigned char r0kh_id[49];
	unsigned char key[33];
};

struct wifi_config {
	char driver[32];
	char interface[32];
	char ctrl_interface[32];
	char bridge[32];

	char disabled;

	char ssid[64];
	unsigned char bssid[32];
	unsigned char channel;
	unsigned char channel_list[32];
	unsigned char auto_chan_sel;
	unsigned char channel_bandwidth;
	unsigned char control_band;
	unsigned char phy_band_select;

//Security
	unsigned char authtype;
	unsigned char encmode;
	unsigned char psk_format;
	unsigned char psk_enable;
	unsigned char wpa_cipher;
	unsigned char wpa2_cipher;
	unsigned char wpa3_cipher;
	unsigned char enable_1x;
	unsigned char passphrase[128];

	unsigned char wepKeyType;
	unsigned char wep_default_key;
	unsigned char wepkey0[128];
	unsigned char wepkey1[128];
	unsigned char wepkey2[128];
	unsigned char wepkey3[128];

#ifdef CONFIG_IEEE80211W
	unsigned char dot11IEEE80211W;
	unsigned char enableSHA256;
#endif

	unsigned int gk_rekey;

//RADIUS
	unsigned char	lan_ip[32];
	unsigned short 	rsPort;
	unsigned char 	rsIpAddr[4];
	unsigned char 	rsPassword[65];
	unsigned short 	rs2Port;
	unsigned char 	rs2IpAddr[4];
	unsigned char 	rs2Password[65];
	unsigned int 	radius_retry_primary_interval;
	unsigned int	radius_acct_interim_interval;

//Operation Mode
	unsigned char wlan_band;
	char country[10];
	unsigned char qos;

//Advanced
	unsigned short fragthres;
	unsigned short rtsthres;
	unsigned short bcnint;

	unsigned char dtimperiod;
	unsigned char preamble;
	unsigned char ignore_broadcast_ssid;
	unsigned char shortGI;
	unsigned char qos_enable;
	unsigned char multicast_to_unicast;
	unsigned char ap_isolate;
	unsigned char coexist;
	unsigned int ap_max_inactivity;
#ifdef WLAN_HS2_CONFIG
	unsigned char hs20;
#endif
	//unsigned int stbc; //TODO the driver not support
	//unsigned int ldpc; //TODO the driver not support
	unsigned char tdls_prohibit;
	unsigned char tdls_prohibit_chan_switch;
	unsigned char guest_access;

//ACL
	unsigned char macaddr_acl;
	struct macaddr mac_acl_list[MAX_ACL_NUM];

//WPS
#ifdef WLAN_WPS_HAPD
	unsigned char wps_state;
	char uuid[37];
	char device_name[64];
	char model_name[64];
	char model_number[64];
	char serial_number[64];
	char device_type[64];
	unsigned char config_method;
#define PIN_LEN	8
	char ap_pin[PIN_LEN+1];
	char upnp_iface[16];
	char friendly_name[64];
	char manufacturer_url[128];
	char model_url[128];
	int eap_server;
	char manufacturer[64];
#endif
#ifdef RTK_HAPD_MULTI_AP
	int multi_ap;
	unsigned char multi_ap_backhaul_ssid[32];
	unsigned char multi_ap_backhaul_passphrase[63];
	unsigned char multi_ap_backhaul_psk_format;
#endif

unsigned char disable_protection;
unsigned char ampdu;

//rates
	unsigned char supported_rates[64];
	unsigned char basic_rates[64];
//11k
#ifdef DOT11K
	unsigned char dot11k_enable;
#endif
#ifdef CONFIG_IEEE80211V
	unsigned char dot11v_enable;
#endif
//11r
#ifdef FAST_BSS_TRANSITION
	unsigned char ft_enable;
	unsigned char ft_mdid[5];
	unsigned char ft_over_ds;
	unsigned char ft_r0kh_id[49];
	unsigned char ft_push;
	unsigned char r1_key_holder[32];
	struct wlftkh wlftkh_list[MAX_WLFTKH_NUM];
#endif

//station
	unsigned int max_num_sta;

#if defined(CONFIG_WLAN_CLIENT_MODE) || defined(UNIVERSAL_REPEATER)//for wpa_supplicant
	int wlan_mode;
	int update_config;
	int scan_ssid;
	int pbss;
	int is_repeater;
#ifdef RTK_HAPD_MULTI_AP
	int multi_ap_backhaul_sta;
#endif
#endif
};

struct hapd_obj {
	char	        name[50];
	int            (*fun)(struct wifi_config *config, char *name, char *result);
};

void start_hapd_wpas_process(int wlan_index, config_wlan_ssid ssid_index);
void stop_hapd_wpas_process(int wlan_index, config_wlan_ssid ssid_index);
//int get_wifi_conf(struct wifi_config *config, int if_idx);

int apply_wpa(struct wifi_config *config);

//Control
int hapd_get_driver(struct wifi_config *config, char *name, char *result);
int hapd_get_interface(struct wifi_config *config, char *name, char *result);
int hapd_get_ctrl_interface(struct wifi_config *config, char *name, char *result);
int hapd_get_bridge(struct wifi_config *config, char *name, char *result);
int hapd_get_ssid(struct wifi_config *config, char *name, char *result);

//Security
int hapd_get_auth_algs(struct wifi_config *config, char *name, char *result);
int hapd_get_wpa(struct wifi_config *config, char *name, char *result);
int hapd_get_wpa_psk(struct wifi_config *config, char *name, char *result);
int hapd_get_wpa_passphrase(struct wifi_config *config, char *name, char *result);
int hapd_get_sae_password(struct wifi_config *config, char *name, char *result);
int hapd_get_wpa_key_mgmt(struct wifi_config *config, char *name, char *result);
int hapd_get_wpa_pairwise(struct wifi_config *config, char *name, char *result);
int hapd_get_rsn_pairwise(struct wifi_config *config, char *name, char *result);
#ifdef CONFIG_IEEE80211W
int hapd_get_ieee80211w(struct wifi_config *config, char *name, char *result);
#endif
int hapd_get_wpa_group_rekey(struct wifi_config *config, char *name, char *result);

int hapd_get_wep_key0(struct wifi_config *config, char *name, char *result);
int hapd_get_wep_key1(struct wifi_config *config, char *name, char *result);
int hapd_get_wep_key2(struct wifi_config *config, char *name, char *result);
int hapd_get_wep_key3(struct wifi_config *config, char *name, char *result);
int hapd_get_wep_default_key(struct wifi_config *config, char *name, char *result);
int hapd_get_wep_key_len_unicast(struct wifi_config *config, char *name, char *result);
int hapd_get_wep_tx_keyidx(struct wifi_config *config, char *name, char *result);
int hapd_get_sae_groups(struct wifi_config *config, char *name, char *result);
int hapd_get_sae_anti_clogging_threshold(struct wifi_config *config, char *name, char *result);

//RADIUS
int hapd_get_own_ip_addr(struct wifi_config *config, char *name, char *result);
int hapd_get_nas_identifier(struct wifi_config *config, char *name, char *result);

int hapd_get_ieee8021x(struct wifi_config *config, char *name, char *result);
int hapd_get_auth_server_addr(struct wifi_config *config, char *name, char *result);
int hapd_get_auth_server_port(struct wifi_config *config, char *name, char *result);
int hapd_get_auth_server_shared_secret(struct wifi_config *config, char *name, char *result);
int hapd_get_acct_server_addr(struct wifi_config *config, char *name, char *result);
int hapd_get_acct_server_port(struct wifi_config *config, char *name, char *result);
int hapd_get_acct_server_shared_secret(struct wifi_config *config, char *name, char *result);
int hapd_get_radius_retry_primary_interval(struct wifi_config *config, char *name, char *result);

int hapd_get_auth_server_addr_2(struct wifi_config *config, char *name, char *result);
int hapd_get_auth_server_port_2(struct wifi_config *config, char *name, char *result);
int hapd_get_auth_server_shared_secret_2(struct wifi_config *config, char *name, char *result);
int hapd_get_acct_server_addr_2(struct wifi_config *config, char *name, char *result);
int hapd_get_acct_server_port_2(struct wifi_config *config, char *name, char *result);
int hapd_get_acct_server_shared_secret_2(struct wifi_config *config, char *name, char *result);
int hapd_get_radius_acct_interim_interval(struct wifi_config *config, char *name, char *result);


//Opeation Mode
int hapd_get_hw_mode(struct wifi_config *config, char *name, char *result);
int hapd_get_ieee80211n(struct wifi_config *config, char *name, char *result);
int hapd_get_require_ht(struct wifi_config *config, char *name, char *result);
int hapd_get_ieee80211ac(struct wifi_config *config, char *name, char *result);
int hapd_get_require_vht(struct wifi_config *config, char *name, char *result);
int hapd_get_ht_capab(struct wifi_config *config, char *name, char *result);
int hapd_get_vht_capab(struct wifi_config *config, char *name, char *result);


//Operation Channel
int hapd_get_vht_oper_chwidth(struct wifi_config *config, char *name, char *result);
int hapd_get_channel(struct wifi_config *config, char *name, char *result);
int hapd_get_vht_oper_centr_freq_seg0_idx(struct wifi_config *config, char *name, char *result);

//HW Setting, Features
int hapd_get_bssid(struct wifi_config *config, char *name, char *result);
int hapd_get_country_code(struct wifi_config *config, char *name, char *result);


//Advanced
int hapd_get_fragm_threshold(struct wifi_config *config, char *name, char *result);
int hapd_get_rts_threshold(struct wifi_config *config, char *name, char *result);
int hapd_get_beacon_int(struct wifi_config *config, char *name, char *result);
int hapd_get_dtim_period(struct wifi_config *config, char *name, char *result);
int hapd_get_preamble(struct wifi_config *config, char *name, char *result);
int hapd_get_ignore_broadcast_ssid(struct wifi_config *config, char *name, char *result);
int hapd_get_wmm_enabled(struct wifi_config *config, char *name, char *result);
int hapd_get_multicast_to_unicast(struct wifi_config *config, char *name, char *result);
int hapd_get_ap_isolate(struct wifi_config *config, char *name, char *result);
int hapd_get_ap_max_inactivity(struct wifi_config *config, char *name, char *result);
#ifdef WLAN_HS2_CONFIG
int hapd_get_hs20(struct wifi_config *config, char *name, char *result);
#endif
int hapd_get_tdls_prohibit(struct wifi_config *config, char *name, char *result);
int hapd_get_tdls_prohibit_chan_switch(struct wifi_config *config, char *name, char *result);
#if 1//def CONFIG_GUEST_ACCESS_SUPPORT
int hapd_get_access_network_type(struct wifi_config *config, char *name, char *result);
#endif
//ACL
int hapd_get_macaddr_acl(struct wifi_config *config, char *name, char *result);
//WPS
#ifdef WLAN_WPS_HAPD
int hapd_get_wps_state(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_uuid(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_device_name(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_model_name(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_model_number(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_serial_number(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_device_type(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_config_methods(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_pbc_in_m1(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_ap_pin(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_upnp_iface(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_friendly_name(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_manufacturer_url(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_model_url(struct wifi_config *config, char *name, char *result);
int hapd_get_wps_eap_server(struct wifi_config *config, char *name, char *result);
#endif
#ifdef RTK_HAPD_MULTI_AP
int hapd_get_multi_ap(struct wifi_config *config, char *name, char *result);
int hapd_get_multi_ap_backhaul_ssid(struct wifi_config *config, char *name, char *result);
int hapd_get_multi_ap_backhaul_wpa_psk(struct wifi_config *config, char *name, char *result);
int hapd_get_multi_ap_backhaul_wpa_passphrase(struct wifi_config *config, char *name, char *result);
#endif

//rates
int hapd_get_supported_rates(struct wifi_config *config, char *name, char *result);
int hapd_get_basic_rates(struct wifi_config *config, char *name, char *result);
#ifdef DOT11K
//11k
int hapd_get_rrm_neighbor_report(struct wifi_config *config, char *name, char *result);
int hapd_get_rrm_beacon_report(struct wifi_config *config, char *name, char *result);
#endif
#ifdef CONFIG_IEEE80211V
//11v
int hapd_get_bss_transition(struct wifi_config *config, char *name, char *result);
#endif
//11r
#ifdef FAST_BSS_TRANSITION
int hapd_get_mobility_domain(struct wifi_config *config, char *name, char *result);
int hapd_get_ft_over_ds(struct wifi_config *config, char *name, char *result);
//int hapd_get_ft_r0_key_lifetime(struct wifi_config *config, char *name, char *result);
//int hapd_get_reassociation_deadline(struct wifi_config *config, char *name, char *result);
int hapd_get_pmk_r1_push(struct wifi_config *config, char *name, char *result);
int hapd_get_r1_key_holder(struct wifi_config *config, char *name, char *result);
int hapd_get_r0kh(struct wifi_config *config, char *name, char *result);
int hapd_get_r1kh(struct wifi_config *config, char *name, char *result);
#endif
//station
int hapd_get_max_num_sta(struct wifi_config *config, char *name, char *result);
//===================================

#endif
