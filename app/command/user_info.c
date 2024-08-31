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
#include "string.h"

static void Usage(void)
{
	print("print user info on cpus:\n");
	print("user_info [cpu] -- print user info on [cpu]\n");
	print("user_info       -- print user info on all cpus\n");
}

static int cmd_user_info_handler(int argc, char *argv[], void *priv)
{
	int cpu;

	if (argc == 1) {
		cpu = atoi(argv[0]);
		dump_user_info_on_cpu(cpu);
	} else if (argc == 0) {
		dump_user_info_on_all_cpu();
	} else
		Usage();

	return 0;
}

static const struct command cmd_user_info = {
	.cmd = "user_info",
	.handler = cmd_user_info_handler,
	.priv = NULL,
};

int cmd_user_info_init()
{
	register_command(&cmd_user_info);

	return 0;
}

APP_COMMAND_REGISTER(user_info, cmd_user_info_init);
