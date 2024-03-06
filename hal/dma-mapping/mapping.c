#include <mm.h>
#include <asm/type.h>
#include <device.h>
#include <list.h>
#include <string.h>
#include <print.h>
#include "dma_mapping.h"

static struct dma_map_ops *dma_mapping_ops = NULL;

static LIST_HEAD(groups);

struct iommu_group *dma_mapping_find_iommu_group(struct device *dev)
{
	struct iommu_group *gp;
	
	list_for_each_entry(gp, &groups, list) {
		if (gp->dev_id == dev->iommu.dev_id) {
			return gp;	
		}	
	}

	return NULL;
}

void dma_mapping_probe_device(struct device *dev)
{
	struct iommu_group *gp;

	if (!dma_mapping_ops)
		return;
	
	gp = dma_mapping_find_iommu_group(dev);
	if (!gp) {
		gp = mm_alloc(sizeof(struct iommu_group));
		print("%s -- alloc group:0x%x iommu_group size:%d\n", __FUNCTION__, gp, sizeof(struct iommu_group));
		memset((char *)gp, 0, sizeof(struct iommu_group));
		gp->dev_id = dev->iommu.dev_id;
		list_add(&gp->list, &groups);
		INIT_LIST_HEAD(&gp->iommus);
	}
	dev->iommu.group = gp;
	list_add(&dev->iommu.list, &gp->iommus);	

	if (!dma_mapping_ops->probe_device)
		return;	
	
	dev->iommu.priv_data = dma_mapping_ops->probe_device(dev);

	if (!dma_mapping_ops->finalize)
		return;
	
	dma_mapping_ops->finalize(dev, 0);	
}

struct dma_map_ops *get_dma_mapping_ops(void)
{
	return dma_mapping_ops;
}

void set_dma_mapping_ops(struct dma_map_ops *ops)
{
	dma_mapping_ops = ops;
}

void *dma_alloc(struct device *dev, unsigned long *iova, unsigned int size)
{
	void *addr = NULL, *p_iova;	

	if (!dma_mapping_ops || !dma_mapping_ops->alloc)
		addr = mm_alloc(size);		
	else {
		p_iova = iova_alloc(dev, size);
		if (p_iova == (void *)U64_MAX) {
			print("alloc iova failed -- %s\n", __FUNCTION__);
			return NULL;
		}
		addr = dma_mapping_ops->alloc(dev, (unsigned long)p_iova, size, NULL);		
	}
	*iova = (unsigned long)p_iova;

	return addr;
}

void *dma_iova_to_phys(struct device *dev, unsigned long iova)
{
	if (!dma_mapping_ops || !dma_mapping_ops->iova_to_phys)	
		return NULL;
	
	return (void *)dma_mapping_ops->iova_to_phys(dev, iova);
}

void *dma_iova_to_phys_with_devid(int dev_id, unsigned long iova)
{
	if (!dma_mapping_ops || !dma_mapping_ops->iova_to_phys_with_devid)
		return NULL;

	return (void *)dma_mapping_ops->iova_to_phys_with_devid(dev_id, iova);
}
