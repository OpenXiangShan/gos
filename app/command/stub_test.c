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

#include "asm/type.h"
#include "print.h"
#include "device.h"
#include "../command.h"
#include "asm/mmio.h"
#include "asm/ptregs.h"
#include "string.h"
#include "task.h"
#include "list.h"
#include "mm.h"
#include "stub.h"

struct stub_info {
	struct list_head list;
	char name[64];
};

static LIST_HEAD(stub_history);

static void handle_ebreak_stub_handler(struct pt_regs *regs)
{
	print("%s %d scause:0x%lx sepc:0x%lx stval:0x%lx\n",
	      __FUNCTION__, __LINE__, regs->scause, regs->sepc, regs->sbadaddr);
}

static void handle_exception_stub_handler(struct pt_regs *regs)
{
	print("%s %d scause:0x%lx sepc:0x%lx stval:0x%lx\n",
	      __FUNCTION__, __LINE__, regs->scause, regs->sepc, regs->sbadaddr);
}

static void handle_task_scheduler_event_stub_handler(struct pt_regs *regs)
{
	struct task_scheduler *ts = (struct task_scheduler *)regs->a0;
	struct task *task = ts->current_task;

	print("schedule task: %s\n", task->name);
}

static int stub_del(char *name)
{
	struct stub_info *si, *n;

	if (name == NULL) {
		list_for_each_entry_safe(si, n, &stub_history, list) {
			unregister_stub(si->name);
			list_del(&si->list);
		}
		return 0;
	}

	list_for_each_entry_safe(si, n, &stub_history, list) {
		if (!strcmp(si->name, name)) {
			unregister_stub(si->name);
			list_del(&si->list);
			return 0;
		}
	}

	return 0;
}

static void show_stub_info(void)
{
	struct stub_info *s;
	int n = 0;

	list_for_each_entry(s, &stub_history, list) {
		print("%d : %s\n", n++, s->name);
	}
}

static int cmd_stub_handler(int argc, char *argv[], void *priv)
{
	struct stub_info *stub;
	if (argc == 0) {
		print("Please input the stub function name\n");
		return -1;
	}

	if (!strncmp(argv[0], "del", sizeof("del"))) {
		if (argc == 2)
			return stub_del(argv[1]);
		else if (argc == 1)
			return stub_del(NULL);
		else {
			print("Invalid params!\n");
			return -1;
		}
	}
	else if (!strncmp(argv[0], "info", sizeof("info"))) {
		show_stub_info();
		return 0;
	}

	if (register_stub(argv[0], default_stub_handler)) {
		print("insert a stub into %s fail\n", argv[0]);
		return -1;
	}

	stub = (struct stub_info *)mm_alloc(sizeof(struct stub_info));
	if (!stub) {
		print("alloc stub_info fail\n");
		return -1;
	}
	strcpy(stub->name, argv[0]);
	list_add_tail(&stub->list, &stub_history);

	return 0;
}

static int cmd_stub_test_handler(int argc, char *argv[], void *priv)
{
	if (argc == 0) {
		return -1;
	}

	if (!strncmp(argv[0], "handle_exception", sizeof("handle_exception"))) {
		register_handle_exception_stub_handler(handle_exception_stub_handler);
		writel(0x1234, 0);
	} else if (!strncmp(argv[0], "do_ebreak", sizeof("do_ebreak"))) {
		register_ebreak_stub_handler(handle_ebreak_stub_handler);
		__asm__ __volatile("ebreak");
	} else if (!strncmp(argv[0], "task_scheduler_event_handler",
			    sizeof("task_scheduler_event_handler"))) {
		register_stub("task_scheduler_event_handler",
			      handle_task_scheduler_event_stub_handler);
	}

	return 0;
}

static const struct command cmd_stub = {
	.cmd = "stub",
	.handler = cmd_stub_handler,
	.priv = NULL,
};

static const struct command cmd_stub_test = {
	.cmd = "stub_test",
	.handler = cmd_stub_test_handler,
	.priv = NULL,
};

int cmd_stub_test_init()
{
	register_command(&cmd_stub_test);
	register_command(&cmd_stub);

	return 0;
}

APP_COMMAND_REGISTER(stub_test, cmd_stub_test_init);
