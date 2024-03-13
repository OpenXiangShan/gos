#ifndef _IRQ_H
#define _IRQ_H

#include <asm/asm-irq.h>
#include "list.h"

#define enable_local_irq __enable_local_irq

#define SCAUSE_IRQ (1UL << 63)

struct device_init_entry;

typedef void (*irq_handler_t)(void *priv);
typedef void (*write_msi_msg_t)(unsigned long msi_addr, unsigned long msi_data,
				int hwirq, void *priv);

struct irq_info {
	struct list_head list;
	int hwirq;
	void (*handler)(void *data);
	void *priv;
	void (*write_msi_msg)(unsigned long msi_addr, unsigned long msi_data,
			      int hwirq, void *priv);
};

struct irq_domain;
struct irq_domain_ops {
	int (*alloc_irqs)(int nr_irqs, void *data);
	int (*mask_irq)(int hwirq, void *data);
	int (*unmask_irq)(int hwirq, void *data);
	int (*activate_irq)(struct irq_domain * domain, int hwirq,
			    write_msi_msg_t write_msi_msg);
	int (*get_msi_msg)(struct irq_domain * domain, int hwirq,
			   unsigned long *msi_addr, unsigned long *msi_data,
			   void *priv);
};

struct irq_domain {
	struct list_head list;
	char name[128];
	struct list_head irq_info_head;
	struct irq_domain *parent_domain;
	struct irq_domain_ops *domain_ops;
	void *priv;
	write_msi_msg_t write_msi_msg;
};

int irq_init(void);
int irq_domain_init_hierarchy(struct irq_domain *domain, char *name,
			      struct irq_domain_ops *ops,
			      struct irq_domain *parent, unsigned int hwirq,
			      void (*handler)(void *data), void *priv);
int irq_domain_init(struct irq_domain *domain, char *name,
		    struct irq_domain_ops *ops, struct irq_domain *parent,
		    void *priv);
struct irq_domain *find_irq_domain(char *name);
struct irq_info *find_irq_info(struct irq_domain *domain, int hwirq);
void handle_irq(unsigned long cause);
int domain_handle_irq(struct irq_domain *domain, unsigned int hwirq,
		      void *data);
int irqchip_setup(struct device_init_entry *hw);
int register_device_irq(struct irq_domain *domain, unsigned int hwirq,
			void (*handler)(void *data), void *priv);
int domain_activate_irq(struct irq_domain *domain, int hwirq,
			write_msi_msg_t write_msi_msg);
irq_handler_t get_irq_handler(int hwirq);
int msi_domain_init(struct irq_domain *domain, char *name,
		    struct irq_domain_ops *ops, struct irq_domain *parent,
		    write_msi_msg_t write_msi_msg, void *priv);

#endif
