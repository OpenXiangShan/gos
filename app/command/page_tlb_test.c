#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"
#include "vmap.h"
#include "asm/pgtable.h"
#include "string.h"

static void Usage(void)
{
	print("Usage: page_tlb_test [cmd]\n");
	print("cmd option:\n");
	print("    -- ACC (page table access bit test)\n");
}

static void page_table_access_bit_test()
{
	char *vaddr;

	vaddr = (char *)vmem_alloc_lazy(PAGE_SIZE, NULL);
	if (!vaddr)
		return;

	print("vaddr:0x%lx\n", vaddr);

	strcpy(vaddr, "Hello");
	print("%s\n", vaddr);
}

static int cmd_page_tlb_test_handler(int argc, char *argv[], void *priv)
{
	if (argc != 1) {
		print("Invalid input params\n");
		Usage();
	}

	if (!strncmp(argv[0], "ACC", sizeof("ACC"))) {
		page_table_access_bit_test();
	}

	return 0;
}

static const struct command cmd_page_tlb_test = {
	.cmd = "page_tlb_test",
	.handler = cmd_page_tlb_test_handler,
	.priv = NULL,
};

int page_tlb_test_init()
{
	register_command(&cmd_page_tlb_test);

	return 0;
}

APP_COMMAND_REGISTER(page_tlb_test, page_tlb_test_init);
