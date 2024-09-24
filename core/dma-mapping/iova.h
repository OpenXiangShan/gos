#ifndef __IOVA_H__
#define __IOVA_H__
#include "device.h"

struct iova {
	struct list_head list;
	unsigned long base;
	unsigned int size;
};

unsigned long iova_alloc(struct list_head *iovad, int len);
void iova_free(struct list_head *iovad, unsigned long addr);

#endif
