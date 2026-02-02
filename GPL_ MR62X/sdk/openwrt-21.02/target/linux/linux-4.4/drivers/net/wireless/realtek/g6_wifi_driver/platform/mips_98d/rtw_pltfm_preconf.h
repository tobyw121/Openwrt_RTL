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

#ifndef PLATFORM_MIPS_98D_RTW_PLTFM_PRECONF_H_
#define PLATFORM_MIPS_98D_RTW_PLTFM_PRECONF_H_

#include "../rtk_ap/preconf_common.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
/* Use DMA handle function in arch/mips on usdk with kernel 5.10*/
#define CONFIG_RTW_VM_CACHE_HANDLING
#endif

#ifdef __MIPSEB__
#define HAL_TO_NONCACHE_ADDR(addr)      (KSEG1ADDR(addr))
#else
#define HAL_TO_NONCACHE_ADDR(addr)      ((void *)(((size_t)(addr))|0x20000000))
#endif

/* Enable FleetConntrack packet forwarding to speed up wlan to eth */
#define CONFIG_RTW_FC_FASTFWD

/* MBSSID/VAP support */
#ifndef CONFIG_RTW_SUPPORT_MBSSID_VAP
#define CONFIG_RTW_SUPPORT_MBSSID_VAP
#endif

/* Enabled RX interrupt mitigation */
#ifndef CONFIG_PCIE_TRX_MIT
#define CONFIG_PCIE_TRX_MIT
#endif

/* Enable system watchdog kick */
#ifndef CONFIG_RTW_WATCHDOG_KICK
#define CONFIG_RTW_WATCHDOG_KICK 0x03FF
#endif

/* To behave as rtl8192cd driver for Hwawei:
 * Interface created initially and remain alive event del/add
 * virtual interface is called */
/* #define CONFIG_RTW_PERSIST_IF */

/* Support hardware RX A-MSDU cut */
#if (defined(CONFIG_RTL8852CE) \
     || defined(CONFIG_RTL8192XB) \
     || defined(CONFIG_RTL8832BR))
#ifndef CONFIG_RTW_HW_RX_AMSDU_CUT
#define CONFIG_RTW_HW_RX_AMSDU_CUT
// #define CONFIG_RTW_ENABLE_HW_RX_AMSDU_CUT
#define CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT
// #define CONFIG_RTW_HW_RX_AMSDU_CUT_NO_HDR_COV
#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
#endif /* 52CE/32BR/92XB */

/* Two RX buffer share one sk_buff's buffer */
#ifndef CONFIG_RTW_LINK_PHL_MASTER
#ifndef CONFIG_RTW_RX_BUF_SHARING
#define CONFIG_RTW_RX_BUF_SHARING
#endif
#endif /* CONFIG_RTW_LINK_PHL_MASTER */

#if defined(CONFIG_LOAD_PHY_PARA_FROM_FILE) && !defined(CONFIG_DYNAMIC_PHY_PARA_MEM)
#define CONFIG_DYNAMIC_PHY_PARA_MEM
#endif

/* Enable proc debug commands */
#ifndef CONFIG_PHL_TEST_SUITE
#define CONFIG_PHL_TEST_SUITE
#endif

/* Dump CAMs for debugging */
#ifndef CONFIG_RTW_DEBUG_CAM
#define CONFIG_RTW_DEBUG_CAM
#endif

/* Driver shows beacon TX report */
#ifndef CONFIG_RTW_DEBUG_TX_RPT
#define CONFIG_RTW_DEBUG_TX_RPT
#endif

/* Driver shows beacon TX reports collected by FW */
#ifndef CONFIG_RTW_DEBUG_BCN_TX
#define CONFIG_RTW_DEBUG_BCN_TX		1
#define CONFIG_RTW_DEBUG_BCN_STATS	1
#endif

/* Record max/min RX packets */
#ifndef CONFIG_RTW_DEBUG_RX_SZ
#define CONFIG_RTW_DEBUG_RX_SZ
#endif /* CONFIG_RTW_DEBUG_RX_SZ */

/* Debug WP tag */
#ifndef CONFIG_RTW_DEBUG_WP_TAG
#define CONFIG_RTW_DEBUG_WP_TAG
#endif /* CONFIG_RTW_DEBUG_WP_TAG */

/* Debug RX buffer cache handling */
#ifndef CONFIG_RTW_DEBUG_RX_CACHE
#define CONFIG_RTW_DEBUG_RX_CACHE
#endif /* CONFIG_RTW_DEBUG_RX_CACHE */

/* Debug struct submit_ctx dynamic alloc/free */
#define CONFIG_RTW_DEBUG_SCTX_ALLOC

/* Disable CFO tracking */
#ifndef DRV_BB_CFO_TRK_DISABLE
/* #define DRV_BB_CFO_TRK_DISABLE */
#endif
#if (defined(CONFIG_SHARE_XSTAL) || defined(CONFIG_AX_SHARE_XTAL))
#define DRV_BB_CFO_TRK_DISABLE_BY_SHARE_XTAL
#endif

/* Wlan on Realtek AP platform is designed for single band */
#ifndef CONFIG_RTW_DEV_IS_SINGLE_BAND
#define CONFIG_RTW_DEV_IS_SINGLE_BAND
#endif

#define RTW_FLASH_98D
#define WKARD_DECIMAL_INPUT
/* Limit A-MPDU number on 2G band */
#define RTW_WKRND_2G_AMPDU_LIMIT	96

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
#define RTW_CUSTOM_PARA_DIR /*get RFE para sub dir from menuconfig*/
#endif /*CONFIG_LOAD_PHY_PARA_FROM_FILE*/

/*Dynamic Adaptivity Control*/
//#define CONFIG_ADPTVTY_CONTROL

/* Debug flags */
/* #define DBG_XMIT_BUF_EXT */

#endif /* PLATFORM_MIPS_98D_RTW_PLTFM_PRECONF_H_ */
