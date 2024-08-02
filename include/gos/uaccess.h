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

#ifndef __UACCESS_H
#define __UACCESS_H

#include "asm/csr.h"
#include "string.h"

static inline int copy_from_user(void *user_ptr, void *addr, unsigned int size)
{
	csr_set(CSR_SSTATUS, SR_SUM);

	memcpy((char *)addr, (char *)user_ptr, size);

	csr_clear(CSR_SSTATUS, SR_SUM);

	return 0;
}

static inline int copy_to_user(void *user_ptr, void *addr, unsigned int size)
{
	csr_set(CSR_SSTATUS, SR_SUM);

	memcpy((char *)user_ptr, (char *)addr, size);

	csr_clear(CSR_SSTATUS, SR_SUM);

	return 0;
}

#endif
