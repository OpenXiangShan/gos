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

#include <device.h>
#include <asm/csr.h>
#include <asm/type.h>
#include <print.h>
#include <asm/mmio.h>
#include <irq.h>
#include <asm/trap.h>
#include "cpu.h"
#include "plic.h"
#include "percpu.h"
#include "asm/sbi.h"
#include "string.h"
#include "vmap.h"

static struct plic plic;
static DEFINE_PER_CPU(struct plic_percpu_info, plic_info);

struct plic_priv_data {
	unsigned char max_priority;
	unsigned char ndev;
};

struct irq_domain *plic_get_irq_domain(void)
{
	return &plic.irq_domain;
}

static void plic_set_prority(int hwirq, int pro)
{
	unsigned long reg = plic.base_address + PRIORITY_PER_ID * hwirq;

	writel(reg, pro);
}

static int plic_enable_irq(int cpu, int hwirq, int enable)
{
	unsigned int hwirq_mask = 1 << (hwirq % 32);
	struct plic_percpu_info *info;
	unsigned long reg;

	if (!cpu_is_online(cpu))
		return -1;

	info = &per_cpu(plic_info, cpu);
	reg = info->enable_base + 4 * (hwirq / 32);

	if (enable)
		writel(reg, readl(reg) | hwirq_mask);
	else
		writel(reg, readl(reg) & ~hwirq_mask);

	return 0;
}

static int plic_mask_irq(struct device *dev, int hwirq, void *data)
{
	plic_set_prority(hwirq, 0);

	return 0;
}

static int plic_unmask_irq(struct device *dev, int hwirq, void *data)
{
	plic_set_prority(hwirq, 1);

	return 0;
}

static int plic_set_affinity(struct device *dev, int hwirq, int cpu)
{
	int cpu_online;

	for_each_online_cpu(cpu_online) {
		plic_enable_irq(cpu_online, hwirq, 0);
	}

	plic_enable_irq(cpu, hwirq, 1);

	return 0;
}

static void plic_handle_irq()
{
	int hwirq;
	int cpu = sbi_get_cpu_id();
	struct plic_percpu_info *info = &per_cpu(plic_info, cpu);
	unsigned long claim_reg = info->base + CONTEXT_CLAIM;

	csr_clear(sie, SIE_SEIE);

	while ((hwirq = readl(claim_reg))) {
		domain_handle_irq(&plic.irq_domain, hwirq, NULL);
		writel(claim_reg, hwirq);
	}

	csr_set(sie, SIE_SEIE);
}

static int __plic_init(struct plic *plic, int cpu)
{
	int hwirq;
	unsigned int nr = plic->ndev;
	struct plic_percpu_info *info = &per_cpu(plic_info, cpu);

	info->base =
	    plic->base_address + CONTEXT_BASE + CPU_TO_HART(cpu) * CONTEXT_SIZE;
	info->enable_base =
	    plic->base_address + CONTEXT_ENABLE_BASE +
	    CPU_TO_HART(cpu) * CONTEXT_ENABLE_SIZE;

	writel(info->base + CONTEXT_THRESHOLD, 0);

	for (hwirq = 1; hwirq <= nr; hwirq++)
		plic_enable_irq(cpu, hwirq, 0);

	csr_set(sie, SIE_SEIE);

	return 0;
}

static int plic_cpuhp_startup(struct cpu_hotplug_notifier *notifier, int cpu)
{
	__plic_init(&plic, cpu);

	return 0;
}

static struct cpu_hotplug_notifier plic_cpuhp_notifier = {
	.startup = plic_cpuhp_startup,
};

static struct irq_domain_ops plic_irq_domain_ops = {
	.mask_irq = plic_mask_irq,
	.unmask_irq = plic_unmask_irq,
	.set_affinity = plic_set_affinity,
};

int plic_init(char *name, unsigned long base, int len,
	      struct irq_domain *parent, void *data)
{
	struct plic_priv_data *priv = (struct plic_priv_data *)data;
	unsigned long addr;
	int hwirq;

	memset((char *)&plic, 0, sizeof(struct plic));

	addr = (unsigned long)ioremap((void *)base, len, 0);

	plic.base_address = addr;
	if (priv) {
		plic.max_priority = priv->max_priority;
		plic.ndev = priv->ndev;
	}

	print("plic: name:%s, base:0x%lx, max_priority: %d, ndev: %d\n",
	      name, plic.base_address,
	      plic.max_priority, plic.ndev);

	__plic_init(&plic, 0);

	for (hwirq = 1; hwirq <= plic.ndev; hwirq++)
		plic_set_prority(hwirq, 0);

	cpu_hotplug_notify_register(&plic_cpuhp_notifier);

	irq_domain_init_cascade(&plic.irq_domain, name, &plic_irq_domain_ops,
				parent, INTERRUPT_CAUSE_EXTERNAL,
				plic_handle_irq, NULL);

	return 0;
}

IRQCHIP_REGISTER(plic, plic_init, "PLIC");
