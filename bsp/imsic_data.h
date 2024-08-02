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

#ifndef __IMSIC_DATA_H
#define __IMSIC_DATA_H

struct imsic_priv_data {
	unsigned int guest_index_bits;
	unsigned int hart_index_bits;
	unsigned int group_index_bits;
	int nr_ids;
	int nr_harts;
	int nr_guests;
};
extern struct imsic_priv_data imsic_hw_data;
extern struct imsic_priv_data imsic_hw_data_m;

#endif
