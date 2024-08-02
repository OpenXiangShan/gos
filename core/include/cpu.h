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

#ifndef __CPU_H
#define __CPU_H

#include <device.h>
#include <asm/ptregs.h>
#include "list.h"
#include "spinlocks.h"
#include "print.h"

#define MAX_CPU_COUNT 64

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
	for (cpu = -1; cpu = cpumask_next(cpu, online_cpu_mask, MAX_CPU_COUNT), cpu < MAX_CPU_COUNT; cpu++)

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
int get_cpu_count(void);
int get_cpu_satp_mode(void);

#endif
