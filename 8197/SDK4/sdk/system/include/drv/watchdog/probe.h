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
 * $Revision: 34390 $
 * $Date: 2012-11-16 10:02:15 +0800 (Fri, 16 Nov 2012) $
 *
 * Purpose : Definition those public watchdog APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) watchdog probe
 * 
 */ 
#ifndef __DRV_WATCHDOG_PROBE_H__
#define __DRV_WATCHDOG_PROBE_H__

/*  
 * Include Files 
 */
#include <common/rt_autoconf.h>
#include <drv/watchdog/watchdog.h>

/* 
 * Symbol Definition 
 */
typedef enum wdg_controller_e
{
#if defined(CONFIG_SDK_RTL8389)
    WDG_R8389,
#endif
#if defined(CONFIG_SDK_RTL8328)
    WDG_R8328,
#endif
#if defined(CONFIG_SDK_RTL8390)
    WDG_R8390,
#endif
#if defined(CONFIG_SDK_RTL8380)
    WDG_R8380,
#endif
    WDG_CTRL_END            
} wdg_controller_t;

typedef struct wdg_mapper_operation_s
{
    /* WATCHDOG */
    int32   (*mode_set)(uint32, drv_watchdog_mode_t);
    int32   (*mode_get)(uint32, drv_watchdog_mode_t *);
    int32   (*scale_set)(uint32, drv_watchdog_scale_t);
    int32   (*scale_get)(uint32, drv_watchdog_scale_t *);
    int32   (*enable_set)(uint32, uint32);
    int32   (*enable_get)(uint32, uint32 *);
    int32   (*kick)(uint32);
    int32   (*init)(uint32);
    int32   (*threshold_set)(uint32, drv_watchdog_threshold_t *);
    int32   (*threshold_get)(uint32, drv_watchdog_threshold_t *);
} wdg_mapper_operation_t;

#define WDG_CTRL(unit)      wdg_ops[wdg_if[unit]]

/* 
 * Data Declaration 
 */
extern wdg_mapper_operation_t wdg_ops[WDG_CTRL_END];
extern uint32 wdg_if[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      watchdog_probe
 * Description:
 *      Probe watchdog module of the specified device.
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
watchdog_probe(uint32 unit);

#endif /* __DRV_WATCHDOG_PROBE_H__ */

