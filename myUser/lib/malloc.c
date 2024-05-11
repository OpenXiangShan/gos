#include "syscall.h"
#include "../asm/pgtable.h"

void *malloc(unsigned int size)
{
	unsigned long buf = syscall(__NR_mmap, size);

	return (void *)buf;
}

void *malloc_pte(unsigned int size, pgprot_t pgprot)
{
	unsigned long buf = syscall(__NR_mmap_pg, size, pgprot);

	return (void *)buf;
}

void free(void *addr, unsigned int size)
{
	syscall(__NR_unmap, addr, size);
}
