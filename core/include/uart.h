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

#ifndef _UART_H
#define _UART_H

#include "device.h"

#define EARLYCON_INIT_TABLE __earlycon_init_table
#define EARLYCON_INIT_TABLE_END __earlycon_init_table_end

struct early_print_device {
	char name[128];
	void (*write)(char *str);
	int early_print_enable;
};

typedef int (*earlycon_init)(unsigned long base, int len,
			     struct early_print_device * device,
			     void *data);

struct earlycon_init_entry {
	char compatible[128];
	earlycon_init init;
};

#define EARLYCON_REGISTER(name, init_fn, compat)                              \
	static const struct earlycon_init_entry __attribute__((used))         \
		__earlycon_entry_##name                                       \
		__attribute__((section(".earlycon_init_table"))) = {          \
			.compatible = compat,                                 \
			.init = init_fn,                                      \
		}


void uart_putc(char c);
void uart_puts(char *str);
int uart_init(void);
int early_print_setup(struct device_init_entry *hw);

#endif
