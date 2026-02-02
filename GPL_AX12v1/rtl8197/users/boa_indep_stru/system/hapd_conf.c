#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
//#include <linux/wireless.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "apmib.h"
#include "sysconf.h"
#include "sys_utility.h"
#include "hapd_conf.h"
#include "hapd_defs.h"
//#include "wifi6_priv_conf.h"

#ifdef WLAN_WPS_HAPD
static char uuid_def[]="12345678-9abc-def0-1234-56789abcdef0";
#endif

static void wlan_get_mib(unsigned int mibID, int wlan_index, int vwlan_index, void *value)
{
	if(wlan_index<0 || wlan_index>=NUM_WLAN_INTERFACE){
		printf("(%s)wrong wlan_idx:%d\n", __FUNCTION__,wlan_index);
		return;
	}
	if(vwlan_index<0 || vwlan_index>NUM_VWLAN_INTERFACE){
		printf("(%s)wrong vwlan_index:%d\n", __FUNCTION__,vwlan_index);
		return;
	}

	apmib_save_wlanIdx();
	wlan_idx = wlan_index;
	vwlan_idx = vwlan_index;
	apmib_get(mibID, value);
	apmib_recov_wlanIdx();
}

static void wlan_set_mib(unsigned int mibID, int wlan_index, int vwlan_index, void *value)
{
	if(wlan_index<0 || wlan_index>=NUM_WLAN_INTERFACE){
		printf("(%s)wrong wlan_idx:%d\n", __FUNCTION__,wlan_index);
		return;
	}
	if(vwlan_index<0 || vwlan_index>NUM_VWLAN_INTERFACE){
		printf("(%s)wrong vwlan_index:%d\n", __FUNCTION__,vwlan_index);
		return;
	}

	apmib_save_wlanIdx();
	wlan_idx = wlan_index;
	vwlan_idx = vwlan_index;
	apmib_set(mibID, value);
	apmib_recov_wlanIdx();
}

static void convert_bin_to_str(unsigned char *bin, int len, char *out)
{
	int i;
	char tmpbuf[10];

	out[0] = '\0';

	for (i=0; i<len; i++) {
		sprintf(tmpbuf, "%02x", bin[i]);
		strcat(out, tmpbuf);
	}
}

static void convert_str_to_ascii(char *str, unsigned char *strhex)
{
	int i,cnt=0;
	char *p = str;
	int len = strlen(str);

	while(*p != '\0') {
		for (i = 0; i < len; i ++)
		{
			if ((*p >= '0') && (*p <= '9'))
				strhex[cnt] = *p - '0' + 0x30;

			if ((*p >= 'A') && (*p <= 'Z'))
				strhex[cnt] = *p - 'A' + 0x41;

			if ((*p >= 'a') && (*p <= 'z'))
				strhex[cnt] = *p - 'a' + 0x61;

			p++;
			cnt++;
		}
	}
}

/* Change the mac address from "xx:xx:xx:xx:xx:xx" to "xxxxxxxxxxxx"*/
int changeMacFormatWlan(char *str, char s)
{
	int i = 0, j = 0;
	int length = strlen(str);
	for(i = 0; i < length; i++)
	{
		if(str[i] == s)
		{
			for(j = i; j<length; ++j)
				str[j] = str[j+1];
			str[j] = '\0';
		}
	}
	return 0;
}

/*setup mac addr if mac[n-1]++ > 0xff and set mac[n-2]++, etc.*/
void setup_mac_addr(unsigned char *macAddr, int index)
{
	if((macAddr[5]+index) > 0xff)
	{
		if((macAddr[4]+1) > 0xff)
		{
			if((macAddr[3]+1) > 0xff)
			{
				printf("something wrong for setting your macAddr[3] > 0xff!!\n");
			}
			macAddr[3]+=1;
		}
		macAddr[4]+=1;
	}
	macAddr[5]+=index;
}

static int get_wlan_mac(unsigned int wlan_index, unsigned int vwlan_index, unsigned char *mac)
{
	unsigned char tmpbuf[6]={0}, ifname[20]={0};
	int ret=0;
	unsigned int is_ax_support=0;

	apmib_save_wlanIdx();

	wlan_idx = wlan_index;
	vwlan_idx = vwlan_index;

	/*
		MIB_WLAN_WLAN_MAC_ADDR is all 0 in default,
		derived by MIB_ELAN_MAC_ADDR in fmtcpip.c
	*/
	apmib_get(MIB_WLAN_WLAN_MAC_ADDR, (void *)tmpbuf);
	if (!memcmp(tmpbuf, "\x00\x00\x00\x00\x00\x00", 6)) {
#ifdef WLAN_MAC_FROM_EFUSE
		if( !get_root_mac(ifname,tmpbuf) ){
#ifdef MBSSID
			if( vwlan_idx > 0 ) {
				if (*(char *)(ifname+4) == '0' ) //wlan0
	    		  *(char *)tmpbuf |= LOCAL_ADMIN_BIT;
	    		else {
	    			(*(char *)tmpbuf) += 4;
	    			(*(char *)tmpbuf) |= LOCAL_ADMIN_BIT;
	    		}
			}
			calc_incr((char *)tmpbuf+MACADDRLEN-1,vwlan_idx);
#endif
		} else
#endif
		{
#ifdef MBSSID
			if (vwlan_idx > 0 && vwlan_idx != NUM_VWLAN_INTERFACE) {
				switch (vwlan_idx)
				{
					case 1:
						apmib_get(MIB_HW_WLAN_ADDR1, (void *)tmpbuf);
						break;
					case 2:
						apmib_get(MIB_HW_WLAN_ADDR2, (void *)tmpbuf);
						break;
					case 3:
						apmib_get(MIB_HW_WLAN_ADDR3, (void *)tmpbuf);
						break;
					case 4:
						apmib_get(MIB_HW_WLAN_ADDR4, (void *)tmpbuf);
						break;
					default:
						printf("Fail to get MAC address of VAP%d!\n", vwlan_idx-1);
						ret = -1;
						goto exit;
				}
			}
			else
#endif
			{
				if(vwlan_idx ==NUM_VWLAN_INTERFACE){
					vwlan_idx = 0;
					apmib_get(MIB_WLAN_WLAN_MAC_ADDR, (void *)tmpbuf); //vxd mac derived by root mac
					if (!memcmp(tmpbuf, "\x00\x00\x00\x00\x00\x00", 6)){
						apmib_get(MIB_HW_WLAN_ADDR5, (void *)tmpbuf);
					}else
						setup_mac_addr(tmpbuf, NUM_VWLAN_INTERFACE);
				}else
					apmib_get(MIB_HW_WLAN_ADDR, (void *)tmpbuf);
			}
		}
	}

	memcpy(mac, tmpbuf, sizeof(tmpbuf));

exit:
	apmib_recov_wlanIdx();
	return ret;
}

#if defined(MBSSID)
static int get_wifi_vif_conf(struct wifi_config *vap_config, int wlan_index, int index)
{
	unsigned int 	tmp32, intVal=0;
	unsigned char	tmpbuf[50];
	unsigned char	tmp_bssid[50];
	char		*name_interface;
#ifdef RTK_HAPD_MULTI_AP
	unsigned char mode=0;
	unsigned char bss_type = 0; 
#endif
	unsigned int wpaAuth=0, encrypt=0, wep=0, wepDefKey=0, wepKeytype=0;

	apmib_save_wlanIdx();

	wlan_get_mib(MIB_WLAN_WLAN_DISABLED, wlan_index, index, (void *)&tmp32);
	if(tmp32){
		vap_config->disabled = 1;
		apmib_recov_wlanIdx();
		return 0;
	}

	printf("[%s][%d][%d-%d] ++ \n", __FUNCTION__, __LINE__, wlan_index, index-1);
	snprintf(vap_config->interface, sizeof(vap_config->interface) - 1, "%s-" VAP_IF_ONLY, WLAN_IF_S[wlan_index], index-1);
	strcpy(vap_config->ctrl_interface, NAME_CTRL_IFACE);
	strcpy(vap_config->bridge, NAME_BR);

	wlan_get_mib(MIB_WLAN_PHY_BAND_SELECT, wlan_index, 0, (void *)&intVal);
	vap_config->phy_band_select = intVal;

	//ssid
	wlan_get_mib(MIB_WLAN_SSID, wlan_index, index, (void *)vap_config->ssid);

	//Security
	wlan_get_mib(MIB_WLAN_WPA_AUTH, wlan_index, index, (void *)&wpaAuth);
	wlan_get_mib(MIB_WLAN_ENCRYPT, wlan_index, index, (void *)&encrypt);
	if(wpaAuth==WPA_AUTH_PSK)
		vap_config->psk_enable = encrypt/2;
	wlan_get_mib(MIB_WLAN_WPA_PSK, wlan_index, index, (void *)vap_config->passphrase);
	wlan_get_mib(MIB_WLAN_WPA_GROUP_REKEY_TIME, wlan_index, index, (void *)&vap_config->gk_rekey);
	wlan_get_mib(MIB_WLAN_AUTH_TYPE, wlan_index, index, (void *)&intVal);
	vap_config->authtype = intVal;
	wlan_get_mib(MIB_WLAN_PSK_FORMAT, wlan_index, index, (void *)&intVal);
	vap_config->psk_format = intVal;
	wlan_get_mib(MIB_WLAN_WPA_CIPHER_SUITE, wlan_index, index, (void *)&intVal);
	vap_config->wpa_cipher = intVal;
	wlan_get_mib(MIB_WLAN_WPA2_CIPHER_SUITE, wlan_index, index, (void *)&intVal);
	vap_config->wpa2_cipher = intVal;
	wlan_get_mib(MIB_WLAN_ENABLE_1X, wlan_index, index, (void *)&intVal);
	vap_config->enable_1x = intVal;
	if(wpaAuth == WPA_AUTH_AUTO && encrypt != ENCRYPT_DISABLED && encrypt != ENCRYPT_WEP)
		vap_config->enable_1x = 1;
#ifdef CONFIG_IEEE80211W
	wlan_get_mib(MIB_WLAN_IEEE80211W, wlan_index, index, (void *)&intVal);
	vap_config->dot11IEEE80211W = intVal;
	wlan_get_mib(MIB_WLAN_SHA256_ENABLE, wlan_index, index, (void *)&intVal);
	vap_config->enableSHA256 = intVal;
#endif

	if(encrypt == ENCRYPT_WEP){
		//config->wep_default_key = Entry.wepDefaultKey;
		wlan_get_mib(MIB_WLAN_WEP, wlan_index, index, (void *)&wep);
		wlan_get_mib(MIB_WLAN_WEP_DEFAULT_KEY, wlan_index, index, (void *)&wepDefKey);
		wlan_get_mib(MIB_WLAN_WEP_KEY_TYPE, wlan_index, index, (void *)&wepKeytype);
		vap_config->wepKeyType = wepKeytype;
		if(wep == WEP64){
			vap_config->encmode = _WEP_40_PRIVACY_;
			wlan_get_mib(MIB_WLAN_WEP64_KEY1, wlan_index, index, (void *)vap_config->wepkey0);
			wlan_get_mib(MIB_WLAN_WEP64_KEY2, wlan_index, index, (void *)vap_config->wepkey1);
			wlan_get_mib(MIB_WLAN_WEP64_KEY3, wlan_index, index, (void *)vap_config->wepkey2);
			wlan_get_mib(MIB_WLAN_WEP64_KEY4, wlan_index, index, (void *)vap_config->wepkey3);
			if(wepDefKey == 1)
				memcpy(vap_config->wepkey0, vap_config->wepkey1, WEP64_KEY_LEN);
			else if(wepDefKey == 2)
				memcpy(vap_config->wepkey0, vap_config->wepkey2, WEP64_KEY_LEN);
			else if(wepDefKey == 3)
				memcpy(vap_config->wepkey0, vap_config->wepkey3, WEP64_KEY_LEN);
		}
		else if(wep == WEP128){
			vap_config->encmode = _WEP_104_PRIVACY_;
			wlan_get_mib(MIB_WLAN_WEP128_KEY1, wlan_index, index, (void *)vap_config->wepkey0);
			wlan_get_mib(MIB_WLAN_WEP128_KEY2, wlan_index, index, (void *)vap_config->wepkey1);
			wlan_get_mib(MIB_WLAN_WEP128_KEY3, wlan_index, index, (void *)vap_config->wepkey2);
			wlan_get_mib(MIB_WLAN_WEP128_KEY4, wlan_index, index, (void *)vap_config->wepkey3);
			if(wepDefKey == 1)
				memcpy(vap_config->wepkey0, vap_config->wepkey1, WEP128_KEY_LEN);
			else if(wepDefKey == 2)
				memcpy(vap_config->wepkey0, vap_config->wepkey2, WEP128_KEY_LEN);
			else if(wepDefKey == 3)
				memcpy(vap_config->wepkey0, vap_config->wepkey3, WEP128_KEY_LEN);
		}
	}
	else if(encrypt)
		vap_config->encmode = _IEEE8021X_PSK_;
	else
		vap_config->encmode = _NO_PRIVACY_;

#if 0
	printf("VAP%u: psk_enable=%d authtype=%d encmode=%d psk_format=%d gk_rekey=%d\n",
	       index-1,
	       vap_config->psk_enable, vap_config->authtype,
	       vap_config->encmode, vap_config->psk_format, vap_config->gk_rekey);

	printf("VAP%u: passphrase=%s \n", index-1, vap_config->passphrase);

	printf("VAP%u: wpa_cipher=0x%x wpa2_cipher=0x%x enable_1x=%d\n",
	       index-1, vap_config->wpa_cipher, vap_config->wpa2_cipher, vap_config->enable_1x);
#ifdef CONFIG_IEEE80211W
	printf("VAP%u: dot11IEEE80211W=%d enableSHA256=%d\n",
	       index-1, vap_config->dot11IEEE80211W, vap_config->enableSHA256);
#endif
#endif
//RADIUS
	apmib_get(MIB_IP_ADDR, (void *)vap_config->lan_ip);
	wlan_get_mib(MIB_WLAN_RS_PORT, wlan_index, index, (void *)&intVal);
	vap_config->rsPort = intVal;
	wlan_get_mib(MIB_WLAN_RS_IP, wlan_index, index, (void *)vap_config->rsIpAddr);
	wlan_get_mib(MIB_WLAN_RS_PASSWORD, wlan_index, index, (void *)vap_config->rsPassword);

	/*
	// branch 3.4.11 does not include the following mib
	vap_config->rs2Port = Entry.rs2Port;
	memcpy(vap_config->rs2IpAddr, Entry.rs2IpAddr, sizeof(Entry.rs2IpAddr));
	memcpy(vap_config->rs2Password, Entry.rs2Password, sizeof(Entry.rs2Password));
	*/
	wlan_get_mib(MIB_WLAN_RS_INTERVAL_TIME, wlan_index, 0, (void *)&tmp32);
	vap_config->radius_retry_primary_interval = tmp32*60;
	wlan_get_mib(MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME, wlan_index, 0, (void *)&tmp32);
	vap_config->radius_acct_interim_interval = tmp32*60;

//Bssid
	memset(tmp_bssid, 0, sizeof(tmp_bssid));
	get_wlan_mac(wlan_index, index, tmp_bssid);
	sprintf(vap_config->bssid, "%02x:%02x:%02x:%02x:%02x:%02x", tmp_bssid[0], tmp_bssid[1],
		tmp_bssid[2], tmp_bssid[3], tmp_bssid[4], tmp_bssid[5]);

//Operation Mode
	wlan_get_mib(MIB_WLAN_BAND, wlan_index, index, (void *)&tmp32);
	vap_config->wlan_band = tmp32;

//Advanced
	wlan_get_mib(MIB_WLAN_DTIM_PERIOD, wlan_index, 0, (void *)&intVal);
	vap_config->dtimperiod = intVal;
	wlan_get_mib(MIB_WLAN_HIDDEN_SSID, wlan_index, index,(void *)&intVal);
	vap_config->ignore_broadcast_ssid = intVal;
	wlan_get_mib(MIB_WLAN_WMM_ENABLED, wlan_index, index,(void *)&intVal);
	vap_config->qos = intVal;
	wlan_get_mib(MIB_WLAN_ACCESS, wlan_index, index, (void *)&intVal);
	vap_config->guest_access = intVal;
	if(vap_config->guest_access)
		vap_config->ap_isolate = 1;
//11k
#ifdef DOT11K
	wlan_get_mib(MIB_WLAN_DOT11K_ENABLE, wlan_index, index, (void *)&intVal);
	vap_config->dot11k_enable = intVal;
#endif
//11v
#ifdef CONFIG_IEEE80211V
	wlan_get_mib(MIB_WLAN_DOT11V_ENABLE, wlan_index, index, (void *)&intVal);
	vap_config->dot11v_enable = intVal;
#endif
//11r
#ifdef FAST_BSS_TRANSITION
	wlan_get_mib(MIB_WLAN_FT_MDID, wlan_index, index, (void *)vap_config->ft_mdid);
	wlan_get_mib(MIB_WLAN_FT_OVER_DS, wlan_index, index, (void *)&intVal);
	vap_config->ft_over_ds = intVal;
	wlan_get_mib(MIB_WLAN_FT_PUSH, wlan_index, index, (void *)&intVal);
	vap_config->ft_push = intVal;
	wlan_get_mib(MIB_WLAN_FT_R0KH_ID, wlan_index, index, (void *)vap_config->ft_r0kh_id);
#endif

//station
#if 0 //TODO
#if 1//defined(CONFIG_ELINK_SUPPORT) || defined(CONFIG_ANDLINK_SUPPORT) || defined(CONFIG_USER_ADAPTER_API_ISP)
	vap_config->max_num_sta = Entry.rtl_link_sta_num;
#endif
#ifdef WLAN_LIMITED_STA_NUM
	if ((vap_config->max_num_sta) && (Entry.stanum < vap_config->max_num_sta))
		vap_config->max_num_sta = Entry.stanum;
	else if (!vap_config->max_num_sta)
		vap_config->max_num_sta = Entry.stanum;
#endif
#endif

//TODO
#ifdef RTK_HAPD_MULTI_AP
	apmib_get(MIB_MAP_CONTROLLER, (void *)&(mode));
	if(mode == 1 || mode == 129 || mode == 130 || mode == 131 || mode == 132 || mode == 133 || mode == 134)
	{
		wlan_get_mib(MIB_WLAN_MAP_BSS_TYPE, wlan_index, index, (void *)&intVal);
		bss_type = intVal;
		vap_config->multi_ap = 0;
		if(bss_type & 0x20){
			vap_config->multi_ap += 2;
		}
		if(bss_type & 0x40)
		{
			vap_config->multi_ap += 1;
			snprintf(vap_config->multi_ap_backhaul_ssid, sizeof(vap_config->multi_ap_backhaul_ssid), "%s", vap_config->ssid);
			snprintf(vap_config->multi_ap_backhaul_passphrase, sizeof(vap_config->multi_ap_backhaul_passphrase), "%s", vap_config->passphrase);
			//strcpy(vap_config->multi_ap_backhaul_passphrase, vap_config->passphrase);
			vap_config->multi_ap_backhaul_psk_format = vap_config->psk_format;
		}
	}
#endif

	apmib_recov_wlanIdx();
	return 0;
}
#endif /* MBSSID */

/*
	get_wifi_conf:
		get wifi_conf to generate hostapd config file
		based on boa mib
*/
int get_wifi_conf(struct wifi_config *config, int if_idx)
{
	unsigned short 	tmp16;
	unsigned int 	tmp32;
	unsigned char tmpbuf[50]={0}, tmpbuf1[50]={0};
	unsigned char tmpMac[6]={0};
	char *name_interface;
	int	entryNum, i,j=0, is_mib_change=0;
	char *rates_list[12] = {"10", "20", "55", "110", "60", "90", "120", "180", "240", "360", "480", "540"};
#ifdef RTK_HAPD_MULTI_AP
	unsigned char mode=0;
	unsigned char bss_type = 0;
#endif
	unsigned int wlanMode=0, vwlan_index=0, intVal=0, intVal2=0;
	unsigned int wpaAuth=0, encrypt=0;
	MACFILTER_T *pAcl=NULL;
#ifdef FAST_BSS_TRANSITION
	FTKH_T *pFtKH=NULL;
#endif
	unsigned int wep=0, wepDefKey=0, wepKeytype=0;

	printf("[%s][%d][if_idx = %d] rpt:%d +++ \n", __FUNCTION__, __LINE__,if_idx,config->is_repeater);

	apmib_save_wlanIdx();

#ifdef UNIVERSAL_REPEATER
	if(config->is_repeater)
	{
		vwlan_index = NUM_VWLAN_INTERFACE;
	}
	else
#endif
	{
		vwlan_index = 0;
		memset(config, 0, sizeof(struct wifi_config));
	}

	strcpy(config->driver, NAME_DRIVER);

	name_interface = NAME_INTF(if_idx);

	strcpy(config->interface, name_interface);

	wlan_get_mib(MIB_WLAN_MODE, if_idx, vwlan_index, (void *)&wlanMode);
	config->wlan_mode = wlanMode;

#ifdef UNIVERSAL_REPEATER
	if(config->is_repeater)
	{
		strcpy(config->ctrl_interface, WPAS_NAME_CTRL_IFACE);

	}
	else
#endif
	{
		if(wlanMode == AP_MODE)
			strcpy(config->ctrl_interface, NAME_CTRL_IFACE);
		else if(wlanMode == CLIENT_MODE)
			strcpy(config->ctrl_interface, WPAS_NAME_CTRL_IFACE);
	}
	strcpy(config->bridge, NAME_BR);

	wlan_get_mib(MIB_WLAN_SSID, if_idx, vwlan_index, (void *)config->ssid);
	wlan_get_mib(MIB_WLAN_CHANNEL, if_idx, 0, (void *)&intVal);
	config->channel = intVal;
	if(config->channel==0)
		config->auto_chan_sel = 1;
	else
		config->auto_chan_sel = 0;

	wlan_get_mib(MIB_WLAN_CHANNEL_BONDING, if_idx, 0, (void *)&intVal);
	config->channel_bandwidth = intVal;
	wlan_get_mib(MIB_WLAN_CONTROL_SIDEBAND, if_idx, 0, (void *)&intVal);
	config->control_band = intVal;
	wlan_get_mib(MIB_WLAN_PHY_BAND_SELECT, if_idx, 0, (void *)&intVal);
	config->phy_band_select = intVal;
	wlan_get_mib(MIB_WLAN_WLAN_DISABLED, if_idx, vwlan_index, (void *)&intVal);
	config->disabled = intVal;
	wlan_get_mib(MIB_WLAN_COEXIST_ENABLED, if_idx, 0, (void *)&intVal);
	config->coexist = intVal;

//hostapd ACS chanlist
//2.4G only for channel 1,6,11
//5G only for non-DFS channel
	int radio_index_2g, radio_index_5g;
#if defined (CONFIG_BAND_2G_ON_WLAN0)
	radio_index_2g = 0;
	radio_index_5g = 1;
#else
	radio_index_2g = 1;
	radio_index_5g = 0;
#endif
	if (if_idx == radio_index_5g)
	{
		if(config->channel_bandwidth == CHANNEL_WIDTH_40 || config->channel_bandwidth == CHANNEL_WIDTH_80)
			strcpy(config->channel_list, "36-48 149-161");
		else
			strcpy(config->channel_list, "36-48 149-165");
	}
	else if (if_idx == radio_index_2g)
		strcpy(config->channel_list, "1 6 11");

//Security
	wlan_get_mib(MIB_WLAN_WPA_AUTH, if_idx, vwlan_index, (void *)&wpaAuth);
	wlan_get_mib(MIB_WLAN_ENCRYPT, if_idx, vwlan_index, (void *)&encrypt);
	if(wpaAuth==WPA_AUTH_PSK)
		config->psk_enable = encrypt/2;
	wlan_get_mib(MIB_WLAN_WPA_PSK, if_idx, vwlan_index, (void *)config->passphrase);
	wlan_get_mib(MIB_WLAN_WPA_GROUP_REKEY_TIME, if_idx, vwlan_index, (void *)&config->gk_rekey);

	wlan_get_mib(MIB_WLAN_AUTH_TYPE, if_idx, vwlan_index, (void *)&intVal);
	config->authtype = intVal;
	wlan_get_mib(MIB_WLAN_PSK_FORMAT, if_idx, vwlan_index, (void *)&intVal);
	config->psk_format = intVal;
	wlan_get_mib(MIB_WLAN_WPA_CIPHER_SUITE, if_idx, vwlan_index, (void *)&intVal);
	config->wpa_cipher = intVal;
	wlan_get_mib(MIB_WLAN_WPA2_CIPHER_SUITE, if_idx, vwlan_index, (void *)&intVal);
	config->wpa2_cipher = intVal;
	wlan_get_mib(MIB_WLAN_ENABLE_1X, if_idx, vwlan_index, (void *)&intVal);
	config->enable_1x = intVal;
	if(wpaAuth == WPA_AUTH_AUTO && encrypt != ENCRYPT_DISABLED && encrypt != ENCRYPT_WEP)
		config->enable_1x = 1;
#ifdef CONFIG_IEEE80211W
	wlan_get_mib(MIB_WLAN_IEEE80211W, if_idx, vwlan_index, (void *)&intVal);
	config->dot11IEEE80211W = intVal;
	wlan_get_mib(MIB_WLAN_SHA256_ENABLE, if_idx, vwlan_index, (void *)&intVal);
	config->enableSHA256 = intVal;
#endif
	if(encrypt == ENCRYPT_WEP){
		//config->wep_default_key = Entry.wepDefaultKey;
		wlan_get_mib(MIB_WLAN_WEP, if_idx, vwlan_index, (void *)&wep);
		wlan_get_mib(MIB_WLAN_WEP_DEFAULT_KEY, if_idx, vwlan_index, (void *)&wepDefKey);
		wlan_get_mib(MIB_WLAN_WEP_KEY_TYPE, if_idx, vwlan_index, (void *)&wepKeytype);
		config->wepKeyType = wepKeytype;
		if(wep == WEP64){
			config->encmode = _WEP_40_PRIVACY_;
			wlan_get_mib(MIB_WLAN_WEP64_KEY1, if_idx, vwlan_index, (void *)config->wepkey0);
			wlan_get_mib(MIB_WLAN_WEP64_KEY2, if_idx, vwlan_index, (void *)config->wepkey1);
			wlan_get_mib(MIB_WLAN_WEP64_KEY3, if_idx, vwlan_index, (void *)config->wepkey2);
			wlan_get_mib(MIB_WLAN_WEP64_KEY4, if_idx, vwlan_index, (void *)config->wepkey3);
			if(wepDefKey == 1)
				memcpy(config->wepkey0, config->wepkey1, WEP64_KEY_LEN);
			else if(wepDefKey == 2)
				memcpy(config->wepkey0, config->wepkey2, WEP64_KEY_LEN);
			else if(wepDefKey == 3)
				memcpy(config->wepkey0, config->wepkey3, WEP64_KEY_LEN);
		}
		else if(wep == WEP128){
			config->encmode = _WEP_104_PRIVACY_;
			wlan_get_mib(MIB_WLAN_WEP128_KEY1, if_idx, vwlan_index, (void *)config->wepkey0);
			wlan_get_mib(MIB_WLAN_WEP128_KEY2, if_idx, vwlan_index, (void *)config->wepkey1);
			wlan_get_mib(MIB_WLAN_WEP128_KEY3, if_idx, vwlan_index, (void *)config->wepkey2);
			wlan_get_mib(MIB_WLAN_WEP128_KEY4, if_idx, vwlan_index, (void *)config->wepkey3);
			if(wepDefKey == 1)
				memcpy(config->wepkey0, config->wepkey1, WEP128_KEY_LEN);
			else if(wepDefKey == 2)
				memcpy(config->wepkey0, config->wepkey2, WEP128_KEY_LEN);
			else if(wepDefKey == 3)
				memcpy(config->wepkey0, config->wepkey3, WEP128_KEY_LEN);
		}
	}
	else if(encrypt)
		config->encmode = _IEEE8021X_PSK_;
	else
		config->encmode = _NO_PRIVACY_;

#if 0
	printf("psk_enable=%d authtype=%d encmode=%d psk_format=%d gk_rekey=%d\n",
		config->psk_enable, config->authtype,
		config->encmode, config->psk_format, config->gk_rekey);

	printf("passphrase:%s \n", config->passphrase);

	printf("wpa_cipher=0x%x wpa2_cipher=0x%x enable_1x=%d\n", config->wpa_cipher, config->wpa2_cipher, config->enable_1x);
#ifdef CONFIG_IEEE80211W
	printf("dot11IEEE80211W=%d enableSHA256=%d\n", config->dot11IEEE80211W, config->enableSHA256);
#endif
#endif

//RADIUS
	apmib_get(MIB_IP_ADDR, (void *)config->lan_ip);
	wlan_get_mib(MIB_WLAN_RS_PORT, if_idx, vwlan_index, (void *)&intVal);
	config->rsPort = intVal;
	wlan_get_mib(MIB_WLAN_RS_IP, if_idx, vwlan_index, (void *)config->rsIpAddr);
	wlan_get_mib(MIB_WLAN_RS_PASSWORD, if_idx, vwlan_index, (void *)config->rsPassword);

	/*
	// branch 3.4.11 does not include the following mib
	config->rs2Port = Entry.rs2Port;
	memcpy(config->rs2IpAddr, Entry.rs2IpAddr, sizeof(Entry.rs2IpAddr));
	memcpy(config->rs2Password, Entry.rs2Password, sizeof(Entry.rs2Password));
	*/

	wlan_get_mib(MIB_WLAN_RS_INTERVAL_TIME, if_idx, 0, (void *)&tmp32);
	config->radius_retry_primary_interval = tmp32*60;
	wlan_get_mib(MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME, if_idx, 0, (void *)&tmp32);
	config->radius_acct_interim_interval = tmp32*60;

//Operation Mode
	wlan_get_mib(MIB_WLAN_BAND, if_idx, 0, (void *)&tmp32);
	config->wlan_band = tmp32;

//HW Setting & Features
	memset(tmpMac, 0, sizeof(tmpMac));
	get_wlan_mac(if_idx, vwlan_index, tmpMac);
	snprintf(config->bssid, sizeof(config->bssid), "%02x:%02x:%02x:%02x:%02x:%02x", tmpMac[0], tmpMac[1],tmpMac[2], tmpMac[3], tmpMac[4], tmpMac[5]);

	memset(config->country, 0, sizeof(config->country));
	wlan_get_mib(MIB_WLAN_COUNTRY_STRING, if_idx, 0, (void *)config->country);
	if(config->country[0] == '\0')
		wlan_get_mib(MIB_HW_COUNTRY_STR, if_idx, 0, (void*)config->country);
	wlan_get_mib(MIB_WLAN_WMM_ENABLED, if_idx, vwlan_index,(void *)&intVal);
	config->qos = intVal;
	wlan_get_mib(MIB_WLAN_HIDDEN_SSID, if_idx, vwlan_index,(void *)&intVal);
	config->ignore_broadcast_ssid = intVal;

//Advanced
	wlan_get_mib(MIB_WLAN_FRAG_THRESHOLD, if_idx, 0, (void *)&intVal);
	config->fragthres = intVal;
	wlan_get_mib(MIB_WLAN_RTS_THRESHOLD, if_idx, 0, (void *)&intVal);
	config->rtsthres = intVal;
	wlan_get_mib(MIB_WLAN_BEACON_INTERVAL, if_idx, 0, (void *)&intVal);
	config->bcnint = intVal;
	wlan_get_mib(MIB_WLAN_DTIM_PERIOD, if_idx, 0, (void *)&intVal);
	config->dtimperiod = intVal;
	wlan_get_mib(MIB_WLAN_PREAMBLE_TYPE, if_idx, 0, (void *)&intVal);
	config->preamble = intVal;
	wlan_get_mib(MIB_WLAN_SHORT_GI, if_idx, 0, (void *)&intVal);
	config->shortGI = intVal;
	config->qos_enable = config->qos;
	wlan_get_mib(MIB_WLAN_MC2U_DISABLED, if_idx, 0, (void *)&intVal);
	config->multicast_to_unicast=1-intVal;
	wlan_get_mib(MIB_WLAN_BLOCK_RELAY, if_idx, 0, (void *)&intVal);
	config->ap_isolate = intVal;
	wlan_get_mib(MIB_WLAN_INACTIVITY_TIME, if_idx, 0, (void *)&intVal);
	config->ap_max_inactivity = intVal/100;
#ifdef WLAN_HS2_CONFIG
	wlan_get_mib(MIB_WLAN_HS2_ENABLE, if_idx, 0, (void *)&intVal);
	config->hs20 = intVal;
#endif
	//mib_get_s(MIB_WLAN_STBC_ENABLED, &config->stbc, sizeof(config->stbc)); //TODO driver not support(Luna SDK mark)
	//mib_get_s(MIB_WLAN_LDPC_ENABLED, &config->ldpc, sizeof(config->ldpc)); //TODO driver not support(Luna SDK mark)
#ifdef TDLS_SUPPORT
	wlan_get_mib(MIB_WLAN_TDLS_PROHIBITED, if_idx, 0, (void *)&intVal);
	config->tdls_prohibit = intVal;
	wlan_get_mib(MIB_WLAN_TDLS_CS_PROHIBITED, if_idx, 0, (void *)&intVal);
	config->tdls_prohibit_chan_switch = intVal;
#endif

//guest access
	wlan_get_mib(MIB_WLAN_ACCESS, if_idx, vwlan_index, (void *)&intVal);
	config->guest_access = intVal;

#if defined(MBSSID)
	#ifdef UNIVERSAL_REPEATER
	if(config->is_repeater == 0)
	#endif
	{
		if(wlanMode ==AP_MODE){
			for (i = WLAN_VAP_ITF_INDEX; i < NUM_VWLAN_INTERFACE; i++) {
				get_wifi_vif_conf(&config[i], if_idx, i);
			}
		}else if(wlanMode ==CLIENT_MODE){
			for (i = WLAN_VAP_ITF_INDEX; i < NUM_VWLAN_INTERFACE; i++) {
				wlan_get_mib(MIB_WLAN_WLAN_DISABLED, if_idx, i, (void *)&intVal);
				if(intVal == 0)
				{
					intVal = 1;
					wlan_set_mib(MIB_WLAN_WLAN_DISABLED, if_idx, i, (void *)&intVal);
					//apmib_update(CURRENT_SETTING);
					is_mib_change = 1;
				}
			}
		}
	}
#endif

//acl
	wlan_get_mib(MIB_WLAN_MACAC_ENABLED, if_idx, 0, (void *)&intVal);
	config->macaddr_acl = intVal;
	if (intVal != 0){
		wlan_get_mib(MIB_WLAN_MACAC_NUM, if_idx, 0, (void *)&entryNum);
		if( entryNum >= MAX_WLAN_AC_NUM ){
			printf("[%s][%d] entryNum overflow ! \n", __FUNCTION__, __LINE__);
			entryNum = MAX_WLAN_AC_NUM;
		}
		for (i=0; i<entryNum; i++) {
			memset(tmpbuf, 0, sizeof(tmpbuf));
			tmpbuf[0] = i+1;
			wlan_get_mib(MIB_WLAN_MACAC_ADDR, if_idx, 0, (void *)tmpbuf);
			pAcl = (MACFILTER_T *)tmpbuf;
			memcpy(config->mac_acl_list[j]._macaddr, pAcl->macAddr,sizeof(config->mac_acl_list[j]._macaddr));
			config->mac_acl_list[j].used=1;
			j++;
		}
	}

//WPS
#ifdef WLAN_WPS_HAPD
	wlan_get_mib(MIB_WLAN_WSC_DISABLE, if_idx, 0, (void *)&intVal);
	if(intVal == 0){
#ifdef WLAN_WPA3
		if(encrypt == ENCRYPT_WPA3)
			config->wps_state = 0;
		else
#endif
		{
			wlan_get_mib(MIB_WLAN_WSC_CONFIGURED, if_idx, 0, (void *)&intVal2);
			config->wps_state = (intVal2==1)? 2:1;
		}
	}
	else
		config->wps_state = 0;

#ifdef UNIVERSAL_REPEATER
	if(config->is_repeater)
	{
		wlan_get_mib(MIB_WLAN_WSC_DISABLE, if_idx, vwlan_index, (void *)&intVal);
		if(intVal == 0)
			config->wps_state = 1;
		else
			config->wps_state = 0;
	}
#endif

	if(config->wps_state){
		// search UUID field, replace last 12 char with hw mac address to avoid client detect wps overlapping
		//wlan_get_mib(MIB_WLAN_WLAN_MAC_ADDR, if_idx, vwlan_index, (void *)tmpbuf);
		//if (!memcmp(tmpbuf, "\x00\x00\x00\x00\x00\x00", 6)) {
			apmib_get(MIB_HW_NIC0_ADDR, (void *)&tmpbuf);
		//}
		convert_bin_to_str((unsigned char *)tmpbuf, 6, tmpbuf1);
		memcpy(uuid_def+strlen(uuid_def)-12, tmpbuf1, 12);
		strcpy(config->uuid, uuid_def);

		apmib_get(MIB_DEVICE_NAME, (void *)&config->device_name);
		strcpy(config->model_name, WPS_MODEL_NAME);
		strcpy(config->model_number, WPS_MODEL_NUMBER);
		strcpy(config->serial_number, WPS_SERIAL_NUMBER);
		strcpy(config->device_type, WPS_DEVICE_TYPE);

		wlan_get_mib(MIB_WLAN_WSC_METHOD, if_idx, 0, (void *)&intVal);
		config->config_method = intVal;
		wlan_get_mib(MIB_HW_WSC_PIN, if_idx, 0, (void *)&config->ap_pin);

		strcpy(config->upnp_iface, NAME_BR);
		strcpy(config->friendly_name, WPS_FRIENDLY_NAME);
		strcpy(config->manufacturer_url, WPS_MANUFACTURER_URL);
		strcpy(config->model_url, WPS_MODEL_URL);
		config->eap_server = 1;
		strcpy(config->manufacturer, WPS_MANUFACTURER);
	}
#endif

		//multi ap backhaul bss config
#ifdef RTK_HAPD_MULTI_AP //TODO
		apmib_get(MIB_MAP_CONTROLLER, (void *)&(mode));
		if(mode == 1 || mode == 129 || mode == 131 || mode == 133)
		{
			//set up controller root interface multi_ap
			wlan_get_mib(MIB_WLAN_MAP_BSS_TYPE, if_idx, 0, (void *)&intVal);
			bss_type = intVal;
			config->multi_ap = 0;
			if (bss_type & 0x20) {
				config->multi_ap += 2;
			}
			if (bss_type & 0x40) {
				config->multi_ap += 1;
			}
			for(i=0;i<=NUM_VWLAN_INTERFACE; i++)
			{
				wlan_get_mib(MIB_WLAN_WLAN_DISABLED, if_idx, i, (void *)&intVal);
				if(intVal == 0)
				{
					wlan_get_mib(MIB_WLAN_MAP_BSS_TYPE, if_idx, i, (void *)&intVal);
					bss_type = intVal;
					if (bss_type & 0x40) {
						wlan_get_mib(MIB_WLAN_SSID, if_idx, i, (void *)config->multi_ap_backhaul_ssid);
						wlan_get_mib(MIB_WLAN_WPA_PSK, if_idx, i, (void *)config->multi_ap_backhaul_passphrase);
						wlan_get_mib(MIB_WLAN_PSK_FORMAT, if_idx, i, (void *)&config->multi_ap_backhaul_psk_format);
						break;
					}
				}
			}
		}
		else if(mode == 2 || mode == 130 || mode == 132 || mode == 134)
		{
#ifdef UNIVERSAL_REPEATER
			if(config->is_repeater)
			{
				wlan_get_mib(MIB_WLAN_WLAN_DISABLED, if_idx, NUM_VWLAN_INTERFACE, (void *)&intVal);
				if(intVal == 0)
				{
					wlan_get_mib(MIB_WLAN_MAP_BSS_TYPE, if_idx, NUM_VWLAN_INTERFACE, (void *)&intVal);
					bss_type = intVal;
					if(bss_type == 0x80)
					{
						config->multi_ap_backhaul_sta = 1;
					}
					else
						config->multi_ap_backhaul_sta = 0;
				}
			}
			else
#endif
			{
				//get root bss type
				wlan_get_mib(MIB_WLAN_MAP_BSS_TYPE, if_idx, 0, (void *)&intVal);
				bss_type = intVal;
				config->multi_ap = 0;
				if(bss_type & 0x20)
				{
					config->multi_ap += 2;
				}
				if(bss_type & 0x40)
				{
					config->multi_ap += 1;
				}
				for(i=0;i<=NUM_VWLAN_INTERFACE; i++)
				{
					wlan_get_mib(MIB_WLAN_WLAN_DISABLED, if_idx, i, (void *)&intVal);
					if(intVal == 0)
					{
						wlan_get_mib(MIB_WLAN_MAP_BSS_TYPE, if_idx, i, (void *)&intVal);
						bss_type = intVal;
						if(bss_type & 0x40)
						{
							wlan_get_mib(MIB_WLAN_SSID, if_idx, i, (void *)config->multi_ap_backhaul_ssid);
							wlan_get_mib(MIB_WLAN_WPA_PSK, if_idx, i, (void *)config->multi_ap_backhaul_passphrase);
							wlan_get_mib(MIB_WLAN_PSK_FORMAT, if_idx, i, (void *)&config->multi_ap_backhaul_psk_format);
							break;
						}
					}
				}
			}
		}
#endif

	//supported_rates
	wlan_get_mib(MIB_WLAN_SUPPORTED_RATES, if_idx, 0, (void *)&tmp32);
	memset(config->supported_rates, 0, sizeof(config->supported_rates));
	for(i=0; i<12; i++)
	{
		memset(tmpbuf, 0, sizeof(tmpbuf));
		tmp16 = tmp32 & 0x01;
		if(tmp16 == 1)
		{
			snprintf(tmpbuf, sizeof(tmpbuf), "%s ", rates_list[i]);
			strncat(config->supported_rates, tmpbuf, sizeof(tmpbuf));
		}

		tmp32 = tmp32 >> 1;
	}
	config->supported_rates[strlen(config->supported_rates)-1] = '\0';

	//basic_rates
	wlan_get_mib(MIB_WLAN_BASIC_RATES, if_idx, 0, (void *)&tmp32);
	memset(config->basic_rates, 0, sizeof(config->basic_rates));
	for(i=0; i<12; i++)
	{
		memset(tmpbuf, 0, sizeof(tmpbuf));
		tmp16 = tmp32 & 0x01;
		if(tmp16 == 1)
		{
			snprintf(tmpbuf, sizeof(tmpbuf), "%s ", rates_list[i]);
			strncat(config->basic_rates, tmpbuf, strlen(tmpbuf));
		}
		tmp32 = tmp32 >> 1;
	}
	config->basic_rates[strlen(config->basic_rates)-1] = '\0';
	//11v
#ifdef CONFIG_IEEE80211V
	wlan_get_mib(MIB_WLAN_DOT11V_ENABLE, if_idx, 0, (void *)&intVal);
	config->dot11v_enable = intVal;
#endif
	//11k
#ifdef DOT11K
	wlan_get_mib(MIB_WLAN_DOT11K_ENABLE, if_idx, 0, (void *)&intVal);
	config->dot11k_enable = intVal;
#endif
	//11r
#ifdef FAST_BSS_TRANSITION
	wlan_get_mib(MIB_WLAN_FT_ENABLE, if_idx, 0, (void *)&intVal);
	config->ft_enable = intVal;
	wlan_get_mib(MIB_WLAN_FT_MDID, if_idx, 0, (void *)config->ft_mdid);
	wlan_get_mib(MIB_WLAN_FT_OVER_DS, if_idx, 0, (void *)&intVal);
	config->ft_over_ds = intVal;
	wlan_get_mib(MIB_WLAN_FT_PUSH, if_idx, 0, (void *)&intVal);
	config->ft_push = intVal;
	wlan_get_mib(MIB_WLAN_FT_R0KH_ID, if_idx, 0, (void *)config->ft_r0kh_id);

	/*Key Holder Configuration*/
	//r1_key_holder
	memset(config->r1_key_holder, 0, sizeof(config->r1_key_holder));
	snprintf(config->r1_key_holder, sizeof(config->r1_key_holder), "%s", config->bssid);
	changeMacFormatWlan(config->r1_key_holder, ':');

	//r0kh & r1kh
	wlan_get_mib(MIB_WLAN_FTKH_NUM, if_idx, 0, (void*)&entryNum);
	if ( entryNum >= MAX_VWLAN_FTKH_NUM ){
		printf("[%s][%d] entryNum overflow ! \n", __FUNCTION__, __LINE__);
		entryNum = MAX_VWLAN_FTKH_NUM;
	}
	for (i=0; i<entryNum; i++) {
		memset(tmpbuf, 0, sizeof(tmpbuf));
		memset(tmpbuf1, 0, sizeof(tmpbuf1));
		tmpbuf[0] = i+1;
		wlan_get_mib(MIB_WLAN_FTKH, if_idx, 0, (void *)tmpbuf);
		pFtKH = (FTKH_T *)tmpbuf;
		memcpy(config->wlftkh_list[i].addr, pFtKH->macAddr,sizeof(config->wlftkh_list[i].addr));
		memcpy(config->wlftkh_list[i].r0kh_id, pFtKH->nas_id, sizeof(config->wlftkh_list[i].r0kh_id)); //TODO
		if(strlen(pFtKH->key) == 16) {
			convert_str_to_ascii(pFtKH->key, tmpbuf1);
			convert_bin_to_str(tmpbuf1, 16, config->wlftkh_list[i].key);
		}
		else if(strlen(pFtKH->key) == 32)
			memcpy(config->wlftkh_list[i].key, pFtKH->key, sizeof(config->wlftkh_list[i].key));
		else {
			printf("[%s][%d] The length of wlftkh_list key is invalid! \n", __FUNCTION__, __LINE__);
			return 1;
		}
	}
#endif

//station
#if 0 //TODO
#if 1//defined(CONFIG_ELINK_SUPPORT) || defined(CONFIG_ANDLINK_SUPPORT) || defined(CONFIG_USER_ADAPTER_API_ISP)
	config->max_num_sta = Entry.rtl_link_sta_num;
#endif
#ifdef WLAN_LIMITED_STA_NUM
	if ((config->max_num_sta) && (Entry.stanum < config->max_num_sta))
		config->max_num_sta = Entry.stanum;
	else if (!config->max_num_sta)
		config->max_num_sta = Entry.stanum;
#endif
#endif

	if(config->wlan_mode == CLIENT_MODE || config->is_repeater)
	{
		config->update_config = 1;
		config->scan_ssid = 1;
		config->pbss = 2;
	}

	apmib_recov_wlanIdx();

	if(is_mib_change)
		apmib_update(CURRENT_SETTING);

	return 0;
}

int apply_radius(struct wifi_config *config)
{
	if(config->enable_1x){

		if(config->psk_enable)
			return NO_RADIUS;

		if(config->encmode==_WEP_40_PRIVACY_ || config->encmode==_WEP_104_PRIVACY_)
			return RADIUS_WEP;

		if(config->wpa_cipher || config->wpa2_cipher || config->wpa3_cipher)
			return RADIUS_WPA;

		return RADIUS_OPEN;
	}
	else
	return NO_RADIUS;
}

int apply_wpa(struct wifi_config *config)
{
	if(config->psk_enable || apply_radius(config)==RADIUS_WPA) {
		return 1;
	}
	else
		return 0;
}

#ifdef WLAN_WPS_HAPD
int apply_wps(struct wifi_config *config)
{
	if(config->wps_state)
		return 1;
	else
		return 0;
}
#endif

#ifdef RTK_HAPD_MULTI_AP
int apply_multi_ap(struct wifi_config *config)
{
	if(config->multi_ap)
		return 1;
	else
		return 0;
}
#endif

//======Basic

int hapd_get_driver(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->driver);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

int hapd_get_interface(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->interface);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

int hapd_get_ctrl_interface(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->ctrl_interface);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

int hapd_get_bridge(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->bridge);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

int hapd_get_ssid(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
#ifdef UNIVERSAL_REPEATER
	if(config->is_repeater)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=\"%s\"\n", name, config->ssid);
	else
#endif
	{
#ifdef CONFIG_WLAN_CLIENT_MODE
		if(config->wlan_mode == AP_MODE)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->ssid);
		else if(config->wlan_mode == CLIENT_MODE)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=\"%s\"\n", name, config->ssid);
#else
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->ssid);
#endif
	}

	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

//======Security
int hapd_get_auth_algs(struct wifi_config *config, char *name, char *result)
{
	unsigned char auth_algs = WPA_AUTH_ALG_OPEN;
	int len = 0;

	if(config->authtype == AUTH_OPEN)
		auth_algs = WPA_AUTH_ALG_OPEN;
	else if(config->authtype == AUTH_SHARED && config->encmode != _NO_PRIVACY_ && !config->psk_enable)
		auth_algs = WPA_AUTH_ALG_SHARED;
	else if(config->authtype == AUTH_BOTH && config->encmode != _NO_PRIVACY_ && !config->psk_enable)
		auth_algs = WPA_AUTH_ALG_OPEN | WPA_AUTH_ALG_SHARED;
	else
		auth_algs = WPA_AUTH_ALG_OPEN;


	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, auth_algs);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_wpa(struct wifi_config *config, char *name, char *result)
{
	unsigned char wpa = 0;
	int len = 0;

	if(config->encmode==0x00)
		return HAPD_GET_OK;

	if(config->psk_enable & PSK_WPA)
		wpa |= WPA_PROTO_WPA;

	if((config->psk_enable & PSK_WPA2)||(config->psk_enable & PSK_WPA3))
		wpa |= WPA_PROTO_RSN;

	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, wpa);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_wpa_psk(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if((config->psk_enable & (PSK_WPA|PSK_WPA2)) && config->psk_format == KEY_HEX){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->passphrase);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
	else
		return HAPD_GET_FAIL;
}

int hapd_get_wpa_passphrase(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->psk_enable && config->psk_format == KEY_ASCII){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->passphrase);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
	else
		return HAPD_GET_FAIL;
}

int hapd_get_sae_password(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->psk_enable & PSK_WPA3){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->passphrase);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
	else
		return HAPD_GET_FAIL;
}

int hapd_get_wpa_key_mgmt(struct wifi_config *config, char *name, char *result)
{
	enum PMF_STATUS cur_pmf_stat;

#ifdef CONFIG_IEEE80211W
	cur_pmf_stat = config->dot11IEEE80211W;
#endif

	int len = 0;
	if(apply_wpa(config)){

		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=", name);

		if((config->psk_enable & (PSK_WPA|PSK_WPA2)) && (cur_pmf_stat!=REQUIRED)){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s%s", result, "WPA-PSK ");
		}
#ifdef FAST_BSS_TRANSITION
		if(config->ft_enable)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s%s", result, "FT-PSK ");
#endif

#ifdef CONFIG_IEEE80211W
		if((config->psk_enable & PSK_WPA2)  && (!(config->wpa_cipher && config->wpa2_cipher))

			&& (config->enableSHA256 || ((config->wlan_mode == CLIENT_MODE || config->is_repeater) && cur_pmf_stat!=NONE))
		)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s%s", result, "WPA-PSK-SHA256 ");
#endif

		if(config->psk_enable & PSK_WPA3)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s%s", result, "SAE ");

		if(apply_radius(config)==RADIUS_WPA)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s%s", result, "WPA-EAP ");

		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s\n", result);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;

		return HAPD_GET_OK;
	}
	else
	{
#if defined(CONFIG_WLAN_CLIENT_MODE) || defined(UNIVERSAL_REPEATER)
		if(config->wlan_mode == CLIENT_MODE || config->is_repeater)
		{
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=NONE\n", name);
			if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
				return HAPD_GET_FAIL;

			return HAPD_GET_OK;
		}
		else
		{
			return HAPD_GET_FAIL;
		}
#else
		return HAPD_GET_FAIL;
#endif
	}
}

int hapd_get_wpa_pairwise(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->encmode==0x00)
		return HAPD_GET_OK;

	if(config->wpa_cipher == CIPHER_TKIP){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "TKIP");
	}else if(config->wpa_cipher == CIPHER_AES){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "CCMP");
	}else if(config->wpa_cipher == CIPHER_MIXED){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "TKIP CCMP");
	}else{
		printf("!! Error,  [%s][%d] unknown wpa_cipher\n", __FUNCTION__, __LINE__);
		return HAPD_GET_FAIL;
	}

	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_rsn_pairwise(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->encmode==0x00)
		return HAPD_GET_OK;

	if(config->psk_enable & (PSK_WPA2|PSK_WPA3)){
		if(config->wpa2_cipher == CIPHER_TKIP){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "TKIP");
		}else if(config->wpa2_cipher == CIPHER_AES){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "CCMP");
		}else if(config->wpa2_cipher == CIPHER_MIXED){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "TKIP CCMP");
		}else{
			printf("!! Error,  [%s][%d] unknown wpa_cipher\n", __FUNCTION__, __LINE__);
			return HAPD_GET_FAIL;
		}
	}

	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;

}

#ifdef CONFIG_IEEE80211W
int hapd_get_ieee80211w(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wpa(config) && (!(config->wpa_cipher && config->wpa2_cipher))){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->dot11IEEE80211W);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
	}
	return HAPD_GET_OK;
}
#endif

int hapd_get_wpa_group_rekey(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wpa(config))
	{
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->gk_rekey);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
	}
	return HAPD_GET_OK;
}

extern int wlan_index;
int hapd_get_wep_key0(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	unsigned int wep_key_type = config->wepKeyType;

	if(config->encmode == _WEP_40_PRIVACY_ || config->encmode == _WEP_104_PRIVACY_)
	{
		unsigned idx, key_len = 0;

		if(config->encmode == _WEP_40_PRIVACY_)
			key_len = 5;
		else
			key_len = 13;

		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=", name);

		if(wep_key_type == 0 && strlen(config->wepkey0) == key_len)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s" "\"" "%s" "\"", result, config->wepkey0);
		else
		{
			if(wep_key_type == 0)
				memset(config->wepkey0, 0, sizeof(config->wepkey0));
			for(idx=0; idx<key_len; idx++)
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s%02x", result, config->wepkey0[idx]);
		}

		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s\n", result);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}

	return HAPD_GET_FAIL;
}
int hapd_get_wep_key1(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	unsigned int wep_key_type = 0;

	wlan_get_mib(MIB_WLAN_WEP_KEY_TYPE, wlan_idx, 0, (void *)&wep_key_type);
	if(config->encmode == _WEP_40_PRIVACY_ || config->encmode == _WEP_104_PRIVACY_)
	{
		unsigned idx, key_len = 0;

		if(config->encmode == _WEP_40_PRIVACY_)
			key_len = 5;
		else
			key_len = 13;

		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=", name);

		if(wep_key_type == 0 && strlen(config->wepkey1) == key_len)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s" "\"" "%s" "\"", result, config->wepkey1);
		else
		{
			if(wep_key_type == 0)
				memset(config->wepkey1, 0, sizeof(config->wepkey1));
			for(idx=0; idx<key_len; idx++){
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s%02x", result, config->wepkey1[idx]);
			}
		}

		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s\n", result);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}

	return HAPD_GET_FAIL;
}
int hapd_get_wep_key2(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	unsigned int wep_key_type = 0;

	wlan_get_mib(MIB_WLAN_WEP_KEY_TYPE, wlan_idx, 0, (void *)&wep_key_type);
	if(config->encmode == _WEP_40_PRIVACY_ || config->encmode == _WEP_104_PRIVACY_)
	{
		unsigned idx, key_len = 0;

		if(config->encmode == _WEP_40_PRIVACY_)
			key_len = 5;
		else
			key_len = 13;

		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=", name);

		if(wep_key_type == 0 && strlen(config->wepkey2) == key_len)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s" "\"" "%s" "\"", result, config->wepkey2);
		else
		{
			if(wep_key_type == 0)
				memset(config->wepkey2, 0, sizeof(config->wepkey2));
			for(idx=0; idx<key_len; idx++)
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s%02x", result, config->wepkey2[idx]);
		}

		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s\n", result);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}

	return HAPD_GET_FAIL;
}
int hapd_get_wep_key3(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	unsigned int wep_key_type = 0;

	wlan_get_mib(MIB_WLAN_WEP_KEY_TYPE, wlan_idx, 0, (void *)&wep_key_type);
	if(config->encmode == _WEP_40_PRIVACY_ || config->encmode == _WEP_104_PRIVACY_)
	{
		unsigned idx, key_len = 0;

		if(config->encmode == _WEP_40_PRIVACY_)
			key_len = 5;
		else
			key_len = 13;

		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=", name);

		if(wep_key_type == 0 && strlen(config->wepkey3) == key_len)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s" "\"" "%s" "\"", result, config->wepkey3);
		else
		{
			if(wep_key_type == 0)
				memset(config->wepkey3, 0, sizeof(config->wepkey3));
			for(idx=0; idx<key_len; idx++)
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s%02x", result, config->wepkey3[idx]);
		}

		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s\n", result);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}

	return HAPD_GET_FAIL;
}

int hapd_get_wep_default_key(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->encmode == _WEP_40_PRIVACY_ || config->encmode == _WEP_104_PRIVACY_)
	{
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, 0);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}

	return HAPD_GET_FAIL;
}
int hapd_get_wep_key_len_unicast(struct wifi_config *config, char *name, char *result)
{
	int len = 0;

	if(config->encmode == _WEP_40_PRIVACY_)
	{
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=5\n", name);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
	else if(config->encmode == _WEP_104_PRIVACY_){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=13\n", name);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}

	return HAPD_GET_FAIL;
}

int hapd_get_wep_tx_keyidx(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->encmode == _WEP_40_PRIVACY_ || config->encmode == _WEP_104_PRIVACY_)
	{
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=0\n", name);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}

	return HAPD_GET_FAIL;
}

int hapd_get_sae_groups(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->psk_enable & PSK_WPA3){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=19 20 21\n", name);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}

	return HAPD_GET_FAIL;
}

int hapd_get_sae_anti_clogging_threshold(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->psk_enable & PSK_WPA3){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=5\n", name);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}

	return HAPD_GET_FAIL;
}


//======RADIUS

int hapd_get_own_ip_addr(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, inet_ntoa(*((struct in_addr *)config->lan_ip)));
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}
int hapd_get_nas_identifier(struct wifi_config *config, char *name, char *result)
{
	int len = 0;

#ifdef FAST_BSS_TRANSITION
	memset(result, 0, sizeof(result));
	if(config->ft_enable) {
		if(strlen(config->ft_r0kh_id) != 0)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->ft_r0kh_id);
		else
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "default.r0kh-id.fqdn");
	}
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
#else
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "test_coding.com");
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
#endif
}

int hapd_get_ieee8021x(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->enable_1x);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_auth_server_addr(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, inet_ntoa(*((struct in_addr *)config->rsIpAddr)));
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}
int hapd_get_auth_server_port(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->rsPort);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_auth_server_shared_secret(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->rsPassword);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}
int hapd_get_acct_server_addr(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, inet_ntoa(*((struct in_addr *)config->rsIpAddr)));
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}
int hapd_get_acct_server_port(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->rsPort);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}
int hapd_get_acct_server_shared_secret(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->rsPassword);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_radius_retry_primary_interval(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->radius_retry_primary_interval);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_auth_server_addr_2(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "auth_server_addr=%s\n", inet_ntoa(*((struct in_addr *)config->rs2IpAddr)));
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}
int hapd_get_auth_server_port_2(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "auth_server_port=%d\n", config->rs2Port);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_auth_server_shared_secret_2(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "auth_server_shared_secret=%s\n", config->rs2Password);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}
int hapd_get_acct_server_addr_2(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "acct_server_addr=%s\n", inet_ntoa(*((struct in_addr *)config->rs2IpAddr)));
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}
int hapd_get_acct_server_port_2(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "acct_server_port=%d\n", config->rs2Port);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}
int hapd_get_acct_server_shared_secret_2(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "acct_server_shared_secret=%s\n", config->rs2Password);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_radius_acct_interim_interval(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_radius(config)==NO_RADIUS)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->radius_acct_interim_interval);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}


//======Opeation Mode
int hapd_get_hw_mode(struct wifi_config *config, char *name, char *result)
{
	unsigned char hw_mode[20];
	int len = 0;

	switch(config->phy_band_select){
		case PHYBAND_2G:
			if(config->wlan_band & WIRELESS_11A)
				strcpy(hw_mode, "a");
			else
				strcpy(hw_mode, "g");
		break;
		case PHYBAND_5G:
			if(config->wlan_band & WIRELESS_11A ||
				config->wlan_band & WIRELESS_11N ||
				config->wlan_band & WIRELESS_11AC ||
				config->wlan_band & WIRELESS_11AX)
				strcpy(hw_mode, "a");
			else
				strcpy(hw_mode, "g");
		break;
		default:
			printf("error!: phy_band_select did not recognize \n");
			return HAPD_GET_FAIL;

	}

	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, hw_mode);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_obss_interval(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->coexist)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, 180);
	else
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->coexist);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

int hapd_get_ieee80211n(struct wifi_config *config, char *name, char *result)
{
	unsigned char ieee80211n;
	int len = 0;

	if(config->encmode == _WEP_40_PRIVACY_ || config->encmode == _WEP_104_PRIVACY_)
		ieee80211n = 0;
	else if(config->wlan_band & WIRELESS_11N || config->wlan_band & WIRELESS_11AC || config->wlan_band & WIRELESS_11AX)
		ieee80211n = 1;
	else
		ieee80211n = 0;

	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, ieee80211n);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}


int hapd_get_require_ht(struct wifi_config *config, char *name, char *result)
{
	unsigned char require_ht;
	int len = 0;

	if(config->wlan_band & WIRELESS_11N)
		require_ht = 1;
	else
		require_ht = 0;

	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, require_ht);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_ieee80211ac(struct wifi_config *config, char *name, char *result)
{
	unsigned char ieee80211ac;
	int len = 0;

	if(config->encmode == _WEP_40_PRIVACY_ || config->encmode == _WEP_104_PRIVACY_)
		ieee80211ac = 0;
	else if(config->wlan_band & WIRELESS_11AC || ((config->wlan_band & WIRELESS_11AX) && (config->phy_band_select==PHYBAND_5G)))
		ieee80211ac = 1;
	else
		ieee80211ac = 0;

	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, ieee80211ac);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_ieee80211ax(struct wifi_config *config, char *name, char *result)
{
	unsigned char ieee80211ax;
	int len = 0;

	if(config->encmode == _WEP_40_PRIVACY_ || config->encmode == _WEP_104_PRIVACY_)
		ieee80211ax = 0;
	else if(config->wlan_band & WIRELESS_11AX)
		ieee80211ax = 1;
	else
		ieee80211ax = 0;

	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, ieee80211ax);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_ieee80211d(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 1;
	sprintf(result, "%s=%d\n", name, config_setting);

	return HAPD_GET_OK;
}

int hapd_get_require_vht(struct wifi_config *config, char *name, char *result)
{
	unsigned char require_vht;
	int len = 0;

	if(config->wlan_band & WIRELESS_11AC)
		require_vht = 1;
	else
		require_vht = 0;

	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, require_vht);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_ht_capab(struct wifi_config *config, char *name, char *result)
{
	char ht_capab[50];
	int len = 0;

	if((config->wlan_band & WIRELESS_11N) == 0 && (config->wlan_band & WIRELESS_11AC) == 0)
		return HAPD_GET_FAIL;

	if(config->channel_bandwidth == CHANNEL_WIDTH_20){
		if(config->shortGI)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=[SHORT-GI-20]", name);
		else
			return HAPD_GET_FAIL;


		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s\n", result);
		return HAPD_GET_OK;
	}

	if (config->auto_chan_sel)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=[HT40+]", name);
	else if(config->channel >= 36){
		if(config->channel <= 144){
			if(config->channel%8)
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=[HT40+]", name);
			else
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=[HT40-]", name);
		}else{
			if((config->channel-1)%8)
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=[HT40+]", name);
			else
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=[HT40-]", name);
		}
	}
	else if(config->control_band == HT_CONTROL_UPPER)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=[HT40-]", name);
	else
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=[HT40+]", name);

	if(config->shortGI)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s[SHORT-GI-20][SHORT-GI-40]", result);
	/*
	if(config->stbc){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s[TX-STBC]", result);
	}
	if(config->ldpc){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s[LDPC]", result);
	}
	*/
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s\n", result);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_vht_capab(struct wifi_config *config, char *name, char *result)
{
	int len = 0,length=0;

	if((config->wlan_band & WIRELESS_11AC) == 0)
		return HAPD_GET_FAIL;
	if(config->shortGI && config->channel_bandwidth == CHANNEL_WIDTH_80){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=[SHORT-GI-80]", name);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN){
			printf("[%s %d]warning, snprintf error!\n",__FUNCTION__,__LINE__);
			return HAPD_GET_FAIL;
		}
	}

	length = HAPD_CONF_RESULT_MAX_LEN-strlen(result);
	len = snprintf(result+strlen(result), length, "%s", "\n");
	if(len < 0 || len >= length){
		printf("[%s %d]warning, snprintf error!\n",__FUNCTION__,__LINE__);
		return HAPD_GET_FAIL;
	}

	return HAPD_GET_OK;
}


//======Operation Channel
int hapd_get_vht_oper_chwidth(struct wifi_config *config, char *name, char *result)
{
	unsigned char vht_oper_chwidth = VHT_CHANWIDTH_80MHZ;
	int len = 0;

	if((config->wlan_band & WIRELESS_11AC) == 0)
		return HAPD_GET_FAIL;

	if(config->channel_bandwidth == CHANNEL_WIDTH_80)
		vht_oper_chwidth = VHT_CHANWIDTH_80MHZ;
	else if(config->channel_bandwidth == CHANNEL_WIDTH_160)
		vht_oper_chwidth = VHT_CHANWIDTH_160MHZ;
	else
		vht_oper_chwidth = VHT_CHANWIDTH_USE_HT;

	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vht_oper_chwidth);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}


int hapd_get_channel(struct wifi_config *config, char *name, char *result)
{
	int len, chan = 0;
	chan = config->auto_chan_sel ? 0 : config->channel;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, chan);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}


int hapd_get_channel_list(struct wifi_config *config, char *name, char *result)
{
	int len = 0;

	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->channel_list);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}


int hapd_get_vht_oper_centr_freq_seg0_idx(struct wifi_config *config, char *name, char *result)
{
	unsigned char centr_freq_seg0_idx = 0;
	int len = 0;

	if((config->wlan_band & WIRELESS_11AC) == 0)
		return HAPD_GET_FAIL;

	if(config->channel_bandwidth == CHANNEL_WIDTH_80){
		if(config->channel <= 48)
			centr_freq_seg0_idx = 42;
		else if(config->channel <= 64)
			centr_freq_seg0_idx = 58;
		else if(config->channel <= 112)
			centr_freq_seg0_idx = 106;
		else if(config->channel <= 128)
			centr_freq_seg0_idx = 122;
		else if(config->channel <= 144)
			centr_freq_seg0_idx = 138;
		else if(config->channel <= 161)
			centr_freq_seg0_idx = 155;
		else if(config->channel <= 177)
			centr_freq_seg0_idx = 171;
	}
	else if(config->channel_bandwidth == CHANNEL_WIDTH_40){
		if(config->channel == 0)
			centr_freq_seg0_idx=0;
		else if(config->channel <= 144){
			if(config->channel%8)
				centr_freq_seg0_idx=config->channel+2;
			else
				centr_freq_seg0_idx=config->channel-2;
		}else{
			if((config->channel-1)%8)
				centr_freq_seg0_idx=config->channel+2;
			else
				centr_freq_seg0_idx=config->channel-2;
		}
	}
	else if(config->channel_bandwidth == CHANNEL_WIDTH_20){
		centr_freq_seg0_idx = config->channel;
	}

	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, centr_freq_seg0_idx);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}


//======HW Setting, Features
int hapd_get_bssid(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->bssid);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}


int hapd_get_country_code(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(strlen(config->country)==0 || !strcmp(config->country, ""))
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "US"); //avoid hostapd dead loop
	else
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->country);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}


//======Advanced
int hapd_get_fragm_threshold(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->fragthres <= 2346)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->fragthres);
	else
		return HAPD_GET_FAIL;
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;

}

int hapd_get_rts_threshold(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->rtsthres);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

int hapd_get_beacon_int(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->bcnint);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

int hapd_get_dtim_period(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->dtimperiod);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

int hapd_get_preamble(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->preamble);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

int hapd_get_ignore_broadcast_ssid(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->ignore_broadcast_ssid);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

int hapd_get_wmm_enabled(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->qos);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
	return HAPD_GET_OK;
}

int hapd_get_multicast_to_unicast(struct wifi_config *config, char *name, char *result)
{

#if 0
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->multicast_to_unicast);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
#endif

	return HAPD_GET_OK;
}

int hapd_get_ap_isolate(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->ap_isolate);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_ap_max_inactivity(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->ap_max_inactivity);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

#ifdef WLAN_HS2_CONFIG
int hapd_get_hs20(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->hs20);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}
#endif

int hapd_get_tdls_prohibit(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->tdls_prohibit);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_tdls_prohibit_chan_switch(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->tdls_prohibit_chan_switch);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

#if 1//def CONFIG_GUEST_ACCESS_SUPPORT
int hapd_get_access_network_type(struct wifi_config *config, char *name, char *result)
{
#if 0
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->guest_access);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;
#endif

	return HAPD_GET_OK;
}
#endif

#define FILE_PATH_MAX_LEN 64
/*
web:	0 acl disable, 1 white, 2 black;
hostapd:	0 = accept unless in deny list, 1 = deny unless in accept list.
*/
int hapd_get_macaddr_acl(struct wifi_config *config, char *name, char *result)
{
	int i = 0;
	FILE *fp = NULL;
	char MAC_LIST_FILE_PERIF[FILE_PATH_MAX_LEN];

	if(config->macaddr_acl != 0 && config->macaddr_acl != 1 && config->macaddr_acl != 2){
		printf("macaddr_acl[%d] not support!!!\n", config->macaddr_acl);
		return HAPD_GET_OK;
	}

	if(config->macaddr_acl == 0){
		return HAPD_GET_OK;
	}else if(config->macaddr_acl == 1){
		snprintf(MAC_LIST_FILE_PERIF, FILE_PATH_MAX_LEN, "/var/run/hostapd_accept_%s", config->interface);
		if(!(fp = fopen(MAC_LIST_FILE_PERIF, "w+"))){
			fprintf(stderr, "fopen ERROR! %s\n", strerror(errno));
			return HAPD_GET_FAIL;
		}
		snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "accept_mac_file=%s\n", MAC_LIST_FILE_PERIF);
		strncat(result, "macaddr_acl=1\n", HAPD_CONF_RESULT_MAX_LEN);
	}else if(config->macaddr_acl == 2){
		snprintf(MAC_LIST_FILE_PERIF, FILE_PATH_MAX_LEN, "/var/run/hostapd_deny_%s", config->interface);
		if(!(fp = fopen(MAC_LIST_FILE_PERIF, "w+"))){
			fprintf(stderr, "fopen ERROR! %s\n", strerror(errno));
			return HAPD_GET_FAIL;
		}
		snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "deny_mac_file=%s\n", MAC_LIST_FILE_PERIF);
		strncat(result, "macaddr_acl=0\n", HAPD_CONF_RESULT_MAX_LEN);
	}

	for(i = 0; i < MAX_ACL_NUM; i++){
		if(config->mac_acl_list[i].used)
			fprintf(fp, "%02x:%02x:%02x:%02x:%02x:%02x\n", config->mac_acl_list[i]._macaddr[0],
				config->mac_acl_list[i]._macaddr[1], config->mac_acl_list[i]._macaddr[2], config->mac_acl_list[i]._macaddr[3],
				config->mac_acl_list[i]._macaddr[4], config->mac_acl_list[i]._macaddr[5]);
	}

	fclose(fp);
	return HAPD_GET_OK;
}

#ifdef WLAN_WPS_HAPD
int hapd_get_wps_state(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->wps_state){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->wps_state);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
	else
		return HAPD_GET_FAIL;

}

int hapd_get_wps_uuid(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->uuid);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_device_name(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->device_name);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_model_name(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->device_name);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_model_number(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->model_number);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_serial_number(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->serial_number);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_device_type(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->device_type);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_config_methods(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		if(config->config_method == 1)
		{
#ifdef UNIVERSAL_REPEATER
			if(config->is_repeater)
			{
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=\"%s\"\n", name, "virtual_display keypad");
			}
			else
#endif
			{
#ifdef CONFIG_WLAN_CLIENT_MODE
				if(config->wlan_mode == AP_MODE)
					len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "virtual_display keypad");
				else if(config->wlan_mode == CLIENT_MODE)
					len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=\"%s\"\n", name, "virtual_display keypad");
#else
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "virtual_display keypad");
#endif
			}

			if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
				return HAPD_GET_FAIL;
		}
		else if(config->config_method == 2)
		{
#ifdef UNIVERSAL_REPEATER
			if(config->is_repeater)
			{
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=\"%s\"\n", name, "physical_push_button virtual_push_button");
			}
			else
#endif
			{
#ifdef CONFIG_WLAN_CLIENT_MODE
				if(config->wlan_mode == AP_MODE)
					len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "physical_push_button virtual_push_button");
				else if(config->wlan_mode == CLIENT_MODE)
					len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=\"%s\"\n", name, "physical_push_button virtual_push_button");
#else
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "physical_push_button virtual_push_button");
#endif
			}

			if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
				return HAPD_GET_FAIL;
		}
		else if(config->config_method == 3)
		{
#ifdef UNIVERSAL_REPEATER
			if(config->is_repeater)
			{
				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=\"%s\"\n", name, "virtual_display keypad physical_push_button virtual_push_button");
			}
			else
#endif
			{
#ifdef CONFIG_WLAN_CLIENT_MODE
				if(config->wlan_mode == AP_MODE)
					len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "virtual_display keypad physical_push_button virtual_push_button");
				else if(config->wlan_mode == CLIENT_MODE)
					len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=\"%s\"\n", name, "virtual_display keypad physical_push_button virtual_push_button");
#else

				len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "virtual_display keypad physical_push_button virtual_push_button");
#endif
			}

			if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
				return HAPD_GET_FAIL;
		}
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_pbc_in_m1(struct wifi_config *config, char *name, char *result)
{
	int len=0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=1\n", name);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_ap_pin(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->ap_pin);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_upnp_iface(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->upnp_iface);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_friendly_name(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->friendly_name);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_manufacturer_url(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->manufacturer_url);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_model_url(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->model_url);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_eap_server(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->eap_server);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_wps_manufacturer(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_wps(config)==0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->manufacturer);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

#endif

#ifdef RTK_HAPD_MULTI_AP
int hapd_get_multi_ap(struct wifi_config *config, char *name, char *result)
{
	int len = 0;

	if(config->multi_ap){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->multi_ap);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
	else
		return HAPD_GET_FAIL;

}

int hapd_get_multi_ap_backhaul_ssid(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_multi_ap(config)==0 || config->multi_ap_backhaul_ssid[0] == 0)
		return HAPD_GET_FAIL;
	else {
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=\"%s\"\n", name, config->multi_ap_backhaul_ssid);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
}

int hapd_get_multi_ap_backhaul_wpa_psk(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_multi_ap(config)==0 || config->multi_ap_backhaul_passphrase[0] == 0)
		return HAPD_GET_FAIL;
	else {
		if(config->multi_ap_backhaul_psk_format == KEY_HEX){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->multi_ap_backhaul_passphrase);
			if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
				return HAPD_GET_FAIL;
			return HAPD_GET_OK;
		}
		else
			return HAPD_GET_FAIL;
	}
}
int hapd_get_multi_ap_backhaul_wpa_passphrase(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(apply_multi_ap(config)==0 || config->multi_ap_backhaul_passphrase[0] == 0)
		return HAPD_GET_FAIL;
	else {
		if(config->multi_ap_backhaul_psk_format == KEY_ASCII){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->multi_ap_backhaul_passphrase);
			if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
				return HAPD_GET_FAIL;
			return HAPD_GET_OK;
		}
		else
			return HAPD_GET_FAIL;
	}
}
#endif

#if defined(WLAN_LOGO_TEST_FOR_SD9) || defined(CONFIG_MBO_SUPPORT)
/*=======================MBO==============================*/
int hapd_get_mbo(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 1;
	if(apply_wpa(config) && config->dot11IEEE80211W==1 && (!(config->wpa_cipher && config->wpa2_cipher))
#ifdef WLAN_WPA3
		&& (config->psk_enable != PSK_WPA3)
#endif
	){
		sprintf(result, "%s=%d\n", name, config_setting);
	}
	return HAPD_GET_OK;
}

int hapd_get_oce(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 4;
	if(apply_wpa(config) && config->dot11IEEE80211W==1 && (!(config->wpa_cipher && config->wpa2_cipher))
#ifdef WLAN_WPA3
		&& (config->psk_enable != PSK_WPA3)
#endif
	)
		sprintf(result, "%s=%d\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_interworking(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 1;
	if(apply_wpa(config) && config->dot11IEEE80211W==1 && (!(config->wpa_cipher && config->wpa2_cipher))
#ifdef WLAN_WPA3
		&& (config->psk_enable != PSK_WPA3)
#endif
	)
		sprintf(result, "%s=%d\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_country3(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 4;
	if(apply_wpa(config) && config->dot11IEEE80211W==1 && (!(config->wpa_cipher && config->wpa2_cipher))
#ifdef WLAN_WPA3
		&& (config->psk_enable != PSK_WPA3)
#endif
	)
		sprintf(result, "%s=0x%02x\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_gas_address3(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 1;
	if(apply_wpa(config) && config->dot11IEEE80211W==1 && (!(config->wpa_cipher && config->wpa2_cipher))
#ifdef WLAN_WPA3
		&& (config->psk_enable != PSK_WPA3)
#endif
	)
		sprintf(result, "%s=%d\n", name, config_setting);
	return HAPD_GET_OK;
}
#endif
#ifdef WLAN_LOGO_TEST_FOR_SD9
/*=======================WPS==============================*/
#if 0
int hapd_get_wps_state(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 2;
	sprintf(result, "%s=%d\n", name, config_setting);
	return HAPD_GET_OK;
}
#endif
int hapd_get_config_methods(struct wifi_config *config, char *name, char *result)
{
	char config_setting[] = "label display push_button keypad ethernet virtual_display";
	sprintf(result, "%s=%s\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_device_name(struct wifi_config *config, char *name, char *result)
{
	char config_setting[] = "Realtek AP";
	sprintf(result, "%s=%s\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_manufacturer(struct wifi_config *config, char *name, char *result)
{
	char config_setting[] = "Realtek";
	sprintf(result, "%s=%s\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_model_name(struct wifi_config *config, char *name, char *result)
{
	char config_setting[] = "WAP";
	sprintf(result, "%s=%s\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_model_number(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 0;
	sprintf(result, "%s=%03d\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_serial_number(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 12345;
	sprintf(result, "%s=%d\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_device_type(struct wifi_config *config, char *name, char *result)
{
	char config_setting[] = "6-0050F204-1";
	sprintf(result, "%s=%s\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_os_version(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 0x01020300;
	sprintf(result, "%s=%08x\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_eap_server(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 1;
	sprintf(result, "%s=%d\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_uuid(struct wifi_config *config, char *name, char *result)
{
	char config_setting[] = "87654321-9abc-def0-1234-56789abc0000";
	sprintf(result, "%s=%s\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_upnp_iface(struct wifi_config *config, char *name, char *result)
{
	char config_setting[] = "br0";
	sprintf(result, "%s=%s\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_friendly_name(struct wifi_config *config, char *name, char *result)
{
	char config_setting[] = "Realtek WPS";
	sprintf(result, "%s=%s\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_ap_setup_locked(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 0;
	sprintf(result, "%s=%d\n", name, config_setting);
	return HAPD_GET_OK;
}

int hapd_get_ap_pin(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 12345670;
	sprintf(result, "%s=%d\n", name, config_setting);
	return HAPD_GET_OK;
}
#endif

int hapd_get_supported_rates(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->supported_rates);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}
int hapd_get_basic_rates(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->basic_rates);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

#ifdef DOT11K
int hapd_get_rrm_neighbor_report(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 1, len = 0;

	if(config->dot11k_enable) {
		len = sprintf(result, "%s=%d\n", name, config_setting);
	}
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_rrm_beacon_report(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 1, len = 0;

	if(config->dot11k_enable) {
		sprintf(result, "%s=%d\n", name, config_setting);
	}
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}
#endif

#ifdef CONFIG_IEEE80211V
int hapd_get_bss_transition(struct wifi_config *config, char *name, char *result)
{
	unsigned int config_setting = 1, len=0;
	if(config->dot11v_enable) {
		sprintf(result, "%s=%d\n", name, config_setting);
	}
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}
#endif

#ifdef FAST_BSS_TRANSITION
int hapd_get_mobility_domain(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->ft_enable) {
		if(strlen(config->ft_mdid) != 0)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->ft_mdid);
		else
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=a1b2\n", name);
	}
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}
int hapd_get_ft_over_ds(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->ft_enable)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->ft_over_ds);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}
/*
int hapd_get_ft_r0_key_lifetime(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->ft_enable)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->ft_r0key_to);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}
int hapd_get_reassociation_deadline(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->ft_enable)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->ft_reasoc_to);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}
*/
int hapd_get_pmk_r1_push(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->ft_enable)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->ft_push);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}
int hapd_get_r1_key_holder(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->ft_enable)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->r1_key_holder);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_r0kh(struct wifi_config *config, char *name, char *result)
{
	int len=0, entryNum=0, i=0;
	wlan_get_mib(MIB_WLAN_FTKH_NUM, wlan_idx, 0, (void*)&entryNum);
	char dot_mac[32] = {0};
	char hex_key[33] = {0};

	if(entryNum == 0)
		return HAPD_GET_OK;
	for(i=0; i<entryNum; i++) {
		snprintf(dot_mac, sizeof(dot_mac), "%02x:%02x:%02x:%02x:%02x:%02x", config->wlftkh_list[i].addr[0], config->wlftkh_list[i].addr[1],
			config->wlftkh_list[i].addr[2], config->wlftkh_list[i].addr[3], config->wlftkh_list[i].addr[4], config->wlftkh_list[i].addr[5]);
		if(i == 0)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s %s %s\n", name, dot_mac,config->wlftkh_list[i].r0kh_id, config->wlftkh_list[i].key);
		else
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s%s=%s %s %s\n", result, name, dot_mac, config->wlftkh_list[i].r0kh_id, config->wlftkh_list[i].key);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
	}
	return HAPD_GET_OK;
}
int hapd_get_r1kh(struct wifi_config *config, char *name, char *result)
{
	int len=0, entryNum=0, i=0;
	wlan_get_mib(MIB_WLAN_FTKH_NUM, wlan_idx, 0, (void*)&entryNum);
	char dot_mac[32] = {0};

	if(entryNum == 0)
		return HAPD_GET_OK;
	for(i=0; i<entryNum; i++) {
		snprintf(dot_mac, sizeof(dot_mac), "%02x:%02x:%02x:%02x:%02x:%02x", config->wlftkh_list[i].addr[0], config->wlftkh_list[i].addr[1],
			config->wlftkh_list[i].addr[2], config->wlftkh_list[i].addr[3], config->wlftkh_list[i].addr[4], config->wlftkh_list[i].addr[5]);
		if(i == 0)
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s %s %s\n", name, dot_mac, dot_mac, config->wlftkh_list[i].key);
		else
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s%s=%s %s %s\n", result, name, dot_mac, dot_mac, config->wlftkh_list[i].key);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
	}
	return HAPD_GET_OK;
}
#endif

//station
int hapd_get_max_num_sta(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if (config->max_num_sta > 0)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->max_num_sta);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

#if defined(CONFIG_WLAN_CLIENT_MODE) || defined(UNIVERSAL_REPEATER)
int hapd_get_wpas_network_start(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s\n", name);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_wpas_update_config(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->update_config);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_wpas_scan_ssid(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->scan_ssid);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_wpas_pbss(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->pbss);
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

//======Security
int hapd_get_wpas_auth_algs(struct wifi_config *config, char *name, char *result)
{
	unsigned char auth_algs = WPA_AUTH_ALG_OPEN;
	int len = 0;
	char alg[16]= {0};
	if(config->psk_enable & PSK_WPA3)
		return HAPD_GET_OK;

	if(config->authtype == _AUTH_ALGM_OPEN_)
		auth_algs = WPA_AUTH_ALG_OPEN;
	else if(config->authtype == _AUTH_ALGM_SHARED_)
		auth_algs = WPA_AUTH_ALG_SHARED;
	else
		auth_algs = WPA_AUTH_ALG_OPEN | WPA_AUTH_ALG_SHARED;

	if(auth_algs & WPA_AUTH_ALG_OPEN)
		strcat(alg, "OPEN ");
	if(auth_algs & WPA_AUTH_ALG_SHARED)
		strcat(alg, "SHARED ");

	len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, alg);

	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

int hapd_get_wpas_pairwise(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->encmode==0x00)
		return HAPD_GET_OK;

	if(config->psk_enable == PSK_WPA)
	{
		if(config->wpa_cipher == CIPHER_TKIP){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "TKIP");
		}else if(config->wpa_cipher == CIPHER_AES){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "CCMP");
		}else if(config->wpa_cipher == CIPHER_MIXED){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "TKIP CCMP");
		}
	}

	if(config->psk_enable & (PSK_WPA2 | PSK_WPA3)){
		if(config->wpa2_cipher == CIPHER_TKIP){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "TKIP");
		}else if(config->wpa2_cipher == CIPHER_AES){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "CCMP");
		}else if(config->wpa2_cipher == CIPHER_MIXED){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "TKIP CCMP");
		}else{
			printf("!! Error,  [%s][%d] unknown wpa_cipher\n", __FUNCTION__, __LINE__);
			return HAPD_GET_FAIL;
		}
	}

	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;

}

int hapd_get_wpas_psk(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->psk_enable){
		if(config->psk_format == KEY_HEX){
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, config->passphrase);
		}else{
			len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=\"%s\"\n", name, config->passphrase);
		}
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
	else
		return HAPD_GET_FAIL;
}

int hapd_get_wpas_sae_password(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->psk_enable & PSK_WPA3){
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=\"%s\"\n", name, config->passphrase);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
		return HAPD_GET_OK;
	}
	else
		return HAPD_GET_FAIL;
}

int hapd_get_wpas_proto(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->encmode == _NO_PRIVACY_ || config->encmode == _WEP_40_PRIVACY_ || config->encmode == _WEP_104_PRIVACY_)
		return HAPD_GET_OK;
	else if(config->wpa_cipher && (config->wpa2_cipher==0))
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "WPA");
	else if(config->wpa_cipher && config->wpa2_cipher)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "WPA RSN");
	else if((config->wpa_cipher==0) && config->wpa2_cipher)
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "RSN");
	else
		return HAPD_GET_FAIL;
	if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}

#ifdef RTK_HAPD_MULTI_AP
int hapd_get_multi_ap_backhaul_sta(struct wifi_config *config, char *name, char *result)
{
	int len = 0;
	if(config->multi_ap_backhaul_sta)
	{
		len = snprintf(result, HAPD_CONF_RESULT_MAX_LEN, "%s=%d\n", name, config->multi_ap_backhaul_sta);
		if(len < 0 || len >= HAPD_CONF_RESULT_MAX_LEN)
			return HAPD_GET_FAIL;
	}
	else
		return HAPD_GET_FAIL;

	return HAPD_GET_OK;
}
#endif
#endif

struct hapd_obj	hapd_conf_table[] =
{
    {"driver",					hapd_get_driver},
    {"interface",				hapd_get_interface},
    {"ctrl_interface",			hapd_get_ctrl_interface},
    {"bridge",					hapd_get_bridge},
    {"ssid",					hapd_get_ssid},

//Security
    {"auth_algs",				hapd_get_auth_algs},
    {"wpa",						hapd_get_wpa},
	{"wpa_psk",					hapd_get_wpa_psk},
	{"wpa_passphrase",			hapd_get_wpa_passphrase},
	{"sae_password",			hapd_get_sae_password},
	{"wpa_key_mgmt",			hapd_get_wpa_key_mgmt},
	{"wpa_pairwise",			hapd_get_wpa_pairwise},
	{"rsn_pairwise",			hapd_get_rsn_pairwise},
#ifdef CONFIG_IEEE80211W
	{"ieee80211w",				hapd_get_ieee80211w},
#endif
	{"wpa_group_rekey",			hapd_get_wpa_group_rekey},

	{"wep_key0",				hapd_get_wep_key0},
    {"wep_key1",                hapd_get_wep_key1},
    {"wep_key2",                hapd_get_wep_key2},
    {"wep_key3",                hapd_get_wep_key3},
	{"wep_default_key",			hapd_get_wep_default_key},
	{"wep_key_len_unicast",		hapd_get_wep_key_len_unicast},
	{"sae_groups",				hapd_get_sae_groups},
	{"sae_anti_clogging_threshold",		hapd_get_sae_anti_clogging_threshold},

//RADIUS
	{"own_ip_addr",					hapd_get_own_ip_addr},
	{"nas_identifier",				hapd_get_nas_identifier},

	{"ieee8021x",					hapd_get_ieee8021x},
	{"auth_server_addr",			hapd_get_auth_server_addr},
	{"auth_server_port",			hapd_get_auth_server_port},
	{"auth_server_shared_secret",	hapd_get_auth_server_shared_secret},
	{"acct_server_addr",			hapd_get_acct_server_addr},
	{"acct_server_port",			hapd_get_acct_server_port},
	{"acct_server_shared_secret",	hapd_get_acct_server_shared_secret},
	{"radius_retry_primary_interval",	hapd_get_radius_retry_primary_interval},

	{"auth_server_addr2",			hapd_get_auth_server_addr_2},
	{"auth_server_port2",			hapd_get_auth_server_port_2},
	{"auth_server_shared_secret2",	hapd_get_auth_server_shared_secret_2},
	{"acct_server_addr2",			hapd_get_acct_server_addr_2},
	{"acct_server_port2",			hapd_get_acct_server_port_2},
	{"acct_server_shared_secret2",	hapd_get_acct_server_shared_secret_2},
	{"radius_acct_interim_interval",	hapd_get_radius_acct_interim_interval},
//
    {"hw_mode",					hapd_get_hw_mode},
    {"obss_interval",			hapd_get_obss_interval},
    {"ieee80211n",				hapd_get_ieee80211n},
//    {"require_ht",				hapd_get_require_ht},
    {"ieee80211ac",				hapd_get_ieee80211ac},
//    {"require_vht",				hapd_get_require_vht},
    {"ieee80211ax",				hapd_get_ieee80211ax},
	{"ieee80211d",				hapd_get_ieee80211d},


	{"ht_capab",				hapd_get_ht_capab},	//Short GI, STBC, LDPC
	{"vht_capab",				hapd_get_vht_capab},

    {"vht_oper_chwidth",		hapd_get_vht_oper_chwidth},
    {"vht_oper_centr_freq_seg0_idx",	hapd_get_vht_oper_centr_freq_seg0_idx},
    {"channel",					hapd_get_channel},

//ACS channel list
    {"chanlist",				hapd_get_channel_list},

	{"bssid",					hapd_get_bssid},
	{"country_code", 			hapd_get_country_code},

//Advanced
	{"fragm_threshold",			hapd_get_fragm_threshold},
	{"rts_threshold",			hapd_get_rts_threshold},
	{"beacon_int",				hapd_get_beacon_int},
	{"dtim_period",				hapd_get_dtim_period},
	{"preamble",				hapd_get_preamble},
	{"ignore_broadcast_ssid",	hapd_get_ignore_broadcast_ssid},
	{"wmm_enabled", 			hapd_get_wmm_enabled},
    {"multicast_to_unicast",	hapd_get_multicast_to_unicast},
    {"ap_isolate",				hapd_get_ap_isolate},
    {"ap_max_inactivity",		hapd_get_ap_max_inactivity},
#ifdef WLAN_HS2_CONFIG
    {"hs20",					hapd_get_hs20},
#endif
    {"tdls_prohibit",			hapd_get_tdls_prohibit},
    {"tdls_prohibit_chan_switch",			hapd_get_tdls_prohibit_chan_switch},
#if 1//def CONFIG_GUEST_ACCESS_SUPPORT
    {"access_network_type",		hapd_get_access_network_type},
#endif
//acl
	{"macaddr_acl",				hapd_get_macaddr_acl},
//WPS
#ifdef WLAN_WPS_HAPD
	{"wps_state",				hapd_get_wps_state},
	{"uuid",    				hapd_get_wps_uuid},
	{"device_name", 			hapd_get_wps_device_name},
	{"model_name",				hapd_get_wps_model_name},
	{"model_number",			hapd_get_wps_model_number},
	{"serial_number",			hapd_get_wps_serial_number},
	{"device_type", 			hapd_get_wps_device_type},
	{"config_methods",			hapd_get_wps_config_methods},
	{"pbc_in_m1",				hapd_get_wps_pbc_in_m1},
	{"ap_pin",  				hapd_get_wps_ap_pin},
	{"upnp_iface",				hapd_get_wps_upnp_iface},
	{"friendly_name",			hapd_get_wps_friendly_name},
	{"manufacturer_url",		hapd_get_wps_manufacturer_url},
	{"model_url",				hapd_get_wps_model_url},
	{"eap_server",				hapd_get_wps_eap_server},
#endif
#ifdef RTK_HAPD_MULTI_AP
	{"multi_ap",				hapd_get_multi_ap},
	{"multi_ap_backhaul_ssid",	hapd_get_multi_ap_backhaul_ssid},
	{"multi_ap_backhaul_wpa_psk", hapd_get_multi_ap_backhaul_wpa_psk},
	{"multi_ap_backhaul_wpa_passphrase", hapd_get_multi_ap_backhaul_wpa_passphrase},
#endif

#if	defined(WLAN_LOGO_TEST_FOR_SD9) || defined(CONFIG_MBO_SUPPORT)
	//MBO
	{"mbo",						hapd_get_mbo},
	{"oce",						hapd_get_oce},
	{"interworking",			hapd_get_interworking},
	{"country3",				hapd_get_country3},
	{"gas_address3",			hapd_get_gas_address3},
#endif
#ifdef WLAN_LOGO_TEST_FOR_SD9
	//WPS
	{"wps_state",				hapd_get_wps_state},
	{"config_methods",			hapd_get_config_methods},
	{"device_name",				hapd_get_device_name},
	{"manufacturer",			hapd_get_manufacturer},
	{"model_name",				hapd_get_model_name},
	{"model_number",			hapd_get_model_number},

	{"serial_number",			hapd_get_serial_number},
	{"device_type",				hapd_get_device_type},
	{"os_version",				hapd_get_os_version},
	{"eap_server",				hapd_get_eap_server},
	{"uuid",					hapd_get_uuid},
	{"upnp_iface",				hapd_get_upnp_iface},
	{"friendly_name",			hapd_get_friendly_name},
	{"ap_setup_locked",			hapd_get_ap_setup_locked},
	{"ap_pin",					hapd_get_ap_pin},
#endif
	//rates
	{"supported_rates",			hapd_get_supported_rates},
	{"basic_rates",				hapd_get_basic_rates},

#ifdef DOT11K
	//11k
	{"rrm_neighbor_report",		hapd_get_rrm_neighbor_report},
	{"rrm_beacon_report",		hapd_get_rrm_beacon_report},
#endif

#ifdef CONFIG_IEEE80211V
	//11v
	{"bss_transition",			hapd_get_bss_transition},
#endif

	//11r
#ifdef FAST_BSS_TRANSITION
	{"mobility_domain",			hapd_get_mobility_domain},
	{"ft_over_ds",				hapd_get_ft_over_ds},
	{"pmk_r1_push",				hapd_get_pmk_r1_push},
	{"r1_key_holder",			hapd_get_r1_key_holder},
	{"r0kh",					hapd_get_r0kh},
	{"r1kh",					hapd_get_r1kh},
#endif

	//station
	{"max_num_sta",				hapd_get_max_num_sta},
};

struct hapd_obj	vap_hapd_conf_table[] =
{
	{"bss",				hapd_get_interface},
	{"ctrl_interface",			hapd_get_ctrl_interface},
	{"ssid",			hapd_get_ssid},
	{"bridge",			hapd_get_bridge},

	// Security
	{"auth_algs",			hapd_get_auth_algs},
	{"wpa",				hapd_get_wpa},
	{"wpa_psk",			hapd_get_wpa_psk},
	{"wpa_passphrase",		hapd_get_wpa_passphrase},
	{"sae_password",		hapd_get_sae_password},
	{"wpa_key_mgmt",		hapd_get_wpa_key_mgmt},
	{"wpa_pairwise",		hapd_get_wpa_pairwise},
	{"rsn_pairwise",		hapd_get_rsn_pairwise},
#ifdef CONFIG_IEEE80211W
	{"ieee80211w",			hapd_get_ieee80211w},
#endif
	{"wpa_group_rekey",		hapd_get_wpa_group_rekey},

	{"wep_key0",			hapd_get_wep_key0},
	{"wep_key1",            hapd_get_wep_key1},
    {"wep_key2",            hapd_get_wep_key2},
    {"wep_key3",            hapd_get_wep_key3},
	{"wep_default_key",		hapd_get_wep_default_key},
	{"wep_key_len_unicast",		hapd_get_wep_key_len_unicast},
#if 0

	// RADIUS
	{"own_ip_addr",			hapd_get_own_ip_addr},
	{"nas_identifier",		hapd_get_nas_identifier},

	{"auth_server_addr",		hapd_get_auth_server_addr},
	{"auth_server_port",		hapd_get_auth_server_port},
	{"auth_server_shared_secret",	hapd_get_auth_server_shared_secret},
	{"acct_server_addr",		hapd_get_acct_server_addr},
	{"acct_server_port",		hapd_get_acct_server_port},
	{"acct_server_shared_secret",	hapd_get_acct_server_shared_secret},

	{"auth_server_addr",		hapd_get_auth_server_addr_2},
	{"auth_server_port",		hapd_get_auth_server_port_2},
	{"auth_server_shared_secret",	hapd_get_auth_server_shared_secret_2},
	{"acct_server_addr",		hapd_get_acct_server_addr_2},
	{"acct_server_port",		hapd_get_acct_server_port_2},
	{"acct_server_shared_secret",	hapd_get_acct_server_shared_secret_2},
#endif
	{"hw_mode",				hapd_get_hw_mode},
	//
	//{"ieee80211n",			hapd_get_ieee80211n},
	//{"require_ht",			hapd_get_require_ht},
	//{"ieee80211ac",			hapd_get_ieee80211ac},
	//{"require_vht",			hapd_get_require_vht},
#ifdef CONFIG_MBO_SUPPORT
	{"ieee80211d",				hapd_get_ieee80211d},
	{"country_code",			hapd_get_country_code},
#endif
#if 0
	{"ht_capab",			hapd_get_ht_capab},	//Short GI, STBC

	// Advanced
	{"fragm_threshold",		hapd_get_fragm_threshold},
	{"rts_threshold",		hapd_get_rts_threshold},
	{"beacon_int",			hapd_get_beacon_int},
#endif
	{"bssid",					hapd_get_bssid},
	{"dtim_period",				hapd_get_dtim_period},
	//{"ieee80211ax",				hapd_get_ieee80211ax},
	{"ignore_broadcast_ssid",	hapd_get_ignore_broadcast_ssid},
	{"wmm_enabled",				hapd_get_wmm_enabled},
	{"ap_isolate",				hapd_get_ap_isolate},
#if 1//def CONFIG_GUEST_ACCESS_SUPPORT
	{"access_network_type",		hapd_get_access_network_type},
#endif
//acl
	{"macaddr_acl",				hapd_get_macaddr_acl},

#ifdef CONFIG_MBO_SUPPORT
//MBO
	{"mbo",						hapd_get_mbo},
	{"oce",						hapd_get_oce},
	{"interworking",			hapd_get_interworking},
	{"country3",				hapd_get_country3},
	{"gas_address3",			hapd_get_gas_address3},
#endif

#ifdef DOT11K
	//11k
	{"rrm_neighbor_report",		hapd_get_rrm_neighbor_report},
	{"rrm_beacon_report",		hapd_get_rrm_beacon_report},
#endif

#ifdef CONFIG_IEEE80211V
	//11v
	{"bss_transition",			hapd_get_bss_transition},
#endif

//11r
#ifdef FAST_BSS_TRANSITION
	{"mobility_domain", 		hapd_get_mobility_domain},
	{"ft_over_ds",				hapd_get_ft_over_ds},
	{"pmk_r1_push", 			hapd_get_pmk_r1_push},
#endif

	//station
	{"max_num_sta",				hapd_get_max_num_sta},

#ifdef RTK_HAPD_MULTI_AP
	{"multi_ap",				hapd_get_multi_ap},
	{"multi_ap_backhaul_ssid",	hapd_get_multi_ap_backhaul_ssid},
	{"multi_ap_backhaul_wpa_psk", hapd_get_multi_ap_backhaul_wpa_psk},
	{"multi_ap_backhaul_wpa_passphrase", hapd_get_multi_ap_backhaul_wpa_passphrase},
#endif

};

#if defined(CONFIG_WLAN_CLIENT_MODE) || defined(UNIVERSAL_REPEATER)
struct hapd_obj	wpas_conf_table[] =
{
	{"ctrl_interface",			hapd_get_ctrl_interface},
	{"update_config",			hapd_get_wpas_update_config},
#ifdef WLAN_WPS_HAPD
	{"uuid",					hapd_get_wps_uuid},
	{"device_name", 			hapd_get_wps_device_name},
	{"manufacturer",			hapd_get_wps_manufacturer},
	{"model_name",				hapd_get_wps_model_name},
	{"model_number",			hapd_get_wps_model_number},
	{"serial_number",			hapd_get_wps_serial_number},
	{"device_type", 			hapd_get_wps_device_type},
	{"config_methods",			hapd_get_wps_config_methods},
#endif
	{"network={",				hapd_get_wpas_network_start},
	{"ssid",					hapd_get_ssid},
	{"scan_ssid",				hapd_get_wpas_scan_ssid},
	{"key_mgmt",				hapd_get_wpa_key_mgmt},
#ifdef CONFIG_IEEE80211W
	{"ieee80211w",				hapd_get_ieee80211w},
#endif
	{"pairwise",				hapd_get_wpas_pairwise},
	{"psk",						hapd_get_wpas_psk},
	{"sae_password",			hapd_get_wpas_sae_password},
	{"pbss",					hapd_get_wpas_pbss},
	{"proto",					hapd_get_wpas_proto},
	{"auth_alg",				hapd_get_wpas_auth_algs},
	{"wep_key0",				hapd_get_wep_key0},
	{"wep_key1",				hapd_get_wep_key1},
	{"wep_key2",				hapd_get_wep_key2},
	{"wep_key3",				hapd_get_wep_key3},
	{"wep_tx_keyidx",			hapd_get_wep_tx_keyidx},
#ifdef RTK_HAPD_MULTI_AP
	{"multi_ap_backhaul_sta",	hapd_get_multi_ap_backhaul_sta},
#endif
	{"}",						hapd_get_wpas_network_start},
};
#endif

void dump_tmp(unsigned char *tmp)
{
	int idx = 0;

	printf("\n tmp=");

	for(idx=0; idx<50; idx++)
	printf("%02x ",tmp[idx]);

	printf("\n\n");
}

#if defined(MBSSID)
void set_hapd_vif_cfg(FILE *hapd_cfg,struct wifi_config *wifi_cfg, int vap_idx)
{
	int idx = 0;
	char result[HAPD_CONF_RESULT_MAX_LEN]={0};
	char name[64]={0};

	printf("\nGenerating VAP#%d \"%s\" configuration\n", vap_idx,
	        wifi_cfg->interface);

	fprintf(hapd_cfg, "\n# VAP#%d \"%s\" configuration\n\n", vap_idx,
	       wifi_cfg->interface);

	for (idx = 0; idx < ARRAY_SIZE(vap_hapd_conf_table); idx++) {

		// printf("[%s][%d] idx=%d name(%s) \n", __FUNCTION__, __LINE__, idx, vap_hapd_conf_table[idx].name);
		memset(result, 0, sizeof(result));

		//should use interface in vif conf
		if(!strcmp(vap_hapd_conf_table[idx].name, "bss"))
			strcpy(name, "interface");
		else
			strcpy(name, vap_hapd_conf_table[idx].name);

		if (vap_hapd_conf_table[idx].fun(wifi_cfg,
		    name, result) == HAPD_GET_OK) {
			if (result[0] == 0)
				continue;

			fprintf(hapd_cfg, "%s", result);

			//printf("[%s][%d] result:%s \n",
			//	__FUNCTION__, __LINE__, result);
		}
	}

}

int gen_hapd_vif_cfg(int if_idx, int vwlan_idx, char *conf_path)
{
	FILE *hapd_cfg=NULL;
	struct wifi_config wifi_cfg={0};
	char *default_conf_path;

	if(get_wifi_vif_conf(&wifi_cfg, if_idx, vwlan_idx))
		return -1;

	if(conf_path == NULL){
		printf("!! Error,  [%s][%d] conf = NULL \n", __FUNCTION__, __LINE__);
		return -1;
	}
	else
		hapd_cfg = fopen(conf_path, "w+");

	if(hapd_cfg == NULL){
		printf("!! Error,  [%s][%d] conf = NULL \n", __FUNCTION__, __LINE__);
		return -1;
	}

	set_hapd_vif_cfg(hapd_cfg,&wifi_cfg, vwlan_idx);

	fclose(hapd_cfg);

	return 0;
}
#endif

#if defined(CONFIG_RTK_HOSTAPD_ONLY_GLOBAL) || defined(MBSSID)

void launch_hapd_vif_by_cmd(int if_idx, int vwlan_index, int flag)
{
	char bss_config[256] = {0}, buffer[256] = {0}, global_ctrl_path[128] = {0};
	char ifname[20] = {0};
	unsigned int is_ax_support = 0;
#ifndef CONFIG_RTK_HOSTAPD_ONLY_GLOBAL
	char pid_path[64] = {0};
#endif

#ifdef CONFIG_RTK_HOSTAPD_ONLY_GLOBAL
	snprintf(global_ctrl_path, sizeof(global_ctrl_path), "%s", HOSTAPD_GLOBAL_CTRL_PATH);
#else
	snprintf(global_ctrl_path, sizeof(global_ctrl_path), "%s.%s", HOSTAPD_GLOBAL_CTRL_PATH, WLAN_IF_S[if_idx]);
#endif

	if(flag)
	{
#ifdef CONFIG_RTK_HOSTAPD_ONLY_GLOBAL
		if(vwlan_index >= 1)
		{
			snprintf(bss_config, sizeof(bss_config), "%s%d-vap%d", "/var/run/hapd_conf_wlan", if_idx, vwlan_index-1);

			if(gen_hapd_vif_cfg(if_idx, vwlan_index, bss_config))
				return;
		}
		else
		{
			snprintf(bss_config, sizeof(bss_config), "%s", CONF_PATH(if_idx));
			gen_hapd_cfg(if_idx);
		}
#else
		snprintf(bss_config, sizeof(bss_config), "%s%d-vap%d", "/var/run/hapd_conf_wlan", if_idx, vwlan_index-1);

		if(gen_hapd_vif_cfg(if_idx, vwlan_index, bss_config))
			return;
#endif

#ifdef CONFIG_BAND_2G_ON_WLAN0
		if(if_idx == 0)
			snprintf(buffer, sizeof(buffer)-strlen(bss_config), "bss_config=phy1:%s", bss_config);
		else if(if_idx == 1)
			snprintf(buffer, sizeof(buffer)-strlen(bss_config), "bss_config=phy0:%s", bss_config);
#else
		snprintf(buffer, sizeof(buffer)-strlen(bss_config), "bss_config=phy%d:%s", if_idx, bss_config);
#endif
		RunSystemCmd(NULL_FILE, WPAS_CLI, "-g", global_ctrl_path, "raw", "ADD", buffer, NULL_STR);

#ifndef CONFIG_RTK_HOSTAPD_ONLY_GLOBAL
		memset(ifname,0,sizeof(ifname));
		memset(pid_path,0,sizeof(pid_path));
		memset(buffer,0,sizeof(buffer));
		snprintf(ifname, sizeof(ifname), VAP_IF, if_idx, vwlan_index-1);
		snprintf(pid_path, sizeof(pid_path), "/var/run/hostapd_cli_systemd_%s.pid", ifname);
		snprintf(buffer, sizeof(buffer),"%s -a /bin/systemd_action -B -P %s -i %s", HOSTAPD_CLI, pid_path, ifname);
		system(buffer);
#endif
	}
	else
	{
		memset(ifname,0,sizeof(ifname));
#ifdef CONFIG_RTK_HOSTAPD_ONLY_GLOBAL
		if(vwlan_index >= 1)
			snprintf(ifname, sizeof(ifname), VAP_IF, if_idx, vwlan_index-1);
		else
			snprintf(ifname, sizeof(ifname), WLAN_IF_S[if_idx]);
#else
		snprintf(ifname, sizeof(ifname), VAP_IF, if_idx, vwlan_index-1);
#endif
		RunSystemCmd(NULL_FILE, IFCONFIG, ifname, "down", NULL_STR);
		RunSystemCmd(NULL_FILE, WPAS_CLI, "-g", global_ctrl_path, "raw", "REMOVE", ifname, NULL_STR);
		wlan_get_mib(MIB_WLAN_AX_SUPPORT, if_idx, 0, (void *)&is_ax_support);
	}
}
#endif

/*
	set_hapd_cfg:
		generate hostapd config file by wifi_conf
*/
void set_hapd_cfg(FILE *hapd_cfg, struct wifi_config *wifi_cfg)
{
	int idx, vap_idx;
	fprintf(hapd_cfg, "logger_stdout_level=4\n");
	fprintf(hapd_cfg, "# Root AP configuration\n\n");
	for(idx=0; idx<ARRAY_SIZE(hapd_conf_table); idx++){
		char result[HAPD_CONF_RESULT_MAX_LEN] = {0};

		if (hapd_conf_table[idx].fun(wifi_cfg, hapd_conf_table[idx].name, result) == HAPD_GET_OK){
			fprintf(hapd_cfg, "%s", result);
		}
	}

	for (vap_idx = 1; vap_idx <= NUM_VWLAN; vap_idx++) {
		if (wifi_cfg[vap_idx].disabled){
			printf("\n%s VAP%d disabled\n", wifi_cfg[0].interface, vap_idx);
			fprintf(hapd_cfg, "\n# %s VAP#%d disabled\n\n", wifi_cfg[0].interface, vap_idx);
			continue;
		}

		printf("\nGenerating VAP%d \"%s\" configuration\n", vap_idx,
		        wifi_cfg[vap_idx].interface);

		fprintf(hapd_cfg, "\n# VAP#%d \"%s\" configuration\n\n", vap_idx,
		        wifi_cfg[vap_idx].interface);

		for (idx = 0; idx < ARRAY_SIZE(vap_hapd_conf_table); idx++) {
			char result[HAPD_CONF_RESULT_MAX_LEN] = {0};

			if (vap_hapd_conf_table[idx].fun(&wifi_cfg[vap_idx],
			    vap_hapd_conf_table[idx].name, result) == HAPD_GET_OK) {
				if (result[0] == 0)
					continue;

				fprintf(hapd_cfg, "%s", result);
			}
		}

	}
}

void gen_hapd_cfg(int if_idx)
{
	FILE *hapd_cfg;
	struct wifi_config wifi_cfg[NUM_VWLAN + 1];

	memset(wifi_cfg, 0, sizeof(struct wifi_config)*(NUM_VWLAN + 1));

	char *conf_path = CONF_PATH(if_idx);

	printf("%s \n",__FUNCTION__);

	if (get_wifi_conf(&wifi_cfg[0], if_idx))
		return;

	hapd_cfg = fopen(conf_path, "w+");

	if(hapd_cfg == NULL){
		printf("!! Error,  [%s][%d] conf = NULL \n", __FUNCTION__, __LINE__);
		return;
	}

	set_hapd_cfg(hapd_cfg, &wifi_cfg[0]);

	fclose(hapd_cfg);

	sleep(2);
}

void launch_hapd_by_cmd(int if_idx)
{
	char global_ctrl_path[128] = {0}, coex_2g_40m_cmd[128] = {0}, pid_path[64] = {0};
	unsigned int force_40m = 0, channel = 0, is_ax_support = 0, i = 0, wlan_disable = 1;
	char cmd[200] = {0}, ifname[20] = {0};

	printf("launch hapd: wlan_idx = %d \n",if_idx);
	snprintf(global_ctrl_path, sizeof(global_ctrl_path), "%s.%s", HOSTAPD_GLOBAL_CTRL_PATH, WLAN_IF_S[if_idx]);

	//wlan_idx = if_idx here
	wlan_get_mib(MIB_WLAN_CHANNEL, if_idx, 0, (void *)&channel);
	wlan_get_mib(MIB_WLAN_AX_SUPPORT, if_idx, 0, (void *)&is_ax_support);
	if(channel==0)
		force_40m = 1;
	if(is_ax_support){
		if(force_40m){
			printf("%s(%d) echo 1 into %s coex_2g_40m\n", __FUNCTION__, __LINE__, WLAN_IF_S[if_idx]);
			snprintf(coex_2g_40m_cmd, sizeof(coex_2g_40m_cmd), "/bin/echo 1 > "LINK_FILE_NAME_FMT, WLAN_IF_S[if_idx], "coex_2g_40m");
			system(coex_2g_40m_cmd);
		}
		else {
			printf("%s(%d) echo 0 into %s coex_2g_40m\n", __FUNCTION__, __LINE__, WLAN_IF_S[if_idx]);
			snprintf(coex_2g_40m_cmd, sizeof(coex_2g_40m_cmd), "/bin/echo 0 > "LINK_FILE_NAME_FMT, WLAN_IF_S[if_idx], "coex_2g_40m");
			system(coex_2g_40m_cmd);
		}
	}
#if 0
	RunSystemCmd(NULL_FILE, "/bin/hostapd", CONF_PATH_WLAN0, "-g", global_ctrl_path, "-P", PID_PATH_WLAN0, "-B", NULL_STR);
#else
	snprintf(cmd, sizeof(cmd),"/bin/hostapd %s -g %s -P %s -B", CONF_PATH(if_idx), global_ctrl_path, PID_PATH(if_idx));
	system(cmd);
#endif
	if(rtk_get_interface_flag(WLAN_IF_S[if_idx], TIMER_COUNT, IS_RUN) == 0)
		printf("launch hapd: wlan_idx = %d failed\n",if_idx);
#if 0
	RunSystemCmd(NULL_FILE, HOSTAPD_CLI, "-a", "/bin/systemd_action", "-B", "-P", "/var/run/hostapd_cli_systemd_wlan0.pid", "-i", WLAN_IF_S[if_idx], NULL_STR);
#else
	snprintf(pid_path, sizeof(pid_path), "/var/run/hostapd_cli_systemd_wlan%d.pid", if_idx);
	snprintf(cmd, sizeof(cmd),"%s -a /bin/systemd_action -B -P %s -i %s", HOSTAPD_CLI, pid_path, WLAN_IF_S[if_idx]);
	system(cmd);

#if defined(MBSSID)
#if defined(UNIVERSAL_REPEATER)
	for(i = WLAN_VAP_ITF_INDEX; i < (WLAN_VAP_ITF_INDEX + NUM_VWLAN_INTERFACE -1); i++)
#else
	for(i = WLAN_VAP_ITF_INDEX; i < (WLAN_VAP_ITF_INDEX+NUM_VWLAN_INTERFACE); i++)
#endif
	{
		wlan_get_mib(MIB_WLAN_WLAN_DISABLED, if_idx, i, (void *)&wlan_disable);
		if(wlan_disable == 0) {
			memset(ifname,0,sizeof(ifname));
			memset(pid_path,0,sizeof(pid_path));
			memset(cmd,0,sizeof(cmd));
			snprintf(ifname, sizeof(ifname), VAP_IF, if_idx, i-1);
			snprintf(pid_path, sizeof(pid_path), "/var/run/hostapd_cli_systemd_%s.pid", ifname);
			snprintf(cmd, sizeof(cmd),"%s -a /bin/systemd_action -B -P %s -i %s", HOSTAPD_CLI, pid_path, ifname);
			system(cmd);
		}
	}
#endif
#endif
#if 0
	if(if_idx == 0){
		if (force_40m){
			printf("Force wlan0 40M co-exist.\n");
			/* ToDo: remove chip name in proc path */
			RunSystemCmd(NULL_FILE, "/bin/echo", "1", ">", "/proc/net/rtl8852ae/wlan0/coex_2g_40m", NULL_STR);
		}
#if 0
		RunSystemCmd(NULL_FILE, "/bin/hostapd", CONF_PATH_WLAN0, "-g", global_ctrl_path, "-P", PID_PATH_WLAN0, "-B", NULL_STR);
#else
		snprintf(cmd, sizeof(cmd),"/bin/hostapd %s -g %s -P %s -B", CONF_PATH_WLAN0, global_ctrl_path, PID_PATH_WLAN0);
		system(cmd);
#endif
		if(rtk_get_interface_flag(WLAN_IF_S[if_idx], TIMER_COUNT, IS_RUN) == 0)
			printf("launch hapd: wlan_idx = %d failed\n",if_idx);

#if 0
		RunSystemCmd(NULL_FILE, HOSTAPD_CLI, "-a", "/bin/systemd_action", "-B", "-P", "/var/run/hostapd_cli_systemd_wlan0.pid", "-i", WLAN_IF_S[if_idx], NULL_STR);
#else
		snprintf(cmd, sizeof(cmd),"%s -a /bin/systemd_action -B -P /var/run/hostapd_cli_systemd_wlan0.pid -i %s", HOSTAPD_CLI, WLAN_IF_S[if_idx]);
		system(cmd);
#endif
	}
	else if(if_idx == 1){
		if (force_40m) {
			printf("Force wlan1 40M co-exist.\n");
			RunSystemCmd(NULL_FILE, "/bin/echo", "1", ">", "/proc/net/rtl8852ae/wlan1/coex_2g_40m", NULL_STR);
		}
#if 0
		RunSystemCmd(NULL_FILE, "/bin/hostapd", CONF_PATH_WLAN1, "-g", global_ctrl_path, "-P", PID_PATH_WLAN1, "-B", NULL_STR);
#else
		snprintf(cmd, sizeof(cmd),"/bin/hostapd %s -g %s -P %s -B", CONF_PATH_WLAN1, global_ctrl_path, PID_PATH_WLAN1);
		system(cmd);
#endif
		if(rtk_get_interface_flag(WLAN_IF_S[if_idx], TIMER_COUNT, IS_RUN) == 0)
			printf("launch hapd: wlan_idx = %d failed\n",if_idx);
#if 0
		RunSystemCmd(NULL_FILE, HOSTAPD_CLI, "-a", "/bin/systemd_action", "-B", "-P", "/var/run/hostapd_cli_systemd_wlan1.pid", "-i", WLAN_IF_S[if_idx], NULL_STR);
#else
		snprintf(cmd, sizeof(cmd),"%s -a /bin/systemd_action -B -P /var/run/hostapd_cli_systemd_wlan1.pid -i %s", HOSTAPD_CLI, WLAN_IF_S[if_idx]);
		system(cmd);
#endif
	}else{
		printf("[Error] cannot get interface index !\n");
	}
#endif
}

void set_wpas_cfg(FILE *wpas_cfg, struct wifi_config *wifi_cfg)
{
	int idx, vap_idx;
#ifdef UNIVERSAL_REPEATER
	if(wifi_cfg->is_repeater)
		fprintf(wpas_cfg, "# Vxd CLIENT configuration\n\n");
	else
#endif
		fprintf(wpas_cfg, "# Root CLIENT configuration\n\n");
	for(idx=0; idx<ARRAY_SIZE(wpas_conf_table); idx++){
		char result[HAPD_CONF_RESULT_MAX_LEN] = {0};

		if (wpas_conf_table[idx].fun(wifi_cfg, wpas_conf_table[idx].name, result) == HAPD_GET_OK){
			fprintf(wpas_cfg, "%s", result);
		}
	}
}

void gen_wpas_cfg(int if_idx, int index)
{
	FILE *wpas_cfg;
	struct wifi_config wifi_cfg;
	char wpas_conf_path[50]={0};
	char tmp_cmd[100]={0}, ifname[20]={0};
	FILE *wpas_wps_stas;
	unsigned int is_ax_support=0;

	memset(&wifi_cfg, 0, sizeof(struct wifi_config));

#ifdef UNIVERSAL_REPEATER
	if(index == NUM_VWLAN_INTERFACE)
	{
		snprintf(wpas_conf_path, sizeof(wpas_conf_path), "%s_vxd", WPAS_CONF_PATH(if_idx));
		wifi_cfg.is_repeater = 1;
	}
	else
#endif
	{
		snprintf(wpas_conf_path, sizeof(wpas_conf_path), "%s", WPAS_CONF_PATH(if_idx));
	}

	if (get_wifi_conf(&wifi_cfg, if_idx))
		return;

	printf("%s \n",__FUNCTION__);

	wpas_cfg = fopen(wpas_conf_path, "w+");

	if(wpas_cfg == NULL){
		printf("!! Error,  [%s][%d] wpas_conf = NULL \n", __FUNCTION__, __LINE__);
		return;
	}

	set_wpas_cfg(wpas_cfg, &wifi_cfg);

	fclose(wpas_cfg);

	sleep(2);

	wlan_get_mib(MIB_WLAN_AX_SUPPORT, if_idx, 0, (void *)&is_ax_support);
#ifdef UNIVERSAL_REPEATER
	if(index == NUM_VWLAN_INTERFACE)
	{
		if(is_ax_support)
		{
			snprintf(tmp_cmd, sizeof(tmp_cmd), "/var/exist_wlan%d-vxd", if_idx);
			if(-1 == access(tmp_cmd, F_OK))
			{
				/*command "iw phy " will lookup file /sys/class/ieee80211/phy$x/index
				  which is generated by rtl_mount_sysfs() int set_init.c
				*/
#ifdef CONFIG_BAND_2G_ON_WLAN0
				if(if_idx == 0)
					snprintf(tmp_cmd, sizeof(tmp_cmd), "/usr/bin/iw phy phy1 interface add %s type station", VXD_IF[if_idx]);
				else if(if_idx == 1)
					snprintf(tmp_cmd, sizeof(tmp_cmd), "/usr/bin/iw phy phy0 interface add %s type station", VXD_IF[if_idx]);
#else
				snprintf(tmp_cmd, sizeof(tmp_cmd), "/usr/bin/iw phy phy%d interface add %s type station", if_idx, VXD_IF[if_idx]);
#endif
				system(tmp_cmd);
				snprintf(tmp_cmd, sizeof(tmp_cmd), "echo 1 > /var/exist_wlan%d-vxd", if_idx);
				system(tmp_cmd);
			}
		}
		start_wifi_priv_cfg(if_idx, index);
	}
#endif

#ifdef UNIVERSAL_REPEATER
	if(index == NUM_VWLAN_INTERFACE)
		snprintf(ifname, sizeof(ifname), "%s", VXD_IF[if_idx]);
	else
#endif
		snprintf(ifname, sizeof(ifname), "%s", WLAN_IF_S[if_idx]);

	if(is_ax_support || index != NUM_VWLAN_INTERFACE)
	{
		snprintf(tmp_cmd, sizeof(tmp_cmd), "ifconfig %s hw ether %s", ifname, wifi_cfg.bssid);
		system(tmp_cmd);
	}

	wpas_wps_stas = fopen(WPAS_WPS_STAS, "w+");
	if(wpas_wps_stas == NULL){
		printf("!! Error,  [%s][%d] wpas_wps_stas = NULL \n", __FUNCTION__, __LINE__);
		return;
	}
	fprintf(wpas_wps_stas, "%s", "PBC Status: Disabled\n");
	fprintf(wpas_wps_stas, "%s", "Last WPS result: None\n");
	fclose(wpas_wps_stas);
}

void launch_wpas_by_cmd(int if_idx, int index)
{
	char tmp_cmd[100]={0}, ifname[50]={0};

	printf("launch wpas: wlan_idx = %d vwlan_idx = %d\n",if_idx, index);

	if(if_idx == 0){
#ifdef UNIVERSAL_REPEATER
		if(index == NUM_VWLAN_INTERFACE)
		{
			RunSystemCmd(NULL_FILE, "/bin/wpa_supplicant", "-D", "nl80211", "-b",NAME_BR, "-i", VXD_IF[if_idx],"-c", WPAS_CONF_PATH_WLAN0_VXD, "-P", WPAS_PID_PATH_WLAN0_VXD, "-B", NULL_STR);
			if(rtk_get_interface_flag(VXD_IF[if_idx], TIMER_COUNT, IS_RUN) == 0)
				printf("launch hapd: wlan_idx = %d failed\n",if_idx);
			RunSystemCmd(NULL_FILE, "brctl", "addif", NAME_BR, VXD_IF[if_idx], NULL_STR);
		}
		else
#endif
		{
			RunSystemCmd(NULL_FILE, "/bin/wpa_supplicant", "-D", "nl80211", "-b",NAME_BR, "-i", WLAN_IF_S[if_idx],"-c", WPAS_CONF_PATH_WLAN0, "-P", WPAS_PID_PATH_WLAN0, "-B", NULL_STR);
			if(rtk_get_interface_flag(WLAN_IF_S[if_idx], TIMER_COUNT, IS_RUN) == 0)
				printf("launch hapd: wlan_idx = %d failed\n",if_idx);
			RunSystemCmd(NULL_FILE, "brctl", "addif", NAME_BR, WLAN_IF_S[if_idx], NULL_STR);
		}

#ifdef UNIVERSAL_REPEATER
		if(index == NUM_VWLAN_INTERFACE){
			//RunSystemCmd(NULL_FILE, WPAS_CLI, "-a", "/bin/systemd_action", "-B", "-P", "/var/run/wpas_cli_systemd_wlan0_vxd.pid", "-i", VXD_IF[if_idx], NULL_STR);
			snprintf(tmp_cmd, sizeof(tmp_cmd), "%s -a /bin/systemd_action -B -P /var/run/wpas_cli_systemd_wlan0_vxd.pid -i %s", WPAS_CLI, VXD_IF[if_idx]);
			system(tmp_cmd);
		}
		else
#endif
		{
			//RunSystemCmd(NULL_FILE, WPAS_CLI, "-a", "/bin/systemd_action", "-B", "-P", "/var/run/wpas_cli_systemd_wlan0.pid", "-i", WLAN_IF_S[if_idx], NULL_STR);
			snprintf(tmp_cmd, sizeof(tmp_cmd), "%s -a /bin/systemd_action -B -P /var/run/wpas_cli_systemd_wlan0.pid -i %s", WPAS_CLI, WLAN_IF_S[if_idx]);
			system(tmp_cmd);
		}
	}
	else if(if_idx == 1){
#ifdef UNIVERSAL_REPEATER
		if(index == NUM_VWLAN_INTERFACE)
		{
			RunSystemCmd(NULL_FILE, "/bin/wpa_supplicant", "-D", "nl80211", "-b",NAME_BR, "-i", VXD_IF[if_idx],"-c", WPAS_CONF_PATH_WLAN1_VXD, "-P", WPAS_PID_PATH_WLAN1_VXD, "-B", NULL_STR);
			if(rtk_get_interface_flag(VXD_IF[if_idx], TIMER_COUNT, IS_RUN) == 0)
				printf("launch hapd: wlan_idx = %d failed\n",if_idx);
			RunSystemCmd(NULL_FILE, "brctl", "addif", NAME_BR, VXD_IF[if_idx], NULL_STR);
		}
		else
#endif
		{
			RunSystemCmd(NULL_FILE, "/bin/wpa_supplicant", "-D", "nl80211", "-b",NAME_BR, "-i", WLAN_IF_S[if_idx],"-c", WPAS_CONF_PATH_WLAN1, "-P", WPAS_PID_PATH_WLAN1, "-B", NULL_STR);
			if(rtk_get_interface_flag(WLAN_IF_S[if_idx], TIMER_COUNT, IS_RUN) == 0)
				printf("launch hapd: wlan_idx = %d failed\n",if_idx);
			RunSystemCmd(NULL_FILE, "brctl", "addif", NAME_BR, WLAN_IF_S[if_idx], NULL_STR);
		}

#ifdef UNIVERSAL_REPEATER
		if(index == NUM_VWLAN_INTERFACE){
			//RunSystemCmd(NULL_FILE, WPAS_CLI, "-a", "/bin/systemd_action", "-B", "-P", "/var/run/wpas_cli_systemd_wlan1_vxd.pid", "-i", VXD_IF[if_idx], NULL_STR);
			snprintf(tmp_cmd, sizeof(tmp_cmd), "%s -a /bin/systemd_action -B -P /var/run/wpas_cli_systemd_wlan1_vxd.pid -i %s", WPAS_CLI, VXD_IF[if_idx]);
			system(tmp_cmd);
		}
		else
#endif
		{
			//RunSystemCmd(NULL_FILE, WPAS_CLI, "-a", "/bin/systemd_action", "-B", "-P", "/var/run/wpas_cli_systemd_wlan1.pid", "-i", WLAN_IF_S[if_idx], NULL_STR);
			snprintf(tmp_cmd, sizeof(tmp_cmd), "%s -a /bin/systemd_action -B -P /var/run/wpas_cli_systemd_wlan1.pid -i %s", WPAS_CLI, WLAN_IF_S[if_idx]);
			system(tmp_cmd);
		}
	}
	else{
		printf("[Error] cannot get interface index !\n");
	}
}

void start_hapd_wpas_process(int wlan_index, config_wlan_ssid ssid_index)
{
	int up_root = 0, i;
#ifdef UNIVERSAL_REPEATER
	int up_vxd = 0;
#endif
#ifdef MBSSID
	int up_vap = 0;
#endif
	unsigned int wlanDisable=1, wlanMode, intValue,is_ax_support=0, setVal=0;
	char tmp_cmd[100]={0}, ifname[20]={0};
	char phy_file[100]={0}, tmpfile[100]={0};

#ifdef UNIVERSAL_REPEATER
	if(ssid_index == NUM_VWLAN_INTERFACE)
		up_vxd = 1;
	else
#endif
	{
		if(ssid_index == CONFIG_SSID_ALL || ssid_index == CONFIG_SSID_ROOT)
		{
			up_root = 1;
#ifdef UNIVERSAL_REPEATER
			if(ssid_index == CONFIG_SSID_ALL)
				up_vxd = 1;
#endif
		}
		else
		{
#ifdef MBSSID
			up_vap = 1;
#endif
		}
	}

	apmib_save_wlanIdx();

	wlan_idx = wlan_index;
	vwlan_idx = 0;
	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisable);

	if (wlanDisable){
		apmib_recov_wlanIdx();
		return;
	}

	if(up_root)
	{
		apmib_get(MIB_WLAN_MODE, (void *)&wlanMode);
		if(wlanMode == AP_MODE)
		{
			gen_hapd_cfg(wlan_idx);
			launch_hapd_by_cmd(wlan_idx);
		}
		else if(wlanMode == CLIENT_MODE)
		{
			gen_wpas_cfg(wlan_idx, 0);
			launch_wpas_by_cmd(wlan_idx, 0);
		}
		if(wlanMode == AP_MODE) {
#if defined(MBSSID)
#if defined(UNIVERSAL_REPEATER)
			for(i = WLAN_VAP_ITF_INDEX; i < (WLAN_VAP_ITF_INDEX + NUM_VWLAN_INTERFACE -1); i++)
#else
			for(i = WLAN_VAP_ITF_INDEX; i < (WLAN_VAP_ITF_INDEX+NUM_VWLAN_INTERFACE); i++)
#endif
			{
				vwlan_idx = i;
				apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisable);
				if(wlanDisable == 0) {
					snprintf(ifname,sizeof(ifname),VAP_IF,wlan_idx,i-1);
					if(rtk_get_interface_flag(ifname, TIMER_COUNT, IS_RUN) == 0)
						printf("launch hapd: %s failed\n",ifname);
				}
			}
#endif
		}
	}

#ifdef UNIVERSAL_REPEATER
	if(up_vxd)
	{
		if(wlan_idx==0)
			apmib_get(MIB_REPEATER_ENABLED1, (void *)&intValue);
		else
			apmib_get(MIB_REPEATER_ENABLED2, (void *)&intValue);
		if(intValue)
		{
			vwlan_idx = NUM_VWLAN_INTERFACE;
			apmib_get(MIB_WLAN_MODE, (void *)&wlanMode);
			snprintf(phy_file, sizeof(phy_file), PHY_INDEX_FILE, wlan_index);
			wlan_get_mib(MIB_WLAN_AX_SUPPORT, wlan_idx, 0, (void *)&is_ax_support);
			if(wlanMode == CLIENT_MODE)
			{
				if(is_ax_support && !isFileExist(phy_file)){
					snprintf(ifname,sizeof(ifname),"%s", VXD_IF[wlan_index]);
					printf("register %s later !\n", ifname);
					snprintf(tmpfile, sizeof(tmpfile), START_VXD_LATER, wlan_idx);
					snprintf(tmp_cmd, sizeof(tmp_cmd), "echo 1 > %s", tmpfile);
					system(tmp_cmd);
				}
				else
				{
#if defined(MBSSID)
				for (i = WLAN_VAP_ITF_INDEX; i < NUM_VWLAN_INTERFACE; i++)
				{
					vwlan_idx = i;
					apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisable);
					if(wlanDisable == 0)
					{
						snprintf(ifname,sizeof(ifname),VAP_IF,wlan_idx,i-1);
						if(rtk_get_interface_flag(ifname, TIMER_COUNT, IS_RUN) == 0)
							printf("launch hapd: %s failed\n",ifname);
					}
				}
#endif
					gen_wpas_cfg(wlan_idx, NUM_VWLAN_INTERFACE);
					launch_wpas_by_cmd(wlan_idx, NUM_VWLAN_INTERFACE);
				}
			}
		}
	}
#endif

#if defined(MBSSID)
	if(up_vap)
	{
		vwlan_idx = ssid_index; //start from 1, ...
		apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisable);
		if(!wlanDisable)
		{
			launch_hapd_vif_by_cmd(wlan_idx, ssid_index, 1);
		}
	}
#endif

	apmib_recov_wlanIdx();
}

void stop_hapd_wpas_process(int wlan_index, config_wlan_ssid ssid_index)
{
	int hapd_pid, down_root = 0, j;
	unsigned int is_ax_support=0;
#ifdef UNIVERSAL_REPEATER
	int down_vxd = 0;
#endif
#if defined(MBSSID)
	int down_vap = 0;
	char ifname[20] = {0};
#endif
	int hapd_cli_pid;
	char tmp_buf[100]={0};

#ifdef CONFIG_LEDS_GPIO
	system("echo none > /sys/class/leds/LED_WPS_G/trigger");
	system("echo 0 > /sys/class/leds/LED_WPS_G/brightness");
#endif

	wlan_get_mib(MIB_WLAN_AX_SUPPORT, wlan_index, 0, (void *)&is_ax_support);
#ifdef UNIVERSAL_REPEATER
	if(ssid_index == NUM_VWLAN_INTERFACE)
		down_vxd = 1;
	else
#endif
	{
		if(ssid_index == CONFIG_SSID_ALL || ssid_index == CONFIG_SSID_ROOT)
		{
			down_root = 1;
#ifdef UNIVERSAL_REPEATER
			if(ssid_index == CONFIG_SSID_ALL)
				down_vxd = 1;
#endif
#if defined(MBSSID)
			if(ssid_index == CONFIG_SSID_ALL)
			{
				down_vap = 1;
			}
#endif
		}
		else
		{
#if defined(MBSSID)
			down_vap = 1;
#endif
		}
	}

	/* kill hostapd_cli for root*/
	if(down_root)
	{
		memset(tmp_buf,0,sizeof(tmp_buf));
		snprintf(tmp_buf, sizeof(tmp_buf), "/var/run/hostapd_cli_systemd_wlan%d.pid", wlan_index);
		hapd_cli_pid = read_pid(tmp_buf);
		if(hapd_cli_pid > 0){
			kill(hapd_cli_pid, 15);
			unlink(tmp_buf);
		}
	}

	/* kill wpa_supplicant_cli for root interface */
	if(down_root)
	{
		memset(tmp_buf,0,sizeof(tmp_buf));
		snprintf(tmp_buf, sizeof(tmp_buf), "/var/run/wpas_cli_systemd_wlan%d.pid", wlan_index);
		hapd_cli_pid = read_pid(tmp_buf);
		if(hapd_cli_pid > 0){
			kill(hapd_cli_pid, 15);
			unlink(tmp_buf);
		}
	}

#if defined(MBSSID)
	if(down_vap)
	{
		for (j = WLAN_VAP_ITF_INDEX; j < NUM_VWLAN_INTERFACE; j++)
		{
			memset(ifname,0,sizeof(ifname));
			rtk_wlan_get_ifname(wlan_index, j, ifname);
			memset(tmp_buf,0,sizeof(tmp_buf));
			snprintf(tmp_buf, sizeof(tmp_buf), "/var/run/hostapd_cli_systemd_%s.pid", ifname);
			hapd_cli_pid = read_pid(tmp_buf);
			if(hapd_cli_pid > 0){
				kill(hapd_cli_pid, 15);
				unlink(tmp_buf);
			}
		}
	}
#endif

#ifdef UNIVERSAL_REPEATER
	if(down_vxd)
	{
		/* kill wpa_supplicant_cli for vxd interface */
		memset(tmp_buf,0,sizeof(tmp_buf));
		snprintf(tmp_buf, sizeof(tmp_buf), "/var/run/wpas_cli_systemd_wlan%d_vxd.pid", wlan_index);
		hapd_cli_pid = read_pid(tmp_buf);
		if(hapd_cli_pid > 0){
			kill(hapd_cli_pid, 15);
			unlink(tmp_buf);
		}

		/* kill wpa_supplicant for vxd interface */
		RunSystemCmd(NULL_FILE, IFCONFIG, VXD_IF[wlan_index], "down", NULL_STR);
		memset(tmp_buf,0,sizeof(tmp_buf));
		if(wlan_index == 0)
			snprintf(tmp_buf, sizeof(tmp_buf), "%s", WPAS_PID_PATH_WLAN0_VXD);
		else if(wlan_index == 1)
			snprintf(tmp_buf, sizeof(tmp_buf), "%s", WPAS_PID_PATH_WLAN1_VXD);
		hapd_cli_pid = read_pid(tmp_buf);
		if(hapd_cli_pid > 0){
			kill(hapd_cli_pid, 15);
			unlink(tmp_buf);
		}
		RunSystemCmd(NULL_FILE, "brctl", "delif", NAME_BR, VXD_IF[wlan_index], NULL_STR);
	}
#endif

#if defined(MBSSID)
	if(down_vap)
	{
		if(ssid_index == CONFIG_SSID_ALL)
		{
			for (j=WLAN_VAP_ITF_INDEX; j<(NUM_VWLAN_INTERFACE); j++)
			{
				launch_hapd_vif_by_cmd(wlan_index, j, 0);
			}
		}
		else
			launch_hapd_vif_by_cmd(wlan_index, ssid_index, 0);
	}
#endif

	/* kill hostapd & wpa_supplicant for root*/
	if(down_root)
	{
		RunSystemCmd(NULL_FILE, IFCONFIG, WLAN_IF_S[wlan_index], "down", NULL_STR);
		memset(tmp_buf,0,sizeof(tmp_buf));
		if(wlan_index == 0)
			snprintf(tmp_buf, sizeof(tmp_buf), "%s", PID_PATH_WLAN0);
		else if(wlan_index == 1)
			snprintf(tmp_buf, sizeof(tmp_buf), "%s", PID_PATH_WLAN1);
		hapd_cli_pid = read_pid(tmp_buf);
		if(hapd_cli_pid > 0){
			kill(hapd_cli_pid, 15);
			unlink(tmp_buf);
		}

#ifdef CONFIG_WLAN_CLIENT_MODE
		memset(tmp_buf,0,sizeof(tmp_buf));
		if(wlan_index == 0)
			snprintf(tmp_buf, sizeof(tmp_buf), "%s", WPAS_PID_PATH_WLAN0);
		else if(wlan_index == 1)
			snprintf(tmp_buf, sizeof(tmp_buf), "%s", WPAS_PID_PATH_WLAN1);
		hapd_cli_pid = read_pid(tmp_buf);
		if(hapd_cli_pid > 0){
			kill(hapd_cli_pid, 15);
			unlink(tmp_buf);
			RunSystemCmd(NULL_FILE, "brctl", "delif", NAME_BR, WLAN_IF_S[wlan_index], NULL_STR);
		}
#endif
	}
}

#ifdef CONFIG_RTK_HOSTAPD_ONLY_GLOBAL
int hapd_is_dif_phy_band(config_wlan_target target, int wlan_index)
{
	int ret = 0;
	int phy_band_select = 0;
	int wlan_index_back = 0, vwlan_index_back = 0;

	wlan_index_back = wlan_idx;
	vwlan_index_back= vwlan_idx;

	wlan_idx = wlan_index;
	vwlan_idx = 0;
	apmib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&phy_band_select);
	if((target == CONFIG_WLAN_2G && phy_band_select == PHYBAND_5G)
		|| (target == CONFIG_WLAN_5G && phy_band_select == PHYBAND_2G)) {
		ret = 1;
	}

	wlan_idx = wlan_index_back;
	vwlan_idx = vwlan_index_back;

	return ret;
}

void start_hapd_global_process(config_wlan_target target, config_wlan_ssid ssid_index)
{
	int up_root = 0;
#ifdef MBSSID
	int up_vap = 0;
	char ifname[20] = {0};
#endif
	int i, j;
	unsigned int wlan_disable = 1, wlan_mode = 0;
	char global_conf_path[128] = {0};
	char cmd[200] = {0};
	int wlan_index_back = 0, vwlan_index_back = 0;

#ifdef UNIVERSAL_REPEATER
	if(ssid_index != NUM_VWLAN_INTERFACE)
#endif
	{
		if(ssid_index == CONFIG_SSID_ALL || ssid_index == CONFIG_SSID_ROOT)
		{
			up_root = 1;
		}
		else
		{
#ifdef MBSSID
			up_vap = 1;
#endif
		}
	}

	wlan_index_back = wlan_idx;
	vwlan_index_back= vwlan_idx;

	for(i = 0; i<NUM_WLAN_INTERFACE; i++)
	{
		wlan_idx = i;
		vwlan_idx = 0;

		if(hapd_is_dif_phy_band(target, i))
			continue;

		wlan_disable = 1;
		apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disable);
		if(wlan_disable)
			continue;

		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
		if(wlan_mode == CLIENT_MODE)
			continue;

		if((target == CONFIG_WLAN_ALL) && up_root)
		{
			gen_hapd_cfg(i);
			strcat(global_conf_path, CONF_PATH(i));
			strcat(global_conf_path, " ");
		}

		if(((target == CONFIG_WLAN_2G) || (target == CONFIG_WLAN_5G)) && up_root)
		{
			launch_hapd_vif_by_cmd(i, 0, 1);
			if(rtk_get_interface_flag(WLAN_IF_S[i], TIMER_COUNT, IS_RUN) == 0)
				printf("launch hapd: wlan_idx = %d failed\n",i);
		}

#if defined(MBSSID)
		if(up_vap)
		{
			vwlan_idx = ssid_index; //start from 1, ...
			wlan_disable = 1;
			apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disable);
			if(!wlan_disable)
			{
				launch_hapd_vif_by_cmd(i, ssid_index, 1);
				snprintf(ifname,sizeof(ifname),VAP_IF,i,ssid_index-1);
				if(rtk_get_interface_flag(ifname, TIMER_COUNT, IS_RUN) == 0)
					printf("launch hapd: %s failed\n",ifname);
			}
		}
#endif
	}

	if((target == CONFIG_WLAN_ALL) && up_root)
	{
		if(strlen(global_conf_path))
		{
			snprintf(cmd, sizeof(cmd),"/bin/hostapd -g %s -P %s -B %s", HOSTAPD_GLOBAL_CTRL_PATH, HOSTAPD_GLOBAL_PID_PATH, global_conf_path);
			system(cmd);

			for(i = 0; i<NUM_WLAN_INTERFACE; i++){
				wlan_idx = i;
				vwlan_idx = 0;
				wlan_disable = 1;

				apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disable);
				if(wlan_disable)
					continue;

				apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
				if(wlan_mode == CLIENT_MODE)
					continue;

				if(rtk_get_interface_flag(WLAN_IF_S[i], TIMER_COUNT, IS_RUN) == 0)
					printf("launch hapd: wlan_idx = %d failed\n",i);

#if defined(MBSSID)
				for (j = WLAN_VAP_ITF_INDEX; j < (NUM_VWLAN+1); j++)
				{
					vwlan_idx = j;
					wlan_disable = 1;
					apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disable);
					if(wlan_disable == 0)
					{
						snprintf(ifname,sizeof(ifname),VAP_IF,i,j-1);
						if(rtk_get_interface_flag(ifname, TIMER_COUNT, IS_RUN) == 0)
							printf("launch hapd: %s failed\n",ifname);

					}
				}
#endif
			}

			snprintf(cmd, sizeof(cmd),"%s -a /bin/systemd_action -B -P %s -i %s", HOSTAPD_CLI, "/var/run/hostapd_cli_systemd.pid", HOSTAPD_GLOBAL);
			system(cmd);
		}
	}

	wlan_idx = wlan_index_back;
	vwlan_idx = vwlan_index_back;
}

void stop_hapd_wpas_global_process(config_wlan_target target, config_wlan_ssid ssid_index)
{
	int down_root = 0;
#ifdef UNIVERSAL_REPEATER
	int down_vxd = 0;
#endif
#if defined(MBSSID)
	int down_vap = 0;
#endif
	int hapd_cli_pid;
	char tmp_buf[100]={0};
	int i, j;
	int wlan_index_back = 0, vwlan_index_back = 0;
	unsigned int wlan_mode = 0;

#ifdef UNIVERSAL_REPEATER
	if(ssid_index == NUM_VWLAN_INTERFACE)
		down_vxd = 1;
	else
#endif
	{
		if(ssid_index == CONFIG_SSID_ALL || ssid_index == CONFIG_SSID_ROOT)
		{
			down_root = 1;
#ifdef UNIVERSAL_REPEATER
			if(ssid_index == CONFIG_SSID_ALL)
				down_vxd = 1;
#endif
#if defined(MBSSID)
			if(ssid_index == CONFIG_SSID_ALL)
			{
				down_vap = 1;
			}
#endif
		}
		else
		{
#if defined(MBSSID)
			down_vap = 1;
#endif
		}
	}

#ifdef CONFIG_LEDS_GPIO
		system("echo none > /sys/class/leds/LED_WPS_G/trigger");
		system("echo 0 > /sys/class/leds/LED_WPS_G/brightness");
#endif

	wlan_index_back = wlan_idx;
	vwlan_index_back= vwlan_idx;

	if((target == CONFIG_WLAN_ALL) && down_root)
	{
		memset(tmp_buf,0,sizeof(tmp_buf));
		snprintf(tmp_buf, sizeof(tmp_buf), "/var/run/hostapd_cli_systemd.pid");
		hapd_cli_pid = read_pid(tmp_buf);
		if(hapd_cli_pid > 0){
			kill(hapd_cli_pid, 15);
			unlink(tmp_buf);
		}
	}

	for(i = 0; i<NUM_WLAN_INTERFACE; i++){
		if(hapd_is_dif_phy_band(target, i))
			continue;

#ifdef UNIVERSAL_REPEATER
		if(down_vxd)
		{
			/* kill wpa_supplicant_cli for vxd interface */
			memset(tmp_buf,0,sizeof(tmp_buf));
			snprintf(tmp_buf, sizeof(tmp_buf), "/var/run/wpas_cli_systemd_wlan%d_vxd.pid", i);
			hapd_cli_pid = read_pid(tmp_buf);
			if(hapd_cli_pid > 0){
				kill(hapd_cli_pid, 15);
				unlink(tmp_buf);
			}

			/* kill wpa_supplicant for vxd interface */
			RunSystemCmd(NULL_FILE, IFCONFIG, VXD_IF[i], "down", NULL_STR);
			memset(tmp_buf,0,sizeof(tmp_buf));
			if(i == 0)
				snprintf(tmp_buf, sizeof(tmp_buf), "%s", WPAS_PID_PATH_WLAN0_VXD);
			else if(i == 1)
				snprintf(tmp_buf, sizeof(tmp_buf), "%s", WPAS_PID_PATH_WLAN1_VXD);
			hapd_cli_pid = read_pid(tmp_buf);
			if(hapd_cli_pid > 0){
				kill(hapd_cli_pid, 15);
				unlink(tmp_buf);
			}
			RunSystemCmd(NULL_FILE, "brctl", "delif", NAME_BR, VXD_IF[i], NULL_STR);
		}
#endif
#if defined(MBSSID)
		if(down_vap)
		{
			if(ssid_index == CONFIG_SSID_ALL)
			{
				for (j=WLAN_VAP_ITF_INDEX; j<(NUM_VWLAN_INTERFACE); j++)
				{
					launch_hapd_vif_by_cmd(i, j, 0);
				}
			}
			else
				launch_hapd_vif_by_cmd(i, ssid_index, 0);
		}
#endif

		if(down_root)
		{
			memset(tmp_buf,0,sizeof(tmp_buf));
			snprintf(tmp_buf, sizeof(tmp_buf), "/var/run/wpas_cli_systemd_wlan%d.pid", i);
			hapd_cli_pid = read_pid(tmp_buf);
			if(hapd_cli_pid > 0){
				kill(hapd_cli_pid, 15);
				unlink(tmp_buf);
			}

			RunSystemCmd(NULL_FILE, IFCONFIG, WLAN_IF_S[i], "down", NULL_STR);
#ifdef CONFIG_WLAN_CLIENT_MODE
			memset(tmp_buf,0,sizeof(tmp_buf));
			if(i == 0)
				snprintf(tmp_buf, sizeof(tmp_buf), "%s", WPAS_PID_PATH_WLAN0);
			else if(i == 1)
				snprintf(tmp_buf, sizeof(tmp_buf), "%s", WPAS_PID_PATH_WLAN1);
			hapd_cli_pid = read_pid(tmp_buf);
			if(hapd_cli_pid > 0){
				kill(hapd_cli_pid, 15);
				unlink(tmp_buf);
				RunSystemCmd(NULL_FILE, "brctl", "delif", NAME_BR, WLAN_IF_S[i], NULL_STR);
			}
#endif
			wlan_idx = i;
			vwlan_idx = 0;
			apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
			if(wlan_mode == AP_MODE)
				launch_hapd_vif_by_cmd(i, 0, 0);
		}
	}

	if((target == CONFIG_WLAN_ALL) && down_root)
	{
		snprintf(tmp_buf, sizeof(tmp_buf), "%s", HOSTAPD_GLOBAL_PID_PATH);
		hapd_cli_pid = read_pid(tmp_buf);
		if(hapd_cli_pid > 0){
			kill(hapd_cli_pid, 15);
			unlink(tmp_buf);
		}
	}

	wlan_idx = wlan_index_back;
	vwlan_idx = vwlan_index_back;
}
#endif
