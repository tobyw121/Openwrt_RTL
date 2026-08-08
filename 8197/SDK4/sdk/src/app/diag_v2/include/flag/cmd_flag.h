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
 * $Revision: 27701 $
 * $Date: 2012-04-02 11:10:51 +0800 (Mon, 02 Apr 2012) $
 *
 * Purpose : include command flag file
 *
 * Feature : None
 *
 */

#ifndef __CMD_FLAG_H__
#define __CMD_FLAG_H__


/*
 * Include Files
 */
#ifdef CONFIG_SDK_RTL8389
#include "cmd_flag_rtl8389.h"
#endif  /* CONFIG_SDK_RTL8389 */

#ifdef CONFIG_SDK_RTL8328
#include "cmd_flag_rtl8328.h"
#endif  /* CONFIG_SDK_RTL8328 */

#ifdef CONFIG_SDK_RTL8390
#include "cmd_flag_rtl8390.h"
#endif  /* CONFIG_SDK_RTL8390 */

#ifdef CONFIG_SDK_RTL8380
#include "cmd_flag_rtl8380.h"
#endif  /* CONFIG_SDK_RTL8380 */

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */

/* Module Name : diag */

#endif /* __CMD_FLAG_H__ */
