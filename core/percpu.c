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
		print("cpu%d -- percpu_buf:0x%lx, percpu_offset:0x%lx\n", i,
		      percpu_buf[i], percpu_offset[i]);
	}

	return 0;
}
