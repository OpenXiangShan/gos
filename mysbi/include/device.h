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

#ifndef DEVICE_H
#define DEVICE_H

#define DEVICE_INIT_TABLE __device_init_table
#define DEVICE_INIT_TABLE_END __device_init_table_end

struct device {
	unsigned long start;
	unsigned int len;
	unsigned int irq;
};

struct device_init_entry {
	char compatible[128];
	unsigned long start;
	unsigned int len;
	char irq_parent[128];
	unsigned int irq[16];
	int irq_num;
	char iommu[128];
	int dev_id;
	void *data;
};

#endif
