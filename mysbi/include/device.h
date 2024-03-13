#ifndef DEVICE_H
#define DEVICE_H

#define DEVICE_INIT_TABLE __device_init_table
#define DEVICE_INIT_TABLE_END __device_init_table_end

struct device {
	unsigned long start;
	unsigned int len;
	unsigned int irq;
};

struct device_init_entry {
	char compatible[128];
	unsigned long start;
	unsigned int len;
	char irq_parent[128];
	unsigned int irq[16];
	int irq_num;
	int dev_id;
	void *data;
};

#endif
