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
#include <vmap.h>
#include <string.h>
#include <mm.h>
#include "command.h"

#define GPA 0x79fff000

static int implicit_load(void)
{
	unsigned long gva = 0;
	unsigned long gpa = GPA;
	unsigned long val = 0;

	gva = (unsigned long)ioremap((void *)gpa, 4096, 0);
	print("GPA:0x%lx, htval should be 0x%lx or 0\n", gpa, gpa << 2);

	val = readl(gva);
	print("0x%x = %x\n", gpa, val);
	return 0;
}

static int implicit_store(void)
{
	unsigned long gva = 0;
	unsigned long gpa = GPA;
	unsigned long val = 0;

	gva = (unsigned long)ioremap((void *)gpa, 4096, 0);
	print("GPA:0x%lx, htval should be 0x%lx or 0\n", gpa, gpa << 2);

	writel(gva, val);
	print("0x%x = %x\n", gpa, val);
	return 0;
}

static int misaligne_load(void)
{
	unsigned long gva = 0;
	unsigned long gpa = 0;
	unsigned long val = 0;

	gva = (unsigned long)mm_alloc(8);
	gva += 1;
	gpa = virt_to_phy(gva);

	print("GPA:0x%lx, htval should be 0x%lx or 0\n", gpa, gpa << 2);

	val = readl(gva);
	print("0x%x = %x\n", gpa, val);
	return 0;
}

static int misaligne_store(void)
{
	unsigned long gva = 0;
	unsigned long gpa = 0;
	unsigned long val = 0;

	gva = (unsigned long)mm_alloc(8);
	gva += 1;
	gpa = virt_to_phy(gva);

	print("GPA:0x%lx, htval should be 0x%lx or 0\n", gpa, gpa << 2);

	writel(gva, val);
	print("0x%x = %x\n", gpa, val);
	return 0;
}

static void Usage(void)
{
	print("Usage: page_tlb_test [cmd] \n");
	print("cmd option:\n");
	print("    -- misaligne_load \n");
	print("    -- misaligne_store \n");
	print("    -- implicit_load \n");
	print("    -- implicit_store \n");
	return;
}

static int cmd_htval_test_handler(int argc, char *argv[], void *priv)
{
	if (argc < 1) {
		print("Invalid input params\n");
		Usage();
		return -1;
	}

	if (!strncmp(argv[0], "misaligne_load", sizeof("misaligne_load"))) {
		misaligne_load();
	} else
	    if (!strncmp(argv[0], "misaligne_store", sizeof("misaligne_store")))
	{
		misaligne_store();
	} else if (!strncmp(argv[0], "implicit_load", sizeof("implicit_load"))) {
		implicit_load();
	} else
	    if (!strncmp(argv[0], "implicit_store", sizeof("implicit_store"))) {
		implicit_store();
	} else {
		print("Unsupport command\n");
		Usage();
		return -1;
	}

	print("test failure\n");	//All test items will raise an exception and will not run here

	return 0;
}

static const struct command cmd_htval_test = {
	.cmd = "htval_test",
	.handler = cmd_htval_test_handler,
	.priv = NULL,
};

int cmd_htval_init()
{
	register_command(&cmd_htval_test);

	return 0;
}

APP_COMMAND_REGISTER(stval, cmd_htval_init);
