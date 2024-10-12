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
#include "machine.h"
#include "virt.h"
#include "print.h"
#include "mm.h"
#include "string.h"
#include "pci_generic_emulator.h"
#include "../bsp/pci_data.h"

static void
pci_generic_write_device_cfg(struct pci_device_emulator *dev,
			     union pci_config_addr cfg,
			     unsigned long data, int len)
{
	struct pci_device_function_emulator *fun;

	fun = dev->function[cfg.fun_number];
	if (!fun) {
		//print("warning -- pci-generic-emu: fun is NULL(%x:%x:%x)\n",
		//      cfg.bus_number, cfg.device_number, cfg.fun_number);
		return;
	}

	if (fun->ops->config_write)
		fun->ops->config_write(fun, cfg.reg_number, data, len);
	else {
		if (len == 1)
			*(fun->cfg_space.data + cfg.reg_number) = (unsigned char)data;
		else if (len == 2)
			*((unsigned short *)(fun->cfg_space.data + cfg.reg_number)) = (unsigned short)data;
		else if (len == 4)
			*((unsigned int *)(fun->cfg_space.data + cfg.reg_number)) = (unsigned int)data;
		else if (len == 8)
			*((unsigned long *)(fun->cfg_space.data + cfg.reg_number)) = data;
	}
}

static unsigned long
pci_generic_read_device_cfg(struct pci_device_emulator * dev,
			    union pci_config_addr cfg, int len)
{
	struct pci_device_function_emulator *fun;
	unsigned long data = 0;

	fun = dev->function[cfg.fun_number];
	if (!fun) {
		//print("warning -- pci-generic-emu: fun is NULL(%x:%x:%x)\n",
		//      cfg.bus_number, cfg.device_number, cfg.fun_number);
		return -1;
	}

	if (fun->ops->config_read)
		data = fun->ops->config_read(fun, cfg.reg_number, len);
	else {
		if (len == 1)
			data = *(fun->cfg_space.data + cfg.reg_number);
		else if (len == 2)
			data = *((unsigned short *)(fun->cfg_space.data + cfg.reg_number));
		else if (len == 4)
			data = *((unsigned int *)(fun->cfg_space.data + cfg.reg_number));
		else if (len == 8)
			data = *((unsigned long *)(fun->cfg_space.data + cfg.reg_number));
	}

	return data;
}

static struct pci_device_emulator *
pci_generic_get_device_config(struct pci_generic_emulator *pci_emulator,
			      union pci_config_addr cfg)
{
	return pci_emulator->slot[cfg.device_number];
}

static void pci_generic_config_mmio_write(struct memory_region *region,
					  unsigned long addr, unsigned long val,
					  unsigned int len)
{
	union pci_config_addr cfg;
	struct pci_device_emulator *devfn;
	struct pci_generic_emulator *pci_emulator = region->machine->pci_emu;

	cfg.addr = addr - region->start;
	devfn = pci_generic_get_device_config(pci_emulator, cfg);
	if (!devfn) {
		//print("warning -- pci-generic-emu: device is NULL(%x:%x:%x)\n",
		//      cfg.bus_number, cfg.device_number, cfg.fun_number);
		return;
	}

	pci_generic_write_device_cfg(devfn, cfg, val, len);
}

static unsigned long pci_generic_config_mmio_read(struct memory_region *region,
						  unsigned long addr, unsigned int len)
{
	union pci_config_addr cfg;
	struct pci_device_emulator *devfn;
	struct pci_generic_emulator *pci_emulator = region->machine->pci_emu;

	cfg.addr = addr - region->start;
	devfn = pci_generic_get_device_config(pci_emulator, cfg);
	if (!devfn) {
		//print("warning -- pci-generic-emu: device is NULL(%x:%x:%x)\n",
		//      cfg.bus_number, cfg.device_number, cfg.fun_number);
		return -1;
	}

	return pci_generic_read_device_cfg(devfn, cfg, len);
}

static const struct memory_region_ops pci_generic_config_mmio_ops = {
	.write = pci_generic_config_mmio_write,
	.read = pci_generic_config_mmio_read,
};

static int create_pci_generic_device_config(struct virt_machine *machine)
{
	unsigned long base = get_machine_memmap_base(VIRT_PCI_CONFIG);
	int size = get_machine_memmap_size(VIRT_PCI_CONFIG);

	add_memory_region(machine,
			  VIRT_PCI_CONFIG,
			  base, size,
			  &pci_generic_config_mmio_ops);
	return 0;
}

static struct pci_device_function_emulator *
__find_contained_pci_device_func(struct pci_device_emulator *bus,
				 unsigned long addr)
{
	return NULL;
}

static struct pci_device_function_emulator *
find_contained_pci_device_func(struct pci_generic_emulator *root,
			       unsigned long addr)
{
	int i, j;
	struct pci_device_emulator *pci_dev;
	struct pci_device_function_emulator *fn = NULL;

	for (i = 0; i < 32; i++) {
		pci_dev = root->slot[i];
		if (!pci_dev)
			continue;
		if (pci_dev->type == 0x1)
			fn = __find_contained_pci_device_func(pci_dev, addr);
	}
	if (!fn) {
		for (i = 0; i < 32; i++) {
			struct pci_device_function_emulator *tmp;
			pci_dev = root->slot[i];
			if (!pci_dev)
				continue;
			if (pci_dev->type == 0x0) {
				for (j = 0; j < 8; j++) {
					tmp = pci_dev->function[j];
					if (!tmp)
						continue;
					if (!tmp->ops || !tmp->ops->contained_pci_address)
						continue;
					tmp->ops->contained_pci_address(tmp, addr);
				}
			}
		}
	}

	return fn;
}

static void pci_generic_mmio_write(struct memory_region *region,
				   unsigned long addr,
				   unsigned long val,
				   unsigned int len)
{
	unsigned int offset;
	unsigned pci_addr;
	struct pci_generic_emulator *pci_emulator = region->machine->pci_emu;

	print("%s -- Do not process...\n", __FUNCTION__);

	if (!pci_emulator)
		return;

	offset = pci_emulator->cpu_addr - pci_emulator->pci_addr;
	pci_addr = addr - offset;

	find_contained_pci_device_func(pci_emulator, pci_addr);
}

static unsigned long pci_generic_mmio_read(struct memory_region *region,
					   unsigned long addr,
					   unsigned int len)
{
	struct pci_generic_emulator *pci_emulator = region->machine->pci_emu;

	print("%s -- Do not process...\n", __FUNCTION__);

	if (!pci_emulator)
		return 0;

	return 0;
}

static const struct memory_region_ops pci_generic_mmio_ops = {
	.write = pci_generic_mmio_write,
	.read = pci_generic_mmio_read,
};

static int create_pci_generic_mmio(struct virt_machine *machine)
{
	unsigned long base = get_machine_memmap_base(VIRT_PCI_MMIO);
	unsigned int size = get_machine_memmap_size(VIRT_PCI_MMIO);

	return add_memory_region(machine,
				 VIRT_PCI_MMIO,
				 base, size,
				 &pci_generic_mmio_ops);
}

int create_pci_generic_device(struct virt_machine *machine)
{
	unsigned long mmio = get_machine_memmap_base(VIRT_PCI_MMIO);
	unsigned int size = get_machine_memmap_size(VIRT_PCI_MMIO);
	struct pci_generic_emulator *pci_emulator;

	pci_emulator = (struct pci_generic_emulator *)mm_alloc(sizeof(struct pci_generic_emulator));
	if (!pci_emulator) {
		print("pci-generic_emu: alloc pci_generic_emulator fail\n");
		return -1;
	}
	memset((char *)pci_emulator, 0, sizeof(struct pci_generic_emulator));

	pci_emulator->pci_addr = mmio;
	pci_emulator->cpu_addr = mmio;
	pci_emulator->pci_mmio_size = size;
	machine->pci_emu = pci_emulator;

	create_pci_generic_device_config(machine);
	create_pci_generic_mmio(machine);

	return 0;
}

int create_pci_generic_priv_data(struct virt_machine *machine, void *ptr)
{
	struct pci_data data;
	int len = sizeof(struct pci_data);

	if (!ptr)
		return 0;

	if (!machine->pci_emu)
		return 0;

	data.pci_addr = machine->pci_emu->pci_addr;
	data.size = machine->pci_emu->pci_mmio_size;
	data.offset = machine->pci_emu->cpu_addr - machine->pci_emu->pci_addr;

	memcpy((char *)ptr, (char *)&data, len);

	return len;
}
