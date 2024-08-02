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

static int cmd_ls_handler(int argc, char *argv[], void *priv)
{
	walk_and_print_command();

	return 0;
}

static const struct command cmd_ls = {
	.cmd = "ls",
	.handler = cmd_ls_handler,
	.priv = NULL,
};

int cmd_ls_init()
{
	register_command(&cmd_ls);

	return 0;
}

APP_COMMAND_REGISTER(ls, cmd_ls_init);
