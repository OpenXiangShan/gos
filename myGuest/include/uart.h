#ifndef	_MYGUEST_UART_H
#define	_MYGUEST_UART_H

#include "device.h"

struct myGuest_uart_ops {
	void (*putc)(char c);
};

void myGuest_uart_putc(char c);
void myGuest_uart_puts(char *str);
int myGuest_uart_init(struct device_init_entry *entry);

#endif
