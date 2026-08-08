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
#include <hal/chipdef/cypress/rtk_cypress_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/cypress/dal_cypress_eee.h>
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
 *      dal_cypress_eee_init
 * Description:
 *      Initialize EEE module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize EEE module before calling any EEE APIs.
 */
int32
dal_cypress_eee_init(uint32 unit)
{
    int32 ret;
    uint32 val;

    RT_DBG(LOG_DEBUG, (MOD_EEE|MOD_DAL), "unit=%d", unit);

    eee_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    eee_sem[unit] = osal_sem_mutex_create();
    if (0 == eee_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_EEE|MOD_DAL), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    if(HAL_IS_ESCHIP(unit))
    {
        val = 0x15;
        if ((ret = reg_field_write(unit,
                              CYPRESS_EEEP_RX_TIMER_100M_CTRLr,
                              CYPRESS_RX_PAUSE_ON_TIMER_100Mf,
                              &val)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
            return ret;
        }

        val = 0xd;
        if ((ret = reg_field_write(unit,
                              CYPRESS_EEEP_RX_TIMER_500M_CTRL0r,
                              CYPRESS_RX_PAUSE_ON_TIMER_500Mf,
                              &val)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
            return ret;
        }


        val = 0xb;
        if ((ret = reg_field_write(unit,
                              CYPRESS_EEEP_RX_TIMER_GIGA_CTRL0r,
                              CYPRESS_RX_PAUSE_ON_TIMER_GIGAf,
                              &val)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
            return ret;
        }

    }


    val = 0x21;
    if ((ret = reg_field_write(unit,
                          CYPRESS_EEE_TX_TIMER_GELITE_CTRLr,
                          CYPRESS_TX_WAKE_TIMER_GELITEf,
                          &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    val = 0x21;
    if ((ret = reg_field_write(unit,
                          CYPRESS_EEE_TX_TIMER_GELITE_CTRLr,
                          CYPRESS_TX_PAUSE_WAKE_TIMER_GELITEf,
                          &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    val = 0x11;
    if ((ret = reg_field_write(unit,
                          CYPRESS_EEE_TX_TIMER_GIGA_CTRLr,
                          CYPRESS_TX_PAUSE_WAKE_TIMER_GEf,
                          &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    val = 0x11;
    if ((ret = reg_field_write(unit,
                          CYPRESS_EEE_TX_TIMER_10G_CTRLr,
                          CYPRESS_TX_PAUSE_WAKE_TIMER_10Gf,
                          &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    eee_init[unit] = INIT_COMPLETED;

    return RT_ERR_OK;
} /* end of dal_cypress_eee_init */

/* Module Name    : EEE                */
/* Sub-module Name: EEE enable/disable */
/* Function Name:
 *      dal_cypress_eee_portEnable_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_eee_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
    if ((ret = reg_array_field_read(unit,
                          CYPRESS_EEE_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEE_TX_ENf,
                          pEnable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "reg_array_field_read failure (ret = 0x%x) unit=%d, port=%d", ret, unit, port);
        return ret;
    }


    EEE_SEM_UNLOCK(unit);
    return ret;
} /* end of dal_cypress_eee_portEnable_get */

/* Function Name:
 *      dal_cypress_eee_portEnable_set
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
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_eee_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret = RT_ERR_FAILED;
    uint32  pollSts;

    /* check Init status */
    RT_INIT_CHK(eee_init[unit]);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "unit=%d, port=%d, enable=%u"
            , unit, port, enable);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);

    EEE_SEM_LOCK(unit);

    /* set value to CHIP */
    /* disable MAC polling */
    pollSts = 0;
    if ((ret = reg_array_field_write(unit,
            CYPRESS_SMI_PORT_POLLING_CTRLr, port, REG_ARRAY_INDEX_NONE,
            CYPRESS_SMI_POLLING_PMSKf, &pollSts)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "port %d disable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    pollSts = 1;
    ret = phy_eeeEnable_set(unit, port, enable);
    if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        /* disable MAC polling */
        reg_array_field_write(unit, CYPRESS_SMI_PORT_POLLING_CTRLr, port,
                REG_ARRAY_INDEX_NONE, CYPRESS_SMI_POLLING_PMSKf, &pollSts);

        EEE_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "phy_eeeEnable_set failure (ret = 0x%x) unit=%d, port=%d", ret, unit, port);
        return ret;
    }

    /* disable MAC polling */
    if ((ret = reg_array_field_write(unit,
            CYPRESS_SMI_PORT_POLLING_CTRLr, port, REG_ARRAY_INDEX_NONE,
            CYPRESS_SMI_POLLING_PMSKf, &pollSts)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "port %d enable MAC polling fail (0x%x)", port, ret);
        return ret;
    }

    if ((ret = reg_array_field_write(unit,
                          CYPRESS_MAC_FORCE_MODE_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEE_100M_ENf,
                          &enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit,
                          CYPRESS_MAC_FORCE_MODE_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEE_500M_ENf,
                          &enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit,
                          CYPRESS_MAC_FORCE_MODE_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEE_1000M_ENf,
                          &enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit,
                          CYPRESS_MAC_FORCE_MODE_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEE_10G_ENf,
                          &enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit,
                          CYPRESS_EEE_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEE_TX_ENf,
                          &enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit,
                          CYPRESS_EEE_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEE_RX_ENf,
                          &enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    EEE_SEM_UNLOCK(unit);
    return ret;
} /* end of dal_cypress_eee_portEnable_set */

/* Function Name:
 *      dal_cypress_eee_portState_get
 * Description:
 *      Get the EEE nego result state of a port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pState - pointer to the EEE port nego result state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_eee_portState_get(uint32 unit, rtk_port_t port, rtk_enable_t *pState)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(eee_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    EEE_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_array_field_read(unit,
                          CYPRESS_MAC_EEE_ABLTYr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEE_ABLTYf,
                          pState)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    EEE_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "*pState=%x", *pState);

    return RT_ERR_OK;
} /* end of dal_cypress_eee_portState_get */

/* Function Name:
 *      dal_cypress_eeep_portEnable_get
 * Description:
 *      Get enable status of EEEP function in the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of EEEP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_eeep_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
    if ((ret = reg_array_field_read(unit,
                          CYPRESS_EEEP_PORT_TX_EN_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEEP_TX_ENf,
                          pEnable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "reg_array_field_read failure (ret = 0x%x) unit=%d, port=%d", ret, unit, port);
        return ret;
    }

    EEE_SEM_UNLOCK(unit);
    return ret;
} /* end of dal_cypress_eeep_portEnable_get */

/* Function Name:
 *      dal_cypress_eeep_portEnable_set
 * Description:
 *      Set enable status of EEEP function in the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of EEEP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_eeep_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret = RT_ERR_FAILED;

    /* check Init status */
    RT_INIT_CHK(eee_init[unit]);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "unit=%d, port=%d, enable=%u"
            , unit, port, enable);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);

    EEE_SEM_LOCK(unit);

    /* set value to CHIP */
    ret = phy_eeepEnable_set(unit, port, enable);
    if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        EEE_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "phy_eeepEnable_set failure (ret = 0x%x) unit=%d, port=%d", ret, unit, port);
        return ret;
    }

    if ((ret = reg_array_field_write(unit,
                          CYPRESS_MAC_FORCE_MODE_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEEP_1000M_ENf,
                          &enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit,
                          CYPRESS_EEEP_PORT_TX_EN_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEEP_TX_ENf,
                          &enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit,
                          CYPRESS_EEEP_PORT_RX_EN_CTRLr,
                          port,
                          REG_ARRAY_INDEX_NONE,
                          CYPRESS_EEEP_RX_ENf,
                          &enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    EEE_SEM_UNLOCK(unit);
    return ret;
} /* end of dal_cypress_eeep_portEnable_set */
