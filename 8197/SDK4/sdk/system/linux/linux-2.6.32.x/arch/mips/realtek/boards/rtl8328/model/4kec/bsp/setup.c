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

static void bsp_machine_restart(char *command)
{
	printk("System restart.\n");
	REG32(0xBB020004) = 0x0;    /* Reset Global Control1 Register */
	while(1);
}

static void bsp_machine_halt(void)
{
	printk("System halted.\n");
	while(1);
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
