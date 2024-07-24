#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"
#include "virt.h"
#include "task.h"
#include "string.h"
#include "mm.h"

static void Usage_run_at()
{
	print("vcpu_run_ext_at [vmid] [cmd] [params1] [params2] ...\n");
}

static void Usage()
{
	print("vcpu_run_ext [cmd] [params1] [params2] ...\n");
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

static const struct command cmd_vcpu_run_ext = {
	.cmd = "vcpu_run_ext",
	.handler = cmd_vcpu_run_ext_handler,
	.priv = NULL,
};

static int vcpu_start_at(void *data)
{
	struct vcpu *vcpu;
	struct virt_run_params *params = (struct virt_run_params *)data;

	print("vmid:%d\n", params->vmid);
	vcpu = get_vcpu(params->vmid);
	if (!vcpu)
		return -1;

	vcpu_run(vcpu, params);

	mm_free((void *)params, sizeof(struct virt_run_params));

	return 0;
}

static int cmd_vcpu_run_ext_at_handler(int argc, char *argv[], void *priv)
{
	int i;
	struct virt_run_params *params;

	if (argc < 2) {
		Usage_run_at();
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
	strcpy(params->command, argv[1]);
	params->argc = argc - 2;
	for (i = 0; i < params->argc; i++)
		strcpy(params->argv[i], argv[i + 2]);
	params->vmid = atoi(argv[0]);

	create_task("vcpu", vcpu_start_at, (void *)params, 0, NULL, 0, NULL);

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
