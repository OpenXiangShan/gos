#ifndef __DMA_MAPPING_H__
#define __DMA_MAPPING_H__

#include "iommu.h"

void *dma_alloc(struct device *dev, unsigned long *ret_iova, int len, int gfp);
int dma_mapping(struct device *dev, unsigned long addr,
		unsigned long *ret_iova, int len, int gfp);

#endif
