#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "plane.h"

int check_position_valid(int total_w, int total_h, int x, int y, int obj_w,
			 int obj_h, char dir);
int check_collision(int a_w, int a_h, int a_x, int a_y, int b_w, int b_h,
		    int b_x, int b_y);

#endif
