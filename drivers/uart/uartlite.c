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

#include "asm/mmio.h"
#include "device.h"
#include "print.h"
#include "asm/type.h"
#include "event.h"
#include "string.h"
#include "uartlite.h"

static unsigned long base_address;

unsigned long uartlite_get_base(void)
{
	return base_address;
}

static void uartlite_putc(char c)
{
	while (readb(base_address + UARTLITE_STAT_REG) & UARTLITE_TX_FULL) ;

	writeb(base_address + UARTLITE_TX_FIFO, c);
}

static void uartlite_puts(char *str)
{
	while (*str != '\0')
		uartlite_putc(*str++);
}

static int __uartlite_init(unsigned long base)
{
	base_address = base;

	writeb(base_address + UARTLITE_CTRL_REG, UARTLITE_RST_FIFO);

	return 0;
}

int uartlite_earlycon_init(unsigned long base,
			   struct early_print_device *device)
{
	__uartlite_init(base);
	device->write = uartlite_puts;

	return 0;
}

EARLYCON_REGISTER(uartlite, uartlite_earlycon_init, "uartlite");

static int uartlite_write(char *buf, unsigned long offset, unsigned int len)
{
	int i;

	for (i = 0; i < len; i++)
		uartlite_putc(*buf++);

	return 0;
}

static int uartlite_read(char *buf, unsigned long offset, unsigned int len,
			 int flag)
{
	if (flag == BLOCK) {
		while (!readb(base_address + UARTLITE_STAT_REG) &
		       UARTLITE_RX_VALID) ;

		*buf++ = readb(base_address + UARTLITE_RX_FIFO);
	} else if (flag == NONBLOCK) {
		if (!readb(base_address + UARTLITE_STAT_REG) &
		    UARTLITE_RX_VALID)
			return 0;
		*buf++ = readb(base_address + UARTLITE_RX_FIFO);
	}

	return 1;
}

static const struct driver_ops uartlite_ops = {
	.write = uartlite_write,
	.read = uartlite_read,
};

int uartlite_init(struct device *dev, void *data)
{
	struct driver *drv;

	drv = dev->drv;
	strcpy(dev->name, "UART0");
	strcpy(drv->name, "UART0");
	drv->ops = &uartlite_ops;

	return 0;
}

DRIVER_REGISTER(uartlite, uartlite_init, "uartlite");
