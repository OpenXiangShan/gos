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

#ifndef __TLB_FLUSH_H__
#define __TLB_FLUSH_H__

static inline void local_flush_tlb_all(void)
{
	__asm__ __volatile__("sfence.vma":::"memory");
}

static inline void local_flush_tlb_all_asid(unsigned long asid)
{
	__asm__ __volatile__("sfence.vma x0, %0"::"r"(asid)
			     :"memory");
}

static inline void local_flush_tlb_page_asid(unsigned long addr, unsigned long asid)
{
	__asm__ __volatile__("sfence.vma %0, %1"::"r"(addr), "r"(asid)
			     :"memory");
}

static inline void local_flush_tlb_page(unsigned long addr)
{
	__asm__ __volatile__("sfence.vma %0"::"r"(addr):"memory");
}

static inline void local_flush_tlb_range(unsigned long start,
			   unsigned long size, unsigned long stride)
{
	if (size <= stride)
		local_flush_tlb_page(start);
	else
		local_flush_tlb_all();
}

static inline void local_flush_tlb_range_asid(unsigned long start,
				unsigned long size, unsigned long stride,
				unsigned long asid)
{
	if (size <= stride)
		local_flush_tlb_page_asid(start, asid);
	else
		local_flush_tlb_all_asid(asid);
}

void sinval_all(void);
void sinval_va(unsigned long va);
void sinval_asid(unsigned long asid);
void sinval_va_asid(unsigned long va, unsigned long asid);
#endif
