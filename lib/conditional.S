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
.global czero_nqz
czero_nqz:
	/*
	 * rs1 = a0 (value)
	 * rs2 = a1 (condition)
	 * rd = a0
	 * 0000111 01011 01010 111 01010 0110011
	 */
	.word 0xeb57533
	ret

.align 3
.global czero_eqz
czero_eqz:
	/*
	 * rs1 = a0 (value)
	 * rs2 = a1 (condition)
	 * rd = a0
	 * 0000111 01011 01010 101 01010 0110011
	 */
	.word 0xeb55533
	ret
