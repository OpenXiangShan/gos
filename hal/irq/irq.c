#include <device.h>
#include <string.h>
#include <trap.h>
#include <print.h>
#include <asm/type.h>
#include <irq.h>
#include <timer.h>

#define MAX_IRQ_NUM 64
static struct irq_info interrupts_ctx[MAX_IRQ_NUM];
static unsigned long irq_bit_flag;

static struct irq_domain domain[] = {
	{
	 .cause = 1,
	 .handler = NULL,
	  },
	{
	 .cause = 5,
	 .handler = NULL,
	  },
	{
	 .cause = 9,
	 .handler = NULL,
	  },
};

#define DOMAIN_NUM sizeof(domain)/sizeof(domain[0])

static struct irq_domain *find_irq_domain(unsigned long cause)
{
	int i;

	for (i = 0; i < DOMAIN_NUM; i++) {
		if (domain[i].cause == cause)
			return &domain[i];
	}

	return NULL;
}

void handle_irq(unsigned long cause)
{
	struct irq_domain *d = find_irq_domain(cause & (~SCAUSE_IRQ));

	if (!d) {
		print("unsupported cause: %d\n", cause & (~SCAUSE_IRQ));
		while (1) ;
		return;
	}

	d->handler();
}

int irqchip_setup(struct device_init_entry *hw)
{
	extern unsigned long IRQCHIP_INIT_TABLE, IRQCHIP_INIT_TABLE_END;
	int driver_nr =
	    (struct irqchip_init_entry *)&IRQCHIP_INIT_TABLE_END -
	    (struct irqchip_init_entry *)&IRQCHIP_INIT_TABLE;
	int driver_nr_tmp = 0;
	int i;
	struct irqchip_init_entry *driver_entry;
	struct device_init_entry *device_entry = hw;
	struct irqchip_init_entry *driver_tmp =
	    (struct irqchip_init_entry *)&IRQCHIP_INIT_TABLE;
	struct irq_domain *d = find_irq_domain(INTERRUPT_CAUSE_EXTERNAL);

	while (strncmp(device_entry->compatible, "THE END", sizeof("THE_END"))) {
		driver_nr_tmp = driver_nr;
		for (driver_entry = driver_tmp; driver_nr_tmp;
		     driver_entry++, driver_nr_tmp--) {
			if (!strncmp
			    (driver_entry->compatible, device_entry->compatible,
			     128)) {
				driver_entry->init(device_entry->start, d,
						   device_entry->data);
			}
		}
		device_entry++;
	}

	for (i = 0; i < 64; i++) {
		interrupts_ctx[i].hwirq = -1;
		interrupts_ctx[i].handler = NULL;
		interrupts_ctx[i].priv = NULL;
	}
	irq_bit_flag = 0;

	return 0;
}

int timer_setup(struct device_init_entry *hw)
{
	extern unsigned long TIMER_INIT_TABLE, TIMER_INIT_TABLE_END;
	int driver_nr =
	    (struct timer_init_entry *)&TIMER_INIT_TABLE_END -
	    (struct timer_init_entry *)&TIMER_INIT_TABLE;
	int driver_nr_tmp = 0;
	struct timer_init_entry *driver_entry;
	struct device_init_entry *device_entry = hw;
	struct timer_init_entry *driver_tmp =
	    (struct timer_init_entry *)&TIMER_INIT_TABLE;
	struct irq_domain *d = find_irq_domain(INTERRUPT_CAUSE_TIMER);

	while (strncmp(device_entry->compatible, "THE END", sizeof("THE END"))) {
		driver_nr_tmp = driver_nr;
		for (driver_entry = driver_tmp; driver_nr_tmp;
		     driver_entry++, driver_nr_tmp--) {
			if (!strncmp
			    (driver_entry->compatible, device_entry->compatible,
			     128)) {
				driver_entry->init(device_entry->start, d,
						   device_entry->data);
			}
		}
		device_entry++;
	}

	return 0;
}

int register_device_irq(int hwirq, void (*handler)(void *data), void *priv)
{
	struct irq_info *interrupt;

	if (0x01 & (irq_bit_flag >> hwirq))
		return -1;

	irq_bit_flag |= (unsigned long)1 << hwirq;

	interrupt = &interrupts_ctx[hwirq];
	interrupt->hwirq = hwirq;
	interrupt->handler = handler;
	interrupt->priv = priv;

	return 0;
}

irq_handler_t get_irq_handler(int hwirq)
{
	if (hwirq > MAX_IRQ_NUM)
		return NULL;

	if (!(0x01 & (irq_bit_flag >> hwirq)))
		return NULL;

	return interrupts_ctx[hwirq].handler;
}

void *get_irq_priv_data(int hwirq)
{
	if (hwirq > MAX_IRQ_NUM)
		return NULL;

	if (0x01 & (irq_bit_flag >> hwirq))
		return NULL;

	return interrupts_ctx[hwirq].priv;
}
