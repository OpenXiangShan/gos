#include <print.h>
#include <device.h>
#include <asm/mmio.h>
#include <asm/type.h>
#include <event.h>
#include "../command.h"

#define TEST_COUNT 3

static int cmd_imsic_test_handler(int argc, char *argv[], void *priv)
{
	int i, fd, n;

	fd = open("IMSIC_TEST");
	if (fd == -1) {
		print("open %s fail.\n", "IMSIC_TEST");
		return -1;
	}

	for (i = 0; i < TEST_COUNT; i++) {
		n = 0;
		print("imsic_test %d times: trigger irq%d\n", i, n);
		ioctl(fd, n++, NULL);
		wait_for_ms(1000);

		print("imsic_test %d times: trigger irq%d\n", i, n);
		ioctl(fd, n++, NULL);
		wait_for_ms(1000);

		print("imsic_test %d times: trigger irq%d\n", i, n);
		ioctl(fd, n++, NULL);
		wait_for_ms(1000);
	}

	return 0;
}

static const struct command cmd_imsic_test = {
	.cmd = "imsic_test",
	.handler = cmd_imsic_test_handler,
	.priv = NULL,
};

int command_imsic_test_init()
{
	register_command(&cmd_imsic_test);

	return 0;
}

APP_COMMAND_REGISTER(imsic_test_command, command_imsic_test_init);
