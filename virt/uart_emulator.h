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

#ifndef __UART_EMULATOR_H
#define __UART_EMULATOR_H

int create_uart_device(struct virt_machine *machine, int id, unsigned long base,
		       unsigned int size);
int uart_device_finialize(struct virt_machine *machine, unsigned long gpa,
			  unsigned int size, int id, int pt);

#endif
