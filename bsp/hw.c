#include <device.h>
#include "plic.h"
#include "clint.h"

extern struct clint_data clint_hw_data;
extern struct clint_data qemu_clint_hw_data;
extern struct plic_data plic_hw_data;

static const struct device_init_entry __attribute__((used))
    device_info[] __attribute__((section(".device_init_table"))) = {
#ifndef USE_QEMU
	{
	 "ns16550a",
	 0x310b0000,
	 0x10000,
	 40,
	 0,
	  },
#else
	{
	 "qemu-8250",
	 0x10000000,
	 0x1000,
	 10,
	 0,
	  },
#endif
	{
	 "memory-map",
	 0x80000000,
	 0x80000000,
	 0xFF,
	 0,
	  },
#ifndef USE_QEMU
	{
	 "PLIC",
	 0x3c000000,
	 0x4000000,
	 0xFF,
	 &plic_hw_data,
	  },
#else
	{
	 "PLIC",
	 0xc000000,
	 0x4000000,
	 0xFF,
	 &plic_hw_data,
	  },
#endif
#ifndef USE_QEMU
	{
	 "clint",
	 0x38000000,
	 0x10000,
	 0xFF,
	 &clint_hw_data,
	  },
#else
	{
	 "clint",
	 0x2000000,
	 0x10000,
	 0xFF,
	 &qemu_clint_hw_data,
	  },
#endif
	{
	 "THE END",
	 0xFF,
	 0xFF,
	 0xFF,
	 0,
	  }
};
