#ifndef IOPGTABLE_H
#define IOPGTABLE_H

int riscv_gstage_map_pages(struct riscv_iommu_device *iommu_dev,
			   unsigned long iova, void *addr, unsigned int size,
			   int gfp);
int riscv_fstage_map_pages(struct riscv_iommu_device *iommu_dev,
			   unsigned long iova, void *addr, unsigned int size,
			   int gfp);
unsigned long riscv_iova_to_phys(struct riscv_iommu_device *dev,
				 unsigned long iova);
unsigned long *riscv_iommu_pt_walk_fetch(unsigned long *ptp, unsigned long iova,
					 unsigned int shift, int root);
unsigned long riscv_iova_to_phys_gstage(struct riscv_iommu_device *dev,
					unsigned long gpa);

#endif
