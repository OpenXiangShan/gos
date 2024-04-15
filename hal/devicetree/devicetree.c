// SPDX-License-Identifier: GPL-2.0
/*
 * Functions for working with the Flattened Device Tree data format
 *
 * Copyright 2009 Benjamin Herrenschmidt, IBM Corp
 * benh@kernel.crashing.org
 */

#include "asm/pgtable.h"
#include "print.h"
#include "asm/type.h"
#include "../fdt/libfdt.h"
#include "uapi/swab.h"

static bool of_fdt_device_is_available(const void *blob, unsigned long node)
{
	const char *status = fdt_getprop(blob, node, "status", NULL);

	if (!status)
		return true;

	if (!strcmp(status, "ok") || !strcmp(status, "okay"))
		return true;

	return false;
}

static inline unsigned long of_read_number(unsigned int *cell, int size)
{
	unsigned long r = 0;

	for (; size--; cell++)
		r = (r << 32) | swab32(*cell);

	return r;
}

static unsigned long dt_mem_next_cell(int s, unsigned int **cellp)
{
	unsigned int *p = *cellp;

	*cellp = p + s;

	return of_read_number(p, s);
}

int dtb_scan_memory(void *dtb_addr,
		    void (*fn)(unsigned long base, unsigned long size))
{
	int node;

	fdt_for_each_subnode(node, dtb_addr, 0) {
		char *type =
		    (char *)fdt_getprop(dtb_addr, node, "device_type", NULL);
		int l;
		unsigned int *reg, *endp;

		if (type == NULL)
			continue;

		if (strcmp(type, "memory"))
			continue;

		if (!of_fdt_device_is_available(dtb_addr, node))
			continue;

		reg = (unsigned int *)fdt_getprop(dtb_addr, node, "reg", &l);
		if (reg == NULL)
			continue;

		endp = reg + (l / sizeof(unsigned int));

		print("memory scan node %s, reg size: %d\n",
		      fdt_get_name(dtb_addr, node, NULL), l);

		while ((endp - reg) >= (2 + 2)) {
			unsigned long base;
			unsigned long size;

			base = dt_mem_next_cell(2, &reg);
			size = dt_mem_next_cell(2, &reg);

			if (size == 0)
				continue;

			print(" - %lx, %lx\n", base, size);
			fn(base, size);
		}
	}

	return 0;
}

void parse_dtb()
{
	//void *dtb_va = (void *)FIXMAP_DTB_START;      

	//dtb_scan_memory(dtb_addr);
}
