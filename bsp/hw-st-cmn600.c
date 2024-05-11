#ifdef USE_ST_CMN600

#include <device.h>
#include "plic.h"
#include "clint.h"
#include "riscv_iommu_data.h"
#include "imsic_data.h"
#include "aplic_data.h"

extern struct clint_data st_clint_hw_data;
extern struct plic_data plic_hw_data;
extern struct riscv_iommu_data riscv_iommu_data;

static const struct device_init_entry __attribute__((used))
    device_info[] __attribute__((section(".device_init_table"))) = {
	{
	 "uartlite",
	 0x40600000,
	 0x1000,
#ifndef USE_AIA
	 "PLIC",
#else
	 "APLIC_S",
#endif
	 { 40,},
	 1,
	 0,
	 0,
	  },
#ifndef USE_AIA
	{
	 "PLIC",
	 0x3c000000,
	 0x4000000,
	 "INTC",
	 { 0xFF,},
	 0,
	 0,
	 &plic_hw_data,
	  },
#else
	{
	 "IMSIC",
	 0x28000000,
	 0x4000000,
	 "INTC",
	 { 0xFF,},
	 0,
	 0,
	 &imsic_hw_data,
	  },
#if 0
	{
	 "APLIC_M",
	 0xc000000,
	 0x4000,
	 "APLIC_S",
	 { 0xFF,},
	 0,
	 0,
	 &aplic_hw_data_m,
	  },
	{
	 "APLIC_S",
	 0xd000000,
	 0x4000,
	 "IMSIC",
	 { 0xFF,},
	 0,
	 0,
	 &aplic_hw_data_s,
	  },
#endif

#endif
#ifdef USE_AIA
	{
	 "imsic,test",
	 0x70000000,
	 0x1000,
	 "IMSIC",
	 { 0xff,},
	 0,
	 0,
	 0,
	  },
#endif
	{
	 "clint",
	 0x38000000,
	 0x10000,
	 "INTC",
	 { 0xFF,},
	 0,
	 0,
	 &st_clint_hw_data,
	  },
	{
	 "THE END",
	 0xFF,
	 0xFF,
	 " ",
	 { 0xFF,},
	 0,
	 0,
	 0,
	  },
};
#endif
