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

#include "timer.h"

static int do_timer(void *data)
{
	int *start = (int *)data;

	*start = 1;

	return 0;
}

void wait_for_event_timeout(void *data, int (*expr)(void *data),
			    unsigned long ms)
{
	int start = 0;

	set_timer(do_timer, ms, &start);

	while (1) {
		if (expr(data))
			break;

		if (start == 1)
			break;
	}
}
