#include "gos.h"
#ifdef CONFIG_SELECT_KMH_FPGA

#include <device.h>
#include "plic.h"
#include "clint.h"
#include "riscv_iommu_data.h"
#include "imsic_data.h"
#include "aplic_data.h"

extern struct clint_data clint_kmh_hw_data;
extern struct clint_data qemu_clint_hw_data;
extern struct plic_data plic_hw_data;
extern struct riscv_iommu_data riscv_iommu_data;

static const struct device_init_entry __attribute__((used))
    device_info[] __attribute__((section(".device_init_table"))) = {
	{
	 "ns16550a",
	 0x310b0000,
	 0x10000,
#ifndef CONFIG_SELECT_AIA
	 "PLIC",
#else
	 "APLIC_S",
#endif
	 { 40,},
	 1,
	 0,
	 0,
	  },
#ifndef CONFIG_SELECT_AIA
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
#endif
	{
	 "clint",
	 0x38000000,
	 0x10000,
	 "INTC",
	 { 0xFF,},
	 0,
	 0,
	 &clint_kmh_hw_data,
	  },
#if 0
	{
	 "dw,dmac",
	 0x30040000,
	 0x10000,
	 "PLIC",
	 { 4,},
	 1,
	 0,
	 0,
	  },
#endif
#ifdef CONFIT_SELECT_AIA
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
