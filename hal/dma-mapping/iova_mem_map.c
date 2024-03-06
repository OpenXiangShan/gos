#include <mm.h>
#include <asm/type.h>
#include <print.h>
#include "device.h"
#include "dma_mapping.h"

static unsigned long iova_address_start = 0;

void *iova_alloc(struct device *dev, unsigned int size)
{
	struct iommu_group *group = dev->iommu.group;
	int page_nr = N_PAGE(size);
	int index = 0, nr = 0;
	unsigned long mem_map;
	void *addr = (void *)iova_address_start;
	unsigned long *maps;
	int per_mem_map = sizeof(maps[0]) * 8;

	if (!group){
		print("%s -- find iommu group failed\n", __FUNCTION__);
		return NULL;
	}
	maps = group->iova_mem_maps;	

	/* 
	 * Find free pages from maps according to page_nr 
	 * index/per_mem_map indicates that the page represented by index is located in which mem_map of maps  
	 * index%per_mem_map indicates that the page represented by index is locate in which bit of its mem_map
	 * If can find page_nr continuous bits in maps, goto success and the addr is the start address of according continues this bits.
	 */
	while (index < TOTAL_IOVA_PAGE_NUM) {
		mem_map = maps[(index / per_mem_map)];
		if (((mem_map >> (index % per_mem_map)) & 0x1) == 0) {
			if (nr++ == page_nr)
				goto success;
		} else {
			nr = 0;
			addr += PAGE_SIZE;
		}

		index++;
	}

	print("out of memory!!\n");

	return (void *)U64_MAX;

success:
	/* 
	 * Set founded page_nr continues bits to 1 in mem_maps
	 * n is the last index of allocated pages, it need to -1 because it added 1 in end of while
	 */
	index = index - 1;
	for (index = index + 1 - page_nr; page_nr; index++, page_nr--) {
		mem_map = maps[(index / per_mem_map)];
		mem_map |= (1 << (index % per_mem_map));
		maps[(index / per_mem_map)] = mem_map;
	}
	
	return addr;	
}

void iova_free(struct device *dev, void *addr, unsigned int size)
{
	struct iommu_group *group = dev->iommu.group;
	unsigned long index;
	unsigned long mem_map;
	unsigned long *maps; 
	int per_mem_map = sizeof(maps[0]) * 8;
	int page_nr = N_PAGE(size);
	
	if (!group){
		print("%s -- find iommu group failed\n", __FUNCTION__);
		return;
	}
	maps = group->iova_mem_maps;	

	index = ((unsigned long)addr - iova_address_start) / PAGE_SIZE;
	if (index >= TOTAL_IOVA_PAGE_NUM)
		return;

	/* set bits in maps according to [addr, addr + size) to 0 */
	for (; page_nr; page_nr--, index++) {
		mem_map = maps[(index / per_mem_map)];
		mem_map &=
		    ~(unsigned long)(((unsigned long)1) <<
				     (index % per_mem_map));
		maps[(index / per_mem_map)] = mem_map;
	}
}
