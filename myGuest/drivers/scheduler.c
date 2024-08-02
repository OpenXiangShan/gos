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

#include "device.h"
#include "asm/mmio.h"
#include "asm/type.h"
#include "print.h"
#include "vmap.h"

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
