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

#include "../../command.h"
#include "clock.h"
#include "spinlocks.h"
#include "task.h"
#include "vmap.h"
#include <asm/type.h>
#include <device.h>
#include <print.h>
#include <string.h>

#define MAX_CPU 32

static int cnt[MAX_CPU];

static int task_args[MAX_CPU];

static int condition = 0;

static int nr_cpu = 2;

static int nr_repeat = 100000;

static spinlock_t _lock;

static int do_cyclic_cas(void *data)
{
	int thread_id = *((int *)data);
	print("Thread %d started\n", thread_id);

	int i, ret;
	int expected = thread_id;
	int expected_origin = thread_id;
	int disired = (thread_id + 1) % nr_cpu;
	int start = get_system_tick();

	for (i = 0; i < nr_repeat; i++) {
repeat:
		ret =
		    __atomic_compare_exchange_n(&condition, &expected, disired,
						0, __ATOMIC_SEQ_CST,
						__ATOMIC_SEQ_CST);
		if (ret == 0) {
			expected = expected_origin;
			goto repeat;
		} else {
			cnt[thread_id]++;
		}
	}

	spin_lock(&_lock);
	print("Thread %d Cnt: %d Ticks: %d \n", thread_id, cnt[thread_id],
	      get_system_tick() - start);
	spin_unlock(&_lock);
	return 0;
}

static int cmd_cyclic_cas_handler(int argc, char *argv[], void *priv)
{
	print(" Cyclic CAS TEST \n");

	char *names[] = { "cpu0", "cpu1", "cpu2", "cpu3",
		"cpu4", "cpu5", "cpu6", "cpu7"
	};

	int i;

	if (argc > 0) {
		nr_cpu = atoi(argv[0]);
	}
	if (argc > 1) {
		nr_repeat = atoi(argv[1]);
	}

	for (i = 0; i < nr_cpu; i++) {
		cnt[i] = 0;
		task_args[i] = i;
		create_task(names[i], do_cyclic_cas, &task_args[i], i, NULL, 0,
			    NULL);
	}

	return 0;
}

static const struct command cmd_cyclic_cas = {
	.cmd = "cycas",
	.handler = cmd_cyclic_cas_handler,
	.priv = NULL,
};

int cmd_cyclic_cas_init()
{
	register_command(&cmd_cyclic_cas);

	return 0;
}

APP_COMMAND_REGISTER(cyclic_cas, cmd_cyclic_cas_init);
