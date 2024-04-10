.section .rodata
.global guest_bin
.global __guest_payload_start
.global __guest_payload_end

.align 2
__guest_payload_start:
guest_bin:
	.incbin "build/myGuest.bin"
__guest_payload_end:
