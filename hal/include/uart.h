#ifndef _UART_H
#define _UART_H

void uart_putc(char c);
void uart_puts(char *str);
int uart_init(void);

int early_print_setup();

#endif
