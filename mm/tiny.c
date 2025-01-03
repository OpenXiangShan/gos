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

#include "list.h"
#include "tiny_mm.h"
#include "mm.h"
#include "print.h"
#include "asm/type.h"
#include "asm/pgtable.h"
#include "string.h"
#include "align.h"

static spinlock_t tiny_lock = __SPINLOCK_INITIALIZER;

static struct list_head tiny_8;
static struct list_head tiny_16;
static struct list_head tiny_32;
static struct list_head tiny_64;
static struct list_head tiny_128;
static struct list_head tiny_256;
static struct list_head tiny_512;
static struct list_head tiny_1024;
static struct list_head tiny_2048;

struct tiny_entry {
	struct list_head *list_head;
	int order;
	int reclaim_level;
	int in_use;
};

static struct tiny_entry tiny_array[] = {
	{ &tiny_8,    0, 1, 0 },
	{ &tiny_16,   0, 1, 0 },
	{ &tiny_32,   1, 1, 0 },
	{ &tiny_64,   1, 1, 0 },
	{ &tiny_128,  2, 1, 0 },
	{ &tiny_256,  2, 1, 0 },
	{ &tiny_512,  3, 1, 0 },
	{ &tiny_1024, 3, 1, 0 },
	{ &tiny_2048, 3, 1, 0 },
};

#define TINY_ARRAY_COUNT sizeof(tiny_array) / sizeof(tiny_array[0])

static struct tiny *get_tiny(unsigned int size)
{
	int i, wholesale;
	struct tiny *tiny, *new;
	struct tiny_meta *meta;

	for (i = 0; i < TINY_ARRAY_COUNT; i++) {
		if (size <= (1UL << (i + 3)))
			goto find;
	}

	return NULL;

find:
	list_for_each_entry(tiny, tiny_array[i].list_head, list) {
		if (tiny->meta.free)
			return tiny;
	}

	/* need to grow */
	wholesale = PAGE_SIZE * (1UL << tiny_array[i].order);
	new = __mm_alloc(wholesale);
	if (!new)
		return NULL;
	memset((char *)new, 0, wholesale);

	new->size = wholesale;

	meta = &new->meta;
	meta->bitmap = (unsigned long *)ALIGN_SIZE((unsigned long)new->buffer, 8);
	meta->unit = 1UL << (i + 3);
	meta->bitmap_cnt = (wholesale / meta->unit) / (8 * sizeof(unsigned long)) + 1;
	new->objs = (void *)((unsigned long)meta->bitmap + meta->bitmap_cnt * sizeof(unsigned long));
	new->objs = (void *)ALIGN_SIZE((unsigned long)new->objs, 8);
	meta->total =
	    ((unsigned long)new + wholesale - (unsigned long)new->objs) / meta->unit;
	meta->free = meta->total;

	list_add_tail(&new->list, tiny_array[i].list_head);
	tiny_array[i].in_use++;

	goto find;

	return NULL;
}

static struct tiny *get_tiny_by_addr(unsigned long addr, int* index)
{
	int i, size;
	struct tiny *tiny;

	for (i = 0; i < TINY_ARRAY_COUNT; i++) {
		list_for_each_entry(tiny, tiny_array[i].list_head, list) {
			size = tiny->meta.unit * tiny->meta.total;
			if (addr >= (unsigned long)tiny->objs &&
			    addr < (unsigned long)tiny->objs + size) {
				*index = i;
				return tiny;
			}
		}
	}

	return NULL;
}

static void *tiny_get_free_obj(struct tiny *tiny)
{
	int index = 0;
	int total = tiny->meta.total;
	unsigned long *bitmap = tiny->meta.bitmap;
	int bitmap_cnt = tiny->meta.bitmap_cnt;
	unsigned int unit = tiny->meta.unit;
	unsigned long addr;

	while (index < total) {
		int nr = index / (sizeof(unsigned long) * 8);
		int per_id = index % (sizeof(unsigned long) * 8);
		unsigned long b;

		if (nr >= bitmap_cnt) {
			print("warning!! %s tiny_%d -- total:%d index:%d bitmap_cnt:%d\n",
			      __FUNCTION__, tiny->meta.unit, tiny->meta.total, index, bitmap_cnt);
			return NULL;
		}
		b = bitmap[nr];

		if (((b >> per_id) & 0x1) == 0) {
			b |= 1UL << per_id;
			tiny->meta.bitmap[nr] = b;
			tiny->meta.free -= 1;
			addr = (unsigned long)tiny->objs +
			       nr * 8 * sizeof(unsigned long) * unit +
			       per_id * unit;
			return (void *)addr;
		}
		index++;
	}

	return NULL;
}

static int tiny_release_obj(struct tiny *tiny, void *addr)
{
	int index =
	    ((unsigned long)addr - (unsigned long)tiny->objs) / tiny->meta.unit;
	int bitmap_cnt = tiny->meta.bitmap_cnt;
	int per_id = index % (8 * sizeof(unsigned long));
	int nr = index / (8 * sizeof(unsigned long));
	unsigned long *bitmap = tiny->meta.bitmap;
	unsigned long b;

	if (nr >= bitmap_cnt) {
		print("warning!! %s tiny_%d -- total:%d index:%d\n",
		      __FUNCTION__, tiny->meta.unit, tiny->meta.total, index);
		return NULL;
	}
	b = bitmap[nr];

	if (!(b & (1UL << per_id)))
		return -1;

	b &= ~(1UL << per_id);
	tiny->meta.bitmap[nr] = b;
	tiny->meta.free += 1;

	return tiny->meta.free;
}

static void tiny_reclaim_obj(struct tiny *tiny)
{
	list_del(&tiny->list);

	__mm_free((void *)tiny, tiny->size);
	tiny = NULL;
}

void *tiny_alloc(unsigned int size)
{
	struct tiny *tiny;
	void *ret;
	irq_flags_t flags;

	spin_lock_irqsave(&tiny_lock, flags);
	tiny = get_tiny(size);
	if (!tiny) {
		spin_unlock_irqrestore(&tiny_lock, flags);
		print("%s -- failed...\n", __FUNCTION__);
		return NULL;
	}

	ret = tiny_get_free_obj(tiny);
	spin_unlock_irqrestore(&tiny_lock, flags);

	return ret;
}

void tiny_free(void *addr)
{
	struct tiny *tiny;
	irq_flags_t flags;
	int free, index;

	spin_lock_irqsave(&tiny_lock, flags);
	tiny = get_tiny_by_addr((unsigned long)addr, &index);
	if (!tiny) {
		spin_unlock_irqrestore(&tiny_lock, flags);
		return;
	}

	free = tiny_release_obj(tiny, addr);
	if (tiny->meta.total == free &&
	    tiny_array[index].reclaim_level < tiny_array[index].in_use) {
		tiny_reclaim_obj(tiny);
		tiny_array[index].in_use--;
	}
	spin_unlock_irqrestore(&tiny_lock, flags);
}

void tiny_init(void)
{
	for (int i = 0; i < TINY_ARRAY_COUNT; i++)
		INIT_LIST_HEAD(tiny_array[i].list_head);
}
