/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
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
 * $Revision: 10886 $
 * $Date: 2010-07-13 20:04:13 +0800 (Tue, 13 Jul 2010) $
 *
 */

/*
 * Include Files
 */
#include <linux/irq.h>
#include <linux/hardirq.h>
#include <asm/irq_cpu.h>
#include <prom.h>
#include <platform.h>

#include "chip.h"

/*
 * Symbol Definition
 */
#if defined(CONFIG_RTL8390_SERIES)	 
#define rtl8390_shutdown_irq      rtl8390_disable_irq
#define rtl8390_mask_and_ack_irq  rtl8390_disable_irq

static void rtl8390_enable_irq(unsigned int irq);
static void rtl8390_disable_irq(unsigned int irq);
static void rtl8390_end_irq(unsigned int irq);
static unsigned int rtl8390_startup_irq(unsigned int irq);
#endif

#if defined(CONFIG_RTL8380_SERIES)	
#define rtl8380_shutdown_irq      rtl8380_disable_irq
#define rtl8380_mask_and_ack_irq  rtl8380_disable_irq

static void rtl8380_enable_irq(unsigned int irq);
static void rtl8380_disable_irq(unsigned int irq);
static void rtl8380_end_irq(unsigned int irq);
static unsigned int rtl8380_startup_irq(unsigned int irq);
#endif

#if defined(CONFIG_RTL8328_SERIES)	
#define rtl8316s_shutdown_irq      rtl8316s_disable_irq
#define rtl8316s_mask_and_ack_irq  rtl8316s_disable_irq

static void rtl8316s_enable_irq(unsigned int irq);
static void rtl8316s_disable_irq(unsigned int irq);
static void rtl8316s_end_irq(unsigned int irq);
static unsigned int rtl8316s_startup_irq(unsigned int irq);
#endif
/*
 * Data Declaration
 */
spinlock_t irq_lock = SPIN_LOCK_UNLOCKED;

#if defined(CONFIG_RTL8390_SERIES)
static struct irq_chip irq_type_rtl8390 = {
   .typename = "RTL8390",
   .startup = rtl8390_startup_irq,
   .shutdown = rtl8390_shutdown_irq,
   .enable = rtl8390_enable_irq,
   .disable = rtl8390_disable_irq,
   .ack = rtl8390_mask_and_ack_irq,
   .end = rtl8390_end_irq,
};
#endif

#if defined(CONFIG_RTL8380_SERIES)
static struct irq_chip irq_type_rtl8380 = {
   .typename = "RTL8380",
   .startup = rtl8380_startup_irq,
   .shutdown = rtl8380_shutdown_irq,
   .enable = rtl8380_enable_irq,
   .disable = rtl8380_disable_irq,
   .ack = rtl8380_mask_and_ack_irq,
   .end = rtl8380_end_irq,
};
#endif

#if defined(CONFIG_RTL8328_SERIES)
static struct irq_chip irq_type_rtl8328 = {
   .typename = "RTL8316S",
   .startup = rtl8316s_startup_irq,
   .shutdown = rtl8316s_shutdown_irq,
   .enable = rtl8316s_enable_irq,
   .disable = rtl8316s_disable_irq,
   .ack = rtl8316s_mask_and_ack_irq,
   .end = rtl8316s_end_irq,
};
#endif

extern unsigned int bsp_chip_id, bsp_chip_rev_id;
extern unsigned int bsp_chip_family_id, bsp_chip_type;

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
#if defined(CONFIG_RTL8390_SERIES) 
static void rtl8390_enable_irq(unsigned int irq)
{
   unsigned long flags;

   spin_lock_irqsave(&irq_lock, flags);
   REG32(GIMR) = REG32(GIMR) | (1 << irq);
   spin_unlock_irqrestore(&irq_lock, flags);
}

static unsigned int rtl8390_startup_irq(unsigned int irq)
{
   rtl8390_enable_irq(irq);

   return 0;
}

static void rtl8390_disable_irq(unsigned int irq)
{
   unsigned long flags;

   spin_lock_irqsave(&irq_lock, flags);
   REG32(GIMR) = REG32(GIMR) & (~(1 << irq));
   spin_unlock_irqrestore(&irq_lock, flags);
}

static void rtl8390_end_irq(unsigned int irq)
{
   unsigned long flags;

   spin_lock_irqsave(&irq_lock, flags);
   REG32(GIMR) = REG32(GIMR) | (1 << irq);
   spin_unlock_irqrestore(&irq_lock, flags);
}
#endif /*End of CONFIG_RTL8390_SERIES*/

#if defined(CONFIG_RTL8380_SERIES) 
static void rtl8380_enable_irq(unsigned int irq)
{
   unsigned long flags;

   spin_lock_irqsave(&irq_lock, flags);
   REG32(GIMR) = REG32(GIMR) | (1 << irq);
   spin_unlock_irqrestore(&irq_lock, flags);
}

static unsigned int rtl8380_startup_irq(unsigned int irq)
{
   rtl8380_enable_irq(irq);

   return 0;
}

static void rtl8380_disable_irq(unsigned int irq)
{
   unsigned long flags;

   spin_lock_irqsave(&irq_lock, flags);
   REG32(GIMR) = REG32(GIMR) & (~(1 << irq));
   spin_unlock_irqrestore(&irq_lock, flags);
}

static void rtl8380_end_irq(unsigned int irq)
{
   unsigned long flags;

   spin_lock_irqsave(&irq_lock, flags);
   REG32(GIMR) = REG32(GIMR) | (1 << irq);
   spin_unlock_irqrestore(&irq_lock, flags);
}

static void rtl8380_wdt_phase1(void)
{
    uint32 tmp = 0;

	REG32(0xb8003154) = 0x80000000; /*WDT PH1 IP clear*/
    OTTO838x_FLASH_DISABLE_4BYTE_ADDR_MODE();
}	

#endif /*End of CONFIG_RTL8380_SERIES*/

#if defined(CONFIG_RTL8328_SERIES)
static void rtl8316s_enable_irq(unsigned int irq)
{
   unsigned long flags;

   spin_lock_irqsave(&irq_lock, flags);
   REG32(GIMR) = REG32(GIMR) | (1 << irq);
   spin_unlock_irqrestore(&irq_lock, flags);
}

static unsigned int rtl8316s_startup_irq(unsigned int irq)
{
   rtl8316s_enable_irq(irq);

   return 0;
}

static void rtl8316s_disable_irq(unsigned int irq)
{
   unsigned long flags;

   spin_lock_irqsave(&irq_lock, flags);
   REG32(GIMR) = REG32(GIMR) & (~(1 << irq));
   spin_unlock_irqrestore(&irq_lock, flags);
}

static void rtl8316s_end_irq(unsigned int irq)
{
   unsigned long flags;

   spin_lock_irqsave(&irq_lock, flags);
   REG32(GIMR) = REG32(GIMR) | (1 << irq);
   spin_unlock_irqrestore(&irq_lock, flags);
}

#endif /*End of CONFIG_RTL8328_SERIES*/


/*
 *   RTL83xx Interrupt Scheme
 *
 *   Source     EXT_INT   IRQ      CPU INT
 *   --------   -------   ------   -------
 *   UART0         31        31           2
 *   UART1         30        30           1
 *   TIMER0        29        29           5
 *   TIMER1         28       28           1
 *   OCPTO         27        27           1
 *   HLXTO         26        26           1
 *   SLXTO         25        25           1
 *   NIC             24        24          4
 *   GPIO_ABCD  23        23          4
 *   GPIO_EFGH  22        22           4     RTL8328 only
 *   RTC            21         21          4     RTL8328 only
 *   SWCORE     20         20           3
 */

void __init arch_init_irq(void)
{
   int i;
#if (defined(CONFIG_RTL8390_SERIES) || defined(CONFIG_RTL8380_SERIES) || defined(CONFIG_RTL8328_SERIES))   
   struct irq_chip * irq_callback_ptr = NULL;
#endif

#if defined(CONFIG_RTL8390_SERIES)
	if((bsp_chip_family_id == RTL8390_FAMILY_ID) || (bsp_chip_family_id == RTL8350_FAMILY_ID))
		irq_callback_ptr = &irq_type_rtl8390;
#endif
#if defined(CONFIG_RTL8380_SERIES)
	if((bsp_chip_family_id == RTL8380_FAMILY_ID) || (bsp_chip_family_id == RTL8330_FAMILY_ID))
		irq_callback_ptr = &irq_type_rtl8380;
#endif
#if defined(CONFIG_RTL8328_SERIES)
	if(bsp_chip_family_id == RTL8328_FAMILY_ID)
		irq_callback_ptr = &irq_type_rtl8328;
#endif

   /* Initialize for IRQ: 0~31 */
   for (i = 0; i < 32; i++) {
      irq_desc[i].chip = irq_callback_ptr;
   }

   /* Disable internal Count register */
   write_c0_cause(read_c0_cause() | (1 << 27));

   /* Clear internal timer interrupt */
   write_c0_compare(0);

   /* Enable all interrupt mask of CPU */
   write_c0_status(read_c0_status() | ST0_IM);

   /* Set GIMR, IRR */
   REG32(GIMR) = TC0_IE | UART0_IE;

   REG32(IRR0) = IRR0_SETTING;
   REG32(IRR1) = IRR1_SETTING;
   REG32(IRR2) = IRR2_SETTING;
   REG32(IRR3) = IRR3_SETTING;
}

asmlinkage void plat_irq_dispatch(void)
{
   unsigned int cpuint_ip = read_c0_cause() & read_c0_status() & ST0_IM;

   if (cpuint_ip & CAUSEF_IP6)
   {
      /* Timer 0 */
      do_IRQ(TC0_IRQ);
   }
   else if (cpuint_ip & CAUSEF_IP5)
   {
        /* For shared interrupts */
        unsigned int extint_ip = REG32(GIMR) & REG32(GISR);

        if (extint_ip & NIC_IP)
        {
            /* NIC */
            do_IRQ(NIC_IRQ);
        }
        else if (extint_ip & GPIO_ABCD_IP)
        {
            /* GPIO ABCD */
            do_IRQ(GPIO_ABCD_IRQ);
        }
#if defined(CONFIG_RTL8328_SERIES)
        /*robin*/
        else if (extint_ip & GPIO_EFGH_IP)
        {
            /* GPIO EFG */
            do_IRQ(GPIO_EFGH_IRQ);
        }
#endif
#if defined(CONFIG_RTL8380_SERIES)
		if((bsp_chip_family_id == RTL8380_FAMILY_ID) || (bsp_chip_family_id == RTL8330_FAMILY_ID))
		{
		    if (extint_ip & WDT_IP1_IP)
			{
			    rtl8380_wdt_phase1();
				/* WDT Phase-1 */
				do_IRQ(WDT_IP1_IRQ);
			}
		} 	  
#endif
   }
   else if (cpuint_ip & CAUSEF_IP4)
   {
      /* SWCORE */
      do_IRQ(SWCORE_IRQ);
   }
   else if (cpuint_ip & CAUSEF_IP3)
   {
      /* UART 0 */
      do_IRQ(UART0_IRQ);
   }
   else if (cpuint_ip & CAUSEF_IP2)
   {
      /* For shared interrupts */
      unsigned int extint_ip = REG32(GIMR) & REG32(GISR);

      if (extint_ip & TC1_IP)
      {
         do_IRQ(TC1_IRQ);
      }
      else if (extint_ip & UART1_IP)
      {
         do_IRQ(UART1_IRQ);
      }
   }
   else
   {
      prom_printf("Unknown_IRQ!\n");
      prom_printf("Current IP: 0x%08X!\n", cpuint_ip);
   }
}
