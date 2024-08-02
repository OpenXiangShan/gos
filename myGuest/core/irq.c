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

#include "irq.h"
#include "asm/type.h"
#include "print.h"

static do_irq_handler_t do_irq_handler = NULL;
static struct msi_irq_ops *msi_ops = NULL;

int alloc_msi_irqs(int nr)
{
	int hwirq, i;

	if (!msi_ops)
		return -1;

	if (!msi_ops->alloc_irqs)
		return -1;

	hwirq = msi_ops->alloc_irqs(nr);
	if (hwirq == -1)
		return -1;

	for (i = 0; i < nr; i++) {
		if (msi_ops->unmask_irq)
			msi_ops->unmask_irq(hwirq + i);
	}

	return hwirq;
}

int irq_handler(void)
{
	if (do_irq_handler)
		do_irq_handler();

	return 0;
}

int compose_msi_msg(int hwirq, unsigned long *msi_addr, unsigned long *msi_data)
{
	if (!msi_ops)
		return -1;

	if (!msi_ops->compose_msi_msg)
		return -1;

	return msi_ops->compose_msi_msg(hwirq, msi_addr, msi_data);
}

void set_msi_irq_ops(struct msi_irq_ops *ops)
{
	msi_ops = ops;
}

void set_irq_handler(do_irq_handler_t handler)
{
	do_irq_handler = handler;
}
