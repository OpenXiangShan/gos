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

#include "asm/mmio.h"
#include "asm/type.h"
#include "print.h"
#include "mm.h"
#include "vmap.h"
#include "pci_simple.h"
#include "dma.h"

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

static unsigned long base_addr;

static int my_pci_dmaengine_m2m(unsigned long src, unsigned long dst, int size)
{
	writeq(base_addr + MY_DMAENGINE_MMIO_CH0_SRC, src);
	writeq(base_addr + MY_DMAENGINE_MMIO_CH0_DST, dst);
	writel(base_addr + MY_DMAENGINE_MMIO_CH0_TRAN_SIZE, size);
	writel(base_addr + MY_DMAENGINE_MMIO_CH0_START, 1);

	while (readl(base_addr + MY_DMAENGINE_MMIO_CH0_DONE)) ;

	return 0;
}

static struct dma_ops my_pci_dmaengine_ops = {
	.dma_m2m = my_pci_dmaengine_m2m,
};

static int my_pci_dmaengine_init(struct pci_device *pdev, void *data)
{
	struct resource res;

	print("my-pci-dmaengine[%x:%x] init\n", pdev->vendor, pdev->device);

	if (pci_simple_get_resource(pdev, 0, &res)) {
		print("pci [%x:%x] -- pci get resource fail..", pdev->vendor, pdev->device);
		return -1;
	}

	base_addr = (unsigned long)ioremap((void *)res.base, res.end - res.base + 1, NULL);
	if (set_dma_ops(&my_pci_dmaengine_ops)) {
		print("%s -- set_dma_ops fail\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

PCI_DRIVER_REGISTER(my_pci_dmaengine, my_pci_dmaengine_init, 0x1234, 0x1);

