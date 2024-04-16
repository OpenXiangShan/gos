#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"
#include "user.h"
#include "task.h"

char *command = NULL;

static int user_task_start(void *data)
{
	struct user *user;
	char *cmd = (char *)data;

	user = user_create();
	if (!user)
		return -1;

	user_mode_run(user, cmd);

	return 0;
}

static void Usage()
{
	print("user_run [cmd]\n");
}

static int cmd_user_run_handler(int argc, char *argv[], void *priv)
{
	if (argc != 1) {
		Usage();
		print("invalid input param.\n");
		return -1;
	}
	command = argv[0];

	create_task("user", user_task_start, command, 0, NULL, 0);

	return 0;
}

static const struct command cmd_user_run = {
	.cmd = "user_run",
	.handler = cmd_user_run_handler,
	.priv = NULL,
};

int cmd_user_run_init()
{
	register_command(&cmd_user_run);

	return 0;
}

APP_COMMAND_REGISTER(user_run, cmd_user_run_init);
