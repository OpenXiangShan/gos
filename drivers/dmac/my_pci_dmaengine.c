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

static struct dmac_my_pci_dmaengine *my_dmac;

static void my_dmaengine_wait_for_complete(void)
{
	while (readl(my_dmac->base + MY_DMAENGINE_MMIO_CH0_DONE)) ;
}

static void my_pci_dmaengine_irq_handler(void *data)
{
	my_dmaengine_wait_for_complete();
	my_dmac->done = 1;
}

#if 0
static int wake_expr(void *data)
{
	int *wake = (int *)data;

	return *wake == 1;
}
#endif

static int my_pci_dmaengine_transfer_m2m(unsigned long src, unsigned long dst, int size, void *priv)
{
	writeq(my_dmac->base + MY_DMAENGINE_MMIO_CH0_SRC, src);
	writeq(my_dmac->base + MY_DMAENGINE_MMIO_CH0_DST, dst);
	writel(my_dmac->base + MY_DMAENGINE_MMIO_CH0_TRAN_SIZE, size);
	writel(my_dmac->base + MY_DMAENGINE_MMIO_CH0_START, 1);
	mb();

#if 0
	wait_for_event_timeout(&my_dmac->done, wake_expr, 5 * 1000 /* 5s */ );
	if (my_dmac->done == 0)
		ret = -1;
	else {
		my_dmac->done = 0;
		ret = 0;
	}
#else
	my_dmaengine_wait_for_complete();
#endif
	return 0;
}

static struct dmac_ops my_pci_dmaengine_ops = {
	.transfer_m2m = my_pci_dmaengine_transfer_m2m,
};

static int my_pci_dmaengine_init(struct pci_device *pdev, void *data)
{
	struct dmac_device *dmac;
	struct device *dev = &pdev->dev;
	struct resource res;
	int size, i;
	int irqs[32], nr;

	print("pci-dev[0:%x:%x:%x]: vendor:0x%x device:0x%x\n",
	      pdev->bus->bus_number, PCI_SLOT(pdev->devfn),
	      PCI_FUNC(pdev->devfn), pdev->vendor, pdev->device);

	my_dmac = (struct dmac_my_pci_dmaengine *)mm_alloc(sizeof(struct dmac_my_pci_dmaengine));
	if (!my_dmac) {
		print("my-pci-dmaengine: alloc dmac device failed..\n");
		return -1;
	}
	memset((char *)my_dmac, 0, sizeof(my_dmac));

	pci_enable_resource(pdev, 1 << 0);

	pci_set_master(pdev, 1);

	pci_get_resource(pdev, 0, &res);

	size = res.end - res.base + 1;
	print("pci-dev[0:%x:%x:%x]: ioremap addr:0x%lx size : 0x%x\n",
	      pdev->bus->bus_number, PCI_SLOT(pdev->devfn),
	      PCI_FUNC(pdev->devfn), res.base, size);
	my_dmac->base = (unsigned long)ioremap((void *)res.base, res.end - res.base + 1, NULL);

	nr = pci_msix_enable(pdev, irqs);
	for (i = 0; i < nr; i++)
		register_device_irq(&pdev->dev, pdev->dev.irq_domain,
				    irqs[i], my_pci_dmaengine_irq_handler, NULL);

	dmac = &my_dmac->dmac;
	dmac->dev = dev;
	dmac->ops = &my_pci_dmaengine_ops;

	register_dmac_device(dmac);

	return 0;
}

PCI_DRIVER_REGISTER(my_pci_dmaengine, my_pci_dmaengine_init, 0x1234, 0x1);

