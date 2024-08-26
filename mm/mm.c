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

#include <mm.h>
#include <device.h>
#include <string.h>
#include <print.h>
#include <asm/type.h>
#include "spinlocks.h"
#include "devicetree.h"
#include "vmap.h"
#include "tiny_mm.h"
#include "gos.h"

extern int mmu_is_on;
extern unsigned long bss_end;
extern unsigned long va_pa_offset;

static spinlock_t mem_lock __attribute__((section(".data"))) =
    __SPINLOCK_INITIALIZER;

static struct memory_block mm_blocks __attribute__((section(".data"))) = { 0 };

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
#if CONFIG_SELECT_4K_DIRECT_MAPPING
	unsigned long ddr_start = PAGE_ALIGN((unsigned long)(&bss_end));
#elif CONFIG_SELECT_2M_DIRECT_MAPPING
	unsigned long ddr_start = PAGE_ALIGN_2M((unsigned long)(&bss_end));
#elif CONFIG_SELECT_1G_DIRECT_MAPPING
	unsigned long ddr_start = PAGE_ALIGN_1G((unsigned long)(&bss_end));
#endif

	if (mm_blocks.avail >= MAX_MEM_BLOCK_COUNT)
		return;

	if (base + size < ddr_start)
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
		print("block size:0x%lx\n", mm_blocks.memory_block_size[mm_blocks.avail]);
		mm_blocks.maps[mm_blocks.avail].map_nr =
		    mm_blocks.memory_block_size[mm_blocks.avail] / PAGE_SIZE;
		mm_blocks.avail++;

		i += MAX_BYTE_PER_MAPS;
	}
}

static void mm_reserved(unsigned long base, unsigned long size)
{
	int i ,n;

	n = mm_blocks.avail;
	for (i = 0; i < n; i++) {
		unsigned long block_start;
		unsigned int block_size;
		unsigned long mem_map;
		int index, nr;
		int page_nr;
		struct mem_maps *mem_maps = &mm_blocks.maps[i];
		int per_mem_map = sizeof(mem_maps->maps[0]) * 8;

		page_nr = size % PAGE_SIZE == 0 ? size / PAGE_SIZE : size / PAGE_SIZE + 1;

		block_start = mm_blocks.memory_block_start[i];
		block_size = mm_blocks.memory_block_size[i];

		if (!(base >= block_start &&
		    (base + size <= block_start + block_size))) {
			if (size != 0)
				print("Invalid Reserved Memory params... Please check dts..\n");
			continue;
		}

		index = (base - block_start) / PAGE_SIZE;
		for (nr = 0; nr < page_nr; nr++, index++) {
			mem_map = mem_maps->maps[(index / per_mem_map)];
			mem_map |= (1UL << (index % per_mem_map));
			mem_maps->maps[(index / per_mem_map)] = mem_map;
		}

		mm_blocks.memory_block_resv_start[mm_blocks.reserved_cnt] = base;
		mm_blocks.memory_block_resv_size[mm_blocks.reserved_cnt] = page_nr * PAGE_SIZE;
		mm_blocks.reserved_cnt++;
	}
}

void mm_init(struct device_init_entry *hw)
{
	int i, n;
	struct mem_maps *maps;

	mm_blocks.avail = 0;
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

		memset((char *)maps->maps, 0, maps->map_nr / 8);

		print
		    ("Available Memory: phy_start_address:0x%lx, phy_end_address:0x%lx, available size:%dKB, %d available pages, page_size:%d\n",
		     mm_blocks.memory_block_start[i],
		     mm_blocks.memory_block_start[i] +
		     mm_blocks.memory_block_size[i],
		     mm_blocks.memory_block_size[i] / 1024, nr_free_pages,
		     PAGE_SIZE);
	}

	mm_blocks.reserved_cnt = 0;
	dtb_scan_reserved_memory((void *)dtb_bin, mm_reserved);

	print("Reserved Memory info --\n");
	for (i = 0; i < mm_blocks.reserved_cnt; i++)
		print("  start:0x%lx, size:0x%lx\n",
		      mm_blocks.memory_block_resv_start[i],
		      mm_blocks.memory_block_resv_size[i]);

#if CONFIG_TINY
	tiny_init();
#endif
}

void *mm_alloc_align(unsigned long align, unsigned int size)
{
	int n, i;
	int page_nr = N_PAGE(size);
	struct mem_maps *mem_maps;
	unsigned long index;
	unsigned long mem_map;
	unsigned long align_addr;
	int per_mem_map;
	void *ret;
	irq_flags_t flags;

	if (align <= PAGE_SIZE)
		return mm_alloc(size);

	align = align / PAGE_SIZE * PAGE_SIZE;

	n = mm_blocks.avail;

	spin_lock_irqsave(&mem_lock, flags);
	for (i = 0; i < n; i++) {
		unsigned long start;
		unsigned int len;
		int total;
		int nr = 0;

		start = mm_blocks.memory_block_start[i];
		len = mm_blocks.memory_block_size[i];
		align_addr = ALIGN_SIZE(start, align);

		if (!(align_addr >= start &&
			align_addr < start + len))
			continue;

		if (!(align_addr + size >= start &&
			align_addr < start + len))
			continue;

		mem_maps = &mm_blocks.maps[i];
		per_mem_map = sizeof(mem_maps->maps[0]) * 8;
		total = mem_maps->map_nr;
		index = ((unsigned long)align_addr - start) / PAGE_SIZE;
		if (index >= total)
			continue;

		while (index < total) {
			mem_map = mem_maps->maps[(index / per_mem_map)];
			if (((mem_map >> (index % per_mem_map)) & (1UL)) == 0) {
				if (++nr == page_nr)
					goto success;
			} else {
				nr = 0;
				align_addr += align;
				index = ((unsigned long)align_addr - start) / PAGE_SIZE;
				continue;
			}

			index++;
		}
	}
	spin_unlock_irqrestore(&mem_lock, flags);
	print("%s -- out of memory!!\n", __FUNCTION__);

	return NULL;

success:
	for (index = index + 1 - page_nr; page_nr; index++, page_nr--) {
		per_mem_map = sizeof(mem_maps->maps[0]) * 8;
		mem_map = mem_maps->maps[(index / per_mem_map)];
		mem_map |= (1UL << (index % per_mem_map));
		mem_maps->maps[(index / per_mem_map)] = mem_map;
	}
	spin_unlock_irqrestore(&mem_lock, flags);

	if (!mmu_is_on)
		ret = (void *)align_addr;
	else
		ret = (void *)((unsigned long)align_addr + va_pa_offset);

	return ret;
}

void *__mm_alloc(unsigned int size)
{
	int page_nr = N_PAGE(size), n, i;
	int index = 0, nr = 0;
	void *ret;
	struct mem_maps *mem_maps;
	unsigned long mem_map;
	int per_mem_map;
	void *addr;
	irq_flags_t flags;

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
		spin_lock_irqsave(&mem_lock, flags);
		while (index < total) {
			mem_map = mem_maps->maps[(index / per_mem_map)];
			if (((mem_map >> (index % per_mem_map)) & (1UL)) == 0) {
				if (++nr == page_nr)
					goto success;
			} else {
				nr = 0;
				addr += (nr + 1) * PAGE_SIZE;
			}

			index++;
		}

		spin_unlock_irqrestore(&mem_lock, flags);
	}
	print("out of memory!!\n");

	return NULL;

success:
	/* 
	 * Set founded page_nr continues bits to 1 in mem_maps
	 */
	for (index = index + 1 - page_nr; page_nr; index++, page_nr--) {
		per_mem_map = sizeof(mem_maps->maps[0]) * 8;
		mem_map = mem_maps->maps[(index / per_mem_map)];
		mem_map |= (1UL << (index % per_mem_map));
		mem_maps->maps[(index / per_mem_map)] = mem_map;
	}

	spin_unlock_irqrestore(&mem_lock, flags);

	if (!mmu_is_on)
		ret = addr;
	else
		ret = (void *)((unsigned long)addr + va_pa_offset);

	return ret;
}

void *mm_alloc_fix(unsigned long addr, unsigned int size)
{
	int page_nr = N_PAGE(size), n, total, i, tmp_nr;
	void *ret;
	unsigned long phy_address_start;
	struct mem_maps *mem_maps;
	unsigned long index, tmp_i;
	unsigned long mem_map;
	int per_mem_map;
	irq_flags_t flags;

	n = mm_blocks.avail;
	spin_lock_irqsave(&mem_lock, flags);
	for (i = 0; i < n; i++) {
		unsigned long start;
		unsigned int len;

		start = mm_blocks.memory_block_start[i];
		len = mm_blocks.memory_block_size[i];

		if (!((unsigned long)addr >= start
		    && (unsigned long)addr < start + len))
			continue;

		mem_maps = &mm_blocks.maps[i];
		per_mem_map = sizeof(mem_maps->maps[0]) * 8;
		total = mem_maps->map_nr;

		phy_address_start = start;
		goto check_and_alloc;
	}

	spin_unlock_irqrestore(&mem_lock, flags);
	return NULL;

check_and_alloc:
	index = ((unsigned long)addr - phy_address_start) / PAGE_SIZE;
	if (index >= total) {
		spin_unlock_irqrestore(&mem_lock, flags);
		return NULL;
	}

	tmp_i = index;
	tmp_nr = page_nr;
	for (; tmp_nr; tmp_nr--, tmp_i++) {
		mem_map = mem_maps->maps[(tmp_i / per_mem_map)];
		if (((mem_map >> (tmp_i % per_mem_map)) & (1UL)) != 0) {
			spin_unlock_irqrestore(&mem_lock, flags);
			return NULL;
		}
	}
	tmp_i = index;
	tmp_nr = page_nr;
	for (; tmp_nr; tmp_nr--, tmp_i++) {
		mem_map = mem_maps->maps[(tmp_i / per_mem_map)];
		mem_map |= (1UL << (tmp_i % per_mem_map));
		mem_maps->maps[(tmp_i / per_mem_map)] = mem_map;

	}
	spin_unlock_irqrestore(&mem_lock, flags);

	if (mmu_is_on)
		ret = (void *)phy_to_virt(addr);
	else
		ret = (void *)addr;

	return ret;
}

void *mm_alloc(unsigned int size)
{
#if CONFIG_TINY
	if (mmu_is_on) {
		if (size <= PAGE_SIZE / 2)
			return tiny_alloc(size);
	}
#endif
	return __mm_alloc(size);
}

struct memory_block *get_mm_blocks()
{
	return &mm_blocks;
}

void __mm_free(void *addr, unsigned int size)
{
	int page_nr = N_PAGE(size), n, total, i;
	unsigned long phy_address_start;
	struct mem_maps *mem_maps;
	unsigned long index;
	unsigned long mem_map;
	int per_mem_map;
	irq_flags_t flags;

	if (mmu_is_on)
		addr = addr - va_pa_offset;

	n = mm_blocks.avail;

	for (i = 0; i < n; i++) {
		unsigned long start;
		unsigned int len;

		start = mm_blocks.memory_block_start[i];
		len = mm_blocks.memory_block_size[i];

		if (!
		    ((unsigned long)addr >= start
		     && (unsigned long)addr < start + len))
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

	spin_lock_irqsave(&mem_lock, flags);
	/* set bits in mem_maps according to [addr, addr + size) to 0 */
	for (; page_nr; page_nr--, index++) {
		mem_map = mem_maps->maps[(index / per_mem_map)];
		mem_map &= ~(unsigned long)((1UL) << (index % per_mem_map));
		mem_maps->maps[(index / per_mem_map)] = mem_map;
	}
	spin_unlock_irqrestore(&mem_lock, flags);
}

void mm_free(void *addr, unsigned int size)
{
#if CONFIG_TINY
	if (mmu_is_on) {
		if (size <= PAGE_SIZE / 2) {
			tiny_free(addr);
			return;
		}
	}
#endif
	__mm_free(addr, size);
}

void reserved_mem_walk(void (*fn)(unsigned long addr, unsigned int nr, void *data), void *data)
{
	int i;

	for (i = 0; i < mm_blocks.reserved_cnt; i++) {
		fn(mm_blocks.memory_block_resv_start[i],
		   mm_blocks.memory_block_resv_size[i],
		   data);
	}
}

void unused_mem_walk(void (*fn)(unsigned long addr, unsigned int nr, void *data), void *data)
{
	int n, i;
	irq_flags_t flags;
	unsigned long mem_end;

	n = mm_blocks.avail;
	mem_end = get_phy_end();
	spin_lock_irqsave(&mem_lock, flags);
	for (i = 0; i < n; i++) {
		int total, nr = 0, index = 0;
		struct mem_maps *mem_maps;
		unsigned long mem_map;
		int per_mem_map;
		void *addr, *addr_walk;

		mem_maps = &mm_blocks.maps[i];

		total = mem_maps->map_nr;
		per_mem_map = sizeof(mem_maps->maps[0]) * 8;
		addr = addr_walk = (void *)mm_blocks.memory_block_start[i];

		while (index <= total) {
			mem_map = mem_maps->maps[(index / per_mem_map)];
			if ((((mem_map >> (index % per_mem_map)) & (1UL)) == 0)
			    && ((unsigned long)addr_walk < mem_end)) {
				nr++;
			} else {
				if (nr != 0)
					fn((unsigned long)addr, PAGE_SIZE * nr, data);
				nr = 0;
				addr += (nr + 1) * PAGE_SIZE;
			}

			addr_walk += PAGE_SIZE;
			index++;
		}
	}
	spin_unlock_irqrestore(&mem_lock, flags);
}

static void mem_range_contain(unsigned long addr, unsigned int size, void *data)
{
	struct mem_range_info{
		unsigned long addr;
		unsigned int size;
		int contain;
	};
	struct mem_range_info *mem = (struct mem_range_info *)data;

	if ((mem->addr >= addr) && ((mem->addr + mem->size) <= (addr + size)))
		mem->contain = 1;
}

int mem_range_is_free(unsigned long addr, unsigned int size)
{
	struct {
		unsigned long addr;
		unsigned int size;
		int contain;
	} tmp = { 0 };

	tmp.addr = addr;
	tmp.size = size;
	tmp.contain = 0;

	unused_mem_walk(mem_range_contain, &tmp);

	return tmp.contain;
}

static void print_unused_mem_info(unsigned long addr, unsigned int len, void *data)
{
	print("addr: 0x%lx len: 0x%x\n", addr, len);
}

void walk_unused_mem_and_print(void)
{
	unused_mem_walk(print_unused_mem_info, NULL);
}
