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
#include "asm/type.h"
#include "timer.h"

static int count_down = 5;

static int do_timer_test(void *data)
{
	myGuest_print("%s -- %ds\n", __FUNCTION__, count_down--);

	return 0;
}

static int cmd_timer_test_handler(int argc, char *argv[], void *priv)
{
	count_down = 5;

	while (count_down > 0) {
		while (set_timer(do_timer_test, 1000, NULL)) ;
	}

	return 0;
}

static const struct command cmd_timer_test = {
	.cmd = "timer_test",
	.handler = cmd_timer_test_handler,
	.priv = NULL,
};

int cmd_timer_test_init()
{
	register_command(&cmd_timer_test);

	return 0;
}

APP_COMMAND_REGISTER(timer_test, cmd_timer_test_init);
