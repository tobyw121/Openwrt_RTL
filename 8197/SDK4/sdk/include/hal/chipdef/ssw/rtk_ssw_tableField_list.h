/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 8999 $
 * $Date: 2010-04-12 15:42:36 +0800 (Mon, 12 Apr 2010) $
 *
 * Purpose : chip table field declaration in the SDK.
 *
 * Feature : chip table field declaration
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>

/*
 * Data Declaration
 */
#if defined(CONFIG_SDK_RTL8389)
extern rtk_tableField_t RTL8389_L2CAM_TABLE_FIELDS[]; 
#endif
#if defined(CONFIG_SDK_RTL8389)
extern rtk_tableField_t RTL8389_L2_TABLE_FIELDS[]; 
#endif
#if defined(CONFIG_SDK_RTL8389)
extern rtk_tableField_t RTL8389_LOG_TABLE_FIELDS[]; 
#endif
#if defined(CONFIG_SDK_RTL8389)
extern rtk_tableField_t RTL8389_MULTICAST_INDEX_TABLE_FIELDS[]; 
#endif
#if defined(CONFIG_SDK_RTL8389)
extern rtk_tableField_t RTL8389_PIE89_TABLE_FIELDS[]; 
#endif
#if defined(CONFIG_SDK_RTL8389)
extern rtk_tableField_t RTL8389_SVLAN_TABLE_FIELDS[]; 
#endif
#if defined(CONFIG_SDK_RTL8389)
extern rtk_tableField_t RTL8389_VLAN_TABLE_FIELDS[]; 
#endif

