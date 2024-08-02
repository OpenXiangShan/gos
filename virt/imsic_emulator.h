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

#ifndef __IMSIC_EMULATOR_H__
#define __IMSIC_EMULATOR_H__

int create_imsic_device(struct virt_machine *machine, int id,
			unsigned long base, unsigned int size);
int imsic_device_finialize(struct virt_machine *machine, unsigned long base,
			   unsigned int size, int id, int pt);
int imsic_gstage_ioremap(unsigned long *pgdp, unsigned long hpa,
			 unsigned long gpa, unsigned int size);

#endif
