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
#include "asm/sbi.h"

static int cmd_hello_handler(int argc, char *argv[], void *priv)
{
	print("Hello Bosc gos Shell!!! cpu:%d\n", sbi_get_cpu_id());
	return TEST_PASS;
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
