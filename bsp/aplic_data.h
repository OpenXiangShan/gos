#ifndef _APLILC_DATA_H
#define _APLIC_DATA_H

#include "imsic_data.h"

struct aplic_priv_data {
	int mode;
	int nr_irqs;
	struct imsic_priv_data *imsic_data;
};

extern struct aplic_priv_data aplic_hw_data;

#endif
