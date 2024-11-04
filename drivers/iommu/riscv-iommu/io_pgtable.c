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
#include "asm/pgtable.h"
#include "mm.h"
#include "print.h"
#include "riscv-iommu.h"

extern int mmu_is_on;

static int iommu_pg_shift = PGDIR_SHIFT_L3;

static unsigned long *riscv_iommu_pt_walk_alloc(unsigned long *ptp,
						unsigned long iova,
						unsigned int shift, int pgsize,
						int root,
						unsigned long (*pg_alloc)(int
									  gfp),
						int gfp)
{
	unsigned long *pte, ptr, pfn;

	if (root) {
		if (!mmu_is_on)
			pte = (unsigned long *)(ptp + ((iova >> shift) & 0x1FF));
		else
			pte =
			    (unsigned long *)phy_to_virt(ptp +
							 ((iova >> shift) &
							  0x1FF));
	}
	else {
		if (!mmu_is_on)
			pte = (unsigned long *)((unsigned long *)
						pfn_to_phys(pte_pfn(*ptp)) +
						((iova >> shift) & 0x1FF));
		else
			pte = (unsigned long *)phy_to_virt((unsigned long *)
							   pfn_to_phys(pte_pfn
								       (*ptp)) +
							   ((iova >> shift) &
							    0x1FF));
	}

	if ((1UL << shift) <= pgsize) {
		return pte;
	}

	if (!pte)
		return NULL;

	if (*pte == NULL) {
		ptr = pg_alloc ? pg_alloc(gfp) : 0;
		if (!ptr)
			return NULL;
		pfn = ptr >> PAGE_SHIFT;
		*pte = (pfn << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;
	}

	return riscv_iommu_pt_walk_alloc(pte, iova, shift - 9, pgsize, 0,
					 pg_alloc, gfp);
}

static unsigned long *riscv_iommu_pt_walk_fetch(unsigned long *ptp,
						unsigned long iova, unsigned int shift,
						int root)
{
	unsigned long *pte;

	if (root) {
		if (!mmu_is_on)
			pte = (unsigned long *)(ptp + ((iova >> shift) & 0x1FF));
		else
			pte =
			    (unsigned long *)phy_to_virt(ptp +
							 ((iova >> shift) &
							  0x1FF));
	}
	else {
		if (!mmu_is_on)
			pte =
			    (unsigned long *)(pfn_to_phys(pte_pfn(*ptp)) +
					      ((iova >> shift) & 0x1FF));
		else
			pte = (unsigned long *)(phy_to_virt((unsigned long *)
							    pfn_to_phys(pte_pfn
									(*ptp))
							    +
							    ((iova >> shift) &
							     0x1FF)));
	}

	if (pmd_leaf(*pte))
		return pte;
	else if (pmd_none(*pte))
		return NULL;
	else if (shift == PAGE_SHIFT)
		return NULL;

	return riscv_iommu_pt_walk_fetch(pte, iova, shift - 9, 0);
}

static unsigned long __riscv_iommu_io_walk_pt(struct riscv_iommu_device *iommu_dev,
					   unsigned long iova, int gstage)
{
	void *pgdp;
	unsigned long *pte;

	if (!iommu_dev) {
		print("%s -- iommu_dev is NULL\n");
		return -1UL;
	}

	if (gstage == RISCV_IOMMU_GSTAGE) {
		pgdp = iommu_dev->pgdp_gstage;
	} else if (gstage == RISCV_IOMMU_FIRST_STAGE) {
		pgdp = iommu_dev->pgdp;
	}

	pte = riscv_iommu_pt_walk_fetch((unsigned long *)virt_to_phy(pgdp), iova, iommu_pg_shift, 1);
	if (!pte)
		return -1UL;
	if (!pmd_present(*pte))
		return -1UL;

	return (unsigned long)((pfn_to_phys(pte_pfn(*pte)) | (iova & (PAGE_SIZE - 1))));
}

int riscv_iommu_io_page_mapping(void *pgdp, unsigned long iova, void *addr, unsigned int size, int gfp)
{
	pgprot_t pgprot;
	int page_nr = N_PAGE(size);
	unsigned long *pte;
	unsigned long pfn, pte_val;

	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	while (page_nr--) {
		pfn = (unsigned long)addr >> PAGE_SHIFT;
		pte =
		    riscv_iommu_pt_walk_alloc((unsigned long *)virt_to_phy(pgdp),
					      iova, iommu_pg_shift, PAGE_SIZE, 1,
					      alloc_zero_page, gfp);
		if (!pte)
			return 0;

		pte_val = pfn_pte(pfn, pgprot);

		*pte = pte_val;

		iova += PAGE_SIZE;
		addr += PAGE_SIZE;
	}

	return 0;
}

static int __riscv_iommu_io_map_pages(struct riscv_iommu_device *iommu_dev,
				   unsigned long iova, void *addr, unsigned int size,
				   int gfp, int gstage)
{
	void *pgdp;

	if (!iommu_dev) {
		print("%s -- iommu_dev is NULL\n");
		return -1;
	}

	if (gstage == RISCV_IOMMU_GSTAGE) {
		pgdp = iommu_dev->pgdp_gstage;
	} else if (gstage == RISCV_IOMMU_FIRST_STAGE) {
		pgdp = iommu_dev->pgdp;
	}

	return riscv_iommu_io_page_mapping(pgdp, iova, addr, size, gfp);

}

unsigned long riscv_iommu_gstage_io_walk_pt(struct riscv_iommu_device *iommu_dev,
					 unsigned long iova)
{
	return __riscv_iommu_io_walk_pt(iommu_dev, iova, RISCV_IOMMU_GSTAGE);
}

unsigned long riscv_iommu_fstage_io_walk_pt(struct riscv_iommu_device *iommu_dev,
					 unsigned long iova)
{
	return __riscv_iommu_io_walk_pt(iommu_dev, iova, RISCV_IOMMU_FIRST_STAGE);
}

int riscv_iommu_gstage_io_map_pages(struct riscv_iommu_device *iommu_dev,
				 unsigned long iova, void *addr, unsigned int size,
				 int gfp)
{
	return __riscv_iommu_io_map_pages(iommu_dev, iova, addr, size, gfp,
				       RISCV_IOMMU_GSTAGE);
}

int riscv_iommu_fstage_io_map_pages(struct riscv_iommu_device *iommu_dev,
				 unsigned long iova, void *addr, unsigned int size,
				 int gfp)
{
	return __riscv_iommu_io_map_pages(iommu_dev, iova, addr, size, gfp,
				       RISCV_IOMMU_FIRST_STAGE);
}

void riscv_iommu_set_pg_shift(int mode)
{
	if (mode == 8)
		iommu_pg_shift = PGDIR_SHIFT_L3;
	else if (mode == 9)
		iommu_pg_shift = PGDIR_SHIFT_L4;
	else if (mode == 0xa)
		iommu_pg_shift = PGDIR_SHIFT_L5;
}
