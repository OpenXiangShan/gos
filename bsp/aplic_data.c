#include "aplic_data.h"

#define APLIC_DIRECT_MODE  0
#define APLIC_MSI_MODE     1

#define APLIC_M_MODE 0
#define APLIC_S_MODE 1

extern struct imsic_priv_data imsic_hw_data;

struct aplic_priv_data aplic_hw_data_m = {
	0xff,
	APLIC_M_MODE,
	APLIC_MSI_MODE,
	64,
	1,
	0,
	&imsic_hw_data,
};

struct aplic_priv_data aplic_hw_data_s = {
	0,
	APLIC_S_MODE,
	APLIC_MSI_MODE,
	64,
	0,
	0xffff,
	&imsic_hw_data,
};
