#ifndef __USER_MALLOC_H
#define __USER_MALLOC_H
#include "../asm/pgtable.h"

void *malloc(unsigned int size);
void free(void *addr, unsigned int size);
void *malloc_pte(unsigned int size, pgprot_t pgprot);


#endif
