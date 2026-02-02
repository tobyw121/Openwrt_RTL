#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/wireless.h>

#include "apmib.h"
#include "wifi6_priv_conf.h"
#include "sys_utility.h"
#include "ieee802_mib.h"

#ifdef CONFIG_APP_MULTI_AP
#include "map_init_wifi6.h"
#endif

//#include "hwnat_ioctl.h"

// for QoS control
#define GBWC_MODE_DISABLE			0
#define GBWC_MODE_LIMIT_MAC_INNER   1 // limit bw by mac address
#define GBWC_MODE_LIMIT_MAC_OUTTER  2 // limit bw by excluding the mac
#define GBWC_MODE_LIMIT_IF_TX	   3 // limit bw by interface tx
#define GBWC_MODE_LIMIT_IF_RX	   4 // limit bw by interface rx
#define GBWC_MODE_LIMIT_IF_TRX	   5 // limit bw by interface tx/rx

const char HOSTAPD_SCRIPT_NAME[] = "/var/hostapd";
const char HOSTAPD[] = "/bin/hostapd";
const char HOSTAPD_CLI[] = "/bin/hostapd_cli";
const char HOSTAPDPID[] = "/var/run/hostapd.pid";
const char WPAS_CLI[] = "/bin/wpa_cli";

const char* WLAN_IF_S[] = {"wlan0", "wlan1"};
const char* VXD_IF[] = {"wlan0-vxd", "wlan1-vxd"};

typedef enum {
    STA_CONTROL_ENABLE = (1<<0),
    STA_CONTROL_PREFER_BAND = (1<<1),
} STA_CONTROL_T;

static inline int
iw_get_ext(int                  skfd,           /* Socket to the kernel */
           const char *               ifname,         /* Device name */
           int                  request,        /* WE ID */
           struct iwreq *       pwrq)           /* Fixed part of the request */
{
 	/* Set device name */
	memset(pwrq->ifr_name, 0, IFNAMSIZ);
	strncpy(pwrq->ifr_name, ifname, IFNAMSIZ-1);
	/* Do the request */
	return(ioctl(skfd, request, pwrq));
}

int get_wifi_mib_priv(int skfd, char *ifname, struct wifi_mib_priv *wifi_mib)
{
	struct iwreq wrq;

	wrq.u.data.flags = SIOCMIBINIT;
	wrq.u.data.pointer = wifi_mib;
	wrq.u.data.length = (unsigned short)sizeof(struct wifi_mib_priv);

	if (iw_get_ext(skfd, ifname, SIOCDEVPRIVATE+2, &wrq) < 0) {
	  close( skfd );
	  printf("%s(%s): ioctl Error.\n", __FUNCTION__, ifname);
	  return -1;
	}

	return 0;
}

int get_wifi_mib(int skfd, char *ifname, struct wifi_mib *pmib)
{
	struct iwreq wrq;

	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);

	if (iw_get_ext(skfd, ifname, SIOCMIBINIT, &wrq) < 0) {
		printf("%s(%s): ioctl Error.\n", __FUNCTION__, ifname);
		return -1;
	}
	return 0;
}

int set_wifi_mib_priv(int skfd, char *ifname, struct wifi_mib_priv *wifi_mib)
{
	struct iwreq wrq;

	wrq.u.data.flags = SIOCMIBSYNC;
	wrq.u.data.pointer = wifi_mib;
	wrq.u.data.length = (unsigned short)sizeof(struct wifi_mib_priv);

	if (iw_get_ext(skfd, ifname, SIOCDEVPRIVATE+2, &wrq) < 0) {
	  close( skfd );
	  printf("%s(%s): ioctl Error.\n", __FUNCTION__, ifname);
	  return -1;
	}

	return 0;
}

int set_wifi_mib(int skfd, char *ifname, struct wifi_mib *pmib)
{
	struct iwreq wrq;

	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);

	if (iw_get_ext(skfd, ifname, SIOCMIBSYNC, &wrq) < 0)
	{
		printf("Set WLAN MIB failed!\n");
		return -1;
	}

	return 0;
}

void gen_wifi_priv_cfg(struct wifi_mib_priv *wifi_mib, int wlan_index, int vwlan_index)
{
#ifdef WLAN_RATE_PRIOR
	unsigned int rate_prior=0;
#endif
	unsigned int intVal=0, mode=0, intVal2=0, rpt_enabled=0;
	unsigned int tx_bandwidth, rx_bandwidth, gbwcmode = GBWC_MODE_DISABLE;
	unsigned int is_ax_support=0, phyband=0, band=0, wlan_mode=0, root_band=0;
	unsigned int roaming_enable=0, roaming_rssi_th1=0, roaming_rssi_th2=0;
	unsigned int roaming_start_time=0, roaming_wait_time=0;

	apmib_save_wlanIdx();

	wlan_idx = wlan_index;

	/* get root value */
	vwlan_idx = 0;

	apmib_get(MIB_WLAN_AX_SUPPORT, (void *)&is_ax_support);
	apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyband);

	//========
	/* vap & root share the same following setting ---start */
	apmib_get(MIB_WLAN_BAND, (void *)&root_band);

	//disable_protection
	apmib_get(MIB_WLAN_PROTECTION_DISABLED, (void *)&intVal);
	wifi_mib->disable_protection = intVal;

	//aggregation
	apmib_get(MIB_WLAN_AGGREGATION, (void *)&intVal);
	wifi_mib->aggregation = intVal;

	//ampdu
	if((intVal & (1<<WLAN_AGGREGATION_AMPDU))==0){
		wifi_mib->ampdu = 0;
	}
	else {
		wifi_mib->ampdu = 1;
	}

	//amsdu
	if((intVal & (1<<WLAN_AGGREGATION_AMSDU))==0) {
		wifi_mib->amsdu = 0;
	}
	else{
		if(is_ax_support)
			wifi_mib->amsdu = 1;
		else
			wifi_mib->amsdu = 2;
	}

	//iapp_enable
	apmib_get(MIB_WLAN_IAPP_DISABLED, (void *)&intVal);
	if(intVal==1)
		wifi_mib->iapp_enable = 0;
	else
		wifi_mib->iapp_enable = 1;

	//a4_enable & multiap_monitor_mode_disable -- TODO
#ifdef CONFIG_APP_MULTI_AP
	apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
	apmib_get(MIB_MAP_CONTROLLER, (void *)&intVal);
	if(intVal){
		apmib_get(MIB_WLAN_MAP_BSS_TYPE, (void *)&intVal);
		if((intVal & MAP_BACKHAUL_BSS) || (intVal & MAP_BACKHAUL_STA) || (vwlan_idx==WLAN_ROOT_ITF_INDEX && wlan_mode == 0))
			wifi_mib->a4_enable = 1;
		wifi_mib->multiap_monitor_mode_disable = 0;
	}else{
		wifi_mib->a4_enable = 0;
		wifi_mib->multiap_monitor_mode_disable = 1;
	}
#else
		wifi_mib->a4_enable = 0;
		wifi_mib->multiap_monitor_mode_disable = 1;
#endif

	//telco_selected
	apmib_get(MIB_TELCO_SELECTED, (void *)&intVal);
	wifi_mib->telco_selected = intVal;

	//regdomain
	apmib_get(MIB_HW_REG_DOMAIN, (void *)&intVal);
	wifi_mib->regdomain = intVal;

	//led_type
	/*
#ifdef CONFIG_WIFI_LED_USE_SOC_GPIO
	#define LEDTYPE 0
#elif defined(CONFIG_E8B)
	#define LEDTYPE 7
#else
	#ifdef CONFIG_00R0
		#define LEDTYPE 7
	#elif defined(CONFIG_RTL_92C_SUPPORT)
		#define LEDTYPE 11
	#else
		#define LEDTYPE 3
	#endif
#endif
	*/
	apmib_get(MIB_HW_LED_TYPE, (void *)&intVal);
	wifi_mib->led_type = intVal;

	//lifetime
#ifdef WLAN_LIFETIME_SUPPORT
	apmib_get(MIB_WLAN_LIFETIME, (void *)&intVal);
	wifi_mib->lifetime = intVal;
#endif

#ifdef WLAN_RATE_PRIOR
	apmib_get(MIB_WLAN_RATE_PRIOR, (void *)&rate_prior);
#endif

	//coexist
//	apmib_get(MIB_WLAN_COEXIST_ENABLED,(void *)&intVal);
//	wifi_mib->coexist = intVal;

	//crossband_enable
#ifdef RTK_CROSSBAND_REPEATER
	apmib_get(MIB_WLAN_CROSSBAND_ENABLE,(void *)&intVal);
	apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
	if ((wlan_mode == AP_MODE || wlan_mode == AP_MESH_MODE || wlan_mode == AP_WDS_MODE)){
		wifi_mib->crossband_enable = intVal;
	}
#endif

#if defined(CONFIG_ANDLINK_SUPPORT)
	//roaming_switch
	apmib_get(MIB_RTL_LINK_ROAMING_SWITCH,(void *)&intVal);
	wifi_mib->roaming_switch = intVal;

	//roaming_qos
	apmib_get(MIB_RTL_LINK_ROAMING_QOS,(void *)&intVal);
	wifi_mib->roaming_qos = intVal;

	//fail_ratio
	apmib_get(MIB_RTL_LINK_ROAMING_FAIL_RATIO,(void *)&intVal);
	wifi_mib->fail_ratio = intVal;
	if(!wifi_mib->fail_ratio)
		wifi_mib->fail_ratio = 100; //default

	//retry_ratio
	apmib_get(MIB_RTL_LINK_ROAMING_RETRY_RATIO,(void *)&intVal);
	wifi_mib->retry_ratio = intVal;
	if(!wifi_mib->retry_ratio)
		wifi_mib->retry_ratio = 100; //default
#endif

	//ofdma
	apmib_get(MIB_WLAN_OFDMA_ENABLED,(void *)&intVal);
	wifi_mib->ofdma_enable = intVal;

	//dfgw_mac
#if 0
	apmib_get(MIB_DEF_GW_MAC, (void *)value);
	value[6] = '\0';
	memcpy(wifi_mib->dfgw_mac, value, sizeof(wifi_mib->dfgw_mac));
#endif

	apmib_get(MIB_WLAN_MC2U_DISABLED, (void *)&intVal);
	wifi_mib->mc2u_disable = intVal;

#ifdef WLAN_ROAMING
	if(phyband==PHYBAND_5G){
		apmib_get(MIB_ROAMING5G_ENABLE, (void *)&roaming_enable);
		apmib_get(MIB_ROAMING5G_STARTTIME, (void *)&roaming_start_time);
		apmib_get(MIB_ROAMING5G_RSSI_TH1, (void *)&roaming_rssi_th1);
		apmib_get(MIB_ROAMING5G_RSSI_TH2, (void *)&roaming_rssi_th2);
	}
	else{
		apmib_get(MIB_ROAMING2G_ENABLE, (void *)&roaming_enable);
		apmib_get(MIB_ROAMING2G_STARTTIME, (void *)&roaming_start_time);
		apmib_get(MIB_ROAMING2G_RSSI_TH1, (void *)&roaming_rssi_th1);
		apmib_get(MIB_ROAMING2G_RSSI_TH2, (void *)&roaming_rssi_th2);
	}
	//roaming_enable
	wifi_mib->roaming_enable = roaming_enable;

	//roaming_start_time
	wifi_mib->roaming_start_time = roaming_start_time;

	//roaming_rssi_th1
	wifi_mib->roaming_rssi_th1 = roaming_rssi_th1;

	//roaming_rssi_th2
	wifi_mib->roaming_rssi_th2 = roaming_rssi_th2;

	//roaming_wait_time
	wifi_mib->roaming_wait_time = 3;
#endif

	/* vap & root share the same following setting ---end */
	//========

	vwlan_idx = vwlan_index;
	apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
	if(wlan_mode == CLIENT_MODE)
	{
		wifi_mib->band = root_band;
	}

	//multiap_bss_type
#ifdef RTK_HAPD_MULTI_AP
	unsigned int bss_type = 0;
	apmib_get(MIB_MAP_CONTROLLER, (void *)&intVal);
	apmib_get(MIB_WLAN_MAP_BSS_TYPE, (void *)&bss_type);
	if(intVal) {
		if(bss_type == 0) {
			bss_type = MAP_FRONTHAUL_BSS;
			apmib_set(MIB_WLAN_MAP_BSS_TYPE, (void *)&bss_type);
			apmib_update(CURRENT_SETTING);
		}
	}
	wifi_mib->multiap_bss_type = bss_type;
#endif

	//func off
	apmib_get(MIB_WLAN_FUNC_OFF, (void *)&intVal);
	wifi_mib->func_off = intVal;

	//hidden SSID
	apmib_get(MIB_WLAN_HIDDEN_SSID, (void *)&intVal);
	wifi_mib->hiddenAP = intVal;

	//gbwcmode & gbwcthrd_tx & gbwcthrd_rx
	apmib_get(MIB_WLAN_TX_RESTRICT, (void *)&tx_bandwidth);
	apmib_get(MIB_WLAN_RX_RESTRICT, (void *)&rx_bandwidth);
	if (tx_bandwidth && rx_bandwidth == 0)
		gbwcmode = GBWC_MODE_LIMIT_IF_TX;
	else if (tx_bandwidth == 0 && rx_bandwidth)
		gbwcmode = GBWC_MODE_LIMIT_IF_RX;
	else if (tx_bandwidth && rx_bandwidth)
		gbwcmode = GBWC_MODE_LIMIT_IF_TRX;
	wifi_mib->gbwcmode = gbwcmode;

#if defined(CONFIG_APP_CTCAPD)
	wifi_mib->gbwcthrd_tx = tx_bandwidth;
	wifi_mib->gbwcthrd_rx = rx_bandwidth;
#else
	wifi_mib->gbwcthrd_tx = tx_bandwidth*1024;
	wifi_mib->gbwcthrd_rx = rx_bandwidth*1024;
#endif

	//manual_priority
#ifdef CONFIG_RTK_SSID_PRIORITY
	apmib_get(MIB_WLAN_MANUAL_PRIORITY, (void *)&intVal);
	wifi_mib->manual_priority = intVal;
#endif

	//autorate
	apmib_get(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&intVal);
	wifi_mib->autorate = intVal;

	//fixrate
	if(intVal == 0)
	{
		apmib_get(MIB_WLAN_FIX_RATE, (void *)&intVal2);
		wifi_mib->fixrate = intVal2;
	}
	else
	{
		if(is_ax_support){
			wifi_mib->fixrate = 0;
		}
	}

	//deny_legacy
	apmib_get(MIB_WLAN_BAND, (void *)&band);
#ifdef WLAN_RATE_PRIOR
	if(rate_prior == 0){
#endif
		if (band == 8) { //pure 11n
			if(phyband == PHYBAND_5G) {//5G
				band += 4; // a
				mode = 4;
			}
			else{
				band += 3;	//b+g+n
				mode = 3;
			}
		}
		else if (band == 2) {	//pure 11g
			band += 1;	//b+g
			mode = 1;
		}
		else if (band == 10) {	//g+n
			band += 1;	//b+g+n
			mode = 1;
		}
		else if(band == 64) {	//pure 11ac
			band += 12; 	//a+n
			mode = 12;
		}
		else if(band == 72) {	//n+ac
			band += 4; 	//a
			mode = 4;
		}
		else mode = 0;
#ifdef WLAN_RATE_PRIOR
	}
	else{
		if(phyband == PHYBAND_5G) {//5G
			band = 76; //pure 11ac
			mode = 12;
		}
		else{
			band = 11; //pure 11n
			mode = 3;
		}
	}
#endif
	wifi_mib->deny_legacy = mode;

	//lgyEncRstrct
	wifi_mib->lgyEncRstrct = 15;

	//monitor_sta_enabled
	//be used in rtk_isp_wlan_adapter.c

	//txbf & txbf_mu
	apmib_get(MIB_WLAN_TX_BEAMFORMING,(void *)&intVal);
	wifi_mib->txbf = intVal;
	if(wifi_mib->txbf){
		apmib_get(MIB_WLAN_TXBF_MU,(void *)&intVal2);
		wifi_mib->txbf_mu = intVal2;
	}else
		wifi_mib->txbf_mu = 0;

#ifdef WLAN_INTF_TXBF_DISABLE
		//txbf must disable if enable antenna diversity
		wifi_mib->txbf = 0;
		wifi_mib->txbf_mu = 0;
#endif

	//opmode
	apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
	if(vwlan_index == WLAN_ROOT_ITF_INDEX){
		if (wlan_mode == CLIENT_MODE)
			intVal = 8; // client
		else	// 0(AP_MODE) or 3(AP_WDS_MODE)
			intVal = 16; // AP
		wifi_mib->opmode = intVal;
	}
#ifdef UNIVERSAL_REPEATER
	// Repeater opmode
	if (vwlan_index == NUM_VWLAN_INTERFACE){
		if(wlan_idx==0)
			apmib_get(MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
		else
			apmib_get(MIB_REPEATER_ENABLED2, (void *)&rpt_enabled);
		if (rpt_enabled) {
			if (wlan_mode == CLIENT_MODE)
				intVal = 8;
			else
				intVal = 16;
			wifi_mib->opmode = intVal;
		}
	}
#endif

#if 0
	//RSSIThreshold
	if(wifi_mib->RSSIThreshold != Entry.rtl_link_rssi_th)
		wifi_mib->RSSIThreshold = Entry.rtl_link_rssi_th;
#endif

	apmib_recov_wlanIdx();
}

void convert_wifi_priv_cfg(struct wifi_mib_priv *priv_mib, struct wifi_mib *wifiMib, int wlan_index, int vwlan_index)
{
	//func_off
	wifiMib->miscEntry.func_off = (unsigned int)priv_mib->func_off;
	//disable_protection
	wifiMib->dot11StationConfigEntry.protectionDisabled = (int)priv_mib->disable_protection;
	//aggregation
	//wifiMib->dot11nConfigEntry.dot11nAMPDU = (unsigned int)priv_mib->aggregation;
	//iapp_enable
	wifiMib->dot11OperationEntry.iapp_enable = (unsigned int)priv_mib->iapp_enable;
	//a4_enable
	wifiMib->miscEntry.a4_enable = priv_mib->a4_enable;
	//multiap_monitor_mode_disable
	wifiMib->multi_ap.multiap_monitor_mode_disable = priv_mib->multiap_monitor_mode_disable;
	//multiap_bss_type
	wifiMib->multi_ap.multiap_bss_type = priv_mib->multiap_bss_type;
	//gbwcmode
	wifiMib->gbwcEntry.GBWCMode = priv_mib->gbwcmode;
	//gbwcthrd_tx
	wifiMib->gbwcEntry.GBWCThrd_tx = priv_mib->gbwcthrd_tx;
	//gbwcthrd_rx
	wifiMib->gbwcEntry.GBWCThrd_rx = priv_mib->gbwcthrd_rx;
	//telco_selected
	wifiMib->miscEntry.telco_selected = priv_mib->telco_selected;
	//regdomain
	wifiMib->dot11StationConfigEntry.dot11RegDomain = (unsigned int)priv_mib->regdomain;
	//led_type
	wifiMib->dot11OperationEntry.ledtype = priv_mib->led_type;
	//manual_priority
	wifiMib->miscEntry.manual_priority = priv_mib->manual_priority;
	//autorate
	wifiMib->dot11StationConfigEntry.autoRate = priv_mib->autorate;
	//fixrate
	wifiMib->dot11StationConfigEntry.fixedTxRate = priv_mib->fixrate;
	//deny_legacy
	wifiMib->dot11StationConfigEntry.legacySTADeny = priv_mib->deny_legacy;
	//lgyEncRstrct
	wifiMib->dot11nConfigEntry.dot11nLgyEncRstrct = priv_mib->lgyEncRstrct;
	//cts2self
	wifiMib->dot11ErpInfo.ctsToSelf = priv_mib->lgyEncRstrct;
	//coexist
	//wifiMib->dot11nConfigEntry.dot11nCoexist = priv_mib->coexist;
	//ampdu
	wifiMib->dot11nConfigEntry.dot11nAMPDU = (unsigned int)priv_mib->ampdu;
	//amsdu
	wifiMib->dot11nConfigEntry.dot11nAMSDU = (unsigned int)priv_mib->amsdu;
	//crossband_enable
	wifiMib->crossBand.crossband_enable = priv_mib->crossband_enable;
	//monitor_sta_enabled
	wifiMib->dot11StationConfigEntry.monitor_sta_enabled = priv_mib->monitor_sta_enabled;
	//txbf
	wifiMib->dot11RFEntry.txbf = priv_mib->txbf;
	//txbf_mu
	wifiMib->dot11RFEntry.txbf_mu = priv_mib->txbf_mu;
	//roaming_switch
	wifiMib->rlr_profile.roaming_switch = priv_mib->roaming_switch;
	//roaming_qos
	wifiMib->rlr_profile.roaming_qos = priv_mib->roaming_qos;
	//fail_ratio
	wifiMib->rlr_profile.fail_ratio = priv_mib->fail_ratio;
	//retry_ratio
	wifiMib->rlr_profile.retry_ratio = priv_mib->retry_ratio;
	//RSSIThreshold
	wifiMib->rlr_profile.RSSIThreshold = (unsigned int)priv_mib->RSSIThreshold;
	//roaming_enable
	wifiMib->roamingEntry.roaming_enable = priv_mib->roaming_enable;
	//roaming_start_time
	wifiMib->roamingEntry.roaming_st_time = priv_mib->roaming_start_time;
	//roaming_rssi_th1
	wifiMib->roamingEntry.roaming_rssi_th1 = priv_mib->roaming_rssi_th1;
	//roaming_rssi_th2
	wifiMib->roamingEntry.roaming_rssi_th2 = priv_mib->roaming_rssi_th2;
	//roaming_wait_time
	wifiMib->roamingEntry.roaming_wait_time = priv_mib->roaming_wait_time;
	//lifetime
	wifiMib->dot11OperationEntry.lifetime = priv_mib->lifetime;
	//opmode
	wifiMib->dot11OperationEntry.opmode = priv_mib->opmode;

	if(priv_mib->opmode == 8 && priv_mib->band != 0)
	{
		wifiMib->dot11BssType.net_work_type = priv_mib->band;
	}
}

void start_wifi_priv_cfg(int wlan_index, int vwlan_index)
{
	int i, j, skfd, count;
	struct iwreq wrq;
	char ifname[16]={0};
	struct wifi_mib_priv priv_mib;
	struct wifi_mib *wifiMib=NULL;
	unsigned int wlanDisabled = 1;
	unsigned int is_ax_support = 0;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return;

	apmib_save_wlanIdx();

	i = wlan_idx = wlan_index;
	vwlan_idx = 0;
	apmib_get(MIB_WLAN_AX_SUPPORT, (void *)&is_ax_support);

	j = vwlan_idx = vwlan_index;
	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisabled);

	if (wlanDisabled){
		goto exit;
	}

	memset(ifname,0,sizeof(ifname));
	if(j == 0)
		snprintf(ifname, sizeof(ifname), WLAN_IF_S[i]);
	else
	{
#ifdef UNIVERSAL_REPEATER
		if(j == NUM_VWLAN_INTERFACE)
			snprintf(ifname, sizeof(ifname), VXD_IF[i]);
		else
#endif
			snprintf(ifname, sizeof(ifname), VAP_IF, i, j-1);
	}

	printf("[%s %d] (%s) +++\n", __FUNCTION__,__LINE__, ifname);
	memset(&priv_mib,0,sizeof(struct wifi_mib_priv));

	/* Get wireless name, Do 10 times, if failed */
	if(rtk_get_interface_flag(ifname, TIMER_COUNT, IS_EXIST) == 0)
	{
		goto exit;
	}

	if(is_ax_support)
	{
		if(get_wifi_mib_priv(skfd,ifname,&priv_mib)==0)
		{
			if(strcmp(priv_mib.rtw_mib_version, RTW_WIFI_MIB_VERSION) || (priv_mib.rtw_mib_size != sizeof(struct wifi_mib_priv))) {
				printf("%s(%s): The mib version or struct size is mismatch!!!\n", __FUNCTION__, ifname);
				goto exit;
			}
			gen_wifi_priv_cfg(&priv_mib, i, j);

			if(set_wifi_mib_priv(skfd,ifname,&priv_mib) != 0)
			{
				printf("%s(%s): set_wifi_priv_mib fail.\n", __FUNCTION__, ifname);
				goto exit;
			}
		}
		else
		{
			printf("%s(%s): get_wifi_priv_mib fail.\n", __FUNCTION__, ifname);
			goto exit;
		}
	}
	else
	{
		if ((wifiMib = (struct wifi_mib *)malloc(sizeof(struct wifi_mib))) == NULL) {
			printf("MIB buffer allocation failed!\n");
			goto exit;
		}

		memset(wifiMib, 0, sizeof(struct wifi_mib));
		if(get_wifi_mib(skfd,ifname,wifiMib)==0)
		{
			gen_wifi_priv_cfg(&priv_mib, i, j);
			convert_wifi_priv_cfg(&priv_mib, wifiMib, i, j);
			if(set_wifi_mib(skfd,ifname,wifiMib) != 0)
			{
				printf("%s(%s): set_wifi_priv_mib fail.\n", __FUNCTION__, ifname);
				goto exit;
			}
		}
		else
		{
			printf("%s(%s): get_wifi_priv_mib fail.\n", __FUNCTION__, ifname);
			goto exit;
		}
	}

exit:
	close( skfd );
	if(wifiMib != NULL)
		free(wifiMib);
	apmib_recov_wlanIdx();
	return;
}
