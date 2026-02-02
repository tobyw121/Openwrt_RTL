#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "../apmib/apmib.h"
#include "../src/utility.h"

#include "wlan_manager_conf.h"

struct wlan_manager_obj	wlan_manager_conf_table[] =
{
	{"band_steering_enable",				wlan_mgr_get_band_steering_enable},
	{"prefer_band",							wlan_mgr_get_prefer_band},
	{"bss_tm_req_th",						wlan_mgr_get_bss_tm_req_th},
	{"bss_tm_req_disassoc_imminent",		wlan_mgr_get_bss_tm_req_disassoc_imminent},
	{"bss_tm_req_disassoc_timer",			wlan_mgr_get_bss_tm_req_disassoc_timer},
	{"roam_detect_th",						wlan_mgr_get_roam_detect_th},
	{"roam_sta_tp_th",						wlan_mgr_get_roam_sta_tp_th},
	{"roam_ch_clm_th",						wlan_mgr_get_roam_ch_clm_th},
	{"prefer_band_rssi_high",				wlan_mgr_get_prefer_band_rssi_high},
	{"prefer_band_rssi_low",				wlan_mgr_get_prefer_band_rssi_low},
	{"band_steering_end",					wlan_mgr_get_band_steering_end},
	/* crossband repeater start */
#if defined(RTK_CROSSBAND_REPEATER)
	{"crossband_enable",					wlan_mgr_get_crossband_enable},
	{"wlan0_ifname",						wlan_mgr_get_wlan0_ifname},
	{"wlan0_phyband_select",				wlan_mgr_get_wlan0_phyband_select},
	{"wlan0_ax_support",					wlan_mgr_get_wlan0_ax_support},
	{"wlan0_rssi_thres",					wlan_mgr_get_wlan0_rssi_thres},
	{"wlan0_cu_thres",						wlan_mgr_get_wlan0_cu_thres},
	{"wlan0_noise_thres",					wlan_mgr_get_wlan0_noise_thres},
	{"wlan0_rssi_weight",					wlan_mgr_get_wlan0_rssi_weight},
	{"wlan0_cu_weight",						wlan_mgr_get_wlan0_cu_weight},
	{"wlan0_noise_weight",					wlan_mgr_get_wlan0_noise_weight},
	{"wlan1_ifname",						wlan_mgr_get_wlan1_ifname},
	{"wlan1_phyband_select",				wlan_mgr_get_wlan1_phyband_select},
	{"wlan1_ax_support",					wlan_mgr_get_wlan1_ax_support},
	{"wlan1_rssi_thres",					wlan_mgr_get_wlan1_rssi_thres},
	{"wlan1_cu_thres",						wlan_mgr_get_wlan1_cu_thres},
	{"wlan1_noise_thres",					wlan_mgr_get_wlan1_noise_thres},
	{"wlan1_rssi_weight",					wlan_mgr_get_wlan1_rssi_weight},
	{"wlan1_cu_weight",						wlan_mgr_get_wlan1_cu_weight},
	{"wlan1_noise_weight",					wlan_mgr_get_wlan1_noise_weight},
#endif
	/* crossband repeater end */
};

#if defined(RTK_CROSSBAND_REPEATER)
static void wlan_mgr_get_mib(unsigned int mibID, int wlan_index, int vwlan_index, void *value)
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

int wlan_mgr_get_wlan0_ifname(char *name, char *result)
{
	int len = 0;

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, WLAN_IF_S[0]);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan0_phyband_select(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_PHY_BAND_SELECT, 0, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan0_ax_support(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_AX_SUPPORT, 0, 0, (void *)&vChar);

	if(vChar==0){
		len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "0");
	}else if(vChar==1){
		len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "1");
	}
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_crossband_enable(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_ENABLE, 0, 0, (void *)&vChar);

	if(vChar==0){
		len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "0");
	}else if(vChar==1){
		len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "1");
	}
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan0_rssi_thres(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_RSSI_THRESHOLD, 0, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan0_cu_thres(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_CU_THRESHOLD, 0, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan0_noise_thres(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_NOISE_THRESHOLD, 0, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan0_rssi_weight(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_RSSI_WEIGHT, 0, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan0_cu_weight(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_CU_WEIGHT, 0, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan0_noise_weight(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_NOISE_WEIGHT, 0, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan1_ifname(char *name, char *result)
{
	int len = 0;

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, WLAN_IF_S[1]);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan1_phyband_select(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_PHY_BAND_SELECT, 1, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan1_ax_support(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_AX_SUPPORT, 1, 0, (void *)&vChar);

	if(vChar==0){
		len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "0");
	}else if(vChar==1){
		len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "1");
	}
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan1_rssi_thres(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_RSSI_THRESHOLD, 1, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan1_cu_thres(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_CU_THRESHOLD, 1, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan1_noise_thres(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_NOISE_THRESHOLD, 1, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan1_rssi_weight(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_RSSI_WEIGHT, 1, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan1_cu_weight(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_CU_WEIGHT, 1, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_wlan1_noise_weight(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;

	wlan_mgr_get_mib(MIB_WLAN_CROSSBAND_NOISE_WEIGHT, 1, 0, (void *)&vChar);

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%d\n", name, vChar);

	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}
#endif

int wlan_mgr_get_band_steering_enable(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;
	char buff[100]={0};

#ifdef BAND_STEERING_SUPPORT
	apmib_get(MIB_WIFI_STA_CONTROL_ENABLE, (void *)&vChar);
#endif
	if(vChar==1){
		snprintf(buff, sizeof(buff), "%s", "1");
	}
	else{
		snprintf(buff, sizeof(buff), "%s", "0");
	}
	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, buff);
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_prefer_band(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=0;
	char buff[100]={0};

#ifdef BAND_STEERING_SUPPORT
	apmib_get(MIB_PREFER_BAND, (void *)&vChar);
#endif
	if(vChar==0){
		snprintf(buff, sizeof(buff), "%s", "5G");
	}else if(vChar==1){
		snprintf(buff, sizeof(buff), "%s", "24G");
	}
	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, buff);
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_bss_tm_req_th(char *name, char *result)
{
	int len = 0;

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "2");
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_bss_tm_req_disassoc_imminent(char *name, char *result)
{
	int len = 0;

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "1");
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_bss_tm_req_disassoc_timer(char *name, char *result)
{
	int len = 0;

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "100");
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_roam_detect_th(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=10;
	char buff[100]={0};
#ifdef BAND_STEERING_SUPPORT
	apmib_get(MIB_STACTRL_ROAM_DETECT_TH, (void *)&vChar);
#endif
	snprintf(buff, sizeof(buff), "%d", vChar);
	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, buff);
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_roam_sta_tp_th(char *name, char *result)
{
	int len = 0;

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "0");
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_roam_ch_clm_th(char *name, char *result)
{
	int len = 0;

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, "0");
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_prefer_band_rssi_high(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=40;
	char buff[100]={0};
#ifdef BAND_STEERING_SUPPORT
	apmib_get(MIB_PREFER_BAND_RSSI_HIGH, (void *)&vChar);
#endif
	snprintf(buff, sizeof(buff), "%d", vChar);
	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, buff);
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_prefer_band_rssi_low(char *name, char *result)
{
	int len = 0;
	unsigned char vChar=20;
	char buff[100]={0};
#ifdef BAND_STEERING_SUPPORT
	apmib_get(MIB_PREFER_BAND_RSSI_LOW, (void *)&vChar);
#endif
	snprintf(buff, sizeof(buff), "%d", vChar);
	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s=%s\n", name, buff);
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

int wlan_mgr_get_band_steering_end(char *name, char *result)
{
	int len = 0;

	len = snprintf(result, WLAN_MANAGER_CONF_RESULT_MAX_LEN, "%s\n", name);
	if(len < 0 || len >= WLAN_MANAGER_CONF_RESULT_MAX_LEN)
		return WLAN_MGR_GET_FAIL;

	return WLAN_MGR_GET_OK;
}

void set_wlan_mgr_cfg(FILE *wlan_manager_cfg)
{
	int idx;
	for(idx = 0; idx < ARRAY_SIZE(wlan_manager_conf_table); idx++){
		char result[WLAN_MANAGER_CONF_RESULT_MAX_LEN] = {0};

		if (wlan_manager_conf_table[idx].fun(wlan_manager_conf_table[idx].name, result) == WLAN_MGR_GET_OK){
			fprintf(wlan_manager_cfg, "%s", result);
		}
	}
}

void gen_wlan_mgr_cfg(void)
{
	FILE *wlan_manager_cfg;

	printf("%s \n",__FUNCTION__);
	wlan_manager_cfg = fopen(CONF_PATH_WLAN_MANAGER, "w+");
	if(wlan_manager_cfg == NULL){
		printf("!! Error,  [%s][%d] conf = NULL \n", __FUNCTION__, __LINE__);
		return;
	}
	set_wlan_mgr_cfg(wlan_manager_cfg);

	fclose(wlan_manager_cfg);
	sleep(2);
}

int rtk_start_wlan_manager(void)
{
	char tmp_cmd[80] = {0};
	unsigned char wm_kill_count = 0;

	gen_wlan_mgr_cfg();

	while (find_pid_by_name("wlan_manager") > 0) {
		if(wm_kill_count > 10) {
			break;
		}
		system("killall -9 wlan_manager >/dev/null 2>&1");
		usleep(100 * 1000);
		wm_kill_count++;
	}

	printf("Wlan Manager Daemon start...\n");
	snprintf(tmp_cmd, sizeof(tmp_cmd), "wlan_manager -config_fname %s &", CONF_PATH_WLAN_MANAGER);
	system(tmp_cmd);
	return 0;
}

int rtk_stop_wlan_manager(void)
{
	if (find_pid_by_name("wlan_manager") > 0) {
		/* signal(SIGTERM, (void *)&send_daemon_off_msg) in wlan_manager.c */
		system("killall -15 wlan_manager >/dev/null 2>&1");
		unlink(CONF_PATH_WLAN_MANAGER);
	}
	return 0;
}
