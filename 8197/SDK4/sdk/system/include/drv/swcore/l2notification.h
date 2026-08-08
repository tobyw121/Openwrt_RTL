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

#ifndef __DRV_SWCORE_L2NOTIFICATION_H__
#define __DRV_SWCORE_L2NOTIFICATION_H__

#include <dal/cypress/dal_cypress_l2.h>

#define INT_NTFY_DONE_OFFSET                    (22)
#define INT_NTFY_DONE_MASK                      (0x1U << INT_NTFY_DONE_OFFSET)
#define INT_NTFY_BUF_RUN_OUT_OFFSET             (21)
#define INT_NTFY_BUF_RUN_OUT_MASK               (0x1U << INT_NTFY_BUF_RUN_OUT_OFFSET)
#define INT_LOCAL_NTFY_BUF_RUN_OUT_OFFSET       (20)
#define INT_LOCAL_NTFY_BUF_RUN_OUT_MASK         (0x1U << INT_LOCAL_NTFY_BUF_RUN_OUT_OFFSET)


#define NBUF_SIZE   10


typedef struct l2_notifyEvent_s
{
    uint32 type:2;
    uint32 fidVid:12;
    uint64 mac:48;
    uint32 slp:6;
    uint32 valid:1;
    uint32 reserved:27;
} __attribute__ ((aligned(1), packed)) l2_notifyEvent_t;

typedef struct l2_notifyNBuf_s
{
    l2_notifyEvent_t event[10];
} l2_notifyNBuf_t;

typedef enum rtk_l2_notifyRingOwner_e
{
    L2_NOTIFY_BLOCK_OWNER_CPU = 0,
    L2_NOTIFY_BLOCK_OWNER_SWITCH,
    L2_NOTIFY_BLOCK_OWNER_END
} rtk_l2_notifyRingOwner_t;

typedef int32 (*dal_notification_callback_t)(uint32 unit, rtk_l2_notifyEventCollectArray_t *pEventCollectArray);
typedef int32 (*dal_notification_runout_callback_t)(uint32 unit, rtk_l2_notifyBufferType_t type);


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
extern int32
drv_swcore_l2_notification_init(uint32 unit, uint32 blockNum);

extern int32
drv_swcore_l2_notification_register(uint32 unit, dal_notification_callback_t cb, dal_notification_runout_callback_t runout_cb);

extern int32
drv_swcore_l2_notification_unregister(uint32 unit);

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
 * Note:
 *      None
 */
extern int32
drv_swcore_l2_notification_isr_handler(void *isr_param);

extern int32
drv_swcore_l2_notification_buf_runout_handler(void *isr_param);

extern int32
drv_swcore_l2_notification_localBuf_runout_handler(void *isr_param);

extern int32
drv_swcore_l2_notification_debug_set(uint32 unit, uint32 flags);

extern int32
drv_swcore_l2_notification_dump_counter(void);

extern int32
drv_swcore_l2_notification_dump(void);

#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
extern int32
drv_swcore_l2_notification_usr_init(void);

extern int32 
drv_swcore_l2_notification_usr_handler(int32 num, int32 curPos);
#endif

#endif  /* __DRV_SWCORE_L2NOTIFICATION_H__ */

