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
#include "string.h"

static void Usage(void)
{
	print("ipi_test cpu=[cpuid] [ipi-id]");
}

static int cmd_ipi_test_handler(int argc, char *argv[], void *priv)
{
	int id, cpu;

	if (argc == 0) {
		send_ipi(0, 0, NULL);
		return 0;
	}

	if (argc != 2)
		return -1;

	if (!strncmp(argv[0], "cpu=", sizeof("cpu=") - 1)) {
		char *tmp = argv[0];
		tmp += sizeof("cpu=") - 1;
		cpu = atoi(tmp);
	} else {
		print("Invalid params...\n");
		Usage();
		return -1;
	}

	id = atoi(argv[1]);
	send_ipi(cpu, id, NULL);

	return 0;
}

static const struct command cmd_ipi_test = {
	.cmd = "ipi_test",
	.handler = cmd_ipi_test_handler,
	.priv = NULL,
};

int cmd_ipi_test_init()
{
	register_command(&cmd_ipi_test);

	return 0;
}

APP_COMMAND_REGISTER(ipi_test, cmd_ipi_test_init);
