#include "user.h"
#include "spinlocks.h"
#include "asm/pgtable.h"
#include "print.h"
#include "asm/type.h"

#define VMAP_START USER_SPACE_DYNAMIC_MMAP
#define VMAP_MAP_NR USER_SPACE_DYNAMIC_MAP_NR
#define VMAP_TOTAL_PAGE_NUM VMAP_MAP_NR * sizeof(unsigned long)

static DEFINE_SPINLOCK(vmem_lock);
unsigned long vmaps[VMAP_MAP_NR];

void *user_vmap_alloc(unsigned int size)
{
	unsigned long *vmem_maps = vmaps;
	int page_nr = N_PAGE(size);
	int index = 0, nr = 0;
	unsigned long mem_map;
	void *addr = (void *)VMAP_START;
	int per_mem_map = sizeof(vmem_maps[0]) * 8;

	spin_lock(&vmem_lock);
	while (index < VMAP_TOTAL_PAGE_NUM) {
		mem_map = vmem_maps[(index / per_mem_map)];
		if (((mem_map >> (index % per_mem_map)) & 0x1) == 0) {
			if (nr++ == page_nr)
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
	index = index - 1;
	for (index = index + 1 - page_nr; page_nr; index++, page_nr--) {
		mem_map = vmem_maps[(index / per_mem_map)];
		mem_map |= (1 << (index % per_mem_map));
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

	index = ((unsigned long)addr - VMAP_START) / PAGE_SIZE;
	if (index >= VMAP_TOTAL_PAGE_NUM)
		return;

	spin_lock(&vmem_lock);
	/* set bits in vmem_maps according to [addr, addr + size) to 0 */
	for (; page_nr; page_nr--, index++) {
		mem_map = vmem_maps[(index / per_mem_map)];
		mem_map &=
		    ~(unsigned long)(((unsigned long)1) <<
				     (index % per_mem_map));
		vmem_maps[(index / per_mem_map)] = mem_map;
	}
	spin_unlock(&vmem_lock);
}
