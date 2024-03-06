#include "mm.h"
#include "string.h"

int pgtable_l4_enabled = 0;
int pgtable_l5_enabled = 0;

unsigned long alloc_zero_page(int gfp)
{
	void *ptr = mm_alloc(4096);

	if (!ptr)
		return 0;

	memset(ptr, 0, 4096);

	return (unsigned long)ptr;
}
