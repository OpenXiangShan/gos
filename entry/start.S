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

#include "gos.h"

.section ".text.boot"
.global __start_gos
__start_gos:
	li t0, 4096
        la sp, stack
	add sp, sp, t0
	jal start_gos
1:
	j 1b

.align 3
.global secondary_start_sbi
secondary_start_sbi:
#ifdef CONFIG_ENABLE_MMU
	la t0, default_pgdp_pa
	ld t0, 0(t0)
	srli t0, t0, 12
	la t1, pgtable_l5_enabled
	lw t1, 0(t1)
	bnez t1, sv57
	la t1, pgtable_l4_enabled
	lw t1, 0(t1)
	bnez t1, sv48
sv39:
	li t1, 0x8000000000000000
	j 2f
sv57:
	li t1, 0xa000000000000000
	j 2f
sv48:
	li t1, 0x9000000000000000
2:
	or t0, t0, t1
	sfence.vma
	csrw satp, t0
#endif
	la t0, secondary_cpus_init_stack
	ld sp, 0(t0)

	la t0, secondary_cpus_init_stack_size
	lw t0, 0(t0)

	beqz t0, 1f

	add sp, sp, t0
	li t0, 0x1000
	mul a5, t0, a0
	sub sp, sp, a5	
	
	la a5, do_exception_vector
	csrw stvec, a5 	

	csrw sscratch, sp
	
	mv a1, sp
	jal secondary_cpus_init
1:
	j 1b

.section ".data.stack"
stack:
	.skip 4096
