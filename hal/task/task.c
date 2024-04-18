#include <asm/type.h>
#include <asm/sbi.h>
#include "task.h"
#include "mm.h"
#include "print.h"
#include "string.h"
#include "percpu.h"
#include "clock.h"
#include "list.h"
#include "cpu.h"

static DEFINE_PER_CPU(struct task_ctrl, tasks);
static DEFINE_PER_CPU(struct task_scheduler, schedulers);

static void task_fn_wrap(void)
{
	int cpu = sbi_get_cpu_id();
	struct task_scheduler *sc = &per_cpu(schedulers, cpu);
	struct task_ctrl *tsk_ctl = &per_cpu(tasks, cpu);
	struct task *cur_task;

	cur_task = sc->current_task;
	if (!cur_task->task_fn)
		return;

	cur_task->task_fn(cur_task->data);

	spin_lock(&tsk_ctl->lock);
	list_del(&cur_task->list);
	sc->current_task = NULL;
	spin_unlock(&tsk_ctl->lock);
}

void walk_task_per_cpu(int cpu)
{
	struct task_ctrl *tsk_ctl = &per_cpu(tasks, cpu);
	struct task *entry;
	int n = 0;

	if (!tsk_ctl)
		return;

	print("==============cpu%d==============\n", cpu);
	spin_lock(&tsk_ctl->lock);
	list_for_each_entry(entry, &tsk_ctl->head, list) {
		print("---> task%d in cpu%d\n", n++, cpu);
		print("name: %s\n", entry->name);
		print("task_fn: 0x%lx\n", entry->task_fn);
		print("stack: 0x%lx\n", entry->stack);
	}
	spin_unlock(&tsk_ctl->lock);
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
		unsigned long stack, unsigned int stack_size)
{
	struct task *new;
	void *p_stack;
	struct task_ctrl *tsk_ctl = &per_cpu(tasks, cpu);

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

	spin_lock(&tsk_ctl->lock);
	list_add_tail(&new->list, &tsk_ctl->head);
	spin_unlock(&tsk_ctl->lock);

	return 0;

free_task:
	mm_free(new, sizeof(struct task));
	return -1;
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

	task = list_entry(list_first(&task_ctl->head), struct task, list);
	list_del(&task->list);
	list_add_tail(&task->list, &task_ctl->head);
	spin_unlock(&task_ctl->lock);

	if (task != ts->current_task) {
		if (!ts->current_task)
			switch_cpu_regs(NULL, &task->regs, ts->irq_regs);
		else
			switch_cpu_regs(&ts->current_task->regs, &task->regs,
					ts->irq_regs);

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

void task_scheduler_enter(struct pt_regs *regs)
{
	int cpu = sbi_get_cpu_id();
	struct task_scheduler *sc = &per_cpu(schedulers, cpu);

	sc->irq_regs = regs;
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
	sc->period_in_ms = TASK_SCHEDULER_PERIOD;

	task_scheduler_clock_event_init(sc);

	return 0;
}
