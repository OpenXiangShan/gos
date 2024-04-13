#include "command.h"
#include "type.h"

static int user_cmd_ls_handler(int argc, char *argv[], void *priv)
{
	walk_and_print_command();

	return 0;
}

static const struct command user_cmd_ls = {
	.cmd = "ls",
	.handler = user_cmd_ls_handler,
	.priv = NULL,
};

int user_cmd_ls_init()
{
	register_command(&user_cmd_ls);

	return 0;
}

APP_COMMAND_REGISTER(ls, user_cmd_ls_init);
