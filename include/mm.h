#ifndef _MM_H
#define _MM_H

#include <device.h>
#include <asm/pgtable.h>

#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & PAGE_MASK)

#define N_PAGE(size)     size % PAGE_SIZE == 0 ? size/PAGE_SIZE : (size/PAGE_SIZE + 1)

void mm_init(struct device_init_entry *hw);
void *mm_alloc(unsigned int size);
void mm_free(void *addr, unsigned int size);
unsigned long alloc_zero_page(int gfp);

#endif
