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

static unsigned long base;

static void my_dmaengine_wait_for_complete(void)
{
	while (readl(base + MY_DMAENGINE_MMIO_CH0_DONE)) ;
}

static int my_dmaengine_ioctl(struct device *dev, unsigned int cmd, void *arg)
{
	int ret = 0;
	struct dmac_ioctl_data *data = (struct dmac_ioctl_data *)arg;
	unsigned long src_addr, des_addr;
	unsigned int size;

	switch (cmd) {
	case MEM_TO_MEM:
		src_addr = (unsigned long)data->src;
		des_addr = (unsigned long)data->dst;
		size = data->size;
		writeq(base + MY_DMAENGINE_MMIO_CH0_SRC, src_addr);
		writeq(base + MY_DMAENGINE_MMIO_CH0_DST, des_addr);
		writel(base + MY_DMAENGINE_MMIO_CH0_TRAN_SIZE, size);
		writel(base + MY_DMAENGINE_MMIO_CH0_START, 1);
		mb();

		my_dmaengine_wait_for_complete();

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
	int size;

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

	drv = dev->drv;
	strcpy(dev->name, "DMAC0");
	strcpy(drv->name, "DMAC0");
	drv->ops = &my_dmaengine_ops;

	register_dmac_device(dev);

	return 0;
}

PCI_DRIVER_REGISTER(my_pci_dmaengine, my_pci_dmaengine_init, 0x1234, 0x1);

