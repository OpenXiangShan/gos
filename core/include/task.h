#ifndef __TASK_H
#define __TASK_H

#include <asm/ptregs.h>
#include "list.h"
#include "timer.h"
#include "spinlocks.h"

#define TASK_SCHEDULER_PERIOD 100

struct task {
	char name[128];
	struct list_head list;
	int (*task_fn)(void *data);
	void *data;
	struct task *self;
	unsigned long stack;
	int cpu;
	int id;
	struct pt_regs regs;
};

struct task_ctrl {
	int cpu;
	struct list_head head;
	spinlock_t lock;
};

struct task_scheduler {
	int cpu;
	struct timer_event_info timer_evt;
	struct task *current_task;
	struct pt_regs *irq_regs;
	int period_in_ms;
};

void walk_task_per_cpu(int cpu);
void walk_task_all_cpu(void);
int percpu_tasks_init(int cpu);
int create_task(char *name, int (*fn)(void *data), void *data, int cpu,
		unsigned long stack, unsigned int stack_size, void *pgd);
int do_idle(void *data);
void task_scheduler_enter(struct pt_regs *regs);
void task_scheduler_exit(struct pt_regs *regs);

#endif
