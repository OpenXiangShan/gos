#include <asm/type.h>
#include <asm/pgtable.h>
#include <mm.h>
#include <print.h>
#include "iommu.h"

static unsigned long *riscv_iommu_pt_walk_alloc(unsigned long *ptp,
						unsigned long iova,
						unsigned int shift, int pgsize,
						int root,
						unsigned long (*pg_alloc)(int
									  gfp),
						int gfp)
{
	unsigned long *pte, ptr, pfn;

	if (root)
		pte = ptp + (iova >> shift);
	else {
		pte = (unsigned long *)pfn_to_phys(pte_pfn(*ptp));
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

	return riscv_iommu_pt_walk_alloc(pte, iova, shift - 9, pgsize, 0,
					 pg_alloc, gfp);
}

unsigned long *riscv_iommu_pt_walk_fetch(unsigned long *ptp,
					 unsigned long iova, unsigned int shift,
					 int root)
{
	unsigned long *pte;

	if (root)
		pte = ptp + (iova >> shift);
	else
		pte = (unsigned long *)pfn_to_phys(pte_pfn(*ptp));

	if (pmd_leaf(*pte))
		return pte;
	else if (pmd_none(*pte))
		return NULL;
	else if (shift == PAGE_SHIFT)
		return NULL;

	return riscv_iommu_pt_walk_fetch(pte, iova, shift - 9, 0);
}

static unsigned long __riscv_iova_to_phys(struct riscv_iommu_device *dev,
					  unsigned long iova, int stage)
{
	unsigned long *pte;
	unsigned long *ptp;
	int shift;

	if (stage == RISCV_IOMMU_FIRST_STAGE) {
		shift = PGDIR_SHIFT;
		ptp = dev->pgdp;
	} else if (stage == RISCV_IOMMU_GSTAGE) {
		shift = PGDIR_SHIFT_GSTAGE;
		ptp = dev->pgdp_gstage;
	} else {
		print("%s -- unsupported stage\n");
		return 0;
	}

	pte = riscv_iommu_pt_walk_fetch(ptp, iova, shift, 1);
	if (!pte) {
		print("%s -- pt walk fetch pte is NULL\n", __FUNCTION__);
		return 0;
	}
	if (!pmd_present(*pte)) {
		print("%s -- pte entry is not persent\n", __FUNCTION__);
		return 0;
	}
	//print("%s -- pte:0x%x, pfn:0x%x PAGE_MASK:0x%x, iova&PAGE_MASK:0x%x\n", __FUNCTION__, pte, pte_pfn(*pte), PAGE_MASK, iova & PAGE_MASK);
	return (pfn_to_phys(pte_pfn(*pte)) | (iova & (PAGE_SIZE - 1)));
}

unsigned long riscv_iova_to_phys(struct riscv_iommu_device *dev,
				 unsigned long iova)
{
	return __riscv_iova_to_phys(dev, iova, RISCV_IOMMU_FIRST_STAGE);
}

unsigned long riscv_iova_to_phys_gstage(struct riscv_iommu_device *dev,
					unsigned long gpa)
{
	return __riscv_iova_to_phys(dev, gpa, RISCV_IOMMU_GSTAGE);
}

static int riscv_map_pages(struct riscv_iommu_device *iommu_dev,
			   unsigned long iova, void *addr, unsigned int size,
			   int gfp, int gstage)
{
	pgprot_t pgprot;
	int page_nr = N_PAGE(size);
	unsigned long *pte;
	unsigned long pfn, pte_val;
	unsigned int pgd_shift = 0;
	void *pgdp;

	if (!iommu_dev) {
		print("%s -- iommu_dev is NULL\n");
		return -1;
	}

	if (gstage == RISCV_IOMMU_GSTAGE) {
		pgd_shift = PGDIR_SHIFT_GSTAGE;
		pgdp = iommu_dev->pgdp_gstage;
	} else if (gstage == RISCV_IOMMU_FIRST_STAGE) {
		pgd_shift = PGDIR_SHIFT;
		pgdp = iommu_dev->pgdp;
	}

	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	while (page_nr--) {
		pfn = (unsigned long)addr >> PAGE_SHIFT;
		pte =
		    riscv_iommu_pt_walk_alloc((unsigned long *)pgdp,
					      iova, pgd_shift, PAGE_SIZE, 1,
					      alloc_zero_page, gfp);
		if (!pte)
			return NULL;

		pte_val = pfn_pte(pfn, pgprot);

		*pte = pte_val;

		iova += PAGE_SIZE;
		addr += PAGE_SIZE;
	}

	return 0;
}

int riscv_gstage_map_pages(struct riscv_iommu_device *iommu_dev,
			   unsigned long iova, void *addr, unsigned int size,
			   int gfp)
{
	return riscv_map_pages(iommu_dev, iova, addr, size, gfp,
			       RISCV_IOMMU_GSTAGE);
}

int riscv_fstage_map_pages(struct riscv_iommu_device *iommu_dev,
			   unsigned long iova, void *addr, unsigned int size,
			   int gfp)
{
	return riscv_map_pages(iommu_dev, iova, addr, size, gfp,
			       RISCV_IOMMU_FIRST_STAGE);
}
