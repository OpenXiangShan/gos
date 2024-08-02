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

#include "uart.h"
#include "asm/mmio.h"
#include "asm/type.h"
#include "uartlite.h"
#include "device.h"
#include "vmap.h"

static unsigned long base_address;

static void uartlite_putc(char c)
{
	while (readb(base_address + UARTLITE_STAT_REG) & UARTLITE_TX_FULL) ;

	writeb(base_address + UARTLITE_TX_FIFO, c);
}

void uart_uartlite_init(unsigned long base, struct myGuest_uart_ops *ops)
{
	base_address = base;

	ops->putc = uartlite_putc;
}

int uartlite_init(unsigned long base, unsigned int len, void *data)
{
	base_address = (unsigned long)ioremap((void *)base, len, NULL);

	return 0;
}

DRIVER_REGISTER(uartlite, uartlite_init, "uartlite");
