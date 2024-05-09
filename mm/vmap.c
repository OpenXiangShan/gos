#include <asm/type.h>
#include <asm/pgtable.h>
#include <asm/barrier.h>
#include "spinlocks.h"
#include "mm.h"
#include "print.h"
#include "string.h"

#define VMAP_START 0xffffffc800000000

#define VMAP_MAP_NR 8192	//2*1024*1024*1024/PAGE_SIZE/(sizeof(unsigned long)*8)
#define VMAP_TOTAL_PAGE_NUM VMAP_MAP_NR * sizeof(unsigned long)

static spinlock_t vmem_lock __attribute__((section(".data"))) =
    __SPINLOCK_INITIALIZER;
static unsigned long vmem_maps[VMAP_MAP_NR] __attribute__((section(".data"))) =
    { 0 };

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
			addr += PAGE_SIZE;
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

void *ioremap(void *addr, unsigned int size, int gfp)
{
	pgprot_t pgprot;
	unsigned long phys = (unsigned long)addr;
	unsigned long virt;

	virt = (unsigned long)vmap_alloc(size);
	if (!virt) {
		print("%s -- vmap out of memory!\n", __FUNCTION__);
		return NULL;
	}

	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);

	if (-1 == mmu_page_mapping(phys, virt, size, pgprot)) {
		print("%s -- page mapping failed\n", __FUNCTION__);
		return NULL;
	}

	__asm__ __volatile("sfence.vma":::"memory");
	return (void *)virt;
}

void iounmap(void *addr, unsigned int size)
{
	void *phys;

	phys = walk_pt_va_to_pa((unsigned long)addr);
	if (!phys)
		return;

	vmap_free(addr, size);
	mm_free((void *)phy_to_virt((unsigned long)phys), size);
}

void *vmem_map(void *addr, unsigned int size, int gfp)
{
	return ioremap(addr, size, gfp);
}

void *vmem_alloc(unsigned int size, int gfp)
{
	int page_nr = N_PAGE(size);
	unsigned long vmap_addr, phys_addr;
	pgprot_t pgprot;

	vmap_addr = (unsigned long)vmap_alloc(size);
	if (!vmap_addr) {
		print("%s -- vmap out of memory!\n", __FUNCTION__);
		return NULL;
	}

	pgprot = __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY);
	while (page_nr--) {
		phys_addr = (unsigned long)mm_alloc(PAGE_SIZE);
		if (-1 ==
		    mmu_page_mapping(phys_addr, vmap_addr, PAGE_SIZE, pgprot)) {
			print("%s -- page mapping failed\n", __FUNCTION__);
			vmap_free((void *)vmap_addr, size);
			return NULL;
		}
		vmap_addr += PAGE_SIZE;
	}

	return (void *)vmap_addr;
}

void vmem_free(void *addr, unsigned int size)
{
	return iounmap(addr, size);
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
