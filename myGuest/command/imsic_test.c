/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "command.h"
#include "print.h"
#include "asm/type.h"
#include "irq.h"
#include "asm/mmio.h"
#include "vmap.h"
#include "asm/pgtable.h"

static int imsic_test_irq_handler(void *data)
{
	print("%s -- test pass!!\n", __FUNCTION__);

	return 0;
}

static int cmd_imsic_test_handler(int argc, char *argv[], void *priv)
{
	int hwirq = 0;
	unsigned long msi_addr, msi_data;
	unsigned long addr;

	myGuest_print("%s %d\n", __FUNCTION__, __LINE__);
	hwirq = alloc_msi_irqs(1);
	if (hwirq == -1)
		return -1;

	if (compose_msi_msg(hwirq, &msi_addr, &msi_data))
		return -1;

	register_irq_handler(hwirq, imsic_test_irq_handler, NULL);

	myGuest_print("msi_addr:0x%lx msi_data:0x%lx\n", msi_addr, msi_data);

	addr = (unsigned long)ioremap((void *)msi_addr, PAGE_SIZE, NULL);

	writel(addr, msi_data);

	iounmap((void *)addr, PAGE_SIZE);

	return 0;
}

static const struct command cmd_imsic_test = {
	.cmd = "imsic_test",
	.handler = cmd_imsic_test_handler,
	.priv = NULL,
};

int cmd_imsic_test_init()
{
	register_command(&cmd_imsic_test);

	return 0;
}

APP_COMMAND_REGISTER(imsic_test, cmd_imsic_test_init);
