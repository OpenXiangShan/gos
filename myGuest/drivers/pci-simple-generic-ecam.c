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

#include "device.h"
#include "asm/mmio.h"
#include "asm/type.h"
#include "vmap.h"
#include "print.h"
#include "pci_simple.h"

struct pci_generic_priv_data {
	unsigned long pci_addr;
	unsigned long size;
	unsigned long offset;
};

int pci_simple_generic_ecam_init(unsigned long base, unsigned int len, void *data)
{
	unsigned long addr;
	struct pci_generic_priv_data *priv = (struct pci_generic_priv_data *)data;

	print("%s base:0x%lx data:0x%lx -- pci_addr:0x%lx cpu_addr:0x%lx size:0x%lx\n",
	      __FUNCTION__, base, priv, priv->pci_addr, priv->pci_addr + priv->offset, priv->size);

	addr = (unsigned long)ioremap((void *)base, len, NULL);

	pci_simple_register_root_bus(addr, priv->pci_addr + priv->offset, priv->size);
	pci_simple_probe_root_bus();

	return 0;
}

DRIVER_REGISTER(pci_simple_generic_ecam, pci_simple_generic_ecam_init, "pci-generic-ecam");
