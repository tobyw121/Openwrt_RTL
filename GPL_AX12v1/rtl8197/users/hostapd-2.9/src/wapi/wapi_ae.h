#ifndef _WAPI_AE_H_
#define _WAPI_AE_H_

#include "wapi_common.h"


#define MAX_RETRY_NUM	3


#define MAX_CERT_LEN	2048
#define MAX_ID_LEN		1024

#define MAX_WAI_PKT_LEN		4096

#define	WAPI_RETURN_SUCCESS	0
#define	WAPI_RETURN_FAILED	-1
#define	WAPI_RETURN_DEASSOC	-2
#define	WAPI_RETURN_DEASSOCALL	-3


#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#define WAPI_ASU_PORT	3810

#define WAI_LENGTH_OFFSET	6
#define ETHHDR_LEN	14

#define BK_LEN 16

#define PAGE_LEN	4096
#define BKID_LEN	16
#define WAPI_PN_LEN	16
#define WAPI_CHALLENGE_LEN	32
#define WAPI_AUTHFLAG_LEN	32
#define PAIRKEY_LEN	16
#define USKSA_LEN	((PAIRKEY_LEN << 2) + WAPI_CHALLENGE_LEN)
#define ADDID_LEN	(ETH_ALEN << 1)
#define WAI_MIC_LEN	20
#define WAI_IV_LEN	16
#define WAI_KEY_AN_ID_LEN	16
#define ISUSK		1
#define WAI_HDR_LEN	12

#define MSK_TEXT "multicast or station key expansion for station unicast and multicast and broadcast"
#define USK_TEXT "pairwise key expansion for unicast and additional keys and nonce"
//#define BIT(n) (1 << (n))


//enum { MSG_MSGDUMP=1, MSG_DEBUG, MSG_INFO, MSG_WARNING, MSG_ERROR };

typedef enum
{ 
	WAPI_IDLE=0, 
	WAPI_AUTH_ACTIVE_SNT,
	WAPI_AUTH_ACTIVE_RCVD,
	WAPI_ACCESS_AUTH_REQ_SNT,
	WAPI_ACCESS_AUTH_REQ_RCVD,
	WAPI_CERT_AUTH_REQ_SNT,
	WAPI_CERT_AUTH_REQ_RCVD,
	WAPI_CERT_AUTH_RSP_SNT,
	WAPI_CERT_AUTH_RSP_RCVD,
	WAPI_ACCESS_AUTH_RSP_SNT,
	WAPI_ACCESS_AUTH_RSP_RCVD,
	
	WAPI_BKSA_ESTABLISH,
	
	WAPI_USK_AGREEMENT_REQ_SNT,
	WAPI_USK_AGREEMENT_REQ_RCVD,
	WAPI_USK_AGREEMENT_RSP_SNT,
	WAPI_USK_AGREEMENT_RSP_RCVD,
	WAPI_USK_AGREEMENT_CONFIRM_SNT,
	WAPI_USK_AGREEMENT_CONFIRM_RCVD,
	
	WAPI_USKA_ESTABLISH,
	
	WAPI_MSK_ANNOUNCEMENT_SNT,
	WAPI_MSK_ANNOUNCEMENT_RCVD,	
	WAPI_MSK_ANNOUNCEMENT_RSP_SNT,
	WAPI_MSK_ANNOUNCEMENT_RSP_RCVD,
	WAPI_MSKA_ESTABLISH
} wapi_states;

#define WAI_VERSION 1
#define WAI_TYPE 1
#define WAI_FLAG_LEN 1

#define GETSHORT(frm, v) do { (v) = (((frm[0]) <<8) | (frm[1]))& 0xffff;} while (0)
#define GETSHORT1(frm, v) do { (v) = (((frm[1]) <<8) | (frm[0]))& 0xffff;} while (0)
#define SETSHORT(frm, v) do{(frm[0])=((v)>>8)&0xff;(frm[1])=((v))&0xff;}while(0)

enum
{ 
	WAI_PREAUTH_START	= 0x01, /*pre-authentication start*/
	WAI_STAKEY_REQUEST = 0x02,	/*STAKey request */
	WAI_AUTHACTIVE	= 0x03, 		/*authentication activation*/
	WAI_ACCESS_AUTH_REQUEST = 0x04, /*access authentication request */
	WAI_ACCESS_AUTH_RESPONSE = 0x05,	/*access authentication response */
	WAI_CERT_AUTH_REQUEST = 0x06,	/*certificate authentication request */
	WAI_CERT_AUTH_RESPONSE = 0x07,	/*certificate authentication response */
	WAI_USK_AGREEMENT_REQUEST = 0x08, /*unicast key agreement request */
	WAI_USK_AGREEMENT_RESPONSE = 0x09,	/* unicast key agreement response */
	WAI_USK_AGREEMENT_CONFIRMATION = 0x0A,/*unicast key agreement confirmation */
	WAI_MSK_ANNOUNCEMENT = 0x0B, /*multicast key/STAKey announcement */
	WAI_MSK_ANNOUNCEMENT_RESPONSE = 0x0C, /*multicast key/STAKey announcement response */
};

enum 
{ 
	CERT_VALID=0, 
	CERT_ISSUER_UNKNOWN=1,
	CERT_CA_UNTRUSTED=2,
	CERT_INVALID=3,
	CERT_SIGN_ERROR=4,
	CERT_REVOKED=5,
	CERT_USED_ERROR=6,
	CERT_STATE_UNKNOWN=7,
	CERT_ERROR_UNKNOWN=8
};

enum 
{ 
	ACCESS_SUCCESS=0,
	CANNOT_VERIFY_CERT=1,
	CERT_ERROR=2,
	LOCAL_POLICY_FORBID=3
};


struct wai_hdr
{
	u8 version[2];
	u8 type;
	u8 stype;
	u8 reserve[2]; 
	u16 length;
	u8 seq_num[2];
	u8 frag_num;
	u8 more_frag;
}__attribute__((__packed__));


typedef enum 
{
	GENERAL_TIMEOUT = 1,
	CERT_AUTH_TIMEOUT = 10,
	ACCESS_AUTH_TIMEOUT = 31	
} TIMEOUT_VALUE;

struct wapi_bksa_cache
{
	u8 bkid[BKID_LEN];
	u8 bk[BK_LEN];
};

struct wapi_usk
{
	u8 uek[PAIRKEY_LEN]; /* Unicast Encryption Key */
	u8 uck[PAIRKEY_LEN]; /* Unicast Integrity check Key (UCK) */
	u8 mak[PAIRKEY_LEN]; /* Message Authentication Key (MAK)*/
	u8 kek[PAIRKEY_LEN]; /*Key Encryption Key (KEK) */
	u8 ae_challenge[WAPI_CHALLENGE_LEN];
};

struct wapi_usksa
{
	u8 uskid;
	struct wapi_usk usk[2];
};

struct wapi_msk
{
	u8 mek[PAIRKEY_LEN]; /* Mcast Encryption Key */
	u8 mck[PAIRKEY_LEN]; /* Mcast Integrity check Key (MCK) */	
};

struct wapi_msksa
{
	u8 mskid;
	u8 txMcastPN[WAPI_PN_LEN];
	u8 key_ann_id[WAPI_PN_LEN];
	u8 nmk[PAIRKEY_LEN];
	struct wapi_msk msk[2];
};

struct wapi_rxfrag
{
	u8 *data;
	int data_len;
	int maxlen;
};
typedef struct _cert_id
{
	u16 cert_flag;
	u16 length;
	u8 data[MAX_CERT_LEN];
}cert_id;

typedef struct _wapi_data
{
	u8 length;
	u8 pad[3];
	u8 data[256];
}wapi_data;

typedef struct _wai_fixdata_id
{
	u16	id_flag;
	u16	id_len;
	u8 	id_data[MAX_ID_LEN];   
}wai_fixdata_id;


typedef struct _id_list
{
	u8 type;
	u16 length;
	u8 pad;
	u16 id_num;
	wai_fixdata_id *id;
}__attribute__((__packed__)) id_list;



typedef struct _para_alg
{
	u8	para_flag;
	u16	para_len;
	u8	para_data[32];
}para_alg, *ppara_alg;

typedef struct _comm_data
{
	u16 length;
	u16 pad_value;
	u8 data[MAX_CERT_LEN];
}comm_data, *pcomm_data,
tkey, *ptkey;

typedef struct _wai_fixdata_alg
{
	u16	alg_length;
	u8	sha256_flag;
	u8	sign_alg;
	para_alg	sign_para;
}wai_fixdata_alg;

typedef struct _psk_info {
	KEY_TYPE kt;		/*Key type*/
	u32  kl;			/*key length*/
	u8 kv[64];			/*value*/
}psk_info;



typedef struct _sign_alg
{
	u16 alg_length;
	u8 sha256_flag;
	u8 sign_alg;
	para_alg sign_para;
}tsign_alg;

typedef struct _sign_value
{
	u16 length;
	u8 data[64];
}tsign_value;

//signature
typedef struct _wai_sign
{
	u8	type;
	u16	length;
	wai_fixdata_id id;
	tsign_alg sign_alg;
	tsign_value	sign_value;
}tsign_info;


typedef struct _wai_socket
{
	int sockfd;
	struct sockaddr_in servaddr;
}wai_socket;

typedef struct _cert_auth_result
{
	u8 type;
	u16	length;
	u8 Nae[WAPI_CHALLENGE_LEN];
	u8 Nasue[WAPI_CHALLENGE_LEN];
	u8 asue_result;
	cert_id	asue_cert;
	u8 ae_result;
	cert_id	ae_cert;
}__attribute__((__packed__)) cert_auth_result;

typedef struct _cert_info {
	int cert_type;
	
	char ca_cert_file[CERT_FILE_NAME_LEN];
	cert_id *ca_cert;
	tkey *ca_pubkey;

	char asu_cert_file[CERT_FILE_NAME_LEN];
	cert_id *asu_cert;
	tkey *asu_pubkey;
	tkey *asu_prikey;
	wai_fixdata_id *asu_id;

	char ae_cert_file[CERT_FILE_NAME_LEN];
	cert_id *ae_cert;
	tkey *ae_pubkey;
	tkey *ae_prikey;
	wai_fixdata_id *ae_id;
}cert_info; 


 struct wapi_asue_st {
 	u8 asue_mac[ETH_ALEN];
	u8 asue_mac_pad[2];
	u8 ae_mac[ETH_ALEN];
	u8 ae_mac_pad[2];

	u8 ae_flag;
	u8 asue_flag;
	u8 flag_pad[2];
	
	u8 Nasue[WAPI_CHALLENGE_LEN];
	u8 Nae[WAPI_CHALLENGE_LEN];
	u8 NaeNext[WAPI_CHALLENGE_LEN];
	u8 ae_auth_flag[WAPI_AUTHFLAG_LEN];

	//u8 asue_wapi_ie[255];
	//u8 asue_wapi_ie_len;
	
	struct wapi_rxfrag *rxfrag;
	u16 rxseq;
	u16 txseq;
	wapi_states wapi_state;
	wai_fixdata_id *asue_id;

	struct wapi_usksa *usksa; 
	struct wapi_bksa_cache *bksa; /* PMKSA cache */

	cert_id *asue_cert;

	wapi_data *asue_tmp_pubkey;
	wapi_data *ae_tmp_pubkey;
	wapi_data *ae_tmp_prikey;

	struct timeout_entry entry;
	u8 *wai_data;
	u32 wai_data_len;	

	id_list *asu_id_list;	

	u8 psk_bk[BK_LEN];

	u8 *wapi_ie;
	u8 wapi_ie_len;

	u8 access_result;	
	u8 usk_update;
	u8 msk_update_done;
};

struct wapi_ae_st {
	u8 ifname[IFNAMSIZ];
	u8 ae_mac[ETH_ALEN];
	u8 ae_mac_pad[2];
	
	AUTH_TYPE auth_type; /*0-open,1-cert,2-psk*/

	wpi_crypto_alg crypto_alg;

	WAPI_ROLE wapi_role;

	cert_info *cert_infos;

	psk_info *psk_infos;	

	para_alg ecdh;
	tsign_alg sign_alg;

	struct wapi_msksa msksa;

	wai_socket *sock;

	u8 wapi_ie[MAX_WAPI_IE_LEN];
	u8 wapi_ie_len;

	u32 key_update_time;
	struct timeout_entry key_update_entry;
	
	u8 msk_update;
	u8 msk_update_alldone;
};


void rtl_wapi_wai_receive(u8* buf, size_t len);
int rtl_wapi_wai_send(u8 *buf, size_t len);

int rtl_wapi_fixdata_id(cert_id *cert_st, wai_fixdata_id *fixdata_id);
struct wapi_rxfrag *rtl_wapi_defrag(struct wapi_asue_st *wapi_asue, struct wapi_rxfrag *rxbuf);
void rtl_wapi_trace(int level, char *fmt, ...);
int rtl_wapi_bk_derivation(u8 *psk, u32 psk_len, struct wapi_asue_st *wapi_asue);
void rtl_wapi_rx_wai(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);

int rtl_wapi_send_auth_active_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_recv_access_auth_request_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_send_cert_auth_request_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_recv_cert_auth_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_send_access_auth_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_send_usk_agreement_request_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_recv_usk_agreement_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_send_usk_agreement_confirm_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_send_msk_announcement_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_recv_msk_announcement_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_local_auth_asue_cert(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int	rtl_wapi_update_usk(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int	rtl_wapi_update_msk(struct wapi_ae_st *wapi_ae);
int rtl_wapi_recv_usk_update_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_recv_msk_update_response_pkt(struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_send_msg_to_asu(u8 *send_buf, int send_len, u8 *recv_buf, int *recv_len, struct wapi_ae_st *wapi_ae, struct wapi_asue_st *wapi_asue);
int rtl_wapi_recv_msg_from_asu(u8 *recv_buf, int *recv_len, int sockfd);
int rtl_wapi_install_usk(struct wapi_asue_st *wapi_asue);
int rtl_wapi_install_msk(struct wapi_ae_st *wapi_ae);

void rtl_run_timeout_entry(void);
void rtl_wapi_timer_reset(struct wapi_asue_st *wapi_asue);
void rtl_wapi_timer_set(struct wapi_asue_st *wapi_asue, int timeout, u8 *wai_data, int len);
void rtl_wapi_timeout_handler(void *arg);
void rtl_add_timeout_entry(timeout_handle_func func, void *arg, int time, struct timeout_entry *entry);
void rtl_del_timeout_entry(struct timeout_entry *entry);

void *rtl_allocate_buffer(int len);
void *rtl_free_buffer(void *buffer, int len);
void *rtl_wapi_free_rxfrag(struct wapi_rxfrag *frag);


int rtl_get_ae_index(u8 *ae_mac);
int rtl_get_asue_index(u8 *sta_mac);
struct wapi_ae_st *rtl_get_ae_by_mac(u8 *ae_mac);
struct wapi_asue_st *rtl_get_asue_by_mac(u8 *sta_mac);


int rtl_wapi_ae_free(struct wapi_ae_st **pwapi_ae);
int rtl_wapi_asue_free(struct wapi_asue_st **pwapi_asue);

int rtl_wapi_ae_init(u8 *ae_mac, struct wapi_config *config);
int rtl_wapi_asue_init(u8 *ae_mac, u8 *sta_mac, u8 *wapi_ie, u8 ie_len, struct wapi_asue_st *wapi_asue);

void rtl_wapi_process_assoc_event(u8 *ae_mac, u8 *sta_mac, u8 *wapi_ie, u8 ie_len);
void rtl_wapi_process_disassoc_event(u8 *ae_mac, u8 *sta_mac);
void rtl_wapi_process_usk_update_event(u8 *sta_mac);
void rtl_wapi_process_msk_update_event(u8 *ae_mac);


void rtl_wapi_ecdh_init(para_alg *ecdh);
void rtl_wapi_sign_alg_init(tsign_alg *sign_alg);
void rtl_wapi_init_tx_mcast_pn(struct wapi_msksa *msksa);

#endif
