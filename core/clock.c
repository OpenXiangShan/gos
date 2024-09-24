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

#include <asm/sbi.h>
#include <asm/type.h>
#include "clock.h"
#include "percpu.h"
#include "list.h"
#include "spinlocks.h"
#include "mm.h"

static struct clock_source *clock_src __attribute__((section(".data"))) = NULL;
static DEFINE_PER_CPU(struct clock_event, clock_event);

unsigned long get_clock_source_freq(void)
{
	return clock_src->freq;
}

unsigned long cycles_to_ms(unsigned long cycles, unsigned long freq_hz)
{
	return (cycles * 1000) / freq_hz;
}

unsigned long cycles_to_us(unsigned long cycles, unsigned long freq_hz)
{
	return (cycles * 1000000) / freq_hz;
}

unsigned long ms_to_cycles(unsigned long ms, unsigned long freq_hz)
{
	return (freq_hz / 1000) * ms;
}

static int clock_event_do_timer_list(struct clock_event *event)
{
	struct timer_event_info *te;
	struct timer_event_info *freeze_te[16];
	irq_flags_t flags;
	int restart = 0, i, freeze_nr = 0;
	unsigned long restart_ptr[16];

	spin_lock_irqsave(&event->lock, flags);

	while (!list_empty(&event->timer_list)) {
		te = list_entry(list_first(&event->timer_list),
				struct timer_event_info, list);

		if (te->freeze == 1) {
			list_del(&te->list);
			freeze_te[freeze_nr++] = te;
			if (freeze_nr >= 16)
				break;
			continue;
		}

		if (te->expiry_time <= get_clocksource_counter()) {
			list_del(&te->list);

			te->handler(te->data);

			if (!te->restart) {
				if (te->del_cb)
					te->del_cb(te);
			} else {
				te->expiry_time =
				    get_system_time() + te->period;
				restart_ptr[restart++] = (unsigned long)te;

				if (restart >= 16)
					break;
			}
		} else
			break;
	}
	if (freeze_nr > 0) {
		for (i = 0; i < freeze_nr; i++)
			list_add_tail(&freeze_te[i]->list, &event->timer_list);
	}
	spin_unlock_irqrestore(&event->lock, flags);

	if (restart > 0) {
		for (i = 0; i < restart; i++)
			register_timer_event((struct timer_event_info *)
					     restart_ptr[i], event->cpu);
	}

	return 0;
}

static int program_next_event(struct clock_event *event,
			      unsigned long expiry_time)
{
	unsigned long value;

	if (!clock_src)
		return -1;

	value = ms_to_cycles(expiry_time, clock_src->freq);

	return event->set_next_event(value, event);
}

void clock_set_next_event(unsigned long expiry_time)
{
	int cpu = sbi_get_cpu_id();
	struct clock_event *event = &per_cpu(clock_event, cpu);

	if (!event)
		return;

	program_next_event(event, expiry_time);
}

void do_clock_event_handler(void)
{
	struct timer_event_info *te;
	int cpu = sbi_get_cpu_id();
	struct clock_event *event = &per_cpu(clock_event, cpu);
	irq_flags_t flags;

	event->evt_handler(event);

	clock_event_do_timer_list(event);

	spin_lock_irqsave(&event->lock, flags);
	if (list_empty(&event->timer_list)) {
		spin_unlock_irqrestore(&event->lock, flags);
		return;
	}
	te = list_entry(list_first(&event->timer_list), struct timer_event_info,
			list);

	spin_unlock_irqrestore(&event->lock, flags);

	program_next_event(event, te->expiry_time);
}

unsigned long get_system_clock_freq(void)
{
	if (!clock_src)
		return -1;

	return clock_src->freq;
}

unsigned long get_system_tick(void)
{
	struct clock_source *source = clock_src;
	unsigned long cycle;

	if (!source || !source->read)
		return 0;

	cycle = source->read(source);

	return cycle;
}

unsigned long get_clocksource_counter(void)
{
	struct clock_source *source = clock_src;
	unsigned long cycle, ms;

	if (!source || !source->read)
		return 0;

	cycle = source->read(source);
	ms = cycles_to_ms(cycle, source->freq);

	return ms;
}

unsigned long get_clocksource_counter_us(void)
{
	struct clock_source *source = clock_src;
	unsigned long cycle, us;

	if (!source || !source->read)
		return 0;

	cycle = source->read(source);
	us = cycles_to_us(cycle, source->freq);

	return us;
}

int unregister_timer_event(struct timer_event_info *timer, int cpu)
{
	struct clock_event *event = &per_cpu(clock_event, cpu);
	struct timer_event_info *te;
	irq_flags_t flags;

	if (!event)
		return -1;

	spin_lock_irqsave(&event->lock, flags);
	list_for_each_entry(te, &event->timer_list, list) {
		if (te == timer) {
			if (te->restart == 1)
				te->restart = 0;
			else
				break;
		}
	}
	spin_unlock_irqrestore(&event->lock, flags);

	return 0;
}

int register_timer_event(struct timer_event_info *timer_event, int cpu)
{
	int found = 0, first = 0;
	struct timer_event_info *te;
	struct clock_event *event = &per_cpu(clock_event, cpu);
	irq_flags_t flags;

	if (!event)
		return -1;

	spin_lock_irqsave(&event->lock, flags);
	if (list_empty(&event->timer_list)) {
		first = 1;
		goto add;
	}

	list_for_each_entry(te, &event->timer_list, list) {
		if (timer_event->expiry_time < te->expiry_time) {
			found = 1;
			break;
		}
	}

add:
	if (found)
		list_add_tail(&timer_event->list, &te->list);
	else
		list_add_tail(&timer_event->list, &event->timer_list);

	spin_unlock_irqrestore(&event->lock, flags);

	if (first) {
		spin_lock_irqsave(&event->lock, flags);
		te = list_entry(list_first(&event->timer_list),
				struct timer_event_info, list);
		spin_unlock_irqrestore(&event->lock, flags);
		program_next_event(event, te->expiry_time);
	}

	return 0;
}

int register_clock_event(struct clock_event *evt, int cpu)
{
	struct clock_event *event = &per_cpu(clock_event, cpu);

	if (!event)
		return -1;

	event->cpu = cpu;
	event->hwirq = evt->hwirq;
	event->evt_handler = evt->evt_handler;
	event->set_next_event = evt->set_next_event;
	INIT_LIST_HEAD(&event->timer_list);
	__SPINLOCK_INIT(&event->lock);

	return 0;
}

int register_clock_source(struct clock_source *src, int cpu)
{
	if (!src)
		return -1;

	clock_src = src;

	return 0;
}
