#ifndef __GUEST_DEVICE_H
#define __GUEST_DEVICE_H

#define DRIVER_INIT_TABLE __driver_init_table
#define DRIVER_INIT_TABLE_END __driver_init_table_end

struct device_init_entry {
	char compatible[128];
	unsigned long start;
	unsigned int len;
	char irq_parent[128];
	int irq[16];
	int irq_num;
	int dev_id;
	void *data;
};

typedef int (*driver_init)(unsigned long base, unsigned int len, void *data);

struct driver_init_entry {
	char compatible[128];
	driver_init init;
};

void create_devices(void);

#define DRIVER_REGISTER(name, init_fn, compat)                                \
	static const struct driver_init_entry __attribute__((used))           \
		__drvier_init_##name                                          \
		__attribute__((section(".driver_init_table"))) = {            \
			.compatible = compat,                                 \
			.init = init_fn,                                      \
		}

#endif
