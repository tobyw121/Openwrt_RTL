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
 * $Revision: 33423 $
 * $Date: 2012-10-15 17:13:32 +0800 (Mon, 15 Oct 2012) $
 *
 * Purpose : Realtek Switch SDK Core Module In User Space.
 *
 * Feature : Realtek Switch SDK Core Module In User Space.
 *
 */

#ifndef __RTCORE_DRV_USR_H__
#define __RTCORE_DRV_USR_H__
/*
 * Include Files
 */
#include <common/type.h>
#include <rtcore/rtcore.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */
/* Function Name:
 *      rtcore_usr_init
 * Description:
 *      Initialize RTCORE user layer module.
 * Input:
 *      NONE
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32 
rtcore_usr_init(void);


/* Function Name:
 *      rtcore_usr_intr_attach
 * Description:
 *      Connect interrupt with rtcore module
 * Input:
 *      unit          - unit id
 *      fHandler      - The interrupt handler function that is going to be attached.
 *                      It is required to be called with 'pHandler_param' argument.
 *      pHandler_param- The argument passed to 'fHandler' interrupt handler when interrupt happen.
 *      type          - The specific interrupt type that we wants to connect with.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      None
 */
extern int32
rtcore_usr_intr_attach(uint32 unit, void (*fHandler)(void*), void *pHandler_param, rtcore_intr_type_t type);

/* Function Name:
 *      rtcore_usr_intr_detach
 * Description:
 *      Disconnect interrupt with rtcore module
 * Input:
 *      unit - unit id
 *      type - The specific interrupt type that we wants to disconnect.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      None
 */
extern int32
rtcore_usr_intr_detach(uint32 unit, rtcore_intr_type_t type);

#endif /* __RTCORE_DRV_USR_H__ */

