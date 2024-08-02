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
#include <print.h>
#include <device.h>
#include "../command.h"
#include "task.h"
#include "spinlocks.h"
#include "print.h"

static spinlock_t _lock;
static int per_cpu_info[2];

int spinlock_test(void *data)
{
	int i, test_cnt;
	int *info = (int *)data;
	int cpu = sbi_get_cpu_id();

	for (test_cnt = 0; test_cnt < 64; test_cnt++) {
		spin_lock(&_lock);
		print("cpu%d: ", cpu);
		for (i = 0; i < 64; i++)
			print("%d", *info);
		print("\n");
		spin_unlock(&_lock);
	}

	return 0;
}

static int cmd_spinlock_test_handler(int argc, char *argv[], void *priv)
{
	int i = 0;

	for (i = 0; i < 2; i++) {
		per_cpu_info[i] = i;
		create_task("spinlock_test", spinlock_test, &per_cpu_info[i], i,
			    NULL, 0, NULL);
	}

	return 0;
}

static const struct command cmd_spinlock_test = {
	.cmd = "spinlock_test",
	.handler = cmd_spinlock_test_handler,
	.priv = NULL,
};

int cmd_spinlock_test_init()
{
	__SPINLOCK_INIT(&_lock);

	register_command(&cmd_spinlock_test);

	return 0;
}

APP_COMMAND_REGISTER(hello, cmd_spinlock_test_init);
