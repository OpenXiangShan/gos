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
.global __vcpu_switch_return
__vcpu_switch_return:
	/* struct vcpu ptr is in sscratch */
	csrrw a0, CSR_SSCRATCH, a0
	
	sd ra, (VIRT_CPU_GUEST_RA)(a0)
	sd sp, (VIRT_CPU_GUEST_SP)(a0)
	sd gp, (VIRT_CPU_GUEST_GP)(a0)
	sd tp, (VIRT_CPU_GUEST_TP)(a0)
	sd a1, (VIRT_CPU_GUEST_A1)(a0)
	sd a2, (VIRT_CPU_GUEST_A2)(a0)
	sd a3, (VIRT_CPU_GUEST_A3)(a0)
	sd a4, (VIRT_CPU_GUEST_A4)(a0)
	sd a5, (VIRT_CPU_GUEST_A5)(a0)
	sd a6, (VIRT_CPU_GUEST_A6)(a0)
	sd a7, (VIRT_CPU_GUEST_A7)(a0)
	sd s0, (VIRT_CPU_GUEST_S0)(a0)
	sd s1, (VIRT_CPU_GUEST_S1)(a0)
	sd s2, (VIRT_CPU_GUEST_S2)(a0)
	sd s3, (VIRT_CPU_GUEST_S3)(a0)
	sd s4, (VIRT_CPU_GUEST_S4)(a0)
	sd s5, (VIRT_CPU_GUEST_S5)(a0)
	sd s6, (VIRT_CPU_GUEST_S6)(a0)
	sd s7, (VIRT_CPU_GUEST_S7)(a0)
	sd s8, (VIRT_CPU_GUEST_S8)(a0)
	sd s9, (VIRT_CPU_GUEST_S9)(a0)
	sd s10, (VIRT_CPU_GUEST_S10)(a0)
	sd s11, (VIRT_CPU_GUEST_S11)(a0)
	sd t0, (VIRT_CPU_GUEST_T0)(a0)
	sd t1, (VIRT_CPU_GUEST_T1)(a0)
	sd t2, (VIRT_CPU_GUEST_T2)(a0)
	sd t3, (VIRT_CPU_GUEST_T3)(a0)
	sd t4, (VIRT_CPU_GUEST_T4)(a0)
	sd t5, (VIRT_CPU_GUEST_T5)(a0)
	sd t6, (VIRT_CPU_GUEST_T6)(a0)

	fsd f0,  (VIRT_G_F0)(a0)
	fsd f1,  (VIRT_G_F1)(a0)
	fsd f2,  (VIRT_G_F2)(a0)
	fsd f3,  (VIRT_G_F3)(a0)
	fsd f4,  (VIRT_G_F4)(a0)
	fsd f5,  (VIRT_G_F5)(a0)
	fsd f6,  (VIRT_G_F6)(a0)
	fsd f7,  (VIRT_G_F7)(a0)
	fsd f8,  (VIRT_G_F8)(a0)
	fsd f9,  (VIRT_G_F9)(a0)
	fsd f10, (VIRT_G_F10)(a0)
	fsd f11, (VIRT_G_F11)(a0)
	fsd f12, (VIRT_G_F12)(a0)
	fsd f13, (VIRT_G_F13)(a0)
	fsd f14, (VIRT_G_F14)(a0)
	fsd f15, (VIRT_G_F15)(a0)
	fsd f16, (VIRT_G_F16)(a0)
	fsd f17, (VIRT_G_F17)(a0)
	fsd f18, (VIRT_G_F18)(a0)
	fsd f19, (VIRT_G_F19)(a0)
	fsd f20, (VIRT_G_F20)(a0)
	fsd f21, (VIRT_G_F21)(a0)
	fsd f22, (VIRT_G_F22)(a0)
	fsd f23, (VIRT_G_F23)(a0)
	fsd f24, (VIRT_G_F24)(a0)
	fsd f25, (VIRT_G_F25)(a0)
	fsd f26, (VIRT_G_F26)(a0)
	fsd f27, (VIRT_G_F27)(a0)
	fsd f28, (VIRT_G_F28)(a0)
	fsd f29, (VIRT_G_F29)(a0)
	fsd f30, (VIRT_G_F30)(a0)
	fsd f31, (VIRT_G_F31)(a0)

	ld t1, (VIRT_CPU_HOST_STVEC)(a0)	
	ld t2, (VIRT_CPU_HOST_SSCRATCH)(a0)	
	ld t4, (VIRT_CPU_HOST_HSTATUS)(a0)	
	ld t5, (VIRT_CPU_HOST_SSTATUS)(a0)	

	csrr t0, CSR_SEPC
	csrw CSR_STVEC, t1
	csrrw t2, CSR_SSCRATCH, t2
	csrrw t4, CSR_HSTATUS, t4
	csrrw t5, CSR_SSTATUS, t5

	sd t0, (VIRT_CPU_GUEST_SEPC)(a0)
	sd t2, (VIRT_CPU_GUEST_A0)(a0)
	sd t4, (VIRT_CPU_GUEST_HSTATUS)(a0)
	sd t5, (VIRT_CPU_GUEST_SSTATUS)(a0)

	ld ra, (VIRT_CPU_HOST_RA)(a0)
	ld sp, (VIRT_CPU_HOST_SP)(a0)
	ld gp, (VIRT_CPU_HOST_GP)(a0)
	ld tp, (VIRT_CPU_HOST_TP)(a0)
	ld a1, (VIRT_CPU_HOST_A1)(a0)
	ld a2, (VIRT_CPU_HOST_A2)(a0)
	ld a3, (VIRT_CPU_HOST_A3)(a0)
	ld a4, (VIRT_CPU_HOST_A4)(a0)
	ld a5, (VIRT_CPU_HOST_A5)(a0)
	ld a6, (VIRT_CPU_HOST_A6)(a0)
	ld a7, (VIRT_CPU_HOST_A7)(a0)
	ld s0, (VIRT_CPU_HOST_S0)(a0)
	ld s1, (VIRT_CPU_HOST_S1)(a0)
	ld s2, (VIRT_CPU_HOST_S2)(a0)
	ld s3, (VIRT_CPU_HOST_S3)(a0)
	ld s4, (VIRT_CPU_HOST_S4)(a0)
	ld s5, (VIRT_CPU_HOST_S5)(a0)
	ld s6, (VIRT_CPU_HOST_S6)(a0)
	ld s7, (VIRT_CPU_HOST_S7)(a0)
	ld s8, (VIRT_CPU_HOST_S8)(a0)
	ld s9, (VIRT_CPU_HOST_S9)(a0)
	ld s10, (VIRT_CPU_HOST_S10)(a0)
	ld s11, (VIRT_CPU_HOST_S11)(a0)
	
	fld f0,  (VIRT_H_F0)(a0)
	fld f1,  (VIRT_H_F1)(a0)
	fld f2,  (VIRT_H_F2)(a0)
	fld f3,  (VIRT_H_F3)(a0)
	fld f4,  (VIRT_H_F4)(a0)
	fld f5,  (VIRT_H_F5)(a0)
	fld f6,  (VIRT_H_F6)(a0)
	fld f7,  (VIRT_H_F7)(a0)
	fld f8,  (VIRT_H_F8)(a0)
	fld f9,  (VIRT_H_F9)(a0)
	fld f10, (VIRT_H_F10)(a0)
	fld f11, (VIRT_H_F11)(a0)
	fld f12, (VIRT_H_F12)(a0)
	fld f13, (VIRT_H_F13)(a0)
	fld f14, (VIRT_H_F14)(a0)
	fld f15, (VIRT_H_F15)(a0)
	fld f16, (VIRT_H_F16)(a0)
	fld f17, (VIRT_H_F17)(a0)
	fld f18, (VIRT_H_F18)(a0)
	fld f19, (VIRT_H_F19)(a0)
	fld f20, (VIRT_H_F20)(a0)
	fld f21, (VIRT_H_F21)(a0)
	fld f22, (VIRT_H_F22)(a0)
	fld f23, (VIRT_H_F23)(a0)
	fld f24, (VIRT_H_F24)(a0)
	fld f25, (VIRT_H_F25)(a0)
	fld f26, (VIRT_H_F26)(a0)
	fld f27, (VIRT_H_F27)(a0)
	fld f28, (VIRT_H_F28)(a0)
	fld f29, (VIRT_H_F29)(a0)
	fld f30, (VIRT_H_F30)(a0)
	fld f31, (VIRT_H_F31)(a0)

	ret

.section .text
.global vcpu_switch_to
vcpu_switch_to:
	// a0 -- vcpu->cpu_context
	
	/* Save Host GPRs */ 
	sd ra, (VIRT_CPU_HOST_RA)(a0)
	sd sp, (VIRT_CPU_HOST_SP)(a0)
	sd gp, (VIRT_CPU_HOST_GP)(a0)
	sd tp, (VIRT_CPU_HOST_TP)(a0)
	sd s0, (VIRT_CPU_HOST_S0)(a0)
	sd s1, (VIRT_CPU_HOST_S1)(a0)
	sd a2, (VIRT_CPU_HOST_A2)(a0)
	sd a3, (VIRT_CPU_HOST_A3)(a0)
	sd a4, (VIRT_CPU_HOST_A4)(a0)
	sd a5, (VIRT_CPU_HOST_A5)(a0)
	sd a6, (VIRT_CPU_HOST_A6)(a0)
	sd a7, (VIRT_CPU_HOST_A7)(a0)
	sd s2, (VIRT_CPU_HOST_S2)(a0)
	sd s3, (VIRT_CPU_HOST_S3)(a0)
	sd s4, (VIRT_CPU_HOST_S4)(a0)
	sd s5, (VIRT_CPU_HOST_S5)(a0)
	sd s6, (VIRT_CPU_HOST_S6)(a0)
	sd s7, (VIRT_CPU_HOST_S7)(a0)
	sd s8, (VIRT_CPU_HOST_S8)(a0)
	sd s9, (VIRT_CPU_HOST_S9)(a0)
	sd s10, (VIRT_CPU_HOST_S10)(a0)
	sd s11, (VIRT_CPU_HOST_S11)(a0)

	fsd f0,  (VIRT_H_F0)(a0)
	fsd f1,  (VIRT_H_F1)(a0)
	fsd f2,  (VIRT_H_F2)(a0)
	fsd f3,  (VIRT_H_F3)(a0)
	fsd f4,  (VIRT_H_F4)(a0)
	fsd f5,  (VIRT_H_F5)(a0)
	fsd f6,  (VIRT_H_F6)(a0)
	fsd f7,  (VIRT_H_F7)(a0)
	fsd f8,  (VIRT_H_F8)(a0)
	fsd f9,  (VIRT_H_F9)(a0)
	fsd f10, (VIRT_H_F10)(a0)
	fsd f11, (VIRT_H_F11)(a0)
	fsd f12, (VIRT_H_F12)(a0)
	fsd f13, (VIRT_H_F13)(a0)
	fsd f14, (VIRT_H_F14)(a0)
	fsd f15, (VIRT_H_F15)(a0)
	fsd f16, (VIRT_H_F16)(a0)
	fsd f17, (VIRT_H_F17)(a0)
	fsd f18, (VIRT_H_F18)(a0)
	fsd f19, (VIRT_H_F19)(a0)
	fsd f20, (VIRT_H_F20)(a0)
	fsd f21, (VIRT_H_F21)(a0)
	fsd f22, (VIRT_H_F22)(a0)
	fsd f23, (VIRT_H_F23)(a0)
	fsd f24, (VIRT_H_F24)(a0)
	fsd f25, (VIRT_H_F25)(a0)
	fsd f26, (VIRT_H_F26)(a0)
	fsd f27, (VIRT_H_F27)(a0)
	fsd f28, (VIRT_H_F28)(a0)
	fsd f29, (VIRT_H_F29)(a0)
	fsd f30, (VIRT_H_F30)(a0)
	fsd f31, (VIRT_H_F31)(a0)

	/* Load Guest CSR values */	
	ld t0, (VIRT_CPU_GUEST_SSTATUS)(a0)
	ld t1, (VIRT_CPU_GUEST_HSTATUS)(a0)
	la t4, __vcpu_switch_return
	ld t5, (VIRT_CPU_GUEST_SEPC)(a0)

	/* Save Host and Restore Guest CSRs */
	csrrw t0, CSR_SSTATUS, t0
	csrrw t1, CSR_HSTATUS, t1
	csrrw t4, CSR_STVEC, t4
	csrw CSR_SEPC, t5

	/* Save vcpu_ptr to sscratch */
	csrrw t3, CSR_SSCRATCH, a0

	/* Save Host Guest CSRs */
	sd t0, (VIRT_CPU_HOST_SSTATUS)(a0)
	sd t1, (VIRT_CPU_HOST_HSTATUS)(a0)
	sd t3, (VIRT_CPU_HOST_SSCRATCH)(a0)
	sd t4, (VIRT_CPU_HOST_STVEC)(a0)
	
	/* Restore Guest GPRs */
	ld ra, (VIRT_CPU_GUEST_RA)(a0)
	ld sp, (VIRT_CPU_GUEST_SP)(a0)
	ld gp, (VIRT_CPU_GUEST_GP)(a0)
	ld tp, (VIRT_CPU_GUEST_TP)(a0)
	ld s0, (VIRT_CPU_GUEST_S3)(a0)
	ld s1, (VIRT_CPU_GUEST_S1)(a0)
	ld a1, (VIRT_CPU_GUEST_A1)(a0)
	ld a2, (VIRT_CPU_GUEST_A2)(a0)
	ld a3, (VIRT_CPU_GUEST_A3)(a0)
	ld a4, (VIRT_CPU_GUEST_A4)(a0)
	ld a5, (VIRT_CPU_GUEST_A5)(a0)
	ld a6, (VIRT_CPU_GUEST_A6)(a0)
	ld a7, (VIRT_CPU_GUEST_A7)(a0)
	ld s2, (VIRT_CPU_GUEST_S2)(a0)
	ld s3, (VIRT_CPU_GUEST_S3)(a0)
	ld s4, (VIRT_CPU_GUEST_S4)(a0)
	ld s5, (VIRT_CPU_GUEST_S5)(a0)
	ld s6, (VIRT_CPU_GUEST_S6)(a0)
	ld s7, (VIRT_CPU_GUEST_S7)(a0)
	ld s8, (VIRT_CPU_GUEST_S8)(a0)
	ld s9, (VIRT_CPU_GUEST_S9)(a0)
	ld s10, (VIRT_CPU_GUEST_S10)(a0)
	ld s11, (VIRT_CPU_GUEST_S11)(a0)
	ld t0, (VIRT_CPU_GUEST_T0)(a0)
	ld t1, (VIRT_CPU_GUEST_T1)(a0)
	ld t2, (VIRT_CPU_GUEST_T2)(a0)
	ld t3, (VIRT_CPU_GUEST_T3)(a0)
	ld t4, (VIRT_CPU_GUEST_T4)(a0)
	ld t5, (VIRT_CPU_GUEST_T5)(a0)
	ld t6, (VIRT_CPU_GUEST_T6)(a0)

	fld f0,  (VIRT_G_F0)(a0)
	fld f1,  (VIRT_G_F1)(a0)
	fld f2,  (VIRT_G_F2)(a0)
	fld f3,  (VIRT_G_F3)(a0)
	fld f4,  (VIRT_G_F4)(a0)
	fld f5,  (VIRT_G_F5)(a0)
	fld f6,  (VIRT_G_F6)(a0)
	fld f7,  (VIRT_G_F7)(a0)
	fld f8,  (VIRT_G_F8)(a0)
	fld f9,  (VIRT_G_F9)(a0)
	fld f10, (VIRT_G_F10)(a0)
	fld f11, (VIRT_G_F11)(a0)
	fld f12, (VIRT_G_F12)(a0)
	fld f13, (VIRT_G_F13)(a0)
	fld f14, (VIRT_G_F14)(a0)
	fld f15, (VIRT_G_F15)(a0)
	fld f16, (VIRT_G_F16)(a0)
	fld f17, (VIRT_G_F17)(a0)
	fld f18, (VIRT_G_F18)(a0)
	fld f19, (VIRT_G_F19)(a0)
	fld f20, (VIRT_G_F20)(a0)
	fld f21, (VIRT_G_F21)(a0)
	fld f22, (VIRT_G_F22)(a0)
	fld f23, (VIRT_G_F23)(a0)
	fld f24, (VIRT_G_F24)(a0)
	fld f25, (VIRT_G_F25)(a0)
	fld f26, (VIRT_G_F26)(a0)
	fld f27, (VIRT_G_F27)(a0)
	fld f28, (VIRT_G_F28)(a0)
	fld f29, (VIRT_G_F29)(a0)
	fld f30, (VIRT_G_F30)(a0)
	fld f31, (VIRT_G_F31)(a0)

	ld a0, (VIRT_CPU_GUEST_A0)(a0)
	
	sret
