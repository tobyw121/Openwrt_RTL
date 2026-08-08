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
 * $Revision: 21606 $
 * $Date: 2011-08-29 14:46:40 +0800 (Mon, 29 Aug 2011) $
 *
 * Purpose : Hardware Abstraction Layer (HAL) control structure and definition in the SDK.
 *
 * Feature : HAL control structure and definition
 *
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <osal/lib.h>
#include <hal/common/halctrl.h>
#include <hal/mac/mac_probe.h>
#include <hal/phy/phy_probe.h>


/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
hal_control_t hal_ctrl[RTK_MAX_NUM_OF_UNIT];


/*
 * Function Declaration
 */

/* Function Name:
 *      hal_init
 * Description:
 *      Initialize the hal layer API.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - General Error
 *      RT_ERR_CHIP_NOT_FOUND   - The chip can not found
 *      RT_ERR_DRIVER_NOT_FOUND - The driver can not found
 * Note:
 *      Initialize the hal layer API, include get the chip id and chip revision
 *      id, get its driver id and driver revision id, then bind to its major
 *      driver. Also initialize its hal_ctrl structure for this specified unit.
 *      Before calling the function, bsps should already scan HW interface, like
 *      PCI device in all buses, or physical Lextra, and the total chip numbers,
 *      its chip id and chip revision id is known and store in database in lower
 *      layer.
 */
int32
hal_init(uint32 unit)
{
    int32           ret = RT_ERR_FAILED;

    osal_memset((uint8 *)&hal_ctrl[unit], 0, sizeof(hal_control_t));

    /* Probe MAC */
    if ((ret = mac_probe(unit)) != RT_ERR_OK)
    {
        RT_ERR(ret, MOD_HAL, "mac_probe(unit=%d) failed!!", unit);
        return ret;
    }

    /* Init MAC */
    if ((ret = mac_init(unit)) != RT_ERR_OK)
    {
        RT_ERR(ret, MOD_HAL, "mac_init(unit=%d) failed!!", unit);
        return ret;
    }

    /* Probe PHY */
    if ((ret = phy_probe(unit)) != RT_ERR_OK)
    {
        RT_ERR(ret, MOD_HAL, "phy_probe(unit=%d) failed!!", unit);
        return ret;
    }

    /* Init PHY */
    if ((ret = phy_init(unit)) != RT_ERR_OK)
    {
        RT_ERR(ret, MOD_HAL, "phy_init(unit=%d) failed!!", unit);
        return ret;
    }

    return RT_ERR_OK;
} /* end of hal_init */

/* Function Name:
 *      hal_ctrlInfo_get
 * Description:
 *      Find the hal control information structure for this specified unit.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      NULL      - Not found
 *      Otherwise - Pointer of hal control information structure that found
 * Note:
 *      The function have found the exactly hal control information structure.
 */
hal_control_t *
hal_ctrlInfo_get(uint32 unit)
{
    if (0 == hal_ctrl[unit].chip_flags)
    {
        return NULL;
    }
    else
    {
        return &hal_ctrl[unit];
    }
} /* end of hal_ctrlInfo_get */

/* Function Name:
 *      hal_portMaxBandwidth_ret
 * Description:
 *      Get the max bandwith of port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      max bandwidth value
 * Note:
 *      The return value is different in FE/GE/10GE port. 
 */
uint32
hal_portMaxBandwidth_ret(uint32 unit, rtk_port_t port)
{
    if (HAL_IS_FE_PORT(unit, port))
        return (hal_ctrl[unit].pDev_info->pCapacityInfo->rate_of_bandwidth_max_fe_port);
    else if (HAL_IS_GE_PORT(unit, port))
        return (hal_ctrl[unit].pDev_info->pCapacityInfo->rate_of_bandwidth_max_ge_port);
    else if (HAL_IS_10GE_PORT(unit, port))
        return (hal_ctrl[unit].pDev_info->pCapacityInfo->rate_of_bandwidth_max_10ge_port);
    else
        return (hal_ctrl[unit].pDev_info->pCapacityInfo->rate_of_bandwidth_max);
} /* end of hal_portMaxBandwidth_ret */

