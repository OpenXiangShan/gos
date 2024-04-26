#include "command.h"
#include "print.h"
#include "asm/type.h"
#include "timer.h"

static int count_down = 5;

static int do_timer_test(void *data)
{
	myGuest_print("%s -- %ds\n", __FUNCTION__, count_down--);

	return 0;
}

static int cmd_timer_test_handler(int argc, char *argv[], void *priv)
{
	count_down = 5;

	while (count_down > 0) {
		while (set_timer(do_timer_test, 1000, NULL)) ;
	}

	return 0;
}

static const struct command cmd_timer_test = {
	.cmd = "timer_test",
	.handler = cmd_timer_test_handler,
	.priv = NULL,
};

int cmd_timer_test_init()
{
	register_command(&cmd_timer_test);

	return 0;
}

APP_COMMAND_REGISTER(timer_test, cmd_timer_test_init);
