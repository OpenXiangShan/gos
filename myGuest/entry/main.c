/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
