void local_flush_tlb_all(void)
{
	__asm__ __volatile__("sfence.vma":::"memory");
}

void local_flush_tlb_all_asid(unsigned long asid)
{
	__asm__ __volatile__("sfence.vma x0, %0"::"r"(asid)
			     :"memory");
}

void local_flush_tlb_page_asid(unsigned long addr, unsigned long asid)
{
	__asm__ __volatile__("sfence.vma %0, %1"::"r"(addr), "r"(asid)
			     :"memory");
}

void local_flush_tlb_page(unsigned long addr)
{
	__asm__ __volatile__("sfence.vma %0"::"r"(addr):"memory");
}

void local_flush_tlb_range(unsigned long start,
			   unsigned long size, unsigned long stride)
{
	if (size <= stride)
		local_flush_tlb_page(start);
	else
		local_flush_tlb_all();
}

void local_flush_tlb_range_asid(unsigned long start,
				unsigned long size, unsigned long stride,
				unsigned long asid)
{
	if (size <= stride)
		local_flush_tlb_page_asid(start, asid);
	else
		local_flush_tlb_all_asid(asid);
}
