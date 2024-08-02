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

#include <timer.h>
#include <device.h>

void wait_for_event(void *data, int (*expr)(void *data))
{
	while(1)
		if(expr(data))
			break;
}

static void do_timer(void *data)
{
	int *start = (int *)data;

	*start = 1;
}

void wait_for_ms(unsigned long ms)
{
	int start = 0;

	set_timer(ms, do_timer, &start);

	while (1)
		if (start == 1)
			break;
}

char wait_for_input_timeout(int fd, unsigned long ms)
{
	char c;
	int start = 0;
	int ret = 0;

	set_timer(ms, do_timer, &start);

	while (1) {
		ret = read(fd, &c, 0, 1, NONBLOCK);
		if (ret > 0)
			break;

		if (start == 1)
			break;
	}

	return c;
}

void wait_for_event_timeout(void *data, int (*expr)(void *data),
			    unsigned long ms)
{
	int start = 0;

	set_timer(ms, do_timer, &start);

	while (1) {
		if (expr(data))
			break;

		if (start == 1)
			break;
	}
}
