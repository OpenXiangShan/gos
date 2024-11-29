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

#ifndef __ALIGN_H__
#define __ALIGN_H__

#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN(p, a) ALIGN_MASK(p, (typeof(p))(a) - 1)
#define PTR_ALIGN(p, a) (typeof(p))ALIGN((unsigned long)p, a)
#define ALIGN_SIZE(addr, size) (((addr) + (size) - 1) & (~((unsigned long)(size) - 1)))
#define ALIGN_SIZE_UP(addr, size) ((addr) & ~((unsigned long)(size) - 1))


#define RESIZE(v, up) ((v / up == 0) ?  (v) : ((v / up + 1)*up))

#endif
