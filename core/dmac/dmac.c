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

#include "print.h"
#include "string.h"
#include "device.h"
#include "asm/pgtable.h"
#include "asm/type.h"
#include "list.h"
#include "mm.h"
#include "irq.h"
#include "dmac.h"
#include "dma-mapping.h"

static LIST_HEAD(dmacs);
static unsigned long dmac_idx_bitmap = 0;

static int find_free_dmac_index(void)
{
	unsigned long bitmap = dmac_idx_bitmap;
	int pos = 0;

	while (bitmap & 0x01) {
		if (pos == 64)
			return -1;
		bitmap = bitmap >> 1;
		pos++;
	}

	dmac_idx_bitmap |= (1UL) << pos;

	return pos;
}

struct dmac_device *get_dmac(char *name)
{
	struct dmac_device *dmac;

	list_for_each_entry(dmac, &dmacs, list){
		if (!strcmp(dmac->name, name))
			return dmac;
	}

	return NULL;
}

void walk_all_dmac(void)
{
	struct dmac_device *dmac;
	int i;

	list_for_each_entry(dmac, &dmacs, list){
		print("%s:\n", dmac->name);
		print("    device:0x%lx\n", dmac->dev);
		print("    device name:%s\n", dmac->dev->compatible);
		if (dmac->dev->irq_domain) {
			print("    irq domain: %s\n", dmac->dev->irq_domain->name);
			print("    irq:");
			for (i = 0; i < dmac->dev->irq_num; i++)
				print("%d ", dmac->dev->irqs[i]);
			print("\n");
		}
	}
}

int register_dmac_device(struct dmac_device *dmac)
{
	int index;

	index = find_free_dmac_index();
	sprintf(dmac->name, "DMAC%d", index);

	list_add_tail(&dmac->list, &dmacs);

	return 0;
}

int dma_transfer(struct dmac_device *dmac, char *dst, char *src, unsigned int size, int dir)
{
	unsigned long src_iova;
	unsigned long dst_iova;

	if ((((unsigned long)dst) % PAGE_SIZE) || (((unsigned long)src) % PAGE_SIZE)) {
		print("dma_transfer: Only support PAGE_SIZE algin memcpy\n");
		return -1;
	}

	if (!dmac || !dmac->ops || !dmac->dev)
		return -1;

	if (dma_mapping(dmac->dev, virt_to_phy(src), &src_iova, size, NULL))
		return -1;

	if (dma_mapping(dmac->dev, virt_to_phy(dst), &dst_iova, size, NULL))
		return -1;

	if (dir == DMAC_XFER_M2M) {
		if (!dmac->ops->transfer_m2m)
			return -1;
		return dmac->ops->transfer_m2m(src_iova, dst_iova, size, dmac->priv);
	}

	return -1;
}

int memcpy_hw(char * name, char *dst, char *src, unsigned int size)
{
	struct dmac_device *dmac;

	list_for_each_entry(dmac, &dmacs, list) {
		if (!strncmp(dmac->name, name, 128))
			goto found;
	}

	return -1;

found:
	return dma_transfer(dmac, dst, src, size, DMAC_XFER_M2M);
}
