#ifndef _IRQ_H
#define _IRQ_H

#include <asm/asm-irq.h>
#include "list.h"

#define enable_local_irq __enable_local_irq

#define SCAUSE_IRQ (1UL << 63)

struct device_init_entry;

typedef void (*irq_handler_t)(void *priv);

struct irq_info {
	struct list_head list;
	int hwirq;
	void (*handler)(void *data);
	void *priv;
};

struct irq_domain {
	struct list_head list;
	char name[128];
	struct list_head irq_info_head;
	struct irq_domain *parent_domain;
};

int irq_init();
int irq_domain_init_hierarchy(struct irq_domain *domain, char *name,
			      struct irq_domain *parent, unsigned int hwirq,
			      void (*handler)(void *data), void *priv);
int irq_domain_init(struct irq_domain *domain, char *name,
		    struct irq_domain *parent);
struct irq_domain *find_irq_domain(char *name);
struct irq_info *find_irq_info(struct irq_domain *domain, int hwirq);
void handle_irq(unsigned long cause);
int domain_handle_irq(struct irq_domain *domain, unsigned int hwirq,
		      void *data);
int irqchip_setup(struct device_init_entry *hw);
int register_device_irq(struct irq_domain *domain, unsigned int hwirq,
			void (*handler)(void *data), void *priv);
irq_handler_t get_irq_handler(int hwirq);

#endif
