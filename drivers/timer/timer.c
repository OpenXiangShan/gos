/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <irq.h>
#include <device.h>
#include <asm/type.h>
#include <asm/trap.h>
#include <asm/mmio.h>
#include <asm/csr.h>
#include <print.h>
#include <asm/sbi.h>
#include <timer.h>
#include "cpu.h"
#include "clock.h"
#include "gos.h"
#include "vmap.h"

#define HZ 1000

#define CLINT_TIMER_CMP 0x4000
#define CLINT_TIMER_VAL 0xbff8

unsigned long clint_freq;
static unsigned long base_address;

struct clint_priv_data {
	unsigned long clint_freq;
};

unsigned long get_cycles(void)
{
	return readq(base_address + CLINT_TIMER_VAL);
}

static void timer_handle_irq(void *data)
{
	do_clock_event_handler();
}

static void __timer_init(void)
{
	csr_set(sie, SIE_STIE);
}

static int timer_set_next_event(unsigned long next, struct clock_event *evt)
{
#if CONFIG_ENABLE_SSTC
	write_csr(CSR_STIMECMP, next);
#else
	sbi_set_timer(next);
#endif
	csr_set(sie, SIE_STIE);

	return 0;
}

static void timer_evt_handler(struct clock_event *evt)
{
	csr_clear(sie, SIE_STIE);
}

static unsigned long timer_counter_read(struct clock_source *src)
{
	return readq(base_address + CLINT_TIMER_VAL);
}

static struct clock_event clock_event_info = {
	.hwirq = INTERRUPT_CAUSE_TIMER,
	.evt_handler = timer_evt_handler,
	.set_next_event = timer_set_next_event,
};

static struct clock_source clock_source_info = {
	.read = timer_counter_read,
};

static void calc_mult_shift(unsigned int *mult, unsigned int *shift,
			    unsigned int from, unsigned int to,
			    unsigned int maxsec)
{
	u64 tmp;
	u32 sft, sftacc = 32;

	/*
	 * Calculate the shift factor which is limiting the conversion
	 * range:
	 */
	tmp = ((u64) maxsec * from) >> 32;
	while (tmp) {
		tmp >>= 1;
		sftacc--;
	}

	/*
	 * Find the conversion shift/mult pair which has the best
	 * accuracy and fits the maxsec conversion range:
	 */
	for (sft = 32; sft > 0; sft--) {
		tmp = (u64) to << sft;
		tmp += from / 2;
		tmp = tmp / from;
		if ((tmp >> sftacc) == 0)
			break;
	}
	*mult = tmp;
	*shift = sft;

}

static void clock_source_init(struct clock_source *src)
{
	unsigned int mult, shift;

	calc_mult_shift(&mult, &shift, clint_freq, NSEC_PER_SEC, 10);

	src->mult = mult;
	src->shift = shift;
	src->freq = clint_freq;
	src->last_cycles = get_cycles();
	//src->last_ns = (get_cycles() * mult) >> shift; 
	src->last_ms = cycles_to_ms(get_cycles(), src->freq);
}

static int timer_cpuhp_startup(struct cpu_hotplug_notifier *notifier, int cpu)
{
	__timer_init();

	register_clock_event(&clock_event_info, cpu);

	return 0;
}

static struct cpu_hotplug_notifier timer_cpuhp_notifier = {
	.startup = timer_cpuhp_startup,
};

int clint_timer_init(unsigned long base, int len, struct irq_domain *d, void *priv)
{
	struct clint_priv_data *data = (struct clint_priv_data *)priv;

	print("%s -- base:0x%lx, clint_freq:0x%x\n", __FUNCTION__, base,
	      data->clint_freq);

	if (!data) {
		print("%s %s %d can not find clint info...\n", __FILE__,
		      __FUNCTION__, __LINE__);
		return -1;
	}

	base_address = (unsigned long)ioremap((void *)base, len, 0);

	clint_freq = data->clint_freq;

	__timer_init();

	clock_source_init(&clock_source_info);

	register_clock_event(&clock_event_info, 0);
	register_clock_source(&clock_source_info, 0);
	register_device_irq(d, INTERRUPT_CAUSE_TIMER, timer_handle_irq, NULL);

	cpu_hotplug_notify_register(&timer_cpuhp_notifier);

	return 0;
}

TIMER_REGISTER(clint, clint_timer_init, "clint");
