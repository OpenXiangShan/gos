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
#include "list.h"
#include "string.h"
#include "display.h"
#include "dma-mapping.h"

static LIST_HEAD(dps);
static unsigned long dp_idx_bitmap = 0;

static int find_free_dp_index(void)
{
	unsigned long bitmap = dp_idx_bitmap;
	int pos = 0;

	while (bitmap & 0x01) {
		if (pos == 64)
			return -1;
		bitmap = bitmap >> 1;
		pos++;
	}

	dp_idx_bitmap |= (1UL) << pos;

	return pos;
}

int display(struct display_device *dp, void *buf)
{
	memcpy(dp->buf, buf, dp->size);

	if (dp->ops && dp->ops->display)
		return dp->ops->display(dp->dma_addr, dp->size, NULL);

	return 0;
}

int request_display_buffer(struct display_device *dp, int size)
{
	dp->buf = dma_alloc(dp->dev, &dp->dma_addr, size, NULL);
	if (!dp->buf)
		return -1;

	dp->size = size;

	return 0;
}

struct display_device *get_display_device(char *name)
{
	struct display_device *dp;

	list_for_each_entry(dp, &dps, list) {
		if (!strcmp(name, dp->name))
			return dp;
	}

	return NULL;
}

int register_display_device(struct display_device *dp)
{
	dp->idx = find_free_dp_index();

	list_add_tail(&dp->list, &dps);

	return 0;
}
