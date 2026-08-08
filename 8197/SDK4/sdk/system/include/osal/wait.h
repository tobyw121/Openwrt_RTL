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
 * $Revision: 40460 $
 * $Date: 2013-06-25 09:51:12 +0800 (Tue, 25 Jun 2013) $
 *
 * Purpose : Definition those APIs interface for separating OS depend system call.
 *           Let the RTK SDK call the layer and become OS independent SDK package.
 *
 * Feature : Wait relative API
 *
 */

#ifndef __OSAL_WAIT_H__
#define __OSAL_WAIT_H__

/*
 * Include Files
 */
#include <common/type.h>
#include <linux/sched.h>

/*
 * Symbol Definition
 */
#define osal_wait_queue_head_t  wait_queue_head_t

/*
 * Function Declaration
 */
#define osal_init_waitqueue_head(x)                     init_waitqueue_head(x)
#define osal_wait_event_interruptible(x, y)             wait_event_interruptible(x, y)
#define osal_wait_event_interruptible_timeout(x, y, z)  wait_event_interruptible_timeout(x, y, z)
#define osal_wake_up_interruptible(x)       wake_up_interruptible(x)
#define osal_wake_up(x)                     wake_up(x)

#endif /* __OSAL_WAIT_H__ */
