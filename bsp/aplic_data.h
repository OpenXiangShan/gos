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

#ifndef _APLIC_DATA_H
#define _APLIC_DATA_H

#include "imsic_data.h"

struct aplic_priv_data {
	int index;
	int mmode;
	int mode;
	int nr_irqs;
	int delegate;
	int child_index;
	struct imsic_priv_data *imsic_data;
};

extern struct aplic_priv_data aplic_hw_data_m;
extern struct aplic_priv_data aplic_hw_data_s;

#endif
