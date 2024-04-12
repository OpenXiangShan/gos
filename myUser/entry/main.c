#include "print.h"
#include "malloc.h"
#include "command.h"

void start_user(char *cmd)
{
	command_init();
	do_command(cmd);
}
