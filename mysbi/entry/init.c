#include <print.h>
#include <asm/csr.h>
#include <sbi.h>
#include "sbi_trap.h"

void gos_init(unsigned int hart, struct sbi_trap_hw_context *ctx)
{
	sbi_init(hart, ctx);
}

void fpu_init()
{
	unsigned long val;

	val = read_csr(misa);
	if (val & (1 << ('D' - 'A')) || val & (1 << ('F' - 'A'))) {
		val = read_csr(mstatus);
		val = INSERT_FIELD(val, MSTATUS_FS, 0x3);
		write_csr(mstatus, val);
		sbi_print("FPU enabled.\n");
	}
}

void boot_hart_start(unsigned int hart, struct sbi_trap_hw_context *ctx)
{
	sbi_print("%s hartid: %d, ctx:%x\n", __FUNCTION__, hart, ctx);

	fpu_init();

	sbi_jump_to_next(ctx);
}

void other_hart_start(unsigned int hart, struct sbi_trap_hw_context *ctx)
{
	int delay = 100;

	sbi_print("%s hartid: %d, ctx:%x\n", __FUNCTION__, hart, ctx);

	fpu_init();

	sbi_secondary_init(hart, ctx);

	while (ctx->wait_var == 0) {
		while (delay--)
			__asm__ volatile ("nop");
		delay = 100;
	}

	sbi_jump_to_next(ctx);
}

void other_hart_init(unsigned int hart, struct sbi_trap_hw_context *ctx)
{
	ctx->wait_var = 0;
}
