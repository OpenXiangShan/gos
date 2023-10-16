#include <asm/type.h>
#include <print.h>
#include <device.h>
#include <string.h>
#include <asm/mmio.h>
#include "../command.h"

void Usage()
{
	print("mem_read Address count\n");
}

static int cmd_mem_read_handler(int argc, char *argv[], void *priv)
{
	unsigned long start;
	unsigned int len;
	int i = 0;

	if (argc != 2) {
		Usage();
		print("invalid input param.\n");
		return -1;
	}

	if (!is_digit(argv[0]) || !is_digit(argv[1])) {
		Usage();
		print("invalid input param.\n");
		return -1;
	}

	start = atoi(argv[0]);
	start = (start >> 2) << 2;
	len = atoi(argv[1]);

	for (i = 0; i < len; i = i + 4) {
		//if (i / 16 * 4 == 0) {
		//      print("\n");
		//      print("0x%x : ", start + 4);
		//}
		print("0x%x : 0x%x\n", start + i, readl(start + i));
	}

	print("\n");

	return 0;
}

static const struct command cmd_mem_read = {
	.cmd = "mem_read",
	.handler = cmd_mem_read_handler,
	.priv = NULL,
};

int cmd_mem_read_init()
{
	register_command(&cmd_mem_read);

	return 0;
}

APP_COMMAND_REGISTER(mem_read, cmd_mem_read_init);
