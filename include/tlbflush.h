#ifndef __TLB_FLUSH_H__
#define __TLB_FLUSH_H__

void local_flush_tlb_all(void);
void local_flush_tlb_all_asid(unsigned long asid);
void local_flush_tlb_page_asid(unsigned long addr, unsigned long asid);
void local_flush_tlb_page(unsigned long addr);
void local_flush_tlb_range(unsigned long start,
			   unsigned long size, unsigned long stride);
void local_flush_tlb_range_asid(unsigned long start,
				unsigned long size, unsigned long stride,
				unsigned long asid);

#endif
