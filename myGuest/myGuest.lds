OUTPUT_ARCH("riscv")
ENTRY(__start_guest)
SECTIONS
{
	. = 0x90000000,	
	.text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.data : { 
		*(.data)
		. = ALIGN(4096);
		*(.data.stack)
		.stack_top = .;
	}
	
	.rodata : { *(.rodata) }
	
	bss_begin = .;
        .bss : { *(.bss*) }
        bss_end = .;	
}
