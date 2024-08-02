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

#ifndef __TINY_MM_H__
#define __TINY_MM_H__

#include "list.h"
#include "asm/pgtable.h"
#include "spinlocks.h"

struct tiny_meta {
	unsigned int unit;
	unsigned long *bitmap;
	unsigned char bitmap_cnt;
	unsigned char total;
	unsigned char free;
};

struct tiny {
	struct list_head list;
	struct tiny_meta meta;
	void *objs;
	unsigned int size;
	char buffer[0];
};

void *tiny_alloc(unsigned int size);
void tiny_free(void *addr);
void tiny_init(void);

#endif
