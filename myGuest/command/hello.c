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

#include "command.h"
#include "print.h"
#include "asm/type.h"

static int cmd_hello_handler(int argc, char *argv[], void *priv)
{
	print("Hello MyGuest!! argc:%d\n", argc);

	for (int i = 0; i < argc; i++)
		print("cmd%d: %s\n", i, argv[i]);

        print("TEST PASS\n");

	return 0;
}

static const struct command cmd_hello = {
	.cmd = "hello",
	.handler = cmd_hello_handler,
	.priv = NULL,
};

int cmd_hello_init()
{
	register_command(&cmd_hello);

	return 0;
}

APP_COMMAND_REGISTER(hello, cmd_hello_init);
