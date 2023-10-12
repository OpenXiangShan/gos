OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS
{	
	. = 0x80000000,
	.text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.data : { *(.data) }
	.rodata : { *(.rodata) }
	bss_begin = .;
        .bss : { *(.bss*) }
        bss_end = .;

	. = ALIGN(8);
	__device_init_table = .;
	.device_init_table : { *(.device_init_table) }
	__device_init_table_end = .;

	. = 0x80010000,
	__payload_start = .;

}
