#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "list.h"
#include "device.h"

struct display_ops {
	int (*display)(unsigned long buf, int size, void *priv);
};

struct display_device {
	struct list_head list;
	char name[64];
	int idx;
	struct device *dev;
	struct display_ops *ops;
	void *priv;
	void *buf;
	unsigned long dma_addr;
	int size;
};

int register_display_device(struct display_device *dp);
int request_display_buffer(struct display_device *dp, int size);
int display(struct display_device *dp, void *buf);
struct display_device *get_display_device(char *name);

#endif
