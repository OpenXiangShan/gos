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
#include "gos-auto/autoconf.h"

OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS
{	
	. = CONFIG_MEM_START,
	.text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.data : { *(.data) }
	.rodata : { *(.rodata) }
	bss_begin = .;
        .bss : { *(.bss*) }
        bss_end = .;

	. = ALIGN(0x1000);
	__device_init_table = .;
	.device_init_table : { *(.device_init_table) }
	__device_init_table_end = .;

	. = CONFIG_MEM_START + 0x10000,
	__boot_option_start = .;

	. = CONFIG_MEM_START + 0x20000,
	__payload_start = .;

}
