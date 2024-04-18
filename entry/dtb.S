.section .rodata
.global dtb_bin
.global __dtb_payload_start
.global __dtb_payload_end

.align 12
__dtb_payload_start:
dtb_bin:
	.incbin "build/input.dtb"
__dtb_payload_end:
