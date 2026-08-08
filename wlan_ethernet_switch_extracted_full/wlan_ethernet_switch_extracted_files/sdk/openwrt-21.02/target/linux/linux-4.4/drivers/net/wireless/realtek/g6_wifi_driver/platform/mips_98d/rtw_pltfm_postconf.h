/******************************************************************************
 *
 * Copyright(c) 2020 Realtek Corporation.
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

#ifndef PLATFORM_MIPS_98D_RTW_PLTFM_POSTCONF_H_
#define PLATFORM_MIPS_98D_RTW_PLTFM_POSTCONF_H_

#ifndef CONFIG_BR_EXT_BRNAME
#define CONFIG_BR_EXT_BRNAME CONFIG_TP_LAN_BRIDGE_NAME
#endif

/* === Force Disable ============= */
#ifdef RTW_WKARD_RATE_DRV_CTRL
#undef RTW_WKARD_RATE_DRV_CTRL
#endif
#ifdef RTW_WKARD_RATE_INIT_6M
#undef RTW_WKARD_RATE_INIT_6M
#endif
#ifdef CONFIG_TX_AMSDU_HW_MODE
#undef CONFIG_TX_AMSDU_HW_MODE
#endif
#ifdef CONFIG_TX_AMSDU_SW_MODE
#undef CONFIG_TX_AMSDU_SW_MODE
#endif

/* === CPU Load Balance ======== */
#define RTW_TX_CPU_BALANCE
#define RTW_RX_CPU_BALANCE

/* Separate TX path into different CPUs */
#ifdef RTW_TX_CPU_BALANCE
	#define CPU_ID_TX_CORE		2
#endif

/* Separate RX path into different CPUs */
#ifdef RTW_RX_CPU_BALANCE
	#define CPU_ID_RX_CORE		1
	#define CPU_ID_RX_REFILL	0
	#define CPU_ID_TBD		0
#endif

/* Recycle RX packet by different CPU */
#ifdef CONFIG_SMP_PHL_RX_RECYCLE
	#define CPU_ID_RX_RECYCLE	0
#endif

/* Indicate to netif by different CPU */
#ifdef CONFIG_SMP_NETIF_RX
	#define CPU_ID_NETIF_RX		3
#endif

/* === TX Features ============= */

/* Power Saving */
#define USE_HIQ
//#define PSPOLL_BY_H2C
#ifdef CONFIG_PROFILING
#define CONFIG_CPU_PROFILING
#endif

//#define CONFIG_PHL_015_devp_tmp

/* CORE TX path shortcut */
//#ifndef CONFIG_RTW_A4_STA		//TODO: Shortcut in A4 sta mode
#define CONFIG_TX_DEFER // for CPU loading

/* for 52C 160M MAX TRX TP */
#ifdef CONFIG_RTL8852C
#define CONFIG_BW160M_EXTREME_THROUGHPUT_TX
//#define CONFIG_BW160M_EXTREME_THROUGHPUT_RX
#endif

#ifdef CONFIG_PHL_015_devp_tmp
#define CONFIG_PHL_TX_STATS_CORE // for CPU loading
#endif
#define CONFIG_CORE_TXSC
#define CONFIG_DIS_TX_INT_CHECK // for CPU loading
//#endif
#ifdef CONFIG_CORE_TXSC
#define CONFIG_RTW_TXSC_USE_HW_SEQ
/* TX shorcut amsdu */
#ifdef CONFIG_TX_AMSDU
#define CONFIG_TXSC_AMSDU
#define CONFIG_AMSDU_HW_TIMER
#endif
/* PHL TX path shortcut */
#define CONFIG_PHL_TXSC
#ifdef CONFIG_PHL_TXSC
#endif
#endif/* CONFIG_CORE_TXSC */

/* Dynamic EDCA */
#define CONFIG_RTW_MANUAL_EDCA
#define CONFIG_DYNAMIC_THROUGHPUT_ENGINE

/* Veriwave Test Refine */
#ifndef CONFIG_RTW_LINK_PHL_MASTER
#define CONFIG_VW_REFINE
//#define CONFIG_ONE_TXQ
#endif /* CONFIG_RTW_LINK_PHL_MASTER */
#define MAX_SKB_XMIT_QUEUE NUM_STA
#define CONFIG_LMT_TXREQ
#define CONFIG_DYN_ALLOC_XMITFRAME
#ifdef CONFIG_FILE_FWIMG
#define CONFIG_DYN_FW_LOAD
#endif
/* for tx cpu load balance */
#ifdef CONFIG_VW_REFINE
#define RTW_CORE_TX_MSDU_TRANSFER_IN_PHL
#endif

#define DYNAMIC_RTS_CONTROL
/* Due to b-cut cts2self doesn't work well, use rtscts instead */
/* #define WKARD_RTSCTS_EN */

/* Indicate to netif by different CPU */
#define CONFIG_SMP_NETIF_RX
#ifdef CONFIG_SMP_NETIF_RX
	#define CPU_ID_NETIF_RX		3
#endif

#define AMSDU_MAX_NUM 5

#define RTW_VW_TXAMSDU_ENHANCE

/* === RX Features ============= */

/* PHL RX path shortcut */
#define PHL_RXSC_ISR
#define PHL_RXSC_AMPDU

#define PHL_RX_BATCH_IND

/* CORE RX path shortcut */
//#ifndef CONFIG_RTW_A4_STA		//TODO: Shortcut in A4 sta mode
#define CONFIG_RTW_CORE_RXSC
//#endif
#ifdef CONFIG_RTW_CORE_RXSC
#define CORE_RXSC_RFRAME
#define CORE_RXSC_AMSDU
#endif

/* diff WLAN# apply diff RX BUF settings */
#define DEBUG_RXBUF_BY_DEV

/* Condig RX BUF settings by CORE */
#define CONFIG_RXBUF_BY_CORE

#ifdef CONFIG_RXBUF_BY_CORE

#ifdef RTL_PRIV_DATA_SIZE
#define SKB_OVERHEAD			(NET_SKB_PAD + RTL_PRIV_DATA_SIZE + SKB_DATA_ALIGN(sizeof(struct skb_shared_info)))
#else
#ifdef CONFIG_RTW_MEMPOOL
#define SKB_OVERHEAD			(NET_SKB_PAD + SKB_DATA_ALIGN(sizeof(struct skb_shared_info))+32)
#else
#define SKB_OVERHEAD			(NET_SKB_PAD + SKB_DATA_ALIGN(sizeof(struct skb_shared_info)))
#endif
#endif /* RTL_PRIV_DATA_SIZE */

/* RXQ_0_BASE_BUF_SIZE specified smallest number of 2's power to include
   maximum expected RX frame size */
#ifdef  CONFIG_RTW_RX_BUF_SHARING
#define RTW_RX_BUF_SHARE_FACTOR (2)
#else
#define RTW_RX_BUF_SHARE_FACTOR (1)
#endif

#define RXQ_0_BASE_BUF_SIZE		(8192)
#define RXQ_0_BUF_SIZE			(RXQ_0_BASE_BUF_SIZE * RTW_RX_BUF_SHARE_FACTOR)
#define MAX_RX_BD_NUM_RXQ_0_CORE	(512)
#define MAX_RX_BUF_NUM_RXQ_0_CORE	((MAX_RX_BD_NUM_RXQ_0_CORE * 3) / 2)
#define RX_BUF_SIZE_RXQ_0_CORE		((RXQ_0_BUF_SIZE - SKB_OVERHEAD)/RTW_RX_BUF_SHARE_FACTOR)

#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
#define ONE_MSDU_BUF_SIZE		(2048)
#define RX_BUF_ONE_MSDU_SIZE		(ONE_MSDU_BUF_SIZE - SKB_OVERHEAD)
#define MAX_MSDU_RXBD_NUM		(512)
#define MAX_MSDU_RXBUF_NUM		(MAX_MSDU_RXBD_NUM * 2)
#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

/*rpq, number of rxbd & rxbuf, size of rxbuf*/
#define MAX_RX_BD_NUM_RPQ_0_CORE	(64)
#define MAX_RX_BUF_NUM_RPQ_0_CORE	(MAX_RX_BD_NUM_RPQ_0_CORE*2)
/* Release report maximum packet size is 508B */
#ifdef CONFIG_RTW_RX_BUF_SHARING
#ifdef CONFIG_RTW_MEMPOOL
#define RPQ_0_BUF_SIZE			(640 * RTW_RX_BUF_SHARE_FACTOR)
#else
#define RPQ_0_BUF_SIZE			(512 * RTW_RX_BUF_SHARE_FACTOR)
#endif
#else
#ifdef CONFIG_RTW_MEMPOOL
/*RPQ_0_BUF_SIZE != RPQ_1_BUF_SIZE since using MEMPOOL*/
#define RPQ_0_BUF_SIZE			(640)
#else
#define RPQ_0_BUF_SIZE			(512)
#endif
#endif
#define RX_BUF_SIZE_RPQ_0_CORE		((RPQ_0_BUF_SIZE - SKB_OVERHEAD)/RTW_RX_BUF_SHARE_FACTOR)

#if defined(DEBUG_RXBUF_BY_DEV)
/*rxq, number of rxbd & rxbuf, size of rxbuf*/
#define MAX_RX_BD_NUM_RXQ_1_CORE	(192)
#define MAX_RX_BUF_NUM_RXQ_1_CORE	(MAX_RX_BD_NUM_RXQ_1_CORE*3)
#define RXQ_1_BASE_BUF_SIZE		(4096)
#define RXQ_1_BUF_SIZE			(RXQ_1_BASE_BUF_SIZE * RTW_RX_BUF_SHARE_FACTOR)
#define RX_BUF_SIZE_RXQ_1_CORE		((RXQ_1_BUF_SIZE - SKB_OVERHEAD)/RTW_RX_BUF_SHARE_FACTOR)
/*rpq, number of rxbd & rxbuf, size of rxbuf*/
#define MAX_RX_BD_NUM_RPQ_1_CORE	(64)
#define MAX_RX_BUF_NUM_RPQ_1_CORE	(MAX_RX_BD_NUM_RPQ_1_CORE*2)
#define RPQ_1_BUF_SIZE			(512 * RTW_RX_BUF_SHARE_FACTOR)
#define RX_BUF_SIZE_RPQ_1_CORE		((RPQ_1_BUF_SIZE - SKB_OVERHEAD)/RTW_RX_BUF_SHARE_FACTOR)

#endif /* DEBUG_RXBUF_BY_DEV */
#else/* CONFIG_RXBUF_BY_CORE */
/* DEV#0*/
/*rxq, number of rxbd & rxbuf*/
#define MAX_RX_BD_NUM_RXQ_0_CORE	256
#define MAX_RX_BUF_NUM_RXQ_0_CORE	512
#define RX_BUF_SIZE_RXQ_0_CORE	11460
/*rpq, number of rxbd & rx buf*/
#define MAX_RX_BD_NUM_RPQ_0_CORE	256
#define MAX_RX_BUF_NUM_RPQ_0_CORE	512
#define RX_BUF_SIZE_RPQ_0_CORE	11460

/* DEV#1*/
/*rxq, number of rxbd & rxbuf*/
#define MAX_RX_BD_NUM_RXQ_1_CORE	256
#define MAX_RX_BUF_NUM_RXQ_1_CORE	512
#define RX_BUF_SIZE_RXQ_1_CORE	11460
/*rpq, number of rxbd & rx buf*/
#define MAX_RX_BD_NUM_RPQ_1_CORE	256
#define MAX_RX_BUF_NUM_RPQ_1_CORE	512
#define RX_BUF_SIZE_RPQ_1_CORE	11460
#endif /* CONFIG_RXBUF_BY_CORE */


/* === OFDMA Features ============= */
#define CONFIG_WFA_OFDMA_Logo_Test
#ifdef CONFIG_WFA_OFDMA_Logo_Test
#define CONFIG_WFA_OFDMA_Logo_Test_Statistic
#endif
#define HALBB_RUA_SUPPORT

/* === Work Around ============= */

/* Special work arounds for 98D */
#define WKARD_98D

#define RTW_WKARD_98D_INTR_EN_TIMING
#define RTW_WKARD_AP_MP
/* temp remove, this is for phl rx lock, unknow issue now */
//#define RTW_WKARD_AP_RESET_RX_LOCK
/* WiFi On/Off with HW On/Off */
#define RTW_WKARD_INTF_RESET
#define RTW_WKARD_BEACON
#define RTW_WKARD_BCNINT_DBG
#define RTW_WKARD_REDUCE_CONNECT_LOG
#define RTW_WKARD_APSEC_MAXTXTP /* for 32br sec max throughput */
#define RTW_WKARD_REFINE_DBCC_TXTP /* for ax3000 dbcc tx tp refine */

#define RTW_SURVEY_EVENT_REFINE
#define RTW_MI_SHARE_BSS_LIST

#define RTW_PHL_DBG_CMD
#define RTW_CORE_PKT_TRACE

#ifdef CONFIG_RTW_MULTI_DEV_MULTI_BAND
#define WKARD_DBCC
#define CPU_ID_TX_PHL_0		3  // 2
#define CPU_ID_TX_PHL_1		2  // 3
#define CPU_ID_RX_CORE_0	1  // 0
#define CPU_ID_RX_CORE_1	0  // 1
#endif /* CONFIG_RTW_MULTI_DEV_MULTI_BAND */

/* force enable rx-amsdu & handled by skb clone */
#define WKARD_RX_AMSDU
#ifdef WKARD_RX_AMSDU
	#ifdef CONFIG_SKB_COPY
	#undef CONFIG_SKB_COPY
	#endif
#endif

/* === Debug ================= */
#define DEBUG_PHL_RX
#define DEBUG_PHL_TX

#if 1
#define DBGP(fmt, args...)	printk("dbg [%s][%d]"fmt, __FUNCTION__, __LINE__, ## args)
#else
#define DBGP(arg...) do {} while (0)
#endif

/* workarond for HALMAC use spinlock instad of */
#define CONFIG_RTW_HALMAC_USE_SPINLOCK

/* === Features ================ */
#ifdef CONFIG_RTW_SUPPORT_MBSSID_VAP

#define CONFIG_RTW_TEST_MBSSID_VAP
#define CONFIG_RTW_DEBUG_MBSSID_VAP	(6)

/* Temporarily support only 2 AP to reduce memory usage */
#ifndef CONFIG_LIMITED_AP_NUM
#define CONFIG_LIMITED_AP_NUM		(6)
#endif
#define CONFIG_LIMITED_VAP_NUM		(CONFIG_LIMITED_AP_NUM - 1)
//#define CONFIG_CLIENT_NUMBER		(CONFIG_PORT_NUMBER - 1)
#ifdef CONFIG_RTW_CLIENT_MODE_SUPPORT
#define CONFIG_CLIENT_NUMBER		(2)
#else
#define CONFIG_CLIENT_NUMBER		(1)
#endif

#define CONFIG_IFACE_NUMBER		(CONFIG_LIMITED_AP_NUM + CONFIG_CLIENT_NUMBER - 1)

#endif /* CONFIG_RTW_SUPPORT_MBSSID_VAP */

#define CONFIG_RTW_HOSTAPD_ACS
#ifdef CONFIG_RTW_HOSTAPD_ACS
	#define WKARD_ACS
	#define WKARD_ACS_DISABLE_DFS_CHANNEL
#endif /* CONFIG_RTW_HOSTAPD_ACS */

#define CONFIG_RTW_AP_EXT_SUPPORT

#define RTW_WKARD_VHT_AMPDU_LEN

/* A4 ADDR support */
#ifdef CONFIG_RTW_A4_STA
	/* Force TX A4 ADDR Data, except EAPOL */
	//#define DEBUG_A4_TXFORCE

	/* Force TX A4 ADDR EAPOL */
	//#define DEBUG_A4_EAPOL

	#define A4_TX_MCAST2UNI
	#define CONFIG_A4_LOOPBACK
	#define CONFIG_DYN_ALLOC_A4_TBL
#endif

#ifdef CONFIG_SLOT_1_TX_BEAMFORMING
#define TX_BEAMFORMING
#endif

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
#define CONFIG_MIB_PROC_CTL
#ifndef CONFIG_RTW_LINK_PHL_MASTER
// e278282411c54b03f65f53b2eb41789d806af49a
#define CONFIG_RTW_LED
#endif /* ONFIG_RTW_LINK_PHL_MASTER */
#define CONFIG_RTW_SW_LED
#ifndef CONFIG_RTW_LINK_PHL_MASTER
// 3b641da917a1efd9e3efae2ec9c1f18eb8d97bc0 chester.ku
#define CONFIG_LIFETIME_FEATURE
#endif /* CONFIG_RTW_LINK_PHL_MASTER */


#ifndef CONFIG_RTW_LINK_PHL_MASTER
// a97c3d86aa2809f76cd73b279dcc7972a4ad1180 chester.ku
#define POWER_PERCENT_ADJUSTMENT
#endif /* CONFIG_RTW_LINK_PHL_MASETER */
/* block sta connect */
#define RTW_BLOCK_STA_CONNECT
#endif


/* Multiap support */
#ifdef CONFIG_RTW_MULTI_AP
#define DEBUG_MAP_NL
#define DEBUG_MAP_UNASSOC
#endif

#ifdef CONFIG_RTW_COMMON_NETLINK
#define RTW_COMMON_NETLINK
#endif

#ifdef CONFIG_RTK_WLAN_EVENT_INDICATE
#ifndef RTK_WLAN_EVENT_INDICATE
#define RTK_WLAN_EVENT_INDICATE
#endif
#endif

#ifdef CONFIG_RTW_SBWC
#ifndef SBWC
#define SBWC
#endif
#endif

#ifdef CONFIG_RTW_GBWC
#ifndef GBWC
#define GBWC
#endif
#endif


#ifdef CONFIG_AP_NEIGHBOR_INFO
#ifndef AP_NEIGHBOR_INFO
#define AP_NEIGHBOR_INFO
#endif
#endif

#if defined(CONFIG_RTW_CURRENT_RATE_ACCOUNTING) || defined(CONFIG_RTW_CURRENT_RATE_ACCOUNTING)
#ifndef RTW_CURRENT_RATE_ACCOUNTING
#define RTW_CURRENT_RATE_ACCOUNTING
#endif
#endif

#ifdef RTK_WLAN_EVENT_INDICATE
/* report some info to users periodically */
#define REPORT_TIMER_SUPPORT

/* obtain unassociated sta info */
#define MONITOR_UNASSOC_STA

/* report roaming info to users */
#define RTL_LINK_ROAMING_REPORT
#endif

/* RF FEM config*/
#if defined(CONFIG_WLAN0_RFE_TYPE_50) || defined(CONFIG_WLAN1_RFE_TYPE_50) || \
	defined(CONFIG_WLAN0_RFE_TYPE_51) || defined(CONFIG_WLAN1_RFE_TYPE_51) || \
	defined(CONFIG_WLAN0_RFE_TYPE_52) || defined(CONFIG_WLAN1_RFE_TYPE_52) || \
	defined(CONFIG_WLAN0_RFE_TYPE_53) || defined(CONFIG_WLAN1_RFE_TYPE_53) || \
	defined(CONFIG_WLAN0_RFE_TYPE_54) || defined(CONFIG_WLAN1_RFE_TYPE_54)

#define CONFIG_RTW_ENABLE_SUB_TYPE
#endif


/*evented netlink*/
#ifdef RTK_WLAN_EVENT_INDICATE
extern int get_nl_eventd_pid(void);
extern struct sock *get_nl_eventd_sk(void);
extern void rtk_eventd_netlink_send(int pid, struct sock *nl_sk, int eventID, char *ifname, char *data, int data_len);
extern struct sock * rtk_eventd_netlink_init(void);
#define WLAN_INFO_LEN		2048
#endif

#define NUM_TXPKT_QUEUE		64
#define RTL_MILISECONDS_TO_JIFFIES(x) (((x)*HZ-1)/1000+1)
#ifdef SBWC
#define SBWC_PERIOD		250
#define SBWC_TO			RTL_MILISECONDS_TO_JIFFIES(SBWC_PERIOD)
#define TX_CONTINUE 0
#define SBWC_FREE_SKB 1
#define SBWC_QUEUE_SKB 2

#ifndef BIT
#define BIT(x)	(1 << (x))
#endif

typedef enum
{
	SBWC_MODE_DISABLE 		= 0,
	SBWC_MODE_LIMIT_STA_TX 	= BIT(0),
	SBWC_MODE_LIMIT_STA_RX 	= BIT(1),
	SBWC_MODE_LIMIT_STA_TRX	= BIT(0) | BIT(1),
} SBWC_MODE;

#endif

#ifdef RTW_STA_BWC
#define LOW_BOUNT_LIMIT		500 // in Kbps
#define STA_BWC_PERIOD		250
#define STA_BWC_TO			RTL_MILISECONDS_TO_JIFFIES(STA_BWC_PERIOD)
#define STA_BWC_TX_CONTINUE 0
#define STA_BWC_FREE_SKB 1
#define STA_BWC_QUEUE_SKB 2
#endif

#ifdef GBWC
#define GBWC_PERIOD		250
#define GBWC_TO			RTL_MILISECONDS_TO_JIFFIES(GBWC_PERIOD)
#define GBWC_TX_CONTINUE 0
#define GBWC_DROP_SKB 1
#define GBWC_QUEUE_SKB 2

typedef enum
{
	GBWC_MODE_DISABLE 			= 0,
	GBWC_MODE_LIMIT_MAC_INNER 	= 1,
	GBWC_MODE_LIMIT_MAC_OUTTER 	= 2,
	GBWC_MODE_LIMIT_IF_TX 		= 3,
	GBWC_MODE_LIMIT_IF_RX 		= 4,
	GBWC_MODE_LIMIT_IF_TRX		= 5,
} GBWC_MODE;
#endif

#ifdef CONFIG_RTW_DE_SUPPORT
#define CONFIG_WLAN_DE_SUPPORT
#endif

#ifdef CONFIG_CROSSBAND_REPEATER_SUPPORT_WIFI5_WIFI6
#ifndef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
#define CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
#endif
#endif

#ifndef DBG_MEM_ALLOC
//#define DBG_MEM_ALLOC

//#define DBG_PHL_MEM_ALLOC
//#define DBG_HAL_MAC_MEM_MOINTOR
//#define DBG_HAL_MEM_MOINTOR
#endif

#ifdef CONFIG_RX_WIFI_FF_AGG
#define CONFIG_ETHER_PKT_AGG

#ifdef RTW_CORE_TX_MSDU_TRANSFER_IN_PHL
#undef RTW_CORE_TX_MSDU_TRANSFER_IN_PHL
#endif
#endif /* CONFIG_RX_WIFI_FF_AGG */

#define CONFIG_RTW_DBG_TX_MGNT	(0)

#endif /* PLATFORM_MIPS_98D_RTW_PLTFM_POSTCONF_H_ */
