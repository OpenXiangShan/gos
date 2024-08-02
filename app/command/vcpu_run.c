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
#include "task.h"
#include "string.h"
#include "mm.h"

static int vcpu_start(void *data)
{
	struct vcpu *vcpu;
	struct virt_run_params *params = (struct virt_run_params *)data;

	vcpu = vcpu_create();
	if (!vcpu)
		return -1;

	vcpu_run(vcpu, params);

	mm_free((void *)params, sizeof(struct virt_run_params));

	return 0;
}

static void Usage()
{
	print("vcpu_run [cmd] [oarams1] [params2] ...\n");
}

static int cmd_vcpu_run_handler(int argc, char *argv[], void *priv)
{
	int i;
	struct virt_run_params *params;

	if (argc == 0) {
		Usage();
		print("invalid input param.\n");
		return -1;
	} else if (argc > 16) {
		print("Too much params...\n");
		return -1;
	}

	params =
	    (struct virt_run_params *)mm_alloc(sizeof(struct virt_run_params));
	if (!params) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	strcpy(params->command, argv[0]);
	params->argc = argc - 1;
	for (i = 0; i < params->argc; i++)
		strcpy(params->argv[i], argv[i + 1]);

	create_task("vcpu", vcpu_start, (void *)params, 0, NULL, 0, NULL);

	return 0;
}

static const struct command cmd_vcpu_run = {
	.cmd = "vcpu_run",
	.handler = cmd_vcpu_run_handler,
	.priv = NULL,
};

int cmd_vcpu_run_init()
{
	register_command(&cmd_vcpu_run);

	return 0;
}

APP_COMMAND_REGISTER(vcpu_run, cmd_vcpu_run_init);
