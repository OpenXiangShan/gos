#ifndef	_UART_H
#define	_UART_H

#include "../sbi/sbi_trap.h"

struct sbi_uart_ops {
	void (*putc)(char c);
};

void sbi_uart_putc(char c);
void sbi_uart_puts(char *str);
int sbi_uart_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx);

#endif
