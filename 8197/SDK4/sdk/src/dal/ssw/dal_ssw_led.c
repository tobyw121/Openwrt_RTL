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

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <osal/sem.h>
#include <drv/gpio/gpio.h>
#include <hal/mac/reg.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/common/halctrl.h>
#include <dal/ssw/dal_ssw_led.h>
#include <rtk/port.h>
#include <rtk/led.h>

/* 
 * Symbol Definition 
 */


/* 
 * Data Declaration 
 */
static uint32       led_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t led_sem[RTK_MAX_NUM_OF_UNIT];


/*
 * Macro Definition
 */
/* led semaphore handling */
#define LED_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(led_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_LED), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define LED_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(led_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_LED), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* 
 * Function Declaration 
 */

/* Function Name:
 *      dal_ssw_led_init
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
int32
dal_ssw_led_init(uint32 unit)
{
    led_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    led_sem[unit] = osal_sem_mutex_create();
    if (0 == led_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_LED), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    /* Initial System LED GPIO of 8389, F2 for system LED, F3 for Alarm LED */
    drv_gpio_init(GPIO_ID(GPIO_PORT_F, 2), GPIO_CTRLFUNC_NORMAL, GPIO_DIR_OUT, GPIO_INT_DISABLE);
    drv_gpio_init(GPIO_ID(GPIO_PORT_F, 3), GPIO_CTRLFUNC_NORMAL, GPIO_DIR_OUT, GPIO_INT_DISABLE);
    
    /* set init flag to complete init */
    led_init[unit] = INIT_COMPLETED;    

    return RT_ERR_OK;
}/* end of dal_ssw_led_init */

/* Function Name:
 *      dal_ssw_led_portEnable_get
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
int32
dal_ssw_led_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32  value = 0;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(led_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    LED_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_PER_PORT_LED_ENABLE_CONTROLr, &value)) != RT_ERR_OK)
    {
        LED_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_LED), "");
        return ret;
    }
    
    *pEnable = (value >> port) & 1;

    LED_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_led_portEnable_get */

/* Function Name:
 *      dal_ssw_led_portEnable_set
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
int32
dal_ssw_led_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  value = 0;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(led_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    LED_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_PER_PORT_LED_ENABLE_CONTROLr, &value)) != RT_ERR_OK)
    {
        LED_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_LED), "");
        return ret;
    }
    if (ENABLED == enable)
        value |= (1 << port);
    else
        value &= ~(1 << port);
    if ((ret = reg_write(unit, SSW_PER_PORT_LED_ENABLE_CONTROLr, &value)) != RT_ERR_OK)
    {
        LED_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_LED), "");
        return ret;
    }

    LED_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_led_portEnable_set */

/* Function Name:
 *      dal_ssw_led_sysEnable_get
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
int32
dal_ssw_led_sysEnable_get(uint32 unit, rtk_led_type_t type, rtk_enable_t *pEnable)
{
    uint32  action = 0;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED), "unit=%d, type=%d", 
           unit, type);
    
    /* check Init status */
    RT_INIT_CHK(led_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(type >= RTK_LED_TYPE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(NULL == pEnable, RT_ERR_NULL_POINTER);

    LED_SEM_LOCK(unit);

    switch (type)
    {
        case LED_TYPE_SYS:
            ret = drv_gpio_dataBit_get(GPIO_ID(GPIO_PORT_F, 2), &action);
            break;
        case LED_TYPE_ALARM:
            ret = drv_gpio_dataBit_get(GPIO_ID(GPIO_PORT_F, 3), &action);
            break;
        case RTK_LED_TYPE_END:
        default:
            LED_SEM_UNLOCK(unit);
            return RT_ERR_FAILED;
    }
    
    if (0 == action) /* low active */
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    LED_SEM_UNLOCK(unit);
    return ret;
} /* end of dal_ssw_led_sysEnable_get */

/* Function Name:
 *      dal_ssw_led_sysEnable_set
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
int32
dal_ssw_led_sysEnable_set(uint32 unit, rtk_led_type_t type, rtk_enable_t enable)
{
    uint32  action = 0;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED), "unit=%d, type=%d, enable=%d", 
           unit, type, enable); 
    
    /* check Init status */
    RT_INIT_CHK(led_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(type >= RTK_LED_TYPE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);

    LED_SEM_LOCK(unit);

    if (ENABLED == enable)
        action = 0; /* low active */
    else
        action = 1;

    switch (type)
    {
        case LED_TYPE_SYS:
            ret = drv_gpio_dataBit_set(GPIO_ID(GPIO_PORT_F, 2), action);
            break;
        case LED_TYPE_ALARM:
            ret = drv_gpio_dataBit_set(GPIO_ID(GPIO_PORT_F, 3), action);
            break;
        case RTK_LED_TYPE_END:
        default:
            LED_SEM_UNLOCK(unit);
            return RT_ERR_FAILED;
    }
    
    LED_SEM_UNLOCK(unit);
    return ret;
} /* end of dal_ssw_led_sysEnable_set */
