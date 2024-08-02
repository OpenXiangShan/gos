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

#ifndef _MM_H
#define _MM_H

#include <device.h>
#include <asm/pgtable.h>

int paging_init(struct device_init_entry *hw);
void enable_mmu(int on);
int mmu_page_mapping(unsigned long phy, unsigned long virt, unsigned int size,
                    pgprot_t pgprot);
int mmu_page_mapping_2M(unsigned long phy, unsigned long virt, unsigned int size,
		        pgprot_t pgprot);
int mmu_page_mapping_1G(unsigned long phy, unsigned long virt, unsigned int size,
		        pgprot_t pgprot);
int mmu_user_page_mapping(unsigned long phy, unsigned long virt,
                         unsigned int size, pgprot_t pgprot);
int mmu_gstage_page_mapping(unsigned long *_pgdp, unsigned long phy,
                           unsigned long virt, unsigned int size,
                           pgprot_t pgprot);
void *walk_pt_va_to_pa(unsigned long va);
void mm_init(struct device_init_entry *hw);
void *mm_alloc(unsigned int size);
void mm_free(void *addr, unsigned int size);
unsigned long alloc_zero_page(int gfp);
unsigned long get_phy_start(void);
unsigned long get_phy_end(void);
void mmu_walk_and_print_pte(unsigned long virt_addr);

static inline void dump_fault_addr_pt(unsigned long addr)
{
	return mmu_walk_and_print_pte(addr);
}

#endif

