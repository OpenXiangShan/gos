#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"

static int cmd_history_handler(int argc, char *argv[], void *pirv)
{
	int n = 0;
	struct cmd_history *history = command_get_history();
	struct cmd_name *cmd;

	if (!history)
		return -1;

	for_each_history_cmd(cmd, history, n)
	    print("%s\n", cmd->name);

	return 0;
}

static const struct command cmd_history = {
	.cmd = "history",
	.handler = cmd_history_handler,
	.priv = NULL,
};

int cmd_history_init()
{
	register_command(&cmd_history);

	return 0;
}

APP_COMMAND_REGISTER(history, cmd_history_init);
