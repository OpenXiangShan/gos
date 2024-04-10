#include "asm/asm-offsets.h"

.section .text
.global __vcpu_switch_return
__vcpu_switch_return:
	nop
	nop
	nop
	nop
	nop
	ret

.section .text
.global vcpu_switch_to
vcpu_switch_to:
	// a0 -- host_context a1 -- guest_context 
	
	/* Save host general register */
	sd x1,  PT_RA(a0)
	sd x3,  PT_GP(a0)
	sd x5,  PT_T0(a0)
	sd x6,  PT_T1(a0)
	sd x7,  PT_T2(a0)
	sd x8,  PT_S0(a0)
	sd x9,  PT_S1(a0)
	//sd x10, PT_A0(a0)
	//sd x11, PT_A1(a0)
	sd x12, PT_A2(a0)
	sd x13, PT_A3(a0)
	sd x14, PT_A4(a0)
	sd x15, PT_A5(a0)
	sd x16, PT_A6(a0)
	sd x17, PT_A7(a0)
	sd x18, PT_S2(a0)
	sd x19, PT_S3(a0)
	sd x20, PT_S4(a0)
	sd x21, PT_S5(a0)
	sd x22, PT_S6(a0)
	sd x23, PT_S7(a0)
	sd x24, PT_S8(a0)
	sd x25, PT_S9(a0)
	sd x26, PT_S10(a0)
	sd x27, PT_S11(a0)
	sd x28, PT_T3(a0)
	sd x29, PT_T4(a0)
	sd x30, PT_T5(a0)
	sd x31, PT_T6(a0)

	csrr t1, sstatus
	sd t1, PT_SSTATUS(a0)

	csrr t1, sepc
	sd t1, PT_SEPC(a0)

	csrr t1, scause
	sd t1, PT_SCAUSE(a0)

	csrr t1, sscratch
	sd t1, PT_TP(a0)

	csrr t1, hstatus
	sd t1, PT_HSTATUS(a0)

	ld t1, PT_SSTATUS(a1)
	csrw sstatus, t1

	ld t1, PT_SEPC(a1)
	csrw sepc, t1

	ld t1, PT_HSTATUS(a1) 
	csrw 0x600, t1

	ld t1, PT_TP(a1)
	csrw sscratch, t1	

	la t1, __vcpu_switch_return	
	csrw stvec, t1
	
	ld x10, PT_A0(a1)
/*
	ld x1,  PT_RA(a1)
	ld x3,  PT_GP(a1)
	ld x4,  PT_TP(a1)
	ld x5,  PT_T0(a1)
	ld x6,  PT_T1(a1)
	ld x7,  PT_T2(a1)
	ld x8,  PT_S0(a1)
	ld x9,  PT_S1(a1)
	ld x10, PT_A0(a1)
	ld x11, PT_A1(a1)
	ld x12, PT_A2(a1)
	ld x13, PT_A3(a1)
	ld x14, PT_A4(a1)
	ld x15, PT_A5(a1)
	ld x16, PT_A6(a1)
	ld x17, PT_A7(a1)
	ld x18, PT_S2(a1)
	ld x19, PT_S3(a1)
	ld x20, PT_S4(a1)
	ld x21, PT_S5(a1)
	ld x22, PT_S6(a1)
	ld x23, PT_S7(a1)
	ld x24, PT_S8(a1)
	ld x25, PT_S9(a1)
	ld x26, PT_S10(a1)
	ld x27, PT_S11(a1)
	ld x28, PT_T3(a1)
	ld x29, PT_T4(a1)
	ld x30, PT_T5(a1)
	ld x31, PT_T6(a1)
*/	
	sret
