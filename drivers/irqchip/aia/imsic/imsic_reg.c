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

#include "imsic_reg.h"

void imsic_eix_update(unsigned long base_id,
		      unsigned long num_id, int pend, int val)
{
	unsigned long i, isel, ireg;
	unsigned long id = base_id, last_id = base_id + num_id;

	while (id < last_id) {
		isel = id / BITS_PER_LONG;
		isel *= BITS_PER_LONG / IMSIC_EIPx_BITS;
		isel += (pend) ? IMSIC_EIP0 : IMSIC_EIE0;

		ireg = 0;
		for (i = id & (__riscv_xlen - 1);
		     (id < last_id) && (i < __riscv_xlen); i++) {
			ireg |= 1 << i;
			id++;
		}

		/*
		 * The IMSIC EIEx and EIPx registers are indirectly
		 * accessed via using ISELECT and IREG CSRs so we
		 * need to access these CSRs without getting preempted.
		 *
		 * All existing users of this function call this
		 * function with local IRQs disabled so we don't
		 * need to do anything special here.
		 */
		if (val)
			imsic_csr_set(isel, ireg);
		else
			imsic_csr_clear(isel, ireg);
	}
}
