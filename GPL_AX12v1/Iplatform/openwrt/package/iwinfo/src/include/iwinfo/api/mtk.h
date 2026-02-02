#ifndef __MTK_H__
#define __MTK_H__

#define MAC_ADDR_LENGTH		6
#define MAX_NUMBER_OF_MAC	64

#define MAX_STA_NUM	256

#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

typedef unsigned char 	UCHAR;
typedef char		CHAR;
typedef unsigned int	UINT32;
typedef unsigned short	USHORT;
typedef short		SHORT;
typedef unsigned long	ULONG;
typedef long long INT64;

#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE				0x8BE0
#endif
#define SIOCIWFIRSTPRIV				SIOCDEVPRIVATE
#endif

#define WLAN_MAX_BSS_NUM            64
#define MAX_NUM_OF_CHANNELS         60
#define MAX_NUM_OF_SURVEY_CNT		64
/* Site Survey List */
#define MAX_LEN_OF_SSID             33
#define MAX_LEN_OF_MAC_STR          18
#define MAX_LEN_OF_SECURITY         32
#define MAX_LEN_OF_WIRELESS_MODE    9

#if defined(PACKAGE_WIFI_DBDC_CHIP_MTK7915) || defined(PACKAGE_WIFI_DBDC_CHIP_MTK7986) || defined(PACKAGE_WIFI_DBDC_CHIP_MTK7981)
#define RTPRIV_IOCTL_SET			(SIOCIWFIRSTPRIV + 0x0B)
#else
#define RTPRIV_IOCTL_SET			(SIOCIWFIRSTPRIV + 0x02)
#endif

#define RTPRIV_IOCTL_GSITESURVEY		(SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_GET_MAC_TABLE		(SIOCIWFIRSTPRIV + 0x0F)

#define RTPRIV_IOCTL_GCHANLIST                      (SIOCIWFIRSTPRIV + 0x10)
#define RTPRIV_IOCTL_GSCANINFO                      (SIOCIWFIRSTPRIV + 0x14)

#define RTPRIV_IOCTL_GBSSINFO	                    (SIOCIWFIRSTPRIV + 0x1C)

#if defined(PACKAGE_WIFI_DBDC_CHIP_MTK7915) || defined(PACKAGE_WIFI_DBDC_CHIP_MTK7986) || defined(PACKAGE_WIFI_DBDC_CHIP_MTK7981)
#define RTPRIV_IOCTL_GSTAINFO						(SIOCIWFIRSTPRIV + 0x1E)
#else
#define RTPRIV_IOCTL_GSTAINFO						(SIOCIWFIRSTPRIV + 0x1F)
#endif

#define MODE_CCK	0
#define MODE_OFDM	1
#define MODE_HTMIX	2

#define CH_MAX_2G_CHANNEL		14	/* Max channel in 2G band */


typedef enum _SEC_AKM_MODE {
	SEC_AKM_OPEN,
	SEC_AKM_SHARED,
	SEC_AKM_AUTOSWITCH,
	SEC_AKM_WPA1, /* Enterprise security over 802.1x */
	SEC_AKM_WPA1PSK,
	SEC_AKM_WPANone, /* For Win IBSS, directly PTK, no handshark */
	SEC_AKM_WPA2, /* Enterprise security over 802.1x */
	SEC_AKM_WPA2PSK,
	SEC_AKM_FT_WPA2,
	SEC_AKM_FT_WPA2PSK,
	SEC_AKM_WPA2_SHA256,
	SEC_AKM_WPA2PSK_SHA256,
	SEC_AKM_TDLS,
	SEC_AKM_SAE_SHA256,
	SEC_AKM_FT_SAE_SHA256,
	SEC_AKM_SUITEB_SHA256,
	SEC_AKM_SUITEB_SHA384,
	SEC_AKM_FT_WPA2_SHA384,
	SEC_AKM_WAICERT, /* WAI certificate authentication */
	SEC_AKM_WAIPSK, /* WAI pre-shared key */
	SEC_AKM_OWE,
	SEC_AKM_WPA3, /* WPA3(ent) = WPA2(ent) + PMF MFPR=1 => WPA3 code flow is same as WPA2, the usage of SEC_AKM_WPA3 is to force pmf on */
	SEC_AKM_MAX /* Not a real mode, defined as upper bound */
} SEC_AKM_MODE, *PSEC_AKM_MODE;



#define IS_AKM_OPEN(_AKMMap)                           ((_AKMMap & (1 << SEC_AKM_OPEN)) > 0)
#define IS_AKM_SHARED(_AKMMap)                       ((_AKMMap & (1 << SEC_AKM_SHARED)) > 0)
#define IS_AKM_AUTOSWITCH(_AKMMap)              ((_AKMMap & (1 << SEC_AKM_AUTOSWITCH)) > 0)
#define IS_AKM_WPA1(_AKMMap)                           ((_AKMMap & (1 << SEC_AKM_WPA1)) > 0)
#define IS_AKM_WPA1PSK(_AKMMap)                    ((_AKMMap & (1 << SEC_AKM_WPA1PSK)) > 0)
#define IS_AKM_WPANONE(_AKMMap)                  ((_AKMMap & (1 << SEC_AKM_WPANone)) > 0)
#define IS_AKM_WPA2(_AKMMap)                          ((_AKMMap & (1 << SEC_AKM_WPA2)) > 0)
#define IS_AKM_WPA2PSK(_AKMMap)                    ((_AKMMap & (1 << SEC_AKM_WPA2PSK)) > 0)
#define IS_AKM_FT_WPA2(_AKMMap)                     ((_AKMMap & (1 << SEC_AKM_FT_WPA2)) > 0)
#define IS_AKM_FT_WPA2PSK(_AKMMap)              ((_AKMMap & (1 << SEC_AKM_FT_WPA2PSK)) > 0)
#define IS_AKM_WPA2_SHA256(_AKMMap)            ((_AKMMap & (1 << SEC_AKM_WPA2_SHA256)) > 0)
#define IS_AKM_WPA2PSK_SHA256(_AKMMap)      ((_AKMMap & (1 << SEC_AKM_WPA2PSK_SHA256)) > 0)
#define IS_AKM_TDLS(_AKMMap)                             ((_AKMMap & (1 << SEC_AKM_TDLS)) > 0)
#define IS_AKM_SAE_SHA256(_AKMMap)                ((_AKMMap & (1 << SEC_AKM_SAE_SHA256)) > 0)
#define IS_AKM_FT_SAE_SHA256(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_FT_SAE_SHA256)) > 0)
#define IS_AKM_SUITEB_SHA256(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_SUITEB_SHA256)) > 0)
#define IS_AKM_SUITEB_SHA384(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_SUITEB_SHA384)) > 0)
#define IS_AKM_FT_WPA2_SHA384(_AKMMap)      ((_AKMMap & (1 << SEC_AKM_FT_WPA2_SHA384)) > 0)
#define IS_AKM_OWE(_AKMMap)      ((_AKMMap & (1 << SEC_AKM_OWE)) > 0)
#define IS_AKM_WPA3(_AKMMap)	 ((_AKMMap & (1 << SEC_AKM_WPA3)) > 0)

typedef enum _SEC_CIPHER_MODE {
	SEC_CIPHER_NONE,
	SEC_CIPHER_WEP40,
	SEC_CIPHER_WEP104,
	SEC_CIPHER_WEP128,
	SEC_CIPHER_TKIP,
	SEC_CIPHER_CCMP128,
	SEC_CIPHER_CCMP256,
	SEC_CIPHER_GCMP128,
	SEC_CIPHER_GCMP256,
	SEC_CIPHER_BIP_CMAC128,
	SEC_CIPHER_BIP_CMAC256,
	SEC_CIPHER_BIP_GMAC128,
	SEC_CIPHER_BIP_GMAC256,
	SEC_CIPHER_WPI_SMS4, /* WPI SMS4 support */
	SEC_CIPHER_MAX /* Not a real mode, defined as upper bound */
} SEC_CIPHER_MODE;


#define IS_CIPHER_NONE(_Cipher)          (((_Cipher) & (1 << SEC_CIPHER_NONE)) > 0)
#define IS_CIPHER_WEP40(_Cipher)          (((_Cipher) & (1 << SEC_CIPHER_WEP40)) > 0)
#define IS_CIPHER_WEP104(_Cipher)        (((_Cipher) & (1 << SEC_CIPHER_WEP104)) > 0)
#define IS_CIPHER_WEP128(_Cipher)        (((_Cipher) & (1 << SEC_CIPHER_WEP128)) > 0)
#define IS_CIPHER_WEP(_Cipher)              (((_Cipher) & ((1 << SEC_CIPHER_WEP40) | (1 << SEC_CIPHER_WEP104) | (1 << SEC_CIPHER_WEP128))) > 0)
#define IS_CIPHER_TKIP(_Cipher)              (((_Cipher) & (1 << SEC_CIPHER_TKIP)) > 0)
#define IS_CIPHER_WEP_TKIP_ONLY(_Cipher)     ((IS_CIPHER_WEP(_Cipher) || IS_CIPHER_TKIP(_Cipher)) && (_Cipher < (1 << SEC_CIPHER_CCMP128)))
#define IS_CIPHER_CCMP128(_Cipher)      (((_Cipher) & (1 << SEC_CIPHER_CCMP128)) > 0)
#define IS_CIPHER_CCMP256(_Cipher)      (((_Cipher) & (1 << SEC_CIPHER_CCMP256)) > 0)
#define IS_CIPHER_GCMP128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_GCMP128)) > 0)
#define IS_CIPHER_GCMP256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_GCMP256)) > 0)
#define IS_CIPHER_BIP_CMAC128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_CMAC128)) > 0)
#define IS_CIPHER_BIP_CMAC256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_CMAC256)) > 0)
#define IS_CIPHER_BIP_GMAC128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_GMAC128)) > 0)
#define IS_CIPHER_BIP_GMAC256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_GMAC256)) > 0)



typedef enum _RT_802_11_PHY_MODE {
	PHY_11BG_MIXED = 0,
	PHY_11B = 1,
	PHY_11A = 2,
	PHY_11ABG_MIXED = 3,
	PHY_11G = 4,
	PHY_11ABGN_MIXED = 5,	/* both band   5 */
	PHY_11N_2_4G = 6,		/* 11n-only with 2.4G band      6 */
	PHY_11GN_MIXED = 7,		/* 2.4G band      7 */
	PHY_11AN_MIXED = 8,		/* 5G  band       8 */
	PHY_11BGN_MIXED = 9,	/* if check 802.11b.      9 */
	PHY_11AGN_MIXED = 10,	/* if check 802.11b.      10 */
	PHY_11N_5G = 11,		/* 11n-only with 5G band                11 */

	PHY_11VHT_N_ABG_MIXED = 12, /* 12 -> AC/A/AN/B/G/GN mixed */
	PHY_11VHT_N_AG_MIXED = 13, /* 13 -> AC/A/AN/G/GN mixed  */
	PHY_11VHT_N_A_MIXED = 14, /* 14 -> AC/AN/A mixed in 5G band */
	PHY_11VHT_N_MIXED = 15, /* 15 -> AC/AN mixed in 5G band */
	PHY_11AX_24G = 16,
	PHY_11AX_5G = 17,
	PHY_11AX_6G = 18,
	PHY_11AX_24G_6G = 19,
	PHY_11AX_5G_6G = 20,
	PHY_11AX_24G_5G_6G = 21,
#ifdef DOT11_EHT_BE
	PHY_11BE_24G = 22,
	PHY_11BE_5G = 23,
	PHY_11BE_6G = 24,
	PHY_11BE_24G_6G = 25,
	PHY_11BE_5G_6G = 26,
	PHY_11BE_24G_5G_6G = 27,
#endif
	PHY_MODE_MAX,
} RT_802_11_PHY_MODE;


static UCHAR CFG_WMODE_MAP[] = {
	PHY_11BG_MIXED, (IWINFO_80211_B | IWINFO_80211_G), /* 0 => B/G mixed */
	PHY_11B, (IWINFO_80211_B), /* 1 => B only */
	PHY_11A, (IWINFO_80211_A), /* 2 => A only */
	PHY_11ABG_MIXED, (IWINFO_80211_A | IWINFO_80211_B | IWINFO_80211_G), /* 3 => A/B/G mixed */
	PHY_11G, IWINFO_80211_G, /* 4 => G only */
	PHY_11ABGN_MIXED, (IWINFO_80211_B | IWINFO_80211_G | IWINFO_80211_N | IWINFO_80211_A), /* 5 => A/B/G/GN/AN mixed */
	PHY_11N_2_4G, (IWINFO_80211_N), /* 6 => N in 2.4G band only */
	PHY_11GN_MIXED, (IWINFO_80211_G | IWINFO_80211_N), /* 7 => G/GN, i.e., no CCK mode */
	PHY_11AN_MIXED, (IWINFO_80211_A | IWINFO_80211_N), /* 8 => A/N in 5 band */
	PHY_11BGN_MIXED, (IWINFO_80211_B | IWINFO_80211_G | IWINFO_80211_N), /* 9 => B/G/GN mode*/
	PHY_11AGN_MIXED, (IWINFO_80211_G | IWINFO_80211_N | IWINFO_80211_A), /* 10 => A/AN/G/GN mode, not support B mode */
	PHY_11N_5G, (IWINFO_80211_N), /* 11 => only N in 5G band */
	PHY_11VHT_N_ABG_MIXED, (IWINFO_80211_B | IWINFO_80211_G | IWINFO_80211_N | IWINFO_80211_A | IWINFO_80211_AC), /* 12 => B/G/GN/A/AN/AC mixed*/
	PHY_11VHT_N_AG_MIXED, (IWINFO_80211_G | IWINFO_80211_N | IWINFO_80211_A | IWINFO_80211_AC), /* 13 => G/GN/A/AN/AC mixed , no B mode */
	PHY_11VHT_N_A_MIXED, (IWINFO_80211_A | IWINFO_80211_N | IWINFO_80211_AC), /* 14 => A/AC/AN mixed */
	PHY_11VHT_N_MIXED, (IWINFO_80211_N | IWINFO_80211_AC), /* 15 => AC/AN mixed, but no A mode */
	PHY_11AX_24G, (IWINFO_80211_B | IWINFO_80211_G | IWINFO_80211_N | IWINFO_80211_AX),
	PHY_11AX_5G, (IWINFO_80211_A | IWINFO_80211_N | IWINFO_80211_AC | IWINFO_80211_AX),
	PHY_11AX_6G, (IWINFO_80211_AX),
	PHY_11AX_24G_6G, (IWINFO_80211_G | IWINFO_80211_N | IWINFO_80211_AX),
	PHY_11AX_5G_6G, (IWINFO_80211_A | IWINFO_80211_N | IWINFO_80211_AC | IWINFO_80211_AX),
	PHY_11AX_24G_5G_6G, (IWINFO_80211_G | IWINFO_80211_A | IWINFO_80211_N | IWINFO_80211_AC | IWINFO_80211_AX),
#ifdef DOT11_EHT_BE
	PHY_11BE_24G,
	 (IWINFO_80211_B | IWINFO_80211_G | IWINFO_80211_N | IWINFO_80211_AX | IWINFO_80211_BE),
	PHY_11BE_5G,
	(IWINFO_80211_A | IWINFO_80211_N | IWINFO_80211_AC | IWINFO_80211_AX | IWINFO_80211_BE),
	PHY_11BE_6G,
	(IWINFO_80211_A | IWINFO_80211_N | IWINFO_80211_AC | IWINFO_80211_AX | IWINFO_80211_BE),
	PHY_11BE_24G_6G,
	(IWINFO_80211_G | IWINFO_80211_N | IWINFO_80211_AX | IWINFO_80211_BE),
	PHY_11BE_5G_6G,
	(IWINFO_80211_A | IWINFO_80211_N | IWINFO_80211_AC | IWINFO_80211_AX | IWINFO_80211_BE),
	PHY_11BE_24G_5G_6G,
	(IWINFO_80211_G | IWINFO_80211_A | IWINFO_80211_N | IWINFO_80211_AC | IWINFO_80211_AX | IWINFO_80211_BE),
#endif /* DOT11_EHT_BE */

	PHY_MODE_MAX, 0 /* default phy mode if not match */
};

struct mtk_channel{
	UINT32 ic_freq;	/* setting in MHz */
	UCHAR ic_ieee;	/* IEEE channel number */
	UCHAR ic_power;	/* maximum regulatory tx power in dBm */
	UCHAR ic_power2;	/* maximum tx power in dBm */
	UCHAR ic_flag;	/* minimum tx power in dBm */
};



struct MTK_CHANINFO {
	UCHAR mtk_nchans;
	struct mtk_channel mtk_chans[60];
};

typedef struct _WLAN_BSS_INFO
{
	char ssid[33];			
	unsigned char bssid[6];	
	unsigned char securityEnable;	
	unsigned int  phymode; 	  
	unsigned int channel;			
	unsigned int rssi;	
	unsigned int authMode;
	unsigned int encrypType;
}WLAN_BSS_INFO;

typedef struct _WLAN_STA_INFO
{
	UCHAR			addr[6];
	UCHAR			aid;
	CHAR 			avgRssi0;
    CHAR 			avgRssi1;
	INT64			txPackets;
	INT64 			rxPackets;
	ULONG 			lastTxRate;
	ULONG 			lastRxRate;
	UINT32 			connectedTime;
}WLAN_STA_INFO;


typedef struct _WLAN_STA_INFO_TABLE {
        UINT32 Num;
        WLAN_STA_INFO Entry[MAX_STA_NUM];
} WLAN_STA_INFO_TABLE, *PWLAN_STA_INFO_TABLE;

typedef struct _SCAN_BSS_TABLE {
	UINT32			BssNr;
	WLAN_BSS_INFO	ScanTbl[MAX_NUM_OF_SURVEY_CNT];
} SCAN_BSS_TABLE, *P_SCAN_BSS_TABLE;


/* MIMO Tx parameter, ShortGI, MCS, STBC, etc.  these are fields in TXWI. Don't change this definition!!! */
typedef union _MACHTTRANSMIT_SETTING {
        struct {
                USHORT MCS:7;   /* MCS */
                USHORT BW:1;    /*channel bandwidth 20MHz or 40 MHz */
                USHORT ShortGI:1;
                USHORT STBC:2;  /*SPACE */
                USHORT rsv:3;
                USHORT MODE:2;  /* Use definition MODE_xxx. */
        } field;
        USHORT word;
} MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
        UCHAR ApIdx;
        UCHAR Addr[MAC_ADDR_LENGTH];
        UCHAR Aid;
        UCHAR Psm;              /* 0:PWR_ACTIVE, 1:PWR_SAVE */
        UCHAR MimoPs;           /* 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled */
        CHAR AvgRssi0;
        CHAR AvgRssi1;
        CHAR AvgRssi2;
        UINT32 ConnectedTime;
        MACHTTRANSMIT_SETTING TxRate;
        UINT32          LastRxRate;
        SHORT           StreamSnr[3];                           /* BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed */
        SHORT           SoundingRespSnr[3];                     /* SNR from Sounding Response. Units=0.25 dB. 22 dB offset removed */
/*      SHORT           TxPER;  */                                      /* TX PER over the last second. Percent */
/*      SHORT           reserved;*/
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
        ULONG Num;
        RT_802_11_MAC_ENTRY Entry[MAX_STA_NUM];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;

typedef struct _CH_FREQ_MAP_{
	int	channel;
	int	freqMHz;
}CH_FREQ_MAP;
#if 0
CH_FREQ_MAP CH_HZ_ID_MAP[]=
	{
		{1, 2412},
		{2, 2417},
		{3, 2422},
		{4, 2427},
		{5, 2432},
		{6, 2437},
		{7, 2442},
		{8, 2447},
		{9, 2452},
		{10, 2457},
		{11, 2462},
		{12, 2467},
		{13, 2472},
		{14, 2484},

		/*  UNII */
		{36, 5180},
		{40, 5200},
		{44, 5220},
		{48, 5240},
		{52, 5260},
		{56, 5280},
		{60, 5300},
		{64, 5320},
		{149, 5745},
		{153, 5765},
		{157, 5785},
		{161, 5805},
		{165, 5825},
		{167, 5835},
		{169, 5845},
		{171, 5855},
		{173, 5865},

		/* HiperLAN2 */
		{100, 5500},
		{104, 5520},
		{108, 5540},
		{112, 5560},
		{116, 5580},
		{120, 5600},
		{124, 5620},
		{128, 5640},
		{132, 5660},
		{136, 5680},
		{140, 5700},

		/* Japan MMAC */
		{34, 5170},
		{38, 5190},
		{42, 5210},
		{46, 5230},

		/*  Japan */
		{184, 4920},
		{188, 4940},
		{192, 4960},
		{196, 4980},

		{208, 5040},	/* Japan, means J08 */
		{212, 5060},	/* Japan, means J12 */   
		{216, 5080},	/* Japan, means J16 */
};
#endif

#endif // __MTK_H__


