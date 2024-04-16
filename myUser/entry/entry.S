.section ".text.boot"
.global __start_user
__start_user:
	la sp, .stack_top
	jal start_user
1:
	j 1b

.section ".data.stack"
stack:
	.skip 4096
