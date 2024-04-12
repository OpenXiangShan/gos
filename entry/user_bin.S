.section .rodata
.global user_bin
.global __user_payload_start
.global __user_payload_end

.align 2
__user_payload_start:
user_bin:
	.incbin "build/myUser.bin"
__user_payload_end:
