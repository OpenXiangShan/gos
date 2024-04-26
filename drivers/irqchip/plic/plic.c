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

static struct plic plic;
static DEFINE_PER_CPU(struct plic_percpu_info, plic_info);

struct plic_priv_data {
	unsigned char max_priority;
	unsigned char ndev;
};

static void plic_set_prority(int hwirq, int pro)
{
	unsigned long reg = plic.base_address + PRIORITY_PER_ID * hwirq;

	writel(reg, 1);
}

static void plic_enable_irq(int cpu, int hwirq, int enable)
{
	unsigned int hwirq_mask = 1 << (hwirq % 32);
	struct plic_percpu_info *info = &per_cpu(plic_info, cpu);
	unsigned long reg = info->enable_base + 4 * (hwirq / 32);

	if (enable)
		writel(reg, readl(reg) | hwirq_mask);
	else
		writel(reg, readl(reg) & ~hwirq_mask);
}

static int plic_mask_irq(int hwirq, void *data)
{
	plic_enable_irq(0, hwirq, 0);

	return 0;
}

static int plic_unmask_irq(int hwirq, void *data)
{
	plic_enable_irq(0, hwirq, 1);

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
	unsigned char max_priority = plic->max_priority;
	struct plic_percpu_info *info = &per_cpu(plic_info, cpu);

	info->base =
	    plic->base_address + CONTEXT_BASE + CPU_TO_HART(cpu) * CONTEXT_SIZE;
	info->enable_base =
	    plic->base_address + CONTEXT_ENABLE_BASE +
	    CPU_TO_HART(cpu) * CONTEXT_ENABLE_SIZE;

	writel(info->base + CONTEXT_THRESHOLD, 0);

	for (hwirq = 1; hwirq <= nr; hwirq++) {
		plic_enable_irq(cpu, hwirq, 0);

		plic_set_prority(hwirq, max_priority);
	}

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
};

int plic_init(char *name, unsigned long base, struct irq_domain *parent,
	      void *data)
{
	struct plic_priv_data *priv = (struct plic_priv_data *)data;
	unsigned char max_priority = 0, ndev = 0;

	memset((char *)&plic, 0, sizeof(struct plic));

	plic.base_address = base;
	if (priv) {
		plic.max_priority = priv->max_priority;
		plic.ndev = priv->ndev;
	}

	print("%s -- %s %d, name:%s, base:0x%lx, max_priority: %d, ndev: %d\n",
	      __FILE__, __FUNCTION__, __LINE__, name, base, max_priority, ndev);

	__plic_init(&plic, 0);

	cpu_hotplug_notify_register(&plic_cpuhp_notifier);

	irq_domain_init_cascade(&plic.irq_domain, name, &plic_irq_domain_ops,
				parent, INTERRUPT_CAUSE_EXTERNAL,
				plic_handle_irq, NULL);

	return 0;
}

IRQCHIP_REGISTER(plic, plic_init, "PLIC");
