
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
 * $Revision: 21577 $
 * $Date: 2011-08-27 12:02:46 +0800 (Sat, 27 Aug 2011) $
 *
 * Purpose : Definition those public global APIs and its data type in the SDK.
 *
 * Feature :  Parameter settings for the system-wise view 
 *
 */

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/debug/rt_log.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/ssw/rtk_ssw_table_struct.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <dal/ssw/dal_ssw_switch.h>
#include <rtk/default.h>
#include <rtk/switch.h>
/* 
 * Symbol Definition 
 */

/* 
 * Data Declaration 
 */
static uint32               switch_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         switch_sem[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */
/* trap semaphore handling */
#define SWITCH_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(switch_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_SWITCH), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define SWITCH_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(switch_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_SWITCH), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

/*
 * Function Declaration
 */

/* Function Name:
 *      dal_ssw_switch_init
 * Description:
 *      Initialize switch module of the specified device.
 * Input:
 *      unit          - unit id
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
dal_ssw_switch_init(uint32 unit)
{
    switch_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    switch_sem[unit] = osal_sem_mutex_create();
    if (0 == switch_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_SWITCH), "semaphore create failed");
        return RT_ERR_FAILED;
    }
        
    /* set init flag to complete init */
    switch_init[unit] = INIT_COMPLETED;
    
    return RT_ERR_OK;
} /* end of dal_ssw_switch_init */

/* Function Name:
 *      dal_ssw_switch_maxPktLen_get
 * Description:
 *      Get the max packet length setting of the specific unit
 * Input:
 *      unit                  - unit id
 * Output:
 *      pLen                 - pointer to the max packet length
 * Return:
 *      RT_ERR_OK 
 *      RT_ERR_FAILED  
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer 
 * Note:
 *      Max packet length setting
 *        - MAXPKTLEN_1522B  
 *        - MAXPKTLEN_1536B  
 *        - MAXPKTLEN_1552B  
 *        - MAXPKTLEN_9216B  
 */
int32
dal_ssw_switch_maxPktLen_get(uint32 unit, rtk_switch_maxPktLen_t *pLen)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pLen), RT_ERR_NULL_POINTER);
    
    value = 0;
    SWITCH_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, SSW_GLOBAL_MAC_CONTROL1r, SSW_SEL_MAX_LENf, &value)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }
    SWITCH_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0x00:
            *pLen = MAXPKTLEN_1522B;
            break;
        case 0x01:
            *pLen = MAXPKTLEN_1536B;
            break;
        case 0x02:
            *pLen = MAXPKTLEN_1552B;
            break;
        case 0x03:
            *pLen = MAXPKTLEN_9216B;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "pLen=%x", *pLen); 
    
    return RT_ERR_OK;
} /* end of dal_ssw_switch_maxPktLen_get */

/* Function Name:
 *      dal_ssw_switch_maxPktLen_set
 * Description:
 *      Set the max packet length of the specific unit
 * Input:
 *      unit           - unit id
 *      len            - max packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 
 *      RT_ERR_FAILED  
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - invalid enum packet length
 * Note:
 *      Max packet length setting
 *        - MAXPKTLEN_1522B
 *        - MAXPKTLEN_1536B
 *        - MAXPKTLEN_1552B
 *        - MAXPKTLEN_9216B
 */
int32
dal_ssw_switch_maxPktLen_set(uint32 unit, rtk_switch_maxPktLen_t len)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, len=%d", unit, len); 
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(len >= MAXPKTLEN_END, RT_ERR_OUT_OF_RANGE);
    
    switch (len)
    {
        case MAXPKTLEN_1522B:
            value = 0x00;
            break;
        case MAXPKTLEN_1536B:
            value = 0x01;
            break;
        case MAXPKTLEN_1552B:
            value = 0x02;
            break;
        case MAXPKTLEN_9216B:
            value = 0x03;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    SWITCH_SEM_LOCK(unit);
    /* program value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_GLOBAL_MAC_CONTROL1r, SSW_SEL_MAX_LENf, &value)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }
    SWITCH_SEM_UNLOCK(unit);
    
    
    return RT_ERR_OK;
} /* end of dal_ssw_switch_maxPktLen_set */

/* Function Name:
 *      dal_ssw_switch_mgmtMacAddr_get
 * Description:
 *      Get Mac address of switch.
 * Input:
 *      unit - unit id
 * Output:
 *      pMac - pointer to Mac address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *  
 */
int32
dal_ssw_switch_mgmtMacAddr_get(uint32 unit, rtk_mac_t *pMac)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d", unit);     
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pMac), RT_ERR_NULL_POINTER);    

    SWITCH_SEM_LOCK(unit);    

    if((ret = reg_read(unit, SSW_SWITCH_MAC_ADDRESS0r, &val)) != RT_ERR_OK)    
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }
    pMac->octet[0] = (val >> 24) & 0xff;
    pMac->octet[1] = (val >> 16) & 0xff;
    pMac->octet[2] = (val >> 8) & 0xff;
    pMac->octet[3] = (val >> 0) & 0xff;

    if((ret = reg_read(unit, SSW_SWITCH_MAC_ADDRESS1r, &val)) != RT_ERR_OK)    
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }
    pMac->octet[4] = (val >> 24) & 0xff;
    pMac->octet[5] = (val >> 16) & 0xff;

    SWITCH_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x",
           pMac->octet[0], pMac->octet[1], pMac->octet[2],
           pMac->octet[3], pMac->octet[4], pMac->octet[5]); 
    
    return RT_ERR_OK;
} /* end of dal_ssw_switch_mgmtMacAddr_get */

/* Function Name:
 *      dal_ssw_switch_mgmtMacAddr_set
 * Description:
 *      Set Mac address of switch.
 * Input:
 *      unit - unit id
 *      pMac - pointer to Mac address
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
dal_ssw_switch_mgmtMacAddr_set(uint32 unit, rtk_mac_t *pMac)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d", unit);     
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pMac), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((pMac->octet[0] & BITMASK_1B) != 0), RT_ERR_MAC);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x",
           pMac->octet[0], pMac->octet[1], pMac->octet[2],
           pMac->octet[3], pMac->octet[4], pMac->octet[5]); 

    SWITCH_SEM_LOCK(unit);    

    val = (pMac->octet[0] << 24) | (pMac->octet[1] << 16) | (pMac->octet[2] << 8) | pMac->octet[3];
    if((ret = reg_write(unit, SSW_SWITCH_MAC_ADDRESS0r, &val)) != RT_ERR_OK)    
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    val = (pMac->octet[4] << 24) | (pMac->octet[5] << 16);
    if((ret = reg_write(unit, SSW_SWITCH_MAC_ADDRESS1r, &val)) != RT_ERR_OK)    
     {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    SWITCH_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_switch_mgmtMacAddr_set */

/* Function Name:
 *      dal_ssw_switch_hwInterfaceDelayEnable_get
 * Description:
 *      Get the delay state of the specified type in the specified unit.
 * Input:
 *      unit    - unit id
 *      type    - interface delay type
 * Output:
 *      pEnable - pointer to enable state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_switch_hwInterfaceDelayEnable_get(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t *pEnable)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, type=%d", unit, type);
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= DELAY_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);    

    SWITCH_SEM_LOCK(unit);    

    switch (type)
    {
        case DELAY_TYPE_INTRA_LINK0_RX:
            if((ret = reg_field_read(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, SSW_SEL_INTRA_LINK_RGMII_RXC_DELAYf, &val)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            *pEnable = val & 0x1;
            break;
        case DELAY_TYPE_INTRA_LINK0_TX:
            if((ret = reg_field_read(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, SSW_SEL_INTRA_LINK_RGMII_TXC_DELAYf, &val)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            *pEnable = val & 0x1;
            break;
        case DELAY_TYPE_INTRA_LINK1_RX:
            if((ret = reg_field_read(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, SSW_SEL_INTRA_LINK_RGMII_RXC_DELAYf, &val)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            *pEnable = (val >> 1) & 0x1;
            break;
        case DELAY_TYPE_INTRA_LINK1_TX:
            if((ret = reg_field_read(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, SSW_SEL_INTRA_LINK_RGMII_TXC_DELAYf, &val)) != RT_ERR_OK)
            {
                SWITCH_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
                return ret;
            }
            *pEnable = (val >> 1) & 0x1;
            break;
        default:
            SWITCH_SEM_UNLOCK(unit);
            return RT_ERR_FAILED;
    }

    SWITCH_SEM_UNLOCK(unit);   

    return RT_ERR_OK;
} /* end of dal_ssw_switch_hwInterfaceDelayEnable_get */

/* Function Name:
 *      dal_ssw_switch_hwInterfaceDelayEnable_set
 * Description:
 *      Set the delay state of the specified type in the specified unit.
 * Input:
 *      unit   - unit id
 *      type   - interface delay type
 *      enable - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_switch_hwInterfaceDelayEnable_set(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t enable)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_SWITCH), "unit=%d, type=%d", unit, type);
    
    /* check Init status */
    RT_INIT_CHK(switch_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((type >= DELAY_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    SWITCH_SEM_LOCK(unit);    
    
    if((ret = reg_read(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, &val)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    switch (type)
    {
        case DELAY_TYPE_INTRA_LINK0_RX:
            val &= ~(1 << 10);
            val |= (enable << 10);
            break;
        case DELAY_TYPE_INTRA_LINK0_TX:
            val &= ~(1 << 7);
            val |= (enable << 7);
            break;
        case DELAY_TYPE_INTRA_LINK1_RX:
            val &= ~(1 << 11);
            val |= (enable << 11);
            break;
        case DELAY_TYPE_INTRA_LINK1_TX:
            val &= ~(1 << 8);
            val |= (enable << 8);
            break;
        default:
            SWITCH_SEM_UNLOCK(unit);
            return RT_ERR_FAILED;
    }

    if((ret = reg_write(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, &val)) != RT_ERR_OK)
    {
        SWITCH_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    }

    SWITCH_SEM_UNLOCK(unit);   
    
    return RT_ERR_OK;
} /* end of dal_ssw_switch_hwInterfaceDelayEnable_set */
