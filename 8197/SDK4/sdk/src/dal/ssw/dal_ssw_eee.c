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
 * $Revision: 9198 $
 * $Date: 2010-04-26 11:58:37 +0800 (Mon, 26 Apr 2010) $
 *
 * Purpose : Definition those public EEE routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) EEE enable/disable
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
#include <osal/lib.h>
#include <osal/memory.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/ssw/dal_ssw_eee.h>
#include <rtk/default.h>
#include <rtk/eee.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
static uint32       eee_init[RTK_MAX_NUM_OF_UNIT];
static osal_mutex_t eee_sem[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Declaration
 */
#define EEE_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(eee_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_EEE|MOD_DAL), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)

#define EEE_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(eee_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_EEE|MOD_DAL), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

/*
 * Function Declaration
 */

/* Function Name:
 *      dal_ssw_eee_init
 * Description:
 *      Initialize EEE module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize EEE module before calling any EEE APIs.
 */
int32
dal_ssw_eee_init(uint32 unit)
{
    RT_DBG(LOG_DEBUG, (MOD_EEE|MOD_DAL), "unit=%d", unit);

    eee_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    eee_sem[unit] = osal_sem_mutex_create();
    if (0 == eee_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_EEE|MOD_DAL), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    eee_init[unit] = INIT_COMPLETED;

    return RT_ERR_OK;
} /* end of dal_ssw_eee_init */

/* Module Name    : EEE                */
/* Sub-module Name: EEE enable/disable */
/* Function Name:
 *      dal_ssw_eee_portEnable_get
 * Description:
 *      Get enable status of EEE function in the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of EEE
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
dal_ssw_eee_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret = RT_ERR_FAILED;

    /* check Init status */
    RT_INIT_CHK(eee_init[unit]);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "unit=%d, port=%d, pEnable=%x"
            , unit, port, pEnable);

    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    EEE_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = phy_eeeEnable_get(unit, port, pEnable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "phy_eeeEnable_get failure (ret = 0x%x) unit=%d, port=%d", ret, unit, port);
        return ret;
    }

    EEE_SEM_UNLOCK(unit);
    return ret;
} /* end of dal_ssw_eee_portEnable_get */

/* Function Name:
 *      dal_ssw_eee_portEnable_set
 * Description:
 *      Set enable status of EEE function in the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of EEE
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
dal_ssw_eee_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret = RT_ERR_FAILED;

    /* check Init status */
    RT_INIT_CHK(eee_init[unit]);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "unit=%d, port=%d, enable=%u"
            , unit, port, enable);

    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);

    EEE_SEM_LOCK(unit);

    /* set value to CHIP */
    if ((ret = phy_eeeEnable_set(unit, port, enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "phy_eeeEnable_set failure (ret = 0x%x) unit=%d, port=%d", ret, unit, port);
        return ret;
    }

    EEE_SEM_UNLOCK(unit);
    return ret;
} /* end of dal_ssw_eee_portEnable_set */
