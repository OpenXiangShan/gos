#include <device.h>
#include "plic.h"
#include "clint.h"
#include "riscv_iommu_data.h"

extern struct clint_data clint_hw_data;
extern struct clint_data qemu_clint_hw_data;
extern struct plic_data plic_hw_data;
extern struct riscv_iommu_data riscv_iommu_data;

static const struct device_init_entry __attribute__((used))
    device_info[] __attribute__((section(".device_init_table"))) = {
#ifndef USE_QEMU
	{
	 "ns16550a",
	 0x310b0000,
	 0x10000,
	 40,
	 0,
	 0,
	  },
#else
	{
	 "qemu-8250",
	 0x10000000,
	 0x1000,
	 10,
	 0,
	 0,
	  },
#endif
#ifdef USE_QEMU
	{
	 "riscv,iommu",
	 0x10001000,
	 0x100,
	 0xFF,
	 0,
	 &riscv_iommu_data,
	  },
	{
	 "riscv,iommu_test",
	 0x10001000,
	 0x100,
	 0xff,
	 0,
	 0,
	  },
	{
	 "riscv,iommu_test2",
	 0x10001000,
	 0x100,
	 0xff,
	 1,
	 0,
	  },
	{
	 "riscv,iommu_test3",
	 0x10001000,
	 0x100,
	 0xff,
	 1,
	 0,
	  },
#endif
	{
	 "memory-map",
	 0x80000000,
	 0x80000000,
	 0xFF,
	 0,
	 0,
	  },
#ifndef USE_QEMU
	{
	 "PLIC",
	 0x3c000000,
	 0x4000000,
	 0xFF,
	 0,
	 &plic_hw_data,
	  },
#else
	{
	 "PLIC",
	 0xc000000,
	 0x4000000,
	 0xFF,
	 0,
	 &plic_hw_data,
	  },
#endif
#ifndef USE_QEMU
	{
	 "clint",
	 0x38000000,
	 0x10000,
	 0xFF,
	 0,
	 &clint_hw_data,
	  },
#else
	{
	 "clint",
	 0x2000000,
	 0x10000,
	 0xFF,
	 0,
	 &qemu_clint_hw_data,
	  },
#endif
#ifndef USE_QEMU
	{
	 "dw,dmac",
	 0x30040000,
	 0x10000,
	 4,
	 0,
	 0,
	  },
#endif
	{
	 "THE END",
	 0xFF,
	 0xFF,
	 0xFF,
	 0,
	 0,
	  },
};
