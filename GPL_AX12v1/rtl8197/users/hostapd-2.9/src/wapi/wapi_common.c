#include "wapi_common.h"
#include "ap/hostapd.h"
#include "ap/wpa_auth.h"
#include "ap/wpa_auth_i.h"
#include "l2_packet/l2_packet.h"


extern int wpa_write_wapi_ie(struct wpa_auth_config *conf, u8 *buf, size_t len);

void rtl_dump_buffer(u8 *buff, int len, char *str)
{
	int i;
	
	if (str)
		printf("\n######Dump %s (len = %d):\n", str, len);

	for (i = 0; i < len; i++) {
		if (i && (i % 16 == 0))
			printf("\n");

		printf("%02x ", buff[i]);
	}
	printf("\n"); 
}

struct hostapd_data *rtl_get_hostapd_data_by_mac(const u8 *ap_mac)
{
	struct hapd_interfaces *hapd_iface = (struct hapd_interfaces *)rtl_get_eloop_ctx();
	if (hapd_iface == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_get_eloop_ctx() fail.\n", __FUNCTION__);
		return NULL;
	}

	struct hostapd_data *hapd = rtl_get_hostapd_data(hapd_iface, ap_mac);
	if (hapd == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call hostapd_get_iface() fail.\n", __FUNCTION__);
		return NULL;
	}

	return hapd;
}

int rtl_wapi_action_deauth(const u8 *ap_mac, const u8 *sta_mac, int reason)
{
	struct hostapd_data *hapd = rtl_get_hostapd_data_by_mac(ap_mac);
	if (hapd == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_get_hostapd_data_by_mac() fail.\n", __FUNCTION__);
		return -1;
	}

	return hostapd_drv_sta_deauth(hapd, sta_mac, reason);
}


int rtl_wapi_set_usk(const u8 *ap_mac, const u8 *sta_mac, int key_idx, const u8 *key, int key_len)
{	
	enum wpa_alg alg = WPA_ALG_SMS4;

	wpa_printf(MSG_DEBUG, "RT_WAPI_SET_USK: Installing USK to the driver.");

	struct hostapd_data *hapd = rtl_get_hostapd_data_by_mac(ap_mac);
	if (hapd == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_get_hostapd_data_by_mac() fail.", __FUNCTION__);
		return -1;
	}

	if (hapd->conf->rsn_pairwise == WPA_CIPHER_SMS4)
		alg = WPA_ALG_SMS4;
	else if (hapd->conf->rsn_pairwise == WPA_CIPHER_GCM_SM4)
		alg = WPA_ALG_GCM_SM4;

	rtl_dump_buffer((u8 *)key, key_len, "unicast key");

	if (hostapd_drv_set_key(hapd->conf->iface, hapd, alg, sta_mac, key_idx, 1, NULL, 0, key, key_len) < 0) {
		wpa_printf(MSG_ERROR, "Failed to set USK to the driver.");
		return -1;
	}

	wpa_printf(MSG_DEBUG, "WAPI: succeeded to set USK to the driver.");
	
	return 0;
}

int rtl_wapi_set_msk(const u8 *ap_mac, int key_idx, const u8* iv, int iv_len, const u8* key, int key_len)
{
	enum wpa_alg alg = WPA_ALG_SMS4;

	wpa_printf(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct hostapd_data *hapd = rtl_get_hostapd_data_by_mac(ap_mac);
	if (hapd == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_get_hostapd_data_by_mac() fail.\n", __FUNCTION__);
		return -1;
	}

	if (hapd->conf->rsn_pairwise == WPA_CIPHER_SMS4)
		alg = WPA_ALG_SMS4;
	else if (hapd->conf->rsn_pairwise == WPA_CIPHER_GCM_SM4)
		alg = WPA_ALG_GCM_SM4;

	rtl_dump_buffer((u8 *)key, key_len, "multicast key");

	//if (hostapd_drv_set_key(hapd->conf->iface, hapd, alg, broadcast_ether_addr, key_idx, 1, iv, iv_len, key, key_len) < 0) {
	
	if (hostapd_drv_set_key(hapd->conf->iface, hapd, alg, broadcast_ether_addr, key_idx, 1, NULL, 0, key, key_len) < 0) {
		wpa_printf(MSG_ERROR, "Failed to set MSK to the driver.");
		return -1;
	}

	wpa_printf(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);

	return 0;
}


void rtl_wapi_receive_wai_packet(void *ctx, const u8 *src_addr, const u8 *buf, size_t len)
{
	wpa_printf(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct hostapd_data *hapd = (struct hostapd_data *)ctx;
	struct l2_ethhdr *l2_hdr = (struct l2_ethhdr *)buf;

	if (memcmp(hapd->own_addr, l2_hdr->h_dest, ETH_ALEN) != 0)
		return;
	if (l2_hdr->h_proto != htons(ETH_P_WAI))
		return;
	if (hapd->conf->wpa != WPA_PROTO_WAPI)
		return;
	
    wpa_printf(MSG_DEBUG, "%s, received %u bytes packet\n", __FUNCTION__, len);

    rtl_wapi_wai_receive(buf, len);
}

int rtl_wapi_init_wai_socket(struct hostapd_data *hapd)
{
	wpa_printf(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	l2_packet_deinit(hapd->l2_wapi);

	hapd->l2_wapi = l2_packet_init(hapd->conf->iface, hapd->own_addr, ETH_P_WAI,
							rtl_wapi_receive_wai_packet, hapd, 1);

	if (hapd->l2_wapi == NULL) {
		wpa_printf(MSG_ERROR, "%s, Failed to init hapd->l2_wapi.\n", __FUNCTION__);
		return -1;
	}

	wpa_printf(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);

	return 0;
}

int rtl_wapi_send_wai_packet(u8 *pbuf, int length)
{
	wpa_printf(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	struct l2_ethhdr *l2_hdr = (struct l2_ethhdr *)pbuf;

	struct hostapd_data *hapd = rtl_get_hostapd_data_by_mac((const u8 *)l2_hdr->h_source);
	if (hapd == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_get_hostapd_data_by_mac() fail.\n", __FUNCTION__);
		return -1;
	}

	wpa_printf(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);

	return l2_packet_send(hapd->l2_wapi, l2_hdr->h_dest, ETH_P_WAI, (const u8*)pbuf, (size_t)length);
}

int rtl_wapi_config_init(struct hostapd_data *hapd, struct wapi_config *config)
{
	int len;

	snprintf(config->ifname, sizeof(config->ifname), "%s", hapd->conf->iface);
	config->wapi_role = hapd->conf->wapi_role;
	config->asu_ip = hapd->conf->asu_ip;

	if (hapd->conf->wpa_key_mgmt == WPA_KEY_MGMT_WAPI_CERT && config->wapi_role != WAPI_AE && config->wapi_role != WAPI_AE_ASU) {
		wpa_printf(MSG_ERROR, "%s, Invalid wapi role.", __FUNCTION__);
		return -1;
	}

	if (config->wapi_role == WAPI_AE && hapd->conf->wpa_key_mgmt == WPA_KEY_MGMT_WAPI_CERT && config->asu_ip.s_addr <= 0) {
		wpa_printf(MSG_ERROR, "%s, Invalid asu ip.", __FUNCTION__);
		return -1;
	}

	if (hapd->conf->rsn_pairwise == WPA_CIPHER_SMS4)
		config->crypto_alg = WPI_SM4;
	else if (hapd->conf->rsn_pairwise == WPA_CIPHER_GCM_SM4)
		config->crypto_alg = WPI_GCM_SM4;
	else 
		return -1;

	config->key_update_time = hapd->conf->key_update_time;

	if (config->key_update_time < MIN_KEY_UPDATE_TIME || config->key_update_time > MAX_KEY_UPDATE_TIME)
		config->key_update_time = DEF_KEY_UPDATE_TIME;

	if ((len = wpa_write_wapi_ie(&hapd->wpa_auth->conf , config->wapi_ie, MAX_WAPI_IE_LEN)) < 0) {
		wpa_printf(MSG_ERROR, "%s, Invalid wapi IE", __FUNCTION__);
		return -1;
	}
	config->wapi_ie_len = len;

	if (hapd->conf->wpa_key_mgmt == WPA_KEY_MGMT_WAPI_PSK) {
		config->auth_type = WAPI_PSK;

		if (hapd->conf->ssid.wpa_passphrase_set && hapd->conf->ssid.wpa_passphrase) {
			config->para.kt = KEY_TYPE_ASCII;
			config->para.kl = strlen(hapd->conf->ssid.wpa_passphrase);
			snprintf(config->para.kv, sizeof(config->para.kv), "%s", hapd->conf->ssid.wpa_passphrase);
		} else if (hapd->conf->ssid.wpa_psk_set && hapd->conf->ssid.wpa_psk) {
			config->para.kt = KEY_TYPE_HEX;
			config->para.kl = PMK_LEN;
			memcpy(config->para.kv, hapd->conf->ssid.wpa_psk->psk, PMK_LEN);
		} else {
			wpa_printf(MSG_ERROR, "%s, Invalid PSK.", __FUNCTION__);
			return -1;
		}		
	}
	else if (hapd->conf->wpa_key_mgmt == WPA_KEY_MGMT_WAPI_CERT) {
		config->auth_type = WAPI_CERT;

		if (!hapd->conf->wapi_ca_cert || !hapd->conf->wapi_ae_cert) {
			return -1;
		}

		snprintf(config->para.ca_cert_file, CERT_FILE_NAME_LEN, "%s", hapd->conf->wapi_ca_cert);
		snprintf(config->para.ae_cert_file, CERT_FILE_NAME_LEN, "%s", hapd->conf->wapi_ae_cert);

		if (hapd->conf->wapi_asu_cert)
			snprintf(config->para.asu_cert_file, CERT_FILE_NAME_LEN, "%s", hapd->conf->wapi_asu_cert);
		else 
			snprintf(config->para.asu_cert_file, CERT_FILE_NAME_LEN, "%s", hapd->conf->wapi_ca_cert);
	} else {
		wpa_printf(MSG_ERROR, "%s, Invalid wapi auth type.", __FUNCTION__);
		return -1;
	}

	return 0;
}

int rtl_wapi_init(struct hostapd_data *hapd)
{
	wpa_printf(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	int ret = -1;
	struct wapi_config *config = NULL;
	
	if (rtl_wapi_init_wai_socket(hapd) < 0) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_wapi_init_wai_socket() fail.\n", __FUNCTION__);
		goto exit;
	}

	config = (struct wapi_config *)malloc(sizeof(struct wapi_config));
	if (config == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call malloc() fail.", __FUNCTION__);
		goto exit;
	}
	
	if (rtl_wapi_config_init(hapd, config) < 0) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_wapi_config_init() fail.", __FUNCTION__);
		goto exit;
	}

	if (rtl_wapi_ae_init(hapd->own_addr, config) < 0) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_wapi_config_init() fail.", __FUNCTION__);
		goto exit;
	}
	
	ret = 0;
	
exit:

	if (ret < 0)
		l2_packet_deinit(hapd->l2_wapi);
	
	if (config)
		free(config);

	wpa_printf(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}





















































