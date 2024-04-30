#ifndef __IMSIC_EMULATOR_H__
#define __IMSIC_EMULATOR_H__

int create_imsic_device(struct virt_machine *machine, int id,
			unsigned long base, unsigned int size);
int imsic_device_finialize(struct virt_machine *machine, unsigned long base,
			   unsigned int size, int id, int pt);
int imsic_gstage_ioremap(unsigned long *pgdp, unsigned long hpa,
			 unsigned long gpa, unsigned int size);

#endif
