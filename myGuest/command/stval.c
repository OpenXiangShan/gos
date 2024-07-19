#include <asm/type.h>
#include <print.h>
#include <device.h>
#include <asm/mmio.h>
#include <vmap.h>
#include <string.h>
#include "command.h"

#define ILLEGAL_INSTRUCTION ".long 0xFFFFFFFF"

static int ebreak_test(void)
{
	asm volatile ("ebreak");
	return 0;
}

static int load_misaligned_test(void)
{
	unsigned int val = 0;
	val = readl(0x80000007);
	print("0x80000007 = %x\n", val);
	return 0;
}

static int store_misaligned_test(void)
{
	unsigned int val = 0;
	writel(0x80000007, val);
	print("0x80000007 = %x\n", val);
	return 0;

}

static int load_page_test(void)
{
	int val = 0;

	val = readl(0xffffffc700000000);
	print("0xffffffc700000000 = %x\n", val);
	return 0;
}

static int store_page_test(void)
{
	int val = 0;

	writel(0xffffffc700000000, val);
	print("0xffffffc700000000 = %x\n", val);
	return 0;
}

static int illegal_instruction_test(void)
{
	asm volatile (ILLEGAL_INSTRUCTION);
	return 0;
}

static void Usage(void)
{
	print("Usage: page_tlb_test [cmd] \n");
	print("cmd option:\n");
	print("    -- ebreak \n");
	print("    -- load_misaligned \n");
	print("    -- store_misaligned \n");
	print("    -- load_page \n");
	print("    -- store_page \n");
	print("    -- illegal_instruction \n");
}

static int cmd_stval_test_handler(int argc, char *argv[], void *priv)
{
	if (argc < 1) {
		print("Invalid input params\n");
		Usage();
		return -1;
	}

	if (!strncmp(argv[0], "ebreak", sizeof("ebreak"))) {
		ebreak_test();
	} else
	    if (!strncmp(argv[0], "load_misaligned", sizeof("load_misaligned")))
	{
		load_misaligned_test();
	} else
	    if (!strncmp
		(argv[0], "store_misaligned", sizeof("store_misaligned"))) {
		store_misaligned_test();
	} else if (!strncmp(argv[0], "load_page", sizeof("load_page"))) {
		load_page_test();
	} else if (!strncmp(argv[0], "store_page", sizeof("store_page"))) {
		store_page_test();
	} else
	    if (!strncmp
		(argv[0], "illegal_instruction",
		 sizeof("illegal_instruction"))) {
		illegal_instruction_test();
	} else {
		print("Unsupport command\n");
		Usage();
		return -1;
	}

	print("test failure\n");	//All test items will raise an exception and will not run here

	return 0;
}

static const struct command cmd_stval_test = {
	.cmd = "stval_test",
	.handler = cmd_stval_test_handler,
	.priv = NULL,
};

int cmd_stval_init()
{
	register_command(&cmd_stval_test);

	return 0;
}

APP_COMMAND_REGISTER(stval, cmd_stval_init);
