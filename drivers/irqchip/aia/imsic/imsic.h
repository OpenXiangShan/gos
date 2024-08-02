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

#ifndef _IMSIC_H
#define _IMSIC_H

#include <asm/type.h>
#include "irq.h"

#define MAX_HARTS 16
#define MAX_GUEST_PER_CPU 64	//1Smode + 63VSmode
#define MAX_IDS 2048

#define TOPEI_ID_SHIFT          16

struct imsic_priv_data {
	u32 guest_index_bits;
	u32 hart_index_bits;
	u32 group_index_bits;
	int nr_ids;
	int nr_harts;
	int nr_guests;
};

struct imsic {
	unsigned long base;
	/*
	 * MSI Target Address Scheme
	 *
	 * XLEN-1                                                12     0
	 * |                                                     |     |
	 * -------------------------------------------------------------
	 * |xxxxxx|Group Index|xxxxxxxxxxx|HART Index|Guest Index|  0  |
	 * -------------------------------------------------------------
	 */
	u32 guest_index_bits;
	u32 hart_index_bits;
	u32 group_index_bits;
	int nr_ids;
	int nr_harts;
	int nr_guests;
	unsigned long interrupt_file_base[MAX_HARTS][MAX_GUEST_PER_CPU];
	unsigned long ids_enable_bits[MAX_IDS / (sizeof(unsigned long) * 8)];
	unsigned long ids_used_bits[MAX_IDS / (sizeof(unsigned long) * 8)];
	unsigned int ids_target_cpu[MAX_IDS];
	struct irq_domain irq_domain;
};

struct irq_domain *imsic_get_irq_domain(void);
unsigned long imsic_get_interrupt_file_base(int cpu, int guest_id);
int imsic_get_hart_index_bits(void);
int imsic_get_group_index_bits(void);

#endif
