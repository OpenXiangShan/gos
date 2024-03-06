#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "dma_mapping.h"

#define TEST_NUM 5

int iommu_test(struct device *dev, void *data)
{
	void *addr;
	unsigned long iova;

	print("\n%s dev_name: %s\n", __FUNCTION__, dev->compatible);
	for (int i = 0; i < TEST_NUM; i++) {
		addr = dma_alloc(dev, &iova, 4096);
		if (!addr)
			return -1;
		print("%s dev_id:%d -- dma_alloc , addr:0x%x, iova:0x%x\n",
		      __FUNCTION__, dev->iommu.dev_id, (unsigned long)addr,
		      iova);
		print("%s dev_id:%d -- iova_to_phys, iova:0x%x, addr:0x%x\n",
		      __FUNCTION__, dev->iommu.dev_id, iova,
		      dma_iova_to_phys(dev, iova));
		print
		    ("%s dev_id:%d -- iova_to_phys_with_devid, iova:0x%x, addr:0x%x\n",
		     __FUNCTION__, dev->iommu.dev_id, iova,
		     dma_iova_to_phys_with_devid(dev->iommu.dev_id, iova));
		print("\n");
	}

	return 0;
}

DRIVER_REGISTER(iommu_test, iommu_test, "riscv,iommu_test");
DRIVER_REGISTER(iommu_test2, iommu_test, "riscv,iommu_test2");
DRIVER_REGISTER(iommu_test3, iommu_test, "riscv,iommu_test3");
