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
#include "chip.h"


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

extern unsigned int bsp_chip_id, bsp_chip_rev_id;
extern unsigned int bsp_chip_family_id, bsp_chip_type;


static int rtk_watchdog_default_func(void)
{
	return 0;
}

int (*rtk_watchdog_kick_func)(void) = rtk_watchdog_default_func;

EXPORT_SYMBOL(rtk_watchdog_kick_func);

void inline bsp_timer_ack(void)
{
#if defined(CONFIG_RTL8390_SERIES)
	if((bsp_chip_family_id == RTL8390_FAMILY_ID) || (bsp_chip_family_id == RTL8350_FAMILY_ID))
	{
	    if (bsp_chip_type == CHIP_MES_TYPE)
    {
        	REG32(RTL8390TC_TCIR) |= RTL8390TC_TC0IP;
	    }
        else
	    {
    	    REG32(RTL8390MP_TC0INT) |= RTL8390MP_TCIP;
	    }
    }
#endif

#if defined(CONFIG_RTL8380_SERIES)
	if((bsp_chip_family_id == RTL8380_FAMILY_ID) || (bsp_chip_family_id == RTL8330_FAMILY_ID))
    {
	    if (bsp_chip_type == CHIP_MES_TYPE)
	    {
    	    REG32(RTL8380TC_TCIR) |= RTL8380TC_TC0IP;
    }
    else
    {
        	REG32(RTL8380MP_TC0INT) |= RTL8380MP_TCIP;
	    }
    }
#endif
#if defined(CONFIG_RTL8328_SERIES)
    if(bsp_chip_family_id == RTL8328_FAMILY_ID)
    {
         REG32(TCIR) |= TC0IP;
    } 
#endif
	rtk_watchdog_kick_func();
}

#if defined(CONFIG_RTL8328_SERIES)
static void rtl8328_bsp_timer_init(void)
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
#endif

#if defined(CONFIG_RTL8390_SERIES)
static void rtl8390_bsp_timer_init(void)
{

	/* Clear Timer IP status */
    if (bsp_chip_type == CHIP_MES_TYPE)
    {
    	if (REG32(RTL8390TC_TCIR) & RTL8390TC_TC0IP)
    		REG32(RTL8390TC_TCIR) |= RTL8390TC_TC0IP;
    	
    	REG32(RTL8390TC_TCCNR) = 0; /* disable timer before setting CDBR */
	
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

    	REG32(RTL8390TC_CDBR) = (DIVISOR_RTL8390) << RTL8390TC_DIVF_OFFSET;
    	REG32(TC0DATA) = ((MHZ * 1000000)/(DIVISOR_RTL8390 * HZ)) << RTL8390TC_TCD_OFFSET;
    
    	REG32(RTL8390TC_TCCNR) = RTL8390TC_TC0EN | RTL8390TC_TC0MODE_TIMER;
    	REG32(RTL8390TC_TCIR) = RTL8390TC_TC0IE;
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
    
        REG32(TC0DATA)= ((MHZ * 1000000)/(DIVISOR_RTL8390 * HZ));
        REG32(RTL8390MP_TC0CTL) = RTL8390MP_TCEN | RTL8390MP_TCMODE_TIMER | DIVISOR_RTL8390 ;
        REG32(RTL8390MP_TC0INT) = RTL8390MP_TCIE;        
    }



}
#endif

#if defined(CONFIG_RTL8380_SERIES)
static void rtl8380_bsp_timer_init(void)
{
	/* Clear Timer IP status */
    if (bsp_chip_type == CHIP_MES_TYPE)
    {
    	if (REG32(RTL8380TC_TCIR) & RTL8380TC_TC0IP)
    		REG32(RTL8380TC_TCIR) |= RTL8380TC_TC0IP;
    	
    	REG32(RTL8380TC_TCCNR) = 0; /* disable timer before setting CDBR */
	
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
    
    	REG32(RTL8380TC_CDBR) = (DIVISOR_RTL8380) << RTL8380TC_DIVF_OFFSET;
    	REG32(TC0DATA) = ((MHZ * 1000000)/(DIVISOR_RTL8380 * HZ)) << RTL8380TC_TCD_OFFSET;
    
    	REG32(RTL8380TC_TCCNR) = RTL8380TC_TC0EN | RTL8380TC_TC0MODE_TIMER;
    	REG32(RTL8380TC_TCIR) = RTL8380TC_TC0IE;
    }
    else
    {
    	if (REG32(RTL8380MP_TC0INT) & RTL8380MP_TCIP)
    		REG32(RTL8380MP_TC0INT) |= RTL8380MP_TCIP;
    	
    	REG32(RTL8380MP_TC0CTL) = 0; /* disable timer before setting CDBR */
	
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
    
        REG32(TC0DATA)= ((MHZ * 1000000)/(DIVISOR_RTL8380* HZ));
        REG32(RTL8380MP_TC0CTL) = RTL8380MP_TCEN | RTL8380MP_TCMODE_TIMER | DIVISOR_RTL8380 ;
        REG32(RTL8380MP_TC0INT) = RTL8380MP_TCIE;        
    }

    return;
}
#endif

void __init bsp_timer_init(void)
{
#if defined(CONFIG_RTL8390_SERIES)
	if((bsp_chip_family_id == RTL8390_FAMILY_ID) || (bsp_chip_family_id == RTL8350_FAMILY_ID))
		rtl8390_bsp_timer_init();
#endif
#if defined(CONFIG_RTL8380_SERIES)
	if((bsp_chip_family_id == RTL8380_FAMILY_ID) || (bsp_chip_family_id == RTL8330_FAMILY_ID))
		rtl8380_bsp_timer_init();
#endif
#if defined(CONFIG_RTL8328_SERIES)
	if(bsp_chip_family_id == RTL8328_FAMILY_ID)
		rtl8328_bsp_timer_init();
#endif
    return;
}

