#ifndef __IOMMU_CORE_H__
#define __IOMMU_CORE_H__

#include "list.h"
#include "device.h"

struct iommu_group {
	struct list_head list;
	int group_id;
	struct list_head devices;
	struct list_head iova_cookie;
	void *pgdp;
	void *pgdp_gstage;
};

struct iommu_ops {
	void *(*alloc)(struct device * dev, unsigned long iova,
		       unsigned int size, unsigned int gfp);
	int (*map_pages)(struct device * dev, unsigned long iova, void *addr,
			 unsigned int size, int gfp);
	unsigned long (*walk_pt)(struct device *dev, unsigned long iova, int gstage);
	int (*probe_device)(struct device * dev);
	int (*finalize)(struct device * dev, int pscid);
	void (*enable_gstage)(struct device * dev, int gstage);
};

struct iommu {
	struct list_head list;
	char name[64];
	struct iommu_ops *ops;
	void *priv;
};

int iommu_map_pages(struct device *dev, unsigned long iova, void *addr, unsigned int size, int gstage);
struct iommu *find_iommu(char *name);
struct iommu *find_default_iommu(void);
void iommu_register(struct iommu *iommu);
void iommu_attach_device(struct device *dev, struct iommu *iommu);
void iommu_detach_device(struct device *dev);
void iommu_device_attach_group(struct device *dev, struct iommu_group *group);
void iommu_device_detach_group(struct device *dev);
struct iommu_group *iommu_alloc_group(void);
struct iommu_group *iommu_get_group(struct device *dev);

#endif
