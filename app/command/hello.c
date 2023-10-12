#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"

static int cmd_hello_handler(int argc, char *argv[], void *priv)
{
	print("Hello Bosc gos Shell!!!\n");

	return 0;
}

static const struct command cmd_hello = {
	.cmd = "hello",
	.handler = cmd_hello_handler,
	.priv = NULL,
};

int cmd_hello_init()
{
	register_command(&cmd_hello);

	return 0;
}

APP_COMMAND_REGISTER(hello, cmd_hello_init);
