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
#include "vmap.h"

#define XILINX_PCIE_DMA_REG_IDR			0x00000138
#define XILINX_PCIE_DMA_REG_IMR			0x0000013c
#define XILINX_PCIE_DMA_REG_PSCR		0x00000144
#define XILINX_PCIE_DMA_REG_RPSC		0x00000148
#define XILINX_PCIE_DMA_REG_MSIBASE1		0x0000014c
#define XILINX_PCIE_DMA_REG_MSIBASE2		0x00000150
#define XILINX_PCIE_DMA_REG_RPEFR		0x00000154
#define XILINX_PCIE_DMA_REG_IDRN		0x00000160
#define XILINX_PCIE_DMA_REG_IDRN_MASK		0x00000164
#define XILINX_PCIE_DMA_REG_MSI_LOW		0x00000170
#define XILINX_PCIE_DMA_REG_MSI_HI		0x00000174
#define XILINX_PCIE_DMA_REG_MSI_LOW_MASK	0x00000178
#define XILINX_PCIE_DMA_REG_MSI_HI_MASK		0x0000017c

#define XILINX_PCIE_DMA_REG_PSCR_LNKUP	(1UL << 11)
#define XILINX_PCIE_DMA_REG_RPSC_BEN	(1UL << 0)

#define XILINX_PCIE_DMA_IMR_ALL_MASK	0x0ff30fe9
#define XILINX_PCIE_DMA_IDR_ALL_MASK	0xffffffff
#define XILINX_PCIE_DMA_IDRN_MASK	GENMASK(19, 16)

struct pci_xdma_bus {
	struct pci_bus bus;
	unsigned long base;
};

static struct pci_xdma_bus *xdma_bus;

static void *pci_host_generic_ecam_map(struct pci_bus *bus, int devfn, int addr)
{
	struct pci_xdma_bus *g_bus = xdma_bus;
	unsigned long base;

	if (!g_bus)
		return NULL;

	base = g_bus->base;

	return (void *)(base + PCI_ECAM_OFFSET(bus->bus_number, devfn, addr));
}

static unsigned int pci_host_generic_ecam_config_read(struct pci_bus *bus,
				int devfn, int addr, int size)
{
	void *ecam_addr;
	unsigned int ret = 0;

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

static int xdma_is_link_up(void)
{
	unsigned int val;

	val = readl(xdma_bus->base + XILINX_PCIE_DMA_REG_PSCR);

	return (val & XILINX_PCIE_DMA_REG_PSCR_LNKUP) ? 1 : 0;
}

static void xdma_init_port(void)
{
	if (xdma_is_link_up())
		print("PCIe Link is UP\n");
	else
		print("PCIe Link is DOWN\n");

	writel(xdma_bus->base + XILINX_PCIE_DMA_REG_IMR,
		~XILINX_PCIE_DMA_IDR_ALL_MASK);

	writel(xdma_bus->base + XILINX_PCIE_DMA_REG_IDR,
		readl(xdma_bus->base + XILINX_PCIE_DMA_REG_IDR)
			& XILINX_PCIE_DMA_IMR_ALL_MASK);
	writel(xdma_bus->base + XILINX_PCIE_DMA_REG_RPSC,
		readl(xdma_bus->base + XILINX_PCIE_DMA_REG_RPSC)
			| XILINX_PCIE_DMA_REG_RPSC_BEN);
}

static struct ecam_ops xdma_config_ops = {
	.map = pci_host_generic_ecam_map,
	.read = pci_host_generic_ecam_config_read,
	.write = pci_host_generic_ecam_config_write,
};

int pci_host_xdma_init(struct device *dev, void *data)
{
	struct pci_bus *bus;
	struct resource res;
	struct pci_priv_data *priv = (struct pci_priv_data *)data;

	print("pci-host-xdma-init: base:0x%lx len:0x%lx cpu_addr:0x%lx pci_addr:0x%lx\n",
	      dev->base, dev->len, priv->pci_addr + priv->offset, priv->pci_addr);

	xdma_bus = (struct pci_xdma_bus *)mm_alloc(sizeof(struct pci_xdma_bus));
	if (!xdma_bus) {
		print("ecam-generic-pci-host: alloc generic pci bus fauiled!!\n");
		return -1;
	}
	xdma_bus->base = (unsigned long)ioremap_2M((void *)dev->base, dev->len, 0);

	res.base = priv->pci_addr;
	res.end = res.base + priv->size - 1;

	xdma_init_port();

	bus = &xdma_bus->bus;
	pci_root_bus_init(dev, bus, &xdma_config_ops, xdma_bus, &res, priv->offset);
	pci_probe_root_bus(bus);

	return 0;
}
DRIVER_REGISTER(pci_host_xdma, pci_host_xdma_init, "xilinx,xdma");
