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
#include "type.h"
#include "vector.h"
#include "string.h"

static void Usage()
{
	printf("Input test case:\n");
	printf("    vadd_vi_0\n");
	printf("    vasubu_vv_8\n");
	printf("    vsadd_vv_2\n");
}

static int user_cmd_vector_test_handler(int argc, char *argv[], void *priv)
{
	int i, ret;

	if (argc < 1) {
		Usage();
		return -1;
	}

	if (!strncmp(argv[0], "vadd_vi_0", sizeof("vadd_vi_0"))) {
		for (i = 0; i < 100; i++) {
			ret = vadd_vi_0_start();
			if (ret)
				break;
		}
	} else if (!strncmp(argv[0], "vasubu_vv_8", sizeof("vasubu_vv_8"))) {
		for (i = 0; i < 100; i++) {
			ret = vasubu_vv_8_start();
			if (ret)
				break;
		}
	} else if (!strncmp(argv[0], "vsadd_vv_2", sizeof("vsadd_vv_2"))) {
		for (i = 0; i < 100; i++) {
			ret = vsadd_vv_2_start();
			if (ret)
				break;
		}
	}

	printf("%s test complete -- times:%d ret:%d\n", argv[0], i, ret);
	return 0;
}

static const struct command user_cmd_vector_test = {
	.cmd = "vector_test",
	.handler = user_cmd_vector_test_handler,
	.priv = NULL,
};

int user_cmd_vector_test_init()
{
	register_command(&user_cmd_vector_test);

	return 0;
}

APP_COMMAND_REGISTER(vector_test, user_cmd_vector_test_init);
