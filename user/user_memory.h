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

#ifndef __USER_MEMORY_H
#define __USER_MEMORY_H
#include "asm/pgtable.h"

struct user_memory_region *find_user_memory_region(struct user *user,
						   unsigned long va);
int add_user_space_memory(struct user *user, unsigned long start,
			  unsigned int len);
void *user_space_mmap(unsigned int size);
void user_space_unmap(void *addr, unsigned int size);
void *user_space_mmap_pg(unsigned int size, pgprot_t pgprot);

#endif
