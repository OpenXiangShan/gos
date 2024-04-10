#include "uart.h"
#include "string.h"
#include "../drivers/qemu-8250.h"

static struct myGuest_uart_ops ops = { 0 };

void myGuest_uart_putc(char c)
{
	if (c == '\n')
		ops.putc('\r');

	ops.putc(c);
}

void myGuest_uart_puts(char *str)
{
	while (*str != '\0')
		myGuest_uart_putc(*str++);
}

int myGuest_uart_init(struct device_init_entry *entry)
{
	struct device_init_entry *device_entry = entry;

	while (strncmp(device_entry->compatible, "THE END", sizeof("THE END"))) {
		if (!strncmp(device_entry->compatible, "qemu-8250", 128)) {
			uart_qemu_8250_init(device_entry->start, &ops);
			return 0;
		}
		device_entry++;
	}

	return -1;
}
