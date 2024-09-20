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
#include "pci.h"
#include "vmap.h"
#include "irq.h"
#include "mm.h"
#include "list.h"
#include "pci_device_driver.h"

static LIST_HEAD(msi_descs);

static struct msi_desc *get_msi_desc(struct irq_domain *d, int irq)
{
	struct msi_desc *desc;

	list_for_each_entry(desc, &msi_descs, list) {
		if (desc->domain == d && desc->hwirq == irq)
			return desc;
	}

	desc = (struct msi_desc *)mm_alloc(sizeof(struct msi_desc));
	if (!desc)
		return NULL;

	desc->hwirq = irq;
	desc->domain = d;
	list_add_tail(&desc->list, &msi_descs);

	return desc;
}

static int pci_msix_get_vec_count(struct pci_device *pdev)
{
	unsigned short msg_ctrl;

	if (!pdev->msix_cap_pos)
		return 0;

	msg_ctrl = pci_read_config_word(pdev->bus, pdev->devfn,
			pdev->msix_cap_pos + PCI_MSIX_FLAGS);

	return ((msg_ctrl & PCI_MSIX_FLAGS_QSIZE) + 1);
}

static unsigned long pci_msix_map(struct pci_device *pdev, int count)
{
	unsigned int msix_table_offset;
	int bar;
	unsigned long addr;
	struct resource res;

	msix_table_offset = pci_read_config_dword(pdev->bus, pdev->devfn,
					pdev->msix_cap_pos + PCI_MSIX_TABLE);
	bar = msix_table_offset & PCI_MSIX_TABLE_BIR;
	msix_table_offset &= PCI_MSIX_TABLE_OFFSET;

	pci_get_resource(pdev, bar, &res);

	addr = (unsigned long)ioremap((void *)(res.base + msix_table_offset),
				      count * PCI_MSIX_ENTRY_SIZE, NULL);

	return addr;
}

static void pci_msix_write_msi_msg(struct device *dev, unsigned long msi_addr,
				   unsigned long msi_data, int hwirq, void *priv)
{
	unsigned int msi_addr_lo, msi_addr_hi;
	void *addr;
	struct msi_desc *desc = get_msi_desc(dev->irq_domain, hwirq);
	unsigned int vector_ctrl;

	if (!desc)
		return;

	addr = desc->base + desc->entry_index * PCI_MSIX_ENTRY_SIZE;

	msi_addr_lo = (unsigned int)(msi_addr & 0xFFFFFFFF);
	msi_addr_hi = (unsigned int)(msi_addr >> 32);

	vector_ctrl = readl(addr + PCI_MSIX_ENTRY_VECTOR_CTRL);
	if (!(vector_ctrl & PCI_MSIX_ENTRY_CTRL_MASKBIT))
		writel(addr + PCI_MSIX_ENTRY_VECTOR_CTRL,
		       vector_ctrl | PCI_MSIX_ENTRY_CTRL_MASKBIT);

	writel(addr + PCI_MSIX_ENTRY_LOWER_ADDR, msi_addr_lo);
	writel(addr + PCI_MSIX_ENTRY_UPPER_ADDR, msi_addr_hi);
	writel(addr + PCI_MSIX_ENTRY_DATA, msi_data);

	writel(addr + PCI_MSIX_ENTRY_VECTOR_CTRL, vector_ctrl & (~PCI_MSIX_ENTRY_CTRL_MASKBIT));
}

void pci_msi_init(struct pci_device *pdev)
{
	pdev->msix_cap_pos = pci_find_capability(pdev, PCI_CAP_ID_MSI);
}

void pci_msix_init(struct pci_device *pdev)
{
	pdev->msix_cap_pos = pci_find_capability(pdev, PCI_CAP_ID_MSIX);
}

static void pci_msix_set_desc(struct device *dev, int hwirq, int nr)
{
	int i;
	struct msi_desc *desc;
	struct pci_device *pdev = to_pci_dev(dev);

	for (i = 0; i < nr; i++) {
		desc = get_msi_desc(pdev->dev.irq_domain, hwirq + i);
		desc->base = (void *)pdev->msix_base;
		desc->entry_index = i;
	}
}

int pci_msix_enable(struct pci_device *pdev, int *irqs)
{
	int count;
	unsigned short msg_ctrl;
	int irq_base, i;

	if (!pdev->dev.irq_domain)
		return 0;

	count = pci_msix_get_vec_count(pdev);
	if (count == 0)
		return 0;
	pdev->msix_base = pci_msix_map(pdev, count);

	irq_base = msi_get_hwirq(&pdev->dev, count, pci_msix_write_msi_msg, pci_msix_set_desc);
	for (i = 0; i < count; i++)
		irqs[i] = irq_base + i;	

	msg_ctrl = pci_read_config_word(pdev->bus, pdev->devfn,
				pdev->msix_cap_pos + PCI_MSIX_FLAGS);
	msg_ctrl |= PCI_MSIX_FLAGS_ENABLE;
	pci_write_config_word(pdev->bus, pdev->devfn,
				pdev->msix_cap_pos + PCI_MSIX_FLAGS, msg_ctrl);	

	return count;
}
