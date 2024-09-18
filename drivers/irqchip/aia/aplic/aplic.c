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

#include <asm/mmio.h>
#include "string.h"
#include "print.h"
#include "device.h"
#include "aplic.h"
#include "aplic_msi.h"
#include "vmap.h"

static struct aplic aplic;

void aplic_irq_mask(int hwirq, void *data)
{
	writel(aplic.base + APLIC_CLRIENUM, hwirq);
}

void aplic_irq_unmask(int hwirq, void *data)
{
	writel(aplic.base + APLIC_SETIENUM, hwirq);
}

int aplic_irq_set_type(int hwirq, int type, void *data)
{
	unsigned long sourcecfg;
	unsigned int val;

	switch (type) {
	case IRQ_TYPE_NONE:
		val = APLIC_SOURCECFG_SM_INACTIVE;
		break;
	case IRQ_TYPE_LEVEL_LOW:
		val = APLIC_SOURCECFG_SM_LEVEL_LOW;
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		val = APLIC_SOURCECFG_SM_LEVEL_HIGH;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		val = APLIC_SOURCECFG_SM_EDGE_FALL;
		break;
	case IRQ_TYPE_EDGE_RISING:
		val = APLIC_SOURCECFG_SM_EDGE_RISE;
		break;
	default:
		return -1;
	}

	sourcecfg = aplic.base + APLIC_SOURCECFG_BASE;
	sourcecfg += (hwirq - 1) * sizeof(unsigned int);
	writel(sourcecfg, val);

	return 0;
}

void aplic_hw_mode_init(struct aplic *p_aplic)
{
	unsigned int val;

	val = readl(p_aplic->base + APLIC_DOMAINCFG);
	val |= APLIC_DOMAINCFG_IE;
	if (p_aplic->mode == APLIC_MSI_MODE)
		val |= APLIC_DOMAINCFG_DM;
	writel(p_aplic->base + APLIC_DOMAINCFG, val);
}

static void aplic_deleg_init(struct aplic *p_aplic)
{
	int i;

	if (p_aplic->delegate) {
		for (i = 0; i < p_aplic->nr_irqs; i++)
			writel(p_aplic->base + APLIC_SOURCECFG_BASE +
			       (i - 1) * sizeof(unsigned int),
			       (1U << 10) | p_aplic->child_index);
		return;
	}
}

static void aplic_hw_init(struct aplic *p_aplic)
{
	int i;

	for (i = 0; i < p_aplic->nr_irqs; i++)
		writel(p_aplic->base + APLIC_CLRIE_BASE +
		       (i / 32) * sizeof(unsigned int), -1U);

	for (i = 1; i <= p_aplic->nr_irqs; i++) {
		writel(p_aplic->base + APLIC_SOURCECFG_BASE +
		       (i - 1) * sizeof(unsigned int), 0);
		writel(p_aplic->base + APLIC_TARGET_BASE +
		       (i - 1) * sizeof(unsigned int), APLIC_DEFAULT_PRIORITY);
	}

	writel(p_aplic->base + APLIC_DOMAINCFG, 0);
}

static void aplic_m_mode_init(struct aplic *p_aplic)
{
	unsigned long imsic_base;
	int hart_index;
	unsigned int low_base_ppn, high_base_ppn, LHXS;
	unsigned int val_smsicfgaddrh = 0, val_smsicfgaddr = 0;

	imsic_base = imsic_get_interrupt_file_base(0, 0);
	hart_index = imsic_get_hart_index_bits();

	low_base_ppn = (imsic_base << 32) >> 32 >> 12;
	high_base_ppn = (imsic_base >> 32) >> 12;
	LHXS = hart_index;

	val_smsicfgaddr = low_base_ppn;
	val_smsicfgaddrh = high_base_ppn | (LHXS << 20);

	writel(p_aplic->base + APLIC_SMSICFGADDR, val_smsicfgaddr);
	writel(p_aplic->base + APLIC_SMSICFGADDRH, val_smsicfgaddrh);
}

int aplic_init(char *name, unsigned long base, int len,
	       struct irq_domain *parent, void *data)
{
	unsigned long addr;
	struct aplic_priv_data *aplic_priv_data =
	    (struct aplic_priv_data *)data;

	memset((char *)&aplic, 0, sizeof(struct aplic));

	addr = (unsigned long)ioremap((void *)base, len, 0);

	aplic.mmode = aplic_priv_data->mmode;
	aplic.mode = aplic_priv_data->mode;
	aplic.index = aplic_priv_data->index;
	aplic.name = name;
	aplic.base = addr;
	aplic.parent = parent;
	aplic.nr_irqs = aplic_priv_data->nr_irqs;
	aplic.delegate = aplic_priv_data->delegate;
	aplic.child_index = aplic_priv_data->child_index;

	print
	    ("%s -- name:%s base:0x%lx mmode:%d mode:%d index:%d delegate:%d child_index:%d\n",
	     __FUNCTION__, aplic.name, aplic.base, aplic.mmode, aplic.mode,
	     aplic.index, aplic.delegate, aplic.child_index);

	if (aplic.mmode == APLIC_M_MODE) {
		aplic_m_mode_init(&aplic);
	}

	if (aplic.delegate) {
		aplic_deleg_init(&aplic);
		return 0;
	}

	aplic_hw_init(&aplic);

	if (aplic.mode == APLIC_DIRECT_MODE) {
		// TODO

		return 0;
	} else if (aplic.mode == APLIC_MSI_MODE) {
		if (!aplic_priv_data->imsic_data)
			return -1;

		aplic.imsic_guest_index_bits =
		    aplic_priv_data->imsic_data->guest_index_bits;
		aplic.imsic_hart_index_bits =
		    aplic_priv_data->imsic_data->hart_index_bits;
		aplic.imsic_group_index_bits =
		    aplic_priv_data->imsic_data->group_index_bits;

		aplic_msi_setup(&aplic);
	} else {
		print("unsupported aplic mode\n");
		return -1;
	}

	print("%s success irq_domain:0x%x\n", __FUNCTION__, &aplic.domain);

	return 0;
}

IRQCHIP_REGISTER(aplic_m, aplic_init, "APLIC_M");
IRQCHIP_REGISTER(aplic_s, aplic_init, "APLIC_S");
