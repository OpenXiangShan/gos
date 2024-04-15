#include <mm.h>
#include <device.h>
#include <string.h>
#include <print.h>
#include <asm/type.h>
#include "spinlocks.h"
#include "devicetree.h"

extern int mmu_is_on;
extern unsigned long bss_end;
extern unsigned long va_pa_offset;

static DEFINE_SPINLOCK(mem_lock);

static struct memory_block mm_blocks = { 0 };

extern char dtb_bin[];

unsigned long get_phy_start(void)
{
	return mm_blocks.memory_block_start[0];
}

unsigned long get_phy_end(void)
{
	int last = mm_blocks.avail - 1;

	return (mm_blocks.memory_block_size[last] +
		mm_blocks.memory_block_start[last]);
}

void memory_block_add(unsigned long base, unsigned long size)
{
	unsigned long i = 0;
	unsigned long ddr_start = PAGE_ALIGN((unsigned long)(&bss_end));

	if (mm_blocks.avail >= MAX_MEM_BLOCK_COUNT)
		return;

	if (base + size < ddr_start)
		return;

	if (base < ddr_start) {
		size -= ddr_start - base;
		base = ddr_start;
	}

	while (i < size) {
		mm_blocks.memory_block_start[mm_blocks.avail] = base + i;
		if (size - i > MAX_BYTE_PER_MAPS)
			mm_blocks.memory_block_size[mm_blocks.avail] =
			    MAX_BYTE_PER_MAPS;
		else
			mm_blocks.memory_block_size[mm_blocks.avail] = size - i;
		mm_blocks.maps[mm_blocks.avail].map_nr =
		    mm_blocks.memory_block_size[mm_blocks.avail] /
		    sizeof(unsigned long);
		mm_blocks.avail++;

		i += MAX_BYTE_PER_MAPS;
	}
}

void mm_init(struct device_init_entry *hw)
{
	int i, n;
	struct mem_maps *maps;

	memset((char *)&mm_blocks, 0, sizeof(struct memory_block));
	mm_blocks.max_byte_per_maps = MAX_BYTE_PER_MAPS;

	dtb_scan_memory((void *)dtb_bin, memory_block_add);

	n = mm_blocks.avail;
	for (i = 0; i < n; i++) {
		unsigned long start;
		unsigned int size;
		unsigned long tmp = 0;
		unsigned long nr_free_pages = 0;

		start = mm_blocks.memory_block_start[i];
		size = mm_blocks.memory_block_size[i];

		tmp = start;
		while (tmp < start + size) {
			nr_free_pages++;
			tmp += PAGE_SIZE;
		}
		maps = &mm_blocks.maps[i];

		memset((char *)maps->maps, 0, sizeof(maps->maps));

		print
		    ("Available Memory: phy_start_address:0x%lx, phy_end_address:0x%lx, available size:%dKB, %d available pages, page_size:%d\n",
		     mm_blocks.memory_block_start[i],
		     mm_blocks.memory_block_start[i] +
		     mm_blocks.memory_block_size[i],
		     mm_blocks.memory_block_size[i] / 1024, nr_free_pages,
		     PAGE_SIZE);
	}
}

void *mm_alloc(unsigned int size)
{
	int page_nr = N_PAGE(size), n, i;
	int index = 0, nr = 0;
	void *ret;
	struct mem_maps *mem_maps;
	unsigned long mem_map;
	int per_mem_map;
	void *addr;

	n = mm_blocks.avail;
	for (i = 0; i < n; i++) {
		int total;

		index = 0;
		nr = 0;
		mem_maps = &mm_blocks.maps[i];

		total = mem_maps->map_nr;
		per_mem_map = sizeof(mem_maps->maps[0]) * 8;
		addr = (void *)mm_blocks.memory_block_start[i];

		/* 
		 * Find free pages from mem_maps according to page_nr 
		 * index/per_mem_map indicates that the page represented by index is located in which mem_map of mem_maps  
		 * index%per_mem_map indicates that the page represented by index is locate in which bit of its mem_map
		 * If can find page_nr continuous bits in mem_maps, goto success and the addr is the start address of according continues this bits.
		 */
		spin_lock(&mem_lock);
		while (index < total) {
			mem_map = mem_maps->maps[(index / per_mem_map)];
			if (((mem_map >> (index % per_mem_map)) & 0x1) == 0) {
				if (nr++ == page_nr)
					goto success;
			} else {
				nr = 0;
				addr += PAGE_SIZE;
			}

			index++;
		}

		spin_unlock(&mem_lock);
	}
	print("out of memory!!\n");

	return NULL;

success:
	/* 
	 * Set founded page_nr continues bits to 1 in mem_maps
	 * n is the last index of allocated pages, it need to -1 because it added 1 in end of while
	 */
	index = index - 1;
	for (index = index + 1 - page_nr; page_nr; index++, page_nr--) {
		per_mem_map = sizeof(mem_maps->maps[0]) * 8;
		mem_map = mem_maps->maps[(index / per_mem_map)];
		mem_map |= (1 << (index % per_mem_map));
		mem_maps->maps[(index / per_mem_map)] = mem_map;
	}

	spin_unlock(&mem_lock);

	if (!mmu_is_on)
		ret = addr;
	else
		ret = (void *)((unsigned long)addr + va_pa_offset);

	return ret;
}

struct memory_block *get_mm_blocks()
{
	return &mm_blocks;
}

void mm_free(void *addr, unsigned int size)
{
	int page_nr = N_PAGE(size), n, total, i;
	unsigned long phy_address_start;
	struct mem_maps *mem_maps;
	unsigned long index;
	unsigned long mem_map;
	int per_mem_map;

	if (mmu_is_on)
		addr = addr - va_pa_offset;

	n = mm_blocks.avail;

	for (i = 0; i < n; i++) {
		unsigned long start;
		unsigned int size;

		start = mm_blocks.memory_block_start[i];
		size = mm_blocks.memory_block_size[i];

		if (!
		    ((unsigned long)addr >= start
		     && (unsigned long)addr < start + size))
			continue;

		mem_maps = &mm_blocks.maps[i];
		per_mem_map = sizeof(mem_maps->maps[0]) * 8;
		total = mem_maps->map_nr;

		phy_address_start = start;
		goto release;
	}

	return;

release:
	index = ((unsigned long)addr - phy_address_start) / PAGE_SIZE;
	if (index >= total)
		return;

	spin_lock(&mem_lock);
	/* set bits in mem_maps according to [addr, addr + size) to 0 */
	for (; page_nr; page_nr--, index++) {
		mem_map = mem_maps->maps[(index / per_mem_map)];
		mem_map &=
		    ~(unsigned long)(((unsigned long)1) <<
				     (index % per_mem_map));
		mem_maps->maps[(index / per_mem_map)] = mem_map;
	}
	spin_unlock(&mem_lock);
}
