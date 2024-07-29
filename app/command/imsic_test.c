#include <print.h>
#include <device.h>
#include <asm/mmio.h>
#include <asm/type.h>
#include <event.h>
#include "../command.h"
#include "asm/sbi.h"
#include "asm/pgtable.h"
#include "vmap.h"

#define TEST_COUNT 3

static int cmd_imsic_m_test_handler(int argc, char *argv[], void *priv)
{
	int i, nr_irqs = 1;;
	int msi_data;
	unsigned long msi_addr, msi_addr_va;

	msi_addr = sbi_get_m_msi_addr();
	msi_data = sbi_get_m_msi_data(nr_irqs);
	if (msi_data == -1)
		return -1;

	msi_addr_va = (unsigned long)ioremap((void *)msi_addr, PAGE_SIZE, NULL);
	if (!msi_addr_va)
		return -1;

	for (i = 0; i < nr_irqs; i++) {
		print("write %d to 0x%lx\n", msi_data, msi_addr);
		writel(msi_addr_va, msi_data);
	}

	return 0;
}

static int cmd_imsic_test_handler(int argc, char *argv[], void *priv)
{
	int i, fd, n;

	fd = open("IMSIC_TEST");
	if (fd == -1) {
		print("open %s fail.\n", "IMSIC_TEST");
		return -1;
	}

	for (i = 0; i < TEST_COUNT; i++) {
		n = 0;
		print("imsic_test %d times: trigger irq%d\n", i, n);
		ioctl(fd, n++, NULL);
		wait_for_ms(1000);

		print("imsic_test %d times: trigger irq%d\n", i, n);
		ioctl(fd, n++, NULL);
		wait_for_ms(1000);

		print("imsic_test %d times: trigger irq%d\n", i, n);
		ioctl(fd, n++, NULL);
		wait_for_ms(1000);
	}

	return 0;
}

static const struct command cmd_imsic_m_test = {
	.cmd = "imsic_test_m",
	.handler = cmd_imsic_m_test_handler,
	.priv = NULL,
};
static const struct command cmd_imsic_test = {
	.cmd = "imsic_test",
	.handler = cmd_imsic_test_handler,
	.priv = NULL,
};

int command_imsic_test_init()
{
	register_command(&cmd_imsic_test);
	register_command(&cmd_imsic_m_test);

	return 0;
}

APP_COMMAND_REGISTER(imsic_test_command, command_imsic_test_init);
