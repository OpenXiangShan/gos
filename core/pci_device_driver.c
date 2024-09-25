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

#include "pci.h"
#include "list.h"
#include "asm/type.h"
#include "pci_device_driver.h"
#include "device.h"
#include "mm.h"
#include "string.h"
#include "print.h"
#include "iommu.h"

static LIST_HEAD(pci_devices);
static LIST_HEAD(pci_drivers);

static struct pci_driver *create_pci_driver(void)
{
	struct pci_driver *new;

	new = mm_alloc(sizeof(struct pci_driver));
	if (!new) {
		print("%s -- Out of memory!\n");
		return NULL;
	}
	memset((char *)new, 0 , sizeof(struct pci_driver));


	list_add_tail(&new->list, &pci_drivers);

	add_driver(&new->drv);

	return new;
}

static struct iommu *pci_get_bus_iommu(struct pci_device *pdev)
{
	struct pci_bus *root = pdev->bus;

	while (root->parent)
		root = root->parent;

	return root->dev->iommu;
}

void pci_set_device_iommu(struct pci_device *pdev)
{
	pdev->dev.iommu = pci_get_bus_iommu(pdev);

	pdev->dev.dev_id = PCI_DEVID(pdev->bus->bus_number, pdev->devfn);

	if (pdev->dev.iommu)
		iommu_attach_device(&pdev->dev, pdev->dev.iommu);
}

int pci_register_device(struct pci_device *pci_dev)
{
	list_add_tail(&pci_dev->dev.list, &pci_devices);

	return 0;
}

int pci_probe_driver(void)
{
	extern struct pci_driver_init_entry PCI_DRIVER_INIT_TABLE,
		PCI_DRIVER_INIT_TABLE_END;
	struct pci_device *pci_dev;
	struct device *dev;
	struct pci_driver *pdrv;
	struct driver *drv;

	list_for_each_entry(dev, &pci_devices, list) {
		struct pci_driver_init_entry *entry;
		struct pci_driver_init_entry *from = &PCI_DRIVER_INIT_TABLE;
		struct pci_driver_init_entry *to = &PCI_DRIVER_INIT_TABLE_END;
		int nr = to - from;

		pci_dev = to_pci_dev(dev);
		pdrv = create_pci_driver();

		for (entry = from; nr; entry++, nr--) {
			if (entry->vendor_id == pci_dev->vendor &&
			    entry->device_id == pci_dev->device) {
				drv = &pdrv->drv;
				drv->probe = 1;
				dev->probe = 1;
				dev->drv = drv;
				drv->dev = dev;
				entry->init(pci_dev, NULL);
			}
		}
	}

	return 0;
}

void walk_pci_devices(int print_conf)
{
	struct device *dev;
	struct pci_device *pdev;
	int i;
	char config[256];

	list_for_each_entry(dev, &pci_devices, list) {
		pdev = to_pci_dev(dev);
		print("pci 0:%x:%x:%x: [%x:%x]:\n",
		       pdev->bus->bus_number, PCI_SLOT(pdev->devfn),
		       PCI_FUNC(pdev->devfn), pdev->vendor,
		       pdev->device);
		for (i = 0; i < 6; i++) {
			if (pdev->bar[i].size != 0) {
				print("    BAR[%d] : [0x%lx - 0x%lx]\n",
				      i,
				      pdev->bar[i].base,
				      pdev->bar[i].base + pdev->bar[i].size - 1);
			}
		}
		if (print_conf) {
			memset(config, 0, 256);
			pci_get_config(pdev, config);
			print("    00: ");
			for (i = 0; i < 256; i++) {
				if (i % 16 == 0 && i != 0) {
					print("\n");
					print("    %02x: ", i);
				}
				print("%02x ", config[i]);
			}
			print("\n");
		}
	}
}
