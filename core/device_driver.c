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

#include <device.h>
#include <asm/type.h>
#include <string.h>
#include <mm.h>
#include <print.h>
#include "irq.h"
#include "vmap.h"
#include "list.h"
#include "iommu.h"
#include "pci_device_driver.h"
#include "gos.h"

extern int mmu_is_on;

static LIST_HEAD(_devices);
static LIST_HEAD(_drivers);
static unsigned long driver_idx_bitmap = 0x1;

static int find_free_drv_index(void)
{
	unsigned long bitmap = driver_idx_bitmap;
	int pos = 0;

	while (bitmap & 0x01) {
		if (pos == 64)
			return -1;
		bitmap = bitmap >> 1;
		pos++;
	}

	driver_idx_bitmap |= (1UL) << pos;

	return pos;
}

static struct device *create_device(struct device_init_entry *entry)
{
	struct device *new;
	int i;

	new = (struct device *)mm_alloc(sizeof(struct device));
	if (!new) {
		print("%s -- alloc device failed!\n", __FUNCTION__);
		return NULL;
	}
	memset((char *)new, 0, sizeof(struct device));

	new->base = entry->start;
	new->len = entry->len;
	new->irq_num = entry->irq_num;
	for(i = 0; i < entry->irq_num; i++)
		new->irqs[i] = entry->irq[i];
	new->irq_domain = find_irq_domain(entry->irq_parent);
#if CONFIG_IOMMU
	new->iommu = find_iommu(entry->iommu);
#endif
	strcpy(new->compatible, entry->compatible);

	list_add_tail(&new->list, &_devices);

	return new;
}

void add_driver(struct driver *drv)
{
	drv->index = find_free_drv_index();

	list_add_tail(&drv->list, &_drivers);
}

struct driver *create_driver(struct driver_init_entry *entry)
{
	struct driver *new;

	new = (struct driver *)mm_alloc(sizeof(struct driver));
	if (!new) {
		print("%s -- alloc driver failed!\n", __FUNCTION__);
		return NULL;
	}

	new->index = find_free_drv_index();

	list_add_tail(&new->list, &_drivers);

	return new;
}

static int __NoNeed_create_device(struct device_init_entry *entry)
{
	if (!strncmp(entry->compatible, "clint", sizeof("clint")))
		return 1;
	if (!strncmp(entry->compatible, "PLIC", sizeof("PLIC")))
		return 1;
	if (!strncmp(entry->compatible, "IMSIC", sizeof("IMSIC")))
		return 1;
	if (!strncmp(entry->compatible, "IMSIC_M", sizeof("IMSIC_M")))
		return 1;
	if (!strncmp(entry->compatible, "APLIC_S", sizeof("APLIC_S")))
		return 1;
	if (!strncmp(entry->compatible, "APLIC_M", sizeof("APLIC_M")))
		return 1;

	return 0;
}

static int __probe_device_table(struct driver_init_entry *driver_head,
				struct driver_init_entry *driver_end,
				struct device_init_entry *hw)
{
	struct driver_init_entry *driver_entry;
	struct device_init_entry *device_entry = hw;
	struct device *dev;
	struct driver *drv;
	int driver_nr = driver_end - driver_head;
	int driver_nr_tmp;

	while (strncmp(device_entry->compatible, "THE END", sizeof("THE END"))) {
		if (__NoNeed_create_device(device_entry))
			goto next_device_entry;

		driver_nr_tmp = driver_nr;
		dev = create_device(device_entry);
		if (dev->iommu)
			iommu_attach_device(dev, dev->iommu, 0);
		for (driver_entry = driver_head; driver_nr_tmp;
		     driver_entry++, driver_nr_tmp--) {
			drv = create_driver(driver_entry);
			if (!strncmp
			    (driver_entry->compatible, device_entry->compatible,
			     128)) {
				strcpy(dev->compatible,
				       device_entry->compatible);
				dev->drv = drv;
				dev->probe = 1;
				drv->dev = dev;
				drv->probe = 1;
				driver_entry->init(dev, device_entry->data);
			}
		}
next_device_entry:
		device_entry++;
	}

	return 0;
}

int device_driver_init(struct device_init_entry *hw)
{
	extern struct driver_init_entry DRIVER_INIT_TABLE,
	    DRIVER_INIT_TABLE_END;

	/* probe devices and drivers */
	return __probe_device_table((struct driver_init_entry *)&DRIVER_INIT_TABLE,
				    (struct driver_init_entry *)&DRIVER_INIT_TABLE_END,
				    hw);
}

int open(char *name)
{
	struct driver *drv;

	list_for_each_entry(drv, &_drivers, list) {
		if (!drv->probe)
			continue;
		if (!strncmp(name, drv->name, 64))
			return drv->index;
	}

	return -1;
}

int read(int fd, char *buf, unsigned long offset, unsigned int len, int flag)
{
	struct driver *drv;

	list_for_each_entry(drv, &_drivers, list) {
		if (drv->index == fd)
			goto find;
	}

	return -1;
find:

	if (!drv->ops->read) {
		return NULL;
	}

	return drv->ops->read(drv->dev, buf, offset, len, flag);

}

int write(int fd, char *buf, unsigned long offset, unsigned int len)
{
	struct driver *drv;

	list_for_each_entry(drv, &_drivers, list) {
		if (drv->index == fd)
			goto find;
	}

	return -1;
find:

	if (!drv->ops->write) {
		return NULL;
	}

	return drv->ops->write(drv->dev, buf, offset, len);
}

int ioctl(int fd, unsigned int cmd, void *arg)
{
	struct driver *drv;

	list_for_each_entry(drv, &_drivers, list) {
		if (drv->index == fd)
			goto find;
	}

	return -1;
find:
	if (!drv->ops || !drv->ops->ioctl) {
		return NULL;
	}

	return drv->ops->ioctl(drv->dev, cmd, arg);
}

struct device *get_device(char *name)
{
	struct device *dev;
#if CONFIG_PCI
	dev = pci_get_device(name);
	if (dev)
		return dev;
#endif
	list_for_each_entry(dev, &_devices, list) {
		if (!strncmp(dev->compatible, name, 128))
			return dev;
	}

	return NULL;
}

struct list_head *get_devices(void)
{
	return &_devices;
}

struct list_head *get_drivers(void)
{
	return &_drivers;
}

void walk_devices()
{
	struct device *dev;
	int id = 0, i;

	print("================= walk devices =================\n");
	list_for_each_entry(dev, &_devices, list) {
		print("device %d\n", id++);
		print("    name: %s\n", dev->compatible);
		print("    base address: 0x%lx\n", dev->base);
		if (dev->irq_domain) {
			print("    irq domain: %s\n", dev->irq_domain->name);
			print("    irq: ");
			for (i = 0; i < dev->irq_num; i++)
				print("%d ", dev->irqs[i]);
			print("\n");
		}
		if (dev->iommu) {
			print("    iommu: %s\n", dev->iommu->name);
		}
		print("    probe: %d\n", dev->probe);

	}
}

void device_get_resource(struct device *dev, struct resource *res)
{
	res->base = dev->base;
	res->end = dev->base + dev->len - 1;
}
