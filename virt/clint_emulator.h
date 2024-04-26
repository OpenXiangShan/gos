#ifndef __CLINT_EMULATOR_H__
#define __CLINT_EMULATOR_H__

int create_clint_device(struct virt_machine *machine, int id,
			unsigned long base, unsigned int size);
int create_clint_priv_data(void *ptr);

#endif
