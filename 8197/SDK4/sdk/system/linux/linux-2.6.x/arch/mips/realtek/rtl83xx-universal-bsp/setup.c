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
 * $Revision: 15674 $
 * $Date: 2011-02-09 14:27:13 +0800 (Wed, 09 Feb 2011) $
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

#include "chip.h"


/*
 * Symbol Definition
 */
static void __init serial_init(void);

#if defined(CONFIG_RTL8390_SERIES)
static void __init rtl8390_time_init(void);
static void rtl8390_timer_ack(void);
static void __init rtl8390_timer_setup(struct irqaction *irq);
#endif
#if defined(CONFIG_RTL8380_SERIES)
static void __init rtl8380_time_init(void);
static void rtl8380_timer_ack(void);
static void __init rtl8380_timer_setup(struct irqaction *irq);
#endif
#if defined(CONFIG_RTL8328_SERIES)
static void __init rtl8328_time_init(void);
static void rtl8328_timer_ack(void);
static void __init rtl8328_timer_setup(struct irqaction *irq);
#endif

/*
 * Data Declaration
 */

unsigned int bsp_chip_id, bsp_chip_rev_id;
unsigned int bsp_chip_family_id, bsp_chip_type;

extern int bsp_drv_swcore_cid_get(unsigned int unit, unsigned int *pCid, unsigned int *pCrevid);


/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
const char *get_system_type(void)
{
	if(bsp_chip_family_id == RTL8390_FAMILY_ID)
		return "RTL8390";
	if(bsp_chip_family_id == RTL8380_FAMILY_ID)
		return "RTL8380";
	if(bsp_chip_family_id == RTL8328_FAMILY_ID)
		return "RTL8328";

	return "Unknow chip";
}

void (* hook_restart_func)(void) = NULL;

void rtk_hook_restart_function(void (*func)(void))
{
	hook_restart_func = func;
	return;
}


#if defined(CONFIG_RTL8390_SERIES)
void rtl8390_machine_restart(char *command)
{
	if(hook_restart_func != NULL)
	{
		hook_restart_func();
	}
	printk("System restart.\n");
	REG32(0xBB000014) = 0xFFFFFFFF;    /* Reset whole chip */

}
#endif

#if defined(CONFIG_RTL8380_SERIES)
void rtl8380_machine_restart(char *command)
{
	uint32 tmp = 0;
	
	if(hook_restart_func != NULL)
	{
		hook_restart_func();
	}
	
	OTTO838x_FLASH_DISABLE_4BYTE_ADDR_MODE();	
	
	printk("System restart.\n");
	REG32(0xBB000040) = 0x1;    /* Reset Global Control Register */

}
#endif
#if defined(CONFIG_RTL8328_SERIES)
void rtl8328_machine_restart(char *command)
{
	if(hook_restart_func != NULL)
	{
		hook_restart_func();
	}
	printk("System restart.\n");
	REG32(0xBB020004) = 0x0;    /* Reset Global Control1 Register */

}
#endif

void __init prom_init(void)
{
	int ret;

	int argc = fw_arg0;
	char **arg = (char **)fw_arg1;
	int i;

	bsp_chip_id = 0;
	bsp_chip_rev_id = 0;
	bsp_chip_family_id = 0;
	bsp_chip_type = 0;

	ret = bsp_drv_swcore_cid_get((unsigned int)0, (unsigned int *)&bsp_chip_id, (unsigned int *)&bsp_chip_rev_id);
	if(ret == -1)
	{
		printk("\nbsp_init(), RTK Switch chip is not found!!!\n");
	}else
	{
#if defined(CONFIG_RTL8390_SERIES)
	if((bsp_chip_id & FAMILY_ID_MASK) == RTL8390_FAMILY_ID)
		bsp_chip_family_id = RTL8390_FAMILY_ID;
	if((bsp_chip_id & FAMILY_ID_MASK) == RTL8350_FAMILY_ID)
		bsp_chip_family_id = RTL8350_FAMILY_ID;
#endif
#if defined(CONFIG_RTL8380_SERIES)
	if((bsp_chip_id & FAMILY_ID_MASK) == RTL8380_FAMILY_ID)
		bsp_chip_family_id = RTL8380_FAMILY_ID;
	if((bsp_chip_id & FAMILY_ID_MASK) == RTL8330_FAMILY_ID)
		bsp_chip_family_id = RTL8330_FAMILY_ID;
#endif
#if defined(CONFIG_RTL8328_SERIES)
	if((bsp_chip_id & RTL8328_FAMILY_ID) == RTL8328_FAMILY_ID)
		bsp_chip_family_id = RTL8328_FAMILY_ID;
#endif
	}

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
#if defined(CONFIG_RTL8390_SERIES)
	if((bsp_chip_family_id == RTL8390_FAMILY_ID) || (bsp_chip_family_id == RTL8350_FAMILY_ID))
	{
		_machine_restart = rtl8390_machine_restart;

		/* Platform Specific Setup */
		serial_init();

		board_time_init = rtl8390_time_init;
		mips_timer_ack = rtl8390_timer_ack;
	}
#endif
#if defined(CONFIG_RTL8380_SERIES)
	if((bsp_chip_family_id == RTL8380_FAMILY_ID) || (bsp_chip_family_id == RTL8330_FAMILY_ID))
	{
		_machine_restart = rtl8380_machine_restart;

		/* Platform Specific Setup */
		serial_init();

		board_time_init = rtl8380_time_init;
		mips_timer_ack = rtl8380_timer_ack;
	}
#endif
#if defined(CONFIG_RTL8328_SERIES)
	if(bsp_chip_family_id == RTL8328_FAMILY_ID)
	{
		_machine_restart = rtl8328_machine_restart;

		/* Platform Specific Setup */
		serial_init();

		board_time_init = rtl8328_time_init;
		mips_timer_ack = rtl8328_timer_ack;
	}
#endif


}

#if defined(CONFIG_RTL8390_SERIES)
static void rtl8390_timer_ack(void)
{
    unsigned int model_info;
    unsigned int chip_info;
    static unsigned int is_probe = 0, is_tc = 0;

    if (is_probe == 0)
    {
        model_info = REG32(0xBB000FF0);
        REG32(0xBB000FF4) = 0xA0000000;
        chip_info = REG32(0xBB000FF4);
        REG32(0xBB000FF4) = 0;
        if ((chip_info & 0xFFFF) == 0x0399)
            is_tc = 1;
        else
            is_tc = 0;
        is_probe = 1;
    }

    if (is_tc)
    {
        REG32(RTL8390ES_TCIR) |= RTL8390ES_TC0IP;
    }
    else
    {
        REG32(RTL8390MP_TC0INT) |= RTL8390MP_TCIP;
    }

    return;
}

static void __init rtl8390_time_init(void)
{
}


static void __init rtl8390_timer_setup(struct irqaction *irq)
{
    unsigned int model_info;
    unsigned int chip_info;


    model_info = REG32(0xBB000FF0);
    REG32(0xBB000FF4) = 0xA0000000;
    chip_info = REG32(0xBB000FF4);
    REG32(0xBB000FF4) = 0;

	/* Setup Timer0 */

	/* Clear Timer IP status */
    if ((chip_info & 0xFFFF) == 0x0399)
    {
    	if (REG32(RTL8390ES_TCIR) & RTL8390ES_TC0IP)
    		REG32(RTL8390ES_TCIR) |= RTL8390ES_TC0IP;

    	/* Here irq->handler is passed from outside */
    	irq->handler = timer_interrupt;
    	setup_irq(TC0_IRQ, irq);

    	REG32(RTL8390ES_TCCNR) = 0; /* disable timer before setting CDBR */
    	REG32(RTL8390ES_CDBR) = (DIVISOR_RTL8390) << RTL8390ES_DIVF_OFFSET;
    	REG32(TC0DATA) = ((MHZ * 1000000)/(DIVISOR_RTL8390 * HZ)) << RTL8390ES_TCD_OFFSET;

    	REG32(RTL8390ES_TCCNR) = RTL8390ES_TC0EN | RTL8390ES_TC0MODE_TIMER;
    	REG32(RTL8390ES_TCIR) = RTL8390ES_TC0IE;
    }
    else
    {
        if (REG32(RTL8390MP_TC0INT) & RTL8390MP_TCIP)
            REG32(RTL8390MP_TC0INT) |= RTL8390MP_TCIP;

        /* Here irq->handler is passed from outside */
        irq->handler = timer_interrupt;
        setup_irq(TC0_IRQ, irq);

        REG32(RTL8390MP_TC0CTL) = 0; /* disable timer before setting CDBR */
        REG32(TC0DATA)= ((MHZ * 1000000)/(DIVISOR_RTL8390 * HZ));
        REG32(RTL8390MP_TC0CTL) = RTL8390MP_TCEN | RTL8390MP_TCMODE_TIMER | DIVISOR_RTL8390 ;
        REG32(RTL8390MP_TC0INT) = RTL8390MP_TCIE;
    }

	return;
}
#endif /*End of CONFIG_RTL8390_SERIES*/

#if defined(CONFIG_RTL8380_SERIES)
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

static void __init rtl8380_timer_setup(struct irqaction *irq)
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
#endif /*End of CONFIG_RTL8380_SERIES*/

#if defined(CONFIG_RTL8328_SERIES)
static void rtl8328_timer_ack(void)
{
	REG32(TCIR) |= TC0IP;
}

static void __init rtl8328_time_init(void)
{
}
static void __init rtl8328_timer_setup(struct irqaction *irq)
{
	/* Setup Timer0 */

	/* Clear Timer IP status */
	if (REG32(TCIR) & TC0IP)
		REG32(TCIR) |= TC0IP;

	/* Here irq->handler is passed from outside */
	irq->handler = timer_interrupt;
	setup_irq(TC0_IRQ, irq);

	REG32(TCCNR) = 0; /* disable timer before setting CDBR */
	REG32(CDBR) = (DIVISOR) << DIVF_OFFSET;
	REG32(TC0DATA) = ((MHZ * 1000000)/(DIVISOR * HZ)) << TCD_OFFSET;

	REG32(TCCNR) = TC0EN | TC0MODE_TIMER;
	REG32(TCIR) = TC0IE;

}
#endif /*End of CONFIG_RTL8328_SERIES*/

void __init plat_timer_setup(struct irqaction *irq)
{
#if defined(CONFIG_RTL8390_SERIES)
	if((bsp_chip_family_id == RTL8390_FAMILY_ID) || (bsp_chip_family_id == RTL8350_FAMILY_ID))
		rtl8390_timer_setup(irq);
#endif
#if defined(CONFIG_RTL8380_SERIES)
	if((bsp_chip_family_id == RTL8380_FAMILY_ID) || (bsp_chip_family_id == RTL8330_FAMILY_ID))
		rtl8380_timer_setup(irq);
#endif
#if defined(CONFIG_RTL8328_SERIES)
	if(bsp_chip_family_id == RTL8328_FAMILY_ID)
		rtl8328_timer_setup(irq);
#endif

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

EXPORT_SYMBOL(rtk_hook_restart_function);

