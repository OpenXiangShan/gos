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

#include "asm/type.h"
#include "command.h"
#include "print.h"
#include "dma.h"
#include "mm.h"

static int check_mem(char *src, char *dst, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		if (src[i] != dst[i])
			return -1;
	}

	return 0;
}

static int cmd_dma_test_handler(int argc, char *argv[], void *priv)
{
	char *src, *dst;
	int i;

	src = (char *)mm_alloc(4096);
	dst = (char *)mm_alloc(4096);

	for (i = 0; i < 4096; i++)
		src[i] = i;

	if (dma_m2m((void *)src, (void *)dst, 4096)) {
		print("dma_test (memory to memory) -- test fail\n");
		return -1;
	}

	if (check_mem(src, dst, 4095)) {
		print("dma_test (memory to memory) -- test fail\n");
		return -1;
	}

	for (i = 0; i < 16; i++)
		print("src[%d]: %d -- dst[%d]: %d\n", i, src[i], i, dst[i]);

	print("dma_test (memory to memory) -- test pass!!\n");

	mm_free((void *)src, 4096);
	mm_free((void *)dst, 4096);

	return 0;
}

static const struct command cmd_dma_test = {
	.cmd = "dma_test",
	.handler = cmd_dma_test_handler,
	.priv = NULL,
};

int cmd_dma_test_init()
{
	register_command(&cmd_dma_test);

	return 0;
}

APP_COMMAND_REGISTER(dma_test, cmd_dma_test_init);
