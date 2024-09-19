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
#include "string.h"
#include "mm.h"
#include "asm/tlbflush.h"
#include "asm/csr.h"
#include "gos.h"
#include "asm/sbi.h"
#include "vmap.h"

#define DIS_PAGE_TABLE  0x1FF
static char *str[]={"sinval test -- This is pa1", "sinval test -- This is pa2",
	"sinval test -- This is pa3", "sinval test -- This is pa4"};

static void Usage(void)
{
	print("Usage: page_tlb_test [cmd] [param] [control]\n");
	print("cmd option:\n");
	print("    -- sinval.vma_all (sinval.vma test)\n");
	print("    -- pte_flag_test (modify pte flag bit)\n");
	print("    -- sinval.addr (flush spec addr)\n");
	print("    -- sinval.asid (flush spec asid)\n");
	print("    -- pte_g_test (pte global test)\n");
	print("param option:\n");
	print("    -- cmd: pte_flag_test (modify pte flag bit)\n");
	print("    param value is 0x1ff, display current pte value, the param flag bits are as follows\n");
	print("     | 9             8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 \n");
	print("       reserved for SW   D   A   G   U   X   W   R   V  \n");
	print("    -- cmd: pte_g_test (pte global test)\n");
	print("       value=0(not flush the tlb)\n");
	print("       value=1 (use sinval.va flush tlb)\n");
	print("       value=2 (use sinval.asid flush tlb)\n");
	print("       value=3 (use sinval.va and sinval.asid flush tlb)\n");
	print("       value=4 (flush all tlb)\n");
	print("control option:\n");
	print("    -- cmd: pte_flag_test (modify pte flag bit)\n");
	print("       value=1(After changing the page table, first access the page table and then flush the cache)\n");
	print("       value=2(After changing the page table, first flush the cache and then access the page table)\n");
}

static void flush_tlb_asid()
{
	unsigned long old;
	unsigned long asid_bits;

	old = read_csr(CSR_SATP);
	asid_bits = (old >> SATP_ASID_SHIFT)  & SATP_ASID_MASK;
	mb();
	sinval_asid(asid_bits);
	mb();
}

static void flush_tlb_asid_addr(unsigned long addr)
{
	unsigned long old;
	unsigned long asid_bits;

	old = read_csr(CSR_SATP);
	asid_bits = (old >> SATP_ASID_SHIFT)  & SATP_ASID_MASK;
	mb();
	sinval_va_asid(addr, asid_bits);
	mb();
}

/*
 * PTE format:
 * | XLEN-1  10 | 9             8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0
 *       PFN      reserved for SW   D   A   G   U   X   W   R   V
 */
static void page_table_flag_test(char *param, char *cflag)
{
	void *vaddr;
	unsigned long *pte;
	unsigned long pte_val;
	int pte_flag, fence_flag;

	if (strlen(param) == 0)  //param is empty
		print("Please set pet flag value! \n");
	else
		pte_flag = atoi(param);


	vaddr = mm_alloc(PAGE_SIZE);
	if (!vaddr) {
		goto ret;
	}

	print("vaddr:0x%lx\n", vaddr);
	strcpy(vaddr, "Hello");
	pte = mmu_get_pte((unsigned long)vaddr);
	if (!pte)
		return;
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

		print("TEST PASS\n");

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

	print("test start --> load 0x%lx,(before sinval.vma)\n", va);
	print("0x%lx : %s\n", va, (char *)va);

	if (flag == 1) {
		print(" ============== sinval.va flush tlb ==============\n");
		mb();
		sinval_va((unsigned long )va);
		mb();
	}else if (flag == 2) {
		print(" ============== sinval.asid flush tlb ==============\n");
		flush_tlb_asid();
	}else if (flag == 3) {
		print(" ========== sinval.asid and sinval.addr flush tlb ====\n");
		flush_tlb_asid_addr((unsigned long)va);
	}else if (flag == 4) {
		print(" ========== flush all tlb ====\n");
		mb();
		sinval_all();
		mb();
	}else if (flag == 0)
		print(" ============== Not flush tlb ============\n");
	print("test start --> load 0x%lx(after sinval.vma)\n", va);
	print("0x%lx : %s\n", va, (char *)va);

ret2:
	mm_free((void *)phy_to_virt(pa2), PAGE_SIZE);
ret1:
	mm_free((void *)phy_to_virt(pa1), PAGE_SIZE);

	return 0;
}

/*
 * flag=0, not flush all tlb
 * flag=1, va not flush, using the sinval.addr command to refresh tlb
 * flag=2, va1 not flush, using the sinval.asid command to refresh tlb
 *
 */
static void sinval_param_test(char flag)
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
		print("TEST PASS\n");
	}else if (flag == 2) {
		ret =  v_p_address_mapping(va, str[0], str[1], 2, pgprot);
		if (ret == -1)
			goto err;
		print("-------------------------------------------------------- \n");
		ret =  v_p_address_mapping(va1, str[2], str[3], 0, pgprot);
		if (ret == -1)
			goto err1;
		print("TEST PASS\n");
	}
err:
	vmap_free(va, PAGE_SIZE);
err1:
	vmap_free(va1, PAGE_SIZE);
}

static int sinval_g_test(char *param)
{
	int ret = 0;
	void *va;
	int pte_flag;
	pgprot_t pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY | _PAGE_GLOBAL);

	if (param == NULL){  //param is empty
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

static void sinval_vma_all_test(void)
{
	pgprot_t pgprot;
	void *addr;
	void *va, *pa1, *pa2;
	int i = 0;
	unsigned long mask1, mask2;

	addr = mm_alloc(PAGE_SIZE);
	if (!addr) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret;
	}
	pa1 = (void *)virt_to_phy(addr);
	strcpy((char *)addr, "sinval test -- This is pa1");
	print("print value in 0x%lx(pa1): %s\n", pa1, (char *)addr);

	addr = mm_alloc(PAGE_SIZE);
	if (!addr) {
		print("%s -- Out of memory\n", __FUNCTION__);
		goto ret2;
	}
	pa2 = (void *)virt_to_phy(addr);
	strcpy((char *)addr, "sinval test -- This is pa2");
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
	    mmu_page_mapping_no_sfence((unsigned long)pa2, (unsigned long)va,
				       PAGE_SIZE, pgprot)) {
		print("%s -- page mapping failed\n", __FUNCTION__);
		goto ret4;
	}

	print("test start --> load 0x%lx(before sinval.vma)\n", va);
	for (i = 0; i < 5; i++) {
		print("0x%lx : %s\n", va, (char *)va);
		if (!strcmp((char *)va, "sinval test -- This is pa1")) {
			mask1 |= 1 << i;
		}
	}

	mb();
	sinval_all();
	mb();

	print("test start --> load 0x%lx(after sinval.vma)\n", va);
	for (i = 0; i < 5; i++) {
		print("0x%lx : %s\n", va, (char *)va);
		if (!strcmp((char *)va, "sinval test -- This is pa2")) {
			mask2 |= 1 << i;
		}
	}

	if ((mask1 & 0x1f) == 0x1f && (mask2 & 0x1f) == 0x1f) {
		print("TEST PASS\n");
	} else {
		print("TEST FAIL\n");
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

static int cmd_sinval_test_handler(int argc, char *argv[], void *priv)
{
	if (argc < 1) {
		print("Invalid input params\n");
		Usage();
		return -1;
	}

	if (!strncmp(argv[0], "sinval.vma_all", sizeof("sinval.vma_all"))) {
		sinval_vma_all_test();
	} else if (!strncmp(argv[0], "pte_flag_test", sizeof("pte_flag_test"))) {
		page_table_flag_test(argv[1], argv[2]);
	} else if (!strncmp(argv[0], "sinval.addr", sizeof("sinval.addr"))) {
		sinval_param_test(1);
	} else if (!strncmp(argv[0], "sinval.asid", sizeof("sinval.asid"))) {
		sinval_param_test(2);
	} else if (!strncmp(argv[0], "pte_g_test", sizeof("pte_g_test"))) {
		sinval_g_test(argv[1]);
		print("TEST PASS\n");
	} else {
		print("Unsupport command\n");
		Usage();
		return -1;
	}

	return 0;
}

static const struct command cmd_sinval_test = {
	.cmd = "sinval_test",
	.handler = cmd_sinval_test_handler,
	.priv = NULL,
};

int sinval_test_init()
{
	register_command(&cmd_sinval_test);

	return 0;
}

APP_COMMAND_REGISTER(sinval_test, sinval_test_init);
