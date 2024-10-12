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

#ifndef __MY_PCI_DMAENGINE_H__
#define __MY_PCI_DMAENGINE_H__

#include "dmac.h"

#define MY_DMAENGINE_MMIO_CH0_SRC        0x0
#define MY_DMAENGINE_MMIO_CH0_DST        0x8
#define MY_DMAENGINE_MMIO_CH0_TRAN_SIZE  0x100
#define MY_DMAENGINE_MMIO_CH0_START      0x200
#define MY_DMAENGINE_MMIO_CH0_DONE       0x10

#define MY_DMAENGINE_MMIO_CH1_SRC        0x1000
#define MY_DMAENGINE_MMIO_CH1_DST        0x1008
#define MY_DMAENGINE_MMIO_CH1_TRAN_SIZE  0x1100
#define MY_DMAENGINE_MMIO_CH1_START      0x1200
#define MY_DMAENGINE_MMIO_CH1_DONE       0x1010

struct dmac_my_pci_dmaengine {
	struct dmac_device dmac;
	unsigned long base;
	int done;
};

#endif

