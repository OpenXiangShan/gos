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

#ifndef _PLIC_H
#define _PLIC_H

#include "irq.h"

#define CONTEXT_BASE      0x200000
#define CONTEXT_SIZE      0x1000
#define CONTEXT_THRESHOLD 0x0
#define CONTEXT_CLAIM     0x04

#define CONTEXT_ENABLE_BASE  0x2000
#define CONTEXT_ENABLE_SIZE  0x80

#define PRIORITY_PER_ID  4

#define CPU_TO_HART(cpu) (2 * cpu + 1)

struct plic_percpu_info {
	unsigned long base;
	unsigned long enable_base;
};

struct plic {
	struct irq_domain irq_domain;
	unsigned long base_address;
	unsigned char max_priority;
	unsigned char ndev;
};

struct irq_domain *plic_get_irq_domain(void);

#endif
