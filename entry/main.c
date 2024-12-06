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
#include "uart.h"
#include "ipi.h"
#include "fs.h"
#include "../app/command.h"

extern const char logo[];

void start_gos(unsigned int hart_id,
	       struct device_init_entry *hw, char *boot_option)
{
	unsigned long start, end;

	start = sbi_get_cpu_cycles();

	early_print_setup(hw);

#ifdef CONFIG_PRINT_LOGO
	print(logo);
#endif
	print("complier info:\n");
#ifdef CONFIG_ARCH
	print("    arch : %s\n", CONFIG_ARCH);
#endif
	print("    uname: %s@%s\n", BUILD_USER, CONFIG_UNAME_RELEASE);
	print("    time : %s\n", BUILD_TIME);
	print("    git-info: %s\n", BUILD_GIT_INFO);

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

	init_timer(hw);

	irqchip_setup(hw);

	device_driver_init(hw);

#ifdef CONFIG_ENABLE_FS
	fs_init();
#endif
	bringup_secondary_cpus(hw);

	percpu_tasks_init(0);

	ipi_percpu_init();

#ifdef CONFIG_VIRT
	vcpu_init();
#endif

#ifdef CONFIG_USER
	user_init();
#endif
	end = sbi_get_cpu_cycles();
	print("gos startup success, cost: %d(cycles)\n", end - start);

#ifdef CONFIG_ENABLE_INIT_FS
	mount_init_fs();
#endif
	set_print_time(0);

	create_task("shell_init", shell_init, NULL, 0, NULL, 0, NULL);

	enable_local_irq();
}
