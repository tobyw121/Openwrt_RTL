#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

#include "apmib.h"
#include "sysconf.h"
#include "set_init_wifi6.h"
#include "sys_utility.h"

const char IWPRIV[] = "/bin/iwpriv";
const char EBTABLES[] = "/bin/ebtables";
const char IPTABLES[] = "/bin/iptables";
#ifdef CONFIG_IPV6
const char IP6TABLES[] = "/bin/ip6tables";
#endif
const char FW_FORWARD[] = "FORWARD";
const char FW_INPUT[] = "INPUT";
const char FW_DROP[] = "DROP";

#ifdef CONFIG_GUEST_ACCESS_SUPPORT
const char GUEST_ACCESS_CHAIN_EB[] = "guest_access";
const char GUEST_ACCESS_CHAIN_IP[] = "guest_access";
const char GUEST_ACCESS_CHAIN_IPV6[] = "guest_access";
#endif

void rtk_wlan_get_ifname(int wlan_index, int vwlan_index, char *ifname)
{
	if(vwlan_index==0)
		snprintf(ifname, IFNAMSIZ, "%s", WLAN_IF_S[wlan_index]);
	else if (vwlan_index>=1 && vwlan_index<(NUM_VWLAN_INTERFACE))
		snprintf(ifname, IFNAMSIZ, VAP_IF, wlan_index, vwlan_index-1);
	else if (vwlan_index == NUM_VWLAN_INTERFACE) {
		snprintf(ifname, IFNAMSIZ, "%s", VXD_IF[wlan_index]);
	}
}

int read_wlan_hwmib_from(int wlan_idx)
{
	unsigned int hw_wlan0_5g, hw_5g_wlan=0;
	int ret = -1;

	if(apmib_get(MIB_HW_5G_ON_WLAN, (void *)&hw_5g_wlan)==0){
		hw_5g_wlan = 0;
		printf("mib_get MIB_HW_5G_ON_WLAN fail, hw 5g on wlan0 in default !!!!!\n");
	}

	if(hw_5g_wlan==0)
		hw_wlan0_5g = 1;
	else
		hw_wlan0_5g = 0;

#ifdef CONFIG_BAND_2G_ON_WLAN0
	if((wlan_idx==0 && hw_wlan0_5g) || (wlan_idx==1 && !hw_wlan0_5g))
		ret = 1;
	else if((wlan_idx==0 && !hw_wlan0_5g) || (wlan_idx==1 && hw_wlan0_5g))
		ret = 0;
#else
	if((wlan_idx==0 && hw_wlan0_5g) || (wlan_idx==1 && !hw_wlan0_5g))
		ret = 0;
	else if((wlan_idx==0 && !hw_wlan0_5g) || (wlan_idx==1 && hw_wlan0_5g))
		ret = 1;
#endif

#ifdef CONFIG_BAND_2G_ON_WLAN0
	printf("(%s)[hs]wlan0_5g:%d, [cs]select 2g on wlan0, read hwmib from HW_WLAN%d\n", WLAN_IF_S[wlan_idx], hw_wlan0_5g, ret);
#else
	printf("(%s)[hs]wlan0_5g:%d, [cs]select 5g on wlan0, read hwmib from HW_WLAN%d\n", WLAN_IF_S[wlan_idx], hw_wlan0_5g, ret);
#endif

	return ret;
}

#ifdef WLAN_TXPOWER_HIGH
int getTxPowerHigh(int phyband)
{
	unsigned int intVal, txpower_high=0;
	int i;

	apmib_save_wlanIdx();

	for(i=0; i<NUM_WLAN_INTERFACE; i++){
		wlan_idx = i;
		vwlan_idx = 0;
		apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal);
		if(phyband == intVal){
			apmib_get(MIB_WLAN_TX_POWER_HIGH, (void *)&intVal);
			break;
		}
	}

	apmib_recov_wlanIdx();

	return txpower_high;
}
#endif

// 100mW = 20dB, 200mW=23dB
// 80mw = 19.03, (20-19.03)*2 = 2
int getTxPowerScale(int phyband, int mode)
{
#ifdef WLAN_TXPOWER_HIGH
	if(phyband == PHYBAND_2G)
	{
		switch (mode)
		{
			case 0: //100% or 200%
				if(getTxPowerHigh(phyband))
					return -4; //200%
				return 0; //100%
			case 1: //80%
				return 2;
			case 2: //60%
				return 5;
			case 3: //35%
				return 9;
			case 4: //15%
				return 17;
		}
	}
	else{ //5G
		switch (mode)
		{
			case 0: //100% or 140%
				if(getTxPowerHigh(phyband))
					return -3; //140%
					//return -6; //200%
				return 0; //100%
			case 1: //80%
				return 2;
			case 2: //60%
				return 5;
			case 3: //35%
				return 9;
			case 4: //15%
				return 17;
		}
	}
#else
	switch (mode)
	{
		case 0: //100%
			return 0;
#if defined(CONFIG_CMCC) || defined(CONFIG_CU)
		case 1: //80%
			return 2;
		case 2: //60%
			return 4;
		case 3: //40%
			return 8;
		case 4: //20%
			return 14;
#else
		case 1: //70%
			return 3;
		case 2: //50%
			return 6;
		case 3: //35%
			return 9;
		case 4: //15%
			return 17;
#endif
	}
#endif
	return 0;
}

int do_cmd(const char *filename, char *argv [], int dowait)
{
	pid_t pid, wpid;
	int stat=0, st;

	if((pid = vfork()) == 0) {
		/* the child */
		char *env[3];

		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			stat = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&st)) != pid)
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					stat = 0;
					break;
				}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		stat = -1;
	}
	return st;
}

/*
	format: type + interface + set_mib/get_mib/flash_set + mib name + others
*/
int iwpriv_cmd(int type, ...)
{
	va_list ap;
	int k=0, i, len;
	char *s, *s2;
	char *argv[24];
	int status=0;
	unsigned char value[196];
	char parm[2048];
	int mib_id, mode, intVal, dpk_len=0, tssi_len=0;
	unsigned char dpk_value[1024]={0};

	TRACE(STA_SCRIPT, "%s ", IWPRIV);
	va_start(ap, type);

	s = va_arg(ap, char *); //wlan interface name
	argv[++k] = s;
	TRACE(STA_SCRIPT|STA_NOTAG, "%s ", s);
	s = va_arg(ap, char *); //cmd, ie set_mib
	argv[++k] = s;
	TRACE(STA_SCRIPT|STA_NOTAG, "%s ", s);
	s = va_arg(ap, char *); //cmd detail

	if(type & IWPRIV_GETMIB){
		mib_id = va_arg(ap, int);
		apmib_get(mib_id, (void *)value); //tesia
	}
	else{
		if(type & IWPRIV_HS){
			if(type & IWPRIV_HW2G) //eg:tx power
				memcpy(value, va_arg(ap, char *), MAX_CHAN_NUM);
			else if(type & IWPRIV_HWDPK)
			{
				dpk_len = va_arg(ap, int);
				memcpy(dpk_value, va_arg(ap, char *), dpk_len);
			}
			else if(type & IWPRIV_AX){
				tssi_len = va_arg(ap, int);
				memcpy(value, va_arg(ap, char *), tssi_len);
			}
			/*else
				memcpy(value, va_arg(ap, char *), MAX_5G_CHANNEL_NUM);*/
		}
	}

	if(!(type & IWPRIV_HS)){
		if(type & IWPRIV_GETMIB){
			if(type & IWPRIV_INT) //int
				snprintf(parm, sizeof(parm), "%s=%u", s, value[0]); //TODO
			else //string
				snprintf(parm, sizeof(parm), "%s=%s", s, value);
		}
		else{
			if(type & IWPRIV_INT){ //int
				intVal = va_arg(ap, int);
				snprintf(parm, sizeof(parm), "%s=%u", s, intVal);
			}
			else{ //string
				s2 = va_arg(ap, char *);
				snprintf(parm, sizeof(parm), "%s=%s", s, s2);
			}
		}
	}
	else{//IWPRIV_HS
		snprintf(parm, sizeof(parm), "%s=", s);
		if(type & IWPRIV_AX){
			for(i=0; i<tssi_len; i++) {
				len = sizeof(parm)-strlen(parm);
				if(snprintf(parm+strlen(parm), len, "%02x", value[i]) >= len){
					printf("[%s %d]warning, string truncated\n",__FUNCTION__,__LINE__);
				}
			}

		}
		else if(type & IWPRIV_HW2G){ //2G
			if(type & IWPRIV_TXPOWER){
				mode = va_arg(ap, int);
				intVal = getTxPowerScale(PHYBAND_2G, mode);
				for(i=0; i<MAX_CHAN_NUM; i++) {
					if(value[i]!=0){
						if((value[i] - intVal)>=1)
							value[i] -= intVal;
						else
							value[i] = 1;
					}
					len = sizeof(parm)-strlen(parm);
					if(snprintf(parm+strlen(parm), len, "%02x", value[i]) >= len){
						printf("[%s %d]warning, string truncated\n",__FUNCTION__,__LINE__);
					}
				}
			}
			else{
				for(i=0; i<MAX_CHAN_NUM; i++) {
					len = sizeof(parm)-strlen(parm);
					if(snprintf(parm+strlen(parm), len, "%02x", value[i]) >= len){
						printf("[%s %d]warning, string truncated\n",__FUNCTION__,__LINE__);
					}
				}
			}

		}
		else if(type & IWPRIV_HWDPK){
			for(i=0; i<dpk_len; i++) {
				len = sizeof(parm)-strlen(parm);
				if(snprintf(parm+strlen(parm), len, "%02x", dpk_value[i]) >= len){
					printf("[%s %d]warning, string truncated\n",__FUNCTION__,__LINE__);
				}
			}
		}

	}
	argv[++k] = parm;
	TRACE(STA_SCRIPT|STA_NOTAG, "%s ", argv[k]);
	TRACE(STA_SCRIPT|STA_NOTAG, "\n");
	argv[k+1] = NULL;
	status = do_cmd(IWPRIV, argv, 1);
	va_end(ap);

	return status;
}

int set_hw_mib(int ax_flag, int wlan_index)
{
	/*TBD: move to individual function*/
	unsigned char value[128];
	unsigned char hwmib_idx;
	unsigned int intVal=0;
	unsigned char wlan_interface[64] = {0};
	unsigned int hw_wlan0_5g=0, hw_5g_wlan=0;

	memset(wlan_interface, 0, sizeof(wlan_interface));
	snprintf(wlan_interface, sizeof(wlan_interface), WLAN_IF_S[wlan_index]);

	if(ax_flag == 1)
	{
		/* Get wireless name */
		if(rtk_get_interface_flag(wlan_interface, TIMER_COUNT, IS_EXIST) == 0)
		{
			printf("[%s %d] the interface (%s) is't up yet!\n", __FUNCTION__, __LINE__,wlan_interface);
			return -1;
		}

		/*
			for AX: wlan hw flash mib is not swaped,
					and read true band mib to set driver
		*/
		hwmib_idx = read_wlan_hwmib_from(wlan_index);
		if(hwmib_idx==0){
			apmib_get(MIB_HW_XCAP_WLAN0_AX, (void *)&intVal);
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "xcap", intVal);

			apmib_get(MIB_HW_RFE_TYPE_WLAN0, (void *)&intVal);
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "rfe", intVal);

			apmib_get(MIB_HW_THERMAL_A_WLAN0, (void *)&intVal);
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "thermalA", intVal);

			apmib_get(MIB_HW_THERMAL_B_WLAN0, (void *)&intVal);
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "thermalB", intVal);

			apmib_get(MIB_HW_THERMAL_C_WLAN0, (void *)&intVal);
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "thermalC", intVal);

			apmib_get(MIB_HW_THERMAL_D_WLAN0, (void *)&intVal);
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "thermalD", intVal);
		}else if(hwmib_idx==1){
			apmib_get(MIB_HW_XCAP_WLAN1_AX, (void *)&intVal);
#ifdef CONFIG_SHARE_XSTAL
			apmib_get(MIB_HW_XCAP_WLAN0_AX, (void *)&intVal);
#endif /*CONFIG_SHARE_XSTAL*/
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "xcap", intVal);

			apmib_get(MIB_HW_RFE_TYPE_WLAN1, (void *)&intVal);
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "rfe", intVal);

			apmib_get(MIB_HW_THERMAL_A_WLAN1, (void *)&intVal);
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "thermalA", intVal);

			apmib_get(MIB_HW_THERMAL_B_WLAN1, (void *)&intVal);
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "thermalB", intVal);

			apmib_get(MIB_HW_THERMAL_C_WLAN1, (void *)&intVal);
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "thermalC", intVal);

			apmib_get(MIB_HW_THERMAL_D_WLAN1, (void *)&intVal);
			iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "thermalD", intVal);
		}else
			printf("wrong wlan hw mib idx !!!\n");

		apmib_get(MIB_HW_CUSTOM_PARA_PATH, (void *)value);
		iwpriv_cmd(0, wlan_interface, "flash_set", "para_path", value);

		/* RF TSSI DE calibration */
		apmib_get(MIB_HW_2G_CCK_TSSI_A, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "2G_cck_tssi_A", MAX_2G_TSSI_CCK_SIZE_AX, value);
		apmib_get(MIB_HW_2G_CCK_TSSI_B, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "2G_cck_tssi_B", MAX_2G_TSSI_CCK_SIZE_AX, value);
		apmib_get(MIB_HW_2G_CCK_TSSI_C, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "2G_cck_tssi_C", MAX_2G_TSSI_CCK_SIZE_AX, value);
		apmib_get(MIB_HW_2G_CCK_TSSI_D, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "2G_cck_tssi_D", MAX_2G_TSSI_CCK_SIZE_AX, value);

		apmib_get(MIB_HW_2G_BW40_1S_TSSI_A, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "2G_bw40_1s_tssi_A", MAX_2G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_2G_BW40_1S_TSSI_B, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "2G_bw40_1s_tssi_B", MAX_2G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_2G_BW40_1S_TSSI_C, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "2G_bw40_1s_tssi_C", MAX_2G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_2G_BW40_1S_TSSI_D, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "2G_bw40_1s_tssi_D", MAX_2G_TSSI_BW40_SIZE_AX, value);

		apmib_get(MIB_HW_5G_BW40_1S_TSSI_A, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_bw40_1s_tssi_A", MAX_5G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_5G_BW40_1S_TSSI_B, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_bw40_1s_tssi_B", MAX_5G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_5G_BW40_1S_TSSI_C, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_bw40_1s_tssi_C", MAX_5G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_5G_BW40_1S_TSSI_D, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_bw40_1s_tssi_D", MAX_5G_TSSI_BW40_SIZE_AX, value);

		/* RF TSSI SLOPE K, GAIN DIFF */
		apmib_get(MIB_HW_5G_TSSI_GAIN_DIFF_A, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_GAIN_DIFF_A", MAX_5G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_5G_TSSI_GAIN_DIFF_B, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_GAIN_DIFF_B", MAX_5G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_5G_TSSI_GAIN_DIFF_C, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_GAIN_DIFF_C", MAX_5G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_5G_TSSI_GAIN_DIFF_D, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_GAIN_DIFF_D", MAX_5G_TSSI_BW40_SIZE_AX, value);

		/* RF TSSI SLOPE K, CW DIFF */
		apmib_get(MIB_HW_5G_TSSI_CW_DIFF_A, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_CW_DIFF_A", MAX_5G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_5G_TSSI_CW_DIFF_B, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_CW_DIFF_B", MAX_5G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_5G_TSSI_CW_DIFF_C, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_CW_DIFF_C", MAX_5G_TSSI_BW40_SIZE_AX, value);
		apmib_get(MIB_HW_5G_TSSI_CW_DIFF_D, (void *)value);
		iwpriv_cmd(IWPRIV_HS | IWPRIV_AX, wlan_interface, "flash_set", "5G_CW_DIFF_D", MAX_5G_TSSI_BW40_SIZE_AX, value);

		/* RF 2G RX GAIN K */
		apmib_get(MIB_HW_RX_GAINGAP_2G_CCK_AB, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "2G_rx_gain_cck", intVal);
		apmib_get(MIB_HW_RX_GAINGAP_2G_OFDM_AB, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "2G_rx_gain_ofdm", intVal);

		/* RF 5G RX GAIN K */
		apmib_get(MIB_HW_RX_GAINGAP_5GL_AB, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "5G_rx_gain_low", intVal);
		apmib_get(MIB_HW_RX_GAINGAP_5GM_AB, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "5G_rx_gain_mid", intVal);
		apmib_get(MIB_HW_RX_GAINGAP_5GH_AB, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "5G_rx_gain_high", intVal);

		apmib_get(MIB_HW_CHANNEL_PLAN_5G, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "chan_plan", intVal);
	}
	else
	{
		unsigned char value[1024];
		unsigned int phyband = 0, mode = 0;

		snprintf(wlan_interface, sizeof(wlan_interface), WLAN_IF_S[wlan_index]);
		if(rtk_get_interface_flag(wlan_interface, TIMER_COUNT, IS_EXIST) == 0)
		{
			printf("[%s %d] the interface (%s) is't up yet!", __FUNCTION__, __LINE__, wlan_interface);
			return -1;
		}

		apmib_save_wlanIdx();

		wlan_idx = wlan_index;
		vwlan_idx = 0;
		apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyband);
		if(phyband != PHYBAND_2G)
		{
			printf("%s(%d) wlan(%d) phyband is wrong!\n", __FUNCTION__,__LINE__, wlan_idx);
			return -1;
		}

		hwmib_idx = read_wlan_hwmib_from(wlan_index);

		wlan_idx = hwmib_idx;
		if ( apmib_get( MIB_WLAN_RFPOWER_SCALE, (void *)&mode) == 0) {
			printf("Get MIB_TX_POWER error!\n");
		}

		apmib_get(MIB_HW_TX_POWER_CCK_A, (void *)value);
		iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, wlan_interface, "set_mib", "pwrlevelCCK_A", value, mode);

		apmib_get(MIB_HW_TX_POWER_CCK_B, (void *)value);
		iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, wlan_interface, "set_mib", "pwrlevelCCK_B", value, mode);

		apmib_get(MIB_HW_TX_POWER_TSSI_CCK_A, (void *)value);
		iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, wlan_interface, "set_mib", "pwrlevel_TSSICCK_A", value, mode);

		apmib_get(MIB_HW_TX_POWER_TSSI_CCK_B, (void *)value);
		iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, wlan_interface, "set_mib", "pwrlevel_TSSICCK_B", value, mode);

		apmib_get(MIB_HW_TX_POWER_HT40_1S_A, (void *)value);
		iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, wlan_interface, "set_mib", "pwrlevelHT40_1S_A", value, mode);

		apmib_get(MIB_HW_TX_POWER_HT40_1S_B, (void *)value);
		iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, wlan_interface, "set_mib", "pwrlevelHT40_1S_B", value, mode);

		apmib_get(MIB_HW_TX_POWER_TSSI_HT40_1S_A, (void *)value);
		iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G|IWPRIV_TXPOWER, wlan_interface, "set_mib", "pwrlevel_TSSIHT40_1S_A", value, mode);

		apmib_get(MIB_HW_TX_POWER_TSSI_HT40_1S_B, (void *)value);
		iwpriv_cmd(IWPRIV_HS |IWPRIV_HW2G|IWPRIV_TXPOWER, wlan_interface, "set_mib", "pwrlevel_TSSIHT40_1S_B", value, mode);

#if 0  //TO add mib
		apmib_get(MIB_HW_TX_POWER_HT40_2S, (void *)value);
		iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, wlan_interface, "set_mib", "pwrdiffHT40_2S", value);

		apmib_get(MIB_HW_TX_POWER_HT20, (void *)value);
		iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, wlan_interface, "set_mib", "pwrdiffHT20", value);
#endif
		apmib_get(MIB_HW_TX_POWER_DIFF_OFDM, (void *)value);
		iwpriv_cmd(IWPRIV_HS|IWPRIV_HW2G, wlan_interface, "set_mib", "pwrdiffOFDM", value);

		apmib_get(MIB_HW_11N_TSSI_ENABLE, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "set_mib", "tssi_enable", intVal);

		apmib_get(MIB_HW_11N_TSSI1, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "set_mib", "tssi1", intVal);

		apmib_get(MIB_HW_11N_TSSI2, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "set_mib", "tssi2", intVal);

		apmib_get(MIB_HW_11N_THER, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "set_mib", "ther", intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "set_mib", "thermal1", intVal);

		apmib_get(MIB_HW_11N_THER_2, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "set_mib", "thermal2", intVal);

#ifdef CONFIG_SHARE_XSTAL
		//share xstal depends on hw design,
		//for 97G: 2g share 5g xstal
		apmib_get(MIB_HW_5G_ON_WLAN, (void *)&hw_5g_wlan);
		if(hw_5g_wlan==0)
			hw_wlan0_5g = 1;
		else
			hw_wlan0_5g = 0;

		if(hw_wlan0_5g){
			apmib_get(MIB_HW_XCAP_WLAN0_AX, (void *)&intVal);
			if(!intVal){
				printf("hw calibration 5G on WLAN0, but MIB_HW_XCAP_WLAN0_AX is 0\n");
			}
		}else{
			apmib_get(MIB_HW_XCAP_WLAN1_AX, (void *)&intVal);
			if(!intVal){
				printf("hw calibration 5G on WLAN1, but MIB_HW_XCAP_WLAN1_AX is 0\n");
			}
		}
		if(wlan_index==0)
			snprintf(wlan_interface, sizeof(wlan_interface), WLAN_IF_S[1]);
		else if(wlan_index==1)
			snprintf(wlan_interface, sizeof(wlan_interface), WLAN_IF_S[0]);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "flash_set", "xcap", intVal);
#else
		apmib_get(MIB_HW_11N_XCAP, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "set_mib", "xcap", intVal);
#endif
#if 0 //TO add mib
		apmib_get(MIB_HW_MIMO_TR_MODE, (void *)&intVal);
		iwpriv_cmd(IWPRIV_INT, wlan_interface, "set_mib", "MIMO_TR_mode", intVal);
#endif

		/* for 97G, 8812F, 8832AX...
		   DPK parameters is set in wlan driver
		   DPK mib is not needed
		*/
		apmib_recov_wlanIdx();
	}
	return 0;
}

int check_wlan_mib()
{
	int i, j;
	unsigned int is_ax_support=0, intVal=0, isMibUpdate=0;

	apmib_save_wlanIdx();

	for (i = 0; i < NUM_WLAN_INTERFACE; i++)
	{
		wlan_idx = i;
		vwlan_idx = 0;
		apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);
		if(intVal)
			continue;
		apmib_get(MIB_WLAN_AX_SUPPORT, (void *)&is_ax_support);
		for (j = 1; j <= NUM_VWLAN_INTERFACE; j++)
		{
			vwlan_idx = j;
			apmib_get(MIB_WLAN_AX_SUPPORT, (void *)&intVal);

			//Change the is_ax_support value of vaps
			if(is_ax_support != intVal){
				apmib_set(MIB_WLAN_AX_SUPPORT, (void *)&is_ax_support);
				isMibUpdate = 1;
			}
		}

		vwlan_idx = 0;
		if(set_hw_mib(is_ax_support, i))
			printf("%s (%d) Set wlan%d hw mib failed!\n", __FUNCTION__, __LINE__, i);
	}

	apmib_recov_wlanIdx();
	if(isMibUpdate)
		apmib_update(CURRENT_SETTING);

	return 0;
}

int set_wifi_misc(int wlan_index, int vwlan_index)
{
	char ifname[64] = {0}, ifname_root[64] = {0};
	char filename_ax[64] = {0}, filename_ac[64] = {0}, cmd_file[128] = {0}, filename_new[64] = {0}, cmd[128] = {0};
	unsigned int wlanDisable=1, wlanDisable_root=0;
	unsigned int band=1, band_root=0;
	unsigned int is_ax_support = 0;
	unsigned int intValue, level_percent;
	unsigned int enabled = 0;
	DIR *dir;

	apmib_save_wlanIdx();

	memset(ifname, 0, sizeof(ifname));
	memset(ifname_root, 0, sizeof(ifname_root));
	snprintf(ifname_root, sizeof(ifname_root), WLAN_IF_S[wlan_index]);
#ifdef UNIVERSAL_REPEATER
	if(vwlan_index == NUM_VWLAN_INTERFACE)
		snprintf(ifname, sizeof(ifname), VXD_IF[wlan_index]);
	else
#endif
	if(vwlan_index == WLAN_ROOT_ITF_INDEX)
		snprintf(ifname, sizeof(ifname), WLAN_IF_S[wlan_index]);
	else
		snprintf(ifname, sizeof(ifname), VAP_IF, wlan_index, vwlan_index-1);

	wlan_idx = wlan_index;
	vwlan_idx = 0;
	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisable_root);
	apmib_get(MIB_WLAN_BAND, (void *)&band_root);
	apmib_get(MIB_WLAN_AX_SUPPORT, (void *)&is_ax_support);

	vwlan_idx = vwlan_index;
	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlanDisable);
	apmib_get(MIB_WLAN_BAND, (void *)&band);

	if(vwlan_index != WLAN_ROOT_ITF_INDEX)
	{
		snprintf(filename_new, sizeof(filename_new), "/var/%s", ifname);
		if(!wlanDisable && !wlanDisable_root)
		{
			dir = opendir(filename_new);
			if(NULL != dir)
			{
				printf("File: %s has been linked.\n", filename_new);
				closedir(dir);
			}
			else
			{
				snprintf(filename_ax, sizeof(filename_ax), DIR_WIFI6_DRV, ifname);
				snprintf(filename_ac, sizeof(filename_ac), DIR_WIFI5_DRV, ifname);

				if(1 == is_ax_support)
					snprintf(cmd_file, sizeof(cmd_file), "ln -snf %s %s", filename_ax, filename_new);
				else if(0 == is_ax_support)
					snprintf(cmd_file, sizeof(cmd_file), "ln -snf %s %s", filename_ac, filename_new);
				system(cmd_file);
			}

			if(vwlan_index != NUM_VWLAN_INTERFACE){
				snprintf(cmd, sizeof(cmd), "iwpriv %s set_mib band=%d", ifname, band);
				if(rtk_get_interface_flag(ifname, 5, IS_RUN))
				{
					system(cmd);
				}
			}
		}
	}
	else //root
	{
		vwlan_idx = 0;
		if(is_ax_support == 0)
		{
			apmib_get(MIB_WLAN_MC2U_DISABLED, (void *)&intValue);
			snprintf(cmd, sizeof(cmd), "iwpriv %s set_mib mc2u_disable=%d", ifname, intValue);
			system(cmd);
		}
		else
		{
			apmib_get(MIB_WLAN_RFPOWER_SCALE, (void *)&intValue);
			if(intValue < 5)
			{
				switch(intValue)
				{
					case 0:
						level_percent = 100;
						break;
					case 1:
						level_percent = 70;
						break;
					case 2:
						level_percent = 50;
						break;
					case 3:
						level_percent = 35;
						break;
					case 4:
						level_percent = 15;
						break;
					default:
						break;
				}
			}
			else if(intValue >= 10 && intValue <= 90 && intValue%10 == 0)
				level_percent = intValue;
			else
			{
				level_percent = 100;
				printf("%s(%d) The value of MIB_WLAN_RFPOWER_SCALE set is wrong!\n", __FUNCTION__, __LINE__);
			}

			apmib_get(MIB_WLAN_TWT_ENABLED, (void *)&enabled);

			if(is_interface_run(ifname)){
				iwpriv_cmd(IWPRIV_INT, ifname, "set_mib", "powerpercent", level_percent);
#ifdef WLAN_LIFETIME_SUPPORT
				/*lifetime must be set after wlan interface open*/
				unsigned int lifetime=0;
				apmib_get(MIB_WLAN_LIFETIME, (void *)&lifetime);
				iwpriv_cmd(IWPRIV_INT, ifname, "set_mib", "lifetime", lifetime);
#endif
				iwpriv_cmd(IWPRIV_INT, ifname, "set_mib", "twt_enable", enabled);
			}else{
				printf("ifname: %s is not run.\n", ifname);
			}
		}
	}

	apmib_recov_wlanIdx();
	return 0;
}

#ifdef CONFIG_GUEST_ACCESS_SUPPORT
int setup_wlan_guestaccess()
{
	unsigned int root_disabled=1, vap_disabled=1, guest_access=0, brsc_blacklist_en=0;
	char ifname[16]={0}, parm[100]={0};
	int i, j;

	apmib_save_wlanIdx();

	//clean old chain
	clean_wlan_guestaccess();
	//add new chain
	RunSystemCmd(NULL_FILE, EBTABLES, "-N", GUEST_ACCESS_CHAIN_EB, NULL_STR);
	RunSystemCmd(NULL_FILE, EBTABLES, "-P", GUEST_ACCESS_CHAIN_EB, "RETURN", NULL_STR);
	RunSystemCmd(NULL_FILE, EBTABLES, "-I", FW_FORWARD, "-j", GUEST_ACCESS_CHAIN_EB, NULL_STR);
	RunSystemCmd(NULL_FILE, IPTABLES, "-N", GUEST_ACCESS_CHAIN_IP, NULL_STR);
	RunSystemCmd(NULL_FILE, IPTABLES, "-I", FW_INPUT, "-j", GUEST_ACCESS_CHAIN_IP, NULL_STR);
#ifdef CONFIG_IPV6
	RunSystemCmd(NULL_FILE, IP6TABLES, "-N", GUEST_ACCESS_CHAIN_IPV6, NULL_STR);
	RunSystemCmd(NULL_FILE, IP6TABLES, "-I", FW_INPUT, "-j", GUEST_ACCESS_CHAIN_IPV6, NULL_STR);
#endif

	for(i=0; i<NUM_WLAN_INTERFACE; ++i){
		wlan_idx = i;
		vwlan_idx = 0;
		apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&root_disabled);
		if(root_disabled)
			continue;
		for(j=0; j<NUM_VWLAN_INTERFACE; ++j){
			vwlan_idx = j;
			apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&vap_disabled);
			if(vap_disabled)
				continue;
			apmib_get(MIB_WLAN_ACCESS, (void *)&guest_access);
			rtk_wlan_get_ifname(i, j, ifname);
			if(guest_access){
				brsc_blacklist_en = 1;
#ifdef CONFIG_RTW_BRSC
				snprintf(parm, sizeof(parm), "echo add_black_ifname %s > %s", ifname, PROC_BRSC);
				system(parm);
#endif
				/*********between guest access and lan**********/
				RunSystemCmd(NULL_FILE, EBTABLES, "-A", GUEST_ACCESS_CHAIN_EB, "-i", ifname, "-o", "eth0", "-j", (char *)FW_DROP, NULL_STR);
				RunSystemCmd(NULL_FILE, EBTABLES, "-A", GUEST_ACCESS_CHAIN_EB, "-i", "eth0", "-o", ifname, "-j", (char *)FW_DROP, NULL_STR);
				/*********between guest access and wlan**********/
				RunSystemCmd(NULL_FILE, EBTABLES, "-A", GUEST_ACCESS_CHAIN_EB, "-i", ifname, "-o", "wlan+", "-j", (char *)FW_DROP, NULL_STR);
				RunSystemCmd(NULL_FILE, EBTABLES, "-A", GUEST_ACCESS_CHAIN_EB, "-i", "wlan+", "-o", ifname, "-j", (char *)FW_DROP, NULL_STR);
				/*********guest access sta can't access to DUT web page*******/
				/*Note: if /proc/sys/net/bridge/bridge-nf-call-iptables value is 0, then the following iptable rule is not effective*/
				RunSystemCmd(NULL_FILE, IPTABLES, "-A", GUEST_ACCESS_CHAIN_IP, "-m", "physdev", "--physdev-in", ifname, "-p", "tcp", "-j", (char *)FW_DROP, NULL_STR);
				#ifdef CONFIG_IPV6
				/*Note: if /proc/sys/net/bridge/bridge-nf-call-ip6tables value is 0, then the following ip6table rule is not effective*/
				RunSystemCmd(NULL_FILE, IP6TABLES, "-A", GUEST_ACCESS_CHAIN_IPV6, "-m", "physdev", "--physdev-in", ifname, "-p", "tcp", "-j", (char *)FW_DROP, NULL_STR);
				#endif
				/*********block stations under guest access**********/
				iwpriv_cmd(IWPRIV_INT, ifname, "set_mib", "block_relay", 1);
				iwpriv_cmd(IWPRIV_INT, ifname, "set_mib", "guest_access", 1);
			}else{
				/* block_relay is setting via hostapd ap_isolate */
				iwpriv_cmd(IWPRIV_INT, ifname, "set_mib", "guest_access", 0);
			}
		}
	}
#ifdef CONFIG_RTW_BRSC
	if(brsc_blacklist_en){
		snprintf(parm, sizeof(parm), "echo blacklist_enable > %s", PROC_BRSC);
		system(parm);
	}
#endif
	apmib_recov_wlanIdx();
	return 0;
}

int clean_wlan_guestaccess()
{
	char parm[100]={0};

	//flush all rules in GUEST_ACCESS_CHAIN_EB
	RunSystemCmd(NULL_FILE, EBTABLES, "-F", GUEST_ACCESS_CHAIN_EB, NULL_STR);
	//delete GUEST_ACCESS_CHAIN_EB referenced in FORWARD
	RunSystemCmd(NULL_FILE, EBTABLES, "-D", FW_FORWARD,"-j", GUEST_ACCESS_CHAIN_EB, NULL_STR);
	//delete GUEST_ACCESS_CHAIN_EB
	RunSystemCmd(NULL_FILE, EBTABLES, "-X", GUEST_ACCESS_CHAIN_EB, NULL_STR);
	RunSystemCmd(NULL_FILE, IPTABLES, "-F", GUEST_ACCESS_CHAIN_IP, NULL_STR);
	RunSystemCmd(NULL_FILE, IPTABLES, "-D", FW_INPUT,"-j", GUEST_ACCESS_CHAIN_IP, NULL_STR);
	RunSystemCmd(NULL_FILE, IPTABLES, "-X", GUEST_ACCESS_CHAIN_IP, NULL_STR);
#ifdef CONFIG_IPV6
	RunSystemCmd(NULL_FILE, IP6TABLES, "-F", GUEST_ACCESS_CHAIN_IPV6, NULL_STR);
	RunSystemCmd(NULL_FILE, IP6TABLES, "-D", FW_INPUT,"-j", GUEST_ACCESS_CHAIN_IPV6, NULL_STR);
	RunSystemCmd(NULL_FILE, IP6TABLES, "-X", GUEST_ACCESS_CHAIN_IPV6, NULL_STR);
#endif
#ifdef CONFIG_RTW_BRSC
	snprintf(parm, sizeof(parm), "echo blacklist_disable > %s", PROC_BRSC);
	system(parm);
	snprintf(parm, sizeof(parm), "echo flush_black_ifname > %s", PROC_BRSC);
	system(parm);
#endif
}
#endif

#ifdef MULTI_AP_OPEN_ENCRYPT_SUPPORT

#define SUPPORT_OPEN_ENCRYPT 		1
#define NOT_SUPPORT_OPEN_ENCRYPT 	0

int write_backhaul_encrypt_mode_flag(void)
{
	FILE *fp=NULL;
	unsigned char backhaul_encrypt_none_en = 0;
	apmib_get(MIB_MAP_BACKHAUL_ENCRYPT_NONE, (void *)&backhaul_encrypt_none_en);

	if((fp = fopen("/tmp/backhaul_encrypt_mode", "w")) == NULL)
	{
		printf("***** Open file /tmp/backhaul_encrypt_mode failed !\n");
		return -1;
	}
	fprintf(fp, "%d\n", backhaul_encrypt_none_en);
	fclose(fp);
	return 0;

}

#endif

#ifdef CONFIG_RTK_HOSTAPD_ONLY_GLOBAL
#if defined(MBSSID)
int set_vap_wifi_priv_cfg(int wlan_index, config_wlan_ssid ssid_index)
{
	int j;

	for (j=1; j<(NUM_VWLAN+1); j++){
		if(ssid_index!=CONFIG_SSID_ALL && ssid_index!=j) {
			continue;
		}
		start_wifi_priv_cfg(wlan_index, j);
	}

	return 0;
}
#endif

int is_dif_phy_band(config_wlan_target target, int wlan_index)
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
#endif

/*
	startWlan_wifi6():
		set wlan mib
		start hostapd/wpa_supplicant
		start wlan app
*/
int startWlan_wifi6(config_wlan_target target, config_wlan_ssid ssid_index)
{
	int i,j;
	unsigned int is_ax_support = 0, wlan_disable=1;
	int phy_band_select=0;
#ifdef RTK_WLAN_MANAGER
	unsigned int band_steering_enable;
	unsigned int crossband_enable;
#endif
#ifdef CONFIG_RTK_HOSTAPD_ONLY_GLOBAL
	unsigned int wlan_mode = 0;
#endif

	if(ssid_index == CONFIG_SSID_ALL)
		check_wlan_mib();

	apmib_save_wlanIdx();

#ifdef MULTI_AP_OPEN_ENCRYPT_SUPPORT
	write_backhaul_encrypt_mode_flag();
	setupMultiAPController_encrypt();
#endif

#ifdef CONFIG_RTK_HOSTAPD_ONLY_GLOBAL
	for(i = 0; i<NUM_WLAN_INTERFACE; i++){
		wlan_idx = i;
		vwlan_idx = 0;

		if(is_dif_phy_band(target, i))
			continue;

		wlan_disable = 1;
		apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disable);
		if(wlan_disable)
			continue;

		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);

		if(ssid_index==CONFIG_SSID_ALL || ssid_index==CONFIG_SSID_ROOT){
			start_wifi_priv_cfg(i ,WLAN_ROOT_ITF_INDEX);
			if(wlan_mode == CLIENT_MODE)
				start_hapd_wpas_process(i, CONFIG_SSID_ROOT);
		}

		apmib_get(MIB_WLAN_AX_SUPPORT, (void *)&is_ax_support);
		if(!is_ax_support)
		{
#if defined(MBSSID)
			set_vap_wifi_priv_cfg(i, ssid_index);
#endif
		}
	}

	start_hapd_global_process(target, ssid_index);

	for(i = 0; i<NUM_WLAN_INTERFACE; i++){
		wlan_idx = i;
		vwlan_idx = 0;

		if(is_dif_phy_band(target, i))
			continue;

		wlan_disable = 1;
		apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disable);
		if(wlan_disable)
			continue;

#ifdef UNIVERSAL_REPEATER
		if(ssid_index==CONFIG_SSID_ALL || ssid_index==NUM_VWLAN_INTERFACE)
		{
			apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
			if(wlan_mode == AP_MODE)
				start_hapd_wpas_process(i, NUM_VWLAN_INTERFACE);
		}
#endif

		apmib_get(MIB_WLAN_AX_SUPPORT, (void *)&is_ax_support);
		if(is_ax_support)
		{
#if defined(MBSSID)
			set_vap_wifi_priv_cfg(i, ssid_index);
#endif
		}

		for(j=0; j<(NUM_VWLAN_INTERFACE+1); j++){
			if(ssid_index!=CONFIG_SSID_ALL && ssid_index!=j) {
				continue;
			}
			set_wifi_misc(i, j);
		}
	}

#else
	for(i = 0; i<NUM_WLAN_INTERFACE; i++){
		wlan_idx = i;
		vwlan_idx = 0;
		apmib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&phy_band_select);
		if((target == CONFIG_WLAN_2G && phy_band_select == PHYBAND_5G)
			|| (target == CONFIG_WLAN_5G && phy_band_select == PHYBAND_2G)) {
			continue;
		}

		apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disable);
		if(wlan_disable)
			continue;

		if(ssid_index==CONFIG_SSID_ALL || ssid_index==CONFIG_SSID_ROOT){
			start_wifi_priv_cfg(i ,WLAN_ROOT_ITF_INDEX);
		}

		/*
			wifi6 vap is registered in hostapd, set mib after start hostapd
			wifi5 vap is registered in wlan driver, set mib before start hostapd
		*/
		apmib_get(MIB_WLAN_AX_SUPPORT, (void *)&is_ax_support);
		if(is_ax_support){ //wifi6
			start_hapd_wpas_process(i, ssid_index);
		}

		for (j=1; j<NUM_VWLAN_INTERFACE; j++){
			if(ssid_index!=CONFIG_SSID_ALL && ssid_index!=j) {
				continue;
			}
			start_wifi_priv_cfg(i, j);
			set_wifi_misc(i, j);
		}

		if(!is_ax_support){ //wifi5
			start_hapd_wpas_process(i, ssid_index);
		}

		for(j=0; j<(NUM_VWLAN_INTERFACE+1); j++){
			if(ssid_index!=CONFIG_SSID_ALL && ssid_index!=j) {
				continue;
			}
			set_wifi_misc(i, j);
		}

#ifdef UNIVERSAL_REPEATER
		if(ssid_index==CONFIG_SSID_ALL || ssid_index==NUM_VWLAN_INTERFACE)
			set_wifi_misc(i, NUM_VWLAN_INTERFACE);
#endif
	}
#endif

#ifdef RTK_WLAN_MANAGER
	wlan_idx = 0;
	vwlan_idx = 0;
#ifdef BAND_STEERING_SUPPORT
	apmib_get(MIB_WIFI_STA_CONTROL_ENABLE, (void *)&band_steering_enable);
#endif
#ifdef RTK_CROSSBAND_REPEATER
	apmib_get(MIB_WLAN_CROSSBAND_ENABLE, (void *)&crossband_enable);
#endif
	if(band_steering_enable || crossband_enable)
		rtk_start_wlan_manager();
#endif

	apmib_recov_wlanIdx();

#if 0//def CONFIG_GUEST_ACCESS_SUPPORT
	/* please see setup_wlan_guestaccess() in setinit() */
	if(ssid_index == CONFIG_SSID_ALL){
		setup_wlan_guestaccess();
	}
#endif

#ifdef RTK_THERMAL_CONTROL
	system("/bin/sh /bin/wlan_thermal.sh");
#endif

	return 0;
}

/*
	stopWlan_wifi6():
		kill hostapd/wpa_supplicant
		kill hostapd_cli/wpa_supplicant_cli
		kill wlan app
*/
int stopWlan_wifi6(config_wlan_target target, config_wlan_ssid ssid_index)
{
	int status = 0,j, phy_band_select;

	apmib_save_wlanIdx();

	//kill hostapd
#ifdef CONFIG_RTK_HOSTAPD_ONLY_GLOBAL
	stop_hapd_wpas_global_process(target, ssid_index);
#else
	for(j=0;j<NUM_WLAN_INTERFACE;j++){
		wlan_idx = j;
		vwlan_idx = 0;
		apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phy_band_select);
		if((target == CONFIG_WLAN_2G && phy_band_select == PHYBAND_5G)
		|| (target == CONFIG_WLAN_5G && phy_band_select == PHYBAND_2G)) {
			continue;
		}
		stop_hapd_wpas_process(j, ssid_index);
	}
#endif
	//kill wlan app
	//TODO
	apmib_recov_wlanIdx();
#ifdef RTK_WLAN_MANAGER
	rtk_stop_wlan_manager();
#endif

	return status;
}
