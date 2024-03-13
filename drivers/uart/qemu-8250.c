#include <asm/mmio.h>
#include <device.h>
#include <print.h>
#include <asm/type.h>
#include <event.h>
#include <string.h>
#include "qemu-8250.h"

static unsigned long base_address;
static int wakeup = 0;

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

#define UART_DEFAULT_BAUD  115200
static int qemu_8250_init(unsigned long base)
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

	return 0;
}

static void qemu_8250_irq_handler(void *data)
{
	wakeup = 1;
}

static int qemu_8250_write(char *buf, unsigned long offset, unsigned int len)
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

static int qemu_8250_read(char *buf, unsigned long offset, unsigned int len,
			  int flag)
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

	print("%s -- base: 0x%x, len: %d, nr_irqs:%d irq: %d\n", __FUNCTION__,
	      dev->start, dev->len, dev->irq_num, dev->irqs[0]);

	writeb(dev->start + UART_IER, 1);

	nr_irqs = get_hwirq(dev, irqs);
	for (i = 0; i < nr_irqs; i++)
		register_device_irq(dev->irq_domain, irqs[i],
				    qemu_8250_irq_handler, NULL);

	drv = dev->drv;
	strcpy(dev->name, "UART0");
	strcpy(drv->name, "UART0");
	drv->ops = &qemu_8250_ops;

	return 0;
}

DRIVER_REGISTER(qemu_8250, qemu_8250_driver_init, "qemu-8250");

int qemu_8250_earlycon_init(unsigned long base,
			    struct early_print_device *device)
{
	qemu_8250_init(base);
	device->write = qemu_8250_puts;

	return 0;
}

EARLYCON_REGISTER(qemu_8250, qemu_8250_earlycon_init, "qemu-8250");
