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

#include "asm/type.h"
#include "asm/result.h"
#include "print.h"
#include "device.h"
#include "../command.h"
#include "vmap.h"
#include "asm/pgtable.h"
#include "string.h"
#include "mm.h"
#include "asm/tlbflush.h"

#define GOOD_RESULT "Excepted result"
#define BAD_RESULT "Unexcepted result"

static int cmd_svnapot_test_ext_handler(int argc, char *argv[], void *priv)
{
	void *addr_napot;
	void *addr_common[64];
	unsigned long *pte[64];
	int size_napot = PAGE_64K_SIZE;
	int pnum = PAGE_64K_SIZE / PAGE_SIZE, i;

	addr_napot = (void *)vmem_alloc_huge(size_napot, PAGE_64K_SIZE, NULL);
	if (!addr_napot) {
		print("Out of memory!\n");
		return -1;
	}

	for (i = 0; i < pnum; i++) {
		char *addr = (char *)addr_napot + i * PAGE_SIZE;
		strcpy(addr, GOOD_RESULT);
		pte[i] = mmu_get_pte((unsigned long)addr);
		print("va:0x%lx pte:0x%lx\n", addr, *pte[i]);
	}

	for (i = 0; i < pnum - 1; i++) {
		char *addr;

		addr_common[i] = mm_alloc(PAGE_SIZE);
		if (!addr_common[i]) {
			print("Out of memory!\n");
			return -1;
		}

		addr = (char *)addr_common[i];
		strcpy(addr, BAD_RESULT);
	}

	print("1. Load addr_napot to let it fill into tlb, %s\n", (char *)addr_napot);
	print("2. Modify the last %d PTEs in napot\n", pnum - 1);
	for (i = 0; i < pnum - 1; i++) {
		pgprot_t pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);
		unsigned long addr = (unsigned long)addr_common[i];
		unsigned long pfn = virt_to_phy(addr) >> PAGE_SHIFT;
		*pte[i + 1] = pfn_pte(pfn, pgprot);
	}

	mb();

	print("3. Load the last %d page in napot\n", pnum - 1);
	for (i = 0; i < pnum - 1; i++) {
		char *addr = (char *)addr_napot + (i + 1) * PAGE_SIZE;
		print("addr:0x%lx -- %s\n", addr, addr);
		if (strcmp(addr, GOOD_RESULT))
			TEST_FAIL;
	}

	print("4. Load the last %d page after sfence\n", pnum - 1);

	local_flush_tlb_all();
	for (i = 0; i < pnum - 1; i++) {
		char *addr = (char *)addr_napot + (i + 1) * PAGE_SIZE;
		print("addr:0x%lx -- %s\n", addr, addr);
		if (strcmp(addr, BAD_RESULT))
			TEST_FAIL;
	}


	TEST_PASS;

	return 0;
}

static int cmd_svnapot_test_handler(int argc, char *argv[], void *priv)
{
	char *addr;
	int size = PAGE_64K_SIZE, i;
	int pnum = PAGE_64K_SIZE / PAGE_SIZE;
	unsigned long *pte;

	addr = (char *)vmem_alloc_huge(size, PAGE_64K_SIZE, NULL);

	for (i = 0; i < pnum; i++) {
		addr[i * PAGE_SIZE] = i;
		addr[i * PAGE_SIZE + PAGE_SIZE - 1] = 2*i;
		pte = mmu_get_pte((unsigned long)(addr + i * PAGE_SIZE));
		print("pte:0x%lx\n", *pte);
		print("addr[%d]:%d addr[%d]:%d\n",
			i * PAGE_SIZE, addr[i * PAGE_SIZE],
			i * PAGE_SIZE + PAGE_SIZE - 1,
			addr[i * PAGE_SIZE + PAGE_SIZE - 1]);
	}

	print("TEST PASS\n");

	return 0;
}

static const struct command cmd_svnapot_test = {
	.cmd = "svnapot_test",
	.handler = cmd_svnapot_test_handler,
	.priv = NULL,
};

static const struct command cmd_svnapot_test_ext = {
	.cmd = "svnapot_test_ext",
	.handler = cmd_svnapot_test_ext_handler,
	.priv = NULL,
};

int svnapot_test_init()
{
	register_command(&cmd_svnapot_test);
	register_command(&cmd_svnapot_test_ext);

	return 0;
}

APP_COMMAND_REGISTER(svnapot_test, svnapot_test_init);
