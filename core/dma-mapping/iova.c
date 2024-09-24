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

#include "asm/type.h"
#include "device.h"
#include "list.h"
#include "iova.h"
#include "mm.h"
#include "asm/pgtable.h"

#define IOVA_START 0
#define IOVA_END ((-1UL) - 1)

unsigned long iova_alloc(struct list_head *iovad, int len)
{
	struct iova *iova, *next, *new;
	int find = 0;
	int page_nr = N_PAGE(len);
	int size = page_nr * PAGE_SIZE;

	list_for_each_entry(iova, iovad, list) {
		next = list_next_entry(iova, list);
		if (iova->base + iova->size + size < next->base) {
			find = 1;
			break;
		}
	}

	new = (struct iova *)mm_alloc(sizeof(struct iova));
	if (!new)
		return (-1UL);

	if (find) {
		new->base = iova->base + iova->size;
		new->size = size;
	}
	else {
		new->base = IOVA_START;
		new->size = size;
	}

	if (new->base + new->size >= IOVA_END) {
		mm_free(new, sizeof(struct iova));
		return (-1UL);
	}

	list_add_tail(&new->list, iovad);

	return new->base;
}

void iova_free(struct list_head *iovad, unsigned long addr)
{
	struct iova *iova, *n;

	list_for_each_entry_safe(iova, n, iovad, list) {
		if (iova->base == addr) {
			list_del(&iova->list);
			mm_free(iova, sizeof(struct iova));
		}
	}
}
