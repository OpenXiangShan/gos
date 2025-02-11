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
#include <asm/sbi.h>
#include <asm/csr.h>
#include "task.h"
#include "mm.h"
#include "print.h"
#include "string.h"
#include "percpu.h"
#include "clock.h"
#include "list.h"
#include "cpu.h"
#include "asm/tlbflush.h"
#include "spinlocks.h"
#include "gos.h"
#include "timer.h"
#include "irq.h"

extern int mmu_is_on;
static DEFINE_PER_CPU(struct task_ctrl, tasks);
static DEFINE_PER_CPU(struct task_scheduler, schedulers);
static unsigned long task_id_bitmap = 0;
static spinlock_t task_id_lock = __SPINLOCK_INITIALIZER;

static int alloc_task_id(void)
{
	unsigned long bitmap;
	int pos = 0;

	spin_lock(&task_id_lock);
	bitmap = task_id_bitmap;
	while (bitmap & 0x01) {
		if (pos == 64)
			return -1;
		bitmap = bitmap >> 1;
		pos++;
	}

	task_id_bitmap |= (1UL << pos);
	spin_unlock(&task_id_lock);

	return pos;
}
static void free_task_id(int id)
{
	spin_lock(&task_id_lock);
	task_id_bitmap &= ~(1UL << id);
	spin_unlock(&task_id_lock);
}

static void task_fn_wrap(void)
{
	int cpu = sbi_get_cpu_id();
	struct task_scheduler *sc = &per_cpu(schedulers, cpu);
	struct task_ctrl *tsk_ctl = &per_cpu(tasks, cpu);
	struct task *cur_task;

	cur_task = sc->current_task;
	if (!cur_task || !cur_task->task_fn)
		return;

	cur_task->task_fn(cur_task->data);

	spin_lock(&tsk_ctl->lock);
	list_del(&cur_task->list);
	free_task_id(cur_task->id);
	sc->current_task = NULL;
	spin_unlock(&tsk_ctl->lock);
}

void walk_task_per_cpu(int cpu)
{
	struct task_ctrl *tsk_ctl = &per_cpu(tasks, cpu);
	struct task *entry;
	int n = 0;
	int flags;

	if (!tsk_ctl)
		return;

	print("==============cpu%d==============\n", cpu);
	spin_lock_irqsave(&tsk_ctl->lock, flags);
	list_for_each_entry(entry, &tsk_ctl->head, list) {
		print("---> task%d in cpu%d\n", n++, cpu);
		print("name: %s\n", entry->name);
		print("task_fn: 0x%lx\n", entry->task_fn);
		print("stack: 0x%lx\n", entry->stack);
		print("task_id: %d\n", entry->id);
		if (entry->status == TASK_STATUS_READY)
			print("status: Ready\n");
		else if (entry->status == TASK_STATUS_SLEEP)
			print("status: Sleep\n");
	}
	spin_unlock_irqrestore(&tsk_ctl->lock, flags);
	print("\n");
}

void walk_task_all_cpu(void)
{
	int cpu;

	for_each_online_cpu(cpu) {
		walk_task_per_cpu(cpu);
	}
}

int create_task(char *name, int (*fn)(void *data), void *data, int cpu,
		unsigned long stack, unsigned int stack_size, void *pgd)
{
	struct task *new;
	void *p_stack;
	struct task_ctrl *tsk_ctl;
	int online_cpu;

#ifndef CONFIG_ENABLE_MULTI_TASK
	enable_local_irq();
	return fn(data);
#endif
	for_each_online_cpu(online_cpu) {
		if (online_cpu == cpu)
			goto continue_to_run;
	}

	return -1;
continue_to_run:
	tsk_ctl = &per_cpu(tasks, cpu);
	if (!tsk_ctl)
		return -1;

	new = mm_alloc(sizeof(struct task));
	if (!new) {
		print("%s -- Out of memory\n");
		return -1;
	}

	memset((char *)new, 0, sizeof(struct task));

	strcpy(new->name, name);
	new->task_fn = fn;
	new->data = data;
	new->cpu = cpu;
	new->id = alloc_task_id();
	new->status = TASK_STATUS_READY;

	if (stack)
		new->stack = stack + stack_size;
	else {
		p_stack = mm_alloc(4096);
		if (!p_stack) {
			print("%s -- Out of memory\n", __FUNCTION__);
			goto free_task;
		}
		new->stack = (unsigned long)p_stack + 4096;
	}

	cpu_regs_init(&new->regs);
	new->regs.sepc = (unsigned long)task_fn_wrap;
	new->regs.ra = (unsigned long)task_fn_wrap;
	new->regs.sp = (unsigned long)new->stack;

	/* Enable FPU in S-mode task context accordingly */
	if (read_csr(sstatus) & SR_FS)
		new->regs.sstatus |= SR_FS;

#if CONFIG_ENABLE_VECTOR
	if (read_csr(sstatus) & SR_VS)
		new->regs.sstatus |= SR_VS;
#endif

	if (mmu_is_on) {
		if (pgd) {
			new->regs.satp = (((unsigned long)pgd) >> PAGE_SHIFT) |
						SATP_MODE |
						(((unsigned long)new->id) << SATP_ASID_SHIFT);
			new->pgdp = pgd;
		}
		else {
			new->regs.satp = (get_default_pgd() >> PAGE_SHIFT) |
						SATP_MODE |
						(((unsigned long)new->id) << SATP_ASID_SHIFT);
			new->pgdp = (void *)get_default_pgd();
		}
	}

	spin_lock(&tsk_ctl->lock);
	list_add_tail(&new->list, &tsk_ctl->head);
	spin_unlock(&tsk_ctl->lock);

	return 0;

free_task:
	mm_free(new, sizeof(struct task));
	return -1;
}

int create_user_task(char *name, int (*fn)(void *data), void *data, int cpu,
		     unsigned long stack, unsigned int stack_size, void *pgd)
{
	void *default_pgd = (void *)phy_to_virt(get_default_pgd());
	void *pgdp;

	if (!pgd) {
		pgdp = mm_alloc(4096);
		if (!pgdp)
			return -1;
	}
	else
		pgdp = (void *)phy_to_virt((unsigned long)pgd);

	memcpy((char *)pgdp, (char *)default_pgd, 4096);

	return create_task(name, fn, data, cpu, stack, stack_size, (void *)virt_to_phy(pgdp));
}

static void switch_mm(struct task *task)
{
	local_flush_tlb_all();
}

static void task_scheduler_event_handler(void *data)
{
	struct task_scheduler *ts = (struct task_scheduler *)data;
	int cpu = sbi_get_cpu_id();
	struct task_ctrl *task_ctl = &per_cpu(tasks, cpu);
	struct task *task;

	if (!ts)
		return;

	spin_lock(&task_ctl->lock);
	if (list_empty(&task_ctl->head)) {
		spin_unlock(&task_ctl->lock);
		ts->timer_evt.restart = 0;
		return;
	}

again:
	task = list_entry(list_first(&task_ctl->head), struct task, list);
	list_del(&task->list);
	list_add_tail(&task->list, &task_ctl->head);

	if (task->status == TASK_STATUS_SLEEP)
		goto again;
	if (!strncmp(task->name, "idle", sizeof("idle"))) {
		if (list_entry(list_first(&task_ctl->head), struct task, list) != task)
			goto again;
	}
	spin_unlock(&task_ctl->lock);

	if (task != ts->current_task) {
		if (!ts->current_task)
			switch_cpu_regs(NULL, &task->regs, ts->irq_regs);
		else
			switch_cpu_regs(&ts->current_task->regs, &task->regs,
					ts->irq_regs);

		switch_mm(task);

		ts->current_task = task;
	}
}

static int task_scheduler_clock_event_init(struct task_scheduler *ts)
{
	ts->timer_evt.data = (void *)ts;
	ts->timer_evt.expiry_time = get_system_time() + ts->period_in_ms;
	ts->timer_evt.restart = 1;
	ts->timer_evt.handler = task_scheduler_event_handler;
	ts->timer_evt.period = ts->period_in_ms;

	register_timer_event(&ts->timer_evt, ts->cpu);

	return 0;
}

static void _schedule(int status)
{
	int cpu = sbi_get_cpu_id();
	int irq_is_on = local_irq_is_on();
	struct task_scheduler *sc = &per_cpu(schedulers, cpu);
	struct task_ctrl *task_ctl = &per_cpu(tasks, cpu);
	struct task *task, *task_tmp;

	if (irq_is_on)
		disable_local_irq();
#if 0
	if (sc->current_task && !strncmp(sc->current_task->name, "idle", sizeof("idle"))) {
		if (irq_is_on)
			enable_local_irq();
		return;
	}
#endif
	if (sc->current_task) {
		sc->current_task->status = status;
		spin_lock(&task_ctl->lock);
		list_for_each_entry_safe(task, task_tmp, &task_ctl->head, list) {
			if (sc->current_task == task) {
				list_del(&task->list);
				break;
			}
		}
		list_add_tail(&task->list, &task_ctl->head);
		spin_unlock(&task_ctl->lock);
	}

	sc->timer_evt.expiry_time = get_system_time();
	clock_set_next_event(sc->timer_evt.expiry_time);

	if (irq_is_on)
		enable_local_irq();
}

void schedule(void)
{
	_schedule(TASK_STATUS_READY);
}

void Sleep(void)
{
	_schedule(TASK_STATUS_SLEEP);
}

void set_task_status(struct task *task, int status)
{
	disable_local_irq();
	if (!strncmp(task->name, "idle", sizeof("idle"))) {
		enable_local_irq();
		return;
	}
	task->status = status;
	enable_local_irq();
}

static void task_sleep_timer(void *data)
{
	struct task *task = (struct task *)data;

	set_task_status(task, TASK_STATUS_READY);
}

void sleep_to_timeout(int ms)
{
	int cpu = sbi_get_cpu_id();
	struct task_scheduler *sc = &per_cpu(schedulers, cpu);

	disable_local_irq();
	set_timer(ms, task_sleep_timer, sc->current_task);
	Sleep();
	enable_local_irq();
}

void task_scheduler_enter(struct pt_regs *regs)
{
	int cpu = sbi_get_cpu_id();
	struct task_scheduler *sc = &per_cpu(schedulers, cpu);

	sc->irq_regs = regs;
}

struct task *get_current_task(void)
{
	int cpu = sbi_get_cpu_id();
	struct task_scheduler *sc = &per_cpu(schedulers, cpu);

	if (!sc)
		return NULL;

	return sc->current_task;
}

void task_scheduler_exit(struct pt_regs *regs)
{
}

int percpu_tasks_init(int cpu)
{
	struct task_ctrl *tsk = &per_cpu(tasks, cpu);
	struct task_scheduler *sc = &per_cpu(schedulers, cpu);

	__SPINLOCK_INIT(&tsk->lock);
	INIT_LIST_HEAD(&tsk->head);
	tsk->cpu = cpu;

	sc->cpu = cpu;
	sc->current_task = NULL;
#ifdef CONFIG_TASK_SCHEDULER_PERIOD
	sc->period_in_ms = CONFIG_TASK_SCHEDULER_PERIOD;
#endif
	task_scheduler_clock_event_init(sc);

	return 0;
}
