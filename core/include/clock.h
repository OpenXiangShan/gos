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

#ifndef __CLOCK_H
#define __CLOCK_H

#include "list.h"
#include "spinlocks.h"
#include "timer.h"

#define NSEC_PER_SEC 1000000000UL

struct clock_source {
	unsigned long last_ms;
	unsigned long last_cycles;
	unsigned int freq;
	unsigned int mult;
	unsigned int shift;
	unsigned long (*read)(struct clock_source * src);
};

struct clock_event {
	int cpu;
	int hwirq;
	void (*evt_handler)(struct clock_event * evt);
	int (*set_next_event)(unsigned long next, struct clock_event * evt);
	struct list_head timer_list;
	spinlock_t lock;
};

int register_clock_source(struct clock_source *src, int cpu);
int register_clock_event(struct clock_event *evt, int cpu);
int register_timer_event(struct timer_event_info *timer_event, int cpu);
int unregister_timer_event(struct timer_event_info *timer, int cpu);
void do_clock_event_handler(void);
unsigned long get_clocksource_counter(void);
unsigned long get_clocksource_counter_us(void);
unsigned long cycles_to_ms(unsigned long cycles, unsigned long freq_hz);
unsigned long ms_to_cycles(unsigned long ms, unsigned long freq_hz);
unsigned long get_clock_source_freq(void);
unsigned long get_system_tick(void);
unsigned long get_system_clock_freq(void);
void clock_set_next_event(unsigned long expiry_time);

#endif
