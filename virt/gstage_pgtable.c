#include "mm.h"
#include "asm/pgtable.h"

int gstage_page_mapping(unsigned long *pgdp, unsigned long hpa,
			unsigned long gpa, unsigned int size)
{
	pgprot_t pgprot;

	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC |
		     _PAGE_DIRTY | _PAGE_USER);

	return mmu_gstage_page_mapping(pgdp, hpa, gpa, size, pgprot);
}
