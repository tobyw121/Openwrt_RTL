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
 * $Revision: 37253 $
 * $Date: 2013-02-27 10:31:44 +0800 (Wed, 27 Feb 2013) $
 *
 * Purpose : SoC register definition
 *
 * Feature : Define the INTERRUPT registers
 *
 */

#ifndef __INTERRUPT_REG_H__
#define __INTERRUPT_REG_H__

#include <soc/var/var_interrupt_reg.h>

#ifndef INTERRUPT_REG_DEFINIED
#define INTERRUPT_REG_DEFINIED

/*
 * Interrupt Controller
 */
#define GIMR            0xB8003000
   #define UART0_IE        (1 << 31)
   #define UART1_IE        (1 << 30)
   #define TC0_IE          (1 << 29)
   #define TC1_IE          (1 << 28)
   #define OCPTO_IE        (1 << 27)
   #define HLXTO_IE        (1 << 26)
   #define SLXTO_IE        (1 << 25)
   #define NIC_IE          (1 << 24)
   #define GPIO_ABCD_IE    (1 << 23)
   #define GPIO_EFGH_IE    (1 << 22)
   #define RTC_IE          (1 << 21)
   #define WDT_IP1_IE	   (1 << 19)
   #define WDT_IP2_IE      (1 << 18)

#define GISR            0xB8003004
   #define UART0_IP        (1 << 31)
   #define UART1_IP        (1 << 30)
   #define TC0_IP          (1 << 29)
   #define TC1_IP          (1 << 28)
   #define OCPTO_IP        (1 << 27)
   #define HLXTO_IP        (1 << 26)
   #define SLXTO_IP        (1 << 25)
   #define NIC_IP          (1 << 24)
   #define GPIO_ABCD_IP    (1 << 23)
   #define GPIO_EFGH_IP    (1 << 22)
   #define RTC_IP          (1 << 21)
   #define WDT_IP1_IP	   (1 << 19)
   #define WDT_IP2_IP	   (1 << 18)   

#define IRR0            0xB8003008

#define IRR1            0xB800300C

#define IRR2            0xB8003010

#define IRR3            0xB8003014


#endif /* end of INTERRUPT_REG_DEFINIED */

#endif /* end of __INTERRUPT_REG_H__ */

