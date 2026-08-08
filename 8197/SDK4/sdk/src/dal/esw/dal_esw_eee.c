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
 * $Revision: 40305 $
 * $Date: 2013-06-19 11:39:38 +0800 (Wed, 19 Jun 2013) $
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
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <hal/common/miim.h>
#include <dal/esw/dal_esw_port.h>
#include <dal/esw/dal_esw_eee.h>
#include <rtk/default.h>
#include <rtk/eee.h>

/*
 * Symbol Definition
 */
typedef struct dal_esw_eee_info_s
{
    uint8   cfg_enable[RTK_MAX_NUM_OF_PORTS];
} dal_esw_eee_info_t;

/*
 * Data Declaration
 */
static uint32       eee_init[RTK_MAX_NUM_OF_UNIT];
static osal_mutex_t eee_sem[RTK_MAX_NUM_OF_UNIT];
static dal_esw_eee_info_t   *pEee_info[RTK_MAX_NUM_OF_UNIT];

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
static int32 _dal_esw_eee_init_config(uint32 unit);

/* Function Name:
 *      _dal_esw_eee_init_config
 * Description:
 *      Initialize default configuration for eee module of the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
static int32
_dal_esw_eee_init_config(uint32 unit)
{
    int32   ret;
    rtk_port_t  port, max_port;
    rtk_enable_t    enable;

    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {        
        if (!HAL_IS_PORT_EXIST(unit, port)) 
        {
            continue;
        }
        
        /* Config MAC */
        if (!HAL_IS_CPU_PORT(unit, port))
        {
            if ((ret = dal_esw_eee_portEnable_get(unit, port, &enable)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_EEE), "EEE init get EEE status failed");
                return ret;
            }
            pEee_info[unit]->cfg_enable[port] = enable;
        }
    }   
    
    return RT_ERR_OK;
} /* end of _dal_esw_eee_init_config */

/* Function Name:
 *      dal_esw_eee_init
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
dal_esw_eee_init(uint32 unit)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_EEE|MOD_DAL), "unit=%d", unit);

    eee_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    eee_sem[unit] = osal_sem_mutex_create();
    if (0 == eee_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_EEE|MOD_DAL), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    pEee_info[unit] = (dal_esw_eee_info_t *)osal_alloc(sizeof(dal_esw_eee_info_t));
    if (NULL == pEee_info[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_EEE), "memory allocate failed");
        return RT_ERR_FAILED;
    }        
    
    osal_memset(pEee_info[unit], 0, sizeof(dal_esw_eee_info_t));

    eee_init[unit] = INIT_COMPLETED;

    if (( ret = _dal_esw_eee_init_config(unit)) != RT_ERR_OK)
    {
        eee_init[unit] = INIT_NOT_COMPLETED;
        osal_free(pEee_info[unit]);
        pEee_info[unit] = NULL;
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "Eee default configuration init failed");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_esw_eee_init */

/* Module Name    : EEE                */
/* Sub-module Name: EEE enable/disable */
/* Function Name:
 *      dal_esw_eee_portEnable_get
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
dal_esw_eee_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
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
    if ((ret = reg_array_field_read(unit, ESW_PORT_EEE_MAC_CONTROL0r
                        , port, REG_ARRAY_INDEX_NONE, ESW_EN_TX_EEEf, pEnable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    EEE_SEM_UNLOCK(unit);
    return ret;
} /* end of dal_esw_eee_portEnable_get */

/* Function Name:
 *      dal_esw_eee_portEnable_set
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
dal_esw_eee_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
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
    if ((ret = reg_array_field_write(unit, ESW_PORT_EEE_MAC_CONTROL0r
                        , port, REG_ARRAY_INDEX_NONE, ESW_EN_TX_EEEf, &enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit, ESW_PORT_EEE_MAC_CONTROL0r
                        , port, REG_ARRAY_INDEX_NONE, ESW_EN_RX_EEEf, &enable)) != RT_ERR_OK)
    {
        EEE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_EEE), "");
        return ret;
    }

    pEee_info[unit]->cfg_enable[port] = enable;

    /* set value to CHIP */
    ret = phy_eeeEnable_set(unit, port, enable);
    if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        EEE_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EEE), "phy_eeeEnable_set failure (ret = 0x%x) unit=%d, port=%d", ret, unit, port);
        return ret;
    }

    EEE_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of dal_esw_eee_portEnable_set */

#if defined(CONFIG_SDK_WA_EEE_COMPATIBLE)
/* Function Name:
 *      dal_esw_eee_compatible_workaround
 * Description:
 *      Workaround for eee compatible problem in 8208D chip of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for eee compatible problem in 8208D chip
 */
int32
dal_esw_eee_compatible_workaround(uint32 unit)
{
    uint32  port;
    uint32  value, value1;
    rtk_port_speed_t    speed;
    rtk_port_duplex_t   duplex;
    rtk_enable_t        eee_state, autoneg_state, value2;

    for (port = 0; port < 23; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
            continue;
        /* Read twice time due to chip have latch mechanism */
        hal_miim_read(unit, port, 0, 1, &value);
        hal_miim_read(unit, port, 0, 1, &value);

        if (value & (1<<2))
        {
            if (dal_esw_port_speedDuplex_get(unit, port, &speed, &duplex) != RT_ERR_OK)
                continue;
            if (PORT_SPEED_100M == speed && PORT_HALF_DUPLEX == duplex)
            {
                if (dal_esw_eee_portEnable_get(unit, port, &eee_state) != RT_ERR_OK)
                    continue;
                if (dal_esw_port_phyAutoNegoEnable_get(unit, port, &autoneg_state) != RT_ERR_OK)
                    continue;
                if (ENABLED == eee_state && ENABLED == autoneg_state)
                {
                    value2 = DISABLED; 
                    reg_array_field_write(unit, ESW_PORT_EEE_MAC_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_EN_TX_EEEf, &value2);
                    reg_array_field_write(unit, ESW_PORT_EEE_MAC_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_EN_RX_EEEf, &value2);
                    /* Disable EEE in PHY */
                    hal_miim_read(unit, port, 4, 16, &value1);
                    value1 &= ~(0x3 << 12); /* bit13: 10M EEE, bit12: 100M EEE n-way ability */
                    hal_miim_write(unit, port, 4, 16, value1);
                    /* Restart N-Way */
                    hal_miim_read(unit, port, 0, 0, &value1);
                    hal_miim_write(unit, port, 0, 0, (value1 | (1 << 9)));
                    RT_DBG(LOG_DEBUG, (MOD_EEE|MOD_DAL), "unit=%d port %d disable EEE", unit, port);
                }
            }
            else
            {
                if (dal_esw_eee_portEnable_get(unit, port, &eee_state) != RT_ERR_OK)
                    continue;
                if (dal_esw_port_phyAutoNegoEnable_get(unit, port, &autoneg_state) != RT_ERR_OK)
                    continue;
                if (DISABLED == eee_state && ENABLED == autoneg_state && ENABLED == pEee_info[unit]->cfg_enable[port])
                {
                    value1 = ENABLED;
                    reg_array_field_write(unit, ESW_PORT_EEE_MAC_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_EN_TX_EEEf, &value1);
                    reg_array_field_write(unit, ESW_PORT_EEE_MAC_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_EN_RX_EEEf, &value1);
                    /* Enable EEE in PHY */
                    hal_miim_read(unit, port, 4, 16, &value1);
                    value1 |= (0x3 << 12); /* bit13: 10M EEE, bit12: 100M EEE n-way ability */
                    hal_miim_write(unit, port, 4, 16, value1);
                    /* Restart N-Way */
                    hal_miim_read(unit, port, 0, 0, &value1);
                    hal_miim_write(unit, port, 0, 0, (value1 | (1 << 9)));
                    RT_DBG(LOG_DEBUG, (MOD_EEE|MOD_DAL), "unit=%d port %d enable EEE", unit, port);
                }
            }
        }
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_eee_compatible_workaround */
#endif
