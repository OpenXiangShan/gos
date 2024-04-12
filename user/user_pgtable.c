#include "mm.h"
#include "asm/pgtable.h"

int user_page_mapping(unsigned long phy, unsigned long virt, unsigned int size)
{
	pgprot_t pgprot;

	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC |
		     _PAGE_DIRTY | _PAGE_USER);

	return mmu_user_page_mapping(phy, virt, size, pgprot);
}
