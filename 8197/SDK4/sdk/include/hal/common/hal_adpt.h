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
 * $Revision: 27279 $
 * $Date: 2012-03-15 18:55:27 +0800 (Thu, 15 Mar 2012) $
 *
 * Purpose : Definition the basic types in the SDK.
 *
 * Feature : type definition
 *
 */

#ifndef __HAL_COMMON_HAL_ADPT_H__
#define __HAL_COMMON_HAL_ADPT_H__

/*
 * Include Files
 */
#include <common/rt_autoconf.h>

/*
 * Symbol Definition
 */
#if defined(CONFIG_SDK_RTL8390)
#define MAX_PHY_PORT            (52)    /* number of physical ports */
#else
#define MAX_PHY_PORT            (28)    /* number of physical ports */
#endif
#define MAX_TRK_PORT            (8)     /* number of trunk ports */
#define MAX_CPU_PORT            (1)     /* number of cpu ports */
#define MAX_PHY_N_CPU_PORT      (MAX_PHY_PORT + MAX_CPU_PORT)
#define MAX_LOGIC_PORT          (MAX_PHY_N_CPU_PORT + MAX_TRK_PORT)

#define MAX_PORT                (MAX_PHY_PORT)

/*
 * Data Type Declaration
 */


/*
 * Macro Definition
 */

#endif /* __HAL_COMMON_HAL_ADPT_H__ */

