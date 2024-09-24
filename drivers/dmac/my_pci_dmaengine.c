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

#include "my_pci_dmaengine.h"
#include "print.h"
#include "pci_device_driver.h"
#include "pci.h"
#include "vmap.h"
#include "asm/type.h"
#include "asm/mmio.h"
#include "dmac.h"
#include "mm.h"
#include "cache_flush.h"
#include "asm/barrier.h"
#include "string.h"
#include "irq.h"
#include "event.h"
#include "dma-mapping.h"

static int done = 0;
static unsigned long base;

static void my_dmaengine_wait_for_complete(void)
{
	while (readl(base + MY_DMAENGINE_MMIO_CH0_DONE)) ;
}

static void my_pci_dmaengine_irq_handler(void *data)
{
	my_dmaengine_wait_for_complete();
	done = 1;
}

static int wake_expr(void *data)
{
	int *wake = (int *)data;

	return *wake == 1;
}

static int my_dmaengine_ioctl(struct device *dev, unsigned int cmd, void *arg)
{
	int ret = 0;
	struct dmac_ioctl_data *data = (struct dmac_ioctl_data *)arg;
	unsigned int size;
	unsigned long src_iova, dst_iova;
	unsigned long src_addr, dst_addr;

	switch (cmd) {
	case MEM_TO_MEM_FIX:
		src_addr = (unsigned long)data->src;
		dst_addr = (unsigned long)data->dst;
		size = data->size;
		if (dma_mapping(dev, src_addr, &src_iova, size, NULL))
			return -1;
		if (dma_mapping(dev, dst_addr, &dst_iova, size, NULL))
			return -1;
		writeq(base + MY_DMAENGINE_MMIO_CH0_SRC, src_iova);
		writeq(base + MY_DMAENGINE_MMIO_CH0_DST, dst_iova);
		writel(base + MY_DMAENGINE_MMIO_CH0_TRAN_SIZE, size);
		writel(base + MY_DMAENGINE_MMIO_CH0_START, 1);
		mb();

#if 0
		wait_for_event_timeout(&done, wake_expr, 5 * 1000 /* 5s */ );
		if (done == 0)
			ret = -1;
		else {
			done = 0;
			ret = 0;
		}
#else
		my_dmaengine_wait_for_complete();
#endif
		break;
	case MEM_TO_MEM:
		size = data->size;
		src_addr = (unsigned long)dma_alloc(dev, &src_iova, size, NULL);
		if (!src_addr)
			return -1;

		for (int i = 0; i < size; i++)
			((char *)(phy_to_virt(src_addr)))[i] = i;

		dst_addr = (unsigned long)dma_alloc(dev, &dst_iova, size, NULL);
		if (!dst_addr)
			return -1;
		print("src_addr:0x%lx dst_addr:0x%lx\n", src_addr, dst_addr);
		writeq(base + MY_DMAENGINE_MMIO_CH0_SRC, src_iova);
		writeq(base + MY_DMAENGINE_MMIO_CH0_DST, dst_iova);
		writel(base + MY_DMAENGINE_MMIO_CH0_TRAN_SIZE, size);
		writel(base + MY_DMAENGINE_MMIO_CH0_START, 1);
		mb();
#if 0
		wait_for_event_timeout(&done, wake_expr, 5 * 1000 /* 5s */ );
		if (done == 0)
			ret = -1;
		else {
			done = 0;
			ret = 0;
		}
#else
		my_dmaengine_wait_for_complete();
#endif
		for (int i = 0; i < size; i++)
			print("dst[%d]: %d\n", i, ((char *)(phy_to_virt(dst_addr)))[i]);
		break;
	}

	return ret;
}

static const struct driver_ops my_dmaengine_ops = {
	.ioctl = my_dmaengine_ioctl,
};

static int my_pci_dmaengine_init(struct pci_device *pdev, void *data)
{
	struct driver *drv;
	struct device *dev = &pdev->dev;
	struct resource res;
	int size, i;
	int irqs[32], nr;

	print("pci-dev[0:%x:%x:%x]: vendor:0x%x device:0x%x\n",
	      pdev->bus->bus_number, PCI_SLOT(pdev->devfn),
	      PCI_FUNC(pdev->devfn), pdev->vendor, pdev->device);

	pci_enable_resource(pdev, 1 << 0);

	pci_set_master(pdev, 1);

	pci_get_resource(pdev, 0, &res);

	size = res.end - res.base + 1;
	print("pci-dev[0:%x:%x:%x]: ioremap addr:0x%lx size : 0x%x\n",
	      pdev->bus->bus_number, PCI_SLOT(pdev->devfn),
	      PCI_FUNC(pdev->devfn), res.base, size);
	base = (unsigned long)ioremap((void *)res.base, res.end - res.base + 1, NULL);

	nr = pci_msix_enable(pdev, irqs);
	for (i = 0; i < nr; i++)
		register_device_irq(&pdev->dev, pdev->dev.irq_domain,
				    irqs[i], my_pci_dmaengine_irq_handler, NULL);

	drv = dev->drv;
	drv->ops = &my_dmaengine_ops;

	register_dmac_device(dev);

	return 0;
}

PCI_DRIVER_REGISTER(my_pci_dmaengine, my_pci_dmaengine_init, 0x1234, 0x1);

