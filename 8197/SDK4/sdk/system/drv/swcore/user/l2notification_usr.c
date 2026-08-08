
#include <fcntl.h>
#include <sys/ioctl.h>
#include <osal/cache.h>
#include <common/error.h>
#include <common/debug/rt_log.h>
#include <drv/intr/intr.h>
#include <drv/swcore/l2notification.h>
#include <rtcore/rtcore.h>
#include <rtcore/user/rtcore_drv_usr.h>
#include <osal/lib.h>
#include <osal/thread.h>
#include <rtk/l2.h>
#include <ioal/ioal_init.h>


#define L2NOTIFY_ADDR_KRN2USR(krn_addr)    (((uint32)krn_addr - (uint32)KRNVIRT(RTL8390_L2NOTIFY_PHYS_BASE)) + l2Notify_dma_base)
#define L2NOTIFY_ADDR_USR2KRN(usr_addr)    (((uint32)usr_addr - l2Notify_dma_base) + (uint32)KRNVIRT(RTL8390_L2NOTIFY_PHYS_BASE))


extern uint32       l2Notify_dma_base;

dal_notification_callback_t         dal_notification_callback = NULL;
dal_notification_runout_callback_t  dal_notification_runout_callback = NULL;

uint32 l2Notify_buf_vir_addres;

int32 
drv_swcore_l2_notification_usr_init(void)
{
    int32   ret = RT_ERR_FAILED;

    osal_memset((void*)(l2Notify_buf_vir_addres), 0, sizeof(rtk_l2_notifyEventEntry_t) * RTL8390_L2NOTIFY_RING_SIZE * NBUF_SIZE);
    if ((ret = osal_cache_memory_flush((uint32)(L2NOTIFY_ADDR_USR2KRN(l2Notify_buf_vir_addres)), sizeof(rtk_l2_notifyEventEntry_t) * RTL8390_L2NOTIFY_RING_SIZE * NBUF_SIZE)) != RT_ERR_OK)
    {
        return ret;
    }
    return RT_ERR_OK;
} /* end of rtcore_usr_init */

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

int32 
drv_swcore_l2_notification_usr_handler(int32 num, int32 curPos)
{
    int32   ret = RT_ERR_FAILED;
    rtk_l2_notifyEventEntry_t           *pEventArray;
    rtk_l2_notifyEventCollectArray_t    userCollection;

    
    pEventArray = (rtk_l2_notifyEventEntry_t*)l2Notify_buf_vir_addres;

    userCollection.baseAddr = pEventArray;
    userCollection.entryNum = num;
    if (curPos >= num)
    {
        userCollection.wrap = 0;
        userCollection.startPtr = (curPos - num);
    }
    else
    {
        userCollection.wrap = 1;
        userCollection.startPtr = (RTL8390_L2NOTIFY_RING_SIZE * NBUF_SIZE) - (num - curPos);
    }
    if (dal_notification_callback)
            dal_notification_callback(0, &userCollection);

    if ((ret = osal_cache_memory_flush((uint32)(L2NOTIFY_ADDR_USR2KRN(l2Notify_buf_vir_addres)), sizeof(rtk_l2_notifyEventEntry_t) * RTL8390_L2NOTIFY_RING_SIZE * NBUF_SIZE)) != RT_ERR_OK)
    {
        return ret;
    }
    
    return RT_ERR_OK;
}

int32 
drv_swcore_l2_notification_buf_runout_handler(void *isr_param)
{
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

