#include "uart.h"
#include "asm/mmio.h"
#include "asm/type.h"
#include "qemu-8250.h"
#include "device.h"
#include "vmap.h"

static unsigned long base_address;

static void qemu_8250_putc(char c)
{
	while ((readb(base_address + UART_LSR) & UART_LSR_EMPTY) == 0) ;

	writeb(base_address + UART_DAT, c);
}

void uart_qemu_8250_init(unsigned long base, struct myGuest_uart_ops *ops)
{
	base_address = base;

	ops->putc = qemu_8250_putc;
}

int qemu_8250_init(unsigned long base, unsigned int len, void *data)
{
	base_address = (unsigned long)ioremap((void *)base, len, NULL);

	return 0;
}

DRIVER_REGISTER(qemu_8250, qemu_8250_init, "qemu-8250");
