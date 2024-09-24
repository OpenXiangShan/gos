/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <asm/type.h>
#include <print.h>
#include <device.h>
#include <asm/mmio.h>
#include <asm/trap.h>
#include <asm/sbi.h>
#include <vmap.h>
#include <string.h>
#include "../command.h"

#define ILLEGAL_INSTRUCTION ".long 0xFFFFFFFF"


static int ebreak_test(void)
{
	unsigned long medeleg = 0;

	medeleg = sbi_get_medeleg();
	medeleg |= (1UL << CAUSE_BREAKPOINT);
	sbi_set_medeleg(medeleg);
	asm volatile("ebreak");
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

static int load_access_test(void)
{
	unsigned long addr = 0;
	unsigned long val = 0;

	addr = (unsigned long)ioremap(0, 4096, 0);

	val = readl(addr);
	print("0x%x = %x\n", addr, val);
	return 0;
}

static int store_access_test(void)
{
	unsigned long addr = 0;
	unsigned long val = 0;

	addr = (unsigned long)ioremap(0, 4096, 0);

	writel(addr, val);
	print("0x%x = %x\n", addr, val);
	return 0;
}

static int illegal_instruction_test(void)
{
	asm volatile(ILLEGAL_INSTRUCTION);
	return 0;
}

static void Usage(void)
{
	print("Usage: stval_test [cmd] \n");
	print("cmd option:\n");
	print("    -- ebreak \n");
	print("    -- load_misaligned \n");
	print("    -- store_misaligned \n");
	print("    -- load_page \n");
	print("    -- store_page \n");
	print("    -- load_access \n");
	print("    -- store_access \n");
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
	} else if (!strncmp(argv[0], "load_misaligned", sizeof("load_misaligned"))) {
		load_misaligned_test();
	} else if (!strncmp(argv[0], "store_misaligned", sizeof("store_misaligned"))) {
		store_misaligned_test();
	} else if (!strncmp(argv[0], "load_page", sizeof("load_page"))) {
		load_page_test();
	} else if (!strncmp(argv[0], "store_page", sizeof("store_page"))) {
		store_page_test();
	} else if (!strncmp(argv[0], "load_access", sizeof("load_access"))) {
		load_access_test();
	} else if (!strncmp(argv[0], "store_access", sizeof("store_access"))) {
		store_access_test();
	} else if (!strncmp(argv[0], "illegal_instruction", sizeof("illegal_instruction"))) {
		illegal_instruction_test();
	} else {
		print("Unsupport command\n");
		Usage();
		return -1;
	}

	print("test failure\n");//All test items will raise an exception and will not run here

	return 0;
}



static const struct command cmd_stval_test= {
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

