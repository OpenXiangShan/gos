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
#include "../command.h"
#include "asm/sbi.h"
#include "ipi.h"

static int cmd_wfi_test_handler(int argc, char *argv[], void *priv)
{
	print("enter wfi... press any key to exit...\n");
	__asm__ __volatile__("wfi");
	print("exit wfi...\n");

	return 0;
}

static const struct command cmd_wfi_test = {
	.cmd = "wfi_test",
	.handler = cmd_wfi_test_handler,
	.priv = NULL,
};

int cmd_wfi_test_init()
{
	register_command(&cmd_wfi_test);

	return 0;
}

APP_COMMAND_REGISTER(wfi_test, cmd_wfi_test_init);
