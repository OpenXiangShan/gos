#include "machine.h"
#include "device.h"
#include "mm.h"
#include "print.h"
#include "string.h"
#include "list.h"
#include "asm/pgtable.h"
#include "uart_emulator.h"
#include "memory_emulator.h"
#include "asm/type.h"
#include "device.h"

static struct virt_machine_memmap virt_memmap[] = {
	[VIRT_MEMORY] = { 0x90000000, 0x1000000 },
	[VIRT_UART] = { 0x310b0000, 0x10000 },
	[VIRT_SRAM] = { 0x1000, 0x1000 },
};

#define VIRT_MEMMAP_CNT (sizeof(virt_memmap)/sizeof(virt_memmap[0]))

static int memory_region_is_overlay(unsigned long start, unsigned long end,
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

struct memory_region *find_memory_region(struct virt_machine *machine,
					 unsigned long gpa)
{
	struct memory_region *entry;

	spin_lock(&machine->lock);
	list_for_each_entry(entry, &machine->memory_region_list, list) {
		if (gpa >= entry->start && gpa <= entry->end) {
			spin_unlock(&machine->lock);
			return entry;
		}
	}
	spin_unlock(&machine->lock);

	return NULL;
}

struct memory_region *find_memory_region_by_id(struct virt_machine *machine,
					       int id)
{
	struct memory_region *entry;

	spin_lock(&machine->lock);
	list_for_each_entry(entry, &machine->memory_region_list, list) {
		if (entry->id == id) {
			spin_unlock(&machine->lock);
			return entry;
		}
	}
	spin_unlock(&machine->lock);

	return NULL;
}

int add_memory_region(struct virt_machine *machine, int id, unsigned long base,
		      unsigned int size, const struct memory_region_ops *ops)
{
	struct memory_region *region;
	struct memory_region *entry;

	region = mm_alloc(sizeof(struct memory_region));
	if (!region) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return -1;
	}

	region->id = id;
	region->start = base;
	region->end = base + size;
	region->ops = ops;

	spin_lock(&machine->lock);
	list_for_each_entry(entry, &machine->memory_region_list, list) {
		if (memory_region_is_overlay
		    (entry->start, entry->end, region->start, region->end)
		    || (entry->id == region->id)) {
			spin_unlock(&machine->lock);
			mm_free(region, sizeof(struct memory_region));
			return -1;
		}
	}

	list_add_tail(&region->list, &machine->memory_region_list);
	spin_unlock(&machine->lock);

	return 0;
}

static int uart_device_emulate(struct virt_machine *machine, char *name, int pt)
{
	int nr, found = 0;
	struct device *dev;
	struct devices *p_devs = get_devices();
	struct memory_region *region;
	unsigned long gpa;
	unsigned int size;

	region = find_memory_region_by_id(machine, VIRT_UART);
	gpa = virt_memmap[VIRT_UART].base;
	size = virt_memmap[VIRT_UART].size;

	if (!region)
		return -1;

	nr = p_devs->avail;
	for_each_device(dev, p_devs->p_devices, nr) {
		if (!strncmp(dev->compatible, name, 128)) {
			found = 1;
			goto find;
		}
	}

find:
	if (!found)
		return -1;

	region->hpa_base = dev->base;

	if (pt) {
		if (!region->ops->ioremap)
			return -1;

		if (!machine->gstage_pgdp)
			return -1;

		return region->ops->
		    ioremap((unsigned long *)machine->gstage_pgdp, gpa, size);
	}

	return 0;
}

int machine_finialize(struct virt_machine *machine)
{
	/* uart gstage ioremap */
	uart_device_emulate(machine, "qemu-8250", 0);

	return 0;
}

int machine_init(struct virt_machine *machine)
{
	struct device_init_entry *device_entry;
	struct device_init_entry *entry;
	int n = 0;

	device_entry = (struct device_init_entry *)
	    mm_alloc(sizeof(struct device_init_entry) * (VIRT_MEMMAP_CNT + 1));

	if (!device_entry) {
		print("%s -- Out of memory!\n", __FUNCTION__);
		return -1;
	}

	INIT_LIST_HEAD(&machine->memory_region_list);
	__SPINLOCK_INIT(&machine->lock);

	/* create memory-map device_init_entry */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "memory-map");
	entry->start = virt_memmap[VIRT_MEMORY].base;
	entry->len = virt_memmap[VIRT_MEMORY].size;
	/* create memory device */
	create_memory_device(machine, VIRT_MEMORY,
			     virt_memmap[VIRT_MEMORY].base,
			     virt_memmap[VIRT_MEMORY].size);

	/* create uart device_init_entry */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "qemu-8250");
	entry->start = virt_memmap[VIRT_UART].base;
	entry->len = virt_memmap[VIRT_UART].size;
	n++;
	/* create uart device */
	create_uart_device(machine, VIRT_UART, virt_memmap[VIRT_UART].base,
			   virt_memmap[VIRT_UART].size);

	/* create sram device_init_entry */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "sram");
	entry->start = virt_memmap[VIRT_SRAM].base;
	entry->len = virt_memmap[VIRT_SRAM].size;
	n++;
	/* cresate sram device */
	create_sram_device(machine, VIRT_SRAM, virt_memmap[VIRT_SRAM].base,
			   virt_memmap[VIRT_SRAM].size);
	/* end symbol */
	entry = &device_entry[n];
	strcpy((char *)entry->compatible, "THE END");

	machine->device_entry = device_entry;
	machine->device_entry_count = VIRT_MEMMAP_CNT;

	return 0;
}
