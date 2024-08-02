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

static void Usage()
{
	print
	    ("dma_test_ext [dst_addr] [src_addr] [size] [data_width] [awlen]\n");
}

static int cmd_dma_test_ext_handler(int argc, char *argv[], void *priv)
{
	int ret = 0;
	int i = 0;
	void *dst, *src;
	unsigned int data_width, awlen;
	unsigned int size;
	unsigned int diff = 0, start_time = 0;

	unsigned int aa = 0;
	for (i = 0; i < 16384; i++)
		*((unsigned int *)0x900a0000 + i) = aa++;

	if (argc != 5) {
		Usage();
		return -1;
	}

	if (!is_digit((char *)argv[0]) || !is_digit((char *)argv[1]) ||
	    !is_digit((char *)argv[2]) ||
	    !is_digit((char *)argv[3]) || !is_digit((char *)argv[4])) {
		print("invalid input params.\n");
		return -1;
	}

	dst = (void *)atoi(argv[0]);
	src = (void *)atoi(argv[1]);
	size = atoi(argv[2]);
	data_width = atoi(argv[3]);
	awlen = atoi(argv[4]);

	start_time = get_system_time_ms();
	ret = dma_transfer(dst, src, size, data_width, awlen);
	if (ret == -1) {
		print("memcpy_hw failed, timeout...\n");
		return -1;
	}

	diff = get_system_time_ms() - start_time;

	for (i = 0; i < size; i++) {
		if (*((unsigned int *)0x900a0000 + i) !=
		    *((unsigned int *)0x95000000 + i))
			break;
	}

	print("dma_test success! cost: %dms\n", diff);

	return 0;
}

static const struct command cmd_dma_test_ext = {
	.cmd = "dma_test_ext",
	.handler = cmd_dma_test_ext_handler,
	.priv = NULL,
};

int dma_test_ext_init()
{
	register_command(&cmd_dma_test_ext);

	return 0;
}

APP_COMMAND_REGISTER(dma_test_ext, dma_test_ext_init);
