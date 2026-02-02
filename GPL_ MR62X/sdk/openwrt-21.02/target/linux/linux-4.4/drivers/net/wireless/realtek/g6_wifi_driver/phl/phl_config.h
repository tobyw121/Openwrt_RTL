/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#ifndef _PHL_CONFIG_H_
#define _PHL_CONFIG_H_

/* Define correspoding PHL Feature based on information from the Core */
#ifdef PHL_PLATFORM_AP
#define PHL_FEATURE_AP
#elif defined(PHL_PLATFORM_LINUX) || defined(PHL_PLATFORM_WINDOWS)
#define PHL_FEATURE_NIC
#else
#define PHL_FEATURE_NONE
#endif

/******************* PLATFORM Section **************************/
#ifdef PHL_FEATURE_NONE/* enable compile flag for phl only compilation check */
	#define CONFIG_DFS 1
	#define CONFIG_USB_TX_AGGREGATION
	#define CONFIG_USB_RX_AGGREGATION
	#define CONFIG_USB_TX_PADDING_CHK
	#define CONFIG_LOAD_PHY_PARA_FROM_FILE

	#define CONFIG_WOW
	#define CONFIG_WPA3_SUITEB_SUPPORT
	#define CONFIG_SYNC_INTERRUPT

	#define CONFIG_MR_SUPPORT
	#ifdef CONFIG_MR_SUPPORT
		#define CONFIG_SCC_SUPPORT
		#define CONFIG_MCC_SUPPORT
		#ifdef CONFIG_MCC_SUPPORT
			#define MCC_ROLE_NUM 2
			#define RTW_WKARD_GO_BT_TS_ADJUST_VIA_NOA
			#define RTW_WKARD_HALRF_MCC
		#endif /*CONFIG_MCC_SUPPORT*/

		#define CONFIG_DBCC_SUPPORT

		#define DBG_PHL_CHAN
		#define DBG_PHL_MR
		#define PHL_MR_PROC_CMD
		#define DBG_CHCTX_RMAP
	#endif /*CONFIG_MR_SUPPORT*/

	#define DBG_PHL_MAC_REG_RW

	#define CONFIG_RTW_ACS
	#define CONFIG_RX_PSTS_PER_PKT

	#define CONFIG_PHL_TXSC
	#define CONFIG_RTW_TXSC_USE_HW_SEQ
	#define RTW_PHL_BCN
	#define CONFIG_PHL_SDIO_RX_NETBUF_ALLOC_IN_PHL
	#define CONFIG_PHL_TWT
	#define CONFIG_CMD_DISP
	#ifdef CONFIG_CMD_DISP
		#define CONFIG_PHL_ECSA
		/*#define CONFIG_CMD_DISP_SOLO_MODE*/
		#define CONFIG_PHL_CMD_SCAN
		#define CONFIG_PHL_CMD_SER
		#define CONFIG_PHL_CMD_BTC
		#define CONFIG_PHL_CMD_BF
	#endif
	#ifdef CONFIG_PCI_HCI
		#define PCIE_TRX_MIT_EN
	#endif
	#define CONFIG_RTW_HW_RX_AMSDU_CUT
	#define DEBUG_PHL_RX
#define CONFIG_PHL_P2PPS
	#define CONFIG_6GHZ
	#define CONFIG_PHL_THERMAL_PROTECT
	#define CONFIG_PHL_RELEASE_RPT_ENABLE
	#define CONFIG_PHL_DRV_HAS_NVM
#endif /* PHL_FEATURE_NONE */

#ifdef PHL_PLATFORM_WINDOWS
	#ifndef CONFIG_FSM
		#define CONFIG_FSM
	#endif
	#ifndef CONFIG_CMD_DISP
		#define CONFIG_CMD_DISP
	#endif
#endif

#ifdef PHL_PLATFORM_LINUX
#ifdef CONFIG_PCI_HCI
/* Driver needs to handle cache-memory coherence for DMA */
#if defined(CONFIG_DMA_NONCOHERENT) || defined(CONFIG_DMA_MAYBE_COHERENT)
#define PHL_DMA_NONCOHERENT
#endif
/* DMA is IOMMU. map/unmap must be paired, including size */
#if defined(CONFIG_IOMMU_DMA)
#define PHL_DMA_IOMMU
#endif
#endif /* CONFIG_PCI_HCI */
/* DMA address is 64-bit */
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
#define PHL_DMA_ADDR_64
#endif
	/* comment out cfg temporarily */
	/*
	#define CONFIG_FSM

	#ifndef CONFIG_FSM
		#define CONFIG_CMD_DISP
	#endif
	*/
#endif

/******************* Feature flags **************************/
#define MAC_PHL_TEMP_CROSS_DEFINE

#ifdef CONFIG_PHL_TEST_SUITE
#define CONFIG_PHL_TEST_MP
#define CONFIG_PHL_TEST_VERIFY
#endif

#ifdef CONFIG_CORE_SYNC_INTERRUPT
#define CONFIG_SYNC_INTERRUPT
#endif

#ifdef CONFIG_WOW
#define CONFIG_WOWLAN
/* #define RTW_WKARD_WOW_SKIP_AOAC_RPT */
/* #define RTW_WKARD_WOW_SKIP_WOW_CAM_CONFIG */
#define RTW_WKARD_WOW_L2_PWR
#define DBG_RST_BDRAM_TIME
#endif

/*CONFIG_IFACE_NUMBER*/
#ifdef CONFIG_IFACE_NUMBER
#define MAX_WIFI_ROLE_NUMBER CONFIG_IFACE_NUMBER
#else
#define MAX_WIFI_ROLE_NUMBER 5
#endif

#if defined(CONFIG_RTW_SUPPORT_MBSSID_VAP) && defined(CONFIG_LIMITED_VAP_NUM)
#define MAX_MBSSID_NUMBER	(CONFIG_LIMITED_VAP_NUM)
#else
#define MAX_MBSSID_NUMBER	(0)
#endif /* CONFIG_RTW_SUPPORT_MBSSID_VAP */

#ifdef CONFIG_CONCURRENT_MODE
#define CONFIG_MR_SUPPORT
#endif

#ifdef CONFIG_REUSED_FWDL_BUF
	#define CONFIG_PHL_REUSED_FWDL_BUF
#endif

#ifdef CONFIG_MR_SUPPORT
#define CONFIG_SCC_SUPPORT
#ifndef PHL_FEATURE_AP
#define CONFIG_MCC_SUPPORT
#endif
#ifdef CONFIG_MCC_SUPPORT
#define MCC_ROLE_NUM 2
#define RTW_WKARD_GO_BT_TS_ADJUST_VIA_NOA
#define RTW_WKARD_HALRF_MCC
#endif /*CONFIG_MCC_SUPPORT*/
/*#define CONFIG_DBCC_SUPPORT*/

#define DBG_PHL_CHAN
#define DBG_PHL_MR
#define PHL_MR_PROC_CMD
#define DBG_CHCTX_RMAP
#endif

#define DBG_PHL_STAINFO
#define PHL_MAX_STA_NUM 128

/**** CONFIG_CMD_DISP ***/
#ifdef DISABLE_CMD_DISPR
#undef CONFIG_CMD_DISP
#endif

#ifdef CONFIG_CMD_DISP
/* enable SOLO mode define to create seperated background thread per dispatcher,
 * otherwise, all dispatcher would share single background thread, which is in share mode.
*/
/*#define CONFIG_CMD_DISP_SOLO_MODE*/

/* Enable Self-Defined Sequence feature for sender to rearrange dispatch order,
 * Since this is not a mandatory feature and would have addiional memory cost (arround 2200 Bytes)
 * Disable by default.
*/
/*#define CONFIG_CMD_DISP_SUPPORT_CUSTOM_SEQ*/

#define CONFIG_PHL_CMD_SCAN

#ifdef CONFIG_CMD_SER
#define CONFIG_PHL_CMD_SER
#endif

#define CONFIG_SND_CMD

#define CONFIG_PHL_CMD_BF

#endif /**** CONFIG_CMD_DISP ***/

#ifdef WKARD_PON_PLATFORM
#define CONFIG_GEN_GIT_INFO 0
#else
#define CONFIG_GEN_GIT_INFO 1
#endif
/*#define CONFIG_NEW_HALMAC_INTERFACE*/

#ifdef CONFIG_BTC
#define CONFIG_BTCOEX
#endif

#ifdef CONFIG_USB_TX_PADDING_CHK
#define CONFIG_PHL_USB_TX_PADDING_CHK
#endif

#ifdef CONFIG_USB_TX_AGGREGATION
#define CONFIG_PHL_USB_TX_AGGREGATION
#endif

#ifdef CONFIG_USB_RX_AGGREGATION
#define CONFIG_PHL_USB_RX_AGGREGATION
#endif

#if CONFIG_DFS
#define CONFIG_PHL_DFS_SWITCH_CH_WITH_CSA
#ifdef CONFIG_DFS_MASTER
#define CONFIG_PHL_DFS
#endif
#endif

#ifdef CONFIG_WPP
#define CONFIG_PHL_WPP
#endif

#ifdef CONFIG_TCP_CSUM_OFFLOAD_RX
#define CONFIG_PHL_CSUM_OFFLOAD_RX
#endif

#ifdef CONFIG_RX_PSTS_PER_PKT
#define CONFIG_PHL_RX_PSTS_PER_PKT
#define RTW_WKARD_DISABLE_PSTS_PER_PKT_DATA
#endif

#ifdef CONFIG_SDIO_RX_NETBUF_ALLOC_IN_PHL
#define CONFIG_PHL_SDIO_RX_NETBUF_ALLOC_IN_PHL
#endif

#ifdef CONFIG_SDIO_READ_RXFF_IN_INT
#define CONFIG_PHL_SDIO_READ_RXFF_IN_INT
#endif

#ifdef CONFIG_ECSA
#define CONFIG_PHL_ECSA
#endif

#if defined(CONFIG_TWT) || defined(CONFIG_RTW_TWT)
#define CONFIG_PHL_TWT
#endif

#ifdef CONFIG_RA_TXSTS_DBG
#define CONFIG_PHL_RA_TXSTS_DBG
#endif

#ifdef CONFIG_RELEASE_RPT
#define CONFIG_PHL_RELEASE_RPT_ENABLE
#endif

#ifdef CONFIG_P2PPS
#define CONFIG_PHL_P2PPS
#endif

#ifdef CONFIG_PCI_HCI

#ifdef CONFIG_PCIE_TRX_MIT
#define PCIE_TRX_MIT_EN
#endif

#ifndef PHL_RX_HEADROOM
#define PHL_RX_HEADROOM	(0)
#endif

#endif /* CONFIG_PCI_HCI */

#ifdef CONFIG_THERMAL_PROTECT
#define CONFIG_PHL_THERMAL_PROTECT
#endif

#ifdef CONFIG_RX_BATCH_IND
#define PHL_RX_BATCH_IND
#endif

#ifdef CONFIG_TDLS
#define CONFIG_PHL_TDLS
#endif

#define PHL_DIS_CTRL_FRAME_TO_HOST
#ifdef PHL_DIS_CTRL_FRAME_TO_HOST
/* Disable ALL CTRL frame report to host */
#define PHL_DIS_CTRL_FRAME_TO_HOST_ALL		0
/* Disable ACK + BA + RTS + CTS frame report to host */
#define PHL_DIS_CTRL_FRAME_TO_HOST_POLICY_1	1
#define PHL_DIS_CTRL_FRAME_TO_HOST_POLICY	PHL_DIS_CTRL_FRAME_TO_HOST_POLICY_1
#endif

#ifdef CONFIG_RTW_DRV_HAS_NVM
/* Driver has its own NVM to store MP calibration values
 * and system configuration instead of efuse. */
#define CONFIG_PHL_DRV_HAS_NVM
#endif /* CONFIG_RTW_DRV_HAS_NVM */

#ifdef CONFIG_PCI_TRX_RES_DBG
#define CONFIG_PHL_PCI_TRX_RES_DBG
#endif

/******************* WKARD flags **************************/
#define RTW_WKARD_P2PPS_REFINE
#define RTW_WKARD_P2PPS_SINGLE_NOA
#define RTW_WKARD_P2PPS_NOA_MCC

#ifdef PHL_PLATFORM_LINUX
#define RTW_WKARD_RF_CR_DUMP
#define RTW_WKARD_LINUX_CMD_WKARD
#endif

#define RTW_WKARD_PHY_CAP

#define RTW_WKARD_LAMODE

#define RTW_WKARD_TXSC

#define RTW_WKARD_BB_C2H

/* Workaround for doing hal reset in changing MP mode will lost the mac entry */
#define RTW_WKARD_MP_MODE_CHANGE

/*
 * One workaround of EFUSE operation
 *  1. Dump EFUSE with FW fail
 */
#define RTW_WKARD_EFUSE_OPERATION

#define RTW_WKARD_STA_BCN_INTERVAL

#define RTW_WKARD_SER_L1_EXPIRE

#ifdef CONFIG_USB_HCI
#define RTW_WKARD_SER_USB_POLLING_EVENT
#endif

/* #define RTW_WKARD_SER_USB_DISABLE_L1_RCVY_FLOW */

#define RTW_WKARD_BTC_RFETYPE

#define RTW_WKARD_TXBD_UPD_LMT 	/* 8852AE/8852BE txbd index update limitation */

#ifdef CONFIG_WPA3_SUITEB_SUPPORT
#define RTW_WKARD_HW_MGNT_GCMP_256_DISABLE
#endif

/* Workaround for cmac table config
 * - Default is disabled until halbb is ready
 * - This workaround will be removed once fw handles this cfg
 */
/*#define RTW_WKARD_DEF_CMACTBL_CFG*/

/* Workaround for efuse read hidden report
 * - Default is disabled until halmac is ready
 */

#define RTW_WKARD_PRELOAD_TRX_RESET

/* Workaround for cmac table config
 * - This workaround will be removed once fw handles this cfg
 */
#define RTW_WKARD_DEF_CMACTBL_CFG

#define RTW_WKARD_USB_TXAGG_BULK_END_WD
#ifdef CONFIG_HOMOLOGATION
#define CONFIG_PHL_HOMOLOGATION
#endif

#ifdef RTW_WKARD_TX_DISABLE_BFEE
#define RTW_WKARD_DYNAMIC_BFEE_CAP
#endif

#ifdef RTW_WKARD_NTFY_MEDIA_STS
#define RTW_WKARD_PHL_NTFY_MEDIA_STS
#endif

#ifdef RTW_WKARD_PHY_INFO_NTFY
#define CONFIG_PHY_INFO_NTFY
#endif

/* LPS should disable other role */
#define RTW_WKARD_LPS_ROLE_CONFIG

#ifdef PHL_PLATFORM_WINDOWS
#define RTW_WKARD_WIN_TRX_BALANCE
#define RTW_WKARD_DYNAMIC_LTR
#endif

#ifdef PHL_PLATFORM_WINDOWS
#define CONFIG_WOW_WITH_SER
#endif

/*
 * Workaround for phl_mr_offch_hdl sleep after issue null data,
 * - This workaround will be removed once tx report is ready
 */
#define RTW_WKARD_ISSUE_NULL_SLEEP_PROTECTION

/*
 * Workaround for blocking scan during MCC
 */
#ifdef CONFIG_MCC_SUPPORT
#define RTW_WKARD_SKIP_SCAN_IN_MCC
#endif

#ifdef RTW_WKARD_LPS_IQK_TWICE
#define RTW_WKARD_PHL_LPS_IQK_TWICE
#endif

#ifdef RTW_WKARD_FSM_SCAN_PASSIVE_TO_ACTIVE
#define RTW_WKARD_PHL_FSM_SCAN_PASSIVE_TO_ACTIVE
#endif

#define RTW_WKARD_BUSCAP_IN_HALSPEC

#define RTW_WKARD_IBSS_SNIFFER_MODE


#define RTW_WKARD_SINGLE_PATH_RSSI

/* #define CONFIG_6GHZ */

#define RTW_WKARD_HIQ_STUCK

#define RTW_WKARD_BFEE_DISABLE_NG16

#define RTW_WKARD_HW_WMM_ALLOCATE

#ifdef RTW_WKARD_BFEE_AID
#define RTW_WKARD_BFEE_SET_AID
#endif

#ifdef RTW_WKARD_TRIGGER_FRAME_PARSER
#define RTW_WKARD_RX_FLTR_HE_TF
#endif

#ifdef RTW_WKARD_DISABLE_2G40M_ULOFDMA
#define RTW_WKARD_BB_DISABLE_STA_2G40M_ULOFDMA
#endif

#ifdef PHL_FEATURE_AP
/* decrease 2.4G beacon err rate */
#define RTW_WKARD_BCN_ERR

#ifdef RTW_WKARD_BCN_ERR
#define WKARD_BCN_ERR_HOLD_TIME (16 * 1000 / 32)
#define WKARD_BCN_ERR_ECW_MAX	(4)
#define WKARD_BCN_ERR_ECW_MIN	(3)
#define WKARD_BCN_ERR_AIFS	(0x15)
#endif /* RTW_WKARD_BCN_ERR*/

#ifdef CONFIG_PCI_HCI
/* MAC/BB T/RX hang recovery */
#ifndef CONFIG_RTW_HW_TRX_WATCHDOG
#define CONFIG_RTW_HW_TRX_WATCHDOG		(1)  /* 0 to disable. Set non-zero to enable by default */
#endif
#ifndef CONFIG_RTW_HW_TRX_WATCHDOG_LIMIT
#define CONFIG_RTW_HW_TRX_WATCHDOG_LIMIT 	(10) /* Times of watchdog check failure x2 seconds */
#endif

#endif /* CONFIG_PCI_HCI */

/* Retry HAL start to recover firmware download failure */
#define PHL_RETRY_MAC_INIT (2)

#endif /* PHL_FEATURE_AP */

#endif /*_PHL_CONFIG_H_*/
