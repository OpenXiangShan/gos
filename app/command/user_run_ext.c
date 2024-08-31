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
#include <print.h>
#include <device.h>
#include "../command.h"
#include "user.h"
#include "task.h"
#include "string.h"
#include "mm.h"
#include "cpu.h"

static void Usage_run_at()
{
	print
	    ("user_run_ext_at cpu=[cpu id] userid=[userid] [cmd] [params1] [params2] ...\n");
}

static void Usage()
{
	print("user_run_ext [cmd] [params1] [params2] ...\n");
	print("user_run_ext cpu=[cpu id] [cmd] [params1] [params2] ...\n");
}

static int user_start(void *data)
{
	struct user *user;
	struct user_run_params *params = (struct user_run_params *)data;

	user = user_create_force();
	if (!user)
		return -1;

	user_mode_run(user, params);

	mm_free((void *)params, sizeof(struct user_run_params));

	return 0;
}

static int cmd_user_run_ext_handler(int argc, char *argv[], void *priv)
{
	int i, cpu = 0, bg = 0, offset = 0;
	struct user_run_params *params;

	if (argc == 0) {
		Usage();
		print("invalid input param.\n");
		return -1;
	} else if (argc > 16) {
		print("Too much params...\n");
		return -1;
	}

	if (!strncmp(argv[offset], "cpu=", sizeof("cpu=") - 1)) {
		char *tmp = argv[offset];
		tmp += sizeof("cpu=") - 1;
		cpu = atoi(tmp);
		offset += 1;
	}

	if (!strncmp(argv[offset], "bg=", sizeof("bg=") - 1)) {
		char *tmp = argv[offset];
		tmp += sizeof("bg=") - 1;
		bg = atoi(tmp);
		offset += 1;
	}

	params =
	    (struct user_run_params *)mm_alloc(sizeof(struct user_run_params));
	if (!params) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	params->bg = bg;
	strcpy(params->command, argv[offset]);
	params->argc = argc - 1 - offset;
	for (i = 0; i < params->argc; i++)
		strcpy(params->argv[i], argv[i + 1 + offset]);

	print("user_run_ext on cpu%d command:%s bg:%d\n", cpu, params->command,
	      params->bg);
	if (create_user_task
	    ("user", user_start, (void *)params, cpu, NULL, 0, NULL)) {
		int n;
		print("create task on cpu%d failed...\n", cpu);
		print("online cpu info:\n");
		for_each_online_cpu(n) {
			print("  cpu%d\n", n);
		}
	}

	return 0;
}

static const struct command cmd_user_run_ext = {
	.cmd = "user_run_ext",
	.handler = cmd_user_run_ext_handler,
	.priv = NULL,
};

static int user_start_at(void *data)
{
	struct user *user;
	struct user_run_params *params = (struct user_run_params *)data;

	print("%s -- cpu:%d userid:%d\n",
	      __FUNCTION__, params->cpu, params->userid);

	user = get_user(params->userid, params->cpu);
	if (!user) {
		print("There is no vm(userid=%d) is running on cpu%d\n",
		      params->userid, params->cpu);
		return -1;
	}

	user_mode_run(user, params);

	mm_free((void *)params, sizeof(struct user_run_params));

	return 0;
}

static int cmd_user_run_ext_at_handler(int argc, char *argv[], void *priv)
{
	int i, cpu = 0, userid = 1;
	struct user_run_params *params;

	if (argc < 3) {
		Usage_run_at();
		print("invalid input param.\n");
		return -1;
	} else if (argc > 16) {
		print("Too much params...\n");
		return -1;
	}

	if (!strncmp(argv[0], "cpu=", sizeof("cpu=") - 1)) {
		char *tmp = argv[0];
		tmp += sizeof("cpu=") - 1;
		cpu = atoi(tmp);
	} else {
		print("Invalid params...\n");
		Usage_run_at();
	}

	if (!strncmp(argv[1], "userid=", sizeof("userid=") - 1)) {
		char *tmp = argv[1];
		tmp += sizeof("userid=") - 1;
		userid = atoi(tmp);
	} else {
		print("Invalid params...\n");
		Usage_run_at();
	}

	params =
	    (struct user_run_params *)mm_alloc(sizeof(struct user_run_params));
	if (!params) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	strcpy(params->command, argv[2]);
	params->argc = argc - 3;
	for (i = 0; i < params->argc; i++)
		strcpy(params->argv[i], argv[i + 3]);
	params->cpu = cpu;
	params->userid = userid;

	if (create_user_task
	    ("user", user_start_at, (void *)params, 0, NULL, 0, NULL)) {
		int n;
		print("create task on cpu%d failed...\n", cpu);
		print("online cpu info:\n");
		for_each_online_cpu(n) {
			print("  cpu%d\n", n);
		}
	}

	return 0;
}

static const struct command cmd_user_run_ext_at = {
	.cmd = "user_run_ext_at",
	.handler = cmd_user_run_ext_at_handler,
	.priv = NULL,
};

int cmd_user_run_ext_init()
{
	register_command(&cmd_user_run_ext);
	register_command(&cmd_user_run_ext_at);

	return 0;
}

APP_COMMAND_REGISTER(user_run_ext, cmd_user_run_ext_init);
