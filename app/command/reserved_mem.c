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
#include "mm.h"

static void reserved_mem_info(unsigned long addr, unsigned int size, void *data)
{
	print("  addr: 0x%lx size: 0x%lx\n", addr, size);
}

static int cmd_reserved_mem_handler(int argc, char *argv[], void *priv)
{
	print("reserved memory info --\n");
	reserved_mem_walk(reserved_mem_info, NULL);

	return 0;
}

static const struct command cmd_reserved_mem = {
	.cmd = "reserved_mem_info",
	.handler = cmd_reserved_mem_handler,
	.priv = NULL,
};

int cmd_reserved_mem_init()
{
	register_command(&cmd_reserved_mem);

	return 0;
}

APP_COMMAND_REGISTER(reserved_mem, cmd_reserved_mem_init);
