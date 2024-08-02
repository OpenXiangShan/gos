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
#include "cpu.h"

static void Usage_run_at()
{
	print("vcpu_run_ext_at cpu=[cpu id] vmid=[vmid] [cmd] [params1] [params2] ...\n");
}

static void Usage()
{
	print("vcpu_run_ext [cmd] [params1] [params2] ...\n");
	print("vcpu_run_ext cpu=[cpu id] [cmd] [params1] [params2] ...\n");
}

static int vcpu_start(void *data)
{
	struct vcpu *vcpu;
	struct virt_run_params *params = (struct virt_run_params *)data;

	vcpu = vcpu_create_force();
	if (!vcpu)
		return -1;

	vcpu_run(vcpu, params);

	mm_free((void *)params, sizeof(struct virt_run_params));

	return 0;
}

static int cmd_vcpu_run_ext_handler(int argc, char *argv[], void *priv)
{
	int i, cpu = 0, bg = 0, offset = 0;
	struct virt_run_params *params;

	if (argc == 0) {
		Usage();
		print("invalid input param.\n");
		return -1;
	} else if (argc > 16) {
		print("Too much params...\n");
		return -1;
	}

	if (!strncmp(argv[offset], "cpu=", sizeof("cpu=") - 1)) {
		char *tmp = argv[offset];
		tmp += sizeof("cpu=") - 1;
		cpu = atoi(tmp);
		offset += 1;
	}

	if (!strncmp(argv[offset], "bg=", sizeof("bg=") - 1)) {
		char *tmp = argv[offset];
		tmp += sizeof("bg=") - 1;
		bg = atoi(tmp);
		offset += 1;
	}

	params =
	    (struct virt_run_params *)mm_alloc(sizeof(struct virt_run_params));
	if (!params) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	params->bg = bg;
	strcpy(params->command, argv[offset]);
	params->argc = argc - 1 - offset;
	for (i = 0; i < params->argc; i++)
		strcpy(params->argv[i], argv[i + 1 + offset]);

	print("vcpu_run_ext on cpu%d command:%s\n", cpu, params->command);
	if (create_task("vcpu", vcpu_start, (void *)params, cpu, NULL, 0, NULL)) {
		int n;
		print("create task on cpu%d failed...\n", cpu);
		print("online cpu info:\n");
		for_each_online_cpu(n) {
			print("  cpu%d\n", n);
		}
	}

	return 0;
}

static const struct command cmd_vcpu_run_ext = {
	.cmd = "vcpu_run_ext",
	.handler = cmd_vcpu_run_ext_handler,
	.priv = NULL,
};

static int vcpu_start_at(void *data)
{
	struct vcpu *vcpu;
	struct virt_run_params *params = (struct virt_run_params *)data;

	print("%s -- cpu:%d vmid:%d\n",
	      __FUNCTION__, params->cpu, params->vmid);

	vcpu = get_vcpu(params->vmid, params->cpu);
	if (!vcpu) {
		print("There is no vm(vmid=%d) is running on cpu%d\n",
		      params->vmid, params->cpu);
		return -1;
	}

	vcpu_run(vcpu, params);

	mm_free((void *)params, sizeof(struct virt_run_params));

	return 0;
}

static int cmd_vcpu_run_ext_at_handler(int argc, char *argv[], void *priv)
{
	int i, cpu = 0, vmid = 1;
	struct virt_run_params *params;

	if (argc < 3) {
		Usage_run_at();
		print("invalid input param.\n");
		return -1;
	} else if (argc > 16) {
		print("Too much params...\n");
		return -1;
	}

	if (!strncmp(argv[0], "cpu=", sizeof("cpu=") - 1)) {
		char *tmp = argv[0];
		tmp += sizeof("cpu=") - 1;
		cpu = atoi(tmp);
	}
	else {
		print("Invalid params...\n");
		Usage_run_at();
	}

	if (!strncmp(argv[1], "vmid=", sizeof("vmid=") - 1)) {
		char *tmp = argv[1];
		tmp += sizeof("vmid=") - 1;
		vmid = atoi(tmp);
	}
	else {
		print("Invalid params...\n");
		Usage_run_at();
	}

	params =
	    (struct virt_run_params *)mm_alloc(sizeof(struct virt_run_params));
	if (!params) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	strcpy(params->command, argv[2]);
	params->argc = argc - 3;
	for (i = 0; i < params->argc; i++)
		strcpy(params->argv[i], argv[i + 3]);
	params->cpu = cpu;
	params->vmid = vmid;

	if (create_task("vcpu", vcpu_start_at, (void *)params, 0, NULL, 0, NULL)) {
		int n;
		print("create task on cpu%d failed...\n", cpu);
		print("online cpu info:\n");
		for_each_online_cpu(n) {
			print("  cpu%d\n", n);
		}
	}

	return 0;
}

static const struct command cmd_vcpu_run_ext_at = {
	.cmd = "vcpu_run_ext_at",
	.handler = cmd_vcpu_run_ext_at_handler,
	.priv = NULL,
};

int cmd_vcpu_run_ext_init()
{
	register_command(&cmd_vcpu_run_ext);
	register_command(&cmd_vcpu_run_ext_at);

	return 0;
}

APP_COMMAND_REGISTER(vcpu_run_ext, cmd_vcpu_run_ext_init);
