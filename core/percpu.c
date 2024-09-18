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

#include "percpu.h"
#include "mm.h"
#include "cpu.h"
#include "print.h"
#include "string.h"

extern char __percpu_start;
extern char __percpu_end;

unsigned long percpu_buf[MAX_CPU_COUNT] = { 0 };
unsigned long percpu_offset[MAX_CPU_COUNT] = { 0 };

int percpu_init()
{
	unsigned long percpu_sec_base = (unsigned long)&__percpu_start;
	unsigned int percpu_sec_size = &__percpu_end - &__percpu_start;
	int i;
	int n = get_cpu_count();

	if (n > MAX_CPU_COUNT)
		n = MAX_CPU_COUNT;

	if (percpu_sec_size == 0)
		return 0;

	for (i = 0; i < n; i++) {
		percpu_buf[i] = (unsigned long)mm_alloc(percpu_sec_size);
		memset((char *)percpu_buf[i], 0, percpu_sec_size);
		percpu_offset[i] = percpu_buf[i] - percpu_sec_base;
		print("Percpu: cpu%d -- percpu_buf:0x%lx, percpu_offset:0x%lx\n", i,
		      percpu_buf[i], percpu_offset[i]);
	}

	return 0;
}
