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

#ifndef _TYPE_H
#define _TYPE_H

#define NULL 0

#define U64_MAX (~(0ULL))
#define BITS_PER_LONG 64

#define INT_MAX (~(0U))

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

typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned long size_t;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif
