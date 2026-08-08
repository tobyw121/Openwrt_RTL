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

/*
 * Symbol Definition
 */
#define rtl8380_shutdown_irq      rtl8380_disable_irq
#define rtl8380_mask_and_ack_irq  rtl8380_disable_irq

static void rtl8380_enable_irq(unsigned int irq);
static void rtl8380_disable_irq(unsigned int irq);
static void rtl8380_end_irq(unsigned int irq);
static unsigned int rtl8380_startup_irq(unsigned int irq);

/*
 * Data Declaration
 */
spinlock_t irq_lock = SPIN_LOCK_UNLOCKED;

static struct irq_chip irq_type = {
   .typename = "RTL8380",
   .startup = rtl8380_startup_irq,
   .shutdown = rtl8380_shutdown_irq,
   .enable = rtl8380_enable_irq,
   .disable = rtl8380_disable_irq,
   .ack = rtl8380_mask_and_ack_irq,
   .end = rtl8380_end_irq,
};

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
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

/*
 *   RTL8380 Interrupt Scheme (Subject to change)
 *
 *   Source     EXT_INT   IRQ      CPU INT
 *   --------   -------   ------   -------
 *   UART0      31        31       2
 *   UART1      30        30       1
 *   TIMER0     29        29       5
 *   TIMER1     28        28       1
 *   OCPTO      27        27       1
 *   HLXTO      26        26       1
 *   SLXTO      25        25       1
 *   NIC        24        24       4
 *   GPIO_ABCD  23        23       4
 *   GPIO_EFGH  22        22       4
 *   RTC        21        21       4
 */

void __init arch_init_irq(void)
{
   int i;

   /* Initialize for IRQ: 0~31 */
   for (i = 0; i < 32; i++) {
      irq_desc[i].chip = &irq_type;
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
#if 0
        /*robin*/
        else if (extint_ip & GPIO_EFGH_IP)
        {
            /* GPIO EFG */
            do_IRQ(GPIO_EFGH_IRQ);
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
