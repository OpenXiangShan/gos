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
#include "print.h"
#include "device.h"
#include "spinlocks.h"
#include "mm.h"
#include "string.h"
#include "../../../command.h"
#include "consumer_producer.h"
#include "consumer_producer_dummy_case.h"

struct consumer_producer_test_case {
	const char name[64];
	void (*test_case_handler)(void);
};

static struct consumer_producer_test_case _case[] = {
	{"dummy",  consumer_producer_dummy_case_handler },
};
#define TEST_CASE_COUNT (sizeof(_case) / sizeof(_case[0]))

static int cmd_consumer_producer_test_handler(int argc, char *argv[], void *priv)
{
	int i;

	if (argc <= 0) {
		print("Invalid input params...\n");
		return -1;
	}

	for (i = 0; i < TEST_CASE_COUNT; i++) {
		if (!strcmp(argv[0], _case[i].name))
			_case[i].test_case_handler();
	}

	return 0;
}

static const struct command cmd_consumer_producer_test = {
	.cmd = "consumer_producer_test",
	.handler = cmd_consumer_producer_test_handler,
	.priv = NULL,
};

int cmd_consumer_producer_test_init()
{
	register_command(&cmd_consumer_producer_test);

	return 0;
}

APP_COMMAND_REGISTER(consumer_producer_test, cmd_consumer_producer_test_init);
