#include "device.h"
#include "asm/mmio.h"
#include "asm/type.h"
#include "print.h"

#define REG_DO_WHAT 0x0
#define REG_TIMEOUT 0x4

#define SLEEP 1
#define SCHEDULE 2

static unsigned long base_address;
extern int mmu_is_on;

void scheduler_sleep_to_timeout(int ms)
{
	writel(base_address + REG_TIMEOUT, ms);
	writel(base_address + REG_DO_WHAT, SLEEP);
}

void scheduler_schedule(void)
{
	writel(base_address + REG_DO_WHAT, SCHEDULE);
}

int scheduler_init(unsigned long base, unsigned int len, void *data)
{
	if (mmu_is_on)
		base_address = (unsigned long)ioremap((void *)base, len, NULL);
	else
		base_address = base;

	return 0;
}

DRIVER_REGISTER(scheduler, scheduler_init, "scheduler");
