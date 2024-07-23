#ifndef __SCHEDULER_EMULATOR_H__
#define __SCHEDULER_EMULATOR_H__

int create_scheduler_device(struct virt_machine *machine, int id,
			    unsigned long base, unsigned int size);

#endif
