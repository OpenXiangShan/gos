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
#include "list.h"
#include "string.h"
#include "mm.h"

static LIST_HEAD(iommus);
static LIST_HEAD(groups);

int iommu_map_msi_addr(struct device *dev, unsigned long msi_pa,
		       unsigned long msi_va, int len)
{
	struct iommu *iommu = dev->iommu;

	if (!iommu || !iommu->ops->map_msi_addr)
		return -1;

	return iommu->ops->map_msi_addr(dev, msi_pa, msi_va, len);
}

int iommu_map_pages(struct device *dev, unsigned long iova, void *addr,
		    unsigned int size, int gstage)
{
	struct iommu *iommu = dev->iommu;

	if (!iommu || !iommu->ops->map_pages)
		return -1;

	return iommu->ops->map_pages(dev, iova, addr, size, gstage);
}

struct iommu_group *iommu_alloc_group(void)
{
	struct iommu_group *new;

	new = (struct iommu_group *)mm_alloc(sizeof(struct iommu_group));
	if (!new)
		return NULL;

	INIT_LIST_HEAD(&new->devices);
	INIT_LIST_HEAD(&new->iova_cookie);
	list_add_tail(&new->list, &groups);

	return new;
}

struct iommu_group *iommu_get_group(struct device *dev)
{
	return dev->iommu_group;
}

void iommu_attach_device(struct device *dev, struct iommu *iommu, int gstage)
{
	if (!dev->iommu_group) {
		struct iommu_group *group;
		group = iommu_alloc_group();
		dev->iommu_group = group;
		list_add_tail(&dev->iommu_group_list, &group->devices);
	}
	dev->iommu = iommu;

	if (iommu->ops->probe_device)
		iommu->ops->probe_device(dev);

	if (iommu->ops->enable_gstage)
		iommu->ops->enable_gstage(dev, gstage);

	if (iommu->ops->finalize)
		iommu->ops->finalize(dev, 0);
}

void iommu_device_attach_group(struct device *dev, struct iommu_group *group)
{
	dev->iommu_group = group;
	list_add_tail(&dev->iommu_group_list, &group->devices);
}

void iommu_device_dettach_group(struct device *dev)
{
	struct iommu_group *group = dev->iommu_group;

	list_del(&dev->iommu_group_list);
	dev->iommu_group = NULL;

	if (list_empty(&group->devices))
		list_del(&group->list);
}

void iommu_register(struct iommu *iommu)
{
	list_add_tail(&iommu->list, &iommus);
}

struct iommu *find_iommu(char *name)
{
	struct iommu *iommu;

	list_for_each_entry(iommu, &iommus, list) {
		if (!strncmp(iommu->name, name, 64))
			return iommu;
	}

	return NULL;
}

struct iommu *find_default_iommu(void)
{
	return find_iommu("riscv,iommu");
}
