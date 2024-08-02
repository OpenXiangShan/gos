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

#include <print.h>
#include <device.h>
#include "event_loop.h"

void event_loop_start(int (*process)(char command))
{
	int fd, i;
	int ret;
	char buf[64];

	fd = open("UART0");
	if (fd < 0) {
		print("open %s failed...\n", "UART0");
		return;
	}

	while (1) {
		ret = read(fd, buf, 0, 64, BLOCK);
		for (i = 0; i < ret; i++) {
			if (process(buf[i]))
				goto end;
		}
	}
end:
	return;
}
