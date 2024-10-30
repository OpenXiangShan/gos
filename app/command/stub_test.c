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
#include "stub.h"
#include "string.h"
#include "task.h"

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

static int cmd_stub_test_handler(int argc, char *argv[], void *priv)
{
	if (argc == 0) {
		print("Please input the stub function name\n");
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
	else {
		if (register_stub(argv[0], default_stub_handler)) {
			print("insert a stub into %s fail\n", argv[0]);
		}
	}

	return 0;
}

static const struct command cmd_stub_test = {
	.cmd = "stub_test",
	.handler = cmd_stub_test_handler,
	.priv = NULL,
};

int cmd_stub_test_init()
{
	register_command(&cmd_stub_test);

	return 0;
}

APP_COMMAND_REGISTER(stub_test, cmd_stub_test_init);
