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
 * $Revision: 55230 $
 * $Date: 2015-01-26 21:56:17 +0800 (Mon, 26 Jan 2015) $
 *
 * Purpose : user main function for Linux
 *
 * Feature : 1) Init RTK Layer API
 *           2) Create SDK Main Thread
 *           3) Invoke SDK Diag Shell
 */

#define LINUX_NIC_USER_MODE_SAMPLE

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <ioal/ioal_init.h>
#include <drv/nic/probe.h>
#include <drv/nic/nic.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/print.h>
#include <osal/sem.h>
#include <rtk/init.h>
#include <rtk/l2.h>
#include <drv/swcore/chip.h>
#include <drv/swcore/l2notification.h>

#if defined(LINUX_NIC_USER_MODE_SAMPLE)
#include <stdlib.h>
#include <rtcore/user/rtcore_drv_usr.h>
#include <drv/nic/common.h>
#include <drv/nic/nic_rx.h>
#endif 

/* 
 * Symbol Definition     
 */


/* 
 * Data Declaration      
 */
#if defined(LINUX_NIC_USER_MODE_SAMPLE)
static uint32 pkt_buf_vir_addres;
static uint32 pkt_buf_stat[200];

extern uint32 l2Notify_buf_vir_addres;
#endif
/* 
 * Function Declaration  
 */
#if defined(LINUX_NIC_USER_MODE_SAMPLE)
static int32 rtnic_pkt_buf_init(uint32 unit);
#if defined(CONFIG_SDK_DRIVER_L2NTFY)
static int32 rt_l2Notify_buf_init(uint32 unit);
#endif
static int32 rtnic_pkt_alloc(uint32 unit, int32 size, uint32 flags, drv_nic_pkt_t **ppPacket);
static int32 rtnic_pkt_free(uint32 unit, drv_nic_pkt_t *pPacket);
static drv_nic_rx_t rtnic_rx_callback(uint32 unit, drv_nic_pkt_t *pPacket, void *pCookie);
static void rtnic_tx_callback(uint32 unit, drv_nic_pkt_t *pPacket, void *pCookie);
#endif
#if defined(CONFIG_SDK_APP_DIAG)
extern int diag_main(int argc, char** argv);
#endif

static osal_mutex_t         pkt_sem;
#define PKT_SEM_LOCK()    \
do {\
    if (osal_sem_mutex_take(pkt_sem, OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        osal_printf("semaphore lock failed\n");\
    }\
} while(0)
#define PKT_SEM_UNLOCK()   \
do {\
    if (osal_sem_mutex_give(pkt_sem) != RT_ERR_OK)\
    {\
         osal_printf("semaphore unlock failed\n");\
    }\
} while(0)

int32 app_notifyHandler_example(uint32 unit, rtk_l2_notifyEventCollectArray_t *pEventCollectArray)
{
    int32 num, i, counter = 0;
    rtk_l2_notifyEventEntry_t *pEventArray;
    static uint32 j = 0;
    

    num = pEventCollectArray->entryNum;
        
    pEventArray = pEventCollectArray->baseAddr + pEventCollectArray->startPtr;
    if (pEventCollectArray->wrap)
    {
        for (i = 0; i < ((RTL8390_L2NOTIFY_RING_SIZE * NBUF_SIZE) - pEventCollectArray->startPtr); i++)
        {
            printf("%s():%d   num:%d  No:%d  pos:%d type:%d  octet:%x:%x:%x:%x\n", __FUNCTION__, __LINE__, num, j, pEventCollectArray->startPtr + i, pEventArray[i].type, pEventArray[i].mac.octet[2], pEventArray[i].mac.octet[3], pEventArray[i].mac.octet[4], pEventArray[i].mac.octet[5]);
            counter++;
            j++;
        }
        
        pEventArray = pEventCollectArray->baseAddr;
        for (i = 0; i < (pEventCollectArray->entryNum - counter); i++)
        {
            printf("%s():%d   num:%d  No:%d  pos:%d   type:%d  octet:%x:%x:%x:%x\n", __FUNCTION__, __LINE__, num, j, i, pEventArray[i].type, pEventArray[i].mac.octet[2], pEventArray[i].mac.octet[3], pEventArray[i].mac.octet[4], pEventArray[i].mac.octet[5]);
            j++;
        }
    }
    else
    {
        for (i = 0; i < num; i++)
        {
            printf("%s():%d   num:%d  No:%d  pos:%d   type:%d  octet:%x:%x:%x:%x\n", __FUNCTION__, __LINE__, num, j, pEventCollectArray->startPtr + i, pEventArray[i].type, pEventArray[i].mac.octet[2], pEventArray[i].mac.octet[3], pEventArray[i].mac.octet[4], pEventArray[i].mac.octet[5]);
            j++;
        }
    }
    
    
    return 0;
}

int32 app_notifyRunoutHandler_example(uint32 unit, rtk_l2_notifyBufferType_t type)
{
    return 0;
}

/* Function Name:
 *      sdk_main
 * Description:
 *      sdk main function
 * Input:
 *      data - may hold pointer or word
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None.
 */
void sdk_main(void)
{
#if defined(CONFIG_SDK_APP_DIAG)

    /* Run diag shell */
    osal_printf("Start to run Diag Shell....\n\n"); 
    diag_main(0, NULL);
#else

    osal_printf("Please add your application here....\n\n");
#endif
} /* end of sdk_main */

/* Function Name:
 *      main
 * Description:
 *      the normal application entry point
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None.
 */
int main(int argc, char** argv)
{
    int32  ret;
    drv_nic_initCfg_t initcfg;
#if defined(CONFIG_SDK_DRIVER_L2NTFY)
    uint32 chip_id, chip_revId;
#endif
    
    initcfg.pkt_alloc = (drv_nic_pkt_alloc_f)rtnic_pkt_alloc;
    initcfg.pkt_free = (drv_nic_pkt_free_f)rtnic_pkt_free;
    initcfg.pkt_size = 1632;

    /* Init RTK Layer API */
    osal_printf("Init RTK Layer....");
   
    RT_ERR_CHK(rt_log_init(), ret);
    RT_ERR_CHK(ioal_init(DEFAULT_INIT_UNIT_ID), ret);
    RT_ERR_CHK(rtk_init(DEFAULT_INIT_UNIT_ID), ret);
     
    osal_printf("OK\n"); 

#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
    /* Probe the nic */
    RT_ERR_CHK(nic_probe(DEFAULT_INIT_UNIT_ID), ret);  
#endif
    
#if defined(LINUX_NIC_USER_MODE_SAMPLE)         
    rtnic_pkt_buf_init(DEFAULT_INIT_UNIT_ID);        
    
    drv_nic_init(DEFAULT_INIT_UNIT_ID, &initcfg);

    rtcore_usr_init();    
   
    nic_rx_thread_cb_register(DEFAULT_INIT_UNIT_ID, 0, rtnic_rx_callback, NULL, 0);

    /* L2 notification */
#if defined(CONFIG_SDK_DRIVER_L2NTFY)
    drv_swcore_cid_get(0, &chip_id, &chip_revId);
    if (((chip_id & FAMILY_ID_MASK) == RTL8390_FAMILY_ID) || ((chip_id & FAMILY_ID_MASK) == RTL8350_FAMILY_ID))
    rt_l2Notify_buf_init(DEFAULT_INIT_UNIT_ID);
#endif
#endif
        
#if defined(CONFIG_SDK_DRIVER_L2NTFY)
    if (((chip_id & FAMILY_ID_MASK) == RTL8390_FAMILY_ID) || ((chip_id & FAMILY_ID_MASK) == RTL8350_FAMILY_ID))
    rtk_l2_notificationEventHandler_register(DEFAULT_INIT_UNIT_ID, app_notifyHandler_example, app_notifyRunoutHandler_example);
#endif

    sdk_main();
    
    return RT_ERR_OK;
} /* end of main */

#if defined(LINUX_NIC_USER_MODE_SAMPLE)
/* Function Name:
 *      rtnic_pkt_buf_init
 * Description:
 *      packet allocation 
 * Input:
 *      unit     - unit id
 *      size     - alloc size
 *      flags    - alloc flags
 * Output:
 *      ppPacket - pointer buffer to the allocated packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */ 
static int32 rtnic_pkt_buf_init(uint32 unit)
{
    /* Get packet buffer start address that is mapped to user space */
    ioal_init_memRegion_get(unit, IOAL_MEM_DMA, &pkt_buf_vir_addres);
    
    /* Create mutex semaphore */
    pkt_sem = osal_sem_mutex_create();

    /* Init buffer state */
    osal_memset(pkt_buf_stat, 0, sizeof(pkt_buf_stat));
    
    return RT_ERR_OK;
}

#if defined(CONFIG_SDK_DRIVER_L2NTFY)
static int32 rt_l2Notify_buf_init(uint32 unit)
{
    /* Get packet buffer start address that is mapped to user space */
    ioal_init_memRegion_get(unit, IOAL_MEM_L2NOTIFY_DMA, &l2Notify_buf_vir_addres);
    

    drv_swcore_l2_notification_usr_init();
    
    return RT_ERR_OK;
}
#endif

/* Function Name:
 *      rtnic_pkt_alloc
 * Description:
 *      packet allocation 
 * Input:
 *      unit     - unit id
 *      size     - alloc size
 *      flags    - alloc flags
 * Output:
 *      ppPacket - pointer buffer to the allocated packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */ 
static int32 rtnic_pkt_alloc(uint32 unit, int32 size, uint32 flags, drv_nic_pkt_t **ppPacket)
{
    uint32 i;
    uint8 *pBuf;
    drv_nic_pkt_t *pPacket;

    PKT_SEM_LOCK();    
    pPacket = osal_alloc(sizeof(drv_nic_pkt_t));

    for(i = 0; i < 200; i++)
    {
        if(FALSE == pkt_buf_stat[i])
        {
            pkt_buf_stat[i] = TRUE;
            break;
        }
    }

    if (200 == i)
    {
        osal_free(pPacket);
        PKT_SEM_UNLOCK();
        osal_printf("rtnic_pkt_alloc: run out of memory\n");
        return RT_ERR_FAILED;
    }
    
    pBuf = (uint8 *)(pkt_buf_vir_addres + (size * i));

    osal_memset(pPacket, 0, sizeof(drv_nic_pkt_t));
    pPacket->head = (uint8 *)(pBuf);
    pPacket->data = (pPacket->head + 18);
    pPacket->tail = (pBuf + size);  /* No reserve any tail space */
    pPacket->end = pPacket->tail;
    pPacket->length = 0;
    pPacket->buf_id = (void *)i;
    pPacket->next = NULL;

    *ppPacket = pPacket;
    PKT_SEM_UNLOCK();
    return RT_ERR_OK;
} /* end of rtnic_pkt_alloc */

/* Function Name:
 *      rtnic_pkt_free
 * Description:
 *      free allocated packet 
 * Input:
 *      unit    - unit id
 *      pPacket - pointer buffer to the packet 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */ 
static int32 rtnic_pkt_free(uint32 unit, drv_nic_pkt_t *pPacket)
{
    uint32 id;
    
    id = (uint32)pPacket->buf_id;
    
    if (NULL == pPacket)
    {
        osal_printf("Error: pPacket is NULL at %s():%d\n", __FUNCTION__, __LINE__);
        return RT_ERR_FAILED;
    }

    if (FALSE == pkt_buf_stat[id])
    {
        osal_printf("Error: skb is NULL at %s():%d\n", __FUNCTION__, __LINE__);
        return RT_ERR_FAILED;
    }
    
    PKT_SEM_LOCK();  
    pkt_buf_stat[id] = FALSE;
    
    osal_free(pPacket);
    PKT_SEM_UNLOCK();
    return RT_ERR_OK;
} /* end of rtnic_pkt_free */

static drv_nic_rx_t rtnic_rx_callback(uint32 unit, drv_nic_pkt_t *pPacket, void *pCookie)
{
    int32  retval;
    drv_nic_pkt_t *pTx_packet;

    if (NULL == pPacket)
    {
        osal_printf("Error: pPacket is NULL at %s():%d\n", __FUNCTION__, __LINE__);
        goto _exit;
    }

    if ((retval = rtnic_pkt_alloc(unit, 1632, 0, &pTx_packet)) != RT_ERR_OK)
    {
        osal_printf("Error: Can't allocate TX packet buffer %s():%d\n", __FUNCTION__, __LINE__);
        goto _exit;
    }        


    osal_memcpy(pTx_packet->data, pPacket->data, pPacket->length);
    pTx_packet->tail = pTx_packet->data + pPacket->length;
    pTx_packet->length = pPacket->length;
    
    if ((retval = drv_nic_pkt_tx(0, pTx_packet, rtnic_tx_callback, (void *)NULL)) != RT_ERR_OK)
    {
        osal_printf("Error: drv_nic_pkt_tx TX packet failed %s():%d\n", __FUNCTION__, __LINE__);
        osal_printf("Directly free pTx_packet\n");
        if ((retval = rtnic_pkt_free(unit, pTx_packet)) != RT_ERR_OK)
        {
            osal_printf("Error: Can't free TX packet buffer %s():%d\n", __FUNCTION__, __LINE__);
            goto _exit;
        }         
        goto _exit;
    }         

    if ((retval = rtnic_pkt_free(unit, pPacket)) != RT_ERR_OK)
    {
        osal_printf("Error: Can't free RX packet buffer %s():%d\n", __FUNCTION__, __LINE__);
        goto _exit;
    }  

    return NIC_RX_HANDLED_OWNED;

_exit:
    return NIC_RX_NOT_HANDLED;
} /* end of rtnic_rx_callback */


static void rtnic_tx_callback(uint32 unit, drv_nic_pkt_t *pPacket, void *pCookie)
{
    if (NULL == pPacket)
    {
        osal_printf("Error: pPacket is NULL at %s():%d\n", __FUNCTION__, __LINE__);
        return;
    }
    
    rtnic_pkt_free(unit, pPacket);

    return;
} /* end of rtnic_tx_callback */
#endif
