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

#ifndef __VMAP_H
#define __VMAP_H

void *ioremap(void *addr, unsigned int size, int gfp);
void iounmap(void *addr, unsigned int size);
void *vmem_map(void *addr, unsigned int size, int gfp);
void *vmap_alloc(unsigned int size);
void vmap_free(void *addr, unsigned int size);
void *vmem_alloc(unsigned int size, int gfp);
void *vmap_alloc_align(unsigned long align, unsigned int size);
void vmem_free(void *addr, unsigned int size);
void *vmem_alloc_lazy(unsigned int size, int gfp);
void *vmem_alloc_huge(unsigned int size, int page_size, int gfp);
void vmem_free_huge(void *addr, unsigned int size, int page_size);

#endif
