#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"
#include "user.h"
#include "task.h"
#include "string.h"
#include "mm.h"

static int user_task_start(void *data)
{
	struct user *user;
	struct user_run_params *params = (struct user_run_params *)data;

	user = user_create();
	if (!user)
		return -1;

	user_mode_run(user, params);

	return 0;
}

static void Usage()
{
	print("user_run [cmd] [params1] [params2] ...\n");
}

static int cmd_user_run_handler(int argc, char *argv[], void *priv)
{
	struct user_run_params *params;
	int i;

	if (argc == 0) {
		Usage();
		print("invalid input param.\n");
		return -1;
	} else if (argc > 16) {
		print("Too much params...\n");
		return -1;
	}

	params =
	    (struct user_run_params *)mm_alloc(sizeof(struct user_run_params));
	if (!params) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}
	strcpy(params->command, argv[0]);
	params->argc = argc - 1;
	for (i = 0; i < params->argc; i++)
		strcpy(params->argv[i], argv[i + 1]);

	create_task("user", user_task_start, (void *)params, 0, NULL, 0, NULL);

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
