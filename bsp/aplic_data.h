#ifndef _APLILC_DATA_H
#define _APLIC_DATA_H

#include "imsic_data.h"

struct aplic_priv_data {
	int index;
	int mmode;
	int mode;
	int nr_irqs;
	int delegate;
	int child_index;
	struct imsic_priv_data *imsic_data;
};

extern struct aplic_priv_data aplic_hw_data_m;
extern struct aplic_priv_data aplic_hw_data_s;

#endif
