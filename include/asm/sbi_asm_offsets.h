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

#ifndef __ASM_OFFSETS_H__
#define __ASM_OFFSETS_H__

#define PT_SIZE 264		/* sizeof(struct sbi_trap_regs) */
#define PT_MEPC 0		/* offsetof(struct sbi_trap_regs, sepc) */
#define PT_RA 8			/* offsetof(struct sbi_trap_regs, ra) */
#define PT_SP 16		/* offsetof(struct sbi_trap_regs, sp) */
#define PT_GP 24		/* offsetof(struct sbi_trap_regs, gp) */
#define PT_TP 32		/* offsetof(struct sbi_trap_regs, tp) */
#define PT_T0 40		/* offsetof(struct sbi_trap_regs, t0) */
#define PT_T1 48		/* offsetof(struct sbi_trap_regs, t1) */
#define PT_T2 56		/* offsetof(struct sbi_trap_regs, t2) */
#define PT_FP 64		/* offsetof(struct sbi_trap_regs, s0) */
#define PT_S0 64		/* offsetof(struct sbi_trap_regs, s0) */
#define PT_S1 72		/* offsetof(struct sbi_trap_regs, s1) */
#define PT_A0 80		/* offsetof(struct sbi_trap_regs, a0) */
#define PT_A1 88		/* offsetof(struct sbi_trap_regs, a1) */
#define PT_A2 96		/* offsetof(struct sbi_trap_regs, a2) */
#define PT_A3 104		/* offsetof(struct sbi_trap_regs, a3) */
#define PT_A4 112		/* offsetof(struct sbi_trap_regs, a4) */
#define PT_A5 120		/* offsetof(struct sbi_trap_regs, a5) */
#define PT_A6 128		/* offsetof(struct sbi_trap_regs, a6) */
#define PT_A7 136		/* offsetof(struct sbi_trap_regs, a7) */
#define PT_S2 144		/* offsetof(struct sbi_trap_regs, s2) */
#define PT_S3 152		/* offsetof(struct sbi_trap_regs, s3) */
#define PT_S4 160		/* offsetof(struct sbi_trap_regs, s4) */
#define PT_S5 168		/* offsetof(struct sbi_trap_regs, s5) */
#define PT_S6 176		/* offsetof(struct sbi_trap_regs, s6) */
#define PT_S7 184		/* offsetof(struct sbi_trap_regs, s7) */
#define PT_S8 192		/* offsetof(struct sbi_trap_regs, s8) */
#define PT_S9 200		/* offsetof(struct sbi_trap_regs, s9) */
#define PT_S10 208		/* offsetof(struct sbi_trap_regs, s10) */
#define PT_S11 216		/* offsetof(struct sbi_trap_regs, s11) */
#define PT_T3 224		/* offsetof(struct sbi_trap_regs, t3) */
#define PT_T4 232		/* offsetof(struct sbi_trap_regs, t4) */
#define PT_T5 240		/* offsetof(struct sbi_trap_regs, t5) */
#define PT_T6 248		/* offsetof(struct sbi_trap_regs, t6) */
#define PT_MSTATUS 256		/* offsetof(struct sbi_trap_regs, sstatus) */
#endif
