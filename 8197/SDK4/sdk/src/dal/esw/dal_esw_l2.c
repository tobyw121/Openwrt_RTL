/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 41321 $
 * $Date: 2013-07-19 15:27:25 +0800 (Fri, 19 Jul 2013) $
 *
 * Purpose : Definition those public L2 APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Mac address flush
 *           2) Address learning limit
 *           3) Parameter for L2 lookup and learning engine
 *           4) Unicast address
 *           5) L2 multicast
 *           6) IP multicast
 *           7) Multicast forwarding table
 *           8) CPU mac
 *           9) Port move
 *           10) Parameter for lookup miss
 *           11) Parameter for MISC
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
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/esw/dal_esw_l2.h>
#include <dal/esw/dal_esw_vlan.h>
#include <dal/esw/dal_esw_trunk.h>
#include <rtk/l2.h>
#include <rtk/default.h>

/* 
 * Symbol Definition 
 */
#undef  CONFIG_SDK_WA_LIMIT_LEARN_COUNT
#define CONFIG_SDK_WA_FORWARD_TABLE

#define END_OF_MCAST_IDX    (0xFFFF)
#define MCAST_IDX_ALLOCATED (0xFFFE) 
#define DAL_ESW_FIDMACCONSTRN_MAX_ENTRY     (31)
#define ACCADDR_L2TYPE_OFFSET               (14)

#ifndef CAM
#define CAM 1
#endif

enum dal_esw_l2_hash_algo_e
{
    ESW_L2_HASH_ALGO0,
    ESW_L2_HASH_ALGO1,
    ESW_L2_HASH_ALGO_END,
};

typedef enum l2_entry_type_e
{
    L2_UNICAST = 0,
    L2_MULTICAST,
    IP_MULTICAST,
    L2_ENTRY_TYPE_END
} l2_entry_type_t;

typedef enum dal_esw_l2_getMethod_e
{
    L2_GET_EXIST_ONLY = 0,
    L2_GET_EXIST_OR_FREE,
    L2_GET_FREE_ONLY,
    DAL_ESW_GETMETHOD_END
} dal_esw_l2_getMethod_t;

typedef enum dal_esw_l2_indexType_e
{
    L2_IN_HASH = 0,
    L2_IN_CAM,
    DAL_ESW_L2_INDEXTYPE_END
} dal_esw_l2_indexType_t;

typedef enum fwd_entry_state_e
{
    ENTRY_STATE_AVAILABLE = 0,
    ENTRY_STATE_BACKUP,
    ENTRY_STATE_INUSED,
    ENTRY_STATE_END
} fwd_entry_state_t;

typedef struct dal_esw_l2_entry_s
{
    l2_entry_type_t  entry_type; /* unicast, l2 multicast, ip multicast */
    uint32           is_entry_exist;
    uint32  valid;
    union
    {
        struct unicast_entry_s
        {
            rtk_fid_t   fid;
            rtk_mac_t   mac;
            rtk_port_t  port;
            uint32      aging;
            uint32      sablock;
            uint32      dablock;
            uint32      auth;
            uint32      is_static;
            uint32      nh;
            uint32      suspend ;
        } unicast;
        struct l2mcast_entry_s
        {
            rtk_vlan_t  rvid;
            rtk_mac_t   mac;
            uint32      index;
        } l2mcast;
        struct ipmcast_entry_s
        {
            ipaddr_t    dip;
            ipaddr_t    sip;
            uint32      index;
        } ipmcast;
    };
} dal_esw_l2_entry_t;

typedef struct dal_esw_l2_index_s
{
    uint32  index_type;     /* In CAM or In HASH */
    uint32  index;
    uint32  hashdepth;      /* only useful when entry is in hash table */
} dal_esw_l2_index_t;

typedef struct dal_esw_mcast_index_s {
    uint16  next_index;
    uint16  ref_count;
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
    uint32  crossVlan;
    rtk_portmask_t  portmask;    
    fwd_entry_state_t   entry_state;
    dal_esw_l2_index_t  l2_index;
#endif    
} dal_esw_mcast_index_t;

typedef struct dal_esw_mcast_index_pool_s {
    dal_esw_mcast_index_t   *pMcast_index_pool;
    uint32                  size_of_mcast_fwd_index;
    uint16                  free_index_head;
    uint16                  free_entry_count;
} dal_esw_mcast_index_pool_t;


/* 
 * Data Declaration 
 */
static uint32               l2_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         l2_sem[RTK_MAX_NUM_OF_UNIT];
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
static uint32               fwd_entry_inused_count;
#endif

/* Multicast database */
static dal_esw_mcast_index_pool_t    mcast_idx_pool[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */
 
/* L2 semaphore handling */
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
static int32 _dal_esw_l2_init_config(uint32 unit);
static int32 _dal_esw_l2_getExistOrFreeL2Entry(uint32 unit, dal_esw_l2_entry_t *pL2_entry, dal_esw_l2_getMethod_t get_method
        , dal_esw_l2_index_t *pL2_index);
static int32 _dal_esw_l2_getL2EntryfromHash(uint32 unit, uint32 hashkey, uint32 location, 
                              dal_esw_l2_entry_t *pL2_entry, uint32 *pIsValid);
static int32 _dal_esw_l2_entryToHashKey(uint32 unit, dal_esw_l2_entry_t *pL2_entry, uint32 *pKey);
static int32 _dal_esw_l2_compareEntry(dal_esw_l2_entry_t *pSrcEntry, dal_esw_l2_entry_t *pDstEntry);
static int32 _dal_esw_l2_getL2EntryfromCAM(uint32 unit, uint32 index, 
                              dal_esw_l2_entry_t *pL2_entry, uint32 *pIsValid);
static int32 _dal_esw_l2_setL2CAMEntry(uint32 unit, dal_esw_l2_entry_t *pL2_entry, dal_esw_l2_index_t *pL2_index);
static int32 _dal_esw_l2_setL2HASHEntry(uint32 unit, dal_esw_l2_entry_t *pL2_entry, dal_esw_l2_index_t *pL2_index);
static int32 _dal_esw_l2_freeMcastIdx(uint32 unit, int32 mcastIdx);
static int32 _dal_esw_l2_allocMcastIdx(uint32 unit, int32 *pMcastIdx);
static int32 _dal_esw_l2_isMcastIdxUsed(uint32 unit, int32 mcastIdx);
static int32 _dal_esw_l2_nextValidAddr_get(uint32 unit, int32 *pScan_idx, uint32 type,
                            uint32 include_static, dal_esw_l2_entry_t  *pL2_data);
static int32 _dal_esw_l2_nextValidAddrByRange_get(uint32 unit, int32 *pScan_idx, uint32 end_idx, uint32 type,
                            uint32 include_static, dal_esw_l2_entry_t  *pL2_data);
static int32 _dal_esw_hashKeyToL2_entry(uint32 unit, uint32 hashKey, uint32 aed0, uint32 aed1, 
                            dal_esw_l2_entry_t *pL2_entry);
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
static int32 _dal_esw_l2_mcastFwdIndex_arrange(uint32 unit, int32 index);
#endif
static int32 _dal_esw_l2_getFirstDynamicEntry(uint32 unit, dal_esw_l2_entry_t *pL2_entry,
                            dal_esw_l2_index_t *pL2_index);


/* Function Name:
 *      dal_esw_l2_init
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
dal_esw_l2_init(uint32 unit)
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

    if ((ret = table_size_get(unit, ESW_FORWARDINGt, &mcast_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    L2_SEM_LOCK(unit);
           
    /* allocate memory for free multicast index */
    mcast_idx_pool[unit].pMcast_index_pool = (dal_esw_mcast_index_t *)osal_alloc(mcast_tableSize * sizeof(dal_esw_mcast_index_t));
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
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
        if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit) || CHIP_REV_ID_B == HAL_GET_CHIP_REV_ID(unit))
        {
            mcast_idx_pool[unit].pMcast_index_pool[index].crossVlan = 0;
            osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[index].portmask.bits, 0, sizeof(rtk_portmask_t));
            mcast_idx_pool[unit].pMcast_index_pool[index].entry_state = ENTRY_STATE_AVAILABLE;
            osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[index].l2_index, 0, sizeof(dal_esw_l2_index_t));
        }
#endif
    }
    mcast_idx_pool[unit].pMcast_index_pool[mcast_tableSize - 1].next_index = END_OF_MCAST_IDX;
    mcast_idx_pool[unit].pMcast_index_pool[mcast_tableSize - 1].ref_count = 0;    
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
    if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit) || CHIP_REV_ID_B == HAL_GET_CHIP_REV_ID(unit))
    {
        mcast_idx_pool[unit].pMcast_index_pool[mcast_tableSize - 1].crossVlan = 0;
        osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[mcast_tableSize - 1].portmask.bits, 0, sizeof(rtk_portmask_t));
        mcast_idx_pool[unit].pMcast_index_pool[mcast_tableSize - 1].entry_state = ENTRY_STATE_AVAILABLE;
        osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[mcast_tableSize - 1].l2_index, 0, sizeof(dal_esw_l2_index_t));
        fwd_entry_inused_count = 0;
    }
#endif

    L2_SEM_UNLOCK(unit);
    
    /* set init flag to complete init */
    l2_init[unit] = INIT_COMPLETED;
    
    if ((ret = _dal_esw_l2_init_config(unit)) != RT_ERR_OK)
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
} /* end of dal_esw_l2_init */

/* Module Name    : L2                */
/* Sub-module Name: Mac address flush */

/* Function Name:
 *      dal_esw_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer buffer of state of HW clear linkdown port mac
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Make sure chip have supported the function before using the API.
 *      2. The API is apply to whole system.
 *      3. The status of flush linkdown port address is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_esw_l2_flushLinkDownPortAddrEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);
    
    if(( ret = reg_field_read(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, 
            ESW_LINK_DOWN_PORT_INVALIDf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_l2_flushLinkDownPortAddrEnable_get*/

/* Function Name:
 *      dal_esw_l2_flushLinkDownPortAddrEnable_set
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
 *      - DISABLED
 *      - ENABLED
 */
int32
dal_esw_l2_flushLinkDownPortAddrEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);    

    L2_SEM_LOCK(unit);
    
    if((ret = reg_field_write(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, 
                        ESW_LINK_DOWN_PORT_INVALIDf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}/*end of dal_esw_l2_flushLinkDownPortAddrEnable_set*/

/* Function Name:
 *      dal_esw_l2_ucastAddr_flush
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
 *      None
 */
int32
dal_esw_l2_ucastAddr_flush(uint32 unit, rtk_l2_flushCfg_t *pConfig)
{
    int32  ret = RT_ERR_OK;
    uint32 busy = 0, val;
    uint32 tryTime = 0;
    uint32 trunkMode, trunk_representPort;
    rtk_portmask_t  trunk_portmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

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

    if(pConfig->flushByPort)
    {
        if (ENABLED == pConfig->portOrTrunk)
            RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, pConfig->port), RT_ERR_PORT_ID);
        else
        {
            RT_PARAM_CHK(pConfig->port >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
            if ((ret = dal_esw_trunk_mode_get(unit, &trunkMode)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if (TRUNK_MODE_DUMB == trunkMode)
            {
                if ((ret = dal_esw_trunk_port_get(unit, pConfig->port, &trunk_portmask)) != RT_ERR_OK)
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
            else
            {
                if ((ret = dal_esw_trunk_representPort_get(unit, pConfig->port, &trunk_representPort)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                pConfig->port = trunk_representPort;
            }
        }
    }

    L2_SEM_LOCK(unit); 

    /*Assign Port*/
    if(( ret = reg_field_write(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL0r, 
                ESW_SPPORTf, &(pConfig->flushByPort))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }    
    
    if(pConfig->flushByPort)
    {
        /*Config target port number*/
        if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL0r, 
                    ESW_PORTNUMBERf, &(pConfig->port))) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    /*Assign vid*/
    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL0r, ESW_SPFIDf, &(pConfig->flushByVid))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    if(pConfig->flushByVid)
    {
        /*Config target vid/fid*/
        if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL0r, ESW_FIDf, &(pConfig->vid))) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    
    /*Flush static entry or not*/
    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL0r, ESW_STATIC_ACTf, &(pConfig->flushStaticAddr))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /*Assign mac address*/
    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL1r, ESW_SPMACf, &(pConfig->flushByMac))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    if(pConfig->flushByMac)
    {
        /*Config target mac address*/
        val = (pConfig->ucastAddr.octet[0] << 8)|pConfig->ucastAddr.octet[1];
        if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL1r, ESW_SPMAC_47_32f, &val)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        val = (pConfig->ucastAddr.octet[2] << 24)|(pConfig->ucastAddr.octet[3] << 16)|(pConfig->ucastAddr.octet[4] << 8)|pConfig->ucastAddr.octet[5];
        if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL2r, ESW_SPMAC_31_0f, &val)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    if(pConfig->flushAddrOnAllPorts)
    {
        val = ENABLED;
        if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL0r, ESW_SPPORTf, &val)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        
        /*port number 31 means all the port*/
        val = 31;
        if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL0r, ESW_PORTNUMBERf, &val)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    /*Trigger Fulsh action*/
    val = ENABLED;
    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL0r, ESW_REMOVEACT_TRAGf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    osal_time_usleep(10000);

    /*Wait flush action completed*/
    do{        
        if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL0r, ESW_REMOVEACT_TRAGf, &busy)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        tryTime++;
        if(512 == tryTime)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_BUSYWAIT_TIMEOUT, (MOD_DAL|MOD_L2), "Busy wait timeout");
            return RT_ERR_BUSYWAIT_TIMEOUT;
        }
    }while(busy);

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_ucastAddr_flush*/

/* Module Name    : L2                     */
/* Sub-module Name: Address learning limit */

/* Function Name:
 *      dal_esw_l2_portLearningCnt_get
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
dal_esw_l2_portLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((NULL == pMac_cnt), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_read(unit, ESW_PORT_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0r,
                         port, REG_ARRAY_INDEX_NONE, ESW_PCURMACNUMf, pMac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac_cnt%d", *pMac_cnt); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_l2_portLearningCnt_get*/

/* Function Name:
 *      dal_esw_l2_portLimitLearningCntEnable_get
 * Description:
 *      Get enable status of limiting MAC learning on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of limiting MAC learning
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
dal_esw_l2_portLimitLearningCntEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_read(unit, ESW_PORT_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL1r,
                    port, REG_ARRAY_INDEX_NONE, ESW_PMACNUMCTLf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_portLimitLearningCntEnable_get*/

/* Function Name:
 *      dal_esw_l2_portLimitLearningCntEnable_set
 * Description:
 *      Set enable status of limiting MAC learning on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of limiting MAC learning
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
dal_esw_l2_portLimitLearningCntEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, enable=%d", unit, port, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);    

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_write(unit, ESW_PORT_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL1r,
                        port, REG_ARRAY_INDEX_NONE, ESW_PMACNUMCTLf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_portLimitLearningCntEnable_set*/


/* Function Name:
 *      dal_esw_l2_portLimitLearningCnt_get
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
dal_esw_l2_portLimitLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((NULL == pMac_cnt), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);
    
    if((ret = reg_array_field_read(unit, ESW_PORT_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0r, 
                  port, REG_ARRAY_INDEX_NONE, ESW_PMAXMACNUMf, pMac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac_cnt=%d", *pMac_cnt); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_l2_portLimitLearningCnt_get*/

/* Function Name:
 *      dal_esw_l2_portLimitLearningCnt_set
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
dal_esw_l2_portLimitLearningCnt_set(uint32 unit, rtk_port_t port, uint32 mac_cnt)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, mac_cnt=%d", 
                unit, port, mac_cnt); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((mac_cnt > HAL_L2_LEARN_LIMIT_CNT_MAX(unit)) && (mac_cnt != HAL_L2_LEARN_LIMIT_CNT_DISABLE(unit)), RT_ERR_LIMITED_L2ENTRY_NUM);

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_write(unit, ESW_PORT_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0r,
                        port, REG_ARRAY_INDEX_NONE, ESW_PMAXMACNUMf, &mac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_portLimitLearningCnt_set*/


/* Function Name:
 *      dal_esw_l2_portLimitLearningCntAction_get
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
dal_esw_l2_portLimitLearningCntAction_get(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", 
                unit, port); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);  

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_read(unit, ESW_PORT_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL1r, 
                port, REG_ARRAY_INDEX_NONE, ESW_L2LIMACTf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    switch(val)
    {
        case 0:
            *pAction = LIMIT_LEARN_CNT_ACTION_DROP;
            break;
        case 1:
            *pAction = LIMIT_LEARN_CNT_ACTION_FORWARD;
            break;
        case 2:
            *pAction = LIMIT_LEARN_CNT_ACTION_TO_CPU;
            break;
        default:
            break;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAction%d", *pAction); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_l2_portLimitLearningCntAction_get*/

/* Function Name:
 *      dal_esw_l2_portLimitLearningCntAction_set
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
 * Note:
 *      1. The action symbol as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
int32
dal_esw_l2_portLimitLearningCntAction_set(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t action)
{
    int32  ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, action=%d", 
                unit, port, action); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  

    switch(action)
    {
        case LIMIT_LEARN_CNT_ACTION_DROP:
            val = 0;
            break;
        case LIMIT_LEARN_CNT_ACTION_FORWARD:
            val = 1;
            break;
        case LIMIT_LEARN_CNT_ACTION_TO_CPU:
            val = 2;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_L2), "The error input action");
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_write(unit, ESW_PORT_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL1r,
                    port,0, ESW_L2LIMACTf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_l2_portLimitLearningCntAction_set*/

/* Function Name:
 *      dal_esw_l2_portLastLearnedMac_get
 * Description:
 *      Get lastest learned MAC address on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pFid - lastest learned fid
 *      pMac - lastest learned mac address
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
dal_esw_l2_portLastLearnedMac_get(uint32 unit, rtk_port_t port, rtk_fid_t *pFid, rtk_mac_t *pMac)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", 
                unit, port); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((NULL == pFid) || (NULL == pMac), RT_ERR_NULL_POINTER);  

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_read(unit, ESW_PORT_LAST_PACKET_INFORMATION_CONTROL1r,
                port, REG_ARRAY_INDEX_NONE, ESW_PLAVIDf, pFid)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    if((ret = reg_array_read(unit, ESW_PORT_LAST_PACKET_INFORMATION_CONTROL0r,
                 port, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    pMac->octet[0] = (val >> 24) & 0xff;
    pMac->octet[1] = (val >> 16) & 0xff;
    pMac->octet[2] = (val >> 8) & 0xff;
    pMac->octet[3] = val & 0xff;
    
    if((ret = reg_array_field_read(unit, ESW_PORT_LAST_PACKET_INFORMATION_CONTROL1r,
                port, REG_ARRAY_INDEX_NONE, ESW_PLASMAC_15_0f, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    
    pMac->octet[4] = (val >> 8) & 0xff;
    pMac->octet[5] = val & 0xff;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pFid=%d, pMac=0x%02x:%02x:%02x:%02x:%02x:%02x", 
               *pFid, pMac->octet[0],pMac->octet[1],pMac->octet[2],pMac->octet[3],pMac->octet[4],
               pMac->octet[5]); 
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_portLastLearnedMac_get*/


/* Function Name:
 *      dal_esw_l2_fidLimitLearningEntry_get
 * Description:
 *      Get FID MAC limit entry.
 * Input:
 *      unit              - unit id
 *      fid_macLimit_idx  - index of FID MAC limit entry
 * Output:
 *      pFidMacLimitEntry - pointer to MAC limit entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
int32
dal_esw_l2_fidLimitLearningEntry_get(
    uint32                    unit,
    uint32                    fid_macLimit_idx,
    rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry)
{
    int32   ret;
    uint32 baseAddr;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, fid_macLimit_idx=%d", 
                unit, fid_macLimit_idx); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((fid_macLimit_idx >= HAL_L2_FID_LEARN_LIMIT_ENTRY_MAX(unit)), RT_ERR_OUT_OF_RANGE); 
    RT_PARAM_CHK((NULL == pFidMacLimitEntry), RT_ERR_NULL_POINTER);     

    osal_memset(pFidMacLimitEntry, 0, sizeof(rtk_l2_fidMacLimitEntry_t));

    baseAddr = ESW_FID0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0r + 4 * fid_macLimit_idx;

    L2_SEM_LOCK(unit);

    /*Get valid bit of  the entry*/
    if((ret = reg_field_read(unit, baseAddr + 1, ESW_FIDMACNUMCTLf, &pFidMacLimitEntry->enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    if(pFidMacLimitEntry->enable)
    {
        /*Get Max Counter*/
        if((ret = reg_field_read(unit, baseAddr, ESW_FIDMAXMACNUMf, &pFidMacLimitEntry->maxNum)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        /*Get The Port Mask*/
        if((ret = reg_array_field_read(unit, ESW_FID_MAC_ADDRESS_NUMBER_CONSTRAIN_PORMASK_CONTROLr, fid_macLimit_idx, 
                        REG_ARRAY_INDEX_NONE, ESW_VPMASKf, &pFidMacLimitEntry->portmask.bits[0])) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        /*Get action of  the entry*/
        if((ret = reg_field_read(unit, baseAddr + 1, ESW_FIDL2LIMACTf, &val)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        
        switch(val)
        {
            case 0:
                pFidMacLimitEntry->action = LIMIT_LEARN_CNT_ACTION_DROP;
                break;
            case 1:
                pFidMacLimitEntry->action = LIMIT_LEARN_CNT_ACTION_FORWARD;
                break;
            case 2:
                pFidMacLimitEntry->action = LIMIT_LEARN_CNT_ACTION_TO_CPU;
                break;
            default:
                break;
        }

        /*Get The target fid*/
        if((ret = reg_field_read(unit, baseAddr + 1, ESW_FIDf, &pFidMacLimitEntry->fid)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }   
    }

    L2_SEM_UNLOCK(unit);   

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pFidMacLimitEntry->enable=%d, pFidMacLimitEntry->fid=%d,\
        pFidMacLimitEntry->maxNum=%d, pFidMacLimitEntry->portmask=0x%x, pFidMacLimitEntry->action=%d", 
        pFidMacLimitEntry->enable, pFidMacLimitEntry->fid,
        pFidMacLimitEntry->maxNum, pFidMacLimitEntry->portmask, pFidMacLimitEntry->action); 
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_fidLimitLearningEntry_get*/    

/* Function Name:
 *      dal_esw_l2_fidLimitLearningEntry_set
 * Description:
 *      Set FID MAC limit entry.
 * Input:
 *      unit              - unit id
 *      fid_macLimit_idx  - index of FID MAC limit entry
 *      pFidMacLimitEntry - MAC limit entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FID                  - invalid fid
 *      RT_ERR_PORT_MASK            - invalid portmask
 *      RT_ERR_LIMITED_L2ENTRY_NUM  - invalid limited L2 entry number
 *      RT_ERR_FWD_ACTION           - invalid forwarding action
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
int32
dal_esw_l2_fidLimitLearningEntry_set(
    uint32                    unit,
    uint32                    fid_macLimit_idx,
    rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry)
{
    int32   ret;
    uint32 baseAddr;
    rtk_l2_flushCfg_t config;
    uint32 val, tmpVal, busy;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, fid_macLimit_idx=%d", 
                unit, fid_macLimit_idx); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((fid_macLimit_idx >= HAL_L2_FID_LEARN_LIMIT_ENTRY_MAX(unit)), RT_ERR_OUT_OF_RANGE); 
    RT_PARAM_CHK((NULL == pFidMacLimitEntry), RT_ERR_NULL_POINTER); 

    switch(pFidMacLimitEntry->action)
    {
        case LIMIT_LEARN_CNT_ACTION_DROP:
            val = 0;
            break;
        case LIMIT_LEARN_CNT_ACTION_FORWARD:
            val = 1;
            break;
        case LIMIT_LEARN_CNT_ACTION_TO_CPU:
            val = 2;
            break;
        default:
            RT_ERR(RT_ERR_FWD_ACTION, (MOD_DAL|MOD_L2), "The error input action");
            return RT_ERR_INPUT;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pFidMacLimitEntry->enable=%d, pFidMacLimitEntry->fid=%d,\
        pFidMacLimitEntry->maxNum=%d, pFidMacLimitEntry->portmask=0x%x, pFidMacLimitEntry->action=%d", 
        pFidMacLimitEntry->enable, pFidMacLimitEntry->fid,
        pFidMacLimitEntry->maxNum, pFidMacLimitEntry->portmask, pFidMacLimitEntry->action); 

    baseAddr = ESW_FID0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0r + 4 * fid_macLimit_idx;

    L2_SEM_LOCK(unit);

    /*Disable the entry*/
    if(DISABLED == pFidMacLimitEntry->enable)
    {
        tmpVal = DISABLED;
        if((ret = reg_field_write(unit, baseAddr + 1, ESW_FIDMACNUMCTLf, &tmpVal)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        /*clear the current mac counter*/
        tmpVal = ENABLED;
        if((ret = reg_field_write(unit, baseAddr + 1, ESW_FIDVCN_CLRf, &tmpVal)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        /*asic bug: can not auto clear the bit*/
        tmpVal = DISABLED;
        if((ret = reg_field_write(unit, baseAddr + 1, ESW_FIDVCN_CLRf, &tmpVal)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }


        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Disable Fid Mac Constrain Function, fid_macLimit_idx=%d",
                        fid_macLimit_idx);
        
        L2_SEM_UNLOCK(unit);
        
        return RT_ERR_OK;        
    }

    /*First set action = Forward*/
    tmpVal = 1; /*Forward*/
    if((ret = reg_field_write(unit, baseAddr + 1, ESW_FIDL2LIMACTf, &tmpVal)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /*Then Se Max Counter == 0*/
    tmpVal = 0;
    if((ret = reg_field_write(unit, baseAddr, ESW_FIDMAXMACNUMf, &tmpVal)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /*Set The target fid*/
    if((ret = reg_field_write(unit, baseAddr + 1, ESW_FIDf, &(pFidMacLimitEntry->fid))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }   

    /*Set The Port Mask*/
    if((ret = reg_array_field_write(unit, ESW_FID_MAC_ADDRESS_NUMBER_CONSTRAIN_PORMASK_CONTROLr, fid_macLimit_idx, 
                    REG_ARRAY_INDEX_NONE, ESW_VPMASKf, &(pFidMacLimitEntry->portmask.bits[0]))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }    

    /*Enable The Entry*/
    tmpVal = ENABLED;
    if((ret = reg_field_write(unit, baseAddr + 1, ESW_FIDMACNUMCTLf, &tmpVal)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    } 

    L2_SEM_UNLOCK(unit);

    /*Flush The L2 Table of the Fid & port*/
    osal_memset(&config, 0, sizeof(rtk_l2_flushCfg_t));
    config.flushByVid = TRUE;
    config.vid = pFidMacLimitEntry->fid;
    config.flushStaticAddr = 0;
    if((ret = dal_esw_l2_ucastAddr_flush(unit, &config)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }  

    do
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL0r, ESW_REMOVEACT_TRAGf, &busy)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }while(busy);

    L2_SEM_LOCK(unit);

    /*clear the current mac counter*/
    tmpVal = ENABLED;
    if((ret = reg_field_write(unit, baseAddr + 1, ESW_FIDVCN_CLRf, &tmpVal)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }    

    /*asic bug: can not auto clear the bit*/
    tmpVal = DISABLED;
    if((ret = reg_field_write(unit, baseAddr + 1, ESW_FIDVCN_CLRf, &tmpVal)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /*Then Set the target Max Counter*/
    if((ret = reg_field_write(unit, baseAddr, ESW_FIDMAXMACNUMf, &(pFidMacLimitEntry->maxNum))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /*Then set real action*/    
    if((ret = reg_field_write(unit, baseAddr + 1, ESW_FIDL2LIMACTf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }       

    L2_SEM_UNLOCK(unit);    
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_fidLimitLearningEntry_set*/    

/* Function Name:
 *      dal_esw_l2_fidLearningCnt_get
 * Description:
 *      Get number of learned MAC addresses on specified fid.
 * Input:
 *      unit              - unit id
 *      fid_macLimit_idx  - index of FID MAC limit entry
 * Output:
 *      pNum - number of learned MAC addresses
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_l2_fidLearningCnt_get(uint32 unit, uint32 fid_macLimit_idx, uint32 *pNum)
{
    int32   ret;
    uint32 baseAddr;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, fid_macLimit_idx=%d", 
                unit, fid_macLimit_idx); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((fid_macLimit_idx >= HAL_L2_FID_LEARN_LIMIT_ENTRY_MAX(unit)), RT_ERR_OUT_OF_RANGE); 
    RT_PARAM_CHK((NULL == pNum), RT_ERR_NULL_POINTER); 

    baseAddr = ESW_FID0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0r + 4 * fid_macLimit_idx;

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, baseAddr, ESW_FIDCURMACNUMf, pNum)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pNum=%d", *pNum); 
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_fidLearningCnt_get*/  

/* Function Name:
 *      dal_esw_l2_fidLearningCnt_reset
 * Description:
 *      Reset number of learned MAC addresses on specified entry of fid MAC limit.
 * Input:
 *      unit             - unit id
 *      fid_macLimit_idx - index of FID MAC limit entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
int32
dal_esw_l2_fidLearningCnt_reset(uint32 unit, uint32 fid_macLimit_idx)
{
    int32   ret;
    uint32  baseAddr, val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, fid_macLimit_idx=%d", 
                unit, fid_macLimit_idx); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((fid_macLimit_idx >= HAL_L2_FID_LEARN_LIMIT_ENTRY_MAX(unit)), RT_ERR_OUT_OF_RANGE);  

    baseAddr = ESW_FID0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0r + 4 * fid_macLimit_idx;

    L2_SEM_LOCK(unit);

    val = ENABLED;
    if((ret = reg_field_write(unit, baseAddr + 1, ESW_FIDVCN_CLRf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    val = DISABLED;
    if((ret = reg_field_write(unit, baseAddr + 1, ESW_FIDVCN_CLRf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_fidLearningCnt_reset*/  

/* Function Name:
 *      dal_esw_l2_fidLastLearnedMac_get
 * Description:
 *      Get lastest learned MAC address on specifid fid.
 * Input:
 *      unit              - unit id
 *      fid_macLimit_idx  - index of FID MAC limit entry
 * Output:
 *      pFid              - pointer to filter id
 *      pMac              - lastest learned mac address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_l2_fidLastLearnedMac_get(
    uint32      unit,
    uint32      fid_macLimit_idx,
    rtk_fid_t   *pFid,
    rtk_mac_t   *pMac)
{
    int32   ret;
    uint32 baseAddr, val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, fid_macLimit_idx=%d", 
                unit, fid_macLimit_idx); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((fid_macLimit_idx >= HAL_L2_FID_LEARN_LIMIT_ENTRY_MAX(unit)), RT_ERR_OUT_OF_RANGE);  
    RT_PARAM_CHK((NULL == pFid) || (NULL == pMac), RT_ERR_INPUT);  

    baseAddr = ESW_FID0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0r + 4 * fid_macLimit_idx;

    L2_SEM_LOCK(unit);

    /*Check The entry ia valid*/
    if((ret = reg_field_read(unit, baseAddr + 1, ESW_FIDMACNUMCTLf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    if(val != ENABLED)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_L2), "The Entry is not valid");
        return RT_ERR_FAILED;
    }    

    if((ret = reg_read(unit, baseAddr + 2, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    
    pMac->octet[0] = (val >> 24) & 0xff;
    pMac->octet[1] = (val >> 16) & 0xff;
    pMac->octet[2] = (val >> 8) & 0xff;
    pMac->octet[3] = val & 0xff;
    
    if((ret = reg_field_read(unit, baseAddr + 3, ESW_FIDLASMAC_15_0f, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    pMac->octet[4] = (val >> 8) & 0xff;
    pMac->octet[5] = val & 0xff;

    /*Set The target fid*/
    if((ret = reg_field_read(unit, baseAddr + 1, ESW_FIDf, pFid)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }   

    L2_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pFid=%d, pMac=0x%02x:%02x:%02x:%02x:%02x:%02x", 
               *pFid, pMac->octet[0],pMac->octet[1],pMac->octet[2],pMac->octet[3],pMac->octet[4],
               pMac->octet[5]); 
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_fidLastLearnedMac_get*/    

/* Function Name:
 *      dal_esw_l2_limitLearningTrapPri_get
 * Description:
 *      Get priority of trapped packet when number mac address over limitation.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority of trapped packet
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
dal_esw_l2_limitLearningTrapPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d",  unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER); 

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_MAC_CONSTRIAN_TRAP_CONTROLr, 
                    ESW_PVMLPRIf, pPriority)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pPriority=%d",  *pPriority); 

    return RT_ERR_OK;
}   /*end of dal_esw_l2_limitLearningTrapPri_get*/

/* Function Name:
 *      dal_esw_l2_limitLearningTrapPri_set
 * Description:
 *      Set priority of trapped packet when number mac address over limitation.
 * Input:
 *      unit     - unit id
 *      priority - priority of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_l2_limitLearningTrapPri_set(uint32 unit, rtk_pri_t priority)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, priority=%d",
                    unit, priority); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_PRIORITY); 

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_MAC_CONSTRIAN_TRAP_CONTROLr, 
                    ESW_PVMLPRIf, &priority)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_limitLearningTrapPri_set*/

/* Function Name:
 *      dal_esw_l2_limitLearningTrapPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
dal_esw_l2_limitLearningTrapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);     

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_MAC_CONSTRIAN_TRAP_CONTROLr, 
                    ESW_DFRPVMLPRIf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
}   /*end of dal_esw_l2_limitLearningTrapPriEnable_get*/

/* Function Name:
 *      dal_esw_l2_limitLearningTrapPriEnable_set
 * Description:
 *      Set priority priority assignment status for trapped packet.
 * Input:
 *      unit     - unit id
 *      enable - priority assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
 int32
dal_esw_l2_limitLearningTrapPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);      

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_MAC_CONSTRIAN_TRAP_CONTROLr, 
                    ESW_DFRPVMLPRIf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_limitLearningTrapPriEnable_set*/

/* Function Name:
 *      dal_esw_l2_limitLearningTrapDP_get
 * Description:
 *      Get drop precedence of trapped packet when number mac address over limitation.
 * Input:
 *      unit - unit id
 * Output:
 *      pDp  - drop precedence of trapped packet
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
dal_esw_l2_limitLearningTrapDP_get(uint32 unit, uint32 *pDp)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d",  unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pDp), RT_ERR_NULL_POINTER); 

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_MAC_CONSTRIAN_TRAP_CONTROLr, 
                    ESW_PVMLDPf, pDp)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pDp=%d",  *pDp); 

    return RT_ERR_OK;
}   /*end of dal_esw_l2_limitLearningTrapDP_get*/

/* Function Name:
 *      dal_esw_l2_limitLearningTrapDP_set
 * Description:
 *      Set drop precedence of trapped packet when number mac address over limitation.
 * Input:
 *      unit - unit id
 *      dp   - drop precedence of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 * Note:
 *      None
 */
int32
dal_esw_l2_limitLearningTrapDP_set(uint32 unit, uint32 dp)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, priority=%d",
                    unit, dp); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((dp > 3), RT_ERR_PRIORITY); 

    L2_SEM_LOCK(unit);

    /*Set lookup miss dp value*/
    if((ret = reg_field_write(unit, ESW_MAC_CONSTRIAN_TRAP_CONTROLr, 
                    ESW_PVMLDPf, &dp)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_limitLearningTrapDP_set*/

/* Function Name:
 *      dal_esw_l2_limitLearningTrapDPEnable_get
 * Description:
 *      Get drop procedence assignment status for trapped packet.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to drop procedence assignment status for trapped packet
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
dal_esw_l2_limitLearningTrapDPEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);     

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_MAC_CONSTRIAN_TRAP_CONTROLr, 
                    ESW_DFRPVMLDPf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
}   /*end of dal_esw_l2_limitLearningTrapDPEnable_get*/

/* Function Name:
 *      dal_esw_l2_limitLearningTrapDPEnable_set
 * Description:
 *      Set drop procedence drop procedence assignment status for trapped packet.
 * Input:
 *      unit     - unit id
 *      enable - drop procedence assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid drop procedence value
 * Note:
 *      None
 */
 int32
dal_esw_l2_limitLearningTrapDPEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);      

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_MAC_CONSTRIAN_TRAP_CONTROLr, 
                    ESW_DFRPVMLDPf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_limitLearningTrapDPEnable_set*/


/* Function Name:
 *      dal_esw_l2_limitLearningTrapAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
dal_esw_l2_limitLearningTrapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d",  unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);   

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_MAC_CONSTRIAN_TRAP_CONTROLr, 
                    ESW_CPUTAGf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_limitLearningTrapAddCPUTagEnable_get*/

/* Function Name:
 *      dal_esw_l2_limitLearningTrapAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
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
dal_esw_l2_limitLearningTrapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);   

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_MAC_CONSTRIAN_TRAP_CONTROLr, 
                    ESW_CPUTAGf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_limitLearningTrapAddCPUTagEnable_set*/

/* Function Name:
 *      dal_esw_l2_aging_get
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
dal_esw_l2_aging_get(uint32 unit, uint32 *pAging_time)
{
     int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pAging_time), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, ESW_AGEUNITf, pAging_time)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /* 
     * translate to user time: Val(user) = 0.1024 * 6 * Val(chip)
     * Val(user) = (0.1024 *6)*Val(user) = (0.6144)*Val(user)
     *           = (6144/10000)*Val(user) = (384/625)*Val(user)
     */
#if defined(CONFIG_SDK_WA_LIMIT_LEARN_COUNT)
    {
        hal_control_t *pHal_ctrl;

        if ((pHal_ctrl = hal_ctrlInfo_get(unit)) == NULL)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return RT_ERR_FAILED;
        }

        if ((RTL8328M_CHIP_ID == pHal_ctrl->chip_id || RTL8328S_CHIP_ID == pHal_ctrl->chip_id) &&
            (CHIP_REV_ID_A == pHal_ctrl->chip_rev_id))
        {
            *pAging_time = (uint32)(((*pAging_time)*384+624)/625)/2;
        }
        else
        {
            *pAging_time = (uint32)((*pAging_time)*384+624)/625;
        }
    }
#else
    *pAging_time = (uint32)((*pAging_time)*384+624)/625;
#endif

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAging_time=%d", *pAging_time); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_aging_get*/

/* Function Name:
 *      dal_esw_l2_aging_set
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
dal_esw_l2_aging_set(uint32 unit, uint32 aging_time)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, aging_time=%d", unit, aging_time); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    L2_SEM_LOCK(unit);

    /* 
     * translate to asic time: Val(user) = 0.1024 * 6 * Val(chip)
     * Val(chip) = Val(user)/(0.1024 *6) = Val(user)/(0.6144)
     *           = Val(user)*10000/6144 = Val(user)*625/384
     */
#if defined(CONFIG_SDK_WA_LIMIT_LEARN_COUNT)
    {
        hal_control_t *pHal_ctrl;

        if ((pHal_ctrl = hal_ctrlInfo_get(unit)) == NULL)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return RT_ERR_FAILED;
        }

        if ((RTL8328M_CHIP_ID == pHal_ctrl->chip_id || RTL8328S_CHIP_ID == pHal_ctrl->chip_id) &&
            (CHIP_REV_ID_A == pHal_ctrl->chip_rev_id))
        {
            aging_time = (uint32)((aging_time*625)/384)*2;
        }
        else
        {
            aging_time = (uint32)(aging_time*625)/384;
        }
    }
#else
    aging_time = (uint32)(aging_time*625)/384;
#endif

    /*Set Ageunit*/
    if((ret = reg_field_write(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, ESW_AGEUNITf, &aging_time)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /*Trigger Ageunit change*/
    val = ENABLED;
    if((ret = reg_field_write(unit,ESW_SOURCE_MAC_LEARNING_CONTROLr, ESW_CHAGEUNITf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/*end of dal_esw_l2_aging_set*/


/* Function Name:
 *      dal_esw_l2_camEnable_get
 * Description:
 *      Get enable status of CAM entry.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CAM entry
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
dal_esw_l2_camEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_LUTCAMENf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 
    return RT_ERR_OK;
}/*end of dal_esw_l2_camEnable_get*/


/* Function Name:
 *      dal_esw_l2_camEnable_set
 * Description:
 *      Set enable status of CAM entry.
 * Input:
 *      unit   - unit id
 *      enable - enable status of CAM entry
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
dal_esw_l2_camEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);    

    L2_SEM_LOCK(unit);
    
    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_LUTCAMENf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/*end of dal_esw_l2_camEnable_set*/

/* Function Name:
 *      dal_esw_l2_hashAlgo_get
 * Description:
 *      Get hash algorithm of layer2 switching.
 * Input:
 *      unit       - unit id
 * Output:
 *      pHash_algo - pointer to hash algorithm of layer2 switching
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
dal_esw_l2_hashAlgo_get(uint32 unit, uint32 *pHash_algo)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pHash_algo), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_L2_HASH_ALGOf, pHash_algo)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pHash_algo=%d", *pHash_algo); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_hashAlgo_get*/


/* Function Name:
 *      dal_esw_l2_hashAlgo_set
 * Description:
 *      Set hash algorithm of layer2 switching.
 * Input:
 *      unit            - unit id
 *      hash_algo       - hash algorithm of layer2 switching
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      1. clear the address table before configing the hash algorithm
 *
 */
int32
dal_esw_l2_hashAlgo_set(uint32 unit, uint32 hash_algo)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, hash_algo=%d", unit, hash_algo); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((hash_algo >= ESW_L2_HASH_ALGO_END), RT_ERR_INPUT);    

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_L2_HASH_ALGOf, &hash_algo)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_hashAlgo_get*/


/* Function Name:
 *      dal_esw_l2_vlanMode_get
 * Description:
 *      Get vlan(inner/outer vlan) for L2 lookup on specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 * Output:
 *      pVlanMode       - pointer to inner/outer vlan
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      - BASED_ON_INNER_VLAN
 *      - BASED_ON_OUTER_VLAN
 */
int32
dal_esw_l2_vlanMode_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pVlanMode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((NULL == pVlanMode), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_read(unit, ESW_PORT_FORWARDING_TAG_SELECT_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_FWDBASE_IVID_OVIDf, pVlanMode)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pVlanMode=%d", *pVlanMode); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_vlanMode_get*/


/* Function Name:
 *      dal_esw_l2_vlanMode_set
 * Description:
 *      Set vlan(inner/outer vlan) for L2 lookup on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      vlanMode - inner/outer vlan
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
 *      Vlan mode is as following
 *      - BASED_ON_INNER_VLAN
 *      - BASED_ON_OUTER_VLAN
 */
int32
dal_esw_l2_vlanMode_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t vlanMode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vlanMode=%d", unit, vlanMode); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((vlanMode >= FWD_VLAN_MODE_END), RT_ERR_INPUT);    

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_write(unit, ESW_PORT_FORWARDING_TAG_SELECT_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_FWDBASE_IVID_OVIDf, &vlanMode)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/*end of dal_esw_l2_vlanMode_set*/

/* Module Name    : L2      */
/* Sub-module Name: Unicast */

/* Function Name:
 *      dal_esw_l2_learningEnable_get
 * Description:
 *      Get enable status of address learning on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of address learning
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
dal_esw_l2_learningEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_read(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r, 
                    port, REG_ARRAY_INDEX_NONE, ESW_SML_ENf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_learningEnable_get*/


/* Function Name:
 *      dal_esw_l2_learningEnable_set
 * Description:
 *      Set enable status of address learning on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of address learning
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
dal_esw_l2_learningEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);    

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_write(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r, 
                    port, REG_ARRAY_INDEX_NONE, ESW_SML_ENf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_learningEnable_set*/

/* Function Name:
 *      dal_esw_l2_newMacOp_get
 * Description:
 *      Get learning mode and forwarding action of new learned address on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pLrnMode   - pointer to learning mode
 *      pFwdAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 *
 *      Learning mode is as following
 *      - HARDWARE_LEARNING
 *      - SOFTWARE_LEARNING
 *      - NOT_LEARNING
 */
int32
dal_esw_l2_newMacOp_get(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_l2_newMacLrnMode_t  *pLrnMode, 
    rtk_action_t            *pFwdAction)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((NULL == pLrnMode) || (NULL == pFwdAction), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    /*get new mac action*/
    if((ret = reg_array_field_read(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r, 
                   port, REG_ARRAY_INDEX_NONE, ESW_SML_NEW_OPf, pLrnMode)) != RT_ERR_OK)
   {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /*get new mac pkt action*/
    if((ret = reg_array_field_read(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r,
                    port, REG_ARRAY_INDEX_NONE, ESW_SML_NEW_PKT_OPf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /*translate real new mac pkt action to rtk_action*/
    if(val == 0)
        *pFwdAction = ACTION_FORWARD;
    else if(val == 1)
        *pFwdAction = ACTION_COPY2CPU;
    else if(val == 2)
        *pFwdAction = ACTION_TRAP2CPU;
    else
        *pFwdAction = ACTION_DROP;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pLrnMode=%d, pFwdAction=%d", *pLrnMode, *pFwdAction); 
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_newMacOp_get*/

/* Function Name:
 *      dal_esw_l2_newMacOp_set
 * Description:
 *      Set learning mode and forwarding action of new learned address on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      lrnMode   - learning mode
 *      fwdAction - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 *
 *      Learning mode is as following
 *      - HARDWARE_LEARNING
 *      - SOFTWARE_LEARNING
 *      - NOT_LEARNING
 */
int32
dal_esw_l2_newMacOp_set(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_l2_newMacLrnMode_t  lrnMode, 
    rtk_action_t            fwdAction)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, lrnMode=%d, fwdAction=%d", 
                unit, lrnMode, fwdAction); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);  
    RT_PARAM_CHK((lrnMode >= LEARNING_MODE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((fwdAction >= ACTION_TO_GUESTVLAN), RT_ERR_INPUT);    

    L2_SEM_LOCK(unit);

    /*set new mac action*/
    if((ret = reg_array_field_write(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r,
                    port, REG_ARRAY_INDEX_NONE, ESW_SML_NEW_OPf, &lrnMode)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /*translate rtk_action to real new mac pkt action*/
    if(fwdAction == ACTION_FORWARD)
        val =0;
    else if(fwdAction == ACTION_COPY2CPU)
        val =1;
    else if(fwdAction == ACTION_TRAP2CPU)
        val =2;
    else
        val =3;

    /*set new mac pkt action*/
    if((ret = reg_array_field_write(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r,
                    port, REG_ARRAY_INDEX_NONE, ESW_SML_NEW_PKT_OPf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }   

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_newMacOp_set*/

/* Function Name:
 *      dal_esw_l2_LRUEnable_get
 * Description:
 *      Get enable status of least recent used address replace.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of least recent used address replace
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
dal_esw_l2_LRUEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, 
                    ESW_L2LRUf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_LRUEnable_get*/


/* Function Name:
 *      dal_esw_l2_LRUEnable_set
 * Description:
 *      Set enable status of least recent used address replace.
 * Input:
 *      unit   - unit id
 *      enable - enable status of least recent used address replace
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
dal_esw_l2_LRUEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, 
                    ESW_L2LRUf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/*end of dal_esw_l2_LRUEnable_set*/

/* Function Name:
 *      dal_esw_l2_ucastLookupMode_get
 * Description:
 *      Get lookup mode for unicast address.
 * Input:
 *      unit              - unit id
 * Output:
 *      pUcast_lookupMode - pointer to lookup mode for unicast address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Lookup mode for multicast address is as following
 *      - UC_LOOKUP_ON_VID
 *      - UC_LOOKUP_ON_FID
 */
int32
dal_esw_l2_ucastLookupMode_get(uint32 unit, rtk_l2_ucastLookupMode_t *pUcast_lookupMode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pUcast_lookupMode), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_BASED_ON_VID_OR_FID_DECISION_CONTROLr, 
                    ESW_ULFIDf, pUcast_lookupMode)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pUcast_lookupMode=%d", *pUcast_lookupMode); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_ucastLookupMode_get*/


/* Function Name:
 *      dal_esw_l2_ucastLookupMode_set
 * Description:
 *      Set lookup mode for unicast address.
 * Input:
 *      unit             - unit id
 *      ucast_lookupMode - lookup mode for unicast address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Lookup mode for multicast address is as following
 *      - UC_LOOKUP_ON_VID
 *      - UC_LOOKUP_ON_FID
 */
 int32
dal_esw_l2_ucastLookupMode_set(uint32 unit, rtk_l2_ucastLookupMode_t ucast_lookupMode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, ucast_lookupMode=%d", unit, ucast_lookupMode); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((ucast_lookupMode >= UC_LOOKUP_END), RT_ERR_INPUT);    

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_BASED_ON_VID_OR_FID_DECISION_CONTROLr, 
                    ESW_ULFIDf, &ucast_lookupMode)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/*end of dal_esw_l2_ucastLookupMode_set*/


/* Module Name    : L2              */
/* Sub-module Name: Unicast address */

/* Function Name:
 *      dal_esw_l2_addr_init
 * Description:
 *      Initialize content of buffer of L2 entry.
 *      Will fill vid ,MAC address and reset other field of L2 entry.
 * Input:
 *      unit     - unit id
 *      vid      - vlan id
 *      pMac     - MAC address
 *      pL2_addr - L2 entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize L2 entry before add it.
 */
int32
dal_esw_l2_addr_init(
    uint32              unit,
    rtk_vlan_t          vid,
    rtk_mac_t           *pMac,
    rtk_l2_ucastAddr_t  *pL2_addr)
{ 
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pL2_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pMac, RT_ERR_NULL_POINTER);    
    RT_PARAM_CHK((pMac->octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK((vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    osal_memset(pL2_addr, 0, sizeof(rtk_l2_ucastAddr_t));
    pL2_addr->vid = vid;
    osal_memcpy(&pL2_addr->mac.octet[0], &pMac->octet[0], sizeof(rtk_mac_t));

    return RT_ERR_OK;    
}/*end of dal_esw_l2_addr_init*/

/* Function Name:
 *      dal_esw_l2_addr_add
 * Description:
 *      Add L2 entry to ASIC.
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
 *      RT_ERR_L2_NO_EMPTY_ENTRY - no empty entry in L2 table
 * Note:
 *      (1) Need to initialize L2 entry before add it.
 *      (2) The API can supported add by port or trunk-id view both.
 *          - If pL2_addr->flags have turn on the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the 
 *            pL2_addr->trk_gid is valid and pL2_addr->port is invalid.
 *          - If pL2_addr->flags have turn off the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the 
 *            pL2_addr->port is valid and pL2_addr->trk_gid is invalid.
 */
int32
dal_esw_l2_addr_add(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr)
{
    int32               ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;
    rtk_trunk_mode_t mode;
    rtk_port_t resperentPort;   

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK((NULL == pL2_addr), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pL2_addr->mac.octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK((pL2_addr->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d, \
           port=%d, flags=0x%x, state=0x%x, auth=%d"
           , unit, pL2_addr->vid, pL2_addr->port, pL2_addr->flags, pL2_addr->state, pL2_addr->auth); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "mac=%x-%x-%x-%x-%x-%x",
           pL2_addr->mac.octet[0], pL2_addr->mac.octet[1], pL2_addr->mac.octet[2],
           pL2_addr->mac.octet[3], pL2_addr->mac.octet[4], pL2_addr->mac.octet[5]); 

    if(!(pL2_addr->flags & RTK_L2_UCAST_FLAG_DA_BLOCK))
    {
        if((ret = dal_esw_trunk_mode_get(unit, &mode)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "Get trunk mode failed");  
            return ret;
        }

        if (!(pL2_addr->flags & RTK_L2_UCAST_FLAG_TRUNK_PORT))
        {
            for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
            {
                if (dal_esw_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
                {
                    if ((first_trunkMember = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
                    {
                        /* no trunk member */
                        continue;
                    }
    
                    if(RTK_PORTMASK_IS_PORT_SET(trunk_portmask, pL2_addr->port))
                    {
                        /*dumbmode*/
                        if(TRUNK_MODE_DUMB == mode)
                        {
                            if ( !(first_trunkMember == pL2_addr->port))
                            {
                                /* this port is trunk member and not first trunk member, not allow to add */
                                RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "The target port number is not represent port");
                                return RT_ERR_PORT_ID;
                            }
                        }
                        else    /*Normal mode*/
                        {
                            if(dal_esw_trunk_representPort_get(unit, trk_gid, &resperentPort) != RT_ERR_OK)
                            {                    
                                RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "Get represent port number");
                                return RT_ERR_PORT_ID;
                            }
    
                            if ((resperentPort  != pL2_addr->port) )
                            {
                                /* this port is trunk member and not first trunk member, not allow to add */
                                RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "The target port number is not represent port");
                                return RT_ERR_PORT_ID;
                            }
                        }
                    }
                     
                }
            }
        }
        else
        {
            RT_PARAM_CHK(pL2_addr->trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
            /* Transfer pL2_addr->trk_gid to represent port and update pL2_addr->port */
            if (TRUNK_MODE_DUMB == mode)
            {
                if ((ret = dal_esw_trunk_port_get(unit, pL2_addr->trk_gid, &trunk_portmask)) != RT_ERR_OK)
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
            else
            {
                if ((ret = dal_esw_trunk_representPort_get(unit, pL2_addr->trk_gid, &resperentPort)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                pL2_addr->port = resperentPort;
            }
        }
    }   
    
    /* search exist or free entry */    
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid = pL2_addr->vid;
    osal_memcpy(&l2_entry.unicast.mac.octet[0], &pL2_addr->mac.octet[0], sizeof(rtk_mac_t));
  
    L2_SEM_LOCK(unit);

    ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry);
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
            ret = _dal_esw_l2_getFirstDynamicEntry(unit, &l2_entry, &index_entry);
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
    l2_entry.unicast.port       = pL2_addr->port;
    l2_entry.unicast.aging      = 7;
    l2_entry.unicast.sablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_SA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.dablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_DA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.auth       = pL2_addr->auth;
    l2_entry.unicast.is_static  = (pL2_addr->flags & RTK_L2_UCAST_FLAG_STATIC) ? TRUE: FALSE;
    l2_entry.unicast.nh  = (pL2_addr->flags & RTK_L2_UCAST_FLAG_NEXTHOP) ? TRUE: FALSE;
    l2_entry.unicast.suspend  = (pL2_addr->state & RTK_L2_UCAST_STATE_SUSPEND) ? TRUE: FALSE;
    l2_entry.valid = TRUE; 
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in HASH */
        ret = _dal_esw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_esw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    } 
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_esw_l2_addr_add */

/* Function Name:
 *      dal_esw_l2_addr_del
 * Description:
 *      Delete a L2 unicast address entry from the specified device.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 *      pMac  - mac address
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
dal_esw_l2_addr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;

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
    osal_memcpy(&l2_entry.unicast.mac.octet[0], &pMac->octet[0], sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);

    if ((ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    /*Invalid the entry */
    l2_entry.valid = FALSE;    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in HASH */
        ret = _dal_esw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_esw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    } 
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_esw_l2_addr_del */

/* Function Name:
 *      dal_esw_l2_addr_get
 * Description:
 *      Get L2 entry based on specified vid and MAC address
 * Input:
 *      unit     - unit id
 * Output:
 *      pL2_addr - pointer to L2 entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      (1) If pL2_addr->flags have turn on the RTK_L2_UCAST_FLAG_TRUNK_PORT flag,
 *          mean the pL2_addr->trk_gid is valid and pL2_addr->port is valid also.
 *          The pL2_addr->port value is the represent port of pL2_addr->trk_gid.
 *      (2) If pL2_addr->flags have turn off the RTK_L2_UCAST_FLAG_TRUNK_PORT flag,
 *          mean the pL2_addr->port is valid and pL2_addr->trk_gid is invalid.
 */
int32
dal_esw_l2_addr_get(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr)
{
    int32 ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;
    rtk_trunk_mode_t mode;
    rtk_port_t resperentPort;   

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d", 
           unit, pL2_addr->vid);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pL2_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pL2_addr->mac.octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK((pL2_addr->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x", 
           pL2_addr->mac.octet[0], pL2_addr->mac.octet[1], pL2_addr->mac.octet[2],
           pL2_addr->mac.octet[3], pL2_addr->mac.octet[4], pL2_addr->mac.octet[5]);

    /*Clear pL2_addr Info*/
    pL2_addr->flags = 0;
    pL2_addr->state = 0;

    /* search exist or free entry */    
    l2_entry.entry_type     = L2_UNICAST;
    l2_entry.unicast.fid    = pL2_addr->vid;
    osal_memcpy(&l2_entry.unicast.mac.octet[0], &pL2_addr->mac.octet[0], sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);
    
    if ((ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    
    /* fill content */
    osal_memcpy(&pL2_addr->mac.octet[0], &l2_entry.unicast.mac.octet[0], sizeof(rtk_mac_t));
    pL2_addr->vid       = l2_entry.unicast.fid;
    pL2_addr->port      = l2_entry.unicast.port;
    if (index_entry.index_type == L2_IN_HASH)
        pL2_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pL2_addr->l2_idx = (1 << 14) + index_entry.index;
    if((ret = dal_esw_trunk_mode_get(unit, &mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "Get trunk mode failed");  
        return ret;
    }

    for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
    {
        if (dal_esw_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
        {
            if ((first_trunkMember = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
            {
                /* no trunk member */
                continue;
            }

            /*dumbmode*/
            if(mode == TRUNK_MODE_DUMB)
            {
                if ((RTK_PORTMASK_IS_PORT_SET(trunk_portmask, pL2_addr->port)) &&
                    (first_trunkMember == pL2_addr->port))
                {
                    pL2_addr->trk_gid = trk_gid;
                    pL2_addr->flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
                    break;
                }
            }
            else    /*Normal mode*/
            {
                if(dal_esw_trunk_representPort_get(unit, trk_gid, &resperentPort) != RT_ERR_OK)
                {                    
                    RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "Get represent port number");
                    return RT_ERR_PORT_ID;
                }

                if ((resperentPort == pL2_addr->port))
                {
                    pL2_addr->trk_gid = trk_gid;
                    pL2_addr->flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
                    break;
                }
            } 
        }
    }
    if(l2_entry.unicast.sablock)
        pL2_addr->flags |= RTK_L2_UCAST_FLAG_SA_BLOCK;
    if(l2_entry.unicast.dablock)
    {
        pL2_addr->flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
        pL2_addr->port = 31;
    }
    pL2_addr->auth      = l2_entry.unicast.auth;
    if(l2_entry.unicast.is_static)
        pL2_addr->flags |= RTK_L2_UCAST_FLAG_STATIC;
    if(l2_entry.unicast.nh)
        pL2_addr->flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
    if(l2_entry.unicast.suspend)
        pL2_addr->state |= RTK_L2_UCAST_STATE_SUSPEND;
    if(l2_entry.unicast.aging == 0)
        pL2_addr->isAged = TRUE;
    else
        pL2_addr->isAged = FALSE;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "vid=%d, port=%d, flags=0x%x, auth=%d\
           state=0x%x", pL2_addr->vid, pL2_addr->port, pL2_addr->flags, pL2_addr->auth, pL2_addr->state);
    
    return RT_ERR_OK;
} /* end of dal_esw_l2_addr_get */

/* Function Name:
 *      dal_esw_l2_addr_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      (1) The API can supported add by port or trunk-id view both.
 *          - If pL2_addr->flags have turn on the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the 
 *            pL2_addr->trk_gid is valid and pL2_addr->port is invalid.
 *          - If pL2_addr->flags have turn off the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the 
 *            pL2_addr->port is valid and pL2_addr->trk_gid is invalid.
 */
int32
dal_esw_l2_addr_set(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr)
{
    int32               ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;
    rtk_trunk_mode_t mode;
    uint32 resperentPort;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d, \
           port=%d, flags=0x%x, state=0x%x, auth=%d"
           , unit, pL2_addr->vid, pL2_addr->port, pL2_addr->flags, pL2_addr->state, pL2_addr->auth); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pL2_addr, RT_ERR_NULL_POINTER);
    //RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, pL2_addr->port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pL2_addr->mac.octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK((pL2_addr->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "mac=%x-%x-%x-%x-%x-%x",
           pL2_addr->mac.octet[0], pL2_addr->mac.octet[1], pL2_addr->mac.octet[2],
           pL2_addr->mac.octet[3], pL2_addr->mac.octet[4], pL2_addr->mac.octet[5]); 

    if(!(pL2_addr->flags & RTK_L2_UCAST_FLAG_DA_BLOCK))
    {
        if((ret = dal_esw_trunk_mode_get(unit, &mode)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "Get trunk mode failed");  
            return ret;
        }
            
        if (!(pL2_addr->flags & RTK_L2_UCAST_FLAG_TRUNK_PORT))
        {
            for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
            {
                if (dal_esw_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
                {
                    if ((first_trunkMember = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
                    {
                        /* no trunk member */
                        continue;
                    }
    
                    /*dumbmode*/
                    if(mode == TRUNK_MODE_DUMB)
                    {
                        if ((RTK_PORTMASK_IS_PORT_SET(trunk_portmask, pL2_addr->port)) &&
                        !(first_trunkMember == pL2_addr->port))
                        {
                            /* this port is trunk member and not first trunk member, not allow to add */
                            RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "The target port number is not represent port");
                            return RT_ERR_PORT_ID;
                        }
                    }
                    else    /*Normal mode*/
                    {
                        if(dal_esw_trunk_representPort_get(unit, trk_gid, &resperentPort) != RT_ERR_OK)
                        {                    
                            RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "Get represent port number");
                            return RT_ERR_PORT_ID;
                        }
    
                        if ((resperentPort != pL2_addr->port))
                        {
                            /* this port is trunk member and not first trunk member, not allow to add */
                            RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "The target port number is not represent port");
                            return RT_ERR_PORT_ID;
                        }
                    } 
                }
            }
        }
        else
        {
            RT_PARAM_CHK(pL2_addr->trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
            /* Transfer pL2_addr->trk_gid to represent port and update pL2_addr->port */
            if (TRUNK_MODE_DUMB == mode)
            {
                if ((ret = dal_esw_trunk_port_get(unit, pL2_addr->trk_gid, &trunk_portmask)) != RT_ERR_OK)
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
            else
            {
                if ((ret = dal_esw_trunk_representPort_get(unit, pL2_addr->trk_gid, &resperentPort)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                pL2_addr->port = resperentPort;
            }
        }
    }   
    
    /* search exist or free entry */    
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid = pL2_addr->vid;
    osal_memcpy(&l2_entry.unicast.mac.octet[0], &pL2_addr->mac.octet[0], sizeof(rtk_mac_t));
    
    L2_SEM_LOCK(unit);

    if ((ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
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
    l2_entry.unicast.port       = pL2_addr->port;
    l2_entry.unicast.aging      = 7;
    l2_entry.unicast.sablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_SA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.dablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_DA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.auth       = pL2_addr->auth;
    l2_entry.unicast.is_static  = (pL2_addr->flags & RTK_L2_UCAST_FLAG_STATIC) ? TRUE: FALSE;
    l2_entry.unicast.nh  = (pL2_addr->flags & RTK_L2_UCAST_FLAG_NEXTHOP) ? TRUE: FALSE;
    l2_entry.unicast.suspend  = (pL2_addr->state & RTK_L2_UCAST_STATE_SUSPEND) ? TRUE: FALSE;
    l2_entry.valid = TRUE; 

    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in HASH */
        ret = _dal_esw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_esw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    } 
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_esw_l2_addr_set */

/* Function Name:
 *      dal_esw_l2_addr_delAll
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
dal_esw_l2_addr_delAll(uint32 unit, uint32 include_static)
{
    int32       ret;
    uint32      busyloop;
    uint32      value;
    rtk_l2_flushCfg_t config;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, include_static=%d", 
           unit, include_static);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    osal_memset(&config, 0, sizeof(rtk_l2_flushCfg_t));
    config.flushAddrOnAllPorts = TRUE;
    config.flushStaticAddr = include_static;
    if (( ret = dal_esw_l2_ucastAddr_flush(unit, &config)) != RT_ERR_OK)
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
        if ((reg_field_read(unit, ESW_ADDRESS_TABLE_DELETE_CONTROL0r, ESW_REMOVEACT_TRAGf, &value)) == RT_ERR_OK)
        {
            if (0 == value)
            {
                break;
            }
        }
    }
        
    return RT_ERR_OK;
} /* end of dal_esw_l2_addr_delAll */


/* Function Name:
 *      dal_esw_l2_nextValidAddr_get
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
 *      3. The pScan_idx is the input and also is the output argument.
 */
int32
dal_esw_l2_nextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    uint32              include_static,
    rtk_l2_ucastAddr_t  *pL2_data)
{
    int32  ret;
    dal_esw_l2_entry_t  l2_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;
    rtk_trunk_mode_t mode;
    rtk_port_t resperentPort;   

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK((NULL == pScan_idx), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pL2_data), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((include_static > 1), RT_ERR_NULL_POINTER);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, include_static=%d", 
           unit, *pScan_idx, include_static);

    osal_memset(pL2_data, 0, sizeof(rtk_l2_ucastAddr_t));
    
    L2_SEM_LOCK(unit);
    
    if ((ret = _dal_esw_l2_nextValidAddr_get(unit, pScan_idx, L2_UNICAST, include_static, &l2_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    
    osal_memcpy(&pL2_data->mac.octet[0], &l2_entry.unicast.mac.octet[0], sizeof(rtk_mac_t));
    pL2_data->vid       = l2_entry.unicast.fid;
    pL2_data->port      = l2_entry.unicast.port;

    pL2_data->l2_idx    = *pScan_idx;

    if((ret = dal_esw_trunk_mode_get(unit, &mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "Get trunk mode failed");  
        return ret;
    }

    for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
    {
        if (dal_esw_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
        {
            if ((first_trunkMember = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
            {
                /* no trunk member */
                continue;
            }

            /*dumbmode*/
            if(mode == TRUNK_MODE_DUMB)
            {
                if ((RTK_PORTMASK_IS_PORT_SET(trunk_portmask, pL2_data->port)) &&
                    (first_trunkMember == pL2_data->port))
                {
                    pL2_data->trk_gid = trk_gid;
                    pL2_data->flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
                    break;
                    /* this port is trunk member and not first trunk member, not allow to add */
                    //RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "The target port number is not represent port");
                    //return RT_ERR_PORT_ID;
                }
            }
            else    /*Normal mode*/
            {
                if(dal_esw_trunk_representPort_get(unit, trk_gid, &resperentPort) != RT_ERR_OK)
                {                    
                    RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "Get represent port number");
                    return RT_ERR_PORT_ID;
                }

                if ((resperentPort == pL2_data->port))
                {
                    pL2_data->trk_gid = trk_gid;
                    pL2_data->flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
                    break;
                    /* this port is trunk member and not first trunk member, not allow to add */
                    //RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "The target port number is not represent port");
                    //return RT_ERR_PORT_ID;
                }
            } 
        }
    }

    if(l2_entry.unicast.sablock)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_SA_BLOCK;
    if(l2_entry.unicast.dablock)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
    pL2_data->auth      = l2_entry.unicast.auth;
    if(l2_entry.unicast.is_static)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_STATIC;
    if(l2_entry.unicast.nh)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
    if(l2_entry.unicast.suspend)
        pL2_data->state |= RTK_L2_UCAST_STATE_SUSPEND;
    if(l2_entry.unicast.aging == 0)
        pL2_data->isAged = TRUE;
    else
        pL2_data->isAged = FALSE;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "vid=%d, port=%d, flags=0x%x, auth=%d\
           state=0x%x", pL2_data->vid, pL2_data->port, pL2_data->flags, pL2_data->auth, pL2_data->state);
    
    return RT_ERR_OK;
} /* end of dal_esw_l2_nextValidAddr_get */    


/* Function Name:
 *      dal_esw_l2_nextValidMcastAddr_get
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
 *      3. The pScan_idx is the input and also is the output argument.
 */
int32
dal_esw_l2_nextValidMcastAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    rtk_l2_mcastAddr_t  *pL2_data)
{
    int32  ret;
    dal_esw_l2_entry_t  l2_entry;
    multicast_index_entry_t mcast_entry;
    uint32 value;
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK((NULL == pScan_idx), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pL2_data), RT_ERR_NULL_POINTER);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d", 
           unit, *pScan_idx);

    osal_memset(pL2_data, 0, sizeof(rtk_l2_mcastAddr_t));
    
    L2_SEM_LOCK(unit);
    if ((ret = _dal_esw_l2_nextValidAddr_get(unit, pScan_idx, L2_MULTICAST, TRUE, &l2_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    pL2_data->rvid = l2_entry.l2mcast.rvid;
    osal_memcpy(&pL2_data->mac, &l2_entry.l2mcast.mac, sizeof(rtk_mac_t));
    pL2_data->fwdIndex = l2_entry.l2mcast.index;
    pL2_data->l2_idx   = *pScan_idx;

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    if ((ret = table_read(unit, ESW_FORWARDINGt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, pL2_data->portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &pL2_data->crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_AGINGf, &value, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    if(value)
        pL2_data->isAged = TRUE;
    else
        pL2_data->isAged = FALSE;
    
    L2_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, rvid=%d, mac=%x-%x-%x-%x-%x-%x, portmask=0x%x, crsvlan=%d, aged=%d", 
           unit, *pScan_idx, pL2_data->rvid, pL2_data->mac.octet[0], pL2_data->mac.octet[1], pL2_data->mac.octet[2], pL2_data->mac.octet[3], 
           pL2_data->mac.octet[4], pL2_data->mac.octet[5], pL2_data->portmask.bits[0],pL2_data->crossVlan, pL2_data->isAged);

    return RT_ERR_OK;
} /* end of dal_esw_l2_nextValidMcastAddr_get*/    

/* Function Name:
 *      dal_esw_l2_nextValidIpMcastAddr_get
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
 *      3. The pScan_idx is the input and also is the output argument.
 */
int32
dal_esw_l2_nextValidIpMcastAddr_get(
    uint32                  unit,
    int32                   *pScan_idx,
    rtk_l2_ipMcastAddr_t    *pL2_data)
{
    int32  ret;
    dal_esw_l2_entry_t      l2_entry;
    multicast_index_entry_t mcast_entry;
    uint32 value;
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK((NULL == pScan_idx), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pL2_data), RT_ERR_NULL_POINTER);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d", 
           unit, *pScan_idx);

    osal_memset(pL2_data, 0, sizeof(rtk_l2_ipMcastAddr_t));
    
    L2_SEM_LOCK(unit);
    
    if ((ret = _dal_esw_l2_nextValidAddr_get(unit, pScan_idx, IP_MULTICAST, TRUE, &l2_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    pL2_data->dip  = l2_entry.ipmcast.dip;
    pL2_data->sip  = l2_entry.ipmcast.sip;
    pL2_data->rvid = 0; /*Not used by the chip*/
    pL2_data->fwdIndex = l2_entry.ipmcast.index;
    pL2_data->l2_idx = *pScan_idx;

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    if ((ret = table_read(unit, ESW_FORWARDINGt, l2_entry.ipmcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, pL2_data->portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &pL2_data->crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_AGINGf, &value, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    if(value)
        pL2_data->isAged = TRUE;
    else
        pL2_data->isAged = FALSE;
    
    L2_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, rvid=%d, dip=%x, sip=%x, portmask=0x%x, crsvlan=%d, aged=%d", 
           unit, *pScan_idx, pL2_data->rvid, pL2_data->dip, pL2_data->sip, pL2_data->portmask.bits[0],pL2_data->crossVlan, pL2_data->isAged);

    return RT_ERR_OK;
} /* end of dal_esw_l2_nextValidIpMcastAddr_get */    

/* Module Name    : L2           */
/* Sub-module Name: l2 multicast */

/* Function Name:
 *      dal_esw_l2_mcastLookupMode_get
 * Description:
 *      Get lookup mode for multicast address.
 * Input:
 *      unit              - unit id
 * Output:
 *      pMcast_lookupMode - pointer to lookup mode for multicast address
 *      pFixed_fid        - pointer to FID for MC_LOOKUP_ON_FIXED_FID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Lookup mode for multicast address is as following
 *      - MC_LOOKUP_ON_VID
 *      - MC_LOOKUP_ON_FID
 *      - MC_LOOKUP_ON_FIXED_FID
 *      
 *      When use MC_LOOKUP_ON_FIXED_FID, switch will use fixed_fid as lookup fid
 */
 int32
dal_esw_l2_mcastLookupMode_get(
    uint32                      unit, 
    rtk_l2_mcastLookupMode_t    *pMcast_lookupMode, 
    rtk_fid_t                   *pFixed_fid)    
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMcast_lookupMode), RT_ERR_NULL_POINTER);    
    RT_PARAM_CHK((NULL == pFixed_fid), RT_ERR_NULL_POINTER);

    /*Get multicast fix fid option*/
    if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_BASED_ON_VID_OR_FID_DECISION_CONTROLr, 
                 ESW_FIXMCFIDf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /*If multicast fix fid*/
    if(val == 1)
    {
        *pMcast_lookupMode = MC_LOOKUP_ON_FIXED_FID;

        /*Get fixed fid*/
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_BASED_ON_VID_OR_FID_DECISION_CONTROLr, 
                        ESW_MCFIDf, pFixed_fid)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        /*Get multciast lookup mode:(vid/fid)*/
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_BASED_ON_VID_OR_FID_DECISION_CONTROLr, 
                        ESW_MLFIDf, pMcast_lookupMode)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
 
    }
    
    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMcast_lookupMode=%d, pFixed_fid=%d", 
                    *pMcast_lookupMode, *pFixed_fid); 

    return RT_ERR_OK;      
}/*end of dal_esw_l2_mcastLookupMode_get */
/*end of dal_esw_l2_LRUEnable_get*/

/* Function Name:
 *      dal_esw_l2_mcastLookupMode_set
 * Description:
 *      Set lookup mode for multicast address.
 * Input:
 *      unit             - unit id
 *      mcast_lookupMode - lookup mode for multicast address
 *      fixed_fid        - FID for MC_LOOKUP_ON_FIXED_FID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_FID              - invalid fid
 * Note:
 *      Lookup mode for multicast address is as following
 *      - MC_LOOKUP_ON_VID
 *      - MC_LOOKUP_ON_FID
 *      - MC_LOOKUP_ON_FIXED_FID
 *      
 *      When use MC_LOOKUP_ON_FIXED_FID, switch will use fixed_fid as lookup fid
 */
 int32
dal_esw_l2_mcastLookupMode_set(
    uint32                      unit, 
    rtk_l2_mcastLookupMode_t    mcast_lookupMode, 
    rtk_fid_t                   fixed_fid)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, mcast_lookupMode=%d, fixed_fid=%d", 
                unit, mcast_lookupMode, fixed_fid); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((mcast_lookupMode >= MC_LOOKUP_END), RT_ERR_INPUT);    
    if(mcast_lookupMode == MC_LOOKUP_ON_FIXED_FID)
        RT_PARAM_CHK((fixed_fid > RTK_VLAN_ID_MAX), RT_ERR_FID);    

    L2_SEM_LOCK(unit);

    /*If mutilcast lookup on fix fid*/
    if(mcast_lookupMode == MC_LOOKUP_ON_FIXED_FID)
    {
        val = ENABLED;
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_BASED_ON_VID_OR_FID_DECISION_CONTROLr, 
                        ESW_FIXMCFIDf, &val)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        /*Set fixed fid*/
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_BASED_ON_VID_OR_FID_DECISION_CONTROLr, 
                        ESW_MCFIDf, &fixed_fid)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        /*Set multciast lookup mode:(vid/fid)*/
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_BASED_ON_VID_OR_FID_DECISION_CONTROLr, 
                        ESW_MLFIDf, &mcast_lookupMode)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        } 

        /*Disable Multicast Fix Fid*/
        val = DISABLED;
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_BASED_ON_VID_OR_FID_DECISION_CONTROLr, 
                        ESW_FIXMCFIDf, &val)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_mcastLookupMode_set*/

/* Function Name:
 *      dal_esw_l2_mcastBlockPortmask_get
 * Description:
 *      Get portmask that block multicast forwarding.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPortmask - pointer to portmask
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
dal_esw_l2_mcastBlockPortmask_get(uint32 unit, rtk_portmask_t *pPortmask)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);
    osal_memset(pPortmask, 0, sizeof(rtk_portmask_t));
    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_MULTICAST_CONTROL1r, 
                    ESW_MULDISPORTMASKf, &pPortmask->bits[0])) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pPortmask=%d", pPortmask->bits[0]); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_mcastBlockPortmask_get*/

/* Function Name:
 *      dal_esw_l2_mcastBlockPortmask_set
 * Description:
 *      Set portmask that block multicast forwarding.
 * Input:
 *      unit      - unit id
 *      pPortmask - portmask
 * Output:
 *      None.
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
dal_esw_l2_mcastBlockPortmask_set(uint32 unit, rtk_portmask_t *pPortmask)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pPortmask=%d", pPortmask->bits[0]); 

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_MULTICAST_CONTROL1r, 
                    ESW_MULDISPORTMASKf, &(pPortmask->bits[0]))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/*end of dal_esw_l2_mcastBlockPortmask_set*/

/* Function Name:
 *      dal_esw_l2_mcastAddr_init
 * Description:
 *      Initialize content of buffer of L2 multicast entry.
 *      Will fill vid ,MAC address and reset other field of L2 multicast entry.
 * Input:
 *      unit        - unit id
 *      vid         - vlan id
 *      pMac        - MAC address
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize L2 multicast entry before add it.
 */
int32
dal_esw_l2_mcastAddr_init(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac, rtk_l2_mcastAddr_t *pMcast_addr)
{  
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pMac, RT_ERR_NULL_POINTER);    
    RT_PARAM_CHK((pMac->octet[0] & BITMASK_1B) != 1, RT_ERR_MAC);
    RT_PARAM_CHK((vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    osal_memset(pMcast_addr, 0, sizeof(rtk_l2_mcastAddr_t));
    pMcast_addr->rvid = vid;
    osal_memcpy(&pMcast_addr->mac.octet[0], &pMac->octet[0], sizeof(rtk_mac_t));

    return RT_ERR_OK;    
}/*end of dal_esw_l2_mcastAddr_init*/

/* Function Name:
 *      dal_esw_l2_mcastAddr_add
 * Description:
 *      Add L2 multicast entry to ASIC.
 * Input:
 *      unit        - unit id
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize L2 multicast entry before add it.
 */
 int32
dal_esw_l2_mcastAddr_add(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32   ret;
   
    pMcast_addr->fwdIndex = -1; /* for automatically allocate */

    ret = dal_esw_l2_mcastAddr_add_with_index(unit, pMcast_addr);

    return ret;
    
} /* end of dal_esw_l2_mcastAddr_add */

/* Function Name:
 *      dal_esw_l2_mcastAddr_del
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
dal_esw_l2_mcastAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
    multicast_index_entry_t mcast_entry;
#endif
    
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
    
    ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    _dal_esw_l2_freeMcastIdx(unit, (int32)l2_entry.l2mcast.index);
    
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
    if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit) || CHIP_REV_ID_B == HAL_GET_CHIP_REV_ID(unit))
    {
        if ((l2_entry.l2mcast.index != 0) && (mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].entry_state == ENTRY_STATE_BACKUP))
        {
            /* when multicast entry is freed, reset the portmask to zero */
            osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
            if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(l2_entry.l2mcast.index - 1), (uint32 *)&mcast_entry)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                return RT_ERR_FAILED;
            }
    
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].ref_count = 0;
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].crossVlan = 0;
            osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].portmask, 0, sizeof(rtk_portmask_t));
            osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].l2_index, 0, sizeof(dal_esw_l2_index_t));
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].entry_state = ENTRY_STATE_AVAILABLE;
        }
    
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].crossVlan = 0;
        osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].portmask, 0, sizeof(rtk_portmask_t));
        osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].l2_index, 0, sizeof(dal_esw_l2_index_t));
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].entry_state = ENTRY_STATE_AVAILABLE;
        fwd_entry_inused_count--;
        _dal_esw_l2_mcastFwdIndex_arrange(unit, l2_entry.l2mcast.index);
        
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "delete fwd index:0x%u\n", l2_entry.l2mcast.index);
    }
#endif

    osal_memset(&l2_entry.l2mcast.mac, 0, sizeof(rtk_mac_t));
    l2_entry.l2mcast.rvid    = 0;
    l2_entry.l2mcast.index   = 0;
    l2_entry.valid = FALSE;
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_esw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_esw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_esw_l2_mcastAddr_del */

/* Function Name:
 *      dal_esw_l2_mcastAddr_get
 * Description:
 *      Update content of L2 multicast entry.
 * Input:
 *      unit        - unit id
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
 int32
dal_esw_l2_mcastAddr_get(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32 ret;
    
    ret = dal_esw_l2_mcastAddr_get_with_index(unit, pMcast_addr);
    
    return ret;
} /* end of dal_esw_l2_mcastAddr_get */

/* Function Name:
 *      dal_esw_l2_mcastAddr_set
 * Description:
 *      Get L2 multicast entry based on specified vid and MAC address
 * Input:
 *      unit        - unit id
 * Output:
 *      pMcast_addr - pointer to L2 multicast entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
 int32
dal_esw_l2_mcastAddr_set(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32   ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMcast_addr->mac.octet[0] & BITMASK_1B) != 1, RT_ERR_MAC);
    RT_PARAM_CHK((pMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pMcast_addr->rvid=%d", unit, pMcast_addr->rvid);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "cross vlan=%d", pMcast_addr->crossVlan);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMcast_addr->mac=%x-%x-%x-%x-%x-%x, pMcast_addr->portmask=0x%x",
           pMcast_addr->mac.octet[0], pMcast_addr->mac.octet[1], pMcast_addr->mac.octet[2], 
           pMcast_addr->mac.octet[3], pMcast_addr->mac.octet[4], pMcast_addr->mac.octet[5], pMcast_addr->portmask.bits[0]);
    
    /* search exist or free entry */    
    l2_entry.entry_type = L2_MULTICAST;
    l2_entry.l2mcast.rvid    = pMcast_addr->rvid;
    osal_memcpy(&l2_entry.l2mcast.mac.octet[0], &pMcast_addr->mac.octet[0], sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);
    
    ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if (index_entry.index_type == L2_IN_HASH)
        pMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pMcast_addr->l2_idx = (1 << 14) + index_entry.index;
    
    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
    if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, &pMcast_addr->portmask.bits[0], (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &pMcast_addr->crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = table_write(unit, ESW_FORWARDINGt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
    if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit) || CHIP_REV_ID_B == HAL_GET_CHIP_REV_ID(unit))
    {
        if ((l2_entry.l2mcast.index != 0) && (mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].entry_state == ENTRY_STATE_AVAILABLE))
        {
            if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(l2_entry.l2mcast.index - 1), (uint32 *)&mcast_entry)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                return RT_ERR_FAILED;
            }
    
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].ref_count = mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].ref_count;
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].crossVlan = pMcast_addr->crossVlan;
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].portmask = pMcast_addr->portmask;
            osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].l2_index, &index_entry, sizeof(dal_esw_l2_index_t));
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index - 1].entry_state = ENTRY_STATE_BACKUP;
        }
    
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].crossVlan = pMcast_addr->crossVlan;
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].portmask = pMcast_addr->portmask;
        osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].l2_index, &index_entry, sizeof(dal_esw_l2_index_t));
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].entry_state = ENTRY_STATE_INUSED;
        
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "set fwd index:0x%u\n", l2_entry.l2mcast.index);
    }
    else
    {
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].crossVlan = pMcast_addr->crossVlan;
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].portmask = pMcast_addr->portmask;
    
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "set fwd index:0x%u\n", l2_entry.l2mcast.index);
    }
#else
    mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].crossVlan = pMcast_addr->crossVlan;
    mcast_idx_pool[unit].pMcast_index_pool[l2_entry.l2mcast.index].portmask = pMcast_addr->portmask;

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "set fwd index:0x%u\n", l2_entry.l2mcast.index);
#endif    
    
    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_l2_mcastAddr_set */

/* Function Name:
 *      dal_esw_l2_mcastAddr_add_with_index
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
 *      If fwdIndex is larger than or equal to 0, will use fwdIndex as multicast index.
 *      If fwdIndex is smaller than 0, will allocate a free index and return it.
 */
 int32
dal_esw_l2_mcastAddr_add_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32 ret, ret_1;
    int32 fwdIndex;
    dal_esw_l2_entry_t  l2_entry, dynamic_l2_entry;
    dal_esw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    RT_PARAM_CHK(NULL == pMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMcast_addr->mac.octet[0] & BITMASK_1B) != 1, RT_ERR_MAC);
    RT_PARAM_CHK(pMcast_addr->fwdIndex >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    RT_PARAM_CHK((pMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_esw_l2_mcastAddr_add_with_index: unit=%d, rvid=%d, pMac=%x-%x-%x-%x-%x-%x \
           pPortmask=%x, pIndex=%d", unit, pMcast_addr->rvid, pMcast_addr->mac.octet[0], pMcast_addr->mac.octet[1], pMcast_addr->mac.octet[2], 
           pMcast_addr->mac.octet[3], pMcast_addr->mac.octet[4], pMcast_addr->mac.octet[5], pMcast_addr->portmask.bits[0], pMcast_addr->fwdIndex);
    
    /* search exist or free entry */    
    l2_entry.entry_type = L2_MULTICAST;
    l2_entry.l2mcast.rvid    = pMcast_addr->rvid;
    osal_memcpy(&l2_entry.l2mcast.mac, &(pMcast_addr->mac), sizeof(rtk_mac_t));

    ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry);
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

            ret_1= _dal_esw_l2_getFirstDynamicEntry(unit, &dynamic_l2_entry, &index_entry);
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
    if ((ret = _dal_esw_l2_allocMcastIdx(unit, &(pMcast_addr->fwdIndex))) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
    if (fwdIndex < 0)
    { /* Will configure portmask when fwdIndex is allocate automatically */        
        if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, &(pMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }

        if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &pMcast_addr->crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }

        if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)pMcast_addr->fwdIndex, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }

#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
        if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit) || CHIP_REV_ID_B == HAL_GET_CHIP_REV_ID(unit))
        {
            if ((pMcast_addr->fwdIndex != 0) && (mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex - 1].entry_state == ENTRY_STATE_AVAILABLE))
            {
                if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(pMcast_addr->fwdIndex - 1), (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                    return RT_ERR_FAILED;
                }
                mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex - 1].ref_count = mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex].ref_count;
                mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex - 1].crossVlan = pMcast_addr->crossVlan;
                mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex - 1].portmask = pMcast_addr->portmask;
                osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex - 1].l2_index, &index_entry, sizeof(dal_esw_l2_index_t));
                mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex - 1].entry_state = ENTRY_STATE_BACKUP;
            }
    
            mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex].crossVlan = pMcast_addr->crossVlan;
            mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex].portmask = pMcast_addr->portmask;
            osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex].l2_index, &index_entry, sizeof(dal_esw_l2_index_t));
            mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex].entry_state = ENTRY_STATE_INUSED;
            fwd_entry_inused_count++;
    
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "add set fwd index:0x%u\n", pMcast_addr->fwdIndex);
        }
        else
        {
            mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex].crossVlan = pMcast_addr->crossVlan;
            mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex].portmask = pMcast_addr->portmask;
            
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "add set fwd index:0x%u\n", pMcast_addr->fwdIndex);
        }
#else
        mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex].crossVlan = pMcast_addr->crossVlan;
        mcast_idx_pool[unit].pMcast_index_pool[pMcast_addr->fwdIndex].portmask = pMcast_addr->portmask;
        
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "add set fwd index:0x%u\n", pMcast_addr->fwdIndex);
#endif
    }
    
    /* fill content */
    osal_memcpy(&l2_entry.l2mcast.mac, &(pMcast_addr->mac), sizeof(rtk_mac_t));
    l2_entry.l2mcast.rvid    = pMcast_addr->rvid;
    l2_entry.l2mcast.index   = (uint32)(pMcast_addr->fwdIndex);
    l2_entry.valid = TRUE;
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_esw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if(L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_esw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    
    if (RT_ERR_OK != ret)
    {
        _dal_esw_l2_freeMcastIdx(unit, (int32)l2_entry.l2mcast.index);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    
    return RT_ERR_OK;
} /* end of dal_esw_l2_mcastAddr_add_with_index */

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
dal_esw_l2_mcastAddr_get_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32 ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_esw_l2_mcastAddr_get: unit=%d, vid=%d, pMac=%x-%x-%x-%x-%x-%x", 
           unit, pMcast_addr->rvid, pMcast_addr->mac.octet[0], pMcast_addr->mac.octet[1], pMcast_addr->mac.octet[2], pMcast_addr->mac.octet[2], 
           pMcast_addr->mac.octet[3], pMcast_addr->mac.octet[4], pMcast_addr->mac.octet[5]);

    
    RT_PARAM_CHK(NULL == pMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMcast_addr->mac.octet[0] & BITMASK_1B) != 1, RT_ERR_MAC);
    RT_PARAM_CHK((pMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* search exist or free entry */    
    l2_entry.entry_type     = L2_MULTICAST;
    l2_entry.l2mcast.rvid   = pMcast_addr->rvid;
    osal_memcpy(&l2_entry.l2mcast.mac, &(pMcast_addr->mac), sizeof(rtk_mac_t));

    ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
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
    
    if ((ret = table_read(unit, ESW_FORWARDINGt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, &(pMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &pMcast_addr->crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_esw_l2_mcastAddr_get: pPortmask=%x, crossVlan:%u", 
        pMcast_addr->portmask.bits[0],pMcast_addr->crossVlan);
    
    return ret;
} /* end of dal_esw_l2_mcastAddr_get_with_index */

/* Function Name:
 *      dal_esw_l2_mcastFwdIndex_alloc
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
dal_esw_l2_mcastFwdIndex_alloc(
    uint32          unit,
    int32           *pFwdIndex)
{
    
    RT_PARAM_CHK(NULL == pFwdIndex, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(*pFwdIndex >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", 
           unit, *pFwdIndex);
    
    if (*pFwdIndex >= 0)
    {
        if (RT_ERR_OK == _dal_esw_l2_isMcastIdxUsed(unit, *pFwdIndex))
        {
            return RT_ERR_L2_MCAST_FWD_ENTRY_EXIST;
        }
    }
    
    return _dal_esw_l2_allocMcastIdx(unit, pFwdIndex);
} /* end of dal_esw_l2_mcastFwdIndex_alloc */

/* Function Name:
 *      dal_esw_l2_mcastFwdIndex_free
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
dal_esw_l2_mcastFwdIndex_free(
    uint32          unit,
    int32           index)
{
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", 
           unit, index);

    RT_PARAM_CHK((index >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index) || index < 0, RT_ERR_L2_MULTI_FWD_INDEX);
    
    if (RT_ERR_OK != _dal_esw_l2_isMcastIdxUsed(unit, index))
    {
        return RT_ERR_L2_MCAST_FWD_ENTRY_NOT_EXIST;
    }
    
    return _dal_esw_l2_freeMcastIdx(unit, index);
} /* end of dal_esw_l2_mcastFwdIndex_free */

/* Function Name:
 *      dal_esw_l2_mcastFwdIndexFreeCount_get
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
dal_esw_l2_mcastFwdIndexFreeCount_get(uint32 unit, uint32 *pFreeCount)
{
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pFreeCount, RT_ERR_NULL_POINTER);
    
    L2_SEM_LOCK(unit);
    *pFreeCount = mcast_idx_pool[unit].free_entry_count;
    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_l2_mcastFwdIndexFreeCount_get */

/* Module Name    : L2           */
/* Sub-module Name: IP multicast */

/* Function Name:
 *      dal_esw_l2_ipmcEnable_get
 * Description:
 *      Get enable status of layer2 ip multicast switching.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of layer3 ip multicast switching
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
dal_esw_l2_ipmcEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, 
                    ESW_IPMCf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_ipmcEnable_get*/

/* Function Name:
 *      dal_esw_l2_ipmcEnable_set
 * Description:
 *      Set enable status of layer2 ip multicast switching.
 * Input:
 *      unit   - unit id
 *      enable - enable status of layer2 ip multicast switching
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
dal_esw_l2_ipmcEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, 
                    ESW_IPMCf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/*end of dal_esw_l2_ipmcEnable_set*/

/* Function Name:
 *      dal_esw_l2_ipmcMode_get
 * Description:
 *      Get lookup mode of layer2 ip multicast switching.
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - pointer to lookup mode of layer2 ip multicast switching
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Lookup mode of layer2 ip multicast switching is as following
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_ONLY
 */
int32
dal_esw_l2_ipmcMode_get(uint32 unit, rtk_l2_ipmcMode_t *pMode)
{
    int32   ret;
    uint32  mode = 0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, 
                    ESW_IPMCSRCf, &mode)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    switch (mode)
    {
        case 0:
            *pMode = LOOKUP_ON_DIP_AND_SIP;
            break;
        case 1:
            *pMode = LOOKUP_ON_DIP_ONLY;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMode=%d", *pMode); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_ipmcMode_get*/


/* Function Name:
 *      dal_esw_l2_ipmcMode_set
 * Description:
 *      Set lookup mode of layer2 ip multicast switching.
 * Input:
 *      unit - unit id
 *      mode - lookup mode of layer2 ip multicast switching
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Lookup mode of layer2 ip multicast switching is as following
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_ONLY
 */
int32
dal_esw_l2_ipmcMode_set(uint32 unit, rtk_l2_ipmcMode_t mode)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, mode=%d", unit, mode); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((mode >= IPMC_MODE_END), RT_ERR_NULL_POINTER);    

    switch (mode)
    {
        case LOOKUP_ON_DIP_AND_SIP:
            value = 0;
            break;
        case LOOKUP_ON_DIP_ONLY:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, 
                    ESW_IPMCSRCf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/*end of dal_esw_l2_ipmcMode_set*/


/* Function Name:
 *      dal_esw_l2_ipMcastAddr_init
 * Description:
 *      Initialize content of buffer of IP multicast entry.
 *      Will destination IP ,source IP and reset other field of IP multicast entry.
 * Input:
 *      unit          - unit id
 *      dip           - destination IP
 *      sip           - source IP
 *      pIpMcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
int32
dal_esw_l2_ipMcastAddr_init(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{  
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK(NULL == pIpMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((dip >>28) & BITMASK_4B) != 0xE, RT_ERR_IPV4_ADDRESS);

    osal_memset(pIpMcast_addr, 0, sizeof(rtk_l2_ipMcastAddr_t));
    pIpMcast_addr->dip = dip;
    pIpMcast_addr->sip = sip;
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_mcastAddr_init*/

/* Function Name:
 *      dal_esw_l2_ipMcastAddr_add
 * Description:
 *      Add IP multicast entry to ASIC.
 * Input:
 *      unit          - unit id
 *      pIpMcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
int32
dal_esw_l2_ipMcastAddr_add(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    int32 ret;
    
    pIpMcast_addr->fwdIndex = -1;

    ret = dal_esw_l2_ipMcastAddr_add_with_index(unit, pIpMcast_addr);
    
    return ret;
} /* end of dal_esw_l2_ipMcastAddr_add */

/* Function Name:
 *      dal_esw_l2_ipMcastAddr_del
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
dal_esw_l2_ipMcastAddr_del(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_vlan_t vid)
{
    int32 ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
    multicast_index_entry_t mcast_entry;
#endif
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, sip=%x, dip=%x, vid=%d", 
           unit, sip, dip, vid);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    
    RT_PARAM_CHK((vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* search exist or free entry */    
    l2_entry.entry_type   = IP_MULTICAST;
    l2_entry.ipmcast.dip  = dip;
    l2_entry.ipmcast.sip  = sip;
    
    L2_SEM_LOCK(unit);

    ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    _dal_esw_l2_freeMcastIdx(unit, (int32)l2_entry.ipmcast.index);
    
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
    if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit) || CHIP_REV_ID_B == HAL_GET_CHIP_REV_ID(unit))
    {
        if ((l2_entry.ipmcast.index != 0) && (mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].entry_state == ENTRY_STATE_BACKUP))
        {
            /* when multicast entry is freed, reset the portmask to zero */
            osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
            if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(l2_entry.ipmcast.index - 1), (uint32 *)&mcast_entry)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                return RT_ERR_FAILED;
            }
    
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].ref_count = 0;
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].crossVlan = 0;
            osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].portmask, 0, sizeof(rtk_portmask_t));
            osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].l2_index, 0, sizeof(dal_esw_l2_index_t));
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].entry_state = ENTRY_STATE_AVAILABLE;
        }
    
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].crossVlan = 0;
        osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].portmask, 0, sizeof(rtk_portmask_t));
        osal_memset(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].l2_index, 0, sizeof(dal_esw_l2_index_t));
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].entry_state = ENTRY_STATE_AVAILABLE;
        fwd_entry_inused_count--;
        _dal_esw_l2_mcastFwdIndex_arrange(unit, l2_entry.ipmcast.index);
        
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "delete fwd index:0x%u\n", l2_entry.ipmcast.index);
    }
#endif

    l2_entry.ipmcast.dip     = 0;
    l2_entry.ipmcast.sip     = 0;
    l2_entry.ipmcast.index     = 0;
    l2_entry.valid = FALSE;
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_esw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_esw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return ret;
} /* end of dal_esw_l2_ipMcastAddr_del */

/* Function Name:
 *      dal_esw_l2_ipMcastAddr_get
 * Description:
 *      Get IP multicast entry on specified dip and sip.
 * Input:
 *      unit          - unit id
 *      pIpMcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
int32
dal_esw_l2_ipMcastAddr_get(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    int32 ret;
    
    ret = dal_esw_l2_ipMcastAddr_get_with_index(unit, pIpMcast_addr);
    
    return ret;
} /* end of dal_esw_l2_ipMcastAddr_get */

/* Function Name:
 *      dal_esw_l2_ipMcastAddr_set
 * Description:
 *      Update content of IP multicast entry.
 * Input:
 *      unit          - unit id
 *      pIpMcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
int32
dal_esw_l2_ipMcastAddr_set(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    int32 ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pIpMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((pIpMcast_addr->dip >>28) & BITMASK_4B) != 0xE, RT_ERR_IPV4_ADDRESS);
    RT_PARAM_CHK((pIpMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_esw_l2_ipMcastAddr_set: unit=%d, sip=%x, dip=%x, vid=%d, pPortmask=%x", 
           unit, pIpMcast_addr->sip, pIpMcast_addr->dip, pIpMcast_addr->rvid, pIpMcast_addr->portmask.bits[0]);

    /* search exist or free entry */    
    l2_entry.entry_type = IP_MULTICAST;
    l2_entry.ipmcast.dip = pIpMcast_addr->dip;
    l2_entry.ipmcast.sip = pIpMcast_addr->sip;

    L2_SEM_LOCK(unit);

    ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    if (index_entry.index_type == L2_IN_HASH)
        pIpMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pIpMcast_addr->l2_idx = (1 << 14) + index_entry.index;

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
    if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, &(pIpMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &pIpMcast_addr->crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)l2_entry.ipmcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
    if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit) || CHIP_REV_ID_B == HAL_GET_CHIP_REV_ID(unit))
    {
        if ((l2_entry.ipmcast.index != 0) && (mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].entry_state == ENTRY_STATE_AVAILABLE))
        {
            if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(l2_entry.ipmcast.index - 1), (uint32 *)&mcast_entry)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                return RT_ERR_FAILED;
            }
    
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].ref_count = mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].ref_count;
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].crossVlan = pIpMcast_addr->crossVlan;
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].portmask = pIpMcast_addr->portmask;
            osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].l2_index, &index_entry, sizeof(dal_esw_l2_index_t));
            mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index - 1].entry_state = ENTRY_STATE_BACKUP;
        }
    
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].crossVlan = pIpMcast_addr->crossVlan;
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].portmask = pIpMcast_addr->portmask;
        osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].l2_index, &index_entry, sizeof(dal_esw_l2_index_t));
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].entry_state = ENTRY_STATE_INUSED;
    
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "set fwd index:0x%u\n", l2_entry.ipmcast.index);
    }
    else
    {
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].crossVlan = pIpMcast_addr->crossVlan;
        mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].portmask = pIpMcast_addr->portmask;
    
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "set fwd index:0x%u\n", l2_entry.ipmcast.index);
    }
#else
    mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].crossVlan = pIpMcast_addr->crossVlan;
    mcast_idx_pool[unit].pMcast_index_pool[l2_entry.ipmcast.index].portmask = pIpMcast_addr->portmask;

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "set fwd index:0x%u\n", l2_entry.ipmcast.index);
#endif    
    
    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_esw_l2_ipMcastAddr_set */

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
dal_esw_l2_ipMcastAddr_add_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    int32 ret, ret_1;
    int32 fwdIndex;
    dal_esw_l2_entry_t  l2_entry, dynamic_l2_entry;
    dal_esw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;
    
    RT_PARAM_CHK(NULL == pIpMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pIpMcast_addr->fwdIndex >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    RT_PARAM_CHK((pIpMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_esw_l2_ipMcastAddr_add_with_index: unit=%d, sip=%x, dip=%x, vid=%d, pPortmask=%x", 
           unit, pIpMcast_addr->sip, pIpMcast_addr->dip, pIpMcast_addr->rvid, pIpMcast_addr->portmask.bits[0]);

    /* search exist or free entry */    
    l2_entry.entry_type = IP_MULTICAST;
    l2_entry.ipmcast.dip     = pIpMcast_addr->dip;
    l2_entry.ipmcast.sip     = pIpMcast_addr->sip;

    ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry);    
    
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

            ret_1 = _dal_esw_l2_getFirstDynamicEntry(unit, &dynamic_l2_entry, &index_entry);
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
    if ((ret = _dal_esw_l2_allocMcastIdx(unit, &(pIpMcast_addr->fwdIndex))) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
    if (fwdIndex < 0)
    { /* Will configure portmask when fwdIndex is allocate automatically */        
        if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, &(pIpMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }

        if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &pIpMcast_addr->crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }

        if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(pIpMcast_addr->fwdIndex), (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }

#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
        if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit) || CHIP_REV_ID_B == HAL_GET_CHIP_REV_ID(unit))
        {
            if ((pIpMcast_addr->fwdIndex != 0) && (mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex - 1].entry_state == ENTRY_STATE_AVAILABLE))
            {
                if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(pIpMcast_addr->fwdIndex - 1), (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                    return RT_ERR_FAILED;
                }
        
                mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex - 1].ref_count = mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex].ref_count;
                mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex - 1].crossVlan = pIpMcast_addr->crossVlan;
                mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex - 1].portmask = pIpMcast_addr->portmask;
                osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex - 1].l2_index, &index_entry, sizeof(dal_esw_l2_index_t));
                mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex - 1].entry_state = ENTRY_STATE_BACKUP;
            }
    
            mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex].crossVlan = pIpMcast_addr->crossVlan;
            mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex].portmask = pIpMcast_addr->portmask;
            osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex].l2_index, &index_entry, sizeof(dal_esw_l2_index_t));
            mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex].entry_state = ENTRY_STATE_INUSED;
            fwd_entry_inused_count++;
            
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "add set fwd index:0x%u\n", pIpMcast_addr->fwdIndex);
        }
        else
        {
            mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex].crossVlan = pIpMcast_addr->crossVlan;
            mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex].portmask = pIpMcast_addr->portmask;
    
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "add set fwd index:0x%u\n", pIpMcast_addr->fwdIndex);
        }
#else
        mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex].crossVlan = pIpMcast_addr->crossVlan;
        mcast_idx_pool[unit].pMcast_index_pool[pIpMcast_addr->fwdIndex].portmask = pIpMcast_addr->portmask;

        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "add set fwd index:0x%u\n", pIpMcast_addr->fwdIndex);
#endif
    }
    
    l2_entry.ipmcast.dip     = pIpMcast_addr->dip;
    l2_entry.ipmcast.sip     = pIpMcast_addr->sip;
    l2_entry.ipmcast.index     = (uint32)(pIpMcast_addr->fwdIndex);
    l2_entry.valid = TRUE;
    
    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_esw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if(L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_esw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    
    if (RT_ERR_OK != ret)
    {
        _dal_esw_l2_freeMcastIdx(unit, (int32)l2_entry.ipmcast.index);
        return ret;
    }
    
    
    return RT_ERR_OK;
} /* end of dal_esw_l2_ipMcastAddr_add_with_index */
    
/* Function Name:
 *      dal_esw_l2_ipMcastAddr_get_with_index
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
dal_esw_l2_ipMcastAddr_get_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    int32 ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_esw_l2_ipMcastAddr_get: unit=%d, sip=%x, dip=%x, vid=%d", 
           unit, pIpMcast_addr->sip, pIpMcast_addr->dip, pIpMcast_addr->rvid);
    
    RT_PARAM_CHK(NULL == pIpMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pIpMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* search exist or free entry */    
    l2_entry.entry_type = IP_MULTICAST;
    l2_entry.ipmcast.dip     = pIpMcast_addr->dip;
    l2_entry.ipmcast.sip     = pIpMcast_addr->sip;

    ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);
    
    if (RT_ERR_OK != ret)
    {
        return ret;
    }
    
    if (index_entry.index_type == L2_IN_HASH)
        pIpMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pIpMcast_addr->l2_idx = (1 << 14) + index_entry.index;
    
    pIpMcast_addr->fwdIndex = (int32) l2_entry.ipmcast.index;
    
    if ((ret = table_read(unit, ESW_FORWARDINGt, l2_entry.ipmcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }
    
    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, &(pIpMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
        return ret;
    }

    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &pIpMcast_addr->crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_esw_l2_ipMcastAddr_get: pPortmask=%x", 
           pIpMcast_addr->portmask.bits[0]);
    
    return ret;
} /* end of dal_esw_l2_ipMcastAddr_get_with_index */



/* Function Name:
 *      dal_esw_l2_ipmc_routerPorts_get
 * Description:
 *      Get router ports for ip multicast switching. 
 *      All ip multicast packet will be forwarded to router ports.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPortmask - pointer to router portmask
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
dal_esw_l2_ipmc_routerPorts_get(uint32 unit, rtk_portmask_t *pPortmask)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);    

    L2_SEM_LOCK(unit);
    osal_memset(pPortmask, 0, sizeof(rtk_portmask_t));
    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_MULTICAST_CONTROL0r, 
                    ESW_ROUTERPORTMASKf, &pPortmask->bits[0])) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pPortmask=%d", pPortmask->bits[0]); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_ipmc_routerPorts_get*/

/* Function Name:
 *      dal_esw_l2_ipmc_routerPorts_set
 * Description:
 *      Set router ports for ip multicast switching. 
 *      All ip multicast packet will be forwarded to router ports.
 * Input:
 *      unit      - unit id
 *      pPortmask - router portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_PORT_MASK    - invalid portmask
 * Note:
 *      None
 */
int32
dal_esw_l2_ipmc_routerPorts_set(uint32 unit, rtk_portmask_t *pPortmask)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pPortmask=%d", pPortmask->bits[0]); 

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_MULTICAST_CONTROL0r, 
                    ESW_ROUTERPORTMASKf, &(pPortmask->bits[0]))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/*end of dal_esw_l2_ipmc_routerPorts_set*/

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchAction_get
 * Description:
 *      Get forwarding action when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit             - unit id
 *      type             - mismatch type
 * Output:
 *      pMismatch_action - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 *
 *      Forwarding action is as following:
 *      - L2_IPMC_MIS_DROP
 *      - L2_IPMC_MIS_TRAP
 *      - L2_IPMC_MIS_FWD_AS_L2
 *      - L2_IPMC_MIS_FWD_AS_IPMC
 */
int32 
dal_esw_l2_ipmcDstAddrMismatchAction_get(
    uint32                          unit,
    rtk_l2_ipmc_mismatchType_t      type,
    rtk_l2_ipmcMismatch_action_t    *pMismatch_action)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d", unit, type); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((NULL == pMismatch_action), RT_ERR_NULL_POINTER);   

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, 
                        ESW_L2MCIPMCf, pMismatch_action)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        
        /*Translate to ipmc action*/
        if(*pMismatch_action == 0)
            *pMismatch_action = L2_IPMC_MIS_FWD_AS_L2;
        else if(*pMismatch_action == 1)
            *pMismatch_action = L2_IPMC_MIS_FWD_AS_IPMC;
        else if(*pMismatch_action == 2)
            *pMismatch_action = L2_IPMC_MIS_TRAP;
        else
            *pMismatch_action = L2_IPMC_MIS_DROP;
    }
    else
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, 
                        ESW_L2UCIPMCf, pMismatch_action)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        
        /*Translate to ipmc action*/
        if(*pMismatch_action == 0)
            *pMismatch_action = L2_IPMC_MIS_FWD_AS_L2;
        else if(*pMismatch_action == 1)
            *pMismatch_action = L2_IPMC_MIS_TRAP;
        else if(*pMismatch_action == 2)
            *pMismatch_action = L2_IPMC_MIS_DROP;
        else
            *pMismatch_action = L2_IPMC_MIS_FWD_AS_IPMC;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMismatch_action=%d", *pMismatch_action); 
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_ipmcDstAddrMismatchAction_get*/

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchAction_set
 * Description:
 *      Set forwarding action when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit            - unit id
 *      type            - mismatch type
 *      mismatch_action - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 *
 *      Forwarding action is as following:
 *      - L2_IPMC_MIS_DROP
 *      - L2_IPMC_MIS_TRAP
 *      - L2_IPMC_MIS_FWD_AS_L2
 *      - L2_IPMC_MIS_FWD_AS_IPMC
 */
int32 
dal_esw_l2_ipmcDstAddrMismatchAction_set(
    uint32                          unit,
    rtk_l2_ipmc_mismatchType_t      type,
    rtk_l2_ipmcMismatch_action_t    mismatch_action)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d, mismatch_action=%d", 
                    unit, type, mismatch_action); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((mismatch_action >= L2_IPMC_MIS_END), RT_ERR_FWD_ACTION);   

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {
        /*Translate to asic ipmc action*/
        if(mismatch_action == L2_IPMC_MIS_FWD_AS_L2)
            mismatch_action = 0;
        else if(mismatch_action == L2_IPMC_MIS_FWD_AS_IPMC)
            mismatch_action = 1;
        else if(mismatch_action == L2_IPMC_MIS_TRAP)
            mismatch_action = 2;
        else
            mismatch_action = 3;
        
        if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, 
                        ESW_L2MCIPMCf, &mismatch_action)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }   
    }
    else
    {
        /*Translate to asic ipmc action*/
        if(mismatch_action == L2_IPMC_MIS_FWD_AS_L2)
            mismatch_action = 0;
        else if(mismatch_action == L2_IPMC_MIS_TRAP)
            mismatch_action = 1;
        else if(mismatch_action == L2_IPMC_MIS_DROP)
            mismatch_action = 2;
        else
            mismatch_action = 3;

        if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, 
                        ESW_L2UCIPMCf, &mismatch_action)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }      
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_ipmcDstAddrMismatchAction_set*/

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchPri_get
 * Description:
 *      Get priority of trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit      - unit id
 *      type      - mismatch type
 * Output:
 *      pPriority - pointer to priority of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32 
dal_esw_l2_ipmcDstAddrMismatchPri_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_pri_t *pPriority)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d", 
                    unit, type); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);   

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {        
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2MCCPRIf, pPriority)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {        
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2UCPRIf, pPriority)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pPriority=%d", *pPriority); 
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_ipmcDstAddrMismatchPri_get*/

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchPri_set
 * Description:
 *      Set priority of trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit     - unit id
 *      type     - mismatch type
 *      priority - priority of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32 
dal_esw_l2_ipmcDstAddrMismatchPri_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_pri_t priority)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d, priority=%d", 
                    unit, type, priority); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_PRIORITY);   

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {    
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2MCCPRIf, &priority)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }        
       
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2UCPRIf, &priority)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_ipmcDstAddrMismatchPri_set*/

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
dal_esw_l2_ipmcDstAddrMismatchPriEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d", 
                    unit, type); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);     

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {    
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_DFRL2MCPRIf, pEnable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_DFRL2UCPRIf, pEnable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_ipmcDstAddrMismatchPriEnable_get*/

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchPriEnable_set
 * Description:
 *      Set priority priority assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit     - unit id
 *      enable - priority assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_l2_ipmcDstAddrMismatchPriEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d, enable=%d", 
                    unit, type, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);      

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {    
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_DFRL2MCPRIf, &enable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_DFRL2UCPRIf, &enable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_ipmcDstAddrMismatchPriEnable_set*/


/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchDP_get
 * Description:
 *      Get drop precedence of trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit - unit id
 *      type - mismatch type
 * Output:
 *      pDp  - pointer to drop precedence of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
 int32 
dal_esw_l2_ipmcDstAddrMismatchDP_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, uint32 *pDp)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d", 
                    unit, type); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((NULL == pDp), RT_ERR_NULL_POINTER);   

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {        
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2MCDPf, pDp)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {        
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2UCDPf, pDp)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pDp=%d", *pDp); 
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_ipmcDstAddrMismatchDP_get*/


/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchDP_set
 * Description:
 *      Set drop precedence of trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit - unit id
 *      type - mismatch type
 *      dp   - drop precedence of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32 
dal_esw_l2_ipmcDstAddrMismatchDP_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, uint32 dp)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d, dp=%d", 
                    unit, type, dp); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((dp > 3), RT_ERR_DROP_PRECEDENCE);   

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2MCDPf, &dp)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2UCDPf, &dp)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_ipmcDstAddrMismatchDP_set*/

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchDPEnable_get
 * Description:
 *      Get drop procedence assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to drop procedence assignment status for trapped packet
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
dal_esw_l2_ipmcDstAddrMismatchDPEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d", 
                    unit, type); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);     

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_DFRL2MCDPf, pEnable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_DFRL2UCDPf, pEnable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_ipmcDstAddrMismatchDPEnable_get*/

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchDPEnable_set
 * Description:
 *      Set drop procedence drop procedence assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit     - unit id
 *      enable - drop procedence assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid drop procedence value
 * Note:
 *      None
 */
int32
dal_esw_l2_ipmcDstAddrMismatchDPEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d, enable=%d", 
                    unit, type, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);      

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_DFRL2MCDPf, &enable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_DFRL2UCDPf, &enable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_ipmcDstAddrMismatchDPEnable_set*/


/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 *      type    - mismatch type
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
dal_esw_l2_ipmcDstAddrMismatchAddCPUTagEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d", 
                    unit, type); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);   

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2MCCPUTAGf, pEnable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_read(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2UCCPUTAGf, pEnable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_ipmcDstAddrMismatchAddCPUTagEnable_get*/

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
 *      type   - mismatch type
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
dal_esw_l2_ipmcDstAddrMismatchAddCPUTagEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d, enable=%d", 
                    unit, type, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= L2_IPMC_MIS_TYPE_END), RT_ERR_INPUT);    
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);   

    L2_SEM_LOCK(unit);

    if(type == L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR)
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2MCCPUTAGf, &enable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_TRAP_CONTROLr, 
                        ESW_L2UCCPUTAGf, &enable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_ipmcDstAddrMismatchAddCPUTagEnable_set*/

/* Module Name    : L2                         */
/* Sub-module Name: Multicast forwarding table */
/* Function Name:
 *      dal_esw_l2_mcastFwdPortmask_set
 * Description:
 *      Get portmask of multicast forwarding entry
 * Input:
 *      unit       - unit id
 *      index      - index of multicast forwarding portmask
 *      *pPortmask - pointer buffer of ip multicast ports
 *      crossVlan - cross vlan flag of ip multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 */
int32
dal_esw_l2_mcastFwdPortmask_set(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask, uint32 crossVlan)
{
    int32   ret;
    multicast_index_entry_t mcast_entry;
    uint32 val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", 
       unit, index);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pPortmask, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(index >= mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pPortmask=0x%x", 
           pPortmask->bits[0]);
    
    L2_SEM_LOCK(unit);
    
    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    if ((ret = table_read(unit, ESW_FORWARDINGt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, &pPortmask->bits[0], (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    /*Clear the Aging bits*/
    val = 0;
    if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_AGINGf, &val, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = table_write(unit, ESW_FORWARDINGt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_l2_mcastFwdPortmask_set */    

/* Function Name:
 *      dal_esw_l2_mcastFwdPortmask_get
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
 */
int32
dal_esw_l2_mcastFwdPortmask_get(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask, uint32* pCrossVlan)
{
    int32   ret;
    multicast_index_entry_t mcast_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", 
       unit, index);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pPortmask, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(index >= mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    
    L2_SEM_LOCK(unit);
    
    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    if ((ret = table_read(unit, ESW_FORWARDINGt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    osal_memset(pPortmask, 0, sizeof(rtk_portmask_t));
    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, &pPortmask->bits[0], (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, pCrossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pPortmask=0x%x, pCrossVlan=%u", 
           pPortmask->bits[0], *pCrossVlan);
    
    return RT_ERR_OK;
} /* end of dal_esw_l2_mcastFwdPortmask_get */        

/* Module Name    : L2              */
/* Sub-module Name: CPU MAC address */

/* Function Name:
 *      dal_esw_l2_cpuMacAddr_add
 * Description:
 *      Add a CPU mac address of the vlan id to the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - cpu mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
int32
dal_esw_l2_cpuMacAddr_add(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d", unit, vid);
    
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
    l2_entry.unicast.fid = vid;
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);
    
    if ((ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    /* fill content */
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));
    l2_entry.unicast.fid = vid;
    if (-1 == HAL_GET_CPU_PORT(unit))
    {
        L2_SEM_UNLOCK(unit);
        return RT_ERR_L2_NO_CPU_PORT;
    }
    l2_entry.unicast.port = HAL_GET_CPU_PORT(unit);
    l2_entry.unicast.aging = 3;
    l2_entry.unicast.sablock = 0;
    l2_entry.unicast.dablock = 0;
    l2_entry.unicast.auth = 0;
    l2_entry.unicast.is_static = 1;
    l2_entry.unicast.nh = 0;
    l2_entry.unicast.suspend = 0;
    l2_entry.valid = TRUE;

    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_esw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_esw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    return ret;
} /* end of dal_esw_l2_cpuMacAddr_add */

/* Function Name:
 *      dal_esw_l2_cpuMacAddr_del
 * Description:
 *      Delete a CPU mac address of the vlan id from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - cpu mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
int32
dal_esw_l2_cpuMacAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_esw_l2_entry_t  l2_entry;
    dal_esw_l2_index_t  index_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d", unit, vid);
    
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
    l2_entry.unicast.fid = vid;
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);
    
    if ((ret = _dal_esw_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    /* fill content */
    osal_memset(&l2_entry.unicast.mac, 0, sizeof(rtk_mac_t));
    l2_entry.unicast.fid = 0;
    l2_entry.unicast.port = 0;
    l2_entry.unicast.aging = 0;
    l2_entry.unicast.sablock = 0;
    l2_entry.unicast.dablock = 0;
    l2_entry.unicast.auth = 0;
    l2_entry.unicast.is_static = 0;
    l2_entry.unicast.nh = 0;
    l2_entry.unicast.suspend = 0;
    l2_entry.valid = FALSE;

    if (L2_IN_HASH == index_entry.index_type )
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_esw_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    } 
    else if (L2_IN_CAM == index_entry.index_type )
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_esw_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }
    
    L2_SEM_UNLOCK(unit);
    return ret;
} /* end of dal_esw_l2_cpuMacAddr_del */


/* Function Name:
 *      _dal_esw_l2_init_config
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
_dal_esw_l2_init_config(uint32 unit)
{
    int32   ret;      
    
    if ((ret = dal_esw_l2_flushLinkDownPortAddrEnable_set(unit, RTK_DEFAULT_L2_FLUSH_LINKDOWN_MAC)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }    

#if defined(CONFIG_SDK_WA_LIMIT_LEARN_COUNT)
    {
        uint32  aging_time, val;
        hal_control_t *pHal_ctrl;

        if ((pHal_ctrl = hal_ctrlInfo_get(unit)) == NULL)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return RT_ERR_FAILED;
        }

        if ((RTL8328M_CHIP_ID == pHal_ctrl->chip_id || RTL8328S_CHIP_ID == pHal_ctrl->chip_id) &&
            (CHIP_REV_ID_A == pHal_ctrl->chip_rev_id))
        {
            /* Set Ageunit to twice * original value */
            if((ret = reg_field_read(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, ESW_AGEUNITf, &aging_time)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }
            val = 2*aging_time;
            if((ret = reg_field_write(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, ESW_AGEUNITf, &val)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }
        }
    }
#endif

    if ((ret = dal_esw_l2_camEnable_set(unit, DISABLED)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    if ((ret = dal_esw_l2_LRUEnable_set(unit, DISABLED)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of _dal_esw_l2_init_config */

/* Function Name:
 *      _dal_esw_l2_entryToHashKey
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
_dal_esw_l2_entryToHashKey(uint32 unit, dal_esw_l2_entry_t *pL2_entry, uint32 *pKey)
{
    uint8 hashSeed[8];
    uint32 val, index;
    uint32 hash_algo;    
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, ", unit, pL2_entry->entry_type);

    /* get hash seed from l2 entry */
    switch (pL2_entry->entry_type)
    {
        case L2_UNICAST:
            /* if it is unicast, key will be fid+mac */           
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "fid=%d, Mac=%x-%x-%x-%x-%x-%x", pL2_entry->unicast.fid,
                       pL2_entry->unicast.mac.octet[0], pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], 
                       pL2_entry->unicast.mac.octet[3], pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);
            hashSeed[0] = pL2_entry->unicast.mac.octet[5];
            hashSeed[1] = pL2_entry->unicast.mac.octet[4];
            hashSeed[2] = pL2_entry->unicast.mac.octet[3];
            hashSeed[3] = pL2_entry->unicast.mac.octet[2];
            hashSeed[4] = pL2_entry->unicast.mac.octet[1];
            hashSeed[5] = pL2_entry->unicast.mac.octet[0];
            hashSeed[6] = pL2_entry->unicast.fid & BITMASK_8B;
            hashSeed[7] = pL2_entry->unicast.fid >> 8;
            break;
        case L2_MULTICAST:
            /* if it is l2 multicast, key will be rvid+mac */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "rvid=%d, Mac=%x-%x-%x-%x-%x-%x", pL2_entry->l2mcast.rvid,
                       pL2_entry->l2mcast.mac.octet[0], pL2_entry->l2mcast.mac.octet[1], pL2_entry->l2mcast.mac.octet[2], 
                       pL2_entry->l2mcast.mac.octet[3], pL2_entry->l2mcast.mac.octet[4], pL2_entry->l2mcast.mac.octet[5]);
            hashSeed[0] = pL2_entry->unicast.mac.octet[5];
            hashSeed[1] = pL2_entry->unicast.mac.octet[4];
            hashSeed[2] = pL2_entry->unicast.mac.octet[3];
            hashSeed[3] = pL2_entry->unicast.mac.octet[2];
            hashSeed[4] = pL2_entry->unicast.mac.octet[1];
            hashSeed[5] = pL2_entry->unicast.mac.octet[0];
            hashSeed[6] = pL2_entry->l2mcast.rvid & BITMASK_8B;
            hashSeed[7] = pL2_entry->l2mcast.rvid >> 8;
            break;
        case IP_MULTICAST:
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "sip=%d, dip=%d", pL2_entry->ipmcast.sip, pL2_entry->ipmcast.dip);
            hashSeed[0] = pL2_entry->ipmcast.sip & 0xff;
            hashSeed[1] = (pL2_entry->ipmcast.sip >> 8) & 0xff;
            hashSeed[2] = (pL2_entry->ipmcast.sip >> 16) & 0xff;
            hashSeed[3] = (pL2_entry->ipmcast.sip >> 24) & 0xff;
            hashSeed[4] = pL2_entry->ipmcast.dip & 0xff;
            hashSeed[5] = (pL2_entry->ipmcast.dip >> 8) & 0xff;
            hashSeed[6] = (pL2_entry->ipmcast.dip >> 16) & 0xff;
            hashSeed[7] = (pL2_entry->ipmcast.dip >> 24) & 0xf;
            break;
        default:
            return RT_ERR_FAILED;
    }

    if(reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_L2_HASH_ALGOf, &hash_algo) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_L2), "");
        return RT_ERR_FAILED;
    }
    
    /* TBD */
    if (0 == hash_algo)
    {
        *pKey = hashSeed[0];
        *pKey = (*pKey << 2) ^ hashSeed[1];
        *pKey = (*pKey << 2) ^ hashSeed[2];
        *pKey = (*pKey << 2) ^ hashSeed[3];
        *pKey = (*pKey << 2) ^ hashSeed[4];
        *pKey = (*pKey << 2) ^ hashSeed[5];    
        *pKey = (*pKey << 2) ^ hashSeed[6];
        *pKey = (*pKey << 2) ^ (hashSeed[7] & BITMASK_4B);

        *pKey ^= (*pKey >> 12);

        *pKey =  *pKey & BITMASK_12B;
    } 
    else if (1 == hash_algo)
    {    
        *pKey   =  (hashSeed[0] | (hashSeed[1] << 8));
        *pKey ^= ((hashSeed[1] >> 4) |(hashSeed[2] << 4));
        *pKey ^= (hashSeed[3] | (hashSeed[4] << 8));
        *pKey ^= ((hashSeed[4] >> 4) |(hashSeed[5] << 4));
        val = (hashSeed[6] | (hashSeed[7] << 8));

        /*16 bit swap*/
        for(index = 0; index < 12; index++)
            *pKey ^= (((val >> index)&1) << (11- index));

        *pKey = *pKey & BITMASK_12B;
    } 
    else 
    {
        return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pKey=%d", *pKey);
    return RT_ERR_OK;
} /* end of _dal_esw_l2_entryToHashKey */

/* Function Name:
 *      _dal_esw_l2_nextValidAddr_get
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
_dal_esw_l2_nextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    uint32              type,
    uint32              include_static,
    dal_esw_l2_entry_t  *pL2_data)
{
    int32   ret;
    dal_esw_l2_entry_t  l2_entry;
    uint32  l2_idx;
    uint32  l2cam_idx;
    uint32  l2_tableSize;
    uint32  l2cam_tableSize;
    uint32  hashkey, location;
    uint32  isValid;
    uint32  found;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, type=%d, include_static=%d", 
           unit, *pScan_idx, type, include_static);
    
    if ((ret = table_size_get(unit, ESW_CAM_ISFTIDXt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = table_size_get(unit, ESW_L2_ISFTIDXt, &l2_tableSize)) != RT_ERR_OK)
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
        
        if ((_dal_esw_l2_getL2EntryfromHash(unit, hashkey, location, &l2_entry, &isValid) == RT_ERR_OK)
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
    
    if (FALSE == found)
    {
        for (l2cam_idx = l2_idx - l2_tableSize; l2cam_idx < l2cam_tableSize; l2cam_idx++)
        {
            if ((_dal_esw_l2_getL2EntryfromCAM(unit, l2cam_idx, &l2_entry, &isValid) == RT_ERR_OK)
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
    
    osal_memcpy(pL2_data, &l2_entry, sizeof(dal_esw_l2_entry_t));
    return RT_ERR_OK;
} /* end of _dal_esw_l2_nextValidAddr_get */


/* Function Name:
 *      _dal_esw_l2_nextValidAddrByRange_get
 * Description:
 *      Get next valid L2 unicast, multicast or ip multicast address entry by range from the specified device.
 * Input:
 *      unit           - unit id
 *      pScan_idx      - currently scan index of l2 table to get next.
 *      end_idx        - end index of search range
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
_dal_esw_l2_nextValidAddrByRange_get(
    uint32              unit,
    int32               *pScan_idx,
    uint32              end_idx,
    uint32              type,
    uint32              include_static,
    dal_esw_l2_entry_t  *pL2_data)
{
    int32   ret;
    dal_esw_l2_entry_t  l2_entry;
    uint32  l2_idx;
    uint32  l2cam_idx;
    uint32  l2_tableSize;
    uint32  l2cam_tableSize;
    uint32  hashkey, location;
    uint32  isValid;
    uint32  found;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, type=%d, include_static=%d", 
           unit, *pScan_idx, type, include_static);
    
    if ((ret = table_size_get(unit, ESW_CAM_ISFTIDXt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if ((ret = table_size_get(unit, ESW_L2_ISFTIDXt, &l2_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
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
    for (; (l2_idx < l2_tableSize) && (l2_idx <= end_idx); l2_idx++)
    {
        hashkey = l2_idx >> 2;
        location = l2_idx & BITMASK_2B;
        
        if ((_dal_esw_l2_getL2EntryfromHash(unit, hashkey, location, &l2_entry, &isValid) == RT_ERR_OK)
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
    
    if (FALSE == found && end_idx >= l2_tableSize)
    {
        for (l2cam_idx = l2_idx - l2_tableSize; (l2cam_idx < l2cam_tableSize) && (l2cam_idx <= end_idx - l2_tableSize); l2cam_idx++)
        {
            if ((_dal_esw_l2_getL2EntryfromCAM(unit, l2cam_idx, &l2_entry, &isValid) == RT_ERR_OK)
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
    
    osal_memcpy(pL2_data, &l2_entry, sizeof(dal_esw_l2_entry_t));
    return RT_ERR_OK;
} /* end of _dal_esw_l2_nextValidAddrByRange_get */


/* Function Name:
 *      _dal_esw_l2_setL2HASHEntry
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
_dal_esw_l2_setL2HASHEntry(uint32 unit, dal_esw_l2_entry_t *pL2_entry, dal_esw_l2_index_t *pL2_index)
{
    
    int32   ret;
    l2_entry_t l2_entry;
    uint32  aed0, aed1;
    uint32  l2_index;    
    uint32 val;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, hashdepth=%d, index=%d", 
           unit, pL2_entry->entry_type, pL2_index->hashdepth, pL2_index->index);
   
    osal_memset(&l2_entry, 0, sizeof(l2_entry));
    
    /* Extract content of each kind of entry from l2_enry */
    switch (pL2_entry->entry_type)
    {
        case L2_UNICAST: /* L2 unicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "da_block=%d, sa_block=%d, port=%d, suspend=%d, \
                   is_static=%d, aging=%d, auth=%d, nh=%d, Mac=%x-%x-%x-%x-%x-%x", 
                   pL2_entry->unicast.dablock, pL2_entry->unicast.sablock, pL2_entry->unicast.port, pL2_entry->unicast.suspend, 
                   pL2_entry->unicast.is_static, pL2_entry->unicast.aging, pL2_entry->unicast.auth, pL2_entry->unicast.nh, 
                   pL2_entry->unicast.mac.octet[0],  pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], 
                   pL2_entry->unicast.mac.octet[3],  pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);

            val = 0;
            if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_AETf, &val, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_ISFTIDXf, &val, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_VALIDf, &pL2_entry->valid, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if(pL2_entry->unicast.dablock)
            {
                 val = 31;
                 if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_PORTNUMf, &val, (uint32 *)&l2_entry)) != RT_ERR_OK)
                 {
                     RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                     return ret;
                 }
            }
            else
            {
                if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_PORTNUMf, &pL2_entry->unicast.port, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
            } 

            if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_SBLKf, &pL2_entry->unicast.sablock, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }   

            if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_ISSTATICf, &pL2_entry->unicast.is_static, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_AGINGf, &pL2_entry->unicast.aging, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_DOT1XMACAUTHf, &pL2_entry->unicast.auth, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_SUSPENDf, &pL2_entry->unicast.suspend, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_NHf, &pL2_entry->unicast.nh, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            aed1 = pL2_entry->unicast.mac.octet[5] | (pL2_entry->unicast.mac.octet[4] << 8);
            aed0 = pL2_entry->unicast.mac.octet[3] | (pL2_entry->unicast.mac.octet[2] << 8) | 
                        (pL2_entry->unicast.mac.octet[1] << 16) | ((pL2_entry->unicast.fid & BITMASK_12B) << 24); 
            
            if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_AED_47_16f, &aed0, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_AED_15_0f, &aed1, (uint32 *)&l2_entry)) != RT_ERR_OK)
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

            val = 0;
            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AETf, &val, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            val = 1;
            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_ISFTIDXf, &val, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }	

            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_VALIDf, &pL2_entry->valid, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_FTIDXf, &pL2_entry->l2mcast.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            aed1 = pL2_entry->l2mcast.mac.octet[5] | (pL2_entry->l2mcast.mac.octet[4] << 8);
            aed0 = pL2_entry->l2mcast.mac.octet[3] | (pL2_entry->l2mcast.mac.octet[2] << 8) |
                       (pL2_entry->l2mcast.mac.octet[1] << 16) | ((pL2_entry->l2mcast.rvid & BITMASK_12B) << 24); 

            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AED_47_16f, &aed0, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AED_15_0f, &aed1, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            break;
        case IP_MULTICAST: /* IP multicast */
             RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, dip=%d, sip=%d", 
                   pL2_entry->ipmcast.index, pL2_entry->ipmcast.dip, pL2_entry->ipmcast.sip);           

            val = 1;
            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AETf, &val, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_ISFTIDXf, &val, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_VALIDf, &pL2_entry->valid, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_FTIDXf, &pL2_entry->ipmcast.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            aed1 = pL2_entry->ipmcast.sip & 0xffff;
            aed0 = ((pL2_entry->ipmcast.sip >> 16) & 0xffff) | ((pL2_entry->ipmcast.dip & 0xff) << 16) | ((pL2_entry->ipmcast.dip & 0xff0000) << 8);

            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AED_47_16f, &aed0, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AED_15_0f, &aed1, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            break;
        default:
            return RT_ERR_FAILED;
     }
     
    l2_index =  (pL2_index->hashdepth << 12) | pL2_index->index ;

    /* write entry to chip */
    if ((ret = table_write(unit, ESW_L2_ISFTIDXt, l2_index, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    return RT_ERR_OK;
} /* end of _dal_esw_l2_setL2HASHEntry */


/* Function Name:
 *      _dal_esw_l2_setL2CAMEntry
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
_dal_esw_l2_setL2CAMEntry(uint32 unit, dal_esw_l2_entry_t *pL2_entry, dal_esw_l2_index_t *pL2_index)
{
    int32   ret;
    l2cam_entry_t l2cam_entry;
    uint32  aed0, aed1;
    uint32  l2_index;
    uint32 val;
    uint32 enable;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, hashdepth=%d, index=%d", 
           unit, pL2_entry->entry_type, pL2_index->hashdepth, pL2_index->index);

    /*Check CAM Enable*/
    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_LUTCAMENf, &enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    if (DISABLED == enable )
    {
        /* this is not a valid entry. No need to futher process*/
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Lookup Table Cam is Disabled");
        return RT_ERR_FAILED;
    }        
    
    l2_index = ~(CAM << ACCADDR_L2TYPE_OFFSET) &  pL2_index->index ;
    
    osal_memset(&l2cam_entry, 0, sizeof(l2cam_entry));    
    
    /* Extract content of each kind of entry from l2_enry */
    switch (pL2_entry->entry_type)
    {
        case L2_UNICAST: /* L2 unicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "da_block=%d, sa_block=%d, port=%d, fid=%d, suspend=%d, is_static=%d, aging=%d, auth=%d, \
                   Mac=%x-%x-%x-%x-%x-%x", 
                   pL2_entry->unicast.dablock, pL2_entry->unicast.sablock, pL2_entry->unicast.port, pL2_entry->unicast.fid, pL2_entry->unicast.suspend,
                   pL2_entry->unicast.is_static, pL2_entry->unicast.aging, pL2_entry->unicast.auth, pL2_entry->unicast.mac.octet[0],
                   pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], pL2_entry->unicast.mac.octet[3],
                   pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);     

            val = 0;
            if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_AETf, &val, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_ISFTIDXf, &val, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_VALIDf, &pL2_entry->valid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if(pL2_entry->unicast.dablock)
            {
                val = 31;
                if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_PORTNUMf, &val, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
            }
            else
            {
                if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_PORTNUMf, &pL2_entry->unicast.port, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            } 

            if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_SBLKf, &pL2_entry->unicast.sablock, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }   

            if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_ISSTATICf, &pL2_entry->unicast.is_static, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_AGINGf, &pL2_entry->unicast.aging, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_DOT1XMACAUTHf, &pL2_entry->unicast.auth, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_SUSPENDf, &pL2_entry->unicast.suspend, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            aed1 =  pL2_entry->unicast.mac.octet[5] | (pL2_entry->unicast.mac.octet[4] << 8) |
                        (pL2_entry->unicast.mac.octet[3] << 16) | (pL2_entry->unicast.mac.octet[2] << 24);
            aed0 = (pL2_entry->unicast.mac.octet[2] >> 4) | (pL2_entry->unicast.mac.octet[1] << 4) |
                        (pL2_entry->unicast.mac.octet[0] << 12) | (pL2_entry->unicast.fid << 20);       

            if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_AED_59_28f, &aed0, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_AED_27_0f, &aed1, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
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

            val = 0;
            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_AETf, &val, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            val = 1;
            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_ISFTIDXf, &val, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }	

            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_VALIDf, &pL2_entry->valid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_FTIDXf, &pL2_entry->l2mcast.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            aed1 =  pL2_entry->l2mcast.mac.octet[5] | (pL2_entry->l2mcast.mac.octet[4] << 8) |
                        (pL2_entry->l2mcast.mac.octet[3] << 16) | (pL2_entry->l2mcast.mac.octet[2] << 24);
            aed0 = (pL2_entry->l2mcast.mac.octet[2] >> 4) | (pL2_entry->l2mcast.mac.octet[1] << 4) |
                        (pL2_entry->l2mcast.mac.octet[0] << 12) | (pL2_entry->l2mcast.rvid << 20);      

            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_AED_59_28f, &aed0, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_AED_27_0f, &aed1, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }            
            break;
        case IP_MULTICAST: /* IP multicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, dip=%d, sip=%d", 
                   pL2_entry->ipmcast.index, pL2_entry->ipmcast.dip, pL2_entry->ipmcast.sip);

            val = 1;
            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_AETf, &val, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_ISFTIDXf, &val, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_VALIDf, &pL2_entry->valid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_FTIDXf, &pL2_entry->ipmcast.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            aed1 = pL2_entry->ipmcast.sip & 0xfffffff;
            aed0 = (pL2_entry->ipmcast.sip >> 28) | (pL2_entry->ipmcast.dip << 4);

            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_AED_59_28f, &aed0, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_AED_27_0f, &aed1, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }  
            
            break;
        default:
            return RT_ERR_FAILED;
     }
     
     /* write entry tochip */
    if ((ret = table_write(unit, ESW_CAM_ISFTIDXt, l2_index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of _dal_esw_l2_setL2CAMEntry */


/* Function Name:
 *      _dal_esw_l2_getExistOrFreeL2Entry
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
_dal_esw_l2_getExistOrFreeL2Entry(uint32 unit, dal_esw_l2_entry_t *pL2_entry, dal_esw_l2_getMethod_t get_method
        , dal_esw_l2_index_t *pL2_index)
{
    int32   ret;
    dal_esw_l2_entry_t  l2_entry;
    uint32  hashkey;
    uint32  hash_depth;
    uint32  cam_index;
    uint32  l2cam_tableSize, l2_tableSize;
    uint32  isValid;
    uint32  found_exist;
    uint32  found_free;    
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pL2_entry) || (NULL == pL2_index), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((get_method >= DAL_ESW_GETMETHOD_END), RT_ERR_INPUT);

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, Mac=%x-%x-%x-%x-%x-%x\
           get_method=%d, ", 
           unit, pL2_entry->entry_type, pL2_entry->unicast.mac.octet[0], pL2_entry->unicast.mac.octet[1], 
           pL2_entry->unicast.mac.octet[2], pL2_entry->unicast.mac.octet[3], pL2_entry->unicast.mac.octet[4],
           pL2_entry->unicast.mac.octet[5], get_method);
    
    osal_memset(&l2_entry, 0, sizeof(&l2_entry));

    pL2_entry->is_entry_exist = FALSE;
    
    /* calculate hash key */
    if ((ret = _dal_esw_l2_entryToHashKey(unit, pL2_entry, &hashkey)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    found_exist = FALSE;
    found_free = FALSE;
    
    /* Search L2 unicast entry in hash table */
    for (hash_depth = 0; hash_depth < HAL_L2_HASHDEPTH(unit); hash_depth++)
    {
        if (_dal_esw_l2_getL2EntryfromHash(unit, hashkey, hash_depth, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }
        
        if ((TRUE == isValid ) 
            && (get_method == L2_GET_EXIST_ONLY || get_method == L2_GET_EXIST_OR_FREE)
            && (l2_entry.entry_type == pL2_entry->entry_type))
        {
            if (_dal_esw_l2_compareEntry(&l2_entry, pL2_entry) == RT_ERR_OK)
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
    
    if ((ret = table_size_get(unit, ESW_CAM_ISFTIDXt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if (( ret = table_size_get(unit, ESW_L2_ISFTIDXt, &l2_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    for (cam_index = 0; cam_index < l2cam_tableSize; cam_index++)
    {
        if (_dal_esw_l2_getL2EntryfromCAM(unit, cam_index, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }
        
        if ((TRUE == isValid ) 
            && (get_method == L2_GET_EXIST_ONLY || get_method == L2_GET_EXIST_OR_FREE)
            && (l2_entry.entry_type == pL2_entry->entry_type))
        {
            if (_dal_esw_l2_compareEntry(&l2_entry, pL2_entry) == RT_ERR_OK)
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
} /* end of _dal_esw_l2_getExistOrFreeL2Entry */

/* Function Name:
 *      _dal_esw_l2_getFirstDynamicEntry
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
_dal_esw_l2_getFirstDynamicEntry(uint32 unit, dal_esw_l2_entry_t *pL2_entry
        , dal_esw_l2_index_t *pL2_index)
{
    int32   ret;
    dal_esw_l2_entry_t  l2_entry;
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
    if ((ret = _dal_esw_l2_entryToHashKey(unit, pL2_entry, &hashkey)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    found_exist = FALSE;
    
    /* Search L2 unicast entry in hash table */
    for (hash_depth = 0; hash_depth < HAL_L2_HASHDEPTH(unit); hash_depth++)
    {
        if (_dal_esw_l2_getL2EntryfromHash(unit, hashkey, hash_depth, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }

        if ((TRUE == isValid ) && (l2_entry.entry_type == L2_UNICAST))
        {
            if (l2_entry.unicast.sablock == 0 && l2_entry.unicast.dablock == 0 &&
                l2_entry.unicast.auth == 0 && l2_entry.unicast.is_static == 0 &&
                l2_entry.unicast.nh == 0 && l2_entry.unicast.suspend == 0)
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

    if ((ret = table_size_get(unit, ESW_CAM_ISFTIDXt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    if (( ret = table_size_get(unit, ESW_L2_ISFTIDXt, &l2_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    for (cam_index = 0; cam_index < l2cam_tableSize; cam_index++)
    {
        if (_dal_esw_l2_getL2EntryfromCAM(unit, cam_index, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }

        if ((TRUE == isValid ) && (l2_entry.entry_type == L2_UNICAST))
        {
            if (l2_entry.unicast.sablock == 0 && l2_entry.unicast.dablock == 0 &&
                l2_entry.unicast.auth == 0 && l2_entry.unicast.is_static == 0 &&
                l2_entry.unicast.suspend == 0)
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
} /* end of _dal_esw_l2_getFirstDynamicEntry */

/* Function Name:
 *      _dal_esw_l2_getL2EntryfromCAM
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
_dal_esw_l2_getL2EntryfromCAM(uint32 unit, uint32 index, 
                              dal_esw_l2_entry_t *pL2_entry, uint32 *pIsValid)
{
    int32   ret;
    l2_entry_t  l2_entry;
    uint32  aet;
    uint32  isFtIdx;
    uint32  aging;
    uint32  is_static;
    uint32  auth;
    uint32  aed0, aed1;
    uint32 enable;

     /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pL2_entry), RT_ERR_NULL_POINTER);   
    RT_PARAM_CHK((NULL == pIsValid), RT_ERR_NULL_POINTER);  
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, index=%d", unit, index);

    /*Check CAM Enable*/
    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_LUTCAMENf, &enable)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    if (DISABLED == enable )
    {
        /* this is not a valid entry. No need to futher process*/
        *pIsValid = FALSE;
        pL2_entry->valid = *pIsValid;
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Lookup Table Cam is Disabled");
        return RT_ERR_L2_ENTRY_NOTFOUND;
    }        
        
    /* read entry from chip */
    if ((ret = table_read(unit, ESW_CAM_ISFTIDXt, index, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    /* get valid bit from l2_entry */
    if ((ret = table_field_get(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_VALIDf, pIsValid, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    pL2_entry->valid = *pIsValid;

    /* check whether this entry is valid entry */
    if (*pIsValid == FALSE )
    {
        /* this is not a valid entry. No need to futher process*/
        *pIsValid = FALSE;
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pIsValid=%d", *pIsValid);
        return RT_ERR_OK;
    }
    
    /* get AET bit from l2_entry */
    if ((ret = table_field_get(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_AETf, &aet, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    /* get IsFTIDX bit from l2_entry */
    if ((ret = table_field_get(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_ISFTIDXf, &isFtIdx, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    
    aging = 0;
    is_static = 0;
    auth    = 0;

    /* check whether this entry is l2 multicast entry. 
     * If not l2 multicast entry, get more information for valid check 
     * L2 unicast entry: aet = 0, isFtIdx = 0;
     * L2 Multicast entry: aet =0, isFtIdx = 1;
     * Ip multicast entry: aet = 1, isFtIdx = 1;
     * Invalid entry: aet =1, isFtIdx = 0;
     */   
    
    /* Extract content of each kind of entry from l2_enry */
    switch ((aet << 1) | isFtIdx)
    {
        case 0x0: /* L2 unicast */
            pL2_entry->entry_type = L2_UNICAST;                   

            if ((ret = table_field_get(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_SUSPENDf, &pL2_entry->unicast.suspend, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }	

            if ((ret = table_field_get(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_DOT1XMACAUTHf,
                                                    &pL2_entry->unicast.auth, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }	

            if ((ret = table_field_get(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_AGINGf,
                                                    &pL2_entry->unicast.aging, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_ISSTATICf,
                                                    &pL2_entry->unicast.is_static, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_SBLKf, &pL2_entry->unicast.sablock, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_PORTNUMf, &pL2_entry->unicast.port, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if(pL2_entry->unicast.port > HAL_GET_MAX_PORT(unit))
                pL2_entry->unicast.dablock = TRUE;
            else
                pL2_entry->unicast.dablock = FALSE;

            if ((ret = table_field_get(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_AED_59_28f, &aed0, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_CAM_ISNOTFTIDXt, ESW_CAM_ISNOTFTIDX_AED_27_0f, &aed1, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            pL2_entry->unicast.mac.octet[5] = (aed1 & BITMASK_28B) & 0xff;
            pL2_entry->unicast.mac.octet[4] = ((aed1 & BITMASK_28B) >> 8) & 0xff;
            pL2_entry->unicast.mac.octet[3] = ((aed1 & BITMASK_28B) >> 16) & 0xff;
            pL2_entry->unicast.mac.octet[2] = (((aed1 & BITMASK_28B) >> 24) | ((aed0 & 0xf) << 4)) & 0xff;
            pL2_entry->unicast.mac.octet[1] = (aed0 >> 4) & 0xff;
            pL2_entry->unicast.mac.octet[0] = (aed0 >> 12) & 0xff;
            
            pL2_entry->unicast.fid = (aed0 >> 20) & BITMASK_12B;

            break;
        case 0x1: /* l2 Multicast */            
            pL2_entry->entry_type = L2_MULTICAST;

            if (table_field_get(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_FTIDXf, &pL2_entry->l2mcast.index, (uint32 *)&l2_entry) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return RT_ERR_FAILED;
            }

            if ((ret = table_field_get(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_AED_59_28f, &aed0, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_AED_27_0f, &aed1, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            pL2_entry->l2mcast.mac.octet[5] = (aed1 & BITMASK_28B) & 0xff;
            pL2_entry->l2mcast.mac.octet[4] = ((aed1 & BITMASK_28B) >> 8) & 0xff;
            pL2_entry->l2mcast.mac.octet[3] = ((aed1 & BITMASK_28B) >> 16) & 0xff;
            pL2_entry->l2mcast.mac.octet[2] = (((aed1 & BITMASK_28B) >> 24) | ((aed0 & 0xf) << 4)) & 0xff;
            pL2_entry->l2mcast.mac.octet[1] = (aed0 >> 4) & 0xff;
            pL2_entry->l2mcast.mac.octet[0] = (aed0 >> 12) & 0xff;
            
            pL2_entry->l2mcast.rvid = (aed0 >> 20) & BITMASK_12B;

            break;
        case 0x3: /* IP multicast */           
            pL2_entry->entry_type = IP_MULTICAST;

            if ((ret = table_field_get(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_FTIDXf, &pL2_entry->ipmcast.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return RT_ERR_FAILED;
            }

            if ((ret = table_field_get(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_AED_59_28f, &aed0, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_AED_27_0f, &aed1, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            pL2_entry->ipmcast.sip = aed1 | (aed0 << 28);
            pL2_entry->ipmcast.dip = (aed0 >> 4) | (0xe0 << 24);
                
            break;
        default:
            return RT_ERR_FAILED;
     }
     
     RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pIsValid=%d", *pIsValid);
     
     return RT_ERR_OK;
} /* end of _dal_esw_l2_getL2EntryfromHash */

/* Function Name:
 *      _dal_esw_l2_getL2EntryfromHash
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
_dal_esw_l2_getL2EntryfromHash(uint32 unit, uint32 hashkey, uint32 location, 
                              dal_esw_l2_entry_t *pL2_entry, uint32 *pIsValid)
{
    int32   ret;
    l2_entry_t  l2_entry;
    uint32  l2_index;
    uint32  aet;
    uint32  isFtIdx;
    uint32  aging;
    uint32  is_static;
    uint32  auth;
    uint32  aed0, aed1;

     /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pL2_entry), RT_ERR_NULL_POINTER);   
    RT_PARAM_CHK((NULL == pIsValid), RT_ERR_NULL_POINTER);  
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, hashkey=%d, location=%d, \
           entry_type=%d, Mac=%x-%x-%x-%x-%x-%x", unit, hashkey, location, pL2_entry->entry_type, 
           pL2_entry->unicast.mac.octet[0], pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], 
           pL2_entry->unicast.mac.octet[3], pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);
    
    /* calculate l2 index in hash table */
    l2_index =  (location << 12) | hashkey;
    
    /* read entry from chip */
    if ((ret = table_read(unit, ESW_L2_ISFTIDXt, l2_index, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }   

    /* get valid bit from l2_entry */
    if ((ret = table_field_get(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_VALIDf, pIsValid, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    pL2_entry->valid = *pIsValid;

    /* check whether this entry is valid entry */
    if (*pIsValid == FALSE )
    {
        /* this is not a valid entry. No need to futher process*/
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pIsValid=%d", *pIsValid);
        return RT_ERR_OK;
    }    

    /* get AET bit from l2_entry */
    if ((ret = table_field_get(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AETf, &aet, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    /* get IsFTIDX bit from l2_entry */
    if ((ret = table_field_get(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_ISFTIDXf, &isFtIdx, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }

    aging = 0;
    is_static = 0;
    auth    = 0;

    /* check whether this entry is l2 multicast entry. 
     * If not l2 multicast entry, get more information for valid check 
     * L2 unicast entry: aet = 0, isFtIdx = 0;
     * L2 Multicast entry: aet =0, isFtIdx = 1;
     * Ip multicast entry: aet = 1, isFtIdx = 1;
     * Invalid entry: aet =1, isFtIdx = 0;
     */   
    
    /* Extract content of each kind of entry from l2_enry */
    switch ((aet << 1) | isFtIdx)
    {
        case 0x0: /* L2 unicast */
            pL2_entry->entry_type = L2_UNICAST;
                    
            if ((ret = table_field_get(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_NHf, &pL2_entry->unicast.nh, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_SUSPENDf, &pL2_entry->unicast.suspend, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_DOT1XMACAUTHf, 
                                                    &pL2_entry->unicast.auth, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_AGINGf, 
                                                    &pL2_entry->unicast.aging, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_ISSTATICf,
                                                    &pL2_entry->unicast.is_static, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_SBLKf, &pL2_entry->unicast.sablock, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_PORTNUMf, &pL2_entry->unicast.port, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if(pL2_entry->unicast.port > HAL_GET_MAX_PORT(unit))
                 pL2_entry->unicast.dablock = TRUE;
            else
                pL2_entry->unicast.dablock = FALSE;

            if ((ret = table_field_get(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_AED_47_16f, &aed0, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_AED_15_0f, &aed1, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = _dal_esw_hashKeyToL2_entry(unit, hashkey, aed0, aed1, pL2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            break;
        case 0x1: /* l2 Multicast */            
            pL2_entry->entry_type = L2_MULTICAST;

            if (table_field_get(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_FTIDXf, &pL2_entry->l2mcast.index, (uint32 *)&l2_entry) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return RT_ERR_FAILED;
            }

            if ((ret = table_field_get(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AED_47_16f, &aed0, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AED_15_0f, &aed1, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = _dal_esw_hashKeyToL2_entry(unit, hashkey, aed0, aed1, pL2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            break;
        case 0x3: /* IP multicast */           
            pL2_entry->entry_type = IP_MULTICAST;

            if (table_field_get(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_FTIDXf, &pL2_entry->ipmcast.index, (uint32 *)&l2_entry) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return RT_ERR_FAILED;
            }

            if ((ret = table_field_get(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AED_47_16f, &aed0, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_get(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AED_15_0f, &aed1, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = _dal_esw_hashKeyToL2_entry(unit, hashkey, aed0, aed1, pL2_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            break;
        default:
            return RT_ERR_FAILED;
     }
     
     RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pIsValid=%d", *pIsValid);
     
     return RT_ERR_OK;
} /* end of _dal_esw_l2_getL2EntryfromHash */

/* Function Name:
 *      _dal_esw_hashKeyToL2_entry
 * Description:
 *	 Get entry From Hash Key
 * Input:
 *      unit      - unit id
 *	  hashKey - key for Hash
 *	  aed0	 - address table data0
 *	  aed1	- address table data1
 * Output:
 *      pL2_entry - L2 entry used to generate seed
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32 
_dal_esw_hashKeyToL2_entry(uint32 unit, uint32 hashKey, uint32 aed0, uint32 aed1,dal_esw_l2_entry_t *pL2_entry)
{
    uint8 ald[8];
    uint32 aed[6];
    uint32 hash_algo;
    
    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, hashKey=%d,aed0=%d, aed1=%d", unit, hashKey, aed0, aed1);

    RT_PARAM_CHK((NULL == pL2_entry), RT_ERR_NULL_POINTER); 

    aed[0] = (aed1 & BITMASK_16B) & 0xff;
    aed[1] = ((aed1 & BITMASK_16B) >> 8) & 0xff;
    aed[2] = aed0 & 0xff;
    aed[3] = (aed0 >> 8) & 0xff;
    aed[4] = (aed0 >> 16) & 0xff;
    aed[5] = (aed0 >> 24) & 0xff;

    ald[0] = (aed1 & BITMASK_16B) & 0xff;
    ald[1] = ((aed1 & BITMASK_16B) >> 8) & 0xff;
    ald[2] = aed0 & 0xff;
    ald[3] = (aed0 >> 8) & 0xff;
    ald[4] = (aed0 >> 16) & 0xff;
    ald[6] = (aed0 >> 24) & 0xff;

    if(reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_CONTROLr, ESW_L2_HASH_ALGOf, &hash_algo) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_L2), "");
        return RT_ERR_FAILED;
    }
	
    if (0 == hash_algo)
    {
        ald[7] = ((((hashKey >> 0) ^ (aed[1] >> 0) ^ (aed[2] >> 2) ^ (aed[3] >> 4) ^ (aed[4] >> 6)) & 1) |
                  ((((hashKey >> 1) ^ (aed[1] >> 1) ^ (aed[2] >> 3) ^ (aed[3] >> 5) ^ (aed[4] >> 7)) & 1)) << 1) |
                ((((hashKey >> 2) ^ (aed[0] >> 0) ^ (aed[1] >> 2) ^ (aed[2] >> 4) ^ (aed[3] >> 6) ^ (aed[5] >> 0)) & 1) << 2) |
                ((((hashKey >> 3) ^ (aed[0] >> 1) ^ (aed[1] >> 3) ^ (aed[2] >> 5) ^ (aed[3] >> 7) ^ (aed[5] >> 1)) & 1) << 3);

        ald[5] = (((hashKey >> 4) ^ (aed[0] >> 2) ^ (aed[1] >> 4) ^ (aed[2] >> 6) ^ (aed[5] >> 2)) & 1) |
                ((((hashKey >> 5) ^ (aed[0] >> 3) ^ (aed[1] >> 5) ^ (aed[2] >> 7) ^ (aed[5] >> 3)) & 1) << 1) |
                ((((hashKey >> 6) ^ (aed[0] >> 4) ^ (aed[1] >> 6) ^ (aed[4] >> 0) ^ (aed[5] >> 4)) & 1) << 2) |
                ((((hashKey >> 7) ^ (aed[0] >> 5) ^ (aed[1] >> 7) ^ (aed[4] >> 1) ^ (aed[5] >> 5)) & 1) << 3) |
                ((((hashKey >> 8) ^ (aed[0] >> 6) ^ (aed[3] >> 0) ^ (aed[4] >> 2) ^ (aed[5] >> 6)) & 1) << 4) |
                ((((hashKey >> 9) ^ (aed[0] >> 7) ^ (aed[3] >> 1) ^ (aed[4] >> 3) ^ (aed[5] >> 7)) & 1) << 5) |
                ((((hashKey >> 10) ^ (aed[2] >> 0) ^ (aed[3] >> 2) ^ (aed[4] >> 4)) & 1) << 6) |
                ((((hashKey >> 11) ^ (aed[2] >> 1) ^ (aed[3] >> 3) ^ (aed[4] >> 5)) & 1) << 7);
    } 
    else
    {
        ald[7] = ((((hashKey >> 3) ^ (aed[0] >> 3) ^ (aed[1] >> 7) ^ (aed[3] >> 3) ^ (aed[4] >> 7)) & 1) |
                ((((hashKey >> 2) ^ (aed[0] >> 2) ^ (aed[1] >> 6) ^ (aed[3] >> 2) ^ (aed[4] >> 6)) & 1) << 1) |
                ((((hashKey >> 1) ^ (aed[0] >> 1) ^ (aed[1] >> 5) ^ (aed[3] >> 1) ^ (aed[4] >> 5)) & 1) << 2) |
                ((((hashKey >> 0) ^ (aed[0] >> 0) ^ (aed[1] >> 4) ^ (aed[3] >> 0) ^ (aed[4] >> 4)) & 1) << 3));

        ald[5] = (((hashKey >> 4) ^ (aed[0] >> 4) ^ (aed[2] >> 0) ^ (aed[3] >> 4) ^ (aed[5] >> 7)) & 1) |
                ((((hashKey >> 5) ^ (aed[0] >> 5) ^ (aed[2] >> 1) ^ (aed[3] >> 5) ^ (aed[5] >> 6)) & 1) << 1) |
                ((((hashKey >> 6) ^ (aed[0] >> 6) ^ (aed[2] >> 2) ^ (aed[3] >> 6) ^ (aed[5] >> 5)) & 1) << 2) |
                ((((hashKey >> 7) ^ (aed[0] >> 7) ^ (aed[2] >> 3) ^ (aed[3] >> 7) ^ (aed[5] >> 4)) & 1) << 3) |
                ((((hashKey >> 8) ^ (aed[1] >> 0) ^ (aed[2] >> 4) ^ (aed[4] >> 0) ^ (aed[5] >> 3)) & 1) << 4) |
               ((((hashKey >> 9) ^ (aed[1] >> 1) ^ (aed[2] >> 5) ^ (aed[4] >> 1) ^ (aed[5] >> 2)) & 1) << 5) |
               ((((hashKey >> 10) ^ (aed[1] >> 2) ^ (aed[2] >> 6) ^ (aed[4] >> 2)^(aed[5] >> 1)) & 1) << 6) |
               ((((hashKey >> 11) ^ (aed[1] >> 3) ^ (aed[2] >> 7) ^ (aed[4] >> 3)^(aed[5] >> 0)) & 1) << 7);
    } 

    /*translate hashKey to entry content*/
    switch (pL2_entry->entry_type)
    {
        case L2_UNICAST:
                pL2_entry->unicast.mac.octet[5] = ald[0];
                pL2_entry->unicast.mac.octet[4] = ald[1];
                pL2_entry->unicast.mac.octet[3] = ald[2];
                pL2_entry->unicast.mac.octet[2] = ald[3];
                pL2_entry->unicast.mac.octet[1] = ald[4];
                pL2_entry->unicast.mac.octet[0] = ald[5];
                pL2_entry->unicast.fid = ald[6] | ((ald[7] & 0xf) << 8);
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "entry_type=%d, fid=%d,\
                    Mac=%x-%x-%x-%x-%x-%x", unit, pL2_entry->entry_type, pL2_entry->unicast.fid,
                    pL2_entry->unicast.mac.octet[0], pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], 
                    pL2_entry->unicast.mac.octet[3], pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);
                break;
        case L2_MULTICAST:
                pL2_entry->l2mcast.mac.octet[5] = ald[0];
                pL2_entry->l2mcast.mac.octet[4] = ald[1];
                pL2_entry->l2mcast.mac.octet[3] = ald[2];
                pL2_entry->l2mcast.mac.octet[2] = ald[3];
                pL2_entry->l2mcast.mac.octet[1] = ald[4];
                pL2_entry->l2mcast.mac.octet[0] = ald[5];
                pL2_entry->l2mcast.rvid = ald[6] | ((ald[7] & 0xf) << 8);
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "entry_type=%d, rvid=%d,\
                    Mac=%x-%x-%x-%x-%x-%x", unit, pL2_entry->entry_type, pL2_entry->l2mcast.rvid,
                    pL2_entry->l2mcast.mac.octet[0], pL2_entry->l2mcast.mac.octet[1], pL2_entry->l2mcast.mac.octet[2], 
                    pL2_entry->l2mcast.mac.octet[3], pL2_entry->l2mcast.mac.octet[4], pL2_entry->l2mcast.mac.octet[5]);
                break;
        case IP_MULTICAST:
                pL2_entry->ipmcast.sip = ald[0] | (ald[1] << 8) | (ald[2] << 16) | (ald[3] << 24);
                pL2_entry->ipmcast.dip = ald[4] | (ald[5] << 8) | (ald[6] << 16) | ((ald[7] | (0xe0)) << 24);
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "sip=%d, dip=%d", pL2_entry->ipmcast.sip, pL2_entry->ipmcast.dip);
                break;
        default:
                return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}/*end of _dal_esw_hashKeyToL2_entry*/

/* Function Name:
 *      _dal_esw_l2_compareEntry
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
_dal_esw_l2_compareEntry(dal_esw_l2_entry_t *pSrcEntry, dal_esw_l2_entry_t *pDstEntry)
{
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
            
            return RT_ERR_OK;
            break;
        default:           
            return RT_ERR_FAILED;
    }
    
} /* end of _dal_esw_l2_compareEntry*/

/* Function Name:
 *      _dal_esw_l2_allocMcastIdx
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
static int32 _dal_esw_l2_allocMcastIdx(uint32 unit, int32 *pMcastIdx)
{
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
    if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit) || CHIP_REV_ID_B == HAL_GET_CHIP_REV_ID(unit))
    {
        uint32  index;
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);
    
        if (*pMcastIdx >= 0)
        {
            mcast_idx_pool[unit].pMcast_index_pool[*pMcastIdx].ref_count++;
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "*pMcastIdx=%d", *pMcastIdx);
            return RT_ERR_OK;
        }
    
        if (fwd_entry_inused_count < mcast_idx_pool[unit].size_of_mcast_fwd_index/2)
        {
            /* Find AA and return 2nd A index */
            for (index = 0; index < mcast_idx_pool[unit].size_of_mcast_fwd_index-1; index++)
            {
                if (mcast_idx_pool[unit].pMcast_index_pool[index].entry_state == ENTRY_STATE_AVAILABLE && 
                    mcast_idx_pool[unit].pMcast_index_pool[index+1].entry_state == ENTRY_STATE_AVAILABLE)
                {
                    //if (0 == mcast_idx_pool[unit].pMcast_index_pool[index+1].ref_count)
                    {
                        *pMcastIdx = index+1;
                        mcast_idx_pool[unit].pMcast_index_pool[index+1].ref_count++;
                        mcast_idx_pool[unit].free_entry_count--;
                        return RT_ERR_OK;
                    }
                }
            }
            return RT_ERR_FAILED;
        }
        else
        {
            /* Find B index */
            for (index = 0; index < mcast_idx_pool[unit].size_of_mcast_fwd_index; index++)
            {
                if (mcast_idx_pool[unit].pMcast_index_pool[index].entry_state == ENTRY_STATE_AVAILABLE)
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Something wrong due to index=%d is available state", index);
                }
    
                if (mcast_idx_pool[unit].pMcast_index_pool[index].entry_state == ENTRY_STATE_BACKUP)
                {
                    *pMcastIdx = index;
                    mcast_idx_pool[unit].pMcast_index_pool[index].ref_count = 1;
                    mcast_idx_pool[unit].free_entry_count--;
                    return RT_ERR_OK;
                }
            }
        }
    
        return RT_ERR_L2_INDEXTBL_FULL;
    }
    else
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
    }
#else
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
#endif
} /* _dal_esw_l2_allocMcastIdx */

/* Function Name:
 *      _dal_esw_l2_freeMcastIdx
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
static int32 _dal_esw_l2_freeMcastIdx(uint32 unit, int32 mcastIdx)
{
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
    if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit) || CHIP_REV_ID_B == HAL_GET_CHIP_REV_ID(unit))
    {
        int32   ret;
        multicast_index_entry_t mcast_entry;
        
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, mcastIdx=%d", unit, mcastIdx);
            
        mcast_idx_pool[unit].pMcast_index_pool[mcastIdx].ref_count--;
        
        if (0 == mcast_idx_pool[unit].pMcast_index_pool[mcastIdx].ref_count)
        {
            if (mcast_idx_pool[unit].pMcast_index_pool[mcastIdx].entry_state == ENTRY_STATE_INUSED)
            {
                /* when multicast entry is freed, reset the portmask to zero */
                osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                if ((ret = table_write(unit, ESW_FORWARDINGt, mcastIdx, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                    return ret;
                }
            }
            else
            {
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Delete index=%d state [value: %d] is not inused", mcastIdx, mcast_idx_pool[unit].pMcast_index_pool[mcastIdx].entry_state);
            }
            mcast_idx_pool[unit].free_entry_count++;
        }
        
        return RT_ERR_OK;
    }
    else
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
            if ((ret = table_write(unit, ESW_FORWARDINGt, mcastIdx, (uint32 *)&mcast_entry)) != RT_ERR_OK)
            {
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                return ret;
            }
        }
        
        return RT_ERR_OK;
    }
#else
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
        if ((ret = table_write(unit, ESW_FORWARDINGt, mcastIdx, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
            return ret;
        }
    }
    
    return RT_ERR_OK;
#endif
} /* _dal_esw_l2_freeMcastIdx */

/* Function Name:
 *      _dal_esw_l2_isMcastIdxUsed
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
static int32 _dal_esw_l2_isMcastIdxUsed(uint32 unit, int32 mcastIdx)
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
    
} /* _dal_esw_l2_isMcastIdxUsed */


/* Module Name    : L2        */
/* Sub-module Name: Port move */

/* Function Name:
 *      dal_esw_l2_legalMoveToPorts_get
 * Description:
 *      Get legal ports for moving to on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pLegalPorts - pointer to legal ports for moving to
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
dal_esw_l2_legalMoveToPorts_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pLegalPorts)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d",  unit, port); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);   
    RT_PARAM_CHK((NULL == pLegalPorts), RT_ERR_NULL_POINTER); 

    L2_SEM_LOCK(unit);

    osal_memset(pLegalPorts, 0, sizeof(rtk_portmask_t));
    if((ret = reg_array_field_read(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL1r,
                    port, REG_ARRAY_INDEX_NONE, ESW_SML_MOV_PMf, &pLegalPorts->bits[0])) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pLegalPorts=0x%x",  pLegalPorts->bits[0]); 

    return RT_ERR_OK;
}   /*end of dal_esw_l2_legalMoveToPorts_get*/

/* Function Name:
 *      dal_esw_l2_legalMoveToPorts_set
 * Description:
 *      Set legal ports for moving to on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      pLegalPorts - legal ports for moving to
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_PORT_MASK        - invalid portmask
 * Note:
 *      None
 */
int32
dal_esw_l2_legalMoveToPorts_set(uint32 unit, rtk_port_t port, rtk_portmask_t *pLegalPorts)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d",  unit, port); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);   
    RT_PARAM_CHK((NULL == pLegalPorts), RT_ERR_NULL_POINTER); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pLegalPorts=0x%x",  pLegalPorts->bits[0]); 

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_write(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL1r,
                    port, REG_ARRAY_INDEX_NONE, ESW_SML_MOV_PMf, &(pLegalPorts->bits[0]))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_legalMoveToPorts_set*/

/* Function Name:
 *      dal_esw_l2_illegalPortMoveAction_get
 * Description:
 *      Get forwarding action when illegal port moving happen on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pFwdAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
int32
dal_esw_l2_illegalPortMoveAction_get(
    uint32              unit, 
    rtk_port_t          port, 
    rtk_action_t        *pFwdAction)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d",  unit, port); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);   
    RT_PARAM_CHK((NULL == pFwdAction), RT_ERR_NULL_POINTER); 

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_read(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r, 
                            port, REG_ARRAY_INDEX_NONE, ESW_SML_ILLEGALMVACTf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /*Translate to rtk common action*/
    if(val == 0)
        *pFwdAction = ACTION_FORWARD;
    else if(val == 1)
        *pFwdAction = ACTION_COPY2CPU;
    else if(val == 2)
        *pFwdAction = ACTION_TRAP2CPU;
    else
        *pFwdAction = ACTION_DROP; 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pFwdAction=%d",  *pFwdAction); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_illegalPortMoveAction_get*/
  

/* Function Name:
 *      dal_esw_l2_illegalPortMoveAction_set
 * Description:
 *      Set forwarding action when illegal port moving happen on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      fwdAction - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
int32
dal_esw_l2_illegalPortMoveAction_set(
    uint32              unit, 
    rtk_port_t          port, 
    rtk_action_t        fwdAction)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, fwdAction=%d",  
                    unit, port, fwdAction); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);   
    RT_PARAM_CHK((fwdAction >= ACTION_TO_GUESTVLAN), RT_ERR_FWD_ACTION); 

    /*Translate to asic port moving action*/
    if(fwdAction == ACTION_FORWARD)
        val = 0;
    else if(fwdAction == ACTION_COPY2CPU)
        val = 1;
    else if(fwdAction == ACTION_TRAP2CPU)
        val = 2;
    else
        val = 3; 
    
    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_write(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r, 
                    port, REG_ARRAY_INDEX_NONE, ESW_SML_ILLEGALMVACTf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);   

    return RT_ERR_OK;
}   /*end of dal_esw_l2_illegalPortMoveAction_set*/

/* Function Name:
 *      dal_esw_l2_legalPortMoveAction_get
 * Description:
 *      Get forwarding action when legal port moving happen on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pFwdAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
int32
dal_esw_l2_legalPortMoveAction_get(
    uint32              unit, 
    rtk_port_t          port, 
    rtk_action_t        *pFwdAction)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d",  
                    unit, port); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);   
    RT_PARAM_CHK((NULL == pFwdAction), RT_ERR_NULL_POINTER); 

    L2_SEM_LOCK(unit); 

    /*get port move legal action value*/
    if((ret = reg_array_field_read(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r, 
                        port, REG_ARRAY_INDEX_NONE, ESW_SML_LEGALMVACTf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit); 

    /*Translate to rtk common action*/
    if(val == 0)
        *pFwdAction = ACTION_FORWARD;
    else if(val == 1)
        *pFwdAction = ACTION_COPY2CPU;
    else if(val == 2)
        *pFwdAction = ACTION_TRAP2CPU;
    else
        *pFwdAction = ACTION_DROP; 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pFwdAction=%d",  *pFwdAction); 

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_esw_l2_legalPortMoveAction_set
 * Description:
 *      Set forwarding action when legal port moving happen on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      fwdAction - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
 int32
dal_esw_l2_legalPortMoveAction_set(
    uint32              unit, 
    rtk_port_t          port, 
    rtk_action_t        fwdAction)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, fwdAction=%d",  
                    unit, port, fwdAction); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);   
    RT_PARAM_CHK((fwdAction >= ACTION_TO_GUESTVLAN), RT_ERR_FWD_ACTION); 

    /*Translate to asic port moving action*/
    if(fwdAction == ACTION_FORWARD)
        val = 0;
    else if(fwdAction == ACTION_COPY2CPU)
        val = 1;
    else if(fwdAction == ACTION_TRAP2CPU)
        val = 2;
    else
        val = 3; 

    L2_SEM_LOCK(unit); 

    if((ret = reg_array_field_write(unit, ESW_PORT_LAYER2_MAC_ADDRESS_LEARNING_CONTROL0r, 
                        port, REG_ARRAY_INDEX_NONE, ESW_SML_LEGALMVACTf, &val)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);    

    return RT_ERR_OK;
}

/* Module Name    : L2                        */
/* Sub-module Name: Parameter for lookup miss */

/* Function Name:
 *      dal_esw_l2_lookupMissAction_get
 * Description:
 *      Get forwarding action when destination address lookup miss.
 * Input:
 *      unit    - unit id
 *      type    - type of lookup miss
 * Output:
 *      pAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid type of lookup miss
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 *
 *      Forwarding action is as following:
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_FLOOD_IN_VLAN
 *      - ACTION_FLOOD_IN_ALL_PORT  (only for DLF_TYPE_IPMC)
 */
 int32
dal_esw_l2_lookupMissAction_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t *pAction)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d",  unit, type); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type == DLF_TYPE_IP6MC), RT_ERR_INPUT);
    RT_PARAM_CHK((type == DLF_TYPE_ANY), RT_ERR_INPUT);
    RT_PARAM_CHK((type >= DLF_TYPE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER); 

    L2_SEM_LOCK(unit);

    switch(type)
    {
        case DLF_TYPE_IPMC:
            if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                            ESW_L3MMISSOPf, &val)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }
            
            /*Translate to rtk common action*/
            if(val == 0)
                *pAction = ACTION_DROP;
            else if(val == 1)
                *pAction = ACTION_TRAP2CPU;
            else if(val == 2)
                *pAction = ACTION_FLOOD_IN_VLAN; 
            else
                *pAction = ACTION_FLOOD_IN_ROUTER_PORTS;

            break;            
        case DLF_TYPE_UCAST:
            if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                            ESW_L2UMISSOPf, &val)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }
            
            /*Translate to rtk common action*/
            if(val == 0)
                *pAction = ACTION_DROP;
            else if(val == 1)
                *pAction = ACTION_TRAP2CPU;
            else
                *pAction = ACTION_FLOOD_IN_VLAN; 
            break;    
        case DLF_TYPE_MCAST:
            if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                            ESW_L2MMISSOPf, &val)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }
            
            /*Translate to rtk common action*/
            if(val == 0)
                *pAction = ACTION_DROP;
            else if(val == 1)
                *pAction = ACTION_TRAP2CPU;
            else if(val == 2)
                *pAction = ACTION_FLOOD_IN_VLAN; 
            else
                *pAction = ACTION_FLOOD_IN_ALL_PORT; 
            break;    
        case DLF_TYPE_BCAST:
            if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                            ESW_L2BMISSOPf, &val)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }
            
            /*Translate to rtk common action*/
            if(val == 0)
                *pAction = ACTION_DROP;
            else if(val == 1)
                *pAction = ACTION_TRAP2CPU;
            else
                *pAction = ACTION_FLOOD_IN_VLAN; 
            break;     
        default:
            break;
    }

    L2_SEM_UNLOCK(unit);    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAction=%d",  *pAction); 

    return RT_ERR_OK;
}/*end of dal_esw_l2_lookupMissAction_get*/

/* Function Name:
 *      dal_esw_l2_lookupMissAction_set
 * Description:
 *      Set forwarding action when destination address lookup miss.
 * Input:
 *      unit   - unit id
 *      type   - type of lookup miss
 *      action - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid type of lookup miss
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 *
 *      Forwarding action is as following:
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_FLOOD_IN_VLAN
 *      - ACTION_FLOOD_IN_ALL_PORT  (only for DLF_TYPE_IPMC)
 */
int32
dal_esw_l2_lookupMissAction_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t action)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d, action=%d",  unit, type, action); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type == DLF_TYPE_IP6MC), RT_ERR_INPUT);
    RT_PARAM_CHK((type == DLF_TYPE_ANY), RT_ERR_INPUT);
    RT_PARAM_CHK((type >= DLF_TYPE_END), RT_ERR_INPUT);   
    RT_PARAM_CHK((action >= ACTION_END), RT_ERR_NULL_POINTER); 

    L2_SEM_LOCK(unit);

    switch(type)
    {
        case DLF_TYPE_IPMC:
            /*Translate to rtk common action*/
            if(action == ACTION_DROP)
                val = 0;
            else if(action == ACTION_TRAP2CPU)
                val = 1;
            else if(action == ACTION_FLOOD_IN_VLAN)
                val = 2; 
            else if (action == ACTION_FLOOD_IN_ROUTER_PORTS)
                val = 3;
            else
            {
                L2_SEM_UNLOCK(unit);
                RT_ERR(RT_ERR_INPUT, (MOD_DAL|MOD_L2), "");
                return RT_ERR_INPUT;
            }    
            
            if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                            ESW_L3MMISSOPf, &val)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }  
            break;            
        case DLF_TYPE_UCAST:
             /*Translate to rtk common action*/
            if(action == ACTION_DROP)
                val = 0;
            else if(action == ACTION_TRAP2CPU)
                val = 1;
            else if(action == ACTION_FLOOD_IN_VLAN)
                val = 2; 
            else
            {
                L2_SEM_UNLOCK(unit);
                RT_ERR(RT_ERR_INPUT, (MOD_DAL|MOD_L2), "");
                return RT_ERR_INPUT;
            }
            
            if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                            ESW_L2UMISSOPf, &val)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }           
           
            break;    
        case DLF_TYPE_MCAST:
            /*Translate to rtk common action*/
            if(action == ACTION_DROP)
                val = 0;
            else if(action == ACTION_TRAP2CPU)
                val = 1;
            else if(action == ACTION_FLOOD_IN_VLAN)
                val = 2; 
            else if (action == ACTION_FLOOD_IN_ALL_PORT) /*should be flood in router ports*/ 
                val = 3;
            else
            {
                L2_SEM_UNLOCK(unit);
                RT_ERR(RT_ERR_INPUT, (MOD_DAL|MOD_L2), "");
                return RT_ERR_INPUT;
            }
            
            if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                            ESW_L2MMISSOPf, &val)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }
            
            break;    
        case DLF_TYPE_BCAST:
            /*Translate to rtk common action*/
            if(action == ACTION_DROP)
                val = 0;
            else if(action == ACTION_TRAP2CPU)
                val = 1;
            else if(action == ACTION_FLOOD_IN_VLAN)
                val = 2; 
            else
            {
                L2_SEM_UNLOCK(unit);
                RT_ERR(RT_ERR_INPUT, (MOD_DAL|MOD_L2), "");
                return RT_ERR_INPUT;
            }
            
            if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                            ESW_L2BMISSOPf, &val)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }
            break;     
        default:
            
            break;
    }

    L2_SEM_UNLOCK(unit);    

    return RT_ERR_OK;
}/*end of dal_esw_l2_lookupMissAction_set*/


/* Function Name:
 *      dal_esw_l2_lookupMissPri_get
 * Description:
 *      Get priority of trapped packet when destination address lookup miss.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 */
int32 
dal_esw_l2_lookupMissPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    int32   ret;     
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d",  unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER); 

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                    ESW_LMPRIf, pPriority)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pPriority=%d",  *pPriority); 

    return RT_ERR_OK;
}   /*end of dal_esw_l2_lookupMissPri_get*/


/* Function Name:
 *      dal_esw_l2_lookupMissPri_set
 * Description:
 *      Set priority of trapped packet when destination address lookup miss.
 * Input:
 *      unit     - unit id
 *      priority - priority of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 */
int32 
dal_esw_l2_lookupMissPri_set(uint32 unit, rtk_pri_t priority)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, priority=%d",
                    unit, priority); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_PRIORITY); 

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                    ESW_LMPRIf, &priority)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_lookupMissPri_set*/

/* Function Name:
 *      dal_esw_l2_lookupMissPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
dal_esw_l2_lookupMissPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);     

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                    ESW_DFRLMPRIf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
}   /*end of dal_esw_l2_lookupMissPriEnable_get*/

/* Function Name:
 *      dal_esw_l2_lookupMissPriEnable_set
 * Description:
 *      Set priority priority assignment status for trapped packet.
 * Input:
 *      unit     - unit id
 *      enable - priority assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
 int32
dal_esw_l2_lookupMissPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);      

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                    ESW_DFRLMPRIf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_lookupMissPriEnable_set*/


/* Function Name:
 *      dal_esw_l2_lookupMissDP_get
 * Description:
 *      Get drop precedence of trapped packet when destination address lookup miss.
 * Input:
 *      unit - unit id
 * Output:
 *      pDp  - pointer to drop precedence of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 */
int32 
dal_esw_l2_lookupMissDP_get(uint32 unit, uint32 *pDp)
{
    int32   ret;    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d",  unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pDp), RT_ERR_NULL_POINTER); 

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                    ESW_LMDPf, pDp)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pDp=%d",  *pDp); 

    return RT_ERR_OK;
}   /*end of dal_esw_l2_lookupMissDP_get*/


/* Function Name:
 *      dal_esw_l2_lookupMissDP_set
 * Description:
 *      Set drop precedence of trapped packet when destination address lookup miss.
 * Input:
 *      unit - unit id
 *      dp   - drop precedence of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 */
int32 
dal_esw_l2_lookupMissDP_set(uint32 unit, uint32 dp)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, dp=%d",
                    unit, dp); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((dp > 3), RT_ERR_PRIORITY); 

    L2_SEM_LOCK(unit);

    /*Set lookup miss dp value*/
    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                    ESW_LMDPf, &dp)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_lookupMissDP_set*/

/* Function Name:
 *      dal_esw_l2_lookupMissDPEnable_get
 * Description:
 *      Get drop procedence assignment status for trapped packet.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to drop procedence assignment status for trapped packet
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
dal_esw_l2_lookupMissDPEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);     

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                    ESW_DFRLMDPf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 

    return RT_ERR_OK;
}   /*end of dal_esw_l2_lookupMissDPEnable_get*/

/* Function Name:
 *      dal_esw_l2_lookupMissDPEnable_set
 * Description:
 *      Set drop procedence drop procedence assignment status for trapped packet.
 * Input:
 *      unit     - unit id
 *      enable - drop procedence assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid drop procedence value
 * Note:
 *      None
 */
 int32
dal_esw_l2_lookupMissDPEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);      

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                    ESW_DFRLMDPf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_lookupMissDPEnable_set*/


/* Function Name:
 *      dal_esw_l2_lookupMissAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
dal_esw_l2_lookupMissAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d",  unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER); 

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                    ESW_LMCPUTAGf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d",  *pEnable); 

    return RT_ERR_OK;
}   /*end of dal_esw_l2_lookupMissAddCPUTagEnable_get*/

/* Function Name:
 *      dal_esw_l2_lookupMissAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
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
dal_esw_l2_lookupMissAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d",  unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT); 

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_ADDRESS_TABLE_LOOKUP_MISS_CONTROLr, 
                    ESW_LMCPUTAGf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}   /*end of dal_esw_l2_lookupMissAddCPUTagEnable_set*/


/* Module Name    : L2                 */
/* Sub-module Name: Parameter for MISC */

/* Function Name:
 *      dal_esw_l2_srcPortEgrFilterMask_get
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
dal_esw_l2_srcPortEgrFilterMask_get(uint32 unit, rtk_portmask_t *pFilter_portmask)
{
    int32   ret;
    uint32 port, enable;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d",  unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pFilter_portmask), RT_ERR_NULL_POINTER);     

    L2_SEM_LOCK(unit);   

    osal_memset(pFilter_portmask, 0, sizeof(rtk_portmask_t));
    for(port = 0; port <= HAL_GET_MAX_PORT(unit); port++)
    {
        if((ret = reg_array_field_read(unit, ESW_PORT_FWD_SOURCE_PORT_FILTERr,
                        port, REG_ARRAY_INDEX_NONE, ESW_SPFTf, &enable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        if(enable)
            BITMAP_SET(pFilter_portmask->bits, port);
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pFilter_portmask=0x%x",  pFilter_portmask->bits[0]); 
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_srcPortEgrFilterMask_get*/

/* Function Name:
 *      dal_esw_l2_srcPortEgrFilterMask_set
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
dal_esw_l2_srcPortEgrFilterMask_set(uint32 unit, rtk_portmask_t *pFilter_portmask)
{
    int32   ret;
    uint32  port, val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d",  unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pFilter_portmask), RT_ERR_NULL_POINTER);   

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pFilter_portmask=0x%x",  pFilter_portmask->bits[0]); 

    L2_SEM_LOCK(unit);   

    for(port = 0; port <= HAL_GET_MAX_PORT(unit); port++)
    {
        if(BITMAP_IS_SET(pFilter_portmask->bits, port))
        {
            val = ENABLED;
            if((ret = reg_array_field_write(unit, ESW_PORT_FWD_SOURCE_PORT_FILTERr,
                            port, REG_ARRAY_INDEX_NONE, ESW_SPFTf, &val)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }
        }
        else
        {
            val = DISABLED;
            if((ret = reg_array_field_write(unit, ESW_PORT_FWD_SOURCE_PORT_FILTERr,
                           port, REG_ARRAY_INDEX_NONE, ESW_SPFTf, &val)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                return ret;
            }
        }
    }

    L2_SEM_UNLOCK(unit);   

    return RT_ERR_OK;    
}/*end of dal_esw_l2_srcPortEgrFilterMask_set*/

/* Function Name:
 *      dal_esw_l2_srcPortEgrFilterMask_add
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
dal_esw_l2_srcPortEgrFilterMask_add(uint32 unit, rtk_port_t filter_port)
{
    int32   ret;
    rtk_portmask_t filter_portmask;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, filter_port=%d",  unit, filter_port); 

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, filter_port), RT_ERR_PORT_ID);  

    if((ret = dal_esw_l2_srcPortEgrFilterMask_get(unit, &filter_portmask)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    BITMAP_SET(filter_portmask.bits, filter_port);

     if((ret = dal_esw_l2_srcPortEgrFilterMask_set(unit, &filter_portmask)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

     return RT_ERR_OK;
}/*end of dal_esw_l2_srcPortEgrFilterMask_add*/

/* Function Name:
 *      dal_esw_l2_srcPortEgrFilterMask_del
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
dal_esw_l2_srcPortEgrFilterMask_del(uint32 unit, rtk_port_t filter_port)
{
    int32   ret;
    rtk_portmask_t filter_portmask;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, filter_port=%d",  unit, filter_port); 

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, filter_port), RT_ERR_PORT_ID);  

    if((ret = dal_esw_l2_srcPortEgrFilterMask_get(unit, &filter_portmask)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    BITMAP_CLEAR(filter_portmask.bits, filter_port);

     if((ret = dal_esw_l2_srcPortEgrFilterMask_set(unit, &filter_portmask)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }
     return RT_ERR_OK;
}/*end of dal_esw_l2_srcPortEgrFilterMask_del*/



/* Function Name:
 *      dal_esw_l2_exceptionAddrAction_get
 * Description:
 *      Get forwarding action of packet with exception address.
 * Input:
 *      unit       - unit id
 *      exceptType - type of exception address
 * Output:
 *      pAction    - pointer to forward action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_EXCEPT_ADDR_TYPE  - invalid exception address type
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Exception address type is as following
 *      - SA_IS_MCAST
 *      - SA_IS_BCAST
 *      - SA_IS_ZERO
 *
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
int32
dal_esw_l2_exceptionAddrAction_get(
    uint32                          unit, 
    rtk_l2_exceptionAddrType_t      exceptType, 
    rtk_action_t                    *pAction)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, exceptType=%d",  unit, exceptType); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((exceptType == SA_IS_BCAST_OR_MCAST), RT_ERR_L2_EXCEPT_ADDR_TYPE);
    RT_PARAM_CHK((exceptType >= EXCEPT_ADDR_TYPE_END), RT_ERR_L2_EXCEPT_ADDR_TYPE);   
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);   

    L2_SEM_LOCK(unit);

    if(exceptType == SA_IS_MCAST)
    {
        if((ret = reg_field_read(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, ESW_MLTSA_DROPf, pAction)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else if(exceptType == SA_IS_BCAST)
    {
        if((ret = reg_field_read(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, ESW_BROSA_DROPf, pAction)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_read(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, ESW_ZEROSA_DROPf, pAction)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAction=%d",  *pAction); 
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_exceptionAddrAction_get*/

/* Function Name:
 *      dal_esw_l2_exceptionAddrAction_set
 * Description:
 *      Set forwarding action of packet with exception address.
 * Input:
 *      unit       - unit id
 *      exceptType - type of exception address
 *      action     - forward action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_EXCEPT_ADDR_TYPE  - invalid exception address type
 *      RT_ERR_FWD_ACTION           - invalid forwarding action
 * Note:
 *      Exception address type is as following
 *      - SA_IS_MCAST
 *      - SA_IS_BCAST
 *      - SA_IS_ZERO
 *
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
int32
dal_esw_l2_exceptionAddrAction_set(
    uint32                          unit, 
    rtk_l2_exceptionAddrType_t      exceptType, 
    rtk_action_t                    action)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, exceptType=%d, action=%d",  unit, exceptType, action); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((exceptType == SA_IS_BCAST_OR_MCAST), RT_ERR_L2_EXCEPT_ADDR_TYPE);
    RT_PARAM_CHK((exceptType >= EXCEPT_ADDR_TYPE_END), RT_ERR_L2_EXCEPT_ADDR_TYPE);   
    RT_PARAM_CHK((action > ACTION_DROP), RT_ERR_FWD_ACTION);   

    L2_SEM_LOCK(unit);

    if(exceptType == SA_IS_MCAST)
    {
        if((ret = reg_field_write(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, ESW_MLTSA_DROPf, &action)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else if(exceptType == SA_IS_BCAST)
    {
        if((ret = reg_field_write(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, ESW_BROSA_DROPf, &action)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if((ret = reg_field_write(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, ESW_ZEROSA_DROPf, &action)) != RT_ERR_OK)
         {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}   /*end of dal_esw_l2_exceptionAddrAction_set*/

/* Function Name:
 *      dal_esw_l2_trapPri_get
 * Description:
 *      Get priority of trapped packet.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority
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
dal_esw_l2_trapPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d",  unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);   

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, 
                    ESW_SMLPRIf, pPriority)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pPriority=%d", *pPriority); 
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_trapPri_get*/


/* Function Name:
 *      dal_esw_l2_trapPri_set
 * Description:
 *      Set priority of trapped packet.
 * Input:
 *      unit     - unit id
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
int32
dal_esw_l2_trapPri_set(uint32 unit, rtk_pri_t priority)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, priority=%d", unit, priority); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_PRIORITY);   

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, 
                    ESW_SMLPRIf, &priority)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_trapPri_set*/


/* Function Name:
 *      dal_esw_l2_trapPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
dal_esw_l2_trapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d",  unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);     

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, 
                    ESW_SMLDFPRIf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d",  *pEnable); 
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_trapPriEnable_get*/

/* Function Name:
 *      dal_esw_l2_trapPriEnable_set
 * Description:
 *      Set priority priority assignment status for trapped packet.
 * Input:
 *      unit     - unit id
 *      enable - priority assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
 int32
dal_esw_l2_trapPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);      
    
    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, 
                    ESW_SMLDFPRIf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_trapPriEnable_set*/


/* Function Name:
 *      dal_esw_l2_trapAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
dal_esw_l2_trapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d",  unit); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);   

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, 
                    ESW_SMLCPUTAGf, pEnable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable); 
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_trapAddCPUTagEnable_get*/


/* Function Name:
 *      dal_esw_l2_trapAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
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
dal_esw_l2_trapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable); 

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);   

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, ESW_SOURCE_MAC_LEARNING_CONTROLr, 
                    ESW_SMLCPUTAGf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;    
}/*end of dal_esw_l2_trapAddCPUTagEnable_set*/

/* Function Name:
 *      dal_esw_l2_addrEntry_get
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
 *      (1) The index valid range is from 0 to (L2 hash table size + L2 CAM table size - 1)
 *          - 0 ~ (L2 hash table size - 1) entry in L2 hash table
 *          - (L2 hash table size) ~ (L2 hash table size + L2 CAM table size - 1) entry in L2 CAM table
 *      (2) The output entry have 2 variables (valid and entry_type) and its detail data structure
 *          - valid: 1 mean the entry is valid; 0: invalid
 *          - entry_type: FLOW_TYPE_UNICAST, FLOW_TYPE_L2_MULTI and FLOW_TYPE_IP_MULTI
 *                        the field is ignored if valid field is 0.
 *          - detail data structure is ignored if valid is 0, and its filed meanings is depended
 *            on the entry_type value.
 *      (3) If pL2_entry->flags have enabled the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the
 *          pL2_entry->unicast.trk_gid is valid trunk id value.
 */
int32
dal_esw_l2_addrEntry_get(uint32 unit, uint32 index, rtk_l2_entry_t *pL2_entry)
{
    uint32  l2_tableSize, l2cam_tableSize;
    uint32  hashkey = 0, location = 0, isValid = 0, l2cam_idx = 0;
    uint32  value = 0;
    int32   ret;
    dal_esw_l2_entry_t  l2_entry;
    multicast_index_entry_t mcast_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;
    rtk_trunk_mode_t mode;
    rtk_port_t resperentPort;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", unit, index);
    
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* check index valid range */
    if ((ret = table_size_get(unit, ESW_CAM_ISFTIDXt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
        return ret;
    }
    if ((ret = table_size_get(unit, ESW_L2_ISFTIDXt, &l2_tableSize)) != RT_ERR_OK)
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
        if ((ret = _dal_esw_l2_getL2EntryfromHash(unit, hashkey, location, &l2_entry, &isValid)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
    }
    else
    {   /* L2 CAM Table Entry */
        l2cam_idx = index & 0xFFF;
        if ((ret = _dal_esw_l2_getL2EntryfromCAM(unit, l2cam_idx, &l2_entry, &isValid)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
            return ret;
        }
    }

    /* Transfer from dal_esw_l2_entry_t to rtk_l2_entry_t */
    pL2_entry->valid = l2_entry.valid;
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
                if((ret = dal_esw_trunk_mode_get(unit, &mode)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "Get trunk mode failed");  
                    return ret;
                }
            
                for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
                {
                    if (dal_esw_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
                    {
                        if ((first_trunkMember = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
                        {
                            /* no trunk member */
                            continue;
                        }
            
                        /*dumbmode*/
                        if(mode == TRUNK_MODE_DUMB)
                        {
                            if ((RTK_PORTMASK_IS_PORT_SET(trunk_portmask, pL2_entry->unicast.port)) &&
                                (first_trunkMember == pL2_entry->unicast.port))
                            {
                                pL2_entry->unicast.trk_gid = trk_gid;
                                pL2_entry->unicast.flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
                                break;
                            }
                        }
                        else    /*Normal mode*/
                        {
                            if(dal_esw_trunk_representPort_get(unit, trk_gid, &resperentPort) != RT_ERR_OK)
                            {                  
                                L2_SEM_UNLOCK(unit);  
                                RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "Get represent port number");
                                return RT_ERR_PORT_ID;
                            }
            
                            if ((resperentPort == pL2_entry->unicast.port))
                            {
                                pL2_entry->unicast.trk_gid = trk_gid;
                                pL2_entry->unicast.flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
                                break;
                            }
                        } 
                    }
                }
                if(l2_entry.unicast.sablock)
                    pL2_entry->unicast.flags |= RTK_L2_UCAST_FLAG_SA_BLOCK;
                if(l2_entry.unicast.dablock)
                    pL2_entry->unicast.flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
                if(l2_entry.unicast.is_static)
                    pL2_entry->unicast.flags |= RTK_L2_UCAST_FLAG_STATIC;
                if(l2_entry.unicast.nh)
                    pL2_entry->unicast.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
                if(l2_entry.unicast.suspend)
                    pL2_entry->unicast.state |= RTK_L2_UCAST_STATE_SUSPEND;
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
                if ((ret = table_read(unit, ESW_FORWARDINGt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, pL2_entry->l2mcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &pL2_entry->l2mcast.crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_AGINGf, &value, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                if(value)
                    pL2_entry->l2mcast.isAged = TRUE;
                else
                    pL2_entry->l2mcast.isAged = FALSE;
                break;
    
            case FLOW_TYPE_IP4_MULTI:
                pL2_entry->ipmcast.rvid = 0;
                pL2_entry->ipmcast.dip = l2_entry.ipmcast.dip;
                pL2_entry->ipmcast.sip = l2_entry.ipmcast.sip;
                pL2_entry->ipmcast.fwdIndex = l2_entry.ipmcast.index;
                osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                if ((ret = table_read(unit, ESW_FORWARDINGt, l2_entry.ipmcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, pL2_entry->ipmcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &pL2_entry->ipmcast.crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_AGINGf, &value, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    L2_SEM_UNLOCK(unit);
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                if(value)
                    pL2_entry->ipmcast.isAged = TRUE;
                else
                    pL2_entry->ipmcast.isAged = FALSE;
                break;
    
            default:
                L2_SEM_UNLOCK(unit);
                return RT_ERR_FAILED;
        }
    }        

    L2_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of dal_esw_l2_addrEntry_get */

/* Function Name:
 *      dal_esw_l2_limitLearningCnt_workaround
 * Description:
 *      Delete all L2 unicast address entry which aging bit is less or equal 3 from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the mac limiting count wrong problem
 */
int32
dal_esw_l2_limitLearningCnt_workaround(uint32 unit)
{
    int32   ret = RT_ERR_FAILED;
    int32   scan_idx;
    dal_esw_l2_entry_t  l2_entry;
    rtk_vlan_t  vid;
    rtk_mac_t   mac;

    scan_idx = -1;
    while(1)
    {
        if ((ret = _dal_esw_l2_nextValidAddr_get(unit, &scan_idx, L2_UNICAST, 0, &l2_entry)) != RT_ERR_OK)
        {
            return ret;
        }

        if (3 >= l2_entry.unicast.aging) /*  == 1 */
        {
            vid = l2_entry.unicast.fid;
            osal_memcpy(&mac.octet[0], &l2_entry.unicast.mac.octet[0], sizeof(rtk_mac_t));
            if ((ret = dal_esw_l2_addr_del(unit, vid, &mac)) != RT_ERR_OK)
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "dal_esw_l2_addr_del failed - unit=%d, vid=%d", unit, vid);
            }
        }
    }

    return RT_ERR_OK;
} /* end of dal_esw_l2_limitLearningCnt_workaround */

/* Function Name:
 *      dal_esw_l2_limitLearningCntByRange_workaround
 * Description:
 *      Delete all L2 unicast address entry which aging bit is less or equal 3 from the specified device.
 * Input:
 *      unit      - unit id
 *      start_idx - start index
 *      end_idx   - end index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the mac limiting count wrong problem
 */
int32
dal_esw_l2_limitLearningCntByRange_workaround(uint32 unit, uint32 start_idx, uint32 end_idx)
{
    int32   ret = RT_ERR_FAILED;
    int32   scan_idx;
    dal_esw_l2_entry_t  l2_entry;
    rtk_vlan_t  vid;
    rtk_mac_t   mac;

    if (start_idx == 0)
        scan_idx = -1;
    else
        scan_idx = start_idx - 1;

    while(1)
    {
        if ((ret = _dal_esw_l2_nextValidAddrByRange_get(unit, &scan_idx, end_idx, L2_UNICAST, 0, &l2_entry)) != RT_ERR_OK)
        {
            return ret;
        }

        if (3 >= l2_entry.unicast.aging) /* == 1 */
        {
            vid = l2_entry.unicast.fid;
            osal_memcpy(&mac.octet[0], &l2_entry.unicast.mac.octet[0], sizeof(rtk_mac_t));
            if ((ret = dal_esw_l2_addr_del(unit, vid, &mac)) != RT_ERR_OK)
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "dal_esw_l2_addr_del failed - unit=%d, vid=%d", unit, vid);
            }
        }
    }

    return RT_ERR_OK;
} /* end of dal_esw_l2_limitLearningCntByRange_workaround */

/* Function Name:
 *      dal_esw_l2_forwardTablePortmask_workaround
 * Description:
 *      Rewrite the all forward table from shadow to chip in the specified device.
 * Input:
 *      unit      - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the forward table portmask problem
 */
int32
dal_esw_l2_forwardTablePortmask_workaround(uint32 unit)
{
#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
    if (CHIP_REV_ID_A == HAL_GET_CHIP_REV_ID(unit) || CHIP_REV_ID_B == HAL_GET_CHIP_REV_ID(unit))
    {
        int32   ret = RT_ERR_FAILED;
        uint32  index;
        multicast_index_entry_t mcast_entry;
    
        for(index = 0; index < mcast_idx_pool[unit].size_of_mcast_fwd_index; index++)
        {
            if(mcast_idx_pool[unit].pMcast_index_pool[index].entry_state == ENTRY_STATE_BACKUP || 
               mcast_idx_pool[unit].pMcast_index_pool[index].entry_state == ENTRY_STATE_INUSED)
            {
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Rewrite fwd index:%d\n", index);
                osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
                if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, 
                    &mcast_idx_pool[unit].pMcast_index_pool[index].portmask.bits[0], (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
    
                if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, 
                    &mcast_idx_pool[unit].pMcast_index_pool[index].crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                
                if ((ret = table_write(unit, ESW_FORWARDINGt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
            }
        }
    }
    else
    {
        int32   ret = RT_ERR_FAILED;
        uint32  index;
        multicast_index_entry_t mcast_entry;
    
        for(index = 0; index < mcast_idx_pool[unit].size_of_mcast_fwd_index; index++)
        {
            if(mcast_idx_pool[unit].pMcast_index_pool[index].ref_count > 0 )
            {
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Rewrite fwd index:%d\n", index);
                osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
    
                if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, 
                    &mcast_idx_pool[unit].pMcast_index_pool[index].portmask.bits[0], (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
    
                if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, 
                    &mcast_idx_pool[unit].pMcast_index_pool[index].crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
                
                if ((ret = table_write(unit, ESW_FORWARDINGt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "");  
                    return ret;
                }
            }
        }
    }
#else
    int32   ret = RT_ERR_FAILED;
    uint32  index;
    multicast_index_entry_t mcast_entry;

    for(index = 0; index < mcast_idx_pool[unit].size_of_mcast_fwd_index; index++)
    {
        if(mcast_idx_pool[unit].pMcast_index_pool[index].ref_count > 0 )
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Rewrite fwd index:%d\n", index);
            osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

            if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, 
                &mcast_idx_pool[unit].pMcast_index_pool[index].portmask.bits[0], (uint32 *) &mcast_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "");  
                return ret;
            }

            if ((ret = table_field_set(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, 
                &mcast_idx_pool[unit].pMcast_index_pool[index].crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
            
            if ((ret = table_write(unit, ESW_FORWARDINGt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "");  
                return ret;
            }
        }
    }
#endif

    return RT_ERR_OK;
} /* end of dal_esw_l2_forwardTablePortmask_workaround */

#if defined(CONFIG_SDK_WA_FORWARD_TABLE)
int32
_dal_esw_l2_mcastFwdIndex_arrange(uint32 unit, int32 index)
{
    uint32  l2_index, val, i;
    int32   ret = RT_ERR_FAILED;
    int32   free_index, move_index;
    multicast_index_entry_t mcast_entry;
    l2_entry_t l2_entry;
    l2cam_entry_t l2cam_entry;
    
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", unit, index);

    RT_PARAM_CHK((index >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index) || index < 0, RT_ERR_L2_MULTI_FWD_INDEX);
    
    if (fwd_entry_inused_count >= mcast_idx_pool[unit].size_of_mcast_fwd_index/2)
    {
        free_index = index;
        if ((free_index%2) == 0)
        {
            /* free_index become backup entry of free_index + 1 */
            osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
            if ((ret = table_read(unit, ESW_FORWARDINGt, (uint32)(free_index + 1), (uint32 *)&mcast_entry)) != RT_ERR_OK)
            {
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                return RT_ERR_FAILED;
            }
            if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(free_index), (uint32 *)&mcast_entry)) != RT_ERR_OK)
            {
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                return RT_ERR_FAILED;
            }
            mcast_idx_pool[unit].pMcast_index_pool[free_index].ref_count = mcast_idx_pool[unit].pMcast_index_pool[free_index+1].ref_count;
            mcast_idx_pool[unit].pMcast_index_pool[free_index].crossVlan = mcast_idx_pool[unit].pMcast_index_pool[free_index+1].crossVlan;
            osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[free_index].portmask, &mcast_idx_pool[unit].pMcast_index_pool[free_index+1].portmask, sizeof(rtk_portmask_t));
            osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index, &mcast_idx_pool[unit].pMcast_index_pool[free_index+1].l2_index, sizeof(dal_esw_l2_index_t));
            mcast_idx_pool[unit].pMcast_index_pool[free_index].entry_state = ENTRY_STATE_BACKUP;
        }
        else
        {
            if (mcast_idx_pool[unit].pMcast_index_pool[free_index-1].entry_state == ENTRY_STATE_AVAILABLE)
            {
                move_index = -1;
                for (i = 0; i < mcast_idx_pool[unit].size_of_mcast_fwd_index; i=i+2)
                {
                    if (mcast_idx_pool[unit].pMcast_index_pool[i].entry_state == ENTRY_STATE_INUSED)
                    {
                        move_index = i;
                        break;
                    }
                }
                if (i == mcast_idx_pool[unit].size_of_mcast_fwd_index || move_index == -1)
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed due to free_index=%d can't find move index", free_index);
                    return RT_ERR_FAILED;
                }
                osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                if ((ret = table_read(unit, ESW_FORWARDINGt, (uint32)(move_index), (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                    return RT_ERR_FAILED;
                }
                if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(free_index-1), (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                    return RT_ERR_FAILED;
                }
                mcast_idx_pool[unit].pMcast_index_pool[free_index-1].ref_count = mcast_idx_pool[unit].pMcast_index_pool[move_index].ref_count;
                mcast_idx_pool[unit].pMcast_index_pool[free_index-1].crossVlan = mcast_idx_pool[unit].pMcast_index_pool[move_index].crossVlan;
                osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[free_index-1].portmask, &mcast_idx_pool[unit].pMcast_index_pool[move_index].portmask, sizeof(rtk_portmask_t));
                osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[free_index-1].l2_index, &mcast_idx_pool[unit].pMcast_index_pool[move_index].l2_index, sizeof(dal_esw_l2_index_t));
                mcast_idx_pool[unit].pMcast_index_pool[free_index-1].entry_state = ENTRY_STATE_BACKUP;
                if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(free_index), (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                    return RT_ERR_FAILED;
                }
                mcast_idx_pool[unit].pMcast_index_pool[free_index].ref_count = mcast_idx_pool[unit].pMcast_index_pool[move_index].ref_count;
                mcast_idx_pool[unit].pMcast_index_pool[free_index].crossVlan = mcast_idx_pool[unit].pMcast_index_pool[move_index].crossVlan;
                osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[free_index].portmask, &mcast_idx_pool[unit].pMcast_index_pool[move_index].portmask, sizeof(rtk_portmask_t));
                osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index, &mcast_idx_pool[unit].pMcast_index_pool[move_index].l2_index, sizeof(dal_esw_l2_index_t));
                mcast_idx_pool[unit].pMcast_index_pool[free_index].entry_state = ENTRY_STATE_INUSED;

                if (mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.index_type == L2_IN_HASH)
                {
                    osal_memset(&l2_entry, 0, sizeof(l2_entry));
                    l2_index = (mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.hashdepth << 12) | mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.index;
                    /* read entry from chip */
                    if ((ret = table_read(unit, ESW_L2_ISFTIDXt, l2_index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    val = free_index;
                    if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_FTIDXf, &val, (uint32 *)&l2_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_write(unit, ESW_L2_ISFTIDXt, l2_index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                }
                else if (mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.index_type == L2_IN_CAM)
                {
                    osal_memset(&l2cam_entry, 0, sizeof(l2cam_entry_t));
                    l2_index = ~(CAM << ACCADDR_L2TYPE_OFFSET) & mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.index;
                    /* read entry from chip */
                    if ((ret = table_read(unit, ESW_CAM_ISFTIDXt, l2_index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    val = free_index;
                    if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_FTIDXf, &val, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_write(unit, ESW_CAM_ISFTIDXt, l2_index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                }
                else
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Error due to index=%d index_type [%d] invalid", free_index, mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.index_type);
                    return RT_ERR_FAILED;
                }

                if (mcast_idx_pool[unit].pMcast_index_pool[move_index+1].entry_state == ENTRY_STATE_INUSED)
                {
                    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                    if ((ret = table_read(unit, ESW_FORWARDINGt, (uint32)(move_index+1), (uint32 *)&mcast_entry)) != RT_ERR_OK)
                    {
                        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                        return RT_ERR_FAILED;
                    }
                    if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(move_index), (uint32 *)&mcast_entry)) != RT_ERR_OK)
                    {
                        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                        return RT_ERR_FAILED;
                    }

                    mcast_idx_pool[unit].pMcast_index_pool[move_index].ref_count = mcast_idx_pool[unit].pMcast_index_pool[move_index+1].ref_count;
                    mcast_idx_pool[unit].pMcast_index_pool[move_index].crossVlan = mcast_idx_pool[unit].pMcast_index_pool[move_index+1].crossVlan;
                    osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[move_index].portmask, &mcast_idx_pool[unit].pMcast_index_pool[move_index+1].portmask, sizeof(rtk_portmask_t));
                    osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[move_index].l2_index, &mcast_idx_pool[unit].pMcast_index_pool[move_index+1].l2_index, sizeof(dal_esw_l2_index_t));
                    mcast_idx_pool[unit].pMcast_index_pool[move_index].entry_state = ENTRY_STATE_BACKUP;
                }
                else
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Error due to move_index=%d next entry is not inused state[value: %d]", move_index, mcast_idx_pool[unit].pMcast_index_pool[move_index+1].entry_state);
                    return RT_ERR_FAILED;
                }
            }
            else if (mcast_idx_pool[unit].pMcast_index_pool[free_index-1].entry_state == ENTRY_STATE_INUSED)
            {
                osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                if ((ret = table_read(unit, ESW_FORWARDINGt, (uint32)(free_index - 1), (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                    return RT_ERR_FAILED;
                }
                if ((ret = table_write(unit, ESW_FORWARDINGt, (uint32)(free_index), (uint32 *)&mcast_entry)) != RT_ERR_OK)
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);  
                    return RT_ERR_FAILED;
                }
                mcast_idx_pool[unit].pMcast_index_pool[free_index].ref_count = mcast_idx_pool[unit].pMcast_index_pool[free_index-1].ref_count;
                mcast_idx_pool[unit].pMcast_index_pool[free_index].crossVlan = mcast_idx_pool[unit].pMcast_index_pool[free_index-1].crossVlan;
                osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[free_index].portmask, &mcast_idx_pool[unit].pMcast_index_pool[free_index-1].portmask, sizeof(rtk_portmask_t));
                osal_memcpy(&mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index, &mcast_idx_pool[unit].pMcast_index_pool[free_index-1].l2_index, sizeof(dal_esw_l2_index_t));

                if (mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.index_type == L2_IN_HASH)
                {
                    osal_memset(&l2_entry, 0, sizeof(l2_entry));
                    l2_index = (mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.hashdepth << 12) | mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.index;
                    /* read entry from chip */
                    if ((ret = table_read(unit, ESW_L2_ISFTIDXt, l2_index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    val = free_index;
                    if ((ret = table_field_set(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_FTIDXf, &val, (uint32 *)&l2_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_write(unit, ESW_L2_ISFTIDXt, l2_index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                }
                else if (mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.index_type == L2_IN_CAM)
                {
                    osal_memset(&l2cam_entry, 0, sizeof(l2cam_entry_t));
                    l2_index = ~(CAM << ACCADDR_L2TYPE_OFFSET) & mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.index;
                    /* read entry from chip */
                    if ((ret = table_read(unit, ESW_CAM_ISFTIDXt, l2_index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    val = free_index;
                    if ((ret = table_field_set(unit, ESW_CAM_ISFTIDXt, ESW_CAM_ISFTIDX_FTIDXf, &val, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_write(unit, ESW_CAM_ISFTIDXt, l2_index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                }
                else
                {
                    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Error due to index=%d index_type [%d] invalid", free_index, mcast_idx_pool[unit].pMcast_index_pool[free_index].l2_index.index_type);
                    return RT_ERR_FAILED;
                }

                mcast_idx_pool[unit].pMcast_index_pool[free_index].entry_state = ENTRY_STATE_INUSED;
                mcast_idx_pool[unit].pMcast_index_pool[free_index-1].entry_state = ENTRY_STATE_BACKUP;
            }
            else
            {
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Error due to index=%d state is %d", free_index-1, mcast_idx_pool[unit].pMcast_index_pool[free_index-1].entry_state);
                return RT_ERR_FAILED;
            }
        }
    }
    
    return RT_ERR_OK;
} /* end of _dal_esw_l2_mcastFwdIndex_arrange */
#endif

/* Function Name:
 *      dal_esw_l2_conflictAddr_get
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
dal_esw_l2_conflictAddr_get(
    uint32          unit,
    rtk_l2_entry_t  *pL2Addr,
    rtk_l2_entry_t  *pCfAddrList,
    uint32          cfAddrList_size,
    uint32          *pCf_retCnt)
{
    uint32  hash_key = 0, hash_depth;
    uint32  isValid, value;
    uint32  cf_num;
    uint32  mode, trk_gid, first_trunkMember, resperentPort;
    rtk_portmask_t  trunk_portmask;
    int32   ret = RT_ERR_FAILED;
    dal_esw_l2_entry_t  l2_entry;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    /* check input parameters */
    RT_PARAM_CHK(NULL == pL2Addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pCfAddrList, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(0 == cfAddrList_size, RT_ERR_INPUT);
    RT_PARAM_CHK(NULL == pCf_retCnt, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pL2Addr->entry_type >= FLOW_TYPE_END, RT_ERR_OUT_OF_RANGE);

    osal_memset(&l2_entry, 0, sizeof(dal_esw_l2_entry_t));
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
    {   /* FLOW_TYPE_IP_MULTI */
        l2_entry.entry_type = FLOW_TYPE_IP4_MULTI;
        l2_entry.ipmcast.dip = pL2Addr->ipmcast.dip;
        l2_entry.ipmcast.sip = pL2Addr->ipmcast.sip;
    }

    _dal_esw_l2_entryToHashKey(unit, &l2_entry, &hash_key);
    cf_num = 0;
    for (hash_depth = 0; hash_depth < HAL_L2_HASHDEPTH(unit) && cf_num < cfAddrList_size; hash_depth++)
    {
        osal_memset(&l2_entry, 0, sizeof(dal_esw_l2_entry_t));
        isValid = 0;
        if (_dal_esw_l2_getL2EntryfromHash(unit, hash_key, hash_depth, &l2_entry, &isValid) != RT_ERR_OK)
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

                    if((ret = dal_esw_trunk_mode_get(unit, &mode)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "Get trunk mode failed");  
                        return ret;
                    }
                
                    for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
                    {
                        if (dal_esw_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
                        {
                            if ((first_trunkMember = RTK_PORTMASK_GET_FIRST_PORT(trunk_portmask)) == -1)
                            {
                                /* no trunk member */
                                continue;
                            }
                
                            /*dumbmode*/
                            if(mode == TRUNK_MODE_DUMB)
                            {
                                if ((RTK_PORTMASK_IS_PORT_SET(trunk_portmask, (pCfAddrList + cf_num)->unicast.port)) &&
                                    (first_trunkMember == (pCfAddrList + cf_num)->unicast.port))
                                {
                                    (pCfAddrList + cf_num)->unicast.trk_gid = trk_gid;
                                    (pCfAddrList + cf_num)->unicast.flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
                                    break;
                                }
                            }
                            else    /*Normal mode*/
                            {
                                if(dal_esw_trunk_representPort_get(unit, trk_gid, &resperentPort) != RT_ERR_OK)
                                {                  
                                    RT_ERR(RT_ERR_PORT_ID, (MOD_DAL|MOD_L2), "Get represent port number");
                                    return RT_ERR_PORT_ID;
                                }
                
                                if ((resperentPort == (pCfAddrList + cf_num)->unicast.port))
                                {
                                    (pCfAddrList + cf_num)->unicast.trk_gid = trk_gid;
                                    (pCfAddrList + cf_num)->unicast.flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
                                    break;
                                }
                            } 
                        }
                    }
                    if(l2_entry.unicast.sablock)
                        (pCfAddrList + cf_num)->unicast.flags |= RTK_L2_UCAST_FLAG_SA_BLOCK;
                    if(l2_entry.unicast.dablock)
                        (pCfAddrList + cf_num)->unicast.flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
                    if(l2_entry.unicast.is_static)
                        (pCfAddrList + cf_num)->unicast.flags |= RTK_L2_UCAST_FLAG_STATIC;
                    if(l2_entry.unicast.nh)
                        (pCfAddrList + cf_num)->unicast.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
                    if(l2_entry.unicast.suspend)
                        (pCfAddrList + cf_num)->unicast.state |= RTK_L2_UCAST_STATE_SUSPEND;
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
                    if ((ret = table_read(unit, ESW_FORWARDINGt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, (pCfAddrList + cf_num)->l2mcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &(pCfAddrList + cf_num)->l2mcast.crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_AGINGf, &value, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if(value)
                        (pCfAddrList + cf_num)->l2mcast.isAged = TRUE;
                    else
                        (pCfAddrList + cf_num)->l2mcast.isAged = FALSE;
                    break;
        
                case FLOW_TYPE_IP4_MULTI:
                    (pCfAddrList + cf_num)->ipmcast.rvid = 0;
                    (pCfAddrList + cf_num)->ipmcast.dip = l2_entry.ipmcast.dip;
                    (pCfAddrList + cf_num)->ipmcast.sip = l2_entry.ipmcast.sip;
                    (pCfAddrList + cf_num)->ipmcast.fwdIndex = l2_entry.ipmcast.index;
                    (pCfAddrList + cf_num)->ipmcast.l2_idx = (hash_key << 2) | hash_depth;
                    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                    if ((ret = table_read(unit, ESW_FORWARDINGt, l2_entry.ipmcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_PMf, (pCfAddrList + cf_num)->ipmcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_CRSVLANf, &(pCfAddrList + cf_num)->ipmcast.crossVlan, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if ((ret = table_field_get(unit, ESW_FORWARDINGt, ESW_FORWARDING_AGINGf, &value, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                    {
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");  
                        return ret;
                    }
                    if(value)
                        (pCfAddrList + cf_num)->ipmcast.isAged = TRUE;
                    else
                        (pCfAddrList + cf_num)->ipmcast.isAged = FALSE;
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
} /* end of dal_esw_l2_conflictAddr_get */

