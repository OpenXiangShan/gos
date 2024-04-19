#include "machine.h"
#include "print.h"

static void memory_test_write(struct memory_region *region,
			      unsigned long addr, unsigned long val,
			      unsigned int len)
{
	print("%s %d\n", __FUNCTION__, __LINE__);
}

static unsigned long memory_test_read(struct memory_region *region,
				      unsigned long addr, unsigned int len)
{
	print("%s %d\n", __FUNCTION__, __LINE__);

	return 0;
}

static const struct memory_region_ops memory_test_ops = {
	.write = memory_test_write,
	.read = memory_test_read,
};

int create_memory_test_device(struct virt_machine *machine, int id,
			      unsigned long base, unsigned int size)
{
	add_memory_region(machine, id, base, size, &memory_test_ops);

	return 0;
}
