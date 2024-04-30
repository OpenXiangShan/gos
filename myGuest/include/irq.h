#ifndef __GUEST_IRQ_H__
#define __GUEST_IRQ_H__

typedef int (*do_irq_handler_t)(void);

struct msi_irq_ops {
	int (*alloc_irqs)(int nr);
	int (*compose_msi_msg)(int hwirq, unsigned long *msi_addr,
			       unsigned long *msi_data);
	int (*mask_irq)(int hwirq);
	int (*unmask_irq)(int hwirq);
};

int irq_handler(void);
void set_msi_irq_ops(struct msi_irq_ops *ops);
int alloc_msi_irqs(int nr);
int compose_msi_msg(int hwirq, unsigned long *msi_addr,
		    unsigned long *msi_data);
void set_irq_handler(do_irq_handler_t handler);

#endif
