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
 * $Revision: 6401 $
 * $Date: 2009-10-14 16:03:12 +0800 (Wed, 14 Oct 2009) $
 *
 * Purpose : SoC register definition
 *
 * Feature : Define the interrupt setting
 *
 */

#ifndef __INTERRUPT_SETTING_H__
#define __INTERRUPT_SETTING_H__

#include <soc/var/var_interrupt_setting.h>

#ifndef INTERRUPT_SETTING_DEFINIED
#define INTERRUPT_SETTING_DEFINIED
/*
 * Interrupt IRQ Assignments
 */
#define UART0_IRQ       31
#define UART1_IRQ       30
#define TC0_IRQ         29
#define TC1_IRQ         28
#define OCPTO_IRQ       27
#define HLXTO_IRQ       26
#define SLXTO_IRQ       25
#define NIC_IRQ         24
#define GPIO_ABCD_IRQ   23
#define GPIO_EFGH_IRQ   22
#define RTC_IRQ         21
#define SWCORE_IRQ      20

/*
 * Interrupt Routing Selection
 */
#define UART0_RS        2
#define UART1_RS        1
#define TC0_RS          5
#define TC1_RS          1
#define OCPTO_RS        1
#define HLXTO_RS        1
#define SLXTO_RS        1
#define NIC_RS          4
#define GPIO_ABCD_RS    4
#define GPIO_EFGH_RS    4
#define RTC_RS          4
#define SWCORE_RS       3

#endif /* end of INTERRUPT_SETTING_DEFINIED */

#endif /* end of __INTERRUPT_SETTING_H__ */

