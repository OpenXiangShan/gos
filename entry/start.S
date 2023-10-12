.section ".text.boot"
.global __start_gos
__start_gos:
	la sp, .stack_top
	jal start_gos
Loop:
	j Loop

.section ".data.stack"
stack:
	.skip 4096
