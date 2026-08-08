/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 21568 $
 * $Date: 2011-08-26 19:16:38 +0800 (Fri, 26 Aug 2011) $
 *
 * Purpose : Definition those public L3 APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) L3 routing
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
#include <dal/esw/dal_esw_l3.h>
#include <rtk/l3.h>

/* 
 * Data Declaration 
 */
static uint32               l3_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         l3_sem[RTK_MAX_NUM_OF_UNIT];


/*
 * Macro Definition
 */
/* vlan semaphore handling */
#define L3_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(l3_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_L3), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define L3_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(l3_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_L3), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* Function Name:
 *      dal_esw_l3_init
 * Description:
 *      Initialize L3 module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize L3 module before calling any L3 APIs.
 */
int32
dal_esw_l3_init(uint32 unit)
{
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d", unit); 
    
    l3_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    l3_sem[unit] = osal_sem_mutex_create();
    if (0 == l3_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_L3|MOD_DAL), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    /* set init flag to complete init */
    l3_init[unit] = INIT_COMPLETED;
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_init*/

/* Module Name    : Layer3 routing                */
/* Sub-module Name: Layer3 routing error handling */

/* Function Name:
 *      dal_esw_l3_ttlExpireAction_get
 * Description:
 *      Get forwarding action when IP ttl expire.
 * Input:
 *      unit    - unit id
 *      type    - type of IP ttl expire
 * Output:
 *      pAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 *      
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
dal_esw_l3_ttlExpireAction_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_action_t *pAction)
{
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);
    
    L3_SEM_LOCK(unit);
    
    if(type == TTL_EXPIRE_MCAST)
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_MIPTTLDROPf, pAction)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "read register error");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_UIPTTLDROPf, pAction)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "read register error");
            return ret;
        }
    }   

    L3_SEM_UNLOCK(unit);

    if(*pAction == 0)
        *pAction = ACTION_DROP;
    else if(*pAction == 1)
        *pAction = ACTION_FORWARD;
    else
        *pAction = ACTION_TRAP2CPU;

    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "pAction=%d", *pAction);
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_ttlExpireAction_get*/

/* Function Name:
 *      dal_esw_l3_ttlExpireAction_set
 * Description:
 *      Set forwarding action when IP ttl expire.
 * Input:
 *      unit   - unit id
 *      type   - type of IP ttl expire
 *      action - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 *      RT_ERR_INPUT            - invalid TTL expire type
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 *      
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
dal_esw_l3_ttlExpireAction_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_action_t action)
{
    int32  ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d, action=%d",
           unit, action);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);

    if(action == ACTION_DROP)
        val = 0;
    else if(action == ACTION_FORWARD)
        val = 1;
    else if(action == ACTION_TRAP2CPU)
        val = 2;
    else
    {
        RT_ERR(RT_ERR_INPUT, (MOD_L3|MOD_DAL), "");
        return RT_ERR_INPUT;
    }

    L3_SEM_LOCK(unit);    

    if(type == TTL_EXPIRE_MCAST)
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_MIPTTLDROPf, &val)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "write register error");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_UIPTTLDROPf, &val)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "write register error");
            return ret;
        }
    } 

    L3_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_ttlExpireAction_set*/

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapPri_get
 * Description:
 *      Get priority of trapped packets when IP ttl expire.
 * Input:
 *      unit      - unit id
 *      type      - type of IP ttl expire
 * Output:
 *      pPriority - pointer to priority of trapped packets
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
int32
dal_esw_l3_ttlExpireTrapPri_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_pri_t *pPriority)
{
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);  

    L3_SEM_LOCK(unit);   
    
    if(type == TTL_EXPIRE_MCAST)
    {       
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_MTTLPRIf, pPriority)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "read register error");
            return ret;
        }
    }
    else
    {        
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_UTTLPRIf, pPriority)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "read register error");
            return ret;
        }
    } 

    L3_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "pPriority=%d", *pPriority);
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_ttlExpireTrapPri_get*/

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapPri_set
 * Description:
 *      Set priority of trapped packets when IP ttl expire.
 * Input:
 *      unit     - unit id
 *      type     - type of IP ttl expire
 *      priority - priority of trapped packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 *      RT_ERR_INPUT            - invalid TTL expire type
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
int32
dal_esw_l3_ttlExpireTrapPri_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_pri_t priority)
{
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d, priority=%d",
           unit, priority);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_INPUT);

    L3_SEM_LOCK(unit);    

    if(type == TTL_EXPIRE_MCAST)
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_MTTLPRIf, &priority)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }

    }
    else
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_UTTLPRIf, &priority)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }   

    L3_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_ttlExpireTrapPri_set*/

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapPriEnable_get
 * Description:
 *      Get priority enable status of trapped packets when IP ttl expire.
 * Input:
 *      unit            - unit id
 *      type            - type of IP ttl expire
 * Output:
 *      pEnable - pointer to enable status of defining trap priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
int32
dal_esw_l3_ttlExpireTrapPriEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable)
{
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);      

    L3_SEM_LOCK(unit);   
    
    if(type == TTL_EXPIRE_MCAST)
    {
         if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_DFRMTTLPRIf, pEnable)) != RT_ERR_OK)
         {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
         }
    }
    else
    {
         if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_DFRUTTLPRIf, pEnable)) != RT_ERR_OK)
         {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
         }
    } 

    L3_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_ttlExpireTrapPriEnable_get*/

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapPriEnable_set
 * Description:
 *      Set priority define status when IP ttl expire traped to cpu.
 * Input:
 *      unit            - unit id
 *      type            - type of IP ttl expire
 *      enable        -  priority status of trapped packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 *      RT_ERR_INPUT            - invalid TTL expire type
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
int32
dal_esw_l3_ttlExpireTrapPriEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable)
{
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d, enable=%d", unit, enable);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);      

    L3_SEM_LOCK(unit);   
    
    if(type == TTL_EXPIRE_MCAST)
    {
         if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_DFRMTTLPRIf, &enable)) != RT_ERR_OK)
         {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
         }
    }
    else
    {
         if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_DFRUTTLPRIf, &enable)) != RT_ERR_OK)
         {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
         }
    } 

    L3_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_ttlExpireTrapPriEnable_set*/


/* Function Name:
 *      dal_esw_l3_ttlExpireTrapDP_get
 * Description:
 *      Get drop precedence of trapped packet when IP ttl expire.
 * Input:
 *      unit - unit id
 *      type - type of IP ttl expire
 * Output:
 *      pDp  - pointer to drop precedence of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
int32
dal_esw_l3_ttlExpireTrapDP_get(uint32 unit, rtk_l3_ttlExpireType_t type, uint32 *pDp)
{
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pDp), RT_ERR_NULL_POINTER);

    L3_SEM_LOCK(unit);

    if(type == TTL_EXPIRE_MCAST)
    {       
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_MTTLDPf, pDp)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }
    else
    {       
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_UTTLDPf, pDp)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }

    L3_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "pDp=%d", *pDp);
    
    return RT_ERR_OK;
}  /*end of dal_esw_l3_ttlExpireTrapDP_get*/

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapDP_set
 * Description:
 *      Set drop precedence of trapped packet when IP ttl expire.
 * Input:
 *      unit - unit id
 *      type - type of IP ttl expire
 *      dp   - drop precedence of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
int32
dal_esw_l3_ttlExpireTrapDP_set(uint32 unit, rtk_l3_ttlExpireType_t type, uint32 dp)
{
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d, dp=%d", unit, dp);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((dp > RTK_DROP_PRECEDENCE_MAX), RT_ERR_INPUT);
    
    L3_SEM_LOCK(unit);

    if(type == TTL_EXPIRE_MCAST)
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_MTTLDPf, &dp)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_UTTLDPf, &dp)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }  

    L3_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_ttlExpireTrapDP_set*/

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapDPEnable_get
 * Description:
 *      Get drop procedence enable status of trapped packets when IP ttl expire.
 * Input:
 *      unit            - unit id
 *      type            - type of IP ttl expire
 * Output:
 *      pEnable - pointer to enable status of defining trap drop procedence
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
int32
dal_esw_l3_ttlExpireTrapDPEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable)
{
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);      

    L3_SEM_LOCK(unit);   
    
    if(type == TTL_EXPIRE_MCAST)
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_DFRMTTLDPf, pEnable)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }
    else
    {
         if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_DFRUTTLDPf, pEnable)) != RT_ERR_OK)
         {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }   

    L3_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_ttlExpireTrapDPEnable_get*/

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapDPEnable_set
 * Description:
 *      Set drop procedence define status when IP ttl expire traped to cpu.
 * Input:
 *      unit            - unit id
 *      type            - type of IP ttl expire
 *      enable        -  drop procedence status of trapped packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid drop procedence value
 *      RT_ERR_INPUT            - invalid TTL expire type
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
int32
dal_esw_l3_ttlExpireTrapDPEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable)
{
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d, enable=%d", unit, enable);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);      

    L3_SEM_LOCK(unit);   
    
    if(type == TTL_EXPIRE_MCAST)
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_DFRMTTLDPf, &enable)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }
    else
    {
         if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_DFRUTTLDPf, &enable)) != RT_ERR_OK)
         {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }   

    L3_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_ttlExpireTrapDPEnable_set*/


/* Function Name:
 *      dal_esw_l3_ttlExpireAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 *      type    - type of IP ttl expire
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_l3_ttlExpireAddCPUTagEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable)
{
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    L3_SEM_LOCK(unit);

    if(type == TTL_EXPIRE_MCAST)
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_MTTLCPUTAGf, pEnable)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }
    else
    {
         if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_UTTLCPUTAGf, pEnable)) != RT_ERR_OK)
         {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    } 

    L3_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_ttlExpireAddCPUTagEnable_get*/

/* Function Name:
 *      dal_esw_l3_ttlExpireAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
 *      type   - type of IP ttl expire
 *      enable - enable status of CPU tag adding
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
dal_esw_l3_ttlExpireAddCPUTagEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable)
{
    int32  ret;
    
    RT_DBG(LOG_DEBUG, (MOD_L3|MOD_DAL), "unit=%d, enable=%d", unit, enable);
    
    /* check Init status */
    RT_INIT_CHK(l3_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= TTL_EXPIRE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);
    
    L3_SEM_LOCK(unit);

    if(type == TTL_EXPIRE_MCAST)
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_MTTLCPUTAGf, &enable)) != RT_ERR_OK)
        {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }
    else
    {
         if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, ESW_UTTLCPUTAGf, &enable)) != RT_ERR_OK)
         {
            L3_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_L3|MOD_DAL), "");
            return ret;
        }
    }   

    L3_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /*end of dal_esw_l3_ttlExpireAddCPUTagEnable_set*/


