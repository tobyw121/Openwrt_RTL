/*
 * Realtek Semiconductor Corp.
 *
 * bsp/timer.c
 *     bsp timer initialization code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/timex.h>
#include <linux/delay.h>

#include <asm/time.h>

#include "bspchip.h"

//#ifdef CONFIG_CEVT_EXT
#if 0
void inline bsp_timer_ack(void)
{
	unsigned volatile int eoi;
	eoi = REG32(BSP_TIMER0_EOI);
}

void __init bsp_timer_init(void)
{
	change_c0_cause(CAUSEF_DC, 0);

	/* disable timer */
	REG32(BSP_TIMER0_TCR) = 0x00000000;

	/* initialize timer registers */
	REG32(BSP_TIMER0_TLCR) = BSP_TIMER0_FREQ / HZ;

	/* hook up timer interrupt handler */
	ext_clockevent_init(BSP_TIMER0_IRQ);

	/* enable timer */
	REG32(BSP_TIMER0_TCR) = 0x00000003;       /* 0000-0000-0000-0011 */
}
#endif /* CONFIG_CEVT_EXT */

//#ifdef CONFIG_CEVT_R4K
#if 0
unsigned int __cpuinit get_c0_compare_int(void)
{
	return BSP_COMPARE_IRQ;
}

void __init bsp_timer_init(void)
{
	/* set cp0_compare_irq and cp0_perfcount_irq */
	cp0_compare_irq = BSP_COMPARE_IRQ;
	cp0_perfcount_irq = BSP_PERFCOUNT_IRQ;

	if (cp0_perfcount_irq == cp0_compare_irq)
		cp0_perfcount_irq = -1;

	mips_hpt_frequency = BSP_CPU0_FREQ / 2;

	write_c0_count(0);
	mips_clockevent_init(cp0_compare_irq);
	mips_clocksource_init();
}
#endif /* CONFIG_CEVT_R4K */

static int rtk_watchdog_default_func(void)
{
	return 0;
}

int (*rtk_watchdog_kick_func)(void) = rtk_watchdog_default_func;

EXPORT_SYMBOL(rtk_watchdog_kick_func);

void inline bsp_timer_ack(void)
{
	REG32(TCIR) |= TC0IP;
	
	rtk_watchdog_kick_func();
}

static void __init rtl8328_time_init(void)
{
}
void __init bsp_timer_init(void)
{
	int ret;

/*	change_c0_cause(CAUSEF_DC, 0); */

	/* Clear Timer IP status */
	if (REG32(TCIR) & TC0IP)
		REG32(TCIR) |= TC0IP;
	
	REG32(TCCNR) = 0; /* disable timer before setting CDBR */
	
#ifdef CONFIG_CEVT_EXT
	/* hook up timer interrupt handler */
	ext_clockevent_init(BSP_TC0_EXT_IRQ);
#endif

#ifdef CONFIG_CEVT_R4K
		/* set cp0_compare_irq and cp0_perfcount_irq */
		cp0_compare_irq = BSP_COMPARE_IRQ;
		cp0_perfcount_irq = BSP_PERFCOUNT_IRQ;
		
		if (cp0_perfcount_irq == cp0_compare_irq)
			cp0_perfcount_irq = -1;
		
		mips_hpt_frequency = BSP_CPU0_FREQ / 2;
		
		write_c0_count(0);
		mips_clockevent_init(cp0_compare_irq);
		mips_clocksource_init();
#endif

	REG32(CDBR) = (DIVISOR) << DIVF_OFFSET;
	REG32(TC0DATA) = ((MHZ * 1000000)/(DIVISOR * HZ)) << TCD_OFFSET;

	REG32(TCCNR) = TC0EN | TC0MODE_TIMER;
	REG32(TCIR) = TC0IE;


}

