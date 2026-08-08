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
 * $Revision: $
 * $Date: $
 *
 * Purpose : Software Control LED
 *
 * Feature : For RTL8231 Serial Mode
 *
 * Variable:
 *               Global paramters define.
 */


#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <drv/swled/swctrl_led_main.h>

swCtrl_led_mapper_operation_t swCtrl_led_ops[SWCTRL_LED_CHIP_END] = 
{
    {   /* SWCTRL_LED_R8390 */
        .getPortSpeedDuplex = rtl8390_getAsicPortSpeedDuplex,   /* should change to 8390 API */
        .getPortLink = rtl8390_getAsicPortLinkStat,             /* should change to 8390 API */
        .getMIBPortCounter = rtl8390_getAsicMIBPortCounter,     /* should change to 8390 API */
    },
};  


