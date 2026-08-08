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
 * $Revision: 30053 $
 * $Date: 2012-06-19 14:12:07 +0800 (Tue, 19 Jun 2012) $
 *
 * Purpose : Definition those public LED APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) LED
 *
 */

#ifndef __DAL_ESW_LED_H__
#define __DAL_ESW_LED_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk/port.h>
#include <rtk/led.h>


/*
 * Data Declaration
 */

/*
 * Macro Definition
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      dal_esw_led_init
 * Description:
 *      Initialize port module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
extern int32
dal_esw_led_init(uint32 unit);

/* Function Name:
 *      dal_esw_led_portEnable_get
 * Description:
 *      Get led status on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the led status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_led_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_led_portEnable_set
 * Description:
 *      Set led status on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - led status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_led_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_led_sysEnable_get
 * Description:
 *      Get led status on specified type.
 * Input:
 *      unit    - unit id
 *      type    - system led type
 * Output:
 *      pEnable - pointer to the led status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_led_sysEnable_get(uint32 unit, rtk_led_type_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_led_sysEnable_set
 * Description:
 *      Set led status on specified type.
 * Input:
 *      unit   - unit id
 *      type   - system led type
 *      enable - led status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_led_sysEnable_set(uint32 unit, rtk_led_type_t type, rtk_enable_t enable);

#if defined(CONFIG_SDK_WA_RTL8231_RESET)
/* Function Name:
 *      dal_esw_led_8231Reset_workaround
 * Description:
 *      Recovery the external GPIO status in the specified device of the unit
 * Input:
 *      unit   - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
dal_esw_led_8231Reset_workaround(uint32 unit);

#endif
#endif /* __DAL_ESW_LED_H__ */
