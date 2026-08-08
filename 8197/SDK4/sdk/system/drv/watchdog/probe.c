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
 * $Revision: 57050 $
 * $Date: 2015-03-23 14:36:24 +0800 (Mon, 23 Mar 2015) $
 *
 * Purpose : Definition those public watchdog APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) watchdog probe
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <drv/swcore/chip.h>
#include <drv/watchdog/probe.h>
#if defined(CONFIG_SDK_RTL8389)
#include <drv/watchdog/r8389.h>
#endif
#if defined(CONFIG_SDK_RTL8328)
#include <drv/watchdog/r8328.h>
#endif
#if defined(CONFIG_SDK_RTL8390)
#include <drv/watchdog/r8390.h>
#endif
#if  defined(CONFIG_SDK_RTL8380)
#include <drv/watchdog/r8380.h>
#endif

/*
 * Symbol Definition
 */
#define WDG_DB_SIZE (sizeof(wdg_db)/sizeof(cid_group_t))

/*
 * Data Declaration
 */
const static cid_group_t wdg_db[] =
{
#if defined(CONFIG_SDK_RTL8389)
    /* RTL8389 series chips */
    {RTL8389M_CHIP_ID, WDG_R8389},
    {RTL8329M_CHIP_ID, WDG_R8389},
    {RTL8377M_CHIP_ID, WDG_R8389},
    {RTL8389L_CHIP_ID, WDG_R8389},
#endif
#if defined(CONFIG_SDK_RTL8328)
    /* RTL8328 series chips */
    {RTL8328M_CHIP_ID, WDG_R8328},
    {RTL8328S_CHIP_ID, WDG_R8328},
    {RTL8328L_CHIP_ID, WDG_R8328},
#endif
#if defined(CONFIG_SDK_RTL8390)
    /* RTL8390 series chips */
    {RTL8352M_CHIP_ID, WDG_R8390},
    {RTL8353M_CHIP_ID, WDG_R8390},
    {RTL8391M_CHIP_ID, WDG_R8390},
    {RTL8392M_CHIP_ID, WDG_R8390},
    {RTL8393M_CHIP_ID, WDG_R8390},
    {RTL8396M_CHIP_ID, WDG_R8390},
    {RTL8352MES_CHIP_ID, WDG_R8390},
    {RTL8353MES_CHIP_ID, WDG_R8390},
    {RTL8392MES_CHIP_ID, WDG_R8390},
    {RTL8393MES_CHIP_ID, WDG_R8390},
    {RTL8396MES_CHIP_ID, WDG_R8390},
#endif
#if defined(CONFIG_SDK_RTL8380)
    /* RTL8380 series chips */
    {RTL8380M_CHIP_ID, WDG_R8380},
    {RTL8330M_CHIP_ID, WDG_R8380},
    {RTL8382M_CHIP_ID, WDG_R8380},
    {RTL8332M_CHIP_ID, WDG_R8380},
    {RTL8380MES_CHIP_ID, WDG_R8380},
    {RTL8330MES_CHIP_ID, WDG_R8380},
    {RTL8382MES_CHIP_ID, WDG_R8380},
    {RTL8332MES_CHIP_ID, WDG_R8380},
#endif
};

wdg_mapper_operation_t wdg_ops[WDG_CTRL_END] =
{
#if defined(CONFIG_SDK_RTL8389)
    {   /* WDG_R8389 */
        .mode_set = r8389_watchdog_mode_set,
        .mode_get = r8389_watchdog_mode_get,
        .scale_set = r8389_watchdog_scale_set,
        .scale_get = r8389_watchdog_scale_get,
        .enable_set = r8389_watchdog_enable_set,
        .enable_get = r8389_watchdog_enable_get,
        .kick = r8389_watchdog_kick,
        .init = r8389_watchdog_init,
        .threshold_set = drv_watchdog_unavail,
        .threshold_get = drv_watchdog_unavail,
    },
#endif
#if defined(CONFIG_SDK_RTL8328)
    {   /* WDG_R8328 */
        .mode_set = r8328_watchdog_mode_set,
        .mode_get = r8328_watchdog_mode_get,
        .scale_set = r8328_watchdog_scale_set,
        .scale_get = r8328_watchdog_scale_get,
        .enable_set = r8328_watchdog_enable_set,
        .enable_get = r8328_watchdog_enable_get,
        .kick = r8328_watchdog_kick,
        .init = r8328_watchdog_init,
        .threshold_set = (int32 (*)(uint32, drv_watchdog_threshold_t *))drv_watchdog_unavail,
        .threshold_get = (int32 (*)(uint32, drv_watchdog_threshold_t *))drv_watchdog_unavail,
    },
#endif
#if defined(CONFIG_SDK_RTL8390)
    {   /* WDG_R8390 */
        .mode_set = r8390_watchdog_mode_set,
        .mode_get = r8390_watchdog_mode_get,
        .scale_set = r8390_watchdog_scale_set,
        .scale_get = r8390_watchdog_scale_get,
        .enable_set = r8390_watchdog_enable_set,
        .enable_get = r8390_watchdog_enable_get,
        .kick = r8390_watchdog_kick,
        .init = r8390_watchdog_init,
        .threshold_set = r8390_watchdog_threshold_set,
        .threshold_get = r8390_watchdog_threshold_get,
    },
#endif
#if defined(CONFIG_SDK_RTL8380)
    {   /* WDG_R8380 */
        .mode_set = r8380_watchdog_mode_set,
        .mode_get = r8380_watchdog_mode_get,
        .scale_set = r8380_watchdog_scale_set,
        .scale_get = r8380_watchdog_scale_get,
        .enable_set = r8380_watchdog_enable_set,
        .enable_get = r8380_watchdog_enable_get,
        .kick = r8380_watchdog_kick,
        .init = r8380_watchdog_init,
        .threshold_set = r8380_watchdog_threshold_set,
        .threshold_get = r8380_watchdog_threshold_get,
    },
#endif
};

uint32 wdg_if[RTK_MAX_NUM_OF_UNIT];
uint32 wdg_chipId[RTK_MAX_NUM_OF_UNIT];

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
int32
watchdog_probe(uint32 unit)
{
    uint32 i;

    for (i = 0; i < WDG_DB_SIZE; i++)
    {
        if(!drv_swcore_cid_cmp(unit, wdg_db[i].cid))
        {
            wdg_if[unit] = wdg_db[i].gid;
            wdg_chipId[unit] = wdg_db[i].cid;
			return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;

} /* end of watchdog_probe */
