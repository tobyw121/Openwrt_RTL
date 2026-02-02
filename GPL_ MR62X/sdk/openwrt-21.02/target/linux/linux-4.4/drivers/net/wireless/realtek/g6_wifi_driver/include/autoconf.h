/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
/***** temporarily flag for IC development phase *******/
#define CONFIG_SINGLE_IMG

/*#define CONFIG_NO_FW*/
/*#define CONFIG_DISABLE_ODM*/

#define CONFIG_TRUNCATE_MODE


/*
 * Work around Config
 */
#define RTW_WKARD_FREE_IFDOWN_RXBUF
#define RTW_WKARD_REGD_DEFAULT_SET
#define RTW_WKARD_DIS_PROBE_REQ_RPT_TO_HOSTAPD
#define RTW_WKARD_PWRLIMIT_DISABLE



/***** temporarily flag *******/
/*
 * Public  General Config
 */
#define AUTOCONF_INCLUDED
#define DRV_NAME "rtk_wifi6"

#ifndef CONFIG_PCI_HCI
#define CONFIG_PCI_HCI
#endif

/*
 * Wi-Fi Functions Config
 */

#define CONFIG_RECV_REORDERING_CTRL

#define CONFIG_80211N_HT
#define CONFIG_80211AC_VHT
#define CONFIG_80211AX_HE
#ifdef CONFIG_80211AC_VHT
	#ifndef CONFIG_80211N_HT
		#define CONFIG_80211N_HT
	#endif
#endif

#ifdef CONFIG_80211AC_VHT
	/*#define CONFIG_BEAMFORMING*/
#endif
#ifdef CONFIG_80211AX_HE
	#ifndef CONFIG_80211N_HT
		#define CONFIG_80211N_HT
	#endif
	#ifndef CONFIG_80211AC_VHT
		#define CONFIG_80211AC_VHT
	#endif
	#ifndef CONFIG_RTW_MBO
		#define CONFIG_RTW_MBO
	#endif
	#ifndef CONFIG_RTW_TWT
		#define CONFIG_RTW_TWT
		#define CONFIG_RTW_TWT_DBG
	#endif
#endif

#ifdef CONFIG_RTW_MBO
	#ifndef CONFIG_RTW_WNM
		#define CONFIG_RTW_WNM
	#endif
	#ifndef CONFIG_RTW_80211K
		#define CONFIG_RTW_80211K
	#endif
#endif /* CONFIG_RTW_MBO */

/*#define CONFIG_IOCTL_CFG80211*/
#ifdef CONFIG_IOCTL_CFG80211
	/*#define RTW_USE_CFG80211_STA_EVENT*/ /* Indecate new sta asoc through cfg80211_new_sta */
	#define CONFIG_CFG80211_FORCE_COMPATIBLE_2_6_37_UNDER
	/*#define CONFIG_DEBUG_CFG80211*/
	/*#define CONFIG_DRV_ISSUE_PROV_REQ*/ /* IOT FOR S2 */
	#define CONFIG_SET_SCAN_DENY_TIMER
#endif
#define CONFIG_TX_AMSDU
#ifdef CONFIG_TX_AMSDU
	#ifdef CONFIG_PLATFORM_RTL8198D
	#define CONFIG_TX_AMSDU_HW_MODE	1
	#else
	#define CONFIG_TX_AMSDU_SW_MODE	1
	#endif
#endif

/*
 * Internal  General Config
 */
/*#define CONFIG_PWRCTRL*/
/*#define CONFIG_H2CLBK*/
#define CONFIG_TRX_BD_ARCH	/* PCI only */
#define USING_RX_TAG

#define CONFIG_EMBEDDED_FWIMG

#ifdef CONFIG_EMBEDDED_FWIMG
	#define	LOAD_FW_HEADER_FROM_DRIVER
#endif
/*#define CONFIG_FILE_FWIMG*/

/* #define CONFIG_XMIT_ACK */
#ifdef CONFIG_XMIT_ACK
	#define CONFIG_ACTIVE_KEEP_ALIVE_CHECK
#endif

#define BUF_DESC_ARCH		/* if defined, hardware follows Rx buffer descriptor architecture */

#ifdef CONFIG_POWER_SAVING

	#define CONFIG_IPS
	#ifdef CONFIG_IPS
	#endif

	#define CONFIG_LPS

	#if defined(CONFIG_LPS)
		/*#define CONFIG_LPS_LCLK*/ /* 32K */
	#endif

	#ifdef CONFIG_LPS_LCLK
		#define CONFIG_XMIT_THREAD_MODE
		#define LPS_RPWM_WAIT_MS 300
	#endif
#endif

	/*#define CONFIG_ANTENNA_DIVERSITY*/


/*#define CONFIG_PCI_ASPM*/
#ifdef CONFIG_PCI_ASPM
#define CONFIG_PCI_DYNAMIC_ASPM
#endif

#define CONFIG_AP_MODE
#ifdef CONFIG_AP_MODE
	#define CONFIG_NATIVEAP_MLME
	#ifndef CONFIG_NATIVEAP_MLME
		#define CONFIG_HOSTAPD_MLME
	#endif
	#ifdef CONFIG_80211AX_HE
		#ifndef CONFIG_RTW_MBO
			#define CONFIG_RTW_MBO
		#endif
	#endif
	/*#define CONFIG_FIND_BEST_CHANNEL*/
	/*#define CONFIG_AUTO_AP_MODE*/
	#define CONFIG_RTW_2G_40M_COEX
	#define CONFIG_RTW_GENERAL_RX_FILTER_MODE

	#define PHL_PLATFORM_AP
#endif

/* #define CONFIG_P2P */
#ifdef CONFIG_P2P
	/* The CONFIG_WFD is for supporting the Wi-Fi display */
	#define CONFIG_WFD

	#define CONFIG_P2P_REMOVE_GROUP_INFO

	/*#define CONFIG_DBG_P2P*/

	#define CONFIG_P2P_PS
	/*#define CONFIG_P2P_IPS*/
	#define CONFIG_P2P_OP_CHK_SOCIAL_CH
	#define CONFIG_CFG80211_ONECHANNEL_UNDER_CONCURRENT  /* replace CONFIG_P2P_CHK_INVITE_CH_LIST flag */
	/*#define CONFIG_P2P_INVITE_IOT*/
#endif

/* Added by Kurt 20110511 */
#ifdef CONFIG_TDLS
	#define CONFIG_TDLS_DRIVER_SETUP
#if 0
	#ifndef CONFIG_WFD
		#define CONFIG_WFD
	#endif
	#define CONFIG_TDLS_AUTOSETUP
#endif
	#define CONFIG_TDLS_AUTOCHECKALIVE
	/* #define CONFIG_TDLS_CH_SW */	/* Enable this flag only when we confirm that TDLS CH SW is supported in FW */
#endif

#define CONFIG_SKB_COPY	/* for amsdu */

/*#define CONFIG_RTW_LED*/
#ifdef CONFIG_RTW_LED
	/*#define CONFIG_RTW_SW_LED*/
	#ifdef CONFIG_RTW_SW_LED
		/*#define CONFIG_RTW_LED_HANDLED_BY_CMD_THREAD*/
	#endif
#endif /* CONFIG_RTW_LED */

#define CONFIG_GLOBAL_UI_PID

#define CONFIG_LAYER2_ROAMING
#define CONFIG_LAYER2_ROAMING_RESUME
/*#define CONFIG_ADAPTOR_INFO_CACHING_FILE*/ /* now just applied on 8192cu only, should make it general...*/
/*#define CONFIG_RESUME_IN_WORKQUEUE*/
/*#define CONFIG_SET_SCAN_DENY_TIMER*/
#define CONFIG_LONG_DELAY_ISSUE
#define CONFIG_NEW_SIGNAL_STAT_PROCESS
/*#define CONFIG_SIGNAL_DISPLAY_DBM*/ /* display RX signal with dbm */
#ifdef CONFIG_SIGNAL_DISPLAY_DBM
/*#define CONFIG_BACKGROUND_NOISE_MONITOR*/
#endif

#define CONFIG_TX_MCAST2UNI		/* Support IP multicast->unicast*/

/*
 * Wi-Fi Alliance (Logo) Related Config
 */
#define WIFI_LOGO_HT_4_2_47		/* use sw mode to handle null data */
#define WIFI_LOGO_HE_4_5_3		/* 5G: Marvell NSS2 Rx (set PPE) */
#define WIFI_LOGO_HE_4_7_1		/* Dual Band: ADDBA RXAMPDU SZ */
#define WIFI_LOGO_HE_4_20_1		/* 24G: Tx LDPC */
#define WIFI_LOGO_HE_4_52_1		/* WKARD A-Control OM */
#define WIFI_LOGO_HE_4_56_1		/* TWT */
#define WIFI_LOGO_MBO_4_2_5_3	/* MBO */
#define WIFI_LOGO_MBO_4_2_5_4	/* MBO */
#define WIFI_LOGO_11N_WMM       /* 2.4G 4.2.22, 4.2.23 */
#define WIFI_LOGO_VHT_4_2_44


/*
 * Software feature Related Config
 */


/*
 * Interface  Related Config
 */

/*
 * HAL  Related Config
 */
#define CONFIG_RX_PACKET_APPEND_FCS


#define DISABLE_BB_RF	0

#ifdef CONFIG_MP_INCLUDED
	#define MP_DRIVER 1
#else
	#define MP_DRIVER 0
#endif

#ifndef EFUSE_MAP_PATH
	#define EFUSE_MAP_PATH "/system/etc/wifi/wifi_efuse.map"
#endif
#ifndef WIFIMAC_PATH
	#define WIFIMAC_PATH "/data/wifimac.txt"
#endif

/* Use cmd frame to issue beacon. Use a fixed buffer for beacon. */
#define CONFIG_BCN_ICF

#ifdef CONFIG_HWSIM
/* Use pure sw beacon */
#undef CONFIG_BCN_ICF
#endif

/* #define RTL8814BE_AMPDU_PRE_TX_OFF */

/*
 * Platform  Related Config
 */
#ifdef CONFIG_ARCH_CORTINA
#define WKARD_CHANGE_IFACE

#define CONFIG_RTW_LED
#ifdef CONFIG_RTW_LED
	#define CONFIG_RTW_SW_LED
#endif
#endif

/* #define	CONFIG_TX_EARLY_MODE */


/*
 * Debug Related Config
 */
#define DBG	1
#define DBG_PRINT_MAC_ERR_STATUS
/*#define CONFIG_RTW_DEBUG_CAM*/

/*#define DBG_CONFIG_ERROR_DETECT*/
/* #define DBG_CONFIG_ERROR_DETECT_INT */
/* #define DBG_CONFIG_ERROR_RESET */

/* #define DBG_IO */
/* #define DBG_DELAY_OS */
/* #define DBG_MEM_ALLOC */
/* #define DBG_IOCTL */

/* #define DBG_TX */
/* #define DBG_XMIT_BUF */
/* #define DBG_XMIT_BUF_EXT */
/* #define DBG_TX_DROP_FRAME */

/* #define DBG_RX_DROP_FRAME */
/* #define DBG_RX_SEQ */
/* #define DBG_RX_SIGNAL_DISPLAY_PROCESSING */
/* #define DBG_RX_SIGNAL_DISPLAY_SSID_MONITORED "jeff-ap" */

/* #define DBG_ROAMING_TEST */

/* #define DBG_HAL_INIT_PROFILING */

/*#define DBG_MEMORY_LEAK*/
/* #define CONFIG_FW_C2H_DEBUG */

#define CONFIG_DBG_COUNTER
#ifdef CONFIG_DBG_COUNTER
#define CONFIG_DBG_HNDSK_MGMT
#endif
#define	DBG_RX_DFRAME_RAW_DATA
#define CONFIG_RA_TXSTS_DBG
/*#define	DBG_TXBD_DESC_DUMP*/

#define CONFIG_PCI_TRX_RES_DBG

#define CONFIG_PCI_BCN_POLLING
//#define RTW_PHL_TEST_FPGA //For 8852A PCIE FPGA TEST

#ifndef CONFIG_DYNAMIC_RX_BUF
#define CONFIG_DYNAMIC_RX_BUF
#endif
#ifndef CONFIG_RELEASE_RPT
#define CONFIG_RELEASE_RPT
#endif

#ifdef CONFIG_RTW_NAPI
#undef CONFIG_RTW_NAPI
#endif
#ifdef CONFIG_RTW_GRO
#undef CONFIG_RTW_GRO
#endif

/* Enable L2 SER handling to recover FW/hardware failures */
#define CONFIG_RTW_HANDLE_SER_L2
#ifdef CONFIG_RTW_HANDLE_SER_L2
	#define CONFIG_PHL_HANDLE_SER_L2
/* Debugging function for L2 SER handling */
/* #define CONFIG_ENABLE_MAC_H2C_AGG */
/* #define CONFIG_ENABLE_MAC_KEY_BACKUP */
#ifndef CONFIG_LMT_TXREQ
	#define CONFIG_LMT_TXREQ
#endif
#endif

