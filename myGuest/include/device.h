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

#ifndef __GUEST_DEVICE_H
#define __GUEST_DEVICE_H

#define DRIVER_INIT_TABLE __driver_init_table
#define DRIVER_INIT_TABLE_END __driver_init_table_end

struct device_init_entry {
	char compatible[128];
	unsigned long start;
	unsigned int len;
	char irq_parent[128];
	int irq[16];
	int irq_num;
	char iommu[128];
	int dev_id;
	void *data;
};

typedef int (*driver_init)(unsigned long base, unsigned int len, void *data);

struct driver_init_entry {
	char compatible[128];
	driver_init init;
};

void create_devices(void);

#define DRIVER_REGISTER(name, init_fn, compat)                                \
	static const struct driver_init_entry __attribute__((used))           \
		__drvier_init_##name                                          \
		__attribute__((section(".driver_init_table"))) = {            \
			.compatible = compat,                                 \
			.init = init_fn,                                      \
		}

#endif
