/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 40320 $
 * $Date: 2013-06-19 16:42:11 +0800 (Wed, 19 Jun 2013) $
 *
 * Purpose : Definition those public port bandwidth control and storm control APIs and its data type
 *           in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Configuration of ingress port bandwidth control (ingress rate limit ).
 *           2) Configuration of egress port bandwidth control (egress rate limit).
 *           3) Configuration of egress queue bandwidth control (egress rate limit).
 *           4) Configuration of storm control
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
#include <dal/esw/dal_esw_rate.h>
#include <rtk/default.h>
#include <rtk/rate.h>


/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
static uint32               rate_init[RTK_MAX_NUM_OF_UNIT];
static osal_mutex_t         rate_sem[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Declaration
 */
#define RATE_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(rate_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_RATE|MOD_DAL), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)

#define RATE_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(rate_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_RATE|MOD_DAL), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


const static uint16 maxMinSchedulerLeakyBucket0_regidx[] = {ESW_PORT_Q0_MAXMIN_SCHEDULER_LEAKY_BUCKET0r,
                                                            ESW_PORT_Q1_MAXMIN_SCHEDULER_LEAKY_BUCKET0r,
                                                            ESW_PORT_Q2_MAXMIN_SCHEDULER_LEAKY_BUCKET0r,
                                                            ESW_PORT_Q3_MAXMIN_SCHEDULER_LEAKY_BUCKET0r,
                                                            ESW_PORT_Q4_MAXMIN_SCHEDULER_LEAKY_BUCKET0r,
                                                            ESW_PORT_Q5_MAXMIN_SCHEDULER_LEAKY_BUCKET0r,
                                                            ESW_PORT_Q6_MAXMIN_SCHEDULER_LEAKY_BUCKET0r,
                                                            ESW_PORT_Q7_MAXMIN_SCHEDULER_LEAKY_BUCKET0r};
const static uint16 maxMinSchedulerLeakyBucket1_regidx[] = {ESW_PORT_Q0_MAXMIN_SCHEDULER_LEAKY_BUCKET1r,
                                                            ESW_PORT_Q1_MAXMIN_SCHEDULER_LEAKY_BUCKET1r,
                                                            ESW_PORT_Q2_MAXMIN_SCHEDULER_LEAKY_BUCKET1r,
                                                            ESW_PORT_Q3_MAXMIN_SCHEDULER_LEAKY_BUCKET1r,
                                                            ESW_PORT_Q4_MAXMIN_SCHEDULER_LEAKY_BUCKET1r,
                                                            ESW_PORT_Q5_MAXMIN_SCHEDULER_LEAKY_BUCKET1r,
                                                            ESW_PORT_Q6_MAXMIN_SCHEDULER_LEAKY_BUCKET1r,
                                                            ESW_PORT_Q7_MAXMIN_SCHEDULER_LEAKY_BUCKET1r};

/*
 * Function Declaration
 */
static int32 _dal_esw_setMcastStormControlType(uint32 unit, rtk_port_t port, rtk_rate_storm_group_t storm_type);
static int32 _dal_esw_rate_init_config(uint32 unit);

/* Function Name:
 *      dal_esw_rate_init
 * Description:
 *      Initial the rate module of the specified device..
 * Input:
 *      unit - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
dal_esw_rate_init(uint32 unit)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_RATE|MOD_DAL), "unit=%d", unit); 
    
    rate_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    rate_sem[unit] = osal_sem_mutex_create();
    if (0 == rate_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_RATE|MOD_DAL), "semaphore create failed");
        return RT_ERR_FAILED;
    }
   
    rate_init[unit] = INIT_COMPLETED;

    if ((ret = _dal_esw_rate_init_config(unit)) != RT_ERR_OK)
    {
        rate_init[unit] = INIT_NOT_COMPLETED;
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_init */

/* Module Name    : Rate                                            */
/* Sub-module Name: Configuration of ingress port bandwidth control */

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlEnable_get
 * Description:
 *      Get the ingress bandwidth control status.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status of ingress bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The status of ingress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_esw_rate_igrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;  
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d",
           unit, port);    
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
       
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_INBW_ENf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    /* chip's value translate */
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
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_igrBandwidthCtrlEnable_get */

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlEnable_set
 * Description:
 *      Set the ingress bandwidth control status.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of ingress bandwidth control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *    The status of ingress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_esw_rate_igrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;  
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, enable=%d",
           unit, port, enable);    
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    /* translate to chip value */
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
      
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_INBW_ENf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);    
      
    return RT_ERR_OK;
} /* end of dal_esw_rate_igrBandwidthCtrlEnable_set */

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get the ingress bandwidth control rate.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pRate - ingress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
int32
dal_esw_rate_igrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d",
           unit, port);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_INBW_RATEf, pRate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pRate=%d", *pRate);     
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_igrBandwidthCtrlRate_get */

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set the ingress bandwidth control rate.
 * Input:
 *      unit - unit id
 *      port - port id
 *      rate - ingress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
int32
dal_esw_rate_igrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, rate=%d",
           unit, port, rate);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((rate > HAL_RATE_OF_BANDWIDTH_MAX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_INBW_RATEf, &rate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_igrBandwidthCtrlRate_set */

/* Function Name:
 *      dal_esw_rate_portIgrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get the status of ingress bandwidth control includes IFG or not on specified port.
 * Input:
 *      unit         - unit id
 *      port         - port id
 * Output:
 *      pIfg_include - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id 
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *      1. Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
int32
dal_esw_rate_portIgrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_port_t port, rtk_enable_t *pIfg_include)
{
    int32 ret;
    uint32 value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d",
           unit, port);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pIfg_include), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_THRESHOLDr, port, REG_ARRAY_INDEX_NONE, ESW_INBW_INC_IFGf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    /* translate to chip value */
    switch (value)
    {
        case 0:
            *pIfg_include = DISABLED;
            break;
        case 1:
            *pIfg_include = ENABLED;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pIfg_include=%d", *pIfg_include);     
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_portIgrBandwidthCtrlIncludeIfg_get */

/* Function Name:
 *      dal_esw_rate_portIgrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of ingress bandwidth control includes IFG or not on specified port.
 * Input:
 *      unit        - unit id
 *      port         - port id
 *      ifg_include - include IFG or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id  
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *      1. Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
int32
dal_esw_rate_portIgrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_port_t port, rtk_enable_t ifg_include)
{
    int32 ret;
    uint32 value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, ifg_include=%d",
           unit, port, ifg_include);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((ifg_include >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    /* translate to chip value */
    switch (ifg_include)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_THRESHOLDr, port, REG_ARRAY_INDEX_NONE, ESW_INBW_INC_IFGf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
} /* end of dal_esw_rate_portIgrBandwidthCtrlIncludeIfg_set */

/* Function Name:
 *      dal_esw_rate_igrBandwidthFlowctrlEnable_get
 * Description:
 *      Get enable status of flowcontrol for ingress bandwidth control on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of flowcontrol for ingress bandwidth control
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
dal_esw_rate_igrBandwidthFlowctrlEnable_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_enable_t        *pEnable)
{
    int32   ret;
    uint32  value;  
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d",
           unit, port);    
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
       
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_INBW_FC_ENf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    /* chip's value translate */
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
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_igrBandwidthFlowctrlEnable_get */

/* Function Name:
 *      dal_esw_rate_igrBandwidthFlowctrlEnable_set
 * Description:
 *      Set enable status of flowcontrol for ingress bandwidth control on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of flowcontrol for ingress bandwidth control
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
dal_esw_rate_igrBandwidthFlowctrlEnable_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_enable_t        enable)
{
    int32   ret;
    uint32  value;  
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, enable=%d",
           unit, port, enable);    
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    /* translate to chip value */
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
      
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP*/    
    if ((ret = reg_array_field_write(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_INBW_FC_ENf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);    
      
    return RT_ERR_OK;
} /* end of dal_esw_rate_igrBandwidthFlowctrlEnable_set */

/* Function Name:
 *      dal_esw_rate_igrBandwidthFlowctrlThresh_get
 * Description:
 *      Get threshold of flowcontrol for ingress bandwidth control on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pThresh - pointer to threshold of flowcontrol
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
dal_esw_rate_igrBandwidthFlowctrlThresh_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_rate_thresh_t   *pThresh)
{
    int32 ret;
    uint32 value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d",
           unit, port);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_read(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_THRESHOLDr, port, REG_ARRAY_INDEX_NONE, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
       
    if ((ret = reg_field_get(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_THRESHOLDr, ESW_INBW_FC_ONf, &pThresh->FC_On, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    if ((ret = reg_field_get(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_THRESHOLDr, ESW_INBW_FC_OFFf, &pThresh->FC_Off, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pThresh->FC_On=%d, pThresh->FC_Off=%d", pThresh->FC_On, pThresh->FC_Off);     
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_igrBandwidthFlowctrlThresh_get */

/* Function Name:
 *      dal_esw_rate_igrBandwidthFlowctrlThresh_set
 * Description:
 *      Set threshold of flowcontrol for ingress bandwidth control on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pThresh - threshold of flowcontrol
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_rate_igrBandwidthFlowctrlThresh_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_rate_thresh_t   *pThresh)
{
    int32 ret;
    uint32 value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, pThresh=%x",
           unit, port, pThresh);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pThresh->FC_On > HAL_THRESH_OF_IGR_BW_FLOWCTRL_MAX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((pThresh->FC_Off > HAL_THRESH_OF_IGR_BW_FLOWCTRL_MAX(unit)), RT_ERR_OUT_OF_RANGE);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_read(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_THRESHOLDr, port, REG_ARRAY_INDEX_NONE, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
       
    if ((ret = reg_field_set(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_THRESHOLDr, ESW_INBW_FC_ONf, &pThresh->FC_On, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    if ((ret = reg_field_set(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_THRESHOLDr, ESW_INBW_FC_OFFf, &pThresh->FC_Off, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    /* program value to CHIP*/
    if ((ret = reg_array_write(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_THRESHOLDr, port, REG_ARRAY_INDEX_NONE, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_igrBandwidthFlowctrlThresh_set */

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlFPEntry_get
 * Description:
 *      Get Fast Path entry for ingress bandwidth control on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      fpIndex  - index for fast path entry
 * Output:
 *      pFpEntry - Pointer to fast path entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_rate_igrBandwidthCtrlFPEntry_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  fpIndex,
    rtk_rate_igr_fpEntry_t *pFpEntry)
{
    int32 ret;
    uint32 value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, fpIndex=%d",
           unit, port, fpIndex);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pFpEntry), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((fpIndex > 2), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_read(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, port, fpIndex, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
       
    if ((ret = reg_field_get(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_VID_VALIDf, &pFpEntry->vlan_check, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_IVID_OVID_SELf, &pFpEntry->inner_or_outer_vlan, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_VIDf, &pFpEntry->vid, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_INNER_PRI_VALIDf, &pFpEntry->innerPri_check, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    if ((ret = reg_field_get(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_INNER_PRIf, &pFpEntry->innerPri, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_OUTER_PRI_VALIDf, &pFpEntry->outerPri_check, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_OUTER_PRIf, &pFpEntry->outerPri, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_get(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_DSCP_VALIDf, &pFpEntry->dscp_check, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    if ((ret = reg_field_get(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_DSCPf, &pFpEntry->dscp, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pFpEntry->vlan_check=%d, pFpEntry->inner_or_outer_vlan=%d, pFpEntry->vid=%d, pFpEntry->innerPri_check=%d, \
                                           pFpEntry->innerPri=%d, pFpEntry->outerPri_check=%d, pFpEntry->outerPri=%d, pFpEntry->dscp_check=%d, \
                                           pFpEntry->dscp=%d", 
                                           pFpEntry->vlan_check, pFpEntry->inner_or_outer_vlan, pFpEntry->vid, pFpEntry->innerPri_check, \
                                           pFpEntry->innerPri, pFpEntry->outerPri_check, pFpEntry->outerPri, pFpEntry->dscp_check, \
                                           pFpEntry->dscp);     
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_igrBandwidthCtrlFPEntry_get */

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlFPEntry_set
 * Description:
 *      Set Fast Path entry for ingress bandwidth control on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      fpIndex  - index for fast path entry
 *      pFpEntry - Fast path entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_rate_igrBandwidthCtrlFPEntry_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  fpIndex,
    rtk_rate_igr_fpEntry_t *pFpEntry)
{
    int32 ret;
    uint32 value;       
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, fpIndex=%d, pFpEntry->vlan_check=%d, pFpEntry->inner_or_outer_vlan=%d, pFpEntry->vid=%d, \
                                           pFpEntry->innerPri_check=%d, pFpEntry->innerPri=%d, pFpEntry->outerPri_check=%d, pFpEntry->outerPri=%d, \
                                           pFpEntry->dscp_check=%d, pFpEntry->dscp=%d", 
                                           unit, port, fpIndex, pFpEntry->vlan_check, pFpEntry->inner_or_outer_vlan, pFpEntry->vid, pFpEntry->innerPri_check, \
                                           pFpEntry->innerPri, pFpEntry->outerPri_check, pFpEntry->outerPri, pFpEntry->dscp_check, \
                                           pFpEntry->dscp);          
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pFpEntry), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((fpIndex > 2), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_read(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, port, fpIndex, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
       
    if ((ret = reg_field_set(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_VID_VALIDf, &pFpEntry->vlan_check, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_IVID_OVID_SELf, &pFpEntry->inner_or_outer_vlan, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_VIDf, &pFpEntry->vid, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_INNER_PRI_VALIDf, &pFpEntry->innerPri_check, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    if ((ret = reg_field_set(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_INNER_PRIf, &pFpEntry->innerPri, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_OUTER_PRI_VALIDf, &pFpEntry->outerPri_check, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_OUTER_PRIf, &pFpEntry->outerPri, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_DSCP_VALIDf, &pFpEntry->dscp_check, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    if ((ret = reg_field_set(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, ESW_DSCPf, &pFpEntry->dscp, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    /* program value to CHIP */    
    if ((ret = reg_array_write(unit, ESW_PORT_INGRESS_BANDWIDTH_CONTROL_SEPARATOR_ENTRYr, port, fpIndex, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);
      
    return RT_ERR_OK;
} /* end of dal_esw_rate_igrBandwidthCtrlFPEntry_set */

/* Module Name    : Rate                                           */
/* Sub-module Name: Configuration of egress port bandwidth control */

/* Function Name:
 *      dal_esw_rate_egrBandwidthCtrlEnable_get
 * Description:
 *      Get the egress bandwidth control status.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status of egress bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The status of egress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_esw_rate_egrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;  
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d",
           unit, port);    
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
       
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_MIN_LEAKY_BUCKET_ENABLEr, port, REG_ARRAY_INDEX_NONE, ESW_MINLB_ENf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    /* chip's value translate */
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
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_egrBandwidthCtrlEnable_get */

/* Function Name:
 *      dal_esw_rate_egrBandwidthCtrlEnable_set
 * Description:
 *      Set the egress bandwidth control status.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of egress bandwidth control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *    The status of egress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
int32
dal_esw_rate_egrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;  
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, enable=%d",
           unit, port, enable);    
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    /* translate to chip value */
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
      
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_MIN_LEAKY_BUCKET_ENABLEr, port, REG_ARRAY_INDEX_NONE, ESW_MINLB_ENf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);    
      
    return RT_ERR_OK;
} /* end of dal_esw_rate_egrBandwidthCtrlEnable_set */

/* Function Name:
 *      dal_esw_rate_egrBandwidthCtrlRate_get
 * Description:
 *      Get the egress bandwidth control rate.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pRate - egress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
int32
dal_esw_rate_egrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d",
           unit, port);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_BANDWITH_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PMBWRATEf, pRate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pRate=%d", *pRate);     
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_egrBandwidthCtrlRate_get */

/* Function Name:
 *      dal_esw_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set the egress bandwidth control rate.
 * Input:
 *      unit - unit id
 *      port - port id
 *      rate - egress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
int32
dal_esw_rate_egrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, rate=%d",
           unit, port, rate);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((rate > HAL_RATE_OF_BANDWIDTH_MAX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_BANDWITH_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PMBWRATEf, &rate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_egrBandwidthCtrlRate_set */

/* Function Name:
 *      dal_esw_rate_portEgrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get status of egress storm control includes IFG
 * Input:
 *      unit         - unit id
 *      port         - port id
 * Output:
 *      pIfg_include - pointer to enable status of egress storm control includes IFG
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
dal_esw_rate_portEgrBandwidthCtrlIncludeIfg_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pIfg_include)
{
    int32 ret;
    uint32 value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d",
           unit, port);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pIfg_include), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_LEAKY_BUKET_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PKTLENWITHPIFGf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    /* translate to chip value */
    switch (value)
    {
        case 0:
            *pIfg_include = DISABLED;
            break;
        case 1:
            *pIfg_include = ENABLED;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pIfg_include=%d", *pIfg_include);     
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_portEgrBandwidthCtrlIncludeIfg_get */

/* Function Name:
 *      dal_esw_rate_portEgrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set status of egress storm control includes IFG
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      ifg_include - enable status of egress storm control includes IFG
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
dal_esw_rate_portEgrBandwidthCtrlIncludeIfg_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    ifg_include)
{
    int32 ret;
    uint32 value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, ifg_include=%d",
           unit, port, ifg_include);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((ifg_include >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    /* translate to chip value */
    switch (ifg_include)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_LEAKY_BUKET_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PKTLENWITHPIFGf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
} /* end of dal_esw_rate_portEgrBandwidthCtrlIncludeIfg_set */

/* Function Name:
 *      dal_esw_rate_portEgrBandwidthCtrlBurstSize_get
 * Description:
 *      Get the egress bandwidth control burst size.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pBurst_size - egress bandwidth control burst size
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is byte.
 *    2) The maximum value of burst_size is 0xFFFF.
 */
int32
dal_esw_rate_portEgrBandwidthCtrlBurstSize_get(uint32 unit, rtk_port_t port, uint32 *pBurst_size)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pBurst_size), RT_ERR_NULL_POINTER);
    
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_LEAKY_BUKET_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_MINBKTBWHIGHTHf, pBurst_size)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pBurst_size=%d", *pBurst_size);     

    return RT_ERR_OK;
} /* end of dal_esw_rate_portEgrBandwidthCtrlBurstSize_get */

/* Function Name:
 *      dal_esw_rate_portEgrBandwidthCtrlBurstSize_set
 * Description:
 *      Set the egress bandwidth control burst size.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      burst_size - egress bandwidth control burst size
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is byte.
 *    2) The maximum value of burst_size is 0xFFFF.
 */
int32
dal_esw_rate_portEgrBandwidthCtrlBurstSize_set(uint32 unit, rtk_port_t port, uint32 burst_size)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, burst_size=%d",
           unit, port, burst_size);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((burst_size > 0xFFFF), RT_ERR_OUT_OF_RANGE);
    
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_LEAKY_BUKET_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_MINBKTBWHIGHTHf, &burst_size)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_portEgrBandwidthCtrlBurstSize_set */

/* Function Name:
 *      dal_esw_rate_egrQueueBwCtrlEnable_get
 * Description:
 *      Get enable status of egress bandwidth control on specified queue.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 * Output:
 *      pEnable - Pointer to enable status of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_rate_egrQueueBwCtrlEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable)
{
    int32   ret;
    uint32  value;  
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, queue=%d",
           unit, port, queue);    
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((queue >= HAL_MAX_NUM_OF_QUEUE(unit)), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
       
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_MAX_LEAKY_BUCKET_ENABLEr, port, queue, ESW_QMAXLB_ENf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    /* chip's value translate */
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
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_egrQueueBwCtrlEnable_get */

/* Function Name:
 *      dal_esw_rate_egrQueueBwCtrlEnable_set
 * Description:
 *      Set enable status of egress bandwidth control on specified queue.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      queue  - queue id
 *      enable - enable status of egress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_rate_egrQueueBwCtrlEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable)
{
    int32   ret;
    uint32  value;  
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, queue=%d, enable=%d",
           unit, port, queue, enable);    
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((queue >= HAL_MAX_NUM_OF_QUEUE(unit)), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    /* translate to chip value */
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
      
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_MAX_LEAKY_BUCKET_ENABLEr, port, queue, ESW_QMAXLB_ENf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);    
      
    return RT_ERR_OK;
} /* end of dal_esw_rate_egrQueueBwCtrlEnable_set */

/* Function Name:
 *      dal_esw_rate_egrQueueBwCtrlRate_get
 * Description:
 *      Get rate of egress bandwidth control on specified queue.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      queue - queue id
 * Output:
 *      pRate - pointer to rate of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8328M is 16Kbps.
 */
int32
dal_esw_rate_egrQueueBwCtrlRate_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pRate)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, queue=%d",
           unit, port, queue);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((queue >= HAL_MAX_NUM_OF_QUEUE(unit)), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, maxMinSchedulerLeakyBucket0_regidx[queue], port, REG_ARRAY_INDEX_NONE, ESW_MAXBKTAVERAGERATEf, pRate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pRate=%d", *pRate);     
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_egrQueueBwCtrlRate_get */

/* Function Name:
 *      dal_esw_rate_egrQueueBwCtrlRate_set
 * Description:
 *      Set rate of egress bandwidth control on specified queue.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      queue - queue id
 *      rate  - rate of egress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_RATE             - Invalid input rate
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8328M is 16Kbps.
 */
int32
dal_esw_rate_egrQueueBwCtrlRate_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      rate)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, queue,=%d, rate=%d",
           unit, port, queue, rate);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((rate > HAL_RATE_OF_BANDWIDTH_MAX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(queue >= HAL_MAX_NUM_OF_QUEUE(unit), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, maxMinSchedulerLeakyBucket0_regidx[queue], port, REG_ARRAY_INDEX_NONE, ESW_MAXBKTAVERAGERATEf, &rate)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_egrQueueBwCtrlRate_set */

/* Function Name:
 *      dal_esw_rate_egrPortQueueBwCtrlBurstSize_get
 * Description:
 *      Get burst size of egress bandwidth control on specified queue.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      queue       - queue id
 * Output:
 *      pBurst_size - pointer to burst size of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is KBytes.
 *    2) The maximum value of burst_size is 63. (mean 63 KBytes)
 */
int32
dal_esw_rate_egrPortQueueBwCtrlBurstSize_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pBurst_size)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, queue,=%d",
           unit, port, queue);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(queue >= HAL_MAX_NUM_OF_QUEUE(unit), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((NULL == pBurst_size), RT_ERR_NULL_POINTER);

    RATE_SEM_LOCK(unit);

    /* program value to CHIP */
    if ((ret = reg_array_field_read(unit, maxMinSchedulerLeakyBucket1_regidx[queue], port, REG_ARRAY_INDEX_NONE, ESW_MAXBKTHIGHTHf, pBurst_size)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pBurst_size=%d", *pBurst_size);     

    return RT_ERR_OK;
} /* end of dal_esw_rate_egrPortQueueBwCtrlBurstSize_get */

/* Function Name:
 *      dal_esw_rate_egrPortQueueBwCtrlBurstSize_set
 * Description:
 *      Set burst size of egress bandwidth control on specified queue.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      queue      - queue id
 *      burst_size - burst size of egress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is KBytes.
 *    2) The maximum value of burst_size is 63. (mean 63 KBytes)
 */
int32
dal_esw_rate_egrPortQueueBwCtrlBurstSize_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      burst_size)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, queue,=%d, burst_size=%d",
           unit, port, queue, burst_size);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(queue >= HAL_MAX_NUM_OF_QUEUE(unit), RT_ERR_QUEUE_ID);
    RT_PARAM_CHK((burst_size > 63), RT_ERR_OUT_OF_RANGE);

    RATE_SEM_LOCK(unit);

    /* program value to CHIP */
    if ((ret = reg_array_field_write(unit, maxMinSchedulerLeakyBucket1_regidx[queue], port, REG_ARRAY_INDEX_NONE, ESW_MAXBKTHIGHTHf, &burst_size)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_egrPortQueueBwCtrlBurstSize_set */

/* Module Name    : Rate                           */
/* Sub-module Name: Configuration of storm control */

/* Function Name:
 *      dal_esw_rate_stormControlRate_get
 * Description:
 *      Get the storm control rate.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pRate      - storm control rate (packet-per-second).
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
int32
dal_esw_rate_stormControlRate_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pRate)
{
    int32   ret;
    uint32  table_idx;
    uint32  field_idx;
    uint32  storm_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d", 
           unit, port, storm_type);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRate), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((storm_type >= STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            table_idx = ESW_TSSCR6t;
            field_idx = ESW_TSSCR6_UNUA_RATEf;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            table_idx = ESW_TSSCR3t;
            field_idx = ESW_TSSCR3_MC_RATEf;
            break;
        case STORM_GROUP_MULTICAST:
            table_idx = ESW_TSSCR3t;
            field_idx = ESW_TSSCR3_MC_RATEf;
            break;
        case STORM_GROUP_BROADCAST:
            table_idx = ESW_TSSCR0t;
            field_idx = ESW_TSSCR0_BC_RATEf;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* read value from CHIP*/
    if ((ret = table_read(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    /* get member set from storm_entry */
    if ((ret = table_field_get(unit, table_idx, field_idx, pRate, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pRate=%d", *pRate);
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlRate_get */

/* Function Name:
 *      dal_esw_rate_stormControlRate_set
 * Description:
 *      Set the storm control rate.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      rate       - storm control rate (packet-per-second).
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input bandwidth
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
int32
dal_esw_rate_stormControlRate_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  rate)
{
    int32   ret;
    uint32  table_idx;
    uint32  field_idx;
    uint32  storm_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d, rate=%d", 
           unit, port, storm_type, rate);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((storm_type >= STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((rate > HAL_RATE_OF_STORM_CONTROL_MAX(unit)), RT_ERR_RATE);
    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            table_idx = ESW_TSSCR6t;
            field_idx = ESW_TSSCR6_UNUA_RATEf;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            table_idx = ESW_TSSCR3t;
            field_idx = ESW_TSSCR3_MC_RATEf;
            break;
        case STORM_GROUP_MULTICAST:
            table_idx = ESW_TSSCR3t;
            field_idx = ESW_TSSCR3_MC_RATEf;
            break;
        case STORM_GROUP_BROADCAST:
            table_idx = ESW_TSSCR0t;
            field_idx = ESW_TSSCR0_BC_RATEf;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* Select correct traffic type */
    _dal_esw_setMcastStormControlType(unit, port, storm_type);
         
    storm_entry = 0;
    /* set member to from storm_entry */
    if ((ret = table_field_set(unit, table_idx, field_idx, &rate, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
            
    /* program value to CHIP*/
    if ((ret = table_write(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
  
    RATE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlRate_set */

/* Function Name:
 *      dal_esw_rate_stormControlEnable_get
 * Description:
 *      Get enable status of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pEnable    - pointer to enable status of storm control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
int32
dal_esw_rate_stormControlEnable_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_enable_t            *pEnable)
{
    int32   ret;
    uint32  value;
    uint32  table_idx;
    uint32  field_idx;
    uint32  storm_entry;
    uint32  mcast_type;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d", 
           unit, port, storm_type);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((storm_type >= STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            table_idx = ESW_TSSCR8t;
            field_idx = ESW_TSSCR8_UNUA_ENABLEf;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_ENABLEf;
            break;
        case STORM_GROUP_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_ENABLEf;
            break;
        case STORM_GROUP_BROADCAST:
            table_idx = ESW_TSSCR2t;
            field_idx = ESW_TSSCR2_BC_ENABLEf;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* read value from CHIP*/
    if ((ret = table_read(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    /* get member set from storm_entry */
    if ((ret = table_field_get(unit, table_idx, field_idx, &value, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);

    /* chip's value translate */
    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        case 1:
            if (storm_type == STORM_GROUP_MULTICAST)
            {
                if ((ret = table_field_get(unit, ESW_TSSCR5t, ESW_TSSCR5_MC_TYPEf, &mcast_type, &storm_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
                    return ret;
                }
                if (1 == mcast_type)
                    *pEnable = ENABLED;
                else
                    *pEnable = DISABLED;
            }
            else
            {
                *pEnable = ENABLED;
            }
            break;

        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlEnable_get */

/* Function Name:
 *      dal_esw_rate_stormControlEnable_set
 * Description:
 *      Set enable status of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      enable     - enable status of storm control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_INPUT                - invalid input parameter
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
int32
dal_esw_rate_stormControlEnable_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_enable_t            enable)
{
    int32   ret;
    uint32  value;    
    uint32  table_idx;
    uint32  field_idx;
    uint32  storm_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d, enable=%d", 
           unit, port, storm_type, enable);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((storm_type >= STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    /* translate to chip value */    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            table_idx = ESW_TSSCR8t;
            field_idx = ESW_TSSCR8_UNUA_ENABLEf;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_ENABLEf;
            break;
        case STORM_GROUP_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_ENABLEf;
            break;
        case STORM_GROUP_BROADCAST:
            table_idx = ESW_TSSCR2t;
            field_idx = ESW_TSSCR2_BC_ENABLEf;
            break;
        default:
            return RT_ERR_FAILED;
    }

    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
        
    RATE_SEM_LOCK(unit);

    /* Select correct traffic type */
    _dal_esw_setMcastStormControlType(unit, port, storm_type);
        

    /* read value from CHIP*/
    if ((ret = table_read(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    /* set member to from storm_entry */
    if ((ret = table_field_set(unit, table_idx, field_idx, &value, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
            
    /* program value to CHIP*/
    if ((ret = table_write(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
  
    RATE_SEM_UNLOCK(unit);    
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlEnable_set */

/* Function Name:
 *      dal_esw_rate_stormControlRateMode_get
 * Description:
 *      Get rate counting mode of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pRate_mode - pointer to rate counting mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *
 *    The rate mode are as following:
 *    - BASED_ON_PKT
 *    - BASED_ON_BYTE
 */
int32
dal_esw_rate_stormControlRateMode_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_rate_storm_group_t      storm_type,
    rtk_rate_storm_rateMode_t   *pRate_mode)
{
    int32   ret;
    uint32  value;
    uint32  table_idx;
    uint32  field_idx;
    uint32  storm_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d", 
           unit, port, storm_type);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pRate_mode), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((storm_type >= STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            table_idx = ESW_TSSCR8t;
            field_idx = ESW_TSSCR8_UNUA_BUNITf;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_BUNITf;
            break;
        case STORM_GROUP_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_BUNITf;
            break;
        case STORM_GROUP_BROADCAST:
            table_idx = ESW_TSSCR2t;
            field_idx = ESW_TSSCR2_BC_BUNITf;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* read value from CHIP*/
    if ((ret = table_read(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    /* get member set from storm_entry */
    if ((ret = table_field_get(unit, table_idx, field_idx, &value, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);

    /* chip's value translate */
    switch (value)
    {
        case 0:
            *pRate_mode = BASED_ON_PKT;
            break;
        case 1:
            *pRate_mode = BASED_ON_BYTE;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pRate_mode=%d", *pRate_mode);
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlRateMode_get */

/* Function Name:
 *      dal_esw_rate_stormControlRateMode_set
 * Description:
 *      Set rate counting mode of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      rate_mode  - Rate counting mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_INPUT                - invalid input parameter
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *
 *    The rate mode are as following:
 *    - BASED_ON_PKT
 *    - BASED_ON_BYTE
 */
int32
dal_esw_rate_stormControlRateMode_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_rate_storm_group_t      storm_type,
    rtk_rate_storm_rateMode_t   rate_mode)
{
    int32   ret;
    uint32  value;    
    uint32  table_idx;
    uint32  field_idx;
    uint32  storm_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d, rate_mode=%d", 
           unit, port, storm_type, rate_mode);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((storm_type >= STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((rate_mode >= STORM_RATE_MODE_END), RT_ERR_INPUT);

    /* translate to chip value */    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            table_idx = ESW_TSSCR8t;
            field_idx = ESW_TSSCR8_UNUA_BUNITf;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_BUNITf;
            break;
        case STORM_GROUP_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_BUNITf;
            break;
        case STORM_GROUP_BROADCAST:
            table_idx = ESW_TSSCR2t;
            field_idx = ESW_TSSCR2_BC_BUNITf;
            break;
        default:
            return RT_ERR_FAILED;
    }

    /* chip's value translate */
    switch (rate_mode)
    {
        case BASED_ON_PKT:
            value = 0;
            break;
        case BASED_ON_BYTE:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
        
    RATE_SEM_LOCK(unit);

    /* Select correct traffic type */
    _dal_esw_setMcastStormControlType(unit, port, storm_type);
        
    /* read value from CHIP*/
    if ((ret = table_read(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    /* set member to from storm_entry */
    if ((ret = table_field_set(unit, table_idx, field_idx, &value, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
            
    /* program value to CHIP*/
    if ((ret = table_write(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
  
    RATE_SEM_UNLOCK(unit);    
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlRateMode_set */

/* Function Name:
 *      dal_esw_rate_stormControlBurstSize_get
 * Description:
 *      Get burst rate of storm control on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      storm_type  - storm group type
 * Output:
 *      pBurst_rate - pointer to burst rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
int32
dal_esw_rate_stormControlBurstSize_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pBurst_rate)
{
    int32   ret;
    uint32  value;
    uint32  table_idx;
    uint32  field_idx;
    uint32  storm_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d", 
           unit, port, storm_type);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pBurst_rate), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((storm_type >= STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            table_idx = ESW_TSSCR8t;                        
            field_idx = ESW_TSSCR8_UNUA_BURSTf;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_BURSTf;
            break;
        case STORM_GROUP_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_BURSTf;
            break;
        case STORM_GROUP_BROADCAST:
            table_idx = ESW_TSSCR2t;
            field_idx = ESW_TSSCR2_BC_BURSTf;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* read value from CHIP*/
    if ((ret = table_read(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    /* get member set from storm_entry */
    if ((ret = table_field_get(unit, table_idx, field_idx, &value, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    RATE_SEM_UNLOCK(unit);

    *pBurst_rate = value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pBurst_rate=%d", *pBurst_rate);
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlBurstSize_get */

/* Function Name:
 *      dal_esw_rate_stormControlBurstSize_set
 * Description:
 *      Set burst rate of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      burst_rate - burst rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_INPUT                - invalid input parameter
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
int32
dal_esw_rate_stormControlBurstSize_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  burst_rate)
{
    int32   ret;
    uint32  table_idx;
    uint32  field_idx;
    uint32  storm_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d, burst_rate=%d", 
           unit, port, storm_type, burst_rate);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((storm_type >= STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((burst_rate > HAL_BURST_RATE_OF_STORM_CONTROL_MAX(unit)), RT_ERR_INPUT);

    /* translate to chip value */    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            table_idx = ESW_TSSCR8t;                        
            field_idx = ESW_TSSCR8_UNUA_BURSTf;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_BURSTf;
            break;
        case STORM_GROUP_MULTICAST:
            table_idx = ESW_TSSCR5t;
            field_idx = ESW_TSSCR5_MC_BURSTf;
            break;
        case STORM_GROUP_BROADCAST:
            table_idx = ESW_TSSCR2t;
            field_idx = ESW_TSSCR2_BC_BURSTf;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RATE_SEM_LOCK(unit);

    /* Select correct traffic type */
    _dal_esw_setMcastStormControlType(unit, port, storm_type);        
    
    /* read value from CHIP*/
    if ((ret = table_read(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
      
    /* set member to from storm_entry */
    if ((ret = table_field_set(unit, table_idx, field_idx, &burst_rate, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    /* program value to CHIP*/
    if ((ret = table_write(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
  
    RATE_SEM_UNLOCK(unit);    
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlBurstSize_set */

/* Function Name:
 *      dal_esw_rate_stormControlIncludeIfg_get
 * Description:
 *      Get enable status of includes IFG for storm control.
 * Input:
 *      unit         - unit id
 * Output:
 *      pIfg_include - pointer to enable status of includes IFG
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_rate_stormControlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include)
{
    int32 ret;
    uint32 value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d", unit);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pIfg_include), RT_ERR_NULL_POINTER);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_field_read(unit, ESW_TRAFFSTORMFILTER_GLOBAL_REGr, ESW_SCMETER_INC_IFGf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    /* translate to chip value */
    switch (value)
    {
        case 0:
            *pIfg_include = DISABLED;
            break;
        case 1:
            *pIfg_include = ENABLED;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pIfg_include=%d", *pIfg_include);     
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlIncludeIfg_get */

/* Function Name:
 *      dal_esw_rate_stormControlIncludeIfg_set
 * Description:
 *      Set enable status of includes IFG for storm control.
 * Input:
 *      unit        - unit id
 *      ifg_include - enable status of includes IFG
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_rate_stormControlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include)
{
    int32 ret;
    uint32 value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, ifg_include=%d", unit, ifg_include);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((ifg_include >= RTK_ENABLE_END), RT_ERR_INPUT);
    
    /* translate to chip value */
    switch (ifg_include)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_field_write(unit, ESW_TRAFFSTORMFILTER_GLOBAL_REGr, ESW_SCMETER_INC_IFGf, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlIncludeIfg_set */

/* Function Name:
 *      dal_esw_rate_stormControlExceed_get
 * Description:
 *      Get exceed status of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pIsExceed  - pointer to exceed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Exceed status is as following
 *      - TRUE      - storm rate is more than configured rate.
 *      - FALSE     - storm rate is never over then configured rate.
 */
int32
dal_esw_rate_stormControlExceed_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pIsExceed)
{
    int32   ret;
    uint32  value;
    uint32  table_idx;
    uint32  field_idx;
    uint32  storm_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d", 
           unit, port, storm_type);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pIsExceed), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((storm_type >= STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            table_idx = ESW_TSSCR7t;
            field_idx = ESW_TSSCR7_UNUA_EXCSTf;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            table_idx = ESW_TSSCR4t;
            field_idx = ESW_TSSCR4_MC_EXCSTf;
            break;
        case STORM_GROUP_MULTICAST:
            table_idx = ESW_TSSCR4t;
            field_idx = ESW_TSSCR4_MC_EXCSTf;
            break;
        case STORM_GROUP_BROADCAST:
            table_idx = ESW_TSSCR1t;
            field_idx = ESW_TSSCR1_BC_EXCSTf;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* read value from CHIP*/
    if ((ret = table_read(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    /* get member set from storm_entry */
    if ((ret = table_field_get(unit, table_idx, field_idx, &value, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    /* clear status by set member set to storm_entry */
    if ((ret = table_field_set(unit, table_idx, field_idx, &value, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    /* program value to CHIP*/
    if ((ret = table_write(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);

    /* chip's value translate */
    switch (value)
    {
        case 0:
            *pIsExceed = FALSE;
            break;
        case 1:
            *pIsExceed = TRUE;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pIsExceed=%d", *pIsExceed);
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlExceed_get */

/* Function Name:
 *      dal_esw_rate_stormControlExceed_reset
 * Description:
 *      Clear exceed status of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 * Note:
 *      None
 */
int32
dal_esw_rate_stormControlExceed_reset(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type)
{
    int32   ret;
    uint32  value;
    uint32  table_idx;
    uint32  field_idx;
    uint32  storm_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, port=%d, storm_type=%d", 
           unit, port, storm_type);
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((storm_type >= STORM_GROUP_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            table_idx = ESW_TSSCR7t;
            field_idx = ESW_TSSCR7_UNUA_EXCSTf;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            table_idx = ESW_TSSCR4t;
            field_idx = ESW_TSSCR4_MC_EXCSTf;
            break;
        case STORM_GROUP_MULTICAST:
            table_idx = ESW_TSSCR4t;
            field_idx = ESW_TSSCR4_MC_EXCSTf;
            break;
        case STORM_GROUP_BROADCAST:
            table_idx = ESW_TSSCR1t;
            field_idx = ESW_TSSCR1_BC_EXCSTf;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RATE_SEM_LOCK(unit);
    
    /* read value from CHIP*/
    if ((ret = table_read(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    /* clear status by set member set to storm_entry */
    value = 1;
    if ((ret = table_field_set(unit, table_idx, field_idx, &value, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    /* program value to CHIP*/
    if ((ret = table_write(unit, table_idx, port, &storm_entry)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlExceed_reset */

/* Function Name:
 *      dal_esw_rate_stormControlRefreshMode_get
 * Description:
 *      Get refresh mode of storm control.
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - pointer to refresh mode of storm control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The rate mode are as following:
 *      - BASED_ON_PKT
 *      - BASED_ON_BYTE
 */
int32
dal_esw_rate_stormControlRefreshMode_get(uint32 unit, rtk_rate_storm_rateMode_t *pMode)
{
    uint32  stormCtrlMeter_timeCnt, stormCtrlMeter_addCnt;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d", unit);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);
    
    RATE_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_field_read(unit, ESW_TRAFFSTORMFILTER_GLOBAL_REGr, ESW_SCMETER_TCNTf, &stormCtrlMeter_timeCnt)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    if ((ret = reg_field_read(unit, ESW_TRAFFSTORMFILTER_GLOBAL_REGr, ESW_SCMETER_ADCNTf, &stormCtrlMeter_addCnt)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    RATE_SEM_UNLOCK(unit);
    
    if ((0x4B == stormCtrlMeter_timeCnt) && (0x3B == stormCtrlMeter_addCnt))
        *pMode = BASED_ON_BYTE;
    else if ((0x9F == stormCtrlMeter_timeCnt) && (1 == stormCtrlMeter_addCnt))
        *pMode = BASED_ON_PKT;
    else
        return RT_ERR_FAILED;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "pMode=%d", *pMode);     
    
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlRefreshMode_get */

/* Function Name:
 *      dal_esw_rate_stormControlRefreshMode_set
 * Description:
 *      Set refresh mode of storm control.
 * Input:
 *      unit - unit id
 *      mode - refresh mode of storm control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The rate mode are as following:
 *      - BASED_ON_PKT
 *      - BASED_ON_BYTE
 */
int32
dal_esw_rate_stormControlRefreshMode_set(uint32 unit, rtk_rate_storm_rateMode_t mode)
{
    uint32  value, stormCtrlMeter_timeCnt, stormCtrlMeter_addCnt;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RATE), "unit=%d, mode=%d", unit, mode);            
    
    /* check Init status */
    RT_INIT_CHK(rate_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mode >= STORM_RATE_MODE_END), RT_ERR_INPUT);
    
    RATE_SEM_LOCK(unit);
    
    if (BASED_ON_BYTE == mode)
    {
        stormCtrlMeter_timeCnt = 0x4B;
        stormCtrlMeter_addCnt = 0x3B;
    }
    else
    {
        stormCtrlMeter_timeCnt = 0x9F;
        stormCtrlMeter_addCnt = 0x1;
    }
    /* program value to CHIP */    
    if((ret = reg_read(unit, ESW_TRAFFSTORMFILTER_GLOBAL_REGr, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    if ((ret = reg_field_set(unit, ESW_TRAFFSTORMFILTER_GLOBAL_REGr, ESW_SCMETER_TCNTf, &stormCtrlMeter_timeCnt, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    if ((ret = reg_field_set(unit, ESW_TRAFFSTORMFILTER_GLOBAL_REGr, ESW_SCMETER_ADCNTf, &stormCtrlMeter_addCnt, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    if((ret = reg_write(unit, ESW_TRAFFSTORMFILTER_GLOBAL_REGr, &value)) != RT_ERR_OK)
    {
        RATE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    RATE_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
} /* end of dal_esw_rate_stormControlRefreshMode_set */

/* Function Name:
 *      _dal_esw_setMcastStormControlType
 * Description:
 *      Select multicast traffic type that is going to apply storm control 
 * Input:
 *      unit        - unit id
 *      pVlan_entry - content of vlan entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
static int32 _dal_esw_setMcastStormControlType(uint32 unit, rtk_port_t port, rtk_rate_storm_group_t storm_type)
{
    int32   ret;
    uint32  mcast_type;
    uint32  storm_entry;
    
    RT_PARAM_CHK((storm_type != STORM_GROUP_UNKNOWN_MULTICAST && storm_type != STORM_GROUP_MULTICAST), RT_ERR_INPUT);

    /* translate to chip value */
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_MULTICAST:
            mcast_type = 0;
            break;
        case STORM_GROUP_MULTICAST:
            mcast_type = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }

    if ((ret = table_read(unit, ESW_TSSCR5t, port, &storm_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    if ((ret = table_field_set(unit, ESW_TSSCR5t, ESW_TSSCR5_MC_TYPEf, &mcast_type, &storm_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    if ((ret = table_write(unit, ESW_TSSCR5t, port, &storm_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }                        
     
    return RT_ERR_OK;
} /* end of _dal_esw_setMcastStormControlType */

/* Function Name:
 *      _dal_esw_rate_init_config
 * Description:
 *      Initialize default configuration for  the rate module of the specified device..
 * Input:
 *      unit       - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
static int32
_dal_esw_rate_init_config(uint32 unit)
{
    int32   ret;
    uint32  value = 1;
    rtk_port_t  port, max_port;
    
    max_port = HAL_GET_MAX_PORT(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_field_write(unit, ESW_TRAFFSTORMFILTER_GLOBAL_REGr, ESW_SYS_UNUA_ENABLEf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    /* program value to CHIP */    
    if ((ret = reg_field_write(unit, ESW_TRAFFSTORMFILTER_GLOBAL_REGr, ESW_SYS_MC_ENABLEf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }

    /* program value to CHIP */    
    if ((ret = reg_field_write(unit, ESW_TRAFFSTORMFILTER_GLOBAL_REGr, ESW_SYS_BC_ENABLEf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
        return ret;
    }
    
    for(port = 0; port <= max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        if ((ret = dal_esw_rate_egrBandwidthCtrlEnable_set(unit, port, RTK_DEFAULT_EGR_BANDWIDTH_CTRL_STATUS)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        }
            
        if ((ret = dal_esw_rate_egrBandwidthCtrlRate_set(unit, port, RTK_DEFAULT_EGR_BANDWIDTH_CTRL_RATE)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        }
        if ((ret = dal_esw_rate_stormControlEnable_set(unit, port, STORM_GROUP_BROADCAST, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        }
        if ((ret = dal_esw_rate_stormControlEnable_set(unit, port, STORM_GROUP_MULTICAST, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        }
        if ((ret = dal_esw_rate_stormControlEnable_set(unit, port, STORM_GROUP_UNKNOWN_MULTICAST, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        }
        if ((ret = dal_esw_rate_stormControlEnable_set(unit, port, STORM_GROUP_UNKNOWN_UNICAST, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_RATE), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of _dal_esw_rate_init_config */
