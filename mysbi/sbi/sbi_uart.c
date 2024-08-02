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

#include <string.h>
#include <print.h>
#include <device.h>
#include <asm/mmio.h>
#include <asm/csr.h>
#include "sbi_trap.h"
#include "sbi_uart.h"
#include "uart_qemu-8250.h"
#include "uart_ns16550a.h"
#include "uart_uartlite.h"

static struct sbi_uart_ops ops = { 0, };

void sbi_uart_putc(char c)
{
	if (c == '\n')
		ops.putc('\r');
	ops.putc(c);
}

void sbi_uart_puts(char *str)
{
	while (*str != '\0')
		sbi_uart_putc(*str++);
}

static int __sbi_uart_init(struct sbi_trap_hw_context *ctx)
{
	extern unsigned long DEVICE_INIT_TABLE, DEVICE_INIT_TABLE_END;
	int nr =
	    (struct device_init_entry *)&DEVICE_INIT_TABLE_END -
	    (struct device_init_entry *)&DEVICE_INIT_TABLE;
	struct device_init_entry *device_entry;

	for (device_entry = (struct device_init_entry *)&DEVICE_INIT_TABLE;
	     nr; device_entry++, nr--) {
		if (!strncmp(device_entry->compatible, "qemu-8250", 128)) {
			ctx->uart_base = device_entry->start;
			uart_qemu_8250_init(device_entry->start, &ops);
			return 0;
		}
		if (!strncmp(device_entry->compatible, "ns16550a", 128)) {
			ctx->uart_base = device_entry->start;
			uart_ns16550a_init(device_entry->start, &ops);
			return 0;
		}
		if (!strncmp(device_entry->compatible, "uartlite", 128)) {
			ctx->uart_base = device_entry->start;
			uart_uartlite_init(device_entry->start, &ops);
			return 0;
		}
	}

	return -1;
}

int sbi_uart_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx)
{
	return __sbi_uart_init(ctx);
}
