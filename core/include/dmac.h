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

enum {
	DMAC_XFER_M2M = 0,
};

struct dmac_ops {
	int (*transfer_m2m)(unsigned long src, unsigned long dst, int size, void *priv);
};

struct dmac_device {
	struct list_head list;
	char name[64];
	struct device *dev;
	struct dmac_ops *ops;
	void *priv;
};

int register_dmac_device(struct dmac_device *dmac);
void walk_all_dmac(void);
int memcpy_hw(char *name, char *dst, char *src, unsigned int size);
int dma_transfer(struct dmac_device *dmac, char *dst, char *src, unsigned int size, int dir);

#endif
