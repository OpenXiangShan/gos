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

#include "asm/type.h"
#include "print.h"
#include "device.h"
#include "string.h"
#include "../command.h"
#include "irq.h"

static void intr_test_irq_handler(void *data)
{
	static int num = 0;

	print("%s count: %d\n", __FUNCTION__, num);
	if (++num >= 10)
		__asm__ __volatile__("li a0, 1;" ".word 0x5006b;");
}

static int cmd_intr_test_handler(int argc, char *argv[], void *priv)
{
	int irqs[16], nr_irqs, i;
	struct device *dev = get_device("intr,test");

	if (!dev) {
		print("No intr,test device\n");
		return -1;
	}

	disable_local_irq();
	nr_irqs = get_hwirq(dev, irqs);
	print("nr_irqs:%d irq:%d\n", nr_irqs, irqs[0]);
	for (i = 0; i < nr_irqs; i++)
		register_device_irq(dev, dev->irq_domain, irqs[i],
				    intr_test_irq_handler, NULL);
	enable_local_irq();

	return 0;
}

static const struct command cmd_intr_test = {
	.cmd = "intr_test",
	.handler = cmd_intr_test_handler,
	.priv = NULL,
};

int cmd_intr_test_init()
{
	register_command(&cmd_intr_test);

	return 0;
}

APP_COMMAND_REGISTER(intr_test, cmd_intr_test_init);
