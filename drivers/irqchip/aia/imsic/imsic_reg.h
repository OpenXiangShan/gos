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

#ifndef _IMSIC_REG_H
#define _IMSIC_REG_H

#include <asm/csr.h>
#include <asm/type.h>

#define IMSIC_DISABLE_EIDELIVERY		0
#define IMSIC_ENABLE_EIDELIVERY			1
#define IMSIC_DISABLE_EITHRESHOLD		1
#define IMSIC_ENABLE_EITHRESHOLD		0

#define IMSIC_MMIO_PAGE_SHIFT           12
#define IMSIC_MMIO_PAGE_SZ              (1UL << IMSIC_MMIO_PAGE_SHIFT)
#define IMSIC_MMIO_PAGE_LE              0x00
#define IMSIC_MMIO_PAGE_BE              0x04

#define IMSIC_MIN_ID                    63
#define IMSIC_MAX_ID                    2048

#define IMSIC_EIDELIVERY                0x70

#define IMSIC_EITHRESHOLD               0x72

#define IMSIC_EIP0                      0x80
#define IMSIC_EIP63                     0xbf
#define IMSIC_EIPx_BITS                 32

#define IMSIC_EIE0                      0xc0
#define IMSIC_EIE63                     0xff
#define IMSIC_EIEx_BITS                 32

#define IMSIC_FIRST                     IMSIC_EIDELIVERY
#define IMSIC_LAST                      IMSIC_EIE63

#define IMSIC_MMIO_SETIPNUM_LE          0x00
#define IMSIC_MMIO_SETIPNUM_BE          0x04

#define imsic_csr_write(__c, __v)		\
do {						\
	write_csr(CSR_ISELECT, __c);		\
	write_csr(CSR_IREG, __v);		\
} while (0)

#define imsic_csr_read(__c)			\
({						\
	unsigned long __v;			\
	write_csr(CSR_ISELECT, __c);		\
	__v = read_csr(CSR_IREG);		\
	__v;					\
})

#define imsic_csr_set(__c, __v)			\
do {						\
	write_csr(CSR_ISELECT, __c);		\
	csr_set(CSR_IREG, __v);			\
} while (0)

#define imsic_csr_clear(__c, __v)		\
do {						\
	write_csr(CSR_ISELECT, __c);		\
	csr_clear(CSR_IREG, __v);		\
} while (0)

void imsic_eix_update(unsigned long base_id,
		      unsigned long num_id, int pend, int val);

static inline void imsic_id_enable(int id)
{
	imsic_eix_update(id, 1, 0, 1);
}

static inline void imsic_id_disable(int id)
{
	imsic_eix_update(id, 1, 0, 0);
}

#endif
