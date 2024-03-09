#include <device.h>
#include <asm/csr.h>
#include <asm/type.h>
#include <print.h>
#include <asm/mmio.h>
#include <irq.h>
#include <asm/trap.h>

#define MAX_CPUS 1
#define CPU_TO_HART(cpu) (2 * cpu + 1)

#define PLIC_PRIORITY(id) (id * 4)
#define PLIC_PENDING(id) (0x1000 + ((id) / 32) * 4)
#define PLIC_MENABLE(hart) (0x2000 + (hart) * 0x80)
#define PLIC_MTHRESHOLD(hart) (0x200000 + (hart) * 0x1000)
#define PLIC_MCLAIM(hart) (0x200004 + (hart) * 0x1000)
#define PLIC_MCOMPLETE(hart) (0x200004 + (hart) * 0x1000)

unsigned int base_address;

static struct irq_domain plic_irq_domain;

struct plic_priv_data {
	unsigned char max_priority;
	unsigned char ndev;
};

static void plic_set_prority(int hwirq, int pro)
{
	unsigned int reg = base_address + PLIC_PRIORITY(hwirq);

	writel(reg, 1);
}

static void plic_enable_irq(int cpu, int hwirq, int enable)
{
	unsigned int hwirq_mask = 1 << (hwirq % 32);
	int hart = CPU_TO_HART(cpu);
	unsigned int reg = base_address + PLIC_MENABLE(hart) + 4 * (hwirq / 32);

	if (enable)
		writel(reg, readl(reg) | hwirq_mask);
	else
		writel(reg, readl(reg) & ~hwirq_mask);
}

static void plic_handle_irq()
{
	int hwirq;
	int hart = CPU_TO_HART(0);
	unsigned int claim_reg = base_address + PLIC_MCLAIM(hart);

	csr_clear(sie, SIE_SEIE);

	while ((hwirq = readl(claim_reg))) {
		domain_handle_irq(&plic_irq_domain, hwirq, NULL);
		writel(claim_reg, hwirq);
	}

	csr_set(sie, SIE_SEIE);
}

static int __plic_init(unsigned long base, struct plic_priv_data *priv)
{
	int i, hwirq;
	unsigned int nr = priv->ndev;
	unsigned char max_priority = priv->max_priority;

	base_address = base;

	for (i = 0; i < MAX_CPUS; i++) {
		writel(base_address + PLIC_MTHRESHOLD(CPU_TO_HART(i)), 0);

		for (hwirq = 1; hwirq <= nr; hwirq++) {
			plic_enable_irq(i, hwirq, 0);

			plic_set_prority(hwirq, max_priority);
		}
	}

	for (i = 0; i < MAX_CPUS; i++) {
		for (hwirq = 1; hwirq <= nr; hwirq++) {
			plic_enable_irq(i, hwirq, 1);
		}
	}

	csr_set(sie, SIE_SEIE);

	return 0;
}

int plic_init(char *name, unsigned long base, struct irq_domain *parent,
	      void *data)
{
	struct plic_priv_data *priv = (struct plic_priv_data *)data;
	unsigned char max_priority = 0, ndev = 0;

	if (priv) {
		max_priority = priv->max_priority;
		ndev = priv->ndev;
	}

	print("%s -- %s %d, name:%s, base:0x%x, max_priority: %d, ndev: %d\n",
	      __FILE__, __FUNCTION__, __LINE__, name, base, max_priority, ndev);

	__plic_init(base, priv);

	irq_domain_init_hierarchy(&plic_irq_domain, name, parent,
				  INTERRUPT_CAUSE_EXTERNAL, plic_handle_irq,
				  NULL);

	return 0;
}

IRQCHIP_REGISTER(plic, plic_init, "PLIC");
