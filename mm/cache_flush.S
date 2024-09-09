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
.global cbo_cache_inval
cbo_cache_inval:
	/*
	 * rs1 = a0 (base)
	 * cbo.inval base
	 * 000000000000 01010 010 00000 0001111
	 */
	.word 0x5200f
	ret

.align 3
.global cbo_cache_clean
cbo_cache_clean:
	/*
	 * rs1 = a0 (base)
	 * cbo.clean base
	 * 000000000001 01010 010 00000 0001111
	 */
	.word 0x15200f
	ret

.align 3
.global cbo_cache_flush
cbo_cache_flush:
	/*
	 * rs1 = a0 (base)
	 * cbo.flush base
	 * 000000000010 01010 010 00000 0001111 
	 */
	.word 0x25200f
	ret

.align 3
.global cbo_cache_zero
cbo_cache_zero:
	/*
	 * rs1 = a0 (base)
	 * 000000000100 01010 010 00000 0001111
	 */
	.word 0x45200f
	ret

.align 3
.global prefetch_i
prefetch_i:
	/*
	 * rs1 = a0 (base)
	 * 0000000 00000 01010 110 00000 0010011
	 */
	.word 0x56013
	.word 0x13600500
	ret

.align 3
.global prefetch_r
prefetch_r:
	/*
	 * rs1 = a0 (base)
	 * 0000000 00001 01010 110 00000 0010011
	 */
	.word 0x13601500
	ret

.align 3
.global prefetch_w
prefetch_w:
	/*
	 * rs1 = a0 (base)
	 * 0000000 00011 01010 110 00000 0010011
	 */
	.word 0x13603500
	ret
