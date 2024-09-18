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

#ifndef __PCI_DEVICE_DRIVER_H__
#define __PCI_DEVICE_DRIVER_H__

#include "pci.h"
#include "container_of.h"
#include "device.h"

#define PCI_DRIVER_INIT_TABLE __pci_driver_init_table
#define PCI_DRIVER_INIT_TABLE_END __pci_driver_init_table_end

typedef int (*pci_driver_init)(struct pci_device *dev, void *data);

struct pci_driver {
	struct list_head list;
	struct driver drv;
};

struct pci_driver_init_entry {
	int vendor_id;
	int device_id;
	pci_driver_init init;
};

#define PCI_DRIVER_REGISTER(name, init_fn, vid, did)         \
	static const struct pci_driver_init_entry __attribute__((used))  \
		__pci_driver_init_##name                                 \
		__attribute__((section(".pci_driver_init_table"))) = {   \
			.vendor_id = vid,                          \
			.device_id = did,                          \
			.init = init_fn,                                 \
		}

#define to_pci_dev(dev) container_of(dev, struct pci_device, dev)
#define to_pci_driver(pdev) container_of(&pdev->dev.drv, struct pci_driver, drv)

int pci_register_device(struct pci_device *pci_dev);
int pci_probe_driver(void);
void walk_pci_devices(void);

#endif
