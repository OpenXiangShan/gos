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

#include "print.h"
#include "pci.h"
#include "asm/type.h"
#include "mm.h"
#include "list.h"
#include "string.h"
#include "align.h"
#include "container_of.h"
#include "pci_device_driver.h"
#include "irq.h"
#include "device.h"
#include "spinlocks.h"
#include "gos.h"

unsigned int pci_read_config_byte(struct pci_bus *bus, int devfn, int addr)
{
	return bus->ops->read(bus, devfn, addr, 1);
}

unsigned int pci_read_config_word(struct pci_bus *bus, int devfn, int addr)
{
	return bus->ops->read(bus, devfn, addr, 2);
}

unsigned int pci_read_config_dword(struct pci_bus *bus, int devfn, int addr)
{
	return bus->ops->read(bus, devfn, addr, 4);
}

unsigned int pci_write_config_byte(struct pci_bus *bus, int devfn, int addr, unsigned int val)
{
	return bus->ops->write(bus, devfn, addr, 1, val);
}

unsigned int pci_write_config_word(struct pci_bus *bus, int devfn, int addr, unsigned int val)
{
	return bus->ops->write(bus, devfn, addr, 2, val);
}

unsigned int pci_write_config_dword(struct pci_bus *bus, int devfn, int addr, unsigned int val)
{
	return bus->ops->write(bus, devfn, addr, 4, val);
}

void pci_get_config(struct pci_device *dev, char *out)
{
	int i;

	for (i = 0; i < 256; i++)
		*out++ = pci_read_config_byte(dev->bus, dev->devfn, i);
}

unsigned int pci_find_capability(struct pci_device *dev, int cap)
{
	unsigned char pos;
	unsigned char cap_id;

	pos = pci_read_config_byte(dev->bus, dev->devfn, PCI_CAPABILITY_START);
	while (pos) {
		cap_id = pci_read_config_byte(dev->bus, dev->devfn, pos);
		if (cap_id == cap)
			return pos;
		pos = pci_read_config_byte(dev->bus, dev->devfn, pos + 1);
	}

	return pos;
}

static void pci_set_device_msi_domain(struct pci_device *dev)
{
	struct irq_domain *domain = find_irq_domain("IMSIC");

	if (!domain)
		return;

	dev->dev.irq_domain = domain;

	__SPINLOCK_INIT(&dev->msi_lock);
}

static void pci_init_capabilities(struct pci_device *dev)
{
	pci_msi_init(dev);
	pci_msix_init(dev);
}

static unsigned int pci_read_dev_vendor_id(struct pci_bus *bus, int devfn)
{
	return pci_read_config_dword(bus, devfn, PCI_VENDOR_ID);
}

static char pci_read_hdr_type(struct pci_bus *bus, int devfn)
{
	return pci_read_config_byte(bus, devfn, PCI_HEAD_TYPE);
}

static int decode_bar(struct bar *b, unsigned int val)
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
	default:
		return -1;
	}

	b->mem_type = mem_type;
	b->addr_type = addr_type;

	return 0;
}

static int __pci_read_bar(struct pci_device *dev, unsigned int reg, struct bar *b)
{
	unsigned int l = 0, sz = 0, mask;
	unsigned long l64, sz64, mask64;
	unsigned int size = 0;
	int ret = 0;

	mask = ~0;

	l = pci_read_config_dword(dev->bus, dev->devfn, reg);
	pci_write_config_dword(dev->bus, dev->devfn, reg, l | mask);
	sz = pci_read_config_dword(dev->bus, dev->devfn, reg);
	pci_write_config_dword(dev->bus, dev->devfn, reg, l);

	decode_bar(b, l);
	if (b->mem_type == pci_mem_type_io) {
		return 0;
	} else {
		l64 = l & PCI_BASE_ADDRESS_MEM_MASK;
		sz64 = sz & PCI_BASE_ADDRESS_MEM_MASK;
		mask64 = (unsigned int)PCI_BASE_ADDRESS_MEM_MASK;
	}

	if (b->addr_type == pci_addr_type_mem_64) {
		l = pci_read_config_dword(dev->bus, dev->devfn, reg + 4);
		pci_write_config_dword(dev->bus, dev->devfn, reg + 4, ~0);
		sz = pci_read_config_dword(dev->bus, dev->devfn, reg + 4);
		pci_write_config_dword(dev->bus, dev->devfn, reg + 4, l);

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

static int pci_read_base_address(struct pci_device *dev, int nr)
{
	int pos;
	unsigned int reg;

	for (pos = 0; pos < nr; pos++) {
		reg = PCI_BASE_ADDRESS_0 + (pos << 2);
		pos += __pci_read_bar(dev, reg, &dev->bar[pos]);	
	}

	return 0;
}

static struct pci_device *pci_setup_device(
		struct pci_bus *bus, int devfn, unsigned int vendor_id, int *is_multifun)
{
	struct pci_device *dev;

	dev = (struct pci_device *)mm_alloc(sizeof(struct pci_device));
	if (!dev) {
		print("pci: pci alloc device failed!! devfn:%d\n", devfn);
		return NULL;
	}

	dev->bus = bus;
	dev->devfn = devfn;
	dev->vendor = vendor_id & 0xffff;
	dev->device = (vendor_id >> 16) & 0xffff;

	list_add_tail(&dev->list, &bus->devices);

	return dev;
}

static void pci_setup_bus_base_limit(struct pci_bus *bus, unsigned int base, unsigned int limit)
{
	unsigned int l;

	if (!bus->parent)
		return;

	l = (base >> 16) & 0xfff0;
	l |= limit & 0xfff00000;

	print("pci: bus%d -- devfn:%d base:0x%x limit:0x%x base-limit:0x%x\n",
		bus->parent->bus_number, bus->devfn, base, limit, l);
	pci_write_config_word(bus->parent, bus->devfn, PCI_MEMORY_BASE, l);
}

static struct pci_bus *pci_setup_bridge(struct pci_bus *bus, int devfn, unsigned int vendor_id)
{
	struct pci_bus *child;
	struct pci_bus *root_bus = bus;

	child = (struct pci_bus *)mm_alloc(sizeof(struct pci_bus));
	if (!child) {
		print("pci: pci alloc bus failed!!\n");
		return NULL;
	}
	INIT_LIST_HEAD(&child->devices);
	INIT_LIST_HEAD(&child->child_buses);
	INIT_LIST_HEAD(&child->res_used);
	child->parent = bus;
	child->devfn = devfn;
	child->vendor = vendor_id & 0xffff;
	child->device = (vendor_id >> 16) & 0xffff;

	while (root_bus->parent)
		root_bus = root_bus->parent;

	child->ops = root_bus->ops;

	list_add_tail(&child->list, &bus->child_buses);

	return child;
}

static int pci_scan_slot(struct pci_bus *bus, int devfn)
{
	unsigned int vendor_id = 0;
	char hdr_type;
	int fn;
	int is_multifun;
	struct pci_device *dev;
	struct pci_bus *child;
	unsigned int cmd = 0;

	for (fn = devfn; fn < devfn + 8; fn++) {
		is_multifun = 0;
		vendor_id = pci_read_dev_vendor_id(bus, fn);
		if (vendor_id == 0xffffffff || vendor_id == 0)
			return -1;
		
		cmd = pci_read_config_word(bus, fn, PCI_COMMAND);
		if (cmd & (PCI_COMMAND_IO | PCI_COMMAND_MEMORY)) {
			cmd &= ~PCI_COMMAND_IO;
			cmd &= ~PCI_COMMAND_MEMORY;
			pci_write_config_word(bus, fn, PCI_COMMAND, cmd);
		}

		hdr_type = pci_read_hdr_type(bus, fn);
		if (hdr_type == PCI_HEADER_TYPE_NORMAL) {
			dev = pci_setup_device(bus, fn, vendor_id, &is_multifun);
			dev->class = pci_read_config_dword(bus, fn, PCI_CLASS_REVISION) >> 8;

			pci_read_base_address(dev, 6);

			print("pci: 0:%x:%x:%x: [%x:%x] type %x class %x\n", 
			       bus->bus_number, PCI_SLOT(fn),
			       PCI_FUNC(fn), dev->vendor,
			       dev->device, hdr_type, dev->class);

		}
		else if (hdr_type == PCI_HEADER_TYPE_BRIDGE) {
			child = pci_setup_bridge(bus, fn, vendor_id);
			child->class = pci_read_config_dword(bus, fn, PCI_CLASS_REVISION) >> 8;
			print("pci: 0:%x:%x:%x: [%x:%x] type %x class %x\n", 
			       bus->bus_number, PCI_SLOT(fn),
			       PCI_FUNC(fn), child->vendor,
			       child->device, hdr_type, child->class);
		}
		else if (hdr_type == PCI_HEADER_TYPE_CARDBUS) {
		
		}

		if (!is_multifun)
			break;
	}

	return 0;
}

static int __pci_create_device(struct pci_device *pci_dev)
{
	struct device *dev;
	struct pci_bus *bus = pci_dev->bus;

	dev = &pci_dev->dev;
	sprintf(dev->name, "0:%x:%x:%x",
		bus->bus_number, PCI_SLOT(pci_dev->devfn), PCI_FUNC(pci_dev->devfn));
	sprintf(dev->compatible, "%x:%x", pci_dev->vendor, pci_dev->device);

	pci_device_init(pci_dev);

	pci_init_capabilities(pci_dev);

	pci_set_device_msi_domain(pci_dev);

	pci_register_device(pci_dev);

	pci_set_device_iommu(pci_dev);

	return 0;
}

static int pci_create_device(struct pci_bus *bus)
{
	struct pci_bus *child;
	struct pci_device *pci_dev;

	pci_for_each_device(pci_dev, &bus->devices)
		__pci_create_device(pci_dev);

	for_each_child_bus(child, &bus->child_buses)
		pci_create_device(child);

	return 0;
}

static int pci_dev_assign_resources(struct pci_bus *bus)
{
	struct pci_bus *child;
	struct resource *r;
	struct pci_bus *root = bus;

	while (root->parent)
		root = root->parent;

	list_for_each_entry(r, &bus->res_used, list) {
		struct pci_dev_res *dev_res;
		struct pci_device *dev;
		unsigned int reg;
		unsigned long pci_addr, cpu_addr;
		unsigned int val;
		struct bar *b;

		dev_res = container_of(r, struct pci_dev_res, res);
		dev = dev_res->dev;
		if (!dev)
			continue;
		b = &dev->bar[dev_res->bar];

		cpu_addr = r->base;
		pci_addr = cpu_addr - root->offset;
		b->base = cpu_addr;
		b->size = r->end - r->base + 1;

		print("pci: 0:%x:%x:%x: [%x:%x] -- BAR[%d] : 0x%lx - 0x%lx\n", 
		       dev->bus->bus_number, PCI_SLOT(dev->devfn),
		       PCI_FUNC(dev->devfn), dev->vendor,
		       dev->device, dev_res->bar,
		       b->base, b->base + b->size);

		reg = PCI_BASE_ADDRESS_0 + dev_res->bar * sizeof(unsigned int);
		val = pci_read_config_dword(dev->bus, dev->devfn, reg) & (0xFUL);
#ifdef CONFIG_SELECT_QEMU
		val |= cpu_addr & 0xffffffffUL;
#else
		val |= pci_addr & 0xffffffffUL;
#endif
		pci_write_config_dword(dev->bus, dev->devfn, reg, val);
		if ((val & 0xFUL) == PCI_BASE_ADDRESS_MEM_TYPE_64) {
			val = pci_addr >> 32;
			pci_write_config_dword(dev->bus, dev->devfn, reg + 4, val);
		}
	}

	for_each_child_bus(child, &bus->child_buses) {
		pci_dev_assign_resources(child);
	}

	return 0;
}

static int pci_scan_bus(struct pci_bus *bus, int bus_number)
{
	int devfn;
	struct pci_bus *child;
	int bus_end;

	for (devfn = 0; devfn < 256; devfn += 8)
		pci_scan_slot(bus, devfn);

	bus_end = bus_number;
	for_each_child_bus(child, &bus->child_buses) {
		int bctl = 0;
		unsigned int buses = 0;

		bctl = pci_read_config_word(bus, child->devfn, PCI_BRIDGE_CONTROL);
		pci_write_config_word(bus, child->devfn, PCI_BRIDGE_CONTROL, bctl & ~PCI_BRIDGE_CTL_MASTER_ABORT);

		pci_write_config_word(bus, child->devfn, PCI_STATUS, 0xffff);

		bus_end += 1;
		child->primary = bus_number;
		child->bus_number = bus_end;
		child->subordinate = 0xFF;

		buses = pci_read_config_dword(bus, child->devfn, PCI_PRIMARY_BUS);
		buses = (buses & 0xff000000)
			| ((unsigned long)(child->primary) << 0)
			| ((unsigned long)(child->bus_number) << 8)
			| ((unsigned long)(child->subordinate) << 16);
		pci_write_config_dword(bus, child->devfn, PCI_PRIMARY_BUS, buses);

		bus_end = pci_scan_bus(child, bus_end);

		child->subordinate = bus_end;
		pci_write_config_byte(bus, child->devfn, PCI_SUBORDINATE_BUS, child->subordinate);
		print("pci: bus:%d primary:%d subordinate:%d\n", child->bus_number, child->primary, child->subordinate);
	}	

	return bus_end;
}

static int __pci_assign_resource(struct pci_bus *root, struct pci_bus *bus)
{
	unsigned long start, end;
	struct resource *r;
	struct pci_bus *child;
	unsigned long base = -1UL, limit = 0;
	struct resource *allocated;

	start = root->res.base;
	end = root->res.end;

	list_for_each_entry(allocated, &root->res_allocated, list) {
		if (allocated->end > start)
			start = allocated->end + 1;
	}
	start = ALIGN_SIZE(start, 1*1024*1024);

	list_for_each_entry(r, &bus->res_used, list) {
		int size = r->end - r->base + 1;
		struct resource *new;

		r->base = ALIGN_SIZE(start, size);
		r->end = r->base + size - 1;

		if (r->end > end) {
			print("pci: %s -- alloc resource failed\n", __FUNCTION__);
			return -1;
		}

		if (base > r->base)
			base = r->base;
		if (limit < r->end)
			limit = r->end;

		new = (struct resource *)mm_alloc(sizeof(struct resource));
		new->base = r->base;
		new->end = r->end;
		list_add_tail(&new->list, &root->res_allocated);

		start = r->end + 1;
		//print("%s -- start:0x%lx end:0x%lx\n", __FUNCTION__, r->base, r->end);
	}

	for_each_child_bus(child, &bus->child_buses) {
		if (child->base < base)
			base = child->base;
		if (child->limit > limit)
			limit = child->limit;	
	}

	bus->base = base;
	bus->limit = limit;

	pci_setup_bus_base_limit(bus, bus->base - root->offset, bus->limit - root->offset);

	return 0;
}

static int __pci_bus_assign_resources(struct pci_bus *bus)
{
	struct pci_bus *root = bus;

	while (root->parent)
		root = root->parent;

	return __pci_assign_resource(root, bus);
}

static int pci_bus_assign_resources(struct pci_bus *bus)
{
	struct pci_bus *child;

	for_each_child_bus(child, &bus->child_buses) {
		pci_bus_assign_resources(child);
	}

	return __pci_bus_assign_resources(bus);	
}

static int is_root_bus(struct pci_bus *bus)
{
	return !bus->parent;
}

static int pci_bus_assign_resources_size(struct pci_bus *bus)
{
	struct pci_bus *child;
	struct pci_device *dev;
	unsigned long size = 0;

	for_each_child_bus(child, &bus->child_buses)
		pci_bus_assign_resources_size(child);

	for_each_child_bus(child, &bus->child_buses) {
		struct resource *r;
		list_for_each_entry(r, &child->res_used, list) {
			size += r->end - r->base + 1;
			//size += RESIZE(r->end - r->base + 1, 1*1024*1024);
		}
	}

	pci_for_each_device(dev, &bus->devices) {
		struct resource *r;
		struct pci_dev_res *dev_res;

		print("pci 0:%x:%x:%x: [%x:%x]:\n", 
		       dev->bus->bus_number, PCI_SLOT(dev->devfn),
		       PCI_FUNC(dev->devfn), dev->vendor,
		       dev->device);

		for (int i = 0; i < 6; i++) {
			if (dev->bar[i].size == 0)
				continue;
			print("  BAR[%d] : [0x%lx - 0x%lx]\n", i, dev->bar[i].base, dev->bar[i].base + dev->bar[i].size - 1);
			dev_res = (struct pci_dev_res *)mm_alloc(sizeof(struct pci_dev_res));
			if (!dev_res) {
				print("%s -- alloc pci_dev_res failed...\n");
				return -1;
			}
			dev_res->dev = dev;
			dev_res->bar = i;

			r = &dev_res->res;
			r->base = dev->bar[i].base;
			r->end = dev->bar[i].base + dev->bar[i].size - 1;
			list_add_tail(&r->list, &bus->res_used);
			size += dev->bar[i].size;
		}
	}

	if (!is_root_bus(bus)) {
		bus->res.base = 0;
		bus->res.end = bus->res.base + size - 1;
	}

	return 0;
}

void pci_set_master(struct pci_device *dev, int enable)
{
	int cmd;

	cmd = pci_read_config_word(dev->bus, dev->devfn, PCI_COMMAND);

	if (enable)
		cmd |= PCI_COMMAND_MASTER;

	else
		cmd &= ~PCI_COMMAND_MASTER;

	pci_write_config_word(dev->bus, dev->devfn, PCI_COMMAND, cmd);
}

void pci_enable_resource(struct pci_device *dev, int mask)
{
	int i;
	struct bar *b;
	int cmd, old;

	cmd = pci_read_config_word(dev->bus, dev->devfn, PCI_COMMAND);
	old = cmd;

	for (i = 0; i < 6; i++) {
		if (!(mask & (1 << i)))
			continue;
		b = &dev->bar[i];
		if (b->mem_type == pci_mem_type_io)
			cmd |= PCI_COMMAND_IO;
		if (b->mem_type == pci_mem_type_mem)
			cmd |= PCI_COMMAND_MEMORY;
	}

	if (cmd != old) {
		print("pci: enabling device (0x%x -> 0x%x)\n", old, cmd);
		pci_write_config_word(dev->bus, dev->devfn, PCI_COMMAND, cmd);
	}
}

void pci_get_resource(struct pci_device *dev, int bar, struct resource *res)
{
	struct bar *b;
	struct pci_bus *root = dev->bus;

	while (root->parent)
		root = root->parent;

	b = &dev->bar[bar];

	if (b->size == 0) {
		res->base = 0;
		res->end = 0;
	}
	else {
		res->base = b->base;
		res->end = res->base + b->size - 1;
	}
}

int pci_probe_root_bus(struct pci_bus *bus)
{
	INIT_LIST_HEAD(&bus->devices);
	INIT_LIST_HEAD(&bus->child_buses);
	INIT_LIST_HEAD(&bus->res_used);
	INIT_LIST_HEAD(&bus->res_allocated);
	bus->parent = NULL;

	print("pci: pci probe root bus...\n");

	pci_scan_bus(bus, 0);

	print("pci: pci bus assign resource...\n");
	pci_bus_assign_resources_size(bus);
	pci_bus_assign_resources(bus);

	print("pci: pci device assign resource...\n");
	pci_dev_assign_resources(bus);

	print("pci: pci create device...\n");
	pci_create_device(bus);

	pci_probe_driver();

	return 0;
}

int pci_root_bus_init(struct device *dev, struct pci_bus *bus,
		      struct ecam_ops *ops, void *data,
		      struct resource *res, unsigned long offset)
{
	bus->ops = ops;
	bus->data = data;
	bus->bus_number = 0;
	bus->parent = NULL;
	bus->res.base = res->base + offset;
	bus->res.end = res->end + offset;
	bus->offset = offset;
	bus->dev = dev;

	return 0;
}
