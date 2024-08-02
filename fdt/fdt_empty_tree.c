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

// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2012 David Gibson, IBM Corporation.
 */
#include "libfdt_env.h"

#include "fdt.h"
#include "libfdt.h"

#include "libfdt_internal.h"

int fdt_create_empty_tree(void *buf, int bufsize)
{
	int err;

	err = fdt_create(buf, bufsize);
	if (err)
		return err;

	err = fdt_finish_reservemap(buf);
	if (err)
		return err;

	err = fdt_begin_node(buf, "");
	if (err)
		return err;

	err = fdt_end_node(buf);
	if (err)
		return err;

	err = fdt_finish(buf);
	if (err)
		return err;

	return fdt_open_into(buf, buf, bufsize);
}
