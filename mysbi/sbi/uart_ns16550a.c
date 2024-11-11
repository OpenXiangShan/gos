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
#include "uart_ns16550a.h"
#include "../../bsp/uart_data.h"
#include "autoconf.h"

static unsigned int UART_CLK;
static unsigned int UART_DEFAULT_BAUD;

#define DW_UART_WAIT_POLL_CNT_MAX 1024

static unsigned long base_address;

static void ns16550a_delay(unsigned int loops)
{
	while (loops--) {
		__asm__ volatile ("nop");
	}
}

static void ns16550a_putc(char c)
{
	unsigned int value;

	value = readl(base_address + LSR);
	while (!(value & 0x60)) {
		ns16550a_delay(100);
		value = readl(base_address + LSR);
	}

	writel(base_address + THR, c);
}

void uart_ns16550a_init(unsigned long base, struct sbi_uart_ops *ops, void *data)
{
	struct uart_data *priv = (struct uart_data *)(data);
	unsigned int divisor;

	UART_DEFAULT_BAUD = priv->baud;
	UART_CLK = 24000000;//24M

	divisor = UART_CLK / (16 * UART_DEFAULT_BAUD);

	base_address = base;

	writel(base_address + LCR, 0x83);

	while (readl(base_address + USR) & 0x1) ;

	writel(base_address + DLH, 0);
	writel(base_address + DLL, divisor);
	writel(base_address + LCR, 0x03);
	writel(base_address + FCR, 0x01);
	writel(base_address + IER, 0);
	writel(base_address + MCR, (RTS | DTR));

	ops->putc = ns16550a_putc;
}

void uart_ns16550a_update_baud(unsigned long base, struct sbi_uart_ops *ops, void *data)
{
	struct uart_data *priv = (struct uart_data *)(data);
	unsigned int divisor;

	UART_DEFAULT_BAUD = priv->baud;
	UART_CLK = priv->clk;

	divisor = priv->clk / (16 * priv->baud);

	base_address = base;
	unsigned long val;
	int poll_cnt;

	poll_cnt = 0;
	do {
		poll_cnt++;
		if (poll_cnt >= DW_UART_WAIT_POLL_CNT_MAX) return;

		/* Clear data received if any */
		val = readl(base_address + 0x14);
		if ((val & 0x03) != 0) {
			val = readl(base_address + 0x00);
			continue;
		}

		/* Wait until line not busy */
		val = readl(base_address + 0x7C);
		if ((val & 0x01) != 0) {
			continue;
		}

		/* Try to latch */
		val = readl(base_address + 0x0C);
		val |= 0x80;
		writel(base_address + 0x0C, val);
		val = readl(base_address + 0x0C);
	} while ((val & 0x80) == 0);

	/* Update new divisor */
	val = divisor & 0xFF; writel(base_address + 0x00, val);	// DLL
	val = divisor >> 8;   writel(base_address + 0x04, val);	// DLH

	/* Wait to clear */
	poll_cnt = 0;
	do {
		val = readl(base_address + 0x0C);
		val &= 0x7F;
		writel(base_address + 0x0C, val);
		val = readl(base_address + 0x0C);
		poll_cnt++;
		if (poll_cnt >= DW_UART_WAIT_POLL_CNT_MAX) return;
	} while ((val & 0x80) != 0);

	ops->putc = ns16550a_putc;
	return;
}
