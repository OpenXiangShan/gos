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

#include "print.h"
#include "timer.h"
#include "asm/type.h"

static do_timer_t irq_do_timer;
static void *irq_data;

static do_timer_t do_timer = NULL;
static void *timer_data;
static unsigned long expiry_time_ms = 0;

static get_system_tick_t __get_system_tick;

unsigned long get_system_tick(void)
{
	return __get_system_tick();
}

unsigned long get_system_time(void)
{
	return __get_system_tick() * 100;	//ms  
}

int irq_do_timer_handler(void)
{
	irq_do_timer(irq_data);
	if (do_timer) {
		if (get_system_time() >= expiry_time_ms) {
			do_timer(timer_data);
			do_timer = NULL;
			timer_data = NULL;
		}
	}

	return 0;
}

void register_timer_irq_handler(do_timer_t do_timer, void *data)
{
	irq_do_timer = do_timer;
	irq_data = data;
}

void register_system_tick_handler(get_system_tick_t handler)
{
	__get_system_tick = handler;
}

int set_timer(do_timer_t handler, unsigned long ms, void *data)
{
	if (do_timer)
		return -1;

	do_timer = handler;
	timer_data = data;
	expiry_time_ms = ms + get_system_time();

	return 0;
}
