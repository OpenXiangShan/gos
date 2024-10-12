/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
	 { 10,},
	 1,
	 " ",
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
	 " ",
	 0,
	 &plic_hw_data,
	  },
#else
	{
	 "IMSIC_M",
	 0x3a800000,
	 0x1000,
	 "INTC",
	 { 0xFF,},
	 0,
	 " ",
	 0,
	 &imsic_hw_data_m,
	  },
	{
	 "IMSIC",
	 0x3b000000,
	 0x2000,
	 "INTC",
	 { 0xFF,},
	 0,
	 " ",
	 0,
	 &imsic_hw_data,
	  },
	{
	 "APLIC_M",
	 0x31100000,
	 0x4000,
	 "APLIC_S",
	 { 0xFF,},
	 0,
	 " ",
	 0,
	 &aplic_hw_data_m,
	  },
	{
	 "APLIC_S",
	 0x31120000,
	 0x4000,
	 "IMSIC",
	 { 0xFF,},
	 0,
	 " ",
	 0,
	 &aplic_hw_data_s,
	  },
#endif
	{
	 "clint",
	 0x38000000,
	 0x10000,
	 "INTC",
	 { 0xFF,},
	 0,
	 " ",
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
	 " ",
	 0,
	 0,
	  },
#endif
#ifdef CONFIG_SELECT_AIA
	{
	 "imsic,test",
	 0x70000000,
	 0x1000,
	 "IMSIC",
	 { 0xff,},
	 0,
	 " ",
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
	 " ",
	 0,
	 0,
	  },
};

#endif
