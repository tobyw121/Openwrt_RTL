/******************************************************************************
 *
 * Copyright(c) 2021 Realtek Corporation.
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

/* Common settings for RTK Wifi6 AP/router solutions. */

#ifndef _RTK_AP_PRECONF_COMMON_H_
#define _RTK_AP_PRECONF_COMMON_H_

#define CONFIG_PHL_USE_KMEM_ALLOC_BY_PAGE_SIZE
#define CONFIG_RTW_LATE_INT_ENABLE

/* CONFIG_RTW_MULTI_DEV_MULTI_BAND replaced WKARD_DBCC
 * distinguish from single-chip DBCC supported in 52A. */
#define CONFIG_RTW_MULTI_DEV_MULTI_BAND

#ifdef CONFIG_RTW_MULTI_DEV_MULTI_BAND
#define WKARD_DBCC /* Keep for linking with old PHL */
#endif

/* Check register access against MAC status to prevent from
 * access hang */
#define CONFIG_MAC_REG_RW_CHK

/* enable xmit frame callback when tx recycle */
#define CONFIG_XMIT_MGMT_ACK

/* Extension of OS handler to support CPU binding for load balance */
#ifndef CONFIG_RTW_OS_HANDLER_EXT
#define CONFIG_RTW_OS_HANDLER_EXT
#endif /* CONFIG_RTW_OS_HANDLER_EXT */

#endif /* _RTK_AP_PRECONF_COMMON_H_ */
