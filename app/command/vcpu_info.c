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
