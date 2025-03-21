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

#include "asm/type.h"
#include "device.h"
#include "iommu.h"
#include "iova.h"
#include "mm.h"
#include "asm/pgtable.h"
#include "align.h"

int dma_mapping(struct device *dev, unsigned long addr,
		unsigned long *ret_iova, int len, int gfp)
{
	struct iommu_group *group;
	struct iommu *iommu = dev->iommu;
	unsigned long iova;
	unsigned long align_start_addr = ALIGN_SIZE_UP(addr, PAGE_SIZE);
	unsigned long align_end_addr = ALIGN_SIZE(addr, PAGE_SIZE);
	int align_nr = (align_end_addr - align_start_addr) / PAGE_SIZE;

	if (align_nr == 0)
		align_nr = 1;

	if (!iommu || !iommu->ops->map_pages) {
		*ret_iova = addr;
		return 0;
	}

	group = iommu_get_group(dev);
	if (!group)
		return -1;

	iova = iova_alloc(&group->iova_cookie, align_nr * PAGE_SIZE);
	if (iova == -1UL)
		return -1;

	if (-1 == iommu->ops->map_pages(dev, iova,
					(void *)align_start_addr,
					align_nr * PAGE_SIZE, 0)) {
		iova_free(&group->iova_cookie, iova);
		return -1;
	}

	*ret_iova = iova;

	return 0;
}

void *dma_alloc(struct device *dev, unsigned long *ret_iova, int len, int gfp)
{
	struct iommu_group *group;
	struct iommu *iommu = dev->iommu;
	unsigned long iova;
	void *addr, *va;

	if (!iommu || !iommu->ops->alloc) {
		va = mm_alloc(len);
		addr = (void *)virt_to_phy(va);
		*ret_iova = (unsigned long)addr;
		return va;
	}

	group = iommu_get_group(dev);
	if (!group)
		return NULL;

	iova = iova_alloc(&group->iova_cookie, len);
	if (iova == -1UL)
		return NULL;

	addr = iommu->ops->alloc(dev, iova, len, NULL);
	if (NULL == addr) {
		iova_free(&group->iova_cookie, iova);
		return NULL;
	}

	*ret_iova = iova;

	return (void *)addr;
}
