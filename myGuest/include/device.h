#ifndef __GUEST_DEVICE_H
#define __GUEST_DEVICE_H

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

void create_devices(void);

#endif
