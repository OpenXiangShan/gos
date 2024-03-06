#ifndef IOMMU_QUEUE_H
#define IOMMU_QUEUE_H

#include "iommu.h"

int riscv_iommu_cmdq_init(struct riscv_iommu *iommu);
int riscv_iommu_fltq_init(struct riscv_iommu *iommu);
int riscv_iommu_priq_init(struct riscv_iommu *iommu);

#endif
