#include "syscall.h"

void *malloc(unsigned int size)
{
	unsigned long buf = syscall(__NR_mmap, size);

	return (void *)buf;
}

void free(void *addr, unsigned int size)
{
	syscall(__NR_unmap, addr, size);
}
