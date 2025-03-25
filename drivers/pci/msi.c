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
#include "asm/bitops.h"
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
	desc->mask = 0;
	desc->is_msix = 0;
	list_add_tail(&desc->list, &msi_descs);

	return desc;
}

static void __pci_msi_mask(struct pci_device *pdev, struct msi_desc *desc,
			   unsigned int clear, unsigned int set)
{
	int flags;

	if (!desc)
		return;

	if (!desc->can_mask)
		return;

	spin_lock_irqsave(&pdev->msi_lock, flags);
	desc->mask &= ~clear;
	desc->mask |= set;
	pci_write_config_dword(pdev->bus, pdev->devfn,
			       desc->msi_attr.mask_pos,
			       desc->mask);
	spin_unlock_irqrestore(&pdev->msi_lock, flags);
}

void pci_msi_mask(struct device *dev, int hwirq)
{
	struct pci_device *pdev = to_pci_dev(dev);
	struct msi_desc *desc = get_msi_desc(dev->irq_domain, hwirq);
	unsigned int mask = 1 << (hwirq - desc->irq_base);

	if (!desc->is_msix)
		__pci_msi_mask(pdev, desc, mask, 0);
}

void pci_msi_unmask(struct device *dev, int hwirq)
{
	struct pci_device *pdev = to_pci_dev(dev);
	struct msi_desc *desc = get_msi_desc(dev->irq_domain, hwirq);
	unsigned int mask = 1 << (hwirq - desc->irq_base);

	if (!desc->is_msix)
		__pci_msi_mask(pdev, desc, 0, mask);
}

static void pci_msi_set_irq_base(struct pci_device *pdev, int irq_base, int nr)
{
	int i;
	struct msi_desc *desc;

	for (i = 0; i < nr; i++) {
		desc = get_msi_desc(pdev->dev.irq_domain, irq_base + i);
		if (!desc)
			continue;

		desc->irq_base = irq_base;
	}
}

int pci_msix_get_vec_count(struct pci_device *pdev)
{
	unsigned short msg_ctrl;

	if (!pdev->msix_cap_pos)
		return 0;

	msg_ctrl = pci_read_config_word(pdev->bus, pdev->devfn,
			pdev->msix_cap_pos + PCI_MSIX_FLAGS);

	return ((msg_ctrl & PCI_MSIX_FLAGS_QSIZE) + 1);
}

int pci_msi_get_vec_count(struct pci_device *pdev)
{
	unsigned short msg_ctrl;

	if (!pdev->msi_cap_pos)
		return 0;

	msg_ctrl = pci_read_config_word(pdev->bus, pdev->devfn,
			pdev->msi_cap_pos + PCI_MSI_FLAGS);

	return 1 << ((msg_ctrl & PCI_MSI_FLAGS_QMASK) >> PCI_MSI_FLAGS_QMASK);
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

static void pci_msi_write_msi_msg(struct device *dev, unsigned long msi_addr,
				   unsigned long msi_data, int hwirq, void *priv)
{
	struct pci_device *pdev = to_pci_dev(dev);
	unsigned int msi_addr_lo, msi_addr_hi;
	struct msi_desc *desc = get_msi_desc(dev->irq_domain, hwirq);
	unsigned short msg_ctrl;

	msi_addr_lo = (unsigned int)(msi_addr & 0xFFFFFFFF);
	msi_addr_hi = (unsigned int)(msi_addr >> 32);

	if (!desc)
		return;
	
	msg_ctrl = pci_read_config_word(pdev->bus, pdev->devfn,
			pdev->msi_cap_pos + PCI_MSI_FLAGS);
	msg_ctrl &= ~PCI_MSI_FLAGS_QSIZE;
	msg_ctrl |= (desc->msi_attr.multiple << PCI_MSI_FLAGS_QSIZE_SHIFT) & PCI_MSI_FLAGS_QSIZE;
	pci_write_config_word(pdev->bus, pdev->devfn, pdev->msi_cap_pos + PCI_MSI_FLAGS, msg_ctrl);

	pci_write_config_dword(pdev->bus, pdev->devfn,
			       pdev->msi_cap_pos + PCI_MSI_ADDRESS_LO,
			       msi_addr_lo);
	if (desc->msi_attr.is_64) {
		pci_write_config_dword(pdev->bus, pdev->devfn,
				       pdev->msi_cap_pos + PCI_MSI_ADDRESS_HI,
				       msi_addr_hi);
		pci_write_config_word(pdev->bus, pdev->devfn,
				      pdev->msi_cap_pos + PCI_MSI_DATA_64,
				      msi_data);
	} else {
		pci_write_config_word(pdev->bus, pdev->devfn,
				       pdev->msi_cap_pos + PCI_MSI_DATA_32,
				       msi_data);
	}
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
	pdev->msi_cap_pos = pci_find_capability(pdev, PCI_CAP_ID_MSI);
}

void pci_msix_init(struct pci_device *pdev)
{
	pdev->msix_cap_pos = pci_find_capability(pdev, PCI_CAP_ID_MSIX);
}

static void pci_msi_set_desc(struct device *dev, int hwirq, int nr)
{
	int i;
	struct msi_desc *desc;
	struct pci_device *pdev = to_pci_dev(dev);
	unsigned short msg_ctrl;

	msg_ctrl = pci_read_config_word(pdev->bus, pdev->devfn,
			pdev->msi_cap_pos + PCI_MSI_FLAGS);

	for (i = 0; i < nr; i++) {
		desc = get_msi_desc(pdev->dev.irq_domain, hwirq + i);
		desc->msi_attr.is_64 = msg_ctrl & PCI_MSI_FLAGS_64BIT;
		desc->msi_attr.multiple = fls64((unsigned long)nr);
		desc->can_mask = msg_ctrl & PCI_MSI_FLAGS_MASKBIT;
		if (desc->msi_attr.is_64)
			desc->msi_attr.mask_pos = pdev->msi_cap_pos + PCI_MSI_MASK_64;
		else
			desc->msi_attr.mask_pos = pdev->msi_cap_pos + PCI_MSI_MASK_32;

		desc->mask = pci_read_config_dword(pdev->bus, pdev->devfn,
						   desc->msi_attr.mask_pos);
	}
}

static void pci_msix_set_desc(struct device *dev, int hwirq, int nr)
{
	int i;
	struct msi_desc *desc;
	struct pci_device *pdev = to_pci_dev(dev);

	for (i = 0; i < nr; i++) {
		desc = get_msi_desc(pdev->dev.irq_domain, hwirq + i);
		desc->base = (void *)pdev->msix_base;
		desc->msi_attr.is_64 = 1;
		desc->entry_index = i;
		desc->is_msix = 1;
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

	irq_base = msi_get_hwirq(&pdev->dev, count,
				 pci_msix_write_msi_msg,
				 pci_msix_set_desc);
	pdev->dev.irq_num = count;
	for (i = 0; i < count; i++) {
		irqs[i] = irq_base + i;
		pdev->dev.irqs[i] = irqs[i];
	}

	msg_ctrl = pci_read_config_word(pdev->bus, pdev->devfn,
				pdev->msix_cap_pos + PCI_MSIX_FLAGS);
	msg_ctrl |= PCI_MSIX_FLAGS_ENABLE;
	pci_write_config_word(pdev->bus, pdev->devfn,
			      pdev->msix_cap_pos + PCI_MSIX_FLAGS, msg_ctrl);

	return count;
}

int pci_msi_enable(struct pci_device *pdev, int *irqs)
{
	int count;
	unsigned short msg_ctrl;
	int irq_base, i;

	count = pci_msi_get_vec_count(pdev);
	if (count == 0)
		return 0;

	irq_base = msi_get_hwirq(&pdev->dev, count,
				 pci_msi_write_msi_msg,
				 pci_msi_set_desc);
	pci_msi_set_irq_base(pdev, irq_base, count);
	pdev->dev.irq_num = count;
	for (i = 0; i < count; i++) {
		irqs[i] = irq_base + i;
		pdev->dev.irqs[i] = irqs[i];
	}

	msg_ctrl = pci_read_config_word(pdev->bus, pdev->devfn,
				pdev->msi_cap_pos + PCI_MSI_FLAGS);
	msg_ctrl |= PCI_MSI_FLAGS_ENABLE;
	pci_write_config_word(pdev->bus, pdev->devfn,
			      pdev->msi_cap_pos + PCI_MSI_FLAGS, msg_ctrl);

	return count;
}
