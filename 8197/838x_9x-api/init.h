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
 * $Revision: 27473 $
 * $Date: 2012-03-23 18:07:16 +0800 (Fri, 23 Mar 2012) $
 *
 * Purpose : Definition of Init API
 *
 * Feature : Initialize All Layers of RTK Module
 *
 */

#ifndef __RTK_INIT_H__
#define __RTK_INIT_H__

/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Symbol Definition
 */
#define DEFAULT_INIT_UNIT_ID    0

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_init
 * Description:
 *      Initialize the specified device
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389, 8328,8390, 8380
 * Note:
 *      INIT must be initialized before using all of APIs in each modules
 */
extern int32
rtk_init(uint32 unit);

#endif /* __RTK_INIT_H__ */
