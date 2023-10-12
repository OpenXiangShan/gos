#ifndef _IRQ_H
#define _IRQ_H

#include <asm/asm-irq.h>

#define enable_local_irq __enable_local_irq

#define SCAUSE_IRQ (1UL << 63)

struct device_init_entry;

typedef void (*irq_handler_t)(void *priv);

struct irq_info {
	int hwirq;
	void (*handler)(void *data);
	void *priv;
};

struct irq_domain {
	unsigned long cause;
	void (*handler)(void);
};

void handle_irq(unsigned long cause);
int irqchip_setup(struct device_init_entry *hw);
int timer_setup(struct device_init_entry *hw);
int register_device_irq(int hwirq, void (*handler)(void *data), void *priv);
irq_handler_t get_irq_handler(int hwirq);
void *get_irq_priv_data(int hwirq);

#endif
