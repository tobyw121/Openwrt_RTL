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
 * $Revision: 54452 $
 * $Date: 2014-12-30 14:41:26 +0800 (Tue, 30 Dec 2014) $
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
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <ioal/mem32.h>
#include <osal/cache.h>
#include <osal/isr.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/time.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/cypress/rtk_cypress_table_struct.h>
#include <hal/chipdef/cypress/rtk_cypress_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <dal/cypress/dal_cypress_trunk.h>
#include <dal/cypress/dal_cypress_l2.h>
#include <dal/cypress/dal_cypress_l3.h>
#include <dal/cypress/dal_cypress_trap.h>
#include <dal/cypress/dal_cypress_vlan.h>
#include <drv/swcore/rtl8390.h>
#include <drv/swcore/l2notification.h>
#include <rtk/default.h>
#include <rtk/l2.h>
#include <rtk/l3.h>
#include <rtk/trap.h>

/*
 * Symbol Definition
 */

#define END_OF_MCAST_IDX    (0xFFFF)
#define MCAST_IDX_ALLOCATED (0xFFFE)
#define PORT_DONT_CARE      (0X3F)

#ifndef SRAM
#define SRAM 0
#endif

#ifndef CAM
#define CAM 1
#endif


typedef enum dal_cypress_l2_getMethod_e
{
    L2_GET_EXIST_ONLY = 0,
    L2_GET_EXIST_OR_FREE,
    L2_GET_FREE_ONLY,
    DAL_CYPRESS_GETMETHOD_END
} dal_cypress_l2_getMethod_t;

typedef struct dal_cypress_mcast_index_s {
    uint16  next_index;
    uint16  ref_count;
} dal_cypress_mcast_index_t;

typedef struct dal_cypress_mcast_index_pool_s {
    dal_cypress_mcast_index_t   *pMcast_index_pool;
    uint32                      size_of_mcast_fwd_index;
    uint16                      free_index_head;
    uint16                      free_entry_count;
} dal_cypress_mcast_index_pool_t;

typedef struct dal_cypress_ip6_cache_entry_s {
    uint16          rvid;
    rtk_ipv6_addr_t dip;
    rtk_ipv6_addr_t sip;
    uint8           valid;
} dal_cypress_ip6_cache_entry_t;

typedef struct dal_cypress_l2_info_s {
    uint8   limitLearningCnt_enable[RTK_MAX_NUM_OF_PORTS];
    uint32  limitLearningCnt[RTK_MAX_NUM_OF_PORTS];
} dal_cypress_l2_info_t;

typedef struct dal_cypress_l2_mcastFwdShadow_s
{
    rtk_portmask_t  portmask;
} dal_cypress_l2_mcastFwdShadow_t;


/*
 * Data Declaration
 */
static uint32               l2_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         l2_sem[RTK_MAX_NUM_OF_UNIT];
static uint32               algoType[RTK_MAX_NUM_OF_UNIT] = {0};
static uint32               ip4HashFmt[RTK_MAX_NUM_OF_UNIT] = {0};
static uint32               ip6HashFmt[RTK_MAX_NUM_OF_UNIT] = {0};
static uint32               ip6SipCareByte;
static uint32               ip6DipCareByte;
static uint8                ucst_dlf_pmsk_inited;
static uint8                bcst_dlf_pmsk_inited;
static rtk_enable_t         l2CamEbl;
static uint8                ipmcst_fvid_cmp;
static int16                ip6CacheIndexTbl[RTK_DEFAULT_L2_IP6_CACHE_IDX_TBL_SIZE];
static dal_cypress_ip6_cache_entry_t ip6CacheTbl[RTK_DEFAULT_L2_IP6_CACHE_TBL_SIZE];
static int16                curCacheTblIdx;
static dal_cypress_l2_info_t   *pL2_info[RTK_MAX_NUM_OF_UNIT];

rtk_l2_notification_callback_t _notification_callback = NULL;
rtk_l2_notification_bufferRunOut_callback_t _bufferRunOut_callback = NULL;

/* Multicast database */
static dal_cypress_mcast_index_pool_t    mcast_idx_pool[RTK_MAX_NUM_OF_UNIT];

static dal_cypress_l2_mcastFwdShadow_t  *mcast_fwdTable_shadow = NULL;

/*
 * Macro Definition
 */
/* semaphore handling */
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
static int32 _dal_cypress_l2_init_config(uint32 unit);
static int32 _dal_cypress_l2_getExistOrFreeL2Entry(uint32 unit, dal_cypress_l2_entry_t *pL2_entry, dal_cypress_l2_getMethod_t get_method, dal_cypress_l2_index_t *pL2_index);
static int32 _dal_cypress_l2_getFirstDynamicEntry(uint32 unit, dal_cypress_l2_entry_t *pL2_entry, dal_cypress_l2_index_t *pL2_index);
static int32 _dal_cypress_l2_entryToHashKey(uint32 unit, dal_cypress_l2_entry_t *pL2_entry, uint32 *pKey);
static int32 _dal_cypress_l2_hashKeyToVid(uint32 unit, dal_cypress_l2_entry_t *pL2_entry, uint32 key);
static int32 _dal_cypress_l2_compareEntry(dal_cypress_l2_entry_t *pSrcEntry, dal_cypress_l2_entry_t *pDstEntry);
static int32 _dal_cypress_l2_getL2EntryfromHash(uint32 unit, uint32 hashkey, uint32 location, dal_cypress_l2_entry_t *pL2_entry, uint32 *pIsValid);
static int32 _dal_cypress_l2_getL2EntryfromCAM(uint32 unit, uint32 index, dal_cypress_l2_entry_t *pL2_entry, uint32 *pIsValid);
static int32 _dal_cypress_l2_setL2CAMEntry(uint32 unit, dal_cypress_l2_entry_t *pL2_entry, dal_cypress_l2_index_t *pL2_index);
static int32 _dal_cypress_l2_setL2HASHEntry(uint32 unit, dal_cypress_l2_entry_t *pL2_entry, dal_cypress_l2_index_t *pL2_index);
static int32 _dal_cypress_l2_freeMcastIdx(uint32 unit, int32 mcastIdx);
static int32 _dal_cypress_l2_allocMcastIdx(uint32 unit, int32 *pMcastIdx);
static int32 _dal_cypress_l2_isMcastIdxUsed(uint32 unit, int32 mcastIdx);
static int32 _dal_cypress_l2_nextValidAddr_get(uint32 unit, int32 *pScan_idx, uint32 type, uint32 include_static, dal_cypress_l2_entry_t  *pL2_data);
static int32 _dal_cypress_l2_lookupMissFloodPortMask_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask);
static int32 _dal_cypress_l2_ip6CareByteConvertToIP(ipaddr_t *pIp4Dip, ipaddr_t *pIp4Sip, rtk_ipv6_addr_t *pIp6Dip, rtk_ipv6_addr_t *pIp6Sip);
/*static int32 _dal_cypress_l2_ip6CareByteConvertToIP6Addr(rtk_ipv6_addr_t *pIp6Dip, rtk_ipv6_addr_t *pIp6Sip, ipaddr_t *pIp4Dip, ipaddr_t *pIp4Sip);*/
#if !defined(__MODEL_USER__)
static int32 _dal_cypress_notifyHandler(uint32 unit, rtk_l2_notifyEventCollectArray_t *pEventCollectArray);
static int32 _dal_cypress_notifyRunOutHandler(uint32 unit, rtk_l2_notifyBufferType_t type);
#endif


/* example handler for notification */
#if 0
int flag1 = 0, flag2 = 0;
int j = 0;
int cur = 0;
int32 app_notifyHandler(uint32 unit, rtk_l2_notifyEventCollectArray_t *pEventCollectArray)
{
    
    int32 i, ret = 0;
    rtk_l2_notifyEventEntry_t *tmpPtr = 0;
    rtk_mac_t   mac = {{0x00, 0x00, 0x08, 0x00, 0x00, 0x00}};

    osal_time_usleep(1000000);
    
    for (i = 0; i < pEventCollectArray->entryNum; i++, j++)
    {
        mac.octet[4] = j >> 8;
        mac.octet[5] = j & 0xff;
        tmpPtr = &pEventCollectArray->eventArray[i];
        ret = osal_memcmp(tmpPtr->mac.octet, mac.octet, 6);
        if (ret)
            break;
    }

    printk("%s():%d   ret:%d\n", __FUNCTION__, __LINE__, ret);
    for (i = 0; i < pEventCollectArray->entryNum; i++)
    {
        tmpPtr = &pEventCollectArray->eventArray[i];

        osal_printf("2. type:%d vid:%d mac:%x:%x:%x:%x  slp:%d\n", tmpPtr->type, tmpPtr->fidVid, tmpPtr->mac.octet[2], tmpPtr->mac.octet[3], tmpPtr->mac.octet[4], tmpPtr->mac.octet[5], tmpPtr->slp);
    }

    cur += pEventCollectArray->entryNum;

    if (cur == 16384)
    osal_printf("%s():%d  cur:%d type:%d\n", __FUNCTION__, __LINE__, cur, tmpPtr->type);
    if (cur > 16384)
        cur =0;
    return 0;
}

int32 app_notifyRunoutHandler(uint32 unit, rtk_l2_notifyBufferType_t type)
{
    //uint32  ret, val = 0;


    if (type == L2_NOTIFY_BUF_NBUF && flag1 == 0)
    {
        osal_printf("%s():%d  L2_NOTIFY_BUF_NBUF\n", __FUNCTION__, __LINE__);
        flag1 = 1;
    }
    else if (type == L2_NOTIFY_BUF_ASIC_FIFO && flag2 == 0)
    {
        flag2 = 1;
        osal_printf("%s():%d  L2_NOTIFY_BUF_ASIC_FIFO\n", __FUNCTION__, __LINE__);

    }
    return 0;
}
#endif

/* Module Name    : L2     */
/* Sub-module Name: Global */
/* Function Name:
 *      dal_cypress_l2_init
 * Description:
 *      Initialize l2 module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize l2 module before calling any l2 APIs.
 */
int32
dal_cypress_l2_init(uint32 unit)
{
    int32   ret;
    uint32  index;
    uint32  mcast_tableSize;
    rtk_portmask_t  portmask;

    l2_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    l2_sem[unit] = osal_sem_mutex_create();
    if (0 == l2_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_L2), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    if ((ret = table_size_get(unit, CYPRESS_MC_PMSKt, &mcast_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_LOCK(unit);

    pL2_info[unit] = (dal_cypress_l2_info_t *)osal_alloc(sizeof(dal_cypress_l2_info_t));
    if (NULL == pL2_info[unit])
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_L2), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    osal_memset(pL2_info[unit], 0, sizeof(dal_cypress_l2_info_t));

    /* allocate memory for free multicast index */
    mcast_idx_pool[unit].pMcast_index_pool = (dal_cypress_mcast_index_t *)osal_alloc(mcast_tableSize * sizeof(dal_cypress_mcast_index_t));
    if (0 == mcast_idx_pool[unit].pMcast_index_pool)
    {
        osal_free(pL2_info[unit]);
        pL2_info[unit] = NULL;
        mcast_idx_pool[unit].pMcast_index_pool = 0;
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "memory allocate failed");
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

    ucst_dlf_pmsk_inited = FALSE;
    bcst_dlf_pmsk_inited = FALSE;
    ipmcst_fvid_cmp = FALSE;

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* multicast forward table shadow */
        mcast_fwdTable_shadow = osal_alloc(mcast_tableSize * sizeof(dal_cypress_l2_mcastFwdShadow_t));
        if (!mcast_fwdTable_shadow)
        {
            osal_free(pL2_info[unit]);
            pL2_info[unit] = NULL;
            osal_free(mcast_idx_pool[unit].pMcast_index_pool);
            mcast_idx_pool[unit].pMcast_index_pool = 0;
            mcast_idx_pool[unit].free_index_head = 0;
            mcast_idx_pool[unit].free_entry_count = 0;
            mcast_idx_pool[unit].size_of_mcast_fwd_index = 0;
            RT_ERR(ret, (MOD_DAL|MOD_L2), "memory allocate failed");
            return RT_ERR_FAILED;
        }

        osal_memset(mcast_fwdTable_shadow, 0, mcast_tableSize * sizeof(dal_cypress_l2_mcastFwdShadow_t));
    }

    L2_SEM_UNLOCK(unit);

    /* set init flag to complete init */
    l2_init[unit] = INIT_COMPLETED;

    if ((ret = _dal_cypress_l2_init_config(unit)) != RT_ERR_OK)
    {
        l2_init[unit] = INIT_NOT_COMPLETED;
        osal_free(pL2_info[unit]);
        pL2_info[unit] = NULL;
        osal_free(mcast_idx_pool[unit].pMcast_index_pool);
        mcast_idx_pool[unit].pMcast_index_pool = 0;
        mcast_idx_pool[unit].free_index_head = 0;
        mcast_idx_pool[unit].free_entry_count = 0;
        mcast_idx_pool[unit].size_of_mcast_fwd_index = 0;
        if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
        {
            osal_free(mcast_fwdTable_shadow);
            mcast_fwdTable_shadow = NULL;
        }
        RT_ERR(ret, (MOD_DAL|MOD_L2), "L2 default config initialize failed");
        return ret;
    }

    /* Init global variables */
    dal_cypress_l2_hashAlgo_get(unit, &algoType[unit]);
    dal_cypress_l2_ip6CareByte_get(unit, L2_SIP_HASH_CARE_BYTE, &ip6SipCareByte);
    dal_cypress_l2_ip6CareByte_get(unit, L2_DIP_HASH_CARE_BYTE, &ip6DipCareByte);
    reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_LUTCAM_ENf, &l2CamEbl);

    osal_memset(ip6CacheIndexTbl, -1, RTK_DEFAULT_L2_IP6_CACHE_IDX_TBL_SIZE);
    curCacheTblIdx = 0;

    if (HAL_IS_ESCHIP(unit))
    {
        uint32 value;

        value = 1;
        if ((ret = reg_field_write(unit, CYPRESS_RST_GLB_CTRLr, CYPRESS_TBL_MC_PMSK_RSTf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "L2 multicast reset fail");
            return ret;
        }
    }

    /* init fwd table entry 0, so that it won't be allocated to other application */
    HAL_GET_ALL_PORTMASK(unit, portmask);
    dal_cypress_l2_mcastFwdPortmask_set(unit, 0, &portmask, DISABLED);
    dal_cypress_l2_lookupMissFloodPortMaskIdx_set(unit, DLF_TYPE_UCAST, 0);
    dal_cypress_l2_lookupMissFloodPortMaskIdx_set(unit, DLF_TYPE_BCAST, 0);
    dal_cypress_l2_addr_delAll(0, TRUE);

#if !defined(__MODEL_USER__)
    drv_swcore_l2_notification_register(unit, _dal_cypress_notifyHandler, _dal_cypress_notifyRunOutHandler);
#endif

    dal_cypress_l2_notificationEventEnable_set(unit, L2_NOTIFY_EVENT_LINKDOWNFLUSH, ENABLED);
#if 0
    /* Example of user's callback function register */
    rtk_l2_notificationEventHandler_register(unit, app_notifyHandler, app_notifyRunoutHandler);
#endif
    return RT_ERR_OK;
} /* end of dal_cypress_l2_init */


/* Function Name:
 *      dal_cypress_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pEnable  - pointer buffer of state of HW clear linkdown port mac
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_flushLinkDownPortAddrEnable_get(uint32 unit, rtk_enable_t *pEnable)
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
    if ((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_LINK_DOWN_P_INVLDf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
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
} /* end of dal_cypress_l2_flushLinkDownPortAddrEnable_get */


/* Function Name:
 *      dal_cypress_l2_flushLinkDownPortAddrEnable_set
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_flushLinkDownPortAddrEnable_set(uint32 unit, rtk_enable_t enable)
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
    if ((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_LINK_DOWN_P_INVLDf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_flushLinkDownPortAddrEnable_set */

/* Function Name:
 *      dal_cypress_l2_ucastAddr_flush
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_VLAN_VID     - invalid vlan id
 * Note:
 *      None
 */
int32
dal_cypress_l2_ucastAddr_flush(uint32 unit, rtk_l2_flushCfg_t *pConfig)
{
    int32   ret;

    if (HAL_IS_ESCHIP(unit))
    {
        uint32  l2_idx;
        uint32  l2_tableSize;
        uint32  hashkey, location;
        uint32  isValid;
        uint32  include_static;
        dal_cypress_l2_entry_t  l2_entry;
        dal_cypress_l2_index_t  index_entry;

        /* check Init status */
        RT_INIT_CHK(l2_init[unit]);

        if ((ret = table_size_get(unit, CYPRESS_L2_UCt, &l2_tableSize)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        include_static = pConfig->flushStaticAddr;
        if (FLUSH_ACT_FLUSH == pConfig->act)
        {
            for (l2_idx = 0; l2_idx < l2_tableSize; l2_idx++)
            {
                hashkey = l2_idx >> 2;
                location = l2_idx & BITMASK_2B;

                ret = _dal_cypress_l2_getL2EntryfromHash(unit, hashkey, location, &l2_entry, &isValid);
                /* Because flush CPU port will cause mac constraint counter on port 51 error, flushing CPU port is forbidden */
                if ((TRUE == isValid) && (L2_UNICAST == l2_entry.entry_type) && (!HAL_IS_CPU_PORT(unit, l2_entry.unicast.port)))
                {
                    if ((FALSE == include_static) && (1 == l2_entry.unicast.is_static))
                    {
                        /* if not need to include static and this entry is static, just skip this entry */
                        continue;
                    }
                    if (1 == l2_entry.unicast.nh)
                    {
                        /* nh entry is not included in this API */
                        continue;
                    }
                    if (pConfig->flushByPort && pConfig->port != l2_entry.unicast.port)
                    {
                        continue;
                    }
                    if (pConfig->flushByVid && pConfig->vid != l2_entry.unicast.fid)
                    {
                        continue;
                    }

                    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
                    l2_entry.entry_type = L2_UNICAST;
                    index_entry.index = hashkey;
                    index_entry.hashdepth = location;
                    ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);

                }
            }
        }
        else
        {
            for (l2_idx = 0; l2_idx < l2_tableSize; l2_idx++)
            {
                hashkey = l2_idx >> 2;
                location = l2_idx & BITMASK_2B;

                ret = _dal_cypress_l2_getL2EntryfromHash(unit, hashkey, location, &l2_entry, &isValid);
                /* Because flush CPU port will cause mac constraint counter on port 51 error, flushing CPU port is forbidden */
                if ((TRUE == isValid) && (L2_UNICAST == l2_entry.entry_type) && (!HAL_IS_CPU_PORT(unit, l2_entry.unicast.port)))
                {
                    if ((FALSE == include_static) && (1 == l2_entry.unicast.is_static))
                    {
                        /* if not need to include static and this entry is static, just skip this entry */
                        continue;
                    }
                    if (1 == l2_entry.unicast.nh)
                    {
                        /* nh entry is not included in this API */
                        continue;
                    }
                    if (pConfig->flushByPort && pConfig->port != l2_entry.unicast.port)
                    {
                        continue;
                    }
                    if (pConfig->flushByVid && pConfig->vid != l2_entry.unicast.fid)
                    {
                        continue;
                    }

                    index_entry.index = hashkey;
                    index_entry.hashdepth = location;
                    l2_entry.unicast.port = pConfig->replacingPort;
                    ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);

                }
            }
        }
    }
    else
    {
        uint32      value, data;

        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, act=%d, byVid=%d, byPort=%d, vid=%d, port=%d, replacingPort=%d",
            unit, pConfig->act, pConfig->flushByVid, pConfig->flushByPort, pConfig->vid, pConfig->port, pConfig->replacingPort);

        /* check Init status */
        RT_INIT_CHK(l2_init[unit]);

        RT_PARAM_CHK((NULL == pConfig), RT_ERR_NULL_POINTER);
        RT_PARAM_CHK((TRUE == pConfig->flushByPort && !HAL_IS_PORT_EXIST(unit, pConfig->port)), RT_ERR_PORT_ID);
        RT_PARAM_CHK((TRUE == pConfig->flushByVid && ((pConfig->vid < RTK_VLAN_ID_MIN) || (pConfig->vid > RTK_VLAN_ID_MAX))), RT_ERR_VLAN_VID);

        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_ACTf, &pConfig->act, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_FVID_CMPf, &pConfig->flushByVid, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_PORT_CMPf, &pConfig->flushByPort, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        value = pConfig->flushStaticAddr ? 0 : 1;  /* 0:include static   1:dynamic only */
        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_ENTRY_TYPEf, &value, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_FVIDf, &pConfig->vid, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_PORTf, &pConfig->port, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_REPLACING_PORTf, &pConfig->replacingPort, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        value = 1;  /* start */
        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_STSf, &value, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }



        L2_SEM_LOCK(unit);
        /* program value to CHIP*/
        if ((ret = reg_write(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, &data)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        do
        {
            if((ret = reg_field_read(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_STSf, &data)) != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }
        } while (data != 0);
        L2_SEM_UNLOCK(unit);
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_ucastAddr_flush */

/* Function Name:
 *      dal_cypress_l2_learningCnt_get
 * Description:
 *      Get the total mac learning counts of the whole specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pMac_cnt - pointer buffer of mac learning counts of the port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The mac learning counts only calculate dynamic mac numbers.
 */
int32
dal_cypress_l2_learningCnt_get(uint32 unit, uint32 *pMac_cnt)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMac_cnt), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if((ret = reg_field_read(unit, CYPRESS_L2_LRN_CONSTRTr, CYPRESS_LRN_CNTf, pMac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac_cnt=%d", *pMac_cnt);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_learningCnt_get */

/* Function Name:
 *      dal_cypress_l2_limitLearningCnt_get
 * Description:
 *      Get the maximum mac limited learning counts of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pMac_cnt - pointer buffer of maximum mac learning counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The maximum mac limited learning counts only limit for dynamic learning mac
 *      address, not apply to static mac address.
 *      2. Set the mac_cnt to 0 mean disable learning in the system.
 */
int32
dal_cypress_l2_limitLearningCnt_get(uint32 unit, uint32 *pMac_cnt)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMac_cnt), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if((ret = reg_field_read(unit, CYPRESS_L2_LRN_CONSTRTr, CYPRESS_CONSTRT_NUMf, pMac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac_cnt=%d", *pMac_cnt);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_limitLearningCnt_get */

/* Function Name:
 *      dal_cypress_l2_limitLearningCnt_set
 * Description:
 *      Set the maximum mac limited learning counts of the specified device.
 * Input:
 *      unit    - unit id
 *      mac_cnt - maximum mac learning counts
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_LIMITED_L2ENTRY_NUM - invalid limited L2 entry number
 * Note:
 *      1. The maximum mac limited learning counts only limit for dynamic learning mac
 *      address, not apply to static mac address.
 *      2. Set the mac_cnt to 0 mean disable learning in the system.
 */
int32
dal_cypress_l2_limitLearningCnt_set(uint32 unit, uint32 mac_cnt)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, mac_cnt=%d", unit, mac_cnt);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((mac_cnt > HAL_L2_LEARN_LIMIT_CNT_MAX(unit)) && (mac_cnt != HAL_L2_LEARN_LIMIT_CNT_DISABLE(unit)), RT_ERR_LIMITED_L2ENTRY_NUM);

    L2_SEM_LOCK(unit);

    /* programming value into CHIP*/
    if((ret = reg_field_write(unit, CYPRESS_L2_LRN_CONSTRTr, CYPRESS_CONSTRT_NUMf, &mac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_limitLearningCnt_set */

/* Function Name:
 *      dal_cypress_l2_limitLearningCntAction_get
 * Description:
 *      Get the action when over learning maximum mac counts of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      *pAction - pointer buffer of action when over learning maximum mac counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The action symbol as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
int32
dal_cypress_l2_limitLearningCntAction_get(uint32 unit, rtk_l2_limitLearnCntAction_t *pAction)
{
    int32   ret;
    uint32  act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if((ret = reg_field_read(unit, CYPRESS_L2_LRN_CONSTRTr, CYPRESS_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /* translate register value to action */
    switch (act)
    {
        case LIMIT_LEARN_CNT_ACTION_DROP:
            *pAction = 1;
            break;
        case LIMIT_LEARN_CNT_ACTION_FORWARD:
            *pAction = 0;
            break;
        case LIMIT_LEARN_CNT_ACTION_TO_CPU:
            *pAction = 2;
            break;
        case LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU:
            *pAction = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAction=%d", *pAction);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_limitLearningCntAction_get */

/* Function Name:
 *      dal_cypress_l2_limitLearningCntAction_set
 * Description:
 *      Set the action when over learning maximum mac counts of the specified device.
 * Input:
 *      unit   - unit id
 *      action - action when over learning maximum mac counts
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      1. The action symbol as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
int32
dal_cypress_l2_limitLearningCntAction_set(uint32 unit, rtk_l2_limitLearnCntAction_t action)
{
    int32   ret;
    uint32  act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, action=%d", unit, action);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(action >= LIMIT_LEARN_CNT_ACTION_END, RT_ERR_INPUT);

    switch (action)
    {
        case LIMIT_LEARN_CNT_ACTION_DROP:
            act = 1;
            break;
        case LIMIT_LEARN_CNT_ACTION_FORWARD:
            act = 0;
            break;
        case LIMIT_LEARN_CNT_ACTION_TO_CPU:
            act = 2;
            break;
        case LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU:
            act = 3;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    /* programming value to CHIP*/
    if((ret = reg_field_write(unit, CYPRESS_L2_LRN_CONSTRTr, CYPRESS_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_limitLearningCntAction_set */

/* Function Name:
 *      dal_cypress_l2_portLearningCnt_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The mac learning counts only calculate dynamic mac numbers.
 */
int32
dal_cypress_l2_portLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt)
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
    if((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_LRN_CONSTRTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_LRN_CNTf, pMac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac_cnt=%d", *pMac_cnt);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_portLearningCnt_get */

/* Function Name:
 *      dal_cypress_l2_portLimitLearningCnt_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The maximum mac learning counts only limit for dynamic learning mac
 *      address, not apply to static mac address.
 *      2. Set the mac_cnt to 0 mean disable learning in the port.
 */
int32
dal_cypress_l2_portLimitLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt)
{
#if defined(CONFIG_SDK_DRIVER_RTK_BACKWARD_COMPATIBLE_8328)
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMac_cnt), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    L2_SEM_LOCK(unit);
    /* get value from shadow */
    (*pMac_cnt) = pL2_info[unit]->limitLearningCnt[port];
    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac_cnt=%d", *pMac_cnt);

    return RT_ERR_OK;
#else
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMac_cnt), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_LRN_CONSTRTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_CONSTRT_NUMf, pMac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac_cnt=%d", *pMac_cnt);

    return RT_ERR_OK;
#endif
} /* end of dal_cypress_l2_portLimitLearningCnt_get */

/* Function Name:
 *      dal_cypress_l2_portLimitLearningCnt_set
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
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_LIMITED_L2ENTRY_NUM - invalid limited L2 entry number
 * Note:
 *      1. The maximum mac learning counts only limit for dynamic learning mac
 *      address, not apply to static mac address.
 *      2. Set the mac_cnt to 0 mean disable learning in the port.
 */
int32
dal_cypress_l2_portLimitLearningCnt_set(uint32 unit, rtk_port_t port, uint32 mac_cnt)
{
#if defined(CONFIG_SDK_DRIVER_RTK_BACKWARD_COMPATIBLE_8328)
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, mac_cnt=%d",
           unit, port, mac_cnt);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((mac_cnt > HAL_L2_LEARN_LIMIT_CNT_MAX(unit)) && (mac_cnt != HAL_L2_LEARN_LIMIT_CNT_DISABLE(unit)), RT_ERR_LIMITED_L2ENTRY_NUM);

    L2_SEM_LOCK(unit);

    if (pL2_info[unit]->limitLearningCnt_enable[port] == ENABLED)
    {
        /* programming value into CHIP*/
        if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_LRN_CONSTRTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_CONSTRT_NUMf, &mac_cnt)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    /* store the value to shadow */
    pL2_info[unit]->limitLearningCnt[port] = mac_cnt;
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
#else
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
    if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_LRN_CONSTRTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_CONSTRT_NUMf, &mac_cnt)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
#endif
} /* end of dal_cypress_l2_portLimitLearningCnt_set */

/* Function Name:
 *      dal_cypress_l2_portLimitLearningCntAction_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The action symbol as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
int32
dal_cypress_l2_portLimitLearningCntAction_get(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction)
{
    int32   ret;
    uint32  act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_LRN_CONSTRTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /* translate register value to action */
    switch (act)
    {
        case LIMIT_LEARN_CNT_ACTION_DROP:
            *pAction = 1;
            break;
        case LIMIT_LEARN_CNT_ACTION_FORWARD:
            *pAction = 0;
            break;
        case LIMIT_LEARN_CNT_ACTION_TO_CPU:
            *pAction = 2;
            break;
        case LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU:
            *pAction = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAction=%d", *pAction);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_portLimitLearningCntAction_get */

/* Function Name:
 *      dal_cypress_l2_portLimitLearningCntAction_set
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_portLimitLearningCntAction_set(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t action)
{
    int32   ret;
    uint32  act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, action=%d", unit, port, action);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(action >= LIMIT_LEARN_CNT_ACTION_END, RT_ERR_INPUT);

    switch (action)
    {
        case LIMIT_LEARN_CNT_ACTION_DROP:
            act = 1;
            break;
        case LIMIT_LEARN_CNT_ACTION_FORWARD:
            act = 0;
            break;
        case LIMIT_LEARN_CNT_ACTION_TO_CPU:
            act = 2;
            break;
        case LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU:
            act = 3;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    /* programming value to CHIP*/
    if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_LRN_CONSTRTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_portLimitLearningCntAction_set */

/* Function Name:
 *      dal_cypress_l2_fidLimitLearningEntry_get
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
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 *      - LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU
 */
int32
dal_cypress_l2_fidLimitLearningEntry_get(uint32 unit, uint32 fid_macLimit_idx, rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, fid_macLimit_idx=%d", unit, fid_macLimit_idx);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((fid_macLimit_idx >= HAL_L2_FID_LEARN_LIMIT_ENTRY_MAX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pFidMacLimitEntry), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);
    if ((ret = reg_array_field_read(unit, CYPRESS_L2_VLAN_LRN_CONSTRTr, REG_ARRAY_INDEX_NONE, fid_macLimit_idx, CYPRESS_FVIDf, &pFidMacLimitEntry->fid)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    if ((ret = reg_array_field_read(unit, CYPRESS_L2_VLAN_LRN_CONSTRTr, REG_ARRAY_INDEX_NONE, fid_macLimit_idx, CYPRESS_PORTf, &pFidMacLimitEntry->port)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    if ((ret = reg_array_field_read(unit, CYPRESS_L2_VLAN_LRN_CONSTRTr, REG_ARRAY_INDEX_NONE, fid_macLimit_idx, CYPRESS_CONSTRT_NUMf, &pFidMacLimitEntry->maxNum)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "fid=%d", pFidMacLimitEntry->fid);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "port=%d", pFidMacLimitEntry->port);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "maxNum=%d", pFidMacLimitEntry->maxNum);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_fidLimitLearningEntry_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 * Note:
 *      Forwarding action is as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 *      - LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU
 */
int32
dal_cypress_l2_fidLimitLearningEntry_set(uint32 unit, uint32 fid_macLimit_idx, rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry)
{
    int32               ret;
    uint32              oriVid, zero = 0;
    rtk_l2_flushCfg_t   config;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((fid_macLimit_idx >= HAL_L2_FID_LEARN_LIMIT_ENTRY_MAX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == pFidMacLimitEntry, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pFidMacLimitEntry->fid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK((pFidMacLimitEntry->port > HAL_GET_MAX_PORT(unit) && pFidMacLimitEntry->port != PORT_DONT_CARE), RT_ERR_PORT_ID);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, fid_macLimit_idx, fid=%d, port=%d, maxNum=%d", unit, fid_macLimit_idx,
                                        pFidMacLimitEntry->fid, pFidMacLimitEntry->port, pFidMacLimitEntry->maxNum);


    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));


    L2_SEM_LOCK(unit);

    if ((ret = reg_array_field_read(unit, CYPRESS_L2_VLAN_LRN_CONSTRTr, REG_ARRAY_INDEX_NONE, fid_macLimit_idx, CYPRESS_FVIDf, &oriVid)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* If this entry has been used, we should flush all MACs leart on this previous vlan */
    if (oriVid != 0)
    {
        config.act = 0;
        config.flushByVid = TRUE;
        config.vid = oriVid;
        L2_SEM_UNLOCK(unit);
        dal_cypress_l2_ucastAddr_flush(unit, &config);
        L2_SEM_LOCK(unit);

    }


    /* Disable learning and flush MACs on specified vlan */
    if ((ret = reg_array_field_write(unit, CYPRESS_L2_VLAN_LRN_CONSTRTr, REG_ARRAY_INDEX_NONE, fid_macLimit_idx, CYPRESS_CONSTRT_NUMf, &zero)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    if ((ret = reg_array_field_write(unit, CYPRESS_L2_VLAN_LRN_CONSTRTr, REG_ARRAY_INDEX_NONE, fid_macLimit_idx, CYPRESS_FVIDf, &pFidMacLimitEntry->fid)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    if ((ret = reg_array_field_write(unit, CYPRESS_L2_VLAN_LRN_CONSTRTr, REG_ARRAY_INDEX_NONE, fid_macLimit_idx, CYPRESS_PORTf, &pFidMacLimitEntry->port)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    config.flushByVid = TRUE;
    config.vid = pFidMacLimitEntry->fid;
    dal_cypress_l2_ucastAddr_flush(unit, &config);

    if ((ret = reg_array_field_write(unit, CYPRESS_L2_VLAN_LRN_CONSTRT_CNTr, REG_ARRAY_INDEX_NONE, fid_macLimit_idx, CYPRESS_LRN_CNTf, &zero)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }


    /* Enable learning to user specified number */
    L2_SEM_LOCK(unit);
    if ((ret = reg_array_field_write(unit, CYPRESS_L2_VLAN_LRN_CONSTRTr, REG_ARRAY_INDEX_NONE, fid_macLimit_idx, CYPRESS_CONSTRT_NUMf, &pFidMacLimitEntry->maxNum)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);


    return ret;
} /* end of dal_cypress_l2_fidLimitLearningEntry_set */

/* Function Name:
 *      dal_cypress_l2_fidLearningCnt_get
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
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_fidLearningCnt_get(uint32 unit, uint32 fid_macLimit_idx, uint32 *pNum)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, fid_macLimit_idx=%d", unit, fid_macLimit_idx);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((fid_macLimit_idx >= HAL_L2_FID_LEARN_LIMIT_ENTRY_MAX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pNum), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);
    if ((ret = reg_array_field_read(unit, CYPRESS_L2_VLAN_LRN_CONSTRT_CNTr, REG_ARRAY_INDEX_NONE, fid_macLimit_idx, CYPRESS_LRN_CNTf, pNum)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pNum=%d", *pNum);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_fidLearningCnt_reset
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
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
int32
dal_cypress_l2_fidLearningCnt_reset(uint32 unit, uint32 fid_macLimit_idx)
{
    int32       ret;
    uint32      value = 0;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((fid_macLimit_idx >= HAL_L2_FID_LEARN_LIMIT_ENTRY_MAX(unit)), RT_ERR_OUT_OF_RANGE);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, fid_macLimit_idx=%d", unit, fid_macLimit_idx);

    L2_SEM_LOCK(unit);

    if ((ret = reg_array_field_write(unit, CYPRESS_L2_VLAN_LRN_CONSTRT_CNTr, REG_ARRAY_INDEX_NONE, fid_macLimit_idx, CYPRESS_LRN_CNTf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return ret;
}

/* Function Name:
 *      dal_cypress_l2_fidLearningCntAction_get
 * Description:
 *      Get the action when over learning maximum mac counts on specified entry of fid MAC limit.
 * Input:
 *      unit                - unit id
 *      fid_macLimit_idx    - index of FID MAC limit entry
 * Output:
 *      pAction             - pointer buffer of action when over learning maximum mac counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_fidLearningCntAction_get(uint32 unit, rtk_l2_limitLearnCntAction_t *pAction)
{
    int32   ret;
    uint32  act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if((ret = reg_field_read(unit, CYPRESS_L2_VLAN_LRN_CONSTRT_ACTr, CYPRESS_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /* translate register value to action */
    switch (act)
    {
        case LIMIT_LEARN_CNT_ACTION_DROP:
            *pAction = 1;
            break;
        case LIMIT_LEARN_CNT_ACTION_FORWARD:
            *pAction = 0;
            break;
        case LIMIT_LEARN_CNT_ACTION_TO_CPU:
            *pAction = 2;
            break;
        case LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU:
            *pAction = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAction=%d", *pAction);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_fidLearningCntAction_set
 * Description:
 *      Set the action when over learning maximum mac counts on specified entry of fid MAC limit.
 * Input:
 *      unit             - unit id
 *      action           - action when over learning maximum mac counts
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_fidLearningCntAction_set(uint32 unit, rtk_l2_limitLearnCntAction_t action)
{
    int32       ret;
    uint32      act;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    switch (action)
    {
        case LIMIT_LEARN_CNT_ACTION_DROP:
            act = 1;
            break;
        case LIMIT_LEARN_CNT_ACTION_FORWARD:
            act = 0;
            break;
        case LIMIT_LEARN_CNT_ACTION_TO_CPU:
            act = 2;
            break;
        case LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU:
            act = 3;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);
    if ((ret = reg_field_write(unit, CYPRESS_L2_VLAN_LRN_CONSTRT_ACTr, CYPRESS_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return ret;
}

/* Function Name:
 *      dal_cypress_l2_aging_get
 * Description:
 *      Get the dynamic address aging time from the specified device.
 * Input:
 *      unit        - unit id
 * Output:
 *      pAging_time - pointer buffer of aging time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Get aging_time as 0 mean disable aging mechanism. (seconds)
 */
int32
dal_cypress_l2_aging_get(uint32 unit, uint32 *pAging_time)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pAging_time), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, CYPRESS_L2_CTRL_1r, CYPRESS_AGE_UNITf, pAging_time)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    *pAging_time = (uint32)(((*pAging_time)*3)/5);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAging_time=%ld", *pAging_time);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_aging_get */

/* Function Name:
 *      dal_cypress_l2_aging_set
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      apply aging_time as 0 mean disable aging mechanism.
 */
int32
dal_cypress_l2_aging_set(uint32 unit, uint32 aging_time)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    aging_time = (aging_time * 5 + 2) / 3;

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, CYPRESS_L2_CTRL_1r, CYPRESS_AGE_UNITf, &aging_time)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_aging_set */

/* Function Name:
 *      dal_cypress_l2_portAgingEnable_get
 * Description:
 *      Get the dynamic address aging out configuration of the specified port to the specified device.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - enable status of aging out
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_portAgingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  enable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_AGING_OUTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_AGING_OUT_ENf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
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
} /* end of dal_cypress_l2_portAgingEnable_get */

/* Function Name:
 *      dal_cypress_l2_portAgingEnable_set
 * Description:
 *      Set the dynamic address aging out configuration of the specified port to the specified device.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - enable status of aging out
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_portAgingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, enable=%d", unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
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

    /* program value from CHIP */
    if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_AGING_OUTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_AGING_OUT_ENf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_portAgingEnable_set */

/* Function Name:
 *      dal_cypress_l2_hashAlgo_get
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
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_hashAlgo_get(uint32 unit, uint32 *pHash_algo)
{
    int32   ret;
    uint32  value = 0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pHash_algo), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    if((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_L2_HASH_ALGOf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    *pHash_algo = value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pHash_algo=%d", *pHash_algo);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_hashAlgo_get */

/* Function Name:
 *      dal_cypress_l2_hashAlgo_set
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
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_hashAlgo_set(uint32 unit, uint32 hash_algo)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d  hash_algo=%d", unit, hash_algo);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_PARAM_CHK((hash_algo >= HAL_MAX_NUM_OF_L2_HASH_ALGO(unit)), RT_ERR_INPUT);

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_L2_HASH_ALGOf, &hash_algo)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    algoType[unit] = hash_algo;

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_hashAlgo_set */

/* Function Name:
 *      dal_cypress_l2_vlanMode_get
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
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_vlanMode_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pVlanMode)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pVlanMode), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_read(unit, CYPRESS_VLAN_PORT_FWDr,
                    port, REG_ARRAY_INDEX_NONE, CYPRESS_FWD_BASEf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /* translate chip's value to definition */
    switch (value)
    {
        case 0:
            *pVlanMode = BASED_ON_INNER_VLAN;
            break;
        case 1:
            *pVlanMode = BASED_ON_OUTER_VLAN;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pVlanMode=%d", *pVlanMode);

    return RT_ERR_OK;
}/* end of dal_cypress_l2_vlanMode_get */

/* Function Name:
 *      dal_cypress_l2_vlanMode_set
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
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_vlanMode_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t vlanMode)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vlanMode=%d", unit, vlanMode);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((vlanMode >= FWD_VLAN_MODE_END), RT_ERR_INPUT);

    /* translate value to chip's definition */
    switch (vlanMode)
    {
        case BASED_ON_INNER_VLAN:
            value = 0;
            break;
        case BASED_ON_OUTER_VLAN:
            value = 1;
            break;
        default:
            return RT_ERR_FAILED;
    }

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_write(unit, CYPRESS_VLAN_PORT_FWDr,
                    port, REG_ARRAY_INDEX_NONE, CYPRESS_FWD_BASEf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/* end of dal_cypress_l2_vlanMode_set */

/* Function Name:
 *      dal_cypress_l2_newMacOp_get
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
dal_cypress_l2_newMacOp_get(uint32 unit, rtk_port_t port, rtk_l2_newMacLrnMode_t *pLrnMode, rtk_action_t *pFwdAction)
{
    int32   ret;
    uint32  mode, act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pLrnMode), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pFwdAction), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_NEW_SALRNr, port, REG_ARRAY_INDEX_NONE, CYPRESS_NEW_SALRNf, &mode)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "err ret = %d", ret);
        return ret;
    }
    if((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_NEW_SA_FWDr, port, REG_ARRAY_INDEX_NONE, CYPRESS_NEW_SA_FWDf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "err ret = %d", ret);
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    switch (mode)
    {
        case 0:
            *pLrnMode = HARDWARE_LEARNING;
            break;
        case 1:
            *pLrnMode = SOFTWARE_LEARNING;
            break;
        case 2:
            *pLrnMode = NOT_LEARNING;
            break;
        default:
            return RT_ERR_FAILED;
    }

    switch (act)
    {
        case 0:
            *pFwdAction = ACTION_FORWARD;
            break;
        case 1:
            *pFwdAction = ACTION_DROP;
            break;
        case 2:
            *pFwdAction = ACTION_TRAP2CPU;
            break;
        case 3:
            *pFwdAction = ACTION_COPY2CPU;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pLrnMode=%d  pFwdAction:%d", *pLrnMode, *pFwdAction);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_newMacOp_get */

/* Function Name:
 *      dal_cypress_l2_newMacOp_set
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
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
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
dal_cypress_l2_newMacOp_set(uint32 unit, rtk_port_t port, rtk_l2_newMacLrnMode_t lrnMode, rtk_action_t fwdAction)
{
    int32   ret;
    uint32  mode, act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(lrnMode >= LEARNING_MODE_END, RT_ERR_INPUT);

    switch (lrnMode)
    {
        case HARDWARE_LEARNING:
            mode = 0;
            break;
        case SOFTWARE_LEARNING:
            mode = 1;
            break;
        case NOT_LEARNING:
            mode = 2;
            break;
        default:
            return RT_ERR_FAILED;
    }

    switch (fwdAction)
    {
        case ACTION_FORWARD:
            act = 0;
            break;
        case ACTION_DROP:
            act = 1;
            break;
        case ACTION_TRAP2CPU:
            act = 2;
            break;
        case ACTION_COPY2CPU:
            act = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_NEW_SALRNr, port, REG_ARRAY_INDEX_NONE, CYPRESS_NEW_SALRNf, &mode)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "err ret = %d", ret);
        return ret;
    }
    if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_NEW_SA_FWDr, port, REG_ARRAY_INDEX_NONE, CYPRESS_NEW_SA_FWDf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "err ret = %d", ret);
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_newMacOp_set */


/* Module Name    : L2      */
/* Sub-module Name: Unicast */

/* Function Name:
 *      dal_cypress_l2_addr_init
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328, 8390
 * Note:
 *      Need to initialize L2 entry before add it.
 */
int32
dal_cypress_l2_addr_init(
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
}

/* Function Name:
 *      dal_cypress_l2_addr_add
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize L2 entry before add it.
 */
int32
dal_cypress_l2_addr_add(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr)
{
    int32               ret;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pL2_addr=%x", unit, pL2_addr);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pL2_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pL2_addr->mac.octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK((!HAL_IS_PORT_EXIST(unit, pL2_addr->port) &&
        (!(pL2_addr->flags & RTK_L2_UCAST_FLAG_SA_BLOCK) && !(pL2_addr->flags & RTK_L2_UCAST_FLAG_DA_BLOCK) && !(pL2_addr->flags & RTK_L2_UCAST_FLAG_TRUNK_PORT)) ), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pL2_addr->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(((pL2_addr->flags & RTK_L2_UCAST_FLAG_NEXTHOP) && pL2_addr->vlan_target >= 2 &&
                   pL2_addr->route_idx >= HAL_MAX_NUM_OF_ROUTE_HOST_ADDR(unit)), RT_ERR_INPUT);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2)
            , "unit=%d, vid=%d, port=%d, flags=%x, trk_gid=%d, state=%d"
            , unit, pL2_addr->vid, pL2_addr->port, pL2_addr->flags
            , pL2_addr->trk_gid, pL2_addr->state);

    if (!(pL2_addr->flags & RTK_L2_UCAST_FLAG_TRUNK_PORT))
    {
        for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
        {
            if (dal_cypress_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
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
        if ((ret = dal_cypress_trunk_port_get(unit, pL2_addr->trk_gid, &trunk_portmask)) != RT_ERR_OK)
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
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid = pL2_addr->vid;
    osal_memcpy(&l2_entry.unicast.mac, &(pL2_addr->mac), sizeof(rtk_mac_t));

    if (HAL_IS_ESCHIP(unit) && (pL2_addr->flags & RTK_L2_UCAST_FLAG_NEXTHOP))
        l2_entry.unicast.nh = TRUE;

    L2_SEM_LOCK(unit);

    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry);
    if (ret == RT_ERR_L2_NO_EMPTY_ENTRY && (pL2_addr->add_op_flags & RTK_L2_ADD_OP_FLAG_REPLACE_DYNAMIC) != 0)
    {
        ret = _dal_cypress_l2_getFirstDynamicEntry(unit, &l2_entry, &index_entry);
        if (ret)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* fill content */
    osal_memcpy(&l2_entry.unicast.mac, &(pL2_addr->mac), sizeof(rtk_mac_t));
    l2_entry.unicast.fid        = pL2_addr->vid;
    l2_entry.unicast.port       = pL2_addr->port;
    l2_entry.unicast.aging      = 7;
    l2_entry.unicast.sablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_SA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.dablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_DA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.is_static  = (pL2_addr->flags & RTK_L2_UCAST_FLAG_STATIC) ? TRUE: FALSE;
    l2_entry.unicast.suspending = (pL2_addr->state & RTK_L2_UCAST_STATE_SUSPEND) ? TRUE: FALSE;
    if (pL2_addr->flags & RTK_L2_UCAST_FLAG_NEXTHOP)
    {
        l2_entry.unicast.nh          = TRUE;
        l2_entry.unicast.vlan_target = pL2_addr->vlan_target;
        l2_entry.unicast.route_idx   = pL2_addr->route_idx;
        if (pL2_addr->isAged == TRUE)
        {
            l2_entry.unicast.aging = 0;
            l2_entry.unicast.port  = 63;
        }
    }
    else
    {
        l2_entry.unicast.nh          = FALSE;
        l2_entry.unicast.agg_vid     = pL2_addr->agg_vid;
    }

    if (L2_IN_HASH == index_entry.index_type)
    {
        /* if found entry is in HASH, programming in HASH */
        ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
        /* return the entry index */
        pL2_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    }
    else if (L2_IN_CAM == index_entry.index_type)
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
        /* return the entry index */
        pL2_addr->l2_idx = index_entry.index;
    }
    else
    {
        ret = RT_ERR_FAILED;
    }

    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_cypress_l2_addr_add */


/* Function Name:
 *      dal_cypress_l2_addr_del
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
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. vid is same as fid in IVL mode.
 */
int32
dal_cypress_l2_addr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;

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
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid = vid;
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);

    if ((ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    /* fill content */
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type = L2_UNICAST;

    if (L2_IN_HASH == index_entry.index_type)
    {
        /* if found entry is in HASH, programming in HASH */
        ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    }
    else if (L2_IN_CAM == index_entry.index_type)
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }

    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_cypress_l2_addr_del */


/* Function Name:
 *      dal_cypress_l2_addr_get
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
 *      RT_ERR_NOT_INIT          - The module is not initial
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
dal_cypress_l2_addr_get(uint32 unit, rtk_l2_ucastAddr_t *pL2_data)
{
    int32 ret;
    uint32                  trk_gid;
    uint32                  first_trunkMember;
    rtk_portmask_t          trunk_portmask;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, vid=%d", unit, pL2_data->vid);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pL2_data, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pL2_data->mac.octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK((pL2_data->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMac=%x-%x-%x-%x-%x-%x",
           pL2_data->mac.octet[0], pL2_data->mac.octet[1], pL2_data->mac.octet[2],
           pL2_data->mac.octet[3], pL2_data->mac.octet[4], pL2_data->mac.octet[5]);

    /* search exist or free entry */
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type     = L2_UNICAST;
    l2_entry.unicast.fid    = pL2_data->vid;
    osal_memcpy(&l2_entry.unicast.mac, &pL2_data->mac, sizeof(rtk_mac_t));

    if (HAL_IS_ESCHIP(unit) && (pL2_data->flags & RTK_L2_UCAST_FLAG_NEXTHOP))
    {
        l2_entry.unicast.nh = TRUE;
    }

    L2_SEM_LOCK(unit);

    if ((ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
    {
        /* Return Fail if not found */
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /* fill content */
    if (HAL_IS_ESCHIP(unit) && (pL2_data->flags & RTK_L2_UCAST_FLAG_NEXTHOP))
    {
        l2_entry.unicast.sablock = FALSE;
        l2_entry.unicast.dablock = FALSE;
        l2_entry.unicast.is_static = FALSE;
        l2_entry.unicast.suspending = FALSE;
        l2_entry.unicast.agg_vid = FALSE;
    }
    else
    {
        osal_memcpy(&pL2_data->mac, &l2_entry.unicast.mac, sizeof(rtk_mac_t));
        pL2_data->vid       = l2_entry.unicast.fid;
    }
    pL2_data->port      = l2_entry.unicast.port;
    pL2_data->flags     = 0;    /* Clear data */
    pL2_data->state     = 0;    /* Clear data */

    for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
    {
        if (dal_cypress_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
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
    if(l2_entry.unicast.is_static)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_STATIC;
    if(l2_entry.unicast.nh)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
    if(l2_entry.unicast.suspending)
        pL2_data->state |= RTK_L2_UCAST_STATE_SUSPEND;
    if(l2_entry.unicast.aging == 0)
        pL2_data->isAged = TRUE;
    else
        pL2_data->isAged = FALSE;
    pL2_data->vlan_target   = l2_entry.unicast.vlan_target;
    pL2_data->route_idx     = l2_entry.unicast.route_idx;
    pL2_data->agg_vid       = l2_entry.unicast.agg_vid;

    if(index_entry.index_type == L2_IN_HASH)
        pL2_data->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pL2_data->l2_idx = index_entry.index;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "vid=%d, port=%d, sa_block=%d, da_block=%d, is_static=%d, nh=%d, suspend=%d, agg_vid=:%d",
        pL2_data->vid, pL2_data->port, l2_entry.unicast.sablock, l2_entry.unicast.dablock, l2_entry.unicast.is_static,
        l2_entry.unicast.nh, l2_entry.unicast.suspending, l2_entry.unicast.agg_vid);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_addr_get */


/* Function Name:
 *      dal_cypress_l2_addr_set
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
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_addr_set(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr)
{
    int32               ret;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pL2_addr=%x", unit, pL2_addr);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pL2_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pL2_addr->mac.octet[0] & BITMASK_1B) != 0, RT_ERR_MAC);
    RT_PARAM_CHK((!HAL_IS_PORT_EXIST(unit, pL2_addr->port) &&
        (!(pL2_addr->flags & RTK_L2_UCAST_FLAG_SA_BLOCK) && !(pL2_addr->flags & RTK_L2_UCAST_FLAG_DA_BLOCK) && !(pL2_addr->flags & RTK_L2_UCAST_FLAG_TRUNK_PORT)) ), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pL2_addr->vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(((pL2_addr->flags & RTK_L2_UCAST_FLAG_NEXTHOP) && pL2_addr->vlan_target >= 2 &&
                   pL2_addr->route_idx >= HAL_MAX_NUM_OF_ROUTE_HOST_ADDR(unit)), RT_ERR_INPUT);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2)
            , "unit=%d, vid=%d, port=%d, flags=%x, trk_gid=%d, state=%d"
            , unit, pL2_addr->vid, pL2_addr->port, pL2_addr->flags
            , pL2_addr->trk_gid, pL2_addr->state);

    if (!(pL2_addr->flags & RTK_L2_UCAST_FLAG_TRUNK_PORT))
    {
    for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
    {
        if (dal_cypress_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
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
        if ((ret = dal_cypress_trunk_port_get(unit, pL2_addr->trk_gid, &trunk_portmask)) != RT_ERR_OK)
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
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid = pL2_addr->vid;
    osal_memcpy(&l2_entry.unicast.mac, &(pL2_addr->mac), sizeof(rtk_mac_t));

    if (HAL_IS_ESCHIP(unit))
    {
        /* add next hop */
        if ((pL2_addr->flags & RTK_L2_UCAST_FLAG_NEXTHOP) &&
                (0xFFFFFFFF != pL2_addr->route_idx))
        {
            rtk_l3_routeEntry_t l3_entry;

            /* check next hop MAC */
            osal_memset(&l3_entry, 0, sizeof(rtk_l3_routeEntry_t));
            if (RT_ERR_OK != (ret = dal_cypress_l3_routeEntry_get(unit,
                    pL2_addr->route_idx, &l3_entry)))
            {
                RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                return ret;
            }

            if (0 != osal_memcmp(&pL2_addr->mac, &l3_entry.hostMac, sizeof(rtk_mac_t)))
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L3), "route idx:%d MAC mismatch", pL2_addr->route_idx);
                return RT_ERR_FAILED;
            }

            /* add/update L2 next hop entry */
            return dal_cypress_l2_addr_add(unit, pL2_addr);
        }
        /* remove next hop */
        else if ((!(pL2_addr->flags & RTK_L2_UCAST_FLAG_NEXTHOP)) &&
                (0xFFFFFFFF == pL2_addr->route_idx))
        {
            l2_entry.unicast.nh = TRUE;
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "nh is True");
        }
    }

    L2_SEM_LOCK(unit);

    if ((ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "%d", ret);
        return ret;
    }

    if (HAL_IS_ESCHIP(unit))
    {
        if ((!(pL2_addr->flags & RTK_L2_UCAST_FLAG_NEXTHOP)) &&
                (0xFFFFFFFF == pL2_addr->route_idx))
        {
            /* del L2 next hop entry */
            osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
            l2_entry.entry_type = L2_UNICAST;

            if (L2_IN_HASH == index_entry.index_type)
            {
                /* if found entry is in HASH, programming in HASH */
                ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
            }
            else
            {
                ret = RT_ERR_FAILED;
            }

            L2_SEM_UNLOCK(unit);
            return ret;
        }
    }

    /* fill content */
    osal_memcpy(&l2_entry.unicast.mac, &(pL2_addr->mac), sizeof(rtk_mac_t));
    l2_entry.unicast.fid        = pL2_addr->vid;
    l2_entry.unicast.port       = pL2_addr->port;
    l2_entry.unicast.aging      = 7;
    l2_entry.unicast.sablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_SA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.dablock    = (pL2_addr->flags & RTK_L2_UCAST_FLAG_DA_BLOCK) ? TRUE: FALSE;
    l2_entry.unicast.is_static  = (pL2_addr->flags & RTK_L2_UCAST_FLAG_STATIC) ? TRUE: FALSE;
    l2_entry.unicast.suspending = (pL2_addr->state & RTK_L2_UCAST_STATE_SUSPEND) ? TRUE: FALSE;
    if (pL2_addr->flags & RTK_L2_UCAST_FLAG_NEXTHOP)
    {
        l2_entry.unicast.nh          = TRUE;
        l2_entry.unicast.vlan_target = pL2_addr->vlan_target;
        l2_entry.unicast.route_idx   = pL2_addr->route_idx;
        if (pL2_addr->isAged == TRUE)
        {
            l2_entry.unicast.aging = 0;
            l2_entry.unicast.port  = 63;
        }
    }
    else
    {
        l2_entry.unicast.nh          = FALSE;
        l2_entry.unicast.agg_vid     = pL2_addr->agg_vid;
    }

    if (L2_IN_HASH == index_entry.index_type)
    {
        /* if found entry is in HASH, programming in HASH */
        ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
        /* return the entry index */
        pL2_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    }
    else if (L2_IN_CAM == index_entry.index_type)
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
        /* return the entry index */
        pL2_addr->l2_idx = index_entry.index;
    }
    else
    {
        ret = RT_ERR_FAILED;
    }

    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_cypress_l2_addr_set */

/* Function Name:
 *      dal_cypress_l2_addr_delAll
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note:
 *      None
 */
int32
dal_cypress_l2_addr_delAll(uint32 unit, uint32 include_static)
{
    int32   ret;

    if (HAL_IS_ESCHIP(unit))
    {
        uint32  l2_idx, cam_index;
        uint32  l2_tableSize, l2cam_tableSize;
        uint32  hashkey, location;
        uint32  isValid;
        dal_cypress_l2_entry_t  l2_entry;
        dal_cypress_l2_index_t  index_entry;

        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, include_static=%d", unit, include_static);

        /* check Init status */
        RT_INIT_CHK(l2_init[unit]);

        if ((ret = table_size_get(unit, CYPRESS_L2_UCt, &l2_tableSize)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        if ((ret = table_size_get(unit, CYPRESS_L2_CAM_UCt, &l2cam_tableSize)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        for (l2_idx = 0; l2_idx < l2_tableSize; l2_idx++)
        {
            hashkey = l2_idx >> 2;
            location = l2_idx & BITMASK_2B;

            ret = _dal_cypress_l2_getL2EntryfromHash(unit, hashkey, location, &l2_entry, &isValid);
            if ((TRUE == isValid) && (L2_UNICAST == l2_entry.entry_type) && (!HAL_IS_CPU_PORT(unit, l2_entry.unicast.port)))
            {
                if (L2_UNICAST == l2_entry.entry_type)
                {
                    if ((FALSE == include_static) && (1 == l2_entry.unicast.is_static))
                    {
                        /* if not need to include static and this entry is static, just skip this entry */
                        continue;
                    }
                    if (HAL_IS_ESCHIP(unit) && 1 == l2_entry.unicast.nh)
                    {
                        /* nh entry is not included in this API */
                        continue;
                    }
                }

                osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
                l2_entry.entry_type = L2_UNICAST;
                index_entry.index = hashkey;
                index_entry.hashdepth = location;
                ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);

            }
        }


        for (cam_index = 0; l2CamEbl && (cam_index < l2cam_tableSize); cam_index++)
        {
            ret = _dal_cypress_l2_getL2EntryfromCAM(unit, cam_index, &l2_entry, &isValid);
            if ((TRUE == isValid) && (L2_UNICAST == l2_entry.entry_type) && (!HAL_IS_CPU_PORT(unit, l2_entry.unicast.port)))
            {
                if (L2_UNICAST == l2_entry.entry_type)
                {
                    if ((FALSE == include_static) && (1 == l2_entry.unicast.is_static))
                    {
                        /* if not need to include static and this entry is static, just skip this entry */
                        continue;
                    }
                    if (HAL_IS_ESCHIP(unit) && 1 == l2_entry.unicast.nh)
                    {
                        /* nh entry is not included in this API */
                        continue;
                    }
                }

                osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
                l2_entry.entry_type = L2_IN_CAM;
                index_entry.index = cam_index;
                index_entry.hashdepth = 0;
                ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);

            }
        }
    }
    else
    {
        uint32      zero = 0;
        uint32      value, data;

        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, include_static=%d", unit, include_static);

        /* check Init status */
        RT_INIT_CHK(l2_init[unit]);

        /* get data from CHIP*/
        L2_SEM_LOCK(unit);
        if ((ret = reg_read(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, &data)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        L2_SEM_UNLOCK(unit);

        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_FVID_CMPf, &zero, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        value = 0;
        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_PORT_CMPf, &value, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_FVIDf, &zero, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_PORTf, &zero, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        value = include_static ? 0 : 1;  /* 0:whole table   1:dynamic only */
        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_ENTRY_TYPEf, &value, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        value = 0;  /* flush */
        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_ACTf, &value, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        value = 1;  /* start */
        if ((ret = reg_field_set(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, CYPRESS_STSf, &value, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        L2_SEM_LOCK(unit);
        /* program value to CHIP*/
        if ((ret = reg_write(unit, CYPRESS_L2_TBL_FLUSH_CTRLr, &data)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        L2_SEM_UNLOCK(unit);
    }


    return RT_ERR_OK;
} /* end of dal_cypress_l2_addr_delAll */


/* Function Name:
 *      dal_cypress_l2_nextValidAddr_get
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
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Note:
 *      1. The function will skip valid l2 multicast and ip multicast entry and
 *         reply next valid L2 unicast address is based on index order of l2 table.
 *      2. Please input -1 for get the first entry of l2 table.
 *      3. The *pScan_idx is the input and also is the output argument.
 */
int32
dal_cypress_l2_nextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    uint32              include_static,
    rtk_l2_ucastAddr_t  *pL2_data)
{
    int32  ret;
    dal_cypress_l2_entry_t  l2_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK((NULL == pScan_idx), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pL2_data), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((include_static > 1), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, include_static=%d", unit, *pScan_idx, include_static);

    osal_memset(pL2_data, 0, sizeof(rtk_l2_ucastAddr_t));

    L2_SEM_LOCK(unit);

    if ((ret = _dal_cypress_l2_nextValidAddr_get(unit, pScan_idx, L2_UNICAST, include_static, &l2_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    osal_memcpy(&pL2_data->mac, &l2_entry.unicast.mac, sizeof(rtk_mac_t));
    pL2_data->vid       = l2_entry.unicast.fid;
    pL2_data->port      = l2_entry.unicast.port;
    if(l2_entry.unicast.nh == TRUE)
    {
        pL2_data->vlan_target = l2_entry.unicast.vlan_target;
        pL2_data->route_idx = l2_entry.unicast.route_idx;
    }
    else
        pL2_data->agg_vid = l2_entry.unicast.agg_vid;


    if(l2_entry.unicast.sablock)
        pL2_data->flags = RTK_L2_UCAST_FLAG_SA_BLOCK;
    if(l2_entry.unicast.dablock)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
    if(l2_entry.unicast.is_static)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_STATIC;
    if(l2_entry.unicast.nh)
        pL2_data->flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
    if(l2_entry.unicast.suspending)
        pL2_data->state |= RTK_L2_UCAST_STATE_SUSPEND;
    if(l2_entry.unicast.aging == 0)
        pL2_data->isAged = TRUE;
    else
        pL2_data->isAged = FALSE;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "vid=%d, port=%d, sa_block=%d, da_block=%d, is_static=%d, nh=%d, suspend=%d",
        pL2_data->vid, pL2_data->port, l2_entry.unicast.sablock, l2_entry.unicast.dablock, l2_entry.unicast.is_static, l2_entry.unicast.nh, l2_entry.unicast.suspending);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_nextValidAddr_get */


/* Module Name    : L2           */
/* Sub-module Name: l2 multicast */

/* Function Name:
 *      dal_cypress_l2_mcastAddr_init
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
dal_cypress_l2_mcastAddr_init(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac, rtk_l2_mcastAddr_t *pMcast_addr)
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
}

/* Function Name:
 *      dal_cypress_l2_mcastAddr_add
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
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Need to initialize L2 multicast entry before add it.
 */
int32
dal_cypress_l2_mcastAddr_add(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32   ret;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    pMcast_addr->fwdIndex = -1; /* for automatically allocate */

    ret = dal_cypress_l2_mcastAddr_add_with_index(unit, pMcast_addr);

    return ret;

} /* end of dal_cypress_l2_mcastAddr_add */


/* Function Name:
 *      dal_cypress_l2_mcastAddr_get
 * Description:
 *      Get L2 multicast entry based on specified vid and MAC address.
 * Input:
 *      unit      - unit id
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_mcastAddr_get(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32 ret;

    ret = dal_cypress_l2_mcastAddr_get_with_index(unit, pMcast_addr);

    return ret;
} /* end of dal_cypress_l2_mcastAddr_get */

/* Function Name:
 *      dal_cypress_l2_mcastAddr_set
 * Description:
 *      Update content of L2 multicast entry.
 * Input:
 *      unit       - unit id
 * Output:
 *      pMcast_addr - pointer to L2 multicast entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_mcastAddr_set(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32 ret;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMcast_addr->mac.octet[0] & BITMASK_1B) != 1, RT_ERR_MAC);
    RT_PARAM_CHK((pMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_mcastAddr_set: unit=%d, vid=%d, pMac=%x-%x-%x-%x-%x-%x \
           pPortmask=%x", unit, pMcast_addr->rvid, pMcast_addr->mac.octet[0], pMcast_addr->mac.octet[1], pMcast_addr->mac.octet[2],
           pMcast_addr->mac.octet[3], pMcast_addr->mac.octet[4], pMcast_addr->mac.octet[5], pMcast_addr->portmask.bits[0]);

    /* search exist or free entry */
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type = L2_MULTICAST;
    l2_entry.l2mcast.rvid    = pMcast_addr->rvid;
    osal_memcpy(&l2_entry.l2mcast.mac, &pMcast_addr->mac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);

    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);

    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

    if ((ret = table_field_set(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &pMcast_addr->portmask.bits[0], (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return RT_ERR_FAILED;
    }

    if ((ret = table_write(unit, CYPRESS_MC_PMSKt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return RT_ERR_FAILED;
    }

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* update multicast shadow */
        RTK_PORTMASK_ASSIGN(mcast_fwdTable_shadow[l2_entry.l2mcast.index].portmask, pMcast_addr->portmask);
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_mcastAddr_set */

/* Function Name:
 *      dal_cypress_l2_mcastAddr_del
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
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_VLAN_VID       - invalid vlan id
 *      RT_ERR_MAC            - invalid mac address
 *      RT_ERR_L2_EMPTY_ENTRY - the entry is empty(invalid)
 * Note:
 *      None
 */
int32
dal_cypress_l2_mcastAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;

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
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type = L2_MULTICAST;
    l2_entry.l2mcast.rvid = vid;
    osal_memcpy(&l2_entry.l2mcast.mac, pMac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);

    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);

    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "%d", ret);
        return ret;
    }

    _dal_cypress_l2_freeMcastIdx(unit, (int32)l2_entry.l2mcast.index);

    osal_memset(&l2_entry.l2mcast.mac, 0, sizeof(rtk_mac_t));
    l2_entry.l2mcast.rvid    = 0;
    l2_entry.l2mcast.index   = 0;

    if (L2_IN_HASH == index_entry.index_type)
    {
        /* if found entry is in HASH, programming in HASH */
        ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    }
    else if (L2_IN_CAM == index_entry.index_type)
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }

    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_cypress_l2_mcastAddr_del */

/* Function Name:
 *      dal_cypress_l2_mcastAddr_add_with_index
 * Description:
 *      Add a L2 multicast address entry and multicast index to the specified device.
 * Input:
 *      unit                   - unit id
 *      vid                    - vlan id
 *      pMcast_addr            - content of L2 multicast address entry
 *      pMacast_addr->fwdIndex - index of portmask entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_MAC                  - invalid mac address
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_ENTRY_EXIST       - the entry is already existed
 * Note:
 *      (1) pMacast_addr->fwdIndex is used for pointing to portmask entry if its value is larger than or equal to 0.
 *          In this case, pMacast_addr->portmask is not used.
 *
 *      (2) Driver automatically allocates a free portmask entry index and return it back to pMacast_addr->fwdIndex
 *          if pMacast_addr->fwdIndex is smaller than 0. In this case, pMacast_addr->portmask is used to
 *          configure the allocated portmask entry.
 */
int32
dal_cypress_l2_mcastAddr_add_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32 ret, ret_1;
    int32 fwdIndex;
    dal_cypress_l2_entry_t  l2_entry, dynamic_l2_entry;
    dal_cypress_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMcast_addr->mac.octet[0] & BITMASK_1B) != 1, RT_ERR_MAC);
    RT_PARAM_CHK(pMcast_addr->fwdIndex >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    RT_PARAM_CHK((pMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_mcastAddr_add_with_index: unit=%d, rvid=%d, pMac=%x-%x-%x-%x-%x-%x \
           pPortmask=%x, pIndex=%d", unit, pMcast_addr->rvid, pMcast_addr->mac.octet[0], pMcast_addr->mac.octet[1], pMcast_addr->mac.octet[2], pMcast_addr->mac.octet[2],
           pMcast_addr->mac.octet[3], pMcast_addr->mac.octet[4], pMcast_addr->mac.octet[5], pMcast_addr->portmask.bits[0], pMcast_addr->fwdIndex);

    /* search exist or free entry */
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type = L2_MULTICAST;
    l2_entry.l2mcast.rvid = pMcast_addr->rvid;
    osal_memcpy(&l2_entry.l2mcast.mac, &(pMcast_addr->mac), sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);

    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry);
    if (ret == RT_ERR_L2_NO_EMPTY_ENTRY && (pMcast_addr->add_op_flags & RTK_L2_ADD_OP_FLAG_REPLACE_DYNAMIC) != 0)
    {
        osal_memcpy(&dynamic_l2_entry, &l2_entry, sizeof(dal_cypress_l2_entry_t));
        dynamic_l2_entry.entry_type  = L2_MULTICAST;

        ret_1 = _dal_cypress_l2_getFirstDynamicEntry(unit, &dynamic_l2_entry, &index_entry);
        if (ret_1)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret_1;
        }
    }
    else if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    if (TRUE == l2_entry.is_entry_exist)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "entry exist");
        return RT_ERR_L2_ENTRY_EXIST;
    }


    if (index_entry.index_type == L2_IN_HASH)
        pMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pMcast_addr->l2_idx = (1 << 14) + index_entry.index;

    fwdIndex = pMcast_addr->fwdIndex;

    /* get a free multicast index */
    if ((ret = _dal_cypress_l2_allocMcastIdx(unit, &(pMcast_addr->fwdIndex))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

    if (fwdIndex < 0)
    { /* Will configure portmask when fwdIndex is allocate automatically */
        if ((ret = table_field_set(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &(pMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return RT_ERR_FAILED;
        }

        if ((ret = table_write(unit, CYPRESS_MC_PMSKt, (uint32)pMcast_addr->fwdIndex, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return RT_ERR_FAILED;
        }

        if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
        {
            /* update multicast shadow */
            RTK_PORTMASK_ASSIGN(mcast_fwdTable_shadow[pMcast_addr->fwdIndex].portmask, pMcast_addr->portmask);
        }
    }

    L2_SEM_UNLOCK(unit);

    /* fill content */
    osal_memcpy(&l2_entry.l2mcast.mac, &(pMcast_addr->mac), sizeof(rtk_mac_t));
    l2_entry.l2mcast.rvid    = pMcast_addr->rvid;
    l2_entry.l2mcast.index   = (uint32)(pMcast_addr->fwdIndex);

    if (L2_IN_HASH == index_entry.index_type)
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    }
    else if(L2_IN_CAM == index_entry.index_type)
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }

    if (RT_ERR_OK != ret)
    {
        _dal_cypress_l2_freeMcastIdx(unit, (int32)l2_entry.l2mcast.index);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_mcastAddr_add_with_index */

/* Function Name:
 *      dal_cypress_l2_mcastAddr_get_with_index
 * Description:
 *      Get a L2 multicast address entry and multicast index from the specified device.
 * Input:
 *      unit        - unit id
 * Output:
 *      pMcast_addr - pointer to content of L2 multicast address entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vlan id
 *      RT_ERR_MAC                  - invalid mac address
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 *      None.
 */
int32
dal_cypress_l2_mcastAddr_get_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    int32 ret;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_mcastAddr_get: unit=%d, vid=%d, pMac=%x-%x-%x-%x-%x-%x",
           unit, pMcast_addr->rvid, pMcast_addr->mac.octet[0], pMcast_addr->mac.octet[1], pMcast_addr->mac.octet[2], pMcast_addr->mac.octet[2],
           pMcast_addr->mac.octet[3], pMcast_addr->mac.octet[4], pMcast_addr->mac.octet[5]);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pMcast_addr->mac.octet[0] & BITMASK_1B) != 1, RT_ERR_MAC);
    RT_PARAM_CHK((pMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* search exist or free entry */
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type     = L2_MULTICAST;
    l2_entry.l2mcast.rvid   = pMcast_addr->rvid;
    osal_memcpy(&l2_entry.l2mcast.mac, &(pMcast_addr->mac), sizeof(rtk_mac_t));

    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);

    if (RT_ERR_OK != ret)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    pMcast_addr->fwdIndex = (int32)l2_entry.l2mcast.index;

    if (index_entry.index_type == L2_IN_HASH)
        pMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pMcast_addr->l2_idx = (1 << 14) + index_entry.index;

    L2_SEM_LOCK(unit);
    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* get multicast forward from shadow */
        RTK_PORTMASK_ASSIGN(pMcast_addr->portmask, mcast_fwdTable_shadow[l2_entry.l2mcast.index].portmask);
        L2_SEM_UNLOCK(unit);
    }
    else
    {
        if ((ret = table_read(unit, CYPRESS_MC_PMSKt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return RT_ERR_FAILED;
        }
        L2_SEM_UNLOCK(unit);

        if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &(pMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return RT_ERR_FAILED;
        }
    }

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_mcastAddr_get: pPortmask=%x", pMcast_addr->portmask.bits[0]);

    return ret;
} /* end of dal_cypress_l2_mcastAddr_get_with_index */

/* Function Name:
 *      dal_cypress_l2_nextValidMcastAddr_get
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
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. The function will skip valid l2 unicast and ip multicast entry and
 *         reply next valid L2 multicast address is based on index order of l2 table.
 *      2. Please input -1 for get the first entry of l2 table.
 *      3. The *pScan_idx is the input and also is the output argument.
 */
int32
dal_cypress_l2_nextValidMcastAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    rtk_l2_mcastAddr_t  *pL2_data)
{
    int32  ret;
    dal_cypress_l2_entry_t  l2_entry;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK((NULL == pScan_idx), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pL2_data), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d", unit, *pScan_idx);

    L2_SEM_LOCK(unit);
    if ((ret = _dal_cypress_l2_nextValidAddr_get(unit, pScan_idx, L2_MULTICAST, FALSE, &l2_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    pL2_data->rvid = l2_entry.l2mcast.rvid;
    osal_memcpy(&pL2_data->mac, &l2_entry.l2mcast.mac, sizeof(rtk_mac_t));

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* get multicast forward from shadow */
        RTK_PORTMASK_ASSIGN(pL2_data->portmask, mcast_fwdTable_shadow[l2_entry.l2mcast.index].portmask);
    }
    else
    {
        osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
        if ((ret = table_read(unit, CYPRESS_MC_PMSKt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, pL2_data->portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, rvid=%d, mac=%x-%x-%x-%x-%x-%x, portmask=0x%x",
           unit, *pScan_idx, pL2_data->rvid, pL2_data->mac.octet[0], pL2_data->mac.octet[1], pL2_data->mac.octet[2], pL2_data->mac.octet[3],
           pL2_data->mac.octet[4], pL2_data->mac.octet[5], pL2_data->portmask.bits[0]);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_nextValidMcastAddr_get */


/* Module Name    : L2           */
/* Sub-module Name: ip multicast */

/* Function Name:
 *      dal_cypress_l2_ipmcMode_get
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
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Lookup mode of layer2 ip multicast switching for RTL8390 is as following
 *      - LOOKUP_ON_FVID_AND_MAC
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_AND_FVID
 */
int32
dal_cypress_l2_ipmcMode_get(uint32 unit, rtk_l2_ipmcMode_t *pMode)
{
    int32   ret;
    uint32  format = 0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_IPV4_MC_HASH_KEY_FMTf, &format)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    switch (format)
    {
        case 0:
            *pMode = LOOKUP_ON_FVID_AND_MAC;
            break;
        case 1:
            *pMode = LOOKUP_ON_DIP_AND_SIP;
            break;
        case 2:
            *pMode = LOOKUP_ON_DIP_AND_FVID;
            break;
        default:
            return RT_ERR_FAILED;
    }
    ip4HashFmt[unit] = *pMode;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMode=%ld", *pMode);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_ipmcMode_get */

/* Function Name:
 *      dal_cypress_l2_ipmcMode_set
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
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Lookup mode of layer2 ip multicast switching for RTL8390 is as following
 *      - LOOKUP_ON_FVID_AND_MAC
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_AND_FVID
 */
int32
dal_cypress_l2_ipmcMode_set(uint32 unit, rtk_l2_ipmcMode_t mode)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, mode=%d", unit, mode);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(mode >= IPMC_MODE_END, RT_ERR_INPUT);

    switch (mode)
    {
        case LOOKUP_ON_FVID_AND_MAC:
            value = 0;
            break;
        case LOOKUP_ON_DIP_AND_SIP:
            value = 1;
            break;
        case LOOKUP_ON_DIP_AND_FVID:
            value = 2;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_IPV4_MC_HASH_KEY_FMTf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    ip4HashFmt[unit] = value;

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/* end of dal_cypress_l2_ipmcMode_set */

/* Function Name:
 *      dal_cypress_l2_ipMcastAddrExt_init
 * Description:
 *      Initialize content of buffer of IP multicast entry.
 *      Will destination IP ,source IP and reset other field of IP multicast entry.
 * Input:
 *      unit                - unit id
 *      pIpMcast_hashKey    - the hash key to initialize content of buffer
 *      pIpMcast_addr       - IP multicast entry
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
dal_cypress_l2_ipMcastAddrExt_init(uint32 unit, rtk_l2_ipMcastHashKey_t *pIpMcast_hashKey, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pIpMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((pIpMcast_hashKey->dip >> 28) & BITMASK_4B) != 0xE, RT_ERR_IPV4_ADDRESS);
    if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
        RT_PARAM_CHK((pIpMcast_hashKey->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    osal_memset(pIpMcast_addr, 0, sizeof(rtk_l2_ipMcastAddr_t));

    pIpMcast_addr->dip = pIpMcast_hashKey->dip;
    if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
        pIpMcast_addr->rvid = pIpMcast_hashKey->rvid;
    else if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
        pIpMcast_addr->sip = pIpMcast_hashKey->sip;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_ipMcastAddr_add
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
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 *      RT_ERR_VLAN_VID             - invalid VLAN ID
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_L2_ENTRY_EXIST       - the entry is already existed
 * Note:
 *      None.
 */
int32
dal_cypress_l2_ipMcastAddr_add(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    int32 ret;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    pIpmcast_addr->fwdIndex = -1;

    ret = dal_cypress_l2_ipMcastAddr_add_with_index(unit, pIpmcast_addr);

    return ret;
} /* end of dal_cypress_l2_ipMcastAddr_add */

/* Function Name:
 *      dal_cypress_l2_ipMcastAddr_get
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
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32
dal_cypress_l2_ipMcastAddr_get(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    int32 ret;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    ret = dal_cypress_l2_ipMcastAddr_get_with_index(unit, pIpmcast_addr);

    return ret;
} /* end of dal_cypress_l2_ipMcastAddr_get */

/* Function Name:
 *      dal_cypress_l2_ipMcastAddr_set
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
 *      RT_ERR_IPV4_ADDRESS - Invalid IPv4 address
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE   - invalid IP multicast lookup mode
 * Note:
 *      None
 */
int32
dal_cypress_l2_ipMcastAddr_set(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    int32   ret;
    uint32  index;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_ipMcastAddr_set: unit=%d, sip=%x, dip=%x, vid=%d, pPortmask=%x",
           unit, pIpmcast_addr->sip, pIpmcast_addr->dip, pIpmcast_addr->rvid, pIpmcast_addr->portmask.bits[0]);

    RT_PARAM_CHK(NULL == pIpmcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((pIpmcast_addr->dip >>28) & BITMASK_4B) != 0xE, RT_ERR_IPV4_ADDRESS);
    RT_PARAM_CHK((pIpmcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));

    if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc_sip.rvid = pIpmcast_addr->rvid;
        l2_entry.ipmcast_ip_mc_sip.dip  = pIpmcast_addr->dip;
        l2_entry.ipmcast_ip_mc_sip.sip  = pIpmcast_addr->sip;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc.rvid = pIpmcast_addr->rvid;
        l2_entry.ipmcast_ip_mc.dip  = pIpmcast_addr->dip;
    }
    else
    {
        return RT_ERR_L2_IPMCAST_LOOKUP_MODE;
    }

    l2_entry.entry_type = IP4_MULTICAST;

    L2_SEM_LOCK(unit);

    /* search exist or free entry */
    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);

    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

    if ((ret = table_field_set(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, pIpmcast_addr->portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
         RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
         return ret;
    }

    if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
        index = l2_entry.ipmcast_ip_mc_sip.index;
    else
        index = l2_entry.ipmcast_ip_mc.index;
    if ((ret = table_write(unit, CYPRESS_MC_PMSKt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* update multicast shadow */
        RTK_PORTMASK_ASSIGN(mcast_fwdTable_shadow[index].portmask, pIpmcast_addr->portmask);
    }

    /* Update fvid if needs */
    if (ipmcst_fvid_cmp == FALSE && LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit] && l2_entry.ipmcast_ip_mc_sip.rvid != pIpmcast_addr->rvid)
    {
        l2_entry.ipmcast_ip_mc_sip.rvid = pIpmcast_addr->rvid;
        if (L2_IN_HASH == index_entry.index_type)
        {
            /* if found entry is in HASH, programming in SRAM */
            ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
        }
        else if(L2_IN_CAM == index_entry.index_type)
        {
            /* if found entry is in CAM, programming in CAM */
            ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
        }
    }
    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_cypress_l2_ipMcastAddr_set */

/* Function Name:
 *      dal_cypress_l2_ipMcastAddr_del
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
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_VLAN_VID                 - invalid vlan id
 *      RT_ERR_L2_EMPTY_ENTRY - the entry is empty(invalid)
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE   - invalid IP multicast lookup mode
 * Note:
 *      (1) In vlan unaware mode (SVL), the vid will be ignored, suggest to
 *         input vid=0 in vlan unaware mode.
 *      (2) In vlan aware mode (IVL), the vid is used.
 */
int32
dal_cypress_l2_ipMcastAddr_del(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_vlan_t vid)
{
    int32 ret;
    int32 mcFwdIndex;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, sip=%x, dip=%x, vid=%d",
           unit, sip, dip, vid);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    /* parameter check */
    RT_PARAM_CHK((vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* search exist or free entry */
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc_sip.rvid = vid;
        l2_entry.ipmcast_ip_mc_sip.dip  = dip;
        l2_entry.ipmcast_ip_mc_sip.sip  = sip;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc.rvid = vid;
        l2_entry.ipmcast_ip_mc.dip  = dip;
    }
    else
    {
        return RT_ERR_L2_IPMCAST_LOOKUP_MODE;
    }

    l2_entry.entry_type   = IP4_MULTICAST;

    L2_SEM_LOCK(unit);

    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);

    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "ret=%d", ret);
        return ret;
    }

    if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
        mcFwdIndex = l2_entry.ipmcast_ip_mc_sip.index;
    else
        mcFwdIndex = l2_entry.ipmcast_ip_mc.index;

    ret = _dal_cypress_l2_freeMcastIdx(unit, (int32)mcFwdIndex);
    if(ret != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return RT_ERR_L2_IPMULTI_PARAM;
    }

    if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc_sip.rvid = 0;
        l2_entry.ipmcast_ip_mc_sip.dip  = 0;
        l2_entry.ipmcast_ip_mc_sip.sip  = 0;
        l2_entry.ipmcast_ip_mc_sip.index = 0;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc.rvid = 0;
        l2_entry.ipmcast_ip_mc.dip  = 0;
        l2_entry.ipmcast_ip_mc.index = 0;
    }

    if (L2_IN_HASH == index_entry.index_type)
    {
        /* if found entry is in HASH, programming in SRAM */
        ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    }
    else if (L2_IN_CAM == index_entry.index_type)
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }

    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_cypress_l2_ipMcastAddr_del */

/* Function Name:
 *      dal_cypress_l2_ipMcastAddr_add_with_index
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
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 *      RT_ERR_VLAN_VID                 - invalid VLAN ID
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE   - invalid IP multicast lookup mode
 *      RT_ERR_L2_ENTRY_EXIST           - the entry is already existed
 * Note:
 *      If fwdIndex is larger than or equal to 0, will use fwdIndex as multicast index
 *          and won't config portmask.
 *
 *      If fwdIndex is smaller than 0, will allocate a free index and return it.
 *          It will also config portmask.
 */
int32
dal_cypress_l2_ipMcastAddr_add_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    int32 ret, ret_1;
    int32 fwdIndex;
    dal_cypress_l2_entry_t  l2_entry, dynamic_l2_entry;
    dal_cypress_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    /* parameter check */
    RT_PARAM_CHK(NULL == pIpMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pIpMcast_addr->fwdIndex >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    RT_PARAM_CHK((pIpMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(((pIpMcast_addr->dip >>28) & BITMASK_4B) != 0xE, RT_ERR_L2_IPMULTI_PARAM);

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_ipMcastAddr_add_with_index: unit=%d, sip=%x, dip=%x, vid=%d, pPortmask=%x",
           unit, pIpMcast_addr->sip, pIpMcast_addr->dip, pIpMcast_addr->rvid, pIpMcast_addr->portmask.bits[0]);

    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));

    if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc_sip.rvid = pIpMcast_addr->rvid;
        l2_entry.ipmcast_ip_mc_sip.dip  = pIpMcast_addr->dip;
        l2_entry.ipmcast_ip_mc_sip.sip  = pIpMcast_addr->sip;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc.rvid = pIpMcast_addr->rvid;
        l2_entry.ipmcast_ip_mc.dip  = pIpMcast_addr->dip;
    }
    else
    {
        return RT_ERR_L2_IPMCAST_LOOKUP_MODE;
    }

    l2_entry.entry_type = IP4_MULTICAST;

    L2_SEM_LOCK(unit);

    /* search exist or free entry */
    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry);

    if (ret == RT_ERR_L2_NO_EMPTY_ENTRY && (pIpMcast_addr->add_op_flags & RTK_L2_ADD_OP_FLAG_REPLACE_DYNAMIC) != 0)
    {
        osal_memcpy(&dynamic_l2_entry, &l2_entry, sizeof(dal_cypress_l2_entry_t));
        dynamic_l2_entry.entry_type  = IP4_MULTICAST;

        ret_1 = _dal_cypress_l2_getFirstDynamicEntry(unit, &dynamic_l2_entry, &index_entry);
        if (ret_1)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret_1;
        }
    }
    else if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    if (TRUE == l2_entry.is_entry_exist)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "entry exist");
        return RT_ERR_L2_ENTRY_EXIST;
    }

    if (index_entry.index_type == L2_IN_HASH)
        pIpMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pIpMcast_addr->l2_idx = (1 << 14) + index_entry.index;

    fwdIndex = pIpMcast_addr->fwdIndex;

    /* get a free multicast index or increase reference count of mcast index */
    if ((ret = _dal_cypress_l2_allocMcastIdx(unit, &(pIpMcast_addr->fwdIndex))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

    if (fwdIndex < 0)
    { /* Will configure portmask when fwdIndex is allocate automatically */
        if ((ret = table_field_set(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &(pIpMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }

        if ((ret = table_write(unit, CYPRESS_MC_PMSKt, (uint32)(pIpMcast_addr->fwdIndex), (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }

        if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
        {
            /* update multicast shadow */
            RTK_PORTMASK_ASSIGN(mcast_fwdTable_shadow[pIpMcast_addr->fwdIndex].portmask, pIpMcast_addr->portmask);
        }
    }

    L2_SEM_UNLOCK(unit);

    if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
        l2_entry.ipmcast_ip_mc_sip.index = (uint32)(pIpMcast_addr->fwdIndex);
    else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
        l2_entry.ipmcast_ip_mc.index = (uint32)(pIpMcast_addr->fwdIndex);

    if (L2_IN_HASH == index_entry.index_type)
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    }
    else if(L2_IN_CAM == index_entry.index_type)
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }

    if (RT_ERR_OK != ret)
    {
        _dal_cypress_l2_freeMcastIdx(unit, (int32)l2_entry.l2mcast.index);
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_ipMcastAddr_add_with_index */

/* Function Name:
 *      dal_cypress_l2_ipMcastAddr_get_with_index
 * Description:
 *      Get a IP multicast address entry and multicast index from the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pIpMcast_addr - pointer to content of IP multicast address entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_VLAN_VID                 - invalid VLAN ID
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE   - invalid IP multicast lookup mode
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 *      None.
 */
int32
dal_cypress_l2_ipMcastAddr_get_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    int32   ret;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_ipMcastAddr_get: unit=%d, sip=%x, dip=%x, vid=%d",
           unit, pIpMcast_addr->sip, pIpMcast_addr->dip, pIpMcast_addr->rvid);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    /* parameter check */
    RT_PARAM_CHK(NULL == pIpMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pIpMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* search exist or free entry */
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc_sip.rvid = pIpMcast_addr->rvid;
        l2_entry.ipmcast_ip_mc_sip.dip  = pIpMcast_addr->dip;
        l2_entry.ipmcast_ip_mc_sip.sip  = pIpMcast_addr->sip;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc.rvid = pIpMcast_addr->rvid;
        l2_entry.ipmcast_ip_mc.dip  = pIpMcast_addr->dip;
    }
    else
    {
        return RT_ERR_L2_IPMCAST_LOOKUP_MODE;
    }

    l2_entry.entry_type = IP4_MULTICAST;

    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);

    if (RT_ERR_OK != ret)
    {
        return ret;
    }

    /* User may don't know the rvid when ipmcst_fvid_cmp is FALSE. Therefore we should assign it again */
    if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
    {
        pIpMcast_addr->rvid = l2_entry.ipmcast_ip_mc_sip.rvid;
        pIpMcast_addr->fwdIndex = l2_entry.ipmcast_ip_mc_sip.index;
    }
    else
    {
        pIpMcast_addr->rvid = l2_entry.ipmcast_ip_mc.rvid;
        pIpMcast_addr->fwdIndex = l2_entry.ipmcast_ip_mc.index;
    }

    if (index_entry.index_type == L2_IN_HASH)
        pIpMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pIpMcast_addr->l2_idx = (1 << 14) + index_entry.index;

    L2_SEM_LOCK(unit);
    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* get multicast forward from shadow */
        RTK_PORTMASK_ASSIGN(pIpMcast_addr->portmask, mcast_fwdTable_shadow[pIpMcast_addr->fwdIndex].portmask);
        L2_SEM_UNLOCK(unit);
    }
    else
    {
        if ((ret = table_read(unit, CYPRESS_MC_PMSKt, pIpMcast_addr->fwdIndex, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }
        L2_SEM_UNLOCK(unit);

        if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &(pIpMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }
    }

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_ipMcastAddr_get: pPortmask=%x", pIpMcast_addr->portmask.bits[0]);

    return ret;
} /* end of dal_cypress_l2_ipMcastAddr_get_with_index */

/* Function Name:
 *      dal_cypress_l2_ipMcastAddrChkEnable_get
 * Description:
 *      Get DIP check status for IPv4 multicast packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to DIP check status for IPv4 multicast packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32
dal_cypress_l2_ipMcastAddrChkEnable_get(uint32 unit, rtk_enable_t *pEnable)
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
    if ((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_IP_MC_DIP_CHKf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
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
}  /* end of dal_cypress_l2_ipMcastAddrChkEnable_get */

/* Function Name:
 *      dal_cypress_l2_ipMcastAddrChkEnable_set
 * Description:
 *      Set DIP check status for IPv4 multicast packet.
 * Input:
 *      unit    - unit id
 *      enable  - DIP check status for IPv4 multicast packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_ipMcastAddrChkEnable_set(uint32 unit, rtk_enable_t enable)
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
    if ((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_IP_MC_DIP_CHKf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_ipMcastAddrChkEnable_set */

/* Function Name:
 *      dal_cypress_l2_ipMcstFidVidCompareEnable_get
 * Description:
 *      Get VID/FID comparison configuration of IP multicast entry.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to FID/VID comparison status for IPv4 multicast packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_ipMcstFidVidCompareEnable_get(uint32 unit, rtk_enable_t *pEnable)
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
    if ((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_IP_MC_FVID_CMPf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
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
} /* end of dal_cypress_l2_ipMcstFidVidCompareEnable_get */

/* Function Name:
 *      dal_cypress_l2_ipMcstFidVidCompareEnable_set
 * Description:
 *      Get VID/FID comparison configuration of IP multicast entry.
 * Input:
 *      unit    - unit id
 *      enable  - configure value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_ipMcstFidVidCompareEnable_set(uint32 unit, rtk_enable_t enable)
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
    if ((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_IP_MC_FVID_CMPf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    ipmcst_fvid_cmp = enable;

    return RT_ERR_OK;
} /* end of dal_cypress_l2_ipMcstFidVidCompareEnable_set */

/* Function Name:
 *      dal_cypress_l2_nextValidIpMcastAddr_get
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
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. The function will skip valid l2 unicast and multicast entry and
 *         reply next valid L2 ip multicast address is based on index order of l2 table.
 *      2. Please input -1 for get the first entry of l2 table.
 *      3. The *pScan_idx is the input and also is the output argument.
 */
int32
dal_cypress_l2_nextValidIpMcastAddr_get(
    uint32                  unit,
    int32                   *pScan_idx,
    rtk_l2_ipMcastAddr_t    *pL2_data)
{
    int32   ret;
    uint32  index = 0;
    dal_cypress_l2_entry_t      l2_entry;
    multicast_index_entry_t     mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK((NULL == pScan_idx), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pL2_data), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d",
           unit, *pScan_idx);

    L2_SEM_LOCK(unit);

    if ((ret = _dal_cypress_l2_nextValidAddr_get(unit, pScan_idx, IP4_MULTICAST, FALSE, &l2_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
    {
        pL2_data->rvid = l2_entry.ipmcast_ip_mc_sip.rvid;
        pL2_data->dip  = l2_entry.ipmcast_ip_mc_sip.dip;
        pL2_data->sip  = l2_entry.ipmcast_ip_mc_sip.sip;
        index           = l2_entry.ipmcast_ip_mc_sip.index;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
    {
        pL2_data->rvid = l2_entry.ipmcast_ip_mc.rvid;
        pL2_data->dip  = l2_entry.ipmcast_ip_mc.dip;
        index           = l2_entry.ipmcast_ip_mc.index;
    }
    else
    {
        L2_SEM_UNLOCK(unit);
        return RT_ERR_L2_IPMCAST_LOOKUP_MODE;
    }

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* get multicast forward from shadow */
        RTK_PORTMASK_ASSIGN(pL2_data->portmask, mcast_fwdTable_shadow[index].portmask);
    }
    else
    {
        if ((ret = table_read(unit, CYPRESS_MC_PMSKt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, pL2_data->portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, rvid=%d, dip=%x, sip=%x, portmask=0x%x",
           unit, *pScan_idx, pL2_data->rvid, pL2_data->dip, pL2_data->sip, pL2_data->portmask.bits[0]);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_nextValidIpMcastAddr_get */

/* Function Name:
 *      dal_cypress_l2_ip6mcMode_get
 * Description:
 *      Get lookup mode of layer2 IPv6 multicast switching.
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - pointer to lookup mode of layer2 IPv6 multicast switching
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Lookup mode of layer2 IPv6 multicast switching for RTL8390 is as following
 *      - LOOKUP_ON_FVID_AND_MAC
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_AND_FVID
 */
int32
dal_cypress_l2_ip6mcMode_get(uint32 unit, rtk_l2_ipmcMode_t *pMode)
{
    int32   ret;
    uint32  format = 0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_IPV6_MC_HASH_KEY_FMTf, &format)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    switch (format)
    {
        case 0:
            *pMode = LOOKUP_ON_FVID_AND_MAC;
            break;
        case 1:
            *pMode = LOOKUP_ON_DIP_AND_SIP;
            break;
        case 2:
            *pMode = LOOKUP_ON_DIP_AND_FVID;
            break;
        default:
            return RT_ERR_FAILED;
    }
    ip6HashFmt[unit] = *pMode;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pMode=%ld", *pMode);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_ip6mcMode_get */

/* Function Name:
 *      dal_cypress_l2_ip6mcMode_set
 * Description:
 *      Set lookup mode of layer2 IPv6 multicast switching.
 * Input:
 *      unit - unit id
 *      mode - lookup mode of layer2 IPv6 multicast switching
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Lookup mode of layer2 IPv6 multicast switching for RTL8390 is as following
 *      - LOOKUP_ON_FVID_AND_MAC
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_AND_FVID
 */
int32
dal_cypress_l2_ip6mcMode_set(uint32 unit, rtk_l2_ipmcMode_t mode)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, mode=%d", unit, mode);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((mode >= IPMC_MODE_END), RT_ERR_INPUT);

    switch (mode)
    {
        case LOOKUP_ON_FVID_AND_MAC:
            value = 0;
            break;
        case LOOKUP_ON_DIP_AND_SIP:
            value = 1;
            break;
        case LOOKUP_ON_DIP_AND_FVID:
            value = 2;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_IPV6_MC_HASH_KEY_FMTf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    ip6HashFmt[unit] = value;

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/* end of dal_cypress_l2_ip6mcMode_set */

/* Function Name:
 *      dal_cypress_l2_ip6CareByte_get
 * Description:
 *      Get the hash care-byte of IPv6 DIP/SIP address. These bytes are used to compose the LUT hash key.
 * Input:
 *      unit      - unit id
 *      type      - type of care-byte
 * Output:
 *      pCareByte - pointer to the care-byte of IPv6 DIP/SIP address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_ip6CareByte_get(uint32 unit, rtk_l2_ip6_careByte_type_t type, uint32 *pCareByte)
{
    int32   ret;
    uint32  field;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, type=%d", unit, type);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(type >= L2_CARE_BYTE_END, RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pCareByte), RT_ERR_NULL_POINTER);

    switch (type)
    {
        case L2_DIP_HASH_CARE_BYTE:
            field = CYPRESS_DIP_CARE_BYTEf;
            break;
        case L2_SIP_HASH_CARE_BYTE:
            field = CYPRESS_SIP_CARE_BYTEf;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, CYPRESS_L2_IPV6_MC_IP_CARE_BYTEr, field, pCareByte)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pCareByte=%#x", *pCareByte);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_ip6CareByte_get */

/* Function Name:
 *      dal_cypress_l2_ip6CareByte_set
 * Description:
 *      Set the hash care-byte of IPv6 DIP/SIP address. These bytes are used to compose the LUT hash key.
 * Input:
 *      unit        - unit id
 *      type        - type of care-byte
 *      careByte    - the care-byte of IPv6 DIP/SIP address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_ip6CareByte_set(uint32 unit, rtk_l2_ip6_careByte_type_t type, uint32 careByte)
{
    int32   ret;
    uint32  field;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, careByte=%d", unit, careByte);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(type >= L2_CARE_BYTE_END, RT_ERR_INPUT);

    switch (type)
    {
        case L2_DIP_HASH_CARE_BYTE:
            field = CYPRESS_DIP_CARE_BYTEf;
            break;
        case L2_SIP_HASH_CARE_BYTE:
            field = CYPRESS_SIP_CARE_BYTEf;
            break;
        default:
            return RT_ERR_INPUT;
            break;
    }

    L2_SEM_LOCK(unit);

    if((ret = reg_field_write(unit, CYPRESS_L2_IPV6_MC_IP_CARE_BYTEr, field, &careByte)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    if (type == L2_DIP_HASH_CARE_BYTE)
        ip6DipCareByte = careByte;
    else
        ip6SipCareByte = careByte;

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/* end of dal_cypress_l2_ip6CareByte_set */

/* Function Name:
 *      dal_cypress_l2_ip6McastAddrExt_init
 * Description:
 *      Initialize content of buffer of IPv6 multicast entry.
 *      Will destination IP ,source IP (or vid) and reset other field of IPv6 multicast entry.
 * Input:
 *      unit                - unit id
 *      pIp6Mcast_hashKey   - the hash key to initialize content of buffer
 *      pIp6Mcast_addr      - IPv6 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV6_ADDRESS     - Invalid IPv6 address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Need to initialize IPv6 multicast entry before add it.
 */
int32
dal_cypress_l2_ip6McastAddrExt_init(uint32 unit, rtk_l2_ip6McastHashKey_t *pIp6Mcast_hashKey, rtk_l2_ip6McastAddr_t *pIp6Mcast_addr)
{
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pIp6Mcast_hashKey, RT_ERR_NULL_POINTER);
    if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
        RT_PARAM_CHK((pIp6Mcast_hashKey->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    osal_memset(pIp6Mcast_addr, 0, sizeof(rtk_l2_ip6McastAddr_t));

    pIp6Mcast_addr->dip = pIp6Mcast_hashKey->dip;
    if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
        pIp6Mcast_addr->rvid = pIp6Mcast_hashKey->rvid;
    else if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
        pIp6Mcast_addr->sip = pIp6Mcast_hashKey->sip;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_ip6McastAddr_add
 * Description:
 *      Add IPv6 multicast entry to ASIC.
 * Input:
 *      unit            - unit id
 *      pIpmcast_addr   - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_IPV6_ADDRESS     - Invalid IPv6 address
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
int32
dal_cypress_l2_ip6McastAddr_add(uint32 unit, rtk_l2_ip6McastAddr_t *pIpmcast_addr)
{
    int32 ret;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    pIpmcast_addr->fwdIndex = -1;

    ret = dal_cypress_l2_ip6McastAddr_add_with_index(unit, pIpmcast_addr);

    return ret;
} /* end of dal_cypress_l2_ip6McastAddr_add */

/* Function Name:
 *      dal_cypress_l2_ip6McastAddr_get
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
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
int32
dal_cypress_l2_ip6McastAddr_get(uint32 unit, rtk_l2_ip6McastAddr_t *pIpmcast_addr)
{
    int32 ret;

    ret = dal_cypress_l2_ip6McastAddr_get_with_index(unit, pIpmcast_addr);

    return ret;
} /* end of dal_cypress_l2_ip6McastAddr_get */

/* Function Name:
 *      dal_cypress_l2_ip6McastAddr_set
 * Description:
 *      Update content of IP multicast entry.
 * Input:
 *      unit            - unit id
 *      pIpmcast_addr   - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_IPV6_ADDRESS - Invalid IPv4 address
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE   - invalid IP multicast lookup mode
 * Note:
 *      None
 */
int32
dal_cypress_l2_ip6McastAddr_set(uint32 unit, rtk_l2_ip6McastAddr_t *pIpmcast_addr)
{
    int32   ret;
    uint32  index;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_ipMcastAddr_set: unit=%d, sip=%x, dip=%x, vid=%d, pPortmask=%x",
           unit, pIpmcast_addr->sip, pIpmcast_addr->dip, pIpmcast_addr->rvid, pIpmcast_addr->portmask.bits[0]);

    RT_PARAM_CHK(NULL == pIpmcast_addr, RT_ERR_NULL_POINTER);
        RT_PARAM_CHK((pIpmcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));

    _dal_cypress_l2_ip6CareByteConvertToIP(&l2_entry.ipmcast_ip_mc_sip.dip, &l2_entry.ipmcast_ip_mc_sip.sip,
        &pIpmcast_addr->dip, &pIpmcast_addr->sip);

    if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc_sip.rvid = pIpmcast_addr->rvid;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc.rvid = pIpmcast_addr->rvid;
    }
    else
    {
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    l2_entry.entry_type = IP6_MULTICAST;

    L2_SEM_LOCK(unit);

    /* search exist or free entry */
    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);

    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }


    if (L2_IN_HASH == index_entry.index_type)
        index = ip6CacheIndexTbl[index_entry.index * HAL_L2_HASHDEPTH(unit) + index_entry.hashdepth];
    else
        index = ip6CacheIndexTbl[16384 + index_entry.index];
    if (osal_memcmp(&ip6CacheTbl[index].dip, &pIpmcast_addr->dip, sizeof(rtk_ipv6_addr_t)) != 0 ||
            osal_memcmp(&ip6CacheTbl[index].sip, &pIpmcast_addr->sip, sizeof(rtk_ipv6_addr_t)) != 0)
    {
        L2_SEM_UNLOCK(unit);
        return RT_ERR_L2_ENTRY_NOTFOUND;
    }

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

    if ((ret = table_field_set(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, pIpmcast_addr->portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
         RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
         return ret;
    }

    if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
        index = l2_entry.ipmcast_ip_mc_sip.index;
    else
        index = l2_entry.ipmcast_ip_mc.index;
    if ((ret = table_write(unit, CYPRESS_MC_PMSKt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* update multicast shadow */
        RTK_PORTMASK_ASSIGN(mcast_fwdTable_shadow[index].portmask, pIpmcast_addr->portmask);
    }

    /* Update fvid if needs */
    if (ipmcst_fvid_cmp == FALSE && LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit] && l2_entry.ipmcast_ip_mc_sip.rvid != pIpmcast_addr->rvid)
    {
        l2_entry.ipmcast_ip_mc_sip.rvid = pIpmcast_addr->rvid;
        if (L2_IN_HASH == index_entry.index_type)
        {
            /* if found entry is in HASH, programming in SRAM */
            ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
        }
        else if(L2_IN_CAM == index_entry.index_type)
        {
            /* if found entry is in CAM, programming in CAM */
            ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
        }
    }
    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_cypress_l2_ip6McastAddr_set */

/* Function Name:
 *      dal_cypress_l2_ip6McastAddr_del
 * Description:
 *      Delete a L2 ipv6 multicast address entry from the specified device.
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
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_L2_EMPTY_ENTRY - the entry is empty(invalid)
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE   - invalid IP multicast lookup mode
 * Note:
 *      1. In vlan unaware mode (SVL), the vid will be ignore, suggest to
 *         input vid=0 in vlan unaware mode.
 *      2. In vlan aware mode (IVL), the vid will be care.
 */
int32
dal_cypress_l2_ip6McastAddr_del(uint32 unit, rtk_ipv6_addr_t sip, rtk_ipv6_addr_t dip, rtk_vlan_t vid)
{
    int32 ret;
    int32   index, mcFwdIndex;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, sip=%x, dip=%x, vid=%d",
           unit, sip, dip, vid);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK((vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* search exist or free entry */
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    _dal_cypress_l2_ip6CareByteConvertToIP(&l2_entry.ipmcast_ip_mc_sip.dip, &l2_entry.ipmcast_ip_mc_sip.sip, &dip, &sip);

    if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc_sip.rvid = vid;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc.rvid = vid;
    }
    else
    {
        return RT_ERR_L2_IPMCAST_LOOKUP_MODE;
    }

    l2_entry.entry_type   = IP6_MULTICAST;

    L2_SEM_LOCK(unit);

    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);

    if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "err ret = %d", ret);
        return ret;
    }

    if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
        mcFwdIndex = l2_entry.ipmcast_ip_mc_sip.index;
    else
        mcFwdIndex = l2_entry.ipmcast_ip_mc.index;

    if (L2_IN_HASH == index_entry.index_type)
        index = ip6CacheIndexTbl[index_entry.index * HAL_L2_HASHDEPTH(unit) + index_entry.hashdepth];
    else
        index = ip6CacheIndexTbl[16384 + index_entry.index];
    if (osal_memcmp(&ip6CacheTbl[index].dip, &dip, sizeof(rtk_ipv6_addr_t)) == 0 &&
            osal_memcmp(&ip6CacheTbl[index].sip, &sip, sizeof(rtk_ipv6_addr_t)) == 0)
    {
        ip6CacheTbl[index].valid = FALSE;
        osal_memset(&ip6CacheTbl[index].dip, 0, sizeof(rtk_ipv6_addr_t));
        osal_memset(&ip6CacheTbl[index].sip, 0, sizeof(rtk_ipv6_addr_t));
        if (index < curCacheTblIdx || curCacheTblIdx == -1)
            curCacheTblIdx = index;
    }
    else
    {
        L2_SEM_UNLOCK(unit);
        return RT_ERR_L2_ENTRY_NOTFOUND;
    }

    ret = _dal_cypress_l2_freeMcastIdx(unit, (int32)mcFwdIndex);
    if(ret != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return RT_ERR_L2_IPMULTI_PARAM;
    }

    if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc_sip.rvid = 0;
        l2_entry.ipmcast_ip_mc_sip.dip  = 0;
        l2_entry.ipmcast_ip_mc_sip.sip  = 0;
        l2_entry.ipmcast_ip_mc_sip.index = 0;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc.rvid = 0;
        l2_entry.ipmcast_ip_mc.dip  = 0;
        l2_entry.ipmcast_ip_mc.index = 0;
    }

    if (L2_IN_HASH == index_entry.index_type)
    {
        /* if found entry is in HASH, programming in SRAM */
        ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    }
    else if (L2_IN_CAM == index_entry.index_type)
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }

    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_cypress_l2_ip6McastAddr_del */

/* Function Name:
 *      dal_cypress_l2_ip6McastAddr_add_with_index
 * Description:
 *      Add a IPv6 multicast address entry and multicast index to the specified device.
 * Input:
 *      unit                     - unit id
 *      vid                      - vlan id
 *      pIpMcast_addr            - content of IP multicast address entry
 * Output:
 *      pIpMacast_addr->fwdIndex - index of multicast forwarding entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE   - invalid IP multicast lookup mode
 * Note:
 *      If fwdIndex is larger than or equal to 0, will use fwdIndex as multicast index
 *          and won't config portmask.
 *
 *      If fwdIndex is smaller than 0, will allocate a free index and return it.
 *          It will also config portmask.
 */
int32
dal_cypress_l2_ip6McastAddr_add_with_index(uint32 unit, rtk_l2_ip6McastAddr_t *pIpMcast_addr)
{
    int32 ret, ret_1;
    int32 fwdIndex, index, found = 0;
    dal_cypress_l2_entry_t  l2_entry, dynamic_l2_entry;
    dal_cypress_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pIpMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pIpMcast_addr->fwdIndex >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);
    RT_PARAM_CHK((pIpMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
    RT_PARAM_CHK(curCacheTblIdx == -1, RT_ERR_L2_IP6_CACHETBL_FULL);

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_ip6McastAddr_add_with_index: unit=%d, sip=%x, dip=%x, vid=%d, pPortmask=%x",
           unit, pIpMcast_addr->sip, pIpMcast_addr->dip, pIpMcast_addr->rvid, pIpMcast_addr->portmask.bits[0]);

    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));

    _dal_cypress_l2_ip6CareByteConvertToIP(&l2_entry.ipmcast_ip_mc_sip.dip, &l2_entry.ipmcast_ip_mc_sip.sip,
        &pIpMcast_addr->dip, &pIpMcast_addr->sip);

    if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
    {
        l2_entry.entry_type             = IP6_MULTICAST;
        l2_entry.ipmcast_ip_mc_sip.rvid = pIpMcast_addr->rvid;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
    {
        l2_entry.entry_type         = IP6_MULTICAST;
        l2_entry.ipmcast_ip_mc.rvid = pIpMcast_addr->rvid;
    }
    else
    {
        return RT_ERR_L2_IPMCAST_LOOKUP_MODE;
    }

    L2_SEM_LOCK(unit);

    /* search exist or free entry */
    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry);

    if (ret == RT_ERR_L2_NO_EMPTY_ENTRY && (pIpMcast_addr->add_op_flags & RTK_L2_ADD_OP_FLAG_REPLACE_DYNAMIC) != 0)
    {
        osal_memcpy(&dynamic_l2_entry, &l2_entry, sizeof(dal_cypress_l2_entry_t));
        dynamic_l2_entry.entry_type  = IP6_MULTICAST;

        ret_1 = _dal_cypress_l2_getFirstDynamicEntry(unit, &dynamic_l2_entry, &index_entry);
        if (ret_1)
        {
            L2_SEM_UNLOCK(unit);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return ret_1;
        }
    }
    else if (RT_ERR_OK != ret)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    if (TRUE == l2_entry.is_entry_exist)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "entry exist");
        if (L2_IN_HASH == index_entry.index_type)
            index = ip6CacheIndexTbl[index_entry.index * HAL_L2_HASHDEPTH(unit) + index_entry.hashdepth];
        else
            index = ip6CacheIndexTbl[16384 + index_entry.index];
        if (osal_memcmp(&ip6CacheTbl[index].dip, &pIpMcast_addr->dip, sizeof(rtk_ipv6_addr_t)) == 0 &&
            osal_memcmp(&ip6CacheTbl[index].sip, &pIpMcast_addr->sip, sizeof(rtk_ipv6_addr_t)) == 0)
        {
            L2_SEM_UNLOCK(unit);
            return RT_ERR_L2_ENTRY_EXIST;
        }
        else
        {
            L2_SEM_UNLOCK(unit);
            return RT_ERR_L2_IP6_HASHKEY_EXIST;
        }
    }

    if (index_entry.index_type == L2_IN_HASH)
        pIpMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pIpMcast_addr->l2_idx = (1 << 14) + index_entry.index;

    ip6CacheTbl[curCacheTblIdx].valid = TRUE;
    osal_memcpy(&ip6CacheTbl[curCacheTblIdx].dip, &pIpMcast_addr->dip, sizeof(rtk_ipv6_addr_t));
    osal_memcpy(&ip6CacheTbl[curCacheTblIdx].sip, &pIpMcast_addr->sip, sizeof(rtk_ipv6_addr_t));
    if (L2_IN_HASH == index_entry.index_type)
        ip6CacheIndexTbl[index_entry.index * HAL_L2_HASHDEPTH(unit) + index_entry.hashdepth] = curCacheTblIdx;
    else
        ip6CacheIndexTbl[16384 + index_entry.index] = curCacheTblIdx;

    for (index = curCacheTblIdx + 1; index < RTK_DEFAULT_L2_IP6_CACHE_TBL_SIZE; index++)
    {
        if (ip6CacheTbl[index].valid == 0)
        {
            curCacheTblIdx = index;
            found = TRUE;
            break;
        }
    }
    if (!found)
        curCacheTblIdx = -1;

    fwdIndex = pIpMcast_addr->fwdIndex;

    /* get a free multicast index or increase reference count of mcast index */
    if ((ret = _dal_cypress_l2_allocMcastIdx(unit, &(pIpMcast_addr->fwdIndex))) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

    if (fwdIndex < 0)
    { /* Will configure portmask when fwdIndex is allocate automatically */
        if ((ret = table_field_set(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &(pIpMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }

        if ((ret = table_write(unit, CYPRESS_MC_PMSKt, (uint32)(pIpMcast_addr->fwdIndex), (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }

        if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
        {
            /* update multicast shadow */
            RTK_PORTMASK_ASSIGN(mcast_fwdTable_shadow[pIpMcast_addr->fwdIndex].portmask, pIpMcast_addr->portmask);
        }
    }

    if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
        l2_entry.ipmcast_ip_mc_sip.index = (uint32)(pIpMcast_addr->fwdIndex);
    else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
        l2_entry.ipmcast_ip_mc.index = (uint32)(pIpMcast_addr->fwdIndex);

    if (L2_IN_HASH == index_entry.index_type)
    {
        /* if found entry is in HASH, programming in SRAM */
        ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    }
    else if(L2_IN_CAM == index_entry.index_type)
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }

    if (RT_ERR_OK != ret)
    {
        _dal_cypress_l2_freeMcastIdx(unit, (int32)l2_entry.l2mcast.index);
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    L2_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of dal_cypress_l2_ip6McastAddr_add_with_index */

/* Function Name:
 *      dal_cypress_l2_ip6McastAddr_get_with_index
 * Description:
 *      Get a IP multicast address entry and multicast index from the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pIpMcast_addr - pointer to content of IP multicast address entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE   - invalid IP multicast lookup mode
 * Note:
 *      None.
 */
int32
dal_cypress_l2_ip6McastAddr_get_with_index(uint32 unit, rtk_l2_ip6McastAddr_t *pIpMcast_addr)
{
    int32   ret;
    int32   index;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;
    multicast_index_entry_t mcast_entry;

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_ipMcastAddr_get: unit=%d, sip=%x, dip=%x, vid=%d",
           unit, pIpMcast_addr->sip, pIpMcast_addr->dip, pIpMcast_addr->rvid);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    /* parameter check */
    RT_PARAM_CHK(NULL == pIpMcast_addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pIpMcast_addr->rvid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);

    /* search exist or free entry */
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    _dal_cypress_l2_ip6CareByteConvertToIP(&l2_entry.ipmcast_ip_mc_sip.dip, &l2_entry.ipmcast_ip_mc_sip.sip,
        &pIpMcast_addr->dip, &pIpMcast_addr->sip);

    if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc_sip.rvid = pIpMcast_addr->rvid;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
    {
        l2_entry.ipmcast_ip_mc.rvid = pIpMcast_addr->rvid;
    }
    else
    {
        return RT_ERR_L2_IPMCAST_LOOKUP_MODE;
    }

    l2_entry.entry_type = IP6_MULTICAST;

    ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry);

    if (RT_ERR_OK != ret)
    {
        return ret;
    }

    if (L2_IN_HASH == index_entry.index_type)
        index = ip6CacheIndexTbl[index_entry.index * HAL_L2_HASHDEPTH(unit) + index_entry.hashdepth];
    else
        index = ip6CacheIndexTbl[16384 + index_entry.index];

    if (index_entry.index_type == L2_IN_HASH)
        pIpMcast_addr->l2_idx = (index_entry.index << 2) | index_entry.hashdepth;
    else
        pIpMcast_addr->l2_idx = (1 << 14) + index_entry.index;


    if (osal_memcmp(&ip6CacheTbl[index].dip, &pIpMcast_addr->dip, sizeof(rtk_ipv6_addr_t)) != 0 ||
            osal_memcmp(&ip6CacheTbl[index].sip, &pIpMcast_addr->sip, sizeof(rtk_ipv6_addr_t)) != 0)
    {
        return RT_ERR_L2_ENTRY_NOTFOUND;
    }

    /* User may don't know the rvid when ipmcst_fvid_cmp is FALSE. Therefore we should assign it again */
    if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
    {
        pIpMcast_addr->rvid = l2_entry.ipmcast_ip_mc_sip.rvid;
        pIpMcast_addr->fwdIndex = l2_entry.ipmcast_ip_mc_sip.index;
    }
    else
    {
        pIpMcast_addr->rvid = l2_entry.ipmcast_ip_mc.rvid;
        pIpMcast_addr->fwdIndex = l2_entry.ipmcast_ip_mc.index;
    }
    L2_SEM_LOCK(unit);
    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* get multicast forward from shadow */
        RTK_PORTMASK_ASSIGN(pIpMcast_addr->portmask, mcast_fwdTable_shadow[pIpMcast_addr->fwdIndex].portmask);
        L2_SEM_UNLOCK(unit);
    }
    else
    {
        if ((ret = table_read(unit, CYPRESS_MC_PMSKt, pIpMcast_addr->fwdIndex, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }

        L2_SEM_UNLOCK(unit);

        if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &(pIpMcast_addr->portmask.bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }
    }

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_ip6McastAddr_get: pPortmask=%x", pIpMcast_addr->portmask.bits[0]);

    return ret;
} /* end of dal_cypress_l2_ipMcastAddr_get_with_index */

/* Function Name:
 *      dal_cypress_l2_nextValidIp6McastAddr_get
 * Description:
 *      Get next valid L2 IPv6 multicast address entry from the specified device.
 * Input:
 *      unit      - unit id
 *      pScan_idx - currently scan index of l2 table to get next.
 * Output:
 *      pL2_data  - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE   - invalid IP multicast lookup mode
 * Note:
 *      1. The function will skip valid l2 unicast and multicast entry and
 *         reply next valid L2 ip multicast address is based on index order of l2 table.
 *      2. Please input -1 for get the first entry of l2 table.
 *      3. The *pScan_idx is the input and also is the output argument.
 */
int32
dal_cypress_l2_nextValidIp6McastAddr_get(uint32 unit, int32 *pScan_idx, rtk_l2_ip6McastAddr_t *pL2_data)
{
    int32   ret;
    uint32  index = 0;
    dal_cypress_l2_entry_t      l2_entry;
    multicast_index_entry_t     mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK((NULL == pScan_idx), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pL2_data), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d", unit, *pScan_idx);

    L2_SEM_LOCK(unit);

    if ((ret = _dal_cypress_l2_nextValidAddr_get(unit, pScan_idx, IP6_MULTICAST, FALSE, &l2_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        return ret;
    }

    /*_dal_cypress_l2_ip6CareByteConvertToIP6Addr(&pL2_data->dip, &pL2_data->sip, &l2_entry.ipmcast_ip_mc_sip.dip, &l2_entry.ipmcast_ip_mc_sip.sip);*/

    index = ip6CacheIndexTbl[*pScan_idx];
    pL2_data->dip = ip6CacheTbl[index].dip;
    pL2_data->sip = ip6CacheTbl[index].sip;

    if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
    {
        pL2_data->rvid = l2_entry.ipmcast_ip_mc_sip.rvid;
        index           = l2_entry.ipmcast_ip_mc_sip.index;
    }
    else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
    {
        pL2_data->rvid = l2_entry.ipmcast_ip_mc.rvid;
        index           = l2_entry.ipmcast_ip_mc.index;
        osal_memset(&pL2_data->sip, 0, sizeof(rtk_ipv6_addr_t));
    }
    else
    {
        L2_SEM_UNLOCK(unit);
        return RT_ERR_L2_IPMCAST_LOOKUP_MODE;
    }

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* get multicast forward from shadow */
        RTK_PORTMASK_ASSIGN(pL2_data->portmask, mcast_fwdTable_shadow[index].portmask);
    }
    else
    {
        osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
        if ((ret = table_read(unit, CYPRESS_MC_PMSKt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, pL2_data->portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, rvid=%d, dip=%x, sip=%x, portmask=0x%x",
           unit, *pScan_idx, pL2_data->rvid, pL2_data->dip, pL2_data->sip, pL2_data->portmask.bits[0]);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_nextValidIpMcastAddr_get */

/* Function Name:
 *      dal_cypress_l2_mcastFwdIndex_alloc
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
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX       - invalid index of multicast forwarding entry
 *      RT_ERR_L2_MCAST_FWD_ENTRY_EXIST - index of forwarding entry is used.
 *      RT_ERR_L2_INDEXTBL_FULL         - L2 index table is full
 * Note:
 *      If *pFwdIndex is larger than or equal to 0, will use *pFwdIndex as multicast index.
 *      If *pFwdIndex is smaller than 0, will allocate a free index and return it.
 */
int32
dal_cypress_l2_mcastFwdIndex_alloc(
    uint32          unit,
    int32           *pFwdIndex)
{

    RT_PARAM_CHK(NULL == pFwdIndex, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(*pFwdIndex >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", unit, *pFwdIndex);

    if (*pFwdIndex >= 0)
    {
        if (RT_ERR_OK == _dal_cypress_l2_isMcastIdxUsed(unit, *pFwdIndex))
        {
            return RT_ERR_L2_MCAST_FWD_ENTRY_EXIST;
        }
    }

    return _dal_cypress_l2_allocMcastIdx(unit, pFwdIndex);
} /* end of dal_cypress_l2_mcastFwdIndex_alloc */

/* Function Name:
 *      dal_cypress_l2_mcastFwdIndex_free
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
 *      RT_ERR_L2_MULTI_FWD_INDEX           - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_MCAST_FWD_ENTRY_NOT_EXIST - index of forwarding entry is not exist
 * Note:
 */
int32
dal_cypress_l2_mcastFwdIndex_free(
    uint32          unit,
    int32           index)
{
    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", unit, index);

    RT_PARAM_CHK((index >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index) || index < 0, RT_ERR_L2_MULTI_FWD_INDEX);

    if (RT_ERR_OK != _dal_cypress_l2_isMcastIdxUsed(unit, index))
    {
        return RT_ERR_L2_MCAST_FWD_ENTRY_NOT_EXIST;
    }

    return _dal_cypress_l2_freeMcastIdx(unit, index);
} /* end of dal_cypress_l2_mcastFwdIndex_free */

/* Function Name:
 *      dal_cypress_l2_mcastFwdIndexFreeCount_get
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
dal_cypress_l2_mcastFwdIndexFreeCount_get(uint32 unit, uint32 *pFreeCount)
{
    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pFreeCount, RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);
    *pFreeCount = mcast_idx_pool[unit].free_entry_count;
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_mcastFwdIndexFreeCount_get */

/* Function Name:
 *      dal_cypress_l2_mcastFwdPortmask_set
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
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 *      The crossVlan will be ignored.
 */
int32
dal_cypress_l2_mcastFwdPortmask_set(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          crossVlan)
{
    int32 ret;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pPortmask, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(index >= mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_mcastFwdPortmask_set: unit=%d, index=%d, pPortmask=%x",
           unit, index, pPortmask->bits[0]);

    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

    if ((ret = table_field_set(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &(pPortmask->bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
         RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
         return ret;
    }

    L2_SEM_LOCK(unit);
    if ((ret = table_write(unit, CYPRESS_MC_PMSKt, (uint32)index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* update multicast shadow */
        RTK_PORTMASK_ASSIGN(mcast_fwdTable_shadow[index].portmask, *pPortmask);
    }

    L2_SEM_UNLOCK(unit);

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_mcastFwdPortmask_set: pPortmask=%x",
           pPortmask->bits[0]);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_mcastFwdPortmask_set */

/* Function Name:
 *      dal_cypress_l2_mcastFwdPortmask_get
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
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 *      The pCrossVlan will be ignored.
 */
int32
dal_cypress_l2_mcastFwdPortmask_get(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          *pCrossVlan)
{
    int32 ret;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    RT_PARAM_CHK(NULL == pPortmask, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(index >= mcast_idx_pool[unit].size_of_mcast_fwd_index, RT_ERR_L2_MULTI_FWD_INDEX);

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_mcastFwdPortmask_get: unit=%d, pPortmask=%x", unit, index);

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        L2_SEM_LOCK(unit);
        /* get multicast forward from shadow */
        RTK_PORTMASK_ASSIGN(*pPortmask, mcast_fwdTable_shadow[index].portmask);
        L2_SEM_UNLOCK(unit);
    }
    else
    {
        osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

        L2_SEM_LOCK(unit);
        if ((ret = table_read(unit, CYPRESS_MC_PMSKt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }
        L2_SEM_UNLOCK(unit);

        if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &(pPortmask->bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }
    }

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "dal_cypress_l2_mcastFwdPortmask_get: pPortmask=%x", pPortmask->bits[0]);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_mcastFwdPortmask_get */



/* Function Name:
 *      dal_cypress_l2_cpuMacAddr_add
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
int32
dal_cypress_l2_cpuMacAddr_add(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;


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
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid = vid;
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);

    if ((ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_OR_FREE, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(ret, (MOD_DAL|MOD_L2), "");
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
    l2_entry.unicast.port       = HAL_GET_CPU_PORT(unit);
    l2_entry.unicast.aging      = 7;
    l2_entry.unicast.sablock    = 0;
    l2_entry.unicast.dablock    = 0;
    l2_entry.unicast.is_static  = 1;
    l2_entry.unicast.suspending = 0;
    l2_entry.unicast.nh         = 0;

    if (L2_IN_HASH == index_entry.index_type)
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    }
    else if (L2_IN_CAM == index_entry.index_type)
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }

    L2_SEM_UNLOCK(unit);

    return ret;

} /* end of dal_cypress_l2_cpuMacAddr_add */


/* Function Name:
 *      dal_cypress_l2_cpuMacAddr_del
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
int32
dal_cypress_l2_cpuMacAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    int32 ret;
    dal_cypress_l2_entry_t  l2_entry;
    dal_cypress_l2_index_t  index_entry;

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
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type = L2_UNICAST;
    l2_entry.unicast.fid = vid;
    osal_memcpy(&l2_entry.unicast.mac, pMac, sizeof(rtk_mac_t));

    L2_SEM_LOCK(unit);

    if ((ret = _dal_cypress_l2_getExistOrFreeL2Entry(unit, &l2_entry, L2_GET_EXIST_ONLY, &index_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "ret=%d", ret);
        return ret;
    }

    /* fill content */
    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    l2_entry.entry_type = L2_UNICAST;

    if (L2_IN_HASH == index_entry.index_type)
    {
        /* if found entry is in HASH, programming in CAM */
        ret = _dal_cypress_l2_setL2HASHEntry(unit, &l2_entry, &index_entry);
    }
    else if (L2_IN_CAM == index_entry.index_type)
    {
        /* if found entry is in CAM, programming in CAM */
        ret = _dal_cypress_l2_setL2CAMEntry(unit, &l2_entry, &index_entry);
    }
    else
    {
        ret = RT_ERR_FAILED;
    }

    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_cypress_l2_cpuMacAddr_del */

/* Function Name:
 *      dal_cypress_l2_legalPortMoveAction_get
 * Description:
 *      Get forwarding action when port moving happen on specified port which is learnt as dynamic entry.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pFwdAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_cypress_l2_legalPortMoveAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pFwdAction)
{
    int32   ret;
    uint32  act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pFwdAction), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_MV_ACTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /* translate register value to action */
    switch (act)
    {
        case ACTION_FORWARD:
            *pFwdAction = 0;
            break;
        case ACTION_DROP:
            *pFwdAction = 1;
            break;
        case ACTION_TRAP2CPU:
            *pFwdAction = 2;
            break;
        case ACTION_COPY2CPU:
            *pFwdAction = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pFwdAction=%d", *pFwdAction);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_legalPortMoveAction_set
 * Description:
 *      Set forwarding action when legal port moving happen on specified port which is learnt as dynamic entry.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      fwdAction - forwarding action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_legalPortMoveAction_set(uint32 unit, rtk_port_t port, rtk_action_t fwdAction)
{
    int32   ret;
    uint32  act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, fwdAction=%d", unit, port, fwdAction);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(fwdAction >= ACTION_END, RT_ERR_FWD_ACTION);

    switch (fwdAction)
    {
        case ACTION_FORWARD:
            act = 0;
            break;
        case ACTION_DROP:
            act = 1;
            break;
        case ACTION_TRAP2CPU:
            act = 2;
            break;
        case ACTION_COPY2CPU:
            act = 3;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    /* programming value to CHIP*/
    if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_MV_ACTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_legalPortMoveflushAddrEnable_get
 * Description:
 *      Get the configuration of HW flush moved-port's mac of the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pEnable     - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_legalPortMoveflushAddrEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_MV_INVALIDATEr, port, REG_ARRAY_INDEX_NONE, CYPRESS_P_MV_INVLDf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /* translate register value */
    if (1 == value)
    {
        *pEnable = ENABLED;
    }
    else
    {
        *pEnable = DISABLED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_legalPortMoveflushAddrEnable_set
 * Description:
 *      Set the configuration of HW flush moved-port's mac of the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      enable      - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_legalPortMoveflushAddrEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, enable=%d", unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((ENABLED != enable && DISABLED != enable), RT_ERR_INPUT);

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
    if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_MV_INVALIDATEr, port, REG_ARRAY_INDEX_NONE, CYPRESS_P_MV_INVLDf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_getL2EntryfromHash
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
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      This is for extern module to call
 */
int32
dal_cypress_l2_getL2EntryfromHash(uint32 unit, uint32 hashkey, uint32 location, dal_cypress_l2_entry_t *pL2_entry, uint32 *pIsValid)
{
    L2_SEM_LOCK(unit);
    _dal_cypress_l2_getL2EntryfromHash(unit, hashkey, location, pL2_entry, pIsValid);
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_staticPortMoveAction_get
 * Description:
 *      Get forwarding action when port moving happen on specified port which is learnt as static entry.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pFwdAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
dal_cypress_l2_staticPortMoveAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pFwdAction)
{
    int32   ret;
    uint32  act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pFwdAction), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_STTC_MV_ACTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /* translate register value to action */
    switch (act)
    {
        case ACTION_FORWARD:
            *pFwdAction = 0;
            break;
        case ACTION_DROP:
            *pFwdAction = 1;
            break;
        case ACTION_TRAP2CPU:
            *pFwdAction = 2;
            break;
        case ACTION_COPY2CPU:
            *pFwdAction = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pFwdAction=%d", *pFwdAction);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_staticPortMoveAction_set
 * Description:
 *      Set forwarding action when legal port moving happen on specified port which is learnt as static entry.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      fwdAction - forwarding action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_FWD_ACTION   - invalid forwarding action
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_staticPortMoveAction_set(uint32 unit, rtk_l2_lookupMissType_t port, rtk_action_t fwdAction)
{
    int32   ret;
    uint32  act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, fwdAction=%d", unit, port, fwdAction);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(fwdAction >= ACTION_END, RT_ERR_FWD_ACTION);

    switch (fwdAction)
    {
        case ACTION_FORWARD:
            act = 0;
            break;
        case ACTION_DROP:
            act = 1;
            break;
        case ACTION_TRAP2CPU:
            act = 2;
            break;
        case ACTION_COPY2CPU:
            act = 3;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    /* programming value to CHIP */
    if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_STTC_MV_ACTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_lookupMissFloodPortMask_get
 * Description:
 *      Get flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      unit            - unit id
 *      type            - type of lookup miss
 * Output:
 *      pFlood_portmask - flooding port mask configuration when unicast/multicast lookup missed.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_L2_PMSK_NOT_INIT - flooding portmask is not initialized
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      In 8390, flood-portmask get/set only supports DLF_TYPE_UCAST and DLF_TYPE_BCAST. For DLF_TYPE_MCAST,
 *      DLF_TYPE_IPMC and DLF_TYPE_IP6MC, please refer to rtk_vlan_profile_set()/rtk_vlan_profile_get().
 */
int32
dal_cypress_l2_lookupMissFloodPortMask_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask)
{
    int32   ret;
    uint32  reg_idx, field;
    uint32  index;
    multicast_index_entry_t mcast_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pFlood_portmask, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(type >= DLF_TYPE_END, RT_ERR_INPUT);

    switch (type)
    {
        case DLF_TYPE_UCAST:
            if (!ucst_dlf_pmsk_inited)
                return RT_ERR_L2_PMSK_NOT_INIT;
            reg_idx = CYPRESS_L2_FLD_PMSKr;
            field = CYPRESS_L2_UNKN_UC_FLD_PMSKf;
            break;
        case DLF_TYPE_BCAST:
            if (!bcst_dlf_pmsk_inited)
                return RT_ERR_L2_PMSK_NOT_INIT;
            reg_idx = CYPRESS_L2_FLD_PMSKr;
            field = CYPRESS_L2_BC_FLD_PMSKf;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    if ((ret = reg_field_read(unit, reg_idx, field, &index)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* get multicast forward from shadow */
        RTK_PORTMASK_ASSIGN(*pFlood_portmask, mcast_fwdTable_shadow[index].portmask);
    }
    else
    {
        if ((ret = table_read(unit, CYPRESS_MC_PMSKt, index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &(pFlood_portmask->bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "flood_portmask=0x%x",
           pFlood_portmask->bits[0]);

    return ret;
}

/* Function Name:
 *      dal_cypress_l2_lookupMissFloodPortMask_set_with_idx
 * Description:
 *      Set flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      unit            - unit id
 *      type            - type of lookup miss
 *      idx             - index to multicast portmask table
 *      pFlood_portmask - flooding port mask configuration when unicast/multicast lookup missed.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_INPUT                    - invalid input parameter
 *      RT_ERR_L2_MCAST_FWD_ENTRY_EXIST - invalid type of lookup miss
 * Note:
 *      In 8390, flood-portmask get/set only supports DLF_TYPE_UCAST and DLF_TYPE_BCAST. For DLF_TYPE_MCAST,
 *      DLF_TYPE_IPMC and DLF_TYPE_IP6MC, please refer to rtk_vlan_profile_set()/rtk_vlan_profile_get().
 */
int32
dal_cypress_l2_lookupMissFloodPortMask_set_with_idx(uint32 unit, rtk_l2_lookupMissType_t type, uint32 idx, rtk_portmask_t *pFlood_portmask)
{
    int32   ret;
    uint32  reg_idx, field, inited, oldIdx;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pFlood_portmask, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((type >= DLF_TYPE_END), RT_ERR_INPUT);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pFlood_portmask=0x%x",
           unit, pFlood_portmask->bits[0]);

    switch (type)
    {
        case DLF_TYPE_UCAST:
            reg_idx = CYPRESS_L2_FLD_PMSKr;
            field = CYPRESS_L2_UNKN_UC_FLD_PMSKf;
            inited = ucst_dlf_pmsk_inited;
            break;
        case DLF_TYPE_BCAST:
            reg_idx = CYPRESS_L2_FLD_PMSKr;
            field = CYPRESS_L2_BC_FLD_PMSKf;
            inited = bcst_dlf_pmsk_inited;
            break;
        default:
            return RT_ERR_FAILED;
            break;
    }

    L2_SEM_LOCK(unit);

    if (inited)
    {
        reg_field_read(unit, reg_idx, field, &oldIdx);
        if (oldIdx != idx)
        {
            dal_cypress_l2_mcastFwdIndex_free(unit, oldIdx);

            ret = dal_cypress_l2_mcastFwdIndex_alloc(unit, (int32 *)&idx);
            if (ret == RT_ERR_L2_MCAST_FWD_ENTRY_EXIST)
            {
                _dal_cypress_l2_allocMcastIdx(unit, (int32 *)&idx);
            }
            else if (ret != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
                return ret;
            }
        }
        else
        {
            /* Need not to alloc */
        }
    }
    else
    {
        ret = dal_cypress_l2_mcastFwdIndex_alloc(unit, (int32 *)&idx);
        if (ret == RT_ERR_L2_MCAST_FWD_ENTRY_EXIST)
        {
            _dal_cypress_l2_allocMcastIdx(unit, (int32 *)&idx);
        }
        else if (ret != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }
    }

    if ((ret = reg_field_write(unit, reg_idx, field, &idx)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    if ((ret = table_field_set(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &(pFlood_portmask->bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    if ((ret = table_write(unit, CYPRESS_MC_PMSKt, (uint32)idx, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* update multicast shadow */
        RTK_PORTMASK_ASSIGN(mcast_fwdTable_shadow[idx].portmask, *pFlood_portmask);
    }

    L2_SEM_UNLOCK(unit);

    if (type == DLF_TYPE_UCAST)
        ucst_dlf_pmsk_inited = TRUE;
    else if (type == DLF_TYPE_BCAST)
        bcst_dlf_pmsk_inited = TRUE;

    return ret;
}

/* Function Name:
 *      dal_cypress_l2_lookupMissFloodPortMask_add
 * Description:
 *      Add one port member to the lookup missed flooding port mask.
 * Input:
 *      unit       - unit id
 *      flood_port - flooding port id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_L2_PMSK_NOT_INIT - flooding portmask is not initialized
 * Note:
 *      None
 */
int32
dal_cypress_l2_lookupMissFloodPortMask_add(uint32 unit, rtk_l2_lookupMissType_t type, rtk_port_t flood_port)
{
    int32 ret;
    rtk_portmask_t  portmask;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, flood_port=%d",
           unit, flood_port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, flood_port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((type >= DLF_TYPE_END), RT_ERR_INPUT);

    osal_memset(&portmask, 0, sizeof(rtk_portmask_t));

    ret = dal_cypress_l2_lookupMissFloodPortMask_get(unit, type, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "ret=%d",ret);
        return ret;
    }

    RTK_PORTMASK_PORT_SET(portmask, flood_port);

    ret = _dal_cypress_l2_lookupMissFloodPortMask_set(unit, type, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "ret=%d",ret);
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_lookupMissFloodPortMask_add */

/* Function Name:
 *      dal_cypress_l2_lookupMissFloodPortMask_del
 * Description:
 *      Delete one port member from the lookup missed flooding port mask.
 * Input:
 *      unit        - unit id
 *      type        - type of lookup miss
 *      flood_port  - flooding port id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_L2_PMSK_NOT_INIT - flooding portmask is not initialized
 * Note:
 *      None
 */
int32
dal_cypress_l2_lookupMissFloodPortMask_del(uint32 unit, rtk_l2_lookupMissType_t type, rtk_port_t flood_port)
{
    int32 ret;
    rtk_portmask_t  portmask;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, flood_port=%d",
           unit, flood_port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, flood_port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((type >= DLF_TYPE_END), RT_ERR_INPUT);

    osal_memset(&portmask, 0, sizeof(rtk_portmask_t));

    ret = dal_cypress_l2_lookupMissFloodPortMask_get(unit, type, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "ret=%d",ret);
        return ret;
    }

    RTK_PORTMASK_PORT_CLEAR(portmask, flood_port);

    ret = _dal_cypress_l2_lookupMissFloodPortMask_set(unit, type, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "ret=%d",ret);
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_lookupMissFloodPortMask_del */

/* Function Name:
 *      dal_cypress_l2_lookupMissFloodPortMaskIdx_get
 * Description:
 *      Get the entry index of forwarding table that is used as unicast/broadcast flooding port mask.
 * Input:
 *      unit    - unit id
 *      type    - type of lookup miss
 * Output:
 *      pIdx    - flooding port mask configuration when unicast/multicast lookup missed.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_L2_PMSK_NOT_INIT - flooding portmask is not initialized
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      In 8390, flood-portmask get/set only supports DLF_TYPE_UCAST and DLF_TYPE_BCAST. For DLF_TYPE_MCAST,
 *      DLF_TYPE_IPMC and DLF_TYPE_IP6MC, please refer to rtk_vlan_profile_set()/rtk_vlan_profile_get().
 */
int32
dal_cypress_l2_lookupMissFloodPortMaskIdx_get(uint32 unit, rtk_l2_lookupMissType_t type, uint32 *pIdx)
{
    int32   ret;
    uint32  reg_idx, field;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(type >= DLF_TYPE_END, RT_ERR_INPUT);
    RT_PARAM_CHK(NULL == pIdx, RT_ERR_NULL_POINTER);

    switch (type)
    {
        case DLF_TYPE_UCAST:
            if (!ucst_dlf_pmsk_inited)
                return RT_ERR_L2_PMSK_NOT_INIT;
            reg_idx = CYPRESS_L2_FLD_PMSKr;
            field = CYPRESS_L2_UNKN_UC_FLD_PMSKf;
            break;
        case DLF_TYPE_BCAST:
            if (!bcst_dlf_pmsk_inited)
                return RT_ERR_L2_PMSK_NOT_INIT;
            reg_idx = CYPRESS_L2_FLD_PMSKr;
            field = CYPRESS_L2_BC_FLD_PMSKf;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    if ((ret = reg_field_read(unit, reg_idx, field, pIdx)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pIdx=%d", *pIdx);

    return ret;
}

/* Function Name:
 *      dal_cypress_l2_lookupMissFloodPortMaskIdx_set
 * Description:
 *      Set the entry index of forwarding table that is used as unicast/broadcast flooding port mask.
 * Input:
 *      unit    - unit id
 *      type    - type of lookup miss
 *      idx     - flooding port mask configuration when unicast/multicast lookup missed.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_L2_PMSK_NOT_INIT - flooding portmask is not initialized
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      In 8390, flood-portmask get/set only supports DLF_TYPE_UCAST and DLF_TYPE_BCAST. For DLF_TYPE_MCAST,
 *      DLF_TYPE_IPMC and DLF_TYPE_IP6MC, please refer to rtk_vlan_profile_set()/rtk_vlan_profile_get().
 */
int32
dal_cypress_l2_lookupMissFloodPortMaskIdx_set(uint32 unit, rtk_l2_lookupMissType_t type, uint32 idx)
{
    int32   ret;
    uint32  reg_idx, field, inited, oldIdx;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= DLF_TYPE_END), RT_ERR_INPUT);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, rtk_l2_lookupMissType_t=%d, index=%d",
           unit, type, idx);

    switch (type)
    {
        case DLF_TYPE_UCAST:
            reg_idx = CYPRESS_L2_FLD_PMSKr;
            field = CYPRESS_L2_UNKN_UC_FLD_PMSKf;
            inited = ucst_dlf_pmsk_inited;
            break;
        case DLF_TYPE_BCAST:
            reg_idx = CYPRESS_L2_FLD_PMSKr;
            field = CYPRESS_L2_BC_FLD_PMSKf;
            inited = bcst_dlf_pmsk_inited;
            break;
        default:
            return RT_ERR_FAILED;
            break;
    }

    L2_SEM_LOCK(unit);

    if (inited)
    {
        reg_field_read(unit, reg_idx, field, &oldIdx);
        if (oldIdx != idx)
        {
            dal_cypress_l2_mcastFwdIndex_free(unit, oldIdx);

            ret = dal_cypress_l2_mcastFwdIndex_alloc(unit, (int32 *)&idx);
            if (ret == RT_ERR_L2_MCAST_FWD_ENTRY_EXIST)
            {
                _dal_cypress_l2_allocMcastIdx(unit, (int32 *)&idx);
            }
            else if (ret != RT_ERR_OK)
            {
                L2_SEM_UNLOCK(unit);
                RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
                return ret;
            }
        }
        else
        {
            /* Need not to alloc */
        }
    }
    else
    {
        ret = dal_cypress_l2_mcastFwdIndex_alloc(unit, (int32 *)&idx);
        if (ret == RT_ERR_L2_MCAST_FWD_ENTRY_EXIST)
        {
            _dal_cypress_l2_allocMcastIdx(unit, (int32 *)&idx);
        }
        else if (ret != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }
    }

    if ((ret = reg_field_write(unit, reg_idx, field, &idx)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    if (type == DLF_TYPE_UCAST)
        ucst_dlf_pmsk_inited = TRUE;
    else if (type == DLF_TYPE_BCAST)
        bcst_dlf_pmsk_inited = TRUE;

    return ret;
}

/* Function Name:
 *      dal_cypress_l2_portLookupMissAction_get
 * Description:
 *      Get forwarding action of specified port when destination address lookup miss.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      type    - type of lookup miss
 * Output:
 *      pAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid type of lookup miss
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_portLookupMissAction_get(uint32 unit, rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t *pAction)
{
    int32   ret;
    uint32  field;
    uint32  act = 0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d  port=%d  type=%d", unit, port, type);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= DLF_TYPE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    switch (type)
    {
        case DLF_TYPE_UCAST:
            field = CYPRESS_L2_UC_LM_ACTf;
            break;
        case DLF_TYPE_MCAST:
            field = CYPRESS_L2_MC_LM_ACTf;
            break;
        case DLF_TYPE_IPMC:
            field = CYPRESS_IP_MC_LM_ACTf;
            break;
        case DLF_TYPE_IP6MC:
            field = CYPRESS_IP6_MC_LM_ACTf;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_LM_ACTr, port, REG_ARRAY_INDEX_NONE, field, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    switch (act)
    {
        case 0:
            *pAction = ACTION_FORWARD;
            break;
        case 1:
            *pAction = ACTION_DROP;
            break;
        case 2:
            *pAction = ACTION_TRAP2CPU;
            break;
        case 3:
            *pAction = ACTION_COPY2CPU;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAction=%ld", *pAction);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_portLookupMissAction_get */

/* Function Name:
 *      dal_cypress_l2_portLookupMissAction_set
 * Description:
 *      Set forwarding action of specified port when destination address lookup miss.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      type    - type of lookup miss
 *      action  - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid type of lookup miss
 * Note:
 *      None
 */
int32
dal_cypress_l2_portLookupMissAction_set(uint32 unit, rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t action)
{
    int32   ret;
    uint32  field;
    uint32  act = 0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, type=%d action=%d", unit, port, type, action);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= DLF_TYPE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((action >= ACTION_END), RT_ERR_INPUT);

    switch (type)
    {
        case DLF_TYPE_UCAST:
            field = CYPRESS_L2_UC_LM_ACTf;
            break;
        case DLF_TYPE_MCAST:
            field = CYPRESS_L2_MC_LM_ACTf;
            break;
        case DLF_TYPE_IPMC:
            field = CYPRESS_IP_MC_LM_ACTf;
            break;
        case DLF_TYPE_IP6MC:
            field = CYPRESS_IP6_MC_LM_ACTf;
            break;
        default:
            return RT_ERR_INPUT;
    }

    switch (action)
    {
        case ACTION_FORWARD:
            act = 0;
            break;
        case ACTION_DROP:
            act = 1;
            break;
        case ACTION_TRAP2CPU:
            act = 2;
            break;
        case ACTION_COPY2CPU:
            act = 3;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_LM_ACTr, port, REG_ARRAY_INDEX_NONE, field, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "ret=%d", ret);
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}/* end of dal_cypress_l2_portLookupMissAction_set */

/* Function Name:
 *      dal_cypress_l2_srcPortEgrFilterMask_get
 * Description:
 *      Get loopback filtering function on specified ports.
 * Input:
 *      unit             - unit id
 * Output:
 *      pFilter_portmask - ports which turn on loopback filtering function
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The loopback(egress port == ingress port) packet is dropped if the function is enabled.
 */
int32
dal_cypress_l2_srcPortEgrFilterMask_get(uint32 unit, rtk_portmask_t *pFilter_portmask)
{
    int32   ret = RT_ERR_FAILED;
    uint32      value;
    rtk_port_t  port, max_port;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pFilter_portmask, RT_ERR_NULL_POINTER);
    osal_memset(pFilter_portmask, 0, sizeof(rtk_portmask_t));

    max_port = HAL_GET_MAX_PORT(unit);
    L2_SEM_LOCK(unit);

    for (port = 0; port <= max_port; port++)
    {
        if ((ret = reg_array_field_read(unit, CYPRESS_L2_IGR_P_FLTRr, port, REG_ARRAY_INDEX_NONE, CYPRESS_EGR_FLTR_ENf, &value)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        pFilter_portmask->bits[port / MASK_BIT_LEN] |= value << (port % MASK_BIT_LEN);
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "filter_portmask=0x%x", pFilter_portmask->bits[0]);

    return ret;
} /* end of dal_cypress_l2_srcPortEgrFilterMask_get */

/* Function Name:
 *      dal_cypress_l2_srcPortEgrFilterMask_set
 * Description:
 *      Set loopback filtering function on specified ports.
 * Input:
 *      unit             - unit id
 *      pFilter_portmask - ports which turn on loopback filtering function
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The loopback(egress port == ingress port) packet is dropped if the function is enabled.
 */
int32
dal_cypress_l2_srcPortEgrFilterMask_set(uint32 unit, rtk_portmask_t *pFilter_portmask)
{
    int32       ret = RT_ERR_FAILED;
    uint32      value;
    rtk_port_t  port, max_port;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pFilter_portmask, RT_ERR_NULL_POINTER);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, filter_portmask=%d", unit, pFilter_portmask->bits[0]);

    max_port = HAL_GET_MAX_PORT(unit);
    L2_SEM_LOCK(unit);

    for (port = 0; port <= max_port; port++)
    {
        value = (pFilter_portmask->bits[port / MASK_BIT_LEN] >> (port % MASK_BIT_LEN)) & 0x1;
        if ((ret = reg_array_field_write(unit, CYPRESS_L2_IGR_P_FLTRr, port, REG_ARRAY_INDEX_NONE, CYPRESS_EGR_FLTR_ENf, &value)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    return ret;
} /* end of dal_cypress_l2_srcPortEgrFilterMask_set */

/* Function Name:
 *      dal_cypress_l2_srcPortEgrFilterMask_add
 * Description:
 *      Enable the loopback filtering function on specified port.
 * Input:
 *      unit        - unit id
 *      filter_port - ports which turn on loopback filtering function
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      The loopback(egress port == ingress port) packet is dropped if the function is enabled.
 */
int32
dal_cypress_l2_srcPortEgrFilterMask_add(uint32 unit, rtk_port_t filter_port)
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

    ret = dal_cypress_l2_srcPortEgrFilterMask_get(unit, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    RTK_PORTMASK_PORT_SET(portmask, filter_port);

    ret = dal_cypress_l2_srcPortEgrFilterMask_set(unit, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_srcPortEgrFilterMask_add */

/* Function Name:
 *      dal_cypress_l2_srcPortEgrFilterMask_del
 * Description:
 *      Disable the loopback filtering function on specified port.
 * Input:
 *      unit        - unit id
 *      filter_port - ports which turn off loopback filtering function
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      The loopback(egress port == ingress port) packet is dropped if the function is enabled.
 */
int32
dal_cypress_l2_srcPortEgrFilterMask_del(uint32 unit, rtk_port_t filter_port)
{
    int32 ret = RT_ERR_FAILED;
    rtk_portmask_t  portmask;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, filter_port=%d",
           unit, filter_port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, filter_port), RT_ERR_PORT_ID);

    osal_memset(&portmask, 0, sizeof(rtk_portmask_t));

    ret = dal_cypress_l2_srcPortEgrFilterMask_get(unit, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    RTK_PORTMASK_PORT_CLEAR(portmask, filter_port);

    ret = dal_cypress_l2_srcPortEgrFilterMask_set(unit, &portmask);
    if (RT_ERR_OK != ret)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_srcPortEgrFilterMask_del */

/* Function Name:
 *      dal_cypress_l2_exceptionAddrAction_get
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
 *      RT_ERR_L2_EXCEPT_ADDR_TYPE  - invalid exception address type
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      SA_IS_ZERO is not supported.
 *      Exception address type is as following
 *      - RTL8390 doesn't provide separated actions for SA_IS_MCAST and SA_IS_BCAST. They use the same action.
 *
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
int32
dal_cypress_l2_exceptionAddrAction_get(uint32 unit, rtk_l2_exceptionAddrType_t exceptType, rtk_action_t *pAction)
{
    int32   ret;
    uint32  val_t, val_d;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((exceptType != SA_IS_BCAST_OR_MCAST) && (exceptType != SA_IS_ZERO), RT_ERR_L2_EXCEPT_ADDR_TYPE);
    RT_PARAM_CHK(NULL == pAction, RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);
    if (SA_IS_BCAST_OR_MCAST == exceptType)
    {
        if ((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_MC_BC_SA_TRAPf, &val_t)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        if ((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_MC_BC_SA_DROPf, &val_d)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_ALL_ZERO_SA_TRAPf, &val_t)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        if ((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_ALL_ZERO_SA_DROPf, &val_d)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    if ((val_t == 0) && (val_d == 0))
        *pAction = ACTION_FORWARD;
    else if ((val_t == 1) && (val_d == 0))
        *pAction = ACTION_TRAP2CPU;
    else if ((val_t == 0) && (val_d == 1))
        *pAction = ACTION_DROP;
    else
    {
        L2_SEM_UNLOCK(unit);
        return RT_ERR_FAILED;
    }

    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAction=%d", *pAction);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_exceptionAddrAction_get */

/* Function Name:
 *      dal_cypress_l2_exceptionAddrAction_set
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
 *      RT_ERR_L2_EXCEPT_ADDR_TYPE  - invalid exception address type
 *      RT_ERR_INPUT                - invalid input parameter
 * Note:
 *      Exception address type is as following
 *      - RTL8390 doesn't provide separated actions for SA_IS_MCAST and SA_IS_BCAST. They use the same action.
 *
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
dal_cypress_l2_exceptionAddrAction_set(uint32 unit, rtk_l2_exceptionAddrType_t exceptType, rtk_action_t action)
{
    int32   ret;
    uint32  val_t, val_d;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, action=%d", unit, action);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((exceptType != SA_IS_BCAST_OR_MCAST) && (exceptType != SA_IS_ZERO), RT_ERR_L2_EXCEPT_ADDR_TYPE);
    RT_PARAM_CHK(((action != ACTION_FORWARD) && (action != ACTION_DROP) && (action != ACTION_TRAP2CPU)), RT_ERR_INPUT);

    /* translate definition to chip's value  */
    if (ACTION_TRAP2CPU == action)
    {
        val_t = 1;
        val_d = 0;
    }
    else if (ACTION_DROP == action)
    {
        val_t = 0;
        val_d = 1;
    }
    else
    {
        val_t = 0;
        val_d = 0;
    }

    L2_SEM_LOCK(unit);

    /* programming value to CHIP */
    if (SA_IS_BCAST_OR_MCAST == exceptType)
    {
        if ((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_MC_BC_SA_TRAPf, &val_t)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        if ((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_MC_BC_SA_DROPf, &val_d)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_ALL_ZERO_SA_TRAPf, &val_t)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        if ((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_ALL_ZERO_SA_DROPf, &val_d)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_exceptionAddrAction_set */

/* Function Name:
 *      dal_cypress_l2_notificationEnable_get
 * Description:
 *      Get L2 notification configuration of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_notificationEnable_get(uint32 unit, rtk_enable_t *pEnable)
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
    if ((ret = reg_field_read(unit, CYPRESS_L2_NOTIFICATION_CTRLr, CYPRESS_NOTIFICATION_ENf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
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
}

/* Function Name:
 *      dal_cypress_l2_notificationEnable_set
 * Description:
 *      Set L2 notification configuration of the specified device.
 * Input:
 *      unit    - unit id
 *      enable  - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 * Note:
 *      None
 */
int32
dal_cypress_l2_notificationEnable_set(uint32 unit, rtk_enable_t enable)
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

    /* programming value to CHIP */
    if ((ret = reg_field_write(unit, CYPRESS_L2_NOTIFICATION_CTRLr, CYPRESS_NOTIFICATION_ENf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_notificationEventEnable_get
 * Description:
 *      Get L2 notification event enable configuration of the specified device.
 * Input:
 *      unit    - unit id
 *      event   - notification type
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      For 8390, only suspend learn and linkdown flush types support enable/disable state configuration
 */
int32
dal_cypress_l2_notificationEventEnable_get(uint32 unit, rtk_l2_notifyEvent_t event, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  enable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((L2_NOTIFY_EVENT_LINKDOWNFLUSH != event && L2_NOTIFY_EVENT_SUSPEND != event), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if (event == L2_NOTIFY_EVENT_LINKDOWNFLUSH)
    {
        if ((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_FLUSH_NOTIFY_ENf, &enable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_field_read(unit, CYPRESS_L2_NOTIFICATION_CTRLr, CYPRESS_SUSPEND_NOTIFICATION_ENf, &enable)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
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
}

/* Function Name:
 *      dal_cypress_l2_notificationEventEnable_set
 * Description:
 *      Set L2 notification event configuration of the specified device.
 * Input:
 *      unit    - unit id
 *      event   - notification type
 *      enable  - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NOT_INIT         - The module is not initial
 * Note:
 *      For 8390, only suspend learn and linkdown flush types support enable/disable state configuration
 */
int32
dal_cypress_l2_notificationEventEnable_set(uint32 unit, rtk_l2_notifyEvent_t event, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((L2_NOTIFY_EVENT_LINKDOWNFLUSH != event && L2_NOTIFY_EVENT_SUSPEND != event), RT_ERR_INPUT);
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

    /* programming value to CHIP */
    if (event == L2_NOTIFY_EVENT_LINKDOWNFLUSH)
    {
        if ((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_FLUSH_NOTIFY_ENf, &value)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_field_write(unit, CYPRESS_L2_NOTIFICATION_CTRLr, CYPRESS_SUSPEND_NOTIFICATION_ENf, &value)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_notificationFifoEmptyStatus_get
 * Description:
 *      Get FIFO empty status of L2 notification module of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pStatus - notify FIFO status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_notificationFifoEmptyStatus_get(uint32 unit, rtk_l2_notifyFifoStatus_t *pStatus)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, CYPRESS_L2_NOTIFICATION_CTRLr, CYPRESS_FIFO_EMPTYf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    /* translate chip's value to definition */
    if (0 == value)
    {
        *pStatus = L2_NOTIFY_FIFO_EMPTY;
    }
    else
    {
        *pStatus = L2_NOTIFY_FIFO_NOT_EMPTY;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pStatus=%d", *pStatus);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_notificationBackPressureThresh_get
 * Description:
 *      Get L2 notification back pressure threshold of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_notificationBackPressureThresh_get(uint32 unit, uint32 *pThresh)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pThresh), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_field_read(unit, CYPRESS_L2_NOTIFICATION_CTRLr, CYPRESS_BP_THRf, pThresh)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_notificationBackPressureThresh_set
 * Description:
 *      Set L2 notification back pressure threshold of the specified device.
 * Input:
 *      unit    - unit id
 *      thresh - threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
int32
dal_cypress_l2_notificationBackPressureThresh_set(uint32 unit, uint32 thresh)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((thresh >= HAL_L2_NOTIFICATION_BP_THRESH_MAX(unit)), RT_ERR_OUT_OF_RANGE);

    L2_SEM_LOCK(unit);

    /* program value to CHIP */
    if ((ret = reg_field_write(unit, CYPRESS_L2_NOTIFICATION_CTRLr, CYPRESS_BP_THRf, &thresh)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_notificationEventHandler_register
 * Description:
 *      Register event handlers for L2 notification module.
 * Input:
 *      unit   - unit id
 *      notification_callback - callback function for notification event
 *      bufferRunOut_callback - callback function for buffer run out event
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 * Note:
 *      None
 */
int32
dal_cypress_l2_notificationEventHandler_register(
    uint32 unit,
    rtk_l2_notification_callback_t notification_callback,
    rtk_l2_notification_bufferRunOut_callback_t bufferRunOut_callback)
{
    _notification_callback = notification_callback;
    _bufferRunOut_callback = bufferRunOut_callback;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_notificationEventHandler_unregister
 * Description:
 *      Unregister event handlers for L2 notification module.
 * Input:
 *      unit   - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 * Note:
 *      None
 */
int32
dal_cypress_l2_notificationEventHandler_unregister(uint32 unit)
{
    _notification_callback = NULL;
    _bufferRunOut_callback = NULL;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_zeroSALearningEnable_get
 * Description:
 *      Get enable status of all-zero-SA learning.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_zeroSALearningEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  enable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_SA_ALL_ZERO_LRNf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
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
}

/* Function Name:
 *      dal_cypress_l2_zeroSALearningEnable_set
 * Description:
 *      Set enable status of all-zero-SA learning.
 * Input:
 *      unit   - unit id
 *      enable - enable status of CPU tag adding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_zeroSALearningEnable_set(uint32 unit, rtk_enable_t enable)
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

    /* programming value to CHIP */
    if ((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_SA_ALL_ZERO_LRNf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_secureMacMode_get
 * Description:
 *      Get enable status of secure source MAC address mode.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CAM entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_cypress_l2_secureMacMode_get(uint32 unit, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP */
    if((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_SECURE_SAf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /* translate chip's value to definition */
    if (1 == value)
    {
        *pEnable = ENABLED;
    }
    else
    {
        *pEnable = DISABLED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_secureMacMode_set
 * Description:
 *      Set enable status of secure source MAC address mode.
 * Input:
 *      unit   - unit id
 *      enable - enable status of SA secure Mac mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_cypress_l2_secureMacMode_set(uint32 unit, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, enable=%d", unit, enable);

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

    /* program value to CHIP */
    if((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_SECURE_SAf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_cypress_l2_portDynamicPortMoveForbidEnable_get
 * Description:
 *      Get the port moveforbiddance configuration of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      (1) Dynamic address entry port move in/out forbiddance enabled port is not allowed and the entry
 *          is not updated.
 *      (2) Regarding the forwrding action, refer to rtk_l2_dynamicPortMoveForbidAction_set.
 */
int32
dal_cypress_l2_portDynamicPortMoveForbidEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  enable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP */
    if((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_MV_FORBIDr, port, REG_ARRAY_INDEX_NONE, CYPRESS_FORBID_ENf, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
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
} /* end of dal_cypress_l2_portDynamicPortMoveForbidEnable_get */

/* Function Name:
 *      dal_cypress_l2_portDynamicPortMoveForbidEnable_set
 * Description:
 *      Set the port move forbiddance configuration of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      (1) Dynamic address entry port move in/out forbiddance enabled port is not allowed and the entry
 *          is not updated.
 *      (2) Regarding the forwrding action, refer to rtk_l2_dynamicPortMoveForbidAction_set.
 */
int32
dal_cypress_l2_portDynamicPortMoveForbidEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, enable=%d", unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
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

    /* program value to CHIP */
    if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_MV_FORBIDr, port, REG_ARRAY_INDEX_NONE, CYPRESS_FORBID_ENf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_portDynamicPortMoveForbidEnable_set */

/* Function Name:
 *      dal_cypress_l2_dynamicPortMoveForbidAction_get
 * Description:
 *      Get the forwarding action when the port moving is detected on port move forbiddance port.
 * Input:
 *      unit     - unit id
 * Output:
 *      *pAction - pointer buffer of forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The action is taken either a dynamic address entry moved out from a port move forbiddance port
 *      or to a port move forbiddance port.
 */
int32
dal_cypress_l2_dynamicPortMoveForbidAction_get(uint32 unit, rtk_action_t *pAction)
{
    int32   ret;
    uint32  act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d", unit);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);

    /* get value from CHIP */
    if((ret = reg_field_read(unit, CYPRESS_L2_CTRL_0r, CYPRESS_FORBID_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    /* translate register value to action */
    switch (act)
    {
        case ACTION_FORWARD:
            *pAction = 0;
            break;
        case ACTION_DROP:
            *pAction = 1;
            break;
        case ACTION_TRAP2CPU:
            *pAction = 2;
            break;
        case ACTION_COPY2CPU:
            *pAction = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pAction=%d", *pAction);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_dynamicPortMoveForbidAction_get */

/* Function Name:
 *      dal_cypress_l2_dynamicPortMoveForbidAction_set
 * Description:
 *      Set the forwarding action when the port moving is detected on port move forbiddance port.
 * Input:
 *      unit   - unit id
 *      action - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The action is taken either a dynamic address entry moved out from a port move forbiddance port
 *      or to a port move forbiddance port.
 */
int32
dal_cypress_l2_dynamicPortMoveForbidAction_set(uint32 unit, rtk_action_t action)
{
    int32   ret;
    uint32  act;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, action=%d", unit, action);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(action >= ACTION_END, RT_ERR_INPUT);

    switch (action)
    {
        case ACTION_FORWARD:
            act = 0;
            break;
        case ACTION_DROP:
            act = 1;
            break;
        case ACTION_TRAP2CPU:
            act = 2;
            break;
        case ACTION_COPY2CPU:
            act = 3;
            break;
        default:
            return RT_ERR_INPUT;
    }

    L2_SEM_LOCK(unit);

    /* programming value to CHIP*/
    if((ret = reg_field_write(unit, CYPRESS_L2_CTRL_0r, CYPRESS_FORBID_ACTf, &act)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_dynamicPortMoveForbidAction_set */

/* Function Name:
 *      dal_cypress_l2_portMacFilterEnable_get
 * Description:
 *      Get the mac filter configuration of the specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      filterMode  - filter DA or SA
 * Output:
 *      pEnable     - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
dal_cypress_l2_portMacFilterEnable_get(uint32 unit, rtk_port_t port, rtk_l2_macFilterMode_t filterMode, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  field;
    uint32  enable;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, blockMode=%d", unit, port, filterMode);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if (MAC_FILTER_MODE_SA == filterMode)
        field = CYPRESS_SA_BLK_ENf;
    else
        field = CYPRESS_DA_BLK_ENf;

    L2_SEM_LOCK(unit);

    /* get value from CHIP */
    if((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_LM_ACTr, port, REG_ARRAY_INDEX_NONE, field, &enable)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
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
}

/* Function Name:
 *      dal_cypress_l2_portMacFilterEnable_set
 * Description:
 *      Set the mac filter configuration of the specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      filterMode  - filter DA or SA
 *      enable      - drop procedence assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
dal_cypress_l2_portMacFilterEnable_set(uint32 unit, rtk_port_t port, rtk_l2_macFilterMode_t filterMode, rtk_enable_t enable)
{
    int32   ret;
    uint32  field;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, blockMode=%d, enable=%d", unit, port, filterMode, enable);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
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

    if (MAC_FILTER_MODE_SA == filterMode)
        field = CYPRESS_SA_BLK_ENf;
    else
        field = CYPRESS_DA_BLK_ENf;

    L2_SEM_LOCK(unit);

    /* program value to CHIP */
    if((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_LM_ACTr, port, REG_ARRAY_INDEX_NONE, field, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
}

/* Module: static function */
/* Function Name:
 *      _dal_cypress_l2_init_config
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
_dal_cypress_l2_init_config(uint32 unit)
{
    int32       ret;
    rtk_port_t  port, max_port;
    uint8       bTChip = FALSE;
#if defined(CONFIG_SDK_DRIVER_RTK_BACKWARD_COMPATIBLE_8328)
    uint32      value = 0;
#endif

    bTChip = HAL_IS_ESCHIP(unit);
    max_port = HAL_GET_MAX_PORT(unit);

    /* Mcast/Bcast SA action */
    if ((ret = dal_cypress_l2_exceptionAddrAction_set(unit, SA_IS_BCAST_OR_MCAST, RTK_DEFAULT_L2_NON_UCAST_SA_ACTION)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* Disable CPU port learning */
    if (bTChip)
    {
        /* CPU port limit learning */
        if ((ret = dal_cypress_l2_portLimitLearningCnt_set(unit, HAL_GET_CPU_PORT(0), 0)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = dal_cypress_l2_portLimitLearningCntAction_set(unit, HAL_GET_CPU_PORT(0), LIMIT_LEARN_CNT_ACTION_FORWARD)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }
    else
    {
        /* CPU port New SA action and learning */
        if ((ret = dal_cypress_l2_newMacOp_set(unit, HAL_GET_CPU_PORT(0), RTK_DEFAULT_L2_CPU_NEW_SA_LEARN_MODE, RTK_DEFAULT_L2_NEW_SA_ACTION)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
    }

    /* IPv4 DIP validation */
    if ((ret = dal_cypress_l2_ipMcastAddrChkEnable_set(unit, RTK_DEFAULT_L2_IP4_MCAST_DIP_VALIDATION_STATE)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* IP multicast VID comparison */
    if ((ret = dal_cypress_l2_ipMcstFidVidCompareEnable_set(unit, RTK_DEFAULT_L2_IP_MCAST_IVL_LOOKUP_STATE)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* IPv6 multicast SIP care byte */
    if ((ret = dal_cypress_l2_ip6CareByte_set(unit, L2_SIP_HASH_CARE_BYTE, RTK_DEFAULT_L2_IP6_MCAST_LOOKUP_SIP_CARE_BYTE)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* IPv6 multicast DIP care byte */
    if ((ret = dal_cypress_l2_ip6CareByte_set(unit, L2_DIP_HASH_CARE_BYTE, RTK_DEFAULT_L2_IP6_MCAST_LOOKUP_DIP_CARE_BYTE)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* Aging time */
    if ((ret = dal_cypress_l2_aging_set(unit, 300)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* Hash algorithm */
    if ((ret = dal_cypress_l2_hashAlgo_set(unit, 0)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* Link down flush state */
    if ((ret = dal_cypress_l2_flushLinkDownPortAddrEnable_set(unit, RTK_DEFAULT_L2_FLUSH_LINKDOWN_MAC)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* Per port SA/DA block state */
    if (!bTChip)
    {
        for (port = 0; port < max_port; port++)
        {
            if (!HAL_IS_PORT_EXIST(unit, port))
            {
                continue;
            }

            if (HAL_IS_CPU_PORT(unit, port))
            {
                if ((ret = dal_cypress_l2_portMacFilterEnable_set(unit, port, MAC_FILTER_MODE_SA, RTK_DEFAULT_L2_SA_BLOCK_CPU_PORT_STATE)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = dal_cypress_l2_portMacFilterEnable_set(unit, port, MAC_FILTER_MODE_DA, RTK_DEFAULT_L2_DA_BLOCK_CPU_PORT_STATE)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }
            else
            {
                if ((ret = dal_cypress_l2_portMacFilterEnable_set(unit, port, MAC_FILTER_MODE_SA, RTK_DEFAULT_L2_SA_BLOCK_PORT_STATE)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = dal_cypress_l2_portMacFilterEnable_set(unit, port, MAC_FILTER_MODE_DA, RTK_DEFAULT_L2_DA_BLOCK_PORT_STATE)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }
        }
    }

#if defined(CONFIG_SDK_DRIVER_RTK_BACKWARD_COMPATIBLE_8328)
    for (port = 0; port < max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        /* get value from CHIP*/
        if ((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_LRN_CONSTRTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_CONSTRT_NUMf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        pL2_info[unit]->limitLearningCnt[port] = value;
        if (value == 0x7FFF)
            pL2_info[unit]->limitLearningCnt_enable[port] = DISABLED;
        else
            pL2_info[unit]->limitLearningCnt_enable[port] = ENABLED;
    }
#endif

    return RT_ERR_OK;
} /* end of _dal_cypress_l2_init_config */

/* Function Name:
 *      _dal_cypress_l2_nextValidAddr_get
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
_dal_cypress_l2_nextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    uint32              type,
    uint32              include_static,
    dal_cypress_l2_entry_t  *pL2_data)
{
    int32   ret;
    dal_cypress_l2_entry_t  l2_entry;
    uint32  l2_idx;
    uint32  l2cam_idx;
    uint32  l2_tableSize;
    uint32  l2cam_tableSize;
    uint32  hashkey, location;
    uint32  isValid;
    uint32  found;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, pScan_idx=%d, type=%d, include_static=%d",
           unit, *pScan_idx, type, include_static);

    if ((ret = table_size_get(unit, CYPRESS_L2_CAM_UCt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    if ((ret = table_size_get(unit, CYPRESS_L2_UCt, &l2_tableSize)) != RT_ERR_OK)
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

    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));

    for (; l2_idx < l2_tableSize; l2_idx++)
    {
        hashkey = l2_idx >> 2;
        location = l2_idx & BITMASK_2B;

        if ((_dal_cypress_l2_getL2EntryfromHash(unit, hashkey, location, &l2_entry, &isValid) == RT_ERR_OK)
            && (TRUE == isValid) && (type == l2_entry.entry_type))
        {
            if (L2_UNICAST == l2_entry.entry_type)
            {
                if ((FALSE == include_static) && (1 == l2_entry.unicast.is_static))
                {
                    /* if not need to include static and this entry is static, just skip this entry */
                    continue;
                }
                if (HAL_IS_ESCHIP(unit) && 1 == l2_entry.unicast.nh)
                {
                    /* nh entry is not included in this API */
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
        for (l2cam_idx = l2_idx - l2_tableSize; l2CamEbl && (l2cam_idx < l2cam_tableSize); l2cam_idx++)
        {
            if ((_dal_cypress_l2_getL2EntryfromCAM(unit, l2cam_idx, &l2_entry, &isValid) == RT_ERR_OK)
                && (TRUE == isValid) && (type == l2_entry.entry_type))
            {
                if (L2_UNICAST == l2_entry.entry_type)
                {
                    if ((FALSE == include_static) && (1 == l2_entry.unicast.is_static))
                    {
                        /* if not need to include static and this entry is static, just skip this entry */
                        continue;
                    }
                    if (HAL_IS_ESCHIP(unit) && 1 == l2_entry.unicast.nh)
                    {
                        /* nh entry is not included in this API */
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
        return RT_ERR_FAILED;
    }

    osal_memcpy(pL2_data, &l2_entry, sizeof(dal_cypress_l2_entry_t));
    return RT_ERR_OK;
} /* end of _dal_cypress_l2_nextValidAddr_get */


/* Function Name:
 *      _dal_cypress_l2_setL2HASHEntry
 * Description:
 *      Set exist entry or free entry
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
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
static int32
_dal_cypress_l2_setL2HASHEntry(uint32 unit, dal_cypress_l2_entry_t *pL2_entry, dal_cypress_l2_index_t *pL2_index)
{
    int32                   ret;
    l2_entry_t              l2_entry;
    uint32                  mac_uint32[2];
    uint32                  l2_index;
    uint32                  ip_multi, ip6_multi;


    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, hashdepth=%d, index=%d",
           unit, pL2_entry->entry_type, pL2_index->hashdepth, pL2_index->index);

    osal_memset(&l2_entry, 0, sizeof(l2_entry));

    /* Extract content of each kind of entry from l2_enry */
    switch (pL2_entry->entry_type)
    {
        case L2_UNICAST: /* L2 unicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "da_block=%d, sa_block=%d, port=%d, is_static=%d, aging=%d, suspend=%d, nh=%d, Mac=%x-%x-%x-%x-%x-%x",
                   pL2_entry->unicast.dablock, pL2_entry->unicast.sablock, pL2_entry->unicast.port,
                   pL2_entry->unicast.is_static, pL2_entry->unicast.aging, pL2_entry->unicast.suspending, pL2_entry->unicast.nh, pL2_entry->unicast.mac.octet[0],
                   pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], pL2_entry->unicast.mac.octet[3],
                   pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);

            if (HAL_IS_ESCHIP(unit) && (pL2_entry->unicast.nh == TRUE))
            {
                if ((ret = table_field_set(unit, CYPRESS_L2_NH_LEGACYt, CYPRESS_L2_NH_LEGACY_HOSTMAC_IDXtf, &pL2_entry->unicast.route_idx, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_NH_LEGACYt, CYPRESS_L2_NH_LEGACY_VID_SELtf, &pL2_entry->unicast.vlan_target, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_NH_LEGACYt, CYPRESS_L2_NH_LEGACY_DVIDtf, &pL2_entry->unicast.fid, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                    return ret;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_NH_LEGACYt, CYPRESS_L2_NH_LEGACY_DPAtf, &pL2_entry->unicast.port, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_NH_LEGACYt, CYPRESS_L2_NH_LEGACY_NEXT_HOPtf, &pL2_entry->unicast.nh, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                    return ret;
                }
            }
            else
            {
                ip_multi = 0;
                ip6_multi = 0;
                if ((ret = table_field_set(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_IP_MCtf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_IP6_MCtf, &ip6_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_DA_BLKtf, &pL2_entry->unicast.dablock, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_SA_BLKtf, &pL2_entry->unicast.sablock, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_SLPtf, &pL2_entry->unicast.port, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_STATICtf, &pL2_entry->unicast.is_static, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_AGEtf, &pL2_entry->unicast.aging, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_SUSPENDtf, &pL2_entry->unicast.suspending, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_NEXT_HOPtf, &pL2_entry->unicast.nh, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if (pL2_entry->unicast.nh == TRUE)
                {
                    if ((ret = table_field_set(unit, CYPRESS_L2_NEXT_HOPt, CYPRESS_L2_NEXT_HOP_VLAN_TARGETtf, &pL2_entry->unicast.vlan_target, (uint32 *)&l2_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                        return ret;
                    }
                    if ((ret = table_field_set(unit, CYPRESS_L2_NEXT_HOPt, CYPRESS_L2_NEXT_HOP_ROUTE_IDXtf, &pL2_entry->unicast.route_idx, (uint32 *)&l2_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                        return ret;
                    }
                }
                else
                {
                    if ((ret = table_field_set(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_VIDtf, &pL2_entry->unicast.agg_vid, (uint32 *)&l2_entry)) != RT_ERR_OK)
                    {
                        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                        return ret;
                    }
                }

                mac_uint32[1] = ((uint32)pL2_entry->unicast.mac.octet[0]) << 8;
                mac_uint32[1] |= ((uint32)pL2_entry->unicast.mac.octet[1]);
                mac_uint32[0] = ((uint32)pL2_entry->unicast.mac.octet[2]) << 24;
                mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[3]) << 16;
                mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[4]) << 8;
                mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[5]);

                if ((ret = table_field_set(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_MACtf, &mac_uint32[0], (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }

            break;
        case L2_MULTICAST: /* l2 Multicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, Mac=%x-%x-%x-%x-%x-%x",
                   pL2_entry->l2mcast.index, pL2_entry->l2mcast.mac.octet[0],
                   pL2_entry->l2mcast.mac.octet[1], pL2_entry->l2mcast.mac.octet[2], pL2_entry->l2mcast.mac.octet[3],
                   pL2_entry->l2mcast.mac.octet[4], pL2_entry->l2mcast.mac.octet[5]);

            ip_multi = 0;
            ip6_multi = 0;
            if ((ret = table_field_set(unit, CYPRESS_L2_MCt, CYPRESS_L2_MC_IP_MCtf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }
            if ((ret = table_field_set(unit, CYPRESS_L2_MCt, CYPRESS_L2_MC_IP6_MCtf, &ip6_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_MCt, CYPRESS_L2_MC_MC_PMSK_IDXtf, &pL2_entry->l2mcast.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            mac_uint32[1] = ((uint32)pL2_entry->l2mcast.mac.octet[0]) << 8;
            mac_uint32[1] |= ((uint32)pL2_entry->l2mcast.mac.octet[1]);
            mac_uint32[0] = ((uint32)pL2_entry->l2mcast.mac.octet[2]) << 24;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[3]) << 16;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[4]) << 8;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[5]);

            if ((ret = table_field_set(unit, CYPRESS_L2_MCt, CYPRESS_L2_MC_MACtf, &mac_uint32[0], (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            break;
        case IP4_MULTICAST: /* IP multicast */
            if (LOOKUP_ON_FVID_AND_MAC == ip4HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, Mac=%x-%x-%x-%x-%x-%x",
                        pL2_entry->ipmcast_mc_ip.index, pL2_entry->ipmcast_mc_ip.mac, pL2_entry->ipmcast_mc_ip.mac.octet[0],
                        pL2_entry->ipmcast_mc_ip.mac.octet[1], pL2_entry->ipmcast_mc_ip.mac.octet[2], pL2_entry->ipmcast_mc_ip.mac.octet[3],
                        pL2_entry->ipmcast_mc_ip.mac.octet[4], pL2_entry->ipmcast_mc_ip.mac.octet[5]);

                ip_multi = 0; /* ip_multi in LOOKUP_ON_FVID_AND_MAC hash mode is set to 0*/
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_IP_MCtf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_FAILED;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_mc_ip.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_FAILED;
                }

                mac_uint32[1] = ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[0]) << 8;
                mac_uint32[1] |= ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[1]);
                mac_uint32[0] = ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[2]) << 24;
                mac_uint32[0] |= ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[3]) << 16;
                mac_uint32[0] |= ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[4]) << 8;
                mac_uint32[0] |= ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[5]);

                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_IP_MCtf, &mac_uint32[0], (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, dip=%d, sip=%d",
                        pL2_entry->ipmcast_ip_mc_sip.index, pL2_entry->ipmcast_ip_mc_sip.rvid, pL2_entry->ipmcast_ip_mc_sip.dip, pL2_entry->ipmcast_ip_mc_sip.sip);

                if (pL2_entry->ipmcast_ip_mc_sip.dip == 0)
                {/* if dip is zero, mean delete entry. So set ip_multi = 0 */
                    ip_multi = 0;
                }
                else
                {
                    ip_multi = 1;
                }
                ip6_multi = 0;

                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_IP_MCtf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_IP6_MCtf, &ip6_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_GIPtf, &pL2_entry->ipmcast_ip_mc_sip.dip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_SIPtf, &pL2_entry->ipmcast_ip_mc_sip.sip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc_sip.rvid, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc_sip.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, dip=%d", pL2_entry->ipmcast_ip_mc.index, pL2_entry->ipmcast_ip_mc.dip);

                if (pL2_entry->ipmcast_ip_mc.dip == 0)
                {/* if dip is zero, mean delete entry. So set ip_multi = 0 */
                    ip_multi = 0;
                }
                else
                {
                    ip_multi = 1;
                }
                ip6_multi = 0;

                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_IP_MCtf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_IP6_MCtf, &ip6_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc.rvid, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_GIPtf, &pL2_entry->ipmcast_ip_mc.dip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }

            break;
        case IP6_MULTICAST: /* IP multicast */
            if (LOOKUP_ON_FVID_AND_MAC == ip6HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, Mac=%x-%x-%x-%x-%x-%x",
                        pL2_entry->ipmcast_mc_ip.index, pL2_entry->ipmcast_mc_ip.mac, pL2_entry->ipmcast_mc_ip.mac.octet[0],
                        pL2_entry->ipmcast_mc_ip.mac.octet[1], pL2_entry->ipmcast_mc_ip.mac.octet[2], pL2_entry->ipmcast_mc_ip.mac.octet[3],
                        pL2_entry->ipmcast_mc_ip.mac.octet[4], pL2_entry->ipmcast_mc_ip.mac.octet[5]);

                ip_multi = 0; /* ip_multi in LOOKUP_ON_FVID_AND_MAC hash mode is set to 0*/
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_IP_MCtf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_FAILED;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_mc_ip.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_FAILED;
                }

                mac_uint32[1] = ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[0]) << 8;
                mac_uint32[1] |= ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[1]);
                mac_uint32[0] = ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[2]) << 24;
                mac_uint32[0] |= ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[3]) << 16;
                mac_uint32[0] |= ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[4]) << 8;
                mac_uint32[0] |= ((uint32)pL2_entry->ipmcast_mc_ip.mac.octet[5]);

                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_IP_MCtf, &mac_uint32[0], (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, dip=%d, sip=%d",
                        pL2_entry->ipmcast_ip_mc_sip.index, pL2_entry->ipmcast_ip_mc_sip.rvid, pL2_entry->ipmcast_ip_mc_sip.dip, pL2_entry->ipmcast_ip_mc_sip.sip);

                if (pL2_entry->ipmcast_ip_mc_sip.dip == 0)
                {/* if dip is zero, mean delete entry. So set ip_multi = 0 */
                    ip_multi = 0;
                    ip6_multi = 0;
                }
                else
                {
                    ip_multi = 1;
                    ip6_multi = 1;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_IP_MCtf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_IP6_MCtf, &ip6_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_GIPtf, &pL2_entry->ipmcast_ip_mc_sip.dip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_SIPtf, &pL2_entry->ipmcast_ip_mc_sip.sip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc_sip.rvid, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc_sip.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, dip=%d", pL2_entry->ipmcast_ip_mc.index, pL2_entry->ipmcast_ip_mc.dip);

                if (pL2_entry->ipmcast_ip_mc.dip == 0)
                {/* if dip is zero, mean delete entry. So set ip_multi = 0 */
                    ip_multi = 0;
                    ip6_multi = 0;
                }
                else
                {
                    ip_multi = 1;
                    ip6_multi = 1;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_IP_MCtf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_IP6_MCtf, &ip6_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc.rvid, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_GIPtf, &pL2_entry->ipmcast_ip_mc.dip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }

            break;
        default:
            return RT_ERR_FAILED;
     }

    l2_index = (SRAM << RTL8390_TBL_ACCESS_L2_CTRL_TBL_OFFSET) | (pL2_index->index << 2) | pL2_index->hashdepth;

     /* write entry from chip */
    if ((ret = table_write(unit, CYPRESS_L2_UCt, l2_index, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of _dal_cypress_l2_setL2HASHEntry */


/* Function Name:
 *      _dal_cypress_l2_setL2CAMEntry
 * Description:
 *      Set exist entry or free entry
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
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
static int32
_dal_cypress_l2_setL2CAMEntry(uint32 unit, dal_cypress_l2_entry_t *pL2_entry, dal_cypress_l2_index_t *pL2_index)
{
    int32   ret;
    l2cam_entry_t l2cam_entry;
    uint32  mac_uint32[2];
    uint32  l2_index;
    uint32  ip_multi, ip6_multi;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, hashdepth=%d, index=%d",
           unit, pL2_entry->entry_type, pL2_index->hashdepth, pL2_index->index);

    l2_index = pL2_index->index ;

    osal_memset(&l2cam_entry, 0, sizeof(l2cam_entry));

    /* Extract content of each kind of entry from l2_enry */
    switch (pL2_entry->entry_type)
    {
        case L2_UNICAST: /* L2 unicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "da_block=%d, sa_block=%d, port=%d, fid=%d, is_static=%d, aging=%d, suspend=%d, nh=%d, \
                   Mac=%x-%x-%x-%x-%x-%x",
                   pL2_entry->unicast.dablock, pL2_entry->unicast.sablock, pL2_entry->unicast.port, pL2_entry->unicast.fid,
                   pL2_entry->unicast.is_static, pL2_entry->unicast.aging, pL2_entry->unicast.suspending, pL2_entry->unicast.nh, pL2_entry->unicast.mac.octet[0],
                   pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], pL2_entry->unicast.mac.octet[3],
                   pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);
            ip_multi = 0;
            ip6_multi = 0;
            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_IP_MCtf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }
            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_IP6_MCtf, &ip6_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_DA_BLKtf, &pL2_entry->unicast.dablock, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_SA_BLKtf, &pL2_entry->unicast.sablock, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_SLPtf, &pL2_entry->unicast.port, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_FID_RVIDtf, &pL2_entry->unicast.fid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_STATICtf, &pL2_entry->unicast.is_static, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_AGEtf, &pL2_entry->unicast.aging, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_SUSPENDtf, &pL2_entry->unicast.suspending, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_NEXT_HOPtf, &pL2_entry->unicast.nh, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_VIDtf, &pL2_entry->unicast.agg_vid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            mac_uint32[1] = ((uint32)pL2_entry->unicast.mac.octet[0]) << 8;
            mac_uint32[1] |= ((uint32)pL2_entry->unicast.mac.octet[1]);
            mac_uint32[0] = ((uint32)pL2_entry->unicast.mac.octet[2]) << 24;
            mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[3]) << 16;
            mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[4]) << 8;
            mac_uint32[0] |= ((uint32)pL2_entry->unicast.mac.octet[5]);

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_MACtf, &mac_uint32[0], (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            break;
        case L2_MULTICAST: /* l2 Multicast */
            RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, Mac=%x-%x-%x-%x-%x-%x",
                   pL2_entry->l2mcast.index, pL2_entry->l2mcast.rvid, pL2_entry->l2mcast.mac.octet[0],
                   pL2_entry->l2mcast.mac.octet[1], pL2_entry->l2mcast.mac.octet[2], pL2_entry->l2mcast.mac.octet[3],
                   pL2_entry->l2mcast.mac.octet[4], pL2_entry->l2mcast.mac.octet[5]);
            ip_multi = 0;
            ip6_multi = 0;
            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_MCt, CYPRESS_L2_CAM_MC_IP_MCtf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }
            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_MCt, CYPRESS_L2_CAM_MC_IP6_MCtf, &ip6_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_MCt, CYPRESS_L2_CAM_MC_MC_PMSK_IDXtf, &pL2_entry->l2mcast.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_MCt, CYPRESS_L2_CAM_MC_FID_RVIDtf, &pL2_entry->l2mcast.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            mac_uint32[1] = ((uint32)pL2_entry->l2mcast.mac.octet[0]) << 8;
            mac_uint32[1] |= ((uint32)pL2_entry->l2mcast.mac.octet[1]);
            mac_uint32[0] = ((uint32)pL2_entry->l2mcast.mac.octet[2]) << 24;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[3]) << 16;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[4]) << 8;
            mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[5]);

            if ((ret = table_field_set(unit, CYPRESS_L2_CAM_MCt, CYPRESS_L2_CAM_MC_MACtf, &mac_uint32[0], (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            break;
        case IP4_MULTICAST: /* IP multicast */
            if (LOOKUP_ON_FVID_AND_MAC == ip4HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, Mac=%x-%x-%x-%x-%x-%x",
                        pL2_entry->ipmcast_mc_ip.index, pL2_entry->ipmcast_mc_ip.rvid, pL2_entry->ipmcast_mc_ip.mac, pL2_entry->ipmcast_mc_ip.mac.octet[0],
                        pL2_entry->ipmcast_mc_ip.mac.octet[1], pL2_entry->ipmcast_mc_ip.mac.octet[2], pL2_entry->ipmcast_mc_ip.mac.octet[3],
                        pL2_entry->ipmcast_mc_ip.mac.octet[4], pL2_entry->ipmcast_mc_ip.mac.octet[5]);

                ip_multi = 0; /* ip_multi in LOOKUP_ON_FVID_AND_MAC hash mode is set to 0*/
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_IP_MCtf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_FAILED;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_mc_ip.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_FAILED;
                }

                mac_uint32[1] = ((uint32)pL2_entry->l2mcast.mac.octet[0]) << 8;
                mac_uint32[1] |= ((uint32)pL2_entry->l2mcast.mac.octet[1]);
                mac_uint32[0] = ((uint32)pL2_entry->l2mcast.mac.octet[2]) << 24;
                mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[3]) << 16;
                mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[4]) << 8;
                mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[5]);

                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_IP_MCtf, &mac_uint32[0], (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, dip=%d, sip=%d",
                        pL2_entry->ipmcast_ip_mc_sip.index, pL2_entry->ipmcast_ip_mc_sip.rvid, pL2_entry->ipmcast_ip_mc_sip.dip, pL2_entry->ipmcast_ip_mc_sip.sip);

                if (pL2_entry->ipmcast_ip_mc_sip.dip == 0)
                {/* if dip is zero, mean delete entry. So set ip_multi = 0 */
                    ip_multi = 0;
                }
                else
                {
                    ip_multi = 1;
                }
                ip6_multi = 0;

                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_IP_MCtf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_IP6_MCtf, &ip6_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_GIPtf, &pL2_entry->ipmcast_ip_mc_sip.dip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_SIPtf, &pL2_entry->ipmcast_ip_mc_sip.sip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc_sip.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc_sip.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, dip=%d", pL2_entry->ipmcast_ip_mc.index, pL2_entry->ipmcast_ip_mc.rvid, pL2_entry->ipmcast_ip_mc.dip);

                if (pL2_entry->ipmcast_ip_mc_sip.dip == 0)
                {/* if dip is zero, mean delete entry. So set ip_multi = 0 */
                    ip_multi = 0;
                }
                else
                {
                    ip_multi = 1;
                }
                ip6_multi = 0;

                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_IP_MCtf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_IP6_MCtf, &ip6_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_CAM_IP_MC_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_GIPtf, &pL2_entry->ipmcast_ip_mc.dip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }

            break;
        case IP6_MULTICAST: /* IPv6 multicast */
            if (LOOKUP_ON_FVID_AND_MAC == ip6HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, Mac=%x-%x-%x-%x-%x-%x",
                        pL2_entry->ipmcast_mc_ip.index, pL2_entry->ipmcast_mc_ip.rvid, pL2_entry->ipmcast_mc_ip.mac, pL2_entry->ipmcast_mc_ip.mac.octet[0],
                        pL2_entry->ipmcast_mc_ip.mac.octet[1], pL2_entry->ipmcast_mc_ip.mac.octet[2], pL2_entry->ipmcast_mc_ip.mac.octet[3],
                        pL2_entry->ipmcast_mc_ip.mac.octet[4], pL2_entry->ipmcast_mc_ip.mac.octet[5]);

                ip_multi = 0; /* ip_multi in LOOKUP_ON_FVID_AND_MAC hash mode is set to 0*/
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_IP_MCtf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_FAILED;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_mc_ip.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_FAILED;
                }

                mac_uint32[1] = ((uint32)pL2_entry->l2mcast.mac.octet[0]) << 8;
                mac_uint32[1] |= ((uint32)pL2_entry->l2mcast.mac.octet[1]);
                mac_uint32[0] = ((uint32)pL2_entry->l2mcast.mac.octet[2]) << 24;
                mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[3]) << 16;
                mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[4]) << 8;
                mac_uint32[0] |= ((uint32)pL2_entry->l2mcast.mac.octet[5]);

                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_IP_MCtf, &mac_uint32[0], (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, dip=%d, sip=%d",
                        pL2_entry->ipmcast_ip_mc_sip.index, pL2_entry->ipmcast_ip_mc_sip.rvid, pL2_entry->ipmcast_ip_mc_sip.dip, pL2_entry->ipmcast_ip_mc_sip.sip);

                if (pL2_entry->ipmcast_ip_mc_sip.dip == 0)
                {/* if dip is zero, mean delete entry. So set ip_multi = 0 */
                    ip_multi = 0;
                    ip6_multi = 0;
                }
                else
                {
                    ip_multi = 1;
                    ip6_multi = 1;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_IP_MCtf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_IP6_MCtf, &ip6_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_GIPtf, &pL2_entry->ipmcast_ip_mc_sip.dip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_SIPtf, &pL2_entry->ipmcast_ip_mc_sip.sip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc_sip.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc_sip.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "index=%d, rvid=%d, dip=%d", pL2_entry->ipmcast_ip_mc.index, pL2_entry->ipmcast_ip_mc.rvid, pL2_entry->ipmcast_ip_mc.dip);

                if (pL2_entry->ipmcast_ip_mc_sip.dip == 0)
                {/* if dip is zero, mean delete entry. So set ip_multi = 0 */
                    ip_multi = 0;
                    ip6_multi = 0;
                }
                else
                {
                    ip_multi = 1;
                    ip6_multi = 1;
                }

                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_IP_MCtf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_IP6_MCtf, &ip6_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_CAM_IP_MC_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_GIPtf, &pL2_entry->ipmcast_ip_mc.dip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_set(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
            }

            break;
        default:
            return RT_ERR_FAILED;
     }

     /* write entry to chip */
    if ((ret = table_write(unit, CYPRESS_L2_CAM_UCt, l2_index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of _dal_cypress_l2_setL2CAMEntry */


/* Function Name:
 *      _dal_cypress_l2_getExistOrFreeL2Entry
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
 *      RT_ERR_VLAN_VID - invalid vid
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
static int32
_dal_cypress_l2_getExistOrFreeL2Entry(uint32 unit, dal_cypress_l2_entry_t *pL2_entry, dal_cypress_l2_getMethod_t get_method
        , dal_cypress_l2_index_t *pL2_index)
{
    int32   ret;
    dal_cypress_l2_entry_t  l2_entry;
    uint32  hashkey;
    uint32  hash_depth;
    uint32  cam_index;
    uint32  l2cam_tableSize;
    uint32  isValid;
    uint32  found_exist;
    uint32  found_free;


    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, entry_type=%d, Mac=%x-%x-%x-%x-%x-%x\
           get_method=%d, nh=%d",
           unit, pL2_entry->entry_type, pL2_entry->unicast.mac.octet[0], pL2_entry->unicast.mac.octet[1],
           pL2_entry->unicast.mac.octet[2], pL2_entry->unicast.mac.octet[3], pL2_entry->unicast.mac.octet[4],
           pL2_entry->unicast.mac.octet[5], get_method, pL2_entry->unicast.nh);

    /* parameter check */
    RT_PARAM_CHK((NULL == pL2_entry) || (NULL == pL2_index), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((get_method >= DAL_CYPRESS_GETMETHOD_END), RT_ERR_INPUT);

    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));

    pL2_entry->is_entry_exist = FALSE;

    /* calculate hash key */
    if ((ret = _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &hashkey)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    found_exist = FALSE;
    found_free = FALSE;

    /* Search L2 unicast entry in hash table */
    for (hash_depth = 0; hash_depth < HAL_L2_HASHDEPTH(unit); hash_depth++)
    {
        if (_dal_cypress_l2_getL2EntryfromHash(unit, hashkey, hash_depth, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }

        if ((TRUE == isValid)
            && (get_method == L2_GET_EXIST_ONLY || get_method == L2_GET_EXIST_OR_FREE)
            && (l2_entry.entry_type == pL2_entry->entry_type))
        {
#if 0
            if (l2_entry.entry_type == L2_UNICAST && (l2_entry.unicast.nh == TRUE))
                continue;
#endif
            if (HAL_IS_ESCHIP(unit) && (l2_entry.unicast.nh != pL2_entry->unicast.nh))
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L3), "nh mismatch");
                continue;
            }

            if (HAL_IS_ESCHIP(unit) && (l2_entry.unicast.nh == TRUE))
            {
                rtk_l3_routeEntry_t l3_entry;

                /* check next hop MAC */
                osal_memset(&l3_entry, 0, sizeof(rtk_l3_routeEntry_t));
                if (RT_ERR_OK != (ret = dal_cypress_l3_routeEntry_get(unit,
                        l2_entry.unicast.route_idx, &l3_entry)))
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                    return ret;
                }

                if (0 != osal_memcmp(&pL2_entry->unicast.mac, &l3_entry.hostMac, sizeof(rtk_mac_t)))
                {
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L3), "route idx:%d MAC mismatch", l2_entry.unicast.route_idx);
                    continue;
                }

                found_exist = TRUE;
                pL2_index->index_type = L2_IN_HASH;
                pL2_index->index = hashkey;
                pL2_index->hashdepth = hash_depth;

                osal_memcpy(&l2_entry.unicast.mac, &pL2_entry->unicast.mac, sizeof(rtk_mac_t));
                l2_entry.unicast.fid = pL2_entry->unicast.fid;

                osal_memcpy(pL2_entry, &l2_entry, sizeof(l2_entry));
                pL2_entry->is_entry_exist = TRUE;

                return RT_ERR_OK;
            }

            if (_dal_cypress_l2_compareEntry(&l2_entry, pL2_entry) == RT_ERR_OK)
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
            if (L2_GET_FREE_ONLY == get_method)
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

    if ((ret = table_size_get(unit, CYPRESS_L2_CAM_UCt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }


    for (cam_index = 0; l2CamEbl && (cam_index < l2cam_tableSize); cam_index++)
    {
        if (_dal_cypress_l2_getL2EntryfromCAM(unit, cam_index, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found */
            break;
        }

        if ((TRUE == isValid)
            && (get_method == L2_GET_EXIST_ONLY || get_method == L2_GET_EXIST_OR_FREE)
            && (l2_entry.entry_type == pL2_entry->entry_type))
        {
            if (_dal_cypress_l2_compareEntry(&l2_entry, pL2_entry) == RT_ERR_OK)
            {
                found_exist = TRUE;
                pL2_index->index_type = L2_IN_CAM;
                pL2_index->index = cam_index;
                pL2_index->hashdepth = 0;
                osal_memcpy(pL2_entry, &l2_entry, sizeof(l2_entry));
                pL2_entry->is_entry_exist = TRUE;
                return RT_ERR_OK;
            }
        }
        else if (FALSE == isValid)
        {
            if (L2_GET_FREE_ONLY == get_method)
            {
                found_free = TRUE;
                pL2_index->index_type = L2_IN_CAM;
                pL2_index->index = cam_index;
                pL2_index->hashdepth = 0;
                return RT_ERR_OK;
            }
            else if ((L2_GET_EXIST_OR_FREE == get_method) && FALSE == found_free)
            {
                found_free = TRUE;
                pL2_index->index_type = L2_IN_CAM;
                pL2_index->index = cam_index;
                pL2_index->hashdepth = 0;
            }
        }
    }

    if ((L2_GET_EXIST_OR_FREE == get_method) && (TRUE == found_free))
    {
        return RT_ERR_OK;
    }
    else if ((L2_GET_EXIST_ONLY == get_method) && (FALSE == found_exist))
    {
        return RT_ERR_L2_ENTRY_NOTFOUND;
    }

    return RT_ERR_L2_NO_EMPTY_ENTRY;
} /* end of _dal_cypress_l2_getExistOrFreeL2Entry */

/* Function Name:
 *      _dal_cypress_l2_getFirstDynamicEntry
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
_dal_cypress_l2_getFirstDynamicEntry(uint32 unit, dal_cypress_l2_entry_t *pL2_entry
        , dal_cypress_l2_index_t *pL2_index)
{
    int32   ret;
    dal_cypress_l2_entry_t  l2_entry;
    uint32  hashkey;
    uint32  hash_depth;
    uint32  cam_index;
    uint32  l2cam_tableSize;
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
    if ((ret = _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &hashkey)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    found_exist = FALSE;

    /* Search L2 unicast entry in hash table */
    for (hash_depth = 0; hash_depth < HAL_L2_HASHDEPTH(unit); hash_depth++)
    {
        if (_dal_cypress_l2_getL2EntryfromHash(unit, hashkey, hash_depth, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }

        if ((TRUE == isValid ) && (l2_entry.entry_type == L2_UNICAST))
        {
            if (l2_entry.unicast.sablock == 0 && l2_entry.unicast.dablock == 0 &&
                l2_entry.unicast.is_static == 0 && l2_entry.unicast.nh == 0 && l2_entry.unicast.suspending == 0)
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

    if ((ret = table_size_get(unit, CYPRESS_L2_CAM_UCt, &l2cam_tableSize)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    for (cam_index = 0; l2CamEbl && (cam_index < l2cam_tableSize); cam_index++)
    {
        if (_dal_cypress_l2_getL2EntryfromCAM(unit, cam_index, &l2_entry, &isValid) != RT_ERR_OK)
        {
            /* not found in hash table, search CAM */
            break;
        }

        if ((TRUE == isValid ) && (l2_entry.entry_type == L2_UNICAST))
        {
            if (l2_entry.unicast.sablock == 0 && l2_entry.unicast.dablock == 0 &&
                l2_entry.unicast.is_static == 0 && l2_entry.unicast.suspending == 0)
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
}

/* Function Name:
 *      _dal_cypress_l2_getL2EntryfromHash
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
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      We can not distinguish an entry between IP multicast in L2 mode and L2 multicast
 */
int32
_dal_cypress_l2_getL2EntryfromHash(uint32 unit, uint32 hashkey, uint32 location, dal_cypress_l2_entry_t *pL2_entry, uint32 *pIsValid)
{
    int32   ret;
    l2_entry_t  l2_entry;
    uint32  l2_index;
    uint32  ip_multi, ip6_multi;
    uint32  mac_uint32[2];
    uint32  is_l2mcast;
    uint32  aging;
    uint32  is_static;
    uint32  dablock, sablock;
    uint32  next_hop;
    uint32  suspending;
    uint32  agg_vid = 0;
    uint32  vid_sel = 0;
    uint32  mac_idx = 0;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, hashkey=%d, location=%d, \
           entry_type=%d, Mac=%x-%x-%x-%x-%x-%x", unit, hashkey, location, pL2_entry->entry_type,
           pL2_entry->unicast.mac.octet[0], pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2],
           pL2_entry->unicast.mac.octet[3], pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* calculate l2 index in hash table */
    l2_index = (SRAM << RTL8390_TBL_ACCESS_L2_CTRL_TBL_OFFSET) | (hashkey << 2) | location;

    /* read entry from chip */
    if ((ret = table_read(unit, CYPRESS_L2_UCt, l2_index, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* get IP_MULTI and IP6_MULTI bits from l2_entry */
    if ((ret = table_field_get(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_IP_MCtf, &ip_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    if ((ret = table_field_get(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_IP6_MCtf, &ip6_multi, (uint32 *)&l2_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* Get mac addr */
    is_l2mcast = 0;
    if (0 == ip_multi)
    {
        if ((ret = table_field_get(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_MACtf, &mac_uint32[0], (uint32 *)&l2_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
            is_l2mcast = (uint8) ((mac_uint32[1] >> 8) & BITMASK_1B);
    }

    aging = 0;
    sablock = 0;
    dablock = 0;
    is_static = 0;

    /*
     * check whether this entry is l2 multicast entry.
     * If not l2 multicast entry, get more information for valid check
     */
    if (0 == is_l2mcast && 0 == ip_multi)
    {
        if ((ret = table_field_get(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_AGEtf, &aging, (uint32 *)&l2_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_SA_BLKtf, &sablock, (uint32 *)&l2_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_DA_BLKtf, &dablock, (uint32 *)&l2_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_STATICtf, &is_static, (uint32 *)&l2_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_SUSPENDtf, &suspending, (uint32 *)&l2_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_NEXT_HOPtf, &next_hop, (uint32 *)&l2_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        /* usual unicast entry or NextHop enry */
        if (next_hop == TRUE)
        {
            if ((ret = table_field_get(unit, CYPRESS_L2_NEXT_HOPt, CYPRESS_L2_NEXT_HOP_VLAN_TARGETtf, &vid_sel, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }
            if ((ret = table_field_get(unit, CYPRESS_L2_NEXT_HOPt, CYPRESS_L2_NEXT_HOP_ROUTE_IDXtf, &mac_idx, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }
        }
        else
        {
            if ((ret = table_field_get(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_VIDtf, &agg_vid, (uint32 *)&l2_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }
        }
    }
    /* check whether this entry is valid entry */
    if (!(ip_multi | is_l2mcast | aging | sablock | dablock | is_static | next_hop))
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

    /* Extract content of each kind of entry from l2_entry */
    switch ((ip6_multi << 2) | (ip_multi << 1) | is_l2mcast)
    {
        case 0x0: /* L2 unicast */
            pL2_entry->entry_type = L2_UNICAST;
            if (HAL_IS_ESCHIP(unit) && next_hop)
            {
                if ((ret = table_field_get(unit, CYPRESS_L2_NH_LEGACYt, CYPRESS_L2_NH_LEGACY_HOSTMAC_IDXtf, &pL2_entry->unicast.route_idx, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_NH_LEGACYt, CYPRESS_L2_NH_LEGACY_VID_SELtf, &pL2_entry->unicast.vlan_target, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_NH_LEGACYt, CYPRESS_L2_NH_LEGACY_DVIDtf, &pL2_entry->unicast.fid, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_NH_LEGACYt, CYPRESS_L2_NH_LEGACY_DPAtf, &pL2_entry->unicast.port, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_NH_LEGACYt, CYPRESS_L2_NH_LEGACY_NEXT_HOPtf, &pL2_entry->unicast.nh, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L3), "");
                    return ret;
                }

                break;
            }
            else
            {
                if ((ret = table_field_get(unit, CYPRESS_L2_UCt, CYPRESS_L2_UC_SLPtf, &pL2_entry->unicast.port, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                pL2_entry->unicast.mac.octet[0] = (uint8)(mac_uint32[1] >> 8);
                pL2_entry->unicast.mac.octet[1] = (uint8)(mac_uint32[1]);
                pL2_entry->unicast.mac.octet[2] = (uint8)(mac_uint32[0] >> 24);
                pL2_entry->unicast.mac.octet[3] = (uint8)(mac_uint32[0] >> 16);
                pL2_entry->unicast.mac.octet[4] = (uint8)(mac_uint32[0] >> 8);
                pL2_entry->unicast.mac.octet[5] = (uint8)(mac_uint32[0]);
                pL2_entry->unicast.sablock      = sablock;
                pL2_entry->unicast.dablock      = dablock;
                pL2_entry->unicast.is_static    = is_static;
                pL2_entry->unicast.aging        = aging;
                pL2_entry->unicast.suspending   = suspending;
                pL2_entry->unicast.nh           = next_hop;
                pL2_entry->unicast.agg_vid      = agg_vid;
                pL2_entry->unicast.vlan_target  = vid_sel;
                pL2_entry->unicast.route_idx    = mac_idx;
            }

            /* convert FID from hash algorithm */
            _dal_cypress_l2_hashKeyToVid(unit, pL2_entry, hashkey);

            break;
        case 0x1: /* l2 Multicast */
            pL2_entry->entry_type = L2_MULTICAST;

            if (table_field_get(unit, CYPRESS_L2_MCt, CYPRESS_L2_MC_MC_PMSK_IDXtf, &pL2_entry->l2mcast.index, (uint32 *)&l2_entry) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return RT_ERR_FAILED;
            }

            pL2_entry->l2mcast.mac.octet[0] = (uint8)(mac_uint32[1] >> 8);
            pL2_entry->l2mcast.mac.octet[1] = (uint8)(mac_uint32[1]);
            pL2_entry->l2mcast.mac.octet[2] = (uint8)(mac_uint32[0] >> 24);
            pL2_entry->l2mcast.mac.octet[3] = (uint8)(mac_uint32[0] >> 16);
            pL2_entry->l2mcast.mac.octet[4] = (uint8)(mac_uint32[0] >> 8);
            pL2_entry->l2mcast.mac.octet[5] = (uint8)(mac_uint32[0]);

            /* convert RVID from hash algorithm */

            _dal_cypress_l2_hashKeyToVid(unit, pL2_entry, hashkey);
            break;
        case 0x2: /* IPv4 multicast */
            pL2_entry->entry_type = IP4_MULTICAST;
            if (LOOKUP_ON_FVID_AND_MAC == ip4HashFmt[unit])
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Should not come here");
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
            {
                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc_sip.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc_sip.rvid, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_GIPtf, &pL2_entry->ipmcast_ip_mc_sip.dip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_SIPtf, &pL2_entry->ipmcast_ip_mc_sip.sip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                pL2_entry->ipmcast_ip_mc_sip.sip &= 0x000fffff;
                _dal_cypress_l2_hashKeyToVid(unit, pL2_entry, hashkey);
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
            {
                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc.rvid, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_GIPtf, &pL2_entry->ipmcast_ip_mc.dip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                /* add 0xE in MSB four bit in DIP */
                pL2_entry->ipmcast_ip_mc.dip = (0xE << 28) | pL2_entry->ipmcast_ip_mc.dip;
                _dal_cypress_l2_hashKeyToVid(unit, pL2_entry, hashkey);
            }

            break;
        case 0x6: /* IPv6 multicast */
            pL2_entry->entry_type = IP6_MULTICAST;
            if (LOOKUP_ON_FVID_AND_MAC == ip6HashFmt[unit])
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Should not come here");
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
            {
                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc_sip.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc_sip.rvid, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_GIPtf, &pL2_entry->ipmcast_ip_mc_sip.dip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MC_SIPt, CYPRESS_L2_IP_MC_SIP_SIPtf, &pL2_entry->ipmcast_ip_mc_sip.sip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                /* add 0xE in MSB four bit in DIP */
                pL2_entry->ipmcast_ip_mc_sip.sip &= 0x000fffff;
                _dal_cypress_l2_hashKeyToVid(unit, pL2_entry, hashkey);
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
            {
                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc.index, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc.rvid, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                if ((ret = table_field_get(unit, CYPRESS_L2_IP_MCt, CYPRESS_L2_IP_MC_GIPtf, &pL2_entry->ipmcast_ip_mc.dip, (uint32 *)&l2_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                _dal_cypress_l2_hashKeyToVid(unit, pL2_entry, hashkey);
            }
            break;

        default:
            return RT_ERR_FAILED;
     }

     RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pIsValid=%d", *pIsValid);

     return RT_ERR_OK;
} /* end of _dal_cypress_l2_getL2EntryfromHash */

/* Function Name:
 *      _dal_cypress_l2_getL2EntryfromCAM
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
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      We can not distinguish an entry between IP multicast in L2 mode and L2 multicast
 */
static int32
_dal_cypress_l2_getL2EntryfromCAM(uint32 unit, uint32 index, dal_cypress_l2_entry_t *pL2_entry, uint32 *pIsValid)
{
    int32   ret;
    l2cam_entry_t  l2cam_entry;
    uint32  l2_index;
    uint32  ip_multi, ip6_multi;
    uint32  mac_uint32[2];
    uint32  is_l2mcast;
    uint32  aging;
    uint32  is_static;
    uint32  dablock, sablock;
    uint32  next_hop;
    uint32  suspending;
    uint32  agg_vid;

    RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "unit=%d, index=%d, entry_type=%d, \
           Mac=%x-%x-%x-%x-%x-%x", unit, index, pL2_entry->entry_type, pL2_entry->unicast.mac.octet[0],
           pL2_entry->unicast.mac.octet[1], pL2_entry->unicast.mac.octet[2], pL2_entry->unicast.mac.octet[3],
           pL2_entry->unicast.mac.octet[4], pL2_entry->unicast.mac.octet[5]);

    /* calculate l2 index in hash table */
    l2_index = index;

    /* read entry from chip */
    if ((ret = table_read(unit, CYPRESS_L2_CAM_UCt, l2_index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    /* get IP_MULTI and IP6_MULTI bits from l2_entry */
    if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_IP_MCtf, &ip_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_IP6_MCtf, &ip6_multi, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    is_l2mcast = 0;
    /* Get mac addr */
    if (0 == ip_multi)
    {
        if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_MACtf, &mac_uint32[0], (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_NEXT_HOPtf, &next_hop, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if (0 == next_hop)
            is_l2mcast = (uint8) ((mac_uint32[1] >> 8) & BITMASK_1B);
    }

    aging = 0;
    sablock = 0;
    dablock = 0;
    is_static = 0;

    /*
     * check whether this entry is l2 multicast entry.
     * If not l2 multicast entry, get more information for valid check
     */
    if (0 == is_l2mcast && 0 == ip_multi)
    {
        if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_AGEtf, &aging, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_SA_BLKtf, &sablock, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }


        if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_DA_BLKtf, &dablock, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_STATICtf, &is_static, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_SUSPENDtf, &suspending, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

        if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_VIDtf, &agg_vid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }

    }

    /* check whether this entry is valid entry */
    if (!(ip_multi | is_l2mcast | aging | sablock | dablock | is_static | next_hop))
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
    switch ((ip6_multi << 2) | (ip_multi << 1) | is_l2mcast)
    {
        case 0x00: /* L2 unicast */
            pL2_entry->entry_type = L2_UNICAST;
            if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_SLPtf, &pL2_entry->unicast.port, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_get(unit, CYPRESS_L2_CAM_UCt, CYPRESS_L2_CAM_UC_FID_RVIDtf, &pL2_entry->unicast.fid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            pL2_entry->unicast.mac.octet[0] = (uint8)(mac_uint32[1] >> 8);
            pL2_entry->unicast.mac.octet[1] = (uint8)(mac_uint32[1]);
            pL2_entry->unicast.mac.octet[2] = (uint8)(mac_uint32[0] >> 24);
            pL2_entry->unicast.mac.octet[3] = (uint8)(mac_uint32[0] >> 16);
            pL2_entry->unicast.mac.octet[4] = (uint8)(mac_uint32[0] >> 8);
            pL2_entry->unicast.mac.octet[5] = (uint8)(mac_uint32[0]);

            pL2_entry->unicast.sablock      = sablock;
            pL2_entry->unicast.dablock      = dablock;
            pL2_entry->unicast.is_static    = is_static;
            pL2_entry->unicast.aging        = aging;
            pL2_entry->unicast.suspending   = suspending;
            pL2_entry->unicast.nh           = next_hop;
            pL2_entry->unicast.agg_vid      = agg_vid;

            break;
        case 0x01: /* l2 Multicast */
            pL2_entry->entry_type = L2_MULTICAST;
            if ((ret = table_field_get(unit, CYPRESS_L2_CAM_MCt, CYPRESS_L2_CAM_MC_MC_PMSK_IDXtf, &pL2_entry->l2mcast.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            if ((ret = table_field_get(unit, CYPRESS_L2_CAM_MCt, CYPRESS_L2_CAM_MC_FID_RVIDtf, &pL2_entry->l2mcast.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                return ret;
            }

            pL2_entry->l2mcast.mac.octet[0] = (uint8)(mac_uint32[1] >> 8);
            pL2_entry->l2mcast.mac.octet[1] = (uint8)(mac_uint32[1]);
            pL2_entry->l2mcast.mac.octet[2] = (uint8)(mac_uint32[0] >> 24);
            pL2_entry->l2mcast.mac.octet[3] = (uint8)(mac_uint32[0] >> 16);
            pL2_entry->l2mcast.mac.octet[4] = (uint8)(mac_uint32[0] >> 8);
            pL2_entry->l2mcast.mac.octet[5] = (uint8)(mac_uint32[0]);

            break;
        case 0x2: /* IPv4 multicast */
            pL2_entry->entry_type = IP4_MULTICAST;
            if (LOOKUP_ON_FVID_AND_MAC == ip4HashFmt[unit])
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Should not come here");
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
            {
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc_sip.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc_sip.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_SIPtf, &pL2_entry->ipmcast_ip_mc_sip.sip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_GIPtf, &pL2_entry->ipmcast_ip_mc_sip.dip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                /* add 0xE in MSB four bit in DIP */
                pL2_entry->ipmcast_ip_mc_sip.dip = (0xE << 28) | pL2_entry->ipmcast_ip_mc_sip.dip;
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
            {
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_GIPtf, &pL2_entry->ipmcast_ip_mc.dip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                /* add 0xE in MSB four bit in DIP */
                pL2_entry->ipmcast_ip_mc.dip = (0xE << 28) | pL2_entry->ipmcast_ip_mc.dip;
            }

            break;
        case 0x6: /* IPv6 multicast */
            pL2_entry->entry_type = IP6_MULTICAST;
            if (LOOKUP_ON_FVID_AND_MAC == ip6HashFmt[unit])
            {
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "Should not come here");
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
            {
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc_sip.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc_sip.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_SIPtf, &pL2_entry->ipmcast_ip_mc_sip.sip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MC_SIPt, CYPRESS_L2_CAM_IP_MC_SIP_GIPtf, &pL2_entry->ipmcast_ip_mc_sip.dip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                /* add 0xE in MSB four bit in DIP */
                pL2_entry->ipmcast_ip_mc_sip.dip = pL2_entry->ipmcast_ip_mc_sip.dip;
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
            {
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_MC_PMSK_IDXtf, &pL2_entry->ipmcast_ip_mc.index, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_FID_RVIDtf, &pL2_entry->ipmcast_ip_mc.rvid, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }
                if ((ret = table_field_get(unit, CYPRESS_L2_CAM_IP_MCt, CYPRESS_L2_CAM_IP_MC_GIPtf, &pL2_entry->ipmcast_ip_mc.dip, (uint32 *)&l2cam_entry)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return ret;
                }

                /* add 0xE in MSB four bit in DIP */
                pL2_entry->ipmcast_ip_mc.dip = pL2_entry->ipmcast_ip_mc.dip;
            }

            break;
        default:
            return RT_ERR_FAILED;
     }

     RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pIsValid=%d", *pIsValid);

     return RT_ERR_OK;
} /* end of _dal_cypress_l2_getL2EntryfromCAM */

int32 _dal_cypress_l2_ip6CareByteConvertToIP(ipaddr_t *pIp4Dip, ipaddr_t *pIp4Sip, rtk_ipv6_addr_t *pIp6Dip, rtk_ipv6_addr_t *pIp6Sip)
        {
    uint8 byte[4];

    byte[0] = pIp6Dip->ipv6_addr[15 - (ip6DipCareByte & 0xf)];
    byte[1] = pIp6Dip->ipv6_addr[15 - ((ip6DipCareByte & 0xf0) >> 4)];
    byte[2] = pIp6Dip->ipv6_addr[15 - ((ip6DipCareByte & 0xf00) >> 8)];
    byte[3] = pIp6Dip->ipv6_addr[15 - ((ip6DipCareByte & 0xf000) >> 12)];
    *pIp4Dip = (byte[3] << 24) | (byte[2] << 16) | (byte[1] << 8) | byte[0];

    byte[0] = pIp6Sip->ipv6_addr[15 - (ip6SipCareByte & 0xf)];
    byte[1] = pIp6Sip->ipv6_addr[15 - ((ip6SipCareByte & 0xf0) >> 4)];
    byte[2] = pIp6Sip->ipv6_addr[15 - ((ip6SipCareByte & 0xf00) >> 8)];
    byte[3] = pIp6Sip->ipv6_addr[15 - ((ip6SipCareByte & 0xf000) >> 12)];
    *pIp4Sip = (byte[3] << 24) | (byte[2] << 16) | (byte[1] << 8) | byte[0];

        return RT_ERR_OK;
    }

#if 0
int32 _dal_cypress_l2_ip6CareByteConvertToIP6Addr(rtk_ipv6_addr_t *pIp6Dip, rtk_ipv6_addr_t *pIp6Sip, ipaddr_t *pIp4Dip, ipaddr_t *pIp4Sip)
                {
    uint8 byte[4];

    byte[0] = *pIp4Dip & 0xff;
    byte[1] = (*pIp4Dip & 0xff00) >> 8;
    byte[2] = (*pIp4Dip & 0xff0000) >> 16;
    byte[3] = (*pIp4Dip & 0xff000000) >> 24;
    pIp6Dip->ipv6_addr[15 - (ip6DipCareByte & 0xf)] = byte[0];
    pIp6Dip->ipv6_addr[15 - ((ip6DipCareByte & 0xf0) >> 4)] = byte[1];
    pIp6Dip->ipv6_addr[15 - ((ip6DipCareByte & 0xf00) >> 8)] = byte[2];
    pIp6Dip->ipv6_addr[15 - ((ip6DipCareByte & 0xf000) >> 12)] = byte[3];

    byte[0] = *pIp4Sip & 0xff;
    byte[1] = (*pIp4Sip & 0xff00) >> 8;
    byte[2] = (*pIp4Sip & 0xff0000) >> 16;
    byte[3] = (*pIp4Sip & 0xff000000) >> 24;
    pIp6Sip->ipv6_addr[15 - (ip6SipCareByte & 0xf)] = byte[0];
    pIp6Sip->ipv6_addr[15 - ((ip6SipCareByte & 0xf0) >> 4)] = byte[1];
    pIp6Sip->ipv6_addr[15 - ((ip6SipCareByte & 0xf00) >> 8)] = byte[2];
    pIp6Sip->ipv6_addr[15 - ((ip6SipCareByte & 0xf000) >> 12)] = byte[3];

     return RT_ERR_OK;
}
#endif

/* Function Name:
 *      _dal_cypress_l2_entryToHashKey
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
_dal_cypress_l2_entryToHashKey(uint32 unit, dal_cypress_l2_entry_t *pL2_entry, uint32 *pKey)
{
    uint64  hashSeed = 0;
    uint32  hash11_6, hash5_0;
    uint32  tmp;

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
        case IP4_MULTICAST:
            if (LOOKUP_ON_FVID_AND_MAC == ip4HashFmt[unit])
            {
                hashSeed = ((uint64)pL2_entry->l2mcast.rvid << 48) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[0] << 40) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[1] << 32) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[2] << 24) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[3] << 16) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[4] << 8) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[5]);
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
            {
                /* sip+dip(LSB 28bits) */
                hashSeed = (((uint64)pL2_entry->ipmcast_ip_mc_sip.dip & 0xf) << 60) | ((uint64)pL2_entry->ipmcast_ip_mc_sip.sip << 28) |
                        ((uint64)pL2_entry->ipmcast_ip_mc_sip.dip >> 4);
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
            {
                /* GIP[3:0], FID/VID(12-bit), 0(20-bit), GIP[31:4] */
                hashSeed = (((uint64)pL2_entry->ipmcast_ip_mc.dip & 0xf) << 60) | ((uint64)pL2_entry->ipmcast_ip_mc.rvid << 48) |
                        ((uint64)pL2_entry->ipmcast_ip_mc.dip >> 4);
            }
            break;
        case IP6_MULTICAST:
            if (LOOKUP_ON_FVID_AND_MAC == ip6HashFmt[unit])
            {
                hashSeed = ((uint64)pL2_entry->l2mcast.rvid << 48) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[0] << 40) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[1] << 32) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[2] << 24) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[3] << 16) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[4] << 8) |
                        ((uint64)pL2_entry->ipmcast_mc_ip.mac.octet[5]);
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
            {
                /* sip+dip(LSB 28bits) */   /*TODO: compose from 128B*/
                hashSeed = (((uint64)pL2_entry->ipmcast_ip_mc_sip.dip & 0xf) << 60) | ((uint64)pL2_entry->ipmcast_ip_mc_sip.sip << 28) |
                        ((uint64)pL2_entry->ipmcast_ip_mc_sip.dip >> 4);
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
            {
                /* GIP[3:0], FID/VID(12-bit), 0(20-bit), GIP[31:4] */
                hashSeed = (((uint64)pL2_entry->ipmcast_ip_mc.dip & 0xf) << 60) | ((uint64)pL2_entry->ipmcast_ip_mc.rvid << 48) |
                        ((uint64)pL2_entry->ipmcast_ip_mc.dip >> 4);
            }
            break;
        default:
            return RT_ERR_FAILED;
    }

    /* TBD */
    if (0 == algoType[unit])
    {
        tmp = (((hashSeed >> 48) & BITMASK_6B) << 6) | ((hashSeed >> 54) & BITMASK_6B);
        *pKey = (uint32) ((hashSeed >> 60)
                        ^ tmp
                        ^ ((hashSeed >> 36) & BITMASK_12B)
                        ^ ((hashSeed >> 24) & BITMASK_12B)
                        ^ ((hashSeed >> 12) & BITMASK_12B)
                        ^ (hashSeed  & BITMASK_12B));
    }
    else if (1 == algoType[unit])
    {
        hash11_6 = (uint32) (((hashSeed >> 60) & BITMASK_6B)
                        ^ ((hashSeed >> 54) & BITMASK_6B)
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
} /* end of _dal_cypress_l2_entryToHashKey */

static int32 _dal_cypress_l2_hashKeyToVid(uint32 unit, dal_cypress_l2_entry_t *pL2_entry, uint32 key)
{
    uint32  partialkey1;


    if (0 == algoType[unit])
    {
        switch (pL2_entry->entry_type)
        {
            case L2_UNICAST:
                pL2_entry->unicast.fid = ((key & BITMASK_6B) << 6) | (key >> 6);
                _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                pL2_entry->unicast.fid  = ((partialkey1 & BITMASK_6B) << 6) | (partialkey1 >> 6);
                break;

            case L2_MULTICAST:
                pL2_entry->l2mcast.rvid = ((key & BITMASK_6B) << 6) | (key >> 6);
                _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                pL2_entry->l2mcast.rvid  = ((partialkey1 & BITMASK_6B) << 6) | (partialkey1 >> 6);
                break;

            case IP4_MULTICAST:
                if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
                {
                    pL2_entry->ipmcast_ip_mc_sip.dip = (0xE << 28) | pL2_entry->ipmcast_ip_mc_sip.dip;
                    pL2_entry->ipmcast_ip_mc_sip.sip = ((((key & BITMASK_6B) << 6) | (key >> 6)) << 20) | pL2_entry->ipmcast_ip_mc_sip.sip;
                    _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                    pL2_entry->ipmcast_ip_mc_sip.sip = ((((partialkey1 & BITMASK_6B) << 6) | (partialkey1 >> 6)) << 20) | (pL2_entry->ipmcast_ip_mc_sip.sip & BITMASK_20B);
                }
                else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
                {
                    pL2_entry->ipmcast_ip_mc.rvid = ((key & BITMASK_6B) << 6) | (key >> 6);
                    _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                    pL2_entry->ipmcast_ip_mc.rvid  = ((partialkey1 & BITMASK_6B) << 6) | (partialkey1 >> 6);
                }
                break;

            case IP6_MULTICAST:
                if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
                {
                    pL2_entry->ipmcast_ip_mc_sip.dip = pL2_entry->ipmcast_ip_mc_sip.dip;
                    pL2_entry->ipmcast_ip_mc_sip.sip = ((((key & BITMASK_6B) << 6) | (key >> 6)) << 20) | pL2_entry->ipmcast_ip_mc_sip.sip;
                    _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                    pL2_entry->ipmcast_ip_mc_sip.sip = ((((partialkey1 & BITMASK_6B) << 6) | (partialkey1 >> 6)) << 20) | (pL2_entry->ipmcast_ip_mc_sip.sip & BITMASK_20B);
                }
                else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
                {
                    pL2_entry->ipmcast_ip_mc.rvid = ((key & BITMASK_6B) << 6) | (key >> 6);
                    _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                    pL2_entry->ipmcast_ip_mc.rvid  = ((partialkey1 & BITMASK_6B) << 6) | (partialkey1 >> 6);
                }
                break;

            default:
                return RT_ERR_FAILED;
        }
    }
    else if (1 == algoType[unit])
    {
        switch (pL2_entry->entry_type)
        {
            case L2_UNICAST:
                pL2_entry->unicast.fid = key;
                _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                pL2_entry->unicast.fid  = partialkey1;
                break;

            case L2_MULTICAST:
                pL2_entry->l2mcast.rvid = key;
                _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                pL2_entry->l2mcast.rvid  = partialkey1;
                break;

            case IP4_MULTICAST:
                if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
                {
                    pL2_entry->ipmcast_ip_mc_sip.dip = (0xE << 28) | pL2_entry->ipmcast_ip_mc_sip.dip;
                    pL2_entry->ipmcast_ip_mc_sip.sip = (key << 20) | pL2_entry->ipmcast_ip_mc_sip.sip;
                    _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                    pL2_entry->ipmcast_ip_mc_sip.sip = (partialkey1 << 20) | (pL2_entry->ipmcast_ip_mc_sip.sip & BITMASK_20B);
                }
                else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
                {
                    pL2_entry->ipmcast_ip_mc.rvid = key;
                    _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                    pL2_entry->ipmcast_ip_mc.rvid  = partialkey1;
                }
                break;

            case IP6_MULTICAST:
                if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
                {
                    pL2_entry->ipmcast_ip_mc_sip.dip = pL2_entry->ipmcast_ip_mc_sip.dip;
                    pL2_entry->ipmcast_ip_mc_sip.sip = (key << 20) | pL2_entry->ipmcast_ip_mc_sip.sip;
                    _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                    pL2_entry->ipmcast_ip_mc_sip.sip = (partialkey1 << 20) | (pL2_entry->ipmcast_ip_mc_sip.sip & BITMASK_20B);
                }
                else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
                {
                    pL2_entry->ipmcast_ip_mc.rvid = key;
                    _dal_cypress_l2_entryToHashKey(unit, pL2_entry, &partialkey1);
                    pL2_entry->ipmcast_ip_mc.rvid  = partialkey1;
                }
                break;

            default:
                return RT_ERR_FAILED;
        }
    }
    else
    {
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      _dal_cypress_l2_compareEntry
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
_dal_cypress_l2_compareEntry(dal_cypress_l2_entry_t *pSrcEntry, dal_cypress_l2_entry_t *pDstEntry)
{
    uint32  unit = 0;

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
        case IP4_MULTICAST:
            if (LOOKUP_ON_FVID_AND_MAC == ip4HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "srcrvid=%d, dstrvid=%d, srcMac=%x-%x-%x-%x-%x-%x,\
                       dstMac=%x-%x-%x-%x-%x-%x", pSrcEntry->ipmcast_mc_ip.rvid, pDstEntry->ipmcast_mc_ip.rvid,
                       pSrcEntry->ipmcast_mc_ip.mac.octet[0], pSrcEntry->ipmcast_mc_ip.mac.octet[1], pSrcEntry->ipmcast_mc_ip.mac.octet[2],
                       pSrcEntry->ipmcast_mc_ip.mac.octet[3], pSrcEntry->ipmcast_mc_ip.mac.octet[4], pSrcEntry->ipmcast_mc_ip.mac.octet[5],
                       pDstEntry->ipmcast_mc_ip.mac.octet[0], pDstEntry->ipmcast_mc_ip.mac.octet[1], pDstEntry->ipmcast_mc_ip.mac.octet[2],
                       pDstEntry->ipmcast_mc_ip.mac.octet[3], pDstEntry->ipmcast_mc_ip.mac.octet[4], pDstEntry->ipmcast_mc_ip.mac.octet[5]);

                if ((osal_memcmp(&pSrcEntry->l2mcast.mac, &pDstEntry->l2mcast.mac, sizeof(rtk_mac_t)))
                   || (pSrcEntry->l2mcast.rvid != pDstEntry->l2mcast.rvid))
                {
                    return RT_ERR_FAILED;
                }
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "srcdip=%d, dstdip=%d, srcsip=%d, dstsip=%d",
                       pSrcEntry->ipmcast_ip_mc_sip.dip, pDstEntry->ipmcast_ip_mc_sip.dip, pSrcEntry->ipmcast_ip_mc_sip.sip, pDstEntry->ipmcast_ip_mc_sip.sip);

                if (ipmcst_fvid_cmp && pSrcEntry->ipmcast_ip_mc_sip.rvid != pDstEntry->ipmcast_ip_mc_sip.rvid)
                    return RT_ERR_FAILED;
                if ((pSrcEntry->ipmcast_ip_mc_sip.dip != pDstEntry->ipmcast_ip_mc_sip.dip)
                    ||(pSrcEntry->ipmcast_ip_mc_sip.sip != pDstEntry->ipmcast_ip_mc_sip.sip))
                {
                    return RT_ERR_FAILED;
                }
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "srcrvid=%d, dstrvid=%d, srcdip=%d, dstdip=%d",
                        pSrcEntry->ipmcast_ip_mc.rvid, pDstEntry->ipmcast_ip_mc.rvid,
                        pSrcEntry->ipmcast_ip_mc.dip, pDstEntry->ipmcast_ip_mc.dip);
                if ((pSrcEntry->ipmcast_ip_mc.dip != pDstEntry->ipmcast_ip_mc.dip)
                    ||(pSrcEntry->ipmcast_ip_mc.rvid != pDstEntry->ipmcast_ip_mc.rvid))
                {
                    return RT_ERR_FAILED;
                }
            }

            return RT_ERR_OK;
            break;
        case IP6_MULTICAST:
            if (LOOKUP_ON_FVID_AND_MAC == ip6HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "srcrvid=%d, dstrvid=%d, srcMac=%x-%x-%x-%x-%x-%x,\
                       dstMac=%x-%x-%x-%x-%x-%x", pSrcEntry->ipmcast_mc_ip.rvid, pDstEntry->ipmcast_mc_ip.rvid,
                       pSrcEntry->ipmcast_mc_ip.mac.octet[0], pSrcEntry->ipmcast_mc_ip.mac.octet[1], pSrcEntry->ipmcast_mc_ip.mac.octet[2],
                       pSrcEntry->ipmcast_mc_ip.mac.octet[3], pSrcEntry->ipmcast_mc_ip.mac.octet[4], pSrcEntry->ipmcast_mc_ip.mac.octet[5],
                       pDstEntry->ipmcast_mc_ip.mac.octet[0], pDstEntry->ipmcast_mc_ip.mac.octet[1], pDstEntry->ipmcast_mc_ip.mac.octet[2],
                       pDstEntry->ipmcast_mc_ip.mac.octet[3], pDstEntry->ipmcast_mc_ip.mac.octet[4], pDstEntry->ipmcast_mc_ip.mac.octet[5]);

                if ((osal_memcmp(&pSrcEntry->l2mcast.mac, &pDstEntry->l2mcast.mac, sizeof(rtk_mac_t)))
                   || (pSrcEntry->l2mcast.rvid != pDstEntry->l2mcast.rvid))
                {
                    return RT_ERR_FAILED;
                }
            }
            else if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "srcdip=%d, dstdip=%d, srcsip=%d, dstsip=%d",
                       pSrcEntry->ipmcast_ip_mc_sip.dip, pDstEntry->ipmcast_ip_mc_sip.dip, pSrcEntry->ipmcast_ip_mc_sip.sip, pDstEntry->ipmcast_ip_mc_sip.sip);
                if (ipmcst_fvid_cmp && pSrcEntry->ipmcast_ip_mc_sip.rvid != pDstEntry->ipmcast_ip_mc_sip.rvid)
                    return RT_ERR_FAILED;
                if ((pSrcEntry->ipmcast_ip_mc_sip.dip != pDstEntry->ipmcast_ip_mc_sip.dip)
                    ||(pSrcEntry->ipmcast_ip_mc_sip.sip != pDstEntry->ipmcast_ip_mc_sip.sip))
                {
                    return RT_ERR_FAILED;
                }
            }
            else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
            {
                RT_DBG(LOG_TRACE, (MOD_DAL|MOD_L2), "srcrvid=%d, dstrvid=%d, srcdip=%d, dstdip=%d",
                        pSrcEntry->ipmcast_ip_mc.rvid, pDstEntry->ipmcast_ip_mc.rvid,
                        pSrcEntry->ipmcast_ip_mc.dip, pDstEntry->ipmcast_ip_mc.dip);
                if ((pSrcEntry->ipmcast_ip_mc.dip != pDstEntry->ipmcast_ip_mc.dip)
                    ||(pSrcEntry->ipmcast_ip_mc.rvid != pDstEntry->ipmcast_ip_mc.rvid))
                {
                    return RT_ERR_FAILED;
                }
            }

            return RT_ERR_OK;
            break;
        default:
            return RT_ERR_FAILED;
    }

} /* end of _dal_cypress_l2_compareEntry*/

/* Function Name:
 *      _dal_cypress_l2_allocMcastIdx
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
static int32 _dal_cypress_l2_allocMcastIdx(uint32 unit, int32 *pMcastIdx)
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
} /* _dal_cypress_l2_allocMcastIdx */

/* Function Name:
 *      _dal_cypress_l2_freeMcastIdx
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
static int32 _dal_cypress_l2_freeMcastIdx(uint32 unit, int32 mcastIdx)
{
    int32   ret;
    multicast_index_entry_t mcast_entry;

    RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, mcastIdx=%d", unit, mcastIdx);
    RT_PARAM_CHK((mcastIdx >= (int32)mcast_idx_pool[unit].size_of_mcast_fwd_index) || mcastIdx < 0, RT_ERR_L2_MULTI_FWD_INDEX);

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
        if ((ret = table_write(unit, CYPRESS_MC_PMSKt, mcastIdx, (uint32 *)&mcast_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
            return ret;
        }

        if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
        {
            /* update multicast shadow */
            RTK_PORTMASK_RESET(mcast_fwdTable_shadow[mcastIdx].portmask);
        }
    }

    return RT_ERR_OK;
} /* _dal_cypress_l2_freeMcastIdx */

/* Function Name:
 *      _dal_cypress_l2_isMcastIdxUsed
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
static int32 _dal_cypress_l2_isMcastIdxUsed(uint32 unit, int32 mcastIdx)
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

} /* _dal_cypress_l2_isMcastIdxUsed */

int32
_dal_cypress_l2_lookupMissFloodPortMask_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask)
{
    int32   ret;
    uint32  reg_idx, field, idx;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(NULL == pFlood_portmask, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((type >= DLF_TYPE_END), RT_ERR_INPUT);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, pFlood_portmask=0x%x",
           unit, pFlood_portmask->bits[0]);

    switch (type)
    {
        case DLF_TYPE_UCAST:
            if (!ucst_dlf_pmsk_inited)
                return RT_ERR_L2_PMSK_NOT_INIT;
            reg_idx = CYPRESS_L2_FLD_PMSKr;
            field = CYPRESS_L2_UNKN_UC_FLD_PMSKf;
            break;
        case DLF_TYPE_BCAST:
            if (!bcst_dlf_pmsk_inited)
                return RT_ERR_L2_PMSK_NOT_INIT;
            reg_idx = CYPRESS_L2_FLD_PMSKr;
            field = CYPRESS_L2_BC_FLD_PMSKf;
            break;
        default:
            return RT_ERR_FAILED;
            break;
    }

    L2_SEM_LOCK(unit);
    if ((ret = reg_field_read(unit, reg_idx, field, &idx)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }


    if ((ret = table_field_set(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, &(pFlood_portmask->bits[0]), (uint32 *) &mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    if ((ret = table_write(unit, CYPRESS_MC_PMSKt, (uint32)idx, (uint32 *)&mcast_entry)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L2), "return failed ret value = %x",ret);
        return ret;
    }

    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
    {
        /* update multicast shadow */
        RTK_PORTMASK_ASSIGN(mcast_fwdTable_shadow[idx].portmask, *pFlood_portmask);
    }

    L2_SEM_UNLOCK(unit);

    return ret;
}

#if !defined(__MODEL_USER__)
/* Function Name:
 *      _dal_cypress_notifyHandler
 * Description:
 *      The notification event handler used to notify user's callback function.
 * Input:
 *      unit                - unit id
 *      pEventCollectArray  - L2 modification events array
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      None
 */
int32 _dal_cypress_notifyHandler(uint32 unit, rtk_l2_notifyEventCollectArray_t *pEventCollectArray)
{
    if (_notification_callback != NULL)
    {
        _notification_callback(unit, pEventCollectArray);
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      _dal_cypress_notifyRunOutHandler
 * Description:
 *      The notification buffer rurout event handler used to notify user's callback function.
 * Input:
 *      unit    - unit id
 *      type    - L2 modification events array
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      None
 */
int32 _dal_cypress_notifyRunOutHandler(uint32 unit, rtk_l2_notifyBufferType_t type)
{
    if (_bufferRunOut_callback != NULL)
    {
        _bufferRunOut_callback(unit, type);
    }
    return RT_ERR_OK;
}
#endif

/* Function Name:
 *      dal_cypress_l2_addrEntry_get
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
 *      1) The index valid range is from 0 to (L2 hash table size - 1)
 *         - 0 ~ (L2 hash table size - 1) entry in L2 hash table
 *      2) The output entry have 2 variables (valid and entry_type) and its detail data structure
 *         - valid: 1 mean the entry is valid; 0: invalid
 *         - entry_type: FLOW_TYPE_UNICAST, FLOW_TYPE_L2_MULTI, FLOW_TYPE_IP4_MULTI and FLOW_TYPE_IP6_MULTI
 *                       the field is ignored if valid field is 0.
 *         - detail data structure is ignored if valid is 0, and its field meanings are depended
 *           on the entry_type value.
 *      3) If pL2_entry->flags have enabled the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the
 *         pL2_entry->unicast.trk_gid value is valid trunk id value.
 */
int32
dal_cypress_l2_addrEntry_get(uint32 unit, uint32 index, rtk_l2_entry_t *pL2_entry)
{
    uint32  l2_tableSize;
    uint32  hashkey = 0, location = 0, isValid = 0;
    int32   ret;
    dal_cypress_l2_entry_t  l2_entry;
    multicast_index_entry_t mcast_entry;
    rtk_portmask_t      trunk_portmask;
    uint32              trk_gid;
    uint32              first_trunkMember;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, index=%d", unit, index);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* check index valid range */
    if ((ret = table_size_get(unit, CYPRESS_L2_UCt, &l2_tableSize)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    RT_PARAM_CHK(index >= l2_tableSize, RT_ERR_OUT_OF_RANGE);
    /* check pointer buffer of l2 table entry */
    RT_PARAM_CHK(NULL == pL2_entry, RT_ERR_NULL_POINTER);

    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));

    L2_SEM_LOCK(unit);

    /* L2 Hash Table Entry */
    hashkey = (index >> 2) & 0xFFF;
    location = index & BITMASK_2B;

    if (HAL_IS_ESCHIP(unit) && (pL2_entry->unicast.flags & RTK_L2_UCAST_FLAG_NEXTHOP))
    {
        l2_entry.unicast.nh = TRUE;
    }

    if ((ret = _dal_cypress_l2_getL2EntryfromHash(unit, hashkey, location, &l2_entry, &isValid)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }


    /* Transfer from dal_cypress_l2_entry_t to rtk_l2_entry_t */
    pL2_entry->valid = isValid;
    if (pL2_entry->valid)
    {
        pL2_entry->entry_type = l2_entry.entry_type;
        switch (pL2_entry->entry_type)
        {
            case FLOW_TYPE_UNICAST:
                pL2_entry->unicast.vid = l2_entry.unicast.fid;
                osal_memcpy(&pL2_entry->unicast.mac, &l2_entry.unicast.mac, sizeof(rtk_mac_t));
                pL2_entry->unicast.port = l2_entry.unicast.port;
                pL2_entry->unicast.flags = 0;
                pL2_entry->unicast.state = 0;
                pL2_entry->unicast.agg_vid = l2_entry.unicast.agg_vid;
                pL2_entry->unicast.vlan_target = l2_entry.unicast.vlan_target;
                pL2_entry->unicast.route_idx = l2_entry.unicast.route_idx;


                for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
                {
                    if (dal_cypress_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
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
                if(l2_entry.unicast.nh)
                    pL2_entry->unicast.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
                if(l2_entry.unicast.suspending)
                    pL2_entry->unicast.state |= RTK_L2_UCAST_STATE_SUSPEND;
                if(l2_entry.unicast.aging == 0)
                    pL2_entry->unicast.isAged = TRUE;
                else
                    pL2_entry->unicast.isAged = FALSE;
                break;

            case FLOW_TYPE_L2_MULTI:
                pL2_entry->l2mcast.rvid = l2_entry.l2mcast.rvid;
                osal_memcpy(&pL2_entry->l2mcast.mac, &l2_entry.l2mcast.mac, sizeof(rtk_mac_t));
                pL2_entry->l2mcast.fwdIndex = l2_entry.l2mcast.index;

                if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
                {
                    /* get multicast forward from shadow */
                    RTK_PORTMASK_ASSIGN(pL2_entry->l2mcast.portmask, mcast_fwdTable_shadow[l2_entry.l2mcast.index].portmask);
                }
                else
                {
                    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                    if ((ret = table_read(unit, CYPRESS_MC_PMSKt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                    {
                        L2_SEM_UNLOCK(unit);
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                        return ret;
                    }
                    if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, pL2_entry->l2mcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                    {
                        L2_SEM_UNLOCK(unit);
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                        return ret;
                    }
                }
                break;

            case FLOW_TYPE_IP4_MULTI:
                if (LOOKUP_ON_FVID_AND_MAC == ip4HashFmt[unit])
                {
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "It should not be existing an IP-multicast entry in FVID_AND_MAC mode");
                }
                else if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
                {
                    pL2_entry->ipmcast.rvid = l2_entry.ipmcast_ip_mc_sip.rvid;
                    pL2_entry->ipmcast.dip = l2_entry.ipmcast_ip_mc_sip.dip;
                    pL2_entry->ipmcast.sip = l2_entry.ipmcast_ip_mc_sip.sip;
                    pL2_entry->ipmcast.fwdIndex = l2_entry.ipmcast_ip_mc_sip.index;
                }
                else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
                {
                    pL2_entry->ipmcast.rvid = l2_entry.ipmcast_ip_mc.rvid;
                    pL2_entry->ipmcast.dip = l2_entry.ipmcast_ip_mc.dip;
                    pL2_entry->ipmcast.fwdIndex = l2_entry.ipmcast_ip_mc.index;
                }
                else
                {
                    L2_SEM_UNLOCK(unit);
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_FAILED;
                }

                if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
                {
                    /* get multicast forward from shadow */
                    RTK_PORTMASK_ASSIGN(pL2_entry->ipmcast.portmask, mcast_fwdTable_shadow[pL2_entry->ipmcast.fwdIndex].portmask);
                }
                else
                {
                    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                    if ((ret = table_read(unit, CYPRESS_MC_PMSKt, pL2_entry->ipmcast.fwdIndex, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                    {
                        L2_SEM_UNLOCK(unit);
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                        return ret;
                    }
                    if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, pL2_entry->ipmcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                    {
                        L2_SEM_UNLOCK(unit);
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                        return ret;
                    }
                }
                break;

            case FLOW_TYPE_IP6_MULTI:
                if (LOOKUP_ON_FVID_AND_MAC == ip6HashFmt[unit])
                {
                    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "It should not be existing an IP-multicast entry in FVID_AND_MAC mode");
                }
                else if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
                {
                    pL2_entry->ipmcast.rvid = l2_entry.ipmcast_ip_mc_sip.rvid;
                    pL2_entry->ipmcast.dip = l2_entry.ipmcast_ip_mc_sip.dip;
                    pL2_entry->ipmcast.sip = l2_entry.ipmcast_ip_mc_sip.sip;
                    pL2_entry->ipmcast.fwdIndex = l2_entry.ipmcast_ip_mc_sip.index;
                }
                else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
                {
                    pL2_entry->ipmcast.rvid = l2_entry.ipmcast_ip_mc.rvid;
                    pL2_entry->ipmcast.dip = l2_entry.ipmcast_ip_mc.dip;
                    pL2_entry->ipmcast.fwdIndex = l2_entry.ipmcast_ip_mc.index;
                }
                else
                {
                    L2_SEM_UNLOCK(unit);
                    RT_ERR(ret, (MOD_DAL|MOD_L2), "");
                    return RT_ERR_FAILED;
                }

                if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
                {
                    /* get multicast forward from shadow */
                    RTK_PORTMASK_ASSIGN(pL2_entry->ipmcast.portmask, mcast_fwdTable_shadow[pL2_entry->ipmcast.fwdIndex].portmask);
                }
                else
                {
                    osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                    if ((ret = table_read(unit, CYPRESS_MC_PMSKt, pL2_entry->ipmcast.fwdIndex, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                    {
                        L2_SEM_UNLOCK(unit);
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                        return ret;
                    }
                    if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, pL2_entry->ipmcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                    {
                        L2_SEM_UNLOCK(unit);
                        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                        return ret;
                    }
                }
                break;

            default:
                L2_SEM_UNLOCK(unit);
                return RT_ERR_FAILED;
        }
    }

    L2_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of dal_cypress_l2_addrEntry_get */

/* Function Name:
 *      dal_cypress_l2_conflictAddr_get
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
dal_cypress_l2_conflictAddr_get(
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
    dal_cypress_l2_entry_t  l2_entry;
    multicast_index_entry_t mcast_entry;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);
    /* check input parameters */
    RT_PARAM_CHK(NULL == pL2Addr, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pCfAddrList, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(0 == cfAddrList_size, RT_ERR_INPUT);
    RT_PARAM_CHK(NULL == pCf_retCnt, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pL2Addr->entry_type >= FLOW_TYPE_END, RT_ERR_OUT_OF_RANGE);

    osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
    /* calculate the hash index from pL2Addr input */
    if (pL2Addr->entry_type == FLOW_TYPE_UNICAST)
    {   /* FLOW_TYPE_UNICAST */
        l2_entry.entry_type = FLOW_TYPE_UNICAST;
        l2_entry.unicast.fid = pL2Addr->unicast.vid;
        osal_memcpy(&l2_entry.unicast.mac, &pL2Addr->unicast.mac, sizeof(rtk_mac_t));
    }
    else if (pL2Addr->entry_type == FLOW_TYPE_L2_MULTI)
    {   /* FLOW_TYPE_L2_MULTI */
        l2_entry.entry_type = FLOW_TYPE_L2_MULTI;
        l2_entry.l2mcast.rvid = pL2Addr->l2mcast.rvid;
        osal_memcpy(&l2_entry.l2mcast.mac, &pL2Addr->l2mcast.mac, sizeof(rtk_mac_t));
    }
    else if (pL2Addr->entry_type == FLOW_TYPE_IP4_MULTI)
    {   /* FLOW_TYPE_IP4_MULTI */
        l2_entry.entry_type = FLOW_TYPE_IP4_MULTI;
        if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
        {
            l2_entry.ipmcast_ip_mc_sip.dip = pL2Addr->ipmcast.dip;
            l2_entry.ipmcast_ip_mc_sip.sip = pL2Addr->ipmcast.sip;
        }
        else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
        {
            l2_entry.ipmcast_ip_mc.rvid = pL2Addr->ipmcast.rvid;
            l2_entry.ipmcast_ip_mc.dip = pL2Addr->ipmcast.dip;
        }
        else
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return RT_ERR_CHIP_NOT_SUPPORTED;
        }
    }
    else if (pL2Addr->entry_type == FLOW_TYPE_IP6_MULTI)
    {   /* FLOW_TYPE_IP6_MULTI */
        l2_entry.entry_type = FLOW_TYPE_IP6_MULTI;
        if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
        {
            l2_entry.ipmcast_ip_mc_sip.dip = pL2Addr->ipmcast.dip;
            l2_entry.ipmcast_ip_mc_sip.sip = pL2Addr->ipmcast.sip;
        }
        else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
        {
            l2_entry.ipmcast_ip_mc.rvid = pL2Addr->ipmcast.rvid;
            l2_entry.ipmcast_ip_mc.dip = pL2Addr->ipmcast.dip;
        }
        else
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
            return RT_ERR_CHIP_NOT_SUPPORTED;
        }
    }
    else
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    _dal_cypress_l2_entryToHashKey(unit, &l2_entry, &hash_key);
    cf_num = 0;
    for (hash_depth = 0; hash_depth < HAL_L2_HASHDEPTH(unit) && cf_num < cfAddrList_size; hash_depth++)
    {
        osal_memset(&l2_entry, 0, sizeof(dal_cypress_l2_entry_t));
        isValid = 0;
        if (_dal_cypress_l2_getL2EntryfromHash(unit, hash_key, hash_depth, &l2_entry, &isValid) != RT_ERR_OK)
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
                    osal_memcpy(&(pCfAddrList + cf_num)->unicast.mac, &l2_entry.unicast.mac, sizeof(rtk_mac_t));
                    (pCfAddrList + cf_num)->unicast.port = l2_entry.unicast.port;
                    (pCfAddrList + cf_num)->unicast.flags = 0;
                    (pCfAddrList + cf_num)->unicast.state = 0;

                    for (trk_gid = 0; trk_gid < HAL_MAX_NUM_OF_TRUNK(unit); trk_gid++)
                    {
                        if (dal_cypress_trunk_port_get(unit, trk_gid, &trunk_portmask) == RT_ERR_OK)
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
                    if(l2_entry.unicast.nh)
                        (pCfAddrList + cf_num)->unicast.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
                    if(l2_entry.unicast.suspending)
                        (pCfAddrList + cf_num)->unicast.state |= RTK_L2_UCAST_STATE_SUSPEND;
                    if(l2_entry.unicast.aging == 0)
                        (pCfAddrList + cf_num)->unicast.isAged = TRUE;
                    else
                        (pCfAddrList + cf_num)->unicast.isAged = FALSE;
                    break;

                case FLOW_TYPE_L2_MULTI:
                    (pCfAddrList + cf_num)->l2mcast.rvid = l2_entry.l2mcast.rvid;
                    osal_memcpy(&(pCfAddrList + cf_num)->l2mcast.mac, &l2_entry.l2mcast.mac, sizeof(rtk_mac_t));
                    (pCfAddrList + cf_num)->l2mcast.fwdIndex = l2_entry.l2mcast.index;
                    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
                    {
                        /* get multicast forward from shadow */
                        L2_SEM_LOCK(unit);
                        RTK_PORTMASK_ASSIGN((pCfAddrList + cf_num)->l2mcast.portmask, mcast_fwdTable_shadow[l2_entry.l2mcast.index].portmask);
                        L2_SEM_UNLOCK(unit);
                    }
                    else
                    {
                        osal_memset(&mcast_entry, 0, sizeof(mcast_entry));

                        if ((ret = table_read(unit, CYPRESS_MC_PMSKt, l2_entry.l2mcast.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                        {
                            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                            return ret;
                        }
                        if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, (pCfAddrList + cf_num)->l2mcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                        {
                            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                            return ret;
                        }
                    }
                    break;

                case FLOW_TYPE_IP4_MULTI:
                    if (LOOKUP_ON_DIP_AND_SIP == ip4HashFmt[unit])
                    {
                        (pCfAddrList + cf_num)->ipmcast.rvid = l2_entry.ipmcast_ip_mc_sip.rvid;
                        (pCfAddrList + cf_num)->ipmcast.dip = l2_entry.ipmcast_ip_mc_sip.dip;
                        (pCfAddrList + cf_num)->ipmcast.sip = l2_entry.ipmcast_ip_mc_sip.sip;
                        (pCfAddrList + cf_num)->ipmcast.fwdIndex = l2_entry.ipmcast_ip_mc_sip.index;
                    }
                    else if (LOOKUP_ON_DIP_AND_FVID == ip4HashFmt[unit])
                    {
                        (pCfAddrList + cf_num)->ipmcast.rvid = l2_entry.ipmcast_ip_mc.rvid;
                        (pCfAddrList + cf_num)->ipmcast.dip = l2_entry.ipmcast_ip_mc.dip;
                        (pCfAddrList + cf_num)->ipmcast.fwdIndex = l2_entry.ipmcast_ip_mc.index;
                    }

                    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
                    {
                        /* get multicast forward from shadow */
                        L2_SEM_LOCK(unit);
                        RTK_PORTMASK_ASSIGN((pCfAddrList + cf_num)->ipmcast.portmask, mcast_fwdTable_shadow[l2_entry.ipmcast_ip_mc.index].portmask);
                        L2_SEM_UNLOCK(unit);
                    }
                    else
                    {
                        osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                        if ((ret = table_read(unit, CYPRESS_MC_PMSKt, l2_entry.ipmcast_ip_mc.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                        {
                            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                            return ret;
                        }
                        if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, (pCfAddrList + cf_num)->ipmcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                        {
                            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                            return ret;
                        }
                    }
                    break;

                case FLOW_TYPE_IP6_MULTI:
                    if (LOOKUP_ON_DIP_AND_SIP == ip6HashFmt[unit])
                    {
                        (pCfAddrList + cf_num)->ipmcast.rvid = l2_entry.ipmcast_ip_mc_sip.rvid;
                        (pCfAddrList + cf_num)->ipmcast.dip = l2_entry.ipmcast_ip_mc_sip.dip;
                        (pCfAddrList + cf_num)->ipmcast.sip = l2_entry.ipmcast_ip_mc_sip.sip;
                        (pCfAddrList + cf_num)->ipmcast.fwdIndex = l2_entry.ipmcast_ip_mc_sip.index;
                    }
                    else if (LOOKUP_ON_DIP_AND_FVID == ip6HashFmt[unit])
                    {
                        (pCfAddrList + cf_num)->ipmcast.rvid = l2_entry.ipmcast_ip_mc.rvid;
                        (pCfAddrList + cf_num)->ipmcast.dip = l2_entry.ipmcast_ip_mc.dip;
                        (pCfAddrList + cf_num)->ipmcast.fwdIndex = l2_entry.ipmcast_ip_mc.index;
                    }

                    if (HAL_GET_CHIP_REV_ID(unit) < CHIP_REV_ID_C)
                    {
                        /* get multicast forward from shadow */
                        L2_SEM_LOCK(unit);
                        RTK_PORTMASK_ASSIGN((pCfAddrList + cf_num)->ipmcast.portmask, mcast_fwdTable_shadow[l2_entry.ipmcast_ip_mc.index].portmask);
                        L2_SEM_UNLOCK(unit);
                    }
                    else
                    {
                        osal_memset(&mcast_entry, 0, sizeof(mcast_entry));
                        if ((ret = table_read(unit, CYPRESS_MC_PMSKt, l2_entry.ipmcast_ip_mc.index, (uint32 *)&mcast_entry)) != RT_ERR_OK)
                        {
                            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                            return ret;
                        }
                        if ((ret = table_field_get(unit, CYPRESS_MC_PMSKt, CYPRESS_MC_PMSK_PMSKtf, (pCfAddrList + cf_num)->ipmcast.portmask.bits, (uint32 *) &mcast_entry)) != RT_ERR_OK)
                        {
                            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
                            return ret;
                        }
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
} /* end of dal_cypress_l2_conflictAddr_get */

#if defined(CONFIG_SDK_DRIVER_RTK_BACKWARD_COMPATIBLE_8328)
/*
 * For some feature in 8390 can full compatible the original 8328 API case,
 * SDK provide those backward compatible RTL8328 APIs for customer.
 * If customer only need the original 8328 feature, can use those API and nothing to change.
 * If customer want to use the enhance 8390 feature, need to call the new 8390 APIs.
 */

/* Function Name:
 *      dal_cypress_l2_portLimitLearningCntEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 */
int32
dal_cypress_l2_portLimitLearningCntEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d", unit, port);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    L2_SEM_LOCK(unit);
    (*pEnable) = pL2_info[unit]->limitLearningCnt_enable[port];
    L2_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pEnable=%d", *pEnable);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_portLimitLearningCntEnable_get */

/* Function Name:
 *      dal_cypress_l2_portLimitLearningCntEnable_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 */
int32
dal_cypress_l2_portLimitLearningCntEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  value = 0;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, port=%d, enable=%d",
           unit, port, enable);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((ENABLED != enable && DISABLED != enable), RT_ERR_INPUT);

    L2_SEM_LOCK(unit);
    if (ENABLED == enable)
        value = pL2_info[unit]->limitLearningCnt[port];
    else
    {
        if ((ret = reg_array_field_read(unit, CYPRESS_L2_PORT_LRN_CONSTRTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_CONSTRT_NUMf, &value)) != RT_ERR_OK)
        {
            L2_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_L2), "");
            return ret;
        }
        pL2_info[unit]->limitLearningCnt[port] = value;
        value = 0x7FFF;
    }

    /* programming value into CHIP*/
    if ((ret = reg_array_field_write(unit, CYPRESS_L2_PORT_LRN_CONSTRTr, port, REG_ARRAY_INDEX_NONE, CYPRESS_CONSTRT_NUMf, &value)) != RT_ERR_OK)
    {
        L2_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }
    pL2_info[unit]->limitLearningCnt_enable[port] = enable;
    L2_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_portLimitLearningCntEnable_set */

/* Function Name:
 *      dal_cypress_l2_ucastLookupMode_get
 * Description:
 *      Get lookup mode for unicast address.
 * Input:
 *      unit              - unit id
 * Output:
 *      pUcast_lookupMode - pointer to lookup mode for unicast address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      (1) Lookup mode for multicast address is as following
 *          - UC_LOOKUP_ON_VID
 *          - UC_LOOKUP_ON_FID
 *      (2) The function is backward compatible RTL8328 APIs.
 *          The mode is return from first exist vlan id.
 */
int32
dal_cypress_l2_ucastLookupMode_get(uint32 unit, rtk_l2_ucastLookupMode_t *pUcast_lookupMode)
{
    int32 ret;
    rtk_vlan_t vid;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    for (vid = RTK_VLAN_ID_MIN; vid <= RTK_VLAN_ID_MAX; vid++)
    {
        ret = dal_cypress_vlan_l2UcastLookupMode_get(unit, vid, pUcast_lookupMode);
        if (ret == RT_ERR_VLAN_ENTRY_NOT_FOUND)
            continue;
        if (ret == RT_ERR_OK)
            return RT_ERR_OK;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_ucastLookupMode_get */

/* Function Name:
 *      dal_cypress_l2_ucastLookupMode_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      (1) Lookup mode for multicast address is as following
 *          - UC_LOOKUP_ON_VID
 *          - UC_LOOKUP_ON_FID
 *      (2) The function is backward compatible RTL8328 APIs.
 *          The mode is applied to all exist vlan.
 */
int32
dal_cypress_l2_ucastLookupMode_set(uint32 unit, rtk_l2_ucastLookupMode_t ucast_lookupMode)
{
    int32 ret;
    rtk_vlan_t vid;

    RT_DBG(LOG_DEBUG, (MOD_L2|MOD_DAL), "unit=%d, ucast_lookupMode=%d", unit, ucast_lookupMode);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(ucast_lookupMode >= UC_LOOKUP_END, RT_ERR_INPUT);

    for (vid = RTK_VLAN_ID_MIN; vid <= RTK_VLAN_ID_MAX; vid++)
    {
        ret = dal_cypress_vlan_l2UcastLookupMode_set(unit, vid, ucast_lookupMode);
        if (ret == RT_ERR_VLAN_ENTRY_NOT_FOUND)
            continue;
        if (ret != RT_ERR_OK)
            return ret;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_ucastLookupMode_set */

/* Function Name:
 *      dal_cypress_l2_mcastLookupMode_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      (1) Lookup mode for multicast address is as following
 *          - MC_LOOKUP_ON_VID
 *          - MC_LOOKUP_ON_FID
 *      (2) The MC_LOOKUP_ON_FIXED_FID is not supported in 8390, fixed_fid is ignore
 *      (3) The function is backward compatible RTL8328 APIs.
 *          The mode is return from first exist vlan id.
 */
int32
dal_cypress_l2_mcastLookupMode_get(
    uint32                      unit,
    rtk_l2_mcastLookupMode_t    *pMcast_lookupMode,
    rtk_fid_t                   *pFixed_fid)
{
    int32 ret;
    rtk_vlan_t vid;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    for (vid = RTK_VLAN_ID_MIN; vid <= RTK_VLAN_ID_MAX; vid++)
    {
        ret = dal_cypress_vlan_l2McastLookupMode_get(unit, vid, pMcast_lookupMode);
        if (ret == RT_ERR_VLAN_ENTRY_NOT_FOUND)
            continue;
        if (ret == RT_ERR_OK)
            return RT_ERR_OK;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_mcastLookupMode_get */

/* Function Name:
 *      dal_cypress_l2_mcastLookupMode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_FID      - invalid fid
 * Note:
 *      (1) Lookup mode for multicast address is as following
 *          - MC_LOOKUP_ON_VID
 *          - MC_LOOKUP_ON_FID
 *      (2) The MC_LOOKUP_ON_FIXED_FID is not supported in 8390, fixed_fid is ignore
 *      (3) The function is backward compatible RTL8328 APIs.
 *          The mcast_lookupMode is applied to all exist vlan.
 */
int32
dal_cypress_l2_mcastLookupMode_set(
    uint32                      unit,
    rtk_l2_mcastLookupMode_t    mcast_lookupMode,
    rtk_fid_t                   fixed_fid)
{
    int32 ret;
    rtk_vlan_t vid;

    RT_DBG(LOG_DEBUG, (MOD_L2|MOD_DAL), "unit=%d, mcast_lookupMode=%d", unit, mcast_lookupMode);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(mcast_lookupMode >= MC_LOOKUP_ON_FIXED_FID, RT_ERR_INPUT);

    for (vid = RTK_VLAN_ID_MIN; vid <= RTK_VLAN_ID_MAX; vid++)
    {
        ret = dal_cypress_vlan_l2McastLookupMode_set(unit, vid, mcast_lookupMode);
        if (ret == RT_ERR_VLAN_ENTRY_NOT_FOUND)
            continue;
        if (ret != RT_ERR_OK)
            return ret;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_mcastLookupMode_set */

/* Function Name:
 *      dal_cypress_l2_lookupMissAction_get
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
 *      RT_ERR_INPUT            - invalid type of lookup miss
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      The function is backward compatible RTL8328 APIs (it is global based).
 *      If have the per-port requirement, please call the dal_cypress_l2_portLookupMissAction_get API.
 */
int32
dal_cypress_l2_lookupMissAction_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t *pAction)
{
    int32       ret;
    rtk_port_t  port;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= DLF_TYPE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    port = HAL_GET_MIN_PORT(unit);
    if ((ret = dal_cypress_l2_portLookupMissAction_get(unit, port, type, pAction)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    if (*pAction == ACTION_FORWARD)
        *pAction = ACTION_FLOOD_IN_VLAN;

    return RT_ERR_OK;
} /* end of dal_cypress_l2_lookupMissAction_get */

/* Function Name:
 *      dal_cypress_l2_lookupMissAction_set
 * Description:
 *      Set forwarding action when destination address lookup miss.
 * Input:
 *      unit    - unit id
 *      type    - type of lookup miss
 *      action  - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid type of lookup miss
 * Note:
 *      The function is backward compatible RTL8328 APIs (it is global based).
 *      If have the per-port requirement, please call the dal_cypress_l2_portLookupMissAction_set API.
 */
int32
dal_cypress_l2_lookupMissAction_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t action)
{
    int32       ret;
    rtk_port_t  port, max_port;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((type >= DLF_TYPE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((action >= ACTION_END), RT_ERR_INPUT);

    if (action == ACTION_FLOOD_IN_VLAN)
        action = ACTION_FORWARD;

    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port <= max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
            continue;

        if((ret = dal_cypress_l2_portLookupMissAction_set(unit, port, type, action)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "ret=%d", ret);
            return ret;
        }
    }

    return RT_ERR_OK;
}/* end of dal_cypress_l2_lookupMissAction_set */

/* Function Name:
 *      dal_cypress_l2_lookupMissPri_get
 * Description:
 *      Get priority of trapped packet when destination address lookup miss.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 *      The function recall the dal_cypress_trap_mgmtFramePri_get with MGMT_TYPE_UNKNOWN_DA option.
 */
int32
dal_cypress_l2_lookupMissPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    int32   ret;

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);

    if ((ret = dal_cypress_trap_mgmtFramePri_get(unit, MGMT_TYPE_UNKNOWN_DA, pPriority)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pPriority=%d",  *pPriority);

    return RT_ERR_OK;
} /* end of dal_cypress_l2_lookupMissPri_get */


/* Function Name:
 *      dal_cypress_l2_lookupMissPri_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PRIORITY - invalid priority value
 * Note:
 *      The function is backward compatible RTL8328 APIs.
 *      The function recall the dal_cypress_trap_mgmtFramePri_set with MGMT_TYPE_UNKNOWN_DA option.
 */
int32
dal_cypress_l2_lookupMissPri_set(uint32 unit, rtk_pri_t priority)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "unit=%d, priority=%d", unit, priority);

    /* check Init status */
    RT_INIT_CHK(l2_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((priority > RTK_DOT1P_PRIORITY_MAX), RT_ERR_PRIORITY);

    if ((ret = dal_cypress_trap_mgmtFramePri_set(unit, MGMT_TYPE_UNKNOWN_DA, priority)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_cypress_l2_lookupMissPri_set */

#endif

/* Function Name:
 *     dal_cypress_l2_getL2EntryfromHash_dump
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
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      We can not distinguish an entry between IP multicast in L2 mode and L2 multicast
 */
int32
dal_cypress_l2_getL2EntryfromHash_dump(uint32 unit, uint32 entry_idx, dal_cypress_l2_entry_t *pL2_entry, uint32 *pIsValid)
{
	uint32  hashkey, location;
    uint32  isValid;
    int32   ret;

	hashkey = entry_idx >> 2;
	location = entry_idx & BITMASK_2B;

	ret = _dal_cypress_l2_getL2EntryfromHash(unit, hashkey, location, pL2_entry, &isValid);
	*pIsValid = isValid;

	return ret;
} /* dal_cypress_l2_getL2EntryfromHash_dump */

