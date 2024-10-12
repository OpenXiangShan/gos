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
#include "mm.h"
#include "pci_simple.h"
#include "print.h"
#include "list.h"
#include "align.h"

static struct pci_simple_root_bus *root = NULL;

static void *pci_simple_config_map(int devfn, int reg)
{
	return (void *)(root->pci_config_base + PCI_ECAM_OFFSET(0, devfn, reg));
}

static unsigned int pci_simple_config_read(int devfn, int reg, int len)
{
	void *ecam_addr;
	unsigned int ret;

	ecam_addr = pci_simple_config_map(devfn, reg);

	if (len == 1)
		ret = readb(ecam_addr);
	else if (len == 2)
		ret = readb(ecam_addr) + (((unsigned int)readb(ecam_addr + 1)) << 8);
	else if (len == 4)
		ret = readl(ecam_addr);

	return ret;
}

static int pci_simple_config_write(int devfn, int reg, int len, unsigned int val)
{
	void *ecam_addr;

	ecam_addr = pci_simple_config_map(devfn, reg);

	if (len == 1)
		writeb(ecam_addr, val);
	else if (len == 2) {
		writeb(ecam_addr, val & 0xff);
		writeb(ecam_addr + 1, (val >> 8) & 0xff);
	}
	else if (len == 4)
		writel(ecam_addr, val);

	return 0;
}

unsigned int pci_simple_read_config_byte(int devfn, int reg)
{
	return pci_simple_config_read(devfn, reg, 1);
}

unsigned int pci_simple_read_config_word(int devfn, int reg)
{
	return pci_simple_config_read(devfn, reg, 2);
}

unsigned int pci_simple_read_config_dword(int devfn, int reg)
{
	return pci_simple_config_read(devfn, reg, 4);
}

int pci_simple_write_config_byte(int devfn, int reg, unsigned int val)
{
	return pci_simple_config_write(devfn, reg, 1, val);
}

int pci_simple_write_config_word(int devfn, int reg, unsigned int val)
{
	return pci_simple_config_write(devfn, reg, 2, val);
}

int pci_simple_write_config_dword(int devfn, int reg, unsigned int val)
{
	return pci_simple_config_write(devfn, reg, 4, val);
}

static unsigned int pci_simple_read_vendor_id(int devfn)
{
	return pci_simple_read_config_dword(devfn, PCI_VENDOR_ID);
}

static int _decode_bar(struct bar *b, unsigned int val)
{
	unsigned int mem_type;
	unsigned long addr_type;

	if ((val & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO) {
		mem_type = pci_mem_type_io;
		return 0;
	}

	if ((val & PCI_BASE_ADDRESS_MEM_PREFETCH) == PCI_BASE_ADDRESS_MEM_PREFETCH)
		mem_type = pci_mem_type_pref_mem;
	else
		mem_type = pci_mem_type_mem;

	switch (val & PCI_BASE_ADDRESS_MEM_TYPE_MASK) {
	case PCI_BASE_ADDRESS_MEM_TYPE_32:
		addr_type = pci_addr_type_mem_32;
		break;
	case PCI_BASE_ADDRESS_MEM_TYPE_1M:
		addr_type = pci_addr_type_mem_1M;
		break;
	case PCI_BASE_ADDRESS_MEM_TYPE_64:
		addr_type = pci_addr_type_mem_64;
		break;
	}

	b->mem_type = mem_type;
	b->addr_type = addr_type;

	return 0;
}

static int __pci_simple_read_bar(struct pci_device *dev, unsigned int reg, struct bar *b)
{
	unsigned int l = 0, sz = 0, mask;
	unsigned long l64, sz64, mask64;
	unsigned int size = 0;
	int ret = 0;

	mask = ~0;

	l = pci_simple_read_config_dword(dev->devfn, reg);
	pci_simple_write_config_dword(dev->devfn, reg, l | mask);
	sz = pci_simple_read_config_dword(dev->devfn, reg);
	pci_simple_write_config_dword(dev->devfn, reg, l);

	_decode_bar(b, l);
	if (b->mem_type == pci_mem_type_io) {
		return 0;
	} else {
		l64 = l & PCI_BASE_ADDRESS_MEM_MASK;
		sz64 = sz & PCI_BASE_ADDRESS_MEM_MASK;
		mask64 = (unsigned int)PCI_BASE_ADDRESS_MEM_MASK;
	}

	if (b->addr_type == pci_addr_type_mem_64) {
		l = pci_simple_read_config_dword(dev->devfn, reg + 4);
		pci_simple_write_config_dword(dev->devfn, reg + 4, ~0);
		sz = pci_simple_read_config_dword(dev->devfn, reg + 4);
		pci_simple_write_config_dword(dev->devfn, reg + 4, l);

		l64 |= ((unsigned long)l << 32);
		sz64 |= ((unsigned long)sz << 32);
		mask64 |= ((unsigned long)~0 << 32);

		ret = 1;
	}

	if (!sz64)
		return ret;

	size = sz64 & mask64;
	size = size & ~(size - 1);

	b->base = l64;
	b->size = size;

	return ret;
}

static int pci_simple_read_base_address(struct pci_device *dev, int nr)
{
	int pos;
	unsigned int reg;

	for (pos = 0; pos < nr; pos++) {
		reg = PCI_BASE_ADDRESS_0 + (pos << 2);
		pos += __pci_simple_read_bar(dev, reg, &dev->bar[pos]);
	}

	return 0;
}

static unsigned long __alloc_resource(int size, unsigned long start, unsigned long end)
{
	unsigned long align;

	align = ALIGN_SIZE(start, size);
	if ((align + size) > end)
		return -1;

	return (align + size);
}

static void pci_simple_assign_device_resource(void)
{
	struct pci_device *dev;
	int i;
	unsigned long pci_mmio_start = root->res.base;
	unsigned long pci_mmio_end = root->res.end;
	struct bar *bar;
	unsigned int reg, val;

	list_for_each_entry(dev, &root->devices, list) {
		print("pci-simple: 0:0:%x:%x: [%x:%x] :\n",
		      PCI_SLOT(dev->devfn),
		      PCI_FUNC(dev->devfn),
		      dev->vendor, dev->device);
		for (i = 0; i < 6; i++) {
			bar = &dev->bar[i];
			if (bar->size == 0)
				continue;
			pci_mmio_start = __alloc_resource(bar->size, pci_mmio_start, pci_mmio_end);
			if (pci_mmio_start == -1) {
				print("warning -- pci_simple_get_assign_device_resource fail\n");
				return;
			}
			bar->base = pci_mmio_start - bar->size;
			print("  BAR[%d] : 0x%lx - 0x%lx\n",
			      i, bar->base, bar->base + bar->size - 1);
			reg = PCI_BASE_ADDRESS_0 + i * sizeof(unsigned int);
			val = pci_simple_read_config_dword(dev->devfn, reg);
			val &= 0xfUL;
			val |= bar->base & 0xffffffffUL;
			pci_simple_write_config_dword(dev->devfn, reg, val);
			if ((val & 0xFUL) == PCI_BASE_ADDRESS_MEM_TYPE_64) {
				val = bar->base >> 32;
				pci_simple_write_config_dword(dev->devfn, reg + 4, val);
			}
		}
	}
}

static struct pci_device *pci_simple_setup_device(int devfn, unsigned int vendor_id)
{
	struct pci_device *dev;

	dev = (struct pci_device *)mm_alloc(sizeof(struct pci_device));
	if (!dev) {
		return NULL;
	}

	dev->devfn = devfn;
	dev->vendor = vendor_id & 0xffff;
	dev->device = (vendor_id >> 16) & 0xffff;

	list_add_tail(&dev->list, &root->devices);

	return dev;
}

static void pci_probe_driver(void)
{
	extern struct pci_driver_init_entry PCI_DRIVER_INIT_TABLE,
		PCI_DRIVER_INIT_TABLE_END;
	struct pci_device *dev;

	list_for_each_entry(dev, &root->devices, list) {
		struct pci_driver_init_entry *entry;
		struct pci_driver_init_entry *from = &PCI_DRIVER_INIT_TABLE;
		struct pci_driver_init_entry *to = &PCI_DRIVER_INIT_TABLE_END;
		int nr = to - from;

		for (entry = from; nr; entry++, nr--) {
			if (entry->vendor_id == dev->vendor &&
			    entry->device_id == dev->device) {
				entry->init(dev, NULL);
			}
		}
	}
}

int pci_simple_get_resource(struct pci_device *dev, int bar, struct resource *res)
{
	struct bar *b = &dev->bar[bar];

	if (!b)
		return -1;

	if (b->size == 0) {
		res->base = 0;
		res->end = 0;
	}
	else {
		res->base = b->base;
		res->end = res->base + b->size - 1;
	}

	return 0;
}

int pci_simple_register_root_bus(unsigned long ecam_addr,
				 unsigned long mmio, unsigned int size)
{
	if (root)
		return -1;

	root = (struct pci_simple_root_bus *)mm_alloc(sizeof(struct pci_simple_root_bus));
	if (!root)
		return -1;

	root->pci_config_base = ecam_addr;
	root->res.base = mmio;
	root->res.end = mmio + size - 1;
	INIT_LIST_HEAD(&root->devices);

	return 0;
}

int pci_simple_probe_root_bus(void)
{
	int devfn, fn;
	unsigned int vendor_id = 0;
	char hdr_type;
	struct pci_device *dev;

	for (devfn = 0; devfn < 256; devfn += 8) {
		for (fn = devfn; fn < devfn + 8; fn++) {
			vendor_id = pci_simple_read_vendor_id(fn);
			if (vendor_id == 0xffffffff)
				break;
			hdr_type = pci_simple_read_config_byte(fn, PCI_HEAD_TYPE);
			if (hdr_type == PCI_HEADER_TYPE_NORMAL) {
				dev = pci_simple_setup_device(devfn, vendor_id);
				if (!dev)
					return -1;
				dev->class = pci_simple_read_config_dword(devfn, PCI_CLASS_REVISION);
				pci_simple_read_base_address(dev, 6);
			}
			else if (hdr_type == PCI_HEADER_TYPE_BRIDGE) {
				print("%s -- find a pcie brigde!! But myGuest do not support that, ToDo...\n", __FUNCTION__);
			}
		}
	}

	pci_simple_assign_device_resource();

	pci_probe_driver();

	return 0;
}
