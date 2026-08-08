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
 * $Revision: 36864 $
 * $Date: 2013-02-05 20:30:03 +0800 (Tue, 05 Feb 2013) $
 *
 * Purpose : Definition those public RTL8231 APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) RTL8231 probe
 * 
 */ 
#ifndef __DRV_RTL8231_PROBE_H__
#define __DRV_RTL8231_PROBE_H__

/*  
 * Include Files 
 */
#include <drv/rtl8231/rtl8231.h>

/* 
 * Symbol Definition 
 */
typedef enum rtl8231_controller_e
{
    RTL8231_R8389,
    RTL8231_R8328,
    RTL8231_R8390,
    RTL8231_R8380,
    RTL8231_CTRL_END            
} rtl8231_controller_t;

typedef struct rtl8231_mapper_operation_s
{
    /* RTL8231 */
    int32   (*i2c_read)(uint32 , uint32 , uint32 , uint32 *);
    int32   (*i2c_write)(uint32 , uint32 , uint32 , uint32 );
    int32   (*mdc_read)(uint32 , uint32 , uint32 , uint32 , uint32 *);
    int32   (*mdc_write)(uint32 , uint32 , uint32 , uint32 , uint32 );
    int32   (*init)(uint32 );
    int32   (*mdcSem_register)(uint32 , drv_rtl8231_mdcSem_cb_f);
    int32   (*mdcSem_unregister)(uint32 , drv_rtl8231_mdcSem_cb_f);
    int32   (*extra_devReady_get)(uint32 , uint32 , uint32 *);
    int32   (*extra_devEnable_get)(uint32 , uint32 , rtk_enable_t *);
    int32   (*extra_devEnable_set)(uint32 , uint32 , rtk_enable_t );
    int32   (*extra_dataBit_get)(uint32 , uint32 , uint32 , uint32 *);
    int32   (*extra_dataBit_set)(uint32 , uint32 , uint32 , uint32 );
    int32   (*extra_direction_get)(uint32 , uint32 , uint32 , drv_gpio_direction_t *);
    int32   (*extra_direction_set)(uint32 , uint32 , uint32 , drv_gpio_direction_t );
    int32   (*extra_i2c_init)(uint32 , uint32 , uint32 , uint32 );
    int32   (*extra_i2c_get)(uint32 , uint32 , uint32 *, uint32 *);
    int32   (*extra_i2c_read)(uint32 , uint32 , uint32 , uint32 *);
    int32   (*extra_i2c_write)(uint32 , uint32 , uint32 , uint32 );
    int32   (*extra_read)(uint32 , uint32 , uint32 , uint32 *);
    int32   (*extra_write)(uint32 , uint32 , uint32 , uint32 );
} rtl8231_mapper_operation_t;

#define RTL8231_CTRL(unit)      rtl8231_ops[rtl8231_if[unit]]

/* 
 * Data Declaration 
 */
extern rtl8231_mapper_operation_t rtl8231_ops[RTL8231_CTRL_END];
extern uint32 rtl8231_if[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rtl8231_probe
 * Description:
 *      Probe rtl8231 module of the specified device.
 * Input:
 *      unit - unit id 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
rtl8231_probe(uint32 unit);

/* Function Name: 
 *      rtl8231_common_unavail
 * Description: 
 *      Return chip not support
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_CHIP_NOT_SUPPORTED   - functions not supported by this chip model
 * Note: 
 *      None
 */ 
extern int32
rtl8231_common_unavail(void);

#endif /* __DRV_RTL8231_PROBE_H__ */

