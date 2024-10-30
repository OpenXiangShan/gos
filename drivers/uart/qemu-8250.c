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

#include <asm/mmio.h>
#include <device.h>
#include <print.h>
#include <asm/type.h>
#include <event.h>
#include <string.h>
#include "qemu-8250.h"
#include <asm/sbi.h>
#include "vmap.h"
#include "irq.h"
#include "uart.h"
#include "../bsp/uart_data.h"

static unsigned long base_address;
static unsigned int size;
static int wakeup = 0;
static unsigned int UART_DEFAULT_BAUD;
static unsigned int uart16550_clock;

unsigned long qemu_8250_get_base(void)
{
	return base_address;
}

unsigned long qemu_8250_get_size(void)
{
	return size;
}

static void qemu_8250_putc(char c)
{
	while ((readb(base_address + UART_LSR) & UART_LSR_EMPTY) == 0) ;

	writeb(base_address + UART_DAT, c);
}

static void qemu_8250_puts(char *str)
{
	while (*str != '\0')
		qemu_8250_putc(*str++);
}

static char qemu_8250_get(void)
{
	if (readb(base_address + UART_LSR) & UART_LSR_DR)
		return readb(base_address + UART_DAT);

	else
		return -1;
}

static int qemu_8250_init(unsigned long base, int len)
{
	unsigned int divisor = uart16550_clock / (16 * UART_DEFAULT_BAUD);

	base_address = (unsigned long)ioremap((void *)base, len, 0);

	writeb(base_address + UART_IER, 0);

	writeb(base_address + UART_LCR, 0x80);
	writeb(base_address + UART_DLL, (unsigned char)divisor);
	writeb(base_address + UART_DLM, (unsigned char)(divisor >> 8));
	writeb(base_address + UART_LCR, 0x3);
	writeb(base_address + UART_FCR, 0xc7);

	return 0;
}

static void qemu_8250_irq_handler(void *data)
{
	wakeup = 1;
}

static int qemu_8250_write(struct device *dev, char *buf,
			   unsigned long offset, unsigned int len)
{
	int i;

	for (i = 0; i < len; i++)
		qemu_8250_putc(*buf++);

	return 0;
}

static int wake_expr(void *data)
{
	int *wake = (int *)data;

	return *wake == 1;
}

static int qemu_8250_read(struct device *dev, char *buf,
			  unsigned long offset, unsigned int len, int flag)
{
	char c;
	unsigned int ret = 0;

	if (flag == BLOCK) {
		wait_for_event(&wakeup, wake_expr);
		wakeup = 0;
	}

	while (ret < len) {
		c = qemu_8250_get();
		if (c < 0 || c == 255)
			return ret;
		else
			*buf++ = c;
		ret++;
	}

	return 0;
}

static const struct driver_ops qemu_8250_ops = {
	.write = qemu_8250_write,
	.read = qemu_8250_read,
};

int qemu_8250_driver_init(struct device *dev, void *data)
{
	struct driver *drv;
	int irqs[16], nr_irqs, i;

	print("8250: base: 0x%lx, len: %d, nr_irqs:%d irq: %d\n",
	      dev->base, dev->len, dev->irq_num, dev->irqs[0]);

	writeb(base_address + UART_IER, 1);

	nr_irqs = get_hwirq(dev, irqs);
	for (i = 0; i < nr_irqs; i++)
		register_device_irq(dev, dev->irq_domain, irqs[i],
				    qemu_8250_irq_handler, NULL);

	drv = dev->drv;
	strcpy(dev->name, "UART0");
	strcpy(drv->name, "UART0");
	drv->ops = &qemu_8250_ops;

	size = dev->len;

	return 0;
}

DRIVER_REGISTER(qemu_8250, qemu_8250_driver_init, "qemu-8250");

int qemu_8250_earlycon_init(unsigned long base, int len,
			    struct early_print_device *device,
			    void *data)
{
	struct uart_data *priv = (struct uart_data *)(data);

	UART_DEFAULT_BAUD = priv->baud;
	uart16550_clock = priv->clk;
	qemu_8250_init(base, len);
	device->write = qemu_8250_puts;

	return 0;
}

EARLYCON_REGISTER(qemu_8250, qemu_8250_earlycon_init, "qemu-8250");
