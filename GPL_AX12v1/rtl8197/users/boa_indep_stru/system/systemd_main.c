#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "apmib.h"
#include "systemd_common.h"

const char HOSTAPD[] = "/bin/hostapd";
const char HOSTAPD_CLI[] = "/bin/hostapd_cli";
const char WPAS_CLI[] = "/bin/wpa_cli";
const char SYSTEMD_ACTION[]	= "/bin/systemd_action";

CONTEXT_S g_context;

char *get_token(char *data, char *token)
{
	char *ptr=data;
	int len=0, idx=0;
	while (*ptr && *ptr != '\n' ) {
		if (*ptr == '=') {
			if (len <= 1)
				return NULL;
			memcpy(token, data, len);

			/* delete ending space */
			for (idx=len-1; idx>=0; idx--) {
				if (token[idx] !=  ' ')
					break;
			}
			token[idx+1] = '\0';

			return ptr+1;
		}
		len++;
		ptr++;
	}
	return NULL;
}

int get_value(char *data, char *value)
{
	char *ptr=data;
	int len=0, idx, i;

	while (*ptr && *ptr != '\n' && *ptr != '\r') {
		len++;
		ptr++;
	}

	/* delete leading space */
	idx = 0;
	while (len-idx > 0) {
		if (data[idx] != ' ')
			break;
		idx++;
	}
	len -= idx;

	/* delete bracing '"' */
	if (data[idx] == '"') {
		for (i=idx+len-1; i>idx; i--) {
			if (data[i] == '"') {
				idx++;
				len = i - idx;
			}
			break;
		}
	}

	if (len > 0) {
		memcpy(value, &data[idx], len);
		value[len] = '\0';
	}
	return len;
}

unsigned int is_root_interface(char *ifname)
{
	int ret = 0;

	if(!ifname)
		return ret;
	if(strstr(ifname, VXD_IF_SUFFIX) || strstr(ifname, VAP_IF_SUFFIX))
		return 0;
	else
		return 1;
}

int check_wlanif_valid(char *wlan_iface_name)
{
	if(!wlan_iface_name)
	{
		printf("wlan ifname NULL\n");
		return 0;
	}
	if(!strstr(wlan_iface_name, WLAN_IF_PREFIX)){
		printf("unkown ifname %s\n", wlan_iface_name);
		return 0;
	}else{
		if(strlen(wlan_iface_name) > strlen(WLAN_IF_PREFIX)+1){
			if(strstr(wlan_iface_name, VAP_IF_SUFFIX))
				return 1;
			else if(strstr(wlan_iface_name, VXD_IF_SUFFIX))
				return 1;
			else
				return 0;
		}else
			return 1;
	}

	return 1;
}

int get_wlanidx_by_name(char *wlan_iface_name, int *wlan_index, int *vwlan_index)
{
	int idx=0, vidx=0;

	if(check_wlanif_valid(wlan_iface_name)==0)
		return 0;

	if(strstr(wlan_iface_name, WLAN_IF_PREFIX)){
		idx = atoi(&wlan_iface_name[4]);
		if (idx >= NUM_WLAN_INTERFACE) {
			printf("invalid wlan interface index number!\n");
			return 0;
		}

		//vap
		if(strstr(wlan_iface_name, VAP_IF_SUFFIX)){
			vidx = atoi(&wlan_iface_name[9]);
			if (vidx >= NUM_VWLAN) {
				printf("invalid virtual wlan interface index number!\n");
				return 0;
			}
			vidx += 1;
		}

		//vxd
		if(strstr(wlan_iface_name, VXD_IF_SUFFIX)){
			vidx = NUM_VWLAN_INTERFACE;
		}

		*wlan_index = idx;
		*vwlan_index = vidx;
		return 1;
	}else
		return 0;
}

int SetWlan_idx(char * wlan_iface_name)
{
	int idx, vidx=0;

	if(check_wlanif_valid(wlan_iface_name)==0)
		return 0;

	idx = atoi(&wlan_iface_name[4]);
	if (idx >= NUM_WLAN_INTERFACE) {
		printf("invalid wlan interface index number!\n");
		return 0;
	}
	wlan_idx = idx;
	vwlan_idx = 0;

	if(strstr(wlan_iface_name, VAP_IF_SUFFIX)){
		vidx = atoi(&wlan_iface_name[9]);
		if (vidx >= NUM_VWLAN) {
			printf("invalid virtual wlan interface index number!\n");
			return 0;
		}
		vwlan_idx = vidx + 1;
	}

	if(strstr(wlan_iface_name, VXD_IF_SUFFIX)){
		vwlan_idx = NUM_VWLAN_INTERFACE;
	}

	return 1;
}

int wps_wlan_restart()
{
	pid_t pid, pid2;
	pid = fork();
	if(pid == 0)
	{
		pid2 = fork();
		if(pid2 == 0)
		{
			system("sysconf wps_wlan_restart");
			exit(0);
		}
		else if(pid2 > 0)
		{
			exit(0);
		}
	}
	else if(pid > 0)
	{
		waitpid(pid, NULL, 0);
	}
	return 0;
}

int get_client_link_status(int wlan_index, int is_vxd)
{
	int is_link=0;
	FILE *fd;
	char buf[1024]={0}, newline[1024]={0}, ifname[20]={0};

	if(is_vxd){
		snprintf(ifname, sizeof(ifname), WLAN_VXD_S, wlan_index);
	}else{
		snprintf(ifname, sizeof(ifname), WLAN_IF_S, wlan_index);
	}
	snprintf(buf, sizeof(buf), "%s status -i %s", WPAS_CLI, ifname);
	fd = popen(buf, "r");
	if(fd)
	{
		if(fgets(newline, 1024, fd) != NULL){
			if(strstr(newline, "bssid="))
			{
				is_link = 1;
				printf("[%s](%d)wlan%d_vxd is link\n",__FUNCTION__,__LINE__,wlan_index);
			}
		}
		pclose(fd);
	}
	return is_link;
}

int wl_get_button_state(int *pState)
{
	char tmpbuf;
	FILE *fp;
	char line[20];

	if ((fp = fopen(PROC_GPIO, "r")) != NULL) {
		fgets(line, sizeof(line), fp);
		if (sscanf(line, "%c", &tmpbuf)) {
			if (tmpbuf == '0')
				*pState = 0;
			else
				*pState = 1;
		}
		else
			*pState = 0;
		fclose(fp);
	}
	else
		*pState = 0;
	return 0;
}

/*
	clear global variable g_wps_state triggered by wps hw push button
*/
void clear_wps_state_global(char *ifname)
{
	int wlan_index=0;

	if(g_context.wps_state & WPS_STATE_PROCESS_ALL)
		g_context.wps_state &= (~WPS_STATE_PROCESS_ALL);
	else{
		if(strstr(ifname, WLAN_IF_PREFIX)){
			wlan_index = atoi(&ifname[4]);
			if(wlan_index == WPS_BTN_WLAN0){
				if(g_context.wps_state & WPS_STATE_PROCESS_WLAN0)
					g_context.wps_state &= (~WPS_STATE_PROCESS_WLAN0);
			}else if(wlan_index == WPS_BTN_WLAN1){
				if(g_context.wps_state & WPS_STATE_PROCESS_WLAN1)
					g_context.wps_state &= (~WPS_STATE_PROCESS_WLAN1);
			}
		}else
			printf("invalid ifname %s\n", ifname);
	}
	return;
}

#ifdef WLAN_WPS_HAPD
int rtk_wlan_config_wps_setting(int wlan_index, int vwlan_index, WPS_CONF_SETTING conf)
{
	char ifname[IFNAMSIZ]={0};
	int ret = 0, i;
	WPS_CONF_SETTING conf1;

	apmib_save_wlanIdx();
	if(vwlan_index == 0)
		snprintf(ifname, sizeof(ifname), WLAN_IF_S, wlan_index);
	else if(vwlan_index == NUM_VWLAN_INTERFACE)
		snprintf(ifname, sizeof(ifname), WLAN_VXD_S, wlan_index);

	SetWlan_idx(ifname);

	memset(&conf1, 0, sizeof(WPS_CONF_SETTING));
	apmib_get(MIB_WLAN_SSID, (void *)conf1.ssid);
	apmib_get(MIB_WLAN_ENCRYPT, (void *)&conf1.encrypt);
	apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&conf1.authType);
	apmib_get(MIB_WLAN_WPA_AUTH, (void *)&conf1.wpaAuth);
#ifdef CONFIG_IEEE80211W
	apmib_get(MIB_WLAN_SHA256_ENABLE, (void *)&conf1.sha256);
	apmib_get(MIB_WLAN_IEEE80211W, (void *)&conf1.dotIEEE80211W);
#endif
	apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&conf1.unicastCipher);
	apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&conf1.wpa2UnicastCipher);
	apmib_get(MIB_WLAN_PSK_FORMAT, (void *)&conf1.wpaPSKFormat);
	apmib_get(MIB_WLAN_WPA_PSK, (void *)&conf1.wpaPSK);
	apmib_get(MIB_WLAN_WSC_ENC, (void *)&conf1.wsc_enc);
	apmib_get(MIB_WLAN_WSC_AUTH, (void *)&conf1.wsc_auth);
	apmib_get(MIB_WLAN_WSC_PSK, (void *)&conf1.wscPsk);
	apmib_get(MIB_WLAN_WSC_CONFIGURED, (void *)&conf1.wsc_configured);
	if(memcmp(&conf1, &conf, sizeof(WPS_CONF_SETTING)))
		ret = 1;
	else
		ret = 0;

	if(ret){
		apmib_set(MIB_WLAN_SSID, (void *)conf.ssid);
		if(vwlan_index == NUM_VWLAN_INTERFACE){
			if(wlan_index==0)
				apmib_set(MIB_REPEATER_SSID1, (void *)conf.ssid);
			else if(wlan_index==1)
				apmib_set(MIB_REPEATER_SSID2, (void *)conf.ssid);
		}
		apmib_set(MIB_WLAN_ENCRYPT, (void *)&conf.encrypt);
		if(conf.encrypt == ENCRYPT_WEP)
			printf("wps configure %s via WEP\n", ifname);
		apmib_set(MIB_WLAN_AUTH_TYPE, (void *)&conf.authType);
		apmib_set(MIB_WLAN_WPA_AUTH, (void *)&conf.wpaAuth);
#ifdef CONFIG_IEEE80211W
		apmib_set(MIB_WLAN_SHA256_ENABLE, (void *)&conf.sha256);
		apmib_set(MIB_WLAN_IEEE80211W, (void *)&conf.dotIEEE80211W);
#endif
		apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&conf.unicastCipher);
		apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&conf.wpa2UnicastCipher);
		apmib_set(MIB_WLAN_PSK_FORMAT, (void *)&conf.wpaPSKFormat);
		apmib_set(MIB_WLAN_WPA_PSK, (void *)conf.wpaPSK);
		apmib_set(MIB_WLAN_WSC_ENC, (void *)&conf.wsc_enc);
		apmib_set(MIB_WLAN_WSC_AUTH, (void *)&conf.wsc_auth);
		apmib_set(MIB_WLAN_WSC_PSK, (void *)conf.wscPsk);
		apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&conf.wsc_configured);
		if(vwlan_index == 0)
		{
			for(i = 0; i<NUM_WLAN_INTERFACE; i++)
			{
				if(i != wlan_index)
				{
					snprintf(ifname, sizeof(ifname), WLAN_IF_S, i);
					SetWlan_idx(ifname);
					apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&conf.wsc_configured);
				}
			}
		}

	}

	apmib_recov_wlanIdx();
	return ret;
}

int rtk_wlan_get_wps_status(char *ifname)
{
	FILE *fp=NULL;
	char buf[1024]={0}, newline[1024]={0};
	int status = 0;
	unsigned int wlanDisabled=1, wlanMode=0, wscDisable=1;

	apmib_save_wlanIdx();
	SetWlan_idx(ifname);

	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisabled);
	apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&wscDisable);
	if(wlanDisabled || wscDisable){
		goto exit;
	}

	apmib_get(MIB_WLAN_MODE, (void *)&wlanMode);
	if(wlanMode == AP_MODE){
		snprintf(buf, sizeof(buf), "%s wps_get_status -i %s", HOSTAPD_CLI, ifname);
		fp = popen(buf, "r");
	}else if(wlanMode == CLIENT_MODE){
		/* wpa_cli does not support command to get wps status */
		fp = fopen(WPAS_WPS_STAS, "r");
	}
	if(fp)
	{
		if(fgets(newline, 1024, fp) != NULL){
			sscanf(newline, "%*[^:]:%s", buf);
			//printf("sw_commit %d\n", version);
			printf("%s\n", buf);
		}

		if(!strcmp(buf, "Active"))
			status = 1;

		if(wlanMode == AP_MODE)
			pclose(fp);
		else if(wlanMode == CLIENT_MODE)
		{
			if(status == 1)
			{
				if(fgets(newline, 1024, fp) != NULL){
					if(fgets(newline, 1024, fp) != NULL)
					{
						sscanf(newline, "%*[^:]:%s", buf);
						//printf("sw_commit %d\n", version);
						printf("%s\n", buf);
					}
				}

				if(strcmp(buf, ifname))
					status = 0;
			}
			fclose(fp);
		}
	}

exit:
	apmib_recov_wlanIdx();
	return status;
}

int rtk_wlan_cancel_wps(char *ifname)
{
	char buf[1024]={0};
	unsigned int wlanDisabled=1, wlanMode=0, wscDisable=1;

	apmib_save_wlanIdx();
	SetWlan_idx(ifname);

	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisabled);
	apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&wscDisable);
	if(wlanDisabled || wscDisable){
		goto exit;
	}

	if(rtk_wlan_get_wps_status(ifname) == 0)
		goto exit;

	apmib_get(MIB_WLAN_MODE, (void *)&wlanMode);
	if(wlanMode == AP_MODE)
	{
		snprintf(buf, sizeof(buf), "%s wps_cancel -i %s", HOSTAPD_CLI, ifname);
		system(buf);
	}
	else if(wlanMode == CLIENT_MODE)
	{
		snprintf(buf, sizeof(buf), "%s wps_cancel -i %s", WPAS_CLI, ifname);
		system(buf);
		/* wpa_cli will not send event to systemd_action */
		snprintf(buf, sizeof(buf), "%s %s %s", SYSTEMD_ACTION, ifname, WPS_EVENT_TIMEOUT);
		system(buf);
	}

exit:
	apmib_recov_wlanIdx();
	return 0;
}

int update_wlan_wps_from_file(void)
{
	FILE *fp=NULL;
	char line[200], value[100], token[40], tmpBuf[40]={0}, *ptr=NULL;
	int set_band=0, i, tmp_wpa=0, tmp_wpa_key_mgmt=0, tmp_wsc_config=0, is_restart_wlan=0;
	int virtual_idx=0, isMibUpdate=0;
	WPS_CONF_SETTING conf;

	for(i=0; i<NUM_WLAN_INTERFACE ; i++)
	{
		virtual_idx = 0;
		if(virtual_idx == 0)
		{
			if(i == 0)
				fp = fopen(CONF_PATH_WLAN0, "r");
			else if(i == 1)
				fp = fopen(CONF_PATH_WLAN1, "r");
		}
		if(!fp){
			printf("[%s]open wlan%d hostapd conf fail\n", __FUNCTION__,i);
			continue;
		}
		if(fgets(line, sizeof(line), fp))
		{
			if(!strstr(line,WPS_CONFIGURE_START))
			{
				fclose(fp);
				continue;
			}
			is_restart_wlan = 1;
			memset(&conf, 0, sizeof(WPS_CONF_SETTING));
			while ( fgets(line, sizeof(line), fp) ) {
				if(strstr(line,WPS_CONFIGURE_END))
					break;
				ptr = (char *)get_token(line, token);
				if (ptr == NULL)
					continue;
				if (get_value(ptr, value)==0)
					continue;
				printf("%s=%s\n", token, value);
				if(!strcmp(token, "wps_state")){
					if(atoi(value) == 2)
						conf.wsc_configured = 1;
					else
						conf.wsc_configured = 0;
				}else if(!strcmp(token, "ssid")){
					snprintf(conf.ssid, MAX_SSID_LEN, "%s", value);
				}else if(!strcmp(token, "wpa")){
					tmp_wpa = atoi(value);
				}else if(!strcmp(token, "wpa_key_mgmt")){
					if(!strcmp(value, "WPA-PSK"))
					{
						tmp_wpa_key_mgmt = 1;
						conf.wpaAuth = WPA_AUTH_PSK;
					}
					else if(!strcmp(value, "WPA-PSK-SHA256"))
					{
						conf.encrypt = ENCRYPT_WPA2;
#ifdef CONFIG_IEEE80211W
						conf.sha256 = 1;
#endif
						conf.wpaAuth = WPA_AUTH_PSK;
					}
				}else if(!strcmp(token, "wpa_pairwise")){
					if(strstr(value,"TKIP") && strstr(value,"CCMP")){
						conf.unicastCipher = WPA_CIPHER_MIXED;
						conf.wpa2UnicastCipher = WPA_CIPHER_MIXED;
					}
					else if(strstr(value,"TKIP")){
						conf.unicastCipher = WPA_CIPHER_TKIP;
						conf.wpa2UnicastCipher = 0;
					}
					else if(strstr(value,"CCMP")){
						conf.unicastCipher = 0;
						conf.wpa2UnicastCipher = WPA_CIPHER_AES;
					}
				}else if(!strcmp(token, "wpa_passphrase")){
					snprintf(conf.wpaPSK, MAX_PSK_LEN+1, "%s", value);
					snprintf(conf.wscPsk, MAX_PSK_LEN+1, "%s", value);
					conf.wpaPSKFormat = 0;
				}else if(!strcmp(token, "wpa_psk")){
					snprintf(conf.wpaPSK, MAX_PSK_LEN+1, "%s", value);
					snprintf(conf.wscPsk, MAX_PSK_LEN+1, "%s", value);
					conf.wpaPSKFormat = 1;
				}else if(!strcmp(token, "auth_algs")){
					if(atoi(value) == 2)
						conf.authType = 1;
					else if(atoi(value) == 1)
						conf.authType = 2;
				}
				else
					printf("[%s %d]unkown mib:%s\n", __FUNCTION__,__LINE__, token);
			}
			fclose(fp);
			/* what about 11w (wsc_enc, wsc_auth ?) */
			if(tmp_wpa_key_mgmt == 1)
			{
				if(tmp_wpa == 3)
				{
					conf.encrypt = ENCRYPT_WPA2_MIXED;
					conf.wsc_auth = WSC_AUTH_WPA2PSKMIXED;
					conf.wsc_enc = WSC_ENCRYPT_TKIPAES;
				}
				else if(tmp_wpa == 2)
				{
					conf.encrypt = ENCRYPT_WPA2;
					conf.wsc_auth = WSC_AUTH_WPA2PSK;
					conf.wsc_enc = WSC_ENCRYPT_AES;
				}
				else if(tmp_wpa == 1)
				{
					conf.encrypt = ENCRYPT_WPA;
					conf.wsc_auth = WSC_AUTH_WPAPSK;
					conf.wsc_enc = WSC_ENCRYPT_TKIP;
				}
			}
			isMibUpdate = rtk_wlan_config_wps_setting(i, virtual_idx, conf);
		}
		else
			fclose(fp);
	}

	if(isMibUpdate)
		apmib_update(CURRENT_SETTING);

	if(is_restart_wlan == 1)
	{
		//wps_wlan_restart();
	}
	return is_restart_wlan;
}

int wlioctl_set_led(int flag)
{
	unsigned char enable=0;

	char tmpbuf[50];

	if (flag == LED_WSC_TIMEOUT || flag == LED_WSC_FAIL) { // for turnkey LED_WSC_END
		enable = WPS_END_LED_config_GPIO_number;
	}
	else if (flag == LED_WSC_START) {// for turnkey LED_WSC_START
		enable = WPS_START_LED_GPIO_number;
	}
	else if (flag == LED_PBC_OVERLAPPED){ // for turnkey LED_PBC_OVERLAPPED
		enable = WPS_PBC_overlapping_GPIO_number;
	}
	else if (flag == LED_WSC_ERROR) {
		enable = WPS_ERROR_LED_GPIO_number;
	} else if (flag == LED_WSC_SUCCESS) {
		enable = WPS_SUCCESS_LED_GPIO_number;
	}
	else if (flag == LED_WSC_NOP) {
		return 0;
	} else {
		enable = (unsigned char)flag;
	}
	snprintf(tmpbuf,sizeof(tmpbuf),"echo %x > /proc/gpio", enable);
	system(tmpbuf);

	return 0;
}

int update_wlan_client_wps_from_file(char *ifname)
{
	FILE *fp=NULL;
	int network_num=0, tmp_wpa_key_mgmt=0, tmp_wpa=0, restart_flag=0;
	char line[200]={0}, value[100], token[40], tmpBuf[40]={0}, *ptr;
	unsigned int rpt_enabled=0, wlanMode=0;
	WPS_CONF_SETTING conf;
	int wlan_index=-1, vwlan_index =-1, isMibUpdate=0;
#ifdef RTK_HAPD_MULTI_AP
	unsigned char mode=0;
#endif

	if(!ifname)
		return;

	if(get_wlanidx_by_name(ifname, &wlan_index, &vwlan_index)==0){
		printf("Err get wlan idx fail via %s\n", ifname);
		return;
	}
	if(wlan_index<0 || vwlan_index<0){
		printf("Err wrong wlan idx for %s\n", ifname);
		return;
	}

#ifdef UNIVERSAL_REPEATER
	if(wlan_index ==0)
		apmib_get(MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
	else if(wlan_index ==1)
		apmib_get(MIB_REPEATER_ENABLED2, (void *)&rpt_enabled);
#endif
	apmib_save_wlanIdx();
	SetWlan_idx(ifname);
	apmib_get(MIB_WLAN_MODE, (void *)&wlanMode);
	if(wlanMode == CLIENT_MODE || (rpt_enabled && (strstr(ifname, VXD_IF_SUFFIX))))
	{
		if(wlan_index==0){
			if(vwlan_index==0){
				fp = fopen(WPAS_CONF_PATH_WLAN0, "r");
			}else if(vwlan_index == NUM_VWLAN_INTERFACE){
				fp = fopen(WPAS_CONF_PATH_WLAN0_VXD, "r");
			}
		}else if(wlan_index==1){
			if(vwlan_index==0){
				fp = fopen(WPAS_CONF_PATH_WLAN1, "r");
			}else if(vwlan_index == NUM_VWLAN_INTERFACE){
				fp = fopen(WPAS_CONF_PATH_WLAN1_VXD, "r");
			}
		}

		if(!fp){
			printf("[%s]open %s wpas conf fail\n", __FUNCTION__, ifname);
			goto exit;
		}

		memset(&conf, 0, sizeof(WPS_CONF_SETTING));
		while (fgets(line, sizeof(line), fp))
		{
			if(strstr(line,"network={"))
				network_num++;

			if(network_num == 2)
			{
				restart_flag = 1;
				while ( fgets(line, sizeof(line), fp) )
				{
					ptr = (char *)get_token(line, token);
					if (ptr == NULL)
						continue;
					if (get_value(ptr, value)==0)
						continue;
					printf("%s=%s\n", token, value);
					if(!strcmp(token, "	ssid"))
					{
						snprintf(conf.ssid, MAX_SSID_LEN, "%s", value);
					}
					else if(!strcmp(token, "	psk"))
					{
						snprintf(conf.wpaPSK, MAX_PSK_LEN+1, "%s", value);
						snprintf(conf.wscPsk, MAX_PSK_LEN+1, "%s", value);
						conf.wpaPSKFormat = 0;
						if(strlen(conf.wpaPSK) > 63)
							conf.wpaPSKFormat = 1;
					}
					else if(!strcmp(token, "	pairwise"))
					{
						if(strstr(value,"TKIP") && strstr(value,"CCMP")){
							conf.unicastCipher = WPA_CIPHER_MIXED;
							conf.wpa2UnicastCipher = WPA_CIPHER_MIXED;
						}
						else if(strstr(value,"TKIP")){
							conf.unicastCipher = WPA_CIPHER_TKIP;
							conf.wpa2UnicastCipher = 0;
						}
						else if(strstr(value,"CCMP")){
							conf.unicastCipher = 0;
							conf.wpa2UnicastCipher = WPA_CIPHER_AES;
						}
					}
					else if(!strcmp(token, "	key_mgmt"))
					{
						if(!strcmp(value, "WPA-PSK"))
						{
							tmp_wpa_key_mgmt = 1;
							conf.wpaAuth = WPA_AUTH_PSK;
						}
						else if(!strcmp(value, "WPA-PSK-SHA256"))
						{
							conf.encrypt = ENCRYPT_WPA2;
#ifdef CONFIG_IEEE80211W
							conf.sha256 = 1;
#endif
							conf.wpaAuth = WPA_AUTH_PSK;
						}
						else if(!strcmp(value, "NONE"))
						{
							conf.encrypt = ENCRYPT_DISABLED;
							conf.wsc_auth = WSC_AUTH_OPEN;
							conf.wsc_enc = WSC_ENCRYPT_NONE;
						}
					}
					else if(!strcmp(token, "	proto"))
					{
						if(strstr(value,"WPA") && strstr(value,"RSN"))
							tmp_wpa = 3;
						else if(strstr(value,"RSN"))
							tmp_wpa = 2;
						else if(strstr(value,"WPA"))
							tmp_wpa = 1;
					}
					else if(!strcmp(token, "	auth_alg"))
					{
						if(strstr(value,"SHARED"))
							conf.authType = 1;
					}
				}
				break;
			}
		}

		fclose(fp);
		if(restart_flag)
		{
			if(tmp_wpa_key_mgmt == 1)
			{
				if(tmp_wpa == 3)
				{
					conf.encrypt = ENCRYPT_WPA2_MIXED;
					conf.wsc_auth = WSC_AUTH_WPA2PSKMIXED;
					conf.wsc_enc = WSC_ENCRYPT_TKIPAES;
				}
				else if(tmp_wpa == 2)
				{
#ifdef WLAN_WPA3 //WPA3 TODO
#ifdef RTK_HAPD_MULTI_AP
					apmib_get(MIB_MAP_CONTROLLER, (void *)&(mode));
					if(mode)
					{
						conf.encrypt = ENCRYPT_WPA2_WPA3_MIXED;
#ifdef CONFIG_IEEE80211W
						conf.dotIEEE80211W = 1;
						conf.sha256 = 0;
#endif
					}
					else
#endif
					{
						conf.encrypt = ENCRYPT_WPA2;
#ifdef CONFIG_IEEE80211W
						conf.dotIEEE80211W = 1;
						conf.sha256 = 0;
#endif
					}
#else
					conf.encrypt = ENCRYPT_WPA2;
#ifdef CONFIG_IEEE80211W
					conf.dotIEEE80211W = 1;
					conf.sha256 = 0;
#endif
#endif
					conf.wsc_auth = WSC_AUTH_WPA2PSK;
					conf.wsc_enc = WSC_ENCRYPT_AES;
				}
				else if(tmp_wpa == 1)
				{
					conf.encrypt = ENCRYPT_WPA;
					conf.wsc_auth = WSC_AUTH_WPAPSK;
					conf.wsc_enc = WSC_ENCRYPT_TKIP;
				}
			}

			conf.wsc_configured = 1;
			isMibUpdate = rtk_wlan_config_wps_setting(wlan_index, vwlan_index, conf);
			if(isMibUpdate){
				apmib_update(CURRENT_SETTING);
			}
		}
	}

exit:
	apmib_recov_wlanIdx();
	return restart_flag;
}

void store_wpas_wps_status(char *event_str, char *ifname)
{
	FILE *wpas_wps_stas;
	unsigned int wlanMode=0;

	if(!event_str || !ifname)
		return;

	apmib_save_wlanIdx();
	SetWlan_idx(ifname);
	apmib_get(MIB_WLAN_MODE, (void *)&wlanMode);
	if(wlanMode == CLIENT_MODE)
	{
		wpas_wps_stas = fopen(WPAS_WPS_STAS, "w+");
		if(wpas_wps_stas == NULL){
			printf("!! Error,  [%s][%d] wpas_wps_stas = NULL \n", __FUNCTION__, __LINE__);
			goto exit;
		}

		if(!strcmp(event_str, WPS_EVENT_ACTIVE))
		{
			fprintf(wpas_wps_stas, "%s", PBC_STATUS_ACTIVE);
			fprintf(wpas_wps_stas, "%s", LAST_WPS_RES_NONE);
		}
		else if(!strcmp(event_str, WPS_EVENT_TIMEOUT))
		{
			fprintf(wpas_wps_stas, "%s", PBC_STATUS_TIMEOUT);
			fprintf(wpas_wps_stas, "%s", LAST_WPS_RES_NONE);
		}
		else if(!strcmp(event_str, WPS_EVENT_OVERLAP))
		{
			fprintf(wpas_wps_stas, "%s", PBC_STATUS_OVERLAP);
			fprintf(wpas_wps_stas, "%s", LAST_WPS_RES_NONE);
		}
		else if(!strcmp(event_str, WPS_EVENT_FAIL))
		{
			fprintf(wpas_wps_stas, "%s", PBC_STATUS_DISABLED);
			fprintf(wpas_wps_stas, "%s", LAST_WPS_RES_FAILED);
		}
		else if(!strcmp(event_str, WPS_EVENT_SUCCESS))
		{
			fprintf(wpas_wps_stas, "%s", PBC_STATUS_DISABLED);
			fprintf(wpas_wps_stas, "%s", LAST_WPS_RES_SUCCESS);
		}
		else
		{
			fprintf(wpas_wps_stas, "%s", PBC_STATUS_DISABLED);
			fprintf(wpas_wps_stas, "%s", LAST_WPS_RES_NONE);
		}

		fprintf(wpas_wps_stas, "%s %s\n", "Interface:", ifname);
		fclose(wpas_wps_stas);
	}

exit:
	apmib_recov_wlanIdx();
	return;
}
void rtk_wlan_wps_pushbtn_trigger_oneband(int wlan_index, int is_vxd_wps)
{
	unsigned int wlanDisabled=1, wscDisabled=1, wlanMode=0;
	char cmd[100]={0}, ifname[20]={0};

	apmib_save_wlanIdx();

	if(is_vxd_wps){
		snprintf(ifname, sizeof(ifname), WLAN_VXD_S, wlan_index);
		vwlan_idx = NUM_VWLAN_INTERFACE;
	}else{
		snprintf(ifname, sizeof(ifname), WLAN_IF_S, wlan_index);
		vwlan_idx = 0;
	}
	wlan_idx = wlan_index;
	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisabled);
	apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&wscDisabled);
	apmib_get(MIB_WLAN_MODE, (void *)&wlanMode);
	if(!wlanDisabled && !wscDisabled){
		if(wlanMode == AP_MODE){
			snprintf(cmd, sizeof(cmd), "%s wps_pbc -i %s", HOSTAPD_CLI, ifname);
		}else if(wlanMode == CLIENT_MODE){
			snprintf(cmd, sizeof(cmd), "%s wps_pbc -i %s", WPAS_CLI, ifname);
		}
		printf("ifname:%s, cmd:%s\n", ifname, cmd);
		system(cmd);
	}

	apmib_recov_wlanIdx();
}

/*
	rtk_wlan_wps_button_repeater:
	for dual band wps:
	(1)if any vxd interface connected, then do wps on root interface
	(2)if all vxd not connected and vxd enable wsc, then do wps on vxd interface

	for single band wps:
	(1)if the specified band vxd interface connected, then do wps on root interface
	(2)if the specified band vxd interface not connected and enable wsc, then do wps on the vxd interface
*/
void rtk_wlan_wps_button_repeater(int wlan_index)
{
	int i=0;
	int is_vxd_link=0, vxd_down_num=0, vxd_wps_dis_num=0;
	unsigned int wlanDisabled=1, wsc_disabled=1, wlanDisabledRoot=1;
	char ifname[20]={0}, cmd[100]={0};
	int max_wlan_num = NUM_WLAN_INTERFACE;

	apmib_save_wlanIdx();

	//dual band wps
	if(wlan_index == WPS_BTN_ALL)
	{
		for(i = 0; i< NUM_WLAN_INTERFACE; i++)
		{
			wlan_idx = i;
			vwlan_idx = NUM_VWLAN_INTERFACE;
			apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisabled);
			apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&wsc_disabled);
			vwlan_idx = 0;
			apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisabledRoot);

			if(wlanDisabledRoot){
				max_wlan_num -= 1;
				continue;
			}
			if( wlanDisabled){
				vxd_down_num++;
				continue;
			}
			if(wsc_disabled){
				vxd_wps_dis_num++;
				continue;
			}
			if(get_client_link_status(i, 1))
				is_vxd_link = 1;
		}
		if((vxd_down_num != max_wlan_num) && (vxd_wps_dis_num != max_wlan_num) && !is_vxd_link)
		{
			/* trigger wps on vxd interface */
			for(i = 0; i< NUM_WLAN_INTERFACE; i++)
			{
				rtk_wlan_wps_pushbtn_trigger_oneband(i, 1);
			}
		}
		else
		{
			/* trigger wps on root interface */
			for(i = 0; i< NUM_WLAN_INTERFACE; i++)
			{
				rtk_wlan_wps_pushbtn_trigger_oneband(i, 0);
			}
		}
	}
	else if(wlan_index>=WPS_BTN_WLAN0 && wlan_index<=WPS_BTN_WLAN1) //single band wps
	{
		wlan_idx = wlan_index;
		vwlan_idx = NUM_VWLAN_INTERFACE;
		apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisabled);
		apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&wsc_disabled);
		if(!wlanDisabled && !wsc_disabled && !get_client_link_status(wlan_index, 1))
		{
			/* trigger wps on vxd interface */
			snprintf(ifname, sizeof(ifname), WLAN_VXD_S, wlan_index);
			snprintf(cmd, sizeof(cmd), "%s wps_pbc -i %s", WPAS_CLI, ifname);
			printf("ifname:%s, cmd:%s\n", ifname, cmd);
			system(cmd);
		}
		else
		{
			/* trigger wps on root interface */
			rtk_wlan_wps_pushbtn_trigger_oneband(wlan_index, 0);
		}
	}
	apmib_recov_wlanIdx();
}
void rtk_wlan_wps_pushbtn_trigger(int wlan_index, int is_vxd_support)
{
	int i=0;

	if(is_vxd_support)
	{
		rtk_wlan_wps_button_repeater(wlan_index);
	}
	else /* just care about wps trigger on root interface */
	{
		if(wlan_index == WPS_BTN_ALL) //dual band wps
		{
			for(i = 0; i< NUM_WLAN_INTERFACE; i++)
			{
				rtk_wlan_wps_pushbtn_trigger_oneband(i, 0);
			}
		}
		else if(wlan_index>=WPS_BTN_WLAN0 && wlan_index<=WPS_BTN_WLAN1) //single band wps
		{
			rtk_wlan_wps_pushbtn_trigger_oneband(wlan_index, 0);
		}
	}
}

int rtk_get_wps_support(int *is_not_dualband, int *is_support_vxd)
{
	FILE *fp;
	char line[200], token[100], value[100], *ptr;

	*is_not_dualband = 0;
	*is_support_vxd = 0;
	fp = fopen(WPS_ETC_CONF, "r");
	if(fp != NULL)
	{
		while ( fgets(line, sizeof(line), fp) ) {
			if(strstr(line, "#end"))
				break;
			if (line[0] == '#')
				continue;
			ptr = get_token(line, token);
			if (ptr == NULL)
				continue;
			if (get_value(ptr, value)==0) {
				continue;
			}
			if (!strcmp(token, "hw_button_trigger_one_interface")) {
				*is_not_dualband = atoi(value);
			}
			else if (!strcmp(token, "hw_button_support_repeater")) {
				*is_support_vxd = atoi(value);
			}
			else
				printf("[%s %d]unkown config:%s\n", __FUNCTION__,__LINE__, token);
		}
		fclose(fp);
	}
	else
	{
#ifdef WLAN_WPS_PUSHBUTTON_TRIGGER_ONE_INTERFACE
		*is_not_dualband = 1;
#else
		*is_not_dualband = 0;
#endif
	}

	return 0;
}

void wps_pushbtn_timeout_handler(int sig)
{
	int state=0, is_not_dualband=0, is_support_vxd=1;

	is_not_dualband = g_context.is_not_dualband;
	is_support_vxd = g_context.is_support_vxd;
	wl_get_button_state(&state);
	if(state > 0)
		g_context.pb_pressed_time++;
	else
		g_context.pb_pressed_time = 0;

	if(is_not_dualband){
		if ((g_context.pb_pressed_time >= WPS_WLAN0_START_TIME && g_context.pb_pressed_time < WPS_WLAN1_START_TIME)
			&& !(g_context.wps_state & WPS_STATE_PROCESS_WLAN0)){
			printf("WPS button for wlan0\n");
			g_context.wps_state |= WPS_STATE_PROCESS_WLAN0;
			g_context.pb_pressed_time = 0;
			rtk_wlan_wps_pushbtn_trigger(WPS_BTN_WLAN0, is_support_vxd);
		}else if(((g_context.pb_pressed_time >= WPS_WLAN1_START_TIME) && (g_context.wps_state == WPS_STATE_NONE))
			&& !(g_context.wps_state & WPS_STATE_PROCESS_WLAN1)){
			printf("WPS button for wlan1\n");
			g_context.wps_state |= WPS_STATE_PROCESS_WLAN1;
			g_context.pb_pressed_time = 0;
			rtk_wlan_wps_pushbtn_trigger(WPS_BTN_WLAN1, is_support_vxd);
		}
	}else{
		if((g_context.pb_pressed_time == WPS_BOTH_START_TIME) && !(g_context.wps_state & WPS_STATE_PROCESS_ALL)){
			printf("WPS button for all band\n");
			g_context.wps_state |= WPS_STATE_PROCESS_ALL;
			rtk_wlan_wps_pushbtn_trigger(WPS_BTN_ALL, is_support_vxd);
			g_context.pb_pressed_time = 0;
		}
	}
	alarm(1);

	return;
}
#endif

#ifdef CONFIG_APP_HOSTAPD
static int restart_wlan=0;
void process_hostapd_event(char *str)
{
	int mac_str[6];
	uint8_t mac_tmp[6];
	int i=0;
	char ifname[IFNAMSIZ]={0}, ifname2[IFNAMSIZ]={0};
	char event_str[256]={0};
	char tmpbuf[200];
	unsigned int wlanMode=0, wlanMode1=0, wscDisable=1;
#if defined(CONFIG_RTL_MULTI_PHY_ETH_WAN) || defined(CONFIG_USER_CTCAPD)
	unsigned char macstr_tmp[18];
#endif
#if defined(CONFIG_RTL_MULTI_PHY_ETH_WAN)
	if(sscanf(str, "%s %s %s", ifname, event_str, macstr_tmp) == 3)
#else
	if(sscanf(str, "%s %s", ifname, event_str) == 2)
#endif
	{
#ifdef WLAN_WPS_HAPD // WPS
		if(!strcmp(event_str, WPS_EVENT_NEW_AP_SETTINGS)) /* unconfigure -> configure mode */
		{
			restart_wlan = update_wlan_wps_from_file();
		}
		else if(!strcmp(event_str, WPS_EVENT_ACTIVE)) /* ap/client start pbc */
		{
			wlioctl_set_led(LED_WSC_START);
			store_wpas_wps_status(event_str, ifname);
		}
		else if(!strcmp(event_str, WPS_EVENT_SUCCESS) || !strcmp(event_str, WPS_EVENT_REG_SUCCESS))
		{
			wlioctl_set_led(LED_WSC_SUCCESS);
			store_wpas_wps_status(event_str, ifname);
			restart_wlan = update_wlan_client_wps_from_file(ifname);
			clear_wps_state_global(ifname);

			apmib_save_wlanIdx();
			SetWlan_idx(ifname);
			if(is_root_interface(ifname)){
				apmib_get(MIB_WLAN_MODE, (void *)&wlanMode);
				if(wlanMode==AP_MODE){
					/* for ap mode, if other band is doing wps, need cancel wps */
					for(i=0; i<NUM_WLAN_INTERFACE; i++)
					{
						memset(ifname2,0,sizeof(ifname2));
						snprintf(ifname2, sizeof(ifname2), WLAN_IF_S, i);
						if(!memcmp(ifname2,ifname,sizeof(ifname2)))
							continue;
						SetWlan_idx(ifname2);
						apmib_get(MIB_WLAN_MODE, (void *)&wlanMode1);
						apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&wscDisable);
						if((wlanMode1==AP_MODE) && (!wscDisable)){
							rtk_wlan_cancel_wps(ifname2);
						}
					}
				}
			}
			apmib_recov_wlanIdx();
			if(restart_wlan){
				restart_wlan = 0;
				wps_wlan_restart();
			}
		}
		else if(!strcmp(event_str, WPS_EVENT_OVERLAP))
		{
			wlioctl_set_led(LED_PBC_OVERLAPPED);
			store_wpas_wps_status(event_str, ifname);
			clear_wps_state_global(ifname);
		}
		else if(!strcmp(event_str, WPS_EVENT_TIMEOUT))
		{
			wlioctl_set_led(LED_WSC_TIMEOUT);
			store_wpas_wps_status(event_str, ifname);
			clear_wps_state_global(ifname);
		}
		else if(!strcmp(event_str, WPS_EVENT_FAIL))
		{
			wlioctl_set_led(LED_WSC_FAIL);
			store_wpas_wps_status(event_str, ifname);
			clear_wps_state_global(ifname);
		}
#endif
#if defined(CONFIG_RTL_MULTI_PHY_ETH_WAN) //todo
		else if(!strcmp(event_str, AP_STA_CONNECTED))
		{
			if(rtk_get_dynamic_load_balance_enable() != 0)
			{
				rtk_set_load_balance_mac_iptables(ifname, macstr_tmp, 1);
			}
		}
		else if(!strcmp(event_str, AP_STA_DISCONNECTED))
		{
			if(rtk_get_dynamic_load_balance_enable() != 0)
			{
				rtk_set_load_balance_mac_iptables(ifname, macstr_tmp, 0);
			}
		}
#endif
		if(!strcmp(event_str, STA_CONNECTED))
		{
#ifdef ARP_LOOP_DETECTION //todo
			if(rtk_wlan_is_vxd_ifname(ifname))
			{
				//printf("[===%s %d]===,link change on :%s,send SIGUSER1 to map_checker\n",__FUNCTION__,__LINE__,ifname);
				link_change_notify_to_map_checker(ifname);
			}
#endif
		}
		else if(!strcmp(event_str, AP_STA_POSSIBLE_PSK_MISMATCH))
		{
#ifdef CONFIG_USER_CTCAPD //todo
			if(sscanf(str, "%s %s %s", ifname, event_str, macstr_tmp) == 3)
			{
				convertMacFormat(macstr_tmp, mac_tmp);
				snprintf(tmpbuf,sizeof(tmpbuf),"ubus send \"passworderror\" \'{\"stamac\": \"%02X%02X%02X%02X%02X%02X\"}\'",
					mac_tmp[0],mac_tmp[1],mac_tmp[2],mac_tmp[3],mac_tmp[4],mac_tmp[5]);
				system(tmpbuf);
			}
#endif
		}
	}
}
#endif

int main(int argc, char *argv[])
{
	fd_set rfds;
	int retval;
#ifdef CONFIG_APP_HOSTAPD
	int hostapd_sockfd, hostapd_stat;
	char event_buf[256]={0};
#endif
	int is_not_dualband=0, is_support_vxd=1;

	setpriority(PRIO_PROCESS, getpid(), -17);

#if defined(CONFIG_APP_HOSTAPD)
	hostapd_stat = init_hostapd_action_socket(&hostapd_sockfd);
#endif

	if (apmib_init() == 0){
		printf("Initialize AP MIB failed!%s:%d\n",__FILE__,__LINE__);
		return;
	}

	memset(&g_context, 0, sizeof(CONTEXT_S));
	rtk_get_wps_support(&is_not_dualband, &is_support_vxd);
	g_context.is_not_dualband = is_not_dualband;
	g_context.is_support_vxd = is_support_vxd;

#ifdef WPS_HW_PUSHBTN_SUPPORT
	signal(SIGALRM, wps_pushbtn_timeout_handler);
	alarm(1);
#endif

	while(1) {
		FD_ZERO(&rfds);
#if defined(CONFIG_APP_HOSTAPD)
		if(hostapd_stat == 0){
			FD_CLR(hostapd_sockfd, &rfds);
			FD_SET(hostapd_sockfd, &rfds);
		}
#endif
		retval = select (FD_SETSIZE, &rfds, NULL, NULL, NULL);
		if (retval == -1) {
			if (errno != EINTR)
				fprintf(stderr, "[%s %d] Error select(): %s\n", __func__, __LINE__, strerror(errno));
		}
		else if (retval) {
#if defined(CONFIG_APP_HOSTAPD)
			if((hostapd_stat == 0) && retval > 0 && FD_ISSET(hostapd_sockfd, &rfds)){
				recv_hostapd_action(hostapd_sockfd, event_buf, sizeof(event_buf));
				process_hostapd_event(event_buf);
			}
#endif
		}
	}

#if defined(CONFIG_APP_HOSTAPD)
	if(hostapd_stat == 0)
		exit_hostapd_action_socket(hostapd_sockfd);
#endif

	return 0;
}
