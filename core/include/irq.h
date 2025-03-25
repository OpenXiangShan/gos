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

#ifndef _IRQ_H
#define _IRQ_H

#include "asm/asm-irq.h"
#include "list.h"
#include "device.h"

#define IRQ_TYPE_NONE           0
#define IRQ_TYPE_EDGE_RISING    1
#define IRQ_TYPE_EDGE_FALLING   2
#define IRQ_TYPE_EDGE_BOTH      (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING)
#define IRQ_TYPE_LEVEL_HIGH     4
#define IRQ_TYPE_LEVEL_LOW      8

#define enable_local_irq __enable_local_irq
#define disable_local_irq __disable_local_irq
#define local_irq_is_on __local_irq_is_on

#define IRQCHIP_INIT_TABLE __irqchip_init_table
#define IRQCHIP_INIT_TABLE_END __irqchip_init_table_end

#define SCAUSE_IRQ (1UL << 63)

struct device_init_entry;

typedef void (*irq_handler_t)(void *priv);
typedef void (*write_msi_msg_t)(struct device *dev, unsigned long msi_addr,
				unsigned long msi_data,
				int hwirq, void *priv);

struct irq_info {
	struct list_head list;
	int hwirq;
	void (*handler)(void *data);
	void *priv;
	void (*write_msi_msg)(struct device *dev, unsigned long msi_addr,
			      unsigned long msi_data, int hwirq, void *priv);
};

struct irq_domain;
struct irq_domain_ops {
	int (*alloc_irqs)(struct device *dev, int nr_irqs, void *data);
	int (*mask_irq)(struct device *dev, int hwirq, void *data);
	int (*unmask_irq)(struct device *dev, int hwirq, void *data);
	int (*activate_irq)(struct device *dev, struct irq_domain * domain, int hwirq,
			    int msi_irq, write_msi_msg_t write_msi_msg);
	int (*get_msi_msg)(struct device *dev, struct irq_domain * domain, int hwirq,
			   unsigned long *msi_addr, unsigned long *msi_data,
			   void *priv);
	int (*set_type)(struct device *dev, int hwirq, int type, void *data);
	int (*set_affinity)(struct device *dev, int hwirq, int cpu);
};

struct irq_domain {
	struct list_head list;
	char name[128];
	struct list_head irq_info_head;
	struct irq_domain *parent_domain;
	struct irq_domain *link_domain;
	struct irq_domain_ops *domain_ops;
	void *priv;
	write_msi_msg_t write_msi_msg;
};

struct msi_attr {
	int is_64;
	int multiple;
	unsigned char mask_pos;
};

struct msi_desc {
	struct list_head list;
	void *base;
	int entry_index;
	int irq_base;
	int hwirq;
	int can_mask;
	unsigned int mask;
	int is_msix;
	struct msi_attr msi_attr;
	struct irq_domain *domain;
};

typedef int (*irqchip_init)(char *name, unsigned long base, int len,
			    struct irq_domain * d, void *priv);

struct irqchip_init_entry {
	char compatible[128];
	irqchip_init init;
};

#define IRQCHIP_REGISTER(name, init_fn, compat)                               \
	static const struct irqchip_init_entry __attribute__((used))          \
		__irqchip_entry_##name                                        \
		__attribute__((section(".irqchip_init_table"))) = {           \
			.compatible = compat,                                 \
			.init = init_fn,                                      \
		}


int msi_get_hwirq_affinity(struct device *dev, int nr_irqs,
			   write_msi_msg_t write_msi_msg, int cpu,
			   void (*set_msi_desc)(struct device *dev, int hwirq, int nr));
int msi_get_hwirq(struct device *dev, int nr_irqs,
		  write_msi_msg_t write_msi_msg,
		  void (*set_msi_desc)(struct device *dev, int hwirq, int nr));
int get_hwirq(struct device *dev, int *ret_irq);
int irq_init(void);
int irq_domain_init_cascade(struct irq_domain *domain, char *name,
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
int register_device_irq(struct device *dev, struct irq_domain *domain, unsigned int hwirq,
			void (*handler)(void *data), void *priv);
int domain_activate_irq(struct device *dev, struct irq_domain *domain, int msi_irq, int hwirq,
			write_msi_msg_t write_msi_msg);
irq_handler_t get_irq_handler(int hwirq);
int msi_domain_init(struct irq_domain *domain, char *name,
		    struct irq_domain_ops *ops, struct irq_domain *parent,
		    write_msi_msg_t write_msi_msg, void *priv);
int msi_domain_init_hierarchy(struct irq_domain *domain, char *name,
			      struct irq_domain_ops *ops,
			      struct irq_domain *base_domain,
			      write_msi_msg_t write_msi_msg, void *priv);
int irq_domain_set_affinity(struct device *dev, struct irq_domain *domain, int hwirq, int cpu);
struct irq_domain *get_intc_domain(void);

#endif
