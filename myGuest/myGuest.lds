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
ENTRY(__start_guest)
SECTIONS
{
	. = CONFIG_MEM_START,
	.text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.data : { 
		*(.data)
		. = ALIGN(4096);
		*(.data.stack)
		.stack_top = .;
	}
	
	.rodata : { *(.rodata) }

	. = ALIGN(8);
	__command_init_table = .;
	.command_init_table : { *(.command_init_table) }
	__command_init_table_end = .;
	
	. = ALIGN(8);
	__driver_init_table = .;
	.driver_init_table : { *(.driver_init_table) }
	__driver_init_table_end = .;

	. = ALIGN(8);
	__pci_driver_init_table = .;
	.pci_driver_init_table : { *(.pci_driver_init_table) }
	__pci_driver_init_table_end = .;
	
	bss_begin = .;
        .bss : { *(.bss*) }
        bss_end = .;	
}
