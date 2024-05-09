#include "clint.h"

struct clint_data clint_hw_data = {
	.clint_freq = 500000,
};

struct clint_data qemu_clint_hw_data = {
	.clint_freq = 10000000,
};

struct clint_data st_clint_hw_data = {
	.clint_freq = 10000000,
};
