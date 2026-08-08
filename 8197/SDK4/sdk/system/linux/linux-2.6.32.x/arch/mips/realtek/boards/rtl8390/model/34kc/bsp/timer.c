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
	
	rtk_watchdog_kick_func();
}

void __init bsp_timer_init(void)
{
    unsigned int model_info;
    unsigned int chip_info;
	int ret;

    model_info = REG32(0xBB000FF0);
    REG32(0xBB000FF4) = 0xA0000000;
    chip_info = REG32(0xBB000FF4);
    REG32(0xBB000FF4) = 0;

/*	change_c0_cause(CAUSEF_DC, 0); */

	/* Clear Timer IP status */
    if ((chip_info & 0xFFFF) == 0x0399)
    {
    	if (REG32(RTL8390ES_TCIR) & RTL8390ES_TC0IP)
    		REG32(RTL8390ES_TCIR) |= RTL8390ES_TC0IP;
    	
    	REG32(RTL8390ES_TCCNR) = 0; /* disable timer before setting CDBR */
	
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

    	REG32(RTL8390ES_CDBR) = (DIVISOR) << RTL8390ES_DIVF_OFFSET;
    	REG32(TC0DATA) = ((MHZ * 1000000)/(DIVISOR * HZ)) << RTL8390ES_TCD_OFFSET;
    
    	REG32(RTL8390ES_TCCNR) = RTL8390ES_TC0EN | RTL8390ES_TC0MODE_TIMER;
    	REG32(RTL8390ES_TCIR) = RTL8390ES_TC0IE;
    }
    else
    {
    	if (REG32(RTL8390MP_TC0INT) & RTL8390MP_TCIP)
    		REG32(RTL8390MP_TC0INT) |= RTL8390MP_TCIP;
    	
    	REG32(RTL8390MP_TC0CTL) = 0; /* disable timer before setting CDBR */
	
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
    
        REG32(TC0DATA)= ((MHZ * 1000000)/(DIVISOR * HZ));
        REG32(RTL8390MP_TC0CTL) = RTL8390MP_TCEN | RTL8390MP_TCMODE_TIMER | DIVISOR ;
        REG32(RTL8390MP_TC0INT) = RTL8390MP_TCIE;        
    }

    return;
}

