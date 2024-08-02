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

#ifndef __PERCPU_H
#define __PERCPU_H

#include "cpu.h"

#define DEFINE_PER_CPU(type, name) \
		__attribute__((section(".percpu"))) __typeof__(type) name

#define RELOC_HIDE(ptr, off)                                            \
({                                                                      \
        unsigned long __ptr;                                            \
        __asm__ ("" : "=r"(__ptr) : "0"(ptr));                          \
        (typeof(ptr)) (__ptr + (off));                                  \
})

#define SHIFT_PERCPU_PTR(__p, __offset) \
	RELOC_HIDE((typeof(*(__p)) *)(__p), (__offset))

extern unsigned long percpu_offset[MAX_CPU_COUNT];

#define per_cpu_offset(x) (percpu_offset[x])

#define per_cpu_ptr(ptr, cpu) SHIFT_PERCPU_PTR((ptr), per_cpu_offset((cpu)))

#define per_cpu(var, cpu) (*per_cpu_ptr(&(var), cpu))

int percpu_init(void);

#endif
