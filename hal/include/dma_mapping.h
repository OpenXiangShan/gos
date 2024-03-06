#ifndef DMA_MAPPING_H
#define DMA_MAPPING_H

#include <device.h>
#include <list.h>

#define IOVA_MEM_MAP_NR 16384	//4*1024*1024*1024/PAGE_SIZE/(sizeof(unsigned long)*8) -- 4G
#define TOTAL_IOVA_PAGE_NUM IOVA_MEM_MAP_NR * sizeof(unsigned long)

struct dma_map_ops {
	void *(*alloc)(struct device * dev, unsigned long iova,
		       unsigned int size, unsigned int gfp);
	void *(*probe_device)(struct device * dev);
	int (*finalize)(struct device * dev, int pscid);
	unsigned long (*iova_to_phys)(struct device * dev, unsigned long iova);
	unsigned long (*iova_to_phys_with_devid)(int dev_id,
						 unsigned long iova);
};

struct iommu_group {
	struct list_head list;
	int dev_id;
	unsigned long iova_mem_maps[IOVA_MEM_MAP_NR];
	struct list_head iommus;
	void *pgdp;
};

void dma_mapping_probe_device(struct device *dev);
struct dma_map_ops *get_dma_mapping_ops(void);
void set_dma_mapping_ops(struct dma_map_ops *ops);
void *dma_alloc(struct device *dev, unsigned long *iova, unsigned int size);
void *iova_alloc(struct device *dev, unsigned int size);
void iova_free(struct device *dev, void *addr, unsigned int size);
struct iommu_group *dma_mapping_find_iommu_group(struct device *dev);
void *dma_iova_to_phys(struct device *dev, unsigned long iova);
void *dma_iova_to_phys_with_devid(int dev_id, unsigned long iova);

#endif
