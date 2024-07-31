#include "device.h"
#include "uart.h"
#include "print.h"
#include "mm.h"
#include "string.h"
#include "command.h"
#include "trap.h"
#include "asm/asm-irq.h"
#include "sleep.h"

void start_guest(struct device_init_entry *entry, struct run_params *params)
{
	myGuest_uart_init(entry);
	myGuest_print_init(params->vmid, params->cpu, params->bg);

	myGuest_print("hello guest os!!\n");

	scheduler_early_init(entry);

	trap_init();

	mm_init(entry);

	paging_init(entry);

	create_devices();

	myGuest_print("guest mmu is on!\n");
	print("satp:0x%lx\n", read_csr(satp));

	command_init();

	__enable_local_irq();

	params->ready = 1;
	while (1) {
		if (params->busy) {
			do_command(params);
			params->busy = 0;
		}
	}
}
