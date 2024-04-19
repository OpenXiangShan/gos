#ifndef __MEMORY_TEST_EMULATOR__
#define __MEMORY_TEST_EMULATOR__

#include "machine.h"

int create_memory_test_device(struct virt_machine *machine, int id,
			      unsigned long base, unsigned int size);

#endif
