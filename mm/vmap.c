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

#include <asm/type.h>
#include <asm/pgtable.h>
#include <asm/barrier.h>
#include "spinlocks.h"
#include "mm.h"
#include "print.h"
#include "string.h"
#include "align.h"
#include "gos.h"

#define VMAP_START 0xffffffc800000000

#define VMAP_MAP_NR 8192UL	//2*1024*1024*1024/PAGE_SIZE/(sizeof(unsigned long)*8)
#define VMAP_TOTAL_PAGE_NUM VMAP_MAP_NR * sizeof(unsigned long) * 8

extern int mmu_is_on;
static spinlock_t vmem_lock = __SPINLOCK_INITIALIZER;
static unsigned long vmem_maps[VMAP_MAP_NR] = { 0 };

void *vmap_alloc(unsigned int size)
{
	int page_nr = N_PAGE(size);
	int index = 0, nr = 0;
	unsigned long mem_map;
	void *addr = (void *)VMAP_START;
	int per_mem_map = sizeof(vmem_maps[0]) * 8;

	if (size == 0)
		return NULL;

	spin_lock(&vmem_lock);
	while (index < VMAP_TOTAL_PAGE_NUM) {
		mem_map = vmem_maps[(index / per_mem_map)];
		if (((mem_map >> (index % per_mem_map)) & 0x1) == 0) {
			if (++nr == page_nr)
				goto success;
		} else {
			nr = 0;
			addr += (nr + 1) * PAGE_SIZE;
			index = ((unsigned long)addr - VMAP_START) / PAGE_SIZE;
			continue;
		}

		index++;
	}

	spin_unlock(&vmem_lock);
	print("vmem -- out of memory!!\n");

	return NULL;

success:
	for (index = index + 1 - page_nr; page_nr; index++, page_nr--) {
		mem_map = vmem_maps[(index / per_mem_map)];
		mem_map |= (1UL << (index % per_mem_map));
		vmem_maps[(index / per_mem_map)] = mem_map;
	}

	spin_unlock(&vmem_lock);

	return addr;
}

void *vmap_alloc_align(unsigned long align, unsigned int size)
{
	int page_nr = N_PAGE(size);
	int per_mem_map = sizeof(vmem_maps[0]) * 8;
	unsigned long mem_map;
	unsigned long addr = VMAP_START;
	unsigned long align_addr;
	int index = 0, nr = 0;

	if (size == 0)
		return NULL;

	if (align <= PAGE_SIZE)
		return vmap_alloc(size);

	align = align / PAGE_SIZE * PAGE_SIZE;
	align_addr = ALIGN_SIZE(addr, align);
	index = (align_addr - addr) / PAGE_SIZE;

	spin_lock(&vmem_lock);
	while (index < VMAP_TOTAL_PAGE_NUM) {
		mem_map = vmem_maps[(index / per_mem_map)];
		if (((mem_map >> (index % per_mem_map)) & 0x1) == 0) {
			if (++nr == page_nr)
				goto success;
		} else {
			nr = 0;
			align_addr += align;
			index = (align_addr - addr) / PAGE_SIZE;
			continue;
		}

		index++;
	}
	spin_unlock(&vmem_lock);
	print("%s -- out of memory!!\n", __FUNCTION__);

	return NULL;

success:
	for (index = index + 1 - page_nr; page_nr; index++, page_nr--) {
		mem_map = vmem_maps[(index / per_mem_map)];
		mem_map |= (1UL << (index % per_mem_map));
		vmem_maps[(index / per_mem_map)] = mem_map;
	}
	spin_unlock(&vmem_lock);

	return (void *)align_addr;
}

void vmap_free(void *addr, unsigned int size)
{
	unsigned long index;
	unsigned long mem_map;
	int per_mem_map = sizeof(vmem_maps[0]) * 8;
	int page_nr = N_PAGE(size);

	index = ((unsigned long)addr - VMAP_START) / PAGE_SIZE;
	if (index >= VMAP_TOTAL_PAGE_NUM)
		return;

	spin_lock(&vmem_lock);
	/* set bits in vmem_maps according to [addr, addr + size) to 0 */
	for (; page_nr; page_nr--, index++) {
		mem_map = vmem_maps[(index / per_mem_map)];
		mem_map &=
		    ~(unsigned long)(((unsigned long)(1UL)) <<
				     (index % per_mem_map));
		vmem_maps[(index / per_mem_map)] = mem_map;
	}
	spin_unlock(&vmem_lock);
}


static void *__ioremap(void *addr, unsigned int size, int align_size, int gfp)
{
	pgprot_t pgprot;
	unsigned long phys = (unsigned long)addr;
	unsigned long virt;

	if (!mmu_is_on)
		return addr;

	if (!IS_ALIGN(phys, align_size))
		return NULL;

	virt = (unsigned long)vmap_alloc_align(align_size, RESIZE(size, align_size));
	if (!virt) {
		print("%s -- vmap out of memory!\n", __FUNCTION__);
		return NULL;
	}

	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	if (align_size == PAGE_SIZE) {
		if (-1 == mmu_page_mapping(phys, virt, size, pgprot)) {
			print("%s -- page mapping failed\n", __FUNCTION__);
			return NULL;
		}
	} else if (align_size == PAGE_2M_SIZE) {
		if (-1 == mmu_page_mapping_2M(phys, virt, size, pgprot)) {
			print("%s -- page mapping failed\n", __FUNCTION__);
			return NULL;
		}
	} else if (align_size == PAGE_1G_SIZE) {
		if (-1 == mmu_page_mapping_1G(phys, virt, size, pgprot)) {
			print("%s -- page mapping failed\n", __FUNCTION__);
			return NULL;
		}
	} else {
		if (-1 == mmu_page_mapping(phys, virt, size, pgprot)) {
			print("%s -- page mapping failed\n", __FUNCTION__);
			return NULL;
		}
	}

	virt |= ((unsigned long)addr) & (align_size - 1);

	return (void *)virt;
}

void *ioremap(void *addr, unsigned int size, int gfp)
{
	return __ioremap(addr, size, PAGE_SIZE, gfp);
}

void *ioremap_2M(void *addr, unsigned int size, int gfp)
{
	return __ioremap(addr, size, PAGE_2M_SIZE, gfp);
}

void *ioremap_1G(void *addr, unsigned int size, int gfp)
{
	return __ioremap(addr, size, PAGE_1G_SIZE, gfp);
}

static void vmap_cancel_mapping(void* addr)
{
	unsigned long *pte;

	pte = mmu_get_pte((unsigned long)addr);
	*pte = 0;
}

void iounmap(void *addr, unsigned int size)
{
	void *phys;

	if (mmu_is_on) {
		phys = walk_pt_va_to_pa((unsigned long)addr);
		if (!phys)
			return;
		vmap_free(addr, size);
		mm_free((void *)phy_to_virt((unsigned long)phys), size);
		vmap_cancel_mapping(addr);
	}
	else
		mm_free(addr, size);
}

void *vmem_map(void *addr, unsigned int size, int gfp)
{
	return ioremap(addr, size, gfp);
}

static void* __vmem_alloc(unsigned int size, int page_size, int gfp)
{
	int page_nr = N_PAGE_EXT(size, page_size);
	unsigned long vmap_addr, phys_addr, vmap_addr_tmp;
	pgprot_t pgprot;

	vmap_addr = (unsigned long)vmap_alloc_align(page_size, size);
	if (!vmap_addr) {
		print("%s -- vmap out of memory!\n", __FUNCTION__);
		return NULL;
	}

	vmap_addr_tmp = vmap_addr;
	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);
	if (gfp & GFP_NOCACHE)
		pgprot_val(pgprot) |=  _PAGE_SVPBMT_NOCACHE;
	if (gfp & GFP_IO)
		pgprot_val(pgprot) |=  _PAGE_SVPBMT_IO;

	while (page_nr--) {
		if (!mmu_is_on)
			phys_addr = (unsigned long)mm_alloc_align(page_size, page_size);
		else
			phys_addr = virt_to_phy((unsigned long)mm_alloc_align(page_size, page_size));

		if (page_size == PAGE_SIZE) {
			if (-1 == mmu_page_mapping(phys_addr, vmap_addr_tmp, PAGE_SIZE, pgprot)) {
				print("%s -- page mapping failed\n", __FUNCTION__);
				vmap_free((void *)vmap_addr, size);
				return NULL;
			}
			vmap_addr_tmp += PAGE_SIZE;
		}
		else if (page_size == PAGE_2M_SIZE) {
			if (-1 == mmu_page_mapping_2M(phys_addr, vmap_addr_tmp, PAGE_2M_SIZE, pgprot)) {
				print("%s -- page mapping failed\n", __FUNCTION__);
				vmap_free((void *)vmap_addr, size);
				return NULL;
			}
			vmap_addr_tmp += PAGE_2M_SIZE;
		}
		else if (page_size == PAGE_1G_SIZE) {
			if (-1 == mmu_page_mapping_1G(phys_addr, vmap_addr_tmp, PAGE_1G_SIZE, pgprot)) {
				print("%s -- page mapping failed\n", __FUNCTION__);
				vmap_free((void *)vmap_addr, size);
				return NULL;
			}
			vmap_addr_tmp += PAGE_1G_SIZE;
		}
#if CONFIG_SVNAPOT
		else if (page_size == PAGE_16K_SIZE) {
			if (-1 == mmu_page_mapping_16k(phys_addr, vmap_addr_tmp, PAGE_16K_SIZE, pgprot)) {
				print("%s -- page mapping failed\n", __FUNCTION__);
				vmap_free((void *)vmap_addr, size);
				return NULL;
			}
			vmap_addr_tmp += PAGE_16K_SIZE;
		}
		else if (page_size == PAGE_32K_SIZE) {
			if (-1 == mmu_page_mapping_32k(phys_addr, vmap_addr_tmp, PAGE_32K_SIZE, pgprot)) {
				print("%s -- page mapping failed\n", __FUNCTION__);
				vmap_free((void *)vmap_addr, size);
				return NULL;
			}
			vmap_addr_tmp += PAGE_32K_SIZE;
		}
		else if (page_size == PAGE_64K_SIZE) {
			if (-1 == mmu_page_mapping_64k(phys_addr, vmap_addr_tmp, PAGE_64K_SIZE, pgprot)) {
				print("%s -- page mapping failed\n", __FUNCTION__);
				vmap_free((void *)vmap_addr, size);
				return NULL;
			}
			vmap_addr_tmp += PAGE_32K_SIZE;
		}
#endif
		else{
			print("Unsupported page size\n");
			return NULL;
		}
	}

	return (void *)vmap_addr;
}

void *vmem_alloc(unsigned int size, int gfp)
{
	return __vmem_alloc(size, PAGE_SIZE, gfp);
}

void vmem_free(void *addr, unsigned int size)
{
	return iounmap(addr, size);
}

void *vmem_alloc_huge(unsigned int size, int page_size, int gfp)
{
	return __vmem_alloc(size, page_size, gfp);
}

void vmem_free_huge(void *addr, unsigned int size, int page_size)
{
	void *phys;

	if (mmu_is_on) {
		phys = walk_pt_va_to_pa_huge((unsigned long)addr, page_size);
		if (!phys)
			return;
		vmap_free(addr, size);
		mm_free((void *)phy_to_virt((unsigned long)phys), size);
		vmap_cancel_mapping(addr);
	}
	else
		mm_free(addr, size);
}

void *vmem_alloc_lazy(unsigned int size, int gfp)
{
	pgprot_t pgprot;
	unsigned long vmap_addr;

	vmap_addr = (unsigned long)vmap_alloc(size);
	if (!vmap_addr) {
		return NULL;
	}

	pgprot = __pgprot(_PAGE_READ | _PAGE_WRITE);

	mmu_page_mapping_lazy(vmap_addr, size, pgprot);

	return (void *)vmap_addr;
}
