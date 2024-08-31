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

#ifndef __TASK_H
#define __TASK_H

#include <asm/ptregs.h>
#include "list.h"
#include "timer.h"
#include "spinlocks.h"

enum {
	TASK_STATUS_READY = 0,
	TASK_STATUS_SLEEP,
};

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
	void *pgdp;
	int status;
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
int create_user_task(char *name, int (*fn)(void *data), void *data, int cpu,
		     unsigned long stack, unsigned int stack_size, void *pgd);
int do_idle(void *data);
void task_scheduler_enter(struct pt_regs *regs);
void task_scheduler_exit(struct pt_regs *regs);
void schedule(void);
void Sleep(void);
struct task *get_current_task(void);
void set_task_status(struct task *task, int status);
void sleep_to_timeout(int ms);

#endif
