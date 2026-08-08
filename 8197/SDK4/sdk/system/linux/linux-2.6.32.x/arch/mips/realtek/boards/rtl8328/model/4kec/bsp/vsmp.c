/*
 * Realtek Semiconductor Corp.
 *
 * bsp/vsmp.c
 *     bsp VSMP initialization code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */

#include <linux/version.h>
#include <linux/smp.h>
#include <linux/interrupt.h>

#ifdef CONFIG_MIPS_MT_SMP

#define BSP_IRQ_IPI_RESCHED		0	/* SW int 0 for resched */
#define BSP_IRQ_IPI_CALL		1	/* SW int 1 for call */

static irqreturn_t bsp_ipi_resched_interrupt(int irq, void *dev_id)
{
	return IRQ_HANDLED;
}

static irqreturn_t bsp_ipi_call_interrupt(int irq, void *dev_id)
{
	smp_call_function_interrupt();
	return IRQ_HANDLED;
}

static struct irqaction irq_resched = {
	.handler    = bsp_ipi_resched_interrupt,
	.flags      = IRQF_DISABLED|IRQF_PERCPU,
	.name       = "IPI_resched"
};

static struct irqaction irq_call = {
	.handler    = bsp_ipi_call_interrupt,
	.flags      = IRQF_DISABLED|IRQF_PERCPU,
	.name       = "IPI_call"
};

static void __init bsp_ipi_init(int irq, struct irqaction *action)
{
	setup_irq(irq, action);
#if 0 /* for IRQ_CPU we dont need to do this */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
	irq_set_handler(irq, handle_percpu_irq);
#else
	set_irq_handler(irq, handle_percpu_irq);
#endif
#endif
}

void __cpuinit bsp_vsmp_init_secondary(void)
{
	change_c0_status(ST0_IM, STATUSF_IP0 | STATUSF_IP1 |
				 STATUSF_IP2 | STATUSF_IP7);
}

void __init bsp_vsmp_irq_init(void)
{
	bsp_ipi_init(BSP_IRQ_IPI_RESCHED, &irq_resched);
	bsp_ipi_init(BSP_IRQ_IPI_CALL, &irq_call);
}
#endif
