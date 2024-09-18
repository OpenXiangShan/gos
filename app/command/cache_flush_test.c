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
#include "../command.h"
#include "cache_flush.h"
#include "string.h"
#include "mm.h"
#include "clock.h"
#include "asm/barrier.h"
#include "vmap.h"
#include "asm/pgtable.h"
#include "cpu.h"
#include "asm/sbi.h"

static void Usage(void)
{
	print("cbo inval [addr]\n");
	print("cbo clean [addr]\n");
	print("cbo flush [addr]\n");
	print("cbo zero [addr]\n");
}

static int cmd_cbo_handler(int argc, char *argv[], void *priv)
{
	unsigned long base;
	unsigned long va;
	pgprot_t pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	if (argc != 2) {
		print("Invalid input params\n");
		Usage();
		return -1;
	}

	va = (unsigned long)vmap_alloc(PAGE_SIZE);
	if (!va) {
		print("%s -- alloc va failed\n", __FUNCTION__);
		return -1;
	}

	base = atoi(argv[1]);
	mmu_page_mapping(base, va, PAGE_SIZE, pgprot);

	if (!strncmp(argv[0], "inval", sizeof("inval"))) {
		cbo_cache_inval(va);
	} else if (!strncmp(argv[0], "clean", sizeof("clean"))) {
		cbo_cache_clean(va);
	} else if (!strncmp(argv[0], "flush", sizeof("flush"))) {
		cbo_cache_flush(va);
	} else if (!strncmp(argv[0], "zero", sizeof("zero"))) {
		cbo_cache_zero(va);
	}

	vmap_free((void *)va, PAGE_SIZE);

	return 0;
}

static int prefetch_w_test(void)
{
	static int test __attribute__((used));
	int *addr, *addr2;
	unsigned long start;
	unsigned long cost_pre, cost_unpre;
	unsigned long mcounteren;
	unsigned long new;

	mcounteren = sbi_get_cpu_mcounteren();
	new = mcounteren | (1UL << 0);
	sbi_set_mcounteren(new);

	addr = (int *)mm_alloc(4096);
	addr2 = (int *)mm_alloc(4096);

	disable_local_irq();

	prefetch_r((unsigned long)addr);

	start = read_csr(cycle);
	test = *addr;
	cost_pre = read_csr(cycle) - start;

	start = read_csr(cycle);
	test = *addr2;
	cost_unpre = read_csr(cycle) - start;

	enable_local_irq();

	print("cost_pre: %dcycles cosr_unpre: %dcycles\n", cost_pre, cost_unpre);

	sbi_set_mcounteren(mcounteren);
	mm_free((void *)addr, 4096);
	mm_free((void *)addr2, 4096);

	return 0;
}

static int cmd_prefetch_handler(int argc, char *argv[], void *priv)
{
	unsigned long base;

	if (argc < 1) {
		print("Invalid input params\n");
		Usage();
		return -1;
	}

	if (argc == 2)
		base = atoi(argv[1]);
	if (!strncmp(argv[0], "i", sizeof("i"))) {
		prefetch_i(base);
	} else if (!strncmp(argv[0], "w", sizeof("w"))) {
		prefetch_w(base);
	} else if (!strncmp(argv[0], "r", sizeof("r"))) {
		prefetch_r(base);
	} else if (!strncmp(argv[0], "test", sizeof("test"))) {
		prefetch_w_test();
	}

	return 0;
}

static void cache_inval_test(void)
{
	void *addr;

	addr = mm_alloc(4096);
	if (!addr) {
		print("%s -- Out of memory!!\n", __FUNCTION__);
		return;
	}

	strcpy((char *)addr, "This is content in cache");
	print("before cache inval\n");
	print("%s\n", (char *)addr);

	cbo_cache_inval((unsigned long)addr);
	mb();

	print("after cache inval\n");
	print("%s\n", (char *)addr);

	mm_free(addr, 4096);
}

static void cache_clean_test(void)
{
	void *addr;
	unsigned long start, before, after;
	unsigned long mcounteren, new;

	mcounteren = sbi_get_cpu_mcounteren();
	new = mcounteren | (1UL << 0);
	sbi_set_mcounteren(new);

	addr = mm_alloc(4096);
	if (!addr) {
		print("%s -- Out of memory!!\n", __FUNCTION__);
		return;
	}

	strcpy((char *)addr, "This is content in cache");

	start = read_csr(cycle);
	print("%s\n", (char *)addr);
	before = read_csr(cycle) - start;

	cbo_cache_clean((unsigned long)addr);
	mb();

	start = read_csr(cycle);
	print("%s\n", (char *)addr);
	after = read_csr(cycle) - start;

	print("before clean cost : %dticks\n", before);
	print("after clean cost : %dticks\n", after);

	sbi_set_mcounteren(mcounteren);

	mm_free((void *)addr, 4096);
}

static void cache_flush_test(void)
{
	void *addr;
	unsigned long start, before, after;
	unsigned long mcounteren, new;

	mcounteren = sbi_get_cpu_mcounteren();
	new = mcounteren | (1UL << 0);
	sbi_set_mcounteren(new);

	addr = mm_alloc(4096);
	if (!addr) {
		print("%s -- Out of memory!!\n", __FUNCTION__);
		return;
	}

	strcpy((char *)addr, "This is content in cache before flush");

	disable_local_irq();

	start = read_csr(cycle);
	print("%s\n", (char *)addr);
	before = read_csr(cycle) - start;

	strcpy((char *)addr, "This is content in cache after flush");

	cbo_cache_flush((unsigned long)addr);
	mb();

	start = read_csr(cycle);
	print("%s\n", (char *)addr);
	after = read_csr(cycle) - start;

	enable_local_irq();
	print("before flush cost : %dticks\n", before);
	print("after flush cost : %dticks\n", after);

	sbi_set_mcounteren(mcounteren);

	mm_free((void *)addr, 4096);
}

static void cache_zero_test(void)
{
	void *addr;

	addr = mm_alloc(4096);
	if (!addr) {
		print("%s -- Out of memory!!\n", __FUNCTION__);
		return;
	}

	strcpy((char *)addr, "This is content in cache");
	print("before cache zero: \n");
	print("    %s\n", (char *)addr);

	cbo_cache_zero((unsigned long)addr);
	mb();

	print("after cache zero: \n");
	print("    %s\n", (char *)addr);

	mm_free(addr, 4096);
}

static int cmd_cache_flush_test_handler(int argc, char *argv[], void *priv)
{
	if (!strncmp(argv[0], "inval", sizeof("inval"))) {
		cache_inval_test();
	} else if (!strncmp(argv[0], "clean", sizeof("clean"))) {
		cache_clean_test();
	} else if (!strncmp(argv[0], "flush", sizeof("flush"))) {
		cache_flush_test();
	} else if (!strncmp(argv[0], "zero", sizeof("zero"))) {
		cache_zero_test();
	}

	return 0;
}

static const struct command cmd_cbo_test = {
	.cmd = "cbo",
	.handler = cmd_cbo_handler,
	.priv = NULL,
};

static const struct command cmd_prefetch_test = {
	.cmd = "prefetch",
	.handler = cmd_prefetch_handler,
	.priv = NULL,
};

static const struct command cmd_cache_flush_test = {
	.cmd = "cache_flush_test",
	.handler = cmd_cache_flush_test_handler,
	.priv = NULL,
};

int cmd_cache_flush_test_init()
{
	register_command(&cmd_cbo_test);
	register_command(&cmd_prefetch_test);
	register_command(&cmd_cache_flush_test);

	return 0;
}

APP_COMMAND_REGISTER(cache_flush_test, cmd_cache_flush_test_init);
