/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 22609 $
 * $Date: 2011-09-16 13:22:21 +0800 (Fri, 16 Sep 2011) $
 *
 * Purpose : Definition those public security APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Attack prevention
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
#include <dal/esw/dal_esw_sec.h>
#include <rtk/default.h>
#include <rtk/sec.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
static uint32               sec_init[RTK_MAX_NUM_OF_UNIT];
static osal_mutex_t         sec_sem[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Declaration
 */
#define SEC_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(sec_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_SEC|MOD_DAL), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)

#define SEC_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(sec_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_SEC|MOD_DAL), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/*
 * Function Declaration
 */

/* Module Name : Security */

/* Function Name:
 *      dal_esw_sec_init
 * Description:
 *      Initialize security module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize security module before calling any sec APIs.
 */
int32
dal_esw_sec_init(uint32 unit)
{
    
    RT_DBG(LOG_DEBUG, (MOD_SEC|MOD_DAL), "unit=%d", unit); 
    
    sec_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    sec_sem[unit] = osal_sem_mutex_create();
    if (0 == sec_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_SEC|MOD_DAL), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    sec_init[unit] = INIT_COMPLETED;
    
    return RT_ERR_OK;
} /* end of dal_esw_sec_init */

/* Module Name    : Security          */
/* Sub-module Name: Attack prevention */

/* Function Name:
 *      dal_esw_sec_portAttackPrevent_get
 * Description:
 *      Get action for each kind of attack on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      attack_type - type of attack
 * Output:
 *      pAction     - pointer to action for attack
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Type of attack is as following:
 *      - SYNFIN_DENY
 *      - XMA_DENY
 *      - XMA_DENY
 *      - SYN_SPORTL1024_DENY
 *      - TCPHDR_MIN_CHECK
 *      - SMURF_DENY
 *      - ICMPV6_PING_MAX_CHECK
 *      - ICMPV4_PING_MAX_CHECK
 *      - ICMP_FRAG_PKTS_DENY
 *      - IPV6_MIN_FRAG_SIZE_CHECK
 *      - POD_DENY
 *      - TCPBLAT_DENY
 *      - UDPBLAT_DENY
 *      - LAND_DENY
 *      - DAEQSA_DENY
 *
 *      Action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
int32
dal_esw_sec_portAttackPrevent_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            *pAction)
{
    int32   ret;
    uint32  value;
    uint32  field_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "unit=%d, port=%d, attack_type=%d",
           unit, port, attack_type);    
    
    /* check Init status */
    RT_INIT_CHK(sec_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((attack_type > SYNFIN_DENY), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    /* chip's value translate */
    switch (attack_type)
    {
        case SYNFIN_DENY:
            field_idx = ESW_SYNFIN_DENYf;
            break;
        case XMA_DENY:
            field_idx = ESW_XMA_DENYf;
            break;
        case NULLSCAN_DENY:
            field_idx = ESW_NULLSCAN_DENYf;
            break;
        case SYN_SPORTL1024_DENY:
            field_idx = ESW_SYN_SPORTL1024_DENYf;
            break;
        case TCPHDR_MIN_CHECK:
            field_idx = ESW_TCPHDR_MIN_ENABLEf;
            break;
        case SMURF_DENY:
            field_idx = ESW_SMURF_DENYf;
            break;
        case ICMPV6_PING_MAX_CHECK:
            field_idx = ESW_ICMPV6_PING_MAX_ENABLEf;
            break;
        case ICMPV4_PING_MAX_CHECK:
            field_idx = ESW_ICMPV4_PING_MAX_ENABLEf;
            break;   
        case ICMP_FRAG_PKTS_DENY:
            field_idx = ESW_ICMP_FRAG_PKTS_ENABLEf;
            break;
        case IPV6_MIN_FRAG_SIZE_CHECK:
            field_idx = ESW_IPV6_MIN_FRAG_SIZE_ENABLEf;
            break;
        case POD_DENY:
            field_idx = ESW_POD_DENYf;
            break;
        case TCPBLAT_DENY:
            field_idx = ESW_TCPBLAT_DENYf;
            break;
        case UDPBLAT_DENY:
            field_idx = ESW_UDPBLAT_DENYf;
            break;
        case LAND_DENY:
            field_idx = ESW_LAND_DENYf;
            break;
        case DAEQSA_DENY:
            field_idx = ESW_DAEQSA_DENYf;
            break;                
        default:
            return RT_ERR_FAILED;
    }
     
    SEC_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_ATTACK_PREVENTION_CONTROLr, port, REG_ARRAY_INDEX_NONE, field_idx, &value)) != RT_ERR_OK)
    {
        SEC_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }
    SEC_SEM_UNLOCK(unit);
    
    /* chip's value translate */
    switch (value)
    {
        case 0:
            *pAction = ACTION_FORWARD;
            break;
        case 1:
            *pAction = ACTION_DROP;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "pAction=%d", *pAction); 
    
    return RT_ERR_OK;
} /* end of dal_esw_sec_portAttackPrevent_get */

/* Function Name:
 *      dal_esw_sec_portAttackPrevent_set
 * Description:
 *      Set action for each kind of attack on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      attack_type - type of attack
 *      action      - action for attack
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Type of attack is as following:
 *      - SYNFIN_DENY
 *      - XMA_DENY
 *      - XMA_DENY
 *      - SYN_SPORTL1024_DENY
 *      - TCPHDR_MIN_CHECK
 *      - SMURF_DENY
 *      - ICMPV6_PING_MAX_CHECK
 *      - ICMPV4_PING_MAX_CHECK
 *      - ICMP_FRAG_PKTS_DENY
 *      - IPV6_MIN_FRAG_SIZE_CHECK
 *      - POD_DENY
 *      - TCPBLAT_DENY
 *      - UDPBLAT_DENY
 *      - LAND_DENY
 *      - DAEQSA_DENY
 *
 *      Action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
int32
dal_esw_sec_portAttackPrevent_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            action)
{
    int32   ret;
    uint32  value;
    uint32  field_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "unit=%d, port=%d, attack_type=%d, action=%d",
           unit, port, attack_type, action);    
    
    /* check Init status */
    RT_INIT_CHK(sec_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((attack_type > SYNFIN_DENY), RT_ERR_INPUT);
    RT_PARAM_CHK((action > ACTION_DROP), RT_ERR_INPUT);

    /* chip's value translate */
    switch (attack_type)
    {
        case SYNFIN_DENY:
            field_idx = ESW_SYNFIN_DENYf;
            break;
        case XMA_DENY:
            field_idx = ESW_XMA_DENYf;
            break;
        case NULLSCAN_DENY:
            field_idx = ESW_NULLSCAN_DENYf;
            break;
        case SYN_SPORTL1024_DENY:
            field_idx = ESW_SYN_SPORTL1024_DENYf;
            break;
        case TCPHDR_MIN_CHECK:
            field_idx = ESW_TCPHDR_MIN_ENABLEf;
            break;
        case SMURF_DENY:
            field_idx = ESW_SMURF_DENYf;
            break;
        case ICMPV6_PING_MAX_CHECK:
            field_idx = ESW_ICMPV6_PING_MAX_ENABLEf;
            break;
        case ICMPV4_PING_MAX_CHECK:
            field_idx = ESW_ICMPV4_PING_MAX_ENABLEf;
            break;   
        case ICMP_FRAG_PKTS_DENY:
            field_idx = ESW_ICMP_FRAG_PKTS_ENABLEf;
            break;
        case IPV6_MIN_FRAG_SIZE_CHECK:
            field_idx = ESW_IPV6_MIN_FRAG_SIZE_ENABLEf;
            break;
        case POD_DENY:
            field_idx = ESW_POD_DENYf;
            break;
        case TCPBLAT_DENY:
            field_idx = ESW_TCPBLAT_DENYf;
            break;
        case UDPBLAT_DENY:
            field_idx = ESW_UDPBLAT_DENYf;
            break;
        case LAND_DENY:
            field_idx = ESW_LAND_DENYf;
            break;
        case DAEQSA_DENY:
            field_idx = ESW_DAEQSA_DENYf;
            break;                
        default:
            return RT_ERR_FAILED;
    }
    
    /* chip's value translate */
    switch (action)
    {
        case ACTION_FORWARD:
            value = 0;
            break;
        case ACTION_DROP:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }
     
    SEC_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_ATTACK_PREVENTION_CONTROLr, port, REG_ARRAY_INDEX_NONE, field_idx, &value)) != RT_ERR_OK)
    {
        SEC_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }
    SEC_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_sec_portAttackPrevent_set */

/* Function Name:
 *      dal_esw_sec_portMinIPv6FragLen_get
 * Description:
 *      Get minimum length of IPv6 fragments on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLength - pointer to minimum length of IPv6 fragments
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
dal_esw_sec_portMinIPv6FragLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "unit=%d, port=%d", unit, port);    
    
    /* check Init status */
    RT_INIT_CHK(sec_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pLength), RT_ERR_NULL_POINTER);
        
    SEC_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_DOS_LENGTH_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_MIN_IPV6_FRAG_LENGTHf, pLength)) != RT_ERR_OK)
    {
        SEC_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }
    SEC_SEM_UNLOCK(unit);
       
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "pLength=%d", *pLength); 
    
    return RT_ERR_OK;
} /* end of dal_esw_sec_portMinIPv6FragLen_get */

/* Function Name:
 *      dal_esw_sec_portMinIPv6FragLen_set
 * Description:
 *      Set minimum length of IPv6 fragments on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      length - minimum length of IPv6 fragments
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
dal_esw_sec_portMinIPv6FragLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "unit=%d, port=%d, length=%d", unit, port, length);    
  
    /* check Init status */
    RT_INIT_CHK(sec_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((length > HAL_SEC_MINIPV6FRAGLEN_MAX(unit)), RT_ERR_OUT_OF_RANGE);
        
    SEC_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_DOS_LENGTH_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_MIN_IPV6_FRAG_LENGTHf, &length)) != RT_ERR_OK)
    {
        SEC_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }
    SEC_SEM_UNLOCK(unit);
       
    return RT_ERR_OK;
} /* end of dal_esw_sec_portMinIPv6FragLen_set */

/* Function Name:
 *      dal_esw_sec_portMaxPingLen_get
 * Description:
 *      Get maximum length of ICMP packet on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLength - pointer to maximum length of ICMP packet
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
dal_esw_sec_portMaxPingLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "unit=%d, port=%d", unit, port);    
    
    /* check Init status */
    RT_INIT_CHK(sec_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pLength), RT_ERR_NULL_POINTER);
        
    SEC_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_DOS_LENGTH_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_MAX_PING_PKT_LENGTHf, pLength)) != RT_ERR_OK)
    {
        SEC_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }
    SEC_SEM_UNLOCK(unit);
       
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "pLength=%d", *pLength); 
    
    return RT_ERR_OK;
} /* end of dal_esw_sec_portMaxPingLen_get */

/* Function Name:
 *      dal_esw_sec_portMaxPingLen_set
 * Description:
 *      Set maximum length of ICMP packet on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      length - maximum length of ICMP packet
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
dal_esw_sec_portMaxPingLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "unit=%d, port=%d, length=%d", unit, port, length);    
    
    /* check Init status */
    RT_INIT_CHK(sec_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((length > HAL_SEC_MAXPINGLEN_MAX(unit)), RT_ERR_OUT_OF_RANGE);
        
    SEC_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_DOS_LENGTH_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_MAX_PING_PKT_LENGTHf, &length)) != RT_ERR_OK)
    {
        SEC_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }
    SEC_SEM_UNLOCK(unit);
       
    return RT_ERR_OK;
} /* end of dal_esw_sec_portMaxPingLen_set */

/* Function Name:
 *      dal_esw_sec_portMinTCPHdrLen_get
 * Description:
 *      Get minimum length of TCP header on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLength - pointer to minimum length of TCP header
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
dal_esw_sec_portMinTCPHdrLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "unit=%d, port=%d", unit, port);    
    
    /* check Init status */
    RT_INIT_CHK(sec_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pLength), RT_ERR_NULL_POINTER);
        
    SEC_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_DOS_LENGTH_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_MIN_TCPHDR_SIZEf, pLength)) != RT_ERR_OK)
    {
        SEC_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }
    SEC_SEM_UNLOCK(unit);
       
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "pLength=%d", *pLength); 
    
    return RT_ERR_OK;
} /* end of dal_esw_sec_portMinTCPHdrLen_get */

/* Function Name:
 *      dal_esw_sec_portMinTCPHdrLen_set
 * Description:
 *      Set minimum length of TCP header on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      length - minimum length of TCP header
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
dal_esw_sec_portMinTCPHdrLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "unit=%d, port=%d, length=%d", unit, port, length);    
    
    /* check Init status */
    RT_INIT_CHK(sec_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((length > 0xff), RT_ERR_OUT_OF_RANGE);
        
    SEC_SEM_LOCK(unit);
    
    /* program value to CHIP */    
    if ((ret = reg_array_field_write(unit, ESW_PORT_DOS_LENGTH_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_MIN_TCPHDR_SIZEf, &length)) != RT_ERR_OK)
    {
        SEC_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }
    SEC_SEM_UNLOCK(unit);
       
    return RT_ERR_OK;
} /* end of dal_esw_sec_portMinTCPHdrLen_set */

/* Function Name:
 *      dal_esw_sec_portSmurfNetmaskLen_get
 * Description:
 *      Get netmask length for preventing SMURF attack on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLength - pointer to netmask length
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
dal_esw_sec_portSmurfNetmaskLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "unit=%d, port=%d", unit, port);    
    
    /* check Init status */
    RT_INIT_CHK(sec_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pLength), RT_ERR_NULL_POINTER);
        
    SEC_SEM_LOCK(unit);
    
    /* get value from CHIP */    
    if ((ret = reg_array_field_read(unit, ESW_PORT_DOS_LENGTH_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_SMURF_NETMASK_LENGTHf, pLength)) != RT_ERR_OK)
    {
        SEC_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }
    SEC_SEM_UNLOCK(unit);
  
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "pLength=%d", *pLength); 
    
    return RT_ERR_OK;
} /* end of dal_esw_sec_portSmurfNetmaskLen_get */

/* Function Name:
 *      dal_esw_sec_portSmurfNetmaskLen_set
 * Description:
 *      Set netmask length for preventing SMURF attack on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      length - netmask length
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
dal_esw_sec_portSmurfNetmaskLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SEC), "unit=%d, port=%d, length=%d", unit, port, length);    
    
    /* check Init status */
    RT_INIT_CHK(sec_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((length > HAL_SEC_SMURFNETMASKLEN_MAX(unit)), RT_ERR_OUT_OF_RANGE);
        
    SEC_SEM_LOCK(unit);
    
    /* program value to CHIP */          
    if ((ret = reg_array_field_write(unit, ESW_PORT_DOS_LENGTH_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_SMURF_NETMASK_LENGTHf, &length)) != RT_ERR_OK)
    {
        SEC_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SEC), "");
        return ret;
    }
    SEC_SEM_UNLOCK(unit);
       
    return RT_ERR_OK;
} /* end of dal_esw_sec_portSmurfNetmaskLen_set */
