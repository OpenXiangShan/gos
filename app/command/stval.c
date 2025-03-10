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
#include "stub.h"
#include <asm/csr.h>

#define ILLEGAL_INSTRUCTION ".long 0xFFFFFFFF"
#define ILLEGAL_INSTRUCTION_ADDR 0xFFFFFFFF
#define INVALID_ADDRESS 0xffffffc700000000UL
#define MISALIGNED_ADDR 0x80000007

unsigned long ref_addr;

//define internal_* functions for sepc comparison
void internal_ebreak()
{
	asm volatile("ebreak");
}

void internal_illegal_instruction()
{
	asm volatile(ILLEGAL_INSTRUCTION);
}

static unsigned int internal_readl(unsigned long addr)
{
	return readl(addr);
}

static void internal_writel(unsigned long addr, int val)
{
	writel(addr, val);
}

static void ebreak_stub_handler(struct pt_regs *regs)
{
	unsigned long stval;

	stval = read_csr(CSR_STVAL);
	print("%s %d scause:0x%lx sepc:0x%lx stval:0x%lx\n",
		  __FUNCTION__, __LINE__, regs->scause, regs->sepc, stval);
	if (regs->sepc == (unsigned long) internal_ebreak &&
		regs->scause == EXC_BREAKPOINT &&
		(stval == 0x0 || stval == (unsigned long) internal_ebreak)) {
		print("TEST PASS\n");
	} else {
		print("TEST FAIL\n");
	}
}

static int ebreak_test(void)
{
	unsigned long medeleg = 0;

	register_ebreak_stub_handler(ebreak_stub_handler);

	medeleg = sbi_get_medeleg();
	medeleg |= (1UL << CAUSE_BREAKPOINT);
	sbi_set_medeleg(medeleg);
	internal_ebreak();
	return 0;
}

static void load_misaligned_stub_handler(struct pt_regs *regs)
{
	unsigned long stval;


	stval = read_csr(CSR_STVAL);
	print("%s %d scause:0x%lx sepc:0x%lx stval:0x%lx\n",
		  __FUNCTION__, __LINE__, regs->scause, regs->sepc, stval);
	//based on the objdump info of internal_readl, the sepc value should be internal_readl addr + 6
	if (regs->scause == EXC_LOAD_MISALIGNED &&
		regs->sepc == (unsigned long) internal_readl + 6 &&
		(stval == 0x0 || stval == MISALIGNED_ADDR)) {
		print("TEST PASS\n");
	} else {
		print("TEST FAIL\n");
	}
}

static int load_misaligned_test(void)
{
	unsigned int val = 0;

	register_handle_exception_stub_handler(load_misaligned_stub_handler);

	val = internal_readl(MISALIGNED_ADDR);
	print("0x%x = %x\n", MISALIGNED_ADDR, val);
	return 0;
}

static void store_misaligned_stub_handler(struct pt_regs *regs)
{
	unsigned long stval;


	stval = read_csr(CSR_STVAL);
	print("%s %d scause:0x%lx sepc:0x%lx stval:0x%lx\n",
		  __FUNCTION__, __LINE__, regs->scause, regs->sepc, stval);
	//based on the objdump info of internal_writel, the sepc value should be internal_writel addr + 20
	if (regs->scause == EXC_STORE_MISALIGNED &&
		regs->sepc == (unsigned long) internal_writel + 20 &&
		(stval == 0x0 || stval == MISALIGNED_ADDR)) {
		print("TEST PASS\n");
	} else {
		print("TEST FAIL\n");
	}
}

static int store_misaligned_test(void)
{
	unsigned int val = 0;

	register_handle_exception_stub_handler(store_misaligned_stub_handler);

	internal_writel(MISALIGNED_ADDR, val);
	print("0x%x = %x\n", MISALIGNED_ADDR, val);
	return 0;

}

static void load_page_stub_handler(struct pt_regs *regs)
{
	unsigned long stval;


	stval = read_csr(CSR_STVAL);
	print("%s %d scause:0x%lx sepc:0x%lx stval:0x%lx\n",
		  __FUNCTION__, __LINE__, regs->scause, regs->sepc, stval);
	//based on the objdump info of internal_readl, the sepc value should be internal_readl addr + 6
	if (regs->scause == EXC_LOAD_PAGE_FAULT &&
		regs->sepc == (unsigned long) internal_readl + 6 &&
		regs->sbadaddr == INVALID_ADDRESS &&
		(stval == 0x0 || stval == INVALID_ADDRESS)) {
		print("TEST PASS\n");
	} else {
		print("TEST FAIL\n");
	}
}

static int load_page_test(void)
{
	int val = 0;

	register_handle_exception_stub_handler(load_page_stub_handler);

	val = internal_readl(INVALID_ADDRESS);
	print("0x%lx = %x\n", INVALID_ADDRESS, val);
	return 0;
}

static void store_page_stub_handler(struct pt_regs *regs)
{
	unsigned long stval;

	stval = read_csr(CSR_STVAL);
	print("%s %d scause:0x%lx sepc:0x%lx stval:0x%lx\n",
		  __FUNCTION__, __LINE__, regs->scause, regs->sepc, stval);
	//based on the objdump info of internal_writel, the sepc value should be internal_writel addr + 20
	if (regs->scause == EXC_STORE_PAGE_FAULT &&
		regs->sepc == (unsigned long) internal_writel + 20 &&
		regs->sbadaddr == INVALID_ADDRESS &&
		(stval == 0x0 || stval == INVALID_ADDRESS)) {
		print("TEST PASS\n");
	} else {
		print("TEST FAIL\n");
	}
}

static int store_page_test(void)
{
	int val = 0;

	register_handle_exception_stub_handler(store_page_stub_handler);

	internal_writel(INVALID_ADDRESS, val);
	print("0x%lx = %x\n", INVALID_ADDRESS, val);
	return 0;
}

static void load_access_stub_handler(struct pt_regs *regs)
{
	unsigned long stval;

	stval = read_csr(CSR_STVAL);
	print("%s %d scause:0x%lx sepc:0x%lx stval:0x%lx\n",
		  __FUNCTION__, __LINE__, regs->scause, regs->sepc, stval);
	//based on the objdump info of internal_readl, the sepc value should be internal_readl addr + 6
	print("ref_addr:0x%lx\n", ref_addr);
	if (regs->scause == EXC_LOAD_ACCESS &&
		regs->sepc == (unsigned long) internal_readl + 6 &&
		regs->sbadaddr == ref_addr &&
		(stval == 0x0 || stval == ref_addr)) {
		print("TEST PASS\n");
	} else {
		print("TEST FAIL\n");
	}
}

static int load_access_test(void)
{
	unsigned long val = 0;

	register_handle_exception_stub_handler(load_access_stub_handler);

	ref_addr = (unsigned long) ioremap(0, 4096, 0);

	val = internal_readl(ref_addr);
	print("0x%x = %x\n", ref_addr, val);
	return 0;
}

static void store_access_stub_handler(struct pt_regs *regs)
{
	unsigned long stval;

	stval = read_csr(CSR_STVAL);
	print("%s %d scause:0x%lx sepc:0x%lx stval:0x%lx\n",
		  __FUNCTION__, __LINE__, regs->scause, regs->sepc, stval);
	//based on the objdump info of internal_writel, the sepc value should be internal_writel addr + 20
	if (regs->scause == EXC_STORE_ACCESS &&
		regs->sepc == (unsigned long) internal_writel + 20 &&
		regs->sbadaddr == ref_addr &&
		(stval == 0x0 || stval == ref_addr)) {
		print("TEST PASS\n");
	} else {
		print("TEST FAIL\n");
	}
}

static int store_access_test(void)
{
	unsigned long val = 0;

	register_handle_exception_stub_handler(store_access_stub_handler);
	ref_addr = (unsigned long) ioremap(0, 4096, 0);

	internal_writel(ref_addr, val);
	print("0x%x = %x\n", ref_addr, val);
	return 0;
}

static void illegal_instruction_stub_handler(struct pt_regs *regs)
{
	unsigned long stval;

	stval = read_csr(CSR_STVAL);
	print("%s %d scause:0x%lx sepc:0x%lx stval:0x%lx\n",
		  __FUNCTION__, __LINE__, regs->scause, regs->sepc, stval);
	if (regs->scause == EXC_INST_ILLEGAL &&
		regs->sepc == (unsigned long) internal_illegal_instruction &&
		regs->sbadaddr == ILLEGAL_INSTRUCTION_ADDR &&
		(stval == 0x0 || stval == ILLEGAL_INSTRUCTION_ADDR)) {
		print("TEST PASS\n");
	} else {
		print("TEST FAIL\n");
	}
}

static int illegal_instruction_test(void)
{
	register_handle_exception_stub_handler(illegal_instruction_stub_handler);

	internal_illegal_instruction();
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

