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

#ifndef __RENDER_H__
#define __RENDER_H__

#include "list.h"
#include "spinlocks.h"

struct position {
	int X;
	int Y;
};

struct layer {
	struct list_head list;
	char name[32];
	int width;
	int height;
	int order;
	char *pattern;
	struct position pos;
};

struct renderer {
	struct list_head layers;
	int width;
	int height;
	char *draw_board;
	spinlock_t lock;
};

void layer_two_dim_process_batch(char *name1, char *name2,
				 int (*process)(struct layer * layer1,
						struct layer * layer2,
						struct layer ** q1,
						struct layer ** q2, int *n1,
						int *n2, void *data),
				 void *data);
void layer_process_batch(char *name,
			 void (*process)(struct layer * layer,
					 struct layer ** q, int *n, void *data),
			 void *data);
struct layer *get_layer(char *name);
struct layer *generate_layer(char *name, const char *p,
			     int width, int height, int order);
void render_add_layer(struct layer *layer, int pos_x, int pos_y);
void layer_release_all(void);
void render_update(void);
int render_init(int width, int height);

#endif
