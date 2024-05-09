#include <sbi_uart.h>
#include <asm/mmio.h>
#include <device.h>
#include "uart_uartlite.h"

static unsigned long base_address;

static void uartlite_putc(char c)
{
	while (readb(base_address + UARTLITE_STAT_REG) & UARTLITE_TX_FULL) ;

	writeb(base_address + UARTLITE_TX_FIFO, c);
}

void uart_uartlite_init(unsigned long base, struct sbi_uart_ops *ops)
{
	base_address = base;

	writeb(base_address + UARTLITE_CTRL_REG, UARTLITE_RST_FIFO);

	ops->putc = uartlite_putc;
}
