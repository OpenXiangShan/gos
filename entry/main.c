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

#include <asm/type.h>
#include <uart.h>
#include <device.h>
#include <print.h>
#include <mm.h>
#include <irq.h>
#include <trap.h>
#include <asm/csr.h>
#include <timer.h>
#include <cpu.h>
#include <percpu.h>
#include <task.h>
#include <devicetree.h>
#include "gos.h"
#include "../app/shell.h"
#include "asm/sbi.h"
#include "virt.h"
#include "user.h"
#include "../app/command.h"

extern const char logo[];

void start_gos(unsigned int hart_id,
	       struct device_init_entry *hw, char *boot_option)
{
	unsigned long start, end;

	start = sbi_get_cpu_cycles();

	early_print_setup(hw);

#ifndef CONFIG_SELECT_VCS
	print(logo);
#endif
	print("complier info:\n");
#ifdef CONFIG_ARCH
	print("    arch : %s\n", CONFIG_ARCH);
#endif
	print("    uname: %s@%s\n", BUILD_USER, CONFIG_UNAME_RELEASE);
	print("    time : %s\n", BUILD_TIME);

	print("Hello, gos!\n");
	print("boot option:\n");
	print("    %s\n", boot_option);

	trap_init();

	mm_init(hw);

	get_cpu_satp_mode();
#ifdef CONFIG_ENABLE_MMU
	paging_init(hw);
#endif
	early_print_setup(hw);

	print("satp:0x%lx\n", read_csr(satp));

	set_test_cmd_ptr(boot_option);

	parse_dtb();

	percpu_init();

	set_online_cpumask(0);

	irq_init();

	irqchip_setup(hw);

	init_timer(hw);

	device_driver_init(hw);

	bringup_secondary_cpus(hw);

	percpu_tasks_init(0);

	vcpu_init();

	user_init();

	end = sbi_get_cpu_cycles();
	print("gos startup success, cost: %d(cycles)\n", end - start);

	enable_local_irq();

	set_print_time(0);

	create_task("shell_init", shell_init, NULL, 0, NULL, 0, NULL);
}
