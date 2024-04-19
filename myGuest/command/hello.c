#include "command.h"
#include "print.h"
#include "asm/type.h"

static int cmd_hello_handler(int argc, char *argv[], void *priv)
{
	print("Hello MyUesr!!\n");

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
