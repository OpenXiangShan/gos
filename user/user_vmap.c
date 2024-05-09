#include "user.h"
#include "spinlocks.h"
#include "asm/pgtable.h"
#include "print.h"
#include "asm/type.h"

#define USER_MAP_START USER_SPACE_DYNAMIC_MMAP
#define USER_MAP_MAP_NR USER_SPACE_DYNAMIC_MAP_NR
#define USER_MAP_TOTAL_PAGE_NUM USER_MAP_MAP_NR * sizeof(unsigned long)

static spinlock_t vmem_lock __attribute__((section(".data"))) =
    __SPINLOCK_INITIALIZER;
unsigned long vmaps[USER_MAP_MAP_NR] __attribute__((section(".data"))) = { 0 };

void *user_vmap_alloc(unsigned int size)
{
	unsigned long *vmem_maps = vmaps;
	int page_nr = N_PAGE(size);
	int index = 0, nr = 0;
	unsigned long mem_map;
	void *addr = (void *)USER_MAP_START;
	int per_mem_map = sizeof(vmem_maps[0]) * 8;

	spin_lock(&vmem_lock);
	while (index < USER_MAP_TOTAL_PAGE_NUM) {
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

void user_vmap_free(void *addr, unsigned int size)
{
	unsigned long *vmem_maps = vmaps;
	unsigned long index;
	unsigned long mem_map;
	int per_mem_map = sizeof(vmem_maps[0]) * 8;
	int page_nr = N_PAGE(size);

	index = ((unsigned long)addr - USER_MAP_START) / PAGE_SIZE;
	if (index >= USER_MAP_TOTAL_PAGE_NUM)
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
