#include "gos/autoconf.h"
#ifdef CONFIG_SELECT_QEMU

#include <device.h>
#include "plic.h"
#include "clint.h"
#include "riscv_iommu_data.h"
#include "imsic_data.h"
#include "aplic_data.h"

extern struct clint_data clint_hw_data;
extern struct clint_data qemu_clint_hw_data;
extern struct plic_data plic_hw_data;
extern struct riscv_iommu_data riscv_iommu_data;

static const struct device_init_entry __attribute__((used))
    device_info[] __attribute__((section(".device_init_table"))) = {
	{
	 "qemu-8250",
	 0x10000000,
	 0x1000,
#ifndef CONFIG_SELECT_AIA
	 "PLIC",
#else
	 "APLIC_S",
#endif
	 { 10,},
	 1,
	 0,
	 0,
	  },
#ifdef IOMMU_PTEWALK_TEST
	{
	 "riscv,iommu",
	 0x10001000,
	 0x100,
	 "PLIC",
	 { 0xFF,},
	 0,
	 0,
	 &riscv_iommu_data,
	  },
	{
	 "riscv,iommu_test",
	 0x10001000,
	 0x100,
	 "PLIC",
	 { 0xff,},
	 0,
	 0,
	 0,
	  },
	{
	 "riscv,iommu_test2",
	 0x10001000,
	 0x100,
	 "PLIC",
	 { 0xff,},
	 0,
	 1,
	 0,
	  },
	{
	 "riscv,iommu_test3",
	 0x10001000,
	 0x100,
	 "PLIC",
	 { 0xff,},
	 0,
	 1,
	 0,
	  },
	{
	 "riscv,iommu_test_two_stage",
	 0x10001000,
	 0x100,
	 "PLIC",
	 { 0xff,},
	 0,
	 1,
	 0,
	  },
	{
	 "riscv,iommu_test_two_stage2",
	 0x10001000,
	 0x100,
	 "PLIC",
	 { 0xff,},
	 0,
	 2,
	 0,
	  },
#endif
#ifndef CONFIG_SELECT_AIA
	{
	 "PLIC",
	 0xc000000,
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
	{
	 "clint",
	 0x2000000,
	 0x10000,
	 "INTC",
	 { 0xFF,},
	 0,
	 0,
	 &qemu_clint_hw_data,
	  },
#ifdef CONFIG_SELECT_AIA
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