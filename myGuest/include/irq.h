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

#ifndef __GUEST_IRQ_H__
#define __GUEST_IRQ_H__

typedef int (*do_irq_handler_t)(void);

struct msi_irq_ops {
	int (*alloc_irqs)(int nr);
	int (*compose_msi_msg)(int hwirq, unsigned long *msi_addr,
			       unsigned long *msi_data);
	int (*mask_irq)(int hwirq);
	int (*unmask_irq)(int hwirq);
};

int irq_handler(void);
void set_msi_irq_ops(struct msi_irq_ops *ops);
int alloc_msi_irqs(int nr);
int compose_msi_msg(int hwirq, unsigned long *msi_addr,
		    unsigned long *msi_data);
void set_irq_handler(do_irq_handler_t handler);

#endif
