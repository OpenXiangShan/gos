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

#include "aplic_data.h"

#define APLIC_DIRECT_MODE  0
#define APLIC_MSI_MODE     1

#define APLIC_M_MODE 0
#define APLIC_S_MODE 1

extern struct imsic_priv_data imsic_hw_data;

struct aplic_priv_data aplic_hw_data_m = {
	0xff,
	APLIC_M_MODE,
	APLIC_MSI_MODE,
	64,
	1,
	0,
	&imsic_hw_data,
};

struct aplic_priv_data aplic_hw_data_s = {
	0,
	APLIC_S_MODE,
	APLIC_MSI_MODE,
	64,
	0,
	0xffff,
	&imsic_hw_data,
};
