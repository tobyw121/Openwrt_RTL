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
 * $Revision: 6226 $
 * $Date: 2009-10-01 18:07:30 +0800 (Thu, 01 Oct 2009) $
 *
 */
 
#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <common/rt_autoconf.h>
#include <soc/soc.h>
#include <soc/type.h>

/*
 *  ====================================
 *  Platform Configurable Common Options
 *  ====================================
 */

#define PROM_DEBUG      0

#ifdef CONFIG_SDK_FPGA_PLATFORM
#define MHZ             20
#else
#define MHZ             200
#endif /*CONFIG_SDK_FPGA_PLATFORM*/
#define SYSCLK          (MHZ * 1000 * 1000)

#define BAUDRATE        38400  /* ex. 19200 or 38400 or 57600 or 115200 */ 

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


#define DIVISOR         10000

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
#define MC_MCR_VAL      0x00000000
#define MC_DCR0_VAL     0x54480000
#define MC_DTCR_VAL     0xFFFF05C0

/*
 * Interrupt Controller
 */
#define IRR0_SETTING    ((UART0_RS  << 28) | \
                         (UART1_RS  << 24) | \
                         (TC0_RS    << 20) | \
                         (TC1_RS    << 16) | \
                         (OCPTO_RS  << 12) | \
                         (HLXTO_RS  << 8)  | \
                         (SLXTO_RS  << 4)  | \
                         (NIC_RS    << 0)    \
                        )
#define IRR1_SETTING    ((GPIO_ABCD_RS << 28) | \
                         (GPIO_EFGH_RS << 24) | \
                         (RTC_RS       << 20) | \
                         (SWCORE_RS    << 16)   \
                        )
#define IRR2_SETTING    0
#define IRR3_SETTING    0

#endif /* _PLATFORM_H */
