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
 
#include <common/error.h>
#include <drv/swcore/l2notification.h>
#include <drv/swcore/rtl8390.h>
#include <ioal/mem32.h>
#include <ioal/ioal_init.h>
#include <osal/cache.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/print.h>
#include <osal/workqueue.h>

static l2_notifyNBuf_t *nBuf = NULL;
static uint32   *eventRing = NULL;
static uint32   _blockNum = 0;
static uint32   lastRingID = 0;
static uint32   runoutCnt = 0;
static uint32   notify_debug_flag;

uint32   totalEventCount = 0;

#define NBUF_SIZE   10

#define DEBUG_STOP_RELEASE_BIT    (1 << NTFY_DEBUG_STOP_RELEASE)    /* 0x00000001 */
#define DEBUG_ATS_TEST              (1 << NTFY_DEBUG_ATS_TEST)        /* 0x00000002 */

#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
extern uint32   l2NotifyEventCnt;
extern uint32   curPos;
#endif


typedef enum notification_debug_e
{   
    /* Rx debug flags */
    NTFY_DEBUG_STOP_RELEASE = 0,   /* 0 */
    NTFY_DEBUG_ATS_TEST, 
} notification_debug_t;

typedef struct ntfy_collectArrayList_s
{
    uint32 entryNum;
    uint32 blockSize;
    osal_list_head_t list;
    l2_notifyNBuf_t *pNBufTmp;
} ntfy_collectArrayList_t;




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
    eventRing += 8;
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

    ioal_mem32_field_write(unit, RTL8390_L2_NOTIFICATION_CTRL_ADDR, RTL8390_L2_NOTIFICATION_CTRL_BP_THR_OFFSET, RTL8390_L2_NOTIFICATION_CTRL_BP_THR_MASK, 100);
	
    
    return RT_ERR_OK;
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
    rtk_l2_notifyEventEntry_t           *pEventArray;
    const   uint32 eventArraySize = RTL8390_L2NOTIFY_RING_SIZE * NBUF_SIZE;

    
    pEventArray = (rtk_l2_notifyEventEntry_t*)UNCACHE(KRNVIRT(RTL8390_L2NOTIFY_PHYS_BASE));


    for(ringID = lastRingID; usedRingCnt < _blockNum; )
    {
        if((eventRing[ringID] & 1) != L2_NOTIFY_BLOCK_OWNER_CPU)
            break;

        for(entryID = 0; entryID < NBUF_SIZE; entryID++)
        {
            if(nBuf[ringID].event[entryID].valid == TRUE)
            {
                pEventArray[curPos].fidVid = nBuf[ringID].event[entryID].fidVid;
                pEventArray[curPos].mac.octet[0] = (uint8)((nBuf[ringID].event[entryID].mac >> 40) & 0xff);
                pEventArray[curPos].mac.octet[1] = (uint8)((nBuf[ringID].event[entryID].mac >> 32) & 0xff);
                pEventArray[curPos].mac.octet[2] = (uint8)((nBuf[ringID].event[entryID].mac >> 24) & 0xff);
                pEventArray[curPos].mac.octet[3] = (uint8)((nBuf[ringID].event[entryID].mac >> 16) & 0xff);
                pEventArray[curPos].mac.octet[4] = (uint8)((nBuf[ringID].event[entryID].mac >> 8) & 0xff);
                pEventArray[curPos].mac.octet[5] = (uint8)(nBuf[ringID].event[entryID].mac & 0xff);
                pEventArray[curPos].type = nBuf[ringID].event[entryID].type;
                pEventArray[curPos].slp = nBuf[ringID].event[entryID].slp;
                
                usedEventCnt++;
                totalEventCount++;
                curPos = (curPos + 1) % eventArraySize;
            }
        }
        ringID = (ringID + 1) % _blockNum;
        usedRingCnt++;
    }

    /* Pass to user space */
    l2NotifyEventCnt = usedEventCnt;
    
    if (usedEventCnt == 0)  /* Sometimes, the data is processed by previous interrupt */
    {
        return RT_ERR_OK;
	}

    
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

    return RT_ERR_OK;
} /* end of drv_swcore_l2_notification_isr_handler */

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

    
    notify_debug_flag = flags;
    
    return RT_ERR_OK;
}

int32
drv_swcore_l2_notification_dump_counter(void)
{
    osal_printf("runoutCnt:%d\n", runoutCnt);
    osal_printf("totalEventCount:%d\n", totalEventCount);
    osal_printf("notify_debug_flag:%d\n", notify_debug_flag);
    
    runoutCnt = 0;
    totalEventCount = 0;
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

