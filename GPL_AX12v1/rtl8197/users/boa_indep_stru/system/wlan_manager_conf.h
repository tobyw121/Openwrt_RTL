#define CONF_PATH_WLAN_MANAGER "/var/run/wlan_manager_conf"

#define WLAN_MGR_GET_OK			0
#define WLAN_MGR_GET_FAIL		1

#define WLAN_MANAGER_CONF_RESULT_MAX_LEN 512
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct wlan_manager_obj {
	char	name[50];
	int		(*fun)(char *name, char *result);
};

int wlan_mgr_get_band_steering_enable(char *name, char *result);
int wlan_mgr_get_prefer_band(char *name, char *result);
int wlan_mgr_get_bss_tm_req_th(char *name, char *result);
int wlan_mgr_get_bss_tm_req_disassoc_imminent(char *name, char *result);
int wlan_mgr_get_bss_tm_req_disassoc_timer(char *name, char *result);
int wlan_mgr_get_roam_detect_th(char *name, char *result);
int wlan_mgr_get_roam_sta_tp_th(char *name, char *result);
int wlan_mgr_get_roam_ch_clm_th(char *name, char *result);
int wlan_mgr_get_prefer_band_rssi_high(char *name, char *result);
int wlan_mgr_get_prefer_band_rssi_low(char *name, char *result);
int wlan_mgr_get_band_steering_end(char *name, char *result);
#if defined(RTK_CROSSBAND_REPEATER)
int wlan_mgr_get_crossband_enable(char *name, char *result);
int wlan_mgr_get_wlan0_ifname(char *name, char *result);
int wlan_mgr_get_wlan0_phyband_select(char *name, char *result);
int wlan_mgr_get_wlan0_ax_support(char *name, char *result);
int wlan_mgr_get_wlan0_rssi_thres(char *name, char *result);
int wlan_mgr_get_wlan0_cu_thres(char *name, char *result);
int wlan_mgr_get_wlan0_noise_thres(char *name, char *result);
int wlan_mgr_get_wlan0_rssi_weight(char *name, char *result);
int wlan_mgr_get_wlan0_cu_weight(char *name, char *result);
int wlan_mgr_get_wlan0_noise_weight(char *name, char *result);
int wlan_mgr_get_wlan1_ifname(char *name, char *result);
int wlan_mgr_get_wlan1_phyband_select(char *name, char *result);
int wlan_mgr_get_wlan1_ax_support(char *name, char *result);
int wlan_mgr_get_wlan1_rssi_thres(char *name, char *result);
int wlan_mgr_get_wlan1_cu_thres(char *name, char *result);
int wlan_mgr_get_wlan1_noise_thres(char *name, char *result);
int wlan_mgr_get_wlan1_rssi_weight(char *name, char *result);
int wlan_mgr_get_wlan1_cu_weight(char *name, char *result);
int wlan_mgr_get_wlan1_noise_weight(char *name, char *result);
#endif
