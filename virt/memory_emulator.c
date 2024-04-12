#include "machine.h"
#include "print.h"

static void memory_write(struct memory_region *region,
			 unsigned long addr, unsigned long val,
			 unsigned int len)
{
	print("%s %d\n", __FUNCTION__, __LINE__);
}

static unsigned long memory_read(struct memory_region *region,
				 unsigned long addr, unsigned int len)
{
	print("%s %d\n", __FUNCTION__, __LINE__);

	return 0;
}

static const struct memory_region_ops memory_ops = {
	.write = memory_write,
	.read = memory_read,
};

int create_memory_device(struct virt_machine *machine, int id,
			 unsigned long base, unsigned int size)
{
	add_memory_region(machine, id, base, size, &memory_ops);

	return 0;
}

static void sram_write(struct memory_region *region, unsigned long addr,
		       unsigned long val, unsigned int len)
{
	print("%s %d\n", __FUNCTION__, __LINE__);
}

static unsigned long sram_read(struct memory_region *region,
			       unsigned long addr, unsigned int len)
{
	print("%s %d\n", __FUNCTION__, __LINE__);

	return 0;
}

static const struct memory_region_ops sram_ops = {
	.write = sram_write,
	.read = sram_read,
};

int create_sram_device(struct virt_machine *machine, int id, unsigned long base,
		       unsigned int size)
{
	add_memory_region(machine, id, base, size, &sram_ops);

	return 0;
}
