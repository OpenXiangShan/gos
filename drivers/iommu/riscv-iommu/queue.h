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

#ifndef IOMMU_QUEUE_H
#define IOMMU_QUEUE_H

#include "iommu.h"

int riscv_iommu_cmdq_init(struct riscv_iommu *iommu);
int riscv_iommu_fltq_init(struct riscv_iommu *iommu);
int riscv_iommu_priq_init(struct riscv_iommu *iommu);

#endif