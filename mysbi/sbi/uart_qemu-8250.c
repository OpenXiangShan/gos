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
#include "uart_qemu-8250.h"

#define UART_DEFAULT_BAUD  115200

static unsigned long base_address;

static void qemu_8250_putc(char c)
{
	while ((readb(base_address + UART_LSR) & UART_LSR_EMPTY) == 0) ;

	writeb(base_address + UART_DAT, c);
}

void uart_qemu_8250_init(unsigned long base, struct sbi_uart_ops *ops)
{
	unsigned int uart16550_clock = 1843200;
	unsigned int divisor = uart16550_clock / (16 * UART_DEFAULT_BAUD);

	base_address = base;

	writeb(base_address + UART_IER, 0);

	writeb(base_address + UART_LCR, 0x80);
	writeb(base_address + UART_DLL, (unsigned char)divisor);
	writeb(base_address + UART_DLM, (unsigned char)(divisor >> 8));
	writeb(base_address + UART_LCR, 0x3);
	writeb(base_address + UART_FCR, 0xc7);

	ops->putc = qemu_8250_putc;
}
