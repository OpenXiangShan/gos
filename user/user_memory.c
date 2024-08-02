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
#include "list.h"
#include "print.h"
#include "spinlocks.h"
#include "user.h"
#include "asm/type.h"
#include "asm/pgtable.h"
#include "user_vmap.h"
#include "user_memory.h"

static int user_memory_region_is_overlay(unsigned long start, unsigned long end,
					 unsigned long new_start,
					 unsigned long new_end)
{
	if (start == new_start || end == new_end)
		return 1;

	if (start > new_start && start < new_end)
		return 1;

	if (end > new_start && end < new_end)
		return 1;

	if (start < new_start && end > new_end)
		return 1;

	if (start > new_start && end < new_end)
		return 1;

	return 0;
}

struct user_memory_region *find_user_memory_region(struct user *user,
						   unsigned long va)
{
	struct user_memory_region *entry;

	spin_lock(&user->lock);
	list_for_each_entry(entry, &user->memory_region, list) {
		if (va >= entry->start && va <= entry->end) {
			spin_unlock(&user->lock);
			return entry;
		}
	}
	spin_unlock(&user->lock);

	return NULL;
}

int add_user_space_memory(struct user *user, unsigned long start,
			  unsigned int len)
{
	struct user_memory_region *region;
	struct user_memory_region *entry;

	region = mm_alloc(sizeof(struct user_memory_region));
	if (!region) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}

	region->start = start;
	region->end = start + len;

	spin_lock(&user->lock);
	list_for_each_entry(entry, &user->memory_region, list) {
		if (user_memory_region_is_overlay(entry->start, entry->end,
						  region->start, region->end)) {
			spin_unlock(&user->lock);
			mm_free(region, sizeof(struct user_memory_region));
			return -1;
		}
	}
	list_add_tail(&region->list, &user->memory_region);
	spin_unlock(&user->lock);

	return 0;
}

void *user_space_mmap(unsigned int size)
{
	int page_nr = N_PAGE(size);
	int alloc_size = page_nr * PAGE_SIZE;
	unsigned long va, pa, addr;

	va = (unsigned long)user_vmap_alloc(alloc_size);
	addr = (unsigned long)mm_alloc(alloc_size);
	if (!addr) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return NULL;
	}
	pa = virt_to_phy(addr);
	user_page_mapping(pa, va, alloc_size);

	return (void *)va;
}

void *user_space_mmap_pg(unsigned int size, pgprot_t pgprot)
{
	int page_nr = N_PAGE(size);
	int alloc_size = page_nr * PAGE_SIZE;
	unsigned long va, pa, addr;

	va = (unsigned long)user_vmap_alloc(alloc_size);
	addr = (unsigned long)mm_alloc(alloc_size);
	if (!addr) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return NULL;
	}
	pa = virt_to_phy(addr);
	user_page_mapping_pg(pa, va, alloc_size, pgprot);

	return (void *)va;
}

void user_space_unmap(void *addr, unsigned int size)
{
	void *phys;

	phys = walk_pt_va_to_pa((unsigned long)addr);
	if (!phys)
		return;

	user_vmap_free(addr, size);
	mm_free((void *)phy_to_virt((unsigned long)phys), size);
}
