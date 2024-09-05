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
#include "command.h"
#include "vmap.h"
#include "asm/pgtable.h"
#include "asm/csr.h"
#include "string.h"
#include "gos/mm.h"
#include "asm/tlbflush.h"

#define DIS_PAGE_TABLE  0x1FF
static char *str[]={"hfence test -- This is pa1", "hfence test -- This is pa2",
	"hfence test -- This is pa3", "hfence test -- This is pa4"};

static void Usage(void)
{
	print("Usage: page_tlb_test [cmd] [param] [control]\n");
	print("cmd option:\n");
	print("    -- Acc (page table access bit test)\n");
	print("    -- hfence.vma_all (hfence.vma test)\n");
	print("    -- satp_bare_test (set satp is bare mode)\n");
	print("    -- pte_flag_test (modify pte flag bit)\n");
	print("    -- hfence.addr (flush spec addr)\n");
	print("    -- hfence.asid (flush spec asid)\n");
	print("    -- pte_g_test (pte global test)\n");
	print("param option:\n");
	print("    -- cmd: pte_flag_test (modify pte flag bit)\n");
	print("    param value is 0x1ff, display current pte value, the param flag bits are as follows\n");
	print("     | 9             8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 \n");
	print("       reserved for SW   D   A   G   U   X   W   R   V  \n");
	print("    -- cmd: pte_g_test (pte global test)\n");
	print("       value=0 (not flush the tlb)\n");
	print("       value=1 (use hfence.va flush tlb)\n");
	print("       value=2 (use hfence.asid flush tlb)\n");
	print("       value=3 (use hfence.va and hfence.asid flush tlb)\n");
	print("       value=4 (flush all tlb)\n");
	print("control option:\n");
	print("    -- cmd: pte_flag_test (modify pte flag bit)\n");
	print("       value=1 (After changing the page table, first access the page table and then flush the cache)\n");
	print("       value=2 (After changing the page table, first flush the cache and then access the page table)\n");
}

static void flush_tlb_asid()
{
	unsigned long old;
	unsigned long asid_bits;

	old = read_csr(CSR_SATP);
	asid_bits = (old >> SATP_ASID_SHIFT)  & SATP_ASID_MASK;
	local_flush_tlb_all_asid(asid_bits);
}

static void flush_tlb_asid_addr(unsigned long addr)
{
	unsigned long old;
	unsigned long asid_bits;

	old = read_csr(CSR_SATP);
	asid_bits = (old >> SATP_ASID_SHIFT)  & SATP_ASID_MASK;

	local_flush_tlb_page_asid(addr, asid_bits);
}

static int v_p_address_mapping(void *va, char *c1, char *c2, char flag,  pgprot_t pgprot)
{
	void *addr, *pa1, *pa2;

	addr = mm_alloc(PAGE_SIZE);
	if (!addr) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret1;
	}
	pa1 = (void *)virt_to_phy(addr);
	strcpy((char *)addr, (char *)c1);
	print("print value in 0x%lx(pa1): %s\n", pa1, (char *)addr);

	addr = mm_alloc(PAGE_SIZE);
	if (!addr) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret2;
	}
	pa2 = (void *)virt_to_phy(addr);
	strcpy((char *)addr, (char *)c2);
	print("print value in 0x%lx(pa2): %s\n", pa2, (char *)addr);

	print("pa1:0x%lx pa2:0x%lx va:0x%lx\n", pa1, pa2, va);


	print("Now map 0x%lx(pa) to 0x%lx(va)\n", pa1, va);
	if (-1 ==
	    mmu_page_mapping((unsigned long)pa1, (unsigned long)va, PAGE_SIZE,
			     pgprot)) {
		print("%s -- page mapping failed\n", __FUNCTION__);
		return -1;
	}
	print("load 0x%lx(--> 0x%lx)(make entry into tlb): %s\n", va, pa1, va);

	print("Now map 0x%lx(pa) to the same va(0x%lx)\n", pa2, va);
	if (-1 ==
	    mmu_page_mapping_no_sfence((unsigned long)pa2, (unsigned long)va, PAGE_SIZE,
					pgprot)) {
		print("%s -- page mapping failed\n", __FUNCTION__);
		return -1;
	}

	print("test start --> load 0x%lx,(before hfence.vma)\n", va);
	print("0x%lx : %s\n", va, (char *)va);

	if (flag == 1) {
		print(" ============== hfence.va flush tlb ==============\n");
		local_flush_tlb_page((unsigned long )va);
	}else if (flag == 2) {
		print(" ============== hfence.asid flush tlb ==============\n");
		flush_tlb_asid();
	}else if (flag == 3) {
		print(" ========== hfence.asid and hfence.addr flush tlb ====\n");
		flush_tlb_asid_addr((unsigned long)va);
	}else if (flag == 4) {
		print(" ========== flush all tlb ====\n");
		local_flush_tlb_all();
	}else if (flag == 0)
		print(" ============== Not flush tlb ============\n");
	print("test start --> load 0x%lx(after hfence.vma)\n", va);
	print("0x%lx : %s\n", va, (char *)va);

ret2:
	mm_free((void *)phy_to_virt(pa2), PAGE_SIZE);
ret1:
	mm_free((void *)phy_to_virt(pa1), PAGE_SIZE);

	return 0;
}

/*
 * flag=0, not flush all tlb
 * flag=1, va not flush, using the hfence.addr command to refresh tlb
 * flag=2, va1 not flush, using the hfence.asid command to refresh tlb
 *
 */
static void hfence_param_test(char flag)
{
	int ret = 0;
	void *va, *va1;
	pgprot_t pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	va = vmap_alloc(PAGE_SIZE);
	if (!va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto err;
	}

	va1 = vmap_alloc(PAGE_SIZE);
	if (!va1) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto err1;
	}

	if (flag == 1) {
		ret =  v_p_address_mapping(va, str[0], str[1], 0, pgprot);
		if (ret == -1)
			goto err;
		print("-------------------------------------------------------- \n");
		ret =  v_p_address_mapping(va1, str[2], str[3], 1, pgprot);
		if (ret == -1)
			goto err1;
	}else if (flag == 2) {
		ret =  v_p_address_mapping(va, str[0], str[1], 2, pgprot);
		if (ret == -1)
			goto err;
		print("-------------------------------------------------------- \n");
		ret =  v_p_address_mapping(va1, str[2], str[3], 0, pgprot);
		if (ret == -1)
			goto err1;
	}
err:
	vmap_free(va, PAGE_SIZE);
err1:
	vmap_free(va1, PAGE_SIZE);
}

static int hfence_g_test(char *param)
{
	int ret = 0;
	void *va;
	int pte_flag;
	pgprot_t pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY | _PAGE_GLOBAL);

	if (param == NULL){
		print("Please set pet flag value! \n");
		return -1;
	}
	else
		pte_flag = atoi(param);

	va = vmap_alloc(PAGE_SIZE);
	if (!va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto err;
	}
	ret =  v_p_address_mapping(va, str[0], str[1], pte_flag, pgprot);
	if (ret == -1)
		goto err;

err:
	vmap_free(va, PAGE_SIZE);

	return 0;
}

/*
 * PTE format:
 * | XLEN-1  10 | 9             8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0
 *       PFN      reserved for SW   D   A   G   U   X   W   R   V
 */
static int page_table_flag_test(char *param, char *cflag)
{
	void *vaddr, *pa;
	unsigned long *pte;
	unsigned long pte_val;
	int pte_flag, fence_flag;
	pgprot_t pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY | _PAGE_GLOBAL);

	if (strlen(param) == 0)  //param is empty
		print("Please set pet flag value! \n");
	else
		pte_flag = atoi(param);

	vaddr = vmap_alloc(PAGE_SIZE);
	if (!vaddr) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret;
	}

	pa = (void *)virt_to_phy(mm_alloc(PAGE_SIZE));
	if (-1 ==
	    mmu_page_mapping((unsigned long)pa, (unsigned long)vaddr, PAGE_SIZE,
			     pgprot)) {
		print("%s -- page mapping failed\n", __FUNCTION__);
		return -1;
	}

	print("vaddr:0x%lx\n", vaddr);
	strcpy(vaddr, "Hello");
	pte = mmu_get_pte((unsigned long)vaddr);
	if (!pte)
		return 0;
	pte_val = *pte;

	print("pte:0x%lx pte_val:0x%lx\n", pte, pte_val);
	print("%s\n", vaddr);
	/*
         *
         *   |PPN[2]|PPN[1]|PPN[0]|Page offset| >> _PAGE_PFN_SHIFT ==>  |PPN[2]|PPN[1]|PPN[0]|
         *
         *    |PPN[2]|PPN[1]|PPN[0]| << _PAGE_PFN_SHIFT ==> |PPN[2]|PPN[1]|PPN[0]|0... |
         *
         *    |PPN[2]|PPN[1]|PPN[0]|0... | | pte_fag  ==> |PPN[2]|PPN[1]|PPN[0]|pte_flag|
         *
         */
	if (pte_flag != DIS_PAGE_TABLE) {
		if (strlen(cflag) == 0)  //control is empty
			print("Please set pet control! \n");
		else
			fence_flag = atoi(cflag);

		pte_val = (((pte_val >> PAGE_SHIFT) << PAGE_SHIFT) | pte_flag);
		print("remove bit -- pte_val:0x%lx\n", pte_val);

		if (fence_flag == 1) {
			*pte = pte_val;
			local_flush_tlb_all();
		}else if (fence_flag == 2) {
			local_flush_tlb_all();
			*pte = pte_val;
		}
		print("%s\n", vaddr);
		print("TEST PASS\n");
    }
ret:
	vmem_free(vaddr, PAGE_SIZE);

	return 0;
}


unsigned long alloc_one_page(int size)
{
	void *ptr = mm_alloc(size);

	if (!ptr) {
		print("%s - %s -- Out of memory\n", __FUNCTION__, __LINE__);
                return NULL;
	}
	memset(ptr, 0, size);

	return(virt_to_phy(ptr));
}

/*
 * satp mode is bare, then flash tlb is exception
 */
static int flush_satp(void)
{
	unsigned long satp;

	void *va = mm_alloc(PAGE_SIZE);

        if (!va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret;
	}
	memset(va, 0, PAGE_SIZE);
	strcpy((char *)	va, "===test===");
	print("va value is : %s\n", (char *)va);

	satp = read_csr(CSR_SATP);
	print("read satp value:0x%lx \n", satp);
	satp &=~ SATP_MODE_BARE;
	print("write satp value:0x%lx, set satp is bare mode \n", satp);
	write_csr(CSR_SATP, satp);

	local_flush_tlb_all();
	print("get va value again: %s\n", (char *)va);
	while(1);

ret:
	vmap_free(va, PAGE_SIZE);
	return 0;
}

void satp_mode_bare_test()
{
	pgprot_t pgprot;
	unsigned long stack;

	stack = alloc_one_page(PAGE_SIZE);
	if (stack == NULL) {
		goto ret;
	}
	pgprot =
		__pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC |
				_PAGE_DIRTY);
	mmu_page_mapping(stack, stack, PAGE_SIZE, pgprot);
	flush_satp();
ret:
    mm_free((void *)stack, PAGE_SIZE);
}

static void page_table_hfence_all_test()
{
	pgprot_t pgprot;
	void *addr;
	void *va, *pa1, *pa2;
	int i = 0;

	addr = mm_alloc(PAGE_SIZE);
	if (!addr) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret;
	}
	pa1 = (void *)virt_to_phy(addr);
	strcpy((char *)addr, "hfence test -- This is pa1");
	print("print value in 0x%lx(pa1): %s\n", pa1, (char *)addr);

	addr = mm_alloc(PAGE_SIZE);
	if (!addr) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret2;
	}
	pa2 = (void *)virt_to_phy(addr);
	strcpy((char *)addr, "hfence test -- This is pa2");
	print("print value in 0x%lx(pa2): %s\n", pa2, (char *)addr);

	va = vmap_alloc(PAGE_SIZE);
	if (!va) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret3;
	}

	print("pa1:0x%lx pa2:0x%lx va:0x%lx\n", pa1, pa2, va);

	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	print("Now map 0x%lx(pa) to 0x%lx(va)\n", pa1, va);
	if (-1 ==
	    mmu_page_mapping((unsigned long)pa1, (unsigned long)va, PAGE_SIZE,
			     pgprot)) {
		print("%s -- page mapping failed\n", __FUNCTION__);
		goto ret4;
	}
	print("load 0x%lx(--> 0x%lx)(make entry into tlb): %s\n", va, pa1, va);

	print("Now map 0x%lx(pa) to the same va(0x%lx)\n", pa2, va);
	if (-1 ==
	    mmu_page_mapping_no_sfence((unsigned long)pa2, (unsigned long)va, PAGE_SIZE,
					pgprot)) {
		print("%s -- page mapping failed\n", __FUNCTION__);
		goto ret4;
	}

	print("test start --> load 0x%lx(before hfence.vma)\n", va);
	for (i = 0; i < 5; i++) {
		print("0x%lx : %s\n", va, (char *)va);
	}

	local_flush_tlb_all();

	print("test start --> load 0x%lx(after hfence.vma)\n", va);
	for (i = 0; i < 5; i++) {
		print("0x%lx : %s\n", va, (char *)va);
	}
ret4:
	vmap_free(va, PAGE_SIZE);
ret3:
	mm_free((void *)phy_to_virt(pa2), PAGE_SIZE);
ret2:
	mm_free((void *)phy_to_virt(pa1), PAGE_SIZE);
ret:
	return;
}

static int cmd_tlb_test_handler(int argc, char *argv[], void *priv)
{
	if (argc < 1) {
		print("Invalid input params\n");
		Usage();
		return -1;
	}

	if (!strncmp(argv[0], "hfence.vma_all", sizeof("hfence.vma_all"))) {
		page_table_hfence_all_test();
	}
	else if (!strncmp(argv[0], "satp_bare_test", sizeof("satp_bare_test"))) {
		satp_mode_bare_test();
	} else if (!strncmp(argv[0], "pte_flag_test", sizeof("pte_flag_test"))) {
		page_table_flag_test(argv[1], argv[2]);
	} else if (!strncmp(argv[0], "hfence.addr", sizeof("hfence.addr"))) {
		hfence_param_test(1);
	} else if (!strncmp(argv[0], "hfence.asid", sizeof("hfence.asid"))) {
		hfence_param_test(2);
	} else if (!strncmp(argv[0], "pte_g_test", sizeof("pte_g_test"))) {
		hfence_g_test(argv[1]);
	} else {
		print("Unsupport command\n");
		Usage();
		return -1;
	}

	return 0;
}

static const struct command cmd_tlb_test = {
	.cmd = "tlb_test",
	.handler = cmd_tlb_test_handler,
	.priv = NULL,
};

int tlb_test_init()
{
	register_command(&cmd_tlb_test);

	return 0;
}

APP_COMMAND_REGISTER(tlb_test, tlb_test_init);
