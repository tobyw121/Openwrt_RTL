/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 22392 $
 * $Date: 2011-09-13 16:19:27 +0800 (Tue, 13 Sep 2011) $
 *
 * Purpose : Definition those public global APIs and its data type in the SDK.
 *

 * Feature : The file have include the following module and sub-modules
 *           1) Flow control configuration
 *           2) Egress drop configuration
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
#include <osal/lib.h>
#include <osal/memory.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <dal/esw/dal_esw_vlan.h>
#include <dal/esw/dal_esw_flowctrl.h>
#include <rtk/default.h>
#include <rtk/flowctrl.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
static uint32               flowctrl_init[RTK_MAX_NUM_OF_UNIT];
static osal_mutex_t         flowctrl_sem[RTK_MAX_NUM_OF_UNIT];

const static uint16 port_queue_tx_page_threshold0_regidx[] = 
    {ESW_PORT_QUEUE0_TX_PAGE_THRESHOLD_CONTROL0r, ESW_PORT_QUEUE1_TX_PAGE_THRESHOLD_CONTROL0r
   , ESW_PORT_QUEUE2_TX_PAGE_THRESHOLD_CONTROL0r, ESW_PORT_QUEUE3_TX_PAGE_THRESHOLD_CONTROL0r
   , ESW_PORT_QUEUE4_TX_PAGE_THRESHOLD_CONTROL0r, ESW_PORT_QUEUE5_TX_PAGE_THRESHOLD_CONTROL0r
   , ESW_PORT_QUEUE6_TX_PAGE_THRESHOLD_CONTROL0r, ESW_PORT_QUEUE7_TX_PAGE_THRESHOLD_CONTROL0r};

const static uint16 q_tx_hth_on_fieldidx[] = 
    {ESW_Q0TXHTH_ONf, ESW_Q1TXHTH_ONf
   , ESW_Q2TXHTH_ONf, ESW_Q3TXHTH_ONf
   , ESW_Q4TXHTH_ONf, ESW_Q5TXHTH_ONf
   , ESW_Q6TXHTH_ONf, ESW_Q7TXHTH_ONf};

const static uint16 port_queue_tx_page_threshold1_regidx[] = 
    {ESW_PORT_QUEUE0_TX_PAGE_THRESHOLD_CONTROL1r, ESW_PORT_QUEUE1_TX_PAGE_THRESHOLD_CONTROL1r
   , ESW_PORT_QUEUE2_TX_PAGE_THRESHOLD_CONTROL1r, ESW_PORT_QUEUE3_TX_PAGE_THRESHOLD_CONTROL1r
   , ESW_PORT_QUEUE4_TX_PAGE_THRESHOLD_CONTROL1r, ESW_PORT_QUEUE5_TX_PAGE_THRESHOLD_CONTROL1r
   , ESW_PORT_QUEUE6_TX_PAGE_THRESHOLD_CONTROL1r, ESW_PORT_QUEUE7_TX_PAGE_THRESHOLD_CONTROL1r};

const static uint16 q_tx_lth_on_fieldidx[] = 
    {ESW_Q0TXLTH_ONf, ESW_Q1TXLTH_ONf
   , ESW_Q2TXLTH_ONf, ESW_Q3TXLTH_ONf
   , ESW_Q4TXLTH_ONf, ESW_Q5TXLTH_ONf
   , ESW_Q6TXLTH_ONf, ESW_Q7TXLTH_ONf};


/*
 * Macro Declaration
 */
#define FLOWCTRL_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(flowctrl_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_FLOWCTRL|MOD_DAL), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)

#define FLOWCTRL_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(flowctrl_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_FLOWCTRL|MOD_DAL), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

/*
 * Function Declaration
 */
static int32 _dal_esw_flowctrl_init_config(uint32 unit);

/* Function Name:
 *      _dal_esw_flowctrl_init_config
 * Description:
 *      Initialize default configuration for flow control module of the specified device.
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
_dal_esw_flowctrl_init_config(uint32 unit)
{
    uint32  cpu_port = 0;
    rtk_port_t  port, max_port;
    int32   ret = RT_ERR_FAILED;

    /* Config CPU to force mode with flow control disabled */
    cpu_port = HAL_GET_CPU_PORT(unit);
    if ((ret = dal_esw_flowctrl_portEnable_set(unit, cpu_port, DISABLED)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "Configure cpu port flow control to disabled failed");
        return ret;
    }

    if ((ret = dal_esw_flowctrl_portPauseForceModeEnable_set(unit, cpu_port, ENABLED)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "Configure cpu port flow control mode to force mode failed");
        return ret;
    }

    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {        
        if (!HAL_IS_PORT_EXIST(unit, port)) 
        {
            continue;
        }

        if ((ret = dal_esw_flowctrl_egrPortDropRefCongestEnable_set(unit, port, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "Configure port egress port drop referenve congest failed");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of _dal_esw_flowctrl_init_config */

/* Function Name:
 *      rtk_flowctrl_init
 * Description:
 *      Initialize flowctrl module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
int32
dal_esw_flowctrl_init(uint32 unit)
{
    int32   ret = RT_ERR_FAILED;
    
    RT_DBG(LOG_DEBUG, (MOD_FLOWCTRL|MOD_DAL), "unit=%d", unit); 
    
    flowctrl_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    flowctrl_sem[unit] = osal_sem_mutex_create();
    if (0 == flowctrl_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_FLOWCTRL|MOD_DAL), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    flowctrl_init[unit] = INIT_COMPLETED;
    
    if ((ret = _dal_esw_flowctrl_init_config(unit)) != RT_ERR_OK)
    {
        flowctrl_init[unit] = INIT_NOT_COMPLETED;
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "Flow control default configuration init failed");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_init */

/* Module Name    : Flow Control               */
/* Sub-module Name: Flow control configuration */

/* Function Name:
 *      dal_esw_flowctrl_portEnable_get
 * Description:
 *      Get enable status of flowcontrol on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of flowcontrol
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, pEnable=%x"
            , unit, port, pEnable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_RX_FLOW_CONTROL_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_RXFCENABLEf, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DAL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_portEnable_get */

/* Function Name:
 *      dal_esw_flowctrl_portEnable_set
 * Description:
 *      Set enable status of flowcontrol on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of flowcontrol
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, enable=%u"
            , unit, port, enable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_RX_FLOW_CONTROL_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_RXFCENABLEf, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_portEnable_set */

/* Function Name:
 *      dal_esw_flowctrl_portPauseForceModeEnable_get
 * Description:
 *      Get enable status of force pause on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of force pause
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_portPauseForceModeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, pEnable=%x"
            , unit, port, pEnable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_FLOW_CONTROL_ABILITY_FORCE_MODE_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_FLOWCTRL_FORCEf, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DAL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_portPauseForceModeEnable_get */

/* Function Name:
 *      dal_esw_flowctrl_portPauseForceModeEnable_set
 * Description:
 *      Set enable status of force pause on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - nable status of force pause
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_portPauseForceModeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, enable=%u"
            , unit, port, enable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_FLOW_CONTROL_ABILITY_FORCE_MODE_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_FLOWCTRL_FORCEf, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_portPauseForceModeEnable_set */

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAction_get
 * Description:
 *      Get action of packet receive when pause on frame sent.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAction - pointer to action of packet receive
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Action of packet receive is as following
 *      - PAUSE_ON_RECEIVE
 *      - PAUSE_ON_DROP
 */
int32
dal_esw_flowctrl_pauseOnAction_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_flowctrl_pauseOnAction_t    *pAction)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, pAction=%x"
            , unit, port, pAction);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, INT_ESW_PORT_FLOW_CONTROL_ENABLE_DROP_PACKET_NUMBER_CONTROL_RTL8328
                        , port, REG_ARRAY_INDEX_NONE, ESW_FCACTIONf, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DAL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pAction = PAUSE_ON_RECEIVE;
            break;
        
        case 1:
            *pAction = PAUSE_ON_DROP;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_pauseOnAction_get */

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAction_set
 * Description:
 *      Set action of packet receive when pause on frame sent.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      action - action of packet receive
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Action of packet receive is as following
 *      - PAUSE_ON_RECEIVE
 *      - PAUSE_ON_DROP
 */
int32
dal_esw_flowctrl_pauseOnAction_set(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_flowctrl_pauseOnAction_t    action)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, action=%d"
            , unit, port, action);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (action)
    {
        case PAUSE_ON_RECEIVE:
            value = 0;
            break;
        
        case PAUSE_ON_DROP:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, INT_ESW_PORT_FLOW_CONTROL_ENABLE_DROP_PACKET_NUMBER_CONTROL_RTL8328
                        , port, REG_ARRAY_INDEX_NONE, ESW_FCACTIONf, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DAL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_pauseOnAction_set */

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAllowedPageNum_get
 * Description:
 *      Get number of allowed page when pause on frame sent.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pPageNum - pointer to number of received page
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_pauseOnAllowedPageNum_get(uint32 unit, rtk_port_t port, uint32 *pPageNum)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, pPageNum=%x"
            , unit, port, pPageNum);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPageNum), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, INT_ESW_PORT_FLOW_CONTROL_ENABLE_DROP_PACKET_NUMBER_CONTROL_RTL8328
                        , port, REG_ARRAY_INDEX_NONE, ESW_FCPAGENUMf, pPageNum)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DAL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_pauseOnAllowedPageNum_get */

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAllowedPageNum_set
 * Description:
 *      Set number of allowed page when pause on frame sent.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pageNum - number of received page
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_pauseOnAllowedPageNum_set(uint32 unit, rtk_port_t port, uint32 pageNum)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, pageNum=%d"
            , unit, port, pageNum);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pageNum > HAL_FLOWCTRL_PAUSEON_PAGE_PACKET_MAX(unit)), RT_ERR_INPUT);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, INT_ESW_PORT_FLOW_CONTROL_ENABLE_DROP_PACKET_NUMBER_CONTROL_RTL8328
                        , port, REG_ARRAY_INDEX_NONE, ESW_FCPAGENUMf, &pageNum)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DAL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_pauseOnAllowedPageNum_set */

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAllowedPktNum_get
 * Description:
 *      Get number of allowed packet when pause on frame sent.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pPktNum - pointer to number of received packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_pauseOnAllowedPktNum_get(uint32 unit, rtk_port_t port, uint32 *pPktNum)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, pPktNum=%x"
            , unit, port, pPktNum);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPktNum), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, INT_ESW_PORT_FLOW_CONTROL_ENABLE_DROP_PACKET_NUMBER_CONTROL_RTL8328
                        , port, REG_ARRAY_INDEX_NONE, ESW_FCPKTNUMf, pPktNum)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DAL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_pauseOnAllowedPktNum_get */

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAllowedPktNum_set
 * Description:
 *      Set number of allowed packet when pause on frame sent.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      pktNum - number of received packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_pauseOnAllowedPktNum_set(uint32 unit, rtk_port_t port, uint32 pktNum)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, pktNum=%d"
            , unit, port, pktNum);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pktNum > HAL_FLOWCTRL_PAUSEON_PAGE_PACKET_MAX(unit)), RT_ERR_INPUT);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, INT_ESW_PORT_FLOW_CONTROL_ENABLE_DROP_PACKET_NUMBER_CONTROL_RTL8328
                        , port, REG_ARRAY_INDEX_NONE, ESW_FCPKTNUMf, &pktNum)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_DAL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_pauseOnAllowedPktNum_set */

/* Function Name:
 *      dal_esw_flowctrl_igrSystemPauseThresh_get
 * Description:
 *      Get ingress system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - pointer to the threshold structure in the system used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_igrSystemPauseThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, pThresh=%x", unit, pThresh);
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL0r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL0r, ESW_SRXHTH_ONf, &(pThresh->highOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL0r, ESW_SRXHTH_OFFf, &(pThresh->highOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL1r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL1r, ESW_SRXLTH_ONf, &(pThresh->lowOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL1r, ESW_SRXLTH_OFFf, &(pThresh->lowOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "highOn=0x%x, highOff=0x%x\
           lowOn=0x%x, lowOff=0x%x", pThresh->highOn, pThresh->highOff, pThresh->lowOn, pThresh->lowOff);

    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_igrSystemPauseThresh_get */

/* Function Name:
 *      dal_esw_flowctrl_igrSystemPauseThresh_set
 * Description:
 *      Set ingress system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 *      pThresh - pointer to the threshold structure in the system used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_igrSystemPauseThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->highOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->highOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d \
           highOn=0x%x, highOff=0x%x, lowOn=0x%x, lowOff=0x%x", unit, pThresh->highOn, pThresh->highOff, 
           pThresh->lowOn, pThresh->lowOff);    
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL0r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL0r, ESW_SRXHTH_ONf, &(pThresh->highOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL0r, ESW_SRXHTH_OFFf, &(pThresh->highOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    /* program value to CHIP*/
    if ((ret = reg_write(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL0r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL1r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL1r, ESW_SRXLTH_ONf, &(pThresh->lowOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL1r, ESW_SRXLTH_OFFf, &(pThresh->lowOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
     /* program value to CHIP*/
    if ((ret = reg_write(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL1r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_igrSystemPauseThresh_set */

/* 
 * Macro Definition
 */
#define RT_IF_ERR_GOTO_HANDLE(op, errHandle, ret) \
    do {\
        if ((ret = (op)) != RT_ERR_OK)\
            goto errHandle;\
    } while(0)

/* Function Name:
 *      dal_esw_flowctrl_igrPortPauseThresh_get
 * Description:
 *      Get used page high/low threshold for the specified ingress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pThresh - pointer to the threshold structure for the port used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_igrPortPauseThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    RT_IF_ERR_GOTO_HANDLE(reg_array_field_read(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL0r,\
        port, REG_ARRAY_INDEX_NONE, ESW_CPAGETH_ONf, &(pThresh->highOn)), errHandle, ret);
    RT_IF_ERR_GOTO_HANDLE(reg_array_field_read(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL0r,\
        port, REG_ARRAY_INDEX_NONE, ESW_CPAGETH_OFFf, &(pThresh->highOff)), errHandle, ret);
    RT_IF_ERR_GOTO_HANDLE(reg_array_field_read(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL1r,\
        port, REG_ARRAY_INDEX_NONE, ESW_GPAGETH_ONf, &(pThresh->lowOn)), errHandle, ret);
    RT_IF_ERR_GOTO_HANDLE(reg_array_field_read(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL1r,\
        port, REG_ARRAY_INDEX_NONE, ESW_GPAGETH_OFFf, &(pThresh->lowOff)), errHandle, ret);
    
    FLOWCTRL_SEM_UNLOCK(unit);
    return RT_ERR_OK;

errHandle:
    FLOWCTRL_SEM_UNLOCK(unit);
    RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
    return ret;
} /* end of dal_esw_flowctrl_igrPortPauseThresh_get */

/* Function Name:
 *      dal_esw_flowctrl_igrPortPauseThresh_set
 * Description:
 *      Set used page high/low threshold for the specified ingress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pThresh - pointer to the threshold structure for the port used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_igrPortPauseThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret = RT_ERR_FAILED;;
    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->highOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->highOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "highOn=0x%x, highOff=0x%x, lowOn=0x%x, lowOff=0x%x",
           pThresh->highOn, pThresh->highOff, pThresh->lowOn, pThresh->lowOff);  
    
    FLOWCTRL_SEM_LOCK(unit);
    
    RT_IF_ERR_GOTO_HANDLE(reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_CPAGETH_ONf, &(pThresh->highOn)), errHandle, ret);
    RT_IF_ERR_GOTO_HANDLE(reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_CPAGETH_OFFf, &(pThresh->highOff)), errHandle, ret);
    RT_IF_ERR_GOTO_HANDLE(reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_GPAGETH_ONf, &(pThresh->lowOn)), errHandle, ret);
    RT_IF_ERR_GOTO_HANDLE(reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_GPAGETH_OFFf, &(pThresh->lowOff)), errHandle, ret);

    FLOWCTRL_SEM_UNLOCK(unit);
    return RT_ERR_OK;

errHandle:
    FLOWCTRL_SEM_UNLOCK(unit);
    RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
    return ret;
} /* end of dal_esw_flowctrl_igrPortPauseThresh_set */

/* Module Name    : Flow Control              */
/* Sub-module Name: Egress drop configuration */

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropMode_get
 * Description:
 *      Get egress drop mode on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pDropMode - pointer to egress drop mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Egress drop mode is as following
 *      - EGR_DROP_FOR_RX_AND_TX_PKTS
 *      - EGR_DROP_FOR_RX_PKTS
 *      - EGR_DROP_FOR_TX_PKTS
 *      - EGR_DROP_DISABLE
 */
int32
dal_esw_flowctrl_egrPortDropMode_get(uint32 unit, rtk_port_t port, rtk_flowctrl_egrDropMode_t *pDropMode)
{
    int32   ret;
    uint32  rxDrop, txDrop;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, pDropMode=%x"
            , unit, port, pDropMode);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pDropMode), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_EGRESS_PROPERTY_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_EN_RXEGRESSDROPf, &rxDrop)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_EGRESS_PROPERTY_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_EN_TXEGRESSDROPf, &txDrop)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    switch ((rxDrop | (txDrop << 1)))
    {
        case 0x0: /* (txDrop == 0 && rxDrop == 0) */
            *pDropMode = EGR_DROP_DISABLE;
            break;
        
        case 0x1: /* txDrop == 0 && rxDrop == 1 */
            *pDropMode = EGR_DROP_FOR_RX_PKTS;
            break;
        
        case 0x2: /* txDrop == 1 && rxDrop == 0 */
            *pDropMode = EGR_DROP_FOR_TX_PKTS;
            break;
           
        case 0x3: /* txDrop == 1 && rxDrop == 1 */
            *pDropMode = EGR_DROP_FOR_RX_AND_TX_PKTS;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrPortDropMode_get */

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropMode_set
 * Description:
 *      Set egress drop mode on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      dropMode - egress drop mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Egress drop mode is as following
 *      - EGR_DROP_FOR_RX_AND_TX_PKTS
 *      - EGR_DROP_FOR_RX_PKTS
 *      - EGR_DROP_FOR_TX_PKTS
 *      - EGR_DROP_DISABLE
 */
int32
dal_esw_flowctrl_egrPortDropMode_set(uint32 unit, rtk_port_t port, rtk_flowctrl_egrDropMode_t dropMode)
{
    int32   ret;
    uint32  rxDrop, txDrop;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, dropMode=%u"
            , unit, port, dropMode);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (dropMode)
    {
        case EGR_DROP_FOR_RX_AND_TX_PKTS: 
            rxDrop = 1;
            txDrop = 1;
            break;
        
        case EGR_DROP_FOR_RX_PKTS: 
            rxDrop = 1;
            txDrop = 0;
            break;
        
        case EGR_DROP_FOR_TX_PKTS: 
            rxDrop = 0;
            txDrop = 1;
            break;
           
        case EGR_DROP_DISABLE: 
            rxDrop = 0;
            txDrop = 0;
            break;
        default:
            return RT_ERR_INPUT;
    }
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_EGRESS_PROPERTY_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_EN_RXEGRESSDROPf, &rxDrop)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, ESW_PORT_EGRESS_PROPERTY_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_EN_TXEGRESSDROPf, &txDrop)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrPortDropMode_set */

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropForceModeEnable_get
 * Description:
 *      Get enable status of force drop mode on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of force drop mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_egrPortDropForceModeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, pEnable=%x"
            , unit, port, pEnable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_EGRESS_PROPERTY_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_EN_FORCEEGRESSDROPf, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrPortDropForceModeEnable_get */

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropForceModeEnable_set
 * Description:
 *      Set enable status of force drop mode on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of force drop mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      When enable force drop mode, this port will ignore flowcontrol result from autonegotiation
 */
int32
dal_esw_flowctrl_egrPortDropForceModeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, enable=%d"
            , unit, port, enable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_EGRESS_PROPERTY_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_EN_FORCEEGRESSDROPf, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrPortDropForceModeEnable_set */

/* Function Name:
 *      dal_esw_flowctrl_egrSystemDropThresh_get
 * Description:
 *      Get egress system drop threshold for the specified unit
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - pointer to the drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_egrSystemDropThresh_get(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, pThresh=%x"
            , unit, pThresh);
    
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_SYSTEM_PAGE_TX_USED_THRESHOLD_CONTROL0r
                        , ESW_STXHTH_ONf, &(pThresh->high))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_read(unit, ESW_SYSTEM_PAGE_TX_USED_THRESHOLD_CONTROL1r
                        , ESW_STXLTH_ONf, &(pThresh->low))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrSystemDropThresh_get */

/* Function Name:
 *      dal_esw_flowctrl_egrSystemDropThresh_set
 * Description:
 *      Set egress drop threshold for the specified egress port
 * Input:
 *      unit    - unit id
 *      pThresh - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_egrSystemDropThresh_set(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh)
{
    uint32  val;
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, pThresh=%x"
            , unit, pThresh);
    
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->high > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->low > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* set value to CHIP*/
    val = pThresh->high;
    if ((ret = reg_field_write(unit, ESW_SYSTEM_PAGE_TX_USED_THRESHOLD_CONTROL0r
                        , ESW_STXHTH_ONf, &val)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }

    val = pThresh->high + 1;
    if ((ret = reg_field_write(unit, ESW_SYSTEM_PAGE_TX_USED_THRESHOLD_CONTROL0r
                        , ESW_STXHTH_OFFf, &val)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    val = pThresh->low;
    if ((ret = reg_field_write(unit, ESW_SYSTEM_PAGE_TX_USED_THRESHOLD_CONTROL1r
                        , ESW_STXLTH_ONf, &val)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    val = pThresh->low + 1;
    if ((ret = reg_field_write(unit, ESW_SYSTEM_PAGE_TX_USED_THRESHOLD_CONTROL1r
                        , ESW_STXLTH_OFFf, &val)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrSystemDropThresh_set */

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropThresh_get
 * Description:
 *      Get egress drop threshold for the specified egress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pThresh - pointer to the drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_egrPortDropThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%u, pThresh=%x"
            , unit, port, pThresh);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_TX_PAGE_THRESHOLD_CONTROL0r, port
                        , REG_ARRAY_INDEX_NONE , ESW_TXHTH_ONf, &(pThresh->high))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_TX_PAGE_THRESHOLD_CONTROL1r, port
                        , REG_ARRAY_INDEX_NONE, ESW_TXLTH_ONf, &(pThresh->low))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrPortDropThresh_get */

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropThresh_set
 * Description:
 *      Set egress drop threshold for the specified egress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pThresh - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      1. For RTL8389/RTL8329 high and low drop threshold should be set to the same value,
 *         otherwise RT_ERR_INPUT will be returned.
 */
int32
dal_esw_flowctrl_egrPortDropThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh)
{
    int32   ret;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%u, pThresh=%x"
            , unit, port, pThresh);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->high > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->low > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_TX_PAGE_THRESHOLD_CONTROL0r, port
                        , REG_ARRAY_INDEX_NONE , ESW_TXHTH_ONf, &(pThresh->high))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, ESW_PORT_TX_PAGE_THRESHOLD_CONTROL1r, port
                        , REG_ARRAY_INDEX_NONE, ESW_TXLTH_ONf, &(pThresh->low))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrPortDropThresh_set */

/* Function Name:
 *      dal_esw_flowctrl_egrPortQueueDropThresh_get
 * Description:
 *      Get egress drop threshold for the specified egress port and egress queue
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 * Output:
 *      pThresh - pointer to the drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_PORT_ID            - invalid port id
 *      RT_ERR_QUEUE_ID           - invalid queue id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      1. For RTL8389/RTL8329, RT_ERR_CHIP_NOT_SUPPORTED will always be returned
 */
int32
dal_esw_flowctrl_egrPortQueueDropThresh_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_flowctrl_drop_thresh_t  *pThresh)
{
    int32   ret;
    uint32  maxTh_reg_idx, minTh_reg_idx;
    uint32  maxTh_field_idx, minTh_field_idx;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, queue=%d, pThresh=%x"
            , unit, port, queue, pThresh);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((queue > HAL_QUEUE_ID_MAX(unit)), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    
    maxTh_reg_idx = port_queue_tx_page_threshold0_regidx[queue];
    minTh_reg_idx = port_queue_tx_page_threshold1_regidx[queue];
    maxTh_field_idx = q_tx_hth_on_fieldidx[queue];
    minTh_field_idx = q_tx_lth_on_fieldidx[queue];
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, maxTh_reg_idx, port, REG_ARRAY_INDEX_NONE
                        , maxTh_field_idx, &(pThresh->high))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, minTh_reg_idx, port, REG_ARRAY_INDEX_NONE
                        , minTh_field_idx, &(pThresh->low))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrPortQueueDropThresh_get */

/* Function Name:
 *      dal_esw_flowctrl_egrPortQueueDropThresh_set
 * Description:
 *      Set egress drop threshold for the specified egress port and egress queue
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 *      pThresh - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_PORT_ID            - invalid port id
 *      RT_ERR_QUEUE_ID           - invalid queue id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      1. For RTL8389/RTL8329, RT_ERR_CHIP_NOT_SUPPORTED will always be returned
 */
int32
dal_esw_flowctrl_egrPortQueueDropThresh_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_flowctrl_drop_thresh_t  *pThresh)
{
    int32   ret;
    uint32  maxTh_reg_idx, minTh_reg_idx;
    uint32  maxTh_field_idx, minTh_field_idx;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, queue=%d, pThresh=%x"
            , unit, port, queue, pThresh);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((queue > HAL_QUEUE_ID_MAX(unit)), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pThresh->high > HAL_FLOWCTRL_THRESH_MAX(unit)), RT_ERR_INPUT);
    RT_PARAM_CHK((pThresh->low > HAL_FLOWCTRL_THRESH_MAX(unit)), RT_ERR_INPUT);
    
    maxTh_reg_idx = port_queue_tx_page_threshold0_regidx[queue];
    minTh_reg_idx = port_queue_tx_page_threshold1_regidx[queue];
    maxTh_field_idx = q_tx_hth_on_fieldidx[queue];
    minTh_field_idx = q_tx_lth_on_fieldidx[queue];
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, maxTh_reg_idx, port, REG_ARRAY_INDEX_NONE
                        , maxTh_field_idx, &(pThresh->high))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, minTh_reg_idx, port, REG_ARRAY_INDEX_NONE
                        , minTh_field_idx, &(pThresh->low))) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrPortQueueDropThresh_set */

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropRefCongestEnable_set
 * Description:
 *      Set enable status of refering source port congest status for egress drop
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of refering source port congest status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_egrPortDropRefCongestEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, enable=%d"
            , unit, port, enable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_INPUT);
    
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        
        case ENABLED:
            value = 1;
            break;
        
        default:
            return RT_ERR_INPUT;
    }
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_EGRESS_PROPERTY_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_EN_RXCONGESTDROPf, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrPortDropRefCongestEnable_set */

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropRefCongestEnable_get
 * Description:
 *      Get enable status of refering source port congest status for egress drop
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of refering source port congest status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_egrPortDropRefCongestEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d, port=%d, pEnable=%x"
            , unit, port, pEnable);
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_EGRESS_PROPERTY_CONTROLr
                        , port, REG_ARRAY_INDEX_NONE, ESW_EN_RXCONGESTDROPf, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        
        case 1:
            *pEnable = ENABLED;
            break;
        
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_egrPortDropRefCongestEnable_get */

/*
 * Flow Control OFF
 */

/* Function Name:
 *      dal_esw_flowctrl_igrSystemCongestThresh_get
 * Description:
 *      Get system used page high/low drop threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - pointer to the threshold structure in the public used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_igrSystemCongestThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    
    osal_memset(pThresh, 0, sizeof(rtk_flowctrl_thresh_t));
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL0r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL0r, ESW_FCOFF_SRXHTH_ONf, &(pThresh->highOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL0r, ESW_FCOFF_SRXHTH_OFFf, &(pThresh->highOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL1r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL1r, ESW_FCOFF_SRXLTH_ONf, &(pThresh->lowOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL1r, ESW_FCOFF_SRXLTH_OFFf, &(pThresh->lowOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "highOn=0x%x, highOff=0x%x\
           lowOn=0x%x, lowOff=0x%x", pThresh->highOn, pThresh->highOff, pThresh->lowOn, pThresh->lowOff);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_igrSystemCongestThresh_get */

/* Function Name:
 *      dal_esw_flowctrl_igrSystemCongestThresh_set
 * Description:
 *      Set system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 *      pThresh - pointer to the threshold structure in the public used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32
dal_esw_flowctrl_igrSystemCongestThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    int32   ret;
    uint32  value;
       
    /* check Init status */
    RT_INIT_CHK(flowctrl_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pThresh->highOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->highOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOn > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(pThresh->lowOff > HAL_FLOWCTRL_THRESH_MAX(unit), RT_ERR_OUT_OF_RANGE);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_FLOWCTRL), "unit=%d \
           highOn=0x%x, highOff=0x%x, lowOn=0x%x, lowOff=0x%x", unit, pThresh->highOn, pThresh->highOff, 
           pThresh->lowOn, pThresh->lowOff);
    
    FLOWCTRL_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL0r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL0r, ESW_FCOFF_SRXHTH_ONf, &(pThresh->highOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL0r, ESW_FCOFF_SRXHTH_OFFf, &(pThresh->highOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
     /* program value to CHIP*/
    if ((ret = reg_write(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL0r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    /* get value from CHIP*/
    if ((ret = reg_read(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL1r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL1r, ESW_FCOFF_SRXLTH_ONf, &(pThresh->lowOn), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL1r, ESW_FCOFF_SRXLTH_OFFf, &(pThresh->lowOff), &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
     /* program value to CHIP*/
    if ((ret = reg_write(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL1r, &value)) != RT_ERR_OK)
    {
        FLOWCTRL_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }
    
    FLOWCTRL_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_flowctrl_igrSystemCongestThresh_set */
