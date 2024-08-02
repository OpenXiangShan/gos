/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

int gstage_page_mapping_2M(unsigned long *pgdp, unsigned long hpa,
			   unsigned long gpa, unsigned int size)
{
	pgprot_t pgprot;

	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC |
		     _PAGE_DIRTY | _PAGE_USER);

	return mmu_gstage_page_mapping_2M(pgdp, hpa, gpa, size, pgprot);
}

int gstage_page_mapping_1G(unsigned long *pgdp, unsigned long hpa,
			   unsigned long gpa, unsigned int size)
{
	pgprot_t pgprot;

	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC |
		     _PAGE_DIRTY | _PAGE_USER);

	return mmu_gstage_page_mapping_1G(pgdp, hpa, gpa, size, pgprot);
}
