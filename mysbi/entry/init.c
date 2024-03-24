#include <print.h>
#include <sbi.h>
#include "sbi_trap.h"

void gos_init(unsigned int hart, struct sbi_trap_hw_context *ctx)
{
	sbi_init(hart, ctx);
}

void boot_hart_start(unsigned int hart, struct sbi_trap_hw_context *ctx)
{
	sbi_print("%s hartid: %d, ctx:%x\n", __FUNCTION__, hart, ctx);
	sbi_jump_to_next(ctx);
}

void other_hart_start(unsigned int hart, struct sbi_trap_hw_context *ctx)
{
	int delay = 100;

	sbi_print("%s hartid: %d, ctx:%x\n", __FUNCTION__, hart, ctx);

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
