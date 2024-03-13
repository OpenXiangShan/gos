#include <asm/mmio.h>
#include "string.h"
#include "print.h"
#include "device.h"
#include "aplic.h"
#include "aplic_msi.h"

static struct aplic aplic;

void aplic_irq_mask(int hwirq, void *data)
{
	writel(aplic.base + APLIC_CLRIENUM, hwirq);
}

void aplic_irq_unmask(int hwirq, void *data)
{
	writel(aplic.base + APLIC_SETIENUM, hwirq);
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

int aplic_init(char *name, unsigned long base, struct irq_domain *parent,
	       void *data)
{
	struct aplic_priv_data *aplic_priv_data =
	    (struct aplic_priv_data *)data;

	memset((char *)&aplic, 0, sizeof(struct aplic));

	aplic.mode = aplic_priv_data->mode;
	aplic.name = name;
	aplic.base = base;
	aplic.parent = parent;
	aplic.nr_irqs = aplic_priv_data->nr_irqs;

	print("%s -- name:%s base:0x%x mode:%d\n", __FUNCTION__, aplic.name,
	      aplic.base, aplic.mode);

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

IRQCHIP_REGISTER(aplic, aplic_init, "APLIC");
