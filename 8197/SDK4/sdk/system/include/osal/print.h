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
 * $Revision: 9758 $
 * $Date: 2010-05-21 17:11:12 +0800 (Fri, 21 May 2010) $
 *
 * Purpose : Definition those APIs interface for separating OS depend system call.
 *           Let the RTK SDK call the layer and become OS independent SDK package.
 *
 * Feature : 
 *
 */

#ifndef __OSAL_PRINT_H__
#define __OSAL_PRINT_H__

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/type.h>

/*
 * Symbol Definition
 */
#if defined(CONFIG_SDK_KERNEL_ECOS) || defined(CONFIG_SDK_KERNEL_LINUX)
  #define OS_PRINT
#endif

#if defined(OS_PRINT)
  #if defined(CONFIG_SDK_KERNEL_ECOS)
    #include <cyg/infra/diag.h>  
    #define osal_printf     diag_printf
  #elif defined(CONFIG_SDK_KERNEL_LINUX) && defined(__KERNEL__)
    #include <linux/kernel.h>  /* for Kernel Space */
    #define osal_printf     printk
  #elif defined(CONFIG_SDK_KERNEL_LINUX)
    #include <stdarg.h>
    #include <stdio.h>         /* for User Space */
    #define osal_printf     printf
  #endif

#define osal_sprintf    sprintf
#define osal_sscanf     sscanf
#define osal_strtol     strtol

#else

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      osal_printf
 * Description:
 *      print function
 * Input:
 *      pFmt, ... - input format with variable-length arguments.
 * Output:
 *      None
 * Return:
 *      Upon successful return, the function returns the number of 
 *      characters printed.
 * Note:
 *      None
 */
extern int32 osal_printf(uint8 *pFmt, ...);
#endif

#endif /* __OSAL_PRINT_H__ */

