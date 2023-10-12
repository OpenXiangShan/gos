.section .rodata
.global test_cmd

test_cmd:
	.incbin "auto_run.bin"
	.byte 0
