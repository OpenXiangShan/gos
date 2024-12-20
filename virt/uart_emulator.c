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
#include "print.h"
#include "mm.h"
#include "device.h"
#include "../drivers/uart/qemu-8250.h"
#include "../drivers/uart/uartlite.h"
#include "../drivers/uart/ns16550a.h"
#include "asm/mmio.h"
#include "string.h"
#include "gos.h"

extern int mmu_is_on;

static void uart_mmio_write(struct memory_region *region,
			    unsigned long addr, unsigned long val,
			    unsigned int len)
{
	unsigned long base = region->hpa_base;
	unsigned long reg = addr - region->start;

	if (len == 1) {
		writeb(base + reg, val);
	} else if (len == 2) {

	} else if (len == 4) {
		writel(base + reg, val);
	} else if (len == 8) {
		writeq(base + reg, val);
	}
}

static unsigned long uart_mmio_read(struct memory_region *region,
				    unsigned long addr, unsigned int len)
{
	unsigned long base = region->hpa_base;
	unsigned long reg = addr - region->start;

	if (len == 1) {
		return readb(base + reg);
	} else if (len == 2) {

	} else if (len == 4) {
		return readl(base + reg);
	} else if (len == 8) {
		return readq(base + reg);
	}

	return 0;
}

static int uart_gstage_ioremap(unsigned long *pgdp,
			       unsigned long gpa, unsigned int size)
{
#if CONFIG_VIRT_UART_8250
	unsigned long addr = qemu_8250_get_base();
#elif CONFIG_VIRT_UART_UARTLITE
	unsigned long addr = uartlite_get_base();
#elif CONFIG_VIRT_UART_NS16550A
	unsigned long addr = ns16550a_get_base();
#endif
	unsigned long hpa;
	pgprot_t pgprot;

	if (!mmu_is_on)
		hpa = addr;
	else
		hpa = (unsigned long)walk_pt_va_to_pa((unsigned long)addr);

	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY |
		     _PAGE_USER);

	print("gstage page mapping[uart] -- gpa:0x%lx -> hpa:0x%lx size:0x%x\n",
	      gpa, hpa, size);
	return mmu_gstage_page_mapping(pgdp, hpa, gpa, size, pgprot);
}

static const struct memory_region_ops uart_memory_ops = {
	.write = uart_mmio_write,
	.read = uart_mmio_read,
};

int create_uart_device(struct virt_machine *machine, int id, unsigned long base,
		       unsigned int size)
{
	add_memory_region(machine, id, base, size, &uart_memory_ops);

	return 0;
}

int uart_device_finialize(struct virt_machine *machine, unsigned long gpa,
			  unsigned int size, int id, int pt)
{
	int found = 0;
	struct device *dev;
	struct memory_region *region;

	region = find_memory_region_by_id(machine, id);
	if (!region)
		return -1;

	for_each_device(dev) {
		if (!strncmp(dev->compatible, "qemu-8250", 128)) {
			found = 1;
			goto find;
		}
		if (!strncmp(dev->compatible, "uartlite", 128)) {
			found = 1;
			goto find;
		}
		if (!strncmp(dev->compatible, "ns16550a", 128)) {
			found = 1;
			goto find;
		}
	}

find:
	if (!found)
		return -1;

	region->hpa_base = dev->base;

	if (pt) {
		if (!machine->gstage_pgdp)
			return -1;

		return uart_gstage_ioremap((unsigned long *)
					   machine->gstage_pgdp, gpa, size);
	}

	return 0;
}
