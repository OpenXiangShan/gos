.section .rodata
.global logo

logo:
	.incbin "entry/logo.bin"
	.byte 0
