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
 * $Revision: 40861 $
 * $Date: 2013-07-09 11:38:50 +0800 (Tue, 09 Jul 2013) $
 *
 * Purpose : SoC register definition
 *
 * Feature : Define the TIMER & WATCHDOG registers
 *
 */

#ifndef __TIMER_WDOG_REG_H__
#define __TIMER_WDOG_REG_H__

#include <soc/var/var_timer_wdog_reg.h>

#ifndef TIMER_WDOG_REG_DEFINIED
#define TIMER_WDOG_REG_DEFINIED

/*
 * Timer/Counter for 8389/8328
 */
#define TC_BASE         (0xB8003100)
#define TC0DATA         (TC_BASE + 0x00)
#define TC1DATA         (TC_BASE + 0x04)
   #define TCD_OFFSET        (8)
#define TC0CNT          (TC_BASE + 0x08)
#define TC1CNT          (TC_BASE + 0x0C)
#define TCCNR           (TC_BASE + 0x10)
   #define TC0EN             (1 << 31)
   #define TC0MODE_TIMER     (1 << 30)
   #define TC1EN             (1 << 29)
   #define TC1MODE_TIMER     (1 << 28)
#define TCIR            (TC_BASE + 0x14)
   #define TC0IE             (1 << 31)
   #define TC1IE             (1 << 30)
   #define TC0IP             (1 << 29)
   #define TC1IP             (1 << 28)
#define CDBR            (TC_BASE + 0x18)
   #define DIVF_OFFSET       (16)
#define WDTCNR          (TC_BASE + 0x1C)
   #define WDT_ENABLE_OFFSET (24)
   #define WDT_ENABLE_MASK   (0xFFU << WDT_ENABLE_OFFSET)
   #define WDT_ENABLE        (0xFF000000)
   #define WDT_DISABLE       (0xA5000000)
   #define WDT_CLR           (1 << 23)   
   #define WDT_SCALE_OFFSET  (21)
   #define WDT_SCALE_MASK    (0x3U << WDT_SCALE_OFFSET)
   #define WDT_SCALE_1       (0 << WDT_SCALE_OFFSET)
   #define WDT_SCALE_2       (1 << WDT_SCALE_OFFSET)
   #define WDT_SCALE_3       (2 << WDT_SCALE_OFFSET)
   #define WDT_SCALE_4       (3 << WDT_SCALE_OFFSET)
#define BSP_WDOG_IE          (0x20000000)
#define BSP_WDOG_NMI_EN      (0x00000040)


/*
 * Timer/Counter for 8390 ES & MP chip
 */
//#define TC_BASE         0xB8003100
//#define TC0DATA         (TC_BASE + 0x00)
#define RTL8390ES_TC1DATA         (TC_BASE + 0x04)
   #define RTL8390ES_TCD_OFFSET      8
#define RTL8390ES_TC0CNT          (TC_BASE + 0x08)
#define RTL8390ES_TC1CNT          (TC_BASE + 0x0C)
#define RTL8390ES_TCCNR           (TC_BASE + 0x10)
   #define RTL8390ES_TC0EN           (1 << 31)
   #define RTL8390ES_TC0MODE_TIMER   (1 << 30)
   #define RTL8390ES_TC1EN           (1 << 29)
   #define RTL8390ES_TC1MODE_TIMER   (1 << 28)
#define RTL8390ES_TCIR            (TC_BASE + 0x14)
   #define RTL8390ES_TC0IE           (1 << 31)
   #define RTL8390ES_TC1IE           (1 << 30)
   #define RTL8390ES_TC0IP           (1 << 29)
   #define RTL8390ES_TC1IP           (1 << 28)
#define RTL8390ES_CDBR            (TC_BASE + 0x18)
   #define RTL8390ES_DIVF_OFFSET     16
#define RTL8390ES_WDTCNR          (TC_BASE + 0x1C)

#define RTL8390MP_TC1DATA          (TC_BASE + 0x10)
#define RTL8390MP_TC0CNT           (TC_BASE + 0x04)
#define RTL8390MP_TC1CNT           (TC_BASE + 0x14)
#define RTL8390MP_TC0CTL           (TC_BASE + 0x08)
#define RTL8390MP_TC1CTL           (TC_BASE + 0x18)
   #define RTL8390MP_TCEN          (1 << 28)
   #define RTL8390MP_TCMODE_TIMER  (1 << 24)
   #define RTL8390MP_TCDIV_FACTOR  (0xFFFF << 0)
#define RTL8390MP_TC0INT           (TC_BASE + 0xC)
#define RTL8390MP_TC1INT           (TC_BASE + 0x1C)
   #define RTL8390MP_TCIE          (1 << 20)
   #define RTL8390MP_TCIP          (1 << 16)

#define DIVISOR_RTL8390        	   55
#define DIVISOR			           1000

/*
 * Timer/Counter for 8390 TC & MP chip
 */
#define RTL8390ES_WDTCNTR   (TC_BASE + 0x1C)
#define RTL8390MP_WDTCNTR   (TC_BASE + 0x50)
   #define RTL8390_WDT_KICK_OFFSET      (31)
   #define RTL8390_WDT_KICK_MASK        (1 << RTL8390_WDT_KICK_OFFSET)
#define RTL8390ES_WDTINTR   (TC_BASE + 0x20)
#define RTL8390MP_WDTINTR   (TC_BASE + 0x54)
   #define RTL8390_WDT_PH1_IP_OFFSET    (31)
   #define RTL8390_WDT_PH1_IP_MASK      (1 << RTL8390_WDT_PH1_IP_OFFSET)
   #define RTL8390_WDT_PH2_IP_OFFSET    (30)
   #define RTL8390_WDT_PH2_IP_MASK      (1 << RTL8390_WDT_PH2_IP_OFFSET)
#define RTL8390ES_WDTCTRL   (TC_BASE + 0x24)
#define RTL8390MP_WDTCTRL   (TC_BASE + 0x58)
   #define RTL8390_WDT_WDT_E_OFFSET     (31)
   #define RTL8390_WDT_WDT_E_MASK       (1 << RTL8390_WDT_WDT_E_OFFSET)
   #define RTL8390_WDT_WDT_CLK_SC_OFFSET    (29)
   #define RTL8390_WDT_WDT_CLK_SC_MASK      (0x3U << RTL8390_WDT_WDT_CLK_SC_OFFSET)
       #define RTL8390_WDT_SCALE_1          (0 << RTL8390_WDT_WDT_CLK_SC_OFFSET)
       #define RTL8390_WDT_SCALE_2          (1 << RTL8390_WDT_WDT_CLK_SC_OFFSET)
       #define RTL8390_WDT_SCALE_3          (2 << RTL8390_WDT_WDT_CLK_SC_OFFSET)
       #define RTL8390_WDT_SCALE_4          (3 << RTL8390_WDT_WDT_CLK_SC_OFFSET)
   #define RTL8390_WDT_PH1_TO_OFFSET    (22)
   #define RTL8390_WDT_PH1_TO_MASK      (0x1FU << RTL8390_WDT_PH1_TO_OFFSET)
   #define RTL8390_WDT_PH2_TO_OFFSET    (15)
   #define RTL8390_WDT_PH2_TO_MASK      (0x1FU << RTL8390_WDT_PH2_TO_OFFSET)
   #define RTL8390_WDT_RESET_MODE_OFFSET    (0)
   #define RTL8390_WDT_RESET_MODE_MASK      (0x3U << RTL8390_WDT_RESET_MODE_OFFSET)
       #define RTL8390_WDT_RESET_MODE_FULL_CHIP (0 << RTL8390_WDT_RESET_MODE_OFFSET)
       #define RTL8390_WDT_RESET_MODE_CPU_IPSEC (1 << RTL8390_WDT_RESET_MODE_OFFSET)
       #define RTL8390_WDT_RESET_MODE_SOFTWARE  (2 << RTL8390_WDT_RESET_MODE_OFFSET)

 /*
   * Timer/Counter for 8380 ES & MP chip
   */
#define TC0DATA         (TC_BASE + 0x00)
#define RTL8380ES_TC1DATA         (TC_BASE + 0x04)
   #define RTL8380ES_TCD_OFFSET      8
#define RTL8380ES_TC0CNT          (TC_BASE + 0x08)
#define RTL8380ES_TC1CNT          (TC_BASE + 0x0C)
#define RTL8380ES_TCCNR           (TC_BASE + 0x10)
   #define RTL8380ES_TC0EN           (1 << 31)
   #define RTL8380ES_TC0MODE_TIMER   (1 << 30)
   #define RTL8380ES_TC1EN           (1 << 29)
   #define RTL8380ES_TC1MODE_TIMER   (1 << 28)
#define RTL8380ES_TCIR            (TC_BASE + 0x14)
   #define RTL8380ES_TC0IE           (1 << 31)
   #define RTL8380ES_TC1IE           (1 << 30)
   #define RTL8380ES_TC0IP           (1 << 29)
   #define RTL8380ES_TC1IP           (1 << 28)
#define RTL8380ES_CDBR            (TC_BASE + 0x18)
   #define RTL8380ES_DIVF_OFFSET     16
#define RTL8380ES_WDTCNR          (TC_BASE + 0x1C)
	   
#define RTL8380MP_TC1DATA          (TC_BASE + 0x10)
#define RTL8380MP_TC0CNT           (TC_BASE + 0x04)
#define RTL8380MP_TC1CNT           (TC_BASE + 0x14)
#define RTL8380MP_TC0CTL           (TC_BASE + 0x08)
#define RTL8380MP_TC1CTL           (TC_BASE + 0x18)
   #define RTL8380MP_TCEN          (1 << 28)
   #define RTL8380MP_TCMODE_TIMER  (1 << 24)
   #define RTL8380MP_TCDIV_FACTOR  (0xFFFF << 0)
#define RTL8380MP_TC0INT           (TC_BASE + 0xC)
#define RTL8380MP_TC1INT           (TC_BASE + 0x1C)
   #define RTL8380MP_TCIE          (1 << 20)
   #define RTL8380MP_TCIP          (1 << 16)
#define RTL8380MP_WDTCNR          (TC_BASE + 0x50)

/*
 * watchDog for 8380 TC & MP chip
 */
#define RTL8380ES_WDTCNTR   (TC_BASE + 0x1C)
#define RTL8380MP_WDTCNTR   (TC_BASE + 0x50)
   #define RTL8380_WDT_KICK_OFFSET      (31)
   #define RTL8380_WDT_KICK_MASK        (1 << RTL8380_WDT_KICK_OFFSET)
#define RTL8380ES_WDTINTR   (TC_BASE + 0x20)
#define RTL8380MP_WDTINTR   (TC_BASE + 0x54)
   #define RTL8380_WDT_PH1_IP_OFFSET    (31)
   #define RTL8380_WDT_PH1_IP_MASK      (1 << RTL8380_WDT_PH1_IP_OFFSET)
   #define RTL8380_WDT_PH2_IP_OFFSET    (30)
   #define RTL8380_WDT_PH2_IP_MASK      (1 << RTL8380_WDT_PH2_IP_OFFSET)
#define RTL8380ES_WDTCTRL   (TC_BASE + 0x24)
#define RTL8380MP_WDTCTRL   (TC_BASE + 0x58)
   #define RTL8380_WDT_WDT_E_OFFSET     (31)
   #define RTL8380_WDT_WDT_E_MASK       (1 << RTL8380_WDT_WDT_E_OFFSET)
   #define RTL8380_WDT_WDT_CLK_SC_OFFSET    (29)
   #define RTL8380_WDT_WDT_CLK_SC_MASK      (0x3U << RTL8380_WDT_WDT_CLK_SC_OFFSET)
       #define RTL8380_WDT_SCALE_1          (0 << RTL8380_WDT_WDT_CLK_SC_OFFSET)
       #define RTL8380_WDT_SCALE_2          (1 << RTL8380_WDT_WDT_CLK_SC_OFFSET)
       #define RTL8380_WDT_SCALE_3          (2 << RTL8380_WDT_WDT_CLK_SC_OFFSET)
       #define RTL8380_WDT_SCALE_4          (3 << RTL8380_WDT_WDT_CLK_SC_OFFSET)
   #define RTL8380_WDT_PH1_TO_OFFSET    (22)
   #define RTL8380_WDT_PH1_TO_MASK      (0x1FU << RTL8380_WDT_PH1_TO_OFFSET)
   #define RTL8380_WDT_PH2_TO_OFFSET    (15)
   #define RTL8380_WDT_PH2_TO_MASK      (0x1FU << RTL8380_WDT_PH2_TO_OFFSET)
   #define RTL8380_WDT_RESET_MODE_OFFSET    (0)
   #define RTL8380_WDT_RESET_MODE_MASK      (0x3U << RTL8380_WDT_RESET_MODE_OFFSET)
       #define RTL8380_WDT_RESET_MODE_FULL_CHIP (0 << RTL8380_WDT_RESET_MODE_OFFSET)
       #define RTL8380_WDT_RESET_MODE_CPU_IPSEC (1 << RTL8380_WDT_RESET_MODE_OFFSET)
       #define RTL8380_WDT_RESET_MODE_SOFTWARE  (2 << RTL8380_WDT_RESET_MODE_OFFSET)



#endif /* end of TIMER_WDOG_REG_DEFINIED */


#endif /* end of __TIMER_WDOG_REG_H__ */

