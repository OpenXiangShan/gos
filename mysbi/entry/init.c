#include <print.h>
#include <sbi.h>

void gos_init(unsigned int hart, struct sbi_trap_hw_context *ctx)
{
	sbi_init(hart, ctx);
}

void boot_hart_start(unsigned int hart, struct sbi_trap_hw_context *ctx)
{
	sbi_print("%s hartid: %d, ctx:%x\n", __FUNCTION__, hart, ctx);
}

void other_hart_start(unsigned int hart, struct sbi_trap_hw_context *ctx)
{
	sbi_print("%s hartid: %d, ctx:%x\n", __FUNCTION__, hart, ctx);
}
