#include <asm/trap.h>

.section ".text.boot"
.global _start
_start:
	li t0, 4096
	la sp, stack_bottom
	add sp, sp, t0 #sp point to stack top
	
	li a3, 0
	la a4, wake
	sd a3, 0(a4)

	csrr a4, mhartid
	
	li t0, SBI_HW_CTX_SIZE
	mul a5, t0, a4
	add a5, sp, a5

	csrw mscratch, sp

	beqz a4, _run 

wait_for_boot_hart:
	la a1, wake
	ld a4, 0(a1)
	beqz a4, wait_for_boot_hart
	j other_hart_run

_run:
	csrr a0, mhartid
	mv a1, a5
	jal gos_init
	
	li t0, SBI_HW_CTX_SIZE
	mul a5, t0, a4
	add a5, sp, a5
	csrr a0, mhartid
	mv a1, a5
	jal boot_hart_start
	
	la a4, wake
	li a3, 1
	sd a3, 0(a4)
Loop:
	j Loop

other_hart_run:
	csrr a0, mhartid
	mv a1, a5
	jal other_hart_start

stack_bottom:
	.skip 8192
wake:
	.skip 8

