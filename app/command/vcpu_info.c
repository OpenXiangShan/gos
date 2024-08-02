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
#include "virt.h"
#include "string.h"

static void Usage(void)
{
	print("print vcpu info on cpus:\n");
	print("vcpu_info [cpu] -- print vcpu info on [cpu]\n");
	print("vcpu_info       -- print vcpu info on all cpus\n");
}

static int cmd_vcpu_info_handler(int argc, char *argv[], void *priv)
{
	int cpu;

	if (argc == 1) {
		cpu = atoi(argv[0]);
		dump_vcpu_info_on_cpu(cpu);
	} else if (argc == 0) {
		dump_vcpu_info_on_all_cpu();
	} else
		Usage();

	return 0;
}

static const struct command cmd_vcpu_info = {
	.cmd = "vcpu_info",
	.handler = cmd_vcpu_info_handler,
	.priv = NULL,
};

int cmd_vcpu_info_init()
{
	register_command(&cmd_vcpu_info);

	return 0;
}

APP_COMMAND_REGISTER(vcpu_info, cmd_vcpu_info_init);
