#include "device.h"
#include "uart.h"
#include "print.h"

void start_guest(struct device_init_entry *entry, char *cmd)
{
	myGuest_uart_init(entry);

	myGuest_print("hello guest os!!\n");
	if (cmd)
		myGuest_print("cmd -- %s\n", cmd);	
}
