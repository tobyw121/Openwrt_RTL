#ifndef __RTW_AP_WAPI_H__
#define __RTW_AP_WAPI_H__

#define ETH_P_WAPI              0x88B4
#define WAPI_VERSION            1
#define WAI_PROTOCOL            1

#define WAPI_KEY_LEN            16
#define WAPI_PN_LEN             16
#define WAPI_GCM_PN_LEN         12
#define WAPI_EXT_HDR_LEN        18
#define WAPI_MIC_LEN            16

#define MMIE_OUI_TYPE           2

#define WAPI_DEBUG
#ifdef WAPI_DEBUG
/* WAPI trace debug */
extern u32 wapi_debug_component;

#define WAPI_TRACE(component, x, args...) \
	do { \
		if (wapi_debug_component & (component)) \
			printk(x, ##args); \
	} while (0);

#else
#define WAPI_TRACE(component, x, args...) do {} while (0)
#endif

enum WAPI_DEBUG_LEVEL {
	WAPI_INIT = BIT(0),
	WAPI_API = BIT(1),
	WAPI_TX = BIT(2),
	WAPI_RX = BIT(3),
	WAPI_MLME = BIT(4),
	WAPI_IOCTL = BIT(5),
	WAPI_ERR = BIT(31)
};

typedef struct _WLAN_HDR_WAPI_EXTENSION {
	u8 keyIdx;
	u8 reserved;
	u8 PN[WAPI_PN_LEN];
} __attribute__((__packed__)) WLAN_HDR_WAPI_EXT;

struct wapi_mmie {
	u8 eid;
	u8 len;
	u8 oui[3];
	u16 sub_type;
	u8 sub_len;
	u16 key_id;
	u8 ipn[WAPI_PN_LEN];
	u8 mic[WAPI_MIC_LEN];
} __attribute__((__packed__));

typedef struct _wapi_key {
	u8 dataKey[WAPI_KEY_LEN];
	u8 micKey[WAPI_KEY_LEN];
} wapiKey;

typedef struct _wapi_ap_info {
	bool wapiMcastEnable;
	u8 txMcastPN[WAPI_PN_LEN];
	u8 rxMcastPN[TID_NUM + 1][WAPI_PN_LEN];

	u8 keyIdx;
	wapiKey wapiMcastKey[2];

#ifdef CONFIG_IEEE80211W
	u8 imk_tx_pn[WAPI_PN_LEN];
	u8 imk_rx_pn[WAPI_PN_LEN];
#endif
} RTL_WAPI_AP_INFO;

typedef struct _wapi_sta_info {
	bool wapiUcastEnable;
	u8 txUcastPN[WAPI_PN_LEN];
	u8 rxUcastPN[TID_NUM + 1][WAPI_PN_LEN];

	u8 keyIdx;
	wapiKey wapiUcastKey[2];
} RTL_WAPI_STA_INFO;

void rtw_wapi_init(_adapter *padapter);
int rtw_validate_wapi_frame(_adapter *padapter, union recv_frame *precv_frame);
void rtw_wapi_get_iv(_adapter *padapter, struct sta_info *psta, u8 key_id, u8 *iv);
int rtw_wapi_set_key(_adapter *padapter, struct ieee_param *param);
int rtw_wapi_check_frame_qos(u8 *whdr, uint len);
#ifdef CONFIG_IEEE80211W
size_t rtw_wapi_build_mmie(_adapter *padapter, u8 *buf, size_t len);
int rtw_wapi_verify_mmie(u8 *mmie, u16 key_id, u8 *rx_pn);

u8 _bip_sms4_protect(u8 *whdr_pos, size_t len, const u8 *key, size_t key_len,
		     const u8 *data, size_t data_len, u8 *mic);
#endif
u32 rtw_sms4_encrypt(_adapter *padapter, u8 *pxmitframe);
u32 rtw_sms4_decrypt(_adapter *padapter, u8 *precvframe);

u32 rtw_gcm_sm4_encrypt(_adapter *padapter, u8 *pxmitframe);
u32 rtw_gcm_sm4_decrypt(_adapter *padapter, u8 *precvframe);

#endif
