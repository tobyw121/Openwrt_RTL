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
#include <drv/gpio/ext_gpio.h>
#include <hal/mac/reg.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/common/halctrl.h>
#include <dal/esw/dal_esw_led.h>
#include <rtk/port.h>
#include <rtk/led.h>

/* 
 * Symbol Definition 
 */
#define CHECKBUSY_TIMES (3000)


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

#define LED_BUSY_WAIT_LOOP(unit, REG, MASK)\
{\
    uint32 i;\
    uint32 regVal;\
    for (i = 0; i < CHECKBUSY_TIMES; i++)\
    {\
        if (reg_read(unit, REG, &regVal) != RT_ERR_OK)\
        {\
            LED_SEM_UNLOCK(unit);\
            return RT_ERR_FAILED;\
        }\
        if (0 == (regVal & MASK))\
        {\
            break;\
        }\
    }\
    if (CHECKBUSY_TIMES == i)\
    {\
        LED_SEM_UNLOCK(unit);\
        return RT_ERR_FAILED;\
    }\
}


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
int32
dal_esw_led_init(uint32 unit)
{
    uint32  led_scan_mode, value;
    int32   ret;
    led_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    led_sem[unit] = osal_sem_mutex_create();
    if (0 == led_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_LED), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    /* set init flag to complete init */
    led_init[unit] = INIT_COMPLETED;    

    /* get led mode from chip strip ping */
    if ((ret = reg_field_read(unit, ESW_LED_SYSTEM_CONTROL0r, ESW_SEL_LED_MODEf, &led_scan_mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_LED), "");
        return ret;
    }

    if (0 == led_scan_mode)
    {   /* Get the device's led mode from register and if is serial mode, enable 
         * software control LED in P26_LED0 for SYS and P26_LED0 for ALARM
         */
        value = 1;
        if ((ret = reg_array_field_write(unit, ESW_LED_SYSTEM_CONTROL1r, 26, 
             REG_ARRAY_INDEX_NONE, ESW_PORT_LED0_SOFT_CONFIGf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }
        value = 1;
        if ((ret = reg_array_field_write(unit, ESW_LED_SYSTEM_CONTROL1r, 27, 
             REG_ARRAY_INDEX_NONE, ESW_PORT_LED0_SOFT_CONFIGf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }
    }

    /*Set Port 24~27 LED to copper+fiber*/
    value = 0;
    if ((ret = reg_field_write(unit, ESW_LED_SYSTEM_CONTROL0r, ESW_EN_P24_27_LED_COPPER_ONLYf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_LED), "");
        return ret;
    }

#if defined(CONFIG_SDK_WA_RTL8231_RESET)
    {
        uint32  led_ic_addr;
        drv_extGpio_devConf_t data;
    
        if ((ret = reg_field_read(unit, ESW_LED_SYSTEM_CONTROL0r, ESW_LED_IC_ADDRf, &led_ic_addr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }

        if (led_scan_mode)
        {
            data.access_mode = EXT_GPIO_ACCESS_MODE_I2C;
            data.address =  led_ic_addr; /* EXT_GPIO_DEV_ID7 */
            data.page = 30;
            drv_extGpio_dev_init(unit, 0, &data);
            RT_LOG(LOG_DEBUG, MOD_DAL, "workaround init 8231 dev 0 ... ");
        }
    }
#endif

    return RT_ERR_OK;
}/* end of dal_esw_led_init */

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
int32
dal_esw_led_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
    if ((ret = reg_read(unit, ESW_PER_PORT_LED_ENABLE_CONTROLr, &value)) != RT_ERR_OK)
    {
        LED_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_LED), "");
        return ret;
    }
    
    *pEnable = (value >> port) & 1;

    LED_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_led_portEnable_get */

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
int32
dal_esw_led_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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
    if ((ret = reg_read(unit, ESW_PER_PORT_LED_ENABLE_CONTROLr, &value)) != RT_ERR_OK)
    {
        LED_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_LED), "");
        return ret;
    }
    if (ENABLED == enable)
        value |= (1 << port);
    else
        value &= ~(1 << port);
    if ((ret = reg_write(unit, ESW_PER_PORT_LED_ENABLE_CONTROLr, &value)) != RT_ERR_OK)
    {
        LED_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_LED), "");
        return ret;
    }

    LED_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_led_portEnable_set */

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
int32
dal_esw_led_sysEnable_get(uint32 unit, rtk_led_type_t type, rtk_enable_t *pEnable)
{
    uint32  value = 0, bit_offset, field_id;
    uint32  led_scan_mode;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED), "unit=%d, type=%d", 
           unit, type); 
    
    /* check Init status */
    RT_INIT_CHK(led_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(type >= RTK_LED_TYPE_END, RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(unit, ESW_LED_SYSTEM_CONTROL0r, ESW_SEL_LED_MODEf, &led_scan_mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_LED), "");
        return ret;
    }

    /* scan mode is supported the sys/alarm led from external 8231 */
    if (led_scan_mode)
    {
        LED_SEM_LOCK(unit);
    
        switch (type)
        {
            case LED_TYPE_SYS:
                bit_offset = 14;
                break;
            case LED_TYPE_ALARM:
                bit_offset = 13;
                break;
            case RTK_LED_TYPE_END:
            default:
                LED_SEM_UNLOCK(unit);
                return RT_ERR_FAILED;
        }
        
        /* Indirect read external gpio address 4 */
        value = 0x1f04;
        if ((ret = reg_write(unit, ESW_TRIGGER_EXTERNAL_GPIOr, &value)) != RT_ERR_OK)
        {
            LED_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }
    
        LED_BUSY_WAIT_LOOP(unit, ESW_TRIGGER_EXTERNAL_GPIOr, 0x1000);
    
        if ((ret = reg_read(unit, ESW_EXTERNAL_DATA_READr, &value)) != RT_ERR_OK)
        {
            LED_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }
        
        *pEnable = (value >> bit_offset) & 1;
    
        LED_SEM_UNLOCK(unit);
    }
    else
    {
        LED_SEM_LOCK(unit);
        switch (type)
        {
            case LED_TYPE_SYS:
                field_id = ESW_P27_LED0f;
                break;
            case LED_TYPE_ALARM:
                field_id = ESW_P26_LED0f;
                break;
            case RTK_LED_TYPE_END:
            default:
                LED_SEM_UNLOCK(unit);
                return RT_ERR_FAILED;
        }

        if ((ret = reg_field_read(unit, ESW_SOFTWARE_LED_CONTROL6r, field_id, &value)) != RT_ERR_OK)
        {
            LED_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }

        *pEnable = (value != 0)?1:0;

        LED_SEM_UNLOCK(unit);
    }

    return RT_ERR_OK;
} /* end of dal_esw_led_sysEnable_get */

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
int32
dal_esw_led_sysEnable_set(uint32 unit, rtk_led_type_t type, rtk_enable_t enable)
{
    uint32  value = 0, bit_offset, field_id;
    uint32  led_scan_mode;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_LED), "unit=%d, type=%d, enable=%d", 
           unit, type, enable); 
    
    /* check Init status */
    RT_INIT_CHK(led_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(type >= RTK_LED_TYPE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);

    if ((ret = reg_field_read(unit, ESW_LED_SYSTEM_CONTROL0r, ESW_SEL_LED_MODEf, &led_scan_mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_LED), "");
        return ret;
    }

    /* scan mode is supported the sys/alarm led from external 8231 */
    if (led_scan_mode)
    {
        LED_SEM_LOCK(unit);
    
        switch (type)
        {
            case LED_TYPE_SYS:
                bit_offset = 14;
                break;
            case LED_TYPE_ALARM:
                bit_offset = 13;
                break;
            case RTK_LED_TYPE_END:
            default:
                LED_SEM_UNLOCK(unit);
                return RT_ERR_FAILED;
        }
    
        /* Indirect read external gpio address 4 */
        value = 0x1f04;
        if ((ret = reg_write(unit, ESW_TRIGGER_EXTERNAL_GPIOr, &value)) != RT_ERR_OK)
        {
            LED_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }
    
        LED_BUSY_WAIT_LOOP(unit, ESW_TRIGGER_EXTERNAL_GPIOr, 0x1000);
    
        if ((ret = reg_read(unit, ESW_EXTERNAL_DATA_READr, &value)) != RT_ERR_OK)
        {
            LED_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }
        
        if (ENABLED == enable)
            value |= (1 << bit_offset);
        else
            value &= ~(1 << bit_offset);
        
        if ((ret = reg_write(unit, ESW_EXTERNAL_DATA_WRITEr, &value)) != RT_ERR_OK)
        {
            LED_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }
    
        value = 0x1704;
        if ((ret = reg_write(unit, ESW_TRIGGER_EXTERNAL_GPIOr, &value)) != RT_ERR_OK)
        {
            LED_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }
    
        LED_SEM_UNLOCK(unit);
    }
    else
    {
        LED_SEM_LOCK(unit);
        switch (type)
        {
            case LED_TYPE_SYS:
                field_id = ESW_P27_LED0f;
                break;
            case LED_TYPE_ALARM:
                field_id = ESW_P26_LED0f;
                break;
            case RTK_LED_TYPE_END:
            default:
                LED_SEM_UNLOCK(unit);
                return RT_ERR_FAILED;
        }

        if (ENABLED == enable)
            value = 1;
        else
            value = 0;

        if ((ret = reg_field_write(unit, ESW_SOFTWARE_LED_CONTROL6r, field_id, &value)) != RT_ERR_OK)
        {
            LED_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_LED), "");
            return ret;
        }

        LED_SEM_UNLOCK(unit);
    }

    return RT_ERR_OK;
} /* end of dal_esw_led_sysEnable_set */


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
int32
dal_esw_led_8231Reset_workaround(uint32 unit)
{
    uint32  dev;
    int32   ret;
    rtk_enable_t    rtl8231_state = ENABLED;

    for (dev = 0; dev < EXT_GPIO_DEV_ID_END; dev++)
    {
        if ((ret=drv_extGpio_devEnable_get(unit, dev, &rtl8231_state)) != RT_ERR_OK)
            continue;

        if (DISABLED == rtl8231_state)
        {   /* 8231 maybe be reset and reconfig to recovery it */
            RT_LOG(LOG_DEBUG, MOD_DAL, "workaround 8231 reset mechanism recovery: unit=%d, dev=%d", unit, dev);
            drv_extGpio_devRecovery_start(unit, dev);
        }
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_led_8231Reset_workaround */
#endif
