#include <mm.h>
#include <device.h>
#include <print.h>
#include <asm/type.h>
#include "spinlocks.h"
#include <string.h>

extern int mmu_is_on;
extern unsigned long bss_end;
extern unsigned long va_pa_offset;

/* each bit identifies a page, MEM_MAP_NR is the number of mem_maps*/
#define MEM_MAP_NR 8192		//2*1024*1024*1024/PAGE_SIZE/(sizeof(unsigned long)*8)
#define TOTAL_PAGE_NUM MEM_MAP_NR * sizeof(unsigned long)

static spinlock_t mem_lock __attribute__((section(".data"))) =
    __SPINLOCK_INITIALIZER;

static unsigned long mem_maps[MEM_MAP_NR];
static unsigned long phy_address_start;
static unsigned long phy_address_end;

static unsigned long get_ddr_end(struct device_init_entry *hw)
{
	struct device_init_entry *entry = hw;

	while (strncmp(entry->compatible, "THE END", sizeof("THE END"))) {
		if (!strncmp
		    (entry->compatible, "memory-map", sizeof("memory-map")))
			return entry->start + entry->len;

		entry++;
	}

	return NULL;
}

unsigned long get_phy_start(void)
{
	return phy_address_start;
}

unsigned long get_phy_end(void)
{
	return phy_address_end;
}

void mm_init(struct device_init_entry *hw)
{
	unsigned long start = PAGE_ALIGN((unsigned long)(&bss_end));
	unsigned long end = get_ddr_end(hw);
	unsigned long nr_free_pages = 0;

	memset((char *)mem_maps, 0, sizeof(unsigned long) * MEM_MAP_NR);
	end &= PAGE_MASK;
	phy_address_start = start;
	phy_address_end = end;

	while (start < end) {
		nr_free_pages++;
		start += PAGE_SIZE;
	}

	print
	    ("Available Memory: phy_start_address:0x%lx, phy_end_address:0x%lx, available size:%dKB, %d available pages, page_size:%d\n",
	     phy_address_start, phy_address_end,
	     (phy_address_end - phy_address_start) / 1024, nr_free_pages,
	     PAGE_SIZE);
}

void *mm_alloc(unsigned int size)
{
	int page_nr = N_PAGE(size);
	int index = 0, nr = 0;
	unsigned long mem_map;
	void *addr = (void *)phy_address_start;
	int per_mem_map = sizeof(mem_maps[0]) * 8;
	void *ret;

	spin_lock(&mem_lock);
	/* 
	 * Find free pages from mem_maps according to page_nr 
	 * index/per_mem_map indicates that the page represented by index is located in which mem_map of mem_maps  
	 * index%per_mem_map indicates that the page represented by index is locate in which bit of its mem_map
	 * If can find page_nr continuous bits in mem_maps, goto success and the addr is the start address of according continues this bits.
	 */
	while (index < TOTAL_PAGE_NUM) {
		mem_map = mem_maps[(index / per_mem_map)];
		if (((mem_map >> (index % per_mem_map)) & (1UL)) == 0) {
			if (++nr == page_nr)
				goto success;
		} else {
			nr = 0;
			addr += PAGE_SIZE;
		}

		index++;
	}

	spin_unlock(&mem_lock);
	print("out of memory!!\n");

	return NULL;

success:
	/* 
	 * Set founded page_nr continues bits to 1 in mem_maps
	 */
	for (index = index + 1 - page_nr; page_nr; index++, page_nr--) {
		mem_map = mem_maps[(index / per_mem_map)];
		mem_map |= (1UL << (index % per_mem_map));
		mem_maps[(index / per_mem_map)] = mem_map;
	}

	spin_unlock(&mem_lock);

	if (!mmu_is_on)
		ret = addr;
	else
		ret = (void *)((unsigned long)addr + va_pa_offset);

	return ret;
}

void mm_free(void *addr, unsigned int size)
{
	unsigned long index;
	unsigned long mem_map;
	int per_mem_map = sizeof(mem_maps[0]) * 8;
	int page_nr = N_PAGE(size);

	if (mmu_is_on)
		addr = addr - va_pa_offset;

	index = ((unsigned long)addr - phy_address_start) / PAGE_SIZE;
	if (index >= TOTAL_PAGE_NUM)
		return;

	spin_lock(&mem_lock);
	/* set bits in mem_maps according to [addr, addr + size) to 0 */
	for (; page_nr; page_nr--, index++) {
		mem_map = mem_maps[(index / per_mem_map)];
		mem_map &= ~(unsigned long)((1UL) << (index % per_mem_map));
		mem_maps[(index / per_mem_map)] = mem_map;
	}
	spin_unlock(&mem_lock);
}
