#include "device.h"
#include <asm/pgtable.h>
#include <asm/type.h>
#include "mm.h"
#include "vmap.h"
#include "print.h"

int page_tlb_test_init(unsigned long base, unsigned int len, void *data)
{
	pgprot_t pgprot;
	unsigned long va;

	va = (unsigned long)vmap_alloc(len);
	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	if (mmu_page_mapping(base, va, len, pgprot))
		return -1;

	print("%s\n", va);

	return 0;
}

DRIVER_REGISTER(page_tlb_test, page_tlb_test_init, "memory-test");
