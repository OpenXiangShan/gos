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

#ifndef _EVENT_H
#define _EVENT_H

void wait_for_event(void *data, int (*expr)(void *));
void wait_for_ms(unsigned long ms);
char wait_for_input(int fd);
char wait_for_input_timeout(int fd, unsigned long ms);
void wait_for_event_timeout(void *data, int (*expr)(void *data),
			    unsigned long ms);

#endif
