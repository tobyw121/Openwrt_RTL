/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 40458 $
 * $Date: 2013-06-25 09:47:32 +0800 (Tue, 25 Jun 2013) $
 *
 * Purpose : Definition those public statistic APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) statistic counter reset
 *           2) statistic counter get
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
#include <ioal/mem32.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <dal/esw/dal_esw_stat.h>
#include <rtk/default.h>
#include <rtk/stat.h>

#undef _DEBUG_MIB_COUNTER

#define DAL_ESW_SYSTEM_GLOBAL_START (0x0)
#define DAL_ESW_SYSTEM_GLOBAL_LEN   (0x19)

#define DAL_ESW_SYSTEM_PORT_START   (0x44)
#define DAL_ESW_SYSTEM_PORT_LEN     (0x44)

#define DAL_ESW_DEBUG_GLOBAL_START  (0x800)
#define DAL_ESW_DEBUG_GLOBAL_LEN    (0x3c)

#define DAL_ESW_DEBUG_PORT_START    (0x842)
#define DAL_ESW_DEBUG_PORT_LEN      (0x21)

/* 
 * Symbol Definition 
 */
typedef struct counter_info_s {
    uint32  reg_index;
    uint32  field_index;
    uint32  word_count;
} counter_info_t;

/* 
 * Data Declaration 
 */
static uint32               stat_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         stat_sem[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */
/* trap semaphore handling */
#define STAT_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(stat_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_STAT),"semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define STAT_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(stat_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_STAT), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* 
 * Function Declaration 
 */
static int32
_dal_esw_stat_init_config(uint32 unit);

/* Function Name:
 *      _mib_read64
 * Description:
 *      Initialize default configuration for stp module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize stat module before calling this API
 */
static int32 _mib_read64(uint32 unit, uint32 addr, uint64 *data)
{
    uint32  reg_data = 0, val;
    uint32  busy = 0;
    uint32  tryTime = 0;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d, addr=%d", 
           unit, addr); 

    RT_PARAM_CHK((data == NULL), RT_ERR_NULL_POINTER);

    STAT_SEM_LOCK(unit);

    *data = 0;

    /*set cpu occupy*/
    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL0r, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    val = 1;
    reg_field_set(unit, ESW_INDIRECT_ACCESS_MIB_COUNTER_CONTROLr, ESW_READCOUNTERTRIGf, &val, &reg_data);    /* 1 = READ EXECUTE */
    reg_field_set(unit, ESW_INDIRECT_ACCESS_MIB_COUNTER_CONTROLr, ESW_MIBCOUNTERADDRf, &addr, &reg_data); 

    if((ret = reg_write(unit, ESW_INDIRECT_ACCESS_MIB_COUNTER_CONTROLr, &reg_data)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    do {
        if((ret = reg_field_read(unit, ESW_INDIRECT_ACCESS_MIB_COUNTER_CONTROLr, 
                        ESW_READCOUNTERTRIGf, &busy)) != RT_ERR_OK)
        {
            STAT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
            return ret;
        }

        tryTime++;
        if(tryTime > 512)
        {
            STAT_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_BUSYWAIT_TIMEOUT, (MOD_DAL|MOD_STAT), "Try times exceed!");        
            return RT_ERR_BUSYWAIT_TIMEOUT;
        }
    } while (busy);

    if((ret = reg_read(unit, ESW_INDIRECT_ACCESS_MIB_COUNTER_DATA0r, &reg_data)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }
    *data = reg_data;
    
    if((ret = reg_read(unit, ESW_INDIRECT_ACCESS_MIB_COUNTER_DATA1r, &reg_data)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }
    *data |= ((uint64)reg_data << 32);

    val = DISABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL0r, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    STAT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "data=%llu", *data);     
    
    return RT_ERR_OK;    
}   /* end of _mib_read64 */


/* Module Name : STAT */

/* Function Name:
 *      dal_esw_stat_init
 * Description:
 *      Initialize stat module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_PORT_CNTR_FAIL   - Could not retrieve/reset Port Counter
 * Note:
 *      Must initialize stat module before calling any stat APIs.
 */
int32
dal_esw_stat_init(uint32 unit)
{
    int32 ret;
    stat_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    stat_sem[unit] = osal_sem_mutex_create();
    if (0 == stat_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, MOD_STP, "semaphore create failed");
        return RT_ERR_FAILED;
    }

    if ((ret = _dal_esw_stat_init_config(unit)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }
          
    /* set init flag to complete init */
    stat_init[unit] = INIT_COMPLETED;
    
    return RT_ERR_OK;
} /* end of dal_esw_stat_init */


/* Function Name:
 *      dal_esw_stat_global_reset
 * Description:
 *      Reset the global counters in the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL - Could not retrieve/reset Global Counter
 * Note:
 *      None
 */
int32
dal_esw_stat_global_reset(uint32 unit)
{
    uint32  busy = 0, val;
    uint32  tryTime = 0;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);

    STAT_SEM_LOCK(unit);

    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL0r, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }
    
    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL1r, ESW_SYSCOUNTERRESETf, &val)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    /*Wait Reset Process completed*/
    do{
        if((ret = reg_field_read(unit, ESW_MIB_COUNTER_CONTROL1r, ESW_SYSCOUNTERRESETf, &busy)) != RT_ERR_OK)
        {
            STAT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
            return ret;
        }

        tryTime++;
        if(tryTime > 512)
        {
            STAT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "Try times exceed!");        
            return ret;
        }
    }while(busy);

    val = DISABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL0r, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    STAT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_stat_global_reset */


/* Function Name:
 *      dal_esw_stat_port_reset
 * Description:
 *      Reset the specified port counters in the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
int32
dal_esw_stat_port_reset(uint32 unit, rtk_port_t port)
{
    uint32  busy = 0, val;
    uint32  tryTime = 0;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);

    STAT_SEM_LOCK(unit);

    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL0r, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }
    
    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL1r, ESW_P0COUNTERRESETf - port, &val)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    /*Wait Reset Process completed*/
    do{
        if((ret = reg_field_read(unit, ESW_MIB_COUNTER_CONTROL1r, ESW_P0COUNTERRESETf - port, &busy)) != RT_ERR_OK)
        {
            STAT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
            return ret;
        }

        tryTime++;
        if(tryTime > 512)
        {
            STAT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "Try times exceed!");        
            return ret;
        }
    }while(busy);

    val = DISABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL0r, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    STAT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_stat_port_reset */


/* Function Name:
 *      dal_esw_stat_global_get
 * Description:
 *      Get one specified global counter in the specified device.
 * Input:
 *      unit     - unit id
 *      cntr_idx - specified global counter index
 * Output:
 *      pCntr    - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL    - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_INVALID_GLOBAL_CNTR - Invalid Global Counter
 * Note:
 *      None
 */
int32
dal_esw_stat_global_get(uint32 unit, rtk_stat_global_type_t cntr_idx, uint64 *pCntr)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pCntr), RT_ERR_NULL_POINTER);

    switch(cntr_idx)
    {
        case DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX:
            if((ret = _mib_read64(unit, DAL_ESW_SYSTEM_GLOBAL_START, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        default:
            break;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_stat_global_get */


/* Function Name:
 *      dal_esw_stat_global_getAll
 * Description:
 *      Get all global counters in the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pGlobal_cntrs - pointer buffer of global counter structure
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL    - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_INVALID_GLOBAL_CNTR - Invalid Global Counter
 * Note:
 *      None
 */
int32
dal_esw_stat_global_getAll(uint32 unit, rtk_stat_global_cntr_t *pGlobal_cntrs)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pGlobal_cntrs), RT_ERR_NULL_POINTER);

    osal_memset(pGlobal_cntrs, 0, sizeof(rtk_stat_global_cntr_t));

    if((ret = dal_esw_stat_global_get(unit, DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX, 
                 (uint64*)&pGlobal_cntrs->dot1dTpLearnedEntryDiscards)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_stat_global_getAll */


/* Function Name:
 *      dal_esw_stat_port_get
 * Description:
 *      Get one specified port counter in the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      cntr_idx - specified port counter index
 * Output:
 *      pCntr    - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
int32
dal_esw_stat_port_get(uint32 unit, rtk_port_t port, rtk_stat_port_type_t cntr_idx, uint64 *pCntr)
{
    uint32  baseAddr = 0;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pCntr), RT_ERR_NULL_POINTER);

    if(port >= 14)
        baseAddr = DAL_ESW_SYSTEM_PORT_START + 4;
    else
        baseAddr = DAL_ESW_SYSTEM_PORT_START;        

    switch(cntr_idx)
    {
        case IF_OUT_OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            *pCntr = (uint32)(*pCntr);
            break;
            
        case IF_HC_OUT_OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case IF_OUT_UCAST_PKTS_CNT_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 2, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            *pCntr = (uint32)(*pCntr);
            break;    
        case IF_HC_OUT_UCAST_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 2, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case IF_OUT_MULTICAST_PKTS_CNT_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 4, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            *pCntr = (uint32)(*pCntr);
            break;
        case IF_HC_OUT_MULTICAST_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 4, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case IF_OUT_BROADCAST_PKTS_CNT_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 6, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            *pCntr = (uint32)(*pCntr);
            break; 
        case IF_HC_OUT_BROADCAST_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 6, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;  
        case IF_IN_OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 8, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            *pCntr = (uint32)(*pCntr);
            break;  
        case IF_HC_IN_OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 8, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;   
        case IF_IN_UCAST_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0xa, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            *pCntr = (uint32)(*pCntr);
            break; 
        case IF_HC_IN_UCAST_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0xa, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;   
        case IF_HC_IN_MULTICAST_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0xc, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;  
        case IF_HC_IN_BROADCAST_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0xe, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case IF_OUT_DISCARDS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x10, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;    
         case IF_IN_DISCARDS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x11, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case IP_IN_RECEIVES_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x12, pCntr)) != RT_ERR_OK)
             {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;   
        case IP_IN_DISCARDS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x13, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;   
        case DOT1D_BASE_PORT_DELAY_EXCEEDED_DISCARDS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x14, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case DOT1D_TP_PORT_IN_DISCARDS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x16, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            *pCntr = (uint32)(*pCntr);
            break;         
        case DOT1D_TP_HC_PORT_IN_DISCARDS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x16, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;         
        case DOT3_STATS_ALIGNMENT_ERRORS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x18, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;           
        case DOT3_STATS_FCS_ERRORS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x19, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case DOT3_STATS_SINGLE_COLLISION_FRAMES_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x1a, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case DOT3_STATS_MULTIPLE_COLLISION_FRAMES_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x1b, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            } 
            break;
        case DOT3_STATS_DEFERRED_TRANSMISSIONS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x1c, pCntr)) != RT_ERR_OK)
             {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            } 
            break;
        case DOT3_STATS_LATE_COLLISIONS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x1d, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case DOT3_STATS_EXCESSIVE_COLLISIONS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x1e, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;    
        case DOT3_STATS_FRAME_TOO_LONGS_INDEX: 
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x20, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;            
        case DOT3_STATS_SYMBOL_ERRORS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x21, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case DOT3_CONTROL_IN_UNKNOWN_OPCODES_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x22, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break; 
        case DOT3_IN_PAUSE_FRAMES_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x23, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case DOT3_OUT_PAUSE_FRAMES_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x24, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case DOT3_OUT_PAUSE_ON_FRAMES_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x25, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;        
        case ETHER_STATS_DROP_EVENTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x26, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;  
        case ETHER_STATS_FRAGMENTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x27, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case ETHER_STATS_JABBERS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x28, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case ETHER_STATS_COLLISIONS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x29, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            } 
            break;
        /*RX Start*/    
        case ETHER_STATS_UNDER_SIZE_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x2a, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break; 
         case ETHER_STATS_PKTS_64OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x2b, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            } 
            break;
        case ETHER_STATS_PKTS_65TO127OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x2c, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            } 
            break;
        case ETHER_STATS_PKTS_128TO255OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x2d, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            } 
            break;
        case ETHER_STATS_PKTS_256TO511OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x2e, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case ETHER_STATS_PKTS_512TO1023OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x2f, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case ETHER_STATS_PKTS_1024TO1518OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x30, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;   
        case ETHER_STATS_OVERSIZE_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x31, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;     
        case ETHER_STATS_TX_UNDER_SIZE_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x32, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break; 
         case ETHER_STATS_TX_PKTS_64OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x33, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            } 
            break;
        case ETHER_STATS_TX_PKTS_65TO127OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x34, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            } 
            break;
        case ETHER_STATS_TX_PKTS_128TO255OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x35, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            } 
            break;
        case ETHER_STATS_TX_PKTS_256TO511OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x36, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case ETHER_STATS_TX_PKTS_512TO1023OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x37, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case ETHER_STATS_TX_PKTS_1024TO1518OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x38, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;   
        case ETHER_STATS_TX_OVERSIZE_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x39, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;          
        case ETHER_STATS_OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x3a, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;          
        case ETHER_STATS_TX_OCTETS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0x3c, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;          

        case ETHER_STATS_MULTICAST_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0xc, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break; 
            
        case ETHER_STATS_BROADCAST_PKTS_INDEX:
            if((ret = _mib_read64(unit, baseAddr + port*DAL_ESW_SYSTEM_PORT_LEN + 0xe, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        default:
            *pCntr = 0;
            break;
    }    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "pCntr=%llu", *pCntr);
    
    return RT_ERR_OK;
} /* end of dal_esw_stat_port_get */


/* Function Name:
 *      dal_esw_stat_port_getAll
 * Description:
 *      Get all counters of one specified port in the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pPort_cntrs - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
int32
dal_esw_stat_port_getAll(uint32 unit, rtk_port_t port, rtk_stat_port_cntr_t *pPort_cntrs)
{
    uint64  counter = 0;
    int32   ret;
    rtk_stat_port_type_t    counter_idx;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pPort_cntrs), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    osal_memset(pPort_cntrs, 0, sizeof(rtk_stat_port_cntr_t));
    
    for (counter_idx = IF_IN_OCTETS_INDEX; counter_idx < MIB_PORT_CNTR_END; counter_idx++)
    {      
        counter = 0;
        if ((ret = dal_esw_stat_port_get(unit, port, counter_idx, &counter)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }
        
        switch (counter_idx)
        {
            case IF_OUT_OCTETS_INDEX:
                pPort_cntrs->ifOutOctets = counter;
                break;
            case IF_HC_OUT_OCTETS_INDEX:
                pPort_cntrs->ifHCOutOctets = counter;
                break;
            case IF_OUT_UCAST_PKTS_CNT_INDEX:
                pPort_cntrs->ifOutUcastPkts = counter;
                break;
            case IF_HC_OUT_UCAST_PKTS_INDEX:
                pPort_cntrs->ifHCOutUcastPkts = counter;
                break;  
            case IF_OUT_MULTICAST_PKTS_CNT_INDEX:
                pPort_cntrs->ifOutMulticastPkts = counter;
                break;
            case IF_HC_OUT_MULTICAST_PKTS_INDEX:
                pPort_cntrs->ifHCOutMulticastPkts = counter;
                break;
            case IF_OUT_BROADCAST_PKTS_CNT_INDEX:
                pPort_cntrs->ifOutBrocastPkts = counter;
                break; 
            case IF_HC_OUT_BROADCAST_PKTS_INDEX:
                pPort_cntrs->ifHCOutBroadcastPkts = counter;
                break;  
            case IF_IN_OCTETS_INDEX:
                pPort_cntrs->ifInOctets = counter;
                break; 
            case IF_HC_IN_OCTETS_INDEX:
                pPort_cntrs->ifHCInOctets = counter;
                break;   
            case IF_IN_UCAST_PKTS_INDEX:
                pPort_cntrs->ifInUcastPkts = counter;
                break;
            case IF_HC_IN_UCAST_PKTS_INDEX:
                pPort_cntrs->ifHCInUcastPkts = counter;
                break;   
            case IF_HC_IN_MULTICAST_PKTS_INDEX:
                pPort_cntrs->ifHCInMulticastPkts = counter;
                break;  
            case IF_HC_IN_BROADCAST_PKTS_INDEX:
                pPort_cntrs->ifHCInBroadcastPkts = counter;
                break;
            case IF_OUT_DISCARDS_INDEX:
                pPort_cntrs->ifOutDiscards = counter;
                break;    
             case IF_IN_DISCARDS_INDEX:
                pPort_cntrs->ifInDiscards = counter;
                break;
            case IP_IN_RECEIVES_INDEX:
                pPort_cntrs->ipInReceives = counter;
                break;   
            case IP_IN_DISCARDS_INDEX:
                pPort_cntrs->ipInDiscards = counter;
                break;   
            case DOT1D_BASE_PORT_DELAY_EXCEEDED_DISCARDS_INDEX:
                pPort_cntrs->dot1dBasePortDelayExceededDiscards = counter;
                break;
            case DOT1D_TP_HC_PORT_IN_DISCARDS_INDEX:
                pPort_cntrs->dot1dTPHCPortInDiscards= counter;
                break;       
            case DOT3_STATS_ALIGNMENT_ERRORS_INDEX:
                pPort_cntrs->dot3StatsAlignmentErrors = counter;
                break;             
            case DOT3_STATS_FCS_ERRORS_INDEX:
                pPort_cntrs->dot3StatsFCSErrors = counter;
                break;
            case DOT3_STATS_SINGLE_COLLISION_FRAMES_INDEX:
                pPort_cntrs->dot3StatsSingleCollisionFrames = counter;
                break;
            case DOT3_STATS_MULTIPLE_COLLISION_FRAMES_INDEX:
                pPort_cntrs->dot3StatsMultipleCollisionFrames = counter;
                break;
            case DOT3_STATS_DEFERRED_TRANSMISSIONS_INDEX:
                pPort_cntrs->dot3StatsDeferredTransmissions = counter;
                break;
            case DOT3_STATS_LATE_COLLISIONS_INDEX:
                pPort_cntrs->dot3StatsLateCollisions = counter;
                break;
            case DOT3_STATS_EXCESSIVE_COLLISIONS_INDEX:
                pPort_cntrs->dot3StatsExcessiveCollisions = counter;
                break;               
            case DOT3_STATS_FRAME_TOO_LONGS_INDEX: 
                pPort_cntrs->dot3StatsFrameTooLongs = counter;
                break;               
            case DOT3_STATS_SYMBOL_ERRORS_INDEX:
                pPort_cntrs->dot3StatsSymbolErrors = counter;
                break;
            case DOT3_CONTROL_IN_UNKNOWN_OPCODES_INDEX:
                pPort_cntrs->dot3ControlInUnknownOpcodes = counter;
                break; 
            case DOT3_IN_PAUSE_FRAMES_INDEX:
                pPort_cntrs->dot3InPauseFrames = counter;
                break;
            case DOT3_OUT_PAUSE_FRAMES_INDEX:
                pPort_cntrs->dot3OutPauseFrames = counter;
                break;
            case DOT3_OUT_PAUSE_ON_FRAMES_INDEX:
                pPort_cntrs->dot3OutPauseOnFrames = counter;
                break;
            case ETHER_STATS_DROP_EVENTS_INDEX:
                pPort_cntrs->etherStatsDropEvents = counter;
                break;  
            case ETHER_STATS_FRAGMENTS_INDEX:
                pPort_cntrs->etherStatsFragments = counter;
                break;
            case ETHER_STATS_JABBERS_INDEX:
                pPort_cntrs->etherStatsJabbers = counter;
                break;
            case ETHER_STATS_COLLISIONS_INDEX:
                pPort_cntrs->etherStatsCollisions = counter;
                break;
            case ETHER_STATS_UNDER_SIZE_PKTS_INDEX:
                pPort_cntrs->etherStatsUndersizePkts = counter;
                break; 
             case ETHER_STATS_PKTS_64OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts64Octets = counter;
                break;
            case ETHER_STATS_PKTS_65TO127OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts65to127Octets = counter;
                break;
            case ETHER_STATS_PKTS_128TO255OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts128to255Octets = counter;
                break;
            case ETHER_STATS_PKTS_256TO511OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts256to511Octets= counter;
                break;
            case ETHER_STATS_PKTS_512TO1023OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts512to1023Octets = counter;
                break;
            case ETHER_STATS_PKTS_1024TO1518OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts1024toMaxOctets = counter;
                break;   
            case ETHER_STATS_OVERSIZE_PKTS_INDEX:
                pPort_cntrs->etherStatsOversizePkts = counter;
                break;                   
            case ETHER_STATS_TX_UNDER_SIZE_PKTS_INDEX:
                pPort_cntrs->etherStatsTxUndersizePkts = counter;
                break; 
             case ETHER_STATS_TX_PKTS_64OCTETS_INDEX:
                pPort_cntrs->etherStatsTxPkts64Octets = counter;
                break;
            case ETHER_STATS_TX_PKTS_65TO127OCTETS_INDEX:
                pPort_cntrs->etherStatsTxPkts65to127Octets = counter;
                break;
            case ETHER_STATS_TX_PKTS_128TO255OCTETS_INDEX:
                pPort_cntrs->etherStatsTxPkts128to255Octets = counter;
                break;
            case ETHER_STATS_TX_PKTS_256TO511OCTETS_INDEX:
                pPort_cntrs->etherStatsTxPkts256to511Octets= counter;
                break;
            case ETHER_STATS_TX_PKTS_512TO1023OCTETS_INDEX:
                pPort_cntrs->etherStatsTxPkts512to1023Octets = counter;
                break;
            case ETHER_STATS_TX_PKTS_1024TO1518OCTETS_INDEX:
                pPort_cntrs->etherStatsTxPkts1024toMaxOctets = counter;
                break;   
            case ETHER_STATS_TX_OVERSIZE_PKTS_INDEX:
                pPort_cntrs->etherStatsTxOversizePkts = counter;
                break;                 
            case ETHER_STATS_OCTETS_INDEX:
                pPort_cntrs->etherStatsOctets = counter;
                break;                
            case ETHER_STATS_TX_OCTETS_INDEX:
                pPort_cntrs->etherStatsTxOctets = counter;
                break;                  
            case ETHER_STATS_MULTICAST_PKTS_INDEX:
                pPort_cntrs->etherStatsMulticastPkts = counter;
                break;
            case ETHER_STATS_BROADCAST_PKTS_INDEX:
                pPort_cntrs->etherStatsBroadcastPkts = counter;
                break;                
            default:
                break;
        }
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_stat_port_getAll */

/* Function Name:
 *      dal_esw_stat_smon_get
 * Description:
 *      Get one specified SMON counter in specified device.
 * Input:
 *      unit            - unit id
 *      cntr_idx        - index of specified SMON counter 
 *      pri             - priority
 * Output:              
 *      pCntr           - pointer buffer of counte value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - inputt parameter may be null pointer
 *      RT_ERR_STAT_SMON_CNTR_FAIL      - Could not retrieve/reset SMON Counter
 *      RT_ERR_STAT_INVALID_SMON_CNTR   - Could not retrieve/reset SMON Counter
 * Note:
 *      None
 */
int32
dal_esw_stat_smon_get(uint32 unit, rtk_pri_t pri, rtk_stat_smon_type_t cntr_idx,  uint64 *pCntr)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d", unit); 
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((pri > RTK_DOT1P_PRIORITY_MAX), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pCntr), RT_ERR_NULL_POINTER);

    switch(cntr_idx)
    {
        case SMON_PRIO_STATS_PKTS:
            if((ret = _mib_read64(unit, DAL_ESW_SYSTEM_GLOBAL_START + 2 + pri, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        case SMON_PRIO_STATS_OCTETS:
            if((ret = _mib_read64(unit, DAL_ESW_SYSTEM_GLOBAL_START + 0xa + 2*pri, pCntr)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
                return ret;
            }
            break;
        default:
            break;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "pCntr=%llu", *pCntr);
    return RT_ERR_OK;
} /* end of dal_esw_stat_smon_get */

/* Function Name:
 *      dal_esw_stat_smon_getAll
 * Description:
 *      Get all specified SMON counter in specified device.
 * Input:
 *      unit            - unit id
 *      cntr_idx        - index of specified SMON counter 
 *      pri             - priority
 * Output:              
 *      pCntr           - pointer buffer of counte value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - inputt parameter may be null pointer
 *      RT_ERR_STAT_SMON_CNTR_FAIL      - Could not retrieve/reset SMON Counter
 * Note:
 *      None
 */
int32
dal_esw_stat_smon_getAll(uint32 unit, rtk_pri_t pri, rtk_stat_smon_cntr_t *pCntr)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d, pri=%d", unit, pri); 
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((pri > RTK_DOT1P_PRIORITY_MAX), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pCntr), RT_ERR_NULL_POINTER);

    osal_memset(pCntr, 0, sizeof(rtk_stat_smon_cntr_t));

    if((ret = _mib_read64(unit, DAL_ESW_SYSTEM_GLOBAL_START + 2 + pri, (uint64*)&pCntr->smonPrioStatsPkts)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    if((ret = _mib_read64(unit, DAL_ESW_SYSTEM_GLOBAL_START + 0xa + 2*pri, &pCntr->smonPrioStatsOctets)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "pCntr->smonPrioStatsOctets=%llu, pCntr->smonPrioStatsPkts=%llu", 
                    pCntr->smonPrioStatsOctets, pCntr->smonPrioStatsPkts); 
    
    return RT_ERR_OK;
} /* end of dal_esw_stat_smon_getAll */

/* Function Name:
 *      _dal_esw_stat_init_config
 * Description:
 *      Initialize default configuration for stat module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize stat module before calling this API
 */
static int32
_dal_esw_stat_init_config(uint32 unit)
{
    uint32  val;
    int32   ret;

    /*Enable Cpu Access*/
    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL0r, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    /*Enable mib counter*/
    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL0r, ESW_ENMIBCOUNTERf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    /*Start System Global Mib Counters*/
    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL2r, ESW_SYSCOUNTERSTARTf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    /*Start System Port Mib Counters*/
    val = 0xFFFFFFFF;
    if((ret = reg_write(unit, ESW_MIB_COUNTER_CONTROL2r,  &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

#if defined(_DEBUG_MIB_COUNTER)
    /*Enable Debug Mib Counter*/
    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL0r,  ESW_ENDBMIBCOUNTERf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    /*Start Debug Mib Counter*/
    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL2r,  ESW_SYSDEBUGCOUNTERSTARTf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }
    
    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL2r,  ESW_PORTDEBUGCOUNTERSTARTf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }
#endif

    /*Disable Cpu Access*/
    val = DISABLED;
    if((ret = reg_field_write(unit, ESW_MIB_COUNTER_CONTROL0r, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");        
        return ret;
    }

    return RT_ERR_OK;
} /* end of _dal_esw_stat_init_config */

