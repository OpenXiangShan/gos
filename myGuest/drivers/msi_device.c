#include "device.h"
#include "asm/mmio.h"
#include "asm/type.h"
#include "print.h"
#include "vmap.h"
#include "irq.h"
#include "asm/pgtable.h"

#define REG_MSI_ADDR 0x0
#define REG_MSI_DATA 0x4
#define REG_TRIGGER  0x8

static unsigned long base_address;
extern int mmu_is_on;

int msi_device_init(unsigned long base, unsigned int len, void *data)
{
	int hwirq = 0;
	unsigned long msi_addr, msi_data;

	print("%s base:0x%lx len:0x%lx\n", __FUNCTION__, base, len);
	hwirq = alloc_msi_irqs(1);
	print("%s -- hwirq:%d\n", __FUNCTION__, hwirq);
	if (hwirq == -1)
		return -1;

	if (compose_msi_msg(hwirq, &msi_addr, &msi_data))
		return -1;

	print("%s -- msi_addr:0x%lx, msi_data:%d\n", __FUNCTION__, msi_addr,
	      msi_data);

	if (mmu_is_on)
		base_address = (unsigned long)ioremap((void *)base, len, NULL);
	else
		base_address = base;

	writel(base_address + REG_MSI_ADDR, msi_addr);
	writel(base_address + REG_MSI_DATA, msi_data);

	return 0;
}

DRIVER_REGISTER(msi_device, msi_device_init, "msi_device");
