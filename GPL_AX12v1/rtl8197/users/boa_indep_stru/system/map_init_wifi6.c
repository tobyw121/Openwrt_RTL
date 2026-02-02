#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#include "apmib.h"
//#include "utility.h"
#include "map_init_wifi6.h"


#define NULL_FILE 0
#define NULL_STR ""


#if defined(TRIBAND_SUPPORT)
#define RADIO_NUMBER 3
#else
#define RADIO_NUMBER 2
#endif

enum {
	MODE_CONTROLLER,
	MODE_AGENT,
	MODE_WFA_TEST
};

#define AUTH_DISABLED              0x01
#define AUTH_WPA               0x02
#define AUTH_WEP               0x04
#define AUTH_WPA2              0x20
#define AUTH_WPA3              0x40

static const uint8_t base64_table[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

uint8_t * map_base64_encode(const uint8_t *src, size_t len,
			      size_t *out_len)
{
	uint8_t *out, *pos;
	const uint8_t *end, *in;
	size_t olen;
	int line_len;

	olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
	olen += olen / 72; /* line feeds */
	olen++; /* nul termination */
	if (olen < len)
		return NULL; /* integer overflow */
	out = malloc(olen);
	if (out == NULL)
		return NULL;

	end = src + len;
	in = src;
	pos = out;
	line_len = 0;
	while (end - in >= 3) {
		*pos++ = base64_table[in[0] >> 2];
		*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = base64_table[in[2] & 0x3f];
		in += 3;
		line_len += 4;
		// if (line_len >= 72) {
		// 	*pos++ = '\n';
		// 	line_len = 0;
		// }
	}

	if (end - in) {
		*pos++ = base64_table[in[0] >> 2];
		if (end - in == 1) {
			*pos++ = base64_table[(in[0] & 0x03) << 4];
			*pos++ = '=';
		} else {
			*pos++ = base64_table[((in[0] & 0x03) << 4) |
					      (in[1] >> 4)];
			*pos++ = base64_table[(in[1] & 0x0f) << 2];
		}
		*pos++ = '=';
		line_len += 4;
	}

	// if (line_len)
	// 	*pos++ = '\n';

	*pos = '\0';
	if (out_len)
		*out_len = pos - out;
	return out;
}

#ifdef MULTI_AP_OPEN_ENCRYPT_SUPPORT
int setupMultiAPController_encrypt(void)
{
	int i=0, j =0, map_idx= WLAN_VAP_ITF_INDEX;
	int backhaul_band = 1;
	unsigned char mibVal = 0, encrypt_val = 0;
	unsigned char backhaul_encrypt_none_en = 0;
	apmib_get(MIB_MAP_BACKHAUL_ENCRYPT_NONE, (void *)&backhaul_encrypt_none_en);

	apmib_get(MIB_MAP_CONTROLLER, (void *)&mibVal);
	if(mibVal == MAP_CONTROLLER)
	{
		apmib_save_wlanIdx();
		for (i = 0; i < NUM_WLAN_INTERFACE; i++) {
			for (j = 0; j < NUM_VWLAN_INTERFACE; j++) {
					wlan_idx = i;
					vwlan_idx = j;
					apmib_get(MIB_WLAN_MAP_BSS_TYPE, (void *)&mibVal);
					if (MAP_BACKHAUL_BSS & mibVal) {
						backhaul_band = i;
						break;
					}
			}
		}
		if(backhaul_encrypt_none_en){
			encrypt_val = ENCRYPT_DISABLED;
		}else{
#ifdef WLAN_WPA3
			encrypt_val = ENCRYPT_WPA2_WPA3_MIXED;
#else
			encrypt_val = ENCRYPT_WPA2;
#endif
		}
			apmib_set(MIB_WLAN_ENCRYPT, (void *)&encrypt_val);

#if defined(WIFI_SIMPLE_CONFIG) || defined(CONFIG_REPEATER_WPS_SUPPORT)
		if(backhaul_encrypt_none_en == 0){
			mibVal = WSC_AUTH_WPA2PSK;
			apmib_set(MIB_WLAN_WSC_AUTH, (void *)&mibVal);
			mibVal = WSC_ENCRYPT_AES;
			apmib_set(MIB_WLAN_WSC_ENC, (void *)&mibVal);
		}
		else{
			mibVal = WSC_AUTH_OPEN;
			apmib_set(MIB_WLAN_WSC_AUTH, (void *)&mibVal);
			mibVal = WSC_ENCRYPT_NONE;
			apmib_set(MIB_WLAN_WSC_ENC, (void *)&mibVal);
		}
		mibVal = 1;
		apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&mibVal);
		mibVal = 0;
		apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&mibVal);
		mibVal = WPA_CIPHER_AES;
		apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&mibVal);
		mibVal = 1;
		apmib_set(MIB_WLAN_HIDDEN_SSID, (void *)&mibVal);
#ifdef CONFIG_IEEE80211W
#ifdef WLAN_WPA3
		if(encrypt_val == ENCRYPT_WPA2_WPA3_MIXED)
		{
			mibVal = 1;
			apmib_set(MIB_WLAN_IEEE80211W, (void *)&mibVal);
			mibVal = 0;
			apmib_set(MIB_WLAN_SHA256_ENABLE, (void *)&mibVal);
		}
		else
		{
			mibVal = 0;
			apmib_set(MIB_WLAN_IEEE80211W, (void *)&mibVal);
			apmib_set(MIB_WLAN_SHA256_ENABLE, (void *)&mibVal);
		}
#else
		mibVal = 0;
		apmib_set(MIB_WLAN_IEEE80211W, (void *)&mibVal);
		apmib_set(MIB_WLAN_SHA256_ENABLE, (void *)&mibVal);
#endif
#endif
#endif
	}
	apmib_recov_wlanIdx();
	return 0;
}
#endif


void map_write_to_config(FILE *fp, uint8_t config_nr, char **config_array)
{
	int i;
	for (i = 0; i < config_nr; i++) {
		fprintf(fp, "%s", config_array[i]);
		if (i < (config_nr - 1)) {
			fprintf(fp, ",");
		} else {
			fprintf(fp, "\n");
		}
	}
}

void map_write_to_config_dec(FILE *fp, uint8_t config_nr, uint8_t *config_array)
{
	int i;
	for (i = 0; i < config_nr; i++) {
		fprintf(fp, "%d", config_array[i]);
		if (i < (config_nr - 1)) {
			fprintf(fp, ",");
		} else {
			fprintf(fp, "\n");
		}
	}
}

void map_write_to_config_dec_int(FILE *fp, unsigned char config_nr, int *config_array)
{
	int i;
	for (i = 0; i < config_nr; i++) {
		fprintf(fp, "%d", config_array[i]);
		if (i < (config_nr - 1)) {
			fprintf(fp, ",");
		} else {
			fprintf(fp, "\n");
		}
	}
}

void map_write_vendor_data_to_config(FILE *fp, struct mib_specific_info *mib_specific_data, unsigned char config_type)
{
	int i = 0, j = 0;

	if(mib_specific_data->vendor_specific_nr){
		if (config_type == MAP_CONFIG_2G)
			fprintf(fp, "%s", "[vendor_data_2.4g]\n");
		else if (config_type == MAP_CONFIG_5GL)
			fprintf(fp, "%s", "[vendor_data_5gl]\n");
		else if (config_type == MAP_CONFIG_5GH)
			fprintf(fp, "%s", "[vendor_data_5gh]\n");

		fprintf(fp, "vendor_data_nr = %d\n", mib_specific_data->vendor_specific_nr);
		fprintf(fp, "%s", "vendor_payload = ");
		for (i = 0; i < mib_specific_data->vendor_specific_nr; i++){
			switch(mib_specific_data->vendor_specific_data[i].tlv_type) {
				case TLV_TYPE_WLAN_ACCESS_CONTROL: {
					fprintf(fp, "%s", "[ACL]");
					for (j = 0; j < mib_specific_data->vendor_specific_data[i].payload_nr; j++){
						struct acl_info *entry_test = NULL;
						entry_test = (struct acl_info *)&(mib_specific_data->vendor_specific_data[i].payload[0]) + j;

						fprintf(fp, "wlan_Idx:%d", entry_test->wlan_Idx);
						fprintf(fp, " ");
						fprintf(fp, "vwlanIdx:%d", entry_test->vwlanIdx);
						fprintf(fp, " ");
						fprintf(fp, "acl_enable:%d", entry_test->acl_enable);
						fprintf(fp, " ");
						fprintf(fp, "entry_num:%d", entry_test->entry_num);
						fprintf(fp, " ");
						fprintf(fp, "macAddr:%02x%02x%02x%02x%02x%02x", entry_test->macAddr[0],entry_test->macAddr[1],entry_test->macAddr[2],
							entry_test->macAddr[3],entry_test->macAddr[4],entry_test->macAddr[5]);
						if (j < (mib_specific_data->vendor_specific_data[i].payload_nr - 1))
							fprintf(fp, ";");
						else if (i != mib_specific_data->vendor_specific_nr - 1)
							fprintf(fp, ",");
					}
					break;
				}
#ifdef WLAN_SCHEDULE_SUPPORT
				case TLV_TYPE_WLAN_SCHEDULE: {
					fprintf(fp, "%s", "[WLAN_SCHEDULE]");
					for (j = 0; j < mib_specific_data->vendor_specific_data[i].payload_nr; j++){
						struct wlan_schedule_info *entry_test = NULL;
						entry_test = (struct wlan_schedule_info *)&(mib_specific_data->vendor_specific_data[i].payload[0]) + j;

						fprintf(fp, "wlan_Idx:%d", entry_test->wlan_Idx);
						fprintf(fp, " ");
						fprintf(fp, "enable:%d", entry_test->enable);
						fprintf(fp, " ");
						fprintf(fp, "entry_num:%d", entry_test->entry_num);
						fprintf(fp, " ");
						fprintf(fp, "mode:%d", entry_test->mode);
						fprintf(fp, " ");
						fprintf(fp, "eco:%d", entry_test->eco);
						fprintf(fp, " ");
						fprintf(fp, "fTime:%d", entry_test->fTime);
						fprintf(fp, " ");
						fprintf(fp, "tTime:%d", entry_test->tTime);
						fprintf(fp, " ");
						fprintf(fp, "day:%d", entry_test->day);
						fprintf(fp, " ");
						fprintf(fp, "inst_num:%d", entry_test->inst_num);

						if (j < (mib_specific_data->vendor_specific_data[i].payload_nr - 1))
							fprintf(fp, ";");
						else if (i != mib_specific_data->vendor_specific_nr - 1)
							fprintf(fp, ",");
					}
					break;
				}
#endif
				default: {
					printf("(%s %d) Unexpected vendor specific data type...\n",__FUNCTION__,__LINE__);
					continue;
				}
			}
		}

		fprintf(fp, "\n");
		fprintf(fp, "%s", "vendor_oui = ");
		for (i = 0; i < mib_specific_data->vendor_specific_nr; i++){
			fprintf(fp, "%02x%02x%02x", mib_specific_data->vendor_specific_data[i].vendor_oui[0],mib_specific_data->vendor_specific_data[i].vendor_oui[1],mib_specific_data->vendor_specific_data[i].vendor_oui[2]);
			if (i < (mib_specific_data->vendor_specific_nr - 1))
				fprintf(fp, ",");
		}
	}
	fprintf(fp, "\n");
}

#if 0
/*
	MIB_MAP_SYNC_SPECIFIC_DATA:
		BIT 0 ---> Sync wlan acl data
		BIT 1 ---> Sync wlan schedule data
		BIT 2-31---> Reserved          */
uint8_t map_read_specific_mib_to_config(struct mib_specific_info *mib_specific_data)
{
	if ((NULL == mib_specific_data) || (NULL != mib_specific_data->vendor_specific_data)) {
		return 1;
	}

	int i = 0, j = 0, k = 0, radio_idx = 0, vap_idx = 0, sync_specific_data = 0;
	unsigned int entryNum = 0, enable = 0, mode = 0;
	int vendor_specific_idx = 0, payload_idx = 0;
#ifdef WLAN_VAP_ACL
	MIB_CE_MBSSIB_T    wlan_entry;
	MIB_CE_WLAN_AC_T   acl_entry;
	struct acl_info    *acl_entry_tmp = {0};
#endif
#ifdef WLAN_SCHEDULE_SUPPORT
	MIB_CE_SCHEDULE_T  schedule_entry;
	struct wlan_schedule_info *schedule_entry_tmp = {0};
#endif

	apmib_get(MIB_MAP_SYNC_SPECIFIC_DATA, (void *)&sync_specific_data);

#ifdef WLAN_VAP_ACL
	if (sync_specific_data & MAP_SYNC_WLAN_ACL_DATA) {
 		printf( "(%s %d) Need sync wlan acl data to agent!\n",__FUNCTION__,__LINE__);

		vendor_specific_idx = mib_specific_data->vendor_specific_nr += 1;
 		mib_specific_data->vendor_specific_data =  (struct vendor_specific_info *)realloc(mib_specific_data->vendor_specific_data, sizeof(struct vendor_specific_info) * vendor_specific_idx);
		mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload_nr  = 0;
		mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload     = NULL;
		mib_specific_data->vendor_specific_data[vendor_specific_idx-1].tlv_type    =  TLV_TYPE_WLAN_ACCESS_CONTROL;
		memcpy(mib_specific_data->vendor_specific_data[vendor_specific_idx-1].vendor_oui, REALTEK_OUI, 3);

		for (i = 0; i < RADIO_NUMBER; i++) {
			for(k = 0; k < WLAN_MBSSID_NUM+1; k++) {
				radio_idx  = i;
				vap_idx = k;
				entryNum = mib_chain_total(MIB_WLAN_AC_TBL + radio_idx + vap_idx*WLAN_MIB_MAPPING_STEP);

				memset(&wlan_entry, 0, sizeof(MIB_CE_MBSSIB_T));
				if(!mib_chain_get(MIB_MBSSIB_TBL + radio_idx, 0, &wlan_entry)){
					printf("(%s) %d Error reading mib in map_read_specific_mib_to_config!\n",__FUNCTION__,__LINE__);
					return 1;
				}

				if (entryNum == 0) { //only sync acl_enable mode to agent
					payload_idx = mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload_nr += 1;

					mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload = realloc(mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload, sizeof(struct acl_info) * payload_idx);

					acl_entry_tmp = (struct acl_info *)&(mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload[0]) + payload_idx - 1;

					memset(acl_entry_tmp, 0, sizeof(struct acl_info));

					acl_entry_tmp->acl_enable   =   wlan_entry.acl_enable;
					acl_entry_tmp->entry_num    =   entryNum;
					acl_entry_tmp->wlan_Idx     =   radio_idx;
				} else {  //sync acl_enable and acl tbl to agent
					for (j = 0; j < entryNum; j++) {
						memset(&acl_entry, 0, sizeof(MIB_CE_WLAN_AC_T));

						if(!mib_chain_get(MIB_WLAN_AC_TBL + radio_idx + vap_idx*WLAN_MIB_MAPPING_STEP, j, &acl_entry)){
							printf("(%s) %d Error reading mib in map_read_specific_mib_to_config!\n",__FUNCTION__,__LINE__);
							return 1;
						}

						payload_idx = mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload_nr += 1;

						mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload = realloc(mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload, sizeof(struct acl_info) * payload_idx);

						acl_entry_tmp = (struct acl_info *)&(mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload[0]) + payload_idx - 1;

						memset(acl_entry_tmp, 0, sizeof(struct acl_info));
						acl_entry_tmp->wlan_Idx     =   acl_entry.wlanIdx;
						acl_entry_tmp->vwlanIdx     =   vap_idx;
						acl_entry_tmp->acl_enable   =   wlan_entry.acl_enable;
						acl_entry_tmp->entry_num    =   entryNum;

						memcpy(acl_entry_tmp->macAddr, acl_entry.macAddr, MACADDRLEN);
					}
				}
			}
		}
	}
#endif

#ifdef WLAN_SCHEDULE_SUPPORT
	if (sync_specific_data & MAP_SYNC_WLAN_SCHEDULE_DATA) {
		printf("(%s %d) Need sync wlan schedule data to agent!\n",__FUNCTION__,__LINE__);

		vendor_specific_idx = mib_specific_data->vendor_specific_nr += 1;
		mib_specific_data->vendor_specific_data =  (struct vendor_specific_info *)realloc(mib_specific_data->vendor_specific_data, sizeof(struct vendor_specific_info) * vendor_specific_idx);
		mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload_nr  = 0;
		mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload     = NULL;
		mib_specific_data->vendor_specific_data[vendor_specific_idx-1].tlv_type    =  TLV_TYPE_WLAN_SCHEDULE;
		memcpy(mib_specific_data->vendor_specific_data[vendor_specific_idx-1].vendor_oui, REALTEK_OUI, 3);

		payload_idx = 0;
		for (i = 0; i < RADIO_NUMBER; i++) {
			radio_idx  = i;
			apmib_get(MIB_WLAN_SCHEDULE_ENABLED + radio_idx, (void *)&enable);
			apmib_get(MIB_WLAN_SCHEDULE_TBL_NUM + radio_idx, (void *)&entryNum);
			apmib_get(MIB_WLAN_SCHEDULE_MODE + radio_idx, (void *)&mode);

			if (entryNum == 0) { //only sync wlan schedule enable and mode to agent
				payload_idx = mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload_nr += 1;

				mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload = realloc(mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload, sizeof(struct wlan_schedule_info) * payload_idx);

				schedule_entry_tmp = (struct wlan_schedule_info *)&(mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload[0]) + payload_idx - 1;

				memset(schedule_entry_tmp, 0, sizeof(struct wlan_schedule_info));
				schedule_entry_tmp->wlan_Idx   = radio_idx;
				schedule_entry_tmp->enable     = enable;
				schedule_entry_tmp->entry_num  = entryNum;
				schedule_entry_tmp->mode       = mode;
 			}else {   //sync wlan schedule enable, mode, tbl_num, tbl to agent
				for (j = 0; j < entryNum; j++) {
					memset(&schedule_entry, 0, sizeof(MIB_CE_SCHEDULE_T));

					if(!mib_chain_get(MIB_WLAN_SCHEDULE_TBL + radio_idx, j, &schedule_entry)){
						printf("(%s) %d Error reading mib in map_read_specific_mib_to_config!\n",__FUNCTION__,__LINE__);
						return 1;
					}

					payload_idx = mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload_nr += 1;

					mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload = realloc(mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload, sizeof(struct wlan_schedule_info) * payload_idx);

					schedule_entry_tmp = (struct wlan_schedule_info *)&(mib_specific_data->vendor_specific_data[vendor_specific_idx-1].payload[0]) + payload_idx - 1;

					memset(schedule_entry_tmp, 0, sizeof(struct wlan_schedule_info));
					schedule_entry_tmp->wlan_Idx   = radio_idx;
					schedule_entry_tmp->enable     = enable;
					schedule_entry_tmp->entry_num  = entryNum;
					schedule_entry_tmp->mode       = mode;
					schedule_entry_tmp->eco        = schedule_entry.eco;
					schedule_entry_tmp->fTime      = schedule_entry.fTime;
					schedule_entry_tmp->tTime      = schedule_entry.tTime;
					schedule_entry_tmp->day        = schedule_entry.day;
					schedule_entry_tmp->inst_num   = schedule_entry.inst_num;
				}
			}
		}
	}
#endif

	return 0;
}
#endif
uint8_t map_read_mib_to_config(struct mib_info *mib_data)
{
	if (NULL == mib_data) {
		return 1;
	}

	int unknown_band = 0;

	const char dummy_ssid[10] = "dummyssid";
	const char dummy_key[10] = "dummykey";

	char wlan0_country_str[10] = "";
	char wlan1_country_str[10] = "";

	apmib_get(MIB_MAP_DEVICE_NAME, (void *)mib_data->device_name_buffer);

	apmib_get(MIB_MAP_CONFIGURED_BAND, (void *)&mib_data->map_configured_band);

	apmib_get(MIB_HW_REG_DOMAIN, (void *)&mib_data->reg_domain);

	apmib_get(MIB_MAP_CONTROLLER, (void *)&mib_data->map_role);

	apmib_get(MIB_OP_MODE, (void *)&mib_data->op_mode);

#ifdef RTK_MULTI_AP_R2
	apmib_get(MIB_MAP_ENABLE_VLAN, (void *)&mib_data->vlan_data.enable_vlan);

	apmib_get(MIB_MAP_PRIMARY_VID, (void *)&mib_data->vlan_data.primary_vid);
#endif

	mib_data->customize_features |= SUPPORT_WIRED_LINER_TOPOLOGY;

#ifdef MULTI_AP_ETH_ADAPTER_SUPPORT
	mib_data->customize_features |= SUPPORT_ETHERNET_ADAPTER;
#endif

#ifdef MULTI_AP_OPEN_ENCRYPT_SUPPORT
	mib_data->customize_features |= SUPPORT_OPEN_ENCRYPT;
#endif

	int i = 0, j = 0;

	size_t  base64_key_len = 0;

	if (NULL != mib_data->radio_data) {
		return 1;
	}

	for (i = 0; i < RADIO_NUMBER; i++) {
		mib_data->radio_nr += 1;
		mib_data->radio_data               = (struct radio_info *)realloc(mib_data->radio_data, sizeof(struct radio_info) * mib_data->radio_nr);
		mib_data->radio_data[i].bss_nr     = 0;
		mib_data->radio_data[i].bss_data   = NULL;
		mib_data->radio_data[i].radio_type = 0xFF;

		wlan_idx  = i;
		vwlan_idx = 0;
		
		if (!apmib_get(MIB_WLAN_CHANNEL, (void *)&mib_data->radio_data[i].radio_channel)) {
			printf("Error reading mib in update_config_file!\n");
			return 0;
		}

		if (0 == wlan_idx) {
			apmib_get(MIB_HW_COUNTRY_STR, (void *)wlan0_country_str);
		}
		if (1 == wlan_idx) {
			apmib_get(MIB_HW_COUNTRY_STR, (void *)wlan1_country_str);
		}

		if (mib_data->radio_data[i].radio_channel >= 100)
			mib_data->radio_data[i].radio_type = MAP_CONFIG_5GH;
		else if (mib_data->radio_data[i].radio_channel >= 36)
			mib_data->radio_data[i].radio_type = MAP_CONFIG_5GL;
		else if (mib_data->radio_data[i].radio_channel)
			mib_data->radio_data[i].radio_type = MAP_CONFIG_2G;
		else {
			unknown_band = 1;
		}

		if (0 == i) {
			if (!apmib_get(MIB_REPEATER_SSID1, (void *)mib_data->radio_data[i].repeater_ssid)) {
				printf("Error reading mib in update_config_file!\n");
				return 0;
			}
		} else if (1 == i) {
			if (!apmib_get(MIB_REPEATER_SSID2, (void *)mib_data->radio_data[i].repeater_ssid)) {
				printf("Error reading mib in update_config_file!\n");
				return 0;
			}
		}

		if (0 == strlen(mib_data->radio_data[i].repeater_ssid)) {
			strcpy(mib_data->radio_data[i].repeater_ssid, dummy_ssid);
		}


		if (!apmib_get(MIB_WLAN_CHANNEL_BONDING, (void *)&mib_data->radio_data[i].radio_band_width)) {
			printf("Error reading mib in update_config_file!\n");
			return 0;
		}

		mib_data->radio_data[i].repeater_ssid_base64 = map_base64_encode(mib_data->radio_data[i].repeater_ssid, strlen(mib_data->radio_data[i].repeater_ssid), &base64_key_len);

		for (j = 0; j < NUM_VWLAN_INTERFACE + 1; j++) {
			mib_data->radio_data[i].bss_nr += 1;
			mib_data->radio_data[i].bss_data = (struct bss_info *)realloc(mib_data->radio_data[i].bss_data, sizeof(struct bss_info) * mib_data->radio_data[i].bss_nr);

			vwlan_idx = j;

			if (!apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&mib_data->radio_data[i].bss_data[j].is_enabled)) {
				printf("Error reading mib in update_config_file!\n");
				return 0;
			}

			mib_data->radio_data[i].bss_data[j].is_enabled = !mib_data->radio_data[i].bss_data[j].is_enabled;


			if (!apmib_get(MIB_WLAN_SSID, (void *)mib_data->radio_data[i].bss_data[j].ssid)) {
				printf("Error reading mib in update_config_file!\n");
				return 0;
			}
			//Insert dummy data for empty SSID
			if (0 == strlen(mib_data->radio_data[i].bss_data[j].ssid)) {
				// printf("read mib of ssid is empty, set dummy value...\n");
				strcpy(mib_data->radio_data[i].bss_data[j].ssid, dummy_ssid);
			}

#ifdef RTK_MULTI_AP_R2
			if (!apmib_get(MIB_WLAN_MAP_VID, (void *)&mib_data->radio_data[i].bss_data[j].vid)) {
				printf("Error reading mib in update_config_file!\n");
				return 0;
			}
#endif
			if (!apmib_get(MIB_WLAN_MAP_BSS_TYPE, (void *)&mib_data->radio_data[i].bss_data[j].network_type)) {
				printf("Error reading mib in update_config_file!\n");
				return 0;
			}
			if (!apmib_get(MIB_WLAN_ENCRYPT, (void *)&mib_data->radio_data[i].bss_data[j].encrypt_type)) {
				printf("Error reading mib in update_config_file!\n");
				return 0;
			}
			if(mib_data->radio_data[i].bss_data[j].encrypt_type == ENCRYPT_WPA2) {
					mib_data->radio_data[i].bss_data[j].authentication_type = AUTH_WPA2;
			} else if(mib_data->radio_data[i].bss_data[j].encrypt_type == ENCRYPT_WPA) {
				mib_data->radio_data[i].bss_data[j].authentication_type = AUTH_WPA;
			} else if(mib_data->radio_data[i].bss_data[j].encrypt_type == ENCRYPT_WPA2_MIXED) {
				mib_data->radio_data[i].bss_data[j].authentication_type = (AUTH_WPA | AUTH_WPA2);
#ifdef WLAN_WPA3
			} else if(mib_data->radio_data[i].bss_data[j].encrypt_type == ENCRYPT_WPA3) {
					mib_data->radio_data[i].bss_data[j].authentication_type = AUTH_WPA3;
			} else if(mib_data->radio_data[i].bss_data[j].encrypt_type == ENCRYPT_WPA2_WPA3_MIXED) {
					mib_data->radio_data[i].bss_data[j].authentication_type = (AUTH_WPA2 | AUTH_WPA3);
#endif
			} else if(mib_data->radio_data[i].bss_data[j].encrypt_type == ENCRYPT_DISABLED) {
					mib_data->radio_data[i].bss_data[j].authentication_type = AUTH_DISABLED;
			} else {
					printf("[%s] encrypt_type: %02x, Unspport! Set as WPA2\n", __FUNCTION__, mib_data->radio_data[i].bss_data[j].encrypt_type);
					mib_data->radio_data[i].bss_data[j].authentication_type = AUTH_WPA2;
			}

			if (!apmib_get(MIB_WLAN_WPA_PSK, (void *)mib_data->radio_data[i].bss_data[j].network_key)) {
				printf("Error reading mib in update_config_file!\n");
				return 0;
			}
			//Insert dummy data for empty PSK
			if (0 == mib_data->radio_data[i].bss_data[j].encrypt_type || 0 == strlen(mib_data->radio_data[i].bss_data[j].network_key)) {
				//printf("read mib[wlan%d_vap%d] of network_key is empty, set dummy value...\n", i, j);
				strcpy(mib_data->radio_data[i].bss_data[j].network_key, dummy_key);
			}

			mib_data->radio_data[i].bss_data[j].network_key_base64 = map_base64_encode(mib_data->radio_data[i].bss_data[j].network_key, strlen(mib_data->radio_data[i].bss_data[j].network_key), &base64_key_len);
			mib_data->radio_data[i].bss_data[j].ssid_base64 = map_base64_encode(mib_data->radio_data[i].bss_data[j].ssid, strlen(mib_data->radio_data[i].bss_data[j].ssid), &base64_key_len);
		}
	}

	if (0 == strcmp(wlan0_country_str, wlan1_country_str)) {
		strcpy(mib_data->country_str, wlan0_country_str);
	}
	else {
		printf("[%s][Error]Mismatch hw_country_str between wlan0 and wlan1. Don't launch MAP daemon\n", __FUNCTION__);
		return 1;
	}

	if (unknown_band) {
#if defined(TRIBAND_SUPPORT)
		mib_data->radio_data[0].radio_type = MAP_CONFIG_5GH;
		mib_data->radio_data[1].radio_type = MAP_CONFIG_5GL;
		mib_data->radio_data[2].radio_type = MAP_CONFIG_2G;
#else
#if defined (CONFIG_BAND_5G_ON_WLAN0)
		mib_data->radio_data[0].radio_type = MAP_CONFIG_5GH;
		mib_data->radio_data[1].radio_type = MAP_CONFIG_2G;
#else
		mib_data->radio_data[0].radio_type = MAP_CONFIG_2G;
		mib_data->radio_data[1].radio_type = MAP_CONFIG_5GH;
#endif
#endif
	}

	return 0;
}

void map_fill_config_data(struct mib_info *mib_data, struct config_info **config_data, uint8_t *config_nr)
{
	int  i, j;

	for (i = 0; i < mib_data->radio_nr; i++) {
		for (j = 0; j < mib_data->radio_data[i].bss_nr; j++) {
			struct bss_info *bss_data = &mib_data->radio_data[i].bss_data[j];

			//if interface was disabled
			if (!bss_data->is_enabled && 0 == j) {
				(*config_nr)++;
				*config_data                                = (struct config_info *)realloc(*config_data, *config_nr * sizeof(struct config_info));
				(*config_data)[*config_nr - 1].config_type  = mib_data->radio_data[i].radio_type;
				(*config_data)[*config_nr - 1].ssid         = "VEVBUkRPV04=";  // "TEARDOWN"
				(*config_data)[*config_nr - 1].network_key  = "aW52YWxpZGtleQ=="; // "invalidkey";
				(*config_data)[*config_nr - 1].network_type = 0x10; // TEAR_DOWN
			}
			//if enabled
			if (bss_data->is_enabled) {
				// Skip vxd
				if (j == NUM_VWLAN_INTERFACE) {
					continue;
				}
				(*config_nr)++;
				*config_data                                = (struct config_info *)realloc(*config_data, *config_nr * sizeof(struct config_info));
				(*config_data)[*config_nr - 1].config_type  = mib_data->radio_data[i].radio_type;
				(*config_data)[*config_nr - 1].ssid         = bss_data->ssid_base64;
				(*config_data)[*config_nr - 1].network_key  = bss_data->network_key_base64;
				(*config_data)[*config_nr - 1].network_type = (unsigned char)bss_data->network_type;
				if(bss_data->encrypt_type == ENCRYPT_WPA2) {
					(*config_data)[*config_nr - 1].authentication_type = AUTH_WPA2;
				} else if(bss_data->encrypt_type == ENCRYPT_WPA) {
				(*config_data)[*config_nr - 1].authentication_type = AUTH_WPA;
				} else if(bss_data->encrypt_type == ENCRYPT_WPA2_MIXED) {
				(*config_data)[*config_nr - 1].authentication_type = (AUTH_WPA | AUTH_WPA2);
#ifdef WLAN_WPA3
				} else if(bss_data->encrypt_type == ENCRYPT_WPA3) {
					(*config_data)[*config_nr - 1].authentication_type = AUTH_WPA3;
				} else if(bss_data->encrypt_type == ENCRYPT_WPA2_WPA3_MIXED) {
					(*config_data)[*config_nr - 1].authentication_type = (AUTH_WPA2 | AUTH_WPA3);
#endif
				} else if(bss_data->encrypt_type == ENCRYPT_DISABLED) {
					(*config_data)[*config_nr - 1].authentication_type = AUTH_DISABLED;
				} else {
					printf("[%s] encrypt_type: %02x, Unspport! Set as WPA2\n", __FUNCTION__, bss_data->encrypt_type);
					(*config_data)[*config_nr - 1].authentication_type = AUTH_WPA2;
				}
			}
		}
	}
}

uint8_t map_update_config_file(struct mib_info *mib_data, struct mib_specific_info *mib_specific_data, char *config_file_path_from, char *config_file_path_to, uint8_t mode)
{
	//check for path validity
	char *ext = strrchr(config_file_path_from, '.');
	if (!ext || strcmp(ext, ".conf")) {
		printf("[CONFIG] Invalid config path: %s\n", config_file_path_from);
		return 1;
	}

	//start to read from mib for config data
	struct config_info *config_data = NULL;
	uint8_t config_nr = 0;

	if (MODE_CONTROLLER == mode) {
		map_fill_config_data(mib_data, &config_data, &config_nr);
	}

	//read the original config file for global setting
	FILE *fp = fopen(config_file_path_from, "r");
	if (fp == NULL) {
		printf("Error opening config file!\n");
		return 1;
	}

	char ** lines = NULL;
	size_t  len   = 0;
	ssize_t read;
	int     line_nr = 0;
	int     i       = 0;
	lines           = (char **) malloc(1 * sizeof(char *));
	lines[i]		= NULL;

	while ((read = getline(&lines[i], &len, fp)) != -1) {
		if ('\n' == lines[i][0]) {
			continue;
		}

		if ('[' == lines[i][0]) {
			if (0 == strncmp(lines[i], "[global]", 8)) {
				i     = 1;
				lines = (char **)realloc(lines, (i + 1) * sizeof(char *));
				lines[i]		= NULL;
				continue;
			} else {
				break;
			}
		}

		if (0 != i) {
			i++;
			lines 			= (char **)realloc(lines, (i + 1) * sizeof(char *));
			lines[i]		= NULL;
		}
	}

	line_nr = i;
	fclose(fp);
	//write info into the new config file
	fp = fopen(config_file_path_to, "w");
	for (i = 0; i < line_nr; i++) {
		fprintf(fp, "%s", lines[i]);
		free(lines[i]);
	}
	free(lines);

	char device_name_buffer[30];
	apmib_get(MIB_MAP_DEVICE_NAME, (void *)device_name_buffer);
	fprintf(fp, "%s", "device_name = ");
	fprintf(fp, "%s\n", device_name_buffer);

#if defined(TRIBAND_SUPPORT)
	fprintf(fp, "radio_name_5gh = wlan0\n");
	fprintf(fp, "radio_name_5gl = wlan1\n");
	fprintf(fp, "radio_name_24g = wlan2\n");
#else
#if 0
#if defined (WLAN1_5G_SUPPORT)
	fprintf(fp, "radio_name_5gh = wlan1\n");
	fprintf(fp, "radio_name_5gl = wlan1\n");
	fprintf(fp, "radio_name_24g = wlan0\n");
#else
	fprintf(fp, "radio_name_5gh = wlan0\n");
	fprintf(fp, "radio_name_5gl = wlan0\n");
	fprintf(fp, "radio_name_24g = wlan1\n");
#endif
#endif
#ifdef CONFIG_BAND_5G_ON_WLAN0
	fprintf(fp, "radio_name_5gh = wlan0\n");
	fprintf(fp, "radio_name_5gl = wlan0\n");
	fprintf(fp, "radio_name_24g = wlan1\n");
#else
	fprintf(fp, "radio_name_5gh = wlan1\n");
	fprintf(fp, "radio_name_5gl = wlan1\n");
	fprintf(fp, "radio_name_24g = wlan0\n");
#endif
#endif

	fprintf(fp, "vap_prefix = vap\n");

#if defined (RTK_COMMON_NETLINK)
	fprintf(fp, "common_netlink = 1\n");
#else
	fprintf(fp, "common_netlink = 0\n");
#endif

#if defined (MULTI_AP_STP_SWTICH)
	fprintf(fp, "stp_monitor_enable = 1\n");
#else
	fprintf(fp, "stp_monitor_enable = 0\n");
#endif

	/* Dot11k Beacon Request Channel
	-0:scan all channel in domain
	-1:scan all channel in channel report
	-2:scan station connection channel
	*/
	fprintf(fp, "beacon_request_ch = 0\n");

	if (MODE_WFA_TEST == mode) {
		FILE *fp_radio = fopen("/var/multiap_radio.conf", "w");
		#if defined(TRIBAND_SUPPORT)
			fprintf(fp_radio, "radio_name_5gh = wlan0\n");
			fprintf(fp_radio, "radio_name_5gl = wlan1\n");
			fprintf(fp_radio, "radio_name_24g = wlan2\n");
		#else
		#if 0
		#if defined (WLAN1_5G_SUPPORT)
			fprintf(fp_radio, "radio_name_5gh = wlan1\n");
			fprintf(fp_radio, "radio_name_5gl = wlan1\n");
			fprintf(fp_radio, "radio_name_24g = wlan0\n");
		#else
			fprintf(fp_radio, "radio_name_5gh = wlan0\n");
			fprintf(fp_radio, "radio_name_5gl = wlan0\n");
			fprintf(fp_radio, "radio_name_24g = wlan1\n");
		#endif
		#endif
		#ifdef CONFIG_BAND_5G_ON_WLAN0
		fprintf(fp_radio, "radio_name_5gh = wlan0\n");
		fprintf(fp_radio, "radio_name_5gl = wlan0\n");
		fprintf(fp_radio, "radio_name_24g = wlan1\n");
		#else
		fprintf(fp_radio, "radio_name_5gh = wlan1\n");
		fprintf(fp_radio, "radio_name_5gl = wlan1\n");
		fprintf(fp_radio, "radio_name_24g = wlan0\n");
		#endif
		#endif

		fprintf(fp_radio, "vap_prefix = vap\n\n");

		fclose(fp_radio);
	}

#ifdef RTK_MULTI_AP_R2
	int     j       = 0;
	if (MODE_CONTROLLER == mode) {
		if (mib_data->vlan_data.enable_vlan) {
			if(0 == mib_data->vlan_data.primary_vid) {
				printf("[CONFIG] Primary Vid is not configured!\n");
				return 1;
			}
			for (i = 0; i < mib_data->radio_nr; i++) {
				for (j = 0; j < mib_data->radio_data[i].bss_nr; j++) {
					if ((mib_data->radio_data[i].bss_data[j].network_type == MAP_BACKHAUL_BSS) || (mib_data->radio_data[i].bss_data[j].network_type == MAP_BACKHAUL_STA))
						continue;
					if ((1 == mib_data->radio_data[i].bss_data[j].is_enabled) && (0 == mib_data->radio_data[i].bss_data[j].vid)) {
						printf("[CONFIG] Vid of %s is not configured!\n", mib_data->radio_data[i].bss_data[j].ssid);
						return 1;
					}
				}
			}
		} else {
			printf("[CONFIG] VLAN Configuration is disabled!\n");
		}
	}

	if (-1 != access("/tmp/map_vlan_setting_configured", F_OK)) {
		FILE *vlan_fp = fopen("/tmp/map_vlan_setting_configured", "r");
		char *  vlan_line = NULL;
		size_t  vlan_len  = 0;
		ssize_t vlan_read;

		while ((vlan_read = getline(&vlan_line, &vlan_len, vlan_fp)) != -1) {
			char      ssid[64];
			int        vid = 0;
			sscanf(vlan_line, "%s %d", ssid, &vid);
			printf("SSID %s, VID: %d\n", ssid, vid);

			if (0 == strcmp(ssid, "Primary")) {
				fprintf(fp, "\n[vlan]\n");
				fprintf(fp, "primary_vid = %d\n", vid);
				break;
			}
		}

		fclose(vlan_fp);
	}
#endif

	//if it is agent or test mode device, stop here
	if (MODE_CONTROLLER != mode) {
		fclose(fp);
		return 0;
	}

	uint8_t config_number 	= 0;
	char ** ssids         			= (char **)malloc(1 * sizeof(char *));
	char ** network_keys 		 	= (char **)malloc(1 * sizeof(char *));
	uint8_t * network_types 	= (uint8_t *)malloc(1 * sizeof(uint8_t));
	uint8_t *authentication_types = (uint8_t *)malloc(1 * sizeof(uint8_t));

	for (i = 0; i < config_nr; i++) {
		if (MAP_CONFIG_2G == config_data[i].config_type) {
			config_number++;
			ssids         = (char **)realloc(ssids, (config_number) * sizeof(char *));
			network_keys  = (char **)realloc(network_keys, (config_number) * sizeof(char *));
			network_types = (uint8_t *)realloc(network_types, (config_number) * sizeof(uint8_t));
			authentication_types = (uint8_t *)realloc(authentication_types, (config_number) * sizeof(uint8_t));
			ssids[config_number - 1]         = config_data[i].ssid;
			network_keys[config_number - 1]  = config_data[i].network_key;
			network_types[config_number - 1] = config_data[i].network_type;
			authentication_types[config_number - 1] = config_data[i].authentication_type;
		}
	}

	if(config_number) {
		fprintf(fp, "[2.4g_config_data]\n");

		fprintf(fp, "number = %d\n", config_number);

		fprintf(fp, "ssid = ");
		map_write_to_config(fp, config_number, ssids);

		fprintf(fp, "network_key = ");
		map_write_to_config(fp, config_number, network_keys);

		fprintf(fp, "network_type = ");
		map_write_to_config_dec(fp, config_number, network_types);

		fprintf(fp, "authentication_type = ");
		map_write_to_config_dec(fp, config_number, authentication_types);
		if(NULL != mib_specific_data)
			map_write_vendor_data_to_config(fp, mib_specific_data, MAP_CONFIG_2G);
	}

	free(ssids);
	free(network_keys);
	free(network_types);
	free(authentication_types);

	config_number = 0;
	ssids         = (char **)malloc(1 * sizeof(char *));
	network_keys  = (char **)malloc(1 * sizeof(char *));
	network_types = (uint8_t *)malloc(1 * sizeof(uint8_t));
	authentication_types = (uint8_t *)malloc(1 * sizeof(uint8_t));

	for (i = 0; i < config_nr; i++) {
#ifdef TRIBAND_SUPPORT
		if (MAP_CONFIG_5GL == config_data[i].config_type) {
#else
		if (MAP_CONFIG_5GL == config_data[i].config_type || MAP_CONFIG_5GH == config_data[i].config_type) {
#endif
			config_number++;
			ssids         = (char **)realloc(ssids, (config_number) * sizeof(char *));
			network_keys  = (char **)realloc(network_keys, (config_number) * sizeof(char *));
			network_types = (uint8_t *)realloc(network_types, (config_number) * sizeof(uint8_t));
			authentication_types = (uint8_t *)realloc(authentication_types, (config_number) * sizeof(uint8_t));
			ssids[config_number - 1] = strdup(config_data[i].ssid);
			network_keys[config_number - 1]  = strdup(config_data[i].network_key);
			network_types[config_number - 1] = config_data[i].network_type;
			authentication_types[config_number - 1] = config_data[i].authentication_type;
		}
	}

	if(config_number) {
		fprintf(fp, "[5gl_config_data]\n");

		fprintf(fp, "number = %d\n", config_number);

		fprintf(fp, "ssid = ");
		map_write_to_config(fp, config_number, ssids);

		fprintf(fp, "network_key = ");
		map_write_to_config(fp, config_number, network_keys);

		fprintf(fp, "network_type = ");
		map_write_to_config_dec(fp, config_number, network_types);

		fprintf(fp, "authentication_type = ");
		map_write_to_config_dec(fp, config_number, authentication_types);
		if(NULL != mib_specific_data)
			map_write_vendor_data_to_config(fp, mib_specific_data, MAP_CONFIG_5GL);
	}

	free(ssids);
	free(network_keys);
	free(network_types);
	free(authentication_types);

	config_number = 0;
	ssids         = (char **)malloc(1 * sizeof(char *));
	network_keys  = (char **)malloc(1 * sizeof(char *));
	network_types = (uint8_t *)malloc(1 * sizeof(uint8_t));
	authentication_types = (uint8_t *)malloc(1 * sizeof(uint8_t));

	for (i = 0; i < config_nr; i++) {
#ifdef TRIBAND_SUPPORT
		if (MAP_CONFIG_5GH == config_data[i].config_type) {
#else
		if (MAP_CONFIG_5GL == config_data[i].config_type || MAP_CONFIG_5GH == config_data[i].config_type) {
#endif
			config_number++;
			ssids         = (char **)realloc(ssids, (config_number) * sizeof(char *));
			network_keys  = (char **)realloc(network_keys, (config_number) * sizeof(char *));
			network_types = (uint8_t *)realloc(network_types, (config_number) * sizeof(uint8_t));
			authentication_types = (uint8_t *)realloc(authentication_types, (config_number) * sizeof(uint8_t));
			ssids[config_number - 1] = strdup(config_data[i].ssid);
			network_keys[config_number - 1]  = strdup(config_data[i].network_key);
			network_types[config_number - 1] = config_data[i].network_type;
			authentication_types[config_number - 1] = config_data[i].authentication_type;
		}
	}

	if(config_number) {
		fprintf(fp, "[5gh_config_data]\n");

		fprintf(fp, "number = %d\n", config_number);

		fprintf(fp, "ssid = ");
		map_write_to_config(fp, config_number, ssids);

		fprintf(fp, "network_key = ");
		map_write_to_config(fp, config_number, network_keys);

		fprintf(fp, "network_type = ");
		map_write_to_config_dec(fp, config_number, network_types);

		fprintf(fp, "authentication_type = ");
		map_write_to_config_dec(fp, config_number, authentication_types);

		if(NULL != mib_specific_data)
			map_write_vendor_data_to_config(fp, mib_specific_data, MAP_CONFIG_5GH);
	}

	free(ssids);
	free(network_keys);
	free(network_types);
	free(authentication_types);

#ifdef RTK_MULTI_AP_R2
	if (mib_data->vlan_data.enable_vlan) {
		fprintf(fp, "[vlan_detail]\n");

		fprintf(fp, "ssid = ");
		map_write_to_config(fp, mib_data->vlan_data.vssid_number, mib_data->vlan_data.vssids);

		fprintf(fp, "vid = ");
		map_write_to_config_dec_int(fp, mib_data->vlan_data.vssid_number, mib_data->vlan_data.vids);

		free(mib_data->vlan_data.vssids);
		free(mib_data->vlan_data.vids);
	}
#endif

	fclose(fp);
	return 0;
}

uint8_t map_write_mib_data(struct mib_info *mib_data, char *file_path)
{
	//check for path validity
	char *ext = strrchr(file_path, '.');
	if (!ext || strcmp(ext, ".conf")) {
		printf("[CONFIG] Invalid config path: %s\n", file_path);
		return 1;
	}

	FILE *fp = fopen(file_path, "w");

	fprintf(fp, "[global]\n");

	fprintf(fp, "%s", "configured_band = ");
	fprintf(fp, "%d\n", mib_data->map_configured_band);

	fprintf(fp, "%s", "hw_reg_domain = ");
	fprintf(fp, "%d\n", mib_data->reg_domain);

	fprintf(fp, "%s", "hw_country_str = ");
	fprintf(fp, "%s\n", mib_data->country_str);

	fprintf(fp, "%s", "customize_features = ");
	fprintf(fp, "%d\n", mib_data->customize_features);

	int i = 0, j = 0;
	for (i = 0; i < mib_data->radio_nr; i++) {
		if (MAP_CONFIG_2G == mib_data->radio_data[i].radio_type) {
			fprintf(fp, "[mib_info_2.4g]\n");
		} else if (MAP_CONFIG_5GH == mib_data->radio_data[i].radio_type) {
			fprintf(fp, "[mib_info_5gh]\n");
		} else if (MAP_CONFIG_5GL == mib_data->radio_data[i].radio_type) {
			fprintf(fp, "[mib_info_5gl]\n");
		} else {
			continue;
		}

		fprintf(fp, "channel = %d\n", mib_data->radio_data[i].radio_channel);
		fprintf(fp, "channel_bandwidth = %d\n", mib_data->radio_data[i].radio_band_width);
#ifdef UNIVERSAL_REPEATER
		fprintf(fp, "repeater_ssid = %s\n", mib_data->radio_data[i].repeater_ssid_base64);
#endif
		fprintf(fp, "bss_number = %d\n", mib_data->radio_data[i].bss_nr);

		char **        ssids         = (char **)malloc((mib_data->radio_data[i].bss_nr) * sizeof(char *));
		char **        network_keys  = (char **)malloc((mib_data->radio_data[i].bss_nr) * sizeof(char *));
		uint8_t *network_types = (uint8_t *)malloc((mib_data->radio_data[i].bss_nr) * sizeof(uint8_t));
		uint8_t *is_enableds   = (uint8_t *)malloc((mib_data->radio_data[i].bss_nr) * sizeof(uint8_t));
		uint8_t *encrypt_types = (uint8_t *)malloc((mib_data->radio_data[i].bss_nr) * sizeof(uint8_t));
		uint8_t *authentication_types = (uint8_t *)malloc((mib_data->radio_data[i].bss_nr) * sizeof(uint8_t));

		for (j = 0; j < mib_data->radio_data[i].bss_nr; j++) {
			ssids[j]         = mib_data->radio_data[i].bss_data[j].ssid_base64;
			network_keys[j]  = mib_data->radio_data[i].bss_data[j].network_key_base64;
			network_types[j] = mib_data->radio_data[i].bss_data[j].network_type;
			is_enableds[j]   = mib_data->radio_data[i].bss_data[j].is_enabled;
			encrypt_types[j] = mib_data->radio_data[i].bss_data[j].encrypt_type;
			authentication_types[j] = mib_data->radio_data[i].bss_data[j].authentication_type;
		}
		fprintf(fp, "ssid = ");
		map_write_to_config(fp, mib_data->radio_data[i].bss_nr, ssids);
		fprintf(fp, "network_key = ");
		map_write_to_config(fp, mib_data->radio_data[i].bss_nr, network_keys);
		fprintf(fp, "network_type = ");
		map_write_to_config_dec(fp, mib_data->radio_data[i].bss_nr, network_types);
		fprintf(fp, "is_enabled = ");
		map_write_to_config_dec(fp, mib_data->radio_data[i].bss_nr, is_enableds);
		fprintf(fp, "encrypt_type = ");
		map_write_to_config_dec(fp, mib_data->radio_data[i].bss_nr, encrypt_types);
		fprintf(fp, "authentication_type = ");
		map_write_to_config_dec(fp, mib_data->radio_data[i].bss_nr, authentication_types);

		free(ssids);
		free(network_keys);
		free(network_types);
		free(is_enableds);
		free(encrypt_types);
		free(authentication_types);
	}

	fclose(fp);
	return 0;
}
#ifdef RTK_MULTI_AP_R2
uint8_t map_vlan_mib_to_file(struct mib_info *mib_data)
{
	int i = 0, j = 0, k = 0;
	mib_data->vlan_data.vssid_number = 0;
	mib_data->vlan_data.vssids       = NULL;
	mib_data->vlan_data.vids         = NULL;
	char ** decoded_ssids            = NULL;

	for (i = 0; i < mib_data->radio_nr; i++) {
		for (j = 0; j < mib_data->radio_data[i].bss_nr; j++) {
			//remove duplicate ssid and vid
			for (k = 0; k < mib_data->vlan_data.vssid_number; k++) {
				if (!(strcmp(mib_data->vlan_data.vssids[k], mib_data->radio_data[i].bss_data[j].ssid_base64)) && (mib_data->vlan_data.vids[k] == mib_data->radio_data[i].bss_data[j].vid))
					break;
			}

			if ((mib_data->radio_data[i].bss_data[j].network_type == MAP_BACKHAUL_BSS) || (mib_data->radio_data[i].bss_data[j].network_type == MAP_BACKHAUL_STA) || !(mib_data->radio_data[i].bss_data[j].is_enabled))
				continue;

			if (k == mib_data->vlan_data.vssid_number) {
				mib_data->vlan_data.vssids      = (char **)realloc(mib_data->vlan_data.vssids, (mib_data->vlan_data.vssid_number + 1) * sizeof(char *));
				decoded_ssids      = (char **)realloc(decoded_ssids, (mib_data->vlan_data.vssid_number + 1) * sizeof(char *));
				mib_data->vlan_data.vids        = (int *)realloc(mib_data->vlan_data.vids, (mib_data->vlan_data.vssid_number + 1) * sizeof(int));
				mib_data->vlan_data.vssids[mib_data->vlan_data.vssid_number]        = strdup(mib_data->radio_data[i].bss_data[j].ssid_base64);
				decoded_ssids[mib_data->vlan_data.vssid_number]        = strdup(mib_data->radio_data[i].bss_data[j].ssid);
				mib_data->vlan_data.vids[mib_data->vlan_data.vssid_number]          = mib_data->radio_data[i].bss_data[j].vid;
				mib_data->vlan_data.vssid_number ++;
			}
		}
	}

	FILE *fp = fopen("/tmp/map_vlan_setting", "w");
	fprintf(fp, "Primary %d\n", mib_data->vlan_data.primary_vid);
	for (i = 0; i < mib_data->vlan_data.vssid_number; i++) {
		printf("VLAN %d %s %d\n", i, decoded_ssids[i], mib_data->vlan_data.vids[i]);
		fprintf(fp, "%s %d\n", decoded_ssids[i], mib_data->vlan_data.vids[i]);
	}
	fclose(fp);

	system("echo 1 > /tmp/map_vlan_need_reset");

	if(decoded_ssids)
		free(decoded_ssids);
	return 0;
}
#endif
void map_free_mib_data(struct mib_info *mib_data)
{
	int i = 0, j = 0;
	for (i = 0; i < mib_data->radio_nr; i++) {
		for (j = 0; j < mib_data->radio_nr; j++) {
			if(mib_data->radio_data[i].bss_data[j].ssid_base64)
				free(mib_data->radio_data[i].bss_data[j].ssid_base64);
			if(mib_data->radio_data[i].bss_data[j].network_key_base64)
				free(mib_data->radio_data[i].bss_data[j].network_key_base64);
		}
		if(mib_data->radio_data[i].bss_data)
			free(mib_data->radio_data[i].bss_data);
#ifdef WLAN_UNIVERSAL_REPEATER
		if(mib_data->radio_data[i].repeater_ssid_base64)
			free(mib_data->radio_data[i].repeater_ssid_base64);
#endif
	}
	if(mib_data->radio_data)
		free(mib_data->radio_data);
}

void map_free_specific_mib_data(struct mib_specific_info *mib_specific_data)
{
	int i = 0;

	for(i = 0; i < mib_specific_data->vendor_specific_nr; i++){
		if(mib_specific_data->vendor_specific_data[i].payload)
			free(mib_specific_data->vendor_specific_data[i].payload);
	}
	if(mib_specific_data->vendor_specific_data)
		free(mib_specific_data->vendor_specific_data);
}

#if defined(WLAN_MULTI_AP_ROLE_AUTO_SELECT)
int map_parse_easymesh_log_file(char *filename)
{
	int ret=0;
	FILE *fp=NULL;
	char buffer[256]={0};

	if(filename == NULL)
		return ret;

	if((fp = fopen(filename,"r")) == NULL)
		return ret;	
	
	while(fgets(buffer, sizeof(buffer), fp)) {
		if(strstr(buffer, "CONTROLLER_FOUND")){
			ret = 1;
			break;
		}
	}
	fclose(fp);

	return ret;
}

int map_link_parse_ping_file(char *filename)
{
	int ret=0;
	FILE *fp=NULL;
	char buffer[200]={0};

	if(filename == NULL)
		return ret;

	if((fp = fopen(filename,"r")) == NULL)
		return ret;	

	while(fgets(buffer, sizeof(buffer), fp)) {
		if(strstr(buffer, "1 packets transmitted, 1 packets received")){
			ret = 1;
			break;
		}		
	}
	fclose(fp);
	unlink(filename);

	return ret;
}

int map_checkping(char *server1)
{
	int ret=0;
	char buffer[128]={0};
	
	snprintf(buffer,sizeof(buffer),"ping %s -c 1 2> %s > %s",server1, "/var/tmp/tmp_ping", "/var/tmp/tmp_ping");	
	ret=system(buffer);	
	if(ret){
		if(!map_link_parse_ping_file("/var/tmp/tmp_ping"))
			return 0;
	}			
	return 1;
}
int map_getDefaultRoute(char *interface)
{
	char buff[1024], iface[16];
	char gate_addr[128], net_addr[128], mask_addr[128];
	int num, iflags, metric, refcnt, use, mss, window, irtt;
	FILE *fp = fopen(CHECK_ROUTE_FILE, "r");
	char *fmt;
	int found=0;
	unsigned long addr;

	if (!fp) {
       	printf("ERROR (%s)%d Open %s file error.\n", __FUNCTION__, __LINE__, CHECK_ROUTE_FILE);
		return 0;
    }

	fmt = "%16s %128s %128s %X %d %d %d %128s %d %d %d";

	while (fgets(buff, 1023, fp)) {
		num = sscanf(buff, fmt, iface, net_addr, gate_addr,
		     		&iflags, &refcnt, &use, &metric, mask_addr, &mss, &window, &irtt);
		if (num < 10 || !(iflags & 0x0001) || !(iflags & 0x0002) || strcmp(iface, interface))
	    		continue;

		found = 1;
		break;
	}

    fclose(fp);
    return found;
}
 
int map_is_controller_or_agent(void)
{
	int check_file_times=0, is_controller_found=0;
	int eStatus=0, LogicPort=0, phyPort=0, ethportstatus=0; 
	char ifname[16]={0};
	int wisp_wan_id=0, isGatewanExist=0, isPingCheckPass=0;
	unsigned char mibVal, opmode=0;
#ifdef CONFIG_USER_DHCPCLIENT_MODE
	unsigned char value[64] = {0};
	int dhcpc_pid = -1;
	unsigned char orig_dhcp_mode = 0, dhcp_mode = 0;
#endif
#ifdef CONFIG_AUTO_DHCP_CHECK
	const char AUTO_DHCPPID[] = "/var/run/auto_dhcp.pid";
#endif

	apmib_get(MIB_OP_MODE, (void *)&opmode);
    if(opmode==GATEWAY_MODE){
        snprintf(ifname, sizeof(ifname), "%s", WAN_IF);
    } else if(opmode==WISP_MODE){
        snprintf(ifname, sizeof(ifname), VXD_IF, wisp_wan_id);
    } else if(opmode == BRIDGE_MODE) {
        snprintf(ifname, sizeof(ifname), "%s", BR_IF);
	}

#ifdef CONFIG_USER_DHCPCLIENT_MODE
	apmib_get( MIB_DHCP, (void *)&orig_dhcp_mode);
	dhcp_mode = orig_dhcp_mode;
#endif

	for(check_file_times=0;check_file_times<=ROLE_QUERY_TIME;check_file_times++){
		if(map_parse_easymesh_log_file(ROLE_QUERY_FILE)){
			is_controller_found = 1;
			break;
		}else{
			for(LogicPort = 0; LogicPort < ELANVIF_NUM; LogicPort++){
				phyPort = rtk_port_get_lan_phyID(LogicPort);
				rtk_port_link_get(phyPort, &eStatus);
				if(eStatus == 1){
					ethportstatus = 1;
					break;
				}
			}
			if(ethportstatus){
				isGatewanExist = map_getDefaultRoute(ifname);
				if(isGatewanExist){
					isPingCheckPass = map_checkping(PING_PUB_IP_ADDR);
				}
			}
		}
		sleep(ROLE_QUERY_INTERVAL);
	}

#if defined (WLAN1_5G_SUPPORT)
	int radio_index = 1;
#else
	int radio_index = 0;
#endif // WLAN1_5G_SUPPORT
	char backbaul_link[4]={0};
	snprintf(backbaul_link, sizeof(backbaul_link), "%d", radio_index);
	
	if(is_controller_found){
		printf("[%s %d] Easymesh controller found, auto select role as agent!\n",__FUNCTION__,__LINE__);
		mibVal = MAP_AGENT;
		setupMultiAPAgent("agent", "auto", backbaul_link, &dhcp_mode);
	}else if(!is_controller_found && (ethportstatus && isGatewanExist && isPingCheckPass)){
		printf("[%s %d] Easymesh controller not found, auto select role as controller!\n",__FUNCTION__,__LINE__);
		mibVal = MAP_CONTROLLER;
		setupMultiAPController("controller", "auto", backbaul_link, &dhcp_mode);
	}else{
		mibVal = MAP_AUTO;
		setupMultiAPAuto("auto", "auto", backbaul_link, &dhcp_mode);
	}
	//apmib_set(MIB_MAP_CONTROLLER, (void *)&mibVal);

#ifdef CONFIG_AUTO_DHCP_CHECK
	kill_by_pidfile_new((const char*)AUTO_DHCPPID, SIGTERM);
#endif

#ifdef CONFIG_USER_DHCPCLIENT_MODE
	if(orig_dhcp_mode != dhcp_mode){
		if(orig_dhcp_mode == DHCPV4_LAN_CLIENT){
			snprintf(value, 64, "%s.%s", (char*)DHCPC_PID, ALIASNAME_BR0);
			dhcpc_pid = read_pid((char*)value);
			if (dhcpc_pid > 0)
				kill(dhcpc_pid, SIGTERM);
		}

		if( dhcp_mode == DHCPV4_LAN_CLIENT ){
			setupDHCPClient();
#if defined(CONFIG_RTK_L34_ENABLE)
			if( dhcp_mode == DHCP_LAN_CLIENT ){
				va_cmd(IFCONFIG, 6, 1, (char*)LANIF, "0.0.0.0", "netmask", "255.255.255.0", "mtu", "1500");
				RG_reset_LAN();
			}
#endif
		}else{
			/* recover LAN IP address */
			restart_lanip();
		}
		restart_dhcp();
	}
#endif

#ifdef CONFIG_AUTO_DHCP_CHECK
	if(opmode == BRIDGE_MODE && dhcp_mode == AUTO_DHCPV4_BRIDGE)
	{	
		va_cmd("/bin/Auto_DHCP_Check", 8, 0, "-a", "15", "-n", "5", "-d", "30", "-m", "10");
		start_process_check_pidfile((const char*)AUTO_DHCPPID);
	}
#endif

#ifdef COMMIT_IMMEDIATELY
 	 Commit();
#endif

	if(mibVal == MAP_CONTROLLER){
		config_WLAN(ACT_RESTART, CONFIG_SSID_ALL);
	}

}
#endif
void rtk_stop_multi_ap_service()
{
	//reset rtw_map_user_pid to 0 through mib multiap_ext_cmd
	//avoid code dump due to error pid num in core_map_nl_event_send() 
#ifdef IS_AX_SUPPORT
	system("iwpriv wlan0 set_mib multiap_ext_cmd=1");
	system("iwpriv wlan1 set_mib multiap_ext_cmd=1");
#endif

	if (find_pid_by_name("map_checker") > 0) {
		system("killall -9 map_checker >/dev/null 2>&1");
	}

	if (find_pid_by_name("map_controller") > 0) {
		system("killall -9 map_controller >/dev/null 2>&1");
	}

	if (find_pid_by_name("map_agent") > 0) {
		system("killall -9 map_agent >/dev/null 2>&1");
	}

#ifdef WLAN_CTC_MULTI_AP
	if (find_pid_by_name("map_agent_ctc") > 0) {
		system("killall -9 map_agent_ctc >/dev/null 2>&1");
	}
#endif

	if (find_pid_by_name("map_controller_test") > 0) {
		system("killall -9 map_controller_test >/dev/null 2>&1");
	}

	if (find_pid_by_name("map_agent_test") > 0) {
		system("killall -9 map_agent_test >/dev/null 2>&1");
	}

	system("rm /tmp/map_*.txt >/dev/null 2>&1");
	system("rm /tmp/topology_json >/dev/null 2>&1");
	system("rm /tmp/map_agent_sta_info/*.info >/dev/null 2>&1");
#ifdef RTK_HAPD_MULTI_AP
	system("rm -f /var/wps_gpio");
#endif
}

void setup_multi_ap_acl(void)
{
	unsigned char acl_cmd[256] = {0};
	int map_state = 0;

	apmib_get(MIB_MAP_CONTROLLER, (void *)&map_state);

	if(0 == map_state)
		return 0;

	//Process all 1905 pkts with software.
	snprintf(acl_cmd, sizeof(acl_cmd), "%s", "echo config 1 in > /proc/rtl865x/acl");
	system(acl_cmd);
	memset(acl_cmd, 0, sizeof(acl_cmd));
	snprintf(acl_cmd, sizeof(acl_cmd), "%s", "echo reg br0 -40000 in > /proc/rtl865x/acl");
	system(acl_cmd);
	memset(acl_cmd, 0, sizeof(acl_cmd));
	snprintf(acl_cmd, sizeof(acl_cmd), "%s", "echo add netif br0 chain -40000 act toCPU type mac rule ethType 893a FFFF dmac 0180C2000000 FFFFFF000000 > /proc/rtl865x/acl");
	system(acl_cmd);
	memset(acl_cmd, 0, sizeof(acl_cmd));
	snprintf(acl_cmd, sizeof(acl_cmd), "%s", "echo reg br0 -40001 in > /proc/rtl865x/acl");
	system(acl_cmd);
	memset(acl_cmd, 0, sizeof(acl_cmd));
	snprintf(acl_cmd, sizeof(acl_cmd), "%s", "echo add netif br0 chain -40001 act permit type srcfilter rule srcPhyport 100 proto l2 > /proc/rtl865x/acl");
	system(acl_cmd);
}

int rtk_setup_multi_ap_block(uint8_t map_state)
{
	int ret = 0;

	//Add ebtables rules.
	ret |= RunSystemCmd(NULL_FILE, "ebtables", "-F", "wlan_map_block", NULL_STR);

	ret |= RunSystemCmd(NULL_FILE, "ebtables", "-N", "wlan_map_block", "-P", "RETURN", NULL_STR);
	ret |= RunSystemCmd(NULL_FILE, "ebtables", "-D", "FORWARD", "-j", "wlan_map_block", NULL_STR);
	ret |= RunSystemCmd(NULL_FILE, "ebtables", "-A", "FORWARD", "-j", "wlan_map_block", NULL_STR);

	ret |= RunSystemCmd(NULL_FILE, "ebtables", "-N", "map_portmapping", "-P", "RETURN", NULL_STR);
	ret |= RunSystemCmd(NULL_FILE, "ebtables", "-D", "FORWARD", "-j", "map_portmapping", NULL_STR);
	ret |= RunSystemCmd(NULL_FILE, "ebtables", "-A", "FORWARD", "-j", "map_portmapping", NULL_STR);

	if (map_state) {
		//ebtables -A FORWARD  -d 01:80:c2:00:00:13 -j DROP
		ret |= RunSystemCmd(NULL_FILE, "ebtables", "-A", "wlan_map_block", "-d", "01:80:c2:00:00:13", "-j", "DROP", NULL_STR);

		if(MAP_CONTROLLER == map_state) {
			ret |= RunSystemCmd(NULL_FILE, "brctl", "stp", "br0", "on", NULL_STR);
			ret |= RunSystemCmd(NULL_FILE, "brctl", "setbridgeprio", "br0", "0", NULL_STR);
		} else if (MAP_AGENT == map_state) {
			ret |= RunSystemCmd(NULL_FILE, "brctl", "stp", "br0", "on", NULL_STR);
			ret |= RunSystemCmd(NULL_FILE, "brctl", "setbridgeprio", "br0", "61440", NULL_STR);
		}
	} else {
		ret |= RunSystemCmd(NULL_FILE, "brctl", "setbridgeprio", "br0", "32768", NULL_STR);
	}
	setup_multi_ap_acl();
	return ret;
}
#if 0
void map_get_ext_conf_data(struct map_ext_conf *map_ext_data, uint8_t        role)
{
	int i=0;
	MIB_CE_MBSSIB_T Entry;

	for (i = 0; i < NUM_WLAN_INTERFACE; i++) {
		if(role == MAP_CONTROLLER){
			mib_chain_local_mapping_get(MIB_MBSSIB_TBL, i, 1, (void *)&Entry);
#ifdef IS_AX_SUPPORT
			if(Entry.multiap_bss_type == MAP_BACKHAUL_BSS)
#else
			if(Entry.wlanDisabled == 0 && Entry.wsc_disabled == 0 && Entry.multiap_bss_type == MAP_BACKHAUL_BSS )
#endif
			{
				map_ext_data->backhaul_radio_index = i;
			}
		}else if(role == MAP_AGENT || role == MAP_AUTO){
			mib_chain_local_mapping_get(MIB_MBSSIB_TBL, i, WLAN_REPEATER_ITF_INDEX, (void *)&Entry);
#ifdef IS_AX_SUPPORT
			if(Entry.multiap_bss_type == MAP_BACKHAUL_STA)
#else
			if(Entry.wlanDisabled == 0 && Entry.wsc_disabled == 0 && Entry.multiap_bss_type == MAP_BACKHAUL_STA)
#endif
			{
				map_ext_data->backhaul_radio_index = i;
			}
		}
	}

#ifdef BACKHAUL_REAL_TIME_SWITCH
	if(role == MAP_AGENT || role == MAP_AUTO){
		map_ext_data->backhaul_switch = 1;
		map_ext_data->check_interval = 5;
		map_ext_data->link_down_threshold = 1;
		map_ext_data->discovery_down_threshold = 1;
	}
#endif

#ifdef ARP_LOOP_DETECTION
	if(role == MAP_AGENT || role == MAP_AUTO){
		map_ext_data->arp_loop = 1;
	}
#endif

	return;
}

void map_write_ext_conf_data(struct map_ext_conf *map_ext_data, char *file_path)
{
	//check for path validity
	char *ext = strrchr(file_path, '.');
	if (!ext || strcmp(ext, ".conf")) {
		printf("[CONFIG] Invalid config path: %s\n", file_path);
		return;
	}

	FILE *fp = fopen(file_path, "w");

	if (fp == NULL) {
		printf("ERROR (%s)%d Open %s file error.\n", __FUNCTION__, __LINE__, file_path);
		return;
	}

	fprintf(fp, "[global]\n");

	fprintf(fp, "%s", "backhaul_radio = ");
	fprintf(fp, "%d\n", map_ext_data->backhaul_radio_index);

	fprintf(fp, "%s", "backhaul_switch = ");
	fprintf(fp, "%d\n", map_ext_data->backhaul_switch);

	fprintf(fp, "%s", "check_interval = ");
	fprintf(fp, "%d\n", map_ext_data->check_interval);

	fprintf(fp, "%s", "link_down_threshold = ");
	fprintf(fp, "%d\n", map_ext_data->link_down_threshold);

	fprintf(fp, "%s", "discovery_down_threshold = ");
	fprintf(fp, "%d\n", map_ext_data->discovery_down_threshold);

	fprintf(fp, "%s", "arp_loop = ");
	fprintf(fp, "%d\n", map_ext_data->arp_loop);

	fclose(fp);
	return;
}
#endif


int rtk_start_multi_ap_service(void)
{
	unsigned int        map_state = 0;

	uint8_t        map_kill_count = 0;

	//set default name for the map device if device_name is empty
	char map_device_name[30] = {0};

	unsigned int map_log_level        = 0;
	int     i                    = 0;
	char    log_level_string[10] = { 0 };
	char    cmd[128]             = { 0 };

	printf("[%s:%d] sun_test \n", __FUNCTION__, __LINE__);

#ifdef WLAN_DUALBAND_CONCURRENT
	int orig_wlan_idx = wlan_idx;
	wlan_idx = 0;
#endif

	int status = 0xFF;

	int multiap_profile = 1;

#ifdef RTK_MULTI_AP_R2
	multiap_profile = 2;
#endif

#ifdef RTK_HAPD_MULTI_AP
	system("echo 0 > /var/wps_gpio");
#endif
	system("mkdir /tmp/map_agent_sta_info/");

	rtk_stop_multi_ap_service();

	apmib_get(MIB_MAP_DEVICE_NAME, (void *)map_device_name);

	apmib_get(MIB_MAP_CONTROLLER, (void *)&map_state);

	apmib_get(MIB_MAP_LOG_LEVEL, (void *)&map_log_level);

	rtk_setup_multi_ap_block(map_state);

#ifndef CONFIG_USER_RTK_SYSLOG
	if (0 == find_pid_by_name("syslogd")) {
		system("syslogd >/dev/null 2>&1");
	}
#endif

	if(0 == map_state){
		printf("[%s:%d] sun_test  map_state = %d \n", __FUNCTION__, __LINE__,map_state);
		return 0;
	}

	struct mib_info mib_data                   = {0};
	struct mib_specific_info mib_specific_data = {0};
	struct map_ext_conf map_ext_data           = {0};

	mib_data.radio_data                        = NULL;
	mib_specific_data.vendor_specific_data     = NULL;
	map_ext_data.backhaul_radio_index          = -1;

	if (map_read_mib_to_config(&mib_data))
		return 1;

	map_write_mib_data(&mib_data, "/var/multiap_mib.conf");
#if 0
	map_get_ext_conf_data(&map_ext_data, map_state);
	map_write_ext_conf_data(&map_ext_data, MAP_EXT_DATA);
#endif
	for(i = 0; i < map_log_level; i++) {
		strcat(log_level_string, "v");
	}
	
#ifdef RTK_MULTI_AP_R2
	if(-1 != access("/tmp/map_vlan_reset", F_OK) && -1 != access("/tmp/map_vlan_need_reset", F_OK)) {
		system("sh /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_need_reset");
	}
	
	if((1 == map_state) && (mib_data.vlan_data.enable_vlan)) {
		map_vlan_mib_to_file(&mib_data);
	}

	if(-1 != access("/tmp/map_vlan_setting", F_OK)) {
		printf("Configuring VLAN......\n");
		system("map_vlan_config");
	}
#endif

#if 1
	switch(map_state){
		case MAP_CONTROLLER:{
			if (!strcmp(map_device_name, "")) {
				char* default_name_controller = "EasyMesh_Controller";
				// set into mib
				if (!apmib_set(MIB_MAP_DEVICE_NAME, (void *)default_name_controller)) {
					printf("[Error] : Failed to set AP mib MIB_MAP_DEVICE_NAME\n");
					return 1;
				}
			}
#if 0
			map_read_specific_mib_to_config(&mib_specific_data);
#endif
		//	map_update_config_file(&mib_data, &mib_specific_data, "/etc/multiap.conf", "/var/multiap.conf", MODE_CONTROLLER);
			map_update_config_file(&mib_data, NULL, "/etc/multiap.conf", "/var/multiap.conf", MODE_CONTROLLER);


			while (find_pid_by_name("map_controller") > 0) {
				if(map_kill_count > 10) {
					break;
				}
				system("killall -9 map_controller >/dev/null 2>&1");
				usleep(100 * 1000);
				map_kill_count++;
			}
#ifndef CONFIG_USER_RTK_MULTI_AP_CT_TEST
			sprintf(cmd, "/bin/map_controller -d%s -p%d > /dev/console &", log_level_string, multiap_profile);
			status = system(cmd);
			printf("Multi AP controller daemon is running with %d\n", status);

			sprintf(cmd, "map_checker -%s -p%d &", log_level_string, multiap_profile);
			status = system(cmd);
			printf("Multi AP checker is running with %d\n", status);
#endif
			break;
		}
		case MAP_AGENT:{
			if (!strcmp(map_device_name, "")) {
				char* default_name_agent = "EasyMesh_Agent";
				// set into mib
				if (!apmib_set(MIB_MAP_DEVICE_NAME, (void *)default_name_agent)) {
					printf("[Error] : Failed to set AP mib MIB_MAP_DEVICE_NAME\n");
					return 1;
				}
			}
			//map_update_config_file(&mib_data, &mib_specific_data, "/etc/multiap.conf", "/var/multiap.conf", MODE_AGENT);
			map_update_config_file(&mib_data, NULL, "/etc/multiap.conf", "/var/multiap.conf", MODE_AGENT);
			while (find_pid_by_name("map_agent") > 0) {
				if(map_kill_count > 10) {
					break;
				}
				system("killall -9 map_agent >/dev/null 2>&1");
				usleep(100 * 1000);
				map_kill_count++;
			}

#ifndef CONFIG_USER_RTK_MULTI_AP_CT_TEST

#ifdef WLAN_CTC_MULTI_AP
			sprintf(cmd, "map_agent_ctc -d%s > /dev/console", log_level_string);
#else
			sprintf(cmd, "map_agent -d%s -p%d > /dev/console", log_level_string, multiap_profile);
#endif
			status = system(cmd);
			printf("Multi AP agent daemon is running with %d\n", status);

			sprintf(cmd, "map_checker -%s -p%d", log_level_string, multiap_profile);
			status = system(cmd);
			printf("Multi AP checker is running with %d\n", status);
#endif
			break;
		}
#if defined(WLAN_MULTI_AP_ROLE_AUTO_SELECT)
		case MAP_AUTO:{
			//start map_agent first
			if (!strcmp(map_device_name, "")) {
				char* default_name_agent = "EasyMesh_Agent";
				// set into mib
				if (!apmib_set(MIB_MAP_DEVICE_NAME, (void *)default_name_agent)) {
					printf("[Error] : Failed to set AP mib MIB_MAP_DEVICE_NAME\n");
					return 1;
				}
			}
			mib_data.map_role = MAP_AGENT;
			map_update_config_file(&mib_data, &mib_specific_data, "/etc/multiap.conf", "/var/multiap.conf", MODE_AGENT);
			while (find_pid_by_name("map_agent") > 0) {
				if(map_kill_count > 10) {
					break;
				}
				system("killall -9 map_agent >/dev/null 2>&1");
				usleep(100 * 1000);
				map_kill_count++;
			}
#ifdef WLAN_CTC_MULTI_AP
			sprintf(cmd, "map_agent_ctc -d%s > /dev/console", log_level_string);
#else
			sprintf(cmd, "map_agent -d%s -p%d > /dev/console", log_level_string, multiap_profile);
#endif
			status = system(cmd);
			printf("Multi AP agent daemon is running with %d\n", status);

			sprintf(cmd, "map_checker -%s -p%d", log_level_string, multiap_profile);
			status = system(cmd);
			printf("Multi AP checker is running with %d\n", status);
			mib_data.map_role = MAP_AUTO;
			//do role selection
			int pid, pid2;
			
			pid = fork();
			if(pid == 0){
				pid2 = fork();
				if(pid2 == 0){
					map_is_controller_or_agent();
					exit(0);
				}else if(pid2>0){
					exit(0);
				}
			}else if(pid>0){
				waitpid(pid, NULL, 0);
			}
			break;
		}
#endif
		case MAP_CONTROLLER_WFA:
		case MAP_CONTROLLER_WFA_R2: {
#ifdef RTK_MULTI_AP_WFA
			system("ifconfig eth0 up; brctl addif br0 eth0");
#endif
			if (!strcmp(map_device_name, "")) {
				char* default_name_agent = "EasyMesh_Test_Controller";
				// set into mib
				if (!apmib_set(MIB_MAP_DEVICE_NAME, (void *)default_name_agent)) {
					printf("[Error] : Failed to set AP mib MIB_MAP_DEVICE_NAME\n");
					return 1;
				}
			}
			map_update_config_file(&mib_data, &mib_specific_data, "/etc/multiap.conf", "/var/multiap.conf", MODE_WFA_TEST);
			while (find_pid_by_name("map_controller") > 0) {
				if(map_kill_count > 10) {
					break;
				}
				system("killall -9 map_controller > /dev/null 2>&1");
				usleep(100 * 1000);
				map_kill_count++;
			}
#ifdef RTK_MULTI_AP_R2
			if(MAP_CONTROLLER_WFA_R2 == map_state) {
				sprintf(cmd, "map_controller -dt%s -p2 > /dev/console", log_level_string);
			} else {
				sprintf(cmd, "map_controller -dt%s -p1 > /dev/console", log_level_string);
			}
#else
			sprintf(cmd, "map_controller -dt%s > /dev/console", log_level_string);
#endif
			status = system(cmd);
			printf("Multi AP controller logo test daemon is running with %d\n", status);

			break;
		}
		case MAP_AGENT_WFA:
		case MAP_AGENT_WFA_R2:{
#ifdef RTK_MULTI_AP_WFA
			if(-1 == access("/tmp/map_backhaul_teardown", F_OK)) {
				system("ifconfig wlan0-vxd down; ifconfig wlan1-vxd down; ifconfig wlan2-vxd down; ifconfig eth0 down > /dev/null 2>&1");
				system("echo 1 > /tmp/map_backhaul_teardown");
				break;
			}
#endif
			if (!strcmp(map_device_name, "")) {
				char* default_name_agent = "EasyMesh_Test_Agent";
				// set into mib
				if (!apmib_set(MIB_MAP_DEVICE_NAME, (void *)default_name_agent)) {
					printf("[Error] : Failed to set AP mib MIB_MAP_DEVICE_NAME\n");
					return 1;
				}
			}
			map_update_config_file(&mib_data, &mib_specific_data, "/etc/multiap.conf", "/var/multiap.conf", MODE_WFA_TEST);
			while (find_pid_by_name("map_agent") > 0) {
				if(map_kill_count > 10) {
					break;
				}
				system("killall -9 map_agent > /dev/null 2>&1");
				usleep(100 * 1000);
				map_kill_count++;
			}
			if(-1 == access("/tmp/map_lock", F_OK)) {
#ifdef RTK_MULTI_AP_R2
				if(MAP_AGENT_WFA_R2 == map_state) {
					sprintf(cmd, "map_agent -dt%s -p2 > /dev/console", log_level_string);
				} else {
					sprintf(cmd, "map_agent -dt%s -p1 > /dev/console", log_level_string);
				}
#else
				sprintf(cmd, "map_agent -dt%s > /dev/console", log_level_string);
#endif
				status = system(cmd);
				printf("Multi AP agent logo test daemon is running with %d\n", status);
			}
			break;
		}
	}
#endif
	map_free_mib_data(&mib_data);
//	map_free_specific_mib_data(&mib_specific_data);


#ifdef WLAN_DUALBAND_CONCURRENT
	wlan_idx = orig_wlan_idx;
#endif

	return 0;
}

#if 0
void map_restart_wlan()
{
	if(-1 != access("/tmp/map_vlan_reset", F_OK) && -1 != access("/tmp/map_vlan_need_reset", F_OK)) {
		system("sh /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_need_reset");
	}
	config_WLAN(ACT_RESTART, CONFIG_SSID_ALL);
}
#endif

