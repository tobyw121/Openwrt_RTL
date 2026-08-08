/*
 * Copyright (C) 2009 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: 29336 $
 * $Date: 2012-05-28 19:55:01 +0800 (Mon, 28 May 2012) $
 *
 * Purpose : sys symbol and data type definition in the SDK.
 *
 * Feature : sys symbol and data type definition
 *
 */

#ifndef __DRV_SWCORE_SYS_H__
#define __DRV_SWCORE_SYS_H__

/*
 * Include Files
 */
#include <common/error.h>

/*
 * Symbol Definition
 */
#define SYS_UNIT_ID         0    /* unit 0 */
 
typedef enum sys_mac_chip_e
{
#if defined(CONFIG_SDK_RTL8389) 
    SYS_MAC_RTL8389,
#endif
#if defined(CONFIG_SDK_RTL8328) 
    SYS_MAC_RTL8328,    
#endif    
#if defined(CONFIG_SDK_RTL8390) 
    SYS_MAC_RTL8390,    
#endif    
#if defined(CONFIG_SDK_RTL8380) 
    SYS_MAC_RTL8380,    
#endif    
    SYS_MAC_END            
} sys_mac_chip_t;

typedef struct sys_mac_reg_s
{
    uint32 mac_high;
    uint32 mac_low;
    uint32 low_offset;
    uint32 low_mask; 
        
} sys_mac_reg_t;

/*
 * Data Type Definition
 */

 
/*
 * Function Declaration
 */

/* Function Name:
 *      drv_swcore_sysMac_get
 * Description:
 *      Get mac address of the system
 * Input:
 *      None
 * Output:
 *      pMacAddr - mac address
 * Return:
 *      RT_ERR_OK  
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern int32
drv_swcore_sysMac_get(uint8 *pMacAddr);

/* Function Name:
 *      drv_swcore_sysMac_set
 * Description:
 *      Set mac address of the system
 * Input:
 *      pMacAddr - mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern int32
drv_swcore_sysMac_set(uint8 *pMacAddr);

#endif  /* __DRV_SWCORE_SYS_H__ */

