#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"
#include "virt.h"
#include "task.h"

static int vcpu_start(void *data)
{
	struct vcpu *vcpu;

	vcpu = vcpu_create();
	if (!vcpu)
		return -1;

	vcpu_run(vcpu, NULL);

	return 0;
}

static int cmd_vcpu_run_handler(int argc, char *argv[], void *priv)
{
	create_task("vcpu", vcpu_start, NULL, 1, NULL, 0);

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
