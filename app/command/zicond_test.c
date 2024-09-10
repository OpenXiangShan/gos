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
#include "asm/condition.h"

static int cmd_zicond_test_handler(int argc, char *argv[], void *priv)
{
	unsigned long v = 666;
	unsigned long c1 = 0;
	unsigned long c2 = 1;

	print("czero.nqz rs=%d rs2=%d rd=%d\n", v, c1, czero_nqz(v, c1));
	print("czero.nqz rs=%d rs2=%d rd=%d\n", v, c2, czero_nqz(v, c2));
	print("czero.eqz rs=%d rs2=%d rd=%d\n", v, c1, czero_eqz(v, c1));
	print("czero.eqz rs=%d rs2=%d rd=%d\n", v, c2, czero_eqz(v, c2));

	return 0;
}

static const struct command cmd_zicond_test = {
	.cmd = "zicond_test",
	.handler = cmd_zicond_test_handler,
	.priv = NULL,
};

int cmd_zicond_test_init()
{
	register_command(&cmd_zicond_test);

	return 0;
}

APP_COMMAND_REGISTER(zicond_test, cmd_zicond_test_init);
