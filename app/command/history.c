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

static int cmd_history_handler(int argc, char *argv[], void *pirv)
{
	int n = 0;
	struct cmd_history *history = command_get_history();
	struct cmd_name *cmd;

	if (!history)
		return -1;

	for_each_history_cmd(cmd, history, n)
	    print("%s\n", cmd->name);

	return 0;
}

static const struct command cmd_history = {
	.cmd = "history",
	.handler = cmd_history_handler,
	.priv = NULL,
};

int cmd_history_init()
{
	register_command(&cmd_history);

	return 0;
}

APP_COMMAND_REGISTER(history, cmd_history_init);
