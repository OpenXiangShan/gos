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
#include "asm/pgtable.h"
#include "dma.h"

static struct dma_ops *dma_ops = NULL;

int set_dma_ops(struct dma_ops *ops)
{
	if (dma_ops)
		return -1;

	dma_ops = ops;

	return 0;
}

int dma_m2m(void *src, void *dst, int size)
{
	unsigned long src_pa, dst_pa;

	if (!dma_ops)
		return -1;

	src_pa = virt_to_phy(src);
	dst_pa = virt_to_phy(dst);

	return dma_ops->dma_m2m(src_pa, dst_pa, size);
}
