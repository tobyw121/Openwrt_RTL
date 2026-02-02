#ifndef WIFI6_PRIV_CONF_H
#define WIFI6_PRIV_CONF_H

#define MACADDRLEN	6
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#include "../../../backports-5.2.8-1/drivers/net/wireless/realtek/g6_wifi_driver/include/rtw_mib.h"

#define WLAN_ROOT_ITF_INDEX		0
#define WLAN_VAP_ITF_INDEX		1

#define SIOCMIBINIT		0x8B42
#define SIOCMIBSYNC		0x8B43

extern const char HOSTAPD_SCRIPT_NAME[];
extern const char HOSTAPD[];
extern const char HOSTAPD_CLI[];
extern const char HOSTAPDPID[];
extern const char WPAS_CLI[];

extern const char* WLAN_IF_S[];
extern const char* VXD_IF[];

#define VAP_IF_ONLY   "vap%d"
#define VAP_IF		  "wlan%d-vap%d"
#define PHY_INDEX_FILE	"/sys/class/ieee80211/phy%d/index"
#define START_VXD_LATER		"/tmp/start_vxd_later%d"
#define PROC_BRSC	"/proc/rtl865x/brsc"

#define TIMER_COUNT 10

enum { IS_WLAN_ROOT=0, IS_WLAN_VAP=1, IS_WLAN_VXD=2};
typedef enum { WLAN_AGGREGATION_AMPDU=0, WLAN_AGGREGATION_AMSDU=1} WLAN_AGGREGATION_FLAG_T;

#define WIFI_PRIV_GET_OK		0
#define WIFI_PRIV_GET_FAIL		1

void start_wifi_priv_cfg(int wlan_index, int vwlan_index);
void gen_wifi_priv_cfg(struct wifi_mib_priv *wifi_mib, int wlan_idx, int vwlan_idx);

int get_wifi_mib_priv(int skfd, char *ifname, struct wifi_mib_priv *wifi_mib);
int set_wifi_mib_priv(int skfd, char *ifname, struct wifi_mib_priv *wifi_mib);

//===================================
#endif

