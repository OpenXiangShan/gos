#ifndef __PLANE_H__
#define __PLANE_H__

#include "render.h"

struct materal {
	char *name;
	int width;
	int height;
	const char *pattern;
	int x;
	int y;
	unsigned int speed;
	unsigned int speed_div;
};

void event_loop_start(int (*process)(char command));

#endif
