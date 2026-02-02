#ifndef _WAPI_COMMON_H_
#define _WAPI_COMMON_H_

#include <time.h>
#include "includes.h"
#include "common.h"

#define CERT_FILE_NAME_LEN	64
#define MAX_AP_IFACE	16
#define MAX_STA_NUM		64

#define ETH_P_WAI 0x88b4

#define _EID_WAPI_	68
#define MAX_WAPI_IE_LEN	255
#define MIN_WAPI_IE_LEN	22

#define MIN_KEY_UPDATE_TIME		60
#define MAX_KEY_UPDATE_TIME		31536000 //(60 * 60 * 24 * 365)
#define DEF_KEY_UPDATE_TIME		120 //86400		


typedef enum {
	WAPI_NONE = 0,	/*no WAPI	*/
	WAPI_CERT,		/*Certificate*/
	WAPI_PSK,		/*Pre-PSK*/
}AUTH_TYPE;

typedef enum {
	WAPI_AE = 1,
	WAPI_ASU = 2,
	WAPI_AE_ASU = 4,
}WAPI_ROLE;

typedef enum {
	KEY_TYPE_ASCII = 0,	/*ascii		*/
	KEY_TYPE_HEX,		/*HEX*/
}KEY_TYPE;

typedef enum _wpi_crypto_alg {
	WPI_SM4 = 1,
	WPI_GCM_SM4
} wpi_crypto_alg;

struct wapi_config {
	u8 ifname[IFNAMSIZ];

	AUTH_TYPE auth_type;

	WAPI_ROLE wapi_role;

	wpi_crypto_alg crypto_alg;

	u8 wapi_ie[MAX_WAPI_IE_LEN];
	u8 wapi_ie_len;

	struct in_addr asu_ip;
	
	u32 key_update_time;

	union {
		struct {
			KEY_TYPE kt;				/*Key type*/
			u32 kl;			/*key length*/
			u8 kv[64];		/*value*/
		};
		struct {
			char ca_cert_file[CERT_FILE_NAME_LEN];
			char asu_cert_file[CERT_FILE_NAME_LEN];
			char ae_cert_file[CERT_FILE_NAME_LEN];			
		};
	} para;
};


typedef void (*timeout_handle_func)(void *arg);
 
 struct timeout_entry {
	 int timeout;
	 int counter;
	 struct timespec time;
	 void *arg;
	 timeout_handle_func func;
	 struct timeout_entry *next;
 };

 extern struct timeout_entry *entry_list;

#endif
