#include "aplic_data.h"

#define APLIC_DIRECT_MODE  0
#define APLIC_MSI_MODE     1

extern struct imsic_priv_data imsic_hw_data;

struct aplic_priv_data aplic_hw_data = {
	APLIC_MSI_MODE,
	2048,
	&imsic_hw_data,
};
