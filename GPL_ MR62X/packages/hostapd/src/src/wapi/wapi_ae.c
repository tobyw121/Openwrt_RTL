#include <net/if.h>
#include <linux/sockios.h>

#include "wapi_ae.h"
#include "l2_packet/l2_packet.h"
#include "wapi_alogrithm.h"
#include "x509.h"

struct wapi_ae_st *s_ae[MAX_AP_IFACE];
struct wapi_asue_st *s_asue[MAX_STA_NUM];

static int debug_level = MSG_DEBUG;
extern int ecc192_genkey(unsigned char *priv_key, unsigned char *pub_key);
extern int ecc192_ecdh(const unsigned char *priv_key, const unsigned char *pub_key,unsigned char *ecdhkey);

void rtl_wapi_trace(int level, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (level >= debug_level) {
		vprintf(fmt, ap);
		printf("\n");
	}
	va_end(ap);
}

void rtl_wapi_deauth(struct wapi_asue_st *wapi_asue)
{
	wapi_asue->wapi_state = WAPI_IDLE;
	if (memcmp(wapi_asue->ae_mac, "\x00\x00\x00\x00\x00\x00", ETH_ALEN) && 
		memcmp(wapi_asue->asue_mac, "\x00\x00\x00\x00\x00\x00", ETH_ALEN)) 
		rtl_wapi_action_deauth(wapi_asue->ae_mac, wapi_asue->asue_mac, 0);

		rtl_wapi_asue_free(&wapi_asue);
}

void rtl_wapi_deauth_all(void)
{
	int i;
	struct wapi_asue_st *wapi_asue = NULL;
	
	for (i = 0; i < MAX_STA_NUM; i++) {	
		wapi_asue = s_asue[i];

		if (!wapi_asue)
			continue;

		rtl_wapi_deauth(wapi_asue);
	}	
}

void rtl_wapi_process_assoc_event(u8 *ae_mac, u8 *sta_mac, u8 *wapi_ie, u8 ie_len)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	int idx = rtl_get_asue_index(sta_mac);
	if (idx < 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: get asue index fail", __FUNCTION__);
		return;
	}
	if (s_asue[idx])
		rtl_wapi_asue_free(&s_asue[idx]);

	s_asue[idx] = (struct wapi_asue_st *)rtl_allocate_buffer(sizeof(struct wapi_asue_st));
	if (s_asue[idx] == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s(): allocate asue fail", __FUNCTION__);
		return;
	}	

	if (rtl_wapi_asue_init(ae_mac, sta_mac, wapi_ie, ie_len, s_asue[idx]) < 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: asue init fail", __FUNCTION__);
		return;
	}
	struct wapi_asue_st *wapi_asue = s_asue[idx];	
	
	struct wapi_ae_st *wapi_ae = rtl_get_ae_by_mac(ae_mac);
	if (wapi_ae == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: wapi_ae :" MACSTR "is NULL", __FUNCTION__, MAC2STR(ae_mac));
		return;
	}	

	rtl_wapi_trace(MSG_DEBUG, "%s: WAPI auth type is %s", __FUNCTION__, (wapi_ae->auth_type == WAPI_CERT) ? "WAPI_CERT" : "WAPI_PSK");
	
	if (wapi_ae->auth_type == WAPI_CERT) {
		//send authenticate active packet
		rtl_wapi_send_auth_active_pkt(wapi_ae, wapi_asue);
	} else if (wapi_ae->auth_type == WAPI_PSK) {
		//generate bk and bkid
		rtl_wapi_bk_derivation(wapi_ae->psk_infos->kv , wapi_ae->psk_infos->kl, wapi_asue);
		rtl_wapi_send_usk_agreement_request_pkt(wapi_ae, wapi_asue);
	}

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);
}

void rtl_wapi_process_disassoc_event(u8 *ae_mac, u8 *sta_mac)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	struct wapi_ae_st *wapi_ae = rtl_get_ae_by_mac(ae_mac);
	struct wapi_asue_st *wapi_asue = rtl_get_asue_by_mac(sta_mac);

	if (!wapi_ae || !wapi_asue) {
		rtl_wapi_trace(MSG_ERROR, "%s: get wapi_ae or wapi_asue fail", __FUNCTION__);
		return;
	}

	rtl_wapi_deauth(wapi_asue);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);
}

void rtl_wapi_process_usk_update_event(u8 *sta_mac)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	struct wapi_asue_st *wapi_asue = rtl_get_asue_by_mac(sta_mac);
	if (!wapi_asue) {
		rtl_wapi_trace(MSG_ERROR, "%s: get wapi_asue fail", __FUNCTION__);
		return;
	}

	struct wapi_ae_st *wapi_ae = rtl_get_ae_by_mac(wapi_asue->ae_mac);
	if (!wapi_ae) {
		rtl_wapi_trace(MSG_ERROR, "%s: get wapi_ae fail", __FUNCTION__);
		return;
	}

	rtl_wapi_update_usk(wapi_ae, wapi_asue);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);
}

void rtl_wapi_process_msk_update_event(u8 *ae_mac)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);	

	struct wapi_ae_st *wapi_ae = rtl_get_ae_by_mac(ae_mac);
	if (!wapi_ae) {
		rtl_wapi_trace(MSG_ERROR, "%s: get wapi_ae fail", __FUNCTION__);
		return;
	}

	rtl_wapi_update_msk(wapi_ae);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);
}

int rtl_get_ae_index(u8 *ae_mac)
{
	int i;
	struct wapi_ae_st *wapi_ae = NULL;
	
	for (i = 0; i < MAX_AP_IFACE; i++) {
		wapi_ae = s_ae[i];
		if (wapi_ae && memcmp(wapi_ae->ae_mac, ae_mac, ETH_ALEN) == 0)
			break;
	}
	if (i < MAX_AP_IFACE)
		return i;

	for (i = 0; i < MAX_AP_IFACE; i++) {
		wapi_ae = s_ae[i];
		if (wapi_ae == NULL)
			break;
	}
	if (i < MAX_AP_IFACE)
		return i;

	return -1;	
}

struct wapi_ae_st *rtl_get_ae_by_mac(u8 *ae_mac)
{
	int idx = rtl_get_ae_index(ae_mac);

	if (idx < 0)
		return NULL;

	return s_ae[idx];	
}

int rtl_get_asue_index(u8 *sta_mac)
{
	int i;
	struct wapi_asue_st *wapi_asue = NULL;
	
	for (i = 0; i < MAX_STA_NUM; i++) {
		wapi_asue = s_asue[i];
		if (wapi_asue && memcmp(wapi_asue->asue_mac, sta_mac, ETH_ALEN) == 0)
			break;
	}
	if (i < MAX_STA_NUM)
		return i;

	for (i = 0; i < MAX_STA_NUM; i++) {
		wapi_asue = s_asue[i];
		if (wapi_asue == NULL)
			break;
	}
	if (i < MAX_STA_NUM)
		return i;

	return -1;	
}

struct wapi_asue_st *rtl_get_asue_by_mac(u8 *sta_mac)
{
	int idx = rtl_get_asue_index(sta_mac);

	if (idx < 0)
		return NULL;

	return s_asue[idx];	
}

int rtl_wapi_asue_init(u8 *ae_mac, u8 *sta_mac, u8 *wapi_ie, u8 ie_len, struct wapi_asue_st *wapi_asue)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	int ret = -1;
	struct wapi_asue_st *pasue = wapi_asue;

	if (!ae_mac || !sta_mac || !wapi_asue) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid arguments", __FUNCTION__);
		return ret;
	}	

	memcpy(pasue->asue_mac, sta_mac, ETH_ALEN);
	memcpy(pasue->ae_mac, ae_mac, ETH_ALEN);

	if (pasue->wapi_ie)
		rtl_free_buffer(pasue->wapi_ie, pasue->wapi_ie_len);
	pasue->wapi_ie = (u8 *)rtl_allocate_buffer(MAX_WAPI_IE_LEN);
	if (!pasue->wapi_ie) {
		rtl_wapi_trace(MSG_ERROR, "%s(): allocate wapi_ie fail", __FUNCTION__);
		return ret;
	}
	memcpy(pasue->wapi_ie, wapi_ie, ie_len);
	pasue->wapi_ie_len = ie_len;
	//rtl_dump_buffer(wapi_ie, ie_len, "WAPI IE from ASUE associate request");


	//init usksa
	pasue->usksa = (struct wapi_usksa *)rtl_allocate_buffer(sizeof(struct wapi_usksa));
	if (!pasue->usksa) {
		rtl_wapi_trace(MSG_ERROR, "%s(): allocate usksa fail", __FUNCTION__);
		return ret;
	}

	//init bksa
	pasue->bksa = (struct wapi_bksa_cache *)rtl_allocate_buffer(sizeof(struct wapi_bksa_cache));
	if (!pasue->bksa) {
		rtl_wapi_trace(MSG_ERROR, "%s(): allocate bksa fail", __FUNCTION__);
		return ret;
	}		

	ret = 0;

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d) <<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}

int rtl_wapi_sock_close(struct wapi_ae_st *wapi_ae)
{
	if (wapi_ae->sock) {
		if (wapi_ae->sock->sockfd > 0)
			close(wapi_ae->sock->sockfd);
		
		wapi_ae->sock = rtl_free_buffer(wapi_ae->sock, sizeof(wai_socket));
	}
	
	return 0;
}

int rtl_wapi_sock_init(struct wapi_config *config, struct wapi_ae_st *wapi_ae)
{
	int ret = -1;
	struct sockaddr_in servaddr;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd <= 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: create socket fail", __FUNCTION__);
		goto exit;
	}
	
	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(WAPI_ASU_PORT);
	servaddr.sin_addr = config->asu_ip;

	rtl_wapi_trace(MSG_ERROR, "%s: asu_ip = %s", __FUNCTION__, inet_ntoa(config->asu_ip));

	if (wapi_ae->sock)
		rtl_wapi_sock_close(wapi_ae);
	
	wapi_ae->sock = rtl_allocate_buffer(sizeof(wai_socket));
	if (!wapi_ae->sock) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate wapi sock fail", __FUNCTION__);
		goto exit;
	}	

	wapi_ae->sock->sockfd = fd;
	memcpy(&wapi_ae->sock->servaddr, &servaddr, sizeof(struct sockaddr_in));

	ret = 0;
	
exit:
	if (ret < 0) {
		if (fd > 0)
			close(fd);

		rtl_wapi_sock_close(wapi_ae);
	}

	return ret;
}

int rtl_wapi_ae_init(u8 *ae_mac, struct wapi_config *config)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	int ret = -1;
	int idx = rtl_get_ae_index(ae_mac);
	if (idx < 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: get ae index fail", __FUNCTION__);
		return ret;
	}
	struct wapi_ae_st *wapi_ae = s_ae[idx];

	if (wapi_ae)
		rtl_wapi_ae_free(&wapi_ae);

	wapi_ae = (struct wapi_ae_st *)rtl_allocate_buffer(sizeof(struct wapi_ae_st));
	if (wapi_ae == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s(): allocate ae fail", __FUNCTION__);
		goto exit;
	}
	s_ae[idx] = wapi_ae;

	snprintf(wapi_ae->ifname, sizeof(wapi_ae->ifname), "%s", config->ifname);
	memcpy(wapi_ae->ae_mac, ae_mac, ETH_ALEN);
	wapi_ae->auth_type = config->auth_type;
	wapi_ae->wapi_role = config->wapi_role;
	wapi_ae->crypto_alg = config->crypto_alg;
	wapi_ae->key_update_time = config->key_update_time;

	rtl_wapi_trace(MSG_DEBUG, "%s: ifname = %s ae_mac = " MACSTR "auth_type = %d wapi_role = %d crypto_alg = %d", 
		 		__FUNCTION__, wapi_ae->ifname, MAC2STR(wapi_ae->ae_mac), wapi_ae->auth_type, wapi_ae->wapi_role, wapi_ae->crypto_alg);

	memcpy(wapi_ae->wapi_ie, config->wapi_ie, config->wapi_ie_len);
	wapi_ae->wapi_ie_len = config->wapi_ie_len;

	if (wapi_ae->wapi_role == WAPI_AE && 
		wapi_ae->auth_type == WAPI_CERT && 
		rtl_wapi_sock_init(config, wapi_ae)) {
		rtl_wapi_trace(MSG_ERROR, "%s(): init wapi socket fail", __FUNCTION__);
		goto exit;
	}

	if (wapi_ae->auth_type == WAPI_CERT) {
		char *ca_cert_file = config->para.ca_cert_file;
		char *asu_cert_file = config->para.asu_cert_file;
		char *ae_cert_file = config->para.ae_cert_file;
		ret = rtl_wapi_cert_init(wapi_ae, ca_cert_file, asu_cert_file, ae_cert_file);
		if (ret < 0)
			goto exit;

		rtl_wapi_ecdh_init(&wapi_ae->ecdh);
		rtl_wapi_sign_alg_init(&wapi_ae->sign_alg);
		
	} else if (wapi_ae->auth_type == WAPI_PSK) {
		ret = rtl_wapi_psk_init(wapi_ae, config->para.kt, config->para.kl, config->para.kv);
	}

	rtl_wapi_init_tx_mcast_pn(&wapi_ae->msksa);
	//rtl_wapi_install_msk(wapi_ae);

exit:

	if (ret < 0)
		rtl_wapi_ae_free(&wapi_ae);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d) <<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}

int rtl_wapi_ae_free(struct wapi_ae_st **pwapi_ae)
{
	struct wapi_ae_st *wapi_ae = *pwapi_ae;
	if (wapi_ae) {
		rtl_wapi_cert_free(&wapi_ae->cert_infos);
		rtl_free_buffer(wapi_ae->psk_infos, sizeof(psk_info));
		
		rtl_wapi_sock_close(wapi_ae);
		
		wapi_ae = rtl_free_buffer(wapi_ae, sizeof(struct wapi_ae_st));

		*pwapi_ae = NULL;
	}
	
	return 0;
}

int rtl_wapi_asue_free(struct wapi_asue_st **pwapi_asue)
{
	struct wapi_asue_st *wapi_asue = *pwapi_asue;
	if (wapi_asue) {

		wapi_asue->rxfrag = rtl_wapi_free_rxfrag(wapi_asue->rxfrag);		
		wapi_asue->asue_id = rtl_free_buffer(wapi_asue->asue_id, sizeof(wai_fixdata_id));
		wapi_asue->usksa = rtl_free_buffer(wapi_asue->usksa, sizeof(struct wapi_usksa));
		wapi_asue->bksa = rtl_free_buffer(wapi_asue->bksa, sizeof(struct wapi_bksa_cache));
		wapi_asue->asue_cert = rtl_free_buffer(wapi_asue->asue_cert, sizeof(cert_id));
		wapi_asue->asue_tmp_pubkey = rtl_free_buffer(wapi_asue->asue_tmp_pubkey, sizeof(wapi_data));
		wapi_asue->ae_tmp_pubkey = rtl_free_buffer(wapi_asue->ae_tmp_pubkey, sizeof(wapi_data));
		wapi_asue->ae_tmp_prikey = rtl_free_buffer(wapi_asue->ae_tmp_prikey, sizeof(wapi_data));
		wapi_asue->wai_data= rtl_free_buffer(wapi_asue->wai_data, wapi_asue->wai_data_len);
		wapi_asue->wapi_ie = rtl_free_buffer(wapi_asue->wapi_ie, MAX_WAPI_IE_LEN);
		
		rtl_del_timeout_entry(&wapi_asue->entry);	

		if (wapi_asue->asu_id_list) {
			rtl_free_buffer(wapi_asue->asu_id_list->id, wapi_asue->asu_id_list->id_num * sizeof(wai_fixdata_id));
			wapi_asue->asu_id_list = rtl_free_buffer(wapi_asue->asu_id_list, sizeof(id_list));
		}

		wapi_asue = rtl_free_buffer(wapi_asue, sizeof(struct wapi_asue_st));

		*pwapi_asue = NULL;
	}
	
	return 0;
}

void rtl_del_timeout_entry(struct timeout_entry *entry)
{
	struct timeout_entry **copp, *freep;	

	for (copp = &entry_list; (freep = *copp); copp = &freep->next)
		if (freep == entry) {
			*copp = freep->next;
			break;
		}
}

void rtl_wapi_ecdh_init(para_alg *ecdh)
{
	u8 oid_der[11] = {0x06, 0x09, 0x2a, 0x81, 0x1c, 0xd7, 0x63, 0x01, 0x01, 0x02, 0x01};
	
	ecdh->para_flag = 1;
	ecdh->para_len = sizeof(oid_der);
	memcpy(ecdh->para_data, oid_der, ecdh->para_len);
}

void rtl_wapi_sign_alg_init(tsign_alg *sign_alg)
{
	u8 oid_der[11] = {0x06, 0x09, 0x2a, 0x81, 0x1c, 0xd7, 0x63, 0x01, 0x01, 0x02, 0x01};
	
	sign_alg->alg_length = 16;
	sign_alg->sha256_flag = 1;
	sign_alg->sign_alg = 1;
	sign_alg->sign_para.para_flag = 1;
	sign_alg->sign_para.para_len = sizeof(oid_der);
	memcpy(sign_alg->sign_para.para_data, oid_der, sign_alg->sign_para.para_len);
}

void rtl_add_timeout_entry(timeout_handle_func func, void *arg, int time, struct timeout_entry *entry)
{
	struct timespec timenow;
	struct timeout_entry *p, **pp;

	memset(&timenow, 0, sizeof(struct timespec));

	rtl_del_timeout_entry(entry);

	entry->timeout = time;
	entry->arg = arg;
	entry->func = func;
	clock_gettime(CLOCK_MONOTONIC, &timenow);
	entry->time.tv_sec = timenow.tv_sec + time;
	entry->time.tv_nsec = timenow.tv_nsec;

	/*
	* Find correct place and link it in.
	*/
	for (pp = &entry_list; (p = *pp); pp = &p->next)
		if (entry->time.tv_sec < p->time.tv_sec
			|| (entry->time.tv_sec == p->time.tv_sec
			&& entry->time.tv_nsec < p->time.tv_nsec))
			break;
		
	entry->next = p;
	*pp = entry;
}

void rtl_wapi_timeout_handler(void *arg)
{
	struct wapi_asue_st *wapi_asue = (struct wapi_asue_st *)arg;

	int counter = wapi_asue->entry.counter;

	if (counter >= MAX_RETRY_NUM) {
		rtl_del_timeout_entry(&wapi_asue->entry);		
		memset(&wapi_asue->entry, 0 , sizeof(struct timeout_entry));
		rtl_wapi_deauth(wapi_asue);
	} else {
		rtl_wapi_wai_send(wapi_asue->wai_data, wapi_asue->wai_data_len);
		wapi_asue->entry.counter++;
		
		rtl_add_timeout_entry(rtl_wapi_timeout_handler, wapi_asue, wapi_asue->entry.timeout, &wapi_asue->entry);
	}	
}

void rtl_wapi_timer_set(struct wapi_asue_st *wapi_asue, int timeout, u8 *wai_data, int len)
{
	memset(&wapi_asue->entry, 0, sizeof(struct timeout_entry));	
	if (wapi_asue->wai_data)
		rtl_free_buffer(wapi_asue->wai_data, wapi_asue->wai_data_len);

	wapi_asue->wai_data = (u8 *)rtl_allocate_buffer(len);
	if (!wapi_asue->wai_data) {
		rtl_wapi_trace(MSG_ERROR, "%s(): allocate wai_data fail", __FUNCTION__);
		return;
	}

	memcpy(wapi_asue->wai_data, wai_data, len);
	wapi_asue->wai_data_len = len;	
	
	rtl_add_timeout_entry(rtl_wapi_timeout_handler, wapi_asue, timeout, &wapi_asue->entry);
}

void rtl_wapi_timer_reset(struct wapi_asue_st *wapi_asue)
{	
	rtl_del_timeout_entry(&wapi_asue->entry);

	if (wapi_asue->wai_data)
		wapi_asue->wai_data = rtl_free_buffer(wapi_asue->wai_data, wapi_asue->wai_data_len);
}

void rtl_wapi_key_update_timeout_handler(void *arg)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	struct wapi_asue_st *wapi_asue = (struct wapi_asue_st *)arg;
	struct wapi_ae_st *wapi_ae = rtl_get_ae_by_mac(wapi_asue->ae_mac);
	if (!wapi_ae) {
		rtl_wapi_trace(MSG_ERROR, "%s: get wapi_ae fail", __FUNCTION__);
		return;
	}
	rtl_wapi_update_usk(wapi_ae, wapi_asue);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);
}

void rtl_wapi_key_update_timer_set(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter (key_update_time = %d) >>>>>>>>>>>>>>>>>", __FUNCTION__, wapi_ae->key_update_time);
	
	rtl_add_timeout_entry(rtl_wapi_key_update_timeout_handler, wapi_asue, wapi_ae->key_update_time, &wapi_ae->key_update_entry);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);
}

void *rtl_allocate_buffer(int len)
{
	char *buffer=NULL;
	buffer = (char *)malloc(len);
	if(buffer)
		memset(buffer, 0, len);
	else
		buffer = NULL;
	return buffer;
}

void *rtl_free_buffer(void *buffer, int len)
{
	char *tmpbuf = (char *)buffer;

	if(tmpbuf != NULL)
	{
		memset(tmpbuf, 0, len);
		free(tmpbuf);
		return NULL;
	}
	else
		return NULL;
}

int rtk_get_interface_mtu(u8 *ifname, int *mtu)
{
	int skfd, ret = -1;
	struct ifreq ifr;
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	ret = ioctl(skfd, SIOCGIFMTU, &ifr);
	close(skfd);

	if (ret == 0)
		*mtu = ifr.ifr_mtu;
	
	return ret;
}

int rtl_wapi_send_wai_fragment_packet(u8 *buf, size_t len, int mtu)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	int i;
	u8 fragment_buf[2048];
	int frag_data_len, frag_pkt_len;
	struct wai_hdr *frag_wai_hdr = NULL;

	struct l2_ethhdr *l2_hdr = (struct l2_ethhdr *)buf;
	struct wai_hdr *hdr = (struct wai_hdr *)(buf + ETHHDR_LEN);
	u8 *wai_data = buf + ETHHDR_LEN + WAI_HDR_LEN;
	
	int wai_data_len = len - ETHHDR_LEN - WAI_HDR_LEN;
	int fragment_len = mtu - WAI_HDR_LEN;

	int fragment_num = (wai_data_len + fragment_len - 1 ) / fragment_len;

	for (i = 0; i < fragment_num; i++) {
		memset(fragment_buf, 0, sizeof(fragment_buf));
		
		frag_data_len = (wai_data_len > fragment_len ? fragment_len : wai_data_len);

		memcpy(fragment_buf, buf, ETHHDR_LEN + WAI_HDR_LEN);
		memcpy(fragment_buf + ETHHDR_LEN + WAI_HDR_LEN, wai_data, frag_data_len);

		wai_data_len -= frag_data_len;
		wai_data += frag_data_len;

		frag_wai_hdr = (struct wai_hdr *)(fragment_buf + ETHHDR_LEN);
		frag_wai_hdr->frag_num = i;
		frag_wai_hdr->more_frag = (wai_data_len == 0 ? 0 : 1);
		frag_wai_hdr->length = htons(frag_data_len + WAI_HDR_LEN);

		frag_pkt_len = frag_data_len + WAI_HDR_LEN + ETHHDR_LEN;

		rtl_wapi_send_wai_packet(fragment_buf, frag_pkt_len);
	}

	return 0;
}

int rtl_wapi_wai_send(u8 *buf, size_t len)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	int mtu;

	struct l2_ethhdr *l2_hdr = (struct l2_ethhdr *)buf;
	struct wapi_ae_st *wapi_ae = rtl_get_ae_by_mac(l2_hdr->h_source);
	if (!wapi_ae) {
		rtl_wapi_trace(MSG_ERROR, "%s: wapi_ae is NULL", __FUNCTION__);
		return -1;
	}

	if (rtk_get_interface_mtu(wapi_ae->ifname, &mtu) < 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: get interface %s MTU fail", __FUNCTION__, wapi_ae->ifname);
		return -1;
	}

	if (len - ETHHDR_LEN > mtu)
		return rtl_wapi_send_wai_fragment_packet(buf, len, mtu);
	else
		return rtl_wapi_send_wai_packet(buf, len);
	
}

void rtl_wapi_wai_receive(u8* buf, size_t len)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	struct l2_ethhdr *l2_hdr = (struct l2_ethhdr *)buf;
	
	struct wapi_ae_st *wapi_ae = rtl_get_ae_by_mac(l2_hdr->h_dest);
	struct wapi_asue_st *wapi_asue = rtl_get_asue_by_mac(l2_hdr->h_source);

	if (!wapi_ae || !wapi_asue) {
		rtl_wapi_trace(MSG_ERROR, "%s: wapi_ae or wapi_asue is NULL", __FUNCTION__);
		return;
	}
	if (wapi_ae->auth_type != WAPI_CERT && wapi_ae->auth_type != WAPI_PSK)
		return;

	struct wapi_rxfrag rxbuf ,*temp_rxbuf= NULL;
	rxbuf.data = buf;
	rxbuf.data_len = len;
	temp_rxbuf = rtl_wapi_defrag(wapi_asue, &rxbuf);
	if (temp_rxbuf)
		rtl_wapi_rx_wai(wapi_ae, wapi_asue);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);
}


void rtl_wapi_init_tx_mcast_pn(struct wapi_msksa *msksa)
{	
	int i;

	for (i = 0; i < WAPI_PN_LEN; i += 2) {
		msksa->txMcastPN[i] = 0x5c;
		msksa->txMcastPN[i+1] = 0x36;
	}	

	memcpy(msksa->key_ann_id, msksa->txMcastPN, WAI_KEY_AN_ID_LEN);

	get_random(msksa->nmk, PAIRKEY_LEN);
}

int rtl_wapi_bk_derivation(u8 *psk, u32 psk_len, struct wapi_asue_st *wapi_asue)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	u8 input_text[] = "preshared key expansion for authentication and key negotiation";
	
	if (!psk || !psk_len || !wapi_asue) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid argument", __FUNCTION__);
		return -1;
	}

	u8 *bk = wapi_asue->bksa->bk;
	
	KD_hmac_sha256(input_text, strlen((char*)input_text), psk, psk_len, bk, BK_LEN);

	u8 addid[ADDID_LEN] = {0};
	
	memcpy(addid, wapi_asue->ae_mac, ETH_ALEN);
	memcpy(addid + ETH_ALEN, wapi_asue->asue_mac, ETH_ALEN);

	u8 *bkid = wapi_asue->bksa->bkid;
	KD_hmac_sha256(addid, ADDID_LEN, bk, BK_LEN, bkid, BKID_LEN);

	wapi_asue->wapi_state = WAPI_BKSA_ESTABLISH;

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);
	
	return 0;
}


void* MEMCPY(void *dbuf, const void *srcbuf, int len) 
{
	memcpy(dbuf, srcbuf, len); 
	return (char*)dbuf+len;
}

static struct wapi_rxfrag  *rtl_wapi_malloc_rxfrag(int maxlen)
{
	struct wapi_rxfrag *frag = NULL;
	frag  = (struct wapi_rxfrag *)rtl_allocate_buffer(sizeof(struct wapi_rxfrag));
	if(frag != NULL) { 
		frag->maxlen = maxlen;
		frag->data = (u8*)rtl_allocate_buffer(frag->maxlen);
		if (frag->data == NULL)
			frag = (struct wapi_rxfrag *)rtl_free_buffer(frag, sizeof(struct wapi_rxfrag));
	}
	return frag;	
}

void *rtl_wapi_free_rxfrag(struct wapi_rxfrag *frag)
{
	if(frag != NULL){
		rtl_free_buffer((void *)(frag->data), frag->maxlen);
		rtl_free_buffer((void *)frag, sizeof(struct wapi_rxfrag ));
	}
	return NULL;
}
static void rtl_wapi_put_frag(struct wapi_rxfrag *frag, u8 *data, int len)
{
	memcpy((unsigned char *)frag->data + frag->data_len, data, len);
	frag->data_len += len;

	u16 tmp = (u16)frag->data_len;
	SETSHORT((frag->data + ETHHDR_LEN + WAI_LENGTH_OFFSET), tmp);
}
struct wapi_rxfrag *rtl_wapi_defrag(struct wapi_asue_st *wapi_asue, struct wapi_rxfrag *rxbuf)
{
	u8 *buf = (u8 *)(rxbuf->data + ETHHDR_LEN);
	int len = rxbuf->data_len - ETHHDR_LEN;
	
	struct wai_hdr *hdr = (struct wai_hdr *)buf;
	u16 rxseq=0, last_rxseq=0;
	u8 fragno, last_fragno;
	u8 more_frag = hdr->more_frag;
	
	struct wapi_rxfrag *wai_frame = NULL;
	GETSHORT(hdr->seq_num, rxseq);
	fragno = hdr->frag_num;

	/* Quick way out, if there's nothing to defragment */
	if ((!more_frag) && (!fragno) && (!wapi_asue->rxfrag)) {
		rtl_wapi_trace(MSG_DEBUG, "%s:%d", __FUNCTION__, __LINE__);

		wapi_asue->rxfrag = rtl_wapi_malloc_rxfrag(PAGE_LEN);
		if (wapi_asue->rxfrag) {
			memcpy(wapi_asue->rxfrag->data, rxbuf->data, rxbuf->data_len);
			wapi_asue->rxfrag->data_len = rxbuf->data_len;
		} else {
			rtl_wapi_trace(MSG_ERROR, "%s: allocate wapi_asue->rxfrag fail", __FUNCTION__);
			return NULL;
		}
			
		return rxbuf;
	}

	/*
	 * Update the time stamp.  As a side effect, it
	 * also makes sure that the timer will not change
	 * ni->ni_rxfrag[0] for at least 1 second, or in
	 * other words, for the remaining of this function.
	 */
	/*
	 * Validate that fragment is in order and
	 * related to the previous ones.
	 */
	if (wapi_asue->rxfrag) {
		struct wai_hdr *hdr1;
		rtl_wapi_trace(MSG_DEBUG, "%s:%d", __FUNCTION__, __LINE__);

		hdr1 = (struct wai_hdr *)(wapi_asue->rxfrag->data + ETHHDR_LEN);
		GETSHORT(hdr1->seq_num, last_rxseq);
		last_fragno = hdr1->frag_num;
		if ((rxseq != last_rxseq) 
		    || (fragno != last_fragno + 1) 
			|| (wapi_asue->rxfrag->maxlen - wapi_asue->rxfrag->data_len < len)
			) {
			/*
			 * Unrelated fragment or no space for it,
			 * clear current fragments
			 */
			wapi_asue->rxfrag = rtl_wapi_free_rxfrag(wapi_asue->rxfrag);
		}
		rtl_wapi_trace(MSG_DEBUG, "%s:%d", __FUNCTION__, __LINE__);
	}
	/* If this is the first fragment */
 	if (!wapi_asue->rxfrag && !fragno) {
		wapi_asue->rxfrag = rtl_wapi_malloc_rxfrag(PAGE_LEN);
		/* If more frags are coming */
		if (more_frag) {
			rtl_wapi_put_frag(wapi_asue->rxfrag, buf - ETHHDR_LEN, len + ETHHDR_LEN);
			rtl_wapi_trace(MSG_DEBUG, "%s:%d", __FUNCTION__, __LINE__);		
		}
	} else if (wapi_asue->rxfrag) {
		struct wai_hdr *lhdr = (struct wai_hdr *)(wapi_asue->rxfrag->data + ETHHDR_LEN);
		rtl_wapi_put_frag(wapi_asue->rxfrag , buf + WAI_HDR_LEN, len - WAI_HDR_LEN);
		*(u16 *)lhdr->seq_num = *(u16*)hdr->seq_num;
		lhdr->frag_num = hdr->frag_num;
	}
		
	if (more_frag) {
		/* More to come */
		wai_frame = NULL;
	} else {
		/* Last fragment received, we're done! */
		wai_frame = wapi_asue->rxfrag;
	}
	return wai_frame;
}


static int rtl_wapi_check_wai_frame(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	u16 version = 0, rxseq = 0, frmlen = 0;
	
	u8 *buf = wapi_asue->rxfrag->data + ETHHDR_LEN;
	int len = wapi_asue->rxfrag->data_len - ETHHDR_LEN;
	struct wai_hdr *hdr = (struct wai_hdr *)buf;
	
	if(len < WAI_HDR_LEN) {
		rtl_wapi_trace(MSG_ERROR, "WAPILib: WAI frame too short, len %d",  len);
		return -1;
	}
	
	version = ((hdr->version[0]<<8)| hdr->version[1]);
	GETSHORT(hdr->version, version);
	if( version != WAI_VERSION) {
		rtl_wapi_trace(MSG_ERROR, "WAPILib: WAI frame Version(%u) is wrong", version);
		return -1;
	}

	if(hdr->type != WAI_TYPE) {
		rtl_wapi_trace(MSG_ERROR, "WAPILib: WAI frame type(%u) is wrong", hdr->type);
		return -1;
	}
	
	if ((wapi_ae->auth_type == WAPI_PSK && hdr->stype < WAI_USK_AGREEMENT_REQUEST)
		|| hdr->stype < WAI_AUTHACTIVE) {
		rtl_wapi_trace(MSG_ERROR, "WAPILib: WAI frame stype(%u) is wrong ",hdr->stype);
		return -1;
	}
	
	GETSHORT((buf + WAI_LENGTH_OFFSET), frmlen);
	if (len != frmlen) {
		rtl_wapi_trace(MSG_ERROR, "WAPILib: WAI frame length(%u) is wrong",  frmlen);
		return -1;
	}

	GETSHORT(hdr->seq_num, rxseq);
	if (rxseq <= wapi_asue->rxseq) {
		rtl_wapi_trace(MSG_ERROR, "%s: WAI frame sequence is wrong, rxseq=%d wapi_asue->rxseq=%d", __FUNCTION__,rxseq,wapi_asue->rxseq);

		/*
		According to WAPI Guide, the sequence number of the first WAI packet is 1, and the subsequent WAI packet are incremented by 1.
		But some mobile phones (Redmi K40 Pro/iQOO Neo3) send WAI packet with sequence number always 1. 
		In order to make these mobile phones connnect successfully, does not drop the WAI packet with wrong sequence number.
		*/
		
		//return -1; 
	}
	
	return 0;
}

int rtl_wapi_compare_identity(u8 *remote, wai_fixdata_id *local)
{
	u16 remote_id_flag, remote_id_len;
	u8 *p = remote;
	
	GETSHORT(p, remote_id_flag); p += sizeof(u16);
	GETSHORT(p, remote_id_len); p += sizeof(u16);

	if (remote_id_flag != local->id_flag 
		|| remote_id_len != local->id_len 
		|| memcmp(p, local->id_data, remote_id_len))
		return -1;

	return 0;
}

int rtl_wapi_compare_ecdh(u8 *remote, para_alg *local)
{
	u8 remote_para_flag;
	u16 remote_para_len;
	u8 *p = remote;
	
	remote_para_flag = *p; p += 1;
	GETSHORT(p, remote_para_len); p += 2;

	if (remote_para_flag != local->para_flag 
		|| remote_para_len != local->para_len
		|| memcmp(p, local->para_data, remote_para_len))
		return -1;

	return 0;
}

int rtl_check_asu_id_list(u8 *remote, struct wapi_asue_st *wapi_asue)
{
	int i, ret = -1;
	u8 *p = remote;
	u16 id_list_len, id_num;
	u16 id_flag, id_len;
	u16 tmp_len = 0;
	wai_fixdata_id *tmp_id;
	
	u8 type = *p; 
	p += 1;
	if (type != 3) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid id list type", __FUNCTION__);
		return ret;
	}
	GETSHORT(p, id_list_len); 
	p += 2;
	p += 1; // resverd one byte
	GETSHORT(p, id_num);
	p += 2;

	if (wapi_asue->asu_id_list) {
		rtl_free_buffer(wapi_asue->asu_id_list->id, wapi_asue->asu_id_list->id_num * sizeof(wai_fixdata_id));
		rtl_free_buffer(wapi_asue->asu_id_list, sizeof(id_list));
	}
	wapi_asue->asu_id_list = (id_list *)rtl_allocate_buffer(sizeof(id_list));
	if (!wapi_asue->asu_id_list) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate asu_id_list fail", __FUNCTION__);
		goto exit;
	}
	wapi_asue->asu_id_list->type = type;
	wapi_asue->asu_id_list->length = id_list_len;
	wapi_asue->asu_id_list->id_num = id_num;
	wapi_asue->asu_id_list->id = (wai_fixdata_id *)rtl_allocate_buffer(id_num * sizeof(wai_fixdata_id));
	if (!wapi_asue->asu_id_list->id) {		
		rtl_wapi_trace(MSG_ERROR, "%s: allocate asu_id_list id fail", __FUNCTION__);
		goto exit;
	}

	tmp_len += 1 + 2;

	for (i = 0; i < id_num; i++) {
		tmp_id = &wapi_asue->asu_id_list->id[i];
	
		GETSHORT(p, id_flag);
		tmp_id->id_flag = id_flag;
		p += 2;
	
		GETSHORT(p, id_len);
		tmp_id->id_len = id_len;
		p += 2;

		memcpy(tmp_id->id_data, p, id_len);
		
		p += id_len;

		tmp_len += id_len + 4;
	}

	if (tmp_len != id_list_len) {
		rtl_wapi_trace(MSG_ERROR, "%s: id_list_len is not right", __FUNCTION__);
		goto exit;
	}

	ret = 0;

exit:

	if (ret < 0) {

		if (wapi_asue->asu_id_list) {
			rtl_free_buffer(wapi_asue->asu_id_list->id, wapi_asue->asu_id_list->id_num * sizeof(wai_fixdata_id));
			wapi_asue->asu_id_list = rtl_free_buffer(wapi_asue->asu_id_list, sizeof(id_list));
		}
	}

	return ret;
}


cert_auth_result *rtl_wapi_parse_cert_auth_result(u8 *cert_auth_rst)
{
	u8 *p = cert_auth_rst;
	cert_auth_result *result = (cert_auth_result *)rtl_allocate_buffer(sizeof(cert_auth_result));
	if (!result) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate result fail", __FUNCTION__);
		return NULL;
	}

	result->type = *p;
	if (result->type != 2) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid type of certificate auth result", __FUNCTION__);
		goto exit;
	}	
	p += 1;
	
	GETSHORT(p, result->length);
	p += 2;

	memcpy(result->Nae, p, WAPI_CHALLENGE_LEN);
	p += WAPI_CHALLENGE_LEN;

	memcpy(result->Nasue, p, WAPI_CHALLENGE_LEN);
	p += WAPI_CHALLENGE_LEN;

	result->asue_result = *p;
	p += 1;

	GETSHORT(p, result->asue_cert.cert_flag);
	p += 2;

	GETSHORT(p, result->asue_cert.length);
	p += 2;

	memcpy(result->asue_cert.data, p, result->asue_cert.length);
	p += result->asue_cert.length;

	result->ae_result = *p;
	p += 1;

	GETSHORT(p, result->ae_cert.cert_flag);
	p += 2;

	GETSHORT(p, result->ae_cert.length);
	p += 2;

	memcpy(result->ae_cert.data, p, result->ae_cert.length);
	p += result->ae_cert.length;

	return result;

exit:

	if (result)
		rtl_free_buffer(result, sizeof(cert_auth_result));
	
	return NULL;
}



tsign_info *rtl_wapi_parse_signature(u8 *signature)
{
	u8 *p = signature;
	tsign_info *sign = (tsign_info *)rtl_allocate_buffer(sizeof(tsign_info));
	if (!sign) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate sign fail", __FUNCTION__);
		return NULL;
	}

	sign->type = *p;
	if (sign->type != 1) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid type", __FUNCTION__);
		goto exit;
	}	
	p += 1;
	
	GETSHORT(p, sign->length);
	p += 2;

	GETSHORT(p, sign->id.id_flag);
	p += 2;

	GETSHORT(p, sign->id.id_len);
	p += 2;

	memcpy(sign->id.id_data, p, sign->id.id_len);
	p += sign->id.id_len;

	GETSHORT(p, sign->sign_alg.alg_length);
	p += 2;

	sign->sign_alg.sha256_flag = *p;
	p += 1;

	sign->sign_alg.sign_alg = *p;
	p += 1;

	sign->sign_alg.sign_para.para_flag = *p;
	p += 1;

	GETSHORT(p, sign->sign_alg.sign_para.para_len);
	p += 2;

	memcpy(sign->sign_alg.sign_para.para_data, p, sign->sign_alg.sign_para.para_len);
	p += sign->sign_alg.sign_para.para_len;

	GETSHORT(p, sign->sign_value.length);
	p += 2;

	memcpy(sign->sign_value.data, p, sign->sign_value.length);
	p += sign->sign_value.length;

	return sign;

exit:

	if (sign)
		rtl_free_buffer(sign, sizeof(tsign_info));
	
	return NULL;
}

static void rtl_wapi_key_derivation(u8 *inkey, int inkey_len, u8 *text, int text_len, u8 *outkey, int outkey_len, u8 isusk)
{

	KD_hmac_sha256(text, text_len, inkey, inkey_len, outkey, outkey_len);
	
	if(isusk)
		mhash_sha256(outkey + outkey_len - WAPI_CHALLENGE_LEN, WAPI_CHALLENGE_LEN, outkey + outkey_len - WAPI_CHALLENGE_LEN);
}

int rtl_wapi_fixdata_id(cert_id *cert_st, wai_fixdata_id *fixdata_id)
{
	u8 *temp = NULL;
	wapi_data *subject_name = NULL;
	wapi_data *issure_name = NULL;
	wapi_data *serial_no = NULL;

	if (!fixdata_id || !cert_st)
		return -1;
	
	temp= fixdata_id->id_data;
	fixdata_id->id_flag = cert_st->cert_flag;
	
	subject_name = rtl_wapi_cert_get_subject_name(cert_st);
	issure_name = rtl_wapi_cert_get_issuer_name(cert_st);
	serial_no = rtl_wapi_cert_get_serial_number(cert_st);
	
	if (!subject_name || !issure_name || !serial_no) {
		rtl_wapi_trace(MSG_ERROR, "%s: Get subject_name, issure_name or serial_no fail.", __FUNCTION__);
		return -1;
	}

	memcpy(temp, subject_name->data, subject_name->length);
	temp += subject_name->length;

	memcpy(temp, issure_name->data, issure_name->length);
	temp += issure_name->length;

	memcpy(temp, serial_no->data, serial_no->length);
	temp +=serial_no->length;

	fixdata_id->id_len = temp - fixdata_id->id_data;
	rtl_free_buffer(subject_name, sizeof(wapi_data));
	rtl_free_buffer(issure_name, sizeof(wapi_data));
	rtl_free_buffer(serial_no, sizeof(wapi_data));
	
	return 0;
}

static int rtl_wapi_eck_derivation(struct wapi_asue_st *wapi_asue)
{
	/* get the public key and the private key for ECC */
	if (ecc192_genkey(wapi_asue->ae_tmp_prikey->data, wapi_asue->ae_tmp_pubkey->data))
		return -1;

	wapi_asue->ae_tmp_prikey->length = SECKEY_LEN;
	wapi_asue->ae_tmp_pubkey->length = PUBKEY2_LEN;

	return 0;
}

static int rtl_wapi_certauthbk_derivation(struct wapi_asue_st *wapi_asue)
{
	char input_text[] = "base key expansion for key and additional nonce";
	u8 text[256] = {0};
	u8 temp_out[48] = {0};
	u8  ecdhkey[24] = {0};
	int ret = -1;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	if (wapi_asue->ae_tmp_prikey)
		rtl_free_buffer(wapi_asue->ae_tmp_prikey, sizeof(wapi_data));
	
	wapi_asue->ae_tmp_prikey = (wapi_data *)rtl_allocate_buffer(sizeof(wapi_data));
	if (!wapi_asue->ae_tmp_prikey) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate ae_tmp_prikey fail", __FUNCTION__);
		goto exit;
	}

	if (wapi_asue->ae_tmp_pubkey)
		rtl_free_buffer(wapi_asue->ae_tmp_pubkey, sizeof(wapi_data));
	
	wapi_asue->ae_tmp_pubkey = (wapi_data *)rtl_allocate_buffer(sizeof(wapi_data));
	if (!wapi_asue->ae_tmp_pubkey) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate ae_tmp_pubkey fail", __FUNCTION__);
		goto exit;
	}

	if (rtl_wapi_eck_derivation(wapi_asue)) {
		rtl_wapi_trace(MSG_ERROR, "%s: call rtl_wapi_eck_derivation() fail", __FUNCTION__);
		goto exit;
	}

	if (!ecc192_ecdh(wapi_asue->ae_tmp_prikey->data, wapi_asue->asue_tmp_pubkey->data, ecdhkey)) {
		rtl_wapi_trace(MSG_DEBUG, "%s: call ecc192_ecdh() fail", __FUNCTION__);
		goto exit;
	}

	memset(text, 0, sizeof(text));
	memcpy(text, wapi_asue->Nae, WAPI_CHALLENGE_LEN);
	memcpy(text + WAPI_CHALLENGE_LEN, wapi_asue->Nasue, WAPI_CHALLENGE_LEN);
	memcpy(text + (WAPI_CHALLENGE_LEN << 1), input_text, strlen(input_text));
	KD_hmac_sha256(text, (WAPI_CHALLENGE_LEN << 1) + strlen(input_text), 
						ecdhkey, 24,
						temp_out, BK_LEN + WAPI_AUTHFLAG_LEN);	
	
	memcpy(wapi_asue->bksa->bk, temp_out, BK_LEN);
	
	memset(text, 0, sizeof(text));
	memcpy(text, wapi_asue->ae_mac, ETH_ALEN);
	memcpy(text + ETH_ALEN, wapi_asue->asue_mac, ETH_ALEN);
	
	KD_hmac_sha256(text, (ETH_ALEN << 1),
						wapi_asue->bksa->bk, BK_LEN, 
						wapi_asue->bksa->bkid, BKID_LEN);
	
	mhash_sha256(temp_out + BK_LEN, WAPI_AUTHFLAG_LEN, wapi_asue->ae_auth_flag);

	wapi_asue->wapi_state = WAPI_BKSA_ESTABLISH;
	
	ret = 0;

exit:

	if (ret < 0) {
		if (wapi_asue->ae_tmp_prikey)
			wapi_asue->ae_tmp_prikey = rtl_free_buffer(wapi_asue->ae_tmp_prikey, sizeof(wapi_data));
		if (wapi_asue->ae_tmp_pubkey)
			wapi_asue->ae_tmp_pubkey = rtl_free_buffer(wapi_asue->ae_tmp_pubkey, sizeof(wapi_data));		
	}

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}

static u8 *rtl_build_ethernet_hdr(u8 *buffer, u8 *ae_mac, u8 *asue_mac)
{
	struct l2_ethhdr *l2_hdr = (struct l2_ethhdr *)buffer;
	memcpy(l2_hdr->h_dest, asue_mac, ETH_ALEN);
	memcpy(l2_hdr->h_source, ae_mac, ETH_ALEN);
	l2_hdr->h_proto = htons(ETH_P_WAI);	

	return (u8 *)(l2_hdr + 1);
}

int rtl_wapi_increase_pn(u8 *pn, u8 count)
{
	int i;

	for (i = WAPI_PN_LEN - 1; i >= 0; i--) {
		if (pn[i] + count <= 0xff) {
			pn[i] += count;
			return WAPI_RETURN_SUCCESS;
		} else {
			pn[i] += count;
			count = 1;
		}
	}

	return WAPI_RETURN_FAILED;
}

static u8 *rtl_wapi_build_hdr(u8 *pos, u16 txseq, u8 stype)
{
	struct wai_hdr *hdr = (struct wai_hdr *)pos;
	
	SETSHORT(hdr->version, WAI_VERSION);
	hdr->type = WAI_TYPE;
	hdr->stype= stype;
	SETSHORT(hdr->reserve, 0);
	hdr->length = 0x0000;
	SETSHORT(hdr->seq_num, txseq);
	hdr->frag_num = 0;
	hdr->more_frag = 0;
	return (u8 *)(hdr+1);
}
static void rtl_wapi_set_length(u8 *pos, u16 length)
{
	SETSHORT((pos + ETHHDR_LEN + 6), (length - ETHHDR_LEN));
}



void rtl_wapi_rx_wai(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	struct wai_hdr *hdr = NULL;
	int ret = 0;
	u16 plen, frmlen = 0;

	u8 *buf = wapi_asue->rxfrag->data;
	
	if (wapi_asue->wapi_state < WAPI_AUTH_ACTIVE_SNT) {
		rtl_wapi_trace(MSG_ERROR, "wapi_asue->wapi_state < WAPI_AUTH_ACTIVE_SNT");
		goto exit;
	}
	if (wapi_ae->auth_type == WAPI_CERT && 
		(!wapi_ae->cert_infos || !wapi_ae->cert_infos->ae_cert || !wapi_ae->cert_infos->ca_cert)) {
		rtl_wapi_trace(MSG_ERROR, "No cert");
		goto exit;
	}

	hdr = (struct wai_hdr *)(buf + ETHHDR_LEN);
	if (rtl_wapi_check_wai_frame(wapi_ae, wapi_asue)) {
		rtl_wapi_trace(MSG_ERROR, "WAPILib: WAI frame is wrong");
		goto exit;
	}
	
	frmlen = ntohs(hdr->length);
	plen = frmlen - WAI_HDR_LEN;
	rtl_wapi_trace(MSG_DEBUG, "%s  plen = %d, hdr->length = %d", __FUNCTION__, plen, frmlen);

	switch (wapi_asue->wapi_state) {
		
		case WAPI_AUTH_ACTIVE_SNT:
			if (hdr->stype == WAI_ACCESS_AUTH_REQUEST) {
				rtl_wapi_trace(MSG_DEBUG, "%s: received WAI_ACCESS_AUTH_REQUEST", __FUNCTION__);				
				rtl_wapi_timer_reset(wapi_asue);
			
				ret = rtl_wapi_recv_access_auth_request_pkt(wapi_ae, wapi_asue);

				if (wapi_asue->wapi_state == WAPI_ACCESS_AUTH_REQ_RCVD) {

					if (wapi_ae->wapi_role == WAPI_AE) {						
						ret = rtl_wapi_send_cert_auth_request_pkt(wapi_ae, wapi_asue);

						if (wapi_asue->wapi_state == WAPI_CERT_AUTH_RSP_RCVD) {
							
							ret = rtl_wapi_recv_cert_auth_response_pkt(wapi_ae, wapi_asue);

							if (ret == 0) {
								if (wapi_asue->access_result == ACCESS_SUCCESS)
									rtl_wapi_certauthbk_derivation(wapi_asue);
								
								ret = rtl_wapi_send_access_auth_response_pkt(wapi_ae, wapi_asue);

								if (wapi_asue->access_result != ACCESS_SUCCESS)
									rtl_wapi_deauth(wapi_asue);
								else if (ret == 0)
									rtl_wapi_send_usk_agreement_request_pkt(wapi_ae, wapi_asue);		
							}
						}				
					}
					else if (wapi_ae->wapi_role == WAPI_AE_ASU) {
						//local authenticate ASUE certificate
						ret = rtl_wapi_local_auth_asue_cert(wapi_ae, wapi_asue);
						if (ret == 0) {
							if (wapi_asue->access_result == ACCESS_SUCCESS)
								rtl_wapi_certauthbk_derivation(wapi_asue);
							
							ret = rtl_wapi_send_access_auth_response_pkt(wapi_ae, wapi_asue);

							if (wapi_asue->access_result != ACCESS_SUCCESS)
								rtl_wapi_deauth(wapi_asue);
							else if (ret == 0)
								rtl_wapi_send_usk_agreement_request_pkt(wapi_ae, wapi_asue);		
						}
					}
				}			
			}
			break;
		case WAPI_USK_AGREEMENT_REQ_SNT:
			if (hdr->stype == WAI_USK_AGREEMENT_RESPONSE) {
				rtl_wapi_trace(MSG_DEBUG, "%s: received WAI_USK_AGREEMENT_RESPONSE", __FUNCTION__);
				rtl_wapi_timer_reset(wapi_asue);
				ret = rtl_wapi_recv_usk_agreement_response_pkt(wapi_ae, wapi_asue);
				if (ret == WAPI_RETURN_SUCCESS) {					
					ret = rtl_wapi_send_usk_agreement_confirm_pkt(wapi_ae, wapi_asue);

					if (ret == WAPI_RETURN_SUCCESS)
						rtl_wapi_send_msk_announcement_pkt(wapi_ae, wapi_asue);
				}
				else if (ret == WAPI_RETURN_DEASSOC)
					rtl_wapi_deauth(wapi_asue);
			}		    
			break;
		case WAPI_MSK_ANNOUNCEMENT_SNT:
			if (hdr->stype == WAI_MSK_ANNOUNCEMENT_RESPONSE) {
		    	rtl_wapi_trace(MSG_DEBUG, "%s: received WAI_MSK_ANNOUNCEMENT_RESPONSE\n", __FUNCTION__);
				rtl_wapi_timer_reset(wapi_asue);
		
				ret = rtl_wapi_recv_msk_announcement_response_pkt(wapi_ae, wapi_asue);

				if (ret == WAPI_RETURN_SUCCESS)
					rtl_wapi_install_msk(wapi_ae);

				rtl_wapi_key_update_timer_set(wapi_ae, wapi_asue);				
			}
			break;
		case WAPI_MSKA_ESTABLISH:				
			if (hdr->stype == WAI_USK_AGREEMENT_RESPONSE) {
		    	rtl_wapi_trace(MSG_DEBUG, "%s: received WAI_USK_AGREEMENT_RESPONSE\n", __FUNCTION__);
				rtl_wapi_timer_reset(wapi_asue);			
				ret = rtl_wapi_recv_usk_update_response_pkt(wapi_ae, wapi_asue);

				//when complete unicast key update, do multicast key update
				rtl_wapi_update_msk(wapi_ae);
				
				rtl_wapi_key_update_timer_set(wapi_ae, wapi_asue);
			}
			if (hdr->stype == WAI_MSK_ANNOUNCEMENT_RESPONSE) {
		    	rtl_wapi_trace(MSG_DEBUG, "%s: received WAI_MSK_ANNOUNCEMENT_RESPONSE\n", __FUNCTION__);
				rtl_wapi_timer_reset(wapi_asue);
				ret = rtl_wapi_recv_msk_update_response_pkt(wapi_ae, wapi_asue);				
			}
			break;

		default:
			break;
	}

exit:

	wapi_asue->rxfrag = rtl_wapi_free_rxfrag(wapi_asue->rxfrag); 
}


int rtl_wapi_send_auth_active_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	int ret = -1;
	u8 tbuf[MAX_WAI_PKT_LEN];
	u8 *pos = NULL;

	rtl_wapi_trace(MSG_DEBUG, "WAPILib: in %s:%d", __FUNCTION__, __LINE__);

	memset(tbuf, 0, sizeof(tbuf));

	pos = tbuf;
	pos = rtl_build_ethernet_hdr(pos, wapi_ae->ae_mac, wapi_asue->asue_mac);
	pos = rtl_wapi_build_hdr(pos, ++wapi_asue->txseq, WAI_AUTHACTIVE);

	//Flag
	wapi_asue->ae_flag = 0;
	*pos = wapi_asue->ae_flag;
	pos = pos + 1;

	//Authentication flag
	if (!(wapi_asue->ae_flag & BIT(0)))
		get_random(wapi_asue->ae_auth_flag, WAPI_AUTHFLAG_LEN);	
	pos= (u8*)MEMCPY(pos, wapi_asue->ae_auth_flag, WAPI_AUTHFLAG_LEN);

	//ASU identity
	SETSHORT(pos, wapi_ae->cert_infos->asu_id->id_flag); pos += 2;
	SETSHORT(pos, wapi_ae->cert_infos->asu_id->id_len); pos += 2;
	pos = (u8*)MEMCPY(pos, wapi_ae->cert_infos->asu_id->id_data, wapi_ae->cert_infos->asu_id->id_len);
	
	//AE certificate
	SETSHORT(pos, wapi_ae->cert_infos->ae_cert->cert_flag); pos += 2;
	SETSHORT(pos, wapi_ae->cert_infos->ae_cert->length); pos += 2;
	pos= (u8*)MEMCPY(pos, wapi_ae->cert_infos->ae_cert->data, wapi_ae->cert_infos->ae_cert->length);

	//ECDH parameter
	*pos = wapi_ae->ecdh.para_flag;	pos++;
	SETSHORT(pos, wapi_ae->ecdh.para_len); pos += 2;
	pos= (u8*)MEMCPY(pos, wapi_ae->ecdh.para_data, wapi_ae->ecdh.para_len);	

	rtl_wapi_set_length(tbuf, pos-tbuf);
	rtl_wapi_wai_send(tbuf, pos-tbuf);

	rtl_wapi_timer_set(wapi_asue, GENERAL_TIMEOUT, tbuf, pos-tbuf);

	wapi_asue->wapi_state = WAPI_AUTH_ACTIVE_SNT;

	ret = 0;

exit:
	
	return ret;
}

int rtl_wapi_recv_msg_from_asu(u8 *recv_buf, int *recv_len, int sockfd)
{
	int ret, len;
	fd_set fds;
	struct timeval tv;

	tv.tv_sec = CERT_AUTH_TIMEOUT;
	tv.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);

	ret = select(sockfd + 1, &fds, NULL, NULL, &tv);
		
	if (ret <= 0)
		return -1;

	if ((len = recv(sockfd, recv_buf, *recv_len, 0)) < 0)
		return -1;

	*recv_len = len;
		
	return 0;
}

int rtl_wapi_send_msg_to_asu(u8 *send_buf, int send_len, u8 *recv_buf, int *recv_len, struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	int ret = -1;
	int retry_count = 0;

	int fd = wapi_ae->sock->sockfd;
	struct sockaddr_in *servaddr = &wapi_ae->sock->servaddr;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	while(retry_count <= MAX_RETRY_NUM) {
		
		if (sendto(fd, send_buf, send_len, 0, (struct sockaddr *)servaddr, sizeof(struct sockaddr_in)) < 0) {
			rtl_wapi_trace(MSG_ERROR, "%s: send msg to asu fail", __FUNCTION__);
			return ret;
		}

		wapi_asue->wapi_state = WAPI_CERT_AUTH_REQ_SNT;

		if ((ret = rtl_wapi_recv_msg_from_asu(recv_buf, recv_len, fd)) == 0) {
			wapi_asue->wapi_state = WAPI_CERT_AUTH_RSP_RCVD;
			break;
		}

		retry_count++;
	}

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}

int rtl_wapi_recv_msk_update_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	int i, all_done = 0;
	struct wapi_asue_st *pasue = NULL;
	int ret = WAPI_RETURN_FAILED;
	
	if (!wapi_ae->msk_update)
		return ret;

	ret = rtl_wapi_recv_msk_announcement_response_pkt(wapi_ae, wapi_asue);
	if (ret != WAPI_RETURN_SUCCESS)
		return ret;

	wapi_asue->msk_update_done = 1;
	all_done = 1;

	for (i = 0; i < MAX_STA_NUM; i++) {	
		pasue = s_asue[i];

		if (!pasue)
			continue;

		if (pasue->wapi_state != WAPI_MSKA_ESTABLISH) {
			rtl_wapi_deauth(pasue);
			continue;
		}

		if (!pasue->msk_update_done) {
			all_done = 0;
			break;
		}
	}

	if (all_done) {	

		rtl_wapi_install_msk(wapi_ae);

		wapi_ae->msksa.mskid = !wapi_ae->msksa.mskid;
		wapi_ae->msk_update = 0;
		wapi_ae->msk_update_alldone = 1;
	}

	return ret;
}

int rtl_wapi_recv_usk_update_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	int ret = WAPI_RETURN_FAILED;
	
	if (wapi_asue->usk_update) {
		ret = rtl_wapi_recv_usk_agreement_response_pkt(wapi_ae, wapi_asue);

		if (ret != WAPI_RETURN_SUCCESS)
			return ret;

		ret = rtl_wapi_send_usk_agreement_confirm_pkt(wapi_ae, wapi_asue);

		if (ret != WAPI_RETURN_SUCCESS)
			return ret;

		rtl_wapi_timer_reset(wapi_asue);

		wapi_asue->usksa->uskid = !wapi_asue->usksa->uskid;
		
		wapi_asue->usk_update = 0;		
	} else {
		u8 *payload = wapi_asue->rxfrag->data + ETHHDR_LEN + WAI_HDR_LEN;
		u8 usk_update = BIT(4) & payload[0];
		u8 uskid = payload[1 + BKID_LEN];		
	
		if (usk_update && uskid != wapi_asue->usksa->uskid)
				ret = rtl_wapi_update_usk(wapi_ae, wapi_asue);
	}

	return ret;
}

int	rtl_wapi_update_msk(struct wapi_ae_st *wapi_ae)
{
	int i;
	struct wapi_asue_st *wapi_asue = NULL;
	int ret = WAPI_RETURN_FAILED;
	
	get_random(wapi_ae->msksa.nmk, PAIRKEY_LEN);
	wapi_ae->msk_update = 0;
	wapi_ae->msk_update_alldone = 0;

	for (i = 0; i < MAX_STA_NUM; i++) {	
		wapi_asue = s_asue[i];

		if (!wapi_asue)
			continue;

		if (wapi_asue->wapi_state != WAPI_MSKA_ESTABLISH) {
			rtl_wapi_deauth(wapi_asue);
			continue;
		}

		wapi_ae->msk_update = 1;
		wapi_asue->msk_update_done = 0;
		
		ret = rtl_wapi_send_msk_announcement_pkt(wapi_ae, wapi_asue);

		if (ret != WAPI_RETURN_SUCCESS) {
			rtl_wapi_deauth(wapi_asue);
			continue;
		}
	}
	return ret;	
}

int	rtl_wapi_update_usk(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	int ret;
	
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	wapi_asue->usk_update = 1;
	
	ret = rtl_wapi_send_usk_agreement_request_pkt(wapi_ae, wapi_asue);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}

int rtl_wapi_local_auth_asue_cert(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	if (rtl_wapi_cert_verify(wapi_ae->cert_infos->ca_pubkey, wapi_asue->asue_cert) < 0) {
		rtl_wapi_trace(MSG_DEBUG, "%s:%d local verify ASUE certificate fail.", __FUNCTION__, __LINE__);
		wapi_asue->access_result = CERT_ERROR;
	} else {
		rtl_wapi_trace(MSG_DEBUG, "%s:%d local verify ASUE certificate success.", __FUNCTION__, __LINE__);
		wapi_asue->access_result = ACCESS_SUCCESS;
	}

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);
	
	return 0;
}

int rtl_wapi_recv_msk_announcement_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	int offset = 0;
	u8 flag = 0, mskid = 0;
	u8 uskid = 0, key_idx = 0;
	u8 *addid = NULL, *key_ann_id = NULL;
	u8 *mic = NULL; 
	u8 local_mic[WAI_MIC_LEN];
	int ret = WAPI_RETURN_FAILED;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct wai_hdr *waihdr = (struct wai_hdr *)(wapi_asue->rxfrag->data + ETHHDR_LEN);
	u8 *payload = wapi_asue->rxfrag->data + ETHHDR_LEN + WAI_HDR_LEN;
	int len = wapi_asue->rxfrag->data_len - ETHHDR_LEN - WAI_HDR_LEN;	
	
	//Flag
	flag = payload[0];
	if (flag != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid flag", __FUNCTION__);
		goto exit;
	}
	offset += WAI_FLAG_LEN;

	//MSKID
	mskid = *(payload + offset);
	if (mskid != wapi_ae->msksa.mskid && wapi_ae->msk_update == 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: MSKID is not same", __FUNCTION__);
		goto exit;
	}
	offset += 1;

	//USKID
	uskid = *(payload + offset);
	if (uskid != wapi_asue->usksa->uskid) {
		rtl_wapi_trace(MSG_ERROR, "%s: USKID is not same", __FUNCTION__);
		goto exit;
	}
	offset += 1;

	if (wapi_ae->msk_update)
		key_idx = !wapi_ae->msksa.mskid;
	else 
		key_idx = wapi_ae->msksa.mskid;	

	//ADDID
	addid = payload + offset;
	if (memcmp(addid, wapi_ae->ae_mac, ETH_ALEN) || 
		memcmp(addid + ETH_ALEN, wapi_asue->asue_mac, ETH_ALEN)) {
		rtl_wapi_trace(MSG_ERROR, "%s: ADDID is not same", __FUNCTION__);
		goto exit;
	}
	offset += ADDID_LEN;

	//Key announce ID
	key_ann_id = payload + offset;
	if (memcmp(key_ann_id, wapi_ae->msksa.key_ann_id, WAPI_PN_LEN)) {
		rtl_wapi_trace(MSG_ERROR, "%s: Key announcement ID is not same", __FUNCTION__);
		goto exit;
	}
	offset += WAI_KEY_AN_ID_LEN;

	//Check MIC
	mic = payload + offset;

	wapi_hmac_sha256(payload, len - WAI_MIC_LEN, wapi_asue->usksa->usk[uskid].mak, PAIRKEY_LEN, local_mic, WAI_MIC_LEN);
	if (memcmp(mic, local_mic, WAI_MIC_LEN)) {
		rtl_wapi_trace(MSG_ERROR, "%s: MIC mismatch", __FUNCTION__);
		goto exit;
	}

	wapi_asue->wapi_state = WAPI_MSKA_ESTABLISH;
	
	GETSHORT(waihdr->seq_num, wapi_asue->rxseq);

	rtl_wapi_trace(MSG_DEBUG, "%s: wapi_asue->rxseq=%d", __FUNCTION__, wapi_asue->rxseq);
	
	ret = WAPI_RETURN_SUCCESS;
	
exit:

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}


int rtl_wapi_send_msk_announcement_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{	
	u8 mskid, uskid;
	u8 tbuf[MAX_WAI_PKT_LEN];
	u8 *pos = NULL;
	int ret = WAPI_RETURN_FAILED;
	
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	memset(tbuf, 0, sizeof(tbuf));
	
	pos = tbuf;
	pos = rtl_build_ethernet_hdr(pos, wapi_ae->ae_mac, wapi_asue->asue_mac);
	pos = rtl_wapi_build_hdr(pos, ++wapi_asue->txseq, WAI_MSK_ANNOUNCEMENT);

	if (wapi_ae->msk_update)
		mskid = !wapi_ae->msksa.mskid;
	else 
		mskid = wapi_ae->msksa.mskid;

	//Flag
	u8 flag = 0;
	*pos = flag;
	pos = pos + 1;

	//MSKID
	*pos = mskid;
	pos = pos + 1;

	//USKID
	uskid = wapi_asue->usksa->uskid;
	*pos = uskid;
	pos = pos + 1;

	//ADDID
	pos= (u8*)MEMCPY(pos, wapi_ae->ae_mac, ETH_ALEN);
	pos= (u8*)MEMCPY(pos, wapi_asue->asue_mac, ETH_ALEN);

	//Data PN		
	pos= (u8*)MEMCPY(pos, wapi_ae->msksa.txMcastPN, WAPI_PN_LEN);

	//Key announce ID
	if (wapi_ae->msk_update) {
		if (rtl_wapi_increase_pn(wapi_ae->msksa.key_ann_id, 1) == WAPI_RETURN_FAILED) {
			ret = WAPI_RETURN_DEASSOCALL;
			rtl_wapi_deauth_all();
			goto exit;
		}
	}	
	pos= (u8*)MEMCPY(pos, wapi_ae->msksa.key_ann_id, WAPI_PN_LEN);

	//Key Data
	*pos = PAIRKEY_LEN;
	pos = pos + 1;
	
	if (wapi_ae->crypto_alg == WPI_SM4) {		
		//rtl_dump_buffer(wapi_ae->msksa.key_ann_id, WAPI_PN_LEN, "AE SMS4 IV :");
		sms4_encrypt(wapi_ae->msksa.key_ann_id, wapi_ae->msksa.nmk, PAIRKEY_LEN, wapi_asue->usksa->usk[uskid].kek, pos);
	
		pos += PAIRKEY_LEN;
	} else if (wapi_ae->crypto_alg == WPI_GCM_SM4) {

		//rtl_dump_buffer(wapi_ae->msksa.nmk, PAIRKEY_LEN, "AE nmk :");
		//rtl_dump_buffer(wapi_ae->msksa.key_ann_id + 4, 12, "AE GCM-SM4 IV :");
	
		sm4_gcm_encrypt(wapi_ae->msksa.nmk, PAIRKEY_LEN, pos,
					wapi_asue->usksa->usk[uskid].kek, wapi_ae->msksa.key_ann_id + 4, 12, 
					NULL, 0, NULL, 0);
		pos += PAIRKEY_LEN;
	}

	//MIC
	u8 *mic_data = tbuf + ETHHDR_LEN + WAI_HDR_LEN;
	int mic_data_len = pos - tbuf - ETHHDR_LEN - WAI_HDR_LEN;
	wapi_hmac_sha256(mic_data, mic_data_len, wapi_asue->usksa->usk[uskid].mak, PAIRKEY_LEN, pos, WAI_MIC_LEN);
	pos += WAI_MIC_LEN;	

	rtl_wapi_set_length(tbuf, pos-tbuf);
	
	rtl_wapi_wai_send(tbuf, pos-tbuf);
	
	rtl_wapi_timer_set(wapi_asue, GENERAL_TIMEOUT, tbuf, pos-tbuf);

	if (wapi_asue->wapi_state == WAPI_USKA_ESTABLISH)
		wapi_asue->wapi_state = WAPI_MSK_ANNOUNCEMENT_SNT;

	ret = WAPI_RETURN_SUCCESS;

exit:

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}

int rtl_wapi_send_usk_agreement_confirm_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{	
	u8 flag = 0, uskid = 0;
	u8 tbuf[MAX_WAI_PKT_LEN];
	u8 *pos = NULL;
	int ret = WAPI_RETURN_FAILED;
	
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	memset(tbuf, 0, sizeof(tbuf));
	
	pos = tbuf;
	pos = rtl_build_ethernet_hdr(pos, wapi_ae->ae_mac, wapi_asue->asue_mac);
	pos = rtl_wapi_build_hdr(pos, ++wapi_asue->txseq, WAI_USK_AGREEMENT_CONFIRMATION);	

	if (wapi_asue->usk_update) {
		flag |= BIT(4);
		uskid = !wapi_asue->usksa->uskid;
	} else {
		flag = 0;
		uskid = wapi_asue->usksa->uskid;
	}

	//Flag	
	*pos = flag;
	pos = pos + 1;

	//BKID
	pos= (u8*)MEMCPY(pos, wapi_asue->bksa->bkid, BKID_LEN);

	//USKID
	*pos = uskid;
	pos = pos + 1;

	//ADDID
	pos= (u8*)MEMCPY(pos, wapi_ae->ae_mac, ETH_ALEN);
	pos= (u8*)MEMCPY(pos, wapi_asue->asue_mac, ETH_ALEN);

	//ASUE challenge
	pos= (u8*)MEMCPY(pos, wapi_asue->Nasue, WAPI_CHALLENGE_LEN);

	//AE IE
	pos= (u8*)MEMCPY(pos, wapi_ae->wapi_ie, wapi_ae->wapi_ie_len);

	//MIC
	u8 *mic_data = tbuf + ETHHDR_LEN + WAI_HDR_LEN;
	int mic_data_len = pos - tbuf - ETHHDR_LEN - WAI_HDR_LEN;
	wapi_hmac_sha256(mic_data, mic_data_len, wapi_asue->usksa->usk[uskid].mak, PAIRKEY_LEN, pos, WAI_MIC_LEN);
	pos += WAI_MIC_LEN;	

	rtl_wapi_set_length(tbuf, pos-tbuf);
	
	rtl_wapi_wai_send(tbuf, pos-tbuf);
	
	rtl_wapi_timer_set(wapi_asue, GENERAL_TIMEOUT, tbuf, pos-tbuf);

	if (wapi_asue->wapi_state == WAPI_USK_AGREEMENT_RSP_RCVD)
		wapi_asue->wapi_state = WAPI_USKA_ESTABLISH;

	ret = WAPI_RETURN_SUCCESS;

exit:

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}

int rtl_wapi_recv_usk_agreement_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{	
	int offset = 0;
	u8 flag = 0, usk_update = 0;
	u8 *bkid = NULL;
	u8 uskid = 0, key_idx = 0;
	u8 *addid = NULL, *Nasue = NULL, *Nae = NULL;
	u8 *WIEasue = NULL;
	u8 wapi_id = 0, wapi_ie_len = 0;
	u8 *mic = NULL;	
	u8 *text = NULL;
	int text_len = 0;
	u8 local_mic[WAI_MIC_LEN];
	int ret = WAPI_RETURN_FAILED;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct wai_hdr *waihdr = (struct wai_hdr *)(wapi_asue->rxfrag->data + ETHHDR_LEN);
	u8 *payload = wapi_asue->rxfrag->data + ETHHDR_LEN + WAI_HDR_LEN;
	int len = wapi_asue->rxfrag->data_len - ETHHDR_LEN - WAI_HDR_LEN;	
	
	//Flag
	flag = payload[0];
	usk_update = BIT(4) & flag;
	offset += WAI_FLAG_LEN;

	//BKID
	bkid = payload + offset;
	if (memcmp(bkid, wapi_asue->bksa->bkid, BKID_LEN)) {
		rtl_wapi_trace(MSG_ERROR, "%s: BKID is not same", __FUNCTION__);
		goto exit;
	}
	offset += BKID_LEN;

	//USKID
	uskid = *(payload + offset);
	offset += 1;

	if (usk_update && uskid != !wapi_asue->usksa->uskid) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid USKID", __FUNCTION__);
		goto exit;
	}

	if (wapi_asue->usk_update)
		key_idx = !wapi_asue->usksa->uskid;
	else 
		key_idx = wapi_asue->usksa->uskid;

	//ADDID
	addid = payload + offset;
	if (memcmp(addid, wapi_ae->ae_mac, ETH_ALEN) || 
		memcmp(addid + ETH_ALEN, wapi_asue->asue_mac, ETH_ALEN)) {
		rtl_wapi_trace(MSG_ERROR, "%s: ADDID is not same", __FUNCTION__);
		goto exit;
	}
	offset += ADDID_LEN;

	//ASUE challenge
	Nasue = payload + offset;
	memcpy(wapi_asue->Nasue, Nasue, WAPI_CHALLENGE_LEN);
	offset += WAPI_CHALLENGE_LEN;

	//AE challenge
	Nae = payload + offset;
	if (memcmp(Nae, wapi_asue->Nae, WAPI_CHALLENGE_LEN)) {
		rtl_wapi_trace(MSG_ERROR, "%s: AE challenge is not same", __FUNCTION__);
		goto exit;
	}
	offset += WAPI_CHALLENGE_LEN;

	//ASUE IE
	WIEasue = payload + offset;
	wapi_id = *WIEasue;
	if (wapi_id != _EID_WAPI_) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid WAPI IE ID", __FUNCTION__);
		goto exit;
	}	
	wapi_ie_len = *(WIEasue + 1);
	if (usk_update == 0 && 
		(memcmp(WIEasue, wapi_asue->wapi_ie, wapi_ie_len) ||
		wapi_ie_len + 2 != wapi_asue->wapi_ie_len)) {
		ret = WAPI_RETURN_DEASSOC;

		rtl_dump_buffer(wapi_asue->wapi_ie, wapi_asue->wapi_ie_len, "WAPI IE from ASUE associate request");
		rtl_dump_buffer(WIEasue, wapi_ie_len + 2, "WAPI IE from USK agreement response packet");
		
		goto exit;
	}	
	offset += 2 + wapi_ie_len;

	//MIC
	mic = payload + offset;

	//Derivation USK
	text_len = strlen(USK_TEXT) + ADDID_LEN + (WAPI_CHALLENGE_LEN << 1);
	text = (u8 *)rtl_allocate_buffer(text_len);
	if(text == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate text failure", __FUNCTION__);
		goto exit;
	}
	u8 *pos = text;
	
	pos= (u8 *)MEMCPY(pos, addid, ADDID_LEN);
	pos= (u8 *)MEMCPY(pos, wapi_asue->Nae, WAPI_CHALLENGE_LEN);
	pos= (u8 *)MEMCPY(pos, wapi_asue->Nasue, WAPI_CHALLENGE_LEN);
	pos= (u8 *)MEMCPY(pos, USK_TEXT, strlen(USK_TEXT));
	
	rtl_wapi_key_derivation(wapi_asue->bksa->bk, BK_LEN, text, text_len, 
		wapi_asue->usksa->usk[key_idx].uek, USKSA_LEN, ISUSK);
	memcpy(wapi_asue->Nae, wapi_asue->usksa->usk[key_idx].ae_challenge, WAPI_CHALLENGE_LEN);

	//Verify MIC
	wapi_hmac_sha256(payload, len-WAI_MIC_LEN, wapi_asue->usksa->usk[key_idx].mak, PAIRKEY_LEN, local_mic, WAI_MIC_LEN);
	if (memcmp(mic, local_mic, WAI_MIC_LEN)) {
		rtl_wapi_trace(MSG_ERROR, "%s: MIC mismatch", __FUNCTION__);
		goto exit;
	}

	rtl_wapi_install_usk(wapi_asue);

	if (wapi_asue->wapi_state == WAPI_USK_AGREEMENT_REQ_SNT)
		wapi_asue->wapi_state = WAPI_USK_AGREEMENT_RSP_RCVD;

	GETSHORT(waihdr->seq_num, wapi_asue->rxseq);

	rtl_wapi_trace(MSG_DEBUG, "%s: wapi_asue->rxseq=%d", __FUNCTION__, wapi_asue->rxseq);
	
	ret = WAPI_RETURN_SUCCESS;
	
exit:

	if (text)
		rtl_free_buffer(text, text_len);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}


int rtl_wapi_send_usk_agreement_request_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	u8 flag = 0, uskid = 0;
	u8 tbuf[MAX_WAI_PKT_LEN];
	u8 *pos = NULL;
	int ret = WAPI_RETURN_FAILED;
	
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	memset(tbuf, 0, sizeof(tbuf));
	
	pos = tbuf; 
	pos = rtl_build_ethernet_hdr(pos, wapi_ae->ae_mac, wapi_asue->asue_mac);
	pos = rtl_wapi_build_hdr(pos, ++wapi_asue->txseq, WAI_USK_AGREEMENT_REQUEST);

	if (wapi_asue->usk_update) {
		flag |= BIT(4);
		uskid = !wapi_asue->usksa->uskid;
	} else {
		flag = 0;
		uskid = wapi_asue->usksa->uskid;
	}

	//Flag	
	*pos = flag;
	pos = pos + 1;

	//BKID
	pos= (u8*)MEMCPY(pos, wapi_asue->bksa->bkid, BKID_LEN);

	//USKID
	*pos = uskid;
	pos = pos + 1;

	//ADDID
	pos= (u8*)MEMCPY(pos, wapi_ae->ae_mac, ETH_ALEN);
	pos= (u8*)MEMCPY(pos, wapi_asue->asue_mac, ETH_ALEN);

	//AE challenge
	if (!(flag & BIT(4)))
		get_random(wapi_asue->Nae, WAPI_CHALLENGE_LEN);

	pos= (u8*)MEMCPY(pos, wapi_asue->Nae, WAPI_CHALLENGE_LEN);		

	rtl_wapi_set_length(tbuf, pos-tbuf);
	
	rtl_wapi_wai_send(tbuf, pos-tbuf);
	
	rtl_wapi_timer_set(wapi_asue, GENERAL_TIMEOUT, tbuf, pos-tbuf);

	if (wapi_asue->wapi_state == WAPI_BKSA_ESTABLISH)
		wapi_asue->wapi_state = WAPI_USK_AGREEMENT_REQ_SNT;

	ret = WAPI_RETURN_SUCCESS;

exit:

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}

int rtl_wapi_send_access_auth_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	int ret = -1;
	u8 *sign_len_pos = NULL;
	u8 tbuf[MAX_WAI_PKT_LEN];
	u8 *pos = NULL;

	tkey *ae_prikey = NULL;
	u8 *sign_data = NULL;
	int sign_data_len = 0;
	u8 sign_value[64] = {0};
	
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	memset(tbuf, 0, sizeof(tbuf));
	
	pos = tbuf;
	pos = rtl_build_ethernet_hdr(pos, wapi_ae->ae_mac, wapi_asue->asue_mac);
	pos = rtl_wapi_build_hdr(pos, ++wapi_asue->txseq, WAI_ACCESS_AUTH_RESPONSE);

	//Flag
	u8 flag = wapi_asue->ae_flag;
	if (wapi_asue->asue_flag & BIT(2))
		flag |= BIT(3);

	if (wapi_ae->wapi_role == WAPI_AE_ASU)
		flag &= (~ BIT(3));
	
	*pos = flag;
	pos = pos + 1;

	//ASUE challenge
	pos= (u8*)MEMCPY(pos, wapi_asue->Nasue, WAPI_CHALLENGE_LEN);

	//AE challenge
	pos= (u8*)MEMCPY(pos, wapi_asue->Nae, WAPI_CHALLENGE_LEN);

	//Access result
	*pos = wapi_asue->access_result;
	pos = pos + 1;

	//ASUE tmp public key
	if (wapi_asue->asue_tmp_pubkey) {
		*pos = wapi_asue->asue_tmp_pubkey->length;
		pos = pos + 1;
		pos= (u8*)MEMCPY(pos, wapi_asue->asue_tmp_pubkey->data, wapi_asue->asue_tmp_pubkey->length);
	} else {
		rtl_wapi_trace(MSG_DEBUG, "%s: wapi_asue->asue_tmp_pubkey == NULL", __FUNCTION__);
		goto exit;
	}

	//AE tmp public key
	if (wapi_asue->ae_tmp_pubkey) {
		*pos = wapi_asue->ae_tmp_pubkey->length;
		pos = pos + 1;
		pos= (u8*)MEMCPY(pos, wapi_asue->ae_tmp_pubkey->data, wapi_asue->ae_tmp_pubkey->length);
	} else {
		rtl_wapi_trace(MSG_DEBUG, "%s: wapi_asue->ae_tmp_pubkey == NULL", __FUNCTION__);
		goto exit;
	}

	//AE identity	
	SETSHORT(pos, wapi_ae->cert_infos->ae_id->id_flag); pos += 2;
	SETSHORT(pos, wapi_ae->cert_infos->ae_id->id_len); pos += 2;
	pos = (u8*)MEMCPY(pos, wapi_ae->cert_infos->ae_id->id_data, wapi_ae->cert_infos->ae_id->id_len);

	//ASUE identity
	if (wapi_asue->asue_id) {
		SETSHORT(pos, wapi_asue->asue_id->id_flag); pos += 2;
		SETSHORT(pos, wapi_asue->asue_id->id_len); pos += 2;
		pos = (u8*)MEMCPY(pos, wapi_asue->asue_id->id_data, wapi_asue->asue_id->id_len);
	} else {
		rtl_wapi_trace(MSG_DEBUG, "%s: wapi_asue->asue_id == NULL", __FUNCTION__);
		goto exit;
	}

	//Certificate verification result
	if (flag & BIT(3)) {
		u8 *cert_rsp = wapi_asue->wai_data + WAI_HDR_LEN + (ETH_ALEN << 1);
		int cert_rsp_len = wapi_asue->wai_data_len - WAI_HDR_LEN - (ETH_ALEN << 1);
		
		pos = (u8*)MEMCPY(pos, cert_rsp , cert_rsp_len);
	}

	//AE signature
	ae_prikey = wapi_ae->cert_infos->ae_prikey;
	sign_data = tbuf + ETHHDR_LEN + WAI_HDR_LEN;
	sign_data_len = pos - tbuf - ETHHDR_LEN - WAI_HDR_LEN;

	if (!x509_ecc_sign(ae_prikey->data, ae_prikey->length, sign_data, sign_data_len, sign_value)) {
		rtl_wapi_trace(MSG_ERROR, "%s: call x509_ecc_sign() fail", __FUNCTION__);
		goto exit;
	}
	*pos = 1;
	pos = pos + 1;	
	
	sign_len_pos = pos;
	pos += 2;/*length*/
	SETSHORT(pos, wapi_ae->cert_infos->ae_id->id_flag); pos += 2;
	SETSHORT(pos, wapi_ae->cert_infos->ae_id->id_len); pos += 2;
	pos= (u8*)MEMCPY(pos, wapi_ae->cert_infos->ae_id->id_data, wapi_ae->cert_infos->ae_id->id_len);	
	
	SETSHORT(pos, wapi_ae->sign_alg.alg_length); pos += 2;
	*pos = wapi_ae->sign_alg.sha256_flag; pos++;
	*pos = wapi_ae->sign_alg.sign_alg; pos++;
	*pos = wapi_ae->sign_alg.sign_para.para_flag; pos++;
	SETSHORT(pos, wapi_ae->sign_alg.sign_para.para_len); pos += 2;
	pos= (u8*)MEMCPY(pos, wapi_ae->sign_alg.sign_para.para_data, wapi_ae->sign_alg.sign_para.para_len);
	SETSHORT(pos, SIGN_LEN); pos += 2;
	pos = (u8*)MEMCPY(pos, sign_value, SIGN_LEN);
	SETSHORT(sign_len_pos, (pos - sign_len_pos - 2));	

	rtl_wapi_set_length(tbuf, pos-tbuf);
	
	rtl_wapi_wai_send(tbuf, pos-tbuf);
	
	rtl_wapi_timer_set(wapi_asue, GENERAL_TIMEOUT, tbuf, pos-tbuf);

	//wapi_asue->wapi_state = WAPI_ACCESS_AUTH_RSP_SNT;

	ret = 0;

exit:

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}

int rtl_wapi_recv_cert_auth_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	int ret = -1;
	int offset = 0;
	tkey *asu_pubkey = NULL;
	u8 *addid = NULL;
	cert_auth_result *presult = NULL;
	tsign_info *asue_sign = NULL, *ae_sign = NULL;	
	int sign_data_len = 0;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	u8 *payload = wapi_asue->wai_data + WAI_HDR_LEN;
	int len = wapi_asue->wai_data_len - WAI_HDR_LEN;

	addid = payload;
	if (memcmp(addid, wapi_ae->ae_mac, ETH_ALEN) || 
		memcmp(addid + ETH_ALEN, wapi_asue->asue_mac, ETH_ALEN)) {
		rtl_wapi_trace(MSG_ERROR, "%s: ADDID is not same", __FUNCTION__);
		return ret;
	}
	offset += (ETH_ALEN << 1);

	//parse certificate verify result
	presult = rtl_wapi_parse_cert_auth_result(payload + offset);	
	if (!presult) {
		rtl_wapi_trace(MSG_ERROR, "%s: parse certificate authenticate result fail", __FUNCTION__);
		goto exit;
	}
	if (memcmp(presult->Nae, wapi_asue->Nae, WAPI_CHALLENGE_LEN)) {
		rtl_wapi_trace(MSG_ERROR, "%s: AE challenge is not same", __FUNCTION__);
		goto exit;
	}
	offset += presult->length + 3;
	sign_data_len = presult->length + 3;

	//parse ASU trusted by ASUE/AE signature
	asue_sign = rtl_wapi_parse_signature(payload + offset);
	if (!asue_sign) {
		rtl_wapi_trace(MSG_ERROR, "%s: ASUE parse ASU signature fail", __FUNCTION__);
		goto exit;
	}
	offset += asue_sign->length + 3;

	if (presult->asue_result == 1) {
		ae_sign = asue_sign;
		asue_sign = NULL;

		if (offset != len) {
			rtl_wapi_trace(MSG_ERROR, "%s: Invalid length", __FUNCTION__);
			goto exit;
		}
	} else {
	
		if (offset == len) {
			ae_sign = asue_sign;
		} else if (len > offset) {
			ae_sign = rtl_wapi_parse_signature(payload + offset);
			if (!ae_sign) {
				rtl_wapi_trace(MSG_ERROR, "%s: AE parse ASU signature fail", __FUNCTION__);
				goto exit;
			}
			offset += ae_sign->length + 3;
			sign_data_len += asue_sign->length + 3;

			if (offset != len) {
				rtl_wapi_trace(MSG_ERROR, "%s: Invalid length", __FUNCTION__);
				goto exit;
			}
		} else {
			rtl_wapi_trace(MSG_ERROR, "%s: Invalid length", __FUNCTION__);
			goto exit;
		}
	}
	
	//AE verify ASU signature
	u8 *data = payload + (ETH_ALEN << 1);
	int data_len = sign_data_len;
	
	asu_pubkey = wapi_ae->cert_infos->asu_pubkey;
	
	if (!x509_ecc_verify(asu_pubkey->data, asu_pubkey->length, data, data_len,
			ae_sign->sign_value.data, ae_sign->sign_value.length)) {
		rtl_wapi_trace(MSG_ERROR, "%s: AE verify ASU signature fail", __FUNCTION__);
		goto exit;
	}

	if (presult->asue_result == CERT_VALID)
		wapi_asue->access_result = ACCESS_SUCCESS;
	else if (presult->asue_result == CERT_ISSUER_UNKNOWN)
		wapi_asue->access_result = CANNOT_VERIFY_CERT;
	else 
		wapi_asue->access_result = CERT_ERROR;

	ret = 0;
	
exit:
	
	if (presult)
		rtl_free_buffer(presult, sizeof(cert_auth_result));
	if (asue_sign && asue_sign != ae_sign)
		rtl_free_buffer(asue_sign, sizeof(tsign_info));
	if (ae_sign)
		rtl_free_buffer(ae_sign, sizeof(tsign_info));

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}

int rtl_wapi_send_cert_auth_request_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{	
	int ret = -1;
	u8 *pos = NULL;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	u8 *tbuf = (u8 *)rtl_allocate_buffer(MAX_WAI_PKT_LEN);
	u8 *rbuf = (u8 *)rtl_allocate_buffer(MAX_WAI_PKT_LEN);

	if (!tbuf || !rbuf) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate tbuf or rbuf fail", __FUNCTION__);
		goto exit;
	}

	int rbuf_len = MAX_WAI_PKT_LEN;

	pos = tbuf;

	pos = rtl_wapi_build_hdr(pos, ++wapi_asue->txseq, WAI_CERT_AUTH_REQUEST);

	//ADDID
	pos= (u8*)MEMCPY(pos, wapi_ae->ae_mac, ETH_ALEN);
	pos= (u8*)MEMCPY(pos, wapi_asue->asue_mac, ETH_ALEN);

	//AE challenge
	get_random(wapi_asue->Nae, WAPI_CHALLENGE_LEN);
	pos= (u8*)MEMCPY(pos, wapi_asue->Nae, WAPI_CHALLENGE_LEN);

	//ASUE challenge
	pos= (u8*)MEMCPY(pos, wapi_asue->Nasue, WAPI_CHALLENGE_LEN);

	//ASUE certificate
	SETSHORT(pos, wapi_asue->asue_cert->cert_flag); pos += 2;
	SETSHORT(pos, wapi_asue->asue_cert->length); pos += 2;
	pos= (u8*)MEMCPY(pos, wapi_asue->asue_cert->data, wapi_asue->asue_cert->length);

	//AE certificate
	SETSHORT(pos, wapi_ae->cert_infos->ae_cert->cert_flag); pos += 2;
	SETSHORT(pos, wapi_ae->cert_infos->ae_cert->length); pos += 2;
	pos= (u8*)MEMCPY(pos, wapi_ae->cert_infos->ae_cert->data, wapi_ae->cert_infos->ae_cert->length);

	//ASUE trust ASU list
	if (wapi_asue->asu_id_list) {
		int i;
	
		*pos = wapi_asue->asu_id_list->type; pos += 1;
		SETSHORT(pos, wapi_asue->asu_id_list->length); pos += 2;
		pos += 1; //resverd
		SETSHORT(pos, wapi_asue->asu_id_list->id_num); pos += 2;

		for (i = 0; i < wapi_asue->asu_id_list->id_num; i++) {
			SETSHORT(pos, wapi_asue->asu_id_list->id->id_flag); pos += 2;
			SETSHORT(pos, wapi_asue->asu_id_list->id->id_len); pos += 2;
			pos= (u8*)MEMCPY(pos, wapi_asue->asu_id_list->id->id_data, wapi_asue->asu_id_list->id->id_len);
		}
	}

	SETSHORT((tbuf + 6), (pos-tbuf));

	if (rtl_wapi_send_msg_to_asu(tbuf, pos-tbuf, rbuf, &rbuf_len, wapi_ae, wapi_asue) < 0)
		goto exit;

	//copy data to wapi_asue->wai_data
	wapi_asue->wai_data = (u8 *)rtl_allocate_buffer(rbuf_len);
	if (!wapi_asue->wai_data) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate wai_data fail", __FUNCTION__);
		goto exit;
	}
	memcpy(wapi_asue->wai_data, rbuf, rbuf_len);
	wapi_asue->wai_data_len = rbuf_len;

	wapi_asue->wapi_state = WAPI_CERT_AUTH_RSP_RCVD;
	
	ret = 0;

exit:

	if (tbuf)
		tbuf = rtl_free_buffer(tbuf, MAX_WAI_PKT_LEN);
		
	if (rbuf)
		rbuf = rtl_free_buffer(rbuf, MAX_WAI_PKT_LEN);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}



int rtl_wapi_recv_access_auth_request_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue)
{
	int offset = 0;
	u8 flag = 0, opt_field = 0;
	u8 *auth_flag = NULL, *Nasue = NULL;
	u8 *asue_tmp_pubkey = NULL, *ae_id = NULL;
	u8 *asue_cert = NULL, *ecdh = NULL;
	u8 *asu_id_list;
	u8 *asue_sign = NULL;	
	tsign_info *s_asue_sign = NULL;
	tkey *asue_pubkey = NULL;
	int ret = -1;	

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct wai_hdr *waihdr = (struct wai_hdr *)(wapi_asue->rxfrag->data + ETHHDR_LEN);
	u8 *payload = wapi_asue->rxfrag->data + ETHHDR_LEN + WAI_HDR_LEN;
	int len = wapi_asue->rxfrag->data_len - ETHHDR_LEN - WAI_HDR_LEN;	
	
	//Flag
	flag = payload[0];
	opt_field = BIT(3) & flag;

	if ((flag & 0x03) != (wapi_asue->ae_flag & 0x03)) {	
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid flag bit 0 or 1", __FUNCTION__);
		return ret;
	}
	wapi_asue->asue_flag = flag;
	offset += WAI_FLAG_LEN;

	//Authentication flag
	auth_flag = payload + offset;
	if (memcmp(auth_flag, wapi_asue->ae_auth_flag, WAPI_AUTHFLAG_LEN)) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid authentication flag", __FUNCTION__);
		return ret;
	}
	offset += WAPI_AUTHFLAG_LEN;

	//ASUE challenge
	Nasue = payload + offset;
	memcpy(wapi_asue->Nasue, Nasue, WAPI_CHALLENGE_LEN);
	offset += WAPI_CHALLENGE_LEN;

	//ASUE tmp public key
	asue_tmp_pubkey = payload + offset;
	if (wapi_asue->asue_tmp_pubkey)
		rtl_free_buffer(wapi_asue->asue_tmp_pubkey, sizeof(wapi_data));
	wapi_asue->asue_tmp_pubkey = (wapi_data *)rtl_allocate_buffer(sizeof(wapi_data));
	if (!wapi_asue->asue_tmp_pubkey) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate asue_tmp_pubkey fail", __FUNCTION__);
		goto exit;	
	}	
	wapi_asue->asue_tmp_pubkey->length = *asue_tmp_pubkey;
	memcpy(wapi_asue->asue_tmp_pubkey->data, asue_tmp_pubkey + 1, wapi_asue->asue_tmp_pubkey->length);
	offset += 1 + wapi_asue->asue_tmp_pubkey->length;

	//AE identity
	ae_id = payload + offset;
	if (rtl_wapi_compare_identity(ae_id, wapi_ae->cert_infos->ae_id)) {
		rtl_wapi_trace(MSG_ERROR, "%s: not same ae id", __FUNCTION__);
		goto exit;
	}
	offset += wapi_ae->cert_infos->ae_id->id_len + 4;

	//ASUE certificate
	asue_cert = payload + offset;
	if (wapi_asue->asue_cert)
		rtl_free_buffer(wapi_asue->asue_cert, sizeof(cert_id));
	wapi_asue->asue_cert = (cert_id *)rtl_allocate_buffer(sizeof(cert_id));
	if (!wapi_asue->asue_cert) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate asue_cert fail", __FUNCTION__);
		goto exit;
	}	
	GETSHORT(asue_cert, wapi_asue->asue_cert->cert_flag); 
	GETSHORT((asue_cert + 2), wapi_asue->asue_cert->length);
	memcpy(wapi_asue->asue_cert->data, asue_cert + 4, wapi_asue->asue_cert->length);
	offset += wapi_asue->asue_cert->length + 4;

	//Get ASUE ID from certificate
	if (wapi_asue->asue_id)
		rtl_free_buffer(wapi_asue->asue_id, sizeof(wai_fixdata_id));
	wapi_asue->asue_id = (wai_fixdata_id *)rtl_allocate_buffer(sizeof(wai_fixdata_id));
	if (!wapi_asue->asue_id) {
		rtl_wapi_trace(MSG_ERROR, "%s:%d allocate wapi_asue->asue_id fail.", __FUNCTION__, __LINE__);
		goto exit;
	}
	rtl_wapi_fixdata_id(wapi_asue->asue_cert, wapi_asue->asue_id);

	//ECDH parameter
	ecdh = payload + offset;
	if (rtl_wapi_compare_ecdh(ecdh, &wapi_ae->ecdh)) {
		rtl_wapi_trace(MSG_ERROR, "%s: not same ecdh", __FUNCTION__);
		goto exit;
	}
	offset += wapi_ae->ecdh.para_len + 3;

	//ASUE trust asu list
	if (opt_field) {
		asu_id_list = payload + offset;
		if (rtl_check_asu_id_list(asu_id_list, wapi_asue)) {
			rtl_wapi_trace(MSG_ERROR, "%s: check asu list trusted by asue fail", __FUNCTION__);
			goto exit;
		}
		offset += wapi_asue->asu_id_list->length + 3;
	}

	//ASUE signature
	asue_sign = payload + offset;
	s_asue_sign = rtl_wapi_parse_signature(asue_sign);
	if (!s_asue_sign) {
		rtl_wapi_trace(MSG_ERROR, "%s: parse ause signature fail", __FUNCTION__);
		goto exit;
	}

	//get ASUE public key from ASUE certificate
	asue_pubkey = rtl_wapi_cert_get_pubkey(wapi_asue->asue_cert);
	if (!asue_pubkey) {
		rtl_wapi_trace(MSG_ERROR, "%s: get asue public key fail", __FUNCTION__);
		goto exit;
	}

	//Verify ASUE signature
	if (!x509_ecc_verify(asue_pubkey->data, asue_pubkey->length, payload, offset, 
			s_asue_sign->sign_value.data, s_asue_sign->sign_value.length)) {
		rtl_wapi_trace(MSG_ERROR, "%s: verify asue signature fail", __FUNCTION__);
		goto exit;
	}

	wapi_asue->wapi_state = WAPI_ACCESS_AUTH_REQ_RCVD;

	GETSHORT(waihdr->seq_num, wapi_asue->rxseq);

	rtl_wapi_trace(MSG_DEBUG, "%s: wapi_asue->rxseq=%d", __FUNCTION__, wapi_asue->rxseq);
	
	ret = 0;
	
exit:

	if (s_asue_sign)
		rtl_free_buffer(s_asue_sign, sizeof(tsign_info));
	if (asue_pubkey)
		rtl_free_buffer(asue_pubkey, sizeof(tkey));
	
	if (ret < 0) {
		if (wapi_asue->asue_tmp_pubkey)
			wapi_asue->asue_tmp_pubkey = rtl_free_buffer(wapi_asue->asue_tmp_pubkey, sizeof(wapi_data));
		if (wapi_asue->asue_cert)
			wapi_asue->asue_cert = rtl_free_buffer(wapi_asue->asue_cert, sizeof(cert_id));
		if (wapi_asue->asue_id)
			wapi_asue->asue_id = rtl_free_buffer(wapi_asue->asue_id, sizeof(wai_fixdata_id));
	}

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}

int rtl_wapi_install_usk(struct wapi_asue_st *wapi_asue)
{
	u8 key_idx;
	
	if (wapi_asue->usk_update)
		key_idx = !wapi_asue->usksa->uskid;
	else 
		key_idx = wapi_asue->usksa->uskid;
	
	u8 *key = wapi_asue->usksa->usk[key_idx].uek;

	if (0 != rtl_wapi_set_usk(wapi_asue->ae_mac, wapi_asue->asue_mac, key_idx, key, (PAIRKEY_LEN << 1)))
	{
		rtl_wapi_trace(MSG_ERROR, "%s: Failed to set USK to the driver", __FUNCTION__);
		return -1;
	}

	return 0;
}

int rtl_wapi_install_msk(struct wapi_ae_st *wapi_ae)
{
	u8 key_idx;

	//Derivation MSK
	u8 derivedKey[PAIRKEY_LEN << 1];
	rtl_wapi_key_derivation(wapi_ae->msksa.nmk, PAIRKEY_LEN, (u8 *)MSK_TEXT, strlen(MSK_TEXT), 
			derivedKey, PAIRKEY_LEN << 1, 0);

	if (wapi_ae->msk_update)
		key_idx = !wapi_ae->msksa.mskid;
	else 
		key_idx = wapi_ae->msksa.mskid;
	
	memcpy(wapi_ae->msksa.msk[key_idx].mek, derivedKey, PAIRKEY_LEN);
	memcpy(wapi_ae->msksa.msk[key_idx].mck, derivedKey + PAIRKEY_LEN, PAIRKEY_LEN);	
	
	u8 *key = wapi_ae->msksa.msk[key_idx].mek;
	
	if (0 != rtl_wapi_set_msk(wapi_ae->ae_mac, key_idx, wapi_ae->msksa.key_ann_id, WAI_KEY_AN_ID_LEN, key, PAIRKEY_LEN << 1))
	{
		rtl_wapi_trace(MSG_ERROR, "%s: Failed to set MSK to the driver.", __FUNCTION__);
		return -1;
	}

	return 0;
}
