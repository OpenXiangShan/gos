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
	print("Input test case:\n");
	print("    vadd_vi_0\n");
	print("    vasubu_vv_8\n");
	print("    vsadd_vv_2\n");
	print("    v_zicclsm \n");
}

static int user_cmd_vector_test_handler(int argc, char *argv[], void *priv)
{
	int i, ret = 0;

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
	} else if (!strncmp(argv[0], "v_zicclsm", sizeof("v_zicclsm"))) {
		for (i = 0; i < 100; i++) {
			ret = v_zicclsm_test();
			if (ret)
				break;
		}
	} else {
		print("Invalid option\n");
		return -1;
	}

	print("%s test complete -- times:%d ret:%d\n", argv[0], i, ret);
	if (ret == 0)
		return TEST_PASS;
	else
		return TEST_FAIL;
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
