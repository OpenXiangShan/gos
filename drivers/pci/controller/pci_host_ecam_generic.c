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

#include "irq.h"
#include "device.h"
#include "string.h"
#include "print.h"
#include "asm/type.h"
#include "asm/mmio.h"
#include "pci.h"
#include "mm.h"
#include "pci_host_ecam_generic.h"

static struct pci_ecam_generic_bus *generic_bus;

static void *pci_host_generic_ecam_map(struct pci_bus *bus, int devfn, int addr)
{
	struct pci_ecam_generic_bus *g_bus = generic_bus;
	unsigned long base;

	if (!g_bus)
		return NULL;

	base = g_bus->base;

	//print("%s -- busn:%d devfn:%d where:%d addr:0x%lx\n", __FUNCTION__, bus->bus_number, devfn, addr, PCI_ECAM_OFFSET(bus->bus_number, devfn, addr) + 0x30000000);
	return (void *)(base + PCI_ECAM_OFFSET(bus->bus_number, devfn, addr));
}

static unsigned int pci_host_generic_ecam_config_read(struct pci_bus *bus,
				int devfn, int addr, int size)
{
	void *ecam_addr;
	unsigned int ret;

	ecam_addr = bus->ops->map(bus, devfn, addr);
	if (!ecam_addr) {
		print("pci bus map failed!!\n");
		return -1;
	}

	if (size == 1)
		ret = readb(ecam_addr);
	else if (size == 2)
		ret = readb(ecam_addr) + (((unsigned int)readb(ecam_addr + 1)) << 8);
	else if (size == 4)
		ret = readl(ecam_addr);

	return ret;
}

static int pci_host_generic_ecam_config_write(
			struct pci_bus *bus, int devfn, int addr, int size, unsigned int val)
{
	void *ecam_addr;

	//print("+++++ busn:%d devfn:%d where:%d val:0x%lx\n", bus->bus_number, devfn, addr, val);
	ecam_addr = bus->ops->map(bus, devfn, addr);
	if (!ecam_addr) {
		print("pci bus map failed!!\n");
		return -1;
	}

	if (size == 1)
		writeb(ecam_addr, val);
	else if (size == 2) {
		writeb(ecam_addr, val & 0xff);
		writeb(ecam_addr + 1, (val >> 8) & 0xff);
	}
	else if (size == 4)
		writel(ecam_addr, val);

	return 0;
}

static struct ecam_ops generic_ecam_ops = {
	.map = pci_host_generic_ecam_map,
	.read = pci_host_generic_ecam_config_read,
	.write = pci_host_generic_ecam_config_write,
}; 

int pci_host_ecam_generic_init(struct device *dev, void *data)
{
	struct pci_bus *bus;
	struct resource res;
	struct pci_priv_data *priv = (struct pci_priv_data *)data;

	print("%s -- base:0x%lx len:0x%lx cpu_addr:0x%lx pci_addr:0x%lx\n",
		__FUNCTION__, dev->start, dev->len, priv->pci_addr + priv->offset, priv->pci_addr);

	generic_bus = (struct pci_ecam_generic_bus *)mm_alloc(sizeof(struct pci_ecam_generic_bus));
	if (!generic_bus) {
		print("%s -- alloc generic pci bus fauiled!!\n", __FUNCTION__);
		return -1;
	}
	generic_bus->base = dev->base;

	res.base = priv->pci_addr;
	res.end = res.base + priv->size - 1;

	bus = &generic_bus->bus;
	pci_root_bus_init(bus, &generic_ecam_ops, generic_bus, &res, priv->offset);
	pci_probe_root_bus(bus);

	return 0;
}
DRIVER_REGISTER(pci_host_ecam_generic, pci_host_ecam_generic_init, "pci-host-ecam-generic");
