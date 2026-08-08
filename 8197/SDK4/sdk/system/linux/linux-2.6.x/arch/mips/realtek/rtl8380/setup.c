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
 * $Revision: 21069 $
 * $Date: 2011-08-19 10:52:18 +0800 (Fri, 19 Aug 2011) $
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
static void __init serial_init(void);
static void __init rtl8380_time_init(void);
static void rtl8380_timer_ack(void);

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
const char *get_system_type(void)
{
    return "RTL8380";
}

void rtl8380_machine_restart(char *command)
{
	printk("System restart.\n");
    REG32(0xBB000040) = 0x1;    /* Reset Global Control Register */
}

void __init prom_init(void)
{
	prom_console_init();
	prom_meminit();
}

void __init plat_mem_setup(void)
{
    _machine_restart = rtl8380_machine_restart;

	/* Platform Specific Setup */
	serial_init();

    board_time_init = rtl8380_time_init;
    mips_timer_ack = rtl8380_timer_ack;
}

static void rtl8380_timer_ack(void)
{
    unsigned int original_data_intRd;
    unsigned int original_data_chipRd;
    unsigned int temp = 0;
    unsigned int temp_chip_info = 0;
    static unsigned int is_probe = 0, is_tc = 0;

    if (is_probe == 0)
    {
        //prom_printf("rtl8380_timer_ack API\n");
        original_data_intRd = REG32(0xBB000058);
        REG32(0xBB000058) = (original_data_intRd | 0x3);
        original_data_chipRd = REG32(0xBB0000D8);
        REG32(0xBB0000D8) = (original_data_chipRd | 0xA0000000);
        temp = REG32(0xBB0000D4);
        temp_chip_info = REG32(0xBB0000D8);
        REG32(0xBB0000D8) = original_data_chipRd;
        REG32(0xBB000058) = original_data_intRd;
        if ((temp_chip_info & 0xFFFF) == 0x0477)
            is_tc = 1;
        else
            is_tc = 0;
        is_probe = 1;
    }

    if (is_tc)
    {
        //prom_printf("rtl8380_timer_ack API: 8380 engineer sample chip\n");
        REG32(RTL8380ES_TCIR) |= RTL8380ES_TC0IP;
    }
    else
    {
        //prom_printf("rtl8380_timer_ack API: 8380 MP-chip\n");
        REG32(RTL8380MP_TC0INT) |= RTL8380MP_TCIP;
    }

    return;
}

static void __init rtl8380_time_init(void)
{
}

void __init plat_timer_setup(struct irqaction *irq)
{
    unsigned int original_data_intRd;
    unsigned int original_data_chipRd;
    unsigned int temp = 0;
    unsigned int temp_chip_info = 0;

    //prom_printf("plat_timer_setup API\n");
    original_data_intRd = REG32(0xBB000058);
    REG32(0xBB000058) = (original_data_intRd | 0x3);
    original_data_chipRd = REG32(0xBB0000D8);
    REG32(0xBB0000D8) = (original_data_chipRd | 0xA0000000);
    temp = REG32(0xBB0000D4);
    temp_chip_info = REG32(0xBB0000D8);
    REG32(0xBB0000D8) = original_data_chipRd;
    REG32(0xBB000058) = original_data_intRd;

	/* Setup Timer0 */

	/* Clear Timer IP status */
    if ((temp_chip_info & 0xFFFF) == 0x0477)
    {
        //prom_printf("plat_timer_setup API: 8380 engineer sample chip\n");
        if (REG32(RTL8380ES_TCIR) & RTL8380ES_TC0IP)
            REG32(RTL8380ES_TCIR) |= RTL8380ES_TC0IP;
    
        /* Here irq->handler is passed from outside */
        irq->handler = timer_interrupt;
        setup_irq(TC0_IRQ, irq);
    
        REG32(RTL8380ES_TCCNR) = 0; /* disable timer before setting CDBR */
        REG32(RTL8380ES_CDBR) = (DIVISOR) << RTL8380ES_DIVF_OFFSET;
        REG32(TC0DATA) = ((MHZ * 1000000)/(DIVISOR * HZ)) << RTL8380ES_TCD_OFFSET;
    
        REG32(RTL8380ES_TCCNR) = RTL8380ES_TC0EN | RTL8380ES_TC0MODE_TIMER;
        REG32(RTL8380ES_TCIR) = RTL8380ES_TC0IE;
    }
    else
    {
        //prom_printf("plat_timer_setup API: 8380 MP-chip\n");
        if (REG32(RTL8380MP_TC0INT) & RTL8380MP_TCIP)
            REG32(RTL8380MP_TC0INT) |= RTL8380MP_TCIP;
    
        /* Here irq->handler is passed from outside */
        irq->handler = timer_interrupt;
        setup_irq(TC0_IRQ, irq);
    
        REG32(RTL8380MP_TC0CTL) = 0; /* disable timer before setting CDBR */
        REG32(TC0DATA)= ((MHZ * 1000000)/(DIVISOR * HZ));
        REG32(RTL8380MP_TC0CTL) = RTL8380MP_TCEN | RTL8380MP_TCMODE_TIMER | DIVISOR ;
        REG32(RTL8380MP_TC0INT) = RTL8380MP_TCIE;        
    }

    return;
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
	s.custom_divisor = SYSCLK / (BAUDRATE * 16) - 1;

	/* Call early_serial_setup() here, to set up 8250 console driver */
	if (early_serial_setup(&s) != 0) {
		prom_printf("Serial setup failed!\n");
	}
#endif
}
