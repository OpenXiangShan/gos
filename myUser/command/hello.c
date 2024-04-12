#include "command.h"
#include "print.h"
#include "type.h"

static int user_cmd_hello_handler(int argc, char *argv[], void *priv)
{
	printf("Hello MyUesr!!\n");

	return 0;
}

static const struct command user_cmd_hello = {
	.cmd = "hello",
	.handler = user_cmd_hello_handler,
	.priv = NULL,
};

int user_cmd_hello_init()
{
	register_command(&user_cmd_hello);

	return 0;
}

APP_COMMAND_REGISTER(hello, user_cmd_hello_init);
