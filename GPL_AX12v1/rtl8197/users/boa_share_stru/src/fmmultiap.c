/*This file handles MultiAP webpage form request
 *
 */
#include <arpa/inet.h>
#include <dirent.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*-- Local inlcude files --*/
#include "boa.h"
#include "asp_page.h"
#include "apmib.h"
#include "globals.h"
#include "utility.h"
#include "apform.h"

void _set_up_backhaul_credentials()
{
	unsigned int seed       = 0;
	int          randomData = open("/dev/urandom", O_RDONLY);
	int          mibVal     = 1;
	if (randomData < 0) {
		// something went wrong, use fallback
		seed = time(NULL) + rand();
	} else {
		char    myRandomData[50];
		ssize_t result = read(randomData, myRandomData, sizeof myRandomData);
		if (result < 0) {
			// something went wrong, use fallback
			seed = time(NULL) + rand();
		}
		int i = 0;
		for (i = 0; i < 50; i++) {
			seed += (unsigned char)myRandomData[i];
			if (i % 5 == 0) {
				seed = seed * 10;
			}
		}
	}
	srand(seed);
	char SSIDDic[62]       = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
	char NetworkKeyDic[83] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890~!@#0^&*()_+{}[]:;..?";

	char backhaulSSID[21], backhaulNetworkKey[31];
	strcpy(backhaulSSID, "EasyMeshBH-");
	backhaulSSID[20]       = '\0';
	backhaulNetworkKey[30] = '\0';

	// randomly generate SSID post-fix
	int i;
	for (i = 11; i < 20; i++) {
		backhaulSSID[i] = SSIDDic[rand() % 62];
	}
	// randomly generate network key
	for (i = 0; i < 30; i++) {
		backhaulNetworkKey[i] = NetworkKeyDic[rand() % 83];
	}

	// set into mib
	if (!apmib_set(MIB_WLAN_SSID, (void *)backhaulSSID)) {
		printf("[Error] : Failed to set AP mib MIB_WLAN_SSID\n");
		return 0;
	}

	if (!apmib_set(MIB_WLAN_WPA_PSK, (void *)backhaulNetworkKey)) {
		printf("[Error] : Failed to set AP mib MIB_WLAN_WPA_PSK\n");
		return 0;
	}

	if (!apmib_set(MIB_WLAN_WSC_PSK, (void *)backhaulNetworkKey)) {
		printf("[Error] : Failed to set AP mib MIB_WLAN_WPA_PSK\n");
		return 0;
	}

	mibVal = WSC_AUTH_WPA2PSK;
	apmib_set(MIB_WLAN_WSC_AUTH, (void *)&mibVal);
	mibVal = WSC_ENCRYPT_AES;
	apmib_set(MIB_WLAN_WSC_ENC, (void *)&mibVal);
	mibVal = 1;
	apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&mibVal);
	mibVal = WPA_CIPHER_AES;
	apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&mibVal);

	mibVal = 1;
	if (!apmib_set(MIB_WLAN_HIDDEN_SSID, (void *)&mibVal)) {
		printf("[Error] : Failed to set AP mib MIB_WLAN_HIDDEN_SSID\n");
		return 0;
	}
}

void formMultiAP(request *wp, char *path, char *query)
{
	char *submitUrl, *strVal;

	//Check if it is push button press, trigger push button then return.
	strVal    = req_get_cstream_var(wp, ("start_wsc"), "");
	if (strVal[0]) {
		submitUrl = req_get_cstream_var(wp, ("submit-url"), "");
		system("echo 1 > /tmp/virtual_push_button");
		send_redirect_perm(wp, submitUrl);
		return;
	}

	int i, j;
	// Enable dot11kv if not already enabled
	int mibVal = 1;
	strVal     = req_get_cstream_var(wp, ("needEnable11kv"), "");
	if (!strcmp(strVal, "1")) {
		for (i = 0; i < 2; i++) {
			wlan_idx = i;
			for (j = 0; j < 6; j++) {
				vwlan_idx = j;
				apmib_set(MIB_WLAN_DOT11K_ENABLE, (void *)&mibVal);
				apmib_set(MIB_WLAN_DOT11V_ENABLE, (void *)&mibVal);
			}
		}
	}

	char *device_name = req_get_cstream_var(wp, ("device_name_text"), "");
	apmib_set(MIB_MAP_DEVICE_NAME, (void *)device_name);

	// mibVal = 1;
	// apmib_set(MIB_STP_ENABLED, (void *)&mibVal);

	char *role_prev = req_get_cstream_var(wp, ("role_prev"), "");

	char *controller_backhaul_link = req_get_cstream_var(wp, "controller_backhaul_radio", "");
	char *agent_backhaul_link = req_get_cstream_var(wp, "agent_backhaul_radio", "");

	// printf("Controller: %s Agent: %s\n", controller_backhaul_link, agent_backhaul_link);

	// Read role info from form and set to mib accordingly
	strVal = req_get_cstream_var(wp, ("role"), "");
	mibVal = 0;
	if (!strcmp(strVal, "controller")) {
		// Set to controller
		mibVal = 1;
		apmib_set(MIB_MAP_CONTROLLER, (void *)&mibVal);
		apmib_get(MIB_OP_MODE, (void *)&mibVal);
		if(WISP_MODE != mibVal) {
			// Disable repeater
			mibVal = 0;
			apmib_set(MIB_REPEATER_ENABLED1, (void *)&mibVal);
			apmib_set(MIB_REPEATER_ENABLED2, (void *)&mibVal);
			// Disable vxd
			mibVal    = 1;
			wlan_idx  = 0;
			vwlan_idx = 5;
			apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&mibVal);
			wlan_idx  = 1;
			vwlan_idx = 5;
			apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&mibVal);
		}

		// if different from prev role, reset this mib to 0
		if (strcmp(strVal, role_prev)) {
			mibVal = 0;
			apmib_set(MIB_MAP_CONFIGURED_BAND, (void *)&mibVal);
		}

		// enable va0
#ifdef BACKHAUL_LINK_SELECTION
		if(1 != (controller_backhaul_link[0] - '0'))
#endif
		{
			mibVal    = 0;
			wlan_idx  = 0;
			vwlan_idx = 1;
			apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&mibVal);
			mibVal = ENCRYPT_WPA2;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&mibVal);
			mibVal = WPA_AUTH_PSK;
			apmib_set(MIB_WLAN_WPA_AUTH, (void *)&mibVal);
		}
#ifdef BACKHAUL_LINK_SELECTION
		else
#endif
		{
			mibVal    = 0;
			wlan_idx  = 1;
			vwlan_idx = 1;
			apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&mibVal);
			mibVal = ENCRYPT_WPA2;
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&mibVal);
			mibVal = WPA_AUTH_PSK;
			apmib_set(MIB_WLAN_WPA_AUTH, (void *)&mibVal);
		}

		int val;
		for (i = 0; i < 2; i++) {
				wlan_idx  = i;
			for (j = 0; j < 5; j++) {
				vwlan_idx = j;
				if (!apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&val))
					return -1;
				mibVal = 0x20; // fronthaul value
				if (val == 0) // only set to fronthaul if this interface is enabled
					apmib_set(MIB_WLAN_MAP_BSS_TYPE, (void *)&mibVal);
			}

			vwlan_idx = 5;
			mibVal = 0;
			apmib_set(MIB_WLAN_MAP_BSS_TYPE, (void *)&mibVal);
		}

		// {
		// 	wlan_idx  = 0;
		// 	vwlan_idx = 0;
		// 	mibVal = 1;
		// 	apmib_set(MIB_WLAN_STACTRL_ENABLE, (void *)&mibVal);
		// 	apmib_set(MIB_WLAN_STACTRL_PREFER, (void *)&mibVal);
		// }
		// {
		// 	wlan_idx  = 1;
		// 	vwlan_idx = 0;
		// 	mibVal = 1;
		// 	apmib_set(MIB_WLAN_STACTRL_ENABLE, (void *)&mibVal);
		// 	mibVal = 0;
		// 	apmib_set(MIB_WLAN_STACTRL_PREFER, (void *)&mibVal);
		// }

		for (i = 0; i < 2; i++) {
			wlan_idx  = i;
			vwlan_idx = 1;

#ifdef BACKHAUL_LINK_SELECTION
			if(i == (controller_backhaul_link[0] - '0')) {
#endif
				char buffer[64] = {0}, buffer2[64] = {0};
				apmib_get(MIB_WLAN_SSID, (void *)buffer);
				apmib_get(MIB_WLAN_WPA_PSK, (void *)buffer2);
				if (strcmp(strVal, role_prev) || NULL == strstr(buffer, "EasyMeshBH") || 0 == strlen(buffer2)) {
					_set_up_backhaul_credentials();
				}
				mibVal = 0x40;
#ifdef BACKHAUL_LINK_SELECTION
			} else {
				mibVal = 1;
				apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&mibVal);
				mibVal = 0;
			}
#endif

			apmib_set(MIB_WLAN_MAP_BSS_TYPE, (void *)&mibVal);
		}
	} else if (!strcmp(strVal, "agent")) {
		mibVal = DHCP_CLIENT;
		apmib_set(MIB_DHCP, (void *)&mibVal);

		mibVal = 480;
		apmib_set(MIB_DHCP_LEASE_TIME, (void *)&mibVal);
		// Set to agent
		mibVal = 2;
		apmib_set(MIB_MAP_CONTROLLER, (void *)&mibVal);

		// wlan_idx  = 0;
		// vwlan_idx = 0;
		// mibVal = 1;
		// apmib_set(MIB_WLAN_STACTRL_ENABLE, (void *)&mibVal);
		// apmib_set(MIB_WLAN_STACTRL_PREFER, (void *)&mibVal);

		// wlan_idx  = 1;
		// vwlan_idx = 0;
		// mibVal = 1;
		// apmib_set(MIB_WLAN_STACTRL_ENABLE, (void *)&mibVal);
		// mibVal = 0;
		// apmib_set(MIB_WLAN_STACTRL_PREFER, (void *)&mibVal);

		// Enable vxd on 5g, set mode and enable wsc on vxd

#ifdef BACKHAUL_LINK_SELECTION
		for (i = 0; i < 2; i++) {
			wlan_idx  = i;
			vwlan_idx = 5;
			int bss_type = 0;
			if (i == (agent_backhaul_link[0] - '0')) {
				mibVal    = 0;
				bss_type  = 0x80;
			} else {
				mibVal    = 1;
				bss_type  = 0;
			}
			apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&mibVal);
			apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&mibVal);
			mibVal = 1;
			apmib_set(MIB_WLAN_MODE, (void *)&mibVal);
			apmib_set(MIB_WLAN_MAP_BSS_TYPE, (void *)&bss_type);
		}
#else
#ifdef CONFIG_BAND_2G_ON_WLAN0
		wlan_idx  = 1;
#else
		wlan_idx  = 0;
#endif
		vwlan_idx = 5;
		int bss_type = 0;
		mibVal    = 0;
		bss_type  = 0x80;
		apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&mibVal);
		apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&mibVal);
		mibVal = 1;
		apmib_set(MIB_WLAN_MODE, (void *)&mibVal);
		apmib_set(MIB_WLAN_MAP_BSS_TYPE, (void *)&bss_type);
#endif

#ifdef BACKHAUL_LINK_SELECTION
		// Enable repeater
		if (1 == (agent_backhaul_link[0] - '0')) {
			mibVal = 0;
			apmib_set(MIB_REPEATER_ENABLED1, (void *)&mibVal);
			mibVal = 1;
			apmib_set(MIB_REPEATER_ENABLED2, (void *)&mibVal);
		} else {
			mibVal = 1;
			apmib_set(MIB_REPEATER_ENABLED1, (void *)&mibVal);
			mibVal = 0;
			apmib_set(MIB_REPEATER_ENABLED2, (void *)&mibVal);
		}
#else
	#ifdef CONFIG_BAND_2G_ON_WLAN0
		mibVal = 0;
		apmib_set(MIB_REPEATER_ENABLED1, (void *)&mibVal);
		mibVal = 1;
		apmib_set(MIB_REPEATER_ENABLED2, (void *)&mibVal);
	#else
		mibVal = 1;
		apmib_set(MIB_REPEATER_ENABLED1, (void *)&mibVal);
		mibVal = 0;
		apmib_set(MIB_REPEATER_ENABLED2, (void *)&mibVal);
	#endif
#endif

		// if different from prev role, reset this mib to 0
		if (strcmp(strVal, role_prev)) {
			mibVal = 0;
			apmib_set(MIB_MAP_CONFIGURED_BAND, (void *)&mibVal);
		}
	} else if (!strcmp(strVal, "disabled")) {
		mibVal = 0;
		apmib_set(MIB_MAP_CONTROLLER, (void *)&mibVal);

		// Disable repeater
		mibVal = 0;
		apmib_set(MIB_REPEATER_ENABLED1, (void *)&mibVal);
		apmib_set(MIB_REPEATER_ENABLED2, (void *)&mibVal);
		// Disable vxd
		mibVal    = 1;
		wlan_idx  = 0;
		vwlan_idx = 5;
		apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&mibVal);
		wlan_idx  = 1;
		vwlan_idx = 5;
		apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&mibVal);
		// reset configured band to 0
		mibVal = 0;
		apmib_set(MIB_MAP_CONFIGURED_BAND, (void *)&mibVal);
		// Set all interface bss type to 0 for MAP disabled status
		mibVal = 0x00;
		for (i = 0; i < 2; i++) {
			for (j = 0; j < 6; j++) {
				wlan_idx  = i;
				vwlan_idx = j;
				apmib_set(MIB_WLAN_MAP_BSS_TYPE, (void *)&mibVal);
			}
		}
	}

	// update flash
	apmib_update(CURRENT_SETTING);

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");
	strVal    = req_get_cstream_var(wp, ("save_apply"), "");
	// sysconf init   if save_apply
	if (strVal[0]) {
		OK_MSG(submitUrl);
	} else {
		send_redirect_perm(wp, submitUrl);
	}

	return;
}

int showBackhaulSelection(request * wp, int argc, char **argv)
{
	req_format_write(wp, "<tr id=\"controller_backhaul\"> <th width=\"30%%\">Backhaul BSS:</th> <td width=\"70%%\"> \
	<input type=\"radio\" id=\"controller_backhaul_wlan0\" name=\"controller_backhaul_radio\" value=\"0\" onclick=\"isBackhaulOnChange()\">wlan0&nbsp;&nbsp; \
	<input type=\"radio\" id=\"controller_backhaul_wlan1\" name=\"controller_backhaul_radio\" value=\"1\" onclick=\"isBackhaulOnChange()\">wlan1&nbsp;&nbsp;</tr>");
	req_format_write(wp, "<tr id=\"agent_backhaul\"> <th width=\"30%%\">Backhaul STA:</th> <td width=\"70%%\"> \
	<input type=\"radio\" id=\"agent_backhaul_wlan0\" name=\"agent_backhaul_radio\" value=\"0\" onclick=\"isBackhaulOnChange()\">wlan0&nbsp;&nbsp; \
	<input type=\"radio\" id=\"agent_backhaul_wlan1\" name=\"agent_backhaul_radio\" value=\"1\" onclick=\"isBackhaulOnChange()\">wlan1&nbsp;&nbsp;</tr>");
}

#ifdef RTK_MULTI_AP_R2
void formMultiAPVLAN(request *wp, char *path, char *query)
{
	char *submitUrl, *strVal;
	int i, j;
	// Enable VLAN
	strVal = req_get_cstream_var(wp, ("enable_vlan"), "");
	int mibVal = 0;
	if (!strcmp(strVal, "ON")) {
		mibVal = 1;
		apmib_set(MIB_MAP_ENABLE_VLAN, (void *)&mibVal);
	} else if (!strcmp(strVal, "OFF")) {
		apmib_set(MIB_MAP_ENABLE_VLAN, (void *)&mibVal);
	}

	//Set Primary VID
	char *primary_vid = req_get_cstream_var(wp, ("primary_vid_text"), "");
	if(primary_vid[0])
	{
		mibVal=atoi(primary_vid);
		apmib_set(MIB_MAP_PRIMARY_VID, (void *)&mibVal);
	}

	//Set Default Secondary VID
	char *default_secondary_vid = req_get_cstream_var(wp, ("default_secondary_vid_text"), "");
	if(default_secondary_vid[0])
	{
		mibVal=atoi(default_secondary_vid);
		apmib_set(MIB_MAP_DEFAULT_SECONDARY_VID, (void *)&mibVal);
	}

	//Set WLAN_MAP_VID
	strVal = req_get_cstream_var(wp, ("vid_info"), "");
	char* token;
	for (i = 0; i < 2; i++) {
		wlan_idx = i;
		for (j = 0; j < 5; j++) {
			vwlan_idx = j;
			if (0 == i && 0 == j) {
				token = strtok(strVal, "_");
			} else {
				token = strtok(NULL, "_");
			}
			mibVal = atoi(token);
			apmib_set(MIB_WLAN_MAP_VID, (void *)&mibVal);
		}
	}

	// update flash
	apmib_update(CURRENT_SETTING);

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");
	strVal    = req_get_cstream_var(wp, ("save_apply"), "");
	// sysconf init   if save_apply
	if (strVal[0]) {
		OK_MSG(submitUrl);
	} else {
		send_redirect_perm(wp, submitUrl);
	}

	return;
}

enum {
	CHANNEL_SCAN_BAND_2G = 1,
	CHANNEL_SCAN_BAND_5G = 2
};

static int _channel_scan_band = 0;

void formChannelScan(request *wp, char *path, char *query)
{
	char *strVal, *submitUrl;
	int mibVal;

	strVal = req_get_cstream_var(wp, ("scan_band"), "");
	if (strVal[0]) {
		char cmd[64], scan_band;
		if(!strcmp(strVal, "2G")) {
			scan_band = 1;
			_channel_scan_band = CHANNEL_SCAN_BAND_2G;
		} else if(!strcmp(strVal, "5G")) {
			scan_band = 2;
			_channel_scan_band = CHANNEL_SCAN_BAND_5G;
		} else if(!strcmp(strVal, "all")) {
			scan_band = 0;
			_channel_scan_band = CHANNEL_SCAN_BAND_2G | CHANNEL_SCAN_BAND_5G;
		}

		sprintf(cmd, "echo %d > /tmp/channel_scan_band", scan_band);
		printf("%s\n", cmd);
		system(cmd);

		send_redirect_perm(wp, "/multi_ap_channel_scan_result.htm");
		return;
	}

	strVal = req_get_cstream_var(wp, ("band_select_1"), "");
	if (strVal[0]) {
		if (!strcmp(strVal, "selected")) {
			strVal = req_get_cstream_var(wp, ("channel_switch_1"), "");
			if (strVal[0]) {
				mibVal = atoi(strVal);
				wlan_idx = 1;
				vwlan_idx = 0;
				apmib_set(MIB_WLAN_CHANNEL, (void *)&mibVal);
			}
		}
	}

	strVal = req_get_cstream_var(wp, ("band_select_2"), "");
	if (strVal[0]) {
		if (!strcmp(strVal, "selected")) {
			strVal = req_get_cstream_var(wp, ("channel_switch_2"), "");
			if (strVal[0]) {
				mibVal = atoi(strVal);
				wlan_idx = 0;
				vwlan_idx = 0;
				apmib_set(MIB_WLAN_CHANNEL, (void *)&mibVal);
			}
		}
	}
	apmib_update(CURRENT_SETTING);

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");
	OK_MSG(submitUrl);
	return;
}

void get_channel_scan_results(struct channel_scan_results *results, int band)
{
	FILE *fp = NULL, *fp_nr = NULL;
	char *  line = NULL;
	size_t  len  = 0;
	ssize_t read;
	char    element[32];
	int 	val = 0, cu_weight = 0, noise_weight = 0, neighbor_weight = 100, best_score = 0, i = 0;

	if (band == CHANNEL_SCAN_BAND_2G) {
		fp_nr = fopen("/tmp/map_channel_scan_nr_2G", "r");
		fp = fopen("/tmp/map_channel_scan_report_2G", "r");
	} else if (band == CHANNEL_SCAN_BAND_5G) {
		fp_nr = fopen("/tmp/map_channel_scan_nr_5G", "r");
		fp = fopen("/tmp/map_channel_scan_report_5G", "r");
	}

	if (fp_nr && fp) {
		if (getline(&line, &len, fp_nr) != -1) {
			sscanf(line, "%s %d", element, &val);
			// printf("%s = %d\n", element, val);
			if(0 == strcmp(element, "channel_nr")) {
				results->channel_nr = val;
				results->channels = (struct channel_list*)malloc(sizeof(struct channel_list) * results->channel_nr);
			}
			free(line);
			line = NULL;

			while ((read = getline(&line, &len, fp)) != -1) {
				sscanf(line, "%s %d", element, &val);
				// printf("%s = %d\n", element, val);

				if (0 == strcmp(element, "op_class")) {
					results->channels[i].op_class = val;
				} else if (0 == strcmp(element, "channel")) {
					results->channels[i].channel = val;
				} else if (0 == strcmp(element, "scan_status"))	{
					results->channels[i].scan_status = val;
				} else if (0 == strcmp(element, "utilization")) {
					results->channels[i].utilization = val;
				} else if (0 == strcmp(element, "noise")) {
					results->channels[i].noise = val;
				} else if (0 == strcmp(element, "neighbor_nr")) {
					results->channels[i].neighbor_nr = val;
					results->channels[i].score = ((255 - results->channels[i].utilization) * 100) / 255 * cu_weight + (100 - results->channels[i].noise) * noise_weight + (100 - results->channels[i].neighbor_nr) * neighbor_weight;
					results->channels[i].score /= (cu_weight + noise_weight + neighbor_weight);
					i++;
				}
			}
			free(line);

			results->best_channel = results->channels[0].channel;
			best_score = results->channels[0].score;
			for (i = 0; i < results->channel_nr; i++) {
				if (results->channels[i].scan_status == 0 && results->channels[i].score > best_score) {
					results->best_channel = results->channels[i].channel;
					best_score = results->channels[i].score;
				}
			}
			// printf("Best Channel: %d\n", results->best_channel);
		}
	}
	if(fp_nr)
        fclose(fp_nr);
    if(fp)
		fclose(fp);
}

void show_channel_scan_results(request *wp, struct channel_scan_results *results, int *nBytesSent)
{
	int i;

	nBytesSent += req_format_write(wp, ("<br>""<table border=\"1\" width=\"680\">\n"));

	nBytesSent += req_format_write(wp, ("<tr class=\"tbl_head\">"
	"<td align=center ><font size=\"2\"><b>OP Class</b></font></td>\n"
	"<td align=center ><font size=\"2\"><b>Channel</b></font></td>\n"
	"<td align=center ><font size=\"2\"><b>Score</b></font></td>\n"));
	nBytesSent += req_format_write(wp, ("</tr>\n"));

	for (i = 0; i < results->channel_nr; i++) {
		if (results->channels[i].scan_status == 0) {
			nBytesSent += req_format_write(wp, ("<tr class=\"tbl_body\">"
			"<td align=center ><font size=\"2\">%d</td>\n"
			"<td align=center ><font size=\"2\">%d</td>\n"
			"<td align=center ><font size=\"2\">%d</td>\n"),
			results->channels[i].op_class, results->channels[i].channel, results->channels[i].score);
			nBytesSent += req_format_write(wp, ("</tr>\n"));
		}
	}

	nBytesSent += req_format_write(wp, ("</table>\n"));

}

void show_best_channel(request *wp, struct channel_scan_results *results, int *nBytesSent, int *match, int band)
{
	int channel;

	apmib_get(MIB_WLAN_CHANNEL, (void *)&channel);
	// printf("wlan_idx:%d   MIB_WLAN_CHANNEL:%d\n",wlan_idx,channel);

	if(channel == results->best_channel) {
		(*match)++;
		nBytesSent += req_format_write(wp, ("<p>"
		"<font size=\"2\"><b>Current channel %d is already the best channel</b></font>\n""</p>\n"), channel);
	} else {
		nBytesSent += req_format_write(wp, ("<tr >"
		"<td ><input type=\"hidden\" id=\"channel_switch_%d\" value=\"%d\" name=\"channel_switch_%d\"><input type=\"checkbox\" id=\"band_select_%d\" value=\"\" name=\"band_select_%d\"></td>\n"     
		"<td ><font size=\"2\"><b>Switch to best channel %d</b></font></td>\n"
		"</tr>\n"), band, results->best_channel, band, band, band, results->best_channel);
	}
}

int showChannelScanResults(request *wp, int argc, char **argv)
{
    int nBytesSent = 0;
	struct channel_scan_results results_5G = {0}, results_2G = {0};

	// printf("showChannelScanResults!\n");

	if (-1 != access("/tmp/map_channel_scan_report_5G", F_OK) && (_channel_scan_band & CHANNEL_SCAN_BAND_5G)) {
		// printf("access map_channel_scan_report_5G!\n");
		get_channel_scan_results(&results_5G, CHANNEL_SCAN_BAND_5G);
		remove("/tmp/map_channel_scan_report_5G");
		remove("/tmp/map_channel_scan_nr_5G");
	}

	if (-1 != access("/tmp/map_channel_scan_report_2G", F_OK) && (_channel_scan_band & CHANNEL_SCAN_BAND_2G)) {
		// printf("access map_channel_scan_report_2G!\n");
		get_channel_scan_results(&results_2G, CHANNEL_SCAN_BAND_2G);
		remove("/tmp/map_channel_scan_report_2G");
		remove("/tmp/map_channel_scan_nr_2G");
	}

	if(results_5G.channel_nr == 0 && results_2G.channel_nr == 0) {
		nBytesSent += req_format_write(wp, ("<div id=\"reload\"><font size=2> Please wait for the scan result......</div>\n"));
		return nBytesSent;
	}

	int channel_match = 0;
	vwlan_idx = 0;
	if(results_5G.channel_nr > 0) {
		wlan_idx = 0;
		show_best_channel(wp, &results_5G, &nBytesSent, &channel_match, CHANNEL_SCAN_BAND_5G);
	}

	if(results_2G.channel_nr > 0) {
		wlan_idx = 1;
		show_best_channel(wp, &results_2G, &nBytesSent, &channel_match, CHANNEL_SCAN_BAND_2G);
	}

	if((channel_match < 2 && (_channel_scan_band == (CHANNEL_SCAN_BAND_2G | CHANNEL_SCAN_BAND_5G))) ||
	  (channel_match < 1 && (_channel_scan_band == CHANNEL_SCAN_BAND_2G || _channel_scan_band == CHANNEL_SCAN_BAND_5G))) {	
		nBytesSent += req_format_write(wp, ("<p>""<input type=\"submit\" value=\"Save & Apply\" name=\"save_apply\" onClick=\"return saveChanges()\">\n""</p>"));	
	}

	if(results_5G.channel_nr > 0) {
		// printf("show_channel_scan_results 5G!\n");
		show_channel_scan_results(wp, &results_5G, &nBytesSent);
	}

	if(results_2G.channel_nr > 0) {
		// printf("show_channel_scan_results 2G!\n");
		show_channel_scan_results(wp, &results_2G, &nBytesSent);
	}

	if(results_5G.channels) {
		free(results_5G.channels);
	}
	if(results_2G.channels) {
		free(results_2G.channels);
	}

    return nBytesSent;
}
#endif