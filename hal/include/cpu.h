#ifndef __CPU_H
#define __CPU_H

#include <device.h>
#include <asm/ptregs.h>
#include "list.h"
#include "spinlocks.h"
#include "print.h"

#define MAX_CPU_COUNT 8

extern unsigned long online_cpu_mask;
extern spinlock_t cpumask_lock;

struct riscv_hart_info {
	int hart_num;
};

static inline int cpumask_next(int cpu, unsigned long mask, int max)
{
	while ((!((mask >> cpu) & 1UL)) && (cpu < max))
		cpu = cpu + 1;

	return cpu;
}

static inline void clear_online_cpumask(int cpu)
{
	irq_flags_t flags;

	spin_lock_irqsave(&cpumask_lock, flags);
	online_cpu_mask &= ~(1UL << cpu);
	spin_unlock_irqrestore(&cpumask_lock, flags);
}

static inline void set_online_cpumask(int cpu)
{
	irq_flags_t flags;

	spin_lock_irqsave(&cpumask_lock, flags);
	online_cpu_mask |= 1UL << cpu;
	spin_unlock_irqrestore(&cpumask_lock, flags);
}

#define cpu_is_online(cpu) ((online_cpu_mask >> cpu) & 1UL)

#define for_each_online_cpu(cpu) \
	for (cpu = -1; cpu = cpumask_next(cpu, online_cpu_mask, MAX_CPU_COUNT), cpu < MAX_CPU_COUNT;)

struct cpu_hotplug_notifier {
	struct list_head list;
	int (*startup)(struct cpu_hotplug_notifier * notifier, int cpu);
	int (*teardown)(struct cpu_hotplug_notifier * notifier, int cpu);
};

void switch_cpu_regs(struct pt_regs *prev, struct pt_regs *next,
		     struct pt_regs *irq_regs);
void bringup_secondary_cpus(struct device_init_entry *hw);
int cpu_hotplug_notify_register(struct cpu_hotplug_notifier *notifier);
int cpu_hotplug_init(int cpu);
void cpu_regs_init(struct pt_regs *regs);

#endif
