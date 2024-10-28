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

#ifndef _MMIO_H
#define _MMIO_H

#define __arch_getl(a)			(*(volatile unsigned int *)((unsigned long)a))
#define __arch_putl(a,v)		(*(volatile unsigned int *)((unsigned long)a) = (v))

#define __arch_getb(a)			(*(volatile unsigned char *)((unsigned long)a))
#define __arch_putb(a,v)		(*(volatile unsigned char *)((unsigned long)a) = (v))

#define __arch_getq(a)			(*(volatile unsigned long *)((unsigned long)a))
#define __arch_putq(a,v)		(*(volatile unsigned long *)((unsigned long)a) = (v))

#define __iormb()	__asm__ __volatile__ ("fence i,r" : : : "memory")
#define __iowmb()	__asm__ __volatile__ ("fence w,o" : : : "memory")

#define readl(a)	({ unsigned int  __v = __arch_getl(a); __iormb(); __v; })
#define writel(a,v)	({ unsigned int  __v = v; __iowmb(); __arch_putl(a, __v);})

#define readb(a)	({ unsigned char  __v = __arch_getb(a); __iormb(); __v; })
#define writeb(a,v)	({ unsigned char  __v = v; __iowmb(); __arch_putb(a,__v);})

#define readq(a)	({ unsigned long  __v = __arch_getq(a); __iormb(); __v; })
#define writeq(a,v)	({ unsigned long  __v = v; __iowmb(); __arch_putq(a, __v);})

#endif
