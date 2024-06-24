#include "uart.h"
#include "asm/mmio.h"
#include "asm/type.h"
#include "ns16550a.h"
#include "device.h"
#include "vmap.h"

static unsigned long base_address;

static void ns16550a_delay(unsigned int loops)
{
	while (loops--) {
		__asm__ volatile ("nop");
	}
}

static void uart_ns16550a_putc(char c)
{
	unsigned int value;

	value = readl(base_address + LSR);
	while (!(value & LSR_EMPTY)) {
		ns16550a_delay(100);
		value = readl(base_address + LSR);
	}

	writel(base_address + THR, c);
}

void uart_ns16550a_init(unsigned long base, struct myGuest_uart_ops *ops)
{
	base_address = base;

	ops->putc = uart_ns16550a_putc;
}

int ns16550a_init(unsigned long base, unsigned int len, void *data)
{
	base_address = (unsigned long)ioremap((void *)base, len, NULL);

	return 0;
}

DRIVER_REGISTER(ns16550a, ns16550a_init, "ns16550a");
