#include "device.h"
#include "uart.h"
#include "print.h"
#include "mm.h"

void start_guest(struct device_init_entry *entry, char *cmd)
{
	myGuest_uart_init(entry);

	//while (1) {
	myGuest_print("hello guest os!!\n");
	if (cmd)
		myGuest_print("cmd -- %s\n", cmd);
	//}

	mm_init(entry);

	paging_init(entry);

	create_devices();

	myGuest_print("guest mmu is on!\n");
}
