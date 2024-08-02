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

#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "plane.h"

int check_position_valid(int total_w, int total_h, int x, int y, int obj_w,
			 int obj_h, char dir);
int check_collision(int a_w, int a_h, int a_x, int a_y, int b_w, int b_h,
		    int b_x, int b_y);

#endif
