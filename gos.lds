OUTPUT_ARCH("riscv")
ENTRY(__start_gos)
SECTIONS
{	
	. = 0x80010000,
	.text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.data : { 
		*(.data)
		. = ALIGN(4096);
		*(.data.stack)
		.stack_top = .;
	}
		
	.rodata : { *(.rodata) }
	
	. = ALIGN(8);
	__driver_init_table = .;
	.driver_init_table : { *(.driver_init_table) }
	__driver_init_table_end = .;
	
	. = ALIGN(8);
	__earlycon_init_table = .;
	.earlycon_init_table : { *(.earlycon_init_table) }
	__earlycon_init_table_end = .;

	. = ALIGN(8);
	__irqchip_init_table = .;
	.irqchip_init_table : { *(.irqchip_init_table) }
	__irqchip_init_table_end = .;

	. = ALIGN(8);
	__timer_init_table = .;
	.timer_init_table : { *(.timer_init_table) }
	__timer_init_table_end = .;

	. = ALIGN(8);
	__command_init_table = .;
	.command_init_table : { *(.command_init_table) }
	__command_init_table_end = .;

	bss_begin = .;
        .bss : { *(.bss*) }
        bss_end = .;
	
}
