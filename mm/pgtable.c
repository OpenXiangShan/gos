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

#include <asm/pgtable.h>
#include <asm/type.h>
#include "device.h"
#include "mm.h"
#include "string.h"
#include "print.h"
#include "asm/tlbflush.h"
#include "gos.h"
#include "asm/bitops.h"
#include "irq.h"

static void *pgdp = NULL;

int pgtable_l4_enabled = 0;
int pgtable_l5_enabled = 0;

int mmu_is_on = 0;
unsigned long va_pa_offset = 0;

extern unsigned long bss_end;
extern unsigned long __start_gos;

static unsigned long *riscv_pt_walk_alloc(unsigned long *ptp,
					  unsigned long va,
					  unsigned int shift, int pgsize,
					  int root,
					  unsigned long (*pg_alloc)(int gfp),
					  int gfp)
{
	unsigned long *pte, ptr, pfn;

	if (root) {
		if (!mmu_is_on)
			pte = (unsigned long *)(ptp + ((va >> shift) & 0x1FF));
		else
			pte =
			    (unsigned long *)phy_to_virt(ptp +
							 ((va >> shift) &
							  0x1FF));
	} else {
		if (!mmu_is_on)
			pte = (unsigned long *)((unsigned long *)
						pfn_to_phys(pte_pfn(*ptp)) +
						((va >> shift) & 0x1FF));
		else
			pte = (unsigned long *)phy_to_virt((unsigned long *)
							   pfn_to_phys(pte_pfn
								       (*ptp)) +
							   ((va >> shift) &
							    0x1FF));
	}

	if ((1UL << shift) <= pgsize) {
		return pte;
	}

	if (*pte == NULL) {
		ptr = pg_alloc ? pg_alloc(gfp) : 0;
		if (!ptr)
			return NULL;
		pfn = ptr >> PAGE_SHIFT;
		*pte = (pfn << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;
	}

	return riscv_pt_walk_alloc(pte, va, shift - 9, pgsize, 0,
				   pg_alloc, gfp);
}

static unsigned long *mmu_pt_walk_fetch_one(unsigned long *ptp, unsigned long va,
					    unsigned int shift, int root)
{
	unsigned long *pte;

	if (root) {
		if (!mmu_is_on)
			pte = (unsigned long *)(ptp + ((va >> shift) & 0x1FF));
		else
			pte =
			    (unsigned long *)phy_to_virt(ptp +
							 ((va >> shift) &
							  0x1FF));
	} else {
		if (!mmu_is_on)
			pte =
			    (unsigned long *)(pfn_to_phys(pte_pfn(*ptp)) +
					      ((va >> shift) & 0x1FF));
		else
			pte = (unsigned long *)(phy_to_virt((unsigned long *)
							    pfn_to_phys(pte_pfn
									(*ptp))
							    +
							    ((va >> shift) &
							     0x1FF)));
	}

	return pte;
}

static unsigned long *mmu_pt_walk_fetch(unsigned long *ptp, unsigned long va,
					unsigned int shift, int root)
{
	unsigned long *pte;

	if (root) {
		if (!mmu_is_on)
			pte = (unsigned long *)(ptp + ((va >> shift) & 0x1FF));
		else
			pte =
			    (unsigned long *)phy_to_virt(ptp +
							 ((va >> shift) &
							  0x1FF));
	} else {
		if (!mmu_is_on)
			pte =
			    (unsigned long *)(pfn_to_phys(pte_pfn(*ptp)) +
					      ((va >> shift) & 0x1FF));
		else
			pte = (unsigned long *)(phy_to_virt((unsigned long *)
							    pfn_to_phys(pte_pfn
									(*ptp))
							    +
							    ((va >> shift) &
							     0x1FF)));
	}

	if (pmd_leaf(*pte))
		return pte;
	else if (pmd_none(*pte))
		return NULL;
	else if (shift == PAGE_SHIFT)
		return NULL;

	return mmu_pt_walk_fetch(pte, va, shift - 9, 0);
}

static void *__walk_pt_va_to_pa(unsigned long va, int page_size)
{
	unsigned long *pte;

	pte = mmu_pt_walk_fetch(pgdp, va, PGDIR_SHIFT, 1);

	if (!pte) {
		print("%s -- pt walk fetch pte is NULL\n", __FUNCTION__);
		return 0;
	}
	if (!pmd_present(*pte)) {
		print("%s -- pte entry is not persent\n", __FUNCTION__);
		return 0;
	}

	return (void *)((pfn_to_phys(pte_pfn(*pte)) | (va & (page_size - 1))));
}

void *walk_pt_va_to_pa(unsigned long va)
{
	return __walk_pt_va_to_pa(va, PAGE_SIZE);
}

void *walk_pt_va_to_pa_huge(unsigned long va, int page_size)
{
	return __walk_pt_va_to_pa(va, page_size);
}

static int __mmu_page_mapping(unsigned long *_pgdp, unsigned long phy,
			      unsigned long virt, unsigned int size,
			      pgprot_t pgprot, int page_shift)
{
	int page_size = 1UL << page_shift;
	unsigned int page_nr = N_PAGE_EXT(size, page_size);
	unsigned long *pte;
	unsigned long pfn, pte_val;
	unsigned long phy_addr = phy;
	unsigned long virt_addr = virt;

	while (page_nr--) {
		pfn = (unsigned long)phy_addr >> PAGE_SHIFT;
		pte =
		    riscv_pt_walk_alloc(_pgdp,
					virt_addr, PGDIR_SHIFT, page_size, 1,
					alloc_zero_page, 0);
		if (!pte)
			return -1;

		pte_val = pfn_pte(pfn, pgprot);

		*pte = pte_val;

		virt_addr += page_size;
		phy_addr += page_size;
	}

	return 0;
}

static int __mmu_page_mapping_4k(unsigned long *_pgdp, unsigned long phy,
			      unsigned long virt, unsigned int size,
			      pgprot_t pgprot)
{
	return __mmu_page_mapping(_pgdp, phy, virt, size, pgprot, PAGE_SHIFT);
}

static int __mmu_page_mapping_2M(unsigned long *_pgdp, unsigned long phy,
			      unsigned long virt, unsigned int size,
			      pgprot_t pgprot)
{
	return __mmu_page_mapping(_pgdp, phy, virt, size, pgprot, PAGE_2M_SHIFT);
}

static int __mmu_page_mapping_1G(unsigned long *_pgdp, unsigned long phy,
			      unsigned long virt, unsigned int size,
			      pgprot_t pgprot)
{
	return __mmu_page_mapping(_pgdp, phy, virt, size, pgprot, PAGE_1G_SHIFT);
}

unsigned long *mmu_get_pte(unsigned long virt_addr)
{
	unsigned long *pte;

	pte = mmu_pt_walk_fetch(pgdp, virt_addr, PGDIR_SHIFT, 1);

	return pte;
}

void mmu_walk_and_print_pte(unsigned long virt_addr)
{
	unsigned long *pte;
	unsigned long *p = pgdp;
	unsigned int shift = PGDIR_SHIFT;

	print("================= dump page table =================\n");
	print("fault addr:0x%lx\n", virt_addr);
	print("pgdp : 0x%lx\n", p);
	pte = mmu_pt_walk_fetch_one(p, virt_addr, shift, 1);
	while (1) {
		print("0x%lx : 0x%lx\n", virt_to_phy(pte), *pte);
		if (pmd_leaf(*pte))
			goto _return;
		else if (pmd_none(*pte))
			goto _return;
		else if (shift == PAGE_SHIFT)
			goto _return;
		p = pte;
		shift -= 9;
		pte = mmu_pt_walk_fetch_one(p, virt_addr, shift, 0);
	}

_return:
	print("==================================================\n");
}

unsigned long *mmu_get_pte_level(unsigned long virt_addr, int lvl)
{
	unsigned long *pte;
	unsigned long *p = pgdp;
	unsigned int shift = PGDIR_SHIFT;

	pte = mmu_pt_walk_fetch_one(p, virt_addr, shift, 1);

	while (lvl++ < 3) {
		if (pmd_leaf(*pte))
			return NULL;
		else if (pmd_none(*pte))
			return NULL;
		else if (shift == PAGE_SHIFT)
			return NULL;
		p = pte;
		shift -= 9;
		pte = mmu_pt_walk_fetch_one(p, virt_addr, shift, 0);
	}

	return pte;
}

int mmu_page_mapping_lazy(unsigned long virt, unsigned int size,
			  pgprot_t pgprot)
{
	unsigned int page_nr = N_PAGE(size);
	unsigned long *pte;
	unsigned long pte_val;
	unsigned long virt_addr = virt;

	while (page_nr--) {
		pte =
		    riscv_pt_walk_alloc(pgdp,
					virt_addr, PGDIR_SHIFT, PAGE_SIZE, 1,
					alloc_zero_page, 0);
		if (!pte)
			return -1;

		pte_val = pgprot_val(pgprot);

		*pte = pte_val;

		virt_addr += PAGE_SIZE;
	}

	return 0;
}

int mmu_user_page_mapping(unsigned long *pgdp, unsigned long phy,
			  unsigned long virt, unsigned int size,
			  pgprot_t pgprot)
{
	if (__mmu_page_mapping_4k(pgdp, phy, virt, size, pgprot))
		return -1;

	local_flush_tlb_range(virt, size, PAGE_SIZE);

	return 0;
}

int mmu_gstage_page_mapping(unsigned long *_pgdp, unsigned long phy,
			    unsigned long virt, unsigned int size,
			    pgprot_t pgprot)
{
	return __mmu_page_mapping_4k(_pgdp, phy, virt, size, pgprot);
}

int mmu_gstage_page_mapping_2M(unsigned long *_pgdp, unsigned long phy,
			       unsigned long virt, unsigned int size,
			       pgprot_t pgprot)
{
	return __mmu_page_mapping_2M(_pgdp, phy, virt, size, pgprot);
}

int mmu_gstage_page_mapping_1G(unsigned long *_pgdp, unsigned long phy,
			       unsigned long virt, unsigned int size,
			       pgprot_t pgprot)
{
	return __mmu_page_mapping_1G(_pgdp, phy, virt, size, pgprot);
}

int mmu_page_mapping(unsigned long phy, unsigned long virt, unsigned int size,
		     pgprot_t pgprot)
{
	if (__mmu_page_mapping_4k((unsigned long *)pgdp, phy, virt, size, pgprot))
		return -1;

	local_flush_tlb_range(virt, size, PAGE_SIZE);

	return 0;
}

int mmu_page_mapping_2M(unsigned long phy, unsigned long virt, unsigned int size,
		        pgprot_t pgprot)
{
	if (__mmu_page_mapping_2M((unsigned long *)pgdp, phy, virt, size, pgprot))
		return -1;

	local_flush_tlb_range(virt, size, PAGE_2M_SIZE);

	return 0;
}

int mmu_page_mapping_1G(unsigned long phy, unsigned long virt, unsigned int size,
		        pgprot_t pgprot)
{
	if (__mmu_page_mapping_1G((unsigned long *)pgdp, phy, virt, size, pgprot))
		return -1;

	local_flush_tlb_range(virt, size, PAGE_1G_SIZE);

	return 0;
}

#if CONFIG_SVNAPOT
static int get_napot_order(unsigned int size)
{
	int order;

	for_each_napot_order(order) {
		if ((1UL << (order + PAGE_SHIFT)) == size)
			return order;
	}

	return -1;
}

static int __mmu_page_mapping_napot(unsigned long phy, unsigned long virt, unsigned int size,
			   pgprot_t pgprot, int page_size)
{
	int i, order;
	int pte_num = page_size / PAGE_SIZE;
	unsigned long pa = phy;
	unsigned long va = virt;
	unsigned int n = size;
	unsigned long pg;
	pgprot_t new_pg;
	unsigned long napot_bits, napot_mask;
	unsigned long *pte;

	order = get_napot_order(page_size);
	if (order == -1) {
		print("%s -- Unsupport napot size\n", __FUNCTION__);
		return -1;
	}

	pg = pgprot_val(pgprot);
	pg |= _PAGE_NAPOT;
	new_pg = __pgprot(pg);

	napot_bits = 1UL << (order - 1 + _PAGE_PFN_SHIFT);
	napot_mask = GENMASK(order - 1 + _PAGE_PFN_SHIFT, _PAGE_PFN_SHIFT);

	while (n) {
		for (i = 0; i < pte_num; i++) {
			__mmu_page_mapping_4k((unsigned long *)pgdp, pa, va, PAGE_SIZE, new_pg);
			pte = mmu_get_pte(va);
			*pte = (*pte & (~napot_mask)) | ((napot_bits) & napot_mask);
			pa += PAGE_SIZE;
			va += PAGE_SIZE;
		}
		n -= PAGE_SIZE * pte_num;
	}

	local_flush_tlb_range(virt, size, PAGE_SIZE);

	return 0;

}

int mmu_page_mapping_8k(unsigned long phy, unsigned long virt, unsigned int size,
			 pgprot_t pgprot)
{
	return __mmu_page_mapping_napot(phy, virt, size, pgprot, PAGE_8K_SIZE);
}

int mmu_page_mapping_16k(unsigned long phy, unsigned long virt, unsigned int size,
			 pgprot_t pgprot)
{
	return __mmu_page_mapping_napot(phy, virt, size, pgprot, PAGE_16K_SIZE);
}

int mmu_page_mapping_32k(unsigned long phy, unsigned long virt, unsigned int size,
			 pgprot_t pgprot)
{
	return __mmu_page_mapping_napot(phy, virt, size, pgprot, PAGE_32K_SIZE);
}

int mmu_page_mapping_64k(unsigned long phy, unsigned long virt, unsigned int size,
			 pgprot_t pgprot)
{
	return __mmu_page_mapping_napot(phy, virt, size, pgprot, PAGE_64K_SIZE);
}
#else
int mmu_page_mapping_8k(unsigned long phy, unsigned long virt, unsigned int size,
			 pgprot_t pgprot)
{
	return -1;
}
int mmu_page_mapping_16k(unsigned long phy, unsigned long virt, unsigned int size,
			 pgprot_t pgprot)
{
	return -1;
}

int mmu_page_mapping_32k(unsigned long phy, unsigned long virt, unsigned int size,
			 pgprot_t pgprot)
{
	return -1;
}

int mmu_page_mapping_64k(unsigned long phy, unsigned long virt, unsigned int size,
			 pgprot_t pgprot)
{
	return -1;
}
#endif

int mmu_page_mapping_no_sfence(unsigned long phy, unsigned long virt, unsigned int size,
			       pgprot_t pgprot)
{
	return __mmu_page_mapping_4k((unsigned long *)pgdp, phy, virt, size, pgprot);
}

int mmu_direct_page_mapping()
{
	pgprot_t pgprot;
	unsigned long phy_start = get_phy_start();
	struct memory_block *mm_blocks = get_mm_blocks();
	int n, i, ret = 0;

	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	va_pa_offset = PAGE_OFFSET - phy_start;

	n = mm_blocks->avail;
	for (i = 0; i < n; i++) {
		unsigned long start;
		unsigned int size;

		start = mm_blocks->memory_block_start[i];
		size = mm_blocks->memory_block_size[i];

		print("Direct mapping: start:0x%lx size:0x%lx\n", start, size);
#if CONFIG_SELECT_4K_DIRECT_MAPPING
		ret = mmu_page_mapping(start, phy_to_virt(start), size, pgprot);
#elif CONFIG_SELECT_2M_DIRECT_MAPPING
		ret = mmu_page_mapping_2M(start, phy_to_virt(start), size, pgprot);
#elif CONFIG_SELECT_1G_DIRECT_MAPPING
		ret = mmu_page_mapping_1G(start, phy_to_virt(start), size, pgprot);
#endif
		if (ret)
			return ret;
	}

	return 0;
}

static int mmu_hw_page_mapping(struct device_init_entry *hw)
{
	pgprot_t pgprot;
	unsigned long phy_start = 0x80000000;
	unsigned long phy_end = 0x80200000;
	unsigned int size = phy_end - phy_start;
	unsigned long virt_start = (unsigned long)0x80000000;

	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	print("HW mapping: start:0x%lx size:0x%lx\n", phy_start, size);
	return mmu_page_mapping_2M(phy_start, virt_start, size, pgprot);
}

extern char dtb_bin[];
static int mmu_dtb_page_mapping()
{
	pgprot_t pgprot;
	extern unsigned long __dtb_payload_start;
	extern unsigned long __dtb_payload_end;
	unsigned long phy_start = (unsigned long)dtb_bin;
	unsigned size =
	    (char *)&__dtb_payload_end - (char *)&__dtb_payload_start;
	unsigned long virt_start = (unsigned long)FIXMAP_DTB_START;
	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	print("DTB mapping: start:0x%lx size:0x%lx\n", phy_start, size);
	return mmu_page_mapping(phy_start, virt_start, size, pgprot);
}

static int mmu_code_page_mapping()
{
	pgprot_t pgprot;
	unsigned long phy_start = (unsigned long)&__start_gos;
	unsigned long phy_end = (unsigned long)&bss_end;
	unsigned int size = phy_end - phy_start;
	unsigned long virt_start = (unsigned long)&__start_gos;

	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC |
		     _PAGE_DIRTY);

	print("Code mapping: start:0x%lx size:0x%lx\n", phy_start, size);
	return mmu_page_mapping(phy_start, virt_start, size, pgprot);
}

void enable_mmu(int on)
{
	if (!on) {
		write_csr(satp, 0);
		mmu_is_on = 0;
	} else {
		__asm__ __volatile("sfence.vma":::"memory");

		write_csr(satp,
			  (((unsigned long)pgdp) >> PAGE_SHIFT) | SATP_MODE);
		mmu_is_on = 1;
	}
}

unsigned long alloc_zero_page(int gfp)
{
	void *ptr = mm_alloc(4096);
	unsigned long ret;

	if (!ptr)
		return 0;

	memset(ptr, 0, 4096);

	if (mmu_is_on)
		ret = virt_to_phy(ptr);
	else
		ret = (unsigned long)ptr;

	return ret;
}

int paging_init(struct device_init_entry *hw)
{
	if (!pgdp) {
		pgdp = mm_alloc(PAGE_SIZE);
		memset((char *)pgdp, 0, PAGE_SIZE);
	}

	mmu_code_page_mapping();
	mmu_direct_page_mapping();
	mmu_hw_page_mapping(hw);
	mmu_dtb_page_mapping();

	print("Paging init: mmu page mapping finish...\n");

	enable_mmu(1);

	return 0;
}

int do_page_fault(unsigned long addr)
{
	int ret = -1;
	unsigned long phy_start;
	unsigned long virt_start;
	pgprot_t pgprot;

	print("Page Fault -- fault addr:0x%lx\n", addr);

	if (addr >= VMAP_START && addr <= VMAP_END) {
		virt_start = addr;
		if (!mmu_is_on)
			phy_start = (unsigned long)mm_alloc(PAGE_SIZE);
		else
			phy_start =
			    virt_to_phy((unsigned long)mm_alloc(PAGE_SIZE));

		pgprot =
		    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC
			     | _PAGE_DIRTY);

		ret =
		    mmu_page_mapping(phy_start, virt_start, PAGE_SIZE, pgprot);
	}

	return ret;
}

unsigned long get_default_pgd(void)
{
	return (unsigned long)pgdp;
}
