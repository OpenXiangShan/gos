#include <device.h>
#include <string.h>
#include <trap.h>
#include <print.h>
#include <asm/type.h>
#include <irq.h>
#include <timer.h>
#include "mm.h"

#define MAX_IRQ_NUM 64

static LIST_HEAD(irq_domains);

static struct irq_domain intc_domain;

struct irq_domain *find_irq_domain(char *name)
{
	struct irq_domain *domain;

	list_for_each_entry(domain, &irq_domains, list)
	    if (!strncmp(domain->name, name, 128))
		return domain;

	return NULL;
}

struct irq_info *find_irq_info(struct irq_domain *domain, int hwirq)
{
	struct irq_info *info;

	if (!domain)
		return NULL;

	list_for_each_entry(info, &domain->irq_info_head, list)
	    if (info->hwirq == hwirq)
		return info;

	return NULL;
}

void handle_irq(unsigned long cause)
{
	struct irq_domain *d = find_irq_domain("INTC");
	struct irq_info *irq_info;

	if (!d) {
		print("unsupported cause: %d\n", cause & (~SCAUSE_IRQ));
		while (1) ;
		return;
	}

	irq_info = find_irq_info(d, cause & (~SCAUSE_IRQ));
	if (!irq_info || !irq_info->handler)
		return;

	irq_info->handler(irq_info->priv);
}

int irqchip_setup(struct device_init_entry *hw)
{
	extern unsigned long IRQCHIP_INIT_TABLE, IRQCHIP_INIT_TABLE_END;
	int driver_nr =
	    (struct irqchip_init_entry *)&IRQCHIP_INIT_TABLE_END -
	    (struct irqchip_init_entry *)&IRQCHIP_INIT_TABLE;
	int driver_nr_tmp = 0;
	struct irqchip_init_entry *driver_entry;
	struct device_init_entry *device_entry = hw;
	struct irqchip_init_entry *driver_tmp =
	    (struct irqchip_init_entry *)&IRQCHIP_INIT_TABLE;
	struct irq_domain *d;

	while (strncmp(device_entry->compatible, "THE END", sizeof("THE_END"))) {
		driver_nr_tmp = driver_nr;
		for (driver_entry = driver_tmp; driver_nr_tmp;
		     driver_entry++, driver_nr_tmp--) {
			d = find_irq_domain(device_entry->irq_parent);
			if (!strncmp
			    (driver_entry->compatible, device_entry->compatible,
			     128)) {
				driver_entry->init(device_entry->compatible,
						   device_entry->start, d,
						   device_entry->data);
			}
		}
		device_entry++;
	}

	return 0;
}

int domain_handle_irq(struct irq_domain *domain, unsigned int hwirq, void *data)
{
	struct irq_info *irq_info;
	if (!domain->parent_domain)
		return -1;

	irq_info = find_irq_info(domain, hwirq);
	if (!irq_info || !irq_info->handler)
		return -1;

	irq_info->handler(irq_info->priv);

	return 0;
}

int register_device_irq(struct irq_domain *domain, unsigned int hwirq,
			void (*handler)(void *data), void *priv)
{
	struct irq_info *irq_info = NULL;

	irq_info = find_irq_info(domain, hwirq);
	if (!irq_info) {
		irq_info = (struct irq_info *)mm_alloc(sizeof(struct irq_info));
		if (!irq_info) {
			print("%s -- Out of memory\n", __FUNCTION__);
			return -1;
		}
	}

	irq_info->hwirq = hwirq;
	irq_info->handler = handler;
	irq_info->priv = priv;
	list_add(&irq_info->list, &domain->irq_info_head);

	return 0;
}

int irq_domain_init(struct irq_domain *domain, char *name,
		    struct irq_domain *parent)
{
	domain->parent_domain = parent;
	strcpy(domain->name, name);
	INIT_LIST_HEAD(&domain->irq_info_head);
	list_add(&domain->list, &irq_domains);

	return 0;
}

int irq_domain_init_hierarchy(struct irq_domain *domain, char *name,
			      struct irq_domain *parent, unsigned int hwirq,
			      void (*handler)(void *data), void *priv)
{
	register_device_irq(parent, hwirq, handler, priv);

	return irq_domain_init(domain, name, parent);
}

int irq_init()
{
	return irq_domain_init(&intc_domain, "INTC", NULL);
}
