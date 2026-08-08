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
 * Purpose : MAC probe and init service APIs in the SDK.
 *
 * Feature : MAC probe and init service APIs
 *
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <drv/swcore/chip.h>
#include <hal/chipdef/chip.h>
#include <hal/chipdef/driver.h>
#include <hal/common/halctrl.h>


/*
 * Symbol Definition
 */
#define VALUE_NO_INIT   (-1)


/*
 * Macro Definition
 */
#define HAL_ADD_PORT(pDev, portType, port)\
do {\
    pDev->pPortinfo->portType.portNum++;\
    if (pDev->pPortinfo->portType.min < 0) { pDev->pPortinfo->portType.min = port;}\
    if (port > pDev->pPortinfo->portType.max) {pDev->pPortinfo->portType.max = port;}\
    RTK_PORTMASK_PORT_SET(pDev->pPortinfo->portType.portmask, port);\
} while(0);

#define HAL_DEL_PORT(pDev, portType, port)\
do {\
    RTK_PORTMASK_PORT_CLEAR(pDev->pPortinfo->portType.portmask, port);\
    pDev->pPortinfo->portType.portNum--;\
    if (pDev->pPortinfo->portType.portNum == 0) { pDev->pPortinfo->portType.min = pDev->pPortinfo->portType.max = VALUE_NO_INIT;}\
    if (port == pDev->pPortinfo->portType.min) { pDev->pPortinfo->portType.min = RTK_PORTMASK_GET_FIRST_PORT(pDev->pPortinfo->portType.portmask);}\
    if (port == pDev->pPortinfo->portType.max) {pDev->pPortinfo->portType.max = RTK_PORTMASK_GET_LAST_PORT(pDev->pPortinfo->portType.portmask);}\
} while(0);

#define HAL_ADD_SERDES(pDev, serdesType, serdes)\
do {\
    pDev->pSerdesInfo->serdesType.serdesNum++;\
    if (pDev->pSerdesInfo->serdesType.min < 0) { pDev->pSerdesInfo->serdesType.min = serdes;}\
    if (serdes > pDev->pSerdesInfo->serdesType.max) {pDev->pSerdesInfo->serdesType.max = serdes;}\
    RTK_SERDESMASK_SERDES_SET(pDev->pSerdesInfo->serdesType.serdesmask, serdes);\
} while(0);

/*
 * Function Declaration
 */
static int32 _hal_portInfo_init(rt_device_t *pDev);


/* Function Name:
 *      _hal_portInfo_init
 * Description:
 *      Init port info, it will be used to fill port mask, max, min value for each port type
 * Input:
 *      pDev - pointer buffer of device database for inited
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - OK
 * Note:
 */
static int32
_hal_portInfo_init(rt_device_t *pDev)
{
    int32  port;

    /* clear port info database */
    pDev->pPortinfo->port_number = 0;
    pDev->pPortinfo->cpuPort = VALUE_NO_INIT;

    pDev->pPortinfo->fe.portNum = 0;
    pDev->pPortinfo->fe.max = VALUE_NO_INIT;
    pDev->pPortinfo->fe.min = VALUE_NO_INIT;
    osal_memset(&(pDev->pPortinfo->fe.portmask), 0, sizeof(rtk_portmask_t));

    pDev->pPortinfo->fe_int.portNum = 0;
    pDev->pPortinfo->fe_int.max = VALUE_NO_INIT;
    pDev->pPortinfo->fe_int.min = VALUE_NO_INIT;
    osal_memset(&(pDev->pPortinfo->fe_int.portmask), 0, sizeof(rtk_portmask_t));

    pDev->pPortinfo->ge.portNum = 0;
    pDev->pPortinfo->ge.max = VALUE_NO_INIT;
    pDev->pPortinfo->ge.min = VALUE_NO_INIT;
    osal_memset(&(pDev->pPortinfo->ge.portmask), 0, sizeof(rtk_portmask_t));

    pDev->pPortinfo->ether.portNum = 0;
    pDev->pPortinfo->ether.max = VALUE_NO_INIT;
    pDev->pPortinfo->ether.min = VALUE_NO_INIT;
    osal_memset(&(pDev->pPortinfo->ether.portmask), 0, sizeof(rtk_portmask_t));

    pDev->pPortinfo->ge_combo.portNum = 0;
    pDev->pPortinfo->ge_combo.max = VALUE_NO_INIT;
    pDev->pPortinfo->ge_combo.min = VALUE_NO_INIT;
    osal_memset(&(pDev->pPortinfo->ge_combo.portmask), 0, sizeof(rtk_portmask_t));

    pDev->pPortinfo->serdes.portNum = 0;
    pDev->pPortinfo->serdes.max = VALUE_NO_INIT;
    pDev->pPortinfo->serdes.min = VALUE_NO_INIT;
    osal_memset(&(pDev->pPortinfo->serdes.portmask), 0, sizeof(rtk_portmask_t));

    pDev->pPortinfo->ge_10ge.portNum = 0;
    pDev->pPortinfo->ge_10ge.max = VALUE_NO_INIT;
    pDev->pPortinfo->ge_10ge.min = VALUE_NO_INIT;
    osal_memset(&(pDev->pPortinfo->ge_10ge.portmask), 0, sizeof(rtk_portmask_t));

    pDev->pPortinfo->all.portNum = 0;
    pDev->pPortinfo->all.max = VALUE_NO_INIT;
    pDev->pPortinfo->all.min = VALUE_NO_INIT;
    osal_memset(&(pDev->pPortinfo->all.portmask), 0, sizeof(rtk_portmask_t));

    /* Check each port type and fill related info */
    for (port = 0; port < RTK_MAX_NUM_OF_PORTS; port++)
    {
        switch (pDev->pPortinfo->portType[port])
        {
            case RT_INT_FE_PORT:
                HAL_ADD_PORT(pDev, fe_int, port);
            case RT_FE_PORT:
                HAL_ADD_PORT(pDev, fe, port);
                HAL_ADD_PORT(pDev, ether, port);
                HAL_ADD_PORT(pDev, all, port);
                break;

            case RT_GE_PORT:
                HAL_ADD_PORT(pDev, ge, port);
                HAL_ADD_PORT(pDev, ether, port);
                HAL_ADD_PORT(pDev, all, port);
                break;

            case RT_10GE_PORT:
                HAL_ADD_PORT(pDev, ge_10ge, port);
                HAL_ADD_PORT(pDev, ether, port);
                HAL_ADD_PORT(pDev, all, port);
                break;

            case RT_CPU_PORT:
                pDev->pPortinfo->cpuPort = port;
                HAL_ADD_PORT(pDev, all, port);
                break;

            default:
                continue;
        }

        pDev->pPortinfo->port_number++;
    }

    return RT_ERR_OK;
} /* end of _hal_portInfo_init */

#if 0
/* Function Name:
 *      _hal_serdesInfo_init
 * Description:
 *      Init serdes info, it will be used to fill serdes mask, max, min value for each serdes type
 * Input:
 *      pDev - pointer buffer of device database for inited
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - OK
 * Note:
 */
static int32
_hal_serdesInfo_init(rt_device_t *pDev)
{
    int32  serdes;

    /* clear serdes info database */
    pDev->pSerdesInfo->serdes_number = 0;

    pDev->pSerdesInfo->all.serdesNum = 0;
    pDev->pSerdesInfo->all.max = VALUE_NO_INIT;
    pDev->pSerdesInfo->all.min = VALUE_NO_INIT;
    osal_memset(&(pDev->pSerdesInfo->all.serdesmask), 0, sizeof(rtk_serdesmask_t));

    /* Check each serdes type and fill related info */
    for (serdes = 0; serdes < RTK_MAX_NUM_OF_SERDES; serdes++)
    {
        switch (pDev->pSerdesInfo->serdesType[serdes])
        {
            case RT_SERDES_RS8MII:
            case RT_SERDES_SGMII:
            case RT_SERDES_5G:
            case RT_SERDES_R_XAUI:
            case RT_SERDES_10G_R:
                HAL_ADD_SERDES(pDev, all, serdes);
                break;

            default:
                continue;
        }

        pDev->pSerdesInfo->serdes_number++;
    }

    return RT_ERR_OK;
} /* end of _hal_serdesInfo_init */
#endif

/* Public Function Body */
int32
hal_portInfo_update(uint32 unit, int32 port, rt_port_type_t type)
{
    rt_device_t *pDev = NULL;

    switch (type)
    {
        case RT_GE_COMBO_PORT:
            pDev = hal_ctrl[unit].pDev_info;
            pDev->pPortinfo->portType[port] = RT_GE_COMBO_PORT;
            HAL_ADD_PORT(pDev, ge_combo, port);
            break;

        case RT_GE_SERDES_PORT:
            pDev = hal_ctrl[unit].pDev_info;
            pDev->pPortinfo->portType[port] = RT_GE_SERDES_PORT;
            HAL_ADD_PORT(pDev, serdes, port);
            break;

        case RT_10GE_PORT:
            pDev = hal_ctrl[unit].pDev_info;
            pDev->pPortinfo->portType[port] = RT_10GE_PORT;
            HAL_ADD_PORT(pDev, ge_10ge, port);
            break;

        case RT_10GE_SERDES_PORT:
            pDev = hal_ctrl[unit].pDev_info;
            pDev->pPortinfo->portType[port] = RT_10GE_SERDES_PORT;
            HAL_ADD_PORT(pDev, serdes, port);
            HAL_ADD_PORT(pDev, serdes_10ge, port);
            break;

        case RT_PORT_NONE:
            pDev = hal_ctrl[unit].pDev_info;
            switch (pDev->pPortinfo->portType[port])
            {
                case RT_GE_SERDES_PORT:
                    pDev->pPortinfo->portType[port] = type;
                    HAL_DEL_PORT(pDev, serdes, port);
                    HAL_DEL_PORT(pDev, ge, port);
                    HAL_DEL_PORT(pDev, ether, port);
                    HAL_DEL_PORT(pDev, all, port);
                    pDev->pPortinfo->port_number--;
                    break;

                case RT_GE_PORT:
                    pDev->pPortinfo->portType[port] = type;
                    HAL_DEL_PORT(pDev, ge, port);
                    HAL_DEL_PORT(pDev, ether, port);
                    HAL_DEL_PORT(pDev, all, port);
                    pDev->pPortinfo->port_number--;
                    break;

                case RT_10GE_PORT:
                    pDev->pPortinfo->portType[port] = type;
                    HAL_DEL_PORT(pDev, ge_10ge, port);
                    HAL_DEL_PORT(pDev, ether, port);
                    HAL_DEL_PORT(pDev, all, port);
                    pDev->pPortinfo->port_number--;
                    break;

                case RT_10GE_SERDES_PORT:
                    pDev->pPortinfo->portType[port] = type;
                    HAL_DEL_PORT(pDev, serdes, port);
                    HAL_DEL_PORT(pDev, serdes_10ge, port);
                    HAL_DEL_PORT(pDev, ge_10ge, port);
                    HAL_DEL_PORT(pDev, ether, port);
                    HAL_DEL_PORT(pDev, all, port);
                    pDev->pPortinfo->port_number--;
                    break;
				case RT_FE_PORT:
                    pDev->pPortinfo->portType[port] = type;
                    HAL_DEL_PORT(pDev, fe, port);
                    HAL_DEL_PORT(pDev, ether, port);
                    HAL_DEL_PORT(pDev, all, port);
                    pDev->pPortinfo->port_number--;
					break;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    return RT_ERR_OK;
} /* end of hal_portInfo_update */

/* Function Name:
 *      mac_probe
 * Description:
 *      Probe the MAC chip in the specified chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
mac_probe(uint32 unit)
{
    uint32  temp_chip_id = 0, temp_chip_rev_id = 0;
    int32   ret = RT_ERR_FAILED;
    rt_device_t *pDev = NULL;
    rt_driver_t *pMdriver = NULL;

    /* Get chip_id/chip_rev_id and check the value */
    if ((ret = drv_swcore_cid_get(unit, &temp_chip_id, &temp_chip_rev_id)) != RT_ERR_OK)
    {
        RT_ERR(ret, MOD_HAL, "drv_swcore_cid_get(unit=%d) failed!!", unit);
        return ret;
    }

    hal_ctrl[unit].chip_id = temp_chip_id;
    hal_ctrl[unit].chip_rev_id = temp_chip_rev_id;

    RT_LOG(LOG_INFO, MOD_HAL, "unit=%d, chip_id=0x%X, rev_id=0x%X",
           unit, hal_ctrl[unit].chip_id, hal_ctrl[unit].chip_rev_id);

    /* Find device */
    if ((pDev = hal_find_device(hal_ctrl[unit].chip_id, hal_ctrl[unit].chip_rev_id)) == NULL)
    {
        RT_DBG(LOG_MINOR_ERR, MOD_HAL, "hal_find_device(unit=%d, chip_id=0x%X, rev_id=0x%X) failed!!",\
               unit, hal_ctrl[unit].chip_id, hal_ctrl[unit].chip_rev_id);

        return RT_ERR_CHIP_NOT_FOUND;
    }

    if ((CHIP_FAMILY_IS_RTL8330(hal_ctrl[unit].chip_id) || 
         CHIP_FAMILY_IS_RTL8380(hal_ctrl[unit].chip_id)) &&
        (hal_ctrl[unit].chip_rev_id <= CHIP_REV_ID_B) )
    {
        pDev->pCapacityInfo->max_num_of_metering = 64;
    }
    hal_ctrl[unit].pDev_info = pDev;

    /* Find chip major driver */
    if ((pMdriver = hal_find_driver(hal_ctrl[unit].pDev_info->driver_id,\
        hal_ctrl[unit].pDev_info->driver_rev_id)) == NULL)
    {
        RT_DBG(LOG_MINOR_ERR, MOD_HAL, "hal_find_driver(unit=%d, drv_id=0x%X, drv_rev_id=0x%X) failed!!",\
               unit, hal_ctrl[unit].pDev_info->driver_id, hal_ctrl[unit].pDev_info->driver_rev_id);

        return RT_ERR_DRIVER_NOT_FOUND;
    }
    hal_ctrl[unit].pChip_driver = pMdriver;
    hal_ctrl[unit].chip_flags |= HAL_CHIP_ATTACHED;

    /* Get chip mode info */
    if (HAL_IS_RTL8390_FAMILY_ID(unit) || HAL_IS_RTL8350_FAMILY_ID(unit))
    {
        if ((ret = hal_chip_mode_get(unit, pDev)) != RT_ERR_OK)
        {
            RT_ERR(ret, MOD_HAL, "hal_chip_mode_get failed!!");
            return ret;
        }
    }

    /* Initialize port information database */
    if ((ret = _hal_portInfo_init(hal_ctrl[unit].pDev_info)) != RT_ERR_OK)
    {
        RT_ERR(ret, MOD_HAL, "_hal_portInfo_init(unit=%d) failed!!", unit);
        return ret;
    }
#if 0
    /* Initialize serdes information database */
    if ((ret = _hal_serdesInfo_init(hal_ctrl[unit].pDev_info)) != RT_ERR_OK)
    {
        RT_ERR(ret, MOD_HAL, "_hal_serdesInfo_init(unit=%d) failed!!", unit);
        return ret;
    }
#endif

    return RT_ERR_OK;
} /* end of mac_probe */


/* Function Name:
 *      mac_init
 * Description:
 *      Probe the MAC chip in the specified chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
mac_init(uint32 unit)
{
    uint32  i;
    int32   ret = RT_ERR_FAILED;

    /* Create semaphores for HAL protection mechanism */
    hal_ctrl[unit].reg_sem = osal_sem_mutex_create();
    if (0 == hal_ctrl[unit].reg_sem)
    {
        RT_DBG(LOG_MINOR_ERR, MOD_HAL, "unit %d, register semaphore create failed!!", unit);
        return RT_ERR_FAILED;
    }

    for (i = 0; i < RTK_INDIRECT_CTRL_GROUP_END; i++)
    {
        hal_ctrl[unit].tbl_sem[i] = osal_sem_mutex_create();
        if (0 == hal_ctrl[unit].tbl_sem[i])
        {
            RT_DBG(LOG_MINOR_ERR, MOD_HAL, "unit %d, table semaphore %d create failed!!", unit, i);
            return RT_ERR_FAILED;
        }
    }

    hal_ctrl[unit].phy_sem = osal_sem_mutex_create();
    if (0 == hal_ctrl[unit].phy_sem)
    {
        RT_DBG(LOG_MINOR_ERR, MOD_HAL, "unit %d, PHY semaphore create failed!!", unit);
        return RT_ERR_FAILED;
    }

    /* Initialize MAC */
    if ((ret = hal_ctrl[unit].pChip_driver->pMacdrv->fMdrv_init(unit)) != RT_ERR_OK)
    {
        RT_ERR(ret, MOD_HAL, "mac driver init(unit=%d) failed!!", unit);
        return ret;
    }

    hal_ctrl[unit].chip_flags |= HAL_CHIP_INITED;
    return RT_ERR_OK;
} /* end of mac_init */
