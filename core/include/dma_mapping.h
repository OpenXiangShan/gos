/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DMA_MAPPING_H
#define DMA_MAPPING_H

#include <device.h>
#include <list.h>

#define GPA_MEM_MAP_NR 16384
#define IOVA_MEM_MAP_NR 16384	//4*1024*1024*1024/PAGE_SIZE/(sizeof(unsigned long)*8) -- 4G
#define TOTAL_IOVA_PAGE_NUM IOVA_MEM_MAP_NR * sizeof(unsigned long) * 8
#define TOTAL_GPA_PAGE_NUM GPA_MEM_MAP_NR * sizeof(unsigned long) * 8

struct dma_map_ops {
	void *(*alloc)(struct device * dev, unsigned long iova,
		       unsigned int size, unsigned int gfp);
	int (*map_pages)(struct device * dev, unsigned long iova, void *addr,
			 unsigned int size, int gfp);
	void *(*probe_device)(struct device * dev);
	int (*finalize)(struct device * dev, int pscid);
	unsigned long (*iova_to_phys)(struct device * dev, unsigned long iova);
	unsigned long (*iova_to_phys_with_devid)(int dev_id,
						 unsigned long iova);
	void (*enable_gstage)(struct device * dev, int gstage);
	unsigned long (*iova_to_phys_in_two_stage)(struct device * dev,
						   unsigned long iova);
	unsigned long (*iova_to_phys_in_two_stage_with_devid)(int dev_id,
							      unsigned long
							      iova);
};

struct iommu_group {
	struct list_head list;
	int dev_id;
	unsigned long iova_mem_maps[IOVA_MEM_MAP_NR];
	unsigned long gpa_mem_maps[GPA_MEM_MAP_NR];
	struct list_head iommus;
	void *pgdp;
	void *pgdp_gstage;
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

void *gpa_alloc(struct device *dev, unsigned int size);
void gpa_free(struct device *dev, void *addr, unsigned int size);
void *dma_two_stage_alloc(struct device *dev, unsigned long *iova,
			  unsigned long *gpa, unsigned int size);
void dma_gstage_enable(struct device *dev, int gstage);
void *dma_iova_to_phys_in_two_stage(struct device *dev, unsigned long iova);
void *dma_iova_to_phys_in_two_stage_with_devid(int dev_id, unsigned long iova);

#endif
