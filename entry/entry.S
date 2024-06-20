#include "asm/asm-offsets.h"

.align 2
.global do_exception_vector
do_exception_vector:
	addi sp, sp, -(PT_SIZE)

	/* save general Register */
	sd x1,  PT_RA(sp)
	sd x3,  PT_GP(sp)
	sd x5,  PT_T0(sp)
	sd x6,  PT_T1(sp)
	sd x7,  PT_T2(sp)
	sd x8,  PT_S0(sp)
	sd x9,  PT_S1(sp)
	sd x10, PT_A0(sp)
	sd x11, PT_A1(sp)
	sd x12, PT_A2(sp)
	sd x13, PT_A3(sp)
	sd x14, PT_A4(sp)
	sd x15, PT_A5(sp)
	sd x16, PT_A6(sp)
	sd x17, PT_A7(sp)
	sd x18, PT_S2(sp)
	sd x19, PT_S3(sp)
	sd x20, PT_S4(sp)
	sd x21, PT_S5(sp)
	sd x22, PT_S6(sp)
	sd x23, PT_S7(sp)
	sd x24, PT_S8(sp)
	sd x25, PT_S9(sp)
	sd x26, PT_S10(sp)
	sd x27, PT_S11(sp)
	sd x28, PT_T3(sp)
	sd x29, PT_T4(sp)
	sd x30, PT_T5(sp)
	sd x31, PT_T6(sp)

	fsd f0,  PT_F0(sp)
	fsd f1,  PT_F1(sp)
	fsd f2,  PT_F2(sp)
	fsd f3,  PT_F3(sp)
	fsd f4,  PT_F4(sp)
	fsd f5,  PT_F5(sp)
	fsd f6,  PT_F6(sp)
	fsd f7,  PT_F7(sp)
	fsd f8,  PT_F8(sp)
	fsd f9,  PT_F9(sp)
	fsd f10, PT_F10(sp)
	fsd f11, PT_F11(sp)
	fsd f12, PT_F12(sp)
	fsd f13, PT_F13(sp)
	fsd f14, PT_F14(sp)
	fsd f15, PT_F15(sp)
	fsd f16, PT_F16(sp)
	fsd f17, PT_F17(sp)
	fsd f18, PT_F18(sp)
	fsd f19, PT_F19(sp)
	fsd f20, PT_F20(sp)
	fsd f21, PT_F21(sp)
	fsd f22, PT_F22(sp)
	fsd f23, PT_F23(sp)
	fsd f24, PT_F24(sp)
	fsd f25, PT_F25(sp)
	fsd f26, PT_F26(sp)
	fsd f27, PT_F27(sp)
	fsd f28, PT_F28(sp)
	fsd f29, PT_F29(sp)
	fsd f30, PT_F30(sp)
	fsd f31, PT_F31(sp)

	/* save sstatus */
	csrr s1, sstatus
	sd s1, PT_SSTATUS(sp)
	
	/* save sepc */
	csrr s2, sepc
	sd s2, PT_SEPC(sp)

	/* save sbadaddr */
	csrr s3, sbadaddr
	sd s3, PT_SBADADDR(sp)

	/* save scause */
	csrr s4, scause
	sd s4, PT_SCAUSE(sp)

	/* save sscratch */
	csrr s5, sscratch
	sd s5, PT_TP(sp)

	/* save satp */
	csrr s5, satp
	sd s5, PT_SATP(sp)

	addi t0, sp, PT_SIZE
	sd t0, PT_SP(sp)
	/* 
	 * sp -> pt_regs 
	 * s4 -> scause
	 */
	mv a0, sp
	mv a1, s4
	call do_exception

	/* restore */
	ld a0, PT_SSTATUS(sp)
	csrw sstatus, a0

	ld a2, PT_SEPC(sp)
	csrw sepc, a2

	ld s3, PT_SBADADDR(sp)
	csrw sbadaddr, s3

	ld x1,  PT_RA(sp)
	ld x3,  PT_GP(sp)
	ld x4,  PT_TP(sp)
	ld x5,  PT_T0(sp)
	ld x6,  PT_T1(sp)
	ld x7,  PT_T2(sp)
	ld x8,  PT_S0(sp)
	ld x9,  PT_S1(sp)
	ld x10, PT_A0(sp)
	ld x11, PT_A1(sp)
	ld x12, PT_A2(sp)
	ld x13, PT_A3(sp)
	ld x14, PT_A4(sp)
	ld x15, PT_A5(sp)
	ld x16, PT_A6(sp)
	ld x17, PT_A7(sp)
	ld x18, PT_S2(sp)
	ld x19, PT_S3(sp)
	ld x20, PT_S4(sp)
	ld x21, PT_S5(sp)
	ld x22, PT_S6(sp)
	ld x23, PT_S7(sp)
	ld x24, PT_S8(sp)
	ld x25, PT_S9(sp)
	ld x26, PT_S10(sp)
	ld x27, PT_S11(sp)
	ld x28, PT_T3(sp)
	ld x29, PT_T4(sp)
	ld x30, PT_T5(sp)
	ld x31, PT_T6(sp)

	fld f0,  PT_F0(sp)
	fld f1,  PT_F1(sp)
	fld f2,  PT_F2(sp)
	fld f3,  PT_F3(sp)
	fld f4,  PT_F4(sp)
	fld f5,  PT_F5(sp)
	fld f6,  PT_F6(sp)
	fld f7,  PT_F7(sp)
	fld f8,  PT_F8(sp)
	fld f9,  PT_F9(sp)
	fld f10, PT_F10(sp)
	fld f11, PT_F11(sp)
	fld f12, PT_F12(sp)
	fld f13, PT_F13(sp)
	fld f14, PT_F14(sp)
	fld f15, PT_F15(sp)
	fld f16, PT_F16(sp)
	fld f17, PT_F17(sp)
	fld f18, PT_F18(sp)
	fld f19, PT_F19(sp)
	fld f20, PT_F20(sp)
	fld f21, PT_F21(sp)
	fld f22, PT_F22(sp)
	fld f23, PT_F23(sp)
	fld f24, PT_F24(sp)
	fld f25, PT_F25(sp)
	fld f26, PT_F26(sp)
	fld f27, PT_F27(sp)
	fld f28, PT_F28(sp)
	fld f29, PT_F29(sp)
	fld f30, PT_F30(sp)
	fld f31, PT_F31(sp)

	ld s5, PT_SATP(sp)
	ld sp,  PT_SP(sp)
	csrw satp, s5

	sret	

.global task_do_exception_vector
	
