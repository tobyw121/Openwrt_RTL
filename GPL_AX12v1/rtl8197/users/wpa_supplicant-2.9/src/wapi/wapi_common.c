#include "wapi_common.h"
#include "l2_packet/l2_packet.h"
#include "config.h"
#include "wpa_supplicant_i.h"
#include "driver_i.h"



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

struct wpa_supplicant *rtl_get_wpa_supplicant_by_mac(const u8 *mac)
{
	struct wpa_global *global = (struct wpa_global *)rtl_get_eloop_ctx();
	if (global == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_get_eloop_ctx() fail.\n", __FUNCTION__);
		return NULL;
	}

	struct wpa_supplicant *wpa_s = rtl_get_wpa_supplicant(global, mac);
	if (wpa_s == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_get_wpa_supplicant() fail.\n", __FUNCTION__);
		return NULL;
	}

	return wpa_s;
}

int rtl_wapi_set_usk(const u8 *ae_mac, const u8 *sta_mac, int key_idx, const u8 *key, int key_len)
{	
	enum wpa_alg alg = WPA_ALG_SMS4;

	wpa_printf(MSG_DEBUG, "RT_WAPI_SET_USK: Installing USK to the driver.");

	struct wpa_supplicant *wpa_s = rtl_get_wpa_supplicant_by_mac(sta_mac);	
	if (wpa_s == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_get_wpa_supplicant_by_mac() fail.", __FUNCTION__);
		return -1;
	}

	if (wpa_s->pairwise_cipher == WPA_CIPHER_SMS4)
		alg = WPA_ALG_SMS4;
	else if (wpa_s->pairwise_cipher == WPA_CIPHER_GCM_SM4)
		alg = WPA_ALG_GCM_SM4;

	rtl_dump_buffer((u8 *)key, key_len, "unicast key");

	if (wpa_drv_set_key(wpa_s, alg, wpa_s->bssid, key_idx, 1, NULL, 0, key, key_len) < 0) {
		wpa_printf(MSG_ERROR, "Failed to set USK to the driver.");
		return -1;
	}

	wpa_printf(MSG_DEBUG, "WAPI: succeeded to set USK to the driver.");
	
	return 0;
}

int rtl_wapi_set_msk(const u8 *sta_mac, int key_idx, const u8* iv, int iv_len, const u8* key, int key_len)
{
	enum wpa_alg alg = WPA_ALG_SMS4;

	wpa_printf(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct wpa_supplicant *wpa_s = rtl_get_wpa_supplicant_by_mac(sta_mac);
	if (wpa_s == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_get_wpa_supplicant_by_mac() fail.\n", __FUNCTION__);
		return -1;
	}

	if (wpa_s->group_cipher == WPA_CIPHER_SMS4)
		alg = WPA_ALG_SMS4;
	else if (wpa_s->group_cipher == WPA_CIPHER_GCM_SM4)
		alg = WPA_ALG_GCM_SM4;

	rtl_dump_buffer((u8 *)key, key_len, "multicast key");
	
	if (wpa_drv_set_key(wpa_s, alg, broadcast_ether_addr, key_idx, 1, NULL, 0, key, key_len) < 0) {
		wpa_printf(MSG_ERROR, "Failed to set MSK to the driver.");
		return -1;
	}
	
	wpa_printf(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);

	return 0;
}


void rtl_wapi_receive_wai_packet(void *ctx, const u8 *src_addr, const u8 *buf, size_t len)
{
	wpa_printf(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct wpa_supplicant *wpa_s = (struct wpa_supplicant *)ctx;
	struct l2_ethhdr *l2_hdr = (struct l2_ethhdr *)buf;

	if (memcmp(wpa_s->own_addr, l2_hdr->h_dest, ETH_ALEN) != 0)
		return;
	if (l2_hdr->h_proto != htons(ETH_P_WAI))
		return;
	if (wpa_s->wpa_proto != WPA_PROTO_WAPI)
		return;
	
    wpa_printf(MSG_DEBUG, "%s, received %u bytes packet\n", __FUNCTION__, len);

    rtl_wapi_wai_receive(buf, len);
}

int rtl_wapi_init_wai_socket(struct wpa_supplicant *wpa_s)
{
	wpa_printf(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	l2_packet_deinit(wpa_s->l2_wapi);
	wpa_s->l2_wapi = NULL;

	wpa_printf(MSG_DEBUG, "%s: ifname = %s own_addr = " MACSTR, __FUNCTION__, wpa_s->ifname, MAC2STR(wpa_s->own_addr));

	wpa_s->l2_wapi = l2_packet_init(wpa_s->ifname, wpa_s->own_addr, ETH_P_WAI,
							rtl_wapi_receive_wai_packet, wpa_s, 1);

	if (wpa_s->l2_wapi == NULL) {
		wpa_printf(MSG_ERROR, "%s, Failed to init wpa_s->l2_wapi.\n", __FUNCTION__);
		return -1;
	}

	wpa_printf(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);

	return 0;
}

int rtl_wapi_send_wai_packet(u8 *pbuf, int length)
{
	wpa_printf(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	struct l2_ethhdr *l2_hdr = (struct l2_ethhdr *)pbuf;

	struct wpa_supplicant *wpa_s = rtl_get_wpa_supplicant_by_mac((const u8 *)l2_hdr->h_source);
	if (wpa_s == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_get_wpa_supplicant_by_mac() fail.\n", __FUNCTION__);
		return -1;
	}

	wpa_printf(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);

	return l2_packet_send(wpa_s->l2_wapi, l2_hdr->h_dest, ETH_P_WAI, (const u8*)pbuf, (size_t)length);
}

int rtl_wapi_config_init(struct wpa_supplicant *wpa_s, struct wapi_config *config)
{
	int len;

	snprintf(config->ifname, sizeof(config->ifname), "%s", wpa_s->ifname);
	memcpy(config->bssid, wpa_s->bssid, ETH_ALEN);
	memcpy(config->own_addr, wpa_s->own_addr, ETH_ALEN);

	if (wpa_s->pairwise_cipher == WPA_CIPHER_SMS4)
		config->crypto_alg = WPI_SM4;
	else if (wpa_s->pairwise_cipher == WPA_CIPHER_GCM_SM4)
		config->crypto_alg = WPI_GCM_SM4;
	else
		return -1;

	if (wpa_s->assoc_wapi_ie_len) {
		memcpy(config->asue_wapi_ie, wpa_s->assoc_wapi_ie, wpa_s->assoc_wapi_ie_len);
		config->asue_wapi_ie_len = wpa_s->assoc_wapi_ie_len;
	}

	if (wpa_s->ap_wapi_ie_len) {
		memcpy(config->ae_wapi_ie, wpa_s->ap_wapi_ie, wpa_s->ap_wapi_ie_len);
		config->ae_wapi_ie_len = wpa_s->ap_wapi_ie_len;
	}

	if (wpa_s->conf == NULL || wpa_s->conf->ssid == NULL) {
		wpa_printf(MSG_ERROR, "%s, wpa_s->conf or wpa_s->conf->ssid is NULL", __FUNCTION__);
		return -1;
	}

	if (wpa_s->key_mgmt == WPA_KEY_MGMT_WAPI_PSK) {
		config->auth_type = WAPI_PSK;

		if (wpa_s->conf->ssid->passphrase) {
			config->para.kt = KEY_TYPE_ASCII;
			config->para.kl = strlen(wpa_s->conf->ssid->passphrase);
			snprintf(config->para.kv, sizeof(config->para.kv), "%s", wpa_s->conf->ssid->passphrase);
			
			wpa_printf(MSG_DEBUG, "%s: passphrase = %s", __FUNCTION__, config->para.kv);
		} else if (wpa_s->conf->ssid->psk_set) {
			config->para.kt = KEY_TYPE_HEX;
			config->para.kl = PMK_LEN;
			memcpy(config->para.kv, wpa_s->conf->ssid->psk, PMK_LEN);
		} else {
			wpa_printf(MSG_ERROR, "%s, Invalid PSK.", __FUNCTION__);
			return -1;
		}		
	}
	else if (wpa_s->key_mgmt == WPA_KEY_MGMT_WAPI_CERT) {
		config->auth_type = WAPI_CERT;

		if (!wpa_s->conf->ssid->wapi_ca_cert || !wpa_s->conf->ssid->wapi_asue_cert) {
			return -1;
		}

		snprintf(config->para.ca_cert_file, CERT_FILE_NAME_LEN, "%s", wpa_s->conf->ssid->wapi_ca_cert);
		snprintf(config->para.asue_cert_file, CERT_FILE_NAME_LEN, "%s", wpa_s->conf->ssid->wapi_asue_cert);

		if (wpa_s->conf->ssid->wapi_asu_cert)
			snprintf(config->para.asu_cert_file, CERT_FILE_NAME_LEN, "%s", wpa_s->conf->ssid->wapi_asu_cert);
		else 
			snprintf(config->para.asu_cert_file, CERT_FILE_NAME_LEN, "%s", wpa_s->conf->ssid->wapi_ca_cert);
	} else {
		wpa_printf(MSG_ERROR, "%s, Invalid wapi auth type.", __FUNCTION__);
		return -1;
	}

	return 0;
}

int rtl_wapi_init(struct wpa_supplicant *wpa_s)
{
	int ret = -1;
	struct wapi_config *config = NULL;

	wpa_printf(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	if (wpa_s == NULL) {
		wpa_printf(MSG_DEBUG, "%s: wpa_s is NULL", __FUNCTION__);
		return ret;
	}

	if (rtl_wapi_init_wai_socket(wpa_s) < 0) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_wapi_init_wai_socket() fail.\n", __FUNCTION__);
		goto exit;
	}

	config = (struct wapi_config *)malloc(sizeof(struct wapi_config));
	if (config == NULL) {
		wpa_printf(MSG_ERROR, "%s, Call malloc() fail.", __FUNCTION__);
		goto exit;
	}
	
	if (rtl_wapi_config_init(wpa_s, config) < 0) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_wapi_config_init() fail.", __FUNCTION__);
		goto exit;
	}

	if (rtl_wapi_asue_init(config) < 0) {
		wpa_printf(MSG_ERROR, "%s, Call rtl_wapi_asue_init() fail.", __FUNCTION__);
		goto exit;
	}	
	
	ret = 0;
	
exit:

	if (ret < 0) {
		l2_packet_deinit(wpa_s->l2_wapi);
		wpa_s->l2_wapi = NULL;
	}
	
	if (config)
		free(config);

	wpa_printf(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}

