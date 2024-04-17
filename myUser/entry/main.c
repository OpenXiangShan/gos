#include "print.h"
#include "malloc.h"
#include "command.h"

void start_user(struct user_run_params *params)
{
	command_init();
	do_command(params);
}
