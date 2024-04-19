#include "command.h"
#include "print.h"
#include "asm/type.h"
#include "string.h"

static void page_tlb_sfence_all_test()
{

}

static int cmd_page_tlb_test_handler(int argc, char *argv[], void *priv)
{
	if (argc != 1) {
		print("Invalid input params!\n");
		return -1;
	}

	if (!strncmp(argv[0], "sfence_gvma_all", sizeof("sfence_gvma_all"))) {
		page_tlb_sfence_all_test();
	}

	return 0;
}

static const struct command cmd_page_tlb_test = {
	.cmd = "page_tlb_test",
	.handler = cmd_page_tlb_test_handler,
	.priv = NULL,
};

int cmd_page_tlb_test_init(void)
{
	register_command(&cmd_page_tlb_test);

	return 0;
}

APP_COMMAND_REGISTER(page_tlb_test, cmd_page_tlb_test_init);
