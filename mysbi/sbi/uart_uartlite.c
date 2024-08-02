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

#include <sbi_uart.h>
#include <asm/mmio.h>
#include <device.h>
#include "uart_uartlite.h"

static unsigned long base_address;

static void uartlite_putc(char c)
{
	while (readb(base_address + UARTLITE_STAT_REG) & UARTLITE_TX_FULL) ;

	writeb(base_address + UARTLITE_TX_FIFO, c);
}

void uart_uartlite_init(unsigned long base, struct sbi_uart_ops *ops)
{
	base_address = base;

	writeb(base_address + UARTLITE_CTRL_REG, UARTLITE_RST_FIFO);

	ops->putc = uartlite_putc;
}
