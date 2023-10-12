#include <sbi_uart.h>
#include <asm/mmio.h>
#include <device.h>
#include "uart_ns16550a.h"

#define UART_CLK          50000000
#define UART_DEFAULT_BAUD 115200

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

void uart_ns16550a_init(unsigned long base, struct sbi_uart_ops *ops)
{
	unsigned int divisor = UART_CLK / (16 * UART_DEFAULT_BAUD);

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
