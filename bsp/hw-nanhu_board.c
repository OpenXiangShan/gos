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
#ifdef CONFIG_SELECT_NANHU_BOARD

#include "device.h"
#include "plic.h"
#include "clint.h"

extern struct clint_data clint_hw_data;
extern struct plic_data plic_hw_data;

static const struct device_init_entry __attribute__((used))
    device_info[] __attribute__((section(".device_init_table"))) = {
	{
	 "ns16550a",
	 0x50000,
	 0x10000,
	 "APLIC_S",
	 { 40,},
	 1,
	 " ",
	 0,
	 0,
	  },
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
	{
	 "clint",
	 0x38000000,
	 0x10000,
	 "INTC",
	 { 0xFF,},
	 0,
	 " ",
	 0,
	 &clint_hw_data,
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



