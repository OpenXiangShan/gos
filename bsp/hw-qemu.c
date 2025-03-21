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
#ifdef CONFIG_SELECT_QEMU

#include <device.h>
#include "plic.h"
#include "clint.h"
#include "riscv_iommu_data.h"
#include "imsic_data.h"
#include "aplic_data.h"
#include "pci_data.h"
#include "uart_data.h"

extern struct clint_data clint_hw_data;
extern struct clint_data qemu_clint_hw_data;
extern struct plic_data plic_hw_data;
extern struct riscv_iommu_data riscv_iommu_data;
extern struct pci_data generic_ecam_pci_data;
extern struct uart_data qemu8250_uart_data;

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
	 " ",
	 0,
	 &qemu8250_uart_data,
	  },
	{
	 "riscv,iommu",
	 0x10001000,
	 0x100,
#ifndef CONFIG_SELECT_AIA
	 "PLIC",
#else
	 "APLIC_S",
#endif
	 { 0xFF,},
	 0,
	 " ",
	 0,
	 &riscv_iommu_data,
	  },
#ifndef CONFIG_SELECT_AIA
	{
	 "PLIC",
	 0xc000000,
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
	 0x24000000,
	 0x4000000,
	 "INTC",
	 { 0xFF,},
	 0,
	 " ",
	 0,
	 &imsic_hw_data_m,
	  },
	{
	 "IMSIC",
	 0x28000000,
	 0x4000000,
	 "INTC",
	 { 0xFF,},
	 0,
	 " ",
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
	 " ",
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
	 " ",
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
	 " ",
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
	 " ",
	 0,
	 0,
	  },
#endif
	{
	 "pci-host-ecam-generic",
	 0x30000000,
	 0x10000000,
	 " ",
	 { 0xFF,},
	 0,
	 " ",
	 0,
	 &generic_ecam_pci_data,
	  },
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
