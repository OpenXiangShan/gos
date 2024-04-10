#ifndef __MEMORY_EMULATOR_H
#define __MEMORY_EMULATOR_H

int create_memory_device(struct virt_machine *machine, int id,
			 unsigned long base, unsigned int size);
int create_sram_device(struct virt_machine *machine, int id, unsigned long base,
		       unsigned int size);

#endif
