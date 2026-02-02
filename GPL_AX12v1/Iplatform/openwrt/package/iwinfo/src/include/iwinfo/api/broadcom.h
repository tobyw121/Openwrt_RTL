/*
 * Custom OID/ioctl definitions for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Definitions subject to change without notice.
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifndef _BROADCOM_H
#define _BROADCOM_H

/*
 * wangxiaolong@tp-link.com.cn
 * 2020-11-27
 * Content:
 *	Length of shell cmds
 */
#define CMD_ARG_LEN					128

#define WL_MCSSET_LEN				16
#define WL_MAX_STA_COUNT			256


#define WL_BSS_RSSI_OFFSET			82
#define WL_BSS_NOISE_OFFSET			84

#define WLC_IOCTL_MAGIC				0x14e46c77
#define	WLC_IOCTL_MAXLEN			8192

#define WLC_CNTRY_BUF_SZ        	4

#define WLC_GET_MAGIC				0
#define WLC_GET_RATE				12
#define WLC_GET_INFRA				19
#define WLC_GET_AUTH				21
#define WLC_GET_BSSID				23
#define WLC_GET_SSID				25
#define WLC_GET_CHANNEL				29
#define WLC_GET_PASSIVE 			48
#define WLC_GET_COUNTRY				83
#define WLC_GET_REVINFO				98
#define WLC_GET_AP					117
#define WLC_GET_RSSI				127
#define WLC_GET_WSEC				133
#define WLC_GET_PHY_NOISE			135
#define WLC_GET_BSS_INFO			136
#define WLC_GET_ASSOCLIST			159
#define WLC_GET_WPA_AUTH			164
#define WLC_GET_COUNTRY_LIST		261
#define WLC_GET_VAR					262

#define WLC_GET_RADIO				37
#define WLC_GET_CURR_RATESET			114	/* current rateset */
#define WLC_GET_BAND				141
#define WLC_GET_VALID_CHANNELS		217
#define WLC_GET_PHYTYPE				39
#define	WLC_PHY_TYPE_SSN	6
#define	WLC_PHY_TYPE_LCN	8
#define	WLC_PHY_TYPE_LCN40	10
#define WLC_GET_BCNPRD				75

#define	WLC_PHY_TYPE_A		0
#define DOT11_MAX_DEFAULT_KEYS	4	/* number of default keys */

/* Key related defines */
#define DOT11_MAX_DEFAULT_KEYS	4	/* number of default keys */
#define DOT11_MAX_KEY_SIZE	32	/* max size of any key */
#define DOT11_MAX_IV_SIZE	16	/* max size of any IV */
#define DOT11_EXT_IV_FLAG	(1<<5)	/* flag to indicate IV is > 4 bytes */
#define DOT11_WPA_KEY_RSC_LEN   8       /* WPA RSC key len */

#define WEP1_KEY_SIZE		5	/* max size of any WEP key */
#define WEP1_KEY_HEX_SIZE	10	/* size of WEP key in hex. */
#define WEP128_KEY_SIZE		13	/* max size of any WEP key */
#define WEP128_KEY_HEX_SIZE	26	/* size of WEP key in hex. */
#define TKIP_MIC_SIZE		8	/* size of TKIP MIC */
#define TKIP_EOM_SIZE		7	/* max size of TKIP EOM */
#define TKIP_EOM_FLAG		0x5a	/* TKIP EOM flag byte */
#define TKIP_KEY_SIZE		32	/* size of any TKIP key */
#define TKIP_MIC_AUTH_TX	16	/* offset to Authenticator MIC TX key */
#define TKIP_MIC_AUTH_RX	24	/* offset to Authenticator MIC RX key */
#define TKIP_MIC_SUP_RX		TKIP_MIC_AUTH_TX	/* offset to Supplicant MIC RX key */
#define TKIP_MIC_SUP_TX		TKIP_MIC_AUTH_RX	/* offset to Supplicant MIC TX key */
#define AES_KEY_SIZE		16	/* size of AES key */
#define AES_MIC_SIZE		8	/* size of AES MIC */
#define BIP_KEY_SIZE		16	/* size of BIP key */
#define BIP_MIC_SIZE		8   /* sizeof BIP MIC */

/* Override bit for WLC_SET_TXPWR.  if set, ignore other level limits */
#define WL_TXPWR_OVERRIDE	(1U<<31)
#define WL_TXPWR_NEG   (1U<<30)

/* Bit masks for radio disabled status - returned by WL_GET_RADIO */
#define WL_RADIO_SW_DISABLE		(1<<0)
#define WL_RADIO_HW_DISABLE		(1<<1)
#define WL_RADIO_MPC_DISABLE		(1<<2)
#define WL_RADIO_COUNTRY_DISABLE	(1<<3)	/* some countries don't support any channel */

/*
 * One doesn't need to include this file explicitly, gets included automatically if
 * typedefs.h is included.
 */

/* Use BCM_REFERENCE to suppress warnings about intentionally-unused function
 * arguments or local variables.
 */
#define BCM_REFERENCE(data)	((void)(data))

struct wl_ether_addr {
	uint8_t					octet[6];
};

struct wl_maclist {
	uint					count;
	struct wl_ether_addr 	ea[1];
};

typedef struct wl_sta_rssi {
	int						rssi;
	char					mac[6];
	uint16_t				foo;
} wl_sta_rssi_t;

#define WL_NUMRATES     16 /* max # of rates in a rateset */
typedef struct wl_rateset {
    uint32_t  				count;          /* # rates in this set */
    uint8_t   				rates[WL_NUMRATES]; /* rates in 500kbps units w/hi bit set if basic */
} wl_rateset_t;

#if (BCM_WL_STA_INFO_VER >= 5)
#define MCSSET_LEN		16		/* 16-bits per 8-bit set to give 128-bits bitmap of MCS Index */
#define VHT_CAP_MCS_MAP_NSS_MAX         8
typedef struct wl_rateset_args {
	uint32_t	count;								/**< # rates in this set */
	uint8_t		rates[WL_NUMRATES];					/**< rates in 500kbps units w/hi bit set if basic */
	uint8_t   	mcs[MCSSET_LEN];        			/* supported mcs index bit map */
	uint16_t 	vht_mcs[VHT_CAP_MCS_MAP_NSS_MAX]; 	/* supported mcs index bit map per nss */
	} wl_rateset_args_t;
#endif

#if (BCM_WL_STA_INFO_VER >= 7)
#define WL_MAXRATES_IN_SET		16	/**< max # of rates in a rateset */
#define WL_VHT_CAP_MCS_MAP_NSS_MAX	8
#define WL_HE_CAP_MCS_MAP_NSS_MAX	8

#define WLC_MAX_ASSOC_OUI_NUM   6
#define DOT11_OUI_LEN			3	/* d11 OUI length */

#if (BCM_WL_STA_INFO_VER == 8)
#define DOT11_RRM_CAP_LEN		5
#endif

#if (BCM_WL_STA_INFO_SUBVER == 1)  /* set for 6756 ax75v1(wl_driver=v17.10.180) */
#define WL_MAX_BAND		        8

#elif (BCM_WL_STA_INFO_SUBVER == 2)  /* add by songdandan for be900v1 */
#define WL_MAX_BAND		        8
#define WL_RXTX_SZ			2
#define WL_EHT_BW_SZ			3
#define EHT_MCS_MAP_NSS_MAX     	8u
#define WL_MAX_RATE_LOG		16
#define EHT_TID_MAP_DIR_BOTH    	2u
#define WL_STA_MAX_NUM_TID		16

/* Tracking Queue Size and TID of QoS control field */
typedef struct sta_queue_size {
	uint32_t	current_time;
	/* The order of the arrays matches the TID. */
	uint8_t		qsize[WL_STA_MAX_NUM_TID];
	uint32_t	update_time[WL_STA_MAX_NUM_TID];
} sta_queue_size_t;
#endif

typedef struct wl_rateset_args_v2 {
	uint16_t 	version;								/**< version. */
	uint16_t 	len;									/**< length */
	uint32_t	count;								/**< # rates in this set */
	uint8_t		rates[WL_MAXRATES_IN_SET];			/**< rates in 500kbps units w/hi bit set if basic */
	uint8_t   	mcs[MCSSET_LEN];					/**< supported mcs index bit map */
	uint16_t 	vht_mcs[WL_VHT_CAP_MCS_MAP_NSS_MAX]; /**< supported mcs index bit map per nss */
	uint16_t 	he_mcs[WL_HE_CAP_MCS_MAP_NSS_MAX]; 	/**< supported he mcs index bit map per nss */
} wl_rateset_args_v2_t;

typedef struct {
	uint8_t count;
	uint8_t oui[WLC_MAX_ASSOC_OUI_NUM][DOT11_OUI_LEN];
} sta_vendor_oui_t;

#endif 

#define WL_STA_ANT_MAX		4		/* max possible rx antennas */

typedef struct wl_sta_info {
    uint16_t				ver;        /* version of this struct */
    uint16_t				len;        /* length in bytes of this structure */
    uint16_t				cap;        /* sta's advertised capabilities */
#if (BCM_WL_STA_INFO_VER >= 7)
	uint16_t 				PAD0; 
#endif
    uint32_t				flags;      /* flags defined below */
    uint32_t				idle;       /* time since data pkt rx'd from sta */
    unsigned char			ea[6];      /* Station address */
#if (BCM_WL_STA_INFO_VER >=7)
	uint16_t                  PAD1;
#endif
    wl_rateset_t			rateset;    /* rateset in use */
    uint32_t				in;          /* seconds elapsed since associated */
    uint32_t				listen_interval_inms; /* Min Listen interval in ms for this STA */
    uint32_t				tx_pkts;    /* # of packets transmitted */
    uint32_t				tx_failures;    /* # of packets failed */
    uint32_t				rx_ucast_pkts;  /* # of unicast packets received */
    uint32_t				rx_mcast_pkts;  /* # of multicast packets received */
    uint32_t				tx_rate;    /* Rate of last successful tx frame */
    uint32_t				rx_rate;    /* Rate of last successful rx frame */
	uint32_t				rx_decrypt_succeeds;	/* # of packet decrypted successfully */
	uint32_t				rx_decrypt_failures;	/* # of packet decrypted unsuccessfully */
	uint32_t				tx_tot_pkts;	/* # of tx pkts (ucast + mcast) */
	uint32_t				rx_tot_pkts;	/* # of data packets recvd (uni + mcast) */
	uint32_t				tx_mcast_pkts;	/* # of mcast pkts txed */
	uint64_t				tx_tot_bytes;	/* data bytes txed (ucast + mcast) */
	uint64_t				rx_tot_bytes;	/* data bytes recvd (ucast + mcast) */
	uint64_t				tx_ucast_bytes;	/* data bytes txed (ucast) */
	uint64_t				tx_mcast_bytes;	/* # data bytes txed (mcast) */
	uint64_t				rx_ucast_bytes;	/* data bytes recvd (ucast) */
	uint64_t				rx_mcast_bytes;	/* data bytes recvd (mcast) */
	int8_t					rssi[WL_STA_ANT_MAX];	/* average rssi per antenna of data frames */
	int8_t					nf[WL_STA_ANT_MAX];		/* per antenna noise floor */
	uint16_t				aid;					/* association ID */
	uint16_t				ht_capabilities;		/* advertised ht caps */
	uint16_t				vht_flags;				/* converted vht flags */
#if (BCM_WL_STA_INFO_VER >=7)
	uint16_t					PAD2;
#endif
	uint32_t				tx_pkts_retried;		/* # of frames where a retry was necessary */
	uint32_t				tx_pkts_retry_exhausted; /* # of frames where a retry was exhausted */
	int8_t					rx_lastpkt_rssi[WL_STA_ANT_MAX]; /* Per antenna RSSI of last received data frame */
#ifdef USE_WL_SDK7X
	/* TX WLAN retry/failure statistics:
	 * Separated for host requested frames and WLAN locally generated frames.
	 * Include unicast frame only where the retries/failures can be counted.
	 */
	uint32_t			    tx_pkts_total;		/**< # user frames sent successfully */
	uint32_t			    tx_pkts_retries;	/**< # user frames retries */
	uint32_t			    tx_pkts_fw_total;	/**< # FW generated sent successfully */
	uint32_t			    tx_pkts_fw_retries; /**< # retries for FW generated frames */
	uint32_t			    tx_pkts_fw_retry_exhausted; /**< # FW generated where a retry
								 * was exhausted
								 */
	uint32_t			    rx_pkts_retried;	/**< # rx with retry bit set */
	uint32_t			    tx_rate_fallback;	/**< lowest fallback TX rate */

#if (BCM_WL_STA_INFO_VER == 5)
	wl_rateset_args_t		rateset_adv;		/* rateset along with mcs index bitmap */
#endif
#endif

#if (BCM_WL_STA_INFO_VER >=7)
	uint32_t				 rx_dur_total;   /* total user RX duration (estimated) */

	uint16_t				chanspec;		/** chanspec this sta is on */
	uint16_t				PAD3;
	wl_rateset_args_v2_t	rateset_adv;	/* rateset along with mcs index bitmap */
#if (BCM_WL_STA_INFO_VER == 8)
	uint16_t				PAD6; 				/* authentication type */
#else
	uint16_t				wpauth; 
#endif
	uint8_t					algo;					/* crypto algorithm */
	uint8_t					PAD4;
	uint32_t				tx_rspec;		/* Rate of last successful tx frame */
	uint32_t				rx_rspec;		/* Rate of last successful rx frame */
	uint32_t				wnm_cap;			  /* wnm capabilities */
	uint16_t				he_flags;	/* converted he flags */
	uint16_t				PAD5;
	sta_vendor_oui_t		sta_vendor_oui;
#endif
/*add by zhangshengbo for sta_info ver 8, from wlioctl.h*/
#if (BCM_WL_STA_INFO_VER == 8)

	uint8_t			link_bw;
	uint32_t			wpauth;		/* authentication type */
	int8_t			srssi;		/* smoothed rssi info */

#if (BCM_WL_STA_INFO_SUBVER == 1)  /* set for 6756 ax75v1(wl_driver=v17.10.180) */
	uint8_t			 twt_info;
	uint16_t          he_omi;
#elif (BCM_WL_STA_INFO_SUBVER == 2) /* set for be900v1 */
	uint8_t			 twt_info;
	uint16_t		omi;
#endif

	/* #ifdef WL_EAP_STATS */
	uint32_t			tx_mgmt_pkts;	/**< # of management packets txed by driver */
	uint32_t			tx_ctl_pkts;	/**< # of control packets txed by driver */
	uint32_t			rx_mgmt_pkts;	/**< # of management packets rxed by driver */
	uint32_t			rx_ctl_pkts;	/**< # of control packets rxed by driver */
	/* #endif WL_EAP_STATS */

	uint8_t		rrm_capabilities[DOT11_RRM_CAP_LEN];	/* rrm capabilities of station */

#if (BCM_WL_STA_INFO_SUBVER == 1)  /* set for 6756 ax75v1(wl_driver=v17.10.180) */
	uint8_t			PAD7;
	uint16_t		map_flags;	
	uint8_t			bands[WL_MAX_BAND];
#elif(BCM_WL_STA_INFO_SUBVER == 2)
	uint8_t			PAD7;
	uint16_t		map_flags;
	uint8_t			bands[WL_MAX_BAND];
	/* add by songdandan for be900v1 */
	uint32_t 		eht_flags;	/**< converted eht flags */
	uint16_t		adv_eht_mcs[WL_RXTX_SZ][WL_EHT_BW_SZ][EHT_MCS_MAP_NSS_MAX];
	uint32_t		tx_rate_log[WL_MAX_RATE_LOG];	/**< rate used by last N tx frame */
	uint32_t		rx_rate_log[WL_MAX_RATE_LOG];	/**< rate used by last N rx frame */
	/* Default tid-to-link mapping directions for link */
	uint8_t			tid_map_dir_bmp;
	/* Default tid-to-link mapping for link */
	uint8_t			tid_map[EHT_TID_MAP_DIR_BOTH + 1];
	uint8_t			client_mode; /* applicable only if MLO CAPABLE */
	unsigned char		peer_mld[6]; /* Peer MLD Address */
	sta_queue_size_t	queue_size; /* Queue Size and TID of QoS control field */
#endif
#endif
} wl_sta_info_t;

typedef struct wlc_ssid {
	uint32_t				ssid_len;
	unsigned char			ssid[32];
} wlc_ssid_t;

/* Linux network driver ioctl encoding */
typedef struct wl_ioctl {
	uint32_t				cmd;	/* common ioctl definition */
	void					*buf;	/* pointer to user buffer */
	uint32_t				len;	/* length of user buffer */
	uint8_t					set;	/* get or set request (optional) */
	uint32_t				used;	/* bytes read or written (optional) */
	uint32_t				needed;	/* bytes needed (optional) */
} wl_ioctl_t;

/* Revision info */
typedef struct wlc_rev_info {
	uint		vendorid;	/* PCI vendor id */
	uint		deviceid;	/* device id of chip */
	uint		radiorev;	/* radio revision */
	uint		chiprev;	/* chip revision */
	uint		corerev;	/* core revision */
	uint		boardid;	/* board identifier (usu. PCI sub-device id) */
	uint		boardvendor;	/* board vendor (usu. PCI sub-vendor id) */
	uint		boardrev;	/* board revision */
	uint		driverrev;	/* driver version */
	uint		ucoderev;	/* microcode version */
	uint		bus;		/* bus type */
	uint		chipnum;	/* chip number */
} wlc_rev_info_t;

typedef struct wl_country_list {
	uint32_t buflen;
	uint32_t band_set;
	uint32_t band;
	uint32_t count;
	char country_abbrev[1];
} wl_country_list_t;


#endif
