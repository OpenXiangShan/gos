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

#include "clint.h"

struct clint_data clint_hw_data = {
	.clint_freq = 500000,
};

struct clint_data clint_kmh_hw_data = {
	.clint_freq = 1000000,
};

struct clint_data qemu_clint_hw_data = {
	.clint_freq = 10000000,
};

struct clint_data st_clint_hw_data = {
	.clint_freq = 1000000,
};
