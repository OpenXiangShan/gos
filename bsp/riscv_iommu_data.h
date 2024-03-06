#ifndef __RISCV_IOMMU_DATA_H
#define __RISCV_IOMMU_DATA_H

struct riscv_iommu_data {
	int cmdq_len;
	int fltq_len;
	int priq_len;
	int cmdq_irq;
	int fltq_irq;
	int priq_irq;
	int ddt_mode;
	int pg_mode;
};

#endif
