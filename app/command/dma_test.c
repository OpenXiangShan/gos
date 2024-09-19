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
#include <string.h>
#include <dmac.h>
#include <timer.h>
#include "../command.h"
#include "mm.h"

static void Usage()
{
	print("dma_test [dev_name] [size]\n");
}

static int cmd_dma_test_handler(int argc, char *argv[], void *priv)
{
	int ret = 0, i;
	void *dst, *src;
	unsigned int size;
	unsigned int diff = 0, start_time = 0;
	char *name;

	if (argc < 2) {
		Usage();
		return -1;
	}

	if (!is_digit((char *)argv[1])) {
		print("invalid input params.\n");
		return -1;
	}

	name = argv[0];
	size = atoi(argv[1]);
	src = mm_alloc(size);
	if (!src) {
		print("%s -- alloc src failed!\n", __FUNCTION__);
		return -1;
	}
	dst = mm_alloc(size);
	if (!dst) {
		print("%s -- alloc dst failed!\n", __FUNCTION__);
		return -1;
	}

	for (i = 0; i < size; i++)
		*((char *)((unsigned long)src + i)) = i;

	start_time = get_system_time_ms();
	ret = memcpy_hw(name, dst, src, size);
	if (ret == -1) {
		print("memcpy_hw failed, timeout...\n");
		return -1;
	}

	diff = get_system_time_ms() - start_time;

	for (i = 0; i < size; i++) {
		if (*((char *)((unsigned long)src + i)) !=
		    *((char *)((unsigned long)dst + i))) {
			print("dma_test failed!!\n");
			return -1;
		}
	}

	print("dma_test success! cost: %dms\n", diff);

	return 0;
}

static const struct command cmd_dma_test = {
	.cmd = "dma_test",
	.handler = cmd_dma_test_handler,
	.priv = NULL,
};

int dma_test_init()
{
	register_command(&cmd_dma_test);

	return 0;
}

APP_COMMAND_REGISTER(dma_test, dma_test_init);
