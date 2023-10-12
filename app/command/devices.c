#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"

static int cmd_devices_handler(int argc, char *argv[], void *priv)
{
	walk_devices();

	return 0;
}

static const struct command cmd_devices = {
	.cmd = "devices",
	.handler = cmd_devices_handler,
	.priv = NULL,
};

int cmd_devices_init()
{
	register_command(&cmd_devices);

	return 0;
}

APP_COMMAND_REGISTER(devices, cmd_devices_init);
