#ifndef SET_INIT_WIFI6_H
#define SET_INIT_WIFI6_H

#include "wifi6_priv_conf.h"

enum {IWPRIV_GETMIB=1, IWPRIV_HS=2, IWPRIV_INT=4, IWPRIV_HW2G=8, IWPRIV_TXPOWER=16, IWPRIV_HWDPK=32, IWPRIV_AX=64, };

#define MAX_2G_TSSI_CCK_SIZE_AX    6
#define MAX_2G_TSSI_BW40_SIZE_AX   5
#define MAX_5G_TSSI_BW40_SIZE_AX  14
#define MAX_CHAN_NUM			  14

#define STA_NONE	0x0
#define STA_NOTAG	0x00000001
#define STA_INFO	0x00000002
#define STA_SCRIPT	0x00000004
#define STA_WARNING	0x00000008
#define STA_ERR		0x00000010

#define HW_MIB_FROM(name,idx) (idx ? MIB_HW_WLAN1_##name : MIB_HW_##name)

#ifdef CONFIG_GUEST_ACCESS_SUPPORT
int setup_wlan_guestaccess();
int clean_wlan_guestaccess();
#endif
int startWlan_wifi6(config_wlan_target target, config_wlan_ssid ssid_index);
int stopWlan_wifi6(config_wlan_target target, config_wlan_ssid ssid_index);

#endif
