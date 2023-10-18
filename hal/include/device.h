#ifndef DEVICE_H
#define DEVICE_H

#include <irq.h>

#define DRIVER_INIT_TABLE __driver_init_table
#define DRIVER_INIT_TABLE_END __driver_init_table_end

#define EARLYCON_INIT_TABLE __earlycon_init_table
#define EARLYCON_INIT_TABLE_END __earlycon_init_table_end

#define IRQCHIP_INIT_TABLE __irqchip_init_table
#define IRQCHIP_INIT_TABLE_END __irqchip_init_table_end

#define TIMER_INIT_TABLE __timer_init_table
#define TIMER_INIT_TABLE_END __timer_init_table_end

#define BLOCK 0
#define NONBLOCK 1

struct device {
	int in_used;
	int probe;
	char name[64];
	unsigned long start;
	unsigned int len;
	unsigned int irq;
	struct driver *drv;
};

struct devices {
	struct device *p_devices;
	int total;
	int avail;
};

struct driver_ops {
	int (*write)(char *buf, unsigned long offset, unsigned int len);
	int (*read)(char *buf, unsigned long offset, unsigned int len,
		    int flag);
	int (*ioctl)(unsigned int cmd, void *arg);
};

struct driver {
	int in_used;
	int probe;
	char name[64];
	const struct driver_ops *ops;
	struct device *dev;
};

struct drivers {
	struct driver *p_drivers;
	int total;
	int avail;
};

struct device_init_entry {
	char compatible[128];
	unsigned long start;
	unsigned int len;
	unsigned int irq;
	void *data;
};

typedef int (*driver_init)(struct device * dev, void *data);

struct driver_init_entry {
	char compatible[128];
	driver_init init;
};

struct early_print_device {
	char name[128];
	void (*write)(char *str);
	int early_print_enable;
};

typedef int (*earlycon_init)(unsigned long base,
			     struct early_print_device * device);

struct earlycon_init_entry {
	char compatible[128];
	earlycon_init init;
};

typedef int (*irqchip_init)(unsigned long base, struct irq_domain * d,
			    void *priv);

struct irqchip_init_entry {
	char compatible[128];
	irqchip_init init;
};

typedef int (*timer_init)(unsigned long base, struct irq_domain * d,
			  void *priv);

struct timer_init_entry {
	char compatible[128];
	timer_init init;
};

#define DRIVER_REGISTER(name, init_fn, compat)                                \
	static const struct driver_init_entry __attribute__((used))           \
		__drvier_init_##name                                          \
		__attribute__((section(".driver_init_table"))) = {            \
			.compatible = compat,                                 \
			.init = init_fn,                                      \
		}

#define EARLYCON_REGISTER(name, init_fn, compat)                              \
	static const struct earlycon_init_entry __attribute__((used))         \
		__earlycon_entry_##name                                       \
		__attribute__((section(".earlycon_init_table"))) = {          \
			.compatible = compat,                                 \
			.init = init_fn,                                      \
		}

#define IRQCHIP_REGISTER(name, init_fn, compat)                               \
	static const struct irqchip_init_entry __attribute__((used))          \
		__irqchip_entry_##name                                        \
		__attribute__((section(".irqchip_init_table"))) = {           \
			.compatible = compat,                                 \
			.init = init_fn,                                      \
		}

#define TIMER_REGISTER(name, init_fn, compat)                                 \
	static const struct timer_init_entry __attribute__((used))            \
		__timer_entry_##name                                          \
		__attribute__((section(".timer_init_table"))) = {             \
			.compatible = compat,                                 \
			.init = init_fn,                                      \
		}

#define for_each_device(entry, devices, n)                                    \
	for (entry = (struct device *)devices; n > 0; entry++, n--)

#define for_each_driver(entry, drivers, n)                                    \
	for (entry = (struct driver *)drivers; n > 0; entry++, n--)

int device_driver_init(struct device_init_entry *hw);
int earlycon_driver_init(void);
int regist_device_irq(unsigned long hwirq, void (*handler)(void *data),
		      void *priv);
int open(char *name);
int write(int fd, char *buf, unsigned long offset, unsigned int len);
int read(int fd, char *buf, unsigned long offset, unsigned int len, int flag);
int ioctl(int fd, unsigned int cmd, void *arg);
void walk_devices(void);

unsigned long get_cycles(void);

#endif
