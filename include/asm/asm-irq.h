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

#ifndef _ASM_IRQ_H
#define _ASM_IRQ_H

#include <asm/csr.h>

#define local_irq_save(flags) \
	(flags) = csr_read_clear(sstatus, SR_SIE);

#define local_irq_restore(flags) \
	csr_set(sstatus, (flags) & SR_SIE);

static inline void __enable_local_irq(void)
{
	csr_set(sstatus, SR_SIE);
}

static inline void __disable_local_irq(void)
{
	csr_clear(sstatus, SR_SIE);
}

static inline int __local_irq_is_on(void)
{
	return (read_csr(sstatus) & SR_SIE);
}

#endif
