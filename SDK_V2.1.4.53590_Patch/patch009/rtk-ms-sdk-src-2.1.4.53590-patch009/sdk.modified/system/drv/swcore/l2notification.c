/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 9258 $
 * $Date: 2010-04-29 16:37:07 +0800 (Thu, 29 Apr 2010) $
 *
 * Purpose : L2 notification initialization.
 *
 * Feature : L2 notification initialization
 *
 */

/*
 * Include Files
 */
#include <linux/version.h>
 
#include <common/debug/rt_log.h>
#include <common/error.h>
#include <drv/swcore/l2notification.h>
#include <drv/swcore/rtl8390.h>
#include <ioal/mem32.h>
#include <osal/cache.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/print.h>
#include <osal/time.h>
#include <osal/workqueue.h>

static l2_notifyNBuf_t *nBuf = NULL;
static uint32   *eventRing = NULL;
static uint32   _blockNum = 0;
static uint32   lastRingID = 0;
static rtk_l2_notifyEventCollectArray_t _atsArray;      /* For ATS test */
static uint32   totalEventCount = 0;
static uint32   runoutCnt = 0;
static uint32   notify_debug_flag;
static osal_work_struct_t workq;
atomic_t mem_usage = ATOMIC_INIT(0);
atomic_t mem_lock = ATOMIC_INIT(0);
static uint32   lockCnt, releaseCnt;


#define MAX_MEM_USAGE               300000

#define DEBUG_STOP_RELEASE_BIT      (1 << NTFY_DEBUG_STOP_RELEASE)    /* 0x00000001 */
#define DEBUG_SLOW_FREE             (1 << NTFY_DEBUG_SLOW_FREE)     /* 0x00000002 */
#define DEBUG_ATS_TEST              (1 << NTFY_DEBUG_ATS_TEST)      /* 0x00000004 */


typedef enum notification_debug_e
{   
    /* Rx debug flags */
    NTFY_DEBUG_STOP_RELEASE = 0,    /* 0 */
    NTFY_DEBUG_SLOW_FREE,           /* 1 */
    NTFY_DEBUG_ATS_TEST, 
} notification_debug_t;

typedef struct ntfy_collectArrayList_s
{
    uint32 entryNum;
    uint32 blockSize;
    osal_list_head_t list;
    l2_notifyNBuf_t *pNBufTmp;
    uint32 blockMemSize;
} ntfy_collectArrayList_t;


ntfy_collectArrayList_t             ntfyListHead;
dal_notification_callback_t         dal_notification_callback = NULL;
dal_notification_runout_callback_t  dal_notification_runout_callback = NULL;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27))
void drv_swcore_l2_notification_bhWQ(void *data);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))		
void drv_swcore_l2_notification_bhWQ(struct work_struct *data);
#endif


/* Function Name:
 *      drv_swcore_l2_notification_init
 * Description:
 *      Init L2 notification driver of the specified device.
 * Input:
 *      unit        - unit id
 *      blockNum    - number of block to store notification information, each block can store 10 notifications
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
drv_swcore_l2_notification_init(uint32 unit, uint32 blockNum)
{
    int32  ret;
    uint32 index, value;

    ioal_mem32_read(unit, RTL8390_DMA_IF_INTR_MSK_ADDR, &value);
    value |= INT_NTFY_DONE_MASK | INT_NTFY_BUF_RUN_OUT_MASK | INT_LOCAL_NTFY_BUF_RUN_OUT_MASK;
    ioal_mem32_write(unit, RTL8390_DMA_IF_INTR_MSK_ADDR, value);  

    nBuf = osal_alloc(blockNum * sizeof(l2_notifyNBuf_t));
    osal_memset(nBuf, 0, blockNum * sizeof(l2_notifyNBuf_t));
    if ((ret = osal_cache_memory_flush((uint32)nBuf, blockNum * sizeof(l2_notifyNBuf_t))) != RT_ERR_OK)
    {
        return ret;
    }
    nBuf = (l2_notifyNBuf_t *)UNCACHE(nBuf);
    
    eventRing = osal_alloc(blockNum * sizeof(uint32) + 64);
    osal_memset(eventRing, 0, blockNum * sizeof(uint32) + 64);
    if ((ret = osal_cache_memory_flush((uint32)eventRing, blockNum * sizeof(uint32) + 64)) != RT_ERR_OK)
    {
        return ret;
    }
    eventRing += 8;     /* Shift to avoid being effected by cache line */
    eventRing = (uint32 *)UNCACHE(eventRing);
    _blockNum = blockNum;
    for(index = 0; index < blockNum; index ++)
    {
        if(index == blockNum -1)
            eventRing[index] = (uint32)(&nBuf[index]) | (1 << 1) | L2_NOTIFY_BLOCK_OWNER_SWITCH;
        else
            eventRing[index] = (uint32)(&nBuf[index]) | L2_NOTIFY_BLOCK_OWNER_SWITCH;
    }

    value = (uint32)(eventRing);
    ioal_mem32_write(unit, RTL8390_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL_ADDR, value);


    lastRingID = 0;
    _atsArray.eventArray = NULL;

    ioal_mem32_field_write(unit, RTL8390_L2_NOTIFICATION_CTRL_ADDR, RTL8390_L2_NOTIFICATION_CTRL_BP_THR_OFFSET, RTL8390_L2_NOTIFICATION_CTRL_BP_THR_MASK, 100);
	
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27))	    
    OSAL_INIT_WORK(&workq, drv_swcore_l2_notification_bhWQ, &workq);
    OSAL_INIT_LIST_HEAD(&ntfyListHead.list);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))		
    OSAL_INIT_WORK(&workq, drv_swcore_l2_notification_bhWQ);
    OSAL_INIT_LIST_HEAD(&ntfyListHead.list);
#endif


    
    return RT_ERR_OK;
}

int32
drv_swcore_l2_notification_register(uint32 unit, dal_notification_callback_t cb, dal_notification_runout_callback_t runout_cb)
{
    dal_notification_callback = cb;
    dal_notification_runout_callback = runout_cb;
    return RT_ERR_OK;
}

int32
drv_swcore_l2_notification_unregister(uint32 unit)
{
    dal_notification_callback = NULL;
    dal_notification_runout_callback = NULL;
    return RT_ERR_OK;
}



#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27))
void
drv_swcore_l2_notification_bhWQ(void *data)
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
void
drv_swcore_l2_notification_bhWQ(struct work_struct *data)
#endif
{
    static uint32  flag = 0;
    uint32  i, j, val, num = 0, curMem_usage, value;
    ntfy_collectArrayList_t *pEntry, *n;
    rtk_l2_notifyEventCollectArray_t    *pUserCollection = NULL;
    rtk_l2_notifyEventEntry_t           *pEventArray = NULL;


    if (notify_debug_flag & DEBUG_SLOW_FREE)
    {
        if (!flag)
        {
            flag = 1;
            printk("%s():%d  slow mode\n", __FUNCTION__, __LINE__);
        }
        osal_time_udelay(100000);
    }


    
    osal_list_for_each_entry_safe(pEntry, n, &ntfyListHead.list, list)
    {   
        pUserCollection = osal_alloc(sizeof(rtk_l2_notifyEventCollectArray_t));
        if (pUserCollection == NULL)
        {
            RT_ERR(RT_ERR_FAILED, (MOD_L2), "L2 notificatio memory allocate failed");
            goto Err;
        }
        pEventArray = osal_alloc(pEntry->entryNum * sizeof(rtk_l2_notifyEventEntry_t));
        if (pEventArray == NULL)
        {
            RT_ERR(RT_ERR_FAILED, (MOD_L2), "L2 notificatio memory allocate failed");
            goto Err;
        }
        pUserCollection->entryNum = pEntry->entryNum;
        pUserCollection->eventArray = pEventArray;
        for (i = 0; i < pEntry->blockSize; i++)
        {
            for (j = 0; j < NBUF_SIZE; j++)
            {
                if (pEntry->pNBufTmp[i].event[j].valid == TRUE)
                {
                    pEventArray[num].fidVid = pEntry->pNBufTmp[i].event[j].fidVid;
                    pEventArray[num].mac.octet[0] = (uint8)((pEntry->pNBufTmp[i].event[j].mac >> 40) & 0xff);
                    pEventArray[num].mac.octet[1] = (uint8)((pEntry->pNBufTmp[i].event[j].mac >> 32) & 0xff);
                    pEventArray[num].mac.octet[2] = (uint8)((pEntry->pNBufTmp[i].event[j].mac >> 24) & 0xff);
                    pEventArray[num].mac.octet[3] = (uint8)((pEntry->pNBufTmp[i].event[j].mac >> 16) & 0xff);
                    pEventArray[num].mac.octet[4] = (uint8)((pEntry->pNBufTmp[i].event[j].mac >> 8) & 0xff);
                    pEventArray[num].mac.octet[5] = (uint8)(pEntry->pNBufTmp[i].event[j].mac & 0xff);
                    pEventArray[num].type = pEntry->pNBufTmp[i].event[j].type;
                    pEventArray[num].slp = pEntry->pNBufTmp[i].event[j].slp;
                    num++;
                }
            }
        }
        if (num != pEntry->entryNum)
            printk("%s():%d  Warring! Vacancy exists!  num:%d  i:%d\n", __FUNCTION__, __LINE__, num, pEntry->entryNum);
        
        num = 0;
        
        
        if (dal_notification_callback)
            dal_notification_callback(0, pUserCollection);
        
        val = pEntry->blockMemSize;
        osal_list_del(&pEntry->list);
        osal_free(pEntry->pNBufTmp);
        osal_free(pEntry);
        osal_free(pUserCollection);
        osal_free(pEventArray);
        curMem_usage = atomic_sub_return(val, &mem_usage);
        if (curMem_usage == 0 && atomic_read(&mem_lock))
        {
            ioal_mem32_read(0, RTL8390_DMA_IF_INTR_MSK_ADDR, &value);
            atomic_set(&mem_lock, 0);
            drv_swcore_l2_notification_isr_handler(0);
            value |= INT_NTFY_DONE_MASK | INT_NTFY_BUF_RUN_OUT_MASK | INT_LOCAL_NTFY_BUF_RUN_OUT_MASK;
            ioal_mem32_write(0, RTL8390_DMA_IF_INTR_MSK_ADDR, value);
            releaseCnt++;
        }
    }

    return;

Err:
    if (pUserCollection != NULL && pEventArray == NULL)
        osal_free(pUserCollection);
}

/* Function Name:
 *      drv_swcore_l2_notification_isr_handler
 * Description:
 *      L2 notification interrupt handle routine.
 * Input:
 *      isr_param - isr callback parameter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      OSAL_INT_HANDLED
 * Note:
 *      None
 */
int32
drv_swcore_l2_notification_isr_handler(void *isr_param)
{
    uint32  ringID, entryID;
    uint32  usedRingCnt = 0, ringCnt = 0;
    uint32  usedEventCnt = 0;
    ntfy_collectArrayList_t *pCllctAryList = NULL;
    uint32  blockMemSize, total, value, val;


    for(ringID = lastRingID; usedRingCnt < _blockNum; )
    {
        if((eventRing[ringID] & 1) != L2_NOTIFY_BLOCK_OWNER_CPU)
            break;

        for(entryID = 0; entryID < NBUF_SIZE; entryID++)
        {
            if(nBuf[ringID].event[entryID].valid == TRUE)
            {
                usedEventCnt++;
                totalEventCount++;
            }
        }
        ringID = (ringID + 1) % _blockNum;
        usedRingCnt++;
    }

    blockMemSize = sizeof(ntfy_collectArrayList_t) + usedRingCnt * sizeof(l2_notifyNBuf_t);

    if (atomic_read(&mem_lock))
        return RT_ERR_OK;
    if ((val = atomic_read(&mem_usage)) + blockMemSize > MAX_MEM_USAGE)
    {
        atomic_set(&mem_lock, 1);
        ioal_mem32_read(0, RTL8390_DMA_IF_INTR_MSK_ADDR, &value);
        value &= (~INT_NTFY_DONE_MASK) & (~INT_NTFY_BUF_RUN_OUT_MASK) & (~INT_LOCAL_NTFY_BUF_RUN_OUT_MASK);
        ioal_mem32_write(0, RTL8390_DMA_IF_INTR_MSK_ADDR, value);
        lockCnt++;
        return RT_ERR_OK;
    }
    
    if (usedEventCnt == 0)  /* Sometimes, the data is processed by previous interrupt */
    {
        return RT_ERR_OK;
	}

    
    pCllctAryList = osal_alloc(sizeof(ntfy_collectArrayList_t));
    if (pCllctAryList == NULL)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_L2), "L2 notificatio ISR memory allocate failed");
        goto Err;
    }
    pCllctAryList->entryNum = usedEventCnt;
    pCllctAryList->blockSize = usedRingCnt;
    pCllctAryList->pNBufTmp = osal_alloc(usedRingCnt * sizeof(l2_notifyNBuf_t));
    if (pCllctAryList->pNBufTmp == NULL)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_L2), "L2 notificatio ISR memory allocate failed");
        goto Err;
    }
    pCllctAryList->blockMemSize = blockMemSize;
    total = atomic_add_return(blockMemSize, &mem_usage);
    
    if (lastRingID + usedRingCnt > _blockNum)   /* If wrapped, memcpy by two part */
    {
        osal_memcpy(pCllctAryList->pNBufTmp, &nBuf[lastRingID], (_blockNum - lastRingID) * sizeof(l2_notifyNBuf_t));
        osal_memcpy(pCllctAryList->pNBufTmp + (_blockNum - lastRingID), &nBuf[0], (usedRingCnt - (_blockNum - lastRingID)) * sizeof(l2_notifyNBuf_t));
    }
    else
        osal_memcpy(pCllctAryList->pNBufTmp, &nBuf[lastRingID], usedRingCnt * sizeof(l2_notifyNBuf_t));

    
    for(ringID = lastRingID; ringCnt < usedRingCnt; )
    {
        if((eventRing[ringID] & 1) != L2_NOTIFY_BLOCK_OWNER_CPU)
            break;
        
        osal_memset(&nBuf[ringID], 0, sizeof(l2_notifyNBuf_t));
        if (notify_debug_flag & DEBUG_STOP_RELEASE_BIT)
        {
            /* Do nothing */
        }
        else
        {
            eventRing[ringID] |= L2_NOTIFY_BLOCK_OWNER_SWITCH;
        }
        
        lastRingID = (lastRingID + 1) % _blockNum;
        ringID = (ringID + 1) % _blockNum;  
        ringCnt++;
    }

    osal_list_add_tail(&pCllctAryList->list, &ntfyListHead.list);
    osal_schedule_work(&workq);
    
    return RT_ERR_OK;


Err:
    if (pCllctAryList != NULL && pCllctAryList->pNBufTmp == NULL)
        osal_free(pCllctAryList);
    return RT_ERR_OK;
} /* end of drv_swcore_l2_notification_isr_handler */

int32
drv_swcore_l2_notification_buf_runout_handler(void *isr_param)
{
    runoutCnt++;
    if (dal_notification_runout_callback)
        dal_notification_runout_callback(0, L2_NOTIFY_BUF_NBUF);
    return RT_ERR_OK;
}

int32
drv_swcore_l2_notification_localBuf_runout_handler(void *isr_param)
{
    if (dal_notification_runout_callback)
        dal_notification_runout_callback(0, L2_NOTIFY_BUF_ASIC_FIFO);
    return RT_ERR_OK;
}

int32
drv_swcore_l2_notification_debug_set(uint32 unit, uint32 flags)
{
    uint32  ringID;
    
    if ((flags & DEBUG_STOP_RELEASE_BIT) == 0 && (notify_debug_flag & DEBUG_STOP_RELEASE_BIT))
    {
        notify_debug_flag = flags;
        for(ringID = 0; ringID < _blockNum; ringID++)
        {
            eventRing[ringID] |= L2_NOTIFY_BLOCK_OWNER_SWITCH;
        }
        ioal_mem32_write(0, RTL8390_DMA_IF_INTR_STS_ADDR, INT_NTFY_BUF_RUN_OUT_MASK);
    }

    if (flags & DEBUG_ATS_TEST)
    {
        if (_atsArray.eventArray == NULL)
        {
            _atsArray.eventArray = osal_alloc(1024 * sizeof(rtk_l2_notifyEventEntry_t));
            _atsArray.entryNum = 0;
        }
    }
    else if ((flags & DEBUG_ATS_TEST) == 0 && (notify_debug_flag & DEBUG_ATS_TEST))
    {
        osal_free(_atsArray.eventArray);
        _atsArray.eventArray = NULL;
    }
    
    notify_debug_flag = flags;
    
    return RT_ERR_OK;
}

int32
drv_swcore_l2_notification_dump_counter(void)
{
    osal_printf("runoutCnt:%d\n", runoutCnt);
    osal_printf("totalEventCount:%d\n", totalEventCount);
    osal_printf("notify_debug_flag:%d\n", notify_debug_flag);
    osal_printf("mem_usage:%d  lockCnt:%d releaseCnt:%d\n", atomic_read(&mem_usage), lockCnt, releaseCnt);
    
    return RT_ERR_OK;
}

int32
drv_swcore_l2_notification_dump(void)
{
    uint32  ringID, entryID;
    uint64  mac;

    
    for(ringID = 0; ringID < _blockNum; ringID++)
    {
        osal_printf("ringID:%d   own:%s\n", ringID, (eventRing[ringID] & 1) == L2_NOTIFY_BLOCK_OWNER_CPU ? "CPU" : "SW");
        {
            for(entryID = 0; entryID < NBUF_SIZE; entryID++)
            {
                mac = nBuf[ringID].event[entryID].mac;
                osal_printf("%2d-%d type:%d fid:%d  mac:%x-%x-%x-%x-%x-%x  slp:%d  valid:%d\n", ringID, entryID, 
                                nBuf[ringID].event[entryID].type, nBuf[ringID].event[entryID].fidVid,
                                (uint8)((mac >> 40) & 0xff), (uint8)((mac >> 32) & 0xff), (uint8)((mac >> 24) & 0xff),
                                (uint8)((mac >> 16) & 0xff), (uint8)((mac >> 8) & 0xff), (uint8)(mac & 0xff), 
                                nBuf[ringID].event[entryID].slp, nBuf[ringID].event[entryID].valid);
                
            }
        }
    }

    osal_printf("Current ringID:%d \n", lastRingID);
    return RT_ERR_OK;
}

