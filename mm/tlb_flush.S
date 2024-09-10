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

.align 3
.global sinval_all
sinval_all:
	/*
	 * sinval all
	 * 0001011 00000 00000 000 00000 1110011
	 */
	 .word 0x16000073
	 ret

.align 3
.global sinval_va
sinval_va:
	/*
	 * rs1 = a0 (va)
	 * rs2 = zero
	 * sinval a0
	 * 0001011 00000 01010 000 00000 1110011
	 */
	 .word 0x16050073
	 ret

.align 3
.global sinval_asid
sinval_asid:
	/*
	 * rs1 = zero
	 * rs2 = a0 (asid)
	 * sinval a0
	 * 0001011 01010 00000 000 00000 1110011
	 */
	 .word 0x16a00073
	 ret

.global sinval_va_asid
sinval_va_asid:
	/*
	 * rs1 = a0 (va)
	 * rs2 = a1 (asid)
	 * sinval a0
	 * 0001011 01011 01010 000 00000 1110011
	 */
	 .word 0x16b50073
	 ret
