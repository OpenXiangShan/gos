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

#ifndef	_MYGUEST_UART_H
#define	_MYGUEST_UART_H

#include "device.h"

struct myGuest_uart_ops {
	void (*putc)(char c);
};

void myGuest_uart_putc(char c);
void myGuest_uart_puts(char *str);
int myGuest_uart_init(struct device_init_entry *entry);

#endif
