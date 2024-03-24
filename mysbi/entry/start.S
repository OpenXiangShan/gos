#include <asm/trap.h>

.section ".text.boot"
.global _start
_start:	
	li a3, 0
	la a4, wake
	sd a3, 0(a4)

	csrr a4, mhartid

	/*
	 stack_bottom
	 +--------+
	 +        +
	 +  ...   +
	 +        +
	 +--------+
	 +        + 1024 cpu n
	 +--------+
         +        + 1024 cpu n-1
         +--------+ 
            ...
         +--------+
	 +        + 1024 cpu 0
	 +--------+ stack top = stack_bottom - 8192
	*/
	
	li t0, 8192
	la sp, stack_bottom
	add sp, sp, t0
	li t0, 0x400
	mul a5, t0, a4
	sub sp, sp, a5 

	li t0, 0x400
	sub a5, sp, t0 //ctx point to stack bootom

	csrw mscratch, sp

	beqz a4, _run 

	csrr a0, mhartid
	mv a1, a5
	jal other_hart_init
wait_for_boot_hart:
	la a1, wake
	ld a4, 0(a1)
	beqz a4, wait_for_boot_hart
	j other_hart_run

_run:
	csrr a0, mhartid
	mv a1, a5
	jal gos_init

	la a4, wake
	li a3, 1
	sd a3, 0(a4)

	li t0, 0x400
	csrr a5, mscratch
	sub a5, a5, t0
	csrr a0, mhartid
	mv a1, a5
	jal boot_hart_start
	
	csrr a0, mhartid
	j sbi_exit	
other_hart_run:
	csrr a0, mhartid
	mv a1, a5
	jal other_hart_start

	csrr a0, mhartid
	j sbi_exit	

stack_bottom:
	.skip 8192
wake:
	.skip 8

