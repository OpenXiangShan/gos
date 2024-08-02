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

#include <asm/type.h>
#include <device.h>
#include <print.h>
#include <mm.h>
#include <asm/mmio.h>
#include "queue.h"
#include <irq.h>

int riscv_iommu_cmdq_init(struct riscv_iommu *iommu)
{
	struct riscv_iommu_queue *q;
	unsigned long cqb;
	unsigned int cqh;

	print("%s -- cmdq_len:%d, cmdq_irq:%d\n", __FUNCTION__, iommu->cmdq_len,
	      iommu->cmdq_irq);

	q = &iommu->cmdq;
	q->base = mm_alloc(iommu->cmdq_len);
	if (!q->base) {
		print("%s - %s -- Out of memory", __FUNCTION__, __LINE__);
		return -1;
	}

	cqb = ((((unsigned long)q->base) >> 2) & CQB_PPN_MASK) | CQ_LOG2SZ_1;
	writeq(iommu->base + IOMMU_CQB_OFFSET, cqb);

	cqh = readl(iommu->base + IOMMU_CQH_OFFSET);
	writel(iommu->base + IOMMU_CQT_OFFSET, cqh);

	writel(iommu->base + IOMMU_CQCSR_OFFSET, CQCSR_CQEN | CQCSR_CIE);

	while (!(readl(iommu->base + IOMMU_CQCSR_OFFSET) & CQCSR_CQON)) ;

	iommu->cmdq.irq = iommu->cmdq_irq;

	return 0;
}

int riscv_iommu_fltq_init(struct riscv_iommu *iommu)
{
	struct riscv_iommu_queue *q;
	unsigned long fqb;
	unsigned int fqt;

	print("%s -- fltq_len:%d, fltq_irq:%d\n", __FUNCTION__, iommu->fltq_len,
	      iommu->fltq_irq);

	q = &iommu->fltq;
	q->base = mm_alloc(iommu->cmdq_len);
	if (!q->base) {
		print("%s - %s -- Out of memory", __FUNCTION__, __LINE__);
		return -1;
	}

	fqb = ((((unsigned long)q->base) >> 2) & FQB_PPN_MASK) | FQ_LOG2SZ_1;
	writeq(iommu->base + IOMMU_FQB_OFFSET, fqb);

	fqt = readl(iommu->base + IOMMU_FQT_OFFSET);
	writel(iommu->base + IOMMU_FQH_OFFSET, fqt);

	writel(iommu->base + IOMMU_FQCSR_OFFSET, FQCSR_FQEN | FQCSR_FIE);

	while (!(readl(iommu->base + IOMMU_FQCSR_OFFSET) & FQCSR_FQON)) ;

	iommu->fltq.irq = iommu->fltq_irq;

	return 0;
}

int riscv_iommu_priq_init(struct riscv_iommu *iommu)
{

	return 0;
}
