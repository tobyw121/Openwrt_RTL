/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2007 MIPS Technologies, Inc.
 * Copyright (C) 2007 Ralf Baechle <ralf@linux-mips.org>
 */
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/smp.h>

#include <asm/time.h>
#include <asm/cevt-rlx.h>

/*
 * The SMTC Kernel for the 34K, 1004K, et. al. replaces several
 * of these routines with SMTC-specific variants.
 */

static int rlx_next_event(unsigned long delta,
                           struct clock_event_device *evt)
{
	unsigned int cnt;
	int res;

	cnt = read_c0_count();
	cnt += delta;
	write_c0_compare(cnt);
	res = ((int)(read_c0_count() - cnt) >= 0) ? -ETIME : 0;
	return res;
}

void rlx_set_clock_mode(enum clock_event_mode mode,
				struct clock_event_device *evt)
{
	/* Nothing to do ...  */
}

DEFINE_PER_CPU(struct clock_event_device, rlx_clockevent_device);

irqreturn_t c0_compare_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *cd;
	int cpu = smp_processor_id();

	/*
	 * The same applies to performance counter interrupts.  But with the
	 * above we now know that the reason we got here must be a timer
	 * interrupt.  Being the paranoiacs we are we check anyway.
	 */
	if ((read_c0_cause() & (1 << CAUSEB_TI))) {
		/* Clear Count/Compare Interrupt */
		write_c0_compare(read_c0_compare());
		cd = &per_cpu(rlx_clockevent_device, cpu);
		cd->event_handler(cd);
	}

	return IRQ_HANDLED;
}

struct irqaction c0_compare_irqaction = {
	.handler = c0_compare_interrupt,
	.flags = IRQF_DISABLED | IRQF_PERCPU | IRQF_TIMER,
	.name = "timer",
};


void rlx_event_handler(struct clock_event_device *dev)
{
}

static int c0_compare_int_pending(void)
{
	return (read_c0_cause() & (1 << CAUSEB_TI));
}

/*
 * Compare interrupt can be routed and latched outside the core,
 * so wait up to worst case number of cycle counter ticks for timer interrupt
 * changes to propagate to the cause register.
 */
#define COMPARE_INT_SEEN_TICKS 50

int c0_compare_int_usable(void)
{
	unsigned int delta;
	unsigned int cnt;

	/*
	 * IP7 already pending?  Try to clear it by acking the timer.
	 */
	if (c0_compare_int_pending()) {
		cnt = read_c0_count();
		write_c0_compare(cnt);
		back_to_back_c0_hazard();
		while (read_c0_count() < (cnt  + COMPARE_INT_SEEN_TICKS))
			if (!c0_compare_int_pending())
				break;
		if (c0_compare_int_pending())
			return 0;
	}

	for (delta = 0x10; delta <= 0x400000; delta <<= 1) {
		cnt = read_c0_count();
		cnt += delta;
		write_c0_compare(cnt);
		back_to_back_c0_hazard();
		if ((int)(read_c0_count() - cnt) < 0)
		    break;
		/* increase delta if the timer was already expired */
	}

	while ((int)(read_c0_count() - cnt) <= 0)
		;	/* Wait for expiry  */

	while (read_c0_count() < (cnt + COMPARE_INT_SEEN_TICKS))
		if (c0_compare_int_pending())
			break;
	if (!c0_compare_int_pending())
		return 0;
	cnt = read_c0_count();
	write_c0_compare(cnt);
	back_to_back_c0_hazard();
	while (read_c0_count() < (cnt + COMPARE_INT_SEEN_TICKS))
		if (!c0_compare_int_pending())
			break;
	if (c0_compare_int_pending())
		return 0;

	/*
	 * Feels like a real count / compare timer.
	 */
	return 1;
}

int __cpuinit rlx_clockevent_init(int irq)
{
	uint64_t mips_freq = rlx_hpt_frequency;
	unsigned int cpu = smp_processor_id();
	struct clock_event_device *cd;

	if (!c0_compare_int_usable())
		return -ENXIO;

	cd = &per_cpu(rlx_clockevent_device, cpu);

	cd->name		= "RLX";
	cd->features		= CLOCK_EVT_FEAT_ONESHOT;

	/* Calculate the min / max delta */
	cd->mult		= div_sc((unsigned long) mips_freq, NSEC_PER_SEC, 32);
	cd->shift		= 32;
	cd->max_delta_ns	= clockevent_delta2ns(0x7fffffff, cd);
	cd->min_delta_ns	= clockevent_delta2ns(0x300, cd);

	cd->rating		= 300;
	cd->irq			= irq;
	cd->cpumask		= cpumask_of(cpu);
	cd->set_next_event	= rlx_next_event;
	cd->set_mode		= rlx_set_clock_mode;
	cd->event_handler	= rlx_event_handler;

	clockevents_register_device(cd);

	setup_irq(irq, &c0_compare_irqaction);

	return 0;
}
