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

#ifndef IOPGTABLE_H
#define IOPGTABLE_H

int riscv_gstage_map_pages(struct riscv_iommu_device *iommu_dev,
			   unsigned long iova, void *addr, unsigned int size,
			   int gfp);
int riscv_fstage_map_pages(struct riscv_iommu_device *iommu_dev,
			   unsigned long iova, void *addr, unsigned int size,
			   int gfp);
unsigned long riscv_iova_to_phys(struct riscv_iommu_device *dev,
				 unsigned long iova);
unsigned long *riscv_iommu_pt_walk_fetch(unsigned long *ptp, unsigned long iova,
					 unsigned int shift, int root);
unsigned long riscv_iova_to_phys_gstage(struct riscv_iommu_device *dev,
					unsigned long gpa);

#endif
