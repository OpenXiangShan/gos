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
#include <string.h>
#include <trap.h>
#include <print.h>
#include <asm/type.h>
#include <asm/sbi.h>
#include <irq.h>
#include <timer.h>
#include "mm.h"
#include "vmap.h"
#include "irq.h"

extern int mmu_is_on;

static LIST_HEAD(irq_domains);
static struct irq_domain intc_domain;

int irq_domain_set_affinity(struct device *dev, struct irq_domain *domain, int hwirq, int cpu)
{
	if (!domain || !domain->domain_ops || !domain->domain_ops->set_affinity)
		return -1;

	return domain->domain_ops->set_affinity(dev, hwirq, cpu);
}

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

	list_for_each_entry(info, &domain->irq_info_head, list) {
		if (info->hwirq == hwirq)
			return info;
	}

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
						   device_entry->start,
						   device_entry->len, d,
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

	irq_info = find_irq_info(domain, hwirq);
	if (!irq_info || !irq_info->handler)
		return -1;

	irq_info->handler(irq_info->priv);

	return 0;
}

int get_hwirq(struct device *dev, int *ret_irq)
{
	struct irq_domain *irq_domain = dev->irq_domain;
	int *irqs = dev->irqs, i, num, hwirq;

	if (dev->irq_num > MAX_IRQ_NUM)
		num = MAX_IRQ_NUM;
	else
		num = dev->irq_num;

	if (num == 0)
		return 0;

	if (!irqs)
		return -1;

	if (!irq_domain->link_domain) {
		for (i = 0; i < num; i++)
			ret_irq[i] = irqs[i];

		if (!irq_domain->domain_ops
		    || !irq_domain->domain_ops->alloc_irqs)
			goto out;
	}

	hwirq = irq_domain->domain_ops->alloc_irqs(dev, num, irq_domain->priv);
	if (hwirq == -1)
		return 0;

	for (i = 0; i < num; i++) {
		domain_activate_irq(dev, irq_domain, irqs[i], hwirq + i, NULL);
	}

	if (irq_domain->link_domain) {
		for (i = 0; i < num; i++)
			ret_irq[i] = hwirq + i;

		for (i = 0; i < num; i++) {
			if (irq_domain->link_domain->domain_ops
			    && irq_domain->link_domain->domain_ops->set_type)
				irq_domain->link_domain->domain_ops->
				    set_type(dev, hwirq + i, IRQ_TYPE_LEVEL_HIGH,
					     irq_domain->link_domain->priv);

			if (irq_domain->link_domain->domain_ops
			    && irq_domain->link_domain->domain_ops->unmask_irq)
				irq_domain->link_domain->domain_ops->
				    unmask_irq(dev, hwirq + i,
					       irq_domain->link_domain->priv);
		}
	}

out:
	for (i = 0; i < num; i++) {
		if (irq_domain->domain_ops && irq_domain->domain_ops->set_type)
			irq_domain->domain_ops->set_type(dev, irqs[i],
							 IRQ_TYPE_LEVEL_HIGH,
							 irq_domain->priv);

		if (irq_domain->domain_ops
		    && irq_domain->domain_ops->unmask_irq)
			irq_domain->domain_ops->unmask_irq(dev, irqs[i],
							   irq_domain->priv);

		if (irq_domain->domain_ops
		    && irq_domain->domain_ops->set_affinity)
			irq_domain->domain_ops->set_affinity(dev, irqs[i], 0);
	}

	return num;
}

int msi_get_hwirq_affinity(struct device *dev, int nr_irqs,
			   write_msi_msg_t write_msi_msg, int cpu,
			   void (*set_msi_desc)(struct device *dev, int hwirq, int nr))
{
	int i, hwirq;
	struct irq_domain *irq_domain = dev->irq_domain;

	hwirq = irq_domain->domain_ops->alloc_irqs(dev, nr_irqs, irq_domain->priv);
	if (hwirq == -1)
		return 0;

	if (set_msi_desc)
		set_msi_desc(dev, hwirq, nr_irqs);

	if (!irq_domain->domain_ops || !irq_domain->domain_ops->set_affinity)
		goto activate_irqs;

	for (i = 0; i < nr_irqs; i++)
		irq_domain->domain_ops->set_affinity(dev, hwirq + i, cpu);

activate_irqs:
	for (i = 0; i < nr_irqs; i++) {
		domain_activate_irq(dev, irq_domain, hwirq + i, hwirq + i,
				    write_msi_msg);

		if (irq_domain->domain_ops && irq_domain->domain_ops->set_type)
			irq_domain->domain_ops->set_type(dev, hwirq + i,
							 IRQ_TYPE_LEVEL_HIGH,
							 irq_domain->priv);

		if (irq_domain->domain_ops
		    && irq_domain->domain_ops->unmask_irq)
			irq_domain->domain_ops->unmask_irq(dev, hwirq + i,
							   irq_domain->priv);
	}

	return hwirq;
}

int msi_get_hwirq(struct device *dev, int nr_irqs,
		  write_msi_msg_t write_msi_msg,
		  void (*set_msi_desc)(struct device *dev, int hwirq, int nr))
{
	struct irq_domain *irq_domain = dev->irq_domain;
	int i, hwirq;

	if (!irq_domain->domain_ops->alloc_irqs)
		return 0;

	hwirq = irq_domain->domain_ops->alloc_irqs(dev, nr_irqs, irq_domain->priv);
	if (hwirq == -1)
		return 0;

	if (set_msi_desc)
		set_msi_desc(dev, hwirq, nr_irqs);

	for (i = 0; i < nr_irqs; i++) {
		domain_activate_irq(dev, irq_domain, hwirq + i, hwirq + i,
				    write_msi_msg);

		if (irq_domain->domain_ops && irq_domain->domain_ops->set_type)
			irq_domain->domain_ops->set_type(dev, hwirq + i,
							 IRQ_TYPE_LEVEL_HIGH,
							 irq_domain->priv);

		if (irq_domain->domain_ops
		    && irq_domain->domain_ops->unmask_irq)
			irq_domain->domain_ops->unmask_irq(dev, hwirq + i,
							   irq_domain->priv);
	}

	return hwirq;
}

int register_device_irq(struct device *dev, struct irq_domain *irq_domain, unsigned int hwirq,
			void (*handler)(void *data), void *priv)
{
	struct irq_info *irq_info = NULL;
	struct irq_domain *domain;

	if (irq_domain->link_domain)
		domain = irq_domain->link_domain;
	else
		domain = irq_domain;

	irq_info = find_irq_info(domain, hwirq);
	if (!irq_info) {
		irq_info = (struct irq_info *)mm_alloc(sizeof(struct irq_info));
		if (!irq_info) {
			print("%s -- Out of memory\n", __FUNCTION__);
			return -1;
		}
		list_add(&irq_info->list, &domain->irq_info_head);
	}

	irq_info->hwirq = hwirq;
	irq_info->handler = handler;
	irq_info->priv = priv;

	return 0;
}

int domain_activate_irq(struct device *dev,
			struct irq_domain *domain,
			int msi_irq, int hwirq,
			write_msi_msg_t write_msi_msg)
{
	struct irq_domain *p_domain = domain;
	unsigned long msi_addr = 0, msi_data = 0;

	while (p_domain) {
		if (p_domain && p_domain->domain_ops->get_msi_msg)
			break;

		p_domain = p_domain->parent_domain;
	}

	if (p_domain && p_domain->domain_ops->get_msi_msg)
		p_domain->domain_ops->get_msi_msg(dev, p_domain, hwirq,
						  &msi_addr, &msi_data,
						  p_domain->priv);

	if (write_msi_msg) {
		write_msi_msg(dev, msi_addr, msi_data, msi_irq, NULL);
		return 0;
	}

	p_domain = domain;
	while (p_domain) {
		if (p_domain && p_domain->write_msi_msg)
			break;

		p_domain = p_domain->parent_domain;
	}

	if (p_domain && p_domain->write_msi_msg)
		p_domain->write_msi_msg(dev, msi_addr, msi_data, msi_irq,
					domain->priv);

	return 0;
}

int irq_domain_init(struct irq_domain *domain,
		    char *name, struct irq_domain_ops *ops,
		    struct irq_domain *parent, void *priv)
{
	domain->parent_domain = parent;
	strcpy(domain->name, name);
	INIT_LIST_HEAD(&domain->irq_info_head);
	list_add(&domain->list, &irq_domains);
	domain->priv = priv;
	domain->domain_ops = ops;

	return 0;
}

int msi_domain_init(struct irq_domain *domain, char *name,
		    struct irq_domain_ops *ops, struct irq_domain *parent,
		    write_msi_msg_t write_msi_msg, void *priv)
{
	memset((char *)domain, 0, sizeof(struct irq_domain));
	domain->write_msi_msg = write_msi_msg;

	return irq_domain_init(domain, name, ops, parent, priv);
}

int msi_domain_init_hierarchy(struct irq_domain *domain, char *name,
			      struct irq_domain_ops *ops,
			      struct irq_domain *base_domain,
			      write_msi_msg_t write_msi_msg, void *priv)
{
	memset((char *)domain, 0, sizeof(struct irq_domain));
	domain->write_msi_msg = write_msi_msg;
	domain->link_domain = base_domain;

	return irq_domain_init(domain, name, ops, base_domain, priv);
}

int irq_domain_init_cascade(struct irq_domain *domain, char *name,
			    struct irq_domain_ops *ops,
			    struct irq_domain *parent, unsigned int hwirq,
			    void (*handler)(void *data), void *priv)
{
	memset((char *)domain, 0, sizeof(struct irq_domain));
	register_device_irq(NULL, parent, hwirq, handler, priv);

	return irq_domain_init(domain, name, ops, parent, priv);
}

struct irq_domain *get_intc_domain(void)
{
	return &intc_domain;
}

int irq_init(void)
{
	memset((char *)&intc_domain, 0, sizeof(struct irq_domain));
	return irq_domain_init(&intc_domain, "INTC", NULL, NULL, NULL);
}
