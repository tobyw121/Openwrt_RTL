/*
 * Realtek Semiconductor Corp.
 *
 * bsp/irq.c
 *     bsp interrupt initialization and handler code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/timex.h>
#include <linux/random.h>
#include <linux/irq.h>
#include <linux/version.h>

#include <asm/bitops.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/irq_cpu.h>
#include <asm/system.h>

#include <asm/mipsregs.h>

#include "bspchip.h"

/*
 * Symbol Definition
 */

#define bsp_ictl_shutdown_irq      bsp_ictl_disable_irq
#define bsp_ictl_mask_and_ack_irq  bsp_ictl_disable_irq

spinlock_t irq_lock = SPIN_LOCK_UNLOCKED;

irqreturn_t bsp_ictl_irq_dispatch1(int, void *);
irqreturn_t bsp_ictl_irq_dispatch2(int, void *);
irqreturn_t bsp_ictl_irq_dispatch3(int, void *);
irqreturn_t bsp_ictl_irq_dispatch4(int, void *);
irqreturn_t bsp_ictl_irq_dispatch5(int, void *);


static struct irqaction irq_cascade1 = { 	
	.handler = bsp_ictl_irq_dispatch1,	
	.name = "RTL8390 IRQ cascade1",
};

static struct irqaction irq_cascade2 = { 	
	.handler = bsp_ictl_irq_dispatch2,	
	.name = "RTL8390 IRQ cascade2",
};

static struct irqaction irq_cascade3 = { 	
	.handler = bsp_ictl_irq_dispatch3,	
	.name = "RTL8390 IRQ cascade3",
};

static struct irqaction irq_cascade4 = { 	
	.handler = bsp_ictl_irq_dispatch4,	
	.name = "RTL8390 IRQ cascade4",
};

static struct irqaction irq_cascade5 = { 	
	.handler = bsp_ictl_irq_dispatch5,	
	.flags = IRQF_DISABLED | IRQF_PERCPU,	
	.name = "RTL8390 IRQ cascade5",
};

static void bsp_ictl_enable_irq(unsigned int irq)
{   
	unsigned long flags;   

	spin_lock_irqsave(&irq_lock, flags);   
	REG32(GIMR) = REG32(GIMR) | (1 << ICTL_OFFSET(irq));   
	spin_unlock_irqrestore(&irq_lock, flags);
}

static unsigned int bsp_ictl_startup_irq(unsigned int irq)
{   
	bsp_ictl_enable_irq(irq);   
	return 0;
}

static void bsp_ictl_disable_irq(unsigned int irq)
{   
	unsigned long flags;   

	spin_lock_irqsave(&irq_lock, flags);   
	REG32(GIMR) = REG32(GIMR) & (~(1 << ICTL_OFFSET(irq)));   
	spin_unlock_irqrestore(&irq_lock, flags);
}

static void bsp_ictl_end_irq(unsigned int irq)
{   
	unsigned long flags;   

	spin_lock_irqsave(&irq_lock, flags);   
	REG32(GIMR) = REG32(GIMR) | (1 << ICTL_OFFSET(irq));   
	spin_unlock_irqrestore(&irq_lock, flags);
}

static struct irq_chip bsp_ictl_irq = {	
		.name = "RTL8390 ICTL",	
		.startup = bsp_ictl_startup_irq,	
		.shutdown = bsp_ictl_shutdown_irq,	
		.enable = bsp_ictl_enable_irq,	
		.disable = bsp_ictl_disable_irq,	
		.ack = bsp_ictl_mask_and_ack_irq,	
		.mask = bsp_ictl_mask_and_ack_irq,	
		.unmask = bsp_ictl_enable_irq,	
		.end = bsp_ictl_end_irq,
};

/* 
*   RTL8390 Interrupt Scheme
* 
*   Source       EXT_INT   IRQ        CPU INT 
*   --------   -------   ------   ------- 
*   UART0          39            39         IP3 
*   UART1          38            38         IP2 
*   TIMER0         37            37         IP6 
*   TIMER1         36            36         IP2 
*   OCPTO          35            35         IP2
*   HLXTO          34            34         IP2
*   SLXTO          33            33         IP2
*   NIC              32            32         IP5
*   GPIO_ABCD   31            31         IP5
*   SWCORE       28            28         IP4 
*/

irqreturn_t bsp_ictl_irq_dispatch1(int cpl, void *dev_id)
{	
	/* For shared interrupts */	
	unsigned int extint_ip = REG32(GIMR) & REG32(GISR);		

	if (extint_ip & TC1_IP)		
		do_IRQ(BSP_TC1_EXT_IRQ);	
	else if (extint_ip & UART1_IP)		
		do_IRQ(BSP_UART1_EXT_IRQ);	
	else		
		spurious_interrupt();	
	
	return IRQ_HANDLED;
}

irqreturn_t bsp_ictl_irq_dispatch2(int cpl, void *dev_id)
{	
	do_IRQ(BSP_UART0_EXT_IRQ);	
	return IRQ_HANDLED;
}

irqreturn_t bsp_ictl_irq_dispatch3(int cpl, void *dev_id)
{	
	do_IRQ(BSP_SWCORE_EXT_IRQ);	
	return IRQ_HANDLED;
}

irqreturn_t bsp_ictl_irq_dispatch4(int cpl, void *dev_id)
{	
	/* For shared interrupts */	
	unsigned int extint_ip = REG32(GIMR) & REG32(GISR);		

	if (extint_ip & NIC_IP) /* NIC */		
		do_IRQ(BSP_NIC_EXT_IRQ);	
	else if (extint_ip & GPIO_ABCD_IP)  /* GPIO ABCD */		
		do_IRQ(BSP_GPIO_ABCD_EXT_IRQ);	
	else		
		spurious_interrupt();		

	return IRQ_HANDLED;
}

irqreturn_t bsp_ictl_irq_dispatch5(int cpl, void *dev_id)
{	
	do_IRQ(BSP_TC0_EXT_IRQ);	

	return IRQ_HANDLED;
}


/*
 * Data Declaration
 */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
static void bsp_ictl_irq_mask(struct irq_data *d)
{
	REG32(BSP_ICTL_MASK) |= (1 << (d->irq - BSP_IRQ_ICTL_BASE));
}

static void bsp_ictl_irq_unmask(struct irq_data *d)
{
	REG32(BSP_ICTL_MASK) &= ~(1 << (d->irq - BSP_IRQ_ICTL_BASE));
}

static struct irq_chip bsp_ictl_irq = {
	.name = "Sheipa ICTL",
	.irq_ack = bsp_ictl_irq_mask,
	.irq_mask = bsp_ictl_irq_mask,
	.irq_unmask = bsp_ictl_irq_unmask,
};
#endif

void bsp_irq_dispatch(void)
{
	unsigned int pending;	

	pending = read_c0_cause() & read_c0_status() & ST0_IM;	

	if (pending & CAUSEF_IP6)		
		do_IRQ(6);	
	else if (pending & CAUSEF_IP5)		
		do_IRQ(5);	
	else if (pending & CAUSEF_IP4)		
		do_IRQ(4);	
	else if (pending & CAUSEF_IP3)		
		do_IRQ(3);	
	else if (pending & CAUSEF_IP2)		
		do_IRQ(2);	else		
		spurious_interrupt();
}
static void __init bsp_ictl_irq_init(unsigned int irq_base)
{	
	int i;	

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
	for (i=0; i < BSP_IRQ_ICTL_NUM; i++) 
		irq_set_chip_and_handler(irq_base + i, &bsp_ictl_irq, handle_level_irq);
#else
	for (i=0; i < BSP_IRQ_ICTL_NUM; i++) 
		set_irq_chip_and_handler(irq_base + i, &bsp_ictl_irq, handle_level_irq);
#endif 

	setup_irq(BSP_ICTL1_IRQ, &irq_cascade1);	
	setup_irq(BSP_ICTL2_IRQ, &irq_cascade2);	
	setup_irq(BSP_ICTL3_IRQ, &irq_cascade3);	
	setup_irq(BSP_ICTL4_IRQ, &irq_cascade4);	
	setup_irq(BSP_ICTL5_IRQ, &irq_cascade5);
	
	/* Set GIMR, IRR */	
	REG32(GIMR) = TC0_IE | UART0_IE;		
	REG32(IRR0) = IRR0_SETTING;	
	REG32(IRR1) = IRR1_SETTING;	
	REG32(IRR2) = IRR2_SETTING;	
	REG32(IRR3) = IRR3_SETTING;
}

void __init bsp_irq_init(void)
{
	extern void bsp_vsmp_irq_init(void);

	/* initialize IRQ action handlers */
	mips_cpu_irq_init(BSP_IRQ_CPU_BASE);
	bsp_ictl_irq_init(BSP_IRQ_ICTL_BASE);

#ifdef CONFIG_MIPS_MT_SMP
	bsp_vsmp_irq_init();
#endif

}
