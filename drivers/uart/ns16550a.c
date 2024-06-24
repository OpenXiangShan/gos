#include <asm/mmio.h>
#include <device.h>
#include <print.h>
#include <asm/type.h>
#include <event.h>
#include <string.h>
#include "ns16550a.h"

#define UART_CLK          50000000
#define UART_DEFAULT_BAUD 115200

static unsigned long base_address;
static int wakeup = 0;
static char ret_char;

unsigned long ns16550a_get_base(void)
{
	return base_address;
}

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
	while (!(value & LSR_EMPTY)) {
		ns16550a_delay(100);
		value = readl(base_address + LSR);
	}

	writel(base_address + THR, c);
}

static void ns16550a_puts(char *str)
{
	while (*str != '\0')
		ns16550a_putc(*str++);
}

static char ns16550a_getc(void)
{
	if (readl(base_address + LSR) & DR)
		return readl(base_address + RBR);
	else
		return -1;
}

static void ns16550a_irq_handler(void *data)
{
	char c;

	c = ns16550a_getc();
	if (c == 255)
		return;
	else
		ret_char = c;

	wakeup = 1;
}

static int ns16550a_write(char *buf, unsigned long offset, unsigned int len)
{
	int i;

	for (i = 0; i < len; i++)
		ns16550a_putc(*buf++);

	return 0;
}

static int ns16550a_wake_expr(void *data)
{
	int *wake = (int *)data;

	return *wake == 1;
}

static int ns16550a_read(char *buf, unsigned long offset, unsigned int len,
			 int flag)
{
	if (flag == BLOCK) {
		wait_for_event(&wakeup, ns16550a_wake_expr);
		wakeup = 0;
		*buf++ = ret_char;
	} else if (flag == NONBLOCK) {
		ret_char = ns16550a_getc();
		if (ret_char < 0 || ret_char == 255)
			return 0;
		*buf++ = ret_char;
	}

	return 1;
}

static int __ns16550a_init(unsigned long base)
{
	unsigned int divisor = UART_CLK / (16 * UART_DEFAULT_BAUD);

	base_address = base;

	writel(base_address + LCR, 0x83);

	while (readl(base_address + USR) & 0x1) ;

	writel(base_address + DLH, 0);
	writel(base_address + DLL, divisor);
	writel(base_address + LCR, 0x03);
	writel(base_address + FCR, 0xc7 /*0x01 */ );
	writel(base_address + IER, 0);
	writel(base_address + MCR, (RTS | DTR));

	return 0;
}

int ns16550a_earlycon_init(unsigned long base,
			   struct early_print_device *device)
{
	__ns16550a_init(base);
	device->write = ns16550a_puts;

	return 0;
}

EARLYCON_REGISTER(ns16550a, ns16550a_earlycon_init, "ns16550a");

static const struct driver_ops ns16550a_ops = {
	.write = ns16550a_write,
	.read = ns16550a_read,
};

int ns16550a_init(struct device *dev, void *data)
{
	struct driver *drv;
	int irqs[16], nr_irqs, i;

	print("%s %d base: 0x%lx, len: %d, irq: %d\n", __FUNCTION__, __LINE__,
	      dev->start, dev->len, dev->irqs[0]);

	while (readl(dev->start + USR) & 0x1) ;

	writel(dev->start + IER, 1);

	nr_irqs = get_hwirq(dev, irqs);
	for (i = 0; i < nr_irqs; i++)
		register_device_irq(dev->irq_domain, irqs[i],
				    ns16550a_irq_handler, NULL);

	drv = dev->drv;
	strcpy(dev->name, "UART0");
	strcpy(drv->name, "UART0");
	drv->ops = &ns16550a_ops;

	return 0;
}

DRIVER_REGISTER(ns16550a, ns16550a_init, "ns16550a");
