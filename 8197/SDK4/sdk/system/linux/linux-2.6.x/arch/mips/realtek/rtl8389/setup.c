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
 * $Revision: 30016 $
 * $Date: 2012-06-18 14:06:18 +0800 (Mon, 18 Jun 2012) $
 *
 */

/*
 * Include Files
 */
#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/tty.h>
#include <linux/serial.h>
#include <linux/serial_core.h>

#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/reboot.h>
#include <asm/irq.h>
#include <asm/serial.h>
#include <asm/io.h>
#include <asm/time.h>

#include <prom.h>
#include <platform.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
static void __init serial_init(void);
static void __init rtl8389_time_init(void);
static void rtl8389_timer_ack(void);

const char *get_system_type(void)
{
    return "RTL8389";
}

void rtl8389_machine_restart(char *command)
{
    printk("Machine Restart ...\n");

    /* Reset system throuth GPIO A7 */
    REG32(0xB8003500) &= ~(0x1 << 31);
    REG32(0xB8003508) |=  (0x1 << 31);
    REG32(0xB800350C) |=  (0x1 << 31);
    REG32(0xB800350C) &= ~(0x1 << 31);

    /* Reset System through GPIO E7 pin */
    REG32(0xB800351C) &= ~(0x1 << 31);
    REG32(0xB8003524) |=  (0x1 << 31);
    REG32(0xB8003528) &= ~(0x1 << 31);

#if 0   /* only for next-cut chip */
    /* Reset System through register */
    REG32(0xBB001E70) = 0x00000000;
#endif
}

void __init prom_init(void)
{
	int argc = fw_arg0;
	char **arg = (char **)fw_arg1;
	int i;
	
    prom_console_init();
    prom_meminit();

	/* if user passes kernel args, ignore the default one */
	if (argc > 1)
		  arcs_cmdline[0] = '\0';
	
	/* arg[0] is "g", the rest is boot parameters */
	for (i = 1; i < argc; i++) {
	  	if (strlen(arcs_cmdline) + strlen(arg[i] + 1)
		  >= sizeof(arcs_cmdline))
		  break;
	   	strcat(arcs_cmdline, arg[i]);
   	    strcat(arcs_cmdline, " ");
	}	
}

void __init plat_mem_setup(void)
{
    _machine_restart = rtl8389_machine_restart;

    /* Platform Specific Setup */
    serial_init();

    board_time_init = rtl8389_time_init;
    mips_timer_ack = rtl8389_timer_ack;
}

static void rtl8389_timer_ack(void)
{
    REG32(TCIR) |= TC0IP;
}

static void __init rtl8389_time_init(void)
{
}

void __init plat_timer_setup(struct irqaction *irq)
{
    /* 
     * Setup Timer0 
     */

    /* Clear Timer IP status */
    if (REG32(TCIR) & TC0IP)
        REG32(TCIR) |= TC0IP;

    /* Here irq->handler is passed from outside */
    irq->handler = timer_interrupt;
    setup_irq(TC0_IRQ, irq);

    REG32(TCCNR) = 0; /* disable timer before setting CDBR */
    REG32(CDBR) = (DIVISOR) << DIVF_OFFSET;
    REG32(TC0DATA) = ((MHZ * 1000000) / (DIVISOR * HZ)) << TCD_OFFSET;

    REG32(TCCNR) = TC0EN | TC0MODE_TIMER;
    REG32(TCIR) = TC0IE;
}

static void __init serial_init(void)
{
#ifdef CONFIG_SERIAL_8250
    struct uart_port s;

    memset(&s, 0, sizeof(s));

    s.type = PORT_16550A;
    s.membase = (unsigned char *) UART0_BASE;
    s.irq = UART0_IRQ;
    s.uartclk = SYSCLK - BAUDRATE * 24;
    s.flags = UPF_SKIP_TEST | UPF_LOW_LATENCY | UPF_SPD_CUST;
    s.iotype = UPIO_MEM;
    s.regshift = 2;
    s.fifosize = 1;
    s.custom_divisor = (SYSCLK / (BAUDRATE * 16)) - 1;

    /* Call early_serial_setup() here, to set up 8250 console driver */
    if (early_serial_setup(&s) != 0) {
        prom_printf("Serial setup failed!\n");
    }
#endif
}

