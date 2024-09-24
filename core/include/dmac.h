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

#ifndef DMAC_H
#define DMAC_H

#include "list.h"
#include "device.h"

enum DMA_TYPE {
	MEM_TO_MEM_FIX = 0,
	MEM_TO_MEM,
};

struct dmac_ioctl_data {
	void *src;
	void *dst;
	unsigned int blockTS;
	unsigned int src_addr_inc;
	unsigned int des_addr_inc;
	unsigned int src_width;
	unsigned int des_width;
	unsigned int src_burstsize;
	unsigned int des_burstsize;
	unsigned int burst_len;
	unsigned int size;
};

struct dmac_device {
	struct list_head list;
	char name[64];
	struct device *dev;
};

int register_dmac_device(struct device *dev);
void walk_all_dmac(void);
int memcpy_hw(char *name, char *dst, char *src, unsigned int size);
int dma_transfer(char *name, char *dst, char *src, unsigned int size,
		 unsigned int data_width, unsigned int burst_len);

#endif
