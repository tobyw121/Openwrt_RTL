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
 * $Revision: 41321 $
 * $Date: 2013-07-19 15:27:25 +0800 (Fri, 19 Jul 2013) $
 *
 * Purpose : Definition those public L2 APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) l2 address table
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
#include <osal/time.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/ssw/rtk_ssw_table_struct.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <dal/ssw/dal_ssw_trunk.h>
#include <dal/ssw/dal_ssw_l2.h>
#include <rtk/default.h>
#include <rtk/l2.h>

/* 
 * Symbol Definition 
 */

#define END_OF_MCAST_IDX    (0xFFFF)
#define MCAST_IDX_ALLOCATED (0xFFFE)

/* Special definition for RTL8389 and 8329 */
#define ACCADDR_L2TYPE_OFFSET (14)

#ifndef SRAM
#define SRAM 0
#endif

#ifndef CAM
#define CAM 1
#endif


typedef enum l2_entry_type_e
{
    L2_UNICAST = 0,
    L2_MULTICAST,
    IP_MULTICAST,
    L2_ENTRY_TYPE_END
} l2_entry_type_t;

typedef enum dal_ssw_l2_getMethod_e
{
    L2_GET_EXIST_ONLY = 0,
    L2_GET_EXIST_OR_FREE,
    L2_GET_FREE_ONLY,
    DAL_SSW_GETMETHOD_END
} dal_ssw_l2_getMethod_t;

typedef enum dal_ssw_l2_indexType_e
{
    L2_IN_HASH = 0,
    L2_IN_CAM,
    DAL_SSW_L2_INDEXTYPE_END
} dal_ssw_l2_indexType_t;

typedef struct dal_ssw_l2_entry_s {
    l2_entry_type_t  entry_type; /* unicast, l2 multicast, ip multicast */
    uint32           is_entry_exist;
    union {
        struct unicast_entry_s {
            rtk_fid_t   fid;
            rtk_mac_t   mac;
            rtk_port_t  port;
            uint32      aging;
            uint32      sablock;
            uint32      dablock;
            uint32      auth;
            uint32      is_static;
        } unicast;
        struct l2mcast_entry_s {
            rtk_vlan_t  rvid;
            rtk_mac_t   mac;
            uint32      index;
        } l2mcast;
        struct ipmcast_entry_s {
            rtk_vlan_t  rvid;
            ipaddr_t    dip;
            ipaddr_t    sip;
            uint32      index;
        } ipmcast;
    };
} dal_ssw_l2_entry_t;

typedef struct dal_ssw_l2_index_s {
    uint32  index_type;     /* In CAM or In HASH */
    uint32  index;
    uint32  hashdepth;      /* only useful when entry is in hash table */
} dal_ssw_l2_index_t;

typedef struct dal_ssw_mcast_index_s {
    uint16  next_index;
    uint16  ref_count;
} dal_ssw_mcast_index_t;

typedef struct dal_ssw_mcast_index_pool_s {
    dal_ssw_mcast_index_t   *pMcast_index_pool;
    uint32                  size_of_mcast_fwd_index;
    uint16                  free_index_head;
    uint16                  free_entry_count;
} dal_ssw_mcast_index_pool_t;

/* 
 * Data Declaration 
 */
static uint32               l2_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         l2_sem[RTK_MAX_NUM_OF_UNIT];
static uint32               algoType[RTK_MAX_NUM_OF_UNIT] = {0};

/* Multicast database */
static dal_ssw_mcast_index_pool_t    mcast_idx_pool[RTK_MAX_NUM_OF_UNIT];

const static uint16 learned_l2_entry_counter_regidx[] = 
{  SSW_LEARNED_L2_ENTRY_COUNTER0r, SSW_LEARNED_L2_ENTRY_COUNTER0r\
 , SSW_LEARNED_L2_ENTRY_COUNTER1r, SSW_LEARNED_L2_ENTRY_COUNTER1r\
 , SSW_LEARNED_L2_ENTRY_COUNTER2r, SSW_LEARNED_L2_ENTRY_COUNTER2r\
 , SSW_LEARNED_L2_ENTRY_COUNTER3r, SSW_LEARNED_L2_ENTRY_COUNTER3r\
 , SSW_LEARNED_L2_ENTRY_COUNTER4r, SSW_LEARNED_L2_ENTRY_COUNTER4r\
 , SSW_LEARNED_L2_ENTRY_COUNTER5r, SSW_LEARNED_L2_ENTRY_COUNTER5r\
 , SSW_LEARNED_L2_ENTRY_COUNTER6r, SSW_LEARNED_L2_ENTRY_COUNTER6r\
 , SSW_LEARNED_L2_ENTRY_COUNTER7r, SSW_LEARNED_L2_ENTRY_COUNTER7r\
 , SSW_LEARNED_L2_ENTRY_COUNTER8r, SSW_LEARNED_L2_ENTRY_COUNTER8r\
 , SSW_LEARNED_L2_ENTRY_COUNTER9r, SSW_LEARNED_L2_ENTRY_COUNTER9r\
 , SSW_LEARNED_L2_ENTRY_COUNTER10r, SSW_LEARNED_L2_ENTRY_COUNTER10r\
 , SSW_LEARNED_L2_ENTRY_COUNTER11r, SSW_LEARNED_L2_ENTRY_COUNTER11r\
 , SSW_LEARNED_L2_ENTRY_COUNTER12r, SSW_LEARNED_L2_ENTRY_COUNTER12r\
 , SSW_LEARNED_L2_ENTRY_COUNTER13r, SSW_LEARNED_L2_ENTRY_COUNTER13r\
 , SSW_LEARNED_L2_ENTRY_COUNTER14r};

const static uint16 limited_l2_entry_control_regidx[] = 
{  SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL0r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL0r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL1r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL1r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL2r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL2r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL3r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL3r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL4r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL4r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL5r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL5r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL6r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL6r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL7r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL7r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL8r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL8r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL9r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL9r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL10r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL10r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL11r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL11r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL12r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL12r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL13r, SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL13r\
 , SSW_LIMITED_L2_ENTRY_NUMBER_CONTROL14r};

static uint16 lrncnt_fieldidx[] = {SSW_P0_LL2CNTf, SSW_P1_LL2CNTf, SSW_P2_LL2CNTf, SSW_P3_LL2CNTf, SSW_P4_LL2CNTf, SSW_P5_LL2CNTf, SSW_P6_LL2CNTf, SSW_P7_LL2CNTf, SSW_P8_LL2CNTf, SSW_P9_LL2CNTf\
                         ,SSW_P10_LL2CNTf, SSW_P11_LL2CNTf, SSW_P12_LL2CNTf, SSW_P13_LL2CNTf, SSW_P14_LL2CNTf, SSW_P15_LL2CNTf, SSW_P16_LL2CNTf, SSW_P17_LL2CNTf, SSW_P18_LL2CNTf, SSW_P19_LL2CNTf\
                         ,SSW_P20_LL2CNTf, SSW_P21_LL2CNTf, SSW_P22_LL2CNTf, SSW_P23_LL2CNTf, SSW_P24_LL2CNTf, SSW_P25_LL2CNTf, SSW_P26_LL2CNTf, SSW_P27_LL2CNTf, SSW_P28_LL2CNTf};
static uint16 limit_lrncnt_fieldidx[] = {SSW_P0_L2LIMNUMf, SSW_P1_L2LIMNUMf, SSW_P2_L2LIMNUMf, SSW_P3_L2LIMNUMf, SSW_P4_L2LIMNUMf, SSW_P5_L2LIMNUMf, SSW_P6_L2LIMNUMf, SSW_P7_L2LIMNUMf, SSW_P8_L2LIMNUMf, SSW_P9_L2LIMNUMf\
                         ,SSW_P10_L2LIMNUMf, SSW_P11_L2LIMNUMf, SSW_P12_L2LIMNUMf, SSW_P13_L2LIMNUMf, SSW_P14_L2LIMNUMf, SSW_P15_L2LIMNUMf, SSW_P16_L2LIMNUMf, SSW_P17_L2LIMNUMf, SSW_P18_L2LIMNUMf, SSW_P19_L2LIMNUMf\
                         ,SSW_P20_L2LIMNUMf, SSW_P21_L2LIMNUMf, SSW_P22_L2LIMNUMf, SSW_P23_L2LIMNUMf, SSW_P24_L2LIMNUMf, SSW_P25_L2LIMNUMf, SSW_P26_L2LIMNUMf, SSW_P27_L2LIMNUMf, SSW_P28_L2LIMNUMf};
static uint16 limit_lrnact_fieldidx[] = {SSW_P0_L2LIMACTf, SSW_P1_L2LIMACTf, SSW_P2_L2LIMACTf, SSW_P3_L2LIMACTf, SSW_P4_L2LIMACTf, SSW_P5_L2LIMACTf, SSW_P6_L2LIMACTf, SSW_P7_L2LIMACTf, SSW_P8_L2LIMACTf, SSW_P9_L2LIMACTf\
                         ,SSW_P10_L2LIMACTf, SSW_P11_L2LIMACTf, SSW_P12_L2LIMACTf, SSW_P13_L2LIMACTf, SSW_P14_L2LIMACTf, SSW_P15_L2LIMACTf, SSW_P16_L2LIMACTf, SSW_P17_L2LIMACTf, SSW_P18_L2LIMACTf, SSW_P19_L2LIMACTf\
                         ,SSW_P20_L2LIMACTf, SSW_P21_L2LIMACTf, SSW_P22_L2LIMACTf, SSW_P23_L2LIMACTf, SSW_P24_L2LIMACTf, SSW_P25_L2LIMACTf, SSW_P26_L2LIMACTf, SSW_P27_L2LIMACTf, SSW_P28_L2LIMACTf};
static uint16 limit_lrntrap_fieldidx[] = {SSW_P0_L2LIMACT_TRAPf, SSW_P1_L2LIMACT_TRAPf, SSW_P2_L2LIMACT_TRAPf, SSW_P3_L2LIMACT_TRAPf, SSW_P4_L2LIMACT_TRAPf, SSW_P5_L2LIMACT_TRAPf, SSW_P6_L2LIMACT_TRAPf, SSW_P7_L2LIMACT_TRAPf, SSW_P8_L2LIMACT_TRAPf, SSW_P9_L2LIMACT_TRAPf\
                         ,SSW_P10_L2LIMACT_TRAPf, SSW_P11_L2LIMACT_TRAPf, SSW_P12_L2LIMACT_TRAPf, SSW_P13_L2LIMACT_TRAPf, SSW_P14_L2LIMACT_TRAPf, SSW_P15_L2LIMACT_TRAPf, SSW_P16_L2LIMACT_TRAPf, SSW_P17_L2LIMACT_TRAPf, SSW_P18_L2LIMACT_TRAPf, SSW_P19_L2LIMACT_TRAPf\
                         ,SSW_P20_L2LIMACT_TRAPf, SSW_P21_L2LIMACT_TRAPf, SSW_P22_L2LIMACT_TRAPf, SSW_P23_L2LIMACT_TRAPf, SSW_P24_L2LIMACT_TRAPf, SSW_P25_L2LIMACT_TRAPf, SSW_P26_L2LIMACT_TRAPf, SSW_P27_L2LIMACT_TRAPf};
/*
 * Macro Definition
 */
/* vlan semaphore handling */
#define L2_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(l2_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_L2), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define L2_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(l2_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_L2), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* 
 * Function Declaration 
 */
static int32 _dal_ssw_l2_init_config(uint32 unit);
static int32 _dal_ssw_l2_getExistOrFreeL2Entry(uint32 unit, dal_ssw_l2_entry_t *pL2_entry, dal_ssw_l2_getMethod_t get_method
        , dal_ssw_l2_index_t *pL2_index);
static int32 _dal_ssw_l2_getL2EntryfromHash(uint32 unit, uint32 hashkey, uint32 location, 
                              dal_ssw_l2_entry_t *pL2_entry, uint32 *pIsValid);
static int32 _dal_ssw_l2_entryToHashKey(uint32 unit, dal_ssw_l2_entry_t *pL2_entry, uint32 *pKey);
static int32 _dal_ssw_l2_compareEntry(dal_ssw_l2_entry_t *pSrcEntry, dal_ssw_l2_entry_t *pDstEntry);
static int32 _dal_ssw_l2_getL2EntryfromCAM(uint32 unit, uint32 index, 
                              dal_ssw_l2_entry_t *pL2_entry, uint32 *pIsValid);
static int32 _dal_ssw_l2_setL2CAMEntry(uint32 unit, dal_ssw_l2_entry_t *pL2_entry, dal_ssw_l2_index_t *pL2_index);
static int32 _dal_ssw_l2_setL2HASHEntry(uint32 unit, dal_ssw_l2_entry_t *pL2_entry, dal_ssw_l2_index_t *pL2_index);
static int32 _dal_ssw_l2_freeMcastIdx(uint32 unit, int32 mcastIdx);
static int32 _dal_ssw_l2_allocMcastIdx(uint32 unit, int32 *pMcastIdx);
static int32 _dal_ssw_l2_isMcastIdxUsed(uint32 unit, int32 mcastIdx);
static int32 _dal_ssw_l2_nextValidAddr_get(uint32 unit, int32 *pScan_idx, uint32 type,
                            uint32 include_static, dal_ssw_l2_entry_t  *pL2_data);
static int32 _dal_ssw_l2_getFirstDynamicEntry(uint32 unit, dal_ssw_l2_entry_t *pL2_entry,
                            dal_ssw_l2_index_t *pL2_index);


/* Module Name    : L2     */
/* Sub-module Name: Global */

/* Function Name:
 *      dal_ssw_l2_init
 * Description:
 *      Initialize l2 module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize l2 module before calling any l2 APIs.
 */
int32
dal_ssw_l2_init(uint32 unit)
{
    int32   ret;
    uint32  index;
    uint32  mcast_tableSize;
    
    l2_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    l2_sem[unit] = osal_sem_mutex_create();
    if (0 == l2_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_L2), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    if ((ret = table_size_get(unit, SSW_MULTICAST_INDEX_TABLEt, &mcast_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    L2_SEM_LOCK(unit);
           
    /* allocate memory for free multicast index */
    mcast_idx_pool[unit].pMcast_index_pool = (dal_ssw_mcast_index_t *)osal_alloc(mcast_tableSize * sizeof(dal_ssw_mcast_index_t));
    if (0 == mcast_idx_pool[unit].pMcast_index_pool)
    {
        
        mcast_idx_pool[unit].pMcast_index_pool = 0;
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "memory allocate failed", ret);  
        return ret;
    }
    
    mcast_idx_pool[unit].size_of_mcast_fwd_index = mcast_tableSize;
    mcast_idx_pool[unit].free_entry_count = mcast_tableSize;
    /* first free index is 0 */
    mcast_idx_pool[unit].free_index_head = 0;

    /* create free link-list for all entry, from 0 ~ max index - 2 */
    for (index = 0; index < (mcast_tableSize - 1); index++)
    {
        mcast_idx_pool[unit].pMcast_index_pool[index].next_index = index + 1;
        mcast_idx_pool[unit].pMcast_index_pool[index].ref_count = 0;
    }
    mcast_idx_pool[unit].pMcast_index_pool[mcast_tableSize - 1].next_index = END_OF_MCAST_IDX;
    mcast_idx_pool[unit].pMcast_index_pool[mcast_tableSize - 1].ref_count = 0;    

    L2_SEM_UNLOCK(unit);
    
    /* set init flag to complete init */
    l2_init[unit] = INIT_COMPLETED;
    
    if ((ret = _dal_ssw_l2_init_config(unit)) != RT_ERR_OK)
    {
        l2_init[unit] = INIT_NOT_COMPLETED;
        osal_free(mcast_idx_pool[unit].pMcast_index_pool);
        mcast_idx_pool[unit].pMcast_index_pool = 0;
        mcast_idx_pool[unit].free_index_head = 0;
        mcast_idx_pool[unit].free_entry_count = 0;
        mcast_idx_pool[unit].size_of_mcast_fwd_index = 0;
        RT_ERR(ret, (MOD_DAL|MOD_L2), "L2 default config initialize failed");  
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_ssw_l2_init */


/* Function Name:
 *      dal_ssw_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pEnable  - pointer buffer of state of HW clear linkdown port mac
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Make sure chip have supported the function before using the API.
 *      2. The API is apply to whole system.
 *      3. The status of flush linkdown port address is as following:
 *        - DISABLED
 *        - ENABLED
 */
int32
dal_ssw_l2_flushLinkDownPortAddrEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  enable;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    L2_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, SSW_L2_TABLE_CONTROLr, SSW_LINK_DOWN_PORT_INVALIDf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);
    
    /* translate chip's value to definition */
    if (1 == enable)
    {
        *pEnable = ENABLED;
    } 
    else 
    {
        *pEnable = DISABLED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_flushLinkDownPortAddrEnable_get */


/* Function Name:
 *      dal_ssw_l2_flushLinkDownPortAddrEnable_set
 * Description:
 *      Set HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      unit   - unit id
 *      enable - configure value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      1. Make sure chip have supported the function before using the API.
 *      2. The API is apply to whole system.
 *      3. The status of flush linkdown port address is as following:
 *        - DISABLED
 *        - ENABLED
 */
int32
dal_ssw_l2_flushLinkDownPortAddrEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d",
           unit, enable);
        
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((ENABLED != enable && DISABLED != enable), RT_ERR_INPUT);
    
    /* translate definition to chip's value  */
    if (ENABLED == enable)
    {
        value = 1;
    } 
    else
    {
        value = 0;
    }
    
    L2_SEM_LOCK(unit);
    
    /* programming value to CHIP*/
    if ((ret = reg_field_write(unit, SSW_L2_TABLE_CONTROLr, SSW_LINK_DOWN_PORT_INVALIDf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_flushLinkDownPortAddrEnable_set */


/* Function Name:
 *      dal_ssw_l2_ucastAddr_flush
 * Description:
 *      Flush unicast address
 * Input:
 *      unit    - unit id
 *      pConfig - flush config
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      (1) The API have the same function as dal_ssw_l2_flushType_set, 
 *          but use structure argument for future new chip extension.
 *      (2) Some fields of structure is inactive in ssw series chips as following:
 *          - pConfig->flushByMac
 *          - pConfig->ucastAddr
 *          - pConfig->flushStaticAddr
 *          - pConfig->flushAddrOnAllPorts
 */
int32
dal_ssw_l2_ucastAddr_flush(uint32 unit, rtk_l2_flushCfg_t *pConfig)
{
    int32   ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_entry_t  null_l2_entry;
    rtk_portmask_t  trunk_portmask;
    uint32  cmp_fid;
    uint32  cmp_port;
    uint32  l2cam_idx;
    uint32  l2_tableSize;
    uint32  l2cam_tableSize;
    uint32  isValid;
    uint32  value, temp;
    
    dal_ssw_l2_index_t l2_index;
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pConfig), RT_ERR_NULL_POINTER);    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pConfig->flushAddrOnAllPorts=%d, pConfig->flushByMac=%d,\
                 pConfig->flushByPort=%d, pConfig->flushByVid=%d, pConfig->flushStaticAddr=%d, pConfig->port=%d, \
                 pConfig->portOrTrunk=%d, pConfig->vid=%d, pConfig->ucastAddr=0x%02x:%02x:%02x:%02x:%02x:%02x", 
                 pConfig->flushAddrOnAllPorts, pConfig->flushByMac,   pConfig->flushByPort, pConfig->flushByVid, 
                 pConfig->flushStaticAddr, pConfig->port, pConfig->portOrTrunk, pConfig->vid, 
                 pConfig->ucastAddr.octet[0], pConfig->ucastAddr.octet[1],pConfig->ucastAddr.octet[2],
                 pConfig->ucastAddr.octet[3],pConfig->ucastAddr.octet[4],pConfig->ucastAddr.octet[5]); 

    if (( ret = table_size_get(unit, SSW_L2CAM_TABLEt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if (( ret = table_size_get(unit, SSW_L2_TABLEt, &l2_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    
    cmp_fid = pConfig->flushByVid;
    cmp_port = pConfig->flushByPort;
    /* translate definition to chip's value  */
    if (ENABLED == pConfig->flushByVid)
        RT_PARAM_CHK(pConfig->vid > RTK_VLAN_ID_MAX, RT_ERR_L2_FID);
    if (ENABLED == pConfig->flushByPort)
    {
        if (ENABLED == pConfig->portOrTrunk)
            RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, pConfig->port), RT_ERR_PORT_ID);
        else
        {
            RT_PARAM_CHK(pConfig->port >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
            if ((ret = dal_ssw_trunk_port_get(unit, pConfig->port, &trunk_portmask)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }   
            
            if ((ret = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
            {
                /* no trunk member */
                RT_ERR(RT_ERR_LA_TRUNK_ID, (MOD_DAL|MOD_L2), "");
                return RT_ERR_LA_TRUNK_ID;
            }
            
            pConfig->port = (uint32)ret;
        }
    }
    
    /* E0005360: Clear L2 entries resides in CAM */
    l2cam_tableSize = l2cam_tableSize - l2_tableSize;
    osal_memset(&null_l2_entry, 0, sizeof(dal_ssw_l2_entry_t));
    null_l2_entry.entry_type = L2_UNICAST;
    l2_index.index_type = L2_IN_CAM;

    L2_SEM_LOCK(unit);  

    for (l2cam_idx = 0; l2cam_idx < l2cam_tableSize; l2cam_idx++)
    {
        if ((_dal_ssw_l2_getL2EntryfromCAM(unit, l2cam_idx, &l2_entry, &isValid) == RT_ERR_OK)
            && (TRUE == isValid) 
            && (L2_UNICAST == l2_entry.entry_type) 
            && (TRUE != l2_entry.unicast.is_static)
            && (TRUE != l2_entry.unicast.auth)) 
        {                                 
            if (cmp_fid == 1 && cmp_port == 1)
            {
               if (l2_entry.unicast.fid != pConfig->vid || l2_entry.unicast.port != pConfig->port)
                  continue;
            }
            else if (cmp_fid == 1)
            {
               if (l2_entry.unicast.fid != pConfig->vid)
                  continue;
            }
            else if (cmp_port == 1)
            {
               if (l2_entry.unicast.port != pConfig->port)
                  continue;
            }

            l2_index.index = l2cam_idx;     
            if ((ret = _dal_ssw_l2_setL2CAMEntry(unit, &null_l2_entry, &l2_index)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }         
        }        
    } 
    /* End of E0005360 */
             
    /* get value from CHIP*/
    if ((ret = reg_read(unit, SSW_L2_TABLE_FLUSH_CONTROLr, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_L2_TABLE_FLUSH_CONTROLr, SSW_FID_CMPf, &cmp_fid, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_L2_TABLE_FLUSH_CONTROLr, SSW_PORT_CMPf, &cmp_port, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_L2_TABLE_FLUSH_CONTROLr, SSW_FIDf, &(pConfig->vid), &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = reg_field_set(unit, SSW_L2_TABLE_FLUSH_CONTROLr, SSW_PORT_NUMf, &(pConfig->port), &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    temp = 1;
    if ((ret = reg_field_set(unit, SSW_L2_TABLE_FLUSH_CONTROLr, SSW_ACTf, &temp, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    /* program value to CHIP*/
    if ((ret = reg_write(unit, SSW_L2_TABLE_FLUSH_CONTROLr, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
   
    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_ucastAddr_flush */


/* Function Name:
 *      dal_ssw_l2_portLearningCnt_get
 * Description:
 *      Get the mac learning counts of the port from the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pMac_cnt - pointer buffer of mac learning counts of the port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The mac learning counts only calculate dynamic mac numbers.
 *      2. RTL8329/RTL8389 chip have support the counter per port, but don't provide
 *         API to get right now.
 */
int32
dal_ssw_l2_portLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pMac_cnt), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    L2_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, learned_l2_entry_counter_regidx[port], (uint32 )lrncnt_fieldidx[port], pMac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    L2_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac_cnt=%d", *pMac_cnt);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_portLearningCnt_get */


/* Function Name:
 *      dal_ssw_l2_portLimitLearningCnt_get
 * Description:
 *      Get the maximum mac learning counts of the port from the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pMac_cnt - pointer buffer of maximum mac learning counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The maximum mac learning counts only limit for dynamic learning mac
 *      address, not apply to static mac address.
 *      2. Set the mac_cnt to 0 mean disable learning in the port.
 */
int32
dal_ssw_l2_portLimitLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pMac_cnt), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    L2_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, limited_l2_entry_control_regidx[port], (uint32 )limit_lrncnt_fieldidx[port], pMac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    L2_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac_cnt=%d", *pMac_cnt);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_portLimitLearningCnt_get */


/* Function Name:
 *      dal_ssw_l2_portLimitLearningCnt_set
 * Description:
 *      Set the maximum mac learning counts of the port to the specified device.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mac_cnt - maximum mac learning counts
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_LIMITED_L2ENTRY_NUM - invalid limited L2 entry number
 * Note:
 *      1. The maximum mac learning counts only limit for dynamic learning mac
 *      address, not apply to static mac address.
 *      2. Set the mac_cnt to 0 mean disable learning in the port.
 */
int32
dal_ssw_l2_portLimitLearningCnt_set(uint32 unit, rtk_port_t port, uint32 mac_cnt)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, mac_cnt=%d", 
           unit, port, mac_cnt);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((mac_cnt > HAL_L2_LEARN_LIMIT_CNT_MAX(unit)) && (mac_cnt != HAL_L2_LEARN_LIMIT_CNT_DISABLE(unit)), RT_ERR_LIMITED_L2ENTRY_NUM);
    
    L2_SEM_LOCK(unit);
    
    /* programming value into CHIP*/
    if ((ret = reg_field_write(unit, limited_l2_entry_control_regidx[port], (uint32 )limit_lrncnt_fieldidx[port], &mac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_portLimitLearningCnt_set */


/* Function Name:
 *      dal_ssw_l2_portLimitLearningCntAction_get
 * Description:
 *      Get the action when over learning maximum mac counts of the port from the specified device.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAction - pointer buffer of action when over learning maximum mac counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The action symbol as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
int32
dal_ssw_l2_portLimitLearningCntAction_get(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction)
{
    int32   ret;
    uint32  l2_act;
    uint32  l2_trap;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    
    L2_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, limited_l2_entry_control_regidx[port], (uint32 )limit_lrnact_fieldidx[port], &l2_act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = reg_field_read(unit, SSW_VLAN_CONTROLr, (uint32 )limit_lrntrap_fieldidx[port], &l2_trap)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    L2_SEM_UNLOCK(unit);
    
    /* translate register value to action */
    if ((0 == l2_act) && (0 == l2_trap))
    {
        *pAction = LIMIT_LEARN_CNT_ACTION_DROP;
    } 
    else if ((1 == l2_act) && (0 == l2_trap))
    {
        *pAction = LIMIT_LEARN_CNT_ACTION_FORWARD;
    } 
    else if ((1 == l2_act) && (1 == l2_trap))
    {
        *pAction = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    } 
    else 
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_L2), "");
        return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAction=%d", *pAction);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_portLimitLearningCntAction_get */


/* Function Name:
 *      dal_ssw_l2_portLimitLearningCntAction_set
 * Description:
 *      Set the action when over learning maximum mac counts of the port to the specified device.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      action - action when over learning maximum mac counts
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      1. The action symbol as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
int32
dal_ssw_l2_portLimitLearningCntAction_set(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t action)
{
    int32   ret;
    uint32  l2_act;
    uint32  l2_trap;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, action=%d",
           unit, port, action);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(action >= LIMIT_LEARN_CNT_ACTION_END, RT_ERR_INPUT);
    
    switch (action)
    {
        case LIMIT_LEARN_CNT_ACTION_DROP:
            l2_act = 0;
            l2_trap = 0;
            break;
        case LIMIT_LEARN_CNT_ACTION_FORWARD:
            l2_act = 1;
            l2_trap = 0;
            break;
        case LIMIT_LEARN_CNT_ACTION_TO_CPU:
            l2_act = 1;
            l2_trap = 1;
            break;
        default:
            return RT_ERR_INPUT;
    }
    
    L2_SEM_LOCK(unit);
    
    /* programming value to CHIP*/
    if ((ret = reg_field_write(unit, limited_l2_entry_control_regidx[port], (uint32 )limit_lrnact_fieldidx[port], &l2_act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = reg_field_write(unit, SSW_VLAN_CONTROLr, (uint32 )limit_lrntrap_fieldidx[port], &l2_trap)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_portLimitLearningCntAction_set */


/* Function Name:
 *      dal_ssw_l2_aging_get
 * Description:
 *      Get the dynamic address aging time from the specified device.
 * Input:
 *      unit        - unit id
 * Output:
 *      pAging_time - pointer buffer of aging time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Get aging_time as 0 mean disable aging mechanism. (seconds)
 */
int32
dal_ssw_l2_aging_get(uint32 unit, uint32 *pAging_time)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);    
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pAging_time), RT_ERR_NULL_POINTER);
    
    *pAging_time = 300; /* In rtl8389 or rtl8329, aging time is fixed */
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAging_time=%ld", *pAging_time);  
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_aging_get */


/* Function Name:
 *      dal_ssw_l2_aging_set
 * Description:
 *      Set the dynamic address aging time to the specified device.
 * Input:
 *      unit       - unit id
 *      aging_time - aging time
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      1. RTL8329/RTL8389 aging time is not configurable.
 *      2. apply aging_time as 0 mean disable aging mechanism.
 */
int32
dal_ssw_l2_aging_set(uint32 unit, uint32 aging_time)
{
    return RT_ERR_CHIP_NOT_SUPPORTED;
} /* end of dal_ssw_l2_aging_set */


/* Module Name    : L2      */
/* Sub-module Name: Unicast */

/* Function Name:
 *      dal_ssw_l2_addr_add
 * Description:
 *      Add L2 entry to ASIC.
 * Input:
 *      unit      - unit id
 *      pL2_addr - L2 entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vlan id
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_INPUT             - invalid input parameter
 *      RT_ERR_L2_NO_EMPTY_ENTRY - no empty entry in L2 table
 * Note:
 *      Need to initialize L2 entry before add it.
 */
int32
dal_ssw_l2_addr_add(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr)
{
    int32               ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pL2_addr=%x"
           , unit, pL2_addr); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pL2_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pL2_addr->mac.octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, pL2_addr->port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pL2_addr->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2)
            , "unit=%d, vid=%d, port=%d, flags=%x, trk_gid=%d, state=%d, auth=%x"
            , unit, pL2_addr->vid, pL2_addr->port, pL2_addr->flags
            , pL2_addr->trk_gid, pL2_addr->state, pL2_addr->auth); 
    
    if (!(pL2_addr->flags & RTK_L2_UCAST_FLAG_TRUNK_PORT))
    {
        for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
        {
            if (dal_ssw_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
            {
                if ((first_trunkMember = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
                {
                    /* no trunk member */
                    continue;
                }
                
                if ((RTK_PORTMASK_IS_PORT_SET(trunk_portmask, pL2_addr->port)) &&
                    !(first_trunkMember == pL2_addr->port))
                {
                    /* this port is trunk member and not first trunk member, not allow to add */
                    RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_PORT_ID;
                }
            }
        }
    }
    else
    {
        RT_PARAM_CHK(pL2_addr->trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
        /* Transfer pL2_addr->trk_gid to represent port and update pL2_addr->port */
        if ((ret = dal_ssw_trunk_port_get(unit, pL2_addr->trk_gid, &trunk_portmask)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }   
        
        if ((ret = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
        {
            /* no trunk member */
            RT_ERR(RT_ERR_LA_TRUNK_ID, (MOD_DAL|MOD_L2), "");
            return RT_ERR_LA_TRUNK_ID;
        }
        pL2_addr->port = (uint32)ret;
    }
    
    /* search exist or free entry */    
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid    = pL2_addr->vid;
    osal_memcpy(&l2_entry.unicast.mac, &(pL2_addr->mac), sizeof(rtk_mac_t));
    
    L2_SEM_LOCK(unit);

    ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry);
    if (ret == RT_ERR_L2_NO_EMPTY_ENTRY)
    {
        if ((pL2_addr->add_op_flags & RTK_L2_ADD_OP_FLAG_REPLACE_DYNAMIC) == 0)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
        else
        {
            ret = _dal_ssw_l2_getFirstDynamicEntry(unit, &l2_entry, &index_entry);
            if (ret == RT_ERR_L2_ENTRY_NOTFOUND)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            l2_entry.entry_type = L2_UNICAST;
            l2_entry.unicast.fid = pL2_addr->vid;
            osal_memcpy(&l2_entry.unicast.mac.octet[0], &pL2_addr->mac.octet[0], sizeof(rtk_mac_t));
        }
    }
    
    if (index_entry.index_type == L2_IN_HASH)
        pL2_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pL2_addr->l2_idx = (1 << 14) + index_entry.index;
    
    /* fill content */
    osal_memcpy(&l2_entry.unicast.mac, &(pL2_addr->mac), sizeof(rtk_mac_t));
    l2_entry.unicast.fid        = pL2_addr->vid;
    l2_entry.unicast.port       = pL2_addr->port;
    l2_entry.unicast.aging      = 3;
    l2_entry.unicast.sablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_SA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.dablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_DA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.auth       = pL2_addr->auth;
    l2_entry.unicast.is_static  = (pL2_addr->flags & RTK_L2_UCAST_FLAG_STATIC) ? TRUE: FALSE;
    
    if (!l2_entry.unicast.is_static)
        l2_entry.unicast.aging  = 3;

    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_ssw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_ssw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    } 
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_ssw_l2_addr_add */

/* Function Name:
 *      dal_ssw_l2_authAddr_add
 * Description:
 *      Add a L2 unicast auth address entry to the specified device.
 * Input:
 *      unit      - unit id
 *      vid       - vlan id
 *      pMac      - mac address
 *      port      - port id
 *      is_static - the attribute of mac address is static or dynamic.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_L2_NO_EMPTY_ENTRY - no empty entry in L2 table
 * Note:
 *      1. vid is same as fid in IVL mode.
 */
int32
dal_ssw_l2_authAddr_add(
    uint32      unit,
    rtk_vlan_t  vid,
    rtk_mac_t   *pMac,
    rtk_port_t  port,
    uint32      is_static)
{
    int32 ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d, \
           port=%d, is_static=%d", unit, vid, port, is_static);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pMac, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMac->octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x",
           pMac->octet[0], pMac->octet[1], pMac->octet[2],
           pMac->octet[3], pMac->octet[4], pMac->octet[5]);
    
    /* search exist or free entry */    
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid    = vid;
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    /* fill content */
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));
    l2_entry.unicast.fid    = vid;
    l2_entry.unicast.port    = port;
    l2_entry.unicast.aging    = 3;
    l2_entry.unicast.sablock    = 0;
    l2_entry.unicast.dablock    = 0;
    l2_entry.unicast.auth    = 1;
    l2_entry.unicast.is_static    = is_static;
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_ssw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_ssw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    } 
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_ssw_l2_authAddr_add */


/* Function Name:
 *      dal_ssw_l2_addr_del
 * Description:
 *      Delete a L2 unicast address entry from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. vid is same as fid in IVL mode.
 *      2. For IVL and SVL co-work mode, need to discuss API late.
 */
int32
dal_ssw_l2_addr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d", 
           unit, vid);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pMac, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMac->octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK((vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x", 
           pMac->octet[0], pMac->octet[1], pMac->octet[2],
           pMac->octet[3], pMac->octet[4], pMac->octet[5]);
    
    /* search exist or free entry */    
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid    = vid;
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);

    if ((ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    /* fill content */
    osal_memset(&l2_entry.unicast.mac, 0, sizeof(rtk_mac_t));
    l2_entry.unicast.fid    = 0;
    l2_entry.unicast.port    = 0;
    l2_entry.unicast.aging    = 0;
    l2_entry.unicast.sablock    = 0;
    l2_entry.unicast.dablock    = 0;
    l2_entry.unicast.auth    = 0;
    l2_entry.unicast.is_static    = 0;
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_ssw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_ssw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    } 
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_ssw_l2_addr_del */


/* Function Name:
 *      dal_ssw_l2_addr_get
 * Description:
 *      Get a L2 unicast address entry from the specified device.
 * Input:
 *      unit     - unit id
 *      pL2_data - structure of l2 address data
 * Output:
 *      pL2_data - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. vid is same as fid in IVL mode.
 *      2. For IVL and SVL co-work mode, need to discuss API late.
 *      3. The *pL2_data.vid and *pL2_data.mac is input key
 *      4. The *pL2_data.port, *pL2_data.auth, *pL2_data.sa_block, 
 *         *pL2_data.da_block and *pL2_data.is_static is output.
 */
int32
dal_ssw_l2_addr_get(uint32 unit, rtk_l2_ucastAddr_t *pL2_data)
{
    int32 ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d", 
           unit, pL2_data->vid);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pL2_data, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pL2_data->mac.octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK((pL2_data->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x", 
           pL2_data->mac.octet[0], pL2_data->mac.octet[1], pL2_data->mac.octet[2],
           pL2_data->mac.octet[3], pL2_data->mac.octet[4], pL2_data->mac.octet[5]);

    /*Clear pL2_data Info*/
    pL2_data->flags = 0;
    pL2_data->state = 0;

    /* search exist or free entry */    
    l2_entry.entry_type     = L2_UNICAST;
    l2_entry.unicast.fid    = pL2_data->vid;
    osal_memcpy(&l2_entry.unicast.mac, &pL2_data->mac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    
    /* fill content */
    osal_memcpy(&pL2_data->mac, &l2_entry.unicast.mac, sizeof(rtk_mac_t));
    pL2_data->vid       = l2_entry.unicast.fid;
    pL2_data->port      = l2_entry.unicast.port;
    if (index_entry.index_type == L2_IN_HASH)
        pL2_data->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pL2_data->l2_idx = (1 << 14) + index_entry.index;

    for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
    {
        if (dal_ssw_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
        {
            if ((first_trunkMember = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
            {
                /* no trunk member */
                continue;
            }

            if ((RTK_PORTMASK_IS_PORT_SET(trunk_portmask, pL2_data->port)) &&
                (first_trunkMember == pL2_data->port))
            {
                pL2_data->trk_gid = trk_gid;
                pL2_data->flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
                break;
            }
        }
    }
    if(l2_entry.unicast.sablock)
        pL2_data->flags = RTK_L2_UCAST_FLAG_SA_BLOCK;
    if(l2_entry.unicast.dablock)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
    pL2_data->auth      = l2_entry.unicast.auth;
    if(l2_entry.unicast.is_static)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_STATIC;
    if(l2_entry.unicast.aging == 0)
        pL2_data->isAged = TRUE;
    else
        pL2_data->isAged = FALSE;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "vid=%d, port=%d, sa_block=%d, da_block=%d, auth=%d\
           is_static=%d", pL2_data->vid, pL2_data->port, l2_entry.unicast.sablock, l2_entry.unicast.dablock, l2_entry.unicast.auth, l2_entry.unicast.is_static);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_addr_get */


/* Function Name:
 *      dal_ssw_l2_addr_set
 * Description:
 *      Update content of L2 entry.
 * Input:
 *      unit     - unit id
 *      pL2_addr - L2 entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vlan id
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_INPUT             - invalid input parameter
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
int32
dal_ssw_l2_addr_set(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr)
{
    int32               ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pL2_addr=%x"
           , unit, pL2_addr); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pL2_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pL2_addr->mac.octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, pL2_addr->port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pL2_addr->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2)
            , "unit=%d, vid=%d, port=%d, flags=%x, trk_gid=%d, state=%d, auth=%x"
            , unit, pL2_addr->vid, pL2_addr->port, pL2_addr->flags
            , pL2_addr->trk_gid, pL2_addr->state, pL2_addr->auth); 
    
    if (!(pL2_addr->flags & RTK_L2_UCAST_FLAG_TRUNK_PORT))
    {
        for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
        {
            if (dal_ssw_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
            {
                if ((first_trunkMember = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
                {
                    /* no trunk member */
                    continue;
                }
                
                if ((RTK_PORTMASK_IS_PORT_SET(trunk_portmask, pL2_addr->port)) &&
                    !(first_trunkMember == pL2_addr->port))
                {
                    /* this port is trunk member and not first trunk member, not allow to add */
                    RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_PORT_ID;
                }
            }
        }
    }
    else
    {
        RT_PARAM_CHK(pL2_addr->trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
        /* Transfer pL2_addr->trk_gid to represent port and update pL2_addr->port */
        if ((ret = dal_ssw_trunk_port_get(unit, pL2_addr->trk_gid, &trunk_portmask)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }   
        
        if ((ret = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
        {
            /* no trunk member */
            RT_ERR(RT_ERR_LA_TRUNK_ID, (MOD_DAL|MOD_L2), "");
            return RT_ERR_LA_TRUNK_ID;
        }
        pL2_addr->port = (uint32)ret;
    }
    
    /* search exist or free entry */    
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid    = pL2_addr->vid;
    osal_memcpy(&l2_entry.unicast.mac, &(pL2_addr->mac), sizeof(rtk_mac_t));
    
    L2_SEM_LOCK(unit);

    if ((ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if (index_entry.index_type == L2_IN_HASH)
        pL2_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pL2_addr->l2_idx = (1 << 14) + index_entry.index;

    /* fill content */
    osal_memcpy(&l2_entry.unicast.mac, &(pL2_addr->mac), sizeof(rtk_mac_t));
    l2_entry.unicast.fid        = pL2_addr->vid;
    l2_entry.unicast.port       = pL2_addr->port;
    l2_entry.unicast.aging      = 3;
    l2_entry.unicast.sablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_SA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.dablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_DA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.auth       = pL2_addr->auth;
    l2_entry.unicast.is_static  = (pL2_addr->flags & RTK_L2_UCAST_FLAG_STATIC) ? TRUE: FALSE;
    
    if (!l2_entry.unicast.is_static)
        l2_entry.unicast.aging = 3;

    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_ssw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_ssw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    } 
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_ssw_l2_addr_set */

/* Function Name:
 *      dal_ssw_l2_addr_delAll
 * Description:
 *      Delete all L2 unicast address entry from the specified device.
 * Input:
 *      unit           - unit id
 *      include_static - include static mac or not?
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_l2_addr_delAll(uint32 unit, uint32 include_static)
{
    int32       ret;
    rtk_port_t  port;
    uint32      busyloop;
    uint32      value;
    rtk_l2_flushCfg_t   flush_config;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, include_static=%d", 
           unit, include_static);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    osal_memset(&flush_config, 0, sizeof(rtk_l2_flushCfg_t));
    flush_config.flushByPort = ENABLED;
    flush_config.portOrTrunk = ENABLED;
    for (port = HAL_GET_MIN_PORT(unit); port < HAL_GET_MAX_PORT(unit); port++)
    {
        if(!HAL_IS_PORT_EXIST(unit, port))
            continue;

        flush_config.port = port;
        if (( ret = dal_ssw_l2_ucastAddr_flush(unit, &flush_config)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        }
        
        /* init value to 1 for preventing misunderstanding when reg_field_read fail */
        value = 1;
        /* will loop for 2 second(200 * 10000 microsecond) */
        for (busyloop = 0; busyloop < 200; busyloop++)
        {
            osal_time_usleep(10000);
            
            /* read value from CHIP*/
            if ((reg_field_read(unit, SSW_L2_TABLE_FLUSH_CONTROLr, SSW_ACTf, &value)) == RT_ERR_OK)
            {
                if (0 == value)
                {
                    break;
                }
            }
        }
    }
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_addr_delAll */


/* Function Name:
 *      dal_ssw_l2_nextValidAddr_get
 * Description:
 *      Get next valid L2 unicast address entry from the specified device.
 * Input:
 *      unit           - unit id
 *      pScan_idx      - currently scan index of l2 table to get next.
 *      include_static - the get type, include static mac or not.
 * Output:
 *      pL2_data       - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. The function will skip valid l2 multicast and ip multicast entry and 
 *         reply next valid L2 unicast address is based on index order of l2 table.
 *      2. Please input -1 for get the first entry of l2 table.
 *      3. The *pScan_idx is the input and also is the output argument.
 */
int32
dal_ssw_l2_nextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    uint32              include_static,
    rtk_l2_ucastAddr_t  *pL2_data)
{
    int32  ret;
    dal_ssw_l2_entry_t  l2_entry;
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK((NULL == pScan_idx), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pL2_data), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((include_static > 1), RT_ERR_NULL_POINTER);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, include_static=%d", 
           unit, *pScan_idx, include_static);
    
    osal_memset(pL2_data, 0, sizeof(rtk_l2_ucastAddr_t));

    L2_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_l2_nextValidAddr_get(unit, pScan_idx, L2_UNICAST, include_static, &l2_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }
    
    L2_SEM_UNLOCK(unit);
    
    osal_memcpy(&pL2_data->mac, &l2_entry.unicast.mac, sizeof(rtk_mac_t));
    pL2_data->vid       = l2_entry.unicast.fid;
    pL2_data->port      = l2_entry.unicast.port;

    pL2_data->l2_idx    = *pScan_idx;

    if(l2_entry.unicast.sablock)
        pL2_data->flags = RTK_L2_UCAST_FLAG_SA_BLOCK;
    if(l2_entry.unicast.dablock)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
    pL2_data->auth      = l2_entry.unicast.auth;
    if(l2_entry.unicast.is_static)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_STATIC;
    if(l2_entry.unicast.aging == 0)
        pL2_data->isAged = TRUE;
    else
        pL2_data->isAged = FALSE;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "vid=%d, port=%d, sa_block=%d, da_block=%d, auth=%d\
           is_static=%d", pL2_data->vid, pL2_data->port, l2_entry.unicast.sablock, l2_entry.unicast.dablock, l2_entry.unicast.auth, l2_entry.unicast.is_static);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_nextValidAddr_get */


/* Module Name    : L2           */
/* Sub-module Name: l2 multicast */

/* Function Name:
 *      dal_ssw_l2_mcastAddr_add
 * Description:
 *      Add L2 multicast entry to ASIC.
 * Input:
 *      unit      - unit id
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize L2 multicast entry before add it.
 */
int32
dal_ssw_l2_mcastAddr_add(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32   ret;
   
    pMcast_addr->fwdIndex = -1; /* for automatically allocate */

    ret = dal_ssw_l2_mcastAddr_add_with_index(unit, pMcast_addr);

    return ret;
    
} /* end of dal_ssw_l2_mcastAddr_add */


/* Function Name:
 *      dal_ssw_l2_mcastAddr_get
 * Description:
 *      Update content of L2 multicast entry.
 * Input:
 *      unit      - unit id
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_ssw_l2_mcastAddr_get(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32 ret;
    
    ret = dal_ssw_l2_mcastAddr_get_with_index(unit, pMcast_addr);
    
    return ret;
} /* end of dal_ssw_l2_mcastAddr_get */

/* Function Name:
 *      dal_ssw_l2_mcastAddr_set
 * Description:
 *      Set a L2 multicast address entry to the specified device.
 * Input:
 *      unit       - unit id
 * Output:
 *      pMcast_addr - pointer to L2 multicast entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
int32
dal_ssw_l2_mcastAddr_set(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32 ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMcast_addr->mac.octet[0] & BITMASK_1B) != 1, RT_ERR_MAC);
    RT_PARAM_CHK((pMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_mcastAddr_set: unit=%d, vid=%d, pMac=%x-%x-%x-%x-%x-%x \
           pPortmask=%x", unit, pMcast_addr->rvid, pMcast_addr->mac.octet[0], pMcast_addr->mac.octet[1], pMcast_addr->mac.octet[2],
           pMcast_addr->mac.octet[3], pMcast_addr->mac.octet[4], pMcast_addr->mac.octet[5], pMcast_addr->portmask.bits[0]);
    
    /* search exist or free entry */    
    l2_entry.entry_type = L2_MULTICAST;
    l2_entry.l2mcast.rvid    = pMcast_addr->rvid;
    osal_memcpy(&l2_entry.l2mcast.mac.octet[0], &pMcast_addr->mac.octet[0], sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);

    ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    if (index_entry.index_type == L2_IN_HASH)
        pMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pMcast_addr->l2_idx = (1 << 14) + index_entry.index;

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
    if ((ret = table_field_set(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, &pMcast_addr->portmask.bits[0], (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    if ((ret = table_write(unit, SSW_MULTICAST_INDEX_TABLEt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_l2_mcastAddr_set */

/* Function Name:
 *      dal_ssw_l2_mcastAddr_del
 * Description:
 *      Delete a L2 multicast address entry from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - multicast mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_L2_HASH_KEY    - invalid L2 Hash key
 *      RT_ERR_L2_EMPTY_ENTRY - the entry is empty(invalid)
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
int32
dal_ssw_l2_mcastAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d", 
           unit, vid);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pMac, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMac->octet[0] & BITMASK_1B) != 1, RT_ERR_MAC);
    RT_PARAM_CHK((vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x", 
           pMac->octet[0], pMac->octet[1], pMac->octet[2], 
           pMac->octet[3], pMac->octet[4], pMac->octet[5]);

    /* search exist or free entry */    
    l2_entry.entry_type = L2_MULTICAST;
    l2_entry.l2mcast.rvid = vid;
    osal_memcpy(&l2_entry.l2mcast.mac, pMac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);
    
    ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    _dal_ssw_l2_freeMcastIdx(unit, (int32)l2_entry.l2mcast.index);
    
    osal_memset(&l2_entry.l2mcast.mac, 0, sizeof(rtk_mac_t));
    l2_entry.l2mcast.rvid    = 0;
    l2_entry.l2mcast.index   = 0;
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_ssw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_ssw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_ssw_l2_mcastAddr_del */

/* Function Name:
 *      dal_ssw_l2_mcastAddr_add_with_index
 * Description:
 *      Add a L2 multicast address entry and multicast index to the specified device.
 * Input:
 *      unit                   - unit id
 *      vid                    - vlan id
 *      pMcast_addr            - content of L2 multicast address entry
 * Output:
 *      pMacast_addr->fwdIndex - index of multicast forwarding entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_HASH_KEY          - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_NO_EMPTY_ENTRY    - no empty entry in L2 table
 * Note:
 *      If fwdIndex is larger than or equal to 0, will use fwdIndex as multicast index 
 *          and won't config portmask.
 *
 *      If fwdIndex is smaller than 0, will allocate a free index and return it. 
 *          It will also config portmask.
 */
 int32
dal_ssw_l2_mcastAddr_add_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32 ret, ret_1;
    int32 fwdIndex;
    dal_ssw_l2_entry_t  l2_entry, dynamic_l2_entry;
    dal_ssw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    RT_PARAM_CHK(NULL == pMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMcast_addr->mac.octet[0] & BITMASK_1B) != 1, RT_ERR_MAC);
    RT_PARAM_CHK(pMcast_addr->fwdIndex >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    RT_PARAM_CHK((pMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_mcastAddr_add_with_index: unit=%d, rvid=%d, pMac=%x-%x-%x-%x-%x-%x \
           pPortmask=%x, pIndex=%d", unit, pMcast_addr->rvid, pMcast_addr->mac.octet[0], pMcast_addr->mac.octet[1], pMcast_addr->mac.octet[2], pMcast_addr->mac.octet[2], 
           pMcast_addr->mac.octet[3], pMcast_addr->mac.octet[4], pMcast_addr->mac.octet[5], pMcast_addr->portmask.bits[0], pMcast_addr->fwdIndex);
    
    /* search exist or free entry */    
    l2_entry.entry_type = L2_MULTICAST;
    l2_entry.l2mcast.rvid    = pMcast_addr->rvid;
    osal_memcpy(&l2_entry.l2mcast.mac, &(pMcast_addr->mac), sizeof(rtk_mac_t));

    ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry);
    if (RT_ERR_OK != ret && RT_ERR_L2_NO_EMPTY_ENTRY != ret)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    if (ret == RT_ERR_L2_NO_EMPTY_ENTRY)
    {
        if ((pMcast_addr->add_op_flags & RTK_L2_ADD_OP_FLAG_REPLACE_DYNAMIC) == 0)
        {   
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
        else
        {
            dynamic_l2_entry.entry_type = L2_MULTICAST;
            dynamic_l2_entry.l2mcast.rvid    = pMcast_addr->rvid;
            osal_memcpy(&dynamic_l2_entry.l2mcast.mac, &(pMcast_addr->mac), sizeof(rtk_mac_t));

            ret_1= _dal_ssw_l2_getFirstDynamicEntry(unit, &dynamic_l2_entry, &index_entry);
            if (ret_1 == RT_ERR_L2_ENTRY_NOTFOUND)
            {   
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret_1;
            }
        }
    }

    if (index_entry.index_type == L2_IN_HASH)
        pMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pMcast_addr->l2_idx = (1 << 14) + index_entry.index;

    if (TRUE == l2_entry.is_entry_exist)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "entry exist");  
        return RT_ERR_L2_ENTRY_EXIST;
    }
    
    fwdIndex = pMcast_addr->fwdIndex;
    
    /* get a free multicast index */
    if ((ret = _dal_ssw_l2_allocMcastIdx(unit, &(pMcast_addr->fwdIndex))) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
    if (fwdIndex < 0)
    { /* Will configure portmask when fwdIndex is allocate automatically */
        if ((ret = table_field_set(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, &(pMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        
        if ((ret = table_write(unit, SSW_MULTICAST_INDEX_TABLEt, (uint32)pMcast_addr->fwdIndex, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
    }
    
    /* fill content */
    osal_memcpy(&l2_entry.l2mcast.mac, &(pMcast_addr->mac), sizeof(rtk_mac_t));
    l2_entry.l2mcast.rvid    = pMcast_addr->rvid;
    l2_entry.l2mcast.index   = (uint32)(pMcast_addr->fwdIndex);
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_ssw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if(L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_ssw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    
    if (RT_ERR_OK != ret)
    {
        _dal_ssw_l2_freeMcastIdx(unit, (int32)l2_entry.l2mcast.index);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_mcastAddr_add_with_index */

/* Function Name:
 *      dal_esw_l2_mcastAddr_get_with_index
 * Description:
 *      Get a L2 multicast address entry and multicast index from the specified device.
 * Input:
 *      unit        - unit id
 * Output:
 *      pMcast_addr - pointer to content of L2 multicast address entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_HASH_KEY          - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_ENTRY_NOTFOUND    - specified entry not found
 * Note:
 *      None.
 */
int32
dal_ssw_l2_mcastAddr_get_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32 ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_mcastAddr_get: unit=%d, vid=%d, pMac=%x-%x-%x-%x-%x-%x", 
           unit, pMcast_addr->rvid, pMcast_addr->mac.octet[0], pMcast_addr->mac.octet[1], pMcast_addr->mac.octet[2], pMcast_addr->mac.octet[2], 
           pMcast_addr->mac.octet[3], pMcast_addr->mac.octet[4], pMcast_addr->mac.octet[5]);

    
    RT_PARAM_CHK(NULL == pMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMcast_addr->mac.octet[0] & BITMASK_1B) != 1, RT_ERR_MAC);
    RT_PARAM_CHK((pMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* search exist or free entry */    
    l2_entry.entry_type     = L2_MULTICAST;
    l2_entry.l2mcast.rvid   = pMcast_addr->rvid;
    osal_memcpy(&l2_entry.l2mcast.mac, &(pMcast_addr->mac), sizeof(rtk_mac_t));

    ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
    if (RT_ERR_OK != ret)
    {
        /* fill content */
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    if (index_entry.index_type == L2_IN_HASH)
        pMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pMcast_addr->l2_idx = (1 << 14) + index_entry.index;
    
    pMcast_addr->fwdIndex = (int32)l2_entry.l2mcast.index;
    
    if ((ret = table_read(unit, SSW_MULTICAST_INDEX_TABLEt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    if ((ret = table_field_get(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, &(pMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_mcastAddr_get: pPortmask=%x", pMcast_addr->portmask.bits[0]);
    
    return ret;
} /* end of dal_ssw_l2_mcastAddr_get_with_index */

/* Function Name:
 *      dal_ssw_l2_nextValidMcastAddr_get
 * Description:
 *      Get next valid L2 multicast address entry from the specified device.
 * Input:
 *      unit      - unit id
 *      pScan_idx - currently scan index of l2 table to get next.
 * Output:
 *      pL2_data  - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. The function will skip valid l2 unicast and ip multicast entry and 
 *         reply next valid L2 multicast address is based on index order of l2 table.
 *      2. Please input -1 for get the first entry of l2 table.
 *      3. The *pScan_idx is the input and also is the output argument.
 */
int32
dal_ssw_l2_nextValidMcastAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    rtk_l2_mcastAddr_t  *pL2_data)
{
    int32  ret;
    dal_ssw_l2_entry_t  l2_entry;
    multicast_index_entry_t mcast_entry;
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK((NULL == pScan_idx), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pL2_data), RT_ERR_NULL_POINTER);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d", 
           unit, *pScan_idx);
    
    L2_SEM_LOCK(unit);
    if ((ret = _dal_ssw_l2_nextValidAddr_get(unit, pScan_idx, L2_MULTICAST, TRUE, &l2_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    pL2_data->rvid = l2_entry.l2mcast.rvid;
    osal_memcpy(&pL2_data->mac, &l2_entry.l2mcast.mac, sizeof(rtk_mac_t));
    pL2_data->fwdIndex = l2_entry.l2mcast.index;
    pL2_data->l2_idx   = *pScan_idx;

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    if ((ret = table_read(unit, SSW_MULTICAST_INDEX_TABLEt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = table_field_get(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, pL2_data->portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    L2_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, rvid=%d, mac=%x-%x-%x-%x-%x-%x, portmask=0x%x", 
           unit, *pScan_idx, pL2_data->rvid, pL2_data->mac.octet[0], pL2_data->mac.octet[1], pL2_data->mac.octet[2], pL2_data->mac.octet[3], 
           pL2_data->mac.octet[4], pL2_data->mac.octet[5], pL2_data->portmask.bits[0]);

    return RT_ERR_OK;
} /* end of dal_ssw_l2_nextValidMcastAddr_get */


/* Module Name    : L2           */
/* Sub-module Name: ip multicast */

/* Function Name:
 *      dal_ssw_l2_ipMcastAddr_add
 * Description:
 *      Add IP multicast entry to ASIC.
 * Input:
 *      unit      - unit id
 *      pIpmcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
int32
dal_ssw_l2_ipMcastAddr_add(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    int32 ret;
    
    pIpmcast_addr->fwdIndex = -1;

    ret = dal_ssw_l2_ipMcastAddr_add_with_index(unit, pIpmcast_addr);
    
    return ret;
} /* end of dal_ssw_l2_ipMcastAddr_add */


/* Function Name:
 *      dal_ssw_l2_ipMcastAddr_get
 * Description:
 *      Get IP multicast entry on specified dip and sip.
 * Input:
 *      unit      - unit id
 *      pIpmcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
int32
dal_ssw_l2_ipMcastAddr_get(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    int32 ret;
    
    ret = dal_ssw_l2_ipMcastAddr_get_with_index(unit, pIpmcast_addr);
    
    return ret;
} /* end of dal_ssw_l2_ipMcastAddr_get */

/* Function Name:
 *      dal_ssw_l2_ipMcastAddr_set
 * Description:
 *      Update content of IP multicast entry.
 * Input:
 *      unit       - unit id
 *      pIpmcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_IPV4_ADDRESS - Invalid IPv4 address
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
int32
dal_ssw_l2_ipMcastAddr_set(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    int32 ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pIpmcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((pIpmcast_addr->dip >>28) & BITMASK_4B) != 0xE, RT_ERR_IPV4_ADDRESS);
    RT_PARAM_CHK((pIpmcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_ipMcastAddr_set: unit=%d, sip=%x, dip=%x, vid=%d, pPortmask=%x", 
           unit, pIpmcast_addr->sip, pIpmcast_addr->dip, pIpmcast_addr->rvid, pIpmcast_addr->portmask.bits[0]);

    /* search exist or free entry */    
    l2_entry.entry_type = IP_MULTICAST;
    l2_entry.ipmcast.rvid    = pIpmcast_addr->rvid;
    l2_entry.ipmcast.dip = pIpmcast_addr->dip;
    l2_entry.ipmcast.sip = pIpmcast_addr->sip;

    L2_SEM_LOCK(unit);

    ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    if (index_entry.index_type == L2_IN_HASH)
        pIpmcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pIpmcast_addr->l2_idx = (1 << 14) + index_entry.index;

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
    if ((ret = table_field_set(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, pIpmcast_addr->portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
         RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
         return ret;
    }
    
    if ((ret = table_write(unit, SSW_MULTICAST_INDEX_TABLEt, l2_entry.ipmcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_ssw_l2_ipMcastAddr_set */

/* Function Name:
 *      dal_ssw_l2_ipMcastAddr_del
 * Description:
 *      Delete a L2 ip multicast address entry from the specified device.
 * Input:
 *      unit - unit id
 *      sip  - source ip address
 *      dip  - destination ip address
 *      vid  - vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_L2_HASH_KEY    - invalid L2 Hash key
 *      RT_ERR_L2_EMPTY_ENTRY - the entry is empty(invalid)
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. In vlan unaware mode (SVL), the vid will be ignore, suggest to 
 *         input vid=0 in vlan unaware mode.
 *      2. In vlan aware mode (IVL), the vid will be care.
 */
int32
dal_ssw_l2_ipMcastAddr_del(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_vlan_t vid)
{
    int32 ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, sip=%x, dip=%x, vid=%d", 
           unit, sip, dip, vid);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK((vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* search exist or free entry */    
    l2_entry.entry_type   = IP_MULTICAST;
    l2_entry.ipmcast.rvid = vid;
    l2_entry.ipmcast.dip  = dip;
    l2_entry.ipmcast.sip  = sip;
    
    L2_SEM_LOCK(unit);

    ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    _dal_ssw_l2_freeMcastIdx(unit, (int32)l2_entry.l2mcast.index);
    
    l2_entry.ipmcast.rvid    = 0;
    l2_entry.ipmcast.dip     = 0;
    l2_entry.ipmcast.sip     = 0;
    l2_entry.ipmcast.index     = 0;
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_ssw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_ssw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_ssw_l2_ipMcastAddr_del */

/* Function Name:
 *      dal_esw_l2_ipMcastAddr_add_with_index
 * Description:
 *      Add a IP multicast address entry and multicast index to the specified device.
 * Input:
 *      unit                     - unit id
 *      vid                      - vlan id
 *      pIpMcast_addr            - content of IP multicast address entry
 * Output:
 *      pIpMacast_addr->fwdIndex - index of multicast forwarding entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_HASH_KEY          - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_NO_EMPTY_ENTRY    - no empty entry in L2 table
 * Note:
 *      If fwdIndex is larger than or equal to 0, will use fwdIndex as multicast index 
 *          and won't config portmask.
 *
 *      If fwdIndex is smaller than 0, will allocate a free index and return it. 
 *          It will also config portmask.
 */
int32
dal_ssw_l2_ipMcastAddr_add_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    int32 ret, ret_1;
    int32 fwdIndex;
    dal_ssw_l2_entry_t  l2_entry, dynamic_l2_entry;
    dal_ssw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;
    
    RT_PARAM_CHK(NULL == pIpMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pIpMcast_addr->fwdIndex >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    RT_PARAM_CHK((pIpMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_ipMcastAddr_add_with_index: unit=%d, sip=%x, dip=%x, vid=%d, pPortmask=%x", 
           unit, pIpMcast_addr->sip, pIpMcast_addr->dip, pIpMcast_addr->rvid, pIpMcast_addr->portmask.bits[0]);

    /* search exist or free entry */    
    l2_entry.entry_type = IP_MULTICAST;
    l2_entry.ipmcast.rvid    = pIpMcast_addr->rvid;
    l2_entry.ipmcast.dip     = pIpMcast_addr->dip;
    l2_entry.ipmcast.sip     = pIpMcast_addr->sip;

    ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry);
    
    if (RT_ERR_OK != ret && RT_ERR_L2_NO_EMPTY_ENTRY != ret)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    if (ret == RT_ERR_L2_NO_EMPTY_ENTRY)
    {
        if ((pIpMcast_addr->add_op_flags & RTK_L2_ADD_OP_FLAG_REPLACE_DYNAMIC) == 0)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
        else
        {
            dynamic_l2_entry.entry_type  = IP_MULTICAST;
            dynamic_l2_entry.ipmcast.dip = pIpMcast_addr->dip;
            dynamic_l2_entry.ipmcast.sip = pIpMcast_addr->sip;

            ret_1 = _dal_ssw_l2_getFirstDynamicEntry(unit, &dynamic_l2_entry, &index_entry);
            if (ret_1 == RT_ERR_L2_ENTRY_NOTFOUND)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret_1;
            }
        }
    }

    if (index_entry.index_type == L2_IN_HASH)
        pIpMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pIpMcast_addr->l2_idx = (1 << 14) + index_entry.index;

    if (TRUE == l2_entry.is_entry_exist)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "entry exist");  
        return RT_ERR_L2_ENTRY_EXIST;
    }
    
    fwdIndex = pIpMcast_addr->fwdIndex;
    
    /* get a free multicast index or increase reference count of mcast index */
    if ((ret = _dal_ssw_l2_allocMcastIdx(unit, &(pIpMcast_addr->fwdIndex))) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
    if (fwdIndex < 0)
    { /* Will configure portmask when fwdIndex is allocate automatically */
        if ((ret = table_field_set(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, &(pIpMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
            return ret;
        }
        
        if ((ret = table_write(unit, SSW_MULTICAST_INDEX_TABLEt, (uint32)(pIpMcast_addr->fwdIndex), (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
            return ret;
        }
    }
    
    l2_entry.ipmcast.rvid    = pIpMcast_addr->rvid;
    l2_entry.ipmcast.dip     = pIpMcast_addr->dip;
    l2_entry.ipmcast.sip     = pIpMcast_addr->sip;
    l2_entry.ipmcast.index     = (uint32)(pIpMcast_addr->fwdIndex);
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_ssw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if(L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_ssw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    
    if (RT_ERR_OK != ret)
    {
        _dal_ssw_l2_freeMcastIdx(unit, (int32)l2_entry.l2mcast.index);
        return ret;
    }
    
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_ipMcastAddr_add_with_index */
    
/* Function Name:
 *      dal_ssw_l2_ipMcastAddr_get_with_index
 * Description:
 *      Get a IP multicast address entry and multicast index from the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pIpMcast_addr - pointer to content of IP multicast address entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_HASH_KEY          - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_ENTRY_NOTFOUND    - specified entry not found
 * Note:
 *      None.
 */
int32
dal_ssw_l2_ipMcastAddr_get_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    int32 ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_ipMcastAddr_get: unit=%d, sip=%x, dip=%x, vid=%d", 
           unit, pIpMcast_addr->sip, pIpMcast_addr->dip, pIpMcast_addr->rvid);
    
    RT_PARAM_CHK(NULL == pIpMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pIpMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* search exist or free entry */    
    l2_entry.entry_type = IP_MULTICAST;
    l2_entry.ipmcast.rvid    = pIpMcast_addr->rvid;
    l2_entry.ipmcast.dip     = pIpMcast_addr->dip;
    l2_entry.ipmcast.sip     = pIpMcast_addr->sip;

    ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
    if (RT_ERR_OK != ret)
    {
        return ret;
    }
    
    if (index_entry.index_type == L2_IN_HASH)
        pIpMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pIpMcast_addr->l2_idx = (1 << 14) + index_entry.index;

    pIpMcast_addr->fwdIndex = (int32) l2_entry.ipmcast.index;
    
    if ((ret = table_read(unit, SSW_MULTICAST_INDEX_TABLEt, l2_entry.ipmcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    if ((ret = table_field_get(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, &(pIpMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_ipMcastAddr_get: pPortmask=%x", 
           pIpMcast_addr->portmask.bits[0]);
    
    return ret;
} /* end of dal_ssw_l2_ipMcastAddr_get_with_index */


/* Function Name:
 *      dal_ssw_l2_nextValidIpMcastAddr_get
 * Description:
 *      Get next valid L2 ip multicast address entry from the specified device.
 * Input:
 *      unit      - unit id
 *      pScan_idx - currently scan index of l2 table to get next.
 * Output:
 *      pL2_data  - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. The function will skip valid l2 unicast and multicast entry and 
 *         reply next valid L2 ip multicast address is based on index order of l2 table.
 *      2. Please input -1 for get the first entry of l2 table.
 *      3. The *pScan_idx is the input and also is the output argument.
 */
int32
dal_ssw_l2_nextValidIpMcastAddr_get(
    uint32                  unit,
    int32                   *pScan_idx,
    rtk_l2_ipMcastAddr_t    *pL2_data)
{
    int32  ret;
    dal_ssw_l2_entry_t      l2_entry;
    multicast_index_entry_t mcast_entry;
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK((NULL == pScan_idx), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pL2_data), RT_ERR_NULL_POINTER);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d", 
           unit, *pScan_idx);
    
    L2_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_l2_nextValidAddr_get(unit, pScan_idx, IP_MULTICAST, TRUE, &l2_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    pL2_data->rvid = l2_entry.ipmcast.rvid;
    pL2_data->dip  = l2_entry.ipmcast.dip;
    pL2_data->sip  = l2_entry.ipmcast.sip;
    pL2_data->fwdIndex = l2_entry.ipmcast.index;
    pL2_data->l2_idx = *pScan_idx;

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    if ((ret = table_read(unit, SSW_MULTICAST_INDEX_TABLEt, l2_entry.ipmcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = table_field_get(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, pL2_data->portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    L2_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, rvid=%d, dip=%x, sip=%x, portmask=0x%x", 
           unit, *pScan_idx, pL2_data->rvid, pL2_data->dip, pL2_data->sip, pL2_data->portmask.bits[0]);

    return RT_ERR_OK;
} /* end of dal_ssw_l2_nextValidIpMcastAddr_get */

/* Function Name:
 *      dal_ssw_l2_mcastFwdIndex_alloc
 * Description:
 *      Allocate index for multicast forwarding entry
 * Input:
 *      unit           - unit id
 *      pFwdIndex      - pointer to index of multicast forwarding entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX       - invalid index of multicast forwarding entry
 *      RT_ERR_L2_MCAST_FWD_ENTRY_EXIST - index of forwarding entry is used.
 * Note:
 *      If *pFwdIndex is larger than or equal to 0, will use *pFwdIndex as multicast index.
 *      If *pFwdIndex is smaller than 0, will allocate a free index and return it.
 */
int32
dal_ssw_l2_mcastFwdIndex_alloc(
    uint32          unit,
    int32           *pFwdIndex)
{
    
    RT_PARAM_CHK(NULL == pFwdIndex, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(*pFwdIndex >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", 
           unit, *pFwdIndex);
    
    if (*pFwdIndex >= 0)
    {
        if (RT_ERR_OK == _dal_ssw_l2_isMcastIdxUsed(unit, *pFwdIndex))
        {
            return RT_ERR_L2_MCAST_FWD_ENTRY_EXIST;
        }
    }
    
    return _dal_ssw_l2_allocMcastIdx(unit, pFwdIndex);
} /* end of dal_ssw_l2_mcastFwdIndex_alloc */

/* Function Name:
 *      dal_ssw_l2_mcastFwdIndex_free
 * Description:
 *      Free index for multicast forwarding entry
 * Input:
 *      unit       - unit id
 *      index      - index of multicast forwarding entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                      - invalid unit id
 *      RT_ERR_L2_MULTI_FWD_INDEX           - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_MCAST_FWD_ENTRY_NOT_EXIST - index of forwarding entry is not exist
 * Note:
 */
int32
dal_ssw_l2_mcastFwdIndex_free(
    uint32          unit,
    int32           index)
{
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", 
           unit, index);

    RT_PARAM_CHK((index >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index) || index < 0, RT_ERR_L2_MULTI_FWD_INDEX);
    
    if (RT_ERR_OK != _dal_ssw_l2_isMcastIdxUsed(unit, index))
    {
        return RT_ERR_L2_MCAST_FWD_ENTRY_NOT_EXIST;
    }
    
    return _dal_ssw_l2_freeMcastIdx(unit, index);
} /* end of dal_ssw_l2_mcastFwdIndex_free */

/* Function Name:
 *      dal_ssw_l2_mcastFwdIndexFreeCount_get
 * Description:
 *      Get free count of multicast forwarding entry
 * Input:
 *      unit       - unit id
 *      pFreeCount - pointer to free count of multicast forwarding entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_ssw_l2_mcastFwdIndexFreeCount_get(uint32 unit, uint32 *pFreeCount)
{
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pFreeCount, RT_ERR_NULL_POINTER);
    
    L2_SEM_LOCK(unit);
    *pFreeCount = mcast_idx_pool[unit].free_entry_count;
    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_mcastFwdIndexFreeCount_get */

/* Function Name:
 *      dal_ssw_l2_mcastFwdPortmask_set
 * Description:
 *      Get portmask of multicast forwarding entry
 * Input:
 *      unit       - unit id
 *      index      - index of multicast forwarding portmask
 *      *pPortmask - pointer buffer of ip multicast ports
 *      crossVlan  - cross vlan flag of ip multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 *      The crossVlan will be ignore in ssw.
 */
int32
dal_ssw_l2_mcastFwdPortmask_set(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          crossVlan)
{
    int32 ret;
    multicast_index_entry_t mcast_entry;
    
    RT_PARAM_CHK(NULL == pPortmask, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(index >= mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_mcastFwdPortmask_set: unit=%d, index=%d, pPortmask=%x", 
           unit, index, pPortmask->bits[0]);
    
    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
    if ((ret = table_field_set(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, &(pPortmask->bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
         RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
         return ret;
    }
    
    if ((ret = table_write(unit, SSW_MULTICAST_INDEX_TABLEt, (uint32)index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_mcastFwdPortmask_set: pPortmask=%x", 
           pPortmask->bits[0]);

    return RT_ERR_OK;
} /* end of dal_ssw_l2_mcastFwdPortmask_set */

/* Function Name:
 *      dal_ssw_l2_mcastFwdPortmask_get
 * Description:
 *      Get portmask of multicast forwarding entry
 * Input:
 *      unit       - unit id
 *      index      - index of multicast forwarding portmask
 * Output:
 *      *pPortmask - pointer buffer of ip multicast ports
 *      *pCrossVlan - pointer of cross vlan flag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 *      The pCrossVlan will be ignore in ssw.
 */
int32
dal_ssw_l2_mcastFwdPortmask_get(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          *pCrossVlan)
{
    int32 ret;
    multicast_index_entry_t mcast_entry;
    
    RT_PARAM_CHK(NULL == pPortmask, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(index >= mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_mcastFwdPortmask_get: unit=%d, pPortmask=%x", 
           unit, index);
    
    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
    if ((ret = table_read(unit, SSW_MULTICAST_INDEX_TABLEt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    if ((ret = table_field_get(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, &(pPortmask->bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_ssw_l2_mcastFwdPortmask_get: pPortmask=%x", 
           pPortmask->bits[0]);
    
    return RT_ERR_OK;
} /* end of dal_ssw_l2_mcastFwdPortmask_get */
    


/* Function Name:
 *      dal_ssw_l2_cpuMacAddr_add
 * Description:
 *      Add a CPU mac adress of the vlan id to the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - cpu mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_VLAN_VID - invalid vid
 *      RT_ERR_L2_NO_EMPTY_ENTRY - no empty entry in L2 table
 * Note:
 *      None
 */
int32
dal_ssw_l2_cpuMacAddr_add(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;
    
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d", 
           unit, vid);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pMac, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMac->octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK((vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x", 
           pMac->octet[0], pMac->octet[1], pMac->octet[2], 
           pMac->octet[3], pMac->octet[4], pMac->octet[5]);

    /* search exist or free entry */    
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid    = vid;
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    /* fill content */
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));
    l2_entry.unicast.fid    = vid;
    if (-1 == HAL_GET_CPU_PORT(unit))
    {
        L2_SEM_UNLOCK(unit);
        return RT_ERR_L2_NO_CPU_PORT;
    }
    l2_entry.unicast.port    = HAL_GET_CPU_PORT(unit);
    l2_entry.unicast.aging    = 3;
    l2_entry.unicast.sablock    = 0;
    l2_entry.unicast.dablock    = 0;
    l2_entry.unicast.auth    = 0;
    l2_entry.unicast.is_static    = 1;
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_ssw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_ssw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
    
} /* end of dal_ssw_l2_cpuMacAddr_add */


/* Function Name:
 *      dal_ssw_l2_cpuMacAddr_del
 * Description:
 *      Delete a CPU mac adress of the vlan id from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - cpu mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_VLAN_VID - invalid vid
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
int32
dal_ssw_l2_cpuMacAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_ssw_l2_entry_t  l2_entry;
    dal_ssw_l2_index_t  index_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d", 
           unit, vid);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pMac, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMac->octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK((vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x", 
           pMac->octet[0], pMac->octet[1], pMac->octet[2], 
           pMac->octet[3], pMac->octet[4], pMac->octet[5]);

    /* search exist or free entry */    
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid    = vid;
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);
    
    if ((ret = _dal_ssw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    /* fill content */
    osal_memset(&l2_entry.unicast.mac, 0, sizeof(rtk_mac_t));
    l2_entry.unicast.fid    = 0;
    l2_entry.unicast.port    = 0;
    l2_entry.unicast.aging    = 0;
    l2_entry.unicast.sablock    = 0;
    l2_entry.unicast.dablock    = 0;
    l2_entry.unicast.auth    = 0;
    l2_entry.unicast.is_static    = 0;
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_ssw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_ssw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_ssw_l2_cpuMacAddr_del */


/* Module: static function */
/* Function Name:
 *      _dal_ssw_l2_init_config
 * Description:
 *      Initialize config of l2 module for the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize l2 module before calling this API.
 */
static int32
_dal_ssw_l2_init_config(uint32 unit)
{
    int32   ret;
    
    
    if ((ret = dal_ssw_l2_flushLinkDownPortAddrEnable_set(unit, RTK_DEFAULT_L2_FLUSH_LINKDOWN_MAC)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_l2_init_config */

/* Function Name:
 *      _dal_ssw_l2_nextValidAddr_get
 * Description:
 *      Get next valid L2 unicast, multicast or ip multicast address entry from the specified device.
 * Input:
 *      unit           - unit id
 *      pScan_idx      - currently scan index of l2 table to get next.
 *      type           - address type
 *      include_static - the get type, include static mac or not.
 * Output:
 *      pL2_data       - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 *      RT_ERR_OUT_OF_RANGE      - input parameter out of range
 * Note:
 *      1. The function will skip valid l2 multicast and ip multicast entry and 
 *         reply next valid L2 unicast address is based on index order of l2 table.
 *      2. Please input -1 for get the first entry of l2 table.
 *      3. The *pScan_idx is the input and also is the output argument.
 *      4. Valid type value is L2_UNICAST, L2_MULTICAST and IP_MULTICAST
 */
static int32
_dal_ssw_l2_nextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    uint32              type,
    uint32              include_static,
    dal_ssw_l2_entry_t  *pL2_data)
{
    int32   ret;
    dal_ssw_l2_entry_t  l2_entry;
    uint32  l2_idx;
    uint32  l2cam_idx;
    uint32  l2_tableSize;
    uint32  l2cam_tableSize;
    uint32  hashkey, location;
    uint32  isValid;
    uint32  found;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, type=%d, include_static=%d", 
           unit, *pScan_idx, type, include_static);
    
    if ((ret = table_size_get(unit, SSW_L2CAM_TABLEt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = table_size_get(unit, SSW_L2_TABLEt, &l2_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    RT_PARAM_CHK(*pScan_idx >= (int32)(l2_tableSize+l2cam_tableSize), RT_ERR_OUT_OF_RANGE);
    
    l2cam_idx = 0;
    if (*pScan_idx < 0)
    {
        l2_idx = 0;
    }
    else
    {
        l2_idx = *pScan_idx + 1;
    }
    
    found = FALSE;    
    for (; l2_idx < l2_tableSize; l2_idx++)
    {
        hashkey = l2_idx >> 2;
        location = l2_idx & BITMASK_2B;
        
        if ((_dal_ssw_l2_getL2EntryfromHash(unit, hashkey, location, &l2_entry, &isValid) == RT_ERR_OK)
            && (TRUE == isValid) && (type == l2_entry.entry_type))
        {
            if (L2_UNICAST == l2_entry.entry_type)
            {
                if ((FALSE == include_static) && (1 == l2_entry.unicast.is_static))
                {
                    /* if not need to include static and this entry is static, just skip this entry */
                    continue;
                }
            }
            
            found = TRUE;
            *pScan_idx = l2_idx;
            break;
        }
    }
    
    /* in 8389, l2cam size that get from HAL is cam size + l2 table size */
    l2cam_tableSize = l2cam_tableSize - l2_tableSize;
    
    if (FALSE == found)
    {
        for (l2cam_idx = l2_idx - l2_tableSize; l2cam_idx < l2cam_tableSize; l2cam_idx++)
        {
            if ((_dal_ssw_l2_getL2EntryfromCAM(unit, l2cam_idx, &l2_entry, &isValid) == RT_ERR_OK)
                && (TRUE == isValid) && (type == l2_entry.entry_type))
            {
                if (L2_UNICAST == l2_entry.entry_type)
                {
                    if ((FALSE == include_static) && (1 == l2_entry.unicast.is_static))
                    {
                        /* if not need to include static and this entry is static, just skip this entry */
                        continue;
                    }
                }
                
                found = TRUE;
                *pScan_idx = l2cam_idx + l2_tableSize;
                break;
            }
        }
    }
    
    if (FALSE == found)
    {
        return RT_ERR_L2_ENTRY_NOTFOUND;
    }
    
    osal_memcpy(pL2_data, &l2_entry, sizeof(dal_ssw_l2_entry_t));
    return RT_ERR_OK;
} /* end of _dal_ssw_l2_nextValidAddr_get */


/* Function Name:
 *      _dal_ssw_l2_setL2HASHEntry
 * Description:
 *      Get exist entry or free entry
 * Input:
 *      unit       - unit id
 *      pL2_entry  - L2 entry used to do search
 *      pL2_index  - index of found entry(free or exist)
 *                      - index_type = IN_CAM mean this index is in CAM
 *                      - index_type = IN_HASH mean this index is in HASH
 *      get_method - use which method to get
 *                      - L2_GET_EXIST_ONLY
 *                      - L2_GET_EXIST_OR_FREE
 *                      - L2_GET_FREE_ONLY
 * Output:
 *      pL2_entry  - L2 entry 
 *      pL2_index  - the index of L2 entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
static int32
_dal_ssw_l2_setL2HASHEntry(uint32 unit, dal_ssw_l2_entry_t *pL2_entry, dal_ssw_l2_index_t *pL2_index)
{
    
    int32   ret;
    l2_entry_t l2_entry;
    uint32  mac_uint32[2];
    uint32  l2_index;
    uint32  ip_multi;
    
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, hashdepth=%d, index=%d", 
           unit, pL2_entry->entry_type, pL2_index->hashdepth, pL2_index->index);
   
    osal_memset(&l2_entry, 0, sizeof(l2_entry));
    
    /* Extract content of each kind of entry from l2_enry */
    switch (pL2_entry->entry_type)
    {
        case L2_UNICAST: /* L2 unicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "da_block=%d, sa_block=%d, port=%d, is_static=%d, aging=%d, auth=%d, Mac=%x-%x-%x-%x-%x-%x", 
                   pL2_entry->unicast.dablock, pL2_entry->unicast.sablock, pL2_entry->unicast.port, 
                   pL2_entry->unicast.is_static, pL2_entry->unicast.aging, pL2_entry->unicast.auth, pL2_entry->unicast.mac.octet[0],
                   pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], pL2_entry->unicast.mac.octet[3],
                   pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);
            
            ip_multi = 0;
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_IP_MULTIf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_DABLOCKf, &pL2_entry->unicast.dablock, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_SABLOCKf, &pL2_entry->unicast.sablock, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_SPAf, &pL2_entry->unicast.port, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_STATICf, &pL2_entry->unicast.is_static, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_AGEf, &pL2_entry->unicast.aging, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_AUTHf, &pL2_entry->unicast.auth, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            mac_uint32[1] = ((uint32)pL2_entry->unicast.mac.octet[0]) << 8;
            mac_uint32[1] |= ((uint32)pL2_entry->unicast.mac.octet[1]);
            mac_uint32[0] = ((uint32)pL2_entry->unicast.mac.octet[2]) << 24;
            mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[3]) << 16;
            mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[4]) << 8;
            mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[5]);
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_MACf, &mac_uint32[0], (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            break;
        case L2_MULTICAST: /* l2 Multicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, Mac=%x-%x-%x-%x-%x-%x", 
                   pL2_entry->l2mcast.index, pL2_entry->l2mcast.mac.octet[0],
                   pL2_entry->l2mcast.mac.octet[1], pL2_entry->l2mcast.mac.octet[2], pL2_entry->l2mcast.mac.octet[3],
                   pL2_entry->l2mcast.mac.octet[4], pL2_entry->l2mcast.mac.octet[5]);

            ip_multi = 0;
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_IP_MULTIf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_INDEXf, &pL2_entry->l2mcast.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            mac_uint32[1] = ((uint32)pL2_entry->l2mcast.mac.octet[0]) << 8;
            mac_uint32[1] |= ((uint32)pL2_entry->l2mcast.mac.octet[1]);
            mac_uint32[0] = ((uint32)pL2_entry->l2mcast.mac.octet[2]) << 24;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[3]) << 16;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[4]) << 8;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[5]);
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_MACf, &mac_uint32[0], (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            break;
        case IP_MULTICAST: /* IP multicast */
             RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, dip=%d, sip=%d", 
                   pL2_entry->ipmcast.index, pL2_entry->ipmcast.rvid, pL2_entry->ipmcast.dip, pL2_entry->ipmcast.sip);           
            
            if (pL2_entry->ipmcast.dip == 0)
            {/* if dip is zero, mean delete entry. So set ip_multi = 0 */
                ip_multi = 0;
            }
            else
            {
                ip_multi = 1;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_IP_MULTIf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_INDEXf, &pL2_entry->ipmcast.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_IP_CVIDf, &pL2_entry->ipmcast.rvid, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_DIPf, &pL2_entry->ipmcast.dip, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2_TABLEt, SSW_L2_TABLE_SIPf, &pL2_entry->ipmcast.sip, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            break;
        default:
            return RT_ERR_FAILED;
     }
     
    l2_index = (SRAM << ACCADDR_L2TYPE_OFFSET) | (pL2_index->hashdepth << 12) | pL2_index->index ;
    
     /* read entry from chip */
    if ((ret = table_write(unit, SSW_L2_TABLEt, l2_index, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_l2_setL2HASHEntry */


/* Function Name:
 *      _dal_ssw_l2_setL2CAMEntry
 * Description:
 *      Get exist entry or free entry
 * Input:
 *      unit       - unit id
 *      pL2_entry  - L2 entry used to do search
 *      pL2_index  - index of found entry(free or exist)
 *                      - index_type = IN_CAM mean this index is in CAM
 *                      - index_type = IN_HASH mean this index is in HASH
 *      get_method - use which method to get
 *                      - L2_GET_EXIST_ONLY
 *                      - L2_GET_EXIST_OR_FREE
 *                      - L2_GET_FREE_ONLY
 * Output:
 *      pL2_index  - the index of L2 entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
static int32
_dal_ssw_l2_setL2CAMEntry(uint32 unit, dal_ssw_l2_entry_t *pL2_entry, dal_ssw_l2_index_t *pL2_index)
{
    int32   ret;
    l2cam_entry_t l2cam_entry;
    uint32  mac_uint32[2];
    uint32  l2_index;
    uint32  ip_multi;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, hashdepth=%d, index=%d", 
           unit, pL2_entry->entry_type, pL2_index->hashdepth, pL2_index->index);
    
    l2_index = (CAM << ACCADDR_L2TYPE_OFFSET) | pL2_index->index ;
    
    osal_memset(&l2cam_entry, 0, sizeof(l2cam_entry));
    
    
    /* Extract content of each kind of entry from l2_enry */
    switch (pL2_entry->entry_type)
    {
        case L2_UNICAST: /* L2 unicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "da_block=%d, sa_block=%d, port=%d, fid=%d, is_static=%d, aging=%d, auth=%d, \
                   Mac=%x-%x-%x-%x-%x-%x", 
                   pL2_entry->unicast.dablock, pL2_entry->unicast.sablock, pL2_entry->unicast.port, pL2_entry->unicast.fid, 
                   pL2_entry->unicast.is_static, pL2_entry->unicast.aging, pL2_entry->unicast.auth, pL2_entry->unicast.mac.octet[0],
                   pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], pL2_entry->unicast.mac.octet[3],
                   pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);            
            ip_multi = 0;
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_IP_MULTIf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_DABLOCKf, &pL2_entry->unicast.dablock, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_SABLOCKf, &pL2_entry->unicast.sablock, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_SPAf, &pL2_entry->unicast.port, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_FIDf, &pL2_entry->unicast.fid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_STATICf, &pL2_entry->unicast.is_static, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_AGEf, &pL2_entry->unicast.aging, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_AUTHf, &pL2_entry->unicast.auth, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            mac_uint32[1] = ((uint32)pL2_entry->unicast.mac.octet[0]) << 8;
            mac_uint32[1] |= ((uint32)pL2_entry->unicast.mac.octet[1]);
            mac_uint32[0] = ((uint32)pL2_entry->unicast.mac.octet[2]) << 24;
            mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[3]) << 16;
            mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[4]) << 8;
            mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[5]);
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_MACf, &mac_uint32[0], (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            break;
        case L2_MULTICAST: /* l2 Multicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, Mac=%x-%x-%x-%x-%x-%x", 
                   pL2_entry->l2mcast.index, pL2_entry->l2mcast.rvid, pL2_entry->l2mcast.mac.octet[0],
                   pL2_entry->l2mcast.mac.octet[1], pL2_entry->l2mcast.mac.octet[2], pL2_entry->l2mcast.mac.octet[3],
                   pL2_entry->l2mcast.mac.octet[4], pL2_entry->l2mcast.mac.octet[5]);
            ip_multi = 0;
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_IP_MULTIf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_INDEXf, &pL2_entry->l2mcast.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_M_CVIDf, &pL2_entry->l2mcast.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            mac_uint32[1] = ((uint32)pL2_entry->l2mcast.mac.octet[0]) << 8;
            mac_uint32[1] |= ((uint32)pL2_entry->l2mcast.mac.octet[1]);
            mac_uint32[0] = ((uint32)pL2_entry->l2mcast.mac.octet[2]) << 24;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[3]) << 16;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[4]) << 8;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[5]);
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_MACf, &mac_uint32[0], (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            break;
        case IP_MULTICAST: /* IP multicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, dip=%d, sip=%d", 
                   pL2_entry->ipmcast.index, pL2_entry->ipmcast.rvid, pL2_entry->ipmcast.dip, pL2_entry->ipmcast.sip);
            
            if (pL2_entry->ipmcast.dip == 0)
            {/* if dip is zero, mean delete entry. So set ip_multi = 0 */
                ip_multi = 0;
            } 
            else
            {
                ip_multi = 1;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_IP_MULTIf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_INDEXf, &pL2_entry->ipmcast.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_IP_CVIDf, &pL2_entry->ipmcast.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_DIPf, &pL2_entry->ipmcast.dip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_SIPf, &pL2_entry->ipmcast.sip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            break;
        default:
            return RT_ERR_FAILED;
     }
     
     /* read entry from chip */
    if ((ret = table_write(unit, SSW_L2CAM_TABLEt, l2_index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_l2_setL2CAMEntry */


/* Function Name:
 *      _dal_ssw_l2_getExistOrFreeL2Entry
 * Description:
 *      Get exist entry or free entry
 * Input:
 *      unit       - unit id
 *      pL2_entry  - L2 entry used to do search
 *      pL2_index  - index of found entry(free or exist)
 *                      - index_type = IN_CAM mean this index is in CAM
 *                      - index_type = IN_HASH mean this index is in HASH
 *      get_method - use which method to get
 *                      - L2_GET_EXIST_ONLY
 *                      - L2_GET_EXIST_OR_FREE
 *                      - L2_GET_FREE_ONLY
 * Output:
 *      pL2_entry  - L2 entry
 *      pL2_index  - the index of L2 entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_VLAN_VID - invalid vid
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 *      RT_ERR_L2_NO_EMPTY_ENTRY - no empty entry in L2 table
 * Note:
 *      None
 */
static int32
_dal_ssw_l2_getExistOrFreeL2Entry(uint32 unit, dal_ssw_l2_entry_t *pL2_entry, dal_ssw_l2_getMethod_t get_method
        , dal_ssw_l2_index_t *pL2_index)
{
    int32   ret;
    dal_ssw_l2_entry_t  l2_entry;
    uint32  hashkey;
    uint32  hash_depth;
    uint32  cam_index;
    uint32  l2cam_tableSize, l2_tableSize;
    uint32  isValid;
    uint32  found_exist;
    uint32  found_free;
    
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, Mac=%x-%x-%x-%x-%x-%x\
           get_method=%d, ", 
           unit, pL2_entry->entry_type, pL2_entry->unicast.mac.octet[0], pL2_entry->unicast.mac.octet[1], 
           pL2_entry->unicast.mac.octet[2], pL2_entry->unicast.mac.octet[3], pL2_entry->unicast.mac.octet[4],
           pL2_entry->unicast.mac.octet[5], get_method);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pL2_entry) || (NULL == pL2_index), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((get_method >= DAL_SSW_GETMETHOD_END), RT_ERR_INPUT);
    
    osal_memset(&l2_entry, 0, sizeof(&l2_entry));
    
    pL2_entry->is_entry_exist = FALSE;
    
    /* calculate hash key */
    if ((ret = _dal_ssw_l2_entryToHashKey(unit, pL2_entry, &hashkey)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    found_exist = FALSE;
    found_free = FALSE;
    
    /* Search L2 unicast entry in hash table */
    for (hash_depth = 0; hash_depth < HAL_L2_HASHDEPTH(unit); hash_depth++)
    {
        if (_dal_ssw_l2_getL2EntryfromHash(unit, hashkey, hash_depth, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }
        
        if ((TRUE == isValid ) 
            && (get_method == L2_GET_EXIST_ONLY || get_method == L2_GET_EXIST_OR_FREE)
            && (l2_entry.entry_type == pL2_entry->entry_type))
        {
            if (_dal_ssw_l2_compareEntry(&l2_entry, pL2_entry) == RT_ERR_OK)
            {
                found_exist = TRUE;
                pL2_index->index_type = L2_IN_HASH;
                pL2_index->index = hashkey;
                pL2_index->hashdepth = hash_depth;
                osal_memcpy(pL2_entry, &l2_entry, sizeof(l2_entry));
                pL2_entry->is_entry_exist = TRUE;
                return RT_ERR_OK;
            }
        } 
        else if (FALSE == isValid)
        {
            if (L2_GET_FREE_ONLY == get_method )
            {
                found_free = TRUE;
                pL2_index->index_type = L2_IN_HASH;
                pL2_index->index = hashkey;
                pL2_index->hashdepth = hash_depth;
                return RT_ERR_OK;
            } 
            else if ((L2_GET_EXIST_OR_FREE == get_method) && FALSE == found_free)
            {
                found_free = TRUE;
                pL2_index->index_type = L2_IN_HASH;
                pL2_index->index = hashkey;
                pL2_index->hashdepth = hash_depth;
            }
        } 
    }
    
    if ((ret = table_size_get(unit, SSW_L2CAM_TABLEt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if (( ret = table_size_get(unit, SSW_L2_TABLEt, &l2_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    l2cam_tableSize = l2cam_tableSize - l2_tableSize;
    
    for (cam_index = 0; cam_index < l2cam_tableSize; cam_index++)
    {
        if (_dal_ssw_l2_getL2EntryfromCAM(unit, cam_index, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }
        
        if ((TRUE == isValid ) 
            && (get_method == L2_GET_EXIST_ONLY || get_method == L2_GET_EXIST_OR_FREE)
            && (l2_entry.entry_type == pL2_entry->entry_type))
        {
            if (_dal_ssw_l2_compareEntry(&l2_entry, pL2_entry) == RT_ERR_OK)
            {
                found_exist = TRUE;
                pL2_index->index_type = L2_IN_CAM;
                pL2_index->index = cam_index;
                osal_memcpy(pL2_entry, &l2_entry, sizeof(l2_entry));
                pL2_entry->is_entry_exist = TRUE;
                return RT_ERR_OK;
            }
        } 
        else if (FALSE == isValid)
        {
            if (L2_GET_FREE_ONLY == get_method )
            {
                found_free = TRUE;
                pL2_index->index_type = L2_IN_CAM;
                pL2_index->index = cam_index;
                return RT_ERR_OK;
            } 
            else if ((L2_GET_EXIST_OR_FREE == get_method) && FALSE == found_free)
            {
                found_free = TRUE;
                pL2_index->index_type = L2_IN_CAM;
                pL2_index->index = cam_index;
            }
        } 
    }

    if ((L2_GET_EXIST_OR_FREE == get_method) && (TRUE == found_free))
    {
        return RT_ERR_OK;
    }

    if (L2_GET_EXIST_ONLY == get_method)
    {
        return RT_ERR_L2_ENTRY_NOTFOUND;
    }
    else /* if (L2_GET_EXIST_OR_FREE == get_method || L2_GET_FREE_ONLY == get_method) */
    {
        return RT_ERR_L2_NO_EMPTY_ENTRY;
    }
} /* end of _dal_ssw_l2_getExistOrFreeL2Entry */

/* Function Name:
 *      _dal_ssw_l2_getFirstDynamicEntry
 * Description:
 *      Get first dynamic entry
 * Input:
 *      unit       - unit id
 *      pL2_entry  - L2 entry used to do search
 *      pL2_index  - index of found entry(free or exist)
 *                      - index_type = IN_CAM mean this index is in CAM
 *                      - index_type = IN_HASH mean this index is in HASH
 * Output:
 *      pL2_entry  - L2 entry
 *      pL2_index  - the index of L2 entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_VLAN_VID - invalid vid
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
static int32
_dal_ssw_l2_getFirstDynamicEntry(uint32 unit, dal_ssw_l2_entry_t *pL2_entry
        , dal_ssw_l2_index_t *pL2_index)
{
    int32   ret;
    dal_ssw_l2_entry_t  l2_entry;
    uint32  hashkey;
    uint32  hash_depth;
    uint32  cam_index;
    uint32  l2cam_tableSize, l2_tableSize;
    uint32  isValid;
    uint32  found_exist;
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pL2_entry) || (NULL == pL2_index), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, Mac=%x-%x-%x-%x-%x-%x", 
           unit, pL2_entry->entry_type, pL2_entry->unicast.mac.octet[0], pL2_entry->unicast.mac.octet[1], 
           pL2_entry->unicast.mac.octet[2], pL2_entry->unicast.mac.octet[3], pL2_entry->unicast.mac.octet[4],
           pL2_entry->unicast.mac.octet[5]);
    
    osal_memset(&l2_entry, 0, sizeof(&l2_entry));

    pL2_entry->is_entry_exist = FALSE;
    
    /* calculate hash key */
    if ((ret = _dal_ssw_l2_entryToHashKey(unit, pL2_entry, &hashkey)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    found_exist = FALSE;
    
    /* Search L2 unicast entry in hash table */
    for (hash_depth = 0; hash_depth < HAL_L2_HASHDEPTH(unit); hash_depth++)
    {
        if (_dal_ssw_l2_getL2EntryfromHash(unit, hashkey, hash_depth, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }

        if ((TRUE == isValid ) && (l2_entry.entry_type == L2_UNICAST))
        {
            if (l2_entry.unicast.sablock == 0 && l2_entry.unicast.dablock == 0 &&
                l2_entry.unicast.auth == 0 && l2_entry.unicast.is_static == 0)
            {
                found_exist = TRUE;
                pL2_index->index_type = L2_IN_HASH;
                pL2_index->index = hashkey;
                pL2_index->hashdepth = hash_depth;
                osal_memcpy(pL2_entry, &l2_entry, sizeof(l2_entry));
                pL2_entry->is_entry_exist = TRUE;
                return RT_ERR_OK;
            }
        }
    }

    if ((ret = table_size_get(unit, SSW_L2CAM_TABLEt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if (( ret = table_size_get(unit, SSW_L2_TABLEt, &l2_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    for (cam_index = 0; cam_index < l2cam_tableSize; cam_index++)
    {
        if (_dal_ssw_l2_getL2EntryfromCAM(unit, cam_index, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }

        if ((TRUE == isValid ) && (l2_entry.entry_type == L2_UNICAST))
        {
            if (l2_entry.unicast.sablock == 0 && l2_entry.unicast.dablock == 0 &&
                l2_entry.unicast.auth == 0 && l2_entry.unicast.is_static == 0)
            {
                found_exist = TRUE;
                pL2_index->index_type = L2_IN_CAM;
                pL2_index->index = cam_index;
                osal_memcpy(pL2_entry, &l2_entry, sizeof(l2_entry));
                pL2_entry->is_entry_exist = TRUE;
                return RT_ERR_OK;
            }
        } 
    }

    if (TRUE == found_exist)
        return RT_ERR_OK;
    else
        return RT_ERR_L2_ENTRY_NOTFOUND;
} /* end of _dal_ssw_l2_getFirstDynamicEntry */

/* Function Name:
 *      _dal_ssw_l2_getL2EntryfromCAM
 * Description:
 *      Get L2 Entry from CAM
 * Input:
 *      unit      - unit id
 *      index     - CAM index for this Entry
 *      pL2_entry - L2 entry used to do search
 *      pIsValid  - Is valid entry
 *
 * Output:
 *      pL2_entry - L2 entry 
 *      pIsValid  - Is valid or invalid entry
 *                    TRUE: valid entry
 *                    FALSE: invalid entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
static int32
_dal_ssw_l2_getL2EntryfromCAM(uint32 unit, uint32 index, 
                              dal_ssw_l2_entry_t *pL2_entry, uint32 *pIsValid)
{
    int32   ret;
    l2cam_entry_t  l2cam_entry;
    uint32  l2_index;
    uint32  ip_multi;
    uint32  mac_uint32[2];
    uint32  is_l2mcast;
    uint32  aging;
    uint32  is_static;
    uint32  auth;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, index=%d, entry_type=%d, \
           Mac=%x-%x-%x-%x-%x-%x", unit, index, pL2_entry->entry_type, pL2_entry->unicast.mac.octet[0], 
           pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], pL2_entry->unicast.mac.octet[3], 
           pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);
    
    /* calculate l2 index in hash table */
    l2_index = (CAM << ACCADDR_L2TYPE_OFFSET) | index ;
    
    /* read entry from chip */
    if ((ret = table_read(unit, SSW_L2CAM_TABLEt, l2_index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    /* get IP_MULTI from l2_entry */
    if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_IP_MULTIf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    is_l2mcast = 0;
    /* check whether this entry is L2 unicast or multicast */
    if (0 == ip_multi)
    {
        if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_MACf, &mac_uint32[0], (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        is_l2mcast = (uint8) ( (mac_uint32[1] >> 8) & BITMASK_1B);
    }
    
    aging = 0;
    is_static = 0;
    auth    = 0;
    /* check whether this entry is l2 multicast entry. 
       If not l2 multicast entry, get more information for valid check 
     */
    if (0 == is_l2mcast && 0 == ip_multi)
    {
        if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_AGEf, &aging, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
        
        if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_STATICf, &is_static, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
        
        if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_AUTHf, &auth, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
    }
    
    /* check whether this entry is valid entry */
    if (!(ip_multi | is_l2mcast | aging | is_static | auth))
    {
        /* this is not a valid entry. No need to futher process*/
        *pIsValid = FALSE;
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pIsValid=%d", *pIsValid);
        return RT_ERR_OK;
    } 
    else 
    {
        *pIsValid = TRUE;
    }
    
    /* Extract content of each kind of entry from l2_enry */
    switch ((ip_multi << 1) | is_l2mcast)
    {
        case 0x00: /* L2 unicast */
            pL2_entry->entry_type = L2_UNICAST;         
            if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_DABLOCKf, &pL2_entry->unicast.dablock, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_SABLOCKf, &pL2_entry->unicast.sablock, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_SPAf, &pL2_entry->unicast.port, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_FIDf, &pL2_entry->unicast.fid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            pL2_entry->unicast.mac.octet[0] = (uint8)(mac_uint32[1] >> 8); 
            pL2_entry->unicast.mac.octet[1] = (uint8)(mac_uint32[1]);
            pL2_entry->unicast.mac.octet[2] = (uint8)(mac_uint32[0] >> 24); 
            pL2_entry->unicast.mac.octet[3] = (uint8)(mac_uint32[0] >> 16); 
            pL2_entry->unicast.mac.octet[4] = (uint8)(mac_uint32[0] >> 8); 
            pL2_entry->unicast.mac.octet[5] = (uint8)(mac_uint32[0]);
            
            pL2_entry->unicast.is_static  = is_static;
            pL2_entry->unicast.aging      = aging;
            pL2_entry->unicast.auth       = auth;
            
            break;
        case 0x01: /* l2 Multicast */
            pL2_entry->entry_type = L2_MULTICAST;
            if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_INDEXf, &pL2_entry->l2mcast.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_M_CVIDf, &pL2_entry->l2mcast.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            pL2_entry->l2mcast.mac.octet[0] = (uint8)(mac_uint32[1] >> 8); 
            pL2_entry->l2mcast.mac.octet[1] = (uint8)(mac_uint32[1]);
            pL2_entry->l2mcast.mac.octet[2] = (uint8)(mac_uint32[0] >> 24); 
            pL2_entry->l2mcast.mac.octet[3] = (uint8)(mac_uint32[0] >> 16); 
            pL2_entry->l2mcast.mac.octet[4] = (uint8)(mac_uint32[0] >> 8); 
            pL2_entry->l2mcast.mac.octet[5] = (uint8)(mac_uint32[0]);
            
            break;
        case 0x2: /* IP multicast */
            pL2_entry->entry_type = IP_MULTICAST;
            if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_INDEXf, &pL2_entry->ipmcast.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_IP_CVIDf, &pL2_entry->ipmcast.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_DIPf, &pL2_entry->ipmcast.dip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2CAM_TABLEt, SSW_L2CAM_TABLE_SIPf, &pL2_entry->ipmcast.sip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            /* add 0xE in first four bit in DIP */
            pL2_entry->ipmcast.dip = (0xE << 28) | pL2_entry->ipmcast.dip;
            
            break;
        default:
            return RT_ERR_FAILED;
     }
     
     RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pIsValid=%d", *pIsValid);
     
     return RT_ERR_OK;
} /* end of _dal_ssw_l2_getL2EntryfromCAM */

/* Function Name:
 *      _dal_ssw_l2_getL2EntryfromHash
 * Description:
 *      Get L2 Entry from Chip
 * Input:
 *      unit      - unit id
 *      hashKey   - Hash Key for this Entry
 *      location  - Entry location in Hash Bucket
 *      pL2_entry - L2 entry used to do search
 *      pIsValid  - Is valid entry
 *
 * Output:
 *      pL2_entry - L2 entry 
 *      pIsValid  - Is valid or invalid entry
 *                    TRUE: valid entry
 *                    FALSE: invalid entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
static int32
_dal_ssw_l2_getL2EntryfromHash(uint32 unit, uint32 hashkey, uint32 location, 
                              dal_ssw_l2_entry_t *pL2_entry, uint32 *pIsValid)
{
    int32   ret;
    l2_entry_t  l2_entry;
    uint32  l2_index;
    uint32  ip_multi;
    uint32  mac_uint32[2];
    uint32  is_l2mcast;
    uint32  aging;
    uint32  is_static;
    uint32  auth;
    uint32  partialkey1;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, hashkey=%d, location=%d, \
           entry_type=%d, Mac=%x-%x-%x-%x-%x-%x", unit, hashkey, location, pL2_entry->entry_type, 
           pL2_entry->unicast.mac.octet[0], pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], 
           pL2_entry->unicast.mac.octet[3], pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);
    
    /* calculate l2 index in hash table */
    l2_index = (SRAM << ACCADDR_L2TYPE_OFFSET) | (location << 12) | hashkey;
    
    /* read entry from chip */
    if ((ret = table_read(unit, SSW_L2_TABLEt, l2_index, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    /* get IP_MULTI from l2_entry */
    if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_IP_MULTIf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    is_l2mcast = 0;
    /* check whether this entry is L2 unicast or multicast */
    if (0 == ip_multi)
    {
        if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_MACf, &mac_uint32[0], (uint32 *)&l2_entry)) != RT_ERR_OK) 
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
        is_l2mcast = (uint8) ( (mac_uint32[1] >> 8) & BITMASK_1B);
    }
    
    aging = 0;
    is_static = 0;
    auth    = 0;
    /* check whether this entry is l2 multicast entry. 
       If not l2 multicast entry, get more information for valid check 
     */
    if (0 == is_l2mcast && 0 == ip_multi)
    {
        if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_AGEf, &aging, (uint32 *)&l2_entry)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
        
        if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_STATICf, &is_static, (uint32 *)&l2_entry)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
        
        if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_AUTHf, &auth, (uint32 *)&l2_entry)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
    }
    
    /* check whether this entry is valid entry */
    if (!(ip_multi | is_l2mcast | aging | is_static | auth))
    {
        /* this is not a valid entry. No need to futher process*/
        *pIsValid = FALSE;
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pIsValid=%d", *pIsValid);
        return RT_ERR_OK;
    } 
    else 
    {
        *pIsValid = TRUE;
    }
    
    /* Extract content of each kind of entry from l2_enry */
    switch ((ip_multi << 1) | is_l2mcast)
    {
        case 0x0: /* L2 unicast */
            pL2_entry->entry_type = L2_UNICAST;
                    
            if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_DABLOCKf, &pL2_entry->unicast.dablock, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_SABLOCKf, &pL2_entry->unicast.sablock, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_SPAf, &pL2_entry->unicast.port, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            pL2_entry->unicast.mac.octet[0] = (uint8)(mac_uint32[1] >> 8); 
            pL2_entry->unicast.mac.octet[1] = (uint8)(mac_uint32[1]);
            pL2_entry->unicast.mac.octet[2] = (uint8)(mac_uint32[0] >> 24); 
            pL2_entry->unicast.mac.octet[3] = (uint8)(mac_uint32[0] >> 16); 
            pL2_entry->unicast.mac.octet[4] = (uint8)(mac_uint32[0] >> 8); 
            pL2_entry->unicast.mac.octet[5] = (uint8)(mac_uint32[0]);
            
            pL2_entry->unicast.is_static  = is_static;
            pL2_entry->unicast.aging      = aging;
            pL2_entry->unicast.auth       = auth;
            
            /* convert FID from hash algorithm */
            pL2_entry->unicast.fid  = hashkey;
            if ((ret = _dal_ssw_l2_entryToHashKey(unit, pL2_entry, &partialkey1)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            
            pL2_entry->unicast.fid  = partialkey1;          
            break;
        case 0x1: /* l2 Multicast */            
            pL2_entry->entry_type = L2_MULTICAST;
            
            if (table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_INDEXf, &pL2_entry->l2mcast.index, (uint32 *)&l2_entry) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return RT_ERR_FAILED;
            }
            
            pL2_entry->l2mcast.mac.octet[0] = (uint8)(mac_uint32[1] >> 8); 
            pL2_entry->l2mcast.mac.octet[1] = (uint8)(mac_uint32[1]);
            pL2_entry->l2mcast.mac.octet[2] = (uint8)(mac_uint32[0] >> 24); 
            pL2_entry->l2mcast.mac.octet[3] = (uint8)(mac_uint32[0] >> 16); 
            pL2_entry->l2mcast.mac.octet[4] = (uint8)(mac_uint32[0] >> 8); 
            pL2_entry->l2mcast.mac.octet[5] = (uint8)(mac_uint32[0]);
            
            /* convert RVID from hash algorithm */
                          
            pL2_entry->l2mcast.rvid  = hashkey;
            if (( ret = _dal_ssw_l2_entryToHashKey(unit, pL2_entry, &partialkey1)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            pL2_entry->l2mcast.rvid  = partialkey1;           
            break;
        case 0x2: /* IP multicast */           
            pL2_entry->entry_type = IP_MULTICAST;
            
            if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_INDEXf, &pL2_entry->ipmcast.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_IP_CVIDf, &pL2_entry->ipmcast.rvid, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_DIPf, &pL2_entry->ipmcast.dip, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_get(unit, SSW_L2_TABLEt, SSW_L2_TABLE_SIPf, &pL2_entry->ipmcast.sip, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            
            pL2_entry->ipmcast.sip = pL2_entry->ipmcast.sip | (hashkey << 20);
            
            if (( ret = _dal_ssw_l2_entryToHashKey(unit, pL2_entry, &partialkey1)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            /* add 0xE in first four bit in DIP */
            pL2_entry->ipmcast.dip = (0xE << 28) | pL2_entry->ipmcast.dip;
            pL2_entry->ipmcast.sip = (pL2_entry->ipmcast.sip & BITMASK_20B) | (partialkey1 << 20);                        
            break;
        default:
            return RT_ERR_FAILED;
     }
     
     RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pIsValid=%d", *pIsValid);
     
     return RT_ERR_OK;
} /* end of _dal_ssw_l2_getL2EntryfromHash */

/* Function Name:
 *      _dal_ssw_l2_entryToHashKey
 * Description:
 *      Translate L2 entry to seed of Hash
 * Input:
 *      unit      - unit id
 *      pL2_entry - L2 entry used to generate seed
 *      pKey      - key for Hash
 * Output:
 *      pKey      - key for hash
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32
_dal_ssw_l2_entryToHashKey(uint32 unit, dal_ssw_l2_entry_t *pL2_entry, uint32 *pKey)
{
    uint64  hashSeed;
    uint32  hash11_6, hash5_0;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, \
           Mac=%x-%x-%x-%x-%x-%x", unit, pL2_entry->entry_type, 
           pL2_entry->unicast.mac.octet[0], pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], 
           pL2_entry->unicast.mac.octet[3], pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);

    /* get hash seed from l2 entry */
    switch (pL2_entry->entry_type)
    {
        case L2_UNICAST:
            /* if it is unicast, key will be fid+mac */
            hashSeed = ((uint64)pL2_entry->unicast.fid << 48) |
                    ((uint64)pL2_entry->unicast.mac.octet[0] << 40) |
                    ((uint64)pL2_entry->unicast.mac.octet[1] << 32) |
                    ((uint64)pL2_entry->unicast.mac.octet[2] << 24) |
                    ((uint64)pL2_entry->unicast.mac.octet[3] << 16) |
                    ((uint64)pL2_entry->unicast.mac.octet[4] << 8) |
                    ((uint64)pL2_entry->unicast.mac.octet[5]);
            break;
        case L2_MULTICAST:
            /* if it is l2 multicast, key will be rvid+mac */
            hashSeed = ((uint64)pL2_entry->l2mcast.rvid << 48) |
                    ((uint64)pL2_entry->l2mcast.mac.octet[0] << 40) |
                    ((uint64)pL2_entry->l2mcast.mac.octet[1] << 32) |
                    ((uint64)pL2_entry->l2mcast.mac.octet[2] << 24) |
                    ((uint64)pL2_entry->l2mcast.mac.octet[3] << 16) |
                    ((uint64)pL2_entry->l2mcast.mac.octet[4] << 8) |
                    ((uint64)pL2_entry->l2mcast.mac.octet[5]);
            break;
        case IP_MULTICAST:
            /* if it is ip multicast, key will be sip+dip(first 28bits) */
            hashSeed = ((uint64)pL2_entry->ipmcast.sip << 28) |
                    ((uint64)pL2_entry->ipmcast.dip & BITMASK_28B);
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    if ((hashSeed & 0xf000000000000000ULL) != 0)
    {
        /* hash seed should only have 60 bits, if there have value in highest nibble, return fail */
        return RT_ERR_L2_HASH_KEY;
    }
    
    /* TBD */
    if (0 == algoType[unit])
    {
        *pKey = (uint32) (((hashSeed >> 48) & BITMASK_12B) 
                        ^ ((hashSeed >> 36) & BITMASK_12B) 
                        ^ ((hashSeed >> 24) & BITMASK_12B) 
                        ^ ((hashSeed >> 12) & BITMASK_12B) 
                        ^ (hashSeed  & BITMASK_12B));
    } 
    else if (1 == algoType[unit])
    {
        hash11_6 = (uint32) (((hashSeed >> 54) & BITMASK_6B) 
                        ^ ((hashSeed >> 36) & BITMASK_6B) 
                        ^ ((hashSeed >> 30) & BITMASK_6B) 
                        ^ ((hashSeed >> 12) & BITMASK_6B) 
                        ^ ((hashSeed >> 6)  & BITMASK_6B));
        hash5_0 = (uint32) (((hashSeed >> 48) & BITMASK_6B) 
                        ^ ((hashSeed >> 42) & BITMASK_6B) 
                        ^ ((hashSeed >> 24) & BITMASK_6B) 
                        ^ ((hashSeed >> 18) & BITMASK_6B) 
                        ^ ((hashSeed)  & BITMASK_6B));
        *pKey = (hash11_6 << 6) | hash5_0;
    } 
    else 
    {
        return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pKey=%d", *pKey);
    return RT_ERR_OK;
} /* end of _dal_ssw_l2_entryToHashKey */

/* Function Name:
 *      _dal_ssw_l2_compareEntry
 * Description:
 *      Compare L2 entry
 * Input:
 *      pSrcEntry - source entry
 *      pDstEntry - Destination entry
 * Output:
 *
 * Return:
 *      RT_ERR_OK     - key of two entry is same
 *      RT_ERR_FAILED - key of two entry is different
 * Note:
 *      None
 */
static int32
_dal_ssw_l2_compareEntry(dal_ssw_l2_entry_t *pSrcEntry, dal_ssw_l2_entry_t *pDstEntry)
{
    uint32  ipmcast_rvid_check;
    int32   ret;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "entry_type=%d", pSrcEntry->entry_type);
    
    switch (pSrcEntry->entry_type)
    {
        case L2_UNICAST:
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "srcfid=%d, dstfid=%d, srcMac=%x-%x-%x-%x-%x-%x,\
                   dstMac=%x-%x-%x-%x-%x-%x", pSrcEntry->unicast.fid, pDstEntry->unicast.fid,
                   pSrcEntry->unicast.mac.octet[0], pSrcEntry->unicast.mac.octet[1], pSrcEntry->unicast.mac.octet[2], 
                   pSrcEntry->unicast.mac.octet[3], pSrcEntry->unicast.mac.octet[4], pSrcEntry->unicast.mac.octet[5],
                   pDstEntry->unicast.mac.octet[0], pDstEntry->unicast.mac.octet[1], pDstEntry->unicast.mac.octet[2], 
                   pDstEntry->unicast.mac.octet[3], pDstEntry->unicast.mac.octet[4], pDstEntry->unicast.mac.octet[5]);
            
            if ((osal_memcmp(&pSrcEntry->unicast.mac, &pDstEntry->unicast.mac, sizeof(rtk_mac_t)))
               || (pSrcEntry->unicast.fid != pDstEntry->unicast.fid))
            {
                return RT_ERR_FAILED;
            }
            
            return RT_ERR_OK;
            break;
        case L2_MULTICAST:
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "srcrvid=%d, dstrvid=%d, srcMac=%x-%x-%x-%x-%x-%x,\
                   dstMac=%x-%x-%x-%x-%x-%x", pSrcEntry->l2mcast.rvid, pDstEntry->l2mcast.rvid,
                   pSrcEntry->l2mcast.mac.octet[0], pSrcEntry->l2mcast.mac.octet[1], pSrcEntry->l2mcast.mac.octet[2], 
                   pSrcEntry->l2mcast.mac.octet[3], pSrcEntry->l2mcast.mac.octet[4], pSrcEntry->l2mcast.mac.octet[5],
                   pDstEntry->l2mcast.mac.octet[0], pDstEntry->l2mcast.mac.octet[1], pDstEntry->l2mcast.mac.octet[2], 
                   pDstEntry->l2mcast.mac.octet[3], pDstEntry->l2mcast.mac.octet[4], pDstEntry->l2mcast.mac.octet[5]);
            
            if ((osal_memcmp(&pSrcEntry->l2mcast.mac, &pDstEntry->l2mcast.mac, sizeof(rtk_mac_t)))
               || (pSrcEntry->l2mcast.rvid != pDstEntry->l2mcast.rvid))
            {
                return RT_ERR_FAILED;
            }
            
            return RT_ERR_OK;
            break;
        case IP_MULTICAST:
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "srcdip=%d, dstdip=%d, srcsip=%d, dstsip=%d", 
                   pSrcEntry->ipmcast.dip, pDstEntry->ipmcast.dip, pSrcEntry->ipmcast.sip, pDstEntry->ipmcast.sip);
            if ((pSrcEntry->ipmcast.dip != pDstEntry->ipmcast.dip)
                ||(pSrcEntry->ipmcast.sip != pDstEntry->ipmcast.sip))
            {
                return RT_ERR_FAILED;
            }

            if ((ret = reg_field_read(0, SSW_L2_TABLE_CONTROLr, SSW_IPMUL_CMPRVIDf, &ipmcast_rvid_check)) != RT_ERR_OK)
            {
                return ret;
            }

            if ((ipmcast_rvid_check == 1) && (pSrcEntry->ipmcast.rvid != pDstEntry->ipmcast.rvid))
            {
                return RT_ERR_FAILED;
            }
            
            return RT_ERR_OK;
            break;
        default:           
            return RT_ERR_FAILED;
    }
    
} /* end of _dal_ssw_l2_compareEntry*/

/* Function Name:
 *      _dal_ssw_l2_allocMcastIdx
 * Description:
 *      get a free mcast index
 * Input:
 *      unit      - unit id
 *      pMcastIdx - buffer to store free idx
 * Output:
 *
 * Return:
 *      RT_ERR_OK               - key of two entry is same
 *      RT_ERR_FAILED           - key of two entry is different
 *      RT_ERR_L2_INDEXTBL_FULL - L2 index table is full
 * Note:
 *      None
 */
static int32 _dal_ssw_l2_allocMcastIdx(uint32 unit, int32 *pMcastIdx)
{
    int16   free_idx;  
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    if (*pMcastIdx >= 0)
    {
        mcast_idx_pool[unit].pMcast_index_pool[*pMcastIdx].ref_count++;
        return RT_ERR_OK;
    }
    
    while(mcast_idx_pool[unit].free_index_head != END_OF_MCAST_IDX)
    {
        free_idx = mcast_idx_pool[unit].free_index_head;
        mcast_idx_pool[unit].free_index_head = mcast_idx_pool[unit].pMcast_index_pool[free_idx].next_index;
        mcast_idx_pool[unit].pMcast_index_pool[free_idx].next_index = MCAST_IDX_ALLOCATED;
    
        if (0 == mcast_idx_pool[unit].pMcast_index_pool[free_idx].ref_count)
        {
            *pMcastIdx = free_idx;
            mcast_idx_pool[unit].pMcast_index_pool[free_idx].ref_count++;
            mcast_idx_pool[unit].free_entry_count--;
            return RT_ERR_OK;
        }
    }
    
    return RT_ERR_L2_INDEXTBL_FULL;
} /* _dal_ssw_l2_allocMcastIdx */

/* Function Name:
 *      _dal_ssw_l2_freeMcastIdx
 * Description:
 *      Free a mcast index
 * Input:
 *      unit     - unit id
 *      mcastIdx - multicast index to free
 * Output:
 *
 * Return:
 *      RT_ERR_OK     - key of two entry is same
 *      RT_ERR_FAILED - key of two entry is different
 * Note:
 *      None
 */
static int32 _dal_ssw_l2_freeMcastIdx(uint32 unit, int32 mcastIdx)
{
    int32   ret;
    multicast_index_entry_t mcast_entry;
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, mcastIdx=%d", unit, mcastIdx);
        
    mcast_idx_pool[unit].pMcast_index_pool[mcastIdx].ref_count--;
    
    if (0 == mcast_idx_pool[unit].pMcast_index_pool[mcastIdx].ref_count)
    {
        if ( MCAST_IDX_ALLOCATED == mcast_idx_pool[unit].pMcast_index_pool[mcastIdx].next_index)
        {
            mcast_idx_pool[unit].pMcast_index_pool[mcastIdx].next_index = mcast_idx_pool[unit].free_index_head;
            mcast_idx_pool[unit].free_index_head = mcastIdx;
            mcast_idx_pool[unit].free_entry_count++;
        }
        
        /* when multicast entry is freed, reset the portmask to zero */
        osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
        if ((ret = table_write(unit, SSW_MULTICAST_INDEX_TABLEt, mcastIdx, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
            return ret;
        }
    }
    
    return RT_ERR_OK;
} /* _dal_ssw_l2_freeMcastIdx */

/* Function Name:
 *      _dal_ssw_l2_isMcastIdxUsed
 * Description:
 *      Free a mcast index
 * Input:
 *      unit     - unit id
 *      mcastIdx - multicast index for checking
 * Output:
 *
 * Return:
 *      RT_ERR_OK     - index is used
 *      RT_ERR_FAILED - index is not used
 * Note:
 *      None
 */
static int32 _dal_ssw_l2_isMcastIdxUsed(uint32 unit, int32 mcastIdx)
{
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, mcastIdx=%d", unit, mcastIdx);
        
    if (0 != mcast_idx_pool[unit].pMcast_index_pool[mcastIdx].ref_count)
    {
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_FAILED;
    }
    
} /* _dal_ssw_l2_isMcastIdxUsed */

/* Function Name:
 *      dal_ssw_l2_lookupMissFloodPortMask_get
 * Description:
 *      Get flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      unit            - unit id     
 *      type        - type of lookup miss
 * Output:
 *      pFlood_portmask - flooding port mask configuration when unicast/multicast lookup missed.
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
dal_ssw_l2_lookupMissFloodPortMask_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask)
{
    int32   ret;
    uint32  reg_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
           
    /* parameter check */
    RT_PARAM_CHK(NULL == pFlood_portmask, RT_ERR_NULL_POINTER);

    reg_idx = SSW_LOOKUP_MISS_FLOODING_PORTMASKr;

    L2_SEM_LOCK(unit); 
    if ((ret = reg_field_read(unit, reg_idx, SSW_FLOODING_PORTMASKf, &(pFlood_portmask->bits[0]))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    L2_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "flood_portmask=0x%x", 
           pFlood_portmask->bits[0]);
           
    return ret;
} /* end of dal_ssw_l2_lookupMissFloodPortMask_get */

/* Function Name:
 *      dal_ssw_l2_lookupMissFloodPortMask_set
 * Description:
 *      Set flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      unit            - unit id  
 *      type        - type of lookup miss
 *      pFlood_portmask - flooding port mask configuration when unicast/multicast lookup missed.   
 * Output:
 *      None.
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
dal_ssw_l2_lookupMissFloodPortMask_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask)
{
    int32   ret;
    uint32  reg_idx;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
           
    /* parameter check */
    RT_PARAM_CHK(NULL == pFlood_portmask, RT_ERR_NULL_POINTER);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pFlood_portmask=0x%x", 
           unit, pFlood_portmask->bits[0]);

    reg_idx = SSW_LOOKUP_MISS_FLOODING_PORTMASKr;

    L2_SEM_LOCK(unit); 
    if ((ret = reg_field_write(unit, reg_idx, SSW_FLOODING_PORTMASKf, &(pFlood_portmask->bits[0]))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_ssw_l2_lookupMissFloodPortMask_set */

/* Function Name:
 *      dal_ssw_l2_lookupMissFloodPortMask_add
 * Description:
 *      Add one port member to flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      unit       - unit id   
 *      type        - type of lookup miss
 *      flood_port - port id that is going to be added in flooding port mask.  
 * Output:
 *      None.
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
dal_ssw_l2_lookupMissFloodPortMask_add(uint32 unit, rtk_l2_lookupMissType_t type, rtk_port_t flood_port)
{
    int32 ret;
    rtk_portmask_t  portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, flood_port=%d", 
           unit, flood_port);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, flood_port), RT_ERR_PORT_ID);
    
    osal_memset(&portmask, 0, sizeof(rtk_portmask_t));
    
    ret = dal_ssw_l2_lookupMissFloodPortMask_get(unit, type, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    RTK_PORTMASK_PORT_SET(portmask, flood_port);
    
    ret = dal_ssw_l2_lookupMissFloodPortMask_set(unit, type, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    return RT_ERR_OK;    
} /* end of dal_ssw_l2_lookupMissFloodPortMask_add */

/* Function Name:
 *      dal_ssw_l2_lookupMissFloodPortMask_del
 * Description:
 *      Del one port member in flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      unit       - unit id   
 *      type        - type of lookup miss
 *      flood_port - port id that is going to be added in flooding port mask.  
 * Output:
 *      None.
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
dal_ssw_l2_lookupMissFloodPortMask_del(uint32 unit, rtk_l2_lookupMissType_t type, rtk_port_t flood_port)
{
    int32 ret;
    rtk_portmask_t  portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, flood_port=%d", 
           unit, flood_port);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, flood_port), RT_ERR_PORT_ID);
    
    osal_memset(&portmask, 0, sizeof(rtk_portmask_t));
    
    ret = dal_ssw_l2_lookupMissFloodPortMask_get(unit, type, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    RTK_PORTMASK_PORT_CLEAR(portmask, flood_port);
    
    ret = dal_ssw_l2_lookupMissFloodPortMask_set(unit, type, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    return RT_ERR_OK;    
} /* end of dal_ssw_l2_lookupMissFloodPortMask_del */

/* Function Name:
 *      dal_ssw_l2_srcPortEgrFilterMask_get
 * Description:
 *      Get source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit             - unit id     
 * Output:
 *      pFilter_portmask - source port egress filtering configuration when packet egress.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
int32 
dal_ssw_l2_srcPortEgrFilterMask_get(uint32 unit, rtk_portmask_t *pFilter_portmask)
{
    int32   ret;
    uint32  reg_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
           
    /* parameter check */
    RT_PARAM_CHK(NULL == pFilter_portmask, RT_ERR_NULL_POINTER);

    reg_idx = SSW_EN_EGRESS_CONTROLr;

    L2_SEM_LOCK(unit); 
    if ((ret = reg_field_read(unit, reg_idx, SSW_EN_EGRESSf, &(pFilter_portmask->bits[0]))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    L2_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "filter_portmask=0x%x", 
           pFilter_portmask->bits[0]);
           
    return ret;
} /* end of dal_ssw_l2_srcPortEgrFilterMask_get */

/* Function Name:
 *      dal_ssw_l2_srcPortEgrFilterMask_set
 * Description:
 *      Set source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit             - unit id  
 *      pFilter_portmask - source port egress filtering configuration when packet egress.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
int32 
dal_ssw_l2_srcPortEgrFilterMask_set(uint32 unit, rtk_portmask_t *pFilter_portmask)
{
    int32   ret;
    uint32  reg_idx;
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
           
    /* parameter check */
    RT_PARAM_CHK(NULL == pFilter_portmask, RT_ERR_NULL_POINTER);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, filter_portmask=%d", 
           unit, pFilter_portmask->bits[0]);

    reg_idx = SSW_EN_EGRESS_CONTROLr;

    L2_SEM_LOCK(unit); 
    if ((ret = reg_field_write(unit, reg_idx, SSW_EN_EGRESSf, &(pFilter_portmask->bits[0]))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    L2_SEM_UNLOCK(unit);
              
    return ret;
} /* end of dal_ssw_l2_srcPortEgrFilterMask_set */

/* Function Name:
 *      dal_ssw_l2_srcPortEgrFilterMask_add
 * Description:
 *      Add one port member to source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit        - unit id   
 *      filter_port - port id that is going to be added in source port egress filtering mask.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
int32 
dal_ssw_l2_srcPortEgrFilterMask_add(uint32 unit, rtk_port_t filter_port)
{
    int32 ret;
    rtk_portmask_t  portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, filter_port=%d", 
           unit, filter_port);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, filter_port), RT_ERR_PORT_ID);
    
    osal_memset(&portmask, 0, sizeof(rtk_portmask_t));
    
    ret = dal_ssw_l2_srcPortEgrFilterMask_get(unit, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    RTK_PORTMASK_PORT_SET(portmask, filter_port);
    
    ret = dal_ssw_l2_srcPortEgrFilterMask_set(unit, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    return RT_ERR_OK;    
} /* end of dal_ssw_l2_srcPortEgrFilterMask_add */

/* Function Name:
 *      dal_ssw_l2_srcPortEgrFilterMask_del
 * Description:
 *      Del one port member in source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit        - unit id   
 *      filter_port - port id that is going to be deleted in source port egress filtering mask.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
int32 
dal_ssw_l2_srcPortEgrFilterMask_del(uint32 unit, rtk_port_t filter_port)
{
    int32 ret;
    rtk_portmask_t  portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, filter_port=%d", 
           unit, filter_port);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, filter_port), RT_ERR_PORT_ID);
    
    osal_memset(&portmask, 0, sizeof(rtk_portmask_t));
    
    ret = dal_ssw_l2_srcPortEgrFilterMask_get(unit, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    RTK_PORTMASK_PORT_CLEAR(portmask, filter_port);
    
    ret = dal_ssw_l2_srcPortEgrFilterMask_set(unit, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    return RT_ERR_OK;    
} /* end of dal_ssw_l2_srcPortEgrFilterMask_del */

/* Function Name:
 *      dal_ssw_l2_addrEntry_get
 * Description:
 *      Get the L2 table entry by index of the specified unit.
 * Input:
 *      unit  - unit id
 *      index - l2 table index
 * Output:
 *      pL2_entry - pointer buffer of l2 table entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1) The index valid range is from 0 to (L2 hash table size + L2 CAM table size - 1)
 *         - 0 ~ (L2 hash table size - 1) entry in L2 hash table
 *         - (L2 hash table size) ~ (L2 hash table size + L2 CAM table size - 1) entry in L2 CAM table
 *      2) The output entry have 2 variables (valid and entry_type) and its detail data structure
 *         - valid: 1 mean the entry is valid; 0: invalid
 *         - entry_type: FLOW_TYPE_UNICAST, FLOW_TYPE_L2_MULTI and FLOW_TYPE_IP_MULTI
 *                       the field is ignored if valid field is 0.
 *         - detail data structure is ignored if valid is 0, and its filed meanings is depended
 *           on the entry_type value.
 *      3) If pL2_entry->flags have enabled the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the
 *         pL2_entry->unicast.trk_gid value is valid trunk id value.
 */
int32
dal_ssw_l2_addrEntry_get(uint32 unit, uint32 index, rtk_l2_entry_t *pL2_entry)
{
    uint32  l2_tableSize, l2cam_tableSize;
    uint32  hashkey = 0, location = 0, isValid = 0, l2cam_idx = 0;
    int32   ret;
    dal_ssw_l2_entry_t  l2_entry;
    multicast_index_entry_t mcast_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", unit, index);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* check index valid range */
    if ((ret = table_size_get(unit, SSW_L2CAM_TABLEt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    if ((ret = table_size_get(unit, SSW_L2_TABLEt, &l2_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    RT_PARAM_CHK(index >= (int32)(l2_tableSize+l2cam_tableSize), RT_ERR_OUT_OF_RANGE);
    /* check pointer buffer of l2 table entry */
    RT_PARAM_CHK(NULL == pL2_entry, RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    if (index < l2_tableSize)
    {   /* L2 Hash Table Entry */
        hashkey = (index >> 2) & 0xFFF;
        location = index & BITMASK_2B;
        if ((ret = _dal_ssw_l2_getL2EntryfromHash(unit, hashkey, location, &l2_entry, &isValid)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
    }
    else
    {   /* L2 CAM Table Entry */
        l2cam_idx = index & 0xFFF;
        if ((ret = _dal_ssw_l2_getL2EntryfromCAM(unit, l2cam_idx, &l2_entry, &isValid)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
    }

    /* Transfer from dal_ssw_l2_entry_t to rtk_l2_entry_t */
    pL2_entry->valid = isValid;
    if (pL2_entry->valid)
    {
        pL2_entry->entry_type = l2_entry.entry_type;    
        switch (pL2_entry->entry_type)
        {
            case FLOW_TYPE_UNICAST:
                pL2_entry->unicast.vid = l2_entry.unicast.fid;
                osal_memcpy(&pL2_entry->unicast.mac.octet[0], &l2_entry.unicast.mac.octet[0], sizeof(rtk_mac_t));
                pL2_entry->unicast.port = l2_entry.unicast.port;
                pL2_entry->unicast.flags = 0;
                pL2_entry->unicast.state = 0;

                for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
                {
                    if (dal_ssw_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
                    {
                        if ((first_trunkMember = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
                        {
                            /* no trunk member */
                            continue;
                        }
            
                        if ((RTK_PORTMASK_IS_PORT_SET(trunk_portmask, pL2_entry->unicast.port)) &&
                            (first_trunkMember == pL2_entry->unicast.port))
                        {
                            pL2_entry->unicast.trk_gid = trk_gid;
                            pL2_entry->unicast.flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
                            break;
                        }
                    }
                }
                if(l2_entry.unicast.sablock)
                    pL2_entry->unicast.flags |= RTK_L2_UCAST_FLAG_SA_BLOCK;
                if(l2_entry.unicast.dablock)
                    pL2_entry->unicast.flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
                if(l2_entry.unicast.is_static)
                    pL2_entry->unicast.flags |= RTK_L2_UCAST_FLAG_STATIC;
                pL2_entry->unicast.auth = l2_entry.unicast.auth;
                if(l2_entry.unicast.aging == 0)
                    pL2_entry->unicast.isAged = TRUE;
                else
                    pL2_entry->unicast.isAged = FALSE;
                break;
    
            case FLOW_TYPE_L2_MULTI:
                pL2_entry->l2mcast.rvid = l2_entry.l2mcast.rvid;
                osal_memcpy(&pL2_entry->l2mcast.mac.octet[0], &l2_entry.l2mcast.mac.octet[0], sizeof(rtk_mac_t));
                pL2_entry->l2mcast.fwdIndex = l2_entry.l2mcast.index;
                osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                if ((ret = table_read(unit, SSW_MULTICAST_INDEX_TABLEt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                if ((ret = table_field_get(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, pL2_entry->l2mcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                break;
    
            case FLOW_TYPE_IP4_MULTI:
                pL2_entry->ipmcast.rvid = l2_entry.ipmcast.rvid;
                pL2_entry->ipmcast.dip = l2_entry.ipmcast.dip;
                pL2_entry->ipmcast.sip = l2_entry.ipmcast.sip;
                pL2_entry->ipmcast.fwdIndex = l2_entry.ipmcast.index;
                osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                if ((ret = table_read(unit, SSW_MULTICAST_INDEX_TABLEt, l2_entry.ipmcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                if ((ret = table_field_get(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, pL2_entry->ipmcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                break;
    
            default:
                L2_SEM_UNLOCK(unit);
                return RT_ERR_FAILED;
        }
    }        

    L2_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of dal_ssw_l2_addrEntry_get */

/* Function Name:
 *      dal_ssw_l2_conflictAddr_get
 * Description:
 *      Get the conflict L2 table entry from one given L2 address in the specified unit.
 * Input:
 *      unit            - unit id
 *      pL2Addr         - l2 address to find its conflict entries
 *      cfAddrList_size - buffer size of the pCfAddrList
 * Output:
 *      pCfAddrList - pointer buffer of the conflict l2 table entry list
 *      pCf_retCnt  - return number of find conflict l2 table entry list
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      (1) The function can be used if add l2 entry return RT_ERR_L2_NO_EMPTY_ENTRY.
 *          Input the pL2Addr->entry_type and its hash key to get conflict entry information.
 *      (2) User want to prepare the return buffer pCfAddrList and via. cfAddrList_size argument
 *          tell driver its size.
 *      (3) The function will return valid L2 hash entry from the same bucket and the return number
 *          is filled in pCf_retCnt, entry data is filled in pCfAddrList.
 */
int32
dal_ssw_l2_conflictAddr_get(
    uint32          unit,
    rtk_l2_entry_t  *pL2Addr,
    rtk_l2_entry_t  *pCfAddrList,
    uint32          cfAddrList_size,
    uint32          *pCf_retCnt)
{
    uint32  hash_key = 0, hash_depth;
    uint32  isValid;
    uint32  cf_num;
    uint32  trk_gid, first_trunkMember;
    rtk_portmask_t  trunk_portmask;
    int32   ret = RT_ERR_FAILED;
    dal_ssw_l2_entry_t  l2_entry;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    /* check input parameters */
    RT_PARAM_CHK(NULL == pL2Addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pCfAddrList, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(0 == cfAddrList_size, RT_ERR_INPUT);
    RT_PARAM_CHK(NULL == pCf_retCnt, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pL2Addr->entry_type >= FLOW_TYPE_END, RT_ERR_OUT_OF_RANGE);

    osal_memset(&l2_entry, 0, sizeof(dal_ssw_l2_entry_t));
    /* calculate the hash index from pL2Addr input */
    if (pL2Addr->entry_type == FLOW_TYPE_UNICAST)
    {   /* FLOW_TYPE_UNICAST */
        l2_entry.entry_type = FLOW_TYPE_UNICAST;
        l2_entry.unicast.fid = pL2Addr->unicast.vid;
        osal_memcpy(&l2_entry.unicast.mac.octet[0], &pL2Addr->unicast.mac.octet[0], sizeof(rtk_mac_t));
    }
    else if (pL2Addr->entry_type == FLOW_TYPE_L2_MULTI)
    {   /* FLOW_TYPE_L2_MULTI */
        l2_entry.entry_type = FLOW_TYPE_L2_MULTI;
        l2_entry.l2mcast.rvid = pL2Addr->l2mcast.rvid;
        osal_memcpy(&l2_entry.l2mcast.mac.octet[0], &pL2Addr->l2mcast.mac.octet[0], sizeof(rtk_mac_t));
    }
    else
    {   /* FLOW_TYPE_IP4_MULTI */
        l2_entry.entry_type = FLOW_TYPE_IP4_MULTI;
        l2_entry.ipmcast.dip = pL2Addr->ipmcast.dip;
        l2_entry.ipmcast.sip = pL2Addr->ipmcast.sip;
    }

    _dal_ssw_l2_entryToHashKey(unit, &l2_entry, &hash_key);
    cf_num = 0;
    for (hash_depth = 0; hash_depth < HAL_L2_HASHDEPTH(unit) && cf_num < cfAddrList_size; hash_depth++)
    {
        osal_memset(&l2_entry, 0, sizeof(dal_ssw_l2_entry_t));
        isValid = 0;
        if (_dal_ssw_l2_getL2EntryfromHash(unit, hash_key, hash_depth, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }
        if (isValid)
        {   /* filled conflict address to return buffer pCfAddrList */
            (pCfAddrList + cf_num)->entry_type = l2_entry.entry_type;
            (pCfAddrList + cf_num)->valid = isValid;
            switch (l2_entry.entry_type)
            {
                case FLOW_TYPE_UNICAST:
                    (pCfAddrList + cf_num)->unicast.vid = l2_entry.unicast.fid;
                    osal_memcpy(&(pCfAddrList + cf_num)->unicast.mac.octet[0], &l2_entry.unicast.mac.octet[0], sizeof(rtk_mac_t));
                    (pCfAddrList + cf_num)->unicast.port = l2_entry.unicast.port;
                    (pCfAddrList + cf_num)->unicast.flags = 0;
                    (pCfAddrList + cf_num)->unicast.state = 0;
                    (pCfAddrList + cf_num)->unicast.l2_idx = (hash_key << 2) | hash_depth;

                    for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
                    {
                        if (dal_ssw_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
                        {
                            if ((first_trunkMember = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
                            {
                                /* no trunk member */
                                continue;
                            }
                
                            if ((RTK_PORTMASK_IS_PORT_SET(trunk_portmask, (pCfAddrList + cf_num)->unicast.port)) &&
                                (first_trunkMember == (pCfAddrList + cf_num)->unicast.port))
                            {
                                (pCfAddrList + cf_num)->unicast.trk_gid = trk_gid;
                                (pCfAddrList + cf_num)->unicast.flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
                                break;
                            }
                        }
                    }
                    if(l2_entry.unicast.sablock)
                        (pCfAddrList + cf_num)->unicast.flags |= RTK_L2_UCAST_FLAG_SA_BLOCK;
                    if(l2_entry.unicast.dablock)
                        (pCfAddrList + cf_num)->unicast.flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
                    if(l2_entry.unicast.is_static)
                        (pCfAddrList + cf_num)->unicast.flags |= RTK_L2_UCAST_FLAG_STATIC;
                    (pCfAddrList + cf_num)->unicast.auth = l2_entry.unicast.auth;
                    if(l2_entry.unicast.aging == 0)
                        (pCfAddrList + cf_num)->unicast.isAged = TRUE;
                    else
                        (pCfAddrList + cf_num)->unicast.isAged = FALSE;
                    break;
        
                case FLOW_TYPE_L2_MULTI:
                    (pCfAddrList + cf_num)->l2mcast.rvid = l2_entry.l2mcast.rvid;
                    osal_memcpy(&(pCfAddrList + cf_num)->l2mcast.mac.octet[0], &l2_entry.l2mcast.mac.octet[0], sizeof(rtk_mac_t));
                    (pCfAddrList + cf_num)->l2mcast.fwdIndex = l2_entry.l2mcast.index;
                    (pCfAddrList + cf_num)->l2mcast.l2_idx = (hash_key << 2) | hash_depth;
                    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

                    if ((ret = table_read(unit, SSW_MULTICAST_INDEX_TABLEt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_field_get(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, (pCfAddrList + cf_num)->l2mcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    break;
        
                case FLOW_TYPE_IP4_MULTI:
                    (pCfAddrList + cf_num)->ipmcast.rvid = l2_entry.ipmcast.rvid;
                    (pCfAddrList + cf_num)->ipmcast.dip = l2_entry.ipmcast.dip;
                    (pCfAddrList + cf_num)->ipmcast.sip = l2_entry.ipmcast.sip;
                    (pCfAddrList + cf_num)->ipmcast.fwdIndex = l2_entry.ipmcast.index;
                    (pCfAddrList + cf_num)->ipmcast.l2_idx = (hash_key << 2) | hash_depth;
                    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                    if ((ret = table_read(unit, SSW_MULTICAST_INDEX_TABLEt, l2_entry.ipmcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_field_get(unit, SSW_MULTICAST_INDEX_TABLEt, SSW_MULTICAST_INDEX_TABLE_PORTMSKf, (pCfAddrList + cf_num)->ipmcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    break;
        
                default:
                    return RT_ERR_FAILED;
            }

            /* increase the count */
            cf_num++;
        }
    }

    (*pCf_retCnt) = cf_num;
    return RT_ERR_OK;
} /* end of dal_ssw_l2_conflictAddr_get */
