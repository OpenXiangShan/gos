#include "print.h"
#include "malloc.h"
#include "command.h"

void start_user(struct user_run_params *params)
{
	command_init();

	while (1) {
		if (params->busy) {
			do_command(params);
			params->busy = 0;
		}
	}
}
