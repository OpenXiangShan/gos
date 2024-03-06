#include "riscv_iommu_data.h"

struct riscv_iommu_data riscv_iommu_data = {
	4096,
	4096,
	4096,
	0,
	0,
	0,
	4, //ddt_mode -- 3lenel
	8, //pg_mode -- SV39	
};
