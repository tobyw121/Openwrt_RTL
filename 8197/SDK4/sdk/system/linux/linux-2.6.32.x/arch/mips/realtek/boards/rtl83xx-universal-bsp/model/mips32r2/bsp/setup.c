/*
 * Realtek Semiconductor Corp.
 *
 * bsp/setup.c
 *     bsp interrult initialization and handler code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#include <linux/console.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>

#include <asm/addrspace.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <asm/bootinfo.h>
#include <asm/time.h>
#include <asm/reboot.h>

#include <asm/smp-ops.h>

#ifdef CONFIG_CPU_HAS_MIPSMT
#include <asm/mipsmtregs.h>
#endif

#include "bspchip.h"


void (* hook_restart_func)(void) = NULL;

void rtk_hook_restart_function(void (*func)(void))
{
	hook_restart_func = func;
	return;
}

static void bsp_machine_restart(char *command)
{
#if defined(CONFIG_RTL8380_SERIES)
    unsigned int tmp = 0;
#endif
	
	if(hook_restart_func != NULL)
	{
		hook_restart_func();
	}

#if defined(CONFIG_RTL8390_SERIES)
	printk("System restart.\n");
	REG32(0xBB000014) = 0xFFFFFFFF;    /* Reset whole chip */
#endif
#if defined(CONFIG_RTL8380_SERIES)
	OTTO838x_FLASH_DISABLE_4BYTE_ADDR_MODE();
	printk("System restart.\n");
    REG32(0xBB000040) = 0x1;    /* Reset Global Control1 Register */
#endif
#if defined(CONFIG_RTL8328_SERIES)
	printk("System restart.\n");
    REG32(0xBB020004) = 0x0;    /* Reset Global Control1 Register */
	while(1);
#endif
}

static void bsp_machine_halt(void)
{
#if defined(CONFIG_RTL8390_SERIES)
	printk("System halted.\n");
	while(1);
#endif
#if defined(CONFIG_RTL8380_SERIES)
	printk("System halted.\n");
	while(1);
#endif
#if defined(CONFIG_RTL8328_SERIES)
	printk("System halted.\n");
	while(1);
#endif

}

#ifdef CONFIG_MIPS_MT_SMTC
extern struct plat_smp_ops bsp_smtc_smp_ops;
#endif

extern int bsp_serial_init(void);

/* callback function */
void __init bsp_setup(void)
{

	_machine_restart = bsp_machine_restart;
    _machine_halt = bsp_machine_halt;

	bsp_serial_init();
}

EXPORT_SYMBOL(rtk_hook_restart_function);

