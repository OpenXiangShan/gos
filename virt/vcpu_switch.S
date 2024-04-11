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

	ld a0, (VIRT_CPU_GUEST_A0)(a0)
	
	sret
