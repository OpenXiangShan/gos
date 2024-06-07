#include "uart.h"
#include "asm/mmio.h"
#include "asm/type.h"
#include "uartlite.h"
#include "device.h"
#include "vmap.h"

static unsigned long base_address;

static void uartlite_putc(char c)
{
	while (readb(base_address + UARTLITE_STAT_REG) & UARTLITE_TX_FULL) ;

	writeb(base_address + UARTLITE_TX_FIFO, c);
}

void uart_uartlite_init(unsigned long base, struct myGuest_uart_ops *ops)
{
	base_address = base;

	ops->putc = uartlite_putc;
}

int uartlite_init(unsigned long base, unsigned int len, void *data)
{
	base_address = (unsigned long)ioremap((void *)base, len, NULL);

	return 0;
}

DRIVER_REGISTER(uartlite, uartlite_init, "uartlite");
