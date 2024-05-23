#ifndef __TINY_MM_H__
#define __TINY_MM_H__

#include "list.h"
#include "asm/pgtable.h"
#include "spinlocks.h"

struct tiny_meta {
	unsigned int unit;
	unsigned long bitmap;
	unsigned char total;
	unsigned char free;
};

struct tiny {
	struct list_head list;
	struct tiny_meta meta;
	void *objs;
	unsigned int size;
	char buffer[0];
};

void *tiny_alloc(unsigned int size);
void tiny_free(void *addr);
void tiny_init(void);

#endif
