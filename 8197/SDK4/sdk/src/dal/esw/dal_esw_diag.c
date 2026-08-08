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
 * Purpose : Definition those public Port APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) Diag
 * 
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
#include <osal/thread.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/time.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/esw/dal_esw_diag.h>
#include <dal/esw/dal_esw_port.h>
#include <rtk/port.h>

/* 
 * Symbol Definition 
 */


/* 
 * Data Declaration 
 */
static uint32               diag_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         diag_sem[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */
/* diag semaphore handling */
#define DIAG_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(diag_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_DIAG), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define DIAG_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(diag_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_DIAG), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* 
 * Function Declaration 
 */

/* Module Name    : diagnostic     */
/* Sub-module Name: Global */

/* Function Name:
 *      dal_esw_diag_init
 * Description:
 *      Initialize diagnostic module of the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize diag module before calling any diag APIs.
 */
int32
dal_esw_diag_init(uint32 unit)
{
    diag_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    diag_sem[unit] = osal_sem_mutex_create();
    if (0 == diag_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_DIAG), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    
    /* set init flag to complete init */
    diag_init[unit] = INIT_COMPLETED;    

    return RT_ERR_OK;
}/* end of dal_esw_diag_init */

/* Function Name:
 *      dal_esw_diag_portMacRemoteLoopbackEnable_get
 * Description:
 *      Get the mac remote loopback enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the enable status of mac remote loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac remote loopback enable status of the port is as following:
 *         - DISABLE   
 *         - ENABLE
 *      2. Remote loopback is used to loopback packet RX to switch core back to the outer interface.
 */
int32
dal_esw_diag_portMacRemoteLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32  value = 0;
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DIAG), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(diag_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    DIAG_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, 
                                    ESW_ENLBKPBf, &value)) != RT_ERR_OK)
    {
        DIAG_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
        return ret;
    }

    DIAG_SEM_UNLOCK(unit);

    switch(value)
    {
        case 0:
            *pEnable = ENABLED;
            break;
        case 1:
            *pEnable = DISABLED;
            break;
        default:
            break;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DIAG), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
}/*end of dal_esw_diag_portMacRemoteLoopbackEnable_get*/

/* Function Name:
 *      dal_esw_diag_portMacRemoteLoopbackEnable_set
 * Description:
 *      Set the mac remote loopback enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of mac remote loopback
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac remote loopback enable status of the port is as following:
 *         - DISABLE   
 *         - ENABLE
 *      2. Remote loopback is used to loopback packet RX to switch core back to the outer interface.  
 */
int32
dal_esw_diag_portMacRemoteLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  value = 0;
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DIAG), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(diag_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED) && (enable != ENABLED), RT_ERR_INPUT);

    switch(enable)
    {
        case DISABLED:
            value = 1;
            break;
        case ENABLED:
            value = 0;
            break;
        default:
            RT_ERR(RT_ERR_INPUT, (MOD_DAL|MOD_DIAG), "");
            return RT_ERR_INPUT;
    }

    DIAG_SEM_LOCK(unit);

    /* Set value to Chip*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE, 
                                    ESW_ENLBKPBf, &value)) != RT_ERR_OK)
    {
        DIAG_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
        return ret;
    }

    DIAG_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_diag_portMacRemoteLoopbackEnable_set*/


/* Module Name    : Diag */
/* Sub-module Name: RTCT */

/* Function Name:
 *      dal_esw_diag_portRtctResult_get
 * Description:
 *      Get test result of RTCT.
 * Input:
 *      unit        - unit id
 *      port        - the port for retriving RTCT test result
 * Output:
 *      pRtctResult - RTCT result 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NOT_FINISH    - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT       - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      If linkType is PORT_SPEED_1000M, test result will be stored in ge_result. 
 *      If linkType is PORT_SPEED_10M or PORT_SPEED_100M, test result will be stored in fe_result.
 */
int32
dal_esw_diag_portRtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DIAG), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(diag_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pRtctResult), RT_ERR_NULL_POINTER);
    
    DIAG_SEM_LOCK(unit);        

    if (port <= 23)
    {
        if ((ret = phy_rtctResult_get(unit, port, pRtctResult)) != RT_ERR_OK)
        {
            DIAG_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
            return ret;
        }
    }
    else
    {
        /*Config Phy Register*/
        if (!HAL_IS_SERDES_PORT(unit, port))
        {
            if ((ret = phy_rtctResult_get(unit, port, pRtctResult)) != RT_ERR_OK)
            {
                DIAG_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
                return ret;
            }
        }        
    }

    DIAG_SEM_UNLOCK(unit);
   
    return RT_ERR_OK;
} /*end of dal_esw_diag_portRtctResult_get*/

/* Function Name:
 *      dal_esw_diag_portRtctEnable_set
 * Description:
 *      Start RTCT for ports. 
 *      When enable RTCT, the port won't transmit and receive normal traffic.
 * Input:
 *      unit      - unit id
 *      pPortmask - the ports for RTCT test
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
int32 
dal_esw_diag_portRtctEnable_set(uint32 unit, rtk_portmask_t *pPortmask)
{
    int32 ret = RT_ERR_FAILED;
    hal_control_t *pHal_control;    
    rtk_port_t portId;
    rtk_enable_t    enable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DIAG), "unit=%d", 
           unit); 
    
    /* check Init status */
    RT_INIT_CHK(diag_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
        
    if ((pHal_control = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    DIAG_SEM_LOCK(unit);

    /*init Port 0-23*/
    for(portId = HAL_GET_MIN_FE_PORT(unit); portId <= HAL_GET_MAX_FE_PORT(unit); portId++)
    {
        if(RTK_PORTMASK_IS_PORT_SET(*pPortmask, portId))
        {
            if ((ret = dal_esw_port_adminEnable_get(unit, portId, &enable)) != RT_ERR_OK)
            {
                DIAG_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
                return ret;
            }

            if (enable != ENABLED)
            {
                RT_ERR(ret, (MOD_DAL|MOD_DIAG), "Port %d need be enabled first", portId);
                DIAG_SEM_UNLOCK(unit);
                return RT_ERR_FAILED;
            }

            if ((ret = phy_rtct_start(unit, portId)) != RT_ERR_OK)
            {
                DIAG_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
                return ret;
            }
        }
    }

    /*init Port 24-27*/
    for(portId = HAL_GET_MIN_GE_PORT(unit); portId <= HAL_GET_MAX_GE_PORT(unit); portId++)
    {    
        if(RTK_PORTMASK_IS_PORT_SET(*pPortmask, portId))
        {
            /*Config Phy Register*/
            if (!HAL_IS_SERDES_PORT(unit, portId))
            {
                if ((ret = phy_rtct_start(unit, portId)) != RT_ERR_OK)
                {
                    DIAG_SEM_UNLOCK(unit);
                    RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
                    return ret;
                }
            }
        }  
    }

    DIAG_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_diag_portRtctEnable_set */
