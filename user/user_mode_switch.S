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

#include "asm/asm-offsets.h"
#include "asm/csr.h"

.section .text
.align 2
.global __user_mode_switch_return
__user_mode_switch_return:
	csrrw a0, CSR_SSCRATCH, a0

	sd ra, (USER_CPU_U_RA)(a0)
	sd sp, (USER_CPU_U_SP)(a0)
	sd gp, (USER_CPU_U_GP)(a0)
	sd tp, (USER_CPU_U_TP)(a0)
	sd a1, (USER_CPU_U_A1)(a0)
	sd a2, (USER_CPU_U_A2)(a0)
	sd a3, (USER_CPU_U_A3)(a0)
	sd a4, (USER_CPU_U_A4)(a0)
	sd a5, (USER_CPU_U_A5)(a0)
	sd a6, (USER_CPU_U_A6)(a0)
	sd a7, (USER_CPU_U_A7)(a0)
	sd s0, (USER_CPU_U_S0)(a0)
	sd s1, (USER_CPU_U_S1)(a0)
	sd s2, (USER_CPU_U_S2)(a0)
	sd s3, (USER_CPU_U_S3)(a0)
	sd s4, (USER_CPU_U_S4)(a0)
	sd s5, (USER_CPU_U_S5)(a0)
	sd s6, (USER_CPU_U_S6)(a0)
	sd s7, (USER_CPU_U_S7)(a0)
	sd s8, (USER_CPU_U_S8)(a0)
	sd s9, (USER_CPU_U_S9)(a0)
	sd s10, (USER_CPU_U_S10)(a0)
	sd s11, (USER_CPU_U_S11)(a0)
	sd t0, (USER_CPU_U_T0)(a0)
	sd t1, (USER_CPU_U_T1)(a0)
	sd t2, (USER_CPU_U_T2)(a0)
	sd t3, (USER_CPU_U_T3)(a0)
	sd t4, (USER_CPU_U_T4)(a0)
	sd t5, (USER_CPU_U_T5)(a0)
	sd t6, (USER_CPU_U_T6)(a0)

	fsd f0,  (USER_U_F0)(a0)
	fsd f1,  (USER_U_F1)(a0)
	fsd f2,  (USER_U_F2)(a0)
	fsd f3,  (USER_U_F3)(a0)
	fsd f4,  (USER_U_F4)(a0)
	fsd f5,  (USER_U_F5)(a0)
	fsd f6,  (USER_U_F6)(a0)
	fsd f7,  (USER_U_F7)(a0)
	fsd f8,  (USER_U_F8)(a0)
	fsd f9,  (USER_U_F9)(a0)
	fsd f10, (USER_U_F10)(a0)
	fsd f11, (USER_U_F11)(a0)
	fsd f12, (USER_U_F12)(a0)
	fsd f13, (USER_U_F13)(a0)
	fsd f14, (USER_U_F14)(a0)
	fsd f15, (USER_U_F15)(a0)
	fsd f16, (USER_U_F16)(a0)
	fsd f17, (USER_U_F17)(a0)
	fsd f18, (USER_U_F18)(a0)
	fsd f19, (USER_U_F19)(a0)
	fsd f20, (USER_U_F20)(a0)
	fsd f21, (USER_U_F21)(a0)
	fsd f22, (USER_U_F22)(a0)
	fsd f23, (USER_U_F23)(a0)
	fsd f24, (USER_U_F24)(a0)
	fsd f25, (USER_U_F25)(a0)
	fsd f26, (USER_U_F26)(a0)
	fsd f27, (USER_U_F27)(a0)
	fsd f28, (USER_U_F28)(a0)
	fsd f29, (USER_U_F29)(a0)
	fsd f30, (USER_U_F30)(a0)
	fsd f31, (USER_U_F31)(a0)

#if CONFIG_ENABLE_VECTOR
	csrr s1, CSR_VTYPE
	sd s1, USER_U_VTYPE(a0)

	csrr s1, CSR_VL
	sd s1, USER_U_VL(a0)

	csrr s1, CSR_VLENB
	sd s1, USER_U_VLENB(a0)

	csrr s1, CSR_VCSR
	sd s1, USER_U_VCSR(a0)

	csrr s1, CSR_VSTART
	sd s1, USER_U_VSTART(a0)

	vsetvli t0, x0, e8, m8, ta, ma
	addi s3, a0, USER_U_V0
	vse8.v v0, (s3)
	add s3, s3, t0
	vse8.v v8, (s3)
	add s3, s3, t0
	vse8.v v16, (s3)
	add s3, s3, t0
	vse8.v v24, (s3)
#endif
	li s3, USER_CPU_S_STVEC
	add s3, a0, s3
	ld t1, (s3)

	li s3, USER_CPU_S_SSCRATCH
	add s3, a0, s3
	ld t2, (s3)

	li s3, USER_CPU_S_SSTATUS
	add s3, a0, s3
	ld t5, (s3)

	csrr t0, CSR_SEPC
	csrw CSR_STVEC, t1
	csrrw t2, CSR_SSCRATCH, t2
	csrrw t5, CSR_SSTATUS, t5

	sd t0, (USER_CPU_U_SEPC)(a0)
	sd t2, (USER_CPU_U_A0)(a0)
	sd t5, (USER_CPU_U_SSTATUS)(a0)

#if CONFIG_ENABLE_VECTOR
	vsetvli t0, x0, e8, m8, ta, ma
	addi s3, a0, USER_S_V0
	vle8.v v0, (s3)
	add s3, s3, t0
	vle8.v v8, (s3)
	add s3, s3, t0
	vle8.v v16, (s3)
	add s3, s3, t0
	vle8.v v24, (s3)

	ld s3, USER_S_VTYPE(a0)
	ld s4, USER_S_VL(a0)
	vsetvl x0, s4, s3

	ld s3, USER_U_VCSR(a0)
	csrw CSR_VCSR, s3

	ld s3, USER_U_VSTART(a0)
	csrw CSR_VSTART, s3
#endif
	ld ra, (USER_CPU_S_RA)(a0)
	ld sp, (USER_CPU_S_SP)(a0)
	ld gp, (USER_CPU_S_GP)(a0)
	ld tp, (USER_CPU_S_TP)(a0)
	ld a1, (USER_CPU_S_A1)(a0)
	ld a2, (USER_CPU_S_A2)(a0)
	ld a3, (USER_CPU_S_A3)(a0)
	ld a4, (USER_CPU_S_A4)(a0)
	ld a5, (USER_CPU_S_A5)(a0)
	ld a6, (USER_CPU_S_A6)(a0)
	ld a7, (USER_CPU_S_A7)(a0)
	ld s0, (USER_CPU_S_S0)(a0)
	ld s1, (USER_CPU_S_S1)(a0)
	ld s2, (USER_CPU_S_S2)(a0)
	ld s3, (USER_CPU_S_S3)(a0)
	ld s4, (USER_CPU_S_S4)(a0)
	ld s5, (USER_CPU_S_S5)(a0)
	ld s6, (USER_CPU_S_S6)(a0)
	ld s7, (USER_CPU_S_S7)(a0)
	ld s8, (USER_CPU_S_S8)(a0)
	ld s9, (USER_CPU_S_S9)(a0)
	ld s10, (USER_CPU_S_S10)(a0)
	ld s11, (USER_CPU_S_S11)(a0)

	fld f0,  (USER_S_F0)(a0)
	fld f1,  (USER_S_F1)(a0)
	fld f2,  (USER_S_F2)(a0)
	fld f3,  (USER_S_F3)(a0)
	fld f4,  (USER_S_F4)(a0)
	fld f5,  (USER_S_F5)(a0)
	fld f6,  (USER_S_F6)(a0)
	fld f7,  (USER_S_F7)(a0)
	fld f8,  (USER_S_F8)(a0)
	fld f9,  (USER_S_F9)(a0)
	fld f10, (USER_S_F10)(a0)
	fld f11, (USER_S_F11)(a0)
	fld f12, (USER_S_F12)(a0)
	fld f13, (USER_S_F13)(a0)
	fld f14, (USER_S_F14)(a0)
	fld f15, (USER_S_F15)(a0)
	fld f16, (USER_S_F16)(a0)
	fld f17, (USER_S_F17)(a0)
	fld f18, (USER_S_F18)(a0)
	fld f19, (USER_S_F19)(a0)
	fld f20, (USER_S_F20)(a0)
	fld f21, (USER_S_F21)(a0)
	fld f22, (USER_S_F22)(a0)
	fld f23, (USER_S_F23)(a0)
	fld f24, (USER_S_F24)(a0)
	fld f25, (USER_S_F25)(a0)
	fld f26, (USER_S_F26)(a0)
	fld f27, (USER_S_F27)(a0)
	fld f28, (USER_S_F28)(a0)
	fld f29, (USER_S_F29)(a0)
	fld f30, (USER_S_F30)(a0)
	fld f31, (USER_S_F31)(a0)

	ret

.section .text
.align 2
.global user_mode_switch_to
user_mode_switch_to:
	sd ra, (USER_CPU_S_RA)(a0)
	sd sp, (USER_CPU_S_SP)(a0)
	sd gp, (USER_CPU_S_GP)(a0)
	sd tp, (USER_CPU_S_TP)(a0)
	sd s0, (USER_CPU_S_S0)(a0)
	sd s1, (USER_CPU_S_S1)(a0)
	sd a2, (USER_CPU_S_A2)(a0)
	sd a3, (USER_CPU_S_A3)(a0)
	sd a4, (USER_CPU_S_A4)(a0)
	sd a5, (USER_CPU_S_A5)(a0)
	sd a6, (USER_CPU_S_A6)(a0)
	sd a7, (USER_CPU_S_A7)(a0)
	sd s2, (USER_CPU_S_S2)(a0)
	sd s3, (USER_CPU_S_S3)(a0)
	sd s4, (USER_CPU_S_S4)(a0)
	sd s5, (USER_CPU_S_S5)(a0)
	sd s6, (USER_CPU_S_S6)(a0)
	sd s7, (USER_CPU_S_S7)(a0)
	sd s8, (USER_CPU_S_S8)(a0)
	sd s9, (USER_CPU_S_S9)(a0)
	sd s10, (USER_CPU_S_S10)(a0)
	sd s11, (USER_CPU_S_S11)(a0)

	fsd f0,  (USER_S_F0)(a0)
	fsd f1,  (USER_S_F1)(a0)
	fsd f2,  (USER_S_F2)(a0)
	fsd f3,  (USER_S_F3)(a0)
	fsd f4,  (USER_S_F4)(a0)
	fsd f5,  (USER_S_F5)(a0)
	fsd f6,  (USER_S_F6)(a0)
	fsd f7,  (USER_S_F7)(a0)
	fsd f8,  (USER_S_F8)(a0)
	fsd f9,  (USER_S_F9)(a0)
	fsd f10, (USER_S_F10)(a0)
	fsd f11, (USER_S_F11)(a0)
	fsd f12, (USER_S_F12)(a0)
	fsd f13, (USER_S_F13)(a0)
	fsd f14, (USER_S_F14)(a0)
	fsd f15, (USER_S_F15)(a0)
	fsd f16, (USER_S_F16)(a0)
	fsd f17, (USER_S_F17)(a0)
	fsd f18, (USER_S_F18)(a0)
	fsd f19, (USER_S_F19)(a0)
	fsd f20, (USER_S_F20)(a0)
	fsd f21, (USER_S_F21)(a0)
	fsd f22, (USER_S_F22)(a0)
	fsd f23, (USER_S_F23)(a0)
	fsd f24, (USER_S_F24)(a0)
	fsd f25, (USER_S_F25)(a0)
	fsd f26, (USER_S_F26)(a0)
	fsd f27, (USER_S_F27)(a0)
	fsd f28, (USER_S_F28)(a0)
	fsd f29, (USER_S_F29)(a0)
	fsd f30, (USER_S_F30)(a0)
	fsd f31, (USER_S_F31)(a0)

#if CONFIG_ENABLE_VECTOR
	csrr s1, CSR_VTYPE
	sd s1, USER_S_VTYPE(a0)

	csrr s1, CSR_VL
	sd s1, USER_S_VL(a0)

	csrr s1, CSR_VLENB
	sd s1, USER_S_VLENB(a0)

	csrr s1, CSR_VCSR
	sd s1, USER_S_VCSR(a0)

	csrr s1, CSR_VSTART
	sd s1, USER_S_VSTART(a0)

	vsetvli t0, x0, e8, m8, ta, ma
	addi s3, a0, USER_S_V0
	vse8.v v0, (s3)
	add s3, s3, t0
	vse8.v v8, (s3)
	add s3, s3, t0
	vse8.v v16, (s3)
	add s3, s3, t0
	vse8.v v24, (s3)
#endif
	ld t0, (USER_CPU_U_SSTATUS)(a0)
	la t4, __user_mode_switch_return
	ld t5, (USER_CPU_U_SEPC)(a0)

	csrrw t0, CSR_SSTATUS, t0
	csrrw t4, CSR_STVEC, t4
	csrw CSR_SEPC, t5

	csrrw t3, CSR_SSCRATCH, a0

	li s3, USER_CPU_S_SSTATUS
	add s3, a0, s3
	sd t0, (s3)

	li s3, USER_CPU_S_HSTATUS
	add s3, a0, s3
	sd t1, (s3)

	li s3, USER_CPU_S_SSCRATCH
	add s3, a0, s3
	sd t3, (s3)

	li s3, USER_CPU_S_STVEC
	add s3, a0, s3
	sd t4, (s3)

#if CONFIG_ENABLE_VECTOR
	vsetvli t0, x0, e8, m8, ta, ma
	addi s3, a0, USER_U_V0
	vle8.v v0, (s3)
	add s3, s3, t0
	vle8.v v8, (s3)
	add s3, s3, t0
	vle8.v v16, (s3)
	add s3, s3, t0
	vle8.v v24, (s3)

	ld s3, USER_U_VTYPE(a0)
	ld s4, USER_U_VL(a0)
	vsetvl x0, s4, s3

	ld s3, USER_U_VCSR(a0)
	csrw CSR_VCSR, s3

	ld s3, USER_U_VSTART(a0)
	csrw CSR_VSTART, s3
#endif
	ld ra, (USER_CPU_U_RA)(a0)
	ld sp, (USER_CPU_U_SP)(a0)
	ld gp, (USER_CPU_U_GP)(a0)
	ld tp, (USER_CPU_U_TP)(a0)
	ld s0, (USER_CPU_U_S3)(a0)
	ld s1, (USER_CPU_U_S1)(a0)
	ld a1, (USER_CPU_U_A1)(a0)
	ld a2, (USER_CPU_U_A2)(a0)
	ld a3, (USER_CPU_U_A3)(a0)
	ld a4, (USER_CPU_U_A4)(a0)
	ld a5, (USER_CPU_U_A5)(a0)
	ld a6, (USER_CPU_U_A6)(a0)
	ld a7, (USER_CPU_U_A7)(a0)
	ld s2, (USER_CPU_U_S2)(a0)
	ld s3, (USER_CPU_U_S3)(a0)
	ld s4, (USER_CPU_U_S4)(a0)
	ld s5, (USER_CPU_U_S5)(a0)
	ld s6, (USER_CPU_U_S6)(a0)
	ld s7, (USER_CPU_U_S7)(a0)
	ld s8, (USER_CPU_U_S8)(a0)
	ld s9, (USER_CPU_U_S9)(a0)
	ld s10, (USER_CPU_U_S10)(a0)
	ld s11, (USER_CPU_U_S11)(a0)
	ld t0, (USER_CPU_U_T0)(a0)
	ld t1, (USER_CPU_U_T1)(a0)
	ld t2, (USER_CPU_U_T2)(a0)
	ld t3, (USER_CPU_U_T3)(a0)
	ld t4, (USER_CPU_U_T4)(a0)
	ld t5, (USER_CPU_U_T5)(a0)
	ld t6, (USER_CPU_U_T6)(a0)

	fld f0,  (USER_U_F0)(a0)
	fld f1,  (USER_U_F1)(a0)
	fld f2,  (USER_U_F2)(a0)
	fld f3,  (USER_U_F3)(a0)
	fld f4,  (USER_U_F4)(a0)
	fld f5,  (USER_U_F5)(a0)
	fld f6,  (USER_U_F6)(a0)
	fld f7,  (USER_U_F7)(a0)
	fld f8,  (USER_U_F8)(a0)
	fld f9,  (USER_U_F9)(a0)
	fld f10, (USER_U_F10)(a0)
	fld f11, (USER_U_F11)(a0)
	fld f12, (USER_U_F12)(a0)
	fld f13, (USER_U_F13)(a0)
	fld f14, (USER_U_F14)(a0)
	fld f15, (USER_U_F15)(a0)
	fld f16, (USER_U_F16)(a0)
	fld f17, (USER_U_F17)(a0)
	fld f18, (USER_U_F18)(a0)
	fld f19, (USER_U_F19)(a0)
	fld f20, (USER_U_F20)(a0)
	fld f21, (USER_U_F21)(a0)
	fld f22, (USER_U_F22)(a0)
	fld f23, (USER_U_F23)(a0)
	fld f24, (USER_U_F24)(a0)
	fld f25, (USER_U_F25)(a0)
	fld f26, (USER_U_F26)(a0)
	fld f27, (USER_U_F27)(a0)
	fld f28, (USER_U_F28)(a0)
	fld f29, (USER_U_F29)(a0)
	fld f30, (USER_U_F30)(a0)
	fld f31, (USER_U_F31)(a0)

	ld a0, (USER_CPU_U_A0)(a0)

	sret

