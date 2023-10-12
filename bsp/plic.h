#ifndef _HW_PLIC_H
#define _HW_PLIC_H

struct plic_data {
	unsigned char max_priority;
	unsigned char ndev;
};

extern struct plic_data plic_hw_data;

#endif
