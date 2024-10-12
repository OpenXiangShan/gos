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

#include "machine.h"
#include "asm/type.h"
#include "device.h"
#include "mm.h"
#include "print.h"
#include "string.h"
#include "list.h"
#include "asm/pgtable.h"
#include "container_of.h"
#include "uart_emulator.h"
#include "memory_emulator.h"
#include "memory_test_emulator.h"
#include "clint_emulator.h"
#include "imsic_emulator.h"
#include "scheduler_emulator.h"
#include "pci_generic_emulator.h"
#include "vcpu_pt_remapping.h"
#include "gos.h"

static struct virt_machine_memmap virt_memmap[] = {
	[VIRT_MEMORY] = { CONFIG_VIRT_MEMORY_BASE, CONFIG_VIRT_MEMORY_LEN },
	[VIRT_UART]   = { CONFIG_VIRT_UART_BASE, CONFIG_VIRT_UART_LEN },
	[VIRT_SRAM]   = { CONFIG_VIRT_SRAM_BASE, CONFIG_VIRT_SRAM_LEN },
	[VIRT_TEST]   = { CONFIG_VIRT_MEMORY_TEST_BASE, CONFIG_VIRT_MEMORY_TEST_LEN },
#if CONFIG_VIRT_ENABLE_TIMER
	[VIRT_CLINT]  = { CONFIG_VIRT_CLINT_BASE, CONFIG_VIRT_CLINT_LEN },
#endif
#if CONFIG_VIRT_ENABLE_AIA
	[VIRT_IMSIC]  = { CONFIG_VIRT_IMSIC_BASE, CONFIG_VIRT_IMSIC_LEN },
#endif
	[VIRT_SCHEDULER]  = { CONFIG_VIRT_SCHEDULER_BASE, CONFIG_VIRT_SCHEDULER_LEN },
#if CONFIG_VIRT_ENABLE_PCI_GENERIC
	[VIRT_PCI_CONFIG] = { CONFIG_VIRT_PCI_GENERIC_BASE, CONFIG_VIRT_PCI_GENERIC_LEN },
	[VIRT_PCI_MMIO]   = { CONFIG_VIRT_PCI_MMIO_BASE, CONFIG_VIRT_PCI_MMIO_LEN },
#endif
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

unsigned long get_machine_memmap_base(int id)
{
	return virt_memmap[id].base;
}

unsigned int get_machine_memmap_size(int id)
{
	return virt_memmap[id].size;
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
	region->machine = machine;

	spin_lock(&machine->lock);
	list_for_each_entry(entry, &machine->memory_region_list, list) {
		if (memory_region_is_overlay
		    (entry->start, entry->end, region->start, region->end)
		    || (region->id != 0xFF && (entry->id == region->id))) {
			print("add memory region failed... start:0x%lx end:0x%lx\n", region->start, region->end);
			mm_free(region, sizeof(struct memory_region));
			spin_unlock(&machine->lock);
			return -1;
		}
	}

	list_add_tail(&region->list, &machine->memory_region_list);
	spin_unlock(&machine->lock);

	return 0;
}

void device_entry_data_redirect(struct virt_machine *machine)
{
	struct device_init_entry *device_entry = machine->device_entry;
	struct device_init_entry *device_entry_host_va =
	    machine->device_entry_host_va;
	struct device_init_entry *entry, *entry_host_va;
	int n;

	for (n = 0; n < machine->device_entry_count; n++) {
		entry = &device_entry[n];
		if ((unsigned long)entry->data == -1)
			continue;
		entry_host_va = &device_entry_host_va[n];
		entry->data += (unsigned long)machine->entry_data_gpa;
		entry_host_va->data = entry->data;
		print("entry_host_va device:%s entry_data:0x%lx\n",
		      entry_host_va->compatible, entry_host_va->data);
	}
}

int machine_finialize(struct virt_machine *machine)
{
	uart_device_finialize(machine, virt_memmap[VIRT_UART].base,
			      virt_memmap[VIRT_UART].size, VIRT_UART, 1);

	return 0;
}

static int get_extern_device_entry_count(struct virt_machine *machine)
{
	struct extern_device_init_entry *entry;
	int n = 0;

	list_for_each_entry(entry, &machine->extern_device_entry_list, list) {
		n++;
	}

	return n;
}

int machine_init(struct virt_machine *machine)
{
	struct extern_device_init_entry *extern_entry;
	struct device_init_entry *device_entry;
	struct device_init_entry *entry;
	int n = 0, n_extern = 0;
	void *entry_data_ptr;
	int entry_data_len = 0;

	n_extern = get_extern_device_entry_count(machine);

	device_entry = (struct device_init_entry *)
	    mm_alloc(sizeof(struct device_init_entry) * (VIRT_MEMMAP_CNT + n_extern + 1));
	if (!device_entry) {
		print("%s -- Out of memory!\n", __FUNCTION__);
		return -1;
	}
	memset((char *)device_entry, 0,
	       sizeof(struct device_init_entry) * (VIRT_MEMMAP_CNT + 1));

	entry_data_ptr = mm_alloc(PAGE_SIZE);
	if (!entry_data_ptr) {
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
	entry->data = (void *)-1;
	/* create memory device */
	create_memory_device(machine, VIRT_MEMORY,
			     virt_memmap[VIRT_MEMORY].base,
			     virt_memmap[VIRT_MEMORY].size);
#if CONFIG_VIRT_UART_8250
	/* create uart device_init_entry */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "qemu-8250");
	entry->start = virt_memmap[VIRT_UART].base;
	entry->len = virt_memmap[VIRT_UART].size;
	entry->data = (void *)-1;
	/* create uart device */
	create_uart_device(machine, VIRT_UART, virt_memmap[VIRT_UART].base,
			   virt_memmap[VIRT_UART].size);
#elif CONFIG_VIRT_UART_UARTLITE
	/* create uart device_init_entry */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "uartlite");
	entry->start = virt_memmap[VIRT_UART].base;
	entry->len = virt_memmap[VIRT_UART].size;
	entry->data = (void *)-1;
	/* create uart device */
	create_uart_device(machine, VIRT_UART, virt_memmap[VIRT_UART].base,
			   virt_memmap[VIRT_UART].size);
#elif CONFIG_VIRT_UART_NS16550A
	/* create uart device_init_entry */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "ns16550a");
	entry->start = virt_memmap[VIRT_UART].base;
	entry->len = virt_memmap[VIRT_UART].size;
	entry->data = (void *)-1;
	/* create uart device */
	create_uart_device(machine, VIRT_UART, virt_memmap[VIRT_UART].base,
			   virt_memmap[VIRT_UART].size);
#endif

	/* create sram device_init_entry */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "sram");
	entry->start = virt_memmap[VIRT_SRAM].base;
	entry->len = virt_memmap[VIRT_SRAM].size;
	entry->data = (void *)-1;
	/* cresate sram device */
	create_sram_device(machine, VIRT_SRAM, virt_memmap[VIRT_SRAM].base,
			   virt_memmap[VIRT_SRAM].size);

	/* create test devices */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "memory-test");
	entry->start = virt_memmap[VIRT_TEST].base;
	entry->len = virt_memmap[VIRT_TEST].size;
	entry->data = (void *)-1;
	create_memory_test_device(machine, VIRT_TEST,
				  virt_memmap[VIRT_TEST].base,
				  virt_memmap[VIRT_TEST].size);
#if CONFIG_VIRT_ENABLE_TIMER
	/* create clint device */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "clint");
	entry->start = virt_memmap[VIRT_CLINT].base;
	entry->len = virt_memmap[VIRT_CLINT].size;
	create_clint_device(machine, VIRT_CLINT,
			    virt_memmap[VIRT_CLINT].base,
			    virt_memmap[VIRT_CLINT].size);
	entry->data = (void *)((unsigned long)entry_data_len);
	entry_data_len += create_clint_priv_data(entry_data_ptr + entry_data_len);
#endif

#if CONFIG_VIRT_ENABLE_AIA
	/* create imsic device */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "imsic");
	entry->start = virt_memmap[VIRT_IMSIC].base;
	entry->len = virt_memmap[VIRT_IMSIC].size;
	entry->data = (void *)-1;
	create_imsic_device(machine, VIRT_IMSIC,
			    virt_memmap[VIRT_IMSIC].base,
			    virt_memmap[VIRT_IMSIC].size);
#endif
	/* create scheduler device */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "scheduler");
	entry->start = virt_memmap[VIRT_SCHEDULER].base;
	entry->len = virt_memmap[VIRT_SCHEDULER].size;
	entry->data = (void *)-1;
	create_scheduler_device(machine, VIRT_SCHEDULER,
				virt_memmap[VIRT_SCHEDULER].base,
				virt_memmap[VIRT_SCHEDULER].size);

#if CONFIG_VIRT_ENABLE_PCI_GENERIC
	/* create pci generic device */
	entry = &device_entry[n++];
	strcpy((char *)entry->compatible, "pci-generic-ecam");
	entry->start = virt_memmap[VIRT_PCI_CONFIG].base;
	entry->len = virt_memmap[VIRT_PCI_CONFIG].size;
	create_pci_generic_device(machine);
	entry->data = (void *)((unsigned long)entry_data_len);
	entry_data_len += create_pci_generic_priv_data(machine, entry_data_ptr + entry_data_len);
#endif
	/* create extern devices */
	list_for_each_entry(extern_entry, &machine->extern_device_entry_list, list) {
		struct device_init_entry *e = &extern_entry->entry;
		entry = &device_entry[n++];
		strcpy(entry->compatible, e->compatible);
		entry->start = e->start;
		entry->len = e->len;
		entry->data = (void *)e->data;
		add_memory_region(machine, 0xFF, entry->start, entry->len, extern_entry->ops);
	}

	/* end symbol */
	entry = &device_entry[n];
	strcpy((char *)entry->compatible, "THE END");
	entry->data = (void *)-1;

	machine->device_entry = device_entry;
	machine->entry_data = entry_data_ptr;
	machine->entry_data_len = entry_data_len;
	machine->device_entry_count = VIRT_MEMMAP_CNT + 1 + n_extern;

	return 0;
}

void machine_extern_device_entry_register(struct list_head *l,
					  struct extern_device_init_entry *entry)
{
	list_add_tail(&entry->list, l);
}
