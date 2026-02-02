#include <net/if.h>
#include <linux/sockios.h>

#include "wapi_asue.h"
#include "l2_packet/l2_packet.h"
#include "wapi_alogrithm.h"
#include "x509.h"

struct wapi_asue_st *s_asue = NULL;

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
	
	rtl_wapi_asue_free(wapi_asue);	
}

void rtl_wapi_process_assoc_event(u8 *ae_mac, u8 *asue_mac, u8 *wapi_ie, u8 ie_len)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);		

	struct wapi_asue_st *wapi_asue = rtl_get_asue_by_mac(asue_mac);

	if (wapi_asue == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: wapi_asue is NULL", __FUNCTION__);
		return;
	}

	if (memcmp(wapi_asue->ae_mac, ae_mac, ETH_ALEN) != 0) {
		memcpy(wapi_asue->ae_mac, ae_mac, ETH_ALEN);
		rtl_wapi_trace(MSG_DEBUG, "%s: new ae_mac :" MACSTR, __FUNCTION__, MAC2STR(ae_mac));
	}

	if (wapi_ie && 
	   (wapi_asue->asue_wapi_ie_len != ie_len || 
		memcmp(wapi_asue->asue_wapi_ie, wapi_ie, ie_len) != 0)) {
		memcpy(wapi_asue->asue_wapi_ie, wapi_ie, ie_len);
		wapi_asue->asue_wapi_ie_len = ie_len;
	}
	
	if (wapi_asue->auth_type == WAPI_CERT) {		
		wapi_asue->wapi_state = WAPI_IDLE;		
	} else if (wapi_asue->auth_type == WAPI_PSK) {
		//generate bk and bkid
		rtl_wapi_bk_derivation(wapi_asue->psk_infos->kv , wapi_asue->psk_infos->kl, wapi_asue);
	}

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);
}

int rtl_wapi_process_disassoc_event(u8 *ae_mac, u8 *asue_mac)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	if (ae_mac == NULL || asue_mac == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: ae_mac or asue_mac is NULL", __FUNCTION__);
		return -1;
	}	
	
	rtl_wapi_trace(MSG_DEBUG, "%s: ae_mac = " MACSTR " asue_mac = " MACSTR ,__FUNCTION__, MAC2STR(ae_mac), MAC2STR(asue_mac));

	struct wapi_asue_st *wapi_asue = rtl_get_asue_by_mac(asue_mac);

	if (wapi_asue == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: wapi_asue is NULL", __FUNCTION__);
		return -1;
	}

	if (memcmp(wapi_asue->ae_mac, ae_mac, ETH_ALEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: ae_mac is not same", __FUNCTION__);
		return -1;
	}

	rtl_wapi_deauth(wapi_asue);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);

	return 0;
}

struct wapi_asue_st *rtl_get_asue_by_mac(u8 *asue_mac)
{
	struct wapi_asue_st *wapi_asue = s_asue;
	
	if (wapi_asue == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: wapi_asue is NULL", __FUNCTION__);
		return NULL;
	}
	
	if (memcmp(wapi_asue->asue_mac, asue_mac, ETH_ALEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: asue_mac :" MACSTR "is not same", __FUNCTION__, MAC2STR(asue_mac));
		return NULL;
	}

	return wapi_asue;
}

int rtl_wapi_asue_init(struct wapi_config *config)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	int ret = -1;

	if (s_asue == NULL) {
		s_asue = (struct wapi_asue_st *)rtl_allocate_buffer(sizeof(struct wapi_asue_st));
		if (s_asue == NULL) {
			rtl_wapi_trace(MSG_ERROR, "%s(): allocate s_asue fail", __FUNCTION__);
			goto exit;
		}
	}	
	struct wapi_asue_st *pasue = s_asue;

	snprintf(pasue->ifname, sizeof(pasue->ifname), "%s", config->ifname);
	memcpy(pasue->ae_mac, config->bssid, ETH_ALEN);
	memcpy(pasue->asue_mac, config->own_addr, ETH_ALEN);
	pasue->auth_type = config->auth_type;
	pasue->crypto_alg = config->crypto_alg;
	
	rtl_wapi_trace(MSG_DEBUG, "%s: ifname = %s ae_mac = " MACSTR " asue_mac = " MACSTR " auth_type = %d  crypto_alg = %d", 
			__FUNCTION__, pasue->ifname, MAC2STR(pasue->ae_mac), MAC2STR(pasue->asue_mac), pasue->auth_type, pasue->crypto_alg);

	if (pasue->asue_wapi_ie)
		rtl_free_buffer(pasue->asue_wapi_ie, MAX_WAPI_IE_LEN);
	pasue->asue_wapi_ie = (u8 *)rtl_allocate_buffer(MAX_WAPI_IE_LEN);
	if (!pasue->asue_wapi_ie) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate asue_wapi_ie fail", __FUNCTION__);
		goto exit;
	}
	memcpy(pasue->asue_wapi_ie, config->asue_wapi_ie, config->asue_wapi_ie_len);
	pasue->asue_wapi_ie_len = config->asue_wapi_ie_len;
	rtl_dump_buffer(pasue->asue_wapi_ie, pasue->asue_wapi_ie_len, "WAPI IE from ASUE associate request");

	if (pasue->ae_wapi_ie)
		rtl_free_buffer(pasue->ae_wapi_ie, MAX_WAPI_IE_LEN);
	pasue->ae_wapi_ie = (u8 *)rtl_allocate_buffer(MAX_WAPI_IE_LEN);
	if (!pasue->ae_wapi_ie) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate ae_wapi_ie fail", __FUNCTION__);
		goto exit;
	}
	memcpy(pasue->ae_wapi_ie, config->ae_wapi_ie, config->ae_wapi_ie_len);
	pasue->ae_wapi_ie_len = config->ae_wapi_ie_len;
	rtl_dump_buffer(pasue->ae_wapi_ie, pasue->ae_wapi_ie_len, "WAPI IE from AE Beacon/ProbeResp");

	//init usksa
	pasue->usksa = (struct wapi_usksa *)rtl_allocate_buffer(sizeof(struct wapi_usksa));
	if (!pasue->usksa) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate usksa fail", __FUNCTION__);
		goto exit;
	}

	//init bksa
	pasue->bksa = (struct wapi_bksa_cache *)rtl_allocate_buffer(sizeof(struct wapi_bksa_cache));
	if (!pasue->bksa) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate bksa fail", __FUNCTION__);
		goto exit;
	}

	if (pasue->auth_type == WAPI_CERT) {
		char *ca_cert_file = config->para.ca_cert_file;
		char *asu_cert_file = config->para.asu_cert_file;
		char *asue_cert_file = config->para.asue_cert_file;
		ret = rtl_wapi_cert_init(pasue, ca_cert_file, asu_cert_file, asue_cert_file);
		if (ret < 0)
			goto exit;

		rtl_wapi_sign_alg_init(&pasue->sign_alg);
		
	} else if (pasue->auth_type == WAPI_PSK) {
		ret = rtl_wapi_psk_init(pasue, config->para.kt, config->para.kl, config->para.kv);
	}

	rtl_wapi_init_tx_mcast_pn(&pasue->msksa);	

	ret = 0;

exit:

	if (ret < 0)
		rtl_wapi_asue_free(pasue);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d) <<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}

int rtl_wapi_asue_free(struct wapi_asue_st *wapi_asue)
{
	if (wapi_asue) {
		
		wapi_asue->rxfrag = rtl_wapi_free_rxfrag(wapi_asue->rxfrag);		
		wapi_asue->ae_asu_id = rtl_free_buffer(wapi_asue->ae_asu_id, sizeof(wai_fixdata_id));
		wapi_asue->ae_id = rtl_free_buffer(wapi_asue->ae_id, sizeof(wai_fixdata_id));
		wapi_asue->usksa = rtl_free_buffer(wapi_asue->usksa, sizeof(struct wapi_usksa));
		wapi_asue->bksa = rtl_free_buffer(wapi_asue->bksa, sizeof(struct wapi_bksa_cache));
		wapi_asue->ae_cert = rtl_free_buffer(wapi_asue->ae_cert, sizeof(cert_id));
		wapi_asue->asue_tmp_pubkey = rtl_free_buffer(wapi_asue->asue_tmp_pubkey, sizeof(wapi_data));
		wapi_asue->ae_tmp_pubkey = rtl_free_buffer(wapi_asue->ae_tmp_pubkey, sizeof(wapi_data));
		wapi_asue->asue_tmp_prikey = rtl_free_buffer(wapi_asue->asue_tmp_prikey, sizeof(wapi_data));
		wapi_asue->wai_data= rtl_free_buffer(wapi_asue->wai_data, wapi_asue->wai_data_len);
		wapi_asue->asue_wapi_ie = rtl_free_buffer(wapi_asue->asue_wapi_ie, MAX_WAPI_IE_LEN);
		wapi_asue->ae_wapi_ie = rtl_free_buffer(wapi_asue->ae_wapi_ie, MAX_WAPI_IE_LEN);

		rtl_wapi_cert_free(&wapi_asue->cert_infos);
		wapi_asue->psk_infos = rtl_free_buffer(wapi_asue->psk_infos, sizeof(psk_info));
		
		rtl_del_timeout_entry(&wapi_asue->entry);	

		wapi_asue = rtl_free_buffer(wapi_asue, sizeof(struct wapi_asue_st));

		s_asue = NULL;
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
	struct wapi_asue_st *wapi_asue = rtl_get_asue_by_mac(l2_hdr->h_source);
	if (!wapi_asue) {
		rtl_wapi_trace(MSG_ERROR, "%s: wapi_asue is NULL", __FUNCTION__);
		return -1;
	}

	if (rtk_get_interface_mtu(wapi_asue->ifname, &mtu) < 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: get interface %s MTU fail", __FUNCTION__, wapi_asue->ifname);
		return -1;
	}

	if (len - ETHHDR_LEN > mtu)
		return rtl_wapi_send_wai_fragment_packet(buf, len, mtu);
	else
		return rtl_wapi_send_wai_packet(buf, len);
	
}

void rtl_wapi_wai_receive(u8 *buf, size_t len)
{
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	struct l2_ethhdr *l2_hdr = (struct l2_ethhdr *)buf;
	
	struct wapi_asue_st *wapi_asue = rtl_get_asue_by_mac(l2_hdr->h_dest);

	if (!wapi_asue) {
		rtl_wapi_trace(MSG_ERROR, "%s: wapi_asue is NULL", __FUNCTION__);
		return;
	}
	if (wapi_asue->auth_type != WAPI_CERT && wapi_asue->auth_type != WAPI_PSK) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid auth_type", __FUNCTION__);
		return;
	}

	struct wapi_rxfrag rxbuf ,*temp_rxbuf= NULL;
	rxbuf.data = buf;
	rxbuf.data_len = len;
	temp_rxbuf = rtl_wapi_defrag(wapi_asue, &rxbuf);
	if (temp_rxbuf)
		rtl_wapi_rx_wai(wapi_asue);

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

static int rtl_wapi_check_wai_frame(struct wapi_asue_st *wapi_asue)
{
	u16 version = 0, rxseq = 0, frmlen = 0;
	
	u8 *buf = wapi_asue->rxfrag->data + ETHHDR_LEN;
	int len = wapi_asue->rxfrag->data_len - ETHHDR_LEN;
	struct wai_hdr *hdr = (struct wai_hdr *)buf;
	
	if (len < WAI_HDR_LEN) {
		rtl_wapi_trace(MSG_ERROR, "%s: WAI frame too short, len %d", __FUNCTION__, len);
		return -1;
	}
	
	version = ((hdr->version[0]<<8)| hdr->version[1]);
	GETSHORT(hdr->version, version);
	if ( version != WAI_VERSION) {
		rtl_wapi_trace(MSG_ERROR, "%s: WAI frame Version(%u) is wrong", __FUNCTION__, version);
		return -1;
	}

	if (hdr->type != WAI_TYPE) {
		rtl_wapi_trace(MSG_ERROR, "%s: WAI frame type(%u) is wrong", __FUNCTION__, hdr->type);
		return -1;
	}
	
	if ((wapi_asue->auth_type == WAPI_PSK && hdr->stype < WAI_USK_AGREEMENT_REQUEST)
		|| hdr->stype < WAI_AUTHACTIVE) {
		rtl_wapi_trace(MSG_ERROR, "%s: WAI frame stype(%u) is wrong ",__FUNCTION__, hdr->stype);
		return -1;
	}
	
	GETSHORT((buf + WAI_LENGTH_OFFSET), frmlen);
	if (len != frmlen) {
		rtl_wapi_trace(MSG_ERROR, "%s: WAI frame length(%u) is wrong", __FUNCTION__, frmlen);
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
	int ret = -1;
	
	if (wapi_asue->asue_tmp_pubkey == NULL)
		wapi_asue->asue_tmp_pubkey = (wapi_data *)rtl_allocate_buffer(sizeof(wapi_data));
	if (wapi_asue->asue_tmp_pubkey == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate asue_tmp_pubkey fail", __FUNCTION__);
		goto exit;
	}

	if (wapi_asue->asue_tmp_prikey == NULL)
		wapi_asue->asue_tmp_prikey = (wapi_data *)rtl_allocate_buffer(sizeof(wapi_data));
	if (wapi_asue->asue_tmp_prikey == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate asue_tmp_prikey fail", __FUNCTION__);
		goto exit;
	}	

	/* get the public key and the private key for ECC */
	if (ecc192_genkey(wapi_asue->asue_tmp_prikey->data, wapi_asue->asue_tmp_pubkey->data))
		goto exit;

	wapi_asue->asue_tmp_prikey->length = SECKEY_LEN;
	wapi_asue->asue_tmp_pubkey->length = PUBKEY2_LEN;

	ret = 0;

exit:

	if (ret < 0) {
		if (wapi_asue->asue_tmp_prikey)
			wapi_asue->asue_tmp_prikey = rtl_free_buffer(wapi_asue->asue_tmp_prikey, sizeof(wapi_data));
		if (wapi_asue->asue_tmp_pubkey)
			wapi_asue->asue_tmp_pubkey = rtl_free_buffer(wapi_asue->asue_tmp_pubkey, sizeof(wapi_data));
	}

	return ret;
}

static int rtl_wapi_certauthbk_derivation(struct wapi_asue_st *wapi_asue)
{
	char input_text[] = "base key expansion for key and additional nonce";
	u8 text[256] = {0};
	u8 temp_out[48] = {0};
	u8  ecdhkey[24] = {0};
	int ret = WAPI_RETURN_FAILED;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);	

	if (!ecc192_ecdh(wapi_asue->asue_tmp_prikey->data, wapi_asue->ae_tmp_pubkey->data, ecdhkey)) {
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
	
	ret = WAPI_RETURN_SUCCESS;

exit:	

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}

static u8 *rtl_build_ethernet_hdr(u8 *buffer, u8 *src_mac, u8 *dest_mac)
{
	struct l2_ethhdr *l2_hdr = (struct l2_ethhdr *)buffer;
	memcpy(l2_hdr->h_dest, dest_mac, ETH_ALEN);
	memcpy(l2_hdr->h_source, src_mac, ETH_ALEN);
	l2_hdr->h_proto = htons(ETH_P_WAI);	

	return (u8 *)(l2_hdr + 1);
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
	SETSHORT((pos + ETHHDR_LEN + WAI_LENGTH_OFFSET), (length - ETHHDR_LEN));
}

void rtl_wapi_rx_wai(struct wapi_asue_st *wapi_asue)
{
	struct wai_hdr *hdr = NULL;
	int ret = 0;
	u16 plen, frmlen = 0;

	u8 *buf = wapi_asue->rxfrag->data;
	
	if (wapi_asue->auth_type == WAPI_CERT && 
		(!wapi_asue->cert_infos || !wapi_asue->cert_infos->asue_cert || !wapi_asue->cert_infos->ca_cert)) {
		rtl_wapi_trace(MSG_ERROR, "No cert");
		goto exit;
	}

	hdr = (struct wai_hdr *)(buf + ETHHDR_LEN);
	if (rtl_wapi_check_wai_frame(wapi_asue)) {
		rtl_wapi_trace(MSG_ERROR, "%s: WAI frame is wrong", __FUNCTION__);
		goto exit;
	}
	
	frmlen = ntohs(hdr->length);
	plen = frmlen - WAI_HDR_LEN;
	rtl_wapi_trace(MSG_DEBUG, "%s  plen = %d, hdr->length = %d", __FUNCTION__, plen, frmlen);

	switch (wapi_asue->wapi_state) {

		case WAPI_IDLE:
			if (hdr->stype == WAI_AUTHACTIVE) {
				rtl_wapi_trace(MSG_DEBUG, "%s: received WAI_AUTHACTIVE", __FUNCTION__);

				ret = rtl_wapi_recv_auth_active_pkt(wapi_asue);
				if (ret == WAPI_RETURN_SUCCESS) {
					ret = rtl_wapi_send_access_auth_request_pkt(wapi_asue);
				}
			}			
			break;

		case WAPI_ACCESS_AUTH_REQ_SNT:
			if (hdr->stype == WAI_ACCESS_AUTH_RESPONSE) {
				rtl_wapi_trace(MSG_DEBUG, "%s: received WAI_ACCESS_AUTH_RESPONSE", __FUNCTION__);
				rtl_wapi_timer_reset(wapi_asue);

				ret = rtl_wapi_recv_access_auth_response_pkt(wapi_asue);
				if (ret == WAPI_RETURN_DEASSOC)
					rtl_wapi_deauth(wapi_asue);
				else if (ret == WAPI_RETURN_SUCCESS)
					rtl_wapi_certauthbk_derivation(wapi_asue);
			}
			break;

		case WAPI_BKSA_ESTABLISH:
		case WAPI_MSKA_ESTABLISH:
			if (hdr->stype == WAI_USK_AGREEMENT_REQUEST) {
				rtl_wapi_trace(MSG_DEBUG, "%s: received WAI_USK_AGREEMENT_REQUEST", __FUNCTION__);

				ret = rtl_wapi_recv_usk_agreement_request_pkt(wapi_asue);

				if (ret == WAPI_RETURN_SUCCESS)
					ret = rtl_wapi_send_usk_agreement_response_pkt(wapi_asue);
			}
			break;

		case WAPI_USK_AGREEMENT_RSP_SNT:
			if (hdr->stype == WAI_USK_AGREEMENT_CONFIRMATION) {				
				rtl_wapi_trace(MSG_DEBUG, "%s: received WAI_USK_AGREEMENT_CONFIRMATION", __FUNCTION__);
				rtl_wapi_timer_reset(wapi_asue);

				ret = rtl_wapi_recv_usk_agreement_confirm_pkt(wapi_asue);
				if (ret == WAPI_RETURN_DEASSOC)
					rtl_wapi_deauth(wapi_asue);				
			}
			break;

		case WAPI_USKA_ESTABLISH:
			if (hdr->stype == WAI_MSK_ANNOUNCEMENT) {
				rtl_wapi_trace(MSG_DEBUG, "%s: received WAI_MSK_ANNOUNCEMENT", __FUNCTION__);

				ret = rtl_wapi_recv_msk_announcement_pkt(wapi_asue);
			
				if (ret == WAPI_RETURN_SUCCESS)
					ret = rtl_wapi_send_msk_announcement_response_pkt(wapi_asue);
			}
			break;		
		
		default:
			rtl_wapi_trace(MSG_DEBUG, "%s: wapi state %d", __FUNCTION__,wapi_asue->wapi_state);
			break;
	}

exit:

	wapi_asue->rxfrag = rtl_wapi_free_rxfrag(wapi_asue->rxfrag); 
}


int rtl_wapi_send_msk_announcement_response_pkt(struct wapi_asue_st *wapi_asue)
{
	int ret = WAPI_RETURN_FAILED;
	u8 tbuf[MAX_WAI_PKT_LEN];
	u8 *pos = NULL;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	memset(tbuf, 0, sizeof(tbuf));

	pos = tbuf;
	pos = rtl_build_ethernet_hdr(pos, wapi_asue->asue_mac, wapi_asue->ae_mac);
	pos = rtl_wapi_build_hdr(pos, ++wapi_asue->txseq, WAI_MSK_ANNOUNCEMENT_RESPONSE);

	u8 *payload = wapi_asue->rxfrag->data + ETHHDR_LEN + WAI_HDR_LEN;
	u8 uskid = payload[2];

	//Flag + MSKID + USKID + ADDID	
	pos = (u8*)MEMCPY(pos, payload, WAI_FLAG_LEN + 1 + 1 + ADDID_LEN);

	//Key announce ID	
	pos = (u8*)MEMCPY(pos, wapi_asue->msksa.key_ann_id, WAPI_PN_LEN);

	//MIC
	u8 *mic_data = tbuf + ETHHDR_LEN + WAI_HDR_LEN;
	int mic_data_len = pos - tbuf - ETHHDR_LEN - WAI_HDR_LEN;
	wapi_hmac_sha256(mic_data, mic_data_len, wapi_asue->usksa->usk[uskid].mak, PAIRKEY_LEN, pos, WAI_MIC_LEN);
	pos += WAI_MIC_LEN;

	rtl_wapi_set_length(tbuf, pos-tbuf);
	rtl_wapi_wai_send(tbuf, pos-tbuf);

	//rtl_wapi_timer_set(wapi_asue, GENERAL_TIMEOUT, tbuf, pos-tbuf);

	//wapi_asue->wapi_state = WAPI_MSK_ANNOUNCEMENT_RSP_SNT;

	wapi_asue->wapi_state = WAPI_MSKA_ESTABLISH;

	ret = WAPI_RETURN_SUCCESS;

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}


int rtl_wapi_recv_msk_announcement_pkt(struct wapi_asue_st *wapi_asue)
{
	int offset = 0;
	u8 flag = 0;
	u8 mskid = 0, uskid = 0;
	u8 *addid = NULL;
	u8 *PN = NULL, *key_ann_id = NULL;
	u8 *key_data = NULL, *mic = NULL;
	u8 local_mic[WAI_MIC_LEN] = {0};
	u8 tmp_msk[PAIRKEY_LEN] = {0};
	
	int ret = WAPI_RETURN_FAILED;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct wai_hdr *waihdr = (struct wai_hdr *)(wapi_asue->rxfrag->data + ETHHDR_LEN);
	u8 *payload = wapi_asue->rxfrag->data + ETHHDR_LEN + WAI_HDR_LEN;
	int len = wapi_asue->rxfrag->data_len - ETHHDR_LEN - WAI_HDR_LEN;

	//Flag
	flag = payload[0];
	if (flag & BIT(5)) {
		rtl_wapi_trace(MSG_ERROR, "%s: Not support STAKey", __FUNCTION__);
		goto exit;
	}
	offset += WAI_FLAG_LEN;

	//MSKID
	mskid = payload[offset];
	wapi_asue->msksa.mskid = mskid;
	offset += 1;

	//USKID
	uskid = payload[offset];
	offset += 1;

	//ADDID
	addid = payload + offset;
	if (memcmp(addid, wapi_asue->ae_mac, ETH_ALEN) != 0 || 
		memcmp(addid + ETH_ALEN , wapi_asue->asue_mac, ETH_ALEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: addid is not same", __FUNCTION__);
		goto exit;
	}
	offset += ADDID_LEN;

	//Data PN
	PN = payload + offset;
	offset += WAPI_PN_LEN;

	//Key announce ID
	key_ann_id = payload + offset;
	if (memcmp(key_ann_id, wapi_asue->msksa.key_ann_id, WAPI_PN_LEN) < 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: invalid key_ann_id", __FUNCTION__);
		goto exit;
	}
	memcpy(wapi_asue->msksa.key_ann_id, key_ann_id, WAPI_PN_LEN);
	offset += WAPI_PN_LEN;

	//Key Data
	key_data = payload + offset;
	if (*key_data != PAIRKEY_LEN) {
		rtl_wapi_trace(MSG_ERROR, "%s: invalid key length", __FUNCTION__);
		goto exit;
	}
	memcpy(tmp_msk, key_data + 1, PAIRKEY_LEN);

	if (wapi_asue->crypto_alg == WPI_SM4) {
		sms4_encrypt(wapi_asue->msksa.key_ann_id, key_data + 1, PAIRKEY_LEN, wapi_asue->usksa->usk[uskid].kek, tmp_msk);
	} else if (wapi_asue->crypto_alg == WPI_GCM_SM4) {
		
		rtl_dump_buffer(wapi_asue->msksa.key_ann_id + 4, 12, "ASUE GCM-SM4 IV :");
	
		sm4_gcm_decrypt(key_data + 1, PAIRKEY_LEN, tmp_msk,
					wapi_asue->usksa->usk[uskid].kek, wapi_asue->msksa.key_ann_id + 4, 12, 
					NULL, 0, NULL, 0);

		rtl_dump_buffer(tmp_msk, PAIRKEY_LEN, "ASUE nmk :");
	}
	memcpy(wapi_asue->msksa.nmk, tmp_msk, PAIRKEY_LEN);
	offset += PAIRKEY_LEN + 1;

	//MIC
	mic = payload + offset;

	wapi_hmac_sha256(payload, len - WAI_MIC_LEN, wapi_asue->usksa->usk[uskid].mak, PAIRKEY_LEN, local_mic, WAI_MIC_LEN);
	if (memcmp(mic, local_mic, WAI_MIC_LEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: MIC mismatch", __FUNCTION__);
		goto exit;
	}

	GETSHORT(waihdr->seq_num, wapi_asue->rxseq);

	rtl_wapi_trace(MSG_DEBUG, "%s: wapi_asue->rxseq=%d", __FUNCTION__, wapi_asue->rxseq);	

	wapi_asue->wapi_state = WAPI_MSK_ANNOUNCEMENT_RCVD;

	rtl_wapi_install_msk(wapi_asue);
	
	ret = WAPI_RETURN_SUCCESS;

exit:

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}


int rtl_wapi_recv_usk_agreement_confirm_pkt(struct wapi_asue_st *wapi_asue)
{
	int offset = 0;
	u8 flag = 0;
	u8 usk_update = 0, uskid = 0;
	u8 *bkid = NULL, *add_id = NULL;
	u8 *Nasue = NULL, *ae_ie = NULL, *mic = NULL;	
	int ret = WAPI_RETURN_FAILED;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct wai_hdr *waihdr = (struct wai_hdr *)(wapi_asue->rxfrag->data + ETHHDR_LEN);
	u8 *payload = wapi_asue->rxfrag->data + ETHHDR_LEN + WAI_HDR_LEN;
	int len = wapi_asue->rxfrag->data_len - ETHHDR_LEN - WAI_HDR_LEN;

	//Flag
	flag = payload[0];
	if (flag & BIT(4))
		usk_update = 1;
	offset += WAI_FLAG_LEN;

	//BKID
	bkid = payload + offset;
	if (memcmp(bkid, wapi_asue->bksa->bkid, BKID_LEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: bk_id is not same", __FUNCTION__);
		goto exit;
	}
	offset += BKID_LEN;

	//USKID
	uskid = *(payload + offset);
	if (usk_update && uskid != !wapi_asue->usksa->uskid) {
		rtl_wapi_trace(MSG_ERROR, "%s: usk_id is not same", __FUNCTION__);
		goto exit;
	}
	offset += 1;

	//ADDID
	add_id = payload + offset;
	if (memcmp(add_id, wapi_asue->ae_mac, ETH_ALEN) != 0 || 
		memcmp(add_id + ETH_ALEN , wapi_asue->asue_mac, ETH_ALEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: add_id is not same", __FUNCTION__);
		goto exit;
	}
	offset += ADDID_LEN;

	//ASUE challenge
	Nasue = payload + offset;
	if (memcmp(Nasue, wapi_asue->Nasue, WAPI_CHALLENGE_LEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: Nasue is not same", __FUNCTION__);
		goto exit;
	}	
	offset += WAPI_CHALLENGE_LEN;

	//AE WAPI IE
	ae_ie = payload + offset;
	if (*ae_ie != _EID_WAPI_) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid AE WAPI IE ID", __FUNCTION__);
		goto exit;
	}
	if (usk_update == 0 && 
		(memcmp(ae_ie, wapi_asue->ae_wapi_ie, wapi_asue->ae_wapi_ie_len) != 0 || 
		 *(ae_ie + 1) + 2 != wapi_asue->ae_wapi_ie_len)) {
		rtl_wapi_trace(MSG_ERROR, "%s: Invalid AE WAPI IE", __FUNCTION__);
		rtl_dump_buffer(wapi_asue->ae_wapi_ie, wapi_asue->ae_wapi_ie_len, "AE WAPI IE from beacon/probe response packet");
		rtl_dump_buffer(ae_ie, *(ae_ie + 1) + 2, "AE WAPI IE from USK agreement confirm packet");

		ret = WAPI_RETURN_DEASSOC;
		goto exit;
	}
	offset += *(ae_ie + 1) + 2;

	//MIC
	mic = payload + offset;

	u8 *mic_data = payload;
	int mic_data_len = offset;
	u8 local_mic[WAI_MIC_LEN];
	wapi_hmac_sha256(mic_data, mic_data_len, wapi_asue->usksa->usk[uskid].mak, PAIRKEY_LEN, local_mic, WAI_MIC_LEN);
	if (memcmp(mic, local_mic, WAI_MIC_LEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: MIC mismatch", __FUNCTION__);
		goto exit;
	}

	wapi_asue->usksa->uskid = uskid;
	
	GETSHORT(waihdr->seq_num, wapi_asue->rxseq);

	rtl_wapi_trace(MSG_DEBUG, "%s: wapi_asue->rxseq=%d", __FUNCTION__, wapi_asue->rxseq);	

	wapi_asue->wapi_state = WAPI_USK_AGREEMENT_CONFIRM_RCVD;

	rtl_wapi_install_usk(wapi_asue);		

	wapi_asue->wapi_state = WAPI_USKA_ESTABLISH;
	
	ret = WAPI_RETURN_SUCCESS;

exit:

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}

int rtl_wapi_send_usk_agreement_response_pkt(struct wapi_asue_st *wapi_asue)
{
	int ret = WAPI_RETURN_FAILED;
	u8 tbuf[MAX_WAI_PKT_LEN];
	u8 *pos = NULL;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	memset(tbuf, 0, sizeof(tbuf));

	pos = tbuf;
	pos = rtl_build_ethernet_hdr(pos, wapi_asue->asue_mac, wapi_asue->ae_mac);
	pos = rtl_wapi_build_hdr(pos, ++wapi_asue->txseq, WAI_USK_AGREEMENT_RESPONSE);

	u8 *payload = wapi_asue->rxfrag->data + ETHHDR_LEN + WAI_HDR_LEN;
	u8 uskid = payload[WAI_FLAG_LEN + BK_LEN];

	//Flag + BKID + USKID + ADDID	
	pos = (u8*)MEMCPY(pos, payload, WAI_FLAG_LEN + BKID_LEN + 1 + ADDID_LEN);	

	//ASUE challenge
	pos = (u8*)MEMCPY(pos, wapi_asue->Nasue, WAPI_CHALLENGE_LEN);

	//AE challenge
	pos = (u8*)MEMCPY(pos, wapi_asue->Nae, WAPI_CHALLENGE_LEN);

	//ASUE WAPI IE
	pos = (u8*)MEMCPY(pos, wapi_asue->asue_wapi_ie, wapi_asue->asue_wapi_ie_len);

	//MIC
	u8 *mic_data = tbuf + ETHHDR_LEN + WAI_HDR_LEN;
	int mic_data_len = pos - tbuf - ETHHDR_LEN - WAI_HDR_LEN;
	wapi_hmac_sha256(mic_data, mic_data_len, wapi_asue->usksa->usk[uskid].mak, PAIRKEY_LEN, pos, WAI_MIC_LEN);
	pos += WAI_MIC_LEN;

	rtl_wapi_set_length(tbuf, pos-tbuf);
	rtl_wapi_wai_send(tbuf, pos-tbuf);

	rtl_wapi_timer_set(wapi_asue, GENERAL_TIMEOUT, tbuf, pos-tbuf);

	wapi_asue->wapi_state = WAPI_USK_AGREEMENT_RSP_SNT;

	ret = WAPI_RETURN_SUCCESS;

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}

int rtl_wapi_recv_usk_agreement_request_pkt(struct wapi_asue_st *wapi_asue)
{
	int offset = 0;
	u8 flag = 0;
	u8 usk_update = 0, uskid = 0;
	u8 *bkid = NULL, *add_id = NULL;
	u8 *Nae = NULL;
	u8 *text = NULL;
	int text_len = 0;
	
	int ret = WAPI_RETURN_FAILED;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct wai_hdr *waihdr = (struct wai_hdr *)(wapi_asue->rxfrag->data + ETHHDR_LEN);
	u8 *payload = wapi_asue->rxfrag->data + ETHHDR_LEN + WAI_HDR_LEN;
	int len = wapi_asue->rxfrag->data_len - ETHHDR_LEN - WAI_HDR_LEN;

	//Flag
	flag = payload[0];
	if (flag & BIT(4))
		usk_update = 1;
	wapi_asue->ae_flag = flag;
	offset += WAI_FLAG_LEN;

	//BKID
	bkid = payload + offset;
	if (memcmp(bkid, wapi_asue->bksa->bkid, BKID_LEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: bkid is not same", __FUNCTION__);
		goto exit;
	}
	offset += BKID_LEN;

	//USKID
	uskid = *(payload + offset);
	if (usk_update && uskid != !wapi_asue->usksa->uskid) {
		rtl_wapi_trace(MSG_ERROR, "%s: usk_id is not same", __FUNCTION__);
		goto exit;
	}
	offset += 1;

	//ADDID
	add_id = payload + offset;
	if (memcmp(add_id, wapi_asue->ae_mac, ETH_ALEN) != 0 || 
		memcmp(add_id + ETH_ALEN , wapi_asue->asue_mac, ETH_ALEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: add_id is not same", __FUNCTION__);
		goto exit;
	}
	offset += ADDID_LEN;

	//AE challenge
	Nae = payload + offset;
	u8 *ae_challenge = wapi_asue->usksa->usk[wapi_asue->usksa->uskid].asue_challenge;
	if (usk_update && memcmp(Nae, ae_challenge, WAPI_CHALLENGE_LEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: Nae is not same", __FUNCTION__);
		goto exit;
	}
	memcpy(wapi_asue->Nae, Nae, WAPI_CHALLENGE_LEN);
	offset += WAPI_CHALLENGE_LEN;

	//Derivation USK
	text_len = strlen(USK_TEXT) + ADDID_LEN + (WAPI_CHALLENGE_LEN << 1);
	text = (u8 *)rtl_allocate_buffer(text_len);
	if(text == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate text failure", __FUNCTION__);
		goto exit;
	}
	u8 *pos = text;

	get_random(wapi_asue->Nasue, WAPI_CHALLENGE_LEN);
	
	pos= (u8 *)MEMCPY(pos, add_id, ADDID_LEN);
	pos= (u8 *)MEMCPY(pos, wapi_asue->Nae, WAPI_CHALLENGE_LEN);
	pos= (u8 *)MEMCPY(pos, wapi_asue->Nasue, WAPI_CHALLENGE_LEN);
	pos= (u8 *)MEMCPY(pos, USK_TEXT, strlen(USK_TEXT));
	
	rtl_wapi_key_derivation(wapi_asue->bksa->bk, BK_LEN, text, text_len, 
		wapi_asue->usksa->usk[uskid].uek, USKSA_LEN, ISUSK);

	GETSHORT(waihdr->seq_num, wapi_asue->rxseq);

	rtl_wapi_trace(MSG_DEBUG, "%s: wapi_asue->rxseq=%d", __FUNCTION__, wapi_asue->rxseq);	

	wapi_asue->wapi_state = WAPI_USK_AGREEMENT_REQ_RCVD;
	
	ret = WAPI_RETURN_SUCCESS;

exit:

	if (text)
		rtl_free_buffer(text, text_len);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}



int rtl_wapi_recv_access_auth_response_pkt(struct wapi_asue_st *wapi_asue)
{
	int offset = 0;
	u8 flag = 0;
	u8 *Nasue = NULL, *Nae = NULL;
	u8 *asue_tmp_pubkey = NULL, *ae_tmp_pubkey = NULL;
	u8 *ae_id = NULL, *asue_id = NULL;
	u8 *ae_sign = NULL, *asu_sign = NULL;
	tsign_info *s_ae_sign = NULL, *s_asu_sign = NULL;
	tkey *ae_pubkey = NULL, *asu_pubkey = NULL;
	u8 access_result;
	cert_auth_result *presult = NULL;
	u8 *auth_result = NULL;
	
	int ret = WAPI_RETURN_FAILED;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct wai_hdr *waihdr = (struct wai_hdr *)(wapi_asue->rxfrag->data + ETHHDR_LEN);
	u8 *payload = wapi_asue->rxfrag->data + ETHHDR_LEN + WAI_HDR_LEN;
	int len = wapi_asue->rxfrag->data_len - ETHHDR_LEN - WAI_HDR_LEN;

	//Flag
	flag = payload[0];
	if ((flag & 0x03) != (wapi_asue->asue_flag & 0x03)) {
		rtl_wapi_trace(MSG_ERROR, "%s: flag is not same", __FUNCTION__);
		goto exit;
	}
	offset += WAI_FLAG_LEN;

	//ASUE challenge
	Nasue = payload + offset;
	if (memcmp(Nasue, wapi_asue->Nasue, WAPI_CHALLENGE_LEN) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: Nasue is not same", __FUNCTION__);
		goto exit;
	}
	offset += WAPI_CHALLENGE_LEN;

	//AE challenge
	Nae = payload + offset;
	memcpy(wapi_asue->Nae, Nae, WAPI_CHALLENGE_LEN);
	offset += WAPI_CHALLENGE_LEN;

	//Access result
	access_result = *(payload + offset);
	offset += 1;

	//ASUE tmporary public key
	asue_tmp_pubkey = payload + offset;
	if (*asue_tmp_pubkey != wapi_asue->asue_tmp_pubkey->length || 
		memcmp(asue_tmp_pubkey + 1, wapi_asue->asue_tmp_pubkey->data, wapi_asue->asue_tmp_pubkey->length) != 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: asue_tmp_pubkey is not same", __FUNCTION__);
		goto exit;
	}
	offset += 1 + wapi_asue->asue_tmp_pubkey->length;

	//AE tmporary public key
	ae_tmp_pubkey = payload + offset;
	if (wapi_asue->ae_tmp_pubkey == NULL)
		wapi_asue->ae_tmp_pubkey = (wapi_data *)rtl_allocate_buffer(sizeof(wapi_data));
	if (wapi_asue->ae_tmp_pubkey == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate ae_tmp_pubkey fail", __FUNCTION__);
		goto exit;
	}
	wapi_asue->ae_tmp_pubkey->length = *ae_tmp_pubkey;
	memcpy(wapi_asue->ae_tmp_pubkey->data, ae_tmp_pubkey + 1, wapi_asue->ae_tmp_pubkey->length);	
	offset += 1 + wapi_asue->ae_tmp_pubkey->length;

	//AE identity
	ae_id = payload + offset;
	if (rtl_wapi_compare_identity(ae_id, wapi_asue->ae_id)) {
		rtl_wapi_trace(MSG_ERROR, "%s: ae_id is not same", __FUNCTION__);
		goto exit;
	}
	offset += 4 + wapi_asue->ae_id->id_len;

	//ASUE identity
	asue_id = payload + offset;
	if (rtl_wapi_compare_identity(asue_id, wapi_asue->cert_infos->asue_id)) {
		rtl_wapi_trace(MSG_ERROR, "%s: asue_id is not same", __FUNCTION__);
		goto exit;
	}
	offset += 4 + wapi_asue->cert_infos->asue_id->id_len;

	//Certificate verification result
	if (flag & BIT(3)) {
		auth_result = payload + offset;
		presult = rtl_wapi_parse_cert_auth_result(auth_result);
		if (!presult) {
			rtl_wapi_trace(MSG_ERROR, "%s: parse certificate authenticate result fail", __FUNCTION__);
			goto exit;
		}
		if (presult->ae_result != 0) {
			rtl_wapi_trace(MSG_ERROR, "%s: ae_result is invalid", __FUNCTION__);
			ret = WAPI_RETURN_DEASSOC;
			goto exit;
		}
		offset += presult->length + 3;

		//Verify ASU signature
		asu_sign = payload + offset;
		s_asu_sign = rtl_wapi_parse_signature(asu_sign);
		if (!s_asu_sign) {
			rtl_wapi_trace(MSG_ERROR, "%s: parse asu signature fail", __FUNCTION__);
			goto exit;
		}
		asu_pubkey = wapi_asue->cert_infos->asu_pubkey;
		if (!asu_pubkey) {
			rtl_wapi_trace(MSG_ERROR, "%s: get asu public key fail", __FUNCTION__);
			goto exit;
		}
		
		if (!x509_ecc_verify(asu_pubkey->data, asu_pubkey->length, auth_result, presult->length + 3, 
			s_asu_sign->sign_value.data, s_asu_sign->sign_value.length)) {
			rtl_wapi_trace(MSG_ERROR, "%s: verify asu signature fail", __FUNCTION__);
			goto exit;
		}
		offset += s_asu_sign->length + 3;
	}

	//AE signature
	ae_sign = payload + offset;
	s_ae_sign = rtl_wapi_parse_signature(ae_sign);
	if (!s_ae_sign) {
		rtl_wapi_trace(MSG_ERROR, "%s: parse ae signature fail", __FUNCTION__);
		goto exit;
	}

	//get AE public key from AE certificate
	ae_pubkey = rtl_wapi_cert_get_pubkey(wapi_asue->ae_cert);
	if (!ae_pubkey) {
		rtl_wapi_trace(MSG_ERROR, "%s: get ae public key fail", __FUNCTION__);
		goto exit;
	}

	//Verify AE signature
	if (!x509_ecc_verify(ae_pubkey->data, ae_pubkey->length, payload, offset, 
			s_ae_sign->sign_value.data, s_ae_sign->sign_value.length)) {
		rtl_wapi_trace(MSG_ERROR, "%s: verify ae signature fail", __FUNCTION__);
		goto exit;
	}

	if (access_result != ACCESS_SUCCESS) {
		rtl_wapi_trace(MSG_ERROR, "%s: access result is not success", __FUNCTION__);
		ret = WAPI_RETURN_DEASSOC;
		goto exit;
	}	

	GETSHORT(waihdr->seq_num, wapi_asue->rxseq);

	rtl_wapi_trace(MSG_DEBUG, "%s: wapi_asue->rxseq=%d", __FUNCTION__, wapi_asue->rxseq);	

	wapi_asue->wapi_state = WAPI_ACCESS_AUTH_RSP_RCVD;
	
	ret = WAPI_RETURN_SUCCESS;

exit:

	if (presult)
		rtl_free_buffer(presult, sizeof(cert_auth_result));
	if (s_asu_sign)
		rtl_free_buffer(s_asu_sign, sizeof(tsign_info));
	if (s_ae_sign)
		rtl_free_buffer(s_ae_sign, sizeof(tsign_info));	
	if (ae_pubkey)
		rtl_free_buffer(ae_pubkey, sizeof(tkey));	
	
	if (ret != WAPI_RETURN_SUCCESS) {
		if (wapi_asue->ae_tmp_pubkey)
			wapi_asue->ae_tmp_pubkey = rtl_free_buffer(wapi_asue->ae_tmp_pubkey, sizeof(wapi_data));		
	}

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}




int rtl_wapi_send_access_auth_request_pkt(struct wapi_asue_st *wapi_asue)
{
	int ret = WAPI_RETURN_FAILED;
	u8 tbuf[MAX_WAI_PKT_LEN];
	u8 *pos = NULL;

	tkey *asue_prikey = NULL;
	u8 *sign_data = NULL;
	u8 *sign_len_pos = NULL;
	int sign_data_len = 0;
	u8 sign_value[64] = {0};

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	memset(tbuf, 0, sizeof(tbuf));

	pos = tbuf;
	pos = rtl_build_ethernet_hdr(pos, wapi_asue->asue_mac, wapi_asue->ae_mac);
	pos = rtl_wapi_build_hdr(pos, ++wapi_asue->txseq, WAI_ACCESS_AUTH_REQUEST);

	//Flag
	*pos = (wapi_asue->ae_flag | BIT(2));
	*pos &= 0x0f;
	*pos &= (~ BIT(3));
	wapi_asue->asue_flag = *pos;
	pos = pos + WAI_FLAG_LEN;

	//Authentication flag
	pos = (u8*)MEMCPY(pos, wapi_asue->ae_auth_flag, WAPI_AUTHFLAG_LEN);

	//ASUE challenge
	get_random(wapi_asue->Nasue, WAPI_CHALLENGE_LEN);
	pos = (u8*)MEMCPY(pos, wapi_asue->Nasue, WAPI_CHALLENGE_LEN);
	
	//ASUE temporary public key
	*pos = wapi_asue->asue_tmp_pubkey->length;
	pos = pos + 1;
	pos = (u8*)MEMCPY(pos, wapi_asue->asue_tmp_pubkey->data, wapi_asue->asue_tmp_pubkey->length);

	//AE identity
	SETSHORT(pos, wapi_asue->ae_id->id_flag); pos += 2;
	SETSHORT(pos, wapi_asue->ae_id->id_len); pos += 2;
	pos = (u8*)MEMCPY(pos, wapi_asue->ae_id->id_data, wapi_asue->ae_id->id_len);

	//ASUE certificate
	SETSHORT(pos, wapi_asue->cert_infos->asue_cert->cert_flag); pos += 2;
	SETSHORT(pos, wapi_asue->cert_infos->asue_cert->length); pos += 2;
	pos = (u8*)MEMCPY(pos, wapi_asue->cert_infos->asue_cert->data, wapi_asue->cert_infos->asue_cert->length);

	//ECDH parameter
	*pos = wapi_asue->ecdh.para_flag; pos += 1;
	SETSHORT(pos, wapi_asue->ecdh.para_len); pos += 2;
	pos = (u8*)MEMCPY(pos, wapi_asue->ecdh.para_data, wapi_asue->ecdh.para_len);
	
	//ASUE signature
	asue_prikey = wapi_asue->cert_infos->asue_prikey;
	sign_data = tbuf + ETHHDR_LEN + WAI_HDR_LEN;
	sign_data_len = pos - tbuf - ETHHDR_LEN - WAI_HDR_LEN;

	if (!x509_ecc_sign(asue_prikey->data, asue_prikey->length, sign_data, sign_data_len, sign_value)) {
		rtl_wapi_trace(MSG_ERROR, "%s: call x509_ecc_sign() fail", __FUNCTION__);
		goto exit;
	}
	*pos = 1;
	pos = pos + 1;
	
	sign_len_pos = pos;
	pos += 2;/*length*/
	SETSHORT(pos, wapi_asue->cert_infos->asue_id->id_flag); pos += 2;
	SETSHORT(pos, wapi_asue->cert_infos->asue_id->id_len); pos += 2;
	pos= (u8*)MEMCPY(pos, wapi_asue->cert_infos->asue_id->id_data, wapi_asue->cert_infos->asue_id->id_len); 
	
	SETSHORT(pos, wapi_asue->sign_alg.alg_length); pos += 2;
	*pos = wapi_asue->sign_alg.sha256_flag; pos++;
	*pos = wapi_asue->sign_alg.sign_alg; pos++;
	*pos = wapi_asue->sign_alg.sign_para.para_flag; pos++;
	SETSHORT(pos, wapi_asue->sign_alg.sign_para.para_len); pos += 2;
	pos= (u8*)MEMCPY(pos, wapi_asue->sign_alg.sign_para.para_data, wapi_asue->sign_alg.sign_para.para_len);
	SETSHORT(pos, SIGN_LEN); pos += 2;
	pos = (u8*)MEMCPY(pos, sign_value, SIGN_LEN);
	SETSHORT(sign_len_pos, (pos - sign_len_pos - 2));	

	rtl_wapi_set_length(tbuf, pos-tbuf);
	rtl_wapi_wai_send(tbuf, pos-tbuf);

	rtl_wapi_timer_set(wapi_asue, ACCESS_AUTH_TIMEOUT, tbuf, pos-tbuf);

	wapi_asue->wapi_state = WAPI_ACCESS_AUTH_REQ_SNT;

	ret = WAPI_RETURN_SUCCESS;

exit:

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}


int rtl_wapi_recv_auth_active_pkt(struct wapi_asue_st *wapi_asue)
{
	int offset = 0;
	u8 flag = 0;
	u8 *auth_flag = NULL, *ecdh = NULL;
	u8 *asu_id = NULL, *ae_cert = NULL;
	int ret = WAPI_RETURN_FAILED;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	struct wai_hdr *waihdr = (struct wai_hdr *)(wapi_asue->rxfrag->data + ETHHDR_LEN);
	u8 *payload = wapi_asue->rxfrag->data + ETHHDR_LEN + WAI_HDR_LEN;
	int len = wapi_asue->rxfrag->data_len - ETHHDR_LEN - WAI_HDR_LEN;

	//Flag
	flag = payload[0];
	wapi_asue->ae_flag = flag;
	offset += WAI_FLAG_LEN;

	//Authentication flag
	auth_flag = payload + offset;
	if (flag & BIT(0)) { //BK update
		if (memcmp(auth_flag, wapi_asue->ae_auth_flag, WAPI_AUTHFLAG_LEN) != 0) {
			rtl_wapi_trace(MSG_ERROR, "%s: AE authentication flag is not same", __FUNCTION__);
			goto exit;
		}
	} else {
		memcpy(wapi_asue->ae_auth_flag, auth_flag, WAPI_AUTHFLAG_LEN);
	}
	offset += WAPI_AUTHFLAG_LEN;

	//ASU identity
	if (wapi_asue->ae_asu_id == NULL)
		wapi_asue->ae_asu_id = (wai_fixdata_id *)rtl_allocate_buffer(sizeof(wai_fixdata_id));
	if (wapi_asue->ae_asu_id == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate ae_asu_id fail", __FUNCTION__);
		goto exit;
	}
	asu_id = payload + offset;
	GETSHORT(asu_id, wapi_asue->ae_asu_id->id_flag); 
	GETSHORT((asu_id + 2), wapi_asue->ae_asu_id->id_len);
	memcpy(wapi_asue->ae_asu_id->id_data, asu_id + 4, wapi_asue->ae_asu_id->id_len);
	offset += wapi_asue->ae_asu_id->id_len + 4;

	//AE certificate
	if (wapi_asue->ae_cert == NULL)
		wapi_asue->ae_cert = (cert_id *)rtl_allocate_buffer(sizeof(cert_id));
	if (wapi_asue->ae_cert == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate ae_cert fail", __FUNCTION__);
		goto exit;
	}
	ae_cert = payload + offset;
	GETSHORT(ae_cert, wapi_asue->ae_cert->cert_flag); 
	GETSHORT((ae_cert + 2), wapi_asue->ae_cert->length);
	memcpy(wapi_asue->ae_cert->data, ae_cert + 4, wapi_asue->ae_cert->length);
	offset += wapi_asue->ae_cert->length + 4;


	if (wapi_asue->ae_id == NULL)
		wapi_asue->ae_id = (wai_fixdata_id *)rtl_allocate_buffer(sizeof(wai_fixdata_id));
	if (wapi_asue->ae_id == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: allocate ae_id fail", __FUNCTION__);
		goto exit;
	}
	rtl_wapi_fixdata_id(wapi_asue->ae_cert, wapi_asue->ae_id);

	//ECDH parameter
	ecdh = payload + offset;
	wapi_asue->ecdh.para_flag = ecdh[0];
	GETSHORT((ecdh + 1), wapi_asue->ecdh.para_len);
	memcpy(wapi_asue->ecdh.para_data, ecdh + 3, wapi_asue->ecdh.para_len);	
	offset += wapi_asue->ecdh.para_len + 3;

	//Generate ASUE temporary private and public key
	if (rtl_wapi_eck_derivation(wapi_asue) < 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: call rtl_wapi_eck_derivation() fail", __FUNCTION__);
		goto exit;
	}	

	GETSHORT(waihdr->seq_num, wapi_asue->rxseq);

	rtl_wapi_trace(MSG_DEBUG, "%s: wapi_asue->rxseq=%d", __FUNCTION__, wapi_asue->rxseq);

	wapi_asue->wapi_state = WAPI_AUTH_ACTIVE_RCVD;
	
	ret = WAPI_RETURN_SUCCESS;

exit:	
	
	if (ret != WAPI_RETURN_SUCCESS) {
		if (wapi_asue->ae_asu_id)
			wapi_asue->ae_asu_id = rtl_free_buffer(wapi_asue->ae_asu_id, sizeof(wai_fixdata_id));
		if (wapi_asue->ae_cert)
			wapi_asue->ae_cert = rtl_free_buffer(wapi_asue->ae_cert, sizeof(cert_id));
		if (wapi_asue->ae_id)
			wapi_asue->ae_id = rtl_free_buffer(wapi_asue->ae_id, sizeof(wai_fixdata_id));
	}

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);
	
	return ret;
}

int rtl_wapi_install_usk(struct wapi_asue_st *wapi_asue)
{
	u8 key_idx = wapi_asue->usksa->uskid;
	
	u8 *key = wapi_asue->usksa->usk[key_idx].uek;

	if (0 != rtl_wapi_set_usk(wapi_asue->ae_mac, wapi_asue->asue_mac, key_idx, key, (PAIRKEY_LEN << 1)))
	{
		rtl_wapi_trace(MSG_ERROR, "%s: Failed to set USK to the driver", __FUNCTION__);
		return -1;
	}

	return 0;
}

int rtl_wapi_install_msk(struct wapi_asue_st *wapi_asue)
{
	u8 key_idx;

	//Derivation MSK
	u8 derivedKey[PAIRKEY_LEN << 1];
	rtl_wapi_key_derivation(wapi_asue->msksa.nmk, PAIRKEY_LEN, (u8 *)MSK_TEXT, strlen(MSK_TEXT), 
			derivedKey, PAIRKEY_LEN << 1, 0);

	key_idx = wapi_asue->msksa.mskid;
	
	memcpy(wapi_asue->msksa.msk[key_idx].mek, derivedKey, PAIRKEY_LEN);
	memcpy(wapi_asue->msksa.msk[key_idx].mck, derivedKey + PAIRKEY_LEN, PAIRKEY_LEN);	
	
	u8 *key = wapi_asue->msksa.msk[key_idx].mek;
	
	if (0 != rtl_wapi_set_msk(wapi_asue->asue_mac, key_idx, wapi_asue->msksa.key_ann_id, WAI_KEY_AN_ID_LEN, key, PAIRKEY_LEN << 1))
	{
		rtl_wapi_trace(MSG_ERROR, "%s: Failed to set MSK to the driver.", __FUNCTION__);
		return -1;
	}

	return 0;
}
