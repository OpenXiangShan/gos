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
#include "string.h"

static unsigned long start;
static unsigned int max_len;

static void find_max_unused_mem_block(unsigned long addr, unsigned int len, void *data)
{
	if (len > max_len) {
		max_len = len;
		start = addr;
	}
}

static int cmd_mem_walk_handler(int argc, char *argv[], void *priv)
{
	unsigned long addr;

	start = 0;
	max_len = 0;

	unused_mem_walk(find_max_unused_mem_block, NULL);
	addr = (unsigned long)mm_alloc_fix(start, max_len);
	if (!addr) {
		print("mm alloc from 0x%lx failed\n", max_len, start);
		return -1;
	}

	print("alloc %d memory from 0x%lx success!!\n", max_len, start);

	strcpy((char *)addr, "mem walk test pass!!\n");
	print("%s", addr);

	mm_free((void *)addr, max_len);

	return 0;
}

static const struct command cmd_mem_walk = {
	.cmd = "mem_walk",
	.handler = cmd_mem_walk_handler,
	.priv = NULL,
};

int cmd_mem_walk_init()
{
	register_command(&cmd_mem_walk);

	return 0;
}

APP_COMMAND_REGISTER(mem_walk, cmd_mem_walk_init);
