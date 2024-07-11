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

extern const char logo[];

void start_gos(unsigned int hart_id, struct device_init_entry *hw)
{
	unsigned long start, end;

	start = sbi_get_cpu_cycles();

	early_print_setup(hw);

#ifndef CONFIG_SELECT_VCS
	print(logo);
#endif
	print("complier info:\n");
	print("    arch : %s\n", CONFIG_ARCH);
	print("    uname: %s@%s\n", BUILD_USER, CONFIG_UNAME_RELEASE);
	print("    time : %s\n", BUILD_TIME);

	print("Hello, gos!\n");
	trap_init();

	mm_init(hw);

	get_cpu_satp_mode();
#ifdef CONFIG_ENABLE_MMU
	paging_init(hw);
#endif
	early_print_setup(hw);

	print("satp:0x%lx\n", read_csr(satp));
	parse_dtb();

	percpu_init();

	set_online_cpumask(0);

	irq_init();

	irqchip_setup(hw);

	init_timer(hw);

	device_driver_init(hw);

	bringup_secondary_cpus(hw);

	percpu_tasks_init(0);

	enable_local_irq();

	end = sbi_get_cpu_cycles();
	print("gos startup success, cost: %d(cycles)\n", end - start);

	create_task("shell_init", shell_init, NULL, 0, NULL, 0, NULL);
}
