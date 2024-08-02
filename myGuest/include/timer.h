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

#ifndef __TIMER_H__
#define __TIMER_H__

typedef int (*do_timer_t)(void *data);
typedef unsigned long (*get_system_tick_t)(void);

int irq_do_timer_handler(void);
void register_timer_irq_handler(do_timer_t do_timer, void *data);
unsigned long get_system_tick(void);
unsigned long get_system_time(void);
void register_system_tick_handler(get_system_tick_t handler);
int set_timer(do_timer_t handler, unsigned long ms, void *data);

#endif
