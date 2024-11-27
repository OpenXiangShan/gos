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
#include "percpu.h"
#include "ipi.h"
#include "list.h"
#include "mm.h"
#include "print.h"

static struct ipi_ops *ipi_ops = NULL;
static DEFINE_PER_CPU(struct ipi_msg_ctrl, ipi_msg_ctrls);

typedef void (*ipi_handler_t)(int cpu, void *data);

static void ipi_do_nothing(int cpu, void *data)
{
	print("%s cpu%d\n", __FUNCTION__, cpu);
}

static ipi_handler_t ipi_handler[] = {
	ipi_do_nothing,
};

#define IPI_HANDLER_CNT (sizeof(ipi_handler) / sizeof(ipi_handler[0]))

int process_ipi(int cpu)
{
	struct ipi_msg_ctrl *ctrl;
	struct ipi_msg *msg, *tmp;

	ctrl = &per_cpu(ipi_msg_ctrls, cpu);
	if (!ctrl)
		return -1;

	spin_lock(&ctrl->lock);
	list_for_each_entry_safe(msg, tmp, &ctrl->ipi_msg, list) {
		if (msg->id >= IPI_HANDLER_CNT) {
			list_del(&msg->list);
			continue;
		}
		ipi_handler[msg->id] (cpu, msg->data);
		list_del(&msg->list);
	}
	spin_unlock(&ctrl->lock);

	return 0;
}

int send_ipi(int cpu, int id, void *data)
{
	struct ipi_msg_ctrl *ctrl;
	struct ipi_msg *msg;
	int flags;

	if (!cpu_is_online(cpu))
		return -1;

	ctrl = &per_cpu(ipi_msg_ctrls, cpu);
	if (!ctrl)
		return -1;

	msg = (struct ipi_msg *)mm_alloc(sizeof(struct ipi_msg));
	if (!msg)
		return -1;

	msg->id = id;
	msg->data = data;

	spin_lock_irqsave(&ctrl->lock, flags);
	list_add_tail(&msg->list, &ctrl->ipi_msg);
	spin_unlock_irqrestore(&ctrl->lock, flags);

	if (ipi_ops && ipi_ops->send_ipi)
		return ipi_ops->send_ipi(cpu, id, data);

	return -1;
}

void register_ipi(struct ipi_ops *ops)
{
	ipi_ops = ops;
}

void ipi_percpu_init(void)
{
	int cpu;
	struct ipi_msg_ctrl *ctrl;

	for_each_online_cpu(cpu) {
		ctrl = &per_cpu(ipi_msg_ctrls, cpu);
		INIT_LIST_HEAD(&ctrl->ipi_msg);
		__SPINLOCK_INIT(&ctrl->lock);
	}
}
