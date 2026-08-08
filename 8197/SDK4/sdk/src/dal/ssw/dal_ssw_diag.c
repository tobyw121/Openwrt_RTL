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
#include <hal/chipdef/ssw/rtk_ssw_table_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/chipdef/ssw/rtk_ssw_regField_list.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/ssw/dal_ssw_diag.h>
#include <dal/ssw/dal_ssw_port.h>
#include <rtk/port.h>

/* 
 * Symbol Definition 
 */


/* 
 * Data Declaration 
 */
static uint32               diag_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         diag_sem[RTK_MAX_NUM_OF_UNIT];

const static uint16 portMacControl_regidx[] = {SSW_PORT_0_1_MAC_CONTROLr, SSW_PORT_0_1_MAC_CONTROLr,
                                               SSW_PORT_2_3_MAC_CONTROLr, SSW_PORT_2_3_MAC_CONTROLr,
                                               SSW_PORT_4_5_MAC_CONTROLr, SSW_PORT_4_5_MAC_CONTROLr,
                                               SSW_PORT_6_7_MAC_CONTROLr, SSW_PORT_6_7_MAC_CONTROLr,
                                               SSW_PORT_8_9_MAC_CONTROLr, SSW_PORT_8_9_MAC_CONTROLr,
                                               SSW_PORT_10_11_MAC_CONTROLr, SSW_PORT_10_11_MAC_CONTROLr,
                                               SSW_PORT_12_13_MAC_CONTROLr, SSW_PORT_12_13_MAC_CONTROLr,
                                               SSW_PORT_14_15_MAC_CONTROLr, SSW_PORT_14_15_MAC_CONTROLr,
                                               SSW_PORT_16_17_MAC_CONTROLr, SSW_PORT_16_17_MAC_CONTROLr,
                                               SSW_PORT_18_19_MAC_CONTROLr, SSW_PORT_18_19_MAC_CONTROLr,
                                               SSW_PORT_20_21_MAC_CONTROLr, SSW_PORT_20_21_MAC_CONTROLr,
                                               SSW_PORT_22_23_MAC_CONTROLr, SSW_PORT_22_23_MAC_CONTROLr,
                                               SSW_PORT_24_25_MAC_CONTROLr, SSW_PORT_24_25_MAC_CONTROLr,
                                               SSW_PORT_26_27_MAC_CONTROLr, SSW_PORT_26_27_MAC_CONTROLr,
                                               SSW_PORT_28_MAC_CONTROLr};
const static uint16 portRLB_fieldidx[] = { SSW_EN_MAC_RLB_P0f, SSW_EN_MAC_RLB_P1f, SSW_EN_MAC_RLB_P2f, SSW_EN_MAC_RLB_P3f, SSW_EN_MAC_RLB_P4f, SSW_EN_MAC_RLB_P5f, SSW_EN_MAC_RLB_P6f, SSW_EN_MAC_RLB_P7f, SSW_EN_MAC_RLB_P8f, SSW_EN_MAC_RLB_P9f,
                                           SSW_EN_MAC_RLB_P10f, SSW_EN_MAC_RLB_P11f, SSW_EN_MAC_RLB_P12f, SSW_EN_MAC_RLB_P13f, SSW_EN_MAC_RLB_P14f, SSW_EN_MAC_RLB_P15f, SSW_EN_MAC_RLB_P16f, SSW_EN_MAC_RLB_P17f, SSW_EN_MAC_RLB_P18f, SSW_EN_MAC_RLB_P19f, 
                                           SSW_EN_MAC_RLB_P20f, SSW_EN_MAC_RLB_P21f, SSW_EN_MAC_RLB_P22f, SSW_EN_MAC_RLB_P23f, SSW_EN_MAC_RLB_P24f, SSW_EN_MAC_RLB_P25f, SSW_EN_MAC_RLB_P26f, SSW_EN_MAC_RLB_P27f, SSW_EN_MAC_RLB_P28f};
const static uint16 portLLB_fieldidx[] = { SSW_EN_MAC_LLB_P0f, SSW_EN_MAC_LLB_P1f, SSW_EN_MAC_LLB_P2f, SSW_EN_MAC_LLB_P3f, SSW_EN_MAC_LLB_P4f, SSW_EN_MAC_LLB_P5f, SSW_EN_MAC_LLB_P6f, SSW_EN_MAC_LLB_P7f, SSW_EN_MAC_LLB_P8f, SSW_EN_MAC_LLB_P9f,
                                           SSW_EN_MAC_LLB_P10f, SSW_EN_MAC_LLB_P11f, SSW_EN_MAC_LLB_P12f, SSW_EN_MAC_LLB_P13f, SSW_EN_MAC_LLB_P14f, SSW_EN_MAC_LLB_P15f, SSW_EN_MAC_LLB_P16f, SSW_EN_MAC_LLB_P17f, SSW_EN_MAC_LLB_P18f, SSW_EN_MAC_LLB_P19f, 
                                           SSW_EN_MAC_LLB_P20f, SSW_EN_MAC_LLB_P21f, SSW_EN_MAC_LLB_P22f, SSW_EN_MAC_LLB_P23f, SSW_EN_MAC_LLB_P24f, SSW_EN_MAC_LLB_P25f, SSW_EN_MAC_LLB_P26f, SSW_EN_MAC_LLB_P27f, SSW_EN_MAC_RLB_P28f};

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
 *      dal_ssw_diag_init
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
dal_ssw_diag_init(uint32 unit)
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
}/* end of dal_ssw_diag_init */

/* Function Name:
 *      dal_ssw_diag_portMacRemoteLoopbackEnable_get
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
 *         - DISABLED   
 *         - ENABLED
 *      2. Remote loopback is used to loopback packet RX to switch core back to the outer interface.
 */
int32
dal_ssw_diag_portMacRemoteLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
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
    if ((ret = reg_field_read(unit, (uint32)portMacControl_regidx[port], (uint32)portRLB_fieldidx[port], pEnable)) != RT_ERR_OK)
    {
        DIAG_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
        return ret;
    }

    DIAG_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DIAG), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
}/*end of dal_ssw_diag_portMacRemoteLoopbackEnable_get*/

/* Function Name:
 *      dal_ssw_diag_portMacRemoteLoopbackEnable_set
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
 *         - DISABLED   
 *         - ENABLED
 *      2. Remote loopback is used to loopback packet RX to switch core back to the outer interface.  
 */
int32
dal_ssw_diag_portMacRemoteLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DIAG), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(diag_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    DIAG_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, (uint32)portMacControl_regidx[port], (uint32)portRLB_fieldidx[port], &enable)) != RT_ERR_OK)
    {
        DIAG_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
        return ret;
    }

    DIAG_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /*end of dal_ssw_diag_portMacRemoteLoopbackEnable_set*/

/* Function Name:
 *      dal_ssw_diag_portMacLocalLoopbackEnable_get
 * Description:
 *      Get the mac local loopback enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the enable status of mac local loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      1. The mac local loopback enable status of the port is as following:
 *         - DISABLED
 *         - ENABLED
 *      2. Local loopback is used to loopback packet TX from switch core back to switch core.
 */
int32
dal_ssw_diag_portMacLocalLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
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
    if ((ret = reg_field_read(unit, (uint32)portMacControl_regidx[port], (uint32)portLLB_fieldidx[port], pEnable)) != RT_ERR_OK)
    {
        DIAG_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
        return ret;
    }

    DIAG_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DIAG), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
} /* end of dal_ssw_diag_portMacLocalLoopbackEnable_get */

/* Function Name:
 *      dal_ssw_diag_portMacLocalLoopbackEnable_set
 * Description:
 *      Set the mac local loopback enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of mac local loopback
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
 *      1. The mac local loopback enable status of the port is as following:
 *         - DISABLED
 *         - ENABLED
 *      2. Local loopback is used to loopback packet TX from switch core back to switch core.
 */
int32
dal_ssw_diag_portMacLocalLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DIAG), "unit=%d, port=%d, enable=%d", 
           unit, port, enable); 
    
    /* check Init status */
    RT_INIT_CHK(diag_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    DIAG_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, (uint32)portMacControl_regidx[port], (uint32)portLLB_fieldidx[port], &enable)) != RT_ERR_OK)
    {
        DIAG_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
        return ret;
    }

    DIAG_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_diag_portMacLocalLoopbackEnable_set */


/* Module Name    : Diag */
/* Sub-module Name: RTCT */

/* Module Name    : Port */
/* Sub-module Name: RTCT */

/* Function Name:
 *      dal_ssw_diag_portRtctResult_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_RTCT_NOT_FINISH   - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT      - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      If linkType is PORT_SPEED_1000M, test result will be stored in ge_result. 
 *      If linkType is PORT_SPEED_10M or PORT_SPEED_100M, test result will be stored in fe_result.
 */
int32
dal_ssw_diag_portRtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DIAG), "unit=%d, port=%d", 
           unit, port); 
    
    /* check Init status */
    RT_INIT_CHK(diag_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pRtctResult), RT_ERR_NULL_POINTER);

    osal_memset(pRtctResult, 0, sizeof(rtk_rtctResult_t));

    DIAG_SEM_LOCK(unit);

    /* Get RTCT Result */
    if ((ret = phy_rtctResult_get(unit, port, pRtctResult)) != RT_ERR_OK)
    {
        DIAG_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
        return ret;
    }
    DIAG_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /*end of dal_ssw_diag_portRtctResult_get*/

/* Function Name:
 *      dal_ssw_diag_portRtct_start
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 
dal_ssw_diag_portRtct_start(uint32 unit, rtk_portmask_t *pPortmask)
{
    rtk_port_t  port, max_port;
    int32       ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_DIAG), "unit=%d, *pPortmask=0x%x", 
           unit, *pPortmask); 
    
    /* check Init status */
    RT_INIT_CHK(diag_init[unit]);
    
    DIAG_SEM_LOCK(unit);
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        if (RTK_PORTMASK_IS_PORT_SET(*pPortmask, port))
        {
            if ((ret = phy_rtct_start(unit, port)) != RT_ERR_OK)
            {
                DIAG_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_DIAG), "");
                return ret;
            }
        }
    }
    DIAG_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_diag_portRtct_start */
