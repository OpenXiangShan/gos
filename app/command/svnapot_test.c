#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"
#include "vmap.h"
#include "asm/pgtable.h"
#include "string.h"
#include "mm.h"
#include "tlbflush.h"

static int cmd_svnapot_test_handler(int argc, char *argv[], void *priv)
{
	char *addr;
	int size = PAGE_64K_SIZE;

	addr = (char *)vmem_alloc_huge(size, PAGE_64K_SIZE, NULL);
	addr[0] = 66;
	addr[size - 1] = 88;

	print("addr[0]:%d addr[%d]:%d\n", addr[0], size - 1, addr[size - 1]);

	return 0;
}

static const struct command cmd_svnapot_test = {
	.cmd = "svnapot_test",
	.handler = cmd_svnapot_test_handler,
	.priv = NULL,
};

int svnapot_test_init()
{
	register_command(&cmd_svnapot_test);

	return 0;
}

APP_COMMAND_REGISTER(svnapot_test, svnapot_test_init);
