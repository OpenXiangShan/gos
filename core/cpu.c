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

#include <asm/sbi.h>
#include <asm/type.h>
#include <asm/ptregs.h>
#include "string.h"
#include "device.h"
#include "irq.h"
#include "print.h"
#include "mm.h"
#include "task.h"
#include "list.h"
#include "spinlocks.h"
#include "cpu.h"
#include "devicetree.h"
#include "../fdt/libfdt.h"

extern char dtb_bin[];
extern int mmu_is_on;
extern void do_exception_vector(void);
extern int pgtable_l4_enabled;
extern int pgtable_l5_enabled;

spinlock_t cpumask_lock __attribute__((section(".data"))) =
    __SPINLOCK_INITIALIZER;

static spinlock_t notifier_lock __attribute__((section(".data"))) =
    __SPINLOCK_INITIALIZER;

unsigned long online_cpu_mask __attribute__((section(".data"))) = 0;

static LIST_HEAD(notifier_list);

extern char secondary_start_sbi[];

static int sbi_cpu_start(unsigned int hartid)
{
	unsigned long boot_addr = (unsigned long)secondary_start_sbi;

	return SBI_CALL_2(SBI_HART_START, hartid, boot_addr);
}

void switch_cpu_regs(struct pt_regs *prev, struct pt_regs *next,
		     struct pt_regs *irq_regs)
{
	if (prev)
		memcpy((char *)prev, (char *)irq_regs, sizeof(struct pt_regs));

	if (irq_regs)
		memcpy((void *)irq_regs, (void *)next, sizeof(struct pt_regs));
}

static void scan_dtb_cpus_cb(void *dtb, int offset, void *data)
{
	int *n = (int *)data;
	(*n)++;
}

static void scan_dtb_cpu_satp_mode(void *dtb, int offset, void *data)
{
	const char *mode;

	mode = fdt_getprop((const void *)dtb, offset, "mmu-type", NULL);
	print("%s -- mode:%s\n", __FUNCTION__, mode);
	if (!strcmp(mode, "riscv,sv39")) {
		pgtable_l4_enabled = 0;
		pgtable_l5_enabled = 0;
	}
	else if (!strcmp(mode, "riscv,sv48")) {
		pgtable_l4_enabled = 1;
		pgtable_l5_enabled = 0;
	}
	else if (!strcmp(mode, "riscv,sv57")) {
		pgtable_l4_enabled = 1;
		pgtable_l5_enabled = 1;
	}
	else {
		print("Unsupport satp mode.. default use sv39...\n");
		pgtable_l4_enabled = 0;
		pgtable_l5_enabled = 0;
	}
}

int get_cpu_count()
{
	void *dtb_ptr;
	int ret = 0;

	if (mmu_is_on)
		dtb_ptr = (void *)FIXMAP_DTB_START;
	else
		dtb_ptr = (void *)dtb_bin;

	dtb_scan_cpus(dtb_ptr, scan_dtb_cpus_cb, &ret);

	return ret;
}

int get_cpu_satp_mode()
{
	void *dtb_ptr;

	if (mmu_is_on)
		return -1;

	dtb_ptr = (void *)dtb_bin;

	dtb_scan_cpus(dtb_ptr, scan_dtb_cpu_satp_mode, NULL);

	return 0;
}

void bringup_secondary_cpus(struct device_init_entry *hw)
{
	int i, cpu_count;

	cpu_count = get_cpu_count();

	for (i = 1; i < cpu_count; i++) {
		if (!sbi_cpu_start(i)) {
			set_online_cpumask(i);
		}
	}

	print("bringup %d cpus success\n", cpu_count);
}

void cpu_regs_init(struct pt_regs *regs)
{
	if (!regs)
		return;

	memset((char *)regs, 0, sizeof(struct pt_regs));

	regs->sstatus = SR_SPP | SR_SPIE;
}

static int cpu_hotplug_notify_callback(int cpu)
{
	struct cpu_hotplug_notifier *n;

	spin_lock(&notifier_lock);

	list_for_each_entry(n, &notifier_list, list) {
		if (n->startup)
			n->startup(n, cpu);
	}

	spin_unlock(&notifier_lock);

	return 0;
}

void secondary_cpus_init(unsigned int hart_id, unsigned long stack)
{
	write_csr(sie, -1);

	enable_mmu(1);

	cpu_hotplug_notify_callback(hart_id);

	percpu_tasks_init(hart_id);

	create_task("idle", do_idle, NULL, hart_id, NULL, 0, NULL);

	enable_local_irq();
}

int cpu_hotplug_notify_register(struct cpu_hotplug_notifier *notifier)
{
	struct cpu_hotplug_notifier *n;
	int cpu;

	spin_lock(&notifier_lock);

	list_for_each_entry(n, &notifier_list, list) {
		if (n == notifier) {
			spin_unlock(&notifier_lock);
			return -1;
		}
	}

	list_add(&notifier->list, &notifier_list);

	spin_unlock(&notifier_lock);

	for_each_online_cpu(cpu) {
		if (notifier->startup)
			notifier->startup(notifier, cpu);
	}

	return 0;
}

int cpu_hotplug_init(int cpu)
{
	set_online_cpumask(cpu);

	return 0;
}
