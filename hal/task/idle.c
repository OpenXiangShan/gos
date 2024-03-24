#include <asm/sbi.h>
#include "task.h"
#include "print.h"

int do_idle(void *data)
{
	while (1) {
		//print("do idle cpu%d\n", sbi_get_cpu_id());
		__asm__ __volatile__("wfi");
	}
}
