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

#include <dmac.h>
#include <print.h>
#include <string.h>
#include "device.h"
#include "asm/pgtable.h"
#include "list.h"
#include "mm.h"
#include "irq.h"

extern int mmu_is_on;
static LIST_HEAD(dmacs);
static unsigned long dmac_idx_bitmap __attribute__((section(".data"))) = 0;

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

int register_dmac_device(struct device *dev)
{
	struct dmac_device *dmac;
	int index;

	dmac = (struct dmac_device *)mm_alloc(sizeof(struct dmac_device));
	if (!dmac) {
		print("dmac: register dmac failed.. Out of memory\n");
		return -1;
	}

	index = find_free_dmac_index();
	sprintf(dmac->name, "DMAC%d", index);
	strcpy(dev->name, dmac->name);
	strcpy(dev->drv->name, dmac->name);

	dmac->dev = dev;
	list_add_tail(&dmac->list, &dmacs);

	return 0;
}

int memcpy_hw(char * name, char *dst, char *src, unsigned int size)
{
	void *_src, *_dst;
	struct dmac_ioctl_data data;
	unsigned int dma_width = 0;
	unsigned int blockTS = (size >> dma_width) - 1;
	int fd = 0;

	fd = open(name);
	if (fd == -1) {
		print("open %s fail.\n", name);
		return -1;
	}

	memset((char *)&data, 0, sizeof(struct dmac_ioctl_data));

	if (mmu_is_on) {
		_src = (void *)virt_to_phy(src);
		_dst = (void *)virt_to_phy(dst);
	}

	data.src = _src;
	data.dst = _dst;
	data.blockTS = blockTS;
	data.src_addr_inc = 0;
	data.des_addr_inc = 0;
	data.src_width = dma_width;
	data.des_width = dma_width;
	data.src_burstsize = 0;
	data.des_burstsize = 0;
	data.burst_len = 7;
	data.size = size;

	return ioctl(fd, MEM_TO_MEM, &data);
}

int dma_transfer(char * name, char *dst, char *src, unsigned int size,
		 unsigned int data_width, unsigned int burst_len)
{
	struct dmac_ioctl_data data;
	unsigned int blockTS = (size >> data_width) - 1;
	int fd = 0;

	fd = open(name);
	if (fd == -1) {
		print("open %s fail.\n", name);
		return -1;
	}

	memset((char *)&data, 0, sizeof(struct dmac_ioctl_data));

	data.src = src;
	data.dst = dst;
	data.blockTS = blockTS;
	data.src_addr_inc = 0;
	data.des_addr_inc = 0;
	data.src_width = data_width;
	data.des_width = data_width;
	data.src_burstsize = 0;
	data.des_burstsize = 0;
	data.burst_len = burst_len;
	data.size = size;

	return ioctl(fd, MEM_TO_MEM, &data);
}
