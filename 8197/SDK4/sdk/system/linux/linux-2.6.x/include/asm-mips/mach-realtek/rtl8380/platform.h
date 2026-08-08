/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Revision: 19821 $
 * $Date: 2011-07-19 16:14:06 +0800 (Tue, 19 Jul 2011) $
 *
 */

#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <common/rt_autoconf.h>

/*
 *  =============
 *  Utilty Macros
 *  =============
 */
#define REG8(reg)    (*(volatile unsigned char *)((unsigned int)reg))
#define REG32(reg)   (*(volatile unsigned int *)((unsigned int)reg))


/*
 *  ====================================
 *  Platform Configurable Common Options
 *  ====================================
 */

#define PROM_DEBUG      0

#ifdef CONFIG_SDK_FPGA_PLATFORM
#define MHZ             30
#else
#define MHZ             200
#endif
#define SYSCLK          MHZ * 1000 * 1000

#define BAUDRATE        115200 /* ex. 19200 or 38400 or 57600 or 115200 */ 
                               /* For Early Debug */

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
#define	SWCORE_IRQ	20
#define WDT_IP1_IRQ     19
#define WDT_IP2_IRQ     18

/*
 * Interrupt Routing Selection
 */
#define UART0_RS       2
#define UART1_RS       1
#define TC0_RS         5
#define TC1_RS         1
#define OCPTO_RS       1
#define HLXTO_RS       1
#define SLXTO_RS       1
#define NIC_RS         4
#define GPIO_ABCD_RS   4
#define GPIO_EFGH_RS   4
#define RTC_RS         4
#define	SWCORE_RS      3
#define WDT_IP1_RS     4
#define WDT_IP2_RS     5


#define DIVISOR         1000

#if DIVISOR > (1 << 16)
#error "Exceed the Maximum Value of DivFactor"
#endif

/*
 *  ==========================
 *  Platform Register Settings
 *  ==========================
 */

/*
 * Memory Controller
 */
#define MC_MCR          0xB8001000
   #define MC_MCR_VAL      0x00000000

#define MC_DCR          0xB8001004
   #define MC_DCR0_VAL     0x54480000

#define MC_DTCR         0xB8001008
   #define MC_DTCR_VAL     0xFFFF05C0

/*
 * Reset
 */
#define	RGCR			0xBB001E70


/*
 * UART
 */
#define UART0_BASE      0xB8002000
#define UART0_RBR       (UART0_BASE + 0x000)
#define UART0_THR       (UART0_BASE + 0x000)
#define UART0_DLL       (UART0_BASE + 0x000)
#define UART0_IER       (UART0_BASE + 0x004)
#define UART0_DLM       (UART0_BASE + 0x004)
#define UART0_IIR       (UART0_BASE + 0x008)
#define UART0_FCR       (UART0_BASE + 0x008)
#define UART0_LCR       (UART0_BASE + 0x00C)
#define UART0_MCR       (UART0_BASE + 0x010)
#define UART0_LSR       (UART0_BASE + 0x014)

#define BSP_UART1_BASE      0xB8002100
#define UART1_RBR       (BSP_UART1_BASE + 0x000)
#define UART1_THR       (BSP_UART1_BASE + 0x000)
#define UART1_DLL       (BSP_UART1_BASE + 0x000)
#define UART1_IER       (BSP_UART1_BASE + 0x004)
#define UART1_DLM       (BSP_UART1_BASE + 0x004)
#define UART1_IIR       (BSP_UART1_BASE + 0x008)
#define UART1_FCR       (BSP_UART1_BASE + 0x008)
   #define FCR_EN          0x01
   #define FCR_RXRST       0x02
   #define     RXRST             0x02
   #define FCR_TXRST       0x04
   #define     TXRST             0x04
   #define FCR_DMA         0x08
   #define FCR_RTRG        0xC0
   #define     CHAR_TRIGGER_01   0x00
   #define     CHAR_TRIGGER_04   0x40
   #define     CHAR_TRIGGER_08   0x80
   #define     CHAR_TRIGGER_14   0xC0
#define UART1_LCR       (BSP_UART1_BASE + 0x00C)
   #define LCR_WLN         0x03
   #define     CHAR_LEN_5        0x00
   #define     CHAR_LEN_6        0x01
   #define     CHAR_LEN_7        0x02
   #define     CHAR_LEN_8        0x03
   #define LCR_STB         0x04
   #define     ONE_STOP          0x00
   #define     TWO_STOP          0x04
   #define LCR_PEN         0x08
   #define     PARITY_ENABLE     0x01
   #define     PARITY_DISABLE    0x00
   #define LCR_EPS         0x30
   #define     PARITY_ODD        0x00
   #define     PARITY_EVEN       0x10
   #define     PARITY_MARK       0x20
   #define     PARITY_SPACE      0x30
   #define LCR_BRK         0x40
   #define LCR_DLAB        0x80
   #define     DLAB              0x80
#define UART1_MCR       (BSP_UART1_BASE + 0x010)
#define UART1_LSR       (BSP_UART1_BASE + 0x014)
   #define LSR_DR          0x01
   #define     RxCHAR_AVAIL      0x01
   #define LSR_OE          0x02
   #define LSR_PE          0x04
   #define LSR_FE          0x08
   #define LSR_BI          0x10
   #define LSR_THRE        0x20
   #define     TxCHAR_AVAIL      0x00
   #define     TxCHAR_EMPTY      0x20
   #define LSR_TEMT        0x40
   #define LSR_RFE         0x80


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
   #define WDT_IP2_IE	   (1 << 18)

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
#define IRR0_SETTING    ((UART0_RS  << 28) | \
                         (UART1_RS  << 24) | \
                         (TC0_RS    << 20) | \
                         (TC1_RS    << 16) | \
                         (OCPTO_RS  << 12) | \
                         (HLXTO_RS  << 8)  | \
                         (SLXTO_RS  << 4)  | \
                         (NIC_RS    << 0)    \
                        )

#define IRR1            0xB800300C
#define IRR1_SETTING    ((GPIO_ABCD_RS << 28) | \
                         (GPIO_EFGH_RS << 24) | \
                         (RTC_RS       << 20) | \
                         (SWCORE_RS    << 16)   \
                        )

#define IRR2            0xB8003010
#define IRR2_SETTING    0

#define IRR3            0xB8003014
#define IRR3_SETTING    0

/*
 * Timer/Counter for 8380 TC & MP chip
 */
#define TC_BASE         0xB8003100
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


#endif /* _PLATFORM_H */
