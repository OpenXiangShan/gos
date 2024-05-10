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

extern char dtb_bin[];
extern int mmu_is_on;
extern void do_exception_vector(void);

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
