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

#ifndef __PCI_H__
#define __PCI_H__

#include "list.h"
#include "device.h"

#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND   0x04
#define  PCI_COMMAND_IO      0x1
#define  PCI_COMMAND_MEMORY  0x2

#define PCI_STATUS 0x06

#define PCI_CLASS_REVISION 0x08

#define PCI_HEAD_TYPE 0x0e

#define PCI_BASE_ADDRESS_0  0x10
#define PCI_BASE_ADDRESS_1  0x14
#define PCI_BASE_ADDRESS_2  0x18
#define PCI_BASE_ADDRESS_3  0x1c
#define PCI_BASE_ADDRESS_4  0x20
#define PCI_BASE_ADDRESS_5  0x24

#define  PCI_BASE_ADDRESS_SPACE		0x01	/* 0 = memory, 1 = I/O */
#define  PCI_BASE_ADDRESS_SPACE_IO	0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY	0x00
#define  PCI_BASE_ADDRESS_MEM_TYPE_MASK	0x06
#define  PCI_BASE_ADDRESS_MEM_TYPE_32	0x00	/* 32 bit address */
#define  PCI_BASE_ADDRESS_MEM_TYPE_1M	0x02	/* Below 1M [obsolete] */
#define  PCI_BASE_ADDRESS_MEM_TYPE_64	0x04	/* 64 bit address */
#define  PCI_BASE_ADDRESS_MEM_PREFETCH	0x08	/* prefetchable? */
#define  PCI_BASE_ADDRESS_MEM_MASK	(~0x0fUL)
#define  PCI_BASE_ADDRESS_IO_MASK	(~0x03UL)

#define PCI_PRIMARY_BUS     0x18
#define PCI_SECONDARY_BUS   0x19
#define PCI_SUBORDINATE_BUS 0x1a

#define PCI_BRIDGE_CONTROL 0x3e
#define PCI_BRIDGE_CTL_MASTER_ABORT 0x20

#define PCI_HEADER_TYPE_NORMAL  0
#define PCI_HEADER_TYPE_BRIDGE  1
#define PCI_HEADER_TYPE_CARDBUS 2

#define PCI_ECAM_BUS_SHIFT	20 /* Bus number */
#define PCI_ECAM_DEVFN_SHIFT	12 /* Device and Function number */

#define PCI_ECAM_BUS_MASK	0xff
#define PCI_ECAM_DEVFN_MASK	0xff
#define PCI_ECAM_REG_MASK	0xfff /* Limit offset to a maximum of 4K */

#define PCI_ECAM_BUS(x)	(((x) & PCI_ECAM_BUS_MASK) << PCI_ECAM_BUS_SHIFT)
#define PCI_ECAM_DEVFN(x)	(((x) & PCI_ECAM_DEVFN_MASK) << PCI_ECAM_DEVFN_SHIFT)
#define PCI_ECAM_REG(x)	((x) & PCI_ECAM_REG_MASK)

#define PCI_ECAM_OFFSET(bus, devfn, where) \
	(PCI_ECAM_BUS(bus) | \
	 PCI_ECAM_DEVFN(devfn) | \
	 PCI_ECAM_REG(where))

#define PCI_SLOT(devfn)         (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn)         ((devfn) & 0x07)

enum {
	pci_mem_type_io = 0,
	pci_mem_type_mem,
	pci_mem_type_pref_mem,
};

enum {
	pci_addr_type_mem_32 = 0,
	pci_addr_type_mem_1M,
	pci_addr_type_mem_64,
};

struct pci_priv_data {
	unsigned long pci_addr;
	unsigned long size;
	unsigned long offset;
};

struct pci_bus;

struct resource {
	struct list_head list;
	unsigned long base;
	unsigned int end;
};

struct bar {
	int mem_type;
	int addr_type;
	unsigned long base;
	unsigned int size;
};

struct pci_device {
	struct list_head list;
	struct pci_bus *bus;
	unsigned int devfn;
	unsigned int vendor;
	unsigned int device;
	unsigned int class;
	struct device dev;
	struct bar bar[6];
};

struct pci_dev_res {
	struct resource res;
	struct pci_device *dev;
	int bar;
};

struct ecam_ops {
	void* (*map)(struct pci_bus *bus, int devfn, int addr);
	unsigned int (*read)(struct pci_bus *bus, int devfn, int addr, int size);
	int (*write)(struct pci_bus *bus, int devfn, int addr, int size, unsigned int val);
};

struct pci_bus {
	struct list_head list;
	int devfn;
	int bus_number;
	int subordinate;
	int primary;
	struct list_head devices;
	struct list_head child_buses;
	struct pci_bus *parent;
	struct ecam_ops *ops;
	void *data;
	unsigned int vendor;
	unsigned int device;
	unsigned int class;
	struct resource res;
	struct list_head res_used;
	unsigned long offset;
	int base;
	int limit;
};

void pci_enable_resource(struct pci_device *dev, int mask);
void pci_get_resource(struct pci_device *dev, int bar, struct resource *res);
int pci_root_bus_init(struct pci_bus *bus, struct ecam_ops *ops, void *data,
		      struct resource *res, unsigned long offset);
int pci_probe_root_bus(struct pci_bus *bus);
unsigned int pci_read_config_dword(struct pci_bus *bus, int devfn, int addr);
unsigned int pci_read_config_word(struct pci_bus *bus, int devfn, int addr);
unsigned int pci_read_config_byte(struct pci_bus *bus, int devfn, int addr);

#define for_each_child_bus(bus, head) \
	list_for_each_entry(bus, head, list)

#define pci_for_each_device(dev, head) \
	list_for_each_entry(dev, head, list)

#endif
