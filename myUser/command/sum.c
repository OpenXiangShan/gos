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

#define TEST_COUNT 10000000

static int user_cmd_sum_handler(int argc, char *argv[], void *priv)
{
	int i;
	int sum;
	unsigned long count = 0;

	while (1) {
		sum = 0;
		for (i = 1; i <= 100; i++)
			sum += i;

		if (sum != 5050) {
			print("%s -- test failed!!! test count:%dtimes\n",
			       __FUNCTION__, count);
			break;
		}

		if (++count == TEST_COUNT) {
			print("%s -- test pass!! test count:%dtimes\n",
			       __FUNCTION__, count);
			break;
		}
	}

	return 0;
}

static const struct command user_cmd_sum = {
	.cmd = "sum",
	.handler = user_cmd_sum_handler,
	.priv = NULL,
};

int user_cmd_sum_init()
{
	register_command(&user_cmd_sum);

	return 0;
}

APP_COMMAND_REGISTER(sum, user_cmd_sum_init);
