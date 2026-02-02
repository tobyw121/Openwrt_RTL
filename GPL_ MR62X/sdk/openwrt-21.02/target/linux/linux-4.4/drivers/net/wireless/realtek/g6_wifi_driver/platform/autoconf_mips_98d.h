#if 0 //def CONFIG_PLATFORM_RTL8198D

#define CONFIG_RTW_FC_FASTFWD

#ifndef CONFIG_RTW_SUPPORT_MBSSID_VAP
#define CONFIG_RTW_SUPPORT_MBSSID_VAP
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

/* === TX Features ============= */

/* CORE TX path shortcut */
#define CONFIG_CORE_TXSC
#ifdef CONFIG_CORE_TXSC
/* TX shorcut amsdu */
#ifdef CONFIG_TX_AMSDU
#define CONFIG_TXSC_AMSDU
#endif
/* PHL TX path shortcut */
#define CONFIG_PHL_TXSC
#endif/* CONFIG_CORE_TXSC */

/* Separate TX path into different CPUs */
//#define RTW_TX_CPU_BALANCE
#ifdef RTW_TX_CPU_BALANCE
	#define CPU_ID_TX_CORE		3
#endif

/* === RX Features ============= */

/* Separate RX path into different CPUs */
#define RTW_RX_CPU_BALANCE
#ifdef RTW_RX_CPU_BALANCE
	#define CPU_ID_RX_CORE		1
	#define CPU_ID_RX_REFILL	0
	#define CPU_ID_TBD		0
#endif

#define PHL_RXSC_ISR

/* === Work Around ============= */

/* Special work arounds for 98D */
#define WKARD_98D

/* WiFi On/Off with HW On/Off */
#define RTW_WKARD_INTF_RESET

#define RTW_WKARD_BEACON

/* force enable rx-amsdu & handled by skb clone */
#define WKARD_RX_AMSDU
#ifdef WKARD_RX_AMSDU
	#ifdef CONFIG_SKB_COPY
	#undef CONFIG_SKB_COPY
	#endif
#endif

/* === Debug ================= */
#define DEBUG_PHL_RX

#if 1
#define DBGP(fmt, args...)	printk("dbg [%s][%d]"fmt, __FUNCTION__, __LINE__, ## args)
#else
#define DBGP(arg...) do {} while (0)
#endif

#endif
