#include "command.h"
#include "print.h"
#include "asm/type.h"
#include "string.h"
#include "../drivers/page_tlb_test.h"

static int cmd_memory_test_handler(int argc, char *argv[], void *priv)
{
	unsigned long addr = get_memory_test_addr();

	print("addr:0x%lx -- %s\n", addr, addr);

	return 0;
}

static const struct command cmd_memory_test = {
	.cmd = "memory_test",
	.handler = cmd_memory_test_handler,
	.priv = NULL,
};

int cmd_memory_test_init(void)
{
	register_command(&cmd_memory_test);

	return 0;
}

APP_COMMAND_REGISTER(memory_test, cmd_memory_test_init);
