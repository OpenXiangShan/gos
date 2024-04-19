#include "device.h"
#include "uart.h"
#include "print.h"
#include "mm.h"
#include "string.h"
#include "command.h"

void start_guest(struct device_init_entry *entry, struct run_params *params)
{
	myGuest_uart_init(entry);

	myGuest_print("hello guest os!!\n");

	mm_init(entry);

	paging_init(entry);

	create_devices();

	myGuest_print("guest mmu is on!\n");

	command_init();

	while (1) {
		if (params->busy) {
			do_command(params);
			params->busy = 0;
		}
	}
}
