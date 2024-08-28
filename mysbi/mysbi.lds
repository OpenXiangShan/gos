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

OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS
{	
	. = 0x80000000,
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

	. = 0x801f0000,
	__boot_option_start = .;

	. = 0x80200000,
	__payload_start = .;

}
