#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "string.h"
#include "../command.h"
#include "asm/pgtable.h"
#include "mm.h"

static unsigned long mem_start, mem_size;
static unsigned long mem_data;

static void Usage(void)
{
	print("Usage: scan_mem_test [param]\n");
	print("param option:\n");
	print("-- mem_start: memory start addr \n");
	print("-- mem_size : memory addr length\n");
	print
	    ("-- mem_data : memory test data, if not  entered, use default value\n");
}

void ddr_set(void *src, unsigned long data, unsigned long size)
{
	unsigned long i;
	unsigned long *start = src;
	unsigned int plot = size / 8 / 4;

	print("ddr start == 0x%lx\n", start);
	for (i = 0; i < size / 8; i++) {
		*(start + i) = data;
		if ((i % plot) == 0)
			print("write addr is :0x%lx \n", start + i);
	}
}

int ddr_check(void *src, unsigned long data, unsigned long size)
{
	unsigned long i;
	unsigned long *start = src;
	unsigned int plot = size / 8 / 4;

	for (i = 0; i < size / 8; i++) {
		if (*(start + i) != data) {
			print("ERROR, start = %p i=%0x \r\n", start, i);
			return 1;
		}
		if ((i % plot) == 0)
			print("check addr is :0x%lx \n", start + i);
	}
	return 0;
}

static int cmd_scan_mem_test_handler(int argc, char *argv[], void *priv)
{
	void *va;

	if (argc < 2) {
		print("Invalid input params\n");
		Usage();
		return -1;
	}

	mem_start = atoi(argv[0]);
	mem_size = atoi(argv[1]);

	if (argv[2] == NULL)
		mem_data = 0x89984546148341;
	else
		mem_data = atoi(argv[2]);
	print("mem_start:0x%lx, mem_size:0x%lx, mem_data:0x%lx \n", mem_start,
	      mem_size, mem_data);

	if (!(mem_range_is_free(mem_start, mem_size))) {
		print("please use unused mem:\n");
		walk_unused_mem_and_print();
		return 0;
	}

	va = mm_alloc_fix(mem_start, mem_size);
	if (!va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret;
	}

	ddr_set(va, mem_data, mem_size);
	int status = ddr_check(va, mem_data, mem_size);
	if (status)
		print("ddr data is not good \r\n");
	else
		print("ddr data is ok \r\n");
ret:
	mm_free(va, mem_size);

	return 0;
}

static const struct command cmd_scan_mem_test = {
	.cmd = "scan_mem_test",
	.handler = cmd_scan_mem_test_handler,
	.priv = NULL,
};

int cmd_scan_mem_test_init()
{
	register_command(&cmd_scan_mem_test);

	return 0;
}

APP_COMMAND_REGISTER(scan_mem_test, cmd_scan_mem_test_init);
