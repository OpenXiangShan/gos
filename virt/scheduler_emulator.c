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

#include "machine.h"
#include "virt.h"
#include "print.h"
#include "mm.h"
#include "task.h"

#define REG_DO_WHAT 0x0
#define REG_TIMEOUT 0x4

#define SLEEP 1
#define SCHEDULE 2

static unsigned int timeout_ms = 1000;

static void scheduler_mmio_write(struct memory_region *region,
				 unsigned long addr, unsigned long val,
				 unsigned int len)
{
	unsigned long reg = addr - region->start;

	if (reg == REG_DO_WHAT) {
		if (val == SLEEP) {
			sleep_to_timeout(timeout_ms);
		} else if (val == SCHEDULE) {
			schedule();
		}
	} else if (reg == REG_TIMEOUT) {
		timeout_ms = val;
	}
}

static unsigned long scheduler_mmio_read(struct memory_region *region,
					 unsigned long addr, unsigned int len)
{
	return 0;
}

static const struct memory_region_ops scheduler_mmio_ops = {
	.write = scheduler_mmio_write,
	.read = scheduler_mmio_read,
};

int create_scheduler_device(struct virt_machine *machine, int id,
			    unsigned long base, unsigned int size)
{
	return add_memory_region(machine, id, base, size, &scheduler_mmio_ops);
}
