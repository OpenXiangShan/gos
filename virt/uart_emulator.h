#ifndef __UART_EMULATOR_H
#define __UART_EMULATOR_H

int create_uart_device(struct virt_machine *machine, int id, unsigned long base,
		       unsigned int size);
int uart_device_finialize(struct virt_machine *machine, unsigned long gpa,
			  unsigned int size, int id, int pt);

#endif
