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

#include "plane.h"

int check_collision(int a_w, int a_h, int a_x, int a_y,
		    int b_w, int b_h, int b_x, int b_y)
{
	if ((b_x >= a_x) &&
	    ((b_x + b_w) <= (a_x + a_w)) &&
	    (b_y >= a_y) && ((b_y + b_h) <= (a_y + a_h)))
		return 1;

	if ((a_x >= b_x) &&
	    ((a_x + a_w) <= (b_x + b_w)) &&
	    (a_y >= b_y) && ((a_y + a_h) <= (b_y + b_h)))
		return 1;

	return 0;
}

int check_position_valid(int total_w, int total_h, int x, int y, int obj_w,
			 int obj_h, char dir)
{
	int left = x;
	int right = x + obj_w;
	int top = y;
	int bottom = y + obj_h;

	if (dir == 'w') {
		if (top == 0)
			return 0;
	} else if (dir == 's') {
		if (bottom == total_h)
			return 0;
	} else if (dir == 'a') {
		if (left == 0)
			return 0;
	} else if (dir == 'd') {
		if (right == total_w)
			return 0;
	}

	return 1;
}
