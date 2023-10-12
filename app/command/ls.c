#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"

static int cmd_ls_handler(int argc, char *argv[], void *priv)
{
	walk_and_print_command();

	return 0;
}

static const struct command cmd_ls = {
	.cmd = "ls",
	.handler = cmd_ls_handler,
	.priv = NULL,
};

int cmd_ls_init()
{
	register_command(&cmd_ls);

	return 0;
}

APP_COMMAND_REGISTER(ls, cmd_ls_init);
