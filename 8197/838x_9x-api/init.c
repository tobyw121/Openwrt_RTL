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
 * $Revision: 30055 $
 * $Date: 2012-06-19 15:33:08 +0800 (Tue, 19 Jun 2012) $
 *
 * Purpose : Definition of Init API
 *
 * Feature : Initialize All Layers of RTK Module
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <ioal/ioal_init.h>
#include <hal/common/halctrl.h>
#include <dal/dal_mgmt.h>
#include <rtk/init.h>
#include <rtk/default.h>
#include <rtk/dot1x.h>
#include <rtk/filter.h>
#include <rtk/flowctrl.h>
#include <rtk/l2.h>
#include <rtk/mirror.h>
#include <rtk/port.h>
#include <rtk/qos.h>
#include <rtk/rate.h>
#include <rtk/stat.h>
#include <rtk/stp.h>
#include <rtk/svlan.h>
#include <rtk/switch.h>
#include <rtk/trap.h>
#include <rtk/trunk.h>
#include <rtk/vlan.h>
#if defined(CONFIG_SDK_DRIVER_RTK_CUSTOMER)
#include <rtk/customer_hook.h>
#endif

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
 *      rtk_init
 * Description:
 *      Initialize the specified device
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      INIT must be initialized before using all of APIs in each modules
 */
int32
rtk_init(uint32 unit)
{
    int32 ret = RT_ERR_FAILED;

    /* Check whether unit id is valid(not out of range) */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID), RT_ERR_UNIT_ID);

    RT_DBG(LOG_EVENT, MOD_INIT, "rtk_init Start!!");

    /* Initialize the hal layer */
    if ((ret = hal_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_INIT, "hal_init Failed!!");
        return ret;
    }
    RT_DBG(LOG_EVENT, MOD_INIT, "hal_init Completed!!");

    /* Initialize the dal layer */
    if ((ret = dal_mgmt_init()) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_INIT, "dal_mgmt_init Failed!!");
        return ret;
    }
    RT_DBG(LOG_EVENT, MOD_INIT, "dal_mgmt_init Completed!!");

    if ((ret = dal_mgmt_initDevice(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_INIT, "dal_mgmt_initDevice Failed!!");
        return ret;
    }
	
    RT_DBG(LOG_EVENT, MOD_INIT, "dal_mgmt_initDevice Completed!!");

#if defined(CONFIG_SDK_DRIVER_RTK_CUSTOMER)
    /*Customer RTK API initial process will be executed over here*/
    if ((ret = rtk_customer_api_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_INIT, "rtk_customer_api_init Failed!!");
        return ret;
    }
#endif	

    return ret;

} /* end of rtk_init */
