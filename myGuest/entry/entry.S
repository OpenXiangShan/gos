.section ".text.boot"
.global __start_guest
__start_guest:
	la sp, .stack_top
	jal start_guest
1:
	j 1b
