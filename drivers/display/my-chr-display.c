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
#include "asm/mmio.h"
#include "pci.h"
#include "print.h"
#include "irq.h"
#include "vmap.h"
#include "mm.h"
#include "event.h"
#include "string.h"
#include "pci_device_driver.h"
#include "display.h"

#define MY_CHR_DISPLAY_NAME "MY_CHR_DISPLAY"

#define MY_CHR_DISPLAY_MMIO_CH0_SRC    0x0
#define MY_CHR_DISPLAY_MMIO_CH0_DONE   0x10
#define MY_CHR_DISPLAY_MMIO_SIZE       0x100
#define MY_CHR_DISPLAY_MMIO_DOOR_BELL  0x200

static struct display_device my_chr_display;
static unsigned long base = 0;
static int done;

static void my_chr_display_wait_for_complete(void)
{
	while (!readl(base + MY_CHR_DISPLAY_MMIO_CH0_DONE)) ;
}

#if 0
static void my_chr_display_irq_handler(void *data)
{
	my_chr_display_wait_for_complete();
	done = 1;
}

static int wake_expr(void *data)
{
	int *wake = (int *)data;

	return *wake == 1;
}
#endif
static int my_chr_display_show(unsigned long buf, int size, void *priv)
{
	int ret = 0;

	writeq(base + MY_CHR_DISPLAY_MMIO_CH0_SRC, buf);
	writel(base + MY_CHR_DISPLAY_MMIO_SIZE, size);
	writel(base + MY_CHR_DISPLAY_MMIO_DOOR_BELL, 1);

	my_chr_display_wait_for_complete();
#if 0
	wait_for_event_timeout(&done, wake_expr, 5 * 1000 /* 5s */ );
	if (done == 0)
		ret = -1;
	else {
		done = 0;
		ret = 0;
	}
#endif
	return ret;
}

static struct display_ops my_chr_display_ops = {
	.display = my_chr_display_show,
};

static void __my_chr_display_init(struct pci_device *pdev, struct display_device *dp)
{
	strcpy(dp->name, MY_CHR_DISPLAY_NAME);
	dp->dev = &pdev->dev;
	dp->ops = &my_chr_display_ops;
	dp->priv = NULL;
}

static int my_chr_display_init(struct pci_device *pdev, void *data)
{
	struct resource res;
	int size, i, nr, irqs[32];

	print("my-chr-display(pci-dev[0:%x:%x:%x]): vendor:0x%x device:0x%x\n",
	      pdev->bus->bus_number, PCI_SLOT(pdev->devfn),
	      PCI_FUNC(pdev->devfn), pdev->vendor, pdev->device);

	pci_enable_resource(pdev, 1 << 0);

	pci_set_master(pdev, 1);

	pci_get_resource(pdev, 0, &res);
	size = res.end - res.base + 1;
	base = (unsigned long)ioremap((void *)res.base, res.end - res.base + 1, NULL);

	print("pci-dev[0:%x:%x:%x]: ioremap addr:0x%lx size : 0x%x\n",
	      pdev->bus->bus_number, PCI_SLOT(pdev->devfn),
	      PCI_FUNC(pdev->devfn), res.base, size);
#if 0
	nr = pci_msix_enable(pdev, irqs);
	for (i = 0; i < nr; i++)
		register_device_irq(&pdev->dev, pdev->dev.irq_domain,
				    irqs[i], my_chr_display_irq_handler, NULL);
#endif
	__my_chr_display_init(pdev, &my_chr_display);
	register_display_device(&my_chr_display);

	return 0;
}

PCI_DRIVER_REGISTER(my_chr_display, my_chr_display_init, 0x1234, 0x2);
