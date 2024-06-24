#include <asm/pgtable.h>
#include <asm/type.h>
#include "device.h"
#include "mm.h"
#include "string.h"
#include "print.h"
#include "asm/csr.h"
#include "vmap.h"
#include "asm/barrier.h"
#include "asm/tlbflush.h"

#define PAGE_OFFSET 0xffffffd800000000

static void *pgdp __attribute__((section(".data"))) = NULL;

int pgtable_l4_enabled __attribute__((section(".data"))) = 0;
int pgtable_l5_enabled __attribute__((section(".data"))) = 0;

int mmu_is_on __attribute__((section(".data"))) = 0;
unsigned long va_pa_offset __attribute__((section(".data"))) = 0;

extern unsigned long bss_end;
extern unsigned long __start_guest;

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

void *walk_pt_va_to_pa(unsigned long va)
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

	return (void *)((pfn_to_phys(pte_pfn(*pte)) | (va & (PAGE_SIZE - 1))));
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

static int __mmu_page_mapping(unsigned long *_pgdp, unsigned long phy,
			      unsigned long virt, unsigned int size,
			      pgprot_t pgprot)
{
	unsigned int page_nr = N_PAGE(size);
	unsigned long *pte;
	unsigned long pfn, pte_val;
	unsigned long phy_addr = phy;
	unsigned long virt_addr = virt;

	while (page_nr--) {
		pfn = (unsigned long)phy_addr >> PAGE_SHIFT;
		pte =
		    riscv_pt_walk_alloc(_pgdp,
					virt_addr, PGDIR_SHIFT, PAGE_SIZE, 1,
					alloc_zero_page, 0);
		if (!pte)
			return -1;

		pte_val = pfn_pte(pfn, pgprot);

		*pte = pte_val;

		virt_addr += PAGE_SIZE;
		phy_addr += PAGE_SIZE;
	}

	return 0;
}

int mmu_user_page_mapping(unsigned long phy, unsigned long virt,
			  unsigned int size, pgprot_t pgprot)
{
	return __mmu_page_mapping(pgdp, phy, virt, size, pgprot);
}

int mmu_gstage_page_mapping(unsigned long *_pgdp, unsigned long phy,
			    unsigned long virt, unsigned int size,
			    pgprot_t pgprot)
{
	return __mmu_page_mapping(_pgdp, phy, virt, size, pgprot);
}

int mmu_page_mapping(unsigned long phy, unsigned long virt, unsigned int size,
		     pgprot_t pgprot)
{
	if (__mmu_page_mapping((unsigned long *)pgdp, phy, virt, size,
				  pgprot))
		return -1;

	local_flush_tlb_range(virt, size, PAGE_SIZE);

	return 0;
}

unsigned long *mmu_get_pte(unsigned long virt_addr)
{
	unsigned long *pte;

	pte = mmu_pt_walk_fetch(pgdp, virt_addr, PGDIR_SHIFT, 1);

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

static int mmu_direct_page_mapping()
{
	pgprot_t pgprot;
	unsigned long phy_start = get_phy_start();
	unsigned long phy_end = get_phy_end();
	unsigned int size = phy_end - phy_start;
	unsigned long virt_addr = PAGE_OFFSET;

	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	va_pa_offset = PAGE_OFFSET - phy_start;

	myGuest_print("%s -- pa:0x%lx va:0x%lx len:0x%x\n", __FUNCTION__,
		      phy_start, virt_addr, size);
	return mmu_page_mapping(phy_start, virt_addr, size, pgprot);
}

static int mmu_hw_page_mapping(struct device_init_entry *hw)
{
	pgprot_t pgprot;
	unsigned long phy_start = (unsigned long)hw;
	unsigned long virt_start = FIXMAP_HW_START;
	unsigned int size = FIXMAP_HW_LEN;

	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	myGuest_print("%s -- pa:0x%lx va:0x%lx len:0x%x\n", __FUNCTION__,
		      phy_start, virt_start, size);
	return mmu_page_mapping(phy_start, virt_start, size, pgprot);
}

int mmu_page_mapping_no_sfence(unsigned long phy, unsigned long virt, unsigned int size,
			       pgprot_t pgprot)
{
	return __mmu_page_mapping((unsigned long *)pgdp, phy, virt, size, pgprot);
}

static int mmu_code_page_mapping()
{
	pgprot_t pgprot;
	unsigned long phy_start = (unsigned long)&__start_guest;
	unsigned long phy_end = (unsigned long)&bss_end;
	unsigned int size = phy_end - phy_start;
	unsigned long virt_start = (unsigned long)&__start_guest;

	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC |
		     _PAGE_DIRTY);

	myGuest_print("%s -- pa:0x%lx va:0x%lx len:0x%x\n", __FUNCTION__,
		      phy_start, virt_start, size);
	return mmu_page_mapping(phy_start, virt_start, size, pgprot);
}

static unsigned long mmu_sram_page_mapping(struct device_init_entry *hw)
{
	pgprot_t pgprot;
	struct device_init_entry *device_entry = hw;
	unsigned long pa, va;
	unsigned int size;

	while (strncmp(device_entry->compatible, "THE END", sizeof("THE END"))) {
		if (!strncmp(device_entry->compatible, "sram", 128)) {
			pa = device_entry->start;
			size = device_entry->len;
			goto found;
		}
		device_entry++;
	}

	return -1;
found:
	va = pa;
	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	myGuest_print("%s -- pa:0x%lx va:0x%lx len:0x%x\n", __FUNCTION__, pa,
		      va, size);
	return mmu_page_mapping(pa, va, size, pgprot);
}

void enable_mmu(int on)
{
	if (!on) {
		write_csr(satp, 0);
		mmu_is_on = 0;
	} else {
		__asm__ __volatile("sfence.vma":::"memory");
		mb();

		write_csr(satp,
			  (((unsigned long)pgdp) >> PAGE_SHIFT) | SATP_MODE);

		mb();
		mmu_is_on = 1;
	}
}

int paging_init(struct device_init_entry *hw)
{
	if (!pgdp)
		pgdp = mm_alloc(PAGE_SIZE);

	if (!pgdp) {
		myGuest_print("alloc pgd failed...\n");
		return -1;
	}
	memset((char *)pgdp, 0, PAGE_SIZE);

	mmu_code_page_mapping();
	mmu_direct_page_mapping();
	mmu_hw_page_mapping(hw);
	mmu_sram_page_mapping(hw);

	print("mmu page mapping finish...\n");

	mb();

	enable_mmu(1);

	return 0;
}
