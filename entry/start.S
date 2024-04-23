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
	li t0, 8192
	la sp, boot_stack
	add sp, sp, t0
	li t0, 0x400
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

boot_stack:
	.skip 8192
