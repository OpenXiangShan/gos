#include <asm/mmio.h>
#include "aplic_msi.h"
#include "aplic.h"
#include "irq.h"
#include "print.h"
#include "../imsic/imsic.h"

void aplic_msi_write_msi_msg(unsigned long msi_addr, unsigned long msi_data,
			     int hwirq, void *priv)
{
	struct aplic *p_aplic = (struct aplic *)priv;
	unsigned long target;
	unsigned int hart_id, guest_id, val;

	target = p_aplic->base + APLIC_TARGET_BASE;
	target += (hwirq - 1) * sizeof(unsigned int);

	guest_id =
	    (msi_addr >> 12) & (~((~0UL) << p_aplic->imsic_guest_index_bits));
	hart_id =
	    (msi_addr >> (12 + p_aplic->imsic_guest_index_bits)) &
	    (~((~0UL) << p_aplic->imsic_hart_index_bits));

	val = (hart_id & APLIC_TARGET_HART_IDX_MASK)
	    << APLIC_TARGET_HART_IDX_SHIFT;
	val |= (guest_id & APLIC_TARGET_GUEST_IDX_MASK)
	    << APLIC_TARGET_GUEST_IDX_SHIFT;
	val |= (msi_data & APLIC_TARGET_EIID_MASK);

	//print
	//    ("%s -- hwirq:%d, msi_addr:0x%x, msi_data:%d hart_id:%d guest_id:%d target:0x%x val:0x%x\n",
	//     __FUNCTION__, hwirq, msi_addr, msi_data, hart_id, guest_id, target, val);
	writel(target, val);
}

static int aplic_msi_alloc_irqs(int nr_irqs, void *data)
{
	struct aplic *p_aplic = (struct aplic *)data;
	struct irq_domain *domain = &p_aplic->domain;
	int hwirq = -1, i;

	if (domain->parent_domain
	    && domain->parent_domain->domain_ops->alloc_irqs)
		hwirq =
		    domain->parent_domain->domain_ops->alloc_irqs(nr_irqs,
								  domain->
								  parent_domain->
								  priv);

	if (hwirq != -1) {
		for (i = 0; i < nr_irqs; i++)
			register_device_irq(domain->parent_domain, hwirq + i,
					    NULL, data);
	}

	return hwirq;
}

static int aplic_msi_mask_irq(int hwirq, void *data)
{
	aplic_irq_mask(hwirq, data);

	return 0;
}

static int aplic_msi_unmask_irq(int hwirq, void *data)
{
	aplic_irq_unmask(hwirq, data);

	return 0;
}

static struct irq_domain_ops aplic_msi_domain_ops = {
	.alloc_irqs = aplic_msi_alloc_irqs,
	.mask_irq = aplic_msi_mask_irq,
	.unmask_irq = aplic_msi_unmask_irq,
	.activate_irq = domain_activate_irq,
	.set_type = aplic_irq_set_type,
};

int aplic_msi_setup(struct aplic *aplic)
{
	struct irq_domain *base_domain;

	aplic_hw_mode_init(aplic);

	base_domain = imsic_get_irq_domain();
	msi_domain_init_hierarchy(&aplic->domain, aplic->name,
				  &aplic_msi_domain_ops, base_domain,
				  aplic_msi_write_msi_msg, aplic);

	return 0;
}
