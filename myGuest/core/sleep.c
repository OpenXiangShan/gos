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
#include "asm/type.h"
#include "string.h"
#include "../drivers/scheduler.h"
#include "print.h"

int sleep(int ms)
{
	scheduler_sleep_to_timeout(ms);

	return 0;
}

int schedule(void)
{
	scheduler_schedule();

	return 0;
}

int scheduler_early_init(struct device_init_entry *entry)
{
	struct device_init_entry *device_entry = entry;
	while (strncmp(device_entry->compatible, "THE END", sizeof("THE END"))) {
		if (!strncmp(device_entry->compatible, "scheduler", 128)) {
			scheduler_init(device_entry->start, device_entry->len,
				       NULL);
		}
		device_entry++;
	}

	return 0;
}
