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
