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
 * $Revision: 20569 $
 * $Date: 2011-08-05 17:46:23 +0800 (Fri, 05 Aug 2011) $
 *
 * Purpose : DRV APIs definition.
 *
 * Feature : SMI relative API
 *
 */
#ifndef __DRV_EXT_SMI_H__
#define __DRV_EXT_SMI_H__

/*
* Include Files
*/ 
#include <common/rt_error.h>
     
/*
* Symbol Definition
*/

/*
 * Data Declaration
 */
 
/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */ 

/* Function Name:
 *      drv_extSmi_dev_init
 * Description:
 *      ext SMI init function
 * Input:
 *      portSCK - SCK port id
 *      pinSCK  - SCK pin
 *      portSDA - SDA port id
 *      pinSDA  - SDA pin 
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
extern int32 drv_extSmi_dev_init(uint32 unit, uint32 periferal, uint32 phyAddrSCK, uint32 phyAddrSDA, uint32 gpioIdSCK, uint32 gpioIdSDA);

extern int32 drv_extSmi_read(uint32 unit, uint32 periferal, uint32 mAddrs, uint32 *pRdata);

/* Function Name:
 *      drv_smi_write
 * Description:
 *      SMI write wrapper function
 * Input:
 *      mAddrs  - address
 *      wData   - data write 
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
extern int32 drv_extSmi_write(uint32 unit, uint32 periferal, uint32 mAddrs, uint32 wData);

#endif /* __DRV_EXT_SMI_H__ */

